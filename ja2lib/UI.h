#ifndef __UI_H
#define __UI_H

#include "SGP/Types.h"

bool IsTacticalMode();

bool IsMapScreen();

const SGPRect* GetMapCenteringRect();

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc();

// Get merc from a character list by index.
// Valid indeces are [0..MAX_CHARACTER_COUNT).
struct SOLDIERTYPE* GetMercFromCharacterList(int index);

#endif
