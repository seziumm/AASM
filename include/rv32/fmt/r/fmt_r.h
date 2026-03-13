#ifndef _FMT_R_H
#define _FMT_R_H

#include <utils/type.h>

struct fmt_r
{
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  u32 funct7 : 7;
};

#endif /* _FMT_R_H */
