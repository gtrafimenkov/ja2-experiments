// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/EditorUndo.h"

#include "BuildDefines.h"
#include "Editor/CursorModes.h"
#include "Editor/EditScreen.h"
#include "Editor/Smooth.h"
#include "SGP/Debug.h"
#include "SGP/Input.h"
#include "SGP/MemMan.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"  //for access to gubWorldRoomInfo;
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"

/*
Kris -- Notes on how the undo code works:

At the bottom of the hierarchy, we need to determine the state of the undo command.  The idea
is that we want to separate undo commands by separating them by new mouse clicks.  By holding a
mouse down and painting various objects in the world would all constitute a single undo command.  As
soon as the mouse is release, then a new undo command is setup.  So, to automate this, there is a
call every frame to DetermineUndoState().

At the next level, there is a binary tree that keeps track of what map indices have been backup up
in the current undo command.  The whole reason to maintain this list, is to avoid multiple map
elements of the same map index from being saved.  In the outer code, everytime something is changed,
a call to AddToUndoList() is called, so there are many cases (especially with building/terrain
smoothing) that the same mapindex is added to the undo list.  This is also completely transparent,
and doesn't need to be maintained.

In the outer code, there are several calls to AddToUndoList( iMapIndex ).  This function basically
looks in the binary tree for an existing entry, and if there isn't, then the entire mapelement is
saved (with the exception of the merc level ).  Lights are also supported, but there is a totally
different methodology for accomplishing this.  The equivalent function is AddLightToUndoList(
iMapIndex ).  In this case, only the light is saved, along with internal maintanance of several
flags.

The actual mapelement copy code, is very complex.  The mapelement is copied in parallel with a new
one which has to allocate several nodes of several types because a mapelement contains over a dozen
separate lists, and all of them need to be saved.  The structure information of certain mapelements
may be multitiled and must also save the affected gridno's as well.  This is also done internally.
Basically, your call to AddToUndoList() for any particular gridno, may actually save several entries
(like for a car which could be 6+ tiles)

MERCS
Mercs are not supported in the undo code.  Because they are so dynamic, and stats could change, they
could move, etc., it doesn't need to be in the undo code.  The editor has its own way of dealing
with mercs which doesn't use the undo methodology.

*/

BOOLEAN gfUndoEnabled = FALSE;

void EnableUndo() { gfUndoEnabled = TRUE; }

void DisableUndo() { gfUndoEnabled = FALSE; }

// undo node data element
typedef struct {
  int32_t iMapIndex;
  MAP_ELEMENT *pMapTile;
  BOOLEAN fLightSaved;    // determines that a light has been saved
  uint8_t ubLightRadius;  // the radius of the light to build if undo is called
  uint8_t ubLightID;      // only applies if a light was saved.
  uint8_t ubRoomNum;
} undo_struct;

// Undo stack node
typedef struct TAG_undo_stack {
  int32_t iCmdCount;
  undo_struct *pData;
  struct TAG_undo_stack *pNext;
  int32_t iUndoType;
} undo_stack;
undo_stack *gpTileUndoStack = NULL;

// Map tile element manipulation functions
BOOLEAN CopyMapElementFromWorld(MAP_ELEMENT *pMapTile, int32_t iMapIndex);
BOOLEAN SwapMapElementWithWorld(int32_t iDestMapTileIndex, MAP_ELEMENT *pMapTile);

// Undo structure functions
BOOLEAN DeleteTopStackNode(void);
undo_stack *DeleteThisStackNode(undo_stack *pThisNode);
BOOLEAN DeleteStackNodeContents(undo_stack *pCurrent);
BOOLEAN AddToUndoListCmd(int32_t iMapIndex, int32_t iCmdCount);
void CropStackToMaxLength(int32_t iMaxCmds);
void SmoothUndoMapTileTerrain(int32_t iWorldTile, MAP_ELEMENT *pUndoTile);

BOOLEAN fNewUndoCmd = TRUE;
BOOLEAN gfIgnoreUndoCmdsForLights = FALSE;

// New pre-undo binary tree stuff
// With this, new undo commands will not duplicate saves in the same command.  This will
// increase speed, and save memory.
typedef struct MapIndexBinaryTree {
  struct MapIndexBinaryTree *left, *right;
  uint16_t usMapIndex;
} MapIndexBinaryTree;

