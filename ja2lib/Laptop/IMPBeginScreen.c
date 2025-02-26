// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/IMPBeginScreen.h"

#include <string.h>

#include "Laptop/CharProfile.h"
#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPFinish.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPPortraits.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/IMPVoices.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Line.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Strategic/GameClock.h"
#include "Strategic/Strategic.h"
#include "Tactical/MercHiring.h"
#include "Tactical/SoldierProfileType.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define FULL_NAME_CURSOR_Y LAPTOP_SCREEN_WEB_UL_Y + 138
#define NICK_NAME_CURSOR_Y LAPTOP_SCREEN_WEB_UL_Y + 195
#define MALE_BOX_X 2 + 192 + LAPTOP_SCREEN_UL_X
#define MALE_BOX_Y 254 + LAPTOP_SCREEN_WEB_UL_Y
#define MALE_BOX_WIDTH 24 - 2
#define MALE_BOX_HEIGHT 24 - 2
#define FEMALE_BOX_X 2 + 302 + LAPTOP_SCREEN_UL_X
#define MAX_FULL_NAME 29
#define MAX_NICK_NAME 8
#define FULL_NAME_REGION_WIDTH 230
#define NICK_NAME_REGION_WIDTH 100

// genders
enum {
  IMP_FEMALE = 0,
  IMP_MALE,
};

// TextEnterMode .. whether user is entering full name or nick name, or gender selection
enum {
  FULL_NAME_MODE,
  NICK_NAME_MODE,
  MALE_GENDER_SELECT,
  FEMALE_GENDER_SELECT,
};

// beginning character stats
wchar_t pFullNameString[128];
wchar_t pNickNameString[128];

// positions in name strings
uint32_t uiFullNameCharacterPosition = 0;
uint32_t uiNickNameCharacterPosition = 0;

// non gender
int8_t bGenderFlag = -1;

// IMP begin page buttons
int32_t giIMPBeginScreenButton[1];
int32_t giIMPBeginScreenButtonImage[1];

// current mode of entering text we are in, ie FULL or Nick name?
uint8_t ubTextEnterMode = 0;

// cursor position
uint32_t uiNickNameCursorPosition = 196 + LAPTOP_SCREEN_UL_X;
uint32_t uiFullNameCursorPosition = 196 + LAPTOP_SCREEN_UL_X;

// whther a new char has been entered ( to force redraw)
BOOLEAN fNewCharInString = FALSE;

// mouse regions
struct MOUSE_REGION gIMPBeginScreenMouseRegions[4];

// function definitions
void CreateIMPBeginScreenButtons(void);
void RemoveIMPBeginScreenButtons(void);
void BtnIMPBeginScreenDoneCallback(GUI_BUTTON *btn, int32_t reason);
void GetPlayerKeyBoardInputForIMPBeginScreen(void);
void HandleBeginScreenTextEvent(uint32_t uiKey);
void DisplayFullNameStringCursor(void);
void DisplayNickNameStringCursor(void);
void DisplayPlayerFullNameString(void);
void DisplayPlayerNickNameString(void);
void CopyFirstNameIntoNickName(void);
void IncrementTextEnterMode(void);
void DisplayMaleGlowCursor(void);
void RenderMaleGenderIcon(void);
void DisplayFemaleGlowCursor(void);
void RenderFemaleGenderIcon(void);
void CreateIMPBeginScreenMouseRegions(void);
void DestroyIMPBeginScreenMouseRegions(void);
void RenderGender(void);
void DecrementTextEnterMode(void);
void Print8CharacterOnlyString(void);
BOOLEAN CheckCharacterInputForEgg(void);

// mouse region callbacks
void SelectFullNameRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectNickNameRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectMaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectFemaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void MvtOnMaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void MvtOnFemaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

void EnterIMPBeginScreen(void) {
  // reset all variables

  memset(pFullNameString, 0, sizeof(pFullNameString));
  memset(pNickNameString, 0, sizeof(pNickNameString));

  // if we are not restarting...then copy over name, set cursor and array positions
  if (iCurrentProfileMode != 0) {
    wcscpy(pFullNameString, pFullName);
    wcscpy(pNickNameString, pNickName);
    uiFullNameCharacterPosition = wcslen(pFullNameString);
    uiNickNameCharacterPosition = wcslen(pNickNameString);
    uiFullNameCursorPosition =
        196 + LAPTOP_SCREEN_UL_X + StringPixLength(pFullNameString, FONT14ARIAL);
    uiNickNameCursorPosition =
        196 + LAPTOP_SCREEN_UL_X + StringPixLength(pNickNameString, FONT14ARIAL);

    // set gender too
    bGenderFlag = fCharacterIsMale;

  } else {
    uiNickNameCursorPosition = 196 + LAPTOP_SCREEN_UL_X;
    uiFullNameCursorPosition = 196 + LAPTOP_SCREEN_UL_X;
    uiFullNameCharacterPosition = 0;
    uiNickNameCharacterPosition = 0;
    bGenderFlag = -1;
  }

  ubTextEnterMode = 0;

  // draw name if any
  fNewCharInString = TRUE;

  // render the screen on entry
  RenderIMPBeginScreen();

  if (fFinishedCharGeneration) {
    ubTextEnterMode = 5;
  } else {
    fFirstIMPAttribTime = TRUE;
  }

  // create mouse regions
  CreateIMPBeginScreenMouseRegions();

  // create buttons needed for begin screen
  CreateIMPBeginScreenButtons();

  return;
}

