// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/Overhead.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Editor/EditorMercs.h"
#include "FadeScreen.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/History.h"
#include "MessageBoxScreen.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/GameInit.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMercHandler.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/StrategicTurns.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/AirRaid.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/AutoBandage.h"
#include "Tactical/Boxing.h"
#include "Tactical/Campaign.h"
#include "Tactical/CivQuotes.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/DrugsAndAlcohol.h"
#include "Tactical/EndGame.h"
#include "Tactical/FOV.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleUIPlan.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/Items.h"
#include "Tactical/Keys.h"
#include "Tactical/LOS.h"
#include "Tactical/Morale.h"
#include "Tactical/OppList.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierFunctions.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierTile.h"
#include "Tactical/Squads.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/Weapons.h"
#include "Tactical/WorldItems.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Smell.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

extern uint8_t gubAICounter;

#define RT_DELAY_BETWEEN_AI_HANDLING 50
#define RT_AI_TIMESLICE 10

int32_t giRTAILastUpdateTime = 0;
uint32_t guiAISlotToHandle = 0;
#define HANDLE_OFF_MAP_MERC 0xFFFF
#define RESET_HANDLE_OF_OFF_MAP_MERCS 0xFFFF
uint32_t guiAIAwaySlotToHandle = RESET_HANDLE_OF_OFF_MAP_MERCS;

#define PAUSE_ALL_AI_DELAY 1500

BOOLEAN gfPauseAllAI = FALSE;
int32_t giPauseAllAITimer = 0;

extern void RecalculateOppCntsDueToNoLongerNeutral(struct SOLDIERTYPE *pSoldier);
extern void SetSoldierAniSpeed(struct SOLDIERTYPE *pSoldier);
extern void HandleExplosionQueue(void);
extern void UpdateForContOverPortrait(struct SOLDIERTYPE *pSoldier, BOOLEAN fOn);
extern void HandleSystemNewAISituation(struct SOLDIERTYPE *pSoldier, BOOLEAN fResetABC);

extern BOOLEAN NPCInRoom(uint8_t ubProfileID, uint8_t ubRoomID);

extern int8_t gbInvalidPlacementSlot[NUM_INV_SLOTS];

void ResetAllMercSpeeds();
void HandleBloodForNewGridNo(struct SOLDIERTYPE *pSoldier);
BOOLEAN HandleAtNewGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving);
void DoCreatureTensionQuote(struct SOLDIERTYPE *pSoldier);
void HandleCreatureTenseQuote();

void RemoveSoldierFromTacticalSector(struct SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected);
void HandleEndDemoInCreatureLevel();
void DeathTimerCallback(void);
void CaptureTimerCallback(void);

extern void CheckForAlertWhenEnemyDies(struct SOLDIERTYPE *pDyingSoldier);
extern void PlaySoldierFootstepSound(struct SOLDIERTYPE *pSoldier);
extern void HandleKilledQuote(struct SOLDIERTYPE *pKilledSoldier,
                              struct SOLDIERTYPE *pKillerSoldier, int16_t sGridNo, int8_t bLevel);
extern uint16_t PickSoldierReadyAnimation(struct SOLDIERTYPE *pSoldier, BOOLEAN fEndReady);

extern void PlayStealthySoldierFootstepSound(struct SOLDIERTYPE *pSoldier);

extern BOOLEAN gfSurrendered;

// GLOBALS
#define START_DEMO_SCENE 3
#define NUM_RANDOM_SCENES 4

#ifdef NETWORKED
extern uint8_t gfAmIHost;
extern BOOLEAN gfAmINetworked;
#endif

#define NEW_FADE_DELAY 60

// ATE: GLOBALS FOR E3
uint8_t gubCurrentScene = 0;
char gzLevelFilenames[][50] = {
    "A9.dat",         "ScotTBMines.dat",  "LindaTBCaves.dat", "LindaRTDesert.dat",
    "IanRTNight.dat", "LindaRTCave1.dat", "LindaRTCave2.dat"

};

TacticalStatusType gTacticalStatus;

int8_t ubLevelMoveLink[10] = {1, 2, 3, 4, 0, 0, 0, 0, 0, 0};

// Soldier List used for all soldier overhead interaction
struct SOLDIERTYPE Menptr[TOTAL_SOLDIERS];
struct SOLDIERTYPE *MercPtrs[TOTAL_SOLDIERS];

struct SOLDIERTYPE *MercSlots[TOTAL_SOLDIERS];
uint32_t guiNumMercSlots = 0;

struct SOLDIERTYPE *AwaySlots[TOTAL_SOLDIERS];
uint32_t guiNumAwaySlots = 0;

// DEF: changed to have client wait for gPlayerNum assigned from host
uint8_t gbPlayerNum = 0;

// Global for current selected soldier
uint16_t gusSelectedSoldier = NO_SOLDIER;
int8_t gbShowEnemies = FALSE;

BOOLEAN gfMovingAnimation = FALSE;

char gzAlertStr[][30] = {"GREEN", "YELLOW", "RED", "BLACK"};

char gzActionStr[][30] = {
    "NONE",

    "RANDOM PATROL",
    "SEEK FRIEND",
    "SEEK OPPONENT",
    "TAKE COVER",
    "GET CLOSER",

    "POINT PATROL",
    "LEAVE WATER GAS",
    "SEEK NOISE",
    "ESCORTED MOVE",
    "RUN AWAY",

    "KNIFE MOVE",
    "APPROACH MERC",
    "TRACK",
    "EAT",
    "PICK UP ITEM",

    "SCHEDULE MOVE",
    "WALK",
    "RUN",
    "MOVE TO CLIMB",
    "CHG FACING",

    "CHG STANCE",
    "YELLOW ALERT",
    "RED ALERT",
    "CREATURE CALL",
    "PULL TRIGGER",

    "USE DETONATOR",
    "FIRE GUN",
    "TOSS PROJECTILE",
    "KNIFE STAB",
    "THROW KNIFE",

    "GIVE AID",
    "WAIT",
    "PENDING ACTION",
    "DROP ITEM",
    "COWER",

    "STOP COWERING",
    "OPEN/CLOSE DOOR",
    "UNLOCK DOOR",
    "LOCK DOOR",
    "LOWER GUN",

    "ABSOLUTELY NONE",
    "CLIMB ROOF",
    "END TURN",
    "EC&M",
    "TRAVERSE DOWN",
    "OFFER SURRENDER",
};

char gzDirectionStr[][30] = {"NORTHEAST", "EAST", "SOUTHEAST", "SOUTH",
                             "SOUTHWEST", "WEST", "NORTHWEST", "NORTH"};

// TEMP VALUES FOR TEAM DEAFULT POSITIONS
uint8_t bDefaultTeamRanges[MAXTEAMS][2] = {
    {0, 19},                                // 20  US
    {20, 51},                               // 32  ENEMY
    {52, 83},                               // 32    CREATURE
    {84, 115},                              // 32    REBELS ( OUR GUYS )
    {116, MAX_NUM_SOLDIERS - 1},            // 32  CIVILIANS
    {MAX_NUM_SOLDIERS, TOTAL_SOLDIERS - 1}  // PLANNING SOLDIERS
};

COLORVAL bDefaultTeamColors[MAXTEAMS] = {FROMRGB(255, 255, 0),   FROMRGB(255, 0, 0),
                                         FROMRGB(255, 0, 255),   FROMRGB(0, 255, 0),
                                         FROMRGB(255, 255, 255), FROMRGB(0, 0, 255)};

// UTILITY FUNCTIONS
int8_t NumActiveAndConsciousTeamMembers(uint8_t ubTeam);
uint8_t NumEnemyInSector();
uint8_t NumEnemyInSectorExceptCreatures();
uint8_t NumCapableEnemyInSector();

BOOLEAN KillIncompacitatedEnemyInSector();
BOOLEAN CheckForLosingEndOfBattle();
void EndBattleWithUnconsciousGuysCallback(uint8_t bExitValue);
uint8_t NumEnemyInSectorNotDeadOrDying();
uint8_t NumBloodcatsInSectorNotDeadOrDying();

uint8_t gubWaitingForAllMercsToExitCode = 0;
int8_t gbNumMercsUntilWaitingOver = 0;
uint32_t guiWaitingForAllMercsToExitData[3];
uint32_t guiWaitingForAllMercsToExitTimer = 0;
BOOLEAN gfKillingGuysForLosingBattle = FALSE;

int32_t GetFreeMercSlot(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumMercSlots; uiCount++) {
    if ((MercSlots[uiCount] == NULL)) return ((int32_t)uiCount);
  }

  if (guiNumMercSlots < TOTAL_SOLDIERS) return ((int32_t)guiNumMercSlots++);

  return (-1);
}

void RecountMercSlots(void) {
  int32_t iCount;

  if (guiNumMercSlots > 0) {
    // set equal to 0 as a default
    for (iCount = guiNumMercSlots - 1; (iCount >= 0); iCount--) {
      if ((MercSlots[iCount] != NULL)) {
        guiNumMercSlots = (uint32_t)(iCount + 1);
        return;
      }
    }
    // no mercs found
    guiNumMercSlots = 0;
  }
}

int32_t AddMercSlot(struct SOLDIERTYPE *pSoldier) {
  int32_t iMercIndex;

  if ((iMercIndex = GetFreeMercSlot()) == (-1)) return (-1);

  MercSlots[iMercIndex] = pSoldier;

  return (iMercIndex);
}

BOOLEAN RemoveMercSlot(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiCount;

  CHECKF(pSoldier != NULL);

  for (uiCount = 0; uiCount < guiNumMercSlots; uiCount++) {
    if (MercSlots[uiCount] == pSoldier) {
      MercSlots[uiCount] = NULL;
      RecountMercSlots();
      return (TRUE);
    }
  }

  // TOLD TO DELETE NON-EXISTANT SOLDIER
  return (FALSE);
}

int32_t GetFreeAwaySlot(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumAwaySlots; uiCount++) {
    if ((AwaySlots[uiCount] == NULL)) return ((int32_t)uiCount);
  }

  if (guiNumAwaySlots < TOTAL_SOLDIERS) return ((int32_t)guiNumAwaySlots++);

  return (-1);
}

void RecountAwaySlots(void) {
  int32_t iCount;

  if (guiNumAwaySlots > 0) {
    for (iCount = guiNumAwaySlots - 1; (iCount >= 0); iCount--) {
      if ((AwaySlots[iCount] != NULL)) {
        guiNumAwaySlots = (uint32_t)(iCount + 1);
        return;
      }
    }
    // no mercs found
    guiNumAwaySlots = 0;
  }
}

int32_t AddAwaySlot(struct SOLDIERTYPE *pSoldier) {
  int32_t iAwayIndex;

  if ((iAwayIndex = GetFreeAwaySlot()) == (-1)) return (-1);

  AwaySlots[iAwayIndex] = pSoldier;

  return (iAwayIndex);
}

BOOLEAN RemoveAwaySlot(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiCount;

  CHECKF(pSoldier != NULL);

  for (uiCount = 0; uiCount < guiNumAwaySlots; uiCount++) {
    if (AwaySlots[uiCount] == pSoldier) {
      AwaySlots[uiCount] = NULL;
      RecountAwaySlots();
      return (TRUE);
    }
  }

  // TOLD TO DELETE NON-EXISTANT SOLDIER
  return (FALSE);
}

int32_t MoveSoldierFromMercToAwaySlot(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fRet;

  fRet = RemoveMercSlot(pSoldier);
  if (!fRet) {
    return (-1);
  }

  if (!(pSoldier->uiStatusFlags & SOLDIER_OFF_MAP)) {
    RemoveManFromTeam(pSoldier->bTeam);
  }

  pSoldier->bInSector = FALSE;
  pSoldier->uiStatusFlags |= SOLDIER_OFF_MAP;
  return (AddAwaySlot(pSoldier));
}

int32_t MoveSoldierFromAwayToMercSlot(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fRet;

  fRet = RemoveAwaySlot(pSoldier);
  if (!fRet) {
    return (-1);
  }

  AddManToTeam(pSoldier->bTeam);

  pSoldier->bInSector = TRUE;
  pSoldier->uiStatusFlags &= (~SOLDIER_OFF_MAP);
  return (AddMercSlot(pSoldier));
}

BOOLEAN InitTacticalEngine() {
  // Init renderer
  InitRenderParams(0);

  // Init dirty queue system
  InitializeBaseDirtyRectQueue();

  // Init Interface stuff
  InitializeTacticalInterface();

  // Init system objects
  InitializeGameVideoObjects();

  // Init palette system
  LoadPaletteData();

  if (!LoadLockTable()) return (FALSE);

  InitInteractiveTileManagement();

  // init path code
  if (!InitPathAI()) {
    return (FALSE);
  }

  // init AI
  if (!InitAI()) {
    return (FALSE);
  }

  // Init Overhead
  if (!InitOverhead()) {
    return (FALSE);
  }

#ifdef NETWORKED
  if (!gfAmINetworked) gfAmIHost = TRUE;
#endif

  return (TRUE);
}

void ShutdownTacticalEngine() {
  DeletePaletteData();

  ShutdownStaticExternalNPCFaces();

  ShutDownPathAI();
  ShutdownInteractiveTileManagement();
  UnLoadCarPortraits();

  ShutdownNPCQuotes();
}

BOOLEAN InitOverhead() {
  uint32_t cnt;
  uint8_t cnt2;

  memset(MercSlots, 0, sizeof(MercSlots));
  memset(AwaySlots, 0, sizeof(AwaySlots));

  // Set pointers list
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    MercPtrs[cnt] = GetSoldierByID(cnt);
    MercPtrs[cnt]->bActive = FALSE;
  }

  memset(&gTacticalStatus, 0, sizeof(TacticalStatusType));

  // Set team values
  for (cnt = 0; cnt < MAXTEAMS; cnt++) {
    // For now, set hard-coded values
    gTacticalStatus.Team[cnt].bFirstID = bDefaultTeamRanges[cnt][0];
    gTacticalStatus.Team[cnt].bLastID = bDefaultTeamRanges[cnt][1];
    gTacticalStatus.Team[cnt].RadarColor = bDefaultTeamColors[cnt];

    if (cnt == gbPlayerNum || cnt == PLAYER_PLAN) {
      SetTeamSide(cnt, 0);
      gTacticalStatus.Team[cnt].bHuman = TRUE;
    } else {
      if (cnt == MILITIA_TEAM) {
        // militia guys on our side!
        SetTeamSide(cnt, 0);
      } else if (cnt == CREATURE_TEAM) {
        // creatures are on no one's side but their own
        // NB side 2 is used for hostile rebels....
        SetTeamSide(cnt, 3);
      } else {
        // hostile (enemies, or civilians; civs are potentially hostile but neutral)
        SetTeamSide(cnt, 1);
      }
      gTacticalStatus.Team[cnt].bHuman = FALSE;
    }

    gTacticalStatus.Team[cnt].ubLastMercToRadio = NOBODY;
    gTacticalStatus.Team[cnt].bTeamActive = FALSE;
    gTacticalStatus.Team[cnt].bAwareOfOpposition = FALSE;

    // set team values in soldier structures for all who are on this team
    for (cnt2 = gTacticalStatus.Team[cnt].bFirstID; cnt2 <= gTacticalStatus.Team[cnt].bLastID;
         cnt2++) {
      MercPtrs[cnt2]->bTeam = (int8_t)cnt;
    }
  }

  // Zero out merc slots!
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    MercSlots[cnt] = NULL;
  }

  // Set other tactical flags
  gTacticalStatus.uiFlags = TURNBASED | TRANSLUCENCY_TYPE;
  gTacticalStatus.sSlideTarget = NOWHERE;
  gTacticalStatus.uiTimeOfLastInput = GetJA2Clock();
  gTacticalStatus.uiTimeSinceDemoOn = GetJA2Clock();
  gTacticalStatus.uiCountdownToRestart = GetJA2Clock();
  gTacticalStatus.fGoingToEnterDemo = FALSE;
  gTacticalStatus.fNOTDOLASTDEMO = FALSE;
  gTacticalStatus.fDidGameJustStart = TRUE;
  gTacticalStatus.ubLastRequesterTargetID = NO_PROFILE;

  for (cnt = 0; cnt < NUM_PANIC_TRIGGERS; cnt++) {
    gTacticalStatus.sPanicTriggerGridNo[cnt] = NOWHERE;
  }
  /*	for ( cnt = 0; cnt < NUM_TOPTIONS; cnt++ )
          {
                  gGameSettings.fOptions[ cnt ] = 1;
          }

          gGameSettings.fOptions[ TOPTION_RTCONFIRM ] = 0;
          gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] = 0;
  */
  gTacticalStatus.bRealtimeSpeed = MAX_REALTIME_SPEED_VAL / 2;

  gfInAirRaid = FALSE;
  gpCustomizableTimerCallback = NULL;

  // Reset cursor
  gpItemPointer = NULL;
  gpItemPointerSoldier = NULL;
  memset(gbInvalidPlacementSlot, 0, sizeof(gbInvalidPlacementSlot));

  InitCivQuoteSystem();

  ZeroAnimSurfaceCounts();

  return (TRUE);
}

BOOLEAN ShutdownOverhead() {
  uint32_t cnt;

  // Delete any soldiers which have been created!
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    if (MercPtrs[cnt] != NULL) {
      if (MercPtrs[cnt]->bActive) {
        DeleteSoldier(MercPtrs[cnt]);
      }
    }
  }

  return (TRUE);
}

BOOLEAN GetSoldier(struct SOLDIERTYPE **ppSoldier, uint16_t usSoldierIndex) {
  // Check range of index given
  *ppSoldier = NULL;

  if (usSoldierIndex < 0 || usSoldierIndex > TOTAL_SOLDIERS - 1) {
    // Set debug message
    return (FALSE);
  }

  // Check if a guy exists here
  // Does another soldier exist here?
  if (MercPtrs[usSoldierIndex]->bActive) {
    // Set Existing guy
    *ppSoldier = MercPtrs[usSoldierIndex];
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN NextAIToHandle(uint32_t uiCurrAISlot) {
  uint32_t cnt;

  if (uiCurrAISlot >= guiNumMercSlots) {
    // last person to handle was an off-map merc, so now we start looping at the beginning
    // again
    cnt = 0;
  } else {
    // continue on from the last person we handled
    cnt = uiCurrAISlot + 1;
  }

  for (; cnt < guiNumMercSlots; cnt++) {
    if (MercSlots[cnt] && ((MercSlots[cnt]->bTeam != gbPlayerNum) ||
                           (MercSlots[cnt]->uiStatusFlags & SOLDIER_PCUNDERAICONTROL))) {
      // aha! found an AI guy!
      guiAISlotToHandle = cnt;
      return (TRUE);
    }
  }

  // set so that even if there are no off-screen mercs to handle, we will loop back to
  // the start of the array
  guiAISlotToHandle = HANDLE_OFF_MAP_MERC;

  // didn't find an AI guy to handle after the last one handled and the # of slots
  // it's time to check for an off-map merc... maybe
  if (guiNumAwaySlots > 0) {
    if ((guiAIAwaySlotToHandle + 1) >= guiNumAwaySlots) {
      // start looping from the beginning
      cnt = 0;
    } else {
      // continue on from the last person we handled
      cnt = guiAIAwaySlotToHandle + 1;
    }

    for (; cnt < guiNumAwaySlots; cnt++) {
      if (AwaySlots[cnt] && AwaySlots[cnt]->bTeam != gbPlayerNum) {
        // aha! found an AI guy!
        guiAIAwaySlotToHandle = cnt;
        return (FALSE);
      }
    }

    // reset awayAISlotToHandle, but DON'T loop again, away slots not that important
    guiAIAwaySlotToHandle = RESET_HANDLE_OF_OFF_MAP_MERCS;
  }

  return (FALSE);
}

void PauseAITemporarily(void) {
  gfPauseAllAI = TRUE;
  giPauseAllAITimer = GetJA2Clock();
}

void PauseAIUntilManuallyUnpaused(void) {
  gfPauseAllAI = TRUE;
  giPauseAllAITimer = 0;
}

void UnPauseAI(void) {
  // overrides any timer too
  gfPauseAllAI = FALSE;
  giPauseAllAITimer = 0;
}

float gdRadiansForAngle[] = {
    (float)PI, (float)(3 * PI / 4), (float)(PI / 2),  (float)((PI) / 4),

    (float)0,  (float)((-PI) / 4),  (float)(-PI / 2), (float)(-3 * PI / 4),

};

BOOLEAN ExecuteOverhead() {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  int16_t sAPCost;
  int16_t sBPCost;
  float dXPos, dYPos;
  float dAngle;
  BOOLEAN fKeepMoving;
  int8_t bShadeLevel;
  BOOLEAN fNoAPsForPendingAction;
  int16_t sGridNo;
  struct STRUCTURE *pStructure;
  BOOLEAN fHandleAI = FALSE;

  // Diagnostic Stuff
  static int32_t iTimerTest = 0;
  static int32_t iTimerVal = 0;

  gfMovingAnimation = FALSE;

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    if (IsSolActive(pSoldier)) {
      if (pSoldier->uiStatusFlags & SOLDIER_GREEN_RAY)
        LightShowRays((int16_t)(pSoldier->dXPos / CELL_X_SIZE),
                      (int16_t)(pSoldier->dYPos / CELL_Y_SIZE), FALSE);
    }
  }

  if (COUNTERDONE(TOVERHEAD)) {
    // Reset counter
    RESETCOUNTER(TOVERHEAD);

    // Diagnostic Stuff
    iTimerVal = GetJA2Clock();
    giTimerDiag = iTimerVal - iTimerTest;
    iTimerTest = iTimerVal;

    // ANIMATED TILE STUFF
    UpdateAniTiles();

    // BOMBS!!!
    HandleExplosionQueue();

    HandleCreatureTenseQuote();

    CheckHostileOrSayQuoteList();

    if (gfPauseAllAI && giPauseAllAITimer && (iTimerVal - giPauseAllAITimer > PAUSE_ALL_AI_DELAY)) {
      // ok, stop pausing the AI!
      gfPauseAllAI = FALSE;
    }

    if (!gfPauseAllAI) {
      // AI limiting crap
      gubAICounter = 0;
      if (!((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT))) {
        if ((iTimerVal - giRTAILastUpdateTime) > RT_DELAY_BETWEEN_AI_HANDLING) {
          giRTAILastUpdateTime = iTimerVal;
          // figure out which AI guy to handle this time around,
          // starting with the slot AFTER the current AI guy
          fHandleAI = NextAIToHandle(guiAISlotToHandle);
        }
      }
    }

    for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
      pSoldier = MercSlots[cnt];

      if (pSoldier != NULL) {
        HandlePanelFaceAnimations(pSoldier);

        // Handle damage counters
        if (pSoldier->fDisplayDamage) {
          if (TIMECOUNTERDONE(pSoldier->DamageCounter, DAMAGE_DISPLAY_DELAY)) {
            pSoldier->bDisplayDamageCount++;
            pSoldier->sDamageX += 1;
            pSoldier->sDamageY -= 1;

            RESETTIMECOUNTER(pSoldier->DamageCounter, DAMAGE_DISPLAY_DELAY);
          }

          if (pSoldier->bDisplayDamageCount >= 8) {
            pSoldier->bDisplayDamageCount = 0;
            pSoldier->sDamage = 0;
            pSoldier->fDisplayDamage = FALSE;
          }
        }

        // Handle reload counters
        if (pSoldier->fReloading) {
          if (TIMECOUNTERDONE(pSoldier->ReloadCounter, pSoldier->sReloadDelay)) {
            pSoldier->fReloading = FALSE;
            pSoldier->fPauseAim = FALSE;
            /*
            DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - realtime
            reloading") ); FreeUpAttacker( GetSolID(pSoldier) );
            */
          }
        }

        // Checkout fading
        if (pSoldier->fBeginFade) {
          if (TIMECOUNTERDONE(pSoldier->FadeCounter, NEW_FADE_DELAY)) {
            RESETTIMECOUNTER(pSoldier->FadeCounter, NEW_FADE_DELAY);

            // Fade out....
            if (pSoldier->fBeginFade == 1) {
              bShadeLevel = (pSoldier->ubFadeLevel & 0x0f);
              bShadeLevel = min(bShadeLevel + 1, SHADE_MIN);

              if (bShadeLevel >= (SHADE_MIN - 3)) {
                pSoldier->fBeginFade = FALSE;
                pSoldier->bVisible = -1;

                // Set levelnode shade level....
                if (pSoldier->pLevelNode) {
                  pSoldier->pLevelNode->ubShadeLevel = bShadeLevel;
                }

                // Set Anim speed accordingly!
                SetSoldierAniSpeed(pSoldier);
              }

              bShadeLevel |= (pSoldier->ubFadeLevel & 0x30);
              pSoldier->ubFadeLevel = bShadeLevel;
            } else if (pSoldier->fBeginFade == 2) {
              bShadeLevel = (pSoldier->ubFadeLevel & 0x0f);
              // ubShadeLevel =max(ubShadeLevel-1, gpWorldLevelData[ pSoldier->sGridNo
              // ].pLandHead->ubShadeLevel );

              bShadeLevel = bShadeLevel - 1;

              if (bShadeLevel <= 0) {
                bShadeLevel = 0;
              }

              if (bShadeLevel <= (gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel)) {
                bShadeLevel = (gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel);

                pSoldier->fBeginFade = FALSE;
                // pSoldier->bVisible = -1;
                // pSoldier->ubFadeLevel = gpWorldLevelData[ pSoldier->sGridNo
                // ].pLandHead->ubShadeLevel;

                // Set levelnode shade level....
                if (pSoldier->pLevelNode) {
                  pSoldier->pLevelNode->ubShadeLevel = bShadeLevel;
                }

                // Set Anim speed accordingly!
                SetSoldierAniSpeed(pSoldier);
              }

              bShadeLevel |= (pSoldier->ubFadeLevel & 0x30);
              pSoldier->ubFadeLevel = bShadeLevel;
            }
          }
        }

        // Check if we have a new visiblity and shade accordingly down
        if (pSoldier->bLastRenderVisibleValue != pSoldier->bVisible) {
          HandleCrowShadowVisibility(pSoldier);

          // Check for fade out....
          if (pSoldier->bVisible == -1 && pSoldier->bLastRenderVisibleValue >= 0) {
            if (pSoldier->sGridNo != NOWHERE) {
              pSoldier->ubFadeLevel = gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;
            } else {
            }
            pSoldier->fBeginFade = TRUE;
            pSoldier->sLocationOfFadeStart = pSoldier->sGridNo;

            // OK, re-evaluate guy's roof marker
            HandlePlacingRoofMarker(pSoldier, pSoldier->sGridNo, FALSE, FALSE);

            pSoldier->bVisible = -2;
          }

          // Check for fade in.....
          if (pSoldier->bVisible != -1 && pSoldier->bLastRenderVisibleValue == -1 &&
              pSoldier->bTeam != gbPlayerNum) {
            pSoldier->ubFadeLevel = (SHADE_MIN - 3);
            pSoldier->fBeginFade = 2;
            pSoldier->sLocationOfFadeStart = pSoldier->sGridNo;

            // OK, re-evaluate guy's roof marker
            HandlePlacingRoofMarker(pSoldier, pSoldier->sGridNo, TRUE, FALSE);
          }
        }
        pSoldier->bLastRenderVisibleValue = pSoldier->bVisible;

        // Handle stationary polling...
        if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY) ||
            pSoldier->fNoAPToFinishMove) {
          // Are are stationary....
          // Were we once moving...?
          if (pSoldier->fSoldierWasMoving && pSoldier->bVisible > -1) {
            pSoldier->fSoldierWasMoving = FALSE;

            HandlePlacingRoofMarker(pSoldier, pSoldier->sGridNo, TRUE, FALSE);

            if (!gGameSettings.fOptions[TOPTION_MERC_ALWAYS_LIGHT_UP]) {
              DeleteSoldierLight(pSoldier);

              SetCheckSoldierLightFlag(pSoldier);
            }
          }
        } else {
          // We are moving....
          // Were we once stationary?
          if (!pSoldier->fSoldierWasMoving) {
            pSoldier->fSoldierWasMoving = TRUE;

            HandlePlacingRoofMarker(pSoldier, pSoldier->sGridNo, FALSE, FALSE);
          }
        }

        // Handle animation update counters
        // ATE: Added additional check here for special value of anispeed that pauses all updates
        if (TIMECOUNTERDONE(pSoldier->UpdateCounter, pSoldier->sAniDelay) &&
            pSoldier->sAniDelay != 10000) {
#ifdef NETWORKED
          // DEF:
          // Check for TIMING delay here only if in Realtime
          if (gTacticalStatus.uiFlags & REALTIME)
            if (pSoldier->fIsSoldierMoving) CheckForSlowSoldier(pSoldier);
#endif

          // Check if we need to look for items
          if (pSoldier->uiStatusFlags & SOLDIER_LOOKFOR_ITEMS) {
            RevealRoofsAndItems(pSoldier, TRUE, FALSE, pSoldier->bLevel, FALSE);
            pSoldier->uiStatusFlags &= (~SOLDIER_LOOKFOR_ITEMS);
          }

          // Check if we need to reposition light....
          if (pSoldier->uiStatusFlags & SOLDIER_RECHECKLIGHT) {
            PositionSoldierLight(pSoldier);
            pSoldier->uiStatusFlags &= (~SOLDIER_RECHECKLIGHT);
          }

          RESETTIMECOUNTER(pSoldier->UpdateCounter, pSoldier->sAniDelay);

          fNoAPsForPendingAction = FALSE;

#ifdef NETWORKED
          // Get the path update, if there is 1
          if (pSoldier->fSoldierUpdatedFromNetwork) UpdateSoldierFromNetwork(pSoldier);
#endif

          // Check if we are moving and we deduct points and we have no points
          if (!((gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_MOVING | ANIM_SPECIALMOVE)) &&
                pSoldier->fNoAPToFinishMove) &&
              !pSoldier->fPauseAllAnimation) {
            if (!AdjustToNextAnimationFrame(pSoldier)) {
              continue;
            }

            if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE)) {
              // Check if we are waiting for an opened path
              HandleNextTileWaiting(pSoldier);
            }

            // Update world data with new position, etc
            // Determine gameworld cells corrds of guy
            if (gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_MOVING | ANIM_SPECIALMOVE) &&
                !(pSoldier->uiStatusFlags & SOLDIER_PAUSEANIMOVE)) {
              fKeepMoving = TRUE;

              pSoldier->fPausedMove = FALSE;

              // CHECK TO SEE IF WE'RE ON A MIDDLE TILE
              if (pSoldier->fPastXDest && pSoldier->fPastYDest) {
                pSoldier->fPastXDest = pSoldier->fPastYDest = FALSE;
                // assign X/Y values back to make sure we are at the center of the tile
                // (to prevent mercs from going through corners of tiles and producing
                // structure data complaints)

                // pSoldier->dXPos = pSoldier->sDestXPos;
                // pSoldier->dYPos = pSoldier->sDestYPos;

                HandleBloodForNewGridNo(pSoldier);

                if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE) &&
                    pSoldier->sGridNo != pSoldier->sFinalDestination) {
                } else {
                  // OK, we're at the MIDDLE of a new tile...
                  HandleAtNewGridNo(pSoldier, &fKeepMoving);
                }

                if (gTacticalStatus.bBoxingState != NOT_BOXING &&
                    (gTacticalStatus.bBoxingState == BOXING_WAITING_FOR_PLAYER ||
                     gTacticalStatus.bBoxingState == PRE_BOXING ||
                     gTacticalStatus.bBoxingState == BOXING)) {
                  BoxingMovementCheck(pSoldier);
                }

                // Are we at our final destination?
                if (pSoldier->sFinalDestination == pSoldier->sGridNo) {
                  // Cancel path....
                  pSoldier->usPathIndex = pSoldier->usPathDataSize = 0;

                  // Cancel reverse
                  pSoldier->bReverse = FALSE;

                  // OK, if we are the selected soldier, refresh some UI stuff
                  if (pSoldier->ubID == (uint8_t)gusSelectedSoldier) {
                    gfUIRefreshArrows = TRUE;
                  }

                  // ATE: Play landing sound.....
                  if (pSoldier->usAnimState == JUMP_OVER_BLOCKING_PERSON) {
                    PlaySoldierFootstepSound(pSoldier);
                  }

                  // If we are a robot, play stop sound...
                  if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
                    PlaySoldierJA2Sample(pSoldier->ubID, ROBOT_STOP, RATE_11025,
                                         SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
                                         SoundDir(pSoldier->sGridNo), TRUE);
                  }

                  // Update to middle if we're on destination
                  dXPos = pSoldier->sDestXPos;
                  dYPos = pSoldier->sDestYPos;
                  EVENT_SetSoldierPosition(pSoldier, dXPos, dYPos);
