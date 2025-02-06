// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "VSurface.h"

#include <stdio.h>
#include <stdlib.h>

#include "Rect.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "StrUtils.h"
#include "jplatform_video.h"
#include "platform_strings.h"

struct VSurface *vsPrimary = NULL;
struct VSurface *vsBackBuffer = NULL;
struct VSurface *vsFB = NULL;
struct VSurface *vsMouseBuffer = NULL;
struct VSurface *vsMouseBufferOriginal = NULL;

struct JRect r2jr(const struct Rect *r) {
  struct JRect box = {.x = r->left, .y = r->top, .w = r->right - r->left, .h = r->bottom - r->top};
  return box;
}

struct JRect sgpr2jr(const SGPRect *r) {
  struct JRect box = {
      .x = r->iLeft, .y = r->iTop, .w = r->iRight - r->iLeft, .h = r->iBottom - r->iTop};
  return box;
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
  CHECKF(hImage->usWidth >= JSurface_Width(hVSurface));
  CHECKF(hImage->usHeight >= JSurface_Height(hVSurface));

  // Check BPP and see if they are the same
  if (hImage->ubBitDepth != JSurface_BPP(hVSurface)) {
    // They are not the same, but we can go from 8->16 without much cost
    if (hImage->ubBitDepth == 8 && JSurface_BPP(hVSurface) == 16) {
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
  usEffectiveWidth = (uint16_t)(uiPitch / (JSurface_BPP(hVSurface) / 8));

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
  if (!CopyImageToBuffer(hImage, fBufferBPP, pDest, usEffectiveWidth, JSurface_Height(hVSurface),
                         usX, usY, &aRect)) {
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
               String("Error Occured Copying HIMAGE to struct VSurface*"));
    UnlockVSurface(hVSurface);
    return (FALSE);
  }

  // All is OK
  UnlockVSurface(hVSurface);

  return (TRUE);
}

static BOOLEAN InternalShadowVideoSurfaceRect(struct VSurface *dest, int32_t X1, int32_t Y1,
                                              int32_t X2, int32_t Y2,
                                              BOOLEAN fLowPercentShadeTable) {
  if (dest == NULL) {
    return FALSE;
  }

  uint16_t *pBuffer;
  uint32_t uiPitch;
  SGPRect area;

  if (X1 < 0) X1 = 0;

  if (X2 < 0) return (FALSE);

  if (Y2 < 0) return (FALSE);

  if (Y1 < 0) Y1 = 0;

  if (X2 >= JSurface_Width(dest)) X2 = JSurface_Width(dest) - 1;

  if (Y2 >= JSurface_Height(dest)) Y2 = JSurface_Height(dest) - 1;

  if (X1 >= JSurface_Width(dest)) return (FALSE);

  if (Y1 >= JSurface_Height(dest)) return (FALSE);

  if ((X2 - X1) <= 0) return (FALSE);

  if ((Y2 - Y1) <= 0) return (FALSE);

  area.iTop = Y1;
  area.iBottom = Y2;
  area.iLeft = X1;
  area.iRight = X2;

  // Lock video surface
  pBuffer = (uint16_t *)LockVSurface(dest, &uiPitch);

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

  UnlockVSurface(dest);
  return (TRUE);
}

BOOLEAN ShadowVideoSurfaceRect(struct VSurface *dest, int32_t X1, int32_t Y1, int32_t X2,
                               int32_t Y2) {
  return (InternalShadowVideoSurfaceRect(dest, X1, Y1, X2, Y2, FALSE));
}

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(struct VSurface *dest, int32_t X1, int32_t Y1,
                                                   int32_t X2, int32_t Y2) {
  return (InternalShadowVideoSurfaceRect(dest, X1, Y1, X2, Y2, TRUE));
}

//
// This function will stretch the source image to the size of the dest rect.
// If the 2 images are not 16 Bpp, it returns false.
BOOLEAN BltStretchVSurface(struct VSurface *dest, struct VSurface *src, SGPRect *SrcRect,
                           SGPRect *DestRect) {
  if ((JSurface_BPP(dest) != 16) && (JSurface_BPP(src) != 16)) {
    return FALSE;
  };
  struct JRect srcBox = sgpr2jr(SrcRect);
  struct JRect destBox = sgpr2jr(DestRect);
  JSurface_BlitRectToRect(src, dest, &srcBox, &destBox);
  return TRUE;
}

BOOLEAN ShadowVideoSurfaceImage(struct VSurface *dest, struct VObject *hImageHandle, int32_t iPosX,
                                int32_t iPosY) {
  // Horizontal shadow
  ShadowVideoSurfaceRect(dest, iPosX + 3, iPosY + hImageHandle->pETRLEObject->usHeight,
                         iPosX + hImageHandle->pETRLEObject->usWidth,
                         iPosY + hImageHandle->pETRLEObject->usHeight + 3);

  // vertical shadow
  ShadowVideoSurfaceRect(dest, iPosX + hImageHandle->pETRLEObject->usWidth, iPosY + 3,
                         iPosX + hImageHandle->pETRLEObject->usWidth + 3,
                         iPosY + hImageHandle->pETRLEObject->usHeight);
  return (TRUE);
}

// Fills an rectangular area with a specified image value.
BOOLEAN ImageFillVideoSurfaceArea(struct VSurface *dest, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, int16_t Ox, int16_t Oy) {
  int16_t xc, yc, hblits, wblits, aw, pw, ah, ph, w, h, xo, yo;
  ETRLEObject *pTrav;
  SGPRect NewClip, OldClip;

  pTrav = &(BkgrndImg->pETRLEObject[Index]);
  ph = (int16_t)(pTrav->usHeight + pTrav->sOffsetY);
  pw = (int16_t)(pTrav->usWidth + pTrav->sOffsetX);

  ah = (int16_t)(iDestY2 - iDestY1);
  aw = (int16_t)(iDestX2 - iDestX1);

  Ox %= pw;
  Oy %= ph;

  if (Ox > 0) Ox -= pw;
  xo = (-Ox) % pw;

  if (Oy > 0) Oy -= ph;
  yo = (-Oy) % ph;

  if (Ox < 0)
    xo = (-Ox) % pw;
  else {
    xo = pw - (Ox % pw);
    Ox -= pw;
  }

  if (Oy < 0)
    yo = (-Oy) % ph;
  else {
    yo = ph - (Oy % pw);
    Oy -= ph;
  }

  hblits = ((ah + yo) / ph) + (((ah + yo) % ph) ? 1 : 0);
  wblits = ((aw + xo) / pw) + (((aw + xo) % pw) ? 1 : 0);

  if ((hblits == 0) || (wblits == 0)) return (FALSE);

  //
  // Clip fill region coords
  //

  GetClippingRect(&OldClip);

  NewClip.iLeft = iDestX1;
  NewClip.iTop = iDestY1;
  NewClip.iRight = iDestX2;
  NewClip.iBottom = iDestY2;

  if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

  if (NewClip.iLeft > OldClip.iRight) return (FALSE);

  if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

  if (NewClip.iRight < OldClip.iLeft) return (FALSE);

  if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

  if (NewClip.iTop > OldClip.iBottom) return (FALSE);

  if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

  if (NewClip.iBottom < OldClip.iTop) return (FALSE);

  if ((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop)) return (FALSE);

  SetClippingRect(&NewClip);

  yc = (int16_t)iDestY1;
  for (h = 0; h < hblits; h++) {
    xc = (int16_t)iDestX1;
    for (w = 0; w < wblits; w++) {
      BltVideoObject(dest, BkgrndImg, Index, xc + Ox, yc + Oy);
      xc += pw;
    }
    yc += ph;
  }

  SetClippingRect(&OldClip);
  return (TRUE);
}

// Update srcRect and destX, destY so that the resulting target
// rectangle is inside of dest bounds.
static BOOLEAN clipToDestBounds(struct VSurface *dest, struct VSurface *src, int32_t *destX,
                                int32_t *destY, struct Rect *srcRect) {
  Assert(dest != NULL);
  Assert(src != NULL);

  if (JSurface_Height(dest) < JSurface_Height(src)) {
    return (FALSE);
  }
  if (JSurface_Width(dest) < JSurface_Width(src)) {
    return (FALSE);
  }

  // clipping -- added by DB
  struct Rect DestRect = {
      .left = 0, .top = 0, .right = JSurface_Width(dest), .bottom = JSurface_Height(dest)};
  int32_t srcWidth = srcRect->right - srcRect->left;
  int32_t srcHeight = srcRect->bottom - srcRect->top;

  // check for position entirely off the screen
  if (*destX >= JSurface_Width(dest)) return false;
  if (*destY >= JSurface_Height(dest)) return false;
  if ((*destX + srcWidth) < 0) return false;
  if ((*destY + srcHeight) < 0) return false;

  if ((*destX + srcWidth) >= JSurface_Width(dest)) {
    srcRect->right -= ((*destX + srcWidth) - JSurface_Width(dest));
  }
  if ((*destY + srcHeight) >= JSurface_Height(dest)) {
    srcRect->bottom -= ((*destY + srcHeight) - JSurface_Height(dest));
  }
  if (*destX < 0) {
    srcRect->left += -*destX;
    *destX = 0;
  }
  if (*destY < 0) {
    srcRect->top += -*destY;
    *destY = 0;
  }
  return true;
}

static BOOLEAN BltVSurfaceToVSurfaceSubrectInternal_8_8(struct VSurface *dest, struct VSurface *src,
                                                        int32_t destX, int32_t destY,
                                                        struct Rect *srcRect) {
  if (JSurface_BPP(dest) == 8 && JSurface_BPP(src) == 8) {
    if (!clipToDestBounds(dest, src, &destX, &destY, srcRect)) {
      return FALSE;
    }

    uint8_t *pSrcSurface8, *pDestSurface8;
    uint32_t uiSrcPitch, uiDestPitch;
    if ((pSrcSurface8 = (uint8_t *)LockVSurface(src, &uiSrcPitch)) == NULL) {
      return FALSE;
    }

    if ((pDestSurface8 = (uint8_t *)LockVSurface(dest, &uiDestPitch)) == NULL) {
      UnlockVSurface(src);
      return FALSE;
    }

    Blt8BPPTo8BPP(pDestSurface8, uiDestPitch, pSrcSurface8, uiSrcPitch, destX, destY, srcRect);

    UnlockVSurface(src);
    UnlockVSurface(dest);
    return TRUE;
  }
  return FALSE;
}

static BOOLEAN ClipReleatedSrcAndDestRectangles(struct VSurface *dest, struct VSurface *src,
                                                struct Rect *DestRect, struct Rect *SrcRect) {
  Assert(dest != NULL);
  Assert(src != NULL);

