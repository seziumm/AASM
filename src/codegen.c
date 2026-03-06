#include <codegen.h>
#include <common.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ======================================================================
   CONTEXT MANAGEMENT
   ====================================================================== */

#define CODEGEN_INIT_CAPACITY 64

struct codegen_ctx *codegen_ctx_alloc(u0)
{
  struct codegen_ctx *ctx = calloc(1, sizeof(struct codegen_ctx));
  if (ctx == NULL)
    codegen_die(1, "codegen: calloc() failed for ctx");

  ctx->output_cap = CODEGEN_INIT_CAPACITY;
  ctx->output     = malloc(ctx->output_cap * sizeof(u32));
  if (ctx->output == NULL)
    codegen_die(1, "codegen: malloc() failed for output buffer");

  return ctx;
}

u0 codegen_ctx_free(struct codegen_ctx *ctx)
{
  if (ctx == NULL) return;
  free(ctx->output);
  free(ctx);
}

static u0 codegen_emit(struct codegen_ctx *ctx, u32 word)
{
  if (ctx->output_size >= ctx->output_cap)
  {
    ctx->output_cap *= 2;
    u32 *nb = realloc(ctx->output, ctx->output_cap * sizeof(u32));
    if (nb == NULL)
      codegen_die(1, "codegen: realloc() failed expanding output buffer");
    ctx->output = nb;
  }
  ctx->output[ctx->output_size++] = word;
}

/* ======================================================================
   LABEL TABLE
   ====================================================================== */

static u0 codegen_label_define(struct codegen_ctx *ctx,
                               const char *name, u32 addr)
{
  if (ctx->label_count >= CODEGEN_MAX_LABELS)
    codegen_die(1, "codegen: too many labels (max %u)", CODEGEN_MAX_LABELS);

  /* skip leading '&' sigil if present */
  const char *clean = (name[0] == '&') ? name + 1 : name;

  for (u32 i = 0; i < ctx->label_count; i++)
  {
    if (strcmp(ctx->labels[i].name, clean) == 0)
      codegen_die(1, "codegen: duplicate label '%s'", clean);
  }

  ctx->labels[ctx->label_count].name = strdup(clean);
  ctx->labels[ctx->label_count].addr = addr;
  ctx->label_count++;
}

static u32 codegen_label_resolve(struct codegen_ctx *ctx, const char *name)
{
  /* skip leading '!' sigil if present */
  const char *clean = (name[0] == '!') ? name + 1 : name;

  for (u32 i = 0; i < ctx->label_count; i++)
  {
    if (strcmp(ctx->labels[i].name, clean) == 0)
      return ctx->labels[i].addr;
  }
  codegen_die(1, "codegen: undefined label '%s'", clean);
  return 0; /* unreachable */
}

/* ======================================================================
   REGISTER NUMBER
   Maps ABI name or Xn name to register index 0-31.
   ====================================================================== */

static u32 codegen_reg_num(const char *name)
{
  static const char *abi[] = {
    "ZERO","RA","SP","GP","TP",
    "T0","T1","T2",
    "S0","S1",
    "A0","A1","A2","A3","A4","A5","A6","A7",
    "S2","S3","S4","S5","S6","S7","S8","S9","S10","S11",
    "T3","T4","T5","T6"
  };
  /* ABI ordering matches x0-x31 exactly */
  for (u32 i = 0; i < 32; i++)
  {
    if (strcmp(name, abi[i]) == 0) return i;
  }

  /* X-form: X0 … X31 */
  if (name[0] == 'X')
  {
    char *end;
    u32 n = (u32)strtoul(name + 1, &end, 10);
    if (*end == '\0' && n < 32) return n;
  }

  codegen_die(1, "codegen: unknown register '%s'", name);
  return 0; /* unreachable */
}

/* ======================================================================
   FIRST PASS  — collect label addresses
   ====================================================================== */

u0 codegen_first_pass(struct codegen_ctx *ctx, struct ast_node *program)
{
  if (program == NULL || program->type != AST_PROGRAM)
    codegen_die(1, "codegen first-pass: expected AST_PROGRAM");

  for (u32 si = 0; si < program->children_size; si++)
  {
    struct ast_node *section = program->children[si];
    if (section->type != AST_SECTION)
      codegen_die(1, "codegen first-pass: expected AST_SECTION");

    u32 lc = (u32)strtoul(section->data.section.value, NULL, 0);

    for (u32 li = 0; li < section->children_size; li++)
    {
      struct ast_node *label = section->children[li];
      if (label->type != AST_LABEL)
        codegen_die(1, "codegen first-pass: expected AST_LABEL");

      codegen_label_define(ctx, label->data.label.name, lc);

      /* advance lc by 4 bytes per instruction */
      lc += label->children_size * 4;
    }
  }
}

