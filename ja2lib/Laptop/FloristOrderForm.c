// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/FloristOrderForm.h"

#include <stdio.h>

#include "Laptop/Finances.h"
#include "Laptop/Florist.h"
#include "Laptop/FloristCards.h"
#include "Laptop/FloristGallery.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "SGP/ButtonSystem.h"
#include "SGP/English.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Strategic/GameClock.h"
#include "Strategic/Meanwhile.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define FLOWER_ORDEER_TINY_FONT FONT10ARIAL
#define FLOWER_ORDEER_SMALL_FONT FONT12ARIAL
#define FLOWER_ORDEER_BIG_FONT FONT12ARIAL
#define FLOWER_ORDEER_SMALL_COLOR FONT_MCOLOR_WHITE
#define FLOWER_ORDEER_LINK_COLOR FONT_MCOLOR_LTYELLOW
#define FLOWER_ORDEER_DROP_DOWN_FONT FONT12ARIAL
#define FLOWER_ORDEER_DROP_DOWN_COLOR FONT_MCOLOR_WHITE

#define FLOWER_ORDER_STATIC_TEXT_COLOR 76

#define FLOWER_ORDER_FLOWER_BOX_X LAPTOP_SCREEN_UL_X + 7
#define FLOWER_ORDER_FLOWER_BOX_Y LAPTOP_SCREEN_WEB_UL_Y + 63
#define FLOWER_ORDER_FLOWER_BOX_WIDTH 75
#define FLOWER_ORDER_FLOWER_BOX_HEIGHT 100

#define FLOWER_ORDER_SENTIMENT_BOX_X LAPTOP_SCREEN_UL_X + 14
#define FLOWER_ORDER_SENTIMENT_BOX_Y LAPTOP_SCREEN_WEB_UL_Y + 226
#define FLOWER_ORDER_SENTIMENT_BOX_WIDTH 468
#define FLOWER_ORDER_SENTIMENT_BOX_HEIGHT 27

#define FLOWER_ORDER_NAME_BOX_X LAPTOP_SCREEN_UL_X + 60
#define FLOWER_ORDER_NAME_BOX_Y LAPTOP_SCREEN_WEB_UL_Y + 314 - FLOWER_ORDER_SMALLER_PS_OFFSET_Y

#define FLOWER_ORDER_SMALLER_PS_OFFSET_Y 27

#define FLOWER_ORDER_DELIVERY_LOCATION_X LAPTOP_SCREEN_UL_X + 205
#define FLOWER_ORDER_DELIVERY_LOCATION_Y LAPTOP_SCREEN_WEB_UL_Y + 143
#define FLOWER_ORDER_DELIVERY_LOCATION_WIDTH 252
#define FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT 20

#define FLOWER_ORDER_BACK_BUTTON_X LAPTOP_SCREEN_UL_X + 8
#define FLOWER_ORDER_BACK_BUTTON_Y LAPTOP_SCREEN_WEB_UL_Y + 12

#define FLOWER_ORDER_SEND_BUTTON_X LAPTOP_SCREEN_UL_X + 124
#define FLOWER_ORDER_SEND_BUTTON_Y LAPTOP_SCREEN_WEB_UL_Y + 364

#define FLOWER_ORDER_CLEAR_BUTTON_X LAPTOP_SCREEN_UL_X + 215
#define FLOWER_ORDER_CLEAR_BUTTON_Y FLOWER_ORDER_SEND_BUTTON_Y

#define FLOWER_ORDER_GALLERY_BUTTON_X LAPTOP_SCREEN_UL_X + 305
#define FLOWER_ORDER_GALLERY_BUTTON_Y FLOWER_ORDER_SEND_BUTTON_Y

#define FLOWER_ORDER_FLOWER_NAME_X LAPTOP_SCREEN_UL_X + 94
#define FLOWER_ORDER_FLOWER_NAME_Y LAPTOP_SCREEN_WEB_UL_Y + 68

#define FLOWER_ORDER_BOUQUET_NAME_X FLOWER_ORDER_FLOWER_NAME_X
#define FLOWER_ORDER_BOUQUET_NAME_Y \
  FLOWER_ORDER_ORDER_NUM_NAME_Y + 15  // FLOWER_ORDER_FLOWER_NAME_Y + 15

#define FLOWER_ORDER_ORDER_NUM_NAME_X FLOWER_ORDER_BOUQUET_NAME_X
#define FLOWER_ORDER_ORDER_NUM_NAME_Y \
  FLOWER_ORDER_FLOWER_NAME_Y + 15  // FLOWER_ORDER_BOUQUET_NAME_Y + 15

#define FLOWER_ORDER_DATE_X FLOWER_ORDER_FLOWER_NAME_X
#define FLOWER_ORDER_DATE_Y LAPTOP_SCREEN_WEB_UL_Y + 126

#define FLOWER_ORDER_LOCATION_X FLOWER_ORDER_FLOWER_NAME_X
#define FLOWER_ORDER_LOCATION_Y FLOWER_ORDER_DELIVERY_LOCATION_Y + 4

#define FLOWER_ORDER_ADDITIONAL_SERVICES_X FLOWER_ORDER_FLOWER_BOX_X
#define FLOWER_ORDER_ADDITIONAL_SERVICES_Y LAPTOP_SCREEN_WEB_UL_Y + 167

#define FLOWER_ORDER_PERSONAL_SENT_TEXT_X FLOWER_ORDER_ADDITIONAL_SERVICES_X
#define FLOWER_ORDER_PERSONAL_SENT_TEXT_Y LAPTOP_SCREEN_WEB_UL_Y + 212

#define FLOWER_ORDER_PERSONAL_SENT_BOX_X FLOWER_ORDER_SENTIMENT_BOX_X + 5
#define FLOWER_ORDER_PERSONAL_SENT_BOX_Y FLOWER_ORDER_SENTIMENT_BOX_Y + 5
#define FLOWER_ORDER_PERSONAL_SENT_TEXT_WIDTH 457
#define FLOWER_ORDER_PERSONAL_SENT_TEXT_HEIGHT 17  // 44

#define FLOWER_ORDER_BILLING_INFO_X FLOWER_ORDER_ADDITIONAL_SERVICES_X
#define FLOWER_ORDER_BILLING_INFO_Y LAPTOP_SCREEN_WEB_UL_Y + 296 - FLOWER_ORDER_SMALLER_PS_OFFSET_Y

#define FLOWER_ORDER_NAME_TEXT_X FLOWER_ORDER_ADDITIONAL_SERVICES_X
#define FLOWER_ORDER_NAME_TEXT_Y FLOWER_ORDER_NAME_BOX_Y + 4
#define FLOWER_ORDER_NAME_TEXT_WIDTH 50

#define FLOWER_ORDER_NAME_TEXT_BOX_X FLOWER_ORDER_NAME_BOX_X + 3
#define FLOWER_ORDER_NAME_TEXT_BOX_Y FLOWER_ORDER_NAME_BOX_Y + 3
#define FLOWER_ORDER_NAME_TEXT_BOX_WIDTH 257
#define FLOWER_ORDER_NAME_TEXT_BOX_HEIGHT 15

