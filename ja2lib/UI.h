#ifndef __UI_H
#define __UI_H

#include "SGP/Types.h"

bool IsTacticalMode();

bool IsMapScreen();

const SGPRect* GetMapCenteringRect();

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc();

#endif
