#ifndef __TACTICAL_PLACEMENT_GUI_H
#define __TACTICAL_PLACEMENT_GUI_H

#include "MouseInput.h"
#include "SGP/Types.h"

struct MOUSE_REGION;
struct SOLDIERTYPE;

void InitTacticalPlacementGUI();
void TacticalPlacementHandle(const struct MouseInput mouse);

void HandleTacticalPlacementClicksInOverheadMap(struct MOUSE_REGION *reg, INT32 reason,
                                                const struct MouseInput mouse);

extern BOOLEAN gfTacticalPlacementGUIActive;
extern BOOLEAN gfEnterTacticalPlacementGUI;

extern struct SOLDIERTYPE *gpTacticalPlacementSelectedSoldier;
extern struct SOLDIERTYPE *gpTacticalPlacementHilightedSoldier;

// Saved value.  Contains the last choice for future battles.
extern UINT8 gubDefaultButton;

#endif
