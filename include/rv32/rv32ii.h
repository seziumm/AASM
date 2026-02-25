#ifndef _RV32_TYPE_H
#define _RV32_TYPE_H

#include <common.h>
#include <stdatomic.h>
#include <stddef.h>
#include <string.h>
#include <type.h>

enum instr_type_t
{
  R_TYPE,
  I_TYPE,
  S_TYPE,
  B_TYPE,
  U_TYPE,
  J_TYPE
};

enum rv32i_instr_e
{
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
  RV32I_ADDI,
  RV32I_XORI,
  RV32I_ORI,
  RV32I_ANDI,
  RV32I_SLLI,
  RV32I_SRLI,
  RV32I_SRAI,
  RV32I_SLTI,
  RV32I_SLTIU,
  RV32I_LB,
  RV32I_LH,
  RV32I_LW,
  RV32I_LBU,
  RV32I_LHU,
  RV32I_JALR,
  RV32I_ECALL,
  RV32I_EBREAK,
  RV32I_SB,
  RV32I_SH,
  RV32I_SW,
  RV32I_BEQ,
  RV32I_BNE,
  RV32I_BLT,
  RV32I_BGE,
  RV32I_BLTU,
  RV32I_BGEU,
  RV32I_LUI,
  RV32I_AUIPC,
  RV32I_JAL,
  RV32I_NUM_INSTRUCTIONS
};

// --- Union dei campi di una istruzione ---
union rv32_instr_fields_t
{
  struct { u8 funct7, funct3, opcode, rd, rs1, rs2; }   r;
  struct { u8 funct3, opcode, rd, rs1; i32 imm; }       i;
  struct { u8 funct3, opcode, rs1, rs2; i32 imm; }      s;
  struct { u8 funct3, opcode, rs1, rs2; i32 imm; }      b;
  struct { u8 opcode, rd; i32 imm; }                    u;
  struct { u8 opcode, rd; i32 imm; }                    j;
} ;

// --- Struct finale per una entry opcode completa (senza code) ---
struct rv32ii_opcode_entry_t
{
  const char*                    label;     // Mnemonico (es "ADD")
  enum rv32i_instr_e             index;     // Enum/indice
  enum instr_type_t              type;      // Tipo formato
  union rv32_instr_fields_t      fields;    // Campi
};

// --- Macro encoding helpers ---
#define RV32_ENC_R(funct7, funct3, opcode, rd, rs1, rs2) \
  (((u32)(funct7) & 0x7F) << 25 | ((u32)(rs2) & 0x1F) << 20 | ((u32)(rs1) & 0x1F) << 15 | \
   ((u32)(funct3) & 0x07) << 12 | ((u32)(rd) & 0x1F) << 7 | ((u32)(opcode) & 0x7F))

#define RV32_ENC_I(funct3, opcode, rd, rs1, imm) \
  (((u32)(imm) & 0xFFF) << 20 | ((u32)(rs1) & 0x1F) << 15 | ((u32)(funct3) & 0x07) << 12 | \
   ((u32)(rd) & 0x1F) << 7 | ((u32)(opcode) & 0x7F))

#define RV32_ENC_S(funct3, opcode, rs1, rs2, imm) \
  ((((u32)(imm) >> 5) & 0x7F) << 25 | ((u32)(rs2) & 0x1F) << 20 | ((u32)(rs1) & 0x1F) << 15 | \
   ((u32)(funct3) & 0x07) << 12 | ((u32)(imm) & 0x1F) << 7 | ((u32)(opcode) & 0x7F))

#define RV32_ENC_B(funct3, opcode, rs1, rs2, imm) \
  ((((u32)(imm) >> 12) & 0x1) << 31 | (((u32)(imm) >> 5) & 0x3F) << 25 | \
   ((u32)(rs2) & 0x1F) << 20 | ((u32)(rs1) & 0x1F) << 15 | ((u32)(funct3) & 0x07) << 12 | \
   (((u32)(imm) >> 1) & 0xF) << 8 | (((u32)(imm) >> 11) & 0x1) << 7 | ((u32)(opcode) & 0x7F))

#define RV32_ENC_U(opcode, rd, imm) \
  (((u32)(imm) & 0xFFFFF000) | ((u32)(rd) & 0x1F) << 7 | ((u32)(opcode) & 0x7F))

