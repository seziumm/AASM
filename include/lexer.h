#ifndef _LEXER_H
#define _LEXER_H

#include <common.h>
#include <stdarg.h>
#include <token.h>
#include <type.h>

#define LEXER_INIT_CAPACITY 10

/* Prints the current token stream then calls die() with the
   given error code and message.  Used for unrecoverable lexer
   errors (bad character, unknown token, alloc failure). */
#define lexer_die(l, err, ...)  \
  do                            \
  {                             \
    lexer_print(l);             \
    die(err, __VA_ARGS__);      \
  } while (0)

/* ============================================================
 *  Lexer state
 * ============================================================ */

struct lexer
{
  struct token_data **tokens;   /* dynamic array of token pointers */
  u32                 size;     /* number of tokens stored         */
  u32                 capacity; /* allocated slot count            */
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct lexer *lexer_compile(const char *c); /* lex a full source string  */
struct lexer *lexer_init(u0);               /* allocate an empty lexer   */
u0            lexer_free(struct lexer **l); /* free lexer and all tokens */

/* ============================================================
 *  Token stream management
 * ============================================================ */

u0 lexer_push_token_data(struct lexer *l, struct token_data *td); /* append a token          */
u0 lexer_expand(struct lexer *l);                                  /* double token capacity   */

/* ============================================================
 *  Debug
 * ============================================================ */

u0 lexer_print(struct lexer *l); /* print all tokens to stdout */

#endif /* _LEXER_H */
