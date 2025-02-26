// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Soldier.h"
#include "Strategic/MapScreen.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicPathing.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/AIInternals.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/Smell.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"

// from strategic
extern int16_t DirXIncrementer[8];
extern int16_t DirYIncrementer[8];
//
// CJC's DG->JA2 conversion notes
//
// LegalNPCDestination - mode hardcoded to walking; C.O. tear gas related stuff commented out
// TryToResumeMovement - C.O. EscortedMoveCanceled call
// GoAsFarAsPossibleTowards - C.O. stuff related to current animation esp first aid
// SetCivilianDestination - C.O. stuff for if we don't control the civ

int LegalNPCDestination(struct SOLDIERTYPE *pSoldier, int16_t sGridno, uint8_t ubPathMode,
                        uint8_t ubWaterOK, uint8_t fFlags) {
  BOOLEAN fSkipTilesWithMercs;

  if ((sGridno < 0) || (sGridno >= GRIDSIZE)) {
#ifdef RECORDNET
    fprintf(NetDebugFile, "LegalNPC->sDestination: ERROR - rcvd invalid gridno %d", gridno);
#endif

#ifdef BETAVERSION
    NumMessage("LegalNPC->sDestination: ERROR - rcvd invalid gridno ", gridno);
#endif

    return (FALSE);
  }

  // return false if gridno on different level from merc
  if (GridNoOnVisibleWorldTile(pSoldier->sGridNo) &&
      gpWorldLevelData[pSoldier->sGridNo].sHeight != gpWorldLevelData[sGridno].sHeight) {
    return (FALSE);
  }

  // skip mercs if turnbased and adjacent AND not doing an IGNORE_PATH check (which is used almost
  // exclusively by GoAsFarAsPossibleTowards)
  fSkipTilesWithMercs =
      (gfTurnBasedAI && ubPathMode != IGNORE_PATH && SpacesAway(pSoldier->sGridNo, sGridno) == 1);

  // if this gridno is an OK destination
  // AND the gridno is NOT in a tear-gassed tile when we have no gas mask
  // AND someone is NOT already standing there
  // AND we're NOT already standing at that gridno
  // AND the gridno hasn't been black-listed for us

  // Nov 28 98: skip people in destination tile if in turnbased
  if ((NewOKDestination(pSoldier, sGridno, fSkipTilesWithMercs, pSoldier->bLevel)) &&
      (!InGas(pSoldier, sGridno)) && (sGridno != pSoldier->sGridNo) &&
      (sGridno != pSoldier->sBlackList)) {
    // if water's a problem, and gridno is in a water tile (bridges are OK)
    if (!ubWaterOK && Water(sGridno)) return (FALSE);

    // passed all checks, now try to make sure we can get there!
    switch (ubPathMode) {
      // if finding a path wasn't asked for (could have already been done,
      // for example), don't bother
      case IGNORE_PATH:
        return (TRUE);

      case ENSURE_PATH:
        if (FindBestPath(pSoldier, sGridno, pSoldier->bLevel, WALKING, COPYROUTE, fFlags)) {
          return (TRUE);  // legal destination
        } else            // got this far, but found no clear path,
        {
          // so test fails
          return (FALSE);
        }
        // *** NOTE: movement mode hardcoded to WALKING !!!!!
      case ENSURE_PATH_COST:
        return (PlotPath(pSoldier, sGridno, FALSE, FALSE, FALSE, WALKING, FALSE, FALSE, 0));

      default:
#ifdef BETAVERSION
        NumMessage("LegalNPC->sDestination: ERROR - illegal pathMode = ", ubPathMode);
#endif
        return (FALSE);
    }
  } else             // something failed - didn't even have to test path
    return (FALSE);  // illegal destination
}