#ifdef NETWORKED
                  // DEF: Test Code
                  StopSoldierMovementTime(pSoldier);
#endif

                  // Handle New sight
                  // HandleSight(pSoldier,SIGHT_LOOK | SIGHT_RADIO );

                  // CHECK IF WE HAVE A PENDING ANIMATION
                  if (pSoldier->usPendingAnimation != NO_PENDING_ANIMATION) {
                    ChangeSoldierState(pSoldier, pSoldier->usPendingAnimation, 0, FALSE);
                    pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;

                    if (pSoldier->ubPendingDirection != NO_PENDING_DIRECTION) {
                      EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->ubPendingDirection);
                      pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
                    }
                  }

                  // CHECK IF WE HAVE A PENDING ACTION
                  if (pSoldier->ubWaitActionToDo) {
                    if (pSoldier->ubWaitActionToDo == 2) {
                      pSoldier->ubWaitActionToDo = 1;

                      if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN) {
                        // ATE wanted this line here...
                        pSoldier->usPathIndex--;
                        AdjustSoldierPathToGoOffEdge(pSoldier, pSoldier->sGridNo,
                                                     (uint8_t)pSoldier->uiPendingActionData1);
                        continue;
                      }
                    } else if (pSoldier->ubWaitActionToDo == 1) {
                      pSoldier->ubWaitActionToDo = 0;

                      gbNumMercsUntilWaitingOver--;

                      SoldierGotoStationaryStance(pSoldier);

                      // If we are at an exit-grid, make disappear.....
                      if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO) {
                        // Remove!
                        RemoveSoldierFromTacticalSector(pSoldier, TRUE);
                      }
                    }
                  } else if (pSoldier->ubPendingAction != NO_PENDING_ACTION) {
                    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                             String("We are inside the IF PENDING Animation with soldier #%d",
                                    GetSolID(pSoldier)));

                    if (pSoldier->ubPendingAction == MERC_OPENDOOR ||
                        pSoldier->ubPendingAction == MERC_OPENSTRUCT) {
                      sGridNo = pSoldier->sPendingActionData2;
                      // usStructureID           = (uint16_t)pSoldier->uiPendingActionData1;
                      // pStructure = FindStructureByID( sGridNo, usStructureID );

                      // LOOK FOR STRUCT OPENABLE
                      pStructure = FindStructure(sGridNo, STRUCTURE_OPENABLE);

                      if (pStructure == NULL) {
#ifdef JA2BETAVERSION
                        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
                                  L"Told to open struct at %d and none was found", sGridNo);
#endif
                        fKeepMoving = FALSE;
                      } else {
                        CalcInteractiveObjectAPs(sGridNo, pStructure, &sAPCost, &sBPCost);

                        if (EnoughPoints(pSoldier, sAPCost, sBPCost, TRUE)) {
                          InteractWithInteractiveObject(pSoldier, pStructure,
                                                        pSoldier->bPendingActionData3);
                        } else {
                          fNoAPsForPendingAction = TRUE;
                        }
                      }
                    }
                    if (pSoldier->ubPendingAction == MERC_PICKUPITEM) {
                      sGridNo = pSoldier->sPendingActionData2;

                      if (sGridNo == pSoldier->sGridNo) {
                        // OK, now, if in realtime
                        if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
                          // If the two gridnos are not the same, check to see if we can
                          // now go into it
                          if (sGridNo != (int16_t)pSoldier->uiPendingActionData4) {
                            if (NewOKDestination(pSoldier, (int16_t)pSoldier->uiPendingActionData4,
                                                 TRUE, pSoldier->bLevel)) {
                              // GOTO NEW TILE!
                              SoldierPickupItem(pSoldier, pSoldier->uiPendingActionData1,
                                                (int16_t)pSoldier->uiPendingActionData4,
                                                pSoldier->bPendingActionData3);
                              continue;
                            }
                          }
                        }

                        // OK MORON, double CHECK THAT THE ITEM EXISTS HERE...
                        if (pSoldier->uiPendingActionData1 != ITEM_PICKUP_ACTION_ALL) {
                          // if ( ItemExistsAtLocation( (int16_t)( pSoldier->uiPendingActionData4 ),
                          // pSoldier->uiPendingActionData1, pSoldier->bLevel ) )
                          {
                            PickPickupAnimation(pSoldier, pSoldier->uiPendingActionData1,
                                                (int16_t)(pSoldier->uiPendingActionData4),
                                                pSoldier->bPendingActionData3);
                          }
                        } else {
                          PickPickupAnimation(pSoldier, pSoldier->uiPendingActionData1,
                                              (int16_t)(pSoldier->uiPendingActionData4),
                                              pSoldier->bPendingActionData3);
                        }
                      } else {
                        SoldierGotoStationaryStance(pSoldier);
                      }
                    } else if (pSoldier->ubPendingAction == MERC_PUNCH) {
                      // for the benefit of the AI
                      pSoldier->bAction = AI_ACTION_KNIFE_STAB;

                      EVENT_SoldierBeginPunchAttack(pSoldier, pSoldier->sPendingActionData2,
                                                    pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_TALK) {
                      PlayerSoldierStartTalking(pSoldier, (uint8_t)pSoldier->uiPendingActionData1,
                                                TRUE);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_DROPBOMB) {
                      EVENT_SoldierBeginDropBomb(pSoldier);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_STEAL) {
                      // pSoldier->bDesiredDirection = pSoldier->bPendingActionData3;
                      EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->bPendingActionData3);

                      EVENT_InitNewSoldierAnim(pSoldier, STEAL_ITEM, 0, FALSE);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_KNIFEATTACK) {
                      // for the benefit of the AI
                      pSoldier->bAction = AI_ACTION_KNIFE_STAB;

                      EVENT_SoldierBeginBladeAttack(pSoldier, pSoldier->sPendingActionData2,
                                                    pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_GIVEAID) {
                      EVENT_SoldierBeginFirstAid(pSoldier, pSoldier->sPendingActionData2,
                                                 pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_REPAIR) {
                      EVENT_SoldierBeginRepair(pSoldier, pSoldier->sPendingActionData2,
                                               pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_FUEL_VEHICLE) {
                      EVENT_SoldierBeginRefuel(pSoldier, pSoldier->sPendingActionData2,
                                               pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_RELOADROBOT) {
                      EVENT_SoldierBeginReloadRobot(pSoldier, pSoldier->sPendingActionData2,
                                                    pSoldier->bPendingActionData3,
                                                    (int8_t)pSoldier->uiPendingActionData1);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_TAKEBLOOD) {
                      EVENT_SoldierBeginTakeBlood(pSoldier, pSoldier->sPendingActionData2,
                                                  pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_ATTACH_CAN) {
                      EVENT_SoldierBeginAttachCan(pSoldier, pSoldier->sPendingActionData2,
                                                  pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_ENTER_VEHICLE) {
                      EVENT_SoldierEnterVehicle(pSoldier, pSoldier->sPendingActionData2,
                                                pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                      continue;
                    } else if (pSoldier->ubPendingAction == MERC_CUTFFENCE) {
                      EVENT_SoldierBeginCutFence(pSoldier, pSoldier->sPendingActionData2,
                                                 pSoldier->bPendingActionData3);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    } else if (pSoldier->ubPendingAction == MERC_GIVEITEM) {
                      EVENT_SoldierBeginGiveItem(pSoldier);
                      pSoldier->ubPendingAction = NO_PENDING_ACTION;
                    }

                    if (fNoAPsForPendingAction) {
                      // Change status of guy to waiting
                      HaltMoveForSoldierOutOfPoints(pSoldier);
                      fKeepMoving = FALSE;
                      pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
                      pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
                    }

                  } else {
                    // OK, ADJUST TO STANDING, WE ARE DONE
                    // DO NOTHING IF WE ARE UNCONSCIOUS
                    if (pSoldier->bLife >= OKLIFE) {
                      if (pSoldier->ubBodyType == CROW) {
                        // If we are flying, don't stop!
                        if (pSoldier->sHeightAdjustment == 0) {
                          SoldierGotoStationaryStance(pSoldier);
                        }
                      } else {
                        UnSetUIBusy(pSoldier->ubID);

                        SoldierGotoStationaryStance(pSoldier);
                      }
                    }
                  }

                  // RESET MOVE FAST FLAG
                  if ((GetSolProfile(pSoldier) == NO_PROFILE)) {
                    pSoldier->fUIMovementFast = FALSE;
                  }

                  // if AI moving and waiting to process something at end of
                  // move, have them handled the very next frame
                  if (pSoldier->ubQuoteActionID == QUOTE_ACTION_ID_CHECKFORDEST) {
                    pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;
                  }

                  fKeepMoving = FALSE;
                } else if (!pSoldier->fNoAPToFinishMove) {
                  // Increment path....
                  pSoldier->usPathIndex++;

                  if (pSoldier->usPathIndex > pSoldier->usPathDataSize) {
                    pSoldier->usPathIndex = pSoldier->usPathDataSize;
                  }

                  // Are we at the end?
                  if (pSoldier->usPathIndex == pSoldier->usPathDataSize) {
                    // ATE: Pop up warning....
                    if (pSoldier->usPathDataSize != MAX_PATH_LIST_SIZE) {
#ifdef JA2BETAVERSION
                      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
                                L"Path for %s ( %d ) did not make merc get to dest .",
                                pSoldier->name, GetSolID(pSoldier));
#endif
                    }

                    // In case this is an AI person with the path-stored flag set,
                    // turn it OFF since we have exhausted our stored path
                    pSoldier->bPathStored = FALSE;
                    if (pSoldier->sAbsoluteFinalDestination != NOWHERE) {
                      // We have not made it to our dest... but it's better to let the AI handle
                      // this itself, on the very next fram
                      pSoldier->fAIFlags |= AI_HANDLE_EVERY_FRAME;
                    } else {
                      // ATE: Added this to fcalilitate the fact
                      // that our final dest may now have people on it....
                      if (FindBestPath(pSoldier, pSoldier->sFinalDestination, pSoldier->bLevel,
                                       pSoldier->usUIMovementMode, NO_COPYROUTE,
                                       PATH_THROUGH_PEOPLE) != 0) {
                        int16_t sNewGridNo;

                        sNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo,
                                               DirectionInc((uint8_t)guiPathingData[0]));

                        SetDelayedTileWaiting(pSoldier, sNewGridNo, 1);
                      }

                      // We have not made it to our dest... set flag that we are waiting....
                      if (!EVENT_InternalGetNewSoldierPath(pSoldier, pSoldier->sFinalDestination,
                                                           pSoldier->usUIMovementMode, 2, FALSE)) {
                        // ATE: To do here.... we could not get path, so we have to stop
                        SoldierGotoStationaryStance(pSoldier);
                        continue;
                      }
                    }
                  } else {
                    // OK, Now find another dest grindo....
                    if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE)) {
                      // OK, now we want to see if we can continue to another tile...
                      if (!HandleGotoNewGridNo(pSoldier, &fKeepMoving, FALSE,
                                               pSoldier->usAnimState)) {
                        continue;
                      }
                    } else {
                      // Change desired direction
                      // Just change direction
                      EVENT_InternalSetSoldierDestination(
                          pSoldier, pSoldier->usPathingData[pSoldier->usPathIndex], FALSE,
                          pSoldier->usAnimState);
                    }

                    if (gTacticalStatus.bBoxingState != NOT_BOXING &&
                        (gTacticalStatus.bBoxingState == BOXING_WAITING_FOR_PLAYER ||
                         gTacticalStatus.bBoxingState == PRE_BOXING ||
                         gTacticalStatus.bBoxingState == BOXING)) {
                      BoxingMovementCheck(pSoldier);
                    }
                  }
                }
              }

              if ((pSoldier->uiStatusFlags & SOLDIER_PAUSEANIMOVE)) {
                fKeepMoving = FALSE;
              }

              // DO WALKING
              if (!pSoldier->fPausedMove && fKeepMoving) {
                // Determine deltas
                //	dDeltaX = pSoldier->sDestXPos - pSoldier->dXPos;
                // dDeltaY = pSoldier->sDestYPos - pSoldier->dYPos;

                // Determine angle
                //	dAngle = (float)atan2( dDeltaX, dDeltaY );

                dAngle = gdRadiansForAngle[pSoldier->bMovementDirection];

                // For walking, base it on body type!
                if (pSoldier->usAnimState == WALKING) {
                  MoveMerc(pSoldier, gubAnimWalkSpeeds[pSoldier->ubBodyType].dMovementChange,
                           dAngle, TRUE);

                } else {
                  MoveMerc(pSoldier, gAnimControl[pSoldier->usAnimState].dMovementChange, dAngle,
                           TRUE);
                }
              }
            }

            // Check for direction change
            if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_TURNING) {
              TurnSoldier(pSoldier);
            }
          }

#ifdef NETWORKED
          if (!pSoldier->fNoAPToFinishMove) pSoldier->usLastUpdateTime = GetJA2Clock();
          if (pSoldier->fSoldierUpdatedFromNetwork) UpdateSoldierFromNetwork(pSoldier);
#endif
        }

        if (!gfPauseAllAI &&
            (((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) ||
             (fHandleAI && guiAISlotToHandle == cnt) ||
             (pSoldier->fAIFlags & AI_HANDLE_EVERY_FRAME) || gTacticalStatus.fAutoBandageMode)) {
          HandleSoldierAI(pSoldier);
          if (!((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT))) {
            if (GetJA2Clock() - iTimerVal > RT_AI_TIMESLICE) {
              // don't do any more AI this time!
              fHandleAI = FALSE;
            } else {
              // we still have time to handle AI; skip to the next person
              fHandleAI = NextAIToHandle(guiAISlotToHandle);
            }
          }
        }
      }
    }

    if (guiNumAwaySlots > 0 && !gfPauseAllAI && !(gTacticalStatus.uiFlags & INCOMBAT) &&
        guiAISlotToHandle == HANDLE_OFF_MAP_MERC &&
        guiAIAwaySlotToHandle != RESET_HANDLE_OF_OFF_MAP_MERCS) {
      pSoldier = AwaySlots[guiAIAwaySlotToHandle];

      if (pSoldier != NULL) {
        // the ONLY thing to do with away soldiers is process their schedule if they have one
        // and there is an action for them to do (like go on-sector)
        if (pSoldier->fAIFlags & AI_CHECK_SCHEDULE) {
          HandleSoldierAI(pSoldier);
        }
      }
    }

    // Turn off auto bandage if we need to...
    if (gTacticalStatus.fAutoBandageMode) {
      if (!CanAutoBandage(TRUE)) {
        SetAutoBandageComplete();
      }
    }

    // Check if we should be doing a special event once guys get to a location...
    if (gubWaitingForAllMercsToExitCode != 0) {
      // Check if we have gone past our time...
      if ((GetJA2Clock() - guiWaitingForAllMercsToExitTimer) > 2500) {
// OK, set num waiting to 0
#ifdef JA2BETAVERSION
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_DEBUG,
                  L"Waiting too long for Mercs to exit...forcing entry.");
#endif
        gbNumMercsUntilWaitingOver = 0;

        // Reset all waitng codes
        for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
          pSoldier = MercSlots[cnt];
          if (pSoldier != NULL) {
            pSoldier->ubWaitActionToDo = 0;
          }
        }
      }

      if (gbNumMercsUntilWaitingOver == 0) {
        // ATE: Unset flag to ignore sight...
        gTacticalStatus.uiFlags &= (~DISALLOW_SIGHT);

        // OK cheif, do something here....
        switch (gubWaitingForAllMercsToExitCode) {
          case WAIT_FOR_MERCS_TO_WALKOFF_SCREEN:

            if ((gTacticalStatus.ubCurrentTeam == gbPlayerNum)) {
              guiPendingOverrideEvent = LU_ENDUILOCK;
              HandleTacticalUI();
            }
            AllMercsHaveWalkedOffSector();
            break;

          case WAIT_FOR_MERCS_TO_WALKON_SCREEN:

            // OK, unset UI
            if ((gTacticalStatus.ubCurrentTeam == gbPlayerNum)) {
              guiPendingOverrideEvent = LU_ENDUILOCK;
              HandleTacticalUI();
            }
            break;

          case WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO:

            // OK, unset UI
            if ((gTacticalStatus.ubCurrentTeam == gbPlayerNum)) {
              guiPendingOverrideEvent = LU_ENDUILOCK;
              HandleTacticalUI();
            }
            AllMercsWalkedToExitGrid();
            break;
        }

        // ATE; Turn off tactical status flag...
        gTacticalStatus.uiFlags &= (~IGNORE_ALL_OBSTACLES);

        gubWaitingForAllMercsToExitCode = 0;
      }
    }
  }

  // reset these AI-related global variables to 0 to ensure they don't interfere with the UI
  gubNPCAPBudget = 0;
  gubNPCDistLimit = 0;

  return (TRUE);
}

void HaltGuyFromNewGridNoBecauseOfNoAPs(struct SOLDIERTYPE *pSoldier) {
  HaltMoveForSoldierOutOfPoints(pSoldier);
  pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
  pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
  pSoldier->ubPendingAction = NO_PENDING_ACTION;

  UnMarkMovementReserved(pSoldier);

  // Display message if our merc...
  if (pSoldier->bTeam == gbPlayerNum && (gTacticalStatus.uiFlags & INCOMBAT)) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[GUY_HAS_RUN_OUT_OF_APS_STR],
              pSoldier->name);
  }

  UnSetUIBusy(pSoldier->ubID);

  // OK, Unset engaged in CONV, something changed...
  UnSetEngagedInConvFromPCAction(pSoldier);
}

void HandleLocateToGuyAsHeWalks(struct SOLDIERTYPE *pSoldier) {
  // Our guys if option set,
  if (pSoldier->bTeam == gbPlayerNum) {
    // IF tracking on, center on guy....
    if (gGameSettings.fOptions[TOPTION_TRACKING_MODE]) {
      LocateSoldier(pSoldier->ubID, FALSE);
    }
  } else {
    // Others if visible...
    if (pSoldier->bVisible != -1) {
      // ATE: If we are visible, and have not already removed roofs, goforit
      if (pSoldier->bLevel > 0) {
        if (!(gTacticalStatus.uiFlags & SHOW_ALL_ROOFS)) {
          gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
          SetRenderFlags(RENDER_FLAG_FULL);
        }
      }

      LocateSoldier(pSoldier->ubID, FALSE);
    }
  }
}

BOOLEAN HandleGotoNewGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving,
                            BOOLEAN fInitialMove, uint16_t usAnimState) {
  int16_t sAPCost;
  int16_t sBPCost;
  uint16_t usNewGridNo, sOverFenceGridNo, sMineGridNo;

  if (gTacticalStatus.uiFlags & INCOMBAT && fInitialMove) {
    HandleLocateToGuyAsHeWalks(pSoldier);
  }

  // Default to TRUE
  (*pfKeepMoving) = TRUE;

  // Check for good breath....
  // if ( pSoldier->bBreath < OKBREATH && !fInitialMove )
  if (pSoldier->bBreath < OKBREATH) {
    // OK, first check for b== 0
    // If our currentd state is moving already....( this misses the first tile, so the user
    // Sees some change in their click, but just one tile
    if (pSoldier->bBreath == 0) {
      // Collapse!
      pSoldier->bBreathCollapsed = TRUE;
      pSoldier->bEndDoorOpenCode = FALSE;

      if (fInitialMove) {
        UnSetUIBusy(pSoldier->ubID);
      }

      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Out of Breath"));
      return (FALSE);
    }

    // OK, if we are collapsed now, check for OK breath instead...
    if (pSoldier->bCollapsed) {
      // Collapse!
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Has Collapsed"));
      pSoldier->bBreathCollapsed = TRUE;
      pSoldier->bEndDoorOpenCode = FALSE;
      return (FALSE);
    }
  }

  usNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo,
                          DirectionInc((uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex]));

  // OK, check if this is a fence cost....
  if (gubWorldMovementCosts[usNewGridNo][(uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex]]
                           [pSoldier->bLevel] == TRAVELCOST_FENCE) {
    // We have been told to jump fence....

    // Do we have APs?
    sAPCost = AP_JUMPFENCE;
    sBPCost = BP_JUMPFENCE;

    if (EnoughPoints(pSoldier, sAPCost, sBPCost, FALSE)) {
      // ATE: Check for tile being clear....
      sOverFenceGridNo = NewGridNo(
          usNewGridNo, DirectionInc((uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex + 1]));

      if (HandleNextTile(pSoldier, (int8_t)pSoldier->usPathingData[pSoldier->usPathIndex + 1],
                         sOverFenceGridNo, pSoldier->sFinalDestination)) {
        // We do, adjust path data....
        pSoldier->usPathIndex++;
        // We go two, because we really want to start moving towards the NEXT gridno,
        // if we have any...

        // LOCK PENDING ACTION COUNTER
        pSoldier->uiStatusFlags |= SOLDIER_LOCKPENDINGACTIONCOUNTER;

        SoldierGotoStationaryStance(pSoldier);

        // OK, jump!
        BeginSoldierClimbFence(pSoldier);

        pSoldier->fContinueMoveAfterStanceChange = 2;
      }

    } else {
      HaltGuyFromNewGridNoBecauseOfNoAPs(pSoldier);
      (*pfKeepMoving) = FALSE;
    }

    return (FALSE);
  } else if (InternalDoorTravelCost(
                 pSoldier, usNewGridNo,
                 gubWorldMovementCosts[usNewGridNo]
                                      [(uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex]]
                                      [pSoldier->bLevel],
                 (BOOLEAN)(pSoldier->bTeam == gbPlayerNum), NULL, TRUE) == TRAVELCOST_DOOR) {
    struct STRUCTURE *pStructure;
    int8_t bDirection;
    int16_t sDoorGridNo;

    // OK, if we are here, we have been told to get a pth through a door.

    // No need to check if for AI

    // No need to check for right key ( since the path checks for that? )

    // Just for now play the $&&% animation
    bDirection = (uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex];

    // OK, based on the direction, get door gridno
    if (bDirection == NORTH || bDirection == WEST) {
      sDoorGridNo =
          NewGridNo((uint16_t)pSoldier->sGridNo,
                    DirectionInc((uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex]));
    } else if (bDirection == SOUTH || bDirection == EAST) {
      sDoorGridNo = pSoldier->sGridNo;
    } else {
#ifdef JA2TESTVERSION
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
                L"ERROR: Invalid Direction to approach door. (Soldier loc: %d, dir: %d).",
                pSoldier->sGridNo, bDirection);
#endif
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("HandleGotoNewGridNo() Failed: Open door - invalid approach direction"));

      HaltGuyFromNewGridNoBecauseOfNoAPs(pSoldier);
      pSoldier->bEndDoorOpenCode = FALSE;
      (*pfKeepMoving) = FALSE;
      return (FALSE);
    }

    // Get door
    pStructure = FindStructure(sDoorGridNo, STRUCTURE_ANYDOOR);

    if (pStructure == NULL) {
#ifdef JA2TESTVERSION
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
                L"ERROR: Told to open door that does not exist at %d.", sDoorGridNo);
