// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/IMPFinish.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPCompileCharacter.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPPortraits.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/IMPVoices.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/SoundMan.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// min time btween frames of animation
#define ANIMATE_MIN_TIME 200

// buttons
int32_t giIMPFinishButton[6];
int32_t giIMPFinishButtonImage[6];

// we are in fact done
BOOLEAN fFinishedCharGeneration = FALSE;

// portrait position
int16_t sFaceX = 253;
int16_t sFaceY = 245;

// what voice are we playing?
uint32_t uiVoiceSound = 0;

// image handle
uint32_t guiCHARACTERPORTRAIT;
extern int32_t iCurrentPortrait;
extern int32_t iCurrentVoices;

// function definitions
void CreateIMPFinishButtons(void);
void DeleteIMPFinishButtons(void);
BOOLEAN RenderCharProfileFinishFace(void);
void RenderCharFullName(void);
BOOLEAN LoadCharacterPortrait(void);
void DestroyCharacterPortrait(void);

// callbacks
void BtnIMPFinishVoiceCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPFinishAttributesCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPFinishPersonalityCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPFinishStartOverCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPFinishDoneCallback(GUI_BUTTON *btn, int32_t reason);
void BtnIMPFinishPortraitCallback(GUI_BUTTON *btn, int32_t reason);
void FinishMessageBoxCallBack(uint8_t bExitValue);
extern void BtnIMPMainPageVoiceCallback(GUI_BUTTON *btn, int32_t reason);
extern void BtnIMPMainPagePortraitCallback(GUI_BUTTON *btn, int32_t reason);

void EnterIMPFinish(void) {
  // load graphic for portrait
  LoadCharacterPortrait();

  //	CREATE buttons for IMP finish screen
  CreateIMPFinishButtons();

  // set review mode
  fReviewStats = TRUE;
  iCurrentProfileMode = 5;

  // note that we are in fact done char generation
  fFinishedCharGeneration = TRUE;

  return;
}

void RenderIMPFinish(void) {
  // the background
  RenderProfileBackGround();

  // render merc fullname
  RenderCharFullName();

  // indent for text
  RenderBeginIndent(110, 50);

  return;
}

void ExitIMPFinish(void) {
  // remove buttons for IMP finish screen
  DeleteIMPFinishButtons();

  // get rid of graphic for portrait
  DestroyCharacterPortrait();

  return;
}

void HandleIMPFinish(void) { return; }

