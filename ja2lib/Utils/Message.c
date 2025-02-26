// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/Message.h"

#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

#include "BuildDefines.h"
#include "JAScreens.h"
#include "Local.h"
#include "SGP/FileMan.h"
#include "SGP/Font.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "ScreenIDs.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/SoundControl.h"
#include "Utils/TimerControl.h"
#include "Utils/WordWrap.h"

// #include "mbstring.h"

typedef struct {
  uint32_t uiFont;
  uint32_t uiTimeOfLastUpdate;
  uint32_t uiFlags;
  uint32_t uiPadding[3];
  uint16_t usColor;
  BOOLEAN fBeginningOfNewString;

} StringSaveStruct;

#define MAX_LINE_COUNT 6
#define X_START 2
#define Y_START 330
#define MAX_AGE 10000
#define LINE_WIDTH 320
#define MAP_LINE_WIDTH 300
#define WIDTH_BETWEEN_NEW_STRINGS 5

#define BETAVERSION_COLOR FONT_ORANGE
#define TESTVERSION_COLOR FONT_GREEN
#define DEBUG_COLOR FONT_RED
#define DIALOGUE_COLOR FONT_WHITE
#define INTERFACE_COLOR FONT_YELLOW

#define MAP_SCREEN_MESSAGE_FONT TINYFONT1

uint8_t gubStartOfMapScreenMessageList = 0;
uint8_t gubEndOfMapScreenMessageList = 0;

// index of the current string we are looking at
uint8_t gubCurrentMapMessageString = 0;

// temp position for display of marker
// uint8_t ubTempPosition = 0;

// are allowed to beep on message scroll?
BOOLEAN fOkToBeepNewMessage = TRUE;

static ScrollStringStPtr gpDisplayList[MAX_LINE_COUNT];
static ScrollStringStPtr gMapScreenMessageList[256];
ScrollStringStPtr pStringS = NULL;

// first time adding any message to the message dialogue system
BOOLEAN fFirstTimeInMessageSystem = TRUE;
BOOLEAN fDisableJustForIan = FALSE;

BOOLEAN fScrollMessagesHidden = FALSE;
uint32_t uiStartOfPauseTime = 0;

// test extern functions
BOOLEAN RestoreExternBackgroundRectGivenID(int32_t iBack);
extern VIDEO_OVERLAY gVideoOverlays[];

extern uint16_t gusSubtitleBoxWidth;
extern BOOLEAN gfFacePanelActive;

// region created and due to last quote box
extern BOOLEAN fTextBoxMouseRegionCreated;
extern BOOLEAN fDialogueBoxDueToLastMessage;

// prototypes

BOOLEAN CreateStringVideoOverlay(ScrollStringStPtr pStringSt, uint16_t usX, uint16_t usY);
void SetStringVideoOverlayPosition(ScrollStringStPtr pStringSt, uint16_t usX, uint16_t usY);

void BlitString(VIDEO_OVERLAY *pBlitter);
void RemoveStringVideoOverlay(ScrollStringStPtr pStringSt);
void EnableStringVideoOverlay(ScrollStringStPtr pStringSt, BOOLEAN fEnable);

ScrollStringStPtr GetNextString(ScrollStringStPtr pStringSt);
ScrollStringStPtr GetPrevString(ScrollStringStPtr pStringSt);
void AlignString(ScrollStringStPtr pPermStringSt);

int32_t GetMessageQueueSize(void);

ScrollStringStPtr AddString(wchar_t *string, uint16_t usColor, uint32_t uiFont,
                            BOOLEAN fStartOfNewString, uint8_t ubPriority);
void SetString(ScrollStringStPtr pStringSt, wchar_t *String);

void SetStringPosition(ScrollStringStPtr pStringSt, uint16_t x, uint16_t y);
void SetStringColor(ScrollStringStPtr pStringSt, uint16_t color);
ScrollStringStPtr SetStringNext(ScrollStringStPtr pStringSt, ScrollStringStPtr pNext);
ScrollStringStPtr SetStringPrev(ScrollStringStPtr pStringSt, ScrollStringStPtr pPrev);
void AddStringToMapScreenMessageList(wchar_t *pString, uint16_t usColor, uint32_t uiFont,
                                     BOOLEAN fStartOfNewString, uint8_t ubPriority);

// clear up a linked list of wrapped strings
void ClearWrappedStrings(WRAPPED_STRING *pStringWrapperHead);
void WriteMessageToFile(wchar_t *pString);

// tactical screen message
void TacticalScreenMsg(uint16_t usColor, uint8_t ubPriority, wchar_t *pStringA, ...);

// play bee when new message is added
void PlayNewMessageSound(void);

void HandleLastQuotePopUpTimer(void);

// functions

void SetStringFont(ScrollStringStPtr pStringSt, uint32_t uiFont) { pStringSt->uiFont = uiFont; }

uint32_t GetStringFont(ScrollStringStPtr pStringSt) { return pStringSt->uiFont; }

ScrollStringStPtr AddString(wchar_t *pString, uint16_t usColor, uint32_t uiFont,
                            BOOLEAN fStartOfNewString, uint8_t ubPriority) {
  // add a new string to the list of strings
  ScrollStringStPtr pStringSt = NULL;
  pStringSt = (ScrollStringSt *)MemAlloc(sizeof(ScrollStringSt));

  SetString(pStringSt, pString);
  SetStringColor(pStringSt, usColor);
  pStringSt->uiFont = uiFont;
  pStringSt->fBeginningOfNewString = fStartOfNewString;
  pStringSt->uiFlags = ubPriority;

  SetStringNext(pStringSt, NULL);
  SetStringPrev(pStringSt, NULL);
  pStringSt->iVideoOverlay = -1;

  // now add string to map screen strings
  // AddStringToMapScreenMessageList(pString, usColor, uiFont, fStartOfNewString, ubPriority );

  return (pStringSt);
}

void SetString(ScrollStringStPtr pStringSt, wchar_t *pString) {
  // ARM: Why x2 + 4 ???
  pStringSt->pString16 = (wchar_t *)MemAlloc((wcslen(pString) * 2) + 4);
  wcsncpy(pStringSt->pString16, pString, wcslen(pString));
  pStringSt->pString16[wcslen(pString)] = 0;
}

void SetStringPosition(ScrollStringStPtr pStringSt, uint16_t usX, uint16_t usY) {
  SetStringVideoOverlayPosition(pStringSt, usX, usY);
}

void SetStringColor(ScrollStringStPtr pStringSt, uint16_t usColor) { pStringSt->usColor = usColor; }

