// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/PreBattleInterface.h"

#include <stdio.h>

#include "Cheats.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/History.h"
#include "SGP/ButtonSystem.h"
#include "SGP/English.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/Campaign.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Squads.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TacticalPlacementGUI.h"
#include "Town.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/PopUpBox.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"

extern void InitializeTacticalStatusAtBattleStart();
extern BOOLEAN gfDelayAutoResolveStart;
extern BOOLEAN gfTransitionMapscreenToAutoResolve;

#ifdef JA2BETAVERSION
extern BOOLEAN gfExitViewer;
#endif

extern BOOLEAN fMapScreenBottomDirty;

BOOLEAN gfTacticalTraversal = FALSE;
struct GROUP *gpTacticalTraversalGroup = NULL;
struct SOLDIERTYPE *gpTacticalTraversalChosenSoldier = NULL;

BOOLEAN gfAutomaticallyStartAutoResolve = FALSE;
BOOLEAN gfAutoAmbush = FALSE;
BOOLEAN gfHighPotentialForAmbush = FALSE;
BOOLEAN gfGotoSectorTransition = FALSE;
BOOLEAN gfEnterAutoResolveMode = FALSE;
BOOLEAN gfEnteringMapScreenToEnterPreBattleInterface = FALSE;
BOOLEAN gfIgnoreAllInput = TRUE;

enum  // GraphicIDs for the panel
{
  MAINPANEL,
  TITLE_BAR_PIECE,
  TOP_COLUMN,
  BOTTOM_COLUMN,
  UNINVOLVED_HEADER
};

// The start of the black space
#define TOP_Y 113
// The end of the black space
#define BOTTOM_Y 349
// The internal height of the uninvolved panel
#define INTERNAL_HEIGHT 27
// The actual height of the uninvolved panel
#define ACTUAL_HEIGHT 34
// The height of each row
#define ROW_HEIGHT 10

BOOLEAN gfDisplayPotentialRetreatPaths = FALSE;
uint16_t gusRetreatButtonLeft, gusRetreatButtonTop, gusRetreatButtonRight, gusRetreatButtonBottom;

struct GROUP *gpBattleGroup = NULL;

void AutoResolveBattleCallback(GUI_BUTTON *btn, int32_t reason);
void GoToSectorCallback(GUI_BUTTON *btn, int32_t reason);
void RetreatMercsCallback(GUI_BUTTON *btn, int32_t reason);

void GetSoldierConditionInfo(struct SOLDIERTYPE *pSoldier, wchar_t *szCondition,
                             int szConditionSize, uint8_t *pubHPPercent, uint8_t *pubBPPercent);

void CheckForRobotAndIfItsControlled(void);

void PutNonSquadMercsInBattleSectorOnSquads(BOOLEAN fExitVehicles);
void PutNonSquadMercsInPlayerGroupOnSquads(struct GROUP *pGroup, BOOLEAN fExitVehicles);

/*
void InvolvedMoveCallback( struct MOUSE_REGION *reg, int32_t reason );
void InvolvedClickCallback( struct MOUSE_REGION *reg, int32_t reason );
void UninvolvedMoveCallback( struct MOUSE_REGION *reg, int32_t reason );
void UninvolvedClickCallback( struct MOUSE_REGION *reg, int32_t reason );

struct SOLDIERTYPE* InvolvedSoldier( int32_t index );
struct SOLDIERTYPE* UninvolvedSoldier( int32_t index );
*/

struct MOUSE_REGION PBInterfaceBlanket;
BOOLEAN gfPreBattleInterfaceActive = FALSE;
uint32_t iPBButton[3];
uint32_t iPBButtonImage[3];
uint32_t uiInterfaceImages;
BOOLEAN gfRenderPBInterface;
BOOLEAN gfPBButtonsHidden;
BOOLEAN fDisableMapInterfaceDueToBattle = FALSE;

void DoTransitionFromMapscreenToPreBattleInterface();

BOOLEAN gfBlinkHeader;

// mouse regions in mapscreen proper than must have thier help text disabled then re-enabled
extern struct MOUSE_REGION gMapStatusBarsRegion;
extern struct MOUSE_REGION gCharInfoHandRegion;

extern int32_t giMapContractButton;
extern int32_t giCharInfoButton[2];
extern void CreateDestroyMapCharacterScrollButtons(void);
extern void UpdateTheStateOfTheNextPrevMapScreenCharacterButtons(void);

// were we showing the mapscreen inventory when the prebattle interface came up
extern BOOLEAN fShowInventoryFlag;

uint32_t guiNumInvolved;
uint32_t guiNumUninvolved;

// SAVE START

// Using the ESC key in the PBI will get rid of the PBI and go back to mapscreen, but
// only if the PBI isn't persistant (!gfPersistantPBI).
BOOLEAN gfPersistantPBI = FALSE;

// Contains general information about the type of encounter the player is faced with.  This
// determines whether or not you can autoresolve the battle or even retreat.  This code
// dictates the header that is used at the top of the PBI.
uint8_t gubEnemyEncounterCode = NO_ENCOUNTER_CODE;

// The autoresolve during tactical battle option needs more detailed information than the
// gubEnemyEncounterCode can provide.  The explicit version contains possibly unique codes
// for reasons not normally used in the PBI.  For example, if we were fighting the enemy
// in a normal situation, then shot at a civilian, the civilians associated with the victim
// would turn hostile, which would disable the ability to autoresolve the battle.
BOOLEAN gubExplicitEnemyEncounterCode = NO_ENCOUNTER_CODE;

// Location of the current battle (determines where the animated icon is blitted) and if the
// icon is to be blitted.
BOOLEAN gfBlitBattleSectorLocator = FALSE;

uint8_t gubPBSectorX = 0;
uint8_t gubPBSectorY = 0;
uint8_t gubPBSectorZ = 0;

BOOLEAN gfCantRetreatInPBI = FALSE;
// SAVE END

BOOLEAN gfUsePersistantPBI;

int32_t giHilitedInvolved, giHilitedUninvolved;

extern void CalculateGroupRetreatSector(struct GROUP *pGroup);

extern void GetMapscreenMercAssignmentString(struct SOLDIERTYPE *pSoldier, wchar_t sString[]);
extern void GetMapscreenMercLocationString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                           int sStringSize);
extern void GetMapscreenMercDestinationString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                              int sStringSize);
extern void GetMapscreenMercDepartureString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                            int sStringSize, uint8_t *pubFontColor);

#ifdef JA2BETAVERSION
// The group is passed so we can extract the sector location
void ValidateAndCorrectInBattleCounters(struct GROUP *pLocGroup) {
  SECTORINFO *pSector;
  struct GROUP *pGroup;
  uint8_t ubSectorID;
  uint8_t ubInvalidGroups = 0;

  if (!pLocGroup->ubSectorZ) {
    pGroup = gpGroupList;
    while (pGroup) {
      if (!pGroup->fPlayer) {
        if (pGroup->ubSectorX == pLocGroup->ubSectorX &&
            pGroup->ubSectorY == pLocGroup->ubSectorY) {
          if (pGroup->pEnemyGroup->ubAdminsInBattle || pGroup->pEnemyGroup->ubTroopsInBattle ||
              pGroup->pEnemyGroup->ubElitesInBattle) {
            ubInvalidGroups++;
            pGroup->pEnemyGroup->ubAdminsInBattle = 0;
            pGroup->pEnemyGroup->ubTroopsInBattle = 0;
            pGroup->pEnemyGroup->ubElitesInBattle = 0;
          }
        }
      }
      pGroup = pGroup->next;
    }
  }

  ubSectorID = (uint8_t)GetSectorID8(pLocGroup->ubSectorX, pLocGroup->ubSectorY);
  pSector = &SectorInfo[ubSectorID];

  if (ubInvalidGroups || pSector->ubAdminsInBattle || pSector->ubTroopsInBattle ||
      pSector->ubElitesInBattle || pSector->ubCreaturesInBattle) {
    wchar_t str[512];
    swprintf(
        str, ARR_SIZE(str),
        L"Strategic info warning:  Sector 'in battle' counters are not clear when they should be.  "
        L"If you can provide information on how a previous battle was resolved here or nearby "
        L"patrol "
        L"(auto resolve, tactical battle, cheat keys, or retreat),"
        L"please forward that info (no data files necessary) as well as the following code (very "
        L"important):  "
        L"G(%02d:%c%d_b%d) A(%02d:%02d) T(%02d:%02d) E(%02d:%02d) C(%02d:%02d)",
        ubInvalidGroups, pLocGroup->ubSectorY + 'A' - 1, pLocGroup->ubSectorX, pLocGroup->ubSectorZ,
        pSector->ubNumAdmins, pSector->ubAdminsInBattle, pSector->ubNumTroops,
        pSector->ubTroopsInBattle, pSector->ubNumElites, pSector->ubElitesInBattle,
        pSector->ubNumCreatures, pSector->ubCreaturesInBattle);
    DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
    pSector->ubAdminsInBattle = 0;
    pSector->ubTroopsInBattle = 0;
    pSector->ubElitesInBattle = 0;
    pSector->ubCreaturesInBattle = 0;
  }
}
#endif