void CreateIMPFinishButtons(void) {
  wchar_t sString[128];

  // this function will create the buttons needed for th IMP about us page
  // the start over button button
  giIMPFinishButtonImage[0] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  /*giIMPFinishButton[0] = QuickCreateButton( giIMPFinishButtonImage[0], LAPTOP_SCREEN_UL_X + 136 ,
     LAPTOP_SCREEN_WEB_UL_Y + ( 114 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                          BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPFinishStartOverCallback);
*/
  giIMPFinishButton[0] = CreateIconAndTextButton(
      giIMPFinishButtonImage[0], pImpButtonText[7], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 136,
      LAPTOP_SCREEN_WEB_UL_Y + (174), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPFinishStartOverCallback);

  // the done button
  giIMPFinishButtonImage[1] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  /*	giIMPFinishButton[1] = QuickCreateButton( giIMPFinishButtonImage[1], LAPTOP_SCREEN_UL_X +
     136 , LAPTOP_SCREEN_WEB_UL_Y + ( 174 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                                  BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPFinishDoneCallback);
  */
  giIMPFinishButton[1] = CreateIconAndTextButton(
      giIMPFinishButtonImage[1], pImpButtonText[6], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 136,
      LAPTOP_SCREEN_WEB_UL_Y + (114), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPFinishDoneCallback);

  // the personality button
  giIMPFinishButtonImage[2] = LoadButtonImage("LAPTOP\\button_8.sti", -1, 0, -1, 1, -1);

  /*	giIMPFinishButton[2] = QuickCreateButton( giIMPFinishButtonImage[2], LAPTOP_SCREEN_UL_X + 13
     , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                                  BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPFinishPersonalityCallback);
  */
  giIMPFinishButton[2] = CreateIconAndTextButton(
      giIMPFinishButtonImage[2], pImpButtonText[2], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 13,
      LAPTOP_SCREEN_WEB_UL_Y + (245), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPFinishPersonalityCallback);

  SpecifyButtonIcon(giIMPFinishButton[2], guiANALYSE, 0, 33, 23, FALSE);

  // the attribs button
  giIMPFinishButtonImage[3] = LoadButtonImage("LAPTOP\\button_8.sti", -1, 0, -1, 1, -1);
  /*	giIMPFinishButton[3] = QuickCreateButton( giIMPFinishButtonImage[3], LAPTOP_SCREEN_UL_X +
     133 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                                  BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPFinishAttributesCallback);
  */
  giIMPFinishButton[3] = CreateIconAndTextButton(
      giIMPFinishButtonImage[3], pImpButtonText[3], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 133,
      LAPTOP_SCREEN_WEB_UL_Y + (245), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPFinishAttributesCallback);

  SpecifyButtonIcon(giIMPFinishButton[3], guiATTRIBUTEGRAPH, 0, 25, 25, FALSE);

  // the portrait button
  giIMPFinishButtonImage[4] = LoadButtonImage("LAPTOP\\button_8.sti", -1, 0, -1, 1, -1);
  /*	giIMPFinishButton[4] = QuickCreateButton( giIMPFinishButtonImage[4], LAPTOP_SCREEN_UL_X +
     253 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                                  BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPFinishPortraitCallback);
   */
  giIMPFinishButton[4] = CreateIconAndTextButton(
      giIMPFinishButtonImage[4], pImpButtonText[4], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 253,
      LAPTOP_SCREEN_WEB_UL_Y + (245), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPMainPagePortraitCallback);

  SpecifyButtonIcon(giIMPFinishButton[4], guiCHARACTERPORTRAIT, 0, 33, 23, FALSE);

  swprintf(sString, ARR_SIZE(sString), pImpButtonText[5], iCurrentVoices + 1);

  // the voice button
  giIMPFinishButtonImage[5] = LoadButtonImage("LAPTOP\\button_8.sti", -1, 0, -1, 1, -1);
  /* giIMPFinishButton[5] = QuickCreateButton( giIMPFinishButtonImage[5], LAPTOP_SCREEN_UL_X + 373 ,
     LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                          BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPFinishVoiceCallback);
*/
  giIMPFinishButton[5] = CreateIconAndTextButton(
      giIMPFinishButtonImage[5], sString, FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW, FONT_WHITE,
      DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 373, LAPTOP_SCREEN_WEB_UL_Y + (245),
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnIMPMainPageVoiceCallback);

  SpecifyButtonIcon(giIMPFinishButton[5], guiSMALLSILHOUETTE, 0, 33, 23, FALSE);

  SetButtonCursor(giIMPFinishButton[0], CURSOR_WWW);
  SetButtonCursor(giIMPFinishButton[1], CURSOR_WWW);
  SetButtonCursor(giIMPFinishButton[2], CURSOR_WWW);
  SetButtonCursor(giIMPFinishButton[3], CURSOR_WWW);
  SetButtonCursor(giIMPFinishButton[4], CURSOR_WWW);
  SetButtonCursor(giIMPFinishButton[5], CURSOR_WWW);

  return;
}

void DeleteIMPFinishButtons(void) {
  // this function destroys the buttons needed for the IMP about Us Page

  // the back  button
  RemoveButton(giIMPFinishButton[0]);
  UnloadButtonImage(giIMPFinishButtonImage[0]);

  // begin profiling button
  RemoveButton(giIMPFinishButton[1]);
  UnloadButtonImage(giIMPFinishButtonImage[1]);

  // begin personna button
  RemoveButton(giIMPFinishButton[2]);
  UnloadButtonImage(giIMPFinishButtonImage[2]);

  // begin attribs button
  RemoveButton(giIMPFinishButton[3]);
  UnloadButtonImage(giIMPFinishButtonImage[3]);

  // begin portrait button
  RemoveButton(giIMPFinishButton[4]);
  UnloadButtonImage(giIMPFinishButtonImage[4]);

  // begin voice button
  RemoveButton(giIMPFinishButton[5]);
  UnloadButtonImage(giIMPFinishButtonImage[5]);

  return;
}

void BtnIMPFinishStartOverCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP Homepage About US button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[1], LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO,
                         FinishMessageBoxCallBack);
    }
  }
}

void BtnIMPFinishDoneCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for Main Page Begin Profiling

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      iCurrentImpPage = IMP_CONFIRM;
      CreateACharacterFromPlayerEnteredStats();
      fButtonPendingFlag = TRUE;
      iCurrentProfileMode = 0;
      fFinishedCharGeneration = FALSE;
      // ResetCharacterStats( );
    }
  }
}

void BtnIMPFinishPersonalityCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for Main Page Begin Profiling
  static BOOLEAN fAnimateFlag = FALSE;
  static uint32_t uiBaseTime = 0;
  static BOOLEAN fState = 0;
  int32_t iDifference = 0;

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
    uiBaseTime = GetJA2Clock();
    SpecifyButtonText(giIMPFinishButton[2], pImpButtonText[23]);
    fAnimateFlag = TRUE;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fButtonPendingFlag = TRUE;
      uiBaseTime = 0;
      fAnimateFlag = FALSE;
      SpecifyButtonText(giIMPFinishButton[2], pImpButtonText[2]);
    }
  }

  // get amount of time between callbacks
  iDifference = GetJA2Clock() - uiBaseTime;

  if (fAnimateFlag) {
    if (iDifference > ANIMATE_MIN_TIME) {
      uiBaseTime = GetJA2Clock();
      if (fState) {
        SpecifyButtonIcon(giIMPFinishButton[2], guiANALYSE, 1, 33, 23, FALSE);

        fState = FALSE;
      } else {
        SpecifyButtonIcon(giIMPFinishButton[2], guiANALYSE, 0, 33, 23, FALSE);

        fState = TRUE;
      }
    }
  }
}

void BtnIMPFinishAttributesCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for Main Page Begin Profiling

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;
  // if not this far in char generation, don't alot ANY action
  if (iCurrentProfileMode < 2) {
    btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    return;
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      iCurrentImpPage = IMP_ATTRIBUTE_PAGE;
      fButtonPendingFlag = TRUE;
      SpecifyButtonText(giIMPFinishButton[2], pImpButtonText[2]);
    }
  }
}

void BtnIMPFinishPortraitCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for Main Page Begin Profiling

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;
  // if not this far in char generation, don't alot ANY action
  if (iCurrentProfileMode < 3) {
    btn->uiFlags &= ~(BUTTON_CLICKED_ON);

    return;
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
    sFaceX = 253;
    sFaceY = 247;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      sFaceX = 253;
      sFaceY = 245;
    }
  }
}

void BtnIMPFinishVoiceCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for Main Page Begin Profiling

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;
  // if not this far in char generation, don't alot ANY action
  if (iCurrentProfileMode < 4) {
    btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    fButtonPendingFlag = TRUE;
    return;
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);

  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      // play voice
      if (!SoundIsPlaying(uiVoiceSound)) {
        uiVoiceSound = PlayVoice();
      }
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fButtonPendingFlag = TRUE;
    }
  }
}

