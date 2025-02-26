// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/Meanwhile.h"

#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/WorldDef.h"
#include "Town.h"
#include "UI.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"

#define MAX_MEANWHILE_PROFILES 10

int8_t gzMeanwhileStr[][30] = {
    "End of player's first battle",
    "Drassen Lib. ",
    "Cambria Lib.",
    "Alma Lib.",
    "Grumm lib.",
    "Chitzena Lib.",
    "NW SAM",
    "NE SAM",
    "Central SAM",
    "Flowers",
    "Lost town",
    "Interrogation",
    "Creatures",
    "Kill Chopper",
    "AWOL Madlab",
    "Outskirts Meduna",
    "Balime Lib.",
};

// the snap to grid nos for meanwhile scenes
uint16_t gusMeanWhileGridNo[] = {
    12248, 12248, 12248, 12248, 12248, 12248, 12248, 12248, 12248,
    12248, 12248, 8075,  12248, 12248, 12248, 12248, 12248,
};

typedef struct {
  uint8_t ubProfile;
  int16_t sX;
  int16_t sY;
  int16_t sZ;
  int16_t sGridNo;

} NPC_SAVE_INFO;

// BEGIN SERALIZATION
MEANWHILE_DEFINITION gCurrentMeanwhileDef;
MEANWHILE_DEFINITION gMeanwhileDef[NUM_MEANWHILES];
BOOLEAN gfMeanwhileTryingToStart = FALSE;
BOOLEAN gfInMeanwhile = FALSE;
// END SERIALIZATION
uint8_t gsOldSectorX;
uint8_t gsOldSectorY;
uint8_t gsOldSectorZ;
uint8_t gsOldSelectedSectorX;
uint8_t gsOldSelectedSectorY;
uint8_t gsOldSelectedSectorZ;

uint32_t guiOldScreen;
NPC_SAVE_INFO gNPCSaveData[MAX_MEANWHILE_PROFILES];
uint32_t guiNumNPCSaves = 0;
BOOLEAN gfReloadingScreenFromMeanwhile = FALSE;
int16_t gsOldCurInterfacePanel = 0;
BOOLEAN gfWorldWasLoaded = FALSE;
uint8_t ubCurrentMeanWhileId = 0;

void BeginMeanwhileCallBack(uint8_t bExitValue);
void DoneFadeOutMeanwhile(void);
void DoneFadeInMeanwhile(void);
void DoneFadeOutMeanwhileOnceDone(void);
void DoneFadeInMeanwhileOnceDone(void);
void LocateMeanWhileGrid(void);

uint32_t uiMeanWhileFlags = 0;

// meanwhile flag defines
#define END_OF_PLAYERS_FIRST_BATTLE_FLAG 0x00000001
#define DRASSEN_LIBERATED_FLAG 0x00000002
#define CAMBRIA_LIBERATED_FLAG 0x00000004
#define ALMA_LIBERATED_FLAG 0x00000008
#define GRUMM_LIBERATED_FLAG 0x00000010
#define CHITZENA_LIBERATED_FLAG 0x00000020
#define NW_SAM_FLAG 0x00000040
#define NE_SAM_FLAG 0x00000080
#define CENTRAL_SAM_FLAG 0x00000100
#define FLOWERS_FLAG 0x00000200
#define LOST_TOWN_FLAG 0x00000400
#define CREATURES_FLAG 0x00000800
#define KILL_CHOPPER_FLAG 0x00001000
#define AWOL_SCIENTIST_FLAG 0x00002000
#define OUTSKIRTS_MEDUNA_FLAG 0x00004000
#define INTERROGATION_FLAG 0x00008000
#define BALIME_LIBERATED_FLAG 0x00010000

extern void InternalLocateGridNo(uint16_t sGridNo, BOOLEAN fForce);

void ProcessImplicationsOfMeanwhile(void);

