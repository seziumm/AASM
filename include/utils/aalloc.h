/* aalloc.h */
#ifndef _AALLOC_H
#define _AALLOC_H

#include <utils/common.h>
#include <utils/type.h>
#include <stdlib.h>

/*
 * a_malloc() is a wrapper around malloc() that calls
 * die() if the requested size is zero or if the
 * allocation fails.
 */
static inline u0 *a_malloc(size_t n)
{
  if (unlikely(0 == n))
    die(1, "a_malloc: size is 0");

  u0 *p = malloc(n);
  if (unlikely(NULL == p))
    die(1, "a_malloc: out of memory allocating %zu bytes", n);
  return p;
}

/*
 * a_realloc() is a wrapper around realloc() that calls
 * die() if the requested size is zero or if the
 * reallocation fails.
 */
static inline u0 *a_realloc(u0 *ptr, size_t n)
{
  if (unlikely(0 == n))
    die(1, "a_realloc: size is 0");

  u0 *p = realloc(ptr, n);
  if (unlikely(NULL == p))
    die(1, "a_realloc: out of memory reallocating %zu bytes", n);
  return p;
}

/*
 * a_free() is a thin wrapper around free().
 * Accepts NULL safely — free(NULL) is a no-op per C standard.
 */
static inline u0 a_free(u0 *ptr)
{
  free(ptr);
}

#endif /* _AALLOC_H */
