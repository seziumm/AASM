#include <sa.h>
#include <rv32/rv32ii.h>
#include <stdlib.h>
#include <stdio.h>

/* ======================================================================
   HELPERS
   ====================================================================== */

i32 sa_is_type(struct ast_node *node, enum ast_node_type type)
{
  if (NULL == node) return 0;
  return node->type == type;
}

/* ======================================================================
   OPERAND CHECKS
   ====================================================================== */

static u0 sa_check_operand_register(struct ast_node *node)
{
  if (!sa_is_type(node, AST_OP_REGISTER))
    sa_die(1, "SA: expected AST_OP_REGISTER");

  if (!rv32ii_is_register(node->data.op_register.reg))
    sa_die(1, "SA: unknown register '%s'", node->data.op_register.reg);
}

static u0 sa_check_operand_immediate(struct ast_node *node)
{
  if (!sa_is_type(node, AST_OP_IMMEDIATE))
    sa_die(1, "SA: expected AST_OP_IMMEDIATE");
  /* range checks per tipo istruzione vengono fatti in sa_check_instr */
}

static u0 sa_check_operand_memory(struct ast_node *node)
{
  if (!sa_is_type(node, AST_OP_MEMORY))
    sa_die(1, "SA: expected AST_OP_MEMORY");

  if (!rv32ii_is_register(node->data.op_memory.base))
    sa_die(1, "SA: unknown base register '%s' in memory operand",
           node->data.op_memory.base);
}

static u0 sa_check_operand_label_ref(struct ast_node *node)
{
  if (!sa_is_type(node, AST_OP_LABEL_REF))
    sa_die(1, "SA: expected AST_OP_LABEL_REF");

  if (node->data.op_label_ref.name == NULL ||
      node->data.op_label_ref.name[0] == '\0')
    sa_die(1, "SA: empty label reference");
}

/* ======================================================================
   INSTRUCTION CHECKS
   operand counts and types are validated against the RV32I ISA table
   ====================================================================== */

/*
 * Returns 1 if the node is one of the accepted operand types listed in
 * `types` (array of length `n`).
 */
static i32 operand_is_one_of(struct ast_node *op,
                              const enum ast_node_type *types, u32 n)
{
  for (u32 i = 0; i < n; i++)
    if (sa_is_type(op, types[i])) return 1;
  return 0;
}

