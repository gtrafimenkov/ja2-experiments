// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/GameInit.h"

#include "Cheats.h"
#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "JAScreens.h"
#include "Laptop/AIMMembers.h"
#include "Laptop/BobbyR.h"
#include "Laptop/Email.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Laptop.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Strategic/CampaignInit.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/AirRaid.h"
#include "Tactical/AnimationData.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/MercEntering.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Vehicles.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/WorldDef.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"

// Temp function
void QuickSetupOfMercProfileItems(uint32_t uiCount, uint8_t ubProfileIndex);
BOOLEAN QuickGameMemberHireMerc(uint8_t ubCurrentSoldier);
extern uint32_t guiExitScreen;
extern uint32_t uiMeanWhileFlags;
extern BOOLEAN gfGamePaused;

extern UNDERGROUND_SECTORINFO *FindUnderGroundSector(uint8_t sMapX, uint8_t sMapY, uint8_t bMapZ);
extern void InitVehicles();

uint8_t gubScreenCount = 0;

void InitNPCs(void) {
  MERCPROFILESTRUCT *pProfile;

  // add the pilot at a random location!
  pProfile = &(gMercProfiles[SKYRIDER]);
  switch (Random(4)) {
    case 0:
      pProfile->sSectorX = 15;
      pProfile->sSectorY = MAP_ROW_B;
      pProfile->bSectorZ = 0;
      break;
    case 1:
      pProfile->sSectorX = 14;
      pProfile->sSectorY = MAP_ROW_E;
      pProfile->bSectorZ = 0;
      break;
    case 2:
      pProfile->sSectorX = 12;
      pProfile->sSectorY = MAP_ROW_D;
      pProfile->bSectorZ = 0;
      break;
    case 3:
      pProfile->sSectorX = 16;
      pProfile->sSectorY = MAP_ROW_C;
      pProfile->bSectorZ = 0;
      break;
  }

#ifdef JA2TESTVERSION
  ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Skyrider in %c %d", 'A' + pProfile->sSectorY - 1,
            pProfile->sSectorX);
#endif
  // use alternate map, with Skyrider's shack, in this sector
  SectorInfo[GetSectorID8((uint8_t)pProfile->sSectorX, (uint8_t)pProfile->sSectorY)].uiFlags |=
      SF_USE_ALTERNATE_MAP;

  // set up Madlab's secret lab (he'll be added when the meanwhile scene occurs)

  switch (Random(4)) {
    case 0:
      // use alternate map in this sector
      SectorInfo[GetSectorID8(7, MAP_ROW_H)].uiFlags |= SF_USE_ALTERNATE_MAP;
      break;
    case 1:
      SectorInfo[GetSectorID8(16, MAP_ROW_H)].uiFlags |= SF_USE_ALTERNATE_MAP;
      break;
    case 2:
      SectorInfo[GetSectorID8(11, MAP_ROW_I)].uiFlags |= SF_USE_ALTERNATE_MAP;
      break;
    case 3:
      SectorInfo[GetSectorID8(4, MAP_ROW_E)].uiFlags |= SF_USE_ALTERNATE_MAP;
      break;
  }

  // add Micky in random location

  pProfile = &(gMercProfiles[MICKY]);
  switch (Random(5)) {
    case 0:
      pProfile->sSectorX = 9;
      pProfile->sSectorY = MAP_ROW_G;
      pProfile->bSectorZ = 0;
      break;
    case 1:
      pProfile->sSectorX = 13;
      pProfile->sSectorY = MAP_ROW_D;
      pProfile->bSectorZ = 0;
      break;
    case 2:
      pProfile->sSectorX = 5;
      pProfile->sSectorY = MAP_ROW_C;
      pProfile->bSectorZ = 0;
      break;
    case 3:
      pProfile->sSectorX = 2;
      pProfile->sSectorY = MAP_ROW_H;
      pProfile->bSectorZ = 0;
      break;
    case 4:
      pProfile->sSectorX = 6;
      pProfile->sSectorY = MAP_ROW_C;
      pProfile->bSectorZ = 0;
      break;
  }

