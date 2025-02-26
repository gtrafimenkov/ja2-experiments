// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/EditorTaskbarUtils.h"

#include <stdarg.h>
#include <stdio.h>

#include "BuildDefines.h"
#include "Editor/CursorModes.h"
#include "Editor/EditScreen.h"
#include "Editor/EditorBuildings.h"
#include "Editor/EditorCallbackPrototypes.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorItems.h"
#include "Editor/EditorMapInfo.h"
#include "Editor/EditorMercs.h"
#include "Editor/EditorModes.h"
#include "Editor/EditorTaskbarCreation.h"
#include "Editor/EditorTerrain.h"
#include "Editor/ItemStatistics.h"
#include "Editor/SectorSummary.h"
#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Keys.h"
#include "Tactical/MapInformation.h"
#include "Tactical/SoldierFind.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/Pits.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldDef.h"
#include "Utils/FontControl.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/TimerControl.h"
#include "Utils/WordWrap.h"

void RenderEditorInfo();

extern struct ITEM_POOL *gpItemPool;

// editor icon storage vars
int32_t giEditMercDirectionIcons[2];
uint32_t guiMercInventoryPanel;
uint32_t guiOmertaMap;
struct JSurface *vsMercInvPanelBuffers[9];
struct JSurface *vsMercTempBuffer;
int32_t giEditMercImage[2];
uint32_t guiExclamation;
uint32_t guiKeyImage;

// editor Mouseregion storage vars
struct MOUSE_REGION TerrainTileButtonRegion[NUM_TERRAIN_TILE_REGIONS];
struct MOUSE_REGION ItemsRegion;
struct MOUSE_REGION MercRegion;
struct MOUSE_REGION EditorRegion;

void EnableEditorRegion(int8_t bRegionID) {
  switch (bRegionID) {
    case BASE_TERRAIN_TILE_REGION_ID:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      MSYS_EnableRegion(&TerrainTileButtonRegion[bRegionID]);
      break;
    case ITEM_REGION_ID:
      MSYS_EnableRegion(&ItemsRegion);
      break;
    case MERC_REGION_ID:
      MSYS_EnableRegion(&MercRegion);
      break;
  }
}

void DisableEditorRegion(int8_t bRegionID) {
  switch (bRegionID) {
    case BASE_TERRAIN_TILE_REGION_ID:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      MSYS_DisableRegion(&TerrainTileButtonRegion[bRegionID]);
      break;
    case ITEM_REGION_ID:
      MSYS_DisableRegion(&ItemsRegion);
      break;
    case MERC_REGION_ID:
      MSYS_DisableRegion(&MercRegion);
      break;
  }
}

void RemoveEditorRegions() {
  int32_t x;
  MSYS_RemoveRegion(&EditorRegion);
  for (x = BASE_TERRAIN_TILE_REGION_ID; x < NUM_TERRAIN_TILE_REGIONS; x++) {
    MSYS_RemoveRegion(&TerrainTileButtonRegion[x]);
  }
  MSYS_RemoveRegion(&ItemsRegion);
  MSYS_RemoveRegion(&MercRegion);
}

void InitEditorRegions() {
  int32_t x;

  // By doing this, all of the buttons underneath are blanketed and can't be used anymore.
  // Any new buttons will cover this up as well.  Think of it as a barrier between the editor
  // buttons, and the game's interface panel buttons and regions.
  MSYS_DefineRegion(&EditorRegion, 0, 360, 640, 480, MSYS_PRIORITY_NORMAL, 0, MSYS_NO_CALLBACK,
                    MSYS_NO_CALLBACK);

  // Create the regions for the terrain tile selections
  for (x = 0; x < NUM_TERRAIN_TILE_REGIONS; x++) {
    MSYS_DefineRegion(&TerrainTileButtonRegion[x], (int16_t)(261 + x * 42), 369,
                      (int16_t)(303 + x * 42), 391, MSYS_PRIORITY_NORMAL, 0, MSYS_NO_CALLBACK,
                      (MOUSE_CALLBACK)TerrainTileButtonRegionCallback);
    MSYS_SetRegionUserData(&TerrainTileButtonRegion[x], 0, x);
    MSYS_DisableRegion(&TerrainTileButtonRegion[x]);
  }
  gfShowTerrainTileButtons = FALSE;

  // Create the region for the items selection window.
  MSYS_DefineRegion(&ItemsRegion, 100, 360, 540, 440, MSYS_PRIORITY_NORMAL, 0,
                    (MOUSE_CALLBACK)MouseMovedInItemsRegion,
                    (MOUSE_CALLBACK)MouseClickedInItemsRegion);
  MSYS_DisableRegion(&ItemsRegion);

  // Create the region for the merc inventory panel.
  MSYS_DefineRegion(&MercRegion, 175, 361, 450, 460, MSYS_PRIORITY_NORMAL, 0,
                    MouseMovedInMercRegion, MouseClickedInMercRegion);
  MSYS_DisableRegion(&MercRegion);
}

