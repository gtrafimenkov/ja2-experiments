// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Font.h"

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "GameRes.h"
#include "Globals.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/HImage.h"
#include "SGP/MemMan.h"
#include "SGP/PCX.h"
#include "SGP/TranslationTable.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"

//*******************************************************
//
//   Defines
//
//*******************************************************

#define PALETTE_SIZE 768
#define STRING_DELIMITER 0
#define ID_BLACK 0
#define MAX_FONTS 25

//*******************************************************
//
//   Typedefs
//
//*******************************************************

typedef struct {
  uint16_t usDefaultPixelDepth;
  FontTranslationTable *pTranslationTable;
} FontManager;

struct VObject *FontObjs[MAX_FONTS];
int32_t FontsLoaded = 0;

// Destination printing parameters
int32_t FontDefault = (-1);
uint32_t FontDestBuffer = BACKBUFFER;
uint32_t FontDestPitch = 640 * 2;
uint32_t FontDestBPP = 16;
SGPRect FontDestRegion = {0, 0, 640, 480};
BOOLEAN FontDestWrap = FALSE;
uint16_t FontForeground16 = 0;
uint16_t FontBackground16 = 0;
uint16_t FontShadow16 = DEFAULT_SHADOW;
uint8_t FontForeground8 = 0;
uint8_t FontBackground8 = 0;

// Temp, for saving printing parameters
int32_t SaveFontDefault = (-1);
uint32_t SaveFontDestBuffer = BACKBUFFER;
uint32_t SaveFontDestPitch = 640 * 2;
uint32_t SaveFontDestBPP = 16;
SGPRect SaveFontDestRegion = {0, 0, 640, 480};
BOOLEAN SaveFontDestWrap = FALSE;
uint16_t SaveFontForeground16 = 0;
uint16_t SaveFontShadow16 = 0;
uint16_t SaveFontBackground16 = 0;
uint8_t SaveFontForeground8 = 0;
uint8_t SaveFontBackground8 = 0;

//*****************************************************************************
// SetFontColors
//
//	Sets both the foreground and the background colors of the current font. The
// top byte of the parameter word is the background color, and the bottom byte
// is the foreground.
//
//*****************************************************************************
void SetFontColors(uint16_t usColors) {
  uint8_t ubForeground, ubBackground;

  ubForeground = (uint8_t)(usColors & 0xff);
  ubBackground = (uint8_t)((usColors & 0xff00) >> 8);

  SetFontForeground(ubForeground);
  SetFontBackground(ubBackground);
}

//*****************************************************************************
// SetFontForeground
//
//	Sets the foreground color of the currently selected font. The parameter is
// the index into the 8-bit palette. In 8BPP mode, that index number is used
// for the pixel value to be drawn for nontransparent pixels. In 16BPP mode,
// the RGB values from the palette are used to create the pixel color. Note
// that if you change fonts, the selected foreground/background colors will
// stay at what they are currently set to.
//
//*****************************************************************************
void SetFontForeground(uint8_t ubForeground) {
  uint32_t uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  FontForeground8 = ubForeground;

  uiRed = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peRed;
  uiGreen = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peGreen;
  uiBlue = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubForeground].peBlue;

  FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetFontShadow(uint8_t ubShadow) {
  uint32_t uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  // FontForeground8=ubForeground;

  uiRed = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peRed;
  uiGreen = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peGreen;
  uiBlue = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubShadow].peBlue;

  FontShadow16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));

  if (ubShadow != 0) {
    if (FontShadow16 == 0) {
      FontShadow16 = 1;
    }
  }
}