void InitPreBattleInterface(struct GROUP *pBattleGroup, BOOLEAN fPersistantPBI) {
  int32_t i;
  uint8_t ubGroupID = 0;
  uint8_t ubNumStationaryEnemies = 0;
  uint8_t ubNumMobileEnemies = 0;
  uint8_t ubNumMercs;
  BOOLEAN fUsePluralVersion = FALSE;
  int8_t bBestExpLevel = 0;
  BOOLEAN fRetreatAnOption = TRUE;
  SECTORINFO *pSector;

  // ARM: Feb01/98 - Cancel out of mapscreen movement plotting if PBI subscreen is coming up
  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    AbortMovementPlottingMode();
  }

  if (gfPreBattleInterfaceActive) return;

  gfPersistantPBI = fPersistantPBI;

  if (gfPersistantPBI) {
    gfBlitBattleSectorLocator = TRUE;
    gfBlinkHeader = FALSE;

#ifdef JA2BETAVERSION
    if (pBattleGroup) {
      ValidateAndCorrectInBattleCounters(pBattleGroup);
    }
#endif

    //	InitializeTacticalStatusAtBattleStart();
    // CJC, Oct 5 98: this is all we should need from InitializeTacticalStatusAtBattleStart()
    if (gubEnemyEncounterCode != BLOODCAT_AMBUSH_CODE &&
        gubEnemyEncounterCode != ENTERING_BLOODCAT_LAIR_CODE) {
      if (CheckFact(FACT_FIRST_BATTLE_FOUGHT, 0) == FALSE) {
        SetFactTrue(FACT_FIRST_BATTLE_BEING_FOUGHT);
      }
    }

// If we are currently in the AI Viewer development utility, then remove it first.  It automatically
// returns to the mapscreen upon removal, which is where we want to go.
#ifdef JA2BETAVERSION
    if (guiCurrentScreen == AIVIEWER_SCREEN) {
      gfExitViewer = TRUE;
      gpBattleGroup = pBattleGroup;
      gfEnteringMapScreen = TRUE;
      gfEnteringMapScreenToEnterPreBattleInterface = TRUE;
      gfUsePersistantPBI = TRUE;
      return;
    }
#endif

    // ATE: Added check for fPersistantPBI = TRUE if pBattleGroup == NULL
    // Searched code and saw that this condition only happens for creatures
    // fixing a bug
    // if( IsTacticalMode() && pBattleGroup )
    if (IsTacticalMode() && (pBattleGroup || fPersistantPBI)) {
      gpBattleGroup = pBattleGroup;
      gfEnteringMapScreen = TRUE;
      gfEnteringMapScreenToEnterPreBattleInterface = TRUE;
      gfUsePersistantPBI = TRUE;
      return;
    }

    if (gfTacticalTraversal && (pBattleGroup == gpTacticalTraversalGroup || gbWorldSectorZ > 0)) {
      return;
    }

    // reset the help text for mouse regions
    SetRegionFastHelpText(&gCharInfoHandRegion, L"");
    SetRegionFastHelpText(&gMapStatusBarsRegion, L"");

    gfDisplayPotentialRetreatPaths = FALSE;

    gpBattleGroup = pBattleGroup;

    // calc sector values
    if (gpBattleGroup) {
      gubPBSectorX = gpBattleGroup->ubSectorX;
      gubPBSectorY = gpBattleGroup->ubSectorY;
      gubPBSectorZ = gpBattleGroup->ubSectorZ;

      // get number of enemies thought to be here
      SectorInfo[GetSectorID8(gubPBSectorX, gubPBSectorY)].bLastKnownEnemies =
          NumEnemiesInSector(gubPBSectorX, gubPBSectorY);
      MarkForRedrawalStrategicMap();
    } else {
      gubPBSectorX = SectorID8_X(gubSectorIDOfCreatureAttack);
      gubPBSectorY = SectorID8_Y(gubSectorIDOfCreatureAttack);
      gubPBSectorZ = 0;
    }
  } else {  // calculate the non-persistant situation
    gfBlinkHeader = TRUE;

    if (HostileCiviliansPresent()) {  // There are hostile civilians, so no autoresolve allowed.
      gubExplicitEnemyEncounterCode = HOSTILE_CIVILIANS_CODE;
    } else if (HostileBloodcatsPresent()) {  // There are bloodcats in the sector, so no autoresolve
                                             // allowed
      gubExplicitEnemyEncounterCode = HOSTILE_BLOODCATS_CODE;
    } else if (gbWorldSectorZ) {  // We are underground, so no autoresolve allowed
      pSector = &SectorInfo[GetSectorID8(gubPBSectorX, gubPBSectorY)];
      if (pSector->ubCreaturesInBattle) {
        gubExplicitEnemyEncounterCode = FIGHTING_CREATURES_CODE;
      } else if (pSector->ubAdminsInBattle || pSector->ubTroopsInBattle ||
                 pSector->ubElitesInBattle) {
        gubExplicitEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
      }
    } else if (gubEnemyEncounterCode == ENTERING_ENEMY_SECTOR_CODE ||
               gubEnemyEncounterCode == ENEMY_ENCOUNTER_CODE ||
               gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
               gubEnemyEncounterCode == ENEMY_INVASION_CODE ||
               gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
               gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE ||
               gubEnemyEncounterCode == CREATURE_ATTACK_CODE) {  // use same code
      gubExplicitEnemyEncounterCode = gubEnemyEncounterCode;
    } else {
#ifdef JA2BETAVERSION
      DoScreenIndependantMessageBox(
          L"Can't determine valid reason for battle indicator.  Please try to provide information "
          L"as to when and why this indicator first appeared and send whatever files that may "
          L"help.",
          MSG_BOX_FLAG_OK, NULL);
#endif
      gfBlitBattleSectorLocator = FALSE;
      return;
    }
  }

  fMapScreenBottomDirty = TRUE;
  ChangeSelectedMapSector(gubPBSectorX, gubPBSectorY, gubPBSectorZ);
  RenderMapScreenInterfaceBottom();

  // If we are currently in tactical, then set the flag to automatically bring up the mapscreen.
  if (IsTacticalMode()) {
    gfEnteringMapScreen = TRUE;
  }

  if (!fShowTeamFlag) {
    ToggleShowTeamsMode();
  }

  // Define the blanket region to cover all of the other regions used underneath the panel.
  MSYS_DefineRegion(&PBInterfaceBlanket, 0, 0, 261, 359, MSYS_PRIORITY_HIGHEST - 5, 0, 0, 0);

  // Create the panel
  if (!AddVObject(CreateVObjectFromMLGFile(MLG_PREBATTLEPANEL), &uiInterfaceImages))
    AssertMsg(0, "Failed to load interface\\PreBattlePanel.sti");

  // Create the 3 buttons
  iPBButtonImage[0] = LoadButtonImage("INTERFACE\\PreBattleButton.sti", -1, 0, -1, 1, -1);
  if (iPBButtonImage[0] == -1) AssertMsg(0, "Failed to load interface\\PreBattleButton.sti");
  iPBButtonImage[1] = UseLoadedButtonImage(iPBButtonImage[0], -1, 0, -1, 1, -1);
  iPBButtonImage[2] = UseLoadedButtonImage(iPBButtonImage[0], -1, 0, -1, 1, -1);
  iPBButton[0] =
      QuickCreateButton(iPBButtonImage[0], 27, 54, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
                        DEFAULT_MOVE_CALLBACK, AutoResolveBattleCallback);
  iPBButton[1] =
      QuickCreateButton(iPBButtonImage[1], 98, 54, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
                        DEFAULT_MOVE_CALLBACK, GoToSectorCallback);
  iPBButton[2] =
      QuickCreateButton(iPBButtonImage[2], 169, 54, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 2,
                        DEFAULT_MOVE_CALLBACK, RetreatMercsCallback);

  SpecifyGeneralButtonTextAttributes(iPBButton[0], gpStrategicString[STR_PB_AUTORESOLVE_BTN],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SpecifyGeneralButtonTextAttributes(iPBButton[1], gpStrategicString[STR_PB_GOTOSECTOR_BTN],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SpecifyGeneralButtonTextAttributes(iPBButton[2], gpStrategicString[STR_PB_RETREATMERCS_BTN],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SpecifyButtonHilitedTextColors(iPBButton[0], FONT_WHITE, FONT_NEARBLACK);
  SpecifyButtonHilitedTextColors(iPBButton[1], FONT_WHITE, FONT_NEARBLACK);
  SpecifyButtonHilitedTextColors(iPBButton[2], FONT_WHITE, FONT_NEARBLACK);
  SpecifyButtonTextOffsets(iPBButton[0], 8, 7, TRUE);
  SpecifyButtonTextOffsets(iPBButton[1], 8, 7, TRUE);
  SpecifyButtonTextOffsets(iPBButton[2], 8, 7, TRUE);
  SpecifyButtonTextWrappedWidth(iPBButton[0], 51);
  SpecifyButtonTextWrappedWidth(iPBButton[1], 51);
  SpecifyButtonTextWrappedWidth(iPBButton[2], 51);
  SpecifyButtonTextJustification(iPBButton[0], BUTTON_TEXT_CENTER);
  SpecifyButtonTextJustification(iPBButton[1], BUTTON_TEXT_CENTER);
  SpecifyButtonTextJustification(iPBButton[2], BUTTON_TEXT_CENTER);
  AllowDisabledButtonFastHelp(iPBButton[0], TRUE);
  AllowDisabledButtonFastHelp(iPBButton[1], TRUE);
  AllowDisabledButtonFastHelp(iPBButton[2], TRUE);

  gusRetreatButtonLeft = ButtonList[iPBButton[2]]->Area.RegionTopLeftX;
  gusRetreatButtonTop = ButtonList[iPBButton[2]]->Area.RegionTopLeftY;
  gusRetreatButtonRight = ButtonList[iPBButton[2]]->Area.RegionBottomRightX;
  gusRetreatButtonBottom = ButtonList[iPBButton[2]]->Area.RegionBottomRightY;

  SetButtonCursor(iPBButtonImage[0], MSYS_NO_CURSOR);
  SetButtonCursor(iPBButtonImage[1], MSYS_NO_CURSOR);
  SetButtonCursor(iPBButtonImage[2], MSYS_NO_CURSOR);

  HideButton(iPBButton[0]);
  HideButton(iPBButton[1]);
  HideButton(iPBButton[2]);
  gfPBButtonsHidden = TRUE;

  // ARM: this must now be set before any calls utilizing the GetCurrentBattleSectorXYZ() function
  gfPreBattleInterfaceActive = TRUE;

  CheckForRobotAndIfItsControlled();

  // wake everyone up
  WakeUpAllMercsInSectorUnderAttack();

  // Count the number of players involved or not involved in this battle
  guiNumUninvolved = 0;
  guiNumInvolved = 0;
  for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
       i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife &&
        !(MercPtrs[i]->uiStatusFlags & SOLDIER_VEHICLE)) {
      if (PlayerMercInvolvedInThisCombat(MercPtrs[i])) {
        // involved
        if (!ubGroupID) {  // Record the first groupID.  If there are more than one group in this
                           // battle, we
          // can detect it by comparing the first value with future values.  If we do, then
          // we set a flag which determines whether to use the singular help text or plural version
          // for the retreat button.
          ubGroupID = MercPtrs[i]->ubGroupID;
          if (!gpBattleGroup) gpBattleGroup = GetGroup(ubGroupID);
          if (bBestExpLevel > MercPtrs[i]->bExpLevel) bBestExpLevel = MercPtrs[i]->bExpLevel;
          if (MercPtrs[i]->ubPrevSectorID == 255) {  // Not able to retreat (calculate it for group)
            struct GROUP *pTempGroup;
            pTempGroup = GetGroup(ubGroupID);
            Assert(pTempGroup);
            CalculateGroupRetreatSector(pTempGroup);
          }
        } else if (ubGroupID != MercPtrs[i]->ubGroupID) {
          fUsePluralVersion = TRUE;
        }
        guiNumInvolved++;
      } else
        guiNumUninvolved++;
    }
  }

  ubNumStationaryEnemies = NumStationaryEnemiesInSector(gubPBSectorX, gubPBSectorY);
  ubNumMobileEnemies = NumMobileEnemiesInSector(gubPBSectorX, gubPBSectorY);
  ubNumMercs = PlayerMercsInSector(gubPBSectorX, gubPBSectorY, gubPBSectorZ);

  if (gfPersistantPBI) {
    if (!pBattleGroup) {  // creature's attacking!
      gubEnemyEncounterCode = CREATURE_ATTACK_CODE;
    } else if (gpBattleGroup->fPlayer) {
      if (gubEnemyEncounterCode != BLOODCAT_AMBUSH_CODE &&
          gubEnemyEncounterCode != ENTERING_BLOODCAT_LAIR_CODE) {
        if (ubNumStationaryEnemies) {
          gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
        } else {
          gubEnemyEncounterCode = ENEMY_ENCOUNTER_CODE;

          // Don't consider ambushes until the player has reached 25% (normal) progress
          if (gfHighPotentialForAmbush) {
            if (Chance(90)) {
              gubEnemyEncounterCode = ENEMY_AMBUSH_CODE;
            }
          } else if (gfAutoAmbush && ubNumMobileEnemies > ubNumMercs) {
            gubEnemyEncounterCode = ENEMY_AMBUSH_CODE;
          } else if (WhatPlayerKnowsAboutEnemiesInSector(gubPBSectorX, gubPBSectorY) ==
                         KNOWS_NOTHING &&
                     CurrentPlayerProgressPercentage() >=
                         30 - gGameOptions.ubDifficultyLevel *
                                  5) {  // if the enemy outnumbers the players, then there is a
                                        // small chance of the enemies ambushing the group
            if (ubNumMobileEnemies > ubNumMercs) {
              int32_t iChance;
              pSector = &SectorInfo[GetSectorID8(gubPBSectorX, gubPBSectorY)];
              if (!(pSector->uiFlags & SF_ALREADY_VISITED)) {
                iChance = (uint8_t)(4 - bBestExpLevel + 2 * gGameOptions.ubDifficultyLevel +
                                    CurrentPlayerProgressPercentage() / 10);
                if (pSector->uiFlags & SF_ENEMY_AMBUSH_LOCATION) {
                  iChance += 20;
                }
                if (gfCantRetreatInPBI) {
                  iChance += 20;
                }
                if ((int32_t)PreRandom(100) < iChance) {
                  gubEnemyEncounterCode = ENEMY_AMBUSH_CODE;
                }
              }
            }
          }
        }
      }
    } else {  // Are enemies invading a town, or just encountered the player.
      if (GetTownIdForSector(gubPBSectorX, gubPBSectorY)) {
        gubEnemyEncounterCode = ENEMY_INVASION_CODE;
      } else {
        switch (GetSectorID8(gubPBSectorX, gubPBSectorY)) {
          case SEC_D2:
          case SEC_D15:
          case SEC_G8:
            // SAM sites not in towns will also be considered to be important
            gubEnemyEncounterCode = ENEMY_INVASION_CODE;
            break;
          default:
            gubEnemyEncounterCode = ENEMY_ENCOUNTER_CODE;
            break;
        }
      }
    }
  }

  gfHighPotentialForAmbush = FALSE;

  if (gfAutomaticallyStartAutoResolve) {
    DisableButton(iPBButton[1]);
    DisableButton(iPBButton[2]);
  }

  gfRenderPBInterface = TRUE;
  giHilitedInvolved = giHilitedUninvolved = -1;
  MSYS_SetCurrentCursor(CURSOR_NORMAL);
  StopTimeCompression();

  // hide all visible boxes
  HideAllBoxes();
  fShowAssignmentMenu = FALSE;
  fShowContractMenu = FALSE;
  DisableTeamInfoPanels();
  if (ButtonList[giMapContractButton]) {
    HideButton(giMapContractButton);
  }
  if (ButtonList[giCharInfoButton[0]]) {
    HideButton(giCharInfoButton[0]);
  }
  if (ButtonList[giCharInfoButton[1]]) {
    HideButton(giCharInfoButton[1]);
  }
  HideButton(giMapContractButton);

  if (gubEnemyEncounterCode ==
      ENEMY_ENCOUNTER_CODE) {  // we know how many enemies are here, so until we leave the sector,
                               // we will continue to display the value.
    // the flag will get cleared when time advances after the fEnemyInSector flag is clear.
    pSector = &SectorInfo[GetSectorID8(gubPBSectorX, gubPBSectorY)];

    // ALWAYS use these 2 statements together, without setting the boolean, the flag will never be
    // cleaned up!
    pSector->uiFlags |= SF_PLAYER_KNOWS_ENEMIES_ARE_HERE;
    gfResetAllPlayerKnowsEnemiesFlags = TRUE;
  }

  // Set up fast help for buttons depending on the state of the button, and disable buttons
  // when necessary.
  if (gfPersistantPBI) {
    if (gubEnemyEncounterCode == ENTERING_ENEMY_SECTOR_CODE ||
        gubEnemyEncounterCode ==
            ENTERING_BLOODCAT_LAIR_CODE) {  // Don't allow autoresolve for player initiated invasion
                                            // battle types
      DisableButton(iPBButton[0]);
      SetButtonFastHelpText(iPBButton[0], gpStrategicString[STR_PB_DISABLED_AUTORESOLVE_FASTHELP]);
    } else if (gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
               gubEnemyEncounterCode ==
                   BLOODCAT_AMBUSH_CODE) {  // Don't allow autoresolve for ambushes
      DisableButton(iPBButton[0]);
      SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[3]);
    } else {
      SetButtonFastHelpText(iPBButton[0], gpStrategicString[STR_PB_AUTORESOLVE_FASTHELP]);
    }
    SetButtonFastHelpText(iPBButton[1], gpStrategicString[STR_PB_GOTOSECTOR_FASTHELP]);
    if (gfAutomaticallyStartAutoResolve) {
      DisableButton(iPBButton[1]);
    }
    if (gfCantRetreatInPBI) {
      gfCantRetreatInPBI = FALSE;
      fRetreatAnOption = FALSE;
    }
    if (gfAutomaticallyStartAutoResolve || !fRetreatAnOption ||
        gubEnemyEncounterCode == ENEMY_AMBUSH_CODE ||
        gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
        gubEnemyEncounterCode == CREATURE_ATTACK_CODE) {
      DisableButton(iPBButton[2]);
      SetButtonFastHelpText(iPBButton[2], gzNonPersistantPBIText[9]);
    } else {
      if (!fUsePluralVersion) {
        SetButtonFastHelpText(iPBButton[2], gpStrategicString[STR_BP_RETREATSINGLE_FASTHELP]);
      } else {
        SetButtonFastHelpText(iPBButton[2], gpStrategicString[STR_BP_RETREATPLURAL_FASTHELP]);
      }
    }
  } else {  // use the explicit encounter code to determine what get's disable and the associated
            // help text that is used.

    // First of all, the retreat button is always disabled seeing a battle is in progress.
    DisableButton(iPBButton[2]);
    SetButtonFastHelpText(iPBButton[2], gzNonPersistantPBIText[0]);
    SetButtonFastHelpText(iPBButton[1], gzNonPersistantPBIText[1]);
    switch (gubExplicitEnemyEncounterCode) {
      case CREATURE_ATTACK_CODE:
      case ENEMY_ENCOUNTER_CODE:
      case ENEMY_INVASION_CODE:
        SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[2]);
        break;
      case ENTERING_ENEMY_SECTOR_CODE:
        DisableButton(iPBButton[0]);
        SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[3]);
        break;
      case ENEMY_AMBUSH_CODE:
        DisableButton(iPBButton[0]);
        SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[4]);
        break;
      case FIGHTING_CREATURES_CODE:
        DisableButton(iPBButton[0]);
        SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[5]);
        break;
      case HOSTILE_CIVILIANS_CODE:
        DisableButton(iPBButton[0]);
        SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[6]);
        break;
      case HOSTILE_BLOODCATS_CODE:
      case BLOODCAT_AMBUSH_CODE:
      case ENTERING_BLOODCAT_LAIR_CODE:
        DisableButton(iPBButton[0]);
        SetButtonFastHelpText(iPBButton[0], gzNonPersistantPBIText[7]);
        break;
    }
  }

  // Disable the options button when the auto resolve  screen comes up
  EnableDisAbleMapScreenOptionsButton(FALSE);

  SetMusicMode(MUSIC_TACTICAL_ENEMYPRESENT);

  DoTransitionFromMapscreenToPreBattleInterface();
}

