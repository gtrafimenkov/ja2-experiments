// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/MapScreenInterfaceMapInventory.h"

#include <stdlib.h>

#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/ArmsDealerInvInit.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/Items.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/ShopKeeperInterface.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/SysUtil.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

extern BOOLEAN SaveWorldItemsToTempItemFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ,
                                            uint32_t uiNumberOfItems, WORLDITEM *pData);

// status bar colors
#define DESC_STATUS_BAR FROMRGB(201, 172, 133)
#define DESC_STATUS_BAR_SHADOW FROMRGB(140, 136, 119)

// page display positions
#define MAP_INVENTORY_POOL_PAGE_X 506
#define MAP_INVENTORY_POOL_PAGE_Y 336
#define MAP_INVENTORY_POOL_PAGE_WIDTH 46  // 552 - 494
#define MAP_INVENTORY_POOL_PAGE_HEIGHT 345 - 332

// the number of items
#define MAP_INVENTORY_POOL_NUMBER_X 436
#define MAP_INVENTORY_POOL_NUMBER_WIDTH 474 - 434

// location
#define MAP_INVENTORY_POOL_LOC_X 326
#define MAP_INVENTORY_POOL_LOC_WIDTH 366 - 326

// delay for flash of item
#define DELAY_FOR_HIGHLIGHT_ITEM_FLASH 200

// inventory slot font
#define MAP_IVEN_FONT SMALLCOMPFONT

// the position of the background graphic
#define INVEN_POOL_X 261
#define INVEN_POOL_Y 0

// inventory Graphic Offset X and y
#define MAP_INVENTORY_POOL_SLOT_OFFSET_X 2
#define MAP_INVENTORY_POOL_SLOT_OFFSET_Y 5

// height of map inventory pool bar
#define ITEMDESC_ITEM_STATUS_HEIGHT_INV_POOL 20
#define ITEMDESC_ITEM_STATUS_WIDTH_INV_POOL 2

// map bar offsets
#define ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_X 5
#define ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_Y 22

// inventory pool slot positions and sizes
#define MAP_INVENTORY_POOL_SLOT_START_X 271
#define MAP_INVENTORY_POOL_SLOT_START_Y 36
#define MAP_INV_SLOT_COLS 9
#define MAP_INVEN_SLOT_WIDTH 65
#define MAP_INVEN_SPACE_BTWN_SLOTS 72
#define MAP_INVEN_SLOT_HEIGHT 32
#define MAP_INVEN_SLOT_IMAGE_HEIGHT 24

// the current highlighted item
int32_t iCurrentlyHighLightedItem = -1;
BOOLEAN fFlashHighLightInventoryItemOnradarMap = FALSE;

// whether we are showing the inventory pool graphic
BOOLEAN fShowMapInventoryPool = FALSE;

// the v-object index value for the background
uint32_t guiMapInventoryPoolBackground;

// inventory pool list
WORLDITEM *pInventoryPoolList = NULL;

// current page of inventory
int32_t iCurrentInventoryPoolPage = 0;
int32_t iLastInventoryPoolPage = 0;

// total number of slots allocated
int32_t iTotalNumberOfSlots = 0;

int16_t sObjectSourceGridNo = 0;

// number of unseen items in sector
uint32_t uiNumberOfUnSeenItems = 0;

// the inventory slots
struct MOUSE_REGION MapInventoryPoolSlots[MAP_INVENTORY_POOL_SLOT_COUNT];
struct MOUSE_REGION MapInventoryPoolMask;
BOOLEAN fMapInventoryItemCompatable[MAP_INVENTORY_POOL_SLOT_COUNT];
BOOLEAN fChangedInventorySlots = FALSE;

// the unseen items list...have to save this
WORLDITEM *pUnSeenItems = NULL;

// save list to write to temp file
WORLDITEM *pSaveList = NULL;

int32_t giFlashHighlightedItemBaseTime = 0;
int32_t giCompatibleItemBaseTime = 0;

// the buttons and images
uint32_t guiMapInvenButtonImage[3];
uint32_t guiMapInvenButton[3];

BOOLEAN gfCheckForCursorOverMapSectorInventoryItem = FALSE;

extern uint32_t guiNumWorldItems;
extern BOOLEAN fShowInventoryFlag;
extern BOOLEAN fMapScreenBottomDirty;

// outside vidieo objects for cursor
extern uint32_t guiExternVo;
extern uint16_t gusExternVoSubIndex;

extern struct MOUSE_REGION gMPanelRegion;

// map inventory callback
void MapInvenPoolSlots(struct MOUSE_REGION *pRegion, int32_t iReason);
void MapInvenPoolSlotsMove(struct MOUSE_REGION *pRegion, int32_t iReason);
void CreateMapInventoryPoolSlots(void);
void DestroyMapInventoryPoolSlots(void);
void CreateMapInventoryButtons(void);
void DestroyMapInventoryButtons(void);
void ReSizeStashListByThisAmount(int32_t iNumberOfItems);
void DestroyStash(void);
void BuildStashForSelectedSector(uint8_t sMapX, uint8_t sMapY, int8_t sMapZ);
BOOLEAN GetObjFromInventoryStashSlot(struct OBJECTTYPE *pInventorySlot,
                                     struct OBJECTTYPE *pItemPtr);
BOOLEAN RemoveObjectFromStashSlot(struct OBJECTTYPE *pInventorySlot, struct OBJECTTYPE *pItemPtr);
void BeginInventoryPoolPtr(struct OBJECTTYPE *pInventorySlot);
BOOLEAN PlaceObjectInInventoryStash(struct OBJECTTYPE *pInventorySlot, struct OBJECTTYPE *pItemPtr);
void RenderItemsForCurrentPageOfInventoryPool(void);
BOOLEAN RenderItemInPoolSlot(int32_t iCurrentSlot, int32_t iFirstSlotOnPage);
void UpdateHelpTextForInvnentoryStashSlots(void);
void MapInventoryPoolPrevBtn(GUI_BUTTON *btn, int32_t reason);
void MapInventoryPoolNextBtn(GUI_BUTTON *btn, int32_t reason);
void DisplayPagesForMapInventoryPool(void);
void DrawNumberOfIventoryPoolItems(void);
void CreateMapInventoryPoolDoneButton(void);
void DestroyInventoryPoolDoneButton(void);
void MapInventoryPoolDoneBtn(GUI_BUTTON *btn, int32_t reason);
void DisplayCurrentSector(void);
void CheckAndUnDateSlotAllocation(void);
void ClearUpTempUnSeenList(void);
void SaveSeenAndUnseenItems(void);
void DrawTextOnMapInventoryBackground(void);
void DrawTextOnSectorInventory(void);
int32_t GetTotalNumberOfItemsInSectorStash(void);
void HandleMapSectorInventory(void);
void ResetMapSectorInventoryPoolHighLights(void);
void ReBuildWorldItemStashForLoadedSector(int32_t iNumberSeenItems, int32_t iNumberUnSeenItems,
                                          WORLDITEM *pSeenItemsList, WORLDITEM *pUnSeenItemsList);
BOOLEAN IsMapScreenWorldItemVisibleInMapInventory(WORLDITEM *pWorldItem);
BOOLEAN IsMapScreenWorldItemInvisibleInMapInventory(WORLDITEM *pWorldItem);
void CheckGridNoOfItemsInMapScreenMapInventory();
int32_t MapScreenSectorInventoryCompare(const void *pNum1, const void *pNum2);
void SortSectorInventory(WORLDITEM *pInventory, uint32_t uiSizeOfArray);
BOOLEAN CanPlayerUseSectorInventory(struct SOLDIERTYPE *pSelectedSoldier);

extern void StackObjs(struct OBJECTTYPE *pSourceObj, struct OBJECTTYPE *pTargetObj,
                      uint8_t ubNumberToCopy);
extern void MAPEndItemPointer();
extern BOOLEAN GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(uint8_t *psSectorX,
                                                                      uint8_t *psSectorY,
                                                                      int8_t *psSectorZ);

// load the background panel graphics for inventory
BOOLEAN LoadInventoryPoolGraphic(void) {
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\sector_inventory.sti"),
                    &guiMapInventoryPoolBackground));

  return (TRUE);
}

// remove background panel graphics for inventory
void RemoveInventoryPoolGraphic(void) {
  // remove from v-object index
  if (guiMapInventoryPoolBackground) {
    DeleteVideoObjectFromIndex(guiMapInventoryPoolBackground);
    guiMapInventoryPoolBackground = 0;
  }

  return;
}

