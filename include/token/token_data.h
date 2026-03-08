#ifndef _TOKEN_DATA_H
#define _TOKEN_DATA_H

#include <token/token_type.h>
#include <common.h>
#include <type.h>

#define token_die(t, err, ...)  \
  do                            \
  {                             \
    /*token_data_print(t);*/    \
    die(err, __VA_ARGS__);      \
  } while (0)

/* ============================================================
 *  Token data
 * ============================================================ */

struct token_data
{
  enum token_type  type;   /* token class      */
  char            *value;  /* raw lexed string */
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct token_data *token_data_alloc(u0);
struct token_data *token_data_create(enum token_type t, char *value);
u0                 token_data_free(struct token_data **td);

/* ============================================================
 *  Debug
 * ============================================================ */

u0 token_data_print(struct token_data *td);

#endif /* _TOKEN_DATA_H */
