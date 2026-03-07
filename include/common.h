#ifndef _COMMON_H
#define _COMMON_H

#include <stdarg.h>
#include <type.h>

#define STDDIE stderr /* stream used in die   */

u0 die(i32 err, const char *fmt, ...);
char *fread_path(const char *path);

#endif
