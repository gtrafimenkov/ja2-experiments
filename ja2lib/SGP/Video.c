// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Video.h"

#include "SGP/VSurface.h"
#include "SGP/VideoInternal.h"

uint16_t gusScreenWidth;
uint16_t gusScreenHeight;

void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight) {
  *usWidth = (uint16_t)gusScreenWidth;
  *usHeight = (uint16_t)gusScreenHeight;
}

void EraseMouseCursor() {
  uint32_t uiPitch;
  void *pTmpPointer = LockVSurfaceByID(MOUSE_BUFFER, &uiPitch);
  if (pTmpPointer) {
    memset(pTmpPointer, 0, MAX_CURSOR_HEIGHT * uiPitch);
    UnlockVSurfaceByID(MOUSE_BUFFER);
  }
}
