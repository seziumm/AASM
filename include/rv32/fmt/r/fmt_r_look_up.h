#ifndef _FMT_R_LOOKUP_H
#define _FMT_R_LOOKUP_H

#include <rv32/fmt/r/fmt_r.h>
#include <rv32/instr.h>
#include <string.h>
#include <utils/common.h>

static const struct instr fmt_r_array[] =
{
  { .label = "ADD",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x0, .funct7 = 0x00 } },
  { .label = "SUB",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x0, .funct7 = 0x20 } },
  { .label = "SLL",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x1, .funct7 = 0x00 } },
  { .label = "SLT",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x2, .funct7 = 0x00 } },
  { .label = "SLTU",   .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x3, .funct7 = 0x00 } },
  { .label = "XOR",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x4, .funct7 = 0x00 } },
  { .label = "SRL",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x5, .funct7 = 0x00 } },
  { .label = "SRA",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x5, .funct7 = 0x20 } },
  { .label = "OR",     .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x6, .funct7 = 0x00 } },
  { .label = "AND",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x7, .funct7 = 0x00 } },
  /* RV32M */
  { .label = "MUL",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x0, .funct7 = 0x01 } },
  { .label = "MULH",   .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x1, .funct7 = 0x01 } },
  { .label = "MULHSU", .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x2, .funct7 = 0x01 } },
  { .label = "MULHU",  .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x3, .funct7 = 0x01 } },
  { .label = "DIV",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x4, .funct7 = 0x01 } },
  { .label = "DIVU",   .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x5, .funct7 = 0x01 } },
  { .label = "REM",    .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x6, .funct7 = 0x01 } },
  { .label = "REMU",   .type = FMT_R, .r = { .opcode = 0x33, .funct3 = 0x7, .funct7 = 0x01 } },
};

#define FMT_R_ARRAY_SIZE sizeof(fmt_r_array) / sizeof(fmt_r_array[0])

static inline u32 fmt_r_encode(struct instr *e, u8 rd, u8 rs1, u8 rs2)
{
  expect(NULL != e);
  expect(e->type == FMT_R);

  /* maybe we can actually overwrite e but idk this feels safer */
  struct instr c = *e;
  c.r.rd  = rd;
  c.r.rs1 = rs1;
  c.r.rs2 = rs2;
  return c.raw;

}

static const struct instr *fmt_r_look_up(const char *label)
{
  for (u32 i = 0; i < FMT_R_ARRAY_SIZE; ++i)
  {
    if (strcmp(label, fmt_r_array[i].label) == 0)
    {
      return &fmt_r_array[i];
    }
  }
  return NULL;
}

#endif /* _FMT_R_LOOKUP_H */
