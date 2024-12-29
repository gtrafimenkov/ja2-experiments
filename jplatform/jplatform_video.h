#ifndef JA2_JPLATFORM_VIDEO_H
#define JA2_JPLATFORM_VIDEO_H

// Two types of video surfaces are supported:
// - 8 bits per pixel - paletted image
// - 16 bits per pixel in RGB565 encoding

#include <stdbool.h>
#include <stdint.h>

#define RGB565_RED_MASK 0xF800
#define RGB565_GREEN_MASK 0x07E0
#define RGB565_BLUE_MASK 0x001F

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Convert from RGB to 16 bit value
uint16_t rgb32_to_rgb565(uint32_t RGBValue);

// Convert from 16 BPP to RGBvalue
uint32_t rgb565_to_rgb32(uint16_t Value16BPP);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // JA2_JPLATFORM_VIDEO_H
