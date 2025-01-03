// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical.h"

#include "SGP/Types.h"
#include "SectorInfo.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierInitList.h"
#include "Team.h"

struct TacticalState {
  bool MilitiaRefreshRequired;
};

static struct TacticalState _st;

void TacticalMilitiaRefreshRequired() { _st.MilitiaRefreshRequired = true; }

void RemoveMilitiaFromTactical();

void ReinitMilitiaTactical() {
  if (_st.MilitiaRefreshRequired || gTacticalStatus.uiFlags & LOADING_SAVED_GAME) {
    _st.MilitiaRefreshRequired = false;
    RemoveMilitiaFromTactical();
    PrepareMilitiaForTactical();
  }
}

void RemoveMilitiaFromTactical() {
  SOLDIERINITNODE *curr;
  int32_t i;
  for (i = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
       i <= gTacticalStatus.Team[MILITIA_TEAM].bLastID; i++) {
    if (MercPtrs[i]->bActive) {
      TacticalRemoveSoldier(MercPtrs[i]->ubID);
    }
  }
  curr = gSoldierInitHead;
  while (curr) {
    if (curr->pBasicPlacement->bTeam == MILITIA_TEAM) {
      curr->pSoldier = NULL;
    }
    curr = curr->next;
  }
}

void PrepareMilitiaForTactical() {
  if (gbWorldSectorZ > 0) return;

  // Do we have a loaded sector?
  if (gWorldSectorX == 0 && gWorldSectorY == 0) return;

  struct MilitiaCount milCount = GetMilitiaInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
  AddSoldierInitListMilitia(milCount.green, milCount.regular, milCount.elite);
}
