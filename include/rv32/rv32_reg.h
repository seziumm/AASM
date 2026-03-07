#ifndef _RV32_REG_H
#define _RV32_REG_H

#include <string.h>
#include <type.h>

/* ============================================================
 *  Register name table
 *
 *  ABI names ordered by physical register number,
 *  followed by X-form aliases (X0..X31).
 * ============================================================ */

static const struct { const char *name; u8 index; } rv32_reg_table[] =
{
  /* ABI names */
  { "ZERO",  0 }, { "RA",   1 }, { "SP",   2 }, { "GP",   3 },
  { "TP",    4 }, { "T0",   5 }, { "T1",   6 }, { "T2",   7 },
  { "S0",    8 }, { "S1",   9 }, { "A0",  10 }, { "A1",  11 },
  { "A2",   12 }, { "A3",  13 }, { "A4",  14 }, { "A5",  15 },
  { "A6",   16 }, { "A7",  17 }, { "S2",  18 }, { "S3",  19 },
  { "S4",   20 }, { "S5",  21 }, { "S6",  22 }, { "S7",  23 },
  { "S8",   24 }, { "S9",  25 }, { "S10", 26 }, { "S11", 27 },
  { "T3",   28 }, { "T4",  29 }, { "T5",  30 }, { "T6",  31 },
  /* X-form aliases */
  { "X0",   0 }, { "X1",   1 }, { "X2",   2 }, { "X3",   3 },
  { "X4",   4 }, { "X5",   5 }, { "X6",   6 }, { "X7",   7 },
  { "X8",   8 }, { "X9",   9 }, { "X10", 10 }, { "X11", 11 },
  { "X12", 12 }, { "X13", 13 }, { "X14", 14 }, { "X15", 15 },
  { "X16", 16 }, { "X17", 17 }, { "X18", 18 }, { "X19", 19 },
  { "X20", 20 }, { "X21", 21 }, { "X22", 22 }, { "X23", 23 },
  { "X24", 24 }, { "X25", 25 }, { "X26", 26 }, { "X27", 27 },
  { "X28", 28 }, { "X29", 29 }, { "X30", 30 }, { "X31", 31 },
};

#define RV32_REG_TABLE_SIZE (sizeof(rv32_reg_table) / sizeof(rv32_reg_table[0]))

/* ============================================================
 *  Lookup
 *
 *  Returns the physical register number [0..31],
 *  or -1 if the name is not a valid register.
 * ============================================================ */

static inline i32 rv32_reg_lookup(const char *name)
{
  for (u32 i = 0; i < RV32_REG_TABLE_SIZE; ++i)
    if (strcmp(name, rv32_reg_table[i].name) == 0)
      return rv32_reg_table[i].index;

  return -1;
}

#endif /* _RV32_REG_H */
