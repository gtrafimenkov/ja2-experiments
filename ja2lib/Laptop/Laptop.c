// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/Laptop.h"

#include "Cheats.h"
#include "GameLoop.h"
#include "HelpScreen.h"
#include "Laptop/AIM.h"
#include "Laptop/AIMArchives.h"
#include "Laptop/AIMFacialIndex.h"
#include "Laptop/AIMHistory.h"
#include "Laptop/AIMLinks.h"
#include "Laptop/AIMMembers.h"
#include "Laptop/AIMPolicies.h"
#include "Laptop/AIMSort.h"
#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRAmmo.h"
#include "Laptop/BobbyRArmour.h"
#include "Laptop/BobbyRGuns.h"
#include "Laptop/BobbyRMailOrder.h"
#include "Laptop/BobbyRMisc.h"
#include "Laptop/BobbyRShipments.h"
#include "Laptop/BobbyRUsed.h"
#include "Laptop/BrokenLink.h"
#include "Laptop/CharProfile.h"
#include "Laptop/Email.h"
#include "Laptop/Files.h"
#include "Laptop/Finances.h"
#include "Laptop/Florist.h"
#include "Laptop/FloristCards.h"
#include "Laptop/FloristGallery.h"
#include "Laptop/FloristOrderForm.h"
#include "Laptop/Funeral.h"
#include "Laptop/History.h"
#include "Laptop/Insurance.h"
#include "Laptop/InsuranceComments.h"
#include "Laptop/InsuranceContract.h"
#include "Laptop/InsuranceInfo.h"
#include "Laptop/Mercs.h"
#include "Laptop/MercsAccount.h"
#include "Laptop/MercsFiles.h"
#include "Laptop/MercsNoAccount.h"
#include "Laptop/Personnel.h"
#include "Laptop/SirTech.h"
#include "Money.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "Screens.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/GameInit.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicStatus.h"
#include "SysGlobals.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/MercHiring.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/AmbientControl.h"
#include "TileEngine/Environment.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/Message.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "platform.h"

// icons text id's
enum {
  MAIL = 0,
  WWW,
  FINANCIAL,
  PERSONNEL,
  HISTORY,
  FILES,
  MAX_ICON_COUNT,
};

enum {
  NO_REGION = 0,
  EMAIL_REGION,
  WWW_REGION,
  FINANCIAL_REGION,
  PERSONNEL_REGION,
  HISTORY_REGION,
  FILES_REGION,
};

struct rgbcolor {
  uint8_t ubRed;
  uint8_t ubGreen;
  uint8_t ubBlue;
};

typedef struct rgbcolor RGBCOLOR;

RGBCOLOR GlowColors[] = {
    {0, 0, 0},   {25, 0, 0},  {50, 0, 0},  {75, 0, 0},  {100, 0, 0}, {125, 0, 0},
    {150, 0, 0}, {175, 0, 0}, {200, 0, 0}, {225, 0, 0}, {250, 0, 0},
};

// laptop programs
enum {
  LAPTOP_PROGRAM_MAILER,
  LAPTOP_PROGRAM_WEB_BROWSER,
  LAPTOP_PROGRAM_FILES,
  LAPTOP_PROGRAM_PERSONNEL,
  LAPTOP_PROGRAM_FINANCES,
  LAPTOP_PROGRAM_HISTORY,

};

// laptop program states
enum {
  LAPTOP_PROGRAM_MINIMIZED,
  LAPTOP_PROGRAM_OPEN,
};
#define LAPTOP_ICONS_X 33
#define LAPTOP_ICONS_MAIL_Y 35 - 5
#define LAPTOP_ICONS_WWW_Y 102 - 10 - 5
#define LAPTOP_ICONS_FINANCIAL_Y 172 - 10 - 5
#define LAPTOP_ICONS_PERSONNEL_Y 263 - 20 - 5
#define LAPTOP_ICONS_HISTORY_Y 310 - 5
#define LAPTOP_ICONS_FILES_Y 365 - 5 - 5
#define LAPTOP_ICON_TEXT_X 24
#define LAPTOP_ICON_TEXT_WIDTH 103 - 24
#define LAPTOP_ICON_TEXT_HEIGHT 6
#define LAPTOP_ICON_TEXT_MAIL_Y 82 - 5
#define LAPTOP_ICON_TEXT_WWW_Y 153 + 4 - 10 - 5
#define LAPTOP_ICON_TEXT_FINANCIAL_Y 229 - 10 - 5
#define LAPTOP_ICON_TEXT_PERSONNEL_Y 291 + 5 + 5 - 5
#define LAPTOP_ICON_TEXT_HISTORY_Y 346 + 10 + 5 - 5
#define LAPTOP_ICON_TEXT_FILES_Y 412 + 5 + 3 - 5
#define LAPTOPICONFONT FONT10ARIAL
#define BOOK_FONT FONT10ARIAL
#define DOWNLOAD_FONT FONT12ARIAL
#define ERROR_TITLE_FONT FONT14ARIAL
#define ERROR_FONT FONT12ARIAL

#define HISTORY_ICON_OFFSET_X 0
#define FILES_ICON_OFFSET_X 3
#define FINANCIAL_ICON_OFFSET_X 0
#define LAPTOP_ICON_WIDTH 80
#define MAX_BUTTON_COUNT 1
#define ON_BUTTON 0
#define GLOW_DELAY 70
#define WWW_COUNT 6
#define ICON_INTERVAL 150
#define BOOK_X 111
#define BOOK_TOP_Y 79
#define BOOK_HEIGHT 12
#define DOWN_HEIGHT 19
#define BOOK_WIDTH 100
#define SCROLL_MIN -100
#define SCROLL_DIFFERENCE 10

#define LONG_UNIT_TIME 120
#define UNIT_TIME 40
#define LOAD_TIME UNIT_TIME * 30
#define FAST_UNIT_TIME 3
#define FASTEST_UNIT_TIME 2
#define ALMOST_FAST_UNIT_TIME 25
#define ALMOST_FAST_LOAD_TIME ALMOST_FAST_UNIT_TIME * 30
#define FAST_LOAD_TIME FAST_UNIT_TIME * 30
#define LONG_LOAD_TIME LONG_UNIT_TIME * 30
#define FASTEST_LOAD_TIME FASTEST_UNIT_TIME * 30
#define DOWNLOAD_X 300
#define DOWNLOAD_Y 200
#define LAPTOP_WINDOW_X DOWNLOAD_X + 12
#define LAPTOP_WINDOW_Y DOWNLOAD_Y + 25
#define LAPTOP_BAR_Y LAPTOP_WINDOW_Y + 2
#define LAPTOP_BAR_X LAPTOP_WINDOW_X + 1
#define UNIT_WIDTH 4
#define LAPTOP_WINDOW_WIDTH 331 - 181
#define LAPTOP_WINDOW_HEIGHT 240 - 190
#define DOWN_STRING_X DOWNLOAD_X + 47
#define DOWN_STRING_Y DOWNLOAD_Y + 5
#define ERROR_X 300
#define ERROR_Y 200
#define ERROR_BTN_X 43
#define ERROR_BTN_Y ERROR_Y + 70
#define ERROR_TITLE_X ERROR_X + 3
#define ERROR_TITLE_Y ERROR_Y + 3
#define ERROR_BTN_TEXT_X 20
#define ERROR_BTN_TEXT_Y 9
#define ERROR_TEXT_X 0
#define ERROR_TEXT_Y 15
#define LAPTOP_TITLE_ICONS_X 113
#define LAPTOP_TITLE_ICONS_Y 27

// HD flicker times
#define HD_FLICKER_TIME 3000
#define FLICKER_TIME 50

#define NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS 18
#define LAPTOP_TITLE_BAR_WIDTH 500
#define LAPTOP_TITLE_BAR_HEIGHT 24

#define LAPTOP_TITLE_BAR_TOP_LEFT_X 111
#define LAPTOP_TITLE_BAR_TOP_LEFT_Y 25
#define LAPTOP_TITLE_BAR_TOP_RIGHT_X 610
#define LAPTOP_TITLE_BAR_TOP_RIGHT_Y LAPTOP_TITLE_BAR_TOP_LEFT_Y

#define LAPTOP_TITLE_BAR_ICON_OFFSET_X 5
#define LAPTOP_TITLE_BAR_ICON_OFFSET_Y 2
#define LAPTOP_TITLE_BAR_TEXT_OFFSET_X 29  // 18
#define LAPTOP_TITLE_BAR_TEXT_OFFSET_Y 8

#define LAPTOP_PROGRAM_ICON_X LAPTOP_TITLE_BAR_TOP_LEFT_X
#define LAPTOP_PROGRAM_ICON_Y LAPTOP_TITLE_BAR_TOP_LEFT_Y
#define LAPTOP_PROGRAM_ICON_WIDTH 20
#define LAPTOP_PROGRAM_ICON_HEIGHT 20
#define DISPLAY_TIME_FOR_WEB_BOOKMARK_NOTIFY 2000

// the wait time for closing of laptop animation/delay
#define EXIT_LAPTOP_DELAY_TIME 100

static struct JSurface *vsTitleBarSurface;
BOOLEAN gfTitleBarSurfaceAlreadyActive = FALSE;

#define LAPTOP__NEW_FILE_ICON_X 83
#define LAPTOP__NEW_FILE_ICON_Y 412  //(405+19)

#define LAPTOP__NEW_EMAIL_ICON_X (83 - 16)
#define LAPTOP__NEW_EMAIL_ICON_Y LAPTOP__NEW_FILE_ICON_Y

// Mode values
uint32_t guiCurrentLaptopMode;
uint32_t guiPreviousLaptopMode;
uint32_t guiCurrentWWWMode = LAPTOP_MODE_NONE;
int32_t giCurrentSubPage;
uint32_t guiCurrentLapTopCursor;
uint32_t guiPreviousLapTopCursor;
uint32_t guiCurrentSidePanel;  // the current navagation panel on the leftside of the laptop screen
uint32_t guiPreviousSidePanel;

int32_t iHighLightBookLine = -1;
BOOLEAN fFastLoadFlag = FALSE;
BOOLEAN gfSideBarFlag;
BOOLEAN gfEnterLapTop = TRUE;
BOOLEAN gfShowBookmarks = FALSE;

// in progress of loading a page?
BOOLEAN fLoadPendingFlag = FALSE;
BOOLEAN fErrorFlag;

// mark buttons dirty?
BOOLEAN fMarkButtonsDirtyFlag = TRUE;

// redraw afer rendering buttons?
BOOLEAN fReDrawPostButtonRender = FALSE;

// intermediate refresh flag
BOOLEAN fIntermediateReDrawFlag = FALSE;

// in laptop right now?
BOOLEAN fCurrentlyInLaptop = FALSE;

// exit due to a message box pop up?..don't really leave LAPTOP
BOOLEAN fExitDueToMessageBox = FALSE;

// have we visited IMP yety?
BOOLEAN fNotVistedImpYet = TRUE;

// exit laptop during a load?
BOOLEAN fExitDuringLoad = FALSE;

// done loading?
BOOLEAN fDoneLoadPending = FALSE;

// re connecting to a web page?
BOOLEAN fReConnectingFlag = FALSE;

// going a subpage of a web page?..faster access time
BOOLEAN fConnectingToSubPage = FALSE;

// is this our first time in laptop?
BOOLEAN fFirstTimeInLaptop = TRUE;

// redraw the book mark info panel .. for blitting on top of animations
BOOLEAN fReDrawBookMarkInfo = FALSE;

// show the 2 second info about bookmarks being accessed by clicking on web
BOOLEAN fShowBookmarkInfo = FALSE;

// show start button for ATM panel?
extern BOOLEAN fShowAtmPanelStartButton;

// TEMP!	Disables the loadpending delay when switching b/n www pages
BOOLEAN gfTemporaryDisablingOfLoadPendingFlag = FALSE;

// GLOBAL FOR WHICH SCREEN TO EXIT TO FOR LAPTOP
uint32_t guiExitScreen = MAP_SCREEN;

struct MOUSE_REGION gLaptopRegion;
// Laptop screen graphic handle
uint32_t guiLAPTOP;
BOOLEAN fNewWWWDisplay = TRUE;

static BOOLEAN fNewWWW = TRUE;

// Used to store the site to go to after the 'rain delay' message
extern uint32_t guiRainLoop;

int32_t giRainDelayInternetSite = -1;

// have we visitied this site already?
// BOOLEAN fVisitedBookmarkAlready[20];

// the laptop icons
uint32_t guiFILESICON;
uint32_t guiFINANCIALICON;
uint32_t guiHISTORYICON;
uint32_t guiMAILICON;
uint32_t guiPERSICON;
uint32_t guiWWWICON;
uint32_t guiBOOKTOP;
uint32_t guiBOOKHIGH;
uint32_t guiBOOKMID;
uint32_t guiBOOKBOT;
uint32_t guiBOOKMARK;
uint32_t guiGRAPHWINDOW;
uint32_t guiGRAPHBAR;
uint32_t guiLaptopBACKGROUND;
uint32_t guiDOWNLOADTOP;
uint32_t guiDOWNLOADMID;
uint32_t guiDOWNLOADBOT;
uint32_t guiTITLEBARLAPTOP;
uint32_t guiLIGHTS;
uint32_t guiTITLEBARICONS;
static struct JSurface *vsDESKTOP;

// email notification
uint32_t guiUNREAD;
uint32_t guiNEWMAIL;

// laptop button
uint32_t guiLAPTOPBUTTON;
// the sidepanel handle
uint32_t guiLAPTOPSIDEPANEL;

// BOOLEAN		gfNewGameLaptop = TRUE;

// enter new laptop mode due to sliding bars
BOOLEAN fEnteredNewLapTopDueToHandleSlidingBars = FALSE;

// laptop pop up messages index value
int32_t iLaptopMessageBox = -1;

// whether or not we are initing the slide in title bar
BOOLEAN fInitTitle = TRUE;

// tab handled
BOOLEAN fTabHandled = FALSE;

// are we maxing or mining?
BOOLEAN fForward = TRUE;

// BUTTON IMAGES
int32_t giLapTopButton[MAX_BUTTON_COUNT];
int32_t giLapTopButtonImage[MAX_BUTTON_COUNT];
int32_t giErrorButton[1];
int32_t giErrorButtonImage[1];

int32_t gLaptopButton[7];
int32_t gLaptopButtonImage[7];

// minimize button
int32_t gLaptopMinButton[1];
int32_t gLaptopMinButtonImage[1];

int32_t gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY + 1];

// process of mazimizing
BOOLEAN fMaximizingProgram = FALSE;

// program we are maximizing
int8_t bProgramBeingMaximized = -1;

// are we minimizing
BOOLEAN fMinizingProgram = FALSE;

// process openned queue
int32_t gLaptopProgramQueueList[6];

// state of createion of minimize button
BOOLEAN fCreateMinimizeButton = FALSE;

BOOLEAN fExitingLaptopFlag = FALSE;

// HD and power lights on
BOOLEAN fPowerLightOn = TRUE;
BOOLEAN fHardDriveLightOn = FALSE;

// HD flicker
BOOLEAN fFlickerHD = FALSE;

// the screens limiting rect
SGPRect LaptopScreenRect = {LAPTOP_UL_X, LAPTOP_UL_Y - 5, LAPTOP_SCREEN_LR_X + 2,
                            LAPTOP_SCREEN_LR_Y + 5 + 19};

// the sub pages vistsed or not status within the web browser
BOOLEAN gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_SIRTECH - LAPTOP_MODE_WWW];

// int32_t iBookMarkList[MAX_BOOKMARKS];

// mouse regions
struct MOUSE_REGION gEmailRegion;
struct MOUSE_REGION gWWWRegion;
struct MOUSE_REGION gFinancialRegion;
struct MOUSE_REGION gPersonnelRegion;
struct MOUSE_REGION gHistoryRegion;
struct MOUSE_REGION gFilesRegion;
struct MOUSE_REGION gLapTopScreenRegion;
struct MOUSE_REGION gBookmarkMouseRegions[MAX_BOOKMARKS];
struct MOUSE_REGION pScreenMask;
struct MOUSE_REGION gLapTopProgramMinIcon;
struct MOUSE_REGION gNewMailIconRegion;
struct MOUSE_REGION gNewFileIconRegion;

// highlighted mouse region
int32_t giHighLightRegion = NO_REGION;

// highlighted regions
int32_t giCurrentRegion = NO_REGION;
int32_t giOldRegion = NO_REGION;

// used for global variables that need to be saved
LaptopSaveInfoStruct LaptopSaveInfo;

// function calls
uint32_t RenderLaptopPanel();
void RenderLapTopImage();
void GetLaptopKeyboardInput();
uint32_t ExitLaptopMode(uint32_t uiMode);

uint32_t DrawLapTopText();
void BtnOnCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t CreateLaptopButtons();
void DeleteLapTopButtons();
BOOLEAN DeleteLapTopMouseRegions();
BOOLEAN CreateLapTopMouseRegions();
BOOLEAN fReDrawScreenFlag = FALSE;
BOOLEAN fPausedReDrawScreenFlag =
    FALSE;  // used in the handler functions to redraw the screen, after the current frame
void HandleLapTopScreenMouseUi();
void DrawHighLightRegionBox();
void HandleLapTopCursorUpDate();
void PrintBalance(void);

// callbacks
void FinancialRegionButtonCallback(GUI_BUTTON *btn, int32_t reason);
void PersonnelRegionButtonCallback(GUI_BUTTON *btn, int32_t reason);
void WWWRegionButtonCallback(GUI_BUTTON *btn, int32_t reason);
void EmailRegionButtonCallback(GUI_BUTTON *btn, int32_t reason);
void FilesRegionButtonCallback(GUI_BUTTON *btn, int32_t reason);
void HistoryRegionButtonCallback(GUI_BUTTON *btn, int32_t reason);
void LaptopProgramIconMinimizeCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void WWWRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void EmailRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void RestoreOldRegion(int32_t iOldRegion);
void HighLightRegion(int32_t iCurrentRegion);
void FinancialRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void HistoryRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void FilesRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void PersonnelRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void ScreenRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// minimize callback
void LaptopMinimizeProgramButtonCallback(GUI_BUTTON *btn, int32_t reason);

void NewFileIconCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void DisplayBookMarks();
void InitBookMarkList();
BOOLEAN LoadBookmark();
void DeleteBookmark();
void ScrollDisplayText(int32_t iY);
void BookmarkCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void CreateBookMarkMouseRegions();
void BookmarkMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void DeleteBookmarkRegions();
void DeleteLoadPending(void);
BOOLEAN LoadLoadPending(void);
BOOLEAN DisplayLoadPending(void);
void DisplayErrorBox(void);
void CreateDestroyErrorButton(void);
void DrawTextOnErrorButton();
BOOLEAN LeaveLapTopScreen();
void HandleAnimatedButtons(void);
void AnimateButton(uint32_t uiIconID, uint16_t usX, uint16_t usY);
void CreateDestoryBookMarkRegions(void);
void EnterLaptopInitLaptopPages();
void CheckMarkButtonsDirtyFlag(void);
void PostButtonRendering(void);
void ShouldNewMailBeDisplayed(void);
void DisplayPlayersBalanceToDate(void);
void CheckIfNewWWWW(void);
void HandleLapTopESCKey(void);
BOOLEAN InitTitleBarMaximizeGraphics(uint32_t uiBackgroundGraphic, wchar_t *pTitle,
                                     uint32_t uiIconGraphic, uint16_t usIconGraphicIndex);
void RemoveTitleBarMaximizeGraphics();
BOOLEAN DisplayTitleBarMaximizeGraphic(BOOLEAN fForward, BOOLEAN fInit, uint16_t usTopLeftX,
                                       uint16_t usTopLeftY, uint16_t usTopRightX);
void HandleSlidingTitleBar(void);
void ShowLights(void);
void FlickerHDLight(void);
BOOLEAN ExitLaptopDone(void);
void CreateDestroyMinimizeButtonForCurrentMode(void);
void CreateMinimizeButtonForCurrentMode(void);
void DestroyMinimizeButtonForCurrentMode(void);
void InitLaptopOpenQueue(void);
void UpdateListToReflectNewProgramOpened(int32_t iOpenedProgram);
int32_t FindLastProgramStillOpen(void);
void SetCurrentToLastProgramOpened(void);
BOOLEAN HandleExit(void);
void DeleteDesktopBackground(void);
BOOLEAN LoadDesktopBackground(void);
BOOLEAN DrawDeskTopBackground(void);
void PrintDate(void);
void DisplayTaskBarIcons();
void PrintNumberOnTeam(void);
void HandleDefaultWebpageForLaptop(void);
void CreateMinimizeRegionsForLaptopProgramIcons(void);
void DestroyMinimizeRegionsForLaptopProgramIcons(void);
void CreateDestroyMouseRegionForNewMailIcon(void);
void NewEmailIconCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void HandleWWWSubSites(void);
void UpdateStatusOfDisplayingBookMarks(void);
void InitalizeSubSitesList(void);
void SetSubSiteAsVisted(void);
void HandleAltTabKeyInLaptop(void);
void HandleShiftAltTabKeyInLaptop(void);

// display bookmark notify
void DisplayWebBookMarkNotify(void);

// handle timer for bookmark notify
void HandleWebBookMarkNotifyTimer(void);

void CreateBookMarkHelpText(struct MOUSE_REGION *pRegion, uint32_t uiBookMarkID);

void CreateFileAndNewEmailIconFastHelpText(uint32_t uiHelpTextID, BOOLEAN fClearHelpText);
void CreateLaptopButtonHelpText(int32_t iButtonIndex, uint32_t uiButtonHelpTextID);
// ppp

// Used to determine delay if its raining
BOOLEAN IsItRaining();
int32_t WWaitDelayIncreasedIfRaining(int32_t iLoadTime);
void InternetRainDelayMessageBoxCallBack(uint8_t bExitValue);

extern void ClearHistoryList(void);

// TEMP CHEAT
#ifdef JA2TESTVERSION
extern void CheatToGetAll5Merc();
#endif

