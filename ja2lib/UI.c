#include "UI.h"

#include "JAScreens.h"
#include "ScreenIDs.h"
#include "Tactical/Interface.h"

bool IsTacticalMode() { return IsTacticalMode(); }

static SGPRect mapCenteringRect = {0, 0, 640, INV_INTERFACE_START_Y};

const SGPRect* GetMapCenteringRect() { return &mapCenteringRect; }