void DoTransitionFromMapscreenToPreBattleInterface() {
  SGPRect DstRect, PBIRect;
  uint32_t uiStartTime, uiCurrTime;
  int32_t iPercentage, iFactor;
  uint32_t uiTimeRange;
  int16_t sStartLeft, sEndLeft, sStartTop, sEndTop;
  int32_t iLeft, iTop, iWidth, iHeight;
  BOOLEAN fEnterAutoResolveMode = FALSE;

  if (!gfExtraBuffer) return;

  PauseTime(FALSE);

  PBIRect.iLeft = 0;
  PBIRect.iTop = 0;
  PBIRect.iRight = 261;
  PBIRect.iBottom = 359;
  iWidth = 261;
  iHeight = 359;

  uiTimeRange = 1000;
  iPercentage = 0;
  uiStartTime = GetJA2Clock();

  GetScreenXYFromMapXY(gubPBSectorX, gubPBSectorY, &sStartLeft, &sStartTop);
  sStartLeft += MAP_GRID_X / 2;
  sStartTop += MAP_GRID_Y / 2;
  sEndLeft = 131;
  sEndTop = 180;

  // save the mapscreen buffer
  BlitBufferToBuffer(vsFB, vsExtraBuffer, 0, 0, 640, 480);

  if (gfEnterAutoResolveMode) {  // If we are intending on immediately entering autoresolve, change
                                 // the global flag so that it will actually
    // render the interface once.  If gfEnterAutoResolveMode is clear, then
    // RenderPreBattleInterface() won't do anything.
    fEnterAutoResolveMode = TRUE;
    gfEnterAutoResolveMode = FALSE;
  }
  // render the prebattle interface
  RenderPreBattleInterface();

  gfIgnoreAllInput = TRUE;

  if (fEnterAutoResolveMode) {  // Change it back
    gfEnterAutoResolveMode = TRUE;
  }

  BlitBufferToBuffer(vsSaveBuffer, vsFB, 27, 54, 209, 32);
  RenderButtons();
  BlitBufferToBuffer(vsFB, vsSaveBuffer, 27, 54, 209, 32);
  gfRenderPBInterface = TRUE;

  // hide the prebattle interface
  BlitBufferToBuffer(vsExtraBuffer, vsFB, 0, 0, 261, 359);
  PlayJA2SampleFromFile("SOUNDS\\Laptop power up (8-11).wav", RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
  InvalidateScreen();
  RefreshScreen(NULL);

  while (iPercentage < 100) {
    uiCurrTime = GetJA2Clock();
    iPercentage = (uiCurrTime - uiStartTime) * 100 / uiTimeRange;
    iPercentage = min(iPercentage, 100);

    // Factor the percentage so that it is modified by a gravity falling acceleration effect.
    iFactor = (iPercentage - 50) * 2;
    if (iPercentage < 50)
      iPercentage = (uint32_t)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
    else
      iPercentage = (uint32_t)(iPercentage + (100 - iPercentage) * iFactor * 0.01 + 0.05);

    // Calculate the center point.
    iLeft = sStartLeft - (sStartLeft - sEndLeft + 1) * iPercentage / 100;
    if (sStartTop > sEndTop)
      iTop = sStartTop - (sStartTop - sEndTop + 1) * iPercentage / 100;
    else
      iTop = sStartTop + (sEndTop - sStartTop + 1) * iPercentage / 100;

    DstRect.iLeft = iLeft - iWidth * iPercentage / 200;
    DstRect.iRight = DstRect.iLeft + max(iWidth * iPercentage / 100, 1);
    DstRect.iTop = iTop - iHeight * iPercentage / 200;
    DstRect.iBottom = DstRect.iTop + max(iHeight * iPercentage / 100, 1);

    BltStretchVSurface(vsFB, vsSaveBuffer, &PBIRect, &DstRect);

    InvalidateScreen();
    RefreshScreen(NULL);

    // Restore the previous rect.
    BlitBufferToBuffer(vsExtraBuffer, vsFB, (uint16_t)DstRect.iLeft, (uint16_t)DstRect.iTop,
                       (uint16_t)(DstRect.iRight - DstRect.iLeft + 1),
                       (uint16_t)(DstRect.iBottom - DstRect.iTop + 1));
  }
  BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 0, 640, 480);
}

