// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Editor/NewSmooth.h"

#include <stdlib.h>

#include "BuildDefines.h"
#include "Editor/EditSys.h"
#include "Editor/EditorBuildings.h"
#include "Editor/EditorUndo.h"
#include "Editor/SmoothingUtils.h"
#include "SGP/Random.h"
#include "TileEngine/Environment.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"

BOOLEAN CaveAtGridNo(int32_t iMapIndex);
uint16_t GetCaveTileIndexFromPerimeterValue(uint8_t ubTotal);
uint8_t CalcNewCavePerimeterValue(int32_t iMapIndex);

BOOLEAN CaveAtGridNo(int32_t iMapIndex) {
  struct STRUCTURE *pStruct;
  struct LEVELNODE *pLevel;
  if (iMapIndex < 0 || iMapIndex >= NOWHERE) return TRUE;
  pStruct = gpWorldLevelData[iMapIndex].pStructureHead;
  while (pStruct) {
    if (pStruct->fFlags & STRUCTURE_CAVEWALL) {
      return TRUE;
    }
    pStruct = pStruct->pNext;
  }
  // may not have structure information, so check if there is a levelnode flag.
  pLevel = gpWorldLevelData[iMapIndex].pStructHead;
  while (pLevel) {
    if (pLevel->uiFlags & LEVELNODE_CAVE) {
      return TRUE;
    }
    pLevel = pLevel->pNext;
  }
  return FALSE;
}

uint16_t GetCaveTileIndexFromPerimeterValue(uint8_t ubTotal) {
  uint16_t usType = FIRSTWALL;
  uint16_t usIndex;
  uint16_t usTileIndex;

  switch (ubTotal) {
    case 0x00:
    case 0x10:
    case 0x20:
    case 0x30:
    case 0x40:
    case 0x50:
    case 0x60:
    case 0x70:
    case 0x80:
    case 0x90:
    case 0xa0:
    case 0xb0:
    case 0xc0:
    case 0xd0:
    case 0xe0:
    case 0xf0:
      // usIndex = 0;
      // break;
      return 0xffff;
    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
    case 0x41:
    case 0x51:
    case 0x61:
    case 0x71:
    case 0x81:
    case 0x91:
    case 0xa1:
    case 0xb1:
    case 0xc1:
    case 0xd1:
    case 0xe1:
    case 0xf1:
      usType = SECONDWALL;
      usIndex = 1 + (uint16_t)Random(4);
      break;
    case 0x02:
    case 0x12:
    case 0x22:
    case 0x32:
    case 0x42:
    case 0x52:
    case 0x62:
    case 0x72:
    case 0x82:
    case 0x92:
    case 0xa2:
    case 0xb2:
    case 0xc2:
    case 0xd2:
    case 0xe2:
    case 0xf2:
      usType = SECONDWALL;
      usIndex = 5 + (uint16_t)Random(4);
      break;
    case 0x03:
    case 0x13:
    case 0x43:
    case 0x53:
    case 0x83:
    case 0x93:
    case 0xc3:
    case 0xd3:
      usIndex = 1;
      break;
    case 0x04:
    case 0x14:
    case 0x24:
    case 0x34:
    case 0x44:
    case 0x54:
    case 0x64:
    case 0x74:
    case 0x84:
    case 0x94:
    case 0xa4:
    case 0xb4:
    case 0xc4:
    case 0xd4:
    case 0xe4:
    case 0xf4:
      usType = SECONDWALL;
      usIndex = 9 + (uint16_t)Random(4);
      break;
    case 0x05:
    case 0x15:
    case 0x25:
    case 0x35:
    case 0x45:
    case 0x55:
    case 0x65:
    case 0x75:
    case 0x85:
    case 0x95:
    case 0xa5:
    case 0xb5:
    case 0xc5:
    case 0xd5:
    case 0xe5:
    case 0xf5:
      usIndex = 2 + (uint16_t)Random(2);
      break;
    case 0x06:
    case 0x16:
    case 0x26:
    case 0x36:
    case 0x86:
    case 0x96:
    case 0xa6:
    case 0xb6:
      usIndex = 4;
      break;
    case 0x07:
    case 0x17:
    case 0x87:
    case 0x97:
      usIndex = 5;
      break;
    case 0x08:
    case 0x18:
    case 0x28:
    case 0x38:
    case 0x48:
    case 0x58:
    case 0x68:
    case 0x78:
    case 0x88:
    case 0x98:
    case 0xa8:
    case 0xb8:
    case 0xc8:
    case 0xd8:
    case 0xe8:
    case 0xf8:
      usType = SECONDWALL;
      usIndex = 13 + (uint16_t)Random(4);
      break;
    case 0x09:
    case 0x29:
    case 0x49:
    case 0x69:
    case 0x89:
    case 0xa9:
    case 0xc9:
    case 0xe9:
      usIndex = 6;
      break;
    case 0x0a:
    case 0x1a:
    case 0x2a:
    case 0x3a:
    case 0x4a:
    case 0x5a:
    case 0x6a:
    case 0x7a:
    case 0x8a:
    case 0x9a:
    case 0xaa:
    case 0xba:
    case 0xca:
    case 0xda:
    case 0xea:
    case 0xfa:
      usIndex = 7 + (uint16_t)Random(2);
      break;
    case 0x0b:
    case 0x4b:
    case 0x8b:
    case 0xcb:
      usIndex = 9;
      break;
    case 0x0c:
    case 0x1c:
    case 0x2c:
    case 0x3c:
    case 0x4c:
    case 0x5c:
    case 0x6c:
    case 0x7c:
      usIndex = 10;
      break;
    case 0x0d:
    case 0x2d:
    case 0x4d:
    case 0x6d:
      usIndex = 11;
      break;
    case 0x0e:
    case 0x1e:
    case 0x2e:
    case 0x3e:
      usIndex = 12;
      break;
    case 0x0f:
      usIndex = 13;
      break;
    case 0x19:
    case 0x39:
    case 0x59:
    case 0x79:
    case 0x99:
    case 0xb9:
    case 0xd9:
    case 0xf9:
      usIndex = 14 + (uint16_t)Random(2);
      break;
    case 0x1b:
    case 0x5b:
    case 0x9b:
    case 0xdb:
      usIndex = 16;
      break;
    case 0x1d:
    case 0x3d:
    case 0x5d:
    case 0x7d:
      usIndex = 17;
      break;
    case 0x1f:
      usIndex = 18;
      break;
    case 0x23:
    case 0x33:
    case 0x63:
    case 0x73:
    case 0xa3:
    case 0xb3:
    case 0xe3:
    case 0xf3:
      usIndex = 19 + (uint16_t)Random(2);
      break;
    case 0x27:
    case 0x37:
    case 0xa7:
    case 0xb7:
      usIndex = 21;
      break;
    case 0x2b:
    case 0x6b:
    case 0xab:
    case 0xeb:
      usIndex = 22;
      break;
    case 0x2f:
      usIndex = 23;
      break;
    case 0x3b:
    case 0x7b:
    case 0xbb:
    case 0xfb:
      usIndex = 24 + (uint16_t)Random(3);
      break;
    case 0x3f:
      usIndex = 27;
      break;
    case 0x46:
    case 0x56:
    case 0x66:
    case 0x76:
    case 0xc6:
    case 0xd6:
    case 0xe6:
    case 0xf6:
      usIndex = 28 + (uint16_t)Random(2);
      break;
    case 0x47:
    case 0x57:
    case 0xc7:
    case 0xd7:
      usIndex = 30;
      break;
    case 0x4e:
    case 0x5e:
    case 0x6e:
    case 0x7e:
      usIndex = 31;
      break;
    case 0x4f:
      usIndex = 32;
      break;
    case 0x5f:
      usIndex = 33;
      break;
    case 0x67:
    case 0x77:
    case 0xe7:
    case 0xf7:
      usIndex = 34 + (uint16_t)Random(3);
      break;
    case 0x6f:
      usIndex = 37;
      break;
    case 0x7f:
      usIndex = 38 + (uint16_t)Random(2);
      break;
    case 0x8c:
    case 0x9c:
    case 0xac:
    case 0xbc:
    case 0xcc:
    case 0xdc:
    case 0xec:
    case 0xfc:
      usIndex = 40 + (uint16_t)Random(2);
      break;
    case 0x8d:
    case 0xad:
    case 0xcd:
    case 0xed:
      usIndex = 42;
      break;
    case 0x8e:
    case 0x9e:
    case 0xae:
    case 0xbe:
      usIndex = 43;
      break;
    case 0x8f:
      usIndex = 44;
      break;
    case 0x9d:
    case 0xbd:
    case 0xdd:
    case 0xfd:
      usIndex = 45 + (uint16_t)Random(3);
      break;
    case 0x9f:
      usIndex = 48;
      break;
    case 0xaf:
      usIndex = 49;
      break;
    case 0xbf:
      usIndex = 50 + (uint16_t)Random(2);
      break;
    case 0xce:
    case 0xde:
    case 0xee:
    case 0xfe:
      usIndex = 52 + (uint16_t)Random(3);
      break;
    case 0xcf:
      usIndex = 55;
      break;
    case 0xdf:
      usIndex = 56 + (uint16_t)Random(2);
      break;
    case 0xef:
      usIndex = 58 + (uint16_t)Random(2);
      break;
    case 0xff:
      usIndex = 60 + (uint16_t)Random(6);
      break;
  }
  GetTileIndexFromTypeSubIndex(usType, usIndex, &usTileIndex);
  return usTileIndex;
}

