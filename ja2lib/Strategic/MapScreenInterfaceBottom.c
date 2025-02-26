// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/MapScreenInterfaceBottom.h"

#include "GameLoop.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/Finances.h"
#include "MapScreenInterfaceMapInventory.h"
#include "Money.h"
#include "OptionsScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MapScreenInterfaceTownMineInfo.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/MercContract.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AirRaid.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/TacticalSave.h"
#include "TacticalAI/AI.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define MAP_BOTTOM_X 0
#define MAP_BOTTOM_Y 359

#define MESSAGE_SCROLL_AREA_START_X 330
#define MESSAGE_SCROLL_AREA_END_X 344
#define MESSAGE_SCROLL_AREA_WIDTH (MESSAGE_SCROLL_AREA_END_X - MESSAGE_SCROLL_AREA_START_X + 1)

#define MESSAGE_SCROLL_AREA_START_Y 390
#define MESSAGE_SCROLL_AREA_END_Y 448
#define MESSAGE_SCROLL_AREA_HEIGHT (MESSAGE_SCROLL_AREA_END_Y - MESSAGE_SCROLL_AREA_START_Y + 1)

#define SLIDER_HEIGHT 11
#define SLIDER_WIDTH 11

#define SLIDER_BAR_RANGE (MESSAGE_SCROLL_AREA_HEIGHT - SLIDER_HEIGHT)

#define MESSAGE_BTN_SCROLL_TIME 100

// delay for paused flash
#define PAUSE_GAME_TIMER 500

#define MAP_BOTTOM_FONT_COLOR (32 * 4 - 9)

/*
// delay to start auto message scroll
#define DELAY_TO_START_MESSAGE_SCROLL 3000
// delay per auto message scroll
#define DELAY_PER_MESSAGE_SCROLL 300
*/

// button enums
enum {
  MAP_SCROLL_MESSAGE_UP = 0,
  MAP_SCROLL_MESSAGE_DOWN,
};

enum {
  MAP_TIME_COMPRESS_MORE = 0,
  MAP_TIME_COMPRESS_LESS,
};

// GLOBALS

// the dirty state of the mapscreen interface bottom
BOOLEAN fMapScreenBottomDirty = TRUE;

BOOLEAN fMapBottomDirtied = FALSE;

// Used to flag the transition animation from mapscreen to laptop.
BOOLEAN gfStartMapScreenToLaptopTransition = FALSE;

// leaving map screen
BOOLEAN fLeavingMapScreen = FALSE;

// don't start transition from laptop to tactical stuff
BOOLEAN gfDontStartTransitionFromLaptop = FALSE;

// exiting to laptop?
BOOLEAN fLapTop = FALSE;

BOOLEAN gfOneFramePauseOnExit = FALSE;

// we've just scrolled to a new message (for autoscrolling only)
// BOOLEAN gfNewScrollMessage = FALSE;

// exit states
int8_t gbExitingMapScreenToWhere = -1;

uint8_t gubFirstMapscreenMessageIndex = 0;

uint32_t guiCompressionStringBaseTime = 0;

// graphics
uint32_t guiMAPBOTTOMPANEL;
uint32_t guiSliderBar;

// buttons
uint32_t guiMapMessageScrollButtons[2];
uint32_t guiMapBottomExitButtons[3];
uint32_t guiMapBottomTimeButtons[2];

// buttons images
uint32_t guiMapMessageScrollButtonsImage[2];
uint32_t guiMapBottomExitButtonsImage[3];
uint32_t guiMapBottomTimeButtonsImage[2];

// mouse regions
struct MOUSE_REGION gMapMessageScrollBarRegion;
struct MOUSE_REGION gMapPauseRegion;

struct MOUSE_REGION gTimeCompressionMask[3];

// EXTERNS

extern uint8_t gubStartOfMapScreenMessageList;

extern int32_t giMapInvDoneButton;

extern BOOLEAN fInMapMode;
extern BOOLEAN fShowInventoryFlag;
extern BOOLEAN fShowDescriptionFlag;

extern struct MOUSE_REGION gMPanelRegion;

// PROTOTYPES

void LoadMessageSliderBar(void);
void DeleteMessageSliderBar(void);
void DisplayScrollBarSlider();

void CreateMapScreenBottomMessageScrollBarRegion(void);
void DeleteMapScreenBottomMessageScrollRegion(void);

void DisplayCurrentBalanceForMapBottom(void);
void DisplayCurrentBalanceTitleForMapBottom(void);
void DisplayProjectedDailyMineIncome(void);
void DrawNameOfLoadedSector(void);

void EnableDisableBottomButtonsAndRegions(void);
void EnableDisableTimeCompressButtons(void);
void EnableDisableMessageScrollButtonsAndRegions(void);

void DisplayCompressMode(void);
void RemoveCompressModePause(void);
void CreateCompressModePause(void);

void BtnLaptopCallback(GUI_BUTTON *btn, int32_t reason);
void BtnTacticalCallback(GUI_BUTTON *btn, int32_t reason);
void BtnOptionsFromMapScreenCallback(GUI_BUTTON *btn, int32_t reason);

void CompressModeClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void CompressMaskClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void BtnTimeCompressMoreMapScreenCallback(GUI_BUTTON *btn, int32_t reason);
void BtnTimeCompressLessMapScreenCallback(GUI_BUTTON *btn, int32_t reason);

void BtnMessageDownMapScreenCallback(GUI_BUTTON *btn, int32_t reason);
void BtnMessageUpMapScreenCallback(GUI_BUTTON *btn, int32_t reason);

void MapScreenMessageScrollBarCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// void CheckForAndHandleAutoMessageScroll( void );

// FUNCTIONS

