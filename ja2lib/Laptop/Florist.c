// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/Florist.h"

#include "Laptop/FloristCards.h"
#include "Laptop/FloristGallery.h"
#include "Laptop/FloristOrderForm.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define FLORIST_SENTENCE_FONT FONT12ARIAL
#define FLORIST_SENTENCE_COLOR FONT_MCOLOR_WHITE

#define FLORIST_TITLE_FONT FONT14ARIAL
#define FLORIST_TITLE_COLOR FONT_MCOLOR_WHITE

#define FLORIST_BACKGROUND_WIDTH 125
#define FLORIST_BACKGROUND_HEIGHT 100

#define FLORIST_BIG_TITLE_X LAPTOP_SCREEN_UL_X + 113
#define FLORIST_BIG_TITLE_Y LAPTOP_SCREEN_WEB_UL_Y + 0

#define FLORIST_SMALL_TITLE_X LAPTOP_SCREEN_UL_X + 195
#define FLORIST_SMALL_TITLE_Y LAPTOP_SCREEN_WEB_UL_Y + 0
#define FLORIST_SMALL_TITLE_WIDTH 100
#define FLORIST_SMALL_TITLE_HEIGHT 49

#define FLORIST_FIRST_BULLET_X LAPTOP_SCREEN_UL_X + 5
#define FLORIST_FIRST_BULLET_Y LAPTOP_SCREEN_WEB_UL_Y + 135

#define FLORIST_BULLET_OFFSET_Y 54

#define FLORIST_NUMBER_OF_BULLETS 4

#define FLORIST_GALLERY_BUTTON_X LAPTOP_SCREEN_UL_X + 210
#define FLORIST_GALLERY_BUTTON_Y LAPTOP_SCREEN_WEB_UL_Y + 360

#define FLORIST_FIRST_SENTENCE_COLUMN_TEXT_X LAPTOP_SCREEN_UL_X + 53
#define FLORIST_FIRST_SENTENCE_COLUMN_TEXT_WIDTH 136

#define FLORIST_SECOND_SENTENCE_COLUMN_TEXT_X LAPTOP_SCREEN_UL_X + 200
#define FLORIST_SECOND_SENTENCE_COLUMN_TEXT_WIDTH 300

#define FLORIST_COMPANY_INFO_TEXT_X LAPTOP_SCREEN_UL_X + 117
#define FLORIST_COMPANY_INFO_TEXT_WIDTH 290

#define FLORIST_COMPANY_INFO_LINE_1_Y LAPTOP_SCREEN_WEB_UL_Y + 79
#define FLORIST_COMPANY_INFO_LINE_2_Y LAPTOP_SCREEN_WEB_UL_Y + 94
#define FLORIST_COMPANY_INFO_LINE_3_Y LAPTOP_SCREEN_WEB_UL_Y + 107
#define FLORIST_COMPANY_INFO_LINE_4_Y LAPTOP_SCREEN_WEB_UL_Y + 119

uint32_t guiFloristBackground;
uint32_t guiHandBullet;
uint32_t guiLargeTitleSymbol;
uint32_t guiSmallTitleSymbol;

BOOLEAN gfHomePageActive =
    FALSE;  // Specifies whether or not the home page or the sub pages are active

// Buttons

// Graphic for button
int32_t guiGalleryButtonImage;
void BtnGalleryButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiGalleryButton;

// link to the flower home page by clicking on the flower title
struct MOUSE_REGION gSelectedFloristTitleHomeLinkRegion;
void SelectFloristTitleHomeLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

void GameInitFlorist() {}

BOOLEAN EnterFlorist() {
  SetBookMark(FLORIST_BOOKMARK);

  InitFloristDefaults();

  // load the handbullet graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\HandBullet.sti"), &guiHandBullet));

  guiGalleryButtonImage = LoadButtonImage("LAPTOP\\FloristButtons.sti", -1, 0, -1, 1, -1);

  guiGalleryButton = CreateIconAndTextButton(
      guiGalleryButtonImage, sFloristText[FLORIST_GALLERY], FLORIST_BUTTON_TEXT_FONT,
      FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
      FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR, TEXT_CJUSTIFIED,
      FLORIST_GALLERY_BUTTON_X, FLORIST_GALLERY_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnGalleryButtonCallback);
  SetButtonCursor(guiGalleryButton, CURSOR_WWW);

  // reset the currently selected card
  gbCurrentlySelectedCard = -1;

  // Initialize the Florsit Order Page (reset some variables)
  InitFloristOrderForm();

  // Initialize the flower index for the gallery page
  gubCurFlowerIndex = 0;

  RenderFlorist();
  fReDrawScreenFlag = TRUE;

  // set some variables for the order form
  InitFloristOrderFormVariables();

  return (TRUE);
}