int TryToResumeMovement(struct SOLDIERTYPE *pSoldier, int16_t sGridno) {
  uint8_t ubGottaCancel = FALSE;
  uint8_t ubSuccess = FALSE;

  // have to make sure the old destination is still legal (somebody may
  // have occupied the destination gridno in the meantime!)
  if (LegalNPCDestination(pSoldier, sGridno, ENSURE_PATH, WATEROK, 0)) {
#ifdef DEBUGDECISIONS
    DebugAI(String("%d CONTINUES MOVEMENT to gridno %d...\n", GetSolID(pSoldier), gridno));
#endif

    pSoldier->bPathStored = TRUE;  // optimization - Ian

    // make him go to it (needed to continue movement across multiple turns)
    NewDest(pSoldier, sGridno);

    ubSuccess = TRUE;

    // make sure that it worked (check that pSoldier->sDestination == pSoldier->sGridNo)
    if (pSoldier->sDestination == sGridno) {
      ubSuccess = TRUE;
    } else {
#ifdef BETAVERSION
      sprintf(tempstr, "TryToResumeMovement: ERROR - NewDest failed for %s, action CANCELED",
              pSoldier->name);

#ifdef RECORDNET
      fprintf(NetDebugFile, "\n\t%s\n", tempstr);
#endif

      PopMessage(tempstr);
      SaveGame(ERROR_SAVE);
#endif

      // must work even for escorted civs, can't just set the flag
      CancelAIAction(pSoldier, FORCE);
    }

  } else {
    // don't black-list anything here, this situation can come up quite
    // legally if another soldier gets in the way between turns

#ifdef BETAVERSION
    sprintf(tempstr, "TryToResumeMovement: %d can't continue to gridno %d, no longer legal!",
            GetSolID(pSoldier), gridno);

#ifdef RECORDNET
    fprintf(NetDebugFile, "\n\t%s\n", tempstr);
#endif

#ifdef DEBUGDECISIONS
    AIPopMessage(tempstr);
#endif

#endif

    if (!pSoldier->bUnderEscort) {
      CancelAIAction(pSoldier, DONTFORCE);  // no need to force this
    } else {
      // this is an escorted NPC, don't want to just completely stop
      // moving, try to find a nearby "next best" destination if possible
      pSoldier->usActionData = GoAsFarAsPossibleTowards(pSoldier, sGridno, pSoldier->bAction);

      // if it's not possible to get any closer
      if (pSoldier->usActionData == NOWHERE) {
        ubGottaCancel = TRUE;
      } else {
        // change his desired destination to this new one
        sGridno = pSoldier->usActionData;

        // GoAsFar... sets pathStored TRUE only if he could go all the way

        // make him go to it (needed to continue movement across multiple turns)
        NewDest(pSoldier, sGridno);

        // make sure that it worked (check that pSoldier->sDestination == pSoldier->sGridNo)
        if (pSoldier->sDestination == sGridno)
          ubSuccess = TRUE;
        else
          ubGottaCancel = TRUE;
      }

      if (ubGottaCancel) {
        // can't get close, gotta abort the movement!
        CancelAIAction(pSoldier, FORCE);

        // tell the player doing the escorting that civilian has stopped
        // EscortedMoveCanceled(pSoldier,COMMUNICATE);
      }
    }
  }

  return (ubSuccess);
}

int16_t NextPatrolPoint(struct SOLDIERTYPE *pSoldier) {
  // patrol slot 0 is UNUSED, so max patrolCnt is actually only 9
  if ((pSoldier->bPatrolCnt < 1) || (pSoldier->bPatrolCnt >= MAXPATROLGRIDS)) {
#ifdef BETAVERSION
    sprintf(tempstr, "NextPatrolPoint: ERROR: Invalid patrol count = %d for %s",
            pSoldier->bPatrolCnt, pSoldier->name);
    PopMessage(tempstr);
#endif

    return (NOWHERE);
  }

  pSoldier->bNextPatrolPnt++;

  // if there are no more patrol points, return back to the first one
  if (pSoldier->bNextPatrolPnt > pSoldier->bPatrolCnt)
    pSoldier->bNextPatrolPnt = 1;  // ZERO is not used!

  return (pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt]);
}

