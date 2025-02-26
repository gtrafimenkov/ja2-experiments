// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __FLORIST_GALLERY_H
#define __FLORIST_GALLERY_H

#include "SGP/Types.h"

#define FLOR_GALLERY_TEXT_FILE "BINARYDATA\\FlowerDesc.edt"
#define FLOR_GALLERY_TEXT_TITLE_SIZE 80 * 2
#define FLOR_GALLERY_TEXT_PRICE_SIZE 80 * 2
#define FLOR_GALLERY_TEXT_DESC_SIZE 4 * 80 * 2
#define FLOR_GALLERY_TEXT_TOTAL_SIZE 6 * 80 * 2

void GameInitFloristGallery();
BOOLEAN EnterFloristGallery();
void ExitFloristGallery();
void HandleFloristGallery();
void RenderFloristGallery();
void EnterInitFloristGallery();

extern uint32_t guiCurrentlySelectedFlower;
extern uint8_t gubCurFlowerIndex;

extern int32_t guiGalleryButtonImage;

#endif