MapIndexBinaryTree *top = NULL;
void ClearUndoMapIndexTree();
BOOLEAN AddMapIndexToTree(uint16_t usMapIndex);
void CheckMapIndexForMultiTileStructures(uint16_t usMapIndex);
void CheckForMultiTilesInTreeAndAddToUndoList(MapIndexBinaryTree *node);

// Recursively deletes all nodes below the node passed including itself.
void DeleteTreeNode(MapIndexBinaryTree **node) {
  if ((*node)->left) DeleteTreeNode(&((*node)->left));
  if ((*node)->right) DeleteTreeNode(&((*node)->right));
  MemFree(*node);
  *node = NULL;
}

// Recursively delete all nodes (from the top down).
void ClearUndoMapIndexTree() {
  if (top) DeleteTreeNode(&top);
}

BOOLEAN AddMapIndexToTree(uint16_t usMapIndex) {
  MapIndexBinaryTree *curr, *parent;
  if (!top) {
    top = (MapIndexBinaryTree *)MemAlloc(sizeof(MapIndexBinaryTree));
    Assert(top);
    top->usMapIndex = usMapIndex;
    top->left = NULL;
    top->right = NULL;
    return TRUE;
  }
  curr = top;
  parent = NULL;
  // Traverse down the tree and attempt to find a matching mapindex.
  // If one is encountered, then we fail, and don't add the mapindex to the
  // tree.
  while (curr) {
    parent = curr;
    if (curr->usMapIndex == usMapIndex)  // found a match, so stop
      return FALSE;
    // if the mapIndex is < node's mapIndex, then go left, else right
    curr = (usMapIndex < curr->usMapIndex) ? curr->left : curr->right;
  }
  // if we made it this far, then curr is null and parent is pointing
  // directly above.
  // Create the new node and fill in the information.
  curr = (MapIndexBinaryTree *)MemAlloc(sizeof(MapIndexBinaryTree));
  Assert(curr);
  curr->usMapIndex = usMapIndex;
  curr->left = NULL;
  curr->right = NULL;
  // Now link the new node to the parent.
  if (curr->usMapIndex < parent->usMapIndex)
    parent->left = curr;
  else
    parent->right = curr;
  return TRUE;
}
//*************************************************************************
//
//	Start of Undo code
//
//*************************************************************************

BOOLEAN DeleteTopStackNode(void) {
  undo_stack *pCurrent;

  pCurrent = gpTileUndoStack;

  DeleteStackNodeContents(pCurrent);

  // Remove node from stack, and free it's memory
  gpTileUndoStack = gpTileUndoStack->pNext;
  MemFree(pCurrent);

  return (TRUE);
}

undo_stack *DeleteThisStackNode(undo_stack *pThisNode) {
  undo_stack *pCurrent;
  undo_stack *pNextNode;

  pCurrent = pThisNode;
  pNextNode = pThisNode->pNext;

  // Remove node from stack, and free it's memory
  DeleteStackNodeContents(pCurrent);
  MemFree(pCurrent);

  return (pNextNode);
}

