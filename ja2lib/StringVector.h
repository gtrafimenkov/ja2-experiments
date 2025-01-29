#ifndef JA2_STRING_VECTOR
#define JA2_STRING_VECTOR

#include <stdint.h>

struct StringVector {
  char **data;
  size_t size;
  size_t capacity;
};

struct StringVector *sv_new();
void sv_add_string_copy(struct StringVector *vec, const char *str);
void sv_remove_string(struct StringVector *vec, size_t index);
void sv_free(struct StringVector *vec);

#endif  // JA2_STRING_VECTOR
