// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/InsuranceComments.h"

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

#define INS_CMNT_TITLE_Y 52 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CMNT_FIRST_BULLET_X 82 + LAPTOP_SCREEN_UL_X
#define INS_CMNT_FIRST_BULLET_Y 75 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CMNT_REDLINE_WIDTH 384

#define INS_CMNT_COMMENT_OFFSET_Y 20

#define INS_CMNT_NEXT_COMMENT_OFFSET_Y 65

#define INS_CMNT_COMMENT_TEXT_WIDTH 364

#define INS_CMNT_LINK_Y 357 + LAPTOP_SCREEN_WEB_UL_Y
#define INS_CMNT_LINK_WIDTH 90
#define INS_CMNT_LINK_HEIGHT 35
#define INS_CMNT_LINK_OFFSET_X 166

uint32_t guiInsCmntBulletImage;

// link to the varios pages
struct MOUSE_REGION gSelectedInsuranceCommentLinkRegion[3];
void SelectInsuranceCommentLinkRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason);

BOOLEAN DisplayComment(uint8_t ubCommentorsName, uint8_t ubComment, uint16_t usPosY);

void GameInitInsuranceComments() {}

BOOLEAN EnterInsuranceComments() {
  uint8_t i;
  uint16_t usPosX;

  InitInsuranceDefaults();

  // load the Insurance bullet graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\bullet.sti"), &guiInsCmntBulletImage));

  usPosX = INS_CMNT_FIRST_BULLET_X - 6;
  for (i = 0; i < 3; i++) {
    MSYS_DefineRegion(&gSelectedInsuranceCommentLinkRegion[i], usPosX, INS_CMNT_LINK_Y - 1,
                      (uint16_t)(usPosX + INS_CMNT_LINK_WIDTH),
                      INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT + 1, MSYS_PRIORITY_HIGH, CURSOR_WWW,
                      MSYS_NO_CALLBACK, SelectInsuranceCommentLinkRegionCallBack);
    MSYS_AddRegion(&gSelectedInsuranceCommentLinkRegion[i]);
    MSYS_SetRegionUserData(&gSelectedInsuranceCommentLinkRegion[i], 0, i);

    usPosX += INS_CMNT_LINK_OFFSET_X;
  }

  RenderInsuranceComments();

  return (TRUE);
}

void ExitInsuranceComments() {
  uint8_t i;
  RemoveInsuranceDefaults();
  DeleteVideoObjectFromIndex(guiInsCmntBulletImage);

  for (i = 0; i < 3; i++) MSYS_RemoveRegion(&gSelectedInsuranceCommentLinkRegion[i]);
}

void HandleInsuranceComments() {}