//	16 | 1 | 32
//	---+---+---
//	 8 |   | 2
//	---+---+---
//	128| 4 | 64
// These values are combined in any possible order ranging in
// values from 0 - 255.  If there is a cave existing in any of
// these bordering gridnos, then the corresponding number is added
// to this total.  The lookup table has been precalculated to know
// which piece to use for all of these combinations.  In many cases,
// up to 16 combinations can share the same graphic image, as corners
// may not effect the look of the piece.
uint8_t CalcNewCavePerimeterValue(int32_t iMapIndex) {
  uint8_t ubTotal = 0;
  if (CaveAtGridNo(iMapIndex - WORLD_COLS)) ubTotal += 0x01;      // north
  if (CaveAtGridNo(iMapIndex + 1)) ubTotal += 0x02;               // east
  if (CaveAtGridNo(iMapIndex + WORLD_COLS)) ubTotal += 0x04;      // south
  if (CaveAtGridNo(iMapIndex - 1)) ubTotal += 0x08;               // west
  if (CaveAtGridNo(iMapIndex - WORLD_COLS - 1)) ubTotal += 0x10;  // north west
  if (CaveAtGridNo(iMapIndex - WORLD_COLS + 1)) ubTotal += 0x20;  // north east
  if (CaveAtGridNo(iMapIndex + WORLD_COLS + 1)) ubTotal += 0x40;  // south east
  if (CaveAtGridNo(iMapIndex + WORLD_COLS - 1)) ubTotal += 0x80;  // south west
  return ubTotal;
}

void AddCave(int32_t iMapIndex, uint16_t usIndex) {
  struct LEVELNODE *pStruct;

  if (iMapIndex < 0 || iMapIndex >= NOWHERE) return;
  // First toast any existing wall (caves)
  RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
  // Now, add this piece
  if (!AddWallToStructLayer(iMapIndex, usIndex, TRUE)) return;
  // Set the cave flag
  pStruct = gpWorldLevelData[iMapIndex].pStructHead;
  Assert(pStruct);
  pStruct->uiFlags |= LEVELNODE_CAVE;
}

// These walls have shadows associated with them, and are draw when the wall is drawn.
#define EXTERIOR_L_SHADOW_INDEX 30
#define INTERIOR_BOTTOMEND_SHADOW_INDEX 31

// Wall Look Up Table containing variants and indices with each row being a different walltype.
int8_t gbWallTileLUT[NUM_WALL_TYPES][7] = {
    //	The number of variants of this tile type.
    //  |			The first relative index of the wall type (FIRSTWALL, SECONDWALL,
    //  etc. )  walltype + 10
    //	|			|		The 2nd relative index  ( walltype + 11 )
    //	|			|		|		3rd	4th 5th 6th
    //	|			|		|		|		|   |   |
    {6, 10, 11, 12, 27, 28, 29},  // INTERIOR_L
    {6, 7, 8, 9, 24, 25, 26},     // INTERIOR_R
    {6, 4, 5, 6, 21, 22, 23},     // EXTERIOR_L
    {6, 1, 2, 3, 18, 19, 20},     // EXTERIOR_R
    {1, 14, 0, 0, 0, 0, 0},       // INTERIOR_CORNER
    {1, 15, 0, 0, 0, 0, 0},       // INTERIOR_BOTTOMEND
    {1, 13, 0, 0, 0, 0, 0},       // EXTERIOR_BOTTOMEND
    {1, 16, 0, 0, 0, 0, 0},       // INTERIOR_EXTENDED
    {1, 57, 0, 0, 0, 0, 0},       // EXTERIOR_EXTENDED
    {1, 56, 0, 0, 0, 0, 0},       // INTERIOR_EXTENDED_BOTTOMEND
    {1, 17, 0, 0, 0, 0, 0},       // EXTERIOR_EXTENDED_BOTTOMEND
};

