// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/MercTextBox.h"

#include <string.h>

#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define TEXT_POPUP_WINDOW_TEXT_OFFSET_X 8
#define TEXT_POPUP_WINDOW_TEXT_OFFSET_Y 8
#define TEXT_POPUP_STRING_WIDTH 296
#define TEXT_POPUP_GAP_BN_LINES 10
#define TEXT_POPUP_FONT FONT12ARIAL
#define TEXT_POPUP_COLOR FONT_MCOLOR_WHITE

#define MERC_TEXT_FONT FONT12ARIAL
#define MERC_TEXT_COLOR FONT_MCOLOR_WHITE

#define MERC_TEXT_MIN_WIDTH 10
#define MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X 10
#define MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y 10

#define MERC_BACKGROUND_WIDTH 350
#define MERC_BACKGROUND_HEIGHT 200

// the max number of pop up boxes availiable to user
#define MAX_NUMBER_OF_POPUP_BOXES 10

// attempt to add box to pop up box list
int32_t AddPopUpBoxToList(MercPopUpBox *pPopUpTextBox);

// grab box with this id value
MercPopUpBox *GetPopUpBoxIndex(int32_t iId);

// both of the below are index by the enum for thier types - background and border in
// MercTextBox.h

// filenames for border popup .sti's
char *zMercBorderPopupFilenames[] = {
    "INTERFACE\\TactPopUp.sti",     "INTERFACE\\TactRedPopUp.sti", "INTERFACE\\TactBluePopUp.sti",
    "INTERFACE\\TactPopUpMain.sti", "INTERFACE\\LaptopPopup.sti",

};

// filenames for background popup .pcx's
char *zMercBackgroundPopupFilenames[] = {
    "INTERFACE\\TactPopupBackground.pcx",     "INTERFACE\\TactPopupWhiteBackground.pcx",
    "INTERFACE\\TactPopupGreyBackground.pcx", "INTERFACE\\TactPopupBackgroundMain.pcx",
    "INTERFACE\\LaptopPopupBackground.pcx",   "INTERFACE\\imp_popup_background.pcx",
};

// the pop up box structure
MercPopUpBox gBasicPopUpTextBox;

// the current pop up box
MercPopUpBox *gPopUpTextBox = NULL;

// the old one
MercPopUpBox *gOldPopUpTextBox = NULL;

// the list of boxes
MercPopUpBox *gpPopUpBoxList[MAX_NUMBER_OF_POPUP_BOXES];

// the flags
uint32_t guiFlags = 0;
uint32_t guiBoxIcons;
uint32_t guiSkullIcons;

BOOLEAN SetCurrentPopUpBox(uint32_t uiId) {
  // given id of the box, find it in the list and set to current

  // make sure the box id is valid
  if (uiId == (uint32_t)-1) {
    // ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Error: Trying to set Current Popup Box using
    // -1 as an ID" );
    return (FALSE);
  }

  // see if box inited
  if (gpPopUpBoxList[uiId] != NULL) {
    gPopUpTextBox = gpPopUpBoxList[uiId];
    return (TRUE);
  }
  return (FALSE);
}

BOOLEAN OverrideMercPopupBox(MercPopUpBox *pMercBox) {
  // store old box and set current this passed one
  gOldPopUpTextBox = gPopUpTextBox;

  gPopUpTextBox = pMercBox;

  return (TRUE);
}

BOOLEAN ResetOverrideMercPopupBox() {
  gPopUpTextBox = gOldPopUpTextBox;

  return (TRUE);
}

BOOLEAN InitMercPopupBox() {
  int32_t iCounter = 0;

  // init the pop up box list
  for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
    // set ptr to null
    gpPopUpBoxList[iCounter] = NULL;
  }

  // LOAD STOP ICON...
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\msgboxicons.sti"), &guiBoxIcons))
    AssertMsg(0, "Missing INTERFACE\\msgboxicons.sti");

  // LOAD SKULL ICON...
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\msgboxiconskull.sti"), &guiSkullIcons))
    AssertMsg(0, "Missing INTERFACE\\msgboxiconskull.sti");

  return (TRUE);
}

BOOLEAN ShutDownPopUpBoxes() {
  int32_t iCounter = 0;
  for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
    // now attempt to remove this box
    RemoveMercPopupBoxFromIndex(iCounter);
  }

  return (TRUE);
}

