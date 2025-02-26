// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/MapScreenInterfaceTownMineInfo.h"

#include "HelpScreen.h"
#include "Laptop/Finances.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/Font.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MapScreenInterfaceMapInventory.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/HandleUI.h"
#include "Tactical/TacticalSave.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/RenderDirty.h"
#include "Town.h"
#include "Utils/FontControl.h"
#include "Utils/PopUpBox.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"

#define BOX_BUTTON_WIDTH 100
#define BOX_BUTTON_HEIGHT 20

// flag to say if we are showing town/mine box at all
BOOLEAN fShowTownInfo = FALSE;

int32_t ghTownMineBox = -1;
SGPPoint TownMinePosition = {300, 150};
SGPRect TownMineDimensions = {0, 0, 240, 60};

int8_t bCurrentTownMineSectorX = 0;
int8_t bCurrentTownMineSectorY = 0;
int8_t bCurrentTownMineSectorZ = 0;

// inventory button
uint32_t guiMapButtonInventoryImage[2];
uint32_t guiMapButtonInventory[2];

uint16_t sTotalButtonWidth = 0;

extern MINE_LOCATION_TYPE gMineLocation[];
extern MINE_STATUS_TYPE gMineStatus[];
extern BOOLEAN fMapScreenBottomDirty;
// extern uint8_t gubMonsterMineInfestation[];

// create the town/mine info box
void CreateTownInfoBox(void);

// add town text to town info box
void AddTextToTownBox(void);

// add text to mine info box
void AddTextToMineBox(void);

// add text to non-town/non-mine the other boxes
void AddTextToBlankSectorBox(void);

// add "sector" line text to any popup box
void AddSectorToBox(void);

void AddCommonInfoToBox(void);

void AddItemsInSectorToBox(void);

// position town/mine info box on the screen
void PositionTownMineInfoBox(void);

// add the pop up button for the map inventory pop up activation
void AddInventoryButtonForMapPopUpBox(void);

// now remove the above button
void RemoveInventoryButtonForMapPopUpBox(void);

// callback to turn on sector invneotry list
void MapTownMineInventoryButtonCallBack(GUI_BUTTON *btn, int32_t reason);
void MapTownMineExitButtonCallBack(GUI_BUTTON *btn, int32_t reason);
void MinWidthOfTownMineInfoBox(void);

void DisplayTownInfo(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ) {
  // will display town info for a particular town

  // set current sector
  if ((bCurrentTownMineSectorX != sMapX) || (bCurrentTownMineSectorY != sMapY) ||
      (bCurrentTownMineSectorZ != bMapZ)) {
    bCurrentTownMineSectorX = (int8_t)sMapX;
    bCurrentTownMineSectorY = (int8_t)sMapY;
    bCurrentTownMineSectorZ = bMapZ;
  }

  // create destroy the box
  CreateDestroyTownInfoBox();
}