ScrollStringStPtr GetNextString(ScrollStringStPtr pStringSt) {
  // returns pointer to next string line
  if (pStringSt == NULL)
    return NULL;
  else
    return pStringSt->pNext;
}

ScrollStringStPtr GetPrevString(ScrollStringStPtr pStringSt) {
  // returns pointer to previous string line
  if (pStringSt == NULL)
    return NULL;
  else
    return pStringSt->pPrev;
}

ScrollStringStPtr SetStringNext(ScrollStringStPtr pStringSt, ScrollStringStPtr pNext) {
  pStringSt->pNext = pNext;
  return pStringSt;
}

ScrollStringStPtr SetStringPrev(ScrollStringStPtr pStringSt, ScrollStringStPtr pPrev) {
  pStringSt->pPrev = pPrev;
  return pStringSt;
}

BOOLEAN CreateStringVideoOverlay(ScrollStringStPtr pStringSt, uint16_t usX, uint16_t usY) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  // SET VIDEO OVERLAY
  VideoOverlayDesc.sLeft = usX;
  VideoOverlayDesc.sTop = usY;
  VideoOverlayDesc.uiFontID = pStringSt->uiFont;
  VideoOverlayDesc.ubFontBack = FONT_MCOLOR_BLACK;
  VideoOverlayDesc.ubFontFore = (unsigned char)pStringSt->usColor;
  VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
  VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
  swprintf(VideoOverlayDesc.pzText, ARR_SIZE(VideoOverlayDesc.pzText), pStringSt->pString16);
  VideoOverlayDesc.BltCallback = BlitString;
  pStringSt->iVideoOverlay = RegisterVideoOverlay((VOVERLAY_DIRTYBYTEXT), &VideoOverlayDesc);

  if (pStringSt->iVideoOverlay == -1) {
    return (FALSE);
  }

  return (TRUE);
}

void RemoveStringVideoOverlay(ScrollStringStPtr pStringSt) {
  // error check, remove one not there
  if (pStringSt->iVideoOverlay == -1) {
    return;
  }

  RemoveVideoOverlay(pStringSt->iVideoOverlay);
  pStringSt->iVideoOverlay = -1;
}

void SetStringVideoOverlayPosition(ScrollStringStPtr pStringSt, uint16_t usX, uint16_t usY) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  // Donot update if not allocated!
  if (pStringSt->iVideoOverlay != -1) {
    VideoOverlayDesc.uiFlags = VOVERLAY_DESC_POSITION;
    VideoOverlayDesc.sLeft = usX;
    VideoOverlayDesc.sTop = usY;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    UpdateVideoOverlay(&VideoOverlayDesc, pStringSt->iVideoOverlay, FALSE);
  }
}

void BlitString(VIDEO_OVERLAY *pBlitter) {
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;

  // gprintfdirty(pBlitter->sX,pBlitter->sY, pBlitter->zText);
  // RestoreExternBackgroundRect(pBlitter->sX,pBlitter->sY,
  // pBlitter->sX+StringPixLength(pBlitter->zText,pBlitter->uiFontID ),
  // pBlitter->sY+GetFontHeight(pBlitter->uiFontID ));

  if (fScrollMessagesHidden == TRUE) {
    return;
  }

  pDestBuf = LockVSurface(pBlitter->vsDestBuff, &uiDestPitchBYTES);
  SetFont(pBlitter->uiFontID);

  SetFontBackground(pBlitter->ubFontBack);
  SetFontForeground(pBlitter->ubFontFore);
  SetFontShadow(DEFAULT_SHADOW);
  mprintf_buffer_coded(pDestBuf, uiDestPitchBYTES, pBlitter->uiFontID, pBlitter->sX, pBlitter->sY,
                       pBlitter->zText);
  JSurface_Unlock(pBlitter->vsDestBuff);
}

void EnableStringVideoOverlay(ScrollStringStPtr pStringSt, BOOLEAN fEnable) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  if (pStringSt->iVideoOverlay != -1) {
    VideoOverlayDesc.fDisabled = !fEnable;
    VideoOverlayDesc.uiFlags = VOVERLAY_DESC_DISABLED;
    UpdateVideoOverlay(&VideoOverlayDesc, pStringSt->iVideoOverlay, FALSE);
  }
}

void ClearDisplayedListOfTacticalStrings(void) {
  // this function will go through list of display strings and clear them all out
  uint32_t cnt;

  for (cnt = 0; cnt < MAX_LINE_COUNT; cnt++) {
    if (gpDisplayList[cnt] != NULL) {
      // CHECK IF WE HAVE AGED

      // Remove our sorry ass
      RemoveStringVideoOverlay(gpDisplayList[cnt]);
      MemFree(gpDisplayList[cnt]->pString16);
      MemFree(gpDisplayList[cnt]);

      // Free slot
      gpDisplayList[cnt] = NULL;
    }
  }

  return;
}

