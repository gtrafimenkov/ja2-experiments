#include "jplatform_video.h"

#include "SGP/VSurface.h"

#define INITGUID
#include <ddraw.h>
#include <windows.h>

void JSurface_BlitRectToPoint(struct VSurface *src, struct VSurface *dst,
                              struct JRect const *srcBox, int32_t destX, int32_t destY) {
  // using IDirectDrawSurface2_BltFast since scaling is not necessary

  uint32_t flags = src->transparencySet ? DDBLTFAST_SRCCOLORKEY : DDBLTFAST_NOCOLORKEY;

  RECT r = {.left = srcBox->x,
            .right = srcBox->x + srcBox->w,
            .top = srcBox->y,
            .bottom = srcBox->y + srcBox->h};

  HRESULT ReturnCode;
  do {
    ReturnCode =
        IDirectDrawSurface2_BltFast((LPDIRECTDRAWSURFACE2)dst->_platformData2, destX, destY,
                                    (LPDIRECTDRAWSURFACE2)src->_platformData2, &r, flags);
    if (ReturnCode == DDERR_SURFACELOST) {
      break;
    }
  } while (ReturnCode != DD_OK);
}

void JSurface_BlitRectToRect(struct VSurface *src, struct VSurface *dst, struct JRect const *srcBox,
                             struct JRect const *destBox) {
  if (destBox->w <= 0 || destBox->h <= 0 || srcBox->w <= 0 || srcBox->h <= 0) {
    return;
  }

  if (destBox->w == srcBox->w && destBox->h == srcBox->h) {
    JSurface_BlitRectToPoint(src, dst, srcBox, destBox->x, destBox->y);
  } else {
    // Using IDirectDrawSurface2_Blt since we need scaling.

    RECT _srcRect = {.left = srcBox->x,
                     .right = srcBox->x + srcBox->w,
                     .top = srcBox->y,
                     .bottom = srcBox->y + srcBox->h};
    RECT _destRect = {.left = destBox->x,
                      .right = destBox->x + destBox->w,
                      .top = destBox->y,
                      .bottom = destBox->y + destBox->h};

    uint32_t flags = (src->transparencySet ? DDBLT_KEYSRC : 0) | DDBLT_WAIT;

    HRESULT ReturnCode;
    do {
      ReturnCode = IDirectDrawSurface2_Blt((LPDIRECTDRAWSURFACE2)dst->_platformData2, &_destRect,
                                           (LPDIRECTDRAWSURFACE2)src->_platformData2, &_srcRect,
                                           flags, NULL);
    } while (ReturnCode == DDERR_WASSTILLDRAWING);
  }
}
