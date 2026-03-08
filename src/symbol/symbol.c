#include <symbol/symbol.h>
#include <stdlib.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct symbol *symbol_alloc(u0)
{
  struct symbol *s = malloc(sizeof(struct symbol));

  if (NULL == s)
    die(1, "malloc() failed in symbol_alloc()");

  return s;
}

struct symbol *symbol_create(const char *name, u32 addr)
{
  struct symbol *s = symbol_alloc();

  s->name = name;
  s->addr = addr;

  return s;
}

u0 symbol_free(struct symbol **s)
{
  if (NULL == s || NULL == *s) return;

  free(*s);
  *s = NULL;
}
