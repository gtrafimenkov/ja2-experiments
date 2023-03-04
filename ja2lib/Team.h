#ifndef __TEAM_H
#define __TEAM_H

#include "LeanTypes.h"

// DEFINE TEAMS
#define OUR_TEAM 0
#define ENEMY_TEAM 1
#define CREATURE_TEAM 2
#define MILITIA_TEAM 3
#define CIV_TEAM 4
#define LAST_TEAM CIV_TEAM
#define PLAYER_PLAN 5

struct SoldierIDRange {
  i32 firstIndex;
  i32 lastIndex;
};

struct SoldierIDRange GetSoldierRangeForTeam(u8 teamID);

u8 GetTeamSide(u8 teamID);
void SetTeamSide(u8 teamID, u8 side);

#endif