#ifdef JA2TESTVERSION
  ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"%s in %c %d", pProfile->zNickname,
            'A' + pProfile->sSectorY - 1, pProfile->sSectorX);
#endif

  // use alternate map in this sector
  // SectorInfo[ GetSectorID8( pProfile->sSectorX, pProfile->sSectorY ) ].uiFlags |=
  // SF_USE_ALTERNATE_MAP;

  gfPlayerTeamSawJoey = FALSE;

  if (gGameOptions.fSciFi) {
    // add Bob
    pProfile = &(gMercProfiles[BOB]);
    pProfile->sSectorX = 8;
    pProfile->sSectorY = MAP_ROW_F;
    pProfile->bSectorZ = 0;

    // add Gabby in random location
    pProfile = &(gMercProfiles[GABBY]);
    switch (Random(2)) {
      case 0:
        pProfile->sSectorX = 11;
        pProfile->sSectorY = MAP_ROW_H;
        pProfile->bSectorZ = 0;
        break;
      case 1:
        pProfile->sSectorX = 4;
        pProfile->sSectorY = MAP_ROW_I;
        pProfile->bSectorZ = 0;
        break;
    }

#ifdef JA2TESTVERSION
    ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"%s in %c %d", pProfile->zNickname,
              'A' + pProfile->sSectorY - 1, pProfile->sSectorX);
#endif

    // use alternate map in this sector
    SectorInfo[GetSectorID8((uint8_t)pProfile->sSectorX, (uint8_t)pProfile->sSectorY)].uiFlags |=
        SF_USE_ALTERNATE_MAP;
  } else {  // not scifi, so use alternate map in Tixa's b1 level that doesn't have the stairs going
            // down to the caves.
    UNDERGROUND_SECTORINFO *pSector;
    pSector = FindUnderGroundSector(9, 10, 1);  // j9_b1
    if (pSector) {
      pSector->uiFlags |= SF_USE_ALTERNATE_MAP;
    }
  }

  // init hospital variables
  giHospitalTempBalance = 0;
  giHospitalRefund = 0;
  gbHospitalPriceModifier = 0;

  // set up Devin so he will be placed ASAP
  gMercProfiles[DEVIN].bNPCData = 3;
}

void InitBloodCatSectors() {
  int32_t i;
  // Hard coded table of bloodcat populations.  We don't have
  // access to the real population (if different) until we physically
  // load the map.  If the real population is different, then an error
  // will be reported.
  for (i = 0; i < 255; i++) {
    SectorInfo[i].bBloodCats = -1;
  }
  SectorInfo[SEC_A15].bBloodCatPlacements = 9;
  SectorInfo[SEC_B4].bBloodCatPlacements = 9;
  SectorInfo[SEC_B16].bBloodCatPlacements = 8;
  SectorInfo[SEC_C3].bBloodCatPlacements = 12;
  SectorInfo[SEC_C8].bBloodCatPlacements = 13;
  SectorInfo[SEC_C11].bBloodCatPlacements = 7;
  SectorInfo[SEC_D4].bBloodCatPlacements = 8;
  SectorInfo[SEC_D9].bBloodCatPlacements = 12;
  SectorInfo[SEC_E11].bBloodCatPlacements = 10;
  SectorInfo[SEC_E13].bBloodCatPlacements = 14;
  SectorInfo[SEC_F3].bBloodCatPlacements = 13;
  SectorInfo[SEC_F5].bBloodCatPlacements = 7;
  SectorInfo[SEC_F7].bBloodCatPlacements = 12;
  SectorInfo[SEC_F12].bBloodCatPlacements = 9;
  SectorInfo[SEC_F14].bBloodCatPlacements = 14;
  SectorInfo[SEC_F15].bBloodCatPlacements = 8;
  SectorInfo[SEC_G6].bBloodCatPlacements = 7;
  SectorInfo[SEC_G10].bBloodCatPlacements = 12;
  SectorInfo[SEC_G12].bBloodCatPlacements = 11;
  SectorInfo[SEC_H5].bBloodCatPlacements = 9;
  SectorInfo[SEC_I4].bBloodCatPlacements = 8;
  SectorInfo[SEC_I15].bBloodCatPlacements = 8;
  SectorInfo[SEC_J6].bBloodCatPlacements = 11;
  SectorInfo[SEC_K3].bBloodCatPlacements = 12;
  SectorInfo[SEC_K6].bBloodCatPlacements = 14;
  SectorInfo[SEC_K10].bBloodCatPlacements = 12;
  SectorInfo[SEC_K14].bBloodCatPlacements = 14;

  switch (gGameOptions.ubDifficultyLevel) {
    case DIF_LEVEL_EASY:  // 50%
      SectorInfo[SEC_I16].bBloodCatPlacements = 14;
      SectorInfo[SEC_I16].bBloodCats = 14;
      SectorInfo[SEC_N5].bBloodCatPlacements = 8;
      SectorInfo[SEC_N5].bBloodCats = 8;
      break;
    case DIF_LEVEL_MEDIUM:  // 75%
      SectorInfo[SEC_I16].bBloodCatPlacements = 19;
      SectorInfo[SEC_I16].bBloodCats = 19;
      SectorInfo[SEC_N5].bBloodCatPlacements = 10;
      SectorInfo[SEC_N5].bBloodCats = 10;
      break;
    case DIF_LEVEL_HARD:  // 100%
      SectorInfo[SEC_I16].bBloodCatPlacements = 26;
      SectorInfo[SEC_I16].bBloodCats = 26;
      SectorInfo[SEC_N5].bBloodCatPlacements = 12;
      SectorInfo[SEC_N5].bBloodCats = 12;
      break;
  }
}