void KillPreBattleInterface() {
  if (!gfPreBattleInterfaceActive) return;

  fDisableMapInterfaceDueToBattle = FALSE;
  MSYS_RemoveRegion(&PBInterfaceBlanket);

  // The panel
  DeleteVideoObjectFromIndex(uiInterfaceImages);

  // The 3 buttons
  RemoveButton(iPBButton[0]);
  RemoveButton(iPBButton[1]);
  RemoveButton(iPBButton[2]);
  UnloadButtonImage(iPBButtonImage[0]);
  UnloadButtonImage(iPBButtonImage[1]);
  UnloadButtonImage(iPBButtonImage[2]);

  /*
  MSYS_RemoveRegion( &InvolvedRegion );
  if( guiNumUninvolved )
          MSYS_RemoveRegion( &UninvolvedRegion );
  */

  gfPreBattleInterfaceActive = FALSE;

  // UpdateCharRegionHelpText( );

  // re draw affected regions
  MarkForRedrawalStrategicMap();
  fTeamPanelDirty = TRUE;
  fMapScreenBottomDirty = TRUE;
  fCharacterInfoPanelDirty = TRUE;
  gfDisplayPotentialRetreatPaths = FALSE;

  // Enable the options button when the auto resolve  screen comes up
  EnableDisAbleMapScreenOptionsButton(TRUE);

  ColorFillVSurfaceArea(vsSaveBuffer, 0, 0, 261, 359, 0);

  EnableTeamInfoPanels();
  if (ButtonList[giMapContractButton]) {
    ShowButton(giMapContractButton);
  }
  if (ButtonList[giCharInfoButton[0]]) {
    ShowButton(giCharInfoButton[0]);
  }
  if (ButtonList[giCharInfoButton[1]]) {
    ShowButton(giCharInfoButton[1]);
  }
}

void RenderPBHeader(int32_t *piX, int32_t *piWidth) {
  wchar_t str[100];
  int32_t x, width;
  SetFont(FONT10ARIALBOLD);
  if (gfBlinkHeader) {
    if (GetJA2Clock() % 1000 < 667) {
      SetFontForeground(FONT_WHITE);
    } else {
      SetFontForeground(FONT_LTRED);
    }
  } else {
    SetFontForeground(FONT_BEIGE);
  }
  SetFontShadow(FONT_NEARBLACK);
  if (!gfPersistantPBI) {
    swprintf(str, ARR_SIZE(str), gzNonPersistantPBIText[8]);
  } else
    switch (gubEnemyEncounterCode) {
      case ENEMY_INVASION_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_ENEMYINVASION_HEADER]);
        break;
      case ENEMY_ENCOUNTER_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_ENEMYENCOUNTER_HEADER]);
        break;
      case ENEMY_AMBUSH_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_ENEMYAMBUSH_HEADER]);
        gfBlinkHeader = TRUE;
        break;
      case ENTERING_ENEMY_SECTOR_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_ENTERINGENEMYSECTOR_HEADER]);
        break;
      case CREATURE_ATTACK_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_CREATUREATTACK_HEADER]);
        gfBlinkHeader = TRUE;
        break;
      case BLOODCAT_AMBUSH_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_BLOODCATAMBUSH_HEADER]);
        gfBlinkHeader = TRUE;
        break;
      case ENTERING_BLOODCAT_LAIR_CODE:
        swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_ENTERINGBLOODCATLAIR_HEADER]);
        break;
    }
  width = StringPixLength(str, FONT10ARIALBOLD);
  x = 130 - width / 2;
  mprintf(x, 4, str);
  InvalidateRegion(0, 0, 231, 12);
  *piX = x;
  *piWidth = width;
}