void SetLaptopExitScreen(uint32_t uiExitScreen) { guiExitScreen = uiExitScreen; }

void SetLaptopNewGameFlag() { LaptopSaveInfo.gfNewGameLaptop = TRUE; }

void HandleLapTopCursorUpDate() {
  if (guiPreviousLapTopCursor == guiCurrentLapTopCursor) return;
  switch (guiCurrentLapTopCursor) {
    case LAPTOP_PANEL_CURSOR:
      MSYS_SetCurrentCursor(CURSOR_NORMAL);
      break;
    case LAPTOP_SCREEN_CURSOR:
      MSYS_SetCurrentCursor(CURSOR_LAPTOP_SCREEN);
      break;
    case LAPTOP_WWW_CURSOR:
      MSYS_SetCurrentCursor(CURSOR_WWW);
      break;
  }
  guiPreviousLapTopCursor = guiCurrentLapTopCursor;
}
void GetLaptopKeyboardInput() {
  InputAtom InputEvent;
  struct Point MousePos = GetMousePoint();

  fTabHandled = FALSE;

  while (DequeueEvent(&InputEvent) == TRUE) {
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
      case RIGHT_BUTTON_REPEAT:
        MouseSystemHook(RIGHT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case LEFT_BUTTON_REPEAT:
        MouseSystemHook(LEFT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
    }

    HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState);
  }
}

// This is called only once at game initialization.
uint32_t LaptopScreenInit() {
  // Memset the whole structure, to make sure of no 'JUNK'
  memset(&LaptopSaveInfo, 0, sizeof(LaptopSaveInfoStruct));

  LaptopSaveInfo.gfNewGameLaptop = TRUE;

  InitializeNumDaysMercArrive();

  // reset the id of the last hired merc
  LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;

  // reset the flag that enables the 'just hired merc' popup
  LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop = FALSE;

  // Initialize all vars
  guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
  guiPreviousLaptopMode = LAPTOP_MODE_NONE;
  guiCurrentWWWMode = LAPTOP_MODE_NONE;
  guiCurrentSidePanel = FIRST_SIDE_PANEL;
  guiPreviousSidePanel = FIRST_SIDE_PANEL;

  gfSideBarFlag = FALSE;
  gfShowBookmarks = FALSE;
  InitBookMarkList();
  GameInitAIM();
  GameInitAIMMembers();
  GameInitAimFacialIndex();
  GameInitAimSort();
  GameInitAimArchives();
  GameInitAimPolicies();
  GameInitAimLinks();
  GameInitAimHistory();
  GameInitMercs();
  GameInitBobbyR();
  GameInitBobbyRAmmo();
  GameInitBobbyRArmour();
  GameInitBobbyRGuns();
  GameInitBobbyRMailOrder();
  GameInitBobbyRMisc();
  GameInitBobbyRUsed();
  GameInitEmail();
  GameInitCharProfile();
  GameInitFlorist();
  GameInitInsurance();
  GameInitInsuranceContract();
  GameInitFuneral();
  GameInitSirTech();
  GameInitFiles();
  GameInitPersonnel();

  // init program states
  memset(&gLaptopProgramStates, LAPTOP_PROGRAM_MINIMIZED, sizeof(gLaptopProgramStates));

  gfAtLeastOneMercWasHired = FALSE;

  // No longer inits the laptop screens, now InitLaptopAndLaptopScreens() does

  return (1);
}

BOOLEAN InitLaptopAndLaptopScreens() {
  GameInitFinances();
  GameInitHistory();

  // Reset the flag so we can create a new IMP character
  LaptopSaveInfo.fIMPCompletedFlag = FALSE;

  // Reset the flag so that BOBBYR's isnt available at the begining of the game
  LaptopSaveInfo.fBobbyRSiteCanBeAccessed = FALSE;

  return (TRUE);
}

uint32_t DrawLapTopIcons() { return (TRUE); }

uint32_t DrawLapTopText() {
  // show balance
  DisplayPlayersBalanceToDate();

  return (TRUE);
}

// This is only called once at game shutdown.
uint32_t LaptopScreenShutdown() {
  InsuranceContractEndGameShutDown();
  BobbyRayMailOrderEndGameShutDown();
  ShutDownEmailList();

  ClearHistoryList();

  return TRUE;
}

int32_t EnterLaptop() {
  // Create, load, initialize data -- just entered the laptop.

  // we are re entering due to message box, leave NOW!
  if (fExitDueToMessageBox == TRUE) {
    return (TRUE);
  }

  // if the radar map mouse region is still active, disable it.
  if (gRadarRegion.uiFlags & MSYS_REGION_ENABLED) {
    MSYS_DisableRegion(&gRadarRegion);
    /*
                    #ifdef JA2BETAVERSION
                            DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, L"Mapscreen's radar region
       is still active, please tell Dave how you entered Laptop.", LAPTOP_SCREEN, MSG_BOX_FLAG_OK,
       NULL ); #endif
    */
  }

  gfDontStartTransitionFromLaptop = FALSE;

  // Since we are coming in from MapScreen, uncheck the flag
  guiTacticalInterfaceFlags &= ~INTERFACE_MAPSCREEN;

  // ATE: Disable messages....
  DisableScrollMessages();

  // Stop any person from saying anything
  StopAnyCurrentlyTalkingSpeech();

  // Don't play music....
  SetMusicMode(MUSIC_LAPTOP);

  // Stop ambients...
  StopAmbients();

  // if its raining, start the rain showers
  if (IsItRaining()) {
    // Enable the rain delay warning
    giRainDelayInternetSite = -1;

    // lower the volume
    guiRainLoop = PlayJA2Ambient(RAIN_1, LOWVOLUME, 0);
  }

  // pause the game because we dont want time to advance in the laptop
  PauseGame();

  // set the fact we are currently in laptop, for rendering purposes
  fCurrentlyInLaptop = TRUE;

  // reset redraw flag and redraw new mail
  fReDrawScreenFlag = FALSE;
  fReDrawNewMailFlag = TRUE;

  // setup basic cursors
  guiCurrentLapTopCursor = LAPTOP_PANEL_CURSOR;
  guiPreviousLapTopCursor = LAPTOP_NO_CURSOR;

  // sub page
  giCurrentSubPage = 0;
  giCurrentRegion = EMAIL_REGION;

  // load the laptop graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\laptop3.sti"), &guiLAPTOP));

  // background for panel
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\taskbar.sti"), &guiLaptopBACKGROUND));

  // background for panel
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\programtitlebar.sti"), &guiTITLEBARLAPTOP));

  // lights for power and HD
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\lights.sti"), &guiLIGHTS));

  // icons for title bars
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\ICONS.sti"), &guiTITLEBARICONS));

  // load, blt and delete graphics
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\NewMailWarning.sti"), &guiEmailWarning));
  // load background
  LoadDesktopBackground();

  guiCurrentLaptopMode = LAPTOP_MODE_NONE;
  // MSYS_SetCurrentCursor(CURSOR_NORMAL);

  guiCurrentLaptopMode = LAPTOP_MODE_NONE;
  guiPreviousLaptopMode = LAPTOP_MODE_NONE;
  guiCurrentWWWMode = LAPTOP_MODE_NONE;
  guiCurrentSidePanel = FIRST_SIDE_PANEL;
  guiPreviousSidePanel = FIRST_SIDE_PANEL;
  gfSideBarFlag = FALSE;
  CreateLapTopMouseRegions();
  RenderLapTopImage();
  HighLightRegion(giCurrentRegion);
  // AddEmailMessage(L"Entered LapTop",L"Entered", 0, 0);
  // for(iCounter=0; iCounter <10; iCounter++)
  //{
  // AddEmail(3,5,0,0);
  //}
  // the laptop mouse region

  // reset bookmarks flags
  fFirstTimeInLaptop = TRUE;

  // reset all bookmark visits
  memset(&LaptopSaveInfo.fVisitedBookmarkAlready, 0,
         sizeof(LaptopSaveInfo.fVisitedBookmarkAlready));

  // init program states
  memset(&gLaptopProgramStates, LAPTOP_PROGRAM_MINIMIZED, sizeof(gLaptopProgramStates));

  // turn the power on
  fPowerLightOn = TRUE;

  // we are not exiting laptop right now, we just got here
  fExitingLaptopFlag = FALSE;

  // reset program we are maximizing
  bProgramBeingMaximized = -1;

  // reset fact we are maximizing/ mining
  fMaximizingProgram = FALSE;
  fMinizingProgram = FALSE;

  // initialize open queue
  InitLaptopOpenQueue();

  gfShowBookmarks = FALSE;
  LoadBookmark();
  SetBookMark(AIM_BOOKMARK);
  LoadLoadPending();

  DrawDeskTopBackground();

  // create region for new mail icon
  CreateDestroyMouseRegionForNewMailIcon();

  // DEF: Added to Init things in various laptop pages
  EnterLaptopInitLaptopPages();
  InitalizeSubSitesList();

  fShowAtmPanelStartButton = TRUE;

  InvalidateRegion(0, 0, 640, 480);

  return (TRUE);
}

void ExitLaptop() {
  // exit is called due to message box, leave
  if (fExitDueToMessageBox) {
    fExitDueToMessageBox = FALSE;
    return;
  }

  if (DidGameJustStart()) {
    SetMusicMode(MUSIC_LAPTOP);
  } else {
    // Restore to old stuff...
    SetMusicMode(MUSIC_RESTORE);
  }

  // Start ambients...
  BuildDayAmbientSounds();

  // if its raining, start the rain showers
  if (IsItRaining()) {
    // Raise the volume to where it was
    guiRainLoop = PlayJA2Ambient(RAIN_1, MIDVOLUME, 0);
  }

  // release cursor
  FreeMouseCursor();

  // set the fact we are currently not in laptop, for rendering purposes
  fCurrentlyInLaptop = FALSE;

  // Deallocate, save data -- leaving laptop.
  SetRenderFlags(RENDER_FLAG_FULL);

  if (fExitDuringLoad == FALSE) {
    ExitLaptopMode(guiCurrentLaptopMode);
  }

  fExitDuringLoad = FALSE;
  fLoadPendingFlag = FALSE;

  DeleteVideoObjectFromIndex(guiLAPTOP);
  DeleteVideoObjectFromIndex(guiLaptopBACKGROUND);
  DeleteVideoObjectFromIndex(guiTITLEBARLAPTOP);
  DeleteVideoObjectFromIndex(guiLIGHTS);
  DeleteVideoObjectFromIndex(guiTITLEBARICONS);
  DeleteVideoObjectFromIndex(guiEmailWarning);

  // destroy region for new mail icon
  CreateDestroyMouseRegionForNewMailIcon();

  // get rid of desktop
  DeleteDesktopBackground();

  if (fErrorFlag) {
    fErrorFlag = FALSE;
    CreateDestroyErrorButton();
  }
  if (fDeleteMailFlag) {
    fDeleteMailFlag = FALSE;
    CreateDestroyDeleteNoticeMailButton();
  }
  if (fNewMailFlag) {
    fNewMailFlag = FALSE;
    CreateDestroyNewMailButton();
  }

  // get rid of minize button
  CreateDestroyMinimizeButtonForCurrentMode();

  // MSYS_SetCurrentCursor(CURSOR_NORMAL);
  gfEnterLapTop = TRUE;
  DeleteLapTopButtons();
  DeleteLapTopMouseRegions();
  // restore tactical buttons
  // CreateCurrentTacticalPanelButtons();
  gfShowBookmarks = FALSE;
  CreateDestoryBookMarkRegions();

  fNewWWW = TRUE;
  RemoveBookmark(-2);
  DeleteBookmark();
  // DeleteBookmarkRegions();
  DeleteLoadPending();
  fReDrawNewMailFlag = FALSE;

  // Since we are going to MapScreen, check the flag
  guiTacticalInterfaceFlags |= INTERFACE_MAPSCREEN;

  // pause the game because we dont want time to advance in the laptop
  UnPauseGame();
}

void RenderLapTopImage() {
  struct VObject *hLapTopHandle;

  if ((fMaximizingProgram == TRUE) || (fMinizingProgram == TRUE)) {
    return;
  }

  GetVideoObject(&hLapTopHandle, guiLAPTOP);
  BltVideoObject(vsFB, hLapTopHandle, 0, LAPTOP_X, LAPTOP_Y);

  GetVideoObject(&hLapTopHandle, guiLaptopBACKGROUND);
  BltVideoObject(vsFB, hLapTopHandle, 1, 25, 23);

  MarkButtonsDirty();
}
void RenderLaptop() {
  uint32_t uiTempMode = 0;

  if ((fMaximizingProgram == TRUE) || (fMinizingProgram == TRUE)) {
    gfShowBookmarks = FALSE;
    return;
  }

  if (fLoadPendingFlag) {
    uiTempMode = guiCurrentLaptopMode;
    guiCurrentLaptopMode = guiPreviousLaptopMode;
  }

  switch (guiCurrentLaptopMode) {
    case (LAPTOP_MODE_NONE):
      DrawDeskTopBackground();
      break;
    case LAPTOP_MODE_AIM:
      RenderAIM();
      break;
    case LAPTOP_MODE_AIM_MEMBERS:
      RenderAIMMembers();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
      RenderAimFacialIndex();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
      RenderAimSort();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
      RenderAimArchives();
      break;
    case LAPTOP_MODE_AIM_POLICIES:
      RenderAimPolicies();
      break;
    case LAPTOP_MODE_AIM_LINKS:
      RenderAimLinks();
      break;
    case LAPTOP_MODE_AIM_HISTORY:
      RenderAimHistory();
      break;
    case LAPTOP_MODE_MERC:
      RenderMercs();
      break;
    case LAPTOP_MODE_MERC_FILES:
      RenderMercsFiles();
      break;
    case LAPTOP_MODE_MERC_ACCOUNT:
      RenderMercsAccount();
      break;
    case LAPTOP_MODE_MERC_NO_ACCOUNT:
      RenderMercsNoAccount();
      break;

    case LAPTOP_MODE_BOBBY_R:
      RenderBobbyR();
      break;

    case LAPTOP_MODE_BOBBY_R_GUNS:
      RenderBobbyRGuns();
      break;
    case LAPTOP_MODE_BOBBY_R_AMMO:
      RenderBobbyRAmmo();
      break;
    case LAPTOP_MODE_BOBBY_R_ARMOR:
      RenderBobbyRArmour();
      break;
    case LAPTOP_MODE_BOBBY_R_MISC:
      RenderBobbyRMisc();
      break;
    case LAPTOP_MODE_BOBBY_R_USED:
      RenderBobbyRUsed();
      break;
    case LAPTOP_MODE_BOBBY_R_MAILORDER:
      RenderBobbyRMailOrder();
      break;
    case LAPTOP_MODE_CHAR_PROFILE:
      RenderCharProfile();
      break;
    case LAPTOP_MODE_FLORIST:
      RenderFlorist();
      break;
    case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
      RenderFloristGallery();
      break;
    case LAPTOP_MODE_FLORIST_ORDERFORM:
      RenderFloristOrderForm();
      break;
    case LAPTOP_MODE_FLORIST_CARD_GALLERY:
      RenderFloristCards();
      break;

    case LAPTOP_MODE_INSURANCE:
      RenderInsurance();
      break;

    case LAPTOP_MODE_INSURANCE_INFO:
      RenderInsuranceInfo();
      break;

    case LAPTOP_MODE_INSURANCE_CONTRACT:
      RenderInsuranceContract();
      break;

    case LAPTOP_MODE_INSURANCE_COMMENTS:
      RenderInsuranceComments();
      break;

    case LAPTOP_MODE_FUNERAL:
      RenderFuneral();
      break;
    case LAPTOP_MODE_SIRTECH:
      RenderSirTech();
      break;
    case LAPTOP_MODE_FINANCES:
      RenderFinances();
      break;
    case LAPTOP_MODE_PERSONNEL:
      RenderPersonnel();
      break;
    case LAPTOP_MODE_HISTORY:
      RenderHistory();
      break;
    case LAPTOP_MODE_FILES:
      RenderFiles();
      break;
    case LAPTOP_MODE_EMAIL:
      RenderEmail();
      break;
    case (LAPTOP_MODE_WWW):
      DrawDeskTopBackground();
      RenderWWWProgramTitleBar();
      break;
    case (LAPTOP_MODE_BROKEN_LINK):
      RenderBrokenLink();
      break;

    case LAPTOP_MODE_BOBBYR_SHIPMENTS:
      RenderBobbyRShipments();
      break;
  }

  if (guiCurrentLaptopMode >= LAPTOP_MODE_WWW) {
    // render program bar for www program
    RenderWWWProgramTitleBar();
  }

  if (fLoadPendingFlag) {
    guiCurrentLaptopMode = uiTempMode;
    return;
  }

  DisplayProgramBoundingBox(FALSE);

  // mark the buttons dirty at this point
  MarkButtonsDirty();
}

void EnterNewLaptopMode() {
  static BOOLEAN fOldLoadFlag = FALSE;

  if (fExitingLaptopFlag) {
    return;
  }
  // cause flicker, as we are going to a new program/WEB page
  fFlickerHD = TRUE;

  // handle maximizing of programs
  switch (guiCurrentLaptopMode) {
    case (LAPTOP_MODE_EMAIL):
      if (gLaptopProgramStates[LAPTOP_PROGRAM_MAILER] == LAPTOP_PROGRAM_MINIMIZED) {
        // minized, maximized
        if (fMaximizingProgram == FALSE) {
          fInitTitle = TRUE;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[0], guiTITLEBARICONS, 0);
          ExitLaptopMode(guiPreviousLaptopMode);
        }
        fMaximizingProgram = TRUE;
        bProgramBeingMaximized = LAPTOP_PROGRAM_MAILER;
        gLaptopProgramStates[LAPTOP_PROGRAM_MAILER] = LAPTOP_PROGRAM_OPEN;

        return;
      }
      break;
    case (LAPTOP_MODE_FILES):
      if (gLaptopProgramStates[LAPTOP_PROGRAM_FILES] == LAPTOP_PROGRAM_MINIMIZED) {
        // minized, maximized
        if (fMaximizingProgram == FALSE) {
          fInitTitle = TRUE;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[1], guiTITLEBARICONS, 2);
          ExitLaptopMode(guiPreviousLaptopMode);
        }

        // minized, maximized
        fMaximizingProgram = TRUE;
        bProgramBeingMaximized = LAPTOP_PROGRAM_FILES;
        gLaptopProgramStates[LAPTOP_PROGRAM_FILES] = LAPTOP_PROGRAM_OPEN;
        return;
      }
      break;
    case (LAPTOP_MODE_PERSONNEL):
      if (gLaptopProgramStates[LAPTOP_PROGRAM_PERSONNEL] == LAPTOP_PROGRAM_MINIMIZED) {
        // minized, maximized
        if (fMaximizingProgram == FALSE) {
          fInitTitle = TRUE;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[2], guiTITLEBARICONS, 3);
          ExitLaptopMode(guiPreviousLaptopMode);
        }

        // minized, maximized
        fMaximizingProgram = TRUE;
        bProgramBeingMaximized = LAPTOP_PROGRAM_PERSONNEL;
        gLaptopProgramStates[LAPTOP_PROGRAM_PERSONNEL] = LAPTOP_PROGRAM_OPEN;
        return;
      }
      break;
    case (LAPTOP_MODE_FINANCES):
      if (gLaptopProgramStates[LAPTOP_PROGRAM_FINANCES] == LAPTOP_PROGRAM_MINIMIZED) {
        // minized, maximized
        if (fMaximizingProgram == FALSE) {
          fInitTitle = TRUE;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[3], guiTITLEBARICONS, 5);
          ExitLaptopMode(guiPreviousLaptopMode);
        }

        // minized, maximized
        fMaximizingProgram = TRUE;
        bProgramBeingMaximized = LAPTOP_PROGRAM_FINANCES;
        gLaptopProgramStates[LAPTOP_PROGRAM_FINANCES] = LAPTOP_PROGRAM_OPEN;
        return;
      }
      break;
    case (LAPTOP_MODE_HISTORY):
      if (gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY] == LAPTOP_PROGRAM_MINIMIZED) {
        // minized, maximized
        if (fMaximizingProgram == FALSE) {
          fInitTitle = TRUE;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[4], guiTITLEBARICONS, 4);
          ExitLaptopMode(guiPreviousLaptopMode);
        }
        // minized, maximized
        fMaximizingProgram = TRUE;
        bProgramBeingMaximized = LAPTOP_PROGRAM_HISTORY;
        gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY] = LAPTOP_PROGRAM_OPEN;
        return;
      }
      break;
    case (LAPTOP_MODE_NONE):
      // do nothing
      break;
    default:
      if (gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] == LAPTOP_PROGRAM_MINIMIZED) {
        // minized, maximized
        if (fMaximizingProgram == FALSE) {
          fInitTitle = TRUE;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pWebTitle[0], guiTITLEBARICONS, 1);
          ExitLaptopMode(guiPreviousLaptopMode);
        }
        // minized, maximized
        fMaximizingProgram = TRUE;
        bProgramBeingMaximized = LAPTOP_PROGRAM_WEB_BROWSER;
        gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] = LAPTOP_PROGRAM_OPEN;
        return;
      }
      break;
  }

  if ((fMaximizingProgram == TRUE) || (fMinizingProgram == TRUE)) {
    return;
  }

  if ((fOldLoadFlag) && (!fLoadPendingFlag)) {
    fOldLoadFlag = FALSE;
  } else if ((fLoadPendingFlag) && (!fOldLoadFlag)) {
    ExitLaptopMode(guiPreviousLaptopMode);
    fOldLoadFlag = TRUE;
    return;
  } else if ((fOldLoadFlag) && (fLoadPendingFlag)) {
    return;
  } else {
    // do not exit previous mode if coming from sliding bar handler
    if ((fEnteredNewLapTopDueToHandleSlidingBars == FALSE)) {
      ExitLaptopMode(guiPreviousLaptopMode);
    }
  }

  if ((guiCurrentWWWMode == LAPTOP_MODE_NONE) && (guiCurrentLaptopMode >= LAPTOP_MODE_WWW)) {
    RenderLapTopImage();
    giCurrentRegion = WWW_REGION;
    RestoreOldRegion(giOldRegion);
    guiCurrentLaptopMode = LAPTOP_MODE_WWW;
    HighLightRegion(giCurrentRegion);
  } else {
    if (guiCurrentLaptopMode > LAPTOP_MODE_WWW) {
      if (guiPreviousLaptopMode < LAPTOP_MODE_WWW)
        guiCurrentLaptopMode = guiCurrentWWWMode;
      else {
        guiCurrentWWWMode = guiCurrentLaptopMode;
        giCurrentSubPage = 0;
      }
    }
  }

  if (guiCurrentLaptopMode >= LAPTOP_MODE_WWW) {
    RenderWWWProgramTitleBar();
  }

  if ((guiCurrentLaptopMode >= LAPTOP_MODE_WWW) && (guiPreviousLaptopMode >= LAPTOP_MODE_WWW)) {
    gfShowBookmarks = FALSE;
  }

  // Initialize the new mode.
  switch (guiCurrentLaptopMode) {
    case LAPTOP_MODE_AIM:
      EnterAIM();
      break;
    case LAPTOP_MODE_AIM_MEMBERS:
      EnterAIMMembers();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
      EnterAimFacialIndex();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
      EnterAimSort();
      break;

    case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
      EnterAimArchives();
      break;
    case LAPTOP_MODE_AIM_POLICIES:
      EnterAimPolicies();
      break;
    case LAPTOP_MODE_AIM_LINKS:
      EnterAimLinks();
      break;
    case LAPTOP_MODE_AIM_HISTORY:
      EnterAimHistory();
      break;

    case LAPTOP_MODE_MERC:
      EnterMercs();
      break;
    case LAPTOP_MODE_MERC_FILES:
      EnterMercsFiles();
      break;
    case LAPTOP_MODE_MERC_ACCOUNT:
      EnterMercsAccount();
      break;
    case LAPTOP_MODE_MERC_NO_ACCOUNT:
      EnterMercsNoAccount();
      break;

    case LAPTOP_MODE_BOBBY_R:
      EnterBobbyR();
      break;
    case LAPTOP_MODE_BOBBY_R_GUNS:
      EnterBobbyRGuns();
      break;
    case LAPTOP_MODE_BOBBY_R_AMMO:
      EnterBobbyRAmmo();
      break;
    case LAPTOP_MODE_BOBBY_R_ARMOR:
      EnterBobbyRArmour();
      break;
    case LAPTOP_MODE_BOBBY_R_MISC:
      EnterBobbyRMisc();
      break;
    case LAPTOP_MODE_BOBBY_R_USED:
      EnterBobbyRUsed();
      break;
    case LAPTOP_MODE_BOBBY_R_MAILORDER:
      EnterBobbyRMailOrder();
      break;
    case LAPTOP_MODE_CHAR_PROFILE:
      EnterCharProfile();
      break;

    case LAPTOP_MODE_FLORIST:
      EnterFlorist();
      break;
    case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
      EnterFloristGallery();
      break;
    case LAPTOP_MODE_FLORIST_ORDERFORM:
      EnterFloristOrderForm();
      break;
    case LAPTOP_MODE_FLORIST_CARD_GALLERY:
      EnterFloristCards();
      break;

    case LAPTOP_MODE_INSURANCE:
      EnterInsurance();
      break;
    case LAPTOP_MODE_INSURANCE_INFO:
      EnterInsuranceInfo();
      break;
    case LAPTOP_MODE_INSURANCE_CONTRACT:
      EnterInsuranceContract();
      break;
    case LAPTOP_MODE_INSURANCE_COMMENTS:
      EnterInsuranceComments();
      break;

    case LAPTOP_MODE_FUNERAL:
      EnterFuneral();
      break;
    case LAPTOP_MODE_SIRTECH:
      EnterSirTech();
      break;
    case LAPTOP_MODE_FINANCES:
      EnterFinances();
      break;
    case LAPTOP_MODE_PERSONNEL:
      EnterPersonnel();
      break;
    case LAPTOP_MODE_HISTORY:
      EnterHistory();
      break;
    case LAPTOP_MODE_FILES:
      EnterFiles();
      break;
    case LAPTOP_MODE_EMAIL:
      EnterEmail();
      break;
    case LAPTOP_MODE_BROKEN_LINK:
      EnterBrokenLink();
      break;
    case LAPTOP_MODE_BOBBYR_SHIPMENTS:
      EnterBobbyRShipments();
      break;
  }

  // first time using webbrowser in this laptop session
  if ((fFirstTimeInLaptop == TRUE) && (guiCurrentLaptopMode >= LAPTOP_MODE_WWW)) {
    // show bookmarks
    gfShowBookmarks = TRUE;

    // reset flag
    fFirstTimeInLaptop = FALSE;
  }

  if ((!fLoadPendingFlag)) {
    CreateDestroyMinimizeButtonForCurrentMode();
    guiPreviousLaptopMode = guiCurrentLaptopMode;
    SetSubSiteAsVisted();
  }

  DisplayProgramBoundingBox(TRUE);

  // check to see if we need to go to there default web page of not
  // HandleDefaultWebpageForLaptop( );
}

