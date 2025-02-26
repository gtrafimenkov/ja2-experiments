// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "HelpScreen.h"

#include "GameScreen.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "HelpScreenText.h"
#include "Laptop/Laptop.h"
#include "Rect.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Line.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameInit.h"
#include "SysGlobals.h"
#include "Tactical/Overhead.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

extern int16_t gsVIEWPORT_END_Y;
extern void PrintDate(void);
extern void PrintNumberOnTeam(void);
extern void PrintBalance(void);
extern BOOLEAN fMapScreenBottomDirty;
extern BOOLEAN fCharacterInfoPanelDirty;
extern BOOLEAN fTeamPanelDirty;
extern BOOLEAN fMapScreenBottomDirty;
extern BOOLEAN fMapPanelDirty;
extern BOOLEAN gfGamePaused;
extern BOOLEAN fShowMapInventoryPool;

#define HELP_SCREEN_ACTIVE 0x00000001

// The defualt size and placement of the screen
#define HELP_SCREEN_DEFUALT_LOC_X 155
#define HELP_SCREEN_DEFUALT_LOC_Y 105

#define HELP_SCREEN_DEFUALT_LOC_WIDTH HELP_SCREEN_SMALL_LOC_WIDTH + HELP_SCREEN_BUTTON_BORDER_WIDTH
#define HELP_SCREEN_DEFUALT_LOC_HEIGHT 292  // 300

#define HELP_SCREEN_BUTTON_BORDER_WIDTH 92
#define HELP_SCREEN_SMALL_LOC_WIDTH 320
#define HELP_SCREEN_SMALL_LOC_HEIGHT HELP_SCREEN_DEFUALT_LOC_HEIGHT  // 224

#define HELP_SCREEN_BTN_OFFSET_X 11
#define HELP_SCREEN_BTN_OFFSET_Y 12  // 50
#define HELP_SCREEN_BTN_FONT_ON_COLOR 73
#define HELP_SCREEN_BTN_FONT_OFF_COLOR FONT_MCOLOR_WHITE

#define HELP_SCREEN_BTN_FONT_BACK_COLOR 50
#define HELP_SCREEN_BTN_FONT FONT10ARIAL

#define HELP_SCREEN_BTN_WIDTH 77
#define HELP_SCREEN_BTN_HEIGHT 22
#define HELP_SCREEN_GAP_BN_BTNS 8

#define HELP_SCREEN_MARGIN_SIZE 10
#define HELP_SCREEN_TEXT_RIGHT_MARGIN_SPACE 36
#define HELP_SCREEN_TEXT_LEFT_MARGIN_WITH_BTN \
  (HELP_SCREEN_BUTTON_BORDER_WIDTH + 5 + HELP_SCREEN_MARGIN_SIZE)
#define HELP_SCREEN_TEXT_LEFT_MARGIN (5 + HELP_SCREEN_MARGIN_SIZE)

#define HELP_SCREEN_TEXT_OFFSET_Y 48
#define HELP_SCREEN_GAP_BTN_LINES 2

#define HELP_SCREEN_TITLE_BODY_FONT FONT12ARIAL
#define HELP_SCREEN_TITLE_BODY_COLOR FONT_MCOLOR_WHITE  // FONT_NEARBLACK

#define HELP_SCREEN_TEXT_BODY_FONT FONT10ARIAL
#define HELP_SCREEN_TEXT_BODY_COLOR FONT_MCOLOR_WHITE  // FONT_NEARBLACK
#define HELP_SCREEN_TEXT_BACKGROUND 0                  // NO_SHADOW//FONT_MCOLOR_WHITE

#define HELP_SCREEN_TITLE_OFFSET_Y 7
#define HELP_SCREEN_HELP_REMINDER_Y HELP_SCREEN_TITLE_OFFSET_Y + 15

#define HELP_SCREEN_NUM_BTNS 8

#define HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_X 4
#define HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_Y 18
#define HELP_SCREEN_SHOW_HELP_AGAIN_REGION_TEXT_OFFSET_X \
  25 + HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_X
#define HELP_SCREEN_SHOW_HELP_AGAIN_REGION_TEXT_OFFSET_Y \
  (HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_Y)

#define HELP_SCREEN_EXIT_BTN_OFFSET_X 291
#define HELP_SCREEN_EXIT_BTN_LOC_Y 9

#define HELP_SCREEN_

// the type of help screen
#define HLP_SCRN_DEFAULT_TYPE 9
#define HLP_SCRN_BUTTON_BORDER 8

// this is the size of the text buffer where everything will be blitted.
// 2 ( bytest for char ) * width of buffer * height of 1 line * # of text lines
// #define	HLP_SCRN__NUMBER_BYTES_IN_TEXT_BUFFER						( 2
//* HLP_SCRN__WIDTH_OF_TEXT_BUFFER * HLP_SCRN__HEIGHT_OF_1_LINE_IN_BUFFER *
// HLP_SCRN__MAX_NUMBER_OF_LINES_IN_BUFFER )
#define HLP_SCRN__WIDTH_OF_TEXT_BUFFER 280
#define HLP_SCRN__MAX_NUMBER_OF_LINES_IN_BUFFER 170  // 100
#define HLP_SCRN__HEIGHT_OF_1_LINE_IN_BUFFER \
  (GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT) + HELP_SCREEN_GAP_BTN_LINES)
#define HLP_SCRN__MAX_NUMBER_PIXELS_DISPLAYED_IN_TEXT_BUFFER HELP_SCREEN_DEFUALT_LOC_HEIGHT
#define HLP_SCRN__HEIGHT_OF_TEXT_BUFFER \
  (HLP_SCRN__HEIGHT_OF_1_LINE_IN_BUFFER * HLP_SCRN__MAX_NUMBER_OF_LINES_IN_BUFFER)

#define HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER \
  (HLP_SCRN__HEIGHT_OF_TEXT_AREA / HLP_SCRN__HEIGHT_OF_1_LINE_IN_BUFFER)

#define HLP_SCRN__HEIGHT_OF_TEXT_AREA 228

#define HLP_SCRN__HEIGHT_OF_SCROLL_AREA 182
#define HLP_SCRN__WIDTH_OF_SCROLL_AREA 20
#define HLP_SCRN__SCROLL_POSX 292
#define HLP_SCRN__SCROLL_POSY (gHelpScreen.usScreenLocY + 63)

#define HLP_SCRN__SCROLL_UP_ARROW_X 292
#define HLP_SCRN__SCROLL_UP_ARROW_Y 43

#define HLP_SCRN__SCROLL_DWN_ARROW_X HLP_SCRN__SCROLL_UP_ARROW_X
#define HLP_SCRN__SCROLL_DWN_ARROW_Y HLP_SCRN__SCROLL_UP_ARROW_Y + 202

// enums for the different dirty levels
enum {
  HLP_SCRN_DRTY_LVL_NOT_DIRTY,
  HLP_SCRN_DRTY_LVL_REFRESH_TEXT,
  HLP_SCRN_DRTY_LVL_REFRESH_ALL,
};

// new screen:

enum {
  HLP_SCRN_MPSCRN_SCTR_OVERVIEW,
};

// mapscreen, welcome to arulco
enum {
  HLP_SCRN_MPSCRN_OVERVIEW,
  HLP_SCRN_MPSCRN_ASSIGNMENTS,
  HLP_SCRN_MPSCRN_DESTINATIONS,
  HLP_SCRN_MPSCRN_MAP,
  HLP_SCRN_MPSCRN_MILITIA,
  HLP_SCRN_MPSCRN_AIRSPACE,
  HLP_SCRN_MPSCRN_ITEMS,
  HLP_SCRN_MPSCRN_KEYBOARD,

  HLP_SCRN_NUM_MPSCRN_BTNS,

};
// laptop sub pages
enum {
  HLP_SCRN_LPTP_OVERVIEW,
  HLP_SCRN_LPTP_EMAIL,
  HLP_SCRN_LPTP_WEB,
  HLP_SCRN_LPTP_FILES,
  HLP_SCRN_LPTP_HISTORY,
  HLP_SCRN_LPTP_PERSONNEL,
  HLP_SCRN_LPTP_FINANCIAL,
  HLP_SCRN_LPTP_MERC_STATS,

  HLP_SCRN_LPTP_NUM_PAGES,
};

// Mapscreen no one hired yet pages
enum {
  HLP_SCRN_NO_ONE_HIRED,

  HLP_SCRN_NUM_MAPSCREEN_NO_1_HIRED_YET_PAGES,
};

// mapscreen no 1 hired yet pages
enum {
  HLP_SCRN_NOT_IN_ARULCO,

  HLP_SCRN_NUM_NOT_IN_ARULCO_PAGES,
};

// Tactical
enum {
  HLP_SCRN_TACTICAL_OVERVIEW,
  HLP_SCRN_TACTICAL_MOVEMENT,
  HLP_SCRN_TACTICAL_SIGHT,
  HLP_SCRN_TACTICAL_ATTACKING,
  HLP_SCRN_TACTICAL_ITEMS,
  HLP_SCRN_TACTICAL_KEYBOARD,

  HLP_SCRN_NUM_TACTICAL_PAGES,

};

// ddd

HELP_SCREEN_STRUCT gHelpScreen;

typedef struct {
  int32_t iButtonTextNum[HELP_SCREEN_NUM_BTNS];

} HELP_SCREEN_BTN_TEXT_RECORD;