void RenderPreBattleInterface() {
  struct VObject *hVObject;
  int32_t i, x, y, line, width;
  wchar_t str[100];
  wchar_t pSectorName[128];
  uint8_t ubHPPercent, ubBPPercent;
  BOOLEAN fMouseInRetreatButtonArea;
  uint8_t ubJunk;
  // PLAYERGROUP *pPlayer;

  // This code determines if the cursor is inside the rectangle consisting of the
  // retreat button.  If it is inside, then we set up the variables so that the retreat
  // arrows get drawn in the mapscreen.
  if (ButtonList[iPBButton[2]]->uiFlags & BUTTON_ENABLED) {
    if (gusMouseXPos < gusRetreatButtonLeft || gusMouseXPos > gusRetreatButtonRight ||
        gusMouseYPos < gusRetreatButtonTop || gusMouseYPos > gusRetreatButtonBottom)
      fMouseInRetreatButtonArea = FALSE;
    else
      fMouseInRetreatButtonArea = TRUE;
    if (fMouseInRetreatButtonArea != gfDisplayPotentialRetreatPaths) {
      gfDisplayPotentialRetreatPaths = fMouseInRetreatButtonArea;
      MarkForRedrawalStrategicMap();
    }
  }

  if (gfRenderPBInterface) {
    // set font destinanation buffer to the save buffer
    SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

    if (gfPBButtonsHidden) {
      ShowButton(iPBButton[0]);
      ShowButton(iPBButton[1]);
      ShowButton(iPBButton[2]);
      gfPBButtonsHidden = FALSE;
    } else {
      MarkAButtonDirty(iPBButton[0]);
      MarkAButtonDirty(iPBButton[1]);
      MarkAButtonDirty(iPBButton[2]);
    }

    gfRenderPBInterface = FALSE;
    GetVideoObject(&hVObject, uiInterfaceImages);
    // main panel
    BltVideoObject(vsSaveBuffer, hVObject, MAINPANEL, 0, 0);
    // main title

    RenderPBHeader(&x, &width);
    // now draw the title bars up to the text.
    for (i = x - 12; i > 20; i -= 10) {
      BltVideoObject(vsSaveBuffer, hVObject, TITLE_BAR_PIECE, i, 6);
    }
    for (i = x + width + 2; i < 231; i += 10) {
      BltVideoObject(vsSaveBuffer, hVObject, TITLE_BAR_PIECE, i, 6);
    }

    y = BOTTOM_Y - ACTUAL_HEIGHT - ROW_HEIGHT * max(guiNumUninvolved, 1);
    BltVideoObject(vsSaveBuffer, hVObject, UNINVOLVED_HEADER, 8, y);

    SetFont(BLOCKFONT);
    SetFontForeground(FONT_BEIGE);
    swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_LOCATION]);
    width = StringPixLength(str, BLOCKFONT);
    if (width > 64) {
      SetFont(BLOCKFONTNARROW);
      width = StringPixLength(str, BLOCKFONTNARROW);
    }
    mprintf(65 - width, 17, str);

    SetFont(BLOCKFONT);
    if (gubEnemyEncounterCode != CREATURE_ATTACK_CODE) {
      swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_ENEMIES]);
    } else if (gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
               gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE) {
      swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_BLOODCATS]);
    } else {
      swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_CREATURES]);
    }
    width = StringPixLength(str, BLOCKFONT);
    if (width > 52) {
      SetFont(BLOCKFONTNARROW);
      width = StringPixLength(str, BLOCKFONTNARROW);
    }
    mprintf(54 - width, 38, str);

    SetFont(BLOCKFONT);
    swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_MERCS]);
    width = StringPixLength(str, BLOCKFONT);
    if (width > 52) {
      SetFont(BLOCKFONTNARROW);
      width = StringPixLength(str, BLOCKFONTNARROW);
    }
    mprintf(139 - width, 38, str);

    SetFont(BLOCKFONT);
    swprintf(str, ARR_SIZE(str), gpStrategicString[STR_PB_MILITIA]);
    width = StringPixLength(str, BLOCKFONT);
    if (width > 52) {
      SetFont(BLOCKFONTNARROW);
      width = StringPixLength(str, BLOCKFONTNARROW);
    }
    mprintf(224 - width, 38, str);

    // Draw the bottom columns
    for (i = 0; i < (int32_t)max(guiNumUninvolved, 1); i++) {
      y = BOTTOM_Y - ROW_HEIGHT * (i + 1) + 1;
      BltVideoObject(vsSaveBuffer, hVObject, BOTTOM_COLUMN, 161, y);
    }

    for (i = 0; i < (int32_t)(21 - max(guiNumUninvolved, 1)); i++) {
      y = TOP_Y + ROW_HEIGHT * i;
      BltVideoObject(vsSaveBuffer, hVObject, TOP_COLUMN, 186, y);
    }

    // location
    SetFont(FONT10ARIAL);
    SetFontForeground(FONT_YELLOW);
    SetFontShadow(FONT_NEARBLACK);

    GetSectorIDString(gubPBSectorX, gubPBSectorY, gubPBSectorZ, pSectorName, ARR_SIZE(pSectorName),
                      TRUE);
    mprintf(70, 17, L"%s %s", gpStrategicString[STR_PB_SECTOR], pSectorName);

    // enemy
    SetFont(FONT14ARIAL);
    if (gubEnemyEncounterCode == CREATURE_ATTACK_CODE ||
        gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE ||
        gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE ||
        WhatPlayerKnowsAboutEnemiesInSector(gubPBSectorX, gubPBSectorY) != KNOWS_HOW_MANY) {
      // don't know how many
      swprintf(str, ARR_SIZE(str), L"?");
      SectorInfo[GetSectorID8(gubPBSectorX, gubPBSectorY)].bLastKnownEnemies = -2;
    } else {
      // know exactly how many
      i = NumEnemiesInSector(gubPBSectorX, gubPBSectorY);
      swprintf(str, ARR_SIZE(str), L"%d", i);
      SectorInfo[GetSectorID8(gubPBSectorX, gubPBSectorY)].bLastKnownEnemies = (int8_t)i;
    }
    x = 57 + (27 - StringPixLength(str, FONT14ARIAL)) / 2;
    y = 36;
    mprintf(x, y, str);
    // player
    swprintf(str, ARR_SIZE(str), L"%d", guiNumInvolved);
    x = 142 + (27 - StringPixLength(str, FONT14ARIAL)) / 2;
    mprintf(x, y, str);
    // militia
    swprintf(str, ARR_SIZE(str), L"%d", CountAllMilitiaInSector(gubPBSectorX, gubPBSectorY));
    x = 227 + (27 - StringPixLength(str, FONT14ARIAL)) / 2;
    mprintf(x, y, str);
    SetFontShadow(FONT_NEARBLACK);

    SetFont(BLOCKFONT2);
    SetFontForeground(FONT_YELLOW);

    // print out the participants of the battle.
    // |  NAME  | ASSIGN |  COND  |   HP   |   BP   |
    line = 0;
    y = TOP_Y + 1;
    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {
      if (MercPtrs[i]->bActive && MercPtrs[i]->bLife &&
          !(MercPtrs[i]->uiStatusFlags & SOLDIER_VEHICLE)) {
        if (PlayerMercInvolvedInThisCombat(MercPtrs[i])) {  // involved
          if (line == giHilitedInvolved)
            SetFontForeground(FONT_WHITE);
          else
            SetFontForeground(FONT_YELLOW);
          // NAME
          wcscpy(str, MercPtrs[i]->name);
          x = 17 + (52 - StringPixLength(str, BLOCKFONT2)) / 2;
          mprintf(x, y, str);
          // ASSIGN
          GetMapscreenMercAssignmentString(MercPtrs[i], str);
          x = 72 + (54 - StringPixLength(str, BLOCKFONT2)) / 2;
          mprintf(x, y, str);
          // COND
          GetSoldierConditionInfo(MercPtrs[i], str, ARR_SIZE(str), &ubHPPercent, &ubBPPercent);
          x = 129 + (58 - StringPixLength(str, BLOCKFONT2)) / 2;
          mprintf(x, y, str);
          // HP
          swprintf(str, ARR_SIZE(str), L"%d%%", ubHPPercent);
          x = 189 + (25 - StringPixLength(str, BLOCKFONT2)) / 2;
          wcscat(str, L"%");
          mprintf(x, y, str);
          // BP
          swprintf(str, ARR_SIZE(str), L"%d%%", ubBPPercent);
          x = 217 + (25 - StringPixLength(str, BLOCKFONT2)) / 2;
          wcscat(str, L"%");
          mprintf(x, y, str);

          line++;
          y += ROW_HEIGHT;
        }
      }
    }

    // print out the uninvolved members of the battle
    // |  NAME  | ASSIGN |  LOC   |  DEST  |  DEP   |
    if (!guiNumUninvolved) {
      SetFontForeground(FONT_YELLOW);
      wcscpy(str, gpStrategicString[STR_PB_NONE]);
      x = 17 + (52 - StringPixLength(str, BLOCKFONT2)) / 2;
      y = BOTTOM_Y - ROW_HEIGHT + 2;
      mprintf(x, y, str);
    } else {
      y = BOTTOM_Y - ROW_HEIGHT * guiNumUninvolved + 2;
      for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
           i++) {
        if (MercPtrs[i]->bActive && MercPtrs[i]->bLife &&
            !(MercPtrs[i]->uiStatusFlags & SOLDIER_VEHICLE)) {
          if (!PlayerMercInvolvedInThisCombat(MercPtrs[i])) {
            // uninvolved
            if (line == giHilitedUninvolved)
              SetFontForeground(FONT_WHITE);
            else
              SetFontForeground(FONT_YELLOW);
            // NAME
            wcscpy(str, MercPtrs[i]->name);
            x = 17 + (52 - StringPixLength(str, BLOCKFONT2)) / 2;
            mprintf(x, y, str);
            // ASSIGN
            GetMapscreenMercAssignmentString(MercPtrs[i], str);
            x = 72 + (54 - StringPixLength(str, BLOCKFONT2)) / 2;
            mprintf(x, y, str);
            // LOC
            GetMapscreenMercLocationString(MercPtrs[i], str, ARR_SIZE(str));
            x = 128 + (33 - StringPixLength(str, BLOCKFONT2)) / 2;
            mprintf(x, y, str);
            // DEST
            GetMapscreenMercDestinationString(MercPtrs[i], str, ARR_SIZE(str));
            if (wcslen(str) > 0) {
              x = 164 + (41 - StringPixLength(str, BLOCKFONT2)) / 2;
              mprintf(x, y, str);
            }
            // DEP
            GetMapscreenMercDepartureString(MercPtrs[i], str, ARR_SIZE(str), &ubJunk);
            x = 208 + (34 - StringPixLength(str, BLOCKFONT2)) / 2;
            mprintf(x, y, str);
            line++;
            y += ROW_HEIGHT;
          }
        }
      }
    }

    // mark any and ALL pop up boxes as altered
    MarkAllBoxesAsAltered();
    RestoreExternBackgroundRect(0, 0, 261, 359);

    // restore font destinanation buffer to the frame buffer
    SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
  } else if (gfBlinkHeader) {
    RenderPBHeader(&x, &width);  // the text is important enough to blink.
  }

  // InvalidateRegion( 0, 0, 261, 359 );
  if (gfEnterAutoResolveMode) {
    gfEnterAutoResolveMode = FALSE;
    EnterAutoResolveMode(gubPBSectorX, gubPBSectorY);
    // return;
  }

  gfIgnoreAllInput = FALSE;
}

void AutoResolveBattleCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!gfIgnoreAllInput) {
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
#ifdef JA2TESTVERSION
      if (_KeyDown(ALT))
#else
      if (_KeyDown(ALT) && CHEATER_CHEAT_LEVEL())
#endif
      {
        if (!gfPersistantPBI) {
          return;
        }
        PlayJA2Sample(EXPLOSION_1, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
        gStrategicStatus.usPlayerKills += NumEnemiesInSector(gubPBSectorX, gubPBSectorY);
        EliminateAllEnemies(gubPBSectorX, gubPBSectorY);
        SetMusicMode(MUSIC_TACTICAL_VICTORY);
        btn->uiFlags &= ~BUTTON_CLICKED_ON;
        DrawButton(btn->IDNum);
        InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                         btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
        ExecuteBaseDirtyRectQueue();
        EndFrameBufferRender();
        RefreshScreen(NULL);
        KillPreBattleInterface();
        StopTimeCompression();
        SetMusicMode(MUSIC_TACTICAL_NOTHING);
        return;
      }
      gfEnterAutoResolveMode = TRUE;
    }
  }
}

void GoToSectorCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!gfIgnoreAllInput) {
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
#ifdef JA2TESTVERSION
      if (_KeyDown(ALT))
#else
      if (_KeyDown(ALT) && CHEATER_CHEAT_LEVEL())
#endif
      {
        if (!gfPersistantPBI) {
          return;
        }
        PlayJA2Sample(EXPLOSION_1, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
        gStrategicStatus.usPlayerKills += NumEnemiesInSector(gubPBSectorX, gubPBSectorY);
        EliminateAllEnemies(gubPBSectorX, gubPBSectorY);
        SetMusicMode(MUSIC_TACTICAL_VICTORY);
        btn->uiFlags &= ~BUTTON_CLICKED_ON;
        DrawButton(btn->IDNum);
        InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                         btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
        ExecuteBaseDirtyRectQueue();
        EndFrameBufferRender();
        RefreshScreen(NULL);
        KillPreBattleInterface();
        StopTimeCompression();
        SetMusicMode(MUSIC_TACTICAL_NOTHING);
        return;
      }
      if (gfPersistantPBI && gpBattleGroup && gpBattleGroup->fPlayer &&
          gubEnemyEncounterCode != ENEMY_AMBUSH_CODE &&
          gubEnemyEncounterCode != CREATURE_ATTACK_CODE &&
          gubEnemyEncounterCode != BLOODCAT_AMBUSH_CODE) {
        gfEnterTacticalPlacementGUI = TRUE;
      }
      btn->uiFlags &= ~BUTTON_CLICKED_ON;
      DrawButton(btn->IDNum);
      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
      ExecuteBaseDirtyRectQueue();
      EndFrameBufferRender();
      RefreshScreen(NULL);
      if (gubPBSectorX == gWorldSectorX && gubPBSectorY == gWorldSectorY && !gbWorldSectorZ) {
        gfGotoSectorTransition = TRUE;
      }

      // first time going to the sector?
      if (gfPersistantPBI) {
        // put everyone on duty, and remove mercs from vehicles, too
        PutNonSquadMercsInBattleSectorOnSquads(TRUE);

        // we nuke the groups existing route & destination in advance
        ClearMovementForAllInvolvedPlayerGroups();
      } else {  // Clear the battlegroup pointer.
        gpBattleGroup = NULL;
      }

      // must come AFTER anything that needs gpBattleGroup, as it wipes it out
      SetCurrentWorldSector(gubPBSectorX, gubPBSectorY, gubPBSectorZ);

      KillPreBattleInterface();
      SetTacticalInterfaceFlags(0);
    }
  }
}

void RetreatMercsCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!gfIgnoreAllInput) {
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      // get them outta here!
      RetreatAllInvolvedPlayerGroups();

      // NOTE: this code assumes you can never retreat while underground
      HandleLoyaltyImplicationsOfMercRetreat(RETREAT_PBI, gubPBSectorX, gubPBSectorY, 0);
      if (CountAllMilitiaInSector(
              gubPBSectorX,
              gubPBSectorY)) {  // Mercs retreat, but enemies still need to fight the militia
        gfEnterAutoResolveMode = TRUE;
        return;
      }

      // Warp time by 5 minutes so that player can't just go back into the sector he left.
      WarpGameTime(300, WARPTIME_NO_PROCESSING_OF_EVENTS);
      ResetMovementForEnemyGroupsInLocation(gubPBSectorX, gubPBSectorY);

      btn->uiFlags &= ~BUTTON_CLICKED_ON;
      DrawButton(btn->IDNum);
      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
      ExecuteBaseDirtyRectQueue();
      EndFrameBufferRender();
      RefreshScreen(NULL);
      KillPreBattleInterface();
      StopTimeCompression();
      gpBattleGroup = NULL;
      gfBlitBattleSectorLocator = FALSE;

      SetMusicMode(MUSIC_TACTICAL_NOTHING);
    }
  }
}

enum {
  COND_EXCELLENT,
  COND_GOOD,
  COND_FAIR,
  COND_WOUNDED,
  COND_FATIGUED,
  COND_BLEEDING,
  COND_UNCONCIOUS,
  COND_DYING,
  COND_DEAD
};

void GetSoldierConditionInfo(struct SOLDIERTYPE *pSoldier, wchar_t *szCondition,
                             int szConditionSize, uint8_t *pubHPPercent, uint8_t *pubBPPercent) {
  Assert(pSoldier);
  *pubHPPercent = (uint8_t)(pSoldier->bLife * 100 / pSoldier->bLifeMax);
  *pubBPPercent = pSoldier->bBreath;
  // Go from the worst condition to the best.
  if (!pSoldier->bLife) {  // 0 life
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_DEAD]);
  } else if (pSoldier->bLife < OKLIFE &&
             pSoldier->bBleeding) {  // life less than OKLIFE and bleeding
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_DYING]);
  } else if (pSoldier->bBreath < OKBREATH && pSoldier->bCollapsed) {  // breath less than OKBREATH
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_UNCONCIOUS]);
  } else if (pSoldier->bBleeding > MIN_BLEEDING_THRESHOLD) {  // bleeding
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_BLEEDING]);
  } else if (pSoldier->bLife * 100 < pSoldier->bLifeMax * 50) {  // less than 50% life
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_WOUNDED]);
  } else if (pSoldier->bBreath < 50) {  // breath less than half
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_FATIGUED]);
  } else if (pSoldier->bLife * 100 < pSoldier->bLifeMax * 67) {  // less than 67% life
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_FAIR]);
  } else if (pSoldier->bLife * 100 < pSoldier->bLifeMax * 86) {  // less than 86% life
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_GOOD]);
  } else {  // 86%+ life
    swprintf(szCondition, szConditionSize, pConditionStrings[COND_EXCELLENT]);
  }
}

