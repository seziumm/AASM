#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <parser.h>
#include <common.h>
#include <type.h>

/* Prints codegen state then calls die().
   Used for unrecoverable codegen errors. */
#define codegen_die(cg, err, ...)  \
  do                               \
  {                                \
    codegen_print(cg);             \
    die(err, __VA_ARGS__);         \
  } while (0)

/* ============================================================
 *  Codegen state
 * ============================================================ */

struct codegen
{
  struct ast_node    *root;   /* AST root (PROGRAM node)  */
  u32                 pc;     /* current location counter */
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct codegen *codegen_build(struct ast_node *root);
struct codegen *codegen_alloc(u0);
struct codegen *codegen_create(struct ast_node *root);
u0              codegen_free(struct codegen **cg);

/* ============================================================
 *  Helpers
 * ============================================================ */

/* ============================================================
 *  Debug
 * ============================================================ */

u0 codegen_print(struct codegen *cg);

#endif