// An array of record nums for the text on the help buttons
HELP_SCREEN_BTN_TEXT_RECORD gHelpScreenBtnTextRecordNum[HELP_SCREEN_NUMBER_OF_HELP_SCREENS] = {
    // new screen:

    // Laptop button record nums
    //	HELP_SCREEN_LAPTOP,
    {
        {
            HLP_TXT_LAPTOP_BUTTON_1,
            HLP_TXT_LAPTOP_BUTTON_2,
            HLP_TXT_LAPTOP_BUTTON_3,
            HLP_TXT_LAPTOP_BUTTON_4,
            HLP_TXT_LAPTOP_BUTTON_5,
            HLP_TXT_LAPTOP_BUTTON_6,
            HLP_TXT_LAPTOP_BUTTON_7,
            HLP_TXT_LAPTOP_BUTTON_8,
        },
    },

    //	HELP_SCREEN_MAPSCREEN,
    {
        {
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_1,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_2,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_3,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_4,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_5,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_6,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_7,
            HLP_TXT_WELCOM_TO_ARULCO_BUTTON_8,
        },
    },

    //	HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED,
    {
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
        },
    },

    //	HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO,
    {
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
        },
    },

    //	HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY,
    {
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
        },
    },

    //	HELP_SCREEN_TACTICAL,
    {
        {
            HLP_TXT_TACTICAL_BUTTON_1,
            HLP_TXT_TACTICAL_BUTTON_2,
            HLP_TXT_TACTICAL_BUTTON_3,
            HLP_TXT_TACTICAL_BUTTON_4,
            HLP_TXT_TACTICAL_BUTTON_5,
            HLP_TXT_TACTICAL_BUTTON_6,
            -1,
            -1,
        },
    },

    //	HELP_SCREEN_OPTIONS,
    {
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
        },
    },

    //	HELP_SCREEN_LOAD_GAME,
    {
        {
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
            -1,
        },
    },

};

BOOLEAN gfHelpScreenEntry = TRUE;
BOOLEAN gfHelpScreenExit = FALSE;

uint32_t guiHelpScreenBackGround;
struct JSurface *vsHelpScreenTextBufferSurface;

BOOLEAN gfScrollBoxIsScrolling = FALSE;

BOOLEAN gfHaveRenderedFirstFrameToSaveBuffer = FALSE;

//  must use this cause you have ur cursor over a button when entering the help screen, the button
//  will burn though.
// It does this cause that region loses it focus so it draws the button again.
uint8_t gubRenderHelpScreenTwiceInaRow = 0;

// mmm

// region to mask the background
struct MOUSE_REGION gHelpScreenFullScreenMask;
// void SelectHelpTextFullScreenMaskCallBack(struct MOUSE_REGION * pRegion, int32_t iReason );

// region to mask the background
struct MOUSE_REGION gHelpScreenScrollArea;
void SelectHelpScrollAreaMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectHelpScrollAreaCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// region to mask the background
struct MOUSE_REGION gHelpScreenScrollAreaArrows;
void SelectHelpScrollAreaArrowsCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// checkbox to toggle show help again toggle
uint32_t gHelpScreenDontShowHelpAgainToggle;
void BtnHelpScreenDontShowHelpAgainCallback(GUI_BUTTON *btn, int32_t reason);
// struct MOUSE_REGION    HelpScreenDontShowHelpAgainToggleTextRegion;
// void		HelpScreenDontShowHelpAgainToggleTextRegionCallBack(struct MOUSE_REGION * pRegion,
// int32_t iReason );

int32_t giHelpScreenButtonsImage[HELP_SCREEN_NUM_BTNS];
uint32_t guiHelpScreenBtns[HELP_SCREEN_NUM_BTNS];
void BtnHelpScreenBtnsCallback(GUI_BUTTON *btn, int32_t reason);

int32_t giExitBtnImage;
uint32_t guiHelpScreenExitBtn;
void BtnHelpScreenExitCallback(GUI_BUTTON *btn, int32_t reason);

int32_t giHelpScreenScrollArrows[2];
uint32_t guiHelpScreenScrollArrowImage[2];
void BtnHelpScreenScrollArrowsCallback(GUI_BUTTON *btn, int32_t reason);

// ggg

BOOLEAN EnterHelpScreen();
void HandleHelpScreen();
void RenderHelpScreen();
void ExitHelpScreen();

void GetHelpScreenUserInput();
void HelpScreenSpecialExitCode();
void SetSizeAndPropertiesOfHelpScreen();
BOOLEAN DrawHelpScreenBackGround();
void PrepareToExitHelpScreen();
void SpecialHandlerCode();

uint16_t RenderSpecificHelpScreen();

uint16_t RenderLaptopHelpScreen();
uint16_t RenderTacticalHelpScreen();
uint16_t RenderMapScreenHelpScreen();
uint16_t RenderMapScreenNoOneHiredYetHelpScreen();
uint16_t RenderMapScreenNotYetInArulcoHelpScreen();
uint16_t RenderMapScreenSectorInventoryHelpScreen();

void GetHelpScreenTextPositions(uint16_t *pusPosX, uint16_t *pusPosY, uint16_t *pusWidth);
void DisplayCurrentScreenTitleAndFooter();
void GetHelpScreenText(uint32_t uiRecordToGet, wchar_t *pText);
uint16_t GetAndDisplayHelpScreenText(uint32_t uiRecord, uint16_t usPosX, uint16_t usPosY,
                                     uint16_t usWidth);
void CreateHelpScreenButtons();
void RefreshAllHelpScreenButtons();

void RenderTextBufferToScreen();
void RenderCurrentHelpScreenTextToBuffer();
void DestroyHelpScreenTextBuffer();
BOOLEAN CreateHelpScreenTextBuffer();
void ChangeHelpScreenSubPage();
void ClearHelpScreenTextBuffer();
void ChangeTopLineInTextBufferByAmount(int32_t iAmouontToMove);
void DisplayHelpScreenTextBufferScrollBox();
void CalculateHeightAndPositionForHelpScreenScrollBox(int32_t *piHeightOfScrollBox,
                                                      int32_t *iTopOfScrollBox);
void HelpScreenMouseMoveScrollBox(int32_t usMousePosY);
void CreateScrollAreaButtons();
void DeleteScrollArrowButtons();
void ChangeToHelpScreenSubPage(int8_t bNewPage);
BOOLEAN AreWeClickingOnScrollBar(int32_t usMousePosY);

// ppp

void InitHelpScreenSystem() {
  // set some values
  memset(&gHelpScreen, 0, sizeof(gHelpScreen));

  // set it up so we can enter the screen
  gfHelpScreenEntry = TRUE;
  gfHelpScreenExit = FALSE;

  gHelpScreen.bCurrentHelpScreenActiveSubPage = -1;

  gHelpScreen.fHaveAlreadyBeenInHelpScreenSinceEnteringCurrenScreen = FALSE;
}

BOOLEAN ShouldTheHelpScreenComeUp(uint8_t ubScreenID, BOOLEAN fForceHelpScreenToComeUp) {
  // if the screen is being forsced to come up ( user pressed 'h' )
  if (fForceHelpScreenToComeUp) {
    // Set thefact that the user broughtthe help screen up
    gHelpScreen.fForceHelpScreenToComeUp = TRUE;

    goto HELP_SCREEN_SHOULD_COME_UP;
  }

  // if we are already in the help system, return true
  if (gHelpScreen.uiFlags & HELP_SCREEN_ACTIVE) {
    return (TRUE);
  }

  // has the player been in the screen before
  if ((gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen >> ubScreenID) & 0x01) {
    goto HELP_SCREEN_WAIT_1_FRAME;
  }

  // if we have already been in the screen, and the user DIDNT press 'h', leave
  if (gHelpScreen.fHaveAlreadyBeenInHelpScreenSinceEnteringCurrenScreen) {
    return (FALSE);
  }

  // should the screen come up, based on the users choice for it automatically coming up
  //	if( !( gHelpScreen.fHideHelpInAllScreens ) )
  {
    //		goto HELP_SCREEN_WAIT_1_FRAME;
  }

  // the help screen shouldnt come up
  return (FALSE);

HELP_SCREEN_WAIT_1_FRAME:

  // we have to wait 1 frame while the screen renders
  if (gHelpScreen.bDelayEnteringHelpScreenBy1FrameCount < 2) {
    gHelpScreen.bDelayEnteringHelpScreenBy1FrameCount += 1;

    UnmarkButtonsDirty();

    return (FALSE);
  }

HELP_SCREEN_SHOULD_COME_UP:

  // Record which screen it is

  // if its mapscreen
  if (ubScreenID == HELP_SCREEN_MAPSCREEN) {
    // determine which screen it is ( is any mercs hired, did game just start )
    gHelpScreen.bCurrentHelpScreen = HelpScreenDetermineWhichMapScreenHelpToShow();
  } else {
    gHelpScreen.bCurrentHelpScreen = ubScreenID;
  }

  // mark it that the help screnn is enabled
  gHelpScreen.uiFlags |= HELP_SCREEN_ACTIVE;

  // reset
  gHelpScreen.bDelayEnteringHelpScreenBy1FrameCount = 0;

  return (TRUE);
}

void HelpScreenHandler() {
  // if we are just entering the help screen
  if (gfHelpScreenEntry) {
    // setup the help screen
    EnterHelpScreen();

    gfHelpScreenEntry = FALSE;
    gfHelpScreenExit = FALSE;
  }

  RestoreBackgroundRects();

  // get the mouse and keyboard inputs
  GetHelpScreenUserInput();

  // handle the help screen
  HandleHelpScreen();

  // if the help screen is dirty, re-render it
  if (gHelpScreen.ubHelpScreenDirty != HLP_SCRN_DRTY_LVL_NOT_DIRTY) {
    // temp
    //		gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_ALL;

    RenderHelpScreen();
    gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_NOT_DIRTY;
  }

  // render buttons marked dirty
  //  MarkButtonsDirty( );
  RenderButtons();

  SaveBackgroundRects();
  RenderButtonsFastHelp();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();

  // if we are leaving the help screen
  if (gfHelpScreenExit) {
    gfHelpScreenExit = FALSE;

    gfHelpScreenEntry = TRUE;

    // exit mouse regions etc..
    ExitHelpScreen();

    // reset the helpscreen id
    gHelpScreen.bCurrentHelpScreen = -1;
  }
}

