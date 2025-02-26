#include "jplatform_video_windd2.h"

#include "jplatform_video.h"

#define INITGUID
#include <ddraw.h>
#include <windows.h>

struct JSurface {
  uint16_t height;   // Height of Video Surface
  uint16_t width;    // Width of Video Surface
  uint8_t bitDepth;  // 8 or 16
  LPDIRECTDRAWSURFACE dds;
  LPDIRECTDRAWSURFACE2 dds2;
  LPDIRECTDRAWPALETTE ddPalette;
  const uint16_t *palette16;  // A 16BPP palette used for 8->16 blits
  bool transparencySet;

  // Raw pixels.  Available only when the surface is locked.
  void *pixels;
  // Size of single line of pixels in bytes.  Available only when the surface is locked.
  uint32_t pitch;
};

struct JSurface *vsPrimary = NULL;
struct JSurface *vsBackBuffer = NULL;
struct JSurface *vsFB = NULL;

struct JVideoState {
  uint16_t redMask;
  uint16_t greenMask;
  uint16_t blueMask;
  int16_t redShift;
  int16_t blueShift;
  int16_t greenShift;
  uint32_t translucentMask;
  LPDIRECTDRAW _gpDirectDrawObject;
  LPDIRECTDRAW2 gpDirectDrawObject;
};

static struct JVideoState s_state;

HWND ghWindow;  // Main window frame for the application
static uint16_t gusScreenWidth;
static uint16_t gusScreenHeight;

uint16_t JVideo_GetScreenWidth() { return gusScreenWidth; }
uint16_t JVideo_GetScreenHeight() { return gusScreenHeight; }

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

void JSurface_BlitRectToPoint(struct JSurface *src, struct JSurface *dst,
                              struct JRect const *srcBox, int32_t destX, int32_t destY) {
  // using IDirectDrawSurface2_BltFast since scaling is not necessary

  uint32_t flags = src->transparencySet ? DDBLTFAST_SRCCOLORKEY : DDBLTFAST_NOCOLORKEY;

  RECT r = {.left = srcBox->x,
            .right = srcBox->x + srcBox->w,
            .top = srcBox->y,
            .bottom = srcBox->y + srcBox->h};

  HRESULT ReturnCode;
  do {
    ReturnCode = IDirectDrawSurface2_BltFast(dst->dds2, destX, destY, src->dds2, &r, flags);
    if (ReturnCode == DDERR_SURFACELOST) {
      break;
    }
  } while (ReturnCode != DD_OK);
}

void JSurface_BlitRectToRect(struct JSurface *src, struct JSurface *dst, struct JRect const *srcBox,
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
      ReturnCode =
          IDirectDrawSurface2_Blt(dst->dds2, &_destRect, src->dds2, &_srcRect, flags, NULL);
    } while (ReturnCode == DDERR_WASSTILLDRAWING);
  }
}