#endif
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: Door does not exist"));
      HaltGuyFromNewGridNoBecauseOfNoAPs(pSoldier);
      pSoldier->bEndDoorOpenCode = FALSE;
      (*pfKeepMoving) = FALSE;
      return (FALSE);
    }

    // OK, open!
    StartInteractiveObject(sDoorGridNo, pStructure->usStructureID, pSoldier, bDirection);
    InteractWithInteractiveObject(pSoldier, pStructure, bDirection);

    // One needs to walk after....
    if ((pSoldier->bTeam != gbPlayerNum) || (gTacticalStatus.fAutoBandageMode) ||
        (pSoldier->uiStatusFlags & SOLDIER_PCUNDERAICONTROL)) {
      pSoldier->bEndDoorOpenCode = 1;
      pSoldier->sEndDoorOpenCodeData = sDoorGridNo;
    }
    (*pfKeepMoving) = FALSE;
    return (FALSE);
  }

  // Find out how much it takes to move here!
  sAPCost = ActionPointCost(pSoldier, usNewGridNo,
                            (int8_t)pSoldier->usPathingData[pSoldier->usPathIndex], usAnimState);
  sBPCost = TerrainBreathPoints(
      pSoldier, usNewGridNo, (int8_t)pSoldier->usPathingData[pSoldier->usPathIndex], usAnimState);

  // CHECK IF THIS TILE IS A GOOD ONE!
  if (!HandleNextTile(pSoldier, (int8_t)pSoldier->usPathingData[pSoldier->usPathIndex], usNewGridNo,
                      pSoldier->sFinalDestination)) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("HandleGotoNewGridNo() Failed: Tile %d Was blocked", usNewGridNo));

    // ATE: If our own guy and an initial move.. display message
    // if ( fInitialMove && pSoldier->bTeam == gbPlayerNum  )
    //{
    //	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ NO_PATH_FOR_MERC ],
    // pSoldier->name );
    //}

    pSoldier->bEndDoorOpenCode = FALSE;
    // GO on to next guy!
    return (FALSE);
  }

  // just check the tile we're going to walk into
  if (NearbyGroundSeemsWrong(pSoldier, usNewGridNo, FALSE, &sMineGridNo)) {
    if (pSoldier->uiStatusFlags & SOLDIER_PC) {
      // NearbyGroundSeemsWrong returns true with gridno NOWHERE if
      // we find something by metal detector... we should definitely stop
      // but we won't place a locator or say anything

      // IF not in combat, stop them all
      if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
        int32_t cnt2;
        struct SOLDIERTYPE *pSoldier2;

        cnt2 = gTacticalStatus.Team[gbPlayerNum].bLastID;

        // look for all mercs on the same team,
        for (pSoldier2 = MercPtrs[cnt2]; cnt2 >= gTacticalStatus.Team[gbPlayerNum].bFirstID;
             cnt2--, pSoldier2--) {
          if (pSoldier2->bActive) {
            EVENT_StopMerc(pSoldier2, pSoldier2->sGridNo, pSoldier2->bDirection);
          }
        }
      } else {
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
      }

      (*pfKeepMoving) = FALSE;

      if (sMineGridNo != NOWHERE) {
        LocateGridNo(sMineGridNo);
        // we reuse the boobytrap gridno variable here
        gsBoobyTrapGridNo = sMineGridNo;
        gpBoobyTrapSoldier = pSoldier;
        SetStopTimeQuoteCallback(MineSpottedDialogueCallBack);
        TacticalCharacterDialogue(pSoldier, QUOTE_SUSPICIOUS_GROUND);
      }
    } else {
      if (sMineGridNo != NOWHERE) {
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
        (*pfKeepMoving) = FALSE;

        gpWorldLevelData[sMineGridNo].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;

        // better stop and reconsider what to do...
        SetNewSituation(pSoldier);
        ActionDone(pSoldier);
      }
    }
  }

  // ATE: Check if we have sighted anyone, if so, don't do anything else...
  // IN other words, we have stopped from sighting...
  if (pSoldier->fNoAPToFinishMove && !fInitialMove) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("HandleGotoNewGridNo() Failed: No APs to finish move set"));
    pSoldier->bEndDoorOpenCode = FALSE;
    (*pfKeepMoving) = FALSE;
  } else if (pSoldier->usPathIndex == pSoldier->usPathDataSize && pSoldier->usPathDataSize == 0) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("HandleGotoNewGridNo() Failed: No Path"));
    pSoldier->bEndDoorOpenCode = FALSE;
    (*pfKeepMoving) = FALSE;
  }
  // else if ( gTacticalStatus.fEnemySightingOnTheirTurn )
  //{
  // Hault guy!
  //	AdjustNoAPToFinishMove( pSoldier, TRUE );
  //	(*pfKeepMoving ) = FALSE;
  //}
  else if (EnoughPoints(pSoldier, sAPCost, 0, FALSE)) {
    BOOLEAN fDontContinue = FALSE;

    if (pSoldier->usPathIndex > 0) {
      // check for running into gas

      // note: this will have to use the minimum types of structures for tear/creature gas
      // since there isn't a way to retrieve the smoke effect structure
      if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] & ANY_SMOKE_EFFECT &&
          PreRandom(5) == 0) {
        EXPLOSIVETYPE *pExplosive = NULL;
        int8_t bPosOfMask;

        if (pSoldier->inv[HEAD1POS].usItem == GASMASK &&
            pSoldier->inv[HEAD1POS].bStatus[0] >= GASMASK_MIN_STATUS) {
          bPosOfMask = HEAD1POS;
        } else if (pSoldier->inv[HEAD2POS].usItem == GASMASK &&
                   pSoldier->inv[HEAD2POS].bStatus[0] >= GASMASK_MIN_STATUS) {
          bPosOfMask = HEAD2POS;
        } else {
          bPosOfMask = NO_SLOT;
        }

        if (!AM_A_ROBOT(pSoldier)) {
          if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] &
              MAPELEMENT_EXT_TEARGAS) {
            if (!(pSoldier->fHitByGasFlags & HIT_BY_TEARGAS) && bPosOfMask == NO_SLOT) {
              // check for gas mask
              pExplosive = &(Explosive[Item[TEARGAS_GRENADE].ubClassIndex]);
            }
          }
          if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] &
              MAPELEMENT_EXT_MUSTARDGAS) {
            if (!(pSoldier->fHitByGasFlags & HIT_BY_MUSTARDGAS) && bPosOfMask == NO_SLOT) {
              pExplosive = &(Explosive[Item[MUSTARD_GRENADE].ubClassIndex]);
            }
          }
        }
        if (gpWorldLevelData[pSoldier->sGridNo].ubExtFlags[pSoldier->bLevel] &
            MAPELEMENT_EXT_CREATUREGAS) {
          if (!(pSoldier->fHitByGasFlags &
                HIT_BY_CREATUREGAS))  // gas mask doesn't help vs creaturegas
          {
            pExplosive = &(Explosive[Item[SMALL_CREATURE_GAS].ubClassIndex]);
          }
        }
        if (pExplosive) {
          EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
          fDontContinue = TRUE;

          DishOutGasDamage(
              pSoldier, pExplosive, TRUE, FALSE,
              (int16_t)(pExplosive->ubDamage + (uint8_t)PreRandom(pExplosive->ubDamage)),
              (int16_t)(100 * (pExplosive->ubStunDamage +
                               (int16_t)PreRandom((pExplosive->ubStunDamage / 2)))),
              NOBODY);
        }
      }

      if (!fDontContinue) {
        if ((pSoldier->bOverTerrainType == FLAT_FLOOR ||
             pSoldier->bOverTerrainType == PAVED_ROAD) &&
            pSoldier->bLevel == 0) {
          int32_t iMarblesIndex;

          if (ItemTypeExistsAtLocation(pSoldier->sGridNo, MARBLES, 0, &iMarblesIndex)) {
            // Slip on marbles!
            DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
            if (pSoldier->bTeam == gbPlayerNum) {
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[STR_SLIPPED_MARBLES],
                        pSoldier->name);
            }
            RemoveItemFromPool(pSoldier->sGridNo, iMarblesIndex, 0);
            SoldierCollapse(pSoldier);
            if (pSoldier->bActionPoints > 0) {
              pSoldier->bActionPoints -= (int8_t)(Random(pSoldier->bActionPoints) + 1);
            }
            return (FALSE);
          }
        }

        if ((pSoldier->bBlindedCounter > 0) && (pSoldier->usAnimState == RUNNING) &&
            (Random(5) == 0) &&
            OKFallDirection(pSoldier,
                            (int16_t)(pSoldier->sGridNo + DirectionInc(pSoldier->bDirection)),
                            pSoldier->bLevel, pSoldier->bDirection, pSoldier->usAnimState)) {
          // 20% chance of falling over!
          DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[37], pSoldier->name);
          SoldierCollapse(pSoldier);
          if (pSoldier->bActionPoints > 0) {
            pSoldier->bActionPoints -= (int8_t)(Random(pSoldier->bActionPoints) + 1);
          }
          return (FALSE);
        } else if ((GetDrunkLevel(pSoldier) == DRUNK) && (Random(5) == 0) &&
                   OKFallDirection(
                       pSoldier, (int16_t)(pSoldier->sGridNo + DirectionInc(pSoldier->bDirection)),
                       pSoldier->bLevel, pSoldier->bDirection, pSoldier->usAnimState)) {
          // 20% chance of falling over!
          DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[37], pSoldier->name);
          SoldierCollapse(pSoldier);
          if (pSoldier->bActionPoints > 0) {
            pSoldier->bActionPoints -= (int8_t)(Random(pSoldier->bActionPoints) + 1);
          }
          return (FALSE);
        } else
          // ATE; First check for profile
          // Forgetful guy might forget his path
          if ((pSoldier->bTeam == gbPlayerNum) && (GetSolProfile(pSoldier) != NO_PROFILE) &&
              gMercProfiles[GetSolProfile(pSoldier)].bPersonalityTrait == FORGETFUL) {
            if (pSoldier->ubNumTilesMovesSinceLastForget < 255) {
              pSoldier->ubNumTilesMovesSinceLastForget++;
            }

            if (pSoldier->usPathIndex > 2 && (Random(100) == 0) &&
                pSoldier->ubNumTilesMovesSinceLastForget > 200) {
              pSoldier->ubNumTilesMovesSinceLastForget = 0;

              TacticalCharacterDialogue(pSoldier, QUOTE_PERSONALITY_TRAIT);
              EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
              if (pSoldier->bActionPoints > 0) {
                pSoldier->bActionPoints -= (int8_t)(Random(pSoldier->bActionPoints) + 1);
              }

              fDontContinue = TRUE;

              UnSetUIBusy(pSoldier->ubID);
            }
          }
      }
    }

    if (!fDontContinue) {
      // Don't apply the first deduction in points...
      if (usAnimState == CRAWLING && pSoldier->fTurningFromPronePosition > 1) {
      } else {
        // Adjust AP/Breathing points to move
        DeductPoints(pSoldier, sAPCost, sBPCost);
      }

      // OK, let's check for monsters....
      if (pSoldier->uiStatusFlags & SOLDIER_MONSTER) {
        if (!ValidCreatureTurn(pSoldier,
                               (int8_t)(pSoldier->usPathingData[pSoldier->usPathIndex]))) {
          if (!pSoldier->bReverse) {
            pSoldier->bReverse = TRUE;

            if (pSoldier->ubBodyType == INFANT_MONSTER) {
              ChangeSoldierState(pSoldier, WALK_BACKWARDS, 1, TRUE);
            } else {
              ChangeSoldierState(pSoldier, MONSTER_WALK_BACKWARDS, 1, TRUE);
            }
          }
        } else {
          pSoldier->bReverse = FALSE;
        }
      }

      // OK, let's check for monsters....
      if (pSoldier->ubBodyType == BLOODCAT) {
        if (!ValidCreatureTurn(pSoldier,
                               (int8_t)(pSoldier->usPathingData[pSoldier->usPathIndex]))) {
          if (!pSoldier->bReverse) {
            pSoldier->bReverse = TRUE;
            ChangeSoldierState(pSoldier, BLOODCAT_WALK_BACKWARDS, 1, TRUE);
          }
        } else {
          pSoldier->bReverse = FALSE;
        }
      }

      // Change desired direction
      EVENT_InternalSetSoldierDestination(pSoldier, pSoldier->usPathingData[pSoldier->usPathIndex],
                                          fInitialMove, usAnimState);

      // CONTINUE
      // IT'S SAVE TO GO AGAIN, REFRESH flag
      AdjustNoAPToFinishMove(pSoldier, FALSE);
    }
  } else {
    // HALT GUY HERE
    DebugMsg(
        TOPIC_JA2, DBG_LEVEL_3,
        String("HandleGotoNewGridNo() Failed: No APs %d %d", sAPCost, pSoldier->bActionPoints));
    HaltGuyFromNewGridNoBecauseOfNoAPs(pSoldier);
    pSoldier->bEndDoorOpenCode = FALSE;
    (*pfKeepMoving) = FALSE;
  }

  return (TRUE);
}

void HandleMaryArrival(struct SOLDIERTYPE *pSoldier) {
  int16_t sDist;

  if (!pSoldier) {
    pSoldier = FindSoldierByProfileID(MARY, TRUE);
    if (!pSoldier) {
      return;
    }
  }

  if (CheckFact(FACT_JOHN_ALIVE, 0)) {
    return;
  }
  // new requirements: player close by
  else if (PythSpacesAway(pSoldier->sGridNo, 8228) < 40) {
    if (ClosestPC(pSoldier, &sDist) != NOWHERE && sDist > NPC_TALK_RADIUS * 2) {
      // too far away
      return;
    }

    // Mary has arrived
    SetFactTrue(FACT_MARY_OR_JOHN_ARRIVED);

    EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

    TriggerNPCRecord(MARY, 13);
  }
}

void HandleJohnArrival(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pSoldier2 = NULL;
  int16_t sDist;

  if (!pSoldier) {
    pSoldier = FindSoldierByProfileID(JOHN, TRUE);
    if (!pSoldier) {
      return;
    }
  }

  if (PythSpacesAway(pSoldier->sGridNo, 8228) < 40) {
    if (ClosestPC(pSoldier, &sDist) != NOWHERE && sDist > NPC_TALK_RADIUS * 2) {
      // too far away
      return;
    }

    if (CheckFact(FACT_MARY_ALIVE, 0)) {
      pSoldier2 = FindSoldierByProfileID(MARY, FALSE);
      if (pSoldier2) {
        if (PythSpacesAway(pSoldier->sGridNo, pSoldier2->sGridNo) > 8) {
          // too far away!
          return;
        }
      }
    }

    SetFactTrue(FACT_MARY_OR_JOHN_ARRIVED);

    EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

    // if Mary is alive/dead
    if (pSoldier2) {
      EVENT_StopMerc(pSoldier2, pSoldier2->sGridNo, pSoldier2->bDirection);
      TriggerNPCRecord(JOHN, 13);
    } else {
      TriggerNPCRecord(JOHN, 12);
    }
  }
}

BOOLEAN HandleAtNewGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN *pfKeepMoving) {
  int16_t sMineGridNo;
  uint8_t ubVolume;

  // ATE; Handle bad guys, as they fade, to cancel it if
  // too long...
  // ONLY if fading IN!
  if (pSoldier->fBeginFade == 1) {
    if (pSoldier->sLocationOfFadeStart != pSoldier->sGridNo) {
      // Turn off
      pSoldier->fBeginFade = FALSE;

      if (pSoldier->bLevel > 0 && gpWorldLevelData[pSoldier->sGridNo].pRoofHead != NULL) {
        pSoldier->ubFadeLevel = gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubShadeLevel;
      } else {
        pSoldier->ubFadeLevel = gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;
      }

      // Set levelnode shade level....
      if (pSoldier->pLevelNode) {
        pSoldier->pLevelNode->ubShadeLevel = pSoldier->ubFadeLevel;
      }
      pSoldier->bVisible = -1;
    }
  }

  if (gTacticalStatus.uiFlags & INCOMBAT) {
    HandleLocateToGuyAsHeWalks(pSoldier);
  }

  // Default to TRUE
  (*pfKeepMoving) = TRUE;

  pSoldier->bTilesMoved++;
  if (pSoldier->usAnimState == RUNNING) {
    // count running as double
    pSoldier->bTilesMoved++;
  }

  // First if we are in realtime combat or noncombat
  if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    // Update value for RT breath update
    pSoldier->ubTilesMovedPerRTBreathUpdate++;
    // Update last anim
    pSoldier->usLastMovementAnimPerRTBreathUpdate = pSoldier->usAnimState;
  }

  // Update path if showing path in RT
  if (gGameSettings.fOptions[TOPTION_ALWAYS_SHOW_MOVEMENT_PATH]) {
    if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
      gfPlotNewMovement = TRUE;
    }
  }

  // ATE: Put some stuff in here to not handle certain things if we are
  // trversing...
  if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN ||
      gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO) {
    return (TRUE);
  }

  // Check if they are out of breath
  if (CheckForBreathCollapse(pSoldier)) {
    (*pfKeepMoving) = TRUE;
    return (FALSE);
  }

  // see if a mine gets set off...
  if (SetOffBombsInGridNo(pSoldier->ubID, pSoldier->sGridNo, FALSE, pSoldier->bLevel)) {
    (*pfKeepMoving) = FALSE;
    EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
    return (FALSE);
  }

  // Set "interrupt occurred" flag to false so that we will know whether *this
  // particular call* to HandleSight caused an interrupt
  gTacticalStatus.fInterruptOccurred = FALSE;

  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    ubVolume = MovementNoise(pSoldier);
    if (ubVolume > 0) {
      MakeNoise(pSoldier->ubID, pSoldier->sGridNo, pSoldier->bLevel, pSoldier->bOverTerrainType,
                ubVolume, NOISE_MOVEMENT);
      if ((pSoldier->uiStatusFlags & SOLDIER_PC) && (pSoldier->bStealthMode)) {
        PlayStealthySoldierFootstepSound(pSoldier);
      }
    }
  }

  // ATE: Make sure we don't make another interrupt...
  if (!gTacticalStatus.fInterruptOccurred) {
    // Handle New sight
    HandleSight(pSoldier, SIGHT_LOOK | SIGHT_RADIO | SIGHT_INTERRUPT);
  }

  // ATE: Check if we have sighted anyone, if so, don't do anything else...
  // IN other words, we have stopped from sighting...
  if (gTacticalStatus.fInterruptOccurred) {
    // Unset no APs value
    AdjustNoAPToFinishMove(pSoldier, TRUE);

    (*pfKeepMoving) = FALSE;
    pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
    pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;

    // ATE: Cancel only if our final destination
    if (pSoldier->sGridNo == pSoldier->sFinalDestination) {
      pSoldier->ubPendingAction = NO_PENDING_ACTION;
    }

    // this flag is set only to halt the currently moving guy; reset it now
    gTacticalStatus.fInterruptOccurred = FALSE;

    // ATE: Remove this if we were stopped....
    if (gTacticalStatus.fEnemySightingOnTheirTurn) {
      if (gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID == GetSolID(pSoldier)) {
        pSoldier->fPauseAllAnimation = FALSE;
        gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
      }
    }
  } else if (pSoldier->fNoAPToFinishMove) {
    (*pfKeepMoving) = FALSE;
  } else if (pSoldier->usPathIndex == pSoldier->usPathDataSize && pSoldier->usPathDataSize == 0) {
    (*pfKeepMoving) = FALSE;
  } else if (gTacticalStatus.fEnemySightingOnTheirTurn) {
    // Hault guy!
    AdjustNoAPToFinishMove(pSoldier, TRUE);
    (*pfKeepMoving) = FALSE;
  }

  // OK, check for other stuff like mines...
  if (NearbyGroundSeemsWrong(pSoldier, pSoldier->sGridNo, TRUE, (uint16_t *)&sMineGridNo)) {
    if (pSoldier->uiStatusFlags & SOLDIER_PC) {
      // NearbyGroundSeemsWrong returns true with gridno NOWHERE if
      // we find something by metal detector... we should definitely stop
      // but we won't place a locator or say anything

      // IF not in combat, stop them all
      if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
        int32_t cnt2;
        struct SOLDIERTYPE *pSoldier2;

        cnt2 = gTacticalStatus.Team[gbPlayerNum].bLastID;

        // look for all mercs on the same team,
        for (pSoldier2 = MercPtrs[cnt2]; cnt2 >= gTacticalStatus.Team[gbPlayerNum].bFirstID;
             cnt2--, pSoldier2--) {
          if (pSoldier2->bActive) {
            EVENT_StopMerc(pSoldier2, pSoldier2->sGridNo, pSoldier2->bDirection);
          }
        }
      } else {
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
      }

      (*pfKeepMoving) = FALSE;

      if (sMineGridNo != NOWHERE) {
        LocateGridNo(sMineGridNo);
        // we reuse the boobytrap gridno variable here
        gsBoobyTrapGridNo = sMineGridNo;
        gpBoobyTrapSoldier = pSoldier;
        SetStopTimeQuoteCallback(MineSpottedDialogueCallBack);
        TacticalCharacterDialogue(pSoldier, QUOTE_SUSPICIOUS_GROUND);
      }
    } else {
      if (sMineGridNo != NOWHERE) {
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
        (*pfKeepMoving) = FALSE;

        gpWorldLevelData[sMineGridNo].uiFlags |= MAPELEMENT_ENEMY_MINE_PRESENT;

        // better stop and reconsider what to do...
        SetNewSituation(pSoldier);
        ActionDone(pSoldier);
      }
    }
  }

  HandleSystemNewAISituation(pSoldier, FALSE);

  if (pSoldier->bTeam == gbPlayerNum) {
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      // are we there yet?
      if (GetSolSectorX(pSoldier) == 13 && GetSolSectorY(pSoldier) == MAP_ROW_B &&
          GetSolSectorZ(pSoldier) == 0) {
        switch (GetSolProfile(pSoldier)) {
          case SKYRIDER:
            if (PythSpacesAway(pSoldier->sGridNo, 8842) < 11) {
              // Skyrider has arrived!
              EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
              SetFactTrue(FACT_SKYRIDER_CLOSE_TO_CHOPPER);
              TriggerNPCRecord(SKYRIDER, 15);
              SetUpHelicopterForPlayer(13, MAP_ROW_B);
            }
            break;

          case MARY:
            HandleMaryArrival(pSoldier);
            break;

          case JOHN:
            HandleJohnArrival(pSoldier);
            break;
        }
      } else if (GetSolProfile(pSoldier) == MARIA &&
                 (GetSolSectorX(pSoldier) == 6 && GetSolSectorY(pSoldier) == MAP_ROW_C &&
                  GetSolSectorZ(pSoldier) == 0) &&
                 CheckFact(FACT_MARIA_ESCORTED_AT_LEATHER_SHOP, MARIA) == TRUE) {
        // check that Angel is there!
        if (NPCInRoom(ANGEL, 2))  // room 2 is leather shop
        {
          //	UnRecruitEPC( MARIA );
          TriggerNPCRecord(ANGEL, 12);
        }
      } else if ((GetSolProfile(pSoldier) == JOEY) &&
                 (GetSolSectorX(pSoldier) == 8 && GetSolSectorY(pSoldier) == MAP_ROW_G &&
                  GetSolSectorZ(pSoldier) == 0)) {
        // if Joey walks near Martha then trigger Martha record 7
        if (CheckFact(FACT_JOEY_NEAR_MARTHA, 0)) {
          EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
          TriggerNPCRecord(JOEY, 9);
        }
      }

    }
    // Drassen stuff for John & Mary
    else if (gubQuest[QUEST_ESCORT_TOURISTS] == QUESTINPROGRESS && GetSolSectorX(pSoldier) == 13 &&
             GetSolSectorY(pSoldier) == MAP_ROW_B && GetSolSectorZ(pSoldier) == 0) {
      if (CheckFact(FACT_JOHN_ALIVE, 0)) {
        HandleJohnArrival(NULL);
      } else {
        HandleMaryArrival(NULL);
      }
    }

  } else if (pSoldier->bTeam == CIV_TEAM && GetSolProfile(pSoldier) != NO_PROFILE &&
             pSoldier->bNeutral) {
    switch (GetSolProfile(pSoldier)) {
      case JIM:
      case JACK:
      case OLAF:
      case RAY:
      case OLGA:
      case TYRONE: {
        int16_t sDesiredMercDist;

        if (ClosestPC(pSoldier, &sDesiredMercDist) != NOWHERE) {
          if (sDesiredMercDist <= NPC_TALK_RADIUS * 2) {
            // stop
            CancelAIAction(pSoldier, TRUE);
            // aaaaaaaaaaaaaaaaaaaaatttaaaack!!!!
            AddToShouldBecomeHostileOrSayQuoteList(pSoldier->ubID);
            // MakeCivHostile( pSoldier, 2 );
            // TriggerNPCWithIHateYouQuote( GetSolProfile(pSoldier) );
          }
        }
      } break;
      default:
        break;
    }
  }
  return (TRUE);
}

void SelectNextAvailSoldier(struct SOLDIERTYPE *pSoldier) {
  int32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier;
  BOOLEAN fSoldierFound = FALSE;

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (OK_CONTROLLABLE_MERC(pTeamSoldier)) {
      fSoldierFound = TRUE;
      break;
    }
  }

  if (fSoldierFound) {
    SelectSoldier((int16_t)cnt, FALSE, FALSE);
  } else {
    gusSelectedSoldier = NO_SOLDIER;
    // Change UI mode to reflact that we are selected
    guiPendingOverrideEvent = I_ON_TERRAIN;
  }
}

void InternalSelectSoldier(uint16_t usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect,
                           BOOLEAN fFromUI) {
  struct SOLDIERTYPE *pSoldier, *pOldSoldier;

  // ARM: can't call SelectSoldier() in mapscreen, that will initialize interface panels!!!
  // ATE: Adjusted conditions a bit ( sometimes were not getting selected )
  if (guiCurrentScreen == LAPTOP_SCREEN || IsMapScreen_2()) {
    return;
  }

  if (usSoldierID == NOBODY) {
    return;
  }

  // if we are in the shop keeper interface
  if (guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE) {
    // dont allow the player to change the selected merc
    return;
  }

  // Get guy
  pSoldier = MercPtrs[usSoldierID];

  // If we are dead, ignore
  if (!OK_CONTROLLABLE_MERC(pSoldier)) {
    return;
  }

  // Don't do it if we don't have an interrupt
  if (!OK_INTERRUPT_MERC(pSoldier)) {
    // OK, we want to display message that we can't....
    if (fFromUI) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[MERC_IS_UNAVAILABLE_STR],
                pSoldier->name);
    }
    return;
  }

  if (pSoldier->ubID == gusSelectedSoldier) {
    if (!fForceReselect) {
      return;
    }
  }

  // CANCEL FROM PLANNING MODE!
  if (InUIPlanMode()) {
    EndUIPlan();
  }

  // Unselect old selected guy
  if (gusSelectedSoldier != NO_SOLDIER) {
    // Get guy
    pOldSoldier = MercPtrs[gusSelectedSoldier];
    pOldSoldier->fShowLocator = FALSE;
    pOldSoldier->fFlashLocator = FALSE;

    // DB This used to say pSoldier... I fixed it
    if (pOldSoldier->bLevel == 0) {
      // ConcealWalls((int16_t)(pSoldier->dXPos/CELL_X_SIZE),
      // (int16_t)(pSoldier->dYPos/CELL_Y_SIZE), REVEAL_WALLS_RADIUS);
      //	ApplyTranslucencyToWalls((int16_t)(pOldSoldier->dXPos/CELL_X_SIZE),
      //(int16_t)(pOldSoldier->dYPos/CELL_Y_SIZE));
      // LightHideTrees((int16_t)(pOldSoldier->dXPos/CELL_X_SIZE),
      // (int16_t)(pOldSoldier->dYPos/CELL_Y_SIZE));
    }
    // DeleteSoldierLight( pOldSoldier );

    if (pOldSoldier->uiStatusFlags & SOLDIER_GREEN_RAY) {
      LightHideRays((int16_t)(pOldSoldier->dXPos / CELL_X_SIZE),
                    (int16_t)(pOldSoldier->dYPos / CELL_Y_SIZE));
      pOldSoldier->uiStatusFlags &= (~SOLDIER_GREEN_RAY);
    }

    UpdateForContOverPortrait(pOldSoldier, FALSE);
  }

  gusSelectedSoldier = (uint16_t)usSoldierID;

  // find which squad this guy is, then set selected squad to this guy
  SetCurrentSquad(pSoldier->bAssignment, FALSE);

  if (pSoldier->bLevel == 0) {
    // RevealWalls((int16_t)(pSoldier->dXPos/CELL_X_SIZE), (int16_t)(pSoldier->dYPos/CELL_Y_SIZE),
    // REVEAL_WALLS_RADIUS);
    //	CalcTranslucentWalls((int16_t)(pSoldier->dXPos/CELL_X_SIZE),
    //(int16_t)(pSoldier->dYPos/CELL_Y_SIZE));
    // LightTranslucentTrees((int16_t)(pSoldier->dXPos/CELL_X_SIZE),
    // (int16_t)(pSoldier->dYPos/CELL_Y_SIZE));
  }

  // SetCheckSoldierLightFlag( pSoldier );

  // Set interface to reflect new selection!
  SetCurrentTacticalPanelCurrentMerc((uint8_t)usSoldierID);

  // PLay ATTN SOUND
  if (fAcknowledge) {
    if (!gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS])
      DoMercBattleSound(pSoldier, BATTLE_SOUND_ATTN1);
  }

  // Change UI mode to reflact that we are selected
  // NOT if we are locked inthe UI
  if (gTacticalStatus.ubCurrentTeam == OUR_TEAM && gCurrentUIMode != LOCKUI_MODE &&
      gCurrentUIMode != LOCKOURTURN_UI_MODE) {
    guiPendingOverrideEvent = M_ON_TERRAIN;
  }

  ChangeInterfaceLevel(pSoldier->bLevel);

  if (pSoldier->fMercAsleep) {
    PutMercInAwakeState(pSoldier);
  }

  // possibly say personality quote
  if ((pSoldier->bTeam == gbPlayerNum) &&
      (GetSolProfile(pSoldier) != NO_PROFILE &&
       pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__PLAYER_CHARACTER) &&
      !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY)) {
    switch (gMercProfiles[GetSolProfile(pSoldier)].bPersonalityTrait) {
      case PSYCHO:
        if (Random(50) == 0) {
          TacticalCharacterDialogue(pSoldier, QUOTE_PERSONALITY_TRAIT);
          pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
        }
        break;
      default:
        break;
    }
  }

  UpdateForContOverPortrait(pSoldier, TRUE);

  // Remove any interactive tiles we could be over!
  BeginCurInteractiveTileCheck(INTILE_CHECK_SELECTIVE);
}

void SelectSoldier(uint16_t usSoldierID, BOOLEAN fAcknowledge, BOOLEAN fForceReselect) {
  InternalSelectSoldier(usSoldierID, fAcknowledge, fForceReselect, FALSE);
}

BOOLEAN ResetAllAnimationCache() {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // Loop through all mercs and make go
  for (pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++) {
    if (pSoldier != NULL) {
      InitAnimationCache((uint16_t)cnt, &(pSoldier->AnimCache));
    }
  }

  return (TRUE);
}

void LocateSoldier(uint16_t usID, BOOLEAN fSetLocator) {
  struct SOLDIERTYPE *pSoldier;
  int16_t sNewCenterWorldX, sNewCenterWorldY;

  // if (!bCenter && SoldierOnScreen(usID))
  // return;

  // do we need to move the screen?
  // ATE: Force this baby to locate if told to
  if (!SoldierOnScreen(usID) || fSetLocator == 10) {
    // Get pointer of soldier
    pSoldier = MercPtrs[usID];

    // Center on guy
    sNewCenterWorldX = (int16_t)pSoldier->dXPos;
    sNewCenterWorldY = (int16_t)pSoldier->dYPos;

    SetRenderCenter(sNewCenterWorldX, sNewCenterWorldY);

    // Plot new path!
    gfPlotNewMovement = TRUE;
  }

  // do we flash the name & health bars/health string above?
  if (fSetLocator) {
    if (fSetLocator == SETLOCATOR || fSetLocator == 10) {
      ShowRadioLocator((uint8_t)usID, SHOW_LOCATOR_NORMAL);
    } else {
      ShowRadioLocator((uint8_t)usID, SHOW_LOCATOR_FAST);
    }
  }
}

void InternalLocateGridNo(uint16_t sGridNo, BOOLEAN fForce) {
  int16_t sNewCenterWorldX, sNewCenterWorldY;

  ConvertGridNoToCenterCellXY(sGridNo, &sNewCenterWorldX, &sNewCenterWorldY);

  // FIRST CHECK IF WE ARE ON SCREEN
  if (GridNoOnScreen(sGridNo) && !fForce) {
    return;
  }

  SetRenderCenter(sNewCenterWorldX, sNewCenterWorldY);
}

void LocateGridNo(uint16_t sGridNo) { InternalLocateGridNo(sGridNo, FALSE); }

void SlideTo(int16_t sGridno, uint16_t usSoldierID, uint16_t usReasonID, BOOLEAN fSetLocator) {
  int32_t cnt;

  if (usSoldierID == NOBODY) {
    return;
  }

  if (fSetLocator == SETANDREMOVEPREVIOUSLOCATOR) {
    for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
      if (MercPtrs[cnt]->bActive && MercPtrs[cnt]->bInSector) {
        // Remove all existing locators...
        MercPtrs[cnt]->fFlashLocator = FALSE;
      }
    }
  }

  // Locate even if on screen
  if (fSetLocator) ShowRadioLocator((uint8_t)usSoldierID, SHOW_LOCATOR_NORMAL);

  // FIRST CHECK IF WE ARE ON SCREEN
  if (GridNoOnScreen(MercPtrs[usSoldierID]->sGridNo)) {
    return;
  }

  // sGridNo here for DG compatibility
  gTacticalStatus.sSlideTarget = MercPtrs[usSoldierID]->sGridNo;
  gTacticalStatus.sSlideReason = usReasonID;

  // Plot new path!
  gfPlotNewMovement = TRUE;
}

void SlideToLocation(uint16_t usReasonID, int16_t sDestGridNo) {
  if (sDestGridNo == NOWHERE) {
    return;
  }

  // FIRST CHECK IF WE ARE ON SCREEN
  if (GridNoOnScreen(sDestGridNo)) {
    return;
  }

  // sGridNo here for DG compatibility
  gTacticalStatus.sSlideTarget = sDestGridNo;
  gTacticalStatus.sSlideReason = usReasonID;

  // Plot new path!
  gfPlotNewMovement = TRUE;
}

void RebuildAllSoldierShadeTables() {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // Loop through all mercs and make go
  for (pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++) {
    if (IsSolActive(pSoldier)) {
      CreateSoldierPalettes(pSoldier);
    }
  }
}