// Pass in the background index, and pointers to the font and shadow color
void GetMercPopupBoxFontColor(uint8_t ubBackgroundIndex, uint8_t *pubFontColor,
                              uint8_t *pubFontShadowColor);

// Tactical Popup
BOOLEAN LoadTextMercPopupImages(uint8_t ubBackgroundIndex, uint8_t ubBorderIndex) {
  // this function will load the graphics associated with the background and border index values

  // the background
  gPopUpTextBox->vsMercTextPopUpBackground =
      CreateVSurfaceFromFile(zMercBackgroundPopupFilenames[ubBackgroundIndex]);
  if (gPopUpTextBox->vsMercTextPopUpBackground == NULL) {
    return FALSE;
  }
  JSurface_SetColorKey(gPopUpTextBox->vsMercTextPopUpBackground, FROMRGB(0, 0, 0));

  // border
  CHECKF(AddVObject(CreateVObjectFromFile(zMercBorderPopupFilenames[ubBorderIndex]),
                    &gPopUpTextBox->uiMercTextPopUpBorder));

  gPopUpTextBox->fMercTextPopupInitialized = TRUE;

  // so far so good, return successful
  gPopUpTextBox->ubBackgroundIndex = ubBackgroundIndex;
  gPopUpTextBox->ubBorderIndex = ubBorderIndex;

  return (TRUE);
}

void RemoveTextMercPopupImages() {
  // this procedure will remove the background and border video surface/object from the indecies
  if (gPopUpTextBox) {
    if (gPopUpTextBox->fMercTextPopupInitialized) {
      // the background
      JSurface_Free(gPopUpTextBox->vsMercTextPopUpBackground);
      gPopUpTextBox->vsMercTextPopUpBackground = NULL;

      // the border
      DeleteVideoObjectFromIndex(gPopUpTextBox->uiMercTextPopUpBorder);

      gPopUpTextBox->fMercTextPopupInitialized = FALSE;
    }
  }

  // done
  return;
}

BOOLEAN RenderMercPopUpBoxFromIndex(int32_t iBoxId, int16_t sDestX, int16_t sDestY,
                                    struct JSurface *dest) {
  // set the current box
  if (SetCurrentPopUpBox(iBoxId) == FALSE) {
    return (FALSE);
  }

  // now attempt to render the box
  return (RenderMercPopupBox(sDestX, sDestY, dest));
}

BOOLEAN RenderMercPopupBox(int16_t sDestX, int16_t sDestY, struct JSurface *dest) {
  // will render/transfer the image from the buffer in the data structure to the buffer specified by
  // user
  BOOLEAN fReturnValue = TRUE;

  // check to see if we are wanting to blit a transparent background
  if (gPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_TRANS_BACK)
    BltVSurfaceToVSurfaceFast(dest, gPopUpTextBox->sourceBuffer, sDestX, sDestY);
  else
    BltVSurfaceToVSurfaceFast(dest, gPopUpTextBox->sourceBuffer, sDestX, sDestY);

  // blt, and grab return value
  //	fReturnValue = Blt16BPPTo16BPP(pDestBuf, uiDestPitchBYTES, pSrcBuf, uiSrcPitchBYTES, sDestX,
  // sDestY, 0, 0, gPopUpTextBox->sWidth, gPopUpTextBox->sHeight);

  // Invalidate!
  if (dest == vsFB) {
    InvalidateRegion(sDestX, sDestY, (int16_t)(sDestX + gPopUpTextBox->sWidth),
                     (int16_t)(sDestY + gPopUpTextBox->sHeight));
  }

  // return success or failure
  return fReturnValue;
}

int32_t AddPopUpBoxToList(MercPopUpBox *pPopUpTextBox) {
  int32_t iCounter = 0;

  // make sure is a valid box
  if (pPopUpTextBox == NULL) {
    return (-1);
  }

  // attempt to add box to list
  for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
    if (gpPopUpBoxList[iCounter] == NULL) {
      // found a spot, inset
      gpPopUpBoxList[iCounter] = pPopUpTextBox;

      // set as current
      SetCurrentPopUpBox(iCounter);

      // return index value
      return (iCounter);
    }
  }

  // return failure
  return (-1);
}

// get box with this id
MercPopUpBox *GetPopUpBoxIndex(int32_t iId) { return (gpPopUpBoxList[iId]); }