// Roof table -- such a small table, using definitions instead.
#define TOP_ROOF_INDEX 2
#define BOTTOM_ROOF_INDEX 4
#define LEFT_ROOF_INDEX 1
#define RIGHT_ROOF_INDEX 5
#define TOPLEFT_ROOF_INDEX 3
#define TOPRIGHT_ROOF_INDEX 7
#define BOTTOMLEFT_ROOF_INDEX 8
#define BOTTOMRIGHT_ROOF_INDEX 6
#define CENTER_ROOF_BASE_INDEX 9
#define CENTER_ROOF_VARIANTS 3

// slant roof table
#define THIN_BOTTOM 1
#define THIN_TOP 2
#define THIN_LEFT 5
#define THIN_RIGHT 6
#define THICK_BOTTOM 3
#define THICK_TOP 4
#define THICK_LEFT 7
#define THICK_RIGHT 8
#define VWALL_LEFT 32
#define VWALL_RIGHT 33
#define HWALL_LEFT 35
#define HWALL_RIGHT 34

#define FLOOR_VARIANTS 8

// PRIVATELY "ENCAPSULATED" FUNCTIONS

// These construction functions do all the smoothing.
// NOTE:  passing null for wall/roof type will force the function to search for the nearest
//  existing respective type.
void BuildSlantRoof(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                    uint16_t usWallType, uint16_t usRoofType, BOOLEAN fVertical);

void BulldozeNature(uint32_t iMapIndex);
void EraseRoof(uint32_t iMapIndex);
void EraseFloor(uint32_t iMapIndex);
void EraseBuilding(uint32_t iMapIndex);
void EraseFloorOwnedBuildingPieces(uint32_t iMapIndex);
void ConsiderEffectsOfNewWallPiece(uint32_t iMapIndex, uint8_t usWallOrientation);

//----------------------------------------------------------------------------------------------------
// BEGIN IMPLEMENTATION OF PRIVATE FUNCTIONS
//----------------------------------------------------------------------------------------------------

void BuildSlantRoof(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                    uint16_t usWallType, uint16_t usRoofType, BOOLEAN fVertical) {
  int32_t i;
  uint16_t usTileIndex;
  int32_t iMapIndex;
  if (fVertical) {
    iMapIndex = iBottom * WORLD_COLS + iLeft;
    // This happens to be the only mapindex that needs to be backed up.  The rest have already been
    // done because of the building code before this.
    AddToUndoList(iMapIndex + 8);
    // Add the closest viewable pieces.  There are two aframe walls pieces, and extended aframe roof
    // pieces.
    GetTileIndexFromTypeSubIndex(usWallType, VWALL_LEFT, &usTileIndex);
    AddRoofToHead(iMapIndex + 4, usTileIndex);
    GetTileIndexFromTypeSubIndex(usWallType, VWALL_RIGHT, &usTileIndex);
    AddRoofToHead(iMapIndex + 8, usTileIndex);
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_LEFT, &usTileIndex);
    AddRoofToHead(iMapIndex + 3, usTileIndex);
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_RIGHT, &usTileIndex);
    AddRoofToHead(iMapIndex + 7, usTileIndex);
    for (i = iBottom - 1; i > iTop; i--) {
      iMapIndex -= WORLD_COLS;
      GetTileIndexFromTypeSubIndex(usRoofType, THIN_LEFT, &usTileIndex);
      AddRoofToHead(iMapIndex + 3, usTileIndex);
      GetTileIndexFromTypeSubIndex(usRoofType, THIN_RIGHT, &usTileIndex);
      AddRoofToHead(iMapIndex + 7, usTileIndex);
    }
    iMapIndex -= WORLD_COLS;
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_LEFT, &usTileIndex);
    AddRoofToHead(iMapIndex + 3, usTileIndex);
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_RIGHT, &usTileIndex);
    AddRoofToHead(iMapIndex + 7, usTileIndex);
  } else {
    iMapIndex = iTop * WORLD_COLS + iRight;
    // This happens to be the only mapindex that needs to be backed up.  The rest have already been
    // done because of the building code before this.
    AddToUndoList(iMapIndex + 8 * WORLD_COLS);
    // Add the closest viewable pieces.  There are two aframe walls pieces, and extended aframe roof
    // pieces.
    GetTileIndexFromTypeSubIndex(usWallType, HWALL_LEFT, &usTileIndex);
    AddRoofToHead(iMapIndex + 4 * WORLD_COLS, usTileIndex);
    GetTileIndexFromTypeSubIndex(usWallType, HWALL_RIGHT, &usTileIndex);
    AddRoofToHead(iMapIndex + 8 * WORLD_COLS, usTileIndex);
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_TOP, &usTileIndex);
    AddRoofToHead(iMapIndex + 3 * WORLD_COLS, usTileIndex);
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_BOTTOM, &usTileIndex);
    AddRoofToHead(iMapIndex + 7 * WORLD_COLS, usTileIndex);
    for (i = iRight - 1; i > iLeft; i--) {
      iMapIndex--;
      GetTileIndexFromTypeSubIndex(usRoofType, THIN_TOP, &usTileIndex);
      AddRoofToHead(iMapIndex + 3 * WORLD_COLS, usTileIndex);
      GetTileIndexFromTypeSubIndex(usRoofType, THIN_BOTTOM, &usTileIndex);
      AddRoofToHead(iMapIndex + 7 * WORLD_COLS, usTileIndex);
    }
    iMapIndex--;
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_TOP, &usTileIndex);
    AddRoofToHead(iMapIndex + 3 * WORLD_COLS, usTileIndex);
    GetTileIndexFromTypeSubIndex(usRoofType, THICK_BOTTOM, &usTileIndex);
    AddRoofToHead(iMapIndex + 7 * WORLD_COLS, usTileIndex);
  }
}

uint16_t PickAWallPiece(uint16_t usWallPieceType) {
  uint16_t usVariants;
  uint16_t usVariantChosen;
  uint16_t usWallPieceChosen = 0;
  if (usWallPieceType >= 0 || usWallPieceType < NUM_WALL_TYPES) {
    usVariants = gbWallTileLUT[usWallPieceType][0];
    usVariantChosen = (rand() % usVariants) + 1;
    usWallPieceChosen = gbWallTileLUT[usWallPieceType][usVariantChosen];
  }
  return usWallPieceChosen;
}

