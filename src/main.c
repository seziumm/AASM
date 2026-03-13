#include <utils/common.h>
#include <utils/aalloc.h>

i32 main(i32 argc, char **argv)
{
  if (argc != 3)
  {
    die(1, "Usage: ./%s <input.aasm> <out.bin>\n", argv[0]);
  }

  char *buffer = fread_path(argv[1]);

  a_free(buffer);
  return 0;
}