  // Check for invalid start positions and clip by ignoring blit
  if (DestRect->left >= JSurface_Width(dest) || DestRect->top >= JSurface_Height(dest)) {
    return (FALSE);
  }

  if (SrcRect->left >= JSurface_Width(src) || SrcRect->top >= JSurface_Height(src)) {
    return (FALSE);
  }

  // For overruns
  // Clip destination rectangles
  if (DestRect->right > JSurface_Width(dest)) {
    // Both have to be modified or by default streching occurs
    DestRect->right = JSurface_Width(dest);
    SrcRect->right = SrcRect->left + (DestRect->right - DestRect->left);
  }
  if (DestRect->bottom > JSurface_Height(dest)) {
    // Both have to be modified or by default streching occurs
    DestRect->bottom = JSurface_Height(dest);
    SrcRect->bottom = SrcRect->top + (DestRect->bottom - DestRect->top);
  }

  // Clip src rectangles
  if (SrcRect->right > JSurface_Width(src)) {
    // Both have to be modified or by default streching occurs
    SrcRect->right = JSurface_Width(src);
    DestRect->right = DestRect->left + (SrcRect->right - SrcRect->left);
  }
  if (SrcRect->bottom > JSurface_Height(src)) {
    // Both have to be modified or by default streching occurs
    SrcRect->bottom = JSurface_Height(src);
    DestRect->bottom = DestRect->top + (SrcRect->bottom - SrcRect->top);
  }