void RenderIMPBeginScreen(void) {
  // the background
  RenderProfileBackGround();

  // fourth button image 3X
  RenderButton4Image(64, 118);
  RenderButton4Image(64, 178);
  RenderButton4Image(64, 238);

  // the begin screen indents
  RenderBeginIndent(105, 58);

  // full name indent
  RenderNameIndent(194, 132);

  // nick name
  RenderNickNameIndent(194, 192);

  // gender indents
  RenderGenderIndent(192, 252);
  RenderGenderIndent(302, 252);

  // render warning string
  Print8CharacterOnlyString();

  // display player name
  DisplayPlayerFullNameString();
  DisplayPlayerNickNameString();
  RenderMaleGenderIcon();
  RenderFemaleGenderIcon();

  // the gender itself
  RenderGender();

  return;
}

void ExitIMPBeginScreen(void) {
  // remove buttons
  RemoveIMPBeginScreenButtons();

  // remove mouse regions
  DestroyIMPBeginScreenMouseRegions();

  wcscpy(pFullName, pFullNameString);

  // is nick name too long?..shorten
  if (wcslen(pNickNameString) > 8) {
    // null out char 9
    pNickNameString[8] = 0;
  }

  wcscpy(pNickName, pNickNameString);

  // set gender
  fCharacterIsMale = bGenderFlag;

  return;
}

void HandleIMPBeginScreen(void) {
  GetPlayerKeyBoardInputForIMPBeginScreen();

  // has a new char been added to activation string

  // render the cursor
  switch (ubTextEnterMode) {
    case (FULL_NAME_MODE):
      DisplayFullNameStringCursor();
      break;
    case (NICK_NAME_MODE):
      DisplayNickNameStringCursor();
      break;
    case (MALE_GENDER_SELECT):
      DisplayMaleGlowCursor();
      break;
    case (FEMALE_GENDER_SELECT):
      DisplayFemaleGlowCursor();
      break;
  }

  if (fNewCharInString) {
    // display strings
    DisplayPlayerFullNameString();
    DisplayPlayerNickNameString();
    RenderMaleGenderIcon();
    RenderFemaleGenderIcon();

    // the gender itself
    RenderGender();
  }

  return;
}

void CreateIMPBeginScreenButtons(void) {
  // this procedure will create the buttons needed for the IMP BeginScreen

  // ths done button
  giIMPBeginScreenButtonImage[0] = LoadButtonImage("LAPTOP\\button_2.sti", -1, 0, -1, 1, -1);
  /*	giIMPBeginScreenButton[0] = QuickCreateButton( giIMPBeginScreenButtonImage[0],
     LAPTOP_SCREEN_UL_X +  ( 134 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE,
     MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPBeginScreenDoneCallback);
    */

  giIMPBeginScreenButton[0] = CreateIconAndTextButton(
      giIMPBeginScreenButtonImage[0], pImpButtonText[6], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + (134),
      LAPTOP_SCREEN_WEB_UL_Y + (314), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPBeginScreenDoneCallback);

  SetButtonCursor(giIMPBeginScreenButton[0], CURSOR_WWW);
  return;
}

void RemoveIMPBeginScreenButtons(void) {
  // this procedure will destroy the already created buttosn for the IMP BeginScreen

  // the done button
  RemoveButton(giIMPBeginScreenButton[0]);
  UnloadButtonImage(giIMPBeginScreenButtonImage[0]);

  return;
}

void BtnIMPBeginScreenDoneCallback(GUI_BUTTON *btn, int32_t reason) {
  // easter egg check
  BOOLEAN fEggOnYouFace = FALSE;

  // btn callback for IMP Begin Screen done button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (fFinishedCharGeneration) {
        // simply reviewing name and gender, exit to finish page
        iCurrentImpPage = IMP_FINISH;
        fButtonPendingFlag = TRUE;
        return;
      } else {
        if (CheckCharacterInputForEgg()) {
          fEggOnYouFace = TRUE;
        }
      }

      // back to mainpage

      // check to see if a name has been selected, if not, do not allow player to proceed with more
      // char generation
      if ((pFullNameString[0] != 0) && (pFullNameString[0] != L' ') && (bGenderFlag != -1)) {
        // valid full name, check to see if nick name
        if ((pNickNameString[0] == 0) || (pNickNameString[0] == L' ')) {
          // no nick name
          // copy first name to nick name
          CopyFirstNameIntoNickName();
        }
        // ok, now set back to main page, and set the fact we have completed part 1
        if ((iCurrentProfileMode < 1) && (bGenderFlag != -1)) {
          iCurrentProfileMode = 1;
        } else if (bGenderFlag == -1) {
          iCurrentProfileMode = 0;
        }

        // no easter egg?...then proceed along
        if (fEggOnYouFace == FALSE) {
          iCurrentImpPage = IMP_MAIN_PAGE;
          fButtonPendingFlag = TRUE;
        }

      } else {
        // invalid name, reset current mode
        DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[2], LAPTOP_SCREEN, MSG_BOX_FLAG_OK,
                           NULL);
        iCurrentProfileMode = 0;
      }
    }
  }
}

