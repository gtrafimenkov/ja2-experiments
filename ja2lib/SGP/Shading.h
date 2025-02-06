// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _SHADING_H_
#define _SHADING_H_

#include "SGP/VObject.h"
#include "jplatform_video.h"

BOOLEAN ShadesCalculateTables(struct JPaletteEntry *p8BPPPalette);

void BuildShadeTable(void);
void BuildIntensityTable(void);
void SetShadeTablePercent(float uiShadePercent);

void Init8BitTables(void);
BOOLEAN Set8BitModePalette(struct JPaletteEntry *pPal);

extern struct JPaletteEntry Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES + 3][256];
extern uint8_t ubColorTables[HVOBJECT_SHADE_TABLES + 3][256];

extern uint16_t IntensityTable[65536];
extern uint16_t ShadeTable[65536];
extern uint16_t White16BPPPalette[256];
extern float guiShadePercent;
extern float guiBrightPercent;

#define DEFAULT_SHADE_LEVEL 4

#endif