void LoadEditorImages() {
  // Set up the merc inventory panel
  if (!AddVObject(CreateVObjectFromFile("EDITOR\\InvPanel.sti"), &guiMercInventoryPanel))
    AssertMsg(0, "Failed to load data\\editor\\InvPanel.sti");
  // Set up small omerta map
  if (!AddVObject(CreateVObjectFromFile("EDITOR\\omerta.sti"), &guiOmertaMap))
    AssertMsg(0, "Failed to load data\\editor\\omerta.sti");
  // Set up the merc directional buttons.
  giEditMercDirectionIcons[0] = LoadGenericButtonIcon("EDITOR//arrowsoff.sti");
  giEditMercDirectionIcons[1] = LoadGenericButtonIcon("EDITOR//arrowson.sti");

  giEditMercImage[0] = LoadButtonImage("EDITOR\\leftarrow.sti", 0, 1, 2, 3, 4);
  giEditMercImage[1] = LoadButtonImage("EDITOR\\rightarrow.sti", 0, 1, 2, 3, 4);

  if (!AddVObject(CreateVObjectFromFile("EDITOR\\Exclamation.sti"), &guiExclamation))
    AssertMsg(0, "Failed to load data\\editor\\Exclamation.sti");
  if (!AddVObject(CreateVObjectFromFile("EDITOR\\KeyImage.sti"), &guiKeyImage))
    AssertMsg(0, "Failed to load data\\editor\\KeyImage.sti");
}

void DeleteEditorImages() {
  // The merc inventory panel
  DeleteVideoObjectFromIndex(guiMercInventoryPanel);
  DeleteVideoObjectFromIndex(guiOmertaMap);
  // The merc directional buttons
  UnloadGenericButtonIcon((int16_t)giEditMercDirectionIcons[0]);
  UnloadGenericButtonIcon((int16_t)giEditMercDirectionIcons[1]);

  UnloadButtonImage(giEditMercImage[0]);
  UnloadButtonImage(giEditMercImage[1]);
}

void CreateEditorBuffers() {
  int32_t i;
  vsMercTempBuffer = JSurface_Create16bpp(60, 25);
  JSurface_SetColorKey(vsMercTempBuffer, FROMRGB(0, 0, 0));

  // create the nine buffers for the merc's inventory slots.
  for (i = 0; i < 9; i++) {
    vsMercInvPanelBuffers[i] = JSurface_Create16bpp(
        MERCINV_SLOT_HEIGHT, i < 3 ? MERCINV_SMSLOT_WIDTH : MERCINV_LGSLOT_WIDTH);
    JSurface_SetColorKey(vsMercInvPanelBuffers[i], FROMRGB(0, 0, 0));
  }
}

void DeleteEditorBuffers() {
  int32_t i;
  JSurface_Free(vsMercTempBuffer);
  vsMercTempBuffer = NULL;
  for (i = 0; i < 9; i++) {
    JSurface_Free(vsMercInvPanelBuffers[i]);
    vsMercInvPanelBuffers[i] = NULL;
  }
}

void ShowEditorToolbar(int32_t iNewTaskMode) {
  switch (iNewTaskMode) {
    case TASK_TERRAIN:
      ShowEditorButtons(FIRST_TERRAIN_BUTTON, LAST_TERRAIN_BUTTON);
      break;
    case TASK_BUILDINGS:
      ShowEditorButtons(FIRST_BUILDINGS_BUTTON, LAST_BUILDINGS_BUTTON);
      break;
    case TASK_ITEMS:
      ShowEditorButtons(FIRST_ITEMS_BUTTON, LAST_ITEMS_BUTTON);
      break;
    case TASK_MERCS:
      ShowEditorButtons(FIRST_MERCS_TEAMMODE_BUTTON, LAST_MERCS_TEAMMODE_BUTTON);
      break;
    case TASK_MAPINFO:
      ShowEditorButtons(FIRST_MAPINFO_BUTTON, LAST_MAPINFO_BUTTON);
      break;
    case TASK_OPTIONS:
      ShowEditorButtons(FIRST_OPTIONS_BUTTON, LAST_OPTIONS_BUTTON);
      break;
    default:
      return;
  }
}

