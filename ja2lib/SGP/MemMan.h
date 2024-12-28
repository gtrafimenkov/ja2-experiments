// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _MEMMAN_H
#define _MEMMAN_H

#include <malloc.h>

#include "SGP/Types.h"

extern BOOLEAN InitializeMemoryManager(void);
extern void ShutdownMemoryManager(void);

#define MemAlloc(size) malloc((size))
#define MemFree(ptr) free((ptr))
#define MemRealloc(ptr, size) realloc((ptr), (size))

#endif