void HandleLapTopHandles() {
  if (fLoadPendingFlag) return;

  if ((fMaximizingProgram == TRUE) || (fMinizingProgram == TRUE)) {
    return;
  }

  switch (guiCurrentLaptopMode) {
    case LAPTOP_MODE_AIM:

      HandleAIM();
      break;
    case LAPTOP_MODE_AIM_MEMBERS:
      HandleAIMMembers();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
      HandleAimFacialIndex();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
      HandleAimSort();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
      HandleAimArchives();
      break;
    case LAPTOP_MODE_AIM_POLICIES:
      HandleAimPolicies();
      break;
    case LAPTOP_MODE_AIM_LINKS:
      HandleAimLinks();
      break;
    case LAPTOP_MODE_AIM_HISTORY:
      HandleAimHistory();
      break;

    case LAPTOP_MODE_MERC:
      HandleMercs();
      break;
    case LAPTOP_MODE_MERC_FILES:
      HandleMercsFiles();
      break;
    case LAPTOP_MODE_MERC_ACCOUNT:
      HandleMercsAccount();
      break;
    case LAPTOP_MODE_MERC_NO_ACCOUNT:
      HandleMercsNoAccount();
      break;

    case LAPTOP_MODE_BOBBY_R:
      HandleBobbyR();
      break;
    case LAPTOP_MODE_BOBBY_R_GUNS:
      HandleBobbyRGuns();
      break;
    case LAPTOP_MODE_BOBBY_R_AMMO:
      HandleBobbyRAmmo();
      break;
    case LAPTOP_MODE_BOBBY_R_ARMOR:
      HandleBobbyRArmour();
      break;
    case LAPTOP_MODE_BOBBY_R_MISC:
      HandleBobbyRMisc();
      break;
    case LAPTOP_MODE_BOBBY_R_USED:
      HandleBobbyRUsed();
      break;
    case LAPTOP_MODE_BOBBY_R_MAILORDER:
      HandleBobbyRMailOrder();
      break;

    case LAPTOP_MODE_CHAR_PROFILE:
      HandleCharProfile();
      break;
    case LAPTOP_MODE_FLORIST:
      HandleFlorist();
      break;
    case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
      HandleFloristGallery();
      break;
    case LAPTOP_MODE_FLORIST_ORDERFORM:
      HandleFloristOrderForm();
      break;
    case LAPTOP_MODE_FLORIST_CARD_GALLERY:
      HandleFloristCards();
      break;

    case LAPTOP_MODE_INSURANCE:
      HandleInsurance();
      break;

    case LAPTOP_MODE_INSURANCE_INFO:
      HandleInsuranceInfo();
      break;

    case LAPTOP_MODE_INSURANCE_CONTRACT:
      HandleInsuranceContract();
      break;
    case LAPTOP_MODE_INSURANCE_COMMENTS:
      HandleInsuranceComments();
      break;

    case LAPTOP_MODE_FUNERAL:
      HandleFuneral();
      break;
    case LAPTOP_MODE_SIRTECH:
      HandleSirTech();
      break;
    case LAPTOP_MODE_FINANCES:
      HandleFinances();
      break;
    case LAPTOP_MODE_PERSONNEL:
      HandlePersonnel();
      break;
    case LAPTOP_MODE_HISTORY:
      HandleHistory();
      break;
    case LAPTOP_MODE_FILES:
      HandleFiles();
      break;
    case LAPTOP_MODE_EMAIL:
      HandleEmail();
      break;

    case LAPTOP_MODE_BROKEN_LINK:
      HandleBrokenLink();
      break;

    case LAPTOP_MODE_BOBBYR_SHIPMENTS:
      HandleBobbyRShipments();
      break;
  }
}

uint32_t LaptopScreenHandle() {
  // User just changed modes.  This is determined by the button callbacks
  // created in LaptopScreenInit()

  // just entered
  if (gfEnterLapTop) {
    EnterLaptop();
    CreateLaptopButtons();
    gfEnterLapTop = FALSE;
  }

  if (gfStartMapScreenToLaptopTransition) {  // Everything is set up to start the transition
                                             // animation.
    SGPRect SrcRect2, DstRect;
    int32_t iPercentage, iScalePercentage, iFactor;
    uint32_t uiStartTime, uiTimeRange, uiCurrTime;
    int32_t iX, iY, iWidth, iHeight;

    int32_t iRealPercentage;

    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
    // Step 1:  Build the laptop image into the save buffer.
    gfStartMapScreenToLaptopTransition = FALSE;
    RestoreBackgroundRects();
    RenderLapTopImage();
    HighLightRegion(giCurrentRegion);
    RenderLaptop();
    RenderButtons();
    PrintDate();
    PrintBalance();
    PrintNumberOnTeam();
    ShowLights();

    // Step 2:  The mapscreen image is in the EXTRABUFFER, and laptop is in the SAVEBUFFER
    //         Start transitioning the screen.
    DstRect.iLeft = 0;
    DstRect.iTop = 0;
    DstRect.iRight = 640;
    DstRect.iBottom = 480;
    uiTimeRange = 1000;
    iPercentage = iRealPercentage = 0;
    uiStartTime = GetJA2Clock();
    BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 0, 640, 480);
    BlitBufferToBuffer(vsExtraBuffer, vsFB, 0, 0, 640, 480);
    PlayJA2SampleFromFile("SOUNDS\\Laptop power up (8-11).wav", RATE_11025, HIGHVOLUME, 1,
                          MIDDLEPAN);
    while (iRealPercentage < 100) {
      uiCurrTime = GetJA2Clock();
      iPercentage = (uiCurrTime - uiStartTime) * 100 / uiTimeRange;
      iPercentage = min(iPercentage, 100);

      iRealPercentage = iPercentage;

      // Factor the percentage so that it is modified by a gravity falling acceleration effect.
      iFactor = (iPercentage - 50) * 2;
      if (iPercentage < 50)
        iPercentage = (uint32_t)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
      else
        iPercentage = (uint32_t)(iPercentage + (100 - iPercentage) * iFactor * 0.01 + 0.5);

      // Laptop source rect
      if (iPercentage < 99)
        iScalePercentage = 10000 / (100 - iPercentage);
      else
        iScalePercentage = 5333;
      iWidth = 12 * iScalePercentage / 100;
      iHeight = 9 * iScalePercentage / 100;
      iX = 472 - (472 - 320) * iScalePercentage / 5333;
      iY = 424 - (424 - 240) * iScalePercentage / 5333;

      SrcRect2.iLeft = iX - iWidth / 2;
      SrcRect2.iRight = SrcRect2.iLeft + iWidth;
      SrcRect2.iTop = iY - iHeight / 2;
      SrcRect2.iBottom = SrcRect2.iTop + iHeight;

      BltStretchVSurface(vsFB, vsSaveBuffer, &DstRect, &SrcRect2);
      InvalidateScreen();
      RefreshScreen(NULL);
    }
    fReDrawScreenFlag = TRUE;
  }

  // DO NOT MOVE THIS FUNCTION CALL!!!

  // This determines if the help screen should be active
  if (ShouldTheHelpScreenComeUp(HELP_SCREEN_LAPTOP, FALSE)) {
    // handle the help screen
    HelpScreenHandler();
    return (LAPTOP_SCREEN);
  }

  RestoreBackgroundRects();

  // lock cursor to screen
  RestrictMouseCursor(&LaptopScreenRect);

  // handle animated cursors
  HandleAnimatedCursors();
  // Deque all game events
  DequeAllGameEvents(TRUE);

  // handle sub sites..like BR Guns, BR Ammo, Armour, Misc...for WW Wait..since they are not true
  // sub pages and are not individual sites
  HandleWWWSubSites();
  UpdateStatusOfDisplayingBookMarks();

  // check if we need to reset new WWW mode
  CheckIfNewWWWW();

  if (guiCurrentLaptopMode != guiPreviousLaptopMode) {
    if (guiCurrentLaptopMode <= LAPTOP_MODE_WWW) {
      fLoadPendingFlag = FALSE;
    }

    if ((fMaximizingProgram == FALSE) && (fMinizingProgram == FALSE)) {
      if (guiCurrentLaptopMode <= LAPTOP_MODE_WWW) {
        EnterNewLaptopMode();
        if ((fMaximizingProgram == FALSE) && (fMinizingProgram == FALSE)) {
          guiPreviousLaptopMode = guiCurrentLaptopMode;
        }
      } else {
        if (!fLoadPendingFlag) {
          EnterNewLaptopMode();
          guiPreviousLaptopMode = guiCurrentLaptopMode;
        }
      }
    }
  }
  if (fPausedReDrawScreenFlag) {
    fReDrawScreenFlag = TRUE;
    fPausedReDrawScreenFlag = FALSE;
  }

  if (fReDrawScreenFlag) {
    RenderLapTopImage();
    HighLightRegion(giCurrentRegion);
    RenderLaptop();
  }

  // are we about to leave laptop
  if (fExitingLaptopFlag) {
    if (fLoadPendingFlag == TRUE) {
      fLoadPendingFlag = FALSE;
      fExitDuringLoad = TRUE;
    }
    LeaveLapTopScreen();
  }

  if (fExitingLaptopFlag == FALSE) {
    // handle handles for laptop input stream
    HandleLapTopHandles();
  }

  // get keyboard input, handle it
  GetLaptopKeyboardInput();

  // check to see if new mail box needs to be displayed
  DisplayNewMailBox();
  CreateDestroyNewMailButton();

  // create various mouse regions that are global to laptop system
  CreateDestoryBookMarkRegions();
  CreateDestroyErrorButton();

  // check to see if error box needs to be displayed
  DisplayErrorBox();

  // check to see if buttons marked dirty
  CheckMarkButtonsDirtyFlag();

  // check to see if new mail box needs to be displayed
  ShouldNewMailBeDisplayed();

  // check to see if new mail box needs to be displayed
  ReDrawNewMailBox();

  // look for unread email
  LookForUnread();
  // Handle keyboard shortcuts...

  // mouse regions
  // HandleLapTopScreenMouseUi();
  // RenderButtons();
  // RenderButtonsFastHelp( );

  if ((fLoadPendingFlag == FALSE) || (fNewMailFlag)) {
    // render buttons marked dirty
    RenderButtons();

    // render fast help 'quick created' buttons
    //		RenderFastHelp( );
    //	  RenderButtonsFastHelp( );
  }

  // show text on top of buttons
  if ((fMaximizingProgram == FALSE) && (fMinizingProgram == FALSE)) {
    DrawButtonText();
  }

  // check to see if bookmarks need to be displayed
  if (gfShowBookmarks) {
    if (fExitingLaptopFlag)
      gfShowBookmarks = FALSE;
    else
      DisplayBookMarks();
  }

  // check to see if laod pending flag is set
  DisplayLoadPending();

  // check if we are showing message?
  DisplayWebBookMarkNotify();

  if ((fIntermediateReDrawFlag) || (fReDrawPostButtonRender)) {
    // rendering AFTER buttons and button text
    if ((fMaximizingProgram == FALSE) && (fMinizingProgram == FALSE)) {
      PostButtonRendering();
    }
  }
  // PrintBalance( );

  PrintDate();

  PrintBalance();

  PrintNumberOnTeam();
  DisplayTaskBarIcons();

  // handle if we are maximizing a program from a minimized state or vice versa
  HandleSlidingTitleBar();

  // flicker HD light as nessacary
  FlickerHDLight();

  // display power and HD lights
  ShowLights();

  // render frame rate
  DisplayFrameRate();

  // invalidate screen if redrawn
  if (fReDrawScreenFlag == TRUE) {
    InvalidateRegion(0, 0, 640, 480);
    fReDrawScreenFlag = FALSE;
  }

  ExecuteVideoOverlays();

  SaveBackgroundRects();
  //	RenderButtonsFastHelp();
  RenderFastHelp();

  // ex SAVEBUFFER queue
  ExecuteBaseDirtyRectQueue();
  ResetInterface();
  EndFrameBufferRender();
  return (LAPTOP_SCREEN);
}

uint32_t RenderLaptopPanel() { return 0; }

uint32_t ExitLaptopMode(uint32_t uiMode) {
  // Deallocate the previous mode that you were in.

  switch (uiMode) {
    case LAPTOP_MODE_AIM:
      ExitAIM();
      break;
    case LAPTOP_MODE_AIM_MEMBERS:
      ExitAIMMembers();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
      ExitAimFacialIndex();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
      ExitAimSort();
      break;
    case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
      ExitAimArchives();
      break;
    case LAPTOP_MODE_AIM_POLICIES:
      ExitAimPolicies();
      break;
    case LAPTOP_MODE_AIM_LINKS:
      ExitAimLinks();
      break;
    case LAPTOP_MODE_AIM_HISTORY:
      ExitAimHistory();
      break;

    case LAPTOP_MODE_MERC:
      ExitMercs();
      break;
    case LAPTOP_MODE_MERC_FILES:
      ExitMercsFiles();
      break;
    case LAPTOP_MODE_MERC_ACCOUNT:
      ExitMercsAccount();
      break;
    case LAPTOP_MODE_MERC_NO_ACCOUNT:
      ExitMercsNoAccount();
      break;

    case LAPTOP_MODE_BOBBY_R:
      ExitBobbyR();
      break;
    case LAPTOP_MODE_BOBBY_R_GUNS:
      ExitBobbyRGuns();
      break;
    case LAPTOP_MODE_BOBBY_R_AMMO:
      ExitBobbyRAmmo();
      break;
    case LAPTOP_MODE_BOBBY_R_ARMOR:
      ExitBobbyRArmour();
      break;
    case LAPTOP_MODE_BOBBY_R_MISC:
      ExitBobbyRMisc();
      break;
    case LAPTOP_MODE_BOBBY_R_USED:
      ExitBobbyRUsed();
      break;
    case LAPTOP_MODE_BOBBY_R_MAILORDER:
      ExitBobbyRMailOrder();
      break;

    case LAPTOP_MODE_CHAR_PROFILE:
      ExitCharProfile();
      break;
    case LAPTOP_MODE_FLORIST:
      ExitFlorist();
      break;
    case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
      ExitFloristGallery();
      break;
    case LAPTOP_MODE_FLORIST_ORDERFORM:
      ExitFloristOrderForm();
      break;
    case LAPTOP_MODE_FLORIST_CARD_GALLERY:
      ExitFloristCards();
      break;

    case LAPTOP_MODE_INSURANCE:
      ExitInsurance();
      break;

    case LAPTOP_MODE_INSURANCE_INFO:
      ExitInsuranceInfo();
      break;

    case LAPTOP_MODE_INSURANCE_CONTRACT:
      ExitInsuranceContract();
      break;
    case LAPTOP_MODE_INSURANCE_COMMENTS:
      ExitInsuranceComments();
      break;

    case LAPTOP_MODE_FUNERAL:
      ExitFuneral();
      break;
    case LAPTOP_MODE_SIRTECH:
      ExitSirTech();
      break;
    case LAPTOP_MODE_FINANCES:
      ExitFinances();
      break;
    case LAPTOP_MODE_PERSONNEL:
      ExitPersonnel();
      break;
    case LAPTOP_MODE_HISTORY:
      ExitHistory();
      break;
    case LAPTOP_MODE_FILES:
      ExitFiles();
      break;
    case LAPTOP_MODE_EMAIL:
      ExitEmail();
      break;
    case LAPTOP_MODE_BROKEN_LINK:
      ExitBrokenLink();
      break;

    case LAPTOP_MODE_BOBBYR_SHIPMENTS:
      ExitBobbyRShipments();
      break;
  }

  if ((uiMode != LAPTOP_MODE_NONE) && (uiMode < LAPTOP_MODE_WWW)) {
    CreateDestroyMinimizeButtonForCurrentMode();
  }
  return (TRUE);
}

