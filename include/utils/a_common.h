/* common.h */
#ifndef _COMMON_H
#define _COMMON_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils/a_type.h"

#define expect(expr) assert((expr))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define prefetch(x) __builtin_prefetch ((void *)(x), 0)

#define STDDIE stderr /* stream used by die()   */
#define STDDBG stderr /* stream used by debug() */

#define debugf(fmt, ...) \
  debugf_impl(fmt, ##__VA_ARGS__)
/*
 *  debugf()  —  print debug message to STDDBG
 *
 *  Prints a formatted debug message to STDDBG, appends a
 *  newline and flushes the stream.
 */
static inline u0 debugf_impl(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(STDDBG, fmt, args);
  va_end(args);
  fflush(STDDBG);
}

/*
 * die() prints a formatted error message to STDDIE,
 * appends a newline, flushes the stream, then exits
 * with the given error code. Never returns.
 */
static inline u0 die(i32 err, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(STDDIE, fmt, args);
  va_end(args);
  fprintf(STDDIE, "\n");
  fflush(STDDIE);
  exit(err);
}

/*
 * Opens each path in `paths`, reads its raw bytes, and concatenates
 * them into one heap-allocated buffer via a_realloc.
 *
 * The caller is responsible for freeing the returned buffer.
 * On any error (fopen, fstat, fread) the process aborts via die().
 *
 * Parameters:
 *   paths  - array of file paths to read
 *   n      - number of entries in `paths`
 *
 * Returns:
 *   pointer to the squished buffer (never NULL)
 */
char *squish_files(const char **paths, u64 n);

#endif /* _COMMON_H */
