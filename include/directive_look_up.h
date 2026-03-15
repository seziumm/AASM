#ifndef _DIRECTIVE_LOOK_UP_H
#define _DIRECTIVE_LOOK_UP_H

#include <string.h>
#include <utils/common.h>

/* Forward-declare parser and ast_node so the header stays self-contained.
   The actual definitions live in parser.h and ast/ast_node.h. */
struct parser;
struct ast_node;

/* ============================================================
 *  Directive types
 * ============================================================ */

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

/* ============================================================
 *  Directive descriptor
 *
 *  parse_fn mirrors the GNU AS "pseudo-op" handler convention:
 *
 *    struct pseudo_op { const char *poc_name;
 *                       void (*poc_handler)(int); int poc_val; ... };
 *
 *  Here we pass (parser *, parent_node) instead of a bare int so
 *  the handler can both consume tokens and attach children to the
 *  AST node that represents the current section/block.
 * ============================================================ */

typedef struct ast_node *(*directive_parse_fn)(struct parser *p,
                                               struct ast_node *parent);

struct directive
{
  const char          *label;       /* ".CODE", ".BYTE", …               */
  enum directive_type  type;        /* SECTION or DATA                   */
  directive_parse_fn   parse_fn;    /* NULL → default parser handles it  */
};

/* ============================================================
 *  Forward declarations of the per-directive parse handlers.
 *  Implementations live in src/parser.c (or a dedicated
 *  src/directive_handlers.c if you split later).
 * ============================================================ */

struct ast_node *directive_parse_section(struct parser *p, struct ast_node *parent);
struct ast_node *directive_parse_data   (struct parser *p, struct ast_node *parent);

/* ============================================================
 *  Directive table
 * ============================================================ */

static const struct directive directive_array[] =
{
  /* sections */
  { ".CODE",   DIRECTIVE_SECTION, directive_parse_section },
  { ".DATA",   DIRECTIVE_SECTION, directive_parse_section },

  /* data emission — each calls the same generic data handler   */
  { ".BYTE",   DIRECTIVE_DATA,    directive_parse_data    },
  { ".2BYTE",  DIRECTIVE_DATA,    directive_parse_data    },
  { ".4BYTE",  DIRECTIVE_DATA,    directive_parse_data    },
  { ".8BYTE",  DIRECTIVE_DATA,    directive_parse_data    },
  { ".STRING", DIRECTIVE_DATA,    directive_parse_data    },
};

#define DIRECTIVE_ARRAY_SIZE \
  (sizeof(directive_array) / sizeof(directive_array[0]))

/* ============================================================
 *  Lookup
 * ============================================================ */

static inline const struct directive *directive_look_up(const char *label)
{
  for (u32 i = 0; i < DIRECTIVE_ARRAY_SIZE; i++)
    if (strcmp(directive_array[i].label, label) == 0)
      return &directive_array[i];

  return NULL;
}

static inline const struct directive *directive_expect(const char *label,
                                                        enum directive_type type)
{
  const struct directive *d = directive_look_up(label);

  if (unlikely(NULL == d))
    die(1, "directive_expect: unknown directive '%s'", label);

  if (unlikely(d->type != type))
    die(1, "directive_expect: expected %s but '%s' is %s",
        directive_type_to_str(type),
        label,
        directive_type_to_str(d->type));

  return d;
}

/* ============================================================
 *  Dispatch helper — call the directive's parse_fn.
 *  The parser calls this instead of a manual switch.
 * ============================================================ */

static inline struct ast_node *directive_dispatch(const struct directive *d,
                                                   struct parser          *p,
                                                   struct ast_node        *parent)
{
  expect(NULL != d);

  if (unlikely(NULL == d->parse_fn))
    die(1, "directive_dispatch: no parse_fn for '%s'", d->label);

  return d->parse_fn(p, parent);
}

#endif /* _DIRECTIVE_LOOK_UP_H */