void HandleLoadOfMapBottomGraphics(void) {
  // will load the graphics needed for the mapscreen interface bottom

  // will create buttons for interface bottom
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\map_screen_bottom.sti"), &guiMAPBOTTOMPANEL))
    return;

  // load slider bar icon
  LoadMessageSliderBar();

  return;
}

BOOLEAN LoadMapScreenInterfaceBottom(void) {
  CreateButtonsForMapScreenInterfaceBottom();
  CreateMapScreenBottomMessageScrollBarRegion();

  // create pause region
  CreateCompressModePause();

  return (TRUE);
}

void DeleteMapBottomGraphics(void) {
  DeleteVideoObjectFromIndex(guiMAPBOTTOMPANEL);

  // delete slider bar icon
  DeleteMessageSliderBar();

  return;
}

void DeleteMapScreenInterfaceBottom(void) {
  // will delete graphics loaded for the mapscreen interface bottom

  DestroyButtonsForMapScreenInterfaceBottom();
  DeleteMapScreenBottomMessageScrollRegion();

  // remove comrpess mode pause
  RemoveCompressModePause();

  return;
}

void RenderMapScreenInterfaceBottom(void) {
  // will render the map screen bottom interface
  struct VObject *hHandle;
  char bFilename[32];

  // render whole panel
  if (fMapScreenBottomDirty == TRUE) {
    // get and blt panel
    GetVideoObject(&hHandle, guiMAPBOTTOMPANEL);
    BltVideoObject(vsSaveBuffer, hHandle, 0, MAP_BOTTOM_X, MAP_BOTTOM_Y);

    if (GetSectorFlagStatus(sSelMapX, sSelMapY, (uint8_t)iCurrentMapSectorZ, SF_ALREADY_VISITED) ==
        TRUE) {
      GetMapFileName(sSelMapX, sSelMapY, (uint8_t)iCurrentMapSectorZ, bFilename, TRUE, TRUE);
      LoadRadarScreenBitmap(bFilename);
    } else {
      ClearOutRadarMapImage();
    }

    fInterfacePanelDirty = DIRTYLEVEL2;

    // display title
    DisplayCurrentBalanceTitleForMapBottom();

    // dirty buttons
    MarkButtonsDirty();

    // invalidate region
    RestoreExternBackgroundRect(MAP_BOTTOM_X, MAP_BOTTOM_Y, 640 - MAP_BOTTOM_X, 480 - MAP_BOTTOM_Y);

    // re render radar map
    RenderRadarScreen();

    // reset dirty flag
    fMapScreenBottomDirty = FALSE;
    fMapBottomDirtied = TRUE;
  }

  DisplayCompressMode();

  DisplayCurrentBalanceForMapBottom();
  DisplayProjectedDailyMineIncome();

  // draw the name of the loaded sector
  DrawNameOfLoadedSector();

  // display slider on the scroll bar
  DisplayScrollBarSlider();

  // display messages that can be scrolled through
  DisplayStringsInMapScreenMessageList();

  // handle auto scroll
  // CheckForAndHandleAutoMessageScroll( );

  EnableDisableMessageScrollButtonsAndRegions();

  EnableDisableBottomButtonsAndRegions();

  fMapBottomDirtied = FALSE;
  return;
}

BOOLEAN CreateButtonsForMapScreenInterfaceBottom(void) {
  // laptop
  guiMapBottomExitButtonsImage[MAP_EXIT_TO_LAPTOP] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 6, -1, 15, -1);
  guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP] =
      QuickCreateButton(guiMapBottomExitButtonsImage[MAP_EXIT_TO_LAPTOP], 456, 410, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)BtnLaptopCallback);

  // tactical
  guiMapBottomExitButtonsImage[MAP_EXIT_TO_TACTICAL] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 7, -1, 16, -1);

  guiMapBottomExitButtons[MAP_EXIT_TO_TACTICAL] =
      QuickCreateButton(guiMapBottomExitButtonsImage[MAP_EXIT_TO_TACTICAL], 496, 410, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)BtnTacticalCallback);

  // options
  guiMapBottomExitButtonsImage[MAP_EXIT_TO_OPTIONS] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 18, -1, 19, -1);
  guiMapBottomExitButtons[MAP_EXIT_TO_OPTIONS] =
      QuickCreateButton(guiMapBottomExitButtonsImage[MAP_EXIT_TO_OPTIONS], 458, 372, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)BtnOptionsFromMapScreenCallback);

  SetButtonFastHelpText(guiMapBottomExitButtons[0], pMapScreenBottomFastHelp[0]);
  SetButtonFastHelpText(guiMapBottomExitButtons[1], pMapScreenBottomFastHelp[1]);
  SetButtonFastHelpText(guiMapBottomExitButtons[2], pMapScreenBottomFastHelp[2]);

  SetButtonCursor(guiMapBottomExitButtons[0], MSYS_NO_CURSOR);
  SetButtonCursor(guiMapBottomExitButtons[1], MSYS_NO_CURSOR);
  SetButtonCursor(guiMapBottomExitButtons[2], MSYS_NO_CURSOR);

  // time compression buttons
  guiMapBottomTimeButtonsImage[MAP_TIME_COMPRESS_MORE] =
      LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 10, 1, -1, 3, -1);
  guiMapBottomTimeButtons[MAP_TIME_COMPRESS_MORE] = QuickCreateButton(
      guiMapBottomTimeButtonsImage[MAP_TIME_COMPRESS_MORE], 528, 456, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 2, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnTimeCompressMoreMapScreenCallback);

  guiMapBottomTimeButtonsImage[MAP_TIME_COMPRESS_LESS] =
      LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 9, 0, -1, 2, -1);
  guiMapBottomTimeButtons[MAP_TIME_COMPRESS_LESS] = QuickCreateButton(
      guiMapBottomTimeButtonsImage[MAP_TIME_COMPRESS_LESS], 466, 456, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 2, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnTimeCompressLessMapScreenCallback);

  SetButtonFastHelpText(guiMapBottomTimeButtons[0], pMapScreenBottomFastHelp[3]);
  SetButtonFastHelpText(guiMapBottomTimeButtons[1], pMapScreenBottomFastHelp[4]);

  SetButtonCursor(guiMapBottomTimeButtons[0], MSYS_NO_CURSOR);
  SetButtonCursor(guiMapBottomTimeButtons[1], MSYS_NO_CURSOR);

  // scroll buttons
  guiMapMessageScrollButtonsImage[MAP_SCROLL_MESSAGE_UP] =
      LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 11, 4, -1, 6, -1);
  guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_UP] = QuickCreateButton(
      guiMapMessageScrollButtonsImage[MAP_SCROLL_MESSAGE_UP], 331, 371, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnMessageUpMapScreenCallback);

  guiMapMessageScrollButtonsImage[MAP_SCROLL_MESSAGE_DOWN] =
      LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 12, 5, -1, 7, -1);
  guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_DOWN] = QuickCreateButton(
      guiMapMessageScrollButtonsImage[MAP_SCROLL_MESSAGE_DOWN], 331, 452, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnMessageDownMapScreenCallback);

  SetButtonFastHelpText(guiMapMessageScrollButtons[0], pMapScreenBottomFastHelp[5]);
  SetButtonFastHelpText(guiMapMessageScrollButtons[1], pMapScreenBottomFastHelp[6]);
  SetButtonCursor(guiMapMessageScrollButtons[0], MSYS_NO_CURSOR);
  SetButtonCursor(guiMapMessageScrollButtons[1], MSYS_NO_CURSOR);

  return (TRUE);
}

