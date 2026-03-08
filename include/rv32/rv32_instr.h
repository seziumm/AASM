#ifndef _RV32_INSTR_H
#define _RV32_INSTR_H

#include <rv32/rv32_type.h>
#include <string.h>
#include <type.h>

/* ============================================================
 *  RV32I instruction set enum
 * ============================================================ */

enum rv32i_instr
{
  /* R-type arithmetic */
  RV32I_ADD,
  RV32I_SUB,
  RV32I_XOR,
  RV32I_OR,
  RV32I_AND,
  RV32I_SLL,
  RV32I_SRL,
  RV32I_SRA,
  RV32I_SLT,
  RV32I_SLTU,

  /* I-type arithmetic (immediate) */
  RV32I_ADDI,
  RV32I_XORI,
  RV32I_ORI,
  RV32I_ANDI,
  RV32I_SLLI,
  RV32I_SRLI,
  RV32I_SRAI,
  RV32I_SLTI,
  RV32I_SLTIU,

  /* I-type loads */
  RV32I_LB,
  RV32I_LH,
  RV32I_LW,
  RV32I_LBU,
  RV32I_LHU,

  /* I-type jump / system */
  RV32I_JALR,
  RV32I_ECALL,
  RV32I_EBREAK,

  /* S-type stores */
  RV32I_SB,
  RV32I_SH,
  RV32I_SW,

  /* B-type branches */
  RV32I_BEQ,
  RV32I_BNE,
  RV32I_BLT,
  RV32I_BGE,
  RV32I_BLTU,
  RV32I_BGEU,

  /* U-type upper-immediate */
  RV32I_LUI,
  RV32I_AUIPC,

  /* J-type jump */
  RV32I_JAL,

  RV32I_NUM_INSTRUCTIONS
};

/* ============================================================
 *  Opcode table entry  — one raw 32-bit template word per
 *  instruction.  rd / rs / imm are 0; filled by rv32_encode().
 * ============================================================ */

struct rv32ii_opcode_entry
{
  enum rv32i_instr   index;
  enum instr_type    type;
  u32                raw;
};

/* ============================================================
 *  Compile-time template macros
 *
 *  Each macro produces a true integer constant (usable in a
 *  static initializer) encoding only the fixed fields of the
 *  instruction template.  Variable fields (rd, rs1, rs2, imm)
 *  are left as 0 and OR-ed in at encode time.
 *
 *  R  template: opcode | funct3 | funct7
 *  I  template: opcode | funct3 | imm[11:0]  (imm for SRAI/EBREAK only)
 *  S  template: opcode | funct3
 *  B  template: opcode | funct3
 *  U  template: opcode
 *  J  template: opcode
 * ============================================================ */

#define RV32_TMPL_R(opcode, funct3, funct7) \
  ( ((u32)(funct7) << 25) | ((u32)(funct3) << 12) | (u32)(opcode) )
#define RV32_TMPL_I(opcode, funct3, imm) \
  ( ((u32)((imm) & 0xFFF) << 20) | ((u32)(funct3) << 12) | (u32)(opcode) )
#define RV32_TMPL_S(opcode, funct3) ( ((u32)(funct3) << 12) | (u32)(opcode) )
#define RV32_TMPL_B(opcode, funct3) ( ((u32)(funct3) << 12) | (u32)(opcode) )
#define RV32_TMPL_U(opcode) ( (u32)(opcode) )
#define RV32_TMPL_J(opcode) ( (u32)(opcode) )

/* ============================================================
 *  Opcode table
 * ============================================================ */

