// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/MercsAccount.h"

#include "Laptop/Email.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "Laptop/Mercs.h"
#include "Laptop/SpeckQuotes.h"
#include "Money.h"
#include "SGP/ButtonSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Strategic/GameClock.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierProfile.h"
#include "Utils/Cursors.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define MERC_ACCOUNT_TEXT_FONT FONT14ARIAL
#define MERC_ACCOUNT_TEXT_COLOR FONT_MCOLOR_WHITE

#define MERC_ACCOUNT_DYNAMIC_TEXT_FONT FONT12ARIAL
#define MERC_ACCOUNT_DYNAMIC_TEXT_COLOR FONT_MCOLOR_WHITE
#define MERC_ACCOUNT_DEAD_TEXT_COLOR FONT_MCOLOR_RED

#define MERC_AC_ORDER_GRID_X LAPTOP_SCREEN_UL_X + 23
#define MERC_AC_ORDER_GRID_Y LAPTOP_SCREEN_WEB_UL_Y + 59

#define MERC_AC_ACCOUNT_NUMBER_X LAPTOP_SCREEN_UL_X + 23
#define MERC_AC_ACCOUNT_NUMBER_Y LAPTOP_SCREEN_WEB_UL_Y + 13

#define MERC_AC_AUTHORIZE_BUTTON_X 128
#define MERC_AC_AUTHORIZE_BUTTON_Y 380

#define MERC_AC_CANCEL_BUTTON_X 490
#define MERC_AC_CANCEL_BUTTON_Y MERC_AC_AUTHORIZE_BUTTON_Y

#define MERC_AC_ACCOUNT_NUMBER_TEXT_X MERC_AC_ACCOUNT_NUMBER_X + 5
#define MERC_AC_ACCOUNT_NUMBER_TEXT_Y MERC_AC_ACCOUNT_NUMBER_Y + 12

#define MERC_AC_MERC_TITLE_Y MERC_AC_ORDER_GRID_Y + 14
#define MERC_AC_TOTAL_COST_Y MERC_AC_ORDER_GRID_Y + 242

#define MERC_AC_FIRST_COLUMN_X MERC_AC_ORDER_GRID_X + 2
#define MERC_AC_SECOND_COLUMN_X MERC_AC_ORDER_GRID_X + 222
#define MERC_AC_THIRD_COLUMN_X MERC_AC_ORDER_GRID_X + 292
#define MERC_AC_FOURTH_COLUMN_X MERC_AC_ORDER_GRID_X + 382

#define MERC_AC_FIRST_COLUMN_WIDTH 218
#define MERC_AC_SECOND_COLUMN_WIDTH 68
#define MERC_AC_THIRD_COLUMN_WIDTH 88
#define MERC_AC_FOURTH_COLUMN_WIDTH 76

#define MERC_AC_FIRST_ROW_Y MERC_AC_ORDER_GRID_Y + 42
#define MERC_AC_ROW_SIZE 16

uint32_t guiMercOrderGrid;
uint32_t guiAccountNumberGrid;

int32_t giMercTotalContractCharge;

BOOLEAN gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = FALSE;

// The Authorize button
void BtnMercAuthorizeButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiMercAuthorizeBoxButton;
int32_t guiMercAuthorizeButtonImage;

// The Back button
void BtnMercBackButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiMercBackBoxButton;

void DisplayHiredMercs();
void SettleMercAccounts();
void MercAuthorizePaymentMessageBoxCallBack(uint8_t bExitValue);

void GameInitMercsAccount() {}