int8_t PointPatrolAI(struct SOLDIERTYPE *pSoldier) {
  int16_t sPatrolPoint;
  int8_t bOldOrders;

  sPatrolPoint = pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt];

  // if we're already there, advance next patrol point
  if (pSoldier->sGridNo == sPatrolPoint || pSoldier->bNextPatrolPnt == 0) {
    // find next valid patrol point
    do {
      sPatrolPoint = NextPatrolPoint(pSoldier);
    } while ((sPatrolPoint != NOWHERE) &&
             (NewOKDestination(pSoldier, sPatrolPoint, IGNOREPEOPLE, pSoldier->bLevel) < 1));

    // if we're back where we started, then ALL other patrol points are junk!
    if (pSoldier->sGridNo == sPatrolPoint) {
#ifdef BETAVERSION
      NumMessage("PROBLEM WITH SCENARIO: All other patrol points are invalid for guynum ",
                 GetSolID(pSoldier));
#endif
      // force change of orders & an abort
      sPatrolPoint = NOWHERE;
    }
  }

  // if we don't have a legal patrol point
  if (sPatrolPoint == NOWHERE) {
#ifdef BETAVERSION
    NumMessage("PointPatrolAI: ERROR - no legal patrol point for %d", GetSolID(pSoldier));
#endif

    // over-ride orders to something safer
    pSoldier->bOrders = FARPATROL;
    return (FALSE);
  }

  // make sure we can get there from here at this time, if we can't get all
  // the way there, at least do our best to get close
  if (LegalNPCDestination(pSoldier, sPatrolPoint, ENSURE_PATH, WATEROK, 0)) {
    pSoldier->bPathStored = TRUE;  // optimization - Ian
    pSoldier->usActionData = sPatrolPoint;
  } else {
    // temporarily extend roaming range to infinity by changing orders, else
    // this won't work if the next patrol point is > 10 tiles away!
    bOldOrders = pSoldier->bOrders;
    pSoldier->bOrders = ONCALL;

    pSoldier->usActionData = GoAsFarAsPossibleTowards(pSoldier, sPatrolPoint, pSoldier->bAction);

    pSoldier->bOrders = bOldOrders;

    // if it's not possible to get any closer, that's OK, but fail this call
    if (pSoldier->usActionData == NOWHERE) return (FALSE);
  }

  // passed all tests - start moving towards next patrol point
#ifdef DEBUGDECISIONS
  sprintf(tempstr, "%s - POINT PATROL to grid %d", pSoldier->name, pSoldier->usActionData);
  AIPopMessage(tempstr);
#endif

  return (TRUE);
}

int8_t RandomPointPatrolAI(struct SOLDIERTYPE *pSoldier) {
  int16_t sPatrolPoint;
  int8_t bOldOrders, bPatrolIndex;
  int8_t bCnt;

  sPatrolPoint = pSoldier->usPatrolGrid[pSoldier->bNextPatrolPnt];

  // if we're already there, advance next patrol point
  if (pSoldier->sGridNo == sPatrolPoint || pSoldier->bNextPatrolPnt == 0) {
    // find next valid patrol point
    // we keep a count of the # of times we are in here to make sure we don't get into an endless
    // loop
    bCnt = 0;
    do {
      // usPatrolGrid[0] gets used for centre of close etc patrols, so we have to add 1 to the
      // Random #
      bPatrolIndex = (int8_t)PreRandom(pSoldier->bPatrolCnt) + 1;
      sPatrolPoint = pSoldier->usPatrolGrid[bPatrolIndex];
      bCnt++;
    } while ((sPatrolPoint == pSoldier->sGridNo) ||
             ((sPatrolPoint != NOWHERE) && (bCnt < pSoldier->bPatrolCnt) &&
              (NewOKDestination(pSoldier, sPatrolPoint, IGNOREPEOPLE, pSoldier->bLevel) < 1)));

    if (bCnt == pSoldier->bPatrolCnt) {
      // ok, we tried doing this randomly, didn't work well, so now do a linear search
      pSoldier->bNextPatrolPnt = 0;
      do {
        sPatrolPoint = NextPatrolPoint(pSoldier);
      } while ((sPatrolPoint != NOWHERE) &&
               (NewOKDestination(pSoldier, sPatrolPoint, IGNOREPEOPLE, pSoldier->bLevel) < 1));
    }

    // do nothing this time around
    if (pSoldier->sGridNo == sPatrolPoint) {
      return (FALSE);
    }
  }

  // if we don't have a legal patrol point
  if (sPatrolPoint == NOWHERE) {
#ifdef BETAVERSION
    NumMessage("PointPatrolAI: ERROR - no legal patrol point for %d", GetSolID(pSoldier));
#endif

    // over-ride orders to something safer
    pSoldier->bOrders = FARPATROL;
    return (FALSE);
  }

  // make sure we can get there from here at this time, if we can't get all
  // the way there, at least do our best to get close
  if (LegalNPCDestination(pSoldier, sPatrolPoint, ENSURE_PATH, WATEROK, 0)) {
    pSoldier->bPathStored = TRUE;  // optimization - Ian
    pSoldier->usActionData = sPatrolPoint;
  } else {
    // temporarily extend roaming range to infinity by changing orders, else
    // this won't work if the next patrol point is > 10 tiles away!
    bOldOrders = pSoldier->bOrders;
    pSoldier->bOrders = SEEKENEMY;

    pSoldier->usActionData = GoAsFarAsPossibleTowards(pSoldier, sPatrolPoint, pSoldier->bAction);

    pSoldier->bOrders = bOldOrders;

    // if it's not possible to get any closer, that's OK, but fail this call
    if (pSoldier->usActionData == NOWHERE) return (FALSE);
  }

  // passed all tests - start moving towards next patrol point
#ifdef DEBUGDECISIONS
  sprintf(tempstr, "%s - POINT PATROL to grid %d", pSoldier->name, pSoldier->usActionData);
  AIPopMessage(tempstr);
#endif

  return (TRUE);
}

