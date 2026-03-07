#include <token.h>
#include <stdlib.h>
#include <stdio.h>

u0 token_data_free(struct token_data **td)
{
  if(NULL == td || NULL == *td) return;

  free((*td)->value);
  free(*td);

  *td = NULL;
}

struct token_data *token_data_alloc(u0) 
{
  struct token_data *td = malloc(sizeof(struct token_data));

  if(NULL == td)
  {
    token_die(NULL, 1, "A malloc() error occurred in token_data_alloc()");
  }

  return td;
}

struct token_data *token_data_create(enum token_type t, char *value)
{  

  struct token_data *td = token_data_alloc();

  td->type  = t;
  td->value = (char *)value;

  return td;

}

const char *token_to_str(enum token_type t)
{
  switch (t) 
  {
    case TOKEN_COMMA:     return "TOKEN_COMMA";
    case TOKEN_INSTR:     return "TOKEN_INSTR";
    case TOKEN_LABEL:     return "TOKEN_LABEL";
    case TOKEN_LABEL_REF: return "TOKEN_LABEL_REF";
    case TOKEN_LPAREN:    return "TOKEN_LPAREN";
    case TOKEN_NUMBER:    return "TOKEN_NUMBER";
    case TOKEN_REGISTER:  return "TOKEN_REG";
    case TOKEN_RPAREN:    return "TOKEN_RPAREN";
    case TOKEN_SECTION:   return "TOKEN_SECT";
  }

  return "";
}

/* DEBUG */

u0 token_data_print(struct token_data *td) 
{
  if(NULL == td) return;
  printf("TOKEN(%s, \"%s\")", token_to_str(td->type), td->value);
}