void CreateDestroyTownInfoBox(void) {
  // create destroy pop up box for town/mine info
  static BOOLEAN fCreated = FALSE;
  SGPRect pDimensions;
  SGPPoint pPosition;
  TownID bTownId = 0;

  if ((fCreated == FALSE) && (fShowTownInfo == TRUE)) {
    // create pop up box
    CreateTownInfoBox();

    // decide what kind of text to add to display

    if (bCurrentTownMineSectorZ == 0) {
      // only show the mine info when mines button is selected, otherwise we need to see the
      // sector's regular town info
      if ((IsThereAMineInThisSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY) == TRUE) &&
          fShowMineFlag) {
        AddTextToMineBox();
      } else {
        bTownId = GetTownIdForSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);

        // do we add text for the town box?
        if (bTownId != BLANK_SECTOR) {
          // add text for town box
          AddTextToTownBox();
        } else {
          // just a blank sector (handles SAM sites if visible)
          AddTextToBlankSectorBox();
        }
      }

      // add "militia", "militia training", "control" "enemy forces", etc. lines text to any popup
      // box
      AddCommonInfoToBox();
    } else  // underground
    {
      // sector
      AddSectorToBox();
    }

    AddItemsInSectorToBox();

    // set font type
    SetBoxFont(ghTownMineBox, BLOCKFONT2);

    // set highlight color
    SetBoxHighLight(ghTownMineBox, FONT_WHITE);

    SetBoxSecondColumnForeground(ghTownMineBox, FONT_WHITE);
    SetBoxSecondColumnBackground(ghTownMineBox, FONT_BLACK);
    SetBoxSecondColumnHighLight(ghTownMineBox, FONT_WHITE);
    SetBoxSecondColumnShade(ghTownMineBox, FONT_BLACK);
    SetBoxSecondColumnFont(ghTownMineBox, BLOCKFONT2);
    SetBoxSecondColumnMinimumOffset(ghTownMineBox, 20);

    // unhighlighted color
    SetBoxForeground(ghTownMineBox, FONT_YELLOW);

    // background color
    SetBoxBackground(ghTownMineBox, FONT_BLACK);

    // shaded color..for darkened text
    SetBoxShade(ghTownMineBox, FONT_BLACK);

    // give title line (0) different color from the rest
    SetBoxLineForeground(ghTownMineBox, 0, FONT_LTGREEN);

    // ressize box to text
    ResizeBoxToText(ghTownMineBox);

    // make box bigger to this size
    GetBoxSize(ghTownMineBox, &pDimensions);

    if (pDimensions.iRight < BOX_BUTTON_WIDTH) {
      // resize box to fit button
      pDimensions.iRight += BOX_BUTTON_WIDTH;
    }

    pDimensions.iBottom += BOX_BUTTON_HEIGHT;

    SetBoxSize(ghTownMineBox, pDimensions);

    ShowBox(ghTownMineBox);

    // now position box
    MinWidthOfTownMineInfoBox();
    PositionTownMineInfoBox();

    // now add the button
    AddInventoryButtonForMapPopUpBox();

    // now position box
    PositionTownMineInfoBox();

    fCreated = TRUE;
  } else if ((fCreated == TRUE) && (fShowTownInfo == FALSE)) {
    // get box size
    GetBoxSize(ghTownMineBox, &pDimensions);

    // get position
    GetBoxPosition(ghTownMineBox, &pPosition);

    // destroy pop up box
    RemoveBox(ghTownMineBox);
    ghTownMineBox = -1;

    // remove inventory button
    RemoveInventoryButtonForMapPopUpBox();

    // restore background
    RestoreExternBackgroundRect((int16_t)pPosition.iX, (int16_t)pPosition.iY,
                                (int16_t)(pDimensions.iRight - pDimensions.iLeft),
                                (int16_t)(pDimensions.iBottom - pDimensions.iTop + 3));

    fCreated = FALSE;
  }

  return;
}

void CreateTownInfoBox(void) {
  // create basic box
  CreatePopUpBox(&ghTownMineBox, TownMineDimensions, TownMinePosition, (POPUP_BOX_FLAG_CLIP_TEXT));

  // which buffer will box render to
  SetBoxBuffer(ghTownMineBox, vsFB);

  // border type?
  SetBorderType(ghTownMineBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghTownMineBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghTownMineBox, 6, 6, 8, 6);

  // space between lines
  SetLineSpace(ghTownMineBox, 2);

  // set current box to this one
  SetCurrentBox(ghTownMineBox);

  return;
}

