#ifndef _FMT_B_LOOK_UP_H
#define _FMT_B_LOOK_UP_H

#include <rv32/fmt/b/fmt_b.h>
#include <rv32/instr.h>
#include <string.h>
#include <utils/common.h>

static const struct instr fmt_b_array[] =
{
  { .label = "BEQ",  .type = FMT_B, .b = { .opcode = 0x63, .funct3 = 0x0 } },
  { .label = "BNE",  .type = FMT_B, .b = { .opcode = 0x63, .funct3 = 0x1 } },
  { .label = "BLT",  .type = FMT_B, .b = { .opcode = 0x63, .funct3 = 0x4 } },
  { .label = "BGE",  .type = FMT_B, .b = { .opcode = 0x63, .funct3 = 0x5 } },
  { .label = "BLTU", .type = FMT_B, .b = { .opcode = 0x63, .funct3 = 0x6 } },
  { .label = "BGEU", .type = FMT_B, .b = { .opcode = 0x63, .funct3 = 0x7 } },
};

#define FMT_B_ARRAY_SIZE (sizeof(fmt_b_array) / sizeof(fmt_b_array[0]))

static inline u32 fmt_b_encode(struct instr *e, u8 rs1, u8 rs2, i16 imm)
{
  expect(NULL != e);
  expect(e->type == FMT_B);

  struct instr c = *e;
  u32 uimm = (u32)(i32)imm;

  c.b.rs1     = rs1;
  c.b.rs2     = rs2;
  c.b.imm11   = (uimm >> 11) & 0x1;
  c.b.imm4_1  = (uimm >>  1) & 0xF;
  c.b.imm10_5 = (uimm >>  5) & 0x3F;
  c.b.imm12   = (uimm >> 12) & 0x1;
  return c.raw;
}

static inline const struct instr *fmt_b_look_up(const char *label)
{
  for (u32 i = 0; i < FMT_B_ARRAY_SIZE; ++i)
  {
    if (strcmp(label, fmt_b_array[i].label) == 0)
      return &fmt_b_array[i];
  }
  return NULL;
}

#endif /* _FMT_B_LOOK_UP_H */