void ScrollString() {
  uint32_t suiTimer = 0;
  uint32_t cnt;
  int32_t iNumberOfNewStrings = 0;  // the count of new strings, so we can update position by
                                    // WIDTH_BETWEEN_NEW_STRINGS pixels in the y
  int32_t iNumberOfMessagesOnQueue = 0;
  int32_t iMaxAge = 0;
  BOOLEAN fDitchLastMessage = FALSE;

  // UPDATE TIMER
  suiTimer = GetJA2Clock();

  // might have pop up text timer
  HandleLastQuotePopUpTimer();

  if (IsMapScreen_2()) {
    return;
  }

  // DONOT UPDATE IF WE ARE SCROLLING!
  if (gfScrollPending || gfScrollInertia) {
    return;
  }

  // messages hidden
  if (fScrollMessagesHidden) {
    return;
  }

  iNumberOfMessagesOnQueue = GetMessageQueueSize();
  iMaxAge = MAX_AGE;

  if ((iNumberOfMessagesOnQueue > 0) && (gpDisplayList[MAX_LINE_COUNT - 1] != NULL)) {
    fDitchLastMessage = TRUE;
  } else {
    fDitchLastMessage = FALSE;
  }

  if ((iNumberOfMessagesOnQueue * 1000) >= iMaxAge) {
    iNumberOfMessagesOnQueue = (iMaxAge / 1000);
  } else if (iNumberOfMessagesOnQueue < 0) {
    iNumberOfMessagesOnQueue = 0;
  }

  // AGE
  for (cnt = 0; cnt < MAX_LINE_COUNT; cnt++) {
    if (gpDisplayList[cnt] != NULL) {
      if ((fDitchLastMessage) && (cnt == MAX_LINE_COUNT - 1)) {
        gpDisplayList[cnt]->uiTimeOfLastUpdate = iMaxAge;
      }
      // CHECK IF WE HAVE AGED
      if ((suiTimer - gpDisplayList[cnt]->uiTimeOfLastUpdate) >
          (uint32_t)(iMaxAge - (1000 * iNumberOfMessagesOnQueue))) {
        // Remove our sorry ass
        RemoveStringVideoOverlay(gpDisplayList[cnt]);
        MemFree(gpDisplayList[cnt]->pString16);
        MemFree(gpDisplayList[cnt]);

        // Free slot
        gpDisplayList[cnt] = NULL;
      }
    }
  }

  // CHECK FOR FREE SPOTS AND ADD ANY STRINGS IF WE HAVE SOME TO ADD!

  // FIRST CHECK IF WE HAVE ANY IN OUR QUEUE
  if (pStringS != NULL) {
    // CHECK IF WE HAVE A SLOT!
    // CHECK OUR LAST SLOT!
    if (gpDisplayList[MAX_LINE_COUNT - 1] == NULL) {
      // MOVE ALL UP!

      // cpy, then move
      for (cnt = MAX_LINE_COUNT - 1; cnt > 0; cnt--) {
        gpDisplayList[cnt] = gpDisplayList[cnt - 1];
      }

      // now add in the new string
      cnt = 0;
      gpDisplayList[cnt] = pStringS;
      CreateStringVideoOverlay(pStringS, X_START, Y_START);
      if (pStringS->fBeginningOfNewString == TRUE) {
        iNumberOfNewStrings++;
      }

      // set up age
      pStringS->uiTimeOfLastUpdate = GetJA2Clock();

      // now move
      for (cnt = 0; cnt <= MAX_LINE_COUNT - 1; cnt++) {
        // Adjust position!
        if (gpDisplayList[cnt] != NULL) {
          SetStringVideoOverlayPosition(
              gpDisplayList[cnt], X_START,
              (int16_t)((Y_START - ((cnt)*GetFontHeight(SMALLFONT1))) -
                        (int16_t)(WIDTH_BETWEEN_NEW_STRINGS * (iNumberOfNewStrings))));

          // start of new string, increment count of new strings, for spacing purposes
          if (gpDisplayList[cnt]->fBeginningOfNewString == TRUE) {
            iNumberOfNewStrings++;
          }
        }
      }

      // WE NOW HAVE A FREE SPACE, INSERT!

      // Adjust head!
      pStringS = pStringS->pNext;
      if (pStringS) {
        pStringS->pPrev = NULL;
      }

      // check if new meesage we have not seen since mapscreen..if so, beep
      if ((fOkToBeepNewMessage == TRUE) && (gpDisplayList[MAX_LINE_COUNT - 2] == NULL) &&
          ((IsTacticalMode()) || (IsMapScreen_2())) && (gfFacePanelActive == FALSE)) {
        PlayNewMessageSound();
      }
    }
  }
}

void DisableScrollMessages(void) {
  // will stop the scroll of messages in tactical and hide them during an NPC's dialogue
  // disble video overlay for tatcitcal scroll messages
  EnableDisableScrollStringVideoOverlay(FALSE);
  return;
}

void EnableScrollMessages(void) {
  EnableDisableScrollStringVideoOverlay(TRUE);
  return;
}

void HideMessagesDuringNPCDialogue(void) {
  // will stop the scroll of messages in tactical and hide them during an NPC's dialogue
  int32_t cnt;

  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  VideoOverlayDesc.fDisabled = TRUE;
  VideoOverlayDesc.uiFlags = VOVERLAY_DESC_DISABLED;

  fScrollMessagesHidden = TRUE;
  uiStartOfPauseTime = GetJA2Clock();

  for (cnt = 0; cnt < MAX_LINE_COUNT; cnt++) {
    if (gpDisplayList[cnt] != NULL) {
      RestoreExternBackgroundRectGivenID(
          gVideoOverlays[gpDisplayList[cnt]->iVideoOverlay].uiBackground);
      UpdateVideoOverlay(&VideoOverlayDesc, gpDisplayList[cnt]->iVideoOverlay, FALSE);
    }
  }

  return;
}

void UnHideMessagesDuringNPCDialogue(void) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  int32_t cnt = 0;

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  VideoOverlayDesc.fDisabled = FALSE;
  VideoOverlayDesc.uiFlags = VOVERLAY_DESC_DISABLED;
  fScrollMessagesHidden = FALSE;

  for (cnt = 0; cnt < MAX_LINE_COUNT; cnt++) {
    if (gpDisplayList[cnt] != NULL) {
      gpDisplayList[cnt]->uiTimeOfLastUpdate += GetJA2Clock() - uiStartOfPauseTime;
      UpdateVideoOverlay(&VideoOverlayDesc, gpDisplayList[cnt]->iVideoOverlay, FALSE);
    }
  }

  return;
}

// new screen message
void ScreenMsg(uint16_t usColor, uint8_t ubPriority, wchar_t *pStringA, ...) {
  wchar_t DestString[512];
  va_list argptr;

  if (fDisableJustForIan == TRUE) {
    if (ubPriority == MSG_BETAVERSION) {
      return;
    } else if (ubPriority == MSG_TESTVERSION) {
      return;
    } else if (ubPriority == MSG_DEBUG) {
      return;
    }
  }

  if (ubPriority == MSG_DEBUG) {
    usColor = DEBUG_COLOR;
#ifndef _DEBUG
    return;
#endif
  }

  if (ubPriority == MSG_BETAVERSION) {
    usColor = BETAVERSION_COLOR;
#ifndef JA2BETAVERSION
#ifndef JA2TESTVERSION
    return;
#endif
#endif
  }

  if (ubPriority == MSG_TESTVERSION) {
    usColor = TESTVERSION_COLOR;

#ifndef JA2TESTVERSION
    return;
#endif
  }

  va_start(argptr, pStringA);
  vswprintf(DestString, ARR_SIZE(DestString), pStringA, argptr);
  va_end(argptr);

  // pass onto tactical message and mapscreen message
  TacticalScreenMsg(usColor, ubPriority, DestString);

  //	if( ( ubPriority != MSG_DEBUG ) && ( ubPriority != MSG_TESTVERSION ) )
  { MapScreenMessage(usColor, ubPriority, DestString); }

  if (IsMapScreen_2()) {
    PlayNewMessageSound();
  } else {
    fOkToBeepNewMessage = TRUE;
  }

  return;
}