uint32_t CreateLaptopButtons() {
  memset(giLapTopButton, -1, sizeof(giLapTopButton));

  /*giLapTopButtonImage[ON_BUTTON]=  LoadButtonImage( "LAPTOP\\button.sti" ,-1,1,-1,0,-1 );
  giLapTopButton[ON_BUTTON] = QuickCreateButton( giLapTopButtonImage[ON_BUTTON], ON_X, ON_Y,
                                                                                 BUTTON_TOGGLE,
  MSYS_PRIORITY_HIGHEST, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
  (GUI_CALLBACK)BtnOnCallback);
   */

  // the program buttons

  gLaptopButtonImage[0] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 0, -1, 8, -1);
  gLaptopButton[0] = QuickCreateButton(
      gLaptopButtonImage[0], 29, 66, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)EmailRegionButtonCallback);
  CreateLaptopButtonHelpText(gLaptopButton[0], LAPTOP_BN_HLP_TXT_VIEW_EMAIL);

  SpecifyButtonText(gLaptopButton[0], pLaptopIcons[0]);
  SpecifyButtonFont(gLaptopButton[0], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[0], 30, 11, TRUE);
  SpecifyButtonDownTextColors(gLaptopButton[0], 2, 0);
  SpecifyButtonUpTextColors(gLaptopButton[0], 2, 0);

  gLaptopButtonImage[1] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 1, -1, 9, -1);
  gLaptopButton[1] = QuickCreateButton(
      gLaptopButtonImage[1], 29, 98, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)WWWRegionButtonCallback);
  CreateLaptopButtonHelpText(gLaptopButton[1], LAPTOP_BN_HLP_TXT_BROWSE_VARIOUS_WEB_SITES);

  SpecifyButtonText(gLaptopButton[1], pLaptopIcons[1]);
  SpecifyButtonFont(gLaptopButton[1], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[1], 30, 11, TRUE);
  SpecifyButtonUpTextColors(gLaptopButton[1], 2, 0);
  SpecifyButtonDownTextColors(gLaptopButton[1], 2, 0);

  gLaptopButtonImage[2] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 2, -1, 10, -1);
  gLaptopButton[2] = QuickCreateButton(
      gLaptopButtonImage[2], 29, 130, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)FilesRegionButtonCallback);
  CreateLaptopButtonHelpText(gLaptopButton[2], LAPTOP_BN_HLP_TXT_VIEW_FILES_AND_EMAIL_ATTACHMENTS);

  SpecifyButtonText(gLaptopButton[2], pLaptopIcons[5]);
  SpecifyButtonFont(gLaptopButton[2], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[2], 30, 11, TRUE);
  SpecifyButtonUpTextColors(gLaptopButton[2], 2, 0);
  SpecifyButtonDownTextColors(gLaptopButton[2], 2, 0);

  gLaptopButtonImage[3] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 3, -1, 11, -1);
  gLaptopButton[3] = QuickCreateButton(
      gLaptopButtonImage[3], 29, 194, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)PersonnelRegionButtonCallback);
  CreateLaptopButtonHelpText(gLaptopButton[3], LAPTOP_BN_HLP_TXT_VIEW_TEAM_INFO);

  SpecifyButtonText(gLaptopButton[3], pLaptopIcons[3]);
  SpecifyButtonFont(gLaptopButton[3], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[3], 30, 11, TRUE);
  SpecifyButtonUpTextColors(gLaptopButton[3], 2, 0);
  SpecifyButtonDownTextColors(gLaptopButton[3], 2, 0);

  gLaptopButtonImage[4] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 4, -1, 12, -1);
  gLaptopButton[4] = QuickCreateButton(
      gLaptopButtonImage[4], 29, 162, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)HistoryRegionButtonCallback);
  CreateLaptopButtonHelpText(gLaptopButton[4], LAPTOP_BN_HLP_TXT_READ_LOG_OF_EVENTS);

  SpecifyButtonText(gLaptopButton[4], pLaptopIcons[4]);
  SpecifyButtonFont(gLaptopButton[4], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[4], 30, 11, TRUE);
  SpecifyButtonUpTextColors(gLaptopButton[4], 2, 0);
  SpecifyButtonDownTextColors(gLaptopButton[4], 2, 0);

  gLaptopButtonImage[5] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 5, -1, 13, -1);
  gLaptopButton[5] = QuickCreateButton(
      gLaptopButtonImage[5], 29, 226 + 15, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)FinancialRegionButtonCallback);
  CreateLaptopButtonHelpText(gLaptopButton[5],
                             LAPTOP_BN_HLP_TXT_VIEW_FINANCIAL_SUMMARY_AND_HISTORY);

  SpecifyButtonText(gLaptopButton[5], pLaptopIcons[2]);
  SpecifyButtonFont(gLaptopButton[5], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[5], 30, 11, TRUE);
  SpecifyButtonUpTextColors(gLaptopButton[5], 2, 0);
  SpecifyButtonDownTextColors(gLaptopButton[5], 2, 0);

  gLaptopButtonImage[6] = LoadButtonImage("LAPTOP\\buttonsforlaptop.sti", -1, 6, -1, 14, -1);
  gLaptopButton[6] = QuickCreateButton(gLaptopButtonImage[6], 29, 371 + 7,  // DEF: was 19
                                       BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
                                       (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                                       (GUI_CALLBACK)BtnOnCallback);
  CreateLaptopButtonHelpText(gLaptopButton[6], LAPTOP_BN_HLP_TXT_CLOSE_LAPTOP);

  SpecifyButtonText(gLaptopButton[6], pLaptopIcons[6]);
  SpecifyButtonFont(gLaptopButton[6], FONT10ARIAL);
  SpecifyButtonTextOffsets(gLaptopButton[6], 25, 11, TRUE);
  SpecifyButtonUpTextColors(gLaptopButton[6], 2, 0);
  SpecifyButtonDownTextColors(gLaptopButton[6], 2, 0);

  // define the cursor
  SetButtonCursor(gLaptopButton[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[1], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[2], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[3], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[4], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[5], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[6], CURSOR_LAPTOP_SCREEN);

  return (TRUE);
}

void DeleteLapTopButtons() {
  uint32_t cnt;
  /*	for ( cnt = 0; cnt < MAX_BUTTON_COUNT; cnt++ )
          {
                  if (giLapTopButton[ cnt ] != -1 )
                  {
                          RemoveButton( giLapTopButton[ cnt ] );
                  }
          }


          for ( cnt = 0; cnt < MAX_BUTTON_COUNT; cnt++ )
          {
                  UnloadButtonImage( giLapTopButtonImage[ cnt ] );
          }

  */
  for (cnt = 0; cnt < 7; cnt++) {
    RemoveButton(gLaptopButton[cnt]);
    UnloadButtonImage(gLaptopButtonImage[cnt]);
  }
}

void BtnOnCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
    InvalidateRegion(0, 0, 640, 480);

  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      if (HandleExit()) {
        //			 btn->uiFlags&=~(BUTTON_CLICKED_ON);
        fExitingLaptopFlag = TRUE;
        InvalidateRegion(0, 0, 640, 480);
      }
    }
    btn->uiFlags &= ~(BUTTON_CLICKED_ON);
  }
}

BOOLEAN LeaveLapTopScreen(void) {
  if (ExitLaptopDone()) {
    // exit screen is set
    // set new screen
    // if( ( LaptopSaveInfo.gfNewGameLaptop != TRUE ) || !( AnyMercsHired() ) )
    //	{
    SetLaptopExitScreen(MAP_SCREEN);
    //}
    // if( ( LaptopSaveInfo.gfNewGameLaptop )&&( AnyMercsHired() ) )
    //{
    //	SetLaptopExitScreen( GAME_SCREEN );
    //	}

    if (gfAtLeastOneMercWasHired == TRUE) {
      if (LaptopSaveInfo.gfNewGameLaptop) {
        LaptopSaveInfo.gfNewGameLaptop = FALSE;
        fExitingLaptopFlag = TRUE;
        /*guiExitScreen = GAME_SCREEN; */
        InitNewGame(FALSE);
        gfDontStartTransitionFromLaptop = TRUE;
        /*InitHelicopterEntranceByMercs( );
        fFirstTimeInGameScreen = TRUE;*/
        return (TRUE);
      }
    } else {
      gfDontStartTransitionFromLaptop = TRUE;
    }

    SetPendingNewScreen(guiExitScreen);

    if (!gfDontStartTransitionFromLaptop) {
      SGPRect SrcRect2, DstRect;
      int32_t iPercentage, iScalePercentage, iFactor;
      uint32_t uiStartTime, uiTimeRange, uiCurrTime;
      int32_t iX, iY, iWidth, iHeight;
      int32_t iRealPercentage;

      gfDontStartTransitionFromLaptop = TRUE;
      SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
      // Step 1:  Build the laptop image into the save buffer.
      RestoreBackgroundRects();
      RenderLapTopImage();
      HighLightRegion(giCurrentRegion);
      RenderLaptop();
      RenderButtons();
      PrintDate();
      PrintBalance();
      PrintNumberOnTeam();
      ShowLights();

      // Step 2:  The mapscreen image is in the EXTRABUFFER, and laptop is in the SAVEBUFFER
      //         Start transitioning the screen.
      DstRect.iLeft = 0;
      DstRect.iTop = 0;
      DstRect.iRight = 640;
      DstRect.iBottom = 480;
      uiTimeRange = 1000;
      iPercentage = iRealPercentage = 100;
      uiStartTime = GetJA2Clock();
      BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 0, 640, 480);
      PlayJA2SampleFromFile("SOUNDS\\Laptop power down (8-11).wav", RATE_11025, HIGHVOLUME, 1,
                            MIDDLEPAN);
      while (iRealPercentage > 0) {
        BlitBufferToBuffer(vsExtraBuffer, vsFB, 0, 0, 640, 480);

        uiCurrTime = GetJA2Clock();
        iPercentage = (uiCurrTime - uiStartTime) * 100 / uiTimeRange;
        iPercentage = min(iPercentage, 100);
        iPercentage = 100 - iPercentage;

        iRealPercentage = iPercentage;

        // Factor the percentage so that it is modified by a gravity falling acceleration effect.
        iFactor = (iPercentage - 50) * 2;
        if (iPercentage < 50)
          iPercentage = (uint32_t)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
        else
          iPercentage = (uint32_t)(iPercentage + (100 - iPercentage) * iFactor * 0.01 + 0.5);

        // Laptop source rect
        if (iPercentage < 99)
          iScalePercentage = 10000 / (100 - iPercentage);
        else
          iScalePercentage = 5333;
        iWidth = 12 * iScalePercentage / 100;
        iHeight = 9 * iScalePercentage / 100;
        iX = 472 - (472 - 320) * iScalePercentage / 5333;
        iY = 424 - (424 - 240) * iScalePercentage / 5333;

        SrcRect2.iLeft = iX - iWidth / 2;
        SrcRect2.iRight = SrcRect2.iLeft + iWidth;
        SrcRect2.iTop = iY - iHeight / 2;
        SrcRect2.iBottom = SrcRect2.iTop + iHeight;

        BltStretchVSurface(vsFB, vsSaveBuffer, &DstRect, &SrcRect2);
        InvalidateScreen();
        RefreshScreen(NULL);
      }
    }
  }
  return (TRUE);
}

BOOLEAN HandleExit(void) {
  //	static BOOLEAN fSentImpWarningAlready = FALSE;

  // remind player about IMP
  if (LaptopSaveInfo.gfNewGameLaptop != 0) {
    if (!AnyMercsHired()) {
      // AddEmail(0,1, GAME_HELP, GetWorldTotalMin( ) );
      // fExitingLaptopFlag = FALSE;
      // return( FALSE );
    }
  }

  // new game, send email
  if (LaptopSaveInfo.gfNewGameLaptop != 0) {
    // Set an event to send this email ( day 2 8:00-12:00 )
    if ((LaptopSaveInfo.fIMPCompletedFlag == FALSE) &&
        (LaptopSaveInfo.fSentImpWarningAlready == FALSE)) {
      AddFutureDayStrategicEvent(EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL, (8 + Random(4)) * 60, 0, 1);

      /*
       Moved to an event that gets triggered the next day: HaventMadeImpMercEmailCallBack()

                              LaptopSaveInfo.fSentImpWarningAlready = TRUE;
                              AddEmail(IMP_EMAIL_AGAIN,IMP_EMAIL_AGAIN_LENGTH,1, GetWorldTotalMin( )
       );
      */
      fExitingLaptopFlag = TRUE;

      return (FALSE);
    }
  }

  return (TRUE);
}

void HaventMadeImpMercEmailCallBack() {
  // if the player STILL hasnt made an imp merc yet
  if ((LaptopSaveInfo.fIMPCompletedFlag == FALSE) &&
      (LaptopSaveInfo.fSentImpWarningAlready == FALSE)) {
    LaptopSaveInfo.fSentImpWarningAlready = TRUE;
    AddEmail(IMP_EMAIL_AGAIN, IMP_EMAIL_AGAIN_LENGTH, 1, GetWorldTotalMin());
  }
}

BOOLEAN
CreateLapTopMouseRegions() {
  // define regions

  // the entire laptop display region
  MSYS_DefineRegion(&gLapTopScreenRegion, (uint16_t)(LaptopScreenRect.iLeft),
                    (uint16_t)(LaptopScreenRect.iTop), (uint16_t)(LaptopScreenRect.iRight),
                    (uint16_t)(LaptopScreenRect.iBottom), MSYS_PRIORITY_NORMAL + 1,
                    CURSOR_LAPTOP_SCREEN, ScreenRegionMvtCallback, LapTopScreenCallBack);

  // MSYS_AddRegion(&gLapTopScreenRegion);
  return (TRUE);
}

BOOLEAN
DeleteLapTopMouseRegions() {
  MSYS_RemoveRegion(&gLapTopScreenRegion);

  return (TRUE);
}

void FinancialRegionButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      if (giCurrentRegion != FINANCIAL_REGION) giOldRegion = giCurrentRegion;
      giCurrentRegion = FINANCIAL_REGION;
      if (gfShowBookmarks) {
        gfShowBookmarks = FALSE;
        fReDrawScreenFlag = TRUE;
      }
      guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;

      UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_FINANCES);
    }
  }
}

void PersonnelRegionButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (giCurrentRegion != PERSONNEL_REGION) giOldRegion = giCurrentRegion;
      giCurrentRegion = PERSONNEL_REGION;
      guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
      if (gfShowBookmarks) {
        gfShowBookmarks = FALSE;
        fReDrawScreenFlag = TRUE;
      }
      RestoreOldRegion(giOldRegion);
      HighLightRegion(giCurrentRegion);
      gfShowBookmarks = FALSE;

      UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_PERSONNEL);
    }
  }
}

void EmailRegionButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      // set old region
      if (giCurrentRegion != EMAIL_REGION) giOldRegion = giCurrentRegion;

      // stop showing WWW bookmarks
      if (gfShowBookmarks) {
        gfShowBookmarks = FALSE;
      }

      // set current highlight region
      giCurrentRegion = EMAIL_REGION;

      // restore old region
      RestoreOldRegion(giOldRegion);

      // set up current mode
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;

      UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_MAILER);

      // highlight current region
      HighLightRegion(giCurrentRegion);

      // redraw screen
      fReDrawScreenFlag = TRUE;
    }
  }
}

void WWWRegionButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      if (giCurrentRegion != WWW_REGION) giOldRegion = giCurrentRegion;
      if (!fNewWWW) fNewWWWDisplay = FALSE;

      // reset show bookmarks
      if (guiCurrentLaptopMode < LAPTOP_MODE_WWW) {
        gfShowBookmarks = FALSE;
        fShowBookmarkInfo = TRUE;
      } else {
        gfShowBookmarks = !gfShowBookmarks;
      }

      if ((gfShowBookmarks) && (!fNewWWW)) {
        fReDrawScreenFlag = TRUE;
        fNewWWWDisplay = FALSE;
      } else if (fNewWWW) {
        // no longer a new WWW mode
        fNewWWW = FALSE;

        // new WWW to display
        fNewWWWDisplay = TRUE;

        // make sure program is maximized
        if (gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] == LAPTOP_PROGRAM_OPEN) {
          // re render laptop region
          RenderLapTopImage();

          // re render background
          DrawDeskTopBackground();
        }
      }
      giCurrentRegion = WWW_REGION;
      RestoreOldRegion(giOldRegion);
      if (guiCurrentWWWMode != LAPTOP_MODE_NONE)
        guiCurrentLaptopMode = guiCurrentWWWMode;
      else
        guiCurrentLaptopMode = LAPTOP_MODE_WWW;
      UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_WEB_BROWSER);
      HighLightRegion(giCurrentRegion);
      fReDrawScreenFlag = TRUE;
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      // nothing yet

      if (giCurrentRegion != WWW_REGION) giOldRegion = giCurrentRegion;

      giCurrentRegion = WWW_REGION;

      RestoreOldRegion(giOldRegion);

      if (guiCurrentWWWMode != LAPTOP_MODE_NONE)
        guiCurrentLaptopMode = guiCurrentWWWMode;
      else
        guiCurrentLaptopMode = LAPTOP_MODE_WWW;

      HighLightRegion(giCurrentRegion);

      fReDrawScreenFlag = TRUE;
    }
  }
}

void HistoryRegionButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      // if not in history, update to the fact
      if (giCurrentRegion != HISTORY_REGION) giOldRegion = giCurrentRegion;
      if (gfShowBookmarks) {
        // stop showing WWW bookmarks
        gfShowBookmarks = FALSE;
      }

      // current region is history
      giCurrentRegion = HISTORY_REGION;

      // restore old region area
      RestoreOldRegion(giOldRegion);

      // set mode to history
      guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;

      // hightlight current icon
      HighLightRegion(giCurrentRegion);

      UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_HISTORY);

      gfShowBookmarks = FALSE;

      // redraw screen
      fReDrawScreenFlag = TRUE;
    }
  }
}
void FilesRegionButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      // reset old region
      if (giCurrentRegion != FILES_REGION) giOldRegion = giCurrentRegion;

      // stop showing WWW bookmarks
      if (gfShowBookmarks) {
        gfShowBookmarks = FALSE;
        fReDrawScreenFlag = TRUE;
      }

      // set new region
      giCurrentRegion = FILES_REGION;

      // restore old highlight region
      RestoreOldRegion(giOldRegion);

      // highlight new region
      HighLightRegion(giCurrentRegion);

      guiCurrentLaptopMode = LAPTOP_MODE_FILES;

      UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_FILES);

      // redraw screen
      fReDrawScreenFlag = TRUE;
    }
  }
}

void HandleLapTopScreenMouseUi() {
  if (gEmailRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    giHighLightRegion = EMAIL_REGION;
  } else if (gPersonnelRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    giHighLightRegion = PERSONNEL_REGION;
  } else if (gFinancialRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    giHighLightRegion = FINANCIAL_REGION;
  } else if (gWWWRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    giHighLightRegion = WWW_REGION;
  } else if (gFilesRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    giHighLightRegion = FILES_REGION;
  } else if (gHistoryRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    giHighLightRegion = HISTORY_REGION;
  } else
    giHighLightRegion = NO_REGION;
  DrawHighLightRegionBox();
}

void DrawHighLightRegionBox() { return; }

void RestoreOldRegion(int32_t iOldRegion) { return; }

void HighLightRegion(int32_t iCurrentRegion) { return; }

void HandleAnimatedButtons() { return; }
void AnimateButton(uint32_t uiIconID, uint16_t usX, uint16_t usY) { return; }

void WWWRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static int32_t iFrame = 0;
  struct VObject *hLapTopIconHandle;
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iFrame = 0;
    GetVideoObject(&hLapTopIconHandle, guiWWWICON);
    BltVideoObject(vsFB, hLapTopIconHandle, (uint16_t)iFrame, LAPTOP_ICONS_X, LAPTOP_ICONS_WWW_Y);
    DrawLapTopText();
    HighLightRegion(giCurrentRegion);
    InvalidateRegion(0, 0, 640, 480);
  }
}

void EmailRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static int32_t iFrame = 0;
  struct VObject *hLapTopIconHandle;
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iFrame = 0;
    DrawLapTopText();
    GetVideoObject(&hLapTopIconHandle, guiMAILICON);
    BltVideoObject(vsFB, hLapTopIconHandle, (uint16_t)iFrame, LAPTOP_ICONS_X, LAPTOP_ICONS_MAIL_Y);
    if (fUnReadMailFlag) {
      GetVideoObject(&hLapTopIconHandle, guiUNREAD);
      BltVideoObject(vsFB, hLapTopIconHandle, 0, LAPTOP_ICONS_X + CHECK_X,
                     LAPTOP_ICONS_MAIL_Y + CHECK_Y);
    }
    DrawLapTopText();
    HighLightRegion(giCurrentRegion);
    InvalidateRegion(0, 0, 640, 480);
  }
}

void FinancialRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static int32_t iFrame = 0;
  struct VObject *hLapTopIconHandle;
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iFrame = 0;
    GetVideoObject(&hLapTopIconHandle, guiFINANCIALICON);
    BltVideoObject(vsFB, hLapTopIconHandle, (uint16_t)iFrame, LAPTOP_ICONS_X - 4,
                   LAPTOP_ICONS_FINANCIAL_Y);
    DrawLapTopText();
    HighLightRegion(giCurrentRegion);
    InvalidateRegion(0, 0, 640, 480);
  }
}

void HistoryRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static int32_t iFrame = 0;
  struct VObject *hLapTopIconHandle;
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iFrame = 0;

    GetVideoObject(&hLapTopIconHandle, guiHISTORYICON);
    BltVideoObject(vsFB, hLapTopIconHandle, (uint16_t)iFrame, LAPTOP_ICONS_X,
                   LAPTOP_ICONS_HISTORY_Y);
    DrawLapTopText();
    HighLightRegion(giCurrentRegion);
    InvalidateRegion(0, 0, 640, 480);
  }
}

void FilesRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static int32_t iFrame = 0;
  struct VObject *hLapTopIconHandle;
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iFrame = 0;
    GetVideoObject(&hLapTopIconHandle, guiFILESICON);
    BltVideoObject(vsFB, hLapTopIconHandle, (uint16_t)iFrame, LAPTOP_ICONS_X,
                   LAPTOP_ICONS_FILES_Y + 7);
    DrawLapTopText();
    HighLightRegion(giCurrentRegion);
    InvalidateRegion(0, 0, 640, 480);
  }
}

void PersonnelRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static int32_t iFrame = 0;
  struct VObject *hLapTopIconHandle;
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iFrame = 0;

    GetVideoObject(&hLapTopIconHandle, guiPERSICON);
    BltVideoObject(vsFB, hLapTopIconHandle, (uint16_t)iFrame, LAPTOP_ICONS_X,
                   LAPTOP_ICONS_PERSONNEL_Y);
    DrawLapTopText();
    HighLightRegion(giCurrentRegion);
    InvalidateRegion(0, 0, 640, 480);
  }
}