void HideEditorToolbar(int32_t iOldTaskMode) {
  int32_t i, iStart, iEnd;
  switch (iOldTaskMode) {
    case TASK_TERRAIN:
      iStart = FIRST_TERRAIN_BUTTON;
      iEnd = LAST_TERRAIN_BUTTON;
      break;
    case TASK_BUILDINGS:
      iStart = FIRST_BUILDINGS_BUTTON;
      iEnd = LAST_BUILDINGS_BUTTON;
      break;
    case TASK_ITEMS:
      iStart = FIRST_ITEMS_BUTTON;
      iEnd = LAST_ITEMS_BUTTON;
      break;
    case TASK_MERCS:
      iStart = FIRST_MERCS_BUTTON;
      iEnd = LAST_MERCS_BUTTON;
      break;
    case TASK_MAPINFO:
      iStart = FIRST_MAPINFO_BUTTON;
      iEnd = LAST_MAPINFO_BUTTON;
      break;
    case TASK_OPTIONS:
      iStart = FIRST_OPTIONS_BUTTON;
      iEnd = LAST_OPTIONS_BUTTON;
      break;
    default:
      return;
  }
  for (i = iStart; i <= iEnd; i++) {
    HideButton(iEditorButton[i]);
    UnclickEditorButton(i);
  }
}

void CreateEditorTaskbar() {
  InitEditorRegions();
  LoadEditorImages();
  CreateEditorBuffers();
  CreateEditorTaskbarInternal();
  HideItemStatsPanel();
}

void DeleteEditorTaskbar() {
  int32_t x;

  iOldTaskMode = iCurrentTaskbar;

  for (x = 0; x < NUMBER_EDITOR_BUTTONS; x++) RemoveButton(iEditorButton[x]);

  RemoveEditorRegions();
  DeleteEditorImages();
  DeleteEditorBuffers();
}

void DoTaskbar(void) {
  if (!iTaskMode || iTaskMode == iCurrentTaskbar) {
    return;
  }

  gfRenderTaskbar = TRUE;

  HideEditorToolbar(iCurrentTaskbar);

  // Special code when exiting previous editor tab
  switch (iCurrentTaskbar) {
    case TASK_TERRAIN:
      UnclickEditorButton(TAB_TERRAIN);
      HideTerrainTileButtons();
      break;
    case TASK_BUILDINGS:
      UnclickEditorButton(TAB_BUILDINGS);
      KillTextInputMode();
      break;
    case TASK_ITEMS:
      UnclickEditorButton(TAB_ITEMS);
      HideItemStatsPanel();
      if (eInfo.fActive) ClearEditorItemsInfo();
      gfShowPits = FALSE;
      RemoveAllPits();
      break;
    case TASK_MERCS:
      UnclickEditorButton(TAB_MERCS);
      IndicateSelectedMerc(SELECT_NO_MERC);
      SetMercEditingMode(MERC_NOMODE);
      break;
    case TASK_MAPINFO:
      UnclickEditorButton(TAB_MAPINFO);
      ExtractAndUpdateMapInfo();
      KillTextInputMode();
      HideExitGrids();
      break;
    case TASK_OPTIONS:
      UnclickEditorButton(TAB_OPTIONS);
      break;
  }

  // Setup the new tab mode
  iCurrentTaskbar = iTaskMode;
  ShowEditorToolbar(iTaskMode);
  iTaskMode = TASK_NONE;

  // Special code when entering a new editor tab
  switch (iCurrentTaskbar) {
    case TASK_MERCS:
      ClickEditorButton(TAB_MERCS);
      ClickEditorButton(MERCS_ENEMY);
      iDrawMode = DRAW_MODE_ENEMY;
      SetMercEditingMode(MERC_TEAMMODE);
      fBuildingShowRoofs = FALSE;
      UpdateRoofsView();
      break;
    case TASK_TERRAIN:
      ClickEditorButton(TAB_TERRAIN);
      ShowTerrainTileButtons();
      SetEditorTerrainTaskbarMode(TERRAIN_FGROUND_TEXTURES);
      break;
    case TASK_BUILDINGS:
      ClickEditorButton(TAB_BUILDINGS);
      if (fBuildingShowRoofs) ClickEditorButton(BUILDING_TOGGLE_ROOF_VIEW);
      if (fBuildingShowWalls) ClickEditorButton(BUILDING_TOGGLE_WALL_VIEW);
      if (fBuildingShowRoomInfo) ClickEditorButton(BUILDING_TOGGLE_INFO_VIEW);
      if (gfCaves) {
        ClickEditorButton(BUILDING_CAVE_DRAWING);
        iDrawMode = DRAW_MODE_CAVES;
      } else {
        ClickEditorButton(BUILDING_NEW_ROOM);
        iDrawMode = DRAW_MODE_ROOM;
      }
      TerrainTileDrawMode = TERRAIN_TILES_BRETS_STRANGEMODE;
      SetEditorSmoothingMode(gMapInformation.ubEditorSmoothingType);
      gusSelectionType = gusSavedBuildingSelectionType;
      SetupTextInputForBuildings();
      break;
    case TASK_ITEMS:
      SetFont(FONT10ARIAL);
      SetFontForeground(FONT_YELLOW);
      ClickEditorButton(TAB_ITEMS);
      ClickEditorButton(ITEMS_WEAPONS + eInfo.uiItemType - TBAR_MODE_ITEM_WEAPONS);
      InitEditorItemsInfo(eInfo.uiItemType);
      ShowItemStatsPanel();
      gfShowPits = TRUE;
      AddAllPits();
      iDrawMode = DRAW_MODE_PLACE_ITEM;
      break;
    case TASK_MAPINFO:
      ClickEditorButton(TAB_MAPINFO);
      if (gfFakeLights) ClickEditorButton(MAPINFO_TOGGLE_FAKE_LIGHTS);
      ClickEditorButton(MAPINFO_ADD_LIGHT1_SOURCE);
      iDrawMode = DRAW_MODE_LIGHT;
      TerrainTileDrawMode = TERRAIN_TILES_BRETS_STRANGEMODE;
      SetupTextInputForMapInfo();
      break;
    case TASK_OPTIONS:
      ClickEditorButton(TAB_OPTIONS);
      TerrainTileDrawMode = TERRAIN_TILES_NODRAW;
      break;
  }
}

