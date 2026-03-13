#ifndef _DIRECTIVE_LOOKUP_H
#define _DIRECTIVE_LOOKUP_H

#include <string.h>
#include <utils/common.h>

struct directive
{
  const char *label;
};

static const struct directive directive_array[] =
{
  { ".ALIGN"   },
  { ".BYTE"    },
  { ".HALF"    },
  { ".WORD"    },
  { ".STRING"  },
  { ".SPACE"   },
  { ".SECTION" },
  { ".TEXT"    },
  { ".DATA"    },
  { ".BSS"     },
};

#define DIRECTIVE_ARRAY_SIZE (sizeof(directive_array) / sizeof(directive_array[0]))

const struct directive *directive_look_up(const char *label)
{
  for (u32 i = 0; i < DIRECTIVE_ARRAY_SIZE; ++i)
  {
    if (strcmp(label, directive_array[i].label) == 0)
    {
      return &directive_array[i];
    }
  }
  return NULL;
}

#endif /* _DIRECTIVE_LOOKUP_H */