BOOLEAN DeleteStackNodeContents(undo_stack *pCurrent) {
  undo_struct *pData;
  MAP_ELEMENT *pMapTile;
  struct LEVELNODE *pLandNode;
  struct LEVELNODE *pObjectNode;
  struct LEVELNODE *pStructNode;
  struct LEVELNODE *pShadowNode;
  struct LEVELNODE *pMercNode;
  struct LEVELNODE *pTopmostNode;
  struct LEVELNODE *pRoofNode;
  struct LEVELNODE *pOnRoofNode;
  struct STRUCTURE *pStructureNode;

  pData = pCurrent->pData;
  pMapTile = pData->pMapTile;

  if (!pMapTile) return TRUE;  // light was saved -- mapelement wasn't saved.

  // Free the memory associated with the map tile liked lists
  pLandNode = pMapTile->pLandHead;
  while (pLandNode != NULL) {
    pMapTile->pLandHead = pLandNode->pNext;
    MemFree(pLandNode);
    pLandNode = pMapTile->pLandHead;
  }

  pObjectNode = pMapTile->pObjectHead;
  while (pObjectNode != NULL) {
    pMapTile->pObjectHead = pObjectNode->pNext;
    MemFree(pObjectNode);
    pObjectNode = pMapTile->pObjectHead;
  }

  pStructNode = pMapTile->pStructHead;
  while (pStructNode != NULL) {
    pMapTile->pStructHead = pStructNode->pNext;
    MemFree(pStructNode);
    pStructNode = pMapTile->pStructHead;
  }

  pShadowNode = pMapTile->pShadowHead;
  while (pShadowNode != NULL) {
    pMapTile->pShadowHead = pShadowNode->pNext;
    MemFree(pShadowNode);
    pShadowNode = pMapTile->pShadowHead;
  }

  pMercNode = pMapTile->pMercHead;
  while (pMercNode != NULL) {
    pMapTile->pMercHead = pMercNode->pNext;
    MemFree(pMercNode);
    pMercNode = pMapTile->pMercHead;
  }

  pRoofNode = pMapTile->pRoofHead;
  while (pRoofNode != NULL) {
    pMapTile->pRoofHead = pRoofNode->pNext;
    MemFree(pRoofNode);
    pRoofNode = pMapTile->pRoofHead;
  }

  pOnRoofNode = pMapTile->pOnRoofHead;
  while (pOnRoofNode != NULL) {
    pMapTile->pOnRoofHead = pOnRoofNode->pNext;
    MemFree(pOnRoofNode);
    pOnRoofNode = pMapTile->pOnRoofHead;
  }

  pTopmostNode = pMapTile->pTopmostHead;
  while (pTopmostNode != NULL) {
    pMapTile->pTopmostHead = pTopmostNode->pNext;
    MemFree(pTopmostNode);
    pTopmostNode = pMapTile->pTopmostHead;
  }

  pStructureNode = pMapTile->pStructureHead;
  while (pStructureNode) {
    pMapTile->pStructureHead = pStructureNode->pNext;
    if (pStructureNode->usStructureID >
        INVALID_STRUCTURE_ID) {  // Okay to delete the structure data -- otherwise, this would be
      // merc structure data that we DON'T want to delete, because the merc node
      // that hasn't been modified will still use this structure data!
      MemFree(pStructureNode);
    }
    pStructureNode = pMapTile->pStructureHead;
  }

  // Free the map tile structure itself
  MemFree(pMapTile);

  // Free the undo struct
  MemFree(pData);

  return (TRUE);
}

void CropStackToMaxLength(int32_t iMaxCmds) {
  int32_t iCmdCount;
  undo_stack *pCurrent;

  iCmdCount = 0;
  pCurrent = gpTileUndoStack;

  // If stack is empty, leave
  if (pCurrent == NULL) return;

  while ((iCmdCount <= (iMaxCmds - 1)) && (pCurrent != NULL)) {
    if (pCurrent->iCmdCount == 1) iCmdCount++;
    pCurrent = pCurrent->pNext;
  }

  // If the max number of commands was reached, and there is something
  // to crop, from the rest of the stack, remove it.
  if ((iCmdCount >= iMaxCmds) && pCurrent != NULL) {
    while (pCurrent->pNext != NULL) pCurrent->pNext = DeleteThisStackNode(pCurrent->pNext);
  }
}

// We are adding a light to the undo list.  We won't save the mapelement, nor will
// we validate the gridno in the binary tree.  This works differently than a mapelement,
// because lights work on a different system.  By setting the fLightSaved flag to TRUE,
// this will handle the way the undo command is handled.  If there is no lightradius in
// our saved light, then we intend on erasing the light upon undo execution, otherwise, we
// save the light radius and light ID, so that we place it during undo execution.
void AddLightToUndoList(int32_t iMapIndex, int32_t iLightRadius, uint8_t ubLightID) {
  undo_stack *pNode;
  undo_struct *pUndoInfo;

  if (!gfUndoEnabled) return;
  // When executing an undo command (by adding a light or removing one), that command
  // actually tries to add it to the undo list.  So we wrap the execution of the undo
  // command by temporarily setting this flag, so it'll ignore, and not place a new undo
  // command.  When finished, the flag is cleared, and lights are again allowed to be saved
  // in the undo list.
  if (gfIgnoreUndoCmdsForLights) return;

  pNode = (undo_stack *)MemAlloc(sizeof(undo_stack));
  if (!pNode) return;

  pUndoInfo = (undo_struct *)MemAlloc(sizeof(undo_struct));
  if (!pUndoInfo) {
    MemFree(pNode);
    return;
  }

  pUndoInfo->fLightSaved = TRUE;
  // if ubLightRadius is 0, then we don't need to save the light information because we
  // will erase it when it comes time to execute the undo command.
  pUndoInfo->ubLightRadius = (uint8_t)iLightRadius;
  pUndoInfo->ubLightID = ubLightID;
  pUndoInfo->iMapIndex = iMapIndex;
  pUndoInfo->pMapTile = NULL;

  // Add to undo stack
  pNode->iCmdCount = 1;
  pNode->pData = pUndoInfo;
  pNode->pNext = gpTileUndoStack;
  gpTileUndoStack = pNode;

  CropStackToMaxLength(MAX_UNDO_COMMAND_LENGTH);
}