// Disables the task bar, but leaves it on screen. Used when a selection window is up.
void DisableEditorTaskbar(void) {
  int32_t x;
  for (x = 0; x < NUMBER_EDITOR_BUTTONS; x++) DisableButton(iEditorButton[x]);
}

void EnableEditorTaskbar(void) {
  int32_t x;

  for (x = 0; x < NUMBER_EDITOR_BUTTONS; x++) EnableButton(iEditorButton[x]);
  // Keep permanent buttons disabled.
  DisableButton(iEditorButton[MERCS_1]);
  DisableButton(iEditorButton[MAPINFO_LIGHT_PANEL]);
  DisableButton(iEditorButton[MAPINFO_RADIO_PANEL]);
  DisableButton(iEditorButton[ITEMSTATS_PANEL]);
  DisableButton(iEditorButton[MERCS_PLAYERTOGGLE]);
  DisableButton(iEditorButton[MERCS_PLAYER]);
  if (iCurrentTaskbar == TASK_ITEMS) DetermineItemsScrolling();
}

// A specialized mprint function that'll restore the editor panel underneath the
// string before rendering the string.  This is obviously only useful for drawing text
// in the editor taskbar.
void mprintfEditor(int16_t x, int16_t y, wchar_t *pFontString, ...) {
  va_list argptr;
  wchar_t string[512];
  uint16_t uiStringLength, uiStringHeight;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  uiStringLength = StringPixLength(string, FontDefault);
  uiStringHeight = GetFontHeight(FontDefault);

  ClearTaskbarRegion(x, y, (int16_t)(x + uiStringLength), (int16_t)(y + uiStringHeight));
  mprintf(x, y, string);
}

void ClearTaskbarRegion(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom) {
  ColorFillVSurfaceArea(vsButtonDest, sLeft, sTop, sRight, sBottom, gusEditorTaskbarColor);

  if (!sLeft) {
    ColorFillVSurfaceArea(vsButtonDest, 0, sTop, 1, sBottom, gusEditorTaskbarHiColor);
    sLeft++;
  }
  if (sTop == 360) {
    ColorFillVSurfaceArea(vsButtonDest, sLeft, 360, sRight, 361, gusEditorTaskbarHiColor);
    sTop++;
  }
  if (sBottom == 480)
    ColorFillVSurfaceArea(vsButtonDest, sLeft, 479, sRight, 480, gusEditorTaskbarLoColor);
  if (sRight == 640)
    ColorFillVSurfaceArea(vsButtonDest, 639, sTop, 640, sBottom, gusEditorTaskbarLoColor);

  InvalidateRegion(sLeft, sTop, sRight, sBottom);
}