int32_t PrepareMercPopupBox(int32_t iBoxId, uint8_t ubBackgroundIndex, uint8_t ubBorderIndex,
                            wchar_t *pString, uint16_t usWidth, uint16_t usMarginX,
                            uint16_t usMarginTopY, uint16_t usMarginBottomY, uint16_t *pActualWidth,
                            uint16_t *pActualHeight) {
  uint16_t usNumberVerticalPixels;
  uint16_t usTextWidth, usHeight;
  uint16_t i;
  struct VObject *hImageHandle;
  uint16_t usPosY, usPosX;
  uint16_t usStringPixLength;
  SGPRect DestRect;
  uint32_t uiDestPitchBYTES;
  uint32_t uiSrcPitchBYTES;
  uint16_t *pDestBuf;
  uint8_t *pSrcBuf;
  uint8_t ubFontColor, ubFontShadowColor;
  uint16_t usColorVal;
  uint16_t usLoopEnd;
  int16_t sDispTextXPos;
  MercPopUpBox *pPopUpTextBox = NULL;

  if (usWidth >= 640) return (-1);

  if (usWidth <= MERC_TEXT_MIN_WIDTH) usWidth = MERC_TEXT_MIN_WIDTH;

  // check id value, if -1, box has not been inited yet
  if (iBoxId == -1) {
    // no box yet

    // create box
    pPopUpTextBox = (MercPopUpBox *)MemAlloc(sizeof(MercPopUpBox));

    // copy over ptr
    gPopUpTextBox = pPopUpTextBox;

    // Load appropriate images
    if (LoadTextMercPopupImages(ubBackgroundIndex, ubBorderIndex) == FALSE) {
      MemFree(pPopUpTextBox);
      return (-1);
    }

  } else {
    // has been created already,
    // Check if these images are different

    // grab box
    pPopUpTextBox = GetPopUpBoxIndex(iBoxId);

    // box has valid id and no instance?..error
    Assert(pPopUpTextBox);

    // copy over ptr
    gPopUpTextBox = pPopUpTextBox;

    if (ubBackgroundIndex != pPopUpTextBox->ubBackgroundIndex ||
        ubBorderIndex != pPopUpTextBox->ubBorderIndex ||
        !pPopUpTextBox->fMercTextPopupInitialized) {
      // Remove old, set new
      RemoveTextMercPopupImages();
      if (LoadTextMercPopupImages(ubBackgroundIndex, ubBorderIndex) == FALSE) {
        return (-1);
      }
    }
  }

  gPopUpTextBox->uiFlags = guiFlags;
  // reset flags
  guiFlags = 0;

  usStringPixLength = WFStringPixLength(pString, TEXT_POPUP_FONT);

  if (usStringPixLength < (usWidth - (MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X) * 2)) {
    usWidth = usStringPixLength + MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2;
    usTextWidth = usWidth - (MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X) * 2 + 1;
  } else {
    usTextWidth = usWidth - (MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X) * 2 + 1 - usMarginX;
  }

  usNumberVerticalPixels =
      IanWrappedStringHeight(0, 0, usTextWidth, 2, TEXT_POPUP_FONT, MERC_TEXT_COLOR, pString,
                             FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  usHeight = usNumberVerticalPixels + MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X * 2;

  // Add height for margins
  usHeight += usMarginTopY + usMarginBottomY;

  // Add width for margins
  usWidth += (usMarginX * 2);

  // Add width for iconic...
  if ((pPopUpTextBox->uiFlags &
       (MERC_POPUP_PREPARE_FLAGS_STOPICON | MERC_POPUP_PREPARE_FLAGS_SKULLICON))) {
    // Make minimun height for box...
    if (usHeight < 45) {
      usHeight = 45;
    }
    usWidth += 35;
  }

  if (usWidth >= MERC_BACKGROUND_WIDTH) usWidth = MERC_BACKGROUND_WIDTH - 1;
  // make sure the area isnt bigger then the background texture
  if ((usWidth >= MERC_BACKGROUND_WIDTH) || usHeight >= MERC_BACKGROUND_HEIGHT) {
    if (iBoxId == -1) {
      MemFree(pPopUpTextBox);
    }

    return (-1);
  }
  // Create a background video surface to blt the face onto
  pPopUpTextBox->sourceBuffer = JSurface_Create16bpp(usWidth, usHeight);
  pPopUpTextBox->fMercTextPopupSurfaceInitialized = TRUE;

  pPopUpTextBox->sWidth = usWidth;
  pPopUpTextBox->sHeight = usHeight;

  *pActualWidth = usWidth;
  *pActualHeight = usHeight;

  DestRect.iLeft = 0;
  DestRect.iTop = 0;
  DestRect.iRight = DestRect.iLeft + usWidth;
  DestRect.iBottom = DestRect.iTop + usHeight;

  if (pPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_TRANS_BACK) {
    // Zero with yellow,
    // Set source transparcenty
    JSurface_SetColorKey(pPopUpTextBox->sourceBuffer, FROMRGB(255, 255, 0));

    pDestBuf = (uint16_t *)LockVSurface(pPopUpTextBox->sourceBuffer, &uiDestPitchBYTES);

    usColorVal = rgb32_to_rgb16(FROMRGB(255, 255, 0));
    usLoopEnd = (usWidth * usHeight);

    for (i = 0; i < usLoopEnd; i++) {
      pDestBuf[i] = usColorVal;
    }

    JSurface_Unlock(pPopUpTextBox->sourceBuffer);

  } else {
    if (pPopUpTextBox->vsMercTextPopUpBackground == NULL) {
      AssertMsg(0, String("Failed to find VSurface: %d", pPopUpTextBox->vsMercTextPopUpBackground));
    }

    pDestBuf = (uint16_t *)LockVSurface(pPopUpTextBox->sourceBuffer, &uiDestPitchBYTES);
    pSrcBuf = LockVSurface(pPopUpTextBox->vsMercTextPopUpBackground, &uiSrcPitchBYTES);

    Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES,
                                pPopUpTextBox->vsMercTextPopUpBackground, pSrcBuf, uiSrcPitchBYTES,
                                0, 0, &DestRect);

    JSurface_Unlock(pPopUpTextBox->vsMercTextPopUpBackground);
    JSurface_Unlock(pPopUpTextBox->sourceBuffer);
  }

  GetVideoObject(&hImageHandle, pPopUpTextBox->uiMercTextPopUpBorder);

  usPosX = usPosY = 0;
  // blit top row of images
  for (i = TEXT_POPUP_GAP_BN_LINES; i < usWidth - TEXT_POPUP_GAP_BN_LINES;
       i += TEXT_POPUP_GAP_BN_LINES) {
    // TOP ROW
    BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 1, i, usPosY);
    // BOTTOM ROW
    BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 6, i,
                   usHeight - TEXT_POPUP_GAP_BN_LINES + 6);
  }

  // blit the left and right row of images
  usPosX = 0;
  for (i = TEXT_POPUP_GAP_BN_LINES; i < usHeight - TEXT_POPUP_GAP_BN_LINES;
       i += TEXT_POPUP_GAP_BN_LINES) {
    BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 3, usPosX, i);
    BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 4, usPosX + usWidth - 4, i);
  }

  // blt the corner images for the row
  // top left
  BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 0, 0, usPosY);
  // top right
  BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 2, usWidth - TEXT_POPUP_GAP_BN_LINES,
                 usPosY);
  // bottom left
  BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 5, 0,
                 usHeight - TEXT_POPUP_GAP_BN_LINES);
  // bottom right
  BltVideoObject(pPopUpTextBox->sourceBuffer, hImageHandle, 7, usWidth - TEXT_POPUP_GAP_BN_LINES,
                 usHeight - TEXT_POPUP_GAP_BN_LINES);

  // Icon if ness....
  if (pPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_STOPICON) {
    BltVObjectFromIndex(pPopUpTextBox->sourceBuffer, guiBoxIcons, 0, 5, 4);
  }
  if (pPopUpTextBox->uiFlags & MERC_POPUP_PREPARE_FLAGS_SKULLICON) {
    BltVObjectFromIndex(pPopUpTextBox->sourceBuffer, guiSkullIcons, 0, 9, 4);
  }

  // Get the font and shadow colors
  GetMercPopupBoxFontColor(ubBackgroundIndex, &ubFontColor, &ubFontShadowColor);

  SetFontShadow(ubFontShadowColor);
  SetFontDestBuffer(pPopUpTextBox->sourceBuffer, 0, 0, usWidth, usHeight, FALSE);

  // Display the text
  sDispTextXPos = (int16_t)((MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_X + usMarginX));

  if (pPopUpTextBox->uiFlags &
      (MERC_POPUP_PREPARE_FLAGS_STOPICON | MERC_POPUP_PREPARE_FLAGS_SKULLICON)) {
    sDispTextXPos += 30;
  }

