#ifndef _LEXER_H
#define _LEXER_H

#include <type.h>
#include <stdarg.h>
#include <common.h>

#define lexer_die(l, err, ...)   \
  do {                           \
    lexer_print(l);              \
    die(err, __VA_ARGS__);       \
  } while (0)


#define LEXER_INIT_CAPACITY         10

enum token
{
  token_comma,   // ','
  token_lparen,  // '('
  token_rparen,  // ')'
  token_number,  // '123'
  token_instr,   // 'ADD ... '
  token_register,// 'X1'
  token_section, // '.section .data' or '.org 0x10000'
  token_label    // '&loop' => 'loop:'
};

struct token_data
{
  char *value;
  enum token type;
};


struct lexer
{
  struct token_data **tokens;
  u32 size; /* number of tokens */
  u32 capacity;
};


struct lexer *lexer_compile(char *c);
struct lexer *lexer_init(u0);
struct token_data *lexer_peek(struct lexer *l, u32 pos);
u0 lexer_push_token_data(struct lexer *l, struct token_data *td);
u0 lexer_push_token_value(struct lexer *l, enum token t, char *value);
u0 lexer_expand(struct lexer *l);

struct token_data *token_data_alloc(enum token t, char *value);


/* DEBUG STUFF */

u0 token_data_print(struct token_data *td);
u0 lexer_print(struct lexer *l);
char *token_to_str(enum token t);

/* ============= */

#endif