// Kris:
// This is a new function which duplicates the older "yellow info boxes" that
// are common throughout the editor.  This draws the yellow box with the indentation
// look.
void DrawEditorInfoBox(wchar_t *str, uint32_t uiFont, uint16_t x, uint16_t y, uint16_t w,
                       uint16_t h) {
  uint16_t usFillColorDark, usFillColorLight, usFillColorBack;
  uint16_t x2, y2;
  uint16_t usStrWidth;

  x2 = x + w;
  y2 = y + h;

  usFillColorDark = rgb32_to_rgb16(FROMRGB(24, 61, 81));
  usFillColorLight = rgb32_to_rgb16(FROMRGB(136, 138, 135));
  usFillColorBack = rgb32_to_rgb16(FROMRGB(250, 240, 188));

  ColorFillVSurfaceArea(vsButtonDest, x, y, x2, y2, usFillColorDark);
  ColorFillVSurfaceArea(vsButtonDest, x + 1, y + 1, x2, y2, usFillColorLight);
  ColorFillVSurfaceArea(vsButtonDest, x + 1, y + 1, x2 - 1, y2 - 1, usFillColorBack);

  usStrWidth = StringPixLength(str, uiFont);
  if (usStrWidth > w) {  // the string is too long, so use the wrapped method
    y += 1;
    DisplayWrappedString(x, y, w, 2, uiFont, FONT_BLACK, str, FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    return;
  }
  // center the string vertically and horizontally.
  SetFont(uiFont);
  SetFontForeground(FONT_BLACK);
  SetFontShadow(FONT_BLACK);
  x += (w - (uint16_t)StringPixLength(str, uiFont)) / 2;
  y += (h - (uint16_t)GetFontHeight(uiFont)) / 2;
  mprintf(x, y, L"%s", str);
  InvalidateRegion(x, y, x2, y2);
}

void ClickEditorButton(int32_t iEditorButtonID) {
  GUI_BUTTON *butn;
  if (iEditorButtonID < 0 || iEditorButtonID >= NUMBER_EDITOR_BUTTONS) return;
  if (iEditorButton[iEditorButtonID] != -1) {
    butn = ButtonList[iEditorButton[iEditorButtonID]];
    if (butn) butn->uiFlags |= BUTTON_CLICKED_ON;
  }
}

void UnclickEditorButton(int32_t iEditorButtonID) {
  GUI_BUTTON *butn;
  if (iEditorButtonID < 0 || iEditorButtonID >= NUMBER_EDITOR_BUTTONS) return;
  if (iEditorButton[iEditorButtonID] != -1) {
    butn = ButtonList[iEditorButton[iEditorButtonID]];
    if (butn) butn->uiFlags &= (~BUTTON_CLICKED_ON);
  }
}

void HideEditorButton(int32_t iEditorButtonID) { HideButton(iEditorButton[iEditorButtonID]); }

void ShowEditorButton(int32_t iEditorButtonID) { ShowButton(iEditorButton[iEditorButtonID]); }

void DisableEditorButton(int32_t iEditorButtonID) { DisableButton(iEditorButton[iEditorButtonID]); }

void EnableEditorButton(int32_t iEditorButtonID) { EnableButton(iEditorButton[iEditorButtonID]); }

void ClickEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID) {
  int32_t i;
  GUI_BUTTON *b;
  for (i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++) {
    Assert(iEditorButton[i] != -1);
    b = ButtonList[iEditorButton[i]];
    Assert(b);
    b->uiFlags |= BUTTON_CLICKED_ON;
  }
}

void UnclickEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID) {
  int32_t i;
  GUI_BUTTON *b;
  for (i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++) {
    Assert(iEditorButton[i] != -1);
    b = ButtonList[iEditorButton[i]];
    Assert(b);
    b->uiFlags &= (~BUTTON_CLICKED_ON);
  }
}

void HideEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID) {
  int32_t i;
  for (i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++) HideButton(iEditorButton[i]);
}

void ShowEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID) {
  int32_t i;
  for (i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++) ShowButton(iEditorButton[i]);
}

void DisableEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID) {
  int32_t i;
  for (i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++) DisableButton(iEditorButton[i]);
}

void EnableEditorButtons(int32_t iFirstEditorButtonID, int32_t iLastEditorButtonID) {
  int32_t i;
  for (i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++) EnableButton(iEditorButton[i]);
}