  // For underruns
  // Clip destination rectangles
  if (DestRect->left < 0) {
    // Both have to be modified or by default streching occurs
    DestRect->left = 0;
    SrcRect->left = SrcRect->right - (DestRect->right - DestRect->left);
  }
  if (DestRect->top < 0) {
    // Both have to be modified or by default streching occurs
    DestRect->top = 0;
    SrcRect->top = SrcRect->bottom - (DestRect->bottom - DestRect->top);
  }

  // Clip src rectangles
  if (SrcRect->left < 0) {
    // Both have to be modified or by default streching occurs
    SrcRect->left = 0;
    DestRect->left = DestRect->right - (SrcRect->right - SrcRect->left);
  }
  if (SrcRect->top < 0) {
    // Both have to be modified or by default streching occurs
    SrcRect->top = 0;
    DestRect->top = DestRect->bottom - (SrcRect->bottom - SrcRect->top);
  }

  return (TRUE);
}

BOOLEAN BltVSurfaceRectToPoint(struct VSurface *dest, struct VSurface *src, int32_t iDestX,
                               int32_t iDestY, struct Rect *SrcRect) {
  // Setup dest rectangle
  struct Rect DestRect;
  DestRect.top = (int)iDestY;
  DestRect.left = (int)iDestX;
  DestRect.bottom = (int)iDestY + (SrcRect->bottom - SrcRect->top);
  DestRect.right = (int)iDestX + (SrcRect->right - SrcRect->left);

  struct Rect SrcRectCopy = *SrcRect;

  // Do Clipping of rectangles
  if (!ClipReleatedSrcAndDestRectangles(dest, src, &DestRect, &SrcRectCopy)) {
    // Returns false because dest start is > dest size
    return (TRUE);
  }
  struct JRect srcBox = r2jr(&SrcRectCopy);
  struct JRect destBox = r2jr(&DestRect);
  JSurface_BlitRectToRect(src, dest, &srcBox, &destBox);
  return (TRUE);
}

BOOLEAN BltVSurfaceToVSurfaceSubrect(struct VSurface *dest, struct VSurface *src, int32_t destX,
                                     int32_t destY, struct Rect *srcRect) {
  if (JSurface_BPP(dest) == 16 && JSurface_BPP(src) == 16) {
    if (clipToDestBounds(dest, src, &destX, &destY, srcRect)) {
      return BltVSurfaceRectToPoint(dest, src, destX, destY, srcRect);
    }
  } else if (JSurface_BPP(dest) == 8 && JSurface_BPP(src) == 8) {
    return BltVSurfaceToVSurfaceSubrectInternal_8_8(dest, src, destX, destY, srcRect);
  }
  return FALSE;
}

BOOLEAN BltVSurfaceToVSurfaceFast(struct VSurface *dest, struct VSurface *src, int32_t destX,
                                  int32_t destY) {
  if (JSurface_BPP(dest) == 16 && JSurface_BPP(src) == 16) {
    CHECKF(destX >= 0);
    CHECKF(destY >= 0);
    struct JRect srcBox = {.x = 0, .y = 0, .w = JSurface_Width(src), .h = JSurface_Height(src)};
    JSurface_BlitRectToPoint(src, dest, &srcBox, destX, destY);
    return TRUE;
  } else if (JSurface_BPP(dest) == 8 && JSurface_BPP(src) == 8) {
    struct Rect SrcRect = {
        .top = 0, .left = 0, .bottom = JSurface_Height(src), .right = JSurface_Width(src)};
    return BltVSurfaceToVSurfaceSubrectInternal_8_8(dest, src, destX, destY, &SrcRect);
  }
  return FALSE;
}

BOOLEAN BltVSurfaceToVSurface(struct VSurface *dest, struct VSurface *src, int32_t destX,
                              int32_t destY) {
  struct Rect SrcRect = {
      .top = 0, .left = 0, .bottom = JSurface_Height(src), .right = JSurface_Width(src)};
  if (JSurface_BPP(dest) == 16 && JSurface_BPP(src) == 16) {
    return BltVSurfaceRectToPoint(dest, src, destX, destY, &SrcRect);
  } else if (JSurface_BPP(dest) == 8 && JSurface_BPP(src) == 8) {
    return BltVSurfaceToVSurfaceSubrectInternal_8_8(dest, src, destX, destY, &SrcRect);
  }
  return FALSE;
}

void VSurfaceErase(struct VSurface *vs) {
  uint32_t uiPitch;
  void *pTmpPointer = LockVSurface(vs, &uiPitch);
  if (pTmpPointer) {
    memset(pTmpPointer, 0, JSurface_Height(vs) * uiPitch);
    UnlockVSurface(vs);
  }
}

extern void SetClippingRect(SGPRect *clip);
extern void GetClippingRect(SGPRect *clip);

#define DEFAULT_NUM_REGIONS 5
#define DEFAULT_VIDEO_SURFACE_LIST_SIZE 10

BOOLEAN InitializeVideoSurfaceManager() {
  RegisterDebugTopic(TOPIC_VIDEOSURFACE, "Video Surface Manager");
  return TRUE;
}

BOOLEAN ShutdownVideoSurfaceManager() {
  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Shutting down the Video Surface manager");
  UnRegisterDebugTopic(TOPIC_VIDEOSURFACE, "Video Objects");
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Fills an rectangular area with a specified color value.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN ColorFillVSurfaceArea(struct VSurface *dest, int32_t iDestX1, int32_t iDestY1,
                              int32_t iDestX2, int32_t iDestY2, uint16_t Color16BPP) {
  SGPRect Clip;
  GetClippingRect(&Clip);

  if (iDestX1 < Clip.iLeft) iDestX1 = Clip.iLeft;

  if (iDestX1 > Clip.iRight) return (FALSE);

  if (iDestX2 > Clip.iRight) iDestX2 = Clip.iRight;

  if (iDestX2 < Clip.iLeft) return (FALSE);

  if (iDestY1 < Clip.iTop) iDestY1 = Clip.iTop;

  if (iDestY1 > Clip.iBottom) return (FALSE);

  if (iDestY2 > Clip.iBottom) iDestY2 = Clip.iBottom;

  if (iDestY2 < Clip.iTop) return (FALSE);

  if ((iDestX2 <= iDestX1) || (iDestY2 <= iDestY1)) return (FALSE);

  struct JRect r = {.x = iDestX1, .y = iDestY1, .w = iDestX2 - iDestX1, .h = iDestY2 - iDestY1};
  JSurface_FillRect(dest, &r, Color16BPP);
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manipulation Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct VSurface *CreateVSurfaceBlank(uint16_t width, uint16_t height, uint8_t bitDepth) {
  Assert(height > 0);
  Assert(width > 0);

  switch (bitDepth) {
    case 8:
      return JSurface_Create8bpp(width, height);

    case 16:
      return JSurface_Create16bpp(width, height);

    default:
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid BPP value, can only be 8 or 16.");
      return NULL;
  }
}

struct VSurface *CreateVSurfaceFromFile(const char *filepath) {
  HIMAGE hImage = CreateImage(filepath, IMAGE_ALLIMAGEDATA);

  if (hImage == NULL) {
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid Image Filename given");
    return (NULL);
  }

  Assert(hImage->usHeight > 0);
  Assert(hImage->usWidth > 0);

  struct VSurface *vs = CreateVSurfaceBlank(hImage->usWidth, hImage->usHeight, hImage->ubBitDepth);
  if (!vs) {
    DestroyImage(hImage);
    return NULL;
  }

  SGPRect tempRect;
  tempRect.iLeft = 0;
  tempRect.iTop = 0;
  tempRect.iRight = hImage->usWidth - 1;
  tempRect.iBottom = hImage->usHeight - 1;
  SetVideoSurfaceDataFromHImage(vs, hImage, 0, 0, &tempRect);

  if (hImage->ubBitDepth == 8) {
    VSurface_SetPalette32(vs, hImage->pPalette);
  }

  DestroyImage(hImage);

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Success in CreateVSurfaceFromFile"));

  return vs;
}

uint8_t *LockVSurface(struct VSurface *vs, uint32_t *pPitch) {
  if (!JSurface_Lock(vs)) {
    return NULL;
  }
  *pPitch = JSurface_Pitch(vs);
  return (uint8_t *)JSurface_GetPixels(vs);
}

void UnlockVSurface(struct VSurface *vs) { JSurface_Unlock(vs); }

// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
void VSurface_SetPalette32(struct VSurface *vs, struct JPaletteEntry *pal) {
  Assert(vs != NULL);

  JSurface_SetPalette32(vs, pal);
  JSurface_SetPalette16(vs, Create16BPPPalette(pal));

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Video Surface Palette change successfull"));
}
