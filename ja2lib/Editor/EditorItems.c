// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/EditorItems.h"

#include "BuildDefines.h"
#include "Editor/EditScreen.h"
#include "Editor/EditSys.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorMercs.h"
#include "Editor/EditorTaskbarUtils.h"
#include "Editor/ItemStatistics.h"
#include "Editor/SelectWin.h"
#include "SGP/Font.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Tactical/ActionItems.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Keys.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/Weapons.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/Pits.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define NUMBER_TRIGGERS 27
#define PRESSURE_ACTION_ID (NUMBER_TRIGGERS - 1)

extern struct ITEM_POOL *gpEditingItemPool;

// Simply counts the number of items in the world.  This is used for display purposes.
uint16_t CountNumberOfEditorPlacementsInWorld(uint16_t usEInfoIndex,
                                              uint16_t *pusQuantity);  // wrapper for the next three
uint16_t CountNumberOfItemPlacementsInWorld(uint16_t usItem, uint16_t *pusQuantity);
uint16_t CountNumberOfItemsWithFrequency(uint16_t usItem, int8_t bFrequency);
uint16_t CountNumberOfPressureActionsInWorld();
uint16_t CountNumberOfKeysOfTypeInWorld(uint8_t ubKeyID);

// Finds and selects the next item when right clicking on an item type.  Only works if the
// item actually exists in the world.
void FindNextItemOfSelectedType();  // wrapper for the next four
void SelectNextTriggerWithFrequency(uint16_t usItem, int8_t bFrequency);
void SelectNextItemOfType(uint16_t usItem);
void SelectNextPressureAction();
void SelectNextKeyOfType(uint8_t ubKeyID);

int32_t giDefaultExistChance = 100;

typedef struct IPListNode {
  int16_t sGridNo;
  struct IPListNode *next;
} IPListNode;

IPListNode *pIPHead = NULL;

IPListNode *gpCurrItemPoolNode = NULL;
struct ITEM_POOL *gpItemPool = NULL;

void BuildItemPoolList() {
  struct ITEM_POOL *temp;
  IPListNode *tail;
  uint16_t i;
  KillItemPoolList();
  for (i = 0; i < WORLD_MAX; i++) {
    if (GetItemPool(i, &temp, 0)) {
      if (!pIPHead) {
        pIPHead = (IPListNode *)MemAlloc(sizeof(IPListNode));
        Assert(pIPHead);
        tail = pIPHead;
      } else {
        tail->next = (IPListNode *)MemAlloc(sizeof(IPListNode));
        Assert(tail->next);
        tail = tail->next;
      }
      ShowItemCursor(i);
      tail->sGridNo = i;
      tail->next = NULL;
    }
  }
  gpCurrItemPoolNode = pIPHead;
  SpecifyItemToEdit(NULL, -1);
}

void KillItemPoolList() {
  IPListNode *pIPCurr;
  pIPCurr = pIPHead;
  while (pIPCurr) {
    HideItemCursor(pIPCurr->sGridNo);
    pIPHead = pIPHead->next;
    MemFree(pIPCurr);
    pIPCurr = pIPHead;
  }
  pIPHead = NULL;
}

// Contains global information about the editor items
// May be expanded to encapsulate the entire editor later.
EditorItemsInfo eInfo;

// Does some precalculations regarding the number of each item type, so that it
// isn't calculated every time a player changes categories.
void EntryInitEditorItemsInfo() {
  int32_t i;
  INVTYPE *item;
  eInfo.vsBuffer = 0;
  eInfo.fKill = 0;
  eInfo.fActive = 0;
  eInfo.sWidth = 0;
  eInfo.sHeight = 0;
  eInfo.sScrollIndex = 0;
  eInfo.sSelItemIndex = 0;
  eInfo.sHilitedItemIndex = -1;
  eInfo.sNumItems = 0;
  eInfo.pusItemIndex = NULL;
  if (eInfo.fGameInit) {  // This only gets called one time in game execution.
    memset(&eInfo, 0, sizeof(EditorItemsInfo));
    eInfo.sHilitedItemIndex = -1;
    eInfo.uiItemType = TBAR_MODE_ITEM_WEAPONS;
    // Pre-calculate the number of each item type.
    eInfo.sNumTriggers = NUMBER_TRIGGERS;
    for (i = 0; i < MAXITEMS; i++) {
      item = &Item[i];
      if (Item[i].fFlags & ITEM_NOT_EDITOR) continue;
      if (i == SWITCH || i == ACTION_ITEM) {
      } else
        switch (item->usItemClass) {
          case IC_GUN:
          case IC_BLADE:
          case IC_THROWN:
          case IC_LAUNCHER:
          case IC_THROWING_KNIFE:
            eInfo.sNumWeapons++;
            break;
          case IC_PUNCH:
            if (i != NOTHING) {
              eInfo.sNumWeapons++;
            }
            break;
          case IC_AMMO:
            eInfo.sNumAmmo++;
            break;
          case IC_ARMOUR:
            eInfo.sNumArmour++;
            break;
          case IC_GRENADE:
          case IC_BOMB:
            eInfo.sNumExplosives++;
            break;
          case IC_MEDKIT:
          case IC_KIT:
          case IC_FACE:
          case IC_MISC:
          case IC_MONEY:
            if (eInfo.sNumEquipment1 < 30)
              eInfo.sNumEquipment1++;
            else if (eInfo.sNumEquipment2 < 30)
              eInfo.sNumEquipment2++;
            else
              eInfo.sNumEquipment3++;
            break;
            // case IC_KEY:
            //	eInfo.sNumKeys++;
            //	break;
        }
    }
    eInfo.sNumKeys = NUM_KEYS;
  }
}

