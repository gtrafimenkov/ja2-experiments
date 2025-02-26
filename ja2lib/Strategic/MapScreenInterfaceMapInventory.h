// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _MAP_INTERFACE_MAP_INVEN_H
#define _MAP_INTERFACE_MAP_INVEN_H

#include "SGP/Types.h"
#include "Tactical/WorldItems.h"

// this is how close one has to be in the loaded sector to pickup an item
#define MAX_DISTANCE_TO_PICKUP_ITEM 5

// number of inventory slots
#define MAP_INVENTORY_POOL_SLOT_COUNT 45

// whether we are showing the inventory pool graphic
extern BOOLEAN fShowMapInventoryPool;

// load inventory pool graphic
BOOLEAN LoadInventoryPoolGraphic(void);

// remove inventory pool graphic
void RemoveInventoryPoolGraphic(void);

// blit the inventory graphic
void BlitInventoryPoolGraphic(void);

// which buttons in map invneotyr panel?
void HandleButtonStatesWhileMapInventoryActive(void);

// handle creation and destruction of map inventory pool buttons
void CreateDestroyMapInventoryPoolButtons(BOOLEAN fExitFromMapScreen);

// bail out of sector inventory mode if it is on
void CancelSectorInventoryDisplayIfOn(BOOLEAN fExitFromMapScreen);

int32_t GetSizeOfStashInSector(uint8_t sMapX, uint8_t sMapY, int8_t sMapZ,
                               BOOLEAN fCountStacksAsOne);

// get total number of items in sector
int32_t GetTotalNumberOfItems(void);

// handle flash of inventory items
void HandleFlashForHighLightedItem(void);

// the list for the inventory
extern WORLDITEM *pInventoryPoolList;

// autoplace down object
BOOLEAN AutoPlaceObjectInInventoryStash(struct OBJECTTYPE *pItemPtr);

// the current inventory item
extern int32_t iCurrentlyHighLightedItem;
extern BOOLEAN fFlashHighLightInventoryItemOnradarMap;
extern int16_t sObjectSourceGridNo;
extern WORLDITEM *pInventoryPoolList;
extern int32_t iCurrentInventoryPoolPage;
extern BOOLEAN fMapInventoryItemCompatable[];

BOOLEAN IsMapScreenWorldItemInvisibleInMapInventory(WORLDITEM *pWorldItem);
BOOLEAN IsMapScreenWorldItemVisibleInMapInventory(WORLDITEM *pWorldItem);

#endif
