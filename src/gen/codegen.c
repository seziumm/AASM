#include <gen/codegen.h>
#include <directive_utils.h>
#include <rv32/instr_utils.h>
#include <rv32/reg/reg_utils.h>
#include <token/token_array.h>
#include <token/token_type.h>
#include <utils/aalloc.h>
#include <utils/common.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================
 *  Emit  —  write one little-endian u32 to output file
 * ============================================================ */

static inline u0 codegen_emit(struct codegen_ctx *ctx, u32 word)
{
  fwrite(&word, sizeof(u32), 1, ctx->out);
  ctx->pc += 4;
}

/* ============================================================
 *  Symbol table helpers
 * ============================================================ */

static u0 codegen_label_push(struct codegen_ctx *ctx, const char *name, u32 addr)
{
  if (unlikely(ctx->labels_size >= CODEGEN_LABEL_MAX))
    die(1, "codegen: too many labels (max %d)", CODEGEN_LABEL_MAX);

  ctx->labels[ctx->labels_size].name = name;
  ctx->labels[ctx->labels_size].addr = addr;
  ctx->labels_size++;
}

static u32 codegen_label_addr(struct codegen_ctx *ctx, const char *name)
{
  const char *bare = (name[0] == '&' || name[0] == '@') ? name + 1 : name;

  for (u32 i = 0; i < ctx->labels_size; i++)
  {
    const char *stored = ctx->labels[i].name;
    if (stored[0] == '&' || stored[0] == '@') stored++;
    if (strcmp(bare, stored) == 0) return ctx->labels[i].addr;
  }

  die(1, "codegen: undefined label '%s'", name);
  return 0;
}

/* ============================================================
 *  Cursor helpers
 * ============================================================ */

static inline struct token *cg_peek(struct codegen_ctx *ctx, u32 pos)
{
  if (pos >= ctx->ta->size) return NULL;
  return ctx->ta->tokens[pos];
}

static inline i32 cg_is(struct codegen_ctx *ctx, u32 pos, enum token_type tt)
{
  struct token *t = cg_peek(ctx, pos);
  return t != NULL && t->type == tt;
}

static inline u0 cg_skip_comma(struct codegen_ctx *ctx, u32 *pos)
{
  if (cg_is(ctx, *pos, TOKEN_COMMA)) (*pos)++;
}

/* ============================================================
 *  Operand readers  —  all use cg_expect internally
 * ============================================================ */

static inline u8 cg_read_reg(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_REGISTER);
  return (u8)reg_from_label(t->value)->index;
}

static inline i32 cg_read_imm(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_expect(ctx, pos, TOKEN_NUMBER);
  return (i32)atoi(t->value);
}

/* imm(rs1) — used by loads and stores */
static u0 cg_read_mem(struct codegen_ctx *ctx, u32 *pos, i32 *imm, u8 *rs1)
{
  *imm = cg_read_imm(ctx, pos);
  cg_expect(ctx, pos, TOKEN_LPAREN);
  *rs1 = cg_read_reg(ctx, pos);
  cg_expect(ctx, pos, TOKEN_RPAREN);
}

/* immediate or pc-relative label ref */
static i32 cg_read_imm_or_ref(struct codegen_ctx *ctx, u32 *pos)
{
  struct token *t = cg_peek(ctx, *pos);

  if (unlikely(NULL == t))
    die(1, "codegen: unexpected end of tokens");

  if (t->type == TOKEN_NUMBER)
    return cg_read_imm(ctx, pos);

  if (t->type == TOKEN_LABEL_REF)
  {
    u32 addr = codegen_label_addr(ctx, t->value);
    (*pos)++;
    return (i32)(addr - ctx->pc);
  }

  die(1, "codegen: expected immediate or label ref, got %s (\"%s\")",
      token_type_to_str(t->type), t->value);
  return 0;
}

/* ============================================================
 *  Pass 1  —  simulate PC, record label addresses
 * ============================================================ */

static u32 instr_token_count(struct codegen_ctx *ctx, u32 pos)
{
  u32 count = 0;
  while (pos < ctx->ta->size)
  {
    enum token_type tt = ctx->ta->tokens[pos]->type;
    if (tt == TOKEN_INSTR || tt == TOKEN_LABEL || tt == TOKEN_DIRECTIVE) break;
    count++; pos++;
  }
  return count;
}