void InitEditorItemsInfo(uint32_t uiItemType) {
  uint8_t *pDestBuf, *pSrcBuf;
  uint32_t uiSrcPitchBYTES, uiDestPitchBYTES;
  INVTYPE *item;
  SGPRect SaveRect, NewRect;
  struct VObject *hVObject;
  uint32_t uiVideoObjectIndex;
  int16_t sWidth, sOffset, sStart;
  int16_t i, x, y;
  uint16_t usCounter;
  wchar_t pStr[100];  //, pStr2[ 100 ];
  wchar_t pItemName[SIZE_ITEM_NAME];
  BOOLEAN fTypeMatch;
  int32_t iEquipCount = 0;

  // Check to make sure that there isn't already a valid eInfo
  if (eInfo.fActive) {
    if (eInfo.uiItemType == uiItemType) {  // User clicked on the same item classification -- ignore
      return;
    } else {  // User selected a different item classification -- delete it first.
      ClearEditorItemsInfo();
      ClearTaskbarRegion(100, 360, 480, 440);
    }
  } else {
    // Clear the menu area, so that the buffer doesn't get corrupted.
    ClearTaskbarRegion(100, 360, 480, 440);
  }
  EnableEditorRegion(ITEM_REGION_ID);

  eInfo.uiItemType = uiItemType;
  eInfo.fActive = TRUE;
  // Begin initialization of data.
  switch (uiItemType) {
    case TBAR_MODE_ITEM_WEAPONS:
      eInfo.sNumItems = eInfo.sNumWeapons;
      eInfo.sScrollIndex = eInfo.sSaveWeaponsScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelWeaponsIndex;
      break;
    case TBAR_MODE_ITEM_AMMO:
      eInfo.sNumItems = eInfo.sNumAmmo;
      eInfo.sScrollIndex = eInfo.sSaveAmmoScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelAmmoIndex;
      break;
    case TBAR_MODE_ITEM_ARMOUR:
      eInfo.sNumItems = eInfo.sNumArmour;
      eInfo.sScrollIndex = eInfo.sSaveArmourScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelArmourIndex;
      break;
    case TBAR_MODE_ITEM_EXPLOSIVES:
      eInfo.sNumItems = eInfo.sNumExplosives;
      eInfo.sScrollIndex = eInfo.sSaveExplosivesScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelExplosivesIndex;
      break;
    case TBAR_MODE_ITEM_EQUIPMENT1:
      eInfo.sNumItems = eInfo.sNumEquipment1;
      eInfo.sScrollIndex = eInfo.sSaveEquipment1ScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelEquipment1Index;
      break;
    case TBAR_MODE_ITEM_EQUIPMENT2:
      eInfo.sNumItems = eInfo.sNumEquipment2;
      eInfo.sScrollIndex = eInfo.sSaveEquipment2ScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelEquipment2Index;
      break;
    case TBAR_MODE_ITEM_EQUIPMENT3:
      eInfo.sNumItems = eInfo.sNumEquipment3;
      eInfo.sScrollIndex = eInfo.sSaveEquipment3ScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelEquipment3Index;
      break;
    case TBAR_MODE_ITEM_TRIGGERS:
      eInfo.sNumItems = eInfo.sNumTriggers;
      eInfo.sScrollIndex = eInfo.sSaveTriggersScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelTriggersIndex;
      break;
    case TBAR_MODE_ITEM_KEYS:
      eInfo.sNumItems = eInfo.sNumKeys;
      eInfo.sScrollIndex = eInfo.sSaveKeysScrollIndex;
      eInfo.sSelItemIndex = eInfo.sSaveSelKeysIndex;
      break;
    default:
      // error
      return;
  }
  // Allocate memory to store all the item pointers.
  eInfo.pusItemIndex = (uint16_t *)MemAlloc(sizeof(uint16_t) * eInfo.sNumItems);

  // Disable the appropriate scroll buttons based on the saved scroll index if applicable
  // Left most scroll position
  DetermineItemsScrolling();
  // calculate the width of the buffer based on the number of items.
  // every pair of items (odd rounded up) requires 60 pixels for width.
  // the minimum buffer size is 420.  Height is always 80 pixels.
  eInfo.sWidth = (eInfo.sNumItems > 12) ? ((eInfo.sNumItems + 1) / 2) * 60 : 360;
  eInfo.sHeight = 80;
  // Create item buffer
  //!!!Memory check.  Create the item buffer
  eInfo.vsBuffer = JSurface_Create16bpp(eInfo.sWidth, eInfo.sHeight);
  if (eInfo.vsBuffer == NULL) {
    eInfo.fKill = TRUE;
    eInfo.fActive = FALSE;
    return;
  }
  JSurface_SetColorKey(eInfo.vsBuffer, FROMRGB(0, 0, 0));

  pDestBuf = LockVSurface(eInfo.vsBuffer, &uiDestPitchBYTES);
  pSrcBuf = LockVSurface(vsFB, &uiSrcPitchBYTES);

  // copy a blank chunk of the editor interface to the new buffer.
  for (i = 0; i < eInfo.sWidth; i += 60) {
    Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES,
                    0 + i, 0, 100, 360, 60, 80);
  }

  JSurface_Unlock(eInfo.vsBuffer);
  JSurface_Unlock(vsFB);

  x = 0;
  y = 0;
  usCounter = 0;
  NewRect.iTop = 0;
  NewRect.iBottom = eInfo.sHeight;
  NewRect.iLeft = 0;
  NewRect.iRight = eInfo.sWidth;
  GetClippingRect(&SaveRect);
  SetClippingRect(&NewRect);
  if (eInfo.uiItemType ==
      TBAR_MODE_ITEM_KEYS) {  // Keys use a totally different method for determining
    for (i = 0; i < eInfo.sNumItems; i++) {
      item = &Item[KeyTable[0].usItem + LockTable[i].usKeyItem];
      uiVideoObjectIndex = GetInterfaceGraphicForItem(item);
      GetVideoObject(&hVObject, uiVideoObjectIndex);

      // Store these item pointers for later when rendering selected items.
      eInfo.pusItemIndex[i] = KeyTable[0].usItem + LockTable[i].usKeyItem;

      SetFont(SMALLCOMPFONT);
      SetFontForeground(FONT_MCOLOR_WHITE);
      SetFontDestBuffer(eInfo.vsBuffer, 0, 0, eInfo.sWidth, eInfo.sHeight, FALSE);

      swprintf(pStr, ARR_SIZE(pStr), L"%S", LockTable[i].ubEditorName);
      DisplayWrappedString(x, (uint16_t)(y + 25), 60, 2, SMALLCOMPFONT, FONT_WHITE, pStr,
                           FONT_BLACK, TRUE, CENTER_JUSTIFIED);

      // Calculate the center position of the graphic in a 60 pixel wide area.
      sWidth = hVObject->pETRLEObject[item->ubGraphicNum].usWidth;
      sOffset = hVObject->pETRLEObject[item->ubGraphicNum].sOffsetX;
      sStart = x + (60 - sWidth - sOffset * 2) / 2;

      BltVideoObjectOutlineFromIndex(eInfo.vsBuffer, uiVideoObjectIndex, item->ubGraphicNum, sStart,
                                     y + 2, 0, FALSE);
      // cycle through the various slot positions (0,0), (0,40), (60,0), (60,40), (120,0)...
      if (y == 0) {
        y = 40;
      } else {
        y = 0;
        x += 60;
      }
    }
  } else
    for (i = 0; i < eInfo.sNumItems; i++) {
      fTypeMatch = FALSE;
      while (usCounter < MAXITEMS && !fTypeMatch) {
        item = &Item[usCounter];
        if (Item[usCounter].fFlags & ITEM_NOT_EDITOR) {
          usCounter++;
          continue;
        }
        if (eInfo.uiItemType == TBAR_MODE_ITEM_TRIGGERS) {
          if (i < PRESSURE_ACTION_ID)
            usCounter = (i % 2) ? ACTION_ITEM : SWITCH;
          else
            usCounter = ACTION_ITEM;
          fTypeMatch = TRUE;
          item = &Item[usCounter];
        } else
          switch (item->usItemClass) {
            case IC_GUN:
            case IC_BLADE:
            case IC_LAUNCHER:
            case IC_THROWN:
            case IC_THROWING_KNIFE:
              fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_WEAPONS;
              break;
            case IC_PUNCH:
              if (i != NOTHING) {
                fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_WEAPONS;
              } else {
                fTypeMatch = FALSE;
              }
              break;
            case IC_AMMO:
              fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_AMMO;
              break;
            case IC_ARMOUR:
              fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_ARMOUR;
              break;
            case IC_GRENADE:
            case IC_BOMB:
              fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EXPLOSIVES;
              break;
            case IC_MEDKIT:
            case IC_KIT:
            case IC_FACE:
            case IC_MISC:
            case IC_MONEY:
              if (usCounter == ACTION_ITEM || usCounter == SWITCH) break;
              if (iEquipCount < 30)
                fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EQUIPMENT1;
              else if (iEquipCount < 60)
                fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EQUIPMENT2;
              else
                fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EQUIPMENT3;
              iEquipCount++;
              break;
          }
        if (fTypeMatch) {
          uiVideoObjectIndex = GetInterfaceGraphicForItem(item);
          GetVideoObject(&hVObject, uiVideoObjectIndex);

          // Store these item pointers for later when rendering selected items.
          eInfo.pusItemIndex[i] = usCounter;

          SetFont(SMALLCOMPFONT);
          SetFontForeground(FONT_MCOLOR_WHITE);
          SetFontDestBuffer(eInfo.vsBuffer, 0, 0, eInfo.sWidth, eInfo.sHeight, FALSE);

          if (eInfo.uiItemType != TBAR_MODE_ITEM_TRIGGERS) {
            LoadItemInfo(usCounter, pItemName, NULL);
            swprintf(pStr, ARR_SIZE(pStr), L"%s", pItemName);
          } else {
            if (i == PRESSURE_ACTION_ID) {
              swprintf(pStr, ARR_SIZE(pStr), L"Pressure Action");
            } else if (i < 2) {
              if (usCounter == SWITCH)
                swprintf(pStr, ARR_SIZE(pStr), L"Panic Trigger1");
              else
                swprintf(pStr, ARR_SIZE(pStr), L"Panic Action1");
            } else if (i < 4) {
              if (usCounter == SWITCH)
                swprintf(pStr, ARR_SIZE(pStr), L"Panic Trigger2");
              else
                swprintf(pStr, ARR_SIZE(pStr), L"Panic Action2");
            } else if (i < 6) {
              if (usCounter == SWITCH)
                swprintf(pStr, ARR_SIZE(pStr), L"Panic Trigger3");
              else
                swprintf(pStr, ARR_SIZE(pStr), L"Panic Action3");
            } else {
              if (usCounter == SWITCH)
                swprintf(pStr, ARR_SIZE(pStr), L"Trigger%d", (i - 4) / 2);
              else
                swprintf(pStr, ARR_SIZE(pStr), L"Action%d", (i - 4) / 2);
            }
          }
          DisplayWrappedString(x, (uint16_t)(y + 25), 60, 2, SMALLCOMPFONT, FONT_WHITE, pStr,
                               FONT_BLACK, TRUE, CENTER_JUSTIFIED);

          // Calculate the center position of the graphic in a 60 pixel wide area.
          sWidth = hVObject->pETRLEObject[item->ubGraphicNum].usWidth;
          sOffset = hVObject->pETRLEObject[item->ubGraphicNum].sOffsetX;
          sStart = x + (60 - sWidth - sOffset * 2) / 2;

          if (sWidth) {
            BltVideoObjectOutlineFromIndex(eInfo.vsBuffer, uiVideoObjectIndex, item->ubGraphicNum,
                                           sStart, y + 2, 0, FALSE);
          }
          // cycle through the various slot positions (0,0), (0,40), (60,0), (60,40), (120,0)...
          if (y == 0) {
            y = 40;
          } else {
            y = 0;
            x += 60;
          }
        }
        usCounter++;
      }
    }
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
  SetClippingRect(&SaveRect);
  gfRenderTaskbar = TRUE;
}