void GetPlayerKeyBoardInputForIMPBeginScreen(void) {
  InputAtom InputEvent;

  // handle input events
  while (DequeueEvent(&InputEvent)) {
    if (!HandleTextInput(&InputEvent) &&
        (InputEvent.usEvent == KEY_DOWN || InputEvent.usEvent == KEY_REPEAT)) {
      switch (InputEvent.usParam) {
        case ((ENTER)):
          // check to see if gender was highlighted..if so, select it
          if (FEMALE_GENDER_SELECT == ubTextEnterMode) {
            bGenderFlag = IMP_FEMALE;
          } else if (MALE_GENDER_SELECT == ubTextEnterMode) {
            bGenderFlag = IMP_MALE;
          }

          // increment to next selection box
          IncrementTextEnterMode();
          fNewCharInString = TRUE;
          break;
        case (SPACE):
          // handle space bar
          if (FEMALE_GENDER_SELECT == ubTextEnterMode) {
            bGenderFlag = IMP_FEMALE;
            DecrementTextEnterMode();
          } else if (MALE_GENDER_SELECT == ubTextEnterMode) {
            bGenderFlag = IMP_MALE;
            IncrementTextEnterMode();
          } else {
            HandleBeginScreenTextEvent(InputEvent.usParam);
          }
          fNewCharInString = TRUE;
          break;
        case ((ESC)):
          LeaveLapTopScreen();
          break;
        case ((TAB)):
          // tab hit, increment to next selection box
          IncrementTextEnterMode();
          fNewCharInString = TRUE;
          break;
        case (265):
          // tab and shift
          DecrementTextEnterMode();
          fNewCharInString = TRUE;
          break;
        default:
          HandleBeginScreenTextEvent(InputEvent.usParam);
          break;
      }
    }
  }
  return;
}

void HandleBeginScreenTextEvent(uint32_t uiKey) {
  // this function checks to see if a letter or a backspace was pressed, if so, either put char to
  // screen or delete it

  switch (uiKey) {
    case (BACKSPACE):

      switch (ubTextEnterMode) {
        case (FULL_NAME_MODE):
          if (uiFullNameCharacterPosition >= 0) {
            // decrement StringPosition
            if (uiFullNameCharacterPosition > 0) {
              uiFullNameCharacterPosition -= 1;
            }

            // null out char
            pFullNameString[uiFullNameCharacterPosition] = 0;

            // move cursor back by sizeof char
            uiFullNameCursorPosition =
                196 + LAPTOP_SCREEN_UL_X + StringPixLength(pFullNameString, FONT14ARIAL);

            // string has been altered, redisplay
            fNewCharInString = TRUE;
          }
          break;
        case (NICK_NAME_MODE):
          if (uiNickNameCharacterPosition >= 0) {
            // decrement StringPosition
            if (uiNickNameCharacterPosition > 0) uiNickNameCharacterPosition -= 1;

            // null out char
            pNickNameString[uiNickNameCharacterPosition] = 0;

            // move cursor back by sizeof char
            uiNickNameCursorPosition =
                196 + LAPTOP_SCREEN_UL_X + StringPixLength(pNickNameString, FONT14ARIAL);

            // string has been altered, redisplay
            fNewCharInString = TRUE;
          }

          break;
      }
      break;

    default:
      if ((uiKey >= 'A' && uiKey <= 'Z') || (uiKey >= 'a' && uiKey <= 'z') ||
          (uiKey >= '0' && uiKey <= '9') || uiKey == '_' || uiKey == '.' || uiKey == ' ') {
        // if the current string position is at max or great, do nothing
        switch (ubTextEnterMode) {
          case (FULL_NAME_MODE):
            if (uiFullNameCharacterPosition >= MAX_FULL_NAME) {
              break;
            } else {
              if (uiFullNameCharacterPosition < 1) {
                uiFullNameCharacterPosition = 0;
              }
              // make sure we haven't moved too far
              if ((uiFullNameCursorPosition + StringPixLength((wchar_t *)&uiKey, FONT14ARIAL)) >
                  FULL_NAME_REGION_WIDTH + 196 + LAPTOP_SCREEN_UL_X) {
                // do nothing for now, when pop up is in place, display
                break;
              }
              // valid char, capture and convert to wchar_t
              pFullNameString[uiFullNameCharacterPosition] = (wchar_t)uiKey;

              // null out next char position
              pFullNameString[uiFullNameCharacterPosition + 1] = 0;

              // move cursor position ahead
              uiFullNameCursorPosition =
                  196 + LAPTOP_SCREEN_UL_X + StringPixLength(pFullNameString, FONT14ARIAL);

              // increment string position
              uiFullNameCharacterPosition += 1;

              // string has been altered, redisplay
              fNewCharInString = TRUE;
            }
            break;
          case (NICK_NAME_MODE):
            if (uiNickNameCharacterPosition >= MAX_NICK_NAME) {
              break;
            } else {
              if (uiNickNameCharacterPosition == -1) {
                uiNickNameCharacterPosition = 0;
              }

              // make sure we haven't moved too far
              if ((uiNickNameCursorPosition + StringPixLength((wchar_t *)&uiKey, FONT14ARIAL)) >
                  NICK_NAME_REGION_WIDTH + 196 + LAPTOP_SCREEN_UL_X) {
                // do nothing for now, when pop up is in place, display
                break;
              }

              // valid char, capture and convert to wchar_t
              pNickNameString[uiNickNameCharacterPosition] = (wchar_t)uiKey;

              // null out next char position
              pNickNameString[uiNickNameCharacterPosition + 1] = 0;

              // move cursor position ahead
              uiNickNameCursorPosition =
                  196 + LAPTOP_SCREEN_UL_X + StringPixLength(pNickNameString, FONT14ARIAL);

              // increment string position
              uiNickNameCharacterPosition += 1;

              // string has been altered, redisplay
              fNewCharInString = TRUE;
            }

            break;
        }
      }
      break;
  }
  return;
}