void DestroyButtonsForMapScreenInterfaceBottom(void) {
  // will destroy the buttons for the mapscreen bottom interface

  RemoveButton(guiMapBottomExitButtons[0]);
  RemoveButton(guiMapBottomExitButtons[1]);
  RemoveButton(guiMapBottomExitButtons[2]);
  RemoveButton(guiMapMessageScrollButtons[0]);
  RemoveButton(guiMapMessageScrollButtons[1]);
  RemoveButton(guiMapBottomTimeButtons[0]);
  RemoveButton(guiMapBottomTimeButtons[1]);

  UnloadButtonImage(guiMapBottomExitButtonsImage[0]);
  UnloadButtonImage(guiMapBottomExitButtonsImage[1]);
  UnloadButtonImage(guiMapBottomExitButtonsImage[2]);
  UnloadButtonImage(guiMapMessageScrollButtonsImage[0]);
  UnloadButtonImage(guiMapMessageScrollButtonsImage[1]);
  UnloadButtonImage(guiMapBottomTimeButtonsImage[0]);
  UnloadButtonImage(guiMapBottomTimeButtonsImage[1]);

  // reset dirty flag
  fMapScreenBottomDirty = TRUE;

  return;
}

void BtnLaptopCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }

    // redraw region
    if (btn->Area.uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }

    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON | BUTTON_DIRTY);
      DrawButton(btn->IDNum);

      RequestTriggerExitFromMapscreen(MAP_EXIT_TO_LAPTOP);
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }
  }
}

void BtnTacticalCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    // redraw region
    if (btn->Area.uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }
  }
}

void BtnOptionsFromMapScreenCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    // redraw region
    if (btn->uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fMapScreenBottomDirty = TRUE;

      RequestTriggerExitFromMapscreen(MAP_EXIT_TO_OPTIONS);
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }
  }
}

void DrawNameOfLoadedSector(void) {
  wchar_t sString[128];
  int16_t sFontX, sFontY;

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  SetFont(COMPFONT);
  SetFontForeground(183);
  SetFontBackground(FONT_BLACK);

  GetSectorIDString(sSelMapX, sSelMapY, (int8_t)(iCurrentMapSectorZ), sString, ARR_SIZE(sString),
                    TRUE);
  ReduceStringLength(sString, ARR_SIZE(sString), 80, COMPFONT);

  VarFindFontCenterCoordinates(548, 426, 80, 16, COMPFONT, &sFontX, &sFontY, sString);
  mprintf(sFontX, sFontY, L"%s", sString);
}

void CompressModeClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & (MSYS_CALLBACK_REASON_RBUTTON_UP | MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    if (CommonTimeCompressionChecks() == TRUE) return;

    RequestToggleTimeCompression();
  }
}

void BtnTimeCompressMoreMapScreenCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (CommonTimeCompressionChecks() == TRUE) return;

    // redraw region
    if (btn->uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fMapScreenBottomDirty = TRUE;

      RequestIncreaseInTimeCompression();
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (CommonTimeCompressionChecks() == TRUE) return;
  }
}

void BtnTimeCompressLessMapScreenCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (CommonTimeCompressionChecks() == TRUE) return;

    // redraw region
    if (btn->uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fMapScreenBottomDirty = TRUE;

      RequestDecreaseInTimeCompression();
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (CommonTimeCompressionChecks() == TRUE) return;
  }
}

