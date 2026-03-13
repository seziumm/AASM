#ifndef _TOKEN_H
#define _TOKEN_H

#include <token/token_type.h>

struct token
{
  enum token_type  type;   /* token class      */
  char            *value;  /* raw lexed string */
};

struct token      *token_create(enum token_type t, char *value);
u0                 token_free(struct token **td);
u0                 token_free_contents(struct token **td);

u0 token_print(struct token *td);

#endif /* _TOKEN_H */