static const struct rv32ii_opcode_entry rv32ii_opcode_list[RV32I_NUM_INSTRUCTIONS] =
{
  /* ---- R-type arithmetic ---------------------------------- */
  { RV32I_ADD,    R_TYPE, RV32_TMPL_R(0x33, 0x0, 0x00) },
  { RV32I_SUB,    R_TYPE, RV32_TMPL_R(0x33, 0x0, 0x20) },
  { RV32I_XOR,    R_TYPE, RV32_TMPL_R(0x33, 0x4, 0x00) },
  { RV32I_OR,     R_TYPE, RV32_TMPL_R(0x33, 0x6, 0x00) },
  { RV32I_AND,    R_TYPE, RV32_TMPL_R(0x33, 0x7, 0x00) },
  { RV32I_SLL,    R_TYPE, RV32_TMPL_R(0x33, 0x1, 0x00) },
  { RV32I_SRL,    R_TYPE, RV32_TMPL_R(0x33, 0x5, 0x00) },
  { RV32I_SRA,    R_TYPE, RV32_TMPL_R(0x33, 0x5, 0x20) },
  { RV32I_SLT,    R_TYPE, RV32_TMPL_R(0x33, 0x2, 0x00) },
  { RV32I_SLTU,   R_TYPE, RV32_TMPL_R(0x33, 0x3, 0x00) },

  /* ---- I-type arithmetic (immediate) --------------------- */
  { RV32I_ADDI,   I_TYPE, RV32_TMPL_I(0x13, 0x0, 0)    },
  { RV32I_XORI,   I_TYPE, RV32_TMPL_I(0x13, 0x4, 0)    },
  { RV32I_ORI,    I_TYPE, RV32_TMPL_I(0x13, 0x6, 0)    },
  { RV32I_ANDI,   I_TYPE, RV32_TMPL_I(0x13, 0x7, 0)    },
  { RV32I_SLLI,   I_TYPE, RV32_TMPL_I(0x13, 0x1, 0)    },
  { RV32I_SRLI,   I_TYPE, RV32_TMPL_I(0x13, 0x5, 0)    },
  { RV32I_SRAI,   I_TYPE, RV32_TMPL_I(0x13, 0x5, 0x20) },  /* imm[11:5]=0x20 encodes funct7 */
  { RV32I_SLTI,   I_TYPE, RV32_TMPL_I(0x13, 0x2, 0)    },
  { RV32I_SLTIU,  I_TYPE, RV32_TMPL_I(0x13, 0x3, 0)    },

  /* ---- I-type loads -------------------------------------- */
  { RV32I_LB,     I_TYPE, RV32_TMPL_I(0x03, 0x0, 0)    },
  { RV32I_LH,     I_TYPE, RV32_TMPL_I(0x03, 0x1, 0)    },
  { RV32I_LW,     I_TYPE, RV32_TMPL_I(0x03, 0x2, 0)    },
  { RV32I_LBU,    I_TYPE, RV32_TMPL_I(0x03, 0x4, 0)    },
  { RV32I_LHU,    I_TYPE, RV32_TMPL_I(0x03, 0x5, 0)    },

  /* ---- I-type jump / system ------------------------------ */
  { RV32I_JALR,   I_TYPE, RV32_TMPL_I(0x67, 0x0, 0)    },
  { RV32I_ECALL,  I_TYPE, RV32_TMPL_I(0x73, 0x0, 0)    },
  { RV32I_EBREAK, I_TYPE, RV32_TMPL_I(0x73, 0x0, 1)    },  /* imm=1 distinguishes from ECALL */

  /* ---- S-type stores ------------------------------------- */
  { RV32I_SB,     S_TYPE, RV32_TMPL_S(0x23, 0x0)        },
  { RV32I_SH,     S_TYPE, RV32_TMPL_S(0x23, 0x1)        },
  { RV32I_SW,     S_TYPE, RV32_TMPL_S(0x23, 0x2)        },

  /* ---- B-type branches ----------------------------------- */
  { RV32I_BEQ,    B_TYPE, RV32_TMPL_B(0x63, 0x0)        },
  { RV32I_BNE,    B_TYPE, RV32_TMPL_B(0x63, 0x1)        },
  { RV32I_BLT,    B_TYPE, RV32_TMPL_B(0x63, 0x4)        },
  { RV32I_BGE,    B_TYPE, RV32_TMPL_B(0x63, 0x5)        },
  { RV32I_BLTU,   B_TYPE, RV32_TMPL_B(0x63, 0x6)        },
  { RV32I_BGEU,   B_TYPE, RV32_TMPL_B(0x63, 0x7)        },

  /* ---- U-type upper-immediate ---------------------------- */
  { RV32I_LUI,    U_TYPE, RV32_TMPL_U(0x37)             },
  { RV32I_AUIPC,  U_TYPE, RV32_TMPL_U(0x17)             },

  /* ---- J-type jump --------------------------------------- */
  { RV32I_JAL,    J_TYPE, RV32_TMPL_J(0x6F)             },
};