void DetermineItemsScrolling() {
  if (!eInfo.sScrollIndex)
    DisableEditorButton(ITEMS_LEFTSCROLL);
  else
    EnableEditorButton(ITEMS_LEFTSCROLL);
  // Right most scroll position.  Calculated by taking every pair of numItems rounded up,
  // and subtracting 7 (because a scroll index 0 is disabled if there are <=12 items,
  // index 1 for <=14 items, index 2 for <=16 items...
  if (eInfo.sScrollIndex == max(((eInfo.sNumItems + 1) / 2) - 6, 0))
    DisableEditorButton(ITEMS_RIGHTSCROLL);
  else
    EnableEditorButton(ITEMS_RIGHTSCROLL);
}

void RenderEditorItemsInfo() {
  uint8_t *pDestBuf, *pSrcBuf;
  uint32_t uiSrcPitchBYTES, uiDestPitchBYTES;
  INVTYPE *item;
  struct VObject *hVObject;
  uint32_t uiVideoObjectIndex;
  int16_t i;
  int16_t minIndex, maxIndex;
  int16_t sWidth, sOffset, sStart, x, y;
  uint16_t usNumItems;
  uint16_t usQuantity;

  if (!eInfo.fActive) {
    return;
  }
  if (gusMouseXPos < 110 || gusMouseXPos > 480 || gusMouseYPos < 360 ||
      gusMouseYPos > 440) {  // Mouse has moved out of the items display region -- so nothing can be
                             // highlighted.
    eInfo.sHilitedItemIndex = -1;
  }
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  pSrcBuf = LockVSurface(eInfo.vsBuffer, &uiSrcPitchBYTES);

  // copy the items buffer to the editor bar
  Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES, 110,
                  360, 60 * eInfo.sScrollIndex, 0, 360, 80);

  JSurface_Unlock(eInfo.vsBuffer);
  JSurface_Unlock(vsFB);

  // calculate the min and max index that is currently shown.  This determines
  // if the highlighted and/or selected items are drawn with the outlines.
  minIndex = eInfo.sScrollIndex * 2;
  maxIndex = minIndex + 11;

  // draw the hilighted item if applicable.
  if (eInfo.sHilitedItemIndex >= minIndex && eInfo.sHilitedItemIndex <= maxIndex) {
    if (eInfo.pusItemIndex) {
      item = &Item[eInfo.pusItemIndex[eInfo.sHilitedItemIndex]];
      uiVideoObjectIndex = GetInterfaceGraphicForItem(item);
      GetVideoObject(&hVObject, uiVideoObjectIndex);
      x = (eInfo.sHilitedItemIndex / 2 - eInfo.sScrollIndex) * 60 + 110;
      y = 360 + (eInfo.sHilitedItemIndex % 2) * 40;
      sWidth = hVObject->pETRLEObject[item->ubGraphicNum].usWidth;
      sOffset = hVObject->pETRLEObject[item->ubGraphicNum].sOffsetX;
      sStart = x + (60 - sWidth - sOffset * 2) / 2;
      if (sWidth) {
        BltVideoObjectOutlineFromIndex(vsFB, uiVideoObjectIndex, item->ubGraphicNum, sStart, y + 2,
                                       rgb32_to_rgb16(FROMRGB(250, 250, 0)), TRUE);
      }
    }
  }
  // draw the selected item
  if (eInfo.sSelItemIndex >= minIndex && eInfo.sSelItemIndex <= maxIndex) {
    if (eInfo.pusItemIndex) {
      item = &Item[eInfo.pusItemIndex[eInfo.sSelItemIndex]];
      uiVideoObjectIndex = GetInterfaceGraphicForItem(item);
      GetVideoObject(&hVObject, uiVideoObjectIndex);
      x = (eInfo.sSelItemIndex / 2 - eInfo.sScrollIndex) * 60 + 110;
      y = 360 + (eInfo.sSelItemIndex % 2) * 40;
      sWidth = hVObject->pETRLEObject[item->ubGraphicNum].usWidth;
      sOffset = hVObject->pETRLEObject[item->ubGraphicNum].sOffsetX;
      sStart = x + (60 - sWidth - sOffset * 2) / 2;
      if (sWidth) {
        BltVideoObjectOutlineFromIndex(vsFB, uiVideoObjectIndex, item->ubGraphicNum, sStart, y + 2,
                                       rgb32_to_rgb16(FROMRGB(250, 0, 0)), TRUE);
      }
    }
  }
  // draw the numbers of each visible item that currently resides in the world.
  maxIndex = min(maxIndex, eInfo.sNumItems - 1);
  for (i = minIndex; i <= maxIndex; i++) {
    usNumItems = CountNumberOfEditorPlacementsInWorld(i, &usQuantity);
    if (usNumItems) {
      x = (i / 2 - eInfo.sScrollIndex) * 60 + 110;
      y = 360 + (i % 2) * 40;
      SetFont(FONT10ARIAL);
      SetFontForeground(FONT_YELLOW);
      SetFontShadow(FONT_NEARBLACK);
      if (usNumItems == usQuantity)
        mprintf(x + 12, y + 4, L"%d", usNumItems);
      else
        mprintf(x + 12, y + 4, L"%d(%d)", usNumItems, usQuantity);
    }
  }
}

