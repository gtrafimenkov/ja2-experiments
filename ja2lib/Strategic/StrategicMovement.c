// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/StrategicMovement.h"

#include <memory.h>
#include <stdlib.h>

#include "GameSettings.h"
#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/MemMan.h"
#include "SGP/Random.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/Campaign.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/InventoryChoosing.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/IsometricUtils.h"
#include "Town.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"

// the delay for a group about to arrive
#define ABOUT_TO_ARRIVE_DELAY 5

struct GROUP *gpGroupList;

struct GROUP *gpPendingSimultaneousGroup = NULL;

// is the bottom of the map panel dirty?
extern BOOLEAN fMapScreenBottomDirty;
extern BOOLEAN gfUsePersistantPBI;

#ifdef JA2BETAVERSION
extern BOOLEAN gfExitViewer;
void ValidateGroups(struct GROUP *pGroup);
#endif

extern BOOLEAN gubNumAwareBattles;
extern int8_t SquadMovementGroups[];
extern int8_t gubVehicleMovementGroups[];

BOOLEAN gfDelayAutoResolveStart = FALSE;

BOOLEAN gfRandomizingPatrolGroup = FALSE;

uint8_t gubNumGroupsArrivedSimultaneously = 0;

// Doesn't require text localization.  This is for debug strings only.
uint8_t gszTerrain[NUM_TRAVTERRAIN_TYPES][15] = {
    "TOWN",  "ROAD",  "PLAINS",        "SAND",     "SPARSE",   "DENSE",      "SWAMP",
    "WATER", "HILLS", "GROUNDBARRIER", "NS_RIVER", "EW_RIVER", "EDGEOFWORLD"};

BOOLEAN gfUndergroundTacticalTraversal = FALSE;

// remembers which player group is the Continue/Stop prompt about?  No need to save as long as you
// can't save while prompt ON
struct GROUP *gpGroupPrompting = NULL;

uint32_t uniqueIDMask[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Internal function manipulation prototypes

uint8_t AddGroupToList(struct GROUP *pGroup);

void HandleOtherGroupsArrivingSimultaneously(uint8_t ubSectorX, uint8_t ubSectorY,
                                             uint8_t ubSectorZ);
BOOLEAN PossibleToCoordinateSimultaneousGroupArrivals(struct GROUP *pGroup);

void HandleNonCombatGroupArrival(struct GROUP *pGroup, BOOLEAN fMainGroup, BOOLEAN fNeverLeft);

struct GROUP *gpInitPrebattleGroup = NULL;
void TriggerPrebattleInterface(uint8_t ubResult);

// Save the L.L. for the playerlist into the save game file
BOOLEAN SavePlayerGroupList(HWFILE hFile, struct GROUP *pGroup);

// Loads the LL for the playerlist from the savegame file
BOOLEAN LoadPlayerGroupList(HWFILE hFile, struct GROUP **pGroup);

BOOLEAN SaveEnemyGroupStruct(HWFILE hFile, struct GROUP *pGroup);
BOOLEAN LoadEnemyGroupStructFromSavedGame(HWFILE hFile, struct GROUP *pGroup);

BOOLEAN LoadWayPointList(HWFILE hFile, struct GROUP *pGroup);
BOOLEAN SaveWayPointList(HWFILE hFile, struct GROUP *pGroup);

extern void RandomMercInGroupSaysQuote(struct GROUP *pGroup, uint16_t usQuoteNum);

void SetLocationOfAllPlayerSoldiersInGroup(struct GROUP *pGroup, uint8_t sSectorX, uint8_t sSectorZ,
                                           int8_t bSectorZ);

// If there are bloodcats in this sector, then it internally
BOOLEAN TestForBloodcatAmbush(struct GROUP *pGroup);
void NotifyPlayerOfBloodcatBattle(uint8_t ubSectorX, uint8_t ubSectorY);

BOOLEAN GroupBetweenSectorsAndSectorXYIsInDifferentDirection(struct GROUP *pGroup,
                                                             uint8_t ubSectorX, uint8_t ubSectorY);

void CancelEmptyPersistentGroupMovement(struct GROUP *pGroup);

BOOLEAN HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(struct GROUP *pGroup);
BOOLEAN WildernessSectorWithAllProfiledNPCsNotSpokenWith(uint8_t sSectorX, uint8_t sSectorY,
                                                         int8_t bSectorZ);
void HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback(uint8_t ubExitValue);
void DelayEnemyGroupsIfPathsCross(struct GROUP *pPlayerGroup);

uint8_t NumberMercsInVehicleGroup(struct GROUP *pGroup);

// waiting for input from user
BOOLEAN gfWaitingForInput = FALSE;

// Player grouping functions
//.........................
// Creates a new player group, returning the unique ID of that group.  This is the first
// step before adding waypoints and members to the player group.
uint8_t CreateNewPlayerGroupDepartingFromSector(uint8_t ubSectorX, uint8_t ubSectorY) {
  struct GROUP *pNew;
  AssertMsg(ubSectorX >= 1 && ubSectorX <= 16,
            String("CreateNewPlayerGroup with out of range sectorX value of %d", ubSectorX));
  AssertMsg(ubSectorY >= 1 && ubSectorY <= 16,
            String("CreateNewPlayerGroup with out of range sectorY value of %d", ubSectorY));
  pNew = (struct GROUP *)MemAlloc(sizeof(struct GROUP));
  AssertMsg(pNew, "MemAlloc failure during CreateNewPlayerGroup.");
  memset(pNew, 0, sizeof(struct GROUP));
  pNew->pPlayerList = NULL;
  pNew->pWaypoints = NULL;
  pNew->ubSectorX = pNew->ubNextX = ubSectorX;
  pNew->ubSectorY = pNew->ubNextY = ubSectorY;
  pNew->ubOriginalSector = (uint8_t)GetSectorID8(ubSectorX, ubSectorY);
  pNew->fPlayer = TRUE;
  pNew->ubMoveType = ONE_WAY;
  pNew->ubNextWaypointID = 0;
  pNew->ubFatigueLevel = 100;
  pNew->ubRestAtFatigueLevel = 0;
  pNew->ubTransportationMask = FOOT;
  pNew->fVehicle = FALSE;
  pNew->ubCreatedSectorID = pNew->ubOriginalSector;
  pNew->ubSectorIDOfLastReassignment = 255;

  return AddGroupToList(pNew);
}

uint8_t CreateNewVehicleGroupDepartingFromSector(uint8_t ubSectorX, uint8_t ubSectorY,
                                                 uint32_t uiUNISEDVehicleId) {
  struct GROUP *pNew;
  AssertMsg(ubSectorX >= 1 && ubSectorX <= 16,
            String("CreateNewVehicleGroup with out of range sectorX value of %d", ubSectorX));
  AssertMsg(ubSectorY >= 1 && ubSectorY <= 16,
            String("CreateNewVehicleGroup with out of range sectorY value of %d", ubSectorY));
  pNew = (struct GROUP *)MemAlloc(sizeof(struct GROUP));
  AssertMsg(pNew, "MemAlloc failure during CreateNewVehicleGroup.");
  memset(pNew, 0, sizeof(struct GROUP));
  pNew->pWaypoints = NULL;
  pNew->ubSectorX = pNew->ubNextX = ubSectorX;
  pNew->ubSectorY = pNew->ubNextY = ubSectorY;
  pNew->ubOriginalSector = (uint8_t)GetSectorID8(ubSectorX, ubSectorY);
  pNew->ubMoveType = ONE_WAY;
  pNew->ubNextWaypointID = 0;
  pNew->ubFatigueLevel = 100;
  pNew->ubRestAtFatigueLevel = 0;
  pNew->fVehicle = TRUE;
  pNew->fPlayer = TRUE;
  pNew->pPlayerList = NULL;
  pNew->ubCreatedSectorID = pNew->ubOriginalSector;
  pNew->ubSectorIDOfLastReassignment = 255;

  // get the type
  pNew->ubTransportationMask = CAR;

  return (AddGroupToList(pNew));
}

// Allows you to add players to the group.
BOOLEAN AddPlayerToGroup(uint8_t ubGroupID, struct SOLDIERTYPE *pSoldier) {
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer, *curr;
  pGroup = GetGroup(ubGroupID);
  Assert(pGroup);
  pPlayer = (PLAYERGROUP *)MemAlloc(sizeof(PLAYERGROUP));
  Assert(pPlayer);
  AssertMsg(pGroup->fPlayer, "Attempting AddPlayerToGroup() on an ENEMY group!");
  pPlayer->pSoldier = pSoldier;
  pPlayer->ubProfileID = GetSolProfile(pSoldier);
  pPlayer->ubID = GetSolID(pSoldier);
  pPlayer->bFlags = 0;
  pPlayer->next = NULL;

  if (!pGroup->pPlayerList) {
    pGroup->pPlayerList = pPlayer;
    pGroup->ubGroupSize = 1;
    pGroup->ubPrevX = (uint8_t)((pSoldier->ubPrevSectorID % 16) + 1);
    pGroup->ubPrevY = (uint8_t)((pSoldier->ubPrevSectorID / 16) + 1);
    pGroup->ubSectorX = (uint8_t)GetSolSectorX(pSoldier);
    pGroup->ubSectorY = (uint8_t)pSoldier->sSectorY;
    pGroup->ubSectorZ = (uint8_t)pSoldier->bSectorZ;

    // set group id
    pSoldier->ubGroupID = ubGroupID;

    return TRUE;
  } else {
    curr = pGroup->pPlayerList;
    pSoldier->ubNumTraversalsAllowedToMerge = curr->pSoldier->ubNumTraversalsAllowedToMerge;
    pSoldier->ubDesiredSquadAssignment = curr->pSoldier->ubDesiredSquadAssignment;
    while (curr->next) {
      if (curr->ubProfileID == GetSolProfile(pSoldier))
        AssertMsg(0, String("Attempting to add an already existing merc to group (ubProfile=%d).",
                            GetSolProfile(pSoldier)));
      curr = curr->next;
    }
    curr->next = pPlayer;

    // set group id
    pSoldier->ubGroupID = ubGroupID;

    pGroup->ubGroupSize++;
    return TRUE;
  }
}

// remove all grunts from player mvt grp
BOOLEAN RemoveAllPlayersFromGroup(uint8_t ubGroupId) {
  struct GROUP *pGroup;

  // grab group id
  pGroup = GetGroup(ubGroupId);

  // init errors checks
  AssertMsg(pGroup,
            String("Attempting to RemovePlayerFromGroup( %d ) from non-existant group", ubGroupId));

  return RemoveAllPlayersFromPGroup(pGroup);
}

BOOLEAN RemoveAllPlayersFromPGroup(struct GROUP *pGroup) {
  PLAYERGROUP *curr;

  AssertMsg(pGroup->fPlayer, "Attempting RemovePlayerFromGroup() on an ENEMY group!");

  curr = pGroup->pPlayerList;
  while (curr) {
    pGroup->pPlayerList = pGroup->pPlayerList->next;

    curr->pSoldier->ubPrevSectorID = (uint8_t)GetSectorID8(pGroup->ubPrevX, pGroup->ubPrevY);
    curr->pSoldier->ubGroupID = 0;

    MemFree(curr);

    curr = pGroup->pPlayerList;
  }
  pGroup->ubGroupSize = 0;

  if (!pGroup->fPersistant) {  // remove the empty group
    RemovePGroup(pGroup);
  } else {
    CancelEmptyPersistentGroupMovement(pGroup);
  }

  return TRUE;
}

BOOLEAN RemovePlayerFromPGroup(struct GROUP *pGroup, struct SOLDIERTYPE *pSoldier) {
  PLAYERGROUP *prev, *curr;
  AssertMsg(pGroup->fPlayer, "Attempting RemovePlayerFromGroup() on an ENEMY group!");

  curr = pGroup->pPlayerList;

  if (!curr) {
    return FALSE;
  }

  if (curr->pSoldier == pSoldier) {  // possibly the only node
    pGroup->pPlayerList = pGroup->pPlayerList->next;

    // delete the node
    MemFree(curr);

    // process info for soldier
    pGroup->ubGroupSize--;
    pSoldier->ubPrevSectorID = (uint8_t)GetSectorID8(pGroup->ubPrevX, pGroup->ubPrevY);
    pSoldier->ubGroupID = 0;

    // if there's nobody left in the group
    if (pGroup->ubGroupSize == 0) {
      if (!pGroup->fPersistant) {  // remove the empty group
        RemovePGroup(pGroup);
      } else {
        CancelEmptyPersistentGroupMovement(pGroup);
      }
    }

    return TRUE;
  }
  prev = NULL;

  while (curr) {  // definately more than one node

    if (curr->pSoldier == pSoldier) {
      // detach and delete the node
      if (prev) {
        prev->next = curr->next;
      }
      MemFree(curr);

      // process info for soldier
      pSoldier->ubGroupID = 0;
      pGroup->ubGroupSize--;
      pSoldier->ubPrevSectorID = (uint8_t)GetSectorID8(pGroup->ubPrevX, pGroup->ubPrevY);

      return TRUE;
    }

    prev = curr;
    curr = curr->next;
  }

  // !curr
  return FALSE;
}

BOOLEAN RemovePlayerFromGroup(uint8_t ubGroupID, struct SOLDIERTYPE *pSoldier) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);

  // KM : August 6, 1999 Patch fix
  //     Because the release build has no assertions, it was still possible for the group to be
  //     null, causing a crash.  Instead of crashing, it'll simply return false.
  if (!pGroup) {
    return FALSE;
  }
  // end

  AssertMsg(pGroup, String("Attempting to RemovePlayerFromGroup( %d, %d ) from non-existant group",
                           ubGroupID, GetSolProfile(pSoldier)));

  return RemovePlayerFromPGroup(pGroup, pSoldier);
}

BOOLEAN GroupReversingDirectionsBetweenSectors(struct GROUP *pGroup, uint8_t ubSectorX,
                                               uint8_t ubSectorY, BOOLEAN fBuildingWaypoints) {
  // if we're not between sectors, or we are but we're continuing in the same direction as before
  if (!GroupBetweenSectorsAndSectorXYIsInDifferentDirection(pGroup, ubSectorX, ubSectorY)) {
    // then there's no need to reverse directions
    return FALSE;
  }

  // The new direction is reversed, so we have to go back to the sector we just left.

  // Search for the arrival event, and kill it!
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // Adjust the information in the group to reflect the new movement.
  pGroup->ubPrevX = pGroup->ubNextX;
  pGroup->ubPrevY = pGroup->ubNextY;
  pGroup->ubNextX = pGroup->ubSectorX;
  pGroup->ubNextY = pGroup->ubSectorY;
  pGroup->ubSectorX = pGroup->ubPrevX;
  pGroup->ubSectorY = pGroup->ubPrevY;

  if (pGroup->fPlayer) {
    // ARM: because we've changed the group's ubSectoryX and ubSectorY, we must now also go and
    // change the sSectorX and sSectorY of all the soldiers in this group so that they stay in
    // synch.  Otherwise pathing and movement problems will result since the group is in one place
    // while the merc is in another...
    SetLocationOfAllPlayerSoldiersInGroup(pGroup, pGroup->ubSectorX, pGroup->ubSectorY, 0);
  }

  // IMPORTANT: The traverse time doesn't change just because we reverse directions!  It takes the
  // same time no matter which direction you're going in!  This becomes critical in case the player
  // reverse directions again before moving!

  // The time it takes to arrive there will be exactly the amount of time we have been moving away
  // from it.
  SetGroupArrivalTime(pGroup,
                      pGroup->uiTraverseTime - pGroup->uiArrivalTime + GetWorldTotalMin() * 2);

  // if they're not already there
  if (pGroup->uiArrivalTime > GetWorldTotalMin()) {
    // Post the replacement event to move back to the previous sector!
    AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID);

    if (pGroup->fPlayer) {
      if ((pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY) > GetWorldTotalMin()) {
        // Post the about to arrive event
        AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE,
                          pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID);
      }
    }
  } else {
    // IMPORTANT: this can't be called during RebuildWayPointsForGroupPath(), since it will clear
    // the mercpath prematurely by assuming the mercs are now at their final destination when only
    // the first waypoint is in place!!! To handle this situation, RebuildWayPointsForGroupPath()
    // will issue it's own call after it's ready for it.
    if (!fBuildingWaypoints) {
      // never really left.  Must set check for battle TRUE in order for
      // HandleNonCombatGroupArrival() to run!
      GroupArrivedAtSector(pGroup->ubGroupID, TRUE, TRUE);
    }
  }

  return TRUE;
}

BOOLEAN GroupBetweenSectorsAndSectorXYIsInDifferentDirection(struct GROUP *pGroup,
                                                             uint8_t ubSectorX, uint8_t ubSectorY) {
  int32_t currDX, currDY, newDX, newDY;
  uint8_t ubNumUnalignedAxes = 0;

  if (!pGroup->fBetweenSectors) return (FALSE);

  // Determine the direction the group is currently traveling in
  currDX = pGroup->ubNextX - pGroup->ubSectorX;
  currDY = pGroup->ubNextY - pGroup->ubSectorY;

  // Determine the direction the group would need to travel in to reach the given sector
  newDX = ubSectorX - pGroup->ubSectorX;
  newDY = ubSectorY - pGroup->ubSectorY;

  // clip the new dx/dy values to +/- 1
  if (newDX) {
    ubNumUnalignedAxes++;
    newDX /= abs(newDX);
  }
  if (newDY) {
    ubNumUnalignedAxes++;
    newDY /= abs(newDY);
  }

  // error checking
  if (ubNumUnalignedAxes > 1) {
    AssertMsg(FALSE, String("Checking a diagonal move for direction change, groupID %d. AM-0",
                            pGroup->ubGroupID));
    return FALSE;
  }

  // Compare the dx/dy's.  If they're exactly the same, group is travelling in the same direction as
  // before, so we're not changing directions. Note that 90-degree orthogonal changes are considered
  // changing direction, as well as the full 180-degree reversal. That's because the party must
  // return to the previous sector in each of those cases, too.
  if (currDX == newDX && currDY == newDY) return (FALSE);

  // yes, we're between sectors, and we'd be changing direction to go to the given sector
  return (TRUE);
}

// Appends a waypoint to the end of the list.  Waypoint MUST be on the
// same horizontal or vertical level as the last waypoint added.
BOOLEAN AddWaypointToPGroup(struct GROUP *pGroup, uint8_t ubSectorX,
                            uint8_t ubSectorY)  // Same, but overloaded
{
  WAYPOINT *pWay;
  uint8_t ubNumAlignedAxes = 0;
  BOOLEAN fReversingDirection = FALSE;

  AssertMsg(ubSectorX >= 1 && ubSectorX <= 16,
            String("AddWaypointToPGroup with out of range sectorX value of %d", ubSectorX));
  AssertMsg(ubSectorY >= 1 && ubSectorY <= 16,
            String("AddWaypointToPGroup with out of range sectorY value of %d", ubSectorY));

  if (!pGroup) return FALSE;

  // At this point, we have the group, and a valid coordinate.  Now we must
  // determine that this waypoint will be aligned exclusively to either the x or y axis of
  // the last waypoint in the list.
  pWay = pGroup->pWaypoints;
  if (!pWay) {
    if (GroupReversingDirectionsBetweenSectors(pGroup, ubSectorX, ubSectorY, TRUE)) {
      if (pGroup->fPlayer) {
        // because we reversed, we must add the new current sector back at the head of everyone's
        // mercpath
        AddSectorToFrontOfMercPathForAllSoldiersInGroup(pGroup, pGroup->ubSectorX,
                                                        pGroup->ubSectorY);
      }

      // Very special case that requiring specific coding.  Check out the comments
      // at the above function for more information.
      fReversingDirection = TRUE;
      // ARM:  Kris - new rulez.  Must still fall through and add a waypoint anyway!!!
    } else {  // No waypoints, so compare against the current location.
      if (pGroup->ubSectorX == ubSectorX) {
        ubNumAlignedAxes++;
      }
      if (pGroup->ubSectorY == ubSectorY) {
        ubNumAlignedAxes++;
      }
    }
  } else {  // we do have a waypoint list, so go to the last entry
    while (pWay->next) {
      pWay = pWay->next;
    }
    // now, we are pointing to the last waypoint in the list
    if (pWay->x == ubSectorX) {
      ubNumAlignedAxes++;
    }
    if (pWay->y == ubSectorY) {
      ubNumAlignedAxes++;
    }
  }

  if (!fReversingDirection) {
    if (ubNumAlignedAxes == 0) {
      AssertMsg(FALSE, String("Invalid DIAGONAL waypoint being added for groupID %d. AM-0",
                              pGroup->ubGroupID));
      return FALSE;
    }

    if (ubNumAlignedAxes >= 2) {
      AssertMsg(FALSE, String("Invalid IDENTICAL waypoint being added for groupID %d. AM-0",
                              pGroup->ubGroupID));
      return FALSE;
    }

    // has to be different in exactly 1 axis to be a valid new waypoint
    Assert(ubNumAlignedAxes == 1);
  }

  if (!pWay) {  // We are adding the first waypoint.
    pGroup->pWaypoints = (WAYPOINT *)MemAlloc(sizeof(WAYPOINT));
    pWay = pGroup->pWaypoints;
  } else {  // Add the waypoint to the end of the list
    pWay->next = (WAYPOINT *)MemAlloc(sizeof(WAYPOINT));
    pWay = pWay->next;
  }

  AssertMsg(pWay, "Failed to allocate memory for waypoint.");

  // Fill in the information for the new waypoint.
  pWay->x = ubSectorX;
  pWay->y = ubSectorY;
  pWay->next = NULL;

  // IMPORTANT:
  // The first waypoint added actually initiates the group's movement to the next sector.
  if (pWay == pGroup->pWaypoints) {
    // don't do this if we have reversed directions!!!  In that case, the required work has already
    // been done back there
    if (!fReversingDirection) {
      // We need to calculate the next sector the group is moving to and post an event for it.
      InitiateGroupMovementToNextSector(pGroup);
    }
  }

  if (pGroup->fPlayer) {
    PLAYERGROUP *curr;
    // Also, nuke any previous "tactical traversal" information.
    curr = pGroup->pPlayerList;
    while (curr) {
      curr->pSoldier->ubStrategicInsertionCode = 0;
      curr = curr->next;
    }
  }

  return TRUE;
}

