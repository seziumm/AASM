#ifndef _INSTR_H
#define _INSTR_H

#include <utils/common.h>
#include <rv32/fmt/r/fmt_r.h>
#include <rv32/fmt/i/fmt_i.h>
#include <rv32/fmt/fmt_type.h>
#include <utils/type.h>

struct instr
{
  const char   *label;
  enum fmt_type type;
  union
  {
    struct fmt_r r;
    struct fmt_i i;
    u32 raw;
  };
};

#endif /* _INSTR_H */


// /* S-type
//  * [6:0]   opcode
//  * [11:7]  imm[4:0]
//  * [14:12] funct3
//  * [19:15] rs1
//  * [24:20] rs2
//  * [31:25] imm[11:5]
//  */
// struct s_type
// {
//   u32 opcode  : 7;
//   u32 imm4_0  : 5;
//   u32 funct3  : 3;
//   u32 rs1     : 5;
//   u32 rs2     : 5;
//   u32 imm11_5 : 7;
// };
//
// /* B-type
//  * [6:0]   opcode
//  * [7]     imm[11]
//  * [11:8]  imm[4:1]
//  * [14:12] funct3
//  * [19:15] rs1
//  * [24:20] rs2
//  * [30:25] imm[10:5]
//  * [31]    imm[12]
//  */
// struct b_type
// {
//   u32 opcode  : 7;
//   u32 imm11   : 1;
//   u32 imm4_1  : 4;
//   u32 funct3  : 3;
//   u32 rs1     : 5;
//   u32 rs2     : 5;
//   u32 imm10_5 : 6;
//   u32 imm12   : 1;
// };
//
// /* U-type
//  * [6:0]   opcode
//  * [11:7]  rd
//  * [31:12] imm[31:12]
//  */
// struct u_type
// {
//   u32 opcode   : 7;
//   u32 rd       : 5;
//   u32 imm31_12 : 20;
// };
//
// /* J-type
//  * [6:0]   opcode
//  * [11:7]  rd
//  * [19:12] imm[19:12]
//  * [20]    imm[11]
//  * [30:21] imm[10:1]
//  * [31]    imm[20]
//  */
// struct j_type
// {
//   u32 opcode   : 7;
//   u32 rd       : 5;
//   u32 imm19_12 : 8;
//   u32 imm11    : 1;
//   u32 imm10_1  : 10;
//   u32 imm20    : 1;
// };
