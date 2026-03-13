#include <token/token_array.h>
#include <utils/aalloc.h>
#include <stdlib.h>


struct token_array *token_array_create(u0)
{
  struct token_array *tdt    = a_malloc(sizeof(struct token_array));
  struct token      **tokens = a_malloc(TOKEN_ARRAY_INIT_CAPACITY * sizeof(struct token *));

  *tdt = (struct token_array)
  {
    .tokens   = tokens,
    .size     = 0,
    .capacity = TOKEN_ARRAY_INIT_CAPACITY,
  };

  return tdt;
}

u0 token_array_free(struct token_array **tdt)
{
  if (NULL == tdt || NULL == *tdt) return;

  token_array_free_contents(*tdt);
  a_free(*tdt);
  *tdt = NULL;
}

u0 token_array_free_contents(struct token_array *tdt)
{
  if (NULL == tdt) return;

  for (u32 i = 0; i < tdt->size; i++)
    token_free(&tdt->tokens[i]);

  a_free(tdt->tokens);

  *tdt = (struct token_array)
  {
    .tokens = NULL,
    .size = 0,
    .capacity = 0
  };
}

static u0 token_array_expand(struct token_array *tdt)
{
  u32               new_capacity = tdt->capacity * 2;
  struct token **new_tokens = a_realloc(tdt->tokens, new_capacity * sizeof(struct token *));

  tdt->tokens   = new_tokens;
  tdt->capacity = new_capacity;
}

u0 token_array_push(struct token_array *tdt, struct token *td)
{
  if (NULL == tdt || NULL == td) return;

  if (tdt->size >= tdt->capacity)
    token_array_expand(tdt);

  tdt->tokens[tdt->size++] = td;
}

u0 token_array_print(struct token_array *tdt)
{
  if (NULL == tdt) return;

  for (u32 i = 0; i < tdt->size; i++)
    token_print(tdt->tokens[i]);
}