BOOLEAN AddWaypointToGroup(uint8_t ubGroupID, uint8_t ubSectorX, uint8_t ubSectorY) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);
  return AddWaypointToPGroup(pGroup, ubSectorX, ubSectorY);
}

// NOTE: This does NOT expect a strategic sector ID
BOOLEAN AddWaypointIDToGroup(uint8_t ubGroupID, uint8_t ubSectorID) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);
  return AddWaypointIDToPGroup(pGroup, ubSectorID);
}

// NOTE: This does NOT expect a strategic sector ID
BOOLEAN AddWaypointIDToPGroup(struct GROUP *pGroup, uint8_t ubSectorID) {
  uint8_t ubSectorX, ubSectorY;
  ubSectorX = SectorID8_X(ubSectorID);
  ubSectorY = SectorID8_Y(ubSectorID);
  return AddWaypointToPGroup(pGroup, ubSectorX, ubSectorY);
}

BOOLEAN AddWaypointStrategicIDToGroup(uint8_t ubGroupID, uint32_t uiSectorID) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);
  return AddWaypointStrategicIDToPGroup(pGroup, uiSectorID);
}

BOOLEAN AddWaypointStrategicIDToPGroup(struct GROUP *pGroup, uint32_t uiSectorID) {
  uint8_t ubSectorX, ubSectorY;
  ubSectorX = SectorID16_X(uiSectorID);
  ubSectorY = SectorID16_Y(uiSectorID);
  return AddWaypointToPGroup(pGroup, ubSectorX, ubSectorY);
}

// Enemy grouping functions -- private use by the strategic AI.
//............................................................
struct GROUP *CreateNewEnemyGroupDepartingFromSector(uint32_t uiSector, uint8_t ubNumAdmins,
                                                     uint8_t ubNumTroops, uint8_t ubNumElites) {
  struct GROUP *pNew;
  AssertMsg(uiSector >= 0 && uiSector <= 255,
            String("CreateNewEnemyGroup with out of range value of %d", uiSector));
  pNew = (struct GROUP *)MemAlloc(sizeof(struct GROUP));
  AssertMsg(pNew, "MemAlloc failure during CreateNewEnemyGroup.");
  memset(pNew, 0, sizeof(struct GROUP));
  pNew->pEnemyGroup = (ENEMYGROUP *)MemAlloc(sizeof(ENEMYGROUP));
  AssertMsg(pNew->pEnemyGroup, "MemAlloc failure during enemy group creation.");
  memset(pNew->pEnemyGroup, 0, sizeof(ENEMYGROUP));
  pNew->pWaypoints = NULL;
  pNew->ubSectorX = SectorID8_X(uiSector);
  pNew->ubSectorY = SectorID8_Y(uiSector);
  pNew->ubOriginalSector = (uint8_t)uiSector;
  pNew->fPlayer = FALSE;
  pNew->ubMoveType = CIRCULAR;
  pNew->ubNextWaypointID = 0;
  pNew->ubFatigueLevel = 100;
  pNew->ubRestAtFatigueLevel = 0;
  pNew->pEnemyGroup->ubNumAdmins = ubNumAdmins;
  pNew->pEnemyGroup->ubNumTroops = ubNumTroops;
  pNew->pEnemyGroup->ubNumElites = ubNumElites;
  pNew->ubGroupSize = (uint8_t)(ubNumTroops + ubNumElites);
  pNew->ubTransportationMask = FOOT;
  pNew->fVehicle = FALSE;
  pNew->ubCreatedSectorID = pNew->ubOriginalSector;
  pNew->ubSectorIDOfLastReassignment = 255;

#ifdef JA2BETAVERSION
  {
    wchar_t str[512];
    if (PlayerMercsInSector(pNew->ubSectorX, pNew->ubSectorY, 0) ||
        CountAllMilitiaInSector(pNew->ubSectorX, pNew->ubSectorY)) {
      swprintf(str, ARR_SIZE(str),
               L"Attempting to send enemy troops from player occupied location.  "
               L"Please ALT+TAB out of the game before doing anything else and send 'Strategic "
               L"Decisions.txt' "
               L"and this message.  You'll likely need to revert to a previous save.  If you can "
               L"reproduce this "
               L"with a save close to this event, that would really help me! -- KM:0");
      DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
    } else if (pNew->ubGroupSize > 25) {
      swprintf(str, ARR_SIZE(str),
               L"Strategic AI warning:  Creating an enemy group containing %d soldiers "
               L"(%d admins, %d troops, %d elites) in sector %c%d.  This message is a temporary "
               L"test message "
               L"to evaluate a potential problems with very large enemy groups.",
               pNew->ubGroupSize, ubNumAdmins, ubNumTroops, ubNumElites, pNew->ubSectorY + 'A' - 1,
               pNew->ubSectorX);
      DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
    }
  }
#endif

  if (AddGroupToList(pNew)) return pNew;
  return NULL;
}

// INTERNAL LIST MANIPULATION FUNCTIONS

// When adding any new group to the list, this is what must be done:
// 1)  Find the first unused ID (unique)
// 2)  Assign that ID to the new group
// 3)  Insert the group at the end of the list.
uint8_t AddGroupToList(struct GROUP *pGroup) {
  struct GROUP *curr;
  uint32_t bit, index, mask;
  uint8_t ID = 0;
  // First, find a unique ID
  while (++ID) {
    index = ID / 32;
    bit = ID % 32;
    mask = 1 << bit;
    if (!(uniqueIDMask[index] & mask)) {  // found a free ID
      pGroup->ubGroupID = ID;
      uniqueIDMask[index] += mask;
      // add group to list now.
      curr = gpGroupList;
      if (curr) {  // point to the last item in list.
        while (curr->next) curr = curr->next;
        curr->next = pGroup;
      } else  // new list
        gpGroupList = pGroup;
      pGroup->next = NULL;
      return ID;
    }
  }
  return FALSE;
}

void RemoveGroupIdFromList(uint8_t ubId) {
  struct GROUP *pGroup;

  if (ubId == 0) {
    // no group, leave
    return;
  }

  // get group
  pGroup = GetGroup(ubId);

  // is there in fact a group?
  Assert(pGroup);

  // now remove this group
  RemoveGroupFromList(pGroup);
}
// Destroys the waypoint list, detaches group from list, then deallocated the memory for the group
void RemoveGroupFromList(struct GROUP *pGroup) {
  struct GROUP *curr, *temp;
  curr = gpGroupList;
  if (!curr) return;
  if (curr == pGroup) {  // Removing head
    gpGroupList = curr->next;
  } else
    while (curr->next) {           // traverse the list
      if (curr->next == pGroup) {  // the next node is the one we want to remove
        temp = curr;
        // curr now points to the nod we want to remove
        curr = curr->next;
        // detach the node from the list
        temp->next = curr->next;
        break;
      }
      curr = curr->next;
    }

  if (curr == pGroup) {  // we found the group, so now remove it.
    uint32_t bit, index, mask;

    // clear the unique group ID
    index = pGroup->ubGroupID / 32;
    bit = pGroup->ubGroupID % 32;
    mask = 1 << bit;

    if (!(uniqueIDMask[index] & mask)) {
      mask = mask;
    }

    uniqueIDMask[index] -= mask;

    MemFree(curr);
    curr = NULL;
  }
}

struct GROUP *GetGroup(uint8_t ubGroupID) {
  struct GROUP *curr;
  curr = gpGroupList;
  while (curr) {
    if (curr->ubGroupID == ubGroupID) return curr;
    curr = curr->next;
  }
  return NULL;
}

void HandleImportantPBIQuote(struct SOLDIERTYPE *pSoldier, struct GROUP *pInitiatingBattleGroup) {
  // wake merc up for THIS quote
  if (pSoldier->fMercAsleep) {
    TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_ENEMY_PRESENCE,
                                              DIALOGUE_SPECIAL_EVENT_SLEEP, 0, 0);
    TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_ENEMY_PRESENCE,
                                              DIALOGUE_SPECIAL_EVENT_BEGINPREBATTLEINTERFACE,
                                              (uintptr_t)pInitiatingBattleGroup, 0);
    TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_ENEMY_PRESENCE,
                                              DIALOGUE_SPECIAL_EVENT_SLEEP, 1, 0);
  } else {
    TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_ENEMY_PRESENCE,
                                              DIALOGUE_SPECIAL_EVENT_BEGINPREBATTLEINTERFACE,
                                              (uintptr_t)pInitiatingBattleGroup, 0);
  }
}

// If this is called, we are setting the game up to bring up the prebattle interface.  Before doing
// so, one of the involved mercs will pipe up.  When he is finished, we automatically go into the
// mapscreen, regardless of the mode we are in.
void PrepareForPreBattleInterface(struct GROUP *pPlayerDialogGroup,
                                  struct GROUP *pInitiatingBattleGroup) {
  // ATE; Changed alogrithm here...
  // We first loop through the group and save ubID's ov valid guys to talk....
  // ( Can't if sleeping, unconscious, and EPC, etc....
  uint8_t ubMercsInGroup[20] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pSoldier;
  PLAYERGROUP *pPlayer;

  if (fDisableMapInterfaceDueToBattle) {
    AssertMsg(
        0,
        "fDisableMapInterfaceDueToBattle is set before attempting to bring up PBI.  Please send "
        "PRIOR save if possible and details on anything that just happened before this battle.");
    return;
  }

  // Pipe up with quote...
  AssertMsg(pPlayerDialogGroup, "Didn't get a player dialog group for prebattle interface.");

  pPlayer = pPlayerDialogGroup->pPlayerList;
  AssertMsg(pPlayer,
            String("Player group %d doesn't have *any* players in it!  (Finding dialog group)",
                   pPlayerDialogGroup->ubGroupID));

  while (pPlayer != NULL) {
    pSoldier = pPlayer->pSoldier;

    if (pSoldier->bLife >= OKLIFE && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
        !AM_A_ROBOT(pSoldier) && !AM_AN_EPC(pSoldier)) {
      ubMercsInGroup[ubNumMercs] = GetSolID(pSoldier);
      ubNumMercs++;
    }

    pPlayer = pPlayer->next;
  }

  // Set music
  SetMusicMode(MUSIC_TACTICAL_ENEMYPRESENT);

  if ((gfTacticalTraversal && pInitiatingBattleGroup == gpTacticalTraversalGroup) ||
      (pInitiatingBattleGroup && !pInitiatingBattleGroup->fPlayer &&
       pInitiatingBattleGroup->ubSectorX == gWorldSectorX &&
       pInitiatingBattleGroup->ubSectorY == gWorldSectorY &&
       !gbWorldSectorZ)) {  // At least say quote....
    if (ubNumMercs > 0) {
      if (pPlayerDialogGroup->uiFlags & GROUPFLAG_JUST_RETREATED_FROM_BATTLE) {
        gfCantRetreatInPBI = TRUE;
      }

      ubChosenMerc = (uint8_t)Random(ubNumMercs);

      pSoldier = MercPtrs[ubMercsInGroup[ubChosenMerc]];
      gpTacticalTraversalChosenSoldier = pSoldier;

      if (!gfTacticalTraversal) {
        HandleImportantPBIQuote(pSoldier, pInitiatingBattleGroup);
      }

      InterruptTime();
      PauseGame();
      LockPauseState(11);

      if (!gfTacticalTraversal) fDisableMapInterfaceDueToBattle = TRUE;
    }
    return;
  }

  // Randomly pick a valid merc from the list we have created!
  if (ubNumMercs > 0) {
    if (pPlayerDialogGroup->uiFlags & GROUPFLAG_JUST_RETREATED_FROM_BATTLE) {
      gfCantRetreatInPBI = TRUE;
    }

    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    pSoldier = MercPtrs[ubMercsInGroup[ubChosenMerc]];

    HandleImportantPBIQuote(pSoldier, pInitiatingBattleGroup);
    InterruptTime();
    PauseGame();
    LockPauseState(12);

    // disable exit from mapscreen and what not until face done talking
    fDisableMapInterfaceDueToBattle = TRUE;
  } else {
    // ATE: What if we have unconscious guys, etc....
    // We MUST start combat, but donot play quote...
    InitPreBattleInterface(pInitiatingBattleGroup, TRUE);
  }
}

#ifdef JA2BETAVERSION
extern void ValidatePlayersAreInOneGroupOnly();
#endif

BOOLEAN CheckConditionsForBattle(struct GROUP *pGroup) {
  struct GROUP *curr;
  struct GROUP *pPlayerDialogGroup = NULL;
  PLAYERGROUP *pPlayer;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fBattlePending = FALSE;
  BOOLEAN fAliveMerc = FALSE;
  BOOLEAN fMilitiaPresent = FALSE;
  BOOLEAN fCombatAbleMerc = FALSE;
  BOOLEAN fBloodCatAmbush = FALSE;

  if (gfWorldLoaded) {  // look for people arriving in the currently loaded sector.  This handles
                        // reinforcements.
    curr = FindMovementGroupInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, TRUE);
    if (!gbWorldSectorZ &&
        PlayerMercsInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ) &&
        pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY &&
        curr) {  // Reinforcements have arrived!
#ifdef JA2BETAVERSION
      if (guiCurrentScreen == AIVIEWER_SCREEN) {
        gfExitViewer = TRUE;
      }
#endif
      if (gTacticalStatus.fEnemyInSector) {
        HandleArrivalOfReinforcements(pGroup);
        return (TRUE);
      }
    }
  }

  if (!DidGameJustStart()) {
    gubEnemyEncounterCode = NO_ENCOUNTER_CODE;
  }

  HandleOtherGroupsArrivingSimultaneously(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ);

  curr = gpGroupList;
  while (curr) {
    if (curr->fPlayer && curr->ubGroupSize) {
      if (!curr->fBetweenSectors) {
        if (curr->ubSectorX == pGroup->ubSectorX && curr->ubSectorY == pGroup->ubSectorY &&
            !curr->ubSectorZ) {
          if (!GroupHasInTransitDeadOrPOWMercs(curr) &&
              (!IsGroupTheHelicopterGroup(curr) || !fHelicopterIsAirBorne) &&
              (!curr->fVehicle || NumberMercsInVehicleGroup(curr))) {
            // Now, a player group is in this sector.  Determine if the group contains any mercs
            // that can fight. Vehicles, EPCs and the robot doesn't count.  Mercs below OKLIFE do.
            pPlayer = curr->pPlayerList;
            while (pPlayer) {
              pSoldier = pPlayer->pSoldier;
              if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
                if (!AM_A_ROBOT(pSoldier) && !AM_AN_EPC(pSoldier) && pSoldier->bLife >= OKLIFE) {
                  fCombatAbleMerc = TRUE;
                }
                if (IsSolAlive(pSoldier)) {
                  fAliveMerc = TRUE;
                }
              }
              pPlayer = pPlayer->next;
            }
            if (!pPlayerDialogGroup && fCombatAbleMerc) {
              pPlayerDialogGroup = curr;
            }
            if (fCombatAbleMerc) {
              break;
            }
          }
        }
      }
    }
    curr = curr->next;
  }

  if (pGroup->fPlayer) {
    pPlayerDialogGroup = pGroup;

    if (NumEnemiesInSector(pGroup->ubSectorX, pGroup->ubSectorY)) {
      fBattlePending = TRUE;
    }

    if (pGroup->uiFlags & GROUPFLAG_HIGH_POTENTIAL_FOR_AMBUSH &&
        fBattlePending) {  // This group has just arrived in a new sector from an adjacent sector
                           // that he retreated from
      // If this battle is an encounter type battle, then there is a 90% chance that the battle will
      // become an ambush scenario.
      gfHighPotentialForAmbush = TRUE;
    }

    // If there are bloodcats in this sector, then it internally checks and handles it
    if (TestForBloodcatAmbush(pGroup)) {
      fBloodCatAmbush = TRUE;
      fBattlePending = TRUE;
    }

    if (fBattlePending &&
        (!fBloodCatAmbush || gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE)) {
      if (PossibleToCoordinateSimultaneousGroupArrivals(pGroup)) {
        return FALSE;
      }
    }
  } else {
    if (CountAllMilitiaInSector(pGroup->ubSectorX, pGroup->ubSectorY)) {
      fMilitiaPresent = TRUE;
      fBattlePending = TRUE;
    }
    if (fAliveMerc) {
      fBattlePending = TRUE;
    }
  }

  if (!fAliveMerc &&
      !fMilitiaPresent) {  // empty vehicle, everyone dead, don't care.  Enemies don't care.
    return FALSE;
  }

  if (fBattlePending) {  // A battle is pending, but the player's could be all unconcious or dead.
// Go through every group until we find at least one concious merc.  The looping will determine
// if there are any live mercs and/or concious ones.  If there are no concious mercs, but alive
// ones, then we will go straight to autoresolve, where the enemy will likely annihilate them or
// capture them. If there are no alive mercs, then there is nothing anybody can do.  The enemy will
// completely ignore this, and continue on.
#ifdef JA2BETAVERSION
    ValidateGroups(pGroup);
#endif

    if (gubNumGroupsArrivedSimultaneously) {  // Because this is a battle case, clear all the group
                                              // flags
      curr = gpGroupList;
      while (curr && gubNumGroupsArrivedSimultaneously) {
        if (curr->uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY) {
          curr->uiFlags &= ~GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
          gubNumGroupsArrivedSimultaneously--;
        }
        curr = curr->next;
      }
    }

    gpInitPrebattleGroup = pGroup;

    if (gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
        gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE) {
      NotifyPlayerOfBloodcatBattle(pGroup->ubSectorX, pGroup->ubSectorY);
      return TRUE;
    }

    if (!fCombatAbleMerc) {  // Prepare for instant autoresolve.
      gfDelayAutoResolveStart = TRUE;
      gfUsePersistantPBI = TRUE;
      if (fMilitiaPresent) {
        NotifyPlayerOfInvasionByEnemyForces(pGroup->ubSectorX, pGroup->ubSectorY, 0,
                                            TriggerPrebattleInterface);
      } else {
        wchar_t str[256];
        wchar_t pSectorStr[128];
        GetSectorIDString(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, pSectorStr,
                          ARR_SIZE(pSectorStr), TRUE);
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_DIALOG_ENEMIES_ATTACK_UNCONCIOUSMERCS],
                 pSectorStr);
        DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, TriggerPrebattleInterface);
      }
    }

#ifdef JA2BETAVERSION
    if (guiCurrentScreen == AIVIEWER_SCREEN) gfExitViewer = TRUE;
#endif

    if (pPlayerDialogGroup) {
      PrepareForPreBattleInterface(pPlayerDialogGroup, pGroup);
    }
    return TRUE;
  }
  return FALSE;
}

void TriggerPrebattleInterface(uint8_t ubResult) {
  StopTimeCompression();
  SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_TRIGGERPREBATTLEINTERFACE,
                                (uintptr_t)gpInitPrebattleGroup, 0, 0, 0, 0);
  gpInitPrebattleGroup = NULL;
}