/*
void InvolvedMoveCallback( struct MOUSE_REGION *reg, int32_t reason )
{
        gfRenderPBInterface = TRUE;
        if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
        {
                giHilitedInvolved = giHilitedUninvolved = -1;
                return;
        }
        giHilitedInvolved = reg->RelativeYPos / 10;
        giHilitedUninvolved = -1;
}

void InvolvedClickCallback( struct MOUSE_REGION *reg, int32_t reason )
{
        if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                struct SOLDIERTYPE *pSoldier;
                int16_t y;
                pSoldier = InvolvedSoldier( giHilitedInvolved );
                if( !pSoldier )
                        return;
                y = (int16_t)(reg->RegionTopLeftY + giHilitedUninvolved * ROW_HEIGHT + 5);
                if( y + 102 >= 360 )
                        y -= 102;
                if( gusMouseXPos >= 76 && gusMouseXPos <= 129 )
                        ActivateSoldierPopup( pSoldier, ASSIGNMENT_POPUP, 102, y );
                gfRenderPBInterface = TRUE;
        }
}

void UninvolvedMoveCallback( struct MOUSE_REGION *reg, int32_t reason )
{
        gfRenderPBInterface = TRUE;
        if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
        {
                giHilitedInvolved = giHilitedUninvolved = -1;
                return;
        }
        giHilitedUninvolved = reg->RelativeYPos / 10;
        giHilitedInvolved = -1;
}

void UninvolvedClickCallback( struct MOUSE_REGION *reg, int32_t reason )
{
        if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                struct SOLDIERTYPE *pSoldier;
                int16_t y;
                pSoldier = UninvolvedSoldier( giHilitedUninvolved );
                if( !pSoldier )
                        return;
                y = (int16_t)(reg->RegionTopLeftY + giHilitedUninvolved * ROW_HEIGHT + 5);
                if( y + 102 >= 360 )
                        y -= 102;
                if( gusMouseXPos >= 76 && gusMouseXPos <= 129 )
                {
                        ActivateSoldierPopup( pSoldier, ASSIGNMENT_POPUP, 102, y );
                }
                else if( gusMouseXPos >= 169 && gusMouseXPos <= 204 )
                {
                        ActivateSoldierPopup( pSoldier, DESTINATION_POPUP, 186, y );
                }
                else if( gusMouseXPos >= 208 && gusMouseXPos <= 236 )
                {
                        ActivateSoldierPopup( pSoldier, CONTRACT_POPUP, 172, y );
                }
                gfRenderPBInterface = TRUE;
        }
}

struct SOLDIERTYPE* InvolvedSoldier( int32_t index )
{
        struct GROUP *pGroup;
        PLAYERGROUP *pPlayer=NULL;
        BOOLEAN fFound = FALSE;
        if( index < 0 || index > 19 )
                return NULL;
        pGroup = gpGroupList;
        while( pGroup && !fFound )
        {
                if ( PlayerGroupInvolvedInThisCombat( pGroup ) )
                {
                        pPlayer = pGroup->pPlayerList;
                        while( pPlayer )
                        {
                                index--;
                                if( index <= 0 )
                                {
                                        fFound = TRUE;
                                        break;
                                }
                                pPlayer = pPlayer->next;
                        }
                }
                pGroup = pGroup->next;
        }
        if( !fFound )
                return NULL;
        return pPlayer->pSoldier;
}

struct SOLDIERTYPE* UninvolvedSoldier( int32_t index )
{
        struct GROUP *pGroup;
        PLAYERGROUP *pPlayer=NULL;
        BOOLEAN fFound = FALSE;
        if( index < 0 || index > 19 )
                return NULL;
        pGroup = gpGroupList;
        while( pGroup && !fFound )
        {
                if ( pGroup->fPlayer && !PlayerGroupInvolvedInThisCombat( pGroup ) )
                {
                        pPlayer = pGroup->pPlayerList;
                        while( pPlayer )
                        {
                                index--;
                                if( index <= 0 )
                                {
                                        fFound = TRUE;
                                        break;
                                }
                                pPlayer = pPlayer->next;
                        }
                }
                pGroup = pGroup->next;
        }
        if( !fFound )
                return NULL;
        return pPlayer->pSoldier;
}
*/

void ActivatePreBattleAutoresolveAction() {
  if (ButtonList[iPBButton[0]]->uiFlags &
      BUTTON_ENABLED) {  // Feign call the autoresolve button using the callback
    AutoResolveBattleCallback(ButtonList[iPBButton[0]], MSYS_CALLBACK_REASON_LBUTTON_UP);
  }
}

void ActivatePreBattleEnterSectorAction() {
  if (ButtonList[iPBButton[1]]->uiFlags &
      BUTTON_ENABLED) {  // Feign call the enter sector button using the callback
    GoToSectorCallback(ButtonList[iPBButton[1]], MSYS_CALLBACK_REASON_LBUTTON_UP);
  }
}

void ActivatePreBattleRetreatAction() {
  if (ButtonList[iPBButton[2]]->uiFlags &
      BUTTON_ENABLED) {  // Feign call the retreat button using the callback
    RetreatMercsCallback(ButtonList[iPBButton[2]], MSYS_CALLBACK_REASON_LBUTTON_UP);
  }
}

void ActivateAutomaticAutoResolveStart() {
  ButtonList[iPBButton[0]]->uiFlags |= BUTTON_CLICKED_ON;
  gfIgnoreAllInput = FALSE;
  AutoResolveBattleCallback(ButtonList[iPBButton[0]], MSYS_CALLBACK_REASON_LBUTTON_UP);
}

void CalculateNonPersistantPBIInfo() {
  // We need to set up the non-persistant PBI
  if (!gfBlitBattleSectorLocator || gubPBSectorX != gWorldSectorX ||
      gubPBSectorY != gWorldSectorY ||
      gubPBSectorZ != gbWorldSectorZ) {  // Either the locator isn't on or the locator info is in a
                                         // different sector

    // Calculated the encounter type
    gubEnemyEncounterCode = NO_ENCOUNTER_CODE;
    gubExplicitEnemyEncounterCode = NO_ENCOUNTER_CODE;
    if (HostileCiviliansPresent()) {  // There are hostile civilians, so no autoresolve allowed.
      gubExplicitEnemyEncounterCode = HOSTILE_CIVILIANS_CODE;
    } else if (HostileBloodcatsPresent()) {  // There are bloodcats in the sector, so no autoresolve
                                             // allowed
      gubExplicitEnemyEncounterCode = HOSTILE_BLOODCATS_CODE;
    } else if (gbWorldSectorZ) {
      UNDERGROUND_SECTORINFO *pSector =
          FindUnderGroundSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
      Assert(pSector);
      if (pSector->ubCreaturesInBattle) {
        gubExplicitEnemyEncounterCode = FIGHTING_CREATURES_CODE;
      } else if (pSector->ubAdminsInBattle || pSector->ubTroopsInBattle ||
                 pSector->ubElitesInBattle) {
        gubExplicitEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
        gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
      }
    } else {
      SECTORINFO *pSector =
          &SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)];
      Assert(pSector);
      if (pSector->ubCreaturesInBattle) {
        gubExplicitEnemyEncounterCode = FIGHTING_CREATURES_CODE;
      } else if (pSector->ubAdminsInBattle || pSector->ubTroopsInBattle ||
                 pSector->ubElitesInBattle) {
        gubExplicitEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
        gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
      }
    }
    if (gubExplicitEnemyEncounterCode !=
        NO_ENCOUNTER_CODE) {  // Set up the location as well as turning on the blit flag.
      gubPBSectorX = (uint8_t)gWorldSectorX;
      gubPBSectorY = (uint8_t)gWorldSectorY;
      gubPBSectorZ = (uint8_t)gbWorldSectorZ;
      gfBlitBattleSectorLocator = TRUE;
    }
  }
}

void ClearNonPersistantPBIInfo() { gfBlitBattleSectorLocator = FALSE; }

void PutNonSquadMercsInBattleSectorOnSquads(BOOLEAN fExitVehicles) {
  struct GROUP *pGroup, *pNextGroup;

  // IMPORTANT: Have to do this by group, so everyone inside vehicles gets assigned to the same
  // squad.  Needed for the tactical placement interface to work in case of simultaneous
  // multi-vehicle arrivals!

  pGroup = gpGroupList;
  while (pGroup) {
    // store ptr to next group in list, temporary groups will get deallocated as soon as the merc in
    // it is put on a squad!
    pNextGroup = pGroup->next;

    if (PlayerGroupInvolvedInThisCombat(pGroup)) {
      // the helicopter group CAN be involved, if it's on the ground, in which case everybody must
      // get out of it
      if (IsGroupTheHelicopterGroup(pGroup)) {
        // only happens if chopper is on the ground...
        Assert(!fHelicopterIsAirBorne);

        // put anyone in it into movement group
        MoveAllInHelicopterToFootMovementGroup();
      } else {
        PutNonSquadMercsInPlayerGroupOnSquads(pGroup, fExitVehicles);
      }
    }

    pGroup = pNextGroup;
  }
}

void PutNonSquadMercsInPlayerGroupOnSquads(struct GROUP *pGroup, BOOLEAN fExitVehicles) {
  PLAYERGROUP *pPlayer, *pNextPlayer;
  struct SOLDIERTYPE *pSoldier;
  int8_t bUniqueVehicleSquad = -1;
  BOOLEAN fSuccess;

  if (pGroup->fVehicle) {
    // put these guys on their own squad (we need to return their group ID, and can only return one,
    // so they need a unique one
    bUniqueVehicleSquad = GetFirstEmptySquad();
    if (bUniqueVehicleSquad == -1) {
      return;
    }
  }

  pPlayer = pGroup->pPlayerList;

  while (pPlayer) {
    pSoldier = pPlayer->pSoldier;
    Assert(pSoldier);

    // store ptr to next soldier in group, once removed from group, his info will get memfree'd!
    pNextPlayer = pPlayer->next;

    if (IsSolActive(pSoldier) && pSoldier->bLife && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      // if involved, but off-duty (includes mercs inside vehicles!)
      if (PlayerMercInvolvedInThisCombat(pSoldier) && (pSoldier->bAssignment >= ON_DUTY)) {
        // if in a vehicle, pull him out
        if (GetSolAssignment(pSoldier) == VEHICLE) {
          if (fExitVehicles) {
            TakeSoldierOutOfVehicle(pSoldier);

            // put them on the unique squad assigned to people leaving this vehicle.  Can't add them
            // to existing squads, because if this is a simultaneous group attack, the mercs could
            // be coming from different sides, and the placement screen can't handle mercs on the
            // same squad arriving from difference edges!
            fSuccess = AddCharacterToSquad(pSoldier, bUniqueVehicleSquad);
          }
        } else {
          // add him to ANY on duty foot squad
          fSuccess = AddCharacterToAnySquad(pSoldier);
        }

        // it better work...
        Assert(fSuccess);

        // clear any desired squad assignments
        pSoldier->ubNumTraversalsAllowedToMerge = 0;
        pSoldier->ubDesiredSquadAssignment = NO_ASSIGNMENT;

        // stand him up
        MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);
      }
    }

    pPlayer = pNextPlayer;
  }
}

