// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/BobbyRMailOrder.h"

#include <string.h>

#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRGuns.h"
#include "Laptop/Finances.h"
#include "Laptop/Laptop.h"
#include "Money.h"
#include "SGP/ButtonSystem.h"
#include "SGP/FileMan.h"
#include "SGP/Input.h"
#include "SGP/Line.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/SoldierProfile.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

typedef struct {
  uint16_t usOverNightExpress;
  uint16_t us2DaysService;
  uint16_t usStandardService;
} BobbyROrderLocationStruct;

BobbyROrderLocationStruct BobbyROrderLocations[] = {
    {20, 15, 10},
    {295, 150, 85},
    {200 * 5 / 2, 100 * 2, 50 * 2},  // the only one that really matters
    {100, 55, 30},
    {95, 65, 40},
    {55, 40, 25},
    {35, 25, 15},
    {200 * 5 / 2, 100 * 2, 50 * 2},
    {190, 90, 45},
    {35, 25, 15},
    {100, 55, 30},
    {35, 25, 15},
    {45, 30, 20},
    {55, 40, 25},
    {100, 55, 30},
    {100, 55, 30},
    {45, 30, 20},
};

// drop down menu
enum {
  BR_DROP_DOWN_NO_ACTION,
  BR_DROP_DOWN_CREATE,
  BR_DROP_DOWN_DESTROY,
  BR_DROP_DOWN_DISPLAY,
};

#define BOBBYR_ORDER_NUM_SHIPPING_CITIES 17
#define BOBBYR_NUM_DISPLAYED_CITIES 10

#define OVERNIGHT_EXPRESS 1
#define TWO_BUSINESS_DAYS 2
#define STANDARD_SERVICE 3

#define MIN_SHIPPING_WEIGHT 20

#define BOBBYR_ORDER_TITLE_TEXT_FONT FONT14ARIAL
#define BOBBYR_ORDER_TITLE_TEXT_COLOR 157

#define BOBBYR_FONT_BLACK 2

#define BOBBYR_ORDER_STATIC_TEXT_FONT FONT12ARIAL
#define BOBBYR_ORDER_STATIC_TEXT_COLOR 145

#define BOBBYR_DISCLAIMER_FONT FONT10ARIAL

#define BOBBYR_ORDER_DYNAMIC_TEXT_FONT FONT12ARIAL
#define BOBBYR_ORDER_DYNAMIC_TEXT_COLOR FONT_MCOLOR_WHITE

#define BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR FONT_MCOLOR_WHITE

#define BOBBYR_DROPDOWN_FONT FONT12ARIAL

#define BOBBYR_ORDERGRID_X LAPTOP_SCREEN_UL_X + 2
#define BOBBYR_ORDERGRID_Y LAPTOP_SCREEN_WEB_UL_Y + 62

#define BOBBYR_BOBBY_RAY_TITLE_X LAPTOP_SCREEN_UL_X + 171
#define BOBBYR_BOBBY_RAY_TITLE_Y LAPTOP_SCREEN_WEB_UL_Y + 3
#define BOBBYR_BOBBY_RAY_TITLE_WIDTH 160
#define BOBBYR_BOBBY_RAY_TITLE_HEIGHT 35

#define BOBBYR_LOCATION_BOX_X LAPTOP_SCREEN_UL_X + 276
#define BOBBYR_LOCATION_BOX_Y LAPTOP_SCREEN_WEB_UL_Y + 62

#define BOBBYR_DELIVERYSPEED_X LAPTOP_SCREEN_UL_X + 276
#define BOBBYR_DELIVERYSPEED_Y LAPTOP_SCREEN_WEB_UL_Y + 149

#define BOBBYR_CLEAR_ORDER_X LAPTOP_SCREEN_UL_X + 309
#define BOBBYR_CLEAR_ORDER_Y LAPTOP_SCREEN_WEB_UL_Y + 268  // LAPTOP_SCREEN_WEB_UL_Y + 252

#define BOBBYR_ACCEPT_ORDER_X LAPTOP_SCREEN_UL_X + 299
#define BOBBYR_ACCEPT_ORDER_Y LAPTOP_SCREEN_WEB_UL_Y + 303  // LAPTOP_SCREEN_WEB_UL_Y + 288

#define BOBBYR_GRID_ROW_OFFSET 20
#define BOBBYR_GRID_TITLE_OFFSET 27

#define BOBBYR_GRID_FIRST_COLUMN_X 3   // BOBBYR_ORDERGRID_X + 3
#define BOBBYR_GRID_FIRST_COLUMN_Y 37  // BOBBYR_ORDERGRID_Y + 37
#define BOBBYR_GRID_FIRST_COLUMN_WIDTH 23

#define BOBBYR_GRID_SECOND_COLUMN_X 28  // BOBBYR_ORDERGRID_X + 28
#define BOBBYR_GRID_SECOND_COLUMN_Y BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_SECOND_COLUMN_WIDTH 40

#define BOBBYR_GRID_THIRD_COLUMN_X 70  // BOBBYR_ORDERGRID_X + 70
#define BOBBYR_GRID_THIRD_COLUMN_Y BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_THIRD_COLUMN_WIDTH 111

#define BOBBYR_GRID_FOURTH_COLUMN_X 184  // BOBBYR_ORDERGRID_X + 184
#define BOBBYR_GRID_FOURTH_COLUMN_Y BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_FOURTH_COLUMN_WIDTH 40

#define BOBBYR_GRID_FIFTH_COLUMN_X 224  // BOBBYR_ORDERGRID_X + 224
#define BOBBYR_GRID_FIFTH_COLUMN_Y BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_FIFTH_COLUMN_WIDTH 42

#define BOBBYR_SUBTOTAL_WIDTH 212
#define BOBBYR_SUBTOTAL_X BOBBYR_GRID_FIRST_COLUMN_X
#define BOBBYR_SUBTOTAL_Y BOBBYR_GRID_FIRST_COLUMN_Y + BOBBYR_GRID_ROW_OFFSET * 10 + 3

#define BOBBYR_SHIPPING_N_HANDLE_Y BOBBYR_SUBTOTAL_Y + 17
#define BOBBYR_GRAND_TOTAL_Y BOBBYR_SHIPPING_N_HANDLE_Y + 20

#define BOBBYR_SHIPPING_LOCATION_TEXT_X BOBBYR_LOCATION_BOX_X + 8
#define BOBBYR_SHIPPING_LOCATION_TEXT_Y BOBBYR_LOCATION_BOX_Y + 8

#define BOBBYR_SHIPPING_SPEED_X BOBBYR_SHIPPING_LOCATION_TEXT_X
#define BOBBYR_SHIPPING_SPEED_Y BOBBYR_DELIVERYSPEED_Y + 11

#define BOBBYR_SHIPPING_COST_X BOBBYR_SHIPPING_SPEED_X + 130

#define BOBBYR_OVERNIGHT_EXPRESS_Y BOBBYR_DELIVERYSPEED_Y + 42

#define BOBBYR_ORDER_FORM_TITLE_X BOBBYR_BOBBY_RAY_TITLE_X
#define BOBBYR_ORDER_FORM_TITLE_Y BOBBYR_BOBBY_RAY_TITLE_Y + 37
#define BOBBYR_ORDER_FORM_TITLE_WIDTH 159

#define BOBBYR_BACK_BUTTON_X 130
#define BOBBYR_BACK_BUTTON_Y 400 + LAPTOP_SCREEN_WEB_DELTA_Y + 4

#define BOBBYR_HOME_BUTTON_X 515
#define BOBBYR_HOME_BUTTON_Y BOBBYR_BACK_BUTTON_Y

#define BOBBYR_SHIPMENT_BUTTON_X \
  (LAPTOP_SCREEN_UL_X + (LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X - 75) / 2)
#define BOBBYR_SHIPMENT_BUTTON_Y BOBBYR_BACK_BUTTON_Y

#define SHIPPING_SPEED_LIGHT_WIDTH 9
#define SHIPPING_SPEED_LIGHT_HEIGHT 9

#define BOBBYR_CONFIRM_ORDER_X 220
#define BOBBYR_CONFIRM_ORDER_Y 170

#define BOBBYR_CITY_START_LOCATION_X BOBBYR_LOCATION_BOX_X + 6
#define BOBBYR_CITY_START_LOCATION_Y BOBBYR_LOCATION_BOX_Y + 61
#define BOBBYR_DROP_DOWN_WIDTH 182  // 203
#define BOBBYR_DROP_DOWN_HEIGHT 19
#define BOBBYR_CITY_NAME_OFFSET 6

#define BOBBYR_SCROLL_AREA_X BOBBYR_CITY_START_LOCATION_X + BOBBYR_DROP_DOWN_WIDTH
#define BOBBYR_SCROLL_AREA_Y BOBBYR_CITY_START_LOCATION_Y
#define BOBBYR_SCROLL_AREA_WIDTH 22
#define BOBBYR_SCROLL_AREA_HEIGHT 139
#define BOBBYR_SCROLL_AREA_HEIGHT_MINUS_ARROWS \
  (BOBBYR_SCROLL_AREA_HEIGHT - (2 * BOBBYR_SCROLL_ARROW_HEIGHT) - 8)

#define BOBBYR_SCROLL_UP_ARROW_X BOBBYR_SCROLL_AREA_X
#define BOBBYR_SCROLL_UP_ARROW_Y BOBBYR_SCROLL_AREA_Y + 5
#define BOBBYR_SCROLL_DOWN_ARROW_X BOBBYR_SCROLL_UP_ARROW_X
#define BOBBYR_SCROLL_DOWN_ARROW_Y BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT - 24
#define BOBBYR_SCROLL_ARROW_WIDTH 18
#define BOBBYR_SCROLL_ARROW_HEIGHT 20

#define BOBBYR_SHIPPING_LOC_AREA_L_X BOBBYR_LOCATION_BOX_X + 9
#define BOBBYR_SHIPPING_LOC_AREA_T_Y BOBBYR_LOCATION_BOX_Y + 39

#define BOBBYR_SHIPPING_LOC_AREA_R_X BOBBYR_LOCATION_BOX_X + 206
#define BOBBYR_SHIPPING_LOC_AREA_B_Y BOBBYR_LOCATION_BOX_Y + 57

#define BOBBYR_SHIPPING_SPEED_NUMBER_X BOBBYR_SHIPPING_COST_X
#define BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH 37
#define BOBBYR_SHIPPING_SPEED_NUMBER_1_Y BOBBYR_OVERNIGHT_EXPRESS_Y

#define BOBBYR_SHIPPING_SPEED_NUMBER_2_Y BOBBYR_OVERNIGHT_EXPRESS_Y
#define BOBBYR_SHIPPING_SPEED_NUMBER_3_Y BOBBYR_OVERNIGHT_EXPRESS_Y

#define BOBBYR_TOTAL_SAVED_AREA_X BOBBYR_ORDERGRID_X + 221
#define BOBBYR_TOTAL_SAVED_AREA_Y BOBBYR_ORDERGRID_Y + 237

#define BOBBYR_USED_WARNING_X 122
#define BOBBYR_USED_WARNING_Y 382 + LAPTOP_SCREEN_WEB_DELTA_Y

#define BOBBYR_PACKAXGE_WEIGHT_X BOBBYR_LOCATION_BOX_X
#define BOBBYR_PACKAXGE_WEIGHT_Y LAPTOP_SCREEN_WEB_UL_Y + 249
#define BOBBYR_PACKAXGE_WEIGHT_WIDTH 188

uint16_t gShippingSpeedAreas[] = {585, 218 + LAPTOP_SCREEN_WEB_DELTA_Y,
                                  585, 238 + LAPTOP_SCREEN_WEB_DELTA_Y,
                                  585, 258 + LAPTOP_SCREEN_WEB_DELTA_Y};

// Identifier for the images
uint32_t guiBobbyRayTitle;
uint32_t guiBobbyROrderGrid;
uint32_t guiBobbyRLocationGraphic;
uint32_t guiDeliverySpeedGraphic;
uint32_t guiConfirmGraphic;
uint32_t guiTotalSaveArea;  // used as a savebuffer for the subtotal, s&h, and grand total values
uint32_t guiGoldArrowImages;
uint32_t guiPackageWeightImage;

BOOLEAN gfReDrawBobbyOrder = FALSE;

int32_t giGrandTotal;
uint32_t guiShippingCost;
uint32_t guiSubTotal;

uint8_t gubSelectedLight;

BOOLEAN gfDrawConfirmOrderGrpahic;
BOOLEAN gfDestroyConfirmGrphiArea;

