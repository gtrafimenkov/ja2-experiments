// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/RenderWorld.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "Editor/EditSys.h"
#include "GameSettings.h"
#include "Globals.h"
#include "JAScreens.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Line.h"
#include "SGP/Shading.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/HandleItems.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/LOS.h"
#include "Tactical/Overhead.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/Weapons.h"
#include "Tactical/WorldItems.h"
#include "TacticalAI/AI.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderZ.c"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"
#include "Utils/SoundControl.h"
#include "Utils/TimerControl.h"

#ifdef __GCC
// since some of the code is not complied on Linux
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

BOOLEAN fLandLayerDirty = TRUE;

extern int16_t gsVIEWPORT_START_X;
extern int16_t gsVIEWPORT_START_Y;
extern int16_t gsVIEWPORT_END_Y;
extern int16_t gsVIEWPORT_WINDOW_END_Y;
extern int16_t gsVIEWPORT_WINDOW_START_Y;
extern int16_t gsVIEWPORT_END_X;

uint16_t *gpZBuffer = NULL;
BOOLEAN gfTagAnimatedTiles = TRUE;

int16_t gsCurrentGlowFrame = 0;
int16_t gsCurrentItemGlowFrame = 0;

int16_t gCenterWorldX, gCenterWorldY;

extern BOOLEAN gfUIShowExitEast;
extern BOOLEAN gfUIShowExitWest;
extern BOOLEAN gfUIShowExitNorth;
extern BOOLEAN gfUIShowExitSouth;

extern BOOLEAN gfTopMessageDirty;

// VIEWPORT OFFSET VALUES
// NOTE:
// THESE VALUES MUST BE MULTIPLES OF TILE SIZES!
#define VIEWPORT_XOFFSET_S WORLD_TILE_X * 1
#define VIEWPORT_YOFFSET_S WORLD_TILE_Y * 2
#define LARGER_VIEWPORT_XOFFSET_S (VIEWPORT_XOFFSET_S * 3)
#define LARGER_VIEWPORT_YOFFSET_S (VIEWPORT_YOFFSET_S * 5)

#define TILES_TYPE_BITMASK 0x00040000

#define TILES_TYPE_MASK 0x07ffffff

#define TILES_DIRTY 0x80000000
#define TILES_DYNAMIC 0x40000000
#define TILES_NOZWRITE 0x20000000
#define TILES_MARKED 0x10000000
#define TILES_NOZ 0x04000000
#define TILES_DOALL 0x02000000
#define TILES_OBSCURED 0x01000000

#define MAX_RENDERED_ITEMS 3

// RENDERER FLAGS FOR DIFFERENT RENDER LEVELS
enum {
  RENDER_STATIC_LAND,
  RENDER_STATIC_OBJECTS,
  RENDER_STATIC_SHADOWS,
  RENDER_STATIC_STRUCTS,
  RENDER_STATIC_ROOF,
  RENDER_STATIC_ONROOF,
  RENDER_STATIC_TOPMOST,
  RENDER_DYNAMIC_LAND,
  RENDER_DYNAMIC_OBJECTS,
  RENDER_DYNAMIC_SHADOWS,
  RENDER_DYNAMIC_STRUCT_MERCS,
  RENDER_DYNAMIC_MERCS,
  RENDER_DYNAMIC_STRUCTS,
  RENDER_DYNAMIC_ROOF,
  RENDER_DYNAMIC_HIGHMERCS,
  RENDER_DYNAMIC_ONROOF,
  RENDER_DYNAMIC_TOPMOST,
  NUM_RENDER_FX_TYPES

};

#define SCROLL_INTERTIA_STEP1 6
#define SCROLL_INTERTIA_STEP2 8

// #define SHORT_ROUND( x ) ( (int16_t)( ( x * 1000 ) / 1000 ) )
#define SHORT_ROUND(x) (x)

#define NUM_ITEM_CYCLE_COLORS 60

uint16_t us16BPPItemCycleWhiteColors[NUM_ITEM_CYCLE_COLORS];
uint16_t us16BPPItemCycleRedColors[NUM_ITEM_CYCLE_COLORS];
uint16_t us16BPPItemCycleYellowColors[NUM_ITEM_CYCLE_COLORS];

int16_t gsLobOutline;
int16_t gsThrowOutline;
int16_t gsGiveOutline;
int16_t gusNormalItemOutlineColor;
int16_t gusYellowItemOutlineColor;

int16_t gsRenderHeight = 0;
BOOLEAN gfRenderFullThisFrame = 0;

// uint8_t		gubIntTileCheckFlags	 = INTILE_CHECK_FULL;
uint8_t gubIntTileCheckFlags = INTILE_CHECK_SELECTIVE;

uint8_t ubRGBItemCycleWhiteColors[] = {
    25,  25,  25,  50,  50,  50,  75,  75,  75,  100, 100, 100, 125, 125, 125, 150,
    150, 150, 175, 175, 175, 200, 200, 200, 225, 225, 225, 250, 250, 250,

    250, 250, 250, 225, 225, 225, 200, 200, 200, 175, 175, 175, 150, 150, 150, 125,
    125, 125, 100, 100, 100, 75,  75,  75,  50,  50,  50,  25,  25,  25,

    25,  25,  25,  50,  50,  50,  75,  75,  75,  100, 100, 100, 125, 125, 125, 150,
    150, 150, 175, 175, 175, 200, 200, 200, 225, 225, 225, 250, 250, 250,

    250, 250, 250, 225, 225, 225, 200, 200, 200, 175, 175, 175, 150, 150, 150, 125,
    125, 125, 100, 100, 100, 75,  75,  75,  50,  50,  50,  25,  25,  25,

    25,  25,  25,  50,  50,  50,  75,  75,  75,  100, 100, 100, 125, 125, 125, 150,
    150, 150, 175, 175, 175, 200, 200, 200, 225, 225, 225, 250, 250, 250,

    250, 250, 250, 225, 225, 225, 200, 200, 200, 175, 175, 175, 150, 150, 150, 125,
    125, 125, 100, 100, 100, 75,  75,  75,  50,  50,  50,  25,  25,  25

};

uint8_t ubRGBItemCycleRedColors[] = {
    25,  0, 0, 50,  0, 0, 75,  0, 0, 100, 0, 0, 125, 0, 0,
    150, 0, 0, 175, 0, 0, 200, 0, 0, 225, 0, 0, 250, 0, 0,

    250, 0, 0, 225, 0, 0, 200, 0, 0, 175, 0, 0, 150, 0, 0,
    125, 0, 0, 100, 0, 0, 75,  0, 0, 50,  0, 0, 25,  0, 0,

    25,  0, 0, 50,  0, 0, 75,  0, 0, 100, 0, 0, 125, 0, 0,
    150, 0, 0, 175, 0, 0, 200, 0, 0, 225, 0, 0, 250, 0, 0,

    250, 0, 0, 225, 0, 0, 200, 0, 0, 175, 0, 0, 150, 0, 0,
    125, 0, 0, 100, 0, 0, 75,  0, 0, 50,  0, 0, 25,  0, 0,

    25,  0, 0, 50,  0, 0, 75,  0, 0, 100, 0, 0, 125, 0, 0,
    150, 0, 0, 175, 0, 0, 200, 0, 0, 225, 0, 0, 250, 0, 0,

    250, 0, 0, 225, 0, 0, 200, 0, 0, 175, 0, 0, 150, 0, 0,
    125, 0, 0, 100, 0, 0, 75,  0, 0, 50,  0, 0, 25,  0, 0,

};

uint8_t ubRGBItemCycleYellowColors[] = {
    25,  25,  0, 50,  50,  0, 75,  75,  0, 100, 100, 0, 125, 125, 0,
    150, 150, 0, 175, 175, 0, 200, 200, 0, 225, 225, 0, 250, 250, 0,

    250, 250, 0, 225, 225, 0, 200, 200, 0, 175, 175, 0, 150, 150, 0,
    125, 125, 0, 100, 100, 0, 75,  75,  0, 50,  50,  0, 25,  25,  0,

    25,  25,  0, 50,  50,  0, 75,  75,  0, 100, 100, 0, 125, 125, 0,
    150, 150, 0, 175, 175, 0, 200, 200, 0, 225, 225, 0, 250, 250, 0,

    250, 250, 0, 225, 225, 0, 200, 200, 0, 175, 175, 0, 150, 150, 0,
    125, 125, 0, 100, 100, 0, 75,  75,  0, 50,  50,  0, 25,  25,  0,

    25,  25,  0, 50,  50,  0, 75,  75,  0, 100, 100, 0, 125, 125, 0,
    150, 150, 0, 175, 175, 0, 200, 200, 0, 225, 225, 0, 250, 250, 0,

    250, 250, 0, 225, 225, 0, 200, 200, 0, 175, 175, 0, 150, 150, 0,
    125, 125, 0, 100, 100, 0, 75,  75,  0, 50,  50,  0, 25,  25,  0,

};

#define NUMSPEEDS 5

uint8_t gubNewScrollXSpeeds[2][NUMSPEEDS] = {
    {40, 80, 100, 180, 200},  // Non-video mode scroll
    {20, 40, 80, 80, 80}      // Video mode scroll
};

uint8_t gubNewScrollYSpeeds[2][NUMSPEEDS] = {
    {40, 80, 100, 180, 200},  // Non-video mode scroll
    {10, 20, 60, 80, 80}      // Video mode scroll
};

// These speeds are only an indication of how long to do each subtile step until moving on to
// another
uint8_t gubNewScrollIDSpeeds[] = {10, 10, 20, 20, 20};

uint8_t gubScrollSpeedStartID = 2;
uint8_t gubScrollSpeedEndID = 4;

uint8_t gubCurScrollSpeedID = 1;

BOOLEAN gfDoVideoScroll = TRUE;
BOOLEAN gfDoSubtileScroll = FALSE;

BOOLEAN gfScrollPending = FALSE;

uint32_t uiLayerUsedFlags = 0xffffffff;
uint32_t uiAdditiveLayerUsedFlags = 0xffffffff;

// Array of shade values to use.....
#define NUM_GLOW_FRAMES 30

#if 0
int16_t	gsGlowFrames[] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,

	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,

	9,
	8,
	7,
	6,
	5,
	4,
	3,
	2,
	1,
	0,
};
#endif

int16_t gsGlowFrames[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    2, 4, 6, 8, 9, 8, 6, 4, 2, 0,
};

// This has to be the same # of frames as NUM_GLOW_FRAMES
int16_t gsFastGlowFrames[] = {
    0, 0, 6, 7, 8, 9, 8, 7, 6, 5,

    0, 0, 6, 7, 8, 9, 8, 7, 6, 5,

    0, 0, 6, 7, 8, 9, 8, 7, 6, 5,
};

// The glow frame pointer can be adjusted to use a faster/ slower glow
int16_t *gpGlowFramePointer = gsGlowFrames;

uint32_t gScrollDirectionFlags[NUM_WORLD_DIRECTIONS] = {
    SCROLL_UP | SCROLL_RIGHT,  SCROLL_RIGHT, SCROLL_DOWN | SCROLL_RIGHT, SCROLL_DOWN,
    SCROLL_DOWN | SCROLL_LEFT, SCROLL_LEFT,  SCROLL_UP | SCROLL_LEFT,    SCROLL_UP,
};

int16_t SCROLL_X_STEP = (WORLD_TILE_X);
int16_t SCROLL_Y_STEP = (WORLD_TILE_Y * 2);

int16_t gsVIEWPORT_START_X = 0;
int16_t gsVIEWPORT_START_Y = 0;
int16_t gsVIEWPORT_END_Y = 360;
int16_t gsVIEWPORT_WINDOW_END_Y = 360;
int16_t gsVIEWPORT_WINDOW_START_Y = 0;
int16_t gsVIEWPORT_END_X = 640;

int16_t gsTopLeftWorldX, gsTopLeftWorldY;
int16_t gsTopRightWorldX, gsTopRightWorldY;
int16_t gsBottomLeftWorldX, gsBottomLeftWorldY;
int16_t gsBottomRightWorldX, gsBottomRightWorldY;
BOOLEAN gfIgnoreScrolling = FALSE;

BOOLEAN gfIgnoreScrollDueToCenterAdjust = FALSE;

// GLOBAL SCROLLING PARAMS
int16_t gTopLeftWorldLimitX, gTopLeftWorldLimitY;
int16_t gTopRightWorldLimitX, gTopRightWorldLimitY;
int16_t gBottomLeftWorldLimitX, gBottomLeftWorldLimitY;
int16_t gBottomRightWorldLimitX, gBottomRightWorldLimitY;
int16_t Slide;
int16_t gsTLX, gsTLY, gsTRX, gsTRY;
int16_t gsBLX, gsBLY, gsBRX, gsBRY;
int16_t gsCX, gsCY;
double gdScaleX, gdScaleY;

#define FASTMAPROWCOLTOPOS(r, c) ((r) * WORLD_COLS + (c))

BOOLEAN gfScrollInertia = FALSE;

// GLOBALS FOR CALCULATING STARTING PARAMETERS
int16_t gsStartPointX_W, gsStartPointY_W;
int16_t gsStartPointX_S, gsStartPointY_S;
int16_t gsStartPointX_M, gsStartPointY_M;
int16_t gsEndXS, gsEndYS;
// LARGER OFFSET VERSION FOR GIVEN LAYERS
int16_t gsLStartPointX_W, gsLStartPointY_W;
int16_t gsLStartPointX_S, gsLStartPointY_S;
int16_t gsLStartPointX_M, gsLStartPointY_M;
int16_t gsLEndXS, gsLEndYS;

BOOLEAN gfRenderScroll = FALSE;
BOOLEAN gfScrollStart = FALSE;
int16_t gsScrollXIncrement;
int16_t gsScrollYIncrement;
int32_t guiScrollDirection;

// Rendering flags (full, partial, etc.)
uint32_t gRenderFlags = 0;

SGPRect gClippingRect = {0, 0, 640, 360};
SGPRect gOldClipRect;
int16_t gsRenderCenterX;
int16_t gsRenderCenterY;
int16_t gsRenderWorldOffsetX = -1;
int16_t gsRenderWorldOffsetY = -1;
SGPRect gSelectRegion;
uint32_t fSelectMode = NO_SELECT;
SGPPoint gSelectAnchor;

typedef struct {
  BOOLEAN fDynamic;
  BOOLEAN fZWrite;
  BOOLEAN fZBlitter;
  BOOLEAN fShadowBlitter;
  BOOLEAN fLinkedListDirection;
  BOOLEAN fMerc;
  BOOLEAN fCheckForRedundency;
  BOOLEAN fMultiZBlitter;
  BOOLEAN fConvertTo16;
  BOOLEAN fObscured;

} RenderFXType;

RenderFXType RenderFX[] = {
    {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE},  // STATIC LAND
    {FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE},     // STATIC OBJECTS
    {FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},     // STATIC SHADOWS
    {FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE},     // STATIC STRUCTS
    {FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},    // STATIC ROOF
    {FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE},     // STATIC ONROOF
    {FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},    // STATIC TOPMOST
    {TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE},    // DYNAMIC LAND
    {TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE},     // DYNAMIC OBJECT
    {TRUE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},    // DYNAMIC SHADOW
    {TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE},     // DYNAMIC STRUCT MERCS
    {TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE},     // DYNAMIC MERCS
    {TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},    // DYNAMIC STRUCT
    {TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},    // DYNAMIC ROOF
    {TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE},     // DYNAMIC HIGHMERCS
    {TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE},    // DYNAMIC ONROOF
    {TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE}     // DYNAMIC TOPMOST

};

uint8_t RenderFXStartIndex[] = {
    LAND_START_INDEX,     // STATIC LAND
    OBJECT_START_INDEX,   // STATIC OBJECTS
    SHADOW_START_INDEX,   // STATIC SHADOWS
    STRUCT_START_INDEX,   // STATIC STRUCTS
    ROOF_START_INDEX,     // STATIC ROOF
    ONROOF_START_INDEX,   // STATIC ONROOF
    TOPMOST_START_INDEX,  // STATIC TOPMOST
    LAND_START_INDEX,     // DYNAMIC LAND
    OBJECT_START_INDEX,   // DYNAMIC OBJECT
    SHADOW_START_INDEX,   // DYNAMIC SHADOW
    MERC_START_INDEX,     // DYNAMIC STRUCT MERCS
    MERC_START_INDEX,     // DYNAMIC MERCS
    STRUCT_START_INDEX,   // DYNAMIC STRUCT
    ROOF_START_INDEX,     // DYNAMIC ROOF
    MERC_START_INDEX,     // DYNAMIC HIGHMERCS
    ONROOF_START_INDEX,   // DYNAMIC ONROOF
    TOPMOST_START_INDEX,  // DYNAMIC TOPMOST
};

// int16_t gsCoordArray[ 500 ][ 500 ][ 4 ];
// int16_t gsCoordArrayX;
// int16_t gsCoordArrayY;

// void SetRenderGlobals( int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
// int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS ); void TempRenderTiles(uint32_t uiFlags,
// int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S, int16_t sStartPointY_S,
// int16_t sEndXS, int16_t sEndYS ); void TempRenderTiles(uint32_t uiFlags, int16_t sStartPointX_M,
// int16_t sStartPointY_M, int16_t sStartPointX_S, int16_t sStartPointY_S, int16_t sEndXS, int16_t
// sEndYS, uint8_t ubNumLevels, uint32_t *puiLevels );

void ExamineZBufferForHiddenTiles(int16_t sStartPointX_M, int16_t sStartPointY_M,
                                  int16_t sStartPointX_S, int16_t sStartPointY_S, int16_t sEndXS,
                                  int16_t sEndYS);

// void ReRenderWorld(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom);
void ClearMarkedTiles(void);
void CorrectRenderCenter(int16_t sRenderX, int16_t sRenderY, int16_t *pSNewX, int16_t *pSNewY);
void ScrollBackground(uint32_t uiDirection, int16_t sScrollXIncrement, int16_t sScrollYIncrement);

void CalcRenderParameters(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom);
void ResetRenderParameters();

BOOLEAN Zero8BPPDataTo16BPPBufferTransparent(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                             struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                             uint16_t usIndex);

BOOLEAN Blt8BPPDataTo16BPPBufferTransInvZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                          uint16_t *pZBuffer, uint16_t usZValue,
                                          struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                          uint16_t usIndex);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion,
    int16_t sZIndex, uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncObscureClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                     uint16_t *pZBuffer, uint16_t usZValue,
                                                     struct VObject *hSrcVObject, int32_t iX,
                                                     int32_t iY, uint16_t usIndex,
                                                     SGPRect *clipregion);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion,
    int16_t sZIndex, uint16_t *p16BPPPalette);
BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion);

void RenderRoomInfo(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                    int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS);

#ifdef _DEBUG

extern uint8_t gubFOVDebugInfoInfo[WORLD_MAX];
extern uint8_t gubGridNoMarkers[WORLD_MAX];
extern uint8_t gubGridNoValue;

extern BOOLEAN gfDisplayCoverValues;
extern BOOLEAN gfDisplayGridNoVisibleValues = 0;
extern int16_t gsCoverValue[WORLD_MAX];
extern int16_t gsBestCover;

void RenderFOVDebugInfo(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                        int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS);
void RenderCoverDebugInfo(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                          int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS);
void RenderGridNoVisibleDebugInfo(int16_t sStartPointX_M, int16_t sStartPointY_M,
                                  int16_t sStartPointX_S, int16_t sStartPointY_S, int16_t sEndXS,
                                  int16_t sEndYS);

#endif

void DeleteFromWorld(uint16_t usTileIndex, uint32_t uiRenderTiles, uint16_t usIndex);

void RenderHighlight(int16_t sMouseX_M, int16_t sMouseY_M, int16_t sStartPointX_M,
                     int16_t sStartPointY_M, int16_t sStartPointX_S, int16_t sStartPointY_S,
                     int16_t sEndXS, int16_t sEndYS);
BOOLEAN CheckRenderCenter(int16_t sNewCenterX, int16_t sNewCenterY);

BOOLEAN RevealWalls(int16_t sX, int16_t sY, int16_t sRadius) {
  struct LEVELNODE *pStruct;
  int16_t sCountX, sCountY;
  uint32_t uiTile;
  TILE_ELEMENT *TileElem;

  for (sCountY = sY - sRadius; sCountY < (sY + sRadius + 2); sCountY++)
    for (sCountX = sX - sRadius; sCountX < (sX + sRadius + 2); sCountX++) {
      uiTile = FASTMAPROWCOLTOPOS(sCountY, sCountX);
      pStruct = gpWorldLevelData[uiTile].pStructHead;

      while (pStruct != NULL) {
        TileElem = &(gTileDatabase[pStruct->usIndex]);
        switch (TileElem->usWallOrientation) {
          case NO_ORIENTATION:
            break;

          case INSIDE_TOP_RIGHT:
          case OUTSIDE_TOP_RIGHT:
            if (sCountX >= sX) {
              pStruct->uiFlags |= LEVELNODE_REVEAL;
            }
            break;

          case INSIDE_TOP_LEFT:
          case OUTSIDE_TOP_LEFT:
            if (sCountY >= sY) {
              pStruct->uiFlags |= LEVELNODE_REVEAL;
            }
            break;
        }
        pStruct = pStruct->pNext;
      }
    }

  return (TRUE);
}

BOOLEAN ConcealWalls(int16_t sX, int16_t sY, int16_t sRadius) {
  struct LEVELNODE *pStruct;
  int16_t sCountX, sCountY;
  uint32_t uiTile;
  BOOLEAN fRerender = FALSE;
  TILE_ELEMENT *TileElem;

  for (sCountY = sY - sRadius; sCountY < (sY + sRadius + 2); sCountY++)
    for (sCountX = sX - sRadius; sCountX < (sX + sRadius + 2); sCountX++) {
      uiTile = FASTMAPROWCOLTOPOS(sCountY, sCountX);
      pStruct = gpWorldLevelData[uiTile].pStructHead;

      while (pStruct != NULL) {
        TileElem = &(gTileDatabase[pStruct->usIndex]);
        switch (TileElem->usWallOrientation) {
          case NO_ORIENTATION:
            break;

          case INSIDE_TOP_RIGHT:
          case OUTSIDE_TOP_RIGHT:
            if (sCountX >= sX) {
              pStruct->uiFlags &= (~LEVELNODE_REVEAL);
              fRerender = TRUE;
            }
            break;

          case INSIDE_TOP_LEFT:
          case OUTSIDE_TOP_LEFT:
            if (sCountY >= sY) {
              pStruct->uiFlags &= (~LEVELNODE_REVEAL);
              fRerender = TRUE;
            }
            break;
        }
        pStruct = pStruct->pNext;
      }
    }

  /*
          if(fRerender)
                  SetRenderFlags(RENDER_FLAG_FULL);
  */

  return (TRUE);
}

void ConcealAllWalls(void) {
  struct LEVELNODE *pStruct;
  uint32_t uiCount;

  for (uiCount = 0; uiCount < WORLD_MAX; uiCount++) {
    pStruct = gpWorldLevelData[uiCount].pStructHead;
    while (pStruct != NULL) {
      pStruct->uiFlags &= (~LEVELNODE_REVEAL);
      pStruct = pStruct->pNext;
    }
  }
}

void ResetLayerOptimizing(void) {
  uiLayerUsedFlags = 0xffffffff;
  uiAdditiveLayerUsedFlags = 0;
}

void ResetSpecificLayerOptimizing(uint32_t uiRowFlag) { uiLayerUsedFlags |= uiRowFlag; }

void SumAddiviveLayerOptimization(void) { uiLayerUsedFlags = uiAdditiveLayerUsedFlags; }

void SetRenderFlags(uint32_t uiFlags) { gRenderFlags |= uiFlags; }

void ClearRenderFlags(uint32_t uiFlags) { gRenderFlags &= (~uiFlags); }

uint32_t GetRenderFlags(void) { return (gRenderFlags); }

void RenderSetShadows(BOOLEAN fShadows) {
  if (fShadows)
    gRenderFlags |= RENDER_FLAG_SHADOWS;
  else
    gRenderFlags &= (~RENDER_FLAG_SHADOWS);
}

