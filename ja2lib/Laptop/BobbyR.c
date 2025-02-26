// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/BobbyR.h"

#include <string.h>

#include "Laptop/BobbyRGuns.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "Laptop/StoreInventory.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Tactical/ArmsDealerInvInit.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Weapons.h"
#include "Utils/Cursors.h"
#include "Utils/Message.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#ifdef JA2TESTVERSION
#define BR_INVENTORY_TURNOVER_DEBUG
#endif

#define BOBBIES_SIGN_FONT FONT14ARIAL
#define BOBBIES_SIGN_COLOR 2
#define BOBBIES_SIGN_BACKCOLOR FONT_MCOLOR_BLACK
#define BOBBIES_SIGN_BACKGROUNDCOLOR 78  // NO_SHADOW

#define BOBBIES_NUMBER_SIGNS 5

#define BOBBIES_SENTENCE_FONT FONT12ARIAL
#define BOBBIES_SENTENCE_COLOR FONT_MCOLOR_WHITE
#define BOBBIES_SENTENCE_BACKGROUNDCOLOR 2  // NO_SHADOW//226

#define BOBBY_WOOD_BACKGROUND_X LAPTOP_SCREEN_UL_X
#define BOBBY_WOOD_BACKGROUND_Y LAPTOP_SCREEN_WEB_UL_Y
#define BOBBY_WOOD_BACKGROUND_WIDTH 125
#define BOBBY_WOOD_BACKGROUND_HEIGHT 100

#define BOBBY_RAYS_NAME_X LAPTOP_SCREEN_UL_X + 77
#define BOBBY_RAYS_NAME_Y LAPTOP_SCREEN_WEB_UL_Y + 0
#define BOBBY_RAYS_NAME_WIDTH 344
#define BOBBY_RAYS_NAME_HEIGHT 66

#define BOBBYS_PLAQUES_X LAPTOP_SCREEN_UL_X + 39
#define BOBBYS_PLAQUES_Y LAPTOP_SCREEN_WEB_UL_Y + 174
#define BOBBYS_PLAQUES_WIDTH 414
#define BOBBYS_PLAQUES_HEIGHT 190

#define BOBBIES_TOPHINGE_X LAPTOP_SCREEN_UL_X
#define BOBBIES_TOPHINGE_Y LAPTOP_SCREEN_WEB_UL_Y + 42

#define BOBBIES_BOTTOMHINGE_X LAPTOP_SCREEN_UL_X
#define BOBBIES_BOTTOMHINGE_Y LAPTOP_SCREEN_WEB_UL_Y + 338

#define BOBBIES_STORE_PLAQUE_X LAPTOP_SCREEN_UL_X + 148
#define BOBBIES_STORE_PLAQUE_Y LAPTOP_SCREEN_WEB_UL_Y + 66
#define BOBBIES_STORE_PLAQUE_HEIGHT 93

#define BOBBIES_HANDLE_X LAPTOP_SCREEN_UL_X + 457
#define BOBBIES_HANDLE_Y LAPTOP_SCREEN_WEB_UL_Y + 147

#define BOBBIES_FIRST_SENTENCE_X LAPTOP_SCREEN_UL_X
#define BOBBIES_FIRST_SENTENCE_Y BOBBIES_STORE_PLAQUE_Y + BOBBIES_STORE_PLAQUE_HEIGHT - 3
#define BOBBIES_FIRST_SENTENCE_WIDTH 500

#define BOBBIES_2ND_SENTENCE_X LAPTOP_SCREEN_UL_X
#define BOBBIES_2ND_SENTENCE_Y BOBBIES_FIRST_SENTENCE_Y + 13
#define BOBBIES_2ND_SENTENCE_WIDTH 500

#define BOBBIES_CENTER_SIGN_OFFSET_Y 23

#define BOBBIES_USED_SIGN_X BOBBYS_PLAQUES_X + 93
#define BOBBIES_USED_SIGN_Y BOBBYS_PLAQUES_Y + 32
#define BOBBIES_USED_SIGN_WIDTH 92
#define BOBBIES_USED_SIGN_HEIGHT 50
#define BOBBIES_USED_SIGN_TEXT_OFFSET BOBBIES_USED_SIGN_Y + 10