//*****************************************************************************
// SetFontBackground
//
//	Sets the Background color of the currently selected font. The parameter is
// the index into the 8-bit palette. In 8BPP mode, that index number is used
// for the pixel value to be drawn for nontransparent pixels. In 16BPP mode,
// the RGB values from the palette are used to create the pixel color. If the
// background value is zero, the background of the font will be transparent.
// Note that if you change fonts, the selected foreground/background colors will
// stay at what they are currently set to.
//
//*****************************************************************************
void SetFontBackground(uint8_t ubBackground) {
  uint32_t uiRed, uiGreen, uiBlue;

  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;

  FontBackground8 = ubBackground;

  uiRed = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peRed;
  uiGreen = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peGreen;
  uiBlue = (uint32_t)FontObjs[FontDefault]->pPaletteEntry[ubBackground].peBlue;

  FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

// Kris:  These are new counterparts to the above functions.  They won't
//			 effect an 8BPP font, only 16.
void SetRGBFontForeground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontForeground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetRGBFontBackground(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontBackground16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}

void SetRGBFontShadow(uint32_t uiRed, uint32_t uiGreen, uint32_t uiBlue) {
  if ((FontDefault < 0) || (FontDefault > MAX_FONTS)) return;
  FontShadow16 = Get16BPPColor(FROMRGB(uiRed, uiGreen, uiBlue));
}
// end Kris

//*****************************************************************************
// ResetFontObjectPalette
//
//	Sets the palette of a font, using an 8 bit palette (which is converted to
// the appropriate 16-bit palette, and assigned to the struct VObject*).
//
//*****************************************************************************
BOOLEAN ResetFontObjectPalette(int32_t iFont) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  SetFontObjectPalette8BPP(iFont, FontObjs[iFont]->pPaletteEntry);

  return (TRUE);
}

//*****************************************************************************
// SetFontObjectPalette8BPP
//
//	Sets the palette of a font, using an 8 bit palette (which is converted to
// the appropriate 16-bit palette, and assigned to the struct VObject*).
//
//*****************************************************************************
uint16_t *SetFontObjectPalette8BPP(int32_t iFont, struct SGPPaletteEntry *pPal8) {
  uint16_t *pPal16;

  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  if ((pPal16 = Create16BPPPalette(pPal8)) == NULL) return (NULL);

  FontObjs[iFont]->p16BPPPalette = pPal16;
  FontObjs[iFont]->pShadeCurrent = pPal16;

  return (pPal16);
}

//*****************************************************************************
// SetFontObjectPalette16BPP
//
//	Sets the palette of a font, using a 16 bit palette.
//
//*****************************************************************************
uint16_t *SetFontObjectPalette16BPP(int32_t iFont, uint16_t *pPal16) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  FontObjs[iFont]->p16BPPPalette = pPal16;
  FontObjs[iFont]->pShadeCurrent = pPal16;

  return (pPal16);
}

//*****************************************************************************
// GetFontObjectPalette16BPP
//
//	Sets the palette of a font, using a 16 bit palette.
//
//*****************************************************************************
uint16_t *GetFontObjectPalette16BPP(int32_t iFont) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  return (FontObjs[iFont]->p16BPPPalette);
}

//*****************************************************************************
// GetFontObject
//
//	Returns the VOBJECT pointer of a font.
//
//*****************************************************************************
struct VObject *GetFontObject(int32_t iFont) {
  Assert(iFont >= 0);
  Assert(iFont <= MAX_FONTS);
  Assert(FontObjs[iFont] != NULL);

  return (FontObjs[iFont]);
}

//*****************************************************************************
// FindFreeFont
//
//	Locates an empty slot in the font table.
//
//*****************************************************************************
int32_t FindFreeFont(void) {
  int count;

  for (count = 0; count < MAX_FONTS; count++)
    if (FontObjs[count] == NULL) return (count);

  return (-1);
}

