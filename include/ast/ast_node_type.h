#ifndef _AST_NODE_TYPE_H
#define _AST_NODE_TYPE_H

enum ast_node_type
{
  AST_INSTR,
  AST_LABEL,
  AST_LABEL_REF,
  AST_DIRECTIVE,
  AST_ROOT,      /* root node                          */
  AST_REG,       /* register operand   (u8  reg index) */
  AST_IMM,       /* immediate operand  (i32 value)     */
};

const char *ast_node_type_str(enum ast_node_type t);

#endif /* _AST_NODE_TYPE_H */
