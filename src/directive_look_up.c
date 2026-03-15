#include <directive_look_up.h>
#include <ast/ast_node.h>
#include <parser.h>
#include <token/token_type.h>
#include <utils/common.h>

/* ============================================================
 *  directive_parse_section
 *
 *  Handler for .CODE and .DATA.
 *  Consumes tokens until the next DIRECTIVE_SECTION or EOF.
 *  Children: labels, instructions, and data directives.
 * ============================================================ */

struct ast_node *directive_parse_section(struct parser *p, struct ast_node *parent)
{
  (void)parent;

  const struct directive *d    = directive_look_up(parser_peek(p)->value);
  struct ast_node        *node = ast_node_create_directive((struct directive *)d);
  parser_advance(p);   /* consume .CODE / .DATA token */

  while (!parser_at_end(p))
  {
    if (parser_peek_type_is(p, TOKEN_DIRECTIVE))
    {
      const struct directive *d2 = directive_look_up(parser_peek(p)->value);

      if (d2->type == DIRECTIVE_DATA)
      {
        /* .BYTE / .2BYTE / .4BYTE / .8BYTE inside the section */
        ast_node_push(node, directive_dispatch(d2, p, node));
        continue;
      }

      if (d2->type == DIRECTIVE_SECTION)
        return node;   /* next section starts — stop here */

      die(1, "directive_parse_section: unexpected directive '%s'",
          parser_peek(p)->value);
    }

    if (parser_peek_type_is(p, TOKEN_INSTR))
    {
      ast_node_push(node, parse_instr_node(p));
      continue;
    }

    if (parser_peek_type_is(p, TOKEN_LABEL))
    {
      ast_node_push(node, parse_label(p));
      continue;
    }

    die(1, "directive_parse_section: unexpected token %s (\"%s\")",
        token_type_to_str(parser_peek(p)->type),
        parser_peek(p)->value);
  }

  return node;
}

/* ============================================================
 *  directive_parse_data
 *
 *  Handler for .BYTE, .2BYTE, .4BYTE, .8BYTE.
 *  Consumes the directive token and the single integer operand.
 *  Child: one AST_IMM node.
 * ============================================================ */

struct ast_node *directive_parse_data(struct parser *p, struct ast_node *parent)
{
  (void)parent;

  const struct directive *d    = directive_look_up(parser_peek(p)->value);
  struct ast_node        *node = ast_node_create_directive((struct directive *)d);
  parser_advance(p);   /* consume .BYTE / .2BYTE / … token */

  ast_node_push(node, parse_imm(p));   /* single integer operand */

  return node;
}
