// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/TacticalPlacementGUI.h"

#include "GameLoop.h"
#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Interface.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/MapEdgepoints.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/WordWrap.h"

typedef struct MERCPLACEMENT {
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiVObjectID;
  struct MOUSE_REGION region;
  uint8_t ubStrategicInsertionCode;
  BOOLEAN fPlaced;
} MERCPLACEMENT;

MERCPLACEMENT *gMercPlacement = NULL;

enum { DONE_BUTTON, SPREAD_BUTTON, GROUP_BUTTON, CLEAR_BUTTON, NUM_TP_BUTTONS };
uint32_t iTPButtons[NUM_TP_BUTTONS];

extern BOOLEAN gfOverheadMapDirty;
extern BOOLEAN GetOverheadMouseGridNo(int16_t *psGridNo);

uint8_t gubDefaultButton = CLEAR_BUTTON;
BOOLEAN gfTacticalPlacementGUIActive = FALSE;
BOOLEAN gfTacticalPlacementFirstTime = FALSE;
BOOLEAN gfEnterTacticalPlacementGUI = FALSE;
BOOLEAN gfKillTacticalGUI = FALSE;
uint32_t giOverheadPanelImage = 0;
int32_t giOverheadButtonImages[NUM_TP_BUTTONS];
uint32_t giMercPanelImage = 0;
int32_t giPlacements = 0;
BOOLEAN gfTacticalPlacementGUIDirty = FALSE;
BOOLEAN gfValidLocationsChanged = FALSE;
SGPRect gTPClipRect = {0, 0, 0, 0};
BOOLEAN gfValidCursor = FALSE;
BOOLEAN gfEveryonePlaced = FALSE;

uint8_t gubSelectedGroupID = 0;
uint8_t gubHilightedGroupID = 0;
uint8_t gubCursorGroupID = 0;
int8_t gbSelectedMercID = -1;
int8_t gbHilightedMercID = -1;
int8_t gbCursorMercID = -1;
struct SOLDIERTYPE *gpTacticalPlacementSelectedSoldier = NULL;
struct SOLDIERTYPE *gpTacticalPlacementHilightedSoldier = NULL;

BOOLEAN gfNorth, gfEast, gfSouth, gfWest;

void DoneOverheadPlacementClickCallback(GUI_BUTTON *btn, int32_t reason);
void SpreadPlacementsCallback(GUI_BUTTON *btn, int32_t reason);
void GroupPlacementsCallback(GUI_BUTTON *btn, int32_t reason);
void ClearPlacementsCallback(GUI_BUTTON *btn, int32_t reason);
void MercMoveCallback(struct MOUSE_REGION *reg, int32_t reason);
void MercClickCallback(struct MOUSE_REGION *reg, int32_t reason);
void PlaceMercs();
void FastHelpRemovedCallback();
void FastHelpRemoved2Callback();
void DialogRemoved(uint8_t ubResult);

void PutDownMercPiece(int32_t iPlacement);
void PickUpMercPiece(int32_t iPlacement);
void SetCursorMerc(int8_t bPlacementID);
void SelectNextUnplacedUnit();

#ifdef JA2BETAVERSION

BOOLEAN gfNorthValid, gfEastValid, gfSouthValid, gfWestValid;
BOOLEAN gfChangedEntrySide = FALSE;

void FindValidInsertionCode(uint8_t *pubStrategicInsertionCode) {
  if (gMapInformation.sNorthGridNo == -1 && gMapInformation.sEastGridNo == -1 &&
      gMapInformation.sSouthGridNo == -1 && gMapInformation.sWestGridNo == -1) {
    AssertMsg(0, "Map has no entry points at all.  Can't generate edge points.  LC:1");
  }
  if (gMapInformation.sNorthGridNo != -1 && !gps1stNorthEdgepointArray ||
      gMapInformation.sEastGridNo != -1 && !gps1stEastEdgepointArray ||
      gMapInformation.sSouthGridNo != -1 && !gps1stSouthEdgepointArray ||
      gMapInformation.sWestGridNo != -1 && !gps1stWestEdgepointArray) {
    InvalidateScreen();
    DrawTextToScreen(L"Map doesn't has entrypoints without corresponding edgepoints. LC:1", 30, 150,
                     600, FONT10ARIALBOLD, FONT_RED, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED);
    DrawTextToScreen(L"GENERATING MAP EDGEPOINTS!  Please wait...", 30, 160, 600, FONT10ARIALBOLD,
                     FONT_YELLOW, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED);

    RefreshScreen(NULL);
    GenerateMapEdgepoints();
    switch (*pubStrategicInsertionCode) {
      case INSERTION_CODE_NORTH:
        if (!gps1stNorthEdgepointArray)
          AssertMsg(0, "Map Edgepoint generation failed.  KM : 0 -- send map");
        break;
      case INSERTION_CODE_EAST:
        if (!gps1stEastEdgepointArray)
          AssertMsg(0, "Map Edgepoint generation failed.  KM : 0 -- send map");
        break;
      case INSERTION_CODE_SOUTH:
        if (!gps1stSouthEdgepointArray)
          AssertMsg(0, "Map Edgepoint generation failed.  KM : 0 -- send map");
        break;
      case INSERTION_CODE_WEST:
        if (!gps1stWestEdgepointArray)
          AssertMsg(0, "Map Edgepoint generation failed.  KM : 0 -- send map");
        break;
    }
    return;
  }
  if (gMapInformation.sNorthGridNo != -1) {
    *pubStrategicInsertionCode = INSERTION_CODE_NORTH;
    gfChangedEntrySide = TRUE;
  } else if (gMapInformation.sEastGridNo != -1) {
    *pubStrategicInsertionCode = INSERTION_CODE_EAST;
    gfChangedEntrySide = TRUE;
  } else if (gMapInformation.sSouthGridNo != -1) {
    *pubStrategicInsertionCode = INSERTION_CODE_SOUTH;
    gfChangedEntrySide = TRUE;
  } else if (gMapInformation.sWestGridNo != -1) {
    *pubStrategicInsertionCode = INSERTION_CODE_WEST;
    gfChangedEntrySide = TRUE;
  } else {
    AssertMsg(0, "No maps edgepoints at all! KM, LC : 1");
  }
}