BOOLEAN gfCanAcceptOrder;

uint8_t gubDropDownAction;
int8_t gbSelectedCity = -1;  // keeps track of the currently selected city
uint8_t gubCityAtTopOfList;

BOOLEAN gfRemoveItemsFromStock = FALSE;

NewBobbyRayOrderStruct *gpNewBobbyrShipments;
int32_t giNumberOfNewBobbyRShipment;

//
// Buttons
//

// Clear Order Button
void BtnBobbyRClearOrderCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiBobbyRClearOrder;
int32_t guiBobbyRClearOrderImage;

// Accept Order Button
void BtnBobbyRAcceptOrderCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiBobbyRAcceptOrder;
int32_t guiBobbyRAcceptOrderImage;

// Back Button
void BtnBobbyRBackCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiBobbyRBack;
int32_t guiBobbyRBackImage;

// Home Button
void BtnBobbyRHomeCallback(GUI_BUTTON *btn, int32_t reason);
extern uint32_t guiBobbyRHome;
extern int32_t guiBobbyRHomeImage;

// Goto Shipment Page Button
void BtnBobbyRGotoShipmentPageCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiBobbyRGotoShipmentPage;
int32_t giBobbyRGotoShipmentPageImage;

// mouse region for the shipping speed selection area
struct MOUSE_REGION gSelectedShippingSpeedRegion[3];
void SelectShippingSpeedRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region for the confirm area
struct MOUSE_REGION gSelectedConfirmOrderRegion;
void SelectConfirmOrderRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region for the drop down city location area
struct MOUSE_REGION gSelectedDropDownRegion[BOBBYR_ORDER_NUM_SHIPPING_CITIES];
void SelectDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectDropDownMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region for scroll area for the drop down city location area
struct MOUSE_REGION gSelectedScrollAreaDropDownRegion[BOBBYR_ORDER_NUM_SHIPPING_CITIES];
void SelectScrollAreaDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectScrollAreaDropDownMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region to activate the shipping location drop down
struct MOUSE_REGION gSelectedActivateCityDroDownRegion;
void SelectActivateCityDroDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region to close the drop down menu
struct MOUSE_REGION gSelectedCloseDropDownRegion;
void SelectCloseDroDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region to click on the title to go to the home page
struct MOUSE_REGION gSelectedTitleLinkRegion;
void SelectTitleLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// mouse region to click on the up or down arrow on the scroll area
struct MOUSE_REGION gSelectedUpDownArrowOnScrollAreaRegion[2];
void SelectUpDownArrowOnScrollAreaRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

BOOLEAN DrawShippingSpeedLights(uint8_t ubSelectedLight);
BOOLEAN CreateDestroyBobbyRDropDown(uint8_t ubDropDownAction);
void DrawSelectedCity(uint8_t ubNumber);
void DisplayShippingLocationCity();
void DisplayShippingCosts(BOOLEAN fCalledFromOrderPage, int32_t iSubTotal, uint16_t usGridX,
                          uint16_t usGridY, int32_t iOrderNum);
void RemovePurchasedItemsFromBobbyRayInventory();
BOOLEAN IsAnythingPurchasedFromBobbyRayPage();
void DrawGoldRectangle(int8_t bCityNum);
uint32_t CalcCostFromWeightOfPackage(uint8_t ubTypeOfService);
void ConfirmBobbyRPurchaseMessageBoxCallBack(uint8_t bExitValue);
void PurchaseBobbyOrder();
uint32_t CalcPackageTotalWeight();
void DisplayPackageWeight();
void ShutDownBobbyRNewMailOrders();
// ppp

void GameInitBobbyRMailOrder() {
  gubSelectedLight = 0;

  gpNewBobbyrShipments = NULL;
  giNumberOfNewBobbyRShipment = 0;
}

BOOLEAN EnterBobbyRMailOrder() {
  uint16_t i;

  gfReDrawBobbyOrder = FALSE;
  gfDrawConfirmOrderGrpahic = FALSE;
  gfDestroyConfirmGrphiArea = FALSE;
  gfCanAcceptOrder = TRUE;
  gubDropDownAction = BR_DROP_DOWN_NO_ACTION;

  // load the Order Grid graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyOrderGrid.sti"), &guiBobbyROrderGrid));

  // load the Location graphic and add it
  CHECKF(
      AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyLocationBox.sti"), &guiBobbyRLocationGraphic));

  // load the delivery speed graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyDeliverySpeed.sti"),
                    &guiDeliverySpeedGraphic));

  // load the delivery speed graphic and add it
  CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_CONFIRMORDER), &guiConfirmGraphic));

  // load the delivery speed graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\TotalSaveArea.sti"), &guiTotalSaveArea));

  // border
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\TactPopUp.sti"), &guiDropDownBorder));

  // Gold Arrow for the scroll area
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\GoldArrows.sti"), &guiGoldArrowImages));

  // Package Weight Graphic
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\PackageWeight.sti"), &guiPackageWeightImage));

  InitBobbyRWoodBackground();

  //
  // Init the button areas
  //

  // Clear Order button
  guiBobbyRClearOrderImage = LoadButtonImage("LAPTOP\\EraseOrderButton.sti", -1, 0, -1, 1, -1);
  guiBobbyRClearOrder = CreateIconAndTextButton(
      guiBobbyRClearOrderImage, BobbyROrderFormText[BOBBYR_CLEAR_ORDER],
      BOBBYR_ORDER_TITLE_TEXT_FONT, BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
      BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED, BOBBYR_CLEAR_ORDER_X,
      BOBBYR_CLEAR_ORDER_Y + 4, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnBobbyRClearOrderCallback);
  SetButtonCursor(guiBobbyRClearOrder, CURSOR_LAPTOP_SCREEN);
  SpecifyDisabledButtonStyle(guiBobbyRClearOrder, DISABLED_STYLE_NONE);
  SpecifyButtonTextOffsets(guiBobbyRClearOrder, 39, 10, TRUE);

  // Accept Order button
  guiBobbyRAcceptOrderImage = LoadButtonImage("LAPTOP\\AcceptOrderButton.sti", 2, 0, -1, 1, -1);
  guiBobbyRAcceptOrder = CreateIconAndTextButton(
      guiBobbyRAcceptOrderImage, BobbyROrderFormText[BOBBYR_ACCEPT_ORDER],
      BOBBYR_ORDER_TITLE_TEXT_FONT, BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
      BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED, BOBBYR_ACCEPT_ORDER_X,
      BOBBYR_ACCEPT_ORDER_Y + 4, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnBobbyRAcceptOrderCallback);
  SetButtonCursor(guiBobbyRAcceptOrder, CURSOR_LAPTOP_SCREEN);
  SpecifyButtonTextOffsets(guiBobbyRAcceptOrder, 43, 24, TRUE);

  SpecifyDisabledButtonStyle(guiBobbyRAcceptOrder, DISABLED_STYLE_SHADED);

  if (gbSelectedCity == -1) DisableButton(guiBobbyRAcceptOrder);

  // if there is anything to buy, dont disable the accept button
  //	if( !IsAnythingPurchasedFromBobbyRayPage() )
  {
    //		DisableButton( guiBobbyRAcceptOrder );
  }

  guiBobbyRBackImage = LoadButtonImage("LAPTOP\\CatalogueButton.sti", -1, 0, -1, 1, -1);
  guiBobbyRBack = CreateIconAndTextButton(
      guiBobbyRBackImage, BobbyROrderFormText[BOBBYR_BACK], BOBBYR_GUNS_BUTTON_FONT,
      BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR, BOBBYR_GUNS_TEXT_COLOR_OFF,
      BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED, BOBBYR_BACK_BUTTON_X, BOBBYR_BACK_BUTTON_Y,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnBobbyRBackCallback);
  SetButtonCursor(guiBobbyRBack, CURSOR_LAPTOP_SCREEN);

  guiBobbyRHomeImage = UseLoadedButtonImage(guiBobbyRBackImage, -1, 0, -1, 1, -1);
  guiBobbyRHome = CreateIconAndTextButton(
      guiBobbyRHomeImage, BobbyROrderFormText[BOBBYR_HOME], BOBBYR_GUNS_BUTTON_FONT,
      BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR, BOBBYR_GUNS_TEXT_COLOR_OFF,
      BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED, BOBBYR_HOME_BUTTON_X, BOBBYR_HOME_BUTTON_Y,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnBobbyRHomeCallback);
  SetButtonCursor(guiBobbyRHome, CURSOR_LAPTOP_SCREEN);

  giBobbyRGotoShipmentPageImage = UseLoadedButtonImage(guiBobbyRBackImage, -1, 0, -1, 1, -1);
  guiBobbyRGotoShipmentPage = CreateIconAndTextButton(
      giBobbyRGotoShipmentPageImage, BobbyROrderFormText[BOBBYR_GOTOSHIPMENT_PAGE],
      BOBBYR_GUNS_BUTTON_FONT, BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
      BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR, TEXT_CJUSTIFIED,
      BOBBYR_SHIPMENT_BUTTON_X, BOBBYR_SHIPMENT_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      DEFAULT_MOVE_CALLBACK, BtnBobbyRGotoShipmentPageCallback);
  SetButtonCursor(guiBobbyRGotoShipmentPage, CURSOR_LAPTOP_SCREEN);

  for (i = 0; i < 3; i++) {
    MSYS_DefineRegion(&gSelectedShippingSpeedRegion[i], gShippingSpeedAreas[i * 2],
                      gShippingSpeedAreas[i * 2 + 1],
                      (uint16_t)(gShippingSpeedAreas[i * 2] + SHIPPING_SPEED_LIGHT_WIDTH),
                      (uint16_t)(gShippingSpeedAreas[i * 2 + 1] + SHIPPING_SPEED_LIGHT_HEIGHT),
                      MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
                      SelectShippingSpeedRegionCallBack);
    MSYS_AddRegion(&gSelectedShippingSpeedRegion[i]);
    MSYS_SetRegionUserData(&gSelectedShippingSpeedRegion[i], 0, i);
  }

  // confirmorder mouse region, occupies the entrie screen and is present only when the confirm
  // order graphic s on screen.  When user clicks anywhere the graphic disappears
  MSYS_DefineRegion(&gSelectedConfirmOrderRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y,
                    LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH + 1, CURSOR_WWW,
                    MSYS_NO_CALLBACK, SelectConfirmOrderRegionCallBack);
  MSYS_AddRegion(&gSelectedConfirmOrderRegion);
  MSYS_DisableRegion(&gSelectedConfirmOrderRegion);

  // click on the shipping location to activate the drop down menu
  MSYS_DefineRegion(&gSelectedActivateCityDroDownRegion, BOBBYR_SHIPPING_LOC_AREA_L_X,
                    BOBBYR_SHIPPING_LOC_AREA_T_Y, BOBBYR_SHIPPING_LOC_AREA_R_X,
                    BOBBYR_SHIPPING_LOC_AREA_B_Y, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
                    SelectActivateCityDroDownRegionCallBack);
  MSYS_AddRegion(&gSelectedActivateCityDroDownRegion);

  // click anywhere on the screen to close the window( only when the drop down window is active)
  MSYS_DefineRegion(&gSelectedCloseDropDownRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y,
                    LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH - 1,
                    CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, SelectCloseDroDownRegionCallBack);
  MSYS_AddRegion(&gSelectedCloseDropDownRegion);
  MSYS_DisableRegion(&gSelectedCloseDropDownRegion);

  CreateBobbyRayOrderTitle();

  guiShippingCost = 0;

  gfRemoveItemsFromStock = FALSE;

  RenderBobbyRMailOrder();
  return (TRUE);
}