void HandlePlayerTeamMemberDeath(struct SOLDIERTYPE *pSoldier) {
  int32_t cnt;
  int32_t iNewSelectedSoldier;
  struct SOLDIERTYPE *pTeamSoldier;
  BOOLEAN fMissionFailed = TRUE;
  int8_t bBuddyIndex;

  VerifyPublicOpplistDueToDeath(pSoldier);

  ReceivingSoldierCancelServices(pSoldier);

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive && pTeamSoldier->bInSector) {
      iNewSelectedSoldier = cnt;
      fMissionFailed = FALSE;
      break;
    }
  }

  if (!fMissionFailed) {
    if (gTacticalStatus.fAutoBandageMode) {
      if (pSoldier->ubAutoBandagingMedic != NOBODY) {
        CancelAIAction(MercPtrs[pSoldier->ubAutoBandagingMedic], TRUE);
      }
    }

    // see if this was the friend of a living merc
    cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
    for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
         cnt++, pTeamSoldier++) {
      if (pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bLife >= OKLIFE) {
        bBuddyIndex = WhichBuddy(pTeamSoldier->ubProfile, GetSolProfile(pSoldier));
        switch (bBuddyIndex) {
          case 0:
            // buddy #1 died!
            TacticalCharacterDialogue(pTeamSoldier, QUOTE_BUDDY_ONE_KILLED);
            break;
          case 1:
            // buddy #2 died!
            TacticalCharacterDialogue(pTeamSoldier, QUOTE_BUDDY_TWO_KILLED);
            break;
          case 2:
            // learn to like buddy died!
            TacticalCharacterDialogue(pTeamSoldier, QUOTE_LEARNED_TO_LIKE_MERC_KILLED);
            break;
          default:
            break;
        }
      }
    }

    // handle stuff for Carmen if Slay is killed
    switch (GetSolProfile(pSoldier)) {
      case SLAY:
        pTeamSoldier = FindSoldierByProfileID(CARMEN, FALSE);
        if (pTeamSoldier && pTeamSoldier->bAttitude == ATTACKSLAYONLY &&
            ClosestPC(pTeamSoldier, NULL) != NOWHERE) {
          // Carmen now becomes friendly again
          TriggerNPCRecord(CARMEN, 29);
        }
        break;
      case ROBOT:
        if (CheckFact(FACT_FIRST_ROBOT_DESTROYED, 0) == FALSE) {
          SetFactTrue(FACT_FIRST_ROBOT_DESTROYED);
          SetFactFalse(FACT_ROBOT_READY);
        } else {
          SetFactTrue(FACT_SECOND_ROBOT_DESTROYED);
        }
        break;
    }
  }

  // Make a call to handle the strategic things, such as Life Insurance, record it in history file
  // etc.
  StrategicHandlePlayerTeamMercDeath(pSoldier);

  CheckForEndOfBattle(FALSE);

  if (gusSelectedSoldier == GetSolID(pSoldier)) {
    if (!fMissionFailed) {
      SelectSoldier((int16_t)iNewSelectedSoldier, FALSE, FALSE);
    } else {
      gusSelectedSoldier = NO_SOLDIER;
      // Change UI mode to reflact that we are selected
      guiPendingOverrideEvent = I_ON_TERRAIN;
    }
  }
}

void HandleNPCTeamMemberDeath(struct SOLDIERTYPE *pSoldierOld) {
  struct SOLDIERTYPE *pKiller = NULL;
  BOOLEAN bVisible;

  pSoldierOld->uiStatusFlags |= SOLDIER_DEAD;
  bVisible = pSoldierOld->bVisible;

  VerifyPublicOpplistDueToDeath(pSoldierOld);

  if (pSoldierOld->ubProfile != NO_PROFILE) {
    // mark as dead!
    gMercProfiles[pSoldierOld->ubProfile].bMercStatus = MERC_IS_DEAD;
    //
    gMercProfiles[pSoldierOld->ubProfile].bLife = 0;

    if (!(pSoldierOld->uiStatusFlags & SOLDIER_VEHICLE) && !TANK(pSoldierOld)) {
      if (pSoldierOld->ubAttackerID != NOBODY) {
        pKiller = MercPtrs[pSoldierOld->ubAttackerID];
      }
      if (pKiller && pKiller->bTeam == OUR_TEAM) {
        AddHistoryToPlayersLog(HISTORY_MERC_KILLED_CHARACTER, pSoldierOld->ubProfile,
                               GetWorldTotalMin(), (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
      } else {
        AddHistoryToPlayersLog(HISTORY_NPC_KILLED, pSoldierOld->ubProfile, GetWorldTotalMin(),
                               (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
      }
    }
  }

  if (pSoldierOld->bTeam == CIV_TEAM) {
    struct SOLDIERTYPE *pOther;

    // ATE: Added string to player
    if (bVisible != -1 && pSoldierOld->ubProfile != NO_PROFILE) {
      ScreenMsg(FONT_RED, MSG_INTERFACE, pMercDeadString[0], pSoldierOld->name);
    }

    switch (pSoldierOld->ubProfile) {
      case BRENDA:
        SetFactTrue(FACT_BRENDA_DEAD);
        {
          pOther = FindSoldierByProfileID(HANS, FALSE);
          if (pOther && pOther->bLife >= OKLIFE && pOther->bNeutral &&
              (SpacesAway(pSoldierOld->sGridNo, pOther->sGridNo) <= 12)) {
            TriggerNPCRecord(HANS, 10);
          }
        }
        break;
      case PABLO:
        AddFutureDayStrategicEvent(EVENT_SECOND_AIRPORT_ATTENDANT_ARRIVED, 480 + Random(60), 0, 1);
        break;
      case ROBOT:
        if (CheckFact(FACT_FIRST_ROBOT_DESTROYED, 0) == FALSE) {
          SetFactTrue(FACT_FIRST_ROBOT_DESTROYED);
        } else {
          SetFactTrue(FACT_SECOND_ROBOT_DESTROYED);
        }
        break;
      case DRUGGIST:
      case SLAY:
      case ANNIE:
      case CHRIS:
      case TIFFANY:
      case T_REX:
        MakeRemainingTerroristsTougher();
        if (pSoldierOld->ubProfile == DRUGGIST) {
          pOther = FindSoldierByProfileID(MANNY, 0);
          if (pOther && pOther->bActive && pOther->bInSector && pOther->bLife >= OKLIFE) {
            // try to make sure he isn't cowering etc
            pOther->sNoiseGridno = NOWHERE;
            pOther->bAlertStatus = STATUS_GREEN;
            TriggerNPCRecord(MANNY, 10);
          }
        }
        break;
      case JIM:
      case JACK:
      case OLAF:
      case RAY:
      case OLGA:
      case TYRONE:
        MakeRemainingAssassinsTougher();
        break;

      case ELDIN:
        // the security guard...  Results in an extra loyalty penalty for Balime (in addition to
        // civilian murder)

        /* Delayed loyalty effects elimininated.  Sep.12/98.  ARM
                                        // create the event value, for town BALIME
                                        uiLoyaltyValue = BuildLoyaltyEventValue( BALIME,
           LOYALTY_PENALTY_ELDIN_KILLED, FALSE );
                                        // post the event, 30 minute delay
                                        AddStrategicEvent( EVENT_TOWN_LOYALTY_UPDATE,
           GetWorldTotalMin() + 30, uiLoyaltyValue );
        */
        DecrementTownLoyalty(BALIME, LOYALTY_PENALTY_ELDIN_KILLED);
        break;
      case JOEY:
        // check to see if Martha can see this
        pOther = FindSoldierByProfileID(MARTHA, FALSE);
        if (pOther && (PythSpacesAway(pOther->sGridNo, pSoldierOld->sGridNo) < 10 ||
                       SoldierToSoldierLineOfSightTest(pOther, pSoldierOld,
                                                       (uint8_t)MaxDistanceVisible(), TRUE) != 0)) {
          // Martha has a heart attack and croaks
          TriggerNPCRecord(MARTHA, 17);

          /* Delayed loyalty effects elimininated.  Sep.12/98.  ARM
                                                  // create the event value, for town CAMBRIA
                                                  uiLoyaltyValue = BuildLoyaltyEventValue( CAMBRIA,
             LOYALTY_PENALTY_MARTHA_HEART_ATTACK, FALSE );
                                                  // post the event, 30 minute delay
                                                  AddStrategicEvent( EVENT_TOWN_LOYALTY_UPDATE,
             GetWorldTotalMin() + 30, uiLoyaltyValue );
          */
          DecrementTownLoyalty(CAMBRIA, LOYALTY_PENALTY_MARTHA_HEART_ATTACK);
        } else  // Martha doesn't see it.  She lives, but Joey is found a day or so later anyways
        {
          /*
                                                  // create the event value, for town CAMBRIA
                                                  uiLoyaltyValue = BuildLoyaltyEventValue( CAMBRIA,
             LOYALTY_PENALTY_JOEY_KILLED, FALSE );
                                                  // post the event, 30 minute delay
                                                  AddStrategicEvent( EVENT_TOWN_LOYALTY_UPDATE,
             GetWorldTotalMin() + ( ( 12 + Random(13)) * 60 ), uiLoyaltyValue );
          */
          DecrementTownLoyalty(CAMBRIA, LOYALTY_PENALTY_JOEY_KILLED);
        }
        break;
      case DYNAMO:
        // check to see if dynamo quest is on
        if (gubQuest[QUEST_FREE_DYNAMO] == QUESTINPROGRESS) {
          EndQuest(QUEST_FREE_DYNAMO, (uint8_t)pSoldierOld->sSectorX,
                   (uint8_t)pSoldierOld->sSectorY);
        }
        break;
      case KINGPIN:
        // check to see if Kingpin money quest is on
        if (gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS) {
          EndQuest(QUEST_KINGPIN_MONEY, (uint8_t)pSoldierOld->sSectorX,
                   (uint8_t)pSoldierOld->sSectorY);
          HandleNPCDoAction(KINGPIN, NPC_ACTION_GRANT_EXPERIENCE_3, 0);
        }
        SetFactTrue(FACT_KINGPIN_DEAD);
        ExecuteStrategicAIAction(STRATEGIC_AI_ACTION_KINGPIN_DEAD, 0, 0);
        break;
      case DOREEN:
        // Doreen's dead
        if (CheckFact(FACT_DOREEN_HAD_CHANGE_OF_HEART, 0)) {
          // tsk tsk, player killed her after getting her to reconsider, lose the bonus for sparing
          // her
          DecrementTownLoyalty(DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED);
        }  // then get the points for freeing the kids though killing her
        IncrementTownLoyalty(DRASSEN, LOYALTY_BONUS_CHILDREN_FREED_DOREEN_KILLED);
        // set the fact true so we have a universal check for whether the kids can go
        SetFactTrue(FACT_DOREEN_HAD_CHANGE_OF_HEART);
        EndQuest(QUEST_FREE_CHILDREN, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
        if (CheckFact(FACT_KIDS_ARE_FREE, 0) == FALSE) {
          HandleNPCDoAction(DOREEN, NPC_ACTION_FREE_KIDS, 0);
        }
        break;
    }

    // Are we looking at the queen?
    if (pSoldierOld->ubProfile == QUEEN) {
      if (pSoldierOld->ubAttackerID != NOBODY) {
        pKiller = MercPtrs[pSoldierOld->ubAttackerID];
      }

      BeginHandleDeidrannaDeath(pKiller, pSoldierOld->sGridNo, pSoldierOld->bLevel);
    }

    // crows/cows are on the civilian team, but none of the following applies to them
    if ((pSoldierOld->ubBodyType != CROW) && (pSoldierOld->ubBodyType != COW)) {
      // If the civilian's killer is known
      if (pSoldierOld->ubAttackerID != NOBODY) {
        // handle death of civilian..and if it was intentional
        HandleMurderOfCivilian(pSoldierOld, pSoldierOld->fIntendedTarget);
      }
    }
  } else if (pSoldierOld->bTeam == MILITIA_TEAM) {
    int8_t bMilitiaRank;

    bMilitiaRank = SoldierClassToMilitiaRank(pSoldierOld->ubSoldierClass);

    if (bMilitiaRank != -1) {
      // remove this militia from the strategic records
      StrategicRemoveMilitiaFromSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, bMilitiaRank,
                                       1);
    }

    // If the militia's killer is known
    if (pSoldierOld->ubAttackerID != NOBODY) {
      // also treat this as murder - but player will never be blamed for militia death he didn't
      // cause
      HandleMurderOfCivilian(pSoldierOld, pSoldierOld->fIntendedTarget);
    }

    HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_NATIVE_KILLED, (uint8_t)gWorldSectorX,
                             (uint8_t)gWorldSectorY, gbWorldSectorZ);
  } else  // enemies and creatures... should any of this stuff not be called if a creature dies?
  {
    if (pSoldierOld->ubBodyType == QUEENMONSTER) {
      struct SOLDIERTYPE *pKiller = NULL;

      if (pSoldierOld->ubAttackerID != NOBODY) {
        pKiller = MercPtrs[pSoldierOld->ubAttackerID];

        BeginHandleQueenBitchDeath(pKiller, pSoldierOld->sGridNo, pSoldierOld->bLevel);
      }
    }

    if (pSoldierOld->bTeam == ENEMY_TEAM) {
      gTacticalStatus.ubArmyGuysKilled++;
      TrackEnemiesKilled(ENEMY_KILLED_IN_TACTICAL, pSoldierOld->ubSoldierClass);
    }
    // If enemy guy was killed by the player, give morale boost to player's team!
    if (pSoldierOld->ubAttackerID != NOBODY &&
        MercPtrs[pSoldierOld->ubAttackerID]->bTeam == gbPlayerNum) {
      HandleMoraleEvent(MercPtrs[pSoldierOld->ubAttackerID], MORALE_KILLED_ENEMY,
                        (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
    }

    HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ENEMY_KILLED, (uint8_t)gWorldSectorX,
                             (uint8_t)gWorldSectorY, gbWorldSectorZ);

    CheckForAlertWhenEnemyDies(pSoldierOld);

    if (gTacticalStatus.ubTheChosenOne == pSoldierOld->ubID) {
      // reset the chosen one!
      gTacticalStatus.ubTheChosenOne = NOBODY;
    }

    if (pSoldierOld->ubProfile == QUEEN) {
      HandleMoraleEvent(NULL, MORALE_DEIDRANNA_KILLED, (uint8_t)gWorldSectorX,
                        (uint8_t)gWorldSectorY, gbWorldSectorZ);
      MaximizeLoyaltyForDeidrannaKilled();
    } else if (pSoldierOld->ubBodyType == QUEENMONSTER) {
      HandleMoraleEvent(NULL, MORALE_MONSTER_QUEEN_KILLED, (uint8_t)gWorldSectorX,
                        (uint8_t)gWorldSectorY, gbWorldSectorZ);
      IncrementTownLoyaltyEverywhere(LOYALTY_BONUS_KILL_QUEEN_MONSTER);

      // Grant experience gain.....
      HandleNPCDoAction(0, NPC_ACTION_GRANT_EXPERIENCE_5, 0);
    }
  }

  // killing crows/cows is not worth any experience!
  if ((pSoldierOld->ubBodyType != CROW) && (pSoldierOld->ubBodyType != COW) &&
      pSoldierOld->ubLastDamageReason != TAKE_DAMAGE_BLOODLOSS) {
    uint8_t ubAssister;

    // if it was a kill by a player's merc
    if (pSoldierOld->ubAttackerID != NOBODY &&
        MercPtrs[pSoldierOld->ubAttackerID]->bTeam == gbPlayerNum) {
      // EXPERIENCE CLASS GAIN:  Earned a kill
      StatChange(MercPtrs[pSoldierOld->ubAttackerID], EXPERAMT,
                 (uint16_t)(10 * pSoldierOld->bExpLevel), FALSE);
    }

    // JA2 Gold: if previous and current attackers are the same, the next-to-previous attacker gets
    // the assist
    if (pSoldierOld->ubPreviousAttackerID == pSoldierOld->ubAttackerID) {
      ubAssister = pSoldierOld->ubNextToPreviousAttackerID;
    } else {
      ubAssister = pSoldierOld->ubPreviousAttackerID;
    }

    // if it was assisted by a player's merc
    if (ubAssister != NOBODY && MercPtrs[ubAssister]->bTeam == gbPlayerNum) {
      // EXPERIENCE CLASS GAIN:  Earned an assist
      StatChange(MercPtrs[ubAssister], EXPERAMT, (uint16_t)(5 * pSoldierOld->bExpLevel), FALSE);
    }
  }

  if (pSoldierOld->ubAttackerID != NOBODY &&
      MercPtrs[pSoldierOld->ubAttackerID]->bTeam == MILITIA_TEAM) {
    MercPtrs[pSoldierOld->ubAttackerID]->ubMilitiaKills++;
  }

  // if the NPC is a dealer, add the dealers items to the ground
  AddDeadArmsDealerItemsToWorld(pSoldierOld->ubProfile);

  // The queen AI layer must process the event by subtracting forces, etc.
  ProcessQueenCmdImplicationsOfDeath(pSoldierOld);

  // OK, check for existence of any more badguys!
  CheckForEndOfBattle(FALSE);
}

uint8_t LastActiveTeamMember(uint8_t ubTeam) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  cnt = gTacticalStatus.Team[ubTeam].bLastID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt >= gTacticalStatus.Team[ubTeam].bFirstID; cnt--, pSoldier--) {
    if (IsSolActive(pSoldier)) {
      return ((int8_t)cnt);
    }
  }

  return (gTacticalStatus.Team[ubTeam].bLastID);
}

void CheckForPotentialAddToBattleIncrement(struct SOLDIERTYPE *pSoldier) {
  // Check if we are a threat!
  if (!pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum)) {
    // if ( FindObjClass( pSoldier, IC_WEAPON ) != NO_SLOT )
    // We need to exclude cases where a kid is not neutral anymore, but is defenceless!
    if (pSoldier->bTeam == CIV_TEAM) {
      // maybe increment num enemy attacked
      switch (pSoldier->ubCivilianGroup) {
        case REBEL_CIV_GROUP:
        case KINGPIN_CIV_GROUP:
        case HICKS_CIV_GROUP:
          if (FindObjClass(pSoldier, IC_WEAPON) != NO_SLOT) {
            gTacticalStatus.bNumFoughtInBattle[pSoldier->bTeam]++;
          }
          break;
        default:
          // nope!
          break;
      }
    } else {
      // Increment num enemy attacked
      gTacticalStatus.bNumFoughtInBattle[pSoldier->bTeam]++;
    }
  }
}

// internal function for turning neutral to FALSE
void SetSoldierNonNeutral(struct SOLDIERTYPE *pSoldier) {
  pSoldier->bNeutral = FALSE;

  if (gTacticalStatus.bBoxingState == NOT_BOXING) {
    // Special code for strategic implications
    CalculateNonPersistantPBIInfo();
  }
}

// internal function for turning neutral to TRUE
void SetSoldierNeutral(struct SOLDIERTYPE *pSoldier) {
  pSoldier->bNeutral = TRUE;

  if (gTacticalStatus.bBoxingState == NOT_BOXING) {
    // Special code for strategic implications
    // search through civ team looking for non-neutral civilian!
    if (!HostileCiviliansPresent()) {
      CalculateNonPersistantPBIInfo();
    }
  }
}
void MakeCivHostile(struct SOLDIERTYPE *pSoldier, int8_t bNewSide) {
  if (pSoldier->ubBodyType == COW) {
    return;
  }

  // override passed-in value; default is hostile to player, allied to army
  bNewSide = 1;

  switch (GetSolProfile(pSoldier)) {
    case IRA:
    case DIMITRI:
    case MIGUEL:
    case CARLOS:
    case MADLAB:
    case DYNAMO:
    case SHANK:
      // rebels and rebel sympathizers become hostile to player and enemy
      bNewSide = 2;
      break;
    case MARIA:
    case ANGEL:
      if (gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS ||
          gubQuest[QUEST_RESCUE_MARIA] == QUESTDONE) {
        bNewSide = 2;
      }
      break;
    default:
      switch (pSoldier->ubCivilianGroup) {
        case REBEL_CIV_GROUP:
          bNewSide = 2;
          break;
        default:
          break;
      }
      break;
  }

  if (!pSoldier->bNeutral && bNewSide == pSoldier->bSide) {
    // already hostile!
    return;
  }

  if (GetSolProfile(pSoldier) == CONRAD || GetSolProfile(pSoldier) == GENERAL) {
    // change to enemy team
    SetSoldierNonNeutral(pSoldier);
    pSoldier->bSide = bNewSide;
    pSoldier = ChangeSoldierTeam(pSoldier, ENEMY_TEAM);
  } else {
    if (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP) {
      // if Maria is in the sector and escorted, set fact that the escape has
      // been noticed
      if (gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS &&
          gTacticalStatus.bBoxingState == NOT_BOXING) {
        struct SOLDIERTYPE *pMaria = FindSoldierByProfileID(MARIA, FALSE);
        if (pMaria && pMaria->bActive && pMaria->bInSector) {
          SetFactTrue(FACT_MARIA_ESCAPE_NOTICED);
        }
      }
    }
    if (GetSolProfile(pSoldier) == BILLY) {
      // change orders
      pSoldier->bOrders = FARPATROL;
    }
    if (bNewSide != -1) {
      pSoldier->bSide = bNewSide;
    }
    if (pSoldier->bNeutral) {
      SetSoldierNonNeutral(pSoldier);
      RecalculateOppCntsDueToNoLongerNeutral(pSoldier);
    }
  }

  // If we are already in combat...
  if ((gTacticalStatus.uiFlags & INCOMBAT)) {
    CheckForPotentialAddToBattleIncrement(pSoldier);
  }
}

uint8_t CivilianGroupMembersChangeSidesWithinProximity(struct SOLDIERTYPE *pAttacked) {
  struct SOLDIERTYPE *pSoldier;
  uint8_t ubFirstProfile = NO_PROFILE;
  uint8_t cnt;

  if (pAttacked->ubCivilianGroup == NON_CIV_GROUP) {
    return (pAttacked->ubProfile);
  }

  cnt = gTacticalStatus.Team[CIV_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[CIV_TEAM].bLastID; cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife && pSoldier->bNeutral) {
      if (pSoldier->ubCivilianGroup == pAttacked->ubCivilianGroup && pSoldier->ubBodyType != COW) {
        // if in LOS of this guy's attacker
        if ((pAttacked->ubAttackerID != NOBODY &&
             pSoldier->bOppList[pAttacked->ubAttackerID] == SEEN_CURRENTLY) ||
            (PythSpacesAway(pSoldier->sGridNo, pAttacked->sGridNo) < MaxDistanceVisible()) ||
            (pAttacked->ubAttackerID != NOBODY &&
             PythSpacesAway(pSoldier->sGridNo, MercPtrs[pAttacked->ubAttackerID]->sGridNo) <
                 MaxDistanceVisible())) {
          MakeCivHostile(pSoldier, 2);
          if (pSoldier->bOppCnt > 0) {
            AddToShouldBecomeHostileOrSayQuoteList(pSoldier->ubID);
          }

          if (GetSolProfile(pSoldier) != NO_PROFILE && pSoldier->bOppCnt > 0 &&
              (ubFirstProfile == NO_PROFILE || Random(2))) {
            ubFirstProfile = GetSolProfile(pSoldier);
          }
        }
      }
    }
  }

  return (ubFirstProfile);
}

struct SOLDIERTYPE *CivilianGroupMemberChangesSides(struct SOLDIERTYPE *pAttacked) {
  struct SOLDIERTYPE *pNew;
  struct SOLDIERTYPE *pNewAttacked = pAttacked;
  struct SOLDIERTYPE *pSoldier;
  uint8_t cnt;
  uint8_t ubFirstProfile = NO_PROFILE;

  if (pAttacked->ubCivilianGroup == NON_CIV_GROUP) {
    // abort
    return (pNewAttacked);
  }

  // remove anyone (rebels) on our team and put them back in the civ team
  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      if (pSoldier->ubCivilianGroup == pAttacked->ubCivilianGroup) {
        // should become hostile
        if (GetSolProfile(pSoldier) != NO_PROFILE && (ubFirstProfile == NO_PROFILE || Random(2))) {
          ubFirstProfile = GetSolProfile(pSoldier);
        }

        pNew = ChangeSoldierTeam(pSoldier, CIV_TEAM);
        if (pSoldier == pAttacked) {
          pNewAttacked = pNew;
        }
      }
    }
  }

  // now change sides for anyone on the civ team within proximity
  if (ubFirstProfile == NO_PROFILE) {
    // get first profile value
    ubFirstProfile = CivilianGroupMembersChangeSidesWithinProximity(pNewAttacked);
  } else {
    // just call
    CivilianGroupMembersChangeSidesWithinProximity(pNewAttacked);
  }

  /*
          if ( ubFirstProfile != NO_PROFILE )
          {
                  TriggerFriendWithHostileQuote( ubFirstProfile );
          }
  */

  if (gTacticalStatus.fCivGroupHostile[pNewAttacked->ubCivilianGroup] == CIV_GROUP_NEUTRAL) {
    // if the civilian group turning hostile is the Rebels
    if (pAttacked->ubCivilianGroup == REBEL_CIV_GROUP) {
      // we haven't already reduced the loyalty back when we first set the flag to BECOME hostile
      ReduceLoyaltyForRebelsBetrayed();
    }

    AddStrategicEvent(EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE,
                      GetWorldTotalMin() + 300, pNewAttacked->ubCivilianGroup);
    gTacticalStatus.fCivGroupHostile[pNewAttacked->ubCivilianGroup] =
        CIV_GROUP_WILL_EVENTUALLY_BECOME_HOSTILE;
  }

  return (pNewAttacked);
}

void CivilianGroupChangesSides(uint8_t ubCivilianGroup) {
  // change civ group side due to external event (wall blowing up)
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  gTacticalStatus.fCivGroupHostile[ubCivilianGroup] = CIV_GROUP_HOSTILE;

  // now change sides for anyone on the civ team
  cnt = gTacticalStatus.Team[CIV_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[CIV_TEAM].bLastID; cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife && pSoldier->bNeutral) {
      if (pSoldier->ubCivilianGroup == ubCivilianGroup && pSoldier->ubBodyType != COW) {
        MakeCivHostile(pSoldier, 2);
        if (pSoldier->bOppCnt > 0) {
          AddToShouldBecomeHostileOrSayQuoteList(pSoldier->ubID);
        }
        /*
        if ( (GetSolProfile(pSoldier) != NO_PROFILE) && (pSoldier->bOppCnt > 0) && ( ubFirstProfile
        == NO_PROFILE || Random( 2 ) ) )
        {
                ubFirstProfile = GetSolProfile(pSoldier);
        }
        */
      }
    }
  }

  /*
  if ( ubFirstProfile != NO_PROFILE )
  {
          TriggerFriendWithHostileQuote( ubFirstProfile );
  }
  */
}

void HickCowAttacked(struct SOLDIERTYPE *pNastyGuy, struct SOLDIERTYPE *pTarget) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // now change sides for anyone on the civ team
  cnt = gTacticalStatus.Team[CIV_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[CIV_TEAM].bLastID; cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife && pSoldier->bNeutral &&
        pSoldier->ubCivilianGroup == HICKS_CIV_GROUP) {
      if (SoldierToSoldierLineOfSightTest(pSoldier, pNastyGuy, (uint8_t)MaxDistanceVisible(),
                                          TRUE)) {
        CivilianGroupMemberChangesSides(pSoldier);
        break;
      }
    }
  }
}

void MilitiaChangesSides(void) {
  // make all the militia change sides

  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  if (gTacticalStatus.Team[MILITIA_TEAM].bMenInSector == 0) {
    return;
  }

  // remove anyone (rebels) on our team and put them back in the civ team
  cnt = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[MILITIA_TEAM].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      MakeCivHostile(pSoldier, 2);
      RecalculateOppCntsDueToNoLongerNeutral(pSoldier);
    }
  }
}

/*
void MakePotentiallyHostileCivGroupsHostile( void )
{
        uint8_t		ubLoop;

        // loop through all civ groups that might become hostile and set them
        // to hostile
        for ( ubLoop = REBEL_CIV_GROUP; ubLoop < NUM_CIV_GROUPS; ubLoop++ )
        {
                if (gTacticalStatus.fCivGroupHostile[ ubLoop ] == CIV_GROUP_WILL_BECOME_HOSTILE)
                {
                        gTacticalStatus.fCivGroupHostile[ ubLoop ] = CIV_GROUP_HOSTILE;
                }
        }
}
*/

int8_t NumActiveAndConsciousTeamMembers(uint8_t ubTeam) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  uint8_t ubCount = 0;

  cnt = gTacticalStatus.Team[ubTeam].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[ubTeam].bLastID; cnt++, pSoldier++) {
    if (OK_CONTROLLABLE_MERC(pSoldier)) {
      ubCount++;
    }
  }

  return (ubCount);
}

