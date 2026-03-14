#ifndef _FMT_U_H
#define _FMT_U_H

#include <utils/type.h>

struct fmt_u
{
  u32 opcode   : 7;
  u32 rd       : 5;
  u32 imm31_12 : 20;
};

#endif /* _FMT_U_H */
