#include <lc.h>
#include <common.h>
#include <rv32/rv32_type.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct lc_label
{
  char *name;
  u32 addr;
};

struct lc_ctx
{
  struct lexer *lexer;

  struct lc_label *labels;
  u32 label_size;
  u32 label_cap;

  u32 base_addr;
};

static void lc_labels_init(struct lc_ctx *ctx)
{
  ctx->labels = NULL;
  ctx->label_size = 0;
  ctx->label_cap = 0;
}

static void lc_labels_push(struct lc_ctx *ctx, const char *name, u32 addr)
{
  const char *norm = name;
  if (norm[0] == '&')
  {
    norm++;
  }

  if (ctx->label_size >= ctx->label_cap)
  {
    u32 new_cap = ctx->label_cap == 0 ? 8 : ctx->label_cap * 2;
    struct lc_label *new_arr = realloc(ctx->labels, new_cap * sizeof *new_arr);
    if (NULL == new_arr)
    {
      die(1, "MALLOC LABEL TABLE");
    }
    ctx->labels = new_arr;
    ctx->label_cap = new_cap;
  }

  ctx->labels[ctx->label_size].name = strdup(norm);
  ctx->labels[ctx->label_size].addr = addr;
  ctx->label_size++;
}

static const struct lc_label *lc_labels_find(struct lc_ctx *ctx, const char *name)
{
  const char *norm = name;
  if (norm[0] == '&')
  {
    norm++;
  }

  for (u32 i = 0; i < ctx->label_size; i++)
  {
    if (strcmp(ctx->labels[i].name, norm) == 0) return &ctx->labels[i];
  }
  return NULL;
}

static void lc_labels_free(struct lc_ctx *ctx)
{
  for (u32 i = 0; i < ctx->label_size; i++)
  {
    free(ctx->labels[i].name);
  }
  free(ctx->labels);
  ctx->labels = NULL;
  ctx->label_size = 0;
  ctx->label_cap = 0;
}

static i32 lc_reg_index(const char *p)
{
  /* ABI names in ordine di x0..x31 */
  static const char *abi[] =
  {
    "ZERO","RA","SP","GP","TP",      /* x0..x4  */
    "T0","T1","T2",                  /* x5..x7  */
    "S0","S1",                       /* x8..x9  */
    "A0","A1","A2","A3","A4","A5","A6","A7", /* x10..x17 */
    "S2","S3","S4","S5","S6","S7","S8","S9","S10","S11", /* x18..x27 */
    "T3","T4","T5","T6"              /* x28..x31 */
  };

  for (u32 i = 0; i < sizeof(abi)/sizeof(*abi); i++)
  {
    if (strcmp(p, abi[i]) == 0) return (i32)i;
  }

  if (p[0] == 'X')
  {
    char *endptr = NULL;
    long v = strtol(p + 1, &endptr, 10);
    if (*endptr == '\0' && v >= 0 && v <= 31) return (i32)v;
  }

  die(1, "BAD REGISTER %s", p);
  return -1;
}

static i32 lc_parse_imm(const char *p)
{
  char *endptr = NULL;
  long v = strtol(p, &endptr, 0);
  if (*endptr != '\0')
  {
    die(1, "BAD IMMEDIATE %s", p);
  }
  return (i32)v;
}

static u0 lc_first_pass(struct lc_ctx *ctx)
{
  u32 pc = 0;
  ctx->base_addr = 0;

  for (u32 i = 0; i < ctx->lexer->size; i++)
  {
    struct token_data *td = ctx->lexer->tokens[i];

    if (td->type == token_section && strcmp(td->value, ".ADDR") == 0)
    {
      if (i + 1 >= ctx->lexer->size || ctx->lexer->tokens[i+1]->type != token_number)
      {
        die(1, "EXPECTED NUMBER AFTER .ADDR");
      }
      pc = (u32)lc_parse_imm(ctx->lexer->tokens[i+1]->value);
      if (ctx->base_addr == 0)
      {
        ctx->base_addr = pc;
      }
      i++;
    }
    else if (td->type == token_tag)
    {
      if (td->value[0] == '&')
      {
        lc_labels_push(ctx, td->value + 1, pc);
      }
    }
    else if (td->type == token_instr)
    {
      pc += 4;
    }
  }
}

static u32 lc_encode_add(u32 rd, u32 rs1, u32 rs2)
{
  return INSTR_R_ENCODE(0x00, 0x0, 0x33, rd, rs1, rs2);
}

