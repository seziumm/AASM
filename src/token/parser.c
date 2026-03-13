#include <rv32/instr_look_up.h>
#include <ast/ast_node.h>
#include <rv32/reg/reg_look_up.h>
#include <rv32/fmt/r/fmt_r_look_up.h>
#include <rv32/fmt/i/fmt_i_look_up.h>
#include <token/token_array.h>
#include <token/token_type.h>
#include <utils/aalloc.h>
#include <utils/common.h>
#include <stdlib.h>
#include <lexer.h>
#include <parser.h>

static inline i32 parser_at_end(struct parser *p)
{
  return (p->pos >= p->ta->size);
}

static inline struct token *parser_peek(struct parser *p)
{
  if (parser_at_end(p)) return NULL;
  return p->ta->tokens[p->pos];
}

static inline struct token *parser_advance(struct parser *p)
{
  if (parser_at_end(p)) return NULL;
  return p->ta->tokens[p->pos++];
}

static inline u0 parser_skip_comma(struct parser *p)
{
  if (!parser_at_end(p) && parser_peek(p)->type == TOKEN_COMMA)
    parser_advance(p);
}

static inline i32 parser_peek_type_is(struct parser *p, enum token_type tt)
{
  if (parser_at_end(p)) return 0;
  return parser_peek(p)->type == tt;
}

static struct token *parser_expect(struct parser *p, enum token_type expected)
{
  if (parser_at_end(p))
    die(1, "parser: unexpected end of token stream (expected %s)",
        token_type_to_str(expected));

  struct token *t = parser_peek(p);

  if (t->type != expected)
    die(1, "parser: expected %s but got %s (\"%s\")",
        token_type_to_str(expected),
        token_type_to_str(t->type),
        t->value);

  return parser_advance(p);
}

struct parser *parser_create(struct token_array *ta)
{
  struct parser *p = a_malloc(sizeof(struct parser));
  *p = (struct parser){ .ta = ta, .pos = 0 };
  return p;
}


static inline struct ast_node *parse_reg(struct parser *p)
{
  struct token *t   = parser_expect(p, TOKEN_REGISTER);
  const struct reg *r = reg_look_up(t->value);
  expect(NULL != r);
  return ast_node_create_reg((u8)r->index);
}

static inline struct ast_node *parse_imm(struct parser *p)
{
  struct token *t = parser_expect(p, TOKEN_NUMBER);
  return ast_node_create_imm((i32)atoi(t->value));
}

static inline struct ast_node *parse_label_ref(struct parser *p)
{
  struct token *t = parser_expect(p, TOKEN_LABEL_REF); // TODO fix this
  return ast_node_create_label_ref();
}

/* imm ( reg ) — pushes imm then reg as children of instr */
static inline u0 parse_mem(struct parser *p, struct ast_node *instr)
{
  struct token *t;

  t = parser_expect(p, TOKEN_NUMBER);
  ast_node_push(instr, ast_node_create_imm((i32)atoi(t->value)));

  parser_expect(p, TOKEN_LPAREN);

  t = parser_expect(p, TOKEN_REGISTER);
  const struct reg *r = reg_look_up(t->value);
  expect(NULL != r);
  ast_node_push(instr, ast_node_create_reg((u8)r->index));

  parser_expect(p, TOKEN_RPAREN);
}

static u0 parse_instr_operands(struct parser  *p,
                                struct ast_node *node,
                                const struct instr *ins)
{
  enum fmt_type t = ins->type;

  switch (t)
  {
    case FMT_R:
      ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
      ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
      ast_node_push(node, parse_reg(p));
      break;

    case FMT_I:
      /* load: rd , imm ( rs1 ) */
      if (ins->i.opcode == 0x03)
      {
        ast_node_push(node, parse_reg(p));
        parser_skip_comma(p);
        parse_mem(p, node);
      }
      else
      {
        ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
        ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
        ast_node_push(node, parse_imm(p));
      }
      break;

    case FMT_S:
      ast_node_push(node, parse_reg(p));
      parser_skip_comma(p);
      parse_mem(p, node);
      break;

    case FMT_B:
      ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
      ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
      if (parser_peek_type_is(p, TOKEN_LABEL_REF))
        ast_node_push(node, parse_label_ref(p));
      else
        ast_node_push(node, parse_imm(p));
      break;

    case FMT_U:
      ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
      ast_node_push(node, parse_imm(p));
      break;

    case FMT_J:
      ast_node_push(node, parse_reg(p)); parser_skip_comma(p);
      if (parser_peek_type_is(p, TOKEN_LABEL_REF))
        ast_node_push(node, parse_label_ref(p));
      else
        ast_node_push(node, parse_imm(p));
      break;
  }
}

/* ============================================================
 *  Recursive descent
 * ============================================================ */

static struct ast_node *parse_instr_node(struct parser *p)
{
  struct token       *t   = parser_advance(p); /* TOKEN_INSTR */
  const struct instr *ins = instr_look_up(t->value);
  expect(NULL != ins);
  struct ast_node *node = ast_node_create_instr((struct instr *)ins);
  parse_instr_operands(p, node, ins);
  return node;
}

static struct ast_node *parse_label_block(struct parser *p)
{
  struct token    *t   = parser_advance(p);           /* TOKEN_LABEL */
  struct ast_node *lbl = ast_node_create_label();     /* skip '&' */

  while (!parser_at_end(p))
  {
    enum token_type tt = parser_peek(p)->type;

    if (tt == TOKEN_LABEL || tt == TOKEN_DIRECTIVE)
      break;

    if (tt == TOKEN_INSTR)
    {
      ast_node_push(lbl, parse_instr_node(p));
      continue;
    }

    die(1, "parse_label_block: unexpected token %s (\"%s\")",
        token_type_to_str(tt), parser_peek(p)->value);
  }

  return lbl;
}

/* ============================================================
 *  Top-level parse
 * ============================================================ */

struct ast_node *parser_build(struct token_array *ta)
{
  if (NULL == ta) return NULL;

  struct parser   *p    = parser_create(ta);
  struct ast_node *root = ast_node_create(AST_ROOT);

  while (!parser_at_end(p))
  {
    enum token_type tt = parser_peek(p)->type;

    if (tt == TOKEN_LABEL)
    {
      ast_node_push(root, parse_label_block(p));
      continue;
    }

    if (tt == TOKEN_INSTR)
    {
      ast_node_push(root, parse_instr_node(p));
      continue;
    }

    die(1, "parser_build: unexpected token %s (\"%s\")",
        token_type_to_str(tt), parser_peek(p)->value);
  }

  free(p);
  return root;
}
