#ifndef __INPUT_MOUSE_POS
#define __INPUT_MOUSE_POS

#include <stdint.h>

struct MouseInput {
  uint16_t x;
  uint16_t y;
};

struct GameInput {
  struct MouseInput mouse;
};

// extern UINT16 gusMouseXPos;  // X position of the mouse on screen
// extern UINT16 gusMouseYPos;  // y position of the mouse on screen

#endif