void InitStrategicLayer(void) {
  // Clear starategic layer!
  SetupNewStrategicGame();
  InitQuestEngine();

  // Setup a new campaign via the enemy perspective.
  InitNewCampaign();
  // Init Squad Lists
  InitSquads();
  // Init vehicles
  InitVehicles();
  // init town loyalty
  InitTownLoyalty();
  // init the mine management system
  InitializeMines();
  // initialize map screen flags
  InitMapScreenFlags();
  // initialize NPCs, select alternate maps, etc
  InitNPCs();
  // init Skyrider and his helicopter
  InitializeHelicopter();
  // Clear out the vehicle list
  ClearOutVehicleList();

  InitBloodCatSectors();

  InitializeSAMSites();

  // make Orta, Tixa, SAM sites not found
  InitMapSecrets();

  // free up any leave list arrays that were left allocated
  ShutDownLeaveList();
  // re-set up leave list arrays for dismissed mercs
  InitLeaveList();

  // reset time compression mode to X0 (this will also pause it)
  SetGameTimeCompressionLevel(TIME_COMPRESS_X0);

  // select A9 Omerta as the initial selected sector
  ChangeSelectedMapSector(9, 1, 0);

  // Reset these flags or mapscreen could be disabled and cause major headache.
  fDisableDueToBattleRoster = FALSE;
  fDisableMapInterfaceDueToBattle = FALSE;
}

void ShutdownStrategicLayer() {
  DeleteAllStrategicEvents();
  RemoveAllGroups();
  TrashUndergroundSectorInfo();
  DeleteCreatureDirectives();
  KillStrategicAI();
}