#define BOBBIES_MISC_SIGN_X BOBBYS_PLAQUES_X + 238
#define BOBBIES_MISC_SIGN_Y BOBBYS_PLAQUES_Y + 27
#define BOBBIES_MISC_SIGN_WIDTH 103
#define BOBBIES_MISC_SIGN_HEIGHT 57
#define BOBBIES_MISC_SIGN_TEXT_OFFSET BOBBIES_MISC_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_GUNS_SIGN_X BOBBYS_PLAQUES_X + 3
#define BOBBIES_GUNS_SIGN_Y BOBBYS_PLAQUES_Y + 102
#define BOBBIES_GUNS_SIGN_WIDTH 116
#define BOBBIES_GUNS_SIGN_HEIGHT 75
#define BOBBIES_GUNS_SIGN_TEXT_OFFSET BOBBIES_GUNS_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_AMMO_SIGN_X BOBBYS_PLAQUES_X + 150
#define BOBBIES_AMMO_SIGN_Y BOBBYS_PLAQUES_Y + 105
#define BOBBIES_AMMO_SIGN_WIDTH 112
#define BOBBIES_AMMO_SIGN_HEIGHT 71
#define BOBBIES_AMMO_SIGN_TEXT_OFFSET BOBBIES_AMMO_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_ARMOUR_SIGN_X BOBBYS_PLAQUES_X + 290
#define BOBBIES_ARMOUR_SIGN_Y BOBBYS_PLAQUES_Y + 108
#define BOBBIES_ARMOUR_SIGN_WIDTH 114
#define BOBBIES_ARMOUR_SIGN_HEIGHT 70
#define BOBBIES_ARMOUR_SIGN_TEXT_OFFSET BOBBIES_ARMOUR_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_3RD_SENTENCE_X LAPTOP_SCREEN_UL_X
#define BOBBIES_3RD_SENTENCE_Y BOBBIES_BOTTOMHINGE_Y + 40
#define BOBBIES_3RD_SENTENCE_WIDTH 500

#define BOBBY_R_NEW_PURCHASE_ARRIVAL_TIME (1 * 60 * 24)  // minutes in 1 day

#define BOBBY_R_USED_PURCHASE_OFFSET 1000

#define BOBBYR_UNDERCONSTRUCTION_ANI_DELAY 150
#define BOBBYR_UNDERCONSTRUCTION_NUM_FRAMES 5

#define BOBBYR_UNDERCONSTRUCTION_X \
  LAPTOP_SCREEN_UL_X +             \
      (LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X - BOBBYR_UNDERCONSTRUCTION_WIDTH) / 2
#define BOBBYR_UNDERCONSTRUCTION_Y 175
#define BOBBYR_UNDERCONSTRUCTION1_Y 378

#define BOBBYR_UNDERCONSTRUCTION_WIDTH 414
#define BOBBYR_UNDERCONSTRUCTION_HEIGHT 64

#define BOBBYR_UNDER_CONSTRUCTION_TEXT_X LAPTOP_SCREEN_UL_X
#define BOBBYR_UNDER_CONSTRUCTION_TEXT_Y BOBBYR_UNDERCONSTRUCTION_Y + 62 + 60
#define BOBBYR_UNDER_CONSTRUCTION_TEXT_WIDTH LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X

uint32_t guiBobbyName;
uint32_t guiPlaque;
uint32_t guiTopHinge;
uint32_t guiBottomHinge;
uint32_t guiStorePlaque;
uint32_t guiHandle;
uint32_t guiWoodBackground;
uint32_t guiUnderConstructionImage;

/*
uint16_t	gusFirstGunIndex;
uint16_t	gusLastGunIndex;
uint8_t		gubNumGunPages;

uint16_t	gusFirstAmmoIndex;
uint16_t	gusLastAmmoIndex;
uint8_t		gubNumAmmoPages;

uint16_t	gusFirstMiscIndex;
uint16_t	gusLastMiscIndex;
uint8_t		gubNumMiscPages;

uint16_t  gusFirstArmourIndex;
uint16_t  gusLastArmourIndex;
uint8_t		gubNumArmourPages;

uint16_t  gusFirstUsedIndex;
uint16_t  gusLastUsedIndex;
uint8_t		gubNumUsedPages;
*/

uint32_t guiLastBobbyRayPage;

uint8_t gubBobbyRPages[] = {LAPTOP_MODE_BOBBY_R_USED, LAPTOP_MODE_BOBBY_R_MISC,
                            LAPTOP_MODE_BOBBY_R_GUNS, LAPTOP_MODE_BOBBY_R_AMMO,
                            LAPTOP_MODE_BOBBY_R_ARMOR};

// Bobby's Sign menu mouse regions
struct MOUSE_REGION gSelectedBobbiesSignMenuRegion[BOBBIES_NUMBER_SIGNS];
void SelectBobbiesSignMenuRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

BOOLEAN InitBobbiesMouseRegion(uint8_t ubNumerRegions, uint16_t *usMouseRegionPosArray,
                               struct MOUSE_REGION *MouseRegion);
BOOLEAN RemoveBobbiesMouseRegion(uint8_t ubNumberRegions, struct MOUSE_REGION *Mouse_Region);
void HandleBobbyRUnderConstructionAni(BOOLEAN fReset);

void SimulateBobbyRayCustomer(STORE_INVENTORY *pInventoryArray, BOOLEAN fUsed);

void GameInitBobbyR() {}

