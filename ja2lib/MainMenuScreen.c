// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "MainMenuScreen.h"

#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "Globals.h"
#include "JA2Splash.h"
#include "JAScreens.h"
#include "Local.h"
#include "OptionsScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/GameInit.h"
#include "SysGlobals.h"
#include "Tactical/SoldierControl.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define MAINMENU_TEXT_FILE "LoadScreens\\MainMenu.edt"
#define MAINMENU_RECORD_SIZE 80 * 2

// #define TESTFOREIGNFONTS

// MENU ITEMS
enum {
  //	TITLE,
  NEW_GAME,
  LOAD_GAME,
  PREFERENCES,
  CREDITS,
  QUIT,
  NUM_MENU_ITEMS
};

#define MAINMENU_X ((640 - 214) / 2)
#define MAINMENU_TITLE_Y 75
#define MAINMENU_Y 277  // 200
#define MAINMENU_Y_SPACE 37

int32_t iMenuImages[NUM_MENU_ITEMS];
int32_t iMenuButtons[NUM_MENU_ITEMS];

uint16_t gusMainMenuButtonWidths[NUM_MENU_ITEMS];

uint32_t guiMainMenuBackGroundImage;
uint32_t guiJa2LogoImage;

struct MOUSE_REGION gBackRegion;
int8_t gbHandledMainMenu = 0;
BOOLEAN fInitialRender = FALSE;
// BOOLEAN						gfDoHelpScreen = 0;

BOOLEAN gfMainMenuScreenEntry = FALSE;
BOOLEAN gfMainMenuScreenExit = FALSE;

uint32_t guiMainMenuExitScreen = MAINMENU_SCREEN;

extern BOOLEAN gfLoadGameUponEntry;

void ExitMainMenu();
void MenuButtonCallback(GUI_BUTTON *btn, int32_t reason);
void HandleMainMenuInput();
void HandleMainMenuScreen();
void DisplayAssignmentText();
void ClearMainMenu();
void HandleHelpScreenInput();
void SelectMainMenuBackGroundRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SetMainMenuExitScreen(uint32_t uiNewScreen);
void CreateDestroyBackGroundMouseMask(BOOLEAN fCreate);
BOOLEAN CreateDestroyMainMenuButtons(BOOLEAN fCreate);
void RenderMainMenu();
void RestoreButtonBackGrounds();

uint32_t MainMenuScreenInit() { return (TRUE); }

uint32_t MainMenuScreenHandle() {
  uint32_t cnt;

  if (guiSplashStartTime + 4000 > GetJA2Clock()) {
    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
    SetMusicMode(MUSIC_NONE);
    return MAINMENU_SCREEN;  // The splash screen hasn't been up long enough yet.
  }
  if (guiSplashFrameFade) {  // Fade the splash screen.
    if (guiSplashFrameFade > 2)
      ShadowVideoSurfaceRectUsingLowPercentTable(vsFB, 0, 0, 640, 480);
    else if (guiSplashFrameFade > 1)
      ColorFillVSurfaceArea(vsFB, 0, 0, 640, 480, 0);
    else {
      SetMusicMode(MUSIC_MAIN_MENU);
    }

    // while( uiTime + 100 > GetJA2Clock() );

    guiSplashFrameFade--;

    InvalidateScreen();
    EndFrameBufferRender();

    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

    return MAINMENU_SCREEN;
  }

  SetCurrentCursorFromDatabase(CURSOR_NORMAL);

  if (gfMainMenuScreenEntry) {
    InitMainMenu();
    gfMainMenuScreenEntry = FALSE;
    gfMainMenuScreenExit = FALSE;
    guiMainMenuExitScreen = MAINMENU_SCREEN;
    SetMusicMode(MUSIC_MAIN_MENU);
  }

  if (fInitialRender) {
    ClearMainMenu();
    RenderMainMenu();

    fInitialRender = FALSE;
  }

  RestoreButtonBackGrounds();

  // Render buttons
  for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
    MarkAButtonDirty(iMenuButtons[cnt]);
  }

  RenderButtons();

  EndFrameBufferRender();

  //	if ( gfDoHelpScreen )
  //		HandleHelpScreenInput();
  //	else
  HandleMainMenuInput();

  HandleMainMenuScreen();

  if (gfMainMenuScreenExit) {
    ExitMainMenu();
    gfMainMenuScreenExit = FALSE;
    gfMainMenuScreenEntry = TRUE;
  }

  if (guiMainMenuExitScreen != MAINMENU_SCREEN) gfMainMenuScreenEntry = TRUE;

  return (guiMainMenuExitScreen);
}