BOOLEAN InitNewGame(BOOLEAN fReset) {
  int32_t iStartingCash;

  //	static fScreenCount = 0;

  if (fReset) {
    gubScreenCount = 0;
    return (TRUE);
  }

  // reset meanwhile flags
  uiMeanWhileFlags = 0;

  // Reset the selected soldier
  gusSelectedSoldier = NOBODY;

  if (gubScreenCount == 0) {
    if (!LoadMercProfiles()) return (FALSE);
  }

  // Initialize the Arms Dealers and Bobby Rays inventory
  if (gubScreenCount == 0) {
    // Init all the arms dealers inventory
    InitAllArmsDealers();
    InitBobbyRayInventory();
  }

  // clear tactical
  ClearTacticalMessageQueue();

  // clear mapscreen messages
  FreeGlobalMessageList();

  // IF our first time, go into laptop!
  if (gubScreenCount == 0) {
    // Init the laptop here
    InitLaptopAndLaptopScreens();

    InitStrategicLayer();

    // Set new game flag
    SetLaptopNewGameFlag();

    // this is for the "mercs climbing down from a rope" animation, NOT Skyrider!!
    ResetHeliSeats();

    // Setup two new messages!
    AddPreReadEmail(OLD_ENRICO_1, OLD_ENRICO_1_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
    AddPreReadEmail(OLD_ENRICO_2, OLD_ENRICO_2_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
    AddPreReadEmail(RIS_REPORT, RIS_REPORT_LENGTH, RIS_EMAIL, GetWorldTotalMin());
    AddPreReadEmail(OLD_ENRICO_3, OLD_ENRICO_3_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
    AddEmail(IMP_EMAIL_INTRO, IMP_EMAIL_INTRO_LENGTH, CHAR_PROFILE_SITE, GetWorldTotalMin());
    // AddEmail(ENRICO_CONGRATS,ENRICO_CONGRATS_LENGTH,MAIL_ENRICO, GetWorldTotalMin() );

    // ATE: Set starting cash....
    switch (gGameOptions.ubDifficultyLevel) {
      case DIF_LEVEL_EASY:

        iStartingCash = 45000;
        break;

      case DIF_LEVEL_MEDIUM:

        iStartingCash = 35000;
        break;

      case DIF_LEVEL_HARD:

        iStartingCash = 30000;
        break;

      default:
        Assert(0);
        return (FALSE);
    }

    // Setup initial money
    AddTransactionToPlayersBook(ANONYMOUS_DEPOSIT, 0, iStartingCash);

    {
      uint32_t uiDaysTimeMercSiteAvailable = Random(2) + 1;

      // schedule email for message from spec at 7am 3 days in the future
      AddFutureDayStrategicEvent(EVENT_DAY3_ADD_EMAIL_FROM_SPECK, 60 * 7, 0,
                                 uiDaysTimeMercSiteAvailable);
    }

    SetLaptopExitScreen(INIT_SCREEN);
    SetPendingNewScreen(LAPTOP_SCREEN);
    gubScreenCount = 1;

    // Set the fact the game is in progress
    gTacticalStatus.fHasAGameBeenStarted = TRUE;

    return (TRUE);
  }

  if (gubScreenCount == 1) {
    gubScreenCount = 2;
    return (TRUE);
  }

  return (TRUE);
}

BOOLEAN AnyMercsHired() {
  int32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier;
  int16_t bLastTeamID;

  // Find first guy availible in team
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  bLastTeamID = gTacticalStatus.Team[gbPlayerNum].bLastID;

  // look for all mercs on the same team,
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= bLastTeamID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void QuickStartGame() {
  int32_t cnt;
  uint16_t usVal;
  uint8_t ub1 = 0, ub2 = 0;

  for (cnt = 0; cnt < 3; cnt++) {
    if (cnt == 0) {
      usVal = (uint16_t)Random(40);

      QuickSetupOfMercProfileItems(cnt, (uint8_t)usVal);
      QuickGameMemberHireMerc((uint8_t)usVal);
    } else if (cnt == 1) {
      do {
        usVal = (uint16_t)Random(40);
      } while (usVal != ub1);

      QuickSetupOfMercProfileItems(cnt, (uint8_t)usVal);
      QuickGameMemberHireMerc((uint8_t)usVal);
    } else if (cnt == 2) {
      do {
        usVal = (uint16_t)Random(40);
      } while (usVal != ub1 && usVal != ub2);

      QuickSetupOfMercProfileItems(cnt, (uint8_t)usVal);
      QuickGameMemberHireMerc((uint8_t)usVal);
    }
  }
}

// TEMP FUNCTION!
void QuickSetupOfMercProfileItems(uint32_t uiCount, uint8_t ubProfileIndex) {
  // Quickly give some guys we hire some items

  if (uiCount == 0) {
    // CreateGun( GLOCK_17, &(pSoldier->inv[ HANDPOS ] ) );
    // gMercProfiles[ ubProfileIndex ].inv[ HANDPOS ] = HAND_GRENADE;
    // gMercProfiles[ ubProfileIndex ].bInvStatus[ HANDPOS ] = 100;
    // gMercProfiles[ ubProfileIndex ].bInvNumber[ HANDPOS ] = 3;
    gMercProfiles[ubProfileIndex].inv[HANDPOS] = C7;
    gMercProfiles[ubProfileIndex].bInvStatus[HANDPOS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[HANDPOS] = 1;

    gMercProfiles[ubProfileIndex].inv[BIGPOCK1POS] = CAWS;
    gMercProfiles[ubProfileIndex].bInvStatus[BIGPOCK1POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[BIGPOCK1POS] = 1;

    gMercProfiles[ubProfileIndex].bSkillTrait = MARTIALARTS;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK3POS] = KEY_2;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK3POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK3POS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK5POS] = LOCKSMITHKIT;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK5POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK5POS] = 1;

    gMercProfiles[ubProfileIndex].inv[BIGPOCK3POS] = MEDICKIT;
    gMercProfiles[ubProfileIndex].bInvStatus[BIGPOCK3POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[BIGPOCK3POS] = 1;

    gMercProfiles[ubProfileIndex].inv[BIGPOCK4POS] = SHAPED_CHARGE;
    gMercProfiles[ubProfileIndex].bInvStatus[BIGPOCK4POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[BIGPOCK4POS] = 1;

    // TEMP!
    // make carman's opinion of us high!
    gMercProfiles[78].bMercOpinion[ubProfileIndex] = 25;

  } else if (uiCount == 1) {
    gMercProfiles[ubProfileIndex].inv[HANDPOS] = CAWS;
    gMercProfiles[ubProfileIndex].bInvStatus[HANDPOS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[HANDPOS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK3POS] = KEY_1;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK3POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK3POS] = 1;

  } else if (uiCount == 2) {
    gMercProfiles[ubProfileIndex].inv[HANDPOS] = GLOCK_17;
    gMercProfiles[ubProfileIndex].bInvStatus[HANDPOS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[HANDPOS] = 1;

    gMercProfiles[ubProfileIndex].inv[SECONDHANDPOS] = 5;
    gMercProfiles[ubProfileIndex].bInvStatus[SECONDHANDPOS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SECONDHANDPOS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK1POS] = SILENCER;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK1POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK1POS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK2POS] = SNIPERSCOPE;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK2POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK2POS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK3POS] = LASERSCOPE;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK3POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK3POS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK5POS] = BIPOD;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK5POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK5POS] = 1;

    gMercProfiles[ubProfileIndex].inv[SMALLPOCK6POS] = LOCKSMITHKIT;
    gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK6POS] = 100;
    gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK6POS] = 1;

  } else {
    gMercProfiles[ubProfileIndex].inv[HANDPOS] = (uint8_t)Random(30);
    gMercProfiles[ubProfileIndex].bInvNumber[HANDPOS] = 1;
  }

  gMercProfiles[ubProfileIndex].inv[HELMETPOS] = KEVLAR_HELMET;
  gMercProfiles[ubProfileIndex].bInvStatus[HELMETPOS] = 100;
  gMercProfiles[ubProfileIndex].bInvNumber[HELMETPOS] = 1;

  gMercProfiles[ubProfileIndex].inv[VESTPOS] = KEVLAR_VEST;
  gMercProfiles[ubProfileIndex].bInvStatus[VESTPOS] = 100;
  gMercProfiles[ubProfileIndex].bInvNumber[VESTPOS] = 1;

  gMercProfiles[ubProfileIndex].inv[BIGPOCK2POS] = RDX;
  gMercProfiles[ubProfileIndex].bInvStatus[BIGPOCK2POS] = 10;
  gMercProfiles[ubProfileIndex].bInvNumber[BIGPOCK2POS] = 1;

  gMercProfiles[ubProfileIndex].inv[SMALLPOCK4POS] = HAND_GRENADE;
  gMercProfiles[ubProfileIndex].bInvStatus[SMALLPOCK4POS] = 100;
  gMercProfiles[ubProfileIndex].bInvNumber[SMALLPOCK4POS] = 4;

  // Give special items to some NPCs
  // gMercProfiles[ 78 ].inv[ SMALLPOCK4POS ] = TERRORIST_INFO;
  // gMercProfiles[ 78 ].bInvStatus[ SMALLPOCK4POS ] = 100;
  // gMercProfiles[ 78 ].bInvNumber[ SMALLPOCK4POS ] = 1;
}