void CheckForValidMapEdge(uint8_t *pubStrategicInsertionCode) {
  switch (*pubStrategicInsertionCode) {
    case INSERTION_CODE_NORTH:
      if (!gps1stNorthEdgepointArray) FindValidInsertionCode(pubStrategicInsertionCode);
      break;
    case INSERTION_CODE_EAST:
      if (!gps1stEastEdgepointArray) FindValidInsertionCode(pubStrategicInsertionCode);
      break;
    case INSERTION_CODE_SOUTH:
      if (!gps1stSouthEdgepointArray) FindValidInsertionCode(pubStrategicInsertionCode);
      break;
    case INSERTION_CODE_WEST:
      if (!gps1stWestEdgepointArray) FindValidInsertionCode(pubStrategicInsertionCode);
      break;
  }
}

#endif

void InitTacticalPlacementGUI() {
  int32_t i, xp, yp;
  uint8_t ubFaceIndex;
  gfTacticalPlacementGUIActive = TRUE;
  gfTacticalPlacementGUIDirty = TRUE;
  gfValidLocationsChanged = TRUE;
  gfTacticalPlacementFirstTime = TRUE;
  gfNorth = gfEast = gfSouth = gfWest = FALSE;
#ifdef JA2BETAVERSION
  gfNorthValid = gfEastValid = gfSouthValid = gfWestValid = FALSE;
  gfChangedEntrySide = FALSE;
#endif

  // Enter overhead map
  GoIntoOverheadMap();

  // Load the images
  if (!AddVObject(CreateVObjectFromFile("Interface\\OverheadInterface.sti"),
                  &giOverheadPanelImage)) {
    AssertMsg(0, "Failed to load Interface\\OverheadInterface.sti");
  }
  if (!AddVObject(CreateVObjectFromFile("Interface\\panels.sti"), &giMercPanelImage)) {
    AssertMsg(0, "Failed to load Interface\\panels.sti");
  }

  giOverheadButtonImages[DONE_BUTTON] =
      LoadButtonImage("Interface\\OverheadUIButtons.sti", -1, 0, -1, 1, -1);
  if (giOverheadButtonImages[DONE_BUTTON] == -1) {
    AssertMsg(0, "Failed to load Interface\\OverheadUIButtons.sti");
  }
  giOverheadButtonImages[SPREAD_BUTTON] =
      UseLoadedButtonImage(giOverheadButtonImages[DONE_BUTTON], -1, 0, -1, 1, -1);
  giOverheadButtonImages[GROUP_BUTTON] =
      UseLoadedButtonImage(giOverheadButtonImages[DONE_BUTTON], -1, 0, -1, 1, -1);
  giOverheadButtonImages[CLEAR_BUTTON] =
      UseLoadedButtonImage(giOverheadButtonImages[DONE_BUTTON], -1, 0, -1, 1, -1);

  // Create the buttons which provide automatic placements.
  iTPButtons[CLEAR_BUTTON] =
      QuickCreateButton(giOverheadButtonImages[CLEAR_BUTTON], 11, 332, BUTTON_NO_TOGGLE,
                        MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, ClearPlacementsCallback);
  SpecifyGeneralButtonTextAttributes(iTPButtons[CLEAR_BUTTON], gpStrategicString[STR_TP_CLEAR],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SetButtonFastHelpText(iTPButtons[CLEAR_BUTTON], gpStrategicString[STR_TP_CLEARHELP]);
  SetBtnHelpEndCallback(iTPButtons[CLEAR_BUTTON], FastHelpRemoved2Callback);
  iTPButtons[SPREAD_BUTTON] =
      QuickCreateButton(giOverheadButtonImages[SPREAD_BUTTON], 11, 367, BUTTON_NO_TOGGLE,
                        MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, SpreadPlacementsCallback);
  SpecifyGeneralButtonTextAttributes(iTPButtons[SPREAD_BUTTON], gpStrategicString[STR_TP_SPREAD],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SetButtonFastHelpText(iTPButtons[SPREAD_BUTTON], gpStrategicString[STR_TP_SPREADHELP]);
  SetBtnHelpEndCallback(iTPButtons[SPREAD_BUTTON], FastHelpRemovedCallback);
  iTPButtons[GROUP_BUTTON] =
      QuickCreateButton(giOverheadButtonImages[GROUP_BUTTON], 11, 402, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, GroupPlacementsCallback);
  SpecifyGeneralButtonTextAttributes(iTPButtons[GROUP_BUTTON], gpStrategicString[STR_TP_GROUP],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SetButtonFastHelpText(iTPButtons[GROUP_BUTTON], gpStrategicString[STR_TP_GROUPHELP]);
  SetBtnHelpEndCallback(iTPButtons[GROUP_BUTTON], FastHelpRemovedCallback);
  iTPButtons[DONE_BUTTON] = QuickCreateButton(
      giOverheadButtonImages[DONE_BUTTON], 11, 437, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, DoneOverheadPlacementClickCallback);
  SpecifyGeneralButtonTextAttributes(iTPButtons[DONE_BUTTON], gpStrategicString[STR_TP_DONE],
                                     BLOCKFONT, FONT_BEIGE, 141);
  SetButtonFastHelpText(iTPButtons[DONE_BUTTON], gpStrategicString[STR_TP_DONEHELP]);
  SetBtnHelpEndCallback(iTPButtons[DONE_BUTTON], FastHelpRemovedCallback);
  AllowDisabledButtonFastHelp(iTPButtons[DONE_BUTTON], TRUE);

  SpecifyButtonHilitedTextColors(iTPButtons[CLEAR_BUTTON], FONT_WHITE, FONT_NEARBLACK);
  SpecifyButtonHilitedTextColors(iTPButtons[SPREAD_BUTTON], FONT_WHITE, FONT_NEARBLACK);
  SpecifyButtonHilitedTextColors(iTPButtons[GROUP_BUTTON], FONT_WHITE, FONT_NEARBLACK);
  SpecifyButtonHilitedTextColors(iTPButtons[DONE_BUTTON], FONT_WHITE, FONT_NEARBLACK);

  // First pass:  Count the number of mercs that are going to be placed by the player.
  //             This determines the size of the array we will allocate.
  giPlacements = 0;
  for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
       i++) {
    if (MercPtrs[i]->bActive && !MercPtrs[i]->fBetweenSectors &&
        MercPtrs[i]->sSectorX == gpBattleGroup->ubSectorX &&
        MercPtrs[i]->sSectorY == gpBattleGroup->ubSectorY &&
        !(MercPtrs[i]->uiStatusFlags & (SOLDIER_VEHICLE)) &&  // ATE Ignore vehicles
        MercPtrs[i]->bAssignment != ASSIGNMENT_POW && MercPtrs[i]->bAssignment != IN_TRANSIT &&
        !MercPtrs[i]->bSectorZ) {
      giPlacements++;
    }
  }
  // Allocate the array based on how many mercs there are.
  gMercPlacement = (MERCPLACEMENT *)MemAlloc(sizeof(MERCPLACEMENT) * giPlacements);
  Assert(gMercPlacement);
  // Second pass:  Assign the mercs to their respective slots.
  giPlacements = 0;
  for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
       i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife && !MercPtrs[i]->fBetweenSectors &&
        MercPtrs[i]->sSectorX == gpBattleGroup->ubSectorX &&
        MercPtrs[i]->sSectorY == gpBattleGroup->ubSectorY &&
        MercPtrs[i]->bAssignment != ASSIGNMENT_POW && MercPtrs[i]->bAssignment != IN_TRANSIT &&
        !(MercPtrs[i]->uiStatusFlags & (SOLDIER_VEHICLE)) &&  // ATE Ignore vehicles
        !MercPtrs[i]->bSectorZ) {
      // ATE: If we are in a vehicle - remove ourselves from it!
      // if ( MercPtrs[ i ]->uiStatusFlags & ( SOLDIER_DRIVER | SOLDIER_PASSENGER ) )
      //{
      //	RemoveSoldierFromVehicle( MercPtrs[ i ], MercPtrs[ i ]->bVehicleID );
      //}

      if (MercPtrs[i]->ubStrategicInsertionCode == INSERTION_CODE_PRIMARY_EDGEINDEX ||
          MercPtrs[i]->ubStrategicInsertionCode == INSERTION_CODE_SECONDARY_EDGEINDEX) {
        MercPtrs[i]->ubStrategicInsertionCode = (uint8_t)MercPtrs[i]->usStrategicInsertionData;
      }
      gMercPlacement[giPlacements].pSoldier = MercPtrs[i];
      gMercPlacement[giPlacements].ubStrategicInsertionCode = MercPtrs[i]->ubStrategicInsertionCode;
      gMercPlacement[giPlacements].fPlaced = FALSE;
#ifdef JA2BETAVERSION
      CheckForValidMapEdge(&MercPtrs[i]->ubStrategicInsertionCode);
#endif
      switch (MercPtrs[i]->ubStrategicInsertionCode) {
        case INSERTION_CODE_NORTH:
          gfNorth = TRUE;
          break;
        case INSERTION_CODE_EAST:
          gfEast = TRUE;
          break;
        case INSERTION_CODE_SOUTH:
          gfSouth = TRUE;
          break;
        case INSERTION_CODE_WEST:
          gfWest = TRUE;
          break;
      }
      giPlacements++;
    }
  }
  // add all the faces now
  for (i = 0; i < giPlacements; i++) {
    // Load the faces
    SGPFILENAME ImageFile;
    {
      ubFaceIndex = gMercProfiles[GetSolProfile(gMercPlacement[i].pSoldier)].ubFaceIndex;
      if (ubFaceIndex < 100)
        sprintf(ImageFile, "Faces\\65Face\\%02d.sti", ubFaceIndex);
      else
        sprintf(ImageFile, "Faces\\65Face\\%03d.sti", ubFaceIndex);
    }

    if (!AddVObject(CreateVObjectFromFile(ImageFile), &gMercPlacement[i].uiVObjectID)) {
      if (!AddVObject(CreateVObjectFromFile("Faces\\65Face\\speck.sti"),
                      &gMercPlacement[i].uiVObjectID)) {
        AssertMsg(0,
                  String("Failed to load %Faces\\65Face\\%03d.sti or it's placeholder, speck.sti",
                         gMercProfiles[GetSolProfile(gMercPlacement[i].pSoldier)].ubFaceIndex));
      }
    }
    xp = 91 + (i / 2) * 54;
    yp = (i % 2) ? 412 : 361;
    MSYS_DefineRegion(&gMercPlacement[i].region, (uint16_t)xp, (uint16_t)yp, (uint16_t)(xp + 54),
                      (uint16_t)(yp + 62), MSYS_PRIORITY_HIGH, 0, MercMoveCallback,
                      MercClickCallback);
  }

  PlaceMercs();

  if (gubDefaultButton == GROUP_BUTTON) {
    ButtonList[iTPButtons[GROUP_BUTTON]]->uiFlags |= BUTTON_CLICKED_ON;
    for (i = 0; i < giPlacements; i++) {  // go from the currently selected soldier to the end
      if (!gMercPlacement[i].fPlaced) {   // Found an unplaced merc.  Select him.
        gbSelectedMercID = (int8_t)i;
        if (gubDefaultButton == GROUP_BUTTON)
          gubSelectedGroupID = gMercPlacement[i].pSoldier->ubGroupID;
        gfTacticalPlacementGUIDirty = TRUE;
        SetCursorMerc((int8_t)i);
        gpTacticalPlacementSelectedSoldier = gMercPlacement[i].pSoldier;
        break;
      }
    }
  }
}

