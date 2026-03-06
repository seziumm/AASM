#include <ast.h>
#include <lexer.h>
#include <common.h>
#include <sa.h>
#include <codegen.h>
#include <stdlib.h>
#include <stdio.h>


i32 main(i32 argc, char **argv)
{
  if (argc != 3)
  {
    die(1, "Usage: ./%s file.aasm out.bin\n", argv[0]);
    return 1;
  }

  char *buffer = freadbuf(argv[1]);

  /* ---- lex ---- */
  struct lexer *l = lexer_compile(buffer);

  /* ---- parse ---- */
  struct ast_node *program = ast_compile(l);
  ast_print(program, 0);

  /* ---- semantic analysis ---- */
  sa_check(program);
  printf("\nSA: OK\n\n");

  /* ---- code generation ---- */
  u32  base_addr = 0;
  u32  code_size = 0;
  u32 *code      = codegen_compile(program, &code_size);

  /* recover base address from first section */
  if (program->children_size > 0)
    base_addr = (u32)strtoul(program->children[0]->data.section.value, NULL, 0);

  codegen_print(code, code_size, base_addr);

  codegen_write_binary(code, code_size, argv[2]);
  printf("\nWrote %u words to '%s'\n", code_size, argv[2]);

  free(code);
  free(buffer);

  return 0;
}