void WakeUpAllMercsInSectorUnderAttack(void) {
  int32_t iCounter = 0, iNumberOfMercsOnTeam = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  // get number of possible grunts on team
  iNumberOfMercsOnTeam = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // any mercs not on duty should be added to the first avail squad
  for (iCounter = 0; iCounter < iNumberOfMercsOnTeam; iCounter++) {
    pSoldier = &(Menptr[iCounter]);

    if (IsSolActive(pSoldier) && pSoldier->bLife && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      // if involved, but asleep
      if (PlayerMercInvolvedInThisCombat(pSoldier) && (pSoldier->fMercAsleep == TRUE)) {
        // FORCE him wake him up
        SetMercAwake(pSoldier, FALSE, TRUE);
      }
    }
  }
}

// we are entering the sector, clear out all mvt orders for grunts
void ClearMovementForAllInvolvedPlayerGroups(void) {
  struct GROUP *pGroup;

  pGroup = gpGroupList;
  while (pGroup) {
    if (PlayerGroupInvolvedInThisCombat(pGroup)) {
      // clear their strategic movement (mercpaths and waypoints)
      ClearMercPathsAndWaypointsForAllInGroup(pGroup);
    }
    pGroup = pGroup->next;
  }
}

void RetreatAllInvolvedPlayerGroups(void) {
  struct GROUP *pGroup;

  // make sure guys stop their off duty assignments, like militia training!
  // but don't exit vehicles - drive off in them!
  PutNonSquadMercsInBattleSectorOnSquads(FALSE);

  pGroup = gpGroupList;
  while (pGroup) {
    if (PlayerGroupInvolvedInThisCombat(pGroup)) {
      // don't retreat empty vehicle groups!
      if (!pGroup->fVehicle || (pGroup->fVehicle && DoesVehicleGroupHaveAnyPassengers(pGroup))) {
        ClearMercPathsAndWaypointsForAllInGroup(pGroup);
        RetreatGroupToPreviousSector(pGroup);
      }
    }
    pGroup = pGroup->next;
  }
}

BOOLEAN PlayerMercInvolvedInThisCombat(struct SOLDIERTYPE *pSoldier) {
  Assert(pSoldier);
  Assert(IsSolActive(pSoldier));

  if (!pSoldier->fBetweenSectors && pSoldier->bAssignment != IN_TRANSIT &&
      pSoldier->bAssignment != ASSIGNMENT_POW && pSoldier->bAssignment != ASSIGNMENT_DEAD &&
      !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
      // Robot is involved if it has a valid controller with it, uninvolved otherwise
      (!AM_A_ROBOT(pSoldier) || (pSoldier->ubRobotRemoteHolderID != NOBODY)) &&
      !SoldierAboardAirborneHeli(pSoldier)) {
    if (CurrentBattleSectorIs(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                              GetSolSectorZ(pSoldier))) {
      // involved
      return (TRUE);
    }
  }

  // not involved
  return (FALSE);
}

BOOLEAN PlayerGroupInvolvedInThisCombat(struct GROUP *pGroup) {
  Assert(pGroup);

  // player group, non-empty, not between sectors, in the right sector, isn't a group of in transit,
  // dead, or POW mercs, and either not the helicopter group, or the heli is on the ground
  if (pGroup->fPlayer && pGroup->ubGroupSize && !pGroup->fBetweenSectors &&
      !GroupHasInTransitDeadOrPOWMercs(pGroup) &&
      (!IsGroupTheHelicopterGroup(pGroup) || !fHelicopterIsAirBorne)) {
    if (CurrentBattleSectorIs(pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ)) {
      // involved
      return (TRUE);
    }
  }

  // not involved
  return (FALSE);
}

BOOLEAN CurrentBattleSectorIs(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ) {
  uint8_t sBattleSectorX, sBattleSectorY;
  int8_t sBattleSectorZ;
  BOOLEAN fSuccess;

  fSuccess = GetCurrentBattleSectorXYZ(&sBattleSectorX, &sBattleSectorY, &sBattleSectorZ);
  Assert(fSuccess);

  if ((sSectorX == sBattleSectorX) && (sSectorY == sBattleSectorY) &&
      (sSectorZ == sBattleSectorZ)) {
    // yup!
    return (TRUE);
  } else {
    // wrong sector, no battle here
    return (FALSE);
  }
}

void CheckForRobotAndIfItsControlled(void) {
  int32_t i;

  // search for the robot on player's team
  for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
       i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife && AM_A_ROBOT(MercPtrs[i])) {
      // check whether it has a valid controller with it. This sets its ubRobotRemoteHolderID field.
      UpdateRobotControllerGivenRobot(MercPtrs[i]);

      // if he has a controller, set controllers
      if (MercPtrs[i]->ubRobotRemoteHolderID != NOBODY) {
        UpdateRobotControllerGivenController(MercPtrs[MercPtrs[i]->ubRobotRemoteHolderID]);
      }

      break;
    }
  }
}

void LogBattleResults(uint8_t ubVictoryCode) {
  uint8_t sSectorX, sSectorY;
  int8_t sSectorZ;
  GetCurrentBattleSectorXYZ(&sSectorX, &sSectorY, &sSectorZ);
  if (ubVictoryCode == LOG_VICTORY) {
    switch (gubEnemyEncounterCode) {
      case ENEMY_INVASION_CODE:
        AddHistoryToPlayersLog(HISTORY_DEFENDEDTOWNSECTOR, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
      case ENEMY_ENCOUNTER_CODE:
        AddHistoryToPlayersLog(HISTORY_WONBATTLE, 0, GetWorldTotalMin(), sSectorX, sSectorY);
        break;
      case ENEMY_AMBUSH_CODE:
        AddHistoryToPlayersLog(HISTORY_WIPEDOUTENEMYAMBUSH, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
      case ENTERING_ENEMY_SECTOR_CODE:
        AddHistoryToPlayersLog(HISTORY_SUCCESSFULATTACK, 0, GetWorldTotalMin(), sSectorX, sSectorY);
        break;
      case CREATURE_ATTACK_CODE:
        AddHistoryToPlayersLog(HISTORY_CREATURESATTACKED, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
      case BLOODCAT_AMBUSH_CODE:
      case ENTERING_BLOODCAT_LAIR_CODE:
        AddHistoryToPlayersLog(HISTORY_SLAUGHTEREDBLOODCATS, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
    }
  } else {
    switch (gubEnemyEncounterCode) {
      case ENEMY_INVASION_CODE:
        AddHistoryToPlayersLog(HISTORY_LOSTTOWNSECTOR, 0, GetWorldTotalMin(), sSectorX, sSectorY);
        break;
      case ENEMY_ENCOUNTER_CODE:
        AddHistoryToPlayersLog(HISTORY_LOSTBATTLE, 0, GetWorldTotalMin(), sSectorX, sSectorY);
        break;
      case ENEMY_AMBUSH_CODE:
        AddHistoryToPlayersLog(HISTORY_FATALAMBUSH, 0, GetWorldTotalMin(), sSectorX, sSectorY);
        break;
      case ENTERING_ENEMY_SECTOR_CODE:
        AddHistoryToPlayersLog(HISTORY_UNSUCCESSFULATTACK, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
      case CREATURE_ATTACK_CODE:
        AddHistoryToPlayersLog(HISTORY_CREATURESATTACKED, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
      case BLOODCAT_AMBUSH_CODE:
      case ENTERING_BLOODCAT_LAIR_CODE:
        AddHistoryToPlayersLog(HISTORY_KILLEDBYBLOODCATS, 0, GetWorldTotalMin(), sSectorX,
                               sSectorY);
        break;
    }
  }
}

void HandlePreBattleInterfaceStates() {
  if (gfEnteringMapScreenToEnterPreBattleInterface && !gfEnteringMapScreen) {
    gfEnteringMapScreenToEnterPreBattleInterface = FALSE;
    if (!gfUsePersistantPBI) {
      InitPreBattleInterface(NULL, FALSE);
      gfUsePersistantPBI = TRUE;
    } else {
      InitPreBattleInterface(gpBattleGroup, TRUE);
    }
  } else if (gfDelayAutoResolveStart && gfPreBattleInterfaceActive) {
    gfDelayAutoResolveStart = FALSE;
    gfAutomaticallyStartAutoResolve = TRUE;
  } else if (gfAutomaticallyStartAutoResolve) {
    gfAutomaticallyStartAutoResolve = FALSE;
    ActivateAutomaticAutoResolveStart();
  } else if (gfTransitionMapscreenToAutoResolve) {
    gfTransitionMapscreenToAutoResolve = FALSE;
  }
}