void BtnMessageDownMapScreenCallback(GUI_BUTTON *btn, int32_t reason) {
  static int32_t iLastRepeatScrollTime = 0;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    // redraw region
    if (btn->uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);

    iLastRepeatScrollTime = 0;
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // redraw region
      if (btn->uiFlags & MSYS_HAS_BACKRECT) {
        fMapScreenBottomDirty = TRUE;
      }

      // down a line
      MapScreenMsgScrollDown(1);
    }
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    if (GetJA2Clock() - iLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME) {
      // down a line
      MapScreenMsgScrollDown(1);

      iLastRepeatScrollTime = GetJA2Clock();
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    // redraw region
    if (btn->uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);

    iLastRepeatScrollTime = 0;
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // redraw region
      if (btn->uiFlags & MSYS_HAS_BACKRECT) {
        fMapScreenBottomDirty = TRUE;
      }

      // down a page
      MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM);
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_REPEAT) {
    if (GetJA2Clock() - iLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME) {
      // down a page
      MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM);

      iLastRepeatScrollTime = GetJA2Clock();
    }
  }
}

void BtnMessageUpMapScreenCallback(GUI_BUTTON *btn, int32_t reason) {
  static int32_t iLastRepeatScrollTime = 0;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);

    // redraw region
    if (btn->Area.uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    iLastRepeatScrollTime = 0;
  }

  else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // redraw region
      if (btn->uiFlags & MSYS_HAS_BACKRECT) {
        fMapScreenBottomDirty = TRUE;
      }

      // up a line
      MapScreenMsgScrollUp(1);
    }
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    if (GetJA2Clock() - iLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME) {
      // up a line
      MapScreenMsgScrollUp(1);

      iLastRepeatScrollTime = GetJA2Clock();
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    // redraw region
    if (btn->uiFlags & MSYS_HAS_BACKRECT) {
      fMapScreenBottomDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);

    iLastRepeatScrollTime = 0;
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // redraw region
      if (btn->uiFlags & MSYS_HAS_BACKRECT) {
        fMapScreenBottomDirty = TRUE;
      }

      // up a page
      MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_REPEAT) {
    if (GetJA2Clock() - iLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME) {
      // up a page
      MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);

      iLastRepeatScrollTime = GetJA2Clock();
    }
  }
}

void EnableDisableMessageScrollButtonsAndRegions(void) {
  uint8_t ubNumMessages;

  ubNumMessages = GetRangeOfMapScreenMessages();

  // if no scrolling required, or already showing the topmost message
  if ((ubNumMessages <= MAX_MESSAGES_ON_MAP_BOTTOM) || (gubFirstMapscreenMessageIndex == 0)) {
    DisableButton(guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_UP]);
    ButtonList[guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_UP]]->uiFlags &= ~(BUTTON_CLICKED_ON);
  } else {
    EnableButton(guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_UP]);
  }

  // if no scrolling required, or already showing the last message
  if ((ubNumMessages <= MAX_MESSAGES_ON_MAP_BOTTOM) ||
      ((gubFirstMapscreenMessageIndex + MAX_MESSAGES_ON_MAP_BOTTOM) >= ubNumMessages)) {
    DisableButton(guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_DOWN]);
    ButtonList[guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_DOWN]]->uiFlags &=
        ~(BUTTON_CLICKED_ON);
  } else {
    EnableButton(guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_DOWN]);
  }

  if (ubNumMessages <= MAX_MESSAGES_ON_MAP_BOTTOM) {
    MSYS_DisableRegion(&gMapMessageScrollBarRegion);
  } else {
    MSYS_EnableRegion(&gMapMessageScrollBarRegion);
  }
}

void DisplayCompressMode(void) {
  int16_t sX, sY;
  wchar_t sString[128];
  static uint8_t usColor = FONT_LTGREEN;

  // get compress speed
  if (giTimeCompressMode != NOT_USING_TIME_COMPRESSION) {
    if (IsTimeBeingCompressed()) {
      swprintf(sString, ARR_SIZE(sString), L"%s", sTimeStrings[giTimeCompressMode]);
    } else {
      swprintf(sString, ARR_SIZE(sString), L"%s", sTimeStrings[0]);
    }
  }

  RestoreExternBackgroundRect(489, 456, 522 - 489, 467 - 454);
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
  SetFont(COMPFONT);

  if (GetJA2Clock() - guiCompressionStringBaseTime >= PAUSE_GAME_TIMER) {
    if (usColor == FONT_LTGREEN) {
      usColor = FONT_WHITE;
    } else {
      usColor = FONT_LTGREEN;
    }

    guiCompressionStringBaseTime = GetJA2Clock();
  }

  if ((giTimeCompressMode != 0) && (GamePaused() == FALSE)) {
    usColor = FONT_LTGREEN;
  }

  SetFontForeground(usColor);
  SetFontBackground(FONT_BLACK);
  FindFontCenterCoordinates(489, 456, 522 - 489, 467 - 454, sString, COMPFONT, &sX, &sY);
  mprintf(sX, sY, sString);

  return;
}

void CreateCompressModePause(void) {
  MSYS_DefineRegion(&gMapPauseRegion, 487, 456, 522, 467, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR,
                    MSYS_NO_CALLBACK, CompressModeClickCallback);

  SetRegionFastHelpText(&gMapPauseRegion, pMapScreenBottomFastHelp[7]);
}

void RemoveCompressModePause(void) { MSYS_RemoveRegion(&gMapPauseRegion); }

void LoadMessageSliderBar(void) {
  // this function will load the message slider bar

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\map_screen_bottom_arrows.sti"), &guiSliderBar))
    return;
}

void DeleteMessageSliderBar(void) {
  // this function will delete message slider bar
  DeleteVideoObjectFromIndex(guiSliderBar);
}

void CreateMapScreenBottomMessageScrollBarRegion(void) {
  MSYS_DefineRegion(&gMapMessageScrollBarRegion, MESSAGE_SCROLL_AREA_START_X,
                    MESSAGE_SCROLL_AREA_START_Y, MESSAGE_SCROLL_AREA_END_X,
                    MESSAGE_SCROLL_AREA_END_Y, MSYS_PRIORITY_NORMAL, MSYS_NO_CURSOR,
                    MSYS_NO_CALLBACK, MapScreenMessageScrollBarCallBack);
}