void RenderInsuranceComments() {
  //  struct VObject* hPixHandle;
  wchar_t sText[800];
  uint16_t usPosX, usPosY;

  SetFontShadow(INS_FONT_SHADOW);

  DisplayInsuranceDefaults();

  // Display the title slogan
  GetInsuranceText(INS_SNGL_COMMENTSFROM_CLIENTS, sText);
  DrawTextToScreen(sText, LAPTOP_SCREEN_UL_X, INS_CMNT_TITLE_Y,
                   LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, INS_FONT_BIG, INS_FONT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  usPosY = INS_CMNT_FIRST_BULLET_Y;

  // Display the commnet from Gus
  DisplayComment(INS_SNGL_GUS_TARBALLS, INS_MLTI_GUS_SPEECH, usPosY);

  usPosY += INS_CMNT_NEXT_COMMENT_OFFSET_Y;

  // Display the commnet from ali hussean
  DisplayComment(INS_SNGL_ALI_HUSSEAN, INS_MLTI_ALI_HUSSEAN_SPEECH, usPosY);

  usPosY += INS_CMNT_NEXT_COMMENT_OFFSET_Y;

  // Display the commnet from Lance allot
  DisplayComment(INS_SNGL_LANCE_ALLOT, INS_MLTI_LANCE_ALLOT_SPEECH, usPosY);

  usPosY += INS_CMNT_NEXT_COMMENT_OFFSET_Y;

  // Display the commnet from Fred Cousteau
  DisplayComment(INS_SNGL_FRED_COUSTEAU, INS_MLTI_FRED_COUSTEAU_SPEECH, usPosY);

  // Display the link text
  usPosX = INS_CMNT_FIRST_BULLET_X - 6;

  // Display the first link text
  swprintf(sText, ARR_SIZE(sText), L"%s", pMessageStrings[MSG_HOMEPAGE]);
  DisplayWrappedString(usPosX, INS_CMNT_LINK_Y + 13, INS_CMNT_LINK_WIDTH, 2, INS_FONT_MED,
                       INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(usPosX, INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT,
                                (uint16_t)(usPosX + INS_CMNT_LINK_WIDTH),
                                INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT);
  usPosX += INS_CMNT_LINK_OFFSET_X;

  // Display the third link text
  GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK, sText);
  DisplayWrappedString(usPosX, INS_CMNT_LINK_Y + 6, INS_CMNT_LINK_WIDTH, 2, INS_FONT_MED,
                       INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(usPosX, INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT,
                                (uint16_t)(usPosX + INS_CMNT_LINK_WIDTH),
                                INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT);
  usPosX += INS_CMNT_LINK_OFFSET_X;

  // Display the fourth link text
  GetInsuranceText(INS_SNGL_TO_ENTER_REVIEW, sText);
  DisplayWrappedString(usPosX, INS_CMNT_LINK_Y - 1, INS_CMNT_LINK_WIDTH, 2, INS_FONT_MED,
                       INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(usPosX, INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT,
                                (uint16_t)(usPosX + INS_CMNT_LINK_WIDTH),
                                INS_CMNT_LINK_Y + INS_CMNT_LINK_HEIGHT);

  SetFontShadow(DEFAULT_SHADOW);
  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void SelectInsuranceCommentLinkRegionCallBack(struct MOUSE_REGION* pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint32_t uiInsuranceLink = MSYS_GetRegionUserData(pRegion, 0);

    if (uiInsuranceLink == 0)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
    else if (uiInsuranceLink == 1)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_INFO;
    else if (uiInsuranceLink == 2)
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

BOOLEAN DisplayComment(uint8_t ubCommentorsName, uint8_t ubComment, uint16_t usPosY) {
  wchar_t sText[800];
  struct VObject* hPixHandle;
  uint16_t sNumPixels = 0;

  // Get and display the insurance bullet
  GetVideoObject(&hPixHandle, guiInsCmntBulletImage);
  BltVideoObject(vsFB, hPixHandle, 0, INS_CMNT_FIRST_BULLET_X, usPosY);

  // Display the commenters comment
  GetInsuranceText(ubComment, sText);  //+INS_CMNT_COMMENT_OFFSET_Y
  sNumPixels =
      DisplayWrappedString(INS_CMNT_FIRST_BULLET_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                           (uint16_t)(usPosY), INS_CMNT_COMMENT_TEXT_WIDTH, 2, INS_FONT_MED,
                           INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the red bar under the link at the bottom
  DisplaySmallRedLineWithShadow(
      INS_CMNT_FIRST_BULLET_X + INSURANCE_BULLET_TEXT_OFFSET_X, (uint16_t)(usPosY + sNumPixels),
      INS_CMNT_FIRST_BULLET_X + INS_CMNT_REDLINE_WIDTH, (uint16_t)(usPosY + sNumPixels));

  sNumPixels += 4;

  // Display the commenters name
  GetInsuranceText(ubCommentorsName, sText);
  DrawTextToScreen(sText, INS_CMNT_FIRST_BULLET_X + INSURANCE_BULLET_TEXT_OFFSET_X,
                   (uint16_t)(usPosY + sNumPixels), INS_CMNT_REDLINE_WIDTH, INS_FONT_MED,
                   INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

  return (TRUE);
}
