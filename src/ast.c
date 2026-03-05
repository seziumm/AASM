#include <lexer.h>
#include <ast.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================
   NODE MEMORY MANAGEMENT
   ====================================================================== */

void ast_node_expand_children(struct ast_node *node)
{
  u32 old_capacity = node->children_capacity;
  u32 new_capacity = (old_capacity == 0) ? AST_NODE_INIT_CAPACITY : old_capacity * 2;

  struct ast_node **new_children =
      realloc(node->children, new_capacity * sizeof(struct ast_node *));

  if (new_children == NULL)
    ast_die(NULL, 1, "realloc() failed in ast_node_expand_children");

  node->children          = new_children;
  node->children_capacity = new_capacity;
}

void ast_node_push_children(struct ast_node *node, struct ast_node *child)
{
  if (node == NULL || child == NULL) return;

  if (node->children_size >= node->children_capacity)
    ast_node_expand_children(node);

  node->children[node->children_size++] = child;
}

struct ast_node *ast_node_alloc(enum ast_node_type type,
                                u32 children_size,
                                struct ast_node **children)
{
  struct ast_node *node = calloc(1, sizeof(struct ast_node));
  if (node == NULL)
    ast_die(NULL, 1, "calloc() failed in ast_node_alloc");

  node->type            = type;
  node->children_size   = children_size;
  node->children_capacity =
      (children_size > AST_NODE_INIT_CAPACITY) ? children_size : AST_NODE_INIT_CAPACITY;

  if (node->children_capacity == 0)
  {
    node->children = NULL;
    return node;
  }

  node->children = malloc(node->children_capacity * sizeof(struct ast_node *));
  if (node->children == NULL)
    ast_die(NULL, 1, "malloc() failed for children array");

  for (u32 i = 0; i < children_size; i++)
    node->children[i] = children[i];

  return node;
}

void ast_node_free(struct ast_node *node)
{
  if (node == NULL) return;
  for (u32 i = 0; i < node->children_size; i++)
    ast_node_free(node->children[i]);
  free(node->children);
  free(node);
}

/* ======================================================================
   HELPERS
   ====================================================================== */

/* restituisce il token corrente o NULL se fuori bounds */
static inline struct token_data *cur(struct lexer *l, u32 *pos)
{
  return (*pos < l->size) ? l->tokens[*pos] : NULL;
}

/* controlla il tipo del token corrente senza consumarlo */
static inline int peek_type(struct lexer *l, u32 *pos, enum token t)
{
  struct token_data *td = cur(l, pos);
  return td && td->type == t;
}

/* consuma il token corrente e avanza */
static inline struct token_data *consume(struct lexer *l, u32 *pos)
{
  return (*pos < l->size) ? l->tokens[(*pos)++] : NULL;
}

/* consuma oppure die */
static inline struct token_data *expect(struct lexer *l, u32 *pos, enum token t)
{
  if (!peek_type(l, pos, t))
    die(1, "Expected token %s at pos %u but got %s",
        token_to_str(t), *pos,
        cur(l, pos) ? token_to_str(cur(l, pos)->type) : "EOF");
  return consume(l, pos);
}

/* ======================================================================
   OPERAND  (register | immediate | label_ref | memory)
   ====================================================================== */

struct ast_node *ast_create_operand(struct lexer *l, u32 *pos)
{
  struct token_data *td = cur(l, pos);
  if (td == NULL)
    die(1, "Expected operand at pos %u but got EOF", *pos);

  struct ast_node *node = NULL;

  switch (td->type)
  {
    case token_register:
      node = ast_node_alloc(AST_OP_REGISTER, 0, NULL);
      node->data.op_register.reg = td->value;
      consume(l, pos);
      break;

    case token_number:
      node = ast_node_alloc(AST_OP_IMMEDIATE, 0, NULL);
      node->data.op_immediate.value = (i32)strtol(td->value, NULL, 0);
      consume(l, pos);
      break;

    case token_label_ref:
      node = ast_node_alloc(AST_OP_LABEL_REF, 0, NULL);
      node->data.op_label_ref.name = td->value;
      consume(l, pos);
      break;

    /* memory: '(' base_reg ')'  oppure  number '(' base_reg ')' */
    case token_lparen:
    {
      node = ast_node_alloc(AST_OP_MEMORY, 0, NULL);
      node->data.op_memory.displacement = 0;
      consume(l, pos);                              /* consuma '('   */
      struct token_data *reg = expect(l, pos, token_register);
      node->data.op_memory.base = reg->value;
      expect(l, pos, token_rparen);                 /* consuma ')'   */
      break;
    }

    default:
      die(1, "Unexpected token %s as operand at pos %u",
          token_to_str(td->type), *pos);
  }

  return node;
}

/* ======================================================================
   INSTRUCTION   mnemonic [op (',' op)*]
   ====================================================================== */

u32 ast_build_instr_params(struct lexer *l, u32 *pos, struct ast_node *node)
{
  /* legge operandi separati da virgola finché ce ne sono */
  while (*pos < l->size)
  {
    enum token t = l->tokens[*pos]->type;

    /* token che possono iniziare un operando */
    if (t == token_register  ||
        t == token_number    ||
        t == token_label_ref ||
        t == token_lparen)
    {
      ast_node_push_children(node, ast_create_operand(l, pos));

      /* se segue una virgola, la consumiamo e continuiamo */
      if (peek_type(l, pos, token_comma))
        consume(l, pos);
    }
    else
    {
      break; /* fine lista operandi */
    }
  }
  return 0;
}