void DeployGroupToSector(struct GROUP *pGroup) {
  Assert(pGroup);
  if (pGroup->fPlayer) {
    // Update the sector positions of the players...
    return;
  }
  // Assuming enemy code from here on...
}

// This will get called after a battle is auto-resolved or automatically after arriving
// at the next sector during a move and the area is clear.
void CalculateNextMoveIntention(struct GROUP *pGroup) {
  int32_t i;
  WAYPOINT *wp;

  Assert(pGroup);

  // TEMP:  Ignore resting...

  // Should be surely an enemy group that has just made a new decision to go elsewhere!
  if (pGroup->fBetweenSectors) {
    return;
  }

  if (!pGroup->pWaypoints) {
    return;
  }

  // If the waypoints have been cancelled, then stop moving.
  /*
  if( pGroup->fWaypointsCancelled )
  {
          DeployGroupToSector( pGroup );
          return;
  }
  */

  // Determine if we are at a waypoint.
  i = pGroup->ubNextWaypointID;
  wp = pGroup->pWaypoints;
  while (i--) {  // Traverse through the waypoint list to the next waypoint ID
    Assert(wp);
    wp = wp->next;
  }
  Assert(wp);

  // We have the next waypoint, now check if we are actually there.
  if (pGroup->ubSectorX == wp->x &&
      pGroup->ubSectorY == wp->y) {  // We have reached the next waypoint, so now determine what the
                                     // next waypoint is.
    switch (pGroup->ubMoveType) {
      case ONE_WAY:
        if (!wp->next) {  // No more waypoints, so we've reached the destination.
          DeployGroupToSector(pGroup);
          return;
        }
        // Advance destination to next waypoint ID
        pGroup->ubNextWaypointID++;
        break;
      case CIRCULAR:
        wp = wp->next;
        if (!wp) {  // reached the end of the patrol route.  Set to the first waypoint in list,
                    // indefinately.
          // NOTE:  If the last waypoint isn't exclusively aligned to the x or y axis of the first
          //			 waypoint, there will be an assertion failure inside the waypoint
          // movement code.
          pGroup->ubNextWaypointID = 0;
        } else
          pGroup->ubNextWaypointID++;
        break;
      case ENDTOEND_FORWARDS:
        wp = wp->next;
        if (!wp) {
          AssertMsg(pGroup->ubNextWaypointID,
                    "EndToEnd patrol group needs more than one waypoint!");
          pGroup->ubNextWaypointID--;
          pGroup->ubMoveType = ENDTOEND_BACKWARDS;
        } else
          pGroup->ubNextWaypointID++;
        break;
      case ENDTOEND_BACKWARDS:
        if (!pGroup->ubNextWaypointID) {
          pGroup->ubNextWaypointID++;
          pGroup->ubMoveType = ENDTOEND_FORWARDS;
        } else
          pGroup->ubNextWaypointID--;
        break;
    }
  }
  InitiateGroupMovementToNextSector(pGroup);
}

BOOLEAN AttemptToMergeSeparatedGroups(struct GROUP *pGroup, BOOLEAN fDecrementTraversals) {
  return FALSE;
}

void AwardExperienceForTravelling(struct GROUP *pGroup) {
  // based on how long movement took, mercs gain a bit of life experience for travelling
  PLAYERGROUP *pPlayerGroup;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiPoints;
  uint32_t uiCarriedPercent;

  if (!pGroup || !pGroup->fPlayer) {
    return;
  }

  pPlayerGroup = pGroup->pPlayerList;
  while (pPlayerGroup) {
    pSoldier = pPlayerGroup->pSoldier;
    if (pSoldier && !AM_A_ROBOT(pSoldier) && !AM_AN_EPC(pSoldier) &&
        !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      if (pSoldier->bLifeMax < 100) {
        // award exp...
        // amount was originally based on getting 100-bLifeMax points for 12 hours of travel (720)
        // but changed to flat rate since StatChange makes roll vs 100-lifemax as well!
        uiPoints = pGroup->uiTraverseTime / (450 / 100 - pSoldier->bLifeMax);
        if (uiPoints > 0) {
          StatChange(pSoldier, HEALTHAMT, (uint8_t)uiPoints, FALSE);
        }
      }

      if (pSoldier->bStrength < 100) {
        uiCarriedPercent = CalculateCarriedWeight(pSoldier);
        if (uiCarriedPercent > 50) {
          uiPoints = pGroup->uiTraverseTime / (450 / (100 - pSoldier->bStrength));
          StatChange(pSoldier, STRAMT, (uint16_t)(uiPoints * (uiCarriedPercent - 50) / 100), FALSE);
        }
      }
    }
    pPlayerGroup = pPlayerGroup->next;
  }
}

void AddCorpsesToBloodcatLair(uint8_t sSectorX, uint8_t sSectorY) {
  ROTTING_CORPSE_DEFINITION Corpse;
  int16_t sXPos, sYPos;

  memset(&Corpse, 0, sizeof(ROTTING_CORPSE_DEFINITION));

  // Setup some values!
  Corpse.ubBodyType = REGMALE;
  Corpse.sHeightAdjustment = 0;
  Corpse.bVisible = TRUE;

  SET_PALETTEREP_ID(Corpse.HeadPal, "BROWNHEAD");
  SET_PALETTEREP_ID(Corpse.VestPal, "YELLOWVEST");
  SET_PALETTEREP_ID(Corpse.SkinPal, "PINKSKIN");
  SET_PALETTEREP_ID(Corpse.PantsPal, "GREENPANTS");

  Corpse.bDirection = (int8_t)Random(8);

  // Set time of death
  // Make sure they will be rotting!
  Corpse.uiTimeOfDeath = GetWorldTotalMin() - (2 * NUM_SEC_IN_DAY / 60);
  // Set type
  Corpse.ubType = (uint8_t)SMERC_JFK;
  Corpse.usFlags = ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO;

  // 1st gridno
  Corpse.sGridNo = 14319;
  ConvertGridNoToXY(Corpse.sGridNo, &sXPos, &sYPos);
  Corpse.dXPos = (float)(CenterX(sXPos));
  Corpse.dYPos = (float)(CenterY(sYPos));

  // Add the rotting corpse info to the sectors unloaded rotting corpse file
  AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sSectorX, sSectorY, 0, &Corpse);

  // 2nd gridno
  Corpse.sGridNo = 9835;
  ConvertGridNoToXY(Corpse.sGridNo, &sXPos, &sYPos);
  Corpse.dXPos = (float)(CenterX(sXPos));
  Corpse.dYPos = (float)(CenterY(sYPos));

  // Add the rotting corpse info to the sectors unloaded rotting corpse file
  AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sSectorX, sSectorY, 0, &Corpse);

  // 3rd gridno
  Corpse.sGridNo = 11262;
  ConvertGridNoToXY(Corpse.sGridNo, &sXPos, &sYPos);
  Corpse.dXPos = (float)(CenterX(sXPos));
  Corpse.dYPos = (float)(CenterY(sYPos));

  // Add the rotting corpse info to the sectors unloaded rotting corpse file
  AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sSectorX, sSectorY, 0, &Corpse);
}

// ARRIVALCALLBACK
//...............
// This is called whenever any group arrives in the next sector (player or enemy)
// This function will first check to see if a battle should start, or if they
// aren't at the final destination, they will move to the next sector.
void GroupArrivedAtSector(uint8_t ubGroupID, BOOLEAN fCheckForBattle, BOOLEAN fNeverLeft) {
  struct GROUP *pGroup;
  int32_t iVehId = -1;
  PLAYERGROUP *curr;
  uint8_t ubInsertionDirection, ubStrategicInsertionCode;
  struct SOLDIERTYPE *pSoldier = NULL;
  BOOLEAN fExceptionQueue = FALSE;
  BOOLEAN fFirstTimeInSector = FALSE;
  BOOLEAN fGroupDestroyed = FALSE;

  // reset
  gfWaitingForInput = FALSE;

  // grab the group and see if valid
  pGroup = GetGroup(ubGroupID);

  if (pGroup == NULL) {
    return;
  }

  if (pGroup->fPlayer) {
    // Set the fact we have visited the  sector
    curr = pGroup->pPlayerList;
    if (curr) {
      if (curr->pSoldier->bAssignment < ON_DUTY) {
        ResetDeadSquadMemberList(curr->pSoldier->bAssignment);
      }
    }

    while (curr) {
      curr->pSoldier->uiStatusFlags &= ~SOLDIER_SHOULD_BE_TACTICALLY_VALID;
      curr = curr->next;
    }

    if (pGroup->fVehicle) {
      if ((iVehId = (GivenMvtGroupIdFindVehicleId(ubGroupID))) != -1) {
        if (iVehId != iHelicopterVehicleId) {
          if (pGroup->pPlayerList == NULL) {
            // nobody here, better just get out now
            // with vehicles, arriving empty is probably ok, since passengers might have been killed
            // but vehicle lived.
            return;
          }
        }
      }
    } else {
      if (pGroup->pPlayerList == NULL) {
        // nobody here, better just get out now
        AssertMsg(0, String("Player group %d arrived in sector empty.  KM 0", ubGroupID));
        return;
      }
    }
  }
  // Check for exception cases which
  if (gTacticalStatus.bBoxingState != NOT_BOXING) {
    if (!pGroup->fPlayer && pGroup->ubNextX == 5 && pGroup->ubNextY == 4 &&
        pGroup->ubSectorZ == 0) {
      fExceptionQueue = TRUE;
    }
  }
  // First check if the group arriving is going to queue another battle.
  // NOTE:  We can't have more than one battle ongoing at a time.
  if (fExceptionQueue ||
      (fCheckForBattle && gTacticalStatus.fEnemyInSector &&
       FindMovementGroupInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, TRUE) &&
       (pGroup->ubNextX != gWorldSectorX || pGroup->ubNextY != gWorldSectorY ||
        gbWorldSectorZ > 0)) ||
      AreInMeanwhile() ||
      // KM : Aug 11, 1999 -- Patch fix:  Added additional checks to prevent a 2nd battle in the
      // case
      //     where the player is involved in a potential battle with bloodcats/civilians
      (fCheckForBattle && HostileCiviliansPresent()) ||
      (fCheckForBattle && HostileBloodcatsPresent())) {
    // QUEUE BATTLE!
    // Delay arrival by a random value ranging from 3-5 minutes, so it doesn't get the player
    // too suspicious after it happens to him a few times, which, by the way, is a rare occurrence.
    if (AreInMeanwhile()) {
      pGroup->uiArrivalTime++;  // tack on only 1 minute if we are in a meanwhile scene.  This
                                // effectively prevents any battle from occurring while inside a
                                // meanwhile scene.
    } else {
      pGroup->uiArrivalTime += Random(3) + 3;
    }

    if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID))
      AssertMsg(0, "Failed to add movement event.");

    if (pGroup->fPlayer) {
      if (pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin()) {
        AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE,
                          pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID);
      }
    }

    return;
  }

  // Update the position of the group
  pGroup->ubPrevX = pGroup->ubSectorX;
  pGroup->ubPrevY = pGroup->ubSectorY;
  pGroup->ubSectorX = pGroup->ubNextX;
  pGroup->ubSectorY = pGroup->ubNextY;
  pGroup->ubNextX = 0;
  pGroup->ubNextY = 0;

  if (pGroup->fPlayer) {
    if (pGroup->ubSectorZ == 0) {
      SectorInfo[GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY)].bLastKnownEnemies =
          NumEnemiesInSector(pGroup->ubSectorX, pGroup->ubSectorY);
    }

    // award life 'experience' for travelling, based on travel time!
    if (!pGroup->fVehicle) {
      // gotta be walking to get tougher
      AwardExperienceForTravelling(pGroup);
    } else if (!IsGroupTheHelicopterGroup(pGroup)) {
      struct SOLDIERTYPE *pSoldier;
      int32_t iVehicleID;
      iVehicleID = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
      AssertMsg(iVehicleID != -1, "GroupArrival for vehicle group.  Invalid iVehicleID. ");

      pSoldier = GetSoldierStructureForVehicle(iVehicleID);
      AssertMsg(pSoldier, "GroupArrival for vehicle group.  Invalid soldier pointer.");

      SpendVehicleFuel(pSoldier, (int16_t)(pGroup->uiTraverseTime * 6));

      if (!VehicleFuelRemaining(pSoldier)) {
        ReportVehicleOutOfGas(iVehicleID, pGroup->ubSectorX, pGroup->ubSectorY);
        // Nuke the group's path, so they don't continue moving.
        ClearMercPathsAndWaypointsForAllInGroup(pGroup);
      }
    }
  }

  pGroup->uiTraverseTime = 0;
  SetGroupArrivalTime(pGroup, 0);
  pGroup->fBetweenSectors = FALSE;

  MarkForRedrawalStrategicMap();
  fMapScreenBottomDirty = TRUE;

  // if a player group
  if (pGroup->fPlayer) {
    // if this is the last sector along player group's movement path (no more waypoints)
    if (GroupAtFinalDestination(pGroup)) {
      // clear their strategic movement (mercpaths and waypoints)
      ClearMercPathsAndWaypointsForAllInGroup(pGroup);
    }

    // if on surface
    if (pGroup->ubSectorZ == 0) {
      // check for discovering secret locations
      TownID bTownId = GetTownIdForSector(pGroup->ubSectorX, pGroup->ubSectorY);

      if (bTownId == TIXA)
        SetTixaAsFound();
      else if (bTownId == ORTA)
        SetOrtaAsFound();
      else if (IsThisSectorASAMSector(pGroup->ubSectorX, pGroup->ubSectorY, 0))
        SetSAMSiteAsFound(GetSAMIdFromSector(pGroup->ubSectorX, pGroup->ubSectorY, 0));
    }

    if (pGroup->ubSectorX < pGroup->ubPrevX) {
      ubInsertionDirection = SOUTHWEST;
      ubStrategicInsertionCode = INSERTION_CODE_EAST;
    } else if (pGroup->ubSectorX > pGroup->ubPrevX) {
      ubInsertionDirection = NORTHEAST;
      ubStrategicInsertionCode = INSERTION_CODE_WEST;
    } else if (pGroup->ubSectorY < pGroup->ubPrevY) {
      ubInsertionDirection = NORTHWEST;
      ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
    } else if (pGroup->ubSectorY > pGroup->ubPrevY) {
      ubInsertionDirection = SOUTHEAST;
      ubStrategicInsertionCode = INSERTION_CODE_NORTH;
    } else {
      Assert(0);
      return;
    }

    if (pGroup->fVehicle == FALSE) {
      // non-vehicle player group

      curr = pGroup->pPlayerList;
      while (curr) {
        curr->pSoldier->fBetweenSectors = FALSE;
        curr->pSoldier->sSectorX = pGroup->ubSectorX;
        curr->pSoldier->sSectorY = pGroup->ubSectorY;
        curr->pSoldier->bSectorZ = pGroup->ubSectorZ;
        curr->pSoldier->ubPrevSectorID = (uint8_t)GetSectorID8(pGroup->ubPrevX, pGroup->ubPrevY);
        curr->pSoldier->ubInsertionDirection = ubInsertionDirection;

        // don't override if a tactical traversal
        if (curr->pSoldier->ubStrategicInsertionCode != INSERTION_CODE_PRIMARY_EDGEINDEX &&
            curr->pSoldier->ubStrategicInsertionCode != INSERTION_CODE_SECONDARY_EDGEINDEX) {
          curr->pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
        }

        if (curr->pSoldier->pMercPath) {
          // remove head from their mapscreen path list
          curr->pSoldier->pMercPath = RemoveHeadFromStrategicPath(curr->pSoldier->pMercPath);
        }

        // ATE: Alrighty, check if this sector is currently loaded, if so,
        // add them to the tactical engine!
        if (pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY &&
            pGroup->ubSectorZ == gbWorldSectorZ) {
          UpdateMercInSector(curr->pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                             gbWorldSectorZ);
        }
        curr = curr->next;
      }

      // if there's anybody in the group
      if (pGroup->pPlayerList) {
        // don't print any messages when arriving underground (there's no delay involved) or if we
        // never left (cancel)
        if (GroupAtFinalDestination(pGroup) && (pGroup->ubSectorZ == 0) && !fNeverLeft) {
          // if assigned to a squad
          if (pGroup->pPlayerList->pSoldier->bAssignment < ON_DUTY) {
            // squad
            ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pMessageStrings[MSG_ARRIVE],
                      pAssignmentStrings[pGroup->pPlayerList->pSoldier->bAssignment],
                      pMapVertIndex[pGroup->pPlayerList->pSoldier->sSectorY],
                      pMapHortIndex[GetSolSectorX(pGroup->pPlayerList->pSoldier)]);
          } else {
            // a loner
            ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pMessageStrings[MSG_ARRIVE],
                      pGroup->pPlayerList->pSoldier->name,
                      pMapVertIndex[pGroup->pPlayerList->pSoldier->sSectorY],
                      pMapHortIndex[GetSolSectorX(pGroup->pPlayerList->pSoldier)]);
          }
        }
      }
    } else  // vehicle player group
    {
      iVehId = GivenMvtGroupIdFindVehicleId(ubGroupID);
      Assert(iVehId != -1);

      if (pVehicleList[iVehId].pMercPath) {
        // remove head from vehicle's mapscreen path list
        pVehicleList[iVehId].pMercPath =
            RemoveHeadFromStrategicPath(pVehicleList[iVehId].pMercPath);
      }

      // update vehicle position
      SetVehicleSectorValues(iVehId, pGroup->ubSectorX, pGroup->ubSectorY);
      pVehicleList[iVehId].fBetweenSectors = FALSE;

      // update passengers position
      UpdatePositionOfMercsInVehicle(iVehId);

      if (iVehId != iHelicopterVehicleId) {
        pSoldier = GetSoldierStructureForVehicle(iVehId);
        Assert(pSoldier);

        pSoldier->fBetweenSectors = FALSE;
        pSoldier->sSectorX = pGroup->ubSectorX;
        pSoldier->sSectorY = pGroup->ubSectorY;
        pSoldier->bSectorZ = pGroup->ubSectorZ;
        pSoldier->ubInsertionDirection = ubInsertionDirection;

        // ATE: Removed, may 21 - sufficient to use insertion direction...
        // pSoldier->bDesiredDirection = ubInsertionDirection;

        pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;

        // if this sector is currently loaded
        if (pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY &&
            pGroup->ubSectorZ == gbWorldSectorZ) {
          // add vehicle to the tactical engine!
          UpdateMercInSector(pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                             gbWorldSectorZ);
        }

        // set directions of insertion
        curr = pGroup->pPlayerList;
        while (curr) {
          curr->pSoldier->fBetweenSectors = FALSE;
          curr->pSoldier->sSectorX = pGroup->ubSectorX;
          curr->pSoldier->sSectorY = pGroup->ubSectorY;
          curr->pSoldier->bSectorZ = pGroup->ubSectorZ;
          curr->pSoldier->ubInsertionDirection = ubInsertionDirection;

          // ATE: Removed, may 21 - sufficient to use insertion direction...
          // curr->pSoldier->bDesiredDirection = ubInsertionDirection;

          curr->pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;

          // if this sector is currently loaded
          if (pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY &&
              pGroup->ubSectorZ == gbWorldSectorZ) {
            // add passenger to the tactical engine!
            UpdateMercInSector(curr->pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                               gbWorldSectorZ);
          }

          curr = curr->next;
        }
      } else {
        if (HandleHeliEnteringSector((uint8_t)pVehicleList[iVehId].sSectorX,
                                     (uint8_t)pVehicleList[iVehId].sSectorY) == TRUE) {
          // helicopter destroyed
          fGroupDestroyed = TRUE;
        }
      }

      if (!fGroupDestroyed) {
        // don't print any messages when arriving underground, there's no delay involved
        if (GroupAtFinalDestination(pGroup) && (pGroup->ubSectorZ == 0) && !fNeverLeft) {
          ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pMessageStrings[MSG_ARRIVE],
                    pVehicleStrings[pVehicleList[iVehId].ubVehicleType],
                    pMapVertIndex[pGroup->ubSectorY], pMapHortIndex[pGroup->ubSectorX]);
        }
      }
    }

    if (!fGroupDestroyed) {
      // check if sector had been visited previously
      fFirstTimeInSector = !GetSectorFlagStatus(pGroup->ubSectorX, pGroup->ubSectorY,
                                                pGroup->ubSectorZ, SF_ALREADY_VISITED);

      // on foot, or in a vehicle other than the chopper
      if (!pGroup->fVehicle || !IsGroupTheHelicopterGroup(pGroup)) {
        // ATE: Add a few corpse to the bloodcat lair...
        if (GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY) == SEC_I16 && fFirstTimeInSector) {
          AddCorpsesToBloodcatLair(pGroup->ubSectorX, pGroup->ubSectorY);
        }

        // mark the sector as visited already
        SetSectorFlag(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, SF_ALREADY_VISITED);
      }
    }

    // update character info
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  }

  if (!fGroupDestroyed) {
    // Determine if a battle should start.
    // if a battle does start, or get's delayed, then we will keep the group in memory including
    // all waypoints, until after the battle is resolved.  At that point, we will continue the
    // processing.
    if (fCheckForBattle && !CheckConditionsForBattle(pGroup) && !gfWaitingForInput) {
      struct GROUP *next;
      HandleNonCombatGroupArrival(pGroup, TRUE, fNeverLeft);

      if (gubNumGroupsArrivedSimultaneously) {
        pGroup = gpGroupList;
        while (gubNumGroupsArrivedSimultaneously && pGroup) {
          next = pGroup->next;
          if (pGroup->uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY) {
            gubNumGroupsArrivedSimultaneously--;
            HandleNonCombatGroupArrival(pGroup, FALSE, FALSE);
          }
          pGroup = next;
        }
      }
    } else {  // Handle cases for pre battle conditions
      pGroup->uiFlags = 0;
      if (gubNumAwareBattles) {  // When the AI is looking for the players, and a battle is
                                 // initiated, then
        // decrement the value, otherwise the queen will continue searching to infinity.
        gubNumAwareBattles--;
      }
    }
  }
  gfWaitingForInput = FALSE;
}

