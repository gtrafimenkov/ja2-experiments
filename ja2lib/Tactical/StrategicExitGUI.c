// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/StrategicExitGUI.h"

#include <stdio.h>

#include "FadeScreen.h"
#include "GameScreen.h"
#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/English.h"
#include "SGP/MouseSystem.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Squads.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/MusicControl.h"
#include "Utils/PopUpBox.h"
#include "Utils/Text.h"

BOOLEAN gfInSectorExitMenu = FALSE;

void CheckLoadMapCallback(GUI_BUTTON *btn, int32_t reason);
void SingleMoveCallback(GUI_BUTTON *btn, int32_t reason);
void AllMoveCallback(GUI_BUTTON *btn, int32_t reason);

void OKCallback(GUI_BUTTON *btn, int32_t reason);
void CancelCallback(GUI_BUTTON *btn, int32_t reason);

void SectorExitBackgroundCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void SingleRegionCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void AllRegionCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void LoadRegionCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void SingleRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void AllRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void LoadRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

typedef struct {
  struct MOUSE_REGION BackRegion;
  struct MOUSE_REGION SingleRegion;
  struct MOUSE_REGION LoadRegion;
  struct MOUSE_REGION AllRegion;
  uint32_t uiLoadCheckButton;
  uint32_t uiSingleMoveButton;
  uint32_t uiAllMoveButton;
  uint32_t uiOKButton;
  uint32_t uiCancelButton;
  int32_t iBoxId;
  int32_t iButtonImages;
  uint16_t usWidth;
  uint16_t usHeight;
  int16_t sX;
  int16_t sY;
  int16_t sAdditionalData;
  uint8_t ubFlags;
  uint8_t ubLeaveSectorType;
  uint8_t ubLeaveSectorCode;
  uint8_t ubDirection;
  uint8_t ubNumPeopleOnSquad;
  int8_t bSingleMoveWillIsolateEPC;  // if not -1, then that means the slot number is an EPC
  int8_t bHandled;
  BOOLEAN fRender;
  BOOLEAN fGotoSector;
  BOOLEAN fGotoSectorText;
  BOOLEAN fSingleMove;
  BOOLEAN fAllMove;
  BOOLEAN fSingleMoveDisabled;
  BOOLEAN fGotoSectorDisabled;
  BOOLEAN fAllMoveDisabled;
  BOOLEAN fGotoSectorHilighted;
  BOOLEAN fSingleMoveHilighted;
  BOOLEAN fAllMoveHilighted;
  BOOLEAN fMultipleSquadsInSector;
  BOOLEAN fSingleMoveOn;
  BOOLEAN fAllMoveOn;
  BOOLEAN fSelectedMercIsEPC;
  BOOLEAN fSquadHasMultipleEPCs;
  BOOLEAN fUncontrolledRobotInSquad;

} EXIT_DIALOG_STRUCT;

EXIT_DIALOG_STRUCT gExitDialog;

uint8_t gubExitGUIDirection;
int16_t gsExitGUIAdditionalData;
uint8_t gsWarpWorldX;
uint8_t gsWarpWorldY;
int8_t gbWarpWorldZ;
int16_t gsWarpGridNo;