//*****************************************************************************
// LoadFontFile
//
//	Loads a font from an ETRLE file, and inserts it into one of the font slots.
//  This function returns (-1) if it fails, and debug msgs for a reason.
//  Otherwise the font number is returned.
//*****************************************************************************
int32_t LoadFontFile(char *filename) {
  VOBJECT_DESC vo_desc;
  uint32_t LoadIndex;

  Assert(filename != NULL);
  Assert(strlen(filename));

  if ((LoadIndex = FindFreeFont()) == (-1)) {
    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Out of font slots (%s)", filename));
    FatalError("Cannot init FONT file %s", filename);
    return (-1);
  }

  vo_desc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
  strcpy(vo_desc.ImageFile, filename);

  if ((FontObjs[LoadIndex] = CreateVideoObject(&vo_desc)) == NULL) {
    DbgMessage(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error creating VOBJECT (%s)", filename));
    FatalError("Cannot init FONT file %s", filename);
    return (-1);
  }

  if (FontDefault == (-1)) FontDefault = LoadIndex;

  return (LoadIndex);
}

//*****************************************************************************
// UnloadFont - Delete the font structure
//
//	Deletes the video object of a particular font. Frees up the memory and
// resources allocated for it.
//
//*****************************************************************************
void UnloadFont(uint32_t FontIndex) {
  Assert(FontIndex >= 0);
  Assert(FontIndex <= MAX_FONTS);
  Assert(FontObjs[FontIndex] != NULL);

  DeleteVideoObject(FontObjs[FontIndex]);
  FontObjs[FontIndex] = NULL;
}

//*****************************************************************************
// GetWidth
//
//	Returns the width of a given character in the font.
//
//*****************************************************************************
uint32_t GetWidth(struct VObject *hSrcVObject, int16_t ssIndex) {
  ETRLEObject *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[ssIndex]);
  return ((uint32_t)(pTrav->usWidth + pTrav->sOffsetX));
}

//*****************************************************************************
// StringPixLengthArg
//
//		Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.
//    'uiCharCount' specifies how many characters of the string are counted.
//*****************************************************************************
int16_t StringPixLengthArg(int32_t usUseFont, uint32_t uiCharCount, wchar_t *pFontString, ...) {
  va_list argptr;
  wchar_t string[512];

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  // make sure the character count is legal
  if (uiCharCount > wcslen(string)) {
    uiCharCount = wcslen(string);
  } else {
    if (uiCharCount < wcslen(string)) {
      // less than the full string, so whack off the end of it (it's temporary anyway)
      string[uiCharCount] = '\0';
    }
  }

  return (StringPixLength(string, usUseFont));
}

//*****************************************************************************
// StringPixLengthArg
//
// Returns the length of a string with a variable number of arguments, in
// pixels, using the current font. Maximum length in characters the string can
// evaluate to is 512.  Because this is for fast help text, all '|' characters are ignored for the
// width calculation.
// 'uiCharCount' specifies how many characters of the string are counted.
// YOU HAVE TO PREBUILD THE FAST HELP STRING!
//*****************************************************************************
int16_t StringPixLengthArgFastHelp(int32_t usUseFont, int32_t usBoldFont, uint32_t uiCharCount,
                                   wchar_t *pFontString) {
  wchar_t string[512];
  uint32_t i, index;
  int16_t sBoldDiff = 0;
  wchar_t str[2];

  Assert(pFontString != NULL);

  wcscpy(string, pFontString);

  // make sure the character count is legal
  if (uiCharCount > wcslen(string)) {
    uiCharCount = wcslen(string);
  } else {
    if (uiCharCount < wcslen(string)) {
      // less than the full string, so whack off the end of it (it's temporary anyway)
      string[uiCharCount] = '\0';
    }
  }
  // now eliminate all '|' characters from the string.
  i = 0;
  while (i < uiCharCount) {
    if (string[i] == '|') {
      for (index = i; index < uiCharCount; index++) {
        string[index] = string[index + 1];
      }
      uiCharCount--;
      // now we have eliminated the '|' character, so now calculate the size difference of the
      // bolded character.
      str[0] = string[i];
      str[1] = 0;
      sBoldDiff += StringPixLength(str, usBoldFont) - StringPixLength(str, usUseFont);
    }
    i++;
  }
  return StringPixLength(string, usUseFont) + sBoldDiff;
}

