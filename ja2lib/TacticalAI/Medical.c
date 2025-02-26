// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Items.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierFunctions.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/AIInternals.h"
#include "TileEngine/Buildings.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"

extern BOOLEAN gfAutoBandageFailed;

//
// This file contains code devoted to the player AI-controlled medical system.  Maybe it
// can be used or adapted for the enemies too...
//

#define NOT_GOING_TO_DIE -1
#define NOT_GOING_TO_COLLAPSE -1

// can this grunt be bandaged by a teammate?
BOOLEAN CanCharacterBeAutoBandagedByTeammate(struct SOLDIERTYPE *pSoldier);

// c an this grunt help anyone else out?
BOOLEAN CanCharacterAutoBandageTeammate(struct SOLDIERTYPE *pSoldier);

BOOLEAN FindAutobandageClimbPoint(int16_t sDesiredGridNo, BOOLEAN fClimbUp) {
  // checks for existance of location to climb up to building, not occupied by a medic
  BUILDING *pBuilding;
  uint8_t ubNumClimbSpots;
  uint8_t ubLoop;
  uint8_t ubWhoIsThere;

  pBuilding = FindBuilding(sDesiredGridNo);
  if (!pBuilding) {
    return (FALSE);
  }

  ubNumClimbSpots = pBuilding->ubNumClimbSpots;

  for (ubLoop = 0; ubLoop < ubNumClimbSpots; ubLoop++) {
    ubWhoIsThere = WhoIsThere2(pBuilding->sUpClimbSpots[ubLoop], 1);
    if (ubWhoIsThere != NOBODY && !CanCharacterAutoBandageTeammate(MercPtrs[ubWhoIsThere])) {
      continue;
    }
    ubWhoIsThere = WhoIsThere2(pBuilding->sDownClimbSpots[ubLoop], 0);
    if (ubWhoIsThere != NOBODY && !CanCharacterAutoBandageTeammate(MercPtrs[ubWhoIsThere])) {
      continue;
    }
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN FullPatientCheck(struct SOLDIERTYPE *pPatient) {
  uint8_t cnt;
  struct SOLDIERTYPE *pSoldier;

  if (CanCharacterAutoBandageTeammate(pPatient)) {
    // can bandage self!
    return (TRUE);
  }

  if (pPatient->bLevel != 0) {  // look for a clear spot for jumping up

    // special "closest" search that ignores climb spots IF they are occupied by non-medics
    return (FindAutobandageClimbPoint(pPatient->sGridNo, TRUE));
  } else {
    // run though the list of chars on team
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pSoldier++) {
      // can this character help out?
      if (CanCharacterAutoBandageTeammate(pSoldier) == TRUE) {
        // can this guy path to the patient?
        if (pSoldier->bLevel == 0) {
          // do a regular path check
          if (FindBestPath(pSoldier, pPatient->sGridNo, 0, WALKING, NO_COPYROUTE,
                           PATH_THROUGH_PEOPLE)) {
            return (TRUE);
          }
        } else {
          // if on different levels, assume okay
          return (TRUE);
        }
      }
    }
  }
  return (FALSE);
}

BOOLEAN CanAutoBandage(BOOLEAN fDoFullCheck) {
  // returns false if we should stop being in auto-bandage mode
  uint8_t cnt;
  uint8_t ubMedics = 0, ubPatients = 0;
  struct SOLDIERTYPE *pSoldier;
  static uint8_t ubIDForFullCheck = NOBODY;

  // run though the list of chars on team
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // can this character help out?
    if (CanCharacterAutoBandageTeammate(pSoldier) == TRUE) {
      // yep, up the number of medics in sector
      ubMedics++;
    }
  }

  if (ubMedics == 0) {
    // no one that can help
    return (FALSE);
  }

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // can this character be helped out by a teammate?
    if (CanCharacterBeAutoBandagedByTeammate(pSoldier) == TRUE) {
      // yep, up the number of patients awaiting treatment in sector
      ubPatients++;
      if (fDoFullCheck) {
        if (ubIDForFullCheck == NOBODY) {
          // do this guy NEXT time around
          ubIDForFullCheck = cnt;
        } else if (cnt == ubIDForFullCheck) {
          // test this guy
          if (FullPatientCheck(pSoldier) == FALSE) {
            // shit!
            gfAutoBandageFailed = TRUE;
            return (FALSE);
          }
          // set ID for full check to NOBODY; will be set to someone later in loop, or to
          // the first guy on the next pass
          ubIDForFullCheck = NOBODY;
        }
      }
    }
    // is this guy REACHABLE??
  }

  if (ubPatients == 0) {
    // there is no one to help
    return (FALSE);
  } else {
    // got someone that can help and help wanted
    return (TRUE);
  }
}

