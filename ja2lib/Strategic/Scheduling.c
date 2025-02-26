// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/Scheduling.h"

#include <memory.h>

#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/Keys.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/StructureWrap.h"
#include "TacticalAI/AI.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"

#ifdef JA2EDITOR
extern wchar_t gszScheduleActions[NUM_SCHEDULE_ACTIONS][20];
#endif

BOOLEAN GetEarliestMorningScheduleEvent(SCHEDULENODE *pSchedule, uint32_t *puiTime);
int8_t GetEmptyScheduleEntry(SCHEDULENODE *pSchedule);
BOOLEAN ScheduleHasMorningNonSleepEntries(SCHEDULENODE *pSchedule);

#define FOURPM 960

// waketime is the # of minutes in the day minus the sleep time
#define WAKETIME(x) (NUM_SEC_IN_DAY / NUM_SEC_IN_MIN - x)

// #define DISABLESCHEDULES

SCHEDULENODE *gpScheduleList = NULL;
uint8_t gubScheduleID = 0;
void ReverseSchedules();

void PrepareScheduleForAutoProcessing(SCHEDULENODE *pSchedule, uint32_t uiStartTime,
                                      uint32_t uiEndTime);

// IMPORTANT:
// This function adds a NEWLY allocated schedule to the list.  The pointer passed is totally
// separate.  So make sure that you delete the pointer if you don't need it anymore.  The editor
// uses a single static node to copy data from, hence this method.
void CopyScheduleToList(SCHEDULENODE *pSchedule, SOLDIERINITNODE *pNode) {
  SCHEDULENODE *curr;
  curr = gpScheduleList;
  gpScheduleList = (SCHEDULENODE *)MemAlloc(sizeof(SCHEDULENODE));
  memcpy(gpScheduleList, pSchedule, sizeof(SCHEDULENODE));
  gpScheduleList->next = curr;
  gubScheduleID++;
  // Assign all of the links
  gpScheduleList->ubScheduleID = gubScheduleID;
  gpScheduleList->ubSoldierID = pNode->pSoldier->ubID;
  pNode->pDetailedPlacement->ubScheduleID = gubScheduleID;
  pNode->pSoldier->ubScheduleID = gubScheduleID;
  if (gubScheduleID > 40) {  // Too much fragmentation, clean it up...
    OptimizeSchedules();
    if (gubScheduleID > 32) {
      AssertMsg(0, "TOO MANY SCHEDULES POSTED!!!");
    }
  }
}

SCHEDULENODE *GetSchedule(uint8_t ubScheduleID) {
  SCHEDULENODE *curr;
  curr = gpScheduleList;
  while (curr) {
    if (curr->ubScheduleID == ubScheduleID) return curr;
    curr = curr->next;
  }
  return NULL;
}

// Removes all schedules from the event list, and cleans out the list.
void DestroyAllSchedules() {
  SCHEDULENODE *curr;
  // First remove all of the events.
  DeleteAllStrategicEventsOfType(EVENT_PROCESS_TACTICAL_SCHEDULE);
  // Now, delete all of the schedules.
  while (gpScheduleList) {
    curr = gpScheduleList;
    gpScheduleList = gpScheduleList->next;
    MemFree(curr);
  }
  gpScheduleList = NULL;
  gubScheduleID = 0;
}

// cleans out the schedule list without touching events, for saving & loading games
void DestroyAllSchedulesWithoutDestroyingEvents() {
  SCHEDULENODE *curr;

  // delete all of the schedules.
  while (gpScheduleList) {
    curr = gpScheduleList;
    gpScheduleList = gpScheduleList->next;
    MemFree(curr);
  }
  gpScheduleList = NULL;
  gubScheduleID = 0;
}

void DeleteSchedule(uint8_t ubScheduleID) {
  SCHEDULENODE *curr, *temp = NULL;

  if (!gpScheduleList) {
    // ScreenMsg( 0, MSG_BETAVERSION, L"Attempting to delete schedule that doesn't exist -- KM : 2"
    // );
    return;
  }

  curr = gpScheduleList;

  if (gpScheduleList->ubScheduleID == ubScheduleID) {  // Deleting the head
    temp = gpScheduleList;
    gpScheduleList = gpScheduleList->next;
  } else
    while (curr->next) {
      if (curr->next->ubScheduleID == ubScheduleID) {
        temp = curr->next;
        curr->next = temp->next;
        break;
      }
      curr = curr->next;
    }
  if (temp) {
    DeleteStrategicEvent(EVENT_PROCESS_TACTICAL_SCHEDULE, temp->ubScheduleID);
    MemFree(temp);
  }
}

void ProcessTacticalSchedule(uint8_t ubScheduleID) {
  SCHEDULENODE *pSchedule;
  struct SOLDIERTYPE *pSoldier;
  int32_t iScheduleIndex = 0;
  BOOLEAN fAutoProcess;

  // Attempt to locate the schedule.
  pSchedule = GetSchedule(ubScheduleID);
  if (!pSchedule) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Schedule callback:  Schedule ID of %d not found.",
              ubScheduleID);
#endif
    return;
  }
  // Attempt to access the soldier involved
  if (pSchedule->ubSoldierID >= TOTAL_SOLDIERS) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Schedule callback:  Illegal soldier ID of %d.",
              pSchedule->ubSoldierID);
#endif
    return;
  }

  // Validate the existance of the soldier.
  pSoldier = MercPtrs[pSchedule->ubSoldierID];
  if (pSoldier->bLife < OKLIFE) {
    // dead or dying!
    return;
  }

  if (!IsSolActive(pSoldier)) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Schedule callback:  Soldier isn't active.  Name is %s.",
              pSoldier->name);