int16_t InternalGoAsFarAsPossibleTowards(struct SOLDIERTYPE *pSoldier, int16_t sDesGrid,
                                         int8_t bReserveAPs, int8_t bAction, int8_t fFlags) {
  int16_t sLoop, sAPCost;
  int16_t sTempDest, sGoToGrid;
  uint16_t sOrigin;
  uint16_t usMaxDist;
  uint8_t ubDirection, ubDirsLeft, ubDirChecked[8], fFound = FALSE;
  int8_t bAPsLeft, fPathFlags;
  uint8_t ubRoomRequired = 0, ubTempRoom;

  if (bReserveAPs == -1) {
    // default reserve points
    if (CREATURE_OR_BLOODCAT(pSoldier)) {
      bReserveAPs = 0;
    } else {
      bReserveAPs = MAX_AP_CARRIED;
    }
  }

  sTempDest = -1;

  // obtain maximum roaming distance from soldier's sOrigin
  usMaxDist = RoamingRange(pSoldier, &sOrigin);

  if (pSoldier->bOrders <= CLOSEPATROL &&
      (pSoldier->bTeam == CIV_TEAM || GetSolProfile(pSoldier) != NO_PROFILE)) {
    if (InARoom(pSoldier->usPatrolGrid[0], &ubRoomRequired)) {
      // make sure this doesn't interfere with pathing for scripts
      if (pSoldier->sAbsoluteFinalDestination != NOWHERE) {
        ubRoomRequired = 0;
      }
    }
  }

  pSoldier->usUIMovementMode = DetermineMovementMode(pSoldier, bAction);
  if (pSoldier->usUIMovementMode == RUNNING && fFlags & FLAG_CAUTIOUS) {
    pSoldier->usUIMovementMode = WALKING;
  }

#ifdef DEBUGDECISIONS
  sprintf(tempstr, "%s wants to go towards %d (has range %d)", pSoldier->name, sDesGrid, usMaxDist);
  AIPopMessage(tempstr);
#endif

  // if soldier is ALREADY at the desired destination, quit right away
  if (sDesGrid == pSoldier->sGridNo) {
    return (NOWHERE);
  }

  // don't try to approach go after noises or enemies actually in water
  // would be too easy to throw rocks in water, etc. & distract the AI
  if (Water(sDesGrid)) {
    return (NOWHERE);
  }

  fPathFlags = 0;
  if (CREATURE_OR_BLOODCAT(pSoldier)) {
    fPathFlags = PATH_CLOSE_GOOD_ENOUGH;
  }

  // first step: try to find an OK destination at or near the desired gridno
  if (!LegalNPCDestination(pSoldier, sDesGrid, ENSURE_PATH, NOWATER, fPathFlags)) {
#ifdef DEBUGDECISIONS
    AIPopMessage("destination Grid # itself not valid, looking around it");
#endif
    if (CREATURE_OR_BLOODCAT(pSoldier)) {
      // we tried to get close, failed; abort!
      return (NOWHERE);
    } else {
      // else look at the 8 nearest gridnos to sDesGrid for a valid destination

      // clear ubDirChecked flag for all 8 directions
      for (ubDirection = 0; ubDirection < 8; ubDirection++) ubDirChecked[ubDirection] = FALSE;

      ubDirsLeft = 8;

      // examine all 8 spots around 'sDesGrid'
      // keep looking while directions remain and a satisfactory one not found
      for (ubDirsLeft = 8; ubDirsLeft != 0; ubDirsLeft--) {
        if (fFound) {
          break;
        }
        // randomly select a direction which hasn't been 'checked' yet
        do {
          ubDirection = (uint8_t)Random(8);
        } while (ubDirChecked[ubDirection]);

        ubDirChecked[ubDirection] = TRUE;

        // determine the gridno 1 tile away from current friend in this direction
        sTempDest = NewGridNo(sDesGrid, DirectionInc((int16_t)(ubDirection + 1)));

        // if that's out of bounds, ignore it & check next direction
        if (sTempDest == sDesGrid) continue;

        if (LegalNPCDestination(pSoldier, sTempDest, ENSURE_PATH, NOWATER, 0)) {
          fFound = TRUE;  // found a spot

#ifdef DEBUGDECISIONS
          AINumMessage("Found a spot!  ubDirection = ", ubDirection + 1);
#endif

          break;  // stop checking in other directions
        }
      }

      if (!fFound) {
#ifdef DEBUGDECISIONS
        AINumMessage("Couldn't find OK destination around grid #", sDesGrid);
#endif

        return (NOWHERE);
      }

      // found a good grid #, this becomes our actual desired grid #
      sDesGrid = sTempDest;
    }
  }

  // HAVE FOUND AN OK destination AND PLOTTED A VALID BEST PATH TO IT

#ifdef DEBUGDECISIONS
  AINumMessage("Chosen legal destination is gridno ", sDesGrid);
  AINumMessage("Tracing along path, pathRouteToGo = ", pSoldier->pathRouteToGo);
#endif

  sGoToGrid = pSoldier->sGridNo;  // start back where soldier is standing now
  sAPCost = 0;                    // initialize path cost counter

  // we'll only go as far along the plotted route as is within our
  // permitted roaming range, and we'll stop as soon as we're down to <= 5 APs

  for (sLoop = 0; sLoop < (pSoldier->usPathDataSize - pSoldier->usPathIndex); sLoop++) {
    // what is the next gridno in the path?

    // sTempDest = NewGridNo( sGoToGrid,DirectionInc( (int16_t) (pSoldier->usPathingData[sLoop] + 1)
    // )
    // );
    sTempDest = NewGridNo(sGoToGrid, DirectionInc((int16_t)(pSoldier->usPathingData[sLoop])));
    // NumMessage("sTempDest = ",sTempDest);

    // this should NEVER be out of bounds
    if (sTempDest == sGoToGrid) {
#ifdef BETAVERSION
      sprintf(tempstr,
              "GoAsFarAsPossibleTowards: ERROR - gridno along valid route is invalid!  guynum %d, "
              "sTempDest = %d",
              GetSolID(pSoldier), sTempDest);

#ifdef RECORDNET
      fprintf(NetDebugFile, "\n\t%s\n", tempstr);
#endif

      PopMessage(tempstr);
      SaveGame(ERROR_SAVE);
#endif

      break;  // quit here, sGoToGrid is where we are going
    }

    // if this takes us beyond our permitted "roaming range"
    if (SpacesAway(sOrigin, sTempDest) > usMaxDist)
      break;  // quit here, sGoToGrid is where we are going

    if (ubRoomRequired) {
      if (!(InARoom(sTempDest, &ubTempRoom) && ubTempRoom == ubRoomRequired)) {
        // quit here, limited by room!
        break;
      }
    }

    if ((fFlags & FLAG_STOPSHORT) && SpacesAway(sDesGrid, sTempDest) <= STOPSHORTDIST) {
      break;  // quit here, sGoToGrid is where we are going
    }

    // if this gridno is NOT a legal NPC destination
    // DONT'T test path again - that would replace the traced path! - Ian
    // NOTE: It's OK to go *THROUGH* water to try and get to the destination!
    if (!LegalNPCDestination(pSoldier, sTempDest, IGNORE_PATH, WATEROK, 0))
      break;  // quit here, sGoToGrid is where we are going

    // CAN'T CALL PathCost() HERE! IT CALLS findBestPath() and overwrites
    //       pathRouteToGo !!!  Gotta calculate the cost ourselves - Ian
    //
    // ubAPsLeft = pSoldier->bActionPoints -
    // PathCost(pSoldier,sTempDest,FALSE,FALSE,FALSE,FALSE,FALSE);

    if (gfTurnBasedAI) {
      // if we're just starting the "costing" process (first gridno)
      if (sLoop == 0) {
        if (pSoldier->usUIMovementMode == RUNNING) {
          sAPCost += AP_START_RUN_COST;
        }
      }

      // ATE: Direction here?
      sAPCost += EstimateActionPointCost(
          pSoldier, sTempDest, (int8_t)pSoldier->usPathingData[sLoop], pSoldier->usUIMovementMode,
          (int8_t)sLoop, (int8_t)pSoldier->usPathDataSize);

      bAPsLeft = pSoldier->bActionPoints - sAPCost;
    }

    // if after this, we have <= 5 APs remaining, that's far enough, break out
    // (the idea is to preserve APs so we can crouch or react if
    // necessary, and benefit from the carry-over next turn if not needed)
    // This routine is NOT used by any GREEN AI, so such caution is warranted!

    if (gfTurnBasedAI && (bAPsLeft < bReserveAPs))
      break;
    else {
      sGoToGrid = sTempDest;  // we're OK up to here

      // if exactly 5 APs left, don't bother checking any further
      if (gfTurnBasedAI && (bAPsLeft == bReserveAPs)) break;
    }
  }

  // if it turned out we couldn't go even 1 tile towards the desired gridno
  if (sGoToGrid == pSoldier->sGridNo) {
#ifdef DEBUGDECISIONS
    sprintf(tempstr, "%s will go NOWHERE, path doesn't meet criteria", pSoldier->name);
    AIPopMessage(tempstr);
#endif

    return (NOWHERE);  // then go nowhere
  } else {
    // possible optimization - stored path IS good if we're going all the way
    if (sGoToGrid == sDesGrid) {
      pSoldier->bPathStored = TRUE;
      pSoldier->sFinalDestination = sGoToGrid;
    } else if (pSoldier->usPathIndex == 0) {
      // we can hack this surely! -- CJC
      pSoldier->bPathStored = TRUE;
      pSoldier->sFinalDestination = sGoToGrid;
      pSoldier->usPathDataSize = sLoop + 1;
    }

#ifdef DEBUGDECISIONS
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"%d to %d with %d APs left",
              GetSolID(pSoldier), sGoToGrid, pSoldier->bActionPoints);
#endif

    return (sGoToGrid);
  }
}

