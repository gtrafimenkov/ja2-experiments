// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameLoop.h"

#include <stdio.h>

#include "FadeScreen.h"
#include "GameSettings.h"
#include "Globals.h"
#include "HelpScreen.h"
#include "Init.h"
#include "JAScreens.h"
#include "Laptop/Finances.h"
#include "Laptop/Laptop.h"
#include "OptionsScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/Types.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "Screens.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/PreBattleInterface.h"
#include "SysGlobals.h"
#include "Tactical/Interface.h"
#include "Tactical/Overhead.h"
#include "Tactical/TacticalSave.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "platform.h"

uint32_t guiPendingScreen = NO_PENDING_SCREEN;
uint32_t guiPreviousScreen = NO_PENDING_SCREEN;

#define DONT_CHECK_FOR_FREE_SPACE 255
uint8_t gubCheckForFreeSpaceOnHardDriveCount = DONT_CHECK_FOR_FREE_SPACE;

extern BOOLEAN DoSkiMessageBox(uint8_t ubStyle, wchar_t* zString, uint32_t uiExitScreen,
                               uint8_t ubFlags, MSGBOX_CALLBACK ReturnCallback);
extern void NotEnoughHardDriveSpaceForQuickSaveMessageBoxCallBack(uint8_t bExitValue);
extern BOOLEAN gfTacticalPlacementGUIActive;
extern BOOLEAN gfTacticalPlacementGUIDirty;
extern BOOLEAN gfValidLocationsChanged;
extern BOOLEAN gfInMsgBox;

// callback to confirm game is over
void EndGameMessageBoxCallBack(uint8_t bExitValue);
void HandleNewScreenChange(uint32_t uiNewScreen, uint32_t uiOldScreen);

// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game

#ifdef JA2BETAVERSION
BOOLEAN gubReportMapscreenLock = 0;
void ReportMapscreenErrorLock() {
  switch (gubReportMapscreenLock) {
    case 1:
      DoScreenIndependantMessageBox(
          L"You have just loaded the game which is in a state that you shouldn't be able to.  You "
          L"can still play, but there should be a sector with enemies co-existing with mercs.  "
          L"Please don't report that.",
          MSG_BOX_FLAG_OK, NULL);
      fDisableDueToBattleRoster = FALSE;
      fDisableMapInterfaceDueToBattle = FALSE;
      gubReportMapscreenLock = 0;
      break;
    case 2:
      DoScreenIndependantMessageBox(
          L"You have just saved the game which is in a state that you shouldn't be able to.  "
          L"Please report circumstances (ex:  merc in other sector pipes up about enemies), etc.  "
          L"Autocorrected, but if you reload the save, don't report the error appearing in load.",
          MSG_BOX_FLAG_OK, NULL);
      fDisableDueToBattleRoster = FALSE;
      fDisableMapInterfaceDueToBattle = FALSE;
      gubReportMapscreenLock = 0;
      break;
  }
}
#endif

BOOLEAN InitializeGame(void) {
  uint32_t uiIndex;

  ClearAllDebugTopics();
  RegisterJA2DebugTopic(TOPIC_JA2OPPLIST, "Reg");
  // RegisterJA2DebugTopic( TOPIC_MEMORY_MANAGER, "Reg" );

  // Initlaize mouse subsystems
  MSYS_Init();
  InitButtonSystem();
  InitCursors();

  // Init Fonts
  if (!InitializeFonts()) {
    // Send debug message and quit
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "COULD NOT INUT FONT SYSTEM...");
    return (ERROR_SCREEN);
  }

  // Deletes all the Temp files in the Maps\Temp directory
  InitTacticalSave(TRUE);

  // preload mapscreen graphics
  HandlePreloadOfMapGraphics();

  // Initialize Game Screens.
  for (uiIndex = 0; uiIndex < MAX_SCREENS; uiIndex++) {
    if ((*(GameScreens[uiIndex].InitializeScreen))() ==
        FALSE) {  // Failed to initialize one of the screens.
      return FALSE;
    }
  }

  // Init the help screen system
  InitHelpScreenSystem();

  // Loads the saved (if any) general JA2 game settings
  LoadGameSettings();

  // Initialize the Game options ( Gun nut, scifi and dif. levels
  InitGameOptions();

  guiCurrentScreen = INIT_SCREEN;

  return TRUE;
}

// The ShutdownGame function will free up/undo all things that were started in InitializeGame()
// It will also be responsible to making sure that all Gaming Engine tasks exit properly