static u32 lc_encode_addi(u32 rd, u32 rs1, i32 imm)
{
  return INSTR_I_ENCODE(0x0, 0x13, rd, rs1, (u32)imm);
}

static u32 lc_encode_sw(u32 rs2, u32 rs1, i32 imm)
{
  return INSTR_S_ENCODE(0x2, 0x23, rs1, rs2, (u32)imm);
}

static u32 lc_encode_lw(u32 rd, u32 rs1, i32 imm)
{
  return INSTR_I_ENCODE(0x2, 0x03, rd, rs1, (u32)imm);
}

static u32 lc_encode_bne(u32 rs1, u32 rs2, i32 imm)
{
  return INSTR_B_ENCODE(0x1, 0x63, rs1, rs2, (u32)imm);
}

static u32 lc_encode_jalr(u32 rd, u32 rs1, i32 imm)
{
  return INSTR_I_ENCODE(0x0, 0x67, rd, rs1, (u32)imm);
}

struct lc_buf
{
  u32 *code;
  u32 size;
  u32 cap;
};

static void lc_buf_init(struct lc_buf *b)
{
  b->code = NULL;
  b->size = 0;
  b->cap  = 0;
}

static void lc_buf_push(struct lc_buf *b, u32 v)
{
  if (b->size >= b->cap)
  {
    u32 new_cap = b->cap == 0 ? 16 : b->cap * 2;
    u32 *new_code = realloc(b->code, new_cap * sizeof *new_code);
    if (NULL == new_code)
    {
      die(1, "MALLOC CODE BUFFER");
    }
    b->code = new_code;
    b->cap  = new_cap;
  }
  b->code[b->size++] = v;
}

static void lc_buf_free(struct lc_buf *b)
{
  free(b->code);
  b->code = NULL;
  b->size = 0;
  b->cap  = 0;
}

/* ELF32 header strutture minime per RISC-V */

typedef struct
{
  u8  e_ident[16];
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  u32 e_entry;
  u32 e_phoff;
  u32 e_shoff;
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize;
  u16 e_phnum;
  u16 e_shentsize;
  u16 e_shnum;
  u16 e_shstrndx;
} Elf32_Ehdr_AASM;

typedef struct
{
  u32 p_type;
  u32 p_offset;
  u32 p_vaddr;
  u32 p_paddr;
  u32 p_filesz;
  u32 p_memsz;
  u32 p_flags;
  u32 p_align;
} Elf32_Phdr_AASM;

static u0 lc_write_elf(struct lc_buf *b, const char *output_path, u32 base_addr)
{
  FILE *f = fopen(output_path, "wb");
  if (NULL == f)
  {
    die(1, "CANNOT OPEN OUTPUT %s", output_path);
  }

  Elf32_Ehdr_AASM eh;
  memset(&eh, 0, sizeof eh);

  eh.e_ident[0] = 0x7f;
  eh.e_ident[1] = 'E';
  eh.e_ident[2] = 'L';
  eh.e_ident[3] = 'F';
  eh.e_ident[4] = 1; /* 32-bit */
  eh.e_ident[5] = 1; /* little-endian */
  eh.e_ident[6] = 1; /* version */

  eh.e_type      = 2;   /* ET_EXEC */
  eh.e_machine   = 243; /* EM_RISCV */
  eh.e_version   = 1;
  eh.e_entry     = base_addr;
  eh.e_phoff     = sizeof(Elf32_Ehdr_AASM);
  eh.e_shoff     = 0;
  eh.e_flags     = 0;
  eh.e_ehsize    = sizeof(Elf32_Ehdr_AASM);
  eh.e_phentsize = sizeof(Elf32_Phdr_AASM);
  eh.e_phnum     = 1;
  eh.e_shentsize = 0;
  eh.e_shnum     = 0;
  eh.e_shstrndx  = 0;

  Elf32_Phdr_AASM ph;
  memset(&ph, 0, sizeof ph);

  ph.p_type   = 1; /* PT_LOAD */
  ph.p_offset = sizeof(Elf32_Ehdr_AASM) + sizeof(Elf32_Phdr_AASM);
  ph.p_vaddr  = base_addr;
  ph.p_paddr  = base_addr;
  ph.p_filesz = b->size * sizeof(u32);
  ph.p_memsz  = b->size * sizeof(u32);
  ph.p_flags  = 0x5; /* R + X */
  ph.p_align  = 4;

  if (fwrite(&eh, sizeof eh, 1, f) != 1)
  {
    fclose(f);
    die(1, "WRITE ELF HEADER ERROR");
  }

  if (fwrite(&ph, sizeof ph, 1, f) != 1)
  {
    fclose(f);
    die(1, "WRITE ELF PHDR ERROR");
  }

  if (b->size > 0)
  {
    if (fwrite(b->code, sizeof(u32), b->size, f) != b->size)
    {
      fclose(f);
      die(1, "WRITE ELF CODE ERROR");
    }
  }

  fclose(f);
}

