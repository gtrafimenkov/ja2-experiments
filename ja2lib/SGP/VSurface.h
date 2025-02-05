// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "Rect.h"
#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "jplatform_video.h"

struct VObject;
struct VSurface;

typedef uint32_t VSurfID;

extern struct VSurface *vsPrimary;
extern struct VSurface *vsBackBuffer;
extern struct VSurface *vsFB;
extern struct VSurface *vsMouseBuffer;
extern struct VSurface *vsMouseBufferOriginal;

extern struct VSurface *vsSaveBuffer;
extern struct VSurface *vsExtraBuffer;

extern BOOLEAN gfExtraBuffer;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface SGP Module
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// This structure is a video Surface. Contains a HLIST of regions
//

struct VSurface {
  uint16_t usHeight;        // Height of Video Surface
  uint16_t usWidth;         // Width of Video Surface
  uint8_t ubBitDepth;       // 8 or 16
  void *_platformData1;     // platform-specific data (Direct Draw One Interface)
  void *_platformData2;     // platform-specific data (Direct Draw Two Interface)
  void *_platformPalette;   // platform-specific data (LPDIRECTDRAWPALETTE)
  uint16_t *p16BPPPalette;  // A 16BPP palette used for 8->16 blits
  bool transparencySet;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manager Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

BOOLEAN ColorFillVSurfaceArea(struct VSurface *dest, int32_t iDestX1, int32_t iDestY1,
                              int32_t iDestX2, int32_t iDestY2, uint16_t Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(struct VSurface *dest, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, int16_t Ox, int16_t Oy);

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
BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct JPaletteEntry *pPalette);

// Returns a flat pointer for direct manipulation of data
uint8_t *LockVSurface(struct VSurface *hVSurface, uint32_t *pPitch);

// Must be called after Locking buffer call above
void UnlockVSurface(struct VSurface *hVSurface);

// Set data from HIMAGE.
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, HIMAGE hImage, uint16_t usX,
                                      uint16_t usY, SGPRect *pSrcRect);

// Sets Transparency color into HVSurface and the underlying DD surface
void SetVideoSurfaceTransparencyColor(struct VSurface *vs, COLORVAL TransColor);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct JPaletteEntry *pSrcPalette);

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVSurface(struct VSurface *hVSurface);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN BltVSurfaceRectToPoint(struct VSurface *dest, struct VSurface *src, int32_t iDestX,
                               int32_t iDestY, struct Rect *SrcRect);

BOOLEAN BltVSurfaceToVSurface(struct VSurface *dest, struct VSurface *src, int32_t destX,
                              int32_t destY);
BOOLEAN BltVSurfaceToVSurfaceSubrect(struct VSurface *dest, struct VSurface *src, int32_t destX,
                                     int32_t destY, struct Rect *srcRect);

BOOLEAN BltVSurfaceToVSurfaceFast(struct VSurface *dest, struct VSurface *src, int32_t destX,
                                  int32_t destY);

BOOLEAN ShadowVideoSurfaceRect(struct VSurface *dest, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2);
BOOLEAN ShadowVideoSurfaceImage(struct VSurface *dest, struct VObject *hImageHandle, int32_t iPosX,
                                int32_t iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVSurface(struct VSurface *dest, struct VSurface *src, SGPRect *SrcRect,
                           SGPRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(struct VSurface *dest, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2);

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

struct JRect r2jr(const struct Rect *r);
struct JRect sgpr2jr(const SGPRect *r);

#endif
