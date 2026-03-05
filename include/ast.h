#ifndef _AST_H
#define _AST_H

#include <lexer.h>
#include <type.h>

#define ast_die(l, err, ...)     \
  do                             \
  {                              \
    die(err, __VA_ARGS__);       \
  } while (0)

#define AST_NODE_INIT_CAPACITY 4

enum ast_node_type 
{
  AST_PROGRAM,
  AST_SECTION,
  AST_LABEL,
  AST_INSTRUCTION,
  AST_OP_REGISTER,
  AST_OP_IMMEDIATE,
  AST_OP_MEMORY,
  AST_OP_LABEL_REF
};

struct ast_node 
{
  enum ast_node_type type;
  struct ast_node  **children;
  u32 children_size;
  u32 children_capacity;

  union 
  {
    struct {} program;

    struct { char *value; } section;
    struct { char *name;  } label;

    struct { char *mnemonic; } instruction;

    struct { char *reg;      } op_register;
    struct { i32   value;    } op_immediate;
    struct { char *name;     } op_label_ref;
    struct 
    {
      char *base;
      i32   displacement;
    } op_memory;
  } data;
};

/* ---- node helpers ---- */
struct ast_node *ast_node_alloc(enum ast_node_type type,
                                u32 children_size,
                                struct ast_node **children);
void ast_node_push_children(struct ast_node *node, struct ast_node *child);
void ast_node_expand_children(struct ast_node *node);
void ast_node_free(struct ast_node *node);

/* ---- builder (params) ---- */
u32 ast_build_program_params  (struct lexer *l, u32 *pos, struct ast_node *node);
u32 ast_build_section_params  (struct lexer *l, u32 *pos, struct ast_node *node);
u32 ast_build_label_params    (struct lexer *l, u32 *pos, struct ast_node *node);
u32 ast_build_instr_params    (struct lexer *l, u32 *pos, struct ast_node *node);

/* ---- create ---- */
struct ast_node *ast_create_program    (struct lexer *l, u32 *pos);
struct ast_node *ast_create_section    (struct lexer *l, u32 *pos);
struct ast_node *ast_create_label      (struct lexer *l, u32 *pos);
struct ast_node *ast_create_instr      (struct lexer *l, u32 *pos);
struct ast_node *ast_create_operand    (struct lexer *l, u32 *pos);

/* ---- API ---- */
struct ast_node *ast_compile(struct lexer *l);
void             ast_print  (struct ast_node *node, int depth);

#endif