// set flag for this event
void SetMeanWhileFlag(uint8_t ubMeanwhileID) {
  switch (ubMeanwhileID) {
    case END_OF_PLAYERS_FIRST_BATTLE:
      uiMeanWhileFlags |= END_OF_PLAYERS_FIRST_BATTLE_FLAG;
      break;
    case DRASSEN_LIBERATED:
      uiMeanWhileFlags |= DRASSEN_LIBERATED_FLAG;
      break;
    case CAMBRIA_LIBERATED:
      uiMeanWhileFlags |= CAMBRIA_LIBERATED_FLAG;
      break;
    case ALMA_LIBERATED:
      uiMeanWhileFlags |= ALMA_LIBERATED_FLAG;
      break;
    case GRUMM_LIBERATED:
      uiMeanWhileFlags |= GRUMM_LIBERATED_FLAG;
      break;
    case CHITZENA_LIBERATED:
      uiMeanWhileFlags |= CHITZENA_LIBERATED_FLAG;
      break;
    case BALIME_LIBERATED:
      uiMeanWhileFlags |= BALIME_LIBERATED_FLAG;
      break;
    case NW_SAM:
      uiMeanWhileFlags |= NW_SAM_FLAG;
      break;
    case NE_SAM:
      uiMeanWhileFlags |= NE_SAM_FLAG;
      break;
    case CENTRAL_SAM:
      uiMeanWhileFlags |= CENTRAL_SAM_FLAG;
      break;
    case FLOWERS:
      uiMeanWhileFlags |= FLOWERS_FLAG;
      break;
    case LOST_TOWN:
      uiMeanWhileFlags |= LOST_TOWN_FLAG;
      break;
    case CREATURES:
      uiMeanWhileFlags |= CREATURES_FLAG;
      break;
    case KILL_CHOPPER:
      uiMeanWhileFlags |= KILL_CHOPPER_FLAG;
      break;
    case AWOL_SCIENTIST:
      uiMeanWhileFlags |= AWOL_SCIENTIST_FLAG;
      break;
    case OUTSKIRTS_MEDUNA:
      uiMeanWhileFlags |= OUTSKIRTS_MEDUNA_FLAG;
      break;
    case INTERROGATION:
      uiMeanWhileFlags |= INTERROGATION_FLAG;
      break;
  }
}

// is this flag set?
BOOLEAN GetMeanWhileFlag(uint8_t ubMeanwhileID) {
  uint32_t uiTrue = FALSE;
  switch (ubMeanwhileID) {
    case END_OF_PLAYERS_FIRST_BATTLE:
      uiTrue = (uiMeanWhileFlags & END_OF_PLAYERS_FIRST_BATTLE_FLAG);
      break;
    case DRASSEN_LIBERATED:
      uiTrue = (uiMeanWhileFlags & DRASSEN_LIBERATED_FLAG);
      break;
    case CAMBRIA_LIBERATED:
      uiTrue = (uiMeanWhileFlags & CAMBRIA_LIBERATED_FLAG);
      break;
    case ALMA_LIBERATED:
      uiTrue = (uiMeanWhileFlags & ALMA_LIBERATED_FLAG);
      break;
    case GRUMM_LIBERATED:
      uiTrue = (uiMeanWhileFlags & GRUMM_LIBERATED_FLAG);
      break;
    case CHITZENA_LIBERATED:
      uiTrue = (uiMeanWhileFlags & CHITZENA_LIBERATED_FLAG);
      break;
    case BALIME_LIBERATED:
      uiTrue = (uiMeanWhileFlags & BALIME_LIBERATED_FLAG);
      break;
    case NW_SAM:
      uiTrue = (uiMeanWhileFlags & NW_SAM_FLAG);
      break;
    case NE_SAM:
      uiTrue = (uiMeanWhileFlags & NE_SAM_FLAG);
      break;
    case CENTRAL_SAM:
      uiTrue = (uiMeanWhileFlags & CENTRAL_SAM_FLAG);
      break;
    case FLOWERS:
      uiTrue = (uiMeanWhileFlags & FLOWERS_FLAG);
      break;
    case LOST_TOWN:
      uiTrue = (uiMeanWhileFlags & LOST_TOWN_FLAG);
      break;
    case CREATURES:
      uiTrue = (uiMeanWhileFlags & CREATURES_FLAG);
      break;
    case KILL_CHOPPER:
      uiTrue = (uiMeanWhileFlags & KILL_CHOPPER_FLAG);
      break;
    case AWOL_SCIENTIST:
      uiTrue = (uiMeanWhileFlags & AWOL_SCIENTIST_FLAG);
      break;
    case OUTSKIRTS_MEDUNA:
      uiTrue = (uiMeanWhileFlags & OUTSKIRTS_MEDUNA_FLAG);
      break;
    case INTERROGATION:
      uiTrue = (uiMeanWhileFlags & INTERROGATION_FLAG);
      break;
  }

  if (uiTrue) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

int32_t GetFreeNPCSave(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumNPCSaves; uiCount++) {
    if ((gNPCSaveData[uiCount].ubProfile == NO_PROFILE)) return ((int32_t)uiCount);
  }

  if (guiNumNPCSaves < MAX_MEANWHILE_PROFILES) return ((int32_t)guiNumNPCSaves++);

  return (-1);
}