void ClearWrappedStrings(WRAPPED_STRING *pStringWrapperHead) {
  WRAPPED_STRING *pNode = pStringWrapperHead;
  WRAPPED_STRING *pDeleteNode = NULL;
  // clear out a link list of wrapped string structures

  // error check, is there a node to delete?
  if (pNode == NULL) {
    // leave,
    return;
  }

  do {
    // set delete node as current node
    pDeleteNode = pNode;

    // set current node as next node
    pNode = pNode->pNextWrappedString;

    // delete the string
    MemFree(pDeleteNode->sString);
    pDeleteNode->sString = NULL;

    // clear out delete node
    MemFree(pDeleteNode);
    pDeleteNode = NULL;

  } while (pNode);

  //	MemFree( pNode );

  pStringWrapperHead = NULL;
}

// new tactical and mapscreen message system
void TacticalScreenMsg(uint16_t usColor, uint8_t ubPriority, wchar_t *pStringA, ...) {
  // this function sets up the string into several single line structures

  ScrollStringStPtr pStringSt;
  uint32_t uiFont = TINYFONT1;
  va_list argptr;

  wchar_t DestString[512], DestStringA[512];
  ScrollStringStPtr pTempStringSt = NULL;
  WRAPPED_STRING *pStringWrapper = NULL;
  WRAPPED_STRING *pStringWrapperHead = NULL;
  BOOLEAN fNewString = FALSE;
  uint16_t usLineWidthIfWordIsWiderThenWidth = 0;

  if (giTimeCompressMode > TIME_COMPRESS_X1) {
    return;
  }

  if (fDisableJustForIan == TRUE && ubPriority != MSG_ERROR && ubPriority != MSG_INTERFACE) {
    return;
  }

  if (ubPriority == MSG_BETAVERSION) {
    usColor = BETAVERSION_COLOR;
#ifndef JA2BETAVERSION
#ifndef JA2TESTVERSION
    return;
#endif
#endif
    WriteMessageToFile(DestString);
  }

  if (ubPriority == MSG_TESTVERSION) {
    usColor = TESTVERSION_COLOR;

#ifndef JA2TESTVERSION
    return;
#endif

    WriteMessageToFile(DestString);
  }

  if (fFirstTimeInMessageSystem) {
    // Init display array!
    memset(gpDisplayList, 0, sizeof(gpDisplayList));
    fFirstTimeInMessageSystem = FALSE;
  }

  pStringSt = pStringS;
  while (GetNextString(pStringSt)) pStringSt = GetNextString(pStringSt);

  va_start(argptr, pStringA);  // Set up variable argument pointer
  vswprintf(DestString, ARR_SIZE(DestString), pStringA,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  if (ubPriority == MSG_DEBUG) {
#ifndef _DEBUG
    return;
#endif
    usColor = DEBUG_COLOR;
    wcscpy(DestStringA, DestString);
    swprintf(DestString, ARR_SIZE(DestString), L"Debug: %s", DestStringA);
    WriteMessageToFile(DestStringA);
  }

  if (ubPriority == MSG_DIALOG) {
    usColor = DIALOGUE_COLOR;
  }

  if (ubPriority == MSG_INTERFACE) {
    usColor = INTERFACE_COLOR;
  }

  pStringWrapperHead = LineWrap(uiFont, LINE_WIDTH, &usLineWidthIfWordIsWiderThenWidth, DestString);
  pStringWrapper = pStringWrapperHead;
  if (!pStringWrapper) return;

  fNewString = TRUE;
  while (pStringWrapper->pNextWrappedString != NULL) {
    if (!pStringSt) {
      pStringSt = AddString(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority);
      fNewString = FALSE;
      pStringSt->pNext = NULL;
      pStringSt->pPrev = NULL;
      pStringS = pStringSt;
    } else {
      pTempStringSt = AddString(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority);
      fNewString = FALSE;
      pTempStringSt->pPrev = pStringSt;
      pStringSt->pNext = pTempStringSt;
      pStringSt = pTempStringSt;
      pTempStringSt->pNext = NULL;
    }
    pStringWrapper = pStringWrapper->pNextWrappedString;
  }
  pTempStringSt = AddString(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority);
  if (pStringSt) {
    pStringSt->pNext = pTempStringSt;
    pTempStringSt->pPrev = pStringSt;
    pStringSt = pTempStringSt;
    pStringSt->pNext = NULL;
  } else {
    pStringSt = pTempStringSt;
    pStringSt->pNext = NULL;
    pStringSt->pPrev = NULL;
    pStringS = pStringSt;
  }

  // clear up list of wrapped strings
  ClearWrappedStrings(pStringWrapperHead);

  return;
}

void MapScreenMessage(uint16_t usColor, uint8_t ubPriority, wchar_t *pStringA, ...) {
  // this function sets up the string into several single line structures

  ScrollStringStPtr pStringSt;
  uint32_t uiFont = MAP_SCREEN_MESSAGE_FONT;
  va_list argptr;
  wchar_t DestString[512], DestStringA[512];
  WRAPPED_STRING *pStringWrapper = NULL;
  WRAPPED_STRING *pStringWrapperHead = NULL;
  BOOLEAN fNewString = FALSE;
  uint16_t usLineWidthIfWordIsWiderThenWidth;

  if (fDisableJustForIan == TRUE) {
    if (ubPriority == MSG_BETAVERSION) {
      return;
    } else if (ubPriority == MSG_TESTVERSION) {
      return;
    } else if (ubPriority == MSG_DEBUG) {
      return;
    }
  }

  if (ubPriority == MSG_BETAVERSION) {
    usColor = BETAVERSION_COLOR;
#ifndef JA2BETAVERSION
#ifndef JA2TESTVERSION
    return;
#endif
#endif

    WriteMessageToFile(DestString);
  }

  if (ubPriority == MSG_TESTVERSION) {
    usColor = TESTVERSION_COLOR;

#ifndef JA2TESTVERSION
    return;
#endif
    WriteMessageToFile(DestString);
  }
  // OK, check if we are ani imeediate feedback message, if so, do something else!
  if (ubPriority == MSG_UI_FEEDBACK) {
    va_start(argptr, pStringA);  // Set up variable argument pointer
    vswprintf(DestString, ARR_SIZE(DestString), pStringA,
              argptr);  // process gprintf string (get output str)
    va_end(argptr);

    BeginUIMessage(DestString);
    return;
  }

  if (ubPriority == MSG_SKULL_UI_FEEDBACK) {
    va_start(argptr, pStringA);  // Set up variable argument pointer
    vswprintf(DestString, ARR_SIZE(DestString), pStringA,
              argptr);  // process gprintf string (get output str)
    va_end(argptr);

    InternalBeginUIMessage(TRUE, DestString);
    return;
  }

  // check if error
  if (ubPriority == MSG_ERROR) {
    va_start(argptr, pStringA);  // Set up variable argument pointer
    vswprintf(DestString, ARR_SIZE(DestString), pStringA,
              argptr);  // process gprintf string (get output str)
    va_end(argptr);

    swprintf(DestStringA, ARR_SIZE(DestStringA), L"DEBUG: %s", DestString);

    BeginUIMessage(DestStringA);
    WriteMessageToFile(DestStringA);

    return;
  }

  // OK, check if we are an immediate MAP feedback message, if so, do something else!
  if ((ubPriority == MSG_MAP_UI_POSITION_UPPER) || (ubPriority == MSG_MAP_UI_POSITION_MIDDLE) ||
      (ubPriority == MSG_MAP_UI_POSITION_LOWER)) {
    va_start(argptr, pStringA);  // Set up variable argument pointer
    vswprintf(DestString, ARR_SIZE(DestString), pStringA,
              argptr);  // process gprintf string (get output str)
    va_end(argptr);

    BeginMapUIMessage(ubPriority, DestString);
    return;
  }

  if (fFirstTimeInMessageSystem) {
    // Init display array!
    memset(gpDisplayList, 0, sizeof(gpDisplayList));
    fFirstTimeInMessageSystem = FALSE;
  }

  pStringSt = pStringS;
  while (GetNextString(pStringSt)) pStringSt = GetNextString(pStringSt);

  va_start(argptr, pStringA);  // Set up variable argument pointer
  vswprintf(DestString, ARR_SIZE(DestString), pStringA,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  if (ubPriority == MSG_DEBUG) {
#ifndef _DEBUG
    return;
#endif
    usColor = DEBUG_COLOR;
    wcscpy(DestStringA, DestString);
    swprintf(DestString, ARR_SIZE(DestString), L"Debug: %s", DestStringA);
  }

  if (ubPriority == MSG_DIALOG) {
    usColor = DIALOGUE_COLOR;
  }

  if (ubPriority == MSG_INTERFACE) {
    usColor = INTERFACE_COLOR;
  }

  pStringWrapperHead =
      LineWrap(uiFont, MAP_LINE_WIDTH, &usLineWidthIfWordIsWiderThenWidth, DestString);
  pStringWrapper = pStringWrapperHead;
  if (!pStringWrapper) return;

  fNewString = TRUE;

  while (pStringWrapper->pNextWrappedString != NULL) {
    AddStringToMapScreenMessageList(pStringWrapper->sString, usColor, uiFont, fNewString,
                                    ubPriority);
    fNewString = FALSE;

    pStringWrapper = pStringWrapper->pNextWrappedString;
  }

  AddStringToMapScreenMessageList(pStringWrapper->sString, usColor, uiFont, fNewString, ubPriority);

  // clear up list of wrapped strings
  ClearWrappedStrings(pStringWrapperHead);

  // play new message beep
  // PlayNewMessageSound( );

  MoveToEndOfMapScreenMessageList();
}

// add string to the map screen message list
void AddStringToMapScreenMessageList(wchar_t *pString, uint16_t usColor, uint32_t uiFont,
                                     BOOLEAN fStartOfNewString, uint8_t ubPriority) {
  ScrollStringStPtr pStringSt = NULL;

  pStringSt = (ScrollStringSt *)MemAlloc(sizeof(ScrollStringSt));

  SetString(pStringSt, pString);
  SetStringColor(pStringSt, usColor);
  pStringSt->uiFont = uiFont;
  pStringSt->fBeginningOfNewString = fStartOfNewString;
  pStringSt->uiFlags = ubPriority;
  pStringSt->iVideoOverlay = -1;

  // next/previous are not used, it's strictly a wraparound queue
  SetStringNext(pStringSt, NULL);
  SetStringPrev(pStringSt, NULL);

  // Figure out which queue slot index we're going to use to store this
  // If queue isn't full, this is easy, if is is full, we'll re-use the oldest slot
  // Must always keep the wraparound in mind, although this is easy enough with a static, fixed-size
  // queue.

  // always store the new message at the END index

  // check if slot is being used, if so, clear it up
  if (gMapScreenMessageList[gubEndOfMapScreenMessageList] != NULL) {
    MemFree(gMapScreenMessageList[gubEndOfMapScreenMessageList]->pString16);
    MemFree(gMapScreenMessageList[gubEndOfMapScreenMessageList]);
  }

  // store the new message there
  gMapScreenMessageList[gubEndOfMapScreenMessageList] = pStringSt;

  // increment the end
  gubEndOfMapScreenMessageList = (gubEndOfMapScreenMessageList + 1) % 256;

  // if queue is full, end will now match the start
  if (gubEndOfMapScreenMessageList == gubStartOfMapScreenMessageList) {
    // if that's so, increment the start
    gubStartOfMapScreenMessageList = (gubStartOfMapScreenMessageList + 1) % 256;
  }
}

void DisplayStringsInMapScreenMessageList(void) {
  uint8_t ubCurrentStringIndex;
  uint8_t ubLinesPrinted;
  int16_t sY;
  uint16_t usSpacing;

  SetFontDestBuffer(vsFB, 17, 360 + 6, 407, 360 + 101, FALSE);

  SetFont(MAP_SCREEN_MESSAGE_FONT);  // no longer supports variable fonts
  SetFontBackground(FONT_BLACK);
  SetFontShadow(DEFAULT_SHADOW);

  ubCurrentStringIndex = gubCurrentMapMessageString;

  sY = 377;
  usSpacing = GetFontHeight(MAP_SCREEN_MESSAGE_FONT);

  for (ubLinesPrinted = 0; ubLinesPrinted < MAX_MESSAGES_ON_MAP_BOTTOM; ubLinesPrinted++) {
    // reached the end of the list?
    if (ubCurrentStringIndex == gubEndOfMapScreenMessageList) {
      break;
    }

    // nothing stored there?
    if (gMapScreenMessageList[ubCurrentStringIndex] == NULL) {
      break;
    }

    // set font color
    SetFontForeground((uint8_t)(gMapScreenMessageList[ubCurrentStringIndex]->usColor));

    // print this line
    mprintf_coded(20, sY, gMapScreenMessageList[ubCurrentStringIndex]->pString16);

    sY += usSpacing;

    // next message index to print (may wrap around)
    ubCurrentStringIndex = (ubCurrentStringIndex + 1) % 256;
  }

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
}

void EnableDisableScrollStringVideoOverlay(BOOLEAN fEnable) {
  // will go through the list of video overlays for the tactical scroll message system, and
  // enable/disable video overlays depending on fEnable
  int8_t bCounter = 0;

  for (bCounter = 0; bCounter < MAX_LINE_COUNT; bCounter++) {
    // if valid, enable/disable
    if (gpDisplayList[bCounter] != NULL) {
      EnableVideoOverlay(fEnable, gpDisplayList[bCounter]->iVideoOverlay);
    }
  }

  return;
}

void PlayNewMessageSound(void) {
  // play a new message sound, if there is one playing, do nothing
  static uint32_t uiSoundId = NO_SAMPLE;

  if (uiSoundId != NO_SAMPLE) {
    // is sound playing?..don't play new one
    if (SoundIsPlaying(uiSoundId) == TRUE) {
      return;
    }
  }

  // otherwise no sound playing, play one
  uiSoundId = PlayJA2SampleFromFile("Sounds\\newbeep.wav", RATE_11025, MIDVOLUME, 1, MIDDLEPAN);

  return;
}

BOOLEAN SaveMapScreenMessagesToSaveGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten;
  uint32_t uiCount;
  uint32_t uiSizeOfString;
  StringSaveStruct StringSave;

  //	write to the begining of the message list
  FileMan_Write(hFile, &gubEndOfMapScreenMessageList, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) {
    return (FALSE);
  }

  FileMan_Write(hFile, &gubStartOfMapScreenMessageList, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) {
    return (FALSE);
  }

  //	write the current message string
  FileMan_Write(hFile, &gubCurrentMapMessageString, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) {
    return (FALSE);
  }

  // Loopthrough all the messages
  for (uiCount = 0; uiCount < 256; uiCount++) {
    if (gMapScreenMessageList[uiCount]) {
      uiSizeOfString = (wcslen(gMapScreenMessageList[uiCount]->pString16) + 1) * 2;
    } else
      uiSizeOfString = 0;

    //	write to the file the size of the message
    FileMan_Write(hFile, &uiSizeOfString, sizeof(uint32_t), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(uint32_t)) {
      return (FALSE);
    }

    // if there is a message
    if (uiSizeOfString) {
      //	write the message to the file
      FileMan_Write(hFile, gMapScreenMessageList[uiCount]->pString16, uiSizeOfString,
                    &uiNumBytesWritten);
      if (uiNumBytesWritten != uiSizeOfString) {
        return (FALSE);
      }

      // Create  the saved string struct
      StringSave.uiFont = gMapScreenMessageList[uiCount]->uiFont;
      StringSave.usColor = gMapScreenMessageList[uiCount]->usColor;
      StringSave.fBeginningOfNewString = gMapScreenMessageList[uiCount]->fBeginningOfNewString;
      StringSave.uiTimeOfLastUpdate = gMapScreenMessageList[uiCount]->uiTimeOfLastUpdate;
      StringSave.uiFlags = gMapScreenMessageList[uiCount]->uiFlags;

      // Write the rest of the message information to the saved game file
      FileMan_Write(hFile, &StringSave, sizeof(StringSaveStruct), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(StringSaveStruct)) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadMapScreenMessagesFromSaveGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead;
  uint32_t uiCount;
  uint32_t uiSizeOfString;
  StringSaveStruct StringSave;
  wchar_t SavedString[512];

  // clear tactical message queue
  ClearTacticalMessageQueue();

  gubEndOfMapScreenMessageList = 0;
  gubStartOfMapScreenMessageList = 0;
  gubCurrentMapMessageString = 0;

  //	Read to the begining of the message list
  FileMan_Read(hFile, &gubEndOfMapScreenMessageList, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) {
    return (FALSE);
  }

  //	Read the current message string
  FileMan_Read(hFile, &gubStartOfMapScreenMessageList, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) {
    return (FALSE);
  }

  //	Read the current message string
  FileMan_Read(hFile, &gubCurrentMapMessageString, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) {
    return (FALSE);
  }

  // Loopthrough all the messages
  for (uiCount = 0; uiCount < 256; uiCount++) {
    //	Read to the file the size of the message
    FileMan_Read(hFile, &uiSizeOfString, sizeof(uint32_t), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(uint32_t)) {
      return (FALSE);
    }

    // if there is a message
    if (uiSizeOfString) {
      //	Read the message from the file
      FileMan_Read(hFile, SavedString, uiSizeOfString, &uiNumBytesRead);
      if (uiNumBytesRead != uiSizeOfString) {
        return (FALSE);
      }

      // if there is an existing string,delete it
      if (gMapScreenMessageList[uiCount]) {
        if (gMapScreenMessageList[uiCount]->pString16) {
          MemFree(gMapScreenMessageList[uiCount]->pString16);
          gMapScreenMessageList[uiCount]->pString16 = NULL;
        }
      } else {
        // There is now message here, add one
        ScrollStringSt *sScroll;

        sScroll = (ScrollStringSt *)MemAlloc(sizeof(ScrollStringSt));
        if (sScroll == NULL) return (FALSE);

        memset(sScroll, 0, sizeof(ScrollStringSt));

        gMapScreenMessageList[uiCount] = sScroll;
      }

      // allocate space for the new string
      gMapScreenMessageList[uiCount]->pString16 = (wchar_t *)MemAlloc(uiSizeOfString);
      if (gMapScreenMessageList[uiCount]->pString16 == NULL) return (FALSE);

      memset(gMapScreenMessageList[uiCount]->pString16, 0, uiSizeOfString);

      // copy the string over
      wcscpy(gMapScreenMessageList[uiCount]->pString16, SavedString);

      // Read the rest of the message information to the saved game file
      FileMan_Read(hFile, &StringSave, sizeof(StringSaveStruct), &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(StringSaveStruct)) {
        return (FALSE);
      }

      // Create  the saved string struct
      gMapScreenMessageList[uiCount]->uiFont = StringSave.uiFont;
      gMapScreenMessageList[uiCount]->usColor = StringSave.usColor;
      gMapScreenMessageList[uiCount]->uiFlags = StringSave.uiFlags;
      gMapScreenMessageList[uiCount]->fBeginningOfNewString = StringSave.fBeginningOfNewString;
      gMapScreenMessageList[uiCount]->uiTimeOfLastUpdate = StringSave.uiTimeOfLastUpdate;
    } else
      gMapScreenMessageList[uiCount] = NULL;
  }

  // this will set a valid value for gubFirstMapscreenMessageIndex, which isn't being saved/restored
  MoveToEndOfMapScreenMessageList();

  return (TRUE);
}

void HandleLastQuotePopUpTimer(void) {
  if ((fTextBoxMouseRegionCreated == FALSE) || (fDialogueBoxDueToLastMessage == FALSE)) {
    return;
  }

  // check if timed out
  if (GetJA2Clock() - guiDialogueLastQuoteTime > guiDialogueLastQuoteDelay) {
    // done clear up
    ShutDownLastQuoteTacticalTextBox();
    guiDialogueLastQuoteTime = 0;
    guiDialogueLastQuoteDelay = 0;
  }
}

ScrollStringStPtr MoveToBeginningOfMessageQueue(void) {
  ScrollStringStPtr pStringSt = pStringS;

  if (pStringSt == NULL) {
    return (NULL);
  }

  while (pStringSt->pPrev) {
    pStringSt = pStringSt->pPrev;
  }

  return (pStringSt);
}

int32_t GetMessageQueueSize(void) {
  ScrollStringStPtr pStringSt = pStringS;
  int32_t iCounter = 0;

  pStringSt = MoveToBeginningOfMessageQueue();

  while (pStringSt) {
    iCounter++;
    pStringSt = pStringSt->pNext;
  }

  return (iCounter);
}

void ClearTacticalMessageQueue(void) {
  ScrollStringStPtr pStringSt = pStringS, pOtherStringSt = pStringS;

  ClearDisplayedListOfTacticalStrings();

  // now run through all the tactical messages
  while (pStringSt) {
    pOtherStringSt = pStringSt;
    pStringSt = pStringSt->pNext;
    MemFree(pOtherStringSt->pString16);
    MemFree(pOtherStringSt);
  }

  pStringS = NULL;

  return;
}

void WriteMessageToFile(wchar_t *pString) {
#ifdef JA2BETAVERSION

  FILE *fp;

  fp = fopen("DebugMessage.txt", "a");

  if (fp == NULL) {
    return;
  }

  fprintf(fp, "%S\n", pString);
  fclose(fp);

#endif
}

void InitGlobalMessageList(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    gMapScreenMessageList[iCounter] = NULL;
  }

  gubEndOfMapScreenMessageList = 0;
  gubStartOfMapScreenMessageList = 0;
  gubCurrentMapMessageString = 0;
  //	ubTempPosition = 0;

  return;
}

void FreeGlobalMessageList(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 256; iCounter++) {
    // check if next unit is empty, if not...clear it up
    if (gMapScreenMessageList[iCounter] != NULL) {
      MemFree(gMapScreenMessageList[iCounter]->pString16);
      MemFree(gMapScreenMessageList[iCounter]);
    }
  }

  InitGlobalMessageList();

  return;
}

uint8_t GetRangeOfMapScreenMessages(void) {
  uint8_t ubRange = 0;

  // NOTE: End is non-inclusive, so start/end 0/0 means no messages, 0/1 means 1 message, etc.
  if (gubStartOfMapScreenMessageList <= gubEndOfMapScreenMessageList) {
    ubRange = gubEndOfMapScreenMessageList - gubStartOfMapScreenMessageList;
  } else {
    // this should always be 255 now, since this only happens when queue fills up, and we never
    // remove any messages
    ubRange = (gubEndOfMapScreenMessageList + 256) - gubStartOfMapScreenMessageList;
  }

  return (ubRange);
}

/*
BOOLEAN IsThereAnEmptySlotInTheMapScreenMessageList( void )
{
        // find if there is an empty slot

        if( gMapScreenMessageList[ ( uint8_t )( gubEndOfMapScreenMessageList + 1 ) ] != NULL )
        {
                return ( FALSE );
        }
        else
        {
                return( TRUE );
        }
}


uint8_t GetFirstEmptySlotInTheMapScreenMessageList( void )
{
        uint8_t ubSlotId = 0;

        // find first empty slot in list

        if( IsThereAnEmptySlotInTheMapScreenMessageList(  ) == FALSE)
        {
                ubSlotId = gubEndOfMapScreenMessageList;
                return( ubSlotId );
        }
        else
        {
                // start at head of list
                ubSlotId = gubEndOfMapScreenMessageList;

                // run through list
                while( gMapScreenMessageList[ ubSlotId ] != NULL )
                {
                        ubSlotId++;
                }

        }
        return( ubSlotId );
}



void SetCurrentMapScreenMessageString( uint8_t ubCurrentStringPosition )
{
        // will attempt to set current string to this value, or the closest one
  uint8_t ubCounter = 0;

        if( gMapScreenMessageList[ ubCurrentStringPosition ] == NULL )
        {
                // no message here, run down to nearest
                ubCounter = ubCurrentStringPosition;
                ubCounter--;

                while(  ( gMapScreenMessageList[ ubCounter ] == NULL )&&( ubCounter !=
ubCurrentStringPosition ) )
                {
                        if( ubCounter == 0 )
                        {
                                ubCounter = 255;
                        }
                        else
                        {
                                ubCounter--;
                        }
                }

                ubCurrentStringPosition = ubCounter;

        }
        return;
}


uint8_t GetTheRelativePositionOfCurrentMessage( void )
{
        uint8_t ubPosition = 0;

        if( gubEndOfMapScreenMessageList > gubStartOfMapScreenMessageList)
        {
                ubPosition = gubCurrentMapMessageString - gubStartOfMapScreenMessageList;
        }
        else
        {
                ubPosition = ( 255 - gubStartOfMapScreenMessageList ) + gubCurrentMapMessageString;
        }


        return( ubPosition );
}




void MoveCurrentMessagePointerDownList( void )
{
        // check to see if we can move 'down' to newer messages?
        if( gMapScreenMessageList[ ( uint8_t )( gubCurrentMapMessageString  + 1 )  ] != NULL )
        {
                if(  ( uint8_t ) ( gubCurrentMapMessageString + 1 ) != gubEndOfMapScreenMessageList
)
                {
                        if( ( AreThereASetOfStringsAfterThisIndex( gubCurrentMapMessageString,
MAX_MESSAGES_ON_MAP_BOTTOM ) == TRUE ) )
                        {
                                gubCurrentMapMessageString++;
                        }
                }
        }
}


void MoveCurrentMessagePointerUpList(void )
{
                // check to see if we can move 'down' to newer messages?
        if( gMapScreenMessageList[ ( uint8_t )( gubCurrentMapMessageString  - 1 )  ] != NULL )
        {
                if( ( uint8_t ) ( gubCurrentMapMessageString - 1 ) != gubEndOfMapScreenMessageList )
                {
                        gubCurrentMapMessageString--;
                }
        }

}



void ScrollToHereInMapScreenMessageList( uint8_t ubPosition )
{
        // a position ranging from 0 to 255 where 0 is top and 255 is bottom
        // get the range of messages, * ubPosition /255 and set current to this position
        uint8_t ubTestPosition = gubCurrentMapMessageString;
        uint8_t ubRange = 0;

        ubRange = GetRangeOfMapScreenMessages( );

        if( ubRange > 1 )
        {
                ubRange += 9;
        }

        ubTestPosition = ( uint8_t )( gubEndOfMapScreenMessageList - ( uint8_t )(  ubRange  ) + (  (
( uint8_t )( ubRange )  * ubPosition ) / 256 ) );

        if( AreThereASetOfStringsAfterThisIndex( ubTestPosition, MAX_MESSAGES_ON_MAP_BOTTOM ) ==
TRUE )
        {
                gubCurrentMapMessageString = ubTestPosition;
        }

        ubTempPosition = ubTestPosition;

        return;
}


BOOLEAN AreThereASetOfStringsAfterThisIndex( uint8_t ubMsgIndex, int32_t iNumberOfStrings )
{
        int32_t iCounter;

        // go through this number of strings, if they pass, then we have at least iNumberOfStrings
after index ubMsgIndex for( iCounter = 0; iCounter < iNumberOfStrings; iCounter++ )
        {
                // start checking AFTER this index, so skip ahead to the next index BEFORE checking
                if( ubMsgIndex < 255 )
                {
                        ubMsgIndex++;
                }
                else
                {
                        ubMsgIndex = 0;
                }

                if( gMapScreenMessageList[ ubMsgIndex ] == NULL )
                {
                        return ( FALSE );
                }

                if( ubMsgIndex == gubEndOfMapScreenMessageList )
                {
                        return( FALSE );
                }
        }

        return( TRUE );
}



uint8_t GetCurrentMessageValue( void )
{
        // return the value of the current message in the list, relative to the start of the list

        if( GetRangeOfMapScreenMessages( ) >= 255  )
        {
          return( gubCurrentMapMessageString - gubStartOfMapScreenMessageList );
        }
        else
        {
                return( gubCurrentMapMessageString );
        }
}



uint8_t GetCurrentTempMessageValue( void )
{
        if( GetRangeOfMapScreenMessages( ) >= 255  )
        {
                return( ubTempPosition - gubEndOfMapScreenMessageList );
        }
        else
        {
                return( ubTempPosition );
        }
}


uint8_t GetNewMessageValueGivenPosition( uint8_t ubPosition )
{
        // if we were to scroll to this position, what would current message index value be?

        return( ( uint8_t )( ( gubEndOfMapScreenMessageList - ( uint8_t )(
GetRangeOfMapScreenMessages( ) ) ) + ( uint8_t )( ( GetRangeOfMapScreenMessages( ) * ubPosition ) /
255 ) ) );

}


BOOLEAN IsThisTheLastMessageInTheList( void )
{
        // is the current message the last message in the list?

        if( ( ( uint8_t )( gubCurrentMapMessageString + 1 ) ) == ( gubEndOfMapScreenMessageList ) &&
( GetRangeOfMapScreenMessages( ) < 255 ) )
        {
                return( TRUE );
        }
        else if( gMapScreenMessageList[ ( uint8_t )( gubCurrentMapMessageString + 1 ) ] == NULL )
        {
                return( TRUE );
        }
        else
        {
                if( AreThereASetOfStringsAfterThisIndex( gubCurrentMapMessageString,
MAX_MESSAGES_ON_MAP_BOTTOM ) == FALSE )
                {
                        return( TRUE );
                }
                else
                {
                        return ( FALSE );
                }
        }
}


BOOLEAN IsThisTheFirstMessageInTheList( void )
{
        // is the current message the first message in the list?

        if( ( gubCurrentMapMessageString ) == ( gubEndOfMapScreenMessageList ) )
        {
                return( TRUE );
        }
        else
        {
                return ( FALSE );
        }
}


void DisplayLastMessage( void )
{
        // start at end of list go back until message flag says dialogue
        uint8_t ubCounter = 0;
        BOOLEAN fNotDone = TRUE;
        BOOLEAN fFound = FALSE;
        BOOLEAN fSecondNewString = FALSE;
        wchar_t sString[ 256 ];

        sString[ 0 ] = 0;


        // set counter to end of list
        while( ( gMapScreenMessageList[ ( uint8_t )( ubCounter  + 1 )  ] != NULL ) && ( ( uint8_t )
( ubCounter + 1 ) != gubEndOfMapScreenMessageList ) )
        {
                ubCounter++;
        }

        // now start moving back until dialogue is found
        while( fNotDone )
        {
                if( ubCounter == gubEndOfMapScreenMessageList )
                {
                        fNotDone = FALSE;
                        fFound = FALSE;
                        continue;
                }

                if( gMapScreenMessageList[ ubCounter ] == NULL )
                {
                        fNotDone = FALSE;
                        fFound = FALSE;
                        continue;
                }
                // check if message if dialogue
                if( gMapScreenMessageList[ ubCounter ]->uiFlags == MSG_DIALOG )
                {
                        if( gMapScreenMessageList[ ubCounter ]-> fBeginningOfNewString == TRUE )
                        {
                                // yup
                                fNotDone = FALSE;
                                fFound = TRUE;

                                // now display message
                                continue;
                        }
                }

                ubCounter--;
        }

        if( fFound == TRUE )
        {
                fNotDone = TRUE;

                while( fNotDone )
                {
                        if( gMapScreenMessageList[ ubCounter ] )
                        {
                                if( ( fSecondNewString ) && ( gMapScreenMessageList[ ubCounter ] ->
fBeginningOfNewString ) )
                                {
                                        fNotDone = FALSE;
                                }
                                else if( gMapScreenMessageList[ ubCounter ]->uiFlags == MSG_DIALOG )
                                {
                                        wcscat( sString, gMapScreenMessageList[ ubCounter
]->pString16 ); wcscat( sString, L" " );
                                }

                                if( ( gMapScreenMessageList[ ubCounter ] ->  fBeginningOfNewString )
)
                                {
                                        fSecondNewString = TRUE;
                                }

                        }
                        else
                        {
                                fNotDone = FALSE;
                        }

                        // the next string
                        ubCounter++;
                }
                // execute text box
                ExecuteTacticalTextBoxForLastQuote( ( int16_t )( ( 640 - gusSubtitleBoxWidth ) / 2
), sString );
        }

        return;
}

*/