BOOLEAN EnterBobbyR() {
  uint8_t i;

  // an array of mouse regions for the bobbies signs.  Top Left corner, bottom right corner
  uint16_t usMouseRegionPosArray[] = {BOBBIES_USED_SIGN_X,
                                      BOBBIES_USED_SIGN_Y,
                                      BOBBIES_USED_SIGN_X + BOBBIES_USED_SIGN_WIDTH,
                                      BOBBIES_USED_SIGN_Y + BOBBIES_USED_SIGN_HEIGHT,
                                      BOBBIES_MISC_SIGN_X,
                                      BOBBIES_MISC_SIGN_Y,
                                      BOBBIES_MISC_SIGN_X + BOBBIES_MISC_SIGN_WIDTH,
                                      BOBBIES_MISC_SIGN_Y + BOBBIES_MISC_SIGN_HEIGHT,
                                      BOBBIES_GUNS_SIGN_X,
                                      BOBBIES_GUNS_SIGN_Y,
                                      BOBBIES_GUNS_SIGN_X + BOBBIES_GUNS_SIGN_WIDTH,
                                      BOBBIES_GUNS_SIGN_Y + BOBBIES_GUNS_SIGN_HEIGHT,
                                      BOBBIES_AMMO_SIGN_X,
                                      BOBBIES_AMMO_SIGN_Y,
                                      BOBBIES_AMMO_SIGN_X + BOBBIES_AMMO_SIGN_WIDTH,
                                      BOBBIES_AMMO_SIGN_Y + BOBBIES_AMMO_SIGN_HEIGHT,
                                      BOBBIES_ARMOUR_SIGN_X,
                                      BOBBIES_ARMOUR_SIGN_Y,
                                      BOBBIES_ARMOUR_SIGN_X + BOBBIES_ARMOUR_SIGN_WIDTH,
                                      BOBBIES_ARMOUR_SIGN_Y + BOBBIES_ARMOUR_SIGN_HEIGHT};

  InitBobbyRWoodBackground();

  // load the Bobbyname graphic and add it
  CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_BOBBYNAME), &guiBobbyName));

  // load the plaque graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyPlaques.sti"), &guiPlaque));

  // load the TopHinge graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyTopHinge.sti"), &guiTopHinge));

  // load the BottomHinge graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyBottomHinge.sti"), &guiBottomHinge));

  // load the Store Plaque graphic and add it
  CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_STOREPLAQUE), &guiStorePlaque));

  // load the Handle graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyHandle.sti"), &guiHandle));

  InitBobbiesMouseRegion(BOBBIES_NUMBER_SIGNS, usMouseRegionPosArray,
                         gSelectedBobbiesSignMenuRegion);

  if (!LaptopSaveInfo.fBobbyRSiteCanBeAccessed) {
    // load the Handle graphic and add it
    CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\UnderConstruction.sti"),
                      &guiUnderConstructionImage));

    for (i = 0; i < BOBBIES_NUMBER_SIGNS; i++) {
      MSYS_DisableRegion(&gSelectedBobbiesSignMenuRegion[i]);
    }

    LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction =
        BOBBYR_BEEN_TO_SITE_ONCE;
  }

  SetBookMark(BOBBYR_BOOKMARK);
  HandleBobbyRUnderConstructionAni(TRUE);

  RenderBobbyR();

  return (TRUE);
}

void ExitBobbyR() {
  DeleteVideoObjectFromIndex(guiBobbyName);
  DeleteVideoObjectFromIndex(guiPlaque);
  DeleteVideoObjectFromIndex(guiTopHinge);
  DeleteVideoObjectFromIndex(guiBottomHinge);
  DeleteVideoObjectFromIndex(guiStorePlaque);
  DeleteVideoObjectFromIndex(guiHandle);

  if (!LaptopSaveInfo.fBobbyRSiteCanBeAccessed) {
    DeleteVideoObjectFromIndex(guiUnderConstructionImage);
  }

  DeleteBobbyRWoodBackground();

  RemoveBobbiesMouseRegion(BOBBIES_NUMBER_SIGNS, gSelectedBobbiesSignMenuRegion);

  guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R;
}

void HandleBobbyR() { HandleBobbyRUnderConstructionAni(FALSE); }