// KM:  New method is coded for more sophistocated rules.  All the information is stored within the
// gExitDialog struct 		 and calculated upon entry to this function instead of passing in
// multiple
// arguments and calculating it prior.
BOOLEAN InternalInitSectorExitMenu(uint8_t ubDirection, int16_t sAdditionalData) {
  uint32_t uiTraverseTimeInMinutes;
  struct SOLDIERTYPE *pSoldier;
  int32_t i;
  SGPRect aRect;
  uint16_t usTextBoxWidth, usTextBoxHeight;
  uint16_t usMapPos = 0;
  int8_t bExitCode = -1;
  BOOLEAN OkExitCode;

  // STEP 1:  Calculate the information for the exit gui
  memset(&gExitDialog, 0, sizeof(EXIT_DIALOG_STRUCT));
  gExitDialog.bSingleMoveWillIsolateEPC = -1;

  // OK, bring up dialogue... first determine some logic here...
  switch (ubDirection) {
    case EAST:
      bExitCode = EAST_STRATEGIC_MOVE;
      break;
    case WEST:
      bExitCode = WEST_STRATEGIC_MOVE;
      break;
    case NORTH:
      bExitCode = NORTH_STRATEGIC_MOVE;
      break;
    case SOUTH:
      bExitCode = SOUTH_STRATEGIC_MOVE;
      break;
    case DIRECTION_EXITGRID:
      bExitCode = -1;
      usMapPos = sAdditionalData;
      break;
  }

  OkExitCode = OKForSectorExit(bExitCode, usMapPos, &uiTraverseTimeInMinutes);

  if (uiTraverseTimeInMinutes <=
      5) {  // if the traverse time is short, then traversal is percieved to be instantaneous.
    gExitDialog.fGotoSectorText = TRUE;
  }

  if (OkExitCode == 1) {
    gExitDialog.fAllMoveDisabled = TRUE;
    gExitDialog.fSingleMoveOn = TRUE;
    gExitDialog.fSingleMove = TRUE;
    if (gfRobotWithoutControllerAttemptingTraversal) {
      gfRobotWithoutControllerAttemptingTraversal = FALSE;
      gExitDialog.fUncontrolledRobotInSquad = TRUE;
    }

  } else if (OkExitCode == 2) {
    gExitDialog.fAllMoveOn = TRUE;
    gExitDialog.fAllMove = TRUE;
  }

  if (gTacticalStatus.uiFlags & INCOMBAT) {
    int32_t i, cnt = 0;
    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {
      if (OK_INSECTOR_MERC(MercPtrs[i])) cnt++;
    }
    if (cnt != 1) {
      gExitDialog.fGotoSectorDisabled = TRUE;
    }
  }

  // STEP 2:  Setup the exit gui

  EnterModalTactical(TACTICAL_MODAL_WITHMOUSE);
  gfIgnoreScrolling = TRUE;

  aRect.iTop = 0;
  aRect.iLeft = 0;
  aRect.iBottom = INV_INTERFACE_START_Y;
  aRect.iRight = 640;

  if (gExitDialog
          .fAllMoveOn) {  // either an all-move in non-combat, or the last concious guy in combat.
    gExitDialog.fGotoSector = TRUE;
  }

  gExitDialog.ubNumPeopleOnSquad =
      NumberOfPlayerControllableMercsInSquad(MercPtrs[gusSelectedSoldier]->bAssignment);

  // Determine
  for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
       i++) {
    pSoldier = MercPtrs[i];
    if (i == gusSelectedSoldier) {
      continue;
    }
    if (!pSoldier->fBetweenSectors && GetSolSectorX(pSoldier) == gWorldSectorX &&
        GetSolSectorY(pSoldier) == gWorldSectorY && GetSolSectorZ(pSoldier) == gbWorldSectorZ &&
        pSoldier->bLife >= OKLIFE &&
        pSoldier->bAssignment != MercPtrs[gusSelectedSoldier]->bAssignment &&
        pSoldier->bAssignment != ASSIGNMENT_POW && pSoldier->bAssignment != IN_TRANSIT &&
        pSoldier->bAssignment !=
            ASSIGNMENT_DEAD) {  // KM:  We need to determine if there are more than one squad
                                // (meaning other concious mercs in a different squad or assignment)
      //		 These conditions were done to the best of my knowledge, so if there are
      // other situations that require modification, 		 then feel free to do so.
      gExitDialog.fMultipleSquadsInSector = TRUE;
      break;
    }
  }

  // Double check that ...
  // if we are a EPC and are the selected guy, make single move off and disable it....
  if (AM_AN_EPC(MercPtrs[gusSelectedSoldier])) {
    // Check if there are more than one in this squad
    if (gExitDialog.ubNumPeopleOnSquad > 1) {
      gExitDialog.fSingleMoveOn = FALSE;
      gExitDialog.fAllMoveOn = TRUE;
      gExitDialog.fSelectedMercIsEPC = TRUE;
    }
    gExitDialog.fSingleMoveDisabled = TRUE;
  } else {  // check to see if we have one selected merc and one or more EPCs.
    // If so, don't allow the selected merc to leave by himself.
    // Assuming that the matching squad assignment is in the same sector.
    uint8_t ubNumMercs = 1;  // selected soldier is a merc
    uint8_t ubNumEPCs = 0;
    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {
      if (i == gusSelectedSoldier) {
        continue;
      }
      if (MercPtrs[i]->bAssignment == MercPtrs[gusSelectedSoldier]->bAssignment) {
        if (AM_AN_EPC(MercPtrs[i])) {
          ubNumEPCs++;
          // record the slot of the epc.  If there are more than one EPCs, then
          // it doesn't matter.  This is used in building the text message explaining
          // why the selected merc can't leave.  This is how we extract the EPC's name.
          gExitDialog.bSingleMoveWillIsolateEPC = (int8_t)i;
        } else {  // We have more than one merc, so we will allow the selected merc to leave alone
                  // if
          // the user so desired.
          ubNumMercs++;
          break;
        }
      }
    }

    if (ubNumMercs == 1 && ubNumEPCs >= 1) {
      gExitDialog.fSingleMoveOn = FALSE;
      gExitDialog.fAllMoveOn = TRUE;
      gExitDialog.fSingleMoveDisabled = TRUE;
      if (ubNumEPCs > 1) {
        gExitDialog.fSquadHasMultipleEPCs = TRUE;
        ;
      }
    }
  }

  if (gTacticalStatus.fEnemyInSector) {
    if (gExitDialog.fMultipleSquadsInSector) {  // We have multiple squads in a hostile sector. That
                                                // means that we can't load the adjacent sector.
      gExitDialog.fGotoSectorDisabled = TRUE;
      gExitDialog.fGotoSector = FALSE;
    } else if (GetNumberOfMilitiaInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                          gbWorldSectorZ)) {
      // Leaving this sector will result in militia being forced to
      // fight the battle, can't load adjacent sector.
      gExitDialog.fGotoSectorDisabled = TRUE;
      gExitDialog.fGotoSector = FALSE;
    }
    if (!gExitDialog.fMultipleSquadsInSector && !gExitDialog.fAllMoveOn) {
      gExitDialog.fGotoSectorDisabled = TRUE;
      gExitDialog.fGotoSector = FALSE;
    }
  }

  if (!gExitDialog.fMultipleSquadsInSector && gExitDialog.fAllMoveOn) {
    gExitDialog.fGotoSectorDisabled = TRUE;
  }

  gExitDialog.ubDirection = ubDirection;
  gExitDialog.sAdditionalData = sAdditionalData;

  gExitDialog.iBoxId = PrepareMercPopupBox(
      -1, DIALOG_MERC_POPUP_BACKGROUND, DIALOG_MERC_POPUP_BORDER, TacticalStr[EXIT_GUI_TITLE_STR],
      100, 85, 2, 75, &usTextBoxWidth, &usTextBoxHeight);

  gExitDialog.sX = (int16_t)((((aRect.iRight - aRect.iLeft) - usTextBoxWidth) / 2) + aRect.iLeft);
  gExitDialog.sY = (int16_t)((((aRect.iBottom - aRect.iTop) - usTextBoxHeight) / 2) + aRect.iTop);
  gExitDialog.usWidth = usTextBoxWidth;
  gExitDialog.usHeight = usTextBoxHeight;

  guiPendingOverrideEvent = EX_EXITSECTORMENU;
  HandleTacticalUI();

  gfInSectorExitMenu = TRUE;

  MSYS_DefineRegion(&(gExitDialog.BackRegion), 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 1,
                    CURSOR_NORMAL, MSYS_NO_CALLBACK, SectorExitBackgroundCallback);

  gExitDialog.iButtonImages = LoadButtonImage("INTERFACE\\popupbuttons.sti", -1, 0, -1, 1, -1);

  MSYS_DefineRegion(&gExitDialog.SingleRegion, (int16_t)(gExitDialog.sX + 20),
                    (int16_t)(gExitDialog.sY + 37), (int16_t)(gExitDialog.sX + 45 + 120),
                    (int16_t)(gExitDialog.sY + 37 + 12), MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL,
                    SingleRegionMoveCallback, SingleRegionCallback);
  MSYS_AllowDisabledRegionFastHelp(&gExitDialog.SingleRegion, TRUE);

  MSYS_DefineRegion(&(gExitDialog.AllRegion), (int16_t)(gExitDialog.sX + 20),
                    (int16_t)(gExitDialog.sY + 57), (int16_t)(gExitDialog.sX + 45 + 120),
                    (int16_t)(gExitDialog.sY + 57 + 12), MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL,
                    AllRegionMoveCallback, AllRegionCallback);
  MSYS_AllowDisabledRegionFastHelp(&gExitDialog.AllRegion, TRUE);

  MSYS_DefineRegion(&(gExitDialog.LoadRegion), (int16_t)(gExitDialog.sX + 155),
                    (int16_t)(gExitDialog.sY + 45), (int16_t)(gExitDialog.sX + 180 + 85),
                    (int16_t)(gExitDialog.sY + 45 + 15), MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL,
                    LoadRegionMoveCallback, LoadRegionCallback);
  MSYS_AllowDisabledRegionFastHelp(&gExitDialog.LoadRegion, TRUE);

  gExitDialog.uiLoadCheckButton = CreateCheckBoxButton(
      (int16_t)(gExitDialog.sX + 155), (int16_t)(gExitDialog.sY + 43), "INTERFACE\\popupcheck.sti",
      MSYS_PRIORITY_HIGHEST, CheckLoadMapCallback);

  gExitDialog.uiSingleMoveButton = CreateCheckBoxButton(
      (int16_t)(gExitDialog.sX + 20), (int16_t)(gExitDialog.sY + 35),
      "INTERFACE\\popupradiobuttons.sti", MSYS_PRIORITY_HIGHEST, SingleMoveCallback);

  gExitDialog.uiAllMoveButton = CreateCheckBoxButton(
      (int16_t)(gExitDialog.sX + 20), (int16_t)(gExitDialog.sY + 55),
      "INTERFACE\\popupradiobuttons.sti", MSYS_PRIORITY_HIGHEST, AllMoveCallback);

  gExitDialog.uiOKButton = CreateIconAndTextButton(
      gExitDialog.iButtonImages, TacticalStr[OK_BUTTON_TEXT_STR], FONT12ARIAL, FONT_MCOLOR_WHITE,
      DEFAULT_SHADOW, FONT_MCOLOR_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
      (int16_t)(gExitDialog.sX + 65), (int16_t)(gExitDialog.sY + 78), BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)OKCallback);

  gExitDialog.uiCancelButton = CreateIconAndTextButton(
      gExitDialog.iButtonImages, TacticalStr[CANCEL_BUTTON_TEXT_STR], FONT12ARIAL,
      FONT_MCOLOR_WHITE, DEFAULT_SHADOW, FONT_MCOLOR_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
      (int16_t)(gExitDialog.sX + 135), (int16_t)(gExitDialog.sY + 78), BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)CancelCallback);

  gfIgnoreScrolling = TRUE;

  InterruptTime();
  PauseGame();
  LockPauseState(21);

  return (TRUE);
}

