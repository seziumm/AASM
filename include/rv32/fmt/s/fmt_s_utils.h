#ifndef _FMT_S_UTILS_H
#define _FMT_S_UTILS_H

#include <rv32/fmt/s/fmt_s.h>
#include <rv32/instr.h>
#include <string.h>
#include <utils/common.h>

static const struct instr fmt_s_array[] =
{
  { .label = "SB", .type = FMT_S, .s = { .opcode = 0x23, .funct3 = 0x0 } },
  { .label = "SH", .type = FMT_S, .s = { .opcode = 0x23, .funct3 = 0x1 } },
  { .label = "SW", .type = FMT_S, .s = { .opcode = 0x23, .funct3 = 0x2 } },
};

#define FMT_S_ARRAY_SIZE (sizeof(fmt_s_array) / sizeof(fmt_s_array[0]))

/* Returns the instr descriptor for the given mnemonic, or NULL. */
static inline const struct instr *fmt_s_from_label(const char *label)
{
  for (u32 i = 0; i < FMT_S_ARRAY_SIZE; i++)
    if (strcmp(label, fmt_s_array[i].label) == 0)
      return &fmt_s_array[i];
  return NULL;
}

/* Encode an S-type instruction word. */
static inline u32 fmt_s_encode(const struct instr *e, u8 rs1, u8 rs2, i16 imm)
{
  expect(NULL != e);
  expect(e->type == FMT_S);
  struct instr c = *e;
  c.s.rs1     = rs1;
  c.s.rs2     = rs2;
  c.s.imm4_0  = ((u32)(i32)imm >> 0) & 0x1F;
  c.s.imm11_5 = ((u32)(i32)imm >> 5) & 0x7F;
  return c.raw;
}

#endif /* _FMT_S_UTILS_H */