// if language represents words with a single char
#ifdef TAIWAN
  {
    // Enable the use of single word wordwrap
    if (gfUseWinFonts) {
      UseSingleCharWordsForWordWrap(TRUE);
    }

    // Display the text
    DisplayWrappedString(sDispTextXPos,
                         (int16_t)((MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y + usMarginTopY)),
                         usTextWidth, 2, MERC_TEXT_FONT, ubFontColor, pString, FONT_MCOLOR_BLACK,
                         FALSE, LEFT_JUSTIFIED);

    // Disable the use of single word wordwrap
    UseSingleCharWordsForWordWrap(FALSE);
  }
#else
  {
    // Display the text
    DisplayWrappedString(sDispTextXPos,
                         (int16_t)((MERC_TEXT_POPUP_WINDOW_TEXT_OFFSET_Y + usMarginTopY)),
                         usTextWidth, 2, MERC_TEXT_FONT, ubFontColor, pString, FONT_MCOLOR_BLACK,
                         FALSE, LEFT_JUSTIFIED);
  }
#endif

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
  SetFontShadow(DEFAULT_SHADOW);

  if (iBoxId == -1) {
    // now return attemp to add to pop up box list, if successful will return index
    return (AddPopUpBoxToList(pPopUpTextBox));
  } else {
    // set as current box
    SetCurrentPopUpBox(iBoxId);

    return (iBoxId);
  }
}