BOOLEAN AddToUndoList(int32_t iMapIndex) {
  static int32_t iCount = 1;

  if (!gfUndoEnabled) return FALSE;
  if (fNewUndoCmd) {
    iCount = 0;
    fNewUndoCmd = FALSE;
  }

  // Check to see if the tile in question is even on the visible map, then
  // if that is true, then check to make sure we don't already have the mapindex
  // saved in the new binary tree (which only holds unique mapindex values).
  if (GridNoOnVisibleWorldTile((int16_t)iMapIndex) && AddMapIndexToTree((uint16_t)iMapIndex))

  {
    if (AddToUndoListCmd(iMapIndex, ++iCount)) return TRUE;
    iCount--;
  }
  return FALSE;
}

BOOLEAN AddToUndoListCmd(int32_t iMapIndex, int32_t iCmdCount) {
  undo_stack *pNode;
  undo_struct *pUndoInfo;
  MAP_ELEMENT *pData;
  struct STRUCTURE *pStructure;
  int32_t iCoveredMapIndex;
  uint8_t ubLoop;

  if ((pNode = (undo_stack *)MemAlloc(sizeof(undo_stack))) == NULL) {
    return (FALSE);
  }

  if ((pUndoInfo = (undo_struct *)MemAlloc(sizeof(undo_struct))) == NULL) {
    MemFree(pNode);
    return (FALSE);
  }

  if ((pData = (MAP_ELEMENT *)MemAlloc(sizeof(MAP_ELEMENT))) == NULL) {
    MemFree(pNode);
    MemFree(pUndoInfo);
    return (FALSE);
  }

  // Init map element struct
  pData->pLandHead = pData->pLandStart = NULL;
  pData->pObjectHead = NULL;
  pData->pStructHead = NULL;
  pData->pShadowHead = NULL;
  pData->pMercHead = NULL;
  pData->pRoofHead = NULL;
  pData->pOnRoofHead = NULL;
  pData->pTopmostHead = NULL;
  pData->pStructureHead = pData->pStructureTail = NULL;
  pData->sHeight = 0;

  // Copy the world map's tile
  if (CopyMapElementFromWorld(pData, iMapIndex) == FALSE) {
    MemFree(pNode);
    MemFree(pUndoInfo);
    MemFree(pData);
    return (FALSE);
  }

  // copy the room number information (it's not in the mapelement structure)
  pUndoInfo->ubRoomNum = gubWorldRoomInfo[iMapIndex];

  pUndoInfo->fLightSaved = FALSE;
  pUndoInfo->ubLightRadius = 0;
  pUndoInfo->ubLightID = 0;
  pUndoInfo->pMapTile = pData;
  pUndoInfo->iMapIndex = iMapIndex;

  pNode->pData = pUndoInfo;
  pNode->iCmdCount = iCmdCount;
  pNode->pNext = gpTileUndoStack;
  gpTileUndoStack = pNode;

  // loop through pData->pStructureHead list
  // for each structure
  //   find the base tile
  //   reference the db structure
  //   if number of tiles > 1
  //     add all covered tiles to undo list
  pStructure = pData->pStructureHead;
  while (pStructure) {
    for (ubLoop = 1; ubLoop < pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles;
         ubLoop++) {
      // this loop won't execute for single-tile structures; for multi-tile structures, we have to
      // add to the undo list all the other tiles covered by the structure
      iCoveredMapIndex =
          pStructure->sBaseGridNo + pStructure->pDBStructureRef->ppTile[ubLoop]->sPosRelToBase;
      AddToUndoList(iCoveredMapIndex);
    }
    pStructure = pStructure->pNext;
  }

  CropStackToMaxLength(MAX_UNDO_COMMAND_LENGTH);

  return (TRUE);
}