#endif
  }

  // Okay, now we have good pointers to the soldier and the schedule.
  // Now, determine which time in this schedule that we are processing.
  fAutoProcess = FALSE;
  if (guiCurrentScreen != GAME_SCREEN) {
#ifdef JA2TESTVERSION
    // ScreenMsg( FONT_RED, MSG_TESTVERSION, L"Schedule callback occurred outside of tactical --
    // Auto processing!" );
#endif
    fAutoProcess = TRUE;
  } else {
    for (iScheduleIndex = 0; iScheduleIndex < MAX_SCHEDULE_ACTIONS; iScheduleIndex++) {
      if (pSchedule->usTime[iScheduleIndex] == GetWorldMinutesInDay()) {
#ifdef JA2TESTVERSION
        // ScreenMsg( FONT_RED, MSG_TESTVERSION, L"Processing schedule on time -- AI processing!" );
#endif
        break;
      }
    }
    if (iScheduleIndex == MAX_SCHEDULE_ACTIONS) {
      fAutoProcess = TRUE;
#ifdef JA2TESTVERSION
      // ScreenMsg( FONT_RED, MSG_TESTVERSION, L"Possible timewarp causing schedule callback to
      // occur late -- Auto processing!" );
#endif
    }
  }
  if (fAutoProcess) {
    uint32_t uiStartTime, uiEndTime;
    // Grab the last time the eventlist was queued.  This will tell us how much time has passed
    // since that moment, and how long we need to auto process this schedule.
    uiStartTime = (guiTimeOfLastEventQuery / 60) % NUM_MIN_IN_DAY;
    uiEndTime = GetWorldMinutesInDay();
    if (uiStartTime != uiEndTime) {
      PrepareScheduleForAutoProcessing(pSchedule, uiStartTime, uiEndTime);
    }
  } else {
    // turn off all active-schedule flags before setting
    // the one that should be active!
    pSchedule->usFlags &= ~SCHEDULE_FLAGS_ACTIVE_ALL;

    switch (iScheduleIndex) {
      case 0:
        pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE1;
        break;
      case 1:
        pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE2;
        break;
      case 2:
        pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE3;
        break;
      case 3:
        pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE4;
        break;
    }
    pSoldier->fAIFlags |= AI_CHECK_SCHEDULE;
    pSoldier->bAIScheduleProgress = 0;
  }
}

// Called before leaving the editor, or saving the map.  This recalculates
// all of the schedule IDs from scratch and adjusts the effected structures accordingly.
void OptimizeSchedules() {
  SCHEDULENODE *pSchedule;
  SOLDIERINITNODE *pNode;
  uint8_t ubOldScheduleID;
  gubScheduleID = 0;
  pSchedule = gpScheduleList;
  while (pSchedule) {
    gubScheduleID++;
    ubOldScheduleID = pSchedule->ubScheduleID;
    if (ubOldScheduleID !=
        gubScheduleID) {  // The schedule ID has changed, so change all links accordingly.
      pSchedule->ubScheduleID = gubScheduleID;
      pNode = gSoldierInitHead;
      while (pNode) {
        if (pNode->pDetailedPlacement &&
            pNode->pDetailedPlacement->ubScheduleID == ubOldScheduleID) {
          // Temporarily add 100 to the ID number to ensure that it doesn't get used again later.
          // We will remove it immediately after this loop is complete.
          pNode->pDetailedPlacement->ubScheduleID = gubScheduleID + 100;
          if (pNode->pSoldier) {
            pNode->pSoldier->ubScheduleID = gubScheduleID;
          }
          break;
        }
        pNode = pNode->next;
      }
    }
    pSchedule = pSchedule->next;
  }
  // Remove the +100 IDs.
  pNode = gSoldierInitHead;
  while (pNode) {
    if (pNode->pDetailedPlacement && pNode->pDetailedPlacement->ubScheduleID > 100) {
      pNode->pDetailedPlacement->ubScheduleID -= 100;
    }
    pNode = pNode->next;
  }
}

// Called when transferring from the game to the editor.
void PrepareSchedulesForEditorEntry() {
  SCHEDULENODE *curr, *prev, *temp;

  // Delete all schedule events.  The editor will automatically warp all civilians to their starting
  // locations.
  DeleteAllStrategicEventsOfType(EVENT_PROCESS_TACTICAL_SCHEDULE);

  // Now, delete all of the temporary schedules.
  curr = gpScheduleList;
  prev = NULL;
  while (curr) {
    if (curr->usFlags & SCHEDULE_FLAGS_TEMPORARY) {
      if (prev)
        prev->next = curr->next;
      else
        gpScheduleList = gpScheduleList->next;
      MercPtrs[curr->ubSoldierID]->ubScheduleID = 0;
      temp = curr;
      curr = curr->next;
      MemFree(temp);
      gubScheduleID--;
    } else {
      if (curr->usFlags & SCHEDULE_FLAGS_SLEEP_CONVERTED) {  // uncovert it!
        int32_t i;
        for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
          // if( i
        }
      }
      prev = curr;
      curr = curr->next;
    }
  }
}

// Called when leaving the editor to enter the game.  This posts all of the events that apply.
void PrepareSchedulesForEditorExit() { PostSchedules(); }

void LoadSchedules(int8_t **hBuffer) {
  SCHEDULENODE *pSchedule = NULL;
  SCHEDULENODE temp;
  uint8_t ubNum;

  // delete all the schedules we might have loaded (though we shouldn't have any loaded!!)
  if (gpScheduleList) {
    DestroyAllSchedules();
  }

  LOADDATA(&ubNum, *hBuffer, sizeof(uint8_t));
  gubScheduleID = 1;
  while (ubNum) {
    LOADDATA(&temp, *hBuffer, sizeof(SCHEDULENODE));

    if (gpScheduleList) {
      pSchedule->next = (SCHEDULENODE *)MemAlloc(sizeof(SCHEDULENODE));
      Assert(pSchedule->next);
      pSchedule = pSchedule->next;
      memcpy(pSchedule, &temp, sizeof(SCHEDULENODE));
    } else {
      gpScheduleList = (SCHEDULENODE *)MemAlloc(sizeof(SCHEDULENODE));
      Assert(gpScheduleList);
      memcpy(gpScheduleList, &temp, sizeof(SCHEDULENODE));
      pSchedule = gpScheduleList;
    }
    pSchedule->ubScheduleID = gubScheduleID;
    pSchedule->ubSoldierID = NO_SOLDIER;
    pSchedule->next = NULL;
    gubScheduleID++;
    ubNum--;
  }
  // Schedules are posted when the soldier is added...
}