void DoneFadeInWarp(void) {}

void DoneFadeOutWarpCallback(void) {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // Warp!

  // Set insertion data...
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // Are we in this sector, On the current squad?
    if (IsSolActive(pSoldier) && pSoldier->bLife >= OKLIFE && pSoldier->bInSector) {
      gfTacticalTraversal = TRUE;
      SetGroupSectorValue(gsWarpWorldX, gsWarpWorldY, gbWarpWorldZ, pSoldier->ubGroupID);

      // Set next sectore
      pSoldier->sSectorX = gsWarpWorldX;
      pSoldier->sSectorY = gsWarpWorldY;
      pSoldier->bSectorZ = gbWarpWorldZ;

      // Set gridno
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
      pSoldier->usStrategicInsertionData = gsWarpGridNo;
      // Set direction to face....
      pSoldier->ubInsertionDirection = 100 + NORTHWEST;
    }
  }

  // OK, insertion data found, enter sector!
  SetCurrentWorldSector(gsWarpWorldX, gsWarpWorldY, gbWarpWorldZ);

  // OK, once down here, adjust the above map with crate info....
  gfTacticalTraversal = FALSE;
  gpTacticalTraversalGroup = NULL;
  gpTacticalTraversalChosenSoldier = NULL;

  gFadeInDoneCallback = DoneFadeInWarp;

  FadeInGameScreen();
}