// From a given gridNo and perspective (wallpiece), it will calculate the new piece, and
// where to place it as well as handle the special cases.
// NOTE:  Placing top and left pieces are placed relative to the gridno, and the gridNo will
// shift accordingly to place the piece.  Pretend you are the floor, and you want to place a piece
// to the left.  You pass your position, and INTERIOR_LEFT, with interior meaning from the inside of
// a building.  If you were outside the building, you would call EXTERIOR_LEFT.  The left tile will
// be placed on gridNo - 1!  Up tiles will be placed on gridNo - 160.
// NOTE:  Passing NULL for usWallType will force it to calculate the closest existing wall type, and
//  use that for building this new wall.  It is necessary for restructuring a building, but not for
//  adding on to an existing building, where the type is already known.
void BuildWallPiece(uint32_t iMapIndex, uint8_t ubWallPiece, uint16_t usWallType) {
  int16_t sIndex;
  uint16_t usTileIndex;
  uint16_t ubWallClass;
  struct LEVELNODE *pStruct;
  if (!usWallType) {
    usWallType = SearchForWallType(iMapIndex);
  }
  switch (ubWallPiece) {
    case EXTERIOR_TOP:
      iMapIndex -= WORLD_COLS;
      // exterior bottom left corner generated
      if (!gfBasement && GetVerticalWall(iMapIndex - 1) &&
          !GetVerticalWall(iMapIndex + WORLD_COLS -
                           1)) {  // Special case where a shadow has to be created as it now is a
                                  // bottom corner and
        // must contribute to the bottom shadow.
        AddToUndoList(iMapIndex - 1);
        GetTileIndexFromTypeSubIndex(usWallType, INTERIOR_BOTTOMEND_SHADOW_INDEX, &usTileIndex);
        AddExclusiveShadow(iMapIndex - 1, usTileIndex);
      }
      if ((pStruct = GetVerticalWall(iMapIndex)))  // right corner
      {  // Special case where placing the new wall will generate a corner to the right, so replace
        // the vertical piece with a bottomend.
        sIndex = PickAWallPiece(EXTERIOR_BOTTOMEND);
        AddToUndoList(iMapIndex);
        GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
        ReplaceStructIndex(iMapIndex, pStruct->usIndex, usTileIndex);
      }
      ubWallClass = EXTERIOR_L;
      if (!gfBasement) {
        // All exterior_l walls have shadows.
        GetTileIndexFromTypeSubIndex(usWallType, EXTERIOR_L_SHADOW_INDEX, &usTileIndex);
        AddExclusiveShadow(iMapIndex, usTileIndex);
      }
      break;
    case EXTERIOR_BOTTOM:
      ubWallClass = INTERIOR_L;
      if ((pStruct = GetVerticalWall(iMapIndex + WORLD_COLS - 1)) &&
          !GetVerticalWall(iMapIndex - 1)) {
        sIndex = PickAWallPiece(INTERIOR_EXTENDED);
        AddToUndoList(iMapIndex + WORLD_COLS - 1);
        GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
        ReplaceStructIndex(iMapIndex + WORLD_COLS - 1, pStruct->usIndex, usTileIndex);
      }
      break;
    case EXTERIOR_LEFT:
      iMapIndex--;
      ubWallClass = EXTERIOR_R;
      if (GetHorizontalWall(iMapIndex)) {  // Special case where placing the new wall will generate
                                           // a corner.  This piece
        // becomes an exterior bottomend, but nothing else is effected.
        ubWallClass = EXTERIOR_BOTTOMEND;
      }
      if (GetHorizontalWall(iMapIndex - WORLD_COLS + 1)) {
        if (ubWallClass == EXTERIOR_BOTTOMEND)
          ubWallClass = EXTERIOR_EXTENDED_BOTTOMEND;
        else
          ubWallClass = EXTERIOR_EXTENDED;
      }
      break;
    case EXTERIOR_RIGHT:
      ubWallClass = INTERIOR_R;
      if (GetHorizontalWall(iMapIndex - WORLD_COLS + 1) &&
          !GetHorizontalWall(iMapIndex - WORLD_COLS)) {
        ubWallClass = INTERIOR_EXTENDED;
      } else if (GetHorizontalWall(iMapIndex) && !GetVerticalWall(iMapIndex + WORLD_COLS)) {
        ubWallClass = INTERIOR_BOTTOMEND;
      }
      break;
    case INTERIOR_TOP:
      iMapIndex -= WORLD_COLS;
      ubWallClass = INTERIOR_L;
      // check for a lower left corner.
      if ((pStruct = GetVerticalWall(iMapIndex + WORLD_COLS - 1))) {
        // Replace the piece with an extended piece.
        sIndex = PickAWallPiece(INTERIOR_EXTENDED);
        AddToUndoList(iMapIndex + WORLD_COLS - 1);
        GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
        ReplaceStructIndex(iMapIndex + WORLD_COLS - 1, pStruct->usIndex, usTileIndex);
        // NOTE:  Not yet checking for interior extended bottomend!
      }
      if ((pStruct = GetVerticalWall(iMapIndex))) {
        sIndex = PickAWallPiece(INTERIOR_BOTTOMEND);
        AddToUndoList(iMapIndex);
        GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
        ReplaceStructIndex(iMapIndex, pStruct->usIndex, usTileIndex);
      }
      break;
    case INTERIOR_BOTTOM:
      ubWallClass = EXTERIOR_L;
      if ((pStruct = GetVerticalWall(iMapIndex)))  // right corner
      {  // Special case where placing the new wall will generate a corner to the right, so replace
        // the vertical piece with a bottomend.
        sIndex = PickAWallPiece(EXTERIOR_BOTTOMEND);
        AddToUndoList(iMapIndex);
        GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
        ReplaceStructIndex(iMapIndex, pStruct->usIndex, usTileIndex);
      }
      if ((pStruct = GetVerticalWall(iMapIndex + WORLD_COLS - 1)) &&
          !GetVerticalWall(iMapIndex - 1)) {
        sIndex = PickAWallPiece(EXTERIOR_EXTENDED);
        AddToUndoList(iMapIndex + WORLD_COLS - 1);
        GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
        ReplaceStructIndex(iMapIndex + WORLD_COLS - 1, pStruct->usIndex, usTileIndex);
      }
      if (!gfBasement) {
        // All exterior_l walls have shadows.
        GetTileIndexFromTypeSubIndex(usWallType, EXTERIOR_L_SHADOW_INDEX, &usTileIndex);
        AddExclusiveShadow(iMapIndex, usTileIndex);
      }
      break;
    case INTERIOR_LEFT:
      iMapIndex--;
      ubWallClass = INTERIOR_R;
      if (GetHorizontalWall(iMapIndex)) {
        ubWallClass = INTERIOR_BOTTOMEND;
      }
      if (!gfBasement && GetHorizontalWall(iMapIndex + 1)) {
        AddToUndoList(iMapIndex);
        GetTileIndexFromTypeSubIndex(usWallType, INTERIOR_BOTTOMEND_SHADOW_INDEX, &usTileIndex);
        AddExclusiveShadow(iMapIndex, usTileIndex);
      }
      if (GetHorizontalWall(iMapIndex - WORLD_COLS + 1)) {
        if (ubWallClass == INTERIOR_BOTTOMEND)
          ubWallClass = INTERIOR_EXTENDED_BOTTOMEND;
        else
          ubWallClass = INTERIOR_EXTENDED;
      }
      break;
    case INTERIOR_RIGHT:
      ubWallClass = EXTERIOR_R;
      if (GetHorizontalWall(iMapIndex)) {  // Special case where placing the new wall will generate
                                           // a corner.  This piece
        // becomes an exterior bottomend, but nothing else is effected.
        ubWallClass = EXTERIOR_BOTTOMEND;
      }
      if (GetHorizontalWall(iMapIndex - WORLD_COLS + 1)) {
        if (ubWallClass == EXTERIOR_BOTTOMEND)
          ubWallClass = EXTERIOR_EXTENDED_BOTTOMEND;
        else
          ubWallClass = EXTERIOR_EXTENDED;
      }
      if (!gfBasement && GetHorizontalWall(iMapIndex + 1) && !GetHorizontalWall(iMapIndex) &&
          !FloorAtGridNo(iMapIndex + WORLD_COLS)) {
        GetTileIndexFromTypeSubIndex(usWallType, INTERIOR_BOTTOMEND_SHADOW_INDEX, &usTileIndex);
        AddExclusiveShadow(iMapIndex, usTileIndex);
      }
      break;
  }
  sIndex = PickAWallPiece(ubWallClass);
  GetTileIndexFromTypeSubIndex(usWallType, sIndex, &usTileIndex);
  AddToUndoList(iMapIndex);
  AddWallToStructLayer(iMapIndex, usTileIndex, FALSE);
}