#define RV32_ENC_J(opcode, rd, imm) \
  ((((u32)(imm) >> 20) & 0x1) << 31 | (((u32)(imm) >> 1) & 0x3FF) << 21 | \
   (((u32)(imm) >> 11) & 0x1) << 20 | ((u32)(imm) & 0xFF000) | ((u32)(rd) & 0x1F) << 7 | ((u32)(opcode) & 0x7F))

// --- Macro initializer compatibile con la nuova struct ---
#define RV32II_ENTRY(label, idx, type, fields_init) \
{ .label = label, .index = idx, .type = type, .fields = fields_init }

// --- Tabella istruzioni RV32I come array di struct (direttamente wrappato) ---
static const struct rv32ii_opcode_entry_t rv32ii_opcode_list[RV32I_NUM_INSTRUCTIONS] = {
  /* R-type arithmetic */
  { "ADD",    RV32I_ADD,   R_TYPE,   { .r = {0x00, 0x0, 0x33, 0,0,0} } },
  { "SUB",    RV32I_SUB,   R_TYPE,   { .r = {0x20, 0x0, 0x33, 0,0,0} } },
  { "XOR",    RV32I_XOR,   R_TYPE,   { .r = {0x00, 0x4, 0x33, 0,0,0} } },
  { "OR",     RV32I_OR,    R_TYPE,   { .r = {0x00, 0x6, 0x33, 0,0,0} } },
  { "AND",    RV32I_AND,   R_TYPE,   { .r = {0x00, 0x7, 0x33, 0,0,0} } },
  { "SLL",    RV32I_SLL,   R_TYPE,   { .r = {0x00, 0x1, 0x33, 0,0,0} } },
  { "SRL",    RV32I_SRL,   R_TYPE,   { .r = {0x00, 0x5, 0x33, 0,0,0} } },
  { "SRA",    RV32I_SRA,   R_TYPE,   { .r = {0x20, 0x5, 0x33, 0,0,0} } },
  { "SLT",    RV32I_SLT,   R_TYPE,   { .r = {0x00, 0x2, 0x33, 0,0,0} } },
  { "SLTU",   RV32I_SLTU,  R_TYPE,   { .r = {0x00, 0x3, 0x33, 0,0,0} } },
  /* I-type arithmetic (immediate) */
  { "ADDI",   RV32I_ADDI,  I_TYPE,   { .i = {0x0,  0x13, 0, 0, 0} } },
  { "XORI",   RV32I_XORI,  I_TYPE,   { .i = {0x4,  0x13, 0, 0, 0} } },
  { "ORI",    RV32I_ORI,   I_TYPE,   { .i = {0x6,  0x13, 0, 0, 0} } },
  { "ANDI",   RV32I_ANDI,  I_TYPE,   { .i = {0x7,  0x13, 0, 0, 0} } },
  { "SLLI",   RV32I_SLLI,  I_TYPE,   { .i = {0x1,  0x13, 0, 0, 0} } },
  { "SRLI",   RV32I_SRLI,  I_TYPE,   { .i = {0x5,  0x13, 0, 0, 0} } },
  { "SRAI",   RV32I_SRAI,  I_TYPE,   { .i = {0x5,  0x13, 0, 0, 0x20} } },
  { "SLTI",   RV32I_SLTI,  I_TYPE,   { .i = {0x2,  0x13, 0, 0, 0} } },
  { "SLTIU",  RV32I_SLTIU, I_TYPE,   { .i = {0x3,  0x13, 0, 0, 0} } },
  /* Load I-type */
  { "LB",     RV32I_LB,    I_TYPE,   { .i = {0x0,  0x03, 0, 0, 0} } },
  { "LH",     RV32I_LH,    I_TYPE,   { .i = {0x1,  0x03, 0, 0, 0} } },
  { "LW",     RV32I_LW,    I_TYPE,   { .i = {0x2,  0x03, 0, 0, 0} } },
  { "LBU",    RV32I_LBU,   I_TYPE,   { .i = {0x4,  0x03, 0, 0, 0} } },
  { "LHU",    RV32I_LHU,   I_TYPE,   { .i = {0x5,  0x03, 0, 0, 0} } },
  /* I-type Jump and System */
  { "JALR",   RV32I_JALR,  I_TYPE,   { .i = {0x0,  0x67, 0, 0, 0} } },
  { "ECALL",  RV32I_ECALL, I_TYPE,   { .i = {0x0,  0x73, 0, 0, 0} } },
  { "EBREAK", RV32I_EBREAK,I_TYPE,   { .i = {0x0,  0x73, 0, 0, 1} } },
  /* S-type Store */
  { "SB",     RV32I_SB,    S_TYPE,   { .s = {0x0, 0x23, 0, 0, 0} } },
  { "SH",     RV32I_SH,    S_TYPE,   { .s = {0x1, 0x23, 0, 0, 0} } },
  { "SW",     RV32I_SW,    S_TYPE,   { .s = {0x2, 0x23, 0, 0, 0} } },
  /* B-type branch */
  { "BEQ",    RV32I_BEQ,   B_TYPE,   { .b = {0x0, 0x63, 0, 0, 0} } },
  { "BNE",    RV32I_BNE,   B_TYPE,   { .b = {0x1, 0x63, 0, 0, 0} } },
  { "BLT",    RV32I_BLT,   B_TYPE,   { .b = {0x4, 0x63, 0, 0, 0} } },
  { "BGE",    RV32I_BGE,   B_TYPE,   { .b = {0x5, 0x63, 0, 0, 0} } },
  { "BLTU",   RV32I_BLTU,  B_TYPE,   { .b = {0x6, 0x63, 0, 0, 0} } },
  { "BGEU",   RV32I_BGEU,  B_TYPE,   { .b = {0x7, 0x63, 0, 0, 0} } },
  /* U-type upper immediate */
  { "LUI",    RV32I_LUI,   U_TYPE,   { .u = {0x37, 0, 0} } },
  { "AUIPC",  RV32I_AUIPC, U_TYPE,   { .u = {0x17, 0, 0} } },
  /* J-type jump */
  { "JAL",    RV32I_JAL,   J_TYPE,   { .j = {0x6F, 0, 0} } }
};

