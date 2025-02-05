#ifndef JA2_JPLATFORM_VIDEO_H
#define JA2_JPLATFORM_VIDEO_H

// Two types of video surfaces are supported:
// - 8 bits per pixel - paletted image
// - 16 bits per pixel in RGB565 encoding

#include <stdbool.h>
#include <stdint.h>

struct JRect {
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

// inline void JRect_set(struct JRect *r, int32_t x, int32_t y, int32_t w, int32_t h) {
//   r->x = x;
//   r->y = y;
//   r->w = w;
//   r->h = h;
// }

// // Square collection of pixels
// struct JSurface;

// // Internal video state
// struct JVideoState;

// #define RGB565_RED_MASK 0xF800
// #define RGB565_GREEN_MASK 0x07E0
// #define RGB565_BLUE_MASK 0x001F

struct JPaletteEntry {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t _unused;

  // Don't change this structure.  On Windows it must be the same as PALETTEENTRY.
};

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct JVideoInitParams;

bool JVideo_Init(char *appName, uint16_t screenWidth, uint16_t screenHeight,
                 struct JVideoInitParams *initParams);
void JVideo_Shutdown();

uint16_t JVideo_GetScreenWidth();
uint16_t JVideo_GetScreenHeight();

// void JVideo_ToggleFullScreen(struct JVideoState *v);
// void JVideo_ToggleMouseGrab(struct JVideoState *v);
// void JVideo_HideCursor();

// void JVideo_RenderSufaceCopy(struct JVideoState *v, struct JSurface *s, struct JRect *r);
// void JVideo_RenderAndPresent(struct JVideoState *v, struct JSurface *s);

// void JVideo_SimulateMouseMovement(struct JVideoState *v, uint32_t newPosX, uint32_t newPosY);

// Lock the surface to get access to raw pixels.
bool JSurface_Lock(struct VSurface *s);

// Unlock the previously locked surface.
void JSurface_Unlock(struct VSurface *s);

int JSurface_Pitch(struct VSurface *s);

void *JSurface_GetPixels(struct VSurface *s);

struct VSurface *JSurface_CreateWithDefaultBpp(uint16_t width, uint16_t height);
struct VSurface *JSurface_Create8bpp(uint16_t width, uint16_t height);
struct VSurface *JSurface_Create16bpp(uint16_t width, uint16_t height);
bool JSurface_Restore(struct VSurface *vs);
bool JSurface_Flip(struct VSurface *vs);

uint16_t JSurface_Width(struct VSurface *s);
uint16_t JSurface_Height(struct VSurface *s);
uint8_t JSurface_BPP(struct VSurface *s);

void JSurface_SetColorKey(struct VSurface *s, uint32_t key);
// bool JSurface_IsColorKeySet(struct JSurface *s);
// void JSurface_Fill(struct JSurface *s, uint16_t color);
void JSurface_FillRect(struct VSurface *vs, struct JRect *rect, uint16_t color);

void JSurface_Free(struct VSurface *s);

void JSurface_SetPalette(struct VSurface *vs, struct JPaletteEntry *pal);
bool JSurface_GetPalette(struct VSurface *vs, struct JPaletteEntry *pal);

// void JSurface_Blit(struct JSurface *src, struct JSurface *dst);

// void JSurface_BlitToPoint(struct JSurface *src, struct JSurface *dst, int32_t destX, int32_t
// destY);

void JSurface_BlitRectToPoint(struct VSurface *src, struct VSurface *dst,
                              struct JRect const *srcBox, int32_t destX, int32_t destY);

void JSurface_BlitRectToRect(struct VSurface *src, struct VSurface *dst, struct JRect const *srcBox,
                             struct JRect const *destBox);

// // Convert from RGB to 16 bit value
// uint16_t rgb32_to_rgb565(uint32_t RGBValue);

// // Convert from 16 BPP to RGBvalue
// uint32_t rgb565_to_rgb32(uint16_t Value16BPP);

void JVideo_GetRGBDistributionMasks(uint32_t *red, uint32_t *green, uint32_t *blue);
uint32_t JVideo_GetTranslucentMask();
uint16_t JVideo_PackRGB16(uint8_t r, uint8_t g, uint8_t b);
void JVideo_UnpackRGB16(uint16_t rgb16, uint8_t *r, uint8_t *g, uint8_t *b);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // JA2_JPLATFORM_VIDEO_H