void HandleNonCombatGroupArrival(struct GROUP *pGroup, BOOLEAN fMainGroup, BOOLEAN fNeverLeft) {
  // if any mercs are actually in the group

  if (StrategicAILookForAdjacentGroups(
          pGroup)) {  // The routine actually just deleted the enemy group (player's don't get
                      // deleted), so we are done!
    return;
  }

  if (pGroup->fPlayer) {
    // The group will always exist after the AI was processed.

    // Determine if the group should rest, change routes, or continue moving.
    // if on foot, or in a vehicle other than the helicopter
    if (!pGroup->fVehicle || !IsGroupTheHelicopterGroup(pGroup)) {
      // take control of sector
      SetThisSectorAsPlayerControlled(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ,
                                      FALSE);
    }

    // if this is the last sector along their movement path (no more waypoints)
    if (GroupAtFinalDestination(pGroup)) {
      // if currently selected sector has nobody in it
      if (PlayerMercsInSector((uint8_t)sSelMapX, (uint8_t)sSelMapY, (uint8_t)iCurrentMapSectorZ) ==
          0) {
        // make this sector strategically selected
        ChangeSelectedMapSector(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ);
      }

      // if on foot or in a vehicle other than the helicopter (Skyrider speaks for heli movement)
      if (!pGroup->fVehicle || !IsGroupTheHelicopterGroup(pGroup)) {
        StopTimeCompression();

        // if traversing tactically, or we never left (just canceling), don't do this
        if (!gfTacticalTraversal && !fNeverLeft) {
          RandomMercInGroupSaysQuote(pGroup, QUOTE_MERC_REACHED_DESTINATION);
        }
      }
    }
    // look for NPCs to stop for, anyone is too tired to keep going, if all OK rebuild waypoints &
    // continue movement NOTE: Only the main group (first group arriving) will stop for NPCs, it's
    // just too much hassle to stop them all
    PlayerGroupArrivedSafelyInSector(pGroup, fMainGroup);
  } else {
    if (!pGroup->fDebugGroup) {
      CalculateNextMoveIntention(pGroup);
    } else {
      RemovePGroup(pGroup);
    }
  }
  // Clear the non-persistant flags.
  pGroup->uiFlags = 0;
}

// Because a battle is about to start, we need to go through the event list and look for other
// groups that may arrive at the same time -- enemies or players, and blindly add them to the sector
// without checking for battle conditions, as it has already determined that a new battle is about
// to start.
void HandleOtherGroupsArrivingSimultaneously(uint8_t ubSectorX, uint8_t ubSectorY,
                                             uint8_t ubSectorZ) {
  STRATEGICEVENT *pEvent;
  uint32_t uiCurrTimeStamp;
  struct GROUP *pGroup;
  uiCurrTimeStamp = GetWorldTotalSeconds();
  pEvent = gpEventList;
  gubNumGroupsArrivedSimultaneously = 0;
  while (pEvent && pEvent->uiTimeStamp <= uiCurrTimeStamp) {
    if (pEvent->ubCallbackID == EVENT_GROUP_ARRIVAL && !(pEvent->ubFlags & SEF_DELETION_PENDING)) {
      pGroup = GetGroup((uint8_t)pEvent->uiParam);
      Assert(pGroup);
      if (pGroup->ubNextX == ubSectorX && pGroup->ubNextY == ubSectorY &&
          pGroup->ubSectorZ == ubSectorZ) {
        if (pGroup->fBetweenSectors) {
          GroupArrivedAtSector((uint8_t)pEvent->uiParam, FALSE, FALSE);
          pGroup->uiFlags |= GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
          gubNumGroupsArrivedSimultaneously++;
          DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);
          pEvent = gpEventList;
          continue;
        }
      }
    }
    pEvent = pEvent->next;
  }
}

// The user has just approved to plan a simultaneous arrival.  So we will syncronize all of the
// involved groups so that they arrive at the same time (which is the time the final group would
// arrive).
void PrepareGroupsForSimultaneousArrival() {
  struct GROUP *pGroup;
  uint32_t uiLatestArrivalTime = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iVehId = 0;

  pGroup = gpGroupList;
  while (pGroup) {  // For all of the groups that haven't arrived yet, determine which one is going
                    // to take the longest.
    if (pGroup != gpPendingSimultaneousGroup && pGroup->fPlayer && pGroup->fBetweenSectors &&
        pGroup->ubNextX == gpPendingSimultaneousGroup->ubSectorX &&
        pGroup->ubNextY == gpPendingSimultaneousGroup->ubSectorY &&
        !IsGroupTheHelicopterGroup(pGroup)) {
      uiLatestArrivalTime = max(pGroup->uiArrivalTime, uiLatestArrivalTime);
      pGroup->uiFlags |= GROUPFLAG_SIMULTANEOUSARRIVAL_APPROVED | GROUPFLAG_MARKER;
    }
    pGroup = pGroup->next;
  }
  // Now, go through the list again, and reset their arrival event to the latest arrival time.
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup->uiFlags & GROUPFLAG_MARKER) {
      DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

      // NOTE: This can cause the arrival time to be > GetWorldTotalMin() + TraverseTime, so keep
      // that in mind if you have any code that uses these 3 values to figure out how far along its
      // route a group is!
      SetGroupArrivalTime(pGroup, uiLatestArrivalTime);
      AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID);

      if (pGroup->fPlayer) {
        if (pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin()) {
          AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE,
                            pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID);
        }
      }

      DelayEnemyGroupsIfPathsCross(pGroup);

      pGroup->uiFlags &= ~GROUPFLAG_MARKER;
    }
    pGroup = pGroup->next;
  }
  // We still have the first group that has arrived.  Because they are set up to be in the
  // destination sector, we will "warp" them back to the last sector, and also setup a new arrival
  // time for them.
  pGroup = gpPendingSimultaneousGroup;
  pGroup->ubNextX = pGroup->ubSectorX;
  pGroup->ubNextY = pGroup->ubSectorY;
  pGroup->ubSectorX = pGroup->ubPrevX;
  pGroup->ubSectorY = pGroup->ubPrevY;
  SetGroupArrivalTime(pGroup, uiLatestArrivalTime);
  pGroup->fBetweenSectors = TRUE;

  if (pGroup->fVehicle) {
    if ((iVehId = (GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID))) != -1) {
      pVehicleList[iVehId].fBetweenSectors = TRUE;

      // set up vehicle soldier
      pSoldier = GetSoldierStructureForVehicle(iVehId);

      if (pSoldier) {
        pSoldier->fBetweenSectors = TRUE;
      }
    }
  }

  AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID);

  if (pGroup->fPlayer) {
    if (pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin()) {
      AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY,
                        pGroup->ubGroupID);
    }
  }
  DelayEnemyGroupsIfPathsCross(pGroup);
}

// See if there are other groups OTW.  If so, and if we haven't asked the user yet to plan
// a simultaneous attack, do so now, and readjust the groups accordingly.  If it is possible
// to do so, then we will set up the gui, and postpone the prebattle interface.
BOOLEAN PossibleToCoordinateSimultaneousGroupArrivals(struct GROUP *pFirstGroup) {
  struct GROUP *pGroup;
  uint8_t ubNumNearbyGroups = 0;

  // If the user has already been asked, then don't ask the question again!
  if (pFirstGroup->uiFlags &
          (GROUPFLAG_SIMULTANEOUSARRIVAL_APPROVED | GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED) ||
      IsGroupTheHelicopterGroup(pFirstGroup)) {
    return FALSE;
  }

  // We can't coordinate simultaneous attacks on a sector without any stationary forces!  Otherwise,
  // it is possible that they will be gone when you finally arrive. if(
  // !NumStationaryEnemiesInSector( pFirstGroup->ubSectorX, pFirstGroup->ubSectorY ) ) 	return
  // FALSE;

  // Count the number of groups that are scheduled to arrive in the same sector and are currently
  // adjacent to the sector in question.
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup != pFirstGroup && pGroup->fPlayer && pGroup->fBetweenSectors &&
        pGroup->ubNextX == pFirstGroup->ubSectorX && pGroup->ubNextY == pFirstGroup->ubSectorY &&
        !(pGroup->uiFlags & GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED) &&
        !IsGroupTheHelicopterGroup(pGroup)) {
      pGroup->uiFlags |= GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED;
      ubNumNearbyGroups++;
    }
    pGroup = pGroup->next;
  }

  if (ubNumNearbyGroups) {  // postpone the battle until the user answers the dialog.
    wchar_t str[255];
    wchar_t *pStr, *pEnemyType;
    InterruptTime();
    PauseGame();
    LockPauseState(13);
    gpPendingSimultaneousGroup = pFirstGroup;
    // Build the string
    if (ubNumNearbyGroups == 1) {
      pStr = gpStrategicString[STR_DETECTED_SINGULAR];
    } else {
      pStr = gpStrategicString[STR_DETECTED_PLURAL];
    }
    if (gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE) {
      pEnemyType = gpStrategicString[STR_PB_BLOODCATS];
    } else {
      pEnemyType = gpStrategicString[STR_PB_ENEMIES];
    }
    // header, sector, singular/plural str, confirmation string.
    // Ex:  Enemies have been detected in sector J9 and another squad is
    //     about to arrive.  Do you wish to coordinate a simultaneous arrival?
    swprintf(str, ARR_SIZE(str), pStr,
             pEnemyType,  // Enemy type (Enemies or bloodcats)
             'A' + gpPendingSimultaneousGroup->ubSectorY - 1,
             gpPendingSimultaneousGroup->ubSectorX);  // Sector location
    wcscat(str, L"  ");
    wcscat(str, gpStrategicString[STR_COORDINATE]);
    // Setup the dialog

    // Kris August 03, 1999 Bug fix:  Changed 1st line to 2nd line to fix game breaking if this
    // dialog came up while in tactical.
    //                               It would kick you to mapscreen, where things would break...
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, str, guiCurrentScreen, MSG_BOX_FLAG_YESNO,
                    PlanSimultaneousGroupArrivalCallback);

    gfWaitingForInput = TRUE;
    return TRUE;
  }
  return FALSE;
}

void PlanSimultaneousGroupArrivalCallback(uint8_t bMessageValue) {
  if (bMessageValue == MSG_BOX_RETURN_YES) {
    PrepareGroupsForSimultaneousArrival();
  } else {
    PrepareForPreBattleInterface(gpPendingSimultaneousGroup, gpPendingSimultaneousGroup);
  }
  UnLockPauseState();
  UnPauseGame();
}

void DelayEnemyGroupsIfPathsCross(struct GROUP *pPlayerGroup) {
  struct GROUP *pGroup;
  pGroup = gpGroupList;
  while (pGroup) {
    if (!pGroup->fPlayer) {  // then check to see if this group will arrive in next sector before
                             // the player group.
      if (pGroup->uiArrivalTime <
          pPlayerGroup
              ->uiArrivalTime) {  // check to see if enemy group will cross paths with player group.
        if (pGroup->ubNextX == pPlayerGroup->ubSectorX &&
            pGroup->ubNextY == pPlayerGroup->ubSectorY &&
            pGroup->ubSectorX == pPlayerGroup->ubNextX &&
            pGroup->ubSectorY ==
                pPlayerGroup->ubNextY) {  // Okay, the enemy group will cross paths with the player,
                                          // so find and delete the arrival event
          // and repost it in the future (like a minute or so after the player arrives)
          DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

          // NOTE: This can cause the arrival time to be > GetWorldTotalMin() + TraverseTime, so
          // keep that in mind if you have any code that uses these 3 values to figure out how far
          // along its route a group is!
          SetGroupArrivalTime(pGroup, pPlayerGroup->uiArrivalTime + 1 + Random(10));
          if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID))
            AssertMsg(0, "Failed to add movement event.");
        }
      }
    }
    pGroup = pGroup->next;
  }
}

void InitiateGroupMovementToNextSector(struct GROUP *pGroup) {
  int32_t dx, dy;
  int32_t i;
  uint8_t ubDirection;
  uint8_t ubSector;
  WAYPOINT *wp;
  int32_t iVehId = -1;
  struct SOLDIERTYPE *pSoldier = NULL;
  uint32_t uiSleepMinutes = 0;

  Assert(pGroup);
  i = pGroup->ubNextWaypointID;
  wp = pGroup->pWaypoints;
  while (i--) {  // Traverse through the waypoint list to the next waypoint ID
    Assert(wp);
    wp = wp->next;
  }
  Assert(wp);
  // We now have the correct waypoint.
  // Analyse the group and determine which direction it will move from the current sector.
  dx = wp->x - pGroup->ubSectorX;
  dy = wp->y - pGroup->ubSectorY;
  if (dx && dy) {  // Can't move diagonally!
    AssertMsg(0, String("Attempting to move to waypoint in a diagonal direction from sector %d,%d "
                        "to sector %d,%d",
                        pGroup->ubSectorX, pGroup->ubSectorY, wp->x, wp->y));
  }
  if (!dx && !dy)  // Can't move to position currently at!
    AssertMsg(
        0, String("Attempting to move to waypoint %d, %d that you are already at!", wp->x, wp->y));
  // Clip dx/dy value so that the move is for only one sector.
  if (dx >= 1) {
    ubDirection = EAST_STRATEGIC_MOVE;
    dx = 1;
  } else if (dy >= 1) {
    ubDirection = SOUTH_STRATEGIC_MOVE;
    dy = 1;
  } else if (dx <= -1) {
    ubDirection = WEST_STRATEGIC_MOVE;
    dx = -1;
  } else if (dy <= -1) {
    ubDirection = NORTH_STRATEGIC_MOVE;
    dy = -1;
  } else {
    Assert(0);
    return;
  }
  // All conditions for moving to the next waypoint are now good.
  pGroup->ubNextX = (uint8_t)(dx + pGroup->ubSectorX);
  pGroup->ubNextY = (uint8_t)(dy + pGroup->ubSectorY);
  // Calc time to get to next waypoint...
  ubSector = (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY);
  if (!pGroup->ubSectorZ) {
    BOOLEAN fCalcRegularTime = TRUE;
    if (!pGroup->fPlayer) {  // Determine if the enemy group is "sleeping".  If so, then simply
                             // delay their arrival time by the amount of time
      // they are going to be sleeping for.
      if (GetWorldHour() >= 21 || GetWorldHour() <= 4) {  // It is definitely night time.
        if (Chance(67)) {                                 // 2 in 3 chance of going to sleep.
          pGroup->uiTraverseTime = GetSectorMvtTimeForGroup(ubSector, ubDirection, pGroup);
          uiSleepMinutes = 360 + Random(121);  // 6-8 hours sleep
          fCalcRegularTime = FALSE;
        }
      }
    }
    if (fCalcRegularTime) {
      pGroup->uiTraverseTime = GetSectorMvtTimeForGroup(ubSector, ubDirection, pGroup);
    }
  } else {
    pGroup->uiTraverseTime = 1;
  }

  if (pGroup->uiTraverseTime == 0xffffffff) {
    AssertMsg(
        0, String("Group %d (%s) attempting illegal move from %c%d to %c%d (%s).",
                  pGroup->ubGroupID, (pGroup->fPlayer) ? "Player" : "AI", pGroup->ubSectorY + 'A',
                  pGroup->ubSectorX, pGroup->ubNextY + 'A', pGroup->ubNextX,
                  gszTerrain[GetSectorInfoByID8(ubSector)->ubTraversability[ubDirection]]));
  }

  // add sleep, if any
  pGroup->uiTraverseTime += uiSleepMinutes;

  if (gfTacticalTraversal && gpTacticalTraversalGroup == pGroup) {
    if (gfUndergroundTacticalTraversal) {  // underground movement between sectors takes 1 minute.
      pGroup->uiTraverseTime = 1;
    } else {  // strategic movement between town sectors takes 5 minutes.
      pGroup->uiTraverseTime = 5;
    }
  }

  // if group isn't already between sectors
  if (!pGroup->fBetweenSectors) {
    // put group between sectors
    pGroup->fBetweenSectors = TRUE;
    // and set it's arrival time
    SetGroupArrivalTime(pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime);
  }
  // NOTE: if the group is already between sectors, DON'T MESS WITH ITS ARRIVAL TIME!  THAT'S NOT
  // OUR JOB HERE!!!

  // special override for AI patrol initialization only
  if (gfRandomizingPatrolGroup) {  // We're initializing the patrol group, so randomize the enemy
                                   // groups to have extremely quick and varying
    // arrival times so that their initial positions aren't easily determined.
    pGroup->uiTraverseTime = 1 + Random(pGroup->uiTraverseTime - 1);
    SetGroupArrivalTime(pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime);
  }

  if (pGroup->fVehicle == TRUE) {
    // vehicle, set fact it is between sectors too
    if ((iVehId = (GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID))) != -1) {
      pVehicleList[iVehId].fBetweenSectors = TRUE;
      pSoldier = GetSoldierStructureForVehicle(iVehId);

      if (pSoldier) {
        pSoldier->fBetweenSectors = TRUE;

        // OK, Remove the guy from tactical engine!
        RemoveSoldierFromTacticalSector(pSoldier, TRUE);
      }
    }
  }

  // Post the event!
  if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID))
    AssertMsg(0, "Failed to add movement event.");

  // For the case of player groups, we need to update the information of the soldiers.
  if (pGroup->fPlayer) {
    PLAYERGROUP *curr;

    if (pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin()) {
      AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY,
                        pGroup->ubGroupID);
    }

    curr = pGroup->pPlayerList;
    while (curr) {
      curr->pSoldier->fBetweenSectors = TRUE;

      // OK, Remove the guy from tactical engine!
      RemoveSoldierFromTacticalSector(curr->pSoldier, TRUE);

      curr = curr->next;
    }
    CheckAndHandleUnloadingOfCurrentWorld();

    // If an enemy group will be crossing paths with the player group, delay the enemy group's
    // arrival time so that the player will always encounter that group.
    if (!pGroup->ubSectorZ) {
      DelayEnemyGroupsIfPathsCross(pGroup);
    }
  }
}

void RemoveGroupWaypoints(uint8_t ubGroupID) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);
  Assert(pGroup);
  RemovePGroupWaypoints(pGroup);
}

void RemovePGroupWaypoints(struct GROUP *pGroup) {
  WAYPOINT *wp;
  // if there aren't any waypoints to delete, then return.  This also avoids setting
  // the fWaypointsCancelled flag.
  if (!pGroup->pWaypoints) return;
  // remove all of the waypoints.
  while (pGroup->pWaypoints) {
    wp = pGroup->pWaypoints;
    pGroup->pWaypoints = pGroup->pWaypoints->next;
    MemFree(wp);
  }
  pGroup->ubNextWaypointID = 0;
  pGroup->pWaypoints = NULL;

  // By setting this flag, it acknowledges the possibility that the group is currently between
  // sectors, and will continue moving until it reaches the next sector.  If the user decides to
  // change directions, during this process, the arrival event must be modified to send the group
  // back. pGroup->fWaypointsCancelled = TRUE;
}

// set groups waypoints as cancelled
void SetWayPointsAsCanceled(uint8_t ubGroupID) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);
  Assert(pGroup);

  // pGroup -> fWaypointsCancelled = TRUE;

  return;
}