void RebuildRoofUsingFloorInfo(int32_t iMapIndex, uint16_t usRoofType) {
  uint16_t usRoofIndex, usTileIndex;
  BOOLEAN fTop = FALSE, fBottom = FALSE, fLeft = FALSE, fRight = FALSE;
  if (!usRoofType) {
    usRoofType = SearchForRoofType(iMapIndex);
  }
  if (usRoofType == 0xffff) return;  // no roof type around, so don't draw one.
  // Analyse the mapindex for walls and set the flags.
  // NOTE:  There is no support for more than 2 side on a roof, so if there is, draw TOPLEFT
  AddToUndoList(iMapIndex);
  EraseRoof(iMapIndex);

  fTop = FloorAtGridNo(iMapIndex - WORLD_COLS) ? FALSE : TRUE;
  fLeft = FloorAtGridNo(iMapIndex - 1) ? FALSE : TRUE;
  fBottom = FloorAtGridNo(iMapIndex + WORLD_COLS) ? FALSE : TRUE;
  fRight = FloorAtGridNo(iMapIndex + 1) ? FALSE : TRUE;
  if (fTop && fLeft)
    usRoofIndex = TOPLEFT_ROOF_INDEX;
  else if (fTop && fRight)
    usRoofIndex = TOPRIGHT_ROOF_INDEX;
  else if (fBottom && fLeft)
    usRoofIndex = BOTTOMLEFT_ROOF_INDEX;
  else if (fBottom && fRight)
    usRoofIndex = BOTTOMRIGHT_ROOF_INDEX;
  else if (fTop)
    usRoofIndex = TOP_ROOF_INDEX;
  else if (fBottom)
    usRoofIndex = BOTTOM_ROOF_INDEX;
  else if (fLeft)
    usRoofIndex = LEFT_ROOF_INDEX;
  else if (fRight)
    usRoofIndex = RIGHT_ROOF_INDEX;
  else
    usRoofIndex = CENTER_ROOF_BASE_INDEX + (rand() % CENTER_ROOF_VARIANTS);
  GetTileIndexFromTypeSubIndex(usRoofType, usRoofIndex, &usTileIndex);
  AddRoofToHead(iMapIndex, usTileIndex);
  // if the editor view roofs is off, then the new roofs need to be hidden.
  if (!fBuildingShowRoofs) {
    HideStructOfGivenType(iMapIndex, usRoofType, TRUE);
  }
}

// Given a gridno, it will erase the current roof, and calculate the new roof piece based on the
// wall orientions giving priority to the top and left walls before anything else.
// NOTE:  passing NULL for usRoofType will force the function to calculate the nearest roof type,
//  and use that for the new roof.  This is needed when erasing parts of multiple buildings
//  simultaneously.
void RebuildRoof(uint32_t iMapIndex, uint16_t usRoofType) {
  uint16_t usRoofIndex, usTileIndex;
  BOOLEAN fTop, fBottom, fLeft, fRight;
  if (!usRoofType) {
    usRoofType = SearchForRoofType(iMapIndex);
  }
  if (usRoofType == 0xffff) return;  // no roof type around, so don't draw one.
  // Analyse the mapindex for walls and set the flags.
  // NOTE:  There is no support for more than 2 side on a roof, so if there is, draw TOPLEFT
  AddToUndoList(iMapIndex);
  EraseRoof(iMapIndex);

  fTop = GetHorizontalWall(iMapIndex - WORLD_COLS) ? TRUE : FALSE;
  fLeft = GetVerticalWall(iMapIndex - 1) ? TRUE : FALSE;
  fBottom = GetHorizontalWall(iMapIndex) ? TRUE : FALSE;
  fRight = GetVerticalWall(iMapIndex) ? TRUE : FALSE;
  if (fTop && fLeft)
    usRoofIndex = TOPLEFT_ROOF_INDEX;
  else if (fTop && fRight)
    usRoofIndex = TOPRIGHT_ROOF_INDEX;
  else if (fBottom && fLeft)
    usRoofIndex = BOTTOMLEFT_ROOF_INDEX;
  else if (fBottom && fRight)
    usRoofIndex = BOTTOMRIGHT_ROOF_INDEX;
  else if (fTop)
    usRoofIndex = TOP_ROOF_INDEX;
  else if (fBottom)
    usRoofIndex = BOTTOM_ROOF_INDEX;
  else if (fLeft)
    usRoofIndex = LEFT_ROOF_INDEX;
  else if (fRight)
    usRoofIndex = RIGHT_ROOF_INDEX;
  else
    usRoofIndex = CENTER_ROOF_BASE_INDEX + (rand() % CENTER_ROOF_VARIANTS);
  GetTileIndexFromTypeSubIndex(usRoofType, usRoofIndex, &usTileIndex);
  AddRoofToHead(iMapIndex, usTileIndex);
  // if the editor view roofs is off, then the new roofs need to be hidden.
  if (!fBuildingShowRoofs) {
    HideStructOfGivenType(iMapIndex, usRoofType, TRUE);
  }
}

