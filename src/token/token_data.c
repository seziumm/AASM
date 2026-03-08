#include <token/token_data.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct token_data *token_data_alloc(u0)
{
  struct token_data *td = malloc(sizeof(struct token_data));

  if (NULL == td)
    token_die(NULL, 1, "malloc() failed in token_data_alloc()");

  return td;
}

struct token_data *token_data_create(enum token_type t, char *value)
{
  struct token_data *td = token_data_alloc();

  td->type  = t;
  td->value = value;

  return td;
}

u0 token_data_free(struct token_data **td)
{
  if (NULL == td || NULL == *td) return;

  free((*td)->value);
  free(*td);
  *td = NULL;
}

/* ============================================================
 *  Debug
 * ============================================================ */

u0 token_data_print(struct token_data *td)
{
  if (NULL == td) return;
  printf("TOKEN(%s, \"%s\")\n", token_to_str(td->type), td->value);
}
