#include <rv32/rv32_type.h>
#include <rv32/rv32_reg.h>
#include <rv32/rv32_instr.h>
#include <parser.h>
#include <codegen.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================
 *  Local symbol table  (visible only in this translation unit)
 * ============================================================ */

#define SYM_INIT_CAPACITY 8

struct sym { const char *name; u32 addr; };

static struct sym *sym_table   = NULL;
static u32         sym_size     = 0;
static u32         sym_capacity = 0;

static u0 sym_init(u0)
{
  sym_table    = malloc(SYM_INIT_CAPACITY * sizeof(struct sym));
  sym_size     = 0;
  sym_capacity = SYM_INIT_CAPACITY;
}

static u0 sym_free(u0)
{
  free(sym_table);
  sym_table    = NULL;
  sym_size     = 0;
  sym_capacity = 0;
}

static u0 sym_push(const char *name, u32 addr)
{
  /* Ignore re-registration of the same label (pass 2 calling build_label) */
  for (u32 i = 0; i < sym_size; i++)
    if (strcmp(sym_table[i].name, name) == 0) return;

  if (sym_size >= sym_capacity)
  {
    sym_capacity *= 2;
    sym_table = realloc(sym_table, sym_capacity * sizeof(struct sym));
  }

  sym_table[sym_size++] = (struct sym){ name, addr };
}

/* Returns address or crashes if not found. */
static u32 sym_expect(const char *name, u32 pc)
{
  for (u32 i = 0; i < sym_size; i++)
    if (strcmp(sym_table[i].name, name) == 0)
      return sym_table[i].addr;

  die(1, "sym_expect: undefined label @%s at pc=0x%08X", name, pc);
  return 0; /* unreachable */
}

/* ============================================================
 *  Node type guards
 * ============================================================ */

static inline u0 expect_type(struct ast_node *node, enum ast_node_type t)
{
  if (node->type != t)
    die(1, "expect_type: expected %s but found %s",
      ast_node_type_str(t), ast_node_type_str(node->type));
}

static inline u8  expect_reg(struct ast_node *node)
{
  expect_type(node, AST_REG);
  return node->as_reg.reg;
}

static inline i32 expect_imm(struct ast_node *node)
{
  expect_type(node, AST_IMM);
  return node->as_imm.value;
}

static inline i32 expect_imm_or_ref(struct ast_node *node, u32 pc)
{
  if (node->type == AST_IMM)
    return node->as_imm.value;

  if (node->type == AST_LABEL_REF)
  {
    u32 addr = sym_expect(node->as_label_ref.name, pc);
    return (i32)(addr - pc);
  }

  die(1, "expect_imm_or_ref: expected AST_IMM or AST_LABEL_REF but found %s",
    ast_node_type_str(node->type));
  return 0; /* unreachable */
}

/* ============================================================
 *  Emit  —  write one little-endian u32 to output file
 * ============================================================ */

#define emit(word, out) fwrite(&(u32){(word)}, 4, 1, (out))

/* ============================================================
 *  Forward declaration
 * ============================================================ */

static u0 build_instr(struct ast_node *node, u32 *pc, FILE *out);

/* ============================================================
 *  Instruction encoding
 * ============================================================ */

static u0 build_instr(struct ast_node *node, u32 *pc, FILE *out)
{
  expect_type(node, AST_INSTR);

  enum rv32i_instr                  t = node->as_instr.instr;
  const struct rv32ii_opcode_entry *e = rv32ii_instr_from_enum(t);

  u8  rd  = 0;
  u8  rs1 = 0;
  u8  rs2 = 0;
  i32 imm = 0;

  if (t == RV32I_ECALL || t == RV32I_EBREAK)
  {
    emit(rv32_encode(e, 0, 0, 0, 0), out);
    *pc += 4;
    return;
  }

  switch (e->type)
  {
    case R_TYPE:
      rd  = expect_reg(node->children[0]);
      rs1 = expect_reg(node->children[1]);
      rs2 = expect_reg(node->children[2]);
      break;

    case I_TYPE:
      rd  = expect_reg(node->children[0]);
      rs1 = expect_reg(node->children[1]);
      imm = expect_imm(node->children[2]);
      break;

    case S_TYPE:
      /* children: rs2 , imm , rs1  (parse_mem pushes imm then rs1) */
      rs2 = expect_reg(node->children[0]);
      imm = expect_imm(node->children[1]);
      rs1 = expect_reg(node->children[2]);
      break;

    case B_TYPE:
      rs1 = expect_reg(node->children[0]);
      rs2 = expect_reg(node->children[1]);
      imm = expect_imm_or_ref(node->children[2], *pc);
      break;

    case U_TYPE:
      rd  = expect_reg(node->children[0]);
      imm = expect_imm(node->children[1]);
      break;

    case J_TYPE:
      rd  = expect_reg(node->children[0]);
      imm = expect_imm_or_ref(node->children[1], *pc);
      break;
  }

  emit(rv32_encode(e, rd, rs1, rs2, imm), out);
  *pc += 4;
}