void RenderTiles(uint32_t uiFlags, int32_t iStartPointX_M, int32_t iStartPointY_M,
                 int32_t iStartPointX_S, int32_t iStartPointY_S, int32_t iEndXS, int32_t iEndYS,
                 uint8_t ubNumLevels, uint32_t *puiLevels, uint16_t *psLevelIDs) {
  // #if 0

  struct LEVELNODE *pNode;  //, *pLand, *pStruct; //*pObject, *pTopmost, *pMerc;
  struct SOLDIERTYPE *pSoldier, *pSelSoldier;
  struct VObject *hVObject;
  ETRLEObject *pTrav;
  TILE_ELEMENT *TileElem = NULL;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf = NULL;
  uint16_t usAnimSurface;
  int8_t bXOddFlag = 0;
  int32_t iAnchorPosX_M, iAnchorPosY_M;
  int32_t iAnchorPosX_S, iAnchorPosY_S;
  int32_t iTempPosX_M, iTempPosY_M;
  int32_t iTempPosX_S, iTempPosY_S;
  float dOffsetX, dOffsetY;
  float dTempX_S, dTempY_S;
  uint32_t uiTileIndex;
  uint16_t usImageIndex, *pShadeTable, *pDirtyBackPtr;
  uint32_t uiBrushWidth, uiBrushHeight, uiDirtyFlags;
  int16_t sTileHeight, sXPos, sYPos, sZLevel;
  int16_t sMouseX_M, sMouseY_M;
  BOOLEAN fShadowBlitter = FALSE;
  BOOLEAN fZBlitter = FALSE;
  BOOLEAN fZWrite = FALSE;
  BOOLEAN fLinkedListDirection = TRUE;
  BOOLEAN fRenderTile = TRUE;
  BOOLEAN fMerc = FALSE;
  BOOLEAN fCheckForRedundency = FALSE;
  uint32_t uiRowFlags;
  BOOLEAN fDynamic = TRUE;
  BOOLEAN fEndRenderRow = FALSE;
  BOOLEAN fEndRenderCol = FALSE;
  BOOLEAN fPixelate = FALSE;
  BOOLEAN fMultiZBlitter = FALSE;
  BOOLEAN fWallTile = FALSE;
  BOOLEAN fMultiTransShadowZBlitter = FALSE;
  int16_t sMultiTransShadowZBlitterIndex = -1;
  BOOLEAN fTranslucencyType = FALSE;
  int16_t sX, sY;
  BOOLEAN fTileInvisible = FALSE;
  BOOLEAN fConvertTo16 = FALSE;
  BOOLEAN fBlit16 = FALSE;
  uint32_t cnt;
  static uint8_t ubLevelNodeStartIndex[NUM_RENDER_FX_TYPES];
  BOOLEAN bItemOutline;
  uint16_t usOutlineColor = 0;

  static int32_t iTileMapPos[500];
  uint32_t uiMapPosIndex;
  uint8_t bBlitClipVal;
  int8_t bItemCount, bVisibleItemCount;
  // uint16_t			us16BPPIndex;
  RenderFXType RenderingFX;
  BOOLEAN fCheckForMouseDetections = FALSE;
  static RenderFXType RenderFXList[NUM_RENDER_FX_TYPES];
  BOOLEAN fSaveZ;
  int16_t sWorldY;
  int16_t sZOffsetX = -1;
  int16_t sZOffsetY = -1;
  BOOLEAN fIntensityBlitter;
  int16_t gsForceSoldierZLevel;
  ROTTING_CORPSE *pCorpse = NULL;
  BOOLEAN fUseTileElem;
  uint32_t uiLevelNodeFlags;
  uint32_t uiTileElemFlags = 0;
  int8_t bGlowShadeOffset;
  BOOLEAN fObscured;
  BOOLEAN fObscuredBlitter;
  int16_t sModifiedTileHeight;
  BOOLEAN fDoRow;
  uint16_t **pShadeStart;

  uint32_t uiSaveBufferPitchBYTES;
  uint8_t *pSaveBuf;
  struct ITEM_POOL *pItemPool = NULL;
  BOOLEAN fHiddenTile = FALSE;
  uint32_t uiAniTileFlags = 0;

  // Init some variables
  usImageIndex = 0;
  sZLevel = 0;
  uiDirtyFlags = 0;
  pShadeTable = NULL;

  // Begin Render Loop
  iAnchorPosX_M = iStartPointX_M;
  iAnchorPosY_M = iStartPointY_M;
  iAnchorPosX_S = iStartPointX_S;
  iAnchorPosY_S = iStartPointY_S;

  if (!(uiFlags & TILES_DIRTY)) pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  if (uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE) {
    if (ShouldCheckForMouseDetections()) {
      BeginCurInteractiveTileCheck(gubIntTileCheckFlags);
      fCheckForMouseDetections = TRUE;

      // If we are in edit mode, don't do this...
      if (gfEditMode) {
        fCheckForMouseDetections = FALSE;
      }
    }
  }

  // if((uiFlags&TILES_TYPE_MASK)==TILES_STATIC_LAND)
  GetMouseXY(&sMouseX_M, &sMouseY_M);

  pDirtyBackPtr = NULL;

  if (gTacticalStatus.uiFlags & TRANSLUCENCY_TYPE) fTranslucencyType = TRUE;

  for (cnt = 0; cnt < ubNumLevels; cnt++) {
    ubLevelNodeStartIndex[cnt] = RenderFXStartIndex[psLevelIDs[cnt]];
    RenderFXList[cnt] = RenderFX[psLevelIDs[cnt]];
  }

  do {
    iTempPosX_M = iAnchorPosX_M;
    iTempPosY_M = iAnchorPosY_M;
    iTempPosX_S = iAnchorPosX_S;
    iTempPosY_S = iAnchorPosY_S;

    uiMapPosIndex = 0;

    // Build tile index list
    do {
      iTileMapPos[uiMapPosIndex] = FASTMAPROWCOLTOPOS(iTempPosY_M, iTempPosX_M);

      iTempPosX_S += 40;
      iTempPosX_M++;
      iTempPosY_M--;

      uiMapPosIndex++;

    } while (iTempPosX_S < iEndXS);

    for (cnt = 0; cnt < ubNumLevels; cnt++) {
      uiRowFlags = puiLevels[cnt];
      fDoRow = TRUE;

      if ((uiRowFlags & TILES_ALL_DYNAMICS) && !(uiLayerUsedFlags & uiRowFlags) &&
          !(uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE)) {
        fDoRow = FALSE;
      }

      if (fDoRow) {
        iTempPosX_M = iAnchorPosX_M;
        iTempPosY_M = iAnchorPosY_M;
        iTempPosX_S = iAnchorPosX_S;
        iTempPosY_S = iAnchorPosY_S;

        fEndRenderRow = FALSE;
        uiMapPosIndex = 0;

        if (bXOddFlag > 0) iTempPosX_S += 20;

        do {
          uiTileIndex = iTileMapPos[uiMapPosIndex];
          uiMapPosIndex++;

          // if ( 0 )
          if (uiTileIndex < GRIDSIZE) {
            // OK, we're sreaching through this loop anyway, might as well check for mouse position
            // over objects...
            // Experimental!
            if (uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE) {
              if (fCheckForMouseDetections && gpWorldLevelData[uiTileIndex].pStructHead != NULL) {
                LogMouseOverInteractiveTile((int16_t)uiTileIndex);
              }
            }

            if ((uiFlags & TILES_MARKED) &&
                !(gpWorldLevelData[uiTileIndex].uiFlags & MAPELEMENT_REDRAW)) {
              pNode = NULL;
            } else {
              // pNode = gpWorldLevelData[ uiTileIndex ].pLevelNodes[ RenderFXStartIndex[
              // psLevelIDs[ cnt ] ] ]; pNode = gpWorldLevelData[ uiTileIndex ].pLevelNodes[ 0 ];
              // pNode=NULL;
              pNode = gpWorldLevelData[uiTileIndex].pLevelNodes[ubLevelNodeStartIndex[cnt]];
            }

            bItemCount = 0;
            bVisibleItemCount = 0;
            pItemPool = NULL;

            while (pNode != NULL) {
              RenderingFX = RenderFXList[cnt];

              fObscured = RenderingFX.fObscured;
              fDynamic = RenderingFX.fDynamic;

              fMerc = RenderingFX.fMerc;
              fZWrite = RenderingFX.fZWrite;
              fZBlitter = RenderingFX.fZBlitter;
              fShadowBlitter = RenderingFX.fShadowBlitter;
              fLinkedListDirection = RenderingFX.fLinkedListDirection;
              fCheckForRedundency = RenderingFX.fCheckForRedundency;
              fMultiZBlitter = RenderingFX.fMultiZBlitter;
              fConvertTo16 = RenderingFX.fConvertTo16;
              fIntensityBlitter = FALSE;
              fSaveZ = FALSE;
              fWallTile = FALSE;
              gsForceSoldierZLevel = FALSE;
              pSoldier = NULL;
              fUseTileElem = FALSE;
              fMultiTransShadowZBlitter = FALSE;
              fObscuredBlitter = FALSE;
              fTranslucencyType = TRUE;
              uiAniTileFlags = 0;

              uiLevelNodeFlags = pNode->uiFlags;

              if (fCheckForRedundency) {
                if ((gpWorldLevelData[uiTileIndex].uiFlags & MAPELEMENT_REDUNDENT)) {
                  // IF WE DONOT WANT TO RE-EVALUATE FIRST
                  if (!(gpWorldLevelData[uiTileIndex].uiFlags & MAPELEMENT_REEVALUATE_REDUNDENCY) &&
                      !(gTacticalStatus.uiFlags & NOHIDE_REDUNDENCY)) {
                    pNode = NULL;
                    break;
                  }
                }
              }

              // Force z-buffer blitting for marked tiles ( even ground!)
              if ((uiFlags & TILES_MARKED)) {
                fZBlitter = TRUE;
              }

              // Looking up height every time here is alot better than doing it above!
              sTileHeight = gpWorldLevelData[uiTileIndex].sHeight;

              sModifiedTileHeight = (((sTileHeight / 80) - 1) * 80);

              if (sModifiedTileHeight < 0) {
                sModifiedTileHeight = 0;
              }

              fRenderTile = TRUE;
              pDirtyBackPtr = NULL;
              if (uiLevelNodeFlags & LEVELNODE_REVEAL) {
                if (!fDynamic)
                  fRenderTile = FALSE;
                else
                  fPixelate = TRUE;
              } else
                fPixelate = FALSE;

              // non-type specific setup
              sXPos = (int16_t)iTempPosX_S;
              sYPos = (int16_t)iTempPosY_S;

              // setup for any tile type except mercs
              if (!fMerc) {
                if (!(uiLevelNodeFlags & (LEVELNODE_ROTTINGCORPSE | LEVELNODE_CACHEDANITILE))) {
                  if ((uiLevelNodeFlags & LEVELNODE_REVEALTREES)) {
                    TileElem = &(gTileDatabase[pNode->usIndex + 2]);
                  } else {
                    TileElem = &(gTileDatabase[pNode->usIndex]);
                  }

                  // HANDLE INDEPENDANT-PER-TILE ANIMATIONS ( IE: DOORS, EXPLODING THINGS, ETC )
                  if (fDynamic) {
                    if ((uiLevelNodeFlags & LEVELNODE_ANIMATION)) {
                      if (pNode->sCurrentFrame != -1) {
                        Assert(TileElem->pAnimData != NULL);
                        TileElem =
                            &gTileDatabase[TileElem->pAnimData->pusFrames[pNode->sCurrentFrame]];
                      }
                    }
                  }
                }

                // Check for best translucency
                if (uiLevelNodeFlags & LEVELNODE_USEBESTTRANSTYPE) {
                  fTranslucencyType = FALSE;
                }

                if ((uiLevelNodeFlags & (LEVELNODE_ROTTINGCORPSE | LEVELNODE_CACHEDANITILE))) {
                  if (fDynamic) {
                    if (!(uiLevelNodeFlags & (LEVELNODE_DYNAMIC)) &&
                        !(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC))
                      fRenderTile = FALSE;
                  } else if ((uiLevelNodeFlags & (LEVELNODE_DYNAMIC)))
                    fRenderTile = FALSE;
                } else {
                  // Set Tile elem flags here!
                  uiTileElemFlags = TileElem->uiFlags;
                  // Set valid tile elem!
                  fUseTileElem = TRUE;

                  if (fDynamic || fPixelate) {
                    if (!fPixelate) {
                      if (!(uiTileElemFlags & ANIMATED_TILE) && !(uiTileElemFlags & DYNAMIC_TILE) &&
                          !(uiLevelNodeFlags & LEVELNODE_DYNAMIC) &&
                          !(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC))
                        fRenderTile = FALSE;
                      else if (!(uiTileElemFlags & DYNAMIC_TILE) &&
                               !(uiLevelNodeFlags & LEVELNODE_DYNAMIC) &&
                               !(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC))
                      //	else if((TileElem->uiFlags&ANIMATED_TILE) )
                      {
                        Assert(TileElem->pAnimData != NULL);
                        TileElem =
                            &gTileDatabase[TileElem->pAnimData
                                               ->pusFrames[TileElem->pAnimData->bCurrentFrame]];
                        uiTileElemFlags = TileElem->uiFlags;
                      }
                    }

                  } else if ((uiTileElemFlags & ANIMATED_TILE) ||
                             (uiTileElemFlags & DYNAMIC_TILE) ||
                             (uiLevelNodeFlags & LEVELNODE_DYNAMIC)) {
                    if (!(uiFlags & TILES_OBSCURED) || (uiTileElemFlags & ANIMATED_TILE)) {
                      fRenderTile = FALSE;
                    }
                  }
                }

                // OK, ATE, CHECK FOR AN OBSCURED TILE AND MAKE SURE IF struct LEVELNODE IS SET
                // WE DON'T RENDER UNLESS WE HAVE THE RENDER FLAG SET!
                if (fObscured) {
                  if ((uiFlags & TILES_OBSCURED)) {
                    if (uiLevelNodeFlags & LEVELNODE_SHOW_THROUGH) {
                      fObscuredBlitter = TRUE;
                    } else {
                      // Don;t render if we are not on this render loop!
                      fRenderTile = FALSE;
                    }
                  } else {
                    if (uiLevelNodeFlags & LEVELNODE_SHOW_THROUGH) {
                      fRenderTile = FALSE;
                    }
                  }
                }

                // If flag says to do dynamic as well, render!
                if ((uiFlags & TILES_DOALL)) {
                  fRenderTile = TRUE;
                }

                // If we are on the struct layer, check for if it's hidden!
                if (uiRowFlags & (TILES_STATIC_STRUCTURES | TILES_DYNAMIC_STRUCTURES |
                                  TILES_STATIC_SHADOWS | TILES_DYNAMIC_SHADOWS)) {
                  if (fUseTileElem) {
                  }
                }

                if (fRenderTile) {
                  // Set flag to set layer as used
                  if (fDynamic || fPixelate) {
                    uiAdditiveLayerUsedFlags |= uiRowFlags;
                  }

                  if (uiLevelNodeFlags & LEVELNODE_DYNAMICZ) {
                    fSaveZ = TRUE;
                    fZWrite = TRUE;
                  }

                  if ((uiLevelNodeFlags & LEVELNODE_CACHEDANITILE)) {
                    hVObject = gpTileCache[pNode->pAniTile->sCachedTileID].pImagery->vo;
                    usImageIndex = pNode->pAniTile->sCurrentFrame;
                    uiAniTileFlags = pNode->pAniTile->uiFlags;

                    // Position corpse based on it's float position
                    if ((uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)) {
                      pCorpse = &(gRottingCorpse[pNode->pAniTile->uiUserData]);

                      pShadeTable = pCorpse->pShades[pNode->ubShadeLevel];

                      // pShadeTable = pCorpse->p16BPPPalette;

                      dOffsetX = pCorpse->def.dXPos - gsRenderCenterX;
                      dOffsetY = pCorpse->def.dYPos - gsRenderCenterY;

                      // OK, if this is a corpse.... stop if not visible
                      if (pCorpse->def.bVisible != 1 &&
                          !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
                        // CONTINUE, DONOT RENDER
                        if (!fLinkedListDirection)
                          pNode = pNode->pPrevNode;
                        else
                          pNode = pNode->pNext;

                        continue;
                      }
                    } else {
                      dOffsetX = (float)(pNode->pAniTile->sRelativeX - gsRenderCenterX);
                      dOffsetY = (float)(pNode->pAniTile->sRelativeY - gsRenderCenterY);
                    }

                    // Calculate guy's position
                    FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

                    sXPos = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)dTempX_S;
                    sYPos = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)dTempY_S -
                            sTileHeight;

                    // Adjust for offset position on screen
                    sXPos -= gsRenderWorldOffsetX;
                    sYPos -= gsRenderWorldOffsetY;

                  } else {
                    hVObject = TileElem->hTileSurface;
                    usImageIndex = TileElem->usRegionIndex;

                    // ADJUST FOR WORLD MAPELEM HIEGHT
                    sYPos -= TileElem->sOffsetHeight;

                    if ((TileElem->uiFlags & IGNORE_WORLD_HEIGHT)) {
                      sYPos = sYPos - sModifiedTileHeight;
                      // sYPos -= sTileHeight;
                    }

                    if (!(uiLevelNodeFlags & LEVELNODE_IGNOREHEIGHT) &&
                        !(TileElem->uiFlags & IGNORE_WORLD_HEIGHT))
                      sYPos -= sTileHeight;

                    if (!(uiFlags & TILES_DIRTY)) {
                      hVObject->pShadeCurrent = hVObject->pShades[pNode->ubShadeLevel];
                      hVObject->pShade8 = ubColorTables[pNode->ubShadeLevel];
                    }
                  }

                  // ADJUST FOR RELATIVE OFFSETS
                  if (uiLevelNodeFlags & LEVELNODE_USERELPOS) {
                    sXPos += pNode->sRelativeX;
                    sYPos += pNode->sRelativeY;
                  }

                  if (uiLevelNodeFlags & LEVELNODE_USEZ) {
                    sYPos -= pNode->sRelativeZ;
                  }

                  // ADJUST FOR ABSOLUTE POSITIONING
                  if (uiLevelNodeFlags & LEVELNODE_USEABSOLUTEPOS) {
                    dOffsetX = (float)(pNode->sRelativeX - gsRenderCenterX);
                    dOffsetY = (float)(pNode->sRelativeY - gsRenderCenterY);

                    // OK, DONT'T ASK... CONVERSION TO PROPER Y NEEDS THIS...
                    dOffsetX -= CELL_Y_SIZE;

                    FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

                    sXPos = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) +
                            (int16_t)SHORT_ROUND(dTempX_S);
                    sYPos = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) +
                            (int16_t)SHORT_ROUND(dTempY_S);

                    // Adjust for offset position on screen
                    sXPos -= gsRenderWorldOffsetX;
                    sYPos -= gsRenderWorldOffsetY;

                    sYPos -= pNode->sRelativeZ;
                  }
                }

                // COUNT # OF ITEMS AT THIS LOCATION
                if (uiLevelNodeFlags & LEVELNODE_ITEM) {
                  // OK set item pool for this location....
                  if (bItemCount == 0) {
                    pItemPool = pNode->pItemPool;
                  } else {
                    pItemPool = pItemPool->pNext;
                  }

                  if (bItemCount < MAX_RENDERED_ITEMS) {
                    bItemCount++;

                    if (gWorldItems[pItemPool->iItemIndex].bVisible == VISIBLE) {
                      bVisibleItemCount++;
                    }
                  }

                  // LIMIT RENDERING OF ITEMS TO ABOUT 7, DO NOT RENDER HIDDEN ITEMS TOO!
                  if (bVisibleItemCount == MAX_RENDERED_ITEMS ||
                      (gWorldItems[pItemPool->iItemIndex].bVisible != VISIBLE) ||
                      (pItemPool->usFlags & WORLD_ITEM_DONTRENDER)) {
                    if (!(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS)) {
                      // CONTINUE, DONOT RENDER
                      if (!fLinkedListDirection)
                        pNode = pNode->pPrevNode;
                      else
                        pNode = pNode->pNext;
                      continue;
                    }
                  }

                  if (pItemPool->bRenderZHeightAboveLevel > 0) {
                    sYPos -= pItemPool->bRenderZHeightAboveLevel;
                  }
                }

                // If render tile is false...
                if (!fRenderTile) {
                  if (!fLinkedListDirection)
                    pNode = pNode->pPrevNode;
                  else
                    pNode = pNode->pNext;

                  continue;
                }
              }

              // specific code for node types on a per-tile basis
              switch (uiRowFlags) {
                case TILES_STATIC_LAND:

                  LandZLevel(iTempPosX_M, iTempPosY_M);
                  break;

                case TILES_STATIC_OBJECTS:

                  // ATE: Modified to use constant z level, as these are same level as land items
                  ObjectZLevel(TileElem, pNode, iTempPosX_M, iTempPosY_M);
                  break;

                case TILES_STATIC_STRUCTURES:

                  StructZLevel(iTempPosX_M, iTempPosY_M);

                  if (fUseTileElem && (TileElem->uiFlags & MULTI_Z_TILE)) {
                    fMultiZBlitter = TRUE;
                  }

                  // ATE: if we are a wall, set flag
                  if (fUseTileElem && (TileElem->uiFlags & WALL_TILE)) {
                    fWallTile = TRUE;
                  }

                  break;

                case TILES_STATIC_ROOF:

                  RoofZLevel(iTempPosX_M, iTempPosY_M);

                  // Automatically adjust height!
                  sYPos -= WALL_HEIGHT;

                  // ATE: Added for shadows on roofs
                  if (fUseTileElem && (TileElem->uiFlags & ROOFSHADOW_TILE)) {
                    fShadowBlitter = TRUE;
                  }
                  break;
                case TILES_STATIC_ONROOF:

                  OnRoofZLevel(iTempPosX_M, iTempPosY_M);
                  // Automatically adjust height!
                  sYPos -= WALL_HEIGHT;
                  break;

                case TILES_STATIC_TOPMOST:

                  TopmostZLevel(iTempPosX_M, iTempPosY_M);
                  break;

                case TILES_STATIC_SHADOWS:

                  ShadowZLevel(iTempPosX_M, iTempPosY_M);

                  if (uiLevelNodeFlags & LEVELNODE_EXITGRID) {
                    fIntensityBlitter = TRUE;
                    fShadowBlitter = FALSE;
                  }
                  break;

                case TILES_DYNAMIC_LAND:

                  LandZLevel(iTempPosX_M, iTempPosY_M);
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  break;
                case TILES_DYNAMIC_SHADOWS:

                  ShadowZLevel(iTempPosX_M, iTempPosY_M);
                  // sZLevel=SHADOW_Z_LEVEL;
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  break;
                case TILES_DYNAMIC_OBJECTS:

                  ObjectZLevel(TileElem, pNode, iTempPosX_M, iTempPosY_M);
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  break;

                case TILES_DYNAMIC_STRUCTURES:

                  StructZLevel(iTempPosX_M, iTempPosY_M);
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  break;
                case TILES_DYNAMIC_ROOF:

                  sYPos -= WALL_HEIGHT;

                  RoofZLevel(iTempPosX_M, iTempPosY_M);
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  // For now, adjust to hieght of a wall ( 50 temp, make define )
                  // if ( TileElem->fType > FOOTPRINTS )
                  //{
                  //	sYPos -= 58;
                  //}
                  break;

                case TILES_DYNAMIC_ONROOF:

                  OnRoofZLevel(iTempPosX_M, iTempPosY_M);
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  // Automatically adjust height!
                  sYPos -= WALL_HEIGHT;
                  break;

                case TILES_DYNAMIC_TOPMOST:
                  TopmostZLevel(iTempPosX_M, iTempPosY_M);
                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED;
                  break;

                case TILES_DYNAMIC_MERCS:
                case TILES_DYNAMIC_HIGHMERCS:
                case TILES_DYNAMIC_STRUCT_MERCS:

                  // Set flag to set layer as used
                  uiAdditiveLayerUsedFlags |= uiRowFlags;

                  pSoldier = pNode->pSoldier;

                  if (uiRowFlags == TILES_DYNAMIC_MERCS) {
                    // If we are multi-tiled, ignore here
                    if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE_Z) {
                      pNode = pNode->pNext;
                      continue;
                    }

                    // If we are at a higher level, no not do anything unless we are at the highmerc
                    // stage
                    if (pSoldier->bLevel > 0) {
                      pNode = pNode->pNext;
                      continue;
                    }
                  }

                  if (uiRowFlags == TILES_DYNAMIC_HIGHMERCS) {
                    // If we are multi-tiled, ignore here
                    if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE_Z) {
                      pNode = pNode->pNext;
                      continue;
                    }

                    // If we are at a lower level, no not do anything unless we are at the highmerc
                    // stage
                    if (pSoldier->bLevel == 0) {
                      pNode = pNode->pNext;
                      continue;
                    }
                  }

                  if (uiRowFlags == TILES_DYNAMIC_STRUCT_MERCS) {
                    // If we are not multi-tiled, ignore here
                    if (!(pSoldier->uiStatusFlags & SOLDIER_MULTITILE_Z)) {
                      // If we are at a low level, no not do anything unless we are at the merc
                      // stage
                      if (pSoldier->bLevel == 0) {
                        pNode = pNode->pNext;
                        continue;
                      }
                    }

                    if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE_Z) {
                      fSaveZ = TRUE;
                      fMultiTransShadowZBlitter = TRUE;
                      fZBlitter = TRUE;

                      // ATE: Use one direction for queen!
                      if (pSoldier->ubBodyType == QUEENMONSTER) {
                        sMultiTransShadowZBlitterIndex = 0;
                      } else {
                        sMultiTransShadowZBlitterIndex = gOneCDirection[pSoldier->bDirection];
                      }
                    }
                  }

                  // IF we are not active, or are a placeholder for multi-tile animations do nothing
                  // if ( !IsSolActive(pSoldier)  )
                  if (!IsSolActive(pSoldier) || (uiLevelNodeFlags & LEVELNODE_MERCPLACEHOLDER)) {
                    pNode = pNode->pNext;
                    continue;
                  }

                  // Skip if we cannot see the guy!
                  if (pSoldier->bLastRenderVisibleValue == -1 &&
                      !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
                    pNode = pNode->pNext;
                    continue;
                  }

                  // Get animation surface....
                  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

                  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
                    pNode = pNode->pNext;
                    continue;
                  }

                  // Shade guy always lighter than sceane default!
                  {
                    uint8_t ubShadeLevel;

                    ubShadeLevel = (pNode->ubShadeLevel & 0x0f);
                    ubShadeLevel = max(ubShadeLevel - 2, DEFAULT_SHADE_LEVEL);
                    ubShadeLevel |= (pNode->ubShadeLevel & 0x30);

                    if (pSoldier->fBeginFade) {
                      pShadeTable = pSoldier->pCurrentShade =
                          pSoldier->pShades[pSoldier->ubFadeLevel];
                    } else {
                      pShadeTable = pSoldier->pCurrentShade = pSoldier->pShades[ubShadeLevel];
                    }
                  }

                  // Position guy based on guy's position
                  dOffsetX = pSoldier->dXPos - gsRenderCenterX;
                  dOffsetY = pSoldier->dYPos - gsRenderCenterY;

                  // Calculate guy's position
                  FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

                  sXPos = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)dTempX_S;
                  sYPos = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)dTempY_S -
                          sTileHeight;

                  // Adjust for offset position on screen
                  sXPos -= gsRenderWorldOffsetX;
                  sYPos -= gsRenderWorldOffsetY;

                  // Adjust for soldier height
                  sYPos -= pSoldier->sHeightAdjustment;

                  // Handle shade stuff....
                  if (!pSoldier->fBeginFade) {
                    // Special effect - draw ghost if is seen by a guy in player's team but not
                    // current guy ATE: Todo: setup flag for 'bad-guy' - can releive some checks in
                    // renderer
                    if (!pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum)) {
                      if (gusSelectedSoldier != NOBODY) {
                        pSelSoldier = MercPtrs[gusSelectedSoldier];
                      } else {
                        pSelSoldier = NULL;
                      }

                      bGlowShadeOffset = 0;

                      if (gTacticalStatus.ubCurrentTeam == gbPlayerNum) {
                        // Shade differently depending on visiblity
                        if (pSoldier->bLastRenderVisibleValue == 0) {
                          bGlowShadeOffset = 10;
                        }

                        if (pSelSoldier != NULL) {
                          if (pSelSoldier->bOppList[pSoldier->ubID] != SEEN_CURRENTLY) {
                            if (pSoldier->usAnimState != CHARIOTS_OF_FIRE &&
                                pSoldier->usAnimState != BODYEXPLODING) {
                              bGlowShadeOffset = 10;
                            }
                          }
                        }
                      }

                      if (pSoldier->bLevel == 0) {
                        pShadeStart = &(pSoldier->pGlowShades[0]);
                      } else {
                        pShadeStart = &(pSoldier->pShades[20]);
                      }

                      // Set shade
                      // If a bad guy is highlighted
                      if (gfUIHandleSelectionAboveGuy == TRUE &&
                          MercPtrs[gsSelectedGuy]->bSide != gbPlayerNum) {
                        if (gsSelectedGuy == GetSolID(pSoldier)) {
                          pShadeTable =
                              pShadeStart[gsGlowFrames[gsCurrentGlowFrame] + bGlowShadeOffset];
                          gsForceSoldierZLevel = TOPMOST_Z_LEVEL;
                        } else {
                          // Are we dealing with a not-so visible merc?
                          if (bGlowShadeOffset == 10) {
                            pShadeTable = pSoldier->pEffectShades[0];
                          }
                        }
                      } else {
                        // OK,not highlighted, but maybe we are in enemy's turn and they have the
                        // baton

                        // AI's turn?
                        if (gTacticalStatus.ubCurrentTeam != OUR_TEAM) {
                          // Does he have baton?
                          if ((pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL)) {
                            pShadeTable = pShadeStart[gpGlowFramePointer[gsCurrentGlowFrame] +
                                                      bGlowShadeOffset];

                            if (gpGlowFramePointer[gsCurrentGlowFrame] >= 7) {
                              gsForceSoldierZLevel = TOPMOST_Z_LEVEL;
                            }
                          }
                        } else {
                          pShadeTable = pShadeStart[gpGlowFramePointer[gsCurrentGlowFrame] +
                                                    bGlowShadeOffset];

                          if (gpGlowFramePointer[gsCurrentGlowFrame] >= 7) {
                            gsForceSoldierZLevel = TOPMOST_Z_LEVEL;
                          }
                        }
                      }
                    }
                  }

                  // Calculate Z level
                  SoldierZLevel(pSoldier, iTempPosX_M, iTempPosY_M);

                  if (!(uiFlags & TILES_DIRTY)) {
                    if (pSoldier->fForceShade) {
                      pShadeTable = pSoldier->pForcedShade;
                    }
                  }

                  // check if we are a merc duplicate, if so, only do minimal stuff!
                  if (pSoldier->ubID >= MAX_NUM_SOLDIERS) {
                    // Shade gray
                    pShadeTable = pSoldier->pEffectShades[1];
                  }

                  hVObject = gAnimSurfaceDatabase[usAnimSurface].hVideoObject;

                  if (hVObject == NULL) {
                    pNode = pNode->pNext;
                    continue;
                  }

                  // ATE: If we are in a gridno that we should not use obscure blitter, set!
                  if (!(gpWorldLevelData[uiTileIndex].ubExtFlags[0] &
                        MAPELEMENT_EXT_NOBURN_STRUCT)) {
                    fObscuredBlitter = TRUE;
                  } else {
                    // ATE: Artificially increase z=level...
                    sZLevel += 2;
                  }

                  usImageIndex = pSoldier->usAniFrame;

                  uiDirtyFlags = BGND_FLAG_SINGLE | BGND_FLAG_ANIMATED | BGND_FLAG_MERC;
                  break;
              }

              // Adjust for interface level
              sYPos += gsRenderHeight;

              // OK, check for struct LEVELNODE HIDDEN...
              fHiddenTile = FALSE;

              if (uiLevelNodeFlags & LEVELNODE_HIDDEN) {
                fHiddenTile = TRUE;

                if (TileElem != NULL) {
                  // If we are a roof and have SHOW_ALL_ROOFS on, turn off hidden tile check!
                  if ((TileElem->uiFlags & ROOF_TILE) &&
                      (gTacticalStatus.uiFlags & SHOW_ALL_ROOFS)) {
                    // Turn off
                    fHiddenTile = FALSE;
                  }
                }
              }

              if (fRenderTile && !fHiddenTile) {
                fTileInvisible = FALSE;

                if ((uiLevelNodeFlags & LEVELNODE_ROTTINGCORPSE)) {
                  // Set fmerc flag!
                  fMerc = TRUE;
                  fZWrite = TRUE;

                  // if ( hVObject->ppZStripInfo != NULL )
                  {
                    sMultiTransShadowZBlitterIndex = GetCorpseStructIndex(&(pCorpse->def), TRUE);
                    fMultiTransShadowZBlitter = TRUE;
                  }
                }

                if ((uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC) && !(uiFlags & TILES_DIRTY)) {
                  // Remove flags!
                  pNode->uiFlags &= (~LEVELNODE_LASTDYNAMIC);
                  fZWrite = TRUE;
                }

                if (uiLevelNodeFlags & LEVELNODE_NOWRITEZ) {
                  fZWrite = FALSE;
                }

                if (uiFlags & TILES_NOZWRITE) fZWrite = FALSE;

                if (uiFlags & TILES_NOZ) {
                  fZBlitter = FALSE;
                }

                if ((uiLevelNodeFlags & LEVELNODE_WIREFRAME)) {
                  if (!gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME]) {
                    fTileInvisible = TRUE;
                  }
                }

                // RENDER
                if (fTileInvisible) {
                } else if (uiLevelNodeFlags & LEVELNODE_DISPLAY_AP && !(uiFlags & TILES_DIRTY)) {
                  pTrav = &(hVObject->pETRLEObject[usImageIndex]);
                  sXPos += pTrav->sOffsetX;
                  sYPos += pTrav->sOffsetY;

                  if (gfUIDisplayActionPointsInvalid) {
                    SetFontBackground(FONT_MCOLOR_BLACK);
                    SetFontForeground(FONT_MCOLOR_WHITE);
                  } else {
                    SetFontBackground(FONT_MCOLOR_BLACK);
                    SetFontForeground(FONT_MCOLOR_WHITE);
                  }

                  if (gfUIDisplayActionPointsBlack) {
                    SetFontBackground(FONT_MCOLOR_BLACK);
                    SetFontForeground(FONT_MCOLOR_BLACK);
                  }

                  SetFont(TINYFONT1);
                  SetFontDestBuffer(vsSaveBuffer, 0, gsVIEWPORT_WINDOW_START_Y, 640,
                                    gsVIEWPORT_WINDOW_END_Y, FALSE);
                  VarFindFontCenterCoordinates(sXPos, sYPos, 1, 1, TINYFONT1, &sX, &sY, L"%d",
                                               pNode->uiAPCost);
                  mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY, L"%d",
                                 pNode->uiAPCost);
                  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
                } else if ((uiLevelNodeFlags & LEVELNODE_ERASEZ) && !(uiFlags & TILES_DIRTY)) {
                  Zero8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                       hVObject, sXPos, sYPos, usImageIndex);
                } else if ((uiLevelNodeFlags & LEVELNODE_ITEM) && !(uiFlags & TILES_DIRTY)) {
                  BOOLEAN fZBlit = FALSE;

                  if (uiRowFlags == TILES_STATIC_ONROOF || uiRowFlags == TILES_DYNAMIC_ONROOF) {
                    usOutlineColor = gusYellowItemOutlineColor;
                    bItemOutline = TRUE;
                    fZBlit = TRUE;
                  } else {
                    usOutlineColor = gusNormalItemOutlineColor;
                    bItemOutline = TRUE;
                    fZBlit = TRUE;
                  }

                  if (gGameSettings.fOptions[TOPTION_GLOW_ITEMS]) {
                    if (uiRowFlags == TILES_STATIC_ONROOF || uiRowFlags == TILES_DYNAMIC_ONROOF) {
                      usOutlineColor = us16BPPItemCycleYellowColors[gsCurrentItemGlowFrame];
                      bItemOutline = TRUE;
                    } else {
                      if (gTacticalStatus.uiFlags & RED_ITEM_GLOW_ON) {
                        usOutlineColor = us16BPPItemCycleRedColors[gsCurrentItemGlowFrame];
                        bItemOutline = TRUE;
                      } else {
                        usOutlineColor = us16BPPItemCycleWhiteColors[gsCurrentItemGlowFrame];
                        bItemOutline = TRUE;
                      }
                    }
                  }

                  // else
                  //{
                  //	usOutlineColor = us16BPPItemCycleWhiteColors[ pItemPool->bFlashColor ];
                  //	bItemOutline = TRUE;
                  //}

                  bBlitClipVal =
                      BltIsClippedOrOffScreen(hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);

                  if (bBlitClipVal == FALSE) {
                    if (fZBlit) {
                      if (fObscuredBlitter) {
                        Blt8BPPDataTo16BPPBufferOutlineZPixelateObscured(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                            sXPos, sYPos, usImageIndex, usOutlineColor, bItemOutline);
                      } else {
                        Blt8BPPDataTo16BPPBufferOutlineZ(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                            sXPos, sYPos, usImageIndex, usOutlineColor, bItemOutline);
                      }
                    } else {
                      Blt8BPPDataTo16BPPBufferOutline((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                      hVObject, sXPos, sYPos, usImageIndex,
                                                      usOutlineColor, bItemOutline);
                    }
                  } else if (bBlitClipVal == TRUE) {
                    if (fZBlit) {
                      if (fObscuredBlitter) {
                        Blt8BPPDataTo16BPPBufferOutlineZPixelateObscuredClip(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                            sXPos, sYPos, usImageIndex, usOutlineColor, bItemOutline,
                            &gClippingRect);
                      } else {
                        Blt8BPPDataTo16BPPBufferOutlineZClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                             gpZBuffer, sZLevel, hVObject, sXPos,
                                                             sYPos, usImageIndex, usOutlineColor,
                                                             bItemOutline, &gClippingRect);
                      }
                    } else {
                      Blt8BPPDataTo16BPPBufferOutlineClip(
                          (uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos,
                          usImageIndex, usOutlineColor, bItemOutline, &gClippingRect);
                    }
                  }
                }
                // ATE: Check here for a lot of conditions!
                else if (((uiLevelNodeFlags & LEVELNODE_PHYSICSOBJECT)) &&
                         !(uiFlags & TILES_DIRTY)) {
                  bItemOutline = TRUE;

                  if (uiLevelNodeFlags & LEVELNODE_PHYSICSOBJECT) {
                    bItemOutline = FALSE;
                  }

                  bBlitClipVal =
                      BltIsClippedOrOffScreen(hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);

                  if (fShadowBlitter) {
                    if (bBlitClipVal == FALSE) {
                      Blt8BPPDataTo16BPPBufferShadowZNB((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                        gpZBuffer, sZLevel, hVObject, sXPos, sYPos,
                                                        usImageIndex);
                    } else {
                      Blt8BPPDataTo16BPPBufferShadowZNBClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                            gpZBuffer, sZLevel, hVObject, sXPos,
                                                            sYPos, usImageIndex, &gClippingRect);
                    }
                  } else {
                    if (bBlitClipVal == FALSE) {
                      Blt8BPPDataTo16BPPBufferOutlineZNB(
                          (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                          sXPos, sYPos, usImageIndex, usOutlineColor, bItemOutline);
                    } else if (bBlitClipVal == TRUE) {
                      Blt8BPPDataTo16BPPBufferOutlineClip(
                          (uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos,
                          usImageIndex, usOutlineColor, bItemOutline, &gClippingRect);
                    }
                  }
                } else if (uiFlags & TILES_DIRTY) {
                  if (!(uiLevelNodeFlags & LEVELNODE_LASTDYNAMIC)) {
                    pTrav = &(hVObject->pETRLEObject[usImageIndex]);
                    uiBrushHeight = (uint32_t)pTrav->usHeight;
                    uiBrushWidth = (uint32_t)pTrav->usWidth;
                    sXPos += pTrav->sOffsetX;
                    sYPos += pTrav->sOffsetY;

                    RegisterBackgroundRect(
                        uiDirtyFlags, NULL, sXPos, sYPos, (int16_t)(sXPos + uiBrushWidth),
                        (int16_t)(min((int16_t)(sYPos + uiBrushHeight), gsVIEWPORT_WINDOW_END_Y)));

                    if (fSaveZ) {
                      RegisterBackgroundRect(uiDirtyFlags | BGND_FLAG_SAVE_Z, NULL, sXPos, sYPos,
                                             (int16_t)(sXPos + uiBrushWidth),
                                             (int16_t)(min((int16_t)(sYPos + uiBrushHeight),
                                                           gsVIEWPORT_WINDOW_END_Y)));
                    }
                  }
                } else {
                  if (fMultiTransShadowZBlitter) {
                    if (fZBlitter) {
                      if (fObscuredBlitter) {
                        Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                            sXPos, sYPos, usImageIndex, &gClippingRect,
                            sMultiTransShadowZBlitterIndex, pShadeTable);
                      } else {
                        Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                            sXPos, sYPos, usImageIndex, &gClippingRect,
                            sMultiTransShadowZBlitterIndex, pShadeTable);
                      }
                    } else {
                      // Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t*)pDestBuf,
                      // uiDestPitchBYTES, hVObject, sXPos, sYPos, usImageIndex, &gClippingRect );
                    }
                  } else if (fMultiZBlitter) {
                    if (fZBlitter) {
                      if (fObscuredBlitter) {
                        Blt8BPPDataTo16BPPBufferTransZIncObscureClip(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                            sXPos, sYPos, usImageIndex, &gClippingRect);
                      } else {
                        if (fWallTile) {
                          Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                              sXPos, sYPos, usImageIndex, &gClippingRect);
                        } else {
                          Blt8BPPDataTo16BPPBufferTransZIncClip(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                              sXPos, sYPos, usImageIndex, &gClippingRect);
                        }
                      }
                    } else {
                      Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf,
                                                              uiDestPitchBYTES, hVObject, sXPos,
                                                              sYPos, usImageIndex, &gClippingRect);
                    }
                  } else {
                    bBlitClipVal = BltIsClippedOrOffScreen(hVObject, sXPos, sYPos, usImageIndex,
                                                           &gClippingRect);

                    if (bBlitClipVal == TRUE) {
                      if (fPixelate) {
                        if (fTranslucencyType) {
                          Blt8BPPDataTo16BPPBufferTransZNBClipTranslucent(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                              sXPos, sYPos, usImageIndex, &gClippingRect);
                        } else {
                          Blt8BPPDataTo16BPPBufferTransZNBClipPixelate(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                              sXPos, sYPos, usImageIndex, &gClippingRect);
                        }
                      } else if (fMerc) {
                        if (fZBlitter) {
                          if (fZWrite) {
                            Blt8BPPDataTo16BPPBufferTransShadowZClip(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect, pShadeTable);
                          } else {
                            if (fObscuredBlitter) {
                              Blt8BPPDataTo16BPPBufferTransShadowZNBObscuredClip(
                                  (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                  hVObject, sXPos, sYPos, usImageIndex, &gClippingRect,
                                  pShadeTable);
                            } else {
                              Blt8BPPDataTo16BPPBufferTransShadowZNBClip(
                                  (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                  hVObject, sXPos, sYPos, usImageIndex, &gClippingRect,
                                  pShadeTable);
                            }
                          }

                          if ((uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)) {
                            pSaveBuf = LockVSurface(vsSaveBuffer, &uiSaveBufferPitchBYTES);

                            // BLIT HERE
                            Blt8BPPDataTo16BPPBufferTransShadowClip(
                                (uint16_t *)pSaveBuf, uiSaveBufferPitchBYTES, hVObject, sXPos,
                                sYPos, usImageIndex, &gClippingRect, pShadeTable);

                            JSurface_Unlock(vsSaveBuffer);

                            // Turn it off!
                            pNode->uiFlags &= (~LEVELNODE_UPDATESAVEBUFFERONCE);
                          }

                        } else {
                          Blt8BPPDataTo16BPPBufferTransShadowClip(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos,
                              usImageIndex, &gClippingRect, pShadeTable);
                        }
                      } else if (fShadowBlitter) {
                        if (fZBlitter) {
                          if (fZWrite)
                            Blt8BPPDataTo16BPPBufferShadowZClip(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
                          else
                            Blt8BPPDataTo16BPPBufferShadowZClip(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
                        } else {
                          Blt8BPPDataTo16BPPBufferShadowClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                             hVObject, sXPos, sYPos, usImageIndex,
                                                             &gClippingRect);
                        }
                      } else if (fIntensityBlitter) {
                        if (fZBlitter) {
                          if (fZWrite)
                            Blt8BPPDataTo16BPPBufferIntensityZClip(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
                          else
                            Blt8BPPDataTo16BPPBufferIntensityZClip(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
                        } else {
                          Blt8BPPDataTo16BPPBufferIntensityClip(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos,
                              usImageIndex, &gClippingRect);
                        }
                      } else if (fZBlitter) {
                        if (fZWrite) {
                          if (fObscuredBlitter) {
                            Blt8BPPDataTo16BPPBufferTransZClipPixelateObscured(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
                          } else {
                            Blt8BPPDataTo16BPPBufferTransZClip(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);
                          }
                        } else {
                          Blt8BPPDataTo16BPPBufferTransZNBClip(
                              (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject,
                              sXPos, sYPos, usImageIndex, &gClippingRect);
                        }

                        if ((uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)) {
                          pSaveBuf = LockVSurface(vsSaveBuffer, &uiSaveBufferPitchBYTES);

                          // BLIT HERE
                          Blt8BPPDataTo16BPPBufferTransZClip(
                              (uint16_t *)pSaveBuf, uiSaveBufferPitchBYTES, gpZBuffer, sZLevel,
                              hVObject, sXPos, sYPos, usImageIndex, &gClippingRect);

                          JSurface_Unlock(vsSaveBuffer);

                          // Turn it off!
                          pNode->uiFlags &= (~LEVELNODE_UPDATESAVEBUFFERONCE);
                        }

                      } else
                        Blt8BPPDataTo16BPPBufferTransparentClip(
                            (uint16_t *)pDestBuf, uiDestPitchBYTES, hVObject, sXPos, sYPos,
                            usImageIndex, &gClippingRect);

                    } else if (bBlitClipVal == FALSE) {
                      if (fPixelate) {
                        if (fTranslucencyType) {
                          if (fZWrite)
                            Blt8BPPDataTo16BPPBufferTransZTranslucent(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                          else
                            Blt8BPPDataTo16BPPBufferTransZNBTranslucent(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                        } else {
                          if (fZWrite)
                            Blt8BPPDataTo16BPPBufferTransZPixelate(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                          else
                            Blt8BPPDataTo16BPPBufferTransZNBPixelate(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                        }
                      } else if (fMerc) {
                        if (fZBlitter) {
                          if (fZWrite) {
                            Blt8BPPDataTo16BPPBufferTransShadowZ(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
                          } else {
                            if (fObscuredBlitter) {
                              Blt8BPPDataTo16BPPBufferTransShadowZNBObscured(
                                  (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                  hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
                            } else {
                              Blt8BPPDataTo16BPPBufferTransShadowZNB(
                                  (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                  hVObject, sXPos, sYPos, usImageIndex, pShadeTable);
                            }
                          }

                          if ((uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)) {
                            pSaveBuf = LockVSurface(vsSaveBuffer, &uiSaveBufferPitchBYTES);

                            // BLIT HERE
                            Blt8BPPDataTo16BPPBufferTransShadow(
                                (uint16_t *)pSaveBuf, uiSaveBufferPitchBYTES, hVObject, sXPos,
                                sYPos, usImageIndex, pShadeTable);

                            JSurface_Unlock(vsSaveBuffer);

                            // Turn it off!
                            pNode->uiFlags &= (~LEVELNODE_UPDATESAVEBUFFERONCE);
                          }

                        } else {
                          Blt8BPPDataTo16BPPBufferTransShadow((uint16_t *)pDestBuf,
                                                              uiDestPitchBYTES, hVObject, sXPos,
                                                              sYPos, usImageIndex, pShadeTable);
                        }
                      } else if (fShadowBlitter) {
                        if (fZBlitter) {
                          if (fZWrite)
                            Blt8BPPDataTo16BPPBufferShadowZ((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                            gpZBuffer, sZLevel, hVObject, sXPos,
                                                            sYPos, usImageIndex);
                          else
                            Blt8BPPDataTo16BPPBufferShadowZNB((uint16_t *)pDestBuf,
                                                              uiDestPitchBYTES, gpZBuffer, sZLevel,
                                                              hVObject, sXPos, sYPos, usImageIndex);
                        } else {
                          Blt8BPPDataTo16BPPBufferShadow((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                         hVObject, sXPos, sYPos, usImageIndex);
                        }
                      } else if (fIntensityBlitter) {
                        if (fZBlitter) {
                          if (fZWrite)
                            Blt8BPPDataTo16BPPBufferIntensityZ(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                          else
                            Blt8BPPDataTo16BPPBufferIntensityZNB(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                        } else {
                          Blt8BPPDataTo16BPPBufferIntensity((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                            hVObject, sXPos, sYPos, usImageIndex);
                        }
                      } else if (fZBlitter) {
                        if (fZWrite) {
                          // TEST
                          // Blt8BPPDataTo16BPPBufferTransZPixelate( (uint16_t*)pDestBuf,
                          // uiDestPitchBYTES, gpZBuffer, sZLevel, hVObject, sXPos, sYPos,
                          // usImageIndex);

                          if (fObscuredBlitter) {
                            Blt8BPPDataTo16BPPBufferTransZPixelateObscured(
                                (uint16_t *)pDestBuf, uiDestPitchBYTES, gpZBuffer, sZLevel,
                                hVObject, sXPos, sYPos, usImageIndex);
                          } else {
                            Blt8BPPDataTo16BPPBufferTransZ((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                           gpZBuffer, sZLevel, hVObject, sXPos,
                                                           sYPos, usImageIndex);
                          }
                        } else
                          Blt8BPPDataTo16BPPBufferTransZNB((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                           gpZBuffer, sZLevel, hVObject, sXPos,
                                                           sYPos, usImageIndex);

                        if ((uiLevelNodeFlags & LEVELNODE_UPDATESAVEBUFFERONCE)) {
                          pSaveBuf = LockVSurface(vsSaveBuffer, &uiSaveBufferPitchBYTES);

                          // BLIT HERE
                          Blt8BPPDataTo16BPPBufferTransZ((uint16_t *)pSaveBuf,
                                                         uiSaveBufferPitchBYTES, gpZBuffer, sZLevel,
                                                         hVObject, sXPos, sYPos, usImageIndex);

                          JSurface_Unlock(vsSaveBuffer);

                          // Turn it off!
                          pNode->uiFlags &= (~LEVELNODE_UPDATESAVEBUFFERONCE);
                        }

                      } else
                        Blt8BPPDataTo16BPPBufferTransparent((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                            hVObject, sXPos, sYPos, usImageIndex);
                    }
                  }
                }

                // RENDR APS ONTOP OF PLANNED MERC GUY
                if (fRenderTile && !(uiFlags & TILES_DIRTY)) {
                  if (fMerc) {
                    if (pSoldier != NULL && GetSolID(pSoldier) >= MAX_NUM_SOLDIERS) {
                      SetFont(TINYFONT1);
                      SetFontDestBuffer(vsSaveBuffer, 0, gsVIEWPORT_WINDOW_START_Y, 640,
                                        gsVIEWPORT_WINDOW_END_Y, FALSE);
                      VarFindFontCenterCoordinates(sXPos, sYPos, 1, 1, TINYFONT1, &sX, &sY, L"%d",
                                                   pSoldier->ubPlannedUIAPCost);
                      mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY, L"%d",
                                     pSoldier->ubPlannedUIAPCost);
                      SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
                    }
                  }
                }
              }

              if (!fLinkedListDirection)
                pNode = pNode->pPrevNode;
              else
                pNode = pNode->pNext;

              // pNode = NULL;
            }
          } else {
            if (gfEditMode) {
              // ATE: Used here in the editor to denote then an area is not in the world

              // Kris:  Fixed a couple things here...
              //  First, there was a problem with the vsFB already being locked which caused
              //  failures,
              //	and eventual crashes, so if it reaches this code, the buffer needs to be
              // unlocked first, as
              //  it gets locked and unlocked internally within ColorFillVSurfaceArea().  I'm
              //  surprised
              //	this problem didn't surface a long time ago.  Anyway, it seems that
              // scrolling to the bottom 	right hand corner of the map, would cause the end of
              // the world to be drawn.  Now, this would 	only crash on my computer and not
              // Emmons, so
              // this should work.  Also, I changed the color 	from fluorescent green to black,
              // which
              // is easier on the eyes, and prevent the drawing of the 	end of the world if it would
              // be drawn on the editor's taskbar.
              if (iTempPosY_S < 360) {
                if (!(uiFlags & TILES_DIRTY)) JSurface_Unlock(vsFB);
                ColorFillVSurfaceArea(vsFB, iTempPosX_S, iTempPosY_S, (int16_t)(iTempPosX_S + 40),
                                      (int16_t)(min(iTempPosY_S + 20, 360)),
                                      rgb32_to_rgb16(FROMRGB(0, 0, 0)));
                if (!(uiFlags & TILES_DIRTY)) pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
              }
            }
          }

          iTempPosX_S += 40;
          iTempPosX_M++;
          iTempPosY_M--;

          if (iTempPosX_S >= iEndXS) {
            fEndRenderRow = TRUE;
          }

        } while (!fEndRenderRow);
      }
    }
    //	} while( FALSE );

    if (bXOddFlag > 0) {
      iAnchorPosY_M++;
    } else {
      iAnchorPosX_M++;
    }

    bXOddFlag = !bXOddFlag;
    iAnchorPosY_S += 10;

    if (iAnchorPosY_S >= iEndYS) {
      fEndRenderCol = TRUE;
    }
  } while (!fEndRenderCol);

  if (!(uiFlags & TILES_DIRTY)) JSurface_Unlock(vsFB);

  if (uiFlags & TILES_DYNAMIC_CHECKFOR_INT_TILE) {
    EndCurInteractiveTileCheck();
  }
}

void DeleteFromWorld(uint16_t usTileIndex, uint32_t uiRenderTiles, uint16_t usIndex) {
  switch (uiRenderTiles) {
    case TILES_DYNAMIC_LAND:
    case TILES_STATIC_LAND:
      RemoveLand(usTileIndex, usIndex);
      break;
    case TILES_DYNAMIC_OBJECTS:
    case TILES_STATIC_OBJECTS:
      RemoveObject(usTileIndex, usIndex);
      break;
    case TILES_STATIC_STRUCTURES:
    case TILES_DYNAMIC_STRUCTURES:
      RemoveStruct(usTileIndex, usIndex);
      break;
    case TILES_DYNAMIC_ROOF:
    case TILES_STATIC_ROOF:
      RemoveRoof(usTileIndex, usIndex);
      break;
    case TILES_STATIC_ONROOF:
      RemoveOnRoof(usTileIndex, usIndex);
      break;

    case TILES_DYNAMIC_TOPMOST:
    case TILES_STATIC_TOPMOST:
      RemoveTopmost(usTileIndex, usIndex);
      break;
  }
}

// memcpy's the background to the new scroll position, and renders the missing strip
// via the RenderStaticWorldRect. Dynamic stuff will be updated on the next frame
// by the normal render cycle
void ScrollBackground(uint32_t uiDirection, int16_t sScrollXIncrement, int16_t sScrollYIncrement) {
  // RestoreBackgroundRects();

  if (!gfDoVideoScroll) {
    // Clear z-buffer
    memset(gpZBuffer, LAND_Z_LEVEL, 1280 * gsVIEWPORT_END_Y);

    RenderStaticWorldRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X,
                          gsVIEWPORT_END_Y, FALSE);

    FreeBackgroundRectType(BGND_FLAG_ANIMATED);
  } else {
    if (gfRenderScroll == FALSE) {
      guiScrollDirection = uiDirection;
      gfScrollStart = TRUE;
      gsScrollXIncrement = 0;
      gsScrollYIncrement = 0;
    } else {
      guiScrollDirection |= uiDirection;
      gfScrollStart = FALSE;
    }

    gfRenderScroll = TRUE;
    gsScrollXIncrement += sScrollXIncrement;
    gsScrollYIncrement += sScrollYIncrement;
  }
}

// Render routine takes center X, Y and Z coordinate and gets world
// Coordinates for the window from that using the following functions
// For coordinate transformations

void RenderWorld() {
  TILE_ELEMENT *TileElem;
  TILE_ANIMATION_DATA *pAnimData;
  uint32_t cnt = 0;

  gfRenderFullThisFrame = FALSE;

  // If we are testing renderer, set background to pink!
  if (gTacticalStatus.uiFlags & DEBUGCLIFFS) {
    ColorFillVSurfaceArea(vsFB, 0, gsVIEWPORT_WINDOW_START_Y, 640, gsVIEWPORT_WINDOW_END_Y,
                          rgb32_to_rgb16(FROMRGB(0, 255, 0)));
    SetRenderFlags(RENDER_FLAG_FULL);
  }

  if (gTacticalStatus.uiFlags & SHOW_Z_BUFFER) {
    SetRenderFlags(RENDER_FLAG_FULL);
  }

  //	SetRenderFlags(RENDER_FLAG_FULL);

  // FOR NOW< HERE, UPDATE ANIMATED TILES
  if (COUNTERDONE(ANIMATETILES)) {
    RESETCOUNTER(ANIMATETILES);

    while (cnt < gusNumAnimatedTiles) {
      TileElem = &(gTileDatabase[gusAnimatedTiles[cnt]]);

      pAnimData = TileElem->pAnimData;

      Assert(pAnimData != NULL);

      pAnimData->bCurrentFrame++;

      if (pAnimData->bCurrentFrame >= pAnimData->ubNumFrames) pAnimData->bCurrentFrame = 0;
      cnt++;
    }
  }

  // HERE, UPDATE GLOW INDEX
  if (COUNTERDONE(GLOW_ENEMYS)) {
    RESETCOUNTER(GLOW_ENEMYS);

    gsCurrentGlowFrame++;

    if (gsCurrentGlowFrame == NUM_GLOW_FRAMES) {
      gsCurrentGlowFrame = 0;
    }

    gsCurrentItemGlowFrame++;

    if (gsCurrentItemGlowFrame == NUM_ITEM_CYCLE_COLORS) {
      gsCurrentItemGlowFrame = 0;
    }
  }

  // RenderStaticWorldRect( gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X,
  // gsVIEWPORT_END_Y ); AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X,
  // gsVIEWPORT_END_Y ); return;

  // #if 0

  if (gRenderFlags & RENDER_FLAG_FULL) {
    gfRenderFullThisFrame = TRUE;

    gfTopMessageDirty = TRUE;

    // Dirty the interface...
    fInterfacePanelDirty = DIRTYLEVEL2;

    // Apply scrolling sets some world variables
    ApplyScrolling(gsRenderCenterX, gsRenderCenterY, TRUE, FALSE);
    ResetLayerOptimizing();

    if ((gRenderFlags & RENDER_FLAG_NOZ)) {
      RenderStaticWorldRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X,
                            gsVIEWPORT_END_Y, FALSE);
    } else {
      RenderStaticWorld();
    }

    if (!(gRenderFlags & RENDER_FLAG_SAVEOFF)) UpdateSaveBuffer();

  } else if (gRenderFlags & RENDER_FLAG_MARKED) {
    ResetLayerOptimizing();
    RenderMarkedWorld();
    if (!(gRenderFlags & RENDER_FLAG_SAVEOFF)) UpdateSaveBuffer();
  }

  if (gfScrollInertia == FALSE || (gRenderFlags & RENDER_FLAG_NOZ) ||
      (gRenderFlags & RENDER_FLAG_FULL) || (gRenderFlags & RENDER_FLAG_MARKED)) {
    RenderDynamicWorld();
  }

  if (gfScrollInertia) {
    EmptyBackgroundRects();
  }

  if (gRenderFlags & RENDER_FLAG_ROOMIDS) {
    RenderRoomInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
                   gsEndYS);
  }

#ifdef _DEBUG
  if (gRenderFlags & RENDER_FLAG_FOVDEBUG) {
    RenderFOVDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
                       gsEndYS);
  } else if (gfDisplayCoverValues) {
    RenderCoverDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
                         gsEndXS, gsEndYS);
  } else if (gfDisplayGridNoVisibleValues) {
    RenderGridNoVisibleDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
                                 gsEndXS, gsEndYS);
  }

#endif

  // #endif

  // RenderStaticWorldRect( gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X,
  // gsVIEWPORT_END_Y ); AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X,
  // gsVIEWPORT_END_Y );

  if (gRenderFlags & RENDER_FLAG_MARKED) ClearMarkedTiles();

  if (gRenderFlags & RENDER_FLAG_CHECKZ && !(gTacticalStatus.uiFlags & NOHIDE_REDUNDENCY)) {
    ExamineZBufferRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X,
                       gsVIEWPORT_WINDOW_END_Y);
  }

  gRenderFlags &=
      (~(RENDER_FLAG_FULL | RENDER_FLAG_MARKED | RENDER_FLAG_ROOMIDS | RENDER_FLAG_CHECKZ));

  if (gTacticalStatus.uiFlags & SHOW_Z_BUFFER) {
    // COPY Z BUFFER TO FRAME BUFFER
    uint32_t uiDestPitchBYTES;
    uint16_t *pDestBuf;
    uint32_t cnt;
    wchar_t zVal;

    pDestBuf = (uint16_t *)LockVSurface(vsFB, &uiDestPitchBYTES);

    for (cnt = 0; cnt < (640 * 480); cnt++) {
      // Get Z value
      zVal = gpZBuffer[cnt];
      pDestBuf[cnt] = zVal;
    }

    JSurface_Unlock(vsFB);
  }
}

// Start with a center X,Y,Z world coordinate and render direction
// Determine WorldIntersectionPoint and the starting block from these
// Then render away!
void RenderStaticWorldRect(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom,
                           BOOLEAN fDynamicsToo) {
  uint32_t uiLevelFlags[10];
  uint16_t sLevelIDs[10];

  // Calculate render starting parameters
  CalcRenderParameters(sLeft, sTop, sRight, sBottom);

  // Reset layer optimizations
  ResetLayerOptimizing();

  // STATICS
  uiLevelFlags[0] = TILES_STATIC_LAND;
  // uiLevelFlags[1] = TILES_STATIC_OBJECTS;
  sLevelIDs[0] = RENDER_STATIC_LAND;
  // sLevelIDs[1]		= RENDER_STATIC_OBJECTS;
  RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
              gsLEndYS, 1, uiLevelFlags, sLevelIDs);

  // #if 0

  uiLevelFlags[0] = TILES_STATIC_OBJECTS;
  sLevelIDs[0] = RENDER_STATIC_OBJECTS;
  RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
              gsLEndYS, 1, uiLevelFlags, sLevelIDs);

  if (gRenderFlags & RENDER_FLAG_SHADOWS) {
    uiLevelFlags[0] = TILES_STATIC_SHADOWS;
    sLevelIDs[0] = RENDER_STATIC_SHADOWS;
    RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
                gsLEndYS, 1, uiLevelFlags, sLevelIDs);
  }

  uiLevelFlags[0] = TILES_STATIC_STRUCTURES;
  uiLevelFlags[1] = TILES_STATIC_ROOF;
  uiLevelFlags[2] = TILES_STATIC_ONROOF;
  uiLevelFlags[3] = TILES_STATIC_TOPMOST;

  sLevelIDs[0] = RENDER_STATIC_STRUCTS;
  sLevelIDs[1] = RENDER_STATIC_ROOF;
  sLevelIDs[2] = RENDER_STATIC_ONROOF;
  sLevelIDs[3] = RENDER_STATIC_TOPMOST;

  RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
              gsLEndYS, 4, uiLevelFlags, sLevelIDs);

  // ATE: Do obsucred layer!
  uiLevelFlags[0] = TILES_STATIC_STRUCTURES;
  sLevelIDs[0] = RENDER_STATIC_STRUCTS;
  uiLevelFlags[1] = TILES_STATIC_ONROOF;
  sLevelIDs[1] = RENDER_STATIC_ONROOF;
  RenderTiles(TILES_OBSCURED, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S,
              gsLStartPointY_S, gsLEndXS, gsLEndYS, 2, uiLevelFlags, sLevelIDs);

  // uiLevelFlags[0] = TILES_DYNAMIC_MERCS;
  // uiLevelFlags[1] = TILES_DYNAMIC_HIGHMERCS;

  // sLevelIDs[0]    = RENDER_DYNAMIC_MERCS;
  // sLevelIDs[1]		= RENDER_DYNAMIC_HIGHMERCS;
  // RenderTiles( 0, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
  // gsEndYS, 1, uiLevelFlags, sLevelIDs );

  if (fDynamicsToo) {
    // DYNAMICS
    uiLevelFlags[0] = TILES_DYNAMIC_LAND;
    uiLevelFlags[1] = TILES_DYNAMIC_OBJECTS;
    uiLevelFlags[2] = TILES_DYNAMIC_SHADOWS;
    uiLevelFlags[3] = TILES_DYNAMIC_STRUCT_MERCS;
    uiLevelFlags[4] = TILES_DYNAMIC_MERCS;
    uiLevelFlags[5] = TILES_DYNAMIC_STRUCTURES;
    uiLevelFlags[6] = TILES_DYNAMIC_ROOF;
    uiLevelFlags[7] = TILES_DYNAMIC_HIGHMERCS;
    uiLevelFlags[8] = TILES_DYNAMIC_ONROOF;

    sLevelIDs[0] = RENDER_DYNAMIC_LAND;
    sLevelIDs[1] = RENDER_DYNAMIC_OBJECTS;
    sLevelIDs[2] = RENDER_DYNAMIC_SHADOWS;
    sLevelIDs[3] = RENDER_DYNAMIC_STRUCT_MERCS;
    sLevelIDs[4] = RENDER_DYNAMIC_MERCS;
    sLevelIDs[5] = RENDER_DYNAMIC_STRUCTS;
    sLevelIDs[6] = RENDER_DYNAMIC_ROOF;
    sLevelIDs[7] = RENDER_DYNAMIC_HIGHMERCS;
    sLevelIDs[8] = RENDER_DYNAMIC_ONROOF;
    RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
                gsLEndYS, 9, uiLevelFlags, sLevelIDs);

    SumAddiviveLayerOptimization();
  }

  ResetRenderParameters();

  if (!gfDoVideoScroll) {
    // AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X,
    // gsVIEWPORT_WINDOW_END_Y );
    AddBaseDirtyRect(sLeft, sTop, sRight, sBottom);
  }

  // #endif
}

void RenderStaticWorld() {
  uint32_t uiLevelFlags[9];
  uint16_t sLevelIDs[9];

  // Calculate render starting parameters
  CalcRenderParameters(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);

  // Clear z-buffer
  memset(gpZBuffer, LAND_Z_LEVEL, 1280 * gsVIEWPORT_END_Y);

  FreeBackgroundRectType(BGND_FLAG_ANIMATED);
  InvalidateBackgroundRects();

  uiLevelFlags[0] = TILES_STATIC_LAND;
  // uiLevelFlags[1] = TILES_STATIC_OBJECTS;
  sLevelIDs[0] = RENDER_STATIC_LAND;
  // sLevelIDs[1]		= RENDER_STATIC_OBJECTS;
  RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
              gsLEndYS, 1, uiLevelFlags, sLevelIDs);

  uiLevelFlags[0] = TILES_STATIC_OBJECTS;
  sLevelIDs[0] = RENDER_STATIC_OBJECTS;
  RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
              gsLEndYS, 1, uiLevelFlags, sLevelIDs);

  if (gRenderFlags & RENDER_FLAG_SHADOWS) {
    uiLevelFlags[0] = TILES_STATIC_SHADOWS;
    sLevelIDs[0] = RENDER_STATIC_SHADOWS;
    RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
                gsLEndYS, 1, uiLevelFlags, sLevelIDs);
  }

  uiLevelFlags[0] = TILES_STATIC_STRUCTURES;
  uiLevelFlags[1] = TILES_STATIC_ROOF;
  uiLevelFlags[2] = TILES_STATIC_ONROOF;
  uiLevelFlags[3] = TILES_STATIC_TOPMOST;

  sLevelIDs[0] = RENDER_STATIC_STRUCTS;
  sLevelIDs[1] = RENDER_STATIC_ROOF;
  sLevelIDs[2] = RENDER_STATIC_ONROOF;
  sLevelIDs[3] = RENDER_STATIC_TOPMOST;

  RenderTiles(0, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S, gsLStartPointY_S, gsLEndXS,
              gsLEndYS, 4, uiLevelFlags, sLevelIDs);

  // ATE: Do obsucred layer!
  uiLevelFlags[0] = TILES_STATIC_STRUCTURES;
  sLevelIDs[0] = RENDER_STATIC_STRUCTS;
  uiLevelFlags[1] = TILES_STATIC_ONROOF;
  sLevelIDs[1] = RENDER_STATIC_ONROOF;
  RenderTiles(TILES_OBSCURED, gsLStartPointX_M, gsLStartPointY_M, gsLStartPointX_S,
              gsLStartPointY_S, gsLEndXS, gsLEndYS, 2, uiLevelFlags, sLevelIDs);

  AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X,
                   gsVIEWPORT_WINDOW_END_Y);

  ResetRenderParameters();
}

