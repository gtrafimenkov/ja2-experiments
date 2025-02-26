// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/EditorBuildings.h"

#include "BuildDefines.h"
#include "Editor/EditSys.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorModes.h"
#include "Editor/EditorTaskbarUtils.h"
#include "Editor/EditorTerrain.h"  //for access to TerrainTileDrawMode
#include "Editor/EditorUndo.h"
#include "Editor/NewSmooth.h"
#include "Editor/SelectWin.h"
#include "Editor/SmoothingUtils.h"
#include "SGP/Debug.h"
#include "SGP/Video.h"
#include "Tactical/Keys.h"
#include "TileEngine/Environment.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"  //HideStructOfGivenType()
#include "Utils/FontControl.h"
#include "Utils/TextInput.h"

BOOLEAN fBuildingShowRoofs, fBuildingShowWalls, fBuildingShowRoomInfo;
uint16_t usCurrentMode;
uint8_t gubCurrRoomNumber;
uint8_t gubMaxRoomNumber;
BOOLEAN gfEditingDoor;

// BEGINNNING OF BUILDING INITIALIZATION FUNCTIONS
void GameInitEditorBuildingInfo() {
  fBuildingShowRoofs = TRUE;
  fBuildingShowWalls = TRUE;
  fBuildingShowRoomInfo = FALSE;
  usCurrentMode = BUILDING_PLACE_WALLS;
  gubCurrRoomNumber = gubMaxRoomNumber = 1;
}

// BEGINNING OF BUILDING UTILITY FUNCTIONS
void UpdateRoofsView() {
  int32_t x;
  uint16_t usType;
  for (x = 0; x < WORLD_MAX; x++) {
    for (usType = FIRSTROOF; usType <= LASTSLANTROOF; usType++) {
      HideStructOfGivenType(x, usType, (BOOLEAN)(!fBuildingShowRoofs));
    }
  }
  gfRenderWorld = TRUE;
}

void UpdateWallsView() {
  int32_t cnt;
  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    if (fBuildingShowWalls) {
      RemoveWallLevelnodeFlags((int16_t)cnt, LEVELNODE_HIDDEN);
    } else {
      SetWallLevelnodeFlags((int16_t)cnt, LEVELNODE_HIDDEN);
    }
  }
  gfRenderWorld = TRUE;
}

void UpdateBuildingsInfo() {
  // print the headers on top of the columns
  SetFont(SMALLCOMPFONT);
  SetFontForeground(FONT_RED);
  mprintfEditor(112, 362, L"TOGGLE");
  mprintfEditor(114, 372, L"VIEWS");
  SetFontForeground(FONT_YELLOW);
  mprintfEditor(185, 362, L"SELECTION METHOD");
  SetFontForeground(FONT_LTGREEN);
  mprintfEditor(290, 362, L"SMART METHOD");
  SetFontForeground(FONT_LTBLUE);
  mprintfEditor(390, 362, L"BUILDING METHOD");
  SetFontForeground(FONT_GRAY2);
  mprintfEditor(437, 404, L"Room#");
}

// Uses a recursive method to elimate adjacent tiles of structure information.
// The code will attempt to delete the current mapindex, then search using this method:
// 1) if there isn't structure info here, return.
// 2) if there is structure info here, delete it now.
// 3) KillBuilding at x-1, y.
// 4) KillBuilding at x  , y-1.
// 5) KillBuilding at x+1, y.
// 6) KillBuilding at x  , y+1.
void KillBuilding(uint32_t iMapIndex) {
  BOOLEAN fFound = FALSE;

  if (!gfBasement) fFound |= RemoveAllRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, LASTITEM);
  fFound |= RemoveAllLandsOfTypeRange(iMapIndex, FIRSTFLOOR, LASTFLOOR);

  EraseBuilding(iMapIndex);
  gubWorldRoomInfo[iMapIndex] = 0;

  if (!fFound) {
    if (gfBasement) RebuildRoof(iMapIndex, 0);
    return;
  }

  if (GridNoOnVisibleWorldTile((uint16_t)(iMapIndex - WORLD_COLS)))
    KillBuilding(iMapIndex - WORLD_COLS);
  if (GridNoOnVisibleWorldTile((uint16_t)(iMapIndex + WORLD_COLS)))
    KillBuilding(iMapIndex + WORLD_COLS);
  if (GridNoOnVisibleWorldTile((uint16_t)(iMapIndex + 1))) KillBuilding(iMapIndex + 1);
  if (GridNoOnVisibleWorldTile((uint16_t)(iMapIndex - 1))) KillBuilding(iMapIndex - 1);

  if (gfBasement) RebuildRoof(iMapIndex, 0);
}

