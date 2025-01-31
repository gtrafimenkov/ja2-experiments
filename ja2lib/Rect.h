#ifndef JA2_RECT_H
#define JA2_RECT_H

#include <stdint.h>

struct Rect {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
};

int32_t GetRectWidth(const struct Rect* r);
int32_t GetRectHeight(const struct Rect* r);

#endif  // JA2_RECT_H