uint32_t MainMenuScreenShutdown() { return (FALSE); }

void HandleMainMenuScreen() {
  if (gbHandledMainMenu != 0) {
    // Exit according to handled value!
    switch (gbHandledMainMenu) {
      case QUIT:
        gfMainMenuScreenExit = TRUE;
        gfProgramIsRunning = FALSE;
        break;

      case NEW_GAME:

        //					gfDoHelpScreen = 1;
        //				gfMainMenuScreenExit = TRUE;
        //				if( !gfDoHelpScreen )
        //					SetMainMenuExitScreen( INIT_SCREEN );
        break;

      case LOAD_GAME:
        // Select the game which is to be restored
        guiPreviousOptionScreen = guiCurrentScreen;
        guiMainMenuExitScreen = SAVE_LOAD_SCREEN;
        gbHandledMainMenu = 0;
        gfSaveGame = FALSE;
        gfMainMenuScreenExit = TRUE;

        break;

      case PREFERENCES:
        guiPreviousOptionScreen = guiCurrentScreen;
        guiMainMenuExitScreen = OPTIONS_SCREEN;
        gbHandledMainMenu = 0;
        gfMainMenuScreenExit = TRUE;
        break;

      case CREDITS:
        guiMainMenuExitScreen = CREDIT_SCREEN;
        gbHandledMainMenu = 0;
        gfMainMenuScreenExit = TRUE;
        break;
    }
  }
}

BOOLEAN InitMainMenu() {
  //	gfDoHelpScreen = 0;

  // Check to see whatr saved game files exist
  InitSaveGameArray();

  // Create the background mouse mask
  CreateDestroyBackGroundMouseMask(TRUE);

  CreateDestroyMainMenuButtons(TRUE);

  // load background graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LOADSCREENS\\MainMenuBackGround.sti"),
                    &guiMainMenuBackGroundImage));

  // load ja2 logo graphic and add it
  //	FilenameForBPP("INTERFACE\\Ja2_2.sti", VObjectDesc.ImageFile);
  CHECKF(AddVObject(CreateVObjectFromFile("LOADSCREENS\\Ja2Logo.sti"), &guiJa2LogoImage));

  /*
          // Gray out some buttons based on status of game!
          if( gGameSettings.bLastSavedGameSlot < 0 || gGameSettings.bLastSavedGameSlot >=
     NUM_SAVE_GAMES )
          {
                  DisableButton( iMenuButtons[ LOAD_GAME ] );
          }
          //The ini file said we have a saved game, but there is no saved game
          else if( gbSaveGameArray[ gGameSettings.bLastSavedGameSlot ] == FALSE )
                  DisableButton( iMenuButtons[ LOAD_GAME ] );
  */

  // if there are no saved games, disable the button
  if (!IsThereAnySavedGameFiles()) DisableButton(iMenuButtons[LOAD_GAME]);

  //	DisableButton( iMenuButtons[ CREDITS ] );
  //	DisableButton( iMenuButtons[ TITLE ] );

  gbHandledMainMenu = 0;
  fInitialRender = TRUE;

  SetPendingNewScreen(MAINMENU_SCREEN);
  guiMainMenuExitScreen = MAINMENU_SCREEN;

  InitGameOptions();

  DequeueAllKeyBoardEvents();

  return (TRUE);
}