void RenderMapEntryPointsAndLights() {
  int16_t sGridNo;
  int16_t sScreenX, sScreenY;
  int32_t i;
  if (gfSummaryWindowActive) return;
  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_YELLOW);
  SetFontShadow(FONT_NEARBLACK);
  sGridNo = gMapInformation.sNorthGridNo;
  if (sGridNo != -1) {
    GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY >= -20 && sScreenY < 340 && sScreenX >= -40 && sScreenX < 640) {
      DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 40, 2, FONT10ARIAL, FONT_YELLOW,
                           L"North Entry Point", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    }
  }
  sGridNo = gMapInformation.sWestGridNo;
  if (sGridNo != -1) {
    GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY >= -20 && sScreenY < 340 && sScreenX >= -40 && sScreenX < 640) {
      DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 40, 2, FONT10ARIAL, FONT_YELLOW,
                           L"West Entry Point", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    }
  }
  sGridNo = gMapInformation.sEastGridNo;
  if (sGridNo != -1) {
    GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY >= -20 && sScreenY < 340 && sScreenX >= -40 && sScreenX < 640) {
      DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 40, 2, FONT10ARIAL, FONT_YELLOW,
                           L"East Entry Point", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    }
  }
  sGridNo = gMapInformation.sSouthGridNo;
  if (sGridNo != -1) {
    GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY >= -20 && sScreenY < 340 && sScreenX >= -40 && sScreenX < 640) {
      DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 40, 2, FONT10ARIAL, FONT_YELLOW,
                           L"South Entry Point", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    }
  }
  sGridNo = gMapInformation.sCenterGridNo;
  if (sGridNo != -1) {
    GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY >= -20 && sScreenY < 340 && sScreenX >= -40 && sScreenX < 640) {
      DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 40, 2, FONT10ARIAL, FONT_YELLOW,
                           L"Center Entry Point", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    }
  }
  sGridNo = gMapInformation.sIsolatedGridNo;
  if (sGridNo != -1) {
    GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY >= -20 && sScreenY < 340 && sScreenX >= -40 && sScreenX < 640) {
      DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 40, 2, FONT10ARIAL, FONT_YELLOW,
                           L"Isolated Entry Point", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
    }
  }
  // Do the lights now.
  for (i = 0; i < MAX_LIGHT_SPRITES; i++) {
    if (LightSprites[i].uiFlags & LIGHT_SPR_ACTIVE) {
      sGridNo = LightSprites[i].iY * WORLD_COLS + LightSprites[i].iX;
      GetGridNoScreenPos(sGridNo, 0, &sScreenX, &sScreenY);
      if (sScreenY >= -50 && sScreenY < 300 && sScreenX >= -40 && sScreenX < 640) {
        if (LightSprites[i].uiFlags & LIGHT_PRIMETIME)
          DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 50, 2, FONT10ARIAL, FONT_ORANGE,
                               L"Prime", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
        else if (LightSprites[i].uiFlags & LIGHT_NIGHTTIME)
          DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 50, 2, FONT10ARIAL, FONT_RED,
                               L"Night", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
        else
          DisplayWrappedString(sScreenX, (int16_t)(sScreenY - 5), 50, 2, FONT10ARIAL, FONT_YELLOW,
                               L"24Hour", FONT_BLACK, TRUE, CENTER_JUSTIFIED);
      }
    }
  }
}

void BuildTriggerName(struct OBJECTTYPE *pItem, wchar_t *szItemName, int bufSize) {
  if (pItem->usItem == SWITCH) {
    if (pItem->bFrequency == PANIC_FREQUENCY)
      swprintf(szItemName, bufSize, L"Panic Trigger1");
    else if (pItem->bFrequency == PANIC_FREQUENCY_2)
      swprintf(szItemName, bufSize, L"Panic Trigger2");
    else if (pItem->bFrequency == PANIC_FREQUENCY_3)
      swprintf(szItemName, bufSize, L"Panic Trigger3");
    else
      swprintf(szItemName, bufSize, L"Trigger%d", pItem->bFrequency - 50);
  } else {  // action item
    if (pItem->bDetonatorType == BOMB_PRESSURE)
      swprintf(szItemName, bufSize, L"Pressure Action");
    else if (pItem->bFrequency == PANIC_FREQUENCY)
      swprintf(szItemName, bufSize, L"Panic Action1");
    else if (pItem->bFrequency == PANIC_FREQUENCY_2)
      swprintf(szItemName, bufSize, L"Panic Action2");
    else if (pItem->bFrequency == PANIC_FREQUENCY_3)
      swprintf(szItemName, bufSize, L"Panic Action3");
    else
      swprintf(szItemName, bufSize, L"Action%d", pItem->bFrequency - 50);
  }
}

void RenderDoorLockInfo() {
  int16_t i, xp, yp;
  int16_t sScreenX, sScreenY;
  wchar_t str[50];
  for (i = 0; i < gubNumDoors; i++) {
    GetGridNoScreenPos(DoorTable[i].sGridNo, 0, &sScreenX, &sScreenY);
    if (sScreenY > 390) continue;
    if (DoorTable[i].ubLockID != 255)
      swprintf(str, ARR_SIZE(str), L"%S", LockTable[DoorTable[i].ubLockID].ubEditorName);
    else
      swprintf(str, ARR_SIZE(str), L"No Lock ID");
    xp = sScreenX - 10;
    yp = sScreenY - 40;
    DisplayWrappedString(xp, yp, 60, 2, FONT10ARIAL, FONT_LTKHAKI, str, FONT_BLACK, TRUE,
                         CENTER_JUSTIFIED);
    if (DoorTable[i].ubTrapID) {
      SetFont(FONT10ARIAL);
      SetFontForeground(FONT_RED);
      SetFontShadow(FONT_NEARBLACK);
      switch (DoorTable[i].ubTrapID) {
        case EXPLOSION:
          swprintf(str, ARR_SIZE(str), L"Explosion Trap");
          break;
        case ELECTRIC:
          swprintf(str, ARR_SIZE(str), L"Electric Trap");
          break;
        case SIREN:
          swprintf(str, ARR_SIZE(str), L"Siren Trap");
          break;
        case SILENT_ALARM:
          swprintf(str, ARR_SIZE(str), L"Silent Alarm");
          break;
        case SUPER_ELECTRIC:
          swprintf(str, ARR_SIZE(str), L"Super Electric Trap");
          break;
      }
      xp = sScreenX + 20 - StringPixLength(str, FONT10ARIAL) / 2;
      yp = sScreenY;
      mprintf(xp, yp, str);
      swprintf(str, ARR_SIZE(str), L"Trap Level %d", DoorTable[i].ubTrapLevel);
      xp = sScreenX + 20 - StringPixLength(str, FONT10ARIAL) / 2;
      mprintf(xp, yp + 10, str);
    }
  }
}

