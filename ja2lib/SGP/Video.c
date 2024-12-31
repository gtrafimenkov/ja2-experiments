// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Video.h"

#include "SGP/VideoInternal.h"

uint16_t gusScreenWidth;
uint16_t gusScreenHeight;

void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight) {
  *usWidth = (uint16_t)gusScreenWidth;
  *usHeight = (uint16_t)gusScreenHeight;
}
