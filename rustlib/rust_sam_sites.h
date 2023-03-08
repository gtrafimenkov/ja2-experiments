#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Return total number of SAM sites
 */
uint8_t GetSamSiteCount(void);

/**
 * Return X location of i-th SAM site
 */
uint8_t GetSamSiteX(uint8_t i);

/**
 * Return Y location of i-th SAM site
 */
uint8_t GetSamSiteY(uint8_t i);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