// blit the background panel for the inventory
void BlitInventoryPoolGraphic(void) {
  struct VObject *hHandle;

  // blit inventory pool graphic to the screen
  GetVideoObject(&hHandle, guiMapInventoryPoolBackground);
  BltVideoObject(vsSaveBuffer, hHandle, 0, INVEN_POOL_X, INVEN_POOL_Y);

  // resize list
  CheckAndUnDateSlotAllocation();

  // now the items
  RenderItemsForCurrentPageOfInventoryPool();

  // now update help text
  UpdateHelpTextForInvnentoryStashSlots();

  // show which page and last page
  DisplayPagesForMapInventoryPool();

  // draw number of items in current inventory
  DrawNumberOfIventoryPoolItems();

  // display current sector inventory pool is at
  DisplayCurrentSector();

  DrawTextOnMapInventoryBackground();

  // re render buttons
  MarkButtonsDirty();

  // which buttons will be active and which ones not
  HandleButtonStatesWhileMapInventoryActive();

  return;
}

void RenderItemsForCurrentPageOfInventoryPool(void) {
  int32_t iCounter = 0;

  // go through list of items on this page and place graphics to screen
  for (iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++) {
    RenderItemInPoolSlot(iCounter, (iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT));
  }

  return;
}

BOOLEAN RenderItemInPoolSlot(int32_t iCurrentSlot, int32_t iFirstSlotOnPage) {
  // render item in this slot of the list
  int16_t sX, sY;
  struct VObject *hHandle;
  wchar_t sString[64];
  int16_t sWidth = 0, sHeight = 0;
  int16_t sOutLine = 0;
  BOOLEAN fOutLine = FALSE;

  // check if anything there
  if (pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage].o.ubNumberOfObjects == 0) {
    return (FALSE);
  }

  GetVideoObject(&hHandle,
                 GetInterfaceGraphicForItem(
                     &(Item[pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage].o.usItem])));

  // set sx and sy
  sX = (int16_t)(MAP_INVENTORY_POOL_SLOT_OFFSET_X + MAP_INVENTORY_POOL_SLOT_START_X +
                 ((MAP_INVEN_SPACE_BTWN_SLOTS) * (iCurrentSlot / MAP_INV_SLOT_COLS)));
  sY = (int16_t)(MAP_INVENTORY_POOL_SLOT_START_Y +
                 ((MAP_INVEN_SLOT_HEIGHT) * (iCurrentSlot % (MAP_INV_SLOT_COLS))));

  if (fMapInventoryItemCompatable[iCurrentSlot]) {
    sOutLine = rgb32_to_rgb16(FROMRGB(255, 255, 255));
    fOutLine = TRUE;
  } else {
    sOutLine = us16BPPItemCyclePlacedItemColors[0];
    fOutLine = FALSE;
  }

  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  INVRenderItem(vsSaveBuffer, NULL, &(pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage].o),
                (int16_t)(sX + 7), sY, 60, 25, DIRTYLEVEL2, NULL, 0, fOutLine, sOutLine);  // 67

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  // now draw bar for condition
  // Display ststus
  DrawItemUIBarEx(
      &(pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage].o), 0,
      (int16_t)(ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_X + MAP_INVENTORY_POOL_SLOT_START_X +
                ((MAP_INVEN_SPACE_BTWN_SLOTS) * (iCurrentSlot / MAP_INV_SLOT_COLS))),
      (int16_t)(ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_Y + MAP_INVENTORY_POOL_SLOT_START_Y +
                ((MAP_INVEN_SLOT_HEIGHT) * (iCurrentSlot % (MAP_INV_SLOT_COLS)))),
      ITEMDESC_ITEM_STATUS_WIDTH_INV_POOL, ITEMDESC_ITEM_STATUS_HEIGHT_INV_POOL,
      rgb32_to_rgb16(DESC_STATUS_BAR), rgb32_to_rgb16(DESC_STATUS_BAR_SHADOW), vsSaveBuffer);

  //
  // if the item is not reachable, or if the selected merc is not in the current sector
  //
  if (!(pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage].usFlags & WORLD_ITEM_REACHABLE) ||
      !((Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorX == sSelMapX) &&
        (Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorY == sSelMapY) &&
        (Menptr[gCharactersList[bSelectedInfoChar].usSolID].bSectorZ == iCurrentMapSectorZ))) {
    // Shade the item
    DrawHatchOnInventory(vsSaveBuffer, sX, sY, MAP_INVEN_SLOT_WIDTH, MAP_INVEN_SLOT_IMAGE_HEIGHT);
  }

  // the name

  wcscpy(sString, ShortItemNames[pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage].o.usItem]);

  if (StringPixLength(sString, MAP_IVEN_FONT) >= (MAP_INVEN_SLOT_WIDTH)) {
    ReduceStringLength(sString, ARR_SIZE(sString),
                       (int16_t)(MAP_INVEN_SLOT_WIDTH - StringPixLength(L" ...", MAP_IVEN_FONT)),
                       MAP_IVEN_FONT);
  }

  FindFontCenterCoordinates(
      (int16_t)(4 + MAP_INVENTORY_POOL_SLOT_START_X +
                ((MAP_INVEN_SPACE_BTWN_SLOTS) * (iCurrentSlot / MAP_INV_SLOT_COLS))),
      0, MAP_INVEN_SLOT_WIDTH, 0, sString, MAP_IVEN_FONT, &sWidth, &sHeight);

  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  SetFont(MAP_IVEN_FONT);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  mprintf(sWidth,
          (int16_t)(3 + ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_Y + MAP_INVENTORY_POOL_SLOT_START_Y +
                    ((MAP_INVEN_SLOT_HEIGHT) * (iCurrentSlot % (MAP_INV_SLOT_COLS)))),
          sString);

  /*
          if( pInventoryPoolList[ iCurrentSlot + iFirstSlotOnPage ].o.ubNumberOfObjects > 1 )
          {
                  swprintf( sString, L"x%d",  pInventoryPoolList[ iCurrentSlot + iFirstSlotOnPage
     ].o.ubNumberOfObjects );

                  // find font right coord
                  FindFontRightCoordinates( ( int16_t )( ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_X +
     MAP_INVENTORY_POOL_SLOT_START_X - 1 + ( ( MAP_INVEN_SPACE_BTWN_SLOTS ) * ( iCurrentSlot /
     MAP_INV_SLOT_COLS ) ) ),0, MAP_INVEN_SPACE_BTWN_SLOTS - 10, 0, sString, MAP_IVEN_FONT, &sX, &sY
     );

                  sY = ( int16_t )( 3 + ITEMDESC_ITEM_STATUS_INV_POOL_OFFSET_Y +
     MAP_INVENTORY_POOL_SLOT_START_Y + ( ( MAP_INVEN_SLOT_HEIGHT ) * ( iCurrentSlot % (
     MAP_INV_SLOT_COLS ) ) ) ) - 7;

                  // print string
                  mprintf( sX, sY, sString );
          }
  */

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  return (TRUE);
}

void UpdateHelpTextForInvnentoryStashSlots(void) {
  wchar_t pStr[512];
  int32_t iCounter = 0;
  int32_t iFirstSlotOnPage = (iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT);

  // run through list of items in slots and update help text for mouse regions
  for (iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++) {
    if (pInventoryPoolList[iCounter + iFirstSlotOnPage].o.ubNumberOfObjects > 0) {
      GetHelpTextForItem(pStr, ARR_SIZE(pStr), &(pInventoryPoolList[iCounter + iFirstSlotOnPage].o),
                         NULL);
      SetRegionFastHelpText(&(MapInventoryPoolSlots[iCounter]), pStr);

      /*
      // set text for current item
      if( pInventoryPoolList[ iCounter + iFirstSlotOnPage ].o.usItem == MONEY )
      {
              swprintf( pStr, L"$%ld", pInventoryPoolList[ iCounter + iFirstSlotOnPage
      ].o.uiMoneyAmount ); SetRegionFastHelpText( &(MapInventoryPoolSlots[ iCounter ]), pStr );
      }
      else
      {
              SetRegionFastHelpText( &(MapInventoryPoolSlots[ iCounter ]), ItemNames[
      pInventoryPoolList[ iCounter + iFirstSlotOnPage ].o.usItem ] );
      }
      */
    } else {
      // OK, for each item, set dirty text if applicable!
      SetRegionFastHelpText(&(MapInventoryPoolSlots[iCounter]), L"");
    }
  }

  return;
}