uint8_t FindNextActiveAndAliveMerc(struct SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife,
                                   BOOLEAN fOnlyRegularMercs) {
  uint8_t bLastTeamID;
  int32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier;

  cnt = GetSolID(pSoldier) + 1;
  bLastTeamID = gTacticalStatus.Team[pSoldier->bTeam].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pTeamSoldier++) {
    if (fOnlyRegularMercs) {
      if (pTeamSoldier->bActive && (AM_AN_EPC(pTeamSoldier) || AM_A_ROBOT(pTeamSoldier))) {
        continue;
      }
    }

    if (fGoodForLessOKLife) {
      if (pTeamSoldier->bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector &&
          pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY &&
          OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    } else {
      if (OK_CONTROLLABLE_MERC(pTeamSoldier) && OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    }
  }

  // none found,
  // Now loop back
  cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
  bLastTeamID = GetSolID(pSoldier);

  for (pTeamSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pTeamSoldier++) {
    if (fOnlyRegularMercs) {
      if (pTeamSoldier->bActive && (AM_AN_EPC(pTeamSoldier) || AM_A_ROBOT(pTeamSoldier))) {
        continue;
      }
    }

    if (fGoodForLessOKLife) {
      if (pTeamSoldier->bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector &&
          pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY &&
          OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    } else {
      if (OK_CONTROLLABLE_MERC(pTeamSoldier) && OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    }
  }

  // IF we are here, keep as we always were!
  return (pSoldier->ubID);
}

struct SOLDIERTYPE *FindNextActiveSquad(struct SOLDIERTYPE *pSoldier) {
  int32_t cnt, cnt2;

  for (cnt = pSoldier->bAssignment + 1; cnt < NUMBER_OF_SQUADS; cnt++) {
    for (cnt2 = 0; cnt2 < NUMBER_OF_SOLDIERS_PER_SQUAD; cnt2++) {
      if (Squad[cnt][cnt2] != NULL && Squad[cnt][cnt2]->bInSector &&
          OK_INTERRUPT_MERC(Squad[cnt][cnt2]) && OK_CONTROLLABLE_MERC(Squad[cnt][cnt2]) &&
          !(Squad[cnt][cnt2]->uiStatusFlags & SOLDIER_VEHICLE)) {
        return (Squad[cnt][cnt2]);
      }
    }
  }

  // none found,
  // Now loop back
  for (cnt = 0; cnt <= pSoldier->bAssignment; cnt++) {
    for (cnt2 = 0; cnt2 < NUMBER_OF_SOLDIERS_PER_SQUAD; cnt2++) {
      if (Squad[cnt][cnt2] != NULL && Squad[cnt][cnt2]->bInSector &&
          OK_INTERRUPT_MERC(Squad[cnt][cnt2]) && OK_CONTROLLABLE_MERC(Squad[cnt][cnt2]) &&
          !(Squad[cnt][cnt2]->uiStatusFlags & SOLDIER_VEHICLE)) {
        return (Squad[cnt][cnt2]);
      }
    }
  }

  // IF we are here, keep as we always were!
  return (pSoldier);
}

uint8_t FindPrevActiveAndAliveMerc(struct SOLDIERTYPE *pSoldier, BOOLEAN fGoodForLessOKLife,
                                   BOOLEAN fOnlyRegularMercs) {
  uint8_t bLastTeamID;
  int32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier;

  // loop back
  bLastTeamID = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
  cnt = GetSolID(pSoldier) - 1;

  for (pTeamSoldier = MercPtrs[cnt]; cnt >= bLastTeamID; cnt--, pTeamSoldier--) {
    if (fOnlyRegularMercs) {
      if (AM_AN_EPC(pTeamSoldier) || AM_A_ROBOT(pTeamSoldier)) {
        continue;
      }
    }

    if (fGoodForLessOKLife) {
      // Check for bLife > 0
      if (pTeamSoldier->bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector &&
          pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY &&
          OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    } else {
      if (OK_CONTROLLABLE_MERC(pTeamSoldier) && OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    }
  }

  bLastTeamID = GetSolID(pSoldier);
  cnt = gTacticalStatus.Team[pSoldier->bTeam].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt >= bLastTeamID; cnt--, pTeamSoldier--) {
    if (fOnlyRegularMercs) {
      if (AM_AN_EPC(pTeamSoldier) || AM_A_ROBOT(pTeamSoldier)) {
        continue;
      }
    }

    if (fGoodForLessOKLife) {
      if (pTeamSoldier->bLife > 0 && pTeamSoldier->bActive && pTeamSoldier->bInSector &&
          pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bAssignment < ON_DUTY &&
          OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    } else {
      if (OK_CONTROLLABLE_MERC(pTeamSoldier) && OK_INTERRUPT_MERC(pTeamSoldier) &&
          GetSolAssignment(pSoldier) == pTeamSoldier->bAssignment) {
        return ((uint8_t)cnt);
      }
    }
  }

  // none found,
  // IF we are here, keep as we always were!
  return (pSoldier->ubID);
}

BOOLEAN CheckForPlayerTeamInMissionExit() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  uint8_t bGuysIn = 0;

  // End the turn of player charactors
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bLife >= OKLIFE) {
      if (pSoldier->fInMissionExitNode) {
        bGuysIn++;
      }
    }
  }

  if (bGuysIn == 0) {
    return (FALSE);
  }

  if (NumActiveAndConsciousTeamMembers(gbPlayerNum) == 0) {
    return (FALSE);
  }

  if (bGuysIn == NumActiveAndConsciousTeamMembers(gbPlayerNum)) {
    return (TRUE);
  }

  return (FALSE);
}

void EndTacticalDemo() {
  gTacticalStatus.uiFlags &= (~DEMOMODE);
  gTacticalStatus.fGoingToEnterDemo = FALSE;
}

uint32_t EnterTacticalDemoMode() {
  uint8_t ubNewScene = gubCurrentScene;
  uint8_t ubNumScenes = NUM_RANDOM_SCENES;

  gTacticalStatus.uiTimeOfLastInput = GetJA2Clock();

  // REMOVE ALL EVENTS!
  DequeAllGameEvents(FALSE);

  // Switch into realtime/demo
  gTacticalStatus.uiFlags |= (REALTIME | DEMOMODE);
  gTacticalStatus.uiFlags &= (~TURNBASED);
  gTacticalStatus.uiFlags &= (~NPC_TEAM_DEAD);
  gTacticalStatus.uiFlags &= (~PLAYER_TEAM_DEAD);

  // Force load of guys
  SetLoadOverrideParams(TRUE, FALSE, NULL);

  // Load random level
  // Dont't do first three levels
  if (gTacticalStatus.fNOTDOLASTDEMO) {
    ubNumScenes--;
  }

  do {
    ubNewScene = START_DEMO_SCENE + (uint8_t)Random(ubNumScenes);

  } while (ubNewScene == gubCurrentScene);

  gubCurrentScene = ubNewScene;

  // Set demo timer
  gTacticalStatus.uiTimeSinceDemoOn = GetJA2Clock();

  gfSGPInputReceived = FALSE;

  gTacticalStatus.fGoingToEnterDemo = FALSE;

  return (INIT_SCREEN);
}

char *GetSceneFilename() { return (gzLevelFilenames[gubCurrentScene]); }

extern BOOLEAN InternalOkayToAddStructureToWorld(int16_t sBaseGridNo, int8_t bLevel,
                                                 struct DB_STRUCTURE_REF *pDBStructureRef,
                                                 int16_t sExclusionID, BOOLEAN fIgnorePeople);

// NB if making changes don't forget to update NewOKDestinationAndDirection
int16_t NewOKDestination(struct SOLDIERTYPE *pCurrSoldier, int16_t sGridNo, BOOLEAN fPeopleToo,
                         int8_t bLevel) {
  uint8_t bPerson;
  struct STRUCTURE *pStructure;
  int16_t sDesiredLevel;
  BOOLEAN fOKCheckStruct;

  if (!GridNoOnVisibleWorldTile(sGridNo)) {
    return (TRUE);
  }

  if (fPeopleToo && (bPerson = WhoIsThere2(sGridNo, bLevel)) != NO_SOLDIER) {
    // we could be multitiled... if the person there is us, and the gridno is not
    // our base gridno, skip past these checks
    if (!(bPerson == pCurrSoldier->ubID && sGridNo != pCurrSoldier->sGridNo)) {
      if (pCurrSoldier->bTeam == gbPlayerNum) {
        if ((Menptr[bPerson].bVisible >= 0) || (gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
          return (FALSE);  // if someone there it's NOT OK
      } else {
        return (FALSE);  // if someone there it's NOT OK
      }
    }
  }

  // Check structure database
  if ((pCurrSoldier->uiStatusFlags & SOLDIER_MULTITILE) && !(gfEstimatePath)) {
    uint16_t usAnimSurface;
    struct STRUCTURE_FILE_REF *pStructureFileRef;
    BOOLEAN fOk;
    int8_t bLoop;
    uint16_t usStructureID = INVALID_STRUCTURE_ID;

    // this could be kinda slow...

    // Get animation surface...
    usAnimSurface = DetermineSoldierAnimationSurface(pCurrSoldier, pCurrSoldier->usUIMovementMode);
    // Get structure ref...
    pStructureFileRef =
        GetAnimationStructureRef(pCurrSoldier->ubID, usAnimSurface, pCurrSoldier->usUIMovementMode);

    // opposite directions should be mirrors, so only check 4
    if (pStructureFileRef) {
      // if ANY direction is valid, consider moving here valid
      for (bLoop = 0; bLoop < NUM_WORLD_DIRECTIONS; bLoop++) {
        // ATE: Only if we have a levelnode...
        if (pCurrSoldier->pLevelNode != NULL && pCurrSoldier->pLevelNode->pStructureData != NULL) {
          usStructureID = pCurrSoldier->pLevelNode->pStructureData->usStructureID;
        } else {
          usStructureID = INVALID_STRUCTURE_ID;
        }

        fOk = InternalOkayToAddStructureToWorld(sGridNo, bLevel,
                                                &(pStructureFileRef->pDBStructureRef[bLoop]),
                                                usStructureID, (BOOLEAN)!fPeopleToo);
        if (fOk) {
          return (TRUE);
        }
      }
    }
    return (FALSE);
  } else {
    // quick test
    if (gpWorldLevelData[sGridNo].pStructureHead != NULL) {
      // Something is here, check obstruction in future
      if (bLevel == 0) {
        sDesiredLevel = STRUCTURE_ON_GROUND;
      } else {
        sDesiredLevel = STRUCTURE_ON_ROOF;
      }

      pStructure = FindStructure(sGridNo, STRUCTURE_BLOCKSMOVES);

      // ATE: If we are trying to get a path to an exit grid AND
      // we are a cave....still allow this..
      // if ( pStructure && gfPlotPathToExitGrid && pStructure->fFlags & STRUCTURE_CAVEWALL )
      if (pStructure && gfPlotPathToExitGrid) {
        pStructure = NULL;
      }

      while (pStructure != NULL) {
        if (!(pStructure->fFlags & STRUCTURE_PASSABLE)) {
          fOKCheckStruct = TRUE;

          // Check if this is a multi-tile
          if ((pStructure->fFlags & STRUCTURE_MOBILE) &&
              (pCurrSoldier->uiStatusFlags & SOLDIER_MULTITILE)) {
            // Check IDs with soldier's ID
            if (pCurrSoldier->pLevelNode != NULL &&
                pCurrSoldier->pLevelNode->pStructureData != NULL &&
                pCurrSoldier->pLevelNode->pStructureData->usStructureID ==
                    pStructure->usStructureID) {
              fOKCheckStruct = FALSE;
            }
          }

          if (fOKCheckStruct) {
            if (pStructure->sCubeOffset == sDesiredLevel) {
              return (FALSE);
            }
          }
        }

        pStructure = FindNextStructure(pStructure, STRUCTURE_BLOCKSMOVES);
      }
    }
  }
  return (TRUE);
}

// NB if making changes don't forget to update NewOKDestination
int16_t NewOKDestinationAndDirection(struct SOLDIERTYPE *pCurrSoldier, int16_t sGridNo,
                                     int8_t bDirection, BOOLEAN fPeopleToo, int8_t bLevel) {
  uint8_t bPerson;
  struct STRUCTURE *pStructure;
  int16_t sDesiredLevel;
  BOOLEAN fOKCheckStruct;

  if (fPeopleToo && (bPerson = WhoIsThere2(sGridNo, bLevel)) != NO_SOLDIER) {
    // we could be multitiled... if the person there is us, and the gridno is not
    // our base gridno, skip past these checks
    if (!(bPerson == pCurrSoldier->ubID && sGridNo != pCurrSoldier->sGridNo)) {
      if (pCurrSoldier->bTeam == gbPlayerNum) {
        if ((Menptr[bPerson].bVisible >= 0) || (gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
          return (FALSE);  // if someone there it's NOT OK
      } else {
        return (FALSE);  // if someone there it's NOT OK
      }
    }
  }

  // Check structure database
  if ((pCurrSoldier->uiStatusFlags & SOLDIER_MULTITILE) && !(gfEstimatePath)) {
    uint16_t usAnimSurface;
    struct STRUCTURE_FILE_REF *pStructureFileRef;
    BOOLEAN fOk;
    int8_t bLoop;
    uint16_t usStructureID = INVALID_STRUCTURE_ID;

    // this could be kinda slow...

    // Get animation surface...
    usAnimSurface = DetermineSoldierAnimationSurface(pCurrSoldier, pCurrSoldier->usUIMovementMode);
    // Get structure ref...
    pStructureFileRef =
        GetAnimationStructureRef(pCurrSoldier->ubID, usAnimSurface, pCurrSoldier->usUIMovementMode);

    if (pStructureFileRef) {
      // use the specified direction for checks
      bLoop = bDirection;

      {
        // ATE: Only if we have a levelnode...
        if (pCurrSoldier->pLevelNode != NULL && pCurrSoldier->pLevelNode->pStructureData != NULL) {
          usStructureID = pCurrSoldier->pLevelNode->pStructureData->usStructureID;
        }

        fOk = InternalOkayToAddStructureToWorld(
            sGridNo, pCurrSoldier->bLevel,
            &(pStructureFileRef->pDBStructureRef[gOneCDirection[bLoop]]), usStructureID,
            (BOOLEAN)!fPeopleToo);
        if (fOk) {
          return (TRUE);
        }
      }
    }
    return (FALSE);
  } else {
    // quick test
    if (gpWorldLevelData[sGridNo].pStructureHead != NULL) {
      // Something is here, check obstruction in future
      if (bLevel == 0) {
        sDesiredLevel = STRUCTURE_ON_GROUND;
      } else {
        sDesiredLevel = STRUCTURE_ON_ROOF;
      }

      pStructure = FindStructure(sGridNo, STRUCTURE_BLOCKSMOVES);

      // ATE: If we are trying to get a path to an exit grid AND
      // we are a cave....still allow this..
      // if ( pStructure && gfPlotPathToExitGrid && pStructure->fFlags & STRUCTURE_CAVEWALL )
      if (pStructure && gfPlotPathToExitGrid) {
        pStructure = NULL;
      }

      while (pStructure != NULL) {
        if (!(pStructure->fFlags & STRUCTURE_PASSABLE)) {
          fOKCheckStruct = TRUE;

          // Check if this is a multi-tile
          if ((pStructure->fFlags & STRUCTURE_MOBILE) &&
              (pCurrSoldier->uiStatusFlags & SOLDIER_MULTITILE)) {
            // Check IDs with soldier's ID
            if (pCurrSoldier->pLevelNode != NULL &&
                pCurrSoldier->pLevelNode->pStructureData != NULL &&
                pCurrSoldier->pLevelNode->pStructureData->usStructureID ==
                    pStructure->usStructureID) {
              fOKCheckStruct = FALSE;
            }
          }

          if (fOKCheckStruct) {
            if (pStructure->sCubeOffset == sDesiredLevel) {
              return (FALSE);
            }
          }
        }

        pStructure = FindNextStructure(pStructure, STRUCTURE_BLOCKSMOVES);
      }
    }
  }
  return (TRUE);
}

// Kris:
BOOLEAN FlatRoofAboveGridNo(int32_t iMapIndex) {
  struct LEVELNODE *pRoof;
  uint32_t uiTileType;
  pRoof = gpWorldLevelData[iMapIndex].pRoofHead;
  while (pRoof) {
    if (pRoof->usIndex != NO_TILE) {
      GetTileType(pRoof->usIndex, &uiTileType);
      if (uiTileType >= FIRSTROOF && uiTileType <= LASTROOF) return TRUE;
    }
    pRoof = pRoof->pNext;
  }
  return FALSE;
}

// Kris:
// ASSUMPTION:  This function assumes that we are checking on behalf of a single tiled merc.  This
// function 						 should not be used for checking on behalf
// of multi-tiled "things". I wrote this function for editor use.  I don't personally care about
// multi-tiled stuff. All I want to know is whether or not I can put a merc here.  In most cases, I
// won't be dealing with multi-tiled mercs, and the rarity doesn't justify the needs.  I just wrote
// this to be quick and dirty, and I don't expect it to perform perfectly in all situations.
BOOLEAN IsLocationSittable(int32_t iMapIndex, BOOLEAN fOnRoof) {
  struct STRUCTURE *pStructure;
  int16_t sDesiredLevel;
  if (WhoIsThere2((int16_t)iMapIndex, 0) != NO_SOLDIER) return FALSE;
  // Locations on roofs without a roof is not possible, so
  // we convert the onroof intention to ground.
  if (fOnRoof && !FlatRoofAboveGridNo(iMapIndex)) fOnRoof = FALSE;
  // Check structure database
  if (gpWorldLevelData[iMapIndex].pStructureHead) {
    // Something is here, check obstruction in future
    sDesiredLevel = fOnRoof ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
    pStructure = FindStructure((int16_t)iMapIndex, STRUCTURE_BLOCKSMOVES);
    while (pStructure) {
      if (!(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel)
        return FALSE;
      pStructure = FindNextStructure(pStructure, STRUCTURE_BLOCKSMOVES);
    }
  }
  return TRUE;
}

BOOLEAN IsLocationSittableExcludingPeople(int32_t iMapIndex, BOOLEAN fOnRoof) {
  struct STRUCTURE *pStructure;
  int16_t sDesiredLevel;

  // Locations on roofs without a roof is not possible, so
  // we convert the onroof intention to ground.
  if (fOnRoof && !FlatRoofAboveGridNo(iMapIndex)) fOnRoof = FALSE;
  // Check structure database
  if (gpWorldLevelData[iMapIndex].pStructureHead) {
    // Something is here, check obstruction in future
    sDesiredLevel = fOnRoof ? STRUCTURE_ON_ROOF : STRUCTURE_ON_GROUND;
    pStructure = FindStructure((int16_t)iMapIndex, STRUCTURE_BLOCKSMOVES);
    while (pStructure) {
      if (!(pStructure->fFlags & STRUCTURE_PASSABLE) && pStructure->sCubeOffset == sDesiredLevel)
        return FALSE;
      pStructure = FindNextStructure(pStructure, STRUCTURE_BLOCKSMOVES);
    }
  }
  return TRUE;
}

BOOLEAN TeamMemberNear(int8_t bTeam, int16_t sGridNo, int32_t iRange) {
  uint8_t bLoop;
  struct SOLDIERTYPE *pSoldier;

  for (bLoop = gTacticalStatus.Team[bTeam].bFirstID, pSoldier = MercPtrs[bLoop];
       bLoop <= gTacticalStatus.Team[bTeam].bLastID; bLoop++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && (pSoldier->bLife >= OKLIFE) &&
        !(pSoldier->uiStatusFlags & SOLDIER_GASSED)) {
      if (PythSpacesAway(pSoldier->sGridNo, sGridNo) <= iRange) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

int16_t FindAdjacentGridEx(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t *pubDirection,
                           int16_t *psAdjustedGridNo, BOOLEAN fForceToPerson, BOOLEAN fDoor) {
  // psAdjustedGridNo gets the original gridno or the new one if updated
  // It will ONLY be updated IF we were over a merc, ( it's updated to their gridno )
  // pubDirection gets the direction to the final gridno
  // fForceToPerson: forces the grid under consideration to be the one occupiedby any target
  // in that location, because we could be passed a gridno based on the overlap of soldier's graphic
  // fDoor determines whether special door-handling code should be used (for interacting with doors)

  int16_t sDistance = 0;
  int16_t sDirs[4] = {NORTH, EAST, SOUTH, WEST};
  int32_t cnt;
  int16_t sClosest = NOWHERE, sSpot, sOkTest;
  int16_t sCloseGridNo = NOWHERE;
  uint32_t uiMercFlags;
  uint16_t usSoldierIndex;
  uint8_t ubDir;
  struct STRUCTURE *pDoor;
  // struct STRUCTURE                            *pWall;
  uint8_t ubWallOrientation;
  BOOLEAN fCheckGivenGridNo = TRUE;
  uint8_t ubTestDirection;
  EXITGRID ExitGrid;

  // Set default direction
  if (pubDirection) {
    *pubDirection = pSoldier->bDirection;
  }

  // CHECK IF WE WANT TO FORCE GRIDNO TO PERSON
  if (psAdjustedGridNo != NULL) {
    *psAdjustedGridNo = sGridNo;
  }

  // CHECK IF IT'S THE SAME ONE AS WE'RE ON, IF SO, RETURN THAT!
  if (pSoldier->sGridNo == sGridNo && !FindStructure(sGridNo, (STRUCTURE_SWITCH))) {
    // OK, if we are looking for a door, it may be in the same tile as us, so find the direction we
    // have to face to get to the door, not just our initial direction...
    // If we are in the same tile as a switch, we can NEVER pull it....
    if (fDoor) {
      // This can only happen if a door was to the south to east of us!

      // Do south!
      // sSpot = NewGridNo( sGridNo, DirectionInc( SOUTH ) );

      // ATE: Added: Switch behave EXACTLY like doors
      pDoor = FindStructure(sGridNo, (STRUCTURE_ANYDOOR));

      if (pDoor != NULL) {
        // Get orinetation
        ubWallOrientation = pDoor->ubWallOrientation;

        if (ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT) {
          // To the south!
          sSpot = NewGridNo(sGridNo, DirectionInc(SOUTH));
          if (pubDirection) {
            (*pubDirection) = (uint8_t)GetDirectionFromGridNo(sSpot, pSoldier);
          }
        }

        if (ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT) {
          // TO the east!
          sSpot = NewGridNo(sGridNo, DirectionInc(EAST));
          if (pubDirection) {
            (*pubDirection) = (uint8_t)GetDirectionFromGridNo(sSpot, pSoldier);
          }
        }
      }
    }

    // Use soldier's direction
    return (sGridNo);
  }

  // Look for a door!
  if (fDoor) {
    pDoor = FindStructure(sGridNo, (STRUCTURE_ANYDOOR | STRUCTURE_SWITCH));
  } else {
    pDoor = NULL;
  }

  if (fForceToPerson) {
    if (FindSoldier(sGridNo, &usSoldierIndex, &uiMercFlags, FIND_SOLDIER_GRIDNO)) {
      sGridNo = MercPtrs[usSoldierIndex]->sGridNo;
      if (psAdjustedGridNo != NULL) {
        *psAdjustedGridNo = sGridNo;

        // Use direction to this guy!
        if (pubDirection) {
          (*pubDirection) = (uint8_t)GetDirectionFromGridNo(sGridNo, pSoldier);
        }
      }
    }
  }

  if ((sOkTest = NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) >
      0)  // no problem going there! nobody on it!
  {
    // OK, if we are looking to goto a switch, ignore this....
    if (pDoor) {
      if (pDoor->fFlags & STRUCTURE_SWITCH) {
        // Don't continuel
        fCheckGivenGridNo = FALSE;
      }
    }

    // If there is an exit grid....
    if (GetExitGrid(sGridNo, &ExitGrid)) {
      // Don't continuel
      fCheckGivenGridNo = FALSE;
    }

    if (fCheckGivenGridNo) {
      sDistance = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                           (int16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                           pSoldier->bActionPoints);

      if (sDistance > 0) {
        if (sDistance < sClosest) {
          sClosest = sDistance;
          sCloseGridNo = (int16_t)sGridNo;
        }
      }
    }
  }

  for (cnt = 0; cnt < 4; cnt++) {
    // MOVE OUT TWO DIRECTIONS
    sSpot = NewGridNo(sGridNo, DirectionInc(sDirs[cnt]));

    ubTestDirection = (uint8_t)sDirs[cnt];

    // For switches, ALLOW them to walk through walls to reach it....
    if (pDoor && pDoor->fFlags & STRUCTURE_SWITCH) {
      ubTestDirection = gOppositeDirection[ubTestDirection];
    }

    if (fDoor) {
      if (gubWorldMovementCosts[sSpot][ubTestDirection][pSoldier->bLevel] >= TRAVELCOST_BLOCKED) {
        // obstacle or wall there!
        continue;
      }
    } else {
      // this function returns original MP cost if not a door cost
      if (DoorTravelCost(pSoldier, sSpot,
                         gubWorldMovementCosts[sSpot][ubTestDirection][pSoldier->bLevel], FALSE,
                         NULL) >= TRAVELCOST_BLOCKED) {
        // obstacle or wall there!
        continue;
      }
    }

    // Eliminate some directions if we are looking at doors!
    if (pDoor != NULL) {
      // Get orinetation
      ubWallOrientation = pDoor->ubWallOrientation;

      // Refuse the south and north and west  directions if our orientation is top-right
      if (ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT) {
        if (sDirs[cnt] == NORTH || sDirs[cnt] == WEST || sDirs[cnt] == SOUTH) continue;
      }

      // Refuse the north and west and east directions if our orientation is top-right
      if (ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT) {
        if (sDirs[cnt] == NORTH || sDirs[cnt] == WEST || sDirs[cnt] == EAST) continue;
      }
    }

    // If this spot is our soldier's gridno use that!
    if (sSpot == pSoldier->sGridNo) {
      // Use default diurection ) soldier's direction )

      // OK, at least get direction to face......
      // Defaults to soldier's facing dir unless we change it!
      // if ( pDoor != NULL )
      {
        // Use direction to the door!
        if (pubDirection) {
          (*pubDirection) = (uint8_t)GetDirectionFromGridNo(sGridNo, pSoldier);
        }
      }
      return (sSpot);
    }

    // don't store path, just measure it
    ubDir = (uint8_t)GetDirectionToGridNoFromGridNo(sSpot, sGridNo);

    if ((NewOKDestinationAndDirection(pSoldier, sSpot, ubDir, TRUE, pSoldier->bLevel) > 0) &&
        ((sDistance = PlotPath(pSoldier, sSpot, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                               (int16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                               pSoldier->bActionPoints)) > 0)) {
      if (sDistance < sClosest) {
        sClosest = sDistance;
        sCloseGridNo = (int16_t)sSpot;
      }
    }
  }

  if (sClosest != NOWHERE) {
    // Take last direction and use opposite!
    // This will be usefull for ours and AI mercs

    // If our gridno is the same ( which can be if we are look at doors )
    if (sGridNo == sCloseGridNo) {
      if (pubDirection) {
        // ATE: Only if we have a valid door!
        if (pDoor) {
          switch (pDoor->pDBStructureRef->pDBStructure->ubWallOrientation) {
            case OUTSIDE_TOP_LEFT:
            case INSIDE_TOP_LEFT:

              *pubDirection = SOUTH;
              break;

            case OUTSIDE_TOP_RIGHT:
            case INSIDE_TOP_RIGHT:

              *pubDirection = EAST;
              break;
          }
        }
      }
    } else {
      // Calculate direction if our gridno is different....
      ubDir = (uint8_t)GetDirectionToGridNoFromGridNo(sCloseGridNo, sGridNo);
      if (pubDirection) {
        *pubDirection = ubDir;
      }
    }
    // if ( psAdjustedGridNo != NULL )
    //{
    //		(*psAdjustedGridNo) = sCloseGridNo;
    //}
    if (sCloseGridNo == NOWHERE) {
      return (-1);
    }
    return (sCloseGridNo);
  } else
    return (-1);
}

int16_t FindNextToAdjacentGridEx(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                 uint8_t *pubDirection, int16_t *psAdjustedGridNo,
                                 BOOLEAN fForceToPerson, BOOLEAN fDoor) {
  // This function works in a similar way as FindAdjacentGridEx, but looks for a location 2 tiles
  // away

  // psAdjustedGridNo gets the original gridno or the new one if updated
  // pubDirection gets the direction to the final gridno
  // fForceToPerson: forces the grid under consideration to be the one occupiedby any target
  // in that location, because we could be passed a gridno based on the overlap of soldier's graphic
  // fDoor determines whether special door-handling code should be used (for interacting with doors)

  int16_t sDistance = 0;
  int16_t sDirs[4] = {NORTH, EAST, SOUTH, WEST};
  int32_t cnt;
  int16_t sClosest = WORLD_MAX, sSpot, sSpot2, sOkTest;
  int16_t sCloseGridNo = NOWHERE;
  uint32_t uiMercFlags;
  uint16_t usSoldierIndex;
  uint8_t ubDir;
  struct STRUCTURE *pDoor;
  uint8_t ubWallOrientation;
  BOOLEAN fCheckGivenGridNo = TRUE;
  uint8_t ubTestDirection;
  uint8_t ubWhoIsThere;

  // CHECK IF WE WANT TO FORCE GRIDNO TO PERSON
  if (psAdjustedGridNo != NULL) {
    *psAdjustedGridNo = sGridNo;
  }

  // CHECK IF IT'S THE SAME ONE AS WE'RE ON, IF SO, RETURN THAT!
  if (pSoldier->sGridNo == sGridNo) {
    *pubDirection = pSoldier->bDirection;
    return (sGridNo);
  }

  // Look for a door!
  if (fDoor) {
    pDoor = FindStructure(sGridNo, (STRUCTURE_ANYDOOR | STRUCTURE_SWITCH));
  } else {
    pDoor = NULL;
  }

  if (fForceToPerson) {
    if (FindSoldier(sGridNo, &usSoldierIndex, &uiMercFlags, FIND_SOLDIER_GRIDNO)) {
      sGridNo = MercPtrs[usSoldierIndex]->sGridNo;
      if (psAdjustedGridNo != NULL) {
        *psAdjustedGridNo = sGridNo;
      }
    }
  }

  if ((sOkTest = NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) >
      0)  // no problem going there! nobody on it!
  {
    // OK, if we are looking to goto a switch, ignore this....
    if (pDoor) {
      if (pDoor->fFlags & STRUCTURE_SWITCH) {
        // Don't continuel
        fCheckGivenGridNo = FALSE;
      }
    }

    if (fCheckGivenGridNo) {
      sDistance = PlotPath(pSoldier, sGridNo, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                           (int16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                           pSoldier->bActionPoints);

      if (sDistance > 0) {
        if (sDistance < sClosest) {
          sClosest = sDistance;
          sCloseGridNo = (int16_t)sGridNo;
        }
      }
    }
  }

  for (cnt = 0; cnt < 4; cnt++) {
    // MOVE OUT TWO DIRECTIONS
    sSpot = NewGridNo(sGridNo, DirectionInc(sDirs[cnt]));

    ubTestDirection = (uint8_t)sDirs[cnt];

    if (pDoor && pDoor->fFlags & STRUCTURE_SWITCH) {
      ubTestDirection = gOppositeDirection[ubTestDirection];
    }

    if (gubWorldMovementCosts[sSpot][ubTestDirection][pSoldier->bLevel] >= TRAVELCOST_BLOCKED) {
      // obstacle or wall there!
      continue;
    }

    ubWhoIsThere = WhoIsThere2(sSpot, pSoldier->bLevel);
    if (ubWhoIsThere != NOBODY && ubWhoIsThere != GetSolID(pSoldier)) {
      // skip this direction b/c it's blocked by another merc!
      continue;
    }

    // Eliminate some directions if we are looking at doors!
    if (pDoor != NULL) {
      // Get orinetation
      ubWallOrientation = pDoor->ubWallOrientation;

      // Refuse the south and north and west  directions if our orientation is top-right
      if (ubWallOrientation == OUTSIDE_TOP_RIGHT || ubWallOrientation == INSIDE_TOP_RIGHT) {
        if (sDirs[cnt] == NORTH || sDirs[cnt] == WEST || sDirs[cnt] == SOUTH) continue;
      }

      // Refuse the north and west and east directions if our orientation is top-right
      if (ubWallOrientation == OUTSIDE_TOP_LEFT || ubWallOrientation == INSIDE_TOP_LEFT) {
        if (sDirs[cnt] == NORTH || sDirs[cnt] == WEST || sDirs[cnt] == EAST) continue;
      }
    }

    // first tile is okay, how about the second?
    sSpot2 = NewGridNo(sSpot, DirectionInc(sDirs[cnt]));
    if (gubWorldMovementCosts[sSpot2][sDirs[cnt]][pSoldier->bLevel] >= TRAVELCOST_BLOCKED ||
        DoorTravelCost(pSoldier, sSpot2,
                       gubWorldMovementCosts[sSpot2][sDirs[cnt]][pSoldier->bLevel],
                       (BOOLEAN)(pSoldier->bTeam == gbPlayerNum),
                       NULL) == TRAVELCOST_DOOR)  // closed door blocks!
    {
      // obstacle or wall there!
      continue;
    }

    ubWhoIsThere = WhoIsThere2(sSpot2, pSoldier->bLevel);
    if (ubWhoIsThere != NOBODY && ubWhoIsThere != GetSolID(pSoldier)) {
      // skip this direction b/c it's blocked by another merc!
      continue;
    }

    sSpot = sSpot2;

    // If this spot is our soldier's gridno use that!
    if (sSpot == pSoldier->sGridNo) {
      if (pubDirection) {
        (*pubDirection) = (uint8_t)GetDirectionFromGridNo(sGridNo, pSoldier);
      }
      //*pubDirection = pSoldier->bDirection;
      return (sSpot);
    }

    ubDir = (uint8_t)GetDirectionToGridNoFromGridNo(sSpot, sGridNo);

    // don't store path, just measure it
    if ((NewOKDestinationAndDirection(pSoldier, sSpot, ubDir, TRUE, pSoldier->bLevel) > 0) &&
        ((sDistance = PlotPath(pSoldier, sSpot, NO_COPYROUTE, NO_PLOT, TEMPORARY,
                               (int16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                               pSoldier->bActionPoints)) > 0)) {
      if (sDistance < sClosest) {
        sClosest = sDistance;
        sCloseGridNo = (int16_t)sSpot;
      }
    }
  }

  if (sClosest != NOWHERE) {
    // Take last direction and use opposite!
    // This will be usefull for ours and AI mercs

    // If our gridno is the same ( which can be if we are look at doors )
    if (sGridNo == sCloseGridNo) {
      if (pubDirection) {
        // ATE: Only if we have a valid door!
        if (pDoor) {
          switch (pDoor->pDBStructureRef->pDBStructure->ubWallOrientation) {
            case OUTSIDE_TOP_LEFT:
            case INSIDE_TOP_LEFT:

              *pubDirection = SOUTH;
              break;

            case OUTSIDE_TOP_RIGHT:
            case INSIDE_TOP_RIGHT:

              *pubDirection = EAST;
              break;
          }
        }
      }
    } else {
      // Calculate direction if our gridno is different....
      ubDir = (uint8_t)GetDirectionToGridNoFromGridNo(sCloseGridNo, sGridNo);
      if (pubDirection) {
        *pubDirection = ubDir;
      }
    }

    if (sCloseGridNo == NOWHERE) {
      return (-1);
    }
    return (sCloseGridNo);
  } else
    return (-1);

  /*
  if (sCloseGridNo != NOWHERE)
{
           // Take last direction and use opposite!
           // This will be usefull for ours and AI mercs

           // If our gridno is the same ( which can be if we are look at doors )
           if ( sGridNo == sCloseGridNo )
           {
                          switch( pDoor->pDBStructureRef->pDBStructure->ubWallOrientation )
                          {
                                  case OUTSIDE_TOP_LEFT:
                                  case INSIDE_TOP_LEFT:

                                   *pubDirection = SOUTH;
                                          break;

                                  case OUTSIDE_TOP_RIGHT:
                                  case INSIDE_TOP_RIGHT:

                                   *pubDirection = EAST;
                                          break;
                          }
           }
           else
           {
                          // Calculate direction if our gridno is different....
                          ubDir = (uint8_t)GetDirectionToGridNoFromGridNo( sCloseGridNo, sGridNo );
                          *pubDirection = ubDir;
           }
           return( sCloseGridNo );
}
  else
          return( -1 );
  */
}

int16_t FindAdjacentPunchTarget(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTargetSoldier,
                                int16_t *psAdjustedTargetGridNo, uint8_t *pubDirection) {
  int16_t cnt;
  int16_t sSpot;
  uint8_t ubGuyThere;

  for (cnt = 0; cnt < NUM_WORLD_DIRECTIONS; cnt++) {
    sSpot = (int16_t)NewGridNo(pSoldier->sGridNo, DirectionInc(cnt));

    if (DoorTravelCost(pSoldier, sSpot, gubWorldMovementCosts[sSpot][cnt][pSoldier->bLevel], FALSE,
                       NULL) >= TRAVELCOST_BLOCKED) {
      // blocked!
      continue;
    }

    // Check for who is there...
    ubGuyThere = WhoIsThere2(sSpot, pSoldier->bLevel);

    if (pTargetSoldier != NULL && ubGuyThere == pTargetSoldier->ubID) {
      // We've got a guy here....
      // Who is the one we want......
      *psAdjustedTargetGridNo = pTargetSoldier->sGridNo;
      *pubDirection = (uint8_t)cnt;
      return (sSpot);
    }
  }

  return (NOWHERE);
}

BOOLEAN UIOKMoveDestination(struct SOLDIERTYPE *pSoldier, uint16_t usMapPos) {
  BOOLEAN fVisible;

  // Check if a hidden tile exists but is not revealed
  if (DoesGridnoContainHiddenStruct(usMapPos, &fVisible)) {
    if (!fVisible) {
      // The player thinks this is OK!
      return (TRUE);
    }
  }

  if (!NewOKDestination(pSoldier, usMapPos, FALSE, (int8_t)gsInterfaceLevel)) {
    return (FALSE);
  }

  // ATE: If we are a robot, see if we are being validly controlled...
  if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
    if (!CanRobotBeControlled(pSoldier)) {
      // Display message that robot cannot be controlled....
      return (2);
    }
  }

  // ATE: Experiment.. take out
  // else if ( IsRoofVisible( usMapPos ) && gsInterfaceLevel == 0 )
  //{
  //	return( FALSE );
  //}

  return (TRUE);
}

void HandleTeamServices(uint8_t ubTeamNum) {
  int32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier, *pTargetSoldier;
  uint32_t uiPointsUsed;
  uint16_t usSoldierIndex;
  uint16_t usKitPts;
  int8_t bSlot;
  BOOLEAN fDone;

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[ubTeamNum].bFirstID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[ubTeamNum].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive && pTeamSoldier->bInSector) {
      fDone = FALSE;
      // Check for different events!
      // FOR DOING AID
      if (pTeamSoldier->usAnimState == GIVING_AID) {
        // Get victim pointer
        usSoldierIndex = WhoIsThere2(pTeamSoldier->sTargetGridNo, pTeamSoldier->bLevel);
        if (usSoldierIndex != NOBODY) {
          pTargetSoldier = MercPtrs[usSoldierIndex];

          if (pTargetSoldier->ubServiceCount) {
            usKitPts = TotalPoints(&(pTeamSoldier->inv[HANDPOS]));

            uiPointsUsed = SoldierDressWound(pTeamSoldier, pTargetSoldier, usKitPts, usKitPts);

            // Determine if they are all banagded
            if (!pTargetSoldier->bBleeding && pTargetSoldier->bLife >= OKLIFE) {
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[MERC_IS_ALL_BANDAGED_STR],
                        pTargetSoldier->name);

              // Cancel all services for this guy!
              ReceivingSoldierCancelServices(pTargetSoldier);
              fDone = TRUE;
            }

            UseKitPoints(&(pTeamSoldier->inv[HANDPOS]), (uint16_t)uiPointsUsed, pTeamSoldier);

            // Get new total
            usKitPts = TotalPoints(&(pTeamSoldier->inv[HANDPOS]));

            // WHETHER OR NOT recipient is all bandaged, check if we've used them up!
            if (usKitPts <= 0)  // no more bandages
            {
              if (fDone) {
                // don't swap if we're done
                bSlot = NO_SLOT;
              } else {
                bSlot = FindObj(pTeamSoldier, FIRSTAIDKIT);
                if (bSlot == NO_SLOT) {
                  bSlot = FindObj(pTeamSoldier, MEDICKIT);
                }
              }
              if (bSlot != NO_SLOT) {
                SwapObjs(&(pTeamSoldier->inv[HANDPOS]), &(pTeamSoldier->inv[bSlot]));
              } else {
                ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
                          TacticalStr[MERC_IS_OUT_OF_BANDAGES_STR], pTeamSoldier->name);
                GivingSoldierCancelServices(pTeamSoldier);

                if (!gTacticalStatus.fAutoBandageMode) {
                  DoMercBattleSound(pTeamSoldier, (int8_t)(BATTLE_SOUND_CURSE1));
                }
              }
            }
          }
        }
      }
    }
  }
}

void HandlePlayerServices(struct SOLDIERTYPE *pTeamSoldier) {
  struct SOLDIERTYPE *pTargetSoldier;
  uint32_t uiPointsUsed;
  uint16_t usSoldierIndex;
  uint16_t usKitPts;
  int8_t bSlot;
  BOOLEAN fDone = FALSE;

  if (pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive) {
    // Check for different events!
    // FOR DOING AID
    if (pTeamSoldier->usAnimState == GIVING_AID) {
      // Get victim pointer
      usSoldierIndex = WhoIsThere2(pTeamSoldier->sTargetGridNo, pTeamSoldier->bLevel);

      if (usSoldierIndex != NOBODY) {
        pTargetSoldier = MercPtrs[usSoldierIndex];

        if (pTargetSoldier->ubServiceCount) {
          usKitPts = TotalPoints(&(pTeamSoldier->inv[HANDPOS]));

          uiPointsUsed = SoldierDressWound(pTeamSoldier, pTargetSoldier, usKitPts, usKitPts);

          // Determine if they are all banagded
          if (!pTargetSoldier->bBleeding && pTargetSoldier->bLife >= OKLIFE) {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[MERC_IS_ALL_BANDAGED_STR],
                      pTargetSoldier->name);

            // Cancel all services for this guy!
            ReceivingSoldierCancelServices(pTargetSoldier);
            fDone = TRUE;
          }

          UseKitPoints(&(pTeamSoldier->inv[HANDPOS]), (uint16_t)uiPointsUsed, pTeamSoldier);

          // Get new total
          usKitPts = TotalPoints(&(pTeamSoldier->inv[HANDPOS]));

          // WHETHER OR NOT recipient is all bandaged, check if we've used them up!
          if (usKitPts <= 0)  // no more bandages
          {
            if (fDone) {
              // don't swap if we're done
              bSlot = NO_SLOT;
            } else {
              bSlot = FindObj(pTeamSoldier, FIRSTAIDKIT);
              if (bSlot == NO_SLOT) {
                bSlot = FindObj(pTeamSoldier, MEDICKIT);
              }
            }

            if (bSlot != NO_SLOT) {
              SwapObjs(&(pTeamSoldier->inv[HANDPOS]), &(pTeamSoldier->inv[bSlot]));
            } else {
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
                        TacticalStr[MERC_IS_OUT_OF_BANDAGES_STR], pTeamSoldier->name);
              GivingSoldierCancelServices(pTeamSoldier);

              if (!gTacticalStatus.fAutoBandageMode) {
                DoMercBattleSound(pTeamSoldier, (int8_t)(BATTLE_SOUND_CURSE1));
              }
            }
          }
        }
      }
    }
  }
}

void CommonEnterCombatModeCode() {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  gTacticalStatus.uiFlags |= INCOMBAT;

  // gTacticalStatus.ubAttackBusyCount = 0;

  // Reset num enemies fought flag...
  memset(&(gTacticalStatus.bNumFoughtInBattle), 0, MAXTEAMS);
  gTacticalStatus.ubLastBattleSectorX = (uint8_t)gWorldSectorX;
  gTacticalStatus.ubLastBattleSectorY = (uint8_t)gWorldSectorY;
  gTacticalStatus.fLastBattleWon = FALSE;
  gTacticalStatus.fItemsSeenOnAttack = FALSE;

  // ATE: If we have an item pointer end it!
  CancelItemPointer();

  ResetInterfaceAndUI();
  ResetMultiSelection();

  // OK, loop thorugh all guys and stop them!
  // Loop through all mercs and make go
  for (pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++) {
    if (IsSolActive(pSoldier)) {
      if (pSoldier->bInSector && pSoldier->ubBodyType != CROW) {
        // Set some flags for quotes
        pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_IN_SHIT);
        pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_MULTIPLE_CREATURES);

        // Hault!
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

        // END AI actions
        CancelAIAction(pSoldier, TRUE);

        // turn off AI controlled flag
        pSoldier->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;

        // Check if this guy is an enemy....
        CheckForPotentialAddToBattleIncrement(pSoldier);

        // If guy is sleeping, wake him up!
        if (pSoldier->fMercAsleep == TRUE) {
          ChangeSoldierState(pSoldier, WKAEUP_FROM_SLEEP, 1, TRUE);
        }

        // ATE: Refresh APs
        CalcNewActionPoints(pSoldier);

        if (GetSolProfile(pSoldier) != NO_PROFILE) {
          if (pSoldier->bTeam == CIV_TEAM && pSoldier->bNeutral) {
            // only set precombat gridno if unset
            if (gMercProfiles[GetSolProfile(pSoldier)].sPreCombatGridNo == 0 ||
                gMercProfiles[GetSolProfile(pSoldier)].sPreCombatGridNo == NOWHERE) {
              gMercProfiles[GetSolProfile(pSoldier)].sPreCombatGridNo = pSoldier->sGridNo;
            }
          } else {
            gMercProfiles[GetSolProfile(pSoldier)].sPreCombatGridNo = NOWHERE;
          }
        }

        if (!gTacticalStatus.fHasEnteredCombatModeSinceEntering) {
          // ATE: reset player's movement mode at the very start of
          // combat
          // if ( pSoldier->bTeam == gbPlayerNum )
          //{
          // pSoldier->usUIMovementMode = RUNNING;
          //}
        }
      }
    }
  }

  gTacticalStatus.fHasEnteredCombatModeSinceEntering = TRUE;

  SyncStrategicTurnTimes();

  // Play tune..
  PlayJA2Sample(ENDTURN_1, RATE_11025, MIDVOLUME, 1, MIDDLEPAN);

  // Say quote.....

  // Change music modes
  SetMusicMode(MUSIC_TACTICAL_BATTLE);
}

void EnterCombatMode(uint8_t ubStartingTeam) {
  uint32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier;

  if (gTacticalStatus.uiFlags & INCOMBAT) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Can't enter combat when already in combat");
    // we're already in combat!
    return;
  }

  // Alrighty, don't do this if no enemies in sector
  if (NumCapableEnemyInSector() == 0) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Can't enter combat when no capable enemies");
    // ScreenMsg( MSG_FONT_RED, MSG_DEBUG, L"Trying to init combat when no enemies around!." );
    return;
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Entering combat mode");

  // ATE: Added here to guarentee we have fEnemyInSector
  // Mostly this was not getting set if:
  // 1 ) we see bloodcats ( which makes them hostile )
  // 2 ) we make civs hostile
  // only do this once they are seen.....
  if (!gTacticalStatus.fEnemyInSector) {
    SetEnemyPresence();
  }

  CommonEnterCombatModeCode();

  if (ubStartingTeam == gbPlayerNum) {
    // OK, make sure we have a selected guy
    if (MercPtrs[gusSelectedSoldier]->bOppCnt == 0) {
      // OK, look through and find one....
      for (cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID, pTeamSoldier = MercPtrs[cnt];
           cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID; cnt++, pTeamSoldier++) {
        if (OK_CONTROLLABLE_MERC(pTeamSoldier) && pTeamSoldier->bOppCnt > 0) {
          SelectSoldier(pTeamSoldier->ubID, FALSE, TRUE);
        }
      }
    }

    StartPlayerTeamTurn(FALSE, TRUE);
  } else {
    // have to call EndTurn so that we freeze the interface etc
    EndTurn(ubStartingTeam);
  }
}

void ExitCombatMode() {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Exiting combat mode");

  // Leave combat mode
  gTacticalStatus.uiFlags &= (~INCOMBAT);

  EndTopMessage();

  // OK, we have exited combat mode.....
  // Reset some flags for no aps to move, etc

  // Set virgin sector to true....
  gTacticalStatus.fVirginSector = TRUE;

  // Loop through all mercs and make go
  for (pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++) {
    if (IsSolActive(pSoldier)) {
      if (IsSolInSector(pSoldier)) {
        // Reset some flags
        if (pSoldier->fNoAPToFinishMove && pSoldier->bLife >= OKLIFE) {
          AdjustNoAPToFinishMove(pSoldier, FALSE);
          SoldierGotoStationaryStance(pSoldier);
        }

        // Cancel pending events
        pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
        pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
        pSoldier->ubPendingAction = NO_PENDING_ACTION;

        // Reset moved flag
        pSoldier->bMoved = FALSE;

        // Set final destination
        pSoldier->sFinalDestination = pSoldier->sGridNo;

        // remove AI controlled flag
        pSoldier->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
      }
    }
  }

  // Change music modes
  gfForceMusicToTense = TRUE;

  SetMusicMode(MUSIC_TACTICAL_ENEMYPRESENT);

  BetweenTurnsVisibilityAdjustments();

  // pause the AI for a bit
  PauseAITemporarily();

  // reset muzzle flashes
  TurnOffEveryonesMuzzleFlashes();

  // zap interrupt list
  ClearIntList();

  // dirty interface
  DirtyMercPanelInterface(MercPtrs[0], DIRTYLEVEL2);

  // ATE: If we are IN_CONV - DONT'T DO THIS!
  if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
    HandleStrategicTurnImplicationsOfExitingCombatMode();
  }

  // Make sure next opplist decay DOES happen right after we go to RT
  // since this would be the same as what would happen at the end of the turn
  gTacticalStatus.uiTimeSinceLastOpplistDecay =
      max(0, GetWorldTotalSeconds() - TIME_BETWEEN_RT_OPPLIST_DECAYS);
  NonCombatDecayPublicOpplist(GetWorldTotalSeconds());
}

void SetEnemyPresence() {
  // We have an ememy present....

  // Check if we previously had no enemys present and we are in a virgin secotr ( no enemys spotted
  // yet )
  if (!gTacticalStatus.fEnemyInSector && gTacticalStatus.fVirginSector) {
    // If we have a guy selected, say quote!
    // For now, display ono status message
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ENEMY_IN_SECTOR_STR]);

    // Change music modes..

    // If we are just starting game, don't do this!
    if (!DidGameJustStart() && !AreInMeanwhile()) {
      SetMusicMode(MUSIC_TACTICAL_ENEMYPRESENT);
    }

    // Say quote...
    // SayQuoteFromAnyBodyInSector( QUOTE_ENEMY_PRESENCE );

    gTacticalStatus.fEnemyInSector = TRUE;
  }
}