#define FLOWER_ORDER_CHECK_WIDTH 20
#define FLOWER_ORDER_CHECK_HEIGHT 17

#define FLOWER_ORDER_CHECK_BOX_0_X LAPTOP_SCREEN_UL_X + 186
#define FLOWER_ORDER_CHECK_BOX_0_Y FLOWER_ORDER_DATE_Y - 3

#define FLOWER_ORDER_CHECK_BOX_1_X LAPTOP_SCREEN_UL_X + 270
#define FLOWER_ORDER_CHECK_BOX_1_Y FLOWER_ORDER_CHECK_BOX_0_Y

#define FLOWER_ORDER_CHECK_BOX_2_X LAPTOP_SCREEN_UL_X + 123
#define FLOWER_ORDER_CHECK_BOX_2_Y FLOWER_ORDER_ADDITIONAL_SERVICES_Y

#define FLOWER_ORDER_CHECK_BOX_3_X LAPTOP_SCREEN_UL_X + 269
#define FLOWER_ORDER_CHECK_BOX_3_Y FLOWER_ORDER_CHECK_BOX_2_Y

#define FLOWER_ORDER_CHECK_BOX_4_X FLOWER_ORDER_CHECK_BOX_2_X
#define FLOWER_ORDER_CHECK_BOX_4_Y FLOWER_ORDER_CHECK_BOX_2_Y + 25

#define FLOWER_ORDER_CHECK_BOX_5_X FLOWER_ORDER_CHECK_BOX_3_X
#define FLOWER_ORDER_CHECK_BOX_5_Y FLOWER_ORDER_CHECK_BOX_4_Y

#define FLOWER_ORDER_LINK_TO_CARD_GALLERY_X LAPTOP_SCREEN_UL_X + 190
#define FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y \
  LAPTOP_SCREEN_WEB_UL_Y + 284 - FLOWER_ORDER_SMALLER_PS_OFFSET_Y

#define FLOWER_ORDER_DROP_DOWN_LOCATION_X FLOWER_ORDER_DELIVERY_LOCATION_X
#define FLOWER_ORDER_DROP_DOWN_LOCATION_Y FLOWER_ORDER_DELIVERY_LOCATION_Y + 19
#define FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH 230

#define FLOWER_ORDER_DROP_DOWN_CITY_START_X FLOWER_ORDER_DROP_DOWN_LOCATION_X + 5
#define FLOWER_ORDER_DROP_DOWN_CITY_START_Y FLOWER_ORDER_DROP_DOWN_LOCATION_Y + 3

#define FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS 75
#define FLOWER_ORDER_NAME_FIELD_NUM_CHARS 35

typedef struct {
  uint8_t ubNextDayDeliveryCost;
  uint8_t ubWhenItGetsThereCost;
} FlowerOrderLocationStruct;

#define FLOWER_ORDER_NUMBER_OF_DROP_DOWN_LOCATIONS 17

FlowerOrderLocationStruct FlowerOrderLocations[FLOWER_ORDER_NUMBER_OF_DROP_DOWN_LOCATIONS] = {
    {20, 15}, {95, 70}, {100, 75}, {50, 35}, {70, 50}, {45, 35}, {30, 25}, {100, 75}, {100, 75},
    {30, 25}, {95, 70}, {30, 25},  {40, 30}, {45, 35}, {95, 70}, {50, 40}, {40, 30}};

uint32_t guiDeliveryLocation;
uint32_t guiFlowerFrame;
uint32_t guiCurrentlySelectedFlowerImage;
uint32_t guiNameBox;
uint32_t guiPersonalSentiments;
uint32_t guiFlowerOrderCheckBoxButtonImage;
uint32_t guiDropDownBorder;

BOOLEAN gfFLoristCheckBox0Down = FALSE;  // next day delviery
BOOLEAN gfFLoristCheckBox1Down = TRUE;   // when it gets there delivery
BOOLEAN gfFLoristCheckBox2Down = FALSE;
BOOLEAN gfFLoristCheckBox3Down = FALSE;
BOOLEAN gfFLoristCheckBox4Down = FALSE;
BOOLEAN gfFLoristCheckBox5Down = FALSE;

uint32_t guiFlowerPrice;

// drop down menu
enum {
  FLOWER_ORDER_DROP_DOWN_NO_ACTION,
  FLOWER_ORDER_DROP_DOWN_CREATE,
  FLOWER_ORDER_DROP_DOWN_DESTROY,
  FLOWER_ORDER_DROP_DOWN_DISPLAY,
};
// the current mode of the drop down display
uint8_t gubFlowerDestDropDownMode;
uint8_t gubCurrentlySelectedFlowerLocation;

wchar_t gsSentimentTextField[FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS] = {0};
wchar_t gsNameTextField[FLOWER_ORDER_NAME_FIELD_NUM_CHARS] = {0};

// buttons
int32_t guiFlowerOrderButtonImage;

uint8_t gubFlowerOrder_AdditioanalServicePrices[] = {10, 20, 10, 10};

void BtnFlowerOrderBackButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiFlowerOrderBackButton;

void BtnFlowerOrderSendButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiFlowerOrderSendButton;

void BtnFlowerOrderClearButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiFlowerOrderClearButton;

void BtnFlowerOrderGalleryButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiFlowerOrderGalleryButton;

// Clicking on OrderCheckBox
struct MOUSE_REGION gSelectedFloristCheckBoxRegion[6];
void SelectFlorsitCheckBoxRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// link to the card gallery
struct MOUSE_REGION gSelectedFloristCardGalleryLinkRegion;
void SelectFloristCardGalleryLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// link to the flower gallery by clicking on the flower
struct MOUSE_REGION gSelectedFloristGalleryLinkRegion;
void SelectFloristGalleryLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// the drop down for the city
struct MOUSE_REGION gSelectedFloristDropDownRegion;
void SelectFloristDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// to disable the drop down window
struct MOUSE_REGION gSelectedFloristDisableDropDownRegion;
void SelectFloristDisableDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region for the drop down city location area
struct MOUSE_REGION gSelectedFlowerDropDownRegion[FLOWER_ORDER_NUMBER_OF_DROP_DOWN_LOCATIONS];
void SelectFlowerDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectFlowerDropDownMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// to select typing in the personal sentiment box
// struct MOUSE_REGION    gSelectedFloristPersonalSentimentBoxRegion;
// void SelectFloristPersonalSentimentBoxRegionCallBack(struct MOUSE_REGION * pRegion, int32_t
// iReason
// );

void DisplayFloristCheckBox();
void DisplayFlowerDynamicItems();
BOOLEAN CreateDestroyFlowerOrderDestDropDown(uint8_t ubDropDownMode);
void FlowerOrderDrawSelectedCity(uint8_t ubNumber);
void FlowerOrderDisplayShippingLocationCity();
void InitFlowerOrderTextInputBoxes();
void DestroyFlowerOrderTextInputBoxes();
void HandleFloristOrderKeyBoardInput();
void FlowerOrderUserTextFieldCallBack(uint8_t ubID, BOOLEAN fEntering);

void GameInitFloristOrderForm() {}