// create and remove buttons for inventory
void CreateDestroyMapInventoryPoolButtons(BOOLEAN fExitFromMapScreen) {
  static BOOLEAN fCreated = FALSE;

  /* player can leave items underground, no?
          if( iCurrentMapSectorZ )
          {
                  fShowMapInventoryPool = FALSE;
          }
  */

  if ((fShowMapInventoryPool) && (fCreated == FALSE)) {
    if ((gWorldSectorX == sSelMapX) && (gWorldSectorY == sSelMapY) &&
        (gbWorldSectorZ == iCurrentMapSectorZ)) {
      // handle all reachable before save
      HandleAllReachAbleItemsInTheSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                         gbWorldSectorZ);
    }

    // destroy buttons for map border
    DeleteMapBorderButtons();

    fCreated = TRUE;

    // also create the inventory slot
    CreateMapInventoryPoolSlots();

    // create buttons
    CreateMapInventoryButtons();

    // build stash
    BuildStashForSelectedSector(sSelMapX, sSelMapY, (int8_t)(iCurrentMapSectorZ));

    CreateMapInventoryPoolDoneButton();

    MarkForRedrawalStrategicMap();
    fMapScreenBottomDirty = TRUE;
  } else if ((fShowMapInventoryPool == FALSE) && (fCreated == TRUE)) {
    // check fi we are in fact leaving mapscreen
    if (fExitFromMapScreen == FALSE) {
      // recreate mapborder buttons
      CreateButtonsForMapBorder();
    }
    fCreated = FALSE;

    // destroy the map inventory slots
    DestroyMapInventoryPoolSlots();

    // destroy map inventory buttons
    DestroyMapInventoryButtons();

    DestroyInventoryPoolDoneButton();

    // clear up unseen list
    ClearUpTempUnSeenList();

    // now save results
    SaveSeenAndUnseenItems();

    DestroyStash();

    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;

    // DEF: added to remove the 'item blip' from staying on the radar map
    iCurrentlyHighLightedItem = -1;

    // re render radar map
    RenderRadarScreen();
  }

  // do our handling here
  HandleMapSectorInventory();
}

void CancelSectorInventoryDisplayIfOn(BOOLEAN fExitFromMapScreen) {
  if (fShowMapInventoryPool) {
    // get rid of sector inventory mode & buttons
    fShowMapInventoryPool = FALSE;
    CreateDestroyMapInventoryPoolButtons(fExitFromMapScreen);
  }
}

void ClearUpTempUnSeenList(void) {
  // save these items and all the others
  if (pUnSeenItems == NULL) {
    return;
  }

  // build the list based on this
  pSaveList = pUnSeenItems;
  pUnSeenItems = NULL;

  return;
}

void SaveSeenAndUnseenItems(void) {
  WORLDITEM *pSeenItemsList = NULL;
  int32_t iCounter = 0;
  int32_t iItemCount = 0;
  int32_t iTotalNumberItems = 0;

  // allocate space
  iTotalNumberItems = GetTotalNumberOfItems();

  // if there are seen items, build a temp world items list of them and save them
  if (iTotalNumberItems > 0) {
    pSeenItemsList = (WORLDITEM *)MemAlloc((sizeof(WORLDITEM) * (iTotalNumberItems)));

    // copy
    for (iCounter = 0; iCounter < iTotalNumberOfSlots; iCounter++) {
      if (pInventoryPoolList[iCounter].o.ubNumberOfObjects > 0) {
        // copy object stuff
        memcpy(&(pSeenItemsList[iItemCount]), &(pInventoryPoolList[iCounter]), sizeof(WORLDITEM));

        // check if item actually lives at a gridno
        // if not, check predicessor, iItemCount is not 0
        if (pSeenItemsList[iItemCount].sGridNo == 0) {
          if (iItemCount > 0) {
            // borrow from predicessor
            pSeenItemsList[iItemCount].sGridNo = pSeenItemsList[iItemCount - 1].sGridNo;
          } else {
            // get entry grid location
          }
        }
        pSeenItemsList[iItemCount].fExists = TRUE;
        pSeenItemsList[iItemCount].bVisible = TRUE;
        iItemCount++;
      }
    }
  }

  // if this is the loaded sector handle here
  if ((gWorldSectorX == sSelMapX) && (gWorldSectorY == sSelMapY) &&
      (gbWorldSectorZ == (int8_t)(iCurrentMapSectorZ))) {
    ReBuildWorldItemStashForLoadedSector(iItemCount, uiNumberOfUnSeenItems, pSeenItemsList,
                                         pSaveList);
  } else {
    // now copy over unseen and seen
    if (uiNumberOfUnSeenItems > 0) {
      // over write file and copy unseen
      AddWorldItemsToUnLoadedSector(sSelMapX, sSelMapY, (int8_t)(iCurrentMapSectorZ), 0,
                                    uiNumberOfUnSeenItems, pSaveList, TRUE);

      // check if seen items exist too
      if (iItemCount > 0) {
        AddWorldItemsToUnLoadedSector(sSelMapX, sSelMapY, (int8_t)(iCurrentMapSectorZ), 0,
                                      iItemCount, pSeenItemsList, FALSE);
      }

    } else if (iItemCount > 0) {
      // copy only seen items
      AddWorldItemsToUnLoadedSector(sSelMapX, sSelMapY, (int8_t)(iCurrentMapSectorZ), 0, iItemCount,
                                    pSeenItemsList, TRUE);
    } else {
      // get rid of the file
      SaveWorldItemsToTempItemFile(sSelMapX, sSelMapY, (int8_t)(iCurrentMapSectorZ), 0, NULL);
      return;
    }
  }

  // now clear out seen list
  if (pSeenItemsList != NULL) {
    MemFree(pSeenItemsList);
    pSeenItemsList = NULL;
  }

  // clear out unseen list
  if (pSaveList != NULL) {
    MemFree(pSaveList);
    pSaveList = NULL;
  }

  uiNumberOfUnSeenItems = 0;
  iItemCount = 0;
}

// the screen mask bttn callaback...to disable the inventory and lock out the map itself
void MapInvenPoolScreenMaskCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if ((iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)) {
    fShowMapInventoryPool = FALSE;
  }

  return;
}

void CreateMapInventoryPoolSlots(void) {
  int32_t iCounter = 0;
  int16_t sX = 0, sY = 0;
  int16_t sXA = 0, sYA = 0;
  int16_t sULX = 0, sULY = 0;
  int16_t sBRX = 0, sBRY = 0;

  MSYS_DefineRegion(&MapInventoryPoolMask, MAP_INVENTORY_POOL_SLOT_START_X, 0, 640, 360,
                    MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK,
                    MapInvenPoolScreenMaskCallback);

  for (iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++) {
    sX = (iCounter / MAP_INV_SLOT_COLS);
    sY = (iCounter % (MAP_INV_SLOT_COLS));

    sXA = sX + 1;
    sYA = sY + 1;

    sULX = MAP_INVENTORY_POOL_SLOT_START_X + 4;
    sULY = MAP_INVENTORY_POOL_SLOT_START_Y + 1;

    sULX += (int16_t)(sX * MAP_INVEN_SPACE_BTWN_SLOTS);
    sULY += (int16_t)((sY * MAP_INVEN_SLOT_HEIGHT));

    sBRX = (int16_t)(MAP_INVENTORY_POOL_SLOT_START_X + (sXA * MAP_INVEN_SPACE_BTWN_SLOTS));
    sBRY = (int16_t)(MAP_INVENTORY_POOL_SLOT_START_Y + (sYA * MAP_INVEN_SLOT_HEIGHT)) - 1;

    MSYS_DefineRegion(&MapInventoryPoolSlots[iCounter], sULX, sULY, sBRX, sBRY, MSYS_PRIORITY_HIGH,
                      MSYS_NO_CURSOR, MapInvenPoolSlotsMove, MapInvenPoolSlots);

    MSYS_SetRegionUserData(&MapInventoryPoolSlots[iCounter], 0, iCounter);
  }
}

void DestroyMapInventoryPoolSlots(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++) {
    MSYS_RemoveRegion(&MapInventoryPoolSlots[iCounter]);
  }

  // remove map inventory mask
  MSYS_RemoveRegion(&MapInventoryPoolMask);
}

void MapInvenPoolSlotsMove(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iCounter = 0;

  iCounter = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    iCurrentlyHighLightedItem = iCounter;
    fChangedInventorySlots = TRUE;
    gfCheckForCursorOverMapSectorInventoryItem = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iCurrentlyHighLightedItem = -1;
    fChangedInventorySlots = TRUE;
    gfCheckForCursorOverMapSectorInventoryItem = FALSE;

    // re render radar map
    RenderRadarScreen();
  }
}