void RecountNPCSaves(void) {
  int32_t uiCount;

  for (uiCount = guiNumNPCSaves - 1; (uiCount >= 0); uiCount--) {
    if ((gNPCSaveData[uiCount].ubProfile != NO_PROFILE)) {
      guiNumNPCSaves = (uint32_t)(uiCount + 1);
      break;
    }
  }
}

void ScheduleMeanwhileEvent(MEANWHILE_DEFINITION *pMeanwhileDef, uint32_t uiTime) {
  // event scheduled to happen before, ignore
  if (GetMeanWhileFlag(pMeanwhileDef->ubMeanwhileID) == TRUE) {
    return;
  }

  // set the meanwhile flag for this event
  SetMeanWhileFlag(pMeanwhileDef->ubMeanwhileID);

  // set the id value
  ubCurrentMeanWhileId = pMeanwhileDef->ubMeanwhileID;

  // Copy definiaiotn structure into position in global array....
  memcpy(&(gMeanwhileDef[pMeanwhileDef->ubMeanwhileID]), pMeanwhileDef,
         sizeof(MEANWHILE_DEFINITION));

  // A meanwhile.. poor elliot!
  // increment his slapped count...

  // We need to do it here 'cause they may skip it...
  if (gMercProfiles[ELLIOT].bNPCData != 17) {
    gMercProfiles[ELLIOT].bNPCData++;
  }

  AddStrategicEvent(EVENT_MEANWHILE, uiTime, pMeanwhileDef->ubMeanwhileID);
}

BOOLEAN BeginMeanwhile(uint8_t ubMeanwhileID) {
  int32_t cnt;

  // copy meanwhile data from array to structure for current
  memcpy(&gCurrentMeanwhileDef, &(gMeanwhileDef[ubMeanwhileID]), sizeof(MEANWHILE_DEFINITION));

  gfMeanwhileTryingToStart = TRUE;
  PauseGame();
  // prevent anyone from messing with the pause!
  LockPauseState(6);

  // Set NO_PROFILE info....
  for (cnt = 0; cnt < MAX_MEANWHILE_PROFILES; cnt++) {
    gNPCSaveData[cnt].ubProfile = NO_PROFILE;
  }

  return (TRUE);
}

void BringupMeanwhileBox() {
  wchar_t zStr[256];

#ifdef JA2TESTVERSION
  swprintf(
      zStr, ARR_SIZE(zStr),
      L"Meanwhile..... ( %S : Remember to make sure towns are controlled if required by script )",
      gzMeanwhileStr[gCurrentMeanwhileDef.ubMeanwhileID]);
#else
  swprintf(zStr, ARR_SIZE(zStr), L"%s.....", pMessageStrings[MSG_MEANWHILE]);
#endif

#ifdef JA2TESTVERSION
  if (gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION)
#else
  if (gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION &&
      MeanwhileSceneSeen(gCurrentMeanwhileDef.ubMeanwhileID))
#endif
  {
    DoMessageBox(MSG_BOX_BASIC_STYLE, zStr, guiCurrentScreen, MSG_BOX_FLAG_OKSKIP,
                 BeginMeanwhileCallBack, NULL);
  } else {
    DoMessageBox(MSG_BOX_BASIC_STYLE, zStr, guiCurrentScreen, (uint8_t)MSG_BOX_FLAG_OK,
                 BeginMeanwhileCallBack, NULL);
  }
}

void CheckForMeanwhileOKStart() {
  if (gfMeanwhileTryingToStart) {
    // Are we in prebattle interface?
    if (gfPreBattleInterfaceActive) {
      return;
    }

    if (!InterfaceOKForMeanwhilePopup()) {
      return;
    }

    if (!DialogueQueueIsEmptyOrSomebodyTalkingNow()) {
      return;
    }

    gfMeanwhileTryingToStart = FALSE;

    guiOldScreen = guiCurrentScreen;

    if (IsTacticalMode()) {
      LeaveTacticalScreen(GAME_SCREEN);
    }

    // We need to make sure we have no item - at least in tactical
    // In mapscreen, time is paused when manipulating items...
    CancelItemPointer();

    BringupMeanwhileBox();
  }
}