void ShutdownGame(void) {
  // handle shutdown of game with respect to preloaded mapscreen graphics
  HandleRemovalOfPreLoadedMapGraphics();

  ShutdownJA2();

  // Save the general save game settings to disk
  SaveGameSettings();

  // shutdown the file database manager
  ShutDownFileDatabase();

  // Deletes all the Temp files in the Maps\Temp directory
  InitTacticalSave(FALSE);
}

// This is the main Gameloop. This should eventually by one big switch statement which represents
// the state of the game (i.e. Main Menu, PC Generation, Combat loop, etc....)
// This function exits constantly and reenters constantly

void GameLoop(void) {
  InputAtom InputEvent;
  uint32_t uiOldScreen = guiCurrentScreen;

  struct Point MousePos = GetMousePoint();
  // Hook into mouse stuff for MOVEMENT MESSAGES
  MouseSystemHook(MOUSE_POS, (uint16_t)MousePos.x, (uint16_t)MousePos.y, _LeftButtonDown,
                  _RightButtonDown);
  MusicPoll(FALSE);

  while (DequeueSpecificEvent(&InputEvent, LEFT_BUTTON_REPEAT | RIGHT_BUTTON_REPEAT |
                                               LEFT_BUTTON_DOWN | LEFT_BUTTON_UP |
                                               RIGHT_BUTTON_DOWN | RIGHT_BUTTON_UP) == TRUE) {
    // HOOK INTO MOUSE HOOKS
    switch (InputEvent.usEvent) {
      case LEFT_BUTTON_DOWN:
        MouseSystemHook(LEFT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case LEFT_BUTTON_UP:
        MouseSystemHook(LEFT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case RIGHT_BUTTON_DOWN:
        MouseSystemHook(RIGHT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case RIGHT_BUTTON_UP:
        MouseSystemHook(RIGHT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case LEFT_BUTTON_REPEAT:
        MouseSystemHook(LEFT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case RIGHT_BUTTON_REPEAT:
        MouseSystemHook(RIGHT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
    }
  }

  if (gfGlobalError) {
    guiCurrentScreen = ERROR_SCREEN;
  }

  // if we are to check for free space on the hard drive
  if (gubCheckForFreeSpaceOnHardDriveCount < DONT_CHECK_FOR_FREE_SPACE) {
    // only if we are in a screen that can get this check
    if (IsMapScreen_2() || IsTacticalMode() || guiCurrentScreen == SAVE_LOAD_SCREEN) {
      if (gubCheckForFreeSpaceOnHardDriveCount < 1) {
        gubCheckForFreeSpaceOnHardDriveCount++;
      } else {
        // Make sure the user has enough hard drive space
        if (!DoesUserHaveEnoughHardDriveSpace()) {
          wchar_t zText[512];
          wchar_t zSpaceOnDrive[512];
          uint32_t uiSpaceOnDrive;
          wchar_t zSizeNeeded[512];

          swprintf(zSizeNeeded, ARR_SIZE(zSizeNeeded), L"%d",
                   REQUIRED_FREE_SPACE / BYTESINMEGABYTE);
          InsertCommasForDollarFigure(zSizeNeeded);

          uiSpaceOnDrive = Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom();

          swprintf(zSpaceOnDrive, ARR_SIZE(zSpaceOnDrive), L"%.2f",
                   uiSpaceOnDrive / (float)BYTESINMEGABYTE);

          swprintf(zText, ARR_SIZE(zText), pMessageStrings[MSG_LOWDISKSPACE_WARNING], zSpaceOnDrive,
                   zSizeNeeded);

          if (guiPreviousOptionScreen == MAP_SCREEN)
            DoMapMessageBox(MSG_BOX_BASIC_STYLE, zText, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
          else
            DoMessageBox(MSG_BOX_BASIC_STYLE, zText, GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
        }
        gubCheckForFreeSpaceOnHardDriveCount = DONT_CHECK_FOR_FREE_SPACE;
      }
    }
  }

  // ATE: Force to be in message box screen!
  if (gfInMsgBox) {
    guiPendingScreen = MSG_BOX_SCREEN;
  }

  if (guiPendingScreen != NO_PENDING_SCREEN) {
    // Based on active screen, deinit!
    if (guiPendingScreen != guiCurrentScreen) {
      switch (guiCurrentScreen) {
        case MAP_SCREEN:
          if (guiPendingScreen != MSG_BOX_SCREEN) {
            EndMapScreen(FALSE);
          }
          break;
        case LAPTOP_SCREEN:
          ExitLaptop();
          break;
      }
    }

    // if the screen has chnaged
    if (uiOldScreen != guiPendingScreen) {
      // Set the fact that the screen has changed
      uiOldScreen = guiPendingScreen;

      HandleNewScreenChange(guiPendingScreen, guiCurrentScreen);
    }
    guiCurrentScreen = guiPendingScreen;
    guiPendingScreen = NO_PENDING_SCREEN;
  }

  uiOldScreen = (*(GameScreens[guiCurrentScreen].HandleScreen))();

  // if the screen has chnaged
  if (uiOldScreen != guiCurrentScreen) {
    HandleNewScreenChange(uiOldScreen, guiCurrentScreen);
    guiCurrentScreen = uiOldScreen;
  }

  RefreshScreen(NULL);

  guiGameCycleCounter++;

  UpdateClock();

#ifdef JA2BETAVERSION
  if (gubReportMapscreenLock) {
    ReportMapscreenErrorLock();
  }
#endif
}

void SetCurrentScreen(uint32_t uiNewScreen) {
  guiCurrentScreen = uiNewScreen;
  (*(GameScreens[guiCurrentScreen].HandleScreen))();
}

void SetPendingNewScreen(uint32_t uiNewScreen) { guiPendingScreen = uiNewScreen; }

// Gets called when the screen changes, place any needed in code in here
void HandleNewScreenChange(uint32_t uiNewScreen, uint32_t uiOldScreen) {
  // if we are not going into the message box screen, and we didnt just come from it
  if ((uiNewScreen != MSG_BOX_SCREEN && uiOldScreen != MSG_BOX_SCREEN)) {
    // reset the help screen
    NewScreenSoResetHelpScreen();
  }
}

void HandleShortCutExitState(void) {
  // look at the state of fGameIsRunning, if set false, then prompt user for confirmation

  if (guiCurrentScreen == ERROR_SCREEN) {  // an assert failure, don't bring up the box!
    gfProgramIsRunning = FALSE;
    return;
  }

  if (guiCurrentScreen == AUTORESOLVE_SCREEN) {
    DoMessageBox(MSG_BOX_BASIC_STYLE, pMessageStrings[MSG_EXITGAME], guiCurrentScreen,
                 (uint8_t)(MSG_BOX_FLAG_YESNO | MSG_BOX_FLAG_USE_CENTERING_RECT),
                 EndGameMessageBoxCallBack, GetMapCenteringRect());
    return;
  }

  /// which screen are we in?
  if ((IsMapScreen())) {
    // set up for mapscreen
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMessageStrings[MSG_EXITGAME], MAP_SCREEN,
                    MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack);

  } else if (guiCurrentScreen == LAPTOP_SCREEN) {
    // set up for laptop
    DoLapTopSystemMessageBox(MSG_BOX_LAPTOP_DEFAULT, pMessageStrings[MSG_EXITGAME], LAPTOP_SCREEN,
                             MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack);
  } else if (guiCurrentScreen == SHOPKEEPER_SCREEN) {
    DoSkiMessageBox(MSG_BOX_BASIC_STYLE, pMessageStrings[MSG_EXITGAME], SHOPKEEPER_SCREEN,
                    MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack);
  } else {
    // check if error or editor
#ifdef JA2BETAVERSION
    if (guiCurrentScreen == AIVIEWER_SCREEN || guiCurrentScreen == QUEST_DEBUG_SCREEN) {
      // then don't prompt
      gfProgramIsRunning = FALSE;
      return;
    }
#endif

    if ((guiCurrentScreen == ERROR_SCREEN) || (guiCurrentScreen == EDIT_SCREEN) ||
        (guiCurrentScreen == DEBUG_SCREEN)) {
      // then don't prompt
      gfProgramIsRunning = FALSE;
      return;
    }

    // set up for all otherscreens
    DoMessageBox(MSG_BOX_BASIC_STYLE, pMessageStrings[MSG_EXITGAME], guiCurrentScreen,
                 (uint8_t)(MSG_BOX_FLAG_YESNO | MSG_BOX_FLAG_USE_CENTERING_RECT),
                 EndGameMessageBoxCallBack, GetMapCenteringRect());
  }
}

void EndGameMessageBoxCallBack(uint8_t bExitValue) {
  // yes, so start over, else stay here and do nothing for now
  if (bExitValue == MSG_BOX_RETURN_YES) {
    gfProgramIsRunning = FALSE;
  }

  // If we are in the tactical placement gui, we need this flag set so the interface is updated.
  if (gfTacticalPlacementGUIActive) {
    gfTacticalPlacementGUIDirty = TRUE;
    gfValidLocationsChanged = TRUE;
  }

  return;
}

void NextLoopCheckForEnoughFreeHardDriveSpace() { gubCheckForFreeSpaceOnHardDriveCount = 0; }
