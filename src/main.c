#include "utils/a_alloc.h"
#include "utils/a_common.h"
#include <string.h>

/* Searches for the first occurrence of character (c) in a buffer (buf)
 * of exactly (len) bytes. Returns a pointer to the character if found,
 * NULL otherwise.
 */
char *a_findchar(const char* restrict buf, char c, size_t len)
{
  if (unlikely(len == 0 || buf == NULL)) return NULL;
  const char *ptr = buf;
  uint64_t target = (uint8_t)c * 0x0101010101010101ULL;
  while (len >= 8)
  {
    uint64_t b;
    memcpy(&b, ptr, 8);
    uint64_t mask = b ^ target;
    uint64_t tmp  = (mask - 0x0101010101010101ULL) & ~mask;
    tmp &= 0x8080808080808080ULL;
    if (tmp) return (void *)(ptr + (__builtin_ctzll(tmp) >> 3));
    ptr += 8;
    len -= 8;
  }
  if (len > 0)
  {
    uint64_t word = 0;
    memcpy(&word, ptr, len);
    uint64_t nr   = (8 - len) << 3;
    uint64_t mask = word ^ target;
    uint64_t tmp  = (mask - (0x0101010101010101ULL >> nr)) & ~mask;
    tmp &= (0x8080808080808080ULL >> nr);
    if (tmp) return (void *)(ptr + (__builtin_ctzll(tmp) >> 3));
  }
  return NULL;
}

/* Reads one line from (fd) into a heap-allocated buffer (*buf),
 * storing the length of the line (excluding '\n') in (*len).
 * The buffer is null-terminated. Stops at '\n', empty read, or EOF.
 * Returns the number of bytes in the line, or 0 on empty line / EOF.
 */
ssize_t a_getline(char **buf, size_t *len, FILE *fd)
{
  if (NULL == fd) return 0;
  size_t size     = 0;
  size_t capacity = 32;
  *buf = a_malloc(capacity);
  for (;;)
  {
    if (feof(fd)) goto done;
    size_t ret = fread(*buf + size, 1, capacity - size, fd);
    if (0 == ret) goto done;
    char *n_pos = a_findchar(*buf + size, '\n', ret);
    if (NULL != n_pos)
    {
      size_t consumed = (n_pos - (*buf + size)) + 1; /* +1 salta il '\n' */
      fseek(fd, -(long)(ret - consumed), SEEK_CUR);  /* riavvolge i byte non usati */
      size = n_pos - *buf;
      goto done;
    }
    size += ret;
    if (size == capacity)
    {
      capacity *= 2;
      *buf = a_realloc(*buf, capacity);
    }
  }
done:
  (*buf)[size] = '\0';
  *len = size;
  return size;
}

/* Opens the file at (file) with the given (mode).
 * Terminates the process with an error message if the file cannot be opened.
 * Returns the FILE pointer on success.
 */
FILE *a_fopen(const char *file, const char *mode)
{
  FILE *fd = fopen(file, mode);
  if (NULL == fd)
    die(1, "Error while opening file %s in \"%s\" mode", file, mode);
  return fd;
}

int main(int argc, const char **argv)
{
  if (argc < 2) {
    die(1, "usage: ./%s <in0.aasm> [in1.aasm ...]\n", argv[0]);
  }
  char   *buffer = NULL;
  size_t  len    = 0;
  FILE *fd = a_fopen(argv[1], "r");
  while (!feof(fd))
  {
    a_getline(&buffer, &len, fd);
    printf("%s\n", buffer);
    free(buffer);
    buffer = NULL;
  }
  free(buffer);
  fclose(fd);
  return 0;
}
