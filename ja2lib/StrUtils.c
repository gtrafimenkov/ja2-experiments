// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "StrUtils.h"

// Copy a string safely.
// 0 character at the end of dest is always added.
void strcopy(char *dest, size_t destSize, const char *src) {
  if (destSize > 0) {
    strncpy(dest, src, destSize);
    dest[destSize - 1] = 0;
  }
}

bool strequal(const char *str1, const char *str2) { return strcmp(str1, str2) == 0; }
