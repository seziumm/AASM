/* common.c */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/common.h>
#include <utils/aalloc.h>

char *fread_path(const char *path)
{
  FILE *f = fopen(path, "rb");

  if (unlikely(NULL == f))
    die(1, "fread_path: fopen(%s) failed", path);

  if (unlikely(0 != fseek(f, 0, SEEK_END)))
  {
    fclose(f);
    die(1, "fread_path: fseek() failed");
  }

  long size = ftell(f);
  if (unlikely(size < 0))
  {
    fclose(f);
    die(1, "fread_path: ftell() failed");
  }

  rewind(f);

  char *buffer = a_malloc((size_t)size + 1);

  size_t read_bytes = fread(buffer, 1, (size_t)size, f);
  if (unlikely(read_bytes != (size_t)size))
  {
    a_free(buffer);
    fclose(f);
    die(1, "fread_path: fread() failed (got %zu, expected %ld)", read_bytes, size);
  }

  buffer[size] = '\0';
  fclose(f);
  return buffer;
}