void DisplayFullNameStringCursor(void) {
  // this procdure will draw the activation string cursor on the screen at position cursorx cursory
  uint32_t uiDestPitchBYTES;
  static uint32_t uiBaseTime = 0;
  uint32_t uiDeltaTime = 0;
  static uint32_t iCurrentState = 0;
  uint8_t *pDestBuf;
  static BOOLEAN fIncrement = TRUE;

  if (uiBaseTime == 0) {
    uiBaseTime = GetJA2Clock();
  }

  // get difference
  uiDeltaTime = GetJA2Clock() - uiBaseTime;

  // if difference is long enough, rotate colors
  if (uiDeltaTime > MIN_GLOW_DELTA) {
    if (iCurrentState == 10) {
      // start rotating downward
      fIncrement = FALSE;
    }
    if (iCurrentState == 0) {
      // rotate colors upward
      fIncrement = TRUE;
    }
    // if increment upward, increment iCurrentState
    if (fIncrement) {
      iCurrentState++;
    } else {
      // else downwards
      iCurrentState--;
    }
    // reset basetime to current clock
    uiBaseTime = GetJA2Clock();
  }

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw line in current state
  LineDraw(
      TRUE, (uint16_t)uiFullNameCursorPosition, FULL_NAME_CURSOR_Y - 3,
      (uint16_t)uiFullNameCursorPosition, FULL_NAME_CURSOR_Y + CURSOR_HEIGHT - 2,
      rgb32_to_rgb16(FROMRGB(GlowColorsList[iCurrentState][0], GlowColorsList[iCurrentState][1],
                             GlowColorsList[iCurrentState][2])),
      pDestBuf);

  InvalidateRegion((uint16_t)uiFullNameCursorPosition, FULL_NAME_CURSOR_Y - 3,
                   (uint16_t)uiFullNameCursorPosition + 1,
                   FULL_NAME_CURSOR_Y + CURSOR_HEIGHT + 1 - 2);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
  return;
}

void DisplayNickNameStringCursor(void) {
  // this procdure will draw the activation string cursor on the screen at position cursorx cursory
  uint32_t uiDestPitchBYTES;
  static uint32_t uiBaseTime = 0;
  uint32_t uiDeltaTime = 0;
  uint8_t *pDestBuf;
  static uint32_t iCurrentState = 0;
  static BOOLEAN fIncrement = TRUE;

  if (uiBaseTime == 0) {
    uiBaseTime = GetJA2Clock();
  }

  // get difference
  uiDeltaTime = GetJA2Clock() - uiBaseTime;

  // if difference is long enough, rotate colors
  if (uiDeltaTime > MIN_GLOW_DELTA) {
    if (iCurrentState == 10) {
      // start rotating downward
      fIncrement = FALSE;
    }
    if (iCurrentState == 0) {
      // rotate colors upward
      fIncrement = TRUE;
    }
    // if increment upward, increment iCurrentState
    if (fIncrement) {
      iCurrentState++;
    } else {
      // else downwards
      iCurrentState--;
    }
    // reset basetime to current clock
    uiBaseTime = GetJA2Clock();
  }

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw line in current state
  LineDraw(
      TRUE, (uint16_t)uiNickNameCursorPosition, NICK_NAME_CURSOR_Y,
      (uint16_t)uiNickNameCursorPosition, NICK_NAME_CURSOR_Y + CURSOR_HEIGHT,
      rgb32_to_rgb16(FROMRGB(GlowColorsList[iCurrentState][0], GlowColorsList[iCurrentState][1],
                             GlowColorsList[iCurrentState][2])),
      pDestBuf);

  InvalidateRegion((uint16_t)uiNickNameCursorPosition, NICK_NAME_CURSOR_Y,
                   (uint16_t)uiNickNameCursorPosition + 1, NICK_NAME_CURSOR_Y + CURSOR_HEIGHT + 1);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
  return;
}

