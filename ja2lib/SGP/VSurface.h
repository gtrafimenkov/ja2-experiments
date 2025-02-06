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

extern struct JSurface *vsMouseBuffer;
extern struct JSurface *vsMouseBufferOriginal;

extern struct JSurface *vsSaveBuffer;
extern struct JSurface *vsExtraBuffer;

extern BOOLEAN gfExtraBuffer;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manager Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager();

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager();

BOOLEAN ColorFillVSurfaceArea(struct JSurface *dest, int32_t iDestX1, int32_t iDestY1,
                              int32_t iDestX2, int32_t iDestY2, uint16_t Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(struct JSurface *dest, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, int16_t Ox, int16_t Oy);

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct JSurface *CreateVSurfaceFromFile(const char *filepath);

// Returns a flat pointer for direct manipulation of data
uint8_t *LockVSurface(struct JSurface *hVSurface, uint32_t *pPitch);

// Set data from HIMAGE.
BOOLEAN SetVideoSurfaceDataFromHImage(struct JSurface *hVSurface, HIMAGE hImage, uint16_t usX,
                                      uint16_t usY, SGPRect *pSrcRect);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN BltVSurfaceRectToPoint(struct JSurface *dest, struct JSurface *src, int32_t iDestX,
                               int32_t iDestY, struct Rect *SrcRect);

BOOLEAN BltVSurfaceToVSurface(struct JSurface *dest, struct JSurface *src, int32_t destX,
                              int32_t destY);
BOOLEAN BltVSurfaceToVSurfaceSubrect(struct JSurface *dest, struct JSurface *src, int32_t destX,
                                     int32_t destY, struct Rect *srcRect);

BOOLEAN BltVSurfaceToVSurfaceFast(struct JSurface *dest, struct JSurface *src, int32_t destX,
                                  int32_t destY);

BOOLEAN ShadowVideoSurfaceRect(struct JSurface *dest, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2);
BOOLEAN ShadowVideoSurfaceImage(struct JSurface *dest, struct VObject *hImageHandle, int32_t iPosX,
                                int32_t iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
// enlraged or shunk.
BOOLEAN BltStretchVSurface(struct JSurface *dest, struct JSurface *src, SGPRect *SrcRect,
                           SGPRect *DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(struct JSurface *dest, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2);

void VSurfaceErase(struct JSurface *vs);

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

struct JRect r2jr(const struct Rect *r);
struct JRect sgpr2jr(const SGPRect *r);

#endif