void DeleteMapScreenBottomMessageScrollRegion(void) {
  MSYS_RemoveRegion(&gMapMessageScrollBarRegion);
}

void MapScreenMessageScrollBarCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  uint8_t ubMouseYOffset;
  uint8_t ubDesiredSliderOffset;
  uint8_t ubDesiredMessageIndex;
  uint8_t ubNumMessages;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }

  if (iReason & (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_LBUTTON_REPEAT)) {
    // how many messages are there?
    ubNumMessages = GetRangeOfMapScreenMessages();

    // region is supposed to be disabled if there aren't enough messages to scroll.  Formulas assume
    // this
    if (ubNumMessages > MAX_MESSAGES_ON_MAP_BOTTOM) {
      // where is the mouse?
      struct Point MousePos = GetMousePoint();

      ubMouseYOffset = (int8_t)(MousePos.y - MESSAGE_SCROLL_AREA_START_Y);

      // if clicking in the top 5 pixels of the slider bar
      if (ubMouseYOffset < (SLIDER_HEIGHT / 2)) {
        // scroll all the way to the top
        ubDesiredMessageIndex = 0;
      }
      // if clicking in the bottom 6 pixels of the slider bar
      else if (ubMouseYOffset >= (MESSAGE_SCROLL_AREA_HEIGHT - (SLIDER_HEIGHT / 2))) {
        // scroll all the way to the bottom
        ubDesiredMessageIndex = ubNumMessages - MAX_MESSAGES_ON_MAP_BOTTOM;
      } else {
        // somewhere in between
        ubDesiredSliderOffset = ubMouseYOffset - (SLIDER_HEIGHT / 2);

        Assert(ubDesiredSliderOffset <= SLIDER_BAR_RANGE);

        // calculate what the index should be to place the slider at this offset (round fractions of
        // .5+ up)
        ubDesiredMessageIndex =
            ((ubDesiredSliderOffset * (ubNumMessages - MAX_MESSAGES_ON_MAP_BOTTOM)) +
             (SLIDER_BAR_RANGE / 2)) /
            SLIDER_BAR_RANGE;
      }

      // if it's a change
      if (ubDesiredMessageIndex != gubFirstMapscreenMessageIndex) {
        ChangeCurrentMapscreenMessageIndex(ubDesiredMessageIndex);
      }
    }
  }
}

void DisplayScrollBarSlider() {
  // will display the scroll bar icon
  uint8_t ubNumMessages;
  uint8_t ubSliderOffset;
  struct VObject *hHandle;

  ubNumMessages = GetRangeOfMapScreenMessages();

  // only show the slider if there are more messages than will fit on screen
  if (ubNumMessages > MAX_MESSAGES_ON_MAP_BOTTOM) {
    // calculate where slider should be positioned
    ubSliderOffset = (SLIDER_BAR_RANGE * gubFirstMapscreenMessageIndex) /
                     (ubNumMessages - MAX_MESSAGES_ON_MAP_BOTTOM);

    GetVideoObject(&hHandle, guiSliderBar);
    BltVideoObject(vsFB, hHandle, 8, MESSAGE_SCROLL_AREA_START_X + 2,
                   MESSAGE_SCROLL_AREA_START_Y + ubSliderOffset);
  }
}

/*
void CheckForAndHandleAutoMessageScroll( void )
{
        // will check if we are not at the most recent message, if not, scroll to it
        static int32_t iBaseScrollTime =0;
        static int32_t iBaseScrollDelay =0;
        static BOOLEAN fScrollMessage = FALSE;

        // check if we are at the last message, if so, leave
        if( IsThisTheLastMessageInTheList( ) )
        {
                // leave
                // reset flag
                fScrollMessage = FALSE;
                return;
        }

        // we are not, check how long we have been here?
        if( gfNewScrollMessage == TRUE )
        {
                // we just scrolled to a new message, reset timer
                iBaseScrollTime = GetJA2Clock( );

                // reset flag
                gfNewScrollMessage = FALSE;
                fScrollMessage = FALSE;
        }

        // check timer

        if( GetJA2Clock( ) - iBaseScrollTime > DELAY_TO_START_MESSAGE_SCROLL )
        {

                if( fScrollMessage == FALSE )
                {
                  // set up scroll delay
                 iBaseScrollDelay = GetJA2Clock( );

                 // start scroll
                fScrollMessage = TRUE;

                }

                iBaseScrollTime = GetJA2Clock( );
        }

        if( fScrollMessage == TRUE )
        {
                if( GetJA2Clock( ) - iBaseScrollDelay > DELAY_PER_MESSAGE_SCROLL )
                {
                        // scroll to next message
                        MoveCurrentMessagePointerDownList( );

                        // dirty region
                        fMapScreenBottomDirty = TRUE;

                        // reset delay timer
                        iBaseScrollDelay = GetJA2Clock( );
                }
        }


        return;
}
*/

