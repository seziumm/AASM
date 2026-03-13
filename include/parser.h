#ifndef _PARSER_H
#define _PARSER_H

#include <utils/aalloc.h>

struct parser
{
  struct token_array *ta;
  u32                 pos;
};

struct parser *parser_create(struct token_array *ta);

#endif