void ClearEditorItemsInfo() {
  if (eInfo.vsBuffer) {
    JSurface_Free(eInfo.vsBuffer);
    eInfo.vsBuffer = NULL;
    eInfo.vsBuffer = 0;
  }
  if (eInfo.pusItemIndex) {
    MemFree(eInfo.pusItemIndex);
    eInfo.pusItemIndex = NULL;
  }
  DisableEditorRegion(ITEM_REGION_ID);
  eInfo.fKill = 0;
  eInfo.fActive = 0;
  eInfo.sWidth = 0;
  eInfo.sHeight = 0;
  eInfo.sNumItems = 0;
  // save the highlighted selections
  switch (eInfo.uiItemType) {
    case TBAR_MODE_ITEM_WEAPONS:
      eInfo.sSaveSelWeaponsIndex = eInfo.sSelItemIndex;
      eInfo.sSaveWeaponsScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_AMMO:
      eInfo.sSaveSelAmmoIndex = eInfo.sSelItemIndex;
      eInfo.sSaveAmmoScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_ARMOUR:
      eInfo.sSaveSelArmourIndex = eInfo.sSelItemIndex;
      eInfo.sSaveArmourScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_EXPLOSIVES:
      eInfo.sSaveSelExplosivesIndex = eInfo.sSelItemIndex;
      eInfo.sSaveExplosivesScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_EQUIPMENT1:
      eInfo.sSaveSelEquipment1Index = eInfo.sSelItemIndex;
      eInfo.sSaveEquipment1ScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_EQUIPMENT2:
      eInfo.sSaveSelEquipment2Index = eInfo.sSelItemIndex;
      eInfo.sSaveEquipment2ScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_EQUIPMENT3:
      eInfo.sSaveSelEquipment3Index = eInfo.sSelItemIndex;
      eInfo.sSaveEquipment3ScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_TRIGGERS:
      eInfo.sSaveSelTriggersIndex = eInfo.sSelItemIndex;
      eInfo.sSaveTriggersScrollIndex = eInfo.sScrollIndex;
      break;
    case TBAR_MODE_ITEM_KEYS:
      eInfo.sSaveSelKeysIndex = eInfo.sSelItemIndex;
      eInfo.sSaveKeysScrollIndex = eInfo.sScrollIndex;
      break;
  }
}

void HandleItemsPanel(uint16_t usScreenX, uint16_t usScreenY, int8_t bEvent) {
  int16_t sIndex;
  uint16_t usQuantity;
  // Calc base index from scrolling index
  sIndex = eInfo.sScrollIndex * 2;
  // Determine if the index is in the first row or second row from mouse YPos.
  if (usScreenY >= 400) sIndex++;
  // Add the converted mouse's XPos into a relative index;
  // Calc:  starting from 110, for every 60 pixels, add 2 to the index
  sIndex += ((usScreenX - 110) / 60) * 2;
  switch (bEvent) {
    case GUI_MOVE_EVENT:
      if (sIndex < eInfo.sNumItems) {
        if (eInfo.sHilitedItemIndex != sIndex) gfRenderTaskbar = TRUE;
        // this index will now highlight in yellow.
        eInfo.sHilitedItemIndex = sIndex;
      }
      break;
    case GUI_LCLICK_EVENT:
      if (sIndex < eInfo.sNumItems) {
        // this index will now highlight in red.
        if (eInfo.sSelItemIndex != sIndex) gfRenderTaskbar = TRUE;
        eInfo.sSelItemIndex = sIndex;
        if (gfMercGetItem) {
          gfMercGetItem = FALSE;
          gusMercsNewItemIndex = eInfo.pusItemIndex[eInfo.sSelItemIndex];
          SetMercEditingMode(MERC_INVENTORYMODE);
          ClearEditorItemsInfo();
        }
      }
      break;
    case GUI_RCLICK_EVENT:
      if (gfMercGetItem) {
        gfMercGetItem = FALSE;
        gusMercsNewItemIndex = 0xffff;
        SetMercEditingMode(MERC_INVENTORYMODE);
        ClearEditorItemsInfo();
      } else if (sIndex < eInfo.sNumItems) {
        eInfo.sSelItemIndex = sIndex;
        gfRenderTaskbar = TRUE;
        if (CountNumberOfEditorPlacementsInWorld(eInfo.sSelItemIndex, &usQuantity)) {
          FindNextItemOfSelectedType();
        }
      }
      break;
  }
}

void ShowItemCursor(int32_t iMapIndex) {
  struct LEVELNODE *pNode;
  pNode = gpWorldLevelData[iMapIndex].pTopmostHead;
  while (pNode) {
    if (pNode->usIndex == SELRING) return;
    pNode = pNode->pNext;
  }
  AddTopmostToTail(iMapIndex, SELRING1);
}

void HideItemCursor(int32_t iMapIndex) { RemoveTopmost(iMapIndex, SELRING1); }