int16_t GoAsFarAsPossibleTowards(struct SOLDIERTYPE *pSoldier, int16_t sDesGrid, int8_t bAction) {
  return (InternalGoAsFarAsPossibleTowards(pSoldier, sDesGrid, -1, bAction, 0));
}

void SoldierTriesToContinueAlongPath(struct SOLDIERTYPE *pSoldier) {
  int16_t usNewGridNo, bAPCost;

  // turn off the flag now that we're going to do something about it...
  // ATE: USed to be redundent, now if called befroe NewDest can cause some side efects...
  // AdjustNoAPToFinishMove( pSoldier, FALSE );

  if (pSoldier->bNewSituation == IS_NEW_SITUATION) {
    CancelAIAction(pSoldier, DONTFORCE);
    return;
  }

  if (pSoldier->usActionData >= NOWHERE) {
    CancelAIAction(pSoldier, DONTFORCE);
    return;
  }

  if (!NewOKDestination(pSoldier, pSoldier->usActionData, TRUE, pSoldier->bLevel)) {
    CancelAIAction(pSoldier, DONTFORCE);
    return;
  }

  if (IsActionAffordable(pSoldier)) {
    if (pSoldier->bActionInProgress == FALSE) {
      // start a move that didn't even get started before...
      // hope this works...
      NPCDoesAct(pSoldier);

      // perform the chosen action
      pSoldier->bActionInProgress = ExecuteAction(pSoldier);  // if started, mark us as busy
    } else {
      // otherwise we shouldn't have to do anything(?)
    }
  } else {
    CancelAIAction(pSoldier, DONTFORCE);
#ifdef TESTAI
    DebugMsg(TOPIC_JA2AI, DBG_LEVEL_3,
             String("Soldier (%d) HAS NOT ENOUGH AP to continue along path", GetSolID(pSoldier)));
#endif
  }

  usNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo,
                          DirectionInc((uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex]));

  // Find out how much it takes to move here!
  bAPCost = EstimateActionPointCost(
      pSoldier, usNewGridNo, (int8_t)pSoldier->usPathingData[pSoldier->usPathIndex],
      pSoldier->usUIMovementMode, (int8_t)pSoldier->usPathIndex, (int8_t)pSoldier->usPathDataSize);

  if (pSoldier->bActionPoints >= bAPCost) {
    // seems to have enough points...
    NewDest(pSoldier, usNewGridNo);
    // maybe we didn't actually start the action last turn...
    pSoldier->bActionInProgress = TRUE;
#ifdef TESTAI
    DebugMsg(TOPIC_JA2AI, DBG_LEVEL_3,
             String("Soldier (%d) continues along path", GetSolID(pSoldier)));
#endif
  } else {
    CancelAIAction(pSoldier, DONTFORCE);
#ifdef TESTAI
    DebugMsg(TOPIC_JA2AI, DBG_LEVEL_3,
             String("Soldier (%d) HAS NOT ENOUGH AP to continue along path", GetSolID(pSoldier)));
#endif
  }
}