/* ======================================================================
   SECOND PASS  — encode instructions
   ====================================================================== */

static u32 codegen_encode_instr(struct codegen_ctx *ctx,
                                struct ast_node    *instr,
                                u32                 pc)
{
  const char *mn = instr->data.instruction.mnemonic;
  const struct rv32ii_opcode_entry_t *e = rv32ii_instr_from_label(mn);

  if (e == NULL)
    codegen_die(1, "codegen: unknown mnemonic '%s'", mn);

  struct ast_node **op = instr->children;

  switch (e->type)
  {
    /* ---- R-type:  rd, rs1, rs2 ---- */
    case R_TYPE:
    {
      u32 rd  = codegen_reg_num(op[0]->data.op_register.reg);
      u32 rs1 = codegen_reg_num(op[1]->data.op_register.reg);
      u32 rs2 = codegen_reg_num(op[2]->data.op_register.reg);
      return RV32_ENC_R(e->fields.r.funct7,
                        e->fields.r.funct3,
                        e->fields.r.opcode,
                        rd, rs1, rs2);
    }

    /* ---- I-type ---- */
    case I_TYPE:
    {
      /* ECALL / EBREAK: zero operands, imm encodes the function */
      if (e->index == RV32I_ECALL || e->index == RV32I_EBREAK)
        return RV32_ENC_I(e->fields.i.funct3,
                          e->fields.i.opcode,
                          0, 0, e->fields.i.imm);

      /* shift-immediate (SLLI / SRLI / SRAI):  rd, rs1, shamt
         imm[11:5] = funct7 from table, imm[4:0] = shamt from AST */
      if (e->index == RV32I_SLLI ||
          e->index == RV32I_SRLI ||
          e->index == RV32I_SRAI)
      {
        u32 rd    = codegen_reg_num(op[0]->data.op_register.reg);
        u32 rs1   = codegen_reg_num(op[1]->data.op_register.reg);
        i32 shamt = op[2]->data.op_immediate.value;
        /* funct7 sits in imm[11:5]; shamt in imm[4:0] */
        i32 imm   = ((i32)e->fields.i.imm << 5) | (shamt & 0x1F);
        return RV32_ENC_I(e->fields.i.funct3,
                          e->fields.i.opcode,
                          rd, rs1, (u32)imm);
      }

      /* Load / JALR:  rd, disp(base) */
      if (e->index == RV32I_LB  || e->index == RV32I_LH  ||
          e->index == RV32I_LW  || e->index == RV32I_LBU ||
          e->index == RV32I_LHU || e->index == RV32I_JALR)
      {
        u32 rd  = codegen_reg_num(op[0]->data.op_register.reg);
        u32 rs1 = codegen_reg_num(op[1]->data.op_memory.base);
        i32 imm = op[1]->data.op_memory.displacement;
        return RV32_ENC_I(e->fields.i.funct3,
                          e->fields.i.opcode,
                          rd, rs1, (u32)imm);
      }

      /* arithmetic immediate:  rd, rs1, imm12 */
      {
        u32 rd  = codegen_reg_num(op[0]->data.op_register.reg);
        u32 rs1 = codegen_reg_num(op[1]->data.op_register.reg);
        i32 imm = op[2]->data.op_immediate.value;
        return RV32_ENC_I(e->fields.i.funct3,
                          e->fields.i.opcode,
                          rd, rs1, (u32)imm);
      }
    }

    /* ---- S-type:  rs2, disp(rs1) ---- */
    case S_TYPE:
    {
      u32 rs2 = codegen_reg_num(op[0]->data.op_register.reg);
      u32 rs1 = codegen_reg_num(op[1]->data.op_memory.base);
      i32 imm = op[1]->data.op_memory.displacement;
      return RV32_ENC_S(e->fields.s.funct3,
                        e->fields.s.opcode,
                        rs1, rs2, (u32)imm);
    }

    /* ---- B-type:  rs1, rs2, label_ref ---- */
    case B_TYPE:
    {
      u32 rs1 = codegen_reg_num(op[0]->data.op_register.reg);
      u32 rs2 = codegen_reg_num(op[1]->data.op_register.reg);
      u32 tgt = codegen_label_resolve(ctx, op[2]->data.op_label_ref.name);
      i32 imm = (i32)(tgt - pc);
      return RV32_ENC_B(e->fields.b.funct3,
                        e->fields.b.opcode,
                        rs1, rs2, (u32)imm);
    }

    /* ---- U-type:  rd, imm20
         The assembler convention: the user writes the upper-20-bit value
         (e.g. LUI X28, 65536 means place 65536 in bits[31:12]).
         RV32_ENC_U keeps imm[31:12] as-is, so we must shift left by 12. ---- */
    case U_TYPE:
    {
      u32 rd  = codegen_reg_num(op[0]->data.op_register.reg);
      u32 imm = (u32)op[1]->data.op_immediate.value << 12;
      return RV32_ENC_U(e->fields.u.opcode, rd, imm);
    }

    /* ---- J-type:  rd, label_ref ---- */
    case J_TYPE:
    {
      u32 rd  = codegen_reg_num(op[0]->data.op_register.reg);
      u32 tgt = codegen_label_resolve(ctx, op[1]->data.op_label_ref.name);
      i32 imm = (i32)(tgt - pc);
      return RV32_ENC_J(e->fields.j.opcode, rd, (u32)imm);
    }

    default:
      codegen_die(1, "codegen: unhandled instruction type for '%s'", mn);
  }

  return 0; /* unreachable */
}