void MapInvenPoolSlots(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment screen mask region
  int32_t iCounter = 0;
  int32_t iOldNumberOfObjects = 0;
  wchar_t sString[128];

  iCounter = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)) {
    if (gpItemPointer == NULL) {
      fShowMapInventoryPool = FALSE;
    }
    // else do nothing
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // check if item in cursor, if so, then swap, and no item in curor, pick up, if item in cursor
    // but not box, put in box

    if (gpItemPointer == NULL) {
      // Return if empty
      if (pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .o.usItem == NOTHING)
        return;
    }

    // is this item reachable
    if (!(pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .usFlags &
          WORLD_ITEM_REACHABLE)) {
      if (pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .o.usItem != NOTHING) {
        // not reachable
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[38], MAP_SCREEN, MSG_BOX_FLAG_OK,
                        NULL);
        return;
      }
    }

    // check if selected merc is in this sector, if not, warn them and leave

    // valid character?
    if (gCharactersList[bSelectedInfoChar].fValid == FALSE) {
      DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapInventoryErrorString[1], MAP_SCREEN, MSG_BOX_FLAG_OK,
                      NULL);
      return;
    }

    // if( fShowInventoryFlag )
    {
      // not in sector?
      if ((Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorX != sSelMapX) ||
          (Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorY != sSelMapY) ||
          (Menptr[gCharactersList[bSelectedInfoChar].usSolID].bSectorZ != iCurrentMapSectorZ) ||
          (Menptr[gCharactersList[bSelectedInfoChar].usSolID].fBetweenSectors)) {
        if (gpItemPointer == NULL) {
          swprintf(sString, ARR_SIZE(sString), pMapInventoryErrorString[2],
                   Menptr[gCharactersList[bSelectedInfoChar].usSolID].name);
        } else {
          swprintf(sString, ARR_SIZE(sString), pMapInventoryErrorString[5],
                   Menptr[gCharactersList[bSelectedInfoChar].usSolID].name);
        }
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
        return;
      }
    }

    // If we do not have an item in hand, start moving it
    if (gpItemPointer == NULL) {
      // Return if empty
      if (pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .o.usItem == NOTHING)
        return;

      // if in battle inform player they will have to do this in tactical
      //			if( ( ( gTacticalStatus.fEnemyInSector ) ||( ( sSelMapX ==
      // gWorldSectorX ) && ( sSelMapY == gWorldSectorY ) && ( iCurrentMapSectorZ == gbWorldSectorZ
      // )
      //&& ( gTacticalStatus.uiFlags & INCOMBAT ) ) ) )
      if (!CanPlayerUseSectorInventory(
              GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID))) {
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapInventoryErrorString[3], MAP_SCREEN,
                        MSG_BOX_FLAG_OK, NULL);
        return;
      }

      sObjectSourceGridNo =
          pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .sGridNo;

      BeginInventoryPoolPtr(&(
          pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .o));
    } else {
      if (!CanPlayerUseSectorInventory(
              GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID))) {
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapInventoryErrorString[4], MAP_SCREEN,
                        MSG_BOX_FLAG_OK, NULL);
        return;
      }

      iOldNumberOfObjects =
          pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
              .o.ubNumberOfObjects;

      // Else, try to place here
      if (PlaceObjectInInventoryStash(
              &(pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) +
                                   iCounter]
                    .o),
              gpItemPointer)) {
        // set as reachable and set gridno
        pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) + iCounter]
            .usFlags |= WORLD_ITEM_REACHABLE;

        // if loaded sector, grab grid no of dropping soldier
        // if( ( sSelMapX == gWorldSectorX )&&( gWorldSectorY == sSelMapY ) &&(gbWorldSectorZ ==
        // iCurrentMapSectorZ ) )
        //{
        // nothing here before, then place here
        if (iOldNumberOfObjects == 0) {
          if (sObjectSourceGridNo == NOWHERE) {
            pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) +
                               iCounter]
                .usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;
            pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) +
                               iCounter]
                .sGridNo = sObjectSourceGridNo;
          } else {
            pInventoryPoolList[(iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT) +
                               iCounter]
                .sGridNo = sObjectSourceGridNo;
          }
        }
        //}

        // Check if it's the same now!
        if (gpItemPointer->ubNumberOfObjects == 0) {
          MAPEndItemPointer();
        } else {
          // update ptr
          // now set the cursor
          guiExternVo = GetInterfaceGraphicForItem(&(Item[gpItemPointer->usItem]));
          gusExternVoSubIndex = Item[gpItemPointer->usItem].ubGraphicNum;

          fMapInventoryItem = TRUE;
          MSYS_ChangeRegionCursor(&gMPanelRegion, EXTERN_CURSOR);
          SetCurrentCursorFromDatabase(EXTERN_CURSOR);
        }

        /*
                                        if ( fShowInventoryFlag && bSelectedInfoChar >= 0 )
                                        {
                                                ReevaluateItemHatches( MercPtrs[ gCharactersList[
           bSelectedInfoChar ].usSolID ], FALSE );
                                        }
                                        */
      }
    }

    // dirty region, force update
    MarkForRedrawalStrategicMap();
  }
}

void CreateMapInventoryButtons(void) {
  guiMapInvenButtonImage[0] =
      LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 10, 1, -1, 3, -1);
  guiMapInvenButton[0] = QuickCreateButton(
      guiMapInvenButtonImage[0], 559, 336, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MapInventoryPoolNextBtn);

  guiMapInvenButtonImage[1] =
      LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 9, 0, -1, 2, -1);
  guiMapInvenButton[1] = QuickCreateButton(
      guiMapInvenButtonImage[1], 487, 336, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MapInventoryPoolPrevBtn);

  // reset the current inventory page to be the first page
  iCurrentInventoryPoolPage = 0;

  return;
}

void DestroyMapInventoryButtons(void) {
  RemoveButton(guiMapInvenButton[0]);
  RemoveButton(guiMapInvenButton[1]);

  UnloadButtonImage(guiMapInvenButtonImage[0]);
  UnloadButtonImage(guiMapInvenButtonImage[1]);
  return;
}