extern BOOLEAN gfLastMercTalkedAboutKillingID;

BOOLEAN SoldierHasSeenEnemiesLastFewTurns(struct SOLDIERTYPE *pTeamSoldier) {
  int32_t cnt2;
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  for (cnt = 0; cnt < MAXTEAMS; cnt++) {
    if (GetTeamSide(cnt) != pTeamSoldier->bSide) {
      // check this team for possible enemies
      cnt2 = gTacticalStatus.Team[cnt].bFirstID;
      for (pSoldier = MercPtrs[cnt2]; cnt2 <= gTacticalStatus.Team[cnt].bLastID;
           cnt2++, pSoldier++) {
        if (IsSolActive(pSoldier) && pSoldier->bInSector &&
            (pSoldier->bTeam == gbPlayerNum || pSoldier->bLife >= OKLIFE)) {
          if (!CONSIDERED_NEUTRAL(pTeamSoldier, pSoldier) &&
              (pTeamSoldier->bSide != pSoldier->bSide)) {
            // Have we not seen this guy.....
            if ((pTeamSoldier->bOppList[cnt2] >= SEEN_CURRENTLY) &&
                (pTeamSoldier->bOppList[cnt2] <= SEEN_THIS_TURN)) {
              gTacticalStatus.bConsNumTurnsNotSeen = 0;
              return (TRUE);
            }
          }
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN WeSeeNoOne(void) {
  uint32_t uiLoop;
  struct SOLDIERTYPE *pSoldier;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];
    if (pSoldier != NULL) {
      if (pSoldier->bTeam == gbPlayerNum) {
        if (pSoldier->bOppCnt > 0) {
          return (FALSE);
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN NobodyAlerted(void) {
  uint32_t uiLoop;
  struct SOLDIERTYPE *pSoldier;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];
    if (pSoldier != NULL) {
      if ((pSoldier->bTeam != gbPlayerNum) && (!pSoldier->bNeutral) &&
          (pSoldier->bLife >= OKLIFE) && (pSoldier->bAlertStatus >= STATUS_RED)) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN WeSawSomeoneThisTurn(void) {
  uint32_t uiLoop, uiLoop2;
  struct SOLDIERTYPE *pSoldier;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];
    if (pSoldier != NULL) {
      if (pSoldier->bTeam == gbPlayerNum) {
        for (uiLoop2 = gTacticalStatus.Team[ENEMY_TEAM].bFirstID; uiLoop2 < TOTAL_SOLDIERS;
             uiLoop2++) {
          if (pSoldier->bOppList[uiLoop2] == SEEN_THIS_TURN) {
            return (TRUE);
          }
        }
      }
    }
  }
  return (FALSE);
}

void SayBattleSoundFromAnyBodyInSector(int32_t iBattleSnd) {
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt;

  // Loop through all our guys and randomly say one from someone in our sector

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    // Add guy if he's a candidate...
    if (OK_INSECTOR_MERC(pTeamSoldier) && !AM_AN_EPC(pTeamSoldier) &&
        !(pTeamSoldier->uiStatusFlags & SOLDIER_GASSED) && !(AM_A_ROBOT(pTeamSoldier)) &&
        !pTeamSoldier->fMercAsleep) {
      ubNumMercs++;
    }
  }

  // If we are > 0
  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    DoMercBattleSound(MercPtrs[ubChosenMerc], (uint8_t)iBattleSnd);
  }
}

BOOLEAN CheckForEndOfCombatMode(BOOLEAN fIncrementTurnsNotSeen) {
  struct SOLDIERTYPE *pTeamSoldier;
  uint32_t cnt = 0;
  BOOLEAN fWeSeeNoOne, fNobodyAlerted;
  BOOLEAN fSayQuote = FALSE;
  BOOLEAN fWeSawSomeoneRecently = FALSE, fSomeoneSawSomeoneRecently = FALSE;

  // We can only check for end of combat if in combat mode
  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    return (FALSE);
  }

  // if we're boxing NEVER end combat mode
  if (gTacticalStatus.bBoxingState == BOXING) {
    return (FALSE);
  }

  // First check for end of battle....
  // If there are no enemies at all in the sector
  // Battle end should take presedence!
  if (CheckForEndOfBattle(FALSE)) {
    return (TRUE);
  }

  fWeSeeNoOne = WeSeeNoOne();
  fNobodyAlerted = NobodyAlerted();
  if (fWeSeeNoOne && fNobodyAlerted) {
    // hack!!!
    gTacticalStatus.bConsNumTurnsNotSeen = 5;
  } else {
    // we have to loop through EVERYONE to see if anyone sees a hostile... if so, stay in
    // turnbased...

    for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
      pTeamSoldier = MercSlots[cnt];
      if (pTeamSoldier && pTeamSoldier->bLife >= OKLIFE && !pTeamSoldier->bNeutral) {
        if (SoldierHasSeenEnemiesLastFewTurns(pTeamSoldier)) {
          gTacticalStatus.bConsNumTurnsNotSeen = 0;
          fSomeoneSawSomeoneRecently = TRUE;
          if (pTeamSoldier->bTeam == gbPlayerNum ||
              (pTeamSoldier->bTeam == MILITIA_TEAM &&
               pTeamSoldier->bSide == 0))  // or friendly militia
          {
            fWeSawSomeoneRecently = TRUE;
            break;
          }
        }
      }
    }

    if (fSomeoneSawSomeoneRecently) {
      if (fWeSawSomeoneRecently) {
        gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 0;
      } else {
        // start tracking this
        gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes++;
      }
      return (FALSE);
    }

    // IF here, we don;t see anybody.... increment count for end check
    if (fIncrementTurnsNotSeen) {
      gTacticalStatus.bConsNumTurnsNotSeen++;
    }
  }

  gTacticalStatus.bConsNumTurnsWeHaventSeenButEnemyDoes = 0;

  // If we have reach a point where a cons. number of turns gone by....
  if (gTacticalStatus.bConsNumTurnsNotSeen > 1) {
    gTacticalStatus.bConsNumTurnsNotSeen = 0;

    // Exit mode!
    ExitCombatMode();

    if (fNobodyAlerted) {
      // if we don't see anyone currently BUT we did see someone this turn, THEN don't
      // say quote
      if (fWeSeeNoOne && WeSawSomeoneThisTurn()) {
        // don't say quote
      } else {
        fSayQuote = TRUE;
      }
    } else {
      fSayQuote = TRUE;
    }

    // ATE: Are there creatures here? If so, say another quote...
    if (fSayQuote && (gTacticalStatus.uiFlags & IN_CREATURE_LAIR)) {
      SayQuoteFromAnyBodyInSector(QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE);
    }
    // Are we fighting bloodcats?
    else if (NumBloodcatsInSectorNotDeadOrDying() > 0) {
    } else {
      if (fSayQuote) {
        // Double check by seeing if we have any active enemies in sector!
        if (NumEnemyInSectorNotDeadOrDying() > 0) {
          SayQuoteFromAnyBodyInSector(QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT);
        }
      }
    }
    /*
                    if ( (!fWeSeeNoOne || !fNobodyAlerted) && WeSawSomeoneThisTurn() )
                    {
                            // Say quote to the effect that the battle has lulled
                            SayQuoteFromAnyBodyInSector( QUOTE_WARNING_OUTSTANDING_ENEMY_AFTER_RT );
                    }
    */

    // Begin tense music....
    gfForceMusicToTense = TRUE;
    SetMusicMode(MUSIC_TACTICAL_ENEMYPRESENT);

    return (TRUE);
  }

  return (FALSE);
}

void DeathNoMessageTimerCallback(void) { CheckAndHandleUnloadingOfCurrentWorld(); }

//!!!!
// IMPORTANT NEW NOTE:
// Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
BOOLEAN CheckForEndOfBattle(BOOLEAN fAnEnemyRetreated) {
  struct SOLDIERTYPE *pTeamSoldier;
  BOOLEAN fBattleWon = TRUE;
  BOOLEAN fBattleLost = FALSE;
  int32_t cnt = 0;
  uint16_t usAnimState;

  if (gTacticalStatus.bBoxingState == BOXING) {
    // no way are we going to exit boxing prematurely, thank you! :-)
    return (FALSE);
  }

  // We can only check for end of battle if in combat mode or there are enemies
  // present (they might bleed to death or run off the map!)
  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    if (!(gTacticalStatus.fEnemyInSector)) {
      return (FALSE);
    }
  }

  // ATE: If attack busy count.. get out...
  if ((gTacticalStatus.ubAttackBusyCount > 0)) {
    return (FALSE);
  }

  // OK, this is to releave infinate looping...becasue we can kill guys in this function
  if (gfKillingGuysForLosingBattle) {
    return (FALSE);
  }

  // Check if the battle is won!
  if (NumCapableEnemyInSector() > 0) {
    fBattleWon = FALSE;
  }

  if (CheckForLosingEndOfBattle() == TRUE) {
    fBattleLost = TRUE;
  }

  // NEW (Nov 24, 98)  by Kris
  if (!gbWorldSectorZ &&
      fBattleWon) {  // Check to see if more enemy soldiers exist in the strategic layer
    // It is possible to have more than 20 enemies in a sector.  By failing here,
    // it gives the engine a chance to add these soldiers as reinforcements.  This
    // is naturally handled.
    if (gfPendingEnemies) {
      fBattleWon = FALSE;
    }
  }

  if ((fBattleLost) || (fBattleWon)) {
    if (!gbWorldSectorZ) {
      SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)].bLastKnownEnemies =
          NumEnemiesInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
    }
  }

  // We should NEVER have a battle lost and won at the same time...

  if (fBattleLost) {
    // CJC: End AI's turn here.... first... so that UnSetUIBusy will succeed if militia win
    // battle for us
    EndAllAITurns();

    // Set enemy presence to false
    // This is safe 'cause we're about to unload the friggen sector anyway....
    gTacticalStatus.fEnemyInSector = FALSE;

    // If here, the battle has been lost!
    UnSetUIBusy((uint8_t)gusSelectedSoldier);

    if (gTacticalStatus.uiFlags & INCOMBAT) {
      // Exit mode!
      ExitCombatMode();
    }

    HandleMoraleEvent(NULL, MORALE_HEARD_BATTLE_LOST, (uint8_t)gWorldSectorX,
                      (uint8_t)gWorldSectorY, gbWorldSectorZ);
    HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_BATTLE_LOST, (uint8_t)gWorldSectorX,
                             (uint8_t)gWorldSectorY, gbWorldSectorZ);

    // Play death music
    SetMusicMode(MUSIC_TACTICAL_DEATH);
    SetCustomizableTimerCallbackAndDelay(10000, DeathNoMessageTimerCallback, FALSE);

    if (CheckFact(FACT_FIRST_BATTLE_BEING_FOUGHT, 0)) {
      // this is our first battle... and we lost it!
      SetFactTrue(FACT_FIRST_BATTLE_FOUGHT);
      SetFactFalse(FACT_FIRST_BATTLE_BEING_FOUGHT);
      SetTheFirstBattleSector(
          (int16_t)(GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)));
      HandleFirstBattleEndingWhileInTown((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                         gbWorldSectorZ, FALSE);
    }

    if (NumEnemyInSectorExceptCreatures()) {
      SetThisSectorAsEnemyControlled((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ,
                                     TRUE);
    }

    // ATE: Important! THis is delayed until music ends so we can have proper effect!
    // CheckAndHandleUnloadingOfCurrentWorld();

    // Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
    LogBattleResults(LOG_DEFEAT);
    gfBlitBattleSectorLocator = FALSE;
    return (TRUE);
  }

  // If battle won, do stuff right away!
  if (fBattleWon) {
    if (gTacticalStatus.bBoxingState == NOT_BOXING)  // if boxing don't do any of this stuff
    {
      gTacticalStatus.fLastBattleWon = TRUE;

      // OK, KILL any enemies that are incompacitated
      if (KillIncompacitatedEnemyInSector()) {
        return (FALSE);
      }
    }

    // If here, the battle has been won!
    // hurray! a glorious victory!

    // Set enemy presence to false
    gTacticalStatus.fEnemyInSector = FALSE;

    // CJC: End AI's turn here.... first... so that UnSetUIBusy will succeed if militia win
    // battle for us
    EndAllAITurns();

    UnSetUIBusy((uint8_t)gusSelectedSoldier);

    // ATE:
    // If we ended battle in any team other than the player's
    // we need to end the UI lock using this method....
    guiPendingOverrideEvent = LU_ENDUILOCK;
    HandleTacticalUI();

    if (gTacticalStatus.uiFlags & INCOMBAT) {
      // Exit mode!
      ExitCombatMode();
    }

    if (gTacticalStatus.bBoxingState == NOT_BOXING)  // if boxing don't do any of this stuff
    {
      // Only do some stuff if we actually faught a battle
      if (gTacticalStatus.bNumFoughtInBattle[ENEMY_TEAM] +
              gTacticalStatus.bNumFoughtInBattle[CREATURE_TEAM] +
              gTacticalStatus.bNumFoughtInBattle[CIV_TEAM] >
          0)
      // if ( gTacticalStatus.bNumEnemiesFoughtInBattle > 0 )
      {
        // Loop through all mercs and make go
        for (pTeamSoldier = MercPtrs[gTacticalStatus.Team[gbPlayerNum].bFirstID];
             cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID; cnt++, pTeamSoldier++) {
          if (pTeamSoldier->bActive) {
            if (pTeamSoldier->bInSector) {
              if (pTeamSoldier->bTeam == gbPlayerNum) {
                gMercProfiles[pTeamSoldier->ubProfile].usBattlesFought++;

                // If this guy is OKLIFE & not standing, make stand....
                if (pTeamSoldier->bLife >= OKLIFE && !pTeamSoldier->bCollapsed) {
                  if (pTeamSoldier->bAssignment < ON_DUTY) {
                    // Reset some quote flags....
                    pTeamSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_BUDDY_1_WITNESSED);
                    pTeamSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_BUDDY_2_WITNESSED);
                    pTeamSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_BUDDY_3_WITNESSED);

                    // toggle stealth mode....
                    gfUIStanceDifferent = TRUE;
                    pTeamSoldier->bStealthMode = FALSE;
                    fInterfacePanelDirty = DIRTYLEVEL2;

                    if (gAnimControl[pTeamSoldier->usAnimState].ubHeight != ANIM_STAND) {
                      ChangeSoldierStance(pTeamSoldier, ANIM_STAND);
                    } else {
                      // If they are aiming, end aim!
                      usAnimState = PickSoldierReadyAnimation(pTeamSoldier, TRUE);

                      if (usAnimState != INVALID_ANIMATION) {
                        EVENT_InitNewSoldierAnim(pTeamSoldier, usAnimState, 0, FALSE);
                      }
                    }
                  }
                }
              }
            }
          }
        }

        HandleMoraleEvent(NULL, MORALE_BATTLE_WON, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                          gbWorldSectorZ);
        HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_BATTLE_WON, (uint8_t)gWorldSectorX,
                                 (uint8_t)gWorldSectorY, gbWorldSectorZ);

        // Change music modes
        if (gfLastMercTalkedAboutKillingID == NOBODY ||
            (gfLastMercTalkedAboutKillingID != NOBODY &&
             !(MercPtrs[gfLastMercTalkedAboutKillingID]->uiStatusFlags & SOLDIER_MONSTER))) {
          SetMusicMode(MUSIC_TACTICAL_VICTORY);

          ShouldBeginAutoBandage();
        } else if (gfLastMercTalkedAboutKillingID != NOBODY &&
                   (MercPtrs[gfLastMercTalkedAboutKillingID]->uiStatusFlags & SOLDIER_MONSTER)) {
          ShouldBeginAutoBandage();
        }

        // Say battle end quote....

        if (fAnEnemyRetreated) {
          SayQuoteFromAnyBodyInSector(QUOTE_ENEMY_RETREATED);
        } else {
          // OK, If we have just finished a battle with creatures........ play killed creature
          // quote...
          //
          if (gfLastMercTalkedAboutKillingID != NOBODY &&
              (MercPtrs[gfLastMercTalkedAboutKillingID]->uiStatusFlags & SOLDIER_MONSTER)) {
          } else if (gfLastMercTalkedAboutKillingID != NOBODY &&
                     (MercPtrs[gfLastMercTalkedAboutKillingID]->ubBodyType == BLOODCAT)) {
            SayBattleSoundFromAnyBodyInSector(BATTLE_SOUND_COOL1);
          } else {
            SayQuoteFromAnyBodyInSector(QUOTE_SECTOR_SAFE);
          }
        }

      } else {
        // Change to nothing music...
        SetMusicMode(MUSIC_TACTICAL_NOTHING);
        ShouldBeginAutoBandage();
      }

      // Loop through all militia and restore them to peaceful status
      cnt = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
      for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[MILITIA_TEAM].bLastID;
           cnt++, pTeamSoldier++) {
        if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
          pTeamSoldier->bAlertStatus = STATUS_GREEN;
          CheckForChangingOrders(pTeamSoldier);
          pTeamSoldier->sNoiseGridno = NOWHERE;
          pTeamSoldier->ubNoiseVolume = 0;
          pTeamSoldier->bNewSituation = FALSE;
          pTeamSoldier->bOrders = STATIONARY;
          if (pTeamSoldier->bLife >= OKLIFE) {
            pTeamSoldier->bBleeding = 0;
          }
        }
      }
      gTacticalStatus.Team[MILITIA_TEAM].bAwareOfOpposition = FALSE;

      // Loop through all civs and restore them to peaceful status
      cnt = gTacticalStatus.Team[CIV_TEAM].bFirstID;
      for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[CIV_TEAM].bLastID;
           cnt++, pTeamSoldier++) {
        if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
          pTeamSoldier->bAlertStatus = STATUS_GREEN;
          pTeamSoldier->sNoiseGridno = NOWHERE;
          pTeamSoldier->ubNoiseVolume = 0;
          pTeamSoldier->bNewSituation = FALSE;
          CheckForChangingOrders(pTeamSoldier);
        }
      }
      gTacticalStatus.Team[CIV_TEAM].bAwareOfOpposition = FALSE;
    }

    fInterfacePanelDirty = DIRTYLEVEL2;

    if (gTacticalStatus.bBoxingState == NOT_BOXING)  // if boxing don't do any of this stuff
    {
      LogBattleResults(LOG_VICTORY);

      SetThisSectorAsPlayerControlled((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                      gbWorldSectorZ, TRUE);
      HandleVictoryInNPCSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                               (int16_t)gbWorldSectorZ);
      if (CheckFact(FACT_FIRST_BATTLE_BEING_FOUGHT, 0)) {
        // ATE: Need to trigger record for this event .... for NPC scripting
        TriggerNPCRecord(PACOS, 18);

        // this is our first battle... and we won!
        SetFactTrue(FACT_FIRST_BATTLE_FOUGHT);
        SetFactTrue(FACT_FIRST_BATTLE_WON);
        SetFactFalse(FACT_FIRST_BATTLE_BEING_FOUGHT);
        SetTheFirstBattleSector(
            (int16_t)(GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)));
        HandleFirstBattleEndingWhileInTown((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                           gbWorldSectorZ, FALSE);
      }
    }

    // Whenever returning TRUE, make sure you clear gfBlitBattleSectorLocator;
    gfBlitBattleSectorLocator = FALSE;
    return (TRUE);
  }

  return (FALSE);
}

