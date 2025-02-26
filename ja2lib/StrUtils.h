// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __STRUTILS_H
#define __STRUTILS_H

#include <stdbool.h>
#include <string.h>

// Copy a string safely.
// 0 character at the end of dest is always added.
extern void strcopy(char *dest, size_t destSize, const char *src);

bool strequal(const char *str1, const char *str2);

#endif