void BuildStashForSelectedSector(uint8_t sMapX, uint8_t sMapY, int8_t sMapZ) {
  int32_t iSize = 0;
  uint32_t uiItemCount = 0;
  uint32_t uiTotalNumberOfItems = 0, uiTotalNumberOfRealItems = 0;
  WORLDITEM *pTotalSectorList = NULL;
  int32_t iCounter = 0;
  uint32_t uiTotalNumberOfSeenItems = 0;

  //	#ifdef _DEBUG
  BOOLEAN fReturn = TRUE;
  //	#endif

  // get size of the current stash in sector (count stacks as one item)
  iSize = GetSizeOfStashInSector(sMapX, sMapY, sMapZ, TRUE);

  // round off .. we want at least 1 free page of space...
  iSize = (iSize - (iSize % MAP_INVENTORY_POOL_SLOT_COUNT)) + MAP_INVENTORY_POOL_SLOT_COUNT;

  iTotalNumberOfSlots = iSize;

  // allocate space for list
  pInventoryPoolList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * iSize);

  memset(pInventoryPoolList, 0, sizeof(WORLDITEM) * iSize);

  iLastInventoryPoolPage = ((iTotalNumberOfSlots - 1) / MAP_INVENTORY_POOL_SLOT_COUNT);

  uiNumberOfUnSeenItems = 0;

  // now laod these items into memory, based on fact if sector is in fact loaded
  if ((sMapX == gWorldSectorX) && (gWorldSectorY == sMapY) && (gbWorldSectorZ == sMapZ)) {
    // sector loaded, just copy from list
    for (iCounter = 0; (uint32_t)(iCounter) < guiNumWorldItems; iCounter++) {
      // check if visible, if so, then copy over object type
      // if visible to player, then state fact

      /*
                              if( gWorldItems[ iCounter].bVisible == 1 &&
                                              gWorldItems[ iCounter ].fExists &&
                                              gWorldItems[ iCounter ].o.usItem != SWITCH &&
                                              gWorldItems[ iCounter ].o.bTrap <= 0 )
      */
      if (IsMapScreenWorldItemVisibleInMapInventory(&gWorldItems[iCounter])) {
        // one more item
        memcpy(&(pInventoryPoolList[uiItemCount]), &(gWorldItems[iCounter]), sizeof(WORLDITEM));
        uiItemCount++;
      }
    }

    uiTotalNumberOfSeenItems = uiItemCount;

    // now allocate space for all the unseen items
    if (guiNumWorldItems > uiItemCount) {
      pUnSeenItems = (WORLDITEM *)MemAlloc((guiNumWorldItems - uiItemCount) * sizeof(WORLDITEM));

      uiItemCount = 0;

      // now copy over
      for (iCounter = 0; (uint32_t)iCounter < guiNumWorldItems; iCounter++) {
        //				if( ( gWorldItems[ iCounter ].bVisible  != 1 ) &&
        //						( gWorldItems[ iCounter
        //].o.ubNumberOfObjects
        //>
        // 0
        //)
        //&& 							gWorldItems[ iCounter ].fExists )
        if (IsMapScreenWorldItemInvisibleInMapInventory(&gWorldItems[iCounter])) {
          // one more item
          memcpy(&(pUnSeenItems[uiItemCount]), &(gWorldItems[iCounter]), sizeof(WORLDITEM));

          uiItemCount++;
        }
      }

      // copy over number of unseen
      uiNumberOfUnSeenItems = uiItemCount;
    }
  } else {
    // not loaded, load
    // get total number, visable and invisible
    fReturn = GetNumberOfWorldItemsFromTempItemFile(sMapX, sMapY, (int8_t)(sMapZ),
                                                    &(uiTotalNumberOfItems), FALSE);
    Assert(fReturn);

    fReturn = GetNumberOfActiveWorldItemsFromTempFile(sMapX, sMapY, (int8_t)(sMapZ),
                                                      &(uiTotalNumberOfRealItems));
    Assert(fReturn);

    if (uiTotalNumberOfRealItems > 0) {
      // allocate space for the list
      pTotalSectorList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * uiTotalNumberOfItems);

      // now load into mem
      LoadWorldItemsFromTempItemFile(sMapX, sMapY, (int8_t)(sMapZ), pTotalSectorList);
    }

    // now run through list and
    for (iCounter = 0; (uint32_t)(iCounter) < uiTotalNumberOfRealItems; iCounter++) {
      // if visible to player, then state fact
      /*
                              if( pTotalSectorList[ iCounter].bVisible == 1 &&
                                              pTotalSectorList[ iCounter].fExists &&
                                              pTotalSectorList[ iCounter].o.usItem != SWITCH &&
                                              pTotalSectorList[ iCounter].o.bTrap <= 0 )
      */

      // TEST!!  If the item exists, and is NOT VALID, report it
      if (pTotalSectorList[iCounter].fExists && pTotalSectorList[iCounter].o.usItem > MAXITEMS) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
                  L"The %d item in the list is NOT valid. Please send save.  DF 1.", iCounter);
      }

      if (IsMapScreenWorldItemVisibleInMapInventory(&pTotalSectorList[iCounter])) {
        // one more item
        memcpy(&(pInventoryPoolList[uiItemCount]), &(pTotalSectorList[iCounter]),
               sizeof(WORLDITEM));

        uiItemCount++;
      }
    }

    uiTotalNumberOfSeenItems = uiItemCount;

    // now allocate space for all the unseen items
    if (uiTotalNumberOfRealItems > uiItemCount) {
      pUnSeenItems =
          (WORLDITEM *)MemAlloc((uiTotalNumberOfRealItems - uiItemCount) * sizeof(WORLDITEM));

      uiItemCount = 0;

      // now copy over
      for (iCounter = 0; (uint32_t)iCounter < uiTotalNumberOfItems; iCounter++) {
        /*
                                        if( ( pTotalSectorList[ iCounter].bVisible  != 1 ) &&
                                                        ( pTotalSectorList[
           iCounter].o.ubNumberOfObjects > 0 ) && pTotalSectorList[ iCounter].fExists )
        */
        if (IsMapScreenWorldItemInvisibleInMapInventory(&pTotalSectorList[iCounter])) {
          // one more item
          memcpy(&(pUnSeenItems[uiItemCount]), &(pTotalSectorList[iCounter]), sizeof(WORLDITEM));

          uiItemCount++;
        }
      }

      // copy over number of unseen
      uiNumberOfUnSeenItems = uiItemCount;
    }

    // if anything was alloced, then get rid of it
    if (uiTotalNumberOfRealItems > 0) {
      MemFree(pTotalSectorList);
    }
  }

  // Check to see if any of the items in the list have a gridno of NOWHERE and the entry point flag
  // NOT set
  CheckGridNoOfItemsInMapScreenMapInventory();

  // Sort the sector invenrtory
  SortSectorInventory(pInventoryPoolList, uiTotalNumberOfSeenItems);
}

void ReBuildWorldItemStashForLoadedSector(int32_t iNumberSeenItems, int32_t iNumberUnSeenItems,
                                          WORLDITEM *pSeenItemsList, WORLDITEM *pUnSeenItemsList) {
  int32_t iTotalNumberOfItems = 0;
  int32_t iCurrentItem = 0;
  int32_t iCounter = 0;
  int32_t iRemainder = 0;
  uint32_t uiTotalNumberOfVisibleItems = 0;
  WORLDITEM *pTotalList = NULL;

  // clear out the list
  TrashWorldItems();

  // get total number of items
  iTotalNumberOfItems = iNumberUnSeenItems + iNumberSeenItems;

  iRemainder = iTotalNumberOfItems % 10;

  // if there is a remainder, then add onto end of list
  if (iRemainder) {
    iTotalNumberOfItems += 10 - iRemainder;
  }

  // allocate space for items
  pTotalList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * iTotalNumberOfItems);

  for (iCounter = 0; iCounter < iTotalNumberOfItems; iCounter++) {
    // clear out the structure
    memset(&(pTotalList[iCounter]), 0, sizeof(WORLDITEM));
  }

  // place seen items in the world
  for (iCounter = 0; iCounter < iNumberSeenItems; iCounter++) {
    memcpy(&(pTotalList[iCurrentItem]), &(pSeenItemsList[iCounter]), sizeof(WORLDITEM));
    iCurrentItem++;
  }

  // now store the unseen item list
  for (iCounter = 0; iCounter < iNumberUnSeenItems; iCounter++) {
    memcpy(&(pTotalList[iCurrentItem]), &(pUnSeenItemsList[iCounter]), sizeof(WORLDITEM));
    iCurrentItem++;
  }

  RefreshItemPools(pTotalList, iTotalNumberOfItems);

  // Count the total number of visible items
  for (iCounter = 0; iCounter < iNumberSeenItems; iCounter++) {
    uiTotalNumberOfVisibleItems += pSeenItemsList[iCounter].o.ubNumberOfObjects;
  }

  // reset the visible item count in the sector info struct
  SetNumberOfVisibleWorldItemsInSectorStructureForSector(
      (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ, uiTotalNumberOfVisibleItems);

  // clear out allocated space for total list
  MemFree(pTotalList);

  // reset total list
  pTotalList = NULL;

  return;
}

void ReSizeStashListByThisAmount(int32_t iNumberOfItems) {
  int32_t iSizeOfList = iTotalNumberOfSlots;
  WORLDITEM *pOldList;

  // no items added, leave
  if (iNumberOfItems == 0) {
    return;
  }

  iTotalNumberOfSlots += iNumberOfItems;

  pOldList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * iSizeOfList);
  memset(pOldList, 0, sizeof(WORLDITEM) * iSizeOfList);

  memcpy(pOldList, pInventoryPoolList, sizeof(WORLDITEM) * iSizeOfList);

  // rebuild stash
  pInventoryPoolList =
      (WORLDITEM *)MemRealloc(pInventoryPoolList, sizeof(WORLDITEM) * iTotalNumberOfSlots);

  // set new mem to 0
  memset(pInventoryPoolList, 0, sizeof(WORLDITEM) * iTotalNumberOfSlots);

  // copy old info over
  memcpy(pInventoryPoolList, pOldList, sizeof(WORLDITEM) * iSizeOfList);

  // free memeory
  MemFree(pOldList);

  return;
}

void DestroyStash(void) {
  // clear out stash
  MemFree(pInventoryPoolList);
}