void ExitBobbyRMailOrder() {
  uint16_t i;

  // if we are to remove the items from stock
  if (gfRemoveItemsFromStock) {
    // Remove the items for Boby Rqys Inventory
    RemovePurchasedItemsFromBobbyRayInventory();
  }

  DestroyBobbyROrderTitle();

  DeleteVideoObjectFromIndex(guiBobbyROrderGrid);
  DeleteVideoObjectFromIndex(guiBobbyRLocationGraphic);
  DeleteVideoObjectFromIndex(guiDeliverySpeedGraphic);
  DeleteVideoObjectFromIndex(guiConfirmGraphic);
  DeleteVideoObjectFromIndex(guiTotalSaveArea);
  DeleteVideoObjectFromIndex(guiDropDownBorder);
  DeleteVideoObjectFromIndex(guiGoldArrowImages);
  DeleteVideoObjectFromIndex(guiPackageWeightImage);

  UnloadButtonImage(guiBobbyRClearOrderImage);
  RemoveButton(guiBobbyRClearOrder);

  UnloadButtonImage(guiBobbyRAcceptOrderImage);
  RemoveButton(guiBobbyRAcceptOrder);

  UnloadButtonImage(guiBobbyRBackImage);
  RemoveButton(guiBobbyRBack);

  UnloadButtonImage(giBobbyRGotoShipmentPageImage);
  RemoveButton(guiBobbyRGotoShipmentPage);

  RemoveButton(guiBobbyRHome);
  UnloadButtonImage(guiBobbyRHomeImage);

  DeleteBobbyRWoodBackground();

  for (i = 0; i < 3; i++) {
    MSYS_RemoveRegion(&gSelectedShippingSpeedRegion[i]);
  }

  MSYS_RemoveRegion(&gSelectedConfirmOrderRegion);
  MSYS_RemoveRegion(&gSelectedActivateCityDroDownRegion);
  MSYS_RemoveRegion(&gSelectedCloseDropDownRegion);

  // if the drop down box is active, destroy it
  gubDropDownAction = BR_DROP_DOWN_DESTROY;
  CreateDestroyBobbyRDropDown(BR_DROP_DOWN_DESTROY);
}

void HandleBobbyRMailOrder() {
  if (gfReDrawBobbyOrder) {
    //		RenderBobbyRMailOrder();
    fPausedReDrawScreenFlag = TRUE;
    gfReDrawBobbyOrder = FALSE;
  }

  if (gfDrawConfirmOrderGrpahic) {
    struct VObject *hPixHandle;

    // Bobbyray title
    GetVideoObject(&hPixHandle, guiConfirmGraphic);
    BltVideoObjectOutlineShadowFromIndex(vsFB, guiConfirmGraphic, 0, BOBBYR_CONFIRM_ORDER_X + 3,
                                         BOBBYR_CONFIRM_ORDER_Y + 3);

    BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_CONFIRM_ORDER_X, BOBBYR_CONFIRM_ORDER_Y);
    InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                     LAPTOP_SCREEN_WEB_LR_Y);

    gfDrawConfirmOrderGrpahic = FALSE;
  }

  if (gfDestroyConfirmGrphiArea) {
    gfDestroyConfirmGrphiArea = FALSE;
    gfReDrawBobbyOrder = TRUE;
    MSYS_DisableRegion(&gSelectedConfirmOrderRegion);
    gfCanAcceptOrder = TRUE;
  }

  if (gubDropDownAction != BR_DROP_DOWN_NO_ACTION) {
    CreateDestroyBobbyRDropDown(gubDropDownAction);

    if (gubDropDownAction == BR_DROP_DOWN_CREATE)
      gubDropDownAction = BR_DROP_DOWN_DISPLAY;
    else
      gubDropDownAction = BR_DROP_DOWN_NO_ACTION;
  }
}

void RenderBobbyRMailOrder() {
  uint16_t usPosY;
  struct VObject *hPixHandle;
  uint16_t usHeight;  // usWidth,
  wchar_t sTemp[128];

  DrawBobbyRWoodBackground();

  DrawBobbyROrderTitle();

  // Order Grid
  GetVideoObject(&hPixHandle, guiBobbyROrderGrid);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y);

  // Location graphic
  GetVideoObject(&hPixHandle, guiBobbyRLocationGraphic);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_LOCATION_BOX_X, BOBBYR_LOCATION_BOX_Y);

  // DeliverySpeedGraphic
  GetVideoObject(&hPixHandle, guiDeliverySpeedGraphic);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_DELIVERYSPEED_X, BOBBYR_DELIVERYSPEED_Y);

  // Package Weight
  GetVideoObject(&hPixHandle, guiPackageWeightImage);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_PACKAXGE_WEIGHT_X, BOBBYR_PACKAXGE_WEIGHT_Y);

  //
  // Display the STATIC text
  //

  // Output the title
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_ORDER_FORM], BOBBYR_ORDER_FORM_TITLE_X,
                   BOBBYR_ORDER_FORM_TITLE_Y, BOBBYR_ORDER_FORM_TITLE_WIDTH,
                   BOBBYR_ORDER_TITLE_TEXT_FONT, BOBBYR_ORDER_TITLE_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, CENTER_JUSTIFIED);

  /*
          //Output the qty
          DrawTextToScreen(BobbyROrderFormText[BOBBYR_QTY], BOBBYR_GRID_FIRST_COLUMN_X,
     BOBBYR_GRID_FIRST_COLUMN_Y-BOBBYR_GRID_TITLE_OFFSET, BOBBYR_GRID_FIRST_COLUMN_WIDTH,
     BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
     CENTER_JUSTIFIED);

          //Create a string for the weight ( %s ) ( where %s is the weight string, either kg or lbs
     ) swprintf( sTemp, BobbyROrderFormText[BOBBYR_WEIGHT], GetWeightUnitString( ) );

          //Output the Weight
          DisplayWrappedString(BOBBYR_GRID_SECOND_COLUMN_X, BOBBYR_GRID_SECOND_COLUMN_Y-30,
     BOBBYR_GRID_SECOND_COLUMN_WIDTH, 2, BOBBYR_ORDER_STATIC_TEXT_FONT,
     BOBBYR_ORDER_STATIC_TEXT_COLOR, sTemp, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

          //Output the name
          DrawTextToScreen(BobbyROrderFormText[BOBBYR_NAME], BOBBYR_GRID_THIRD_COLUMN_X,
     BOBBYR_GRID_THIRD_COLUMN_Y-BOBBYR_GRID_TITLE_OFFSET, BOBBYR_GRID_THIRD_COLUMN_WIDTH,
     BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
     CENTER_JUSTIFIED);

          //Output the unit price
          DisplayWrappedString(BOBBYR_GRID_FOURTH_COLUMN_X, BOBBYR_GRID_FOURTH_COLUMN_Y-30,
     BOBBYR_GRID_FOURTH_COLUMN_WIDTH, 2, BOBBYR_ORDER_STATIC_TEXT_FONT,
     BOBBYR_ORDER_STATIC_TEXT_COLOR, BobbyROrderFormText[BOBBYR_UNIT_PRICE], FONT_MCOLOR_BLACK,
     FALSE, CENTER_JUSTIFIED);

          //Output the total
          DrawTextToScreen(BobbyROrderFormText[BOBBYR_TOTAL], BOBBYR_GRID_FIFTH_COLUMN_X,
     BOBBYR_GRID_FIFTH_COLUMN_Y-BOBBYR_GRID_TITLE_OFFSET, BOBBYR_GRID_FIFTH_COLUMN_WIDTH,
     BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
     CENTER_JUSTIFIED);

          //Output the sub total, shipping and handling, and the grand total
          DrawTextToScreen(BobbyROrderFormText[BOBBYR_SUB_TOTAL], BOBBYR_SUBTOTAL_X,
     BOBBYR_SUBTOTAL_Y, BOBBYR_SUBTOTAL_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
     BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
          DrawTextToScreen(BobbyROrderFormText[BOBBYR_S_H], BOBBYR_SUBTOTAL_X,
     BOBBYR_SHIPPING_N_HANDLE_Y, BOBBYR_SUBTOTAL_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
     BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
          DrawTextToScreen(BobbyROrderFormText[BOBBYR_GRAND_TOTAL], BOBBYR_SUBTOTAL_X,
     BOBBYR_GRAND_TOTAL_Y, BOBBYR_SUBTOTAL_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
     BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
  */

  // Output the shipping location
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_SHIPPING_LOCATION], BOBBYR_SHIPPING_LOCATION_TEXT_X,
                   BOBBYR_SHIPPING_LOCATION_TEXT_Y, 0, BOBBYR_ORDER_STATIC_TEXT_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Output the shiupping speed
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_SHIPPING_SPEED], BOBBYR_SHIPPING_SPEED_X,
                   BOBBYR_SHIPPING_SPEED_Y, 0, BOBBYR_ORDER_STATIC_TEXT_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Create a string for the weight ( %s ) ( where %s is the weight string, either kg or lbs )
  swprintf(sTemp, ARR_SIZE(sTemp), BobbyROrderFormText[BOBBYR_COST], GetWeightUnitString());

  // Output the cost
  DrawTextToScreen(sTemp, BOBBYR_SHIPPING_COST_X, BOBBYR_SHIPPING_SPEED_Y, 0,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Output the overnight, business days, standard service
  usPosY = BOBBYR_OVERNIGHT_EXPRESS_Y;
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_OVERNIGHT_EXPRESS], BOBBYR_SHIPPING_SPEED_X, usPosY,
                   0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  usPosY += BOBBYR_GRID_ROW_OFFSET;
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_BUSINESS_DAYS], BOBBYR_SHIPPING_SPEED_X, usPosY, 0,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);
  usPosY += BOBBYR_GRID_ROW_OFFSET;
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_STANDARD_SERVICE], BOBBYR_SHIPPING_SPEED_X, usPosY, 0,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  //	DisplayPurchasedItems();
  DisplayPurchasedItems(TRUE, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y, BobbyRayPurchases, FALSE, -1);

  DrawShippingSpeedLights(gubSelectedLight);

  DisplayShippingLocationCity();

  // Display the 'used' text at the bottom of the screen
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_USED_TEXT], BOBBYR_USED_WARNING_X,
                   BOBBYR_USED_WARNING_Y + 1, 0, BOBBYR_DISCLAIMER_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED | TEXT_SHADOWED);

  // Display the minimum weight disclaimer at the bottom of the page
  usHeight = GetFontHeight(BOBBYR_DISCLAIMER_FONT) + 2;
  swprintf(sTemp, ARR_SIZE(sTemp), L"%s %2.1f %s.", BobbyROrderFormText[BOBBYR_MINIMUM_WEIGHT],
           GetWeightBasedOnMetricOption(MIN_SHIPPING_WEIGHT) / 10.0, GetWeightUnitString());
  DrawTextToScreen(sTemp, BOBBYR_USED_WARNING_X, (uint16_t)(BOBBYR_USED_WARNING_Y + usHeight + 1),
                   0, BOBBYR_DISCLAIMER_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED | TEXT_SHADOWED);

  // Calculate and display the total package weight
  DisplayPackageWeight();

  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void BtnBobbyRClearOrderCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    memset(&BobbyRayPurchases, 0, sizeof(BobbyRayPurchaseStruct) * MAX_PURCHASE_AMOUNT);
    gubSelectedLight = 0;
    gfReDrawBobbyOrder = TRUE;
    gbSelectedCity = -1;
    gubCityAtTopOfList = 0;

    // Get rid of the city drop dowm, if it is being displayed
    gubDropDownAction = BR_DROP_DOWN_DESTROY;

    // disable the accept order button
    DisableButton(guiBobbyRAcceptOrder);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnBobbyRAcceptOrderCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    if (guiSubTotal && gfCanAcceptOrder) {
      // if the player doesnt have enough money
      if (MoneyGetBalance() < giGrandTotal) {
        DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, BobbyROrderFormText[BOBBYR_CANT_AFFORD_PURCHASE],
                           LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
      } else {
        wchar_t zTemp[128];

        // if the city is Drassen, and the airport sector is player controlled
        if (gbSelectedCity == BR_DRASSEN &&
            !StrategicMap[SectorID8To16(SEC_B13)].fEnemyControlled) {
          // Quick hack to bypass the confirmation box
          ConfirmBobbyRPurchaseMessageBoxCallBack(MSG_BOX_RETURN_YES);
        } else {
          // else pop up a confirmation box
          swprintf(zTemp, ARR_SIZE(zTemp), BobbyROrderFormText[BOBBYR_CONFIRM_DEST],
                   pDeliveryLocationStrings[gbSelectedCity]);
          DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, zTemp, LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO,
                             ConfirmBobbyRPurchaseMessageBoxCallBack);
        }
      }
    }

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void DisplayPurchasedItems(BOOLEAN fCalledFromOrderPage, uint16_t usGridX, uint16_t usGridY,
                           BobbyRayPurchaseStruct *pBobbyRayPurchase, BOOLEAN fJustDisplayTitles,
                           int32_t iOrderNum) {
  uint16_t i, j;
  wchar_t sText[400];
  wchar_t sBack[400];
  wchar_t sTemp[20];
  uint16_t usPosY;
  uint32_t uiStartLoc = 0;
  uint32_t uiTotal;
  uint16_t usStringLength;
  uint16_t usPixLength;
  wchar_t OneChar[2];
  int32_t iSubTotal;

  // Output the qty
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_QTY],
                   (uint16_t)(usGridX + BOBBYR_GRID_FIRST_COLUMN_X),
                   (uint16_t)(usGridY + BOBBYR_GRID_FIRST_COLUMN_Y - BOBBYR_GRID_TITLE_OFFSET),
                   BOBBYR_GRID_FIRST_COLUMN_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Create a string for the weight ( %s ) ( where %s is the weight string, either kg or lbs )
  swprintf(sTemp, ARR_SIZE(sTemp), BobbyROrderFormText[BOBBYR_WEIGHT], GetWeightUnitString());

  // Output the Weight
  DisplayWrappedString((uint16_t)(usGridX + BOBBYR_GRID_SECOND_COLUMN_X),
                       (uint16_t)(usGridY + BOBBYR_GRID_SECOND_COLUMN_Y - 30),
                       BOBBYR_GRID_SECOND_COLUMN_WIDTH, 2, BOBBYR_ORDER_STATIC_TEXT_FONT,
                       BOBBYR_ORDER_STATIC_TEXT_COLOR, sTemp, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  // Output the name
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_NAME],
                   (uint16_t)(usGridX + BOBBYR_GRID_THIRD_COLUMN_X),
                   (uint16_t)(usGridY + BOBBYR_GRID_THIRD_COLUMN_Y - BOBBYR_GRID_TITLE_OFFSET),
                   BOBBYR_GRID_THIRD_COLUMN_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Output the unit price
  DisplayWrappedString((uint16_t)(usGridX + BOBBYR_GRID_FOURTH_COLUMN_X),
                       (uint16_t)(usGridY + BOBBYR_GRID_FOURTH_COLUMN_Y - 30),
                       BOBBYR_GRID_FOURTH_COLUMN_WIDTH, 2, BOBBYR_ORDER_STATIC_TEXT_FONT,
                       BOBBYR_ORDER_STATIC_TEXT_COLOR, BobbyROrderFormText[BOBBYR_UNIT_PRICE],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Output the total
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_TOTAL],
                   (uint16_t)(usGridX + BOBBYR_GRID_FIFTH_COLUMN_X),
                   (uint16_t)(usGridY + BOBBYR_GRID_FIFTH_COLUMN_Y - BOBBYR_GRID_TITLE_OFFSET),
                   BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Output the sub total, shipping and handling, and the grand total
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_SUB_TOTAL], (uint16_t)(usGridX + BOBBYR_SUBTOTAL_X),
                   (uint16_t)(usGridY + BOBBYR_SUBTOTAL_Y), BOBBYR_SUBTOTAL_WIDTH,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, RIGHT_JUSTIFIED);
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_S_H], (uint16_t)(usGridX + BOBBYR_SUBTOTAL_X),
                   (uint16_t)(usGridY + BOBBYR_SHIPPING_N_HANDLE_Y), BOBBYR_SUBTOTAL_WIDTH,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, RIGHT_JUSTIFIED);
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_GRAND_TOTAL], (uint16_t)(usGridX + BOBBYR_SUBTOTAL_X),
                   (uint16_t)(usGridY + BOBBYR_GRAND_TOTAL_Y), BOBBYR_SUBTOTAL_WIDTH,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, RIGHT_JUSTIFIED);

  if (fJustDisplayTitles) {
    return;
  }

  if (fCalledFromOrderPage) {
    guiSubTotal = 0;
    giGrandTotal = 0;
  } else {
    iSubTotal = 0;
  }

  if (pBobbyRayPurchase == NULL) {
    return;
  }

  // loop through the array of purchases to display only the items that are purchased
  usPosY = usGridY + BOBBYR_GRID_FIRST_COLUMN_Y + 4;
  for (i = 0; i < MAX_PURCHASE_AMOUNT; i++) {
    // if the item was purchased
    //		if( BobbyRayPurchases[ i ].ubNumberPurchased )
    if (pBobbyRayPurchase[i].ubNumberPurchased) {
      uiTotal = 0;

      // Display the qty, order#, item name, unit price and the total

      // qty
      swprintf(sTemp, ARR_SIZE(sTemp), L"%3d", pBobbyRayPurchase[i].ubNumberPurchased);
      DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_FIRST_COLUMN_X - 2), usPosY,
                       BOBBYR_GRID_FIRST_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT,
                       BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

      // weight
      swprintf(sTemp, ARR_SIZE(sTemp), L"%3.1f",
               GetWeightBasedOnMetricOption(Item[pBobbyRayPurchase[i].usItemIndex].ubWeight) /
                   (float)(10.0) * pBobbyRayPurchase[i].ubNumberPurchased);
      DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_SECOND_COLUMN_X - 2), usPosY,
                       BOBBYR_GRID_SECOND_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT,
                       BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

      // Display Items Name
      if (pBobbyRayPurchase[i].fUsed) {
        uiStartLoc =
            BOBBYR_ITEM_DESC_FILE_SIZE *
            LaptopSaveInfo.BobbyRayUsedInventory[pBobbyRayPurchase[i].usBobbyItemIndex].usItemIndex;
      } else {
        uiStartLoc =
            BOBBYR_ITEM_DESC_FILE_SIZE *
            LaptopSaveInfo.BobbyRayInventory[pBobbyRayPurchase[i].usBobbyItemIndex].usItemIndex;
      }

      if (pBobbyRayPurchase[i].fUsed) {
        LoadEncryptedDataFromFile(BOBBYRDESCFILE, sBack, uiStartLoc, BOBBYR_ITEM_DESC_NAME_SIZE);
        swprintf(sText, ARR_SIZE(sText), L"%s %s", L"*", sBack);
      } else
        LoadEncryptedDataFromFile(BOBBYRDESCFILE, sText, uiStartLoc, BOBBYR_ITEM_DESC_NAME_SIZE);

      // if the name is bigger then can fit into the slot, reduce the size
      if (StringPixLength(sText, BOBBYR_ORDER_DYNAMIC_TEXT_FONT) >
          BOBBYR_GRID_THIRD_COLUMN_WIDTH - 4) {
        usStringLength = (uint16_t)wcslen(sText);
        usPixLength = 0;
        OneChar[1] = L'\0';
        for (j = 0; (i < usStringLength) && (usPixLength < BOBBYR_GRID_THIRD_COLUMN_WIDTH - 16);
             j++) {
          sBack[j] = sText[j];
          OneChar[0] = sBack[j];
          usPixLength += StringPixLength(OneChar, BOBBYR_ORDER_DYNAMIC_TEXT_FONT);
        }
        sBack[j] = 0;
        swprintf(sText, ARR_SIZE(sText), L"%s...", sBack);
      }

      DrawTextToScreen(sText, (uint16_t)(usGridX + BOBBYR_GRID_THIRD_COLUMN_X + 2), usPosY,
                       BOBBYR_GRID_THIRD_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT,
                       BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      // unit price
      swprintf(sTemp, ARR_SIZE(sTemp), L"%d",
               CalcBobbyRayCost(pBobbyRayPurchase[i].usItemIndex,
                                pBobbyRayPurchase[i].usBobbyItemIndex, pBobbyRayPurchase[i].fUsed));
      InsertCommasForDollarFigure(sTemp);
      InsertDollarSignInToString(sTemp);

      DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_FOURTH_COLUMN_X - 2), usPosY,
                       BOBBYR_GRID_FOURTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT,
                       BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

      uiTotal +=
          CalcBobbyRayCost(pBobbyRayPurchase[i].usItemIndex, pBobbyRayPurchase[i].usBobbyItemIndex,
                           pBobbyRayPurchase[i].fUsed) *
          pBobbyRayPurchase[i].ubNumberPurchased;

      swprintf(sTemp, ARR_SIZE(sTemp), L"%d", uiTotal);
      InsertCommasForDollarFigure(sTemp);
      InsertDollarSignInToString(sTemp);

      DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2), usPosY,
                       BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT,
                       BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

      // add the current item total to the sub total
      if (fCalledFromOrderPage) {
        guiSubTotal += uiTotal;
      } else {
        iSubTotal += uiTotal;
      }

      usPosY += BOBBYR_GRID_ROW_OFFSET;
    }
  }

  DisplayShippingCosts(fCalledFromOrderPage, iSubTotal, usGridX, usGridY, iOrderNum);
}

