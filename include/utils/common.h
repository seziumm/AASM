/* common.h */
#ifndef _COMMON_H
#define _COMMON_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/type.h>

#define expect(expr) assert((expr))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

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
 * fread_path() reads the entire contents of the file
 * at the given path into a heap-allocated buffer and
 * returns it as a NUL-terminated string.
 * Calls die() on any I/O error. Caller must free().
 */
char *fread_path(const char *path);



#endif /* _COMMON_H */