void CheckIfMouseLeaveScreen() {
  struct Point MousePos = GetMousePoint();
  if ((MousePos.x > LAPTOP_SCREEN_LR_X) || (MousePos.x < LAPTOP_UL_X) ||
      (MousePos.y < LAPTOP_UL_Y) || (MousePos.y > LAPTOP_SCREEN_LR_Y)) {
    guiCurrentLapTopCursor = LAPTOP_PANEL_CURSOR;
  }
}
void ScreenRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }
  /*if (iReason == MSYS_CALLBACK_REASON_MOVE)
  {
          guiCurrentLapTopCursor=LAPTOP_SCREEN_CURSOR;
  }
if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
  {
CheckIfMouseLeaveScreen();
  }
  */
}

void ReDrawHighLight() {
  HighLightRegion(giCurrentRegion);
  return;
}

void DrawButtonText() {
  if (fErrorFlag) DrawTextOnErrorButton();
  switch (guiCurrentLaptopMode) {
    case LAPTOP_MODE_EMAIL:
      DisplayEmailHeaders();
      break;
  }
  return;
}

void InitBookMarkList() {
  // sets bookmark list to -1
  memset(LaptopSaveInfo.iBookMarkList, -1, sizeof(LaptopSaveInfo.iBookMarkList));
  return;
}

void SetBookMark(int32_t iBookId) {
  // find first empty spot, set to iBookId
  int32_t iCounter = 0;
  if (iBookId != -2) {
    while (LaptopSaveInfo.iBookMarkList[iCounter] != -1) {
      // move trhough list until empty
      if (LaptopSaveInfo.iBookMarkList[iCounter] == iBookId) {
        // found it, return
        return;
      }
      iCounter++;
    }
    LaptopSaveInfo.iBookMarkList[iCounter] = iBookId;
  }
  return;
}

BOOLEAN RemoveBookMark(int32_t iBookId) {
  int32_t iCounter = 0;

  // Loop through the bookmarks to get to the desired bookmark
  while (LaptopSaveInfo.iBookMarkList[iCounter] != iBookId) {
    iCounter++;
  }

  // Did we find the right one?
  if (LaptopSaveInfo.iBookMarkList[iCounter] == iBookId) {
    // Reset it
    LaptopSaveInfo.iBookMarkList[iCounter] = -1;

    // return true signifing that we found it
    return (TRUE);
  }

  // nope, we didnt find it.
  return (FALSE);
}

BOOLEAN LoadBookmark() {
  // grab download bars too

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\downloadtop.sti"), &guiDOWNLOADTOP));

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\downloadmid.sti"), &guiDOWNLOADMID));

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\downloadbot.sti"), &guiDOWNLOADBOT));

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\bookmarktop.sti"), &guiBOOKTOP));

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\bookmarkmiddle.sti"), &guiBOOKMID));

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\webpages.sti"), &guiBOOKMARK));

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\hilite.sti"), &guiBOOKHIGH));
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Bookmarkbottom.sti"), &guiBOOKBOT));

  return (TRUE);
}

void DisplayBookMarks(void) {
  // will look at bookmarklist and set accordingly
  int32_t iCounter = 1;
  // load images
  struct VObject *hLapTopIconHandle;
  // laptop icons
  int16_t sX, sY;

  // check if we are maximizing or minimizing.. if so, do not display
  if ((fMaximizingProgram == TRUE) || (fMinizingProgram == TRUE)) {
    return;
  }

  // font stuff
  SetFont(BOOK_FONT);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // dirty and print to screen
  //	gprintfdirty( sX, sY, pBookmarkTitle[0]);
  //	mprintf(sX, sY,pBookmarkTitle[0] );

  // set buffer
  SetFontDestBuffer(vsFB, BOOK_X, BOOK_TOP_Y, BOOK_X + BOOK_WIDTH - 10, 480, FALSE);

  // blt in book mark background
  while (LaptopSaveInfo.iBookMarkList[iCounter - 1] != -1) {
    if (iHighLightBookLine == iCounter - 1) {
      GetVideoObject(&hLapTopIconHandle, guiBOOKHIGH);
      BltVideoObject(vsFB, hLapTopIconHandle, 0, BOOK_X,
                     BOOK_TOP_Y + (iCounter * (BOOK_HEIGHT + 6)) + 6);
    } else {
      GetVideoObject(&hLapTopIconHandle, guiBOOKMARK);
      BltVideoObject(vsFB, hLapTopIconHandle, 0, BOOK_X,
                     BOOK_TOP_Y + (iCounter * (BOOK_HEIGHT + 6)) + 6);
    }

    if (iHighLightBookLine == iCounter - 1) {
      // blit in text
      SetFontForeground(FONT_WHITE);
      SetFontBackground(FONT_BLACK);
    } else {
      // blit in text
      SetFontForeground(FONT_BLACK);
      SetFontBackground(FONT_BLACK);
    }

    FindFontCenterCoordinates(
        BOOK_X + 3, (uint16_t)(BOOK_TOP_Y + 2 + (iCounter * (BOOK_HEIGHT + 6)) + 6), BOOK_WIDTH - 3,
        BOOK_HEIGHT + 6, pBookMarkStrings[LaptopSaveInfo.iBookMarkList[iCounter - 1]], BOOK_FONT,
        &sX, &sY);

    mprintf(sX, sY, pBookMarkStrings[LaptopSaveInfo.iBookMarkList[iCounter - 1]]);
    iCounter++;
  }

  // blit one more

  if (iHighLightBookLine == iCounter - 1) {
    GetVideoObject(&hLapTopIconHandle, guiBOOKHIGH);
    BltVideoObject(vsFB, hLapTopIconHandle, 0, BOOK_X,
                   BOOK_TOP_Y + (iCounter * (BOOK_HEIGHT + 6)) + 6);
  } else {
    GetVideoObject(&hLapTopIconHandle, guiBOOKMARK);
    BltVideoObject(vsFB, hLapTopIconHandle, 0, BOOK_X,
                   BOOK_TOP_Y + (iCounter * (BOOK_HEIGHT + 6)) + 6);
  }

  if (iHighLightBookLine == iCounter - 1) {
    // blit in text
    SetFontForeground(FONT_WHITE);
    SetFontBackground(FONT_BLACK);
  } else {
    // blit in text
    SetFontForeground(FONT_BLACK);
    SetFontBackground(FONT_BLACK);
  }
  FindFontCenterCoordinates(
      BOOK_X + 3, (uint16_t)(BOOK_TOP_Y + 2 + (iCounter * (BOOK_HEIGHT + 6)) + 6), BOOK_WIDTH - 3,
      BOOK_HEIGHT + 6, pBookMarkStrings[CANCEL_STRING], BOOK_FONT, &sX, &sY);
  mprintf(sX, sY, pBookMarkStrings[CANCEL_STRING]);
  iCounter++;

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  InvalidateRegion(BOOK_X, BOOK_TOP_Y + ((iCounter)*BOOK_HEIGHT) + 12, BOOK_X + BOOK_WIDTH,
                   BOOK_TOP_Y + ((iCounter + 1) * BOOK_HEIGHT) + 16);
  SetFontShadow(DEFAULT_SHADOW);

  InvalidateRegion(BOOK_X, BOOK_TOP_Y, BOOK_X + BOOK_WIDTH,
                   BOOK_TOP_Y + (iCounter + 6) * BOOK_HEIGHT + 16);
  return;
}

void RemoveBookmark(int32_t iBookId) {
  int32_t iCounter = 0;
  if (iBookId == -2) return;
  while (LaptopSaveInfo.iBookMarkList[iCounter] != -1) {
    if (LaptopSaveInfo.iBookMarkList[iCounter] == iBookId) {
      // found, move everyone back
      for (iCounter = iCounter + 1; iCounter < MAX_BOOKMARKS; iCounter++) {
        LaptopSaveInfo.iBookMarkList[iCounter - 1] = LaptopSaveInfo.iBookMarkList[iCounter];
      }
      return;
    }
    iCounter++;
  }
  return;
}

void DeleteBookmark() {
  DeleteVideoObjectFromIndex(guiBOOKTOP);
  DeleteVideoObjectFromIndex(guiBOOKMID);
  DeleteVideoObjectFromIndex(guiBOOKHIGH);
  DeleteVideoObjectFromIndex(guiBOOKBOT);
  DeleteVideoObjectFromIndex(guiBOOKMARK);

  DeleteVideoObjectFromIndex(guiDOWNLOADTOP);
  DeleteVideoObjectFromIndex(guiDOWNLOADMID);
  DeleteVideoObjectFromIndex(guiDOWNLOADBOT);
}

void ScrollDisplayText(int32_t iY) {
  static int32_t iBaseTime = 0;
  static int16_t sCurX;

  // if we are just enetering, set basetime to current clock value
  if (iBaseTime == 0) iBaseTime = GetJA2Clock();

  // long enough time has passed, shift string
  if (GetJA2Clock() - iBaseTime > SCROLL_DIFFERENCE) {
    // reset postion, if scrolled too far
    if (sCurX < SCROLL_MIN)
      sCurX = BOOK_X + BOOK_WIDTH;
    else
      sCurX--;

    // reset base time
    iBaseTime = GetJA2Clock();
  }

  // font stuff
  SetFontDestBuffer(vsFB, BOOK_X, 0, BOOK_X + BOOK_WIDTH, 480, FALSE);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);

  // print the scrolling string for bookmarks
  mprintf(sCurX, iY, pBookmarkTitle[1]);

  // reset buffer
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  // invalidate region
  InvalidateRegion(BOOK_X, iY, BOOK_X + BOOK_WIDTH, iY + BOOK_HEIGHT);
}
void CreateBookMarkMouseRegions() {
  int32_t iCounter = 0;
  // creates regions based on number of entries
  while (LaptopSaveInfo.iBookMarkList[iCounter] != -1) {
    MSYS_DefineRegion(
        &gBookmarkMouseRegions[iCounter], (int16_t)BOOK_X,
        (uint16_t)(BOOK_TOP_Y + ((iCounter + 1) * (BOOK_HEIGHT + 6)) + 6), BOOK_X + BOOK_WIDTH,
        (int16_t)(BOOK_TOP_Y + ((iCounter + 2) * (BOOK_HEIGHT + 6)) + 6), MSYS_PRIORITY_HIGHEST - 2,
        CURSOR_LAPTOP_SCREEN, BookmarkMvtCallBack, BookmarkCallBack);
    // MSYS_AddRegion(&gBookmarkMouseRegions[iCounter]);
    MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter], 0, iCounter);
    MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter], 1, 0);

    // Create the regions help text
    CreateBookMarkHelpText(&gBookmarkMouseRegions[iCounter],
                           LaptopSaveInfo.iBookMarkList[iCounter]);

    iCounter++;
  }
  // now add one more
  // for the cancel button
  MSYS_DefineRegion(
      &gBookmarkMouseRegions[iCounter], (int16_t)BOOK_X,
      (uint16_t)(BOOK_TOP_Y + ((iCounter + 1) * (BOOK_HEIGHT + 6)) + 6), BOOK_X + BOOK_WIDTH,
      (int16_t)(BOOK_TOP_Y + ((iCounter + 2) * (BOOK_HEIGHT + 6)) + 6), MSYS_PRIORITY_HIGHEST - 2,
      CURSOR_LAPTOP_SCREEN, BookmarkMvtCallBack, BookmarkCallBack);
  // MSYS_AddRegion(&gBookmarkMouseRegions[iCounter]);
  MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter], 0, iCounter);
  MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter], 1, CANCEL_STRING);
}

void DeleteBookmarkRegions() {
  int32_t iCounter = 0;
  // deletes bookmark regions
  while (LaptopSaveInfo.iBookMarkList[iCounter] != -1) {
    MSYS_RemoveRegion(&gBookmarkMouseRegions[iCounter]);
    iCounter++;
  }

  // now one for the cancel
  MSYS_RemoveRegion(&gBookmarkMouseRegions[iCounter]);
}

void CreateDestoryBookMarkRegions(void) {
  // checks to see if a bookmark needs to be created or destroyed
  static BOOLEAN fOldShowBookmarks = FALSE;

  if ((gfShowBookmarks) && (!fOldShowBookmarks)) {
    // create regions
    CreateBookMarkMouseRegions();
    fOldShowBookmarks = TRUE;
  } else if ((!gfShowBookmarks) && (fOldShowBookmarks)) {
    // destroy bookmarks
    DeleteBookmarkRegions();
    fOldShowBookmarks = FALSE;
  }
}

void BookmarkCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iCount;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }

  // we are in process of loading
  if (fLoadPendingFlag == TRUE) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    iCount = MSYS_GetRegionUserData(pRegion, 0);
    if (MSYS_GetRegionUserData(pRegion, 1) == CANCEL_STRING) {
      gfShowBookmarks = FALSE;
      fReDrawScreenFlag = TRUE;
    }
    if (LaptopSaveInfo.iBookMarkList[iCount] != -1) {
      GoToWebPage(LaptopSaveInfo.iBookMarkList[iCount]);
    } else {
      return;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    iCount = MSYS_GetRegionUserData(pRegion, 0);
  }
  return;
}

void GoToWebPage(int32_t iPageId) {
  // if it is raining, popup a warning first saying connection time may be slow
  if (IsItRaining()) {
    if (giRainDelayInternetSite == -1) {
      DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, pErrorStrings[4], LAPTOP_SCREEN, MSG_BOX_FLAG_OK,
                         InternetRainDelayMessageBoxCallBack);
      giRainDelayInternetSite = iPageId;
      return;
    }
  } else
    giRainDelayInternetSite = -1;

  switch (iPageId) {
    case AIM_BOOKMARK:
      guiCurrentWWWMode = LAPTOP_MODE_AIM;
      guiCurrentLaptopMode = LAPTOP_MODE_AIM;

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[AIM_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[AIM_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      break;
    case BOBBYR_BOOKMARK:
      guiCurrentWWWMode = LAPTOP_MODE_BOBBY_R;
      guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[BOBBYR_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[BOBBYR_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      break;
    case (IMP_BOOKMARK):
      guiCurrentWWWMode = LAPTOP_MODE_CHAR_PROFILE;
      guiCurrentLaptopMode = LAPTOP_MODE_CHAR_PROFILE;

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[IMP_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[IMP_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      iCurrentImpPage = IMP_HOME_PAGE;
      break;
    case (MERC_BOOKMARK):

      // if the mercs server has gone down, but hasnt come up yet
      if (LaptopSaveInfo.fMercSiteHasGoneDownYet == TRUE &&
          LaptopSaveInfo.fFirstVisitSinceServerWentDown == FALSE) {
        guiCurrentWWWMode = LAPTOP_MODE_BROKEN_LINK;
        guiCurrentLaptopMode = LAPTOP_MODE_BROKEN_LINK;
      } else {
        guiCurrentWWWMode = LAPTOP_MODE_MERC;
        guiCurrentLaptopMode = LAPTOP_MODE_MERC;
      }

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[MERC_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[MERC_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      break;
    case (FUNERAL_BOOKMARK):
      guiCurrentWWWMode = LAPTOP_MODE_FUNERAL;
      guiCurrentLaptopMode = LAPTOP_MODE_FUNERAL;

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[FUNERAL_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[FUNERAL_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      break;
    case (FLORIST_BOOKMARK):
      guiCurrentWWWMode = LAPTOP_MODE_FLORIST;
      guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[FLORIST_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[FLORIST_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      break;

    case (INSURANCE_BOOKMARK):
      guiCurrentWWWMode = LAPTOP_MODE_INSURANCE;
      guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;

      // do we have to have a World Wide Wait
      if (LaptopSaveInfo.fVisitedBookmarkAlready[INSURANCE_BOOKMARK] == FALSE) {
        // reset flag and set load pending flag
        LaptopSaveInfo.fVisitedBookmarkAlready[INSURANCE_BOOKMARK] = TRUE;
        fLoadPendingFlag = TRUE;
      } else {
        // fast reload
        fLoadPendingFlag = TRUE;
        fFastLoadFlag = TRUE;
      }
      break;
  }

  gfShowBookmarks = FALSE;
  fReDrawScreenFlag = TRUE;
  return;
}

void BookmarkMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason == MSYS_CALLBACK_REASON_MOVE) {
    iHighLightBookLine = MSYS_GetRegionUserData(pRegion, 0);
  }
  if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE) {
    iHighLightBookLine = -1;
  }
}

BOOLEAN LoadLoadPending(void) {
  // function will load the load pending graphics
  // reuse bookmark
  // load graph window and bar

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\graphwindow.sti"), &guiGRAPHWINDOW));
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\graphsegment.sti"), &guiGRAPHBAR));

  return (TRUE);
}

BOOLEAN DisplayLoadPending(void) {
  // this function will display the load pending and return if the load is done
  static int32_t iBaseTime = 0;
  static int32_t iTotalTime = 0;
  int32_t iTempTime = 0;
  int32_t iCounter = 0;
  int32_t iDifference = 0;
  struct VObject *hLapTopIconHandle;
  int32_t iLoadTime;
  int32_t iUnitTime;
  int16_t sXPosition = 0, sYPosition = 0;

  // if merc webpage, make it longer
  // TEMP disables the loadpending
  if (gfTemporaryDisablingOfLoadPendingFlag) {
    iLoadTime = 1;
    iUnitTime = 1;
  } else {
    if ((fFastLoadFlag == TRUE) && (fConnectingToSubPage == TRUE)) {
      iUnitTime = FASTEST_UNIT_TIME;
    } else if (fFastLoadFlag == TRUE) {
      iUnitTime = FAST_UNIT_TIME;
    } else if (fConnectingToSubPage == TRUE) {
      iUnitTime = ALMOST_FAST_UNIT_TIME;
    }

    // if we are connecting the MERC site, and the MERC site hasnt yet moved to their new site, have
    // the sloooww wait
    else if (guiCurrentLaptopMode == LAPTOP_MODE_MERC && !LaptopSaveInfo.fMercSiteHasGoneDownYet) {
      iUnitTime = LONG_UNIT_TIME;
    } else {
      iUnitTime = UNIT_TIME;
    }

    iUnitTime += WWaitDelayIncreasedIfRaining(iUnitTime);

    iLoadTime = iUnitTime * 30;
  }

  // we are now waiting on a web page to download, reset counter
  if (!fLoadPendingFlag) {
    fDoneLoadPending = FALSE;
    fFastLoadFlag = FALSE;
    fConnectingToSubPage = FALSE;
    iBaseTime = 0;
    iTotalTime = 0;
    return (FALSE);
  }
  // if total time is exceeded, return (TRUE)
  if (iBaseTime == 0) {
    iBaseTime = GetJA2Clock();
  }

  if (iTotalTime >= iLoadTime) {
    // done loading, redraw screen
    fLoadPendingFlag = FALSE;
    fFastLoadFlag = FALSE;
    iTotalTime = 0;
    iBaseTime = 0;
    fDoneLoadPending = TRUE;
    fConnectingToSubPage = FALSE;
    fPausedReDrawScreenFlag = TRUE;

    return (TRUE);
  }

  iDifference = GetJA2Clock() - iBaseTime;

  // difference has been long enough or we are redrawing the screen
  if ((iDifference) > iUnitTime) {
    // LONG ENOUGH TIME PASSED
    iCounter = 0;
    iBaseTime = GetJA2Clock();
    iTotalTime += iDifference;
    iTempTime = iTotalTime;
  }

  // new mail, don't redraw
  if (fNewMailFlag == TRUE) {
    return (FALSE);
  }

  RenderButtons();

  //	RenderFastHelp( );
  //	RenderButtonsFastHelp( );

  // display top middle and bottom of box
  GetVideoObject(&hLapTopIconHandle, guiDOWNLOADTOP);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, DOWNLOAD_X, DOWNLOAD_Y);
  GetVideoObject(&hLapTopIconHandle, guiDOWNLOADMID);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, DOWNLOAD_X, DOWNLOAD_Y + DOWN_HEIGHT);
  GetVideoObject(&hLapTopIconHandle, guiDOWNLOADBOT);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, DOWNLOAD_X, DOWNLOAD_Y + 2 * DOWN_HEIGHT);
  GetVideoObject(&hLapTopIconHandle, guiTITLEBARICONS);
  BltVideoObject(vsFB, hLapTopIconHandle, 1, DOWNLOAD_X + 4, DOWNLOAD_Y + 1);

  // font stuff
  SetFont(DOWNLOAD_FONT);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // reload or download?
  if (fFastLoadFlag == TRUE) {
    FindFontCenterCoordinates(328, 0, 446 - 328, 0, pDownloadString[1], DOWNLOAD_FONT, &sXPosition,
                              &sYPosition);

    // display download string
    mprintf(sXPosition, DOWN_STRING_Y, pDownloadString[1]);
  } else {
    FindFontCenterCoordinates(328, 0, 446 - 328, 0, pDownloadString[0], DOWNLOAD_FONT, &sXPosition,
                              &sYPosition);

    // display download string
    mprintf(sXPosition, DOWN_STRING_Y, pDownloadString[0]);
  }

  // get and blt the window video object
  GetVideoObject(&hLapTopIconHandle, guiGRAPHWINDOW);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, LAPTOP_WINDOW_X, LAPTOP_WINDOW_Y);

  // check to see if we are only updating screen, but not passed a new element in the load pending
  // display

  iTempTime = iTotalTime;
  // decide how many time units are to be displayed, based on amount of time passed
  while (iTempTime > 0) {
    GetVideoObject(&hLapTopIconHandle, guiGRAPHBAR);
    BltVideoObject(vsFB, hLapTopIconHandle, 0, LAPTOP_BAR_X + (UNIT_WIDTH * iCounter),
                   LAPTOP_BAR_Y);
    iTempTime -= iUnitTime;
    iCounter++;

    // have we gone too far?
    if (iCounter > 30) {
      iTempTime = 0;
    }
  }

  InvalidateRegion(DOWNLOAD_X, DOWNLOAD_Y, DOWNLOAD_X + 150, DOWNLOAD_Y + 100);

  // re draw screen and new mail warning box
  SetFontShadow(DEFAULT_SHADOW);

  MarkButtonsDirty();

  DisableMercSiteButton();

  return (FALSE);
}

void DeleteLoadPending(void) {
  // this funtion will delete the load pending graphics
  // reuse bookmark
  DeleteVideoObjectFromIndex(guiGRAPHBAR);
  DeleteVideoObjectFromIndex(guiGRAPHWINDOW);
  return;
}

void BtnErrorCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
    }
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fErrorFlag = FALSE;
    }
  }
}
void CreateDestroyErrorButton(void) {
  static BOOLEAN fOldErrorFlag = FALSE;
  if ((fErrorFlag) && (!fOldErrorFlag)) {
    // create inventory button
    fOldErrorFlag = TRUE;

    // load image and create error confirm button
    giErrorButtonImage[0] = LoadButtonImage("LAPTOP\\errorbutton.sti", -1, 0, -1, 1, -1);
    giErrorButton[0] =
        QuickCreateButton(giErrorButtonImage[0], ERROR_X + ERROR_BTN_X, ERROR_BTN_Y, BUTTON_TOGGLE,
                          MSYS_PRIORITY_HIGHEST, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                          (GUI_CALLBACK)BtnErrorCallback);

    // define the cursor
    SetButtonCursor(giErrorButton[0], CURSOR_LAPTOP_SCREEN);

    // define the screen mask
    MSYS_DefineRegion(&pScreenMask, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN,
                      MSYS_NO_CALLBACK, LapTopScreenCallBack);

    // add region
    MSYS_AddRegion(&pScreenMask);
  } else if ((!fErrorFlag) && (fOldErrorFlag)) {
    // done dsiplaying, get rid of button and screen mask
    fOldErrorFlag = FALSE;

    RemoveButton(giErrorButton[0]);
    UnloadButtonImage(giErrorButtonImage[0]);

    MSYS_RemoveRegion(&pScreenMask);

    // re draw screen
    fReDrawScreenFlag = TRUE;
  }
  return;
}