extern BOOLEAN gfSchedulesHosed;
BOOLEAN LoadSchedulesFromSave(HWFILE hFile) {
  SCHEDULENODE *pSchedule = NULL;
  SCHEDULENODE temp;
  uint8_t ubNum;
  uint32_t ubRealNum;

  uint32_t uiNumBytesRead, uiNumBytesToRead;

  // LOADDATA( &ubNum, *hBuffer, sizeof( uint8_t ) );
  uiNumBytesToRead = sizeof(uint8_t);
  FileMan_Read(hFile, &ubNum, uiNumBytesToRead, &uiNumBytesRead);
  if (uiNumBytesRead != uiNumBytesToRead) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // Hack problem with schedules getting misaligned.
  ubRealNum = gfSchedulesHosed ? ubNum + 256 : ubNum;

  gubScheduleID = 1;
  while (ubRealNum) {
    uiNumBytesToRead = sizeof(SCHEDULENODE);
    FileMan_Read(hFile, &temp, uiNumBytesToRead, &uiNumBytesRead);
    if (uiNumBytesRead != uiNumBytesToRead) {
      FileMan_Close(hFile);
      return (FALSE);
    }
    // LOADDATA( &temp, *hBuffer, sizeof( SCHEDULENODE ) );

    if (gpScheduleList) {
      pSchedule->next = (SCHEDULENODE *)MemAlloc(sizeof(SCHEDULENODE));
      Assert(pSchedule->next);
      pSchedule = pSchedule->next;
      memcpy(pSchedule, &temp, sizeof(SCHEDULENODE));
    } else {
      gpScheduleList = (SCHEDULENODE *)MemAlloc(sizeof(SCHEDULENODE));
      Assert(gpScheduleList);
      memcpy(gpScheduleList, &temp, sizeof(SCHEDULENODE));
      pSchedule = gpScheduleList;
    }

    // should be unnecessary here, then we can toast reconnect schedule
    /*
    pSchedule->ubScheduleID = gubScheduleID;
    pSchedule->ubSoldierID = NO_SOLDIER;
    */

    pSchedule->next = NULL;
    gubScheduleID++;
    ubRealNum--;
  }
  // Schedules are posted when the soldier is added...
  return (TRUE);
}

// used to fix a bug in the editor where the schedules were reversed.  Because only
// some maps were effected, this feature was required.
void ReverseSchedules() {
  SCHEDULENODE *pReverseHead, *pPrevReverseHead, *pPrevScheduleHead;
  uint8_t ubOppositeID = gubScheduleID;
  // First, remove any gaps which would mess up the reverse ID assignment by optimizing
  // the schedules.
  OptimizeSchedules();
  pReverseHead = NULL;
  while (gpScheduleList) {
    // reverse the ID
    gpScheduleList->ubScheduleID = ubOppositeID;
    ubOppositeID--;
    // detach current schedule head from list and advance it
    pPrevScheduleHead = gpScheduleList;
    gpScheduleList = gpScheduleList->next;
    // get previous reversed list head (even if null)
    pPrevReverseHead = pReverseHead;
    // Assign the previous schedule head to the reverse head
    pReverseHead = pPrevScheduleHead;
    // Point the next to the previous reverse head.
    pReverseHead->next = pPrevReverseHead;
  }
  // Now assign the schedule list to the reverse head.
  gpScheduleList = pReverseHead;
}

// Another debug feature.
void ClearAllSchedules() {
  SOLDIERINITNODE *pNode;
  DestroyAllSchedules();
  pNode = gSoldierInitHead;
  while (pNode) {
    if (pNode->pDetailedPlacement && pNode->pDetailedPlacement->ubScheduleID) {
      pNode->pDetailedPlacement->ubScheduleID = 0;
      if (pNode->pSoldier) {
        pNode->pSoldier->ubScheduleID = 0;
      }
    }
    pNode = pNode->next;
  }
}

BOOLEAN SaveSchedules(HWFILE hFile) {
  SCHEDULENODE *curr;
  uint32_t uiBytesWritten;
  uint8_t ubNum, ubNumFucker;
  int32_t iNum;
  // Now, count the number of schedules in the list
  iNum = 0;
  curr = gpScheduleList;
  while (curr) {
    // skip all default schedules
    if (!(curr->usFlags & SCHEDULE_FLAGS_TEMPORARY)) {
      iNum++;
    }
    curr = curr->next;
  }
  ubNum = (uint8_t)((iNum >= 32) ? 32 : iNum);

  FileMan_Write(hFile, &ubNum, sizeof(uint8_t), &uiBytesWritten);
  if (uiBytesWritten != sizeof(uint8_t)) {
    return (FALSE);
  }
  // Now, save each schedule
  curr = gpScheduleList;
  ubNumFucker = 0;
  while (curr) {
    // skip all default schedules
    if (!(curr->usFlags & SCHEDULE_FLAGS_TEMPORARY)) {
      ubNumFucker++;
      if (ubNumFucker > ubNum) {
        return (TRUE);
      }
      FileMan_Write(hFile, curr, sizeof(SCHEDULENODE), &uiBytesWritten);
      if (uiBytesWritten != sizeof(SCHEDULENODE)) {
        return (FALSE);
      }
    }
    curr = curr->next;
  }
  return (TRUE);
}