// set this groups previous sector values
void SetGroupPrevSectors(uint8_t ubGroupID, uint8_t ubX, uint8_t ubY) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);
  Assert(pGroup);

  // since we have a group, set prev sector's x and y
  pGroup->ubPrevX = ubX;
  pGroup->ubPrevY = ubY;
}

void RemoveGroup(uint8_t ubGroupID) {
  struct GROUP *pGroup;
  pGroup = GetGroup(ubGroupID);

  if (ubGroupID == 51) {
  }

  Assert(pGroup);
  RemovePGroup(pGroup);
}

BOOLEAN gfRemovingAllGroups = FALSE;

void RemovePGroup(struct GROUP *pGroup) {
  uint32_t bit, index, mask;

  if (pGroup->fPersistant && !gfRemovingAllGroups) {
    CancelEmptyPersistentGroupMovement(pGroup);
    return;
    DoScreenIndependantMessageBox(
        L"Strategic Info Warning:  Attempting to delete a persistant group.", MSG_BOX_FLAG_OK,
        NULL);
  }
  // if removing head, then advance head first.
  if (pGroup == gpGroupList)
    gpGroupList = gpGroupList->next;
  else {  // detach this node from the list.
    struct GROUP *curr;
    curr = gpGroupList;
    while (curr->next && curr->next != pGroup) curr = curr->next;
    AssertMsg(curr->next == pGroup, "Trying to remove a strategic group that isn't in the list!");
    curr->next = pGroup->next;
  }

  // Remove the waypoints.
  RemovePGroupWaypoints(pGroup);

  // Remove the arrival event if applicable.
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // Determine what type of group we have (because it requires different methods)
  if (pGroup->fPlayer) {  // Remove player group
    PLAYERGROUP *pPlayer;
    while (pGroup->pPlayerList) {
      pPlayer = pGroup->pPlayerList;
      pGroup->pPlayerList = pGroup->pPlayerList->next;
      MemFree(pPlayer);
    }
  } else {
    RemoveGroupFromStrategicAILists(pGroup->ubGroupID);
    MemFree(pGroup->pEnemyGroup);
  }

  // clear the unique group ID
  index = pGroup->ubGroupID / 32;
  bit = pGroup->ubGroupID % 32;
  mask = 1 << bit;

  if (!(uniqueIDMask[index] & mask)) {
    mask = mask;
  }

  uniqueIDMask[index] -= mask;

  MemFree(pGroup);
  pGroup = NULL;
}

void RemoveAllGroups() {
  gfRemovingAllGroups = TRUE;
  while (gpGroupList) {
    RemovePGroup(gpGroupList);
  }
  gfRemovingAllGroups = FALSE;
}

void SetGroupSectorValue(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ, uint8_t ubGroupID) {
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer;

  // get the group
  pGroup = GetGroup(ubGroupID);

  // make sure it is valid
  Assert(pGroup);

  // Remove waypoints
  RemovePGroupWaypoints(pGroup);

  // set sector x and y to passed values
  pGroup->ubSectorX = pGroup->ubNextX = (uint8_t)sSectorX;
  pGroup->ubSectorY = pGroup->ubNextY = (uint8_t)sSectorY;
  pGroup->ubSectorZ = (uint8_t)sSectorZ;
  pGroup->fBetweenSectors = FALSE;

  // set next sectors same as current
  pGroup->ubOriginalSector = (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY);
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // set all of the mercs in the group so that they are in the new sector too.
  pPlayer = pGroup->pPlayerList;
  while (pPlayer) {
    pPlayer->pSoldier->sSectorX = sSectorX;
    pPlayer->pSoldier->sSectorY = sSectorY;
    pPlayer->pSoldier->bSectorZ = (uint8_t)sSectorZ;
    pPlayer->pSoldier->fBetweenSectors = FALSE;
    pPlayer->pSoldier->uiStatusFlags &= ~SOLDIER_SHOULD_BE_TACTICALLY_VALID;
    pPlayer = pPlayer->next;
  }

  CheckAndHandleUnloadingOfCurrentWorld();
}

void SetEnemyGroupSector(struct GROUP *pGroup, uint8_t ubSectorID) {
  // make sure it is valid
  Assert(pGroup);
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // Remove waypoints
  if (!gfRandomizingPatrolGroup) {
    RemovePGroupWaypoints(pGroup);
  }

  // set sector x and y to passed values
  pGroup->ubSectorX = pGroup->ubNextX = SectorID8_X(ubSectorID);
  pGroup->ubSectorY = pGroup->ubNextY = SectorID8_Y(ubSectorID);
  pGroup->ubSectorZ = 0;
  pGroup->fBetweenSectors = FALSE;
  // pGroup->fWaypointsCancelled = FALSE;
}

void SetGroupNextSectorValue(uint8_t sSectorX, uint8_t sSectorY, uint8_t ubGroupID) {
  struct GROUP *pGroup;

  // get the group
  pGroup = GetGroup(ubGroupID);

  // make sure it is valid
  Assert(pGroup);

  // Remove waypoints
  RemovePGroupWaypoints(pGroup);

  // set sector x and y to passed values
  pGroup->ubNextX = (uint8_t)sSectorX;
  pGroup->ubNextY = (uint8_t)sSectorY;
  pGroup->fBetweenSectors = FALSE;

  // set next sectors same as current
  pGroup->ubOriginalSector = (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY);
}

// get eta of the group with this id
int32_t CalculateTravelTimeOfGroupId(uint8_t ubId) {
  struct GROUP *pGroup;

  // get the group
  pGroup = GetGroup(ubId);

  if (pGroup == NULL) {
    return (0);
  }

  return (CalculateTravelTimeOfGroup(pGroup));
}

int32_t CalculateTravelTimeOfGroup(struct GROUP *pGroup) {
  int32_t iDelta;
  uint32_t uiEtaTime = 0;
  WAYPOINT *pNode = NULL;
  WAYPOINT pCurrent, pDest;

  // check if valid group
  if (pGroup == NULL) {
    // return current time
    return (uiEtaTime);
  }

  // set up next node
  pNode = pGroup->pWaypoints;

  // now get the delta in current sector and next sector
  iDelta = (int32_t)(GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY) -
                     GetSectorID8(pGroup->ubNextX, pGroup->ubNextY));

  if (iDelta == 0) {
    // not going anywhere...return current time
    return (uiEtaTime);
  }

  // if already on the road
  if (pGroup->fBetweenSectors) {
    // to get travel time to the first sector, use the arrival time, this way it accounts for delays
    // due to simul. arrival
    if (pGroup->uiArrivalTime >= GetWorldTotalMin()) {
      uiEtaTime += (pGroup->uiArrivalTime - GetWorldTotalMin());
    }

    // first waypoint is NEXT sector
    pCurrent.x = pGroup->ubNextX;
    pCurrent.y = pGroup->ubNextY;
  } else {
    // first waypoint is CURRENT sector
    pCurrent.x = pGroup->ubSectorX;
    pCurrent.y = pGroup->ubSectorY;
  }

  while (pNode) {
    pDest.x = pNode->x;
    pDest.y = pNode->y;

    // update eta time by the path between these 2 waypts
    uiEtaTime += FindTravelTimeBetweenWaypoints(&pCurrent, &pDest, pGroup);

    pCurrent.x = pNode->x;
    pCurrent.y = pNode->y;

    // next waypt
    pNode = pNode->next;
  }

  return (uiEtaTime);
}

int32_t FindTravelTimeBetweenWaypoints(WAYPOINT *pSource, WAYPOINT *pDest, struct GROUP *pGroup) {
  uint8_t ubStart = 0, ubEnd = 0;
  int32_t iDelta = 0;
  int32_t iCurrentCostInTime = 0;
  uint8_t ubCurrentSector = 0;
  uint8_t ubDirection;
  int32_t iThisCostInTime;

  // find travel time between waypoints
  if (!pSource || !pDest) {
    // no change
    return (iCurrentCostInTime);
  }

  // get start and end setor values
  ubStart = GetSectorID8(pSource->x, pSource->y);
  ubEnd = GetSectorID8(pDest->x, pDest->y);

  // are we in fact moving?
  if (ubStart == ubEnd) {
    // no
    return (iCurrentCostInTime);
  }

  iDelta = (int32_t)(ubEnd - ubStart);

  // which direction are we moving?
  if (iDelta > 0) {
    if (iDelta % (SOUTH_MOVE - 2) == 0) {
      iDelta = (SOUTH_MOVE - 2);
      ubDirection = SOUTH_STRATEGIC_MOVE;
    } else {
      iDelta = EAST_MOVE;
      ubDirection = EAST_STRATEGIC_MOVE;
    }
  } else {
    if (iDelta % (NORTH_MOVE + 2) == 0) {
      iDelta = (NORTH_MOVE + 2);
      ubDirection = NORTH_STRATEGIC_MOVE;
    } else {
      iDelta = WEST_MOVE;
      ubDirection = WEST_STRATEGIC_MOVE;
    }
  }

  for (ubCurrentSector = ubStart; ubCurrentSector != ubEnd; ubCurrentSector += (int8_t)iDelta) {
    // find diff between current and next
    iThisCostInTime = GetSectorMvtTimeForGroup(ubCurrentSector, ubDirection, pGroup);

    if (iThisCostInTime == 0xffffffff) {
      AssertMsg(0, String("Group %d (%s) attempting illegal move from sector %d, dir %d (%s).",
                          pGroup->ubGroupID, (pGroup->fPlayer) ? "Player" : "AI", ubCurrentSector,
                          ubDirection,
                          gszTerrain[SectorInfo[ubCurrentSector].ubTraversability[ubDirection]]));
    }

    // accumulate it
    iCurrentCostInTime += iThisCostInTime;
  }

  return (iCurrentCostInTime);
}

#define FOOT_TRAVEL_TIME 89
#define CAR_TRAVEL_TIME 30
#define TRUCK_TRAVEL_TIME 32
#define TRACKED_TRAVEL_TIME 46
#define AIR_TRAVEL_TIME 10

// CHANGES:  ubDirection contains the strategic move value, not the delta value.
int32_t GetSectorMvtTimeForGroup(uint8_t ubSector, uint8_t ubDirection, struct GROUP *pGroup) {
  int32_t iTraverseTime;
  int32_t iBestTraverseTime = 1000000;
  int32_t iEncumbrance, iHighestEncumbrance = 0;
  struct SOLDIERTYPE *pSoldier;
  PLAYERGROUP *curr;
  BOOLEAN fFoot, fCar, fTruck, fTracked, fAir;
  uint8_t ubTraverseType;
  uint8_t ubTraverseMod;

  // THIS FUNCTION WAS WRITTEN TO HANDLE MOVEMENT TYPES WHERE MORE THAN ONE TRANSPORTAION TYPE IS
  // AVAILABLE.

  // Determine the group's method(s) of tranportation.  If more than one,
  // we will always use the highest time.
  fFoot = (uint8_t)(pGroup->ubTransportationMask & FOOT);
  fCar = (uint8_t)(pGroup->ubTransportationMask & CAR);
  fTruck = (uint8_t)(pGroup->ubTransportationMask & TRUCK);
  fTracked = (uint8_t)(pGroup->ubTransportationMask & TRACKED);
  fAir = (uint8_t)(pGroup->ubTransportationMask & AIR);

  ubTraverseType = GetSectorInfoByID8(ubSector)->ubTraversability[ubDirection];

  if (ubTraverseType == EDGEOFWORLD) return 0xffffffff;  // can't travel here!

  // ARM: Made air-only travel take its normal time per sector even through towns.  Because Skyrider
  // charges by the sector, not by flying time, it's annoying when his default route detours through
  // a town to save time, but costs extra money. This isn't exactly unrealistic, since the chopper
  // shouldn't be faster flying over a town anyway...  Not that other kinds of travel should be
  // either - but the towns represents a kind of warping of our space-time scale as it is...
  if ((ubTraverseType == TOWN) && (pGroup->ubTransportationMask != AIR))
    return 5;  // very fast, and vehicle types don't matter.

  if (fFoot) {
    switch (ubTraverseType) {
      case ROAD:
        ubTraverseMod = 100;
        break;
      case PLAINS:
        ubTraverseMod = 85;
        break;
      case SAND:
        ubTraverseMod = 50;
        break;
      case SPARSE:
        ubTraverseMod = 70;
        break;
      case DENSE:
        ubTraverseMod = 60;
        break;
      case SWAMP:
        ubTraverseMod = 35;
        break;
      case WATER:
        ubTraverseMod = 25;
        break;
      case HILLS:
        ubTraverseMod = 50;
        break;
      case GROUNDBARRIER:
        ubTraverseMod = 0;
        break;
      case NS_RIVER:
        ubTraverseMod = 25;
        break;
      case EW_RIVER:
        ubTraverseMod = 25;
        break;
      default:
        Assert(0);
        return 0xffffffff;
    }
    if (ubTraverseMod == 0) return 0xffffffff;  // Group can't traverse here.
    iTraverseTime = FOOT_TRAVEL_TIME * 100 / ubTraverseMod;
    if (iTraverseTime < iBestTraverseTime) iBestTraverseTime = iTraverseTime;

    if (pGroup->fPlayer) {
      curr = pGroup->pPlayerList;
      while (curr) {
        pSoldier = curr->pSoldier;
        if (pSoldier->bAssignment != VEHICLE) {  // Soldier is on foot and travelling.  Factor
                                                 // encumbrance into movement rate.
          iEncumbrance = CalculateCarriedWeight(pSoldier);
          if (iEncumbrance > iHighestEncumbrance) {
            iHighestEncumbrance = iEncumbrance;
          }
        }
        curr = curr->next;
      }
      if (iHighestEncumbrance > 100) {
        iBestTraverseTime = iBestTraverseTime * iHighestEncumbrance / 100;
      }
    }
  }
  if (fCar) {
    switch (ubTraverseType) {
      case ROAD:
        ubTraverseMod = 100;
        break;
      default:
        ubTraverseMod = 0;
        break;
    }
    if (ubTraverseMod == 0) return 0xffffffff;  // Group can't traverse here.
    iTraverseTime = CAR_TRAVEL_TIME * 100 / ubTraverseMod;
    if (iTraverseTime < iBestTraverseTime) iBestTraverseTime = iTraverseTime;
  }
  if (fTruck) {
    switch (ubTraverseType) {
      case ROAD:
        ubTraverseMod = 100;
        break;
      case PLAINS:
        ubTraverseMod = 75;
        break;
      case SPARSE:
        ubTraverseMod = 60;
        break;
      case HILLS:
        ubTraverseMod = 50;
        break;
      default:
        ubTraverseMod = 0;
        break;
    }
    if (ubTraverseMod == 0) return 0xffffffff;  // Group can't traverse here.
    iTraverseTime = TRUCK_TRAVEL_TIME * 100 / ubTraverseMod;
    if (iTraverseTime < iBestTraverseTime) iBestTraverseTime = iTraverseTime;
  }
  if (fTracked) {
    switch (ubTraverseType) {
      case ROAD:
        ubTraverseMod = 100;
        break;
      case PLAINS:
        ubTraverseMod = 100;
        break;
      case SAND:
        ubTraverseMod = 70;
        break;
      case SPARSE:
        ubTraverseMod = 60;
        break;
      case HILLS:
        ubTraverseMod = 60;
        break;
      case NS_RIVER:
        ubTraverseMod = 20;
        break;
      case EW_RIVER:
        ubTraverseMod = 20;
        break;
      case WATER:
        ubTraverseMod = 10;
        break;
      default:
        ubTraverseMod = 0;
        break;
    }
    if (ubTraverseMod == 0) return 0xffffffff;  // Group can't traverse here.
    iTraverseTime = TRACKED_TRAVEL_TIME * 100 / ubTraverseMod;
    if (iTraverseTime < iBestTraverseTime) iBestTraverseTime = iTraverseTime;
  }
  if (fAir) {
    iTraverseTime = AIR_TRAVEL_TIME;
    if (iTraverseTime < iBestTraverseTime) iBestTraverseTime = iTraverseTime;
  }
  return iBestTraverseTime;
}

// Counts the number of live mercs in any given sector.
uint8_t PlayerMercsInSector(uint8_t ubSectorX, uint8_t ubSectorY, uint8_t ubSectorZ) {
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer;
  uint8_t ubNumMercs = 0;
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup->fPlayer && !pGroup->fBetweenSectors) {
      if (pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY &&
          pGroup->ubSectorZ == ubSectorZ) {
        // we have a group, make sure that it isn't a group containing only dead members.
        pPlayer = pGroup->pPlayerList;
        while (pPlayer) {
          // robots count as mercs here, because they can fight, but vehicles don't
          if ((pPlayer->pSoldier->bLife) && !(pPlayer->pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
            ubNumMercs++;
          }
          pPlayer = pPlayer->next;
        }
      }
    }
    pGroup = pGroup->next;
  }
  return ubNumMercs;
}

uint8_t PlayerGroupsInSector(uint8_t ubSectorX, uint8_t ubSectorY, uint8_t ubSectorZ) {
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer;
  uint8_t ubNumGroups = 0;
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup->fPlayer && !pGroup->fBetweenSectors) {
      if (pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY &&
          pGroup->ubSectorZ == ubSectorZ) {
        // we have a group, make sure that it isn't a group containing only dead members.
        pPlayer = pGroup->pPlayerList;
        while (pPlayer) {
          if (pPlayer->pSoldier->bLife) {
            ubNumGroups++;
            break;
          }
          pPlayer = pPlayer->next;
        }
      }
    }
    pGroup = pGroup->next;
  }
  return ubNumGroups;
}

// is the player group with this id in motion?
BOOLEAN PlayerIDGroupInMotion(uint8_t ubID) {
  struct GROUP *pGroup;

  // get the group
  pGroup = GetGroup(ubID);

  // make sure it is valid

  // no group
  if (pGroup == NULL) {
    return (FALSE);
  }

  return (PlayerGroupInMotion(pGroup));
}

// is the player group in motion?
BOOLEAN PlayerGroupInMotion(struct GROUP *pGroup) { return (pGroup->fBetweenSectors); }

// get travel time for this group
int32_t GetTravelTimeForGroup(uint8_t ubSector, uint8_t ubDirection, uint8_t ubGroup) {
  struct GROUP *pGroup;

  // get the group
  pGroup = GetGroup(ubGroup);

  // make sure it is valid
  Assert(pGroup);

  return (GetSectorMvtTimeForGroup(ubSector, ubDirection, pGroup));
}

int32_t GetTravelTimeForFootTeam(uint8_t ubSector, uint8_t ubDirection) {
  struct GROUP Group;

  // group going on foot
  Group.ubTransportationMask = FOOT;

  return (GetSectorMvtTimeForGroup(ubSector, ubDirection, &(Group)));
}

// Add this group to the current battle fray!
// NOTE:  For enemies, only MAX_STRATEGIC_TEAM_SIZE at a time can be in a battle, so
// if it ever gets past that, god help the player, but we'll have to insert them
// as those slots free up.
void HandleArrivalOfReinforcements(struct GROUP *pGroup) {
  struct SOLDIERTYPE *pSoldier;
  SECTORINFO *pSector;
  int32_t iNumEnemiesInSector;
  int32_t cnt;

  if (pGroup->fPlayer) {  // We don't have to worry about filling up the player slots, because it is
                          // impossible
    // to have more player's in the game then the number of slots available for the player.
    PLAYERGROUP *pPlayer;
    uint8_t ubStrategicInsertionCode;
    // First, determine which entrypoint to use, based on the travel direction of the group.
    if (pGroup->ubSectorX < pGroup->ubPrevX)
      ubStrategicInsertionCode = INSERTION_CODE_EAST;
    else if (pGroup->ubSectorX > pGroup->ubPrevX)
      ubStrategicInsertionCode = INSERTION_CODE_WEST;
    else if (pGroup->ubSectorY < pGroup->ubPrevY)
      ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
    else if (pGroup->ubSectorY > pGroup->ubPrevY)
      ubStrategicInsertionCode = INSERTION_CODE_NORTH;
    else {
      Assert(0);
      return;
    }
    pPlayer = pGroup->pPlayerList;

    cnt = 0;

    while (pPlayer) {
      pSoldier = pPlayer->pSoldier;
      Assert(pSoldier);
      pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
      UpdateMercInSector(pSoldier, pGroup->ubSectorX, pGroup->ubSectorY, 0);
      pPlayer = pPlayer->next;

      // DO arrives quote....
      if (cnt == 0) {
        TacticalCharacterDialogue(pSoldier, QUOTE_MERC_REACHED_DESTINATION);
      }
      cnt++;
    }
    ScreenMsg(FONT_YELLOW, MSG_INTERFACE, Message[STR_PLAYER_REINFORCEMENTS]);

  } else {
    gfPendingEnemies = TRUE;
    ResetMortarsOnTeamCount();
    AddPossiblePendingEnemiesToBattle();
  }
  // Update the known number of enemies in the sector.
  pSector = &SectorInfo[GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY)];
  iNumEnemiesInSector = NumEnemiesInSector(pGroup->ubSectorX, pGroup->ubSectorY);
  if (iNumEnemiesInSector) {
    if (pSector->bLastKnownEnemies >= 0) {
      pSector->bLastKnownEnemies = (int8_t)iNumEnemiesInSector;
    }
    // if we don't know how many enemies there are, then we can't update this value.
  } else {
    pSector->bLastKnownEnemies = 0;
  }
}

