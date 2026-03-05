#include <ast.h>
#include <lexer.h>
#include <common.h>
#include <stdlib.h>


i32 main(i32 argc, char **argv) 
{

  if (argc != 2) 
  {
    die(1, "Usage: ./%s file.aasm\n", argv[0]);
    return 1;
  }

  char *buffer = freadbuf(argv[1]);

  struct lexer *l = lexer_compile(buffer);

  lexer_print(l);
  struct ast_node *node = ast_compile(l);

  ast_print(node, 0);

  free(buffer);

  return 0;
}