// adds text to town info box
void AddTextToTownBox(void) {
  uint32_t hStringHandle = 0;
  wchar_t wString[64];
  uint8_t ubTownId = 0;
  uint16_t usTownSectorIndex;
  int16_t sMineSector = 0;

  // remember town id
  ubTownId = GetTownIdForSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
  Assert((ubTownId >= FIRST_TOWN) && (ubTownId < NUM_TOWNS));

  usTownSectorIndex = GetSectorID8(bCurrentTownMineSectorX, bCurrentTownMineSectorY);

  switch (usTownSectorIndex) {
    case SEC_B13:
      AddMonoString(&hStringHandle, pLandTypeStrings[DRASSEN_AIRPORT_SITE]);
      break;
    case SEC_F8:
      AddMonoString(&hStringHandle, pLandTypeStrings[CAMBRIA_HOSPITAL_SITE]);
      break;
    case SEC_J9:  // Tixa
      if (!fFoundTixa)
        AddMonoString(&hStringHandle, pLandTypeStrings[SAND]);
      else
        AddMonoString(&hStringHandle, pTownNames[TIXA]);
      break;
    case SEC_K4:  // Orta
      if (!fFoundOrta)
        AddMonoString(&hStringHandle, pLandTypeStrings[SWAMP]);
      else
        AddMonoString(&hStringHandle, pTownNames[ORTA]);
      break;
    case SEC_N3:
      AddMonoString(&hStringHandle, pLandTypeStrings[MEDUNA_AIRPORT_SITE]);
      break;
    default:
      if (usTownSectorIndex == SEC_N4 && fSamSiteFound[SAM_SITE_FOUR]) {  // Meduna's SAM site
        AddMonoString(&hStringHandle, pLandTypeStrings[MEDUNA_SAM_SITE]);
      } else {  // town name
        swprintf(wString, ARR_SIZE(wString), L"%s", pTownNames[ubTownId]);
        AddMonoString(&hStringHandle, wString);
      }
      break;
  }
  // blank line
  AddMonoString(&hStringHandle, L"");

  // sector
  AddSectorToBox();

  // town size
  swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[0]);
  AddMonoString(&hStringHandle, wString);
  swprintf(wString, ARR_SIZE(wString), L"%d", GetTownSectorSize(ubTownId));
  AddSecondColumnMonoString(&hStringHandle, wString);

  // main facilities
  swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[8]);
  AddMonoString(&hStringHandle, wString);
  wcscpy(wString, L"");
  GetSectorFacilitiesFlags(bCurrentTownMineSectorX, bCurrentTownMineSectorY, wString,
                           ARR_SIZE(wString));
  AddSecondColumnMonoString(&hStringHandle, wString);

  // the concept of control is only meaningful in sectors where militia can be trained
  if (MilitiaTrainingAllowedInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY, 0)) {
    // town control
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[2]);
    AddMonoString(&hStringHandle, wString);
    swprintf(wString, ARR_SIZE(wString), L"%d%%%%",
             (GetTownSectorsUnderControl(ubTownId) * 100) / GetTownSectorSize(ubTownId));
    AddSecondColumnMonoString(&hStringHandle, wString);
  }

  // the concept of town loyalty is only meaningful in towns where loyalty is tracked
  if (gTownLoyalty[ubTownId].fStarted && gfTownUsesLoyalty[ubTownId]) {
    // town loyalty
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[5]);
    AddMonoString(&hStringHandle, wString);
    swprintf(wString, ARR_SIZE(wString), L"%d%%%%", gTownLoyalty[ubTownId].ubRating);
    AddSecondColumnMonoString(&hStringHandle, wString);
  }

  // if the town has a mine
  sMineSector = GetMineSectorForTown(ubTownId);
  if (sMineSector != -1) {
    // Associated Mine: Sector
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[4]);
    AddMonoString(&hStringHandle, wString);
    GetShortSectorString(SectorID16_X(sMineSector), SectorID16_Y(sMineSector), wString,
                         ARR_SIZE(wString));
    AddSecondColumnMonoString(&hStringHandle, wString);
  }
}