BOOLEAN PlayersBetweenTheseSectors(int16_t sSource, int16_t sDest, int32_t *iCountEnter,
                                   int32_t *iCountExit, BOOLEAN *fAboutToArriveEnter) {
  struct GROUP *curr = gpGroupList;
  int16_t sBattleSector = -1;
  BOOLEAN fMayRetreatFromBattle = FALSE;
  BOOLEAN fRetreatingFromBattle = FALSE;
  BOOLEAN fHelicopterGroup = FALSE;
  uint8_t ubMercsInGroup = 0;

  *iCountEnter = 0;
  *iCountExit = 0;
  *fAboutToArriveEnter = FALSE;

  if (gpBattleGroup) {
    // Assert( gfPreBattleInterfaceActive );
    sBattleSector = (int16_t)GetSectorID8(gpBattleGroup->ubSectorX, gpBattleGroup->ubSectorY);
  }

  // debug only
  if (gfDisplayPotentialRetreatPaths == TRUE) {
    // Assert( gfPreBattleInterfaceActive );
  }

  // get number of characters entering/existing between these two sectors.  Special conditions
  // during pre-battle interface to return where this function is used to show potential retreating
  // directions instead!

  //	check all groups
  while (curr) {
    // if player group
    if (curr->fPlayer == TRUE) {
      fHelicopterGroup = IsGroupTheHelicopterGroup(curr);

      // if this group is aboard the helicopter and we're showing the airspace layer, don't count
      // any mercs aboard the chopper, because the chopper icon itself serves the function of
      // showing the location/size of this group
      if (!fHelicopterGroup || !fShowAircraftFlag) {
        // if only showing retreat paths, ignore groups not in the battle sector
        // if NOT showing retreat paths, ignore groups not between sectors
        if (((gfDisplayPotentialRetreatPaths == TRUE) && (sBattleSector == sSource)) ||
            ((gfDisplayPotentialRetreatPaths == FALSE) && (curr->fBetweenSectors == TRUE))) {
          fMayRetreatFromBattle = FALSE;
          fRetreatingFromBattle = FALSE;

          if ((sBattleSector == sSource) &&
              (GetSectorID8(curr->ubSectorX, curr->ubSectorY) == sSource) &&
              (GetSectorID8(curr->ubPrevX, curr->ubPrevY) == sDest)) {
            fMayRetreatFromBattle = TRUE;
          }

          if ((sBattleSector == sDest) &&
              (GetSectorID8(curr->ubSectorX, curr->ubSectorY) == sDest) &&
              (GetSectorID8(curr->ubPrevX, curr->ubPrevY) == sSource)) {
            fRetreatingFromBattle = TRUE;
          }

          ubMercsInGroup = curr->ubGroupSize;

          if (((GetSectorID8(curr->ubSectorX, curr->ubSectorY) == sSource) &&
               (GetSectorID8(curr->ubNextX, curr->ubNextY) == sDest)) ||
              (fMayRetreatFromBattle == TRUE)) {
            // if it's a valid vehicle, but not the helicopter (which can fly empty)
            if (curr->fVehicle && !fHelicopterGroup &&
                (GivenMvtGroupIdFindVehicleId(curr->ubGroupID) != -1)) {
              // make sure empty vehicles (besides helicopter) aren't in motion!
              Assert(ubMercsInGroup > 0);
              // subtract 1, we don't wanna count the vehicle itself for purposes of showing a
              // number on the map
              ubMercsInGroup--;
            }

            *iCountEnter += ubMercsInGroup;

            if ((curr->uiArrivalTime - GetWorldTotalMin() <= ABOUT_TO_ARRIVE_DELAY) ||
                (fMayRetreatFromBattle == TRUE)) {
              *fAboutToArriveEnter = TRUE;
            }
          } else if (((GetSectorID8(curr->ubSectorX, curr->ubSectorY) == sDest) &&
                      (GetSectorID8(curr->ubNextX, curr->ubNextY) == sSource)) ||
                     (fRetreatingFromBattle == TRUE)) {
            // if it's a valid vehicle, but not the helicopter (which can fly empty)
            if (curr->fVehicle && !fHelicopterGroup &&
                (GivenMvtGroupIdFindVehicleId(curr->ubGroupID) != -1)) {
              // make sure empty vehicles (besides helicopter) aren't in motion!
              Assert(ubMercsInGroup > 0);
              // subtract 1, we don't wanna count the vehicle itself for purposes of showing a
              // number on the map
              ubMercsInGroup--;
            }

            *iCountExit += ubMercsInGroup;
          }
        }
      }
    }

    // next group
    curr = curr->next;
  }

  // if there was actually anyone leaving this sector and entering next
  if (*iCountEnter > 0) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void MoveAllGroupsInCurrentSectorToSector(uint8_t ubSectorX, uint8_t ubSectorY, uint8_t ubSectorZ) {
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer;
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup->fPlayer && pGroup->ubSectorX == gWorldSectorX &&
        pGroup->ubSectorY == gWorldSectorY && pGroup->ubSectorZ == gbWorldSectorZ &&
        !pGroup->fBetweenSectors) {  // This player group is in the currently loaded sector...
      pGroup->ubSectorX = ubSectorX;
      pGroup->ubSectorY = ubSectorY;
      pGroup->ubSectorZ = ubSectorZ;
      pPlayer = pGroup->pPlayerList;
      while (pPlayer) {
        pPlayer->pSoldier->sSectorX = ubSectorX;
        pPlayer->pSoldier->sSectorY = ubSectorY;
        pPlayer->pSoldier->bSectorZ = ubSectorZ;
        pPlayer->pSoldier->fBetweenSectors = FALSE;
        pPlayer = pPlayer->next;
      }
    }
    pGroup = pGroup->next;
  }
  CheckAndHandleUnloadingOfCurrentWorld();
}

void GetGroupPosition(uint8_t *ubNextX, uint8_t *ubNextY, uint8_t *ubPrevX, uint8_t *ubPrevY,
                      uint32_t *uiTraverseTime, uint32_t *uiArriveTime, uint8_t ubGroupId) {
  struct GROUP *pGroup;

  // get the group
  pGroup = GetGroup(ubGroupId);

  // make sure it is valid

  // no group
  if (pGroup == NULL) {
    *ubNextX = 0;
    *ubNextY = 0;
    *ubPrevX = 0;
    *ubPrevY = 0;
    *uiTraverseTime = 0;
    *uiArriveTime = 0;
    return;
  }

  // valid group, grab values
  *ubNextX = pGroup->ubNextX;
  *ubNextY = pGroup->ubNextY;
  *ubPrevX = pGroup->ubPrevX;
  *ubPrevY = pGroup->ubPrevY;
  *uiTraverseTime = pGroup->uiTraverseTime;
  *uiArriveTime = pGroup->uiArrivalTime;

  return;
}

// this is only for grunts who were in mvt groups between sectors and are set to a new
// squad...NOTHING ELSE!!!!!
void SetGroupPosition(uint8_t ubNextX, uint8_t ubNextY, uint8_t ubPrevX, uint8_t ubPrevY,
                      uint32_t uiTraverseTime, uint32_t uiArriveTime, uint8_t ubGroupId) {
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer;

  // get the group
  pGroup = GetGroup(ubGroupId);

  // no group
  if (pGroup == NULL) {
    return;
  }

  // valid group, grab values
  pGroup->ubNextX = ubNextX;
  pGroup->ubNextY = ubNextY;
  pGroup->ubPrevX = ubPrevX;
  pGroup->ubPrevY = ubPrevY;
  pGroup->uiTraverseTime = uiTraverseTime;
  SetGroupArrivalTime(pGroup, uiArriveTime);
  pGroup->fBetweenSectors = TRUE;

  AddWaypointToPGroup(pGroup, pGroup->ubNextX, pGroup->ubNextY);
  // now, if player group set all grunts in the group to be between secotrs
  if (pGroup->fPlayer == TRUE) {
    pPlayer = pGroup->pPlayerList;
    while (pPlayer) {
      pPlayer->pSoldier->fBetweenSectors = TRUE;
      pPlayer = pPlayer->next;
    }
  }

  return;
}

BOOLEAN SaveStrategicMovementGroupsToSaveGameFile(HWFILE hFile) {
  struct GROUP *pGroup = NULL;
  uint32_t uiNumberOfGroups = 0;
  uint32_t uiNumBytesWritten = 0;

  pGroup = gpGroupList;

  // Count the number of active groups
  while (pGroup) {
    uiNumberOfGroups++;
    pGroup = pGroup->next;
  }

  // Save the number of movement groups to the saved game file
  FileMan_Write(hFile, &uiNumberOfGroups, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  pGroup = gpGroupList;

  // Loop through the linked lists and add each node
  while (pGroup) {
    // Save each node in the LL
    FileMan_Write(hFile, pGroup, sizeof(struct GROUP), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(struct GROUP)) {
      // Error Writing group node to disk
      return (FALSE);
    }

    //
    // Save the linked list, for the current type of group
    //

    // If its a player group
    if (pGroup->fPlayer) {
      // if there is a player list, add it
      if (pGroup->ubGroupSize) {
        // Save the player group list
        SavePlayerGroupList(hFile, pGroup);
      }
    } else  // else its an enemy group
    {
      // Make sure the pointer is valid
      Assert(pGroup->pEnemyGroup);

      //
      SaveEnemyGroupStruct(hFile, pGroup);
    }

    // Save the waypoint list for the group, if they have one
    SaveWayPointList(hFile, pGroup);

    pGroup = pGroup->next;
  }

  // Save the unique id mask
  FileMan_Write(hFile, uniqueIDMask, sizeof(uint32_t) * 8, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t) * 8) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadStrategicMovementGroupsFromSavedGameFile(HWFILE hFile) {
  struct GROUP *pGroup = NULL;
  struct GROUP *pTemp = NULL;
  uint32_t uiNumberOfGroups = 0;
  // uint32_t	uiNumBytesWritten=0;
  uint32_t uiNumBytesRead = 0;
  uint32_t cnt;
  uint32_t bit, index, mask;
  uint8_t ubNumPlayerGroupsEmpty = 0;
  uint8_t ubNumEnemyGroupsEmpty = 0;
  uint8_t ubNumPlayerGroupsFull = 0;
  uint8_t ubNumEnemyGroupsFull = 0;

  // delete the existing group list
  while (gpGroupList) RemoveGroupFromList(gpGroupList);

  // load the number of nodes in the list
  FileMan_Read(hFile, &uiNumberOfGroups, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  pGroup = gpGroupList;

  // loop through all the nodes and add them to the LL
  for (cnt = 0; cnt < uiNumberOfGroups; cnt++) {
    // allocate memory for the node
    pTemp = (struct GROUP *)MemAlloc(sizeof(struct GROUP));
    if (pTemp == NULL) return (FALSE);
    memset(pTemp, 0, sizeof(struct GROUP));

    // Read in the node
    FileMan_Read(hFile, pTemp, sizeof(struct GROUP), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(struct GROUP)) {
      // Error Writing size of L.L. to disk
      return (FALSE);
    }

    //
    // Add either the pointer or the linked list.
    //

    if (pTemp->fPlayer) {
      // if there is a player list, add it
      if (pTemp->ubGroupSize) {
        // Save the player group list
        LoadPlayerGroupList(hFile, &pTemp);
      }
    } else  // else its an enemy group
    {
      LoadEnemyGroupStructFromSavedGame(hFile, pTemp);
    }

    // Save the waypoint list for the group, if they have one
    LoadWayPointList(hFile, pTemp);

    pTemp->next = NULL;

    // add the node to the list

    // if its the firs node
    if (cnt == 0) {
      gpGroupList = pTemp;
      pGroup = gpGroupList;
    } else {
      pGroup->next = pTemp;
      pGroup = pGroup->next;
    }
  }

  // Load the unique id mask
  FileMan_Read(hFile, uniqueIDMask, sizeof(uint32_t) * 8, &uiNumBytesRead);

  //@@@ TEMP!
  // Rebuild the uniqueIDMask as a very old bug broke the uniqueID assignments in extremely rare
  // cases.
  memset(uniqueIDMask, 0, sizeof(uint32_t) * 8);
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup->fPlayer) {
      if (pGroup->ubGroupSize) {
        ubNumPlayerGroupsFull++;
      } else {
        ubNumPlayerGroupsEmpty++;
      }
    } else {
      if (pGroup->ubGroupSize) {
        ubNumEnemyGroupsFull++;
      } else {
        ubNumEnemyGroupsEmpty++;
      }
    }
    if (ubNumPlayerGroupsEmpty || ubNumEnemyGroupsEmpty) {
      // report error?
    }
    index = pGroup->ubGroupID / 32;
    bit = pGroup->ubGroupID % 32;
    mask = 1 << bit;
    uniqueIDMask[index] += mask;
    pGroup = pGroup->next;
  }

  if (uiNumBytesRead != sizeof(uint32_t) * 8) {
    return (FALSE);
  }

  return (TRUE);
}

// Saves the Player's group list to the saved game file
BOOLEAN SavePlayerGroupList(HWFILE hFile, struct GROUP *pGroup) {
  uint32_t uiNumberOfNodesInList = 0;
  PLAYERGROUP *pTemp = NULL;
  uint32_t uiNumBytesWritten = 0;
  uint32_t uiProfileID;

  pTemp = pGroup->pPlayerList;

  while (pTemp) {
    uiNumberOfNodesInList++;
    pTemp = pTemp->next;
  }

  // Save the number of nodes in the list
  FileMan_Write(hFile, &uiNumberOfNodesInList, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  pTemp = pGroup->pPlayerList;

  // Loop trhough and save only the players profile id
  while (pTemp) {
    // Save the ubProfile ID for this node
    uiProfileID = pTemp->ubProfileID;
    FileMan_Write(hFile, &uiProfileID, sizeof(uint32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint32_t)) {
      // Error Writing size of L.L. to disk
      return (FALSE);
    }

    pTemp = pTemp->next;
  }

  return (TRUE);
}

BOOLEAN LoadPlayerGroupList(HWFILE hFile, struct GROUP **pGroup) {
  PLAYERGROUP *pTemp = NULL;
  PLAYERGROUP *pHead = NULL;
  uint32_t uiNumberOfNodes = 0;
  uint32_t uiProfileID = 0;
  uint32_t uiNumBytesRead;
  uint32_t cnt = 0;
  int16_t sTempID;
  struct GROUP *pTempGroup = *pGroup;

  //	pTemp = pGroup;

  //	pHead = *pGroup->pPlayerList;

  // Load the number of nodes in the player list
  FileMan_Read(hFile, &uiNumberOfNodes, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  // loop through all the nodes and set them up
  for (cnt = 0; cnt < uiNumberOfNodes; cnt++) {
    // allcate space for the current node
    pTemp = (PLAYERGROUP *)MemAlloc(sizeof(PLAYERGROUP));
    if (pTemp == NULL) return (FALSE);

    // Load the ubProfile ID for this node
    FileMan_Read(hFile, &uiProfileID, sizeof(uint32_t), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(uint32_t)) {
      // Error Writing size of L.L. to disk
      return (FALSE);
    }

    // Set up the current node
    pTemp->ubProfileID = (uint8_t)uiProfileID;
    sTempID = GetSoldierIDFromMercID(pTemp->ubProfileID);

    // Should never happen
    // Assert( sTempID != -1 );
    pTemp->ubID = (uint8_t)sTempID;

    pTemp->pSoldier = GetSoldierByID(pTemp->ubID);

    pTemp->next = NULL;

    // if its the first time through
    if (cnt == 0) {
      pTempGroup->pPlayerList = pTemp;
      pHead = pTemp;
    } else {
      pHead->next = pTemp;

      // move to the next node
      pHead = pHead->next;
    }
  }

  return (TRUE);
}

// Saves the enemy group struct to the saved game struct
BOOLEAN SaveEnemyGroupStruct(HWFILE hFile, struct GROUP *pGroup) {
  uint32_t uiNumBytesWritten = 0;

  // Save the enemy struct info to the saved game file
  FileMan_Write(hFile, pGroup->pEnemyGroup, sizeof(ENEMYGROUP), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(ENEMYGROUP)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  return (TRUE);
}

// Loads the enemy group struct from the saved game file
BOOLEAN LoadEnemyGroupStructFromSavedGame(HWFILE hFile, struct GROUP *pGroup) {
  uint32_t uiNumBytesRead = 0;
  ENEMYGROUP *pEnemyGroup = NULL;

  // Alllocate memory for the enemy struct
  pEnemyGroup = (ENEMYGROUP *)MemAlloc(sizeof(ENEMYGROUP));
  if (pEnemyGroup == NULL) return (FALSE);
  memset(pEnemyGroup, 0, sizeof(ENEMYGROUP));

  // Load the enemy struct
  FileMan_Read(hFile, pEnemyGroup, sizeof(ENEMYGROUP), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(ENEMYGROUP)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  // Assign the struct to the group list
  pGroup->pEnemyGroup = pEnemyGroup;

  return (TRUE);
}

void CheckMembersOfMvtGroupAndComplainAboutBleeding(struct SOLDIERTYPE *pSoldier) {
  // run through members of group
  uint8_t ubGroupId = pSoldier->ubGroupID;
  struct GROUP *pGroup;
  PLAYERGROUP *pPlayer = NULL;
  struct SOLDIERTYPE *pCurrentSoldier = NULL;

  pGroup = GetGroup(ubGroupId);

  // valid group?
  if (pGroup == NULL) {
    return;
  }

  // player controlled group?
  if (pGroup->fPlayer == FALSE) {
    return;
  }

  // make sure there are members in the group..if so, then run through and make each bleeder compain
  pPlayer = pGroup->pPlayerList;

  // is there a player list?
  if (pPlayer == NULL) {
    return;
  }

  BeginLoggingForBleedMeToos(TRUE);

  while (pPlayer) {
    pCurrentSoldier = pPlayer->pSoldier;

    if (pCurrentSoldier->bBleeding > 0) {
      // complain about bleeding
      TacticalCharacterDialogue(pCurrentSoldier, QUOTE_STARTING_TO_BLEED);
    }
    pPlayer = pPlayer->next;
  }

  BeginLoggingForBleedMeToos(FALSE);
}

BOOLEAN SaveWayPointList(HWFILE hFile, struct GROUP *pGroup) {
  uint32_t cnt = 0;
  uint32_t uiNumberOfWayPoints = 0;
  uint32_t uiNumBytesWritten = 0;
  WAYPOINT *pWayPoints = pGroup->pWaypoints;

  // loop trhough and count all the node in the waypoint list
  while (pWayPoints != NULL) {
    uiNumberOfWayPoints++;
    pWayPoints = pWayPoints->next;
  }

  // Save the number of waypoints
  FileMan_Write(hFile, &uiNumberOfWayPoints, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  if (uiNumberOfWayPoints) {
    pWayPoints = pGroup->pWaypoints;
    for (cnt = 0; cnt < uiNumberOfWayPoints; cnt++) {
      // Save the waypoint node
      FileMan_Write(hFile, pWayPoints, sizeof(WAYPOINT), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(WAYPOINT)) {
        // Error Writing size of L.L. to disk
        return (FALSE);
      }

      // Advance to the next waypoint
      pWayPoints = pWayPoints->next;
    }
  }

  return (TRUE);
}

BOOLEAN LoadWayPointList(HWFILE hFile, struct GROUP *pGroup) {
  uint32_t cnt = 0;
  uint32_t uiNumberOfWayPoints = 0;
  uint32_t uiNumBytesRead = 0;
  WAYPOINT *pWayPoints = pGroup->pWaypoints;
  WAYPOINT *pTemp = NULL;

  // Load the number of waypoints
  FileMan_Read(hFile, &uiNumberOfWayPoints, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    // Error Writing size of L.L. to disk
    return (FALSE);
  }

  if (uiNumberOfWayPoints) {
    pWayPoints = pGroup->pWaypoints;
    for (cnt = 0; cnt < uiNumberOfWayPoints; cnt++) {
      // Allocate memory for the node
      pTemp = (WAYPOINT *)MemAlloc(sizeof(WAYPOINT));
      if (pTemp == NULL) return (FALSE);
      memset(pTemp, 0, sizeof(WAYPOINT));

      // Load the waypoint node
      FileMan_Read(hFile, pTemp, sizeof(WAYPOINT), &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(WAYPOINT)) {
        // Error Writing size of L.L. to disk
        return (FALSE);
      }

      pTemp->next = NULL;

      // if its the first node
      if (cnt == 0) {
        pGroup->pWaypoints = pTemp;
        pWayPoints = pTemp;
      } else {
        pWayPoints->next = pTemp;

        // Advance to the next waypoint
        pWayPoints = pWayPoints->next;
      }
    }
  } else
    pGroup->pWaypoints = NULL;

  return (TRUE);
}

void CalculateGroupRetreatSector(struct GROUP *pGroup) {
  SECTORINFO *pSector;
  uint32_t uiSectorID;

  uiSectorID = GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY);
  pSector = &SectorInfo[uiSectorID];

  if (pSector->ubTraversability[NORTH_STRATEGIC_MOVE] != GROUNDBARRIER &&
      pSector->ubTraversability[NORTH_STRATEGIC_MOVE] != EDGEOFWORLD) {
    pGroup->ubPrevX = pGroup->ubSectorX;
    pGroup->ubPrevY = pGroup->ubSectorY - 1;
  } else if (pSector->ubTraversability[EAST_STRATEGIC_MOVE] != GROUNDBARRIER &&
             pSector->ubTraversability[EAST_STRATEGIC_MOVE] != EDGEOFWORLD) {
    pGroup->ubPrevX = pGroup->ubSectorX + 1;
    pGroup->ubPrevY = pGroup->ubSectorY;
  } else if (pSector->ubTraversability[WEST_STRATEGIC_MOVE] != GROUNDBARRIER &&
             pSector->ubTraversability[WEST_STRATEGIC_MOVE] != EDGEOFWORLD) {
    pGroup->ubPrevX = pGroup->ubSectorX - 1;
    pGroup->ubPrevY = pGroup->ubSectorY;
  } else if (pSector->ubTraversability[SOUTH_STRATEGIC_MOVE] != GROUNDBARRIER &&
             pSector->ubTraversability[SOUTH_STRATEGIC_MOVE] != EDGEOFWORLD) {
    pGroup->ubPrevX = pGroup->ubSectorX;
    pGroup->ubPrevY = pGroup->ubSectorY + 1;
  } else {
    AssertMsg(0, String("Player group cannot retreat from sector %c%d ",
                        pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX));
    return;
  }
  if (pGroup->fPlayer) {  // update the previous sector for the mercs
    PLAYERGROUP *pPlayer;
    pPlayer = pGroup->pPlayerList;
    while (pPlayer) {
      pPlayer->pSoldier->ubPrevSectorID = (uint8_t)GetSectorID8(pGroup->ubPrevX, pGroup->ubPrevY);
      pPlayer = pPlayer->next;
    }
  }
}

// Called when all checks have been made for the group (if possible to retreat, etc.)  This function
// blindly determines where to move the group.
void RetreatGroupToPreviousSector(struct GROUP *pGroup) {
  uint8_t ubSector, ubDirection = 255;
  int32_t iVehId, dx, dy;
  Assert(pGroup);
  AssertMsg(!pGroup->fBetweenSectors, "Can't retreat a group when between sectors!");

  if (pGroup->ubPrevX != 16 || pGroup->ubPrevY != 16) {  // Group has a previous sector
    pGroup->ubNextX = pGroup->ubPrevX;
    pGroup->ubNextY = pGroup->ubPrevY;

    // Determine the correct direction.
    dx = pGroup->ubNextX - pGroup->ubSectorX;
    dy = pGroup->ubNextY - pGroup->ubSectorY;
    if (dy == -1 && !dx)
      ubDirection = NORTH_STRATEGIC_MOVE;
    else if (dx == 1 && !dy)
      ubDirection = EAST_STRATEGIC_MOVE;
    else if (dy == 1 && !dx)
      ubDirection = SOUTH_STRATEGIC_MOVE;
    else if (dx == -1 && !dy)
      ubDirection = WEST_STRATEGIC_MOVE;
    else {
      AssertMsg(0, String("Player group attempting illegal retreat from %c%d to %c%d.",
                          pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX, pGroup->ubNextY + 'A' - 1,
                          pGroup->ubNextX));
    }
  } else {  // Group doesn't have a previous sector.  Create one, then recurse
    CalculateGroupRetreatSector(pGroup);
    RetreatGroupToPreviousSector(pGroup);
  }

  // Calc time to get to next waypoint...
  ubSector = (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY);
  pGroup->uiTraverseTime = GetSectorMvtTimeForGroup(ubSector, ubDirection, pGroup);
  if (pGroup->uiTraverseTime == 0xffffffff) {
    AssertMsg(
        0, String("Group %d (%s) attempting illegal move from %c%d to %c%d (%s).",
                  pGroup->ubGroupID, (pGroup->fPlayer) ? "Player" : "AI", pGroup->ubSectorY + 'A',
                  pGroup->ubSectorX, pGroup->ubNextY + 'A', pGroup->ubNextX,
                  gszTerrain[GetSectorInfoByID8(ubSector)->ubTraversability[ubDirection]]));
  }

  if (!pGroup->uiTraverseTime) {  // Because we are in the strategic layer, don't make the arrival
                                  // instantaneous (towns).
    pGroup->uiTraverseTime = 5;
  }

  SetGroupArrivalTime(pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime);
  pGroup->fBetweenSectors = TRUE;
  pGroup->uiFlags |= GROUPFLAG_JUST_RETREATED_FROM_BATTLE;

  if (pGroup->fVehicle == TRUE) {
    // vehicle, set fact it is between sectors too
    if ((iVehId = (GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID))) != -1) {
      pVehicleList[iVehId].fBetweenSectors = TRUE;
    }
  }

  // Post the event!
  if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID))
    AssertMsg(0, "Failed to add movement event.");

  // For the case of player groups, we need to update the information of the soldiers.
  if (pGroup->fPlayer) {
    PLAYERGROUP *curr;
    curr = pGroup->pPlayerList;

    if (pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin()) {
      AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY,
                        pGroup->ubGroupID);
    }

    while (curr) {
      curr->pSoldier->fBetweenSectors = TRUE;

      // OK, Remove the guy from tactical engine!
      RemoveSoldierFromTacticalSector(curr->pSoldier, TRUE);

      curr = curr->next;
    }
  }
}