void DisplayErrorBox(void) {
  // this function will display the error graphic
  struct VObject *hLapTopIconHandle;
  if (!fErrorFlag) return;

  // get and blt top portion
  GetVideoObject(&hLapTopIconHandle, guiBOOKTOP);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y);

  // middle * 5
  GetVideoObject(&hLapTopIconHandle, guiBOOKMID);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y + BOOK_HEIGHT);

  GetVideoObject(&hLapTopIconHandle, guiBOOKMID);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y + 2 * BOOK_HEIGHT);

  GetVideoObject(&hLapTopIconHandle, guiBOOKMID);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y + 3 * BOOK_HEIGHT);

  GetVideoObject(&hLapTopIconHandle, guiBOOKMID);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y + 4 * BOOK_HEIGHT);

  GetVideoObject(&hLapTopIconHandle, guiBOOKMID);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y + 5 * BOOK_HEIGHT);

  // the bottom
  GetVideoObject(&hLapTopIconHandle, guiBOOKBOT);
  BltVideoObject(vsFB, hLapTopIconHandle, 0, ERROR_X, ERROR_Y + 6 * BOOK_HEIGHT);

  // font stuff
  SetFont(ERROR_TITLE_FONT);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // print title
  mprintf(ERROR_TITLE_X, ERROR_TITLE_Y, pErrorStrings[0]);
  SetFontForeground(FONT_BLACK);
  SetFont(ERROR_FONT);

  // display error string
  DisplayWrappedString(
      ERROR_X + ERROR_TEXT_X,
      (uint16_t)(ERROR_Y + ERROR_TEXT_Y +
                 DisplayWrappedString(ERROR_X + ERROR_TEXT_X, ERROR_Y + ERROR_TEXT_Y, BOOK_WIDTH, 2,
                                      ERROR_FONT, FONT_BLACK, pErrorStrings[1], FONT_BLACK, FALSE,
                                      CENTER_JUSTIFIED)),
      BOOK_WIDTH, 2, ERROR_FONT, FONT_BLACK, pErrorStrings[2], FONT_BLACK, FALSE, CENTER_JUSTIFIED);

  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void DrawTextOnErrorButton() {
  // draws text on error button
  SetFont(ERROR_TITLE_FONT);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);
  mprintf(ERROR_X + ERROR_BTN_X + ERROR_BTN_TEXT_X, ERROR_BTN_Y + ERROR_BTN_TEXT_Y,
          pErrorStrings[3]);
  SetFontShadow(DEFAULT_SHADOW);

  InvalidateRegion(ERROR_X, ERROR_Y, ERROR_X + BOOK_WIDTH, ERROR_Y + 6 * BOOK_HEIGHT);
  return;
}

// This function is called every time the laptop is FIRST initialized, ie whenever the laptop is
// loaded.  It calls various init function in the laptop pages that need to be inited when the
// laptop is just loaded.
void EnterLaptopInitLaptopPages() {
  EnterInitAimMembers();
  EnterInitAimArchives();
  EnterInitAimPolicies();
  EnterInitAimHistory();
  EnterInitFloristGallery();
  EnterInitInsuranceInfo();
  EnterInitBobbyRayOrder();
  EnterInitMercSite();

  // init sub pages for WW Wait
  InitIMPSubPageList();
}

void CheckMarkButtonsDirtyFlag(void) {
  // this function checks the fMarkButtonsDirtyFlag, if true, mark buttons dirty
  if (fMarkButtonsDirtyFlag) {
    // flag set, mark buttons and reset
    MarkButtonsDirty();
    fMarkButtonsDirtyFlag = FALSE;
  }

  return;
}

void PostButtonRendering(void) {
  // this function is in place to allow for post button rendering

  switch (guiCurrentLaptopMode) {
    case LAPTOP_MODE_AIM:
      //	    RenderCharProfilePostButton( );
      break;

    case LAPTOP_MODE_AIM_MEMBERS:
      RenderAIMMembersTopLevel();
      break;
  }
  return;
}

void ShouldNewMailBeDisplayed() {
  switch (guiCurrentLaptopMode) {
    case LAPTOP_MODE_AIM_MEMBERS:
      DisableNewMailMessage();
      break;
  }
}

void DisplayPlayersBalanceToDate(void) {
  // print players balance to date
  wchar_t sString[100];
  int16_t sX, sY;

  // initialize string
  memset(sString, 0, sizeof(sString));

  // font stuff
  SetFont(FONT10ARIAL);
  SetFontForeground(142);
  SetFontShadow(NO_SHADOW);

  // parse straigth number
  swprintf(sString, ARR_SIZE(sString), L"%d", MoneyGetBalance());

  // put in commas, then dollar sign
  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);

  // get center
  FindFontCenterCoordinates((int16_t)LAPTOP_ICON_TEXT_X, 0, (int16_t)(LAPTOP_ICON_TEXT_WIDTH),
                            (int16_t)(LAPTOP_ICON_TEXT_HEIGHT), sString, LAPTOPICONFONT, &sX, &sY);
  //	gprintfdirty( sX , LAPTOP_ICON_TEXT_FINANCIAL_Y + 10, sString );
  // printf it!
  if (ButtonList[gLaptopButton[5]]->uiFlags & BUTTON_CLICKED_ON) {
    mprintf(sX + 5, LAPTOP_ICON_TEXT_FINANCIAL_Y + 10 + 5, sString);
  } else {
    mprintf(sX, LAPTOP_ICON_TEXT_FINANCIAL_Y + 10, sString);
  }

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void CheckIfNewWWWW(void) {
  // if no www mode, set new www flag..until new www mode that is not 0

  if (guiCurrentWWWMode == LAPTOP_MODE_NONE) {
    fNewWWW = TRUE;
  } else {
    fNewWWW = FALSE;
  }

  return;
}

void HandleLapTopESCKey(void) {
  // will handle esc key events, since handling depends on state of laptop

  if (fNewMailFlag) {
    // get rid of new mail warning box
    fNewMailFlag = FALSE;
    CreateDestroyNewMailButton();

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLaptop();
  } else if (fDeleteMailFlag) {
    // get rid of delete mail box
    fDeleteMailFlag = FALSE;
    CreateDestroyDeleteNoticeMailButton();

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLaptop();
  } else if (fErrorFlag) {
    // get rid of error warning box
    fErrorFlag = FALSE;
    CreateDestroyErrorButton();

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLaptop();
  }

  else if (gfShowBookmarks) {
    // get rid of bookmarks
    gfShowBookmarks = FALSE;

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLapTopImage();
    RenderLaptop();
  } else {
    // leave
    fExitingLaptopFlag = TRUE;
    HandleExit();
  }

  return;
}

void HandleRightButtonUpEvent(void) {
  // will handle the right button up event
  if (fNewMailFlag) {
    // get rid of new mail warning box
    fNewMailFlag = FALSE;
    CreateDestroyNewMailButton();

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLaptop();
  } else if (fDeleteMailFlag) {
    // get rid of delete mail box
    fDeleteMailFlag = FALSE;
    CreateDestroyDeleteNoticeMailButton();

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLaptop();
  } else if (fErrorFlag) {
    // get rid of error warning box
    fErrorFlag = FALSE;
    CreateDestroyErrorButton();

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLaptop();
  }

  else if (gfShowBookmarks) {
    // get rid of bookmarks
    gfShowBookmarks = FALSE;

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLapTopImage();
    RenderLaptop();
  } else if (fDisplayMessageFlag) {
    fDisplayMessageFlag = FALSE;

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLapTopImage();
    RenderLaptop();

  } else if (fShowBookmarkInfo) {
    fShowBookmarkInfo = FALSE;
  }
}

void HandleLeftButtonUpEvent(void) {
  // will handle the left button up event

  if (gfShowBookmarks) {
    // get rid of bookmarks
    gfShowBookmarks = FALSE;

    // force redraw
    fReDrawScreenFlag = TRUE;
    RenderLapTopImage();
    RenderLaptop();
  } else if (fShowBookmarkInfo) {
    fShowBookmarkInfo = FALSE;
  }
}

void LapTopScreenCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    HandleLeftButtonUpEvent();
    return;
  }
  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    HandleRightButtonUpEvent();
    return;
  }

  return;
}

BOOLEAN DoLapTopMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                           uint8_t ubFlags, MSGBOX_CALLBACK ReturnCallback) {
  SGPRect pCenteringRect = {LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y, LAPTOP_SCREEN_LR_X,
                            LAPTOP_SCREEN_LR_Y};

  // reset exit mode
  fExitDueToMessageBox = TRUE;

  // do message box and return
  iLaptopMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                   (uint8_t)(ubFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                   ReturnCallback, &pCenteringRect);

  // send back return state
  return ((iLaptopMessageBox != -1));
}

BOOLEAN DoLapTopSystemMessageBoxWithRect(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                                         uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                                         const SGPRect *pCenteringRect) {
  // reset exit mode
  fExitDueToMessageBox = TRUE;

  // do message box and return
  iLaptopMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                   (uint16_t)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                   ReturnCallback, pCenteringRect);

  // send back return state
  return ((iLaptopMessageBox != -1));
}

BOOLEAN DoLapTopSystemMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                                 uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback) {
  // reset exit mode
  fExitDueToMessageBox = TRUE;

  // do message box and return
  iLaptopMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                   (uint16_t)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                   ReturnCallback, GetMapCenteringRect());

  // send back return state
  return ((iLaptopMessageBox != -1));
}

// places a tileable pattern down
BOOLEAN WebPageTileBackground(uint8_t ubNumX, uint8_t ubNumY, uint16_t usWidth, uint16_t usHeight,
                              uint32_t uiBackgroundIdentifier) {
  struct VObject *hBackGroundHandle;
  uint16_t x, y, uiPosX, uiPosY;

  // Blt the Wood background
  GetVideoObject(&hBackGroundHandle, uiBackgroundIdentifier);

  uiPosY = LAPTOP_SCREEN_WEB_UL_Y;
  for (y = 0; y < ubNumY; y++) {
    uiPosX = LAPTOP_SCREEN_UL_X;
    for (x = 0; x < ubNumX; x++) {
      BltVideoObject(vsFB, hBackGroundHandle, 0, uiPosX, uiPosY);
      uiPosX += usWidth;
    }
    uiPosY += usHeight;
  }
  return (TRUE);
}

BOOLEAN InitTitleBarMaximizeGraphics(uint32_t uiBackgroundGraphic, wchar_t *pTitle,
                                     uint32_t uiIconGraphic, uint16_t usIconGraphicIndex) {
  struct VObject *hImageHandle;

  Assert(uiBackgroundGraphic);

  // Create a background video surface to blt the title bar onto
  vsTitleBarSurface = JSurface_Create16bpp(LAPTOP_TITLE_BAR_WIDTH, LAPTOP_TITLE_BAR_HEIGHT);
  JSurface_SetColorKey(vsTitleBarSurface, FROMRGB(0, 0, 0));

  // blit the toolbar grapgucs onto the surface
  GetVideoObject(&hImageHandle, uiBackgroundGraphic);
  BltVideoObject(vsTitleBarSurface, hImageHandle, 0, 0, 0);

  // blit th icon onto the tool bar
  GetVideoObject(&hImageHandle, uiIconGraphic);
  BltVideoObject(vsTitleBarSurface, hImageHandle, usIconGraphicIndex,
                 LAPTOP_TITLE_BAR_ICON_OFFSET_X, LAPTOP_TITLE_BAR_ICON_OFFSET_Y);

  SetFontDestBuffer(vsTitleBarSurface, 0, 0, LAPTOP_TITLE_BAR_WIDTH, LAPTOP_TITLE_BAR_HEIGHT,
                    FALSE);
  DrawTextToScreen(pTitle, LAPTOP_TITLE_BAR_TEXT_OFFSET_X, LAPTOP_TITLE_BAR_TEXT_OFFSET_Y, 0,
                   FONT14ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  return (TRUE);
}

BOOLEAN DisplayTitleBarMaximizeGraphic(BOOLEAN fForward, BOOLEAN fInit, uint16_t usTopLeftX,
                                       uint16_t usTopLeftY, uint16_t usTopRightX) {
  static int8_t ubCount;
  int16_t sPosX, sPosY, sPosRightX, sPosBottomY, sWidth, sHeight;
  SGPRect SrcRect;
  SGPRect DestRect;
  static SGPRect LastRect;
  float dTemp;

  if (fInit) {
    if (gfTitleBarSurfaceAlreadyActive) return (FALSE);

    gfTitleBarSurfaceAlreadyActive = TRUE;
    if (fForward) {
      ubCount = 1;
    } else {
      ubCount = NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS - 1;
    }
  }

  dTemp = (LAPTOP_TITLE_BAR_TOP_LEFT_X - usTopLeftX) / (float)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
  sPosX = (int16_t)(usTopLeftX + dTemp * ubCount);

  dTemp =
      (LAPTOP_TITLE_BAR_TOP_RIGHT_X - usTopRightX) / (float)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
  sPosRightX = (int16_t)(usTopRightX + dTemp * ubCount);

  dTemp = (LAPTOP_TITLE_BAR_TOP_LEFT_Y - usTopLeftY) / (float)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
  sPosY = (int16_t)(usTopLeftY + dTemp * ubCount);

  sPosBottomY = LAPTOP_TITLE_BAR_HEIGHT;

  SrcRect.iLeft = 0;
  SrcRect.iTop = 0;
  SrcRect.iRight = LAPTOP_TITLE_BAR_WIDTH;
  SrcRect.iBottom = LAPTOP_TITLE_BAR_HEIGHT;

  // if its the last fram, bit the tittle bar to the final position
  if (ubCount == NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS) {
    DestRect.iLeft = LAPTOP_TITLE_BAR_TOP_LEFT_X;
    DestRect.iTop = LAPTOP_TITLE_BAR_TOP_LEFT_Y;
    DestRect.iRight = LAPTOP_TITLE_BAR_TOP_RIGHT_X;
    DestRect.iBottom = DestRect.iTop + sPosBottomY;
  } else {
    DestRect.iLeft = sPosX;
    DestRect.iTop = sPosY;
    DestRect.iRight = sPosRightX;
    DestRect.iBottom = DestRect.iTop + sPosBottomY;
  }

  if (fForward) {
    // Restore the old rect
    if (ubCount > 1) {
      sWidth = (uint16_t)(LastRect.iRight - LastRect.iLeft);
      sHeight = (uint16_t)(LastRect.iBottom - LastRect.iTop);
      BlitBufferToBuffer(vsSaveBuffer, vsFB, (uint16_t)LastRect.iLeft, (uint16_t)LastRect.iTop,
                         sWidth, sHeight);
    }

    // Save rectangle
    if (ubCount > 0) {
      sWidth = (uint16_t)(DestRect.iRight - DestRect.iLeft);
      sHeight = (uint16_t)(DestRect.iBottom - DestRect.iTop);
      BlitBufferToBuffer(vsFB, vsSaveBuffer, (uint16_t)DestRect.iLeft, (uint16_t)DestRect.iTop,
                         sWidth, sHeight);
    }
  } else {
    // Restore the old rect
    if (ubCount < NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS - 1) {
      sWidth = (uint16_t)(LastRect.iRight - LastRect.iLeft);
      sHeight = (uint16_t)(LastRect.iBottom - LastRect.iTop);
      BlitBufferToBuffer(vsSaveBuffer, vsFB, (uint16_t)LastRect.iLeft, (uint16_t)LastRect.iTop,
                         sWidth, sHeight);
    }

    // Save rectangle
    if (ubCount < NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS) {
      sWidth = (uint16_t)(DestRect.iRight - DestRect.iLeft);
      sHeight = (uint16_t)(DestRect.iBottom - DestRect.iTop);
      BlitBufferToBuffer(vsFB, vsSaveBuffer, (uint16_t)DestRect.iLeft, (uint16_t)DestRect.iTop,
                         sWidth, sHeight);
    }
  }

  BltStretchVSurface(vsFB, vsTitleBarSurface, &SrcRect, &DestRect);

  InvalidateRegion(DestRect.iLeft, DestRect.iTop, DestRect.iRight, DestRect.iBottom);
  InvalidateRegion(LastRect.iLeft, LastRect.iTop, LastRect.iRight, LastRect.iBottom);

  LastRect = DestRect;

  if (fForward) {
    if (ubCount == NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS) {
      gfTitleBarSurfaceAlreadyActive = FALSE;
      return (TRUE);
    } else {
      ubCount++;
      return (FALSE);
    }
  } else {
    if (ubCount == 0) {
      gfTitleBarSurfaceAlreadyActive = FALSE;
      return (TRUE);
    } else {
      ubCount--;
      return (FALSE);
    }
  }

  return (TRUE);
}

void RemoveTitleBarMaximizeGraphics() {
  JSurface_Free(vsTitleBarSurface);
  vsTitleBarSurface = NULL;
}

void HandleSlidingTitleBar(void) {
  if ((fMaximizingProgram == FALSE) && (fMinizingProgram == FALSE)) {
    return;
  }

  if (fExitingLaptopFlag) {
    return;
  }

  if (fMaximizingProgram) {
    switch (bProgramBeingMaximized) {
      case (LAPTOP_PROGRAM_MAILER):
        fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, 29, 66, 29 + 20);
        if (fMaximizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_FILES):
        fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, 29, 120, 29 + 20);
        if (fMaximizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_FINANCES):
        fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, 29, 226, 29 + 20);
        if (fMaximizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_PERSONNEL):
        fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, 29, 194, 29 + 20);
        if (fMaximizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_HISTORY):
        fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, 29, 162, 29 + 20);
        if (fMaximizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_WEB_BROWSER):
        fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, 29, 99, 29 + 20);
        if (fMaximizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
    }

    MarkButtonsDirty();
  } else {
    // minimizing
    switch (bProgramBeingMaximized) {
      case (LAPTOP_PROGRAM_MAILER):
        fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, 29, 66, 29 + 20);
        if (fMinizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_FILES):
        fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, 29, 130, 29 + 20);
        if (fMinizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_FINANCES):
        fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, 29, 226, 29 + 20);
        if (fMinizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_PERSONNEL):
        fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, 29, 194, 29 + 20);
        if (fMinizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_HISTORY):
        fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, 29, 162, 29 + 20);
        if (fMinizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
      case (LAPTOP_PROGRAM_WEB_BROWSER):
        fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, 29, 99, 29 + 20);
        if (fMinizingProgram == FALSE) {
          RemoveTitleBarMaximizeGraphics();
          EnterNewLaptopMode();
          fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
          fPausedReDrawScreenFlag = TRUE;
        }
        break;
    }
  }

  // reset init
  fInitTitle = FALSE;
}

void ShowLights(void) {
  // will show lights depending on state
  struct VObject *hHandle;

  if (fPowerLightOn == TRUE) {
    GetVideoObject(&hHandle, guiLIGHTS);
    BltVideoObject(vsFB, hHandle, 0, 44, 466);
  } else {
    GetVideoObject(&hHandle, guiLIGHTS);
    BltVideoObject(vsFB, hHandle, 1, 44, 466);
  }

  if (fHardDriveLightOn == TRUE) {
    GetVideoObject(&hHandle, guiLIGHTS);
    BltVideoObject(vsFB, hHandle, 0, 88, 466);
  } else {
    GetVideoObject(&hHandle, guiLIGHTS);
    BltVideoObject(vsFB, hHandle, 1, 88, 466);
  }
}

void FlickerHDLight(void) {
  static int32_t iBaseTime = 0;
  static int32_t iTotalDifference = 0;
  int32_t iDifference = 0;

  if (fLoadPendingFlag == TRUE) {
    fFlickerHD = TRUE;
  }

  if (fFlickerHD == FALSE) {
    return;
  }

  if (iBaseTime == 0) {
    iBaseTime = GetJA2Clock();
  }

  iDifference = GetJA2Clock() - iBaseTime;

  if ((iTotalDifference > HD_FLICKER_TIME) && (fLoadPendingFlag == FALSE)) {
    iBaseTime = GetJA2Clock();
    fHardDriveLightOn = FALSE;
    iBaseTime = 0;
    iTotalDifference = 0;
    fFlickerHD = FALSE;
    InvalidateRegion(88, 466, 102, 477);
    return;
  }

  if (iDifference > FLICKER_TIME) {
    iTotalDifference += iDifference;

    if (fLoadPendingFlag == TRUE) {
      iTotalDifference = 0;
    }

    if ((Random(2)) == 0) {
      fHardDriveLightOn = TRUE;
    } else {
      fHardDriveLightOn = FALSE;
    }
    InvalidateRegion(88, 466, 102, 477);
  }

  return;
}

