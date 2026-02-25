#ifndef _RV32_TYPE_H
#define _RV32_TYPE_H

#include <common.h>
#include <stdatomic.h>
#include <stddef.h>
#include <string.h>
#include <type.h>

enum instr_t
{
  R_TYPE,
  I_TYPE,
  S_TYPE,
  B_TYPE,
  U_TYPE,
  J_TYPE,
};


struct instr
{
  const char *label;
  enum instr_t type;
  u32 code;
};



// -----------------------------
// Macro per costruire istruzioni RV32
// -----------------------------

// --- R-TYPE ---
#define INSTR_R_ENCODE(funct7, funct3, opcode, rd, rs1, rs2) \
    (((funct7 & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | \
     ((funct3 & 0x07) << 12) | ((rd & 0x1F) << 7) | (opcode & 0x7F))

// --- I-TYPE ---
#define INSTR_I_ENCODE(funct3, opcode, rd, rs1, imm) \
    (((imm & 0xFFF) << 20) | ((rs1 & 0x1F) << 15) | ((funct3 & 0x07) << 12) | \
     ((rd & 0x1F) << 7) | (opcode & 0x7F))

// --- S-TYPE ---
#define INSTR_S_ENCODE(funct3, opcode, rs1, rs2, imm) \
    ((((imm >> 5) & 0x7F) << 25) | ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | \
     ((funct3 & 0x07) << 12) | ((imm & 0x1F) << 7) | (opcode & 0x7F))

// --- B-TYPE ---
#define INSTR_B_ENCODE(funct3, opcode, rs1, rs2, imm) \
    ((((imm >> 12) & 0x1) << 31) | (((imm >> 5) & 0x3F) << 25) | \
     ((rs2 & 0x1F) << 20) | ((rs1 & 0x1F) << 15) | ((funct3 & 0x07) << 12) | \
     (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 0x1) << 7) | (opcode & 0x7F))

// --- U-TYPE ---
#define INSTR_U_ENCODE(opcode, rd, imm) \
    ((imm & 0xFFFFF000) | ((rd & 0x1F) << 7) | (opcode & 0x7F))

// --- J-TYPE ---
#define INSTR_J_ENCODE(opcode, rd, imm) \
    ((((imm >> 20) & 0x1) << 31) | (((imm >> 1) & 0x3FF) << 21) | \
     (((imm >> 11) & 0x1) << 20) | ((imm & 0xFF000) ) | ((rd & 0x1F) << 7) | (opcode & 0x7F))


// R-Type: rd, rs1, rs2 = 0 di default
#define INSTR_R_INIT_NO_REG(label_str, funct7_val, funct3_val, opcode_val) \
    (struct instr){ .label = (label_str), .type = R_TYPE, \
                    .code = INSTR_R_ENCODE(funct7_val, funct3_val, opcode_val, 0, 0, 0) }

// I-Type: rd, rs1 = 0, imm = 0 di default
#define INSTR_I_INIT_NO_REG(label_str, funct3_val, opcode_val, imm_val) \
    (struct instr){ .label = (label_str), .type = I_TYPE, \
                    .code = INSTR_I_ENCODE(funct3_val, opcode_val, 0, 0, imm_val) }

// S-Type: rs1, rs2 = 0, imm = 0 di default
#define INSTR_S_INIT_NO_REG(label_str, funct3_val, opcode_val, imm_val) \
    (struct instr){ .label = (label_str), .type = S_TYPE, \
                    .code = INSTR_S_ENCODE(funct3_val, opcode_val, 0, 0, imm_val) }

// B-Type: rs1, rs2 = 0, imm = 0 di default
#define INSTR_B_INIT_NO_REG(label_str, funct3_val, opcode_val, imm_val) \
    (struct instr){ .label = (label_str), .type = B_TYPE, \
                    .code = INSTR_B_ENCODE(funct3_val, opcode_val, 0, 0, imm_val) }

// U-Type: rd = 0, imm = 0 di default
#define INSTR_U_INIT_NO_REG(label_str, opcode_val, imm_val) \
    (struct instr){ .label = (label_str), .type = U_TYPE, \
                    .code = INSTR_U_ENCODE(opcode_val, 0, imm_val) }

// J-Type: rd = 0, imm = 0 di default
#define INSTR_J_INIT_NO_REG(label_str, opcode_val, imm_val) \
    (struct instr){ .label = (label_str), .type = J_TYPE, \
                    .code = INSTR_J_ENCODE(opcode_val, 0, imm_val) }

static const struct instr rv32ii[] = 
{
    // R-Type
    INSTR_R_INIT_NO_REG("ADD",  0x00, 0x0, 0x33),
    INSTR_R_INIT_NO_REG("SUB",  0x20, 0x0, 0x33),
    INSTR_R_INIT_NO_REG("XOR",  0x00, 0x4, 0x33),
    INSTR_R_INIT_NO_REG("OR",   0x00, 0x6, 0x33),
    INSTR_R_INIT_NO_REG("AND",  0x00, 0x7, 0x33),
    INSTR_R_INIT_NO_REG("SLL",  0x00, 0x1, 0x33),
    INSTR_R_INIT_NO_REG("SRL",  0x00, 0x5, 0x33),
    INSTR_R_INIT_NO_REG("SRA",  0x20, 0x5, 0x33),
    INSTR_R_INIT_NO_REG("SLT",  0x00, 0x2, 0x33),
    INSTR_R_INIT_NO_REG("SLTU", 0x00, 0x3, 0x33),

    // I-Type
    INSTR_I_INIT_NO_REG("ADDI", 0x0, 0x13, 0),
    INSTR_I_INIT_NO_REG("XORI", 0x4, 0x13, 0),
    INSTR_I_INIT_NO_REG("ORI",  0x6, 0x13, 0),
    INSTR_I_INIT_NO_REG("ANDI", 0x7, 0x13, 0),
    INSTR_I_INIT_NO_REG("SLLI", 0x1, 0x13, 0),
    INSTR_I_INIT_NO_REG("SRLI", 0x5, 0x13, 0),
    INSTR_I_INIT_NO_REG("SRAI", 0x5, 0x13, 0x20),
    INSTR_I_INIT_NO_REG("SLTI", 0x2, 0x13, 0),
    INSTR_I_INIT_NO_REG("SLTIU",0x3, 0x13, 0),
    INSTR_I_INIT_NO_REG("LB",   0x0, 0x03, 0),
    INSTR_I_INIT_NO_REG("LH",   0x1, 0x03, 0),
    INSTR_I_INIT_NO_REG("LW",   0x2, 0x03, 0),
    INSTR_I_INIT_NO_REG("LBU",  0x4, 0x03, 0),
    INSTR_I_INIT_NO_REG("LHU",  0x5, 0x03, 0),
    INSTR_I_INIT_NO_REG("JALR", 0x0, 0x67, 0),
    INSTR_I_INIT_NO_REG("ECALL",0x0, 0x73, 0),
    INSTR_I_INIT_NO_REG("EBREAK",0x0, 0x73, 1),

    // S-Type
    INSTR_S_INIT_NO_REG("SB", 0x0, 0x23, 0),
    INSTR_S_INIT_NO_REG("SH", 0x1, 0x23, 0),
    INSTR_S_INIT_NO_REG("SW", 0x2, 0x23, 0),

    // B-Type
    INSTR_B_INIT_NO_REG("BEQ", 0x0, 0x63, 0),
    INSTR_B_INIT_NO_REG("BNE", 0x1, 0x63, 0),
    INSTR_B_INIT_NO_REG("BLT", 0x4, 0x63, 0),
    INSTR_B_INIT_NO_REG("BGE", 0x5, 0x63, 0),
    INSTR_B_INIT_NO_REG("BLTU",0x6, 0x63, 0),
    INSTR_B_INIT_NO_REG("BGEU",0x7, 0x63, 0),

    // U-Type
    INSTR_U_INIT_NO_REG("LUI",  0x37, 0),
    INSTR_U_INIT_NO_REG("AUIPC",0x17, 0),

    // J-Type
    INSTR_J_INIT_NO_REG("JAL", 0x6F, 0),

    // Pseudo-istruzioni (gestite dall'assembler)
    { .label = "MV",  .type = I_TYPE, .code = 0 },
    { .label = "RET", .type = I_TYPE, .code = 0 }
};




static i32 is_register(char *p)
{
  /* ---------- ABI in ordine di x0..x31 ---------- */
  static char *abi[] = 
  {
    "ZERO","RA","SP","GP","TP",      /* x0..x4  */
    "T0","T1","T2",                  /* x5..x7  */
    "S0","S1",                       /* x8..x9  */
    "A0","A1","A2","A3","A4","A5","A6","A7", /* x10..x17 */
    "S2","S3","S4","S5","S6","S7","S8","S9","S10","S11", /* x18..x27 */
    "T3","T4","T5","T6"              /* x28..x31 */
  };

  for (u32 i = 0; i < sizeof(abi)/sizeof(*abi); i++)
  {
    if (strcmp(p, abi[i]) == 0) return 1;
  }

  /* ---------- X0..X31 ---------- */
  static char *reg[] = 
  {
    "X0","X1","X2","X3","X4","X5","X6","X7","X8","X9",
    "X10","X11","X12","X13","X14","X15","X16","X17","X18","X19",
    "X20","X21","X22","X23","X24","X25","X26","X27","X28","X29",
    "X30","X31"
  };

  for (u32 i = 0; i < sizeof(reg)/sizeof(*reg); i++)
  {
    if (strcmp(p, reg[i]) == 0) return 1;
  }

  return 0;
}

static i32 is_instr(char *p)
{

  for (u32 i = 0; i < sizeof(rv32ii) / sizeof(*rv32ii); i++)
  {
    if (strcmp(p, rv32ii[i].label) == 0) return 1;
  }

  return 0;
}

static const struct instr *get_instr(char *p)
{
  for (u32 i = 0; i < sizeof(rv32ii) / sizeof(*rv32ii); i++)
  {
    if (strcmp(p, rv32ii[i].label) == 0) return &rv32ii[i];
  }

  return NULL;

}

static enum instr_t get_type(char *instr)
{
  struct instr *ins = get_instr(instr);

  if(NULL == ins)
  {
    die(1, "BAD INSTR TYPE WHEN CALLILNG GET_TYPE");
  }

  return ins->type;

}

#endif
