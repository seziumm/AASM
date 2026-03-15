#ifndef _FMT_J_UTILS_H
#define _FMT_J_UTILS_H

#include <rv32/fmt/j/fmt_j.h>
#include <rv32/instr.h>
#include <string.h>
#include <utils/common.h>

static const struct instr fmt_j_array[] =
{
  { .label = "JAL", .type = FMT_J, .j = { .opcode = 0x6F } },
};

#define FMT_J_ARRAY_SIZE (sizeof(fmt_j_array) / sizeof(fmt_j_array[0]))

/* Returns the instr descriptor for the given mnemonic, or NULL. */
static inline const struct instr *fmt_j_from_label(const char *label)
{
  for (u32 i = 0; i < FMT_J_ARRAY_SIZE; i++)
    if (strcmp(label, fmt_j_array[i].label) == 0)
      return &fmt_j_array[i];
  return NULL;
}

/* Encode a J-type instruction word. */
static inline u32 fmt_j_encode(const struct instr *e, u8 rd, i32 imm)
{
  expect(NULL != e);
  expect(e->type == FMT_J);
  struct instr c    = *e;
  u32          uimm = (u32)imm;
  c.j.rd       = rd;
  c.j.imm19_12 = (uimm >> 12) & 0xFF;
  c.j.imm11    = (uimm >> 11) & 0x1;
  c.j.imm10_1  = (uimm >>  1) & 0x3FF;
  c.j.imm20    = (uimm >> 20) & 0x1;
  return c.raw;
}

#endif /* _FMT_J_UTILS_H */
