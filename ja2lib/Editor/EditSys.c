// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/EditSys.h"

#include <stdlib.h>

#include "BuildDefines.h"
#include "Editor/CursorModes.h"
#include "Editor/EditScreen.h"
#include "Editor/EditorBuildings.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorMercs.h"
#include "Editor/EditorTerrain.h"
#include "Editor/EditorUndo.h"
#include "Editor/NewSmooth.h"
#include "Editor/RoadSmoothing.h"
#include "Editor/SelectWin.h"
#include "Editor/Smooth.h"
#include "Editor/SmoothingUtils.h"
#include "SGP/HImage.h"
#include "SGP/MemMan.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/Structure.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"

BOOLEAN PasteHigherTextureFromRadius(int32_t iMapIndex, uint32_t uiNewType, uint8_t ubRadius);
BOOLEAN PasteExistingTexture(uint32_t iMapIndex, uint16_t usIndex);
BOOLEAN PasteExistingTextureFromRadius(int32_t iMapIndex, uint16_t usIndex, uint8_t ubRadius);
BOOLEAN SetLowerLandIndexWithRadius(int32_t iMapIndex, uint32_t uiNewType, uint8_t ubRadius,
                                    BOOLEAN fReplace);

void PasteTextureEx(int16_t sGridNo, uint16_t usType);
void PasteTextureFromRadiusEx(int16_t sGridNo, uint16_t usType, uint8_t ubRadius);

BOOLEAN gfWarning = FALSE;

BOOLEAN gfDoFill = FALSE;
uint16_t CurrentPaste = NO_TILE;
uint16_t gDebrisPaste = NO_TILE;
uint16_t gChangeElevation = FALSE;
uint16_t CurrentStruct = NO_TILE;
uint32_t gDoBanks = NO_BANKS;
uint32_t gDoCliffs = NO_CLIFFS;

//---------------------------------------------------------------------------------------------------------------
//	QuickEraseMapTile
//
//	Performs ersing operation when the DEL key is hit in the editor
//
void QuickEraseMapTile(uint32_t iMapIndex) {
  if (iMapIndex >= 0x8000) return;
  AddToUndoList(iMapIndex);
  DeleteStuffFromMapTile(iMapIndex);
  MarkWorldDirty();
}

//---------------------------------------------------------------------------------------------------------------
//	DeleteStuffFromMapTile
//
//	Common delete function for both QuickEraseMapTile and EraseMapTile
//
void DeleteStuffFromMapTile(uint32_t iMapIndex) {
  // uint16_t		usUseIndex;
  // uint16_t		usType;
  // uint32_t		uiCheckType;
  // uint16_t		usDummy;

  // GetTileType( gpWorldLevelData[ iMapIndex ].pLandHead->usIndex, &uiCheckType );
  // RemoveLand( iMapIndex, gpWorldLevelData[ iMapIndex ].pLandHead->usIndex );
  // SmoothTerrainRadius( iMapIndex, uiCheckType, 1, TRUE );

  RemoveExitGridFromWorld(iMapIndex);
  RemoveAllStructsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  RemoveAllObjectsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  RemoveAllLandsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  RemoveAllRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  RemoveAllOnRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  RemoveAllTopmostsOfTypeRange(iMapIndex, FIRSTTEXTURE, WIREFRAMES);
  PasteRoomNumber(iMapIndex, 0);
}

//---------------------------------------------------------------------------------------------------------------
//	EraseMapTile
//
//	Generic tile erasing function. Erases things from the world depending on the current drawing
// mode
//
void EraseMapTile(uint32_t iMapIndex) {
  int32_t iEraseMode;
  uint32_t uiCheckType;
  if (iMapIndex >= 0x8000) return;

  // Figure out what it is we are trying to erase
  iEraseMode = iDrawMode - DRAW_MODE_ERASE;

  switch (iEraseMode) {
    case DRAW_MODE_NORTHPOINT:
    case DRAW_MODE_WESTPOINT:
    case DRAW_MODE_EASTPOINT:
    case DRAW_MODE_SOUTHPOINT:
    case DRAW_MODE_CENTERPOINT:
    case DRAW_MODE_ISOLATEDPOINT:
      SpecifyEntryPoint(iMapIndex);
      break;
    case DRAW_MODE_EXITGRID:
      AddToUndoList(iMapIndex);
      RemoveExitGridFromWorld(iMapIndex);
      RemoveTopmost((uint16_t)iMapIndex, FIRSTPOINTERS8);
      break;
    case DRAW_MODE_GROUND:
      // Is there ground on this tile? if not, get out o here
      if (gpWorldLevelData[iMapIndex].pLandHead == NULL) break;

      // is there only 1 ground tile here? if so, get out o here
      if (gpWorldLevelData[iMapIndex].pLandHead->pNext == NULL) break;
      AddToUndoList(iMapIndex);
      GetTileType(gpWorldLevelData[iMapIndex].pLandHead->usIndex, &uiCheckType);
      RemoveLand(iMapIndex, gpWorldLevelData[iMapIndex].pLandHead->usIndex);
      SmoothTerrainRadius(iMapIndex, uiCheckType, 1, TRUE);
      break;
    case DRAW_MODE_OSTRUCTS:
    case DRAW_MODE_OSTRUCTS1:
    case DRAW_MODE_OSTRUCTS2:
      AddToUndoList(iMapIndex);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTOSTRUCT, LASTOSTRUCT);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTVEHICLE, SECONDVEHICLE);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTDEBRISSTRUCT, SECONDDEBRISSTRUCT);
      RemoveAllStructsOfTypeRange(iMapIndex, NINTHOSTRUCT, TENTHOSTRUCT);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTLARGEEXPDEBRIS, SECONDLARGEEXPDEBRIS);
      RemoveAllObjectsOfTypeRange(iMapIndex, DEBRIS2MISC, DEBRIS2MISC);
      RemoveAllObjectsOfTypeRange(iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS);
      break;
    case DRAW_MODE_DEBRIS:
      AddToUndoList(iMapIndex);
      RemoveAllObjectsOfTypeRange(iMapIndex, DEBRISROCKS, LASTDEBRIS);
      RemoveAllObjectsOfTypeRange(iMapIndex, DEBRIS2MISC, DEBRIS2MISC);
      RemoveAllObjectsOfTypeRange(iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS);
      break;
    case DRAW_MODE_BANKS:
      AddToUndoList(iMapIndex);
      RemoveAllObjectsOfTypeRange(iMapIndex, FIRSTROAD, LASTROAD);
      // Note, for this routine, cliffs are considered a subset of banks
      RemoveAllStructsOfTypeRange(iMapIndex, ANIOSTRUCT, ANIOSTRUCT);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTCLIFF, LASTBANKS);
      RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTCLIFFSHADOW, LASTCLIFFSHADOW);
      RemoveAllObjectsOfTypeRange(iMapIndex, FIRSTCLIFFHANG, LASTCLIFFHANG);
      RemoveAllStructsOfTypeRange(iMapIndex, FENCESTRUCT, FENCESTRUCT);
      RemoveAllShadowsOfTypeRange(iMapIndex, FENCESHADOW, FENCESHADOW);
      break;
    case DRAW_MODE_FLOORS:
      AddToUndoList(iMapIndex);
      RemoveAllLandsOfTypeRange(iMapIndex, FIRSTFLOOR, LASTFLOOR);
      break;
    case DRAW_MODE_ROOFS:
    case DRAW_MODE_NEWROOF:
      AddToUndoList(iMapIndex);
      RemoveAllRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, LASTITEM);
      RemoveAllOnRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, LASTITEM);
      break;
    case DRAW_MODE_WALLS:
    case DRAW_MODE_DOORS:
    case DRAW_MODE_WINDOWS:
    case DRAW_MODE_BROKEN_WALLS:
      AddToUndoList(iMapIndex);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
      RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTDOOR, LASTDOOR);
      RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTDOORSHADOW, LASTDOORSHADOW);
      break;
    case DRAW_MODE_DECOR:
    case DRAW_MODE_DECALS:
    case DRAW_MODE_ROOM:
    case DRAW_MODE_TOILET:
      AddToUndoList(iMapIndex);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALLDECAL, LASTWALLDECAL);
      RemoveAllStructsOfTypeRange(iMapIndex, FIFTHWALLDECAL, EIGTHWALLDECAL);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTDECORATIONS, LASTDECORATIONS);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTISTRUCT, LASTISTRUCT);
      RemoveAllStructsOfTypeRange(iMapIndex, FIFTHISTRUCT, EIGHTISTRUCT);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTSWITCHES, FIRSTSWITCHES);
      break;
    case DRAW_MODE_CAVES:
      AddToUndoList(iMapIndex);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
      break;
    case DRAW_MODE_ROOMNUM:
      PasteRoomNumber(iMapIndex, 0);
      break;
    case DRAW_MODE_ROADS:
      RemoveAllObjectsOfTypeRange(iMapIndex, ROADPIECES, ROADPIECES);
      break;
    default:
      // DeleteStuffFromMapTile( iMapIndex );
      break;
  }
}

