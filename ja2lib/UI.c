#include "UI.h"

#include "JAScreens.h"
#include "ScreenIDs.h"
#include "Strategic/MapScreenInterface.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/Menptr.h"

bool IsTacticalMode() { return guiCurrentScreen == GAME_SCREEN; }

static SGPRect mapCenteringRect = {0, 0, 640, INV_INTERFACE_START_Y};

const SGPRect* GetMapCenteringRect() { return &mapCenteringRect; }

bool IsMapScreen() { return guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN; }

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc() { return &Menptr[gusUIFullTargetID]; }

// Get merc from a character list by index.
// Valid indeces are [0..MAX_CHARACTER_COUNT).
struct SOLDIERTYPE* GetMercFromCharacterList(int index) {
  return &Menptr[gCharactersList[index].usSolID];
}