void WarpToSurfaceCallback(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_YES) {
    gFadeOutDoneCallback = DoneFadeOutWarpCallback;

    FadeOutGameScreen();
  } else {
    InternalInitSectorExitMenu(gubExitGUIDirection, gsExitGUIAdditionalData);
  }
}

BOOLEAN InitSectorExitMenu(uint8_t ubDirection, int16_t sAdditionalData) {
  gubExitGUIDirection = ubDirection;
  gsExitGUIAdditionalData = sAdditionalData;

  if (gbWorldSectorZ >= 2 && gubQuest[QUEST_CREATURES] == QUESTDONE) {
    uint8_t mapX, mapY;
    if (GetWarpOutOfMineCodes(&mapX, &mapY, &gbWarpWorldZ, &gsWarpGridNo)) {
      gsWarpWorldX = mapX;
      gsWarpWorldY = mapY;
      // ATE: Check if we are in a creature lair and bring up box if so....
      DoMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[33], GAME_SCREEN,
                   (uint8_t)MSG_BOX_FLAG_YESNO, WarpToSurfaceCallback, NULL);

      return (TRUE);
    }
  }

  return (InternalInitSectorExitMenu(ubDirection, sAdditionalData));
}

void UpdateSectorExitMenu() {
  if (gExitDialog.fGotoSector) {
    ButtonList[gExitDialog.uiLoadCheckButton]->uiFlags |= BUTTON_CLICKED_ON;
  } else {
    ButtonList[gExitDialog.uiLoadCheckButton]->uiFlags &= (~BUTTON_CLICKED_ON);
  }

  if (gExitDialog.fSingleMove) {
    ButtonList[gExitDialog.uiSingleMoveButton]->uiFlags |= BUTTON_CLICKED_ON;
  } else {
    ButtonList[gExitDialog.uiSingleMoveButton]->uiFlags &= (~BUTTON_CLICKED_ON);
  }

  if (gExitDialog.fAllMove) {
    ButtonList[gExitDialog.uiAllMoveButton]->uiFlags |= BUTTON_CLICKED_ON;
  } else {
    ButtonList[gExitDialog.uiAllMoveButton]->uiFlags &= (~BUTTON_CLICKED_ON);
  }

  if (gExitDialog.fGotoSectorDisabled) {
    DisableButton(gExitDialog.uiLoadCheckButton);
    MSYS_DisableRegion(&(gExitDialog.LoadRegion));
    if (gExitDialog.fMultipleSquadsInSector && gExitDialog.fGotoSectorText &&
        gTacticalStatus.fEnemyInSector) {  // We have multiple squads in a hostile sector.  That
                                           // means that we can't load the adjacent sector.
      SetButtonFastHelpText(gExitDialog.uiLoadCheckButton,
                            pExitingSectorHelpText[EXIT_GUI_CANT_LEAVE_HOSTILE_SECTOR_HELPTEXT]);
      SetRegionFastHelpText(&gExitDialog.LoadRegion,
                            pExitingSectorHelpText[EXIT_GUI_CANT_LEAVE_HOSTILE_SECTOR_HELPTEXT]);
    } else if (gExitDialog.fGotoSectorText) {  // travesal is quick enough to allow the player to
                                               // "warp" to the next sector and we MUST load it.
      SetButtonFastHelpText(gExitDialog.uiLoadCheckButton,
                            pExitingSectorHelpText[EXIT_GUI_MUST_LOAD_ADJACENT_SECTOR_HELPTEXT]);
      SetRegionFastHelpText(&gExitDialog.LoadRegion,
                            pExitingSectorHelpText[EXIT_GUI_MUST_LOAD_ADJACENT_SECTOR_HELPTEXT]);
    } else {  // traversal takes too long to warrant instant travel (we MUST go to mapscreen)
      SetButtonFastHelpText(gExitDialog.uiLoadCheckButton,
                            pExitingSectorHelpText[EXIT_GUI_MUST_GOTO_MAPSCREEN_HELPTEXT]);
      SetRegionFastHelpText(&gExitDialog.LoadRegion,
                            pExitingSectorHelpText[EXIT_GUI_MUST_GOTO_MAPSCREEN_HELPTEXT]);
    }
  } else {
    EnableButton(gExitDialog.uiLoadCheckButton);
    MSYS_EnableRegion(&(gExitDialog.LoadRegion));
    if (gExitDialog.fGotoSectorText) {  // travesal is quick enough to allow the player to "warp" to
                                        // the next sector and we load it.
      SetButtonFastHelpText(gExitDialog.uiLoadCheckButton,
                            pExitingSectorHelpText[EXIT_GUI_LOAD_ADJACENT_SECTOR_HELPTEXT]);
      SetRegionFastHelpText(&gExitDialog.LoadRegion,
                            pExitingSectorHelpText[EXIT_GUI_LOAD_ADJACENT_SECTOR_HELPTEXT]);
    } else {  // traversal takes too long to warrant instant travel (we go to mapscreen)
      SetButtonFastHelpText(gExitDialog.uiLoadCheckButton,
                            pExitingSectorHelpText[EXIT_GUI_GOTO_MAPSCREEN_HELPTEXT]);
      SetRegionFastHelpText(&gExitDialog.LoadRegion,
                            pExitingSectorHelpText[EXIT_GUI_GOTO_MAPSCREEN_HELPTEXT]);
    }
  }

  if (gExitDialog.fSingleMoveDisabled) {
    DisableButton(gExitDialog.uiSingleMoveButton);
    MSYS_DisableRegion(&(gExitDialog.SingleRegion));
    if (gExitDialog.fSelectedMercIsEPC) {  // EPCs cannot leave the sector alone and must be
                                           // escorted
      wchar_t str[256];
      swprintf(str, ARR_SIZE(str),
               pExitingSectorHelpText[EXIT_GUI_ESCORTED_CHARACTERS_MUST_BE_ESCORTED_HELPTEXT],
               MercPtrs[gusSelectedSoldier]->name);
      SetButtonFastHelpText(gExitDialog.uiSingleMoveButton, str);
      SetRegionFastHelpText(&gExitDialog.SingleRegion, str);
    } else if (gExitDialog.bSingleMoveWillIsolateEPC !=
               -1) {  // It has been previously determined that there are only two mercs in the
                      // squad, the selected merc
      // isn't an EPC, but the other merc is.  That means that this merc cannot leave the sector
      // alone as he would isolate the EPC.
      wchar_t str[256];
      if (!gExitDialog.fSquadHasMultipleEPCs) {
        if (gMercProfiles[MercPtrs[gusSelectedSoldier]->ubProfile].bSex == MALE) {  // male singular
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_SINGULAR],
                   MercPtrs[gusSelectedSoldier]->name,
                   MercPtrs[gExitDialog.bSingleMoveWillIsolateEPC]->name);
        } else {  // female singular
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_SINGULAR],
                   MercPtrs[gusSelectedSoldier]->name,
                   MercPtrs[gExitDialog.bSingleMoveWillIsolateEPC]->name);
        }
      } else {
        if (gMercProfiles[MercPtrs[gusSelectedSoldier]->ubProfile].bSex == MALE) {  // male plural
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_PLURAL],
                   MercPtrs[gusSelectedSoldier]->name);
        } else {  // female plural
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_PLURAL],
                   MercPtrs[gusSelectedSoldier]->name);
        }
      }
      SetButtonFastHelpText(gExitDialog.uiSingleMoveButton, str);
      SetRegionFastHelpText(&gExitDialog.SingleRegion, str);
    }
  } else {
    wchar_t str[256];
    EnableButton(gExitDialog.uiSingleMoveButton);
    MSYS_EnableRegion(&(gExitDialog.SingleRegion));
    swprintf(str, ARR_SIZE(str),
             pExitingSectorHelpText[EXIT_GUI_SINGLE_TRAVERSAL_WILL_SEPARATE_SQUADS_HELPTEXT],
             MercPtrs[gusSelectedSoldier]->name);
    SetButtonFastHelpText(gExitDialog.uiSingleMoveButton, str);
    SetRegionFastHelpText(&gExitDialog.SingleRegion, str);
  }

  if (gExitDialog.fAllMoveDisabled) {
    DisableButton(gExitDialog.uiAllMoveButton);
    MSYS_DisableRegion(&(gExitDialog.AllRegion));
    if (gExitDialog.fUncontrolledRobotInSquad) {
      SetButtonFastHelpText(gExitDialog.uiAllMoveButton, gzLateLocalizedString[1]);
      SetRegionFastHelpText(&gExitDialog.AllRegion, gzLateLocalizedString[1]);
    } else {
      SetButtonFastHelpText(
          gExitDialog.uiAllMoveButton,
          pExitingSectorHelpText[EXIT_GUI_ALL_MERCS_MUST_BE_TOGETHER_TO_ALLOW_HELPTEXT]);
      SetRegionFastHelpText(
          &gExitDialog.AllRegion,
          pExitingSectorHelpText[EXIT_GUI_ALL_MERCS_MUST_BE_TOGETHER_TO_ALLOW_HELPTEXT]);
    }
  } else {
    EnableButton(gExitDialog.uiAllMoveButton);
    MSYS_EnableRegion(&(gExitDialog.AllRegion));
    SetButtonFastHelpText(
        gExitDialog.uiAllMoveButton,
        pExitingSectorHelpText[EXIT_GUI_ALL_TRAVERSAL_WILL_MOVE_CURRENT_SQUAD_HELPTEXT]);
    SetRegionFastHelpText(
        &gExitDialog.AllRegion,
        pExitingSectorHelpText[EXIT_GUI_ALL_TRAVERSAL_WILL_MOVE_CURRENT_SQUAD_HELPTEXT]);
  }
}