void EnableDisableBottomButtonsAndRegions(void) {
  int8_t iExitButtonIndex;

  // this enables and disables the buttons MAP_EXIT_TO_LAPTOP, MAP_EXIT_TO_TACTICAL, and
  // MAP_EXIT_TO_OPTIONS
  for (iExitButtonIndex = 0; iExitButtonIndex < 3; iExitButtonIndex++) {
    if (AllowedToExitFromMapscreenTo(iExitButtonIndex)) {
      EnableButton(guiMapBottomExitButtons[iExitButtonIndex]);
    } else {
      DisableButton(guiMapBottomExitButtons[iExitButtonIndex]);
    }
  }

  // enable/disable time compress buttons and region masks
  EnableDisableTimeCompressButtons();
  CreateDestroyMouseRegionMasksForTimeCompressionButtons();

  // Enable/Disable map inventory panel buttons

  // if in merc inventory panel
  if (fShowInventoryFlag) {
    // and an item is in the cursor
    if ((gMPanelRegion.Cursor == EXTERN_CURSOR) || (InKeyRingPopup() == TRUE) ||
        InItemStackPopup()) {
      DisableButton(giMapInvDoneButton);
    } else {
      EnableButton(giMapInvDoneButton);
    }

    if (fShowDescriptionFlag) {
      ForceButtonUnDirty(giMapInvDoneButton);
    }
  }
}

void EnableDisableTimeCompressButtons(void) {
  if (AllowedToTimeCompress() == FALSE) {
    DisableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_MORE]);
    DisableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_LESS]);
  } else {
    // disable LESS if time compression is at minimum or OFF
    if (!IsTimeCompressionOn() || giTimeCompressMode == TIME_COMPRESS_X0) {
      DisableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_LESS]);
    } else {
      EnableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_LESS]);
    }

    // disable MORE if we're not paused and time compression is at maximum
    // only disable MORE if we're not paused and time compression is at maximum
    if (IsTimeCompressionOn() && (giTimeCompressMode == TIME_COMPRESS_60MINS)) {
      DisableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_MORE]);
    } else {
      EnableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_MORE]);
    }
  }
}

void EnableDisAbleMapScreenOptionsButton(BOOLEAN fEnable) {
  if (fEnable) {
    EnableButton(guiMapBottomExitButtons[MAP_EXIT_TO_OPTIONS]);
  } else {
    DisableButton(guiMapBottomExitButtons[MAP_EXIT_TO_OPTIONS]);
  }
}

BOOLEAN AllowedToTimeCompress(void) {
  // if already leaving, disallow any other attempts to exit
  if (fLeavingMapScreen) {
    return (FALSE);
  }

  // if already going someplace
  if (gbExitingMapScreenToWhere != -1) {
    return (FALSE);
  }

  // if we're locked into paused time compression by some event that enforces that
  if (PauseStateLocked()) {
    return (FALSE);
  }

  // meanwhile coming up
  if (gfMeanwhileTryingToStart) {
    return (FALSE);
  }

  // someone has something to say
  if (!DialogueQueueIsEmpty()) {
    return (FALSE);
  }

  // moving / confirming movement
  if ((bSelectedDestChar != -1) || fPlotForHelicopter || gfInConfirmMapMoveMode ||
      fShowMapScreenMovementList) {
    return (FALSE);
  }

  if (fShowAssignmentMenu || fShowTrainingMenu || fShowAttributeMenu || fShowSquadMenu ||
      fShowContractMenu || fShowRemoveMenu) {
    return (FALSE);
  }

  if (fShowUpdateBox || fShowTownInfo || (sSelectedMilitiaTown != 0)) {
    return (FALSE);
  }

  // renewing contracts
  if (gfContractRenewalSquenceOn) {
    return (FALSE);
  }

  // disabled due to battle?
  if ((fDisableMapInterfaceDueToBattle) || (fDisableDueToBattleRoster)) {
    return (FALSE);
  }

  // if holding an inventory item
  if (fMapInventoryItem) {
    return (FALSE);
  }

  // show the inventory pool?
  if (fShowMapInventoryPool) {
    // prevent time compress (items get stolen over time, etc.)
    return (FALSE);
  }

  // no mercs have ever been hired
  if (gfAtLeastOneMercWasHired == FALSE) {
    return (FALSE);
  }

  /*
          //in air raid
          if( InAirRaid( ) == TRUE )
          {
                  return( FALSE );
          }
  */

  // no usable mercs on team!
  if (!AnyUsableRealMercenariesOnTeam()) {
    return (FALSE);
  }

  // must wait till bombs go off
  if (ActiveTimedBombExists()) {
    return (FALSE);
  }

  // hostile sector / in battle
  if ((gTacticalStatus.uiFlags & INCOMBAT) || (gTacticalStatus.fEnemyInSector)) {
    return (FALSE);
  }

  if (PlayerGroupIsInACreatureInfestedMine()) {
    return FALSE;
  }

  return (TRUE);
}

void DisplayCurrentBalanceTitleForMapBottom(void) {
  wchar_t sString[128];
  int16_t sFontX, sFontY;

  // ste the font buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  SetFont(COMPFONT);
  SetFontForeground(MAP_BOTTOM_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), L"%s", pMapScreenBottomText[0]);

  // center it
  VarFindFontCenterCoordinates(359, 387 - 14, 437 - 359, 10, COMPFONT, &sFontX, &sFontY, sString);

  // print it
  mprintf(sFontX, sFontY, L"%s", sString);

  swprintf(sString, ARR_SIZE(sString), L"%s", zMarksMapScreenText[2]);

  // center it
  VarFindFontCenterCoordinates(359, 433 - 14, 437 - 359, 10, COMPFONT, &sFontX, &sFontY, sString);

  // print it
  mprintf(sFontX, sFontY, L"%s", sString);

  // ste the font buffer
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
  return;
}

void DisplayCurrentBalanceForMapBottom(void) {
  // show the current balance for the player on the map panel bottom
  wchar_t sString[128];
  int16_t sFontX, sFontY;

  // ste the font buffer
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  // set up the font
  SetFont(COMPFONT);
  SetFontForeground(183);
  SetFontBackground(FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), L"%d", MoneyGetBalance());

  // insert

  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);

  // center it
  VarFindFontCenterCoordinates(359, 387 + 2, 437 - 359, 10, COMPFONT, &sFontX, &sFontY, sString);

  // print it
  mprintf(sFontX, sFontY, L"%s", sString);

  return;
}