// adds text to mine info box
void AddTextToMineBox(void) {
  uint8_t ubMineIndex;
  uint8_t ubTown;
  uint32_t hStringHandle;
  wchar_t wString[64];

  ubMineIndex = GetMineIndexForSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);

  // name of town followed by "mine"
  swprintf(wString, ARR_SIZE(wString), L"%s %s", pTownNames[GetTownAssociatedWithMine(ubMineIndex)],
           pwMineStrings[0]);
  AddMonoString(&hStringHandle, wString);

  // blank line
  AddMonoString(&hStringHandle, L"");

  // sector
  AddSectorToBox();

  // mine status
  swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[9]);
  AddMonoString(&hStringHandle, wString);

  // check if mine is empty (abandoned) or running out
  if (gMineStatus[ubMineIndex].fEmpty) {
    // abandonded
    wcscpy(wString, pwMineStrings[5]);
  } else if (gMineStatus[ubMineIndex].fShutDown) {
    // shut down
    wcscpy(wString, pwMineStrings[6]);
  } else if (gMineStatus[ubMineIndex].fRunningOut) {
    // running out
    wcscpy(wString, pwMineStrings[7]);
  } else {
    // producing
    wcscpy(wString, pwMineStrings[8]);
  }
  AddSecondColumnMonoString(&hStringHandle, wString);

  // if still producing
  if (!gMineStatus[ubMineIndex].fEmpty) {
    // current production
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[3]);
    AddMonoString(&hStringHandle, wString);

    swprintf(wString, ARR_SIZE(wString), L"%d", PredictDailyIncomeFromAMine(ubMineIndex));
    InsertCommasForDollarFigure(wString);
    InsertDollarSignInToString(wString);
    AddSecondColumnMonoString(&hStringHandle, wString);

    // potential production
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[4]);
    AddMonoString(&hStringHandle, wString);

    swprintf(wString, ARR_SIZE(wString), L"%d", GetMaxDailyRemovalFromMine(ubMineIndex));
    InsertCommasForDollarFigure(wString);
    InsertDollarSignInToString(wString);
    AddSecondColumnMonoString(&hStringHandle, wString);

    // if potential is not nil
    if (GetMaxPeriodicRemovalFromMine(ubMineIndex) > 0) {
      // production rate (current production as a percentage of potential production)
      swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[10]);
      AddMonoString(&hStringHandle, wString);
      swprintf(wString, ARR_SIZE(wString), L"%d%%%%",
               (PredictDailyIncomeFromAMine(ubMineIndex) * 100) /
                   GetMaxDailyRemovalFromMine(ubMineIndex));
      AddSecondColumnMonoString(&hStringHandle, wString);
    }

    // town control percentage
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[12]);
    AddMonoString(&hStringHandle, wString);
    swprintf(wString, ARR_SIZE(wString), L"%d%%%%",
             (GetTownSectorsUnderControl(gMineLocation[ubMineIndex].bAssociatedTown) * 100) /
                 GetTownSectorSize(gMineLocation[ubMineIndex].bAssociatedTown));
    AddSecondColumnMonoString(&hStringHandle, wString);

    ubTown = gMineLocation[ubMineIndex].bAssociatedTown;
    if (gTownLoyalty[ubTown].fStarted && gfTownUsesLoyalty[ubTown]) {
      // town loyalty percentage
      swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[13]);
      AddMonoString(&hStringHandle, wString);
      swprintf(wString, ARR_SIZE(wString), L"%d%%%%",
               gTownLoyalty[gMineLocation[ubMineIndex].bAssociatedTown].ubRating);
      AddSecondColumnMonoString(&hStringHandle, wString);
    }

    /* gradual monster infestation concept was ditched, now simply IN PRODUCTION or SHUT DOWN
                    // percentage of miners working
                    swprintf( wString, L"%s:", pwMineStrings[ 14 ]);
                    AddMonoString( &hStringHandle, wString );
                    swprintf( wString, L"%d%%%%", gubMonsterMineInfestation[ gMineStatus[
       ubMineIndex ].bMonsters ]); AddSecondColumnMonoString( &hStringHandle, wString );
    */

    // ore type (silver/gold
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwMineStrings[11]);
    AddMonoString(&hStringHandle, wString);
    AddSecondColumnMonoString(&hStringHandle, (gMineStatus[ubMineIndex].ubMineType == SILVER_MINE)
                                                  ? pwMineStrings[1]
                                                  : pwMineStrings[2]);
  }

#ifdef _DEBUG
  // dollar amount remaining in mine
  wcscpy(wString, L"Remaining (DEBUG):");
  AddMonoString(&hStringHandle, wString);

  swprintf(wString, ARR_SIZE(wString), L"%d", GetTotalLeftInMine(ubMineIndex));
  InsertCommasForDollarFigure(wString);
  InsertDollarSignInToString(wString);
  AddSecondColumnMonoString(&hStringHandle, wString);
#endif
}

void AddTextToBlankSectorBox(void) {
  uint32_t hStringHandle;
  uint16_t usSectorValue = 0;

  // get the sector value
  usSectorValue = GetSectorID8(bCurrentTownMineSectorX, bCurrentTownMineSectorY);

  switch (usSectorValue) {
    case SEC_D2:  // Chitzena SAM
      if (!fSamSiteFound[SAM_SITE_ONE])
        AddMonoString(&hStringHandle, pLandTypeStrings[TROPICS]);
      else
        AddMonoString(&hStringHandle, pLandTypeStrings[TROPICS_SAM_SITE]);
      break;
    case SEC_D15:  // Drassen SAM
      if (!fSamSiteFound[SAM_SITE_TWO])
        AddMonoString(&hStringHandle, pLandTypeStrings[SPARSE]);
      else
        AddMonoString(&hStringHandle, pLandTypeStrings[SPARSE_SAM_SITE]);
      break;
    case SEC_I8:  // Cambria SAM
      if (!fSamSiteFound[SAM_SITE_THREE])
        AddMonoString(&hStringHandle, pLandTypeStrings[SAND]);
      else
        AddMonoString(&hStringHandle, pLandTypeStrings[SAND_SAM_SITE]);
      break;
      // SAM Site 4 in Meduna is within town limits, so it's handled in AddTextToTownBox()

    default:
      AddMonoString(&hStringHandle,
                    pLandTypeStrings[(SectorInfo[usSectorValue].ubTraversability[4])]);
      break;
  }

  // blank line
  AddMonoString(&hStringHandle, L"");

  // sector
  AddSectorToBox();
}