void StartMeanwhile() {
  int32_t iIndex;

  // OK, save old position...
  if (gfWorldLoaded) {
    gsOldSectorX = (uint8_t)gWorldSectorX;
    gsOldSectorY = (uint8_t)gWorldSectorY;
    gsOldSectorZ = (int8_t)gbWorldSectorZ;
  }

  gsOldSelectedSectorX = sSelMapX;
  gsOldSelectedSectorY = sSelMapY;
  gsOldSelectedSectorZ = iCurrentMapSectorZ;

  gfInMeanwhile = TRUE;

  // ATE: Change music before load
  SetMusicMode(MUSIC_MAIN_MENU);

  gfWorldWasLoaded = gfWorldLoaded;

  // OK, we have been told to start.....
  SetCurrentInterfacePanel((uint8_t)TEAM_PANEL);

  // Setup NPC locations, depending on meanwhile type...
  switch (gCurrentMeanwhileDef.ubMeanwhileID) {
    case END_OF_PLAYERS_FIRST_BATTLE:
    case DRASSEN_LIBERATED:
    case CAMBRIA_LIBERATED:
    case ALMA_LIBERATED:
    case GRUMM_LIBERATED:
    case CHITZENA_LIBERATED:
    case BALIME_LIBERATED:
    case NW_SAM:
    case NE_SAM:
    case CENTRAL_SAM:
    case FLOWERS:
    case LOST_TOWN:
    case CREATURES:
    case KILL_CHOPPER:
    case AWOL_SCIENTIST:
    case OUTSKIRTS_MEDUNA:

      // SAVE QUEEN!
      iIndex = GetFreeNPCSave();
      if (iIndex != -1) {
        gNPCSaveData[iIndex].ubProfile = QUEEN;
        gNPCSaveData[iIndex].sX = gMercProfiles[QUEEN].sSectorX;
        gNPCSaveData[iIndex].sY = gMercProfiles[QUEEN].sSectorY;
        gNPCSaveData[iIndex].sZ = gMercProfiles[QUEEN].bSectorZ;
        gNPCSaveData[iIndex].sGridNo = gMercProfiles[QUEEN].sGridNo;

        // Force reload of NPC files...
        ReloadQuoteFile(QUEEN);

        ChangeNpcToDifferentSector(QUEEN, 3, 16, 0);
      }

      // SAVE MESSANGER!
      iIndex = GetFreeNPCSave();
      if (iIndex != -1) {
        gNPCSaveData[iIndex].ubProfile = ELLIOT;
        gNPCSaveData[iIndex].sX = gMercProfiles[ELLIOT].sSectorX;
        gNPCSaveData[iIndex].sY = gMercProfiles[ELLIOT].sSectorY;
        gNPCSaveData[iIndex].sZ = gMercProfiles[ELLIOT].bSectorZ;
        gNPCSaveData[iIndex].sGridNo = gMercProfiles[ELLIOT].sGridNo;

        // Force reload of NPC files...
        ReloadQuoteFile(ELLIOT);

        ChangeNpcToDifferentSector(ELLIOT, 3, 16, 0);
      }

      if (gCurrentMeanwhileDef.ubMeanwhileID == OUTSKIRTS_MEDUNA) {
        // SAVE JOE!
        iIndex = GetFreeNPCSave();
        if (iIndex != -1) {
          gNPCSaveData[iIndex].ubProfile = JOE;
          gNPCSaveData[iIndex].sX = gMercProfiles[JOE].sSectorX;
          gNPCSaveData[iIndex].sY = gMercProfiles[JOE].sSectorY;
          gNPCSaveData[iIndex].sZ = gMercProfiles[JOE].bSectorZ;
          gNPCSaveData[iIndex].sGridNo = gMercProfiles[JOE].sGridNo;

          // Force reload of NPC files...
          ReloadQuoteFile(JOE);

          ChangeNpcToDifferentSector(JOE, 3, 16, 0);
        }
      }

      break;

    case INTERROGATION:

      // SAVE QUEEN!
      iIndex = GetFreeNPCSave();
      if (iIndex != -1) {
        gNPCSaveData[iIndex].ubProfile = QUEEN;
        gNPCSaveData[iIndex].sX = gMercProfiles[QUEEN].sSectorX;
        gNPCSaveData[iIndex].sY = gMercProfiles[QUEEN].sSectorY;
        gNPCSaveData[iIndex].sZ = gMercProfiles[QUEEN].bSectorZ;
        gNPCSaveData[iIndex].sGridNo = gMercProfiles[QUEEN].sGridNo;

        // Force reload of NPC files...
        ReloadQuoteFile(QUEEN);

        ChangeNpcToDifferentSector(QUEEN, 7, 14, 0);
      }

      // SAVE MESSANGER!
      iIndex = GetFreeNPCSave();
      if (iIndex != -1) {
        gNPCSaveData[iIndex].ubProfile = ELLIOT;
        gNPCSaveData[iIndex].sX = gMercProfiles[ELLIOT].sSectorX;
        gNPCSaveData[iIndex].sY = gMercProfiles[ELLIOT].sSectorY;
        gNPCSaveData[iIndex].sZ = gMercProfiles[ELLIOT].bSectorZ;
        gNPCSaveData[iIndex].sGridNo = gMercProfiles[ELLIOT].sGridNo;

        // Force reload of NPC files...
        ReloadQuoteFile(ELLIOT);

        ChangeNpcToDifferentSector(ELLIOT, 7, 14, 0);
      }

      // SAVE JOE!
      iIndex = GetFreeNPCSave();
      if (iIndex != -1) {
        gNPCSaveData[iIndex].ubProfile = JOE;
        gNPCSaveData[iIndex].sX = gMercProfiles[JOE].sSectorX;
        gNPCSaveData[iIndex].sY = gMercProfiles[JOE].sSectorY;
        gNPCSaveData[iIndex].sZ = gMercProfiles[JOE].bSectorZ;
        gNPCSaveData[iIndex].sGridNo = gMercProfiles[JOE].sGridNo;

        // Force reload of NPC files...
        ReloadQuoteFile(JOE);

        ChangeNpcToDifferentSector(JOE, 7, 14, 0);
      }

      break;
  }

  // fade out old screen....
  FadeOutNextFrame();

  // Load new map....
  gFadeOutDoneCallback = DoneFadeOutMeanwhile;
}