BOOLEAN EnterHelpScreen() {
  uint16_t usPosX, usPosY;  //, usWidth, usHeight;
                            //	int32_t	iStartLoc;
                            //	wchar_t zText[1024];

  // Clear out all the save background rects
  EmptyBackgroundRects();

  UnmarkButtonsDirty();

  // remeber if the game was paused or not ( so when we exit we know what to do )
  gHelpScreen.fWasTheGamePausedPriorToEnteringHelpScreen = gfGamePaused;

  // pause the game
  PauseGame();

  // Determine the help screen size, based off the help screen
  SetSizeAndPropertiesOfHelpScreen();

  // Create a mouse region 'mask' the entrire screen
  MSYS_DefineRegion(&gHelpScreenFullScreenMask, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST,
                    gHelpScreen.usCursor, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
  MSYS_AddRegion(&gHelpScreenFullScreenMask);

  // Create the exit button
  if (gHelpScreen.bNumberOfButtons != 0)
    usPosX =
        gHelpScreen.usScreenLocX + HELP_SCREEN_EXIT_BTN_OFFSET_X + HELP_SCREEN_BUTTON_BORDER_WIDTH;
  else
    usPosX = gHelpScreen.usScreenLocX + HELP_SCREEN_EXIT_BTN_OFFSET_X;

  usPosY = gHelpScreen.usScreenLocY + HELP_SCREEN_EXIT_BTN_LOC_Y;

  // Create the exit buttons
  giExitBtnImage = LoadButtonImage("INTERFACE\\HelpScreen.sti", -1, 0, 4, 2, 6);

  guiHelpScreenExitBtn = CreateIconAndTextButton(
      giExitBtnImage, L"", HELP_SCREEN_BTN_FONT, HELP_SCREEN_BTN_FONT_ON_COLOR, DEFAULT_SHADOW,
      HELP_SCREEN_BTN_FONT_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, usPosX, usPosY,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, BtnHelpScreenExitCallback);
  SetButtonFastHelpText(guiHelpScreenExitBtn, gzHelpScreenText[HLP_SCRN_TXT__EXIT_SCREEN]);
  SetButtonCursor(guiHelpScreenExitBtn, gHelpScreen.usCursor);

  // Create the buttons needed for the screen
  CreateHelpScreenButtons();

  // if there are buttons
  if (gHelpScreen.bNumberOfButtons != 0)
    usPosX = gHelpScreen.usScreenLocX + HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_X +
             HELP_SCREEN_BUTTON_BORDER_WIDTH;
  else
    usPosX = gHelpScreen.usScreenLocX + HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_X;

  usPosY = gHelpScreen.usScreenLocY + gHelpScreen.usScreenHeight -
           HELP_SCREEN_SHOW_HELP_AGAIN_REGION_OFFSET_Y;

  if (!gHelpScreen.fForceHelpScreenToComeUp) {
    gHelpScreenDontShowHelpAgainToggle =
        CreateCheckBoxButton(usPosX, (uint16_t)(usPosY - 3), "INTERFACE\\OptionsCheckBoxes.sti",
                             MSYS_PRIORITY_HIGHEST, BtnHelpScreenDontShowHelpAgainCallback);

    SetButtonCursor(gHelpScreenDontShowHelpAgainToggle, gHelpScreen.usCursor);

    // Set the state of the chec box
    if (gGameSettings.fHideHelpInAllScreens)
      ButtonList[gHelpScreenDontShowHelpAgainToggle]->uiFlags |= BUTTON_CLICKED_ON;
    else
      ButtonList[gHelpScreenDontShowHelpAgainToggle]->uiFlags &= ~BUTTON_CLICKED_ON;
  }

  // load the help screen background graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\HelpScreen.sti"), &guiHelpScreenBackGround));

  // create the text buffer
  CreateHelpScreenTextBuffer();

  // make sure we redraw everything
  gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_ALL;

  // mark it that we have been in since we enter the current screen
  gHelpScreen.fHaveAlreadyBeenInHelpScreenSinceEnteringCurrenScreen = TRUE;

  // set the fact that we have been to the screen
  gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen &= ~(1 << gHelpScreen.bCurrentHelpScreen);

  // always start at the top
  gHelpScreen.iLineAtTopOfTextBuffer = 0;

  // set it so there was no previous click
  gHelpScreen.iLastMouseClickY = -1;

  // Create the scroll box, and scroll arrow regions/buttons
  CreateScrollAreaButtons();

  // render the active page to the text buffer
  ChangeHelpScreenSubPage();

  // reset scroll box flag
  gfScrollBoxIsScrolling = FALSE;

  // reset first frame buffer
  gfHaveRenderedFirstFrameToSaveBuffer = FALSE;

  gubRenderHelpScreenTwiceInaRow = 0;

  return (TRUE);
}

void HandleHelpScreen() {
  // if any of the possible screens need to have a some code done every loop..  its done in here
  SpecialHandlerCode();

  if (gfScrollBoxIsScrolling) {
    if (gfLeftButtonState) {
      HelpScreenMouseMoveScrollBox(gusMouseYPos);
    } else {
      gfScrollBoxIsScrolling = FALSE;
      gHelpScreen.iLastMouseClickY = -1;
    }
  }

  if (gubRenderHelpScreenTwiceInaRow < 3) {
    // test
    //		gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_ALL;

    gubRenderHelpScreenTwiceInaRow++;

    UnmarkButtonsDirty();
  }

  // refresh all of help screens buttons
  RefreshAllHelpScreenButtons();
}

void RenderHelpScreen() {
  // rrr

  if (gfHaveRenderedFirstFrameToSaveBuffer) {
    // Restore the background before blitting the text back on
    RestoreExternBackgroundRect(gHelpScreen.usScreenLocX, gHelpScreen.usScreenLocY,
                                gHelpScreen.usScreenWidth, gHelpScreen.usScreenHeight);
  }

  if (gHelpScreen.ubHelpScreenDirty == HLP_SCRN_DRTY_LVL_REFRESH_ALL) {
    // Display the helpscreen background
    DrawHelpScreenBackGround();

    // Display the current screens title, and footer info
    DisplayCurrentScreenTitleAndFooter();
  }

  if (!gfHaveRenderedFirstFrameToSaveBuffer) {
    gfHaveRenderedFirstFrameToSaveBuffer = TRUE;

    // blit everything to the save buffer ( cause the save buffer can bleed through )
    BlitBufferToBuffer(vsFB, vsSaveBuffer, gHelpScreen.usScreenLocX, gHelpScreen.usScreenLocY,
                       (uint16_t)(gHelpScreen.usScreenLocX + gHelpScreen.usScreenWidth),
                       (uint16_t)(gHelpScreen.usScreenLocY + gHelpScreen.usScreenHeight));

    UnmarkButtonsDirty();
  }

  // render the text buffer to the screen
  if (gHelpScreen.ubHelpScreenDirty >= HLP_SCRN_DRTY_LVL_REFRESH_TEXT) {
    RenderTextBufferToScreen();
  }
}

void ExitHelpScreen() {
  int32_t i;

  if (!gHelpScreen.fForceHelpScreenToComeUp) {
    // Get the current value of the checkbox
    if (ButtonList[gHelpScreenDontShowHelpAgainToggle]->uiFlags & BUTTON_CLICKED_ON) {
      gGameSettings.fHideHelpInAllScreens = TRUE;
      gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen = 0;
    } else {
      gGameSettings.fHideHelpInAllScreens = FALSE;
    }

    // remove the mouse region for the '[ ] dont show help...'
    RemoveButton(gHelpScreenDontShowHelpAgainToggle);
  }

  // mark it that the help screen is not active
  gHelpScreen.uiFlags &= ~HELP_SCREEN_ACTIVE;

  // remove the mouse region that blankets
  MSYS_RemoveRegion(&gHelpScreenFullScreenMask);

  // checkbox to toggle show help again toggle
  //	MSYS_RemoveRegion( &HelpScreenDontShowHelpAgainToggleTextRegion );

  // remove the hepl graphic
  DeleteVideoObjectFromIndex(guiHelpScreenBackGround);

  // remove the exit button
  RemoveButton(guiHelpScreenExitBtn);

  // if there are any buttons, remove them
  if (gHelpScreen.bNumberOfButtons != 0) {
    for (i = 0; i < gHelpScreen.bNumberOfButtons; i++) {
      UnloadButtonImage(giHelpScreenButtonsImage[i]);
      RemoveButton(guiHelpScreenBtns[i]);
    }
  }

  // destroy the text buffer for the help screen
  DestroyHelpScreenTextBuffer();

  // Handles the dirtying of any special screen we are about to reenter
  HelpScreenSpecialExitCode();

  // if the game was NOT paused
  if (gHelpScreen.fWasTheGamePausedPriorToEnteringHelpScreen == FALSE) {
    // un pause the game
    UnPauseGame();
  }

  // Delete the scroll box, and scroll arrow regions/buttons
  DeleteScrollArrowButtons();

  // reset
  gHelpScreen.fForceHelpScreenToComeUp = FALSE;

  SaveGameSettings();
}

BOOLEAN DrawHelpScreenBackGround() {
  struct VObject *hPixHandle;
  uint16_t usPosX;

  // Get and display the background image
  GetVideoObject(&hPixHandle, guiHelpScreenBackGround);

  usPosX = gHelpScreen.usScreenLocX;

  // if there are buttons, blit the button border
  if (gHelpScreen.bNumberOfButtons != 0) {
    BltVideoObject(vsFB, hPixHandle, HLP_SCRN_BUTTON_BORDER, usPosX, gHelpScreen.usScreenLocY);
    usPosX += HELP_SCREEN_BUTTON_BORDER_WIDTH;
  }

  BltVideoObject(vsFB, hPixHandle, HLP_SCRN_DEFAULT_TYPE, usPosX, gHelpScreen.usScreenLocY);

  InvalidateRegion(gHelpScreen.usScreenLocX, gHelpScreen.usScreenLocY,
                   gHelpScreen.usScreenLocX + gHelpScreen.usScreenWidth,
                   gHelpScreen.usScreenLocY + gHelpScreen.usScreenHeight);

  return (TRUE);
}

void SetSizeAndPropertiesOfHelpScreen() {
  // new screen:
  gHelpScreen.bNumberOfButtons = 0;

  //
  // these are the default settings, so if the screen uses different then defualt, set them in the
  // switch
  //
  {
    gHelpScreen.usScreenWidth = HELP_SCREEN_DEFUALT_LOC_WIDTH;
    gHelpScreen.usScreenHeight = HELP_SCREEN_DEFUALT_LOC_HEIGHT;

    gHelpScreen.usScreenLocX = (640 - gHelpScreen.usScreenWidth) / 2;
    gHelpScreen.usScreenLocY = (480 - gHelpScreen.usScreenHeight) / 2;

    gHelpScreen.bCurrentHelpScreenActiveSubPage = 0;

    gHelpScreen.usCursor = CURSOR_NORMAL;
  }

  switch (gHelpScreen.bCurrentHelpScreen) {
    case HELP_SCREEN_LAPTOP:
      gHelpScreen.bNumberOfButtons = HLP_SCRN_LPTP_NUM_PAGES;
      gHelpScreen.usCursor = CURSOR_LAPTOP_SCREEN;

      // center the screen inside the laptop screen
      gHelpScreen.usScreenLocX =
          LAPTOP_SCREEN_UL_X + (LAPTOP_SCREEN_WIDTH - gHelpScreen.usScreenWidth) / 2;
      gHelpScreen.usScreenLocY =
          LAPTOP_SCREEN_UL_Y + (LAPTOP_SCREEN_HEIGHT - gHelpScreen.usScreenHeight) / 2;

      break;
    case HELP_SCREEN_MAPSCREEN:
      gHelpScreen.bNumberOfButtons = HLP_SCRN_NUM_MPSCRN_BTNS;

      // calc the center position based on the current panel thats being displayed
      gHelpScreen.usScreenLocY = (gsVIEWPORT_END_Y - gHelpScreen.usScreenHeight) / 2;
      break;
    case HELP_SCREEN_TACTICAL:
      gHelpScreen.bNumberOfButtons = HLP_SCRN_NUM_TACTICAL_PAGES;

      // calc the center position based on the current panel thats being displayed
      gHelpScreen.usScreenLocY = (gsVIEWPORT_END_Y - gHelpScreen.usScreenHeight) / 2;
      break;

    case HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED:
    case HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO:
    case HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY:
      gHelpScreen.usScreenWidth = HELP_SCREEN_SMALL_LOC_WIDTH;
      gHelpScreen.usScreenHeight = HELP_SCREEN_SMALL_LOC_HEIGHT;

      // calc screen position since we just set the width and height
      gHelpScreen.usScreenLocX = (640 - gHelpScreen.usScreenWidth) / 2;

      // calc the center position based on the current panel thats being displayed
      gHelpScreen.usScreenLocY = (gsVIEWPORT_END_Y - gHelpScreen.usScreenHeight) / 2;

      gHelpScreen.bNumberOfButtons = 0;
      gHelpScreen.bCurrentHelpScreenActiveSubPage = 0;
      break;

    case HELP_SCREEN_OPTIONS:
    case HELP_SCREEN_LOAD_GAME:
      break;

    default:
#ifdef JA2BETAVERSION
      AssertMsg(0, "Error in help screen.  DF 0");
#else
      break;
#endif
  }

  // if there are buttons
  if (gHelpScreen.bNumberOfButtons != 0)
    gHelpScreen.usLeftMarginPosX = gHelpScreen.usScreenLocX + HELP_SCREEN_TEXT_LEFT_MARGIN_WITH_BTN;
  else
    gHelpScreen.usLeftMarginPosX = gHelpScreen.usScreenLocX + HELP_SCREEN_TEXT_LEFT_MARGIN;
}

void CreateHelpScreenButtons() {
  uint16_t usPosX, usPosY;
  wchar_t sText[1024];
  int32_t i;

  // if there are buttons to create
  if (gHelpScreen.bNumberOfButtons != 0) {
    usPosX = gHelpScreen.usScreenLocX + HELP_SCREEN_BTN_OFFSET_X;
    usPosY = HELP_SCREEN_BTN_OFFSET_Y + gHelpScreen.usScreenLocY;

    // loop through all the buttons, and create them
    for (i = 0; i < gHelpScreen.bNumberOfButtons; i++) {
      // get the text for the button
      GetHelpScreenText(
          gHelpScreenBtnTextRecordNum[gHelpScreen.bCurrentHelpScreen].iButtonTextNum[i], sText);

      /*
                              guiHelpScreenBtns[i] = CreateTextButton( sText, HELP_SCREEN_BTN_FONT,
         HELP_SCREEN_BTN_FONT_COLOR, HELP_SCREEN_BTN_FONT_BACK_COLOR, BUTTON_USE_DEFAULT, usPosX,
         usPosY, HELP_SCREEN_BTN_WIDTH, HELP_SCREEN_BTN_HEIGHT, BUTTON_TOGGLE,
         MSYS_PRIORITY_HIGHEST, BUTTON_NO_CALLBACK, BtnHelpScreenBtnsCallback );
      */

      giHelpScreenButtonsImage[i] = UseLoadedButtonImage(giExitBtnImage, -1, 1, 5, 3, 7);

      guiHelpScreenBtns[i] = CreateIconAndTextButton(
          giHelpScreenButtonsImage[i], sText, HELP_SCREEN_BTN_FONT, HELP_SCREEN_BTN_FONT_ON_COLOR,
          DEFAULT_SHADOW, HELP_SCREEN_BTN_FONT_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, usPosX,
          usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK,
          BtnHelpScreenBtnsCallback);

      SetButtonCursor(guiHelpScreenBtns[i], gHelpScreen.usCursor);
      MSYS_SetBtnUserData(guiHelpScreenBtns[i], 0, i);

      //	SpecifyButtonTextOffsets( guiHelpScreenBtns[i], 19, 9, TRUE );

      usPosY += HELP_SCREEN_BTN_HEIGHT + HELP_SCREEN_GAP_BN_BTNS;
    }

    ButtonList[guiHelpScreenBtns[0]]->uiFlags |= BUTTON_CLICKED_ON;
  }
}

void GetHelpScreenUserInput() {
  InputAtom Event;
  struct Point MousePos = GetMousePoint();

  while (DequeueEvent(&Event)) {
    // HOOK INTO MOUSE HOOKS
    switch (Event.usEvent) {
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
      case RIGHT_BUTTON_REPEAT:
        MouseSystemHook(RIGHT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case LEFT_BUTTON_REPEAT:
        MouseSystemHook(LEFT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
    }

    if (!HandleTextInput(&Event) && Event.usEvent == KEY_UP) {
      switch (Event.usParam) {
        case ESC:
          PrepareToExitHelpScreen();
          break;

        case DNARROW: {
          ChangeTopLineInTextBufferByAmount(1);
        } break;

        case UPARROW: {
          ChangeTopLineInTextBufferByAmount(-1);
        } break;

        case PGUP: {
          ChangeTopLineInTextBufferByAmount(-(HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER - 1));
        } break;
        case PGDN: {
          ChangeTopLineInTextBufferByAmount((HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER - 1));
        } break;

        case LEFTARROW:
          ChangeToHelpScreenSubPage((int8_t)(gHelpScreen.bCurrentHelpScreenActiveSubPage - 1));
          break;

        case RIGHTARROW:
          ChangeToHelpScreenSubPage((int8_t)(gHelpScreen.bCurrentHelpScreenActiveSubPage + 1));
          break;

          /*

                                          case LEFTARROW:
                                          {
                                          }
                                                  break;

                                          case RIGHTARROW:
                                          {
                                          }
                                                  break;
          */

#ifdef JA2TESTVERSION
        // rerender the hepl screen
        case 'r':
          gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_ALL;
          break;

        case 'i':
          InvalidateRegion(0, 0, 640, 480);
          break;

        case 'd':
          InvalidateRegion(gHelpScreen.usScreenLocX, gHelpScreen.usScreenLocY,
                           gHelpScreen.usScreenLocX + gHelpScreen.usScreenWidth,
                           gHelpScreen.usScreenLocY + gHelpScreen.usScreenHeight);
          break;
#endif
      }
    }

    if (!HandleTextInput(&Event) && Event.usEvent == KEY_REPEAT) {
      switch (Event.usParam) {
        case DNARROW: {
          ChangeTopLineInTextBufferByAmount(1);
        } break;

        case UPARROW: {
          ChangeTopLineInTextBufferByAmount(-1);
        } break;

        case PGUP: {
          ChangeTopLineInTextBufferByAmount(-(HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER - 1));
        } break;
        case PGDN: {
          ChangeTopLineInTextBufferByAmount((HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER - 1));
        } break;
      }
    }
  }
}

// Handles anything spcial that must be done when exiting the specific screen we are about to
// reenter ( eg. dirtying of the screen )
void HelpScreenSpecialExitCode() {
  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreen) {
    case HELP_SCREEN_LAPTOP:
      fReDrawScreenFlag = TRUE;
      break;

    case HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED:
    case HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO:
    case HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY:
    case HELP_SCREEN_MAPSCREEN:
      fCharacterInfoPanelDirty = TRUE;
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;
      MarkForRedrawalStrategicMap();
      break;

    case HELP_SCREEN_TACTICAL:
      fInterfacePanelDirty = DIRTYLEVEL2;
      SetRenderFlags(RENDER_FLAG_FULL);
      break;

    case HELP_SCREEN_OPTIONS:
      break;
    case HELP_SCREEN_LOAD_GAME:
      break;

    default:
#ifdef JA2BETAVERSION
      AssertMsg(0, "Error in help screen.  DF 0");
#else
      break;
#endif
  }
}

void PrepareToExitHelpScreen() { gfHelpScreenExit = TRUE; }

// Handles anything special that must be done when exiting the specific screen we are about to
// reenter ( eg. dirtying of the screen )
void SpecialHandlerCode() {
  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreen) {
    case HELP_SCREEN_LAPTOP:
      PrintDate();
      PrintBalance();
      PrintNumberOnTeam();
      break;
    case HELP_SCREEN_MAPSCREEN:
      break;
    case HELP_SCREEN_TACTICAL:
      break;

    case HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED:
      break;
    case HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO:
      break;
    case HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY:
      break;
      break;
    case HELP_SCREEN_OPTIONS:
      break;
    case HELP_SCREEN_LOAD_GAME:
      break;

    default:
#ifdef JA2BETAVERSION
      AssertMsg(0, "Error in help screen:  SpecialHandlerCode().  DF 0");
#else
      break;
#endif
  }
}

uint16_t RenderSpecificHelpScreen() {
  uint16_t usNumVerticalPixelsDisplayed = 0;

  SetFontDestBuffer(vsHelpScreenTextBufferSurface, 0, 0, HLP_SCRN__WIDTH_OF_TEXT_BUFFER,
                    HLP_SCRN__HEIGHT_OF_TEXT_BUFFER, FALSE);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreen) {
    case HELP_SCREEN_LAPTOP:
      usNumVerticalPixelsDisplayed = RenderLaptopHelpScreen();
      break;
    case HELP_SCREEN_MAPSCREEN:
      usNumVerticalPixelsDisplayed = RenderMapScreenHelpScreen();
      break;
    case HELP_SCREEN_TACTICAL:
      usNumVerticalPixelsDisplayed = RenderTacticalHelpScreen();
      break;
    case HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED:
      usNumVerticalPixelsDisplayed = RenderMapScreenNoOneHiredYetHelpScreen();
      break;
    case HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO:
      usNumVerticalPixelsDisplayed = RenderMapScreenNotYetInArulcoHelpScreen();
      break;
    case HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY:
      usNumVerticalPixelsDisplayed = RenderMapScreenSectorInventoryHelpScreen();
      break;
    case HELP_SCREEN_OPTIONS:
      break;
    case HELP_SCREEN_LOAD_GAME:
      break;

    default:
#ifdef JA2BETAVERSION
      SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
      AssertMsg(0, "Error in help screen:  RenderSpecificHelpScreen().  DF 0");
#else
      break;
#endif
  }

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  // add 1 line to the bottom of the buffer
  usNumVerticalPixelsDisplayed += 10;

  return (usNumVerticalPixelsDisplayed);
}

