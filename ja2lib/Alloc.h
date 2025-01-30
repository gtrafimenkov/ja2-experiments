#ifndef JA2_ALLOC
#define JA2_ALLOC

#include <stdint.h>

void* MemAlloc(size_t size);
void MemFree(void* ptr);
void* MemRealloc(void* ptr, size_t size);

void MemZero(void* ptr, size_t size);

int my_asprintf(char** strp, const char* format, ...);

#endif