static bool getRGBDistribution() {
  DDSURFACEDESC SurfaceDescription;
  uint16_t usBit;
  HRESULT ReturnCode;

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_PIXELFORMAT;
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(vsPrimary->dds2, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Ok we now have the surface description, we now can get the information that we need
  //

  s_state.redMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  s_state.greenMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  s_state.blueMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  if ((s_state.redMask == 0x7c00) && (s_state.greenMask == 0x03e0) && (s_state.blueMask == 0x1f)) {
    // RGB 5,5,5
    s_state.translucentMask = 0x3def;
  } else {
    // RGB 5,6,5
    s_state.translucentMask = 0x7bef;
  }

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

static void *mallocZero(size_t size) {
  void *p = malloc(size);
  if (p != NULL) {
    memset(p, 0, size);
  }
  return p;
}

static struct JSurface *CreateVSurfaceInternal(DDSURFACEDESC *descr) {
  struct JSurface *vs = (struct JSurface *)mallocZero(sizeof(struct JSurface));
  if (vs == NULL) {
    return NULL;
  }

  LPDIRECTDRAWSURFACE lpDDS;
  LPDIRECTDRAWSURFACE2 lpDDS2;
  {
    // create the directdraw surface
    HRESULT ReturnCode =
        IDirectDraw2_CreateSurface(s_state.gpDirectDrawObject, descr, &lpDDS, NULL);
    if (ReturnCode != DD_OK) {
      return NULL;
    }

    // get the direct draw surface 2 interface
    IID tmpID = IID_IDirectDrawSurface2;
    ReturnCode = IDirectDrawSurface_QueryInterface(lpDDS, &tmpID, &lpDDS2);
    if (ReturnCode != DD_OK) {
      return NULL;
    }
  }

  vs->dds = lpDDS;
  vs->dds2 = lpDDS2;

  vs->width = (uint16_t)descr->dwWidth;
  vs->height = (uint16_t)descr->dwHeight;
  if (descr->dwFlags & DDSD_PIXELFORMAT) {
    vs->bitDepth = (uint8_t)descr->ddpfPixelFormat.dwRGBBitCount;
  } else {
    DDSURFACEDESC newDescr;
    memset(&newDescr, 0, sizeof(LPDDSURFACEDESC));
    newDescr.dwSize = sizeof(DDSURFACEDESC);
    IDirectDrawSurface2_GetSurfaceDesc(lpDDS2, &newDescr);
    vs->bitDepth = (uint8_t)newDescr.ddpfPixelFormat.dwRGBBitCount;
  }

  return vs;
}

bool JVideo_Init(char *appName, uint16_t screenWidth, uint16_t screenHeight,
                 struct JVideoInitParams *videoInitParams) {
  HINSTANCE hInstance = (HINSTANCE)videoInitParams->hInstance;
  char *ClassName = appName;

  WNDCLASS WindowClass;
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = (WNDPROC)videoInitParams->WindowProc;
  WindowClass.cbClsExtra = 0;
  WindowClass.cbWndExtra = 0;
  WindowClass.hInstance = hInstance;
  WindowClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(videoInitParams->iconID));
  WindowClass.hCursor = NULL;
  WindowClass.hbrBackground = NULL;
  WindowClass.lpszMenuName = NULL;
  WindowClass.lpszClassName = ClassName;
  RegisterClass(&WindowClass);

  ghWindow = CreateWindowEx(WS_EX_TOPMOST, ClassName, ClassName, WS_POPUP | WS_VISIBLE, 0, 0,
                            GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL,
                            NULL, hInstance, NULL);
  if (ghWindow == NULL) {
    return FALSE;
  }

  ShowCursor(FALSE);
  ShowWindow(ghWindow, videoInitParams->usCommandShow);
  UpdateWindow(ghWindow);
  SetFocus(ghWindow);

  gusScreenWidth = screenWidth;
  gusScreenHeight = screenHeight;
  HRESULT ReturnCode = DirectDrawCreate(NULL, &s_state._gpDirectDrawObject, NULL);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  IID tmpID = IID_IDirectDraw2;
  ReturnCode = IDirectDraw_QueryInterface(s_state._gpDirectDrawObject, &tmpID,
                                          (LPVOID *)&s_state.gpDirectDrawObject);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  // Set the exclusive mode
  ReturnCode = IDirectDraw2_SetCooperativeLevel(s_state.gpDirectDrawObject, ghWindow,
                                                DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  // Set the display mode
  ReturnCode =
      IDirectDraw2_SetDisplayMode(s_state.gpDirectDrawObject, screenWidth, screenHeight, 16, 0, 0);
  if (ReturnCode != DD_OK) {
    return FALSE;
  }

  //
  // Initialize Primary Surface along with BackBuffer
  //

  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
  // DDSCAPS_PRIMARYSURFACE
  //   This surface is the primary surface. It represents what is visible to the user at the
  //   moment.
  //
  // DDSCAPS_FLIP
  //   This surface is a part of a surface flipping structure. When this capability is passed to the
  //   application's CreateSurface method, a front buffer and one or more back buffers are created.
  //   DirectDraw sets the DDSCAPS_FRONTBUFFER bit on the front-buffer surface and the
  //   DDSCAPS_BACKBUFFER bit on the surface adjacent to the front-buffer surface. The
  //   dwBackBufferCount member of the DDSURFACEDESC structure must be set to at least 1 in order
  //   for the method call to succeed. The DDSCAPS_COMPLEX capability must always be set when
  //   creating multiple surfaces by using the CreateSurface method.
  //
  // DDSCAPS_COMPLEX
  //   A complex surface is being described. A complex surface results in the creation of more than
  //   one surface. The additional surfaces are attached to the root surface. The complex structure
  //   can be destroyed only by destroying the root.
  SurfaceDescription.dwBackBufferCount = 1;
  vsPrimary = CreateVSurfaceInternal(&SurfaceDescription);
  if (vsPrimary == NULL) {
    return FALSE;
  }

  // getting the back buffer
  {
    LPDIRECTDRAWSURFACE2 backBuffer;

    DDSCAPS SurfaceCaps;
    SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;
    ReturnCode = IDirectDrawSurface2_GetAttachedSurface(vsPrimary->dds2, &SurfaceCaps, &backBuffer);
    if (ReturnCode != DD_OK) {
      return FALSE;
    }

    vsBackBuffer = (struct JSurface *)mallocZero(sizeof(struct JSurface));
    if (vsBackBuffer == NULL) {
      return FALSE;
    }
    DDSURFACEDESC DDSurfaceDesc;
    memset(&DDSurfaceDesc, 0, sizeof(LPDDSURFACEDESC));
    DDSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    IDirectDrawSurface2_GetSurfaceDesc(backBuffer, &DDSurfaceDesc);
    vsBackBuffer->height = (uint16_t)DDSurfaceDesc.dwHeight;
    vsBackBuffer->width = (uint16_t)DDSurfaceDesc.dwWidth;
    vsBackBuffer->bitDepth = (uint8_t)DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount;
    vsBackBuffer->dds = NULL;
    vsBackBuffer->dds2 = (void *)backBuffer;
  }

  getRGBDistribution();
  return &s_state;
}

void JVideo_Shutdown() {
  IDirectDraw2_RestoreDisplayMode(s_state.gpDirectDrawObject);
  IDirectDraw2_SetCooperativeLevel(s_state.gpDirectDrawObject, ghWindow, DDSCL_NORMAL);
  IDirectDraw2_Release(s_state.gpDirectDrawObject);
}

struct JSurface *JSurface_Create8bpp(uint16_t width, uint16_t height) {
  DDPIXELFORMAT PixelFormat;
  memset(&PixelFormat, 0, sizeof(PixelFormat));
  PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

  PixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
  PixelFormat.dwRGBBitCount = 8;

  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(DDSURFACEDESC));
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwWidth = width;
  SurfaceDescription.dwHeight = height;
  SurfaceDescription.ddpfPixelFormat = PixelFormat;

  return CreateVSurfaceInternal(&SurfaceDescription);
}

struct JSurface *JSurface_Create16bpp(uint16_t width, uint16_t height) {
  DDPIXELFORMAT PixelFormat;
  memset(&PixelFormat, 0, sizeof(PixelFormat));
  PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

  PixelFormat.dwFlags = DDPF_RGB;
  PixelFormat.dwRGBBitCount = 16;

  uint32_t uiRBitMask;
  uint32_t uiGBitMask;
  uint32_t uiBBitMask;
  JVideo_GetRGBDistributionMasks(&uiRBitMask, &uiGBitMask, &uiBBitMask);
  PixelFormat.dwRBitMask = uiRBitMask;
  PixelFormat.dwGBitMask = uiGBitMask;
  PixelFormat.dwBBitMask = uiBBitMask;

  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(DDSURFACEDESC));
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwWidth = width;
  SurfaceDescription.dwHeight = height;
  SurfaceDescription.ddpfPixelFormat = PixelFormat;

  return CreateVSurfaceInternal(&SurfaceDescription);
}