void DisplayShippingCosts(BOOLEAN fCalledFromOrderPage, int32_t iSubTotal, uint16_t usGridX,
                          uint16_t usGridY, int32_t iOrderNum) {
  wchar_t sTemp[20];
  struct VObject *hPixHandle;
  int32_t iShippingCost = 0;
  //	int32_t iTotal;

  if (fCalledFromOrderPage) {
    iSubTotal = guiSubTotal;
    //		iTotal = giGrandTotal;

    if (gubSelectedLight == 0)
      guiShippingCost = CalcCostFromWeightOfPackage(0);
    else if (gubSelectedLight == 1)
      guiShippingCost = CalcCostFromWeightOfPackage(1);
    else if (gubSelectedLight == 2)
      guiShippingCost = CalcCostFromWeightOfPackage(2);

    iShippingCost = guiShippingCost;
  } else {
    uint16_t usStandardCost;

    switch (gpNewBobbyrShipments[iOrderNum].ubDeliveryMethod) {
      case 0:
        usStandardCost =
            BobbyROrderLocations[gpNewBobbyrShipments[iOrderNum].ubDeliveryLoc].usOverNightExpress;
        break;
      case 1:
        usStandardCost =
            BobbyROrderLocations[gpNewBobbyrShipments[iOrderNum].ubDeliveryLoc].us2DaysService;
        break;
      case 2:
        usStandardCost =
            BobbyROrderLocations[gpNewBobbyrShipments[iOrderNum].ubDeliveryLoc].usStandardService;
        break;

      default:
        usStandardCost = 0;
    }

    iShippingCost =
        (int32_t)((gpNewBobbyrShipments[iOrderNum].uiPackageWeight / (float)10) * usStandardCost +
                  .5);
  }

  // erase the old area
  // bli the total Saved area onto the grid
  if (fCalledFromOrderPage) {
    GetVideoObject(&hPixHandle, guiTotalSaveArea);
    BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_TOTAL_SAVED_AREA_X, BOBBYR_TOTAL_SAVED_AREA_Y);
  }

  // if there is a shipment, display the s&h charge
  if (iSubTotal) {
    // Display the subtotal
    swprintf(sTemp, ARR_SIZE(sTemp), L"%d", iSubTotal);
    InsertCommasForDollarFigure(sTemp);
    InsertDollarSignInToString(sTemp);

    DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2),
                     (uint16_t)(usGridY + BOBBYR_SUBTOTAL_Y), BOBBYR_GRID_FIFTH_COLUMN_WIDTH,
                     BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR,
                     FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

    // Display the shipping and handling charge
    swprintf(sTemp, ARR_SIZE(sTemp), L"%d", iShippingCost);
    InsertCommasForDollarFigure(sTemp);
    InsertDollarSignInToString(sTemp);

    DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2),
                     (uint16_t)(usGridY + BOBBYR_SHIPPING_N_HANDLE_Y),
                     BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT,
                     BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

    // Display the grand total
    giGrandTotal = iSubTotal + iShippingCost;
    swprintf(sTemp, ARR_SIZE(sTemp), L"%d", giGrandTotal);
    InsertCommasForDollarFigure(sTemp);
    InsertDollarSignInToString(sTemp);

    DrawTextToScreen(sTemp, (uint16_t)(usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2),
                     (uint16_t)(usGridY + BOBBYR_GRAND_TOTAL_Y), BOBBYR_GRID_FIFTH_COLUMN_WIDTH,
                     BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR,
                     FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
  }

  InvalidateRegion(333, 326, 374, 400);
}

void BtnBobbyRBackCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    guiCurrentLaptopMode = guiLastBobbyRayPage;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnBobbyRHomeCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void SelectShippingSpeedRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubSelectedLight = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
    DrawShippingSpeedLights(gubSelectedLight);
    DisplayShippingCosts(TRUE, 0, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y, -1);
  }
}

