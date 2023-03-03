#ifndef _GAME_INIT_OPTIONS_SCREEN_H_
#define _GAME_INIT_OPTIONS_SCREEN_H_

#include "GameInput.h"
#include "SGP/Types.h"

UINT32 GameInitOptionsScreenInit(void);
UINT32 GameInitOptionsScreenHandle(const struct GameInput *gameInput);
UINT32 GameInitOptionsScreenShutdown(void);

#endif
