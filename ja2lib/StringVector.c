#include "StringVector.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

struct StringVector *sv_new() {
  struct StringVector *vec = malloc(sizeof(struct StringVector));
  if (!vec) return NULL;
  vec->data = malloc(INITIAL_CAPACITY * sizeof(char *));
  if (!vec->data) {
    free(vec);
    return NULL;
  }
  vec->size = 0;
  vec->capacity = INITIAL_CAPACITY;
  return vec;
}

void sv_add_string_copy(struct StringVector *vec, const char *str) {
  if (vec->size >= vec->capacity) {
    size_t new_capacity = vec->capacity * 2;
    char **new_data = realloc(vec->data, new_capacity * sizeof(char *));
    if (!new_data) return;
    vec->data = new_data;
    vec->capacity = new_capacity;
  }
  vec->data[vec->size] = strdup(str);
  if (vec->data[vec->size]) vec->size++;
}

void sv_remove_string(struct StringVector *vec, size_t index) {
  if (index >= vec->size) return;
  free(vec->data[index]);
  for (size_t i = index; i < vec->size - 1; i++) {
    vec->data[i] = vec->data[i + 1];
  }
  vec->size--;
}

void sv_free(struct StringVector *vec) {
  if (!vec) return;
  for (size_t i = 0; i < vec->size; i++) {
    free(vec->data[i]);
  }
  free(vec->data);
  free(vec);
}
