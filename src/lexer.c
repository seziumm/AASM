#include <directive_look_up.h>
#include <rv32/instr_look_up.h>
#include <rv32/reg/reg_look_up.h>
#include <rv32/reg/reg.h>
#include <lexer.h>
#include <token/token_array.h>
#include <token/token_type.h>
#include <utils/aalloc.h>
#include <utils/common.h>
#include <string.h>
#include <ctype.h>

static inline i32 isupperdigit(i32 ch)
{
  return isupper(ch) || isdigit(ch);
}

static inline i32 isnotnewline(i32 ch)
{
  return ch != '\n';
}

static inline u32 count_while(const char *c, i32 (*pred)(i32))
{
  const char *base = c;
  while (*c != '\0' && pred((unsigned char)*c))
    c++;
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

static inline u0 lexer_push_token(struct lexer *l, struct token *td)
{
  token_array_push(&l->table, td);
}

static inline u0 lexer_push(struct lexer *l, enum token_type t, char *value)
{
  lexer_push_token(l, token_create(t, value));
}

static u32 lex_whitespace(struct lexer *l, const char *c)
{
  (void)l;
  return (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n');
}

static u32 lex_comment(struct lexer *l, const char *c)
{
  (void)l;
  if (*c != '#') return 0;
  return count_while(c, isnotnewline);
}

static u32 lex_single(struct lexer *l, const char *c)
{
  switch (*c)
  {
    case ',': lexer_push(l, TOKEN_COMMA,  strdup(",")); return 1;
    case '(': lexer_push(l, TOKEN_LPAREN, strdup("(")); return 1;
    case ')': lexer_push(l, TOKEN_RPAREN, strdup(")")); return 1;
  }

  return 0;
}

static u32 lex_directive(struct lexer *l, const char *c)
{
  if (*c != '.') return 0;

  u32 sz = count_while(c + 1, isupper) + 1;
  char *word = strndup(c, sz);

  if(directive_look_up(word))
  {
    lexer_push(l, TOKEN_DIRECTIVE, strndup(c, sz));
    return sz;

  }

  die(1, "LEXER: UNKNOWN DIRECTIVE '%s'", word);

  return 0; // we shut up the warning
}

static u32 lex_label_ref(struct lexer *l, const char *c)
{
  if (*c != '@') return 0;
  u32 sz = count_while(c + 1, isupperdigit) + 1;
  lexer_push(l, TOKEN_LABEL_REF, strndup(c, sz));
  return sz;
}

static u32 lex_label(struct lexer *l, const char *c)
{
  if (*c != '&') return 0;
  u32 sz = count_while(c + 1, isupperdigit) + 1;
  lexer_push(l, TOKEN_LABEL, strndup(c, sz));
  return sz;
}

static u32 lex_number(struct lexer *l, const char *c)
{
  if (!isdigit((unsigned char)*c) && *c != '-') return 0;
  u32 hassign = (*c == '-');
  u32 sz      = count_while(c + hassign, isdigit) + hassign;
  lexer_push(l, TOKEN_NUMBER, strndup(c, sz));
  return sz;
}

static u32 lex_word(struct lexer *l, const char *c)
{
  if (!isupper((unsigned char)*c)) return 0;

  u32   sz   = count_while(c, isupperdigit);
  char *word = strndup(c, sz);

  if (reg_look_up(word))
  {
    lexer_push(l, TOKEN_REGISTER, word);
  }
  else if (instr_look_up(word))
  {
    lexer_push(l, TOKEN_INSTR, word);
  }
  else
  {
    die(1, "LEXER: UNKNOWN WORD '%s'", word);
  }

  return sz;
}

typedef u32 (*lex_fn)(struct lexer *, const char *);
static const lex_fn lex_fns[] = 
{
  lex_whitespace,
  lex_comment,
  lex_single,
  lex_directive,
  lex_label_ref,
  lex_label,
  lex_number,
  lex_word,
  NULL,
};

struct lexer *lexer_compile(const char *c)
{
  struct lexer *l = lexer_create();

  while (*c != '\0')
  {
    u32 ok = 0;

    for (u32 i = 0; lex_fns[i]; i++)
    {
      u32 n = lex_fns[i](l, c);

      if (n > 0) 
      { 
        c += n; 
        ok = 1; 
        break; 
      }
    }

    if (unlikely(!ok))
    {
      die(1, "LEXER: UNEXPECTED CHARACTER '%c'", *c, (unsigned char)*c);
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