BOOLEAN EnterMercsAccount() {
  InitMercBackGround();

  // load the Arrow graphic and add it
  CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_ORDERGRID), &guiMercOrderGrid));

  // load the Arrow graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\AccountNumber.sti"), &guiAccountNumberGrid));

  guiMercAuthorizeButtonImage = LoadButtonImage("LAPTOP\\BigButtons.sti", -1, 0, -1, 1, -1);

  guiMercAuthorizeBoxButton = CreateIconAndTextButton(
      guiMercAuthorizeButtonImage, MercAccountText[MERC_ACCOUNT_AUTHORIZE], FONT12ARIAL,
      MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW, MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
      MERC_AC_AUTHORIZE_BUTTON_X, MERC_AC_AUTHORIZE_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnMercAuthorizeButtonCallback);
  SetButtonCursor(guiMercAuthorizeBoxButton, CURSOR_LAPTOP_SCREEN);
  SpecifyDisabledButtonStyle(guiMercAuthorizeBoxButton, DISABLED_STYLE_SHADED);

  guiMercBackBoxButton = CreateIconAndTextButton(
      guiMercAuthorizeButtonImage, MercAccountText[MERC_ACCOUNT_HOME], FONT12ARIAL,
      MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW, MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED,
      MERC_AC_CANCEL_BUTTON_X, MERC_AC_CANCEL_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnMercBackButtonCallback);
  SetButtonCursor(guiMercBackBoxButton, CURSOR_LAPTOP_SCREEN);
  SpecifyDisabledButtonStyle(guiMercBackBoxButton, DISABLED_STYLE_SHADED);

  //	RenderMercsAccount();

  // if true, will display a msgbox telling user that they dont have enough funds
  gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = FALSE;

  return (TRUE);
}

void ExitMercsAccount() {
  DeleteVideoObjectFromIndex(guiMercOrderGrid);
  DeleteVideoObjectFromIndex(guiAccountNumberGrid);

  UnloadButtonImage(guiMercAuthorizeButtonImage);
  RemoveButton(guiMercAuthorizeBoxButton);
  RemoveButton(guiMercBackBoxButton);

  RemoveMercBackGround();
}

