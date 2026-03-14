#ifndef _INSTR_H
#define _INSTR_H

#include <utils/common.h>
#include <rv32/fmt/r/fmt_r.h>
#include <rv32/fmt/i/fmt_i.h>
#include <rv32/fmt/s/fmt_s.h>
#include <rv32/fmt/b/fmt_b.h>
#include <rv32/fmt/u/fmt_u.h>
#include <rv32/fmt/j/fmt_j.h>
#include <rv32/fmt/fmt_type.h>
#include <utils/type.h>

struct instr
{
  const char   *label;
  enum fmt_type type;
  union
  {
    struct fmt_r r;
    struct fmt_i i;
    struct fmt_s s;
    struct fmt_b b;
    struct fmt_u u;
    struct fmt_j j;
    u32 raw;
  };
};

#endif /* _INSTR_H */