BOOLEAN QuickGameMemberHireMerc(uint8_t ubCurrentSoldier) {
  MERC_HIRE_STRUCT HireMercStruct;

  memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

  HireMercStruct.ubProfileID = ubCurrentSoldier;

  HireMercStruct.sSectorX = gsMercArriveSectorX;
  HireMercStruct.sSectorY = gsMercArriveSectorY;
  HireMercStruct.fUseLandingZoneForArrival = TRUE;

  HireMercStruct.fCopyProfileItemsOver = TRUE;
  HireMercStruct.ubInsertionCode = INSERTION_CODE_CHOPPER;

  HireMercStruct.iTotalContractLength = 7;

  // specify when the merc should arrive
  HireMercStruct.uiTimeTillMercArrives = 0;

  // if we succesfully hired the merc
  if (!HireMerc(&HireMercStruct)) {
    return (FALSE);
  }

  // add an entry in the finacial page for the hiring of the merc
  AddTransactionToPlayersBook(HIRED_MERC, ubCurrentSoldier,
                              -(int32_t)gMercProfiles[ubCurrentSoldier].uiWeeklySalary);

  if (gMercProfiles[ubCurrentSoldier].bMedicalDeposit) {
    // add an entry in the finacial page for the medical deposit
    AddTransactionToPlayersBook(MEDICAL_DEPOSIT, ubCurrentSoldier,
                                -(gMercProfiles[ubCurrentSoldier].sMedicalDepositAmount));
  }

  // add an entry in the history page for the hiring of the merc
  AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_AIM, ubCurrentSoldier, GetWorldTotalMin(), -1, -1);

  return (TRUE);
}