void GetHelpScreenTextPositions(uint16_t *pusPosX, uint16_t *pusPosY, uint16_t *pusWidth) {
  // if there are buttons
  if (pusPosX != NULL) *pusPosX = 0;

  if (pusWidth != NULL)
    *pusWidth = HLP_SCRN__WIDTH_OF_TEXT_BUFFER - 1 * HELP_SCREEN_MARGIN_SIZE;  // DEF was 2

  if (pusPosY != NULL) *pusPosY = 0;
}

void DisplayCurrentScreenTitleAndFooter() {
  int32_t iStartLoc = -1;
  wchar_t zText[1024];
  uint16_t usPosX = 0, usPosY = 0, usWidth = 0;

  // new screen:

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreen) {
    case HELP_SCREEN_LAPTOP:
      iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_LAPTOP_TITLE;
      break;
    case HELP_SCREEN_MAPSCREEN:
      iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_WELCOM_TO_ARULCO_TITLE;
      break;
    case HELP_SCREEN_TACTICAL:
      iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_TACTICAL_TITLE;
      break;
    case HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED:
      iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_MPSCRN_NO_1_HIRED_YET_TITLE;
      break;
    case HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO:
      iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_MPSCRN_NOT_IN_ARULCO_TITLE;
      break;
    case HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY:
      iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_SECTOR_INVTRY_TITLE;
      break;
    case HELP_SCREEN_OPTIONS:
      break;
    case HELP_SCREEN_LOAD_GAME:
      break;

    default:
#ifdef JA2BETAVERSION
      AssertMsg(0, "Error in help screen:  DisplayCurrentScreenTitleAndFooter().  DF 0");
#else
      break;
#endif
  }

  //	GetHelpScreenTextPositions( NULL, NULL, &usWidth );

  if (gHelpScreen.bNumberOfButtons != 0)
    usWidth = gHelpScreen.usScreenWidth - HELP_SCREEN_TEXT_LEFT_MARGIN_WITH_BTN -
              HELP_SCREEN_TEXT_RIGHT_MARGIN_SPACE;
  else
    usWidth = gHelpScreen.usScreenWidth - HELP_SCREEN_TEXT_LEFT_MARGIN -
              HELP_SCREEN_TEXT_RIGHT_MARGIN_SPACE;

  // if this screen has a valid title
  if (iStartLoc != -1) {
    LoadEncryptedDataFromFile(HELPSCREEN_FILE, zText, iStartLoc, HELPSCREEN_RECORD_SIZE);

    SetFontShadow(NO_SHADOW);

    usPosX = gHelpScreen.usLeftMarginPosX;

    //		DrawTextToScreen( zText, usPosX,
    //(uint16_t)(gHelpScreen.usScreenLocY+HELP_SCREEN_TITLE_OFFSET_Y), usWidth,
    //									 HELP_SCREEN_TITLE_BODY_FONT,
    // HELP_SCREEN_TITLE_BODY_COLOR, HELP_SCREEN_TEXT_BACKGROUND, FALSE, CENTER_JUSTIFIED );

    // Display the Title
    IanDisplayWrappedString(
        usPosX, (uint16_t)(gHelpScreen.usScreenLocY + HELP_SCREEN_TITLE_OFFSET_Y), usWidth,
        HELP_SCREEN_GAP_BTN_LINES, HELP_SCREEN_TITLE_BODY_FONT, HELP_SCREEN_TITLE_BODY_COLOR, zText,
        HELP_SCREEN_TEXT_BACKGROUND, FALSE, 0);
  }

  // Display the '( press H to get help... )'
  iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_CONSTANT_SUBTITLE;
  LoadEncryptedDataFromFile(HELPSCREEN_FILE, zText, iStartLoc, HELPSCREEN_RECORD_SIZE);

  usPosX = gHelpScreen.usLeftMarginPosX;

  usPosY = gHelpScreen.usScreenLocY + HELP_SCREEN_HELP_REMINDER_Y;
  //	DrawTextToScreen( zText, usPosX, usPosY, usWidth,
  //								 HELP_SCREEN_TEXT_BODY_FONT,
  // HELP_SCREEN_TITLE_BODY_COLOR, HELP_SCREEN_TEXT_BACKGROUND, FALSE, CENTER_JUSTIFIED );

  IanDisplayWrappedString(usPosX, usPosY, usWidth, HELP_SCREEN_GAP_BTN_LINES,
                          HELP_SCREEN_TITLE_BODY_FONT, HELP_SCREEN_TITLE_BODY_COLOR, zText,
                          HELP_SCREEN_TEXT_BACKGROUND, FALSE, 0);

  if (!gHelpScreen.fForceHelpScreenToComeUp) {
    // calc location for the ' [ x ] Dont display again...'
    iStartLoc = HELPSCREEN_RECORD_SIZE * HLP_TXT_CONSTANT_FOOTER;
    LoadEncryptedDataFromFile(HELPSCREEN_FILE, zText, iStartLoc, HELPSCREEN_RECORD_SIZE);

    usPosX = gHelpScreen.usLeftMarginPosX + HELP_SCREEN_SHOW_HELP_AGAIN_REGION_TEXT_OFFSET_X;

    usPosY = gHelpScreen.usScreenLocY + gHelpScreen.usScreenHeight -
             HELP_SCREEN_SHOW_HELP_AGAIN_REGION_TEXT_OFFSET_Y + 2;

    // Display the ' [ x ] Dont display again...'
    IanDisplayWrappedString(usPosX, usPosY, usWidth, HELP_SCREEN_GAP_BTN_LINES,
                            HELP_SCREEN_TEXT_BODY_FONT, HELP_SCREEN_TITLE_BODY_COLOR, zText,
                            HELP_SCREEN_TEXT_BACKGROUND, FALSE, 0);
  }

  SetFontShadow(DEFAULT_SHADOW);
}

void BtnHelpScreenBtnsCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    //		btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // Get the btn id
    int8_t bRetValue = (uint8_t)MSYS_GetBtnUserData(btn, 0);

    ChangeToHelpScreenSubPage(bRetValue);
    /*
                    //change the current page to the new one
                    gHelpScreen.bCurrentHelpScreenActiveSubPage = ( bRetValue >
       gHelpScreen.bNumberOfButtons ) ? gHelpScreen.bNumberOfButtons-1 : bRetValue;

                    gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_TEXT;

                    for( i=0; i< gHelpScreen.bNumberOfButtons; i++ )
                    {
                            ButtonList[ guiHelpScreenBtns[i] ]->uiFlags &= (~BUTTON_CLICKED_ON );
                    }

                    //change the current sub page, and render it to the buffer
                    ChangeHelpScreenSubPage();
    */
    btn->uiFlags |= BUTTON_CLICKED_ON;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    //		btn->uiFlags &= (~BUTTON_CLICKED_ON );
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void ChangeToHelpScreenSubPage(int8_t bNewPage) {
  int8_t i;

  // if for some reason, we are assigning a lower number
  if (bNewPage < 0) {
    gHelpScreen.bCurrentHelpScreenActiveSubPage = 0;
  }

  // for some reason if the we are passing in a # that is greater then the max, set it to the max
  else if (bNewPage >= gHelpScreen.bNumberOfButtons) {
    gHelpScreen.bCurrentHelpScreenActiveSubPage =
        (gHelpScreen.bNumberOfButtons == 0) ? 0 : gHelpScreen.bNumberOfButtons - 1;
  }

  // if we are selecting the current su page, exit
  else if (bNewPage == gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    return;
  }

  // else assign the new subpage
  else {
    gHelpScreen.bCurrentHelpScreenActiveSubPage = bNewPage;
  }

  // refresh the screen
  gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_TEXT;

  //'undepress' all the buttons
  for (i = 0; i < gHelpScreen.bNumberOfButtons; i++) {
    ButtonList[guiHelpScreenBtns[i]]->uiFlags &= (~BUTTON_CLICKED_ON);
  }

  // depress the proper button
  ButtonList[guiHelpScreenBtns[gHelpScreen.bCurrentHelpScreenActiveSubPage]]->uiFlags |=
      BUTTON_CLICKED_ON;

  // change the current sub page, and render it to the buffer
  ChangeHelpScreenSubPage();
}

