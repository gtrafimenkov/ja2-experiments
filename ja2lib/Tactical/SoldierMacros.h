// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef SOLDIER_MACROS_H
#define SOLDIER_MACROS_H

#include "SGP/Types.h"
#include "Strategic/Assignments.h"
#include "Tactical/AnimationData.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/TeamTurns.h"

// MACROS
#define RPC_RECRUITED(p)        \
  ((p->ubProfile == NO_PROFILE) \
       ? FALSE                  \
       : (gMercProfiles[p->ubProfile].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED))

#define AM_AN_EPC(p)            \
  ((p->ubProfile == NO_PROFILE) \
       ? FALSE                  \
       : (gMercProfiles[p->ubProfile].ubMiscFlags & PROFILE_MISC_FLAG_EPCACTIVE))

#define AM_A_ROBOT(p) \
  ((p->ubProfile == NO_PROFILE) ? FALSE : (gMercProfiles[p->ubProfile].ubBodyType == ROBOTNOWEAPON))

#define OK_ENEMY_MERC(p) (!p->bNeutral && (p->bSide != gbPlayerNum) && p->bLife >= OKLIFE)

// Checks if our guy can be controllable .... checks bInSector, team, on duty, etc...
#define OK_CONTROLLABLE_MERC(p)                                                   \
  (p->bLife >= OKLIFE && p->bActive && p->bInSector && p->bTeam == gbPlayerNum && \
   p->bAssignment < ON_DUTY)

// Checks if our guy can be controllable .... checks bInSector, team, on duty, etc...
#define OK_INSECTOR_MERC(p)                                                       \
  (p->bLife >= OKLIFE && p->bActive && p->bInSector && p->bTeam == gbPlayerNum && \
   p->bAssignment < ON_DUTY)

// Checkf if our guy can be selected and is not in a position where our team has an interupt and he
// does not have one...
#define OK_INTERRUPT_MERC(p) ((INTERRUPT_QUEUED != 0) ? ((p->bMoved) ? FALSE : TRUE) : TRUE)

#define CREATURE_OR_BLOODCAT(p) ((p->uiStatusFlags & SOLDIER_MONSTER) || p->ubBodyType == BLOODCAT)

#define TANK(p) (p->ubBodyType == TANK_NE || p->ubBodyType == TANK_NW)

#define OK_ENTERABLE_VEHICLE(p) \
  ((p->uiStatusFlags & SOLDIER_VEHICLE) && !TANK(p) && p->bLife >= OKLIFE)

#endif
