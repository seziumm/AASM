#ifndef _LEXER_H
#define _LEXER_H

#include <stdarg.h>
#include <token/token_array.h>

#define LEXER_INIT_CAPACITY     4

struct lexer
{
  struct token_array table;
};

struct lexer *lexer_compile(const char *c); /* lex a full source string  */
u0            lexer_free(struct lexer **l); /* free lexer and all tokens */

u0            lexer_print(struct lexer *l); /* print all tokens to stdout */

#endif