BOOLEAN TriggerAtGridNo(int16_t sGridNo) {
  struct ITEM_POOL *pItemPool;
  if (!GetItemPool(sGridNo, &pItemPool, 0)) {
    return FALSE;
  }
  while (pItemPool) {
    if (gWorldItems[pItemPool->iItemIndex].o.usItem == SWITCH) {
      return TRUE;
    }
    pItemPool = pItemPool->pNext;
  }
  return FALSE;
}

void AddSelectedItemToWorld(int16_t sGridNo) {
  struct OBJECTTYPE tempObject;
  struct OBJECTTYPE *pObject;
  INVTYPE *pItem;
  struct ITEM_POOL *pItemPool;
  int32_t iItemIndex;
  int8_t bVisibility = INVISIBLE;
  BOOLEAN fFound = FALSE;
  IPListNode *pIPCurr, *pIPPrev;
  uint16_t usFlags;

  // Extract the currently selected item.
  SpecifyItemToEdit(NULL, -1);

  // memset( &tempObject, 0, sizeof( struct OBJECTTYPE ) );
  if (eInfo.uiItemType == TBAR_MODE_ITEM_KEYS) {
    CreateKeyObject(&tempObject, 1, (uint8_t)eInfo.sSelItemIndex);
  } else {
    CreateItem(eInfo.pusItemIndex[eInfo.sSelItemIndex], 100, &tempObject);
  }
  usFlags = 0;
  switch (tempObject.usItem) {
    case MINE:
      if (bVisibility == BURIED) {
        usFlags |= WORLD_ITEM_ARMED_BOMB;
      }
      break;
    case MONEY:
    case SILVER:
    case GOLD:
      tempObject.bStatus[0] = 100;
      tempObject.uiMoneyAmount = 100 + Random(19901);
      break;
    case OWNERSHIP:
      tempObject.ubOwnerProfile = NO_PROFILE;
      bVisibility = BURIED;
      break;
    case SWITCH:
      if (TriggerAtGridNo(sGridNo)) {  // Restricted to one action per gridno.
        return;
      }
      bVisibility = BURIED;
      tempObject.bStatus[0] = 100;
      tempObject.ubBombOwner = 1;
      if (eInfo.sSelItemIndex < 2)
        tempObject.bFrequency = PANIC_FREQUENCY;
      else if (eInfo.sSelItemIndex < 4)
        tempObject.bFrequency = PANIC_FREQUENCY_2;
      else if (eInfo.sSelItemIndex < 6)
        tempObject.bFrequency = PANIC_FREQUENCY_3;
      else
        tempObject.bFrequency =
            (int8_t)(FIRST_MAP_PLACED_FREQUENCY + (eInfo.sSelItemIndex - 4) / 2);
      usFlags |= WORLD_ITEM_ARMED_BOMB;
      break;
    case ACTION_ITEM:
      bVisibility = BURIED;
      tempObject.bStatus[0] = 100;
      tempObject.ubBombOwner = 1;
      tempObject.bTrap = gbDefaultBombTrapLevel;
      if (eInfo.sSelItemIndex < PRESSURE_ACTION_ID) {
        tempObject.bDetonatorType = BOMB_REMOTE;
        if (eInfo.sSelItemIndex < 2)
          tempObject.bFrequency = PANIC_FREQUENCY;
        else if (eInfo.sSelItemIndex < 4)
          tempObject.bFrequency = PANIC_FREQUENCY_2;
        else if (eInfo.sSelItemIndex < 6)
          tempObject.bFrequency = PANIC_FREQUENCY_3;
        else
          tempObject.bFrequency =
              (int8_t)(FIRST_MAP_PLACED_FREQUENCY + (eInfo.sSelItemIndex - 4) / 2);
      } else {
        tempObject.bDetonatorType = BOMB_PRESSURE;
        tempObject.bDelay = 0;
      }
      ChangeActionItem(&tempObject, gbActionItemIndex);
      tempObject.fFlags |= OBJECT_ARMED_BOMB;
      if (gbActionItemIndex == ACTIONITEM_SMPIT)
        Add3X3Pit(sGridNo);
      else if (gbActionItemIndex == ACTIONITEM_LGPIT)
        Add5X5Pit(sGridNo);
      usFlags |= WORLD_ITEM_ARMED_BOMB;
      break;
  }

  pObject = InternalAddItemToPool(&sGridNo, &tempObject, bVisibility, 0, usFlags, 0, &iItemIndex);
  if (tempObject.usItem != OWNERSHIP) {
    gWorldItems[iItemIndex].ubNonExistChance = (uint8_t)(100 - giDefaultExistChance);
  } else {
    gWorldItems[iItemIndex].ubNonExistChance = 0;
  }

  pItem = &(Item[pObject->usItem]);
  if (pItem->usItemClass == IC_AMMO) {
    if (Random(2)) {
      pObject->ubShotsLeft[0] = Magazine[pItem->ubClassIndex].ubMagSize;
    } else {
      pObject->ubShotsLeft[0] = (uint8_t)Random(Magazine[pItem->ubClassIndex].ubMagSize);
    }
  } else {
    pObject->bStatus[0] = (int8_t)(70 + Random(26));
  }
  if (pItem->usItemClass & IC_GUN) {
    if (pObject->usItem == ROCKET_LAUNCHER) {
      pObject->ubGunShotsLeft = 1;
    } else {
      pObject->ubGunShotsLeft = (uint8_t)(Random(Weapon[pObject->usItem].ubMagSize));
    }
  }

  if (!GetItemPool(sGridNo, &pItemPool, 0)) Assert(0);
  while (pItemPool) {
    if (&(gWorldItems[pItemPool->iItemIndex].o) == pObject) {
      fFound = TRUE;
      // ShowSelectedItem();
      break;
    }
    pItemPool = pItemPool->pNext;
  }
  Assert(fFound);

  gpItemPool = pItemPool;

  SpecifyItemToEdit(pObject, sGridNo);

  // Get access to the itempool.
  // search for a current node in list containing same mapindex
  pIPCurr = pIPHead;
  pIPPrev = NULL;
  while (pIPCurr) {
    pIPPrev = pIPCurr;
    if (pIPCurr->sGridNo == sGridNo) {
      // found one, so we don't need to add it
      gpCurrItemPoolNode = pIPCurr;
      return;
    }
    pIPCurr = pIPCurr->next;
  }
  // there isn't one, so we will add it now.
  ShowItemCursor(sGridNo);
  if (pIPPrev) {
    pIPPrev->next = (IPListNode *)MemAlloc(sizeof(IPListNode));
    Assert(pIPPrev->next);
    pIPPrev = pIPPrev->next;
    pIPPrev->next = NULL;
    pIPPrev->sGridNo = sGridNo;
    gpCurrItemPoolNode = pIPPrev;
  } else {
    pIPHead = (IPListNode *)MemAlloc(sizeof(IPListNode));
    Assert(pIPHead);
    pIPHead->next = NULL;
    pIPHead->sGridNo = sGridNo;
    gpCurrItemPoolNode = pIPHead;
  }
}

