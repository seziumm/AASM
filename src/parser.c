#include <directive_look_up.h>
#include <rv32/instr_look_up.h>
#include <ast/ast_node.h>
#include <rv32/reg/reg_look_up.h>
#include <rv32/fmt/r/fmt_r_look_up.h>
#include <rv32/fmt/i/fmt_i_look_up.h>
#include <rv32/fmt/s/fmt_s_look_up.h>
#include <rv32/fmt/b/fmt_b_look_up.h>
#include <rv32/fmt/u/fmt_u_look_up.h>
#include <rv32/fmt/j/fmt_j_look_up.h>
#include <token/token_array.h>
#include <token/token_type.h>
#include <utils/aalloc.h>
#include <utils/common.h>
#include <stdlib.h>
#include <lexer.h>
#include <parser.h>

static struct token *parser_expect(struct parser *p, enum token_type expected);

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

static inline struct parser *parser_create(struct token_array *ta)
{
  struct parser *p = a_malloc(sizeof(struct parser));
  *p = (struct parser){ .ta = ta, .pos = 0 };
  return p;
}

u0 parse_free(struct parser **p)
{
  if (NULL == p || NULL == *p) return;
  a_free(*p);
  *p = NULL;
}

static inline u0 parse_comma(struct parser *p)
{
  parser_expect(p, TOKEN_COMMA);
}

static inline struct ast_node *parse_reg(struct parser *p)
{
  struct token     *t = parser_expect(p, TOKEN_REGISTER);
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
  /* token value is "@NAME" — store full value; codegen strips '@' on lookup */
  struct token *t = parser_expect(p, TOKEN_LABEL_REF);
  return ast_node_create_label_ref(t->value);
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
  ast_node_push(instr, ast_node_create_reg((u8)r->index));

  parser_expect(p, TOKEN_RPAREN);
}

static inline struct ast_node *parse_label_ref_or_imm(struct parser *p)
{
  if (parser_peek_type_is(p, TOKEN_LABEL_REF))
    return parse_label_ref(p);

  if (parser_peek_type_is(p, TOKEN_NUMBER))
    return parse_imm(p);

  die(1, "parser: expected %s or %s but found %s",
      token_type_to_str(TOKEN_LABEL_REF),
      token_type_to_str(TOKEN_NUMBER),
      token_type_to_str(parser_peek(p)->type));

  return NULL;
}

static u0 parse_instr_operands(struct parser   *p,
                                struct ast_node *node,
                                const struct instr *ins)
{
  switch (ins->type)
  {
    case FMT_R:
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      ast_node_push(node, parse_reg(p));
      break;

    case FMT_I:
      if (fmt_i_look_up_load(ins->label))
      {
        ast_node_push(node, parse_reg(p)); parse_comma(p);
        parse_mem(p, node);
      }
      else
      {
        ast_node_push(node, parse_reg(p)); parse_comma(p);
        ast_node_push(node, parse_reg(p)); parse_comma(p);
        ast_node_push(node, parse_imm(p));
      }
      break;

    case FMT_S:
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      parse_mem(p, node);
      break;

    case FMT_B:
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      ast_node_push(node, parse_label_ref_or_imm(p));
      break;

    case FMT_U:
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      ast_node_push(node, parse_imm(p));
      break;

    case FMT_J:
      ast_node_push(node, parse_reg(p)); parse_comma(p);
      ast_node_push(node, parse_label_ref_or_imm(p));
      break;
  }
}

static struct ast_node *parse_instr_node(struct parser *p)
{
  struct token       *t   = parser_advance(p); /* TOKEN_INSTR */
  const struct instr *ins = instr_look_up(t->value);
  struct ast_node    *node = ast_node_create_instr((struct instr *)ins);
  parse_instr_operands(p, node, ins);
  return node;
}

static struct ast_node *parse_label(struct parser *p)
{
  /* token value is "&NAME" — store full value as label name */
  struct token    *t   = parser_advance(p);
  struct ast_node *lbl = ast_node_create_label(t->value);

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

static struct ast_node *parse_directive_data(struct parser *p)
{
  const struct directive *d = directive_look_up(parser_peek(p)->value);
  struct ast_node *node = ast_node_create_directive((struct directive *)d);
  parser_advance(p);
  ast_node_push(node, parse_imm(p));
  return node;
}

static struct ast_node *parse_directive_section(struct parser *p)
{
  const struct directive *d = directive_look_up(parser_peek(p)->value);
  struct ast_node *node = ast_node_create_directive((struct directive *)d);
  parser_advance(p);

  while (!parser_at_end(p))
  {
    if (parser_peek_type_is(p, TOKEN_DIRECTIVE))
    {
      const struct directive *d2 = directive_look_up(parser_peek(p)->value);

      if (d2->type == DIRECTIVE_DATA)
        ast_node_push(node, parse_directive_data(p));
      else if (d2->type == DIRECTIVE_SECTION)
        return node;
      else
        die(1, "parse_directive: unexpected token %s",
            token_type_to_str(parser_peek(p)->type));
    }
    else if (parser_peek_type_is(p, TOKEN_INSTR))
      ast_node_push(node, parse_instr_node(p));
    else if (parser_peek_type_is(p, TOKEN_LABEL))
      ast_node_push(node, parse_label(p));
    else
      die(1, "parse_directive: unexpected token %s",
          token_type_to_str(parser_peek(p)->type));
  }

  return node;
}

static struct ast_node *parse_directive(struct parser *p)
{
  const struct directive *d = directive_look_up(parser_peek(p)->value);

  switch (d->type)
  {
    case DIRECTIVE_SECTION: return parse_directive_section(p);
    case DIRECTIVE_DATA:    return parse_directive_data(p);
  }

  die(1, "parse_directive: unhandled directive type %d", d->type);
  return NULL;
}

struct ast_node *parser_root(struct token_array *ta)
{
  if (NULL == ta) return NULL;

  struct parser   *p    = parser_create(ta);
  struct ast_node *root = ast_node_create_root();

  while (!parser_at_end(p))
  {
    enum token_type tt = parser_peek(p)->type;

    switch (tt)
    {
      case TOKEN_DIRECTIVE:
        ast_node_push(root, parse_directive(p));
        break;

      default:
        die(1, "parser_root: unexpected token %s (\"%s\")",
            token_type_to_str(tt), parser_peek(p)->value);
    }
  }

  return root;
}