//*****************************************************************************************
//
//  StringNPixLength
//
//  Return the length of the of the string or count characters in the
//  string, which ever comes first.
//
//  Returns int16_t
//
//  Created by:     Gilles Beauparlant
//  Created on:     12/1/99
//
//*****************************************************************************************
int16_t StringNPixLength(wchar_t *string, uint32_t uiMaxCount, int32_t UseFont) {
  uint32_t Cur, uiCharCount;
  wchar_t *curletter;

  Cur = 0;
  uiCharCount = 0;
  curletter = string;

  while ((*curletter) != L'\0' && uiCharCount < uiMaxCount) {
    Cur += GetCharWidth_(FontObjs[UseFont], *curletter++);
    uiCharCount++;
  }
  return ((int16_t)Cur);
}

//*****************************************************************************
//
// StringPixLength
//
//	Returns the length of a string in pixels, depending on the font given.
//
//*****************************************************************************
int16_t StringPixLength(wchar_t *string, int32_t UseFont) {
  uint32_t Cur;
  wchar_t *curletter;

  if (string == NULL) {
    return (0);
  }

  Cur = 0;
  curletter = string;

  while ((*curletter) != L'\0') {
    Cur += GetCharWidth_(FontObjs[UseFont], *curletter++);
  }
  return ((int16_t)Cur);
}

//*****************************************************************************
//
// SaveFontSettings
//
//	Saves the current font printing settings into temporary locations.
//
//*****************************************************************************
void SaveFontSettings(void) {
  SaveFontDefault = FontDefault;
  SaveFontDestBuffer = FontDestBuffer;
  SaveFontDestPitch = FontDestPitch;
  SaveFontDestBPP = FontDestBPP;
  SaveFontDestRegion = FontDestRegion;
  SaveFontDestWrap = FontDestWrap;
  SaveFontForeground16 = FontForeground16;
  SaveFontShadow16 = FontShadow16;
  SaveFontBackground16 = FontBackground16;
  SaveFontForeground8 = FontForeground8;
  SaveFontBackground8 = FontBackground8;
}

//*****************************************************************************
//
// RestoreFontSettings
//
//	Restores the last saved font printing settings from the temporary lactions
//
//*****************************************************************************
void RestoreFontSettings(void) {
  FontDefault = SaveFontDefault;
  FontDestBuffer = SaveFontDestBuffer;
  FontDestPitch = SaveFontDestPitch;
  FontDestBPP = SaveFontDestBPP;
  FontDestRegion = SaveFontDestRegion;
  FontDestWrap = SaveFontDestWrap;
  FontForeground16 = SaveFontForeground16;
  FontShadow16 = SaveFontShadow16;
  FontBackground16 = SaveFontBackground16;
  FontForeground8 = SaveFontForeground8;
  FontBackground8 = SaveFontBackground8;
}

//*****************************************************************************
// GetHeight
//
//	Returns the height of a given character in the font.
//
//*****************************************************************************
uint32_t GetHeight(struct VObject *hSrcVObject, int16_t ssIndex) {
  ETRLEObject *pTrav;

  // Assertions
  Assert(hSrcVObject != NULL);

  // Get Offsets from Index into structure
  pTrav = &(hSrcVObject->pETRLEObject[ssIndex]);
  return ((uint32_t)(pTrav->usHeight + pTrav->sOffsetY));
}

//*****************************************************************************
//
// GetFontHeight
//
//	Returns the height of the first character in a font.
//
//*****************************************************************************
uint16_t GetFontHeight(int32_t FontNum) {
  Assert(FontNum >= 0);
  Assert(FontNum <= MAX_FONTS);
  Assert(FontObjs[FontNum] != NULL);

  return ((uint16_t)GetHeight(FontObjs[FontNum], 0));
}

/* Given a wide char, this function returns the index of the glyph. If no glyph
 * exists for the requested wide char, the glyph index of '?' is returned. */
