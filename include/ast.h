#ifndef _AST_H
#define _AST_H

#include "lexer.h"
#include <stdlib.h>
#include <type.h>
#include <stddef.h>
#include <common.h>

#define ast_die(l, err, ...)     \
  do {                           \
  /*lexer_print(l);*/            \
    die(err, __VA_ARGS__);       \
  } while (0)

#define AST_NODE_INIT_CAPACITY 4

enum ast_node_type 
{
  AST_PROGRAM,
  AST_SECTION,
  AST_DIRECTIVE,
  AST_LABEL,
  AST_INSTRUCTION,
  AST_OPERAND,

  /* tipi di operando */
  AST_OP_REGISTER,
  AST_OP_IMMEDIATE,
  AST_OP_MEMORY,
  AST_OP_LABEL_REF
};


struct ast_node ast_node;

struct ast_node 
{
  enum ast_node_type type;

  struct ast_node **children;
  u32 children_size;
  u32 children_capacity;

  union 
  {

    struct {} program;

    struct {} section;

    struct 
    {
      char *name;
    } label;

    /* INSTRUCTION */
    struct 
    {
      char *mnemonic;   // "mov", "add", ecc.
    } instruction;

    struct 
    {
      char *reg;        // "eax"
    } op_register;

    /* OPERAND - IMMEDIATE */
    struct 
    {
      i32 value;
    } op_immediate;

    /* OPERAND - MEMORY */
    struct 
    {
      char *base;
      i32 displacement; /* offset immediato */
    } op_memory;

    // /* OPERAND - LABEL REF */
    // struct 
    // {
    //   char *label;
    // } op_label_ref;

  } data;
};


static void ast_node_expand_children(struct ast_node *node)
{
  u32 old_capacity = node->children_capacity;

  u32 new_capacity = (old_capacity == 0)
    ? AST_NODE_INIT_CAPACITY
    : old_capacity * 2;

  struct ast_node **new_children = malloc(new_capacity * sizeof(struct ast_node *));

  if (new_children == NULL)
  {
    ast_die(NULL, 1, "A malloc() error occurred");
  }

  for (u32 i = 0; i < node->children_size; i++)
  {
    new_children[i] = node->children[i];
  }

  free(node->children);

  node->children = new_children;
  node->children_capacity = new_capacity;
}

void ast_node_push_children(struct ast_node *node, struct ast_node *child)
{
  if (node == NULL || child == NULL) return;

  if (node->children_size >= node->children_capacity)
  {
    ast_node_expand_children(node);
  }

  node->children[node->children_size] = child;
  node->children_size++;
}

struct ast_node *ast_node_alloc(enum ast_node_type type, u32 children_size, struct ast_node **children)
{
  struct ast_node *node = malloc(sizeof(struct ast_node));

  if (node == NULL)
  {
    ast_die(NULL, 1, "A malloc() error occurred");
  }

  node->type = type;
  node->children_size = children_size;

  node->children_capacity = (children_size > AST_NODE_INIT_CAPACITY) 
    ? children_size 
    : AST_NODE_INIT_CAPACITY;

  if(node->children_capacity == 0)
  {
    node->children = NULL;
    return node;
  }

  node->children = malloc(node->children_capacity * sizeof(struct ast_node *));

  if (node->children == NULL)
  {
    ast_die(NULL, 1, "A malloc() error occurred");
  }

  for (u32 i = 0; i < children_size; i++)
  {
    node->children[i] = children[i];
  }

  return node;
}

struct ast_node *ast_create(struct lexer *l, u32 *pos)
{
  if(*pos >= l->size) return NULL;

  struct ast_node *node = NULL;

  switch (l->tokens[*pos]->type) 
  {
    default:
      break;
  
  }

}

struct ast_node *ast_compile(struct lexer *l)
{
  u32 pos = 0;
  return ast_create(l, &pos);

}

#endif
