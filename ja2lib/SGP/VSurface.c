// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "VSurface.h"

#include "Rect.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/WCheck.h"
#include "StrUtils.h"

struct VSurface *ghPrimary = NULL;
struct VSurface *ghBackBuffer = NULL;
struct VSurface *vsFB = NULL;
struct VSurface *ghMouseBuffer = NULL;

bool AddVSurfaceFromFile(const char *filepath, VSurfID *index) {
  return AddVSurface(CreateVSurfaceFromFile(filepath), index);
}

typedef struct VSURFACE_NODE {
  struct VSurface *hVSurface;
  uint32_t uiIndex;
  struct VSURFACE_NODE *next, *prev;
} VSURFACE_NODE;

VSURFACE_NODE *gpVSurfaceHead = NULL;
VSURFACE_NODE *gpVSurfaceTail = NULL;
uint32_t guiVSurfaceIndex = 0;
uint32_t guiVSurfaceSize = 0;
uint32_t guiVSurfaceTotalAdded = 0;

void InitVSurfaceList() {
  Assert(!gpVSurfaceHead);
  Assert(!gpVSurfaceTail);
  gpVSurfaceHead = gpVSurfaceTail = NULL;
}

void DeinitVSurfaceList() {
  while (gpVSurfaceHead) {
    VSURFACE_NODE *curr = gpVSurfaceHead;
    gpVSurfaceHead = gpVSurfaceHead->next;
    DeleteVideoSurface(curr->hVSurface);
    MemFree(curr);
  }
  gpVSurfaceHead = NULL;
  gpVSurfaceTail = NULL;
  guiVSurfaceIndex = 0;
  guiVSurfaceSize = 0;
  guiVSurfaceTotalAdded = 0;
}

VSurfID AddVSurfaceToList(struct VSurface *vs) {
  // Set into video object list
  if (gpVSurfaceHead) {
    // Add node after tail
    gpVSurfaceTail->next = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceTail->next);  // out of memory?
    gpVSurfaceTail->next->prev = gpVSurfaceTail;
    gpVSurfaceTail->next->next = NULL;
    gpVSurfaceTail = gpVSurfaceTail->next;
  } else {
    // new list
    gpVSurfaceHead = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceHead);  // out of memory?
    gpVSurfaceHead->prev = gpVSurfaceHead->next = NULL;
    gpVSurfaceTail = gpVSurfaceHead;
  }
  // Set the hVSurface into the node.
  gpVSurfaceTail->hVSurface = vs;
  gpVSurfaceTail->uiIndex = guiVSurfaceIndex += 2;
  Assert(guiVSurfaceIndex < 0xfffffff0);  // unlikely that we will ever use 2 billion VSurfaces!
  // We would have to create about 70 VSurfaces per second for 1 year straight to achieve this...
  guiVSurfaceSize++;
  guiVSurfaceTotalAdded++;
  return gpVSurfaceTail->uiIndex;
}

struct VSurface *FindVSurface(VSurfID id) {
  VSURFACE_NODE *curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == id) {
      return curr->hVSurface;
    }
    curr = curr->next;
  }
  return NULL;
}

bool DeleteVSurfaceByIndex(VSurfID id) {
  VSURFACE_NODE *curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == id) {  // Found the node, so detach it and delete it.

      // Deallocate the memory for the video surface
      DeleteVideoSurface(curr->hVSurface);

      if (curr ==
          gpVSurfaceHead) {  // Advance the head, because we are going to remove the head node.
        gpVSurfaceHead = gpVSurfaceHead->next;
      }
      if (curr ==
          gpVSurfaceTail) {  // Back up the tail, because we are going to remove the tail node.
        gpVSurfaceTail = gpVSurfaceTail->prev;
      }
      // Detach the node from the vsurface list
      if (curr->next) {  // Make the prev node point to the next
        curr->next->prev = curr->prev;
      }
      if (curr->prev) {  // Make the next node point to the prev
        curr->prev->next = curr->next;
      }
      // The node is now detached.  Now deallocate it.
      MemFree(curr);
      guiVSurfaceSize--;
      return true;
    }
    curr = curr->next;
  }
  return false;
}

