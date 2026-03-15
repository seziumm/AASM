#ifndef _FMT_U_UTILS_H
#define _FMT_U_UTILS_H

#include <rv32/fmt/u/fmt_u.h>
#include <rv32/instr.h>
#include <string.h>
#include <utils/common.h>

static const struct instr fmt_u_array[] =
{
  { .label = "LUI",   .type = FMT_U, .u = { .opcode = 0x37 } },
  { .label = "AUIPC", .type = FMT_U, .u = { .opcode = 0x17 } },
};

#define FMT_U_ARRAY_SIZE (sizeof(fmt_u_array) / sizeof(fmt_u_array[0]))

/* Returns the instr descriptor for the given mnemonic, or NULL. */
static inline const struct instr *fmt_u_from_label(const char *label)
{
  for (u32 i = 0; i < FMT_U_ARRAY_SIZE; i++)
    if (strcmp(label, fmt_u_array[i].label) == 0)
      return &fmt_u_array[i];
  return NULL;
}

/* Encode a U-type instruction word. */
static inline u32 fmt_u_encode(const struct instr *e, u8 rd, u32 imm20)
{
  expect(NULL != e);
  expect(e->type == FMT_U);
  struct instr c = *e;
  c.u.rd       = rd;
  c.u.imm31_12 = imm20 & 0xFFFFF;
  return c.raw;
}

#endif /* _FMT_U_UTILS_H */