void RenderBobbyR() {
  struct VObject *hPixHandle;
  struct VObject *hStorePlaqueHandle;

  DrawBobbyRWoodBackground();

  // Bobby's Name
  GetVideoObject(&hPixHandle, guiBobbyName);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBY_RAYS_NAME_X, BOBBY_RAYS_NAME_Y);

  // Plaque
  GetVideoObject(&hPixHandle, guiPlaque);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYS_PLAQUES_X, BOBBYS_PLAQUES_Y);

  // Top Hinge
  GetVideoObject(&hPixHandle, guiTopHinge);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBIES_TOPHINGE_X, BOBBIES_TOPHINGE_Y);

  // Bottom Hinge
  GetVideoObject(&hPixHandle, guiBottomHinge);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBIES_BOTTOMHINGE_X, BOBBIES_BOTTOMHINGE_Y);

  // StorePlaque
  GetVideoObject(&hStorePlaqueHandle, guiStorePlaque);
  BltVideoObject(vsFB, hStorePlaqueHandle, 0, BOBBIES_STORE_PLAQUE_X, BOBBIES_STORE_PLAQUE_Y);

  // Handle
  GetVideoObject(&hPixHandle, guiHandle);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBIES_HANDLE_X, BOBBIES_HANDLE_Y);

  SetFontShadow(BOBBIES_SENTENCE_BACKGROUNDCOLOR);

  if (LaptopSaveInfo.fBobbyRSiteCanBeAccessed) {
    // Bobbys first sentence
    DrawTextToScreen(BobbyRaysFrontText[BOBBYR_ADVERTISMENT_1], BOBBIES_FIRST_SENTENCE_X,
                     BOBBIES_FIRST_SENTENCE_Y, BOBBIES_FIRST_SENTENCE_WIDTH, BOBBIES_SENTENCE_FONT,
                     BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, FALSE,
                     CENTER_JUSTIFIED | TEXT_SHADOWED);

    // Bobbys second sentence
    DrawTextToScreen(BobbyRaysFrontText[BOBBYR_ADVERTISMENT_2], BOBBIES_2ND_SENTENCE_X,
                     BOBBIES_2ND_SENTENCE_Y, BOBBIES_2ND_SENTENCE_WIDTH, BOBBIES_SENTENCE_FONT,
                     BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, FALSE,
                     CENTER_JUSTIFIED | TEXT_SHADOWED);
    SetFontShadow(DEFAULT_SHADOW);
  }

  SetFontShadow(BOBBIES_SIGN_BACKGROUNDCOLOR);
  // Text on the Used Sign
  DisplayWrappedString(BOBBIES_USED_SIGN_X, BOBBIES_USED_SIGN_TEXT_OFFSET,
                       BOBBIES_USED_SIGN_WIDTH - 5, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR,
                       BobbyRaysFrontText[BOBBYR_USED], BOBBIES_SIGN_BACKCOLOR, FALSE,
                       CENTER_JUSTIFIED);
  // Text on the Misc Sign
  DisplayWrappedString(BOBBIES_MISC_SIGN_X, BOBBIES_MISC_SIGN_TEXT_OFFSET, BOBBIES_MISC_SIGN_WIDTH,
                       2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_MISC],
                       BOBBIES_SIGN_BACKCOLOR, FALSE, CENTER_JUSTIFIED);
  // Text on the Guns Sign
  DisplayWrappedString(BOBBIES_GUNS_SIGN_X, BOBBIES_GUNS_SIGN_TEXT_OFFSET, BOBBIES_GUNS_SIGN_WIDTH,
                       2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_GUNS],
                       BOBBIES_SIGN_BACKCOLOR, FALSE, CENTER_JUSTIFIED);
  // Text on the Ammo Sign
  DisplayWrappedString(BOBBIES_AMMO_SIGN_X, BOBBIES_AMMO_SIGN_TEXT_OFFSET, BOBBIES_AMMO_SIGN_WIDTH,
                       2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_AMMO],
                       BOBBIES_SIGN_BACKCOLOR, FALSE, CENTER_JUSTIFIED);
  // Text on the Armour Sign
  DisplayWrappedString(BOBBIES_ARMOUR_SIGN_X, BOBBIES_ARMOUR_SIGN_TEXT_OFFSET,
                       BOBBIES_ARMOUR_SIGN_WIDTH, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR,
                       BobbyRaysFrontText[BOBBYR_ARMOR], BOBBIES_SIGN_BACKCOLOR, FALSE,
                       CENTER_JUSTIFIED);
  SetFontShadow(DEFAULT_SHADOW);

  if (LaptopSaveInfo.fBobbyRSiteCanBeAccessed) {
    // Bobbys Third sentence
    SetFontShadow(BOBBIES_SENTENCE_BACKGROUNDCOLOR);
    DrawTextToScreen(BobbyRaysFrontText[BOBBYR_ADVERTISMENT_3], BOBBIES_3RD_SENTENCE_X,
                     BOBBIES_3RD_SENTENCE_Y, BOBBIES_3RD_SENTENCE_WIDTH, BOBBIES_SENTENCE_FONT,
                     BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, FALSE,
                     CENTER_JUSTIFIED | TEXT_SHADOWED);
    SetFontShadow(DEFAULT_SHADOW);
  }

  // if we cant go to any sub pages, darken the page out
  if (!LaptopSaveInfo.fBobbyRSiteCanBeAccessed) {
    ShadowVideoSurfaceRect(vsFB, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                           LAPTOP_SCREEN_WEB_LR_Y);
  }

  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

BOOLEAN InitBobbyRWoodBackground() {
  // load the Wood bacground graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BobbyWood.sti"), &guiWoodBackground));

  return (TRUE);
}