BOOLEAN ExitLaptopDone(void) {
  // check if this is the first time, to reset counter

  static BOOLEAN fOldLeaveLaptopState = FALSE;
  static int32_t iBaseTime = 0;
  int32_t iDifference = 0;

  if (fOldLeaveLaptopState == FALSE) {
    fOldLeaveLaptopState = TRUE;
    iBaseTime = GetJA2Clock();
  }

  fPowerLightOn = FALSE;

  InvalidateRegion(44, 466, 58, 477);
  // get the current difference
  iDifference = GetJA2Clock() - iBaseTime;

  // did we wait long enough?
  if (iDifference > EXIT_LAPTOP_DELAY_TIME) {
    iBaseTime = 0;
    fOldLeaveLaptopState = FALSE;
    return TRUE;
  } else {
    return FALSE;
  }
}

void CreateDestroyMinimizeButtonForCurrentMode(void) {
  // will create the minimize button

  static BOOLEAN fAlreadyCreated = FALSE;
  // check to see if created, if so, do nothing

  // check current mode
  if ((guiCurrentLaptopMode == LAPTOP_MODE_NONE) && (guiPreviousLaptopMode != LAPTOP_MODE_NONE)) {
    fCreateMinimizeButton = FALSE;
  } else if ((guiCurrentLaptopMode != LAPTOP_MODE_NONE)) {
    fCreateMinimizeButton = TRUE;
  } else if ((guiPreviousLaptopMode != LAPTOP_MODE_NONE)) {
    fCreateMinimizeButton = FALSE;
  }

  // leaving laptop, get rid of the button
  if (fExitingLaptopFlag == TRUE) {
    fCreateMinimizeButton = FALSE;
  }

  if ((fAlreadyCreated == FALSE) && (fCreateMinimizeButton == TRUE)) {
    // not created, create
    fAlreadyCreated = TRUE;
    CreateMinimizeButtonForCurrentMode();
    CreateMinimizeRegionsForLaptopProgramIcons();
  } else if ((fAlreadyCreated == TRUE) && (fCreateMinimizeButton == FALSE)) {
    // created and must be destroyed
    fAlreadyCreated = FALSE;
    DestroyMinimizeButtonForCurrentMode();
    DestroyMinimizeRegionsForLaptopProgramIcons();

  } else {
    // do nothing
  }

  return;
}

void CreateMinimizeButtonForCurrentMode(void) {
  // create minimize button
  gLaptopMinButtonImage[0] = LoadButtonImage("LAPTOP\\x.sti", -1, 0, -1, 1, -1);
  gLaptopMinButton[0] =
      QuickCreateButton(gLaptopMinButtonImage[0], 590, 30, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
                        (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)LaptopMinimizeProgramButtonCallback);

  SetButtonCursor(gLaptopMinButton[0], CURSOR_LAPTOP_SCREEN);
  return;
}

void DestroyMinimizeButtonForCurrentMode(void) {
  // destroy minimize button
  RemoveButton(gLaptopMinButton[0]);
  UnloadButtonImage(gLaptopMinButtonImage[0]);
}

void LaptopMinimizeProgramButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      btn->uiFlags |= (BUTTON_CLICKED_ON);
    }
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      switch (guiCurrentLaptopMode) {
        case (LAPTOP_MODE_EMAIL):
          gLaptopProgramStates[LAPTOP_PROGRAM_MAILER] = LAPTOP_PROGRAM_MINIMIZED;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[0], guiTITLEBARICONS, 0);
          SetCurrentToLastProgramOpened();
          fMinizingProgram = TRUE;
          fInitTitle = TRUE;
          break;
        case (LAPTOP_MODE_FILES):
          gLaptopProgramStates[LAPTOP_PROGRAM_FILES] = LAPTOP_PROGRAM_MINIMIZED;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[5], guiTITLEBARICONS, 2);
          SetCurrentToLastProgramOpened();
          fMinizingProgram = TRUE;
          fInitTitle = TRUE;
          break;
        case (LAPTOP_MODE_FINANCES):
          gLaptopProgramStates[LAPTOP_PROGRAM_FINANCES] = LAPTOP_PROGRAM_MINIMIZED;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[2], guiTITLEBARICONS, 5);
          SetCurrentToLastProgramOpened();
          fMinizingProgram = TRUE;
          fInitTitle = TRUE;
          break;
        case (LAPTOP_MODE_HISTORY):
          gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY] = LAPTOP_PROGRAM_MINIMIZED;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[4], guiTITLEBARICONS, 4);
          SetCurrentToLastProgramOpened();
          fMinizingProgram = TRUE;
          fInitTitle = TRUE;
          break;
        case (LAPTOP_MODE_PERSONNEL):
          gLaptopProgramStates[LAPTOP_PROGRAM_PERSONNEL] = LAPTOP_PROGRAM_MINIMIZED;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[3], guiTITLEBARICONS, 3);
          SetCurrentToLastProgramOpened();
          fMinizingProgram = TRUE;
          fInitTitle = TRUE;
          break;
        case (LAPTOP_MODE_NONE):
          // nothing
          break;
        default:
          gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] = LAPTOP_PROGRAM_MINIMIZED;
          InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[7], guiTITLEBARICONS, 1);
          SetCurrentToLastProgramOpened();
          gfShowBookmarks = FALSE;
          fMinizingProgram = TRUE;
          fInitTitle = TRUE;
          break;
      }
    }
  }
}

int32_t FindLastProgramStillOpen(void) {
  int32_t iLowestValue = 6;
  int32_t iLowestValueProgram = 6;
  int32_t iCounter = 0;

  // returns ID of last program open and not minimized
  for (iCounter = 0; iCounter < 6; iCounter++) {
    if (gLaptopProgramStates[iCounter] != LAPTOP_PROGRAM_MINIMIZED) {
      if (gLaptopProgramQueueList[iCounter] < iLowestValue) {
        iLowestValue = gLaptopProgramQueueList[iCounter];
        iLowestValueProgram = iCounter;
      }
    }
  }

  return (iLowestValueProgram);
}

void UpdateListToReflectNewProgramOpened(int32_t iOpenedProgram) {
  int32_t iCounter = 0;

  // will update queue of opened programs to show thier states
  // set iOpenedProgram to 1, and update others

  // increment everyone
  for (iCounter = 0; iCounter < 6; iCounter++) {
    gLaptopProgramQueueList[iCounter]++;
  }

  gLaptopProgramQueueList[iOpenedProgram] = 1;

  return;
}

void InitLaptopOpenQueue(void) {
  int32_t iCounter = 0;

  // set evereyone to 1
  for (iCounter = 0; iCounter < 6; iCounter++) {
    gLaptopProgramQueueList[iCounter] = 1;
  }

  return;
}

void SetCurrentToLastProgramOpened(void) {
  guiCurrentLaptopMode = LAPTOP_MODE_NONE;

  switch (FindLastProgramStillOpen()) {
    case (LAPTOP_PROGRAM_HISTORY):
      guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
      break;
    case (LAPTOP_PROGRAM_MAILER):
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
      break;
    case (LAPTOP_PROGRAM_PERSONNEL):
      guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
      break;
    case (LAPTOP_PROGRAM_FINANCES):
      guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
      break;
    case (LAPTOP_PROGRAM_FILES):
      guiCurrentLaptopMode = LAPTOP_MODE_FILES;
      break;
    case (LAPTOP_PROGRAM_WEB_BROWSER):
      // last www mode
      if (guiCurrentWWWMode != 0) {
        guiCurrentLaptopMode = guiCurrentWWWMode;
      } else {
        guiCurrentLaptopMode = LAPTOP_MODE_WWW;
      }
      // gfShowBookmarks = TRUE;
      fShowBookmarkInfo = TRUE;
      break;
  }
}

void BlitTitleBarIcons(void) {
  struct VObject *hHandle;
  // will blit the icons for the title bar of the program we are in
  switch (guiCurrentLaptopMode) {
    case (LAPTOP_MODE_HISTORY):
      GetVideoObject(&hHandle, guiTITLEBARICONS);
      BltVideoObject(vsFB, hHandle, 4, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
      break;
    case (LAPTOP_MODE_EMAIL):
      GetVideoObject(&hHandle, guiTITLEBARICONS);
      BltVideoObject(vsFB, hHandle, 0, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
      break;
    case (LAPTOP_MODE_PERSONNEL):
      GetVideoObject(&hHandle, guiTITLEBARICONS);
      BltVideoObject(vsFB, hHandle, 3, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
      break;
    case (LAPTOP_MODE_FINANCES):
      GetVideoObject(&hHandle, guiTITLEBARICONS);
      BltVideoObject(vsFB, hHandle, 5, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
      break;
    case (LAPTOP_MODE_FILES):
      GetVideoObject(&hHandle, guiTITLEBARICONS);
      BltVideoObject(vsFB, hHandle, 2, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
      break;
    case (LAPTOP_MODE_NONE):
      // do nothing
      break;
    default:
      // www pages
      GetVideoObject(&hHandle, guiTITLEBARICONS);
      BltVideoObject(vsFB, hHandle, 1, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
      break;
  }
}

BOOLEAN DrawDeskTopBackground(void) {
  uint32_t uiDestPitchBYTES;
  uint32_t uiSrcPitchBYTES;
  uint16_t *pDestBuf;
  uint8_t *pSrcBuf;
  SGPRect clip;

  // set clipping region
  clip.iLeft = 0;
  clip.iRight = 506;
  clip.iTop = 0;
  clip.iBottom = 408 + 19;

  if (vsDESKTOP == NULL) {
    return FALSE;
  }
  pDestBuf = (uint16_t *)LockVSurface(vsFB, &uiDestPitchBYTES);
  pSrcBuf = LockVSurface(vsDESKTOP, &uiSrcPitchBYTES);

  // blit .pcx for the background onto desktop
  Blt8BPPDataSubTo16BPPBuffer(pDestBuf, uiDestPitchBYTES, vsDESKTOP, pSrcBuf, uiSrcPitchBYTES,
                              LAPTOP_SCREEN_UL_X - 2, LAPTOP_SCREEN_UL_Y - 3, &clip);

  // release surfaces
  JSurface_Unlock(vsDESKTOP);
  JSurface_Unlock(vsFB);

  return (TRUE);
}

BOOLEAN LoadDesktopBackground(void) {
  // load desktop background
  SGPFILENAME ImageFile;
  GetMLGFilename(ImageFile, MLG_DESKTOP);
  vsDESKTOP = CreateVSurfaceFromFile(ImageFile);
  if (vsDESKTOP == NULL) {
    return FALSE;
  }
  JSurface_SetColorKey(vsDESKTOP, FROMRGB(0, 0, 0));

  return (TRUE);
}

void DeleteDesktopBackground(void) {
  JSurface_Free(vsDESKTOP);
  vsDESKTOP = NULL;
}

void PrintBalance(void) {
  wchar_t pString[32];
  //	uint16_t usX, usY;

  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  swprintf(pString, ARR_SIZE(pString), L"%d", MoneyGetBalance());
  InsertCommasForDollarFigure(pString);
  InsertDollarSignInToString(pString);

  if (ButtonList[gLaptopButton[5]]->uiFlags & BUTTON_CLICKED_ON) {
    //		gprintfdirty(47 +1, 257 +15 + 1,pString);
    mprintf(47 + 1, 257 + 15 + 1, pString);
  } else {
    //		gprintfdirty(47, 257 +15 ,pString);
    mprintf(47, 257 + 15, pString);
  }

  SetFontShadow(DEFAULT_SHADOW);
}

void PrintNumberOnTeam(void) {
  wchar_t pString[32];
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;
  int32_t iCounter = 0;
  uint16_t usPosX, usPosY;

  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // grab number on team
  pSoldier = MercPtrs[0];

  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    pTeamSoldier = MercPtrs[cnt];

    if ((pTeamSoldier->bActive) && (!(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE))) {
      iCounter++;
    }
  }

  swprintf(pString, ARR_SIZE(pString), L"%s %d", pPersonnelString[0], iCounter);

  if (ButtonList[gLaptopButton[3]]->uiFlags & BUTTON_CLICKED_ON) {
    usPosX = 47 + 1;
    usPosY = 194 + 30 + 1;
  } else {
    usPosX = 47;
    usPosY = 194 + 30;
  }

  mprintf(usPosX, usPosY, pString);

  SetFontShadow(DEFAULT_SHADOW);
}

void PrintDate(void) {
  SetFont(FONT10ARIAL);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);

  SetFontShadow(NO_SHADOW);

  mprintf(30 + (70 - StringPixLength(WORLDTIMESTR, FONT10ARIAL)) / 2, 433, WORLDTIMESTR);

  SetFontShadow(DEFAULT_SHADOW);

  //	RenderClock( 35, 414 );

  /*
          def: removed 3/8/99.
   Now use the render clock function used every where else

          wchar_t pString[ 32 ];
  //	uint16_t usX, usY;

          SetFont( FONT10ARIAL );
          SetFontForeground( FONT_BLACK );
          SetFontBackground( FONT_BLACK );

          SetFontShadow( NO_SHADOW );

          swprintf(pString, ARR_SIZE(pString), L"%s %d", pMessageStrings[ MSG_DAY ], GetWorldDay( )
  );

  //	gprintfdirty(35, 413 + 19,pString);
          mprintf(35, 413 + 19,pString);

          SetFontShadow( DEFAULT_SHADOW );
  */
  return;
}

void DisplayTaskBarIcons() {
  struct VObject *hPixHandle;
  //	uint16_t usPosX;

  //	usPosX = 83;

  GetVideoObject(&hPixHandle, guiTITLEBARICONS);

  if (fNewFilesInFileViewer) {
    // display the files icon, if there is any
    BltVideoObject(vsFB, hPixHandle, 7, LAPTOP__NEW_FILE_ICON_X, LAPTOP__NEW_FILE_ICON_Y);
  }

  // display the email icon, if there is email
  if (fUnReadMailFlag) {
    //		usPosX -= 16;
    BltVideoObject(vsFB, hPixHandle, 6, LAPTOP__NEW_EMAIL_ICON_X, LAPTOP__NEW_EMAIL_ICON_Y);
  }
}

void HandleKeyBoardShortCutsForLapTop(uint16_t usEvent, uint32_t usParam, uint16_t usKeyState) {
  // will handle keyboard shortcuts for the laptop ... to be added to later

  if ((fExitingLaptopFlag == TRUE) || (fTabHandled)) {
    return;
  }

  if ((usEvent == KEY_DOWN) && (usParam == ESC)) {
    // esc hit, check to see if boomark list is shown, if so, get rid of it, otherwise, leave
    HandleLapTopESCKey();
  } else if ((usEvent == KEY_DOWN) && (usParam == TAB)) {
    if (usKeyState & CTRL_DOWN) {
      HandleShiftAltTabKeyInLaptop();
    } else {
      HandleAltTabKeyInLaptop();
    }

    fTabHandled = TRUE;
  }
#ifdef JA2TESTVERSION

  else if ((usEvent == KEY_DOWN) && (usParam == 'm')) {
    if ((usKeyState & ALT_DOWN)) {
      CheatToGetAll5Merc();
    }
  }
#endif

  else if ((usEvent == KEY_DOWN) && (usParam == 'b')) {
    if (CHEATER_CHEAT_LEVEL()) {
      if ((usKeyState & ALT_DOWN))
        LaptopSaveInfo.fBobbyRSiteCanBeAccessed = TRUE;
      else if (usKeyState & CTRL_DOWN) {
        guiCurrentLaptopMode = LAPTOP_MODE_BROKEN_LINK;
      }
    }
  }

  else if ((usEvent == KEY_DOWN) && (usParam == 'x')) {
    if ((usKeyState & ALT_DOWN)) {
      HandleShortCutExitState();
    }
    // LeaveLapTopScreen( );
  }
#ifdef JA2TESTVERSION
  else if ((usEvent == KEY_DOWN) && (usParam == 'q')) {
    // if we dont currently have mercs on the team, hire some
    if (NumberOfMercsOnPlayerTeam() == 0) {
      uint8_t ubRand = (uint8_t)Random(2) + 2;
      TempHiringOfMercs(ubRand, FALSE);
      //	QuickStartGame( );
    }
    MarkButtonsDirty();
    fExitingLaptopFlag = TRUE;
  } else if ((usEvent == KEY_DOWN) && (usParam == 's')) {
    if ((usKeyState & ALT_DOWN)) {
      SetBookMark(AIM_BOOKMARK);
      SetBookMark(BOBBYR_BOOKMARK);
      SetBookMark(IMP_BOOKMARK);
      SetBookMark(MERC_BOOKMARK);
      SetBookMark(FUNERAL_BOOKMARK);
      SetBookMark(FLORIST_BOOKMARK);
      SetBookMark(INSURANCE_BOOKMARK);
    }
  }

  // help screen stuff
  else
#endif
      if ((usEvent == KEY_DOWN) && ((usParam == 'h') || (usParam == 'H'))) {
    ShouldTheHelpScreenComeUp(HELP_SCREEN_LAPTOP, TRUE);
  }
#ifdef JA2BETAVERSION
  // adding all emails
  else if ((usEvent == KEY_DOWN) && (usParam == 'e')) {
    if (CHEATER_CHEAT_LEVEL()) {
      if ((usKeyState & ALT_DOWN)) {
        AddAllEmails();
      }
    }
  }
#endif

  // adding money
  else if ((usEvent == KEY_DOWN) && (usParam == '=')) {
    if (CHEATER_CHEAT_LEVEL()) {
      AddTransactionToPlayersBook(ANONYMOUS_DEPOSIT, 0, 100000);
      MarkButtonsDirty();
    }
  }

  // subtracting money
  else if ((usEvent == KEY_DOWN) && (usParam == '-')) {
    if (CHEATER_CHEAT_LEVEL()) {
      AddTransactionToPlayersBook(ANONYMOUS_DEPOSIT, 0, -10000);
      MarkButtonsDirty();
    }
  }
#ifdef JA2TESTVERSION
  else if ((usEvent == KEY_DOWN) && (usParam == 'd')) {
    if (gfTemporaryDisablingOfLoadPendingFlag)
      gfTemporaryDisablingOfLoadPendingFlag = FALSE;
    else
      gfTemporaryDisablingOfLoadPendingFlag = TRUE;
  } else if ((usEvent == KEY_DOWN) && (usParam == '+')) {
    if (usKeyState & ALT_DOWN) {
      gStrategicStatus.ubHighestProgress += 10;
      if (gStrategicStatus.ubHighestProgress > 100) gStrategicStatus.ubHighestProgress = 100;

      InitAllArmsDealers();
      InitBobbyRayInventory();
    }
  } else if ((usEvent == KEY_DOWN) && (usParam == '-')) {
    if (usKeyState & ALT_DOWN) {
      if (gStrategicStatus.ubHighestProgress >= 10)
        gStrategicStatus.ubHighestProgress -= 10;
      else
        gStrategicStatus.ubHighestProgress = 0;

      InitAllArmsDealers();
      InitBobbyRayInventory();
    }
  } else if ((usEvent == KEY_DOWN) && (usParam == '*')) {
    if (usKeyState & ALT_DOWN) {
      DeleteAllStrategicEventsOfType(EVENT_EVALUATE_QUEEN_SITUATION);
      AdvanceToNextDay();
    }
  } else {
    if ((usEvent == KEY_DOWN) && (usParam == '1')) {
      TempHiringOfMercs(1, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '2')) {
      TempHiringOfMercs(2, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '3')) {
      TempHiringOfMercs(3, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '4')) {
      TempHiringOfMercs(4, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '5')) {
      TempHiringOfMercs(5, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '6')) {
      TempHiringOfMercs(6, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '7')) {
      TempHiringOfMercs(7, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '8')) {
      TempHiringOfMercs(8, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '9')) {
      TempHiringOfMercs(9, FALSE);
    }

    if ((usEvent == KEY_DOWN) && (usParam == '0')) {
      TempHiringOfMercs(10, FALSE);
    }
  }
#endif

  return;
}

BOOLEAN RenderWWWProgramTitleBar(void) {
  // will render the title bar for the www program
  uint32_t uiTITLEFORWWW;
  struct VObject *hHandle;
  int32_t iIndex = 0;
  wchar_t sString[256];

  // title bar - load
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\programtitlebar.sti"), &uiTITLEFORWWW));

  // blit title
  GetVideoObject(&hHandle, uiTITLEFORWWW);
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2);

  // now delete
  DeleteVideoObjectFromIndex(uiTITLEFORWWW);

  // now slapdown text
  SetFont(FONT14ARIAL);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  // display title

  // no page loaded yet, do not handle yet

  if (guiCurrentLaptopMode == LAPTOP_MODE_WWW) {
    mprintf(140, 33, pWebTitle[0]);
  }

  else {
    iIndex = guiCurrentLaptopMode - LAPTOP_MODE_WWW - 1;

    swprintf(sString, ARR_SIZE(sString), L"%s  -  %s", pWebTitle[0], pWebPagesTitles[iIndex]);
    mprintf(140, 33, sString);
  }

  BlitTitleBarIcons();

  DisplayProgramBoundingBox(FALSE);

  // InvalidateRegion( 0, 0, 640, 480 );
  return (TRUE);
}

void HandleDefaultWebpageForLaptop(void) {
  // go to first page in bookmark list
  if (guiCurrentLaptopMode == LAPTOP_MODE_WWW) {
    // if valid entry go there
    if (LaptopSaveInfo.iBookMarkList[0] != -1) {
      GoToWebPage(LaptopSaveInfo.iBookMarkList[0]);
    }
  }

  return;
}

void CreateMinimizeRegionsForLaptopProgramIcons(void) {
  // will create the minizing region to lie over the icon for this particular laptop program

  MSYS_DefineRegion(&gLapTopProgramMinIcon, LAPTOP_PROGRAM_ICON_X, LAPTOP_PROGRAM_ICON_Y,
                    LAPTOP_PROGRAM_ICON_X + LAPTOP_PROGRAM_ICON_WIDTH,
                    LAPTOP_PROGRAM_ICON_Y + LAPTOP_PROGRAM_ICON_HEIGHT, MSYS_PRIORITY_NORMAL + 1,
                    CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LaptopProgramIconMinimizeCallback);

  return;
}

void DestroyMinimizeRegionsForLaptopProgramIcons(void) {
  // will destroy the minizmize regions to be placed over the laptop icons that will be
  // displayed on the top of the laptop program bar

  MSYS_RemoveRegion(&gLapTopProgramMinIcon);

  return;
}

void LaptopProgramIconMinimizeCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // callback handler for the minize region that is attatched to the laptop program icon
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    switch (guiCurrentLaptopMode) {
      case (LAPTOP_MODE_EMAIL):
        gLaptopProgramStates[LAPTOP_PROGRAM_MAILER] = LAPTOP_PROGRAM_MINIMIZED;
        InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[0], guiTITLEBARICONS, 0);
        SetCurrentToLastProgramOpened();
        fMinizingProgram = TRUE;
        fInitTitle = TRUE;
        break;
      case (LAPTOP_MODE_FILES):
        gLaptopProgramStates[LAPTOP_PROGRAM_FILES] = LAPTOP_PROGRAM_MINIMIZED;
        InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[5], guiTITLEBARICONS, 2);
        SetCurrentToLastProgramOpened();
        fMinizingProgram = TRUE;
        fInitTitle = TRUE;
        break;
      case (LAPTOP_MODE_FINANCES):
        gLaptopProgramStates[LAPTOP_PROGRAM_FINANCES] = LAPTOP_PROGRAM_MINIMIZED;
        InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[2], guiTITLEBARICONS, 5);
        SetCurrentToLastProgramOpened();
        fMinizingProgram = TRUE;
        fInitTitle = TRUE;
        break;
      case (LAPTOP_MODE_HISTORY):
        gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY] = LAPTOP_PROGRAM_MINIMIZED;
        InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[4], guiTITLEBARICONS, 4);
        SetCurrentToLastProgramOpened();
        fMinizingProgram = TRUE;
        fInitTitle = TRUE;
        break;
      case (LAPTOP_MODE_PERSONNEL):
        gLaptopProgramStates[LAPTOP_PROGRAM_PERSONNEL] = LAPTOP_PROGRAM_MINIMIZED;
        InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[3], guiTITLEBARICONS, 3);
        SetCurrentToLastProgramOpened();
        fMinizingProgram = TRUE;
        fInitTitle = TRUE;
        break;
      case (LAPTOP_MODE_NONE):
        // nothing
        break;
      default:
        gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] = LAPTOP_PROGRAM_MINIMIZED;
        InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pWebTitle[0], guiTITLEBARICONS, 1);
        SetCurrentToLastProgramOpened();
        gfShowBookmarks = FALSE;
        fShowBookmarkInfo = FALSE;
        fMinizingProgram = TRUE;
        fInitTitle = TRUE;
        break;
    }
  }
  return;
}

