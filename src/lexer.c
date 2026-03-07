#include <lexer.h>
#include <rv32/rv32_reg.h>
#include <rv32/rv32_instr.h>
#include <stdlib.h>
#include <ctype.h>

static inline u0 lexer_push_token_value(struct lexer *l, enum token_type t, char *value)
{
  lexer_push_token_data(l, token_data_create(t, value));
}

static inline u32 count_while(const char *c, i32 (*pred)(i32)) 
{
  u32 sz = 0;
  while (*c != '\0' && pred((unsigned char)*c)) 
  {
    c++;
    sz++;
  }
  return sz;
}

static inline i32 isupperdigit(i32 ch) 
{
  return isupper(ch) || isdigit(ch);
}

static inline i32 isnotnewline(i32 ch) 
{
  return ch != '\n';
}

struct lexer *lexer_compile(const char *c)
{
  struct lexer *l = lexer_init();

  while(*c != '\0') 
  {
    switch (*c) 
    {
      case ' ':
      case '\n':
        {
          c++;
          break;
        }
      case '#':
        {
          c += count_while(c, isnotnewline);
          break;
        }
      case ',':
        {
          c++;
          lexer_push_token_value(l, TOKEN_COMMA, strdup(","));
          break;
        }
      case '(':
        {
          c++;
          lexer_push_token_value(l, TOKEN_LPAREN, strdup("("));
          break;
        }
      case ')':
        {
          c++;
          lexer_push_token_value(l, TOKEN_RPAREN, strdup(")"));
          break;
        }
      case '.':
        {
          u32 sz = count_while(c + 1, isupper) + 1;
          char *new_c = strndup(c, sz);

          lexer_push_token_value(l, TOKEN_SECTION, new_c);

          c += sz;
          break;

        }
      case '@':
        {
          u32 sz = count_while(c + 1, isupperdigit) + 1;
          char *new_c = strndup(c, sz);

          lexer_push_token_value(l, TOKEN_LABEL_REF, new_c);

          c += sz;
          break;

        }
      case '&':
        {
          u32 sz = count_while(c + 1, isupperdigit) + 1;
          char *new_c = strndup(c, sz);

          lexer_push_token_value(l, TOKEN_LABEL, new_c);

          c += sz;
          break;
        }

      default:

        if(isdigit(*c) || *c == '+' || *c == '-')
        {
          u32 hassign = (*c == '+' || *c == '-');
          u32 sz = count_while(c + hassign, isdigit) + hassign;
          char *new_c = strndup(c, sz);
          lexer_push_token_value(l, TOKEN_NUMBER, new_c);

          c += sz;
        }
        else if(isupper(*c))
        {
          u32 sz = count_while(c, isupperdigit);
          char *new_c = strndup(c, sz);

          if(-1 != rv32_reg_lookup(new_c))
          {
            lexer_push_token_value(l, TOKEN_REGISTER, new_c);
          }
          else if(rv32ii_is_instr(new_c))
          {
            lexer_push_token_value(l, TOKEN_INSTR, new_c);
          }
          else 
          {
            lexer_die(l, 1, "BAD WORD %s\n", new_c);

          }

          c += sz;
        }
        else 
        {
          lexer_die(l, 1, "BAD CHARACTER %c\n", *c);
        }


        break;

    }
  }

  return l;
}


u0 lexer_expand(struct lexer *l)
{
    u32 new_capacity = l->capacity * 2;

    struct token_data **new_tokens = realloc(l->tokens, new_capacity * sizeof(struct token_data *));

    if(NULL == new_tokens) 
    {
      lexer_die(l, 1, "A realloc() error occurred");
    }

    l->tokens = new_tokens;
    l->capacity = new_capacity;
}


u0 lexer_push_token_data(struct lexer *l, struct token_data *td)
{

  if(l->size >= l->capacity)
  {
    lexer_expand(l);
  }

  l->tokens[l->size] = td;
  l->size++;
}



struct lexer *lexer_init(u0)
{
  struct lexer *l = malloc(sizeof(struct lexer));

  if(NULL == l)
  {
    lexer_die(NULL, 1, "A malloc() error occurred");
  }

  l->size     = 0;
  l->capacity = LEXER_INIT_CAPACITY;

  l->tokens = malloc(l->capacity * (sizeof(struct token_data *)));

  if(NULL == l->tokens)
  {
    lexer_die(NULL, 1, "A malloc() error occurred");
  }

  return l;

}

u0 lexer_free(struct lexer **l) 
{
  if(NULL == l || NULL == *l) return;

  for(u32 i = 0; i < (*l)->size; i++)
  {
    token_data_free(&(*l)->tokens[i]);
  }

  free((*l)->tokens);
  free(*l);
  *l = NULL;
}

/* DEBUG */

u0 lexer_print(struct lexer *l) 
{
  if(NULL == l) return;
  for(u32 i = 0; i < l->size; i++)
  {
    token_data_print(l->tokens[i]);
  }
}