BOOLEAN CanCharacterAutoBandageTeammate(struct SOLDIERTYPE *pSoldier)
// can this soldier autobandage others in sector
{
  // if the soldier isn't active or in sector, we have problems..leave
  if (!(IsSolActive(pSoldier)) || !IsSolInSector(pSoldier) ||
      (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (GetSolAssignment(pSoldier) == VEHICLE)) {
    return (FALSE);
  }

  // they must have oklife or more, not be collapsed, have some level of medical competence, and
  // have a med kit of some sort
  if ((pSoldier->bLife >= OKLIFE) && !(pSoldier->bCollapsed) && (pSoldier->bMedical > 0) &&
      (FindObjClass(pSoldier, IC_MEDKIT) != NO_SLOT)) {
    return (TRUE);
  }

  return (FALSE);
}

// can this soldier autobandage others in sector
BOOLEAN CanCharacterBeAutoBandagedByTeammate(struct SOLDIERTYPE *pSoldier) {
  // if the soldier isn't active or in sector, we have problems..leave
  if (!(IsSolActive(pSoldier)) || !IsSolInSector(pSoldier) ||
      (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (GetSolAssignment(pSoldier) == VEHICLE)) {
    return (FALSE);
  }

  if (IsSolAlive(pSoldier) && (pSoldier->bBleeding > 0)) {
    // someone's bleeding and not being given first aid!
    return (TRUE);
  }

  return (FALSE);
}

int8_t FindBestPatient(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfDoClimb) {
  uint8_t cnt, cnt2;
  int16_t bBestPriority = 0, sBestAdjGridNo;
  int16_t sPatientGridNo, sBestPatientGridNo;
  int16_t sShortestPath = 1000, sPathCost, sOtherMedicPathCost;
  struct SOLDIERTYPE *pPatient;
  struct SOLDIERTYPE *pBestPatient = NULL;
  struct SOLDIERTYPE *pOtherMedic;
  int8_t bPatientPriority;
  uint8_t ubDirection;
  int16_t sAdjustedGridNo, sAdjacentGridNo, sOtherAdjacentGridNo;
  int16_t sClimbGridNo, sBestClimbGridNo = NOWHERE, sShortestClimbPath = 1000;
  BOOLEAN fClimbingNecessary;

  gubGlobalPathFlags = PATH_THROUGH_PEOPLE;

  // search for someone who needs aid
  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  for (pPatient = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++, pPatient++) {
    if (!(pPatient->bActive) || !(pPatient->bInSector)) {
      continue;  // NEXT!!!
    }

    if (pPatient->bLife > 0 && pPatient->bBleeding && pPatient->ubServiceCount == 0) {
      if (pPatient->bLife < OKLIFE) {
        bPatientPriority = 3;
      } else if (pPatient->bLife < OKLIFE * 2) {
        bPatientPriority = 2;
      } else {
        bPatientPriority = 1;
      }

      if (bPatientPriority >= bBestPriority) {
        if (!ClimbingNecessary(pSoldier, pPatient->sGridNo, pPatient->bLevel)) {
          sPatientGridNo = pPatient->sGridNo;
          sAdjacentGridNo = FindAdjacentGridEx(pSoldier, sPatientGridNo, &ubDirection,
                                               &sAdjustedGridNo, FALSE, FALSE);
          if (sAdjacentGridNo == -1 &&
              gAnimControl[pPatient->usAnimState].ubEndHeight == ANIM_PRONE) {
            // prone; could be the base tile is inaccessible but the rest isn't...
            for (cnt2 = 0; cnt2 < NUM_WORLD_DIRECTIONS; cnt2++) {
              sPatientGridNo = pPatient->sGridNo + DirectionInc(cnt2);
              if (WhoIsThere2(sPatientGridNo, pPatient->bLevel) == pPatient->ubID) {
                // patient is also here, try this location
                sAdjacentGridNo = FindAdjacentGridEx(pSoldier, sPatientGridNo, &ubDirection,
                                                     &sAdjustedGridNo, FALSE, FALSE);
                if (sAdjacentGridNo != -1) {
                  break;
                }
              }
            }
          }

          if (sAdjacentGridNo != -1) {
            if (sAdjacentGridNo == pSoldier->sGridNo) {
              sPathCost = 1;
            } else {
              sPathCost = PlotPath(pSoldier, sAdjacentGridNo, FALSE, FALSE, FALSE, RUNNING, FALSE,
                                   FALSE, 0);
            }

            if (sPathCost != 0) {
              // we can get there... can anyone else?

              if (pPatient->ubAutoBandagingMedic != NOBODY &&
                  pPatient->ubAutoBandagingMedic != GetSolID(pSoldier)) {
                // only switch to this patient if our distance is closer than
                // the other medic's
                pOtherMedic = MercPtrs[pPatient->ubAutoBandagingMedic];
                sOtherAdjacentGridNo = FindAdjacentGridEx(pOtherMedic, sPatientGridNo, &ubDirection,
                                                          &sAdjustedGridNo, FALSE, FALSE);
                if (sOtherAdjacentGridNo != -1) {
                  if (sOtherAdjacentGridNo == pOtherMedic->sGridNo) {
                    sOtherMedicPathCost = 1;
                  } else {
                    sOtherMedicPathCost = PlotPath(pOtherMedic, sOtherAdjacentGridNo, FALSE, FALSE,
                                                   FALSE, RUNNING, FALSE, FALSE, 0);
                  }

                  if (sPathCost >= sOtherMedicPathCost) {
                    // this patient is best served by the merc moving to them now
                    continue;
                  }
                }
              }

              if (bPatientPriority == bBestPriority) {
                // compare path distances
                if (sPathCost > sShortestPath) {
                  continue;
                }
              }

              sShortestPath = sPathCost;
              pBestPatient = pPatient;
              sBestPatientGridNo = sPatientGridNo;
              bBestPriority = bPatientPriority;
              sBestAdjGridNo = sAdjacentGridNo;
            }
          }

        } else {
          sClimbGridNo = NOWHERE;
          // see if guy on another building etc and we need to climb somewhere
          sPathCost = EstimatePathCostToLocation(pSoldier, pPatient->sGridNo, pPatient->bLevel,
                                                 FALSE, &fClimbingNecessary, &sClimbGridNo);
          // if we can get there
          if (sPathCost != 0 && fClimbingNecessary && sPathCost < sShortestClimbPath) {
            sBestClimbGridNo = sClimbGridNo;
            sShortestClimbPath = sPathCost;
          }
        }
      }
    }
  }

  gubGlobalPathFlags = 0;

  if (pBestPatient) {
    if (pBestPatient->ubAutoBandagingMedic != NOBODY) {
      // cancel that medic
      CancelAIAction(MercPtrs[pBestPatient->ubAutoBandagingMedic], TRUE);
    }
    pBestPatient->ubAutoBandagingMedic = GetSolID(pSoldier);
    *pfDoClimb = FALSE;
    if (CardinalSpacesAway(pSoldier->sGridNo, sBestPatientGridNo) == 1) {
      pSoldier->usActionData = sBestPatientGridNo;
      return (AI_ACTION_GIVE_AID);
    } else {
      pSoldier->usActionData = sBestAdjGridNo;
      return (AI_ACTION_GET_CLOSER);
    }
  } else if (sBestClimbGridNo != NOWHERE) {
    *pfDoClimb = TRUE;
    pSoldier->usActionData = sBestClimbGridNo;
    return (AI_ACTION_MOVE_TO_CLIMB);
  } else {
    return (AI_ACTION_NONE);
  }
}

int8_t DecideAutoBandage(struct SOLDIERTYPE *pSoldier) {
  int8_t bSlot;
  BOOLEAN fDoClimb;

  if (pSoldier->bMedical == 0 || pSoldier->ubServicePartner != NOBODY) {
    // don't/can't make decision
    return (AI_ACTION_NONE);
  }

  bSlot = FindObjClass(pSoldier, IC_MEDKIT);
  if (bSlot == NO_SLOT) {
    // no medical kit!
    return (AI_ACTION_NONE);
  }

  if (pSoldier->bBleeding) {
    // heal self first!
    pSoldier->usActionData = pSoldier->sGridNo;
    if (bSlot != HANDPOS) {
      pSoldier->bSlotItemTakenFrom = bSlot;

      SwapObjs(&(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bSlot]));
      /*
      memset( &TempObj, 0, sizeof( struct OBJECTTYPE ) );
      // move the med kit out to temp obj
      SwapObjs( &TempObj, &(pSoldier->inv[bSlot]) );
      // swap the med kit with whatever was in the hand
      SwapObjs( &TempObj, &(pSoldier->inv[HANDPOS]) );
      // replace whatever was in the hand somewhere in inventory
      AutoPlaceObject( pSoldier, &TempObj, FALSE );
      */
    }
    return (AI_ACTION_GIVE_AID);
  }

  //	pSoldier->usActionData = FindClosestPatient( pSoldier );
  pSoldier->bAction = FindBestPatient(pSoldier, &fDoClimb);
  if (pSoldier->bAction != AI_ACTION_NONE) {
    pSoldier->usUIMovementMode = RUNNING;
    if (bSlot != HANDPOS) {
      pSoldier->bSlotItemTakenFrom = bSlot;

      SwapObjs(&(pSoldier->inv[HANDPOS]), &(pSoldier->inv[bSlot]));
    }
    return (pSoldier->bAction);
  }

  // do nothing
  return (AI_ACTION_NONE);
}