BOOLEAN DrawShippingSpeedLights(uint8_t ubSelected) {
  if (ubSelected == 0)
    ColorFillVSurfaceArea(vsFB, gShippingSpeedAreas[0], gShippingSpeedAreas[1],
                          gShippingSpeedAreas[0] + SHIPPING_SPEED_LIGHT_WIDTH,
                          gShippingSpeedAreas[1] + SHIPPING_SPEED_LIGHT_HEIGHT,
                          rgb32_to_rgb16(FROMRGB(0, 255, 0)));
  else
    ColorFillVSurfaceArea(vsFB, gShippingSpeedAreas[0], gShippingSpeedAreas[1],
                          gShippingSpeedAreas[0] + SHIPPING_SPEED_LIGHT_WIDTH,
                          gShippingSpeedAreas[1] + SHIPPING_SPEED_LIGHT_HEIGHT,
                          rgb32_to_rgb16(FROMRGB(0, 0, 0)));

  if (ubSelected == 1)
    ColorFillVSurfaceArea(vsFB, gShippingSpeedAreas[2], gShippingSpeedAreas[3],
                          gShippingSpeedAreas[2] + SHIPPING_SPEED_LIGHT_WIDTH,
                          gShippingSpeedAreas[3] + SHIPPING_SPEED_LIGHT_HEIGHT,
                          rgb32_to_rgb16(FROMRGB(0, 255, 0)));
  else
    ColorFillVSurfaceArea(vsFB, gShippingSpeedAreas[2], gShippingSpeedAreas[3],
                          gShippingSpeedAreas[2] + SHIPPING_SPEED_LIGHT_WIDTH,
                          gShippingSpeedAreas[3] + SHIPPING_SPEED_LIGHT_HEIGHT,
                          rgb32_to_rgb16(FROMRGB(0, 0, 0)));

  if (ubSelected == 2)
    ColorFillVSurfaceArea(vsFB, gShippingSpeedAreas[4], gShippingSpeedAreas[5],
                          gShippingSpeedAreas[4] + SHIPPING_SPEED_LIGHT_WIDTH,
                          gShippingSpeedAreas[5] + SHIPPING_SPEED_LIGHT_HEIGHT,
                          rgb32_to_rgb16(FROMRGB(0, 255, 0)));
  else
    ColorFillVSurfaceArea(vsFB, gShippingSpeedAreas[4], gShippingSpeedAreas[5],
                          gShippingSpeedAreas[4] + SHIPPING_SPEED_LIGHT_WIDTH,
                          gShippingSpeedAreas[5] + SHIPPING_SPEED_LIGHT_HEIGHT,
                          rgb32_to_rgb16(FROMRGB(0, 0, 0)));

  InvalidateRegion(585, 218, 594, 287);
  return (TRUE);
}

void SelectConfirmOrderRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // Remove the items for Boby Rqys Inventory
    RemovePurchasedItemsFromBobbyRayInventory();

    // delete the order
    memset(&BobbyRayPurchases, 0, sizeof(BobbyRayPurchaseStruct) * MAX_PURCHASE_AMOUNT);
    gubSelectedLight = 0;
    gfDestroyConfirmGrphiArea = TRUE;
    gubSelectedLight = 0;

    // Goto The homepage
    guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;

  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // Remove the items for Boby Rqys Inventory
    RemovePurchasedItemsFromBobbyRayInventory();

    // delete the order
    memset(&BobbyRayPurchases, 0, sizeof(BobbyRayPurchaseStruct) * MAX_PURCHASE_AMOUNT);
    gubSelectedLight = 0;
    gfDestroyConfirmGrphiArea = TRUE;
    gubSelectedLight = 0;
  }
}

BOOLEAN CreateDestroyBobbyRDropDown(uint8_t ubDropDownAction) {
  static uint16_t usHeight;
  static BOOLEAN fMouseRegionsCreated = FALSE;

  switch (ubDropDownAction) {
    case BR_DROP_DOWN_NO_ACTION: {
    } break;

    case BR_DROP_DOWN_CREATE: {
      uint8_t i;
      uint16_t usPosX, usPosY, usHeight;
      uint16_t usTemp;
      uint16_t usFontHeight = GetFontHeight(BOBBYR_DROPDOWN_FONT);

      if (fMouseRegionsCreated) {
        gubDropDownAction = BR_DROP_DOWN_DESTROY;

        break;
      }
      fMouseRegionsCreated = TRUE;

      usPosX = BOBBYR_CITY_START_LOCATION_X;
      usPosY = BOBBYR_CITY_START_LOCATION_Y;
      for (i = 0; i < BOBBYR_NUM_DISPLAYED_CITIES; i++) {
        MSYS_DefineRegion(&gSelectedDropDownRegion[i], usPosX, (uint16_t)(usPosY + 4),
                          (uint16_t)(usPosX + BOBBYR_DROP_DOWN_WIDTH - 6),
                          (uint16_t)(usPosY + usFontHeight + 7), MSYS_PRIORITY_HIGH, CURSOR_WWW,
                          SelectDropDownMovementCallBack, SelectDropDownRegionCallBack);
        MSYS_AddRegion(&gSelectedDropDownRegion[i]);
        MSYS_SetRegionUserData(&gSelectedDropDownRegion[i], 0, i);

        usPosY += usFontHeight + 2;
      }
      usTemp = BOBBYR_CITY_START_LOCATION_Y;
      usHeight = usPosY - usTemp + 10;

      // create the scroll bars regions
      // up arrow
      usPosX = BOBBYR_SCROLL_UP_ARROW_X;
      usPosY = BOBBYR_SCROLL_UP_ARROW_Y;
      for (i = 0; i < 2; i++) {
        MSYS_DefineRegion(&gSelectedUpDownArrowOnScrollAreaRegion[i], usPosX, usPosY,
                          (uint16_t)(usPosX + BOBBYR_SCROLL_ARROW_WIDTH),
                          (uint16_t)(usPosY + BOBBYR_SCROLL_ARROW_HEIGHT), MSYS_PRIORITY_HIGH,
                          CURSOR_WWW, MSYS_NO_CALLBACK,
                          SelectUpDownArrowOnScrollAreaRegionCallBack);
        MSYS_AddRegion(&gSelectedUpDownArrowOnScrollAreaRegion[i]);
        MSYS_SetRegionUserData(&gSelectedUpDownArrowOnScrollAreaRegion[i], 0, i);
        usPosX = BOBBYR_SCROLL_DOWN_ARROW_X;
        usPosY = BOBBYR_SCROLL_DOWN_ARROW_Y;
      }

      // the scroll area itself
      usPosX = BOBBYR_SCROLL_AREA_X;
      usPosY = BOBBYR_SCROLL_UP_ARROW_Y + BOBBYR_SCROLL_ARROW_HEIGHT;
      usHeight = BOBBYR_SCROLL_AREA_HEIGHT_MINUS_ARROWS / BOBBYR_ORDER_NUM_SHIPPING_CITIES;
      for (i = 0; i < BOBBYR_ORDER_NUM_SHIPPING_CITIES - 1; i++) {
        MSYS_DefineRegion(&gSelectedScrollAreaDropDownRegion[i], usPosX, usPosY,
                          (uint16_t)(usPosX + BOBBYR_SCROLL_ARROW_WIDTH),
                          (uint16_t)(usPosY + usHeight), MSYS_PRIORITY_HIGH + 1,
                          CURSOR_LAPTOP_SCREEN, SelectScrollAreaDropDownMovementCallBack,
                          SelectScrollAreaDropDownRegionCallBack);
        MSYS_AddRegion(&gSelectedScrollAreaDropDownRegion[i]);
        MSYS_SetRegionUserData(&gSelectedScrollAreaDropDownRegion[i], 0, i);
        usPosY += usHeight;
      }
      // put the last one down to cover the remaining area
      MSYS_DefineRegion(&gSelectedScrollAreaDropDownRegion[i], usPosX, usPosY,
                        (uint16_t)(usPosX + BOBBYR_SCROLL_ARROW_WIDTH), BOBBYR_SCROLL_DOWN_ARROW_Y,
                        MSYS_PRIORITY_HIGH + 1, CURSOR_LAPTOP_SCREEN,
                        SelectScrollAreaDropDownMovementCallBack,
                        SelectScrollAreaDropDownRegionCallBack);
      MSYS_AddRegion(&gSelectedScrollAreaDropDownRegion[i]);
      MSYS_SetRegionUserData(&gSelectedScrollAreaDropDownRegion[i], 0, i);

      MSYS_EnableRegion(&gSelectedCloseDropDownRegion);

      // disable the clear order and accept order buttons, (their rendering interferes with the drop
      // down graphics)
      DisableButton(guiBobbyRClearOrder);

      // FERAL
      //			if( IsAnythingPurchasedFromBobbyRayPage() )
      //			{
      //				SpecifyDisabledButtonStyle( guiBobbyRAcceptOrder,
      // DISABLED_STYLE_NONE ); 				DisableButton(guiBobbyRAcceptOrder);
      //			}
    } break;

    case BR_DROP_DOWN_DESTROY: {
      uint8_t i;

      if (!fMouseRegionsCreated) break;

      for (i = 0; i < BOBBYR_NUM_DISPLAYED_CITIES; i++)
        MSYS_RemoveRegion(&gSelectedDropDownRegion[i]);

      // destroy the scroll bars arrow regions
      for (i = 0; i < 2; i++) MSYS_RemoveRegion(&gSelectedUpDownArrowOnScrollAreaRegion[i]);

      // destroy the scroll bars regions
      for (i = 0; i < BOBBYR_ORDER_NUM_SHIPPING_CITIES; i++)
        MSYS_RemoveRegion(&gSelectedScrollAreaDropDownRegion[i]);

      // display the name on the title bar
      ColorFillVSurfaceArea(vsFB, BOBBYR_SHIPPING_LOC_AREA_L_X, BOBBYR_SHIPPING_LOC_AREA_T_Y,
                            BOBBYR_SHIPPING_LOC_AREA_L_X + 175,
                            BOBBYR_SHIPPING_LOC_AREA_T_Y + BOBBYR_DROP_DOWN_HEIGHT,
                            rgb32_to_rgb16(FROMRGB(0, 0, 0)));

      if (gbSelectedCity == -1)
        DrawTextToScreen(BobbyROrderFormText[BOBBYR_SELECT_DEST],
                         BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET,
                         BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT,
                         BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, FALSE,
                         LEFT_JUSTIFIED);
      else
        DrawTextToScreen(pDeliveryLocationStrings[gbSelectedCity],
                         BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET,
                         BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT,
                         BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, FALSE,
                         LEFT_JUSTIFIED);

      // disable the r\close regiuon
      MSYS_DisableRegion(&gSelectedCloseDropDownRegion);

      // enable the clear order and accept order buttons, (because their rendering interferes with
      // the drop down graphics)
      EnableButton(guiBobbyRClearOrder);
      // FERAL
      //			if( IsAnythingPurchasedFromBobbyRayPage() )
      //			{
      //				SpecifyDisabledButtonStyle( guiBobbyRAcceptOrder,
      // DISABLED_STYLE_SHADED ); 			  EnableButton(guiBobbyRAcceptOrder);
      //			}

      gfReDrawBobbyOrder = TRUE;
      fMouseRegionsCreated = FALSE;
      gubDropDownAction = BR_DROP_DOWN_NO_ACTION;
    } break;

    case BR_DROP_DOWN_DISPLAY: {
      uint8_t i;
      uint16_t usPosY, usPosX;
      struct VObject *hImageHandle;
      struct VObject *hArrowHandle;

      // Display the background for the drop down window
      ColorFillVSurfaceArea(vsFB, BOBBYR_CITY_START_LOCATION_X, BOBBYR_CITY_START_LOCATION_Y,
                            BOBBYR_CITY_START_LOCATION_X + BOBBYR_DROP_DOWN_WIDTH,
                            BOBBYR_CITY_START_LOCATION_Y + BOBBYR_SCROLL_AREA_HEIGHT,
                            rgb32_to_rgb16(FROMRGB(0, 0, 0)));

      //
      // Place the border around the background
      //
      usHeight = BOBBYR_SCROLL_AREA_HEIGHT;

      GetVideoObject(&hImageHandle, guiDropDownBorder);

      usPosX = usPosY = 0;
      // blit top & bottom row of images
      for (i = 10; i < BOBBYR_DROP_DOWN_WIDTH - 10; i += 10) {
        // TOP ROW
        BltVideoObject(vsFB, hImageHandle, 1, i + BOBBYR_CITY_START_LOCATION_X,
                       usPosY + BOBBYR_CITY_START_LOCATION_Y);
        // BOTTOM ROW
        BltVideoObject(vsFB, hImageHandle, 6, i + BOBBYR_CITY_START_LOCATION_X,
                       usHeight - 10 + 6 + BOBBYR_CITY_START_LOCATION_Y);
      }

      // blit the left and right row of images
      usPosX = 0;
      for (i = 10; i < usHeight - 10; i += 10) {
        BltVideoObject(vsFB, hImageHandle, 3, usPosX + BOBBYR_CITY_START_LOCATION_X,
                       i + BOBBYR_CITY_START_LOCATION_Y);
        BltVideoObject(vsFB, hImageHandle, 4,
                       usPosX + BOBBYR_DROP_DOWN_WIDTH - 4 + BOBBYR_CITY_START_LOCATION_X,
                       i + BOBBYR_CITY_START_LOCATION_Y);
      }

      // blt the corner images for the row
      // top left
      BltVideoObject(vsFB, hImageHandle, 0, 0 + BOBBYR_CITY_START_LOCATION_X,
                     usPosY + BOBBYR_CITY_START_LOCATION_Y);
      // top right
      BltVideoObject(vsFB, hImageHandle, 2,
                     BOBBYR_DROP_DOWN_WIDTH - 10 + BOBBYR_CITY_START_LOCATION_X,
                     usPosY + BOBBYR_CITY_START_LOCATION_Y);
      // bottom left
      BltVideoObject(vsFB, hImageHandle, 5, 0 + BOBBYR_CITY_START_LOCATION_X,
                     usHeight - 10 + BOBBYR_CITY_START_LOCATION_Y);
      // bottom right
      BltVideoObject(vsFB, hImageHandle, 7,
                     BOBBYR_DROP_DOWN_WIDTH - 10 + BOBBYR_CITY_START_LOCATION_X,
                     usHeight - 10 + BOBBYR_CITY_START_LOCATION_Y);

      DrawSelectedCity(gbSelectedCity);

      // display the scroll bars regions
      ColorFillVSurfaceArea(vsFB, BOBBYR_SCROLL_AREA_X, BOBBYR_SCROLL_AREA_Y,
                            BOBBYR_SCROLL_AREA_X + BOBBYR_SCROLL_AREA_WIDTH,
                            BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT,
                            rgb32_to_rgb16(FROMRGB(0, 0, 0)));

      // blt right bar of scroll area
      usPosX = 0;
      for (i = 10; i < BOBBYR_SCROLL_AREA_HEIGHT - 10; i += 10) {
        BltVideoObject(vsFB, hImageHandle, 3, BOBBYR_SCROLL_AREA_X + BOBBYR_SCROLL_AREA_WIDTH - 4,
                       i + BOBBYR_CITY_START_LOCATION_Y);
      }

      // blit top row of images
      for (i = 0; i < BOBBYR_SCROLL_AREA_WIDTH; i += 10) {
        // TOP ROW
        BltVideoObject(vsFB, hImageHandle, 1, i + BOBBYR_SCROLL_AREA_X - 10, BOBBYR_SCROLL_AREA_Y);
        // BOTTOM ROW
        BltVideoObject(vsFB, hImageHandle, 6, i + BOBBYR_SCROLL_AREA_X - 10,
                       BOBBYR_SCROLL_AREA_Y - 10 + 6 + BOBBYR_SCROLL_AREA_HEIGHT);
      }

      // top right
      BltVideoObject(vsFB, hImageHandle, 2, BOBBYR_SCROLL_AREA_X + BOBBYR_SCROLL_AREA_WIDTH - 10,
                     BOBBYR_SCROLL_AREA_Y);
      // bottom right
      BltVideoObject(vsFB, hImageHandle, 7, BOBBYR_SCROLL_AREA_X + BOBBYR_SCROLL_AREA_WIDTH - 10,
                     BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT - 10);

      // fix
      BltVideoObject(vsFB, hImageHandle, 4,
                     BOBBYR_DROP_DOWN_WIDTH - 4 + BOBBYR_CITY_START_LOCATION_X,
                     BOBBYR_CITY_START_LOCATION_Y + 2);

      // get and display the up and down arrows
      GetVideoObject(&hArrowHandle, guiGoldArrowImages);
      // top arrow
      BltVideoObject(vsFB, hArrowHandle, 1, BOBBYR_SCROLL_UP_ARROW_X, BOBBYR_SCROLL_UP_ARROW_Y);

      // top arrow
      BltVideoObject(vsFB, hArrowHandle, 0, BOBBYR_SCROLL_DOWN_ARROW_X, BOBBYR_SCROLL_DOWN_ARROW_Y);

      // display the scroll rectangle
      DrawGoldRectangle(gbSelectedCity);

      InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                       LAPTOP_SCREEN_WEB_LR_Y);
    } break;
  }

  return (TRUE);
}

void SelectDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint8_t ubSelected = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
    gbSelectedCity = ubSelected + gubCityAtTopOfList;

    DrawSelectedCity(gbSelectedCity);

    gubDropDownAction = BR_DROP_DOWN_DESTROY;
  }
}

void SelectActivateCityDroDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubDropDownAction = BR_DROP_DOWN_CREATE;
  }
}

void SelectDropDownMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    pRegion->uiFlags |= BUTTON_CLICKED_ON;

    gbSelectedCity = (uint8_t)MSYS_GetRegionUserData(pRegion, 0) + gubCityAtTopOfList;

    gubDropDownAction = BR_DROP_DOWN_DISPLAY;

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

void DrawSelectedCity(uint8_t ubCityNumber) {
  uint16_t usPosY;
  uint16_t usFontHeight = GetFontHeight(BOBBYR_DROPDOWN_FONT);
  uint8_t i;

  // DEBUG: make sure it wont go over array bounds
  if (gubCityAtTopOfList + BOBBYR_NUM_DISPLAYED_CITIES > BOBBYR_ORDER_NUM_SHIPPING_CITIES)
    gubCityAtTopOfList = BOBBYR_ORDER_NUM_SHIPPING_CITIES - BOBBYR_NUM_DISPLAYED_CITIES - 1;

  // Display the list of cities
  usPosY = BOBBYR_CITY_START_LOCATION_Y + 5;
  for (i = gubCityAtTopOfList; i < gubCityAtTopOfList + BOBBYR_NUM_DISPLAYED_CITIES; i++) {
    DrawTextToScreen(pDeliveryLocationStrings[i],
                     BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, usPosY, 0,
                     BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE,
                     LEFT_JUSTIFIED);
    usPosY += usFontHeight + 2;
  }

  if (ubCityNumber != 255)
    usPosY =
        (usFontHeight + 2) * (ubCityNumber - gubCityAtTopOfList) + BOBBYR_CITY_START_LOCATION_Y;
  else
    usPosY = (usFontHeight + 2) * (gubCityAtTopOfList) + BOBBYR_CITY_START_LOCATION_Y;

  // display the name in the list
  ColorFillVSurfaceArea(vsFB, BOBBYR_CITY_START_LOCATION_X + 4, usPosY + 4,
                        BOBBYR_CITY_START_LOCATION_X + BOBBYR_DROP_DOWN_WIDTH - 4,
                        usPosY + usFontHeight + 6, rgb32_to_rgb16(FROMRGB(200, 169, 87)));

  SetFontShadow(NO_SHADOW);
  if (ubCityNumber == 255)
    DrawTextToScreen(pDeliveryLocationStrings[0],
                     BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, (uint16_t)(usPosY + 5),
                     0, BOBBYR_DROPDOWN_FONT, BOBBYR_FONT_BLACK, FONT_MCOLOR_BLACK, FALSE,
                     LEFT_JUSTIFIED);
  else
    DrawTextToScreen(pDeliveryLocationStrings[ubCityNumber],
                     BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, (uint16_t)(usPosY + 5),
                     0, BOBBYR_DROPDOWN_FONT, BOBBYR_FONT_BLACK, FONT_MCOLOR_BLACK, FALSE,
                     LEFT_JUSTIFIED);

  SetFontShadow(DEFAULT_SHADOW);

  DisplayShippingLocationCity();

  if (guiBobbyRAcceptOrder != -1) {
    // if there is anything to buy, dont disable the accept button
    if (IsAnythingPurchasedFromBobbyRayPage() && gbSelectedCity != -1)
      EnableButton(guiBobbyRAcceptOrder);
  }
}

void DisplayShippingLocationCity() {
  wchar_t sTemp[40];
  uint16_t usPosY;

  // display the name on the title bar
  ColorFillVSurfaceArea(vsFB, BOBBYR_SHIPPING_LOC_AREA_L_X, BOBBYR_SHIPPING_LOC_AREA_T_Y,
                        BOBBYR_SHIPPING_LOC_AREA_L_X + 175,
                        BOBBYR_SHIPPING_LOC_AREA_T_Y + BOBBYR_DROP_DOWN_HEIGHT,
                        rgb32_to_rgb16(FROMRGB(0, 0, 0)));

  // if there is no city selected
  if (gbSelectedCity == -1)
    DrawTextToScreen(BobbyROrderFormText[BOBBYR_SELECT_DEST],
                     BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET,
                     BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT,
                     BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  else
    DrawTextToScreen(pDeliveryLocationStrings[gbSelectedCity],
                     BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET,
                     BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT,
                     BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  DisplayShippingCosts(TRUE, 0, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y, -1);

  if (gubDropDownAction == BR_DROP_DOWN_DISPLAY) return;

  // Display the shipping cost
  usPosY = BOBBYR_OVERNIGHT_EXPRESS_Y;

  wcscpy(sTemp, L"$0");

  if (gbSelectedCity != -1) {
    swprintf(sTemp, ARR_SIZE(sTemp), L"%d",
             (int32_t)(BobbyROrderLocations[gbSelectedCity].usOverNightExpress /
                       GetWeightBasedOnMetricOption(1)));
    InsertCommasForDollarFigure(sTemp);
    InsertDollarSignInToString(sTemp);
  }

  DrawTextToScreen(sTemp, BOBBYR_SHIPPING_SPEED_NUMBER_X, usPosY,
                   BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH, BOBBYR_DROPDOWN_FONT,
                   BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
  usPosY += BOBBYR_GRID_ROW_OFFSET;

  if (gbSelectedCity != -1) {
    swprintf(sTemp, ARR_SIZE(sTemp), L"%d",
             (int32_t)(BobbyROrderLocations[gbSelectedCity].us2DaysService /
                       GetWeightBasedOnMetricOption(1)));
    InsertCommasForDollarFigure(sTemp);
    InsertDollarSignInToString(sTemp);
  }

  DrawTextToScreen(sTemp, BOBBYR_SHIPPING_SPEED_NUMBER_X, usPosY,
                   BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH, BOBBYR_DROPDOWN_FONT,
                   BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
  usPosY += BOBBYR_GRID_ROW_OFFSET;

  if (gbSelectedCity != -1) {
    swprintf(sTemp, ARR_SIZE(sTemp), L"%d",
             (int32_t)(BobbyROrderLocations[gbSelectedCity].usStandardService /
                       GetWeightBasedOnMetricOption(1)));
    InsertCommasForDollarFigure(sTemp);
    InsertDollarSignInToString(sTemp);
  }

  DrawTextToScreen(sTemp, BOBBYR_SHIPPING_SPEED_NUMBER_X, usPosY,
                   BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH, BOBBYR_DROPDOWN_FONT,
                   BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
}

void SelectCloseDroDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gubDropDownAction = BR_DROP_DOWN_DESTROY;
  }
}

void RemovePurchasedItemsFromBobbyRayInventory() {
  int16_t i;

  for (i = 0; i < MAX_PURCHASE_AMOUNT; i++) {
    // if the item was purchased
    if (BobbyRayPurchases[i].ubNumberPurchased) {
      // if the item is used
      if (BobbyRayPurchases[i].fUsed) {
        // removee it from Bobby Rays Inventory
        if ((LaptopSaveInfo.BobbyRayUsedInventory[BobbyRayPurchases[i].usBobbyItemIndex]
                 .ubQtyOnHand -
             BobbyRayPurchases[i].ubNumberPurchased) > 0)
          LaptopSaveInfo.BobbyRayUsedInventory[BobbyRayPurchases[i].usBobbyItemIndex].ubQtyOnHand -=
              BobbyRayPurchases[i].ubNumberPurchased;
        else
          LaptopSaveInfo.BobbyRayUsedInventory[BobbyRayPurchases[i].usBobbyItemIndex].ubQtyOnHand =
              0;
      }

      // else the purchase is new
      else {
        // removee it from Bobby Rays Inventory
        if ((LaptopSaveInfo.BobbyRayInventory[BobbyRayPurchases[i].usBobbyItemIndex].ubQtyOnHand -
             BobbyRayPurchases[i].ubNumberPurchased) > 0)
          LaptopSaveInfo.BobbyRayInventory[BobbyRayPurchases[i].usBobbyItemIndex].ubQtyOnHand -=
              BobbyRayPurchases[i].ubNumberPurchased;
        else
          LaptopSaveInfo.BobbyRayInventory[BobbyRayPurchases[i].usBobbyItemIndex].ubQtyOnHand = 0;
      }
    }
  }
  gfRemoveItemsFromStock = FALSE;
}

BOOLEAN IsAnythingPurchasedFromBobbyRayPage() {
  uint16_t i;
  BOOLEAN fReturnType = FALSE;

  for (i = 0; i < MAX_PURCHASE_AMOUNT; i++) {
    // if the item was purchased
    if (BobbyRayPurchases[i].ubNumberPurchased) {
      fReturnType = TRUE;
    }
  }
  return (fReturnType);
}

void SelectTitleLinkRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
  }
}

void SelectScrollAreaDropDownRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint8_t ubCityNum = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    if (ubCityNum < gbSelectedCity) {
      gbSelectedCity--;
      if (gbSelectedCity < gubCityAtTopOfList) gubCityAtTopOfList--;
    }

    if (ubCityNum > gbSelectedCity) {
      gbSelectedCity++;
      if ((gbSelectedCity - gubCityAtTopOfList) >= BOBBYR_NUM_DISPLAYED_CITIES)
        gubCityAtTopOfList++;
    }

    gubDropDownAction = BR_DROP_DOWN_DISPLAY;
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    uint8_t ubCityNum = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    pRegion->uiFlags |= BUTTON_CLICKED_ON;

    if (ubCityNum < gbSelectedCity) {
      gbSelectedCity--;
      if (gbSelectedCity < gubCityAtTopOfList) gubCityAtTopOfList--;
    }

    if (ubCityNum > gbSelectedCity) {
      gbSelectedCity++;
      if ((gbSelectedCity - gubCityAtTopOfList) >= BOBBYR_NUM_DISPLAYED_CITIES)
        gubCityAtTopOfList++;
    }

    gubDropDownAction = BR_DROP_DOWN_DISPLAY;

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

void SelectScrollAreaDropDownMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (gfLeftButtonState) {
      uint8_t ubCityNum = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

      pRegion->uiFlags |= BUTTON_CLICKED_ON;

      if (ubCityNum < gbSelectedCity) {
        gbSelectedCity = ubCityNum;
        if (gbSelectedCity < gubCityAtTopOfList) gubCityAtTopOfList = gbSelectedCity;
      }

      if (ubCityNum > gbSelectedCity) {
        gbSelectedCity = ubCityNum;
        if ((gbSelectedCity - gubCityAtTopOfList) >= BOBBYR_NUM_DISPLAYED_CITIES)
          gubCityAtTopOfList = gbSelectedCity - BOBBYR_NUM_DISPLAYED_CITIES + 1;
      }

      gubDropDownAction = BR_DROP_DOWN_DISPLAY;

      InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY,
                       pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
    }
  }
}

void SelectUpDownArrowOnScrollAreaRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP ||
             iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    uint8_t ubUpArrow = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    if (ubUpArrow) {
      if (gbSelectedCity < BOBBYR_ORDER_NUM_SHIPPING_CITIES - 1) {
        gbSelectedCity++;
      }

      if ((gbSelectedCity - gubCityAtTopOfList) >= BOBBYR_NUM_DISPLAYED_CITIES) {
        gubCityAtTopOfList++;
      }
    } else {
      if (gbSelectedCity != -1) {
        if (gbSelectedCity > 0) gbSelectedCity--;

        if (gbSelectedCity < gubCityAtTopOfList) gubCityAtTopOfList--;
      }
    }

    gubDropDownAction = BR_DROP_DOWN_DISPLAY;
  }
}

void DrawGoldRectangle(int8_t bCityNum) {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usWidth, usTempHeight, usTempPosY, usHeight;
  uint16_t usPosX, usPosY;

  uint16_t temp;

  if (bCityNum == -1) bCityNum = 0;

  usTempPosY = BOBBYR_SCROLL_UP_ARROW_Y;
  usTempPosY += BOBBYR_SCROLL_ARROW_HEIGHT;
  usPosX = BOBBYR_SCROLL_AREA_X;
  usWidth = BOBBYR_SCROLL_AREA_WIDTH - 5;
  usTempHeight = (BOBBYR_SCROLL_AREA_HEIGHT - 2 * BOBBYR_SCROLL_ARROW_HEIGHT) - 8;

  usHeight = usTempHeight / (BOBBYR_ORDER_NUM_SHIPPING_CITIES + 1);

  usPosY = usTempPosY + (uint16_t)(((BOBBYR_SCROLL_AREA_HEIGHT - 2 * BOBBYR_SCROLL_ARROW_HEIGHT) /
                                    (float)(BOBBYR_ORDER_NUM_SHIPPING_CITIES + 1)) *
                                   bCityNum);

  temp =
      BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT - BOBBYR_SCROLL_ARROW_HEIGHT - usHeight - 1;

  if (usPosY >= temp)
    usPosY = BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT - BOBBYR_SCROLL_ARROW_HEIGHT -
             usHeight - 5;

  ColorFillVSurfaceArea(vsFB, BOBBYR_SCROLL_AREA_X, usPosY, BOBBYR_SCROLL_AREA_X + usWidth,
                        usPosY + usHeight, rgb32_to_rgb16(FROMRGB(186, 165, 68)));

  // display the line
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw the gold highlite line on the top and left
  LineDraw(FALSE, usPosX, usPosY, usPosX + usWidth, usPosY, rgb32_to_rgb16(FROMRGB(235, 222, 171)),
           pDestBuf);
  LineDraw(FALSE, usPosX, usPosY, usPosX, usPosY + usHeight, rgb32_to_rgb16(FROMRGB(235, 222, 171)),
           pDestBuf);

  // draw the shadow line on the bottom and right
  LineDraw(FALSE, usPosX, usPosY + usHeight, usPosX + usWidth, usPosY + usHeight,
           rgb32_to_rgb16(FROMRGB(65, 49, 6)), pDestBuf);
  LineDraw(FALSE, usPosX + usWidth, usPosY, usPosX + usWidth, usPosY + usHeight,
           rgb32_to_rgb16(FROMRGB(65, 49, 6)), pDestBuf);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
}

uint32_t CalcCostFromWeightOfPackage(uint8_t ubTypeOfService) {
  uint32_t uiTotalWeight = 0;
  uint16_t usStandardCost = 0;
  uint32_t uiTotalCost = 0;

  if (gbSelectedCity == -1) {
    // shipping rates unknown until destination selected
    return (0);
  }

  // Get the package's weight
  uiTotalWeight = CalcPackageTotalWeight();

  /*	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
          {
                  //if the item was purchased
                  if( BobbyRayPurchases[ i ].ubNumberPurchased )
                  {
                          //add the current weight to the total
                          uiTotalWeight += Item[ BobbyRayPurchases[ i ].usItemIndex ].ubWeight *
     BobbyRayPurchases[ i ].ubNumberPurchased;
                  }
          }
  */
  Assert(ubTypeOfService < 3);

  switch (ubTypeOfService) {
    case 0:
      usStandardCost = BobbyROrderLocations[gbSelectedCity].usOverNightExpress;
      break;
    case 1:
      usStandardCost = BobbyROrderLocations[gbSelectedCity].us2DaysService;
      break;
    case 2:
      usStandardCost = BobbyROrderLocations[gbSelectedCity].usStandardService;
      break;

    default:
      usStandardCost = 0;
  }

  // Get the actual weight ( either in lbs or metric )
  ///	usStandardCost = (uint16_t) GetWeightBasedOnMetricOption( usStandardCost );

  // if the total weight is below a set minimum amount ( 2 kg )
  if (uiTotalWeight < MIN_SHIPPING_WEIGHT) {
    // bring up the base cost
    uiTotalWeight = MIN_SHIPPING_WEIGHT;
  }

  uiTotalCost = (uint32_t)((uiTotalWeight / (float)10) * usStandardCost + .5);

  return (uiTotalCost);
}

void BobbyRayMailOrderEndGameShutDown() {
  ShutDownBobbyRNewMailOrders();
  /*
          if( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray )
          {
                  MemFree( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray );
                  LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
          }
  */
}

void ShutDownBobbyRNewMailOrders() {
  if (gpNewBobbyrShipments != NULL) {
    MemFree(gpNewBobbyrShipments);
    gpNewBobbyrShipments = NULL;
  }
  giNumberOfNewBobbyRShipment = 0;
}

int8_t CalculateOrderDelay(uint8_t ubSelectedService) {
  int8_t bDaysAhead;

  // get the length of time to receive the shipment
  if (ubSelectedService == 0) {
    bDaysAhead = OVERNIGHT_EXPRESS;
  } else if (ubSelectedService == 1) {
    bDaysAhead = TWO_BUSINESS_DAYS;
  } else if (ubSelectedService == 2) {
    bDaysAhead = STANDARD_SERVICE;
  } else {
    bDaysAhead = 0;
  }

  if (gMercProfiles[SAL].bLife == 0) {
    // Sal is dead, so Pablo is dead, so the airport is badly run
    // CJC comment: this seems really extreme!! maybe delay by 1 day randomly but that's it!
    bDaysAhead += (uint8_t)Random(5) + 1;
  }

  return (bDaysAhead);
}

void PurchaseBobbyOrder() {
  // if the shipment is going to Drassen, add the inventory
  if (gbSelectedCity == BR_DRASSEN || gbSelectedCity == BR_MEDUNA) {
    //					BobbyRayOrderStruct *pBobbyRayPurchase;
    //					uint32_t	uiResetTimeSec;
    //		uint8_t	i, ubCount;
    //		uint8_t	cnt;
    //		int8_t		bDaysAhead;

    /*
                    //if we need to add more array elements for the Order Array
                    if( LaptopSaveInfo.usNumberOfBobbyRayOrderItems <=
       LaptopSaveInfo.usNumberOfBobbyRayOrderUsed )
                    {
                            LaptopSaveInfo.usNumberOfBobbyRayOrderItems++;
                            LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = MemRealloc(
       LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray, sizeof( BobbyRayOrderStruct ) *
       LaptopSaveInfo.usNumberOfBobbyRayOrderItems ); if(
       LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray == NULL ) return;

                            memset( &LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[
       LaptopSaveInfo.usNumberOfBobbyRayOrderItems - 1 ], 0, sizeof( BobbyRayOrderStruct ) );
                    }

                    for( cnt =0; cnt< LaptopSaveInfo.usNumberOfBobbyRayOrderItems; cnt++ )
                    {
                            //get an empty element in the array
                            if( !LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].fActive )
                                    break;
                    }
    */

    // gets reset when the confirm order graphic disappears
    gfCanAcceptOrder = FALSE;

    //					pBobbyRayPurchase = MemAlloc( sizeof( BobbyRayOrderStruct )
    //); 					memset(pBobbyRayPurchase, 0, sizeof(
    // BobbyRayOrderStruct
    //)
    //);

    /*
                    ubCount = 0;
                    for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
                    {
                            //if the item was purchased
                            if( BobbyRayPurchases[ i ].ubNumberPurchased )
                            {
                                    //copy the purchases into the struct that will be added to the
       queue memcpy(&LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].BobbyRayPurchase[ ubCount ]
       , &BobbyRayPurchases[i],  sizeof(BobbyRayPurchaseStruct)); ubCount ++;
                            }
                    }

                    LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].ubNumberPurchases = ubCount;
                    LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].fActive = TRUE;
                    LaptopSaveInfo.usNumberOfBobbyRayOrderUsed++;
    */

    // add the delivery
    AddNewBobbyRShipment(BobbyRayPurchases, gbSelectedCity, gubSelectedLight, TRUE,
                         CalcPackageTotalWeight());
  }

  // Add the transaction to the finance page
  AddTransactionToPlayersBook(BOBBYR_PURCHASE, 0, -giGrandTotal);

  // display the confirm order graphic
  gfDrawConfirmOrderGrpahic = TRUE;

  // Get rid of the city drop dowm, if it is being displayed
  gubDropDownAction = BR_DROP_DOWN_DESTROY;

  MSYS_EnableRegion(&gSelectedConfirmOrderRegion);
  gfRemoveItemsFromStock = TRUE;

  gbSelectedCity = -1;
}