void AddSectorToBox(void) {
  wchar_t wString[64];
  wchar_t wString2[10];
  uint32_t hStringHandle = 0;

  // sector
  swprintf(wString, ARR_SIZE(wString), L"%s:", pwMiscSectorStrings[1]);
  AddMonoString(&hStringHandle, wString);

  GetShortSectorString(bCurrentTownMineSectorX, bCurrentTownMineSectorY, wString,
                       ARR_SIZE(wString));
  if (bCurrentTownMineSectorZ != 0) {
    swprintf(wString2, ARR_SIZE(wString2), L"-%d", bCurrentTownMineSectorZ);
    wcscat(wString, wString2);
  }

  AddSecondColumnMonoString(&hStringHandle, wString);
}

void AddCommonInfoToBox(void) {
  wchar_t wString[64];
  uint32_t hStringHandle = 0;
  BOOLEAN fUnknownSAMSite = FALSE;
  uint8_t ubNumEnemies;

  switch (GetSectorID8(bCurrentTownMineSectorX, bCurrentTownMineSectorY)) {
    case SEC_D2:  // Chitzena SAM
      if (!fSamSiteFound[SAM_SITE_ONE]) fUnknownSAMSite = TRUE;
      break;
    case SEC_D15:  // Drassen SAM
      if (!fSamSiteFound[SAM_SITE_TWO]) fUnknownSAMSite = TRUE;
      break;
    case SEC_I8:  // Cambria SAM
      if (!fSamSiteFound[SAM_SITE_THREE]) fUnknownSAMSite = TRUE;
      break;
    // SAM Site 4 in Meduna is within town limits, so it's always controllable
    default:
      break;
  }

  // in sector where militia can be trained,
  // control of the sector matters, display who controls this sector.  Map brightness no longer
  // gives this!
  if (MilitiaTrainingAllowedInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY, 0) &&
      !fUnknownSAMSite) {
    // controlled:
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwMiscSectorStrings[4]);
    AddMonoString(&hStringHandle, wString);

    // No/Yes
    swprintf(wString, ARR_SIZE(wString), L"%s",
             pwMiscSectorStrings[(StrategicMap[GetSectorID16(bCurrentTownMineSectorX,
                                                             bCurrentTownMineSectorY)]
                                      .fEnemyControlled)
                                     ? 6
                                     : 5]);
    AddSecondColumnMonoString(&hStringHandle, wString);

    // militia - is there any?
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[11]);
    AddMonoString(&hStringHandle, wString);

    uint8_t ubMilitiaTotal =
        CountAllMilitiaInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
    if (ubMilitiaTotal > 0) {
      // some militia, show total & their breakdown by level
      struct MilitiaCount milCount =
          GetMilitiaInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);
      swprintf(wString, ARR_SIZE(wString), L"%d  (%d/%d/%d)", ubMilitiaTotal, milCount.green,
               milCount.regular, milCount.elite);
      AddSecondColumnMonoString(&hStringHandle, wString);
    } else {
      // no militia: don't bother displaying level breakdown
      wcscpy(wString, L"0");
      AddSecondColumnMonoString(&hStringHandle, wString);
    }

    // percentage of current militia squad training completed
    swprintf(wString, ARR_SIZE(wString), L"%s:", pwTownInfoStrings[10]);
    AddMonoString(&hStringHandle, wString);
    swprintf(wString, ARR_SIZE(wString), L"%d%%%%",
             SectorInfo[GetSectorID8(bCurrentTownMineSectorX, bCurrentTownMineSectorY)]
                 .ubMilitiaTrainingPercentDone);
    AddSecondColumnMonoString(&hStringHandle, wString);
  }

  // enemy forces
  swprintf(wString, ARR_SIZE(wString), L"%s:", pwMiscSectorStrings[0]);
  AddMonoString(&hStringHandle, wString);

  // how many are there, really?
  ubNumEnemies = NumEnemiesInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY);

  switch (WhatPlayerKnowsAboutEnemiesInSector(bCurrentTownMineSectorX, bCurrentTownMineSectorY)) {
    case KNOWS_NOTHING:
      // show "Unknown"
      wcscpy(wString, pwMiscSectorStrings[3]);
      break;

    case KNOWS_THEYRE_THERE:
      // if there are any there
      if (ubNumEnemies > 0) {
        // show "?", but not exactly how many
        wcscpy(wString, L"?");
      } else {
        // we know there aren't any (or we'd be seing them on map, too)
        wcscpy(wString, L"0");
      }
      break;

    case KNOWS_HOW_MANY:
      // show exactly how many
      swprintf(wString, ARR_SIZE(wString), L"%d", ubNumEnemies);
      break;
  }

  AddSecondColumnMonoString(&hStringHandle, wString);
}