void RenderMarkedWorld(void) {
  uint32_t uiLevelFlags[4];
  uint16_t sLevelIDs[4];

  CalcRenderParameters(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);

  RestoreBackgroundRects();
  FreeBackgroundRectType(BGND_FLAG_ANIMATED);
  InvalidateBackgroundRects();

  ResetLayerOptimizing();

  uiLevelFlags[0] = TILES_STATIC_LAND;
  uiLevelFlags[1] = TILES_STATIC_OBJECTS;
  sLevelIDs[0] = RENDER_STATIC_LAND;
  sLevelIDs[1] = RENDER_STATIC_OBJECTS;
  RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
              gsEndXS, gsEndYS, 2, uiLevelFlags, sLevelIDs);

  if (gRenderFlags & RENDER_FLAG_SHADOWS) {
    uiLevelFlags[0] = TILES_STATIC_SHADOWS;
    sLevelIDs[0] = RENDER_STATIC_SHADOWS;
    RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
                gsEndXS, gsEndYS, 1, uiLevelFlags, sLevelIDs);
  }

  uiLevelFlags[0] = TILES_STATIC_STRUCTURES;
  sLevelIDs[0] = RENDER_STATIC_STRUCTS;
  RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
              gsEndXS, gsEndYS, 1, uiLevelFlags, sLevelIDs);

  uiLevelFlags[0] = TILES_STATIC_ROOF;
  sLevelIDs[0] = RENDER_STATIC_ROOF;
  RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
              gsEndXS, gsEndYS, 1, uiLevelFlags, sLevelIDs);

  uiLevelFlags[0] = TILES_STATIC_ONROOF;
  sLevelIDs[0] = RENDER_STATIC_ONROOF;
  RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
              gsEndXS, gsEndYS, 1, uiLevelFlags, sLevelIDs);

  uiLevelFlags[0] = TILES_STATIC_TOPMOST;
  sLevelIDs[0] = RENDER_STATIC_TOPMOST;
  RenderTiles(TILES_MARKED, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
              gsEndXS, gsEndYS, 1, uiLevelFlags, sLevelIDs);

  AddBaseDirtyRect(gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y, gsVIEWPORT_END_X,
                   gsVIEWPORT_WINDOW_END_Y);

  ResetRenderParameters();
}

