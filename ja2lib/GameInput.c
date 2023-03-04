#include "GameInput.h"

struct GameInput g_gameInput;

struct GameInput* XXX_GetGameInput() { return &g_gameInput; }
struct MouseInput XXX_GetMouseInput() { return g_gameInput.mouse; }
