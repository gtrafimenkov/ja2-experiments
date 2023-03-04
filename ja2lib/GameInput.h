#ifndef __GAME_INPUT_H
#define __GAME_INPUT_H

#include <stdint.h>

#include "MouseInput.h"

struct GameInput {
  struct MouseInput mouse;
};

extern struct GameInput* XXX_GetGameInput();

#endif