void DoneFadeOutMeanwhile() {
  // OK, insertion data found, enter sector!

  SetCurrentWorldSector((uint8_t)gCurrentMeanwhileDef.sSectorX,
                        (uint8_t)gCurrentMeanwhileDef.sSectorY, 0);

  // LocateToMeanwhileCharacter( );
  LocateMeanWhileGrid();

  gFadeInDoneCallback = DoneFadeInMeanwhile;

  FadeInNextFrame();
}

void DoneFadeInMeanwhile() {
  // ATE: double check that we are in meanwhile
  // this is if we cancel right away.....
  if (gfInMeanwhile) {
    giNPCReferenceCount = 1;

    if (gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION) {
      gTacticalStatus.uiFlags |= SHOW_ALL_MERCS;
    }

    TriggerNPCRecordImmediately(gCurrentMeanwhileDef.ubNPCNumber,
                                (uint8_t)gCurrentMeanwhileDef.usTriggerEvent);
  }
}

void BeginMeanwhileCallBack(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_OK || bExitValue == MSG_BOX_RETURN_YES) {
    gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
    // Increment reference count...
    giNPCReferenceCount = 1;

    StartMeanwhile();
  } else {
    // skipped scene!
    ProcessImplicationsOfMeanwhile();
    UnLockPauseState();
    UnPauseGame();
  }
}

BOOLEAN AreInMeanwhile() {
  STRATEGICEVENT *curr;

  // KM:  April 6, 1999
  // Tactical traversal needs to take precedence over meanwhile events.  When tactically traversing,
  // we expect to make it to the other side without interruption.
  if (gfTacticalTraversal) {
    return FALSE;
  }

  if (gfInMeanwhile) {
    return TRUE;
  }
  // Check to make sure a meanwhile scene isn't in the event list occurring at the exact same time
  // as this call.  Meanwhile scenes have precedence over a new battle if they occur in the same
  // second.
  curr = gpEventList;
  while (curr) {
    if (curr->uiTimeStamp == GetWorldTotalSeconds()) {
      if (curr->ubCallbackID == EVENT_MEANWHILE) {
        return TRUE;
      }
    } else {
      return FALSE;
    }
    curr = curr->next;
  }

  return (FALSE);
}