void DisplayPlayerFullNameString(void) {
  // this function will grab the string that the player will enter for activation

  // player gone too far, move back
  if (uiFullNameCharacterPosition > MAX_FULL_NAME) {
    uiFullNameCharacterPosition = MAX_FULL_NAME;
  }

  // restore background
  RenderNameIndent(194, 132);

  // setup the font stuff
  SetFont(FONT14ARIAL);
  SetFontForeground(184);
  SetFontBackground(FONT_BLACK);

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);
  mprintf(LAPTOP_SCREEN_UL_X + 196, FULL_NAME_CURSOR_Y + 1, pFullNameString);

  fNewCharInString = FALSE;
  fReDrawScreenFlag = TRUE;
  return;
}

void DisplayPlayerNickNameString(void) {
  // this function will grab the string that the player will enter for activation

  // player gone too far, move back
  if (uiNickNameCharacterPosition > MAX_NICK_NAME) {
    uiNickNameCharacterPosition = MAX_NICK_NAME;
  }

  // restore background
  RenderNickNameIndent(194, 192);

  // setup the font stuff
  SetFont(FONT14ARIAL);
  SetFontForeground(184);
  SetFontBackground(FONT_BLACK);

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);
  mprintf(LAPTOP_SCREEN_UL_X + 196, NICK_NAME_CURSOR_Y + 4, pNickNameString);

  fNewCharInString = FALSE;
  fReDrawScreenFlag = TRUE;
  return;
}

void DisplayMaleGlowCursor(void) {
  // this procdure will draw the activation string cursor on the screen at position cursorx cursory
  uint32_t uiDestPitchBYTES;
  static uint32_t uiBaseTime = 0;
  uint32_t uiDeltaTime = 0;
  static uint32_t iCurrentState = 0;
  static BOOLEAN fIncrement = TRUE;
  uint8_t *pDestBuf;

  if (uiBaseTime == 0) {
    uiBaseTime = GetJA2Clock();
  }

  // get difference
  uiDeltaTime = GetJA2Clock() - uiBaseTime;

  // if difference is long enough, rotate colors
  if (uiDeltaTime > MIN_GLOW_DELTA) {
    if (iCurrentState == 10) {
      // start rotating downward
      fIncrement = FALSE;
    }
    if (iCurrentState == 0) {
      // rotate colors upward
      fIncrement = TRUE;
    }
    // if increment upward, increment iCurrentState
    if (fIncrement) {
      iCurrentState++;
    } else {
      // else downwards
      iCurrentState--;
    }
    // reset basetime to current clock
    uiBaseTime = GetJA2Clock();
  }

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw rectangle
  RectangleDraw(
      TRUE, MALE_BOX_X, MALE_BOX_Y, MALE_BOX_X + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT,
      rgb32_to_rgb16(FROMRGB(GlowColorsList[iCurrentState][0], GlowColorsList[iCurrentState][1],
                             GlowColorsList[iCurrentState][2])),
      pDestBuf);

  InvalidateRegion((uint16_t)MALE_BOX_X, MALE_BOX_Y, MALE_BOX_X + MALE_BOX_WIDTH + 1,
                   MALE_BOX_Y + MALE_BOX_HEIGHT + 1);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
  return;
}

void DisplayFemaleGlowCursor(void) {
  // this procdure will draw the activation string cursor on the screen at position cursorx cursory
  uint32_t uiDestPitchBYTES;
  static uint32_t uiBaseTime = 0;
  uint32_t uiDeltaTime = 0;
  static uint32_t iCurrentState = 0;
  static BOOLEAN fIncrement = TRUE;
  uint8_t *pDestBuf;

  if (uiBaseTime == 0) {
    uiBaseTime = GetJA2Clock();
  }

  // get difference
  uiDeltaTime = GetJA2Clock() - uiBaseTime;

  // if difference is long enough, rotate colors
  if (uiDeltaTime > MIN_GLOW_DELTA) {
    if (iCurrentState == 10) {
      // start rotating downward
      fIncrement = FALSE;
    }
    if (iCurrentState == 0) {
      // rotate colors upward
      fIncrement = TRUE;
    }
    // if increment upward, increment iCurrentState
    if (fIncrement) {
      iCurrentState++;
    } else {
      // else downwards
      iCurrentState--;
    }
    // reset basetime to current clock
    uiBaseTime = GetJA2Clock();
  }

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw rectangle
  RectangleDraw(
      TRUE, FEMALE_BOX_X, MALE_BOX_Y, FEMALE_BOX_X + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT,
      rgb32_to_rgb16(FROMRGB(GlowColorsList[iCurrentState][0], GlowColorsList[iCurrentState][1],
                             GlowColorsList[iCurrentState][2])),
      pDestBuf);

  InvalidateRegion((uint16_t)FEMALE_BOX_X, MALE_BOX_Y, FEMALE_BOX_X + MALE_BOX_WIDTH + 1,
                   MALE_BOX_Y + MALE_BOX_HEIGHT + 1);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
  return;
}

void CopyFirstNameIntoNickName(void) {
  // this procedure will copy the characters first name in to the nickname for the character
  uint32_t iCounter = 0;
  while ((pFullNameString[iCounter] != L' ') && (iCounter < 20) &&
         (pFullNameString[iCounter] != 0)) {
    // copy charcters into nick name
    pNickNameString[iCounter] = pFullNameString[iCounter];
    iCounter++;
  }

  return;
}

