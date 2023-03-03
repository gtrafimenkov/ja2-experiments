#ifndef __MOUSE_INPUT_H
#define __MOUSE_INPUT_H

#include <stdint.h>

struct MouseInput {
  uint16_t x;
  uint16_t y;
};

extern struct MouseInput XXX_GetMouseInput();

#endif