BOOLEAN EnterFloristOrderForm() {
  uint8_t i;
  char sTemp[40];
  uint16_t usPosX, usWidth, usHeight;

  InitFloristDefaults();

  // load the DeliveryLocation graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\DeliveryLocation.sti"), &guiDeliveryLocation));

  // load the Flower frame graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\FlowerFrame.sti"), &guiFlowerFrame));

  // load the Personel sentiments graphic and add it
  CHECKF(
      AddVObject(CreateVObjectFromFile("LAPTOP\\PersonalSentiments.sti"), &guiPersonalSentiments));

  // load the Name Box graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\NameBox.sti"), &guiNameBox));

  // load the Check Box graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\OrderCheckBox.sti"),
                    &guiFlowerOrderCheckBoxButtonImage));

  // load the currently selected flower bouquet
  sprintf(sTemp, "LAPTOP\\Flower_%d.sti", guiCurrentlySelectedFlower);
  CHECKF(AddVObject(CreateVObjectFromFile(sTemp), &guiCurrentlySelectedFlowerImage));

  // border
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\TactPopUp.sti"), &guiDropDownBorder));

  guiFlowerOrderButtonImage = LoadButtonImage("LAPTOP\\FloristButtons.sti", -1, 0, -1, 1, -1);

  guiFlowerOrderBackButton = CreateIconAndTextButton(
      guiFlowerOrderButtonImage, sOrderFormText[FLORIST_ORDER_BACK], FLORIST_BUTTON_TEXT_FONT,
      FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
      FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR, TEXT_CJUSTIFIED,
      FLOWER_ORDER_BACK_BUTTON_X, FLOWER_ORDER_BACK_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnFlowerOrderBackButtonCallback);
  SetButtonCursor(guiFlowerOrderBackButton, CURSOR_WWW);

  guiFlowerOrderSendButton = CreateIconAndTextButton(
      guiFlowerOrderButtonImage, sOrderFormText[FLORIST_ORDER_SEND], FLORIST_BUTTON_TEXT_FONT,
      FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
      FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR, TEXT_CJUSTIFIED,
      FLOWER_ORDER_SEND_BUTTON_X, FLOWER_ORDER_SEND_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnFlowerOrderSendButtonCallback);
  SetButtonCursor(guiFlowerOrderSendButton, CURSOR_WWW);

  guiFlowerOrderClearButton = CreateIconAndTextButton(
      guiFlowerOrderButtonImage, sOrderFormText[FLORIST_ORDER_CLEAR], FLORIST_BUTTON_TEXT_FONT,
      FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
      FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR, TEXT_CJUSTIFIED,
      FLOWER_ORDER_CLEAR_BUTTON_X, FLOWER_ORDER_CLEAR_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnFlowerOrderClearButtonCallback);
  SetButtonCursor(guiFlowerOrderClearButton, CURSOR_WWW);

  guiFlowerOrderGalleryButton = CreateIconAndTextButton(
      guiFlowerOrderButtonImage, sOrderFormText[FLORIST_ORDER_GALLERY], FLORIST_BUTTON_TEXT_FONT,
      FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
      FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR, TEXT_CJUSTIFIED,
      FLOWER_ORDER_GALLERY_BUTTON_X, FLOWER_ORDER_GALLERY_BUTTON_Y, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnFlowerOrderGalleryButtonCallback);
  SetButtonCursor(guiFlowerOrderGalleryButton, CURSOR_WWW);

  //
  //	The check box mouse regions
  //
  i = 0;
  MSYS_DefineRegion(
      &gSelectedFloristCheckBoxRegion[i], FLOWER_ORDER_CHECK_BOX_0_X, FLOWER_ORDER_CHECK_BOX_0_Y,
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_0_X + FLOWER_ORDER_CHECK_WIDTH),
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_0_Y + FLOWER_ORDER_CHECK_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCheckBoxRegion[i]);
  MSYS_SetRegionUserData(&gSelectedFloristCheckBoxRegion[i], 0, i);
  i++;

  MSYS_DefineRegion(
      &gSelectedFloristCheckBoxRegion[i], FLOWER_ORDER_CHECK_BOX_1_X, FLOWER_ORDER_CHECK_BOX_1_Y,
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_1_X + FLOWER_ORDER_CHECK_WIDTH),
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_1_Y + FLOWER_ORDER_CHECK_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCheckBoxRegion[i]);
  MSYS_SetRegionUserData(&gSelectedFloristCheckBoxRegion[i], 0, i);
  i++;

  MSYS_DefineRegion(
      &gSelectedFloristCheckBoxRegion[i], FLOWER_ORDER_CHECK_BOX_2_X, FLOWER_ORDER_CHECK_BOX_2_Y,
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_2_X + FLOWER_ORDER_CHECK_WIDTH),
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_2_Y + FLOWER_ORDER_CHECK_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCheckBoxRegion[i]);
  MSYS_SetRegionUserData(&gSelectedFloristCheckBoxRegion[i], 0, i);
  i++;

  MSYS_DefineRegion(
      &gSelectedFloristCheckBoxRegion[i], FLOWER_ORDER_CHECK_BOX_3_X, FLOWER_ORDER_CHECK_BOX_3_Y,
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_3_X + FLOWER_ORDER_CHECK_WIDTH),
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_3_Y + FLOWER_ORDER_CHECK_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCheckBoxRegion[i]);
  MSYS_SetRegionUserData(&gSelectedFloristCheckBoxRegion[i], 0, i);
  i++;

  MSYS_DefineRegion(
      &gSelectedFloristCheckBoxRegion[i], FLOWER_ORDER_CHECK_BOX_4_X, FLOWER_ORDER_CHECK_BOX_4_Y,
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_4_X + FLOWER_ORDER_CHECK_WIDTH),
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_4_Y + FLOWER_ORDER_CHECK_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCheckBoxRegion[i]);
  MSYS_SetRegionUserData(&gSelectedFloristCheckBoxRegion[i], 0, i);
  i++;

  MSYS_DefineRegion(
      &gSelectedFloristCheckBoxRegion[i], FLOWER_ORDER_CHECK_BOX_5_X, FLOWER_ORDER_CHECK_BOX_5_Y,
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_5_X + FLOWER_ORDER_CHECK_WIDTH),
      (uint16_t)(FLOWER_ORDER_CHECK_BOX_5_Y + FLOWER_ORDER_CHECK_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCheckBoxRegion[i]);
  MSYS_SetRegionUserData(&gSelectedFloristCheckBoxRegion[i], 0, i);
  i++;

  usPosX =
      StringPixLength(sOrderFormText[FLORIST_ORDER_SELECT_FROM_OURS], FLOWER_ORDEER_SMALL_FONT) +
      2 + FLOWER_ORDER_LINK_TO_CARD_GALLERY_X;
  usWidth =
      StringPixLength(sOrderFormText[FLORIST_ORDER_STANDARDIZED_CARDS], FLOWER_ORDEER_SMALL_FONT);
  usHeight = GetFontHeight(FLOWER_ORDEER_SMALL_FONT);
  MSYS_DefineRegion(&gSelectedFloristCardGalleryLinkRegion, usPosX,
                    FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y, (uint16_t)(usPosX + usWidth),
                    (uint16_t)(FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y + usHeight), MSYS_PRIORITY_HIGH,
                    CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristCardGalleryLinkRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristCardGalleryLinkRegion);

  // flower link
  MSYS_DefineRegion(
      &gSelectedFloristGalleryLinkRegion, FLOWER_ORDER_FLOWER_BOX_X, FLOWER_ORDER_FLOWER_BOX_Y,
      (uint16_t)(FLOWER_ORDER_FLOWER_BOX_X + FLOWER_ORDER_FLOWER_BOX_WIDTH),
      (uint16_t)(FLOWER_ORDER_FLOWER_BOX_Y + FLOWER_ORDER_FLOWER_BOX_HEIGHT), MSYS_PRIORITY_HIGH,
      CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristGalleryLinkRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristGalleryLinkRegion);

  // drop down city location
  MSYS_DefineRegion(
      &gSelectedFloristDropDownRegion, FLOWER_ORDER_DELIVERY_LOCATION_X,
      FLOWER_ORDER_DELIVERY_LOCATION_Y,
      (uint16_t)(FLOWER_ORDER_DELIVERY_LOCATION_X + FLOWER_ORDER_DELIVERY_LOCATION_WIDTH),
      (uint16_t)(FLOWER_ORDER_DELIVERY_LOCATION_Y + FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT),
      MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristDropDownRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristDropDownRegion);

  // to disable the drop down city location
  MSYS_DefineRegion(&gSelectedFloristDisableDropDownRegion, LAPTOP_SCREEN_UL_X,
                    LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y,
                    MSYS_PRIORITY_HIGH + 2, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                    SelectFloristDisableDropDownRegionCallBack);
  MSYS_AddRegion(&gSelectedFloristDisableDropDownRegion);
  MSYS_DisableRegion(&gSelectedFloristDisableDropDownRegion);

  // to select typing in the personal sentiment box
  //	MSYS_DefineRegion( &gSelectedFloristPersonalSentimentBoxRegion,
  // FLOWER_ORDER_SENTIMENT_BOX_X, FLOWER_ORDER_SENTIMENT_BOX_Y,
  //(uint16_t)(FLOWER_ORDER_SENTIMENT_BOX_X + FLOWER_ORDER_SENTIMENT_BOX_WIDTH),
  //(uint16_t)(FLOWER_ORDER_SENTIMENT_BOX_Y + FLOWER_ORDER_SENTIMENT_BOX_HEIGHT),
  // MSYS_PRIORITY_HIGH, 					 CURSOR_WWW, MSYS_NO_CALLBACK,
  // SelectFloristPersonalSentimentBoxRegionCallBack); 	MSYS_AddRegion(
  //&gSelectedFloristPersonalSentimentBoxRegion );

  InitFlowerOrderTextInputBoxes();

  LaptopSaveInfo.uiFlowerOrderNumber += Random(5) + 1;

  RenderFloristOrderForm();

  //	guiFlowerPrice = 0;
  //	gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_NO_ACTION;
  //	gubCurrentlySelectedFlowerLocation = 0;

  return (TRUE);
}

