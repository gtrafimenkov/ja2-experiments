#ifndef __TILE_CACHE_H
#define __TILE_CACHE_H

#include "TileEngine/TileDef.h"
#include "TileEngine/Structure.h"

#define TILE_CACHE_START_INDEX 36000

typedef struct {
  CHAR8 zName[128];        // Name of tile ( filename and directory here )
  CHAR8 zRootName[30];     // Root name
  TILE_IMAGERY *pImagery;  // Tile imagery
  INT16 sHits;
  UINT8 ubNumFrames;
  INT16 sStructRefID;

} TILE_CACHE_ELEMENT;

typedef struct {
  CHAR8 Filename[150];
  CHAR8 zRootName[30];  // Root name
  STRUCTURE_FILE_REF *pStructureFileRef;

} TILE_CACHE_STRUCT;

extern TILE_CACHE_ELEMENT *gpTileCache;

BOOLEAN InitTileCache();
void DeleteTileCache();

INT32 GetCachedTile(STR cFilename);
BOOLEAN RemoveCachedTile(INT32 iCachedTile);

STRUCTURE_FILE_REF *GetCachedTileStructureRefFromFilename(STR8 cFilename);

HVOBJECT GetCachedTileVideoObject(INT32 iIndex);
STRUCTURE_FILE_REF *GetCachedTileStructureRef(INT32 iIndex);
void CheckForAndAddTileCacheStructInfo(LEVELNODE *pNode, INT16 sGridNo, UINT16 usIndex,
                                       UINT16 usSubIndex);
void CheckForAndDeleteTileCacheStructInfo(LEVELNODE *pNode, UINT16 usIndex);
void GetRootName(STR8 pDestStr, STR8 pSrcStr);

// OF COURSE, FOR SPEED, WE EXPORT OUR ARRAY
// ACCESS FUNCTIONS IN RENDERER IS NOT TOO NICE
// ATE

#endif
