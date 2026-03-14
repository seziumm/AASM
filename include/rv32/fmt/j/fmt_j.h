#ifndef _FMT_J_H
#define _FMT_J_H

#include <utils/type.h>

struct fmt_j
{
  u32 opcode   : 7;
  u32 rd       : 5;
  u32 imm19_12 : 8;
  u32 imm11    : 1;
  u32 imm10_1  : 10;
  u32 imm20    : 1;
};

#endif /* _FMT_J_H */