u0 sa_check_instr(struct ast_node *node)
{
  if (!sa_is_type(node, AST_INSTRUCTION))
    sa_die(1, "SA: expected AST_INSTRUCTION");

  const char *mn = node->data.instruction.mnemonic;
  const struct rv32ii_opcode_entry_t *entry = rv32ii_instr_from_label(mn);

  if (entry == NULL)
    sa_die(1, "SA: unknown mnemonic '%s'", mn);

  u32 nops = node->children_size;

  switch (entry->type)
  {
    /* R-type:  rd, rs1, rs2 */
    case R_TYPE:
    {
      if (nops != 3)
        sa_die(1, "SA: '%s' expects 3 operands, got %u", mn, nops);
      sa_check_operand_register(node->children[0]);
      sa_check_operand_register(node->children[1]);
      sa_check_operand_register(node->children[2]);
      break;
    }

    case I_TYPE:
    {
      /* ECALL / EBREAK: no operands */
      if (entry->index == RV32I_ECALL || entry->index == RV32I_EBREAK)
      {
        if (nops != 0)
          sa_die(1, "SA: '%s' expects 0 operands, got %u", mn, nops);
        break;
      }

      /* shift-immediate (SLLI / SRLI / SRAI):  rd, rs1, shamt[0..31] */
      if (entry->index == RV32I_SLLI ||
          entry->index == RV32I_SRLI ||
          entry->index == RV32I_SRAI)
      {
        if (nops != 3)
          sa_die(1, "SA: '%s' expects 3 operands, got %u", mn, nops);
        sa_check_operand_register (node->children[0]);
        sa_check_operand_register (node->children[1]);
        sa_check_operand_immediate(node->children[2]);
        i32 shamt = node->children[2]->data.op_immediate.value;
        if (shamt < 0 || shamt > 31)
          sa_die(1, "SA: '%s' shamt %d out of range [0, 31]", mn, shamt);
        break;
      }

      /* Load / JALR:  rd, disp(base) */
      if (entry->index == RV32I_LB  || entry->index == RV32I_LH  ||
          entry->index == RV32I_LW  || entry->index == RV32I_LBU ||
          entry->index == RV32I_LHU || entry->index == RV32I_JALR)
      {
        if (nops != 2)
          sa_die(1, "SA: '%s' expects 2 operands (rd, mem), got %u", mn, nops);
        sa_check_operand_register(node->children[0]);
        sa_check_operand_memory  (node->children[1]);
        i32 imm = node->children[1]->data.op_memory.displacement;
        if (imm < -2048 || imm > 2047)
          sa_die(1, "SA: '%s' displacement %d out of range [-2048, 2047]", mn, imm);
        break;
      }

      /* arithmetic immediate:  rd, rs1, imm12 */
      if (nops != 3)
        sa_die(1, "SA: '%s' expects 3 operands, got %u", mn, nops);
      sa_check_operand_register (node->children[0]);
      sa_check_operand_register (node->children[1]);
      sa_check_operand_immediate(node->children[2]);
      i32 imm = node->children[2]->data.op_immediate.value;
      if (imm < -2048 || imm > 2047)
        sa_die(1, "SA: '%s' immediate %d out of range [-2048, 2047]", mn, imm);
      break;
    }

    /* S-type:  rs2, disp(rs1) */
    case S_TYPE:
    {
      if (nops != 2)
        sa_die(1, "SA: '%s' expects 2 operands (rs2, mem), got %u", mn, nops);
      sa_check_operand_register(node->children[0]);
      sa_check_operand_memory  (node->children[1]);
      i32 imm = node->children[1]->data.op_memory.displacement;
      if (imm < -2048 || imm > 2047)
        sa_die(1, "SA: '%s' displacement %d out of range [-2048, 2047]", mn, imm);
      break;
    }

    /* B-type:  rs1, rs2, label_ref */
    case B_TYPE:
    {
      if (nops != 3)
        sa_die(1, "SA: '%s' expects 3 operands, got %u", mn, nops);

      sa_check_operand_register  (node->children[0]);
      sa_check_operand_register  (node->children[1]);
      sa_check_operand_label_ref (node->children[2]);
      break;
    }

    /* U-type:  rd, imm20 */
    case U_TYPE:
    {
      if (nops != 2)
        sa_die(1, "SA: '%s' expects 2 operands, got %u", mn, nops);

      sa_check_operand_register (node->children[0]);
      sa_check_operand_immediate(node->children[1]);

      i32 imm = node->children[1]->data.op_immediate.value;
      if (imm < 0 || imm > 0xFFFFF)
        sa_die(1, "SA: '%s' immediate %d out of range [0, 0xFFFFF]", mn, imm);
      break;
    }

    /* J-type:  rd, label_ref */
    case J_TYPE:
    {
      if (nops != 2)
        sa_die(1, "SA: '%s' expects 2 operands, got %u", mn, nops);

      sa_check_operand_register  (node->children[0]);
      sa_check_operand_label_ref (node->children[1]);
      break;
    }

    default:
      sa_die(1, "SA: unhandled instruction type for '%s'", mn);
  }
}

/* ======================================================================
   LABEL CHECK
   ====================================================================== */

u0 sa_check_label(struct ast_node *node, u32 *lc)
{
  if (!sa_is_type(node, AST_LABEL))
    sa_die(1, "SA: expected AST_LABEL");

  if (node->data.label.name == NULL || node->data.label.name[0] == '\0')
    sa_die(1, "SA: label has empty name");

  for (u32 i = 0; i < node->children_size; i++)
  {
    if (!sa_is_type(node->children[i], AST_INSTRUCTION))
      sa_die(1, "SA: non-instruction child inside label '%s'",
             node->data.label.name);

    sa_check_instr(node->children[i]);
    *lc += 4; /* ogni istruzione occupa 4 byte */
  }
}

/* ======================================================================
   SECTION CHECK
   ====================================================================== */

u0 sa_check_section(struct ast_node *node, u32 *lc)
{
  if (!sa_is_type(node, AST_SECTION))
    sa_die(1, "SA: expected AST_SECTION");

  if (node->data.section.value == NULL)
    sa_die(1, "SA: section has no address value");

  u32 section_addr = (u32)strtoul(node->data.section.value, NULL, 0);

  if (*lc > section_addr)
    sa_die(1, "SA: location counter 0x%X exceeds section address 0x%X",
           *lc, section_addr);

  /* salta al nuovo indirizzo di sezione */
  *lc = section_addr;

  for (u32 i = 0; i < node->children_size; i++)
  {
    if (!sa_is_type(node->children[i], AST_LABEL))
      sa_die(1, "SA: expected AST_LABEL as child of SECTION");

    sa_check_label(node->children[i], lc);
  }
}

/* ======================================================================
   PROGRAM CHECK
   ====================================================================== */

u0 sa_check_program(struct ast_node *node, u32 *lc)
{
  if (!sa_is_type(node, AST_PROGRAM))
    sa_die(1, "SA: expected AST_PROGRAM");

  for (u32 i = 0; i < node->children_size; i++)
  {
    sa_check_section(node->children[i], lc);
  }
}

/* ======================================================================
   ENTRY POINT
   ====================================================================== */

u0 sa_check(struct ast_node *node)
{
  u32 lc = 0;
  sa_check_program(node, &lc);
}