void ProcessImplicationsOfMeanwhile(void) {
  switch (gCurrentMeanwhileDef.ubMeanwhileID) {
    case END_OF_PLAYERS_FIRST_BATTLE:
      if (gGameOptions.ubDifficultyLevel ==
          DIF_LEVEL_HARD) {  // Wake up the queen earlier to punish the good players!
        ExecuteStrategicAIAction(STRATEGIC_AI_ACTION_WAKE_QUEEN, 0, 0);
      }
      HandleNPCDoAction(QUEEN, NPC_ACTION_SEND_SOLDIERS_TO_BATTLE_LOCATION, 0);
      break;
    case CAMBRIA_LIBERATED:
    case ALMA_LIBERATED:
    case GRUMM_LIBERATED:
    case CHITZENA_LIBERATED:
    case BALIME_LIBERATED:
      ExecuteStrategicAIAction(STRATEGIC_AI_ACTION_WAKE_QUEEN, 0, 0);
      break;
    case DRASSEN_LIBERATED:
      ExecuteStrategicAIAction(STRATEGIC_AI_ACTION_WAKE_QUEEN, 0, 0);
      HandleNPCDoAction(QUEEN, NPC_ACTION_SEND_SOLDIERS_TO_DRASSEN, 0);
      break;
    case CREATURES:
      // add Rat
      HandleNPCDoAction(QUEEN, NPC_ACTION_ADD_RAT, 0);
      break;
    case AWOL_SCIENTIST: {
      uint8_t sSectorX, sSectorY;

      StartQuest(QUEST_FIND_SCIENTIST, -1, -1);
      // place Madlab and robot!
      if (SectorInfo[GetSectorID8(7, MAP_ROW_H)].uiFlags & SF_USE_ALTERNATE_MAP) {
        sSectorX = 7;
        sSectorY = MAP_ROW_H;
      } else if (SectorInfo[GetSectorID8(16, MAP_ROW_H)].uiFlags & SF_USE_ALTERNATE_MAP) {
        sSectorX = 16;
        sSectorY = MAP_ROW_H;
      } else if (SectorInfo[GetSectorID8(11, MAP_ROW_I)].uiFlags & SF_USE_ALTERNATE_MAP) {
        sSectorX = 11;
        sSectorY = MAP_ROW_I;
      } else if (SectorInfo[GetSectorID8(4, MAP_ROW_E)].uiFlags & SF_USE_ALTERNATE_MAP) {
        sSectorX = 4;
        sSectorY = MAP_ROW_E;
      } else {
        Assert(0);
      }
      gMercProfiles[MADLAB].sSectorX = sSectorX;
      gMercProfiles[MADLAB].sSectorY = sSectorY;
      gMercProfiles[MADLAB].bSectorZ = 0;

      gMercProfiles[ROBOT].sSectorX = sSectorX;
      gMercProfiles[ROBOT].sSectorY = sSectorY;
      gMercProfiles[ROBOT].bSectorZ = 0;
    } break;
    case NW_SAM:
      ExecuteStrategicAIAction(NPC_ACTION_SEND_TROOPS_TO_SAM, SAM_1_X, SAM_1_Y);
      break;
    case NE_SAM:
      ExecuteStrategicAIAction(NPC_ACTION_SEND_TROOPS_TO_SAM, SAM_2_X, SAM_2_Y);
      break;
    case CENTRAL_SAM:
      ExecuteStrategicAIAction(NPC_ACTION_SEND_TROOPS_TO_SAM, SAM_3_X, SAM_3_X);
      break;

    default:
      break;
  }
}

void EndMeanwhile() {
  uint32_t cnt;
  uint8_t ubProfile;

  EmptyDialogueQueue();
  ProcessImplicationsOfMeanwhile();
  SetMeanwhileSceneSeen(gCurrentMeanwhileDef.ubMeanwhileID);

  gfInMeanwhile = FALSE;
  giNPCReferenceCount = 0;

  gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV);

  UnLockPauseState();
  UnPauseGame();

  // ATE: Make sure!
  TurnOffSectorLocator();

  if (gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION) {
    gTacticalStatus.uiFlags &= (~SHOW_ALL_MERCS);

    // OK, load old sector again.....
    FadeOutNextFrame();

    // Load new map....
    gFadeOutDoneCallback = DoneFadeOutMeanwhileOnceDone;
  } else {
    // We leave this sector open for our POWs to escape!
    // Set music mode to enemy present!
    SetMusicMode(MUSIC_TACTICAL_ENEMYPRESENT);

    // ATE: Restore people to saved positions...
    // OK, restore NPC save info...
    for (cnt = 0; cnt < guiNumNPCSaves; cnt++) {
      ubProfile = gNPCSaveData[cnt].ubProfile;

      if (ubProfile != NO_PROFILE) {
        gMercProfiles[ubProfile].sSectorX = gNPCSaveData[cnt].sX;
        gMercProfiles[ubProfile].sSectorY = gNPCSaveData[cnt].sY;
        gMercProfiles[ubProfile].bSectorZ = (int8_t)gNPCSaveData[cnt].sZ;
        gMercProfiles[ubProfile].sGridNo = (int8_t)gNPCSaveData[cnt].sGridNo;

        // Ensure NPC files loaded...
        ReloadQuoteFile(ubProfile);
      }
    }
  }
}

