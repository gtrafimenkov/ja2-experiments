#include "jplatform_video.h"

void JSurface_BlitRectToPoint(struct VSurface *src, struct VSurface *dst,
                              struct JRect const *srcBox, int32_t destX, int32_t destY) {}

void JSurface_BlitRectToRect(struct VSurface *src, struct VSurface *dst, struct JRect const *srcBox,
                             struct JRect const *destBox) {}

void JVideo_GetRGBDistributionMasks(uint32_t *red, uint32_t *green, uint32_t *blue) {}
uint32_t JVideo_GetTranslucentMask() { return 0; }
uint16_t JVideo_PackRGB16(uint8_t r, uint8_t g, uint8_t b) { return 0; }
void JVideo_UnpackRGB16(uint16_t rgb16, uint8_t *r, uint8_t *g, uint8_t *b) {}

bool tmp_getRGBDistribution() { return false; }