/* ============================================================
 *  Label  —  register addr then encode children
 * ============================================================ */

static u0 build_label(struct ast_node *node, u32 *pc, FILE *out)
{
  expect_type(node, AST_LABEL);

  sym_push(node->as_label.name, *pc);

  for (u32 i = 0; i < node->children_size; i++)
    build_instr(node->children[i], pc, out);
}

/* ============================================================
 *  PC fill  —  emit NOPs until pc reaches target
 * ============================================================ */

static u0 fill_pc(u32 *pc, u32 target, FILE *out)
{
  if (target < *pc)
    die(1, "fill_pc: target 0x%08X is behind current pc 0x%08X", target, *pc);

  while (*pc < target)
  {
    emit(CODEGEN_RV32_NOP, out);
    *pc += 4;
  }
}

/* ============================================================
 *  Section  —  seek to addr, encode children
 * ============================================================ */

static u0 build_section(struct ast_node *node, u32 *pc, FILE *out)
{
  expect_type(node, AST_SECTION);

  fill_pc(pc, node->as_section.addr, out);

  for (u32 i = 0; i < node->children_size; i++)
  {
    struct ast_node *child = node->children[i];

    if (child->type == AST_LABEL) { build_label(child, pc, out); continue; }
    if (child->type == AST_INSTR) { build_instr(child, pc, out); continue; }

    die(1, "build_section: expected AST_LABEL or AST_INSTR but found %s",
      ast_node_type_str(child->type));
  }
}

/* ============================================================
 *  Program  —  iterate sections
 * ============================================================ */

static u0 build_program(struct ast_node *node, u32 *pc, FILE *out)
{
  expect_type(node, AST_PROGRAM);

  for (u32 i = 0; i < node->children_size; i++)
  {
    struct ast_node *child = node->children[i];

    if (child->type == AST_SECTION) { build_section(child, pc, out); continue; }

    die(1, "build_program: expected AST_SECTION but found %s",
      ast_node_type_str(child->type));
  }
}

/* ============================================================
 *  Pass 1  —  collect all label addresses (no output)
 * ============================================================ */

static u0 collect_labels_section(struct ast_node *node, u32 *pc)
{
  expect_type(node, AST_SECTION);

  if (node->as_section.addr < *pc)
    die(1, "collect_labels: section 0x%08X is behind pc 0x%08X",
        node->as_section.addr, *pc);

  /* Jump pc to section base (mirrors fill_pc without emitting) */
  *pc = node->as_section.addr;

  for (u32 i = 0; i < node->children_size; i++)
  {
    struct ast_node *child = node->children[i];

    if (child->type == AST_LABEL)
    {
      sym_push(child->as_label.name, *pc);
      /* Each instruction nested under this label is 4 bytes */
      *pc += 4 * child->children_size;
      continue;
    }

    if (child->type == AST_INSTR) { *pc += 4; continue; }

    die(1, "collect_labels: unexpected node %s",
        ast_node_type_str(child->type));
  }
}

static u0 collect_labels(struct ast_node *root)
{
  expect_type(root, AST_PROGRAM);

  u32 pc = CODEGEN_PC_INIT_ADDR;

  for (u32 i = 0; i < root->children_size; i++)
    collect_labels_section(root->children[i], &pc);
}

/* ============================================================
 *  Entry point
 * ============================================================ */

u0 codegen_compile(struct ast_node *root, const char *out_path)
{
  if (NULL == root)
    die(1, "codegen_compile: root is NULL");

  FILE *out = fopen(out_path, "wb");
  if (NULL == out)
    die(1, "codegen_compile: cannot open '%s'", out_path);

  sym_init();

  /* Pass 1: populate symbol table before any encoding (resolves forward refs) */
  collect_labels(root);

  /* Pass 2: encode instructions (all labels now known) */
  u32 pc = CODEGEN_PC_INIT_ADDR;
  build_program(root, &pc, out);

  sym_free();
  fclose(out);
}