void RenderDynamicWorld() {
  uint8_t ubNumLevels;
  uint32_t uiLevelFlags[10];
  uint16_t sLevelIDs[10];

  CalcRenderParameters(gsVIEWPORT_START_X, gsVIEWPORT_START_Y, gsVIEWPORT_END_X, gsVIEWPORT_END_Y);

  RestoreBackgroundRects();

  if (!gfTagAnimatedTiles) {
    uiLevelFlags[0] = TILES_DYNAMIC_OBJECTS;
    uiLevelFlags[1] = TILES_DYNAMIC_SHADOWS;
    uiLevelFlags[2] = TILES_DYNAMIC_STRUCT_MERCS;
    uiLevelFlags[3] = TILES_DYNAMIC_MERCS;
    uiLevelFlags[4] = TILES_DYNAMIC_STRUCTURES;
    uiLevelFlags[5] = TILES_DYNAMIC_HIGHMERCS;
    uiLevelFlags[6] = TILES_DYNAMIC_ROOF;
    uiLevelFlags[7] = TILES_DYNAMIC_ONROOF;
    uiLevelFlags[8] = TILES_DYNAMIC_TOPMOST;

    sLevelIDs[0] = RENDER_DYNAMIC_OBJECTS;
    sLevelIDs[1] = RENDER_DYNAMIC_SHADOWS;
    sLevelIDs[2] = RENDER_DYNAMIC_STRUCT_MERCS;
    sLevelIDs[3] = RENDER_DYNAMIC_MERCS;
    sLevelIDs[4] = RENDER_DYNAMIC_STRUCTS;
    sLevelIDs[5] = RENDER_DYNAMIC_MERCS;
    sLevelIDs[6] = RENDER_DYNAMIC_ROOF;
    sLevelIDs[7] = RENDER_DYNAMIC_ONROOF;
    sLevelIDs[8] = RENDER_DYNAMIC_TOPMOST;

    ubNumLevels = 9;
  } else {
    gfTagAnimatedTiles = FALSE;
    // uiLevelFlags[0] = TILES_DYNAMIC_LAND;
    uiLevelFlags[0] = TILES_DYNAMIC_OBJECTS;
    uiLevelFlags[1] = TILES_DYNAMIC_SHADOWS;
    uiLevelFlags[2] = TILES_DYNAMIC_STRUCT_MERCS;
    uiLevelFlags[3] = TILES_DYNAMIC_MERCS;
    uiLevelFlags[4] = TILES_DYNAMIC_STRUCTURES;
    uiLevelFlags[5] = TILES_DYNAMIC_HIGHMERCS;
    uiLevelFlags[6] = TILES_DYNAMIC_ROOF;
    uiLevelFlags[7] = TILES_DYNAMIC_ONROOF;
    uiLevelFlags[8] = TILES_DYNAMIC_TOPMOST;

    // sLevelIDs[0]		= RENDER_DYNAMIC_LAND;
    sLevelIDs[0] = RENDER_DYNAMIC_OBJECTS;
    sLevelIDs[1] = RENDER_DYNAMIC_SHADOWS;
    sLevelIDs[2] = RENDER_DYNAMIC_STRUCT_MERCS;
    sLevelIDs[3] = RENDER_DYNAMIC_MERCS;
    sLevelIDs[4] = RENDER_DYNAMIC_STRUCTS;
    sLevelIDs[5] = RENDER_DYNAMIC_MERCS;
    sLevelIDs[6] = RENDER_DYNAMIC_ROOF;
    sLevelIDs[7] = RENDER_DYNAMIC_ONROOF;
    sLevelIDs[8] = RENDER_DYNAMIC_TOPMOST;

    ubNumLevels = 9;
  }

  RenderTiles(TILES_DIRTY, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
              gsEndXS, gsEndYS, ubNumLevels, uiLevelFlags, sLevelIDs);

#ifdef JA2EDITOR
  if (!gfEditMode && !gfAniEditMode)
#endif
  {
    RenderTacticalInterface();
  }

  SaveBackgroundRects();

  // uiLevelFlags[0] = TILES_DYNAMIC_LAND;
  uiLevelFlags[0] = TILES_DYNAMIC_OBJECTS;
  uiLevelFlags[1] = TILES_DYNAMIC_SHADOWS;
  uiLevelFlags[2] = TILES_DYNAMIC_STRUCT_MERCS;
  uiLevelFlags[3] = TILES_DYNAMIC_MERCS;
  uiLevelFlags[4] = TILES_DYNAMIC_STRUCTURES;

  // sLevelIDs[0]    = RENDER_DYNAMIC_LAND;
  sLevelIDs[0] = RENDER_DYNAMIC_OBJECTS;
  sLevelIDs[1] = RENDER_DYNAMIC_SHADOWS;
  sLevelIDs[2] = RENDER_DYNAMIC_STRUCT_MERCS;
  sLevelIDs[3] = RENDER_DYNAMIC_MERCS;
  sLevelIDs[4] = RENDER_DYNAMIC_STRUCTS;

  RenderTiles(0, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
              gsEndYS, 5, uiLevelFlags, sLevelIDs);

  uiLevelFlags[0] = TILES_DYNAMIC_ROOF;
  uiLevelFlags[1] = TILES_DYNAMIC_HIGHMERCS;
  uiLevelFlags[2] = TILES_DYNAMIC_ONROOF;

  sLevelIDs[0] = RENDER_DYNAMIC_ROOF;
  sLevelIDs[1] = RENDER_DYNAMIC_HIGHMERCS;
  sLevelIDs[2] = RENDER_DYNAMIC_ONROOF;

  RenderTiles(0, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
              gsEndYS, 3, uiLevelFlags, sLevelIDs);

  uiLevelFlags[0] = TILES_DYNAMIC_TOPMOST;
  sLevelIDs[0] = RENDER_DYNAMIC_TOPMOST;

  // ATE: check here for mouse over structs.....
  RenderTiles(TILES_DYNAMIC_CHECKFOR_INT_TILE, gsStartPointX_M, gsStartPointY_M, gsStartPointX_S,
              gsStartPointY_S, gsEndXS, gsEndYS, 1, uiLevelFlags, sLevelIDs);

  SumAddiviveLayerOptimization();

  ResetRenderParameters();
}

