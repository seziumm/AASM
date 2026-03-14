#include <ast/ast_node_type.h>
#include <directive_look_up.h>
#include "utils/common.h"
#include <ast/ast_node.h>
#include <utils/aalloc.h>
#include <stdlib.h>
#include <stdio.h>

struct ast_node *ast_node_create(enum ast_node_type type)
{
  struct ast_node *n = a_malloc(sizeof(struct ast_node));

  *n = (struct ast_node)
  {
    .children      = a_malloc(sizeof(struct ast_node *) * AST_INIT_CAPACITY),
    .type          = type,
    .children_size = 0,
    .capacity      = AST_INIT_CAPACITY
  };

  return n;

}

struct ast_node *ast_node_create_instr(struct instr *inst)
{
  struct ast_node *n = ast_node_create(AST_INSTR);
  n->as_instr.inst = inst;
  return n;
}

struct ast_node *ast_node_create_label(u0)
{
  struct ast_node *n = ast_node_create(AST_LABEL);
  n->as_label.addr = 0; /* resolved later */
  return n;
}

struct ast_node *ast_node_create_label_ref(u0)
{
  struct ast_node *n = ast_node_create(AST_LABEL_REF);
  n->as_label_ref.addr = 0; /* resolved later */
  return n;
}

struct ast_node *ast_node_create_directive(struct directive *dir)
{
  struct ast_node *n = ast_node_create(AST_DIRECTIVE);
  n->as_directive.dir = dir;
  return n;
}

struct ast_node *ast_node_create_reg(u8 reg)
{
  struct ast_node *n = ast_node_create(AST_REG);
  n->as_reg.reg = reg;
  return n;
}

struct ast_node *ast_node_create_imm(i32 value)
{
  struct ast_node *n = ast_node_create(AST_IMM);
  n->as_imm.value = value;
  return n;
}

struct ast_node *ast_node_create_root(u0)
{
  struct ast_node *n = ast_node_create(AST_ROOT);
  return n;
}

static u0 ast_node_expand(struct ast_node *n)
{
  u32               new_capacity = n->capacity * 2;
  struct ast_node **new_children = a_realloc(n->children, new_capacity * sizeof(struct ast_node *));
  n->children = new_children;
  n->capacity = new_capacity;
}

u0 ast_node_push(struct ast_node *n, struct ast_node *child)
{
  if (NULL == n || NULL == child) return;
  if (n->children_size >= n->capacity)
    ast_node_expand(n);
  n->children[n->children_size++] = child;
}

u0 ast_node_free(struct ast_node **n)
{
  if (NULL == n || NULL == *n) return;

  for (u32 i = 0; i < (*n)->children_size; ++i)
  {
    ast_node_free(&(*n)->children[i]);
  }

  a_free((*n)->children);
  a_free(*n);
  *n = NULL;
}

u0 ast_node_print(struct ast_node *n, u32 depth)
{
  if (NULL == n) return;

  for (u32 i = 0; i < depth; ++i)
  {
    debugf("  ");
  }

  switch (n->type)
  {
    case AST_ROOT:
      debugf("[ROOT]\n");
      break;

    case AST_INSTR:
      debugf("[INSTR] %s\n", n->as_instr.inst->label ? n->as_instr.inst->label : "?");
      break;

    case AST_LABEL:
      debugf("[LABEL] addr=0x%08X\n", n->as_label.addr);
      break;

    case AST_LABEL_REF:
      debugf("[LABEL_REF] addr=0x%08X\n", n->as_label_ref.addr);
      break;

    case AST_DIRECTIVE:
      debugf("[DIRECTIVE] type=%s\n", directive_type_to_str(n->as_directive.dir->type));
      break;

    case AST_REG:
      debugf("[REG] X%u\n", n->as_reg.reg);
      break;

    case AST_IMM:
      debugf("[IMM] %d\n", n->as_imm.value);
      break;

  }

  for (u32 i = 0; i < n->children_size; ++i)
  {
    ast_node_print(n->children[i], depth + 1);
  }
}
