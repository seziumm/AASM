#ifndef _AST_NODE_H
#define _AST_NODE_H

#include <directive_look_up.h>
#include <rv32/instr.h>
#include <ast/ast_node_type.h>

#include <lexer.h>
#include <utils/common.h>

#define AST_INIT_CAPACITY 8

struct ast_node
{
  enum ast_node_type type;

  union
  {
    struct { struct instr *inst;       } as_instr;
    struct { u32 addr;                 } as_label;
    struct { u32 addr;                 } as_label_ref;
    struct { struct directive *dir;    } as_directive;
    struct { u8  reg;                  } as_reg;
    struct { i32 value;                } as_imm;
    struct {                           } as_root;
  };

  struct ast_node **children;
  u32               children_size;
  u32               capacity;
};

struct ast_node *ast_node_create(enum ast_node_type type);
u0               ast_node_free(struct ast_node **n);
u0               ast_node_push(struct ast_node *n, struct ast_node *child);


struct ast_node *ast_node_create_instr(struct instr *inst);
struct ast_node *ast_node_create_label(u0);
struct ast_node *ast_node_create_label_ref(u0);
struct ast_node *ast_node_create_directive(struct directive *dir);
struct ast_node *ast_node_create_reg(u8 reg);
struct ast_node *ast_node_create_imm(i32 value);
struct ast_node *ast_node_create_root(u0);

u0 ast_node_print(struct ast_node *n, u32 depth);

#endif /* _AST_NODE_H */

