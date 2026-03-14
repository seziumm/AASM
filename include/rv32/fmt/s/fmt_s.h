#ifndef _FMT_S_H
#define _FMT_S_H

#include <utils/type.h>

struct fmt_s
{
  u32 opcode  : 7;
  u32 imm4_0  : 5;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 rs2     : 5;
  u32 imm11_5 : 7;
};

#endif /* _FMT_S_H */