void HaltMoveForSoldierOutOfPoints(struct SOLDIERTYPE *pSoldier) {
  // If a special move, ignore this!
  if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE)) {
    return;
  }

  // record that this merc can no longer animate and why...
  AdjustNoAPToFinishMove(pSoldier, TRUE);

  // We'll keep his action intact though...
  DebugAI(String("NO AP TO FINISH MOVE for %d (%d APs left)", GetSolID(pSoldier),
                 pSoldier->bActionPoints));

  // if this dude is under AI right now, then pass the baton to someone else
  if (pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL) {
#ifdef TESTAICONTROL
    DebugAI(String("Ending turn for %d because out of APs for movement", GetSolID(pSoldier)));
#endif

    EndAIGuysTurn(pSoldier);
  }
}

void SetCivilianDestination(uint8_t ubWho, int16_t sGridno) {
  struct SOLDIERTYPE *pSoldier;

  pSoldier = MercPtrs[ubWho];

  /*
   // if we control the civilian
   if (PTR_OURCONTROL)
    {
  */
  // if the destination is different from what he has now
  if (pSoldier->usActionData != sGridno) {
    // store his new destination
    pSoldier->usActionData = sGridno;

    // and cancel any movement in progress that he was still engaged in
    pSoldier->bAction = AI_ACTION_NONE;
    pSoldier->bActionInProgress = FALSE;
  }

  // only set the underEscort flag once you give him a destination
  // (that way AI can keep him appearing to act on his own until you
  // give him orders).
  //
  // Either way, once set, it should stay that way, preventing AI from
  // doing anything other than advance him towards destination.
  pSoldier->bUnderEscort = TRUE;

  // change orders to maximize roaming range so he can Go As Far As Possible
  pSoldier->bOrders = ONCALL;
  /*
    }

   else
    {
     NetSend.msgType = NET_CIV_DEST;
     NetSend.ubID  = GetSolID(pSoldier);
     NetSend.gridno  = gridno;

     // only the civilian's controller needs to know this
     SendNetData(pSoldier->controller);
    }
  */
}