void RenderTacticalPlacementGUI() {
  int32_t i, xp, yp, width;
  int32_t iStartY;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiDestPitchBYTES;
  uint16_t usHatchColor;
  wchar_t str[128];
  uint8_t *pDestBuf;
  uint8_t ubColor;
  if (gfTacticalPlacementFirstTime) {
    gfTacticalPlacementFirstTime = FALSE;
    DisableScrollMessages();
  }
  // Check to make sure that if we have a hilighted merc (not selected) and the mouse has moved out
  // of it's region, then we will clear the hilighted ID, and refresh the display.
  if (!gfTacticalPlacementGUIDirty && gbHilightedMercID != -1) {
    xp = 91 + (gbHilightedMercID / 2) * 54;
    yp = (gbHilightedMercID % 2) ? 412 : 361;
    if (gusMouseXPos < xp || gusMouseXPos > xp + 54 || gusMouseYPos < yp ||
        gusMouseYPos > yp + 62) {
      gbHilightedMercID = -1;
      gubHilightedGroupID = 0;
      SetCursorMerc(gbSelectedMercID);
      gpTacticalPlacementHilightedSoldier = NULL;
    }
  }
  // If the display is dirty render the entire panel.
  if (gfTacticalPlacementGUIDirty) {
    BltVObjectFromIndex(vsFB, giOverheadPanelImage, 0, 0, 320);
    InvalidateRegion(0, 0, 320, 480);
    gfTacticalPlacementGUIDirty = FALSE;
    MarkButtonsDirty();
    // DisableHilightsAndHelpText();
    // RenderButtons();
    // EnableHilightsAndHelpText();
    for (i = 0; i < giPlacements; i++) {  // Render the mercs
      pSoldier = gMercPlacement[i].pSoldier;
      xp = 95 + (i / 2) * 54;
      yp = (i % 2) ? 422 : 371;
      ColorFillVSurfaceArea(vsFB, xp + 36, yp + 2, xp + 44, yp + 30, 0);
      BltVObjectFromIndex(vsFB, giMercPanelImage, 0, xp, yp);
      BltVObjectFromIndex(vsFB, gMercPlacement[i].uiVObjectID, 0, xp + 2, yp + 2);
      // HEALTH BAR
      if (!pSoldier->bLife) continue;
      // yellow one for bleeding
      iStartY = yp + 29 - 27 * pSoldier->bLifeMax / 100;
      ColorFillVSurfaceArea(vsFB, xp + 36, iStartY, xp + 37, yp + 29,
                            rgb32_to_rgb16(FROMRGB(107, 107, 57)));
      ColorFillVSurfaceArea(vsFB, xp + 37, iStartY, xp + 38, yp + 29,
                            rgb32_to_rgb16(FROMRGB(222, 181, 115)));
      // pink one for bandaged.
      iStartY += 27 * pSoldier->bBleeding / 100;
      ColorFillVSurfaceArea(vsFB, xp + 36, iStartY, xp + 37, yp + 29,
                            rgb32_to_rgb16(FROMRGB(156, 57, 57)));
      ColorFillVSurfaceArea(vsFB, xp + 37, iStartY, xp + 38, yp + 29,
                            rgb32_to_rgb16(FROMRGB(222, 132, 132)));
      // red one for actual health
      iStartY = yp + 29 - 27 * pSoldier->bLife / 100;
      ColorFillVSurfaceArea(vsFB, xp + 36, iStartY, xp + 37, yp + 29,
                            rgb32_to_rgb16(FROMRGB(107, 8, 8)));
      ColorFillVSurfaceArea(vsFB, xp + 37, iStartY, xp + 38, yp + 29,
                            rgb32_to_rgb16(FROMRGB(206, 0, 0)));
      // BREATH BAR
      iStartY = yp + 29 - 27 * pSoldier->bBreathMax / 100;
      ColorFillVSurfaceArea(vsFB, xp + 39, iStartY, xp + 40, yp + 29,
                            rgb32_to_rgb16(FROMRGB(8, 8, 132)));
      ColorFillVSurfaceArea(vsFB, xp + 40, iStartY, xp + 41, yp + 29,
                            rgb32_to_rgb16(FROMRGB(8, 8, 107)));
      // MORALE BAR
      iStartY = yp + 29 - 27 * pSoldier->bMorale / 100;
      ColorFillVSurfaceArea(vsFB, xp + 42, iStartY, xp + 43, yp + 29,
                            rgb32_to_rgb16(FROMRGB(8, 156, 8)));
      ColorFillVSurfaceArea(vsFB, xp + 43, iStartY, xp + 44, yp + 29,
                            rgb32_to_rgb16(FROMRGB(8, 107, 8)));
    }
    SetFont(BLOCKFONT);
    SetFontForeground(FONT_BEIGE);
    SetFontShadow(141);

    GetSectorIDString(gubPBSectorX, gubPBSectorY, gubPBSectorZ, str, ARR_SIZE(str), TRUE);

    mprintf(120, 335, L"%s %s -- %s...", gpStrategicString[STR_TP_SECTOR], str,
            gpStrategicString[STR_TP_CHOOSEENTRYPOSITIONS]);

    // Shade out the part of the tactical map that isn't considered placable.
    BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 320, 640, 160);
  }
  if (gfValidLocationsChanged) {
    if (DayTime()) {     // 6AM to 9PM is black
      usHatchColor = 0;  // Black
    } else {             // 9PM to 6AM is gray (black is too dark to distinguish)
      usHatchColor = rgb32_to_rgb16(FROMRGB(63, 31, 31));
    }
    gfValidLocationsChanged--;
    BlitBufferToBuffer(vsSaveBuffer, vsFB, 4, 4, 636, 320);
    InvalidateRegion(4, 4, 636, 320);
    if (gbCursorMercID == -1) {
      gTPClipRect.iLeft = gfWest ? 30 : 4;
      gTPClipRect.iTop = gfNorth ? 30 : 4;
      gTPClipRect.iRight = gfEast ? 610 : 636;
      gTPClipRect.iBottom = gfSouth ? 290 : 320;
    } else {
      gTPClipRect.iLeft = 4;
      gTPClipRect.iTop = 4;
      gTPClipRect.iRight = 636;
      gTPClipRect.iBottom = 320;
      switch (gMercPlacement[gbCursorMercID].ubStrategicInsertionCode) {
        case INSERTION_CODE_NORTH:
          gTPClipRect.iTop = 30;
          break;
        case INSERTION_CODE_EAST:
          gTPClipRect.iRight = 610;
          break;
        case INSERTION_CODE_SOUTH:
          gTPClipRect.iBottom = 290;
          break;
        case INSERTION_CODE_WEST:
          gTPClipRect.iLeft = 30;
          break;
      }
    }
    pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
    Blt16BPPBufferLooseHatchRectWithColor((uint16_t *)pDestBuf, uiDestPitchBYTES, &gTPClipRect,
                                          usHatchColor);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
    RectangleDraw(TRUE, gTPClipRect.iLeft, gTPClipRect.iTop, gTPClipRect.iRight,
                  gTPClipRect.iBottom, usHatchColor, pDestBuf);
    JSurface_Unlock(vsFB);
  }
  for (i = 0; i < giPlacements; i++) {  // Render the merc's names
    pSoldier = gMercPlacement[i].pSoldier;
    xp = 95 + (i / 2) * 54;
    yp = (i % 2) ? 422 : 371;
    // NAME
    if ((gubDefaultButton == GROUP_BUTTON &&
         gMercPlacement[i].pSoldier->ubGroupID == gubSelectedGroupID) ||
        (gubDefaultButton != GROUP_BUTTON && i == gbSelectedMercID)) {
      ubColor = FONT_YELLOW;
    } else if ((gubDefaultButton == GROUP_BUTTON &&
                gMercPlacement[i].pSoldier->ubGroupID == gubHilightedGroupID) ||
               (gubDefaultButton != GROUP_BUTTON && i == gbHilightedMercID)) {
      ubColor = FONT_WHITE;
    } else {
      ubColor = FONT_GRAY3;
    }
    SetFont(FONT10ARIALBOLD);
    SetFontForeground(ubColor);
    SetFontShadow(141);
    // Render the question mark over the face if the merc hasn't yet been placed.
    if (gMercPlacement[i].fPlaced) {
      RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (int16_t)(xp + 16), (int16_t)(yp + 14),
                             (int16_t)(xp + 24), (int16_t)(yp + 22));
    } else {
      mprintf(xp + 16, yp + 14, L"?");
      InvalidateRegion(xp + 16, yp + 14, xp + 24, yp + 22);
    }
    SetFont(BLOCKFONT);
    width = StringPixLength(pSoldier->name, BLOCKFONT);
    xp = xp + (48 - width) / 2;
    yp = yp + 33;
    mprintf(xp, yp, pSoldier->name);
    InvalidateRegion(xp, yp, xp + width, yp + width);
  }
}

