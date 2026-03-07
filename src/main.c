#include "parser.h"
#include <stdlib.h>
#include <lexer.h>

i32 main(i32 argc, char **argv)
{
  if(argc != 3)
  {
    die(1, "Usage: ./%s test.aasm out.bin\n", argv[0]);
    return 1;
  }

  char *buffer = fread_path(argv[1]);

  struct lexer *l = lexer_compile(buffer);

  lexer_print(l);


  struct ast_node *node = parser_build(l);
  ast_node_print(node, 0);

  free(buffer);
  lexer_free(&l);
  ast_node_free(&node);

  return 0;
}