void CycleThroughKnownEnemies() {
  // static to indicate last position we were at:
  struct SOLDIERTYPE *pSoldier;
  static BOOLEAN fFirstTime = TRUE;
  static uint16_t usStartToLook;
  uint32_t cnt;
  BOOLEAN fEnemyBehindStartLook = FALSE;
  BOOLEAN fEnemiesFound = FALSE;

  if (fFirstTime) {
    fFirstTime = FALSE;

    usStartToLook = gTacticalStatus.Team[gbPlayerNum].bLastID;
  }

  for (cnt = gTacticalStatus.Team[gbPlayerNum].bLastID, pSoldier = MercPtrs[cnt];
       cnt < TOTAL_SOLDIERS; cnt++, pSoldier++) {
    // try to find first active, OK enemy
    if (IsSolActive(pSoldier) && pSoldier->bInSector && !pSoldier->bNeutral &&
        (pSoldier->bSide != gbPlayerNum) && IsSolAlive(pSoldier)) {
      if (pSoldier->bVisible != -1) {
        fEnemiesFound = TRUE;

        // If we are > ok start, this is the one!
        if (cnt > usStartToLook) {
          usStartToLook = (uint16_t)cnt;

          // Locate to!
          // LocateSoldier( GetSolID(pSoldier), 1 );

          // ATE: Change to Slide To...
          SlideTo(0, GetSolID(pSoldier), 0, SETANDREMOVEPREVIOUSLOCATOR);
          return;
        } else {
          fEnemyBehindStartLook = TRUE;
        }
      }
    }
  }

  if (!fEnemiesFound) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_ENEMIES_IN_SIGHT_STR]);
  }

  // If here, we found nobody, but there may be someone behind
  // If to, recurse!
  if (fEnemyBehindStartLook) {
    usStartToLook = gTacticalStatus.Team[gbPlayerNum].bLastID;

    CycleThroughKnownEnemies();
  }
}

void CycleVisibleEnemies(struct SOLDIERTYPE *pSrcSoldier) {
  // static to indicate last position we were at:
  struct SOLDIERTYPE *pSoldier;
  uint32_t cnt;

  for (cnt = gTacticalStatus.Team[gbPlayerNum].bLastID, pSoldier = MercPtrs[cnt];
       cnt < TOTAL_SOLDIERS; cnt++, pSoldier++) {
    // try to find first active, OK enemy
    if (IsSolActive(pSoldier) && pSoldier->bInSector && !pSoldier->bNeutral &&
        (pSoldier->bSide != gbPlayerNum) && IsSolAlive(pSoldier)) {
      if (pSrcSoldier->bOppList[pSoldier->ubID] == SEEN_CURRENTLY) {
        // If we are > ok start, this is the one!
        if (cnt > pSrcSoldier->ubLastEnemyCycledID) {
          pSrcSoldier->ubLastEnemyCycledID = (uint8_t)cnt;

          // ATE: Change to Slide To...
          SlideTo(0, GetSolID(pSoldier), 0, SETANDREMOVEPREVIOUSLOCATOR);

          ChangeInterfaceLevel(pSoldier->bLevel);
          return;
        }
      }
    }
  }

  // If here.. reset to zero...
  pSrcSoldier->ubLastEnemyCycledID = 0;

  for (cnt = gTacticalStatus.Team[gbPlayerNum].bLastID, pSoldier = MercPtrs[cnt];
       cnt < TOTAL_SOLDIERS; cnt++, pSoldier++) {
    // try to find first active, OK enemy
    if (IsSolActive(pSoldier) && pSoldier->bInSector && !pSoldier->bNeutral &&
        (pSoldier->bSide != gbPlayerNum) && IsSolAlive(pSoldier)) {
      if (pSrcSoldier->bOppList[pSoldier->ubID] == SEEN_CURRENTLY) {
        // If we are > ok start, this is the one!
        if (cnt > pSrcSoldier->ubLastEnemyCycledID) {
          pSrcSoldier->ubLastEnemyCycledID = (uint8_t)cnt;

          // ATE: Change to Slide To...
          SlideTo(0, GetSolID(pSoldier), 0, SETANDREMOVEPREVIOUSLOCATOR);

          ChangeInterfaceLevel(pSoldier->bLevel);
          return;
        }
      }
    }
  }
}

int8_t CountNonVehiclesOnPlayerTeam(void) {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  int8_t bNumber = 0;

  for (cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID, pSoldier = MercPtrs[cnt];
       cnt <= (uint32_t)(gTacticalStatus.Team[gbPlayerNum].bLastID); cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      bNumber++;
    }
  }

  return (bNumber);
}

BOOLEAN PlayerTeamFull() {
  // last ID for the player team is 19, so long as we have at most 17 non-vehicles...
  if (CountNonVehiclesOnPlayerTeam() <= gTacticalStatus.Team[gbPlayerNum].bLastID - 2) {
    return (FALSE);
  }

  return (TRUE);
}

uint8_t NumPCsInSector(void) {
  struct SOLDIERTYPE *pTeamSoldier;
  uint32_t cnt = 0;
  uint8_t ubNumPlayers = 0;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
    if (MercSlots[cnt]) {
      pTeamSoldier = MercSlots[cnt];
      if (pTeamSoldier->bTeam == gbPlayerNum && pTeamSoldier->bLife > 0) {
        ubNumPlayers++;
      }
    }
  }

  return (ubNumPlayers);
}

uint8_t NumEnemyInSector() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  uint8_t ubNumEnemies = 0;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++) {
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bLife > 0) {
      // Checkf for any more bacguys
      if (!pTeamSoldier->bNeutral && (pTeamSoldier->bSide != 0)) {
        ubNumEnemies++;
      }
    }
  }

  return (ubNumEnemies);
}

uint8_t NumEnemyInSectorExceptCreatures() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  uint8_t ubNumEnemies = 0;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++) {
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bLife > 0 &&
        pTeamSoldier->bTeam != CREATURE_TEAM) {
      // Checkf for any more bacguys
      if (!pTeamSoldier->bNeutral && (pTeamSoldier->bSide != 0)) {
        ubNumEnemies++;
      }
    }
  }

  return (ubNumEnemies);
}

uint8_t NumEnemyInSectorNotDeadOrDying() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  uint8_t ubNumEnemies = 0;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++) {
    // Kill those not already dead.,...
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
      // For sure for flag thet they are dead is not set
      if (!(pTeamSoldier->uiStatusFlags & SOLDIER_DEAD)) {
        // Also, we want to pick up unconcious guys as NOT being capable,
        // but we want to make sure we don't get those ones that are in the
        // process of dying
        if (pTeamSoldier->bLife >= OKLIFE) {
          // Check for any more badguys
          if (!pTeamSoldier->bNeutral && (pTeamSoldier->bSide != 0)) {
            ubNumEnemies++;
          }
        }
      }
    }
  }

  return (ubNumEnemies);
}

uint8_t NumBloodcatsInSectorNotDeadOrDying() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  uint8_t ubNumEnemies = 0;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++) {
    // Kill those not already dead.,...
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
      if (pTeamSoldier->ubBodyType == BLOODCAT) {
        // For sure for flag thet they are dead is not set
        if (!(pTeamSoldier->uiStatusFlags & SOLDIER_DEAD)) {
          // Also, we want to pick up unconcious guys as NOT being capable,
          // but we want to make sure we don't get those ones that are in the
          // process of dying
          if (pTeamSoldier->bLife >= OKLIFE) {
            // Check for any more badguys
            if (!pTeamSoldier->bNeutral && (pTeamSoldier->bSide != 0)) {
              ubNumEnemies++;
            }
          }
        }
      }
    }
  }

  return (ubNumEnemies);
}

uint8_t NumCapableEnemyInSector() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  uint8_t ubNumEnemies = 0;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++) {
    // Kill those not already dead.,...
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
      // For sure for flag thet they are dead is not set
      if (!(pTeamSoldier->uiStatusFlags & SOLDIER_DEAD)) {
        // Also, we want to pick up unconcious guys as NOT being capable,
        // but we want to make sure we don't get those ones that are in the
        // process of dying
        if (pTeamSoldier->bLife < OKLIFE && pTeamSoldier->bLife != 0) {
        } else {
          // Check for any more badguys
          if (!pTeamSoldier->bNeutral && (pTeamSoldier->bSide != 0)) {
            ubNumEnemies++;
          }
        }
      }
    }
  }

  return (ubNumEnemies);
}

BOOLEAN CheckForLosingEndOfBattle() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  int8_t bNumDead = 0, bNumNotOK = 0, bNumInBattle = 0, bNumNotOKRealMercs = 0;
  BOOLEAN fMadeCorpse;
  BOOLEAN fDoCapture = FALSE;
  BOOLEAN fOnlyEPCsLeft = TRUE;
  BOOLEAN fMilitiaInSector = FALSE;

  // ATE: Check for MILITIA - we won't lose if we have some.....
  cnt = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[MILITIA_TEAM].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bSide == gbPlayerNum) {
      if (pTeamSoldier->bLife >= OKLIFE) {
        // We have at least one poor guy who will still fight....
        // we have not lost ( yet )!
        fMilitiaInSector = TRUE;
      }
    }
  }

  // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    // Are we active and in sector.....
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector &&
        !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      bNumInBattle++;

      if (pTeamSoldier->bLife == 0) {
        bNumDead++;
      } else if (pTeamSoldier->bLife < OKLIFE) {
        bNumNotOK++;

        if (!AM_AN_EPC(pTeamSoldier) && !AM_A_ROBOT(pTeamSoldier)) {
          bNumNotOKRealMercs++;
        }
      } else {
        if (!AM_A_ROBOT(pTeamSoldier) || !AM_AN_EPC(pTeamSoldier)) {
          fOnlyEPCsLeft = FALSE;
        }
      }
    }
  }

  // OK< check ALL in battle, if that matches SUM of dead, incompacitated, we're done!
  if ((bNumDead + bNumNotOK) == bNumInBattle || fOnlyEPCsLeft) {
    // Are there militia in sector?
    if (fMilitiaInSector) {
      if (guiCurrentScreen != AUTORESOLVE_SCREEN) {
        // if here, check if we should autoresolve.
        // if we have at least one guy unconscious, call below function...
        if (HandlePotentialBringUpAutoresolveToFinishBattle()) {
          // return false here as we are autoresolving...
          return (FALSE);
        }
      } else {
        return (FALSE);
      }
    }

    // Bring up box if we have ANY guy incompaciteded.....
    if (bNumDead != bNumInBattle) {
      // If we get captured...
      // Your unconscious mercs are captured!

      // Check if we should get captured....
      if (bNumNotOKRealMercs < 4 && bNumNotOKRealMercs > 1) {
        // Check if any enemies exist....
        if (gTacticalStatus.Team[ENEMY_TEAM].bMenInSector > 0) {
          // if( GetWorldDay() > STARTDAY_ALLOW_PLAYER_CAPTURE_FOR_RESCUE && !(
          // gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE ))
          {
            if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTNOTSTARTED ||
                (gubQuest[QUEST_HELD_IN_ALMA] == QUESTDONE &&
                 gubQuest[QUEST_INTERROGATION] == QUESTNOTSTARTED)) {
              fDoCapture = TRUE;
              // CJC Dec 1 2002: fix capture sequences
              BeginCaptureSquence();
            }
          }
        }
      }

      gfKillingGuysForLosingBattle = TRUE;

      // KIll them now...
      // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
      cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

      for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
           cnt++, pTeamSoldier++) {
        // Are we active and in sector.....
        if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
          if ((pTeamSoldier->bLife != 0 && pTeamSoldier->bLife < OKLIFE) ||
              AM_AN_EPC(pTeamSoldier) || AM_A_ROBOT(pTeamSoldier)) {
            // Captured EPCs or ROBOTS will be kiiled in capture routine....
            if (!fDoCapture) {
              // Kill!
              pTeamSoldier->bLife = 0;

              HandleSoldierDeath(pTeamSoldier, &fMadeCorpse);

              // HandlePlayerTeamMemberDeath( pTeamSoldier );
              // Make corpse..
              // TurnSoldierIntoCorpse( pTeamSoldier, TRUE, TRUE );
            }
          }

          // ATE: if we are told to do capture....
          if (pTeamSoldier->bLife != 0 && fDoCapture) {
            EnemyCapturesPlayerSoldier(pTeamSoldier);

            RemoveSoldierFromTacticalSector(pTeamSoldier, TRUE);
          }
        }
      }

      gfKillingGuysForLosingBattle = FALSE;

      if (fDoCapture) {
        EndCaptureSequence();
        SetCustomizableTimerCallbackAndDelay(3000, CaptureTimerCallback, FALSE);
      } else {
        SetCustomizableTimerCallbackAndDelay(10000, DeathTimerCallback, FALSE);
      }
    }
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN KillIncompacitatedEnemyInSector() {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  BOOLEAN fReturnVal = FALSE;

  // Check if the battle is won!
  // Loop through all mercs and make go
  for (pTeamSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pTeamSoldier++, cnt++) {
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bLife < OKLIFE &&
        !(pTeamSoldier->uiStatusFlags & SOLDIER_DEAD)) {
      // Checkf for any more bacguys
      if (!pTeamSoldier->bNeutral && (pTeamSoldier->bSide != gbPlayerNum)) {
        // KIll......
        SoldierTakeDamage(pTeamSoldier, ANIM_CROUCH, pTeamSoldier->bLife, 100,
                          TAKE_DAMAGE_BLOODLOSS, NOBODY, NOWHERE, 0, TRUE);
        fReturnVal = TRUE;
      }
    }
  }
  return (fReturnVal);
}

BOOLEAN AttackOnGroupWitnessed(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTarget) {
  uint32_t uiSlot;
  struct SOLDIERTYPE *pGroupMember;

  // look for all group members... rebels could be on the civ team or ours!
  for (uiSlot = 0; uiSlot < guiNumMercSlots; uiSlot++) {
    pGroupMember = MercSlots[uiSlot];
    if (pGroupMember && (pGroupMember->ubCivilianGroup == pTarget->ubCivilianGroup) &&
        pGroupMember != pTarget) {
      if (pGroupMember->bOppList[pSoldier->ubID] == SEEN_CURRENTLY ||
          pGroupMember->bOppList[pTarget->ubID] == SEEN_CURRENTLY) {
        return (TRUE);
      }
      if (SpacesAway(pGroupMember->sGridNo, pSoldier->sGridNo) < 12 ||
          SpacesAway(pGroupMember->sGridNo, pTarget->sGridNo) < 12) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

int8_t CalcSuppressionTolerance(struct SOLDIERTYPE *pSoldier) {
  int8_t bTolerance;

  // Calculate basic tolerance value
  bTolerance = pSoldier->bExpLevel * 2;
  if (pSoldier->uiStatusFlags & SOLDIER_PC) {
    // give +1 for every 10% morale from 50, for a maximum bonus/penalty of 5.
    bTolerance += (pSoldier->bMorale - 50) / 10;
  } else {
    // give +2 for every morale category from normal, for a max change of 4
    bTolerance += (pSoldier->bAIMorale - MORALE_NORMAL) * 2;
  }

  if (GetSolProfile(pSoldier) != NO_PROFILE) {
    // change tolerance based on attitude
    switch (gMercProfiles[GetSolProfile(pSoldier)].bAttitude) {
      case ATT_AGGRESSIVE:
        bTolerance += 2;
        break;
      case ATT_COWARD:
        bTolerance += -2;
        break;
      default:
        break;
    }
  } else {
    // generic NPC/civvie; change tolerance based on attitude
    switch (pSoldier->bAttitude) {
      case BRAVESOLO:
      case BRAVEAID:
        bTolerance += 2;
        break;
      case AGGRESSIVE:
        bTolerance += 1;
        break;
      case DEFENSIVE:
        bTolerance += -1;
        break;
      default:
        break;
    }
  }

  if (bTolerance < 0) {
    bTolerance = 0;
  }

  return (bTolerance);
}

#define MAX_APS_SUPPRESSED 8
void HandleSuppressionFire(uint8_t ubTargetedMerc, uint8_t ubCausedAttacker) {
  int8_t bTolerance;
  int16_t sClosestOpponent, sClosestOppLoc;
  uint8_t ubPointsLost, ubTotalPointsLost, ubNewStance;
  uint32_t uiLoop;
  uint8_t ubLoop2;
  struct SOLDIERTYPE *pSoldier;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];
    if (pSoldier && IS_MERC_BODY_TYPE(pSoldier) && pSoldier->bLife >= OKLIFE &&
        pSoldier->ubSuppressionPoints > 0) {
      bTolerance = CalcSuppressionTolerance(pSoldier);

      // multiply by 2, add 1 and divide by 2 to round off to nearest whole number
      ubPointsLost = (((pSoldier->ubSuppressionPoints * 6) / (bTolerance + 6)) * 2 + 1) / 2;

      // reduce loss of APs based on stance
      // ATE: Taken out because we can possibly supress ourselves...
      // switch (gAnimControl[ pSoldier->usAnimState ].ubEndHeight)
      //{
      //	case ANIM_PRONE:
      //		ubPointsLost = ubPointsLost * 2 / 4;
      //		break;
      //	case ANIM_CROUCH:
      //		ubPointsLost = ubPointsLost * 3 / 4;
      //		break;
      //	default:
      //		break;
      //}

      // cap the # of APs we can lose
      if (ubPointsLost > MAX_APS_SUPPRESSED) {
        ubPointsLost = MAX_APS_SUPPRESSED;
      }

      ubTotalPointsLost = ubPointsLost;

      // Subtract off the APs lost before this point to find out how many points are lost now
      if (pSoldier->ubAPsLostToSuppression >= ubPointsLost) {
        continue;
      }

      // morale modifier
      if (ubTotalPointsLost / 2 > pSoldier->ubAPsLostToSuppression / 2) {
        for (ubLoop2 = 0;
             ubLoop2 < (ubTotalPointsLost / 2) - (pSoldier->ubAPsLostToSuppression / 2);
             ubLoop2++) {
          HandleMoraleEvent(pSoldier, MORALE_SUPPRESSED, GetSolSectorX(pSoldier),
                            GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));
        }
      }

      ubPointsLost -= pSoldier->ubAPsLostToSuppression;
      ubNewStance = 0;

      // merc may get to react
      if (pSoldier->ubSuppressionPoints >= (130 / (6 + bTolerance))) {
        // merc gets to use APs to react!
        switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
          case ANIM_PRONE:
            // can't change stance below prone!
            break;
          case ANIM_CROUCH:
            if (ubTotalPointsLost >= AP_PRONE && IsValidStance(pSoldier, ANIM_PRONE)) {
              sClosestOpponent = ClosestKnownOpponent(pSoldier, &sClosestOppLoc, NULL);
              if (sClosestOpponent == NOWHERE ||
                  SpacesAway(pSoldier->sGridNo, sClosestOppLoc) > 8) {
                if (ubPointsLost < AP_PRONE) {
                  // Have to give APs back so that we can change stance without
                  // losing more APs
                  pSoldier->bActionPoints += (AP_PRONE - ubPointsLost);
                  ubPointsLost = 0;
                } else {
                  ubPointsLost -= AP_PRONE;
                }
                ubNewStance = ANIM_PRONE;
              }
            }
            break;
          default:  // standing!
            if (pSoldier->bOverTerrainType == LOW_WATER ||
                pSoldier->bOverTerrainType == DEEP_WATER) {
              // can't change stance here!
              break;
            } else if (ubTotalPointsLost >= (AP_CROUCH + AP_PRONE) &&
                       (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_PRONE) &&
                       IsValidStance(pSoldier, ANIM_PRONE)) {
              sClosestOpponent = ClosestKnownOpponent(pSoldier, &sClosestOppLoc, NULL);
              if (sClosestOpponent == NOWHERE ||
                  SpacesAway(pSoldier->sGridNo, sClosestOppLoc) > 8) {
                if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
                  // can only crouch for now
                  ubNewStance = ANIM_CROUCH;
                } else {
                  // lie prone!
                  ubNewStance = ANIM_PRONE;
                }
              } else if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND &&
                         IsValidStance(pSoldier, ANIM_CROUCH)) {
                // crouch, at least!
                ubNewStance = ANIM_CROUCH;
              }
            } else if (ubTotalPointsLost >= AP_CROUCH &&
                       (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_CROUCH) &&
                       IsValidStance(pSoldier, ANIM_CROUCH)) {
              // crouch!
              ubNewStance = ANIM_CROUCH;
            }
            break;
        }
      }

      // Reduce action points!
      pSoldier->bActionPoints -= ubPointsLost;
      pSoldier->ubAPsLostToSuppression = ubTotalPointsLost;

      if ((pSoldier->uiStatusFlags & SOLDIER_PC) && (pSoldier->ubSuppressionPoints > 8) &&
          (pSoldier->ubID == ubTargetedMerc)) {
        if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_BEING_PUMMELED)) {
          pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_BEING_PUMMELED;
          // say we're under heavy fire!

          // ATE: For some reason, we forgot #53!
          if (GetSolProfile(pSoldier) != 53) {
            TacticalCharacterDialogue(pSoldier, QUOTE_UNDER_HEAVY_FIRE);
          }
        }
      }

      if (ubNewStance != 0) {
        // This person is going to change stance

        // This person will be busy while they crouch or go prone
        if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) {
          DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                   String("!!!!!!! Starting suppression, on %d", GetSolID(pSoldier)));

          gTacticalStatus.ubAttackBusyCount++;

          // make sure supressor ID is the same!
          pSoldier->ubSuppressorID = ubCausedAttacker;
        }
        pSoldier->fChangingStanceDueToSuppression = TRUE;
        pSoldier->fDontChargeAPsForStanceChange = TRUE;

        // AI people will have to have their actions cancelled
        if (!(pSoldier->uiStatusFlags & SOLDIER_PC)) {
          CancelAIAction(pSoldier, TRUE);
          pSoldier->bAction = AI_ACTION_CHANGE_STANCE;
          pSoldier->usActionData = ubNewStance;
          pSoldier->bActionInProgress = TRUE;
        }

        // go for it!
        // ATE: Cancel any PENDING ANIMATIONS...
        pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
        // ATE: Turn off non-interrupt flag ( this NEEDS to be done! )
        pSoldier->fInNonintAnim = FALSE;
        pSoldier->fRTInNonintAnim = FALSE;

        ChangeSoldierStance(pSoldier, ubNewStance);
      }

    }  // end of examining one soldier
  }  // end of loop
}