void EnsureDoneButtonStatus() {
  int32_t i;
  // static BOOLEAN fInside = FALSE;
  // BOOLEAN fChanged = FALSE;
  for (i = 0; i < giPlacements; i++) {
    if (!gMercPlacement[i].fPlaced) {
      if (ButtonList[iTPButtons[DONE_BUTTON]]->uiFlags & BUTTON_ENABLED) {
        DisableButton(iTPButtons[DONE_BUTTON]);
        SetButtonFastHelpText(iTPButtons[DONE_BUTTON], gpStrategicString[STR_TP_DISABLED_DONEHELP]);
      }
      return;
    }
  }
  if (!(ButtonList[iTPButtons[DONE_BUTTON]]->uiFlags &
        BUTTON_ENABLED)) {  // only enable it when it is disabled, otherwise the button will stay
                            // down!
    EnableButton(iTPButtons[DONE_BUTTON]);
    SetButtonFastHelpText(iTPButtons[DONE_BUTTON], gpStrategicString[STR_TP_DONEHELP]);
  }
}

void TacticalPlacementHandle() {
  InputAtom InputEvent;

  EnsureDoneButtonStatus();

  RenderTacticalPlacementGUI();

  if (gfRightButtonState) {
    gbSelectedMercID = -1;
    gubSelectedGroupID = 0;
    gpTacticalPlacementSelectedSoldier = NULL;
  }

  while (DequeueEvent(&InputEvent)) {
    if (InputEvent.usEvent == KEY_DOWN) {
      switch (InputEvent.usParam) {
#ifdef JA2TESTVERSION
        case ESC:
          KillTacticalPlacementGUI();
          break;
#endif
        case ENTER:
          if (ButtonList[iTPButtons[DONE_BUTTON]]->uiFlags & BUTTON_ENABLED) {
            KillTacticalPlacementGUI();
          }
          break;
        case 'c':
          ClearPlacementsCallback(ButtonList[iTPButtons[CLEAR_BUTTON]],
                                  MSYS_CALLBACK_REASON_LBUTTON_UP);
          break;
        case 'g':
          GroupPlacementsCallback(ButtonList[iTPButtons[GROUP_BUTTON]],
                                  MSYS_CALLBACK_REASON_LBUTTON_UP);
          break;
        case 's':
          SpreadPlacementsCallback(ButtonList[iTPButtons[SPREAD_BUTTON]],
                                   MSYS_CALLBACK_REASON_LBUTTON_UP);
          break;
        case 'x':
          if (InputEvent.usKeyState & ALT_DOWN) {
            HandleShortCutExitState();
          }
          break;
      }
    }
  }
  gfValidCursor = FALSE;
  if (gbSelectedMercID != -1 && gusMouseYPos < 320) {
    switch (gMercPlacement[gbCursorMercID].ubStrategicInsertionCode) {
      case INSERTION_CODE_NORTH:
        if (gusMouseYPos <= 40) gfValidCursor = TRUE;
        break;
      case INSERTION_CODE_EAST:
        if (gusMouseXPos >= 600) gfValidCursor = TRUE;
        break;
      case INSERTION_CODE_SOUTH:
        if (gusMouseYPos >= 280) gfValidCursor = TRUE;
        break;
      case INSERTION_CODE_WEST:
        if (gusMouseXPos <= 40) gfValidCursor = TRUE;
        break;
    }
    if (gubDefaultButton == GROUP_BUTTON) {
      if (gfValidCursor) {
        SetCurrentCursorFromDatabase(CURSOR_PLACEGROUP);
      } else {
        SetCurrentCursorFromDatabase(CURSOR_DPLACEGROUP);
      }
    } else {
      if (gfValidCursor) {
        SetCurrentCursorFromDatabase(CURSOR_PLACEMERC);
      } else {
        SetCurrentCursorFromDatabase(CURSOR_DPLACEMERC);
      }
    }
  } else {
    SetCurrentCursorFromDatabase(CURSOR_NORMAL);
  }
  if (gfKillTacticalGUI == 1) {
    KillTacticalPlacementGUI();
  } else if (gfKillTacticalGUI == 2) {
    gfKillTacticalGUI = 1;
  }
}

