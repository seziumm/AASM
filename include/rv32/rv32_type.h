#ifndef _RV32_TYPE_H
#define _RV32_TYPE_H

/* ============================================================
 *  Instruction format types
 * ============================================================ */

enum instr_type
{
  R_TYPE,
  I_TYPE,
  S_TYPE,
  B_TYPE,
  U_TYPE,
  J_TYPE
};

int params_of_instr(enum instr_type type)
{
    switch (type)
    {
        case R_TYPE: return 3; // rd, rs1, rs2
        case I_TYPE: return 3; // rd, rs1, imm
        case S_TYPE: return 3; // rs2, rs1, imm
        case B_TYPE: return 3; // rs1, rs2, imm
        case U_TYPE: return 2; // rd, imm
        case J_TYPE: return 2; // rd, imm
        default: return 0;
    }
};

#endif /* _RV32_TYPE_H */
