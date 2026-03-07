#include <stdio.h>
#include <stdlib.h>
#include <lexer.h>

i32 main(i32 argc, char **argv)
{
  if (argc != 3)
  {
    die(1, "Usage: ./%s test.aasm out.bin\n", argv[0]);
    return 1;
  }

  char *buffer = fread_path(argv[1]);

  struct lexer *l = lexer_compile(buffer);

  lexer_print(l);

  lexer_free(&l);
  free(buffer);
  return 0;
}