/* ============================================================
 *  Instruction field bitfields  (C99, mirror the ISA spec)
 *
 *  Used only at encode/decode time, never in static storage.
 * ============================================================ */

struct rv32_r_field               /* R  [funct7 | rs2 | rs1 | funct3 | rd | opcode] */
{
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  u32 funct7 : 7;
};

struct rv32_i_field               /* I  [imm[11:0] | rs1 | funct3 | rd | opcode]   */
{
  u32 opcode : 7;
  u32 rd     : 5;
  u32 funct3 : 3;
  u32 rs1    : 5;
  i32 imm    : 12;
};

struct rv32_s_field               /* S  [imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode] */
{
  u32 opcode : 7;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  i32 imm    : 12;
};

struct rv32_b_field               /* B  [imm[12|10:5] | rs2 | rs1 | funct3 | imm[4:1|11] | opcode] */
{
  u32 opcode : 7;
  u32 funct3 : 3;
  u32 rs1    : 5;
  u32 rs2    : 5;
  i32 imm    : 13;
};

struct rv32_u_field               /* U  [imm[31:12] | rd | opcode]                  */
{
  u32 opcode : 7;
  u32 rd     : 5;
  i32 imm    : 20;
};

struct rv32_j_field               /* J  [imm[20|10:1|11|19:12] | rd | opcode]       */
{
  u32 opcode : 7;
  u32 rd     : 5;
  i32 imm    : 21;
};

/* C99 type-pun union: reading .raw after writing a named field
   is explicitly defined behaviour (ISO C99 §6.5.2.3 note 82). */

union rv32_enc
{
  struct rv32_r_field r;
  struct rv32_i_field i;
  struct rv32_s_field s;
  struct rv32_b_field b;
  struct rv32_u_field u;
  struct rv32_j_field j;
  u32                 raw;
};

/* ============================================================
 *  Encode: OR variable fields into a raw template word
 * ============================================================ */

static inline u32 rv32_encode(const struct rv32ii_opcode_entry *e,
                               u8 rd, u8 rs1, u8 rs2, i32 imm)
{
  union rv32_enc w = { .raw = e->raw };
  u32 u = (u32)imm;

  switch (e->type)
  {
    case R_TYPE:
      w.r.rd  = rd;
      w.r.rs1 = rs1;
      w.r.rs2 = rs2;
      break;

    case I_TYPE:
      w.i.rd  = rd;
      w.i.rs1 = rs1;
      w.i.imm = imm;
      break;

    /* S-type: imm is split [11:5] at bits[31:25] and [4:0] at bits[11:7]  */
    case S_TYPE:
      w.raw |= ((u & 0xFE0u) << 20);   /* imm[11:5] → bits[31:25] */
      w.raw |= ((u & 0x01Fu) <<  7);   /* imm[4:0]  → bits[11:7]  */
      w.raw |= ((u32)rs1 << 15);
      w.raw |= ((u32)rs2 << 20);
      break;

    /* B-type: imm is split [12|10:5] at bits[31:25] and [4:1|11] at bits[11:7] */
    case B_TYPE:
      w.raw |= (((u >> 12) & 0x1u)  << 31);  /* imm[12]   → bit[31]     */
      w.raw |= (((u >>  5) & 0x3Fu) << 25);  /* imm[10:5] → bits[30:25] */
      w.raw |= (((u >>  1) & 0xFu)  <<  8);  /* imm[4:1]  → bits[11:8]  */
      w.raw |= (((u >> 11) & 0x1u)  <<  7);  /* imm[11]   → bit[7]      */
      w.raw |= ((u32)rs1 << 15);
      w.raw |= ((u32)rs2 << 20);
      break;

    case U_TYPE:
      w.u.rd  = rd;
      w.u.imm = imm;
      break;

    /* J-type: imm is split [20|10:1|11|19:12] */
    case J_TYPE:
      w.raw |= (((u >> 20) & 0x1u)  << 31);  /* imm[20]   → bit[31]     */
      w.raw |= (((u >>  1) & 0x3FFu)<< 21);  /* imm[10:1] → bits[30:21] */
      w.raw |= (((u >> 11) & 0x1u)  << 20);  /* imm[11]   → bit[20]     */
      w.raw |= (((u >> 12) & 0xFFu) << 12);  /* imm[19:12]→ bits[19:12] */
      w.raw |= ((u32)rd << 7);
      break;
  }

  return w.raw;
}