void InitFloristOrderFormVariables() {
  guiFlowerPrice = 0;
  gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_NO_ACTION;
  gubCurrentlySelectedFlowerLocation = 0;
}

void ExitFloristOrderForm() {
  uint8_t i;
  RemoveFloristDefaults();

  DeleteVideoObjectFromIndex(guiDeliveryLocation);
  DeleteVideoObjectFromIndex(guiFlowerFrame);
  DeleteVideoObjectFromIndex(guiNameBox);
  DeleteVideoObjectFromIndex(guiPersonalSentiments);
  DeleteVideoObjectFromIndex(guiFlowerOrderCheckBoxButtonImage);
  DeleteVideoObjectFromIndex(guiCurrentlySelectedFlowerImage);
  DeleteVideoObjectFromIndex(guiDropDownBorder);

  for (i = 0; i < 6; i++) MSYS_RemoveRegion(&gSelectedFloristCheckBoxRegion[i]);

  // card gallery link
  MSYS_RemoveRegion(&gSelectedFloristCardGalleryLinkRegion);

  // flower link
  MSYS_RemoveRegion(&gSelectedFloristGalleryLinkRegion);

  // flower link
  MSYS_RemoveRegion(&gSelectedFloristDropDownRegion);

  // to select typing in the personal sentiment box
  //	MSYS_RemoveRegion( &gSelectedFloristPersonalSentimentBoxRegion);

  // disable the drop down window
  MSYS_RemoveRegion(&gSelectedFloristDisableDropDownRegion);

  UnloadButtonImage(guiFlowerOrderButtonImage);

  RemoveButton(guiFlowerOrderBackButton);
  RemoveButton(guiFlowerOrderSendButton);
  RemoveButton(guiFlowerOrderClearButton);
  RemoveButton(guiFlowerOrderGalleryButton);

  // Store the text fields
  Get16BitStringFromField(1, gsSentimentTextField, ARR_SIZE(gsSentimentTextField));
  Get16BitStringFromField(2, gsNameTextField, ARR_SIZE(gsNameTextField));
  gbCurrentlySelectedCard = -1;

  DestroyFlowerOrderTextInputBoxes();
}

void HandleFloristOrderForm() {
  if (gubFlowerDestDropDownMode != FLOWER_ORDER_DROP_DOWN_NO_ACTION) {
    CreateDestroyFlowerOrderDestDropDown(gubFlowerDestDropDownMode);
  }
  HandleFloristOrderKeyBoardInput();

  RenderAllTextFields();
}