struct GROUP *FindMovementGroupInSector(uint8_t ubSectorX, uint8_t ubSectorY, BOOLEAN fPlayer) {
  struct GROUP *pGroup;
  pGroup = gpGroupList;
  while (pGroup) {
    if (pGroup->fPlayer) {
      // NOTE: These checks must always match the INVOLVED group checks in PBI!!!
      if (fPlayer && pGroup->ubGroupSize && !pGroup->fBetweenSectors &&
          pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY && !pGroup->ubSectorZ &&
          !GroupHasInTransitDeadOrPOWMercs(pGroup) &&
          (!IsGroupTheHelicopterGroup(pGroup) || !fHelicopterIsAirBorne)) {
        return pGroup;
      }
    } else if (!fPlayer && pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY &&
               !pGroup->ubSectorZ)
      return pGroup;

    pGroup = pGroup->next;
  }
  return NULL;
}

BOOLEAN GroupAtFinalDestination(struct GROUP *pGroup) {
  WAYPOINT *wp;

  if (pGroup->ubMoveType != ONE_WAY)
    return FALSE;  // Group will continue to patrol, hence never stops.

  // Determine if we are at the final waypoint.
  wp = GetFinalWaypoint(pGroup);

  if (!wp) {  // no waypoints, so the group is at it's destination.  This happens when
    // an enemy group is created in the destination sector (which is legal for
    // staging groups which always stop adjacent to their real sector destination)
    return TRUE;
  }

  // if we're there
  if ((pGroup->ubSectorX == wp->x) && (pGroup->ubSectorY == wp->y)) {
    return TRUE;
  }

  return FALSE;
}

WAYPOINT *GetFinalWaypoint(struct GROUP *pGroup) {
  WAYPOINT *wp;

  Assert(pGroup);

  // Make sure they're on a one way route, otherwise this request is illegal
  Assert(pGroup->ubMoveType == ONE_WAY);

  wp = pGroup->pWaypoints;
  if (wp) {
    while (wp->next) {
      wp = wp->next;
    }
  }

  return (wp);
}

// The sector supplied resets ALL enemy groups in the sector specified.  See comments in
// ResetMovementForEnemyGroup() for more details on what the resetting does.
void ResetMovementForEnemyGroupsInLocation(uint8_t ubSectorX, uint8_t ubSectorY) {
  struct GROUP *pGroup, *next;
  uint8_t sSectorX, sSectorY;
  int8_t sSectorZ;

  GetCurrentBattleSectorXYZ(&sSectorX, &sSectorY, &sSectorZ);
  pGroup = gpGroupList;
  while (pGroup) {
    next = pGroup->next;
    if (!pGroup->fPlayer) {
      if (pGroup->ubSectorX == sSectorX && pGroup->ubSectorY == sSectorY) {
        ResetMovementForEnemyGroup(pGroup);
      }
    }
    pGroup = next;
  }
}

// This function is used to reset the location of the enemy group if they are
// currently between sectors.  If they were 50% of the way from sector A10 to A11,
// then after this function is called, then that group would be 0% of the way from
// sector A10 to A11.  In no way does this function effect the strategic path for
// the group.
void ResetMovementForEnemyGroup(struct GROUP *pGroup) {
  // Validate that the group is an enemy group and that it is moving.
  if (pGroup->fPlayer) {
    return;
  }
  if (!pGroup->fBetweenSectors || !pGroup->ubNextX ||
      !pGroup->ubNextY) {  // Reset the group's assignment by moving it to the group's original
                           // sector as it's pending group.
    RepollSAIGroup(pGroup);
    return;
  }

  // Cancel the event that is posted.
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // Calculate the new arrival time (all data pertaining to movement should be valid)
  if (pGroup->uiTraverseTime > 400) {  // The group was likely sleeping which makes for extremely
                                       // long arrival times.  Shorten it
    // arbitrarily.  Doesn't really matter if this isn't accurate.
    pGroup->uiTraverseTime = 90;
  }
  SetGroupArrivalTime(pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime);

  // Add a new event
  AddStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID);
}

void UpdatePersistantGroupsFromOldSave(uint32_t uiSavedGameVersion) {
  struct GROUP *pGroup = NULL;
  BOOLEAN fDone = FALSE;
  int32_t cnt;
  BOOLEAN fDoChange = FALSE;

  // ATE: If saved game is < 61, we need to do something better!
  if (uiSavedGameVersion < 61) {
    for (cnt = 0; cnt < 55; cnt++) {
      // create mvt groups
      pGroup = GetGroup((uint8_t)cnt);

      if (pGroup != NULL && pGroup->fPlayer) {
        pGroup->fPersistant = TRUE;
      }
    }

    fDoChange = TRUE;
  } else if (uiSavedGameVersion < 63) {
    for (cnt = 0; cnt < NUMBER_OF_SQUADS; cnt++) {
      // create mvt groups
      pGroup = GetGroup(SquadMovementGroups[cnt]);

      if (pGroup != NULL) {
        pGroup->fPersistant = TRUE;
      }
    }

    for (cnt = 0; cnt < MAX_VEHICLES; cnt++) {
      pGroup = GetGroup(gubVehicleMovementGroups[cnt]);

      if (pGroup != NULL) {
        pGroup->fPersistant = TRUE;
      }
    }

    fDoChange = TRUE;
  }

  if (fDoChange) {
    // Remove all empty groups
    fDone = FALSE;
    while (!fDone) {
      pGroup = gpGroupList;
      while (pGroup) {
        if (!pGroup->ubGroupSize && !pGroup->fPersistant) {
          RemovePGroup(pGroup);
          break;
        }
        pGroup = pGroup->next;
        if (!pGroup) {
          fDone = TRUE;
        }
      }
    }
  }
}

// Determines if any particular group WILL be moving through a given sector given it's current
// position in the route and the pGroup->ubMoveType must be ONE_WAY.  If the group is currently
// IN the sector, or just left the sector, it will return FALSE.
BOOLEAN GroupWillMoveThroughSector(struct GROUP *pGroup, uint8_t ubSectorX, uint8_t ubSectorY) {
  WAYPOINT *wp;
  int32_t i, dx, dy;
  uint8_t ubOrigX, ubOrigY;

  Assert(pGroup);
  AssertMsg(pGroup->ubMoveType == ONE_WAY,
            String("GroupWillMoveThroughSector() -- Attempting to test group with an invalid move "
                   "type.  ubGroupID: %d, ubMoveType: %d, sector: %c%d -- KM:0",
                   pGroup->ubGroupID, pGroup->ubMoveType, pGroup->ubSectorY + 'A' - 1,
                   pGroup->ubSectorX));

  // Preserve the original sector values, as we will be temporarily modifying the group's
  // ubSectorX/Y values as we traverse the waypoints.
  ubOrigX = pGroup->ubSectorX;
  ubOrigY = pGroup->ubSectorY;

  i = pGroup->ubNextWaypointID;
  wp = pGroup->pWaypoints;

  if (!wp) {  // This is a floating group!?
    return FALSE;
  }
  while (i--) {  // Traverse through the waypoint list to the next waypoint ID
    Assert(wp);
    wp = wp->next;
  }
  Assert(wp);

  while (wp) {
    while (pGroup->ubSectorX != wp->x || pGroup->ubSectorY != wp->y) {
      // We now have the correct waypoint.
      // Analyse the group and determine which direction it will move from the current sector.
      dx = wp->x - pGroup->ubSectorX;
      dy = wp->y - pGroup->ubSectorY;
      if (dx && dy) {  // Can't move diagonally!
        AssertMsg(
            0,
            String("GroupWillMoveThroughSector() -- Attempting to process waypoint in a diagonal "
                   "direction from sector %c%d to sector %c%d for group at sector %c%d -- KM:0",
                   pGroup->ubSectorY + 'A', pGroup->ubSectorX, wp->y + 'A' - 1, wp->x,
                   ubOrigY + 'A' - 1, ubOrigX));
        pGroup->ubSectorX = ubOrigX;
        pGroup->ubSectorY = ubOrigY;
        return TRUE;
      }
      if (!dx && !dy)  // Can't move to position currently at!
      {
        AssertMsg(0, String("GroupWillMoveThroughSector() -- Attempting to process same waypoint "
                            "at %c%d for group at %c%d -- KM:0",
                            wp->y + 'A' - 1, wp->x, ubOrigY + 'A' - 1, ubOrigX));
        pGroup->ubSectorX = ubOrigX;
        pGroup->ubSectorY = ubOrigY;
        return TRUE;
      }
      // Clip dx/dy value so that the move is for only one sector.
      if (dx >= 1) {
        dx = 1;
      } else if (dy >= 1) {
        dy = 1;
      } else if (dx <= -1) {
        dx = -1;
      } else if (dy <= -1) {
        dy = -1;
      } else {
        Assert(0);
        pGroup->ubSectorX = ubOrigX;
        pGroup->ubSectorY = ubOrigY;
        return TRUE;
      }
      // Advance the sector value
      pGroup->ubSectorX = (uint8_t)(dx + pGroup->ubSectorX);
      pGroup->ubSectorY = (uint8_t)(dy + pGroup->ubSectorY);
      // Check to see if it the sector we are checking to see if this group will be moving through.
      if (pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY) {
        pGroup->ubSectorX = ubOrigX;
        pGroup->ubSectorY = ubOrigY;
        return TRUE;
      }
    }
    // Advance to the next waypoint.
    wp = wp->next;
  }
  pGroup->ubSectorX = ubOrigX;
  pGroup->ubSectorY = ubOrigY;
  return FALSE;
}

int16_t CalculateFuelCostBetweenSectors(uint8_t ubSectorID1, uint8_t ubSectorID2) { return (0); }

BOOLEAN VehicleHasFuel(struct SOLDIERTYPE *pSoldier) {
  Assert(pSoldier->uiStatusFlags & SOLDIER_VEHICLE);
  if (pSoldier->sBreathRed) {
    return TRUE;
  }
  return FALSE;
}

int16_t VehicleFuelRemaining(struct SOLDIERTYPE *pSoldier) {
  Assert(pSoldier->uiStatusFlags & SOLDIER_VEHICLE);
  return pSoldier->sBreathRed;
}

BOOLEAN SpendVehicleFuel(struct SOLDIERTYPE *pSoldier, int16_t sFuelSpent) {
  Assert(pSoldier->uiStatusFlags & SOLDIER_VEHICLE);
  pSoldier->sBreathRed -= sFuelSpent;
  pSoldier->sBreathRed = (int16_t)max(0, pSoldier->sBreathRed);
  pSoldier->bBreath = (int8_t)((pSoldier->sBreathRed + 99) / 100);
  return (FALSE);
}

void AddFuelToVehicle(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pVehicle) {
  struct OBJECTTYPE *pItem;
  int16_t sFuelNeeded, sFuelAvailable, sFuelAdded;
  pItem = &pSoldier->inv[HANDPOS];
  if (pItem->usItem != GAS_CAN) {
#ifdef JA2BETAVERSION
    wchar_t str[100];
    swprintf(str, ARR_SIZE(str), L"%s is supposed to have gas can in hand.  ATE:0", pSoldier->name);
    DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
#endif
    return;
  }
  // Soldier has gas can, so now add gas to vehicle while removing gas from the gas can.
  // A gas can with 100 status translate to 50% of a fillup.
  if (pVehicle->sBreathRed == 10000) {  // Message for vehicle full?
    return;
  }
  if (pItem->bStatus) {  // Fill 'er up.
    sFuelNeeded = 10000 - pVehicle->sBreathRed;
    sFuelAvailable = pItem->bStatus[0] * 50;
    sFuelAdded = min(sFuelNeeded, sFuelAvailable);
    // Add to vehicle
    pVehicle->sBreathRed += sFuelAdded;
    pVehicle->bBreath = (int8_t)(pVehicle->sBreathRed / 100);
    // Subtract from item
    pItem->bStatus[0] = (int8_t)(pItem->bStatus[0] - sFuelAdded / 50);
    if (!pItem->bStatus[0]) {  // Gas can is empty, so toast the item.
      DeleteObj(pItem);
    }
  }
}

void ReportVehicleOutOfGas(int32_t iVehicleID, uint8_t ubSectorX, uint8_t ubSectorY) {
  wchar_t str[255];
  // Report that the vehicle that just arrived is out of gas.
  swprintf(str, ARR_SIZE(str), gzLateLocalizedString[5],
           pVehicleStrings[pVehicleList[iVehicleID].ubVehicleType], ubSectorY + 'A' - 1, ubSectorX);
  DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
}

void SetLocationOfAllPlayerSoldiersInGroup(struct GROUP *pGroup, uint8_t sSectorX, uint8_t sSectorY,
                                           int8_t bSectorZ) {
  PLAYERGROUP *pPlayer = NULL;
  struct SOLDIERTYPE *pSoldier = NULL;

  pPlayer = pGroup->pPlayerList;
  while (pPlayer) {
    pSoldier = pPlayer->pSoldier;

    if (pSoldier != NULL) {
      pSoldier->sSectorX = sSectorX;
      pSoldier->sSectorY = sSectorY;
      pSoldier->bSectorZ = bSectorZ;
    }

    pPlayer = pPlayer->next;
  }

  // if it's a vehicle
  if (pGroup->fVehicle) {
    int32_t iVehicleId = -1;
    VEHICLETYPE *pVehicle = NULL;

    iVehicleId = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
    Assert(iVehicleId != -1);

    pVehicle = &(pVehicleList[iVehicleId]);

    pVehicle->sSectorX = sSectorX;
    pVehicle->sSectorY = sSectorY;
    pVehicle->sSectorZ = bSectorZ;

    // if it ain't the chopper
    if (iVehicleId != iHelicopterVehicleId) {
      pSoldier = GetSoldierStructureForVehicle(iVehicleId);
      Assert(pSoldier);

      // these are apparently unnecessary, since vehicles are part of the pPlayerList in a vehicle
      // group.  Oh well.
      pSoldier->sSectorX = sSectorX;
      pSoldier->sSectorY = sSectorY;
      pSoldier->bSectorZ = bSectorZ;
    }
  }
}

