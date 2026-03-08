#ifndef _PARSER_H
#define _PARSER_H

#include <rv32/rv32_instr.h>
#include <lexer.h>
#include <common.h>

#define AST_INIT_CAPACITY 8

/* Prints the AST then calls die() with the given error code
   and message.  Used for unrecoverable parser errors. */
#define parser_die(n, err, ...) \
  do                            \
  {                             \
    ast_node_print(n, 0);       \
    die(err, __VA_ARGS__);      \
  } while (0)

/* ============================================================
 *  AST node types
 * ============================================================ */

enum ast_node_type
{
  AST_INSTR,
  AST_LABEL,
  AST_LABEL_REF,
  AST_SECTION,
  AST_PROGRAM,   /* root node                          */
  AST_REG,       /* register operand   (u8  reg index) */
  AST_IMM,       /* immediate operand  (i32 value)     */
};

/* ============================================================
 *  AST node
 * ============================================================ */

struct ast_node
{
  enum ast_node_type type;

  union
  {
    struct { enum rv32i_instr instr;     } as_instr;
    struct { const char *name;           } as_label;
    struct { const char *name;           } as_label_ref;
    struct { u32 addr;                   } as_section;
    struct { u8  reg;                    } as_reg;
    struct { i32 value;                  } as_imm;
  };

  struct ast_node **children;
  u32               children_size;
  u32               capacity;
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct ast_node *ast_node_alloc(u0);
struct ast_node *ast_node_create(enum ast_node_type type);
u0               ast_node_free(struct ast_node **n);

/* ============================================================
 *  Children management
 * ============================================================ */

u0 ast_node_push(struct ast_node *parent, struct ast_node *child);

/* ============================================================
 *  Convenience constructors
 * ============================================================ */

struct ast_node *ast_node_create_instr(enum rv32i_instr instr);
struct ast_node *ast_node_create_label(const char *name);
struct ast_node *ast_node_create_label_ref(const char *name);
struct ast_node *ast_node_create_section(u32 addr);
struct ast_node *ast_node_create_reg(u8 reg);
struct ast_node *ast_node_create_imm(i32 value);

/* ============================================================
 *  Parser entry point
 * ============================================================ */

struct ast_node *parser_build(struct lexer *l);

/* ============================================================
 *  Debug
 * ============================================================ */

u0 ast_node_print(struct ast_node *n, u32 depth);
const char *ast_node_type_str(enum ast_node_type t);

#endif /* _PARSER_H */
