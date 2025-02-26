// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/IMPAboutUs.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// IMP AboutUs buttons
int32_t giIMPAboutUsButton[1];
int32_t giIMPAboutUsButtonImage[1];
void CreateIMPAboutUsButtons(void);
void DeleteIMPAboutUsButtons(void);
;

// fucntions
void BtnIMPBackCallback(GUI_BUTTON *btn, int32_t reason);

void EnterIMPAboutUs(void) {
  // create buttons
  CreateIMPAboutUsButtons();

  // entry into IMP about us page
  RenderIMPAboutUs();

  return;
}

void ExitIMPAboutUs(void) {
  // exit from IMP About us page

  // delete Buttons
  DeleteIMPAboutUsButtons();

  return;
}

void RenderIMPAboutUs(void) {
  // rneders the IMP about us page

  // the background
  RenderProfileBackGround();

  // the IMP symbol
  RenderIMPSymbol(106, 1);

  // about us indent
  RenderAboutUsIndentFrame(8, 130);
  // about us indent
  RenderAboutUsIndentFrame(258, 130);

  return;
}

void HandleIMPAboutUs(void) {
  // handles the IMP about us page

  return;
}

void CreateIMPAboutUsButtons(void) {
  // this function will create the buttons needed for th IMP about us page
  // the back button button
  giIMPAboutUsButtonImage[0] = LoadButtonImage("LAPTOP\\button_3.sti", -1, 0, -1, 1, -1);
  /*giIMPAboutUsButton[0] = QuickCreateButton( giIMPAboutUsButtonImage[0], LAPTOP_SCREEN_UL_X +  426
     , LAPTOP_SCREEN_WEB_UL_Y + ( 360 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                                                                          BtnGenericMouseMoveButtonCallback,
     (GUI_CALLBACK)BtnIMPBackCallback); */

  giIMPAboutUsButton[0] = CreateIconAndTextButton(
      giIMPAboutUsButtonImage[0], pImpButtonText[6], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW,
      FONT_WHITE, DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 216,
      LAPTOP_SCREEN_WEB_UL_Y + (360), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPBackCallback);

  SetButtonCursor(giIMPAboutUsButton[0], CURSOR_WWW);

  return;
}

void DeleteIMPAboutUsButtons(void) {
  // this function destroys the buttons needed for the IMP about Us Page

  // the about back button
  RemoveButton(giIMPAboutUsButton[0]);
  UnloadButtonImage(giIMPAboutUsButtonImage[0]);

  return;
}

void BtnIMPBackCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP Homepage About US button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      iCurrentImpPage = IMP_HOME_PAGE;
    }
  }
}