void GetHelpScreenText(uint32_t uiRecordToGet, wchar_t *pText) {
  int32_t iStartLoc = -1;

  iStartLoc = HELPSCREEN_RECORD_SIZE * uiRecordToGet;
  LoadEncryptedDataFromFile(HELPSCREEN_FILE, pText, iStartLoc, HELPSCREEN_RECORD_SIZE);
}

// returns the number of vertical pixels printed
uint16_t GetAndDisplayHelpScreenText(uint32_t uiRecord, uint16_t usPosX, uint16_t usPosY,
                                     uint16_t usWidth) {
  wchar_t zText[1024];
  uint16_t usNumVertPixels = 0;
  uint32_t uiStartLoc;

  SetFontShadow(NO_SHADOW);

  GetHelpScreenText(uiRecord, zText);

  // Get the record
  uiStartLoc = HELPSCREEN_RECORD_SIZE * uiRecord;
  LoadEncryptedDataFromFile(HELPSCREEN_FILE, zText, uiStartLoc, HELPSCREEN_RECORD_SIZE);

  // Display the text
  usNumVertPixels = IanDisplayWrappedString(usPosX, usPosY, usWidth, HELP_SCREEN_GAP_BTN_LINES,
                                            HELP_SCREEN_TEXT_BODY_FONT, HELP_SCREEN_TEXT_BODY_COLOR,
                                            zText, HELP_SCREEN_TEXT_BACKGROUND, FALSE, 0);

  SetFontShadow(DEFAULT_SHADOW);

  return (usNumVertPixels);
}

void BtnHelpScreenDontShowHelpAgainCallback(GUI_BUTTON *btn, int32_t reason) {
  //	uint8_t	ubButton = (uint8_t)MSYS_GetBtnUserData( btn, 0 );

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    /*
                    btn->uiFlags &= ~BUTTON_CLICKED_ON;

                    if( gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen & ( 1 <<
    gHelpScreen.bCurrentHelpScreen ) )
                    {
    //
                            gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen &= ~( 1 <<
    gHelpScreen.bCurrentHelpScreen );
                    }
                    else
                    {
    //			gHelpScreen.usHasPlayerSeenHelpScreenInCurrentScreen |= ( 1 <<
    gHelpScreen.bCurrentHelpScreen );

                    }
    //		btn->uiFlags |= BUTTON_CLICKED_ON;
    */
  }
}

/*
void HelpScreenDontShowHelpAgainToggleTextRegionCallBack(struct MOUSE_REGION * pRegion, int32_t
iReason )
{
        if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
        {
                InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY,
pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
        }


        else if( iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                if( gGameSettings.fOptions[ ubButton ] )
                {
                }
                else
                {
                }
        }
}
*/

// set the fact the we have chmaged to a new screen
void NewScreenSoResetHelpScreen() {
  gHelpScreen.fHaveAlreadyBeenInHelpScreenSinceEnteringCurrenScreen = FALSE;
  gHelpScreen.bDelayEnteringHelpScreenBy1FrameCount = 0;
}

void BtnHelpScreenExitCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);

    PrepareToExitHelpScreen();

    btn->uiFlags &= (~BUTTON_CLICKED_ON);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

uint16_t RenderLaptopHelpScreen() {
  uint16_t usPosX, usPosY, usWidth, usNumVertPixels;
  uint8_t ubCnt;
  uint16_t usTotalNumberOfVerticalPixels = 0;
  uint16_t usFontHeight = GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT);

  if (gHelpScreen.bCurrentHelpScreenActiveSubPage == -1) {
    return (0);
  }

  // Get the position for the text
  GetHelpScreenTextPositions(&usPosX, &usPosY, &usWidth);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    case HLP_SCRN_LPTP_OVERVIEW:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 2; ubCnt++) {
        // Display the text, and get the number of pixels it used to display it
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_LAPTOP_OVERVIEW_P1 + ubCnt, usPosX,
                                                      usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      /*
                              //Display the first paragraph
                              usTotalNumberOfVerticalPixels = GetAndDisplayHelpScreenText(
         HLP_TXT_LAPTOP_OVERVIEW_P1, usPosX, usPosY, usWidth );

                              usPosY = usPosY+ usNumVertPixels + GetFontHeight(
         HELP_SCREEN_TEXT_BODY_FONT );

                              //Display the second paragraph
                              usTotalNumberOfVerticalPixels += GetAndDisplayHelpScreenText(
         HLP_TXT_LAPTOP_OVERVIEW_P2, usPosX, usPosY, usWidth );
      */
      break;

    case HLP_SCRN_LPTP_EMAIL:

      // Display the first paragraph
      usTotalNumberOfVerticalPixels =
          GetAndDisplayHelpScreenText(HLP_TXT_LAPTOP_EMAIL_P1, usPosX, usPosY, usWidth);
      break;

    case HLP_SCRN_LPTP_WEB:

      // Display the first paragraph
      usTotalNumberOfVerticalPixels =
          GetAndDisplayHelpScreenText(HLP_TXT_LAPTOP_WEB_P1, usPosX, usPosY, usWidth);

      break;

    case HLP_SCRN_LPTP_FILES:

      // Display the first paragraph
      usTotalNumberOfVerticalPixels =
          GetAndDisplayHelpScreenText(HLP_TXT_LAPTOP_FILES_P1, usPosX, usPosY, usWidth);
      break;

    case HLP_SCRN_LPTP_HISTORY:
      // Display the first paragraph
      usTotalNumberOfVerticalPixels =
          GetAndDisplayHelpScreenText(HLP_TXT_LAPTOP_HISTORY_P1, usPosX, usPosY, usWidth);

      break;

    case HLP_SCRN_LPTP_PERSONNEL:

      // Display the first paragraph
      usTotalNumberOfVerticalPixels =
          GetAndDisplayHelpScreenText(HLP_TXT_LAPTOP_PERSONNEL_P1, usPosX, usPosY, usWidth);
      break;

    case HLP_SCRN_LPTP_FINANCIAL:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 2; ubCnt++) {
        usNumVertPixels =
            GetAndDisplayHelpScreenText(HLP_TXT_FINANCES_P1 + ubCnt, usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;

    case HLP_SCRN_LPTP_MERC_STATS:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 15; ubCnt++) {
        usNumVertPixels =
            GetAndDisplayHelpScreenText(HLP_TXT_MERC_STATS_P1 + ubCnt, usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;
  }

  return (usTotalNumberOfVerticalPixels);
}

uint16_t RenderMapScreenNoOneHiredYetHelpScreen() {
  uint16_t usPosX, usPosY, usWidth, usNumVertPixels;
  uint8_t ubCnt;
  uint16_t usTotalNumberOfVerticalPixels = 0;
  uint16_t usFontHeight = GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT);

  if (gHelpScreen.bCurrentHelpScreenActiveSubPage == -1) {
    return (0);
  }

  // Get the position for the text
  GetHelpScreenTextPositions(&usPosX, &usPosY, &usWidth);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    case HLP_SCRN_NO_ONE_HIRED:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 2; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_MPSCRN_NO_1_HIRED_YET_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;
  }

  return (usTotalNumberOfVerticalPixels);
}

uint16_t RenderMapScreenNotYetInArulcoHelpScreen() {
  uint16_t usPosX, usPosY, usWidth, usNumVertPixels;
  uint8_t ubCnt;
  uint16_t usTotalNumberOfVerticalPixels = 0;
  uint16_t usFontHeight = GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT);

  if (gHelpScreen.bCurrentHelpScreenActiveSubPage == -1) {
    return (0);
  }

  // Get the position for the text
  GetHelpScreenTextPositions(&usPosX, &usPosY, &usWidth);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    case HLP_SCRN_NOT_IN_ARULCO:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 3; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_MPSCRN_NOT_IN_ARULCO_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;
  }

  return (usTotalNumberOfVerticalPixels);
}

uint16_t RenderMapScreenSectorInventoryHelpScreen() {
  uint16_t usPosX, usPosY, usWidth, usNumVertPixels;
  uint8_t ubCnt;
  uint16_t usTotalNumberOfVerticalPixels = 0;
  uint16_t usFontHeight = GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT);

  if (gHelpScreen.bCurrentHelpScreenActiveSubPage == -1) {
    return (0);
  }

  // Get the position for the text
  GetHelpScreenTextPositions(&usPosX, &usPosY, &usWidth);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    case HLP_SCRN_MPSCRN_SCTR_OVERVIEW:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 2; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_SECTOR_INVTRY_OVERVIEW_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;
  }

  return (usTotalNumberOfVerticalPixels);
}