BOOLEAN RenderCharProfileFinishFace(void) {
  // render the portrait of the current picture
  struct VObject *hHandle;
  uint32_t uiGraphicHandle;

  if (fCharacterIsMale == TRUE) {
    switch (LaptopSaveInfo.iVoiceId) {
      case (0):
        // first portrait

        // load it
        CHECKF(AddVObject(CreateVObjectFromFile("Faces\\SmallFaces\\00.sti"), &uiGraphicHandle));

        // show it
        GetVideoObject(&hHandle, uiGraphicHandle);
        BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sFaceX,
                       LAPTOP_SCREEN_WEB_UL_Y + sFaceY);

        // and kick it's sorry ..umm never mind, outta here
        DeleteVideoObjectFromIndex(uiGraphicHandle);

        break;
      case (1):
        // first portrait

        // load it
        CHECKF(AddVObject(CreateVObjectFromFile("Faces\\SmallFaces\\01.sti"), &uiGraphicHandle));

        // show it
        GetVideoObject(&hHandle, uiGraphicHandle);
        BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sFaceX,
                       LAPTOP_SCREEN_WEB_UL_Y + sFaceY);

        // and kick it's sorry ..umm never mind, outta here
        DeleteVideoObjectFromIndex(uiGraphicHandle);

        break;
      case (2):
        // first portrait

        // load it
        CHECKF(AddVObject(CreateVObjectFromFile("Faces\\SmallFaces\\02.sti"), &uiGraphicHandle));

        // show it
        GetVideoObject(&hHandle, uiGraphicHandle);
        BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sFaceX,
                       LAPTOP_SCREEN_WEB_UL_Y + sFaceY);

        // and kick it's sorry ..umm never mind, outta here
        DeleteVideoObjectFromIndex(uiGraphicHandle);

        break;
    }

  } else {
    switch (LaptopSaveInfo.iVoiceId) {
      case (0):
        // first portrait

        // load it
        CHECKF(AddVObject(CreateVObjectFromFile("Faces\\SmallFaces\\03.sti"), &uiGraphicHandle));

        // show it
        GetVideoObject(&hHandle, uiGraphicHandle);
        BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sFaceX,
                       LAPTOP_SCREEN_WEB_UL_Y + sFaceY);

        // and kick it's sorry ..umm never mind, outta here
        DeleteVideoObjectFromIndex(uiGraphicHandle);

        break;
      case (1):
        // first portrait

        // load it
        CHECKF(AddVObject(CreateVObjectFromFile("Faces\\SmallFaces\\04.sti"), &uiGraphicHandle));

        // show it
        GetVideoObject(&hHandle, uiGraphicHandle);
        BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sFaceX,
                       LAPTOP_SCREEN_WEB_UL_Y + sFaceY);

        // and kick it's sorry ..umm never mind, outta here
        DeleteVideoObjectFromIndex(uiGraphicHandle);

        break;
      case (2):
        // first portrait

        // load it
        CHECKF(AddVObject(CreateVObjectFromFile("Faces\\SmallFaces\\05.sti"), &uiGraphicHandle));

        // show it
        GetVideoObject(&hHandle, uiGraphicHandle);
        BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sFaceX,
                       LAPTOP_SCREEN_WEB_UL_Y + sFaceY);

        // and kick it's sorry ..umm never mind, outta here
        DeleteVideoObjectFromIndex(uiGraphicHandle);

        break;
    }
  }

  // render the nickname
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  SetFont(FONT12ARIAL);

  mprintf(253, 350, pNickName);

  return (TRUE);
}

void RenderCharFullName(void) {
  wchar_t sString[64];
  int16_t sX, sY;

  // render the characters full name
  SetFont(FONT14ARIAL);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), pIMPFinishStrings[0], pFullName);

  FindFontCenterCoordinates(LAPTOP_SCREEN_UL_X, 0, LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, 0,
                            sString, FONT14ARIAL, &sX, &sY);
  mprintf(sX, LAPTOP_SCREEN_WEB_DELTA_Y + 33, sString);
  return;
}

BOOLEAN LoadCharacterPortrait(void) {
  // this function will load the character's portrait, to be used on portrait button

  // load it
  CHECKF(AddVObject(CreateVObjectFromFile(pPlayerSelectedFaceFileNames[iPortraitNumber]),
                    &guiCHARACTERPORTRAIT));

  return (TRUE);
}

void DestroyCharacterPortrait(void) {
  // remove the portrait that was loaded by loadcharacterportrait
  DeleteVideoObjectFromIndex(guiCHARACTERPORTRAIT);

  return;
}

void FinishMessageBoxCallBack(uint8_t bExitValue) {
  // yes, so start over, else stay here and do nothing for now
  if (bExitValue == MSG_BOX_RETURN_YES) {
    iCurrentImpPage = IMP_HOME_PAGE;
    fButtonPendingFlag = TRUE;
    iCurrentProfileMode = 0;
    fFinishedCharGeneration = FALSE;
    ResetCharacterStats();
  }

  if (bExitValue == MSG_BOX_RETURN_NO) {
    MarkButtonsDirty();
  }

  return;
}
