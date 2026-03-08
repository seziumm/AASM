#include <token/token_data_table.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct token_data_table *token_data_table_alloc(u0)
{
  struct token_data_table *tdt = malloc(sizeof(struct token_data_table));

  if (NULL == tdt)
    die(1, "malloc() failed in token_data_table_alloc()");

  return tdt;
}

struct token_data_table *token_data_table_init(u0)
{
  struct token_data_table *tdt    = token_data_table_alloc();
  struct token_data      **tokens = malloc(TOKEN_DATA_TABLE_INIT_CAPACITY * sizeof(struct token_data *));

  if (NULL == tokens)
  {
    free(tdt);
    die(1, "malloc() failed in token_data_table_init()");
  }

  *tdt = (struct token_data_table)
  {
    .tokens   = tokens,
    .size     = 0,
    .capacity = TOKEN_DATA_TABLE_INIT_CAPACITY,
  };

  return tdt;
}

u0 token_data_table_free(struct token_data_table **tdt)
{
  if (NULL == tdt || NULL == *tdt) return;

  token_data_table_free_contents(*tdt);
  free(*tdt);
  *tdt = NULL;
}

u0 token_data_table_free_contents(struct token_data_table *tdt)
{
  if (NULL == tdt) return;

  for (u32 i = 0; i < tdt->size; i++)
    token_data_free(&tdt->tokens[i]);

  free(tdt->tokens);
  tdt->tokens   = NULL;
  tdt->size     = 0;
  tdt->capacity = 0;
}

/* ============================================================
 *  Management
 * ============================================================ */

u0 token_data_table_expand(struct token_data_table *tdt)
{
  u32               new_capacity = tdt->capacity * 2;
  struct token_data **new_tokens = realloc(tdt->tokens, new_capacity * sizeof(struct token_data *));

  if (NULL == new_tokens)
    die(1, "realloc() failed in token_data_table_expand()");

  tdt->tokens   = new_tokens;
  tdt->capacity = new_capacity;
}

u0 token_data_table_push(struct token_data_table *tdt, struct token_data *td)
{
  if (NULL == tdt || NULL == td) return;

  if (tdt->size >= tdt->capacity)
    token_data_table_expand(tdt);

  tdt->tokens[tdt->size++] = td;
}

/* ============================================================
 *  Debug
 * ============================================================ */

u0 token_data_table_print(struct token_data_table *tdt)
{
  if (NULL == tdt) return;

  for (u32 i = 0; i < tdt->size; i++)
    token_data_print(tdt->tokens[i]);
}
