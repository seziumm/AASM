#ifndef _INSTR_LOOK_UP_H
#define _INSTR_LOOK_UP_H

#include <rv32/fmt/r/fmt_r_look_up.h>
#include <rv32/fmt/i/fmt_i_look_up.h>
#include <rv32/fmt/s/fmt_s_look_up.h>
#include <rv32/fmt/b/fmt_b_look_up.h>
#include <rv32/fmt/u/fmt_u_look_up.h>
#include <rv32/fmt/j/fmt_j_look_up.h>

static inline const struct instr *instr_look_up(const char *label)
{
  const struct instr *ins;
  if ((ins = fmt_r_look_up(label))) return ins;
  if ((ins = fmt_i_look_up(label))) return ins;
  if ((ins = fmt_s_look_up(label))) return ins;
  if ((ins = fmt_b_look_up(label))) return ins;
  if ((ins = fmt_u_look_up(label))) return ins;
  if ((ins = fmt_j_look_up(label))) return ins;
  return NULL;
}

#endif /* _INSTR_LOOK_UP_H */
