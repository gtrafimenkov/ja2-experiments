#include <stddef.h>

#include "jplatform_video.h"

struct JSurface *vsPrimary = NULL;
struct JSurface *vsBackBuffer = NULL;
struct JSurface *vsFB = NULL;

void JSurface_BlitRectToPoint(struct JSurface *src, struct JSurface *dst,
                              struct JRect const *srcBox, int32_t destX, int32_t destY) {}

void JSurface_BlitRectToRect(struct JSurface *src, struct JSurface *dst, struct JRect const *srcBox,
                             struct JRect const *destBox) {}

void JVideo_GetRGBDistributionMasks(uint32_t *red, uint32_t *green, uint32_t *blue) {}
uint32_t JVideo_GetTranslucentMask() { return 0; }
uint16_t JVideo_PackRGB16(uint8_t r, uint8_t g, uint8_t b) { return 0; }
void JVideo_UnpackRGB16(uint16_t rgb16, uint8_t *r, uint8_t *g, uint8_t *b) {}

bool JVideo_Init(char *appName, uint16_t screenWidth, uint16_t screenHeight,
                 struct JVideoInitParams *initParams) {
  return false;
}
void JVideo_Shutdown() {}
uint16_t JVideo_GetScreenWidth() { return 0; }
uint16_t JVideo_GetScreenHeight() { return 0; }

struct JSurface *JSurface_Create8bpp(uint16_t width, uint16_t height) { return NULL; };
struct JSurface *JSurface_Create16bpp(uint16_t width, uint16_t height) { return NULL; };
void JSurface_SetPalette32(struct JSurface *vs, struct JPaletteEntry *pal) {};
bool tmp_Set8BPPPalette(struct JPaletteEntry *pPalette) { return false; }
bool JSurface_Restore(struct JSurface *vs) { return false; }
bool JSurface_Flip(struct JSurface *vs) { return false; }
void JSurface_FillRect(struct JSurface *vs, struct JRect *rect, uint16_t color) {}

bool JSurface_Lock(struct JSurface *s) { return false; }
void JSurface_Unlock(struct JSurface *s) {}
int JSurface_Pitch(struct JSurface *s) { return 0; }
void *JSurface_GetPixels(struct JSurface *s) { return NULL; }
void JSurface_SetColorKey(struct JSurface *s, uint32_t key) {}
bool JSurface_GetPalette32(struct JSurface *vs, struct JPaletteEntry *pal) { return false; }
void JSurface_Free(struct JSurface *s) {}
uint8_t JSurface_BPP(struct JSurface *s) { return 0; }
uint16_t JSurface_Width(struct JSurface *s) { return 0; }
uint16_t JSurface_Height(struct JSurface *s) { return 0; }
const uint16_t *JSurface_GetPalette16(struct JSurface *s) { return NULL; }
void JSurface_SetPalette16(struct JSurface *s, const uint16_t *palette16) {}

uint16_t rgb_to_rgb16(uint8_t r, uint8_t g, uint8_t b) { return 0; }
uint16_t rgb32_to_rgb16(uint32_t color32) { return 0; }
uint32_t rgb16_to_rgb32(uint16_t color16) { return 0; }