void CreateDestroyMouseRegionMasksForTimeCompressionButtons(void) {
  BOOLEAN fDisabled = FALSE;
  static BOOLEAN fCreated = FALSE;

  // allowed to time compress?
  if (AllowedToTimeCompress() == FALSE) {
    // no, disable buttons
    fDisabled = TRUE;
  }

  if (fInMapMode == FALSE) {
    fDisabled = FALSE;
  }

  // check if disabled and not created, create
  if ((fDisabled) && (fCreated == FALSE)) {
    // mask over compress more button
    MSYS_DefineRegion(&gTimeCompressionMask[0], 528, 456, 528 + 13, 456 + 14,
                      MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK,
                      CompressMaskClickCallback);

    // mask over compress less button
    MSYS_DefineRegion(&gTimeCompressionMask[1], 466, 456, 466 + 13, 456 + 14,
                      MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK,
                      CompressMaskClickCallback);

    // mask over pause game button
    MSYS_DefineRegion(&gTimeCompressionMask[2], 487, 456, 522, 467, MSYS_PRIORITY_HIGHEST - 1,
                      MSYS_NO_CURSOR, MSYS_NO_CALLBACK, CompressMaskClickCallback);

    fCreated = TRUE;
  } else if ((fDisabled == FALSE) && (fCreated)) {
    // created and no longer need to disable
    MSYS_RemoveRegion(&gTimeCompressionMask[0]);
    MSYS_RemoveRegion(&gTimeCompressionMask[1]);
    MSYS_RemoveRegion(&gTimeCompressionMask[2]);
    fCreated = FALSE;
  }
}

void CompressMaskClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    TellPlayerWhyHeCantCompressTime();
  }
}

void DisplayProjectedDailyMineIncome(void) {
  int32_t iRate = 0;
  static int32_t iOldRate = -1;
  wchar_t sString[128];
  int16_t sFontX, sFontY;

  // grab the rate from the financial system
  iRate = GetProjectedTotalDailyIncome();

  if (iRate != iOldRate) {
    iOldRate = iRate;
    fMapScreenBottomDirty = TRUE;

    // if screen was not dirtied, leave
    if (fMapBottomDirtied == FALSE) {
      return;
    }
  }
  // ste the font buffer
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  // set up the font
  SetFont(COMPFONT);
  SetFontForeground(183);
  SetFontBackground(FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), L"%d", iRate);

  // insert
  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);

  // center it
  VarFindFontCenterCoordinates(359, 433 + 2, 437 - 359, 10, COMPFONT, &sFontX, &sFontY, sString);

  // print it
  mprintf(sFontX, sFontY, L"%s", sString);

  return;
}

BOOLEAN CommonTimeCompressionChecks(void) {
  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();
    return (TRUE);
  }

  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    // abort plotting movement
    AbortMovementPlottingMode();
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN AnyUsableRealMercenariesOnTeam(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iCounter = 0, iNumberOnTeam = 0;

  // this is for speed, this runs once/frame
  iNumberOnTeam = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // get number of mercs on team who are not vehicles or robot, POWs or EPCs
  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pSoldier = GetSoldierByID(iCounter);

    if ((IsSolActive(pSoldier)) && IsSolAlive(pSoldier) &&
        !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(pSoldier) &&
        (pSoldier->bAssignment != ASSIGNMENT_POW) && (pSoldier->bAssignment != ASSIGNMENT_DEAD) &&
        (pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__EPC)) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void RequestTriggerExitFromMapscreen(int8_t bExitToWhere) {
  Assert((bExitToWhere >= MAP_EXIT_TO_LAPTOP) && (bExitToWhere <= MAP_EXIT_TO_SAVE));

  // if allowed to do so
  if (AllowedToExitFromMapscreenTo(bExitToWhere)) {
    // if the screen to exit to is the SAVE screen
    if (bExitToWhere == MAP_EXIT_TO_SAVE) {
      // if the game CAN NOT be saved
      if (!CanGameBeSaved()) {
        // Display a message saying the player cant save now
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW],
                        MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
        return;
      }
    }

    // permit it, and get the ball rolling
    gbExitingMapScreenToWhere = bExitToWhere;

    // delay until mapscreen has had a chance to render at least one full frame
    gfOneFramePauseOnExit = TRUE;
  }
}