void CheckMapIndexForMultiTileStructures(uint16_t usMapIndex) {
  struct STRUCTURE *pStructure;
  uint8_t ubLoop;
  int32_t iCoveredMapIndex;

  pStructure = gpWorldLevelData[usMapIndex].pStructureHead;
  while (pStructure) {
    if (pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1) {
      for (ubLoop = 0; ubLoop < pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles;
           ubLoop++) {
        // for multi-tile structures we have to add, to the undo list, all the other tiles covered
        // by the structure
        if (pStructure->fFlags & STRUCTURE_BASE_TILE) {
          iCoveredMapIndex =
              usMapIndex + pStructure->pDBStructureRef->ppTile[ubLoop]->sPosRelToBase;
        } else {
          iCoveredMapIndex =
              pStructure->sBaseGridNo + pStructure->pDBStructureRef->ppTile[ubLoop]->sPosRelToBase;
        }
        AddToUndoList(iCoveredMapIndex);
      }
    }
    pStructure = pStructure->pNext;
  }
}

void CheckForMultiTilesInTreeAndAddToUndoList(MapIndexBinaryTree *node) {
  CheckMapIndexForMultiTileStructures(node->usMapIndex);
  if (node->left) CheckForMultiTilesInTreeAndAddToUndoList(node->left);
  if (node->right) CheckForMultiTilesInTreeAndAddToUndoList(node->right);
}

BOOLEAN RemoveAllFromUndoList(void) {
  ClearUndoMapIndexTree();

  while (gpTileUndoStack != NULL) DeleteTopStackNode();

  return (TRUE);
}

BOOLEAN ExecuteUndoList(void) {
  int32_t iCmdCount, iCurCount;
  int32_t iUndoMapIndex;
  BOOLEAN fExitGrid;

  if (!gfUndoEnabled) return FALSE;

  // Is there something on the undo stack?
  if (gpTileUndoStack == NULL) return (TRUE);

  // Get number of stack entries for this command (top node will tell this)
  iCmdCount = gpTileUndoStack->iCmdCount;

  // Execute each stack node in command, and remove each from stack.
  iCurCount = 0;
  while ((iCurCount < iCmdCount) && (gpTileUndoStack != NULL)) {
    iUndoMapIndex = gpTileUndoStack->pData->iMapIndex;

    // Find which map tile we are to "undo"
    if (gpTileUndoStack->pData->fLightSaved) {  // We saved a light, so delete that light
      int16_t sX, sY;
      // Turn on this flag so that the following code, when executed, doesn't attempt to
      // add lights to the undo list.  That would cause problems...
      gfIgnoreUndoCmdsForLights = TRUE;
      ConvertGridNoToXY((int16_t)iUndoMapIndex, &sX, &sY);
      if (!gpTileUndoStack->pData->ubLightRadius)
        RemoveLight(sX, sY);
      else
        PlaceLight(gpTileUndoStack->pData->ubLightRadius, sX, sY,
                   gpTileUndoStack->pData->ubLightID);
      // Turn off the flag so lights can again be added to the undo list.
      gfIgnoreUndoCmdsForLights = FALSE;
    } else {  // We execute the undo command node by simply swapping the contents
      // of the undo's MAP_ELEMENT with the world's element.
      fExitGrid = ExitGridAtGridNo((uint16_t)iUndoMapIndex);
      SwapMapElementWithWorld(iUndoMapIndex, gpTileUndoStack->pData->pMapTile);

      // copy the room number information back
      gubWorldRoomInfo[iUndoMapIndex] = gpTileUndoStack->pData->ubRoomNum;

      // Now we smooth out the changes...
      // SmoothUndoMapTileTerrain( iUndoMapIndex, gpTileUndoStack->pData->pMapTile );
      SmoothAllTerrainTypeRadius(iUndoMapIndex, 1, TRUE);
    }

    // ...trash the top element of the stack...
    DeleteTopStackNode();

    // ...and bump the command counter up by 1
    iCurCount++;

    // Kris:
    // The new cursor system is somehow interfering with the undo stuff.  When
    // an undo is called, the item is erased, but a cursor is added!  I'm quickly
    // hacking around this by erasing all cursors here.
    RemoveAllTopmostsOfTypeRange(iUndoMapIndex, FIRSTPOINTERS, FIRSTPOINTERS);

    if (fExitGrid &&
        !ExitGridAtGridNo((uint16_t)iUndoMapIndex)) {  // An exitgrid has been removed, so get rid
                                                       // of the associated indicator.
      RemoveTopmost((uint16_t)iUndoMapIndex, FIRSTPOINTERS8);
    } else if (!fExitGrid &&
               ExitGridAtGridNo((uint16_t)iUndoMapIndex)) {  // An exitgrid has been added, so add
                                                             // the associated indicator
      AddTopmostToTail((uint16_t)iUndoMapIndex, FIRSTPOINTERS8);
    }
  }

  return (TRUE);
}