BOOLEAN HandleScrollDirections(uint32_t ScrollFlags, int16_t sScrollXStep, int16_t sScrollYStep,
                               int16_t *psTempRenderCenterX, int16_t *psTempRenderCenterY,
                               BOOLEAN fCheckOnly) {
  BOOLEAN fAGoodMove = FALSE, fMovedPos = FALSE;
  int16_t sTempX_W, sTempY_W;
  BOOLEAN fUpOK, fLeftOK;
  BOOLEAN fDownOK, fRightOK;
  int16_t sTempRenderCenterX, sTempRenderCenterY;

  sTempRenderCenterX = sTempRenderCenterY = 0;

  // This checking sequence just validates the values!
  if (ScrollFlags & SCROLL_LEFT) {
    FromScreenToCellCoordinates((int16_t)-sScrollXStep, 0, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;

    fMovedPos = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);
    if (fMovedPos) {
      fAGoodMove = TRUE;
    }

    if (!fCheckOnly) {
      ScrollBackground(SCROLL_LEFT, sScrollXStep, sScrollYStep);
    }
  }

  if (ScrollFlags & SCROLL_RIGHT) {
    FromScreenToCellCoordinates(sScrollXStep, 0, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fMovedPos = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);
    if (fMovedPos) {
      fAGoodMove = TRUE;
    }

    if (!fCheckOnly) {
      ScrollBackground(SCROLL_RIGHT, sScrollXStep, sScrollYStep);
    }
  }

  if (ScrollFlags & SCROLL_UP) {
    FromScreenToCellCoordinates(0, (int16_t)-sScrollYStep, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fMovedPos = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);
    if (fMovedPos) {
      fAGoodMove = TRUE;
    }

    if (!fCheckOnly) {
      ScrollBackground(SCROLL_UP, sScrollXStep, sScrollYStep);
    }
  }

  if (ScrollFlags & SCROLL_DOWN) {
    FromScreenToCellCoordinates(0, sScrollYStep, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fMovedPos = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);
    if (fMovedPos) {
      fAGoodMove = TRUE;
    }

    if (!fCheckOnly) {
      ScrollBackground(SCROLL_DOWN, sScrollXStep, sScrollYStep);
    }
  }

  if (ScrollFlags & SCROLL_UPLEFT) {
    // Check up
    FromScreenToCellCoordinates(0, (int16_t)-sScrollYStep, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fUpOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    // Check left
    FromScreenToCellCoordinates((int16_t)-sScrollXStep, 0, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fLeftOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    if (fLeftOK && fUpOK) {
      FromScreenToCellCoordinates((int16_t)-sScrollXStep, (int16_t)-sScrollYStep, &sTempX_W,
                                  &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_UPLEFT, sScrollXStep, sScrollYStep);
      }

    } else if (fUpOK) {
      fAGoodMove = TRUE;

      FromScreenToCellCoordinates(0, (int16_t)-sScrollYStep, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_UP, sScrollXStep, sScrollYStep);
      }

    } else if (fLeftOK) {
      fAGoodMove = TRUE;

      FromScreenToCellCoordinates((int16_t)-sScrollXStep, 0, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_LEFT, sScrollXStep, sScrollYStep);
      }
    }
  }

  if (ScrollFlags & SCROLL_UPRIGHT) {
    // Check up
    FromScreenToCellCoordinates(0, (int16_t)-sScrollYStep, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fUpOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    // Check right
    FromScreenToCellCoordinates(sScrollXStep, 0, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fRightOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    if (fUpOK && fRightOK) {
      FromScreenToCellCoordinates((int16_t)sScrollXStep, (int16_t)-sScrollYStep, &sTempX_W,
                                  &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_UPRIGHT, sScrollXStep, sScrollYStep);
      }

    } else if (fUpOK) {
      fAGoodMove = TRUE;

      FromScreenToCellCoordinates(0, (int16_t)-sScrollYStep, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_UP, sScrollXStep, sScrollYStep);
      }
    } else if (fRightOK) {
      fAGoodMove = TRUE;

      FromScreenToCellCoordinates(sScrollXStep, 0, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_RIGHT, sScrollXStep, sScrollYStep);
      }
    }
  }

  if (ScrollFlags & SCROLL_DOWNLEFT) {
    // Check down......
    FromScreenToCellCoordinates(0, sScrollYStep, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fDownOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    // Check left.....
    FromScreenToCellCoordinates((int16_t)-sScrollXStep, 0, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fLeftOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    if (fLeftOK && fDownOK) {
      fAGoodMove = TRUE;
      FromScreenToCellCoordinates((int16_t)-sScrollXStep, (int16_t)sScrollYStep, &sTempX_W,
                                  &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_DOWNLEFT, sScrollXStep, sScrollYStep);
      }

    } else if (fLeftOK) {
      FromScreenToCellCoordinates((int16_t)-sScrollXStep, 0, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_LEFT, sScrollXStep, sScrollYStep);
      }
    } else if (fDownOK) {
      FromScreenToCellCoordinates(0, sScrollYStep, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_DOWN, sScrollXStep, sScrollYStep);
      }
    }
  }

  if (ScrollFlags & SCROLL_DOWNRIGHT) {
    // Check right
    FromScreenToCellCoordinates(sScrollXStep, 0, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fRightOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    // Check down
    FromScreenToCellCoordinates(0, sScrollYStep, &sTempX_W, &sTempY_W);
    sTempRenderCenterX = gsRenderCenterX + sTempX_W;
    sTempRenderCenterY = gsRenderCenterY + sTempY_W;
    fDownOK = ApplyScrolling(sTempRenderCenterX, sTempRenderCenterY, FALSE, fCheckOnly);

    if (fDownOK && fRightOK) {
      FromScreenToCellCoordinates((int16_t)sScrollXStep, (int16_t)sScrollYStep, &sTempX_W,
                                  &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_DOWNRIGHT, sScrollXStep, sScrollYStep);
      }

    } else if (fDownOK) {
      FromScreenToCellCoordinates(0, sScrollYStep, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_DOWN, sScrollXStep, sScrollYStep);
      }
    } else if (fRightOK) {
      FromScreenToCellCoordinates(sScrollXStep, 0, &sTempX_W, &sTempY_W);
      sTempRenderCenterX = gsRenderCenterX + sTempX_W;
      sTempRenderCenterY = gsRenderCenterY + sTempY_W;
      fAGoodMove = TRUE;

      if (!fCheckOnly) {
        ScrollBackground(SCROLL_RIGHT, sScrollXStep, sScrollYStep);
      }
    }
  }

  (*psTempRenderCenterX) = sTempRenderCenterX;
  (*psTempRenderCenterY) = sTempRenderCenterY;

  return (fAGoodMove);
}

void ScrollWorld() {
  uint32_t ScrollFlags = 0;
  BOOLEAN fDoScroll = FALSE, fMovedPos = FALSE, fAGoodMove = FALSE;
  int16_t sTempRenderCenterX, sTempRenderCenterY;
  int8_t bDirection;
  int16_t sScrollXStep = -1;
  int16_t sScrollYStep = -1;
  BOOLEAN fIgnoreInput = FALSE;
  static uint8_t ubOldScrollSpeed = 0;
  static BOOLEAN fFirstTimeInSlideToMode = TRUE;

  if (gfIgnoreScrollDueToCenterAdjust) {
    //	gfIgnoreScrollDueToCenterAdjust = FALSE;
    return;
  }

  if (gfIgnoreScrolling == 1) {
    return;
  }

  if (gfIgnoreScrolling == 2) {
    fIgnoreInput = TRUE;
  }

  if (gCurrentUIMode == LOCKUI_MODE) {
    fIgnoreInput = TRUE;
  }

  // If in editor, ignore scrolling if any of the shift keys pressed with arrow keys
  if (gfEditMode && (_KeyDown(CTRL) || _KeyDown(ALT))) return;

  // Ignore if ALT DONW
  if (_KeyDown(ALT)) return;

  do {
    if (gfIgnoreScrolling != 3) {
      // Check for sliding
      if (gTacticalStatus.sSlideTarget != NOWHERE) {
        // Ignore all input...
        // Check if we have reached out dest!
        if (fFirstTimeInSlideToMode) {
          ubOldScrollSpeed = gubCurScrollSpeedID;
          fFirstTimeInSlideToMode = FALSE;
        }

        // Make faster!
        // gubCurScrollSpeedID = 2;

        ScrollFlags = 0;
        fDoScroll = FALSE;
        //
        if (SoldierLocationRelativeToScreen(gTacticalStatus.sSlideTarget,
                                            gTacticalStatus.sSlideReason, &bDirection,
                                            &ScrollFlags) &&
            GridNoOnVisibleWorldTile(gTacticalStatus.sSlideTarget)) {
          ScrollFlags = gScrollDirectionFlags[bDirection];
          fDoScroll = TRUE;
          fIgnoreInput = TRUE;
        } else {
          // We've stopped!
          gTacticalStatus.sSlideTarget = NOWHERE;
        }
      } else {
        // Restore old scroll speed
        if (!fFirstTimeInSlideToMode) {
          gubCurScrollSpeedID = ubOldScrollSpeed;
        }
        fFirstTimeInSlideToMode = TRUE;
      }
    }

    if (!fIgnoreInput) {
      // Check keys
      if (_KeyDown(UPARROW)) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_UP;
      }

      if (_KeyDown(DNARROW)) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_DOWN;
      }

      if (_KeyDown(RIGHTARROW)) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_RIGHT;
      }

      if (_KeyDown(LEFTARROW)) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_LEFT;
      }

      // Do mouse - PUT INTO A TIMER!
      // Put a counter on starting from mouse, if we have not started already!
      if (!gfScrollInertia && gfScrollPending == FALSE) {
        if (!COUNTERDONE(STARTSCROLL)) {
          break;
        }
        RESETCOUNTER(STARTSCROLL);
      }

      if (gusMouseYPos == 0) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_UP;
      }

      if (gusMouseYPos >= 479) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_DOWN;
      }

      if (gusMouseXPos >= 639) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_RIGHT;
      }

      if (gusMouseXPos == 0) {
        fDoScroll = TRUE;
        ScrollFlags |= SCROLL_LEFT;
      }
    }

  } while (FALSE);

  if (fDoScroll) {
    if (gfDoSubtileScroll) {
      if (gfScrollInertia > gubNewScrollIDSpeeds[gubCurScrollSpeedID]) {
        gubCurScrollSpeedID++;

        if (gubCurScrollSpeedID > gubScrollSpeedEndID) {
          gubCurScrollSpeedID = gubScrollSpeedEndID;
        }
      }
    }

    // if ( !gfDoVideoScroll )
    //{
    //	gubCurScrollSpeedID = 4;
    //}

    // Adjust speed based on whether shift is down
    if (_KeyDown(SHIFT)) {
      sScrollXStep = gubNewScrollXSpeeds[gfDoVideoScroll][3];
      sScrollYStep = gubNewScrollYSpeeds[gfDoVideoScroll][3];
    } else {
      sScrollXStep = gubNewScrollXSpeeds[gfDoVideoScroll][gubCurScrollSpeedID];
      sScrollYStep = gubNewScrollYSpeeds[gfDoVideoScroll][gubCurScrollSpeedID];
    }

    // Set diagonal flags!
    if ((ScrollFlags & SCROLL_LEFT) && (ScrollFlags & SCROLL_UP)) {
      ScrollFlags = SCROLL_UPLEFT;
    }
    if ((ScrollFlags & SCROLL_RIGHT) && (ScrollFlags & SCROLL_UP)) {
      ScrollFlags = SCROLL_UPRIGHT;
    }
    if ((ScrollFlags & SCROLL_LEFT) && (ScrollFlags & SCROLL_DOWN)) {
      ScrollFlags = SCROLL_DOWNLEFT;
    }
    if ((ScrollFlags & SCROLL_RIGHT) && (ScrollFlags & SCROLL_DOWN)) {
      ScrollFlags = SCROLL_DOWNRIGHT;
    }

    fAGoodMove = HandleScrollDirections(ScrollFlags, sScrollXStep, sScrollYStep,
                                        &sTempRenderCenterX, &sTempRenderCenterY, TRUE);
  }

  // Has this been an OK scroll?
  if (fAGoodMove) {
    if (COUNTERDONE(NEXTSCROLL)) {
      RESETCOUNTER(NEXTSCROLL);

      // Are we starting a new scroll?
      if (gfScrollInertia == 0 && gfScrollPending == FALSE) {
        // We are starting to scroll - setup scroll pending
        gfScrollPending = TRUE;

        // Remove any interface stuff
        ClearInterface();

        // Return so that next frame things will be erased!
        return;
      }

      // If here, set scroll pending to false
      gfScrollPending = FALSE;

      // INcrement scroll intertia
      gfScrollInertia++;

      // Now we actually begin our scrolling
      HandleScrollDirections(ScrollFlags, sScrollXStep, sScrollYStep, &sTempRenderCenterX,
                             &sTempRenderCenterY, FALSE);
    }
  } else {
    // ATE: Also if scroll pending never got to scroll....
    if (gfScrollPending == TRUE) {
      // Do a complete rebuild!
      gfScrollPending = FALSE;

      // Restore Interface!
      RestoreInterface();

      // Delete Topmost blitters saved areas
      DeleteVideoOverlaysArea();
    }

    // Check if we have just stopped scrolling!
    if (gfScrollInertia != FALSE) {
      SetRenderFlags(RENDER_FLAG_FULL | RENDER_FLAG_CHECKZ);

      // Restore Interface!
      RestoreInterface();

      // Delete Topmost blitters saved areas
      DeleteVideoOverlaysArea();
    }

    gfScrollInertia = FALSE;
    gfScrollPending = FALSE;

    if (gfDoSubtileScroll) {
      gubCurScrollSpeedID = gubScrollSpeedStartID;
    }
  }
}

void InitRenderParams(uint8_t ubRestrictionID) {
  int16_t gsTilesX, gsTilesY;
  uint32_t cnt, cnt2;
  double dWorldX, dWorldY;

  switch (ubRestrictionID) {
    case 0:  // Default!

      gTopLeftWorldLimitX = CELL_X_SIZE;
      gTopLeftWorldLimitY = (WORLD_ROWS / 2) * CELL_X_SIZE;

      gTopRightWorldLimitX = (WORLD_COLS / 2) * CELL_Y_SIZE;
      gTopRightWorldLimitY = CELL_X_SIZE;

      gBottomLeftWorldLimitX = ((WORLD_COLS / 2) * CELL_Y_SIZE);
      gBottomLeftWorldLimitY = (WORLD_ROWS * CELL_Y_SIZE);

      gBottomRightWorldLimitX = (WORLD_COLS * CELL_Y_SIZE);
      gBottomRightWorldLimitY = ((WORLD_ROWS / 2) * CELL_X_SIZE);
      break;

    case 1:  // BAEMENT LEVEL 1

      gTopLeftWorldLimitX = (3 * WORLD_ROWS / 10) * CELL_X_SIZE;
      gTopLeftWorldLimitY = (WORLD_ROWS / 2) * CELL_X_SIZE;

      gTopRightWorldLimitX = (WORLD_ROWS / 2) * CELL_X_SIZE;
      gTopRightWorldLimitY = (3 * WORLD_COLS / 10) * CELL_X_SIZE;

      gBottomLeftWorldLimitX = (WORLD_ROWS / 2) * CELL_X_SIZE;
      gBottomLeftWorldLimitY = (7 * WORLD_COLS / 10) * CELL_X_SIZE;

      gBottomRightWorldLimitX = (7 * WORLD_ROWS / 10) * CELL_X_SIZE;
      gBottomRightWorldLimitY = (WORLD_ROWS / 2) * CELL_X_SIZE;
      break;
  }

  gCenterWorldX = (WORLD_ROWS) / 2 * CELL_X_SIZE;
  gCenterWorldY = (WORLD_COLS) / 2 * CELL_Y_SIZE;

  // Convert Bounding box into screen coords
  FromCellToScreenCoordinates(gTopLeftWorldLimitX, gTopLeftWorldLimitY, &gsTLX, &gsTLY);
  FromCellToScreenCoordinates(gTopRightWorldLimitX, gTopRightWorldLimitY, &gsTRX, &gsTRY);
  FromCellToScreenCoordinates(gBottomLeftWorldLimitX, gBottomLeftWorldLimitY, &gsBLX, &gsBLY);
  FromCellToScreenCoordinates(gBottomRightWorldLimitX, gBottomRightWorldLimitY, &gsBRX, &gsBRY);
  FromCellToScreenCoordinates(gCenterWorldX, gCenterWorldY, &gsCX, &gsCY);

  // Adjust for interface height tabbing!
  gsTLY += ROOF_LEVEL_HEIGHT;
  gsTRY += ROOF_LEVEL_HEIGHT;
  gsCY += (ROOF_LEVEL_HEIGHT / 2);

  // Take these spaning distances and determine # tiles spaning
  gsTilesX = (gsTRX - gsTLX) / WORLD_TILE_X;
  gsTilesY = (gsBRY - gsTRY) / WORLD_TILE_Y;

  DebugMsg(TOPIC_JA2, DBG_LEVEL_0,
           String("World Screen Width %d Height %d", (gsTRX - gsTLX), (gsBRY - gsTRY)));

  // Determine scale factors
  // First scale world screen coords for VIEWPORT ratio
  dWorldX = (double)(gsTRX - gsTLX);
  dWorldY = (double)(gsBRY - gsTRY);

  gdScaleX = (double)RADAR_WINDOW_WIDTH / dWorldX;
  gdScaleY = (double)RADAR_WINDOW_HEIGHT / dWorldY;

  for (cnt = 0, cnt2 = 0; cnt2 < NUM_ITEM_CYCLE_COLORS; cnt += 3, cnt2++) {
    us16BPPItemCycleWhiteColors[cnt2] =
        rgb32_to_rgb16(FROMRGB(ubRGBItemCycleWhiteColors[cnt], ubRGBItemCycleWhiteColors[cnt + 1],
                               ubRGBItemCycleWhiteColors[cnt + 2]));
    us16BPPItemCycleRedColors[cnt2] =
        rgb32_to_rgb16(FROMRGB(ubRGBItemCycleRedColors[cnt], ubRGBItemCycleRedColors[cnt + 1],
                               ubRGBItemCycleRedColors[cnt + 2]));
    us16BPPItemCycleYellowColors[cnt2] =
        rgb32_to_rgb16(FROMRGB(ubRGBItemCycleYellowColors[cnt], ubRGBItemCycleYellowColors[cnt + 1],
                               ubRGBItemCycleYellowColors[cnt + 2]));
  }

  gsLobOutline = rgb32_to_rgb16(FROMRGB(10, 200, 10));
  gsThrowOutline = rgb32_to_rgb16(FROMRGB(253, 212, 10));
  gsGiveOutline = rgb32_to_rgb16(FROMRGB(253, 0, 0));

  gusNormalItemOutlineColor = rgb32_to_rgb16(FROMRGB(255, 255, 255));
  gusYellowItemOutlineColor = rgb32_to_rgb16(FROMRGB(255, 255, 0));

  // NOW GET DISTANCE SPANNING WORLD LIMITS IN WORLD COORDS
  // FromScreenToCellCoordinates( ( gTopRightWorldLimitX - gTopLeftWorldLimitX ), (
  // gTopRightWorldLimitY - gTopLeftWorldLimitY ), &gsWorldSpanX, &gsWorldSpanY );

  // CALCULATE 16BPP COLORS FOR ITEMS
}

