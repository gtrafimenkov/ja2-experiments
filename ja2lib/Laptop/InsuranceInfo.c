// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/InsuranceInfo.h"

#include <string.h>

#include "Laptop/Insurance.h"
#include "Laptop/InsuranceText.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define INS_INFO_FRAUD_TEXT_COLOR FONT_MCOLOR_RED

#define INS_INFO_SUBTITLE_X 86 + LAPTOP_SCREEN_UL_X
#define INS_INFO_SUBTITLE_Y 62 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_INFO_SUBTITLE_LINE_Y INS_INFO_SUBTITLE_Y + 14
#define INS_INFO_SUBTITLE_LINE_WIDTH 375

#define INS_INFO_FIRST_PARAGRAPH_WIDTH INS_INFO_SUBTITLE_LINE_WIDTH
#define INS_INFO_FIRST_PARAGRAPH_X INS_INFO_SUBTITLE_X
#define INS_INFO_FIRST_PARAGRAPH_Y INS_INFO_SUBTITLE_LINE_Y + 9

#define INS_INFO_SPACE_BN_PARAGRAPHS 12

#define INS_INFO_INFO_TOC_TITLE_X 170
#define INS_INFO_INFO_TOC_TITLE_Y 54 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_INFO_TOC_SUBTITLE_X INS_INFO_SUBTITLE_X

#define INS_INFO_LINK_TO_CONTRACT_X 235 + LAPTOP_SCREEN_UL_X
#define INS_INFO_LINK_TO_CONTRACT_Y 392 + LAPTOP_SCREEN_WEB_UL_Y
#define INS_INFO_LINK_TO_CONTRACT_WIDTH 97  // 107

#define INS_INFO_LINK_START_OFFSET 20  // 14
#define INS_INFO_LINK_START_X 262 + INS_INFO_LINK_START_OFFSET
#define INS_INFO_LINK_START_Y 392 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_INFO_LINK_TO_CONTRACT_TEXT_Y 355 + LAPTOP_SCREEN_WEB_UL_Y

uint32_t guiBulletImage;

// The list of Info sub pages
enum {
  INS_INFO_INFO_TOC,
  INS_INFO_SUBMIT_CLAIM,
  INS_INFO_PREMIUMS,
  INS_INFO_RENEWL,
  INS_INFO_CANCELATION,
  INS_INFO_LAST_PAGE,
};
uint8_t gubCurrentInsInfoSubPage = 0;

BOOLEAN InsuranceInfoSubPagesVisitedFlag[INS_INFO_LAST_PAGE - 1];

int32_t guiInsPrevButtonImage;
void BtnInsPrevButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsPrevBackButton;

int32_t guiInsNextButtonImage;
void BtnInsNextButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiInsNextBackButton;

// link to the varios pages
struct MOUSE_REGION gSelectedInsuranceInfoLinkRegion;
void SelectInsuranceLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

struct MOUSE_REGION gSelectedInsuranceInfoHomeLinkRegion;
void SelectInsuranceInfoHomeLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

void DisplaySubmitClaimPage();
void DisplayPremiumPage();
void DisplayRenewingPremiumPage();
void DisplayCancelationPagePage();
void DisableArrowButtonsIfOnLastOrFirstPage();
void ChangingInsuranceInfoSubPage(uint8_t ubSubPageNumber);
void DisplayInfoTocPage();

void GameInitInsuranceInfo() {}

void EnterInitInsuranceInfo() {
  memset(&InsuranceInfoSubPagesVisitedFlag, 0, INS_INFO_LAST_PAGE - 1);
}