void KillTacticalPlacementGUI() {
  int32_t i;

  gbHilightedMercID = -1;
  gbSelectedMercID = -1;
  gubSelectedGroupID = 0;
  gubHilightedGroupID = 0;
  gbCursorMercID = -1;
  gpTacticalPlacementHilightedSoldier = NULL;
  gpTacticalPlacementSelectedSoldier = NULL;

  // Destroy the tactical placement gui.
  gfEnterTacticalPlacementGUI = FALSE;
  gfTacticalPlacementGUIActive = FALSE;
  gfKillTacticalGUI = FALSE;
  // Delete video objects
  DeleteVideoObjectFromIndex(giOverheadPanelImage);
  DeleteVideoObjectFromIndex(giMercPanelImage);
  // Delete buttons
  for (i = 0; i < NUM_TP_BUTTONS; i++) {
    UnloadButtonImage(giOverheadButtonImages[i]);
    RemoveButton(iTPButtons[i]);
  }
  // Delete faces and regions
  for (i = 0; i < giPlacements; i++) {
    DeleteVideoObjectFromIndex(gMercPlacement[i].uiVObjectID);
    MSYS_RemoveRegion(&gMercPlacement[i].region);
  }

  if (gsCurInterfacePanel < 0 || gsCurInterfacePanel >= NUM_UI_PANELS)
    gsCurInterfacePanel = TEAM_PANEL;

  SetCurrentInterfacePanel((uint8_t)gsCurInterfacePanel);

  // Leave the overhead map.
  KillOverheadMap();
  // Recreate the tactical panel.
  MSYS_EnableRegion(&gRadarRegion);
  SetCurrentInterfacePanel(TEAM_PANEL);
  // Initialize the rest of the map (AI, enemies, civs, etc.)

  for (i = 0; i < giPlacements; i++) {
    PickUpMercPiece(i);
  }

  PrepareLoadedSector();
  EnableScrollMessages();

#ifdef JA2BETAVERSION
  if (gfChangedEntrySide) {
    ScreenMsg(FONT_RED, MSG_ERROR,
              L"Substituted different entry side due to invalid entry points or map edgepoints.  "
              L"KM, LC : 1");
  }
#endif
}

