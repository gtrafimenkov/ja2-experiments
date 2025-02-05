// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Video.h"

#include <string.h>

#include "SGP/VSurface.h"

void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight) {
  *usWidth = JVideo_GetScreenWidth();
  *usHeight = JVideo_GetScreenHeight();
}

void EraseMouseCursor() { VSurfaceErase(vsMouseBufferOriginal); }
