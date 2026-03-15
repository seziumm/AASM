#include <directive_look_up.h>
#include <rv32/instr_look_up.h>
#include <rv32/reg/reg_look_up.h>
#include <lexer.h>
#include <token/token_array.h>
#include <token/token_type.h>
#include <utils/aalloc.h>
#include <utils/common.h>
#include <string.h>
#include <ctype.h>

static inline i32 isupperdigit(i32 ch) { return isupper(ch) || isdigit(ch); }
static inline i32 isnotnewline(i32 ch) { return ch != '\n'; }
static inline i32 isnotquote  (i32 ch) { return ch != '"';  }

static inline u32 count_while(const char *c, i32 (*pred)(i32))
{
  const char *base = c;
  while (*c != '\0' && pred((unsigned char)*c)) c++;
  return (u32)(c - base);
}

static struct lexer *lexer_create(u0)
{
  struct lexer  *l      = a_malloc(sizeof(struct lexer));
  struct token **tokens = a_malloc(LEXER_INIT_CAPACITY * sizeof(struct token *));

  l->table = (struct token_array)
  {
    .tokens   = tokens,
    .size     = 0,
    .capacity = LEXER_INIT_CAPACITY,
  };

  return l;
}

static inline u0 lexer_push(struct lexer *l, enum token_type t, char *value)
{
  token_array_push(&l->table, token_create(t, value));
}

static inline u32 lex_whitespace(struct lexer *l, const char *c)
{
  (u0)l; (u0)c;
  return 1;
}

static inline u32 lex_comment(struct lexer *l, const char *c)
{
  (u0)l;
  return count_while(c, isnotnewline);
}

static inline u32 lex_comma(struct lexer *l, const char *c)
{
  (u0)c;
  lexer_push(l, TOKEN_COMMA, strdup(","));
  return 1;
}

static inline u32 lex_lparen(struct lexer *l, const char *c)
{
  (u0)c;
  lexer_push(l, TOKEN_LPAREN, strdup("("));
  return 1;
}

static inline u32 lex_rparen(struct lexer *l, const char *c)
{
  (u0)c;
  lexer_push(l, TOKEN_RPAREN, strdup(")"));
  return 1;
}

static u32 lex_directive(struct lexer *l, const char *c)
{

  u32   sz   = count_while(c + 1, isupperdigit) + 1;
  char *word = strndup(c, sz);

  if (!directive_look_up(word))
  {
    die(1, "LEXER: unknown directive '%s'", word);
  }

  lexer_push(l, TOKEN_DIRECTIVE, word);
  return sz;
}

static inline u32 lex_label_ref(struct lexer *l, const char *c)
{
  u32 sz = count_while(c + 1, isupperdigit) + 1;
  lexer_push(l, TOKEN_LABEL_REF, strndup(c, sz));
  return sz;
}

static inline u32 lex_label(struct lexer *l, const char *c)
{
  u32 sz = count_while(c + 1, isupperdigit) + 1;
  lexer_push(l, TOKEN_LABEL, strndup(c, sz));
  return sz;
}

static inline u32 lex_number(struct lexer *l, const char *c)
{
  u32 hassign = (*c == '-');
  u32 digits  = count_while(c + hassign, isdigit);

  if (unlikely(hassign && digits == 0)) die(1, "LEXER: unkown number");

  u32 sz = digits + hassign;
  lexer_push(l, TOKEN_NUMBER, strndup(c, sz));
  return sz;
}

static u32 lex_word(struct lexer *l, const char *c)
{

  u32   sz   = count_while(c, isupperdigit);
  char *word = strndup(c, sz);

  if (reg_look_up(word))
  {
    lexer_push(l, TOKEN_REGISTER, word);
    return sz;
  }

  if (instr_look_up(word))
  {
    lexer_push(l, TOKEN_INSTR, word);
    return sz;
  }

  die(1, "LEXER: unknown word '%s'", word);
  return 0;
}

static u32 lex_string(struct lexer *l, const char *c)
{
  c++;
  u32 sz = count_while(c, isnotquote);

  if (unlikely(*(c + sz) != '"'))
  {
    die(1, "LEXER: unterminated string literal");
  }

  lexer_push(l, TOKEN_STRING, strndup(c, sz));
  return sz + 2;
}

struct lexer *lexer_compile(const char *c)
{
  struct lexer *l = lexer_create();

  while (*c != '\0')
  {
    switch (*c)
    {
      case ' ': case '\t': case '\r': case '\n':
        c += lex_whitespace(l, c); break;
      case '#':
        c += lex_comment(l, c); break;

      case ',': c += lex_comma  (l, c); break;
      case '(': c += lex_lparen (l, c); break;
      case ')': c += lex_rparen (l, c); break;

      case '.': c += lex_directive (l, c); break;
      case '@': c += lex_label_ref (l, c); break;
      case '&': c += lex_label     (l, c); break;
      case '"': c += lex_string    (l, c); break;

      case '-':
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        c += lex_number(l, c); break;

      case 'A': case 'B': case 'C': case 'D': case 'E':
      case 'F': case 'G': case 'H': case 'I': case 'J':
      case 'K': case 'L': case 'M': case 'N': case 'O':
      case 'P': case 'Q': case 'R': case 'S': case 'T':
      case 'U': case 'V': case 'W': case 'X': case 'Y':
      case 'Z':
        c += lex_word(l, c); break;

      default:
        die(1, "LEXER: unexpected character '%c'", *c);
    }
  }

  return l;
}

u0 lexer_free(struct lexer **l)
{
  if (NULL == l || NULL == *l) return;
  token_array_free_contents(&(*l)->table);
  a_free(*l);
  *l = NULL;
}

u0 lexer_print(struct lexer *l)
{
  if (NULL == l) return;
  token_array_print(&l->table);
}