BOOLEAN ProcessImplicationsOfPCAttack(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE **ppTarget,
                                      int8_t bReason) {
  int16_t sTargetXPos, sTargetYPos;
  BOOLEAN fEnterCombat = TRUE;
  struct SOLDIERTYPE *pTarget = *ppTarget;

  if (pTarget->fAIFlags & AI_ASLEEP) {
    // waaaaaaaaaaaaake up!
    pTarget->fAIFlags &= (~AI_ASLEEP);
  }

  if (pTarget->ubProfile == PABLO && CheckFact(FACT_PLAYER_FOUND_ITEMS_MISSING, 0)) {
    SetFactTrue(FACT_PABLO_PUNISHED_BY_PLAYER);
  }

  if (gTacticalStatus.bBoxingState == BOXING) {
    // should have a check for "in boxing ring", no?
    if ((pSoldier->usAttackingWeapon != NOTHING && pSoldier->usAttackingWeapon != BRASS_KNUCKLES) ||
        !(pSoldier->uiStatusFlags & SOLDIER_BOXER)) {
      // someone's cheating!
      if ((Item[pSoldier->usAttackingWeapon].usItemClass == IC_BLADE ||
           Item[pSoldier->usAttackingWeapon].usItemClass == IC_PUNCH) &&
          (pTarget->uiStatusFlags & SOLDIER_BOXER)) {
        // knife or brass knuckles disqualify the player!
        BoxingPlayerDisqualified(pSoldier, BAD_ATTACK);
      } else {
        // anything else is open war!
        // gTacticalStatus.bBoxingState = NOT_BOXING;
        SetBoxingState(NOT_BOXING);
        // if we are attacking a boxer we should set them to neutral (temporarily) so that the rest
        // of the civgroup code works...
        if ((pTarget->bTeam == CIV_TEAM) && (pTarget->uiStatusFlags & SOLDIER_BOXER)) {
          SetSoldierNeutral(pTarget);
        }
      }
    }
  }

  if ((pTarget->bTeam == MILITIA_TEAM) && (pTarget->bSide == gbPlayerNum)) {
    // rebel militia attacked by the player!
    MilitiaChangesSides();
  }
  // JA2 Gold: fix Slay
  else if ((pTarget->bTeam == CIV_TEAM && pTarget->bNeutral) && pTarget->ubProfile == SLAY &&
           pTarget->bLife >= OKLIFE && CheckFact(155, 0) == FALSE) {
    TriggerNPCRecord(SLAY, 1);
  } else if ((pTarget->bTeam == CIV_TEAM) && (pTarget->ubCivilianGroup == 0) &&
             (pTarget->bNeutral) && !(pTarget->uiStatusFlags & SOLDIER_VEHICLE)) {
    if (pTarget->ubBodyType == COW && gWorldSectorX == 10 && gWorldSectorY == MAP_ROW_F) {
      // hicks could get mad!!!
      HickCowAttacked(pSoldier, pTarget);
    } else if (pTarget->ubProfile == PABLO && pTarget->bLife >= OKLIFE &&
               CheckFact(FACT_PABLO_PUNISHED_BY_PLAYER, 0) && !CheckFact(38, 0)) {
      TriggerNPCRecord(PABLO, 3);
    } else {
      // regular civ attacked, turn non-neutral
      AddToShouldBecomeHostileOrSayQuoteList(pTarget->ubID);

      if (pTarget->ubProfile == NO_PROFILE || !(gMercProfiles[pTarget->ubProfile].ubMiscFlags3 &
                                                PROFILE_MISC_FLAG3_TOWN_DOESNT_CARE_ABOUT_DEATH)) {
        // militia, if any, turn hostile
        MilitiaChangesSides();
      }
    }
  } else {
    if (pTarget->ubProfile == CARMEN)  // Carmen
    {
      // Special stuff for Carmen the bounty hunter
      if (GetSolProfile(pSoldier) != SLAY)  // attacked by someone other than Slay
      {
        // change attitude
        pTarget->bAttitude = AGGRESSIVE;
      }
    }

    if (pTarget->ubCivilianGroup && ((pTarget->bTeam == gbPlayerNum) || pTarget->bNeutral)) {
#ifdef JA2TESTVERSION
      if (pTarget->uiStatusFlags & SOLDIER_PC) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"%s is changing teams", pTarget->name);
      }
#endif
      // member of a civ group, either recruited or neutral, so should
      // change sides individually or all together

      CivilianGroupMemberChangesSides(pTarget);

      if (pTarget->ubProfile != NO_PROFILE && pTarget->bLife >= OKLIFE &&
          pTarget->bVisible == TRUE) {
        // trigger quote!
        PauseAITemporarily();
        AddToShouldBecomeHostileOrSayQuoteList(pTarget->ubID);
        // TriggerNPCWithIHateYouQuote( pTarget->ubProfile );
      }
    } else if (pTarget->ubCivilianGroup != NON_CIV_GROUP &&
               !(pTarget->uiStatusFlags & SOLDIER_BOXER)) {
      // Firing at a civ in a civ group who isn't hostile... if anyone in that civ group can see
      // this going on they should become hostile.
      CivilianGroupMembersChangeSidesWithinProximity(pTarget);
    } else if (pTarget->bTeam == gbPlayerNum && !(gTacticalStatus.uiFlags & INCOMBAT)) {
      // firing at one of our own guys who is not a rebel etc
      if (pTarget->bLife >= OKLIFE && !(pTarget->bCollapsed) && !AM_A_ROBOT(pTarget) &&
          (bReason == REASON_NORMAL_ATTACK)) {
        // OK, sturn towards the prick
        // Change to fire ready animation
        ConvertGridNoToXY(pSoldier->sGridNo, &sTargetXPos, &sTargetYPos);

        pTarget->fDontChargeReadyAPs = TRUE;
        // Ready weapon
        SoldierReadyWeapon(pTarget, sTargetXPos, sTargetYPos, FALSE);

        // ATE: Depending on personality, fire back.....

        // Do we have a gun in a\hand?
        if (Item[pTarget->inv[HANDPOS].usItem].usItemClass == IC_GUN) {
          // Toggle burst capable...
          if (!pTarget->bDoBurst) {
            if (IsGunBurstCapable(pTarget, HANDPOS, FALSE)) {
              ChangeWeaponMode(pTarget);
            }
          }

          // Fire back!
          HandleItem(pTarget, pSoldier->sGridNo, pSoldier->bLevel, pTarget->inv[HANDPOS].usItem,
                     FALSE);
        }
      }

      // don't enter combat on attack on one of our own mercs
      fEnterCombat = FALSE;
    }

    // if we've attacked a miner
    if (IsProfileAHeadMiner(pTarget->ubProfile)) {
      PlayerAttackedHeadMiner(pTarget->ubProfile);
    }
  }

  *ppTarget = pTarget;
  return (fEnterCombat);
}

struct SOLDIERTYPE *InternalReduceAttackBusyCount(uint8_t ubID, BOOLEAN fCalledByAttacker,
                                                  uint8_t ubTargetID) {
  // Strange as this may seem, this function returns a pointer to
  // the *target* in case the target has changed sides as a result
  // of being attacked
  struct SOLDIERTYPE *pSoldier;
  struct SOLDIERTYPE *pTarget;
  BOOLEAN fEnterCombat = FALSE;

  if (ubID == NOBODY) {
    pSoldier = NULL;
    pTarget = NULL;
  } else {
    pSoldier = MercPtrs[ubID];
    if (ubTargetID != NOBODY) {
      pTarget = MercPtrs[ubTargetID];
    } else {
      pTarget = NULL;
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String(">>Target ptr is null!"));
    }
  }

  if (fCalledByAttacker) {
    if (pSoldier && Item[pSoldier->inv[HANDPOS].usItem].usItemClass & IC_GUN) {
      if (pSoldier->bBulletsLeft > 0) {
        return (pTarget);
      }
    }
  }

  //	if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT))
  //	{

  if (gTacticalStatus.ubAttackBusyCount == 0) {
    // ATE: We have a problem here... if testversion, report error......
    // But for all means.... DON'T wrap!
    if ((gTacticalStatus.uiFlags & INCOMBAT)) {
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("!!!!!!! &&&&&&& Problem with attacker busy count decrementing past 0.... "
                      "preventing wrap-around."));
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
                L"Attack busy problem. Save, exit and send debug.txt + save file to Sir-Tech.");
#endif
    }
  } else {
    gTacticalStatus.ubAttackBusyCount--;
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("!!!!!!! Ending attack, attack count now %d", gTacticalStatus.ubAttackBusyCount));
  //	}

  if (gTacticalStatus.ubAttackBusyCount > 0) {
    return (pTarget);
  }

  if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT)) {
    // Check to see if anyone was suppressed
    if (pSoldier) {
      HandleSuppressionFire(pSoldier->ubTargetID, ubID);
    } else {
      HandleSuppressionFire(NOBODY, ubID);
    }

    // HandleAfterShootingGuy( pSoldier, pTarget );

    // suppression fire might cause the count to be increased, so check it again
    if (gTacticalStatus.ubAttackBusyCount > 0) {
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("!!!!!!! Starting suppression, attack count now %d",
                      gTacticalStatus.ubAttackBusyCount));
      return (pTarget);
    }
  }

  // ATE: IN MEANWHILES, we have 'combat' in realtime....
  // this is so we DON'T call freeupattacker() which will cancel
  // the AI guy's meanwhile NPC stuff.
  // OK< let's NOT do this if it was the queen attacking....
  if (AreInMeanwhile() && pSoldier != NULL && GetSolProfile(pSoldier) != QUEEN) {
    return (pTarget);
  }

  if (pTarget) {
    // reset # of shotgun pellets hit by
    pTarget->bNumPelletsHitBy = 0;
    // reset flag for making "ow" sound on being shot
  }

  if (pSoldier) {
    // reset attacking hand
    pSoldier->ubAttackingHand = HANDPOS;

    // if there is a valid target, and our attack was noticed
    if (pTarget && (pSoldier->uiStatusFlags & SOLDIER_ATTACK_NOTICED)) {
      // stuff that only applies to when we attack
      if (pTarget->ubBodyType != CROW) {
        if (pSoldier->bTeam == gbPlayerNum) {
          fEnterCombat = ProcessImplicationsOfPCAttack(pSoldier, &pTarget, REASON_NORMAL_ATTACK);
          if (!fEnterCombat) {
            DebugMsg(TOPIC_JA2, DBG_LEVEL_3, ">>Not entering combat as a result of PC attack");
          }
        }
      }

      // global

      // ATE: If we are an animal, etc, don't change to hostile...
      // ( and don't go into combat )
      if (pTarget->ubBodyType == CROW) {
        // Loop through our team, make guys who can see this fly away....
        {
          uint32_t cnt;
          struct SOLDIERTYPE *pTeamSoldier;
          uint8_t ubTeam;

          ubTeam = pTarget->bTeam;

          for (cnt = gTacticalStatus.Team[ubTeam].bFirstID, pTeamSoldier = MercPtrs[cnt];
               cnt <= gTacticalStatus.Team[ubTeam].bLastID; cnt++, pTeamSoldier++) {
            if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
              if (pTeamSoldier->ubBodyType == CROW) {
                if (pTeamSoldier->bOppList[pSoldier->ubID] == SEEN_CURRENTLY) {
                  // ZEROTIMECOUNTER( pTeamSoldier->AICounter );

                  // MakeCivHostile( pTeamSoldier, 2 );

                  HandleCrowFlyAway(pTeamSoldier);
                }
              }
            }
          }
        }

        // Don't enter combat...
        fEnterCombat = FALSE;
      }

      if (gTacticalStatus.bBoxingState == BOXING) {
        if (pTarget && pTarget->bLife <= 0) {
          // someone has won!
          EndBoxingMatch(pTarget);
        }
      }

      // if soldier and target were not both players and target was not under fire before...
      if ((pSoldier->bTeam != gbPlayerNum || pTarget->bTeam != gbPlayerNum)) {
        if (pTarget->bOppList[pSoldier->ubID] != SEEN_CURRENTLY) {
          NoticeUnseenAttacker(pSoldier, pTarget, 0);
        }
        // "under fire" lasts for 2 turns
        pTarget->bUnderFire = 2;
      }

    } else if (pTarget) {
      // something is wrong here!
      if (!pTarget->bActive || !pTarget->bInSector) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3, ">>Invalid target attacked!");
      } else if (!(pSoldier->uiStatusFlags & SOLDIER_ATTACK_NOTICED)) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3, ">>Attack not noticed");
      }
    } else {
      // no target, don't enter combat
      fEnterCombat = FALSE;
    }

    if (pSoldier->fSayAmmoQuotePending) {
      pSoldier->fSayAmmoQuotePending = FALSE;
      TacticalCharacterDialogue(pSoldier, QUOTE_OUT_OF_AMMO);
    }

    if (pSoldier->uiStatusFlags & SOLDIER_PC) {
      UnSetUIBusy(ubID);
    } else {
      FreeUpNPCFromAttacking(ubID);
    }

    if (!fEnterCombat) {
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, ">>Not to enter combat from this attack");
    }

    if (fEnterCombat && !(gTacticalStatus.uiFlags & INCOMBAT)) {
      // Go into combat!

      // If we are in a meanwhile... don't enter combat here...
      if (!AreInMeanwhile()) {
        EnterCombatMode(pSoldier->bTeam);
      }
    }

    pSoldier->uiStatusFlags &= (~SOLDIER_ATTACK_NOTICED);
  }

  if (gTacticalStatus.fKilledEnemyOnAttack) {
    // Check for death quote...
    HandleKilledQuote(MercPtrs[gTacticalStatus.ubEnemyKilledOnAttack],
                      MercPtrs[gTacticalStatus.ubEnemyKilledOnAttackKiller],
                      gTacticalStatus.ubEnemyKilledOnAttackLocation,
                      gTacticalStatus.bEnemyKilledOnAttackLevel);
    gTacticalStatus.fKilledEnemyOnAttack = FALSE;
  }

  // ATE: Check for stat changes....
  HandleAnyStatChangesAfterAttack();

  if (gTacticalStatus.fItemsSeenOnAttack && gTacticalStatus.ubCurrentTeam == gbPlayerNum) {
    gTacticalStatus.fItemsSeenOnAttack = FALSE;

    // Display quote!
    if (!AM_AN_EPC(MercPtrs[gTacticalStatus.ubItemsSeenOnAttackSoldier])) {
      TacticalCharacterDialogueWithSpecialEvent(
          MercPtrs[gTacticalStatus.ubItemsSeenOnAttackSoldier],
          (uint16_t)(QUOTE_SPOTTED_SOMETHING_ONE + Random(2)),
          DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START,
          gTacticalStatus.usItemsSeenOnAttackGridNo, 0);
    } else {
      // Turn off item lock for locators...
      gTacticalStatus.fLockItemLocators = FALSE;
      // Slide to location!
      SlideToLocation(0, gTacticalStatus.usItemsSeenOnAttackGridNo);
    }
  }

  if (gTacticalStatus.uiFlags & CHECK_SIGHT_AT_END_OF_ATTACK) {
    uint8_t ubLoop;
    struct SOLDIERTYPE *pSightSoldier;

    AllTeamsLookForAll(FALSE);

    // call fov code
    ubLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    for (pSightSoldier = MercPtrs[ubLoop]; ubLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         ubLoop++, pSightSoldier++) {
      if (pSightSoldier->bActive && pSightSoldier->bInSector) {
        RevealRoofsAndItems(pSightSoldier, TRUE, FALSE, pSightSoldier->bLevel, FALSE);
      }
    }
    gTacticalStatus.uiFlags &= ~CHECK_SIGHT_AT_END_OF_ATTACK;
  }

  DequeueAllDemandGameEvents(TRUE);

  CheckForEndOfBattle(FALSE);

  // if we're in realtime, turn off the attacker's muzzle flash at this point
  if (!(gTacticalStatus.uiFlags & INCOMBAT) && pSoldier) {
    EndMuzzleFlash(pSoldier);
  }

  if (pSoldier && pSoldier->bWeaponMode == WM_ATTACHED) {
    // change back to single shot
    pSoldier->bWeaponMode = WM_NORMAL;
  }

  // record last target
  // Check for valid target!
  if (pSoldier) {
    pSoldier->sLastTarget = pSoldier->sTargetGridNo;
  }

  return (pTarget);
}

struct SOLDIERTYPE *ReduceAttackBusyCount(uint8_t ubID, BOOLEAN fCalledByAttacker) {
  if (ubID == NOBODY) {
    return (InternalReduceAttackBusyCount(ubID, fCalledByAttacker, NOBODY));
  } else {
    return (InternalReduceAttackBusyCount(ubID, fCalledByAttacker, MercPtrs[ubID]->ubTargetID));
  }
}

struct SOLDIERTYPE *FreeUpAttacker(uint8_t ubID) {
  // Strange as this may seem, this function returns a pointer to
  // the *target* in case the target has changed sides as a result
  // of being attacked

  return (ReduceAttackBusyCount(ubID, TRUE));
}

struct SOLDIERTYPE *FreeUpAttackerGivenTarget(uint8_t ubID, uint8_t ubTargetID) {
  // Strange as this may seem, this function returns a pointer to
  // the *target* in case the target has changed sides as a result
  // of being attacked

  return (InternalReduceAttackBusyCount(ubID, TRUE, ubTargetID));
}

struct SOLDIERTYPE *ReduceAttackBusyGivenTarget(uint8_t ubID, uint8_t ubTargetID) {
  // Strange as this may seem, this function returns a pointer to
  // the *target* in case the target has changed sides as a result
  // of being attacked

  return (InternalReduceAttackBusyCount(ubID, FALSE, ubTargetID));
}

void StopMercAnimation(BOOLEAN fStop) {
  static int8_t bOldRealtimeSpeed;

  if (fStop) {
    if (!(gTacticalStatus.uiFlags & SLOW_ANIMATION)) {
      bOldRealtimeSpeed = gTacticalStatus.bRealtimeSpeed;
      gTacticalStatus.bRealtimeSpeed = -1;

      gTacticalStatus.uiFlags |= (SLOW_ANIMATION);

      ResetAllMercSpeeds();
    }
  } else {
    if (gTacticalStatus.uiFlags & SLOW_ANIMATION) {
      gTacticalStatus.bRealtimeSpeed = bOldRealtimeSpeed;

      gTacticalStatus.uiFlags &= (~SLOW_ANIMATION);

      ResetAllMercSpeeds();
    }
  }
}

void ResetAllMercSpeeds() {
  struct SOLDIERTYPE *pSoldier;
  uint32_t cnt;

  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    pSoldier = MercPtrs[cnt];

    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      SetSoldierAniSpeed(pSoldier);
    }
  }
}

void SetActionToDoOnceMercsGetToLocation(uint8_t ubActionCode, int8_t bNumMercsWaiting,
                                         uint32_t uiData1, uint32_t uiData2, uint32_t uiData3) {
  gubWaitingForAllMercsToExitCode = ubActionCode;
  gbNumMercsUntilWaitingOver = bNumMercsWaiting;
  guiWaitingForAllMercsToExitData[0] = uiData1;
  guiWaitingForAllMercsToExitData[1] = uiData2;
  guiWaitingForAllMercsToExitData[2] = uiData3;

  // Setup timer counter and report back if too long....
  guiWaitingForAllMercsToExitTimer = GetJA2Clock();

  // ATE: Set flag to ignore sight...
  gTacticalStatus.uiFlags |= (DISALLOW_SIGHT);
}

void HandleBloodForNewGridNo(struct SOLDIERTYPE *pSoldier) {
  // Handle bleeding...
  if ((pSoldier->bBleeding > MIN_BLEEDING_THRESHOLD)) {
    int8_t bBlood;

    bBlood = ((pSoldier->bBleeding - MIN_BLEEDING_THRESHOLD) / BLOODDIVISOR);

    if (bBlood > MAXBLOODQUANTITY) {
      bBlood = MAXBLOODQUANTITY;
    }

    // now, he shouldn't ALWAYS bleed the same amount; LOWER it perhaps. If it
    // goes less than zero, then no blood!
    bBlood -= (int8_t)Random(7);

    if (bBlood >= 0) {
      // this handles all soldiers' dropping blood during movement
      DropBlood(pSoldier, bBlood, pSoldier->bVisible);
    }
  }
}

void CencelAllActionsForTimeCompression(void) {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  for (pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++) {
    if (IsSolActive(pSoldier)) {
      if (IsSolInSector(pSoldier)) {
        // Hault!
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

        // END AI actions
        CancelAIAction(pSoldier, TRUE);
      }
    }
  }
}

void AddManToTeam(int8_t bTeam) {
  // ATE: If not loading game!
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Increment men in sector number!
    if (gTacticalStatus.Team[bTeam].bMenInSector == 0) {
      gTacticalStatus.Team[bTeam].bTeamActive = TRUE;
    }
    gTacticalStatus.Team[bTeam].bMenInSector++;
    if (bTeam == ENEMY_TEAM) {
      gTacticalStatus.bOriginalSizeOfEnemyForce++;
    }
  }
}

void RemoveManFromTeam(int8_t bTeam) {
  // ATE; if not loading game!
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Decrement men in sector number!
    gTacticalStatus.Team[bTeam].bMenInSector--;
    if (gTacticalStatus.Team[bTeam].bMenInSector == 0) {
      gTacticalStatus.Team[bTeam].bTeamActive = FALSE;
    } else if (gTacticalStatus.Team[bTeam].bMenInSector < 0) {
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Number of people on team %d dropped to %d",
                bTeam, gTacticalStatus.Team[bTeam].bMenInSector);
#endif
      // reset!
      gTacticalStatus.Team[bTeam].bMenInSector = 0;
    }
  }
}

void RemoveSoldierFromTacticalSector(struct SOLDIERTYPE *pSoldier, BOOLEAN fAdjustSelected) {
  uint8_t ubID;
  struct SOLDIERTYPE *pNewSoldier;

  // reset merc's opplist
  InitSoldierOppList(pSoldier);

  // Remove!
  RemoveSoldierFromGridNo(pSoldier);

  RemoveMercSlot(pSoldier);

  pSoldier->bInSector = FALSE;

  // Select next avialiable guy....
  if (fAdjustSelected) {
    if (IsTacticalMode()) {
      if (gusSelectedSoldier == GetSolID(pSoldier)) {
        ubID = FindNextActiveAndAliveMerc(pSoldier, FALSE, FALSE);

        if (ubID != NOBODY && ubID != gusSelectedSoldier) {
          SelectSoldier(ubID, FALSE, FALSE);
        } else {
          // OK - let's look for another squad...
          pNewSoldier = FindNextActiveSquad(pSoldier);

          if (pNewSoldier != pSoldier) {
            // Good squad found!
            SelectSoldier(pNewSoldier->ubID, FALSE, FALSE);
          } else {
            // if here, make nobody
            gusSelectedSoldier = NOBODY;
          }
        }
      }
      UpdateTeamPanelAssignments();
    } else {
      gusSelectedSoldier = NOBODY;

      if (IsTacticalMode()) {
        // otherwise, make sure interface is team panel...
        UpdateTeamPanelAssignments();
        SetCurrentInterfacePanel((uint8_t)TEAM_PANEL);
      }
    }
  }
}

void DoneFadeOutDueToDeath(void) {
  // Quit game....
  InternalLeaveTacticalScreen(MAINMENU_SCREEN);
  // SetPendingNewScreen( MAINMENU_SCREEN );
}

void EndBattleWithUnconsciousGuysCallback(uint8_t bExitValue) {
  // Enter mapscreen.....
  CheckAndHandleUnloadingOfCurrentWorld();
}

void InitializeTacticalStatusAtBattleStart(void) {
  int8_t bLoop;
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  gTacticalStatus.ubArmyGuysKilled = 0;
  gTacticalStatus.bOriginalSizeOfEnemyForce = 0;

  gTacticalStatus.fPanicFlags = 0;
  gTacticalStatus.fEnemyFlags = 0;
  for (bLoop = 0; bLoop < NUM_PANIC_TRIGGERS; bLoop++) {
    gTacticalStatus.sPanicTriggerGridNo[bLoop] = NOWHERE;
    gTacticalStatus.bPanicTriggerIsAlarm[bLoop] = FALSE;
    gTacticalStatus.ubPanicTolerance[bLoop] = 0;
  }

  for (cnt = 0; cnt < MAXTEAMS; cnt++) {
    gTacticalStatus.Team[cnt].ubLastMercToRadio = NOBODY;
    gTacticalStatus.Team[cnt].bAwareOfOpposition = FALSE;
  }

  gTacticalStatus.ubTheChosenOne = NOBODY;

  ClearIntList();

  // make sure none of our guys have leftover shock values etc
  for (cnt = gTacticalStatus.Team[0].bFirstID; cnt <= gTacticalStatus.Team[0].bLastID; cnt++) {
    pSoldier = MercPtrs[cnt];
    pSoldier->bShock = 0;
    pSoldier->bTilesMoved = 0;
  }

  // loop through everyone; clear misc flags
  for (cnt = 0; cnt <= gTacticalStatus.Team[CIV_TEAM].bLastID; cnt++) {
    MercPtrs[cnt]->ubMiscSoldierFlags = 0;
  }
}

void DoneFadeOutDemoCreatureLevel(void) {
  // OK, insertion data found, enter sector!
  SetCurrentWorldSector(1, 16, 0);

  FadeInGameScreen();
}

void DemoEndOKCallback(int8_t bExitCode) {}

void HandleEndDemoInCreatureLevel() {}

void DeathTimerCallback(void) {
  if (gTacticalStatus.Team[CREATURE_TEAM].bMenInSector >
      gTacticalStatus.Team[ENEMY_TEAM].bMenInSector) {
    DoMessageBox(MSG_BOX_BASIC_STYLE,
                 LargeTacticalStr[LARGESTR_NOONE_LEFT_CAPABLE_OF_BATTLE_AGAINST_CREATURES_STR],
                 GAME_SCREEN, (uint8_t)MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL);
  } else {
    DoMessageBox(MSG_BOX_BASIC_STYLE, LargeTacticalStr[LARGESTR_NOONE_LEFT_CAPABLE_OF_BATTLE_STR],
                 GAME_SCREEN, (uint8_t)MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL);
  }
}

void CaptureTimerCallback(void) {
  if (gfSurrendered) {
    DoMessageBox(MSG_BOX_BASIC_STYLE, LargeTacticalStr[3], GAME_SCREEN, (uint8_t)MSG_BOX_FLAG_OK,
                 EndBattleWithUnconsciousGuysCallback, NULL);
  } else {
    DoMessageBox(MSG_BOX_BASIC_STYLE, LargeTacticalStr[LARGESTR_HAVE_BEEN_CAPTURED], GAME_SCREEN,
                 (uint8_t)MSG_BOX_FLAG_OK, EndBattleWithUnconsciousGuysCallback, NULL);
  }
  gfSurrendered = FALSE;
}

void DoPOWPathChecks(void) {
  int32_t iLoop;
  struct SOLDIERTYPE *pSoldier;

  // loop through all mercs on our team and if they are POWs in sector, do POW path check and
  // put on a squad if available
  for (iLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
       iLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; iLoop++) {
    pSoldier = MercPtrs[iLoop];

    if (IsSolActive(pSoldier) && pSoldier->bInSector &&
        GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
      // check to see if POW has been freed!
      // this will be true if a path can be made from the POW to either of 3 gridnos
      // 10492 (hallway) or 10482 (outside), or 9381 (outside)
      if (FindBestPath(pSoldier, 10492, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE)) {
        // drop out of if
      } else if (FindBestPath(pSoldier, 10482, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE)) {
        // drop out of if
      } else if (FindBestPath(pSoldier, 9381, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE)) {
        // drop out of if
      } else {
        continue;
      }
      // free! free!
      // put them on any available squad
      pSoldier->bNeutral = FALSE;
      AddCharacterToAnySquad(pSoldier);
      DoMercBattleSound(pSoldier, BATTLE_SOUND_COOL1);
    }
  }
}

BOOLEAN HostileCiviliansPresent(void) {
  int32_t iLoop;
  struct SOLDIERTYPE *pSoldier;

  if (gTacticalStatus.Team[CIV_TEAM].bTeamActive == FALSE) {
    return (FALSE);
  }

  for (iLoop = gTacticalStatus.Team[CIV_TEAM].bFirstID;
       iLoop <= gTacticalStatus.Team[CIV_TEAM].bLastID; iLoop++) {
    pSoldier = MercPtrs[iLoop];

    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife > 0 &&
        !pSoldier->bNeutral) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN HostileCiviliansWithGunsPresent(void) {
  int32_t iLoop;
  struct SOLDIERTYPE *pSoldier;

  if (gTacticalStatus.Team[CIV_TEAM].bTeamActive == FALSE) {
    return (FALSE);
  }

  for (iLoop = gTacticalStatus.Team[CIV_TEAM].bFirstID;
       iLoop <= gTacticalStatus.Team[CIV_TEAM].bLastID; iLoop++) {
    pSoldier = MercPtrs[iLoop];

    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife > 0 &&
        !pSoldier->bNeutral) {
      if (FindAIUsableObjClass(pSoldier, IC_WEAPON) == -1) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN HostileBloodcatsPresent(void) {
  int32_t iLoop;
  struct SOLDIERTYPE *pSoldier;

  if (gTacticalStatus.Team[CREATURE_TEAM].bTeamActive == FALSE) {
    return (FALSE);
  }

  for (iLoop = gTacticalStatus.Team[CREATURE_TEAM].bFirstID;
       iLoop <= gTacticalStatus.Team[CREATURE_TEAM].bLastID; iLoop++) {
    pSoldier = MercPtrs[iLoop];

    // KM : Aug 11, 1999 -- Patch fix:  Removed the check for bNeutral.  Bloodcats automatically
    // become hostile 		 on site.  Because the check used to be there, it was possible to
    // get into a 2nd battle elsewhere
    //     which is BAD BAD BAD!
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife > 0 &&
        pSoldier->ubBodyType == BLOODCAT) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void HandleCreatureTenseQuote() {
  uint8_t ubMercsInSector[20] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt;
  int32_t uiTime;

  // Check for quote seeing creature attacks....
  if (gubQuest[QUEST_CREATURES] != QUESTDONE) {
    if (gTacticalStatus.uiFlags & IN_CREATURE_LAIR) {
      if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
        uiTime = GetJA2Clock();

        if ((uiTime - gTacticalStatus.uiCreatureTenseQuoteLastUpdate) >
            (uint32_t)(gTacticalStatus.sCreatureTenseQuoteDelay * 1000)) {
          gTacticalStatus.uiCreatureTenseQuoteLastUpdate = uiTime;

          // set up soldier ptr as first element in mercptrs list
          cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

          // run through list
          for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
               cnt++, pTeamSoldier++) {
            // Add guy if he's a candidate...
            if (OK_INSECTOR_MERC(pTeamSoldier) && !AM_AN_EPC(pTeamSoldier) &&
                !(pTeamSoldier->uiStatusFlags & SOLDIER_GASSED) && !(AM_A_ROBOT(pTeamSoldier)) &&
                !pTeamSoldier->fMercAsleep) {
              ubMercsInSector[ubNumMercs] = (uint8_t)cnt;
              ubNumMercs++;
            }
          }

          // If we are > 0
          if (ubNumMercs > 0) {
            ubChosenMerc = (uint8_t)Random(ubNumMercs);

            DoCreatureTensionQuote(MercPtrs[ubMercsInSector[ubChosenMerc]]);
          }

          // Adjust delay....
          gTacticalStatus.sCreatureTenseQuoteDelay = (int16_t)(60 + Random(60));
        }
      }
    }
  }
}

void DoCreatureTensionQuote(struct SOLDIERTYPE *pSoldier) {
  int32_t iRandomQuote;
  BOOLEAN fCanDoQuote = TRUE;
  int32_t iQuoteToUse;

  // Check for playing smell quote....
  iRandomQuote = Random(3);

  switch (iRandomQuote) {
    case 0:

      iQuoteToUse = QUOTE_SMELLED_CREATURE;
      if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_SMELLED_CREATURE)) {
        // set flag
        pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_SMELLED_CREATURE;
      } else {
        fCanDoQuote = FALSE;
      }
      break;

    case 1:

      iQuoteToUse = QUOTE_TRACES_OF_CREATURE_ATTACK;
      if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK)) {
        // set flag
        pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_SPOTTING_CREATURE_ATTACK;
      } else {
        fCanDoQuote = FALSE;
      }
      break;

    case 2:

      iQuoteToUse = QUOTE_WORRIED_ABOUT_CREATURE_PRESENCE;
      if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES)) {
        // set flag
        pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_WORRIED_ABOUT_CREATURES;
      } else {
        fCanDoQuote = FALSE;
      }
      break;
  }

  if (fCanDoQuote) {
    TacticalCharacterDialogue(pSoldier, (int16_t)iQuoteToUse);
  }
}
