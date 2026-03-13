#include <ast/ast_node_type.h>

const char *ast_node_type_str(enum ast_node_type t)
{
  switch (t)
  {
    case AST_INSTR:     return "AST_INSTR";
    case AST_LABEL:     return "AST_LABEL";
    case AST_LABEL_REF: return "AST_LABEL_REF";
    case AST_DIRECTIVE: return "AST_DIRECTIVE";
    case AST_ROOT:      return "AST_ROOT";
    case AST_REG:       return "AST_REG";
    case AST_IMM:       return "AST_IMM";
  }
  return "";
}