/* ============================================================
 *  Label lookup  (mnemonic string from enum)
 * ============================================================ */

static inline const char *rv32ii_instr_label(enum rv32i_instr idx)
{
  switch (idx)
  {
    case RV32I_ADD:    return "ADD";
    case RV32I_SUB:    return "SUB";
    case RV32I_XOR:    return "XOR";
    case RV32I_OR:     return "OR";
    case RV32I_AND:    return "AND";
    case RV32I_SLL:    return "SLL";
    case RV32I_SRL:    return "SRL";
    case RV32I_SRA:    return "SRA";
    case RV32I_SLT:    return "SLT";
    case RV32I_SLTU:   return "SLTU";
    case RV32I_ADDI:   return "ADDI";
    case RV32I_XORI:   return "XORI";
    case RV32I_ORI:    return "ORI";
    case RV32I_ANDI:   return "ANDI";
    case RV32I_SLLI:   return "SLLI";
    case RV32I_SRLI:   return "SRLI";
    case RV32I_SRAI:   return "SRAI";
    case RV32I_SLTI:   return "SLTI";
    case RV32I_SLTIU:  return "SLTIU";
    case RV32I_LB:     return "LB";
    case RV32I_LH:     return "LH";
    case RV32I_LW:     return "LW";
    case RV32I_LBU:    return "LBU";
    case RV32I_LHU:    return "LHU";
    case RV32I_JALR:   return "JALR";
    case RV32I_ECALL:  return "ECALL";
    case RV32I_EBREAK: return "EBREAK";
    case RV32I_SB:     return "SB";
    case RV32I_SH:     return "SH";
    case RV32I_SW:     return "SW";
    case RV32I_BEQ:    return "BEQ";
    case RV32I_BNE:    return "BNE";
    case RV32I_BLT:    return "BLT";
    case RV32I_BGE:    return "BGE";
    case RV32I_BLTU:   return "BLTU";
    case RV32I_BGEU:   return "BGEU";
    case RV32I_LUI:    return "LUI";
    case RV32I_AUIPC:  return "AUIPC";
    case RV32I_JAL:    return "JAL";
    default:           return "";
  }
}

/* ============================================================
 *  Lookup helpers
 * ============================================================ */

static inline const struct rv32ii_opcode_entry *
rv32ii_instr_from_enum(enum rv32i_instr idx)
{
  if (idx < 0 || idx >= RV32I_NUM_INSTRUCTIONS) return NULL;
  return &rv32ii_opcode_list[idx];
}

static inline const struct rv32ii_opcode_entry *
rv32ii_instr_from_label(const char *label)
{
  for(u32 i = 0; i < RV32I_NUM_INSTRUCTIONS; ++i)
  {
    if (strcmp(label, rv32ii_instr_label(i)) == 0)
      return &rv32ii_opcode_list[i];
  }
  return NULL;
}

static inline i32 rv32ii_is_instr(const char *name)
{
  return rv32ii_instr_from_label(name) != NULL;
}

static inline i32 rv32ii_is_load(enum rv32i_instr idx)
{
  return idx == RV32I_LB  || idx == RV32I_LH  || idx == RV32I_LW
      || idx == RV32I_LBU || idx == RV32I_LHU;
}

#endif /* _RV32_INSTR_H */