BOOLEAN DeleteBobbyRWoodBackground() {
  DeleteVideoObjectFromIndex(guiWoodBackground);
  return (TRUE);
}

BOOLEAN DrawBobbyRWoodBackground() {
  struct VObject *hWoodBackGroundHandle;
  uint16_t x, y, uiPosX, uiPosY;

  // Blt the Wood background
  GetVideoObject(&hWoodBackGroundHandle, guiWoodBackground);

  uiPosY = BOBBY_WOOD_BACKGROUND_Y;
  for (y = 0; y < 4; y++) {
    uiPosX = BOBBY_WOOD_BACKGROUND_X;
    for (x = 0; x < 4; x++) {
      BltVideoObject(vsFB, hWoodBackGroundHandle, 0, uiPosX, uiPosY);
      uiPosX += BOBBY_WOOD_BACKGROUND_WIDTH;
    }
    uiPosY += BOBBY_WOOD_BACKGROUND_HEIGHT;
  }

  return (TRUE);
}

BOOLEAN InitBobbiesMouseRegion(uint8_t ubNumerRegions, uint16_t *usMouseRegionPosArray,
                               struct MOUSE_REGION *MouseRegion) {
  uint8_t i, ubCount = 0;

  for (i = 0; i < ubNumerRegions; i++) {
    // Mouse region for the toc buttons
    MSYS_DefineRegion(&MouseRegion[i], usMouseRegionPosArray[ubCount],
                      usMouseRegionPosArray[ubCount + 1], usMouseRegionPosArray[ubCount + 2],
                      usMouseRegionPosArray[ubCount + 3], MSYS_PRIORITY_HIGH, CURSOR_WWW,
                      MSYS_NO_CALLBACK, SelectBobbiesSignMenuRegionCallBack);
    MSYS_AddRegion(&MouseRegion[i]);
    MSYS_SetRegionUserData(&MouseRegion[i], 0, gubBobbyRPages[i]);

    ubCount += 4;
  }

  return (TRUE);
}

BOOLEAN RemoveBobbiesMouseRegion(uint8_t ubNumberRegions, struct MOUSE_REGION *Mouse_Region) {
  uint8_t i;

  for (i = 0; i < ubNumberRegions; i++) MSYS_RemoveRegion(&Mouse_Region[i]);

  return (TRUE);
}

void SelectBobbiesSignMenuRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint8_t ubNewPage = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
    guiCurrentLaptopMode = ubNewPage;
    //		FindLastItemIndex(ubNewPage);

  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void HandleBobbyRUnderConstructionAni(BOOLEAN fReset) {
  struct VObject *hPixHandle;
  static uint32_t uiLastTime = 1;
  static uint16_t usCount = 0;
  uint32_t uiCurTime = GetJA2Clock();

  if (LaptopSaveInfo.fBobbyRSiteCanBeAccessed) return;

  if (fReset) usCount = 1;

  if (fShowBookmarkInfo) {
    fReDrawBookMarkInfo = TRUE;
  }

  if (((uiCurTime - uiLastTime) > BOBBYR_UNDERCONSTRUCTION_ANI_DELAY) || (fReDrawScreenFlag)) {
    // The undercontsruction graphic
    GetVideoObject(&hPixHandle, guiUnderConstructionImage);
    BltVideoObject(vsFB, hPixHandle, usCount, BOBBYR_UNDERCONSTRUCTION_X,
                   BOBBYR_UNDERCONSTRUCTION_Y);

    BltVideoObject(vsFB, hPixHandle, usCount, BOBBYR_UNDERCONSTRUCTION_X,
                   BOBBYR_UNDERCONSTRUCTION1_Y);

    DrawTextToScreen(BobbyRaysFrontText[BOBBYR_UNDER_CONSTRUCTION],
                     BOBBYR_UNDER_CONSTRUCTION_TEXT_X, BOBBYR_UNDER_CONSTRUCTION_TEXT_Y,
                     BOBBYR_UNDER_CONSTRUCTION_TEXT_WIDTH, FONT16ARIAL, BOBBIES_SENTENCE_COLOR,
                     BOBBIES_SIGN_BACKCOLOR, FALSE, CENTER_JUSTIFIED | INVALIDATE_TEXT);

    InvalidateRegion(BOBBYR_UNDERCONSTRUCTION_X, BOBBYR_UNDERCONSTRUCTION_Y,
                     BOBBYR_UNDERCONSTRUCTION_X + BOBBYR_UNDERCONSTRUCTION_WIDTH,
                     BOBBYR_UNDERCONSTRUCTION_Y + BOBBYR_UNDERCONSTRUCTION_HEIGHT);
    InvalidateRegion(BOBBYR_UNDERCONSTRUCTION_X, BOBBYR_UNDERCONSTRUCTION1_Y,
                     BOBBYR_UNDERCONSTRUCTION_X + BOBBYR_UNDERCONSTRUCTION_WIDTH,
                     BOBBYR_UNDERCONSTRUCTION1_Y + BOBBYR_UNDERCONSTRUCTION_HEIGHT);

    uiLastTime = GetJA2Clock();

    usCount++;

    if (usCount >= BOBBYR_UNDERCONSTRUCTION_NUM_FRAMES) usCount = 0;
  }
}

