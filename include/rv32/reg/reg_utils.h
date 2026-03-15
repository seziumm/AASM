#ifndef _REG_UTILS_H
#define _REG_UTILS_H

#include <rv32/reg/freg.h>
#include <rv32/reg/greg.h>
#include <utils/type.h>

/* Returns the reg descriptor for the given name (e.g. "X5", "SP"),
 * or NULL if the name is not a known register. */
static inline const struct reg *reg_from_label(const char *name)
{
  const struct reg *r;
  if ((r = greg_look_up(name))) return r;
  if ((r = freg_look_up(name))) return r;
  return NULL;
}

#endif /* _REG_UTILS_H */
