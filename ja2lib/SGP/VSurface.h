// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

struct VObject;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface SGP Module
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Defines for special video object handles given to blit function
//

#define PRIMARY_SURFACE 0xFFFFFFF0
#define BACKBUFFER 0xFFFFFFF1
#define vsFB 0xFFFFFFF2
#define MOUSE_BUFFER 0xFFFFFFF3

//
// Defines for blitting
//

#define VS_BLT_COLORFILL 0x000000020
#define VS_BLT_USECOLORKEY 0x000000002
#define VS_BLT_USEDESTCOLORKEY 0x000000200
#define VS_BLT_FAST 0x000000004
#define VS_BLT_CLIPPED 0x000000008
#define VS_BLT_SRCSUBRECT 0x000000040
#define VS_BLT_COLORFILLRECT 0x000000100
#define VS_BLT_MIRROR_Y 0x000001000

//
// Effects structure for specialized blitting
//

typedef struct {
  COLORVAL ColorFill;   // Used for fill effect
  SGPRect SrcRect;      // Given SRC subrect instead of srcregion
  SGPRect FillRect;     // Given SRC subrect instead of srcregion
  uint16_t DestRegion;  // Given a DEST region for dest positions within the VO

} blt_vs_fx;

//
// This structure is a video Surface. Contains a HLIST of regions
//

struct VSurface {
  uint16_t usHeight;          // Height of Video Surface
  uint16_t usWidth;           // Width of Video Surface
  uint8_t ubBitDepth;         // BPP ALWAYS 16!
  void *pSurfaceData;         // A void pointer, but for this implementation, is really a
                              // lpDirectDrawSurface;
  void *pSurfaceData1;        // Direct Draw One Interface
  void *pSavedSurfaceData1;   // A void pointer, but for this implementation, is really a
                              // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video
                              // memory Surfaces so that they my be restored
  void *pSavedSurfaceData;    // A void pointer, but for this implementation, is really a
                              // lpDirectDrawSurface; pSavedSurfaceData is used to hold all video
                              // memory Surfaces so that they my be restored
  uint32_t fFlags;            // Used to describe memory usage, etc
  void *pPalette;             // A void pointer, but for this implementation a DDPalette
  uint16_t *p16BPPPalette;    // A 16BPP palette used for 8->16 blits
  COLORVAL TransparentColor;  // Defaults to 0,0,0
  void *pClipper;             // A void pointer encapsolated as a clipper Surface
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manager Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

extern int32_t giMemUsedInSurfaces;

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

// Restores all video Surfaces in list
BOOLEAN RestoreVideoSurfaces();

// Adds a video Surface to list
BOOLEAN AddVSurface(struct VSurface *vs, uint32_t *uiIndex);

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface(struct VSurface **hVSurface, uint32_t uiIndex);

uint8_t *LockVideoSurface(uint32_t uiVSurface, uint32_t *uiPitch);
void UnLockVideoSurface(uint32_t uiVSurface);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, uint16_t usRegionIndex,
                        int32_t iDestX, int32_t iDestY, uint32_t fBltFlags, blt_vs_fx *pBltFx);

BOOLEAN ColorFillVideoSurfaceArea(uint32_t uiDestVSurface, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, uint16_t Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(uint32_t uiDestVSurface, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, int16_t Ox, int16_t Oy);

// This function sets the global video Surfaces for primary and backbuffer
BOOLEAN SetPrimaryVideoSurfaces();

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(uint32_t uiIndex, COLORVAL TransColor);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct VSurface *CreateVSurfaceBlank(uint16_t width, uint16_t height, uint8_t bitDepth);
struct VSurface *CreateVSurfaceBlank8(uint16_t width, uint16_t height);
struct VSurface *CreateVSurfaceBlank16(uint16_t width, uint16_t height);
struct VSurface *CreateVSurfaceFromFile(const char *filepath);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette);

BOOLEAN RestoreVideoSurface(struct VSurface *hVSurface);

// Returns a flat pointer for direct manipulation of data
uint8_t *LockVideoSurfaceBuffer(struct VSurface *hVSurface, uint32_t *pPitch);

// Must be called after Locking buffer call above
void UnLockVideoSurfaceBuffer(struct VSurface *hVSurface);

// Set data from HIMAGE.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, HIMAGE hImage, uint16_t usX,
                                      uint16_t usY, SGPRect *pSrcRect);

// Sets Transparency color into HVSurface and the underlying DD surface
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *vs, COLORVAL TransColor);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette);

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface);
BOOLEAN DeleteVideoSurfaceFromIndex(uint32_t uiIndex);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoSurfaceToVideoSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                                      uint16_t usIndex, int32_t iDestX, int32_t iDestY,
                                      int32_t fBltFlags, blt_vs_fx *pBltFx);

BOOLEAN ShadowVideoSurfaceRect(uint32_t uiDestVSurface, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2);
BOOLEAN ShadowVideoSurfaceImage(uint32_t uiDestVSurface, struct VObject *hImageHandle,
                                int32_t iPosX, int32_t iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, int32_t iDestX,
                               int32_t iDestY, uint32_t fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(uint32_t uiDestVSurface, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2);

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

typedef uint32_t VSurfID;

bool AddVSurfaceFromFile(const char *filepath, VSurfID *index);

void InitVSurfaceList();
void DeinitVSurfaceList();

VSurfID AddVSurfaceToList(struct VSurface *vs);
bool DeleteVSurfaceFromList(VSurfID id);

struct VSurface *FindVSurface(VSurfID id);

#endif
