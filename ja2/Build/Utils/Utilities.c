#include "Utils/Utilities.h"

#include <stdio.h>
#include <time.h>
#include <windows.h>

#include "FileMan.h"
#include "SGP/SGP.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "SysGlobals.h"
#include "Tactical/Overhead.h"
#include "Tactical/OverheadTypes.h"
#include "Utils/FontControl.h"

#define DATA_8_BIT_DIR "8-Bit\\"

void FilenameForBPP(STR pFilename, STR pDestination) {
  CHAR8 Drive[128], Dir[128], Name[128], Ext[128];

  if (GETPIXELDEPTH() == 16) {
    // no processing for 16 bit names
    strcpy(pDestination, pFilename);
  } else {
    _splitpath(pFilename, Drive, Dir, Name, Ext);

    strcat(Name, "_8");

    strcpy(pDestination, Drive);
    // strcat(pDestination, Dir);
    strcat(pDestination, DATA_8_BIT_DIR);
    strcat(pDestination, Name);
    strcat(pDestination, Ext);
  }
}

BOOLEAN CreateSGPPaletteFromCOLFile(struct SGPPaletteEntry *pPalette, SGPFILENAME ColFile) {
  HWFILE hFileHandle;
  BYTE bColHeader[8];
  UINT32 cnt;

  // See if files exists, if not, return error
  if (!FileMan_Exists(ColFile)) {
    // Return FALSE w/ debug
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot find COL file");
    return (FALSE);
  }

  // Open and read in the file
  if ((hFileHandle = FileMan_Open(ColFile, FILE_ACCESS_READ, FALSE)) == 0) {
    // Return FALSE w/ debug
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot open COL file");
    return (FALSE);
  }

  // Skip header
  FileMan_Read(hFileHandle, bColHeader, sizeof(bColHeader), NULL);

  // Read in a palette entry at a time
  for (cnt = 0; cnt < 256; cnt++) {
    FileMan_Read(hFileHandle, &pPalette[cnt].peRed, sizeof(UINT8), NULL);
    FileMan_Read(hFileHandle, &pPalette[cnt].peGreen, sizeof(UINT8), NULL);
    FileMan_Read(hFileHandle, &pPalette[cnt].peBlue, sizeof(UINT8), NULL);
  }

  // Close file
  FileMan_Close(hFileHandle);

  return (TRUE);
}

BOOLEAN DisplayPaletteRep(PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface) {
  UINT16 us16BPPColor;
  UINT32 cnt1;
  UINT8 ubSize, ubType;
  INT16 sTLX, sTLY, sBRX, sBRY;
  UINT8 ubPaletteRep;

  // Create 16BPP Palette
  CHECKF(GetPaletteRepIndexFromID(aPalRep, &ubPaletteRep));

  SetFont(LARGEFONT1);

  ubType = gpPalRep[ubPaletteRep].ubType;
  ubSize = gpPalRep[ubPaletteRep].ubPaletteSize;

  for (cnt1 = 0; cnt1 < ubSize; cnt1++) {
    sTLX = ubXPos + (UINT16)((cnt1 % 16) * 20);
    sTLY = ubYPos + (UINT16)((cnt1 / 16) * 20);
    sBRX = sTLX + 20;
    sBRY = sTLY + 20;

    us16BPPColor =
        Get16BPPColor(FROMRGB(gpPalRep[ubPaletteRep].r[cnt1], gpPalRep[ubPaletteRep].g[cnt1],
                              gpPalRep[ubPaletteRep].b[cnt1]));

    ColorFillVideoSurfaceArea(uiDestSurface, sTLX, sTLY, sBRX, sBRY, us16BPPColor);
  }

  gprintf(ubXPos + (16 * 20), ubYPos, L"%S", gpPalRep[ubPaletteRep].ID);

  return (TRUE);
}