void JSurface_SetPalette32(struct JSurface *vs, struct JPaletteEntry *pal) {
  if (vs->ddPalette == NULL) {
    IDirectDraw2_CreatePalette(s_state.gpDirectDrawObject, (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
                               (LPPALETTEENTRY)(&pal[0]), &vs->ddPalette, NULL);
  } else {
    IDirectDrawPalette_SetEntries(vs->ddPalette, 0, 0, 256, (PALETTEENTRY *)pal);
  }
}

bool tmp_Set8BPPPalette(struct JPaletteEntry *pPalette) {
  HRESULT ReturnCode;
  struct JPaletteEntry gSgpPalette[256];

  // If we are in 256 colors, then we have to initialize the palette system to 0 (faded out)
  memcpy(gSgpPalette, pPalette, sizeof(struct JPaletteEntry) * 256);

  LPDIRECTDRAWPALETTE gpDirectDrawPalette;
  ReturnCode =
      IDirectDraw_CreatePalette(s_state.gpDirectDrawObject, (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
                                (LPPALETTEENTRY)(&gSgpPalette[0]), &gpDirectDrawPalette, NULL);
  if (ReturnCode != DD_OK) {
    return false;
  }
  // Apply the palette to the surfaces
  ReturnCode = IDirectDrawSurface_SetPalette(vsPrimary->dds2, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    return false;
  }

  ReturnCode = IDirectDrawSurface_SetPalette(vsBackBuffer->dds2, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    return false;
  }

  ReturnCode = IDirectDrawSurface_SetPalette(vsFB->dds2, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    return false;
  }

  return (TRUE);
}

bool JSurface_Restore(struct JSurface *vs) {
  HRESULT ReturnCode = IDirectDrawSurface2_Restore(vs->dds2);
  return ReturnCode == DD_OK;
}

bool JSurface_Flip(struct JSurface *vs) {
  HRESULT ReturnCode;
  do {
    ReturnCode = IDirectDrawSurface_Flip(vs->dds, NULL, DDFLIP_WAIT);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      if (ReturnCode == DDERR_SURFACELOST) {
        return false;
      }
    }

  } while (ReturnCode != DD_OK);
  return true;
}

void JSurface_FillRect(struct JSurface *vs, struct JRect *rect, uint16_t color) {
  RECT r = {
      .left = rect->x,
      .top = rect->y,
      .right = rect->x + rect->w,
      .bottom = rect->y + rect->h,
  };

  DDBLTFX BlitterFX;
  BlitterFX.dwSize = sizeof(DDBLTFX);
  BlitterFX.dwFillColor = color;
  HRESULT ReturnCode;
  do {
    ReturnCode = IDirectDrawSurface2_Blt(vs->dds2, &r, NULL, NULL, DDBLT_COLORFILL, &BlitterFX);
  } while (ReturnCode == DDERR_WASSTILLDRAWING);
}

bool JSurface_Lock(struct JSurface *s) {
  if (s == NULL) {
    return false;
  }

  DDSURFACEDESC descr;
  memset(&descr, 0, sizeof(DDSURFACEDESC));
  descr.dwSize = sizeof(DDSURFACEDESC);

  HRESULT ReturnCode;
  do {
    ReturnCode = IDirectDrawSurface2_Lock(s->dds2, NULL, &descr, 0, NULL);
  } while (ReturnCode == DDERR_WASSTILLDRAWING);

  if (descr.lpSurface == NULL) {
    return false;
  }

  s->pitch = descr.lPitch;
  s->pixels = descr.lpSurface;
  return true;
}

void JSurface_Unlock(struct JSurface *s) {
  if (s == NULL) {
    return;
  }
  IDirectDrawSurface2_Unlock(s->dds2, NULL);
  s->pitch = 0;
  s->pixels = NULL;
}

int JSurface_Pitch(struct JSurface *s) { return s->pitch; }
void *JSurface_GetPixels(struct JSurface *s) { return s->pixels; }

void JSurface_SetColorKey(struct JSurface *s, uint32_t key) {
  s->transparencySet = true;

  DDCOLORKEY ColorKey;
  switch (s->bitDepth) {
    case 8:
      ColorKey.dwColorSpaceLowValue = key;
      ColorKey.dwColorSpaceHighValue = key;
      break;

    case 16:
      ColorKey.dwColorSpaceLowValue = rgb32_to_rgb16(key);
      ColorKey.dwColorSpaceHighValue = ColorKey.dwColorSpaceLowValue;
      break;
  }

  IDirectDrawSurface2_SetColorKey(s->dds2, DDCKEY_SRCBLT, &ColorKey);
}

bool JSurface_GetPalette32(struct JSurface *vs, struct JPaletteEntry *pal) {
  if (vs->ddPalette == NULL) {
    LPDIRECTDRAWPALETTE pDDPalette;
    HRESULT ReturnCode = IDirectDrawSurface2_GetPalette(vs->dds2, &pDDPalette);

    if (ReturnCode == DD_OK) {
      vs->ddPalette = pDDPalette;
    } else {
      return false;
    }
  }

  IDirectDrawPalette_GetEntries(vs->ddPalette, 0, 0, 256, (PALETTEENTRY *)pal);
  return true;
}

void JSurface_Free(struct JSurface *s) {
  if (s == NULL) {
    return;
  }
  if (s->ddPalette != NULL) {
    IDirectDrawPalette_Release(s->ddPalette);
    s->ddPalette = NULL;
  }

  if (s->dds2 != NULL) {
    IDirectDrawSurface2_Release(s->dds2);
    s->dds2 = NULL;
  }

  if (s->dds != NULL) {
    IDirectDrawSurface_Release(s->dds);
    s->dds = NULL;
  }

  if (s->palette16 != NULL) {
    free((void *)s->palette16);
    s->palette16 = NULL;
  }

  free(s);
}

uint8_t JSurface_BPP(struct JSurface *s) { return s->bitDepth; }
uint16_t JSurface_Width(struct JSurface *s) { return s->width; }
uint16_t JSurface_Height(struct JSurface *s) { return s->height; }

const uint16_t *JSurface_GetPalette16(struct JSurface *s) { return s->palette16; }

void JSurface_SetPalette16(struct JSurface *s, const uint16_t *palette16) {
  if (s->palette16 != NULL) {
    free((void *)s->palette16);
    s->palette16 = NULL;
  }
  s->palette16 = palette16;
}

#define BLACK_SUBSTITUTE 0x0001

uint16_t rgb_to_rgb16(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t color16 = JVideo_PackRGB16(r, g, b);

  // if our color worked out to absolute black, and the original wasn't
  // absolute black, convert it to a VERY dark grey to avoid transparency
  // problems

  if (color16 == 0 && (r != 0 || g != 0 || b != 0)) {
    return BLACK_SUBSTITUTE;
  }
  return color16;
}

uint16_t rgb32_to_rgb16(uint32_t color32) {
  uint16_t color16 = JVideo_PackRGB16(color32, color32 >> 8, color32 >> 16);
  if (color16 == 0 && color32 != 0) {
    return BLACK_SUBSTITUTE;
  }
  return color16;
}

uint32_t rgb16_to_rgb32(uint16_t color16) {
  uint8_t r, g, b;
  JVideo_UnpackRGB16(color16, &r, &g, &b);
  return ((uint32_t)r) | (((uint32_t)g) << 8) | (((uint32_t)b) << 16);
}