// Appy? HEahehahehahehae.....
BOOLEAN ApplyScrolling(int16_t sTempRenderCenterX, int16_t sTempRenderCenterY, BOOLEAN fForceAdjust,
                       BOOLEAN fCheckOnly) {
  BOOLEAN fScrollGood = FALSE;
  BOOLEAN fOutLeft = FALSE;
  BOOLEAN fOutRight = FALSE;
  BOOLEAN fOutTop = FALSE;
  BOOLEAN fOutBottom = FALSE;

  double dOpp, dAdj, dAngle;

  int16_t sTopLeftWorldX, sTopLeftWorldY;
  int16_t sTopRightWorldX, sTopRightWorldY;
  int16_t sBottomLeftWorldX, sBottomLeftWorldY;
  int16_t sBottomRightWorldX, sBottomRightWorldY;

  int16_t sTempPosX_W, sTempPosY_W;

  // For debug text for all 4 angles
  double at1, at2, at3, at4;

  int16_t sX_S, sY_S;
  int16_t sScreenCenterX, sScreenCenterY;
  int16_t sDistToCenterY, sDistToCenterX;
  int16_t sNewScreenX, sNewScreenY;
  int16_t sMult;

  // Makesure it's a multiple of 5
  sMult = sTempRenderCenterX / CELL_X_SIZE;
  sTempRenderCenterX = (sMult * CELL_X_SIZE) + (CELL_X_SIZE / 2);

  // Makesure it's a multiple of 5
  sMult = sTempRenderCenterY / CELL_X_SIZE;
  sTempRenderCenterY = (sMult * CELL_Y_SIZE) + (CELL_Y_SIZE / 2);

  // Find the diustance from render center to true world center
  sDistToCenterX = sTempRenderCenterX - gCenterWorldX;
  sDistToCenterY = sTempRenderCenterY - gCenterWorldY;

  // From render center in world coords, convert to render center in "screen" coords
  FromCellToScreenCoordinates(sDistToCenterX, sDistToCenterY, &sScreenCenterX, &sScreenCenterY);

  // Subtract screen center
  sScreenCenterX += gsCX;
  sScreenCenterY += gsCY;

  // Adjust for offset position on screen
  sScreenCenterX -= 0;
  sScreenCenterY -= 10;

  // Get corners in screen coords
  // TOP LEFT
  sX_S = (gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2;
  sY_S = (gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2;

  sTopLeftWorldX = sScreenCenterX - sX_S;
  sTopLeftWorldY = sScreenCenterY - sY_S;

  sTopRightWorldX = sScreenCenterX + sX_S;
  sTopRightWorldY = sScreenCenterY - sY_S;

  sBottomLeftWorldX = sScreenCenterX - sX_S;
  sBottomLeftWorldY = sScreenCenterY + sY_S;

  sBottomRightWorldX = sScreenCenterX + sX_S;
  sBottomRightWorldY = sScreenCenterY + sY_S;

  // Get angles
  // TOP LEFT CORNER FIRST
  dOpp = sTopLeftWorldY - gsTLY;
  dAdj = sTopLeftWorldX - gsTLX;

  dAngle = (double)atan2(dAdj, dOpp);
  at1 = dAngle * 180 / PI;

  if (dAngle < 0) {
    fOutLeft = TRUE;
  } else if (dAngle > PI / 2) {
    fOutTop = TRUE;
  }

  // TOP RIGHT CORNER
  dOpp = sTopRightWorldY - gsTRY;
  dAdj = gsTRX - sTopRightWorldX;

  dAngle = (double)atan2(dAdj, dOpp);
  at2 = dAngle * 180 / PI;

  if (dAngle < 0) {
    fOutRight = TRUE;
  } else if (dAngle > PI / 2) {
    fOutTop = TRUE;
  }

  // BOTTOM LEFT CORNER
  dOpp = gsBLY - sBottomLeftWorldY;
  dAdj = sBottomLeftWorldX - gsBLX;

  dAngle = (double)atan2(dAdj, dOpp);
  at3 = dAngle * 180 / PI;

  if (dAngle < 0) {
    fOutLeft = TRUE;
  } else if (dAngle > PI / 2) {
    fOutBottom = TRUE;
  }

  // BOTTOM RIGHT CORNER
  dOpp = gsBRY - sBottomRightWorldY;
  dAdj = gsBRX - sBottomRightWorldX;

  dAngle = (double)atan2(dAdj, dOpp);
  at4 = dAngle * 180 / PI;

  if (dAngle < 0) {
    fOutRight = TRUE;
  } else if (dAngle > PI / 2) {
    fOutBottom = TRUE;
  }

  sprintf(gDebugStr, "Angles: %d %d %d %d", (int)at1, (int)at2, (int)at3, (int)at4);

  if (!fOutRight && !fOutLeft && !fOutTop && !fOutBottom) {
    fScrollGood = TRUE;
  }

  // If in editor, anything goes
  if (gfEditMode && _KeyDown(SHIFT)) {
    fScrollGood = TRUE;
  }

  // Reset some UI flags
  gfUIShowExitEast = FALSE;
  gfUIShowExitWest = FALSE;
  gfUIShowExitNorth = FALSE;
  gfUIShowExitSouth = FALSE;

  if (!fScrollGood) {
    // Force adjustment, if true
    if (fForceAdjust) {
      if (fOutTop) {
        // Adjust screen coordinates on the Y!
        CorrectRenderCenter(sScreenCenterX, (int16_t)(gsTLY + sY_S), &sNewScreenX, &sNewScreenY);
        FromScreenToCellCoordinates(sNewScreenX, sNewScreenY, &sTempPosX_W, &sTempPosY_W);

        sTempRenderCenterX = sTempPosX_W;
        sTempRenderCenterY = sTempPosY_W;
        fScrollGood = TRUE;
      }

      if (fOutBottom) {
        // OK, Ajust this since we get rounding errors in our two different calculations.
        CorrectRenderCenter(sScreenCenterX, (int16_t)(gsBLY - sY_S - 50), &sNewScreenX,
                            &sNewScreenY);
        FromScreenToCellCoordinates(sNewScreenX, sNewScreenY, &sTempPosX_W, &sTempPosY_W);

        sTempRenderCenterX = sTempPosX_W;
        sTempRenderCenterY = sTempPosY_W;
        fScrollGood = TRUE;
      }

      if (fOutLeft) {
        CorrectRenderCenter((int16_t)(gsTLX + sX_S), sScreenCenterY, &sNewScreenX, &sNewScreenY);
        FromScreenToCellCoordinates(sNewScreenX, sNewScreenY, &sTempPosX_W, &sTempPosY_W);

        sTempRenderCenterX = sTempPosX_W;
        sTempRenderCenterY = sTempPosY_W;
        fScrollGood = TRUE;
      }

      if (fOutRight) {
        CorrectRenderCenter((int16_t)(gsTRX - sX_S), sScreenCenterY, &sNewScreenX, &sNewScreenY);
        FromScreenToCellCoordinates(sNewScreenX, sNewScreenY, &sTempPosX_W, &sTempPosY_W);

        sTempRenderCenterX = sTempPosX_W;
        sTempRenderCenterY = sTempPosY_W;
        fScrollGood = TRUE;
      }

    } else {
      if (fOutRight) {
        // Check where our cursor is!
        if (gusMouseXPos >= 639) {
          gfUIShowExitEast = TRUE;
        }
      }

      if (fOutLeft) {
        // Check where our cursor is!
        if (gusMouseXPos == 0) {
          gfUIShowExitWest = TRUE;
        }
      }

      if (fOutTop) {
        // Check where our cursor is!
        if (gusMouseYPos == 0) {
          gfUIShowExitNorth = TRUE;
        }
      }

      if (fOutBottom) {
        // Check where our cursor is!
        if (gusMouseYPos >= 479) {
          gfUIShowExitSouth = TRUE;
        }
      }
    }
  }

  if (fScrollGood) {
    if (!fCheckOnly) {
      sprintf(gDebugStr, "Center: %d %d ", (int)gsRenderCenterX, (int)gsRenderCenterY);

      // Makesure it's a multiple of 5
      sMult = sTempRenderCenterX / CELL_X_SIZE;
      gsRenderCenterX = (sMult * CELL_X_SIZE) + (CELL_X_SIZE / 2);

      // Makesure it's a multiple of 5
      sMult = sTempRenderCenterY / CELL_X_SIZE;
      gsRenderCenterY = (sMult * CELL_Y_SIZE) + (CELL_Y_SIZE / 2);

      // gsRenderCenterX = sTempRenderCenterX;
      // gsRenderCenterY = sTempRenderCenterY;

      gsTopLeftWorldX = sTopLeftWorldX - gsTLX;
      gsTopLeftWorldY = sTopLeftWorldY - gsTLY;

      gsTopRightWorldX = sTopRightWorldX - gsTLX;
      gsTopRightWorldY = sTopRightWorldY - gsTLY;

      gsBottomLeftWorldX = sBottomLeftWorldX - gsTLX;
      gsBottomLeftWorldY = sBottomLeftWorldY - gsTLY;

      gsBottomRightWorldX = sBottomRightWorldX - gsTLX;
      gsBottomRightWorldY = sBottomRightWorldY - gsTLY;

      SetPositionSndsVolumeAndPanning();
    }

    return (TRUE);
  }

  return (FALSE);
}

void ClearMarkedTiles(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < WORLD_MAX; uiCount++)
    gpWorldLevelData[uiCount].uiFlags &= (~MAPELEMENT_REDRAW);
}

// @@ATECLIP TO WORLD!
void InvalidateWorldRedundencyRadius(int16_t sX, int16_t sY, int16_t sRadius) {
  int16_t sCountX, sCountY;
  uint32_t uiTile;

  SetRenderFlags(RENDER_FLAG_CHECKZ);

  for (sCountY = sY - sRadius; sCountY < (sY + sRadius + 2); sCountY++) {
    for (sCountX = sX - sRadius; sCountX < (sX + sRadius + 2); sCountX++) {
      uiTile = FASTMAPROWCOLTOPOS(sCountY, sCountX);

      gpWorldLevelData[uiTile].uiFlags |= MAPELEMENT_REEVALUATE_REDUNDENCY;
    }
  }
}

void InvalidateWorldRedundency() {
  uint32_t uiCount;

  SetRenderFlags(RENDER_FLAG_CHECKZ);

  for (uiCount = 0; uiCount < WORLD_MAX; uiCount++)
    gpWorldLevelData[uiCount].uiFlags |= MAPELEMENT_REEVALUATE_REDUNDENCY;
}

#define Z_STRIP_DELTA_Y (Z_SUBLAYERS * 10)

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZIncClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                              uint16_t *pZBuffer, uint16_t usZValue,
                                              struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                              uint16_t usIndex, SGPRect *clipregion) {
  uint16_t *p16BPPPalette;
  uint32_t uiOffset;
  uint32_t usHeight, usWidth, Unblitted;
  uint8_t *SrcPtr, *DestPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  int32_t ClipX1, ClipY1, ClipX2, ClipY2;
  uint16_t usZLevel, usZStartLevel, usZColsToGo, usZStartIndex, usCount, usZIndex, usZStartCols;
  int8_t *pZArray;
  ZStripInfo *pZInfo;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (int32_t)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (int32_t)usWidth)) - ClipX2, (int32_t)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (int32_t)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (int32_t)usHeight)) - ClipY2, (int32_t)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((int32_t)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((int32_t)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (int32_t)usWidth) || (RightSkip >= (int32_t)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (int32_t)usHeight) || (BottomSkip >= (int32_t)usHeight)) return (TRUE);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr =
      (uint8_t *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (uint8_t *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

  if (hSrcVObject->ppZStripInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }
  // setup for the z-column blitting stuff
  pZInfo = hSrcVObject->ppZStripInfo[usIndex];
  if (pZInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }

  usZStartLevel =
      (uint16_t)((int16_t)usZValue + ((int16_t)pZInfo->bInitialZChange * Z_STRIP_DELTA_Y));
  // set to odd number of pixels for first column

  if (LeftSkip > pZInfo->ubFirstZStripWidth) {
    usZStartCols = (LeftSkip - pZInfo->ubFirstZStripWidth);
    usZStartCols = 20 - (usZStartCols % 20);
  } else if (LeftSkip < pZInfo->ubFirstZStripWidth)
    usZStartCols = (uint16_t)(pZInfo->ubFirstZStripWidth - LeftSkip);
  else
    usZStartCols = 20;

  usZColsToGo = usZStartCols;

  pZArray = pZInfo->pbZChange;

  if (LeftSkip >= pZInfo->ubFirstZStripWidth) {
    // Index into array after doing left clipping
    usZStartIndex = 1 + ((LeftSkip - pZInfo->ubFirstZStripWidth) / 20);

    // calculates the Z-value after left-side clipping
    if (usZStartIndex) {
      for (usCount = 0; usCount < usZStartIndex; usCount++) {
        switch (pZArray[usCount]) {
          case -1:
            usZStartLevel -= Z_STRIP_DELTA_Y;
            break;
          case 0:  // no change
            break;
          case 1:
            usZStartLevel += Z_STRIP_DELTA_Y;
            break;
        }
      }
    }
  } else
    usZStartIndex = 0;

  usZLevel = usZStartLevel;
  usZIndex = usZStartIndex;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

        // Skips the number of lines clipped at the top
TopSkipLoop:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

        // Start of line loop

        // Skips the pixels hanging outside the left-side boundry
LeftSkipSetup:

		mov		Unblitted, 0  // Unblitted counts any pixels left from a run
		mov		eax, LeftSkip  // after we have skipped enough left-side pixels
		mov		LSCount, eax  // LSCount counts how many pixels skipped so far
		or		eax, eax
		jz		BlitLineSetup  // check for nothing to skip

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNTL1  // *** jumps into non-transparent blit loop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent  // *** jumps into transparent blit loop


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop

            //-------------------------------------------------
            // setup for beginning of line

BlitLineSetup:
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		RSLoop2

        //--------------------------------
        // blitting non-transparent pixels

		and		ecx, 07fH

BlitNTL1:
		mov		ax, [ebx]  // check z-level of pixel
		cmp		ax, usZLevel
		jae		BlitNTL2

		mov		ax, usZLevel  // update z-level of pixel
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]  // copy pixel
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitNTL6

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitNTL5  // dir = 0 no change
		js		BlitNTL4               // dir < 0 z-level down
                    // dir > 0 z-level up (default)
		add		dx, Z_STRIP_DELTA_Y
		jmp		BlitNTL5

BlitNTL4:
		sub		dx, Z_STRIP_DELTA_Y

BlitNTL5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitNTL6:
		dec		LSCount  // decrement pixel length to blit
		jz		RightSkipLoop  // done blitting the visible line

		dec		ecx
		jnz		BlitNTL1  // continue current run

		jmp		BlitDispatch  // done current run, go for another

                    //----------------------------
                    // skipping transparent pixels

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH

BlitTrans2:

		add		edi, 2  // move up the destination pointer
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitTrans1

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitTrans5  // dir = 0 no change
		js		BlitTrans4         // dir < 0 z-level down
                      // dir > 0 z-level up (default)
		add		dx, Z_STRIP_DELTA_Y
		jmp		BlitTrans5

BlitTrans4:
		sub		dx, Z_STRIP_DELTA_Y

BlitTrans5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitTrans1:

		dec		LSCount  // decrement the pixels to blit
		jz		RightSkipLoop  // done the line

		dec		ecx
		jnz		BlitTrans2

		jmp		BlitDispatch

                        //---------------------------------------------
                        // Scans the ETRLE until it finds an EOL marker

RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

RSLoop2:

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

        // reset all the z-level stuff for a new line

		mov		ax, usZStartLevel
		mov		usZLevel, ax
		mov		ax, usZStartIndex
		mov		usZIndex, ax
		mov		ax, usZStartCols
		mov		usZColsToGo, ax


		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZIncClipSaveZBurnsThrough

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncClipZSameZBurnsThrough(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion) {
  uint16_t *p16BPPPalette;
  uint32_t uiOffset;
  uint32_t usHeight, usWidth, Unblitted;
  uint8_t *SrcPtr, *DestPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  int32_t ClipX1, ClipY1, ClipX2, ClipY2;
  uint16_t usZLevel, usZStartLevel, usZColsToGo, usZStartIndex, usCount, usZIndex, usZStartCols;
  int8_t *pZArray;
  ZStripInfo *pZInfo;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (int32_t)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (int32_t)usWidth)) - ClipX2, (int32_t)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (int32_t)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (int32_t)usHeight)) - ClipY2, (int32_t)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((int32_t)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((int32_t)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (int32_t)usWidth) || (RightSkip >= (int32_t)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (int32_t)usHeight) || (BottomSkip >= (int32_t)usHeight)) return (TRUE);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr =
      (uint8_t *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (uint8_t *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

  if (hSrcVObject->ppZStripInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }
  // setup for the z-column blitting stuff
  pZInfo = hSrcVObject->ppZStripInfo[usIndex];
  if (pZInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }

  usZStartLevel =
      (uint16_t)((int16_t)usZValue + ((int16_t)pZInfo->bInitialZChange * Z_STRIP_DELTA_Y));
  // set to odd number of pixels for first column

  if (LeftSkip > pZInfo->ubFirstZStripWidth) {
    usZStartCols = (LeftSkip - pZInfo->ubFirstZStripWidth);
    usZStartCols = 20 - (usZStartCols % 20);
  } else if (LeftSkip < pZInfo->ubFirstZStripWidth)
    usZStartCols = (uint16_t)(pZInfo->ubFirstZStripWidth - LeftSkip);
  else
    usZStartCols = 20;

  usZColsToGo = usZStartCols;

  pZArray = pZInfo->pbZChange;

  if (LeftSkip >= pZInfo->ubFirstZStripWidth) {
    // Index into array after doing left clipping
    usZStartIndex = 1 + ((LeftSkip - pZInfo->ubFirstZStripWidth) / 20);

    // calculates the Z-value after left-side clipping
    if (usZStartIndex) {
      for (usCount = 0; usCount < usZStartIndex; usCount++) {
        switch (pZArray[usCount]) {
          case -1:
            usZStartLevel -= Z_STRIP_DELTA_Y;
            break;
          case 0:  // no change
            break;
          case 1:
            usZStartLevel += Z_STRIP_DELTA_Y;
            break;
        }
      }
    }
  } else
    usZStartIndex = 0;

  usZLevel = usZStartLevel;
  usZIndex = usZStartIndex;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

        // Skips the number of lines clipped at the top
TopSkipLoop:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

        // Start of line loop

        // Skips the pixels hanging outside the left-side boundry
LeftSkipSetup:

		mov		Unblitted, 0  // Unblitted counts any pixels left from a run
		mov		eax, LeftSkip  // after we have skipped enough left-side pixels
		mov		LSCount, eax  // LSCount counts how many pixels skipped so far
		or		eax, eax
		jz		BlitLineSetup  // check for nothing to skip

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNTL1  // *** jumps into non-transparent blit loop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent  // *** jumps into transparent blit loop


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop

            //-------------------------------------------------
            // setup for beginning of line

BlitLineSetup:
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		RSLoop2

        //--------------------------------
        // blitting non-transparent pixels

		and		ecx, 07fH

BlitNTL1:
		mov		ax, [ebx]  // check z-level of pixel
		cmp		ax, usZLevel
		ja		BlitNTL2

		mov		ax, usZLevel  // update z-level of pixel
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]  // copy pixel
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitNTL6

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitNTL5  // dir = 0 no change
		js		BlitNTL4               // dir < 0 z-level down
                    // dir > 0 z-level up (default)
		add		dx, Z_STRIP_DELTA_Y
		jmp		BlitNTL5

BlitNTL4:
		sub		dx, Z_STRIP_DELTA_Y

BlitNTL5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitNTL6:
		dec		LSCount  // decrement pixel length to blit
		jz		RightSkipLoop  // done blitting the visible line

		dec		ecx
		jnz		BlitNTL1  // continue current run

		jmp		BlitDispatch  // done current run, go for another

                    //----------------------------
                    // skipping transparent pixels

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH

BlitTrans2:

		add		edi, 2  // move up the destination pointer
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitTrans1

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitTrans5  // dir = 0 no change
		js		BlitTrans4         // dir < 0 z-level down
                      // dir > 0 z-level up (default)
		add		dx, Z_STRIP_DELTA_Y
		jmp		BlitTrans5

BlitTrans4:
		sub		dx, Z_STRIP_DELTA_Y

BlitTrans5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitTrans1:

		dec		LSCount  // decrement the pixels to blit
		jz		RightSkipLoop  // done the line

		dec		ecx
		jnz		BlitTrans2

		jmp		BlitDispatch

                        //---------------------------------------------
                        // Scans the ETRLE until it finds an EOL marker

RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

RSLoop2:

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

        // reset all the z-level stuff for a new line

		mov		ax, usZStartLevel
		mov		usZLevel, ax
		mov		ax, usZStartIndex
		mov		usZIndex, ax
		mov		ax, usZStartCols
		mov		usZColsToGo, ax


		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

/**********************************************************************************************
 Blt8BPPDataTo16BPPBufferTransZIncObscureClip

        Blits an image into the destination buffer, using an ETRLE brush as a source, and a 16-bit
        buffer as a destination. As it is blitting, it checks the Z value of the ZBuffer, and if the
        pixel's Z level is below that of the current pixel, it is written on, and the Z value is
        updated to the current value,	for any non-transparent pixels. The Z-buffer is 16 bit, and
        must be the same dimensions (including Pitch) as the destination.

        //ATE: This blitter makes the values that are =< z value pixellate rather than not
        // render at all

**********************************************************************************************/
BOOLEAN Blt8BPPDataTo16BPPBufferTransZIncObscureClip(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                                     uint16_t *pZBuffer, uint16_t usZValue,
                                                     struct VObject *hSrcVObject, int32_t iX,
                                                     int32_t iY, uint16_t usIndex,
                                                     SGPRect *clipregion) {
  uint16_t *p16BPPPalette;
  uint32_t uiOffset, uiLineFlag;
  uint32_t usHeight, usWidth, Unblitted;
  uint8_t *SrcPtr, *DestPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  int32_t ClipX1, ClipY1, ClipX2, ClipY2;
  uint16_t usZLevel, usZStartLevel, usZColsToGo, usZStartIndex, usCount, usZIndex, usZStartCols;
  int8_t *pZArray;
  ZStripInfo *pZInfo;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (int32_t)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (int32_t)usWidth)) - ClipX2, (int32_t)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (int32_t)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (int32_t)usHeight)) - ClipY2, (int32_t)usHeight);

  uiLineFlag = (iTempY & 1);

  // calculate the remaining rows and columns to blit
  BlitLength = ((int32_t)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((int32_t)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (int32_t)usWidth) || (RightSkip >= (int32_t)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (int32_t)usHeight) || (BottomSkip >= (int32_t)usHeight)) return (TRUE);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr =
      (uint8_t *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (uint8_t *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

  if (hSrcVObject->ppZStripInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }
  // setup for the z-column blitting stuff
  pZInfo = hSrcVObject->ppZStripInfo[usIndex];
  if (pZInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }

  usZStartLevel =
      (uint16_t)((int16_t)usZValue + ((int16_t)pZInfo->bInitialZChange * Z_STRIP_DELTA_Y));
  // set to odd number of pixels for first column

  if (LeftSkip > pZInfo->ubFirstZStripWidth) {
    usZStartCols = (LeftSkip - pZInfo->ubFirstZStripWidth);
    usZStartCols = 20 - (usZStartCols % 20);
  } else if (LeftSkip < pZInfo->ubFirstZStripWidth)
    usZStartCols = (uint16_t)(pZInfo->ubFirstZStripWidth - LeftSkip);
  else
    usZStartCols = 20;

  usZColsToGo = usZStartCols;

  pZArray = pZInfo->pbZChange;

  if (LeftSkip >= pZInfo->ubFirstZStripWidth) {
    // Index into array after doing left clipping
    usZStartIndex = 1 + ((LeftSkip - pZInfo->ubFirstZStripWidth) / 20);

    // calculates the Z-value after left-side clipping
    if (usZStartIndex) {
      for (usCount = 0; usCount < usZStartIndex; usCount++) {
        switch (pZArray[usCount]) {
          case -1:
            usZStartLevel -= Z_STRIP_DELTA_Y;
            break;
          case 0:  // no change
            break;
          case 1:
            usZStartLevel += Z_STRIP_DELTA_Y;
            break;
        }
      }
    }
  } else
    usZStartIndex = 0;

  usZLevel = usZStartLevel;
  usZIndex = usZStartIndex;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

        // Skips the number of lines clipped at the top
TopSkipLoop:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:

		xor		uiLineFlag, 1
		dec		TopSkip
		jnz		TopSkipLoop

        // Start of line loop

        // Skips the pixels hanging outside the left-side boundry
LeftSkipSetup:

		mov		Unblitted, 0  // Unblitted counts any pixels left from a run
		mov		eax, LeftSkip  // after we have skipped enough left-side pixels
		mov		LSCount, eax  // LSCount counts how many pixels skipped so far
		or		eax, eax
		jz		BlitLineSetup  // check for nothing to skip

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNTL1  // *** jumps into non-transparent blit loop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitTransparent  // *** jumps into transparent blit loop


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop

            //-------------------------------------------------
            // setup for beginning of line

BlitLineSetup:
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		RSLoop2

        //--------------------------------
        // blitting non-transparent pixels

		and		ecx, 07fH

BlitNTL1:
		mov		ax, [ebx]  // check z-level of pixel
		cmp		ax, usZLevel
		jae		BlitPixellate1
		jmp   BlitPixel1

BlitPixellate1:

        // OK, DO PIXELLATE SCHEME HERE!
		test	uiLineFlag, 1
		jz		BlitSkip1

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitPixel1

BlitSkip1:
		test	edi, 2
		jnz		BlitNTL2

BlitPixel1:

		mov		ax, usZLevel  // update z-level of pixel
		mov		[ebx], ax

		xor		eax, eax
		mov		al, [esi]  // copy pixel
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitNTL6

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitNTL5  // dir = 0 no change
		js		BlitNTL4               // dir < 0 z-level down
                    // dir > 0 z-level up (default)
		add		dx, Z_STRIP_DELTA_Y
		jmp		BlitNTL5

BlitNTL4:
		sub		dx, Z_STRIP_DELTA_Y

BlitNTL5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitNTL6:
		dec		LSCount  // decrement pixel length to blit
		jz		RightSkipLoop  // done blitting the visible line

		dec		ecx
		jnz		BlitNTL1  // continue current run

		jmp		BlitDispatch  // done current run, go for another

                    //----------------------------
                    // skipping transparent pixels

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH

BlitTrans2:

		add		edi, 2  // move up the destination pointer
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitTrans1

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitTrans5  // dir = 0 no change
		js		BlitTrans4         // dir < 0 z-level down
                      // dir > 0 z-level up (default)
		add		dx, Z_STRIP_DELTA_Y
		jmp		BlitTrans5

BlitTrans4:
		sub		dx, Z_STRIP_DELTA_Y

BlitTrans5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitTrans1:

		dec		LSCount  // decrement the pixels to blit
		jz		RightSkipLoop  // done the line

		dec		ecx
		jnz		BlitTrans2

		jmp		BlitDispatch

                        //---------------------------------------------
                        // Scans the ETRLE until it finds an EOL marker

RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

RSLoop2:

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

        // reset all the z-level stuff for a new line

		mov		ax, usZStartLevel
		mov		usZLevel, ax
		mov		ax, usZStartIndex
		mov		usZIndex, ax
		mov		ax, usZStartCols
		mov		usZColsToGo, ax


		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

// Blitter Specs
// 1 ) 8 to 16 bpp
// 2 ) strip z-blitter
// 3 ) clipped
// 4 ) trans shadow - if value is 254, makes a shadow
//
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTransShadowIncObscureClip(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion,
    int16_t sZIndex, uint16_t *p16BPPPalette) {
  uint32_t uiOffset, uiLineFlag;
  uint32_t usHeight, usWidth, Unblitted;
  uint8_t *SrcPtr, *DestPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  int32_t ClipX1, ClipY1, ClipX2, ClipY2;
  uint16_t usZLevel, usZStartLevel, usZColsToGo, usZStartIndex, usCount, usZIndex, usZStartCols;
  int8_t *pZArray;
  ZStripInfo *pZInfo;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (int32_t)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (int32_t)usWidth)) - ClipX2, (int32_t)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (int32_t)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (int32_t)usHeight)) - ClipY2, (int32_t)usHeight);

  uiLineFlag = (iTempY & 1);

  // calculate the remaining rows and columns to blit
  BlitLength = ((int32_t)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((int32_t)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (int32_t)usWidth) || (RightSkip >= (int32_t)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (int32_t)usHeight) || (BottomSkip >= (int32_t)usHeight)) return (TRUE);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr =
      (uint8_t *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (uint8_t *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

  if (hSrcVObject->ppZStripInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }
  // setup for the z-column blitting stuff
  pZInfo = hSrcVObject->ppZStripInfo[sZIndex];
  if (pZInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }

  usZStartLevel =
      (uint16_t)((int16_t)usZValue + ((int16_t)pZInfo->bInitialZChange * Z_SUBLAYERS * 10));

  if (LeftSkip > pZInfo->ubFirstZStripWidth) {
    usZStartCols = (LeftSkip - pZInfo->ubFirstZStripWidth);
    usZStartCols = 20 - (usZStartCols % 20);
  } else if (LeftSkip < pZInfo->ubFirstZStripWidth)
    usZStartCols = (uint16_t)(pZInfo->ubFirstZStripWidth - LeftSkip);
  else
    usZStartCols = 20;

  // set to odd number of pixels for first column
  usZColsToGo = usZStartCols;

  pZArray = pZInfo->pbZChange;

  if (LeftSkip >= usZColsToGo) {
    // Index into array after doing left clipping
    usZStartIndex = 1 + ((LeftSkip - pZInfo->ubFirstZStripWidth) / 20);

    // calculates the Z-value after left-side clipping
    if (usZStartIndex) {
      for (usCount = 0; usCount < usZStartIndex; usCount++) {
        switch (pZArray[usCount]) {
          case -1:
            usZStartLevel -= Z_SUBLAYERS;
            break;
          case 0:  // no change
            break;
          case 1:
            usZStartLevel += Z_SUBLAYERS;
            break;
        }
      }
    }
  } else
    usZStartIndex = 0;

  usZLevel = usZStartLevel;
  usZIndex = usZStartIndex;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

        // Skips the number of lines clipped at the top
TopSkipLoop:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:

		xor		uiLineFlag, 1
		dec		TopSkip
		jnz		TopSkipLoop

        // Start of line loop

        // Skips the pixels hanging outside the left-side boundry
LeftSkipSetup:

		mov		Unblitted, 0  // Unblitted counts any pixels left from a run
		mov		eax, LeftSkip  // after we have skipped enough left-side pixels
		mov		LSCount, eax  // LSCount counts how many pixels skipped so far
		or		eax, eax
		jz		BlitLineSetup  // check for nothing to skip

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNTL1  // *** jumps into non-transparent blit loop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax

		mov		Unblitted, 0
		jmp		BlitTransparent  // *** jumps into transparent blit loop


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop

            //-------------------------------------------------
            // setup for beginning of line

BlitLineSetup:
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		RSLoop2

        //--------------------------------
        // blitting non-transparent pixels

		and		ecx, 07fH

BlitNTL1:
		mov		ax, [ebx]  // check z-level of pixel
		cmp		ax, usZLevel
		jae		BlitPixellate1
		jmp		BlitPixel1

BlitPixellate1:

        // OK, DO PIXELLATE SCHEME HERE!
		test	uiLineFlag, 1
		jz		BlitSkip1

		test	edi, 2
		jz		BlitNTL2
		jmp		BlitPixel1

BlitSkip1:
		test	edi, 2
		jnz		BlitNTL2

BlitPixel1:

		mov		ax, usZLevel  // update z-level of pixel
		mov		[ebx], ax

        // Check for shadow...
		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL66

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL66:

		mov		ax, [edx+eax*2]  // Copy pixel
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitNTL6

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitNTL5  // dir = 0 no change
		js		BlitNTL4               // dir < 0 z-level down
                    // dir > 0 z-level up (default)
		add		dx, Z_SUBLAYERS
		jmp		BlitNTL5

BlitNTL4:
		sub		dx, Z_SUBLAYERS

BlitNTL5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitNTL6:
		dec		LSCount  // decrement pixel length to blit
		jz		RightSkipLoop  // done blitting the visible line

		dec		ecx
		jnz		BlitNTL1  // continue current run

		jmp		BlitDispatch  // done current run, go for another

                    //----------------------------
                    // skipping transparent pixels

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH

BlitTrans2:

		add		edi, 2  // move up the destination pointer
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitTrans1

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitTrans5  // dir = 0 no change
		js		BlitTrans4         // dir < 0 z-level down
                      // dir > 0 z-level up (default)
		add		dx, Z_SUBLAYERS
		jmp		BlitTrans5

BlitTrans4:
		sub		dx, Z_SUBLAYERS

BlitTrans5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitTrans1:

		dec		LSCount  // decrement the pixels to blit
		jz		RightSkipLoop  // done the line

		dec		ecx
		jnz		BlitTrans2

		jmp		BlitDispatch

                        //---------------------------------------------
                        // Scans the ETRLE until it finds an EOL marker

RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

RSLoop2:

		xor		uiLineFlag, 1
		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

        // reset all the z-level stuff for a new line

		mov		ax, usZStartLevel
		mov		usZLevel, ax
		mov		ax, usZStartIndex
		mov		usZIndex, ax
		mov		ax, usZStartCols
		mov		usZColsToGo, ax


		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

void CorrectRenderCenter(int16_t sRenderX, int16_t sRenderY, int16_t *pSNewX, int16_t *pSNewY) {
  int16_t sScreenX, sScreenY;
  int16_t sNumXSteps, sNumYSteps;

  // Use radar scale values to get screen values, then convert ot map values, rounding to nearest
  // middle tile
  sScreenX = (int16_t)sRenderX;
  sScreenY = (int16_t)sRenderY;

  // Adjust for offsets!
  sScreenX += 0;
  sScreenY += 10;

  // Adjust to viewport start!
  sScreenX -= ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2);
  sScreenY -= ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2);

  // Make sure these coordinates are multiples of scroll steps
  sNumXSteps = sScreenX / gubNewScrollXSpeeds[gfDoVideoScroll][gubCurScrollSpeedID];
  sNumYSteps = sScreenY / gubNewScrollYSpeeds[gfDoVideoScroll][gubCurScrollSpeedID];

  sScreenX = (sNumXSteps * gubNewScrollXSpeeds[gfDoVideoScroll][gubCurScrollSpeedID]);
  sScreenY = (sNumYSteps * gubNewScrollYSpeeds[gfDoVideoScroll][gubCurScrollSpeedID]);

  // Adjust back
  sScreenX += ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2);
  sScreenY += ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2);

  *pSNewX = sScreenX;
  *pSNewY = sScreenY;
}

