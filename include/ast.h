#ifndef _AST_H
#define _AST_H

#include <lexer.h>
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

struct ast 
{
  struct ast_node *root;
  struct ast_node *head;

};


struct ast_node *ast_node_alloc(struct token_data *td, u32 children)
{
  struct ast_node *node = malloc(sizeof(struct ast_node));

  if(NULL == node)
  {
    ast_die(NULL, 1, "MALLOC ASTD_NODE");
  }

  node->td = td;
  node->next = NULL;
  node->child = NULL;


  node->child_size = children;
  node->child_capacity = AST_NODE_INIT_CAPACITY;

  if(children > 0) 
  {
    node->child = malloc(children * sizeof(struct ast_node *));

    if(NULL == node->child)
    {
      ast_die(NULL, 1, "MALLOC ASTD_NODE_CHILD");
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

// ti dice quanti figli ha una istruzione
// TODO fixare questo non funzoina
u32 ast_children_of(enum token *t)
{
  switch (*t) 
  {
    case token_comma:     return 0;
    case token_lparen:    return 0;
    case token_rparen:    return 0;
    case token_number:    return 0;
    case token_instr:     return 1;
    case token_register:  return 0;
    case token_section:   return 1;
    case token_tag:       return 0;
  }
  
  return 0;

}


void ast_build(struct ast_node *a, struct lexer *l, u32 pos)
{
  if(pos >= l->size) return;

  ast_build(a->next, l, pos + 1);
}

struct ast *ast_init(struct lexer *l)
{
  struct ast *ast_new = ast_alloc(NULL);

  ast_build(ast_new->root, l, 0);

  return ast_new;
}


#endif