BOOLEAN EnterInsuranceInfo() {
  uint16_t usPosX;

  InitInsuranceDefaults();

  // load the Insurance bullet graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\bullet.sti"), &guiBulletImage));

  // left arrow
  guiInsPrevButtonImage = LoadButtonImage("LAPTOP\\InsLeftButton.sti", 2, 0, -1, 1, -1);
  guiInsPrevBackButton = CreateIconAndTextButton(
      guiInsPrevButtonImage, InsInfoText[INS_INFO_PREVIOUS], INS_FONT_BIG, INS_FONT_COLOR,
      INS_FONT_SHADOW, INS_FONT_COLOR, INS_FONT_SHADOW, TEXT_CJUSTIFIED,
      INS_INFO_LEFT_ARROW_BUTTON_X, INS_INFO_LEFT_ARROW_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnInsPrevButtonCallback);
  SetButtonCursor(guiInsPrevBackButton, CURSOR_WWW);
  SpecifyButtonTextOffsets(guiInsPrevBackButton, 17, 16, FALSE);

  // Right arrow
  guiInsNextButtonImage = LoadButtonImage("LAPTOP\\InsRightButton.sti", 2, 0, -1, 1, -1);
  guiInsNextBackButton = CreateIconAndTextButton(
      guiInsNextButtonImage, InsInfoText[INS_INFO_NEXT], INS_FONT_BIG, INS_FONT_COLOR,
      INS_FONT_SHADOW, INS_FONT_COLOR, INS_FONT_SHADOW, TEXT_CJUSTIFIED,
      INS_INFO_RIGHT_ARROW_BUTTON_X, INS_INFO_RIGHT_ARROW_BUTTON_Y, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnInsNextButtonCallback);
  SetButtonCursor(guiInsNextBackButton, CURSOR_WWW);
  SpecifyButtonTextOffsets(guiInsNextBackButton, 18, 16, FALSE);

  usPosX = INS_INFO_LINK_START_X;
  // link to go to the contract page
  // link to go to the home page
  MSYS_DefineRegion(&gSelectedInsuranceInfoHomeLinkRegion, usPosX, INS_INFO_LINK_TO_CONTRACT_Y - 37,
                    (uint16_t)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH),
                    INS_INFO_LINK_TO_CONTRACT_Y + 2, MSYS_PRIORITY_HIGH, CURSOR_WWW,
                    MSYS_NO_CALLBACK, SelectInsuranceInfoHomeLinkRegionCallBack);
  MSYS_AddRegion(&gSelectedInsuranceInfoHomeLinkRegion);

  usPosX += INS_INFO_LINK_START_OFFSET + INS_INFO_LINK_TO_CONTRACT_WIDTH;
  MSYS_DefineRegion(&gSelectedInsuranceInfoLinkRegion, usPosX, INS_INFO_LINK_TO_CONTRACT_Y - 37,
                    (uint16_t)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH),
                    INS_INFO_LINK_TO_CONTRACT_Y + 2, MSYS_PRIORITY_HIGH, CURSOR_WWW,
                    MSYS_NO_CALLBACK, SelectInsuranceLinkRegionCallBack);
  MSYS_AddRegion(&gSelectedInsuranceInfoLinkRegion);

  gubCurrentInsInfoSubPage = INS_INFO_INFO_TOC;

  RenderInsuranceInfo();

  return (TRUE);
}

void ExitInsuranceInfo() {
  RemoveInsuranceDefaults();

  UnloadButtonImage(guiInsPrevButtonImage);
  RemoveButton(guiInsPrevBackButton);

  UnloadButtonImage(guiInsNextButtonImage);
  RemoveButton(guiInsNextBackButton);

  MSYS_RemoveRegion(&gSelectedInsuranceInfoLinkRegion);
  MSYS_RemoveRegion(&gSelectedInsuranceInfoHomeLinkRegion);

  DeleteVideoObjectFromIndex(guiBulletImage);
}

void HandleInsuranceInfo() {}

