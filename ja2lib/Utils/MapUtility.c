// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/MapUtility.h"

#include "Editor/LoadScreen.h"
#include "Globals.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/Line.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Screens.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldDef.h"
#include "Utils/FontControl.h"
#include "Utils/STIConvert.h"
#include "platform.h"

#ifdef JA2EDITOR

#include "Utils/QuantizeWrap.h"

#define MINIMAP_X_SIZE 88
#define MINIMAP_Y_SIZE 44

#define WINDOW_SIZE 2

float gdXStep, gdYStep;
static struct JSurface *vsMiniMap;
static struct JSurface *vs8BitMiniMap;

extern BOOLEAN gfOverheadMapDirty;

// Utililty file for sub-sampling/creating our radar screen maps
// Loops though our maps directory and reads all .map files, subsamples an area, color
// quantizes it into an 8-bit image ans writes it to an sti file in radarmaps.

typedef struct {
  int8_t r;
  int8_t g;
  int8_t b;

} RGBValues;

uint32_t MapUtilScreenInit() { return (TRUE); }

uint32_t MapUtilScreenHandle() {
  static int16_t fNewMap = TRUE;
  static int16_t sFileNum = 0;
  InputAtom InputEvent;
  struct GetFile FileInfo;
  static struct FileDialogList *FListNode;
  static int16_t sFiles = 0, sCurFile = 0;
  static struct FileDialogList *FileList = NULL;
  char zFilename[260], zFilename2[260];
  uint16_t usWidth;
  uint16_t usHeight;
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint16_t *pDestBuf, *pSrcBuf;
  uint8_t *pDataPtr;

  static uint8_t *p24BitDest = NULL;
  static RGBValues *p24BitValues = NULL;

  uint32_t uiRGBColor;

  uint32_t bR, bG, bB, bAvR, bAvG, bAvB;
  int16_t s16BPPSrc, sDest16BPPColor;
  int32_t cnt;

  int16_t sX1, sX2, sY1, sY2, sTop, sBottom, sLeft, sRight;

  float dX, dY, dStartX, dStartY;
  int32_t iX, iY, iSubX1, iSubY1, iSubX2, iSubY2, iWindowX, iWindowY, iCount;
  struct JPaletteEntry pPalette[256];

  sDest16BPPColor = -1;
  bAvR = bAvG = bAvB = 0;

  // Zero out area!
  ColorFillVSurfaceArea(vsFB, 0, 0, (int16_t)(640), (int16_t)(480),
                        rgb32_to_rgb16(FROMRGB(0, 0, 0)));

  if (fNewMap) {
    fNewMap = FALSE;

    // Create render buffer
    GetCurrentVideoSettings(&usWidth, &usHeight);
    vsMiniMap = JSurface_Create16bpp(88, 44);
    JSurface_SetColorKey(vsMiniMap, FROMRGB(0, 0, 0));
    if (vsMiniMap == NULL) {
      return (ERROR_SCREEN);
    }

    // USING BRET's STUFF FOR LOOPING FILES/CREATING LIST, hence AddToFDlgList.....
    if (Plat_GetFileFirst("MAPS\\*.dat", &FileInfo)) {
      FileList = AddToFDlgList(FileList, &FileInfo);
      sFiles++;
      while (Plat_GetFileNext(&FileInfo)) {
        FileList = AddToFDlgList(FileList, &FileInfo);
        sFiles++;
      }
      Plat_GetFileClose(&FileInfo);
    }

    FListNode = FileList;

    // Allocate 24 bit Surface
    p24BitValues = (RGBValues *)MemAlloc(MINIMAP_X_SIZE * MINIMAP_Y_SIZE * sizeof(RGBValues));
    p24BitDest = (uint8_t *)p24BitValues;

    vs8BitMiniMap = JSurface_Create8bpp(88, 44);
    JSurface_SetColorKey(vs8BitMiniMap, FROMRGB(0, 0, 0));
    if (vs8BitMiniMap == NULL) {
      return (ERROR_SCREEN);
    }
  }

  // OK, we are here, now loop through files
  if (sCurFile == sFiles || FListNode == NULL) {
    gfProgramIsRunning = FALSE;
    return (MAPUTILITY_SCREEN);
  }

  sprintf(zFilename, "%s", FListNode->FileInfo.zFileName);

  // OK, load maps and do overhead shrinkage of them...
  if (!LoadWorld(zFilename)) {
    return (ERROR_SCREEN);
  }

  // Render small map
  InitNewOverheadDB((uint8_t)giCurrentTilesetID);

  gfOverheadMapDirty = TRUE;

  RenderOverheadMap(0, (WORLD_COLS / 2), 0, 0, 640, 320, TRUE);

  TrashOverheadMap();

  // OK, NOW PROCESS OVERHEAD MAP ( SHOUIDL BE ON THE FRAMEBUFFER )
  gdXStep = (float)640 / (float)88;
  gdYStep = (float)320 / (float)44;
  dStartX = dStartY = 0;

  // Adjust if we are using a restricted map...
  if (gMapInformation.ubRestrictedScrollID != 0) {
    CalculateRestrictedMapCoords(NORTH, &sX1, &sY1, &sX2, &sTop, 640, 320);
    CalculateRestrictedMapCoords(SOUTH, &sX1, &sBottom, &sX2, &sY2, 640, 320);
    CalculateRestrictedMapCoords(WEST, &sX1, &sY1, &sLeft, &sY2, 640, 320);
    CalculateRestrictedMapCoords(EAST, &sRight, &sY1, &sX2, &sY2, 640, 320);

    gdXStep = (float)(sRight - sLeft) / (float)88;
    gdYStep = (float)(sBottom - sTop) / (float)44;

    dStartX = sLeft;
    dStartY = sTop;
  }

  // LOCK BUFFERS

  dX = dStartX;
  dY = dStartY;

  pDestBuf = (uint16_t *)LockVSurface(vsMiniMap, &uiDestPitchBYTES);
  pSrcBuf = (uint16_t *)LockVSurface(vsFB, &uiSrcPitchBYTES);

  for (iX = 0; iX < 88; iX++) {
    dY = dStartY;

    for (iY = 0; iY < 44; iY++) {
      // OK, AVERAGE PIXELS
      iSubX1 = (int32_t)dX - WINDOW_SIZE;

      iSubX2 = (int32_t)dX + WINDOW_SIZE;

      iSubY1 = (int32_t)dY - WINDOW_SIZE;

      iSubY2 = (int32_t)dY + WINDOW_SIZE;

      iCount = 0;
      bR = bG = bB = 0;

      for (iWindowX = iSubX1; iWindowX < iSubX2; iWindowX++) {
        for (iWindowY = iSubY1; iWindowY < iSubY2; iWindowY++) {
          if (iWindowX >= 0 && iWindowX < 640 && iWindowY >= 0 && iWindowY < 320) {
            s16BPPSrc = pSrcBuf[(iWindowY * (uiSrcPitchBYTES / 2)) + iWindowX];

            uiRGBColor = rgb16_to_rgb32(s16BPPSrc);

            bR += SGPGetRValue(uiRGBColor);
            bG += SGPGetGValue(uiRGBColor);
            bB += SGPGetBValue(uiRGBColor);

            // Average!
            iCount++;
          }
        }
      }

      if (iCount > 0) {
        bAvR = bR / (uint8_t)iCount;
        bAvG = bG / (uint8_t)iCount;
        bAvB = bB / (uint8_t)iCount;

        sDest16BPPColor = rgb32_to_rgb16(FROMRGB(bAvR, bAvG, bAvB));
      }

      // Write into dest!
      pDestBuf[(iY * (uiDestPitchBYTES / 2)) + iX] = sDest16BPPColor;

      p24BitValues[(iY * (uiDestPitchBYTES / 2)) + iX].r = (uint8_t)bAvR;
      p24BitValues[(iY * (uiDestPitchBYTES / 2)) + iX].g = (uint8_t)bAvG;
      p24BitValues[(iY * (uiDestPitchBYTES / 2)) + iX].b = (uint8_t)bAvB;

      // Increment
      dY += gdYStep;
    }

    // Increment
    dX += gdXStep;
  }

  JSurface_Unlock(vsMiniMap);
  JSurface_Unlock(vsFB);

  // QUantize!
  pDataPtr = (uint8_t *)LockVSurface(vs8BitMiniMap, &uiSrcPitchBYTES);
  pDestBuf = (uint16_t *)LockVSurface(vsFB, &uiDestPitchBYTES);
  QuantizeImage(pDataPtr, p24BitDest, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, pPalette);
  JSurface_SetPalette32(vs8BitMiniMap, pPalette);
  JSurface_SetPalette16(vs8BitMiniMap, Create16BPPPalette(pPalette));

  // Blit!
  Blt8BPPDataTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, vs8BitMiniMap, pDataPtr, 300, 360);

  // Write palette!
  {
    int32_t cnt;
    int32_t sX = 0, sY = 420;
    uint16_t usLineColor;

    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

    for (cnt = 0; cnt < 256; cnt++) {
      usLineColor =
          rgb32_to_rgb16(FROMRGB(pPalette[cnt].red, pPalette[cnt].green, pPalette[cnt].blue));
      RectangleDraw(TRUE, sX, sY, sX, (int16_t)(sY + 10), usLineColor, (uint8_t *)pDestBuf);
      sX++;
      RectangleDraw(TRUE, sX, sY, sX, (int16_t)(sY + 10), usLineColor, (uint8_t *)pDestBuf);
      sX++;
    }
  }

  JSurface_Unlock(vsFB);

  // Remove extension
  for (cnt = strlen(zFilename) - 1; cnt >= 0; cnt--) {
    if (zFilename[cnt] == '.') {
      zFilename[cnt] = '\0';
    }
  }

  sprintf(zFilename2, "RADARMAPS\\%s.STI", zFilename);
  WriteSTIFile(pDataPtr, pPalette, MINIMAP_X_SIZE, MINIMAP_Y_SIZE, zFilename2,
               CONVERT_ETRLE_COMPRESS, 0);

  JSurface_Unlock(vs8BitMiniMap);

  SetFont(TINYFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_DKGRAY);
  mprintf(10, 340, L"Writing radar image %S", zFilename2);

  mprintf(10, 350, L"Using tileset %s", gTilesets[giCurrentTilesetID].zName);

  InvalidateScreen();

  while (DequeueEvent(&InputEvent) == TRUE) {
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ESC)) {  // Exit the program
      gfProgramIsRunning = FALSE;
    }
  }

  // Set next
  FListNode = FListNode->pNext;
  sCurFile++;

  return (MAPUTILITY_SCREEN);
}

uint32_t MapUtilScreenShutdown() { return (TRUE); }

#else  // non-editor version

#include "SGP/Types.h"
#include "ScreenIDs.h"

uint32_t MapUtilScreenInit() { return (TRUE); }

uint32_t MapUtilScreenHandle() {
  // If this screen ever gets set, then this is a bad thing -- endless loop
  return (ERROR_SCREEN);
}

uint32_t MapUtilScreenShutdown() { return (TRUE); }

#endif