void RenderSelectedItemBlownUp() {
  uint32_t uiVideoObjectIndex;
  struct VObject *hVObject;
  int16_t sScreenX, sScreenY, xp, yp;
  struct ITEM_POOL *pItemPool;
  wchar_t szItemName[SIZE_ITEM_NAME];
  int32_t i;
  int16_t sWidth, sHeight, sOffsetX, sOffsetY;

  GetGridNoScreenPos(gsItemGridNo, 0, &sScreenX, &sScreenY);

  if (sScreenY > 340) return;

  // Display the enlarged item graphic
  uiVideoObjectIndex = GetInterfaceGraphicForItem(&Item[gpItem->usItem]);
  GetVideoObject(&hVObject, uiVideoObjectIndex);

  sWidth = hVObject->pETRLEObject[Item[gpItem->usItem].ubGraphicNum].usWidth;
  sOffsetX = hVObject->pETRLEObject[Item[gpItem->usItem].ubGraphicNum].sOffsetX;
  xp = sScreenX + (40 - sWidth - sOffsetX * 2) / 2;

  sHeight = hVObject->pETRLEObject[Item[gpItem->usItem].ubGraphicNum].usHeight;
  sOffsetY = hVObject->pETRLEObject[Item[gpItem->usItem].ubGraphicNum].sOffsetY;
  yp = sScreenY + (20 - sHeight - sOffsetY * 2) / 2;

  BltVideoObjectOutlineFromIndex(vsFB, uiVideoObjectIndex, Item[gpItem->usItem].ubGraphicNum, xp,
                                 yp, rgb32_to_rgb16(FROMRGB(0, 140, 170)), TRUE);

  // Display the item name above it
  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_YELLOW);
  SetFontShadow(FONT_NEARBLACK);
  if (gpItem->usItem == ACTION_ITEM || gpItem->usItem == SWITCH) {
    BuildTriggerName(gpItem, szItemName, ARR_SIZE(szItemName));
  } else if (Item[gpItem->usItem].usItemClass == IC_KEY) {
    swprintf(szItemName, ARR_SIZE(szItemName), L"%S", LockTable[gpItem->ubKeyID].ubEditorName);
  } else {
    LoadItemInfo(gpItem->usItem, szItemName, NULL);
  }
  xp = sScreenX - (StringPixLength(szItemName, FONT10ARIAL) - 40) / 2;
  yp -= 10;
  mprintf(xp, yp, szItemName);

  if (gpItem->usItem == ACTION_ITEM) {
    wchar_t *pStr;
    pStr = GetActionItemName(gpItem);
    xp = sScreenX - (StringPixLength(pStr, FONT10ARIALBOLD) - 40) / 2;
    yp += 10;
    SetFont(FONT10ARIALBOLD);
    SetFontForeground(FONT_LTKHAKI);
    mprintf(xp, yp, pStr);
    SetFontForeground(FONT_YELLOW);
  }

  // Count the number of items in the current pool, and display that.
  i = 0;
  GetItemPool(gsItemGridNo, &pItemPool, 0);
  Assert(pItemPool);
  while (pItemPool) {
    i++;
    pItemPool = pItemPool->pNext;
  }
  xp = sScreenX;
  yp = sScreenY + 10;
  mprintf(xp, yp, L"%d", i);

  // If the item is hidden, render a blinking H (just like DG)
  if (gWorldItems[gpItemPool->iItemIndex].bVisible == HIDDEN_ITEM ||
      gWorldItems[gpItemPool->iItemIndex].bVisible == BURIED) {
    SetFont(FONT10ARIALBOLD);
    if (GetJA2Clock() % 1000 > 500) {
      SetFontForeground(249);
    }
    mprintf(sScreenX + 16, sScreenY + 7, L"H");
    InvalidateRegion(sScreenX + 16, sScreenY + 7, sScreenX + 24, sScreenY + 27);
  }
}

