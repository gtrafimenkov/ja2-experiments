// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/PCX.h"

#include <stdarg.h>
#include <stdio.h>

#include "SGP/FileMan.h"
#include "SGP/MemMan.h"
#include "jplatform_video.h"

// Local typedefs

#define PCX_NORMAL 1
#define PCX_RLE 2
#define PCX_256COLOR 4
#define PCX_TRANSPARENT 8
#define PCX_CLIPPED 16
#define PCX_REALIZEPALETTE 32
#define PCX_X_CLIPPING 64
#define PCX_Y_CLIPPING 128
#define PCX_NOTLOADED 256

#define PCX_ERROROPENING 1
#define PCX_INVALIDFORMAT 2
#define PCX_INVALIDLEN 4
#define PCX_OUTOFMEMORY 8

BOOLEAN SetPcxPalette(PcxObject *pCurrentPcxObject, HIMAGE hImage);
BOOLEAN BlitPcxToBuffer(PcxObject *pCurrentPcxObject, uint8_t *pBuffer, uint16_t usBufferWidth,
                        uint16_t usBufferHeight, uint16_t usX, uint16_t usY, BOOLEAN fTransp);
PcxObject *LoadPcx(char *pFilename);

BOOLEAN LoadPCXFileToImage(HIMAGE hImage, uint16_t fContents) {
  PcxObject *pPcxObject;

  // First Load a PCX Image
  pPcxObject = LoadPcx(hImage->ImageFile);

  if (pPcxObject == NULL) {
    return (FALSE);
  }

  // Set some header information
  hImage->usWidth = pPcxObject->usWidth;
  hImage->usHeight = pPcxObject->usHeight;
  hImage->ubBitDepth = 8;
  hImage->fFlags = hImage->fFlags | fContents;

  // Read and allocate bitmap block if requested
  if (fContents & IMAGE_BITMAPDATA) {
    // Allocate memory for buffer
    hImage->p8BPPData = (uint8_t *)MemAlloc(hImage->usWidth * hImage->usHeight);

    if (!BlitPcxToBuffer(pPcxObject, hImage->p8BPPData, hImage->usWidth, hImage->usHeight, 0, 0,
                         FALSE)) {
      MemFree(hImage->p8BPPData);
      return (FALSE);
    }
  }

  if (fContents & IMAGE_PALETTE) {
    SetPcxPalette(pPcxObject, hImage);

    // Create 16 BPP palette if flags and BPP justify
    hImage->pui16BPPPalette = Create16BPPPalette(hImage->pPalette);
  }

  // Free and remove pcx object
  MemFree(pPcxObject->pPcxBuffer);
  MemFree(pPcxObject);

  return (TRUE);
}

PcxObject *LoadPcx(char *pFilename) {
  PcxHeader Header;
  PcxObject *pCurrentPcxObject;
  HWFILE hFileHandle;
  uint32_t uiFileSize;
  uint8_t *pPcxBuffer;

  // Open and read in the file
  if ((hFileHandle = FileMan_Open(pFilename, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE)) ==
      0) {  // damn we failed to open the file
    return NULL;
  }

  uiFileSize = FileMan_GetSize(hFileHandle);
  if (uiFileSize == 0) {  // we failed to size up the file
    return NULL;
  }

  // Create enw pCX object
  pCurrentPcxObject = (PcxObject *)MemAlloc(sizeof(PcxObject));

  if (pCurrentPcxObject == NULL) {
    return (NULL);
  }

  pCurrentPcxObject->pPcxBuffer = (uint8_t *)MemAlloc(uiFileSize - (sizeof(PcxHeader) + 768));

  if (pCurrentPcxObject->pPcxBuffer == NULL) {
    return (NULL);
  }

  // Ok we now have a file handle, so let's read in the data
  FileMan_Read(hFileHandle, &Header, sizeof(PcxHeader), NULL);
  if ((Header.ubManufacturer != 10) || (Header.ubEncoding != 1)) {  // We have an invalid pcx format
    // Delete the object
    MemFree(pCurrentPcxObject->pPcxBuffer);
    MemFree(pCurrentPcxObject);
    return (NULL);
  }

  if (Header.ubBitsPerPixel == 8) {
    pCurrentPcxObject->usPcxFlags = PCX_256COLOR;
  } else {
    pCurrentPcxObject->usPcxFlags = 0;
  }

  pCurrentPcxObject->usWidth = 1 + (Header.usRight - Header.usLeft);
  pCurrentPcxObject->usHeight = 1 + (Header.usBottom - Header.usTop);
  pCurrentPcxObject->uiBufferSize = uiFileSize - 768 - sizeof(PcxHeader);

  // We are ready to read in the pcx buffer data. Therefore we must lock the buffer
  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  FileMan_Read(hFileHandle, pPcxBuffer, pCurrentPcxObject->uiBufferSize, NULL);

  // Read in the palette
  FileMan_Read(hFileHandle, &(pCurrentPcxObject->ubPalette[0]), 768, NULL);

  // Close file
  FileMan_Close(hFileHandle);

  return pCurrentPcxObject;
}