static uint8_t GetIndex(wchar_t const c) {
  if (c == GetZeroGlyphChar()) {
    return 0;
  }
  if (c < TranslationTableSize) {
    uint8_t const idx = TranslationTable[c];
    if (idx != 0) return idx;
  }
  // DebugMsg(TOPIC_FONT_HANDLER, DBG_LEVEL_0, String("Error: Invalid character given U+%04X", c));
  return TranslationTable[L'?'];
}

uint16_t GetCharWidth_(struct VObject *font, wchar_t c) { return GetWidth(font, GetIndex(c)); }

//*****************************************************************************
// SetFont
//
//	Sets the current font number.
//
//*****************************************************************************
BOOLEAN SetFont(int32_t iFontIndex) {
  Assert(iFontIndex >= 0);
  Assert(iFontIndex <= MAX_FONTS);
  Assert(FontObjs[iFontIndex] != NULL);

  FontDefault = iFontIndex;
  return (TRUE);
}

//*****************************************************************************
// SetFontDestBuffer
//
//	Sets the destination buffer for printing to, the clipping rectangle, and
// sets the line wrap on/off. DestBuffer is a VOBJECT handle, not a pointer.
//
//*****************************************************************************
BOOLEAN SetFontDestBuffer(uint32_t DestBuffer, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                          BOOLEAN wrap) {
  Assert(x2 > x1);
  Assert(y2 > y1);

  FontDestBuffer = DestBuffer;

  FontDestRegion.iLeft = x1;
  FontDestRegion.iTop = y1;
  FontDestRegion.iRight = x2;
  FontDestRegion.iBottom = y2;
  FontDestWrap = wrap;

  return (TRUE);
}

//*****************************************************************************
// mprintf
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters. Uses monochrome font color settings
//*****************************************************************************
uint32_t mprintf(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  // Lock the dest buffer
  pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  UnLockVideoSurface(FontDestBuffer);

  return (0);
}

void VarFindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                 int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                 wchar_t *pFontString, ...) {
  wchar_t string[512];
  va_list argptr;

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontRightCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}

void VarFindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                                  int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY,
                                  wchar_t *pFontString, ...) {
  wchar_t string[512];
  va_list argptr;

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontCenterCoordinates(sLeft, sTop, sWidth, sHeight, string, iFontIndex, psNewX, psNewY);
}

void FindFontRightCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                              wchar_t *pStr, int32_t iFontIndex, int16_t *psNewX, int16_t *psNewY) {
  int16_t xp, yp;

  // Compute the coordinates to right justify the text
  xp = ((sWidth - StringPixLength(pStr, iFontIndex))) + sLeft;
  yp = ((sHeight - GetFontHeight(iFontIndex)) / 2) + sTop;

  *psNewX = xp;
  *psNewY = yp;
}

void FindFontCenterCoordinates(int16_t sLeft, int16_t sTop, int16_t sWidth, int16_t sHeight,
                               wchar_t *pStr, int32_t iFontIndex, int16_t *psNewX,
                               int16_t *psNewY) {
  int16_t xp, yp;

  // Compute the coordinates to center the text
  xp = ((sWidth - StringPixLength(pStr, iFontIndex) + 1) / 2) + sLeft;
  yp = ((sHeight - GetFontHeight(iFontIndex)) / 2) + sTop;

  *psNewX = xp;
  *psNewY = yp;
}

//*****************************************************************************
// gprintf
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters.
//*****************************************************************************
uint32_t gprintf(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  // Lock the dest buffer
  pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  UnLockVideoSurface(FontDestBuffer);

  return (0);
}

uint32_t gprintfDirty(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  // Lock the dest buffer
  pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  UnLockVideoSurface(FontDestBuffer);

  InvalidateRegion(x, y, x + StringPixLength(string, FontDefault), y + GetFontHeight(FontDefault));

  return (0);
}
//*****************************************************************************
// gprintf_buffer
//
//	Prints to the currently selected destination buffer, at the X/Y coordinates
// specified, using the currently selected font. Other than the X/Y coordinates,
// the parameters are identical to printf. The resulting string may be no longer
// than 512 word-characters.
//*****************************************************************************
uint32_t gprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                        int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  while ((*curletter) != 0) {
    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontType], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontType], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES,
                                            FontObjs[FontDefault], destx, desty, transletter,
                                            &FontDestRegion);

    destx += GetWidth(FontObjs[FontType], transletter);
  }

  return (0);
}