void ExitMainMenu() {
  //	uint32_t uiDestPitchBYTES; 	uint8_t
  //*pDestBuf;

  //	if( !gfDoHelpScreen )
  { CreateDestroyBackGroundMouseMask(FALSE); }

  CreateDestroyMainMenuButtons(FALSE);

  DeleteVideoObjectFromIndex(guiMainMenuBackGroundImage);
  DeleteVideoObjectFromIndex(guiJa2LogoImage);

  gMsgBox.uiExitScreen = MAINMENU_SCREEN;
}

void MenuButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  int8_t bID;

  bID = (uint8_t)((uintptr_t)btn->UserData[0]);

  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // handle menu
    gbHandledMainMenu = bID;
    RenderMainMenu();

    if (gbHandledMainMenu == NEW_GAME) {
      SetMainMenuExitScreen(GAME_INIT_OPTIONS_SCREEN);
    } else if (gbHandledMainMenu == LOAD_GAME) {
      if (gfKeyState[ALT]) gfLoadGameUponEntry = TRUE;
    }

    btn->uiFlags &= (~BUTTON_CLICKED_ON);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    RenderMainMenu();
    btn->uiFlags |= BUTTON_CLICKED_ON;
  }
}

void MenuButtonMoveCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    //		btn->uiFlags &= (~BUTTON_CLICKED_ON );
    RenderMainMenu();
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    //		btn->uiFlags &= (~BUTTON_CLICKED_ON );
    RenderMainMenu();
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void HandleMainMenuInput() {
  InputAtom InputEvent;

  // Check for esc
  while (DequeueEvent(&InputEvent) == TRUE) {
    if (InputEvent.usEvent == KEY_UP) {
      switch (InputEvent.usParam) {
        /*
                                        case ESC:
                                                gbHandledMainMenu = QUIT;
                                                break;
        */

#ifdef JA2TESTVERSION
        case 'q':
          gbHandledMainMenu = NEW_GAME;
          gfMainMenuScreenExit = TRUE;
          SetMainMenuExitScreen(INIT_SCREEN);

          //						gfDoHelpScreen = 1;
          break;

        case 'i':
          SetPendingNewScreen(INTRO_SCREEN);
          gfMainMenuScreenExit = TRUE;
          break;
#endif

        case 'c':
          gbHandledMainMenu = LOAD_GAME;

          if (gfKeyState[ALT]) gfLoadGameUponEntry = TRUE;

          break;

        case 'o':
          gbHandledMainMenu = PREFERENCES;
          break;

        case 's':
          gbHandledMainMenu = CREDITS;
          break;
      }
    }
  }
}

void HandleHelpScreenInput() {
  InputAtom InputEvent;

  // Check for key
  while (DequeueEvent(&InputEvent) == TRUE) {
    switch (InputEvent.usEvent) {
      case KEY_UP:
        SetMainMenuExitScreen(INIT_SCREEN);
        break;
    }
  }
}

void ClearMainMenu() {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // CLEAR THE FRAME BUFFER
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  memset(pDestBuf, 0, SCREEN_HEIGHT * uiDestPitchBYTES);
  JSurface_Unlock(vsFB);
  InvalidateScreen();
}

void SelectMainMenuBackGroundRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    //		if( gfDoHelpScreen )
    //		{
    //			SetMainMenuExitScreen( INIT_SCREEN );
    //			gfDoHelpScreen = FALSE;
    //		}
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    /*
                    if( gfDoHelpScreen )
                    {
                            SetMainMenuExitScreen( INIT_SCREEN );
                            gfDoHelpScreen = FALSE;
                    }
    */
  }
}

void SetMainMenuExitScreen(uint32_t uiNewScreen) {
  guiMainMenuExitScreen = uiNewScreen;

  // REmove the background region
  CreateDestroyBackGroundMouseMask(FALSE);

  gfMainMenuScreenExit = TRUE;
}

