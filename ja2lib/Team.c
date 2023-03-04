#include "Team.h"

#include "Tactical/Overhead.h"

struct SoldierIDRange GetSoldierRangeForTeam(u8 teamID) {
  struct SoldierIDRange res = {
    firstIndex : gTacticalStatus.Team[teamID].bFirstID,
    lastIndex : gTacticalStatus.Team[teamID].bLastID,
  };
  return res;
}

u8 GetTeamSide(u8 teamID) { return gTacticalStatus.Team[teamID].bSide; }
void SetTeamSide(u8 teamID, u8 side) { gTacticalStatus.Team[teamID].bSide = side; }