struct ast_node *ast_create_instr(struct lexer *l, u32 *pos)
{
  struct token_data *td = expect(l, pos, token_instr);

  struct ast_node *node = ast_node_alloc(AST_INSTRUCTION, 0, NULL);
  node->data.instruction.mnemonic = td->value;

  ast_build_instr_params(l, pos, node);
  return node;
}

/* ======================================================================
   LABEL   name ':' instr*
   ====================================================================== */

u32 ast_build_label_params(struct lexer *l, u32 *pos, struct ast_node *node)
{
  (void)l; (void)pos; (void)node;
  return 0; /* nessun parametro extra dopo il nome */
}

struct ast_node *ast_create_label(struct lexer *l, u32 *pos)
{
  struct token_data *td = expect(l, pos, token_label);

  struct ast_node *node = ast_node_alloc(AST_LABEL, 0, NULL);
  node->data.label.name = td->value;

  ast_build_label_params(l, pos, node);

  /* istruzioni che appartengono a questo label */
  while (*pos < l->size && l->tokens[*pos]->type == token_instr)
    ast_node_push_children(node, ast_create_instr(l, pos));

  return node;
}

/* ======================================================================
   SECTION   '.section' value  label*
   ====================================================================== */

u32 ast_build_section_params(struct lexer *l, u32 *pos, struct ast_node *node)
{
  struct token_data *num = expect(l, pos, token_number);
  node->data.section.value = num->value;
  return 0;
}

struct ast_node *ast_create_section(struct lexer *l, u32 *pos)
{
  expect(l, pos, token_section);

  struct ast_node *node = ast_node_alloc(AST_SECTION, 0, NULL);

  ast_build_section_params(l, pos, node);

  while (*pos < l->size && l->tokens[*pos]->type == token_label)
    ast_node_push_children(node, ast_create_label(l, pos));

  return node;
}

/* ======================================================================
   PROGRAM   section*
   ====================================================================== */

u32 ast_build_program_params(struct lexer *l, u32 *pos, struct ast_node *node)
{
  (void)l; (void)pos; (void)node;
  return 0;
}

struct ast_node *ast_create_program(struct lexer *l, u32 *pos)
{
  struct ast_node *node = ast_node_alloc(AST_PROGRAM, 0, NULL);

  ast_build_program_params(l, pos, node);

  while (*pos < l->size)
  {
    if (l->tokens[*pos]->type == token_section)
      ast_node_push_children(node, ast_create_section(l, pos));
    else
      die(1, "Unexpected token %s at top level (pos %u)",
          token_to_str(l->tokens[*pos]->type), *pos);
  }

  return node;
}

/* ======================================================================
   ENTRY POINT
   ====================================================================== */

struct ast_node *ast_compile(struct lexer *l)
{
  u32 pos = 0;
  return ast_create_program(l, &pos);
}

/* ======================================================================
   DEBUG PRINT
   ====================================================================== */

static const char *ast_type_str(enum ast_node_type t)
{
  switch (t)
  {
    case AST_PROGRAM:     return "PROGRAM";
    case AST_SECTION:     return "SECTION";
    case AST_LABEL:       return "LABEL";
    case AST_INSTRUCTION: return "INSTRUCTION";
    case AST_OP_REGISTER: return "OP_REGISTER";
    case AST_OP_IMMEDIATE:return "OP_IMMEDIATE";
    case AST_OP_MEMORY:   return "OP_MEMORY";
    case AST_OP_LABEL_REF:return "OP_LABEL_REF";
    default:              return "???";
  }
}

u0 ast_print(struct ast_node *node, i32 depth)
{
  if (node == NULL) return;

  for (i32 i = 0; i < depth; i++) printf(i == depth-1 ? "├── " : "│   ");

  switch (node->type)
  {
    case AST_PROGRAM:
      printf("PROGRAM\n");
      break;

    case AST_SECTION:
      printf("SECTION  \033[33m@%s\033[0m\n", node->data.section.value);
      break;

    case AST_LABEL:
      printf("LABEL  \033[36m%s\033[0m\n", node->data.label.name);
      break;

    case AST_INSTRUCTION:
      printf("INSTR  \033[32m%s\033[0m\n", node->data.instruction.mnemonic);
      break;

    case AST_OP_REGISTER:
      printf("REG    \033[35m%s\033[0m\n", node->data.op_register.reg);
      break;

    case AST_OP_IMMEDIATE:
      printf("IMM    \033[31m%d\033[0m\n", node->data.op_immediate.value);
      break;

    case AST_OP_MEMORY:
      printf("MEM    \033[31m%d\033[0m(\033[35m%s\033[0m)\n",
             node->data.op_memory.displacement,
             node->data.op_memory.base);
      break;

    case AST_OP_LABEL_REF:
      printf("LREF   \033[36m%s\033[0m\n", node->data.op_label_ref.name);
      break;
  }

  for (u32 i = 0; i < node->children_size; i++)
    ast_print(node->children[i], depth + 1);
}
