#include "UI.h"

#include "JAScreens.h"
#include "ScreenIDs.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"

bool IsTacticalMode() { return guiCurrentScreen == GAME_SCREEN; }

static SGPRect mapCenteringRect = {0, 0, 640, INV_INTERFACE_START_Y};

const SGPRect* GetMapCenteringRect() { return &mapCenteringRect; }

bool IsMapScreen() { return guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN; }