void CreateDestroyBackGroundMouseMask(BOOLEAN fCreate) {
  static BOOLEAN fRegionCreated = FALSE;

  if (fCreate) {
    if (fRegionCreated) return;

    // Make a mouse region
    MSYS_DefineRegion(&(gBackRegion), 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL,
                      MSYS_NO_CALLBACK, SelectMainMenuBackGroundRegionCallBack);
    // Add region
    MSYS_AddRegion(&(gBackRegion));

    fRegionCreated = TRUE;
  } else {
    if (!fRegionCreated) return;

    MSYS_RemoveRegion(&gBackRegion);
    fRegionCreated = FALSE;
  }
}

BOOLEAN CreateDestroyMainMenuButtons(BOOLEAN fCreate) {
  static BOOLEAN fButtonsCreated = FALSE;
  int32_t cnt;
  SGPFILENAME filename;
  int16_t sSlot;
  int32_t iStartLoc = 0;
#ifndef _DEBUG
  wchar_t zText[512];
#endif

  if (fCreate) {
    if (fButtonsCreated) return (TRUE);

    // reset the variable that allows the user to ALT click on the continue save btn to load the
    // save instantly
    gfLoadGameUponEntry = FALSE;

    // Load button images
    GetMLGFilename(filename, MLG_TITLETEXT);

    iMenuImages[NEW_GAME] = LoadButtonImage(filename, 0, 0, 1, 2, -1);
    sSlot = 0;
    iMenuImages[LOAD_GAME] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 6, 3, 4, 5, -1);
    iMenuImages[PREFERENCES] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 7, 7, 8, 9, -1);
    iMenuImages[CREDITS] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 13, 10, 11, 12, -1);
    iMenuImages[QUIT] = UseLoadedButtonImage(iMenuImages[NEW_GAME], 14, 14, 15, 16, -1);

    for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
      switch (cnt) {
        case NEW_GAME:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((uint16_t)iMenuImages[cnt], sSlot);
          break;
        case LOAD_GAME:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((uint16_t)iMenuImages[cnt], 3);
          break;
        case PREFERENCES:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((uint16_t)iMenuImages[cnt], 7);
          break;
        case CREDITS:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((uint16_t)iMenuImages[cnt], 10);
          break;
        case QUIT:
          gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic((uint16_t)iMenuImages[cnt], 15);
          break;
      }
#ifdef TESTFOREIGNFONTS
      iMenuButtons[cnt] =
          QuickCreateButton(iMenuImages[cnt], (int16_t)(320 - gusMainMenuButtonWidths[cnt] / 2),
                            (int16_t)(0 + (cnt * 18)), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
                            DEFAULT_MOVE_CALLBACK, MenuButtonCallback);
#else
      iMenuButtons[cnt] =
          QuickCreateButton(iMenuImages[cnt], (int16_t)(320 - gusMainMenuButtonWidths[cnt] / 2),
                            (int16_t)(MAINMENU_Y + (cnt * MAINMENU_Y_SPACE)), BUTTON_TOGGLE,
                            MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, MenuButtonCallback);
#endif
      if (iMenuButtons[cnt] == -1) {
        return (FALSE);
      }
      ButtonList[iMenuButtons[cnt]]->UserData[0] = cnt;

#ifndef _DEBUG
      // load up some info from the 'mainmenu.edt' file.  This makes sure the file is present.  The
      // file is
      // 'marked' with a code that identifies the testers
      iStartLoc = MAINMENU_RECORD_SIZE * cnt;
      if (!LoadEncryptedDataFromFile(MAINMENU_TEXT_FILE, zText, iStartLoc, MAINMENU_RECORD_SIZE)) {
        // the file was not able to be loaded properly
        struct SOLDIERTYPE *pSoldier = NULL;

        if (IsSolActive(pSoldier) != TRUE) {
          // something is very wrong
          pSoldier->bActive = pSoldier->bLife;
        }
      }