void InitBobbyRayInventory() {
  // Initializes which NEW items can be bought at Bobby Rays
  InitBobbyRayNewInventory();

  // Initializes the starting values for Bobby Rays NEW Inventory
  SetupStoreInventory(LaptopSaveInfo.BobbyRayInventory, FALSE);

  // Initializes which USED items can be bought at Bobby Rays
  InitBobbyRayUsedInventory();

  // Initializes the starting values for Bobby Rays USED Inventory
  SetupStoreInventory(LaptopSaveInfo.BobbyRayUsedInventory, TRUE);
}

BOOLEAN InitBobbyRayNewInventory() {
  uint16_t i;
  uint16_t usBobbyrIndex = 0;

  memset(LaptopSaveInfo.BobbyRayInventory, 0, sizeof(STORE_INVENTORY) * MAXITEMS);

  // add all the NEW items he can ever sell into his possible inventory list, for now in order by
  // item #
  for (i = 0; i < MAXITEMS; i++) {
    // if Bobby Ray sells this, it can be sold, and it's allowed into this game (some depend on e.g.
    // gun-nut option)
    if ((StoreInventory[i][BOBBY_RAY_NEW] != 0) && !(Item[i].fFlags & ITEM_NOT_BUYABLE) &&
        ItemIsLegal(i)) {
      LaptopSaveInfo.BobbyRayInventory[usBobbyrIndex].usItemIndex = i;
      usBobbyrIndex++;
    }
  }

  if (usBobbyrIndex > 1) {
    // sort this list by object category, and by ascending price within each category
    qsort(LaptopSaveInfo.BobbyRayInventory, usBobbyrIndex, sizeof(STORE_INVENTORY),
          BobbyRayItemQsortCompare);
  }

  // remember how many entries in the list are valid
  LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_NEW] = usBobbyrIndex;
  // also mark the end of the list of valid item entries
  LaptopSaveInfo.BobbyRayInventory[usBobbyrIndex].usItemIndex = BOBBYR_NO_ITEMS;

  return (TRUE);
}

BOOLEAN InitBobbyRayUsedInventory() {
  uint16_t i;
  uint16_t usBobbyrIndex = 0;

  memset(LaptopSaveInfo.BobbyRayUsedInventory, 0, sizeof(STORE_INVENTORY) * MAXITEMS);

  // add all the NEW items he can ever sell into his possible inventory list, for now in order by
  // item #
  for (i = 0; i < MAXITEMS; i++) {
    // if Bobby Ray sells this, it can be sold, and it's allowed into this game (some depend on e.g.
    // gun-nut option)
    if ((StoreInventory[i][BOBBY_RAY_USED] != 0) && !(Item[i].fFlags & ITEM_NOT_BUYABLE) &&
        ItemIsLegal(i)) {
      if ((StoreInventory[i][BOBBY_RAY_USED] != 0) && !(Item[i].fFlags & ITEM_NOT_BUYABLE) &&
          ItemIsLegal(i))
        // in case his store inventory list is wrong, make sure this category of item can be sold
        // used
        if (CanDealerItemBeSoldUsed(i)) {
          LaptopSaveInfo.BobbyRayUsedInventory[usBobbyrIndex].usItemIndex = i;
          usBobbyrIndex++;
        }
    }
  }

  if (usBobbyrIndex > 1) {
    // sort this list by object category, and by ascending price within each category
    qsort(LaptopSaveInfo.BobbyRayUsedInventory, usBobbyrIndex, sizeof(STORE_INVENTORY),
          BobbyRayItemQsortCompare);
  }

  // remember how many entries in the list are valid
  LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_USED] = usBobbyrIndex;
  // also mark the end of the list of valid item entries
  LaptopSaveInfo.BobbyRayUsedInventory[usBobbyrIndex].usItemIndex = BOBBYR_NO_ITEMS;

  return (TRUE);
}