void DoneFadeOutMeanwhileOnceDone() {
  uint32_t cnt;
  uint8_t ubProfile;

  // OK, insertion data found, enter sector!
  gfReloadingScreenFromMeanwhile = TRUE;

  if (gfWorldWasLoaded) {
    SetCurrentWorldSector(gsOldSectorX, gsOldSectorY, (int8_t)gsOldSectorZ);

    ExamineCurrentSquadLights();
  } else {
    TrashWorld();
    // NB no world is loaded!
    gWorldSectorX = 0;
    gWorldSectorY = 0;
    gbWorldSectorZ = -1;
  }

  ChangeSelectedMapSector(gsOldSelectedSectorX, gsOldSelectedSectorY, (int8_t)gsOldSelectedSectorZ);

  gfReloadingScreenFromMeanwhile = FALSE;

  // OK, restore NPC save info...
  for (cnt = 0; cnt < guiNumNPCSaves; cnt++) {
    ubProfile = gNPCSaveData[cnt].ubProfile;

    if (ubProfile != NO_PROFILE) {
      gMercProfiles[ubProfile].sSectorX = gNPCSaveData[cnt].sX;
      gMercProfiles[ubProfile].sSectorY = gNPCSaveData[cnt].sY;
      gMercProfiles[ubProfile].bSectorZ = (int8_t)gNPCSaveData[cnt].sZ;
      gMercProfiles[ubProfile].sGridNo = (int8_t)gNPCSaveData[cnt].sGridNo;

      // Ensure NPC files loaded...
      ReloadQuoteFile(ubProfile);
    }
  }

  gFadeInDoneCallback = DoneFadeInMeanwhileOnceDone;

  // OK, based on screen we were in....
  switch (guiOldScreen) {
    case MAP_SCREEN:
      InternalLeaveTacticalScreen(MAP_SCREEN);
      break;

    case GAME_SCREEN:
      // restore old interface panel flag
      SetCurrentInterfacePanel((uint8_t)TEAM_PANEL);
      break;
  }

  FadeInNextFrame();
}

void DoneFadeInMeanwhileOnceDone() {}

void LocateMeanWhileGrid(void) {
  int16_t sGridNo = 0;

  // go to the approp. gridno
  sGridNo = gusMeanWhileGridNo[ubCurrentMeanWhileId];

  InternalLocateGridNo(sGridNo, TRUE);

  return;
}

void LocateToMeanwhileCharacter() {
  struct SOLDIERTYPE *pSoldier;

  if (gfInMeanwhile) {
    pSoldier = FindSoldierByProfileID(gCurrentMeanwhileDef.ubNPCNumber, FALSE);

    if (pSoldier != NULL) {
      LocateSoldier(pSoldier->ubID, FALSE);
    }
  }
}

BOOLEAN AreReloadingFromMeanwhile() { return (gfReloadingScreenFromMeanwhile); }

uint8_t GetMeanwhileID() { return (gCurrentMeanwhileDef.ubMeanwhileID); }