void HandleMercsAccount() {
  // if true, will display a msgbox telling user that they dont have enough funds
  if (gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning) {
    gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = FALSE;

    DoLapTopMessageBox(MSG_BOX_BLUE_ON_GREY, L"Transfer failed.  No funds available.",
                       LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
  }
}

void RenderMercsAccount() {
  wchar_t sText[100];
  struct VObject *hPixHandle;

  DrawMecBackGround();

  // Account Number Grid
  GetVideoObject(&hPixHandle, guiMercOrderGrid);
  BltVideoObject(vsFB, hPixHandle, 0, MERC_AC_ORDER_GRID_X, MERC_AC_ORDER_GRID_Y);

  // Merc Order Grid
  GetVideoObject(&hPixHandle, guiAccountNumberGrid);
  BltVideoObject(vsFB, hPixHandle, 0, MERC_AC_ACCOUNT_NUMBER_X, MERC_AC_ACCOUNT_NUMBER_Y);

  // Display Players account number
  swprintf(sText, ARR_SIZE(sText), L"%s %05d", MercAccountText[MERC_ACCOUNT_ACCOUNT],
           LaptopSaveInfo.guiPlayersMercAccountNumber);
  DrawTextToScreen(sText, MERC_AC_ACCOUNT_NUMBER_TEXT_X, MERC_AC_ACCOUNT_NUMBER_TEXT_Y, 0,
                   MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);

  // Display the order grid titles
  DrawTextToScreen(MercAccountText[MERC_ACCOUNT_MERC], MERC_AC_FIRST_COLUMN_X, MERC_AC_MERC_TITLE_Y,
                   MERC_AC_FIRST_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DrawTextToScreen(MercAccountText[MERC_ACCOUNT_DAYS], MERC_AC_SECOND_COLUMN_X,
                   MERC_AC_MERC_TITLE_Y, MERC_AC_SECOND_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT,
                   MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DrawTextToScreen(MercAccountText[MERC_ACCOUNT_RATE], MERC_AC_THIRD_COLUMN_X, MERC_AC_MERC_TITLE_Y,
                   MERC_AC_THIRD_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT, MERC_ACCOUNT_TEXT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DrawTextToScreen(MercAccountText[MERC_ACCOUNT_CHARGE], MERC_AC_FOURTH_COLUMN_X,
                   MERC_AC_MERC_TITLE_Y, MERC_AC_FOURTH_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT,
                   MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DrawTextToScreen(MercAccountText[MERC_ACCOUNT_TOTAL], MERC_AC_THIRD_COLUMN_X,
                   MERC_AC_TOTAL_COST_Y, MERC_AC_THIRD_COLUMN_WIDTH, MERC_ACCOUNT_TEXT_FONT,
                   MERC_ACCOUNT_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  DisplayHiredMercs();

  // giMercTotalContractCharge  gets set with the price in DisplayHiredMercs(), so if there is
  // currently no charge, disable the button
  if (giMercTotalContractCharge == 0) {
    DisableButton(guiMercAuthorizeBoxButton);
  }

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnMercAuthorizeButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      wchar_t wzAuthorizeString[512];
      wchar_t wzDollarAmount[128];

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      swprintf(wzDollarAmount, ARR_SIZE(wzDollarAmount), L"%d", giMercTotalContractCharge);

      InsertCommasForDollarFigure(wzDollarAmount);
      InsertDollarSignInToString(wzDollarAmount);

      // create the string to show to the user
      swprintf(wzAuthorizeString, ARR_SIZE(wzAuthorizeString),
               MercAccountText[MERC_ACCOUNT_AUTHORIZE_CONFIRMATION], wzDollarAmount);

      DoLapTopMessageBox(MSG_BOX_BLUE_ON_GREY, wzAuthorizeString, LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO,
                         MercAuthorizePaymentMessageBoxCallBack);

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

void BtnMercBackButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      guiCurrentLaptopMode = LAPTOP_MODE_MERC;
      gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;

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

void DisplayHiredMercs() {
  uint16_t usPosY;
  uint32_t uiContractCharge;
  wchar_t sTemp[20];
  uint8_t i, usMercID;
  uint8_t ubFontColor;

  giMercTotalContractCharge = 0;

  usPosY = MERC_AC_FIRST_ROW_Y + 3;
  for (i = 0; i <= 10; i++) {
    // if it larry Roach burn advance.  ( cause larry is in twice, a sober larry and a stoned larry
    // )
    if (i == MERC_LARRY_ROACHBURN) continue;

    usMercID = GetMercIDFromMERCArray(i);

    // is the merc on the team, or is owed money
    if (IsMercOnTeam((uint8_t)usMercID) || gMercProfiles[usMercID].iMercMercContractLength != 0) {
      // if the merc is dead, make the color red, else white
      if (IsMercDead(usMercID))
        ubFontColor = MERC_ACCOUNT_DEAD_TEXT_COLOR;
      else
        ubFontColor = MERC_ACCOUNT_DYNAMIC_TEXT_COLOR;

      uiContractCharge = 0;

      // Display Mercs Name
      DrawTextToScreen(gMercProfiles[usMercID].zName, MERC_AC_FIRST_COLUMN_X + 5, usPosY,
                       MERC_AC_FIRST_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor,
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      // Display The # of days the merc has worked since last paid

      swprintf(sTemp, ARR_SIZE(sTemp), L"%d", gMercProfiles[usMercID].iMercMercContractLength);
      DrawTextToScreen(sTemp, MERC_AC_SECOND_COLUMN_X, usPosY, MERC_AC_SECOND_COLUMN_WIDTH,
                       MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

      // Display the mercs rate
      swprintf(sTemp, ARR_SIZE(sTemp), L"$%6d", gMercProfiles[usMercID].sSalary);
      DrawTextToScreen(sTemp, MERC_AC_THIRD_COLUMN_X, usPosY, MERC_AC_THIRD_COLUMN_WIDTH,
                       MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

      // Display the total charge
      uiContractCharge =
          gMercProfiles[usMercID].sSalary * gMercProfiles[usMercID].iMercMercContractLength;
      swprintf(sTemp, ARR_SIZE(sTemp), L"$%6d", uiContractCharge);
      DrawTextToScreen(sTemp, MERC_AC_FOURTH_COLUMN_X, usPosY, MERC_AC_FOURTH_COLUMN_WIDTH,
                       MERC_ACCOUNT_DYNAMIC_TEXT_FONT, ubFontColor, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

      giMercTotalContractCharge += uiContractCharge;
      usPosY += MERC_AC_ROW_SIZE;
    }
  }

  swprintf(sTemp, ARR_SIZE(sTemp), L"$%6d", giMercTotalContractCharge);
  DrawTextToScreen(sTemp, MERC_AC_FOURTH_COLUMN_X, MERC_AC_TOTAL_COST_Y,
                   MERC_AC_FOURTH_COLUMN_WIDTH, MERC_ACCOUNT_DYNAMIC_TEXT_FONT,
                   MERC_ACCOUNT_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
}

void SettleMercAccounts() {
  //	struct SOLDIERTYPE *pSoldier;
  int16_t i;
  uint8_t ubMercID;
  int32_t iPartialPayment = 0;
  int32_t iContractCharge = 0;

  // loop through all the MERC mercs the player has on the team
  for (i = 0; i < NUMBER_OF_MERCS; i++) {
    ubMercID = GetMercIDFromMERCArray((uint8_t)i);

    // if the merc is on the team, or does the player owe money for a fired merc
    if (IsMercOnTeam(ubMercID) || (gMercProfiles[ubMercID].iMercMercContractLength != 0)) {
      // Calc the contract charge
      iContractCharge =
          gMercProfiles[ubMercID].sSalary * gMercProfiles[ubMercID].iMercMercContractLength;

      // if the player can afford to pay this merc
      if (MoneyGetBalance() >= iPartialPayment + iContractCharge) {
        // Increment the counter that keeps track of the of the number of days the player has paid
        // for merc services
        LaptopSaveInfo.guiNumberOfMercPaymentsInDays +=
            gMercProfiles[ubMercID].iMercMercContractLength;

        // Then reset the merc contract counter
        gMercProfiles[ubMercID].iMercMercContractLength = 0;

        // Add this mercs contract charge to the total
        iPartialPayment += iContractCharge;

        gMercProfiles[ubMercID].uiTotalCostToDate += iContractCharge;
      }
    }
  }

  if (iPartialPayment == 0) {
    // if true, will display a msgbox telling user that they dont have enough funds
    gfMercPlayerDoesntHaveEnoughMoney_DisplayWarning = TRUE;
    return;
  }

  // add the transaction to the finance page
  AddTransactionToPlayersBook(PAY_SPECK_FOR_MERC, GetMercIDFromMERCArray(gubCurMercIndex),
                              -iPartialPayment);
  AddHistoryToPlayersLog(HISTORY_SETTLED_ACCOUNTS_AT_MERC, GetMercIDFromMERCArray(gubCurMercIndex),
                         GetWorldTotalMin(), -1, -1);

  // Increment the amount of money paid to speck
  LaptopSaveInfo.uiTotalMoneyPaidToSpeck += iPartialPayment;

  // If the player only made a partial payment
  if (iPartialPayment != giMercTotalContractCharge)
    gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_MAKES_PARTIAL_PAYMENT;
  else {
    gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_MAKES_FULL_PAYMENT;

    // if the merc's account was in suspense, re-enable it
    // CJC Dec 1 2002: an invalid account become valid again.
    // if( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_INVALID )
    LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID;

    // Since the player has paid, make sure speck wont complain about the lack of payment
    LaptopSaveInfo.uiSpeckQuoteFlags &= ~SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT;
  }

  // Go to the merc homepage to say the quote
  guiCurrentLaptopMode = LAPTOP_MODE_MERC;
  gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;
}

void MercAuthorizePaymentMessageBoxCallBack(uint8_t bExitValue) {
  // yes, clear the form
  if (bExitValue == MSG_BOX_RETURN_YES) {
    // if the player owes Speck money, then settle the accounts
    if (giMercTotalContractCharge) SettleMercAccounts();
  }
}

uint32_t CalculateHowMuchPlayerOwesSpeck() {
  uint8_t i = 0;
  uint32_t uiContractCharge = 0;
  uint16_t usMercID;

  for (i = 0; i < 10; i++) {
    // if it larry Roach burn advance.  ( cause larry is in twice, a sober larry and a stoned larry
    // )
    if (i == MERC_LARRY_ROACHBURN) continue;

    usMercID = GetMercIDFromMERCArray(i);
    // if( IsMercOnTeam( (uint8_t)usMercID ) )
    {
      // Calc salary for the # of days the merc has worked since last paid
      uiContractCharge +=
          gMercProfiles[usMercID].sSalary * gMercProfiles[usMercID].iMercMercContractLength;
    }
  }

  return (uiContractCharge);
}
