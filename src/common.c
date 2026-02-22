#include <common.h>
#include <stdio.h>
#include <stdlib.h>

u0 die(i32 err, const char *fmt, ...) 
{
    va_list args;
    va_start(args, fmt); 
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(err);
}

char *freadbuf(const char *path) 
{
  FILE *f = fopen(path, "rb");

  if (NULL == f) 
  {
    die(1, "A fopen(%s) error occurred", path);
  }

  if (fseek(f, 0, SEEK_END) != 0) 
  {
    fclose(f);
    die(1, "A fseek() error occurred");
  }

  size_t size = ftell(f);
  if (size < 0) 
  {
    fclose(f);
    die(1, "A ftell() error occurred");
  }

  rewind(f);

  char *buffer = malloc(size);

  if (NULL == buffer) 
  {
    fclose(f);
    die(1, "A malloc() error occurred");
  }

  size_t read_bytes = fread(buffer, 1, size, f);

  if (read_bytes != (size_t)size) 
  {
    free(buffer);
    fclose(f);
    die(1, "A fread() error occurred");
  }

  fclose(f);

  return buffer;
}