void RenderFloristOrderForm() {
  struct VObject *hPixHandle;
  uint16_t usPosX;
  wchar_t sTemp[640];
  uint32_t uiStartLoc = 0;

  DisplayFloristDefaults();

  // The flowe Delivery location
  GetVideoObject(&hPixHandle, guiDeliveryLocation);
  BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_DELIVERY_LOCATION_X,
                 FLOWER_ORDER_DELIVERY_LOCATION_Y);

  // The flowe Flower Frame
  GetVideoObject(&hPixHandle, guiFlowerFrame);
  BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_FLOWER_BOX_X, FLOWER_ORDER_FLOWER_BOX_Y);

  // The currenltly selected flwoer
  GetVideoObject(&hPixHandle, guiCurrentlySelectedFlowerImage);
  BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_FLOWER_BOX_X + 5, FLOWER_ORDER_FLOWER_BOX_Y + 5);

  // The flowe Name Box
  GetVideoObject(&hPixHandle, guiNameBox);
  BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_NAME_BOX_X, FLOWER_ORDER_NAME_BOX_Y);

  // The flowe Personel sentiments
  GetVideoObject(&hPixHandle, guiPersonalSentiments);
  BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_SENTIMENT_BOX_X, FLOWER_ORDER_SENTIMENT_BOX_Y);

  // Bouquet name, price and order number,text
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_NAME_BOUQUET], FLOWER_ORDER_FLOWER_NAME_X,
                   FLOWER_ORDER_FLOWER_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_PRICE], FLOWER_ORDER_BOUQUET_NAME_X,
                   FLOWER_ORDER_BOUQUET_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_ORDER_NUMBER], FLOWER_ORDER_ORDER_NUM_NAME_X,
                   FLOWER_ORDER_ORDER_NUM_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // The flower name
  usPosX = StringPixLength(sOrderFormText[FLORIST_ORDER_NAME_BOUQUET], FLOWER_ORDEER_SMALL_FONT) +
           5 + FLOWER_ORDER_FLOWER_NAME_X;
  uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * guiCurrentlySelectedFlower;
  LoadEncryptedDataFromFile(FLOR_GALLERY_TEXT_FILE, sTemp, uiStartLoc,
                            FLOR_GALLERY_TEXT_TITLE_SIZE);
  DrawTextToScreen(sTemp, usPosX, FLOWER_ORDER_FLOWER_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Deliverry Date
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_DELIVERY_DATE], FLOWER_ORDER_ORDER_NUM_NAME_X,
                   FLOWER_ORDER_DATE_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Next day
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_NEXT_DAY],
                   FLOWER_ORDER_CHECK_BOX_0_X + FLOWER_ORDER_CHECK_WIDTH + 3,
                   FLOWER_ORDER_CHECK_BOX_0_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // When it get there
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_GETS_THERE],
                   FLOWER_ORDER_CHECK_BOX_1_X + FLOWER_ORDER_CHECK_WIDTH + 3,
                   FLOWER_ORDER_CHECK_BOX_1_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Deliverry locatiuon
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_DELIVERY_LOCATION], FLOWER_ORDER_ORDER_NUM_NAME_X,
                   FLOWER_ORDER_LOCATION_Y, 0, FLOWER_ORDEER_BIG_FONT,
                   FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Additional services
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_ADDITIONAL_CHARGES],
                   FLOWER_ORDER_ADDITIONAL_SERVICES_X, FLOWER_ORDER_ADDITIONAL_SERVICES_Y, 0,
                   FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);

  // crushed bouquet
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_CRUSHED],
                   FLOWER_ORDER_CHECK_BOX_2_X + FLOWER_ORDER_CHECK_WIDTH + 3,
                   FLOWER_ORDER_CHECK_BOX_2_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // black roses
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_BLACK_ROSES],
                   FLOWER_ORDER_CHECK_BOX_3_X + FLOWER_ORDER_CHECK_WIDTH + 3,
                   FLOWER_ORDER_CHECK_BOX_3_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // wilted bouquet
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_WILTED],
                   FLOWER_ORDER_CHECK_BOX_4_X + FLOWER_ORDER_CHECK_WIDTH + 3,
                   FLOWER_ORDER_CHECK_BOX_4_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // fruit cake
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_FRUIT_CAKE],
                   FLOWER_ORDER_CHECK_BOX_5_X + FLOWER_ORDER_CHECK_WIDTH + 3,
                   FLOWER_ORDER_CHECK_BOX_5_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Personal sentiment
  usPosX =
      FLOWER_ORDER_PERSONAL_SENT_TEXT_X +
      StringPixLength(sOrderFormText[FLORIST_ORDER_PERSONAL_SENTIMENTS], FLOWER_ORDEER_BIG_FONT) +
      5;
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_PERSONAL_SENTIMENTS],
                   FLOWER_ORDER_PERSONAL_SENT_TEXT_X, FLOWER_ORDER_PERSONAL_SENT_TEXT_Y, 0,
                   FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_CARD_LENGTH], usPosX,
                   FLOWER_ORDER_PERSONAL_SENT_TEXT_Y + 2, 0, FLOWER_ORDEER_TINY_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Billing information
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_BILLING_INFO], FLOWER_ORDER_BILLING_INFO_X,
                   FLOWER_ORDER_BILLING_INFO_Y, 0, FLOWER_ORDEER_BIG_FONT,
                   FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Billing Name
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_NAME], FLOWER_ORDER_NAME_TEXT_X,
                   FLOWER_ORDER_NAME_TEXT_Y, FLOWER_ORDER_NAME_TEXT_WIDTH, FLOWER_ORDEER_BIG_FONT,
                   FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

  // the text to link to the card gallery
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_SELECT_FROM_OURS],
                   FLOWER_ORDER_LINK_TO_CARD_GALLERY_X, FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y, 0,
                   FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  usPosX =
      StringPixLength(sOrderFormText[FLORIST_ORDER_SELECT_FROM_OURS], FLOWER_ORDEER_SMALL_FONT) +
      5 + FLOWER_ORDER_LINK_TO_CARD_GALLERY_X;
  DrawTextToScreen(sOrderFormText[FLORIST_ORDER_STANDARDIZED_CARDS], usPosX,
                   FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y, 0, FLOWER_ORDEER_BIG_FONT,
                   FLOWER_ORDEER_LINK_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  DisplayFloristCheckBox();

  // display all the things that change for the different bouquet collections
  DisplayFlowerDynamicItems();

  // Display the currently selected city
  FlowerOrderDisplayShippingLocationCity();

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnFlowerOrderBackButtonCallback(GUI_BUTTON *btn, int32_t reason) {
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

void BtnFlowerOrderSendButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      // add an entry in the finacial page for the medical deposit
      AddTransactionToPlayersBook(PURCHASED_FLOWERS, 0, -(int32_t)(guiFlowerPrice));

      if (gubCurrentlySelectedFlowerLocation == 7) {
        // sent to meduna!
        if (gfFLoristCheckBox0Down) {
          HandleFlowersMeanwhileScene(0);
        } else {
          HandleFlowersMeanwhileScene(1);
        }
      }

      // increment the order number
      LaptopSaveInfo.uiFlowerOrderNumber += (1 + Random(2));

      guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
      InitFloristOrderForm();

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

void BtnFlowerOrderClearButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
      InitFloristOrderForm();

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

void BtnFlowerOrderGalleryButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;

      // reset the gallery back to page 0
      gubCurFlowerIndex = 0;

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

void SelectFlorsitCheckBoxRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint32_t uiUserData;

    uiUserData = MSYS_GetRegionUserData(pRegion, 0);

    switch (uiUserData) {
      case 0:
        if (gfFLoristCheckBox0Down) {
          gfFLoristCheckBox0Down = FALSE;
          gfFLoristCheckBox1Down = TRUE;
        } else {
          gfFLoristCheckBox0Down = TRUE;
          gfFLoristCheckBox1Down = FALSE;
        }
        break;
      case 1:
        if (gfFLoristCheckBox1Down) {
          gfFLoristCheckBox1Down = FALSE;
          gfFLoristCheckBox0Down = TRUE;
        } else {
          gfFLoristCheckBox1Down = TRUE;
          gfFLoristCheckBox0Down = FALSE;
        }
        break;
      case 2:
        if (gfFLoristCheckBox2Down)
          gfFLoristCheckBox2Down = FALSE;
        else
          gfFLoristCheckBox2Down = TRUE;
        break;
      case 3:
        if (gfFLoristCheckBox3Down)
          gfFLoristCheckBox3Down = FALSE;
        else
          gfFLoristCheckBox3Down = TRUE;
        break;
      case 4:
        if (gfFLoristCheckBox4Down)
          gfFLoristCheckBox4Down = FALSE;
        else
          gfFLoristCheckBox4Down = TRUE;
        break;
      case 5:
        if (gfFLoristCheckBox5Down)
          gfFLoristCheckBox5Down = FALSE;
        else
          gfFLoristCheckBox5Down = TRUE;
        break;
    }
    DisplayFloristCheckBox();
    fPausedReDrawScreenFlag = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void DisplayFloristCheckBox() {
  struct VObject *hPixHandle;

  // check box
  GetVideoObject(&hPixHandle, guiFlowerOrderCheckBoxButtonImage);
  if (gfFLoristCheckBox0Down)
    BltVideoObject(vsFB, hPixHandle, 1, FLOWER_ORDER_CHECK_BOX_0_X, FLOWER_ORDER_CHECK_BOX_0_Y);
  else
    BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_CHECK_BOX_0_X, FLOWER_ORDER_CHECK_BOX_0_Y);

  // first check box
  GetVideoObject(&hPixHandle, guiFlowerOrderCheckBoxButtonImage);
  if (gfFLoristCheckBox1Down)
    BltVideoObject(vsFB, hPixHandle, 1, FLOWER_ORDER_CHECK_BOX_1_X, FLOWER_ORDER_CHECK_BOX_1_Y);
  else
    BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_CHECK_BOX_1_X, FLOWER_ORDER_CHECK_BOX_1_Y);

  // second check box
  GetVideoObject(&hPixHandle, guiFlowerOrderCheckBoxButtonImage);
  if (gfFLoristCheckBox2Down)
    BltVideoObject(vsFB, hPixHandle, 1, FLOWER_ORDER_CHECK_BOX_2_X, FLOWER_ORDER_CHECK_BOX_2_Y);
  else
    BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_CHECK_BOX_2_X, FLOWER_ORDER_CHECK_BOX_2_Y);

  // third check box
  GetVideoObject(&hPixHandle, guiFlowerOrderCheckBoxButtonImage);
  if (gfFLoristCheckBox3Down)
    BltVideoObject(vsFB, hPixHandle, 1, FLOWER_ORDER_CHECK_BOX_3_X, FLOWER_ORDER_CHECK_BOX_3_Y);
  else
    BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_CHECK_BOX_3_X, FLOWER_ORDER_CHECK_BOX_3_Y);

  // Foiurth check box
  GetVideoObject(&hPixHandle, guiFlowerOrderCheckBoxButtonImage);
  if (gfFLoristCheckBox4Down)
    BltVideoObject(vsFB, hPixHandle, 1, FLOWER_ORDER_CHECK_BOX_4_X, FLOWER_ORDER_CHECK_BOX_4_Y);
  else
    BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_CHECK_BOX_4_X, FLOWER_ORDER_CHECK_BOX_4_Y);

  // fifth check box
  GetVideoObject(&hPixHandle, guiFlowerOrderCheckBoxButtonImage);
  if (gfFLoristCheckBox5Down)
    BltVideoObject(vsFB, hPixHandle, 1, FLOWER_ORDER_CHECK_BOX_5_X, FLOWER_ORDER_CHECK_BOX_5_Y);
  else
    BltVideoObject(vsFB, hPixHandle, 0, FLOWER_ORDER_CHECK_BOX_5_X, FLOWER_ORDER_CHECK_BOX_5_Y);

  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void SelectFloristCardGalleryLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_CARD_GALLERY;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

