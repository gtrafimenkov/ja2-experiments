// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "SGP/Container.h"
#include "SGP/HImage.h"
#include "SGP/Types.h"

struct JSurface;

// ************************************************************************************
//
// Video Object SGP Module
//
// ************************************************************************************

// Defines for struct VObject* limits
#define HVOBJECT_SHADE_TABLES 48

#define HVOBJECT_GLOW_GREEN 0
#define HVOBJECT_GLOW_BLUE 1
#define HVOBJECT_GLOW_YELLOW 2
#define HVOBJECT_GLOW_RED 3

// Effects structure for specialized blitting
typedef struct {
  uint32_t uiShadowLevel;
  SGPRect ClipRect;

} blt_fx;

// Z-buffer info structure for properly assigning Z values
typedef struct {
  int8_t bInitialZChange;      // difference in Z value between the leftmost and base strips
  uint8_t ubFirstZStripWidth;  // # of pixels in the leftmost strip
  uint8_t ubNumberOfZChanges;  // number of strips (after the first)
  int8_t *pbZChange;           // change to the Z value in each strip (after the first)
} ZStripInfo;

typedef struct {
  uint16_t *p16BPPData;
  uint16_t usRegionIndex;
  uint8_t ubShadeLevel;
  uint16_t usWidth;
  uint16_t usHeight;
  int16_t sOffsetX;
  int16_t sOffsetY;
} SixteenBPPObjectInfo;

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g, b) \
  ((uint32_t)(((uint8_t)(r) | ((uint16_t)(g) << 8)) | (((uint32_t)(uint8_t)(b)) << 16)))

// VOBJECT FLAGS
#define VOBJECT_FLAG_SHADETABLE_SHARED 0x00000100

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
struct VObject {
  uint32_t fFlags;                      // Special flags
  uint32_t uiSizePixData;               // ETRLE data size
  struct JPaletteEntry *pPaletteEntry;  // 8BPP Palette
  uint16_t *p16BPPPalette;              // A 16BPP palette used for 8->16 blits

  void *pPixData;             // ETRLE pixel data
  ETRLEObject *pETRLEObject;  // Object offset data etc
  SixteenBPPObjectInfo *p16BPPObject;
  uint16_t *pShades[HVOBJECT_SHADE_TABLES];  // Shading tables
  uint16_t *pShadeCurrent;
  uint16_t *pGlow;            // glow highlight table
  uint8_t *pShade8;           // 8-bit shading index table
  uint8_t *pGlow8;            // 8-bit glow table
  ZStripInfo **ppZStripInfo;  // Z-value strip info arrays

  uint16_t usNumberOf16BPPObjects;
  uint16_t usNumberOfObjects;  // Total number of objects
  uint8_t ubBitDepth;          // BPP

  // Reserved for added room and 32-byte boundaries
  uint8_t bReserved[1];
};

// **********************************************************************************
//
// Video Object Manager Functions
//
// **********************************************************************************

// Creates a list to contain video objects
BOOLEAN InitializeVideoObjectManager();

// Deletes any video object placed into list
BOOLEAN ShutdownVideoObjectManager();

// Creates and adds a video object to list
BOOLEAN AddVObject(struct VObject *vo, uint32_t *uiIndex);

// Removes a video object
BOOLEAN DeleteVideoObjectFromIndex(uint32_t uiVObject);

uint16_t CreateObjectPaletteTables(struct VObject *pObj, uint32_t uiType);

// Returns a struct VObject* for the specified index
BOOLEAN GetVideoObject(struct VObject **hVObject, uint32_t uiIndex);

BOOLEAN BltVideoObject(struct JSurface *dest, struct VObject *voSrc, uint16_t usRegionIndex,
                       int32_t iDestX, int32_t iDestY);

// Blits a video object to another video object
BOOLEAN BltVObjectFromIndex(struct JSurface *dest, uint32_t uiSrcVObject, uint16_t usRegionIndex,
                            int32_t iDestX, int32_t iDestY);

// **********************************************************************************
//
// Video Object manipulation functions
//
// **********************************************************************************

struct VObject *CreateVObjectFromFile(const char *filename);
struct VObject *CreateVObjectFromMLGFile(uint16_t usMLGGraphicID);
struct VObject *CreateVObjectFromHImage(HIMAGE hImage);

// Sets struct VObject* palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoObjectPalette(struct VObject *hVObject, struct JPaletteEntry *pSrcPalette);

// Deletes all data
BOOLEAN DeleteVideoObject(struct VObject *hVObject);

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(struct VObject *hVObject);

// Sets the current object shade table
uint16_t SetObjectShade(struct VObject *pObj, uint32_t uiShade);

// Sets the current object shade table using a vobject handle
uint16_t SetObjectHandleShade(uint32_t uiHandle, uint32_t uiShade);

// Fills a rectangular area of an object with a color
uint16_t FillObjectRect(uint32_t iObj, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                        COLORVAL color32);

// Retrieves an struct VObject* pixel value
BOOLEAN GetETRLEPixelValue(uint8_t *pDest, struct VObject *hVObject, uint16_t usETLREIndex,
                           uint16_t usX, uint16_t usY);

// ****************************************************************************
//
// Globals
//
// ****************************************************************************
extern HLIST ghVideoObjects;

// ****************************************************************************
//
// Macros
//
// ****************************************************************************

extern BOOLEAN gfVideoObjectsInit;
#define VideoObjectsInitialized() (gfVideoObjectsInit)

// ****************************************************************************
//
// Blt Functions
//
// ****************************************************************************

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

struct VObject *GetPrimaryVideoObject();
struct VObject *GetBackBufferVideoObject();

BOOLEAN GetVideoObjectETRLEProperties(struct VObject *hVObject, ETRLEObject *pETRLEObject,
                                      uint16_t usIndex);
BOOLEAN GetVideoObjectETRLEPropertiesFromIndex(uint32_t uiVideoObject, ETRLEObject *pETRLEObject,
                                               uint16_t usIndex);
BOOLEAN GetVideoObjectETRLESubregionProperties(uint32_t uiVideoObject, uint16_t usIndex,
                                               uint16_t *pusWidth, uint16_t *pusHeight);

BOOLEAN SetVideoObjectPalette8BPP(int32_t uiVideoObject, struct JPaletteEntry *pPal8);
BOOLEAN SetVideoObjectPalette16BPP(int32_t uiVideoObject, uint16_t *pPal16);
BOOLEAN GetVideoObjectPalette16BPP(int32_t uiVideoObject, uint16_t **ppPal16);
BOOLEAN CopyVideoObjectPalette16BPP(int32_t uiVideoObject, uint16_t *ppPal16);

BOOLEAN ConvertVObjectRegionTo16BPP(struct VObject *hVObject, uint16_t usRegionIndex,
                                    uint8_t ubShadeLevel);
BOOLEAN CheckFor16BPPRegion(struct VObject *hVObject, uint16_t usRegionIndex, uint8_t ubShadeLevel,
                            uint16_t *pusIndex);

BOOLEAN BltVideoObjectOutlineFromIndex(struct JSurface *dest, uint32_t uiSrcVObject,
                                       uint16_t usIndex, int32_t iDestX, int32_t iDestY,
                                       int16_t s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutline(struct JSurface *dest, struct VObject *hSrcVObject, uint16_t usIndex,
                              int32_t iDestX, int32_t iDestY, int16_t s16BPPColor,
                              BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutlineShadowFromIndex(struct JSurface *dest, uint32_t uiSrcVObject,
                                             uint16_t usIndex, int32_t iDestX, int32_t iDestY);

#endif