void RenderInsuranceInfo() {
  wchar_t sText[800];
  uint16_t usPosX;

  DisableArrowButtonsIfOnLastOrFirstPage();

  DisplayInsuranceDefaults();

  SetFontShadow(INS_FONT_SHADOW);

  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_LINE_Y,
                                INS_INFO_SUBTITLE_X + INS_INFO_SUBTITLE_LINE_WIDTH,
                                INS_INFO_SUBTITLE_LINE_Y);

  switch (gubCurrentInsInfoSubPage) {
    case INS_INFO_INFO_TOC:
      DisplayInfoTocPage();
      break;

    case INS_INFO_SUBMIT_CLAIM:
      DisplaySubmitClaimPage();
      break;

    case INS_INFO_PREMIUMS:
      DisplayPremiumPage();
      break;

    case INS_INFO_RENEWL:
      DisplayRenewingPremiumPage();
      break;

    case INS_INFO_CANCELATION:
      DisplayCancelationPagePage();
      break;
  }

  usPosX = INS_INFO_LINK_START_X;

  // Display the red bar under the link at the bottom.  and the text
  DisplaySmallRedLineWithShadow(usPosX, INS_INFO_LINK_TO_CONTRACT_Y,
                                (uint16_t)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH),
                                INS_INFO_LINK_TO_CONTRACT_Y);
  swprintf(sText, ARR_SIZE(sText), L"%s", pMessageStrings[MSG_HOMEPAGE]);
  DisplayWrappedString(usPosX, INS_INFO_LINK_TO_CONTRACT_TEXT_Y + 14,
                       INS_INFO_LINK_TO_CONTRACT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText,
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  usPosX += INS_INFO_LINK_START_OFFSET + INS_INFO_LINK_TO_CONTRACT_WIDTH;

  // Display the red bar under the link at the bottom.  and the text
  DisplaySmallRedLineWithShadow(usPosX, INS_INFO_LINK_TO_CONTRACT_Y,
                                (uint16_t)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH),
                                INS_INFO_LINK_TO_CONTRACT_Y);
  GetInsuranceText(INS_SNGL_TO_ENTER_REVIEW, sText);
  DisplayWrappedString(usPosX, INS_INFO_LINK_TO_CONTRACT_TEXT_Y, INS_INFO_LINK_TO_CONTRACT_WIDTH, 2,
                       INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  SetFontShadow(DEFAULT_SHADOW);

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnInsPrevButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      if (gubCurrentInsInfoSubPage > 0) gubCurrentInsInfoSubPage--;

      ChangingInsuranceInfoSubPage(gubCurrentInsInfoSubPage);
      //			fPausedReDrawScreenFlag = TRUE;

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

void BtnInsNextButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      if (gubCurrentInsInfoSubPage < (INS_INFO_LAST_PAGE - 1)) gubCurrentInsInfoSubPage++;

      ChangingInsuranceInfoSubPage(gubCurrentInsInfoSubPage);

      //			fPausedReDrawScreenFlag = TRUE;

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

void SelectInsuranceLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectInsuranceInfoHomeLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void DisplaySubmitClaimPage() {
  wchar_t sText[800];
  uint16_t usNewLineOffset = 0;
  uint16_t usPosX;

  usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

  // Display the title slogan
  GetInsuranceText(INS_SNGL_SUBMITTING_CLAIM, sText);
  DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the title slogan
  GetInsuranceText(INS_MLTI_U_CAN_REST_ASSURED, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  GetInsuranceText(INS_MLTI_HAD_U_HIRED_AN_INDIVIDUAL, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // display the BIG FRAUD
  GetInsuranceText(INS_SNGL_FRAUD, sText);
  DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, (uint16_t)(usNewLineOffset - 1),
                       INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_BIG, INS_INFO_FRAUD_TEXT_COLOR,
                       sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  usPosX = INS_INFO_FIRST_PARAGRAPH_X + StringPixLength(sText, INS_FONT_BIG) + 2;
  GetInsuranceText(INS_MLTI_WE_RESERVE_THE_RIGHT, sText);
  usNewLineOffset +=
      DisplayWrappedString(usPosX, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  GetInsuranceText(INS_MLTI_SHOULD_THERE_BE_GROUNDS, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  GetInsuranceText(INS_MLTI_SHOULD_SUCH_A_SITUATION, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}

void DisplayPremiumPage() {
  wchar_t sText[800];
  uint16_t usNewLineOffset = 0;
  struct VObject *hPixHandle;

  usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

  // Display the title slogan
  GetInsuranceText(INS_SNGL_PREMIUMS, sText);
  DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  GetInsuranceText(INS_MLTI_EACH_TIME_U_COME_TO_US, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiBulletImage);
  BltVideoObject(vsFB, hPixHandle, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

  GetInsuranceText(INS_MLTI_LENGTH_OF_EMPLOYMENT_CONTRACT, sText);
  usNewLineOffset +=
      DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                           usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiBulletImage);
  BltVideoObject(vsFB, hPixHandle, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

  GetInsuranceText(INS_MLTI_EMPLOYEES_AGE_AND_HEALTH, sText);
  usNewLineOffset +=
      DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                           usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiBulletImage);
  BltVideoObject(vsFB, hPixHandle, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

  GetInsuranceText(INS_MLTI_EMPLOOYEES_TRAINING_AND_EXP, sText);
  usNewLineOffset +=
      DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                           usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}

void DisplayRenewingPremiumPage() {
  wchar_t sText[800];
  uint16_t usNewLineOffset = 0;
  //  struct VObject* hPixHandle;

  usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

  // Display the title slogan
  GetInsuranceText(INS_SNGL_RENEWL_PREMIUMS, sText);
  DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  GetInsuranceText(INS_MLTI_WHEN_IT_COMES_TIME_TO_RENEW, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  GetInsuranceText(INS_MLTI_SHOULD_THE_PROJECT_BE_GOING_WELL, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // display the LOWER PREMIUM FOR RENWING EARLY
  GetInsuranceText(INS_SNGL_LOWER_PREMIUMS_4_RENEWING, sText);
  DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, (uint16_t)(usNewLineOffset - 1),
                       INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_BIG, INS_INFO_FRAUD_TEXT_COLOR,
                       sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS + 2;
}

void DisplayCancelationPagePage() {
  wchar_t sText[800];
  uint16_t usNewLineOffset = 0;

  usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

  // Display the title slogan
  GetInsuranceText(INS_SNGL_POLICY_CANCELATIONS, sText);
  DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  GetInsuranceText(INS_MLTI_WE_WILL_ACCEPT_INS_CANCELATION, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  GetInsuranceText(INS_MLTI_1_HOUR_EXCLUSION_A, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  GetInsuranceText(INS_MLTI_1_HOUR_EXCLUSION_B, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}

void DisableArrowButtonsIfOnLastOrFirstPage() {
  if (gubCurrentInsInfoSubPage == INS_INFO_INFO_TOC)
    DisableButton(guiInsPrevBackButton);
  else
    EnableButton(guiInsPrevBackButton);

  if (gubCurrentInsInfoSubPage == INS_INFO_LAST_PAGE - 1)
    DisableButton(guiInsNextBackButton);
  else
    EnableButton(guiInsNextBackButton);
}

void ChangingInsuranceInfoSubPage(uint8_t ubSubPageNumber) {
  fLoadPendingFlag = TRUE;

  if (InsuranceInfoSubPagesVisitedFlag[ubSubPageNumber] == FALSE) {
    fConnectingToSubPage = TRUE;
    fFastLoadFlag = FALSE;

    InsuranceInfoSubPagesVisitedFlag[ubSubPageNumber] = TRUE;
  } else {
    fConnectingToSubPage = TRUE;
    fFastLoadFlag = TRUE;
  }
}

void DisplayInfoTocPage() {
  wchar_t sText[800];
  uint16_t usNewLineOffset = 0;
  struct VObject *hPixHandle;
  uint16_t usPosY;

  usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

  // Display the title slogan
  GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK, sText);
  DrawTextToScreen(sText, INS_INFO_INFO_TOC_TITLE_X, INS_INFO_INFO_TOC_TITLE_Y, 439, INS_FONT_BIG,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display the First paragraph
  GetInsuranceText(INS_MLTI_HIRING_4_SHORT_TERM_HIGH_RISK_1, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // Display the 2nd paragraph
  GetInsuranceText(INS_MLTI_HIRING_4_SHORT_TERM_HIGH_RISK_2, sText);
  usNewLineOffset += DisplayWrappedString(
      INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
      INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  // Display the sub title
  GetInsuranceText(INS_SNGL_WE_CAN_OFFER_U, sText);
  DrawTextToScreen(sText, INS_INFO_TOC_SUBTITLE_X, usNewLineOffset, 640 - INS_INFO_INFO_TOC_TITLE_X,
                   INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usPosY = usNewLineOffset + 12;
  DisplaySmallRedLineWithShadow(INS_INFO_SUBTITLE_X, usPosY,
                                (uint16_t)(INS_INFO_SUBTITLE_X + INS_INFO_SUBTITLE_LINE_WIDTH),
                                usPosY);

  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  //
  // Premiuns bulleted sentence
  //

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiBulletImage);
  BltVideoObject(vsFB, hPixHandle, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

  GetInsuranceText(INS_MLTI_REASONABLE_AND_FLEXIBLE, sText);
  usNewLineOffset +=
      DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                           usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

  //
  // Quick and efficient claims
  //

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiBulletImage);
  BltVideoObject(vsFB, hPixHandle, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

  GetInsuranceText(INS_MLTI_QUICKLY_AND_EFFICIENT, sText);
  usNewLineOffset +=
      DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                           usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}
