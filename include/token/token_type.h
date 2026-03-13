#ifndef _TOKEN_TYPE_H
#define _TOKEN_TYPE_H

#include <utils/type.h>

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
  TOKEN_DIRECTIVE,  /* '.ALIGN'       */
};

const char *token_type_to_str(enum token_type t);

#endif /* _TOKEN_TYPE_H */
