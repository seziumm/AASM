#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <symbol/symbol.h>
#include <type.h>

#define SYMBOL_INIT_CAPACITY 4

/* ============================================================
 *  Symbol table
 * ============================================================ */

struct symbol_table
{
  struct symbol **symbols;
  u32             size;
  u32             capacity;
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct symbol_table *symbol_table_alloc(u0);
struct symbol_table *symbol_table_init(u0);
u0                   symbol_table_free(struct symbol_table **st);
u0                   symbol_table_free_contents(struct symbol_table *st); /* frees symbols but not the struct itself */

/* ============================================================
 *  Management
 * ============================================================ */

u0  symbol_table_expand(struct symbol_table *st);
u0  symbol_table_add(struct symbol_table *st, const char *name, u32 addr);
i32 symbol_table_lookup(struct symbol_table *st, const char *name);

/* ============================================================
 *  Debug
 * ============================================================ */

u0 symbol_table_print(struct symbol_table *st);

#endif /* _SYMBOL_TABLE_H */