BUILDINGLAYOUTNODE *gpBuildingLayoutList = NULL;
int16_t gsBuildingLayoutAnchorGridNo = -1;
extern void RemoveBuildingLayout();

void DeleteBuildingLayout() {
  BUILDINGLAYOUTNODE *curr;
  // Erases the cursors associated with them.
  RemoveBuildingLayout();
  while (gpBuildingLayoutList) {
    curr = gpBuildingLayoutList;
    gpBuildingLayoutList = gpBuildingLayoutList->next;
    MemFree(curr);
  }
  gpBuildingLayoutList = NULL;
  gsBuildingLayoutAnchorGridNo = -1;
}

void BuildLayout(int32_t iMapIndex, int32_t iOffset) {
  BUILDINGLAYOUTNODE *curr;
  // First, validate the gridno
  iMapIndex += iOffset;
  if (iMapIndex < 0 && iMapIndex >= WORLD_COLS * WORLD_ROWS) return;
  // Now, check if there is a building here
  if (!BuildingAtGridNo(iMapIndex)) {
    if (iOffset == 1 && !BuildingAtGridNo(iMapIndex - 1)) return;
    if (iOffset == WORLD_COLS && !BuildingAtGridNo(iMapIndex - WORLD_COLS)) return;
    if (iOffset == -1 && !GetVerticalWall(iMapIndex)) return;
    if (iOffset == -WORLD_COLS && !GetHorizontalWall(iMapIndex)) return;
  }
  // Now, check to make sure this gridno hasn't already been processed.
  curr = gpBuildingLayoutList;
  while (curr) {
    if ((int16_t)iMapIndex == curr->sGridNo) return;
    curr = curr->next;
  }
  // Good, it hasn't, so process it and add it to the head of the list.
  curr = (BUILDINGLAYOUTNODE *)MemAlloc(sizeof(BUILDINGLAYOUTNODE));
  Assert(curr);
  curr->sGridNo = (int16_t)iMapIndex;
  curr->next = gpBuildingLayoutList;
  gpBuildingLayoutList = curr;

  // Use recursion to process the remainder.
  BuildLayout(iMapIndex, -WORLD_COLS);
  BuildLayout(iMapIndex, -1);
  BuildLayout(iMapIndex, 1);
  BuildLayout(iMapIndex, WORLD_COLS);
}

// The first step is copying a building.  After that, it either must be pasted or moved.
void CopyBuilding(int32_t iMapIndex) {
  AssertMsg(!gpBuildingLayoutList, "Error:  Attempting to copy building multiple times.");

  // First step is to determine if we have a building in the area that we click.  If not, do
  // nothing.
  if (!BuildingAtGridNo(iMapIndex)) return;
  // Okay, a building does exist here to some undetermined capacity.
  // Allocate the basic structure, then calculate the layout.  The head node is
  gpBuildingLayoutList = (BUILDINGLAYOUTNODE *)MemAlloc(sizeof(BUILDINGLAYOUTNODE));
  Assert(gpBuildingLayoutList);
  gpBuildingLayoutList->sGridNo = (int16_t)iMapIndex;
  gpBuildingLayoutList->next = NULL;

  // Set the anchor point for this building -- this is where the user clicked.
  gsBuildingLayoutAnchorGridNo = (int16_t)iMapIndex;

  // Now, recursively expand out while adding unique gridnos to our list.  The recursion will
  // terminate when complete.
  BuildLayout(iMapIndex, -WORLD_COLS);
  BuildLayout(iMapIndex, -1);
  BuildLayout(iMapIndex, 1);
  BuildLayout(iMapIndex, WORLD_COLS);

  // We have our layout.  Now depending on the mode, we will either move the building or
  // copy it.  The layout automatically gets deleted as soon as the user releases the mouse
  // button.
}

