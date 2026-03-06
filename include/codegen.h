#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <ast.h>
#include <type.h>
#include <rv32/rv32ii.h>

#define codegen_die(err, ...)    \
  do                             \
  {                              \
    die(err, __VA_ARGS__);       \
  } while (0)

#define CODEGEN_MAX_LABELS 256

/* ---- label → address table ---- */
struct codegen_label_entry
{
  char *name;
  u32   addr;
};

struct codegen_ctx
{
  struct codegen_label_entry labels[CODEGEN_MAX_LABELS];
  u32   label_count;

  u32  *output;       /* array of encoded 32-bit words            */
  u32   output_size;  /* number of words written                  */
  u32   output_cap;   /* allocated capacity (words)               */

  u32   base_addr;    /* load address of the current section      */
};

/* ---- API ---- */
struct codegen_ctx *codegen_ctx_alloc (u0);
u0                  codegen_ctx_free  (struct codegen_ctx *ctx);

/* two-pass code generation */
u0  codegen_first_pass  (struct codegen_ctx *ctx, struct ast_node *program);
u0  codegen_second_pass (struct codegen_ctx *ctx, struct ast_node *program);

/* entry point: returns allocated word array (caller frees), sets *out_size */
u32 *codegen_compile(struct ast_node *program, u32 *out_size);

/* ---- output ---- */
u0 codegen_write_binary(const u32 *code, u32 size, const char *path);

/* ---- debug ---- */
u0 codegen_print(const u32 *code, u32 size, u32 base_addr);

#endif
