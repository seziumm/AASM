#include <codegen.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct codegen *codegen_alloc(u0)
{
  struct codegen *cg = malloc(sizeof(struct codegen));

  if (NULL == cg)
    die(1, "malloc() failed in codegen_alloc()");

  return cg;
}

struct codegen *codegen_create(struct ast_node *root)
{
  struct codegen *cg = codegen_alloc();

  struct symbol **symbols = malloc(SYMBOL_INIT_CAPACITY * sizeof(struct symbol *));

  if (NULL == symbols)
  {
    free(cg);
    die(1, "malloc() failed in codegen_create()");
  }

  *cg = (struct codegen)
  {
    .root  = root,
    .pc    = 0,
    .table = (struct symbol_table)
    {
      .symbols  = symbols,
      .size     = 0,
      .capacity = SYMBOL_INIT_CAPACITY,
    },
  };

  return cg;
}

u0 codegen_free(struct codegen **cg)
{
  if (NULL == cg || NULL == *cg) return;

  symbol_table_free_contents(&(*cg)->table);
  free(*cg);
  *cg = NULL;
}

/* ============================================================
 *  Debug
 * ============================================================ */

u0 codegen_print(struct codegen *cg)
{
  if (NULL == cg) return;

  printf("CODEGEN(pc=0x%08X)\n", cg->pc);
  symbol_table_print(&cg->table);
  printf("AST:\n");
  ast_node_print(cg->root, 1);
}
