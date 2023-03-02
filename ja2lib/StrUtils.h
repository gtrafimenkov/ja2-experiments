#ifndef __STRUTILS_H
#define __STRUTILS_H

#include <string.h>

// Copy a string safely.
// 0 character at the end of dest is always added.
void strcopy(char *dest, size_t destSize, const char *src);

#endif
