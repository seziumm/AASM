#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <common.h>
#include <type.h>

/* ============================================================
 *  Symbol
 * ============================================================ */

struct symbol
{
  const char *name;   /* points into the AST — not owned */
  u32         addr;   /* absolute address assigned in pass 1 */
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct symbol *symbol_alloc(u0);
struct symbol *symbol_create(const char *name, u32 addr);
u0             symbol_free(struct symbol **s);

#endif /* _SYMBOL_H */