// Each schedule has upto four parts to it, so sort them chronologically.
// Happily, the fields with no times actually are the highest.
BOOLEAN SortSchedule(SCHEDULENODE *pSchedule) {
  int32_t index, i, iBestIndex;
  uint16_t usTime;
  uint16_t usData1;
  uint16_t usData2;
  uint8_t ubAction;
  BOOLEAN fSorted = FALSE;

  // Use a bubblesort method (max:  3 switches).
  index = 0;
  while (index < 3) {
    usTime = 0xffff;
    iBestIndex = index;
    for (i = index; i < MAX_SCHEDULE_ACTIONS; i++) {
      if (pSchedule->usTime[i] < usTime) {
        usTime = pSchedule->usTime[i];
        iBestIndex = i;
      }
    }
    if (iBestIndex != index) {  // we will swap the best index with the current index.
      fSorted = TRUE;
      usTime = pSchedule->usTime[index];
      usData1 = pSchedule->usData1[index];
      usData2 = pSchedule->usData2[index];
      ubAction = pSchedule->ubAction[index];
      pSchedule->usTime[index] = pSchedule->usTime[iBestIndex];
      pSchedule->usData1[index] = pSchedule->usData1[iBestIndex];
      pSchedule->usData2[index] = pSchedule->usData2[iBestIndex];
      pSchedule->ubAction[index] = pSchedule->ubAction[iBestIndex];
      pSchedule->usTime[iBestIndex] = usTime;
      pSchedule->usData1[iBestIndex] = usData1;
      pSchedule->usData2[iBestIndex] = usData2;
      pSchedule->ubAction[iBestIndex] = ubAction;
    }
    index++;
  }
  return fSorted;
}

BOOLEAN BumpAnyExistingMerc(int16_t sGridNo) {
  uint8_t ubID;
  struct SOLDIERTYPE *pSoldier;  // NB this is the person already in the location,
  int16_t sNewGridNo;
  uint8_t ubDir;
  int16_t sCellX, sCellY;

  // this is for autoprocessing schedules...
  // there could be someone in the destination location, in which case
  // we want to 'bump' them to the nearest available spot

  if (!GridNoOnVisibleWorldTile(sGridNo)) {
    return (TRUE);
  }

  ubID = WhoIsThere2(sGridNo, 0);

  if (ubID == NOBODY) {
    return (TRUE);
  }

  pSoldier = MercPtrs[ubID];

  // what if the existing merc is prone?
  sNewGridNo =
      FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, STANDING, 5, &ubDir, 1, pSoldier);
  // sNewGridNo = FindGridNoFromSweetSpotExcludingSweetSpot( pSoldier, sGridNo, 10, &ubDir );

  if (sNewGridNo == NOWHERE) {
    return (FALSE);
  }

  ConvertGridNoToCellXY(sNewGridNo, &sCellX, &sCellY);
  EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);

  return (TRUE);
}

void AutoProcessSchedule(SCHEDULENODE *pSchedule, int32_t index) {
  int16_t sCellX, sCellY, sGridNo;
  int8_t bDirection;
  struct SOLDIERTYPE *pSoldier;

  if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) {
    // CJC, November 28th:  when reloading a saved game we want events posted but no events
    // autoprocessed since that could change civilian positions.  So rather than doing a bunch of
    // checks outside of this function, I thought it easier to screen them out here.
    return;
  }

  pSoldier = MercPtrs[pSchedule->ubSoldierID];

#ifdef JA2EDITOR
  if (GetSolProfile(pSoldier) != NO_PROFILE) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("Autoprocessing schedule action %S for %S (%d) at time %02ld:%02ld (set for "
                    "%02d:%02d), data1 = %d",
                    gszScheduleActions[pSchedule->ubAction[index]], pSoldier->name,
                    GetSolID(pSoldier), GetWorldHour(), guiMin, pSchedule->usTime[index] / 60,
                    pSchedule->usTime[index] % 60, pSchedule->usData1[index]));
  } else {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("Autoprocessing schedule action %S for civ (%d) at time %02ld:%02ld (set for "
                    "%02d:%02d), data1 = %d",
                    gszScheduleActions[pSchedule->ubAction[index]], GetSolID(pSoldier),
                    GetWorldHour(), guiMin, pSchedule->usTime[index] / 60,
                    pSchedule->usTime[index] % 60, pSchedule->usData1[index]));
  }
