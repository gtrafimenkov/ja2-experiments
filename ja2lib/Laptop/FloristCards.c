// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/FloristCards.h"

#include "Laptop/Florist.h"
#include "Laptop/FloristGallery.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define FLORIST_CARDS_SENTENCE_FONT FONT12ARIAL
#define FLORIST_CARDS_SENTENCE_COLOR FONT_MCOLOR_WHITE

#define FLORIST_CARD_FIRST_POS_X LAPTOP_SCREEN_UL_X + 7
#define FLORIST_CARD_FIRST_POS_Y LAPTOP_SCREEN_WEB_UL_Y + 72
#define FLORIST_CARD_FIRST_OFFSET_X 174
#define FLORIST_CARD_FIRST_OFFSET_Y 109

#define FLORIST_CARD_CARD_WIDTH 135
#define FLORIST_CARD_CARD_HEIGHT 100

#define FLORIST_CARD_TEXT_WIDTH 121
#define FLORIST_CARD_TEXT_HEIGHT 90

#define FLORIST_CARD_TITLE_SENTENCE_X LAPTOP_SCREEN_UL_X
#define FLORIST_CARD_TITLE_SENTENCE_Y LAPTOP_SCREEN_WEB_UL_Y + 53
#define FLORIST_CARD_TITLE_SENTENCE_WIDTH LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X

#define FLORIST_CARD_BACK_BUTTON_X LAPTOP_SCREEN_UL_X + 8
#define FLORIST_CARD_BACK_BUTTON_Y LAPTOP_SCREEN_WEB_UL_Y + 12

#define FLORIST_CARD_
#define FLORIST_CARD_
#define FLORIST_CARD_

uint32_t guiCardBackground;

int8_t gbCurrentlySelectedCard;

// link to the card gallery
struct MOUSE_REGION gSelectedFloristCardsRegion[9];
void SelectFloristCardsRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

int32_t guiFlowerCardsButtonImage;
void BtnFlowerCardsBackButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiFlowerCardsBackButton;

void GameInitFloristCards() {}

BOOLEAN EnterFloristCards() {
  uint16_t i, j, usPosX, usPosY;
  uint8_t ubCount;

  InitFloristDefaults();

  // load the Flower Account Box graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\CardBlank.sti"), &guiCardBackground));

  ubCount = 0;
  usPosY = FLORIST_CARD_FIRST_POS_Y;
  for (j = 0; j < 3; j++) {
    usPosX = FLORIST_CARD_FIRST_POS_X;
    for (i = 0; i < 3; i++) {
      MSYS_DefineRegion(&gSelectedFloristCardsRegion[ubCount], usPosX, usPosY,
                        (uint16_t)(usPosX + FLORIST_CARD_CARD_WIDTH),
                        (uint16_t)(usPosY + FLORIST_CARD_CARD_HEIGHT), MSYS_PRIORITY_HIGH,
                        CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristCardsRegionCallBack);
      MSYS_AddRegion(&gSelectedFloristCardsRegion[ubCount]);
      MSYS_SetRegionUserData(&gSelectedFloristCardsRegion[ubCount], 0, ubCount);
      ubCount++;
      usPosX += FLORIST_CARD_FIRST_OFFSET_X;
    }
    usPosY += FLORIST_CARD_FIRST_OFFSET_Y;
  }

  guiFlowerCardsButtonImage = LoadButtonImage("LAPTOP\\FloristButtons.sti", -1, 0, -1, 1, -1);

  guiFlowerCardsBackButton = CreateIconAndTextButton(
      guiFlowerCardsButtonImage, sFloristCards[FLORIST_CARDS_BACK], FLORIST_BUTTON_TEXT_FONT,
      FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
      FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR, TEXT_CJUSTIFIED,
      FLORIST_CARD_BACK_BUTTON_X, FLORIST_CARD_BACK_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnFlowerCardsBackButtonCallback);
  SetButtonCursor(guiFlowerCardsBackButton, CURSOR_WWW);

  // passing the currently selected card to -1, so it is not used
  gbCurrentlySelectedCard = -1;

  RenderFloristCards();
  return (TRUE);
}

void ExitFloristCards() {
  uint8_t i;

  RemoveFloristDefaults();
  DeleteVideoObjectFromIndex(guiCardBackground);

  // card gallery
  for (i = 0; i < 9; i++) MSYS_RemoveRegion(&gSelectedFloristCardsRegion[i]);

  UnloadButtonImage(guiFlowerCardsButtonImage);
  RemoveButton(guiFlowerCardsBackButton);
}

void HandleFloristCards() {}

void RenderFloristCards() {
  uint8_t i, j, ubCount;
  uint16_t usPosX, usPosY;
  wchar_t sTemp[640];
  uint32_t uiStartLoc = 0;
  struct VObject *hPixHandle;
  uint16_t usHeightOffset;

  DisplayFloristDefaults();

  DrawTextToScreen(sFloristCards[FLORIST_CARDS_CLICK_SELECTION], FLORIST_CARD_TITLE_SENTENCE_X,
                   FLORIST_CARD_TITLE_SENTENCE_Y, FLORIST_CARD_TITLE_SENTENCE_WIDTH, FONT10ARIAL,
                   FLORIST_CARDS_SENTENCE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  GetVideoObject(&hPixHandle, guiCardBackground);
  usPosY = FLORIST_CARD_FIRST_POS_Y;
  ubCount = 0;
  for (j = 0; j < 3; j++) {
    usPosX = FLORIST_CARD_FIRST_POS_X;
    for (i = 0; i < 3; i++) {
      // The flowe account box
      BltVideoObject(vsFB, hPixHandle, 0, usPosX, usPosY);

      // Get and display the card saying
      uiStartLoc = FLOR_CARD_TEXT_TITLE_SIZE * ubCount;
      LoadEncryptedDataFromFile(FLOR_CARD_TEXT_FILE, sTemp, uiStartLoc, FLOR_CARD_TEXT_TITLE_SIZE);

      //			DisplayWrappedString((uint16_t)(usPosX+7), (uint16_t)(usPosY+15),
      // FLORIST_CARD_TEXT_WIDTH, 2, FLORIST_CARDS_SENTENCE_FONT, FLORIST_CARDS_SENTENCE_COLOR,
      // sTemp, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
      usHeightOffset = IanWrappedStringHeight(
          (uint16_t)(usPosX + 7), (uint16_t)(usPosY), FLORIST_CARD_TEXT_WIDTH, 2,
          FLORIST_CARDS_SENTENCE_FONT, FLORIST_CARDS_SENTENCE_COLOR, sTemp, 0, FALSE, 0);

      usHeightOffset = (FLORIST_CARD_TEXT_HEIGHT - usHeightOffset) / 2;

      IanDisplayWrappedString((uint16_t)(usPosX + 7), (uint16_t)(usPosY + 10 + usHeightOffset),
                              FLORIST_CARD_TEXT_WIDTH, 2, FLORIST_CARDS_SENTENCE_FONT,
                              FLORIST_CARDS_SENTENCE_COLOR, sTemp, 0, FALSE, 0);

      ubCount++;
      usPosX += FLORIST_CARD_FIRST_OFFSET_X;
    }
    usPosY += FLORIST_CARD_FIRST_OFFSET_Y;
  }

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void SelectFloristCardsRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gbCurrentlySelectedCard = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_ORDERFORM;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void BtnFlowerCardsBackButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_ORDERFORM;

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