// Blitter Specs
// 1 ) 8 to 16 bpp
// 2 ) strip z-blitter
// 3 ) clipped
// 4 ) trans shadow - if value is 254, makes a shadow
//
BOOLEAN Blt8BPPDataTo16BPPBufferTransZTransShadowIncClip(
    uint16_t *pBuffer, uint32_t uiDestPitchBYTES, uint16_t *pZBuffer, uint16_t usZValue,
    struct VObject *hSrcVObject, int32_t iX, int32_t iY, uint16_t usIndex, SGPRect *clipregion,
    int16_t sZIndex, uint16_t *p16BPPPalette) {
  uint32_t uiOffset;
  uint32_t usHeight, usWidth, Unblitted;
  uint8_t *SrcPtr, *DestPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY, LeftSkip, RightSkip, TopSkip, BottomSkip, BlitLength, BlitHeight, LSCount;
  int32_t ClipX1, ClipY1, ClipX2, ClipY2;
  uint16_t usZLevel, usZStartLevel, usZColsToGo, usZStartIndex, usCount, usZIndex, usZStartCols;
  int8_t *pZArray;
  ZStripInfo *pZInfo;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  if (clipregion == NULL) {
    ClipX1 = ClippingRect.iLeft;
    ClipY1 = ClippingRect.iTop;
    ClipX2 = ClippingRect.iRight;
    ClipY2 = ClippingRect.iBottom;
  } else {
    ClipX1 = clipregion->iLeft;
    ClipY1 = clipregion->iTop;
    ClipX2 = clipregion->iRight;
    ClipY2 = clipregion->iBottom;
  }

  // Calculate rows hanging off each side of the screen
  LeftSkip = min(ClipX1 - min(ClipX1, iTempX), (int32_t)usWidth);
  RightSkip = min(max(ClipX2, (iTempX + (int32_t)usWidth)) - ClipX2, (int32_t)usWidth);
  TopSkip = min(ClipY1 - min(ClipY1, iTempY), (int32_t)usHeight);
  BottomSkip = min(max(ClipY2, (iTempY + (int32_t)usHeight)) - ClipY2, (int32_t)usHeight);

  // calculate the remaining rows and columns to blit
  BlitLength = ((int32_t)usWidth - LeftSkip - RightSkip);
  BlitHeight = ((int32_t)usHeight - TopSkip - BottomSkip);

  // check if whole thing is clipped
  if ((LeftSkip >= (int32_t)usWidth) || (RightSkip >= (int32_t)usWidth)) return (TRUE);

  // check if whole thing is clipped
  if ((TopSkip >= (int32_t)usHeight) || (BottomSkip >= (int32_t)usHeight)) return (TRUE);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr =
      (uint8_t *)pBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  ZPtr = (uint8_t *)pZBuffer + (uiDestPitchBYTES * (iTempY + TopSkip)) + ((iTempX + LeftSkip) * 2);
  LineSkip = (uiDestPitchBYTES - (BlitLength * 2));

  if (hSrcVObject->ppZStripInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }
  // setup for the z-column blitting stuff
  pZInfo = hSrcVObject->ppZStripInfo[sZIndex];
  if (pZInfo == NULL) {
    DebugMsg(TOPIC_VIDEOOBJECT, DBG_LEVEL_0, String("Missing Z-Strip info on multi-Z object"));
    return (FALSE);
  }

  usZStartLevel =
      (uint16_t)((int16_t)usZValue + ((int16_t)pZInfo->bInitialZChange * Z_SUBLAYERS * 10));

  if (LeftSkip > pZInfo->ubFirstZStripWidth) {
    usZStartCols = (LeftSkip - pZInfo->ubFirstZStripWidth);
    usZStartCols = 20 - (usZStartCols % 20);
  } else if (LeftSkip < pZInfo->ubFirstZStripWidth)
    usZStartCols = (uint16_t)(pZInfo->ubFirstZStripWidth - LeftSkip);
  else
    usZStartCols = 20;

  // set to odd number of pixels for first column
  usZColsToGo = usZStartCols;

  pZArray = pZInfo->pbZChange;

  if (LeftSkip >= usZColsToGo) {
    // Index into array after doing left clipping
    usZStartIndex = 1 + ((LeftSkip - pZInfo->ubFirstZStripWidth) / 20);

    // calculates the Z-value after left-side clipping
    if (usZStartIndex) {
      for (usCount = 0; usCount < usZStartIndex; usCount++) {
        switch (pZArray[usCount]) {
          case -1:
            usZStartLevel -= Z_SUBLAYERS;
            break;
          case 0:  // no change
            break;
          case 1:
            usZStartLevel += Z_SUBLAYERS;
            break;
        }
      }
    }
  } else
    usZStartIndex = 0;

  usZLevel = usZStartLevel;
  usZIndex = usZStartIndex;

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

		cmp		TopSkip, 0  // check for nothing clipped on top
		je		LeftSkipSetup

        // Skips the number of lines clipped at the top
TopSkipLoop:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		TopSkipLoop
		jz		TSEndLine

		add		esi, ecx
		jmp		TopSkipLoop

TSEndLine:
		dec		TopSkip
		jnz		TopSkipLoop

        // Start of line loop

        // Skips the pixels hanging outside the left-side boundry
LeftSkipSetup:

		mov		Unblitted, 0  // Unblitted counts any pixels left from a run
		mov		eax, LeftSkip  // after we have skipped enough left-side pixels
		mov		LSCount, eax  // LSCount counts how many pixels skipped so far
		or		eax, eax
		jz		BlitLineSetup  // check for nothing to skip

LeftSkipLoop:

		mov		cl, [esi]
		inc		esi

		or		cl, cl
		js		LSTrans

		cmp		ecx, LSCount
		je		LSSkip2  // if equal, skip whole, and start blit with new run
		jb		LSSkip1  // if less, skip whole thing

		add		esi, LSCount  // skip partial run, jump into normal loop for rest
		sub		ecx, LSCount
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0
		jmp		BlitNTL1  // *** jumps into non-transparent blit loop

LSSkip2:
		add		esi, ecx  // skip whole run, and start blit with new run
		jmp		BlitLineSetup


LSSkip1:
		add		esi, ecx  // skip whole run, continue skipping
		sub		LSCount, ecx
		jmp		LeftSkipLoop


LSTrans:
		and		ecx, 07fH
		cmp		ecx, LSCount
		je		BlitLineSetup  // if equal, skip whole, and start blit with new run
		jb		LSTrans1  // if less, skip whole thing

		sub		ecx, LSCount  // skip partial run, jump into normal loop for rest
		mov		eax, BlitLength
		mov		LSCount, eax

		mov		Unblitted, 0
		jmp		BlitTransparent  // *** jumps into transparent blit loop


LSTrans1:
		sub		LSCount, ecx  // skip whole run, continue skipping
		jmp		LeftSkipLoop

            //-------------------------------------------------
            // setup for beginning of line

BlitLineSetup:
		mov		eax, BlitLength
		mov		LSCount, eax
		mov		Unblitted, 0

BlitDispatch:

		cmp		LSCount, 0  // Check to see if we're done blitting
		je		RightSkipLoop

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		RSLoop2

        //--------------------------------
        // blitting non-transparent pixels

		and		ecx, 07fH

BlitNTL1:
		mov		ax, [ebx]  // check z-level of pixel
		cmp		ax, usZLevel
		ja		BlitNTL2

		mov		ax, usZLevel  // update z-level of pixel
		mov		[ebx], ax

        // Check for shadow...
		xor		eax, eax
		mov		al, [esi]
		cmp		al, 254
		jne		BlitNTL66

		mov		ax, [edi]
		mov		ax, ShadeTable[eax*2]
		mov		[edi], ax
		jmp		BlitNTL2

BlitNTL66:

		mov		ax, [edx+eax*2]  // Copy pixel
		mov		[edi], ax

BlitNTL2:
		inc		esi
		add		edi, 2
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitNTL6

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitNTL5  // dir = 0 no change
		js		BlitNTL4               // dir < 0 z-level down
                    // dir > 0 z-level up (default)
		add		dx, Z_SUBLAYERS
		jmp		BlitNTL5

BlitNTL4:
		sub		dx, Z_SUBLAYERS

BlitNTL5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitNTL6:
		dec		LSCount  // decrement pixel length to blit
		jz		RightSkipLoop  // done blitting the visible line

		dec		ecx
		jnz		BlitNTL1  // continue current run

		jmp		BlitDispatch  // done current run, go for another

                    //----------------------------
                    // skipping transparent pixels

BlitTransparent:  // skip transparent pixels

		and		ecx, 07fH

BlitTrans2:

		add		edi, 2  // move up the destination pointer
		add		ebx, 2

		dec		usZColsToGo
		jnz		BlitTrans1

        // update the z-level according to the z-table

		push	edx
		mov		edx, pZArray  // get pointer to array
		xor		eax, eax
		mov		ax, usZIndex  // pick up the current array index
		add		edx, eax
		inc		eax  // increment it
		mov		usZIndex, ax  // store incremented value

		mov		al, [edx]  // get direction instruction
		mov		dx, usZLevel  // get current z-level

		or		al, al
		jz		BlitTrans5  // dir = 0 no change
		js		BlitTrans4         // dir < 0 z-level down
                      // dir > 0 z-level up (default)
		add		dx, Z_SUBLAYERS
		jmp		BlitTrans5

BlitTrans4:
		sub		dx, Z_SUBLAYERS

BlitTrans5:
		mov		usZLevel, dx  // store the now-modified z-level
		mov		usZColsToGo, 20  // reset the next z-level change to 20 cols
		pop		edx

BlitTrans1:

		dec		LSCount  // decrement the pixels to blit
		jz		RightSkipLoop  // done the line

		dec		ecx
		jnz		BlitTrans2

		jmp		BlitDispatch

                        //---------------------------------------------
                        // Scans the ETRLE until it finds an EOL marker

RightSkipLoop:


RSLoop1:
		mov		al, [esi]
		inc		esi
		or		al, al
		jnz		RSLoop1

RSLoop2:

		dec		BlitHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip

        // reset all the z-level stuff for a new line

		mov		ax, usZStartLevel
		mov		usZLevel, ax
		mov		ax, usZStartIndex
		mov		usZIndex, ax
		mov		ax, usZStartCols
		mov		usZColsToGo, ax


		jmp		LeftSkipSetup


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

void RenderRoomInfo(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                    int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS) {
  int8_t bXOddFlag = 0;
  int16_t sAnchorPosX_M, sAnchorPosY_M;
  int16_t sAnchorPosX_S, sAnchorPosY_S;
  int16_t sTempPosX_M, sTempPosY_M;
  int16_t sTempPosX_S, sTempPosY_S;
  BOOLEAN fEndRenderRow = FALSE, fEndRenderCol = FALSE;
  uint16_t usTileIndex;
  int16_t sX, sY;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // Begin Render Loop
  sAnchorPosX_M = sStartPointX_M;
  sAnchorPosY_M = sStartPointY_M;
  sAnchorPosX_S = sStartPointX_S;
  sAnchorPosY_S = sStartPointY_S;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  do {
    fEndRenderRow = FALSE;
    sTempPosX_M = sAnchorPosX_M;
    sTempPosY_M = sAnchorPosY_M;
    sTempPosX_S = sAnchorPosX_S;
    sTempPosY_S = sAnchorPosY_S;

    if (bXOddFlag > 0) sTempPosX_S += 20;

    do {
      usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

      if (usTileIndex < GRIDSIZE) {
        sX = sTempPosX_S + (WORLD_TILE_X / 2) - 5;
        sY = sTempPosY_S + (WORLD_TILE_Y / 2) - 5;

        // THIS ROOM STUFF IS ONLY DONE IN THE EDITOR...
        // ADJUST BY SHEIGHT
        sY -= gpWorldLevelData[usTileIndex].sHeight;
        // sY += gsRenderHeight;

        if (gubWorldRoomInfo[usTileIndex] != NO_ROOM) {
          SetFont(SMALLCOMPFONT);
          SetFontDestBuffer(vsFB, 0, 0, 640, gsVIEWPORT_END_Y, FALSE);
          switch (gubWorldRoomInfo[usTileIndex] % 5) {
            case 0:
              SetFontForeground(FONT_GRAY3);
              break;
            case 1:
              SetFontForeground(FONT_YELLOW);
              break;
            case 2:
              SetFontForeground(FONT_LTRED);
              break;
            case 3:
              SetFontForeground(FONT_LTBLUE);
              break;
            case 4:
              SetFontForeground(FONT_LTGREEN);
              break;
          }
          mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY, L"%d",
                         gubWorldRoomInfo[usTileIndex]);
          SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
        }
      }

      sTempPosX_S += 40;
      sTempPosX_M++;
      sTempPosY_M--;

      if (sTempPosX_S >= sEndXS) {
        fEndRenderRow = TRUE;
      }

    } while (!fEndRenderRow);

    if (bXOddFlag > 0) {
      sAnchorPosY_M++;
    } else {
      sAnchorPosX_M++;
    }

    bXOddFlag = !bXOddFlag;
    sAnchorPosY_S += 10;

    if (sAnchorPosY_S >= sEndYS) {
      fEndRenderCol = TRUE;
    }

  } while (!fEndRenderCol);

  JSurface_Unlock(vsFB);
}

#ifdef _DEBUG