#endif

  // always assume the merc is going to wake, unless the event is a sleep
  pSoldier->fAIFlags &= ~(AI_ASLEEP);

  switch (pSchedule->ubAction[index]) {
    case SCHEDULE_ACTION_LOCKDOOR:
    case SCHEDULE_ACTION_UNLOCKDOOR:
    case SCHEDULE_ACTION_OPENDOOR:
    case SCHEDULE_ACTION_CLOSEDOOR:
      PerformActionOnDoorAdjacentToGridNo(pSchedule->ubAction[index], pSchedule->usData1[index]);
      BumpAnyExistingMerc(pSchedule->usData2[index]);
      ConvertGridNoToCellXY(pSchedule->usData2[index], &sCellX, &sCellY);

      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);
      if (GridNoOnEdgeOfMap(pSchedule->usData2[index], &bDirection)) {
        // civ should go off map; this tells us where the civ will return
        pSoldier->sOffWorldGridNo = pSchedule->usData2[index];
        MoveSoldierFromMercToAwaySlot(pSoldier);
        pSoldier->bInSector = FALSE;
      } else {
        // let this person patrol from here from now on
        pSoldier->usPatrolGrid[0] = pSchedule->usData2[index];
      }
      break;
    case SCHEDULE_ACTION_GRIDNO:
      BumpAnyExistingMerc(pSchedule->usData1[index]);
      ConvertGridNoToCellXY(pSchedule->usData1[index], &sCellX, &sCellY);
      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);
      // let this person patrol from here from now on
      pSoldier->usPatrolGrid[0] = pSchedule->usData1[index];
      break;
    case SCHEDULE_ACTION_ENTERSECTOR:
      if (GetSolProfile(pSoldier) != NO_PROFILE &&
          gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags2 &
              PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR) {
        // never process enter if flag is set
        break;
      }
      BumpAnyExistingMerc(pSchedule->usData1[index]);
      ConvertGridNoToCellXY(pSchedule->usData1[index], &sCellX, &sCellY);
      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);
      MoveSoldierFromAwayToMercSlot(pSoldier);
      pSoldier->bInSector = TRUE;
      // let this person patrol from here from now on
      pSoldier->usPatrolGrid[0] = pSchedule->usData1[index];
      break;
    case SCHEDULE_ACTION_WAKE:
      BumpAnyExistingMerc(pSoldier->sInitialGridNo);
      ConvertGridNoToCellXY(pSoldier->sInitialGridNo, &sCellX, &sCellY);
      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);
      // let this person patrol from here from now on
      pSoldier->usPatrolGrid[0] = pSoldier->sInitialGridNo;
      break;
    case SCHEDULE_ACTION_SLEEP:
      pSoldier->fAIFlags |= AI_ASLEEP;
      // check for someone else in the location
      BumpAnyExistingMerc(pSchedule->usData1[index]);
      ConvertGridNoToCellXY(pSchedule->usData1[index], &sCellX, &sCellY);
      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);
      pSoldier->usPatrolGrid[0] = pSchedule->usData1[index];
      break;
    case SCHEDULE_ACTION_LEAVESECTOR:
      sGridNo = FindNearestEdgePoint(pSoldier->sGridNo);
      BumpAnyExistingMerc(sGridNo);
      ConvertGridNoToCellXY(sGridNo, &sCellX, &sCellY);
      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);

      sGridNo = FindNearbyPointOnEdgeOfMap(pSoldier, &bDirection);
      BumpAnyExistingMerc(sGridNo);
      ConvertGridNoToCellXY(sGridNo, &sCellX, &sCellY);
      EVENT_SetSoldierPositionForceDelete(pSoldier, (float)sCellX, (float)sCellY);

      // ok, that tells us where the civ will return
      pSoldier->sOffWorldGridNo = sGridNo;
      MoveSoldierFromMercToAwaySlot(pSoldier);
      pSoldier->bInSector = FALSE;
      break;
  }
}

void PostSchedule(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiStartTime, uiEndTime;
  int32_t i;
  int8_t bEmpty;
  SCHEDULENODE *pSchedule;
  uint8_t ubTempAction;
  uint16_t usTemp;

  if ((pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP) &&
      (gTacticalStatus.fCivGroupHostile[KINGPIN_CIV_GROUP] ||
       ((gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS) &&
        (CheckFact(FACT_KINGPIN_CAN_SEND_ASSASSINS, KINGPIN)))) &&
      (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C) &&
      (GetSolProfile(pSoldier) == NO_PROFILE)) {
    // no schedules for people guarding Tony's!
    return;
  }

  pSchedule = GetSchedule(pSoldier->ubScheduleID);
  if (!pSchedule) return;

  if (GetSolProfile(pSoldier) != NO_PROFILE && gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags3 &
                                                   PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE) {
    // don't process schedule
    return;
  }

  // if this schedule doesn't have a time associated with it, then generate a time, but only
  // if it is a sleep schedule.
  for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
    if (pSchedule->ubAction[i] == SCHEDULE_ACTION_SLEEP) {
      // first make sure that this merc has a unique spot to sleep in
      SecureSleepSpot(pSoldier, pSchedule->usData1[i]);

      if (pSchedule->usTime[i] == 0xffff) {
        pSchedule->usTime[i] = (uint16_t)((21 * 60) + Random((3 * 60)));  // 9PM - 11:59PM

        if (ScheduleHasMorningNonSleepEntries(pSchedule)) {
          // this guy will sleep until the next non-sleep event
        } else {
          bEmpty = GetEmptyScheduleEntry(pSchedule);
          if (bEmpty != -1) {
            // there is an empty entry for the wakeup call

            // NB the wakeup call must be ordered first! so we have to create the
            // wake action and then swap the two.
            pSchedule->ubAction[bEmpty] = SCHEDULE_ACTION_WAKE;
            pSchedule->usTime[bEmpty] =
                (pSchedule->usTime[i] + (8 * 60)) % NUM_MIN_IN_DAY;  // sleep for 8 hours

            ubTempAction = pSchedule->ubAction[bEmpty];
            pSchedule->ubAction[bEmpty] = pSchedule->ubAction[i];
            pSchedule->ubAction[i] = ubTempAction;

            usTemp = pSchedule->usTime[bEmpty];
            pSchedule->usTime[bEmpty] = pSchedule->usTime[i];
            pSchedule->usTime[i] = usTemp;

            usTemp = pSchedule->usData1[bEmpty];
            pSchedule->usData1[bEmpty] = pSchedule->usData1[i];
            pSchedule->usData1[i] = usTemp;

            usTemp = pSchedule->usData2[bEmpty];
            pSchedule->usData2[bEmpty] = pSchedule->usData2[i];
            pSchedule->usData2[i] = usTemp;
          } else {
            // no morning entries but no space for a wakeup either, will sleep till
            // next non-sleep event
          }
        }
        break;  // The break is here because nobody should have more than one sleep schedule with no
                // time specified.
      }
    }
  }

  pSchedule->ubSoldierID = GetSolID(pSoldier);

  // always process previous 24 hours
  uiEndTime = GetWorldTotalMin();
  uiStartTime = uiEndTime - (NUM_MIN_IN_DAY - 1);

  /*
  //First thing we need is to get the time that the map was last loaded.  If more than 24 hours,
  //then process only 24 hours.  If less, then process all the schedules that would have happened
  within
  //that period of time.
  uiEndTime = GetWorldTotalMin();
  if( GetWorldTotalMin() - guiTimeCurrentSectorWasLastLoaded > NUM_MIN_IN_DAY )
  { //Process the last 24 hours
          uiStartTime = uiEndTime - (NUM_MIN_IN_DAY - 1);
  }
  else
  { //Process the time since we were last here.
          uiStartTime = guiTimeCurrentSectorWasLastLoaded;
  }
  */

  // Need a way to determine if the player has actually modified doors since this civilian was last
  // loaded
  uiEndTime %= NUM_MIN_IN_DAY;
  uiStartTime %= NUM_MIN_IN_DAY;
  PrepareScheduleForAutoProcessing(pSchedule, uiStartTime, uiEndTime);
}

