#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <ast/ast_node.h>
#include <utils/type.h>

/* ============================================================
 *  codegen_ctx  —  code generation context
 * ============================================================ */

#define CODEGEN_LABEL_MAX    64
#define CODEGEN_BUF_INIT_CAP 256

struct codegen_label
{
  const char *name;   /* "&NAME" exactly as stored in the AST label node */
  u32         addr;   /* resolved byte address assigned in pass 1        */
};

struct codegen_ctx
{
  u8  *buf;
  u32  buf_size;
  u32  buf_capacity;

  u32  pc;
  u32  base;

  struct codegen_label labels[CODEGEN_LABEL_MAX];
  u32                  labels_size;
};

struct codegen_ctx *codegen_run(struct ast_node *root, u32 base_addr);
u0 codegen_write(struct codegen_ctx *ctx, const char *path);
u0 codegen_print(struct codegen_ctx *ctx);
u0 codegen_free(struct codegen_ctx **ctx);

#endif /* _CODEGEN_H */
