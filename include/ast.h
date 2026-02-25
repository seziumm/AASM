#ifndef _AST_H
#define _AST_H

#include <rv32/rv32_type.h>
#include <lexer.h>
#include <stdio.h>
#include <stdlib.h>
#include <type.h>
#include <common.h>


struct ast_node 
{
  /* Token associato al nodo (può essere NULL) */
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

struct ast_node *ast_node_alloc(struct token_data *td, u32 children, struct token_data *tdc)
{
  struct ast_node *node = malloc(sizeof *node);
  if (NULL == node) 
  {
    die(1, "MALLOC AST_NODE");
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
    die(1, "MALLOC AST_NODE_CHILD");
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
    die(1, "MALLOC AST ALLOC");
  }

  ast_new->root = root;
  ast_new->head = root;

  return ast_new;

}


struct ast_node *ast_build_program(struct lexer *l, u32 *pos)
{
  if (*pos >= l->size) return NULL;

  struct ast_node *head = NULL;
  struct ast_node *tail = NULL;

  while (*pos < l->size)
  {
    struct token_data *td = l->tokens[*pos];
    struct ast_node *node = ast_node_alloc(td, 0, NULL);

    if (head == NULL)
    {
      head = node;
      tail = node;
    }
    else
    {
      tail->next = node;
      tail = node;
    }

    (*pos)++;
  }

  return head;
}


struct ast *ast_init(struct lexer *l)
{
  struct ast *ast_new = ast_alloc(NULL);
  u32 pos = 0;

  ast_new->root = ast_build_program(l, &pos);

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

  token_data_print((struct token_data *)node->td);

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
