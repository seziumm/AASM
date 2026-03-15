#ifndef _INSTR_UTILS_H
#define _INSTR_UTILS_H

#include <rv32/fmt/r/fmt_r_utils.h>
#include <rv32/fmt/i/fmt_i_utils.h>
#include <rv32/fmt/s/fmt_s_utils.h>
#include <rv32/fmt/b/fmt_b_utils.h>
#include <rv32/fmt/u/fmt_u_utils.h>
#include <rv32/fmt/j/fmt_j_utils.h>

/* Returns the instr descriptor for the given mnemonic searching
 * across all formats, or NULL if not found. */
static inline const struct instr *instr_from_label(const char *label)
{
  const struct instr *ins;
  if ((ins = fmt_r_from_label(label))) return ins;
  if ((ins = fmt_i_from_label(label))) return ins;
  if ((ins = fmt_s_from_label(label))) return ins;
  if ((ins = fmt_b_from_label(label))) return ins;
  if ((ins = fmt_u_from_label(label))) return ins;
  if ((ins = fmt_j_from_label(label))) return ins;
  return NULL;
}

#endif /* _INSTR_UTILS_H */
