#ifndef _TOKEN_H
#define _TOKEN_H

#include <type.h>
#include <common.h>

/* Calls die() with the given error code and message.
   The print is stubbed out until token has a print context. */
#define token_die(t, err, ...)  \
  do                            \
  {                             \
    /*token_data_print(t);*/    \
    die(err, __VA_ARGS__);      \
  } while (0)

/* ============================================================
 *  Token types
 * ============================================================ */

enum token_type
{
  TOKEN_COMMA,      /* ','            */
  TOKEN_INSTR,      /* 'ADD', 'SW'... */
  TOKEN_LABEL,      /* '&LOOP'        */
  TOKEN_LABEL_REF,  /* '@LOOP'        */
  TOKEN_LPAREN,     /* '('            */
  TOKEN_NUMBER,     /* '123', '-4'    */
  TOKEN_REGISTER,   /* 'X1', 'SP'...  */
  TOKEN_RPAREN,     /* ')'            */
  TOKEN_SECTION,    /* '.ADDR 100'    */
};

/* ============================================================
 *  Token data
 * ============================================================ */

struct token_data
{
  enum token_type  type;   /* token class        */
  char            *value;  /* raw lexed string   */
};

/* ============================================================
 *  Lifecycle
 * ============================================================ */

struct token_data *token_data_alloc(u0);                              /* allocate only         */
struct token_data *token_data_create(enum token_type t, char *value); /* allocate and init     */
u0                 token_data_free(struct token_data **td);           /* free value and struct */

/* ============================================================
 *  Helpers
 * ============================================================ */

const char *token_to_str(enum token_type t);

/* ============================================================
 *  Debug
 * ============================================================ */

u0 token_data_print(struct token_data *td);

#endif /* _TOKEN_H */
