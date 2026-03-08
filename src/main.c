#include <codegen.h>
#include <parser.h>
#include <lexer.h>
#include <stdlib.h>

i32 main(i32 argc, char **argv)
{
  if (argc != 3)
    die(1, "Usage: ./%s <input.aasm> <out.bin>\n", argv[0]);

  char            *buffer = fread_path(argv[1]);
  struct lexer    *l      = lexer_compile(buffer);
  struct ast_node *root   = parser_build(l);

  codegen_compile(root, argv[2]);

  free(buffer);
  lexer_free(&l);
  ast_node_free(&root);
  return 0;
}