void IncrementTextEnterMode(void) {
  // this function will incrment which text enter mode we are in, FULLname, NICKname, IMP_MALE or
  // IMP_FEMALE

  // if at IMP_FEMALE gender selection, reset to full name
  if (FEMALE_GENDER_SELECT == ubTextEnterMode) {
    ubTextEnterMode = FULL_NAME_MODE;
  } else {
    // otherwise, next selection
    ubTextEnterMode++;
  }
}

void DecrementTextEnterMode(void) {
  // this function will incrment which text enter mode we are in, FULLname, NICKname, IMP_MALE or
  // IMP_FEMALE

  // if at IMP_FEMALE gender selection, reset to full name
  if (FULL_NAME_MODE == ubTextEnterMode) {
    ubTextEnterMode = FEMALE_GENDER_SELECT;
  } else {
    // otherwise, next selection
    ubTextEnterMode--;
  }
}

void RenderMaleGenderIcon(void) {
  // this function displays a filled box in the IMP_MALE gender box if IMP_MALE has been selected

  // re render indent
  RenderGenderIndent(192, 252);

  // IMP_MALE selected draw box
  if (bGenderFlag == IMP_MALE) {
  }
}

void RenderFemaleGenderIcon(void) {
  // this function displays a filled box in the IMP_MALE gender box if IMP_MALE has been selected

  // re render indent
  RenderGenderIndent(302, 252);

  // IMP_FEMALE selected draw box
  if (bGenderFlag == IMP_FEMALE) {
  }
}

// mouse regions

void CreateIMPBeginScreenMouseRegions(void) {
  // this function creates the IMP mouse regions

  // are we only reviewing text?.. if so, do not create regions
  if (ubTextEnterMode == 5) return;

  // full name region
  MSYS_DefineRegion(&gIMPBeginScreenMouseRegions[0], LAPTOP_SCREEN_UL_X + 196,
                    LAPTOP_SCREEN_WEB_UL_Y + 135, LAPTOP_SCREEN_UL_X + 196 + FULL_NAME_REGION_WIDTH,
                    LAPTOP_SCREEN_WEB_UL_Y + 135 + 24, MSYS_PRIORITY_HIGH, CURSOR_WWW,
                    MSYS_NO_CALLBACK, (MOUSE_CALLBACK)SelectFullNameRegionCallBack);

  // nick name region
  MSYS_DefineRegion(&gIMPBeginScreenMouseRegions[1], LAPTOP_SCREEN_UL_X + 196,
                    LAPTOP_SCREEN_WEB_UL_Y + 195, LAPTOP_SCREEN_UL_X + 196 + NICK_NAME_REGION_WIDTH,
                    LAPTOP_SCREEN_WEB_UL_Y + 195 + 24, MSYS_PRIORITY_HIGH, CURSOR_WWW,
                    MSYS_NO_CALLBACK, (MOUSE_CALLBACK)SelectNickNameRegionCallBack);

  // IMP_MALE gender area
  MSYS_DefineRegion(&gIMPBeginScreenMouseRegions[2], MALE_BOX_X, MALE_BOX_Y,
                    MALE_BOX_X + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT, MSYS_PRIORITY_HIGH,
                    CURSOR_WWW, (MOUSE_CALLBACK)MvtOnMaleRegionCallBack,
                    (MOUSE_CALLBACK)SelectMaleRegionCallBack);

  // IMP_FEMALE gender region
  MSYS_DefineRegion(&gIMPBeginScreenMouseRegions[3], FEMALE_BOX_X, MALE_BOX_Y,
                    FEMALE_BOX_X + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT, MSYS_PRIORITY_HIGH,
                    CURSOR_WWW, (MOUSE_CALLBACK)MvtOnFemaleRegionCallBack,
                    (MOUSE_CALLBACK)SelectFemaleRegionCallBack);

  // add regions
  MSYS_AddRegion(&gIMPBeginScreenMouseRegions[0]);
  MSYS_AddRegion(&gIMPBeginScreenMouseRegions[1]);
  MSYS_AddRegion(&gIMPBeginScreenMouseRegions[2]);
  MSYS_AddRegion(&gIMPBeginScreenMouseRegions[3]);

  return;
}

void DestroyIMPBeginScreenMouseRegions(void) {
  // this function destroys the IMP mouse regions

  // are we only reviewing text?.. if so, do not remove regions
  if (ubTextEnterMode == 5) return;

  // remove regions
  MSYS_RemoveRegion(&gIMPBeginScreenMouseRegions[0]);
  MSYS_RemoveRegion(&gIMPBeginScreenMouseRegions[1]);
  MSYS_RemoveRegion(&gIMPBeginScreenMouseRegions[2]);
  MSYS_RemoveRegion(&gIMPBeginScreenMouseRegions[3]);

  return;
}

void SelectFullNameRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set current mode to full name type in mode
    ubTextEnterMode = FULL_NAME_MODE;
    fNewCharInString = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectNickNameRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set mode to nick name type in
    ubTextEnterMode = NICK_NAME_MODE;
    fNewCharInString = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectMaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set mode to nick name type in
    bGenderFlag = IMP_MALE;
    fNewCharInString = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectFemaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set mode to nick name type in
    bGenderFlag = IMP_FEMALE;
    fNewCharInString = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void MvtOnFemaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // fNewCharInString = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    ubTextEnterMode = FEMALE_GENDER_SELECT;
    fNewCharInString = TRUE;
  }
}

void MvtOnMaleRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // fNewCharInString = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    ubTextEnterMode = MALE_GENDER_SELECT;
    fNewCharInString = TRUE;
  }
}

void RenderGender(void) {
  // this procedure will render the gender of the character int he appropriate box

  // check to see if gender has been in fact set
  if (bGenderFlag == -1) {
    // nope, leave
    return;
  }

  SetFontBackground(FONT_BLACK);
  SetFontForeground(184);
  if (bGenderFlag == IMP_MALE) {
    // IMP_MALE, render x in IMP_MALE box
    mprintf(MALE_BOX_X + 9, MALE_BOX_Y + 6, L"X");
  } else {
    // IMP_FEMALE, render x in IMP_FEMALE box
    mprintf(FEMALE_BOX_X + 9, MALE_BOX_Y + 6, L"X");
  }
}

void Print8CharacterOnlyString(void) {
  SetFontBackground(FONT_BLACK);
  SetFontForeground(FONT_BLACK);
  SetFont(FONT12ARIAL);
  SetFontShadow(NO_SHADOW);

  mprintf(430, LAPTOP_SCREEN_WEB_DELTA_Y + 228, pIMPBeginScreenStrings[0]);

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);
}

