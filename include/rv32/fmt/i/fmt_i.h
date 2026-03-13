#ifndef _FMT_I_H
#define _FMT_I_H

#include <utils/type.h>

struct fmt_i
{
  u32 opcode  : 7;
  u32 rd      : 5;
  u32 funct3  : 3;
  u32 rs1     : 5;
  u32 imm11_0 : 12;
};

#endif /* _FMT_I_H */
