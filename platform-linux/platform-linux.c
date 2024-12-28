// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <wchar.h>
#include <wctype.h>

wchar_t *_wcsupr(wchar_t *str) {
  while (*str) {
    *str = towupper(*str);
    str++;
  }
  return str;
}