void DisplayProgramBoundingBox(BOOLEAN fMarkButtons) {
  // the border fot eh program
  struct VObject *hHandle;

  GetVideoObject(&hHandle, guiLaptopBACKGROUND);
  BltVideoObject(vsFB, hHandle, 1, 25, 23);

  // no laptop mode, no border around the program
  if (guiCurrentLaptopMode != LAPTOP_MODE_NONE) {
    GetVideoObject(&hHandle, guiLaptopBACKGROUND);
    BltVideoObject(vsFB, hHandle, 0, 108, 23);
  }

  if (fMarkButtons || fLoadPendingFlag) {
    MarkButtonsDirty();
    RenderButtons();
  }

  PrintDate();

  PrintBalance();

  PrintNumberOnTeam();

  // new files or email?
  DisplayTaskBarIcons();

  // InvalidateRegion( 0,0, 640, 480 );

  return;
}

void CreateDestroyMouseRegionForNewMailIcon(void) {
  static BOOLEAN fCreated = FALSE;

  //. will toggle creation/destruction of the mouse regions used by the new mail icon

  if (fCreated == FALSE) {
    fCreated = TRUE;
    MSYS_DefineRegion(&gNewMailIconRegion, LAPTOP__NEW_EMAIL_ICON_X, LAPTOP__NEW_EMAIL_ICON_Y + 5,
                      LAPTOP__NEW_EMAIL_ICON_X + 16, LAPTOP__NEW_EMAIL_ICON_Y + 16,
                      MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                      NewEmailIconCallback);
    CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL,
                                          (BOOLEAN)(fUnReadMailFlag == 0));

    MSYS_DefineRegion(&gNewFileIconRegion, LAPTOP__NEW_FILE_ICON_X, LAPTOP__NEW_FILE_ICON_Y + 5,
                      LAPTOP__NEW_FILE_ICON_X + 16, LAPTOP__NEW_FILE_ICON_Y + 16,
                      MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                      NewFileIconCallback);
    CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE,
                                          (BOOLEAN)(fNewFilesInFileViewer == 0));
  } else {
    fCreated = FALSE;
    MSYS_RemoveRegion(&gNewMailIconRegion);
    MSYS_RemoveRegion(&gNewFileIconRegion);
  }
}

void NewEmailIconCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (fUnReadMailFlag) {
      fOpenMostRecentUnReadFlag = TRUE;
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
    }
  }
}

void NewFileIconCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (fNewFilesInFileViewer) {
      fEnteredFileViewerFromNewFileIcon = TRUE;
      guiCurrentLaptopMode = LAPTOP_MODE_FILES;
    }
  }
}

void HandleWWWSubSites(void) {
  // check to see if WW Wait is needed for a sub site within the Web Browser

  if ((guiCurrentLaptopMode == guiPreviousLaptopMode) || (guiCurrentLaptopMode < LAPTOP_MODE_WWW) ||
      (fLoadPendingFlag == TRUE) || (fDoneLoadPending == TRUE) ||
      (guiPreviousLaptopMode < LAPTOP_MODE_WWW)) {
    // no go, leave
    return;
  }

  fLoadPendingFlag = TRUE;
  fConnectingToSubPage = TRUE;

  // fast or slow load?
  if (gfWWWaitSubSitesVisitedFlags[guiCurrentLaptopMode - (LAPTOP_MODE_WWW + 1)] == TRUE) {
    fFastLoadFlag = TRUE;
  }

  // set fact we were here
  gfWWWaitSubSitesVisitedFlags[guiCurrentLaptopMode - (LAPTOP_MODE_WWW + 1)] = TRUE;

  // Dont show the dlownload screen when switching between these pages
  if (((guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS) &&
       (guiPreviousLaptopMode == LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX)) ||
      ((guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX) &&
       (guiPreviousLaptopMode == LAPTOP_MODE_AIM_MEMBERS))) {
    fFastLoadFlag = FALSE;
    fLoadPendingFlag = FALSE;

    // set fact we were here
    gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX - (LAPTOP_MODE_WWW + 1)] =
        TRUE;
    gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_AIM_MEMBERS - (LAPTOP_MODE_WWW + 1)] = TRUE;
  }

  return;
}

void UpdateStatusOfDisplayingBookMarks(void) {
  // this function will disable showing of bookmarks if in process of download or if we miniming web
  // browser
  if ((fLoadPendingFlag == TRUE) || (guiCurrentLaptopMode < LAPTOP_MODE_WWW)) {
    gfShowBookmarks = FALSE;
  }

  return;
}

void InitalizeSubSitesList(void) {
  int32_t iCounter = 0;

  // init all subsites list to not visited
  for (iCounter = LAPTOP_MODE_WWW + 1; iCounter <= LAPTOP_MODE_SIRTECH; iCounter++) {
    gfWWWaitSubSitesVisitedFlags[iCounter - (LAPTOP_MODE_WWW + 1)] = FALSE;
  }
  return;
}

void SetSubSiteAsVisted(void) {
  // sets a www sub site as visited
  if (guiCurrentLaptopMode <= LAPTOP_MODE_WWW) {
    // not at a web page yet
  } else {
    gfWWWaitSubSitesVisitedFlags[guiCurrentLaptopMode - (LAPTOP_MODE_WWW + 1)] = TRUE;
  }
}

void HandleShiftAltTabKeyInLaptop(void) {
  // will handle the alt tab keying in laptop

  // move to next program
  if (fMaximizingProgram == TRUE) {
    return;
  }

  switch (guiCurrentLaptopMode) {
    case (LAPTOP_MODE_FINANCES):
      guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
      break;
    case (LAPTOP_MODE_PERSONNEL):
      guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
      break;
    case (LAPTOP_MODE_HISTORY):
      guiCurrentLaptopMode = LAPTOP_MODE_FILES;
      break;
    case (LAPTOP_MODE_EMAIL):
      guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
      break;
    case (LAPTOP_MODE_FILES):
      guiCurrentLaptopMode = LAPTOP_MODE_WWW;
      break;
    case (LAPTOP_MODE_NONE):
      guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
      break;
    case (LAPTOP_MODE_WWW):
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
      break;
    default:
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
      break;
  }

  fPausedReDrawScreenFlag = TRUE;
}

void HandleAltTabKeyInLaptop(void) {
  // will handle the alt tab keying in laptop

  // move to next program
  // move to next program
  if (fMaximizingProgram == TRUE) {
    return;
  }

  switch (guiCurrentLaptopMode) {
    case (LAPTOP_MODE_FINANCES):
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
      break;
    case (LAPTOP_MODE_PERSONNEL):
      guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
      break;

    case (LAPTOP_MODE_HISTORY):
      guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
      break;
    case (LAPTOP_MODE_EMAIL):
      guiCurrentLaptopMode = LAPTOP_MODE_WWW;
      break;
    case (LAPTOP_MODE_FILES):
      guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
      break;
    case (LAPTOP_MODE_NONE):
      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
      break;
    default:
      guiCurrentLaptopMode = LAPTOP_MODE_FILES;
      break;
  }

  fPausedReDrawScreenFlag = TRUE;
}

// display the 2 second book mark instruction
void DisplayWebBookMarkNotify(void) {
  static BOOLEAN fOldShow = FALSE;
  struct VObject *hLapTopIconHandle;

  // handle the timer for this thing
  HandleWebBookMarkNotifyTimer();

  // are we about to start showing box? or did we just stop?
  if (((fOldShow == FALSE) || (fReDrawBookMarkInfo)) && (fShowBookmarkInfo == TRUE)) {
    fOldShow = TRUE;
    fReDrawBookMarkInfo = FALSE;

    // show background objects
    GetVideoObject(&hLapTopIconHandle, guiDOWNLOADTOP);
    BltVideoObject(vsFB, hLapTopIconHandle, 0, DOWNLOAD_X, DOWNLOAD_Y);
    GetVideoObject(&hLapTopIconHandle, guiDOWNLOADMID);
    BltVideoObject(vsFB, hLapTopIconHandle, 0, DOWNLOAD_X, DOWNLOAD_Y + DOWN_HEIGHT);
    GetVideoObject(&hLapTopIconHandle, guiDOWNLOADBOT);
    BltVideoObject(vsFB, hLapTopIconHandle, 0, DOWNLOAD_X, DOWNLOAD_Y + 2 * DOWN_HEIGHT);
    GetVideoObject(&hLapTopIconHandle, guiTITLEBARICONS);
    BltVideoObject(vsFB, hLapTopIconHandle, 1, DOWNLOAD_X + 4, DOWNLOAD_Y + 1);

    //	MSYS_DefineRegion( &gLapTopScreenRegion, ( uint16_t )( LaptopScreenRect.iLeft ),( uint16_t
    //)(
    // LaptopScreenRect.iTop ),( uint16_t ) ( LaptopScreenRect.iRight ),( uint16_t )(
    // LaptopScreenRect.iBottom ), MSYS_PRIORITY_NORMAL+1,
    // CURSOR_LAPTOP_SCREEN, ScreenRegionMvtCallback, LapTopScreenCallBack );

    // font stuff
    SetFont(DOWNLOAD_FONT);
    SetFontForeground(FONT_WHITE);
    SetFontBackground(FONT_BLACK);
    SetFontShadow(NO_SHADOW);

    // display download string
    mprintf(DOWN_STRING_X, DOWN_STRING_Y, pShowBookmarkString[0]);

    SetFont(BOOK_FONT);
    SetFontForeground(FONT_BLACK);
    SetFontBackground(FONT_BLACK);
    SetFontShadow(NO_SHADOW);

    // now draw the message
    DisplayWrappedString((int16_t)(DOWN_STRING_X - 42), (uint16_t)(DOWN_STRING_Y + 20),
                         BOOK_WIDTH + 45, 2, BOOK_FONT, FONT_BLACK, pShowBookmarkString[1],
                         FONT_BLACK, FALSE, CENTER_JUSTIFIED);

    // invalidate region
    InvalidateRegion(DOWNLOAD_X, DOWNLOAD_Y, DOWNLOAD_X + 150, DOWNLOAD_Y + 100);

  } else if ((fOldShow == TRUE) && (fShowBookmarkInfo == FALSE)) {
    // MSYS_RemoveRegion( &gLapTopScreenRegion );
    fOldShow = FALSE;
    fPausedReDrawScreenFlag = TRUE;
  }

  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void HandleWebBookMarkNotifyTimer(void) {
  static int32_t iBaseTime = 0;
  int32_t iDifference = 0;
  static BOOLEAN fOldShowBookMarkInfo = FALSE;

  // check if maxing or mining?
  if ((fMaximizingProgram == TRUE) || (fMinizingProgram == TRUE)) {
    fOldShowBookMarkInfo |= fShowBookmarkInfo;
    fShowBookmarkInfo = FALSE;
    return;
  }

  // if we were going to show this pop up, but were delayed, then do so now
  fShowBookmarkInfo |= fOldShowBookMarkInfo;

  // reset old flag
  fOldShowBookMarkInfo = FALSE;

  // if current mode is too low, then reset
  if (guiCurrentLaptopMode < LAPTOP_MODE_WWW) {
    fShowBookmarkInfo = FALSE;
  }

  // if showing bookmarks, don't show help
  if (gfShowBookmarks == TRUE) {
    fShowBookmarkInfo = FALSE;
  }

  // check if flag false, is so, leave
  if (fShowBookmarkInfo == FALSE) {
    iBaseTime = 0;
    return;
  }

  // check if this is the first time in here
  if (iBaseTime == 0) {
    iBaseTime = GetJA2Clock();
    return;
  }

  iDifference = GetJA2Clock() - iBaseTime;

  fReDrawBookMarkInfo = TRUE;

  if (iDifference > DISPLAY_TIME_FOR_WEB_BOOKMARK_NOTIFY) {
    // waited long enough, stop showing
    iBaseTime = 0;
    fShowBookmarkInfo = FALSE;
  }

  return;
}

void ClearOutTempLaptopFiles(void) {
  // clear out all temp files from laptop

  // file file
  if ((FileMan_Exists("files.dat") == TRUE)) {
    Plat_ClearFileAttributes("files.dat");
    FileMan_Delete("files.dat");
  }

  // finances
  if ((FileMan_Exists("finances.dat") == TRUE)) {
    Plat_ClearFileAttributes("finances.dat");
    FileMan_Delete("finances.dat");
  }

  // email
  if ((FileMan_Exists("email.dat") == TRUE)) {
    Plat_ClearFileAttributes("email.dat");
    FileMan_Delete("email.dat");
  }

  // history
  if ((FileMan_Exists("history.dat") == TRUE)) {
    Plat_ClearFileAttributes("history.dat");
    FileMan_Delete("history.dat");
  }
}

BOOLEAN SaveLaptopInfoToSavedGame(HWFILE hFile) {
  uint32_t uiNumBytesWritten = 0;
  uint32_t uiSize;

  // Save The laptop information
  FileMan_Write(hFile, &LaptopSaveInfo, sizeof(LaptopSaveInfoStruct), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(LaptopSaveInfoStruct)) {
    return (FALSE);
  }

  // If there is anything in the Bobby Ray Orders on Delivery
  if (LaptopSaveInfo.usNumberOfBobbyRayOrderUsed) {
    // Allocate memory for the information
    uiSize = sizeof(BobbyRayOrderStruct) * LaptopSaveInfo.usNumberOfBobbyRayOrderItems;

    // Load The laptop information
    FileMan_Write(hFile, LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray, uiSize, &uiNumBytesWritten);
    if (uiNumBytesWritten != uiSize) {
      return (FALSE);
    }
  }

  // If there is any Insurance Payouts in progress
  if (LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed) {
    // Allocate memory for the information
    uiSize = sizeof(LIFE_INSURANCE_PAYOUT) * LaptopSaveInfo.ubNumberLifeInsurancePayouts;

    // Load The laptop information
    FileMan_Write(hFile, LaptopSaveInfo.pLifeInsurancePayouts, uiSize, &uiNumBytesWritten);
    if (uiNumBytesWritten != uiSize) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN LoadLaptopInfoFromSavedGame(HWFILE hFile) {
  uint32_t uiNumBytesRead = 0;
  uint32_t uiSize;

  // if there is memory allocated for the BobbyR orders
  if (LaptopSaveInfo.usNumberOfBobbyRayOrderItems) {
    //		if( !LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray )
    //			Assert( 0 );	//Should never happen

    // Free the memory
    if (LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray)
      MemFree(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray);
    LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
  }

  // if there is memory allocated for life insurance payouts
  if (LaptopSaveInfo.ubNumberLifeInsurancePayouts) {
    if (!LaptopSaveInfo.pLifeInsurancePayouts) Assert(0);  // Should never happen

    // Free the memory
    MemFree(LaptopSaveInfo.pLifeInsurancePayouts);
    LaptopSaveInfo.pLifeInsurancePayouts = NULL;
  }

  // Load The laptop information
  FileMan_Read(hFile, &LaptopSaveInfo, sizeof(LaptopSaveInfoStruct), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(LaptopSaveInfoStruct)) {
    return (FALSE);
  }

  // If there is anything in the Bobby Ray Orders on Delivery
  if (LaptopSaveInfo.usNumberOfBobbyRayOrderUsed) {
    // Allocate memory for the information
    uiSize = sizeof(BobbyRayOrderStruct) * LaptopSaveInfo.usNumberOfBobbyRayOrderItems;

    LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = (BobbyRayOrderStruct *)MemAlloc(uiSize);
    Assert(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray);

    // Load The laptop information
    FileMan_Read(hFile, LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray, uiSize, &uiNumBytesRead);
    if (uiNumBytesRead != uiSize) {
      return (FALSE);
    }
  } else {
    LaptopSaveInfo.usNumberOfBobbyRayOrderItems = 0;
    LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
  }

  // If there is any Insurance Payouts in progress
  if (LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed) {
    // Allocate memory for the information
    uiSize = sizeof(LIFE_INSURANCE_PAYOUT) * LaptopSaveInfo.ubNumberLifeInsurancePayouts;

    LaptopSaveInfo.pLifeInsurancePayouts = (LIFE_INSURANCE_PAYOUT *)MemAlloc(uiSize);
    Assert(LaptopSaveInfo.pLifeInsurancePayouts);

    // Load The laptop information
    FileMan_Read(hFile, LaptopSaveInfo.pLifeInsurancePayouts, uiSize, &uiNumBytesRead);
    if (uiNumBytesRead != uiSize) {
      return (FALSE);
    }
  } else {
    LaptopSaveInfo.ubNumberLifeInsurancePayouts = 0;
    LaptopSaveInfo.pLifeInsurancePayouts = NULL;
  }

  return (TRUE);
}

void LaptopSaveVariablesInit() {}

int32_t WWaitDelayIncreasedIfRaining(int32_t iUnitTime) {
  int32_t iRetVal = 0;

  if (guiEnvWeather & WEATHER_FORECAST_THUNDERSHOWERS) {
    iRetVal = (int32_t)(iUnitTime * (float)0.80);
  } else if (guiEnvWeather & WEATHER_FORECAST_SHOWERS) {
    iRetVal = (int32_t)(iUnitTime * (float)0.6);
  }

  return (iRetVal);
}

BOOLEAN IsItRaining() {
  if (guiEnvWeather & WEATHER_FORECAST_SHOWERS || guiEnvWeather & WEATHER_FORECAST_THUNDERSHOWERS)
    return (TRUE);
  else
    return (FALSE);
}

void InternetRainDelayMessageBoxCallBack(uint8_t bExitValue) {
  GoToWebPage(giRainDelayInternetSite);

  // Set to -2 so we dont due the message for this occurence of laptop
  giRainDelayInternetSite = -2;
}

void CreateBookMarkHelpText(struct MOUSE_REGION *pRegion, uint32_t uiBookMarkID) {
  SetRegionFastHelpText(
      pRegion,
      gzLaptopHelpText[BOOKMARK_TEXT_ASSOCIATION_OF_INTERNATION_MERCENARIES + uiBookMarkID]);
}

void CreateFileAndNewEmailIconFastHelpText(uint32_t uiHelpTextID, BOOLEAN fClearHelpText) {
  struct MOUSE_REGION *pRegion;

  switch (uiHelpTextID) {
    case LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL:
      pRegion = &gNewMailIconRegion;
      break;

    case LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE:
      pRegion = &gNewFileIconRegion;
      break;

    default:
      Assert(0);
      return;
  }

  if (fClearHelpText)
    SetRegionFastHelpText(pRegion, L"");
  else
    SetRegionFastHelpText(pRegion, gzLaptopHelpText[uiHelpTextID]);

  // fUnReadMailFlag
  // fNewFilesInFileViewer
}

void CreateLaptopButtonHelpText(int32_t iButtonIndex, uint32_t uiButtonHelpTextID) {
  SetButtonFastHelpText(iButtonIndex, gzLaptopHelpText[uiButtonHelpTextID]);
}