void AddItemsInSectorToBox(void) {
  wchar_t wString[64];
  uint32_t hStringHandle = 0;

  // items in sector (this works even for underground)

  swprintf(wString, ARR_SIZE(wString), L"%s:", pwMiscSectorStrings[2]);
  AddMonoString(&hStringHandle, wString);

  //	swprintf( wString, L"%d", GetSizeOfStashInSector( bCurrentTownMineSectorX,
  // bCurrentTownMineSectorY, bCurrentTownMineSectorZ, FALSE ));
  swprintf(wString, ARR_SIZE(wString), L"%d",
           GetNumberOfVisibleWorldItemsFromSectorStructureForSector(
               bCurrentTownMineSectorX, bCurrentTownMineSectorY, bCurrentTownMineSectorZ));
  AddSecondColumnMonoString(&hStringHandle, wString);
}

void PositionTownMineInfoBox(void) {
  // position town mine info box
  SGPRect pDimensions;
  SGPPoint pPosition;
  int16_t sX = 0, sY = 0;

  // position the box based on x and y of the selected sector
  GetScreenXYFromMapXY(bCurrentTownMineSectorX, bCurrentTownMineSectorY, &sX, &sY);

  // set box position
  pPosition.iX = sX;
  pPosition.iY = sY;

  // set new position
  SetBoxPosition(ghTownMineBox, pPosition);

  // get box size
  GetBoxSize(ghTownMineBox, &pDimensions);

  // get position
  GetBoxPosition(ghTownMineBox, &pPosition);

  if (pDimensions.iRight < (sTotalButtonWidth + 30)) {
    SpecifyBoxMinWidth(ghTownMineBox, (sTotalButtonWidth + 30));
    pDimensions.iRight = sTotalButtonWidth + 30;
  }

  // now position box - the x axis
  if (pPosition.iX < MapScreenRect.iLeft) {
    pPosition.iX = MapScreenRect.iLeft + 5;
  }

  if (pPosition.iX + pDimensions.iRight > MapScreenRect.iRight) {
    pPosition.iX = MapScreenRect.iRight - pDimensions.iRight - 5;
  }

  // position - the y axis
  if (pPosition.iY < MapScreenRect.iTop) {
    pPosition.iY = MapScreenRect.iTop + 5;
  }

  if (pPosition.iY + pDimensions.iBottom > MapScreenRect.iBottom) {
    pPosition.iY = MapScreenRect.iBottom - pDimensions.iBottom - 8;
  }

  // reset position
  SetBoxPosition(ghTownMineBox, pPosition);

  return;
}