void RenderSectorExitMenu() {
  InputAtom Event;

  RestoreBackgroundRects();
  // ATE: Reset mouse Y
  gsGlobalCursorYOffset = 0;
  SetCurrentCursorFromDatabase(CURSOR_NORMAL);

  while (DequeueEvent(&Event)) {
    if (Event.usEvent == KEY_DOWN) {
      switch (Event.usParam) {
        case ESC:
          RemoveSectorExitMenu(FALSE);
          return;
        case ENTER:
          RemoveSectorExitMenu(TRUE);
          return;
      }
    }
  }

  UpdateSectorExitMenu();

  RenderMercPopUpBoxFromIndex(gExitDialog.iBoxId, gExitDialog.sX, gExitDialog.sY, vsFB);
  InvalidateRegion(gExitDialog.sX, gExitDialog.sY, gExitDialog.usWidth, gExitDialog.usHeight);

  SetFont(FONT12ARIAL);
  SetFontBackground(FONT_MCOLOR_BLACK);

  if (gExitDialog.fSingleMoveDisabled) {
    SetFontForeground(FONT_MCOLOR_DKGRAY);
  } else if (gExitDialog.fSingleMoveHilighted) {
    SetFontForeground(FONT_MCOLOR_LTYELLOW);
  } else {
    SetFontForeground(FONT_MCOLOR_WHITE);
  }
  mprintf(gExitDialog.sX + 45, gExitDialog.sY + 37, TacticalStr[EXIT_GUI_SELECTED_MERC_STR]);

  if (gExitDialog.fAllMoveDisabled) {
    SetFontForeground(FONT_MCOLOR_DKGRAY);
  } else if (gExitDialog.fAllMoveHilighted) {
    SetFontForeground(FONT_MCOLOR_LTYELLOW);
  } else {
    SetFontForeground(FONT_MCOLOR_WHITE);
  }
  mprintf(gExitDialog.sX + 45, gExitDialog.sY + 57, TacticalStr[EXIT_GUI_ALL_MERCS_IN_SQUAD_STR]);

  if (gExitDialog.fGotoSectorDisabled) {
    SetFontForeground(FONT_MCOLOR_DKGRAY);
  } else if (gExitDialog.fGotoSectorHilighted) {
    SetFontForeground(FONT_MCOLOR_LTYELLOW);
  } else {
    SetFontForeground(FONT_MCOLOR_WHITE);
  }

  if (gExitDialog.fGotoSectorText) {  // only if tactical traversal is from one town sector to
                                      // another town sector (5 minute convenience warp)
    mprintf(gExitDialog.sX + 180, gExitDialog.sY + 45, TacticalStr[EXIT_GUI_GOTO_SECTOR_STR]);
  } else {  // most sectors don't allow tactical traversal.  Exiting results in entering the
            // mapscreen.
    mprintf(gExitDialog.sX + 180, gExitDialog.sY + 45, TacticalStr[EXIT_GUI_GOTO_MAP_STR]);
  }

  SaveBackgroundRects();
  RenderFastHelp();

  MarkAButtonDirty(gExitDialog.uiLoadCheckButton);
  MarkAButtonDirty(gExitDialog.uiSingleMoveButton);
  MarkAButtonDirty(gExitDialog.uiAllMoveButton);
  MarkAButtonDirty(gExitDialog.uiOKButton);
  MarkAButtonDirty(gExitDialog.uiCancelButton);
}