void ExitFlorist() {
  DeleteVideoObjectFromIndex(guiHandBullet);

  RemoveFloristDefaults();

  UnloadButtonImage(guiGalleryButtonImage);

  RemoveButton(guiGalleryButton);
}

void HandleFlorist() {}

void RenderFlorist() {
  struct VObject *hPixHandle;
  uint16_t i, usPosY;
  uint8_t ubTextCounter;

  GetVideoObject(&hPixHandle, guiHandBullet);

  DisplayFloristDefaults();

  // compnay info
  DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_1_Y,
                       FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT,
                       FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_DROP_ANYWHERE],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_2_Y,
                       FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT,
                       FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_PHONE_NUMBER],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_3_Y,
                       FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT,
                       FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_STREET_ADDRESS],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_4_Y,
                       FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT,
                       FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_WWW_ADDRESS], FONT_MCOLOR_BLACK,
                       FALSE, CENTER_JUSTIFIED);

  usPosY = FLORIST_FIRST_BULLET_Y;
  ubTextCounter = FLORIST_ADVERTISEMENT_1;
  for (i = 0; i < FLORIST_NUMBER_OF_BULLETS; i++) {
    BltVideoObject(vsFB, hPixHandle, 0, FLORIST_FIRST_BULLET_X, usPosY);

    DisplayWrappedString(FLORIST_FIRST_SENTENCE_COLUMN_TEXT_X, (uint16_t)(usPosY + 20),
                         FLORIST_FIRST_SENTENCE_COLUMN_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT,
                         FLORIST_SENTENCE_COLOR, sFloristText[ubTextCounter], FONT_MCOLOR_BLACK,
                         FALSE, LEFT_JUSTIFIED);
    ubTextCounter++;

    DisplayWrappedString(FLORIST_SECOND_SENTENCE_COLUMN_TEXT_X, (uint16_t)(usPosY + 15),
                         FLORIST_SECOND_SENTENCE_COLUMN_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT,
                         FLORIST_SENTENCE_COLOR, sFloristText[ubTextCounter], FONT_MCOLOR_BLACK,
                         FALSE, LEFT_JUSTIFIED);
    ubTextCounter++;

    usPosY += FLORIST_BULLET_OFFSET_Y;
  }

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_UL_Y);
}

BOOLEAN InitFloristDefaults() {
  // load the Florist background graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\leafback.sti"), &guiFloristBackground));

  // if its the first page
  if (guiCurrentLaptopMode == LAPTOP_MODE_FLORIST) {
    // load the small title graphic and add it
    CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_LARGEFLORISTSYMBOL), &guiLargeTitleSymbol));
  } else {
    // load the leaf back graphic and add it
    CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_SMALLFLORISTSYMBOL), &guiSmallTitleSymbol));

    // flower title homepage link
    MSYS_DefineRegion(
        &gSelectedFloristTitleHomeLinkRegion, FLORIST_SMALL_TITLE_X, FLORIST_SMALL_TITLE_Y,
        (uint16_t)(FLORIST_SMALL_TITLE_X + FLORIST_SMALL_TITLE_WIDTH),
        (uint16_t)(FLORIST_SMALL_TITLE_Y + FLORIST_SMALL_TITLE_HEIGHT), MSYS_PRIORITY_HIGH,
        CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristTitleHomeLinkRegionCallBack);
    MSYS_AddRegion(&gSelectedFloristTitleHomeLinkRegion);
  }

  return (TRUE);
}

void DisplayFloristDefaults() {
  struct VObject *hPixHandle;

  WebPageTileBackground(4, 4, FLORIST_BACKGROUND_WIDTH, FLORIST_BACKGROUND_HEIGHT,
                        guiFloristBackground);

  // if its the first page
  if (guiCurrentLaptopMode == LAPTOP_MODE_FLORIST) {
    gfHomePageActive = TRUE;
    GetVideoObject(&hPixHandle, guiLargeTitleSymbol);
    BltVideoObject(vsFB, hPixHandle, 0, FLORIST_BIG_TITLE_X, FLORIST_BIG_TITLE_Y);
  } else {
    gfHomePageActive = FALSE;
    GetVideoObject(&hPixHandle, guiSmallTitleSymbol);
    BltVideoObject(vsFB, hPixHandle, 0, FLORIST_SMALL_TITLE_X, FLORIST_SMALL_TITLE_Y);
  }
}

void RemoveFloristDefaults() {
  DeleteVideoObjectFromIndex(guiFloristBackground);

  // if its the first page
  if (gfHomePageActive) {
    // delete the big title
    DeleteVideoObjectFromIndex(guiLargeTitleSymbol);
  } else {
    // delete the little title
    DeleteVideoObjectFromIndex(guiSmallTitleSymbol);

    MSYS_RemoveRegion(&gSelectedFloristTitleHomeLinkRegion);
  }
}

void BtnGalleryButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void SelectFloristTitleHomeLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}