// Deletes the surface thats contains the border, background and the text.
BOOLEAN RemoveMercPopupBox() {
  int32_t iCounter = 0;

  // make sure the current box does in fact exist
  if (gPopUpTextBox == NULL) {
    // failed..
    return (FALSE);
  }

  // now check to see if inited...
  if (gPopUpTextBox->fMercTextPopupSurfaceInitialized) {
    // now find this box in the list
    for (iCounter = 0; iCounter < MAX_NUMBER_OF_POPUP_BOXES; iCounter++) {
      if (gpPopUpBoxList[iCounter] == gPopUpTextBox) {
        gpPopUpBoxList[iCounter] = NULL;
        iCounter = MAX_NUMBER_OF_POPUP_BOXES;
      }
    }
    // yep, get rid of the bloody...
    JSurface_Free(gPopUpTextBox->sourceBuffer);

    // DEF Added 5/26
    // Delete the background and the border
    RemoveTextMercPopupImages();

    MemFree(gPopUpTextBox);

    // reset current ptr
    gPopUpTextBox = NULL;
  }

  return (TRUE);
}

BOOLEAN RemoveMercPopupBoxFromIndex(uint32_t uiId) {
  // find this box, set it to current, and delete it
  if (SetCurrentPopUpBox(uiId) == FALSE) {
    // failed
    return (FALSE);
  }

  // now try to remove it
  return (RemoveMercPopupBox());
}

// Pass in the background index, and pointers to the font and shadow color
void GetMercPopupBoxFontColor(uint8_t ubBackgroundIndex, uint8_t *pubFontColor,
                              uint8_t *pubFontShadowColor) {
  switch (ubBackgroundIndex) {
    case BASIC_MERC_POPUP_BACKGROUND:
      *pubFontColor = TEXT_POPUP_COLOR;
      *pubFontShadowColor = DEFAULT_SHADOW;
      break;

    case WHITE_MERC_POPUP_BACKGROUND:
      *pubFontColor = 2;
      *pubFontShadowColor = FONT_MCOLOR_WHITE;
      break;

    case GREY_MERC_POPUP_BACKGROUND:
      *pubFontColor = 2;
      *pubFontShadowColor = NO_SHADOW;
      break;

    case LAPTOP_POPUP_BACKGROUND:
      *pubFontColor = TEXT_POPUP_COLOR;
      *pubFontShadowColor = DEFAULT_SHADOW;
      break;

    default:
      *pubFontColor = TEXT_POPUP_COLOR;
      *pubFontShadowColor = DEFAULT_SHADOW;
      break;
  }
}

BOOLEAN SetPrepareMercPopupFlags(uint32_t uiFlags) {
  guiFlags |= uiFlags;
  return (TRUE);
}

BOOLEAN SetPrepareMercPopUpFlagsFromIndex(uint32_t uiFlags, uint32_t uiId) {
  // find this box, set it to current, and delete it
  if (SetCurrentPopUpBox(uiId) == FALSE) {
    // failed
    return (FALSE);
  }

  // now try to remove it
  return (SetPrepareMercPopupFlags(uiFlags));
}