BOOLEAN HandleSectorExitMenu() {
  return (FALSE);  // Why???
}

void RemoveSectorExitMenu(BOOLEAN fOk) {
  wchar_t Str[50];

  if (gfInSectorExitMenu) {
    guiPendingOverrideEvent = A_CHANGE_TO_MOVE;

    RemoveButton(gExitDialog.uiLoadCheckButton);
    RemoveButton(gExitDialog.uiSingleMoveButton);
    RemoveButton(gExitDialog.uiAllMoveButton);
    RemoveButton(gExitDialog.uiOKButton);
    RemoveButton(gExitDialog.uiCancelButton);

    UnloadButtonImage(gExitDialog.iButtonImages);

    MSYS_RemoveRegion(&(gExitDialog.BackRegion));
    MSYS_RemoveRegion(&(gExitDialog.SingleRegion));
    MSYS_RemoveRegion(&(gExitDialog.AllRegion));
    MSYS_RemoveRegion(&(gExitDialog.LoadRegion));

    // Remove the popup box
    RemoveMercPopupBoxFromIndex(gExitDialog.iBoxId);
    gExitDialog.iBoxId = -1;

    gfIgnoreScrolling = FALSE;

    SetRenderFlags(RENDER_FLAG_FULL);

    gfInSectorExitMenu = FALSE;

    UnLockPauseState();
    UnPauseGame();
    EndModalTactical();
    gfIgnoreScrolling = FALSE;

    // if we are an EPC, don't allow this if nobody else on squad
    if (fOk && AM_AN_EPC(MercPtrs[gusSelectedSoldier])) {
      // Check if there are more than one in this squad
      if (gExitDialog.ubNumPeopleOnSquad == 0) {
        swprintf(Str, ARR_SIZE(Str), pMessageStrings[MSG_EPC_CANT_TRAVERSE],
                 MercPtrs[gusSelectedSoldier]->name);

        DoMessageBox(MSG_BOX_BASIC_STYLE, Str, GAME_SCREEN, (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
        return;
      }
    }

    if (fOk) {
      // Handle the effects here!
      if (gExitDialog.fAllMove && gExitDialog.fGotoSector && gExitDialog.fGotoSectorText) {
        JumpIntoAdjacentSector(gExitDialog.ubDirection, JUMP_ALL_LOAD_NEW,
                               gExitDialog.sAdditionalData);
        return;
      }

      // KM : August 6, 1999 Patch fix
      //     Added the !gExitDialog.fGotoSectorText to the conditions to prevent the player from
      //     LOADING an adjacent sector (this only happens when instant traversal is overriden
      //     because of a battle in progress in the previous sector
      if (gExitDialog.fAllMove && (!gExitDialog.fGotoSector || !gExitDialog.fGotoSectorText)) {
        // Here, move all men out of sector but don't load new one...
        JumpIntoAdjacentSector(gExitDialog.ubDirection, JUMP_ALL_NO_LOAD,
                               gExitDialog.sAdditionalData);
      }

      if (gExitDialog.fSingleMove && gExitDialog.fGotoSector && gExitDialog.fGotoSectorText) {
        JumpIntoAdjacentSector(gExitDialog.ubDirection, JUMP_SINGLE_LOAD_NEW,
                               gExitDialog.sAdditionalData);
        return;
      }

      // KM : August 6, 1999 Patch fix
      //     Added the !gExitDialog.fGotoSectorText to the conditions to prevent the player from
      //     LOADING an adjacent sector (this only happens when instant traversal is overriden
      //     because of a battle in progress in the previous sector
      if (gExitDialog.fSingleMove && (!gExitDialog.fGotoSector || !gExitDialog.fGotoSectorText)) {
        // Here, move all men out of sector but don't load new one...
        JumpIntoAdjacentSector(gExitDialog.ubDirection, JUMP_SINGLE_NO_LOAD,
                               gExitDialog.sAdditionalData);
      }
    }
  }
}

void CheckLoadMapCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gExitDialog.fGotoSector = !gExitDialog.fGotoSector;
  }
}

