#ifndef _TOKEN_DATA_TABLE_H
#define _TOKEN_DATA_TABLE_H

#include <token/token_data.h>
#include <type.h>

#define TOKEN_DATA_TABLE_INIT_CAPACITY 4

/* ============================================================
 *  Token data table
 * ============================================================ */

struct token_data_table
{
  struct token_data **tokens;
  u32                 size;
  u32                 capacity;
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct token_data_table *token_data_table_alloc(u0);
struct token_data_table *token_data_table_init(u0);
u0                       token_data_table_free(struct token_data_table **tdt);

/* ============================================================
 *  Management
 * ============================================================ */

u0 token_data_table_expand(struct token_data_table *tdt);
u0 token_data_table_push(struct token_data_table *tdt, struct token_data *td);
u0 token_data_table_free_contents(struct token_data_table *tdt); /* frees tokens but not the struct itself */

/* ============================================================
 *  Debug
 * ============================================================ */

u0 token_data_table_print(struct token_data_table *tdt);

#endif /* _TOKEN_DATA_TABLE_H */
