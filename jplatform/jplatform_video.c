#include "jplatform_video.h"

#define SGPGetRValue(rgb) ((uint8_t)(rgb))
#define SGPGetBValue(rgb) ((uint8_t)((rgb) >> 16))
#define SGPGetGValue(rgb) ((uint8_t)(((uint16_t)(rgb)) >> 8))

#define FROMRGB(r, g, b) \
  ((uint32_t)(((uint8_t)(r) | ((uint16_t)(g) << 8)) | (((uint32_t)(uint8_t)(b)) << 16)))

#define BLACK_SUBSTITUTE 0x0001

// Convert from RGB32 to RGB565 16 bit value
uint16_t rgb32_to_rgb565(uint32_t RGBValue) {
  uint16_t r = SGPGetRValue(RGBValue);
  uint16_t g = SGPGetGValue(RGBValue);
  uint16_t b = SGPGetBValue(RGBValue);

  uint16_t usColor =
      ((r << 8) & RGB565_RED_MASK) | ((g << 3) & RGB565_GREEN_MASK) | ((b >> 3) & RGB565_BLUE_MASK);

  // if our color worked out to absolute black, and the original wasn't
  // absolute black, convert it to a VERY dark grey to avoid transparency
  // problems
  if (usColor == 0 && RGBValue != 0) usColor = BLACK_SUBSTITUTE;

  return usColor;
}

// Convert from RGB565 to RGB32
uint32_t rgb565_to_rgb32(uint16_t Value16BPP) {
  uint8_t r = (Value16BPP & RGB565_RED_MASK) >> 8;
  uint8_t g = (Value16BPP & RGB565_GREEN_MASK) >> 3;
  uint8_t b = (Value16BPP & RGB565_BLUE_MASK) << 3;

  uint32_t val = FROMRGB(r, g, b);
  return val;
}