static inline const struct rv32ii_opcode_entry_t* rv32ii_instr_from_enum(enum rv32i_instr_e idx) 
{
  if (idx < 0 || idx >= RV32I_NUM_INSTRUCTIONS) return NULL;
  return &rv32ii_opcode_list[idx];
}

static inline const struct rv32ii_opcode_entry_t* rv32ii_instr_from_label(const char* label) 
{
  for (u32 i = 0; i < RV32I_NUM_INSTRUCTIONS; ++i)
  {
    if (strcmp(label, rv32ii_opcode_list[i].label) == 0)
      return &rv32ii_opcode_list[i];

  }
  return NULL;
}

static inline int rv32ii_is_instr(const char* name) 
{
  return rv32ii_instr_from_label(name) != NULL;
}

static inline int rv32ii_is_register(const char* name) 
{
  static const char* reg_names[] = {
    "ZERO",
    "RA",
    "SP",
    "GP",
    "TP",
    "T0",
    "T1",
    "T2",
    "T3",
    "T4",
    "T5",
    "T6",
    "S0",
    "S1",
    "S2",
    "S3",
    "S4",
    "S5",
    "S6",
    "S7",
    "S8",
    "S9",
    "S10",
    "S11",
    "A0",
    "A1",
    "A2",
    "A3",
    "A4",
    "A5",
    "A6",
    "A7",
    // X form
    "X0",
    "X1",
    "X2",
    "X3",
    "X4",
    "X5",
    "X6",
    "X7",
    "X8",
    "X9",
    "X10",
    "X11",
    "X12",
    "X13",
    "X14",
    "X15",
    "X16",
    "X17",
    "X18",
    "X19",
    "X20",
    "X21",
    "X22",
    "X23",
    "X24",
    "X25",
    "X26",
    "X27",
    "X28",
    "X29",
    "X30",
    "X31"
  };

  for(u32 i = 0; i < sizeof(reg_names)/sizeof(reg_names[0]); ++i)
  {
    if(strcmp(name, reg_names[i]) == 0) return 1;

  }
  return 0;
}
#endif