void ChooseRandomEdgepoints() {
  int32_t i;
  for (i = 0; i < giPlacements; i++) {
    if (!(gMercPlacement[i].pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      gMercPlacement[i].pSoldier->usStrategicInsertionData =
          ChooseMapEdgepoint(gMercPlacement[i].ubStrategicInsertionCode);
      if (gMercPlacement[i].pSoldier->usStrategicInsertionData != NOWHERE) {
        gMercPlacement[i].pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
      } else {
        if (gMercPlacement[i].pSoldier->usStrategicInsertionData < 0 ||
            gMercPlacement[i].pSoldier->usStrategicInsertionData > WORLD_MAX) {
          i = i;
        }
        gMercPlacement[i].pSoldier->ubStrategicInsertionCode =
            gMercPlacement[i].ubStrategicInsertionCode;
      }
    }

    PutDownMercPiece(i);
  }
  gfEveryonePlaced = TRUE;
}

void PlaceMercs() {
  int32_t i;
  switch (gubDefaultButton) {
    case SPREAD_BUTTON:  // Place mercs randomly along their side using map edgepoints.
      ChooseRandomEdgepoints();
      break;
    case CLEAR_BUTTON:
      for (i = 0; i < giPlacements; i++) {
        PickUpMercPiece(i);
      }
      gubSelectedGroupID = 0;
      gbSelectedMercID = 0;
      SetCursorMerc(0);
      gfEveryonePlaced = FALSE;
      break;
    default:
      return;
  }
  gfTacticalPlacementGUIDirty = TRUE;
}

void DoneOverheadPlacementClickCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gfKillTacticalGUI = 2;
  }
}

void SpreadPlacementsCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubDefaultButton = SPREAD_BUTTON;
    ButtonList[iTPButtons[GROUP_BUTTON]]->uiFlags &= ~BUTTON_CLICKED_ON;
    ButtonList[iTPButtons[GROUP_BUTTON]]->uiFlags |= BUTTON_DIRTY;
    PlaceMercs();
    gubSelectedGroupID = 0;
    gbSelectedMercID = -1;
    SetCursorMerc(-1);
  }
}

void GroupPlacementsCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (gubDefaultButton == GROUP_BUTTON) {
      btn->uiFlags &= ~BUTTON_CLICKED_ON;
      btn->uiFlags |= BUTTON_DIRTY;
      gubDefaultButton = CLEAR_BUTTON;
      gubSelectedGroupID = 0;
    } else {
      btn->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
      gubDefaultButton = GROUP_BUTTON;
      gbSelectedMercID = 0;
      SetCursorMerc(gbSelectedMercID);
      gubSelectedGroupID = gMercPlacement[gbSelectedMercID].pSoldier->ubGroupID;
    }
  }
}

void ClearPlacementsCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    ButtonList[iTPButtons[GROUP_BUTTON]]->uiFlags &= ~BUTTON_CLICKED_ON;
    ButtonList[iTPButtons[GROUP_BUTTON]]->uiFlags |= BUTTON_DIRTY;
    gubDefaultButton = CLEAR_BUTTON;
    PlaceMercs();
  }
}

void MercMoveCallback(struct MOUSE_REGION *reg, int32_t reason) {
  if (reg->uiFlags & MSYS_MOUSE_IN_AREA) {
    int8_t i;
    for (i = 0; i < giPlacements; i++) {
      if (&gMercPlacement[i].region == reg) {
        if (gbHilightedMercID != i) {
          gbHilightedMercID = i;
          if (gubDefaultButton == GROUP_BUTTON)
            gubHilightedGroupID = gMercPlacement[i].pSoldier->ubGroupID;
          SetCursorMerc(i);
          gpTacticalPlacementHilightedSoldier = gMercPlacement[i].pSoldier;
        }
        return;
      }
    }
  }
}

void MercClickCallback(struct MOUSE_REGION *reg, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    int8_t i;
    for (i = 0; i < giPlacements; i++) {
      if (&gMercPlacement[i].region == reg) {
        if (gbSelectedMercID != i) {
          gbSelectedMercID = i;
          gpTacticalPlacementSelectedSoldier = gMercPlacement[i].pSoldier;
          if (gubDefaultButton == GROUP_BUTTON) {
            gubSelectedGroupID = gpTacticalPlacementSelectedSoldier->ubGroupID;
          }
        }
        return;
      }
    }
  }
}

void SelectNextUnplacedUnit() {
  int32_t i;
  if (gbSelectedMercID == -1) return;
  for (i = gbSelectedMercID; i < giPlacements;
       i++) {                          // go from the currently selected soldier to the end
    if (!gMercPlacement[i].fPlaced) {  // Found an unplaced merc.  Select him.
      gbSelectedMercID = (int8_t)i;
      if (gubDefaultButton == GROUP_BUTTON)
        gubSelectedGroupID = gMercPlacement[i].pSoldier->ubGroupID;
      gfTacticalPlacementGUIDirty = TRUE;
      SetCursorMerc((int8_t)i);
      gpTacticalPlacementSelectedSoldier = gMercPlacement[i].pSoldier;
      return;
    }
  }
  for (i = 0; i < gbSelectedMercID;
       i++) {                          // go from the beginning to the currently selected soldier
    if (!gMercPlacement[i].fPlaced) {  // Found an unplaced merc.  Select him.
      gbSelectedMercID = (int8_t)i;
      if (gubDefaultButton == GROUP_BUTTON)
        gubSelectedGroupID = gMercPlacement[i].pSoldier->ubGroupID;
      gfTacticalPlacementGUIDirty = TRUE;
      SetCursorMerc((int8_t)i);
      gpTacticalPlacementSelectedSoldier = gMercPlacement[i].pSoldier;
      return;
    }
  }
  // checked the whole array, and everybody has been placed.  Select nobody.
  if (!gfEveryonePlaced) {
    gfEveryonePlaced = TRUE;
    SetCursorMerc(-1);
    gbSelectedMercID = -1;
    gubSelectedGroupID = 0;
    gfTacticalPlacementGUIDirty = TRUE;
    gfValidLocationsChanged = TRUE;
    gpTacticalPlacementSelectedSoldier = gMercPlacement[i].pSoldier;
  }
}