//---------------------------------------------------------------------------------------------------------------
//	PasteDebris
//
//	Place some "debris" on the map at the current mouse coordinates. This function is called
// repeatedly if 	the current brush size is larger than 1 tile.
//
void PasteDebris(uint32_t iMapIndex) {
  uint16_t usUseIndex;
  uint16_t usUseObjIndex;
  int32_t iRandSelIndex;

  // Get selection list for debris
  pSelList = SelDebris;
  pNumSelList = &iNumDebrisSelected;

  if (iMapIndex < 0x8000) {
    AddToUndoList(iMapIndex);

    // Remove any debris that is currently at this map location
    if (gpWorldLevelData[iMapIndex].pObjectHead != NULL) {
      RemoveAllObjectsOfTypeRange(iMapIndex, ANOTHERDEBRIS, FIRSTPOINTERS - 1);
    }

    // Get a random debris from current selection
    iRandSelIndex = GetRandomSelection();
    if (iRandSelIndex != -1) {
      // Add debris to the world
      usUseIndex = pSelList[iRandSelIndex].usIndex;
      usUseObjIndex = (uint16_t)pSelList[iRandSelIndex].uiObject;

      AddObjectToTail(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
    }
  }
}

void PasteSingleWall(uint32_t iMapIndex) {
  pSelList = SelSingleWall;
  pNumSelList = &iNumWallsSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleDoor(uint32_t iMapIndex) {
  pSelList = SelSingleDoor;
  pNumSelList = &iNumDoorsSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleWindow(uint32_t iMapIndex) {
  pSelList = SelSingleWindow;
  pNumSelList = &iNumWindowsSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleRoof(uint32_t iMapIndex) {
  pSelList = SelSingleRoof;
  pNumSelList = &iNumRoofsSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteRoomNumber(uint32_t iMapIndex, uint8_t ubRoomNumber) {
  if (gubWorldRoomInfo[iMapIndex] != ubRoomNumber) {
    AddToUndoList(iMapIndex);
    gubWorldRoomInfo[iMapIndex] = ubRoomNumber;
  }
}

void PasteSingleBrokenWall(uint32_t iMapIndex) {
  uint16_t usIndex, usObjIndex, usTileIndex, usWallOrientation;

  pSelList = SelSingleBrokenWall;
  pNumSelList = &iNumBrokenWallsSelected;

  usIndex = pSelList[iCurBank].usIndex;
  usObjIndex = (uint16_t)pSelList[iCurBank].uiObject;
  usTileIndex = GetTileIndexFromTypeSubIndex(usObjIndex, usIndex, &usTileIndex);
  GetWallOrientation(usTileIndex, &usWallOrientation);
  if (usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == INSIDE_TOP_RIGHT)
    EraseHorizontalWall(iMapIndex);
  else
    EraseVerticalWall(iMapIndex);

  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleDecoration(uint32_t iMapIndex) {
  pSelList = SelSingleDecor;
  pNumSelList = &iNumDecorSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleDecal(uint32_t iMapIndex) {
  pSelList = SelSingleDecal;
  pNumSelList = &iNumDecalsSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleFloor(uint32_t iMapIndex) {
  pSelList = SelSingleFloor;
  pNumSelList = &iNumFloorsSelected;
  PasteSingleWallCommon(iMapIndex);
}

void PasteSingleToilet(uint32_t iMapIndex) {
  pSelList = SelSingleToilet;
  pNumSelList = &iNumToiletsSelected;
  PasteSingleWallCommon(iMapIndex);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteSingleWallCommon
//
//	Common paste routine for PasteSingleWall, PasteSingleDoor, PasteSingleDecoration, and
//	PasteSingleDecor (above).
//
void PasteSingleWallCommon(uint32_t iMapIndex) {
  uint16_t usUseIndex;
  uint16_t usUseObjIndex;
  uint16_t usTempIndex;

  if (iMapIndex < 0x8000) {
    AddToUndoList(iMapIndex);

    usUseIndex = pSelList[iCurBank].usIndex;
    usUseObjIndex = (uint16_t)pSelList[iCurBank].uiObject;

    // TEMP STUFF FOR ONROOF THINGS!
    if ((usUseObjIndex >= FIRSTONROOF) && (usUseObjIndex <= SECONDONROOF)) {
      // Add to onroof section!
      AddOnRoofToTail(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));

      if (gTileDatabase[(uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex)].sBuddyNum !=
          -1) {
        AddOnRoofToTail(
            iMapIndex,
            gTileDatabase[(uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex)].sBuddyNum);
      }
      return;
    }

    // Make sure A-frames are on roof level!
    if ((usUseIndex >= WALL_AFRAME_START && usUseIndex <= WALL_AFRAME_END)) {
      AddRoofToTail(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
      return;
    }

    if ((usUseObjIndex >= FIRSTDOOR) && (usUseObjIndex <= LASTDOOR)) {
      // PLace shadow for doors
      if (!gfBasement)
        AddExclusiveShadow(
            iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex - FIRSTDOOR + FIRSTDOORSHADOW] +
                                  usUseIndex));
    }

    // Is it a wall?
    if (((usUseObjIndex >= FIRSTWALL) && (usUseObjIndex <= LASTWALL))) {
      // ATE		If it is a wall shadow, place differenty!
      if (usUseIndex == 29 || usUseIndex == 30) {
        if (!gfBasement)
          AddExclusiveShadow(iMapIndex,
                             (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
      } else {
        // Slap down wall/window/door/decoration (no smoothing)
        AddWallToStructLayer(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex),
                             TRUE);
      }
    }
    // Is it a door/window/decoration?
    else if (((usUseObjIndex >= FIRSTDOOR) && (usUseObjIndex <= LASTDOOR)) ||
             ((usUseObjIndex >= FIRSTDECORATIONS) && (usUseObjIndex <= LASTDECORATIONS))) {
      // Slap down wall/window/door/decoration (no smoothing)
      AddWallToStructLayer(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex),
                           TRUE);
    } else if (((usUseObjIndex >= FIRSTROOF) && (usUseObjIndex <= LASTROOF)) ||
               ((usUseObjIndex >= FIRSTSLANTROOF) && (usUseObjIndex <= LASTSLANTROOF))) {
      // Put a roof on this tile (even if nothing else is here)
      RemoveAllRoofsOfTypeRange(iMapIndex, FIRSTROOF, LASTROOF);
      AddRoofToTail(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
    } else if ((usUseObjIndex >= FIRSTFLOOR) && (usUseObjIndex <= LASTFLOOR)) {
      // Drop a floor on this tile
      if (TypeExistsInLandLayer(iMapIndex, usUseObjIndex, &usTempIndex))
        RemoveLand(iMapIndex, usTempIndex);

      AddLandToHead(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
    } else if ((usUseObjIndex >= FIRSTWALLDECAL && usUseObjIndex <= LASTWALLDECAL) ||
               (usUseObjIndex >= FIFTHWALLDECAL && usUseObjIndex <= EIGTHWALLDECAL)) {
      // Plop a decal here
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALLDECAL, LASTWALLDECAL);
      RemoveAllStructsOfTypeRange(iMapIndex, FIFTHWALLDECAL, EIGTHWALLDECAL);

      AddStructToTail(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
    } else if ((usUseObjIndex >= FIRSTISTRUCT && usUseObjIndex <= LASTISTRUCT) ||
               (usUseObjIndex >= FIFTHISTRUCT && usUseObjIndex <= EIGHTISTRUCT)) {
      AddStructToHead(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
    } else if (usUseObjIndex == FIRSTSWITCHES) {
      AddStructToTail(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
    }
  }
}

//---------------------------------------------------------------------------------------------------------------
//	GetRandomIndexByRange
//
//	Returns a randomly picked object index given the current selection list, and the type or
// types of objects we want 	from that list. If no such objects are in the list, we return 0xffff
//(-1).
uint16_t GetRandomIndexByRange(uint16_t usRangeStart, uint16_t usRangeEnd) {
  uint16_t usPickList[50];
  uint16_t usNumInPickList;
  uint16_t usWhich;
  uint16_t usObject;
  // Get a list of valid object to select from
  usNumInPickList = 0;
  for (usWhich = 0; usWhich < *pNumSelList; usWhich++) {
    usObject = (uint16_t)pSelList[usWhich].uiObject;
    if ((usObject >= usRangeStart) && (usObject <= usRangeEnd)) {
      usPickList[usNumInPickList] = usObject;
      usNumInPickList++;
    }
  }
  return (usNumInPickList) ? usPickList[rand() % usNumInPickList] : 0xffff;
}

uint16_t GetRandomTypeByRange(uint16_t usRangeStart, uint16_t usRangeEnd) {
  uint16_t usPickList[50];
  uint16_t usNumInPickList;
  uint16_t i;
  uint16_t usObject;
  uint32_t uiType;
  // Get a list of valid object to select from
  usNumInPickList = 0;
  for (i = 0; i < *pNumSelList; i++) {
    usObject = (uint16_t)pSelList[i].uiObject;
    if ((usObject >= usRangeStart) && (usObject <= usRangeEnd)) {
      GetTileType(usObject, &uiType);
      usPickList[usNumInPickList] = (uint16_t)uiType;
      usNumInPickList++;
    }
  }
  return (usNumInPickList) ? usPickList[rand() % usNumInPickList] : 0xffff;
}

//---------------------------------------------------------------------------------------------------------------
//	PasteStructure			(See also PasteStructure1, PasteStructure2, and
// PasteStructureCommon)
//
//	Puts a structure (trees, trucks, etc.) into the world
//
void PasteStructure(uint32_t iMapIndex) {
  pSelList = SelOStructs;
  pNumSelList = &iNumOStructsSelected;

  PasteStructureCommon(iMapIndex);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteStructure1			(See also PasteStructure, PasteStructure2, and
// PasteStructureCommon)
//
//	Puts a structure (trees, trucks, etc.) into the world
//
void PasteStructure1(uint32_t iMapIndex) {
  pSelList = SelOStructs1;
  pNumSelList = &iNumOStructs1Selected;

  PasteStructureCommon(iMapIndex);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteStructure2			(See also PasteStructure, PasteStructure1, and
// PasteStructureCommon)
//
//	Puts a structure (trees, trucks, etc.) into the world
//
void PasteStructure2(uint32_t iMapIndex) {
  pSelList = SelOStructs2;
  pNumSelList = &iNumOStructs2Selected;

  PasteStructureCommon(iMapIndex);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteStructureCommon
//
//	This is the main (common) structure pasting function. The above three wrappers are only
// required because they 	each use different selection lists. Other than that, they are
// COMPLETELY identical.
//
void PasteStructureCommon(uint32_t iMapIndex) {
  uint32_t fHeadType;
  uint16_t usUseIndex;
  uint16_t usUseObjIndex;
  int32_t iRandSelIndex;
  BOOLEAN fOkayToAdd;

  if (iMapIndex < 0x8000) {
    if (/*fDoPaste &&*/ iMapIndex < 0x8000) {
      iRandSelIndex = GetRandomSelection();
      if (iRandSelIndex == -1) {
        return;
      }

      AddToUndoList(iMapIndex);

      usUseIndex = pSelList[iRandSelIndex].usIndex;
      usUseObjIndex = (uint16_t)pSelList[iRandSelIndex].uiObject;

      // Check with Structure Database (aka ODB) if we can put the object here!
      fOkayToAdd = OkayToAddStructureToWorld(
          (int16_t)iMapIndex, 0,
          gTileDatabase[(gTileTypeStartIndex[usUseObjIndex] + usUseIndex)].pDBStructureRef,
          INVALID_STRUCTURE_ID);
      if (fOkayToAdd ||
          (gTileDatabase[(gTileTypeStartIndex[usUseObjIndex] + usUseIndex)].pDBStructureRef ==
           NULL)) {
        // Actual structure info is added by the functions below
        AddStructToHead(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
        // For now, adjust to shadows by a hard-coded amount,

        // Add mask if in long grass
        GetLandHeadType(iMapIndex, &fHeadType);
      }
    }
  } else if (CurrentStruct == ERASE_TILE && iMapIndex < 0x8000) {
    RemoveAllStructsOfTypeRange(iMapIndex, FIRSTOSTRUCT, LASTOSTRUCT);
    RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTSHADOW, LASTSHADOW);
  }
}

//---------------------------------------------------------------------------------------------------------------
//	PasteBanks
//
//	Places a river bank or cliff into the world
//
void PasteBanks(uint32_t iMapIndex, uint16_t usStructIndex, BOOLEAN fReplace) {
  BOOLEAN fDoPaste = FALSE;
  uint16_t usUseIndex;
  uint16_t usUseObjIndex;

  pSelList = SelBanks;
  pNumSelList = &iNumBanksSelected;

  usUseIndex = pSelList[iCurBank].usIndex;
  usUseObjIndex = (uint16_t)pSelList[iCurBank].uiObject;

  if (iMapIndex < 0x8000) {
    fDoPaste = TRUE;

    if (gpWorldLevelData[iMapIndex].pStructHead != NULL) {
      // CHECK IF THE SAME TILE IS HERE
      if (gpWorldLevelData[iMapIndex].pStructHead->usIndex ==
          (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex)) {
        fDoPaste = FALSE;
      }
    } else {
      // Nothing is here, paste
      fDoPaste = TRUE;
    }

    if (fDoPaste) {
      AddToUndoList(iMapIndex);
      {
        if (usUseObjIndex == FIRSTROAD) {
          AddObjectToHead(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
        } else {
          AddStructToHead(iMapIndex, (uint16_t)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
          // Add shadows
          if (!gfBasement && usUseObjIndex == FIRSTCLIFF) {
            // AddShadowToHead( iMapIndex, (uint16_t)( gTileTypeStartIndex[ usUseObjIndex -
            // FIRSTCLIFF
            // + FIRSTCLIFFSHADOW ] + usUseIndex ) );
            AddObjectToHead(
                iMapIndex,
                (uint16_t)(gTileTypeStartIndex[usUseObjIndex - FIRSTCLIFF + FIRSTCLIFFHANG] +
                           usUseIndex));
          }
        }
      }
    }
  }
}

void PasteRoads(uint32_t iMapIndex) {
  uint16_t usUseIndex;

  pSelList = SelRoads;
  pNumSelList = &iNumRoadsSelected;

  usUseIndex = pSelList[iCurBank].usIndex;

  PlaceRoadMacroAtGridNo(iMapIndex, usUseIndex);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteTexture
//
//	Puts a ground texture in the world. Ground textures are then "smoothed" in order to blend
// the edges with one 	another. The current drawing brush also affects this function.
//
void PasteTexture(uint32_t iMapIndex) {
  ChooseWeightedTerrainTile();  // Kris
  PasteTextureCommon(iMapIndex);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteTextureCommon
//
//	The PasteTexture function calls this one to actually put a ground tile down. If the brush
// size is larger than 	one tile, then the above function will call this one and indicate that they
// should all be placed into the undo 	stack as the same undo command.
//
void PasteTextureCommon(uint32_t iMapIndex) {
  uint8_t ubLastHighLevel;
  uint16_t usTileIndex;
  // uint16_t					Dummy;

  if (CurrentPaste != NO_TILE && iMapIndex < 0x8000) {
    // Set undo, then set new
    AddToUndoList(iMapIndex);

    if (CurrentPaste == DEEPWATERTEXTURE) {
      // IF WE ARE PASTING DEEP WATER AND WE ARE NOT OVER WATER, IGNORE!
      if (TypeExistsInLandLayer(iMapIndex, REGWATERTEXTURE, &usTileIndex)) {
        if (!gTileDatabase[usTileIndex].ubFullTile) {
          return;
        }
      } else {
        return;
      }
    }

    // Don't draw over floors
    if (TypeRangeExistsInLandLayer(iMapIndex, FIRSTFLOOR, FOURTHFLOOR, &usTileIndex)) {
      return;
    }

    // Compare heights and do appropriate action
    if (AnyHeigherLand(iMapIndex, CurrentPaste, &ubLastHighLevel)) {
      // Here we do the following:
      // - Remove old type from layer
      // - Smooth World with old type
      // - Add a 3 by 3 square of new type at head
      // - Smooth World with new type
      PasteHigherTexture(iMapIndex, CurrentPaste);

    } else {
      PasteTextureEx((int16_t)iMapIndex, CurrentPaste);
      SmoothTerrainRadius(iMapIndex, CurrentPaste, 1, TRUE);
    }
  }
}

//---------------------------------------------------------------------------------------------------------------
//	PasteHigherTexture
//
//	Some ground textures should be placed "above" others. That is, grass needs to be placed
//"above" sand etc. 	This function performs the appropriate actions.
//
void PasteHigherTexture(uint32_t iMapIndex, uint32_t fNewType) {
  uint16_t NewTile;
  uint8_t ubLastHighLevel;
  uint32_t *puiDeletedTypes = NULL;
  uint8_t ubNumTypes;
  uint8_t cnt;

  // Here we do the following:
  // - Remove old type from layer
  // - Smooth World with old type
  // - Add a 3 by 3 square of new type at head
  // - Smooth World with new type

  // if ( iMapIndex < 0x8000 && TypeRangeExistsInLandLayer( iMapIndex, FIRSTFLOOR, LASTFLOOR,
  // &NewTile ) ) ATE: DONOT DO THIS!!!!!!! - I know what was intended - not to draw over floors -
  // this
  // I don't know is the right way to do it!
  // return;

  if (iMapIndex < 0x8000 && AnyHeigherLand(iMapIndex, fNewType, &ubLastHighLevel)) {
    AddToUndoList(iMapIndex);

    // - For all heigher level, remove
    RemoveHigherLandLevels(iMapIndex, fNewType, &puiDeletedTypes, &ubNumTypes);

    // Set with a radius of 1 and smooth according to height difference
    SetLowerLandIndexWithRadius(iMapIndex, fNewType, 1, TRUE);

    // Smooth all deleted levels
    for (cnt = 0; cnt < ubNumTypes; cnt++) {
      SmoothTerrainRadius(iMapIndex, puiDeletedTypes[cnt], 1, TRUE);
    }

    MemFree(puiDeletedTypes);

  } else if (iMapIndex < 0x8000) {
    AddToUndoList(iMapIndex);

    GetTileIndexFromTypeSubIndex(fNewType, REQUIRES_SMOOTHING_TILE, &NewTile);
    SetLandIndex(iMapIndex, NewTile, fNewType, FALSE);

    // Smooth item then adding here
    SmoothTerrain(iMapIndex, fNewType, &NewTile, FALSE);

    if (NewTile != NO_TILE) {
      // Change tile
      SetLandIndex(iMapIndex, NewTile, fNewType, FALSE);
    }
  }
}

//---------------------------------------------------------------------------------------------------------------
//	PasteHigherTextureFromRadius
//
//	Like above function except it performs it's operation on a redial area.
//
BOOLEAN PasteHigherTextureFromRadius(int32_t iMapIndex, uint32_t uiNewType, uint8_t ubRadius) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int32_t iNewIndex;
  int32_t iXPos, iYPos;

  // Determine start and end indicies and num rows
  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  iXPos = (iMapIndex % WORLD_COLS);
  iYPos = (iMapIndex - iXPos) / WORLD_COLS;

  if ((iXPos + (int32_t)sLeft) < 0) sLeft = (int16_t)(-iXPos);

  if ((iXPos + (int32_t)sRight) >= WORLD_COLS) sRight = (int16_t)(WORLD_COLS - iXPos - 1);

  if ((iYPos + (int32_t)sTop) >= WORLD_ROWS) sTop = (int16_t)(WORLD_ROWS - iYPos - 1);

  if ((iYPos + (int32_t)sBottom) < 0) sBottom = (int16_t)(-iYPos);

  if (iMapIndex >= 0x8000) return (FALSE);

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      iNewIndex = iMapIndex + (WORLD_COLS * cnt1) + cnt2;

      PasteHigherTexture(iNewIndex, uiNewType);
    }
  }

  return (TRUE);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteExistingTexture
//
BOOLEAN PasteExistingTexture(uint32_t iMapIndex, uint16_t usIndex) {
  uint32_t uiNewType;
  uint16_t usNewIndex;
  // uint16_t					Dummy;

  // If here, we want to make, esentially, what is a type in
  // a level other than TOP-MOST the TOP-MOST level.
  // We should:
  // - remove what was top-most
  // - re-adjust the world to reflect missing top-most peice

  if (iMapIndex >= 0x8000) return (FALSE);

  // if ( TypeRangeExistsInLandLayer( iMapIndex, FIRSTFLOOR, LASTFLOOR, &Dummy ) )
  //	return( FALSE );

  // Get top tile index
  // Remove all land peices except
  GetTileType(usIndex, &uiNewType);

  DeleteAllLandLayers(iMapIndex);

  // ADD BASE LAND AT LEAST!
  usNewIndex = (uint16_t)(rand() % 10);

  // Adjust for type
  usNewIndex += gTileTypeStartIndex[gCurrentBackground];

  // Set land index
  AddLandToHead(iMapIndex, usNewIndex);

  SetLandIndex(iMapIndex, usIndex, uiNewType, FALSE);

  // ATE: Set this land peice to require smoothing again!
  SmoothAllTerrainTypeRadius(iMapIndex, 2, TRUE);

  return (TRUE);
}

//---------------------------------------------------------------------------------------------------------------
//	PasteExistingTextureFromRadius
//
//	As above, but on a radial area
//
BOOLEAN PasteExistingTextureFromRadius(int32_t iMapIndex, uint16_t usIndex, uint8_t ubRadius) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int32_t iNewIndex;
  int32_t leftmost;

  // Determine start end end indicies and num rows
  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  if (iMapIndex >= 0x8000) return (FALSE);

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((iMapIndex + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      iNewIndex = iMapIndex + (WORLD_COLS * cnt1) + cnt2;

      if (iNewIndex >= 0 && iNewIndex < WORLD_MAX && iNewIndex >= leftmost &&
          iNewIndex < (leftmost + WORLD_COLS)) {
        AddToUndoList(iMapIndex);
        PasteExistingTexture(iNewIndex, usIndex);
      }
    }
  }

  return (TRUE);
}

//---------------------------------------------------------------------------------------------------------------
//	SetLowerLandIndexWithRadius
//
//	Puts a land index "under" an existing ground texture. Affects a radial area.
//
BOOLEAN SetLowerLandIndexWithRadius(int32_t iMapIndex, uint32_t uiNewType, uint8_t ubRadius,
                                    BOOLEAN fReplace) {
  uint16_t usTempIndex;
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int32_t iNewIndex;
  BOOLEAN fDoPaste = FALSE;
  int32_t leftmost;
  uint8_t ubLastHighLevel;
  uint32_t *puiSmoothTiles = NULL;
  int16_t sNumSmoothTiles = 0;
  uint16_t usTemp;
  uint16_t NewTile;  //,Dummy;

  // Determine start end end indicies and num rows
  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  if (iMapIndex >= 0x8000) return (FALSE);

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((iMapIndex + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      iNewIndex = iMapIndex + (WORLD_COLS * cnt1) + cnt2;

      if (iNewIndex >= 0 && iNewIndex < WORLD_MAX && iNewIndex >= leftmost &&
          iNewIndex < (leftmost + WORLD_COLS)) {
        if (fReplace) {
          fDoPaste = TRUE;
        } else {
          if (TypeExistsInLandLayer(iNewIndex, uiNewType, &usTempIndex)) {
            fDoPaste = TRUE;
          }
        }

        // if ( fDoPaste && !TypeRangeExistsInLandLayer( iMapIndex, FIRSTFLOOR, LASTFLOOR, &Dummy )
        // )
        if (fDoPaste) {
          if (iMapIndex == iNewIndex) {
            AddToUndoList(iMapIndex);

            // Force middle one to NOT smooth, and set to random 'full' tile
            usTemp = (rand() % 10) + 1;
            GetTileIndexFromTypeSubIndex(uiNewType, usTemp, &NewTile);
            SetLandIndex(iNewIndex, NewTile, uiNewType, FALSE);
          } else if (AnyHeigherLand(iNewIndex, uiNewType, &ubLastHighLevel)) {
            AddToUndoList(iMapIndex);

            // Force middle one to NOT smooth, and set to random 'full' tile
            usTemp = (rand() % 10) + 1;
            GetTileIndexFromTypeSubIndex(uiNewType, usTemp, &NewTile);
            SetLandIndex(iNewIndex, NewTile, uiNewType, FALSE);
          } else {
            AddToUndoList(iMapIndex);

            // Set tile to 'smooth target' tile
            GetTileIndexFromTypeSubIndex(uiNewType, REQUIRES_SMOOTHING_TILE, &NewTile);
            SetLandIndex(iNewIndex, NewTile, uiNewType, FALSE);

            // If we are top-most, add to smooth list
            sNumSmoothTiles++;
            puiSmoothTiles =
                (uint32_t *)MemRealloc(puiSmoothTiles, sNumSmoothTiles * sizeof(uint32_t));
            puiSmoothTiles[sNumSmoothTiles - 1] = iNewIndex;
          }
        }
      }
    }
  }

  // Once here, smooth any tiles that need it
  if (sNumSmoothTiles > 0) {
    for (cnt1 = 0; cnt1 < sNumSmoothTiles; cnt1++) {
      SmoothTerrainRadius(puiSmoothTiles[cnt1], uiNewType, 10, FALSE);
    }
    MemFree(puiSmoothTiles);
  }

  return (TRUE);
}

// ATE FIXES
void PasteTextureEx(int16_t sGridNo, uint16_t usType) {
  uint16_t usIndex;
  uint8_t ubTypeLevel;
  uint16_t NewTile;

  // CHECK IF THIS TEXTURE EXISTS!
  if (TypeExistsInLandLayer(sGridNo, usType, &usIndex)) {
    if (GetTypeLandLevel(sGridNo, usType, &ubTypeLevel)) {
      // If top-land , do not change
      if (ubTypeLevel != LANDHEAD) {
        PasteExistingTexture(sGridNo, usIndex);
      }
    }
  } else {
    // Fill with just first tile, smoothworld() will pick proper piece later
    GetTileIndexFromTypeSubIndex(usType, REQUIRES_SMOOTHING_TILE, &NewTile);

    SetLandIndex(sGridNo, NewTile, usType, FALSE);
  }
}

void PasteTextureFromRadiusEx(int16_t sGridNo, uint16_t usType, uint8_t ubRadius) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int32_t iNewIndex;
  int32_t leftmost;

  // Determine start end end indicies and num rows
  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  if (sGridNo >= 0x8000) return;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      iNewIndex = sGridNo + (WORLD_COLS * cnt1) + cnt2;

      if (iNewIndex >= 0 && iNewIndex < WORLD_MAX && iNewIndex >= leftmost &&
          iNewIndex < (leftmost + WORLD_COLS)) {
        PasteTextureEx(sGridNo, usType);
      }
    }
  }

  return;
}

// FUNCTION TO GIVE NEAREST GRIDNO OF A CLIFF
#define LAND_DROP_1 FIRSTCLIFF1
#define LAND_DROP_2 FIRSTCLIFF11
#define LAND_DROP_3 FIRSTCLIFF12
#define LAND_DROP_4 FIRSTCLIFF15
#define LAND_DROP_5 FIRSTCLIFF8
void RaiseWorldLand() {
  int32_t cnt;
  uint32_t sTempGridNo;
  struct LEVELNODE *pStruct;
  TILE_ELEMENT *pTileElement;
  BOOLEAN fRaiseSet;
  BOOLEAN fSomethingRaised = FALSE;
  uint8_t ubLoop;
  uint16_t usIndex;
  int32_t iStartNumberOfRaises = 0;
  int32_t iNumberOfRaises = 0;
  BOOLEAN fAboutToRaise = FALSE;

  fRaiseSet = FALSE;

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RAISE_LAND_START);
    gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RAISE_LAND_END);
  }

  for (cnt = 0; cnt < WORLD_MAX; cnt++) {
    // Get Structure levelnode
    pStruct = gpWorldLevelData[cnt].pStructHead;
    gpWorldLevelData[cnt].sHeight = 0;

    while (pStruct) {
      pTileElement = &(gTileDatabase[pStruct->usIndex]);
      if (pTileElement->fType == FIRSTCLIFF) {
        fSomethingRaised = TRUE;
        // DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("Cliff found at count=%d",cnt));
        if (pTileElement->ubNumberOfTiles > 1) {
          // DebugMsg(TOPIC_JA2,DBG_LEVEL_3,String("Cliff has %d children",
          // pTileElement->ubNumberOfTiles));
          for (ubLoop = 0; ubLoop < pTileElement->ubNumberOfTiles; ubLoop++) {
            usIndex = pStruct->usIndex;
            // need means to turn land raising on and off based on the tile ID and the offset in the
            // tile database when reading into the mapsystem
            // turning off of land raising can only be done
            // presently by CLIFF object/tileset 1
            // so simply detect this tile set and turn off instead of on
            // element 1 is 12 tiles and is unique

            sTempGridNo = cnt + pTileElement->pTileLocData[ubLoop].bTileOffsetX +
                          pTileElement->pTileLocData[ubLoop].bTileOffsetY * WORLD_COLS;
            // Check for valid gridno
            if (OutOfBounds((int16_t)cnt, (int16_t)sTempGridNo)) {
              continue;
            }
            // if (pTileElement->ubNumberOfTiles==10)
            if ((usIndex == LAND_DROP_1) || (usIndex == LAND_DROP_2) || (usIndex == LAND_DROP_4)) {
              gpWorldLevelData[sTempGridNo].uiFlags &= (~MAPELEMENT_RAISE_LAND_START);
              gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_END;
            } else if ((usIndex == LAND_DROP_5) && (ubLoop == 4)) {
              gpWorldLevelData[sTempGridNo].uiFlags &= (~MAPELEMENT_RAISE_LAND_START);
              gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_END;
              if (!(gpWorldLevelData[sTempGridNo + 1].uiFlags & MAPELEMENT_RAISE_LAND_START)) {
                gpWorldLevelData[sTempGridNo + 1].uiFlags |= MAPELEMENT_RAISE_LAND_START;
              }
            } else if ((usIndex == LAND_DROP_3) &&
                       ((ubLoop == 0) || (ubLoop == 1) || (ubLoop == 2))) {
              gpWorldLevelData[sTempGridNo].uiFlags &= (~MAPELEMENT_RAISE_LAND_START);
              gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_END;
            } else {
              gpWorldLevelData[sTempGridNo].uiFlags |= MAPELEMENT_RAISE_LAND_START;
            }
          }
        } else {
          if (usIndex == LAND_DROP_3) {
            gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RAISE_LAND_START);
            gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RAISE_LAND_END;
          } else {
            // if (pTileElement->ubNumberOfTiles==10)
            if (usIndex == LAND_DROP_1) {
              gpWorldLevelData[cnt].uiFlags &= (~MAPELEMENT_RAISE_LAND_START);
              gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RAISE_LAND_END;
            } else
              gpWorldLevelData[cnt].uiFlags |= MAPELEMENT_RAISE_LAND_START;
          }
        }
      }
      pStruct = pStruct->pNext;
    }
  }

  if (fSomethingRaised == FALSE) {
    // no cliffs
    return;
  }

  // run through again, this pass is for placing raiselandstart in rows that have raiseland end but
  // no raiselandstart
  for (cnt = WORLD_MAX - 1; cnt >= 0; cnt--) {
    if (cnt % WORLD_ROWS == WORLD_ROWS - 1) {
      // start of new row
      fRaiseSet = FALSE;
    }
    if (gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_START) {
      fRaiseSet = TRUE;
    } else if ((gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_END) && (!fRaiseSet)) {
      // there is a dropoff without a rise.
      // back up and set beginning to raiseland start
      gpWorldLevelData[cnt + ((WORLD_ROWS - 1) - (cnt % WORLD_ROWS))].uiFlags &=
          (~MAPELEMENT_RAISE_LAND_END);
      gpWorldLevelData[cnt + ((WORLD_ROWS - 1) - (cnt % WORLD_ROWS))].uiFlags |=
          MAPELEMENT_RAISE_LAND_START;
      if (cnt + ((WORLD_ROWS - 1) - (cnt % WORLD_ROWS)) - WORLD_ROWS > 0) {
        gpWorldLevelData[cnt + ((WORLD_ROWS - 1) - (cnt % WORLD_ROWS)) - WORLD_ROWS].uiFlags &=
            (~MAPELEMENT_RAISE_LAND_END);
        gpWorldLevelData[cnt + ((WORLD_ROWS - 1) - (cnt % WORLD_ROWS)) - WORLD_ROWS].uiFlags |=
            MAPELEMENT_RAISE_LAND_START;
      }
      fRaiseSet = TRUE;
    }
  }
  fRaiseSet = FALSE;
  // Look for a cliff face that is along either the lower edge or the right edge of the map, this is
  // used for a special case fill start at y=159, x= 80 and go to x=159, y=80

  // now check along x=159, y=80 to x=80, y=0
  for (cnt = ((WORLD_COLS * WORLD_ROWS) - (WORLD_ROWS / 2) * (WORLD_ROWS - 2) - 1);
       cnt > WORLD_ROWS - 1; cnt -= (WORLD_ROWS + 1)) {
    if (fAboutToRaise == TRUE) {
      fRaiseSet = TRUE;
      fAboutToRaise = FALSE;
    }

    if ((gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_START) ||
        (gpWorldLevelData[cnt - 1].uiFlags & MAPELEMENT_RAISE_LAND_START) ||
        (gpWorldLevelData[cnt + 1].uiFlags & MAPELEMENT_RAISE_LAND_START)) {
      fAboutToRaise = TRUE;
      fRaiseSet = FALSE;
    } else if ((gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_END) ||
               (gpWorldLevelData[cnt - 1].uiFlags & MAPELEMENT_RAISE_LAND_END) ||
               (gpWorldLevelData[cnt + 1].uiFlags & MAPELEMENT_RAISE_LAND_END)) {
      fRaiseSet = FALSE;
    }
    if (fRaiseSet) {
      gpWorldLevelData[cnt + ((WORLD_ROWS - 1) - (cnt % WORLD_ROWS))].uiFlags |=
          MAPELEMENT_RAISE_LAND_START;
      // gpWorldLevelData[cnt].uiFlags|=MAPELEMENT_RAISE_LAND_START;
      // gpWorldLevelData[cnt-1].uiFlags|=MAPELEMENT_RAISE_LAND_START;
      // DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Land Raise start at count: %d is raised",cnt ));
      // DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Land Raise start at count: %d is raised",cnt - 1
      // ));
    }
  }

  // fRaiseSet = FALSE;

  // Now go through the world, starting at x=max(x) and y=max(y) and work backwards
  // if a cliff is found turn raise flag on, if the end of a screen is found, turn off
  // the system uses world_cord=x+y*(row_size)

  for (cnt = WORLD_MAX - 1; cnt >= 0; cnt--) {
    //  reset the RAISE to FALSE
    // End of the row
    if (!(cnt % WORLD_ROWS)) {
      iNumberOfRaises = 0;
      iStartNumberOfRaises = 0;
    }

    if ((gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_END)) {
      if (cnt > 1) {
        if ((!(gpWorldLevelData[cnt - 1].uiFlags & MAPELEMENT_RAISE_LAND_END) &&
             !(gpWorldLevelData[cnt - 2].uiFlags & MAPELEMENT_RAISE_LAND_END))) {
          iNumberOfRaises--;
        }
      }
    } else if (gpWorldLevelData[cnt].uiFlags & MAPELEMENT_RAISE_LAND_START) {
      // check tile before and after, if either are raise land flagged, then don't increment number
      // of raises
      if (cnt < WORLD_MAX - 2) {
        if ((!(gpWorldLevelData[cnt + 1].uiFlags & MAPELEMENT_RAISE_LAND_START) &&
             !(gpWorldLevelData[cnt + 2].uiFlags & MAPELEMENT_RAISE_LAND_START))) {
          iNumberOfRaises++;
        }
      }
    }

    // look at number of raises.. if negative, then we have more downs than ups, restart row with
    // raises + 1; now raise land of any tile while the flag is on
    if (iNumberOfRaises < 0) {
      // something wrong, reset cnt
      iStartNumberOfRaises++;
      cnt += WORLD_ROWS - cnt % WORLD_ROWS;
      iNumberOfRaises = iStartNumberOfRaises;
      continue;
    }

    if (iNumberOfRaises >= 0) {
      // DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Land Raise start at count: %d is raised",cnt ));
      gpWorldLevelData[cnt].sHeight = iNumberOfRaises * WORLD_CLIFF_HEIGHT;
    }
  }

  for (cnt = WORLD_MAX - 1; cnt >= 0; cnt--) {
    if ((cnt < WORLD_MAX - WORLD_ROWS) && (cnt > WORLD_ROWS)) {
      if ((gpWorldLevelData[cnt + WORLD_ROWS].sHeight ==
           gpWorldLevelData[cnt - WORLD_ROWS].sHeight) &&
          (gpWorldLevelData[cnt].sHeight != gpWorldLevelData[cnt - WORLD_ROWS].sHeight)) {
        gpWorldLevelData[cnt].sHeight = gpWorldLevelData[cnt + WORLD_ROWS].sHeight;
      } else if ((gpWorldLevelData[cnt].sHeight > gpWorldLevelData[cnt - WORLD_ROWS].sHeight) &&
                 (gpWorldLevelData[cnt + WORLD_ROWS].sHeight !=
                  gpWorldLevelData[cnt - WORLD_ROWS].sHeight) &&
                 (gpWorldLevelData[cnt].sHeight > gpWorldLevelData[cnt + WORLD_ROWS].sHeight)) {
        if (gpWorldLevelData[cnt - WORLD_ROWS].sHeight >
            gpWorldLevelData[cnt + WORLD_ROWS].sHeight) {
          gpWorldLevelData[cnt].sHeight = gpWorldLevelData[cnt - WORLD_ROWS].sHeight;
        } else {
          gpWorldLevelData[cnt].sHeight = gpWorldLevelData[cnt + WORLD_ROWS].sHeight;
        }
      }
    }
  }

  //*/
}

void EliminateObjectLayerRedundancy() {
  int32_t i, numRoads, numAnothers;
  uint32_t uiType;
  struct LEVELNODE *pObject, *pValidRoad, *pValidAnother;
  uint16_t usIndex;

  for (i = 0; i < WORLD_MAX; i++) {  // Eliminate all but the last ROADPIECE and ANOTHERDEBRIS
    pObject = gpWorldLevelData[i].pObjectHead;
    pValidRoad = pValidAnother = NULL;
    numRoads = numAnothers = 0;
    while (pObject) {
      GetTileType(pObject->usIndex, &uiType);
      if (uiType == ROADPIECES) {  // keep track of the last valid road piece, and count the total
        pValidRoad = pObject;
        numRoads++;
      } else if (uiType == ANOTHERDEBRIS) {  // keep track of the last valid another debris, and
                                             // count the total
        pValidAnother = pObject;
        numAnothers++;
      }
      pObject = pObject->pNext;
    }
    if (pValidRoad && numRoads > 1) {  // we have more than two roadpieces on the same gridno, so
                                       // get rid of them, replacing it
      // with the visible one.
      usIndex = pValidRoad->usIndex;
      RemoveAllObjectsOfTypeRange(i, ROADPIECES, ROADPIECES);
      AddObjectToHead(i, usIndex);
    }
    if (pValidAnother && numAnothers > 1) {  // we have more than two anotherdebris on the same
                                             // gridno, so get rid of them, replacing it
      // with the visible one.
      usIndex = pValidAnother->usIndex;
      RemoveAllObjectsOfTypeRange(i, ANOTHERDEBRIS, ANOTHERDEBRIS);
      AddObjectToHead(i, usIndex);
    }
  }
}
