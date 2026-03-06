#ifndef _SA_H
#define _SA_H

#include <ast.h>
#include <type.h>

#define sa_die(err, ...)         \
  do                             \
  {                              \
    die(err, __VA_ARGS__);       \
  } while (0)

/* ---- API ---- */
u0 sa_check(struct ast_node *node);

/* ---- internals (exposed for testing) ---- */
i32 sa_is_type  (struct ast_node *node, enum ast_node_type type);
u0  sa_check_program (struct ast_node *node, u32 *lc);
u0  sa_check_section (struct ast_node *node, u32 *lc);
u0  sa_check_label   (struct ast_node *node, u32 *lc);
u0  sa_check_instr   (struct ast_node *node);

#endif
