#include <symbol/symbol_table.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct symbol_table *symbol_table_alloc(u0)
{
  struct symbol_table *st = malloc(sizeof(struct symbol_table));

  if (NULL == st)
    die(1, "malloc() failed in symbol_table_alloc()");

  return st;
}

struct symbol_table *symbol_table_init(u0)
{
  struct symbol_table *st      = symbol_table_alloc();
  struct symbol      **symbols = malloc(SYMBOL_INIT_CAPACITY * sizeof(struct symbol *));

  if (NULL == symbols)
  {
    free(st);
    die(1, "malloc() failed in symbol_table_init()");
  }

  *st = (struct symbol_table)
  {
    .symbols  = symbols,
    .size     = 0,
    .capacity = SYMBOL_INIT_CAPACITY,
  };

  return st;
}

u0 symbol_table_free_contents(struct symbol_table *st)
{
  if (NULL == st) return;

  for (u32 i = 0; i < st->size; i++)
    symbol_free(&st->symbols[i]);

  free(st->symbols);
  st->symbols  = NULL;
  st->size     = 0;
  st->capacity = 0;
}

u0 symbol_table_free(struct symbol_table **st)
{
  if (NULL == st || NULL == *st) return;

  symbol_table_free_contents(*st);
  free(*st);
  *st = NULL;
}

/* ============================================================
 *  Management
 * ============================================================ */

u0 symbol_table_expand(struct symbol_table *st)
{
  u32             new_capacity = st->capacity * 2;
  struct symbol **new_symbols  = realloc(st->symbols, new_capacity * sizeof(struct symbol *));

  if (NULL == new_symbols)
    die(1, "realloc() failed in symbol_table_expand()");

  st->symbols  = new_symbols;
  st->capacity = new_capacity;
}

u0 symbol_table_add(struct symbol_table *st, const char *name, u32 addr)
{
  if (st->size >= st->capacity)
    symbol_table_expand(st);

  st->symbols[st->size++] = symbol_create(name, addr);
}

i32 symbol_table_lookup(struct symbol_table *st, const char *name)
{
  for (u32 i = 0; i < st->size; i++)
    if (strcmp(st->symbols[i]->name, name) == 0)
      return (i32)st->symbols[i]->addr;

  return -1;
}

/* ============================================================
 *  Debug
 * ============================================================ */

u0 symbol_table_print(struct symbol_table *st)
{
  if (NULL == st) return;

  printf("SYMBOL_TABLE(%u):\n", st->size);

  for (u32 i = 0; i < st->size; i++)
    printf("  &%s = 0x%08X\n", st->symbols[i]->name, st->symbols[i]->addr);
}