void BulldozeNature(uint32_t iMapIndex) {
  AddToUndoList(iMapIndex);
  RemoveAllStructsOfTypeRange(iMapIndex, FIRSTISTRUCT, LASTISTRUCT);
  RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTCLIFFSHADOW, LASTCLIFFSHADOW);
  RemoveAllStructsOfTypeRange(iMapIndex, FIRSTOSTRUCT, LASTOSTRUCT);  // outside objects.
  RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTSHADOW, LASTSHADOW);
  RemoveAllStructsOfTypeRange(iMapIndex, FIRSTROAD, LASTROAD);
  RemoveAllObjectsOfTypeRange(iMapIndex, DEBRISROCKS, LASTDEBRIS);
  RemoveAllObjectsOfTypeRange(iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS);
}

void EraseRoof(uint32_t iMapIndex) {
  AddToUndoList(iMapIndex);
  RemoveAllRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, LASTITEM);
  RemoveAllOnRoofsOfTypeRange(iMapIndex, FIRSTTEXTURE, LASTITEM);
  RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTROOF, LASTSLANTROOF);
}

void EraseFloor(uint32_t iMapIndex) {
  AddToUndoList(iMapIndex);
  RemoveAllLandsOfTypeRange(iMapIndex, FIRSTFLOOR, LASTFLOOR);
}

void EraseWalls(uint32_t iMapIndex) {
  AddToUndoList(iMapIndex);
  RemoveAllStructsOfTypeRange(iMapIndex, FIRSTTEXTURE, LASTITEM);
  RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
  RemoveAllShadowsOfTypeRange(iMapIndex, FIRSTDOORSHADOW, LASTDOORSHADOW);
  RemoveAllObjectsOfTypeRange(iMapIndex, DEBRISROCKS, LASTDEBRIS);
  RemoveAllTopmostsOfTypeRange(iMapIndex, WIREFRAMES, WIREFRAMES);
  RemoveAllObjectsOfTypeRange(iMapIndex, DEBRIS2MISC, DEBRIS2MISC);
  RemoveAllObjectsOfTypeRange(iMapIndex, ANOTHERDEBRIS, ANOTHERDEBRIS);
}

void EraseBuilding(uint32_t iMapIndex) {
  EraseRoof(iMapIndex);
  EraseFloor(iMapIndex);
  EraseWalls(iMapIndex);
  gubWorldRoomInfo[iMapIndex] = 0;
}

// Specialized function that will delete only the TOP_RIGHT oriented wall in the gridno to the left
// and the TOP_LEFT oriented wall in the gridno up one as well as the other building information at
// this gridno.
void EraseFloorOwnedBuildingPieces(uint32_t iMapIndex) {
  struct LEVELNODE *pStruct = NULL;
  uint32_t uiTileType;
  uint16_t usWallOrientation;

  if (!gfBasement &&
      !FloorAtGridNo(iMapIndex)) {  // We don't have ownership issues if there isn't a floor here.
    return;
  }
  EraseBuilding(iMapIndex);
  // FIRST, SEARCH AND DESTROY FOR A LEFT NEIGHBORING TILE WITH A TOP_RIGHT ORIENTED WALL
  pStruct = gpWorldLevelData[iMapIndex - 1].pStructHead;
  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &uiTileType);
      if ((uiTileType >= FIRSTWALL && uiTileType <= LASTWALL) ||
          (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)) {
        GetWallOrientation(pStruct->usIndex, &usWallOrientation);
        if (usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) {
          AddToUndoList(iMapIndex - 1);
          RemoveStruct(iMapIndex - 1, pStruct->usIndex);
          RemoveAllShadowsOfTypeRange(iMapIndex - 1, FIRSTWALL, LASTWALL);
          break;  // otherwise, it'll crash because pStruct is toast.
        }
      }
    }
    pStruct = pStruct->pNext;
  }
  // FINALLY, SEARCH AND DESTROY FOR A TOP NEIGHBORING TILE WITH A TOP_LEFT ORIENTED WALL
  pStruct = gpWorldLevelData[iMapIndex - WORLD_COLS].pStructHead;
  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE) {
      GetTileType(pStruct->usIndex, &uiTileType);
      if ((uiTileType >= FIRSTWALL && uiTileType <= LASTWALL) ||
          (uiTileType >= FIRSTDOOR && uiTileType <= LASTDOOR)) {
        GetWallOrientation(pStruct->usIndex, &usWallOrientation);
        if (usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) {
          AddToUndoList(iMapIndex - WORLD_COLS);
          RemoveStruct(iMapIndex - WORLD_COLS, pStruct->usIndex);
          RemoveAllShadowsOfTypeRange(iMapIndex - WORLD_COLS, FIRSTWALL, LASTWALL);
          break;  // otherwise, it'll crash because pStruct is toast.
        }
      }
    }
    pStruct = pStruct->pNext;
  }
}

/*
BOOLEAN CaveAtGridNo( int32_t iMapIndex );
uint16_t GetCaveTileIndexFromPerimeterValue( uint8_t ubTotal );
uint8_t CalcNewCavePerimeterValue( int32_t iMapIndex );
void AddCave( int32_t iMapIndex, uint16_t usIndex );
*/

void RemoveCaveSectionFromWorld(SGPRect *pSelectRegion) {
  uint32_t top, left, right, bottom, x, y;
  uint32_t iMapIndex;
  uint16_t usIndex;
  uint8_t ubPerimeterValue;
  top = pSelectRegion->iTop;
  left = pSelectRegion->iLeft;
  right = pSelectRegion->iRight;
  bottom = pSelectRegion->iBottom;
  // Pass 1:  Remove all pieces in area
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      AddToUndoList(iMapIndex);
      RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
    }
  // Past 2:  Go around outside perimeter and smooth each piece
  for (y = top - 1; y <= bottom + 1; y++)
    for (x = left - 1; x <= right + 1; x++) {
      iMapIndex = y * WORLD_COLS + x;
      if (CaveAtGridNo(iMapIndex)) {
        ubPerimeterValue = CalcNewCavePerimeterValue(iMapIndex);
        usIndex = GetCaveTileIndexFromPerimeterValue(ubPerimeterValue);
        AddToUndoList(iMapIndex);
        if (usIndex != 0xffff)
          AddCave(iMapIndex, usIndex);
        else {  // change piece to stalagmite...
          RemoveAllStructsOfTypeRange(iMapIndex, FIRSTWALL, LASTWALL);
        }
      }
    }
}

