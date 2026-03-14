#ifndef _FMT_I_LOOKUP_H
#define _FMT_I_LOOKUP_H

#include <rv32/fmt/i/fmt_i.h>
#include <rv32/instr.h>
#include <string.h>
#include <utils/common.h>

static const struct instr fmt_i_array[] =
{
  /* JALR */
  { .label = "JALR",  .type = FMT_I, .i = { .opcode = 0x67, .funct3 = 0x0 } },
  /* load */
  { .label = "LB",    .type = FMT_I, .i = { .opcode = 0x03, .funct3 = 0x0 } },
  { .label = "LH",    .type = FMT_I, .i = { .opcode = 0x03, .funct3 = 0x1 } },
  { .label = "LW",    .type = FMT_I, .i = { .opcode = 0x03, .funct3 = 0x2 } },
  { .label = "LBU",   .type = FMT_I, .i = { .opcode = 0x03, .funct3 = 0x4 } },
  { .label = "LHU",   .type = FMT_I, .i = { .opcode = 0x03, .funct3 = 0x5 } },
  /* immediate ALU */
  { .label = "ADDI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x0 } },
  { .label = "SLTI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x2 } },
  { .label = "SLTIU", .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x3 } },
  { .label = "XORI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x4 } },
  { .label = "ORI",   .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x6 } },
  { .label = "ANDI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x7 } },
  /* shift immediate — imm11_0 pre-encoded: funct7[11:5] | shamt[4:0] */
  { .label = "SLLI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x1, .imm11_0 = 0x000 } },
  { .label = "SRLI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x5, .imm11_0 = 0x000 } },
  { .label = "SRAI",  .type = FMT_I, .i = { .opcode = 0x13, .funct3 = 0x5, .imm11_0 = 0x400 } },
};

#define FMT_I_ARRAY_SIZE (sizeof(fmt_i_array) / sizeof(fmt_i_array[0]))

static inline u32 fmt_i_encode(struct instr *e, u8 rd, u8 rs1, u16 imm)
{
  expect(NULL != e);
  expect(e->type == FMT_I);
  struct instr c = *e;
  c.i.rd  = rd;
  c.i.rs1 = rs1;
  /* for shifts imm11_0 has funct7 pre-set — OR in the shamt only */
  c.i.imm11_0 = (c.i.imm11_0 & 0xFE0) | (imm & 0x1F);
  /* for non-shift instructions imm11_0 was 0 so OR is safe */
  if ((c.i.funct3 != 0x1) && (c.i.funct3 != 0x5))
    c.i.imm11_0 = imm & 0xFFF;
  return c.raw;
}

static inline const struct instr *fmt_i_look_up(const char *label)
{
  for (u32 i = 0; i < FMT_I_ARRAY_SIZE; ++i)
  {
    if (strcmp(label, fmt_i_array[i].label) == 0)
    {
      return &fmt_i_array[i];
    }
  }
  return NULL;
}

static inline const struct instr *fmt_i_look_up_load(const char *label)
{
  for (u32 i = 0; i < FMT_I_ARRAY_SIZE; ++i)
  {
    if (strcmp(label, fmt_i_array[i].label) == 0 && fmt_i_array[i].i.opcode == 0x03)
    {
      return &fmt_i_array[i];
    }
  }
  return NULL;

}

#endif /* _FMT_I_LOOKUP_H */
