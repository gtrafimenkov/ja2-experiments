#include "jplatform_video.h"

void JSurface_BlitRectToPoint(struct VSurface *src, struct VSurface *dst,
                              struct JRect const *srcBox, int32_t destX, int32_t destY) {}

void JSurface_BlitRectToRect(struct VSurface *src, struct VSurface *dst, struct JRect const *srcBox,
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

struct VSurface *JSurface_CreateWithDefaultBpp(uint16_t width, uint16_t height) { return NULL; };
struct VSurface *JSurface_Create8bpp(uint16_t width, uint16_t height) { return NULL; };
struct VSurface *JSurface_Create16bpp(uint16_t width, uint16_t height) { return NULL; };
void JSurface_SetPalette(struct VSurface *vs, struct JPaletteEntry *pal) {};
bool tmp_Set8BPPPalette(struct JPaletteEntry *pPalette) { return false; }
bool JSurface_Restore(struct VSurface *vs) { return false; }