void RenderEditorInfo() {
  wchar_t FPSText[50];
  int16_t iMapIndex;

  SetFont(FONT12POINT1);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);

  // Display the mapindex position
  if (GetMouseMapPos(&iMapIndex))
    swprintf(FPSText, ARR_SIZE(FPSText), L"   (%d)   ", iMapIndex);
  else
    swprintf(FPSText, ARR_SIZE(FPSText), L"          ");
  mprintfEditor((uint16_t)(50 - StringPixLength(FPSText, FONT12POINT1) / 2), 463, FPSText);

  switch (iCurrentTaskbar) {
    case TASK_OPTIONS:
      if (!gfWorldLoaded || giCurrentTilesetID < 0)
        mprintf(260, 445, L"No map currently loaded.");
      else
        mprintf(260, 445, L"File:  %S, Current Tileset:  %s", gubFilename,
                gTilesets[giCurrentTilesetID].zName);
      break;
    case TASK_TERRAIN:
      if (gusSelectionType == LINESELECTION)
        swprintf(wszSelType[LINESELECTION], ARR_SIZE(wszSelType[LINESELECTION]), L"Width: %d",
                 gusSelectionWidth);
      DrawEditorInfoBox(wszSelType[gusSelectionType], FONT12POINT1, 220, 430, 60, 30);
      swprintf(FPSText, ARR_SIZE(FPSText), L"%d%%", gusSelectionDensity);
      DrawEditorInfoBox(FPSText, FONT12POINT1, 310, 430, 40, 30);
      break;
    case TASK_ITEMS:
      RenderEditorItemsInfo();
      UpdateItemStatsPanel();
      break;
    case TASK_BUILDINGS:
      UpdateBuildingsInfo();
      if (gusSelectionType == LINESELECTION)
        swprintf(wszSelType[LINESELECTION], ARR_SIZE(wszSelType[LINESELECTION]), L"Width: %d",
                 gusSelectionWidth);
      DrawEditorInfoBox(wszSelType[gusSelectionType], FONT12POINT1, 530, 430, 60, 30);
      break;
    case TASK_MERCS:
      UpdateMercsInfo();
      break;
    case TASK_MAPINFO:
      UpdateMapInfo();
      if (gusSelectionType == LINESELECTION)
        swprintf(wszSelType[LINESELECTION], ARR_SIZE(wszSelType[LINESELECTION]), L"Width: %d",
                 gusSelectionWidth);
      DrawEditorInfoBox(wszSelType[gusSelectionType], FONT12POINT1, 440, 430, 60, 30);
      break;
  }
}

// This is in ButtonSystem.c as a hack.  Because we need to save the buffer whenever we do a full
// taskbar render, we need to draw the buttons without hilites, hence this flag.  This flag is
// always true in ButtonSystem.c, so it won't effect anything else.
extern BOOLEAN gfGotoGridNoUI;

void ProcessEditorRendering() {
  BOOLEAN fSaveBuffer = FALSE;
  if (gfRenderTaskbar)  // do a full taskbar render.
  {
    ClearTaskbarRegion(0, 360, 640, 480);
    RenderTerrainTileButtons();
    MarkButtonsDirty();
    gfRenderTaskbar = FALSE;
    fSaveBuffer = TRUE;
    gfRenderDrawingMode = TRUE;
    gfRenderHilights = FALSE;
    gfRenderMercInfo = TRUE;
  }
  if (gfRenderDrawingMode) {
    if (iCurrentTaskbar == TASK_BUILDINGS || iCurrentTaskbar == TASK_TERRAIN ||
        iCurrentTaskbar == TASK_ITEMS) {
      ShowCurrentDrawingMode();
      gfRenderDrawingMode = FALSE;
    }
  }
  // render dynamically changed buttons only
  RenderButtons();

  if (gfSummaryWindowActive)
    RenderSummaryWindow();
  else if (!gfGotoGridNoUI && !InOverheadMap())
    RenderMercStrings();

  if (gfEditingDoor) RenderDoorEditingWindow();

  if (TextInputMode()) RenderAllTextFields();
  RenderEditorInfo();

  if (!gfSummaryWindowActive && !gfGotoGridNoUI && !InOverheadMap()) {
    if (gpItem && gsItemGridNo != -1) RenderSelectedItemBlownUp();
    if (iCurrentTaskbar == TASK_MAPINFO) RenderMapEntryPointsAndLights();
    if ((iDrawMode == DRAW_MODE_PLACE_ITEM && eInfo.uiItemType == TBAR_MODE_ITEM_KEYS) ||
        iDrawMode == DRAW_MODE_DOORKEYS)
      RenderDoorLockInfo();
  }

  if (fSaveBuffer) BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 360, 640, 120);

  // Make sure this is TRUE at all times.
  // It is set to false when before we save the buffer, so the buttons don't get
  // rendered with hilites, in case the mouse is over one.
  gfRenderHilights = TRUE;

  RenderButtonsFastHelp();
}