BOOLEAN AddVSurface(struct VSurface *vs, uint32_t *puiIndex) {
  if (!vs) {
    return FALSE;
  }

  // Set transparency to default
  SetVideoSurfaceTransparencyColor(vs, FROMRGB(0, 0, 0));

  *puiIndex = AddVSurfaceToList(vs);

  return TRUE;
}

struct VSurface *CreateVSurfaceBlank8(uint16_t width, uint16_t height) {
  return CreateVSurfaceBlank(width, height, 8);
}

struct VSurface *CreateVSurfaceBlank16(uint16_t width, uint16_t height) {
  return CreateVSurfaceBlank(width, height, 16);
}

// Given an HIMAGE object, blit imagery into existing Video Surface. Can be from 8->16 BPP
BOOLEAN SetVideoSurfaceDataFromHImage(struct VSurface *hVSurface, HIMAGE hImage, uint16_t usX,
                                      uint16_t usY, SGPRect *pSrcRect) {
  uint8_t *pDest;
  uint32_t fBufferBPP = 0;
  uint32_t uiPitch;
  uint16_t usEffectiveWidth;
  SGPRect aRect;

  // Assertions
  Assert(hVSurface != NULL);
  Assert(hImage != NULL);

  // Get Size of hImage and determine if it can fit
  CHECKF(hImage->usWidth >= hVSurface->usWidth);
  CHECKF(hImage->usHeight >= hVSurface->usHeight);

  // Check BPP and see if they are the same
  if (hImage->ubBitDepth != hVSurface->ubBitDepth) {
    // They are not the same, but we can go from 8->16 without much cost
    if (hImage->ubBitDepth == 8 && hVSurface->ubBitDepth == 16) {
      fBufferBPP = BUFFER_16BPP;
    }
  } else {
    // Set buffer BPP
    switch (hImage->ubBitDepth) {
      case 8:

        fBufferBPP = BUFFER_8BPP;
        break;

      case 16:

        fBufferBPP = BUFFER_16BPP;
        break;
    }
  }

  Assert(fBufferBPP != 0);

  // Get surface buffer data
  pDest = LockVSurface(hVSurface, &uiPitch);

  // Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
  usEffectiveWidth = (uint16_t)(uiPitch / (hVSurface->ubBitDepth / 8));

  CHECKF(pDest != NULL);

  // Blit Surface
  // If rect is NULL, use entrie image size
  if (pSrcRect == NULL) {
    aRect.iLeft = 0;
    aRect.iTop = 0;
    aRect.iRight = hImage->usWidth;
    aRect.iBottom = hImage->usHeight;
  } else {
    aRect.iLeft = pSrcRect->iLeft;
    aRect.iTop = pSrcRect->iTop;
    aRect.iRight = pSrcRect->iRight;
    aRect.iBottom = pSrcRect->iBottom;
  }

  // This HIMAGE function will transparently copy buffer
  if (!CopyImageToBuffer(hImage, fBufferBPP, pDest, usEffectiveWidth, hVSurface->usHeight, usX, usY,
                         &aRect)) {
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
               String("Error Occured Copying HIMAGE to struct VSurface*"));
    UnLockVideoSurfaceBuffer(hVSurface);
    return (FALSE);
  }

  // All is OK
  UnLockVideoSurfaceBuffer(hVSurface);

  return (TRUE);
}