uint16_t RenderTacticalHelpScreen() {
  uint16_t usPosX, usPosY, usWidth, usNumVertPixels;
  uint8_t ubCnt;
  uint16_t usTotalNumberOfVerticalPixels = 0;
  uint16_t usFontHeight = GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT);

  if (gHelpScreen.bCurrentHelpScreenActiveSubPage == -1) {
    return (0);
  }

  // Get the position for the text
  GetHelpScreenTextPositions(&usPosX, &usPosY, &usWidth);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    case HLP_SCRN_TACTICAL_OVERVIEW:

      // Display all the paragraph
      for (ubCnt = 0; ubCnt < 4; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_TACTICAL_OVERVIEW_P1 + ubCnt, usPosX,
                                                      usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_TACTICAL_MOVEMENT:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 4; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_TACTICAL_MOVEMENT_P1 + ubCnt, usPosX,
                                                      usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_TACTICAL_SIGHT:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 4; ubCnt++) {
        usNumVertPixels =
            GetAndDisplayHelpScreenText(HLP_TXT_TACTICAL_SIGHT_P1 + ubCnt, usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;

    case HLP_SCRN_TACTICAL_ATTACKING:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 3; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_TACTICAL_ATTACKING_P1 + ubCnt, usPosX,
                                                      usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_TACTICAL_ITEMS:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 4; ubCnt++) {
        usNumVertPixels =
            GetAndDisplayHelpScreenText(HLP_TXT_TACTICAL_ITEMS_P1 + ubCnt, usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;

    case HLP_SCRN_TACTICAL_KEYBOARD:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 8; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_TACTICAL_KEYBOARD_P1 + ubCnt, usPosX,
                                                      usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;
  }

  return (usTotalNumberOfVerticalPixels);
}

uint16_t RenderMapScreenHelpScreen() {
  uint16_t usPosX, usPosY, usWidth, usNumVertPixels;
  uint8_t ubCnt;
  uint16_t usTotalNumberOfVerticalPixels = 0;
  uint16_t usFontHeight = GetFontHeight(HELP_SCREEN_TEXT_BODY_FONT);

  if (gHelpScreen.bCurrentHelpScreenActiveSubPage == -1) {
    return (0);
  }

  // Get the position for the text
  GetHelpScreenTextPositions(&usPosX, &usPosY, &usWidth);

  // switch on the current screen
  switch (gHelpScreen.bCurrentHelpScreenActiveSubPage) {
    case HLP_SCRN_MPSCRN_OVERVIEW:

      // Display all the paragraph
      for (ubCnt = 0; ubCnt < 3; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_OVERVIEW_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_MPSCRN_ASSIGNMENTS:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 4; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_ASSNMNT_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_MPSCRN_DESTINATIONS:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 5; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(
            HLP_TXT_WELCOM_TO_ARULCO_DSTINATION_P1 + ubCnt, usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;

    case HLP_SCRN_MPSCRN_MAP:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 3; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_MAP_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_MPSCRN_MILITIA:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 3; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_MILITIA_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }

      break;

    case HLP_SCRN_MPSCRN_AIRSPACE:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 2; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_AIRSPACE_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_MPSCRN_ITEMS:

      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 1; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_ITEMS_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;

    case HLP_SCRN_MPSCRN_KEYBOARD:
      // Display all the paragraphs
      for (ubCnt = 0; ubCnt < 4; ubCnt++) {
        usNumVertPixels = GetAndDisplayHelpScreenText(HLP_TXT_WELCOM_TO_ARULCO_KEYBOARD_P1 + ubCnt,
                                                      usPosX, usPosY, usWidth);

        // move the next text down by the right amount
        usPosY = usPosY + usNumVertPixels + usFontHeight;

        // add the total amount of pixels used
        usTotalNumberOfVerticalPixels += usNumVertPixels + usFontHeight;
      }
      break;
  }

  return (usTotalNumberOfVerticalPixels);
}

void RefreshAllHelpScreenButtons() {
  uint8_t i;

  // loop through all the buttons, and refresh them
  for (i = 0; i < gHelpScreen.bNumberOfButtons; i++) {
    ButtonList[guiHelpScreenBtns[i]]->uiFlags |= BUTTON_DIRTY;
  }

  ButtonList[guiHelpScreenExitBtn]->uiFlags |= BUTTON_DIRTY;

  if (!gHelpScreen.fForceHelpScreenToComeUp) {
    ButtonList[gHelpScreenDontShowHelpAgainToggle]->uiFlags |= BUTTON_DIRTY;
  }

  ButtonList[giHelpScreenScrollArrows[0]]->uiFlags |= BUTTON_DIRTY;
  ButtonList[giHelpScreenScrollArrows[1]]->uiFlags |= BUTTON_DIRTY;
}

int8_t HelpScreenDetermineWhichMapScreenHelpToShow() {
  if (fShowMapInventoryPool) {
    return (HELP_SCREEN_MAPSCREEN_SECTOR_INVENTORY);
  }

  if (AnyMercsHired() == FALSE) {
    return (HELP_SCREEN_MAPSCREEN_NO_ONE_HIRED);
  }

  if (gTacticalStatus.fDidGameJustStart) {
    return (HELP_SCREEN_MAPSCREEN_NOT_IN_ARULCO);
  }

  return (HELP_SCREEN_MAPSCREEN);
}

BOOLEAN CreateHelpScreenTextBuffer() {
  // Create a background video surface to blt the face onto
  vsHelpScreenTextBufferSurface =
      JSurface_Create16bpp(HLP_SCRN__WIDTH_OF_TEXT_BUFFER, HLP_SCRN__HEIGHT_OF_TEXT_BUFFER);
  JSurface_SetColorKey(vsHelpScreenTextBufferSurface, FROMRGB(0, 0, 0));

  return (TRUE);
}

void DestroyHelpScreenTextBuffer() {
  JSurface_Free(vsHelpScreenTextBufferSurface);
  vsHelpScreenTextBufferSurface = NULL;
}

void RenderCurrentHelpScreenTextToBuffer() {
  // clear the buffer ( use 0, black as a transparent color
  ClearHelpScreenTextBuffer();

  // Render the current screen, and get the number of pixels it used to display
  gHelpScreen.usTotalNumberOfPixelsInBuffer = RenderSpecificHelpScreen();

  // calc the number of lines in the buffer
  gHelpScreen.usTotalNumberOfLinesInBuffer =
      gHelpScreen.usTotalNumberOfPixelsInBuffer / (HLP_SCRN__HEIGHT_OF_1_LINE_IN_BUFFER);
}

void RenderTextBufferToScreen() {
  struct Rect SrcRect;

  SrcRect.left = 0;
  SrcRect.top = gHelpScreen.iLineAtTopOfTextBuffer * HLP_SCRN__HEIGHT_OF_1_LINE_IN_BUFFER;
  SrcRect.right = HLP_SCRN__WIDTH_OF_TEXT_BUFFER;
  SrcRect.bottom = SrcRect.top + HLP_SCRN__HEIGHT_OF_TEXT_AREA - (2 * 8);

  BltVSurfaceRectToPoint(vsFB, vsHelpScreenTextBufferSurface, gHelpScreen.usLeftMarginPosX,
                         (gHelpScreen.usScreenLocY + HELP_SCREEN_TEXT_OFFSET_Y), &SrcRect);

  DisplayHelpScreenTextBufferScrollBox();
}

void ChangeHelpScreenSubPage() {
  // reset
  gHelpScreen.iLineAtTopOfTextBuffer = 0;

  RenderCurrentHelpScreenTextToBuffer();

  // enable or disable the help screen arrow buttons
  if (gHelpScreen.usTotalNumberOfLinesInBuffer <= HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER) {
    DisableButton(giHelpScreenScrollArrows[0]);
    DisableButton(giHelpScreenScrollArrows[1]);
  } else {
    EnableButton(giHelpScreenScrollArrows[0]);
    EnableButton(giHelpScreenScrollArrows[1]);
  }
}

void ClearHelpScreenTextBuffer() {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // CLEAR THE FRAME BUFFER
  pDestBuf = LockVSurface(vsHelpScreenTextBufferSurface, &uiDestPitchBYTES);
  memset(pDestBuf, 0, HLP_SCRN__HEIGHT_OF_TEXT_BUFFER * uiDestPitchBYTES);
  JSurface_Unlock(vsHelpScreenTextBufferSurface);
  InvalidateScreen();
}

// - is up, + is down
void ChangeTopLineInTextBufferByAmount(int32_t iAmouontToMove) {
  // if we are moving up
  if (iAmouontToMove < 0) {
    if (gHelpScreen.iLineAtTopOfTextBuffer + iAmouontToMove >= 0) {
      // if we can move up by the requested amount
      if ((gHelpScreen.usTotalNumberOfLinesInBuffer - gHelpScreen.iLineAtTopOfTextBuffer) >
          iAmouontToMove) {
        gHelpScreen.iLineAtTopOfTextBuffer += iAmouontToMove;
      }

      // else, trying to move past the top
      else {
        gHelpScreen.iLineAtTopOfTextBuffer = 0;
      }
    } else {
      gHelpScreen.iLineAtTopOfTextBuffer = 0;
    }
  }

  // else we are moving down
  else {
    // if we dont have to scroll cause there is not enough text
    if (gHelpScreen.usTotalNumberOfLinesInBuffer <=
        HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER) {
      gHelpScreen.iLineAtTopOfTextBuffer = 0;
    } else {
      if ((gHelpScreen.iLineAtTopOfTextBuffer + HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER +
           iAmouontToMove) <= gHelpScreen.usTotalNumberOfLinesInBuffer) {
        gHelpScreen.iLineAtTopOfTextBuffer += iAmouontToMove;
      } else {
        gHelpScreen.iLineAtTopOfTextBuffer = gHelpScreen.usTotalNumberOfLinesInBuffer -
                                             HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER;
      }
    }
  }

  //	RenderCurrentHelpScreenTextToBuffer();

  gHelpScreen.ubHelpScreenDirty = HLP_SCRN_DRTY_LVL_REFRESH_TEXT;
}

void DisplayHelpScreenTextBufferScrollBox() {
  int32_t iSizeOfBox;
  int32_t iTopPosScrollBox = 0;
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;
  uint16_t usPosX;

  if (gHelpScreen.bNumberOfButtons != 0) {
    usPosX = gHelpScreen.usScreenLocX + HLP_SCRN__SCROLL_POSX + HELP_SCREEN_BUTTON_BORDER_WIDTH;
  } else {
    usPosX = gHelpScreen.usScreenLocX + HLP_SCRN__SCROLL_POSX;
  }

  //
  // first calculate the height of the scroll box
  //

  CalculateHeightAndPositionForHelpScreenScrollBox(&iSizeOfBox, &iTopPosScrollBox);

  //
  // next draw the box
  //

  // if there ARE scroll bars, draw the
  if (!(gHelpScreen.usTotalNumberOfLinesInBuffer <=
        HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER)) {
    ColorFillVSurfaceArea(vsFB, usPosX, iTopPosScrollBox, usPosX + HLP_SCRN__WIDTH_OF_SCROLL_AREA,
                          iTopPosScrollBox + iSizeOfBox - 1, rgb32_to_rgb16(FROMRGB(227, 198, 88)));

    // display the line
    pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

    // draw the gold highlite line on the top and left
    LineDraw(FALSE, usPosX, iTopPosScrollBox, usPosX + HLP_SCRN__WIDTH_OF_SCROLL_AREA,
             iTopPosScrollBox, rgb32_to_rgb16(FROMRGB(235, 222, 171)), pDestBuf);
    LineDraw(FALSE, usPosX, iTopPosScrollBox, usPosX, iTopPosScrollBox + iSizeOfBox - 1,
             rgb32_to_rgb16(FROMRGB(235, 222, 171)), pDestBuf);

    // draw the shadow line on the bottom and right
    LineDraw(FALSE, usPosX, iTopPosScrollBox + iSizeOfBox - 1,
             usPosX + HLP_SCRN__WIDTH_OF_SCROLL_AREA, iTopPosScrollBox + iSizeOfBox - 1,
             rgb32_to_rgb16(FROMRGB(65, 49, 6)), pDestBuf);
    LineDraw(FALSE, usPosX + HLP_SCRN__WIDTH_OF_SCROLL_AREA, iTopPosScrollBox,
             usPosX + HLP_SCRN__WIDTH_OF_SCROLL_AREA, iTopPosScrollBox + iSizeOfBox - 1,
             rgb32_to_rgb16(FROMRGB(65, 49, 6)), pDestBuf);

    // unlock frame buffer
    JSurface_Unlock(vsFB);
  }
}

void CreateScrollAreaButtons() {
  uint16_t usPosX, usWidth, usPosY;
  int32_t iPosY, iHeight;

  if (gHelpScreen.bNumberOfButtons != 0) {
    usPosX = gHelpScreen.usScreenLocX + HLP_SCRN__SCROLL_POSX + HELP_SCREEN_BUTTON_BORDER_WIDTH;
  } else {
    usPosX = gHelpScreen.usScreenLocX + HLP_SCRN__SCROLL_POSX;
  }

  usWidth = HLP_SCRN__WIDTH_OF_SCROLL_AREA;

  // Get the height and position of the scroll box
  CalculateHeightAndPositionForHelpScreenScrollBox(&iHeight, &iPosY);

  // Create a mouse region 'mask' the entrire screen
  MSYS_DefineRegion(&gHelpScreenScrollArea, usPosX, (uint16_t)iPosY, (uint16_t)(usPosX + usWidth),
                    (uint16_t)(iPosY + HLP_SCRN__HEIGHT_OF_SCROLL_AREA), MSYS_PRIORITY_HIGHEST,
                    gHelpScreen.usCursor, SelectHelpScrollAreaMovementCallBack,
                    SelectHelpScrollAreaCallBack);
  MSYS_AddRegion(&gHelpScreenScrollArea);

  guiHelpScreenScrollArrowImage[0] =
      LoadButtonImage("INTERFACE\\HelpScreen.sti", 14, 10, 11, 12, 13);
  guiHelpScreenScrollArrowImage[1] =
      UseLoadedButtonImage(guiHelpScreenScrollArrowImage[0], 19, 15, 16, 17, 18);

  if (gHelpScreen.bNumberOfButtons != 0)
    usPosX =
        gHelpScreen.usScreenLocX + HLP_SCRN__SCROLL_UP_ARROW_X + HELP_SCREEN_BUTTON_BORDER_WIDTH;
  else
    usPosX = gHelpScreen.usScreenLocX + HLP_SCRN__SCROLL_UP_ARROW_X;

  usPosY = gHelpScreen.usScreenLocY + HLP_SCRN__SCROLL_UP_ARROW_Y;

  // Create the scroll arrows
  giHelpScreenScrollArrows[0] = QuickCreateButton(
      guiHelpScreenScrollArrowImage[0], usPosX, usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
      DEFAULT_MOVE_CALLBACK, BtnHelpScreenScrollArrowsCallback);
  MSYS_SetBtnUserData(giHelpScreenScrollArrows[0], 0, 0);
  SetButtonCursor(giHelpScreenScrollArrows[0], gHelpScreen.usCursor);

  usPosY = gHelpScreen.usScreenLocY + HLP_SCRN__SCROLL_DWN_ARROW_Y;

  // Create the scroll arrows
  giHelpScreenScrollArrows[1] = QuickCreateButton(
      guiHelpScreenScrollArrowImage[1], usPosX, usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
      DEFAULT_MOVE_CALLBACK, BtnHelpScreenScrollArrowsCallback);
  MSYS_SetBtnUserData(giHelpScreenScrollArrows[1], 0, 1);
  SetButtonCursor(giHelpScreenScrollArrows[1], gHelpScreen.usCursor);
}

void DeleteScrollArrowButtons() {
  int8_t i;
  // remove the mouse region that blankets
  MSYS_RemoveRegion(&gHelpScreenScrollArea);

  for (i = 0; i < 2; i++) {
    RemoveButton(giHelpScreenScrollArrows[i]);
    UnloadButtonImage(guiHelpScreenScrollArrowImage[i]);
  }
}

void CalculateHeightAndPositionForHelpScreenScrollBox(int32_t *piHeightOfScrollBox,
                                                      int32_t *piTopOfScrollBox) {
  int32_t iSizeOfBox, iTopPosScrollBox;
  float dPercentSizeOfBox = 0;
  float dTemp = 0;

  dPercentSizeOfBox = HLP_SCRN__MAX_NUMBER_DISPLAYED_LINES_IN_BUFFER /
                      (float)gHelpScreen.usTotalNumberOfLinesInBuffer;

  // if the # is >= 1 then the box is the full size of the scroll area
  if (dPercentSizeOfBox >= 1.0) {
    iSizeOfBox = HLP_SCRN__HEIGHT_OF_SCROLL_AREA;

    // no need to calc the top spot for the box
    iTopPosScrollBox = HLP_SCRN__SCROLL_POSY;
  } else {
    iSizeOfBox = (int32_t)(dPercentSizeOfBox * HLP_SCRN__HEIGHT_OF_SCROLL_AREA + 0.5);

    //
    // next, calculate the top position of the box
    //
    dTemp = (HLP_SCRN__HEIGHT_OF_SCROLL_AREA / (float)gHelpScreen.usTotalNumberOfLinesInBuffer) *
            gHelpScreen.iLineAtTopOfTextBuffer;

    iTopPosScrollBox = (int32_t)(dTemp + .5) + HLP_SCRN__SCROLL_POSY;
  }

  if (piHeightOfScrollBox != NULL) *piHeightOfScrollBox = iSizeOfBox;

  if (piTopOfScrollBox != NULL) *piTopOfScrollBox = iTopPosScrollBox;
}

void SelectHelpScrollAreaCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gfScrollBoxIsScrolling = FALSE;
    gHelpScreen.iLastMouseClickY = -1;
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    gfScrollBoxIsScrolling = TRUE;
    HelpScreenMouseMoveScrollBox(pRegion->MouseYPos);
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void SelectHelpScrollAreaMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    //		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY,
    // pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
  } else if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    if (gfLeftButtonState) {
      HelpScreenMouseMoveScrollBox(pRegion->MouseYPos);
    }
  }
}