u0 codegen_second_pass(struct codegen_ctx *ctx, struct ast_node *program)
{
  if (program == NULL || program->type != AST_PROGRAM)
    codegen_die(1, "codegen second-pass: expected AST_PROGRAM");

  for (u32 si = 0; si < program->children_size; si++)
  {
    struct ast_node *section = program->children[si];
    u32 lc = (u32)strtoul(section->data.section.value, NULL, 0);

    /* record where this section starts in the output buffer */
    if (si == 0) ctx->base_addr = lc;

    for (u32 li = 0; li < section->children_size; li++)
    {
      struct ast_node *label = section->children[li];

      for (u32 ii = 0; ii < label->children_size; ii++)
      {
        u32 word = codegen_encode_instr(ctx, label->children[ii], lc);
        codegen_emit(ctx, word);
        lc += 4;
      }
    }
  }
}

/* ======================================================================
   ENTRY POINT
   ====================================================================== */

u32 *codegen_compile(struct ast_node *program, u32 *out_size)
{
  struct codegen_ctx *ctx = codegen_ctx_alloc();

  codegen_first_pass (ctx, program);
  codegen_second_pass(ctx, program);

  /* transfer ownership of the output buffer to the caller */
  u32 *result  = ctx->output;
  *out_size    = ctx->output_size;
  ctx->output  = NULL; /* prevent free inside codegen_ctx_free */

  codegen_ctx_free(ctx);
  return result;
}

/* ======================================================================
   BINARY OUTPUT
   ====================================================================== */

u0 codegen_write_binary(const u32 *code, u32 size, const char *path)
{
  FILE *f = fopen(path, "wb");
  if (f == NULL)
    codegen_die(1, "codegen: cannot open '%s' for writing", path);

  size_t written = fwrite(code, sizeof(u32), size, f);
  if (written != size)
  {
    fclose(f);
    codegen_die(1, "codegen: fwrite() failed (%zu of %u words written)",
                written, size);
  }

  fclose(f);
}

/* ======================================================================
   DEBUG PRINT
   ====================================================================== */

u0 codegen_print(const u32 *code, u32 size, u32 base_addr)
{
  if (code == NULL) return;

  printf("%-10s  %-10s  %s\n", "ADDR", "HEX", "BINARY");
  printf("%-10s  %-10s  %s\n",
         "----------", "----------",
         "--------------------------------");

  for (u32 i = 0; i < size; i++)
  {
    u32 addr = base_addr + i * 4;
    u32 word = code[i];

    printf("0x%08X  0x%08X  ", addr, word);
    for (i32 bit = 31; bit >= 0; bit--)
    {
      putchar((word >> bit) & 1 ? '1' : '0');
      if (bit == 12 || bit == 7 || bit == 20 || bit == 25)
        putchar('_');
    }
    putchar('\n');
  }
}