static BOOLEAN InternalShadowVideoSurfaceRect(uint32_t uiDestVSurface, int32_t X1, int32_t Y1,
                                              int32_t X2, int32_t Y2,
                                              BOOLEAN fLowPercentShadeTable) {
  uint16_t *pBuffer;
  uint32_t uiPitch;
  SGPRect area;
  struct VSurface *hVSurface;

  CHECKF(GetVSurfaceByIndexOld(&hVSurface, uiDestVSurface));

  if (X1 < 0) X1 = 0;

  if (X2 < 0) return (FALSE);

  if (Y2 < 0) return (FALSE);

  if (Y1 < 0) Y1 = 0;

  if (X2 >= hVSurface->usWidth) X2 = hVSurface->usWidth - 1;

  if (Y2 >= hVSurface->usHeight) Y2 = hVSurface->usHeight - 1;

  if (X1 >= hVSurface->usWidth) return (FALSE);

  if (Y1 >= hVSurface->usHeight) return (FALSE);

  if ((X2 - X1) <= 0) return (FALSE);

  if ((Y2 - Y1) <= 0) return (FALSE);

  area.iTop = Y1;
  area.iBottom = Y2;
  area.iLeft = X1;
  area.iRight = X2;

  // Lock video surface
  pBuffer = (uint16_t *)LockVideoSurface(uiDestVSurface, &uiPitch);
  // UnLockVideoSurface( uiDestVSurface );

  if (pBuffer == NULL) {
    return (FALSE);
  }

  if (!fLowPercentShadeTable) {
    // Now we have the video object and surface, call the shadow function
    if (!Blt16BPPBufferShadowRect(pBuffer, uiPitch, &area)) {
      // Blit has failed if false returned
      return (FALSE);
    }
  } else {
    // Now we have the video object and surface, call the shadow function
    if (!Blt16BPPBufferShadowRectAlternateTable(pBuffer, uiPitch, &area)) {
      // Blit has failed if false returned
      return (FALSE);
    }
  }

  // Mark as dirty if it's the backbuffer
  // if ( uiDestVSurface == BACKBUFFER )
  //{
  //	InvalidateBackbuffer( );
  //}

  UnLockVideoSurface(uiDestVSurface);
  return (TRUE);
}

BOOLEAN ShadowVideoSurfaceRect(uint32_t uiDestVSurface, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2) {
  return (InternalShadowVideoSurfaceRect(uiDestVSurface, X1, Y1, X2, Y2, FALSE));
}

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(uint32_t uiDestVSurface, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2) {
  return (InternalShadowVideoSurfaceRect(uiDestVSurface, X1, Y1, X2, Y2, TRUE));
}

//
// This function will stretch the source image to the size of the dest rect.
//
// If the 2 images are not 16 Bpp, it returns false.
//
BOOLEAN BltStretchVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, int32_t iDestX,
                               int32_t iDestY, uint32_t fBltFlags, SGPRect *SrcRect,
                               SGPRect *DestRect) {
  struct VSurface *hDestVSurface;
  struct VSurface *hSrcVSurface;

  if (!GetVSurfaceByIndexOld(&hDestVSurface, uiDestVSurface)) {
    return FALSE;
  }
  if (!GetVSurfaceByIndexOld(&hSrcVSurface, uiSrcVSurface)) {
    return FALSE;
  }

  // if the 2 images are not both 16bpp, return FALSE
  if ((hDestVSurface->ubBitDepth != 16) && (hSrcVSurface->ubBitDepth != 16)) return (FALSE);

  struct Rect srcRect = {SrcRect->iLeft, SrcRect->iTop, SrcRect->iRight, SrcRect->iBottom};
  struct Rect destRect = {DestRect->iLeft, DestRect->iTop, DestRect->iRight, DestRect->iBottom};
  return BltVSurface(hDestVSurface, hSrcVSurface, fBltFlags, iDestX, iDestY, &srcRect, &destRect);
}

struct VSurface *GetVSurfaceByID(VSurfID id) {
  switch (id) {
    case PRIMARY_SURFACE:
      return ghPrimary;
    case BACKBUFFER:
      return ghBackBuffer;
    case vsIndexFB:
      return vsFB;
    case MOUSE_BUFFER:
      return ghMouseBuffer;
    default:
      return FindVSurface(id);
  }
}

BOOLEAN GetVSurfaceByIndexOld(struct VSurface **pvs, VSurfID id) {
  *pvs = GetVSurfaceByID(id);
  return *pvs != NULL;
}