void HandleRightClickOnItem(int16_t sGridNo) {
  struct ITEM_POOL *pItemPool;
  IPListNode *pIPCurr;

  if (gsItemGridNo ==
      sGridNo) {  // Clicked on the same gridno as the selected item.  Automatically select the next
    // item in the same pool.
    pItemPool = gpItemPool->pNext;
    if (!pItemPool) {  // currently selected item was last node, so select the head node even if it
                       // is the same.
      GetItemPool(sGridNo, &pItemPool, 0);
    }
  } else if (!GetItemPool(sGridNo, &pItemPool, 0)) {
    // possibly relocate selected item to this gridno?
    return;
  }

  gpItemPool = pItemPool;

  // set up the item pool pointer to point to the same mapindex node
  pIPCurr = pIPHead;
  gpCurrItemPoolNode = NULL;
  while (pIPCurr) {
    if (pIPCurr->sGridNo == sGridNo) {
      gpCurrItemPoolNode = pIPCurr;
      break;
    }
    pIPCurr = pIPCurr->next;
  }
  Assert(gpCurrItemPoolNode);
  SpecifyItemToEdit(&gWorldItems[gpItemPool->iItemIndex].o, gpItemPool->sGridNo);
}

extern void DeleteSelectedMercsItem();

void DeleteSelectedItem() {
  SpecifyItemToEdit(NULL, -1);
  // First, check to see if there even is a currently selected item.
  if (iCurrentTaskbar == TASK_MERCS) {
    DeleteSelectedMercsItem();
    return;
  }
  if (gpItemPool) {  // Okay, we have a selected item...
    int16_t sGridNo;
    // save the mapindex
    if (gpItemPool->pNext) {
      SpecifyItemToEdit(&gWorldItems[gpItemPool->pNext->iItemIndex].o, gpItemPool->sGridNo);
    }
    sGridNo = gpItemPool->sGridNo;
    // remove the item
    if (gWorldItems[gpItemPool->iItemIndex].o.usItem == ACTION_ITEM) {
      if (gWorldItems[gpItemPool->iItemIndex].o.bActionValue == ACTION_ITEM_SMALL_PIT)
        Remove3X3Pit(gWorldItems[gpItemPool->iItemIndex].sGridNo);
      else if (gWorldItems[gpItemPool->iItemIndex].o.bActionValue == ACTION_ITEM_LARGE_PIT)
        Remove5X5Pit(gWorldItems[gpItemPool->iItemIndex].sGridNo);
    }
    if (gpEditingItemPool == gpItemPool) gpEditingItemPool = NULL;
    RemoveItemFromPool(sGridNo, gpItemPool->iItemIndex, 0);
    gpItemPool = NULL;
    // determine if there are still any items at this location
    if (!GetItemPool(sGridNo, &gpItemPool, 0)) {  // no items left, so remove the node from the
                                                  // list.
      IPListNode *pIPPrev, *pIPCurr;
      pIPCurr = pIPHead;
      pIPPrev = NULL;
      while (pIPCurr) {
        if (pIPCurr->sGridNo == sGridNo) {
          if (pIPPrev)  // middle of list
            pIPPrev->next = pIPCurr->next;
          else  // head of list
            pIPHead = pIPHead->next;
          // move the curr item pool to the next one.
          if (pIPCurr->next)
            gpCurrItemPoolNode = pIPCurr->next;
          else
            gpCurrItemPoolNode = pIPHead;
          if (gpCurrItemPoolNode) {
            GetItemPool(gpCurrItemPoolNode->sGridNo, &gpItemPool, 0);
            Assert(gpItemPool);
          }
          // remove node
          HideItemCursor(sGridNo);
          MemFree(pIPCurr);
          pIPCurr = NULL;
          return;
        }
        pIPPrev = pIPCurr;
        pIPCurr = pIPCurr->next;
      }
    }
  }
}

void ShowSelectedItem() {
  if (gpItemPool) {
    gpItemPool->bVisible = INVISIBLE;
    gWorldItems[gpItemPool->iItemIndex].bVisible = INVISIBLE;
  }
}

void HideSelectedItem() {
  if (gpItemPool) {
    gpItemPool->bVisible = HIDDEN_ITEM;
    gWorldItems[gpItemPool->iItemIndex].bVisible = HIDDEN_ITEM;
  }
}

void SelectNextItemPool() {
  if (!gpCurrItemPoolNode) return;
  // remove the current hilight.
  if (gpItemPool) {
    MarkMapIndexDirty(gpItemPool->sGridNo);
  }

  // go to the next node.  If at end of list, choose pIPHead
  if (gpCurrItemPoolNode->next)
    gpCurrItemPoolNode = gpCurrItemPoolNode->next;
  else
    gpCurrItemPoolNode = pIPHead;
  // get the item pool at this node's gridno.
  GetItemPool(gpCurrItemPoolNode->sGridNo, &gpItemPool, 0);
  MarkMapIndexDirty(gpItemPool->sGridNo);
  SpecifyItemToEdit(&gWorldItems[gpItemPool->iItemIndex].o, gpItemPool->sGridNo);
  if (gsItemGridNo != -1) {
    CenterScreenAtMapIndex(gsItemGridNo);
  }
}

void SelectNextItemInPool() {
  if (gpItemPool) {
    if (gpItemPool->pNext) {
      gpItemPool = gpItemPool->pNext;
    } else {
      GetItemPool(gpItemPool->sGridNo, &gpItemPool, 0);
    }
    SpecifyItemToEdit(&gWorldItems[gpItemPool->iItemIndex].o, gpItemPool->sGridNo);
    MarkWorldDirty();
  }
}

void SelectPrevItemInPool() {
  if (gpItemPool) {
    if (gpItemPool->pPrev) {
      gpItemPool = gpItemPool->pPrev;
    } else {
      GetItemPool(gpItemPool->sGridNo, &gpItemPool, 0);
      while (gpItemPool->pNext) {
        gpItemPool = gpItemPool->pNext;
      }
    }
    SpecifyItemToEdit(&gWorldItems[gpItemPool->iItemIndex].o, gpItemPool->sGridNo);
    MarkWorldDirty();
  }
}

void FindNextItemOfSelectedType() {
  uint16_t usItem;
  usItem = eInfo.pusItemIndex[eInfo.sSelItemIndex];
  if (usItem == ACTION_ITEM || usItem == SWITCH) {
    if (eInfo.sSelItemIndex < PRESSURE_ACTION_ID) {
      int8_t bFrequency;
      if (eInfo.sSelItemIndex < 2)
        bFrequency = PANIC_FREQUENCY;
      else if (eInfo.sSelItemIndex < 4)
        bFrequency = PANIC_FREQUENCY_2;
      else if (eInfo.sSelItemIndex < 6)
        bFrequency = PANIC_FREQUENCY_3;
      else
        bFrequency = (int8_t)(FIRST_MAP_PLACED_FREQUENCY + (eInfo.sSelItemIndex - 4) / 2);
      SelectNextTriggerWithFrequency(usItem, bFrequency);
    } else {
      SelectNextPressureAction();
    }
  } else if (Item[usItem].usItemClass == IC_KEY) {
    SelectNextKeyOfType((uint8_t)eInfo.sSelItemIndex);
  } else {
    SelectNextItemOfType(usItem);
  }
}