void AddCaveSectionToWorld(SGPRect *pSelectRegion) {
  int32_t top, left, right, bottom, x, y;
  uint32_t uiMapIndex;
  uint16_t usIndex;
  uint8_t ubPerimeterValue;
  top = pSelectRegion->iTop;
  left = pSelectRegion->iLeft;
  right = pSelectRegion->iRight;
  bottom = pSelectRegion->iBottom;
  // Pass 1:  Add bogus piece to each gridno in region
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      uiMapIndex = y * WORLD_COLS + x;
      if (uiMapIndex < NOWHERE) {
        usIndex = GetCaveTileIndexFromPerimeterValue(0xff);
        AddToUndoList(uiMapIndex);
        AddCave(uiMapIndex, usIndex);
      }
    }
  // Past 2:  Go around outside perimeter and smooth each piece
  for (y = top - 1; y <= bottom + 1; y++)
    for (x = left - 1; x <= right + 1; x++) {
      uiMapIndex = y * WORLD_COLS + x;
      if (uiMapIndex < NOWHERE) {
        if (CaveAtGridNo(uiMapIndex)) {
          ubPerimeterValue = CalcNewCavePerimeterValue(uiMapIndex);
          usIndex = GetCaveTileIndexFromPerimeterValue(ubPerimeterValue);
          AddToUndoList(uiMapIndex);
          if (usIndex != 0xffff)
            AddCave(uiMapIndex, usIndex);
          else {  // change piece to stalagmite...
            RemoveAllStructsOfTypeRange(uiMapIndex, FIRSTWALL, LASTWALL);
          }
        }
      }
    }
}

//--------------------------------------------------------------------------------------------------
// END OF PRIVATE FUNCTION IMPLEMENTATION
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// BEGIN PUBLIC FUNCTION IMPLEMENTATION
//--------------------------------------------------------------------------------------------------

// When the user removes a section from a building, it will not only erase the
// entire highlighted area, it'll repair the building itself so there are no
// outside walls missing from the new building.
void RemoveBuildingSectionFromWorld(SGPRect *pSelectRegion) {
  uint32_t top, left, right, bottom, x, y;
  uint32_t iMapIndex;
  uint16_t usTileIndex;
  uint16_t usFloorType;
  BOOLEAN fFloor;

  top = pSelectRegion->iTop;
  left = pSelectRegion->iLeft;
  right = pSelectRegion->iRight;
  bottom = pSelectRegion->iBottom;

  // 1ST PASS:  Erase all building owned by the floor tile if there is one.
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      EraseFloorOwnedBuildingPieces(
          iMapIndex);  // Erase possible top and left walls in bordering tiles.
    }
  // 2ND PASS:  Build new walls whereever there are neighboring floor tiles.
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      // NOTE:  Top and bottom walls MUST be placed first -- it minimizes the number of special
      // cases.
      if (y == top) {
        fFloor = FloorAtGridNo(iMapIndex - WORLD_COLS);
        if ((gfBasement && !fFloor) ||
            (!gfBasement && fFloor && !GetHorizontalWall(iMapIndex - WORLD_COLS)))
          BuildWallPiece(iMapIndex, EXTERIOR_TOP, 0);
      }
      if (y == bottom) {
        fFloor = FloorAtGridNo(iMapIndex + WORLD_COLS);
        if ((gfBasement && !fFloor) || (!gfBasement && fFloor && !GetHorizontalWall(iMapIndex)))
          BuildWallPiece(iMapIndex, EXTERIOR_BOTTOM, 0);
      }
      if (x == left) {
        fFloor = FloorAtGridNo(iMapIndex - 1);
        if ((gfBasement && !fFloor) || (!gfBasement && fFloor && !GetVerticalWall(iMapIndex - 1)))
          BuildWallPiece(iMapIndex, EXTERIOR_LEFT, 0);
      }
      if (x == right) {
        fFloor = FloorAtGridNo(iMapIndex + 1);
        if ((gfBasement && !fFloor) || (!gfBasement && fFloor && !GetVerticalWall(iMapIndex)))
          BuildWallPiece(iMapIndex, EXTERIOR_RIGHT, 0);
      }
    }
  // 3RD PASS:  Go around the outside of the region, and rebuild the roof.
  if (gfBasement) {
    usFloorType = GetRandomIndexByRange(FIRSTFLOOR, LASTFLOOR);
    if (usFloorType == 0xffff) usFloorType = FIRSTFLOOR;
    for (y = top; y <= bottom; y++)
      for (x = left; x <= right; x++) {
        iMapIndex = y * WORLD_COLS + x;
        GetTileIndexFromTypeSubIndex(usFloorType, 1, &usTileIndex);
        AddLandToHead(iMapIndex, (uint16_t)(usTileIndex + Random(FLOOR_VARIANTS)));
      }
  }
  for (y = top - 1; y <= bottom + 1; y++)
    for (x = left - 1; x <= right + 1; x++) {
      iMapIndex = y * WORLD_COLS + x;
      if (y == top - 1 || y == bottom + 1 || x == left - 1 || x == right + 1) {
        if ((!gfBasement && FloorAtGridNo(iMapIndex)) ||
            (gfBasement && !FloorAtGridNo(iMapIndex))) {
          RebuildRoof(iMapIndex, 0);
        }
      }
    }
}