void RenderFOVDebugInfo(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                        int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS) {
  int8_t bXOddFlag = 0;
  int16_t sAnchorPosX_M, sAnchorPosY_M;
  int16_t sAnchorPosX_S, sAnchorPosY_S;
  int16_t sTempPosX_M, sTempPosY_M;
  int16_t sTempPosX_S, sTempPosY_S;
  BOOLEAN fEndRenderRow = FALSE, fEndRenderCol = FALSE;
  uint16_t usTileIndex;
  int16_t sX, sY;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // Begin Render Loop
  sAnchorPosX_M = sStartPointX_M;
  sAnchorPosY_M = sStartPointY_M;
  sAnchorPosX_S = sStartPointX_S;
  sAnchorPosY_S = sStartPointY_S;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  do {
    fEndRenderRow = FALSE;
    sTempPosX_M = sAnchorPosX_M;
    sTempPosY_M = sAnchorPosY_M;
    sTempPosX_S = sAnchorPosX_S;
    sTempPosY_S = sAnchorPosY_S;

    if (bXOddFlag > 0) sTempPosX_S += 20;

    do {
      usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

      if (usTileIndex < GRIDSIZE) {
        sX = sTempPosX_S + (WORLD_TILE_X / 2) - 5;
        sY = sTempPosY_S + (WORLD_TILE_Y / 2) - 5;

        // Adjust for interface level
        sY -= gpWorldLevelData[usTileIndex].sHeight;
        sY += gsRenderHeight;

        if (gubFOVDebugInfoInfo[usTileIndex] != 0) {
          SetFont(SMALLCOMPFONT);
          SetFontDestBuffer(vsFB, 0, 0, 640, gsVIEWPORT_END_Y, FALSE);
          SetFontForeground(FONT_GRAY3);
          mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY, L"%d",
                         gubFOVDebugInfoInfo[usTileIndex]);
          SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

          Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                                  gTileDatabase[0].hTileSurface, sTempPosX_S,
                                                  sTempPosY_S, 0, &gClippingRect);
        }

        if (gubGridNoMarkers[usTileIndex] == gubGridNoValue) {
          SetFont(SMALLCOMPFONT);
          SetFontDestBuffer(vsFB, 0, 0, 640, gsVIEWPORT_END_Y, FALSE);
          SetFontForeground(FONT_FCOLOR_YELLOW);
          mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY + 4, L"x");
          SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
        }
      }

      sTempPosX_S += 40;
      sTempPosX_M++;
      sTempPosY_M--;

      if (sTempPosX_S >= sEndXS) {
        fEndRenderRow = TRUE;
      }

    } while (!fEndRenderRow);

    if (bXOddFlag > 0) {
      sAnchorPosY_M++;
    } else {
      sAnchorPosX_M++;
    }

    bXOddFlag = !bXOddFlag;
    sAnchorPosY_S += 10;

    if (sAnchorPosY_S >= sEndYS) {
      fEndRenderCol = TRUE;
    }

  } while (!fEndRenderCol);

  JSurface_Unlock(vsFB);
}

void RenderCoverDebugInfo(int16_t sStartPointX_M, int16_t sStartPointY_M, int16_t sStartPointX_S,
                          int16_t sStartPointY_S, int16_t sEndXS, int16_t sEndYS) {
  int8_t bXOddFlag = 0;
  int16_t sAnchorPosX_M, sAnchorPosY_M;
  int16_t sAnchorPosX_S, sAnchorPosY_S;
  int16_t sTempPosX_M, sTempPosY_M;
  int16_t sTempPosX_S, sTempPosY_S;
  BOOLEAN fEndRenderRow = FALSE, fEndRenderCol = FALSE;
  uint16_t usTileIndex;
  int16_t sX, sY;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // Begin Render Loop
  sAnchorPosX_M = sStartPointX_M;
  sAnchorPosY_M = sStartPointY_M;
  sAnchorPosX_S = sStartPointX_S;
  sAnchorPosY_S = sStartPointY_S;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  do {
    fEndRenderRow = FALSE;
    sTempPosX_M = sAnchorPosX_M;
    sTempPosY_M = sAnchorPosY_M;
    sTempPosX_S = sAnchorPosX_S;
    sTempPosY_S = sAnchorPosY_S;

    if (bXOddFlag > 0) sTempPosX_S += 20;

    do {
      usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

      if (usTileIndex < GRIDSIZE) {
        sX = sTempPosX_S + (WORLD_TILE_X / 2) - 5;
        sY = sTempPosY_S + (WORLD_TILE_Y / 2) - 5;

        // Adjust for interface level
        sY -= gpWorldLevelData[usTileIndex].sHeight;
        sY += gsRenderHeight;

        if (gsCoverValue[usTileIndex] != 0x7F7F) {
          SetFont(SMALLCOMPFONT);
          SetFontDestBuffer(vsFB, 0, 0, 640, gsVIEWPORT_END_Y, FALSE);
          if (usTileIndex == gsBestCover) {
            SetFontForeground(FONT_MCOLOR_RED);
          } else if (gsCoverValue[usTileIndex] < 0) {
            SetFontForeground(FONT_MCOLOR_WHITE);
          } else {
            SetFontForeground(FONT_GRAY3);
          }
          mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY, L"%d",
                         gsCoverValue[usTileIndex]);
          SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
        }
      }

      sTempPosX_S += 40;
      sTempPosX_M++;
      sTempPosY_M--;

      if (sTempPosX_S >= sEndXS) {
        fEndRenderRow = TRUE;
      }

    } while (!fEndRenderRow);

    if (bXOddFlag > 0) {
      sAnchorPosY_M++;
    } else {
      sAnchorPosX_M++;
    }

    bXOddFlag = !bXOddFlag;
    sAnchorPosY_S += 10;

    if (sAnchorPosY_S >= sEndYS) {
      fEndRenderCol = TRUE;
    }

  } while (!fEndRenderCol);

  JSurface_Unlock(vsFB);
}

void RenderGridNoVisibleDebugInfo(int16_t sStartPointX_M, int16_t sStartPointY_M,
                                  int16_t sStartPointX_S, int16_t sStartPointY_S, int16_t sEndXS,
                                  int16_t sEndYS) {
  int8_t bXOddFlag = 0;
  int16_t sAnchorPosX_M, sAnchorPosY_M;
  int16_t sAnchorPosX_S, sAnchorPosY_S;
  int16_t sTempPosX_M, sTempPosY_M;
  int16_t sTempPosX_S, sTempPosY_S;
  BOOLEAN fEndRenderRow = FALSE, fEndRenderCol = FALSE;
  uint16_t usTileIndex;
  int16_t sX, sY;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // Begin Render Loop
  sAnchorPosX_M = sStartPointX_M;
  sAnchorPosY_M = sStartPointY_M;
  sAnchorPosX_S = sStartPointX_S;
  sAnchorPosY_S = sStartPointY_S;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  do {
    fEndRenderRow = FALSE;
    sTempPosX_M = sAnchorPosX_M;
    sTempPosY_M = sAnchorPosY_M;
    sTempPosX_S = sAnchorPosX_S;
    sTempPosY_S = sAnchorPosY_S;

    if (bXOddFlag > 0) sTempPosX_S += 20;

    do {
      usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

      if (usTileIndex < GRIDSIZE) {
        sX = sTempPosX_S + (WORLD_TILE_X / 2) - 5;
        sY = sTempPosY_S + (WORLD_TILE_Y / 2) - 5;

        // Adjust for interface level
        sY -= gpWorldLevelData[usTileIndex].sHeight;
        sY += gsRenderHeight;

        SetFont(SMALLCOMPFONT);
        SetFontDestBuffer(vsFB, 0, 0, 640, gsVIEWPORT_END_Y, FALSE);

        if (!GridNoOnVisibleWorldTile(usTileIndex)) {
          SetFontForeground(FONT_MCOLOR_RED);
        } else {
          SetFontForeground(FONT_GRAY3);
        }
        mprintf_buffer(pDestBuf, uiDestPitchBYTES, TINYFONT1, sX, sY, L"%d", usTileIndex);
        SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
      }

      sTempPosX_S += 40;
      sTempPosX_M++;
      sTempPosY_M--;

      if (sTempPosX_S >= sEndXS) {
        fEndRenderRow = TRUE;
      }

    } while (!fEndRenderRow);

    if (bXOddFlag > 0) {
      sAnchorPosY_M++;
    } else {
      sAnchorPosX_M++;
    }

    bXOddFlag = !bXOddFlag;
    sAnchorPosY_S += 10;

    if (sAnchorPosY_S >= sEndYS) {
      fEndRenderCol = TRUE;
    }

  } while (!fEndRenderCol);

  JSurface_Unlock(vsFB);
}

#endif

void ExamineZBufferRect(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom) {
  CalcRenderParameters(sLeft, sTop, sRight, sBottom);

  ExamineZBufferForHiddenTiles(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
                               gsEndXS, gsEndYS);
}

void ExamineZBufferForHiddenTiles(int16_t sStartPointX_M, int16_t sStartPointY_M,
                                  int16_t sStartPointX_S, int16_t sStartPointY_S, int16_t sEndXS,
                                  int16_t sEndYS) {
  int8_t bXOddFlag = 0;
  int16_t sAnchorPosX_M, sAnchorPosY_M;
  int16_t sAnchorPosX_S, sAnchorPosY_S;
  int16_t sTempPosX_M, sTempPosY_M;
  int16_t sTempPosX_S, sTempPosY_S;
  BOOLEAN fEndRenderRow = FALSE, fEndRenderCol = FALSE;
  uint16_t usTileIndex;
  int16_t sX, sY, sWorldX, sZLevel;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  TILE_ELEMENT *TileElem;
  int8_t bBlitClipVal;
  struct LEVELNODE *pObject;

  // Begin Render Loop
  sAnchorPosX_M = sStartPointX_M;
  sAnchorPosY_M = sStartPointY_M;
  sAnchorPosX_S = sStartPointX_S;
  sAnchorPosY_S = sStartPointY_S;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  // Get VObject for firt land peice!
  TileElem = &(gTileDatabase[FIRSTTEXTURE1]);

  do {
    fEndRenderRow = FALSE;
    sTempPosX_M = sAnchorPosX_M;
    sTempPosY_M = sAnchorPosY_M;
    sTempPosX_S = sAnchorPosX_S;
    sTempPosY_S = sAnchorPosY_S;

    if (bXOddFlag > 0) sTempPosX_S += 20;

    do {
      usTileIndex = FASTMAPROWCOLTOPOS(sTempPosY_M, sTempPosX_M);

      if (usTileIndex < GRIDSIZE) {
        // ATE: Don;t let any vehicle sit here....
        if (FindStructure(usTileIndex, (STRUCTURE_MOBILE))) {
          // Continue...
          goto ENDOFLOOP;
        }

        sX = sTempPosX_S;
        sY = sTempPosY_S - gpWorldLevelData[usTileIndex].sHeight;

        // Adjust for interface level
        sY += gsRenderHeight;

        // Caluluate zvalue
        // Look for anything less than struct layer!
        GetWorldXYAbsoluteScreenXY(sTempPosX_M, sTempPosY_M, &sWorldX, &sZLevel);

        sZLevel += gsRenderHeight;

        sZLevel = (sZLevel * Z_SUBLAYERS) + STRUCT_Z_LEVEL;

        if (gpWorldLevelData[usTileIndex].uiFlags & MAPELEMENT_REEVALUATE_REDUNDENCY) {
          bBlitClipVal = BltIsClippedOrOffScreen(TileElem->hTileSurface, sX, sY,
                                                 TileElem->usRegionIndex, &gClippingRect);

          if (bBlitClipVal == FALSE) {
            // Set flag to not evaluate again!
            gpWorldLevelData[usTileIndex].uiFlags &= (~MAPELEMENT_REEVALUATE_REDUNDENCY);

            // OK, first do some rules with exceptions
            // Don't let this happen for roads!
            pObject = gpWorldLevelData[usTileIndex].pObjectHead;

            if (IsTileRedundent(gpZBuffer, sZLevel, TileElem->hTileSurface, sX, sY,
                                TileElem->usRegionIndex)) {
              // Mark in the world!
              gpWorldLevelData[usTileIndex].uiFlags |= MAPELEMENT_REDUNDENT;
            } else {
              // Un Mark in the world!
              gpWorldLevelData[usTileIndex].uiFlags &= (~MAPELEMENT_REDUNDENT);
            }
          }
        }
      }

    ENDOFLOOP:

      sTempPosX_S += 40;
      sTempPosX_M++;
      sTempPosY_M--;

      if (sTempPosX_S >= sEndXS) {
        fEndRenderRow = TRUE;
      }

    } while (!fEndRenderRow);

    if (bXOddFlag > 0) {
      sAnchorPosY_M++;
    } else {
      sAnchorPosX_M++;
    }

    bXOddFlag = !bXOddFlag;
    sAnchorPosY_S += 10;

    if (sAnchorPosY_S >= sEndYS) {
      fEndRenderCol = TRUE;
    }

  } while (!fEndRenderCol);

  JSurface_Unlock(vsFB);
}

void CalcRenderParameters(int16_t sLeft, int16_t sTop, int16_t sRight, int16_t sBottom) {
  int16_t sTempPosX_W, sTempPosY_W;
  int16_t sRenderCenterX_W, sRenderCenterY_W;
  int16_t sOffsetX_W, sOffsetY_W, sOffsetX_S, sOffsetY_S;

  gOldClipRect = gClippingRect;

  // Set new clipped rect
  gClippingRect.iLeft = max(gsVIEWPORT_START_X, sLeft);
  gClippingRect.iRight = min(gsVIEWPORT_END_X, sRight);
  gClippingRect.iTop = max(gsVIEWPORT_WINDOW_START_Y, sTop);
  gClippingRect.iBottom = min(gsVIEWPORT_WINDOW_END_Y, sBottom);

  gsEndXS = sRight + VIEWPORT_XOFFSET_S;
  gsEndYS = sBottom + VIEWPORT_YOFFSET_S;

  sRenderCenterX_W = gsRenderCenterX;
  sRenderCenterY_W = gsRenderCenterY;

  // STEP THREE - determine starting point in world coords
  // a) Determine where in screen coords to start rendering
  gsStartPointX_S = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) - (sLeft - VIEWPORT_XOFFSET_S);
  gsStartPointY_S = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) - (sTop - VIEWPORT_YOFFSET_S);

  // b) Convert these distances into world distances
  FromScreenToCellCoordinates(gsStartPointX_S, gsStartPointY_S, &sTempPosX_W, &sTempPosY_W);

  // c) World start point is Render center minus this distance
  gsStartPointX_W = sRenderCenterX_W - sTempPosX_W;
  gsStartPointY_W = sRenderCenterY_W - sTempPosY_W;

  // NOTE: Increase X map value by 1 tile to offset where on screen we are...
  if (gsStartPointX_W > 0) gsStartPointX_W += CELL_X_SIZE;

  // d) screen start point is screen distances minus screen center
  gsStartPointX_S = sLeft - VIEWPORT_XOFFSET_S;
  gsStartPointY_S = sTop - VIEWPORT_YOFFSET_S;

  // STEP FOUR - Determine Start block
  // a) Find start block
  gsStartPointX_M = (gsStartPointX_W) / CELL_X_SIZE;
  gsStartPointY_M = (gsStartPointY_W) / CELL_Y_SIZE;

  // STEP 5 - Determine Deltas for center and find screen values
  // Make sure these coordinates are multiples of scroll steps
  sOffsetX_W = abs(gsStartPointX_W) - (abs((gsStartPointX_M * CELL_X_SIZE)));
  sOffsetY_W = abs(gsStartPointY_W) - (abs((gsStartPointY_M * CELL_Y_SIZE)));

  FromCellToScreenCoordinates(sOffsetX_W, sOffsetY_W, &sOffsetX_S, &sOffsetY_S);

  if (gsStartPointY_W < 0) {
    gsStartPointY_S += 0;  //(sOffsetY_S/2);
  } else {
    gsStartPointY_S -= sOffsetY_S;
  }
  gsStartPointX_S -= sOffsetX_S;

  // Set globals for render offset
  if (gsRenderWorldOffsetX == -1) {
    gsRenderWorldOffsetX = sOffsetX_S;
  }

  if (gsRenderWorldOffsetY == -1) {
    gsRenderWorldOffsetY = sOffsetY_S;
  }

  /////////////////////////////////////////
  // ATE: CALCULATE LARGER OFFSET VALUES
  gsLEndXS = sRight + LARGER_VIEWPORT_XOFFSET_S;
  gsLEndYS = sBottom + LARGER_VIEWPORT_YOFFSET_S;

  // STEP THREE - determine starting point in world coords
  // a) Determine where in screen coords to start rendering
  gsLStartPointX_S =
      ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) - (sLeft - LARGER_VIEWPORT_XOFFSET_S);
  gsLStartPointY_S =
      ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) - (sTop - LARGER_VIEWPORT_YOFFSET_S);

  // b) Convert these distances into world distances
  FromScreenToCellCoordinates(gsLStartPointX_S, gsLStartPointY_S, &sTempPosX_W, &sTempPosY_W);

  // c) World start point is Render center minus this distance
  gsLStartPointX_W = sRenderCenterX_W - sTempPosX_W;
  gsLStartPointY_W = sRenderCenterY_W - sTempPosY_W;

  // NOTE: Increase X map value by 1 tile to offset where on screen we are...
  if (gsLStartPointX_W > 0) gsLStartPointX_W += CELL_X_SIZE;

  // d) screen start point is screen distances minus screen center
  gsLStartPointX_S = sLeft - LARGER_VIEWPORT_XOFFSET_S;
  gsLStartPointY_S = sTop - LARGER_VIEWPORT_YOFFSET_S;

  // STEP FOUR - Determine Start block
  // a) Find start block
  gsLStartPointX_M = (gsLStartPointX_W) / CELL_X_SIZE;
  gsLStartPointY_M = (gsLStartPointY_W) / CELL_Y_SIZE;

  // Adjust starting screen coordinates
  gsLStartPointX_S -= sOffsetX_S;

  if (gsLStartPointY_W < 0) {
    gsLStartPointY_S += 0;
    gsLStartPointX_S -= 20;
  } else {
    gsLStartPointY_S -= sOffsetY_S;
  }
}

void ResetRenderParameters() {
  // Restore clipping rect
  gClippingRect = gOldClipRect;
}

BOOLEAN Zero8BPPDataTo16BPPBufferTransparent(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                             struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                             uint16_t usIndex) {
  uint32_t uiOffset;
  uint32_t usHeight, usWidth;
  uint8_t *SrcPtr, *DestPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  // Validations
  CHECKF(iTempX >= 0);
  CHECKF(iTempY >= 0);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr = (uint8_t *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		mov		[edi], ax

		inc		esi
		add		edi, 2

BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		mov		[edi], ax

		mov		[edi+2], ax

		add		esi, 2
		add		edi, 4

BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		mov		[edi], ax

		mov		[edi+2], ax

		mov		[edi+4], ax

		mov		[edi+6], ax

		add		esi, 4
		add		edi, 8
		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN Blt8BPPDataTo16BPPBufferTransInvZ(uint16_t *pBuffer, uint32_t uiDestPitchBYTES,
                                          uint16_t *pZBuffer, uint16_t usZValue,
                                          struct VObject *hSrcVObject, int32_t iX, int32_t iY,
                                          uint16_t usIndex) {
  uint16_t *p16BPPPalette;
  uint32_t uiOffset;
  uint32_t usHeight, usWidth;
  uint8_t *SrcPtr, *DestPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY;

  // Assertions
  Assert(hSrcVObject != NULL);
  Assert(pBuffer != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  // Validations
  CHECKF(iTempX >= 0);
  CHECKF(iTempY >= 0);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  DestPtr = (uint8_t *)pBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  ZPtr = (uint8_t *)pZBuffer + (uiDestPitchBYTES * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (uiDestPitchBYTES - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edi, DestPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jne		BlitNTL5

        // mov		[ebx], ax

		xor		ah, ah
		mov		al, [esi]
		mov		ax, [edx+eax*2]
		mov		[edi], ax

BlitNTL5:
		inc		esi
		inc		edi
		inc		ebx
		inc		edi
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		edi, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		edi, LineSkip
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (TRUE);
}

BOOLEAN IsTileRedundent(uint16_t *pZBuffer, uint16_t usZValue, struct VObject *hSrcVObject,
                        int32_t iX, int32_t iY, uint16_t usIndex) {
  uint16_t *p16BPPPalette;
  uint32_t uiOffset;
  uint32_t usHeight, usWidth;
  uint8_t *SrcPtr, *ZPtr;
  uint32_t LineSkip;
  ETRLEObject *pTrav;
  int32_t iTempX, iTempY;
  BOOLEAN fHidden = TRUE;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[usIndex]);
  usHeight = (uint32_t)pTrav->usHeight;
  usWidth = (uint32_t)pTrav->usWidth;
  uiOffset = pTrav->uiDataOffset;

  // Add to start position of dest buffer
  iTempX = iX + pTrav->sOffsetX;
  iTempY = iY + pTrav->sOffsetY;

  // Validations
  CHECKF(iTempX >= 0);
  CHECKF(iTempY >= 0);

  SrcPtr = (uint8_t *)hSrcVObject->pPixData + uiOffset;
  ZPtr = (uint8_t *)pZBuffer + (1280 * iTempY) + (iTempX * 2);
  p16BPPPalette = hSrcVObject->pShadeCurrent;
  LineSkip = (1280 - (usWidth * 2));

#ifdef _WINDOWS
  __asm {

		mov		esi, SrcPtr
		mov		edx, p16BPPPalette
		xor		eax, eax
		mov		ebx, ZPtr
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

        // BlitNonTransLoop:

		xor		eax, eax

BlitNTL4:

		mov		ax, usZValue
		cmp		ax, [ebx]
		jle		BlitNTL5

        //    Set false, flag
		mov   fHidden, 0
		jmp		BlitDone


BlitNTL5:
		inc		esi
		inc		ebx
		inc		ebx

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch


BlitTransparent:

		and		ecx, 07fH
    //		shl		ecx, 1
		add   ecx, ecx
		add		ebx, ecx
		jmp		BlitDispatch


BlitDoneLine:

		dec		usHeight
		jz		BlitDone
		add		ebx, LineSkip
		jmp		BlitDispatch


BlitDone:
  }
#else
  // Linux: NOT IMPLEMENTED
#endif

  return (fHidden);
}

void SetMercGlowFast() {
  // gpGlowFramePointer	= gsFastGlowFrames;
}

void SetMercGlowNormal() { gpGlowFramePointer = gsGlowFrames; }

#if 0
		if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_MOVING )
		{
			if ( sZOffsetY > 0 )
			{
				sZOffsetY++;
			}
			if ( sZOffsetX > 0 )
			{
				sZOffsetX++;
			}
		}

		sZOffsetX = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetX;\
			sZOffsetY = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetY;\


	if ( ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE ) )\
	{\
		sZOffsetX = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetX;\
		sZOffsetY = pNode->pStructureData->pDBStructureRef->pDBStructure->bZTileOffsetY;\
\
		GetMapXYWorldY( sMapX + sZOffsetX, sMapY + sZOffsetY, sWorldY );\
	}\
	else

#endif

void SetRenderCenter(int16_t sNewX, int16_t sNewY) {
  if (gfIgnoreScrolling == 1) {
    return;
  }

  // Apply these new coordinates to the renderer!
  ApplyScrolling(sNewX, sNewY, TRUE, FALSE);

  // Set flag to ignore scrolling this frame
  gfIgnoreScrollDueToCenterAdjust = TRUE;

  // Set full render flag!
  // DIRTY THE WORLD!
  SetRenderFlags(RENDER_FLAG_FULL);

  gfPlotNewMovement = TRUE;

  if (gfScrollPending == TRUE) {
    // Do a complete rebuild!
    gfScrollPending = FALSE;

    // Restore Interface!
    RestoreInterface();

    // Delete Topmost blitters saved areas
    DeleteVideoOverlaysArea();
  }

  gfScrollInertia = FALSE;
}

#ifdef _DEBUG
void RenderFOVDebug() {
  RenderFOVDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
                     gsEndYS);
}

void RenderCoverDebug() {
  RenderCoverDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S, gsEndXS,
                       gsEndYS);
}

void RenderGridNoVisibleDebug() {
  RenderGridNoVisibleDebugInfo(gsStartPointX_M, gsStartPointY_M, gsStartPointX_S, gsStartPointY_S,
                               gsEndXS, gsEndYS);
}

#endif

#ifdef __GCC
#pragma GCC diagnostic pop
#endif
