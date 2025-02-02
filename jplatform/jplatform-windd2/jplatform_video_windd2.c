#include "SGP/VSurface.h"
#include "jplatform_video.h"

struct JVideoState {
  uint16_t redMask;
  uint16_t greenMask;
  uint16_t blueMask;
  int16_t redShift;
  int16_t blueShift;
  int16_t greenShift;
  uint32_t translucentMask;
};

static struct JVideoState s_state;

void JVideo_GetRGBDistributionMasks(uint32_t *red, uint32_t *green, uint32_t *blue) {
  *red = s_state.redMask;
  *green = s_state.greenMask;
  *blue = s_state.blueMask;
}

uint32_t JVideo_GetTranslucentMask() { return s_state.translucentMask; }

uint16_t JVideo_PackRGB16(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t r16, g16, b16;

  if (s_state.redShift < 0)
    r16 = ((uint16_t)r >> abs(s_state.redShift));
  else
    r16 = ((uint16_t)r << s_state.redShift);

  if (s_state.greenShift < 0)
    g16 = ((uint16_t)g >> abs(s_state.greenShift));
  else
    g16 = ((uint16_t)g << s_state.greenShift);

  if (s_state.blueShift < 0)
    b16 = ((uint16_t)b >> abs(s_state.blueShift));
  else
    b16 = ((uint16_t)b << s_state.blueShift);

  return (r16 & s_state.redMask) | (g16 & s_state.greenMask) | (b16 & s_state.blueMask);
}

void JVideo_UnpackRGB16(uint16_t rgb16, uint8_t *r, uint8_t *g, uint8_t *b) {
  uint16_t r16 = rgb16 & s_state.redMask;
  uint16_t g16 = rgb16 & s_state.greenMask;
  uint16_t b16 = rgb16 & s_state.blueMask;

  if (s_state.redShift < 0)
    *r = ((uint32_t)r16 << abs(s_state.redShift));
  else
    *r = ((uint32_t)r16 >> s_state.redShift);

  if (s_state.greenShift < 0)
    *g = ((uint32_t)g16 << abs(s_state.greenShift));
  else
    *g = ((uint32_t)g16 >> s_state.greenShift);

  if (s_state.blueShift < 0)
    *b = ((uint32_t)b16 << abs(s_state.blueShift));
  else
    *b = ((uint32_t)b16 >> s_state.blueShift);
}

/////////////////////////////////////////////////////////////////////////////
// below stuff that uses windows headers
/////////////////////////////////////////////////////////////////////////////

#define INITGUID
#include <ddraw.h>
#include <windows.h>

// Difference between IDirectDrawSurface2_BltFast and IDirectDrawSurface2_Blt:
//
// IDirectDrawSurface2::BltFast
//     Purpose: Optimized for fast, simple blitting operations.
//     Restrictions:
//         Cannot handle stretching or clipping.
//         Cannot apply effects like color keys (unless explicitly allowed).
//         Only supports source color keying, no destination color keying.
//     Use case: When you need the fastest possible blitting for simple copies.
//
// IDirectDrawSurface2::Blt
//     Purpose: A more powerful and flexible blitting function.
//     Capabilities:
//         Supports stretching and shrinking.
//         Supports clipping to the destination surface.
//         Allows source and destination color keying.
//         Can apply effects such as mirroring, alpha blending (if supported by hardware), and
//         rotation.
//     Use case: When you need advanced blitting features.

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

bool tmp_getRGBDistribution() {
  DDSURFACEDESC SurfaceDescription;
  uint16_t usBit;
  HRESULT ReturnCode;

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_PIXELFORMAT;
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc((LPDIRECTDRAWSURFACE2)vsPrimary->_platformData2,
                                                  &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Ok we now have the surface description, we now can get the information that we need
  //

  s_state.redMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  s_state.greenMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  s_state.blueMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  // RGB 5,5,5
  if ((s_state.redMask == 0x7c00) && (s_state.greenMask == 0x03e0) && (s_state.blueMask == 0x1f))
    s_state.translucentMask = 0x3def;
  // RGB 5,6,5
  else  // if((redMask==0xf800) && (greenMask==0x03e0) && (blueMask==0x1f))
    s_state.translucentMask = 0x7bef;

  usBit = 0x8000;
  s_state.redShift = 8;
  while (!(s_state.redMask & usBit)) {
    usBit >>= 1;
    s_state.redShift--;
  }

  usBit = 0x8000;
  s_state.greenShift = 8;
  while (!(s_state.greenMask & usBit)) {
    usBit >>= 1;
    s_state.greenShift--;
  }

  usBit = 0x8000;
  s_state.blueShift = 8;
  while (!(s_state.blueMask & usBit)) {
    usBit >>= 1;
    s_state.blueShift--;
  }

  return TRUE;
}
