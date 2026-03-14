#ifndef _DIRECTIVE_LOOK_UP_H
#define _DIRECTIVE_LOOK_UP_H

#include <string.h>
#include <utils/common.h>

enum directive_type
{
  DIRECTIVE_SECTION,
  DIRECTIVE_DATA,
};

static inline const char *directive_type_to_str(enum directive_type type)
{
  switch (type)
  {
    case DIRECTIVE_SECTION: return "DIRECTIVE_SECTION";
    case DIRECTIVE_DATA:    return "DIRECTIVE_DATA";
  }
  die(1, "directive_type_to_str: unknown type %d", type);
  return "";
}

struct directive
{
  const char          *label;
  enum directive_type  type;
 
};

/* ============================================================
 *  Directive table
 * ============================================================ */

static const struct directive directive_array[] =
{
  /* sections */
  { ".CODE",  DIRECTIVE_SECTION },
  { ".DATA",  DIRECTIVE_SECTION },

  /* data emission */
  { ".BYTE",  DIRECTIVE_DATA    },
  { ".2BYTE", DIRECTIVE_DATA    },
  { ".4BYTE", DIRECTIVE_DATA    },
  { ".8BYTE", DIRECTIVE_DATA    },
};

#define DIRECTIVE_ARRAY_SIZE \
  (sizeof(directive_array) / sizeof(directive_array[0]))

/* ============================================================
 *  Lookup
 * ============================================================ */

static inline const struct directive *directive_look_up(const char *label)
{
  for (u32 i = 0; i < DIRECTIVE_ARRAY_SIZE; i++)
  {
    if (strcmp(directive_array[i].label, label) == 0)
    {
      return &directive_array[i];
    }
  }

  return NULL;
}

static inline const struct directive *directive_expect(const char *label, enum directive_type type)
{
  const struct directive *d = directive_look_up(label);

  if (unlikely(NULL == d))
    die(1, "directive_expect: unknown directive '%s'", label);

  if (unlikely(d->type != type))
    die(1, "directive_expect: expected %s directive but '%s' is %s",
        directive_type_to_str(type),
        label,
        directive_type_to_str(d->type));

  return d;
}

#endif /* _DIRECTIVE_LOOK_UP_H */
