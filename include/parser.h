#ifndef _PARSER_H
#define _PARSER_H

#include <utils/aalloc.h>

struct parser
{
  struct token_array *ta;
  u32                 pos;
};

struct ast_node *parser_root(struct token_array *ta);
u0 parse_free(struct parser **p);

#endif