void HandleCreatureRelease(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  MeanwhileDef.ubMeanwhileID = CREATURES;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleMeanWhileEventPostingForTownLiberation(uint8_t bTownId) {
  // post event for meanwhile whithin the next 6 hours if it still will be daylight, otherwise the
  // next morning
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;
  uint8_t ubId = 0;
  BOOLEAN fHandled = FALSE;

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  // which town iberated?
  switch (bTownId) {
    case DRASSEN:
      ubId = DRASSEN_LIBERATED;
      fHandled = TRUE;
      break;
    case CAMBRIA:
      ubId = CAMBRIA_LIBERATED;
      fHandled = TRUE;
      break;
    case ALMA:
      ubId = ALMA_LIBERATED;
      fHandled = TRUE;
      break;
    case GRUMM:
      ubId = GRUMM_LIBERATED;
      fHandled = TRUE;
      break;
    case CHITZENA:
      ubId = CHITZENA_LIBERATED;
      fHandled = TRUE;
      break;
    case BALIME:
      ubId = BALIME_LIBERATED;
      fHandled = TRUE;
      break;
  }

  if (fHandled) {
    MeanwhileDef.ubMeanwhileID = ubId;

    // schedule the event
    ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
  }
}

void HandleMeanWhileEventPostingForTownLoss(uint8_t bTownId) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  // make sure scene hasn't been used before
  if (GetMeanWhileFlag(LOST_TOWN)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  MeanwhileDef.ubMeanwhileID = LOST_TOWN;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleMeanWhileEventPostingForSAMLiberation(int8_t bSamId) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;
  uint8_t ubId = 0;
  BOOLEAN fHandled = FALSE;

  if (bSamId == -1) {
    // invalid parameter!
    return;
  } else if (bSamId == 3) {
    // no meanwhile scene for this SAM site
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  // which SAM iberated?
  switch (bSamId) {
    case 0:
      ubId = NW_SAM;
      fHandled = TRUE;
      break;
    case 1:
      ubId = NE_SAM;
      fHandled = TRUE;
      break;
    case 2:
      ubId = CENTRAL_SAM;
      fHandled = TRUE;
      break;
    default:
      // wtf?
      break;
  }

  if (fHandled) {
    MeanwhileDef.ubMeanwhileID = ubId;

    // schedule the event
    ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
  }
}

void HandleFlowersMeanwhileScene(int8_t bTimeCode) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  // make sure scene hasn't been used before
  if (GetMeanWhileFlag(FLOWERS)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  // time delay should be based on time code, 0 next day, 1 seeral days (random)
  if (bTimeCode == 0) {
    // 20-24 hours later
    uiTime = GetWorldTotalMin() + 60 * (20 + Random(5));
  } else {
    // 2-4 days later
    uiTime = GetWorldTotalMin() + 60 * (24 + Random(48));
  }

  MeanwhileDef.ubMeanwhileID = FLOWERS;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleOutskirtsOfMedunaMeanwhileScene(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  // make sure scene hasn't been used before
  if (GetMeanWhileFlag(OUTSKIRTS_MEDUNA)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  MeanwhileDef.ubMeanwhileID = OUTSKIRTS_MEDUNA;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleKillChopperMeanwhileScene(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  // make sure scene hasn't been used before
  if (GetMeanWhileFlag(KILL_CHOPPER)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 55 + Random(10);

  MeanwhileDef.ubMeanwhileID = KILL_CHOPPER;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleScientistAWOLMeanwhileScene(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  // make sure scene hasn't been used before
  if (GetMeanWhileFlag(AWOL_SCIENTIST)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  MeanwhileDef.ubMeanwhileID = AWOL_SCIENTIST;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleInterrogationMeanwhileScene(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;

  // make sure scene hasn't been used before
  if (GetMeanWhileFlag(INTERROGATION)) {
    return;
  }

  MeanwhileDef.sSectorX = 7;  // what sector?
  MeanwhileDef.sSectorY = MAP_ROW_N;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 60;

  MeanwhileDef.ubMeanwhileID = INTERROGATION;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleFirstBattleVictory(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;
  uint8_t ubId = 0;

  if (GetMeanWhileFlag(END_OF_PLAYERS_FIRST_BATTLE)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  uiTime = GetWorldTotalMin() + 5;

  ubId = END_OF_PLAYERS_FIRST_BATTLE;

  MeanwhileDef.ubMeanwhileID = ubId;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleDelayedFirstBattleVictory(void) {
  uint32_t uiTime = 0;
  MEANWHILE_DEFINITION MeanwhileDef;
  uint8_t ubId = 0;

  if (GetMeanWhileFlag(END_OF_PLAYERS_FIRST_BATTLE)) {
    return;
  }

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;

  /*
  //It is theoretically impossible to liberate a town within 60 minutes of the first battle (which
  is supposed to
  //occur outside of a town in this scenario).  The delay is attributed to the info taking longer to
  reach the queen. uiTime = GetWorldTotalMin() + 60;
  */
  uiTime = GetWorldTotalMin() + 5;

  ubId = END_OF_PLAYERS_FIRST_BATTLE;

  MeanwhileDef.ubMeanwhileID = ubId;

  // schedule the event
  ScheduleMeanwhileEvent(&MeanwhileDef, uiTime);
}

void HandleFirstBattleEndingWhileInTown(uint8_t sSectorX, uint8_t sSectorY, int16_t bSectorZ,
                                        BOOLEAN fFromAutoResolve) {
  TownID bTownId = 0;
  int16_t sSector = 0;

  if (GetMeanWhileFlag(END_OF_PLAYERS_FIRST_BATTLE)) {
    return;
  }

  // if this is in fact a town and it is the first battle, then set
  // gfFirstBattleMeanwhileScenePending true if  is true then this is the end of the second battle,
  // post the first meanwhile OR, on call to trash world, that means player is leaving sector

  // grab sector value
  sSector = GetSectorID16(sSectorX, sSectorY);

  // get town name id
  bTownId = StrategicMap[sSector].townID;

  if (bTownId == BLANK_SECTOR) {
    // invalid town
    HandleDelayedFirstBattleVictory();
    gfFirstBattleMeanwhileScenePending = FALSE;
  } else if (gfFirstBattleMeanwhileScenePending || fFromAutoResolve) {
    HandleFirstBattleVictory();
    gfFirstBattleMeanwhileScenePending = FALSE;
  } else {
    gfFirstBattleMeanwhileScenePending = TRUE;
  }

  return;
}

void HandleFirstMeanWhileSetUpWithTrashWorld(void) {
  // exiting sector after first battle fought
  if (gfFirstBattleMeanwhileScenePending) {
    HandleFirstBattleVictory();
    gfFirstBattleMeanwhileScenePending = FALSE;
  }
}