BOOLEAN WrapString(STR16 pStr, STR16 pStr2, UINT16 usWidth, INT32 uiFont) {
  UINT32 Cur, uiLet, uiNewLet, uiHyphenLet;
  CHAR16 *curletter, transletter;
  BOOLEAN fLineSplit = FALSE;
  HVOBJECT hFont;

  // CHECK FOR WRAP
  Cur = 0;
  uiLet = 0;
  curletter = pStr;

  // GET FONT
  hFont = GetFontObject(uiFont);

  // LOOP FORWARDS AND COUNT
  while ((*curletter) != 0) {
    transletter = GetIndex(*curletter);
    Cur += GetWidth(hFont, transletter);

    if (Cur > usWidth) {
      // We are here, loop backwards to find a space
      // Generate second string, and exit upon completion.
      uiHyphenLet = uiLet;  // Save the hyphen location as it won't change.
      uiNewLet = uiLet;
      while ((*curletter) != 0) {
        if ((*curletter) == 32) {
          // Split Line!
          fLineSplit = TRUE;

          pStr[uiNewLet] = (INT16)'\0';

          wcscpy(pStr2, &(pStr[uiNewLet + 1]));
        }

        if (fLineSplit) break;

        uiNewLet--;
        curletter--;
      }
      if (!fLineSplit) {
        // We completed the check for a space, but failed, so use the hyphen method.
        swprintf(pStr2, L"-%s", &(pStr[uiHyphenLet]));
        pStr[uiHyphenLet] = (INT16)'/0';
        fLineSplit = TRUE;  // hyphen method
        break;
      }
    }

    //		if ( fLineSplit )
    //			break;

    uiLet++;
    curletter++;
  }

  return (fLineSplit);
}

BOOLEAN IfWinNT(void) {
  OSVERSIONINFO OsVerInfo;

  OsVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&OsVerInfo);

  if (OsVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    return (TRUE);
  else
    return (FALSE);
}

BOOLEAN IfWin95(void) {
  OSVERSIONINFO OsVerInfo;

  OsVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  GetVersionEx(&OsVerInfo);

  if (OsVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    return (TRUE);
  else
    return (FALSE);
}

void HandleLimitedNumExecutions() {
  // Get system directory
  HWFILE hFileHandle;
  CHAR8 ubSysDir[512];
  INT8 bNumRuns;

  GetSystemDirectory(ubSysDir, sizeof(ubSysDir));

  // Append filename
  strcat(ubSysDir, "\\winaese.dll");

  // Open file and check # runs...
  if (FileMan_Exists(ubSysDir)) {
    // Open and read
    if ((hFileHandle = FileMan_Open(ubSysDir, FILE_ACCESS_READ, FALSE)) == 0) {
      return;
    }

    // Read value
    FileMan_Read(hFileHandle, &bNumRuns, sizeof(bNumRuns), NULL);

    // Close file
    FileMan_Close(hFileHandle);

    if (bNumRuns <= 0) {
      // Fail!
      SET_ERROR("Error 1054: Cannot execute - contact Sir-Tech Software.");
      return;
    }

  } else {
    bNumRuns = 10;
  }

  // OK, decrement # runs...
  bNumRuns--;

  // Open and write
  if ((hFileHandle = FileMan_Open(ubSysDir, FILE_ACCESS_WRITE, FALSE)) == 0) {
    return;
  }

  // Write value
  FileMan_Write(hFileHandle, &bNumRuns, sizeof(bNumRuns), NULL);

  // Close file
  FileMan_Close(hFileHandle);
}

SGPFILENAME gCheckFilenames[] = {
    "DATA\\INTRO.SLF",      "DATA\\LOADSCREENS.SLF", "DATA\\MAPS.SLF",
    "DATA\\NPC_SPEECH.SLF", "DATA\\SPEECH.SLF",
};

UINT32 gCheckFileMinSizes[] = {68000000, 36000000, 87000000, 187000000, 236000000};

BOOLEAN HandleJA2CDCheck() { return (TRUE); }

BOOLEAN HandleJA2CDCheckTwo() { return (TRUE); }

BOOLEAN DoJA2FilesExistsOnDrive(CHAR8 *zCdLocation) {
  BOOLEAN fFailed = FALSE;
  CHAR8 zCdFile[SGPFILENAME_LEN];
  INT32 cnt;
  HWFILE hFile;

  for (cnt = 0; cnt < 4; cnt++) {
    // OK, build filename
    sprintf(zCdFile, "%s%s", zCdLocation, gCheckFilenames[cnt]);

    hFile = FileMan_Open(zCdFile, FILE_ACCESS_READ | FILE_OPEN_EXISTING, FALSE);

    // Check if it exists...
    if (!hFile) {
      fFailed = TRUE;
      FileMan_Close(hFile);
      break;
    }
    FileMan_Close(hFile);
  }

  return (!fFailed);
}