void AddInventoryButtonForMapPopUpBox(void) {
  int16_t sX, sY;
  SGPRect pDimensions;
  SGPPoint pPosition;
  uint32_t uiObject;
  ETRLEObject *pTrav;
  int16_t sWidthA = 0, sTotalBoxWidth = 0;
  struct VObject *hHandle;

  // load the button
  AddVObject(CreateVObjectFromFile("INTERFACE\\mapinvbtns.sti"), &uiObject);

  // Calculate smily face positions...
  GetVideoObject(&hHandle, uiObject);
  pTrav = &(hHandle->pETRLEObject[0]);

  sWidthA = pTrav->usWidth;

  pTrav = &(hHandle->pETRLEObject[1]);

  sTotalBoxWidth = sTotalButtonWidth;

  GetBoxSize(ghTownMineBox, &pDimensions);
  GetBoxPosition(ghTownMineBox, &pPosition);

  sX = pPosition.iX + (pDimensions.iRight - sTotalBoxWidth) / 3;
  sY = pPosition.iY + pDimensions.iBottom - ((BOX_BUTTON_HEIGHT + 5));

  guiMapButtonInventoryImage[0] = LoadButtonImage("INTERFACE\\mapinvbtns.sti", -1, 0, -1, 2, -1);

  guiMapButtonInventory[0] = CreateIconAndTextButton(
      guiMapButtonInventoryImage[0], pMapPopUpInventoryText[0], BLOCKFONT2, FONT_WHITE, FONT_BLACK,
      FONT_WHITE, FONT_BLACK, TEXT_CJUSTIFIED, (int16_t)(sX), (int16_t)(sY), BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 1, DEFAULT_MOVE_CALLBACK,
      (GUI_CALLBACK)MapTownMineInventoryButtonCallBack);

  sX = sX + sWidthA + (pDimensions.iRight - sTotalBoxWidth) / 3;
  sY = pPosition.iY + pDimensions.iBottom - ((BOX_BUTTON_HEIGHT + 5));

  guiMapButtonInventoryImage[1] = LoadButtonImage("INTERFACE\\mapinvbtns.sti", -1, 1, -1, 3, -1);

  guiMapButtonInventory[1] = CreateIconAndTextButton(
      guiMapButtonInventoryImage[1], pMapPopUpInventoryText[1], BLOCKFONT2, FONT_WHITE, FONT_BLACK,
      FONT_WHITE, FONT_BLACK, TEXT_CJUSTIFIED, (int16_t)(sX), (int16_t)(sY), BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 1, DEFAULT_MOVE_CALLBACK,
      (GUI_CALLBACK)MapTownMineExitButtonCallBack);

  // delete video object
  DeleteVideoObjectFromIndex(uiObject);

  /*
          // if below ground disable
          if( iCurrentMapSectorZ )
          {
                  DisableButton( guiMapButtonInventory[ 0 ] );
          }
  */

  return;
}

void RemoveInventoryButtonForMapPopUpBox(void) {
  // get rid of button
  RemoveButton(guiMapButtonInventory[0]);
  UnloadButtonImage(guiMapButtonInventoryImage[0]);

  RemoveButton(guiMapButtonInventory[1]);
  UnloadButtonImage(guiMapButtonInventoryImage[1]);

  return;
}

void MapTownMineInventoryButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // done
      fShowMapInventoryPool = TRUE;
      MarkForRedrawalStrategicMap();
      fMapScreenBottomDirty = TRUE;
      fShowTownInfo = FALSE;

      // since we are bring up the sector inventory, check to see if the help screen should come up
      if (ShouldTheHelpScreenComeUp(HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY, FALSE)) {
        // normally this is handled in the screen handler, we have to set up a little different this
        // time around
        ShouldTheHelpScreenComeUp(HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY, TRUE);
      }
    }
  }
}

void MapTownMineExitButtonCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // done
      MarkForRedrawalStrategicMap();
      fMapScreenBottomDirty = TRUE;
      fShowTownInfo = FALSE;
    }
  }
}

// get the min width of the town mine info pop up box
void MinWidthOfTownMineInfoBox(void) {
  struct VObject *hHandle;
  int16_t sWidthA = 0, sWidthB = 0, sTotalBoxWidth = 0;
  uint32_t uiObject;
  ETRLEObject *pTrav;

  AddVObject(CreateVObjectFromFile("INTERFACE\\mapinvbtns.sti"), &uiObject);

  // Calculate smily face positions...
  GetVideoObject(&hHandle, uiObject);
  pTrav = &(hHandle->pETRLEObject[0]);

  sWidthA = pTrav->usWidth;

  pTrav = &(hHandle->pETRLEObject[1]);
  sWidthB = pTrav->usWidth;

  sTotalBoxWidth = sWidthA + sWidthB;
  sTotalButtonWidth = sTotalBoxWidth;

  // delete video object
  DeleteVideoObjectFromIndex(uiObject);
}