int32_t GetSizeOfStashInSector(uint8_t sMapX, uint8_t sMapY, int8_t sMapZ,
                               BOOLEAN fCountStacksAsOne) {
  // get # of items in sector that are visible to the player
  uint32_t uiTotalNumberOfItems = 0, uiTotalNumberOfRealItems = 0;
  WORLDITEM *pTotalSectorList = NULL;
  uint32_t uiItemCount = 0;
  int32_t iCounter = 0;
  BOOLEAN fReturn = TRUE;

  if ((sMapX == gWorldSectorX) && (sMapY == gWorldSectorY) && (sMapZ == gbWorldSectorZ)) {
    uiTotalNumberOfItems = guiNumWorldItems;

    // now run through list and
    for (iCounter = 0; (uint32_t)(iCounter) < uiTotalNumberOfItems; iCounter++) {
      // if visible to player, then state fact
      //			if( gWorldItems[ iCounter ].bVisible == 1 && gWorldItems[ iCounter
      //].fExists )
      if (IsMapScreenWorldItemVisibleInMapInventory(&gWorldItems[iCounter])) {
        // add it
        if (fCountStacksAsOne) {
          uiItemCount++;
        } else {
          uiItemCount += gWorldItems[iCounter].o.ubNumberOfObjects;
        }
      }
    }
  } else {
    // get total number, visable and invisible
    fReturn = GetNumberOfActiveWorldItemsFromTempFile(sMapX, sMapY, (int8_t)(sMapZ),
                                                      &(uiTotalNumberOfRealItems));
    Assert(fReturn);

    fReturn = GetNumberOfWorldItemsFromTempItemFile(sMapX, sMapY, (int8_t)(sMapZ),
                                                    &(uiTotalNumberOfItems), FALSE);
    Assert(fReturn);

    if (uiTotalNumberOfItems > 0) {
      // allocate space for the list
      pTotalSectorList = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * uiTotalNumberOfItems);

      // now load into mem
      LoadWorldItemsFromTempItemFile(sMapX, sMapY, (int8_t)(sMapZ), pTotalSectorList);
    }

    // now run through list and
    for (iCounter = 0; (uint32_t)(iCounter) < uiTotalNumberOfRealItems; iCounter++) {
      // if visible to player, then state fact
      //			if( pTotalSectorList[ iCounter ].bVisible == 1 && pTotalSectorList[
      // iCounter ].fExists )
      if (IsMapScreenWorldItemVisibleInMapInventory(&pTotalSectorList[iCounter])) {
        // add it
        if (fCountStacksAsOne) {
          uiItemCount++;
        } else {
          uiItemCount += pTotalSectorList[iCounter].o.ubNumberOfObjects;
        }
      }
    }

    // if anything was alloced, then get rid of it
    if (pTotalSectorList != NULL) {
      MemFree(pTotalSectorList);
      pTotalSectorList = NULL;

#ifdef JA2BETAVERSION
      if (uiTotalNumberOfItems == 0) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
                  L"pTotalSectorList is NOT NULL when uiTotalNumberOfRealItems is %d.",
                  uiTotalNumberOfRealItems);
      }
#endif
    }
  }

  return (uiItemCount);
}

void BeginInventoryPoolPtr(struct OBJECTTYPE *pInventorySlot) {
  BOOLEAN fOk = FALSE;

  // If not null return
  if (gpItemPointer != NULL) {
    return;
  }

  // if shift key get all

  if (_KeyDown(SHIFT)) {
    // Remove all from soldier's slot
    fOk = RemoveObjectFromStashSlot(pInventorySlot, &gItemPointer);
  } else {
    GetObjFromInventoryStashSlot(pInventorySlot, &gItemPointer);
    fOk = (gItemPointer.ubNumberOfObjects == 1);
  }

  if (fOk) {
    // Dirty interface
    MarkForRedrawalStrategicMap();
    gpItemPointer = &gItemPointer;

    gpItemPointerSoldier = NULL;

    // now set the cursor
    guiExternVo = GetInterfaceGraphicForItem(&(Item[gpItemPointer->usItem]));
    gusExternVoSubIndex = Item[gpItemPointer->usItem].ubGraphicNum;

    fMapInventoryItem = TRUE;
    MSYS_ChangeRegionCursor(&gMPanelRegion, EXTERN_CURSOR);
    SetCurrentCursorFromDatabase(EXTERN_CURSOR);

    if (fShowInventoryFlag && bSelectedInfoChar >= 0) {
      ReevaluateItemHatches(MercPtrs[gCharactersList[bSelectedInfoChar].usSolID], FALSE);
      fTeamPanelDirty = TRUE;
    }
  }
}

// get this item out of the stash slot
BOOLEAN GetObjFromInventoryStashSlot(struct OBJECTTYPE *pInventorySlot,
                                     struct OBJECTTYPE *pItemPtr) {
  // item ptr
  if (!pItemPtr) {
    return (FALSE);
  }

  // if there are only one item in slot, just copy
  if (pInventorySlot->ubNumberOfObjects == 1) {
    memcpy(pItemPtr, pInventorySlot, sizeof(struct OBJECTTYPE));
    DeleteObj(pInventorySlot);
  } else {
    // take one item
    pItemPtr->usItem = pInventorySlot->usItem;

    // find first unempty slot
    pItemPtr->bStatus[0] = pInventorySlot->bStatus[0];
    pItemPtr->ubNumberOfObjects = 1;
    pItemPtr->ubWeight = CalculateObjectWeight(pItemPtr);
    RemoveObjFrom(pInventorySlot, 0);
    pInventorySlot->ubWeight = CalculateObjectWeight(pInventorySlot);
  }

  return (TRUE);
}

BOOLEAN RemoveObjectFromStashSlot(struct OBJECTTYPE *pInventorySlot, struct OBJECTTYPE *pItemPtr) {
  CHECKF(pInventorySlot);

  if (pInventorySlot->ubNumberOfObjects == 0) {
    return (FALSE);
  } else {
    memcpy(pItemPtr, pInventorySlot, sizeof(struct OBJECTTYPE));
    DeleteObj(pInventorySlot);
    return (TRUE);
  }
}

BOOLEAN PlaceObjectInInventoryStash(struct OBJECTTYPE *pInventorySlot,
                                    struct OBJECTTYPE *pItemPtr) {
  uint8_t ubNumberToDrop, ubSlotLimit, ubLoop;

  // if there is something there, swap it, if they are of the same type and stackable then add to
  // the count

  ubSlotLimit = Item[pItemPtr->usItem].ubPerPocket;

  if (pInventorySlot->ubNumberOfObjects == 0) {
    // placement in an empty slot
    ubNumberToDrop = pItemPtr->ubNumberOfObjects;

    if (ubNumberToDrop > ubSlotLimit && ubSlotLimit != 0) {
      // drop as many as possible into pocket
      ubNumberToDrop = ubSlotLimit;
    }

    // could be wrong type of object for slot... need to check...
    // but assuming it isn't
    memcpy(pInventorySlot, pItemPtr, sizeof(struct OBJECTTYPE));

    if (ubNumberToDrop != pItemPtr->ubNumberOfObjects) {
      // in the InSlot copy, zero out all the objects we didn't drop
      for (ubLoop = ubNumberToDrop; ubLoop < pItemPtr->ubNumberOfObjects; ubLoop++) {
        pInventorySlot->bStatus[ubLoop] = 0;
      }
    }
    pInventorySlot->ubNumberOfObjects = ubNumberToDrop;

    // remove a like number of objects from pObj
    RemoveObjs(pItemPtr, ubNumberToDrop);
  } else {
    // replacement/reloading/merging/stacking

    // placement in an empty slot
    ubNumberToDrop = pItemPtr->ubNumberOfObjects;

    if (pItemPtr->usItem == pInventorySlot->usItem) {
      if (pItemPtr->usItem == MONEY) {
        // always allow money to be combined!
        // average out the status values using a weighted average...
        pInventorySlot->bStatus[0] =
            (int8_t)(((uint32_t)pInventorySlot->bMoneyStatus * pInventorySlot->uiMoneyAmount +
                      (uint32_t)pItemPtr->bMoneyStatus * pItemPtr->uiMoneyAmount) /
                     (pInventorySlot->uiMoneyAmount + pItemPtr->uiMoneyAmount));
        pInventorySlot->uiMoneyAmount += pItemPtr->uiMoneyAmount;

        DeleteObj(pItemPtr);
      } else if (ubSlotLimit < 2) {
        // swapping
        SwapObjs(pItemPtr, pInventorySlot);
      } else {
        // stacking
        if (ubNumberToDrop > ubSlotLimit - pInventorySlot->ubNumberOfObjects) {
          ubNumberToDrop = ubSlotLimit - pInventorySlot->ubNumberOfObjects;
        }

        StackObjs(pItemPtr, pInventorySlot, ubNumberToDrop);
      }
    } else {
      SwapObjs(pItemPtr, pInventorySlot);
    }
  }
  return (TRUE);
}

