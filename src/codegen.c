#include <parser.h>
#include <codegen.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */


struct codegen *codegen_build(struct ast_node *root)
{
  if(NULL == root) 
  {
    codegen_die(NULL, 1, "codegen_build() failed because root is NULL");
  }

  struct codegen *cg = codegen_create(root);
  cg->pc = 0;

  return cg;
}

struct codegen *codegen_alloc(u0)
{
  struct codegen *cg = malloc(sizeof(struct codegen));

  if (NULL == cg)
  {
    codegen_die(NULL, 1, "malloc() failed in codegen_alloc()");
  }

  return cg;
}

struct codegen *codegen_create(struct ast_node *root)
{
  struct codegen *cg = codegen_alloc();

  *cg = (struct codegen)
  {
    .root  = root,
    .pc    = 0,
  };

  return cg;
}

u0 codegen_free(struct codegen **cg)
{
  if (NULL == cg || NULL == *cg) return;

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
  printf("AST:\n");
  ast_node_print(cg->root, 1);
}