void DailyUpdateOfBobbyRaysNewInventory() {
  int16_t i;
  uint16_t usItemIndex;
  BOOLEAN fPrevElig;

  // simulate other buyers by reducing the current quantity on hand
  SimulateBobbyRayCustomer(LaptopSaveInfo.BobbyRayInventory, BOBBY_RAY_NEW);

  // loop through all items BR can stock to see what needs reordering
  for (i = 0; i < LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_NEW]; i++) {
    // the index is NOT the item #, get that from the table
    usItemIndex = LaptopSaveInfo.BobbyRayInventory[i].usItemIndex;

    Assert(usItemIndex < MAXITEMS);

    // make sure this item is still sellable in the latest version of the store inventory
    if (StoreInventory[usItemIndex][BOBBY_RAY_NEW] == 0) {
      continue;
    }

    // if the item isn't already on order
    if (LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnOrder == 0) {
      // if the qty on hand is half the desired amount or fewer
      if (LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnHand <=
          (StoreInventory[usItemIndex][BOBBY_RAY_NEW] / 2)) {
        // remember value of the "previously eligible" flag
        fPrevElig = LaptopSaveInfo.BobbyRayInventory[i].fPreviouslyEligible;

        // determine if any can/should be ordered, and how many
        LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnOrder = HowManyBRItemsToOrder(
            usItemIndex, LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnHand, BOBBY_RAY_NEW);

        // if he found some to buy
        if (LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnOrder > 0) {
          // if this is the first day the player is eligible to have access to this thing
          if (!fPrevElig) {
            // eliminate the ordering delay and stock the items instantly!
            // This is just a way to reward the player right away for making progress without the
            // reordering lag...
            AddFreshBobbyRayInventory(usItemIndex);
          } else {
            OrderBobbyRItem(usItemIndex);

#ifdef BR_INVENTORY_TURNOVER_DEBUG
            if (usItemIndex == ROCKET_LAUNCHER)
              MapScreenMessage(0, MSG_DEBUG, L"%s: BR Ordered %d, Has %d", WORLDTIMESTR,
                               LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnOrder,
                               LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnHand);
#endif
          }
        }
      }
    }
  }
}

void DailyUpdateOfBobbyRaysUsedInventory() {
  int16_t i;
  uint16_t usItemIndex;
  BOOLEAN fPrevElig;

  // simulate other buyers by reducing the current quantity on hand
  SimulateBobbyRayCustomer(LaptopSaveInfo.BobbyRayUsedInventory, BOBBY_RAY_USED);

  for (i = 0; i < LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_USED]; i++) {
    // if the used item isn't already on order
    if (LaptopSaveInfo.BobbyRayUsedInventory[i].ubQtyOnOrder == 0) {
      // if we don't have ANY
      if (LaptopSaveInfo.BobbyRayUsedInventory[i].ubQtyOnHand == 0) {
        // the index is NOT the item #, get that from the table
        usItemIndex = LaptopSaveInfo.BobbyRayUsedInventory[i].usItemIndex;
        Assert(usItemIndex < MAXITEMS);

        // make sure this item is still sellable in the latest version of the store inventory
        if (StoreInventory[usItemIndex][BOBBY_RAY_USED] == 0) {
          continue;
        }

        // remember value of the "previously eligible" flag
        fPrevElig = LaptopSaveInfo.BobbyRayUsedInventory[i].fPreviouslyEligible;

        // determine if any can/should be ordered, and how many
        LaptopSaveInfo.BobbyRayUsedInventory[i].ubQtyOnOrder = HowManyBRItemsToOrder(
            usItemIndex, LaptopSaveInfo.BobbyRayUsedInventory[i].ubQtyOnHand, BOBBY_RAY_USED);

        // if he found some to buy
        if (LaptopSaveInfo.BobbyRayUsedInventory[i].ubQtyOnOrder > 0) {
          // if this is the first day the player is eligible to have access to this thing
          if (!fPrevElig) {
            // eliminate the ordering delay and stock the items instantly!
            // This is just a way to reward the player right away for making progress without the
            // reordering lag...
            AddFreshBobbyRayInventory(usItemIndex);
          } else {
            OrderBobbyRItem((int16_t)(usItemIndex + BOBBY_R_USED_PURCHASE_OFFSET));
          }
        }
      }
    }
  }
}

// returns the number of items to order
uint8_t HowManyBRItemsToOrder(uint16_t usItemIndex, uint8_t ubCurrentlyOnHand,
                              uint8_t ubBobbyRayNewUsed) {
  uint8_t ubItemsOrdered = 0;

  Assert(usItemIndex < MAXITEMS);
  // formulas below will fail if there are more items already in stock than optimal
  Assert(ubCurrentlyOnHand <= StoreInventory[usItemIndex][ubBobbyRayNewUsed]);
  Assert(ubBobbyRayNewUsed < BOBBY_RAY_LISTS);

  // decide if he can get stock for this item (items are reordered an entire batch at a time)
  if (ItemTransactionOccurs(-1, usItemIndex, DEALER_BUYING, ubBobbyRayNewUsed)) {
    if (ubBobbyRayNewUsed == BOBBY_RAY_NEW) {
      ubItemsOrdered =
          HowManyItemsToReorder(StoreInventory[usItemIndex][ubBobbyRayNewUsed], ubCurrentlyOnHand);
    } else {
      // Since these are used items we only should order 1 of each type
      ubItemsOrdered = 1;
    }
  } else {
    // can't obtain this item from suppliers
    ubItemsOrdered = 0;
  }

  return (ubItemsOrdered);
}

