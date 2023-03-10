#ifndef __SMOOTHING_UTILS_H
#define __SMOOTHING_UTILS_H

#include "BuildDefines.h"
#include "SGP/Types.h"

// Use these values when specifically replacing a wall with new type.
enum {                 // Wall tile types
  INTERIOR_L,          // interior wall with a top left orientation
  INTERIOR_R,          // interior wall with a top right orientation
  EXTERIOR_L,          // exterior wall with a top left orientation
  EXTERIOR_R,          // exterior wall with a top right orientation
  INTERIOR_CORNER,     // special interior end piece with top left orientation.
                       // The rest of these walls are special wall tiles for top right orientations.
  INTERIOR_BOTTOMEND,  // interior wall for bottom corner
  EXTERIOR_BOTTOMEND,  // exterior wall for bottom corner
  INTERIOR_EXTENDED,   // extended interior wall for top corner
  EXTERIOR_EXTENDED,   // extended exterior wall for top corner
  INTERIOR_EXTENDED_BOTTOMEND,  // extended interior wall for both top and bottom corners.
  EXTERIOR_EXTENDED_BOTTOMEND,  // extended exterior wall for both top and bottom corners.
  NUM_WALL_TYPES
};

// Use these values when passing a ubWallPiece to BuildWallPieces.
enum {
  NO_WALLS,
  INTERIOR_TOP,
  INTERIOR_BOTTOM,
  INTERIOR_LEFT,
  INTERIOR_RIGHT,
  EXTERIOR_TOP,
  EXTERIOR_BOTTOM,
  EXTERIOR_LEFT,
  EXTERIOR_RIGHT,
};

// in newsmooth.c
extern INT8 gbWallTileLUT[NUM_WALL_TYPES][7];
extern void EraseWalls(UINT32 iMapIndex);
extern void BuildWallPiece(UINT32 iMapIndex, UINT8 ubWallPiece, UINT16 usWallType);
// in Smoothing Utils
void RestoreWalls(UINT32 iMapIndex);
UINT16 SearchForRoofType(UINT32 iMapIndex);
UINT16 SearchForWallType(UINT32 iMapIndex);
BOOLEAN RoofAtGridNo(UINT32 iMapIndex);
BOOLEAN BuildingAtGridNo(UINT32 iMapIndex);
struct LEVELNODE* GetHorizontalWall(UINT32 iMapIndex);
struct LEVELNODE* GetVerticalWall(UINT32 iMapIndex);
struct LEVELNODE* GetVerticalFence(UINT32 iMapIndex);
struct LEVELNODE* GetHorizontalFence(UINT32 iMapIndex);
UINT16 GetHorizontalWallType(UINT32 iMapIndex);
UINT16 GetVerticalWallType(UINT32 iMapIndex);
void EraseHorizontalWall(UINT32 iMapIndex);
void EraseVerticalWall(UINT32 iMapIndex);
void ChangeHorizontalWall(UINT32 iMapIndex, UINT16 usNewPiece);
void ChangeVerticalWall(UINT32 iMapIndex, UINT16 usNewPiece);
UINT16 GetWallClass(struct LEVELNODE* pWall);
UINT16 GetVerticalWallClass(UINT16 iMapIndex);
UINT16 GetHorizontalWallClass(UINT16 iMapIndex);
BOOLEAN ValidDecalPlacement(UINT32 iMapIndex);

#endif
