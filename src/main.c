#include "ast/ast_node.h"
#include "lexer.h"
#include "parser.h"
#include <utils/common.h>
#include <utils/aalloc.h>

i32 main(i32 argc, char **argv)
{
  if (argc != 3)
  {
    die(1, "Usage: ./%s <in.aasm> <out.mc>\n", argv[0]);
  }

  char *buffer = fread_path(argv[1]);
  struct lexer *l = lexer_compile(buffer);

  // lexer_print(l);
  struct ast_node *p = parser_root(&l->table);

  ast_node_print(p, 0);

  a_free(buffer);
  lexer_free(&l);
  return 0;
}