#endif
    }

    fButtonsCreated = TRUE;
  } else {
    if (!fButtonsCreated) return (TRUE);

    // Delete images/buttons
    for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
      RemoveButton(iMenuButtons[cnt]);
      UnloadButtonImage(iMenuImages[cnt]);
    }
    fButtonsCreated = FALSE;
  }

  return (TRUE);
}

void RenderMainMenu() {
  struct VObject *hPixHandle;

  // Get and display the background image
  GetVideoObject(&hPixHandle, guiMainMenuBackGroundImage);
  BltVideoObject(vsSaveBuffer, hPixHandle, 0, 0, 0);
  BltVideoObject(vsFB, hPixHandle, 0, 0, 0);

  GetVideoObject(&hPixHandle, guiJa2LogoImage);
  BltVideoObject(vsFB, hPixHandle, 0, 188, 15);
  BltVideoObject(vsSaveBuffer, hPixHandle, 0, 188, 15);

#ifdef TESTFOREIGNFONTS
  DrawTextToScreen(L"LARGEFONT1: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
                   0, 105, 640, LARGEFONT1, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(L"SMALLFONT1: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
                   0, 125, 640, SMALLFONT1, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(L"TINYFONT1: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
                   0, 145, 640, TINYFONT1, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT12POINT1: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 165,
      640, FONT12POINT1, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(L"COMPFONT: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
                   0, 185, 640, COMPFONT, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"SMALLCOMPFONT: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 205,
      640, SMALLCOMPFONT, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT10ROMAN: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 225,
      640, FONT10ROMAN, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT12ROMAN: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 245,
      640, FONT12ROMAN, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT14SANSERIF: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 255,
      640, FONT14SANSERIF, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"MILITARYFONT: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 265,
      640, MILITARYFONT1, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT10ARIAL: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 285,
      640, FONT10ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT14ARIAL: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 305,
      640, FONT14ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT12ARIAL: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 325,
      640, FONT12ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT10ARIALBOLD: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 345,
      640, FONT10ARIALBOLD, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(L"BLOCKFONT: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
                   0, 365, 640, BLOCKFONT, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(L"BLOCKFONT2: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
                   0, 385, 640, BLOCKFONT2, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT12ARIALFIXEDWIDTH: ����������������������������������������" /*gzCopyrightText[ 0 ]*/,
      0, 405, 640, FONT12ARIALFIXEDWIDTH, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE,
      LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT16ARIAL: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 425,
      640, FONT16ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"BLOCKFONTNARROW: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 445,
      640, BLOCKFONTNARROW, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(
      L"FONT14HUMANIST: ����������������������������������������" /*gzCopyrightText[ 0 ]*/, 0, 465,
      640, FONT14HUMANIST, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
#else
  DrawTextToScreen(gzCopyrightText[0], 0, 465, 640, FONT10ARIAL, FONT_MCOLOR_WHITE,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
#endif

  InvalidateRegion(0, 0, 640, 480);
}

void RestoreButtonBackGrounds() {
  uint8_t cnt;

  //	RestoreExternBackgroundRect( (uint16_t)(320 - gusMainMenuButtonWidths[TITLE]/2),
  // MAINMENU_TITLE_Y, gusMainMenuButtonWidths[TITLE], 23 );

#ifndef TESTFOREIGNFONTS
  for (cnt = 0; cnt < NUM_MENU_ITEMS; cnt++) {
    RestoreExternBackgroundRect((uint16_t)(320 - gusMainMenuButtonWidths[cnt] / 2),
                                (int16_t)(MAINMENU_Y + (cnt * MAINMENU_Y_SPACE) - 1),
                                (uint16_t)(gusMainMenuButtonWidths[cnt] + 1), 23);
  }
#endif
}
