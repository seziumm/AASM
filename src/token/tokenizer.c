#include <directive_utils.h>
#include <rv32/instr_utils.h>
#include <rv32/reg/reg_utils.h>
#include <token/tokenizer.h>
#include <token/token_array.h>
#include <token/token_type.h>
#include <utils/aalloc.h>
#include <utils/common.h>
#include <string.h>
#include <ctype.h>


static inline i32 isupperdigit(i32 ch) { return isupper(ch) || isdigit(ch) || (ch == '_'); }
static inline i32 isnotnewline(i32 ch) { return ch != '\n'; }
static inline i32 isnotquote  (i32 ch) { return ch != '"';  }

static inline u32 count_while(const char *c, i32 (*pred)(i32))
{
  const char *base = c;
  while (*c != '\0' && pred((unsigned char)*c)) c++;
  return (u32)(c - base);
}


static struct tokenizer *tokenizer_create(u0)
{
  struct tokenizer *t      = a_malloc(sizeof(struct tokenizer));
  struct token    **tokens = a_malloc(TOKENIZER_INIT_CAPACITY * sizeof(struct token *));

  t->table = (struct token_array)
  {
    .tokens   = tokens,
    .size     = 0,
    .capacity = TOKENIZER_INIT_CAPACITY,
  };

  return t;
}

static inline u0 tok_push(struct tokenizer *t, enum token_type type, char *value)
{
  token_array_push(&t->table, token_create(type, value));
}


/* Whitespace — consumed silently, no token emitted. */
u32 tok_whitespace(struct tokenizer *t, const char *c)
{
  (u0)t; (u0)c;
  return 1;
}

/* Comment '#' to end of line — consumed silently, no token emitted. */
u32 tok_comment(struct tokenizer *t, const char *c)
{
  (u0)t;
  return count_while(c, isnotnewline);
}

/* TOKEN_COMMA ',' */
u32 tok_comma(struct tokenizer *t, const char *c)
{
  (u0)c;
  tok_push(t, TOKEN_COMMA, strdup(","));
  return 1;
}

/* TOKEN_LPAREN '(' */
u32 tok_lparen(struct tokenizer *t, const char *c)
{
  (u0)c;
  tok_push(t, TOKEN_LPAREN, strdup("("));
  return 1;
}

/* TOKEN_RPAREN ')' */
u32 tok_rparen(struct tokenizer *t, const char *c)
{
  (u0)c;
  tok_push(t, TOKEN_RPAREN, strdup(")"));
  return 1;
}

/* TOKEN_DIRECTIVE '.CODE', '.BYTE', ... */
u32 tok_directive(struct tokenizer *t, const char *c)
{
  u32   sz   = count_while(c + 1, isupperdigit) + 1; /* include the '.' */
  char *word = strndup(c, sz);

  if (directive_from_label(word))
  {
    tok_push(t, TOKEN_DIRECTIVE, word);
    return sz;

  }

  die(1, "TOKENIZER: unknown directive '%s'", word);
  return 0;
}

/* TOKEN_LABEL_REF '@NAME' */
u32 tok_label_ref(struct tokenizer *t, const char *c)
{
  u32 sz = count_while(c + 1, isupperdigit) + 1; /* include '@' */
  tok_push(t, TOKEN_LABEL_REF, strndup(c, sz));
  return sz;
}

/* TOKEN_LABEL '&NAME' */
u32 tok_label(struct tokenizer *t, const char *c)
{
  u32 sz = count_while(c + 1, isupperdigit) + 1; /* include '&' */
  tok_push(t, TOKEN_LABEL, strndup(c, sz));
  return sz;
}

/* TOKEN_NUMBER '123', '-4' */
u32 tok_number(struct tokenizer *t, const char *c)
{
  u32 hassign = (*c == '-');
  u32 digits  = count_while(c + hassign, isdigit);

  if (unlikely(hassign && digits == 0))
    die(1, "TOKENIZER: lone '-' is not a number");

  u32 sz = digits + hassign;
  tok_push(t, TOKEN_NUMBER, strndup(c, sz));
  return sz;
}

/* TOKEN_REGISTER or TOKEN_INSTR — both start with an uppercase letter */
u32 tok_word(struct tokenizer *t, const char *c)
{
  u32   sz   = count_while(c, isupperdigit);
  char *word = strndup(c, sz);

  if (reg_from_label(word))
  {
    tok_push(t, TOKEN_REGISTER, word);
    return sz;
  }

  if (instr_from_label(word))
  {
    tok_push(t, TOKEN_INSTR, word);
    return sz;
  }

  die(1, "TOKENIZER: unknown word '%s'", word);
  return 0;
}

/* TOKEN_STRING '"text"' — value stored without surrounding quotes */
u32 tok_string(struct tokenizer *t, const char *c)
{
  c++; /* skip opening quote */
  u32 sz = count_while(c, isnotquote);

  if (unlikely(*(c + sz) != '"'))
    die(1, "TOKENIZER: unterminated string literal");

  tok_push(t, TOKEN_STRING, strndup(c, sz));
  return sz + 2; /* + 2 for the two quote characters */
}


struct tokenizer *tokenizer_compile(const char *c)
{
  struct tokenizer *t = tokenizer_create();

  while (*c != '\0')
  {
    switch (*c)
    {
      case ' ': case '\t': case '\r': case '\n':
        c += tok_whitespace(t, c); break;
      case '#':
        c += tok_comment(t, c); break;

      case ',': c += tok_comma  (t, c); break;
      case '(': c += tok_lparen (t, c); break;
      case ')': c += tok_rparen (t, c); break;

      case '.': c += tok_directive(t, c); break;
      case '@': c += tok_label_ref(t, c); break;
      case '&': c += tok_label    (t, c); break;
      case '"': c += tok_string   (t, c); break;

      case '-':
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        c += tok_number(t, c); break;

      case 'A': case 'B': case 'C': case 'D': case 'E':
      case 'F': case 'G': case 'H': case 'I': case 'J':
      case 'K': case 'L': case 'M': case 'N': case 'O':
      case 'P': case 'Q': case 'R': case 'S': case 'T':
      case 'U': case 'V': case 'W': case 'X': case 'Y':
      case 'Z':
        c += tok_word(t, c); break;

      default:
        die(1, "TOKENIZER: unexpected character '%c'", *c);
    }
  }

  return t;
}


u0 tokenizer_free(struct tokenizer **t)
{
  if (NULL == t || NULL == *t) return;
  token_array_free_contents(&(*t)->table);
  a_free(*t);
  *t = NULL;
}

u0 tokenizer_print(struct tokenizer *t)
{
  if (NULL == t) return;
  token_array_print(&t->table);
}