BOOLEAN CheckCharacterInputForEgg(void) {
#ifndef JA2BETAVERSION
  return (FALSE);
#else
#ifndef _DEBUG
  return (FALSE);
#else
  if ((wcscmp(pFullNameString, L"retraC kraM") == 0) && (wcscmp(pNickNameString, L"BdyCnt"))) {
    wcscpy(pFullNameString, L"Mark Carter");
    wcscpy(pNickNameString, L"BodyCount");
    bGenderFlag = IMP_MALE;
    iHealth = 99;
    iAgility = 99;
    iStrength = 99;
    iDexterity = 99;
    iWisdom = 35;
    iLeadership = 35;
    iMarksmanship = 99;
    iMechanical = 0;
    iExplosives = 99;
    iMedical = 0;
    iSkillA = AMBIDEXT;
    iSkillB = AUTO_WEAPS;
    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_ASSHOLE;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 2;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"hcnerF evaD") == 0) &&
             (wcscmp(pNickNameString, L"Feral") == 0)) {
    wcscpy(pFullNameString, L"Dave French");
    wcscpy(pNickNameString, L"Feral");
    bGenderFlag = IMP_MALE;
    iHealth = 90;
    iAgility = 95;
    iStrength = 90;
    iDexterity = 95;
    iWisdom = 60;
    iLeadership = 60;
    iMarksmanship = 89;
    iMechanical = 40;
    iExplosives = 25;
    iMedical = 60;
    iSkillA = MARTIALARTS;
    iSkillB = AUTO_WEAPS;
    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_FRIENDLY;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 1;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"Marnes") == 0) &&
             (wcscmp(pNickNameString, L"Marnes") == 0)) {
    wcscpy(pFullNameString, L"Kris Marnes");
    wcscpy(pNickNameString, L"Marnes");
    bGenderFlag = IMP_MALE;
    iHealth = 100;
    iAgility = 100;
    iStrength = 100;
    iDexterity = 100;
    iWisdom = 100;
    iLeadership = 100;
    iMarksmanship = 100;
    iMechanical = 100;
    iExplosives = 100;
    iMedical = 100;
    iSkillA = MARTIALARTS;
    iSkillB = AUTO_WEAPS;
    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_FRIENDLY;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 2;
    iPortraitNumber = 1;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"neslO namroN") == 0) &&
             (wcscmp(pNickNameString, L"N.R.G") == 0)) {
    wcscpy(pFullNameString, L"Norman Olsen");
    wcscpy(pNickNameString, L"N.R.G");
    bGenderFlag = IMP_MALE;
    iHealth = 99;
    iAgility = 99;
    iStrength = 99;
    iDexterity = 90;
    iWisdom = 70;
    iLeadership = 35;

    iMarksmanship = 70;
    iMechanical = 65;
    iExplosives = 99;
    iMedical = 75;

    iSkillA = STEALTHY;
    iSkillB = MARTIALARTS;

    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_AGGRESSIVE;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 4;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"snommE werdnA") == 0) &&
             (wcscmp(pNickNameString, L"Bubba") == 0)) {
    wcscpy(pFullNameString, L"Andrew Emmons");
    wcscpy(pNickNameString, L"Bubba");
    bGenderFlag = IMP_MALE;
    iHealth = 97;
    iAgility = 98;
    iStrength = 80;
    iDexterity = 80;
    iWisdom = 99;
    iLeadership = 99;

    iMarksmanship = 99;
    iMechanical = 0;
    iExplosives = 0;
    iMedical = 99;

    iSkillA = AUTO_WEAPS;
    iSkillB = AMBIDEXT;

    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_ARROGANT;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 2;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"nalehW yeoJ") == 0) &&
             (wcscmp(pNickNameString, L"Joeker") == 0)) {
    wcscpy(pFullNameString, L"Joey Whelan");
    wcscpy(pNickNameString, L"Joeker");
    bGenderFlag = 0;
    iHealth = 99;
    iAgility = 99;
    iStrength = 99;
    iDexterity = 99;
    iWisdom = 70;
    iLeadership = 80;

    iMarksmanship = 99;
    iMechanical = 35;
    iExplosives = 99;
    iMedical = 35;

    iSkillA = AUTO_WEAPS;
    iSkillB = MARTIALARTS;

    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_AGGRESSIVE;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 4;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"gnehC cirE") == 0) &&
             (wcscmp(pNickNameString, L"BlakAddr") == 0)) {
    wcscpy(pFullNameString, L"Eric Cheng");
    wcscpy(pNickNameString, L"BlakAddr");
    bGenderFlag = IMP_MALE;
    iHealth = 99;
    iAgility = 99;
    iStrength = 99;
    iDexterity = 99;
    iWisdom = 99;
    iLeadership = 70;

    iMarksmanship = 99;
    iMechanical = 50;
    iExplosives = 99;
    iMedical = 0;

    iSkillA = AUTO_WEAPS;
    iSkillB = MARTIALARTS;

    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_LONER;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 3;
    return TRUE;
  } else if ((wcscmp(pFullNameString, L"Karters Killer Kru") == 0) &&
             (wcscmp(pNickNameString, L"Bitchin") == 0)) {
    wcscpy(pFullNameString, L"Mark Carter");
    wcscpy(pNickNameString, L"BodyCount");
    bGenderFlag = IMP_MALE;
    iHealth = 99;
    iAgility = 99;
    iStrength = 99;
    iDexterity = 99;
    iWisdom = 35;
    iLeadership = 35;
    iMarksmanship = 99;
    iMechanical = 0;
    iExplosives = 99;
    iMedical = 0;
    iSkillA = AMBIDEXT;
    iSkillB = AUTO_WEAPS;
    iPersonality = PSYCHO;
    iAttitude = ATT_ASSHOLE;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 2;

    // DEF: temp
    MERC_HIRE_STRUCT HireMercStruct;
    HireMercStruct.sSectorX = gsMercArriveSectorX;
    HireMercStruct.sSectorY = gsMercArriveSectorY;
    HireMercStruct.bSectorZ = 0;
    HireMercStruct.fUseLandingZoneForArrival = TRUE;
    HireMercStruct.ubInsertionCode = INSERTION_CODE_ARRIVING_GAME;
    HireMercStruct.fCopyProfileItemsOver = TRUE;
    HireMercStruct.iTotalContractLength = 365;
    // specify when the merc should arrive
    HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay();

    HireMercStruct.ubProfileID = 12;
    HireMerc(&HireMercStruct);

    HireMercStruct.ubProfileID = 10;
    HireMerc(&HireMercStruct);

    HireMercStruct.ubProfileID = 25;
    HireMerc(&HireMercStruct);

    HireMercStruct.ubProfileID = 2;
    HireMerc(&HireMercStruct);

    HireMercStruct.ubProfileID = 20;
    HireMerc(&HireMercStruct);

    return TRUE;
  } else if ((wcscmp(pFullNameString, L"dleifmaC sirhC") == 0) &&
             (wcscmp(pNickNameString, L"SSR") == 0)) {
    wcscpy(pFullNameString, L"James Bolivar DiGriz");
    wcscpy(pNickNameString, L"DiGriz");
    bGenderFlag = IMP_MALE;
    iHealth = 99;
    iAgility = 80;
    iStrength = 80;
    iDexterity = 99;
    iWisdom = 70;
    iLeadership = 70;

    iMarksmanship = 99;
    iMechanical = 99;
    iExplosives = 99;
    iMedical = 60;

    iSkillA = ELECTRONICS;
    iSkillB = LOCKPICKING;

    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_LONER;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 3;
    return TRUE;
  }
#endif
#endif
  if ((wcscmp(pFullNameString, L"Test Female") == 0) && (wcscmp(pNickNameString, L"Test") == 0)) {
    wcscpy(pFullNameString, L"Test Female");
    wcscpy(pNickNameString, L"Test");
    bGenderFlag = IMP_FEMALE;
    iHealth = 55;
    iAgility = 55;
    iStrength = 55;
    iDexterity = 55;
    iWisdom = 55;
    iLeadership = 55;

    iMarksmanship = 55;
    iMechanical = 55;
    iExplosives = 55;
    iMedical = 55;

    iSkillA = 0;
    iSkillB = 0;

    iPersonality = NO_PERSONALITYTRAIT;
    iAttitude = ATT_LONER;
    iCurrentImpPage = IMP_FINISH;
    LaptopSaveInfo.iVoiceId = 1;
    iPortraitNumber = 5;
    return (TRUE);
  }
  return FALSE;
}