#define RADIUS 3

int16_t TrackScent(struct SOLDIERTYPE *pSoldier) {
  // This function returns the best gridno to go to based on the scent being followed,
  // and the soldier (creature/animal)'s current direction (which is used to resolve
  // ties.
  int32_t iXDiff, iYDiff, iXIncr;
  int32_t iStart, iXStart, iYStart;
  int32_t iGridNo;
  int8_t bDir;
  int32_t iBestGridNo = NOWHERE;
  uint8_t ubBestDirDiff = 5, ubBestStrength = 0;
  uint8_t ubDirDiff, ubStrength;
  uint8_t ubSoughtSmell;
  MAP_ELEMENT *pMapElement;

  iStart = pSoldier->sGridNo;
  iXStart = iStart % WORLD_COLS;
  iYStart = iStart / WORLD_COLS;

  if (CREATURE_OR_BLOODCAT(pSoldier))  // or bloodcats
  {
    // tracking humans; search the edges of a 7x7 square for the
    // most promising tile
    ubSoughtSmell = HUMAN;
    for (iYDiff = -RADIUS; iYDiff < (RADIUS + 1); iYDiff++) {
      if (iYStart + iYDiff < 0) {
        // outside of map! might be on map further down...
        continue;
      } else if (iYStart + iYDiff > WORLD_ROWS) {
        // outside of bottom of map! abort!
        break;
      }
      if (iYDiff == -RADIUS || iYDiff == RADIUS) {
        iXIncr = 1;
      } else {
        // skip over the spots in the centre of the square
        iXIncr = RADIUS * 2;
      }
      for (iXDiff = -RADIUS; iXDiff < (RADIUS + 1); iXDiff += iXIncr) {
        iGridNo = iStart + iXDiff + iYDiff * WORLD_ROWS;
        if (abs(iGridNo % WORLD_ROWS - iXStart) > RADIUS) {
          // wrapped across map!
          continue;
        }
        if (LegalNPCDestination(pSoldier, pSoldier->usActionData, ENSURE_PATH, WATEROK, 0)) {
          // check this location out
          pMapElement = &(gpWorldLevelData[iGridNo]);
          if (pMapElement->ubSmellInfo && (SMELL_TYPE(pMapElement->ubSmellInfo) == ubSoughtSmell)) {
            ubStrength = SMELL_STRENGTH(pMapElement->ubSmellInfo);
            if (ubStrength > ubBestStrength) {
              iBestGridNo = iGridNo;
              ubBestStrength = ubStrength;
              bDir = atan8((int16_t)iXStart, (int16_t)iYStart, (int16_t)(iXStart + iXDiff),
                           (int16_t)(iYStart + iYDiff));
              // now convert it into a difference in degree between it and our current dir
              ubBestDirDiff = abs(pSoldier->bDirection - bDir);
              if (ubBestDirDiff > 4)  // dir 0 compared with dir 6, for instance
              {
                ubBestDirDiff = 8 - ubBestDirDiff;
              }
            } else if (ubStrength == ubBestStrength) {
              if (iBestGridNo == NOWHERE) {
                // first place we've found with the same strength
                iBestGridNo = iGridNo;
                ubBestStrength = ubStrength;
              } else {
                // use directions to decide between the two
                // start by calculating direction to the new gridno
                bDir = atan8((int16_t)iXStart, (int16_t)iYStart, (int16_t)(iXStart + iXDiff),
                             (int16_t)(iYStart + iYDiff));
                // now convert it into a difference in degree between it and our current dir
                ubDirDiff = abs(pSoldier->bDirection - bDir);
                if (ubDirDiff > 4)  // dir 0 compared with dir 6, for instance
                {
                  ubDirDiff = 8 - ubDirDiff;
                }
                if (ubDirDiff < ubBestDirDiff || ((ubDirDiff == ubBestDirDiff) && Random(2))) {
                  // follow this trail as its closer to the one we're following!
                  // (in the case of a tie, we tossed a coin)
                  ubBestDirDiff = ubDirDiff;
                }
              }
            }
          }
        }
      }
      // go on to next tile
    }
    // go on to next row
  } else {
    // who else can track?
  }
  if (iBestGridNo != NOWHERE) {
    pSoldier->usActionData = (int16_t)iBestGridNo;
    return ((int16_t)iBestGridNo);
  }
  return (0);
}