BOOLEAN AutoPlaceObjectInInventoryStash(struct OBJECTTYPE *pItemPtr) {
  uint8_t ubNumberToDrop, ubSlotLimit, ubLoop;
  struct OBJECTTYPE *pInventorySlot;

  // if there is something there, swap it, if they are of the same type and stackable then add to
  // the count
  pInventorySlot = &(pInventoryPoolList[iTotalNumberOfSlots].o);

  // placement in an empty slot
  ubNumberToDrop = pItemPtr->ubNumberOfObjects;

  ubSlotLimit = ItemSlotLimit(pItemPtr->usItem, BIGPOCK1POS);

  if (ubNumberToDrop > ubSlotLimit && ubSlotLimit != 0) {
    // drop as many as possible into pocket
    ubNumberToDrop = ubSlotLimit;
  }

  // could be wrong type of object for slot... need to check...
  // but assuming it isn't
  memcpy(pInventorySlot, pItemPtr, sizeof(struct OBJECTTYPE));

  if (ubNumberToDrop != pItemPtr->ubNumberOfObjects) {
    // in the InSlot copy, zero out all the objects we didn't drop
    for (ubLoop = ubNumberToDrop; ubLoop < pItemPtr->ubNumberOfObjects; ubLoop++) {
      pInventorySlot->bStatus[ubLoop] = 0;
    }
  }
  pInventorySlot->ubNumberOfObjects = ubNumberToDrop;

  // remove a like number of objects from pObj
  RemoveObjs(pItemPtr, ubNumberToDrop);

  return (TRUE);
}

void MapInventoryPoolNextBtn(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // if can go to next page, go there
      if (iCurrentInventoryPoolPage < (iLastInventoryPoolPage)) {
        iCurrentInventoryPoolPage++;
        MarkForRedrawalStrategicMap();
      }
    }
  }
}

void MapInventoryPoolPrevBtn(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // if can go to next page, go there
      if (iCurrentInventoryPoolPage > 0) {
        iCurrentInventoryPoolPage--;
        MarkForRedrawalStrategicMap();
      }
    }
  }
}

void MapInventoryPoolDoneBtn(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // done
      fShowMapInventoryPool = FALSE;
    }
  }
}

void DisplayPagesForMapInventoryPool(void) {
  // get the current and last pages and display them
  wchar_t sString[32];
  int16_t sX, sY;

  SetFont(COMPFONT);
  SetFontForeground(183);
  SetFontBackground(FONT_BLACK);

  // set the buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  // grab current and last pages
  swprintf(sString, ARR_SIZE(sString), L"%d / %d", iCurrentInventoryPoolPage + 1,
           iLastInventoryPoolPage + 1);

  // grab centered coords
  FindFontCenterCoordinates(MAP_INVENTORY_POOL_PAGE_X, MAP_INVENTORY_POOL_PAGE_Y,
                            MAP_INVENTORY_POOL_PAGE_WIDTH, MAP_INVENTORY_POOL_PAGE_HEIGHT, sString,
                            MAP_SCREEN_FONT, &sX, &sY);

  mprintf(sX, sY, sString);

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
}

int32_t GetTotalNumberOfItemsInSectorStash(void) {
  int32_t iCounter, iCount = 0;

  // run through list of items and find out how many are there
  for (iCounter = 0; iCounter < iTotalNumberOfSlots; iCounter++) {
    if (pInventoryPoolList[iCounter].o.ubNumberOfObjects > 0) {
      iCount += pInventoryPoolList[iCounter].o.ubNumberOfObjects;
    }
  }

  return iCount;
}

int32_t GetTotalNumberOfItems(void) {
  int32_t iCounter, iCount = 0;

  // run through list of items and find out how many are there
  for (iCounter = 0; iCounter < iTotalNumberOfSlots; iCounter++) {
    if (pInventoryPoolList[iCounter].o.ubNumberOfObjects > 0) {
      iCount++;
    }
  }

  return iCount;
}

void DrawNumberOfIventoryPoolItems(void) {
  int32_t iNumberOfItems = 0;
  wchar_t sString[32];
  int16_t sX, sY;

  iNumberOfItems = GetTotalNumberOfItemsInSectorStash();

  // get number of items
  swprintf(sString, ARR_SIZE(sString), L"%d", iNumberOfItems);

  // set font stuff
  SetFont(COMPFONT);
  SetFontForeground(183);
  SetFontBackground(FONT_BLACK);

  // set the buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  // grab centered coords
  FindFontCenterCoordinates(MAP_INVENTORY_POOL_NUMBER_X, MAP_INVENTORY_POOL_PAGE_Y,
                            MAP_INVENTORY_POOL_NUMBER_WIDTH, MAP_INVENTORY_POOL_PAGE_HEIGHT,
                            sString, MAP_SCREEN_FONT, &sX, &sY);

  mprintf(sX, sY, sString);

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  return;
}

void CreateMapInventoryPoolDoneButton(void) {
  // create done button
  guiMapInvenButtonImage[2] = LoadButtonImage("INTERFACE\\done_button.sti", -1, 0, -1, 1, -1);
  guiMapInvenButton[2] = QuickCreateButton(
      guiMapInvenButtonImage[2], 587, 333, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MapInventoryPoolDoneBtn);

  return;
}

void DestroyInventoryPoolDoneButton(void) {
  // destroy ddone button

  RemoveButton(guiMapInvenButton[2]);
  UnloadButtonImage(guiMapInvenButtonImage[2]);

  return;
}

void DisplayCurrentSector(void) {
  // grab current sector being displayed
  wchar_t sString[32];
  int16_t sX, sY;

  swprintf(sString, ARR_SIZE(sString), L"%s%s%s", pMapVertIndex[sSelMapY], pMapHortIndex[sSelMapX],
           pMapDepthIndex[iCurrentMapSectorZ]);

  // set font stuff
  SetFont(COMPFONT);
  SetFontForeground(183);
  SetFontBackground(FONT_BLACK);

  // set the buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  // grab centered coords
  FindFontCenterCoordinates(MAP_INVENTORY_POOL_LOC_X, MAP_INVENTORY_POOL_PAGE_Y,
                            MAP_INVENTORY_POOL_LOC_WIDTH, MAP_INVENTORY_POOL_PAGE_HEIGHT, sString,
                            MAP_SCREEN_FONT, &sX, &sY);

  mprintf(sX, sY, sString);

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
}

void CheckAndUnDateSlotAllocation(void) {
  // will check number of available slots, if less than half a page, allocate a new page
  int32_t iNumberOfTakenSlots = 0;

  // get number of taken slots
  iNumberOfTakenSlots = GetTotalNumberOfItems();

  if ((iTotalNumberOfSlots - iNumberOfTakenSlots) < 2) {
    // not enough space
    // need to make more space
    ReSizeStashListByThisAmount(MAP_INVENTORY_POOL_SLOT_COUNT);
  }

  iLastInventoryPoolPage = ((iTotalNumberOfSlots - 1) / MAP_INVENTORY_POOL_SLOT_COUNT);

  return;
}

void DrawTextOnMapInventoryBackground(void) {
  //	wchar_t sString[ 64 ];
  uint16_t usStringHeight;

  SetFont(MAP_IVEN_FONT);
  SetFontBackground(FONT_BLACK);
  SetFontForeground(FONT_BEIGE);

  // set the buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  // Calculate the height of the string, as it needs to be vertically centered.
  usStringHeight =
      DisplayWrappedString(268, 342, 53, 1, MAP_IVEN_FONT, FONT_BEIGE, pMapInventoryStrings[0],
                           FONT_BLACK, FALSE, RIGHT_JUSTIFIED | DONT_DISPLAY_TEXT);
  DisplayWrappedString(268, (uint16_t)(342 - (usStringHeight / 2)), 53, 1, MAP_IVEN_FONT,
                       FONT_BEIGE, pMapInventoryStrings[0], FONT_BLACK, FALSE, RIGHT_JUSTIFIED);

  // Calculate the height of the string, as it needs to be vertically centered.
  usStringHeight =
      DisplayWrappedString(369, 342, 65, 1, MAP_IVEN_FONT, FONT_BEIGE, pMapInventoryStrings[1],
                           FONT_BLACK, FALSE, RIGHT_JUSTIFIED | DONT_DISPLAY_TEXT);
  DisplayWrappedString(369, (uint16_t)(342 - (usStringHeight / 2)), 65, 1, MAP_IVEN_FONT,
                       FONT_BEIGE, pMapInventoryStrings[1], FONT_BLACK, FALSE, RIGHT_JUSTIFIED);

  DrawTextOnSectorInventory();

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  return;
}

void HandleButtonStatesWhileMapInventoryActive(void) {
  // are we even showing the amp inventory pool graphic?
  if (fShowMapInventoryPool == FALSE) {
    return;
  }

  // first page, can't go back any
  if (iCurrentInventoryPoolPage == 0) {
    DisableButton(guiMapInvenButton[1]);
  } else {
    EnableButton(guiMapInvenButton[1]);
  }

  // last page, go no further
  if (iCurrentInventoryPoolPage == iLastInventoryPoolPage) {
    DisableButton(guiMapInvenButton[0]);
  } else {
    EnableButton(guiMapInvenButton[0]);
  }

  // item picked up ..disable button
  if (gMPanelRegion.Cursor == EXTERN_CURSOR) {
    DisableButton(guiMapInvenButton[2]);
  } else {
    EnableButton(guiMapInvenButton[2]);
  }
}