void SingleMoveAction() {
  // KM: New logic Mar2 '99
  if (!gExitDialog.fMultipleSquadsInSector) {
    if (gTacticalStatus.fEnemyInSector) {  // if enemy in sector, and mercs will be left behind,
                                           // prevent user from selecting load
      gExitDialog.fGotoSectorDisabled = TRUE;
      gExitDialog.fGotoSector = FALSE;
    } else {  // freedom to load or not load
      gExitDialog.fGotoSectorDisabled = FALSE;
    }
  } else {
    gExitDialog.fGotoSector = FALSE;
  }
  gExitDialog.fSingleMove = TRUE;
  gExitDialog.fAllMove = FALSE;
  // end

  // previous logic
  /*
  gExitDialog.fGotoSector = FALSE;
  gExitDialog.fSingleMove = TRUE;
  gExitDialog.fAllMove		= FALSE;
  */
}

void AllMoveAction() {
  // KM: New logic Mar2 '99
  if (!gExitDialog.fMultipleSquadsInSector) {
    gExitDialog.fGotoSectorDisabled = TRUE;
    gExitDialog.fGotoSector = TRUE;
  }
  gExitDialog.fSingleMove = FALSE;
  gExitDialog.fAllMove = TRUE;
  // end

  // previous logic
  /*
  gExitDialog.fSingleMove = FALSE;
  gExitDialog.fAllMove		= TRUE;
  */
}

void SingleMoveCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    SingleMoveAction();
  }
}

void AllMoveCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    AllMoveAction();
  }
}

void OKCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    RemoveSectorExitMenu(TRUE);
  }
}

void CancelCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // OK, exit
    RemoveSectorExitMenu(FALSE);
  }
}

void SectorExitBackgroundCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // gMsgBox.bHandled = MSG_BOX_RETURN_NO;
  }
}

void SingleRegionCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    SingleMoveAction();
  }
}

void AllRegionCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    AllMoveAction();
  }
}

void LoadRegionCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gExitDialog.fGotoSector = !gExitDialog.fGotoSector;
  }
}

void SingleRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    gExitDialog.fSingleMoveHilighted = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    gExitDialog.fSingleMoveHilighted = FALSE;
  }
}

void AllRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    gExitDialog.fAllMoveHilighted = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    gExitDialog.fAllMoveHilighted = FALSE;
  }
}

void LoadRegionMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    gExitDialog.fGotoSectorHilighted = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    gExitDialog.fGotoSectorHilighted = FALSE;
  }
}
