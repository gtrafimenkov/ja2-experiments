#include "UI.h"

#include "JAScreens.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/PreBattleInterface.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"

bool IsTacticalMode() { return guiCurrentScreen == GAME_SCREEN; }

static SGPRect mapCenteringRect = {0, 0, 640, INV_INTERFACE_START_Y};

const SGPRect* GetMapCenteringRect() { return &mapCenteringRect; }

bool IsMapScreen() { return guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN; }
bool IsMapScreen_2() { return guiCurrentScreen == MAP_SCREEN; }

// Get Merc associated with the context menu on tactical screen.
struct SOLDIERTYPE* GetTacticalContextMenuMerc() { return GetSoldierByID(gusUIFullTargetID); }

// Get merc from a character list by index.
// Valid indeces are [0..MAX_CHARACTER_COUNT).
struct SOLDIERTYPE* GetMercFromCharacterList(int index) {
  return GetSoldierByID(gCharactersList[index].usSolID);
}

void SwitchMapToMilitiaMode() {
  if (!fShowMilitia) {
    ToggleShowMilitiaMode();
  }
}

bool IsGoingToAutoresolve() { return gfAutomaticallyStartAutoResolve; }

// Return index of the character selected for assignment
i8 GetCharForAssignmentIndex() { return bSelectedAssignChar; }