void AddBuildingSectionToWorld(SGPRect *pSelectRegion) {
  int32_t top, left, right, bottom, x, y;
  uint32_t iMapIndex;
  uint16_t usFloorType, usWallType, usRoofType;
  uint16_t usTileIndex;
  BOOLEAN fNewBuilding;
  BOOLEAN fSlantRoof = FALSE;
  BOOLEAN fVertical;
  BOOLEAN fFloor;
  top = pSelectRegion->iTop;
  left = pSelectRegion->iLeft;
  right = pSelectRegion->iRight;
  bottom = pSelectRegion->iBottom;

  // Special case scenario:
  // If the user selects a floor without walls, then it is implied that the user wishes to
  // change the floor for say a kitchen which might have a different floor type.
  usWallType = GetRandomIndexByRange(FIRSTWALL, LASTWALL);
  usFloorType = GetRandomIndexByRange(FIRSTFLOOR, LASTFLOOR);
  if (usWallType == 0xffff && usFloorType != 0xffff) {  // allow user to place floors
    for (y = top; y <= bottom; y++)
      for (x = left; x <= right; x++) {
        iMapIndex = y * WORLD_COLS + x;
        EraseFloor(iMapIndex);
        GetTileIndexFromTypeSubIndex(usFloorType, 1, &usTileIndex);
        AddLandToHead(iMapIndex, (uint16_t)(usTileIndex + Random(FLOOR_VARIANTS)));
      }
    // we are done!
    return;
  }

  // 1ST PASS:  Determine if there are any floor tiles in this region.  If there are, then
  //  that signifies that we are concantenating this building to an existing one.  Otherwise,
  //  we are just drawing an individual building.  If we find a floor, extract the type so
  //  we know how to draw it later.
  fNewBuilding = TRUE;
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      if (FloorAtGridNo(iMapIndex)) {
        struct LEVELNODE *pFloor;
        uint32_t uiTileType;
        // If a floor is found, then we are adding to an existing structure.
        fNewBuilding = FALSE;
        // Extract the floor type.  We already checked if there was a floor here, so it is assumed.
        pFloor = gpWorldLevelData[iMapIndex].pLandHead;
        while (pFloor) {
          GetTileType(pFloor->usIndex, &uiTileType);
          if (uiTileType >= FIRSTFLOOR && uiTileType <= LASTFLOOR) {
            usFloorType = (uint16_t)uiTileType;
            break;
          }
        }
        usWallType = SearchForWallType(iMapIndex);
        usRoofType = SearchForRoofType(iMapIndex);
        if (usWallType != 0xffff && usRoofType != 0xffff &&
            usFloorType !=
                0xffff) {  // we have extracted all of the information we need, so we can break out.
          y = bottom;
          break;
        }
      }
    }

  if (fNewBuilding) {
    // if( gfBasement )
    //	return;
    // Get materials via selection window method.
    usWallType = GetRandomIndexByRange(FIRSTWALL, LASTWALL);
    usFloorType = GetRandomIndexByRange(FIRSTFLOOR, LASTFLOOR);
    usRoofType = GetRandomIndexByRange(FIRSTROOF, LASTROOF);
    if (usRoofType == 0xffff) {
      usRoofType = GetRandomIndexByRange(FIRSTSLANTROOF, LASTSLANTROOF);
      if (usRoofType != 0xffff) {
        if (!gfBasement)
          fSlantRoof = TRUE;
        else
          usRoofType = FIRSTROOF;
      }
    }
    if (usWallType == 0xffff) return;
  }

  // 2ND PASS:  Remove all walls in the region that border no floor tile, or simply walls
  //  that are considered exterior walls.  That way, it won't wreck the inside of a building
  //  if you select too much interior.  Also, gridnos that delete walls will also delete the
  //  floor and roof tiles there.  That signifies that the floorless parts will be resmoothed,
  //  and rebuilt in the third pass.
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      if (gfBasement) {
        EraseBuilding(iMapIndex);
      } else if (FloorAtGridNo(iMapIndex) && !fNewBuilding) {
        if (y >= top && !FloorAtGridNo(iMapIndex - WORLD_COLS)) {
          EraseHorizontalWall(iMapIndex - WORLD_COLS);
          EraseFloor(iMapIndex);
          EraseRoof(iMapIndex);
        }
        if (y <= bottom && !FloorAtGridNo(iMapIndex + WORLD_COLS)) {
          EraseHorizontalWall(iMapIndex);
          EraseFloor(iMapIndex);
          EraseRoof(iMapIndex);
        }
        if (x >= left && !FloorAtGridNo(iMapIndex - 1)) {
          EraseVerticalWall(iMapIndex - 1);
          EraseFloor(iMapIndex);
          EraseRoof(iMapIndex);
        }
        if (x <= right && !FloorAtGridNo(iMapIndex + 1)) {
          EraseVerticalWall(iMapIndex);
          EraseFloor(iMapIndex);
          EraseRoof(iMapIndex);
        }
      } else  // we will be building outside of this structure, so bulldoze the nature -- trees,
              // rocks, etc.
      {
        BulldozeNature(iMapIndex);
      }
    }
  // 3RD PASS:  Process the region, and all walls of floorless tiles are rebuilt from interior
  // perspective.
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      if (!FloorAtGridNo(iMapIndex)) {
        if (y == top && !GetHorizontalWall(iMapIndex - WORLD_COLS)) {
          fFloor = FloorAtGridNo(iMapIndex - WORLD_COLS);
          if (gfBasement == fFloor) BuildWallPiece(iMapIndex, INTERIOR_TOP, usWallType);
        }
        if (y == bottom && !GetHorizontalWall(iMapIndex)) {
          fFloor = FloorAtGridNo(iMapIndex + WORLD_COLS);
          if (gfBasement == fFloor) BuildWallPiece(iMapIndex, INTERIOR_BOTTOM, usWallType);
        }
        if (x == left && !GetVerticalWall(iMapIndex - 1)) {
          fFloor = FloorAtGridNo(iMapIndex - 1);
          if (gfBasement == fFloor) BuildWallPiece(iMapIndex, INTERIOR_LEFT, usWallType);
        }
        if (x == right && !GetVerticalWall(iMapIndex)) {
          fFloor = FloorAtGridNo(iMapIndex + 1);
          if (gfBasement == fFloor) BuildWallPiece(iMapIndex, INTERIOR_RIGHT, usWallType);
        }
      }
    }

  // If we are dealing with slant roofs then build the whole thing now.
  // Slant roofs always have a width or height of 8 tiles.
  if (fSlantRoof) {
    fVertical = (bottom - top == 7) ? FALSE : TRUE;
    BuildSlantRoof(left, top, right, bottom, usWallType, usRoofType, fVertical);
  }

  // 4TH PASS:  Process the region, and all floorless tiles get new roofs and floors.
  for (y = top; y <= bottom; y++)
    for (x = left; x <= right; x++) {
      iMapIndex = y * WORLD_COLS + x;
      if (!FloorAtGridNo(iMapIndex)) {
        if (!fSlantRoof) RebuildRoof(iMapIndex, usRoofType);
        if (usFloorType != 0xffff && !gfBasement) {
          GetTileIndexFromTypeSubIndex(usFloorType, 1, &usTileIndex);
          AddLandToHead(iMapIndex, (uint16_t)(usTileIndex + Random(FLOOR_VARIANTS)));
        }
      }
    }
}

void AnalyseCaveMapForStructureInfo() {
  struct LEVELNODE *pStruct;
  uint32_t uiTileType;
  int32_t iMapIndex;
  for (iMapIndex = 0; iMapIndex < WORLD_MAX; iMapIndex++) {
    pStruct = gpWorldLevelData[iMapIndex].pStructHead;
    while (pStruct) {
      if (pStruct->usIndex != NO_TILE) {
        GetTileType(pStruct->usIndex, &uiTileType);
        if (uiTileType == FIRSTWALL) {
          uint16_t usSubIndex;
          GetSubIndexFromTileIndex(pStruct->usIndex, &usSubIndex);
          if (usSubIndex >= 60 && usSubIndex <= 65) {
            pStruct->uiFlags |= LEVELNODE_CAVE;
          }
        }
      }
      pStruct = pStruct->pNext;
    }
  }
}
