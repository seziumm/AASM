#include <utils/aalloc.h>
#include <utils/common.h>
#include <token/token.h>
#include <stdlib.h>

struct token *token_create(enum token_type t, char *value)
{
  struct token *td = a_malloc(sizeof(struct token));

  *td = (struct token)
  {
    .type = t,
    .value = value
  };

  return td;
}

u0 token_free_contents(struct token **td)
{
  a_free((*td)->value);
}

u0 token_free(struct token **td)
{
  if (NULL == td || NULL == *td) return;

  token_free_contents(td);
  a_free(*td);
  *td = NULL;
}

u0 token_print(struct token *td)
{
  if (NULL == td) return;
  debugf("TOKEN(%s, \"%s\")\n", token_type_to_str(td->type), td->value);
}
