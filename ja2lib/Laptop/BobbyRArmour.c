// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/BobbyRArmour.h"

#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRGuns.h"
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

uint32_t guiArmourBackground;
uint32_t guiArmourGrid;

void GameInitBobbyRArmour() {}

BOOLEAN EnterBobbyRArmour() {
  // load the background graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Armourbackground.sti"), &guiArmourBackground));

  // load the gunsgrid graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Armourgrid.sti"), &guiArmourGrid));

  InitBobbyBrTitle();
  // Draw menu bar
  InitBobbyMenuBar();

  SetFirstLastPagesForNew(IC_ARMOUR);
  //	CalculateFirstAndLastIndexs();

  RenderBobbyRArmour();

  return (TRUE);
}

void ExitBobbyRArmour() {
  DeleteVideoObjectFromIndex(guiArmourBackground);
  DeleteVideoObjectFromIndex(guiArmourGrid);
  DeleteBobbyMenuBar();

  DeleteBobbyBrTitle();
  DeleteMouseRegionForBigImage();

  giCurrentSubPage = gusCurWeaponIndex;
  guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_ARMOR;
}

void HandleBobbyRArmour() {}

void RenderBobbyRArmour() {
  struct VObject* hPixHandle;

  WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES,
                        BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiArmourBackground);

  // Display title at top of page
  DisplayBobbyRBrTitle();

  // GunForm
  GetVideoObject(&hPixHandle, guiArmourGrid);
  BltVideoObject(vsFB, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

  DisplayItemInfo(IC_ARMOUR);

  UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty();
  RenderWWWProgramTitleBar();
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}