BOOLEAN AllowedToExitFromMapscreenTo(int8_t bExitToWhere) {
  Assert((bExitToWhere >= MAP_EXIT_TO_LAPTOP) && (bExitToWhere <= MAP_EXIT_TO_SAVE));

  // if already leaving, disallow any other attempts to exit
  if (fLeavingMapScreen) {
    return (FALSE);
  }

  // if already going someplace else
  if ((gbExitingMapScreenToWhere != -1) && (gbExitingMapScreenToWhere != bExitToWhere)) {
    return (FALSE);
  }

  // someone has something to say
  if (!DialogueQueueIsEmpty()) {
    return (FALSE);
  }

  // meanwhile coming up
  if (gfMeanwhileTryingToStart) {
    return (FALSE);
  }

  // if we're locked into paused time compression by some event that enforces that
  if (PauseStateLocked()) {
    return (FALSE);
  }

  // if holding an inventory item
  if (fMapInventoryItem || (gMPanelRegion.Cursor == EXTERN_CURSOR)) {
    return (FALSE);
  }

  if (fShowUpdateBox || fShowTownInfo || (sSelectedMilitiaTown != 0)) {
    return (FALSE);
  }

  // renewing contracts
  if (gfContractRenewalSquenceOn) {
    return (FALSE);
  }

  // battle about to occur?
  if ((fDisableDueToBattleRoster) || (fDisableMapInterfaceDueToBattle)) {
    return (FALSE);
  }

  /*
          // air raid starting
          if( gubAirRaidMode == AIR_RAID_START )
          {
                  // nope
                  return( FALSE );
          }
  */

  // the following tests apply to going tactical screen only
  if (bExitToWhere == MAP_EXIT_TO_TACTICAL) {
    // if in battle or air raid, the ONLY sector we can go tactical in is the one that's loaded
    if (((gTacticalStatus.uiFlags & INCOMBAT) ||
         (gTacticalStatus.fEnemyInSector) /*|| InAirRaid( )*/) &&
        ((sSelMapX != gWorldSectorX) || (sSelMapY != gWorldSectorY) ||
         ((uint8_t)iCurrentMapSectorZ) != gbWorldSectorZ)) {
      return (FALSE);
    }

    // must have some mercs there
    if (!CanGoToTacticalInSector(sSelMapX, sSelMapY, (uint8_t)iCurrentMapSectorZ)) {
      return (FALSE);
    }
  }

  // if we are map screen sector inventory
  if (fShowMapInventoryPool) {
    // dont allow it
    return (FALSE);
  }

  // OK to go there, passed all the checks
  return (TRUE);
}

void HandleExitsFromMapScreen(void) {
  // if going somewhere
  if (gbExitingMapScreenToWhere != -1) {
    // delay all exits by one frame...
    if (gfOneFramePauseOnExit == TRUE) {
      gfOneFramePauseOnExit = FALSE;
      return;
    }

    // make sure it's still legal to do this!
    if (AllowedToExitFromMapscreenTo(gbExitingMapScreenToWhere)) {
      // see where we're trying to go
      switch (gbExitingMapScreenToWhere) {
        case MAP_EXIT_TO_LAPTOP:
          fLapTop = TRUE;
          SetPendingNewScreen(LAPTOP_SCREEN);

          if (gfExtraBuffer) {  // Then initiate the transition animation from the mapscreen to
                                // laptop...
            BlitBufferToBuffer(vsFB, vsExtraBuffer, 0, 0, 640, 480);
            gfStartMapScreenToLaptopTransition = TRUE;
          }
          break;

        case MAP_EXIT_TO_TACTICAL:
          SetCurrentWorldSector(sSelMapX, sSelMapY, (uint8_t)iCurrentMapSectorZ);
          break;

        case MAP_EXIT_TO_OPTIONS:
          guiPreviousOptionScreen = guiCurrentScreen;
          SetPendingNewScreen(OPTIONS_SCREEN);
          break;

        case MAP_EXIT_TO_SAVE:
        case MAP_EXIT_TO_LOAD:
          gfCameDirectlyFromGame = TRUE;
          guiPreviousOptionScreen = guiCurrentScreen;
          SetPendingNewScreen(SAVE_LOAD_SCREEN);
          break;

        default:
          // invalid exit type
          Assert(FALSE);
      }

      // time compression during mapscreen exit doesn't seem to cause any problems, but turn it off
      // as early as we can
      StopTimeCompression();

      // now leaving mapscreen
      fLeavingMapScreen = TRUE;
    }

    // cancel exit, either we're on our way, or we're not allowed to go
    gbExitingMapScreenToWhere = -1;
  }
}

void MapScreenMsgScrollDown(uint8_t ubLinesDown) {
  uint8_t ubNumMessages;

  ubNumMessages = GetRangeOfMapScreenMessages();

  // check if we can go that far, only go as far as we can
  if ((gubFirstMapscreenMessageIndex + MAX_MESSAGES_ON_MAP_BOTTOM + ubLinesDown) > ubNumMessages) {
    ubLinesDown = ubNumMessages - gubFirstMapscreenMessageIndex -
                  min(ubNumMessages, MAX_MESSAGES_ON_MAP_BOTTOM);
  }

  if (ubLinesDown > 0) {
    ChangeCurrentMapscreenMessageIndex((uint8_t)(gubFirstMapscreenMessageIndex + ubLinesDown));
  }
}

void MapScreenMsgScrollUp(uint8_t ubLinesUp) {
  // check if we can go that far, only go as far as we can
  if (gubFirstMapscreenMessageIndex < ubLinesUp) {
    ubLinesUp = gubFirstMapscreenMessageIndex;
  }

  if (ubLinesUp > 0) {
    ChangeCurrentMapscreenMessageIndex((uint8_t)(gubFirstMapscreenMessageIndex - ubLinesUp));
  }
}

void MoveToEndOfMapScreenMessageList(void) {
  uint8_t ubDesiredMessageIndex;
  uint8_t ubNumMessages;

  ubNumMessages = GetRangeOfMapScreenMessages();

  ubDesiredMessageIndex = ubNumMessages - min(ubNumMessages, MAX_MESSAGES_ON_MAP_BOTTOM);
  ChangeCurrentMapscreenMessageIndex(ubDesiredMessageIndex);
}

void ChangeCurrentMapscreenMessageIndex(uint8_t ubNewMessageIndex) {
  Assert(ubNewMessageIndex + MAX_MESSAGES_ON_MAP_BOTTOM <=
         max(MAX_MESSAGES_ON_MAP_BOTTOM, GetRangeOfMapScreenMessages()));

  gubFirstMapscreenMessageIndex = ubNewMessageIndex;
  gubCurrentMapMessageString =
      (gubStartOfMapScreenMessageList + gubFirstMapscreenMessageIndex) % 256;

  // set fact we just went to a new message
  //	gfNewScrollMessage = TRUE;

  // refresh screen
  fMapScreenBottomDirty = TRUE;
}