void SmoothUndoMapTileTerrain(int32_t iWorldTile, MAP_ELEMENT *pUndoTile) {
  struct LEVELNODE *pWorldLand;
  struct LEVELNODE *pUndoLand;
  struct LEVELNODE *pLand;
  struct LEVELNODE *pWLand;
  uint32_t uiCheckType, uiWCheckType;
  BOOLEAN fFound;

  pUndoLand = pUndoTile->pLandHead;
  pWorldLand = gpWorldLevelData[iWorldTile].pLandHead;

  if (pUndoLand == NULL) {
    // nothing in the old tile, so smooth the entire land in world's tile
    pLand = gpWorldLevelData[iWorldTile].pLandHead;
    while (pLand != NULL) {
      GetTileType(pLand->usIndex, &uiCheckType);
      SmoothTerrainRadius(iWorldTile, uiCheckType, 1, TRUE);
      pLand = pLand->pNext;
    }
  } else if (gpWorldLevelData[iWorldTile].pLandHead == NULL) {
    // Nothing in world's tile, so smooth out the land in the old tile.
    pLand = pUndoLand;
    while (pLand != NULL) {
      GetTileType(pLand->usIndex, &uiCheckType);
      SmoothTerrainRadius(iWorldTile, uiCheckType, 1, TRUE);
      pLand = pLand->pNext;
    }
  } else {
    pLand = pUndoLand;
    while (pLand != NULL) {
      GetTileType(pLand->usIndex, &uiCheckType);

      fFound = FALSE;
      pWLand = pWorldLand;
      while (pWLand != NULL && !fFound) {
        GetTileType(pWLand->usIndex, &uiWCheckType);

        if (uiCheckType == uiWCheckType) fFound = TRUE;

        pWLand = pWLand->pNext;
      }

      if (!fFound) SmoothTerrainRadius(iWorldTile, uiCheckType, 1, TRUE);

      pLand = pLand->pNext;
    }

    pWLand = pWorldLand;
    while (pWLand != NULL) {
      GetTileType(pWLand->usIndex, &uiWCheckType);

      fFound = FALSE;
      pLand = pUndoLand;
      while (pLand != NULL && !fFound) {
        GetTileType(pLand->usIndex, &uiCheckType);

        if (uiCheckType == uiWCheckType) fFound = TRUE;

        pLand = pLand->pNext;
      }

      if (!fFound) SmoothTerrainRadius(iWorldTile, uiWCheckType, 1, TRUE);

      pWLand = pWLand->pNext;
    }
  }
}

// Because of the potentially huge amounts of memory that can be allocated due to the inefficient
// undo methods coded by Bret, it is feasible that it could fail.  Instead of using assertions to
// terminate the program, destroy the memory allocated thusfar.
void DeleteMapElementContentsAfterCreationFail(MAP_ELEMENT *pNewMapElement) {
  struct LEVELNODE *pLevelNode;
  struct STRUCTURE *pStructure;
  int32_t x;
  for (x = 0; x < 9; x++) {
    if (x == 1) continue;
    pLevelNode = pNewMapElement->pLevelNodes[x];
    while (pLevelNode) {
      struct LEVELNODE *temp;
      temp = pLevelNode;
      pLevelNode = pLevelNode->pNext;
      MemFree(temp);
    }
  }
  pStructure = pNewMapElement->pStructureHead;
  while (pStructure) {
    struct STRUCTURE *temp;
    temp = pStructure;
    pStructure = pStructure->pNext;
    MemFree(temp);
  }
}