void RandomizePatrolGroupLocation(
    struct GROUP *pGroup) {  // Make sure this is an enemy patrol group
  WAYPOINT *wp;
  uint8_t ubMaxWaypointID = 0;
  uint8_t ubTotalWaypoints;
  uint8_t ubChosen;
  uint8_t ubSectorID;

  // return; //disabled for now

  Assert(!pGroup->fPlayer);
  Assert(pGroup->ubMoveType == ENDTOEND_FORWARDS);
  Assert(pGroup->pEnemyGroup->ubIntention == PATROL);

  // Search for the event, and kill it (if it exists)!
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // count the group's waypoints
  wp = pGroup->pWaypoints;
  while (wp) {
    if (wp->next) {
      ubMaxWaypointID++;
    }
    wp = wp->next;
  }
  // double it (they go back and forth) -- it's using zero based indices, so you have to add one to
  // get the number of actual waypoints in one direction.
  ubTotalWaypoints = (uint8_t)((ubMaxWaypointID) * 2);

  // pick the waypoint they start at
  ubChosen = (uint8_t)Random(ubTotalWaypoints);

  if (ubChosen >=
      ubMaxWaypointID) {  // They chose a waypoint going in the reverse direction, so translate it
    // to an actual waypointID and switch directions.
    pGroup->ubMoveType = ENDTOEND_BACKWARDS;
    pGroup->ubNextWaypointID = ubChosen - ubMaxWaypointID;
    ubChosen = pGroup->ubNextWaypointID + 1;
  } else {
    pGroup->ubMoveType = ENDTOEND_FORWARDS;
    pGroup->ubNextWaypointID = ubChosen + 1;
  }

  // Traverse through the waypoint list again, to extract the location they are at.
  wp = pGroup->pWaypoints;
  while (wp && ubChosen) {
    ubChosen--;
    wp = wp->next;
  }

  // logic error if this fails.  We should have a null value for ubChosen
  Assert(!ubChosen);
  Assert(wp);

  // Move the group to the location of this chosen waypoint.
  ubSectorID = (uint8_t)GetSectorID8(wp->x, wp->y);

  // Set up this global var to randomize the arrival time of the group from
  // 1 minute to actual traverse time between the sectors.
  gfRandomizingPatrolGroup = TRUE;

  SetEnemyGroupSector(pGroup, ubSectorID);
  InitiateGroupMovementToNextSector(pGroup);

  // Immediately turn off the flag once finished.
  gfRandomizingPatrolGroup = FALSE;
}

// Whenever a player group arrives in a sector, and if bloodcats exist in the sector,
// roll the dice to see if this will become an ambush random encounter.
BOOLEAN TestForBloodcatAmbush(struct GROUP *pGroup) {
  SECTORINFO *pSector;
  int32_t iHoursElapsed;
  uint8_t ubSectorID;
  uint8_t ubChance;
  int8_t bDifficultyMaxCats;
  int8_t bProgressMaxCats;
  int8_t bNumMercMaxCats;
  BOOLEAN fAlreadyAmbushed = FALSE;

  if (pGroup->ubSectorZ) {  // no ambushes underground (no bloodcats either)
    return FALSE;
  }

  ubSectorID = (uint8_t)GetSectorID8(pGroup->ubSectorX, pGroup->ubSectorY);
  pSector = &SectorInfo[ubSectorID];

  ubChance = 5 * gGameOptions.ubDifficultyLevel;

  iHoursElapsed = (GetWorldTotalMin() - pSector->uiTimeCurrentSectorWasLastLoaded) / 60;
  if (ubSectorID == SEC_N5 ||
      ubSectorID == SEC_I16) {  // These are special maps -- we use all placements.
    if (pSector->bBloodCats == -1) {
      pSector->bBloodCats = pSector->bBloodCatPlacements;
    } else if (pSector->bBloodCats > 0 &&
               pSector->bBloodCats <
                   pSector
                       ->bBloodCatPlacements) {  // Slowly have them recuperate if we haven't been
                                                 // here for a long time.  The population will
      // come back up to the maximum if left long enough.
      int32_t iBloodCatDiff;
      iBloodCatDiff = pSector->bBloodCatPlacements - pSector->bBloodCats;
      pSector->bBloodCats += (int8_t)min(iHoursElapsed / 18, iBloodCatDiff);
    }
    // Once 0, the bloodcats will never recupe.
  } else if (pSector->bBloodCats == -1) {  // If we haven't been ambushed by bloodcats yet...
    if (gfAutoAmbush || PreChance(ubChance)) {
      // randomly choose from 5-8, 7-10, 9-12 bloodcats based on easy, normal, and hard,
      // respectively
      bDifficultyMaxCats = (int8_t)(Random(4) + gGameOptions.ubDifficultyLevel * 2 + 3);

      // maximum of 3 bloodcats or 1 for every 6%, 5%, 4% progress based on easy, normal, and hard,
      // respectively
      bProgressMaxCats =
          (int8_t)max(CurrentPlayerProgressPercentage() / (7 - gGameOptions.ubDifficultyLevel), 3);

      // make sure bloodcats don't outnumber mercs by a factor greater than 2
      bNumMercMaxCats =
          (int8_t)(PlayerMercsInSector(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ) *
                   2);

      // choose the lowest number of cats calculated by difficulty and progress.
      pSector->bBloodCats = (int8_t)min(bDifficultyMaxCats, bProgressMaxCats);

      if (gGameOptions.ubDifficultyLevel !=
          DIF_LEVEL_HARD) {  // if not hard difficulty, ensure cats never outnumber mercs by a
                             // factor of 2 (min 3 bloodcats)
        pSector->bBloodCats = (int8_t)min(pSector->bBloodCats, bNumMercMaxCats);
        pSector->bBloodCats = (int8_t)max(pSector->bBloodCats, 3);
      }

      // ensure that there aren't more bloodcats than placements
      pSector->bBloodCats = (int8_t)min(pSector->bBloodCats, pSector->bBloodCatPlacements);
    }
  } else if (ubSectorID != SEC_I16) {
    if (!gfAutoAmbush &&
        PreChance(95)) {  // already ambushed here.  But 5% chance of getting ambushed again!
      fAlreadyAmbushed = TRUE;
    }
  }

  if (!fAlreadyAmbushed && ubSectorID != SEC_N5 && pSector->bBloodCats > 0 && !pGroup->fVehicle &&
      !NumEnemiesInSector(pGroup->ubSectorX, pGroup->ubSectorY)) {
    if (ubSectorID != SEC_I16 || !gubFact[FACT_PLAYER_KNOWS_ABOUT_BLOODCAT_LAIR]) {
      gubEnemyEncounterCode = BLOODCAT_AMBUSH_CODE;
    } else {
      gubEnemyEncounterCode = ENTERING_BLOODCAT_LAIR_CODE;
    }
    return TRUE;
  } else {
    gubEnemyEncounterCode = NO_ENCOUNTER_CODE;
    return FALSE;
  }
}

void NotifyPlayerOfBloodcatBattle(uint8_t ubSectorX, uint8_t ubSectorY) {
  wchar_t str[256];
  wchar_t zTempString[128];
  if (gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE) {
    GetSectorIDString(ubSectorX, ubSectorY, 0, zTempString, ARR_SIZE(zTempString), TRUE);
    swprintf(str, ARR_SIZE(str), pMapErrorString[12], zTempString);
  } else if (gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE) {
    wcscpy(str, pMapErrorString[13]);
  }

  if (IsMapScreen_2()) {  // Force render mapscreen (need to update the position of
                          // the group before the dialog appears.
    MarkForRedrawalStrategicMap();
    MapScreenHandle();
    InvalidateScreen();
    RefreshScreen(NULL);
  }

  gfUsePersistantPBI = TRUE;
  DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, TriggerPrebattleInterface);
}

void PlaceGroupInSector(uint8_t ubGroupID, uint8_t sPrevX, uint8_t sPrevY, uint8_t sNextX,
                        uint8_t sNextY, int8_t bZ, BOOLEAN fCheckForBattle) {
  ClearMercPathsAndWaypointsForAllInGroup(GetGroup(ubGroupID));

  // change where they are and where they're going
  SetGroupPrevSectors(ubGroupID, sPrevX, sPrevY);
  SetGroupSectorValue(sPrevX, sPrevY, bZ, ubGroupID);
  SetGroupNextSectorValue(sNextX, sNextY, ubGroupID);

  // call arrive event
  GroupArrivedAtSector(ubGroupID, fCheckForBattle, FALSE);
}

// ARM: centralized it so we can do a comprehensive Assert on it.  Causing problems with helicopter
// group!
void SetGroupArrivalTime(struct GROUP *pGroup, uint32_t uiArrivalTime) {
  // PLEASE CENTRALIZE ALL CHANGES TO THE ARRIVAL TIMES OF GROUPS THROUGH HERE, ESPECIALLY THE
  // HELICOPTER struct GROUP!!!

  // if this group is the helicopter group, we have to make sure that its arrival time is never
  // greater than the sum of the current time and its traverse time, 'cause those 3 values are used
  // to plot its map position!  Because of this the chopper groups must NEVER be delayed for any
  // reason - it gets excluded from simultaneous arrival logic

  // Also note that non-chopper groups can currently be delayed such that this assetion would fail -
  // enemy groups by DelayEnemyGroupsIfPathsCross(), and player groups via
  // PrepareGroupsForSimultaneousArrival().  So we skip the assert.

  if (IsGroupTheHelicopterGroup(pGroup)) {
    // make sure it's valid (NOTE: the correct traverse time must be set first!)
    if (uiArrivalTime > (GetWorldTotalMin() + pGroup->uiTraverseTime)) {
      AssertMsg(FALSE, String("SetGroupArrivalTime: Setting invalid arrival time %d for group %d, "
                              "WorldTime = %d, TraverseTime = %d",
                              uiArrivalTime, pGroup->ubGroupID, GetWorldTotalMin(),
                              pGroup->uiTraverseTime));

      // fix it if assertions are disabled
      uiArrivalTime = GetWorldTotalMin() + pGroup->uiTraverseTime;
    }
  }

  pGroup->uiArrivalTime = uiArrivalTime;
}

// non-persistent groups should be simply removed instead!
void CancelEmptyPersistentGroupMovement(struct GROUP *pGroup) {
  Assert(pGroup);
  Assert(pGroup->ubGroupSize == 0);
  Assert(pGroup->fPersistant);

  // don't do this for vehicle groups - the chopper can keep flying empty,
  // while other vehicles still exist and teleport to nearest sector instead
  if (pGroup->fVehicle) {
    return;
  }

  // prevent it from arriving empty
  DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, pGroup->ubGroupID);

  // release memory for its waypoints
  RemoveGroupWaypoints(pGroup->ubGroupID);

  pGroup->uiTraverseTime = 0;
  SetGroupArrivalTime(pGroup, 0);
  pGroup->fBetweenSectors = FALSE;

  pGroup->ubPrevX = 0;
  pGroup->ubPrevY = 0;
  pGroup->ubSectorX = 0;
  pGroup->ubSectorY = 0;
  pGroup->ubNextX = 0;
  pGroup->ubNextY = 0;
}

// look for NPCs to stop for, anyone is too tired to keep going, if all OK rebuild waypoints &
// continue movement
void PlayerGroupArrivedSafelyInSector(struct GROUP *pGroup, BOOLEAN fCheckForNPCs) {
  BOOLEAN fPlayerPrompted = FALSE;

  Assert(pGroup);
  Assert(pGroup->fPlayer);

  // if we haven't already checked for NPCs, and the group isn't empty
  if (fCheckForNPCs && (HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(pGroup) == TRUE)) {
    // wait for player to answer/confirm prompt before doing anything else
    fPlayerPrompted = TRUE;
  }

  // if we're not prompting the player
  if (!fPlayerPrompted) {
    // and we're not at the end of our road
    if (!GroupAtFinalDestination(pGroup)) {
      if (AnyMercInGroupCantContinueMoving(pGroup)) {
        // stop: clear their strategic movement (mercpaths and waypoints)
        ClearMercPathsAndWaypointsForAllInGroup(pGroup);

        // NOTE: Of course, it would be better if they continued onwards once everyone was ready to
        // go again, in which case we'd want to preserve the plotted path, but since the player can
        // mess with the squads, etc. in the mean-time, that just seemed to risky to try to support.
        // They could get into a fight and be too injured to move, etc.  Basically, we'd have run a
        // complete CanCharacterMoveInStrategic(0 check on all of them. It's a wish list task for
        // AM...

        // stop time so player can react if group was already on the move and suddenly halts
        StopTimeCompression();
      } else {
        // continue onwards: rebuild way points, initiate movement
        RebuildWayPointsForGroupPath(GetGroupMercPathPtr(pGroup), pGroup->ubGroupID);
      }
    }
  }
}

BOOLEAN HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(struct GROUP *pGroup) {
  uint8_t sSectorX = 0, sSectorY = 0;
  int8_t bSectorZ = 0;
  wchar_t sString[128];
  wchar_t wSectorName[128];
  int16_t sStrategicSector;

  Assert(pGroup);
  Assert(pGroup->fPlayer);

  // nobody in the group (perfectly legal with the chopper)
  if (pGroup->pPlayerList == NULL) {
    return (FALSE);
  }

  // chopper doesn't stop for NPCs
  if (IsGroupTheHelicopterGroup(pGroup)) {
    return (FALSE);
  }

  // if we're already in the middle of a prompt (possible with simultaneously group arrivals!),
  // don't try to prompt again
  if (gpGroupPrompting != NULL) {
    return (FALSE);
  }

  // get the sector values
  sSectorX = pGroup->ubSectorX;
  sSectorY = pGroup->ubSectorY;
  bSectorZ = pGroup->ubSectorZ;

  // don't do this for underground sectors
  if (bSectorZ != 0) {
    return (FALSE);
  }

  // get the strategic sector value
  sStrategicSector = GetSectorID16(sSectorX, sSectorY);

  // skip towns/pseudo-towns (anything that shows up on the map as being special)
  if (StrategicMap[sStrategicSector].townID != BLANK_SECTOR) {
    return (FALSE);
  }

  // skip SAM-sites
  if (IsThisSectorASAMSector(sSectorX, sSectorY, bSectorZ)) {
    return (FALSE);
  }

  // check for profiled NPCs in sector
  if (WildernessSectorWithAllProfiledNPCsNotSpokenWith(sSectorX, sSectorY, bSectorZ) == FALSE) {
    return (FALSE);
  }

  // store the group ptr for use by the callback function
  gpGroupPrompting = pGroup;

  // build string for squad
  GetSectorIDString(sSectorX, sSectorY, bSectorZ, wSectorName, ARR_SIZE(wSectorName), FALSE);
  swprintf(sString, ARR_SIZE(sString), pLandMarkInSectorString[0],
           pGroup->pPlayerList->pSoldier->bAssignment + 1, wSectorName);

  if (GroupAtFinalDestination(pGroup)) {
    // do an OK message box
    DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK,
                                  HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback);
  } else {
    // do a CONTINUE/STOP message box
    DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_CONTINUESTOP,
                                  HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback);
  }

  // wait, we're prompting the player
  return (TRUE);
}

BOOLEAN WildernessSectorWithAllProfiledNPCsNotSpokenWith(uint8_t sSectorX, uint8_t sSectorY,
                                                         int8_t bSectorZ) {
  uint8_t ubProfile;
  MERCPROFILESTRUCT *pProfile;
  BOOLEAN fFoundSomebody = FALSE;

  for (ubProfile = FIRST_RPC; ubProfile < NUM_PROFILES; ubProfile++) {
    pProfile = &gMercProfiles[ubProfile];

    // skip stiffs
    if ((pProfile->bMercStatus == MERC_IS_DEAD) || (pProfile->bLife <= 0)) {
      continue;
    }

    // skip vehicles
    if (ubProfile >= PROF_HUMMER && ubProfile <= PROF_HELICOPTER) {
      continue;
    }

    // in this sector?
    if (pProfile->sSectorX == sSectorX && pProfile->sSectorY == sSectorY &&
        pProfile->bSectorZ == bSectorZ) {
      // if we haven't talked to him yet, and he's not currently recruired/escorted by player (!)
      if ((pProfile->ubLastDateSpokenTo == 0) &&
          !(pProfile->ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE))) {
        // then this is a guy we need to stop for...
        fFoundSomebody = TRUE;
      } else {
        // already spoke to this guy, don't prompt about this sector again, regardless of status of
        // other NPCs here (although Hamous wanders around, he never shares the same wilderness
        // sector as other important NPCs)
        return (FALSE);
      }
    }
  }

  return (fFoundSomebody);
}

void HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback(uint8_t ubExitValue) {
  Assert(gpGroupPrompting);

  if ((ubExitValue == MSG_BOX_RETURN_YES) || (ubExitValue == MSG_BOX_RETURN_OK)) {
    // NPCs now checked, continue moving if appropriate
    PlayerGroupArrivedSafelyInSector(gpGroupPrompting, FALSE);
  } else if (ubExitValue == MSG_BOX_RETURN_NO) {
    // stop here

    // clear their strategic movement (mercpaths and waypoints)
    ClearMercPathsAndWaypointsForAllInGroup(gpGroupPrompting);

    //		// if currently selected sector has nobody in it
    //		if ( PlayerMercsInSector( ( uint8_t ) sSelMapX, ( uint8_t ) sSelMapY, ( uint8_t )
    // iCurrentMapSectorZ ) == 0 )
    // New: ALWAYS make this sector strategically selected, even if there were mercs in the
    // previously selected one
    {
      ChangeSelectedMapSector(gpGroupPrompting->ubSectorX, gpGroupPrompting->ubSectorY,
                              gpGroupPrompting->ubSectorZ);
    }

    StopTimeCompression();
  }

  gpGroupPrompting = NULL;

  MarkForRedrawalStrategicMap();
  fMapScreenBottomDirty = TRUE;

  return;
}

BOOLEAN DoesPlayerExistInPGroup(uint8_t ubGroupID, struct SOLDIERTYPE *pSoldier) {
  struct GROUP *pGroup;
  PLAYERGROUP *curr;

  pGroup = GetGroup(ubGroupID);
  Assert(pGroup);

  curr = pGroup->pPlayerList;

  if (!curr) {
    return FALSE;
  }

  while (curr) {  // definately more than one node

    if (curr->pSoldier == pSoldier) {
      return TRUE;
    }

    curr = curr->next;
  }

  // !curr
  return FALSE;
}

BOOLEAN GroupHasInTransitDeadOrPOWMercs(struct GROUP *pGroup) {
  PLAYERGROUP *pPlayer;

  pPlayer = pGroup->pPlayerList;
  while (pPlayer) {
    if (pPlayer->pSoldier) {
      if ((GetSolAssignment(pPlayer->pSoldier) == IN_TRANSIT) ||
          (GetSolAssignment(pPlayer->pSoldier) == ASSIGNMENT_POW) ||
          (GetSolAssignment(pPlayer->pSoldier) == ASSIGNMENT_DEAD)) {
        // yup!
        return (TRUE);
      }
    }

    pPlayer = pPlayer->next;
  }

  // nope
  return (FALSE);
}

uint8_t NumberMercsInVehicleGroup(struct GROUP *pGroup) {
  int32_t iVehicleID;
  iVehicleID = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
  Assert(iVehicleID != -1);
  if (iVehicleID != -1) {
    return (uint8_t)GetNumberInVehicle(iVehicleID);
  }
  return 0;
}

#ifdef JA2BETAVERSION
void ValidateGroups(struct GROUP *pGroup) {
  // Do error checking, and report group
  ValidatePlayersAreInOneGroupOnly();
  if (!pGroup->fPlayer && !pGroup->ubGroupSize) {
    // report error
    wchar_t str[512];
    if (pGroup->ubSectorIDOfLastReassignment == 255) {
      swprintf(str, ARR_SIZE(str),
               L"Enemy group found with 0 troops in it.  This is illegal and group will be deleted."
               L"  Group %d in sector %c%d originated from sector %c%d.",
               pGroup->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
               SectorID8_Y(pGroup->ubCreatedSectorID) + 'A' - 1,
               SectorID8_X(pGroup->ubCreatedSectorID));
    } else {
      swprintf(str, ARR_SIZE(str),
               L"Enemy group found with 0 troops in it.  This is illegal and group will be deleted."
               L"  Group %d in sector %c%d originated from sector %c%d and last reassignment "
               L"location was %c%d.",
               pGroup->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
               SectorID8_Y(pGroup->ubCreatedSectorID) + 'A' - 1,
               SectorID8_X(pGroup->ubCreatedSectorID),
               SectorID8_Y(pGroup->ubSectorIDOfLastReassignment) + 'A' - 1,
               SectorID8_X(pGroup->ubSectorIDOfLastReassignment));
    }
    // correct error

    DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
  }
}
#endif
