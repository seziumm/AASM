/* codegen.c — two-pass code generator for AASM
 *
 * Pass 1: walk the AST, record {name, addr} for every AST_LABEL node.
 * Pass 2: walk again, emit machine words.  For every AST_LABEL_REF
 *         look up the target by name in the label table — no cursor.
 */

#include <codegen.h>

#include <ast/ast_node.h>
#include <ast/ast_node_type.h>
#include <directive_look_up.h>
#include <rv32/instr.h>
#include <rv32/fmt/fmt_type.h>
#include <rv32/fmt/r/fmt_r_look_up.h>
#include <rv32/fmt/i/fmt_i_look_up.h>
#include <rv32/fmt/s/fmt_s_look_up.h>
#include <rv32/fmt/b/fmt_b_look_up.h>
#include <rv32/fmt/u/fmt_u_look_up.h>
#include <rv32/fmt/j/fmt_j_look_up.h>
#include <utils/aalloc.h>
#include <utils/common.h>

#include <stdio.h>
#include <string.h>

/* ============================================================
 *  Context helpers
 * ============================================================ */

static struct codegen_ctx *codegen_ctx_create(u32 base_addr)
{
  struct codegen_ctx *ctx = a_malloc(sizeof(struct codegen_ctx));

  *ctx = (struct codegen_ctx)
  {
    .buf          = a_malloc(CODEGEN_BUF_INIT_CAP),
    .buf_size     = 0,
    .buf_capacity = CODEGEN_BUF_INIT_CAP,
    .pc           = base_addr,
    .base         = base_addr,
    .labels_size  = 0,
  };

  return ctx;
}

/* ============================================================
 *  emit()
 * ============================================================ */

static u0 emit(struct codegen_ctx *ctx, u32 word)
{
  if (ctx->buf_size + 4 > ctx->buf_capacity)
  {
    ctx->buf_capacity *= 2;
    ctx->buf = a_realloc(ctx->buf, ctx->buf_capacity);
  }

  ctx->buf[ctx->buf_size + 0] = (u8)((word >>  0) & 0xFF);
  ctx->buf[ctx->buf_size + 1] = (u8)((word >>  8) & 0xFF);
  ctx->buf[ctx->buf_size + 2] = (u8)((word >> 16) & 0xFF);
  ctx->buf[ctx->buf_size + 3] = (u8)((word >> 24) & 0xFF);

  ctx->buf_size += 4;
  ctx->pc       += 4;
}

/* ============================================================
 *  emit_data()
 * ============================================================ */

static u0 emit_data(struct codegen_ctx *ctx, struct ast_node *dir_node)
{
  expect(dir_node->children_size == 1);
  expect(dir_node->children[0]->type == AST_IMM);

  i32        val    = dir_node->children[0]->as_imm.value;
  const char *label = dir_node->as_directive.dir->label;

  u32 nbytes = 1;
  if      (strcmp(label, ".2BYTE") == 0) nbytes = 2;
  else if (strcmp(label, ".4BYTE") == 0) nbytes = 4;
  else if (strcmp(label, ".8BYTE") == 0) nbytes = 8;

  while (ctx->buf_size + nbytes > ctx->buf_capacity)
  {
    ctx->buf_capacity *= 2;
    ctx->buf = a_realloc(ctx->buf, ctx->buf_capacity);
  }

  for (u32 b = 0; b < nbytes; b++)
    ctx->buf[ctx->buf_size++] = (u8)((val >> (b * 8)) & 0xFF);

  ctx->pc += nbytes;
}

/* ============================================================
 *  Label table — keyed by name
 *
 *  Label token values are "&NAME"; label-ref token values are "@NAME".
 *  We normalise both to the bare NAME (strip the sigil) before
 *  inserting / looking up, so "&LOOP" and "@LOOP" both map to "LOOP".
 * ============================================================ */

static inline const char *strip_sigil(const char *s)
{
  /* skip leading '&' or '@' */
  if (s && (*s == '&' || *s == '@'))
    return s + 1;
  return s;
}

static u0 codegen_label_push(struct codegen_ctx *ctx,
                              const char *name, u32 addr)
{
  if (ctx->labels_size >= CODEGEN_LABEL_MAX)
    die(1, "codegen: too many labels (max %d)", CODEGEN_LABEL_MAX);

  ctx->labels[ctx->labels_size].name = strip_sigil(name);
  ctx->labels[ctx->labels_size].addr = addr;
  ctx->labels_size++;
}

