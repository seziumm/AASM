#include <gen/codegen.h>
#include <token/tokenizer.h>
#include <utils/common.h>
#include <utils/aalloc.h>

i32 main(i32 argc, char **argv)
{
  if (argc != 3)
    die(1, "Usage: ./%s <input.aasm> <output.bin>", argv[0]);

  char             *src = fread_path(argv[1]);
  struct tokenizer *t   = tokenizer_compile(src);

  codegen_run(&t->table, 0x80000000u, argv[2]);

  tokenizer_free(&t);
  a_free(src);
  return 0;
}
