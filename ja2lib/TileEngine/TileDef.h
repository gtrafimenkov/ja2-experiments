// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __TILEDEF_H
#define __TILEDEF_H

#include "TileEngine/TileDat.h"

struct AuxObjectData;
struct VObject;

// CATEGORY TYPES
#define NO_TILE 64000
#define ERASE_TILE 65000
#define REQUIRES_SMOOTHING_TILE 19
#define NUM_WALL_ORIENTATIONS 40

#define WALL_TILE 0x00000001
#define ANIMATED_TILE 0x00000002
#define DYNAMIC_TILE 0x00000004
#define IGNORE_WORLD_HEIGHT 0x00000008
#define ROAD_TILE 0x00000010
#define FULL3D_TILE 0x00000020
#define MULTI_Z_TILE 0x00000080
#define OBJECTLAYER_USEZHEIGHT 0x00000100
#define ROOFSHADOW_TILE 0x00000200
#define ROOF_TILE 0x00000400
#define TRANSLUCENT_TILE 0x00000800
#define HAS_SHADOW_BUDDY 0x00001000
#define AFRAME_TILE 0x00002000
#define HIDDEN_TILE 0x00004000
#define CLIFFHANG_TILE 0x00008000
#define UNDERFLOW_FILLER 0x00010000

#define MAX_ANIMATED_TILES 200
#define WALL_HEIGHT 50

// Kris:  Added the last two bottom corner orientation values.  This won't effect
// current code, but there is new code that makes use of this.  A function called
// uint8_t CalculateWallOrientationsAtGridNo( int32_t iMapIndex ) that will look at all
// of the walls and return the last two wall orientations for tiles with two proper
// wall pieces.
enum WallOrientationDefines {
  NO_ORIENTATION,
  INSIDE_TOP_LEFT,
  INSIDE_TOP_RIGHT,
  OUTSIDE_TOP_LEFT,
  OUTSIDE_TOP_RIGHT,
  INSIDE_BOTTOM_CORNER,
  OUTSIDE_BOTTOM_CORNER
};

// TERRAIN ID VALUES.
typedef enum {
  NO_TERRAIN,
  FLAT_GROUND,
  FLAT_FLOOR,
  PAVED_ROAD,
  DIRT_ROAD,
  LOW_GRASS,
  HIGH_GRASS,
  TRAIN_TRACKS,
  LOW_WATER,
  MED_WATER,
  DEEP_WATER,
  NUM_TERRAIN_TYPES

} TerrainTypeDefines;

// These structures are placed in a list and used for all tile imagery
struct TILE_IMAGERY {
  struct VObject *vo;
  uint32_t fType;
  struct AuxObjectData *pAuxData;
  struct RelTileLoc *pTileLocData;
  struct STRUCTURE_FILE_REF *pStructureFileRef;
  uint8_t ubTerrainID;
  uint8_t bRaisedObjectType;

  // Reserved for added room and 32-byte boundaries
  uint8_t bReserved[2];
};

typedef struct {
  uint16_t *pusFrames;
  int8_t bCurrentFrame;
  uint8_t ubNumFrames;

} TILE_ANIMATION_DATA;

// Tile data element
typedef struct {
  uint16_t fType;
  struct VObject *hTileSurface;
  struct DB_STRUCTURE_REF *pDBStructureRef;
  uint32_t uiFlags;
  struct RelTileLoc *pTileLocData;
  uint16_t usRegionIndex;
  int16_t sBuddyNum;
  uint8_t ubTerrainID;
  uint8_t ubNumberOfTiles;

  uint8_t bZOffsetX;
  uint8_t bZOffsetY;

  // This union contains different data based on tile type
  union {
    // Land and overlay type
    struct {
      int16_t sOffsetHeight;
      uint16_t usWallOrientation;
      uint8_t ubFullTile;

      // For animated tiles
      TILE_ANIMATION_DATA *pAnimData;
    };
  };

  // Reserved for added room and 32-byte boundaries
  uint8_t bReserved[3];

} TILE_ELEMENT, *PTILE_ELEMENT;