/* Returns the address of the label named 'name' (with or without sigil).
 * Calls die() if not found. */
static u32 codegen_label_addr(struct codegen_ctx *ctx, const char *name)
{
  const char *bare = strip_sigil(name);

  for (u32 i = 0; i < ctx->labels_size; i++)
  {
    if (strcmp(ctx->labels[i].name, bare) == 0)
      return ctx->labels[i].addr;
  }

  die(1, "codegen: undefined label '%s'", name);
  return 0;
}

/* ============================================================
 *  Operand accessors
 * ============================================================ */

static inline u8 child_reg(struct ast_node *n, u32 idx)
{
  expect(idx < n->children_size);
  expect(n->children[idx]->type == AST_REG);
  return n->children[idx]->as_reg.reg;
}

static inline i32 child_imm(struct ast_node *n, u32 idx)
{
  expect(idx < n->children_size);
  expect(n->children[idx]->type == AST_IMM);
  return n->children[idx]->as_imm.value;
}

/* Resolve a label-ref child by name and return its address. */
static inline u32 child_label_ref(struct ast_node    *n,
                                   u32                 idx,
                                   struct codegen_ctx *ctx)
{
  expect(idx < n->children_size);
  expect(n->children[idx]->type == AST_LABEL_REF);
  return codegen_label_addr(ctx, n->children[idx]->as_label_ref.name);
}

/* ============================================================
 *  Pass 1 — simulate PC, record label addresses
 * ============================================================ */

static u0 pass1_node(struct codegen_ctx *ctx, struct ast_node *n)
{
  if (NULL == n) return;

  switch (n->type)
  {
    case AST_ROOT:
      for (u32 i = 0; i < n->children_size; i++)
        pass1_node(ctx, n->children[i]);
      break;

    case AST_DIRECTIVE:
      if (n->as_directive.dir->type == DIRECTIVE_SECTION)
      {
        for (u32 i = 0; i < n->children_size; i++)
          pass1_node(ctx, n->children[i]);
      }
      else /* DIRECTIVE_DATA */
      {
        const char *lbl = n->as_directive.dir->label;
        u32 nb = 1;
        if      (strcmp(lbl, ".2BYTE") == 0) nb = 2;
        else if (strcmp(lbl, ".4BYTE") == 0) nb = 4;
        else if (strcmp(lbl, ".8BYTE") == 0) nb = 8;
        ctx->pc += nb;
      }
      break;

    case AST_LABEL:
      n->as_label.addr = ctx->pc;
      codegen_label_push(ctx, n->as_label.name, ctx->pc);

      for (u32 i = 0; i < n->children_size; i++)
        pass1_node(ctx, n->children[i]);
      break;

    case AST_INSTR:
      ctx->pc += 4;
      break;

    case AST_REG:
    case AST_IMM:
    case AST_LABEL_REF:
      break;
  }
}

/* ============================================================
 *  Pass 2 — emit machine code
 * ============================================================ */

static u0 codegen_emit_instr(struct codegen_ctx *ctx, struct ast_node *node)
{
  const struct instr *ins = node->as_instr.inst;

  switch (ins->type)
  {
    case FMT_R:
    {
      u8 rd  = child_reg(node, 0);
      u8 rs1 = child_reg(node, 1);
      u8 rs2 = child_reg(node, 2);
      emit(ctx, fmt_r_encode((struct instr *)ins, rd, rs1, rs2));
      break;
    }

    case FMT_I:
    {
      if (fmt_i_look_up_load(ins->label))
      {
        u8  rd  = child_reg(node, 0);
        i32 imm = child_imm(node, 1);
        u8  rs1 = child_reg(node, 2);
        emit(ctx, fmt_i_encode((struct instr *)ins, rd, rs1, (u16)(i16)imm));
      }
      else
      {
        u8  rd  = child_reg(node, 0);
        u8  rs1 = child_reg(node, 1);
        i32 imm = child_imm(node, 2);
        emit(ctx, fmt_i_encode((struct instr *)ins, rd, rs1, (u16)(i16)imm));
      }
      break;
    }

    case FMT_S:
    {
      u8  rs2 = child_reg(node, 0);
      i32 imm = child_imm(node, 1);
      u8  rs1 = child_reg(node, 2);
      emit(ctx, fmt_s_encode((struct instr *)ins, rs1, rs2, (i16)imm));
      break;
    }

    case FMT_B:
    {
      u8 rs1 = child_reg(node, 0);
      u8 rs2 = child_reg(node, 1);

      i32 offset;
      if (node->children[2]->type == AST_LABEL_REF)
        offset = (i32)(child_label_ref(node, 2, ctx) - ctx->pc);
      else
        offset = child_imm(node, 2);

      emit(ctx, fmt_b_encode((struct instr *)ins, rs1, rs2, (i16)offset));
      break;
    }

    case FMT_U:
    {
      u8  rd  = child_reg(node, 0);
      u32 imm = (u32)child_imm(node, 1);
      emit(ctx, fmt_u_encode((struct instr *)ins, rd, imm));
      break;
    }

    case FMT_J:
    {
      u8 rd = child_reg(node, 0);

      i32 offset;
      if (node->children[1]->type == AST_LABEL_REF)
        offset = (i32)(child_label_ref(node, 1, ctx) - ctx->pc);
      else
        offset = child_imm(node, 1);

      emit(ctx, fmt_j_encode((struct instr *)ins, rd, offset));
      break;
    }
  }
}

