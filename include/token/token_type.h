#ifndef _TOKEN_TYPE_H
#define _TOKEN_TYPE_H

#include <type.h>

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
  TOKEN_SECTION,    /* '.SECTION'     */
};

/* ============================================================
 *  Helpers
 * ============================================================ */

const char *token_to_str(enum token_type t);

#endif /* _TOKEN_TYPE_H */