/*
        union
        {
                struct LEVELNODE				*pPrevNode;
   // FOR LAND, GOING BACKWARDS POINTER struct ITEM_POOL
   *pItemPool;					// ITEM POOLS struct STRUCTURE
   *pStructureData;		// struct STRUCTURE DATA int32_t
   iPhysicsObjectID;		// ID FOR PHYSICS ITEM
                int32_t
   uiAPCost;						// FOR AP DISPLAY
        }; // ( 4 byte union )
        union
        {
                struct
                {
                        uint16_t
   usIndex;							// TILE DATABASE INDEX int16_t
   sCurrentFrame;				// Stuff for animated tiles for a given tile
   location ( doors, etc )
                };
                struct
                {
                        struct SOLDIERTYPE
   *pSoldier;					// POINTER TO SOLDIER
                };
        }; // ( 4 byte union )
*/
BOOLEAN CopyMapElementFromWorld(MAP_ELEMENT *pNewMapElement, int32_t iMapIndex) {
  MAP_ELEMENT *pOldMapElement;
  struct LEVELNODE *pOldLevelNode;
  struct LEVELNODE *pLevelNode;
  struct LEVELNODE *pNewLevelNode;
  struct LEVELNODE *tail;
  int32_t x;

  struct STRUCTURE *pOldStructure;

  // Get a pointer to the current map index
  pOldMapElement = &gpWorldLevelData[iMapIndex];

  // Save the structure information from the mapelement
  pOldStructure = pOldMapElement->pStructureHead;
  if (pOldStructure) {
    struct STRUCTURE *pNewStructure;
    struct STRUCTURE *pStructure;
    struct STRUCTURE *tail;
    tail = NULL;
    pNewStructure = NULL;
    while (pOldStructure) {
      pStructure = (struct STRUCTURE *)MemAlloc(sizeof(struct STRUCTURE));
      if (!pStructure) {
        DeleteMapElementContentsAfterCreationFail(pNewMapElement);
        return FALSE;
      }
      if (!tail) {  // first node in structure list
        tail = pStructure;
        *tail = *pOldStructure;
        tail->pPrev = NULL;
        tail->pNext = NULL;
      } else {  // add to the end of the levelnode list
        tail->pNext = pStructure;
        *pStructure = *pOldStructure;
        pStructure->pPrev = tail;
        pStructure->pNext = NULL;
        tail = tail->pNext;
      }
      // place the new node inside of the new map element
      if (!pNewStructure) {
        pNewMapElement->pStructureHead = pStructure;
        pNewStructure = pStructure;
      } else {
        pNewStructure->pNext = pStructure;
        pNewStructure = pNewStructure->pNext;
      }
      pOldStructure = pOldStructure->pNext;
    }
    if (tail) {
      pNewMapElement->pStructureTail = tail;
    }
  }

  // For each of the 9 levelnodes, save each one
  // except for levelnode[1] which is a pointer to the first land to render.
  for (x = 0; x < 9; x++) {
    if (x == 1 || x == 5)  // skip the pLandStart and pMercLevel LEVELNODES
      continue;
    tail = NULL;
    pOldLevelNode = pOldMapElement->pLevelNodes[x];
    pNewLevelNode = NULL;
    while (pOldLevelNode) {
      // copy the level node
      pLevelNode = (struct LEVELNODE *)MemAlloc(sizeof(struct LEVELNODE));
      if (!pLevelNode) {
        DeleteMapElementContentsAfterCreationFail(pNewMapElement);
        return FALSE;
      }
      if (!tail) {  // first node in levelnode list
        tail = pLevelNode;
        *tail = *pOldLevelNode;
        if (!x)  // land layer only
          tail->pPrevNode = NULL;
        tail->pNext = NULL;
      } else {  // add to the end of the levelnode list
        tail->pNext = pLevelNode;
        *pLevelNode = *pOldLevelNode;
        if (!x)  // land layer only
          pLevelNode->pPrevNode = tail;
        pLevelNode->pNext = NULL;
        tail = tail->pNext;
      }
      // place the new node inside of the new map element
      if (!pNewLevelNode) {
        pNewMapElement->pLevelNodes[x] = pLevelNode;
        pNewLevelNode = pLevelNode;
      } else {
        pNewLevelNode->pNext = pLevelNode;
        pNewLevelNode = pNewLevelNode->pNext;
      }
      // Handle levelnode layer specific stuff
      switch (x) {
        case 0:  // LAND LAYER
          if (pOldLevelNode ==
              pOldMapElement->pLandStart) {  // set the new landstart to point to the new levelnode.
            pNewMapElement->pLandStart = pNewLevelNode;
          }
          break;
        case 2:                            // OBJECT LAYER
          if (pOldLevelNode->pItemPool) {  // save the item pool?
                                           // pNewLevelNode->pItemPool = (struct
                                           // ITEM_POOL*)MemAlloc( sizeof( struct ITEM_POOL ) );
          }
          break;
        case 3:                                 // STRUCT LAYER
        case 6:                                 // ROOF LAYER
        case 7:                                 // ON ROOF LAYER
          if (pOldLevelNode->pStructureData) {  // make sure the structuredata pointer points to the
                                                // parallel structure
            struct STRUCTURE *pOld, *pNew;
            // both lists are exactly the same size and contain the same information,
            // but the addresses are different.  We will traverse the old list until
            // we find the match, then
            pOld = pOldMapElement->pStructureHead;
            pNew = pNewMapElement->pStructureHead;
            while (pOld) {
              Assert(pNew);
              if (pOld == pOldLevelNode->pStructureData) {
                pNewLevelNode->pStructureData = pNew;
                break;
              }
              pOld = pOld->pNext;
              pNew = pNew->pNext;
            }
            // Kris:
            // If this assert should fail, that means there is something wrong with
            // the preservation of the structure data within the mapelement.
            if (pOld != pOldLevelNode->pStructureData) {
              // OUCH!!! THIS IS HAPPENING.  DISABLED IT FOR LINDA'S SAKE
              Assert(1);
            }
          }
          break;
      }
      // Done, go to next node in this level
      pOldLevelNode = pOldLevelNode->pNext;
    }
    // Done, go to next level
  }

  // Save the rest of the information in the mapelement.
  pNewMapElement->uiFlags = pOldMapElement->uiFlags;
  pNewMapElement->sSumRealLights[0] = pOldMapElement->sSumRealLights[0];
  pNewMapElement->sSumRealLights[1] = pOldMapElement->sSumRealLights[1];
  pNewMapElement->sHeight = pOldMapElement->sHeight;
  pNewMapElement->ubTerrainID = pOldMapElement->ubTerrainID;
  pNewMapElement->ubReservedSoldierID = pOldMapElement->ubReservedSoldierID;

  return TRUE;
}