// display the things that change on the screen
void DisplayFlowerDynamicItems() {
  uint32_t uiStartLoc = 0;
  uint16_t usPosX;
  wchar_t sTemp[640];
  //	wchar_t	sText[ 640 ];
  uint16_t usPrice;
  /*
          //display the card saying
          if( gbCurrentlySelectedCard != -1 )
          {
                  //Get and display the card saying
                  //Display Flower Desc

                  uiStartLoc = FLOR_CARD_TEXT_TITLE_SIZE * + gbCurrentlySelectedCard;
                  LoadEncryptedDataFromFile( FLOR_CARD_TEXT_FILE, sTemp, uiStartLoc,
     FLOR_CARD_TEXT_TITLE_SIZE);

                  CleanOutControlCodesFromString(sTemp, sText);

                  DisplayWrappedString( (uint16_t)(FLOWER_ORDER_SENTIMENT_BOX_X+10),
     (uint16_t)(FLOWER_ORDER_SENTIMENT_BOX_Y+7), FLOWER_ORDER_PERSONAL_SENT_TEXT_WIDTH, 2,
     FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR,  sText, FONT_MCOLOR_BLACK, FALSE,
     LEFT_JUSTIFIED);
          }
  */
  // order number
  usPosX = StringPixLength(sOrderFormText[FLORIST_ORDER_ORDER_NUMBER], FLOWER_ORDEER_SMALL_FONT) +
           5 + FLOWER_ORDER_ORDER_NUM_NAME_X;
  swprintf(sTemp, ARR_SIZE(sTemp), L"%d", LaptopSaveInfo.uiFlowerOrderNumber);
  DrawTextToScreen(sTemp, usPosX, FLOWER_ORDER_ORDER_NUM_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  guiFlowerPrice = 0;
  // the user selected crushed bouquet
  if (gfFLoristCheckBox2Down) guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[0];

  // the user selected blak roses
  if (gfFLoristCheckBox3Down) guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[1];

  // the user selected wilted bouquet
  if (gfFLoristCheckBox4Down) guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[2];

  // the user selected fruit cake
  if (gfFLoristCheckBox5Down) guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[3];

  // price
  usPosX = StringPixLength(sOrderFormText[FLORIST_ORDER_PRICE], FLOWER_ORDEER_SMALL_FONT) + 5 +
           FLOWER_ORDER_BOUQUET_NAME_X;
  uiStartLoc =
      FLOR_GALLERY_TEXT_TOTAL_SIZE * guiCurrentlySelectedFlower + FLOR_GALLERY_TEXT_TITLE_SIZE;
  LoadEncryptedDataFromFile(FLOR_GALLERY_TEXT_FILE, sTemp, uiStartLoc,
                            FLOR_GALLERY_TEXT_PRICE_SIZE);
  swscanf(sTemp, L"%hu", &usPrice);

  // if its the next day delivery
  if (gfFLoristCheckBox0Down)
    guiFlowerPrice +=
        usPrice + FlowerOrderLocations[gubCurrentlySelectedFlowerLocation].ubNextDayDeliveryCost;
  // else its the 'when it gets there' delivery
  else
    guiFlowerPrice +=
        usPrice + FlowerOrderLocations[gubCurrentlySelectedFlowerLocation].ubWhenItGetsThereCost;

  swprintf(sTemp, ARR_SIZE(sTemp), L"$%d.00 %s", guiFlowerPrice,
           pMessageStrings[MSG_USDOLLAR_ABBREVIATION]);
  DrawTextToScreen(sTemp, usPosX, FLOWER_ORDER_BOUQUET_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT,
                   FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
}

void SelectFloristGalleryLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectFloristDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_CREATE;
  }
}

void SelectFloristDisableDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_DESTROY;
  }
}

void SelectFlowerDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubCurrentlySelectedFlowerLocation = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
    FlowerOrderDrawSelectedCity(gubCurrentlySelectedFlowerLocation);
    gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_DESTROY;
  }
}

void SelectFlowerDropDownMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    pRegion->uiFlags |= BUTTON_CLICKED_ON;

    gubCurrentlySelectedFlowerLocation = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
    FlowerOrderDrawSelectedCity(gubCurrentlySelectedFlowerLocation);

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

BOOLEAN CreateDestroyFlowerOrderDestDropDown(uint8_t ubDropDownMode) {
  static uint16_t usHeight;
  static BOOLEAN fMouseRegionsCreated = FALSE;

  switch (ubDropDownMode) {
    case FLOWER_ORDER_DROP_DOWN_NO_ACTION: {
    } break;

    case FLOWER_ORDER_DROP_DOWN_CREATE: {
      uint8_t i;
      uint16_t usPosX, usPosY;
      uint16_t usTemp;
      uint16_t usFontHeight = GetFontHeight(FLOWER_ORDEER_DROP_DOWN_FONT);
      uint8_t ubTextFieldID;

      if (fMouseRegionsCreated) {
        return (FALSE);
        break;
      }

      // Get the current text from the text box
      ubTextFieldID = (uint8_t)GetActiveFieldID();

      // if its the personel sentiment field
      if (ubTextFieldID == 1) {
        Get16BitStringFromField(ubTextFieldID, gsSentimentTextField,
                                ARR_SIZE(gsSentimentTextField));
      } else if (ubTextFieldID == 2) {
        // else its the name field
        Get16BitStringFromField(ubTextFieldID, gsNameTextField, ARR_SIZE(gsNameTextField));
      }

      SetActiveField(0);

      fMouseRegionsCreated = TRUE;

      usPosX = FLOWER_ORDER_DROP_DOWN_CITY_START_X;
      usPosY = FLOWER_ORDER_DROP_DOWN_CITY_START_Y;
      for (i = 0; i < FLOWER_ORDER_NUMBER_OF_DROP_DOWN_LOCATIONS; i++) {
        MSYS_DefineRegion(&gSelectedFlowerDropDownRegion[i], usPosX, (uint16_t)(usPosY + 4),
                          (uint16_t)(usPosX + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH),
                          (uint16_t)(usPosY + usFontHeight), MSYS_PRIORITY_HIGH + 3, CURSOR_WWW,
                          SelectFlowerDropDownMovementCallBack, SelectFlowerDropDownRegionCallBack);
        MSYS_AddRegion(&gSelectedFlowerDropDownRegion[i]);
        MSYS_SetRegionUserData(&gSelectedFlowerDropDownRegion[i], 0, i);

        usPosY += usFontHeight + 2;
      }
      usTemp = FLOWER_ORDER_DROP_DOWN_CITY_START_Y;
      usHeight = usPosY - usTemp + 10;

      gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_DISPLAY;
      MSYS_EnableRegion(&gSelectedFloristDisableDropDownRegion);

      // disable the text entry fields
      //			DisableAllTextFields();
      Get16BitStringFromField(1, gsSentimentTextField, ARR_SIZE(gsSentimentTextField));
      KillTextInputMode();

      // disable the clear order and accept order buttons, (their rendering interferes with the drop
      // down graphics)
    } break;

    case FLOWER_ORDER_DROP_DOWN_DESTROY: {
      uint8_t i;

      if (!fMouseRegionsCreated) break;

      for (i = 0; i < FLOWER_ORDER_NUMBER_OF_DROP_DOWN_LOCATIONS; i++)
        MSYS_RemoveRegion(&gSelectedFlowerDropDownRegion[i]);

      // display the name on the title bar
      ColorFillVSurfaceArea(
          vsFB, FLOWER_ORDER_DROP_DOWN_LOCATION_X + 3, FLOWER_ORDER_DELIVERY_LOCATION_Y + 3,
          FLOWER_ORDER_DROP_DOWN_LOCATION_X + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH,
          FLOWER_ORDER_DELIVERY_LOCATION_Y + FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT - 2,
          rgb32_to_rgb16(FROMRGB(0, 0, 0)));
      DrawTextToScreen(pDeliveryLocationStrings[gubCurrentlySelectedFlowerLocation],
                       FLOWER_ORDER_DROP_DOWN_CITY_START_X + 6,
                       FLOWER_ORDER_DROP_DOWN_CITY_START_Y + 3, 0, FLOWER_ORDEER_DROP_DOWN_FONT,
                       FLOWER_ORDEER_DROP_DOWN_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      // enable the drop down region
      MSYS_DisableRegion(&gSelectedFloristDisableDropDownRegion);

      fPausedReDrawScreenFlag = TRUE;
      fMouseRegionsCreated = FALSE;
      gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_NO_ACTION;

      // enable the text entry fields
      InitFlowerOrderTextInputBoxes();
    } break;

    case FLOWER_ORDER_DROP_DOWN_DISPLAY: {
      uint8_t i;
      uint16_t usPosY, usPosX;
      uint16_t usFontHeight = GetFontHeight(FLOWER_ORDEER_DROP_DOWN_FONT);
      struct VObject *hImageHandle;

      // Display the background for the drop down window
      ColorFillVSurfaceArea(
          vsFB, FLOWER_ORDER_DROP_DOWN_LOCATION_X, FLOWER_ORDER_DROP_DOWN_LOCATION_Y,
          FLOWER_ORDER_DROP_DOWN_LOCATION_X + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH,
          FLOWER_ORDER_DROP_DOWN_LOCATION_Y + usHeight, rgb32_to_rgb16(FROMRGB(0, 0, 0)));

      //
      // Place the border around the background
      //

      GetVideoObject(&hImageHandle, guiDropDownBorder);

      usPosX = usPosY = 0;
      // blit top row of images
      for (i = 10; i < FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH - 10; i += 10) {
        // TOP ROW
        BltVideoObject(vsFB, hImageHandle, 1, i + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                       usPosY + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);

        // BOTTOM ROW
        BltVideoObject(vsFB, hImageHandle, 6, i + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                       usHeight - 10 + 6 + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
      }

      // blit the left and right row of images
      usPosX = 0;
      for (i = 10; i < usHeight - 10; i += 10) {
        BltVideoObject(vsFB, hImageHandle, 3, usPosX + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                       i + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
        BltVideoObject(
            vsFB, hImageHandle, 4,
            usPosX + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH - 4 + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
            i + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
      }

      // blt the corner images for the row
      // top left
      BltVideoObject(vsFB, hImageHandle, 0, 0 + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                     usPosY + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
      // top right
      BltVideoObject(vsFB, hImageHandle, 2,
                     FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH - 10 + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                     usPosY + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
      // bottom left
      BltVideoObject(vsFB, hImageHandle, 5, 0 + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                     usHeight - 10 + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
      // bottom right
      BltVideoObject(vsFB, hImageHandle, 7,
                     FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH - 10 + FLOWER_ORDER_DROP_DOWN_LOCATION_X,
                     usHeight - 10 + FLOWER_ORDER_DROP_DOWN_LOCATION_Y);

      // Display the list of cities
      usPosY = FLOWER_ORDER_DROP_DOWN_CITY_START_Y + 3;
      for (i = 0; i < FLOWER_ORDER_NUMBER_OF_DROP_DOWN_LOCATIONS; i++) {
        DrawTextToScreen(pDeliveryLocationStrings[i], FLOWER_ORDER_DROP_DOWN_CITY_START_X + 6,
                         usPosY, 0, FLOWER_ORDEER_DROP_DOWN_FONT, FLOWER_ORDEER_DROP_DOWN_COLOR,
                         FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
        usPosY += usFontHeight + 2;
      }

      FlowerOrderDrawSelectedCity(gubCurrentlySelectedFlowerLocation);

      InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                       LAPTOP_SCREEN_WEB_LR_Y);
    } break;
  }
  return (TRUE);
}

void FlowerOrderDrawSelectedCity(uint8_t ubNumber) {
  uint16_t usPosY;
  uint16_t usFontHeight = GetFontHeight(FLOWER_ORDEER_DROP_DOWN_FONT);

  usPosY = (usFontHeight + 2) * ubNumber + FLOWER_ORDER_DROP_DOWN_CITY_START_Y;

  // display the name in the list
  ColorFillVSurfaceArea(
      vsFB, FLOWER_ORDER_DROP_DOWN_CITY_START_X, usPosY + 2,
      FLOWER_ORDER_DROP_DOWN_CITY_START_X + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH - 9,
      usPosY + usFontHeight + 4, rgb32_to_rgb16(FROMRGB(255, 255, 255)));

  SetFontShadow(NO_SHADOW);
  DrawTextToScreen(pDeliveryLocationStrings[ubNumber], FLOWER_ORDER_DROP_DOWN_CITY_START_X + 6,
                   (uint16_t)(usPosY + 3), 0, FLOWER_ORDEER_DROP_DOWN_FONT, 2, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);
  SetFontShadow(DEFAULT_SHADOW);

  FlowerOrderDisplayShippingLocationCity();
  SetFontShadow(DEFAULT_SHADOW);
}

void FlowerOrderDisplayShippingLocationCity() {
  // display the name on the title bar
  ColorFillVSurfaceArea(
      vsFB, FLOWER_ORDER_DROP_DOWN_LOCATION_X + 3, FLOWER_ORDER_DELIVERY_LOCATION_Y + 3,
      FLOWER_ORDER_DROP_DOWN_LOCATION_X + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH,
      FLOWER_ORDER_DELIVERY_LOCATION_Y + FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT - 2,
      rgb32_to_rgb16(FROMRGB(0, 0, 0)));
  DrawTextToScreen(pDeliveryLocationStrings[gubCurrentlySelectedFlowerLocation],
                   FLOWER_ORDER_DELIVERY_LOCATION_X + 5, FLOWER_ORDER_DELIVERY_LOCATION_Y + 5, 0,
                   FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
}

void InitFlowerOrderTextInputBoxes() {
  uint32_t uiStartLoc = 0;
  wchar_t sTemp[640];
  wchar_t sText[640];

  InitTextInputMode();
  SetTextInputFont((uint16_t)FONT12ARIAL);
  Set16BPPTextFieldColor(rgb32_to_rgb16(FROMRGB(255, 255, 255)));
  SetBevelColors(rgb32_to_rgb16(FROMRGB(136, 138, 135)), rgb32_to_rgb16(FROMRGB(24, 61, 81)));
  SetTextInputRegularColors(2, FONT_WHITE);
  SetTextInputHilitedColors(FONT_WHITE, 2, 141);
  SetCursorColor(rgb32_to_rgb16(FROMRGB(0, 0, 0)));

  AddUserInputField(FlowerOrderUserTextFieldCallBack);

  if (gbCurrentlySelectedCard != -1) {
    // Get and display the card saying
    // Display Flower Desc

    uiStartLoc = FLOR_CARD_TEXT_TITLE_SIZE * +gbCurrentlySelectedCard;
    LoadEncryptedDataFromFile(FLOR_CARD_TEXT_FILE, sTemp, uiStartLoc, FLOR_CARD_TEXT_TITLE_SIZE);
    CleanOutControlCodesFromString(sTemp, sText);

    wcsncpy(gsSentimentTextField, sText, FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS - 1);

    gbCurrentlySelectedCard = -1;
  }

  if (wcslen(gsSentimentTextField) >= FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS - 2) {
    gsSentimentTextField[FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS - 1] = L'\0';
  }

  // personal sentiment box
  AddTextInputField(FLOWER_ORDER_PERSONAL_SENT_BOX_X, FLOWER_ORDER_PERSONAL_SENT_BOX_Y,
                    FLOWER_ORDER_PERSONAL_SENT_TEXT_WIDTH, FLOWER_ORDER_PERSONAL_SENT_TEXT_HEIGHT,
                    MSYS_PRIORITY_HIGH + 2, gsSentimentTextField,
                    FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS, INPUTTYPE_ASCII);

  // Name box
  AddTextInputField(FLOWER_ORDER_NAME_TEXT_BOX_X, FLOWER_ORDER_NAME_TEXT_BOX_Y,
                    FLOWER_ORDER_NAME_TEXT_BOX_WIDTH, FLOWER_ORDER_NAME_TEXT_BOX_HEIGHT,
                    MSYS_PRIORITY_HIGH + 2, gsNameTextField, FLOWER_ORDER_NAME_FIELD_NUM_CHARS,
                    INPUTTYPE_ASCII);
}

void DestroyFlowerOrderTextInputBoxes() { KillTextInputMode(); }

void HandleFloristOrderKeyBoardInput() {
  InputAtom InputEvent;

  while (DequeueEvent(&InputEvent) == TRUE) {
    if (!HandleTextInput(&InputEvent) && InputEvent.usEvent == KEY_DOWN) {
      uint8_t ubTextFieldID;
      switch (InputEvent.usParam) {
        case ENTER:

          ubTextFieldID = (uint8_t)GetActiveFieldID();

          // if its the personel sentiment field
          if (ubTextFieldID == 1) {
            Get16BitStringFromField(ubTextFieldID, gsSentimentTextField,
                                    ARR_SIZE(gsSentimentTextField));
          } else if (ubTextFieldID == 2) {
            // else its the name field
            Get16BitStringFromField(ubTextFieldID, gsNameTextField, ARR_SIZE(gsNameTextField));
          }

          SetActiveField(0);
          break;

        case ESC:
          SetActiveField(0);
          break;

        default:
          HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam,
                                           InputEvent.usKeyState);
          break;
      }
    }
  }
}

void FlowerOrderUserTextFieldCallBack(uint8_t ubID, BOOLEAN fEntering) {
  if (fEntering) {
    //		SetActiveField(1);
  }
}

// Initialize the Florsit Order Page (reset some variables)
void InitFloristOrderForm() {
  gsSentimentTextField[0] = 0;

  gfFLoristCheckBox0Down = FALSE;  // next day delviery
  gfFLoristCheckBox1Down = TRUE;   // when it gets there delivery
  gfFLoristCheckBox2Down = FALSE;
  gfFLoristCheckBox3Down = FALSE;
  gfFLoristCheckBox4Down = FALSE;
  gfFLoristCheckBox5Down = FALSE;

  guiFlowerPrice = 0;

  gubCurrentlySelectedFlowerLocation = 0;
  gbCurrentlySelectedCard = -1;

  gsSentimentTextField[0] = 0;
  gsNameTextField[0] = 0;
}
