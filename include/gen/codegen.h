#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <token/token_array.h>
#include <token/token.h>
#include <token/token_type.h>
#include <utils/type.h>
#include <utils/common.h>
#include <stdio.h>

#define CODEGEN_NOP       0x00000013u  /* ADDI X0, X0, 0 */
#define CODEGEN_LABEL_MAX 500

/* ============================================================
 *  Symbol table
 * ============================================================ */

struct codegen_label
{
  const char *name; /* token value as stored e.g. "&MAIN" */
  u32         addr;
};

/* ============================================================
 *  Codegen context
 * ============================================================ */

struct codegen_ctx
{
  struct token_array   *ta;
  u32                   base;
  u32                   pc;
  FILE                 *out;
  struct codegen_label  labels[CODEGEN_LABEL_MAX];
  u32                   labels_size;
};

/* ============================================================
 *  cg_expect  —  central token validator
 *
 *  Checks that the token at *pos has the expected type,
 *  advances *pos, and returns the token.
 *  Crashes with a descriptive message on mismatch.
 * ============================================================ */

static inline struct token *cg_expect(struct codegen_ctx *ctx,
                                       u32                *pos,
                                       enum token_type     expected)
{
  if (unlikely(*pos >= ctx->ta->size))
    die(1, "cg_expect: unexpected end of tokens (expected %s)",
        token_type_to_str(expected));

  struct token *t = ctx->ta->tokens[*pos];

  if (unlikely(t->type != expected))
    die(1, "cg_expect: expected %s but got %s (\"%s\") at token %u",
        token_type_to_str(expected),
        token_type_to_str(t->type),
        t->value,
        *pos);

  (*pos)++;
  return t;
}

/* ============================================================
 *  Public API
 * ============================================================ */

u0 codegen_run(struct token_array *ta, u32 base, const char *path);

#endif /* _CODEGEN_H */
