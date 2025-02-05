// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __VIDEO_H
#define __VIDEO_H

#include <stdint.h>

#include "SGP/Types.h"

struct JPaletteEntry;
struct VSurface;

#define VIDEO_NO_CURSOR 0xFFFF

extern int32_t giNumFrames;

extern BOOLEAN InitializeVideoManager();

extern void ShutdownVideoManager(void);
extern void SuspendVideoManager(void);
extern BOOLEAN RestoreVideoManager(void);
extern uint16_t GetScreenWidth();
extern uint16_t GetScreenHeight();
extern void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight);
extern void InvalidateRegion(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom);
extern void InvalidateScreen(void);
extern BOOLEAN SetCurrentCursor(uint16_t usVideoObjectSubIndex, uint16_t usOffsetX,
                                uint16_t usOffsetY);
extern void EndFrameBufferRender(void);
extern void PrintScreen(void);

void EraseMouseCursor();
extern BOOLEAN SetMouseCursorProperties(int16_t sOffsetX, int16_t sOffsetY, uint16_t usCursorHeight,
                                        uint16_t usCursorWidth);
void DirtyCursor();

bool tmp_Set8BPPPalette(struct JPaletteEntry *pPalette);

void VideoCaptureToggle(void);

void InvalidateRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags);

void RefreshScreen(void *DummyVariable);

#endif