// depending on the offset, we will either sort in increasing order, or decreasing order.
// This will prevent overlapping problems.
void SortBuildingLayout(int32_t iMapIndex) {
  BUILDINGLAYOUTNODE *head, *curr, *prev, *prevBest, *best;
  int32_t iBestIndex;
  head = NULL;
  if (iMapIndex < gsBuildingLayoutAnchorGridNo) {  // Forward sort (in increasing order)
    while (gpBuildingLayoutList) {
      iBestIndex = -1;
      curr = gpBuildingLayoutList;
      prev = NULL;
      while (curr) {
        if (iBestIndex < curr->sGridNo) {
          iBestIndex = curr->sGridNo;
          prevBest = prev;
          best = curr;
        }
        prev = curr;
        curr = curr->next;
      }
      // detach node from real list
      if (prevBest) prevBest->next = best->next;
      if (best == gpBuildingLayoutList) gpBuildingLayoutList = gpBuildingLayoutList->next;
      // insert node into temp sorted list
      best->next = head;
      head = best;
    }
  } else {  // Reverse sort (in decreasing order)
    while (gpBuildingLayoutList) {
      iBestIndex = 100000;
      curr = gpBuildingLayoutList;
      prev = NULL;
      while (curr) {
        if (iBestIndex > curr->sGridNo) {
          iBestIndex = curr->sGridNo;
          prevBest = prev;
          best = curr;
        }
        prev = curr;
        curr = curr->next;
      }
      // detach node from real list
      if (prevBest) prevBest->next = best->next;
      if (best == gpBuildingLayoutList) gpBuildingLayoutList = gpBuildingLayoutList->next;
      // insert node into temp sorted list
      best->next = head;
      head = best;
    }
  }
  // Now assign the newly sorted list back to the real list.
  gpBuildingLayoutList = head;
}