// This function is called when the game is REstarted.  Things that need to be reinited are placed
// in here
void ReStartingGame() {
  uint16_t cnt;

  // Pause the game
  gfGamePaused = TRUE;

  // Reset the sectors
  gWorldSectorX = gWorldSectorY = 0;
  gbWorldSectorZ = -1;

  SoundStopAll();

  // we are going to restart a game so initialize the variable so we can initialize a new game
  InitNewGame(TRUE);

  // Deletes all the Temp files in the Maps\Temp directory
  InitTacticalSave(TRUE);

  // Loop through all the soldier and delete them all
  for (cnt = 0; cnt < TOTAL_SOLDIERS; cnt++) {
    TacticalRemoveSoldier(cnt);
  }

  // Re-init overhead...
  InitOverhead();

  // Reset the email list
  ShutDownEmailList();

  // Reinit the laptopn screen variables
  InitLaptopAndLaptopScreens();
  LaptopScreenInit();

  // Reload the Merc profiles
  LoadMercProfiles();

  // Reload quote files
  ReloadAllQuoteFiles();

  // Initialize the ShopKeeper Interface ( arms dealer inventory, etc. )
  ShopKeeperScreenInit();

  // Delete the world info
  TrashWorld();

  // Init the help screen system
  InitHelpScreenSystem();

  EmptyDialogueQueue();

  if (InAirRaid()) {
    EndAirRaid();
  }

#ifdef JA2TESTVERSION
  // Reset so we can use the 'cheat key' to start with mercs
  TempHiringOfMercs(0, TRUE);
#endif

  // Make sure the game starts in the TEAM panel ( it wasnt being reset )
  gsCurInterfacePanel = TEAM_PANEL;

  // Delete all the strategic events
  DeleteAllStrategicEvents();

  // This function gets called when ur in a game a click the quit to main menu button, therefore no
  // game is in progress
  gTacticalStatus.fHasAGameBeenStarted = FALSE;

  // Reset timer callbacks
  gpCustomizableTimerCallback = NULL;

  gubCheatLevel = STARTING_CHEAT_LEVEL;
}
