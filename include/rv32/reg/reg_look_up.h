#ifndef _REG_LOOK_UP_H
#define _REG_LOOK_UP_H

#include <rv32/reg/freg.h>
#include <rv32/reg/greg.h>
#include <utils/type.h>

/*
 * reg_look_up() is just a wrapper of
 * greg_look_up() and freg_look_up()
 */
static inline const struct reg *reg_look_up(const char *name)
{
  const struct reg *r;
  if ((r = greg_look_up(name))) return r;
  if ((r = freg_look_up(name))) return r;
  return NULL;
}

#endif /* _REG_LOOK_UP_H */
