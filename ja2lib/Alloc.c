#include "Alloc.h"

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void* MemAlloc(size_t size) { return malloc(size); }
void MemFree(void* ptr) { free(ptr); }
void* MemRealloc(void* ptr, size_t size) { return realloc(ptr, size); }

void MemZero(void* ptr, size_t size) { memset(ptr, 0, size); }

// Allocate memory and zero it.
void* MemAllocZero(size_t size) {
  void* p = MemAlloc(size);
  if (p != NULL) {
    memset(p, 0, size);
  }
  return p;
}

// asprintf is not available of windows
int my_asprintf(char** strp, const char* format, ...) {
  va_list args;

  va_start(args, format);
  int size = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (size < 0) {
    return -1;
  }

  *strp = (char*)malloc(size + 1);
  if (*strp == NULL) {
    return -1;
  }

  va_start(args, format);
  int result = vsnprintf(*strp, size + 1, format, args);
  va_end(args);

  return result;
}