void PasteMapElementToNewMapElement(int32_t iSrcGridNo, int32_t iDstGridNo) {
  MAP_ELEMENT *pSrcMapElement;
  struct LEVELNODE *pNode;
  uint16_t usType;

  DeleteStuffFromMapTile(iDstGridNo);
  DeleteAllLandLayers(iDstGridNo);

  // Get a pointer to the src mapelement
  pSrcMapElement = &gpWorldLevelData[iSrcGridNo];

  // Go through each levelnode, and paste the info into the new gridno
  pNode = pSrcMapElement->pLandHead;
  while (pNode) {
    if (pNode == pSrcMapElement->pLandStart)
      gpWorldLevelData[iDstGridNo].pLandStart = AddLandToTail(iDstGridNo, pNode->usIndex);
    else
      AddLandToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  pNode = pSrcMapElement->pObjectHead;
  while (pNode) {
    AddObjectToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  pNode = pSrcMapElement->pStructHead;
  while (pNode) {
    AddStructToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  pNode = pSrcMapElement->pShadowHead;
  while (pNode) {
    AddShadowToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  pNode = pSrcMapElement->pRoofHead;
  while (pNode) {
    AddRoofToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  pNode = pSrcMapElement->pOnRoofHead;
  while (pNode) {
    AddOnRoofToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  pNode = pSrcMapElement->pTopmostHead;
  while (pNode) {
    if (pNode->usIndex != FIRSTPOINTERS1) AddTopmostToTail(iDstGridNo, pNode->usIndex);
    pNode = pNode->pNext;
  }
  for (usType = FIRSTROOF; usType <= LASTSLANTROOF; usType++) {
    HideStructOfGivenType(iDstGridNo, usType, (BOOLEAN)(!fBuildingShowRoofs));
  }
}

void MoveBuilding(int32_t iMapIndex) {
  BUILDINGLAYOUTNODE *curr;
  int32_t iOffset;
  if (!gpBuildingLayoutList) return;
  SortBuildingLayout(iMapIndex);
  iOffset = iMapIndex - gsBuildingLayoutAnchorGridNo;
  // First time, set the undo gridnos to everything effected.
  curr = gpBuildingLayoutList;
  while (curr) {
    AddToUndoList(curr->sGridNo);
    AddToUndoList(curr->sGridNo + iOffset);
    curr = curr->next;
  }
  // Now, move the building
  curr = gpBuildingLayoutList;
  while (curr) {
    PasteMapElementToNewMapElement(curr->sGridNo, curr->sGridNo + iOffset);
    DeleteStuffFromMapTile(curr->sGridNo);
    curr = curr->next;
  }
  MarkWorldDirty();
}

void PasteBuilding(int32_t iMapIndex) {
  BUILDINGLAYOUTNODE *curr;
  int32_t iOffset;
  if (!gpBuildingLayoutList) return;
  SortBuildingLayout(iMapIndex);
  iOffset = iMapIndex - gsBuildingLayoutAnchorGridNo;
  curr = gpBuildingLayoutList;
  // First time, set the undo gridnos to everything effected.
  while (curr) {
    AddToUndoList(curr->sGridNo);
    AddToUndoList(curr->sGridNo + iOffset);
    curr = curr->next;
  }
  // Now, paste the building (no smoothing)
  curr = gpBuildingLayoutList;
  while (curr) {
    PasteMapElementToNewMapElement(curr->sGridNo, curr->sGridNo + iOffset);
    curr = curr->next;
  }
  MarkWorldDirty();
}

typedef struct ROOFNODE {
  int32_t iMapIndex;
  struct ROOFNODE *next;
} ROOFNODE;

ROOFNODE *gpRoofList = NULL;

void ReplaceRoof(int32_t iMapIndex, uint16_t usRoofType) {
  ROOFNODE *curr;
  // First, validate the gridno
  if (iMapIndex < 0 && iMapIndex >= WORLD_COLS * WORLD_ROWS) return;
  // Now, check if there is a floor here
  if (!FloorAtGridNo(iMapIndex)) return;
  // Now, check to make sure this gridno hasn't already been processed.
  curr = gpRoofList;
  while (curr) {
    if (iMapIndex == curr->iMapIndex) return;
    curr = curr->next;
  }
  // Good, it hasn't, so process it and add it to the head of the list.
  curr = (ROOFNODE *)MemAlloc(sizeof(ROOFNODE));
  Assert(curr);
  curr->iMapIndex = iMapIndex;
  curr->next = gpRoofList;
  gpRoofList = curr;

  RebuildRoofUsingFloorInfo(iMapIndex, usRoofType);

  // Use recursion to process the remainder.
  ReplaceRoof(iMapIndex - WORLD_COLS, usRoofType);
  ReplaceRoof(iMapIndex + WORLD_COLS, usRoofType);
  ReplaceRoof(iMapIndex - 1, usRoofType);
  ReplaceRoof(iMapIndex + 1, usRoofType);
}

void ReplaceBuildingWithNewRoof(int32_t iMapIndex) {
  uint16_t usRoofType;
  ROOFNODE *curr;
  // Not in normal editor mode, then can't do it.
  if (gfBasement || gfCaves) return;
  // if we don't have a floor here, then we can't replace the roof!
  if (!FloorAtGridNo(iMapIndex)) return;
  // Extract the selected roof type.
  usRoofType = (uint16_t)SelSingleNewRoof[iCurBank].uiObject;

  // now start building a linked list of all nodes visited -- start the first node.
  gpRoofList = (ROOFNODE *)MemAlloc(sizeof(ROOFNODE));
  Assert(gpRoofList);
  gpRoofList->iMapIndex = iMapIndex;
  gpRoofList->next = 0;
  RebuildRoofUsingFloorInfo(iMapIndex, usRoofType);

  // Use recursion to process the remainder.
  ReplaceRoof(iMapIndex - WORLD_COLS, usRoofType);
  ReplaceRoof(iMapIndex + WORLD_COLS, usRoofType);
  ReplaceRoof(iMapIndex - 1, usRoofType);
  ReplaceRoof(iMapIndex + 1, usRoofType);

  // Done, so delete the list.
  while (gpRoofList) {
    curr = gpRoofList;
    gpRoofList = gpRoofList->next;
    MemFree(curr);
  }
  gpRoofList = NULL;
}

// internal door editing vars.
int32_t iDoorMapIndex = 0;
enum { DOOR_BACKGROUND, DOOR_OKAY, DOOR_CANCEL, DOOR_LOCKED, NUM_DOOR_BUTTONS };
int32_t iDoorButton[NUM_DOOR_BUTTONS];
struct MOUSE_REGION DoorRegion;
void DoorOkayCallback(GUI_BUTTON *btn, int32_t reason);
void DoorCancelCallback(GUI_BUTTON *btn, int32_t reason);
void DoorToggleLockedCallback(GUI_BUTTON *btn, int32_t reason);

extern BOOLEAN OpenableAtGridNo(uint32_t iMapIndex);

void InitDoorEditing(int32_t iMapIndex) {
  DOOR *pDoor;
  if (!DoorAtGridNo(iMapIndex) && !OpenableAtGridNo(iMapIndex)) return;
  gfEditingDoor = TRUE;
  iDoorMapIndex = iMapIndex;
  DisableEditorTaskbar();
  MSYS_DefineRegion(&DoorRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGH - 2, 0, MSYS_NO_CALLBACK,
                    MSYS_NO_CALLBACK);
  iDoorButton[DOOR_BACKGROUND] =
      CreateTextButton(0, 0, 0, 0, BUTTON_USE_DEFAULT, 200, 130, 240, 100, BUTTON_TOGGLE,
                       MSYS_PRIORITY_HIGH - 1, BUTTON_NO_CALLBACK, BUTTON_NO_CALLBACK);
  DisableButton(iDoorButton[DOOR_BACKGROUND]);
  SpecifyDisabledButtonStyle(iDoorButton[DOOR_BACKGROUND], DISABLED_STYLE_NONE);
  iDoorButton[DOOR_OKAY] = CreateTextButton(
      L"Okay", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT, 330, 195, 50, 30,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, DoorOkayCallback);
  iDoorButton[DOOR_CANCEL] = CreateTextButton(
      L"Cancel", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT, 385, 195, 50, 30,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, DoorCancelCallback);
  InitTextInputModeWithScheme(DEFAULT_SCHEME);
  AddTextInputField(210, 155, 25, 16, MSYS_PRIORITY_HIGH, L"0", 3, INPUTTYPE_NUMERICSTRICT);
  AddTextInputField(210, 175, 25, 16, MSYS_PRIORITY_HIGH, L"0", 2, INPUTTYPE_NUMERICSTRICT);
  AddTextInputField(210, 195, 25, 16, MSYS_PRIORITY_HIGH, L"0", 2, INPUTTYPE_NUMERICSTRICT);
  iDoorButton[DOOR_LOCKED] = CreateCheckBoxButton(210, 215, "EDITOR//SmCheckbox.sti",
                                                  MSYS_PRIORITY_HIGH, DoorToggleLockedCallback);

  pDoor = FindDoorInfoAtGridNo(iDoorMapIndex);
  if (pDoor) {
    if (pDoor->fLocked) {
      ButtonList[iDoorButton[DOOR_LOCKED]]->uiFlags |= BUTTON_CLICKED_ON;
    }
    SetInputFieldStringWithNumericStrictValue(0, pDoor->ubLockID);
    SetInputFieldStringWithNumericStrictValue(1, pDoor->ubTrapID);
    SetInputFieldStringWithNumericStrictValue(2, pDoor->ubTrapLevel);
  } else {
    ButtonList[iDoorButton[DOOR_LOCKED]]->uiFlags |= BUTTON_CLICKED_ON;
  }
}

void ExtractAndUpdateDoorInfo() {
  struct LEVELNODE *pNode;
  int32_t num;
  DOOR door;
  BOOLEAN fCursor = FALSE;
  BOOLEAN fCursorExists = FALSE;

  memset(&door, 0, sizeof(DOOR));

  door.sGridNo = (int16_t)iDoorMapIndex;

  num = min(GetNumericStrictValueFromField(0), NUM_LOCKS - 1);
  door.ubLockID = (uint8_t)num;
  SetInputFieldStringWithNumericStrictValue(0, num);
  if (num >= 0) fCursor = TRUE;

  num = min(max(GetNumericStrictValueFromField(1), 0), 10);
  door.ubTrapID = (uint8_t)num;
  SetInputFieldStringWithNumericStrictValue(1, num);
  if (num) fCursor = TRUE;

  num = min(max(GetNumericStrictValueFromField(2), 0), 20);
  if (door.ubTrapID && !num) num = 1;  // Can't have a trap without a traplevel!
  door.ubTrapLevel = (uint8_t)num;
  SetInputFieldStringWithNumericStrictValue(2, num);
  if (num) fCursor = TRUE;

  if (ButtonList[iDoorButton[DOOR_LOCKED]]->uiFlags & BUTTON_CLICKED_ON) {
    door.fLocked = TRUE;
  } else {
    door.fLocked = FALSE;
  }

  // Find out if we have a rotating key cursor (we will either add one or remove one)
  pNode = gpWorldLevelData[iDoorMapIndex].pTopmostHead;
  while (pNode) {
    if (pNode->usIndex == ROTATINGKEY1) {
      fCursorExists = TRUE;
      break;
    }
    pNode = pNode->pNext;
  }
  if (fCursor) {  // we have a valid door, so add it (or replace existing)
    if (!fCursorExists) AddTopmostToHead(iDoorMapIndex, ROTATINGKEY1);
    // If the door already exists, the new information will replace it.
    AddDoorInfoToTable(&door);
  } else {  // if a door exists here, remove it.
    if (fCursorExists) RemoveAllTopmostsOfTypeRange(iDoorMapIndex, ROTATINGKEY, ROTATINGKEY);
    RemoveDoorInfoFromTable(iDoorMapIndex);
  }
}

void FindNextLockedDoor() {
  DOOR *pDoor;
  int32_t i;
  for (i = iDoorMapIndex + 1; i < WORLD_MAX; i++) {
    pDoor = FindDoorInfoAtGridNo(i);
    if (pDoor) {
      CenterScreenAtMapIndex(i);
      iDoorMapIndex = i;
      return;
    }
  }
  for (i = 0; i <= iDoorMapIndex; i++) {
    pDoor = FindDoorInfoAtGridNo(i);
    if (pDoor) {
      CenterScreenAtMapIndex(i);
      iDoorMapIndex = i;
      return;
    }
  }
}

void RenderDoorEditingWindow() {
  InvalidateRegion(200, 130, 440, 230);
  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_YELLOW);
  SetFontShadow(FONT_NEARBLACK);
  SetFontBackground(0);
  mprintf(210, 140, L"Editing lock attributes at map index %d.", iDoorMapIndex);

  SetFontForeground(FONT_GRAY2);
  mprintf(238, 160, L"Lock ID");
  mprintf(238, 180, L"Trap Type");
  mprintf(238, 200, L"Trap Level");
  mprintf(238, 218, L"Locked");
}

void KillDoorEditing() {
  int32_t i;
  EnableEditorTaskbar();
  MSYS_RemoveRegion(&DoorRegion);
  for (i = 0; i < NUM_DOOR_BUTTONS; i++) RemoveButton(iDoorButton[i]);
  gfEditingDoor = FALSE;
  KillTextInputMode();
}

void DoorOkayCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    ExtractAndUpdateDoorInfo();
    KillDoorEditing();
  }
}

void DoorCancelCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    KillDoorEditing();
  }
}

void DoorToggleLockedCallback(GUI_BUTTON *btn, int32_t reason) {
  // handled in ExtractAndUpdateDoorInfo();
}

void AddLockedDoorCursors() {
  DOOR *pDoor;
  int i;
  for (i = 0; i < gubNumDoors; i++) {
    pDoor = &DoorTable[i];
    AddTopmostToHead(pDoor->sGridNo, ROTATINGKEY1);
  }
}

void RemoveLockedDoorCursors() {
  DOOR *pDoor;
  int i;
  struct LEVELNODE *pNode;
  struct LEVELNODE *pTemp;
  for (i = 0; i < gubNumDoors; i++) {
    pDoor = &DoorTable[i];
    pNode = gpWorldLevelData[pDoor->sGridNo].pTopmostHead;
    while (pNode) {
      if (pNode->usIndex == ROTATINGKEY1) {
        pTemp = pNode;
        pNode = pNode->pNext;
        RemoveTopmost(pDoor->sGridNo, pTemp->usIndex);
      } else
        pNode = pNode->pNext;
    }
  }
}

void SetupTextInputForBuildings() {
  wchar_t str[4];
  InitTextInputModeWithScheme(DEFAULT_SCHEME);
  AddUserInputField(NULL);  // just so we can use short cut keys while not typing.
  swprintf(str, ARR_SIZE(str), L"%d", gubMaxRoomNumber);
  AddTextInputField(410, 400, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT);
}

void ExtractAndUpdateBuildingInfo() {
  wchar_t str[4];
  int32_t temp;
  // extract light1 colors
  temp = min(GetNumericStrictValueFromField(1), 255);
  if (temp != -1) {
    gubCurrRoomNumber = (uint8_t)temp;
  } else {
    gubCurrRoomNumber = 0;
  }
  swprintf(str, ARR_SIZE(str), L"%d", gubCurrRoomNumber);
  SetInputFieldStringWith16BitString(1, str);
  SetActiveField(0);
}