void SelectNextItemOfType(uint16_t usItem) {
  IPListNode *curr;
  struct OBJECTTYPE *pObject;
  if (gpItemPool) {
    curr = pIPHead;
    while (curr) {  // skip quickly to the same gridno as the item pool
      if (curr->sGridNo == gWorldItems[gpItemPool->iItemIndex].sGridNo) {
        gpItemPool = gpItemPool->pNext;
        while (gpItemPool) {
          pObject = &gWorldItems[gpItemPool->iItemIndex].o;
          if (pObject->usItem == usItem) {
            SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
            CenterScreenAtMapIndex(gsItemGridNo);
            return;  // success! (another item in same itempool)
          }
          gpItemPool = gpItemPool->pNext;
        }
        curr = curr->next;
        break;
      }
      curr = curr->next;
    }
    while (curr) {  // search to the end of the list
      GetItemPool(curr->sGridNo, &gpItemPool, 0);
      while (gpItemPool) {
        pObject = &gWorldItems[gpItemPool->iItemIndex].o;
        if (pObject->usItem == usItem) {
          SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
          CenterScreenAtMapIndex(gsItemGridNo);
          return;  // success! (found another item before reaching the end of the list)
        }
        gpItemPool = gpItemPool->pNext;
      }
      curr = curr->next;
    }
  }
  curr = pIPHead;
  while (curr) {  // search to the end of the list
    GetItemPool(curr->sGridNo, &gpItemPool, 0);
    while (gpItemPool) {
      pObject = &gWorldItems[gpItemPool->iItemIndex].o;
      if (pObject->usItem == usItem) {
        SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
        CenterScreenAtMapIndex(gsItemGridNo);
        return;  // success! (found first item in the list)
      }
      gpItemPool = gpItemPool->pNext;
    }
    curr = curr->next;
  }
}

void SelectNextKeyOfType(uint8_t ubKeyID) {
  IPListNode *curr;
  struct OBJECTTYPE *pObject;
  if (gpItemPool) {
    curr = pIPHead;
    while (curr) {  // skip quickly to the same gridno as the item pool
      if (curr->sGridNo == gWorldItems[gpItemPool->iItemIndex].sGridNo) {
        gpItemPool = gpItemPool->pNext;
        while (gpItemPool) {
          pObject = &gWorldItems[gpItemPool->iItemIndex].o;
          if (Item[pObject->usItem].usItemClass == IC_KEY && pObject->ubKeyID == ubKeyID) {
            SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
            CenterScreenAtMapIndex(gsItemGridNo);
            return;  // success! (another item in same itempool)
          }
          gpItemPool = gpItemPool->pNext;
        }
        curr = curr->next;
        break;
      }
      curr = curr->next;
    }
    while (curr) {  // search to the end of the list
      GetItemPool(curr->sGridNo, &gpItemPool, 0);
      while (gpItemPool) {
        pObject = &gWorldItems[gpItemPool->iItemIndex].o;
        if (Item[pObject->usItem].usItemClass == IC_KEY && pObject->ubKeyID == ubKeyID) {
          SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
          CenterScreenAtMapIndex(gsItemGridNo);
          return;  // success! (found another item before reaching the end of the list)
        }
        gpItemPool = gpItemPool->pNext;
      }
      curr = curr->next;
    }
  }
  curr = pIPHead;
  while (curr) {  // search to the end of the list
    GetItemPool(curr->sGridNo, &gpItemPool, 0);
    while (gpItemPool) {
      pObject = &gWorldItems[gpItemPool->iItemIndex].o;
      if (Item[pObject->usItem].usItemClass == IC_KEY && pObject->ubKeyID == ubKeyID) {
        SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
        CenterScreenAtMapIndex(gsItemGridNo);
        return;  // success! (found first item in the list)
      }
      gpItemPool = gpItemPool->pNext;
    }
    curr = curr->next;
  }
}

void SelectNextTriggerWithFrequency(uint16_t usItem, int8_t bFrequency) {
  IPListNode *curr;
  struct OBJECTTYPE *pObject;
  if (gpItemPool) {
    curr = pIPHead;
    while (curr) {  // skip quickly to the same gridno as the item pool
      if (curr->sGridNo == gWorldItems[gpItemPool->iItemIndex].sGridNo) {
        gpItemPool = gpItemPool->pNext;
        while (gpItemPool) {
          pObject = &gWorldItems[gpItemPool->iItemIndex].o;
          if (pObject->usItem == usItem && pObject->bFrequency == bFrequency) {
            SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
            CenterScreenAtMapIndex(gsItemGridNo);
            return;  // success! (another item in same itempool)
          }
          gpItemPool = gpItemPool->pNext;
        }
        curr = curr->next;
        break;
      }
      curr = curr->next;
    }
    while (curr) {  // search to the end of the list
      GetItemPool(curr->sGridNo, &gpItemPool, 0);
      while (gpItemPool) {
        pObject = &gWorldItems[gpItemPool->iItemIndex].o;
        if (pObject->usItem == usItem && pObject->bFrequency == bFrequency) {
          SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
          CenterScreenAtMapIndex(gsItemGridNo);
          return;  // success! (found another item before reaching the end of the list)
        }
        gpItemPool = gpItemPool->pNext;
      }
      curr = curr->next;
    }
  }
  curr = pIPHead;
  while (curr) {  // search to the end of the list
    GetItemPool(curr->sGridNo, &gpItemPool, 0);
    while (gpItemPool) {
      pObject = &gWorldItems[gpItemPool->iItemIndex].o;
      if (pObject->usItem == usItem && pObject->bFrequency == bFrequency) {
        SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
        CenterScreenAtMapIndex(gsItemGridNo);
        return;  // success! (found first item in the list)
      }
      gpItemPool = gpItemPool->pNext;
    }
    curr = curr->next;
  }
}