BOOLEAN BlitPcxToBuffer(PcxObject *pCurrentPcxObject, uint8_t *pBuffer, uint16_t usBufferWidth,
                        uint16_t usBufferHeight, uint16_t usX, uint16_t usY, BOOLEAN fTransp) {
  uint8_t *pPcxBuffer;
  uint8_t ubRepCount;
  uint16_t usMaxX, usMaxY;
  uint32_t uiImageSize;
  uint8_t ubCurrentByte = 0;
  uint8_t ubMode;
  uint16_t usCurrentX, usCurrentY;
  uint32_t uiOffset, uiIndex;
  uint32_t uiNextLineOffset, uiStartOffset, uiCurrentOffset;

  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  if (((pCurrentPcxObject->usWidth + usX) == usBufferWidth) &&
      ((pCurrentPcxObject->usHeight + usY) ==
       usBufferHeight)) {  // Pre-compute PCX blitting aspects.
    uiImageSize = usBufferWidth * usBufferHeight;
    ubMode = PCX_NORMAL;
    uiOffset = 0;
    ubRepCount = 0;

    // Blit Pcx object. Two main cases, one for transparency (0's are skipped and for without
    // transparency.
    if (fTransp == TRUE) {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }
        if (ubCurrentByte != 0) {
          *(pBuffer + uiIndex) = ubCurrentByte;
        }
      }
    } else {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }
        *(pBuffer + uiIndex) = ubCurrentByte;
      }
    }
  } else {  // Pre-compute PCX blitting aspects.
    if ((pCurrentPcxObject->usWidth + usX) >= usBufferWidth) {
      pCurrentPcxObject->usPcxFlags |= PCX_X_CLIPPING;
      usMaxX = usBufferWidth - 1;
    } else {
      usMaxX = pCurrentPcxObject->usWidth + usX;
    }

    if ((pCurrentPcxObject->usHeight + usY) >= usBufferHeight) {
      pCurrentPcxObject->usPcxFlags |= PCX_Y_CLIPPING;
      uiImageSize = pCurrentPcxObject->usWidth * (usBufferHeight - usY);
      usMaxY = usBufferHeight - 1;
    } else {
      uiImageSize = pCurrentPcxObject->usWidth * pCurrentPcxObject->usHeight;
      usMaxY = pCurrentPcxObject->usHeight + usY;
    }

    ubMode = PCX_NORMAL;
    uiOffset = 0;
    ubRepCount = 0;
    usCurrentX = usX;
    usCurrentY = usY;

    // Blit Pcx object. Two main cases, one for transparency (0's are skipped and for without
    // transparency.
    if (fTransp == TRUE) {
      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }
        if (ubCurrentByte != 0) {
          *(pBuffer + (usCurrentY * usBufferWidth) + usCurrentX) = ubCurrentByte;
        }
        usCurrentX++;
        if (usCurrentX > usMaxX) {
          usCurrentX = usX;
          usCurrentY++;
        }
      }
    } else {
      uiStartOffset = (usCurrentY * usBufferWidth) + usCurrentX;
      uiNextLineOffset = uiStartOffset + usBufferWidth;
      uiCurrentOffset = uiStartOffset;

      for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++) {
        if (ubMode == PCX_NORMAL) {
          ubCurrentByte = *(pPcxBuffer + uiOffset++);
          if (ubCurrentByte > 0x0BF) {
            ubRepCount = ubCurrentByte & 0x03F;
            ubCurrentByte = *(pPcxBuffer + uiOffset++);
            if (--ubRepCount > 0) {
              ubMode = PCX_RLE;
            }
          }
        } else {
          if (--ubRepCount == 0) {
            ubMode = PCX_NORMAL;
          }
        }

        if (usCurrentX <
            usMaxX) {  // We are within the visible bounds so we write the byte to buffer
          *(pBuffer + uiCurrentOffset) = ubCurrentByte;
          uiCurrentOffset++;
          usCurrentX++;
        } else {
          if ((uiCurrentOffset + 1) < uiNextLineOffset) {  // Increment the uiCurrentOffset
            uiCurrentOffset++;
          } else {  // Go to next line
            usCurrentX = usX;
            usCurrentY++;
            if (usCurrentY > usMaxY) {
              break;
            }
            uiStartOffset = (usCurrentY * usBufferWidth) + usCurrentX;
            uiNextLineOffset = uiStartOffset + usBufferWidth;
            uiCurrentOffset = uiStartOffset;
          }
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN SetPcxPalette(PcxObject *pCurrentPcxObject, HIMAGE hImage) {
  uint16_t Index;
  uint8_t *pubPalette;

  pubPalette = &(pCurrentPcxObject->ubPalette[0]);

  // Allocate memory for palette
  hImage->pPalette = (struct JPaletteEntry *)MemAlloc(sizeof(struct JPaletteEntry) * 256);

  if (hImage->pPalette == NULL) {
    return (FALSE);
  }

  // Initialize the proper palette entries
  for (Index = 0; Index < 256; Index++) {
    hImage->pPalette[Index].red = *(pubPalette + (Index * 3));
    hImage->pPalette[Index].green = *(pubPalette + (Index * 3) + 1);
    hImage->pPalette[Index].blue = *(pubPalette + (Index * 3) + 2);
    hImage->pPalette[Index]._unused = 0;
  }

  return TRUE;
}