void OrderBobbyRItem(uint16_t usItemIndex) {
  uint32_t uiArrivalTime;

  // add the new item to the queue.  The new item will arrive in 'uiArrivalTime' minutes.
  uiArrivalTime = BOBBY_R_NEW_PURCHASE_ARRIVAL_TIME + Random(BOBBY_R_NEW_PURCHASE_ARRIVAL_TIME / 2);
  uiArrivalTime += GetWorldTotalMin();
  AddStrategicEvent(EVENT_UPDATE_BOBBY_RAY_INVENTORY, uiArrivalTime, usItemIndex);
}

void AddFreshBobbyRayInventory(uint16_t usItemIndex) {
  int16_t sInventorySlot;
  STORE_INVENTORY *pInventoryArray;
  BOOLEAN fUsed;
  uint8_t ubItemQuality;

  if (usItemIndex >= BOBBY_R_USED_PURCHASE_OFFSET) {
    usItemIndex -= BOBBY_R_USED_PURCHASE_OFFSET;
    pInventoryArray = LaptopSaveInfo.BobbyRayUsedInventory;
    fUsed = BOBBY_RAY_USED;
    ubItemQuality = 20 + (uint8_t)Random(60);
  } else {
    pInventoryArray = LaptopSaveInfo.BobbyRayInventory;
    fUsed = BOBBY_RAY_NEW;
    ubItemQuality = 100;
  }

  // find out which inventory slot that item is stored in
  sInventorySlot = GetInventorySlotForItem(pInventoryArray, usItemIndex, fUsed);
  if (sInventorySlot == -1) {
    AssertMsg(FALSE, String("AddFreshBobbyRayInventory(), Item %d not found.  AM-0.", usItemIndex));
    return;
  }

  pInventoryArray[sInventorySlot].ubQtyOnHand += pInventoryArray[sInventorySlot].ubQtyOnOrder;
  pInventoryArray[sInventorySlot].ubItemQuality = ubItemQuality;

#ifdef BR_INVENTORY_TURNOVER_DEBUG
  if (usItemIndex == ROCKET_LAUNCHER && !fUsed)
    MapScreenMessage(0, MSG_DEBUG, L"%s: BR Bought %d, Has %d", WORLDTIMESTR,
                     pInventoryArray[sInventorySlot].ubQtyOnOrder,
                     pInventoryArray[sInventorySlot].ubQtyOnHand);
#endif

  // cancel order
  pInventoryArray[sInventorySlot].ubQtyOnOrder = 0;
}

int16_t GetInventorySlotForItem(STORE_INVENTORY *pInventoryArray, uint16_t usItemIndex,
                                BOOLEAN fUsed) {
  int16_t i;

  for (i = 0; i < LaptopSaveInfo.usInventoryListLength[fUsed]; i++) {
    // if we have some of this item in stock
    if (pInventoryArray[i].usItemIndex == usItemIndex) {
      return (i);
    }
  }

  // not found!
  return (-1);
}

void SimulateBobbyRayCustomer(STORE_INVENTORY *pInventoryArray, BOOLEAN fUsed) {
  int16_t i;
  uint8_t ubItemsSold;

  // loop through all items BR can stock to see what gets sold
  for (i = 0; i < LaptopSaveInfo.usInventoryListLength[fUsed]; i++) {
    // if we have some of this item in stock
    if (pInventoryArray[i].ubQtyOnHand > 0) {
      ubItemsSold = HowManyItemsAreSold(-1, pInventoryArray[i].usItemIndex,
                                        pInventoryArray[i].ubQtyOnHand, fUsed);
      pInventoryArray[i].ubQtyOnHand -= ubItemsSold;

#ifdef BR_INVENTORY_TURNOVER_DEBUG
      if (ubItemsSold > 0) {
        if (i == ROCKET_LAUNCHER && !fUsed)
          MapScreenMessage(0, MSG_DEBUG, L"%s: BR Sold %d, Has %d", WORLDTIMESTR, ubItemsSold,
                           pInventoryArray[i].ubQtyOnHand);
      }
#endif
    }
  }
}

void CancelAllPendingBRPurchaseOrders(void) {
  int16_t i;

  // remove all the BR-Order events off the event queue
  DeleteAllStrategicEventsOfType(EVENT_UPDATE_BOBBY_RAY_INVENTORY);

  // zero out all the quantities on order
  for (i = 0; i < MAXITEMS; i++) {
    LaptopSaveInfo.BobbyRayInventory[i].ubQtyOnOrder = 0;
    LaptopSaveInfo.BobbyRayUsedInventory[i].ubQtyOnOrder = 0;
  }

  // do an extra daily update immediately to create new reorders ASAP
  DailyUpdateOfBobbyRaysNewInventory();
  DailyUpdateOfBobbyRaysUsedInventory();
}