static u0 lc_second_pass(struct lc_ctx *ctx, const char *output_path)
{
  struct lc_buf buf;
  lc_buf_init(&buf);

  u32 pc = ctx->base_addr;

  for (u32 i = 0; i < ctx->lexer->size; i++)
  {
    struct token_data *td = ctx->lexer->tokens[i];

    if (td->type == token_section && strcmp(td->value, ".ADDR") == 0)
    {
      if (i + 1 >= ctx->lexer->size || ctx->lexer->tokens[i+1]->type != token_number)
      {
        die(1, "EXPECTED NUMBER AFTER .ADDR");
      }
      pc = (u32)lc_parse_imm(ctx->lexer->tokens[i+1]->value);
      i++;
      continue;
    }

    if (td->type == token_tag)
    {
      continue;
    }

    if (td->type != token_instr)
    {
      continue;
    }

    const char *mnemo = td->value;
    u32 code = 0;

    if (strcmp(mnemo, "ADD") == 0)
    {
      struct token_data *td_rd  = ctx->lexer->tokens[i+1];
      struct token_data *td_c1  = ctx->lexer->tokens[i+2];
      struct token_data *td_rs1 = ctx->lexer->tokens[i+3];
      struct token_data *td_c2  = ctx->lexer->tokens[i+4];
      struct token_data *td_rs2 = ctx->lexer->tokens[i+5];

      if (td_rd->type != token_register || td_rs1->type != token_register || td_rs2->type != token_register)
      {
        die(1, "ADD EXPECTS REG,REG,REG");
      }

      (void)td_c1;
      (void)td_c2;

      u32 rd  = (u32)lc_reg_index(td_rd->value);
      u32 rs1 = (u32)lc_reg_index(td_rs1->value);
      u32 rs2 = (u32)lc_reg_index(td_rs2->value);

      code = lc_encode_add(rd, rs1, rs2);
    }
    else if (strcmp(mnemo, "ADDI") == 0)
    {
      struct token_data *td_rd  = ctx->lexer->tokens[i+1];
      struct token_data *td_c1  = ctx->lexer->tokens[i+2];
      struct token_data *td_rs1 = ctx->lexer->tokens[i+3];
      struct token_data *td_c2  = ctx->lexer->tokens[i+4];
      struct token_data *td_imm = ctx->lexer->tokens[i+5];

      if (td_rd->type != token_register || td_rs1->type != token_register || td_imm->type != token_number)
      {
        die(1, "ADDI EXPECTS REG,REG,IMM");
      }

      (void)td_c1;
      (void)td_c2;

      u32 rd  = (u32)lc_reg_index(td_rd->value);
      u32 rs1 = (u32)lc_reg_index(td_rs1->value);
      i32 imm = lc_parse_imm(td_imm->value);

      code = lc_encode_addi(rd, rs1, imm);
    }
    else if (strcmp(mnemo, "SW") == 0)
    {
      struct token_data *td_rs2 = ctx->lexer->tokens[i+1];
      struct token_data *td_c1  = ctx->lexer->tokens[i+2];
      struct token_data *td_imm = ctx->lexer->tokens[i+3];
      struct token_data *td_lp  = ctx->lexer->tokens[i+4];
      struct token_data *td_rs1 = ctx->lexer->tokens[i+5];
      struct token_data *td_rp  = ctx->lexer->tokens[i+6];

      if (td_rs2->type != token_register || td_imm->type != token_number ||
          td_rs1->type != token_register || td_lp->type != token_lparen ||
          td_rp->type != token_rparen)
      {
        die(1, "SW EXPECTS REG,IMM(REG)");
      }

      (void)td_c1;

      u32 rs2 = (u32)lc_reg_index(td_rs2->value);
      u32 rs1 = (u32)lc_reg_index(td_rs1->value);
      i32 imm = lc_parse_imm(td_imm->value);

      code = lc_encode_sw(rs2, rs1, imm);
    }
    else if (strcmp(mnemo, "LW") == 0)
    {
      struct token_data *td_rd  = ctx->lexer->tokens[i+1];
      struct token_data *td_c1  = ctx->lexer->tokens[i+2];
      struct token_data *td_imm = ctx->lexer->tokens[i+3];
      struct token_data *td_lp  = ctx->lexer->tokens[i+4];
      struct token_data *td_rs1 = ctx->lexer->tokens[i+5];
      struct token_data *td_rp  = ctx->lexer->tokens[i+6];

      if (td_rd->type != token_register || td_imm->type != token_number ||
          td_rs1->type != token_register || td_lp->type != token_lparen ||
          td_rp->type != token_rparen)
      {
        die(1, "LW EXPECTS REG,IMM(REG)");
      }

      (void)td_c1;

      u32 rd  = (u32)lc_reg_index(td_rd->value);
      u32 rs1 = (u32)lc_reg_index(td_rs1->value);
      i32 imm = lc_parse_imm(td_imm->value);

      code = lc_encode_lw(rd, rs1, imm);
    }
    else if (strcmp(mnemo, "BNE") == 0)
    {
      struct token_data *td_rs1 = ctx->lexer->tokens[i+1];
      struct token_data *td_c1  = ctx->lexer->tokens[i+2];
      struct token_data *td_rs2 = ctx->lexer->tokens[i+3];
      struct token_data *td_c2  = ctx->lexer->tokens[i+4];
      struct token_data *td_lbl = ctx->lexer->tokens[i+5];

      if (td_rs1->type != token_register || td_rs2->type != token_register ||
          td_lbl->type != token_tag)
      {
        die(1, "BNE EXPECTS REG,REG,LABEL");
      }

      (void)td_c1;
      (void)td_c2;

      u32 rs1 = (u32)lc_reg_index(td_rs1->value);
      u32 rs2 = (u32)lc_reg_index(td_rs2->value);

      const struct lc_label *lbl = lc_labels_find(ctx, td_lbl->value);
      if (NULL == lbl)
      {
        die(1, "UNKNOWN LABEL %s", td_lbl->value);
      }

      i32 imm = (i32)lbl->addr - (i32)pc;

      code = lc_encode_bne(rs1, rs2, imm);
    }
    else if (strcmp(mnemo, "MV") == 0)
    {
      struct token_data *td_rd  = ctx->lexer->tokens[i+1];
      struct token_data *td_c1  = ctx->lexer->tokens[i+2];
      struct token_data *td_rs1 = ctx->lexer->tokens[i+3];

      if (td_rd->type != token_register || td_rs1->type != token_register)
      {
        die(1, "MV EXPECTS REG,REG");
      }

      (void)td_c1;

      u32 rd  = (u32)lc_reg_index(td_rd->value);
      u32 rs1 = (u32)lc_reg_index(td_rs1->value);

      code = lc_encode_addi(rd, rs1, 0);
    }
    else if (strcmp(mnemo, "RET") == 0)
    {
      u32 rd  = 0;  /* X0 */
      u32 rs1 = 1;  /* RA */
      i32 imm = 0;

      code = lc_encode_jalr(rd, rs1, imm);
    }
    else
    {
      die(1, "UNSUPPORTED INSTRUCTION %s", mnemo);
    }

    lc_buf_push(&buf, code);
    pc += 4;
  }

  lc_write_elf(&buf, output_path, ctx->base_addr);
  lc_buf_free(&buf);
}

static char *lc_make_output_path(const char *input_path)
{
  size_t len = strlen(input_path);
  char *out = malloc(len + 5);
  if (NULL == out)
  {
    die(1, "MALLOC OUTPUT PATH");
  }

  strcpy(out, input_path);

  char *dot = strrchr(out, '.');
  if (dot != NULL)
  {
    *dot = '\0';
  }
  strcat(out, ".elf");

  return out;
}

u0 lc_assemble(struct lexer *l, const char *input_path)
{
  struct lc_ctx ctx;
  ctx.lexer = l;
  lc_labels_init(&ctx);

  lc_first_pass(&ctx);

  char *out_path = lc_make_output_path(input_path);
  lc_second_pass(&ctx, out_path);

  free(out_path);
  lc_labels_free(&ctx);
}