static u0 codegen_pass1(struct codegen_ctx *ctx)
{
  u32 pc  = ctx->base;
  u32 pos = 0;
  u32 n   = ctx->ta->size;

  while (pos < n)
  {
    struct token *t = ctx->ta->tokens[pos];

    switch (t->type)
    {
      case TOKEN_LABEL:
        codegen_label_push(ctx, t->value, pc);
        pos++;
        break;

      case TOKEN_INSTR:
        pos += 1 + instr_token_count(ctx, pos + 1);
        pc  += 4;
        break;

      case TOKEN_DIRECTIVE:
      {
        const struct directive *d = directive_from_label(t->value);
        pos++; /* consume directive token */
        directive_pass1(d, &pc, &pos, ctx->ta);
        break;
      }

      default:
        pos++;
        break;
    }
  }
}

/* ============================================================
 *  Pass 2  —  emit machine words and data
 * ============================================================ */

static u0 codegen_pass2(struct codegen_ctx *ctx)
{
  u32 pos = 0;
  u32 n   = ctx->ta->size;

  while (pos < n)
  {
    struct token *t = ctx->ta->tokens[pos];

    switch (t->type)
    {
      case TOKEN_LABEL:
        pos++;
        break;

      case TOKEN_DIRECTIVE:
      {
        const struct directive *d = directive_from_label(t->value);
        pos++; /* consume directive token */
        directive_build(d, ctx, &pos);
        break;
      }

      case TOKEN_INSTR:
      {
        const struct instr *ins = instr_from_label(t->value);
        pos++;

        u8  rd  = 0;
        u8  rs1 = 0;
        u8  rs2 = 0;
        i32 imm = 0;
        u32 word;

        switch (ins->type)
        {
          case FMT_R:
            rd  = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            rs1 = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            rs2 = cg_read_reg(ctx, &pos);
            word = fmt_r_encode(ins, rd, rs1, rs2);
            break;

          case FMT_I:
            rd = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            if (fmt_i_load_from_label(ins->label))
            {
              cg_read_mem(ctx, &pos, &imm, &rs1);
            }
            else
            {
              rs1 = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
              imm = cg_read_imm(ctx, &pos);
            }
            word = fmt_i_encode(ins, rd, rs1, (u16)(i16)imm);
            break;

          case FMT_S:
            rs2 = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            cg_read_mem(ctx, &pos, &imm, &rs1);
            word = fmt_s_encode(ins, rs1, rs2, (i16)imm);
            break;

          case FMT_B:
            rs1 = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            rs2 = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            imm = cg_read_imm_or_ref(ctx, &pos);
            word = fmt_b_encode(ins, rs1, rs2, (i16)imm);
            break;

          case FMT_U:
            rd  = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            imm = cg_read_imm(ctx, &pos);
            word = fmt_u_encode(ins, rd, (u32)imm);
            break;

          case FMT_J:
            rd  = cg_read_reg(ctx, &pos); cg_skip_comma(ctx, &pos);
            imm = cg_read_imm_or_ref(ctx, &pos);
            word = fmt_j_encode(ins, rd, imm);
            break;

          default:
            die(1, "codegen: unhandled format for '%s'", ins->label);
            word = 0;
            break;
        }

        codegen_emit(ctx, word);
        break;
      }

      default:
        pos++;
        break;
    }
  }
}

u0 codegen_run(struct token_array *ta, u32 base, const char *path)
{
  if (unlikely(NULL == ta || NULL == path))
    die(1, "codegen_run: NULL argument");

  FILE *out = fopen(path, "wb");
  if (unlikely(NULL == out))
    die(1, "codegen_run: cannot open '%s'", path);

  struct codegen_ctx ctx =
  {
    .ta          = ta,
    .base        = base,
    .pc          = base,
    .out         = out,
    .labels_size = 0,
  };

  codegen_pass1(&ctx);
  ctx.pc = base;
  codegen_pass2(&ctx);

  fclose(out);
}