void PrepareScheduleForAutoProcessing(SCHEDULENODE *pSchedule, uint32_t uiStartTime,
                                      uint32_t uiEndTime) {
  int32_t i;
  BOOLEAN fPostedNextEvent = FALSE;

  if (uiStartTime > uiEndTime) {  // The start time is later in the day than the end time, which
                                  // means we'll be wrapping
    // through midnight and continuing to the end time.
    for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
      if (pSchedule->usTime[i] == 0xffff) break;
      if (pSchedule->usTime[i] >= uiStartTime) {
        AutoProcessSchedule(pSchedule, i);
      }
    }
    for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
      if (pSchedule->usTime[i] == 0xffff) break;
      if (pSchedule->usTime[i] <= uiEndTime) {
        AutoProcessSchedule(pSchedule, i);
      } else {
        // CJC: Note that end time is always passed in here as the current time so
        // GetWorldDayInMinutes will be for the correct day
        AddStrategicEvent(EVENT_PROCESS_TACTICAL_SCHEDULE,
                          GetWorldDayInMinutes() + pSchedule->usTime[i], pSchedule->ubScheduleID);
        fPostedNextEvent = TRUE;
        break;
      }
    }
  } else {  // Much simpler:  start at the start and continue to the end.
    for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
      if (pSchedule->usTime[i] == 0xffff) break;

      if (pSchedule->usTime[i] >= uiStartTime && pSchedule->usTime[i] <= uiEndTime) {
        AutoProcessSchedule(pSchedule, i);
      } else if (pSchedule->usTime[i] >= uiEndTime) {
        fPostedNextEvent = TRUE;
        AddStrategicEvent(EVENT_PROCESS_TACTICAL_SCHEDULE,
                          GetWorldDayInMinutes() + pSchedule->usTime[i], pSchedule->ubScheduleID);
        break;
      }
    }
  }

  if (!fPostedNextEvent) {
    // reached end of schedule, post first event for soldier in the next day
    // 0th event will be first.
    // Feb 1:  ONLY IF THERE IS A VALID EVENT TO POST WITH A VALID TIME!
    if (pSchedule->usTime[0] != 0xffff) {
      AddStrategicEvent(EVENT_PROCESS_TACTICAL_SCHEDULE,
                        GetWorldDayInMinutes() + NUM_MIN_IN_DAY + pSchedule->usTime[0],
                        pSchedule->ubScheduleID);
    }
  }
}

// Leave at night, come back in the morning.  The time variances are a couple hours, so
// the town doesn't turn into a ghost town in 5 minutes.
void PostDefaultSchedule(struct SOLDIERTYPE *pSoldier) {
  int32_t i;
  SCHEDULENODE *curr;

  if (gbWorldSectorZ) {  // People in underground sectors don't get schedules.
    return;
  }
  // Create a new node at the head of the list.  The head will become the new schedule
  // we are about to add.
  curr = gpScheduleList;
  gpScheduleList = (SCHEDULENODE *)MemAlloc(sizeof(SCHEDULENODE));
  memset(gpScheduleList, 0, sizeof(SCHEDULENODE));
  gpScheduleList->next = curr;
  gubScheduleID++;
  // Assign all of the links
  gpScheduleList->ubScheduleID = gubScheduleID;
  gpScheduleList->ubSoldierID = GetSolID(pSoldier);
  pSoldier->ubScheduleID = gubScheduleID;

  // Clear the data inside the schedule
  for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
    gpScheduleList->usTime[i] = 0xffff;
    gpScheduleList->usData1[i] = 0xffff;
    gpScheduleList->usData2[i] = 0xffff;
  }
  // Have the default schedule enter between 7AM and 8AM
  gpScheduleList->ubAction[0] = SCHEDULE_ACTION_ENTERSECTOR;
  gpScheduleList->usTime[0] = (uint16_t)(420 + Random(61));
  gpScheduleList->usData1[0] = pSoldier->sInitialGridNo;
  // Have the default schedule leave between 6PM and 8PM
  gpScheduleList->ubAction[1] = SCHEDULE_ACTION_LEAVESECTOR;
  gpScheduleList->usTime[1] = (uint16_t)(1080 + Random(121));
  gpScheduleList->usFlags |= SCHEDULE_FLAGS_TEMPORARY;

  if (gubScheduleID == 255) {  // Too much fragmentation, clean it up...
    OptimizeSchedules();
    if (gubScheduleID == 255) {
      AssertMsg(0, "TOO MANY SCHEDULES POSTED!!!");
    }
  }

  PostSchedule(pSoldier);
}

