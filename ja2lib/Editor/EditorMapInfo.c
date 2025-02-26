// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/EditorMapInfo.h"

#include "BuildDefines.h"
#include "Editor/EditScreen.h"
#include "Editor/EditSys.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorItems.h"
#include "Editor/EditorMercs.h"
#include "Editor/EditorTaskbarUtils.h"
#include "Editor/EditorTerrain.h"  //for access to TerrainTileDrawMode
#include "Editor/EditorUndo.h"
#include "Editor/ItemStatistics.h"
#include "Editor/SelectWin.h"
#include "SGP/Font.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierProfileType.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/TextInput.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

int8_t gbDefaultLightType = PRIMETIME_LIGHT;

struct JPaletteEntry gEditorLightColor;

BOOLEAN gfEditorForceShadeTableRebuild = FALSE;

void SetupTextInputForMapInfo() {
  wchar_t str[10];

  InitTextInputModeWithScheme(DEFAULT_SCHEME);

  AddUserInputField(NULL);  // just so we can use short cut keys while not typing.

  // light rgb fields
  swprintf(str, ARR_SIZE(str), L"%d", gEditorLightColor.red);
  AddTextInputField(10, 394, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
  swprintf(str, ARR_SIZE(str), L"%d", gEditorLightColor.green);
  AddTextInputField(10, 414, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
  swprintf(str, ARR_SIZE(str), L"%d", gEditorLightColor.blue);
  AddTextInputField(10, 434, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);

  swprintf(str, ARR_SIZE(str), L"%d", gsLightRadius);
  AddTextInputField(120, 394, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
  swprintf(str, ARR_SIZE(str), L"%d", gusLightLevel);
  AddTextInputField(120, 414, 25, 18, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT);

  // Scroll restriction ID
  if (!gMapInformation.ubRestrictedScrollID)
    swprintf(str, ARR_SIZE(str), L"");
  else
    swprintf(str, ARR_SIZE(str), L"%d", gMapInformation.ubRestrictedScrollID);
  AddTextInputField(210, 420, 30, 20, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT);

  // exit grid input fields
  swprintf(str, ARR_SIZE(str), L"%c%d", gExitGrid.ubGotoSectorY + 'A' - 1, gExitGrid.ubGotoSectorX);
  AddTextInputField(338, 363, 30, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_EXCLUSIVE_COORDINATE);
  swprintf(str, ARR_SIZE(str), L"%d", gExitGrid.ubGotoSectorZ);
  AddTextInputField(338, 383, 30, 18, MSYS_PRIORITY_NORMAL, str, 1, INPUTTYPE_NUMERICSTRICT);
  swprintf(str, ARR_SIZE(str), L"%d", gExitGrid.usGridNo);
  AddTextInputField(338, 403, 40, 18, MSYS_PRIORITY_NORMAL, str, 5, INPUTTYPE_NUMERICSTRICT);
}

void UpdateMapInfo() {
  SetFont(FONT10ARIAL);
  SetFontShadow(FONT_NEARBLACK);

  SetFontForeground(FONT_RED);
  mprintf(38, 399, L"R");
  SetFontForeground(FONT_GREEN);
  mprintf(38, 419, L"G");
  SetFontForeground(FONT_DKBLUE);
  mprintf(38, 439, L"B");

  SetFontForeground(FONT_YELLOW);
  mprintf(65, 369, L"Prime");
  mprintf(65, 382, L"Night");
  mprintf(65, 397, L"24Hrs");

  SetFontForeground(FONT_YELLOW);
  mprintf(148, 399, L"Radius");

  if (!gfBasement && !gfCaves) SetFontForeground(FONT_DKYELLOW);
  mprintf(148, 414, L"Underground");
  mprintf(148, 423, L"Light Level");

  SetFontForeground(FONT_YELLOW);
  mprintf(230, 369, L"Outdoors");
  mprintf(230, 384, L"Basement");
  mprintf(230, 399, L"Caves");

  SetFontForeground(FONT_ORANGE);
  mprintf(250, 420, L"Restricted");
  mprintf(250, 430, L"Scroll ID");

  SetFontForeground(FONT_YELLOW);
  mprintf(368, 363, L"Destination");
  mprintf(368, 372, L"Sector");
  mprintf(368, 383, L"Destination");
  mprintf(368, 392, L"Bsmt. Level");
  mprintf(378, 403, L"Dest.");
  mprintf(378, 412, L"GridNo");
  SetFontForeground(FONT_RED);
}

void UpdateMapInfoFields() {
  wchar_t str[10];
  // Update the text fields to reflect the validated values.
  // light rgb fields
  swprintf(str, ARR_SIZE(str), L"%d", gEditorLightColor.red);
  SetInputFieldStringWith16BitString(1, str);
  swprintf(str, ARR_SIZE(str), L"%d", gEditorLightColor.green);
  SetInputFieldStringWith16BitString(2, str);
  swprintf(str, ARR_SIZE(str), L"%d", gEditorLightColor.blue);
  SetInputFieldStringWith16BitString(3, str);

  swprintf(str, ARR_SIZE(str), L"%d", gsLightRadius);
  SetInputFieldStringWith16BitString(4, str);
  swprintf(str, ARR_SIZE(str), L"%d", gusLightLevel);
  SetInputFieldStringWith16BitString(5, str);

  if (!gMapInformation.ubRestrictedScrollID)
    swprintf(str, ARR_SIZE(str), L"");
  else
    swprintf(str, ARR_SIZE(str), L"%d", gMapInformation.ubRestrictedScrollID);
  SetInputFieldStringWith16BitString(6, str);

  ApplyNewExitGridValuesToTextFields();
}

void ExtractAndUpdateMapInfo() {
  wchar_t str[10];
  int32_t temp;
  BOOLEAN fUpdateLight1 = FALSE;
  // extract light1 colors
  temp = min(GetNumericStrictValueFromField(1), 255);
  if (temp != -1 && temp != gEditorLightColor.red) {
    fUpdateLight1 = TRUE;
    gEditorLightColor.red = (uint8_t)temp;
  }
  temp = min(GetNumericStrictValueFromField(2), 255);
  if (temp != -1 && temp != gEditorLightColor.green) {
    fUpdateLight1 = TRUE;
    gEditorLightColor.green = (uint8_t)temp;
  }
  temp = min(GetNumericStrictValueFromField(3), 255);
  if (temp != -1 && temp != gEditorLightColor.blue) {
    fUpdateLight1 = TRUE;
    gEditorLightColor.blue = (uint8_t)temp;
  }
  if (fUpdateLight1) {
    gfEditorForceShadeTableRebuild = TRUE;
    LightSetColors(&gEditorLightColor, 1);
    gfEditorForceShadeTableRebuild = FALSE;
  }

  // extract radius
  temp = max(min(GetNumericStrictValueFromField(4), 8), 1);
  if (temp != -1) gsLightRadius = (int16_t)temp;
  temp = max(min(GetNumericStrictValueFromField(5), 15), 1);
  if (temp != -1 && temp != gusLightLevel) {
    gusLightLevel = (uint16_t)temp;
    gfRenderWorld = TRUE;
    ubAmbientLightLevel = (uint8_t)(EDITOR_LIGHT_MAX - gusLightLevel);
    LightSetBaseLevel(ubAmbientLightLevel);
    LightSpriteRenderAll();
  }

  temp = (int8_t)GetNumericStrictValueFromField(6);
  if (temp == -1)
    gMapInformation.ubRestrictedScrollID = 0;
  else
    gMapInformation.ubRestrictedScrollID = (uint8_t)temp;

  // set up fields for exitgrid information
  Get16BitStringFromField(7, str, ARR_SIZE(str));
  if (str[0] >= 'a' && str[0] <= 'z') str[0] -= 32;  // uppercase it!
  if (str[0] >= 'A' && str[0] <= 'Z' && str[1] >= '0' &&
      str[1] <= '9') {  // only update, if coordinate is valid.
    gExitGrid.ubGotoSectorY = (uint8_t)(str[0] - 'A' + 1);
    gExitGrid.ubGotoSectorX = (uint8_t)(str[1] - '0');
    if (str[2] >= '0' && str[2] <= '9')
      gExitGrid.ubGotoSectorX = (uint8_t)(gExitGrid.ubGotoSectorX * 10 + str[2] - '0');
    gExitGrid.ubGotoSectorX = (uint8_t)max(min(gExitGrid.ubGotoSectorX, 16), 1);
    gExitGrid.ubGotoSectorY = (uint8_t)max(min(gExitGrid.ubGotoSectorY, 16), 1);
  }
  gExitGrid.ubGotoSectorZ = (uint8_t)max(min(GetNumericStrictValueFromField(8), 3), 0);
  gExitGrid.usGridNo = (uint16_t)max(min(GetNumericStrictValueFromField(9), 25600), 0);

  UpdateMapInfoFields();
}

BOOLEAN ApplyNewExitGridValuesToTextFields() {
  wchar_t str[10];
  // exit grid input fields
  if (iCurrentTaskbar != TASK_MAPINFO) return FALSE;
  swprintf(str, ARR_SIZE(str), L"%c%d", gExitGrid.ubGotoSectorY + 'A' - 1, gExitGrid.ubGotoSectorX);
  SetInputFieldStringWith16BitString(7, str);
  swprintf(str, ARR_SIZE(str), L"%d", gExitGrid.ubGotoSectorZ);
  SetInputFieldStringWith16BitString(8, str);
  swprintf(str, ARR_SIZE(str), L"%d", gExitGrid.usGridNo);
  SetInputFieldStringWith16BitString(9, str);
  SetActiveField(0);
  return TRUE;
}

uint16_t usCurrentExitGridNo = 0;
void LocateNextExitGrid() {
  EXITGRID ExitGrid;
  uint16_t i;
  for (i = usCurrentExitGridNo + 1; i < WORLD_MAX; i++) {
    if (GetExitGrid(i, &ExitGrid)) {
      usCurrentExitGridNo = i;
      CenterScreenAtMapIndex(i);
      return;
    }
  }
  for (i = 0; i < usCurrentExitGridNo; i++) {
    if (GetExitGrid(i, &ExitGrid)) {
      usCurrentExitGridNo = i;
      CenterScreenAtMapIndex(i);
      return;
    }
  }
}

void ChangeLightDefault(int8_t bLightType) {
  UnclickEditorButton(MAPINFO_PRIMETIME_LIGHT + gbDefaultLightType);
  gbDefaultLightType = bLightType;
  ClickEditorButton(MAPINFO_PRIMETIME_LIGHT + gbDefaultLightType);
}
