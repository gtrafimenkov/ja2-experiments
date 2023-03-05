#include "Team.h"

#include "Tactical/Overhead.h"

struct SoldierIDRange GetSoldierRangeForTeam(u8 teamID) {
  struct SoldierIDRange res = {
      gTacticalStatus.Team[teamID].bFirstID,
      gTacticalStatus.Team[teamID].bLastID,
  };
  return res;
}

u8 GetTeamSide(u8 teamID) { return gTacticalStatus.Team[teamID].bSide; }
void SetTeamSide(u8 teamID, u8 side) { gTacticalStatus.Team[teamID].bSide = side; }

void GetTeamSoldiers(TeamID teamID, struct SoldierList* list) {
  const TacticalTeamType* team = &gTacticalStatus.Team[teamID];
  list->num = team->bLastID - team->bFirstID + 1;
  int counter = 0;
  for (int i = team->bFirstID; i <= team->bLastID; i++) {
    list->soldiers[counter++] = MercPtrs[i];
  }
}