uint32_t mprintf_buffer(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType, int32_t x,
                        int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  while ((*curletter) != 0) {
    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  return (0);
}

uint32_t mprintf_buffer_coded(uint8_t *pDestBuf, uint32_t uiDestPitchBYTES, uint32_t FontType,
                              int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];
  uint16_t usOldForeColor;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  usOldForeColor = FontForeground16;

  while ((*curletter) != 0) {
    if ((*curletter) == 180) {
      curletter++;
      SetFontForeground((uint8_t)(*curletter));
      curletter++;
    } else if ((*curletter) == 181) {
      FontForeground16 = usOldForeColor;
      curletter++;
    }

    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  return (0);
}

uint32_t mprintf_coded(int32_t x, int32_t y, wchar_t *pFontString, ...) {
  int32_t destx, desty;
  wchar_t *curletter;
  va_list argptr;
  wchar_t string[512];
  uint16_t usOldForeColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  Assert(pFontString != NULL);

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(string, ARR_SIZE(string), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  curletter = string;

  destx = x;
  desty = y;

  usOldForeColor = FontForeground16;

  // Lock the dest buffer
  pDestBuf = LockVideoSurface(FontDestBuffer, &uiDestPitchBYTES);

  while ((*curletter) != 0) {
    if ((*curletter) == 180) {
      curletter++;
      SetFontForeground((uint8_t)(*curletter));
      curletter++;
    } else if ((*curletter) == 181) {
      FontForeground16 = usOldForeColor;
      curletter++;
    }

    uint8_t transletter = GetIndex(*curletter++);

    if (FontDestWrap &&
        BltIsClipped(FontObjs[FontDefault], destx, desty, transletter, &FontDestRegion)) {
      destx = x;
      desty += GetHeight(FontObjs[FontDefault], transletter);
    }

    // Blit directly
    Blt8BPPDataTo16BPPBufferMonoShadowClip(
        (uint16_t *)pDestBuf, uiDestPitchBYTES, FontObjs[FontDefault], destx, desty, transletter,
        &FontDestRegion, FontForeground16, FontBackground16, FontShadow16);
    destx += GetWidth(FontObjs[FontDefault], transletter);
  }

  // Unlock buffer
  UnLockVideoSurface(FontDestBuffer);

  return (0);
}

//*****************************************************************************
// InitializeFontManager
//
//	Starts up the font manager system with the appropriate translation table.
//
//*****************************************************************************
BOOLEAN InitializeFontManager(uint16_t usDefaultPixelDepth) {
  int count;
  uint16_t uiRight, uiBottom;

  FontDefault = (-1);
  FontDestBuffer = BACKBUFFER;
  FontDestPitch = 0;

  GetCurrentVideoSettings(&uiRight, &uiBottom);
  FontDestRegion.iLeft = 0;
  FontDestRegion.iTop = 0;
  FontDestRegion.iRight = (int32_t)uiRight;
  FontDestRegion.iBottom = (int32_t)uiBottom;
  FontDestBPP = 16;

  FontDestWrap = FALSE;

  RegisterDebugTopic(TOPIC_FONT_HANDLER, "Font Manager");

  // Mark all font slots as empty
  for (count = 0; count < MAX_FONTS; count++) FontObjs[count] = NULL;

  return TRUE;
}

//*****************************************************************************
// ShutdownFontManager
//
//	Shuts down, and deallocates all fonts.
//*****************************************************************************
void ShutdownFontManager(void) {
  int32_t count;

  UnRegisterDebugTopic(TOPIC_FONT_HANDLER, "Font Manager");

  for (count = 0; count < MAX_FONTS; count++) {
    if (FontObjs[count] != NULL) UnloadFont(count);
  }
}