void DrawTextOnSectorInventory(void) {
  int16_t sX = 0, sY = 0;
  wchar_t sString[64];

  // parse the string
  swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[11]);

  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  FindFontCenterCoordinates(MAP_INVENTORY_POOL_SLOT_START_X, MAP_INVENTORY_POOL_SLOT_START_Y - 20,
                            630 - MAP_INVENTORY_POOL_SLOT_START_X, GetFontHeight(FONT14ARIAL),
                            sString, FONT14ARIAL, &sX, &sY);

  SetFont(FONT14ARIAL);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  mprintf(sX, sY, sString);

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
}

void HandleFlashForHighLightedItem(void) {
  int32_t iCurrentTime = 0;
  int32_t iDifference = 0;

  // if there is an invalid item, reset
  if (iCurrentlyHighLightedItem == -1) {
    fFlashHighLightInventoryItemOnradarMap = FALSE;
    giFlashHighlightedItemBaseTime = 0;
  }

  // get the current time
  iCurrentTime = GetJA2Clock();

  // if there basetime is uninit
  if (giFlashHighlightedItemBaseTime == 0) {
    giFlashHighlightedItemBaseTime = iCurrentTime;
  }

  iDifference = iCurrentTime - giFlashHighlightedItemBaseTime;

  if (iDifference > DELAY_FOR_HIGHLIGHT_ITEM_FLASH) {
    // reset timer
    giFlashHighlightedItemBaseTime = iCurrentTime;

    // flip flag
    fFlashHighLightInventoryItemOnradarMap = !fFlashHighLightInventoryItemOnradarMap;

    // re render radar map
    RenderRadarScreen();
  }
}

void HandleMouseInCompatableItemForMapSectorInventory(int32_t iCurrentSlot) {
  struct SOLDIERTYPE *pSoldier = NULL;
  static BOOLEAN fItemWasHighLighted = FALSE;

  if (iCurrentSlot == -1) {
    giCompatibleItemBaseTime = 0;
  }

  if (fChangedInventorySlots == TRUE) {
    giCompatibleItemBaseTime = 0;
    fChangedInventorySlots = FALSE;
  }

  // reset the base time to the current game clock
  if (giCompatibleItemBaseTime == 0) {
    giCompatibleItemBaseTime = GetJA2Clock();

    if (fItemWasHighLighted == TRUE) {
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      fItemWasHighLighted = FALSE;
    }
  }

  ResetCompatibleItemArray();
  ResetMapSectorInventoryPoolHighLights();

  if (iCurrentSlot == -1) {
    return;
  }

  // given this slot value, check if anything in the displayed sector inventory or on the mercs
  // inventory is compatable
  if (fShowInventoryFlag) {
    // check if any compatable items in the soldier inventory matches with this item
    if (gfCheckForCursorOverMapSectorInventoryItem) {
      pSoldier = GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID);
      if (pSoldier) {
        if (HandleCompatibleAmmoUIForMapScreen(
                pSoldier,
                iCurrentSlot + (iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT), TRUE,
                FALSE)) {
          if (GetJA2Clock() - giCompatibleItemBaseTime > 100) {
            if (fItemWasHighLighted == FALSE) {
              fTeamPanelDirty = TRUE;
              fItemWasHighLighted = TRUE;
            }
          }
        }
      }
    } else {
      giCompatibleItemBaseTime = 0;
    }
  }

  // now handle for the sector inventory
  if (fShowMapInventoryPool) {
    // check if any compatable items in the soldier inventory matches with this item
    if (gfCheckForCursorOverMapSectorInventoryItem) {
      if (HandleCompatibleAmmoUIForMapInventory(
              pSoldier, iCurrentSlot, (iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT),
              TRUE, FALSE)) {
        if (GetJA2Clock() - giCompatibleItemBaseTime > 100) {
          if (fItemWasHighLighted == FALSE) {
            fItemWasHighLighted = TRUE;
            MarkForRedrawalStrategicMap();
          }
        }
      }
    } else {
      giCompatibleItemBaseTime = 0;
    }
  }

  return;
}

void ResetMapSectorInventoryPoolHighLights(void) {
  int32_t iCounter = 0;

  // now reset the highlight list for the map sector inventory
  for (iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++) {
    if (fMapInventoryItemCompatable[iCounter]) {
      fMapInventoryItemCompatable[iCounter] = FALSE;
    }
  }

  return;
}
void HandleMapSectorInventory(void) {
  // handle mouse in compatable item map sectors inventory
  HandleMouseInCompatableItemForMapSectorInventory(iCurrentlyHighLightedItem);

  return;
}

// CJC look here to add/remove checks for the sector inventory
BOOLEAN IsMapScreenWorldItemVisibleInMapInventory(WORLDITEM *pWorldItem) {
  if (pWorldItem->bVisible == 1 && pWorldItem->fExists && pWorldItem->o.usItem != SWITCH &&
      pWorldItem->o.usItem != ACTION_ITEM && pWorldItem->o.bTrap <= 0) {
    return (TRUE);
  }

  return (FALSE);
}

// CJC look here to add/remove checks for the sector inventory
BOOLEAN IsMapScreenWorldItemInvisibleInMapInventory(WORLDITEM *pWorldItem) {
  if (pWorldItem->fExists && !IsMapScreenWorldItemVisibleInMapInventory(pWorldItem)) {
    return (TRUE);
  }

  return (FALSE);
}

// Check to see if any of the items in the list have a gridno of NOWHERE and the entry point flag
// NOT set
void CheckGridNoOfItemsInMapScreenMapInventory() {
  int32_t iCnt;
  uint32_t uiNumFlagsNotSet = 0;
  int32_t iTotalNumberItems = GetTotalNumberOfItems();

  for (iCnt = 0; iCnt < iTotalNumberItems; iCnt++) {
    if (pInventoryPoolList[iCnt].sGridNo == NOWHERE &&
        !(pInventoryPoolList[iCnt].usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT)) {
      // set the flag
      pInventoryPoolList[iCnt].usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;

      // count the number
      uiNumFlagsNotSet++;
    }
  }

  // loop through all the UNSEEN items
  for (iCnt = 0; iCnt < (int32_t)uiNumberOfUnSeenItems; iCnt++) {
    if (pUnSeenItems[iCnt].sGridNo == NOWHERE &&
        !(pUnSeenItems[iCnt].usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT)) {
      // set the flag
      pUnSeenItems[iCnt].usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;

      // count the number
      uiNumFlagsNotSet++;
    }
  }

#ifdef JA2BETAVERSION
  if (uiNumFlagsNotSet > 0) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION,
              L"Item with invalid gridno doesnt have flag set.", uiNumFlagsNotSet);
  }
#endif
}

void SortSectorInventory(WORLDITEM *pInventory, uint32_t uiSizeOfArray) {
  qsort(pInventory, (size_t)uiSizeOfArray, sizeof(WORLDITEM), MapScreenSectorInventoryCompare);
}

int32_t MapScreenSectorInventoryCompare(const void *pNum1, const void *pNum2) {
  WORLDITEM *pFirst = (WORLDITEM *)pNum1;
  WORLDITEM *pSecond = (WORLDITEM *)pNum2;
  uint16_t usItem1Index;
  uint16_t usItem2Index;
  uint8_t ubItem1Quality;
  uint8_t ubItem2Quality;

  usItem1Index = pFirst->o.usItem;
  usItem2Index = pSecond->o.usItem;

  ubItem1Quality = pFirst->o.bStatus[0];
  ubItem2Quality = pSecond->o.bStatus[0];

  return (CompareItemsForSorting(usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality));
}

BOOLEAN CanPlayerUseSectorInventory(struct SOLDIERTYPE *pSelectedSoldier) {
  uint8_t sSectorX, sSectorY;
  int8_t sSectorZ;
  BOOLEAN fInCombat;

  // Get the sector that has a battle
  fInCombat =
      GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(&sSectorX, &sSectorY, &sSectorZ);

  // if there is a battle going on
  if (fInCombat) {
    // if the selected map is the one with the combat
    if (((sSelMapX == sSectorX) && (sSelMapY == sSectorY) && (iCurrentMapSectorZ == sSectorZ))) {
      return (FALSE);
    }
  }

  return (TRUE);
}