typedef struct {
  int32_t iMapIndex;
  uint8_t ubNumLayers;
  uint16_t *pIndexValues;

} land_undo_struct;

// Globals used
extern TILE_ELEMENT gTileDatabase[NUMBEROFTILES];
extern uint16_t gTileDatabaseSize;
extern uint8_t gFullBaseTileValues[];
extern uint16_t gNumTilesPerType[NUMBEROFTILETYPES];
extern uint16_t gTileTypeStartIndex[NUMBEROFTILETYPES];
extern char *gTileSurfaceName[NUMBEROFTILETYPES];
extern uint8_t gTileTypeLogicalHeight[NUMBEROFTILETYPES];

extern uint16_t gusNumAnimatedTiles;
extern uint16_t gusAnimatedTiles[MAX_ANIMATED_TILES];
extern uint8_t gTileTypeMovementCost[NUM_TERRAIN_TYPES];

void CreateTileDatabase();

// Land level manipulation functions
BOOLEAN GetLandHeadType(int32_t iMapIndex, uint32_t *puiType);

BOOLEAN SetLandIndex(int32_t iMapIndex, uint16_t usIndex, uint32_t uiNewType, BOOLEAN fDelete);

BOOLEAN GetTypeLandLevel(uint32_t iMapIndex, uint32_t uiNewType, uint8_t *pubLevel);
uint8_t GetLandLevelDepth(uint32_t iMapIndex);

BOOLEAN SetLandIndexWithRadius(int32_t iMapIndex, uint16_t usIndex, uint32_t uiNewType,
                               uint8_t ubRadius, BOOLEAN fReplace);

BOOLEAN LandTypeHeigher(uint32_t uiDestType, uint32_t uiSrcType);

BOOLEAN MoveLandIndexToTop(uint32_t iMapIndex, uint16_t usIndex);

// Database access functions
BOOLEAN GetSubIndexFromTileIndex(uint16_t usIndex, uint16_t *pusSubIndex);
BOOLEAN GetTypeSubIndexFromTileIndex(uint32_t uiCheckType, uint16_t usIndex, uint16_t *pusSubIndex);
BOOLEAN GetTypeSubIndexFromTileIndexChar(uint32_t uiCheckType, uint16_t usIndex,
                                         uint8_t *pusSubIndex);
BOOLEAN GetTileIndexFromTypeSubIndex(uint32_t uiCheckType, uint16_t usSubIndex,
                                     uint16_t *pusTileIndex);
BOOLEAN GetTileType(uint16_t usIndex, uint32_t *puiType);
BOOLEAN GetTileFlags(uint16_t usIndex, uint32_t *puiFlags);

BOOLEAN GetTileTypeLogicalHeight(uint32_t fType, uint8_t *pubLogHeight);
BOOLEAN AnyHeigherLand(uint32_t iMapIndex, uint32_t uiSrcType, uint8_t *pubLastLevel);
BOOLEAN AnyLowerLand(uint32_t iMapIndex, uint32_t uiSrcType, uint8_t *pubLastLevel);
BOOLEAN GetWallOrientation(uint16_t usIndex, uint16_t *pusWallOrientation);
BOOLEAN ContainsWallOrientation(int32_t iMapIndex, uint32_t uiType, uint16_t usWallOrientation,
                                uint8_t *pubLevel);
uint8_t CalculateWallOrientationsAtGridNo(int32_t iMapIndex);

void SetSpecificDatabaseValues(uint16_t usType, uint16_t uiDatabaseElem, TILE_ELEMENT *TileElement,
                               BOOLEAN fUseRaisedObjectType);

BOOLEAN AllocateAnimTileData(TILE_ELEMENT *pTileElem, uint8_t ubNumFrames);
void FreeAnimTileData(TILE_ELEMENT *pTileElem);
void DeallocateTileDatabase();

#endif
