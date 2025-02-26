// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/ExitGrids.h"

#include <stdio.h>

#include "Editor/EditorUndo.h"
#include "Editor/Smooth.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Types.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/TileDat.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"

BOOLEAN gfLoadingExitGrids = FALSE;

// used by editor.
EXITGRID gExitGrid = {0, 1, 1, 0};

BOOLEAN gfOverrideInsertionWithExitGrid = FALSE;

int32_t ConvertExitGridToINT32(EXITGRID *pExitGrid) {
  int32_t iExitGridInfo;
  iExitGridInfo = (pExitGrid->ubGotoSectorX - 1) << 28;
  iExitGridInfo += (pExitGrid->ubGotoSectorY - 1) << 24;
  iExitGridInfo += pExitGrid->ubGotoSectorZ << 20;
  iExitGridInfo += pExitGrid->usGridNo & 0x0000ffff;
  return iExitGridInfo;
}

void ConvertINT32ToExitGrid(int32_t iExitGridInfo, EXITGRID *pExitGrid) {
  // convert the int into 4 unsigned bytes.
  pExitGrid->ubGotoSectorX = (uint8_t)(((iExitGridInfo & 0xf0000000) >> 28) + 1);
  pExitGrid->ubGotoSectorY = (uint8_t)(((iExitGridInfo & 0x0f000000) >> 24) + 1);
  pExitGrid->ubGotoSectorZ = (uint8_t)((iExitGridInfo & 0x00f00000) >> 20);
  pExitGrid->usGridNo = (uint16_t)(iExitGridInfo & 0x0000ffff);
}

BOOLEAN GetExitGrid(uint16_t usMapIndex, EXITGRID *pExitGrid) {
  struct LEVELNODE *pShadow;
  pShadow = gpWorldLevelData[usMapIndex].pShadowHead;
  // Search through object layer for an exitgrid
  while (pShadow) {
    if (pShadow->uiFlags & LEVELNODE_EXITGRID) {
      ConvertINT32ToExitGrid(pShadow->iExitGridInfo, pExitGrid);
      return TRUE;
    }
    pShadow = pShadow->pNext;
  }
  pExitGrid->ubGotoSectorX = 0;
  pExitGrid->ubGotoSectorY = 0;
  pExitGrid->ubGotoSectorZ = 0;
  pExitGrid->usGridNo = 0;
  return FALSE;
}

BOOLEAN ExitGridAtGridNo(uint16_t usMapIndex) {
  struct LEVELNODE *pShadow;
  pShadow = gpWorldLevelData[usMapIndex].pShadowHead;
  // Search through object layer for an exitgrid
  while (pShadow) {
    if (pShadow->uiFlags & LEVELNODE_EXITGRID) {
      return TRUE;
    }
    pShadow = pShadow->pNext;
  }
  return FALSE;
}

BOOLEAN GetExitGridLevelNode(uint16_t usMapIndex, struct LEVELNODE **ppLevelNode) {
  struct LEVELNODE *pShadow;
  pShadow = gpWorldLevelData[usMapIndex].pShadowHead;
  // Search through object layer for an exitgrid
  while (pShadow) {
    if (pShadow->uiFlags & LEVELNODE_EXITGRID) {
      *ppLevelNode = pShadow;
      return TRUE;
    }
    pShadow = pShadow->pNext;
  }
  return FALSE;
}

void AddExitGridToWorld(int32_t iMapIndex, EXITGRID *pExitGrid) {
  struct LEVELNODE *pShadow;
  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // Search through object layer for an exitgrid
  while (pShadow) {
    if (pShadow->uiFlags & LEVELNODE_EXITGRID) {  // we have found an existing exitgrid in this
                                                  // node, so replace it with the new information.
      pShadow->iExitGridInfo = ConvertExitGridToINT32(pExitGrid);
      // SmoothExitGridRadius( (uint16_t)iMapIndex, 0 );
      return;
    }
    pShadow = pShadow->pNext;
  }

  // Add levelnode
  AddShadowToHead(iMapIndex, MOCKFLOOR1);
  // Get new node
  pShadow = gpWorldLevelData[iMapIndex].pShadowHead;

  // fill in the information for the new exitgrid levelnode.
  pShadow->iExitGridInfo = ConvertExitGridToINT32(pExitGrid);
  pShadow->uiFlags |= (LEVELNODE_EXITGRID | LEVELNODE_HIDDEN);

  // Add the exit grid to the sector, only if we call ApplyMapChangesToMapTempFile() first.
  if (!gfEditMode && !gfLoadingExitGrids) {
    AddExitGridToMapTempFile((uint16_t)iMapIndex, pExitGrid, (uint8_t)gWorldSectorX,
                             (uint8_t)gWorldSectorY, gbWorldSectorZ);
  }
}