void HandleTacticalPlacementClicksInOverheadMap(struct MOUSE_REGION *reg, int32_t reason) {
  int32_t i;
  int16_t sGridNo;
  BOOLEAN fInvalidArea = FALSE;
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {  // if we have a selected merc, move him to the
                                                   // new closest map edgepoint of his side.
    if (gfValidCursor) {
      if (gbSelectedMercID != -1) {
        if (GetOverheadMouseGridNo(&sGridNo)) {  // we have clicked within a valid part of the map.
          BeginMapEdgepointSearch();

          if (gubDefaultButton == GROUP_BUTTON) {  // We are placing a whole group.
            for (i = 0; i < giPlacements;
                 i++) {  // Find locations of each member of the group, but don't place them yet. If
              // one of the mercs can't be placed, then we won't place any, and tell the user
              // the problem.  If everything's okay, we will place them all.
              if (gMercPlacement[i].pSoldier->ubGroupID == gubSelectedGroupID) {
                gMercPlacement[i].pSoldier->usStrategicInsertionData =
                    SearchForClosestPrimaryMapEdgepoint(sGridNo,
                                                        gMercPlacement[i].ubStrategicInsertionCode);
                if (gMercPlacement[i].pSoldier->usStrategicInsertionData == NOWHERE) {
                  fInvalidArea = TRUE;
                  break;
                }
              }
            }
            if (!fInvalidArea) {  // One or more of the mercs in the group didn't get gridno
                                  // assignments, so we
              // report an error.
              for (i = 0; i < giPlacements; i++) {
                gMercPlacement[i].pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
                if (gMercPlacement[i].pSoldier->ubGroupID == gubSelectedGroupID) {
                  PutDownMercPiece(i);
                }
              }
            }
          } else {  // This is a single merc placement.  If valid, then place him, else report
                    // error.
            gMercPlacement[gbSelectedMercID].pSoldier->usStrategicInsertionData =
                SearchForClosestPrimaryMapEdgepoint(
                    sGridNo, gMercPlacement[gbSelectedMercID].ubStrategicInsertionCode);
            if (gMercPlacement[gbSelectedMercID].pSoldier->usStrategicInsertionData != NOWHERE) {
              gMercPlacement[gbSelectedMercID].pSoldier->ubStrategicInsertionCode =
                  INSERTION_CODE_GRIDNO;
              PutDownMercPiece(gbSelectedMercID);
            } else {
              fInvalidArea = TRUE;
            }

            // gbSelectedMercID++;
            // if( gbSelectedMercID == giPlacements )
            //	gbSelectedMercID = 0;
            // gpTacticalPlacementSelectedSoldier = gMercPlacement[ gbSelectedMercID ].pSoldier;
            gfTacticalPlacementGUIDirty = TRUE;
            // SetCursorMerc( gbSelectedMercID );
          }
          EndMapEdgepointSearch();

          if (fInvalidArea) {  // Report error due to invalid placement.
            SGPRect CenterRect = {220, 120, 420, 200};
            DoMessageBox(MSG_BOX_BASIC_STYLE, gpStrategicString[STR_TP_INACCESSIBLE_MESSAGE],
                         guiCurrentScreen, MSG_BOX_FLAG_OK | MSG_BOX_FLAG_USE_CENTERING_RECT,
                         DialogRemoved, &CenterRect);
          } else {  // Placement successful, so select the next unplaced unit (single or group).
            SelectNextUnplacedUnit();
          }
        }
      }
    } else {  // not a valid cursor location...
      if (gbCursorMercID != -1) {
        SGPRect CenterRect = {220, 120, 420, 200};
        DoMessageBox(MSG_BOX_BASIC_STYLE, gpStrategicString[STR_TP_INVALID_MESSAGE],
                     guiCurrentScreen, MSG_BOX_FLAG_OK | MSG_BOX_FLAG_USE_CENTERING_RECT,
                     DialogRemoved, &CenterRect);
      }
    }
  }
}

void SetCursorMerc(int8_t bPlacementID) {
  if (gbCursorMercID != bPlacementID) {
    if (gbCursorMercID == -1 || bPlacementID == -1 ||
        gMercPlacement[gbCursorMercID].ubStrategicInsertionCode !=
            gMercPlacement[bPlacementID].ubStrategicInsertionCode)
      gfValidLocationsChanged = TRUE;
    gbCursorMercID = bPlacementID;
  }
}

void PutDownMercPiece(int32_t iPlacement) {
  int16_t sGridNo, sCellX, sCellY;
  uint8_t ubDirection;

  struct SOLDIERTYPE *pSoldier;
  pSoldier = gMercPlacement[iPlacement].pSoldier;
  switch (pSoldier->ubStrategicInsertionCode) {
    case INSERTION_CODE_NORTH:
      pSoldier->sInsertionGridNo = gMapInformation.sNorthGridNo;
      break;
    case INSERTION_CODE_SOUTH:
      pSoldier->sInsertionGridNo = gMapInformation.sSouthGridNo;
      break;
    case INSERTION_CODE_EAST:
      pSoldier->sInsertionGridNo = gMapInformation.sEastGridNo;
      break;
    case INSERTION_CODE_WEST:
      pSoldier->sInsertionGridNo = gMapInformation.sWestGridNo;
      break;
    case INSERTION_CODE_GRIDNO:
      pSoldier->sInsertionGridNo = pSoldier->usStrategicInsertionData;
      break;
    default:
      Assert(0);
      break;
  }
  if (gMercPlacement[iPlacement].fPlaced) PickUpMercPiece(iPlacement);
  sGridNo = FindGridNoFromSweetSpot(pSoldier, pSoldier->sInsertionGridNo, 4, &ubDirection);
  if (sGridNo != NOWHERE) {
    ConvertGridNoToCellXY(sGridNo, &sCellX, &sCellY);
    EVENT_SetSoldierPosition(pSoldier, (float)sCellX, (float)sCellY);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);
    pSoldier->ubInsertionDirection = pSoldier->bDirection;
    gMercPlacement[iPlacement].fPlaced = TRUE;
    gMercPlacement[iPlacement].pSoldier->bInSector = TRUE;
  }
}

void PickUpMercPiece(int32_t iPlacement) {
  RemoveSoldierFromGridNo(gMercPlacement[iPlacement].pSoldier);
  gMercPlacement[iPlacement].fPlaced = FALSE;
  gMercPlacement[iPlacement].pSoldier->bInSector = FALSE;
}

void FastHelpRemovedCallback() { gfTacticalPlacementGUIDirty = TRUE; }

void FastHelpRemoved2Callback() {
  gfTacticalPlacementGUIDirty = TRUE;
  gfValidLocationsChanged = 2;  // because fast help text covers it.
}

void DialogRemoved(uint8_t ubResult) {
  gfTacticalPlacementGUIDirty = TRUE;
  gfValidLocationsChanged = TRUE;
}
