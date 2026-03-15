#ifndef _PARSER_H
#define _PARSER_H

#include <utils/aalloc.h>
#include <token/token_array.h>
#include <token/token_type.h>

/* ============================================================
 *  Parser state  —  exposed so directive parse_fn handlers
 *  can consume tokens via the helpers below.
 * ============================================================ */

struct parser
{
  struct token_array *ta;
  u32                 pos;
};

/* ============================================================
 *  Navigation helpers
 * ============================================================ */

struct token *parser_peek        (struct parser *p);
struct token *parser_advance     (struct parser *p);
struct token *parser_expect      (struct parser *p, enum token_type expected);
i32           parser_at_end      (struct parser *p);
i32           parser_peek_type_is(struct parser *p, enum token_type tt);

/* ============================================================
 *  Operand parsers  (shared with directive handlers)
 * ============================================================ */

struct ast_node *parse_reg             (struct parser *p);
struct ast_node *parse_imm             (struct parser *p);
struct ast_node *parse_label_ref       (struct parser *p);
struct ast_node *parse_label_ref_or_imm(struct parser *p);
struct ast_node *parse_instr_node      (struct parser *p);
struct ast_node *parse_label           (struct parser *p);

/* ============================================================
 *  Entry point
 * ============================================================ */

struct ast_node *parser_root(struct token_array *ta);
u0               parse_free (struct parser **p);

#endif /* _PARSER_H */
