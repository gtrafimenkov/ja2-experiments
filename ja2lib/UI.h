#ifndef __UI_H
#define __UI_H

#include "SGP/Types.h"

bool IsTacticalMode();

// Are we on the map screen?
bool IsMapScreen();

// Another check for Are we on the map screen?
// It is not clear how it is different from IsMapScreen().
bool IsMapScreen_2();

const SGPRect* GetMapCenteringRect();

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc();

// Get merc from a character list by index.
// Valid indeces are [0..MAX_CHARACTER_COUNT).
struct SOLDIERTYPE* GetMercFromCharacterList(int index);

#endif