void PostSchedules() {
  SOLDIERINITNODE *curr;
  BOOLEAN fDefaultSchedulesPossible = FALSE;

#if defined(DISABLESCHEDULES)  // definition found at top of this .c file.

  return;

#endif
  // If no way to leave the map, then don't post default schedules.
  if (gMapInformation.sNorthGridNo != -1 || gMapInformation.sEastGridNo != -1 ||
      gMapInformation.sSouthGridNo != -1 || gMapInformation.sWestGridNo != -1) {
    fDefaultSchedulesPossible = TRUE;
  }
  curr = gSoldierInitHead;
  while (curr) {
    if (curr->pSoldier && curr->pSoldier->bTeam == CIV_TEAM) {
      if (curr->pDetailedPlacement && curr->pDetailedPlacement->ubScheduleID) {
        PostSchedule(curr->pSoldier);
      } else if (fDefaultSchedulesPossible) {
        // ATE: There should be a better way here...
        if (curr->pSoldier->ubBodyType != COW && curr->pSoldier->ubBodyType != BLOODCAT &&
            curr->pSoldier->ubBodyType != HUMVEE && curr->pSoldier->ubBodyType != ELDORADO &&
            curr->pSoldier->ubBodyType != ICECREAMTRUCK && curr->pSoldier->ubBodyType != JEEP) {
          PostDefaultSchedule(curr->pSoldier);
        }
      }
    }
    curr = curr->next;
  }
}

void PerformActionOnDoorAdjacentToGridNo(uint8_t ubScheduleAction, uint16_t usGridNo) {
  int16_t sDoorGridNo;
  DOOR *pDoor;

  sDoorGridNo = FindDoorAtGridNoOrAdjacent((int16_t)usGridNo);
  if (sDoorGridNo != NOWHERE) {
    switch (ubScheduleAction) {
      case SCHEDULE_ACTION_LOCKDOOR:
        pDoor = FindDoorInfoAtGridNo(sDoorGridNo);
        if (pDoor) {
          pDoor->fLocked = TRUE;
        }
        // make sure it's closed as well
        ModifyDoorStatus(sDoorGridNo, FALSE, DONTSETDOORSTATUS);
        break;
      case SCHEDULE_ACTION_UNLOCKDOOR:
        pDoor = FindDoorInfoAtGridNo(sDoorGridNo);
        if (pDoor) {
          pDoor->fLocked = FALSE;
        }
        break;
      case SCHEDULE_ACTION_OPENDOOR:
        ModifyDoorStatus(sDoorGridNo, TRUE, DONTSETDOORSTATUS);
        break;
      case SCHEDULE_ACTION_CLOSEDOOR:
        ModifyDoorStatus(sDoorGridNo, FALSE, DONTSETDOORSTATUS);
        break;
    }
  }
}

// Assumes that a schedule has just been processed.  This takes the current time, and compares it to
// the schedule, and looks for the next schedule action that would get processed and posts it.
void PostNextSchedule(struct SOLDIERTYPE *pSoldier) {
  SCHEDULENODE *pSchedule;
  int32_t i, iBestIndex;
  uint16_t usTime, usBestTime;
  pSchedule = GetSchedule(pSoldier->ubScheduleID);
  if (!pSchedule) {  // post default?
    return;
  }
  usTime = (uint16_t)GetWorldMinutesInDay();
  usBestTime = 0xffff;
  iBestIndex = -1;
  for (i = 0; i < MAX_SCHEDULE_ACTIONS; i++) {
    if (pSchedule->usTime[i] == 0xffff) continue;
    if (pSchedule->usTime[i] == usTime) continue;
    if (pSchedule->usTime[i] > usTime) {
      if (pSchedule->usTime[i] - usTime < usBestTime) {
        usBestTime = pSchedule->usTime[i] - usTime;
        iBestIndex = i;
      }
    } else if ((NUM_MIN_IN_DAY - (usTime - pSchedule->usTime[i])) < usBestTime) {
      usBestTime = NUM_MIN_IN_DAY - (usTime - pSchedule->usTime[i]);
      iBestIndex = i;
    }
  }
  Assert(iBestIndex >= 0);

  AddStrategicEvent(EVENT_PROCESS_TACTICAL_SCHEDULE,
                    GetWorldDayInMinutes() + pSchedule->usTime[iBestIndex],
                    pSchedule->ubScheduleID);
}

