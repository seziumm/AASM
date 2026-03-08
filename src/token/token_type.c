#include <token/token_type.h>
#include <stdio.h>

const char *token_to_str(enum token_type t)
{
  switch (t)
  {
    case TOKEN_COMMA:     return "TOKEN_COMMA";
    case TOKEN_INSTR:     return "TOKEN_INSTR";
    case TOKEN_LABEL:     return "TOKEN_LABEL";
    case TOKEN_LABEL_REF: return "TOKEN_LABEL_REF";
    case TOKEN_LPAREN:    return "TOKEN_LPAREN";
    case TOKEN_NUMBER:    return "TOKEN_NUMBER";
    case TOKEN_REGISTER:  return "TOKEN_REG";
    case TOKEN_RPAREN:    return "TOKEN_RPAREN";
    case TOKEN_SECTION:   return "TOKEN_SECT";
  }

  return "";
}