void SelectNextPressureAction() {
  IPListNode *curr;
  struct OBJECTTYPE *pObject;
  if (gpItemPool) {
    curr = pIPHead;
    while (curr) {  // skip quickly to the same gridno as the item pool
      if (curr->sGridNo == gWorldItems[gpItemPool->iItemIndex].sGridNo) {
        gpItemPool = gpItemPool->pNext;
        while (gpItemPool) {
          pObject = &gWorldItems[gpItemPool->iItemIndex].o;
          if (pObject->usItem == ACTION_ITEM && pObject->bDetonatorType == BOMB_PRESSURE) {
            SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
            CenterScreenAtMapIndex(gsItemGridNo);
            return;  // success! (another item in same itempool)
          }
          gpItemPool = gpItemPool->pNext;
        }
        curr = curr->next;
        break;
      }
      curr = curr->next;
    }
    while (curr) {  // search to the end of the list
      GetItemPool(curr->sGridNo, &gpItemPool, 0);
      while (gpItemPool) {
        pObject = &gWorldItems[gpItemPool->iItemIndex].o;
        if (pObject->usItem == ACTION_ITEM && pObject->bDetonatorType == BOMB_PRESSURE) {
          SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
          CenterScreenAtMapIndex(gsItemGridNo);
          return;  // success! (found another item before reaching the end of the list)
        }
        gpItemPool = gpItemPool->pNext;
      }
      curr = curr->next;
    }
  }
  curr = pIPHead;
  while (curr) {  // search to the end of the list
    GetItemPool(curr->sGridNo, &gpItemPool, 0);
    while (gpItemPool) {
      pObject = &gWorldItems[gpItemPool->iItemIndex].o;
      if (pObject->usItem == ACTION_ITEM && pObject->bDetonatorType == BOMB_PRESSURE) {
        SpecifyItemToEdit(pObject, gWorldItems[gpItemPool->iItemIndex].sGridNo);
        CenterScreenAtMapIndex(gsItemGridNo);
        return;  // success! (found first item in the list)
      }
      gpItemPool = gpItemPool->pNext;
    }
    curr = curr->next;
  }
}

uint16_t CountNumberOfItemPlacementsInWorld(uint16_t usItem, uint16_t *pusQuantity) {
  struct ITEM_POOL *pItemPool;
  IPListNode *pIPCurr;
  int16_t num = 0;
  *pusQuantity = 0;
  pIPCurr = pIPHead;
  while (pIPCurr) {
    GetItemPool(pIPCurr->sGridNo, &pItemPool, 0);
    while (pItemPool) {
      if (gWorldItems[pItemPool->iItemIndex].o.usItem == usItem) {
        num++;
        *pusQuantity += gWorldItems[pItemPool->iItemIndex].o.ubNumberOfObjects;
      }
      pItemPool = pItemPool->pNext;
    }
    pIPCurr = pIPCurr->next;
  }
  return num;
}

uint16_t CountNumberOfItemsWithFrequency(uint16_t usItem, int8_t bFrequency) {
  struct ITEM_POOL *pItemPool;
  IPListNode *pIPCurr;
  uint16_t num = 0;
  pIPCurr = pIPHead;
  while (pIPCurr) {
    GetItemPool(pIPCurr->sGridNo, &pItemPool, 0);
    while (pItemPool) {
      if (gWorldItems[pItemPool->iItemIndex].o.usItem == usItem &&
          gWorldItems[pItemPool->iItemIndex].o.bFrequency == bFrequency) {
        num++;
      }
      pItemPool = pItemPool->pNext;
    }
    pIPCurr = pIPCurr->next;
  }
  return num;
}

uint16_t CountNumberOfPressureActionsInWorld() {
  struct ITEM_POOL *pItemPool;
  IPListNode *pIPCurr;
  uint16_t num = 0;
  pIPCurr = pIPHead;
  while (pIPCurr) {
    GetItemPool(pIPCurr->sGridNo, &pItemPool, 0);
    while (pItemPool) {
      if (gWorldItems[pItemPool->iItemIndex].o.usItem == ACTION_ITEM &&
          gWorldItems[pItemPool->iItemIndex].o.bDetonatorType == BOMB_PRESSURE) {
        num++;
      }
      pItemPool = pItemPool->pNext;
    }
    pIPCurr = pIPCurr->next;
  }
  return num;
}

uint16_t CountNumberOfEditorPlacementsInWorld(uint16_t usEInfoIndex, uint16_t *pusQuantity) {
  uint16_t usNumPlacements;
  if (eInfo.uiItemType == TBAR_MODE_ITEM_TRIGGERS) {  // find identical items with same frequency
    int8_t bFrequency;
    if (usEInfoIndex < PRESSURE_ACTION_ID) {
      if (usEInfoIndex < 2)
        bFrequency = PANIC_FREQUENCY;
      else if (usEInfoIndex < 4)
        bFrequency = PANIC_FREQUENCY_2;
      else if (usEInfoIndex < 6)
        bFrequency = PANIC_FREQUENCY_3;
      else
        bFrequency = (int8_t)(FIRST_MAP_PLACED_FREQUENCY + (usEInfoIndex - 4) / 2);
      usNumPlacements =
          CountNumberOfItemsWithFrequency(eInfo.pusItemIndex[usEInfoIndex], bFrequency);
      *pusQuantity = usNumPlacements;
    } else {
      usNumPlacements = CountNumberOfPressureActionsInWorld();
      *pusQuantity = usNumPlacements;
    }
  } else if (eInfo.uiItemType == TBAR_MODE_ITEM_KEYS) {
    usNumPlacements = CountNumberOfKeysOfTypeInWorld((uint8_t)usEInfoIndex);
    *pusQuantity = usNumPlacements;
  } else {
    usNumPlacements =
        CountNumberOfItemPlacementsInWorld(eInfo.pusItemIndex[usEInfoIndex], pusQuantity);
  }
  return usNumPlacements;
}

uint16_t CountNumberOfKeysOfTypeInWorld(uint8_t ubKeyID) {
  struct ITEM_POOL *pItemPool;
  IPListNode *pIPCurr;
  int16_t num = 0;
  pIPCurr = pIPHead;
  while (pIPCurr) {
    GetItemPool(pIPCurr->sGridNo, &pItemPool, 0);
    while (pItemPool) {
      if (Item[gWorldItems[pItemPool->iItemIndex].o.usItem].usItemClass == IC_KEY) {
        if (gWorldItems[pItemPool->iItemIndex].o.ubKeyID == ubKeyID) {
          num++;
        }
      }
      pItemPool = pItemPool->pNext;
    }
    pIPCurr = pIPCurr->next;
  }
  return num;
}

void DisplayItemStatistics() {
  BOOLEAN fUseSelectedItem;
  int16_t usItemIndex;
  wchar_t pItemName[SIZE_ITEM_NAME];

  if (!eInfo.fActive) {
    return;
  }

  // If there is nothing else currently highlited by the mouse, use the selected item.
  fUseSelectedItem =
      eInfo.sHilitedItemIndex == -1 || eInfo.sHilitedItemIndex == eInfo.sSelItemIndex;

  SetFont(SMALLCOMPFONT);
  SetFontForeground((uint8_t)(fUseSelectedItem ? FONT_LTRED : FONT_YELLOW));

  // Extract all of the item information.
  if (!eInfo.pusItemIndex) return;
  usItemIndex =
      eInfo.pusItemIndex[fUseSelectedItem ? eInfo.sSelItemIndex : eInfo.sHilitedItemIndex];
  LoadItemInfo(usItemIndex, pItemName, NULL);

  mprintf(50 - StringPixLength(pItemName, SMALLCOMPFONT) / 2, 403, pItemName);
  mprintf(2, 410, L"Status Info Line 1");
  mprintf(2, 420, L"Status Info Line 2");
  mprintf(2, 430, L"Status Info Line 3");
  mprintf(2, 440, L"Status Info Line 4");
  mprintf(2, 450, L"Status Info Line 5");
}
