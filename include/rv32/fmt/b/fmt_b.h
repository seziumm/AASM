#ifndef _FMT_B_H
#define _FMT_B_H

#include <utils/type.h>

struct fmt_b
{
  u32 opcode  : 7;
  u32 imm11   : 1;
  u32 imm4_1  : 4;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 rs2     : 5;
  u32 imm10_5 : 6;
  u32 imm12   : 1;
};

#endif /* _FMT_B_H */
