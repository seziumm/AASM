#ifndef _AST_H
#define _AST_H

#include <rv32/rv32_type.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <type.h>


#define ast_die(a, err, ...)         \
    do {                             \
        /* ast_print(l);*/           \
        die(err, __VA_ARGS__);       \
    } while (0)

#define AST_NODE_INIT_CAPACITY       1



struct ast_node 
{

  struct token_data *td;
  

  struct ast_node **child;
  u32 child_size;
  u32 child_capacity;

  struct ast_node *next;

};

static void ast_node_print(struct ast_node *node);

struct ast 
{
  struct ast_node *root;
  struct ast_node *head;

};

enum ast_node_type
{
  node_sect,
  node_tag,
  node_instr,
  node_expr
};


struct ast_node *ast_node_alloc(struct token_data *td, u32 children, struct token_data *tdc)
{
  struct ast_node *node = malloc(sizeof *node);
  if (NULL == node) 
  {
    ast_die(NULL, 1, "MALLOC AST_NODE");
  }

  node->td = td;
  node->next = NULL;
  node->child = NULL;

  node->child_size = children;
  node->child_capacity = children;

  if (0 == children) return node;

  node->child = malloc(children * sizeof *node->child);

  if (NULL == node->child) 
  {
    ast_die(NULL, 1, "MALLOC AST_NODE_CHILD");
  }

  if (tdc != NULL) 
  {
    for (u32 i = 0; i < children; i++) 
    {
      node->child[i] = ast_node_alloc(&tdc[i], 0, NULL);
    }
  } 
  else 
  {
    for (u32 i = 0; i < children; i++) 
    {
      node->child[i] = NULL;
    }
  }

  return node;
}

struct ast *ast_alloc(struct ast_node *root)
{
  struct ast *ast_new = malloc(sizeof(struct ast));

  if(NULL == ast_new)
  {
    ast_die(NULL, 1, "MALLOC AST ALLOC");
  }

  ast_new->root = root;
  ast_new->head = root;

  return ast_new;

}

// TODO fix this function
struct ast_node *ast_build_instr_r(struct lexer *l, u32 pos)
{
  if(pos >= l->size) return NULL;

  struct token_data *td = l->tokens[pos];
  struct ast_node *a = ast_node_alloc(td, 5, l->tokens[pos + 1]);
  // check if there is a format like => [reg, reg, imm]
  // check also for comma !!!

  return a;
}

struct ast_node *ast_build_tag(struct lexer *l, u32 pos)
{
  if(pos >= l->size) return NULL;

  struct token_data *td = l->tokens[pos];
  char *value = td->value;

  struct ast_node *a = NULL;

  u32 params = 0;

  switch (td->type) 
  {
    case token_tag:
      a = ast_node_alloc(td, 0, NULL);
      a->next = ast_build_tag(l, pos + 1);

    case token_instr:
      struct instr *ins = get_instr(value);

      // TODO fix this
      switch (ins->type)
      {
        default:
          a = ast_build_instr_r(l, pos);
      }
      break;
    default: 
      goto die;

  }

  return a;

  die:
    ast_die(NULL, 1, "ERROR IN AST_BUILD_SECT");
    return NULL; // just to shut up the lsp
}


struct ast_node *ast_build_sect(struct lexer *l, u32 pos)
{
  if(pos >= l->size) return NULL;

  struct token_data *td = l->tokens[pos];
  struct ast_node *a = NULL;

  switch (td->type) 
  {
    case token_section:

      struct token_data *tdn = lexer_peek(l, pos + 1);

      if(NULL == tdn) goto die;
      if(tdn->type != token_number) goto die;

      a = ast_node_alloc(td, 1, tdn);
      a->next = ast_build_sect(l, pos + 2);
      break;

    case token_tag:
      a = ast_node_alloc(td, 0, NULL);
      a->next = ast_build_tag(l, pos + 1);

      break;
    default: 
      goto die;

  }


  return a;

die:
  ast_die(NULL, 1, "ERROR IN AST_BUILD_SECT");
  return NULL; // just to shut up the lsp

}

struct ast *ast_init(struct lexer *l)
{
  struct ast *ast_new = ast_alloc(NULL);

  ast_new->root = ast_build_sect(l, 0);

  return ast_new;
}


/* DEBUG AST STUFF */


void ast_node_print_internal(struct ast_node *node, u32 depth)
{
  if (node == NULL) return;
  

  for (u32 i = 0; i < depth; i++)
  {
    printf("  ");
  }

  token_data_print(node->td);

  for (u32 i = 0; i < node->child_size; i++)
  {
    ast_node_print_internal(node->child[i], depth + 1);
  }

  ast_node_print_internal(node->next, depth);
}

void ast_node_print(struct ast_node *node)
{
  ast_node_print_internal(node, 0);
}

void ast_print(struct ast *a)
{

  if(NULL == a) return;

  ast_node_print(a->root);

}

/* ========================= */

#endif
