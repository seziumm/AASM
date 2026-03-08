#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <parser.h>
#include <common.h>
#include <type.h>

#define CODEGEN_PC_INIT_ADDR  0x80000000u
#define CODEGEN_RV32_NOP      0x00000013u  /* ADDI X0, X0, 0 */

#define codegen_die(cg, err, ...)  \
  do                               \
  {                                \
    /*codegen_print(cg);*/         \
    die(err, __VA_ARGS__);         \
  } while (0)

/* ============================================================
 *  Entry point
 * ============================================================ */

u0 codegen_compile(struct ast_node *root, const char *out_path);

#endif /* _CODEGEN_H */
