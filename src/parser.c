#include <parser.h>
#include <stdlib.h>
#include <stdio.h>

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct ast_node *ast_node_alloc(u0)
{
  struct ast_node *n = malloc(sizeof(struct ast_node));

  if (NULL == n)
    parser_die(NULL, 1, "malloc() failed in ast_node_alloc()");

  return n;
}

struct ast_node *ast_node_create(enum ast_node_type type)
{
  struct ast_node  *n  = ast_node_alloc();
  struct ast_node **ch = malloc(AST_INIT_CAPACITY * sizeof(struct ast_node *));

  if (NULL == ch)
  {
    free(n);
    parser_die(NULL, 1, "malloc() failed in ast_node_create()");
  }

  *n = (struct ast_node)
  {
    .type      = type,
    .name      = NULL,
    .children  = ch,
    .nchildren = 0,
    .capacity  = AST_INIT_CAPACITY,
  };

  return n;
}

u0 ast_node_free(struct ast_node **n)
{
  if (NULL == n || NULL == *n) return;

  for (u32 i = 0; i < (*n)->nchildren; i++)
    ast_node_free(&(*n)->children[i]);

  free((*n)->children);
  free(*n);
  *n = NULL;
}

/* ============================================================
 *  Children management
 * ============================================================ */

static u0 ast_node_expand(struct ast_node *n)
{
  u32               new_capacity = n->capacity * 2;
  struct ast_node **new_children = realloc(n->children, new_capacity * sizeof(struct ast_node *));

  if (NULL == new_children)
    parser_die(NULL, 1, "realloc() failed in ast_node_expand()");

  n->children = new_children;
  n->capacity = new_capacity;
}

u0 ast_node_push(struct ast_node *parent, struct ast_node *child)
{
  if (NULL == parent || NULL == child) return;

  if (parent->nchildren >= parent->capacity)
    ast_node_expand(parent);

  parent->children[parent->nchildren++] = child;
}

/* ============================================================
 *  Convenience constructors
 * ============================================================ */

struct ast_node *ast_node_create_instr(enum rv32i_instr instr)
{
  struct ast_node *n = ast_node_create(AST_INSTR);
  n->instr = instr;
  return n;
}

struct ast_node *ast_node_create_label(const char *name)
{
  struct ast_node *n = ast_node_create(AST_LABEL);
  n->name = name;
  return n;
}

struct ast_node *ast_node_create_label_ref(const char *name)
{
  struct ast_node *n = ast_node_create(AST_LABEL_REF);
  n->name = name;
  return n;
}

struct ast_node *ast_node_create_section(u32 addr)
{
  struct ast_node *n = ast_node_create(AST_SECTION);
  n->addr = addr;
  return n;
}

/* ============================================================
 *  Debug
 * ============================================================ */

static const char *ast_node_type_str(enum ast_node_type t)
{
  switch (t)
  {
    case AST_INSTR:     return "INSTR";
    case AST_LABEL:     return "LABEL";
    case AST_LABEL_REF: return "LABEL_REF";
    case AST_SECTION:   return "SECTION";
  }
  return "UNKNOWN";
}

u0 ast_node_print(struct ast_node *n, u32 depth)
{
  if (NULL == n) return;

  for (u32 i = 0; i < depth; i++) printf("  ");
  printf("AST(%-10s)", ast_node_type_str(n->type));

  for (u32 i = 0; i < n->nchildren; i++)
    ast_node_print(n->children[i], depth + 1);
}