static u0 pass2_node(struct codegen_ctx *ctx, struct ast_node *n)
{
  if (NULL == n) return;

  switch (n->type)
  {
    case AST_ROOT:
      for (u32 i = 0; i < n->children_size; i++)
        pass2_node(ctx, n->children[i]);
      break;

    case AST_DIRECTIVE:
      if (n->as_directive.dir->type == DIRECTIVE_SECTION)
        for (u32 i = 0; i < n->children_size; i++)
          pass2_node(ctx, n->children[i]);
      else
        emit_data(ctx, n);
      break;

    case AST_LABEL:
      for (u32 i = 0; i < n->children_size; i++)
        pass2_node(ctx, n->children[i]);
      break;

    case AST_INSTR:
      codegen_emit_instr(ctx, n);
      break;

    case AST_REG:
    case AST_IMM:
    case AST_LABEL_REF:
      break;
  }
}

/* ============================================================
 *  Public API
 * ============================================================ */

struct codegen_ctx *codegen_run(struct ast_node *root, u32 base_addr)
{
  if (NULL == root) return NULL;

  struct codegen_ctx *ctx = codegen_ctx_create(base_addr);

  debugf("codegen: pass 1 — label resolution\n");
  pass1_node(ctx, root);

  debugf("codegen: resolved %u label(s)\n", ctx->labels_size);
  for (u32 i = 0; i < ctx->labels_size; i++)
    debugf("  label[%u] \"%s\" = 0x%08X\n",
           i, ctx->labels[i].name, ctx->labels[i].addr);

  debugf("codegen: pass 2 — emission\n");
  ctx->pc = base_addr;

  pass2_node(ctx, root);

  debugf("codegen: emitted %u byte(s)\n", ctx->buf_size);
  return ctx;
}

u0 codegen_write(struct codegen_ctx *ctx, const char *path)
{
  if (NULL == ctx || NULL == path) return;

  FILE *f = fopen(path, "wb");
  if (unlikely(NULL == f))
    die(1, "codegen_write: fopen(%s) failed", path);

  size_t written = fwrite(ctx->buf, 1, ctx->buf_size, f);
  if (unlikely(written != ctx->buf_size))
    die(1, "codegen_write: fwrite failed (%zu / %u bytes)", written, ctx->buf_size);

  fclose(f);
}

u0 codegen_print(struct codegen_ctx *ctx)
{
  if (NULL == ctx) return;

  u32 addr = ctx->base;

  for (u32 i = 0; i + 3 < ctx->buf_size; i += 4)
  {
    u32 word = (u32)ctx->buf[i + 0]
             | (u32)ctx->buf[i + 1] <<  8
             | (u32)ctx->buf[i + 2] << 16
             | (u32)ctx->buf[i + 3] << 24;

    debugf("0x%08X:  %02X %02X %02X %02X  |  0x%08X\n",
        addr,
        ctx->buf[i+0], ctx->buf[i+1], ctx->buf[i+2], ctx->buf[i+3],
        word);

    addr += 4;
  }

  u32 rem = ctx->buf_size & ~3u;
  if (rem < ctx->buf_size)
  {
    debugf("0x%08X:", addr);
    for (u32 i = rem; i < ctx->buf_size; i++)
      debugf(" %02X", ctx->buf[i]);
    debugf("\n");
  }
}

u0 codegen_free(struct codegen_ctx **ctx)
{
  if (NULL == ctx || NULL == *ctx) return;
  a_free((*ctx)->buf);
  a_free(*ctx);
  *ctx = NULL;
}