void RemoveExitGridFromWorld(int32_t iMapIndex) {
  uint16_t usDummy;
  if (TypeExistsInShadowLayer(iMapIndex, MOCKFLOOR, &usDummy)) {
    RemoveAllShadowsOfTypeRange(iMapIndex, MOCKFLOOR, MOCKFLOOR);
  }
}

void SaveExitGrids(HWFILE fp, uint16_t usNumExitGrids) {
  EXITGRID exitGrid;
  uint16_t usNumSaved = 0;
  uint16_t x;
  uint32_t uiBytesWritten;
  FileMan_Write(fp, &usNumExitGrids, 2, &uiBytesWritten);
  for (x = 0; x < WORLD_MAX; x++) {
    if (GetExitGrid(x, &exitGrid)) {
      FileMan_Write(fp, &x, 2, &uiBytesWritten);
      FileMan_Write(fp, &exitGrid, 5, &uiBytesWritten);
      usNumSaved++;
    }
  }
  // If these numbers aren't equal, something is wrong!
  Assert(usNumExitGrids == usNumSaved);
}

void LoadExitGrids(int8_t **hBuffer) {
  EXITGRID exitGrid;
  uint16_t x;
  uint16_t usNumSaved;
  uint16_t usMapIndex;
  gfLoadingExitGrids = TRUE;
  LOADDATA(&usNumSaved, *hBuffer, 2);
  // FileMan_Read( hfile, &usNumSaved, 2, NULL);
  for (x = 0; x < usNumSaved; x++) {
    LOADDATA(&usMapIndex, *hBuffer, 2);
    // FileMan_Read( hfile, &usMapIndex, 2, NULL);
    LOADDATA(&exitGrid, *hBuffer, 5);
    // FileMan_Read( hfile, &exitGrid, 5, NULL);
    AddExitGridToWorld(usMapIndex, &exitGrid);
  }
  gfLoadingExitGrids = FALSE;
}

void AttemptToChangeFloorLevel(int8_t bRelativeZLevel) {
  uint8_t ubLookForLevel = 0;
  uint16_t i;
  if (bRelativeZLevel != 1 && bRelativeZLevel != -1) return;
  // Check if on ground level -- if so, can't go up!
  if (bRelativeZLevel == -1 && !gbWorldSectorZ) {
    ScreenMsg(FONT_DKYELLOW, MSG_INTERFACE, pMessageStrings[MSG_CANT_GO_UP], ubLookForLevel);
    return;
  }
  // Check if on bottom level -- if so, can't go down!
  if (bRelativeZLevel == 1 && gbWorldSectorZ == 3) {
    ScreenMsg(FONT_DKYELLOW, MSG_INTERFACE, pMessageStrings[MSG_CANT_GO_DOWN], ubLookForLevel);
    return;
  }
  ubLookForLevel = (uint8_t)(gbWorldSectorZ + bRelativeZLevel);
  for (i = 0; i < WORLD_MAX; i++) {
    if (GetExitGrid(i, &gExitGrid)) {
      if (gExitGrid.ubGotoSectorZ ==
          ubLookForLevel) {  // found an exit grid leading to the goal sector!
        gfOverrideInsertionWithExitGrid = TRUE;
        // change all current mercs in the loaded sector, and move them
        // to the new sector.
        MoveAllGroupsInCurrentSectorToSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                             ubLookForLevel);
        if (ubLookForLevel)
          ScreenMsg(FONT_YELLOW, MSG_INTERFACE, pMessageStrings[MSG_ENTERING_LEVEL],
                    ubLookForLevel);
        else
          ScreenMsg(FONT_YELLOW, MSG_INTERFACE, pMessageStrings[MSG_LEAVING_BASEMENT]);

        SetCurrentWorldSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, ubLookForLevel);
        gfOverrideInsertionWithExitGrid = FALSE;
      }
    }
  }
}

