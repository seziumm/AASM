#include <ast/ast_node.h>
#include <codegen.h>
#include <lexer.h>
#include <parser.h>
#include <utils/common.h>
#include <utils/aalloc.h>

i32 main(i32 argc, char **argv)
{
  if (argc != 3)
  {
    die(1, "Usage: ./%s <in.aasm> <out.mc>\n", argv[0]);
  }

  /* ---- Lex ---- */
  char         *buffer = fread_path(argv[1]);
  struct lexer *l      = lexer_compile(buffer);

  /* ---- Parse ---- */
  struct ast_node *root = parser_root(&l->table);

  /* ---- Debug: dump AST ---- */
  ast_node_print(root, 0);

  /* ---- Codegen ---- */
  struct codegen_ctx *cg = codegen_run(root, 0x00000000);

  codegen_print(cg);
  codegen_write(cg, argv[2]);

  /* ---- Cleanup ---- */
  codegen_free(&cg);
  ast_node_free(&root);
  lexer_free(&l);
  a_free(buffer);

  return 0;
}