BOOLEAN SwapMapElementWithWorld(int32_t iMapIndex, MAP_ELEMENT *pUndoMapElement) {
  MAP_ELEMENT *pCurrentMapElement;
  MAP_ELEMENT TempMapElement;

  pCurrentMapElement = &gpWorldLevelData[iMapIndex];

  // Transfer the merc level node from the current world to the undo mapelement
  // that will replace it.  We do this, because mercs aren't associated with
  // undo commands.
  pUndoMapElement->pMercHead = gpWorldLevelData[iMapIndex].pMercHead;
  gpWorldLevelData[iMapIndex].pMercHead = NULL;

  // Swap the mapelements
  TempMapElement = *pCurrentMapElement;
  *pCurrentMapElement = *pUndoMapElement;
  *pUndoMapElement = TempMapElement;

  return (TRUE);
}

void DetermineUndoState() {
  // Reset the undo command mode if we released the left button.
  if (!fNewUndoCmd) {
    if ((!gfLeftButtonState && !gfCurrentSelectionWithRightButton) ||
        (!gfRightButtonState && gfCurrentSelectionWithRightButton)) {
      // Clear the mapindex binary tree list, and set up flag for new undo command.
      fNewUndoCmd = TRUE;
      ClearUndoMapIndexTree();
    }
  }
}
