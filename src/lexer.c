#include "rv32/rv32_type.h"
#include <string.h>
#include <common.h>
#include <ctype.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>


/* DEBUG STUFF */

void token_data_print(struct token_data *td) 
{
  if(NULL == td) return;
  printf("%s\t%s\n", token_type_print(td->type), td->value);
}
void lexer_print(struct lexer *l) 
{
  if(NULL == l) return;
  printf("\nSIZE: %d\t\tCAPACITY %d\n", l->size, l->capacity);
  for(u32 i = 0; i < l->size; i++)
  {
    token_data_print(l->tokens[i]);
  }
}

char *token_type_print(enum token t)
{
  switch (t) 
  {
    case token_comma:     return "COMMA";
    case token_lparen:    return "LPAREN";
    case token_rparen:    return "RPAREN";
    case token_number:    return "NUMBER";
    case token_instr:     return "INSTR";
    case token_register:  return "REG";
    case token_section:   return "SECT";
    case token_tag:       return "TAG";
  }

  return "";
}

/* ============= */


i32 is_register(char *p)
{
  /* ---------- ABI ---------- */
  static char *abi[] = 
  {
    "ZERO","RA","SP","GP","TP",
    "T0","T1","T2","T3","T4","T5","T6",
    "S0","S1","S2","S3","S4","S5","S6","S7","S8","S9","S10","S11",
    "A0","A1","A2","A3","A4","A5","A6","A7"
  };

  for (u32 i = 0; i < sizeof(abi)/sizeof(*abi); i++)
  {
    if (strcmp(p, abi[i]) == 0) return 1;
  }

  /* ---------- X0..X31 ---------- */
  static char *reg[] = 
  {
    "X0","X1","X2","X3","X4","X5","X6","X7","X8","X9",
    "X10","X11","X12","X13","X14","X15","X16","X17","X18","X19",
    "X20","X21","X22","X23","X24","X25","X26","X27","X28","X29",
    "X30","X31"
  };

  for (u32 i = 0; i < sizeof(reg)/sizeof(*reg); i++)
  {
    if (strcmp(p, reg[i]) == 0) return 1;
  }

  return 0;
}

i32 is_instr(char *p)
{

  for (u32 i = 0; i < sizeof(rv32ii) / sizeof(*rv32ii); i++)
  {
    if (strcmp(p, rv32ii[i].label) == 0) return 1;
  }

  return 0;
}

u32 lexer_count_while(char *c, i32 (*pred)(i32)) 
{
  u32 sz = 0;
  while (*c != '\0' && pred((unsigned char)*c)) 
  {
    c++;
    sz++;
  }
  return sz;
}

i32 isupperdigit(i32 ch) 
{
    return isupper(ch) || isdigit(ch);
}

i32 isnotnewline(i32 ch) 
{
  return ch != '\n';
}

struct lexer *lexer_compile(char *c)
{
  struct lexer *l = lexer_init();

  while(*c != '\0') 
  {
    switch (*c) 
    {
      case ' ':
      case '\n':
        c++;
        break;
      case '#':
          c += lexer_count_while(c, isnotnewline);
          break;
      case ',':
        c++;
        lexer_push_token_value(l, token_comma, strdup(","));
        break;
      case '(':
        c++;
        lexer_push_token_value(l, token_lparen, strdup("("));
        break;
      case ')':
        c++;
        lexer_push_token_value(l, token_rparen, strdup(")"));
        break;
      default:

        if(*c == '.')
        {
          u32 sz = lexer_count_while(c + 1, isupper) + 1;
          char *new_c = strndup(c, sz);

          lexer_push_token_value(l, token_section, new_c);

          c += sz;
        }
        else if(*c == '&')
        {
          u32 sz = lexer_count_while(c + 1, isupperdigit) + 1;
          char *new_c = strndup(c, sz);

          lexer_push_token_value(l, token_tag, new_c);

          c += sz;

        }
        else if(isdigit(*c) || *c == '+' || *c == '-')
        {
          u32 hassign = (*c == '+' || *c == '-');
          u32 sz = lexer_count_while(c + hassign, isdigit) + hassign;
          char *new_c = strndup(c, sz);
          lexer_push_token_value(l, token_number, new_c);

          c += sz;
        }
        else if(isupper(*c))
        {
          u32 sz = lexer_count_while(c, isupperdigit);
          char *new_c = strndup(c, sz);


          if(is_register(new_c))
          {
            lexer_push_token_value(l, token_register, new_c);
          }
          else if(is_instr(new_c))
          {
            lexer_push_token_value(l, token_instr, new_c);
          }
          else 
          {
            lexer_die(l, 1, "NO WORD FOUND %c\n", *c);
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


void lexer_push_token_value(struct lexer *l, enum token t, char *value)
{
  lexer_push_token_data(l, token_data_alloc(t, value));
}

struct token_data *token_data_alloc(enum token t, char *value)
{
  struct token_data *td = malloc(sizeof(struct token_data));

  if(NULL == td)
  {
    lexer_die(NULL, 1, "A malloc() error occurred");
  }

  td->type  = t;
  td->value = value;

  return td;

}

void token_data_free(struct token_data *td)
{

  free(td->value);
  free(td);

  td = NULL;

}

void lexer_expand(struct lexer *l)
{
    u32 new_capacity = l->capacity * 2;

    struct token_data **new_tokens = realloc(l->tokens, new_capacity * sizeof(struct token_data *));

    if (new_tokens == NULL) 
    {
      lexer_die(l, 1, "A realloc() error occurred");
    }

    l->tokens = new_tokens;
    l->capacity = new_capacity;
}


void lexer_push_token_data(struct lexer *l, struct token_data *td)
{

  if(l->size >= l->capacity)
  {
    lexer_expand(l);
  }

  l->tokens[l->size] = td;
  l->size++;
}


struct lexer *lexer_init(void)
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

void lexer_free(struct lexer *l) 
{

  for(u32 i = 0; i < l->size; i++)
  {
    token_data_free(l->tokens[i]);
  }

  free(l->tokens);
  free(l);

  l = NULL;
}

