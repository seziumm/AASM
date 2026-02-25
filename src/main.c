#include <ast.h>
#include <lexer.h>
#include <common.h>
#include <lc.h>
#include <stdlib.h>
#include <rv32/rv32_type.h>


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
  struct ast *a = ast_init(l);

  printf("\n ===== AST TREE =====\n\n");
  ast_print(a);

  lc_assemble(l, argv[1]);

  free(buffer);
  return 0;
}