uint16_t FindGridNoFromSweetSpotCloseToExitGrid(struct SOLDIERTYPE *pSoldier, int16_t sSweetGridNo,
                                                int8_t ubRadius, uint8_t *pubDirection) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = 0;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  struct SOLDIERTYPE soldier;
  uint8_t ubSaveNPCAPBudget;
  uint8_t ubSaveNPCDistLimit;
  EXITGRID ExitGrid;
  uint8_t ubGotoSectorX, ubGotoSectorY, ubGotoSectorZ;

  // Turn off at end of function...
  gfPlotPathToExitGrid = TRUE;

  // Save AI pathing vars.  changing the distlimit restricts how
  // far away the pathing will consider.
  ubSaveNPCAPBudget = gubNPCAPBudget;
  ubSaveNPCDistLimit = gubNPCDistLimit;
  gubNPCAPBudget = 0;
  gubNPCDistLimit = ubRadius;

  // create dummy soldier, and use the pathing to determine which nearby slots are
  // reachable.
  memset(&soldier, 0, sizeof(struct SOLDIERTYPE));
  soldier.bLevel = 0;
  soldier.bTeam = 1;
  soldier.sGridNo = pSoldier->sGridNo;

  // OK, Get an exit grid ( if possible )
  if (!GetExitGrid(sSweetGridNo, &ExitGrid)) {
    return (NOWHERE);
  }

  // Copy our dest values.....
  ubGotoSectorX = ExitGrid.ubGotoSectorX;
  ubGotoSectorY = ExitGrid.ubGotoSectorY;
  ubGotoSectorZ = ExitGrid.ubGotoSectorZ;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  // in the square region.
  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = pSoldier->sGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX) {
        gpWorldLevelData[sGridNo].uiFlags &= (~MAPELEMENT_REACHABLE);
      }
    }
  }

  // Now, find out which of these gridnos are reachable
  //(use the fake soldier and the pathing settings)
  FindBestPath(&soldier, NOWHERE, 0, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE);

  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((pSoldier->sGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = pSoldier->sGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS) &&
          gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE) {
        // Go on sweet stop
        // ATE: Added this check because for all intensive purposes, cavewalls will be not an OKDEST
        // but we want thenm too...
        if (NewOKDestination(pSoldier, sGridNo, TRUE, pSoldier->bLevel)) {
          if (GetExitGrid(sGridNo, &ExitGrid)) {
            // Is it the same exitgrid?
            if (ExitGrid.ubGotoSectorX == ubGotoSectorX &&
                ExitGrid.ubGotoSectorY == ubGotoSectorY &&
                ExitGrid.ubGotoSectorZ == ubGotoSectorZ) {
              uiRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo);

              if (uiRange < uiLowestRange) {
                sLowestGridNo = sGridNo;
                uiLowestRange = uiRange;
                fFound = TRUE;
              }
            }
          }
        }
      }
    }
  }
  gubNPCAPBudget = ubSaveNPCAPBudget;
  gubNPCDistLimit = ubSaveNPCDistLimit;

  gfPlotPathToExitGrid = FALSE;

  if (fFound) {
    // Set direction to center of map!
    *pubDirection = (uint8_t)GetDirectionToGridNoFromGridNo(
        sLowestGridNo, (((WORLD_ROWS / 2) * WORLD_COLS) + (WORLD_COLS / 2)));
    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}

uint16_t FindClosestExitGrid(struct SOLDIERTYPE *pSoldier, int16_t sSrcGridNo, int8_t ubRadius) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t sGridNo;
  int32_t uiRange, uiLowestRange = 999999;
  int16_t sLowestGridNo = 0;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  EXITGRID ExitGrid;

  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  // clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
  uiLowestRange = 999999;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sSrcGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      sGridNo = sSrcGridNo + (WORLD_COLS * cnt1) + cnt2;
      if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
          sGridNo < (leftmost + WORLD_COLS)) {
        if (GetExitGrid(sGridNo, &ExitGrid)) {
          uiRange = GetRangeInCellCoordsFromGridNoDiff(sSrcGridNo, sGridNo);

          if (uiRange < uiLowestRange) {
            sLowestGridNo = sGridNo;
            uiLowestRange = uiRange;
            fFound = TRUE;
          }
        }
      }
    }
  }

  if (fFound) {
    return (sLowestGridNo);
  } else {
    return (NOWHERE);
  }
}
