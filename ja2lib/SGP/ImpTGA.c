// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

//**************************************************************************
//
// Filename :	impTGA.c
//
//	Purpose :	.tga file importer
//
// Modification history :
//
//		20nov96:HJH				- Creation
//
//**************************************************************************

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "SGP/ImpTGA.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/MemMan.h"
#include "SGP/Types.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "rust_fileman.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

//**************************************************************************
//
//				Function Prototypes
//
//**************************************************************************

BOOLEAN ReadUncompRGBImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap);

//**************************************************************************
//
//				Function Definitions
//
//**************************************************************************

BOOLEAN LoadTGAFileToImage(const char* filePath, struct Image* hImage) {
  FileID hFile = FILE_ID_ERR;
  uint8_t uiImgID, uiColMap, uiType;
  uint32_t uiBytesRead;
  BOOLEAN fReturnVal = FALSE;

  Assert(hImage != NULL);

  if (!(File_Exists(filePath))) {
    return FALSE;
  }

  hFile = File_OpenForReading(filePath);
  if (!(hFile)) {
    return FALSE;
  }

  if (!File_Read(hFile, &uiImgID, sizeof(uint8_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiColMap, sizeof(uint8_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiType, sizeof(uint8_t), &uiBytesRead)) goto end;

  switch (uiType) {
    case 1:
      fReturnVal = FALSE;
      break;
    case 2:
      fReturnVal = ReadUncompRGBImage(hImage, hFile, uiImgID, uiColMap);
      break;
    case 9:
      fReturnVal = FALSE;
      break;
    case 10:
      fReturnVal = FALSE;
      break;
    default:
      break;
  }

  // Set remaining values

end:
  File_Close(hFile);
  return (fReturnVal);
}

BOOLEAN ReadUncompRGBImage(struct Image* hImage, FileID hFile, uint8_t uiImgID, uint8_t uiColMap) {
  uint8_t* pBMData;
  uint8_t* pBMPtr;

  uint16_t uiColMapOrigin;
  uint16_t uiColMapLength;
  uint8_t uiColMapEntrySize;
  uint32_t uiBytesRead;
  uint16_t uiXOrg;
  uint16_t uiYOrg;
  uint16_t uiWidth;
  uint16_t uiHeight;
  uint8_t uiImagePixelSize;
  uint8_t uiImageDescriptor;
  uint32_t iNumValues;
  uint16_t cnt;

  uint32_t i;
  uint8_t r;
  uint8_t g;
  uint8_t b;

  if (!File_Read(hFile, &uiColMapOrigin, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiColMapLength, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiColMapEntrySize, sizeof(uint8_t), &uiBytesRead)) goto end;

  if (!File_Read(hFile, &uiXOrg, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiYOrg, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiWidth, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiHeight, sizeof(uint16_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiImagePixelSize, sizeof(uint8_t), &uiBytesRead)) goto end;
  if (!File_Read(hFile, &uiImageDescriptor, sizeof(uint8_t), &uiBytesRead)) goto end;

  // skip the id
  File_Seek(hFile, uiImgID, FILE_SEEK_CURRENT);

  // skip the colour map
  if (uiColMap != 0) {
    File_Seek(hFile, uiColMapLength * (uiImagePixelSize / 8), FILE_SEEK_CURRENT);
  }

  // Set some struct Image* data values
  hImage->usWidth = uiWidth;
  hImage->usHeight = uiHeight;
  hImage->ubBitDepth = uiImagePixelSize;

  // Allocate memory based on bpp, height, width

  {
    if (uiImagePixelSize == 16) {
      iNumValues = uiWidth * uiHeight;

      hImage->p16BPPData = (uint16_t*)MemAlloc(iNumValues * (uiImagePixelSize / 8));

      if (hImage->p16BPPData == NULL) goto end;

      // Get data pointer
      pBMData = hImage->p8BPPData;

      // Start at end
      pBMData += uiWidth * (uiHeight - 1) * (uiImagePixelSize / 8);

      // Data is stored top-bottom - reverse for SGP struct Image* format
      for (cnt = 0; cnt < uiHeight - 1; cnt++) {
        if (!File_Read(hFile, pBMData, uiWidth * 2, &uiBytesRead)) goto freeEnd;

        pBMData -= uiWidth * 2;
      }
      // Do first row
      if (!File_Read(hFile, pBMData, uiWidth * 2, &uiBytesRead)) goto freeEnd;

      // Convert TGA 5,5,5 16 BPP data into current system 16 BPP Data
      // ConvertTGAToSystemBPPFormat( hImage );

      hImage->fFlags |= IMAGE_BITMAPDATA;
    }

    if (uiImagePixelSize == 24) {
      hImage->p8BPPData = (uint8_t*)MemAlloc(uiWidth * uiHeight * (uiImagePixelSize / 8));

      if (hImage->p8BPPData == NULL) goto end;

      // Get data pointer
      pBMData = (uint8_t*)hImage->p8BPPData;

      // Start at end
      pBMPtr = pBMData + uiWidth * (uiHeight - 1) * 3;

      iNumValues = uiWidth * uiHeight;

      for (cnt = 0; cnt < uiHeight; cnt++) {
        for (i = 0; i < uiWidth; i++) {
          if (!File_Read(hFile, &b, sizeof(uint8_t), &uiBytesRead)) goto freeEnd;
          if (!File_Read(hFile, &g, sizeof(uint8_t), &uiBytesRead)) goto freeEnd;
          if (!File_Read(hFile, &r, sizeof(uint8_t), &uiBytesRead)) goto freeEnd;

          pBMPtr[i * 3] = r;
          pBMPtr[i * 3 + 1] = g;
          pBMPtr[i * 3 + 2] = b;
        }
        pBMPtr -= uiWidth * 3;
      }
      hImage->fFlags |= IMAGE_BITMAPDATA;
    }

#if 0
		// 32 bit not yet allowed in SGP
		else if ( uiImagePixelSize == 32 )
		{
			iNumValues = uiWidth * uiHeight;

			for ( i=0 ; i<iNumValues; i++ )
			{
				if ( !File_Read( hFile, &b, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;
				if ( !File_Read( hFile, &g, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;
				if ( !File_Read( hFile, &r, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;
				if ( !File_Read( hFile, &a, sizeof(uint8_t), &uiBytesRead ) )
					goto freeEnd;

				pBMData[ i*3   ] = r;
				pBMData[ i*3+1 ] = g;
				pBMData[ i*3+2 ] = b;
			}
		}
#endif
  }
  return (TRUE);

end:
  return (FALSE);

freeEnd:
  MemFree(pBMData);
  return (FALSE);
}