void HelpScreenMouseMoveScrollBox(int32_t usMousePosY) {
  int32_t iPosY, iHeight;
  int32_t iNumberOfIncrements = 0;
  float dSizeOfIncrement =
      (HLP_SCRN__HEIGHT_OF_SCROLL_AREA / (float)gHelpScreen.usTotalNumberOfLinesInBuffer);
  float dTemp;
  int32_t iNewPosition;

  CalculateHeightAndPositionForHelpScreenScrollBox(&iHeight, &iPosY);

  if (AreWeClickingOnScrollBar(usMousePosY) || gHelpScreen.iLastMouseClickY != -1) {
    if (gHelpScreen.iLastMouseClickY == -1) gHelpScreen.iLastMouseClickY = usMousePosY;

    if (usMousePosY < gHelpScreen.iLastMouseClickY) {
      //			iNewPosition = iPosY - ( uint16_t)( dSizeOfIncrement + .5);
      iNewPosition = iPosY - (gHelpScreen.iLastMouseClickY - usMousePosY);

    } else if (usMousePosY > gHelpScreen.iLastMouseClickY) {
      //			iNewPosition = iPosY + ( uint16_t)( dSizeOfIncrement + .5);
      iNewPosition = iPosY + usMousePosY - gHelpScreen.iLastMouseClickY;
    } else {
      return;
    }

    dTemp = (iNewPosition - iPosY) / dSizeOfIncrement;

    if (dTemp < 0)
      iNumberOfIncrements = (int32_t)(dTemp - 0.5);
    else
      iNumberOfIncrements = (int32_t)(dTemp + 0.5);

    gHelpScreen.iLastMouseClickY = usMousePosY;

    //		return;
  } else {
    // if the mouse is higher then the top of the scroll area, set it to the top of the scroll area
    if (usMousePosY < HLP_SCRN__SCROLL_POSY) usMousePosY = HLP_SCRN__SCROLL_POSY;

    dTemp = (usMousePosY - iPosY) / dSizeOfIncrement;

    if (dTemp < 0)
      iNumberOfIncrements = (int32_t)(dTemp - 0.5);
    else
      iNumberOfIncrements = (int32_t)(dTemp + 0.5);
  }

  // if there has been a change
  if (iNumberOfIncrements != 0) {
    ChangeTopLineInTextBufferByAmount(iNumberOfIncrements);
  }
}

void BtnHelpScreenScrollArrowsCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    int32_t iButtonID = MSYS_GetBtnUserData(btn, 0);

    btn->uiFlags |= BUTTON_CLICKED_ON;

    // if up
    if (iButtonID == 0) {
      ChangeTopLineInTextBufferByAmount(-1);
    } else {
      ChangeTopLineInTextBufferByAmount(1);
    }

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    int32_t iButtonID = MSYS_GetBtnUserData(btn, 0);

    // if up
    if (iButtonID == 0) {
      ChangeTopLineInTextBufferByAmount(-1);
    } else {
      ChangeTopLineInTextBufferByAmount(1);
    }

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

BOOLEAN AreWeClickingOnScrollBar(int32_t usMousePosY) {
  int32_t iPosY, iHeight;

  CalculateHeightAndPositionForHelpScreenScrollBox(&iHeight, &iPosY);

  if (usMousePosY >= iPosY && usMousePosY < (iPosY + iHeight))
    return (TRUE);
  else
    return (FALSE);
}
