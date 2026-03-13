#ifndef _TOKEN_ARRAY_H
#define _TOKEN_ARRAY_H

#include <token/token.h>

#define TOKEN_ARRAY_INIT_CAPACITY 4

struct token_array
{
  struct token      **tokens;
  u32                 size;
  u32                 capacity;
};

struct token_array      *token_array_create(u0);
u0                       token_array_free_contents(struct token_array *tdt); /* frees tokens but not the struct itself */
u0                       token_array_free(struct token_array **tdt);
u0                       token_array_push(struct token_array *tdt, struct token *td);

u0                       token_array_print(struct token_array *tdt);

#endif /* _TOKEN_TABLE_H */