void AddJohnsGunShipment() {
  BobbyRayPurchaseStruct Temp[MAX_PURCHASE_AMOUNT];
  //	uint8_t	cnt;
  int8_t bDaysAhead;

  // clear out the memory
  memset(Temp, 0, sizeof(BobbyRayPurchaseStruct) * MAX_PURCHASE_AMOUNT);

  /*
          //if we need to add more array elements for the Order Array
          if( LaptopSaveInfo.usNumberOfBobbyRayOrderItems <=
     LaptopSaveInfo.usNumberOfBobbyRayOrderUsed )
          {
                  LaptopSaveInfo.usNumberOfBobbyRayOrderItems++;
                  LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = MemRealloc(
     LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray, sizeof( BobbyRayOrderStruct ) *
     LaptopSaveInfo.usNumberOfBobbyRayOrderItems ); if( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray
     == NULL ) return;

                  memset( &LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[
     LaptopSaveInfo.usNumberOfBobbyRayOrderItems - 1 ], 0, sizeof( BobbyRayOrderStruct ) );
          }

          for( cnt =0; cnt< LaptopSaveInfo.usNumberOfBobbyRayOrderItems; cnt++ )
          {
                  //get an empty element in the array
                  if( !LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].fActive )
                          break;
          }
  */

  // want to add two guns (Automags, AUTOMAG_III), and four clips of ammo.

  Temp[0].usItemIndex = AUTOMAG_III;
  Temp[0].ubNumberPurchased = 2;
  Temp[0].bItemQuality = 100;
  Temp[0].usBobbyItemIndex = 0;  // does this get used anywhere???
  Temp[0].fUsed = FALSE;

  //	memcpy( &(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].BobbyRayPurchase[0]), &Temp,
  // sizeof(BobbyRayPurchaseStruct) );

  Temp[1].usItemIndex = CLIP762N_5_AP;
  Temp[1].ubNumberPurchased = 2;
  Temp[1].bItemQuality = 5;
  Temp[1].usBobbyItemIndex = 0;  // does this get used anywhere???
  Temp[1].fUsed = FALSE;

  /*
          memcpy( &(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].BobbyRayPurchase[1]), &Temp,
     sizeof(BobbyRayPurchaseStruct) );


          LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].ubNumberPurchases = 2;
          LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].fActive = TRUE;
          LaptopSaveInfo.usNumberOfBobbyRayOrderUsed++;
  */
  bDaysAhead = CalculateOrderDelay(2) + 2;

  // add a random amount between so it arrives between 8:00 am and noon
  //	AddFutureDayStrategicEvent( EVENT_BOBBYRAY_PURCHASE, (8 + Random(4) ) * 60, cnt, bDaysAhead
  //);

  // add the delivery	( weight is not needed as it will not be displayed )
  AddNewBobbyRShipment(Temp, BR_DRASSEN, bDaysAhead, FALSE, 0);
}

void ConfirmBobbyRPurchaseMessageBoxCallBack(uint8_t bExitValue) {
  // yes, load the game
  if (bExitValue == MSG_BOX_RETURN_YES) {
    PurchaseBobbyOrder();
  }
}

void EnterInitBobbyRayOrder() {
  memset(&BobbyRayPurchases, 0, sizeof(BobbyRayPurchaseStruct) * MAX_PURCHASE_AMOUNT);
  gubSelectedLight = 0;
  gfReDrawBobbyOrder = TRUE;
  gbSelectedCity = -1;
  gubCityAtTopOfList = 0;

  // Get rid of the city drop dowm, if it is being displayed
  gubDropDownAction = BR_DROP_DOWN_DESTROY;

  // disable the accept order button
  DisableButton(guiBobbyRAcceptOrder);
}

uint32_t CalcPackageTotalWeight() {
  uint16_t i;
  uint32_t uiTotalWeight = 0;

  // loop through all the packages
  for (i = 0; i < MAX_PURCHASE_AMOUNT; i++) {
    // if the item was purchased
    if (BobbyRayPurchases[i].ubNumberPurchased) {
      // add the current weight to the total
      uiTotalWeight +=
          Item[BobbyRayPurchases[i].usItemIndex].ubWeight * BobbyRayPurchases[i].ubNumberPurchased;
    }
  }

  return (uiTotalWeight);
}

void DisplayPackageWeight() {
  wchar_t zTemp[32];
  uint32_t uiTotalWeight = CalcPackageTotalWeight();
  //	float			fWeight = (float)(uiTotalWeight / 10.0);

  // Display the 'Package Weight' text
  DrawTextToScreen(BobbyROrderFormText[BOBBYR_PACKAGE_WEIGHT], BOBBYR_PACKAXGE_WEIGHT_X + 8,
                   BOBBYR_PACKAXGE_WEIGHT_Y + 4, BOBBYR_PACKAXGE_WEIGHT_WIDTH,
                   BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the weight
  //	swprintf( zTemp, L"%3.1f %s", fWeight, pMessageStrings[ MSG_KILOGRAM_ABBREVIATION ] );
  swprintf(zTemp, ARR_SIZE(zTemp), L"%3.1f %s",
           (GetWeightBasedOnMetricOption(uiTotalWeight) / 10.0f), GetWeightUnitString());
  DrawTextToScreen(zTemp, BOBBYR_PACKAXGE_WEIGHT_X + 3, BOBBYR_PACKAXGE_WEIGHT_Y + 4,
                   BOBBYR_PACKAXGE_WEIGHT_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT,
                   BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
}

void BtnBobbyRGotoShipmentPageCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    guiCurrentLaptopMode = LAPTOP_MODE_BOBBYR_SHIPMENTS;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

BOOLEAN CreateBobbyRayOrderTitle() {
  // load BobbyRayTitle graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyRayTitle.sti"), &guiBobbyRayTitle));

  // the link to home page from the title
  MSYS_DefineRegion(&gSelectedTitleLinkRegion, BOBBYR_BOBBY_RAY_TITLE_X, BOBBYR_BOBBY_RAY_TITLE_Y,
                    (BOBBYR_BOBBY_RAY_TITLE_X + BOBBYR_BOBBY_RAY_TITLE_WIDTH),
                    (uint16_t)(BOBBYR_BOBBY_RAY_TITLE_Y + BOBBYR_BOBBY_RAY_TITLE_HEIGHT),
                    MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
                    SelectTitleLinkRegionCallBack);
  MSYS_AddRegion(&gSelectedTitleLinkRegion);

  return (TRUE);
}

void DestroyBobbyROrderTitle() {
  MSYS_RemoveRegion(&gSelectedTitleLinkRegion);
  DeleteVideoObjectFromIndex(guiBobbyRayTitle);
}

void DrawBobbyROrderTitle() {
  struct VObject *hPixHandle;

  // Bobbyray title
  GetVideoObject(&hPixHandle, guiBobbyRayTitle);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_BOBBY_RAY_TITLE_X, BOBBYR_BOBBY_RAY_TITLE_Y);
}

BOOLEAN AddNewBobbyRShipment(BobbyRayPurchaseStruct *pPurchaseStruct, uint8_t ubDeliveryLoc,
                             uint8_t ubDeliveryMethod, BOOLEAN fPruchasedFromBobbyR,
                             uint32_t uiPackageWeight) {
  int32_t iCnt;
  int32_t iFoundSpot = -1;
  uint8_t ubItemCount = 0;
  uint8_t i;
  int8_t bDaysAhead = 0;
  //	uint32_t	uiPackageWeight;
  //	gpNewBobbyrShipments = NULL;
  //	giNumberOfNewBobbyRShipment = 0;

  // loop through and see if there is a free spot to insert the new order
  for (iCnt = 0; iCnt < giNumberOfNewBobbyRShipment; iCnt++) {
    if (!gpNewBobbyrShipments->fActive) {
      iFoundSpot = iCnt;
      break;
    }
  }

  if (iFoundSpot == -1) {
    // increment the number of spots used
    giNumberOfNewBobbyRShipment++;

    // allocate some more memory
    gpNewBobbyrShipments = (NewBobbyRayOrderStruct *)MemRealloc(
        gpNewBobbyrShipments, sizeof(NewBobbyRayOrderStruct) * giNumberOfNewBobbyRShipment);

    iFoundSpot = giNumberOfNewBobbyRShipment - 1;
  }

  // memset the memory
  memset(&gpNewBobbyrShipments[iFoundSpot], 0, sizeof(NewBobbyRayOrderStruct));

  gpNewBobbyrShipments[iFoundSpot].fActive = TRUE;
  gpNewBobbyrShipments[iFoundSpot].ubDeliveryLoc = ubDeliveryLoc;
  gpNewBobbyrShipments[iFoundSpot].ubDeliveryMethod = ubDeliveryMethod;

  if (fPruchasedFromBobbyR)
    gpNewBobbyrShipments[iFoundSpot].fDisplayedInShipmentPage = TRUE;
  else
    gpNewBobbyrShipments[iFoundSpot].fDisplayedInShipmentPage = FALSE;

  // get the apckage weight, if the weight is "below" the minimum, use the minimum
  if (uiPackageWeight < MIN_SHIPPING_WEIGHT) {
    gpNewBobbyrShipments[iFoundSpot].uiPackageWeight = MIN_SHIPPING_WEIGHT;
  } else {
    gpNewBobbyrShipments[iFoundSpot].uiPackageWeight = uiPackageWeight;
  }

  gpNewBobbyrShipments[iFoundSpot].uiOrderedOnDayNum = GetWorldDay();

  // count the number of purchases
  ubItemCount = 0;
  for (i = 0; i < MAX_PURCHASE_AMOUNT; i++) {
    // if the item was purchased
    if (pPurchaseStruct[i].ubNumberPurchased) {
      // copy the new data into the order struct
      memcpy(&gpNewBobbyrShipments[iFoundSpot].BobbyRayPurchase[ubItemCount], &pPurchaseStruct[i],
             sizeof(BobbyRayPurchaseStruct));

      // copy the purchases into the struct that will be added to the queue
      //			memcpy(&LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt
      //].BobbyRayPurchase[ ubCount ] , &BobbyRayPurchases[i],  sizeof(BobbyRayPurchaseStruct));
      ubItemCount++;
    }
  }

  gpNewBobbyrShipments[iFoundSpot].ubNumberPurchases = ubItemCount;

  // get the length of time to receive the shipment
  if (fPruchasedFromBobbyR)
    bDaysAhead = CalculateOrderDelay(ubDeliveryMethod);
  else
    bDaysAhead = ubDeliveryMethod;

  // AddStrategicEvent( EVENT_BOBBYRAY_PURCHASE, uiResetTimeSec, cnt);
  AddFutureDayStrategicEvent(EVENT_BOBBYRAY_PURCHASE, (8 + Random(4)) * 60, iFoundSpot, bDaysAhead);

  return (TRUE);
}

uint16_t CountNumberOfBobbyPurchasesThatAreInTransit() {
  uint16_t usItemCount = 0;
  int32_t iCnt;

  for (iCnt = 0; iCnt < giNumberOfNewBobbyRShipment; iCnt++) {
    if (gpNewBobbyrShipments[iCnt].fActive) {
      usItemCount++;
    }
  }

  return (usItemCount);
}

BOOLEAN NewWayOfSavingBobbyRMailOrdersToSaveGameFile(HWFILE hFile) {
  int32_t iCnt;
  uint32_t uiNumBytesWritten;

  // Write the number of orders
  FileMan_Write(hFile, &giNumberOfNewBobbyRShipment, sizeof(int32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(int32_t)) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  // loop through and save all the mail order slots
  for (iCnt = 0; iCnt < giNumberOfNewBobbyRShipment; iCnt++) {
    // Write the order
    FileMan_Write(hFile, &gpNewBobbyrShipments[iCnt], sizeof(NewBobbyRayOrderStruct),
                  &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(NewBobbyRayOrderStruct)) {
      FileMan_Close(hFile);
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(HWFILE hFile) {
  int32_t iCnt;
  uint32_t uiNumBytesRead;

  // clear out the old list
  ShutDownBobbyRNewMailOrders();

  // Read the number of orders
  FileMan_Read(hFile, &giNumberOfNewBobbyRShipment, sizeof(int32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(int32_t)) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  if (giNumberOfNewBobbyRShipment == 0) {
    gpNewBobbyrShipments = NULL;
  } else {
    // Allocate memory for the list
    gpNewBobbyrShipments = (NewBobbyRayOrderStruct *)MemAlloc(sizeof(NewBobbyRayOrderStruct) *
                                                              giNumberOfNewBobbyRShipment);
    if (gpNewBobbyrShipments == NULL) {
      Assert(0);
      return (FALSE);
    }

    // loop through and load all the mail order slots
    for (iCnt = 0; iCnt < giNumberOfNewBobbyRShipment; iCnt++) {
      // Read the order
      FileMan_Read(hFile, &gpNewBobbyrShipments[iCnt], sizeof(NewBobbyRayOrderStruct),
                   &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(NewBobbyRayOrderStruct)) {
        FileMan_Close(hFile);
        return (FALSE);
      }
    }
  }

  return (TRUE);
}