BOOLEAN ExtractScheduleEntryAndExitInfo(struct SOLDIERTYPE *pSoldier, uint32_t *puiEntryTime,
                                        uint32_t *puiExitTime) {
  int32_t iLoop;
  BOOLEAN fFoundEntryTime = FALSE, fFoundExitTime = FALSE;
  SCHEDULENODE *pSchedule;

  *puiEntryTime = 0;
  *puiExitTime = 0;

  pSchedule = GetSchedule(pSoldier->ubScheduleID);
  if (!pSchedule) {
    // If person had default schedule then would have been assigned and this would
    // have succeeded.
    // Hence this is an error.
    return (FALSE);
  }

  for (iLoop = 0; iLoop < MAX_SCHEDULE_ACTIONS; iLoop++) {
    if (pSchedule->ubAction[iLoop] == SCHEDULE_ACTION_ENTERSECTOR) {
      fFoundEntryTime = TRUE;
      *puiEntryTime = pSchedule->usTime[iLoop];
    } else if (pSchedule->ubAction[iLoop] == SCHEDULE_ACTION_LEAVESECTOR) {
      fFoundExitTime = TRUE;
      *puiExitTime = pSchedule->usTime[iLoop];
    }
  }

  if (fFoundEntryTime && fFoundExitTime) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

// This is for determining shopkeeper's opening/closing hours
BOOLEAN ExtractScheduleDoorLockAndUnlockInfo(struct SOLDIERTYPE *pSoldier, uint32_t *puiOpeningTime,
                                             uint32_t *puiClosingTime) {
  int32_t iLoop;
  BOOLEAN fFoundOpeningTime = FALSE, fFoundClosingTime = FALSE;
  SCHEDULENODE *pSchedule;

  *puiOpeningTime = 0;
  *puiClosingTime = 0;

  pSchedule = GetSchedule(pSoldier->ubScheduleID);
  if (!pSchedule) {
    // If person had default schedule then would have been assigned and this would
    // have succeeded.
    // Hence this is an error.
    return (FALSE);
  }

  for (iLoop = 0; iLoop < MAX_SCHEDULE_ACTIONS; iLoop++) {
    if (pSchedule->ubAction[iLoop] == SCHEDULE_ACTION_UNLOCKDOOR) {
      fFoundOpeningTime = TRUE;
      *puiOpeningTime = pSchedule->usTime[iLoop];
    } else if (pSchedule->ubAction[iLoop] == SCHEDULE_ACTION_LOCKDOOR) {
      fFoundClosingTime = TRUE;
      *puiClosingTime = pSchedule->usTime[iLoop];
    }
  }

  if (fFoundOpeningTime && fFoundClosingTime) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN GetEarliestMorningScheduleEvent(SCHEDULENODE *pSchedule, uint32_t *puiTime) {
  int32_t iLoop;

  *puiTime = 100000;

  for (iLoop = 0; iLoop < MAX_SCHEDULE_ACTIONS; iLoop++) {
    if (pSchedule->usTime[iLoop] < (12 * 60) && pSchedule->usTime[iLoop] < *puiTime) {
      *puiTime = pSchedule->usTime[iLoop];
    }
  }

  if (*puiTime == 100000) {
    return (FALSE);
  } else {
    return (TRUE);
  }
}

BOOLEAN ScheduleHasMorningNonSleepEntries(SCHEDULENODE *pSchedule) {
  int8_t bLoop;

  for (bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++) {
    if (pSchedule->ubAction[bLoop] != SCHEDULE_ACTION_NONE &&
        pSchedule->ubAction[bLoop] != SCHEDULE_ACTION_SLEEP) {
      if (pSchedule->usTime[bLoop] < (12 * 60)) {
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

int8_t GetEmptyScheduleEntry(SCHEDULENODE *pSchedule) {
  int8_t bLoop;

  for (bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++) {
    if (pSchedule->ubAction[bLoop] == SCHEDULE_ACTION_NONE) {
      return (bLoop);
    }
  }

  return (-1);
}

/*
void ReconnectSchedules( void )
{
        uint32_t						uiLoop;
        struct SOLDIERTYPE *			pSoldier;
        SCHEDULENODE *		pSchedule;

        for ( uiLoop = gTacticalStatus.Team[ CIV_TEAM ].bFirstID; uiLoop <= gTacticalStatus.Team[
CIV_TEAM ].bLastID; uiLoop++ )
        {
                pSoldier = MercPtrs[ uiLoop ];
                if ( IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->ubScheduleID != 0 )
                {
                        pSchedule = GetSchedule( pSoldier->ubScheduleID );
                        if ( pSchedule )
                        {
                                // set soldier ptr to point to this guy!
                                pSchedule->ubSoldierID = GetSolID(pSoldier);
                        }
                        else
                        {
                                // need default schedule!
                                //PostDefaultSchedule( pSoldier );
                        }
                }
        }
}
*/

uint16_t FindSleepSpot(SCHEDULENODE *pSchedule) {
  int8_t bLoop;

  for (bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++) {
    if (pSchedule->ubAction[bLoop] == SCHEDULE_ACTION_SLEEP) {
      return (pSchedule->usData1[bLoop]);
    }
  }
  return (NOWHERE);
}

void ReplaceSleepSpot(SCHEDULENODE *pSchedule, uint16_t usNewSpot) {
  int8_t bLoop;

  for (bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++) {
    if (pSchedule->ubAction[bLoop] == SCHEDULE_ACTION_SLEEP) {
      pSchedule->usData1[bLoop] = usNewSpot;
      break;
    }
  }
}

void SecureSleepSpot(struct SOLDIERTYPE *pSoldier, uint16_t usSleepSpot) {
  struct SOLDIERTYPE *pSoldier2;
  uint16_t usSleepSpot2, usNewSleepSpot;
  uint32_t uiLoop;
  SCHEDULENODE *pSchedule;
  uint8_t ubDirection;

  // start after this soldier's ID so we don't duplicate work done in previous passes
  for (uiLoop = GetSolID(pSoldier) + 1; uiLoop <= gTacticalStatus.Team[CIV_TEAM].bLastID;
       uiLoop++) {
    pSoldier2 = MercPtrs[uiLoop];
    if (pSoldier2->bActive && pSoldier2->bInSector && pSoldier2->ubScheduleID != 0) {
      pSchedule = GetSchedule(pSoldier2->ubScheduleID);
      if (pSchedule) {
        usSleepSpot2 = FindSleepSpot(pSchedule);
        if (usSleepSpot2 == usSleepSpot) {
          // conflict!
          // usNewSleepSpot = (int16_t) FindGridNoFromSweetSpotWithStructData( pSoldier2,
          // pSoldier2->usAnimState, usSleepSpot2, 3, &ubDirection, FALSE );
          usNewSleepSpot =
              FindGridNoFromSweetSpotExcludingSweetSpot(pSoldier2, usSleepSpot2, 3, &ubDirection);
          if (usNewSleepSpot != NOWHERE) {
            ReplaceSleepSpot(pSchedule, usNewSleepSpot);
          }
        }
      }
    }
  }
}

/*
void SecureSleepSpots( void )
{
        // make sure no one else has the same sleep dest as another merc, and if they do
        // move extras away!
        uint32_t						uiLoop;
        struct SOLDIERTYPE *			pSoldier;
        SCHEDULENODE *		pSchedule;
        uint16_t						usSleepSpot;

        for ( uiLoop = gTacticalStatus.Team[ CIV_TEAM ].bFirstID; uiLoop <= gTacticalStatus.Team[
CIV_TEAM ].bLastID; uiLoop++ )
        {
                pSoldier = MercPtrs[ uiLoop ];
                if ( IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->ubScheduleID != 0 )
                {
                        pSchedule = GetSchedule( pSoldier->ubScheduleID );
                        if ( pSchedule )
                        {
                                usSleepSpot = FindSleepSpot( pSchedule );
                                if ( usSleepSpot != NOWHERE )
                                {
                                        SecureSleepSpot( pSoldier, usSleepSpot );
                                }
                        }
                }
        }

}
*/
