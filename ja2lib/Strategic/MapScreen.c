// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/MapScreen.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "CharList.h"
#include "Cheats.h"
#include "FadeScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "Globals.h"
#include "HelpScreen.h"
#include "JAScreens.h"
#include "Laptop/Email.h"
#include "Laptop/Finances.h"
#include "Laptop/Personnel.h"
#include "Local.h"
#include "Money.h"
#include "OptionsScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/Line.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Screens.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameInit.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MapScreenInterfaceMapInventory.h"
#include "Strategic/MapScreenInterfaceTownMineInfo.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/MercContract.h"
#include "Strategic/PlayerCommand.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/StrategicTurns.h"
#include "Strategic/TownMilitia.h"
#include "SysGlobals.h"
#include "Tactical/AirRaid.h"
#include "Tactical/Campaign.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/Items.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/Vehicles.h"
#include "Tactical/Weapons.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "Town.h"
#include "UI.h"
#include "Utils/AnimatedProgressBar.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/PopUpBox.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"

// DEFINES

#define MAX_SORT_METHODS 6

// Cursors
#define SCREEN_CURSOR CURSOR_NORMAL

// Fonts
#define CHAR_FONT BLOCKFONT2  // COMPFONT
#define ETA_FONT BLOCKFONT2

// Colors
#define CHAR_INFO_PANEL_BLOCK_COLOR 60

#define FONT_MAP_DKYELLOW 170

#define CHAR_TITLE_FONT_COLOR 6
#define CHAR_TEXT_FONT_COLOR 5

#define STARTING_COLOR_NUM 5

#define MAP_TIME_UNDER_THIS_DISPLAY_AS_HOURS (3 * 24 * 60)

#define MIN_KB_TO_DO_PRE_LOAD (32 * 1024)

#define DELAY_PER_FLASH_FOR_DEPARTING_PERSONNEL 500
#define GLOW_DELAY 70
#define ASSIGNMENT_DONE_FLASH_TIME 500

#define MINS_TO_FLASH_CONTRACT_TIME (4 * 60)

// Coordinate defines

#define TOWN_INFO_X 0
#define TOWN_INFO_Y 1

#define PLAYER_INFO_X 0
#define PLAYER_INFO_Y 107

// item description
#define MAP_ITEMDESC_START_X 0
#define MAP_ITEMDESC_START_Y PLAYER_INFO_Y

#define INV_REGION_X PLAYER_INFO_X
#define INV_REGION_Y PLAYER_INFO_Y
#define INV_REGION_WIDTH 261
#define INV_REGION_HEIGHT 359 - 94
#define INV_BTN_X PLAYER_INFO_X + 217
#define INV_BTN_Y PLAYER_INFO_Y + 210

#define MAP_ARMOR_LABEL_X 208
#define MAP_ARMOR_LABEL_Y 180
#define MAP_ARMOR_X 209
#define MAP_ARMOR_Y 189
#define MAP_ARMOR_PERCENT_X 229
#define MAP_ARMOR_PERCENT_Y 190

#define MAP_WEIGHT_LABEL_X 173
#define MAP_WEIGHT_LABEL_Y 256
#define MAP_WEIGHT_X 176
#define MAP_WEIGHT_Y 266
#define MAP_WEIGHT_PERCENT_X 196
#define MAP_WEIGHT_PERCENT_Y 266

#define MAP_CAMMO_LABEL_X 178
#define MAP_CAMMO_LABEL_Y 283
#define MAP_CAMMO_X 176
#define MAP_CAMMO_Y 292
#define MAP_CAMMO_PERCENT_X 196
#define MAP_CAMMO_PERCENT_Y 293

#define MAP_PERCENT_WIDTH 20
#define MAP_PERCENT_HEIGHT 10

#define MAP_INV_STATS_TITLE_FONT_COLOR 6
#define MAP_INV_STATS_TEXT_FONT_COLOR 5

#define PLAYER_INFO_FACE_START_X 9
#define PLAYER_INFO_FACE_START_Y 17
#define PLAYER_INFO_FACE_END_X 60
#define PLAYER_INFO_FACE_END_Y 76

#define INV_BODY_X 71
#define INV_BODY_Y 116

#define NAME_X 11
#define NAME_WIDTH 62 - NAME_X
#define ASSIGN_X 67
#define ASSIGN_WIDTH 118 - ASSIGN_X
#define SLEEP_X 123
#define SLEEP_WIDTH 142 - SLEEP_X
#define LOC_X 147
#define LOC_WIDTH 179 - LOC_X
#define DEST_ETA_X 184
#define DEST_ETA_WIDTH 217 - DEST_ETA_X
#define TIME_REMAINING_X 222
#define TIME_REMAINING_WIDTH 250 - TIME_REMAINING_X
#define CLOCK_X_START 463 - 18
#define CLOCK_Y_START 298
#define DEST_PLOT_X 463
#define DEST_PLOT_Y 345
#define CLOCK_ETA_X 463 - 15 + 6 + 30
#define CLOCK_HOUR_X_START 463 + 25 + 30
#define CLOCK_MIN_X_START 463 + 45 + 30

// contract
#define CONTRACT_X 185
#define CONTRACT_Y 50
// #define CONTRACT_WIDTH  63
// #define CONTRACT_HEIGHT 10

// trash can
#define TRASH_CAN_X 176
#define TRASH_CAN_Y 211 + PLAYER_INFO_Y
#define TRASH_CAN_WIDTH 193 - 165
#define TRASH_CAN_HEIGHT 239 - 217

// Text offsets
#define Y_OFFSET 2

// The boxes defines
#define TRAIN_Y_OFFSET 53
#define TRAIN_X_OFF 65
#define TRAIN_WID 80
#define TRAIN_HEIG 47
#define STRING_X_OFFSET 10
#define STRING_Y_OFFSET 5
#define POP_UP_BOX_X 120
#define POP_UP_BOX_Y 0
#define POP_UP_BOX_WIDTH 60
#define POP_UP_BOX_HEIGHT 100
#define MOUSE_PTR_Y_OFFSET 3
#define POP_UP_Y_OFFSET 3
#define TRAIN_TEXT_Y_OFFSET 4

// char stat positions
#define STR_X (112)
#define STR_Y 42
#define DEX_X STR_X
#define DEX_Y 32
#define AGL_X STR_X
#define AGL_Y 22
#define LDR_X STR_X
#define LDR_Y 52
#define WIS_X STR_X
#define WIS_Y 62
#define LVL_X (159)
#define LVL_Y AGL_Y
#define MRK_X LVL_X
#define MRK_Y DEX_Y
#define EXP_X LVL_X
#define EXP_Y STR_Y
#define MEC_X LVL_X
#define MEC_Y LDR_Y
#define MED_X LVL_X
#define MED_Y WIS_Y

#define STAT_WID 15
#define STAT_HEI GetFontHeight(CHAR_FONT)

#define PIC_NAME_X 8
#define PIC_NAME_Y (66 + 3)
#define PIC_NAME_WID 60 - PIC_NAME_X
#define PIC_NAME_HEI 75 - PIC_NAME_Y
#define CHAR_NAME_X 14
#define CHAR_NAME_Y (2 + 3)
#define CHAR_NAME_WID 164 - CHAR_NAME_X
#define CHAR_NAME_HEI 11 - CHAR_NAME_Y
#define CHAR_LOC_X 76
#define CHAR_LOC_Y 84
#define CHAR_LOC_WID 16
#define CHAR_LOC_HEI 9
#define CHAR_TIME_REMAINING_X 207
#define CHAR_TIME_REMAINING_Y 65
#define CHAR_TIME_REMAINING_WID 258 - CHAR_TIME_REMAINING_X
#define CHAR_TIME_REMAINING_HEI GetFontHeight(CHAR_FONT)
#define CHAR_SALARY_X CHAR_TIME_REMAINING_X
#define CHAR_SALARY_Y 79
#define CHAR_SALARY_WID CHAR_TIME_REMAINING_WID - 8  // for right justify
#define CHAR_SALARY_HEI CHAR_TIME_REMAINING_HEI
#define CHAR_MEDICAL_X CHAR_TIME_REMAINING_X
#define CHAR_MEDICAL_Y 93
#define CHAR_MEDICAL_WID CHAR_TIME_REMAINING_WID - 8  // for right justify
#define CHAR_MEDICAL_HEI CHAR_TIME_REMAINING_HEI
#define CHAR_ASSIGN_X 182
#define CHAR_ASSIGN1_Y 18
#define CHAR_ASSIGN2_Y 31
#define CHAR_ASSIGN_WID 257 - 178
#define CHAR_ASSIGN_HEI 39 - 29
#define CHAR_HP_X 133
#define CHAR_HP_Y 77 + 3
#define CHAR_HP_WID 175 - CHAR_HP_X
#define CHAR_HP_HEI 90 - CHAR_HP_Y
#define CHAR_MORALE_X 133
#define CHAR_MORALE_Y 91 + 3
#define CHAR_MORALE_WID 175 - CHAR_MORALE_X
#define CHAR_MORALE_HEI 101 - CHAR_MORALE_Y

#define CROSS_X 195
#define CROSS_Y 83
#define CROSS_HEIGHT 20
#define CROSS_WIDTH 20
#define CHAR_PAY_X 150
#define CHAR_PAY_Y 80 + 4
#define CHAR_PAY_HEI GetFontHeight(CHAR_FONT)
#define CHAR_PAY_WID CROSS_X - CHAR_PAY_X
#define SOLDIER_PIC_X 9
#define SOLDIER_PIC_Y 20
#define SOLDIER_HAND_X 6
#define SOLDIER_HAND_Y 81
// #define	TM_INV_WIDTH								58
// #define	TM_INV_HEIGHT								23

#define CLOCK_X 554
#define CLOCK_Y 459

#define RGB_WHITE (FROMRGB(255, 255, 255))
#define RGB_YELLOW (FROMRGB(255, 255, 0))
#define RGB_NEAR_BLACK (FROMRGB(0, 0, 1))

// ENUMS

// ARM: NOTE that these map "events" are never actually saved in a player's game in any way
enum {
  MAP_EVENT_NONE,
  MAP_EVENT_CLICK_SECTOR,
  MAP_EVENT_PLOT_PATH,
  MAP_EVENT_CANCEL_PATH,

#ifdef JA2BETAVERSION
  MAP_EVENT_VIEWAI
#endif
};

// STRUCTURES / TYPEDEFS

struct rgbcolor {
  uint8_t ubRed;
  uint8_t ubGreen;
  uint8_t ubBlue;
};

typedef struct rgbcolor RGBCOLOR;

struct lineoftext {
  wchar_t *pLineText;
  uint32_t uiFont;
  struct lineoftext *pNext;
};

typedef struct lineoftext LineText;
typedef LineText *LineTextPtr;

struct popbox {
  uint16_t usTopX;
  uint16_t usTopY;
  uint16_t usWidth;
  uint16_t usHeight;
  LineTextPtr pBoxText;
  struct popbox *pNext;
};

typedef struct popbox PopUpBox;
typedef PopUpBox *PopUpBoxPtr;

// TABLES

RGBCOLOR GlowColorsA[] = {
    {0, 0, 0},   {25, 0, 0},  {50, 0, 0},  {75, 0, 0},  {100, 0, 0}, {125, 0, 0},
    {150, 0, 0}, {175, 0, 0}, {200, 0, 0}, {225, 0, 0}, {250, 0, 0},
};
/* unused
RGBCOLOR GlowColorsB[]={
        {0,0,0},
        {25,25,0},
        {50,50,0},
        {75,75,0},
        {100,100,0},
        {125,125,0},
        {150,150,0},
        {175,175,0},
        {200,200,0},
        {225,225,0},
        {255,255,0},
};
RGBCOLOR GlowColorsC[]={
        {0,0,0},
        {25,0,25},
        {50,0,50},
        {75,0,75},
        {100,0,100},
        {125,0,125},
        {150,0,150},
        {175,0,175},
        {200,0,200},
        {225,0,225},
        {255,0,255},
};
*/

SGPPoint gMapSortButtons[MAX_SORT_METHODS] = {
    {12, 125}, {68, 125}, {124, 125}, {148, 125}, {185, 125}, {223, 125},
};

// map screen's inventory panel pockets - top right corner coordinates
INV_REGION_DESC gMapScreenInvPocketXY[] = {
    {204, 116},  // HELMETPOS
    {204, 145},  // VESTPOS
    {204, 205},  // LEGPOS,
    {21, 116},   // HEAD1POS
    {21, 140},   // HEAD2POS
    {21, 194},   // HANDPOS,
    {21, 218},   // SECONDHANDPOS
    {98, 251},   // BIGPOCK1
    {98, 275},   // BIGPOCK2
    {98, 299},   // BIGPOCK3
    {98, 323},   // BIGPOCK4
    {22, 251},   // SMALLPOCK1
    {22, 275},   // SMALLPOCK2
    {22, 299},   // SMALLPOCK3
    {22, 323},   // SMALLPOCK4
    {60, 251},   // SMALLPOCK5
    {60, 275},   // SMALLPOCK6
    {60, 299},   // SMALLPOCK7
    {60, 323}    // SMALLPOCK8
};

INV_REGION_DESC gSCamoXY = {
    INV_BODY_X, INV_BODY_Y  // X, Y Location of Map screen's Camouflage region
};

// GLOBAL VARIABLES (OURS)

BOOLEAN fFlashAssignDone = FALSE;
BOOLEAN fInMapMode = FALSE;
BOOLEAN fMapPanelDirty = TRUE;
BOOLEAN fTeamPanelDirty = TRUE;
BOOLEAN fCharacterInfoPanelDirty = TRUE;
BOOLEAN gfLoadPending = FALSE;
BOOLEAN fReDrawFace = FALSE;
// extern BOOLEAN fFirstTimeInMapScreen; // = TRUE;
BOOLEAN fShowInventoryFlag = FALSE;
BOOLEAN fMapInventoryItem = FALSE;
BOOLEAN fShowDescriptionFlag = FALSE;

// are the graphics for mapscreen preloaded?
BOOLEAN fPreLoadedMapGraphics = FALSE;

BOOLEAN gfHotKeyEnterSector = FALSE;
BOOLEAN fOneFrame = FALSE;
BOOLEAN fShowFaceHightLight = FALSE;
BOOLEAN fShowItemHighLight = FALSE;
BOOLEAN gfAllowSkyriderTooFarQuote = FALSE;
BOOLEAN fJustFinishedPlotting = FALSE;

// for the flashing of the contract departure time...for when mercs are leaving in an hour or less
BOOLEAN fFlashContractFlag = FALSE;

BOOLEAN fShowTrashCanHighLight = FALSE;

// the flags for display of pop up boxes/menus
BOOLEAN fEndPlotting = FALSE;

BOOLEAN gfInConfirmMapMoveMode = FALSE;
BOOLEAN gfInChangeArrivalSectorMode = FALSE;

// redraw character list
BOOLEAN fDrawCharacterList = TRUE;

// was the cursor set to the checkmark?
BOOLEAN fCheckCursorWasSet = FALSE;

BOOLEAN fShowingMapDisableBox = FALSE;
BOOLEAN fShowFrameRate = FALSE;
// BOOLEAN fMapExitDueToMessageBox = FALSE;
BOOLEAN fEndShowInventoryFlag = FALSE;

// draw the temp path
BOOLEAN fDrawTempPath = TRUE;

BOOLEAN gfCharacterListInited = FALSE;

BOOLEAN gfGlowTimerExpired = FALSE;

// not required to be saved.  The flag is set to allow mapscreen to render once, then transition the
// current tactical battle into autoresolve.
BOOLEAN gfTransitionMapscreenToAutoResolve = FALSE;

BOOLEAN gfSkyriderEmptyHelpGiven = FALSE;

BOOLEAN gfRequestGiveSkyriderNewDestination = FALSE;

BOOLEAN gfFirstMapscreenFrame = FALSE;

BOOLEAN gfMapPanelWasRedrawn = FALSE;

uint8_t gubMAP_HandInvDispText[NUM_INV_SLOTS];

// currently selected character's list index
int8_t bSelectedInfoChar = -1;

// map sort button images
int32_t giMapSortButtonImage[MAX_SORT_METHODS] = {-1, -1, -1, -1, -1, -1};
int32_t giMapSortButton[MAX_SORT_METHODS] = {-1, -1, -1, -1, -1, -1};

int32_t giCharInfoButtonImage[2];
int32_t giCharInfoButton[2] = {-1, -1};

int32_t giMapInvButtonDoneImage;
int32_t giMapInvDoneButton = -1;

int32_t giMapContractButton = -1;
int32_t giMapContractButtonImage;

// int32_t giMapInvButton = -1;
// int32_t giMapInvButtonImage;

int32_t giSortStateForMapScreenList = 0;

int32_t giCommonGlowBaseTime = 0;
int32_t giFlashAssignBaseTime = 0;
int32_t giFlashContractBaseTime = 0;
uint32_t guiFlashCursorBaseTime = 0;
int32_t giPotCharPathBaseTime = 0;

extern uint8_t gubHandPos;
extern uint16_t gusOldItemIndex;
extern uint16_t gusNewItemIndex;
extern BOOLEAN gfDeductPoints;

extern void CleanUpStack(struct OBJECTTYPE *pObj, struct OBJECTTYPE *pCursorObj);

uint32_t guiCHARLIST;
uint32_t guiCHARINFO;
uint32_t guiSleepIcon;
uint32_t guiCROSS;
uint32_t guiMAPINV;
uint32_t guiMapInvSecondHandBlockout;
uint32_t guiULICONS;
uint32_t guiNewMailIcons;
uint32_t guiLEVELMARKER;  // the white rectangle highlighting the current level on the map border

// misc mouse regions
struct MOUSE_REGION gCharInfoFaceRegion;
struct MOUSE_REGION gCharInfoHandRegion;
struct MOUSE_REGION gMPanelRegion;
struct MOUSE_REGION gMapViewRegion;
struct MOUSE_REGION gMapScreenMaskRegion;
struct MOUSE_REGION gTrashCanRegion;

// mouse regions for team info panel
struct MOUSE_REGION gTeamListNameRegion[MAX_CHARACTER_COUNT];
struct MOUSE_REGION gTeamListAssignmentRegion[MAX_CHARACTER_COUNT];
struct MOUSE_REGION gTeamListSleepRegion[MAX_CHARACTER_COUNT];
struct MOUSE_REGION gTeamListLocationRegion[MAX_CHARACTER_COUNT];
struct MOUSE_REGION gTeamListDestinationRegion[MAX_CHARACTER_COUNT];
struct MOUSE_REGION gTeamListContractRegion[MAX_CHARACTER_COUNT];

struct path *gpCharacterPreviousMercPath[MAX_CHARACTER_COUNT];
struct path *gpHelicopterPreviousMercPath = NULL;

// GLOBAL VARIABLES (EXTERNAL)

extern BOOLEAN fHoveringHelicopter;
extern BOOLEAN fDeletedNode;
extern BOOLEAN gfRenderPBInterface;
extern BOOLEAN fMapScreenBottomDirty;
extern BOOLEAN fResetTimerForFirstEntryIntoMapScreen;
extern BOOLEAN gfStartedFromMapScreen;

extern BOOLEAN gfUsePersistantPBI;

extern BOOLEAN gfOneFramePauseOnExit;

extern int32_t iDialogueBox;
extern int32_t giMapInvDescButton;

extern uint32_t guiBrownBackgroundForTeamPanel;

// the town mine info box
extern int32_t ghTownMineBox;
// border and bottom buttons
extern int32_t giMapBorderButtons[];
extern uint32_t guiMapButtonInventory[];

// the mine icon
extern uint32_t guiMINEICON;
extern uint32_t guiSecItemHiddenVO;

extern uint32_t guiUIMessageTimeDelay;

extern struct path *pTempCharacterPath;
extern struct path *pTempHelicopterPath;

extern BOOLEAN gfAutoAIAware;
extern void HandlePreBattleInterfaceStates();

extern struct OBJECTTYPE *gpItemDescObject;

extern struct SOLDIERTYPE *pProcessingSoldier;

// faces stuff
extern FACETYPE *gpCurrentTalkingFace;
// extern BOOLEAN	gfFacePanelActive;

// externs for highlighting of ammo/weapons
extern uint32_t guiMouseOverItemTime;
extern BOOLEAN gfCheckForMouseOverItem;
extern int8_t gbCheckForMouseOverItemPos;

// Autoresolve sets this variable which defaults to -1 when not needed.
extern int16_t gsEnemyGainedControlOfSectorID;
extern int16_t gsCiviliansEatenByMonsters;

extern BOOLEAN gfFadeOutDone;

extern uint32_t guiPendingScreen;

extern wchar_t gzUserDefinedButton1[128];
extern wchar_t gzUserDefinedButton2[128];

extern BOOLEAN gfMilitiaPopupCreated;

#ifdef JA2TESTVERSION
extern int16_t MSYS_CurrentMX;
extern int16_t MSYS_CurrentMY;
#endif

// PROTOTYPES

// basic input
void GetMapKeyboardInput(uint32_t *puiNewEvent);
void PollLeftButtonInMapView(uint32_t *puiNewEvent);
void PollRightButtonInMapView(uint32_t *puiNewEvent);

// background render
void BlitBackgroundToSaveBuffer(void);

// Drawing the Map
uint32_t HandleMapUI();
void RenderMapCursorsIndexesAnims(void);
static BOOLEAN GetMapXY(int16_t sX, int16_t sY, uint8_t *psMapWorldX, uint8_t *psMapWorldY);

void StartConfirmMapMoveMode(int16_t sMapY);
void EndConfirmMapMoveMode(void);

void CancelMapUIMessage(void);
void MonitorMapUIMessage(void);

static void RenderMapHighlight(uint8_t sMapX, uint8_t sMapY, uint16_t usLineColor,
                               BOOLEAN fStationary);
void ShadeMapElem(uint8_t sMapX, uint8_t sMapY);
void PopupText(wchar_t *pFontString, ...);
void DrawString(wchar_t *pString, uint16_t uiX, uint16_t uiY, uint32_t uiFont);

// Clock
void SetClock(wchar_t *pString);
void SetClockMin(wchar_t *, ...);
void SetClockHour(wchar_t *pStringA, ...);
void SetHourAlternate(wchar_t *pStringA, ...);
void SetDayAlternate(wchar_t *pStringA, ...);

void RenderIconsForUpperLeftCornerPiece(int8_t bCharNumber);

void DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface(int16_t sMapX,
                                                                                int16_t sMapY);
void HandleCursorOverRifleAmmo();

void SetUpCursorForStrategicMap(void);
void HandleAnimatedCursorsForMapScreen();
void CheckToSeeIfMouseHasLeftMapRegionDuringPathPlotting();

// handle change in info char
void HandleChangeOfInfoChar(void);

// update town mine pop up
void UpdateTownMinePopUpDisplay(void);

// Map Buttons
void GlowFace(void);

void ResetMapButtons();

// Drawing Strings
void DrawName(wchar_t *pName, int16_t sRowIndex, int32_t iFont);
void DrawAssignment(int16_t sCharNumber, int16_t sRowIndex, int32_t iFont);
void DrawLocation(int16_t sCharNumber, int16_t sRowIndex, int32_t iFont);
void DrawDestination(int16_t sCharNumber, int16_t sRowIndex, int32_t iFont);
void DrawTimeRemaining(int16_t sCharNumber, int32_t iFont, uint8_t ubFontColor);

void HandleAssignmentsDoneAndAwaitingFurtherOrders(void);
void HandleCommonGlowTimer(void);

void CheckIfPlottingForCharacterWhileAirCraft(void);

// handle highlighting of team panel lines
void HandleChangeOfHighLightedLine(void);

// automatically pause/unpause time compression during certain events
void UpdatePausedStatesDueToTimeCompression(void);

// handle pre battle interface in relation to inventory
void HandlePreBattleInterfaceWithInventoryPanelUp(void);
void CreateDestroyMapCharacterScrollButtons(void);

// sort buttons for team list
void AddTeamPanelSortButtonsForMapScreen(void);
void RemoveTeamPanelSortButtonsForMapScreen(void);
void SortListOfMercsInTeamPanel(BOOLEAN fRetainSelectedMercs);

// Rendering
void RenderCharacterInfoBackground(void);
void RenderTeamRegionBackground(void);
void RenderMapRegionBackground(void);
void HandleHighLightingOfLinesInTeamPanel(void);
void ClearTeamPanel();
void PlotTemporaryPaths(void);
void PlotPermanentPaths(void);
void RenderHandPosItem(void);
void ContractListRegionBoxGlow(uint16_t usCount);
void DisplayDestinationOfCurrentDestMerc(void);
void HandleCharBarRender(void);

// rebuild waypoints for selected character list
void RebuildWayPointsForAllSelectedCharsGroups(void);

extern BOOLEAN HandleNailsVestFetish(struct SOLDIERTYPE *pSoldier, uint32_t uiHandPos,
                                     uint16_t usReplaceItem);

BOOLEAN CharacterIsInTransitAndHasItemPickedUp(int8_t bCharacterNumber);

void InterruptTimeForMenus(void);

// Pop Up Boxes
void CreateAttributeBox(void);
void CreateVehicleBox(void);
void CreateContractBox(struct SOLDIERTYPE *pCharacter);
void CreateAssignmentsBox(void);
void CreateTrainingBox(void);
void CreateMercRemoveAssignBox(void);

void DetermineWhichAssignmentMenusCanBeShown(void);

void DetermineIfContractMenuCanBeShown(void);
void ContractRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void ContractRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void HandleShadingOfLinesForContractMenu(void);

void UpdateStatusOfMapSortButtons(void);

void DisplayIconsForMercsAsleep(void);
BOOLEAN CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed(int8_t bCharId);

void HandlePostAutoresolveMessages();

// screen mask for pop up menus
void ClearScreenMaskForMapScreenExit(void);

void ResetAllSelectedCharacterModes(void);

// The Mouse/Btn Creation/Destruction

// destroy regions/buttons
void DestroyMouseRegionsForTeamList(void);

// create new regions/buttons
void CreateMouseRegionsForTeamList(void);

void EnableDisableTeamListRegionsAndHelpText(void);

// merc about to leave, flash contract time
BOOLEAN AnyMercsLeavingRealSoon();
void HandleContractTimeFlashForMercThatIsAboutLeave(void);

// Mouse Region Callbacks

// team list
void TeamListInfoRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListInfoRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListAssignmentRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListAssignmentRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListDestinationRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListDestinationRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListContractRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListContractRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListSleepRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TeamListSleepRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

void FaceRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void FaceRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void ItemRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void ItemRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// mapscreen mouse region screen mask btn callback
void MapScreenMarkRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// inventory mvt and click callbacks
void MAPInvMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void MAPInvClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void MAPInvClickCamoCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void MAPInvMoveCamoCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void InvmaskRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void TrashCanBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

extern void KeyRingItemPanelButtonCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// handle talking
void HandleSpontanousTalking(void);
BOOLEAN ContinueDialogue(struct SOLDIERTYPE *pSoldier, BOOLEAN fDone);
extern void CreateDestroyMilitiaSectorButtons(void);

// mouse position test
BOOLEAN IsCursorWithInRegion(int16_t sLeft, int16_t sRight, int16_t sTop, int16_t sBottom);

// pop up boxes
void CreateVehicleBox();

void TestMessageSystem(void);

BOOLEAN CheckIfClickOnLastSectorInPath(uint8_t sX, uint8_t sY);

// update any bad assignments..error checking
void UpdateBadAssignments(void);

// hwo many on team, if less than 2, disable prev/next merc buttons
void UpdateTheStateOfTheNextPrevMapScreenCharacterButtons(void);

// inventory
void CreateDestroyTrashCanRegion(void);
void DoneInventoryMapBtnCallback(GUI_BUTTON *btn, int32_t reason);

// handle cursor for invenetory mode..update to object selected, if needed
void HandleMapInventoryCursor();
void MAPEndItemPointer();
void MAPBeginItemPointer(struct SOLDIERTYPE *pSoldier, uint8_t ubHandPos);

// create/destroy inventory button as needed
void CreateDestroyMapInvButton();
void PrevInventoryMapBtnCallback(GUI_BUTTON *btn, int32_t reason);
void NextInventoryMapBtnCallback(GUI_BUTTON *btn, int32_t reason);

// check if cursor needs to be set to checkmark or to the walking guy?
void UpdateCursorIfInLastSector(void);

void ContractButtonCallback(GUI_BUTTON *btn, int32_t reason);
void MapScreenDefaultOkBoxCallback(uint8_t bExitValue);

// blt inventory panel
void BltCharInvPanel();

// show character information
void DrawCharacterInfo(int16_t sCharNumber);
void DisplayCharacterInfo(void);
void UpDateStatusOfContractBox(void);

// get which index in the mapscreen character list is this guy
int32_t GetIndexForThisSoldier(struct SOLDIERTYPE *pSoldier);

void CheckForAndRenderNewMailOverlay();

BOOLEAN MapCharacterHasAccessibleInventory(int8_t bCharNumber);
void CheckForInventoryModeCancellation();

void ChangeMapScreenMaskCursor(uint16_t usCursor);
void CancelOrShortenPlottedPath(void);

BOOLEAN HandleCtrlOrShiftInTeamPanel(int8_t bCharNumber);

int32_t GetContractExpiryTime(struct SOLDIERTYPE *pSoldier);

void ConvertMinTimeToETADayHourMinString(uint32_t uiTimeInMin, wchar_t *sString, size_t bufSize);
int32_t GetGroundTravelTimeOfCharacter(int8_t bCharNumber);

int16_t CalcLocationValueForChar(int32_t iCounter);

void CancelChangeArrivalSectorMode();

void MakeMapModesSuitableForDestPlotting(int8_t bCharNumber);

BOOLEAN AnyMovableCharsInOrBetweenThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

void SwapCharactersInList(int32_t iCharA, int32_t iCharB);

BOOLEAN CanChangeDestinationForCharSlot(int8_t bCharNumber, BOOLEAN fShowErrorMessage);

BOOLEAN RequestGiveSkyriderNewDestination(void);
void ExplainWhySkyriderCantFly(void);
uint8_t PlayerMercsInHelicopterSector(void);

void HandleNewDestConfirmation(uint8_t sMapX, uint8_t sMapY);
void RandomAwakeSelectedMercConfirmsStrategicMove(void);
void DestinationPlottingCompleted(void);

void HandleMilitiaRedistributionClick(void);

void StartChangeSectorArrivalMode(void);
BOOLEAN CanMoveBullseyeAndClickedOnIt(uint8_t sMapX, uint8_t sMapY);
void CreateBullsEyeOrChopperSelectionPopup(void);
void BullsEyeOrChopperSelectionPopupCallback(uint8_t ubExitValue);

void WakeUpAnySleepingSelectedMercsOnFootOrDriving(void);

void GetMapscreenMercAssignmentString(struct SOLDIERTYPE *pSoldier, wchar_t sString[]);
void GetMapscreenMercLocationString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                    int sStringSize);
void GetMapscreenMercDestinationString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                       int sStringSize);
void GetMapscreenMercDepartureString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                     int sStringSize, uint8_t *pubFontColor);

void InitPreviousPaths(void);
void RememberPreviousPathForAllSelectedChars(void);
void RestorePreviousPaths(void);
void ClearPreviousPaths(void);

void SelectAllCharactersInSquad(int8_t bSquadNumber);

BOOLEAN CanDrawSectorCursor(void);
void RestoreMapSectorCursor(uint8_t sMapX, uint8_t sMapY);

void RequestToggleMercInventoryPanel(void);

void RequestContractMenu(void);
void ChangeCharacterListSortMethod(int32_t iValue);

void MapscreenMarkButtonsDirty();

extern BOOLEAN CanRedistributeMilitiaInSector(int16_t sClickedSectorX, int16_t sClickedSectorY,
                                              int8_t bClickedTownId);

extern int32_t GetNumberOfMercsInUpdateList(void);

#ifdef JA2TESTVERSION
void TestDumpStatChanges(void);
void DumpSectorDifficultyInfo(void);
void DumpItemsList(void);
#endif

// the tries to select a mapscreen character by his soldier ID
BOOLEAN SetInfoChar(uint8_t ubID) {
  int8_t bCounter;

  for (bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++) {
    // skip invalid characters
    if (gCharactersList[bCounter].fValid == TRUE) {
      if (gCharactersList[bCounter].usSolID == (uint16_t)ubID) {
        ChangeSelectedInfoChar(bCounter, TRUE);
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

void DisplayDestinationOfCurrentDestMerc(void) {
  // will display the dest of the current dest merc
  wchar_t sString[32];
  int16_t sX, sY;
  int16_t sSector;

  SetFont(MAP_SCREEN_FONT);

  sSector =
      GetLastSectorIdInCharactersPath(GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID));

  SetBoxForeground(ghVehicleBox, FONT_LTGREEN);
  SetBoxBackground(ghVehicleBox, FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), L"%s%s", pMapVertIndex[SectorID16_Y(sSector)],
           pMapHortIndex[SectorID16_X(sSector)]);
  FindFontCenterCoordinates(DEST_PLOT_X, DEST_PLOT_Y, 70, GetFontHeight(MAP_SCREEN_FONT), sString,
                            MAP_SCREEN_FONT, &sX, &sY);

  RestoreExternBackgroundRect(DEST_PLOT_X, DEST_PLOT_Y, 70, GetFontHeight(MAP_SCREEN_FONT));
  mprintf(sX, sY, sString);
}

void ContractBoxGlow(void) {}

void ContractListRegionBoxGlow(uint16_t usCount) {
  static int32_t iColorNum = 10;
  static BOOLEAN fDelta = FALSE;
  uint16_t usColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  int16_t usY = 0;
  int16_t sYAdd = 0;

  // if not glowing right now, leave
  if ((giContractHighLine == -1) || (fResetContractGlow == TRUE) || fShowInventoryFlag) {
    iColorNum = 0;
    fDelta = TRUE;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  // change direction of glow?
  if ((iColorNum == 0) || (iColorNum == 10)) {
    fDelta = !fDelta;
  }

  // increment color
  if (!fDelta)
    iColorNum++;
  else
    iColorNum--;

  if (usCount >= FIRST_VEHICLE) {
    sYAdd = 6;
  } else {
    sYAdd = 0;
  }

  // y start position of box
  usY = (Y_OFFSET * usCount - 1) + (Y_START + (usCount * Y_SIZE) + sYAdd);

  // glow contract box
  usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                   GlowColorsA[iColorNum].ubBlue));
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  RectangleDraw(TRUE, TIME_REMAINING_X, usY, TIME_REMAINING_X + TIME_REMAINING_WIDTH,
                usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);
  InvalidateRegion(TIME_REMAINING_X - 1, usY, TIME_REMAINING_X + TIME_REMAINING_WIDTH + 1,
                   usY + GetFontHeight(MAP_SCREEN_FONT) + 3);
  JSurface_Unlock(vsFB);

  /*
          // restore background
          if((iColorNum==0)||(iColorNum==1))
                  RestoreExternBackgroundRect( CONTRACT_X, CONTRACT_Y, CONTRACT_WIDTH+1,
     CONTRACT_HEIGHT+1 );
  */
}

void GlowFace(void) {
  static int32_t iColorNum = 10;
  static BOOLEAN fDelta = FALSE;
  static BOOLEAN fOldFaceGlow = FALSE;
  uint16_t usColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // not glowing right now, leave
  if (fShowFaceHightLight == FALSE) {
    iColorNum = 0;
    fDelta = TRUE;

    if (fOldFaceGlow == TRUE) {
      RestoreExternBackgroundRect(9, 18, (uint16_t)(61 - 9), (uint16_t)(64 - 18));
    }

    fOldFaceGlow = FALSE;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  fOldFaceGlow = TRUE;

  // change direction of glow?
  if ((iColorNum == 0) || (iColorNum == 10)) {
    fDelta = !fDelta;
  }

  // increment color
  if (!fDelta)
    iColorNum++;
  else
    iColorNum--;

  // glow contract box
  usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                   GlowColorsA[iColorNum].ubBlue));
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  RectangleDraw(TRUE, 9, 18, 60, 63, usColor, pDestBuf);
  InvalidateRegion(9, 18, 61, 64);
  JSurface_Unlock(vsFB);

  // restore background
  if ((iColorNum == 0) || (iColorNum == 1))
    RestoreExternBackgroundRect(9, 18, (uint16_t)(61 - 9), (uint16_t)(64 - 18));
}

void GlowItem(void) {
  static int32_t iColorNum = 10;
  static BOOLEAN fDelta = FALSE;
  static BOOLEAN fOldItemGlow = FALSE;
  uint16_t usColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  // not glowing right now, leave
  if (fShowItemHighLight == FALSE) {
    iColorNum = 0;
    fDelta = TRUE;

    if (fOldItemGlow == TRUE) {
      RestoreExternBackgroundRect(3, 80, (uint16_t)(65 - 3), (uint16_t)(105 - 80));
    }

    fOldItemGlow = FALSE;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  fOldItemGlow = TRUE;

  // change direction of glow?
  if ((iColorNum == 0) || (iColorNum == 10)) {
    fDelta = !fDelta;
  }

  // increment color
  if (!fDelta)
    iColorNum++;
  else
    iColorNum--;

  // restore background
  if ((iColorNum == 0) || (iColorNum == 1)) {
    RestoreExternBackgroundRect(3, 80, (uint16_t)(65 - 3), (uint16_t)(105 - 80));
    RenderHandPosItem();
  }

  // glow contract box
  usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                   GlowColorsA[iColorNum].ubBlue));
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  RectangleDraw(TRUE, 3, 80, 64, 104, usColor, pDestBuf);
  InvalidateRegion(3, 80, 65, 105);
  JSurface_Unlock(vsFB);
}

void GlowTrashCan(void) {
  static int32_t iColorNum = 10;
  static BOOLEAN fOldTrashCanGlow = FALSE;
  uint16_t usColor;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  if (fShowInventoryFlag == FALSE) {
    fShowTrashCanHighLight = FALSE;
  }

  // not glowing right now, leave
  if (fShowTrashCanHighLight == FALSE) {
    iColorNum = 0;

    if (fOldTrashCanGlow == TRUE) {
      RestoreExternBackgroundRect(TRASH_CAN_X, TRASH_CAN_Y, (uint16_t)(TRASH_CAN_WIDTH + 2),
                                  (uint16_t)(TRASH_CAN_HEIGHT + 2));
    }

    fOldTrashCanGlow = FALSE;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  fOldTrashCanGlow = TRUE;

  // glow contract box
  usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                   GlowColorsA[iColorNum].ubBlue));
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  RectangleDraw(TRUE, TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH,
                TRASH_CAN_Y + TRASH_CAN_HEIGHT, usColor, pDestBuf);
  InvalidateRegion(TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH + 1,
                   TRASH_CAN_Y + TRASH_CAN_HEIGHT + 1);
  JSurface_Unlock(vsFB);

  // restore background
  if ((iColorNum == 0) || (iColorNum == 1))
    RestoreExternBackgroundRect(TRASH_CAN_X, TRASH_CAN_Y, (uint16_t)(TRASH_CAN_WIDTH + 2),
                                (uint16_t)(TRASH_CAN_HEIGHT + 2));
}

void DrawFace(int16_t sCharNumber) {
  struct SOLDIERTYPE *pSoldier = NULL;
  static int16_t sOldId = -1;

  // draws the face of the currently selected merc, being displayed int he upper left hand corner

  // grab the soldier
  if (bSelectedInfoChar != -1) {
    if (gCharactersList[bSelectedInfoChar].fValid) {
      GetSoldier(&pSoldier, gCharactersList[bSelectedInfoChar].usSolID);
    }
  }

  if (pSoldier == NULL) {
    return;
  }

  if ((gCharactersList[bSelectedInfoChar].usSolID == sOldId) && (fReDrawFace == FALSE)) {
    // are the same, return
    return;
  }

  // get old id value
  sOldId = gCharactersList[bSelectedInfoChar].usSolID;

  // reset redraw of face
  fReDrawFace = FALSE;

  // render their face
  RenderSoldierFace(pSoldier, SOLDIER_PIC_X, SOLDIER_PIC_Y, TRUE);

  return;
}

void RenderHandPosItem(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  // renders the inventory item in char's right hand

  // ARM: if already in the inventory panel, don't show the item again here, seeing it twice is
  // confusing
  if (fShowInventoryFlag) {
    return;
  }

  // grab the soldier
  if (bSelectedInfoChar != -1) {
    if (gCharactersList[bSelectedInfoChar].fValid) {
      GetSoldier(&pSoldier, gCharactersList[bSelectedInfoChar].usSolID);
    }
  }

  if (pSoldier == NULL) {
    return;
  }

  // check if still alive?
  if (pSoldier->bLife == 0) {
    return;
  }

  SetFont(BLOCKFONT2);
  SetFontForeground(CHAR_INFO_PANEL_BLOCK_COLOR);
  SetFontBackground(FONT_BLACK);

  INVRenderItem(vsSaveBuffer, pSoldier, &(pSoldier->inv[HANDPOS]), SOLDIER_HAND_X, SOLDIER_HAND_Y,
                58, 23, DIRTYLEVEL2, NULL, 0, FALSE, 0);
}

void RenderIconsForUpperLeftCornerPiece(int8_t bCharNumber) {
  struct VObject *hHandle;

  GetVideoObject(&hHandle, guiULICONS);

  // if merc is an AIM merc
  if (Menptr[gCharactersList[bCharNumber].usSolID].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
    // finite contract length icon
    BltVideoObject(vsSaveBuffer, hHandle, 0, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y);
  }

  // if merc has life insurance
  if (Menptr[gCharactersList[bCharNumber].usSolID].usLifeInsurance > 0) {
    // draw life insurance icon
    BltVideoObject(vsSaveBuffer, hHandle, 2, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING);
  }

  // if merc has a medical deposit
  if (Menptr[gCharactersList[bCharNumber].usSolID].usMedicalDeposit > 0) {
    // draw medical deposit icon
    BltVideoObject(vsSaveBuffer, hHandle, 1, CHAR_ICON_X,
                   CHAR_ICON_CONTRACT_Y + (2 * CHAR_ICON_SPACING));
  }
}

void DrawPay(int16_t sCharNumber) {
  // will draw the pay
  int32_t uiSalary;
  wchar_t sString[7];
  int16_t usX, usY;
  int16_t usMercProfileID;

  // get merc id
  usMercProfileID = MercPtrs[gCharactersList[sCharNumber].usSolID]->ubProfile;

  // grab salary
  uiSalary = ((uint32_t)gMercProfiles[usMercProfileID].sSalary);

  // font stuff
  SetFontForeground(CHAR_TITLE_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  // parse salary
  swprintf(sString, ARR_SIZE(sString), L"%d", uiSalary);

  // right justify salary
  FindFontRightCoordinates(CHAR_PAY_X, CHAR_PAY_Y, CHAR_PAY_WID, CHAR_PAY_HEI, sString, CHAR_FONT,
                           &usX, &usY);

  // draw salary
  DrawString(sString, usX, usY, CHAR_FONT);
}

void DrawCharBars(void) {
  uint16_t usSoldierID;
  struct SOLDIERTYPE *pSoldier;

  // will draw the heath, morale and breath bars for a character being displayed in the upper left
  // hand corner

  if ((bSelectedInfoChar == -1) && (bSelectedDestChar == -1)) {
    // error, no character to display right now
    return;
  } else {
    // valid character
    if (bSelectedInfoChar != -1) {
      usSoldierID = gCharactersList[bSelectedInfoChar].usSolID;
    } else {
      usSoldierID = gCharactersList[bSelectedDestChar].usSolID;
    }

    // grab soldier's id number
    GetSoldier(&pSoldier, usSoldierID);

    if (pSoldier == NULL) {
      // no soldier
      return;
    }

    // skip POWs, dead guys
    if ((pSoldier->bLife == 0) || (GetSolAssignment(pSoldier) == ASSIGNMENT_DEAD) ||
        (GetSolAssignment(pSoldier) == ASSIGNMENT_POW)) {
      return;
    }

    // current health
    DrawLifeUIBarEx(pSoldier, BAR_INFO_X, BAR_INFO_Y, 3, 42, TRUE, vsFB);

    // robot doesn't have energy/fuel
    if (!AM_A_ROBOT(pSoldier)) {
      // current energy/fuel
      DrawBreathUIBarEx(pSoldier, BAR_INFO_X + 6, BAR_INFO_Y, 3, 42, TRUE, vsFB);
    }

    // vehicles and robot don't have morale
    if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(pSoldier)) {
      // draw morale bar
      DrawMoraleUIBarEx(pSoldier, BAR_INFO_X + 12, BAR_INFO_Y, 3, 42, TRUE, vsFB);
    }
  }

  return;
}

void DrawCharStats(int16_t sCharNum) {
  // will draw the characters stats, max life, strength, dex, and skills
  wchar_t sString[9];
  int16_t usX, usY;
  // struct VObject* hCrossHandle;
  struct SOLDIERTYPE *pSoldier = NULL;

  pSoldier = GetSoldierByID(gCharactersList[sCharNum].usSolID);

  // set up font
  SetFont(CHAR_FONT);
  SetFontForeground(CHAR_TEXT_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  // strength
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bStrength);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeStrengthTime) &&
      (pSoldier->uiChangeStrengthTime != 0)) {
    if (pSoldier->usValueGoneUp & STRENGTH_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(STR_X, STR_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, STR_Y, CHAR_FONT);

  // dexterity
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bDexterity);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeDexterityTime) &&
      (pSoldier->uiChangeDexterityTime != 0)) {
    if (pSoldier->usValueGoneUp & DEX_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(DEX_X, DEX_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, DEX_Y, CHAR_FONT);

  // agility
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bAgility);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeAgilityTime) &&
      (pSoldier->uiChangeAgilityTime != 0)) {
    if (pSoldier->usValueGoneUp & AGIL_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(AGL_X, AGL_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, AGL_Y, CHAR_FONT);

  // wisdom
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bWisdom);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeWisdomTime) &&
      (pSoldier->uiChangeWisdomTime != 0)) {
    if (pSoldier->usValueGoneUp & WIS_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(WIS_X, WIS_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, WIS_Y, CHAR_FONT);

  // leadership
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bLeadership);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeLeadershipTime) &&
      (pSoldier->uiChangeLeadershipTime != 0)) {
    if (pSoldier->usValueGoneUp & LDR_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(LDR_X, LDR_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, LDR_Y, CHAR_FONT);

  // experience level
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bExpLevel);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeLevelTime) &&
      (pSoldier->uiChangeLevelTime != 0)) {
    if (pSoldier->usValueGoneUp & LVL_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(LVL_X, LVL_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, LVL_Y, CHAR_FONT);

  // marksmanship
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bMarksmanship);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeMarksmanshipTime) &&
      (pSoldier->uiChangeMarksmanshipTime != 0)) {
    if (pSoldier->usValueGoneUp & MRK_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(MRK_X, MRK_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, MRK_Y, CHAR_FONT);

  // explosives
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bExplosive);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeExplosivesTime) &&
      (pSoldier->uiChangeExplosivesTime != 0)) {
    if (pSoldier->usValueGoneUp & EXP_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(EXP_X, EXP_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, EXP_Y, CHAR_FONT);

  // mechanical
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bMechanical);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeMechanicalTime) &&
      (pSoldier->uiChangeMechanicalTime != 0)) {
    if (pSoldier->usValueGoneUp & MECH_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(MEC_X, MEC_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, MEC_Y, CHAR_FONT);

  // medical
  swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bMedical);

  if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeMedicalTime) &&
      (pSoldier->uiChangeMedicalTime != 0)) {
    if (pSoldier->usValueGoneUp & MED_INCREASE) {
      SetFontForeground(FONT_LTGREEN);
    } else {
      SetFontForeground(FONT_RED);
    }
  } else {
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
  }

  // right justify
  FindFontRightCoordinates(MED_X, MED_Y, STAT_WID, STAT_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, MED_Y, CHAR_FONT);

  SetFontForeground(CHAR_TEXT_FONT_COLOR);

  return;
}

void DrawCharHealth(int16_t sCharNum) {
  uint32_t uiHealthPercent = 0;
  wchar_t sString[9];
  int16_t usX, usY;
  struct SOLDIERTYPE *pSoldier = NULL;

  pSoldier = GetSoldierByID(gCharactersList[sCharNum].usSolID);

  if (pSoldier->bAssignment != ASSIGNMENT_POW) {
    // find starting X coordinate by centering all 3 substrings together, then print them separately
    // (different colors)!
    swprintf(sString, ARR_SIZE(sString), L"%d/%d", pSoldier->bLife, pSoldier->bLifeMax);
    FindFontCenterCoordinates(CHAR_HP_X, CHAR_HP_Y, CHAR_HP_WID, CHAR_HP_HEI, sString, CHAR_FONT,
                              &usX, &usY);

    if (pSoldier->bLifeMax > 0) {
      uiHealthPercent = (pSoldier->bLife * 100) / pSoldier->bLifeMax;
    }

    // how is characters life?
    if (uiHealthPercent == 0) {
      // he's dead, Jim
      SetFontForeground(FONT_METALGRAY);
    } else if (uiHealthPercent < 25) {
      // very bad
      SetFontForeground(FONT_RED);
    } else if (uiHealthPercent < 50) {
      // not good
      SetFontForeground(FONT_YELLOW);
    } else {
      // ok
      SetFontForeground(CHAR_TEXT_FONT_COLOR);
    }

    // current life
    swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bLife);
    DrawString(sString, usX, CHAR_HP_Y, CHAR_FONT);
    usX += StringPixLength(sString, CHAR_FONT);

    // slash
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
    wcscpy(sString, L"/");
    DrawString(sString, usX, CHAR_HP_Y, CHAR_FONT);
    usX += StringPixLength(sString, CHAR_FONT);

    if ((GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeHealthTime) &&
        (pSoldier->uiChangeHealthTime != 0)) {
      if (pSoldier->usValueGoneUp & HEALTH_INCREASE) {
        SetFontForeground(FONT_LTGREEN);
      } else {
        SetFontForeground(FONT_RED);
      }
    } else {
      SetFontForeground(CHAR_TEXT_FONT_COLOR);
    }

    // maximum life
    swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bLifeMax);
    DrawString(sString, usX, CHAR_HP_Y, CHAR_FONT);
  } else {
    // POW - health unknown
    SetFontForeground(CHAR_TEXT_FONT_COLOR);
    swprintf(sString, ARR_SIZE(sString), pPOWStrings[1]);
    FindFontCenterCoordinates(CHAR_HP_X, CHAR_HP_Y, CHAR_HP_WID, CHAR_HP_HEI, sString, CHAR_FONT,
                              &usX, &usY);
    DrawString(sString, usX, CHAR_HP_Y, CHAR_FONT);
  }

  SetFontForeground(CHAR_TEXT_FONT_COLOR);
}

// "character" refers to hired people AND vehicles
void DrawCharacterInfo(int16_t sCharNumber) {
  wchar_t sString[80];
  int16_t usX, usY;
  int16_t usMercProfileID;
  int32_t iTimeRemaining = 0;
  int32_t iDailyCost = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  uint32_t uiArrivalTime;

  if (gCharactersList[sCharNumber].fValid == FALSE) {
    return;
  }

  pSoldier = MercPtrs[gCharactersList[sCharNumber].usSolID];

  if (GetSolProfile(pSoldier) == NO_PROFILE) {
    return;
  }

  // draw particular info about a character that are neither attributes nor skills

  // get profile information
  usMercProfileID = GetSolProfile(pSoldier);

  // set font stuff
  SetFont(CHAR_FONT);
  SetFontForeground(CHAR_TEXT_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  // Nickname (beneath Picture)
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // vehicle
    wcscpy(sString, pShortVehicleStrings[pVehicleList[pSoldier->bVehicleID].ubVehicleType]);
  } else {
    // soldier
    wcscpy(sString, gMercProfiles[usMercProfileID].zNickname);
  }

  FindFontCenterCoordinates(PIC_NAME_X, PIC_NAME_Y, PIC_NAME_WID, PIC_NAME_HEI, sString, CHAR_FONT,
                            &usX, &usY);
  DrawString(sString, usX, usY, CHAR_FONT);

  // Full name (Top Box)
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // vehicle
    wcscpy(sString, pVehicleStrings[pVehicleList[pSoldier->bVehicleID].ubVehicleType]);
  } else {
    // soldier
    wcscpy(sString, gMercProfiles[usMercProfileID].zName);
  }

  FindFontCenterCoordinates(CHAR_NAME_X, CHAR_NAME_Y, CHAR_NAME_WID, CHAR_NAME_HEI, sString,
                            CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, usY, CHAR_FONT);

  // Assignment
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    // show vehicle type
    wcscpy(sString, pShortVehicleStrings[pVehicleList[pSoldier->iVehicleId].ubVehicleType]);
  } else {
    wcscpy(sString, pAssignmentStrings[pSoldier->bAssignment]);
  }

  FindFontCenterCoordinates(CHAR_ASSIGN_X, CHAR_ASSIGN1_Y, CHAR_ASSIGN_WID, CHAR_ASSIGN_HEI,
                            sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, usY, CHAR_FONT);

  // second assignment line

  // train self / teammate / by other ?
  if ((GetSolAssignment(pSoldier) == TRAIN_SELF) ||
      (GetSolAssignment(pSoldier) == TRAIN_TEAMMATE) ||
      (GetSolAssignment(pSoldier) == TRAIN_BY_OTHER)) {
    wcscpy(sString, pAttributeMenuStrings[pSoldier->bTrainStat]);
  }
  // train town?
  else if (GetSolAssignment(pSoldier) == TRAIN_TOWN) {
    wcscpy(sString,
           pTownNames[GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier))]);
  }
  // repairing?
  else if (GetSolAssignment(pSoldier) == REPAIR) {
    if (pSoldier->fFixingRobot) {
      // robot
      wcscpy(sString, pRepairStrings[3]);
    }
    /*
                    else if ( pSoldier->fFixingSAMSite )
                    {
                            // SAM site
                            wcscpy( sString, pRepairStrings[ 1 ] );
                    }
    */
    else if (pSoldier->bVehicleUnderRepairID != -1) {
      // vehicle
      wcscpy(sString,
             pShortVehicleStrings[pVehicleList[pSoldier->bVehicleUnderRepairID].ubVehicleType]);
    } else {
      // items
      wcscpy(sString, pRepairStrings[0]);
    }
  }
  // in transit?
  else if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
    // show ETA
    ConvertMinTimeToETADayHourMinString(pSoldier->uiTimeSoldierWillArrive, sString,
                                        ARR_SIZE(sString));
  }
  // traveling ?
  else if (PlayerIDGroupInMotion(GetSoldierGroupId(pSoldier))) {
    // show ETA
    uiArrivalTime = GetWorldTotalMin() + CalculateTravelTimeOfGroupId(GetSoldierGroupId(pSoldier));
    ConvertMinTimeToETADayHourMinString(uiArrivalTime, sString, ARR_SIZE(sString));
  } else {
    // show location
    GetMapscreenMercLocationString(pSoldier, sString, ARR_SIZE(sString));
  }

  if (wcslen(sString) > 0) {
    FindFontCenterCoordinates(CHAR_ASSIGN_X, CHAR_ASSIGN2_Y, CHAR_ASSIGN_WID, CHAR_ASSIGN_HEI,
                              sString, CHAR_FONT, &usX, &usY);
    DrawString(sString, usX, usY, CHAR_FONT);
  }

  // draw health/condition
  DrawCharHealth(sCharNumber);

  // if a vehicle or robot
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
    // we're done - the remainder applies only to people
    return;
  }

  // draw attributes & skills for currently displayed character
  DrawCharStats(sCharNumber);

  // remaining contract length

  // dead?
  if (pSoldier->bLife <= 0) {
    swprintf(sString, ARR_SIZE(sString), L"%s",
             gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
  }
  // what kind of merc
  else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ||
           GetSolProfile(pSoldier) == SLAY) {
    float dTimeLeft = 0.0;

    // amount of time left on contract
    iTimeRemaining = pSoldier->iEndofContractTime - GetWorldTotalMin();

    // if the merc is in transit
    if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
      // and if the ttime left on the cotract is greater then the contract time
      if (iTimeRemaining > (int32_t)(pSoldier->iTotalContractLength * NUM_MIN_IN_DAY)) {
        iTimeRemaining = (pSoldier->iTotalContractLength * NUM_MIN_IN_DAY);
      }
    }

    if (iTimeRemaining >= (24 * 60)) {
      // calculate the exact time left on the contract ( ex 1.8 days )
      dTimeLeft = (float)(iTimeRemaining / (60 * 24.0));

      // more than a day, display in green
      iTimeRemaining /= (60 * 24);
      if (IsSolAlive(pSoldier)) {
        SetFontForeground(FONT_LTGREEN);
      }

      swprintf(sString, ARR_SIZE(sString), L"%.1f%s/%d%s", dTimeLeft,
               gpStrategicString[STR_PB_DAYS_ABBREVIATION], pSoldier->iTotalContractLength,
               gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
    } else {
      // less than a day, display hours left in red
      if (iTimeRemaining > 5) {
        BOOLEAN fNeedToIncrement = FALSE;

        if (iTimeRemaining % 60 != 0) fNeedToIncrement = TRUE;

        iTimeRemaining /= 60;

        if (fNeedToIncrement) iTimeRemaining++;
      } else {
        iTimeRemaining /= 60;
      }

      if (IsSolAlive(pSoldier)) {
        SetFontForeground(FONT_RED);
      }

      swprintf(sString, ARR_SIZE(sString), L"%d%s/%d%s", iTimeRemaining,
               gpStrategicString[STR_PB_HOURS_ABBREVIATION], pSoldier->iTotalContractLength,
               gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
    }
  } else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) {
    int32_t iBeenHiredFor = (GetWorldTotalMin() / NUM_MIN_IN_DAY) - pSoldier->iStartContractTime;

    swprintf(sString, ARR_SIZE(sString), L"%d%s/%d%s",
             gMercProfiles[GetSolProfile(pSoldier)].iMercMercContractLength,
             gpStrategicString[STR_PB_DAYS_ABBREVIATION], iBeenHiredFor,
             gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
  } else {
    swprintf(sString, ARR_SIZE(sString), L"%s",
             gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
  }

  // set font stuff
  SetFontForeground(CHAR_TEXT_FONT_COLOR);
  SetFontBackground(FONT_BLACK);

  // center and draw
  FindFontCenterCoordinates(CHAR_TIME_REMAINING_X, CHAR_TIME_REMAINING_Y, CHAR_TIME_REMAINING_WID,
                            CHAR_TIME_REMAINING_HEI, sString, CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, usY, CHAR_FONT);

  // salary
  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
    // daily rate
    if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK) {
      iDailyCost = (gMercProfiles[GetSolProfile(pSoldier)].uiBiWeeklySalary / 14);
    }
    if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK) {
      iDailyCost = (gMercProfiles[GetSolProfile(pSoldier)].uiWeeklySalary / 7);
    } else {
      iDailyCost = gMercProfiles[GetSolProfile(pSoldier)].sSalary;
    }
  } else {
    iDailyCost = gMercProfiles[GetSolProfile(pSoldier)].sSalary;
  }

  swprintf(sString, ARR_SIZE(sString), L"%d", iDailyCost);

  // insert commas and dollar sign
  InsertCommasForDollarFigure(sString);
  InsertDollarSignInToString(sString);

  FindFontRightCoordinates(CHAR_SALARY_X, CHAR_SALARY_Y, CHAR_SALARY_WID, CHAR_SALARY_HEI, sString,
                           CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, usY, CHAR_FONT);

  // medical deposit
  if (gMercProfiles[Menptr[gCharactersList[sCharNumber].usSolID].ubProfile].sMedicalDepositAmount >
      0) {
    swprintf(sString, ARR_SIZE(sString), L"%d",
             gMercProfiles[Menptr[gCharactersList[sCharNumber].usSolID].ubProfile]
                 .sMedicalDepositAmount);

    // insert commas and dollar sign
    InsertCommasForDollarFigure(sString);
    InsertDollarSignInToString(sString);

    FindFontRightCoordinates(CHAR_MEDICAL_X, CHAR_MEDICAL_Y, CHAR_MEDICAL_WID, CHAR_MEDICAL_HEI,
                             sString, CHAR_FONT, &usX, &usY);
    DrawString(sString, usX, CHAR_MEDICAL_Y, CHAR_FONT);
  }

  /*
          // life insurance
          swprintf(sString, ARR_SIZE(sString), L"%d", Menptr[ gCharactersList[ sCharNumber ].usSolID
     ].usLifeInsuranceAmount ); InsertCommasForDollarFigure( sString ); InsertDollarSignInToString(
     sString ); FindFontRightCoordinates(CHAR_LIFE_INSUR_X, CHAR_LIFE_INSUR_Y, CHAR_LIFE_INSUR_WID,
     CHAR_LIFE_INSUR_HEI, sString, CHAR_FONT, &usX, &usY); DrawString(sString,usX,usY, CHAR_FONT);
  */

  // morale
  if (pSoldier->bAssignment != ASSIGNMENT_POW) {
    if (pSoldier->bLife != 0) {
      GetMoraleString(MercPtrs[gCharactersList[sCharNumber].usSolID], sString);
    } else {
      wcscpy(sString, L"");
    }
  } else {
    // POW - morale unknown
    swprintf(sString, ARR_SIZE(sString), pPOWStrings[1]);
  }

  FindFontCenterCoordinates(CHAR_MORALE_X, CHAR_MORALE_Y, CHAR_MORALE_WID, CHAR_MORALE_HEI, sString,
                            CHAR_FONT, &usX, &usY);
  DrawString(sString, usX, CHAR_MORALE_Y, CHAR_FONT);

  return;
}

// this character is in transit has an item picked up
BOOLEAN CharacterIsInTransitAndHasItemPickedUp(int8_t bCharacterNumber) {
  // valid character?
  if (bCharacterNumber == -1) {
    // nope
    return (FALSE);
  }

  // second validity check
  if (gCharactersList[bCharacterNumber].fValid == FALSE) {
    // nope
    return (FALSE);
  }

  // character in transit?
  if (Menptr[gCharactersList[bCharacterNumber].usSolID].bAssignment != IN_TRANSIT) {
    // nope
    return (FALSE);
  }

  // item picked up?
  if (gMPanelRegion.Cursor != EXTERN_CURSOR) {
    return (FALSE);
  }

  return (TRUE);
}

void DisplayCharacterInfo(void) {
  Assert(bSelectedInfoChar < MAX_CHARACTER_COUNT);
  Assert(gCharactersList[bSelectedInfoChar].fValid);

  // set font buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  // draw character info and face
  DrawCharacterInfo(bSelectedInfoChar);

  RenderHandPosItem();

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  RenderIconsForUpperLeftCornerPiece(bSelectedInfoChar);

  // mark all pop ups as dirty
  MarkAllBoxesAsAltered();
}

int32_t GetPathTravelTimeDuringPlotting(struct path *pPath) {
  int32_t iTravelTime = 0;
  WAYPOINT pCurrent;
  WAYPOINT pNext;
  struct GROUP *pGroup;
  uint8_t ubGroupId = 0;
  BOOLEAN fSkipFirstNode = FALSE;

  if ((bSelectedDestChar == -1) && (fPlotForHelicopter == FALSE)) {
    return (0);
  }

  if (fTempPathAlreadyDrawn == FALSE) {
    return (0);
  }

  if (pPath == NULL) {
    return (0);
  }

  pPath = MoveToBeginningOfPathList(pPath);

  if (fPlotForHelicopter == FALSE) {
    // plotting for a character...
    if (Menptr[gCharactersList[bSelectedDestChar].usSolID].bAssignment == VEHICLE) {
      ubGroupId = pVehicleList[Menptr[gCharactersList[bSelectedDestChar].usSolID].iVehicleId]
                      .ubMovementGroup;
      pGroup = GetGroup(ubGroupId);

      if (pGroup == NULL) {
        SetUpMvtGroupForVehicle(&(Menptr[gCharactersList[bSelectedDestChar].usSolID]));

        // get vehicle id
        ubGroupId = pVehicleList[Menptr[gCharactersList[bSelectedDestChar].usSolID].iVehicleId]
                        .ubMovementGroup;
        pGroup = GetGroup(ubGroupId);
      }
    } else if (Menptr[gCharactersList[bSelectedDestChar].usSolID].uiStatusFlags & SOLDIER_VEHICLE) {
      ubGroupId = pVehicleList[Menptr[gCharactersList[bSelectedDestChar].usSolID].bVehicleID]
                      .ubMovementGroup;
      pGroup = GetGroup(ubGroupId);

      if (pGroup == NULL) {
        SetUpMvtGroupForVehicle(&(Menptr[gCharactersList[bSelectedDestChar].usSolID]));

        // get vehicle id
        ubGroupId = pVehicleList[Menptr[gCharactersList[bSelectedDestChar].usSolID].bVehicleID]
                        .ubMovementGroup;
        pGroup = GetGroup(ubGroupId);
      }
    } else {
      ubGroupId = Menptr[gCharactersList[bSelectedDestChar].usSolID].ubGroupID;
      pGroup = GetGroup((uint8_t)(ubGroupId));
    }
  } else {
    ubGroupId = pVehicleList[iHelicopterVehicleId].ubMovementGroup;
    pGroup = GetGroup(ubGroupId);
  }

  Assert(pGroup);

  // if between sectors
  if (pGroup->fBetweenSectors) {
    // arrival time should always be legal!
    Assert(pGroup->uiArrivalTime >= GetWorldTotalMin());

    // start with time to finish arriving in any traversal already in progress
    iTravelTime = pGroup->uiArrivalTime - GetWorldTotalMin();
    fSkipFirstNode = TRUE;
  } else {
    iTravelTime = 0;
  }

  while (pPath->pNext) {
    if (!fSkipFirstNode) {
      // grab the current location
      pCurrent.x = (uint8_t)(SectorID16_X(pPath->uiSectorId));
      pCurrent.y = (uint8_t)(SectorID16_Y(pPath->uiSectorId));

      // grab the next location
      pNext.x = (uint8_t)(SectorID16_X(pPath->pNext->uiSectorId));
      pNext.y = (uint8_t)(SectorID16_Y(pPath->pNext->uiSectorId));

      iTravelTime += FindTravelTimeBetweenWaypoints(&pCurrent, &pNext, pGroup);
    } else {
      fSkipFirstNode = FALSE;
    }

    pPath = pPath->pNext;
  }

  return (iTravelTime);
}

void DisplayGroundEta(void) {
  uint32_t iTotalTime = 0;

  if (fPlotForHelicopter == TRUE) {
    return;
  }

  if (bSelectedDestChar == -1) {
    return;
  }

  if (!gCharactersList[bSelectedDestChar].fValid) {
    return;
  }

  iTotalTime = GetGroundTravelTimeOfCharacter(bSelectedDestChar);

  // now display it
  SetFont(ETA_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);
  mprintf(CLOCK_ETA_X, CLOCK_Y_START, pEtaString[0]);

  // if less than one day
  if ((iTotalTime / (60 * 24)) < 1) {
    // show hours and minutes
    SetClockMin(L"%d", iTotalTime % 60);
    SetClockHour(L"%d", iTotalTime / 60);
  } else {
    // show days and hours
    SetHourAlternate(L"%d", (iTotalTime / 60) % 24);
    SetDayAlternate(L"%d", iTotalTime / (60 * 24));
  }
}

void HighLightAssignLine() {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usColor;
  static int32_t iColorNum = STARTING_COLOR_NUM;
  static BOOLEAN fDelta = FALSE;
  static int32_t uiOldHighlight = MAX_CHARACTER_COUNT + 1;
  int16_t usCount = 0;
  uint16_t usX;
  uint16_t usY;

  // is this a valid line?
  if ((giAssignHighLine == -1) || fShowInventoryFlag) {
    uiOldHighlight = MAX_CHARACTER_COUNT + 1;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  // check if we have moved lines, if so, reset
  if (uiOldHighlight != giAssignHighLine) {
    iColorNum = STARTING_COLOR_NUM;
    fDelta = FALSE;

    uiOldHighlight = giAssignHighLine;
  }

  if ((iColorNum == 0) || (iColorNum == 10)) {
    fDelta = !fDelta;
  }
  if (!fDelta)
    iColorNum++;
  else
    iColorNum--;

  // usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
  usY = (Y_OFFSET * giAssignHighLine - 1) + (Y_START + (giAssignHighLine * Y_SIZE));

  if (giAssignHighLine >= FIRST_VEHICLE) {
    usY += 6;
  }

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  for (usCount = 0; usCount < MAX_CHARACTER_COUNT; usCount++) {
    if (IsCharacterSelectedForAssignment(usCount) == TRUE) {
      usX = ASSIGN_X;
      // usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
      usY = (Y_OFFSET * usCount - 1) + (Y_START + (usCount * Y_SIZE));
      if (usCount >= FIRST_VEHICLE) {
        usY += 6;
      }

      usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                       GlowColorsA[iColorNum].ubBlue));

      LineDraw(TRUE, usX, usY, usX, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);
      LineDraw(TRUE, usX + ASSIGN_WIDTH, usY, usX + ASSIGN_WIDTH,
               usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);
      if ((usCount == 0) ||
          (usCount != 0 ? !(IsCharacterSelectedForAssignment((uint16_t)(usCount - 1))) : 0) ||
          (usCount == FIRST_VEHICLE)) {
        LineDraw(TRUE, usX, usY, usX + ASSIGN_WIDTH, usY, usColor, pDestBuf);
      }

      if (((usCount == MAX_CHARACTER_COUNT - 1)) ||
          (usCount != (MAX_CHARACTER_COUNT - 1)
               ? !(IsCharacterSelectedForAssignment((uint16_t)(usCount + 1)))
               : 0) ||
          (usCount == FIRST_VEHICLE - 1)) {
        LineDraw(TRUE, usX, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usX + ASSIGN_WIDTH,
                 usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);
      }

      InvalidateRegion(usX, usY, usX + ASSIGN_WIDTH + 1, usY + GetFontHeight(MAP_SCREEN_FONT) + 3);
    }
  }

  JSurface_Unlock(vsFB);
}

void HighLightDestLine() {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usColor;
  static int32_t iColorNum = STARTING_COLOR_NUM;
  static BOOLEAN fDelta = FALSE;
  static int32_t uiOldHighlight = MAX_CHARACTER_COUNT + 1;
  uint16_t usCount = 0;
  uint16_t usX;
  uint16_t usY;

  if ((giDestHighLine == -1) || fShowInventoryFlag) {
    uiOldHighlight = MAX_CHARACTER_COUNT + 1;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  // check if we have moved lines, if so, reset
  if (uiOldHighlight != giDestHighLine) {
    iColorNum = STARTING_COLOR_NUM;
    fDelta = FALSE;

    uiOldHighlight = giDestHighLine;
  }

  if ((iColorNum == 0) || (iColorNum == 10)) {
    fDelta = !fDelta;
  }
  if (!fDelta)
    iColorNum++;
  else
    iColorNum--;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  for (usCount = 0; usCount < MAX_CHARACTER_COUNT; usCount++) {
    if (CharacterIsGettingPathPlotted(usCount) == TRUE) {
      usX = DEST_ETA_X - 4;
      // usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
      usY = (Y_OFFSET * usCount - 1) + (Y_START + (usCount * Y_SIZE));
      if (usCount >= FIRST_VEHICLE) {
        usY += 6;
      }

      usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                       GlowColorsA[iColorNum].ubBlue));

      if ((usCount == 0) ||
          (usCount != 0 ? !(CharacterIsGettingPathPlotted((uint16_t)(usCount - 1))) : 0) ||
          (usCount == FIRST_VEHICLE)) {
        LineDraw(TRUE, usX + 4, usY, usX + DEST_ETA_WIDTH + 4, usY, usColor, pDestBuf);
      }
      if (((usCount == MAX_CHARACTER_COUNT - 1)) ||
          (usCount != (MAX_CHARACTER_COUNT - 1)
               ? !(CharacterIsGettingPathPlotted((uint16_t)(usCount + 1)))
               : 0) ||
          (usCount == FIRST_VEHICLE - 1)) {
        LineDraw(TRUE, usX + 4, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usX + DEST_ETA_WIDTH + 4,
                 usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);
      }

      LineDraw(TRUE, usX + 4, usY, usX + 4, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor,
               pDestBuf);
      LineDraw(TRUE, usX + DEST_ETA_WIDTH + 4, usY, usX + DEST_ETA_WIDTH + 4,
               usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);

      InvalidateRegion(usX, usY, usX + DEST_ETA_WIDTH + 5,
                       usY + GetFontHeight(MAP_SCREEN_FONT) + 3);
    }
  }
  // InvalidateRegion( usX+4, usY, DEST_ETA_WIDTH-10, usY+GetFontHeight(MAP_SCREEN_FONT)+3);
  // InvalidateRegion( usX+10, usY, usX+ASSIGN_WIDTH, usY+GetFontHeight(MAP_SCREEN_FONT)+3);
  JSurface_Unlock(vsFB);
}

void HighLightSleepLine() {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usColor;
  static int32_t iColorNum = STARTING_COLOR_NUM;
  static BOOLEAN fDelta = FALSE;
  static int32_t uiOldHighlight = MAX_CHARACTER_COUNT + 1;
  uint16_t usCount = 0;
  uint16_t usX, usX2;
  uint16_t usY;

  // is this a valid line?
  if ((giSleepHighLine == -1) || fShowInventoryFlag) {
    uiOldHighlight = MAX_CHARACTER_COUNT + 1;
    return;
  }

  // if not ready to change glow phase yet, leave
  if (!gfGlowTimerExpired) return;

  // check if we have moved lines, if so, reset
  if (uiOldHighlight != giSleepHighLine) {
    iColorNum = STARTING_COLOR_NUM;
    fDelta = FALSE;

    uiOldHighlight = giSleepHighLine;
  }

  if ((iColorNum == 0) || (iColorNum == 10)) {
    fDelta = !fDelta;
  }
  if (!fDelta)
    iColorNum++;
  else
    iColorNum--;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  for (usCount = 0; usCount < MAX_CHARACTER_COUNT; usCount++) {
    if (IsCharacterSelectedForSleep(usCount) == TRUE) {
      usX = SLEEP_X - 4;
      usX2 = SLEEP_X + SLEEP_WIDTH;

      // usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
      usY = (Y_OFFSET * usCount - 1) + (Y_START + (usCount * Y_SIZE));
      if (usCount >= FIRST_VEHICLE) {
        usY += 6;
      }

      usColor = rgb32_to_rgb16(FROMRGB(GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen,
                                       GlowColorsA[iColorNum].ubBlue));

      if ((usCount == 0) ||
          (usCount != 0 ? !(IsCharacterSelectedForSleep((uint16_t)(usCount - 1))) : 0) ||
          (usCount == FIRST_VEHICLE)) {
        LineDraw(TRUE, usX + 4, usY, usX2, usY, usColor, pDestBuf);
      }
      if (((usCount == MAX_CHARACTER_COUNT - 1)) ||
          (usCount != (MAX_CHARACTER_COUNT - 1)
               ? !(IsCharacterSelectedForSleep((uint16_t)(usCount + 1)))
               : 0) ||
          (usCount == FIRST_VEHICLE - 1)) {
        LineDraw(TRUE, usX + 4, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usX2,
                 usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);
      }

      LineDraw(TRUE, usX + 4, usY, usX + 4, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor,
               pDestBuf);
      LineDraw(TRUE, usX2, usY, usX2, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, pDestBuf);

      InvalidateRegion(usX, usY, usX2 + 5, usY + GetFontHeight(MAP_SCREEN_FONT) + 3);
    }
  }
  JSurface_Unlock(vsFB);
}

void AddCharacter(struct SOLDIERTYPE *pCharacter) {
  uint16_t usCount = 0;
  uint16_t usVehicleCount = 0, usVehicleLoop = 0;

  // is character valid?
  if (pCharacter == NULL) {
    // not valid, leave
    return;
  }

  // valid character?
  if (pCharacter->bActive == FALSE) {
    return;
  }

  // adding a vehicle?
  if (pCharacter->uiStatusFlags & SOLDIER_VEHICLE) {
    while (usVehicleLoop < MAX_CHARACTER_COUNT) {
      if (gCharactersList[usVehicleLoop].fValid) {
        if (Menptr[usVehicleLoop].uiStatusFlags & SOLDIER_VEHICLE) {
          usVehicleCount++;
        }
      }
      usVehicleLoop++;
    }

    usCount = FIRST_VEHICLE + usVehicleCount;
  } else {
    // go through character list until a blank is reached
    while ((gCharactersList[usCount].fValid) && (usCount < MAX_CHARACTER_COUNT)) {
      usCount++;
    }
  }

  Assert(usCount < MAX_CHARACTER_COUNT);
  if (usCount >= MAX_CHARACTER_COUNT) {
    return;
  }

  // copy over soldier id value
  gCharactersList[usCount].usSolID = (uint16_t)pCharacter->ubID;

  // valid character
  gCharactersList[usCount].fValid = TRUE;

  return;
}

/*
void MoveCharacter(uint16_t uiInitialPosition, uint16_t uiFinalPosition)
{
        if (!gCharactersList[uiInitialPosition].fValid)
                return;
        else
                memcpy(&gCharactersList[uiFinalPosition], &gCharactersList[uiInitialPosition],
sizeof(MapScreenCharacterSt));
}


void SwapCharacters(uint16_t uiInitialPosition, uint16_t uiFinalPosition)
{
        MapScreenCharacterSt pTempChar;
        memcpy(&pTempChar, &gCharactersList[uiInitialPosition], sizeof(MapScreenCharacterSt));
        memcpy(&gCharactersList[uiInitialPosition], &gCharactersList[uiFinalPosition],
sizeof(MapScreenCharacterSt)); memcpy(&gCharactersList[uiFinalPosition], &pTempChar,
sizeof(MapScreenCharacterSt));
}


void RemoveCharacter(uint16_t uiCharPosition)
{
 memset(&gCharactersList[uiCharPosition], 0, sizeof( MapScreenCharacterSt ));
}
*/

void LoadCharacters(void) {
  uint16_t uiCount = 0;
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  pSoldier = MercPtrs[0];

  // fills array with pressence of player controlled characters
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      AddCharacter(pTeamSoldier);
      uiCount++;
    }
  }

  // set info char if no selected
  if (bSelectedInfoChar == -1) {
    if (DialogueActive() == FALSE) {
      ChangeSelectedInfoChar(0, TRUE);
    }
  }

  // check if ANYONE was available
  if (uiCount == 0) {
    // no one to show
    ChangeSelectedInfoChar(-1, TRUE);
    bSelectedDestChar = -1;
    bSelectedAssignChar = -1;
    bSelectedContractChar = -1;
    fPlotForHelicopter = FALSE;
  }
}

void DisplayCharacterList() {
  int16_t sCount = 0;
  uint8_t ubForegroundColor = 0;

  if ((fShowAssignmentMenu == TRUE) && (fTeamPanelDirty == FALSE)) {
    SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
    return;
  }

  // set dest buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);
  SetFont(MAP_SCREEN_FONT);
  SetFontBackground(FONT_BLACK);

  for (sCount = 0; sCount < MAX_CHARACTER_COUNT; sCount++) {
    // skip invalid characters
    if (gCharactersList[sCount].fValid == TRUE) {
      if (sCount == (int16_t)giHighLine) {
        ubForegroundColor = FONT_WHITE;
      }
      // check to see if character is still alive
      else if (Menptr[gCharactersList[sCount].usSolID].bLife == 0) {
        ubForegroundColor = FONT_METALGRAY;
      } else if (CharacterIsGettingPathPlotted(sCount) == TRUE) {
        ubForegroundColor = FONT_LTBLUE;
      }
      // in current sector?
      else if ((Menptr[gCharactersList[sCount].usSolID].sSectorX == sSelMapX) &&
               (Menptr[gCharactersList[sCount].usSolID].sSectorY == sSelMapY) &&
               (Menptr[gCharactersList[sCount].usSolID].bSectorZ == iCurrentMapSectorZ)) {
        // mobile ?
        if ((Menptr[gCharactersList[sCount].usSolID].bAssignment < ON_DUTY) ||
            (Menptr[gCharactersList[sCount].usSolID].bAssignment == VEHICLE))
          ubForegroundColor = FONT_YELLOW;
        else
          ubForegroundColor = FONT_MAP_DKYELLOW;
      } else {
        // not in current sector
        ubForegroundColor = 5;
      }

      SetFontForeground(ubForegroundColor);

      DrawName(Menptr[gCharactersList[sCount].usSolID].name, sCount, MAP_SCREEN_FONT);
      DrawLocation(sCount, sCount, MAP_SCREEN_FONT);
      DrawDestination(sCount, sCount, MAP_SCREEN_FONT);
      DrawAssignment(sCount, sCount, MAP_SCREEN_FONT);
      DrawTimeRemaining(sCount, MAP_SCREEN_FONT, ubForegroundColor);
    }
  }

  HandleDisplayOfSelectedMercArrows();
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  EnableDisableTeamListRegionsAndHelpText();

  // mark all pop ups as dirty, so that any open assigment menus get reblitted over top of the team
  // list
  MarkAllBoxesAsAltered();

  return;
}

// THIS IS STUFF THAT RUNS *ONCE* DURING APPLICATION EXECUTION, AT INITIAL STARTUP
uint32_t MapScreenInit(void) {
  SetUpBadSectorsList();

  // setup message box system
  InitGlobalMessageList();

  // init palettes for big map
  InitializePalettesForMap();

  // set up mapscreen fast help text
  SetUpMapScreenFastHelpText();

  // set up leave list arrays for dismissed mercs
  InitLeaveList();

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\group_confirm.sti"), &guiUpdatePanel));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\group_confirm_tactical.sti"),
                    &guiUpdatePanelTactical));

  return TRUE;
}

uint32_t MapScreenShutdown(void) {
  // free up alloced mapscreen messages
  FreeGlobalMessageList();

  ShutDownPalettesForMap();

  // free memory for leave list arrays for dismissed mercs
  ShutDownLeaveList();

  DeleteVideoObjectFromIndex(guiUpdatePanel);
  DeleteVideoObjectFromIndex(guiUpdatePanelTactical);

  return TRUE;
}

uint32_t MapScreenHandle(void) {
  uint32_t uiNewScreen;
  //	static BOOLEAN fSecondFrame = FALSE;
  int32_t iCounter = 0;

  // DO NOT MOVE THIS FUNCTION CALL!!!
  // This determines if the help screen should be active
  if (ShouldTheHelpScreenComeUp(HelpScreenDetermineWhichMapScreenHelpToShow(), FALSE)) {
    // handle the help screen
    HelpScreenHandler();
    return (MAP_SCREEN);
  }

  // shaded screen, leave
  if (gfLoadPending == 2) {
    gfLoadPending = FALSE;

    // Load Sector
    // BigCheese
    if (!SetCurrentWorldSector(sSelMapX, sSelMapY, (int8_t)iCurrentMapSectorZ)) {
      // Cannot load!
    } else {
      CreateDestroyMapInvButton();
      // define our progress bar
      // CreateProgressBar( 0, 118, 183, 522, 202 );
    }
    return (MAP_SCREEN);
  }

  //	if ( (fInMapMode == FALSE ) && ( fMapExitDueToMessageBox == FALSE ) )
  if (!fInMapMode) {
    gfFirstMapscreenFrame = TRUE;

    InitPreviousPaths();

    // if arrival sector is invalid, reset to A9
    if ((gsMercArriveSectorX < 1) || (gsMercArriveSectorY < 1) || (gsMercArriveSectorX > 16) ||
        (gsMercArriveSectorY > 16)) {
      gsMercArriveSectorX = 9;
      gsMercArriveSectorY = 1;
    }

    gfInConfirmMapMoveMode = FALSE;
    gfInChangeArrivalSectorMode = FALSE;

    fLeavingMapScreen = FALSE;
    fResetTimerForFirstEntryIntoMapScreen = TRUE;
    fFlashAssignDone = FALSE;
    gfEnteringMapScreen = 0;

    guiTacticalInterfaceFlags |= INTERFACE_MAPSCREEN;

    //		fDisabledMapBorder = FALSE;

    // handle the sort buttons
    AddTeamPanelSortButtonsForMapScreen();

    // load bottom graphics
    LoadMapScreenInterfaceBottom();

    MoveToEndOfMapScreenMessageList();

    // if the current time compression mode is something legal in mapscreen, keep it
    if ((giTimeCompressMode >= TIME_COMPRESS_5MINS) &&
        (giTimeCompressMode <= TIME_COMPRESS_60MINS)) {
      // leave the current time compression mode set, but DO stop it
      StopTimeCompression();
    } else {
      // set compressed mode to X0 (which also stops time compression)
      SetGameTimeCompressionLevel(TIME_COMPRESS_X0);
    }

    // disable video overlay for tactical scroll messages
    EnableDisableScrollStringVideoOverlay(FALSE);

    CreateDestroyInsuranceMouseRegionForMercs(TRUE);

    // ATE: Init tactical interface interface ( always to team panel )
    // SetCurrentInterfacePanel( TEAM_PANEL );
    // Do some things to this now that it's initialized
    // MSYS_DisableRegion( &gViewportRegion );
    // MSYS_DisableRegion( &gRadarRegion );
    // Disable all faces
    SetAllAutoFacesInactive();

    if (fPreLoadedMapGraphics == FALSE) {
      // load border graphics
      LoadMapBorderGraphics();

      vsPOPUPTEX = CreateVSurfaceFromFile("INTERFACE\\popupbackground.pcx");
      if (vsPOPUPTEX == NULL) {
        return FALSE;
      }
      JSurface_SetColorKey(vsPOPUPTEX, FROMRGB(0, 0, 0));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\SAM.sti"), &guiSAMICON));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mapcursr.sti"), &guiMAPCURSORS));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Mine_1.sti"), &guiSubLevel1));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Mine_2.sti"), &guiSubLevel2));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Mine_3.sti"), &guiSubLevel3));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\sleepicon.sti"), &guiSleepIcon));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\charinfo.sti"), &guiCHARINFO));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\newgoldpiece3.sti"), &guiCHARLIST));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\boxes.sti"), &guiCHARICONS));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\incross.sti"), &guiCROSS));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mapinv.sti"), &guiMAPINV));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\map_inv_2nd_gun_cover.sti"),
                        &guiMapInvSecondHandBlockout));

      // the upper left corner piece icons
      CHECKF(
          AddVObject(CreateVObjectFromFile("INTERFACE\\top_left_corner_icons.sti"), &guiULICONS));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\map_item.sti"), &guiORTAICON));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\prison.sti"), &guiTIXAICON));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\merc_between_sector_icons.sti"),
                        &guiCHARBETWEENSECTORICONS));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\merc_mvt_green_arrows.sti"),
                        &guiCHARBETWEENSECTORICONSCLOSE));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\GreenArr.sti"), &guiLEVELMARKER));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Helicop.sti"), &guiHelicopterIcon));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\eta_pop_up.sti"), &guiMapBorderEtaPopUp));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\pos2.sti"), &guiMapBorderHeliSectors));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\secondary_gun_hidden.sti"),
                        &guiSecItemHiddenVO));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\selectedchararrow.sti"),
                        &guiSelectedCharArrow));

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mine.sti"), &guiMINEICON));

      AddVObject(CreateVObjectFromFile("INTERFACE\\hilite.sti"), &guiSectorLocatorGraphicID);

      CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\BullsEye.sti"), &guiBULLSEYE));

      HandleLoadOfMapBottomGraphics();

      // load the militia pop up box
      LoadMilitiaPopUpBox();

      // graphic for pool inventory
      LoadInventoryPoolGraphic();

      // Kris:  Added this because I need to blink the icons button.
      AddVObject(CreateVObjectFromFile("INTERFACE\\newemail.sti"), &guiNewMailIcons);
    }

    // create buttons
    CreateButtonsForMapBorder();

    // create mouse regions for level markers
    CreateMouseRegionsForLevelMarkers();

    // change selected sector/level if necessary
    // NOTE: Must come after border buttons are created, since it may toggle them!
    if (AnyMercsHired() == FALSE) {
      // select starting sector (A9 - Omerta)
      ChangeSelectedMapSector(9, 1, 0);
    } else if ((gWorldSectorX > 0) && (gWorldSectorY > 0) && (gbWorldSectorZ != -1)) {
      // select currently loaded sector as the map sector
      ChangeSelectedMapSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
    } else  // no loaded sector
    {
      // only select A9 - Omerta IF there is no current selection, otherwise leave it as is
      if ((sSelMapX == 0) || (sSelMapY == 0) || (iCurrentMapSectorZ == -1)) {
        ChangeSelectedMapSector(9, 1, 0);
      }
    }

    CHECKF(
        AddVObject(CreateVObjectFromFile("INTERFACE\\Bars.sti"), &guiBrownBackgroundForTeamPanel));

    // we are in fact in the map, do not repeat this sequence
    fInMapMode = TRUE;

    // dirty map
    MarkForRedrawalStrategicMap();

    // dirty team region
    fTeamPanelDirty = TRUE;

    // dirty info region
    fCharacterInfoPanelDirty = TRUE;

    // direty map bottom region
    fMapScreenBottomDirty = TRUE;

    // tactical scroll of messages not allowed to beep until new message is added in tactical
    fOkToBeepNewMessage = FALSE;

    // not in laptop, not about to go there either
    fLapTop = FALSE;

    // reset show aircraft flag
    // fShowAircraftFlag = FALSE;

    // reset fact we are showing white bounding box around face
    fShowFaceHightLight = FALSE;
    fShowItemHighLight = FALSE;

    // reset all selected character flags
    ResetAllSelectedCharacterModes();

    if (fFirstTimeInMapScreen == TRUE) {
      fFirstTimeInMapScreen = FALSE;
      //			fShowMapScreenHelpText = TRUE;
    }

    fShowMapInventoryPool = FALSE;

    // init character list - set all values in the list to 0
    InitalizeVehicleAndCharacterList();

    // deselect all entries
    ResetSelectedListForMapScreen();

    LoadCharacters();

    // set up regions
    MSYS_DefineRegion(&gMapViewRegion, MAP_VIEW_START_X + MAP_GRID_X, MAP_VIEW_START_Y + MAP_GRID_Y,
                      MAP_VIEW_START_X + MAP_VIEW_WIDTH + MAP_GRID_X - 1,
                      MAP_VIEW_START_Y + MAP_VIEW_HEIGHT - 1 + 8, MSYS_PRIORITY_HIGH - 3,
                      MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
    MSYS_DefineRegion(&gCharInfoHandRegion, ((int16_t)(4)), ((int16_t)(81)), ((int16_t)(62)),
                      ((int16_t)(103)), MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, ItemRegionMvtCallback,
                      ItemRegionBtnCallback);
    MSYS_DefineRegion(&gCharInfoFaceRegion, (int16_t)PLAYER_INFO_FACE_START_X,
                      (int16_t)PLAYER_INFO_FACE_START_Y, (int16_t)PLAYER_INFO_FACE_END_X,
                      (int16_t)PLAYER_INFO_FACE_END_Y, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR,
                      MSYS_NO_CALLBACK, FaceRegionBtnCallback);

    MSYS_DefineRegion(&gMPanelRegion, INV_REGION_X, INV_REGION_Y, INV_REGION_X + INV_REGION_WIDTH,
                      INV_REGION_Y + INV_REGION_HEIGHT, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR,
                      MSYS_NO_CALLBACK, InvmaskRegionBtnCallBack);
    // screen mask for animated cursors
    MSYS_DefineRegion(&gMapScreenMaskRegion, 0, 0, 640, 480, MSYS_PRIORITY_LOW, CURSOR_NORMAL,
                      MSYS_NO_CALLBACK, MapScreenMarkRegionBtnCallback);

    // set help text for item glow region
    SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[0]);

    // init the timer menus
    InitTimersForMoveMenuMouseRegions();

    giMapContractButtonImage = LoadButtonImage("INTERFACE\\contractbutton.sti", -1, 0, -1, 1, -1);

    // buttonmake
    giMapContractButton = QuickCreateButton(
        giMapContractButtonImage, CONTRACT_X + 5, CONTRACT_Y - 1, BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGHEST - 5, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
        (GUI_CALLBACK)ContractButtonCallback);

    SpecifyButtonText(giMapContractButton, pContractButtonString[0]);
    SpecifyButtonFont(giMapContractButton, MAP_SCREEN_FONT);
    SpecifyButtonUpTextColors(giMapContractButton, CHAR_TEXT_FONT_COLOR, FONT_BLACK);
    SpecifyButtonDownTextColors(giMapContractButton, CHAR_TEXT_FONT_COLOR, FONT_BLACK);

    // create mouse region for pause clock
    CreateMouseRegionForPauseOfClock(CLOCK_REGION_START_X, CLOCK_REGION_START_Y);

    // create mouse regions
    CreateMouseRegionsForTeamList();

    ReBuildCharactersList();

    // create status bar region
    CreateMapStatusBarsRegion();

    // Add region
    MSYS_AddRegion(&gMapViewRegion);
    MSYS_AddRegion(&gCharInfoFaceRegion);
    MSYS_AddRegion(&gMPanelRegion);

    if (!gfFadeOutDone && !gfFadeIn) {
      MSYS_SetCurrentCursor(SCREEN_CURSOR);
    }
    MSYS_DisableRegion(&gMPanelRegion);

    // create contract box
    CreateContractBox(NULL);

    // create the permanent boxes for assignment and its submenus
    fShowAssignmentMenu = TRUE;
    CreateDestroyAssignmentPopUpBoxes();
    fShowAssignmentMenu = FALSE;

    // create merc remove box
    CreateMercRemoveAssignBox();

    // test message
    // TestMessageSystem( );

    // fill in
    ColorFillVSurfaceArea(vsSaveBuffer, 0, 0, 640, 480, rgb32_to_rgb16(RGB_NEAR_BLACK));
    ColorFillVSurfaceArea(vsFB, 0, 0, 640, 480, rgb32_to_rgb16(RGB_NEAR_BLACK));

    if ((fFirstTimeInMapScreen == TRUE) && (AnyMercsHired() == FALSE)) {
      // render both panels for the restore
      RenderMapRegionBackground();
      RenderTeamRegionBackground();

      // now do the warning box
      DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapErrorString[4], MAP_SCREEN, MSG_BOX_FLAG_OK,
                      MapScreenDefaultOkBoxCallback);
    }

    fFirstTimeInMapScreen = FALSE;

    if (gpCurrentTalkingFace != NULL) {
      // GO FROM GAMESCREEN TO MAPSCREEN
      // REMOVE OLD UI
      // Set face inactive!
      // gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
      // SetAutoFaceInActive( gpCurrentTalkingFace->iID );
      // gfFacePanelActive = FALSE;

      // make him continue talking
      ContinueDialogue(MercPtrs[gpCurrentTalkingFace->ubSoldierID], FALSE);

      // reset diabled flag
      // gpCurrentTalkingFace->fDisabled = FALSE;

      // Continue his talking!
    }

    fOneFrame = FALSE;

    if (fEnterMapDueToContract == TRUE) {
      if (pContractReHireSoldier) {
        FindAndSetThisContractSoldier(pContractReHireSoldier);
      }
      fEnterMapDueToContract = FALSE;
    }
  }

  // if not going anywhere else
  if (guiPendingScreen == NO_PENDING_SCREEN) {
    if (HandleFadeOutCallback()) {
      // force mapscreen to be reinitialized even though we're already in it
      EndMapScreen(TRUE);
      return (MAP_SCREEN);
    }

    if (HandleBeginFadeOut(MAP_SCREEN)) {
      return (MAP_SCREEN);
    }
  }

  // check to see if we need to rebuild the characterlist for map screen
  HandleRebuildingOfMapScreenCharacterList();

  HandleStrategicTurn();

  /*
          // update cursor based on state
          if( ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) && (
     gfInChangeArrivalSectorMode == FALSE ) )
          {
                  // reset cursor
      if ( !gfFadeIn )
      {
                    ChangeMapScreenMaskCursor( CURSOR_NORMAL );
      }
          }
  */

  // check if we are going to create or destroy map border graphics?
  CreateDestroyMapInventoryPoolButtons(FALSE);

  // set up buttons for mapscreen scroll
  //	HandleMapScrollButtonStates( );

  // don't process any input until we've been through here once
  if (gfFirstMapscreenFrame == FALSE) {
    // Handle Interface
    uiNewScreen = HandleMapUI();
    if (uiNewScreen != MAP_SCREEN) {
      return (MAP_SCREEN);
    }
  }

  // handle flashing of contract column for any mercs leaving very soon
  HandleContractTimeFlashForMercThatIsAboutLeave();

  if ((fShownAssignmentMenu == FALSE) && (fShowAssignmentMenu == TRUE)) {
    // need a one frame pause
    fShownAssignmentMenu = fShowAssignmentMenu;
    fShowAssignmentMenu = FALSE;
    fOneFrame = TRUE;
  } else if ((fShownContractMenu == FALSE) && (fShowContractMenu == TRUE)) {
    fShownContractMenu = fShowContractMenu;
    fShowContractMenu = FALSE;
    fOneFrame = TRUE;
  } else if (fOneFrame) {
    // one frame passed
    fShowContractMenu = fShownContractMenu;
    fShowAssignmentMenu = fShownAssignmentMenu;
    fOneFrame = FALSE;
  }

  if ((fShownAssignmentMenu == FALSE) && (fShowAssignmentMenu == FALSE)) {
    bSelectedAssignChar = -1;
  }

  HandlePostAutoresolveMessages();

  //	UpdateLevelButtonStates( );

  // NOTE: This must happen *before* UpdateTheStateOfTheNextPrevMapScreenCharacterButtons()
  CreateDestroyMapCharacterScrollButtons();

  // update the prev next merc buttons
  UpdateTheStateOfTheNextPrevMapScreenCharacterButtons();

  // handle for inventory
  HandleCursorOverRifleAmmo();

  // check contract times, update screen if they do change
  CheckAndUpdateBasedOnContractTimes();

  // handle flashing of assignment strings when merc's assignment is done
  HandleAssignmentsDoneAndAwaitingFurtherOrders();

  // handle timing for the various glowing higlights
  HandleCommonGlowTimer();

  // are we attempting to plot a foot/vehicle path during aircraft mode..if so, stop it
  CheckIfPlottingForCharacterWhileAirCraft();

  // check to see if helicopter is available
  // CheckIfHelicopterAvailable( );
  if (fShowMapInventoryPool) {
    HandleFlashForHighLightedItem();
  }

  //	CreateDestroyMovementBox( 0,0,0 );

  // Deque all game events
  DequeAllGameEvents(TRUE);

  // Handle Interface Stuff
  SetUpInterface();

  // reset time compress has occured
  ResetTimeCompressHasOccured();

  // handle change in info char
  HandleChangeOfInfoChar();

  // update status of contract box
  UpDateStatusOfContractBox();

  // error check of assignments
  UpdateBadAssignments();

  // if cursor has left map..will need to update temp path plotting and cursor
  CheckToSeeIfMouseHasLeftMapRegionDuringPathPlotting();

  // update assignment menus and submenus
  HandleShadingOfLinesForAssignmentMenus();

  // check which menus can be shown right now
  DetermineWhichAssignmentMenusCanBeShown();

  // determine if contract menu can be shown
  DetermineIfContractMenuCanBeShown();

  // if pre battle and inventory up?..get rid of inventory
  HandlePreBattleInterfaceWithInventoryPanelUp();

  // create destroy trash can region
  CreateDestroyTrashCanRegion();

  // update these buttons
  UpdateStatusOfMapSortButtons();

  // if in inventory mode, make sure it's still ok
  CheckForInventoryModeCancellation();

  // restore background rects
  RestoreBackgroundRects();

  InterruptTimeForMenus();

  // place down background
  BlitBackgroundToSaveBuffer();

  if (fLeavingMapScreen == TRUE) {
    return (MAP_SCREEN);
  }

  if (fDisableDueToBattleRoster == FALSE) {
    /*
                    // ATE: OK mark is rendering the item every frame - which isn't good
        // however, don't want to break the world here..
        // this line was added so that when the ItemGlow() is on,
        // we're not rendering also, else glow looks bad
        if ( !fShowItemHighLight )
        {
                      RenderHandPosItem();
        }
    */

    if (fDrawCharacterList) {
      if (!fShowInventoryFlag) {
        // if we are not in inventory mode, show character list
        HandleHighLightingOfLinesInTeamPanel();

        DisplayCharacterList();
      }

      fDrawCharacterList = FALSE;
    }
  }

  if (!fShowMapInventoryPool && !gfPauseDueToPlayerGamePause &&
      !IsMapScreenHelpTextUp() /* && !fDisabledMapBorder */) {
    RenderMapCursorsIndexesAnims();
  }

  if (fDisableDueToBattleRoster == FALSE) {
    // render status bar
    HandleCharBarRender();
  }

  if (fShowInventoryFlag || fDisableDueToBattleRoster) {
    for (iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++) {
      UnMarkButtonDirty(giMapSortButton[iCounter]);
    }
  }

  if (fShowContractMenu || fDisableDueToBattleRoster) {
    UnMarkButtonDirty(giMapContractButton);
  }

  // handle any old messages
  ScrollString();

  HandleSpontanousTalking();

  if ((fDisableDueToBattleRoster == FALSE)) {
    // remove the move box once user leaves it
    CreateDestroyMovementBox(0, 0, 0);

    // this updates the move box contents when changes took place
    ReBuildMoveBox();
  }

  if ((fDisableDueToBattleRoster == FALSE) &&
      ((fShowAssignmentMenu == TRUE) || (fShowContractMenu == TRUE))) {
    // highlight lines?
    HandleHighLightingOfLinesInTeamPanel();

    // render glow for contract region
    ContractBoxGlow();
    GlowTrashCan();

    // handle changing of highlighted lines
    HandleChangeOfHighLightedLine();
  }

  if (fDisableDueToBattleRoster == FALSE) {
    // render face of current info char, for animation
    DrawFace(bSelectedInfoChar);

    // handle autofaces
    HandleAutoFaces();
    HandleTalkingAutoFaces();
    /*
                    GlowFace( );
                    GlowItem( );
    */
  }

  // automatically turns off mapscreen ui overlay messages when appropriate
  MonitorMapUIMessage();

  // if heli is around, show it
  if (fHelicopterAvailable && fShowAircraftFlag && (iCurrentMapSectorZ == 0) &&
      !fShowMapInventoryPool) {
    // this is done on EVERY frame, I guess it beats setting entire map dirty all the time while
    // he's moving...
    DisplayPositionOfHelicopter();
  }

  // display town info
  DisplayTownInfo(sSelMapX, sSelMapY, (int8_t)iCurrentMapSectorZ);

  if (fShowTownInfo == TRUE) {
    // force update of town mine info boxes
    ForceUpDateOfBox(ghTownMineBox);
    MapscreenMarkButtonsDirty();
  }

  // update town mine pop up display
  UpdateTownMinePopUpDisplay();

  if (fShowAttributeMenu) {
    // mark all popups as dirty
    MarkAllBoxesAsAltered();
  }

  // if plotting path
  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    // plot out paths
    PlotPermanentPaths();
    PlotTemporaryPaths();

    // show ETA
    RenderMapBorderEtaPopUp();
    DisplayGroundEta();

    // DisplayDestinationOfCurrentDestMerc( );
  }

  HandleContractRenewalSequence();

  // handle dialog
  HandleDialogue();

  // now the border corner piece
  //	RenderMapBorderCorner( );

  // handle display of inventory pop up
  HandleDisplayOfItemPopUpForSector(9, 1, 0);

  // Display Framerate
  DisplayFrameRate();

  // update paused states
  UpdatePausedStatesDueToTimeCompression();

  // is there a description to be displayed?
  RenderItemDescriptionBox();

  // render clock
  RenderClock(CLOCK_X, CLOCK_Y + 1);

#ifdef JA2TESTVERSION
  if (!gfWorldLoaded) {
    SetFont(FONT10ARIAL);
    if (GetJA2Clock() % 1000 < 500)
      SetFontForeground(FONT_DKRED);
    else
      SetFontForeground(FONT_RED);
    mprintf(530, 2, L"TESTVERSION MSG");
    if (GetJA2Clock() % 1000 < 500)
      SetFontForeground(FONT_DKYELLOW);
    else
      SetFontForeground(FONT_YELLOW);
    mprintf(530, 12, L"NO WORLD LOADED");
    InvalidateRegion(530, 2, 640, 23);
  }
#endif

  if (fEndShowInventoryFlag == TRUE) {
    if (InKeyRingPopup() == TRUE) {
      DeleteKeyRingPopup();
    } else {
      fShowInventoryFlag = FALSE;
      // set help text for item glow region
      SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[0]);
    }

    fTeamPanelDirty = TRUE;
    fEndShowInventoryFlag = FALSE;
  }

  // handle animated cursor update
  if (!gfFadeIn) {
    HandleAnimatedCursorsForMapScreen();
  }

  // if inventory is being manipulated, update cursor
  HandleMapInventoryCursor();

  if (fShowDescriptionFlag == TRUE) {
    // unmark done button
    if (gpItemDescObject->usItem == MONEY) {
      MapscreenMarkButtonsDirty();
    }

    if (Item[gpItemDescObject->usItem].usItemClass & IC_GUN) {
      MapscreenMarkButtonsDirty();
    }

    UnMarkButtonDirty(giMapInvDoneButton);
    // UnMarkButtonDirty( giCharInfoButton[ 0 ] );
    // UnMarkButtonDirty( giCharInfoButton[ 1 ] );
    MarkAButtonDirty(giMapInvDescButton);
  } else {
    if (fShowInventoryFlag == TRUE) {
      MarkAButtonDirty(giMapInvDoneButton);
      MarkAButtonDirty(giCharInfoButton[1]);
      MarkAButtonDirty(giCharInfoButton[0]);
    }
  }

  DrawMilitiaPopUpBox();

  if (fDisableDueToBattleRoster == FALSE) {
    CreateDestroyTheUpdateBox();
    DisplaySoldierUpdateBox();
  }

  // pop up display boxes
  DisplayBoxes(vsFB);

  // render buttons
  RenderButtons();

  if (fShowMapScreenMovementList) {
    // redisplay Movement box to blit it over any border buttons, since if long enough it can
    // overlap them
    ForceUpDateOfBox(ghMoveBox);
    DisplayOnePopupBox(ghMoveBox, vsFB);
  }

  if (fShowContractMenu) {
    // redisplay Contract box to blit it over any map sort buttons, since they overlap
    ForceUpDateOfBox(ghContractBox);
    DisplayOnePopupBox(ghContractBox, vsFB);
  }

  // If we have new email, blink the email icon on top of the laptop button.
  CheckForAndRenderNewMailOverlay();

  // handle video overlays
  ExecuteVideoOverlays();

  if (InItemStackPopup()) {
    RenderItemStackPopup(FALSE);
  }

  if (InKeyRingPopup()) {
    RenderKeyRingPopup(FALSE);
  }

  CheckForMeanwhileOKStart();

  // save background rects
  // ATE: DO this BEFORE rendering help text....
  SaveBackgroundRects();

  if ((fDisableDueToBattleRoster == FALSE) && (fShowAssignmentMenu == FALSE) &&
      (fShowContractMenu == FALSE)) {
    // highlight lines?
    HandleHighLightingOfLinesInTeamPanel();

    // render glow for contract region
    ContractBoxGlow();
    GlowTrashCan();

    // handle changing of highlighted lines
    HandleChangeOfHighLightedLine();

    GlowFace();
    GlowItem();
  }

  if (fShowMapScreenHelpText) {
    // display map screen fast help
    DisplayMapScreenFastHelpList();
  } else {
    // render help
    RenderButtonsFastHelp();
  }

  // execute dirty
  ExecuteBaseDirtyRectQueue();

  // update cursor
  UpdateCursorIfInLastSector();

  // about to leave for new map
  if (gfLoadPending == 1) {
    gfLoadPending++;

    // Shade this frame!
    // Remove cursor
    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

    // Shadow area
    ShadowVideoSurfaceRect(vsFB, 0, 0, 640, 480);
    InvalidateScreen();
  }

  // InvalidateRegion( 0,0, 640, 480);
  EndFrameBufferRender();

  // if not going anywhere else
  if (guiPendingScreen == NO_PENDING_SCREEN) {
    if (HandleFadeInCallback()) {
      // force mapscreen to be reinitialized even though we're already in it
      EndMapScreen(TRUE);
    }

    if (HandleBeginFadeIn(MAP_SCREEN)) {
    }
  }

  HandlePreBattleInterfaceStates();

  if (gfHotKeyEnterSector) {
    gfHotKeyEnterSector = FALSE;
    ActivatePreBattleEnterSectorAction();
  }

#ifdef JA2BETAVERSION
  DebugValidateSoldierData();
#endif

  if (gfRequestGiveSkyriderNewDestination) {
    RequestGiveSkyriderNewDestination();
    gfRequestGiveSkyriderNewDestination = FALSE;
  }

  if (gfFirstMapscreenFrame) {
    //		fSecondFrame = TRUE;
    gfFirstMapscreenFrame = FALSE;
  } else {
    // handle exiting from mapscreen due to both exit button clicks and keyboard equivalents
    HandleExitsFromMapScreen();
  }

  return (MAP_SCREEN);
}

void DrawString(wchar_t *pString, uint16_t uiX, uint16_t uiY, uint32_t uiFont) {
  // draw monochrome string
  SetFont(uiFont);
  gprintfdirty(uiX, uiY, pString);
  mprintf(uiX, uiY, pString);
}

void SetDayAlternate(wchar_t *pStringA, ...) {
  // this sets the clock counter, unwind loop
  uint16_t uiX = 0;
  uint16_t uiY = 0;
  wchar_t String[80];
  va_list argptr;

  va_start(argptr, pStringA);                             // Set up variable argument pointer
  vswprintf(String, ARR_SIZE(String), pStringA, argptr);  // process gprintf string (get output str)
  va_end(argptr);

  if (String[1] == 0) {
    String[1] = String[0];
    String[0] = L' ';
  }
  String[2] = gsTimeStrings[3][0];
  String[3] = L' ';
  String[4] = 0;

  uiX = CLOCK_HOUR_X_START - 9;
  uiY = CLOCK_Y_START;

  SetFont(ETA_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);

  // RestoreExternBackgroundRect( uiX, uiY, 20 ,GetFontHeight( ETA_FONT ) );
  mprintf(uiX, uiY, String);
}

void SetHourAlternate(wchar_t *pStringA, ...) {
  // this sets the clock counter, unwind loop
  uint16_t uiX = 0;
  uint16_t uiY = 0;
  wchar_t String[80];
  va_list argptr;

  va_start(argptr, pStringA);                             // Set up variable argument pointer
  vswprintf(String, ARR_SIZE(String), pStringA, argptr);  // process gprintf string (get output str)
  va_end(argptr);

  if (String[1] == 0) {
    String[1] = String[0];
    String[0] = L' ';
  }

  String[2] = gsTimeStrings[0][0];
  String[3] = L' ';
  String[4] = 0;
  uiX = CLOCK_MIN_X_START - 5;
  uiY = CLOCK_Y_START;
  DrawString(String, uiX, uiY, ETA_FONT);

  SetFont(ETA_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);

  // RestoreExternBackgroundRect( uiX, uiY, 20 ,GetFontHeight( ETA_FONT ) );
  mprintf(uiX, uiY, String);
}

void SetClockHour(wchar_t *pStringA, ...) {
  // this sets the clock counter, unwind loop
  uint16_t uiX = 0;
  uint16_t uiY = 0;
  wchar_t String[80];
  va_list argptr;

  va_start(argptr, pStringA);                             // Set up variable argument pointer
  vswprintf(String, ARR_SIZE(String), pStringA, argptr);  // process gprintf string (get output str)
  va_end(argptr);
  if (String[1] == 0) {
    String[1] = String[0];
    String[0] = L' ';
  }
  String[2] = gsTimeStrings[0][0];
  String[3] = L' ';
  String[4] = 0;
  uiX = CLOCK_HOUR_X_START - 8;
  uiY = CLOCK_Y_START;

  SetFont(ETA_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);

  // RestoreExternBackgroundRect( uiX, uiY, 20 ,GetFontHeight( ETA_FONT ) );
  mprintf(uiX, uiY, String);
}

void SetClockMin(wchar_t *pStringA, ...) {
  // this sets the clock counter, unwind loop
  wchar_t String[10];
  va_list argptr;

  va_start(argptr, pStringA);                             // Set up variable argument pointer
  vswprintf(String, ARR_SIZE(String), pStringA, argptr);  // process gprintf string (get output str)
  va_end(argptr);

  if (String[1] == 0) {
    String[1] = String[0];
    String[0] = L' ';
  }
  String[2] = gsTimeStrings[1][0];
  String[3] = L' ';
  String[4] = 0;

  SetFont(ETA_FONT);
  SetFontForeground(FONT_LTGREEN);
  SetFontBackground(FONT_BLACK);

  // RestoreExternBackgroundRect( CLOCK_MIN_X_START - 5, CLOCK_Y_START, 20 ,GetFontHeight( ETA_FONT
  // ) );
  mprintf(CLOCK_MIN_X_START - 5, CLOCK_Y_START, String);
}

void DrawName(wchar_t *pName, int16_t sRowIndex, int32_t iFont) {
  int16_t usX = 0;
  int16_t usY = 0;

  if (sRowIndex < FIRST_VEHICLE) {
    FindFontCenterCoordinates((short)NAME_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE)),
                              (short)NAME_WIDTH, (short)Y_SIZE, pName, (long)iFont, &usX, &usY);
  } else {
    FindFontCenterCoordinates((short)NAME_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE) + 6),
                              (short)NAME_WIDTH, (short)Y_SIZE, pName, (long)iFont, &usX, &usY);
  }

  // RestoreExternBackgroundRect(NAME_X, ((uint16_t)(usY+(Y_OFFSET*sRowIndex+1))), NAME_WIDTH,
  // Y_SIZE);
  DrawString(pName, (uint16_t)usX, ((uint16_t)(usY + (Y_OFFSET * sRowIndex + 1))), iFont);
}

void DrawAssignment(int16_t sCharNumber, int16_t sRowIndex, int32_t iFont) {
  int16_t usX = 0;
  int16_t usY = 0;
  wchar_t sString[32];

  GetMapscreenMercAssignmentString(MercPtrs[gCharactersList[sCharNumber].usSolID], sString);

  if (sRowIndex < FIRST_VEHICLE) {
    FindFontCenterCoordinates((short)ASSIGN_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE)),
                              (short)ASSIGN_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
  } else {
    FindFontCenterCoordinates((short)ASSIGN_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE) + 6),
                              (short)ASSIGN_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
  }

  if (fFlashAssignDone == TRUE) {
    if (Menptr[gCharactersList[sCharNumber].usSolID].fDoneAssignmentAndNothingToDoFlag) {
      SetFontForeground(FONT_RED);
    }
  }

  // RestoreExternBackgroundRect(ASSIGN_X-2, ((uint16_t)(usY+(Y_OFFSET*sRowIndex+1))),
  // ASSIGN_WIDTH+2, Y_SIZE);
  DrawString(sString, (uint16_t)usX, ((uint16_t)(usY + (Y_OFFSET * sRowIndex + 1))), iFont);
}

void DrawLocation(int16_t sCharNumber, int16_t sRowIndex, int32_t iFont) {
  int16_t usX = 0;
  int16_t usY = 0;
  wchar_t sString[32];

  GetMapscreenMercLocationString(MercPtrs[gCharactersList[sCharNumber].usSolID], sString,
                                 ARR_SIZE(sString));

  if (sRowIndex < FIRST_VEHICLE) {
    // center
    FindFontCenterCoordinates((short)LOC_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE)),
                              (short)LOC_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
  } else {
    FindFontCenterCoordinates((short)LOC_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE) + 6),
                              (short)LOC_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
  }
  // restore background
  // RestoreExternBackgroundRect(LOC_X, ((uint16_t)(usY+(Y_OFFSET*sRowIndex+1))), LOC_WIDTH,
  // Y_SIZE);

  // draw string
  DrawString(sString, ((uint16_t)(usX)), ((uint16_t)(usY + (Y_OFFSET * sRowIndex + 1))),
             ((uint32_t)iFont));
}

void DrawDestination(int16_t sCharNumber, int16_t sRowIndex, int32_t iFont) {
  int16_t usX = 0;
  int16_t usY = 0;
  wchar_t sString[32];

  GetMapscreenMercDestinationString(MercPtrs[gCharactersList[sCharNumber].usSolID], sString,
                                    ARR_SIZE(sString));

  if (wcslen(sString) == 0) {
    return;
  }

  if (sRowIndex < FIRST_VEHICLE) {
    FindFontCenterCoordinates((short)DEST_ETA_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE)),
                              (short)DEST_ETA_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX,
                              &usY);
  } else {
    FindFontCenterCoordinates((short)DEST_ETA_X + 1, (short)(Y_START + (sRowIndex * Y_SIZE) + 6),
                              (short)DEST_ETA_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX,
                              &usY);
  }

  // RestoreExternBackgroundRect(DEST_ETA_X+1, ((uint16_t)(usY+(Y_OFFSET*sRowIndex+1))),
  // DEST_ETA_WIDTH-1, Y_SIZE);
  // ShowDestinationOfPlottedPath( sString );
  DrawString(sString, ((uint16_t)(usX)), ((uint16_t)(usY + (Y_OFFSET * sRowIndex + 1))),
             ((uint32_t)iFont));
}

void DrawTimeRemaining(int16_t sCharNumber, int32_t iFont, uint8_t ubFontColor) {
  int16_t usX = 0;
  int16_t usY = 0;
  wchar_t sString[32];

  GetMapscreenMercDepartureString(MercPtrs[gCharactersList[sCharNumber].usSolID], sString,
                                  ARR_SIZE(sString), &ubFontColor);

  // if merc is highlighted, override the color decided above with bright white
  if (sCharNumber == (int16_t)giHighLine) {
    ubFontColor = FONT_WHITE;
  }

  SetFont(iFont);
  SetFontForeground(ubFontColor);

  if (sCharNumber < FIRST_VEHICLE) {
    FindFontCenterCoordinates(
        (short)TIME_REMAINING_X + 1, (short)(Y_START + (sCharNumber * Y_SIZE)),
        (short)TIME_REMAINING_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
  } else {
    FindFontCenterCoordinates(
        (short)TIME_REMAINING_X + 1, (short)(Y_START + (sCharNumber * Y_SIZE) + 6),
        (short)TIME_REMAINING_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
  }

  // RestoreExternBackgroundRect(TIME_REMAINING_X, ((uint16_t)(usY+(Y_OFFSET*sCharNumber+1))),
  // TIME_REMAINING_WIDTH, Y_SIZE);
  DrawString(sString, ((uint16_t)(usX)), ((uint16_t)(usY + (Y_OFFSET * sCharNumber + 1))),
             ((uint32_t)iFont));
}

void RenderMapCursorsIndexesAnims() {
  BOOLEAN fSelectedSectorHighlighted = FALSE;
  BOOLEAN fSelectedCursorIsYellow = TRUE;
  uint16_t usCursorColor;
  uint32_t uiDeltaTime;
  static uint8_t sPrevHighlightedMapX = 0xff, sPrevHighlightedMapY = 0xff;
  static uint8_t sPrevSelectedMapX = 0xff, sPrevSelectedMapY = 0xff;
  static BOOLEAN fFlashCursorIsYellow = FALSE;
  BOOLEAN fDrawCursors;
  BOOLEAN fHighlightChanged = FALSE;

  HandleAnimationOfSectors();

  if (gfBlitBattleSectorLocator) {
    HandleBlitOfSectorLocatorIcon(gubPBSectorX, gubPBSectorY, gubPBSectorZ, LOCATOR_COLOR_RED);
  }

  fDrawCursors = CanDrawSectorCursor();

  // if mouse cursor is over a map sector
  if (fDrawCursors && (GetMouseMapXY(&gsHighlightSectorX, &gsHighlightSectorY))) {
    // handle highlighting of sector pointed at ( WHITE )

    // if we're over a different sector than when we previously blitted this
    if ((gsHighlightSectorX != sPrevHighlightedMapX) ||
        (gsHighlightSectorY != sPrevHighlightedMapY) || gfMapPanelWasRedrawn) {
      if (sPrevHighlightedMapX != 0xff && sPrevHighlightedMapY != 0xff) {
        RestoreMapSectorCursor(sPrevHighlightedMapX, sPrevHighlightedMapY);
      }

      // draw WHITE highlight rectangle
      RenderMapHighlight(gsHighlightSectorX, gsHighlightSectorY, rgb32_to_rgb16(RGB_WHITE), FALSE);

      sPrevHighlightedMapX = gsHighlightSectorX;
      sPrevHighlightedMapY = gsHighlightSectorY;

      fHighlightChanged = TRUE;
    }
  } else {
    // nothing now highlighted
    gsHighlightSectorX = 0xff;
    gsHighlightSectorY = 0xff;

    if (sPrevHighlightedMapX != 0xff && sPrevHighlightedMapY != 0xff) {
      RestoreMapSectorCursor(sPrevHighlightedMapX, sPrevHighlightedMapY);
      fHighlightChanged = TRUE;
    }

    sPrevHighlightedMapX = 0xff;
    sPrevHighlightedMapY = 0xff;
  }

  // handle highlighting of selected sector ( YELLOW ) - don't show it while plotting movement
  if (fDrawCursors && (bSelectedDestChar == -1) && (fPlotForHelicopter == FALSE)) {
    // if mouse cursor is over the currently selected sector
    if ((gsHighlightSectorX == sSelMapX) && (gsHighlightSectorY == sSelMapY)) {
      fSelectedSectorHighlighted = TRUE;

      // do we need to flash the cursor?  get the delta in time
      uiDeltaTime = GetJA2Clock() - guiFlashCursorBaseTime;

      if (uiDeltaTime > 300) {
        guiFlashCursorBaseTime = GetJA2Clock();
        fFlashCursorIsYellow = !fFlashCursorIsYellow;

        fHighlightChanged = TRUE;
      }
    }

    if (!fSelectedSectorHighlighted || fFlashCursorIsYellow) {
      // draw YELLOW highlight rectangle
      usCursorColor = rgb32_to_rgb16(RGB_YELLOW);
    } else {
      // draw WHITE highlight rectangle
      usCursorColor = rgb32_to_rgb16(RGB_WHITE);

      // index letters will also be white instead of yellow so that they flash in synch with the
      // cursor
      fSelectedCursorIsYellow = FALSE;
    }

    // always render this one, it's too much of a pain detecting overlaps with the white cursor
    // otherwise
    RenderMapHighlight(sSelMapX, sSelMapY, usCursorColor, TRUE);

    if ((sPrevSelectedMapX != sSelMapX) || (sPrevSelectedMapY != sSelMapY)) {
      sPrevSelectedMapX = sSelMapX;
      sPrevSelectedMapY = sSelMapY;

      fHighlightChanged = TRUE;
    }
  } else {
    // erase yellow highlight cursor
    if (sPrevSelectedMapX != 0xff && sPrevSelectedMapY != 0xff) {
      RestoreMapSectorCursor(sPrevSelectedMapX, sPrevSelectedMapY);
      fHighlightChanged = TRUE;
    }

    sPrevSelectedMapX = 0xff;
    sPrevSelectedMapY = 0xff;
  }

  if (fHighlightChanged || gfMapPanelWasRedrawn) {
    // redraw sector index letters and numbers
    DrawMapIndexBigMap(fSelectedCursorIsYellow);
  }
}

uint32_t HandleMapUI() {
  uint32_t uiNewEvent = MAP_EVENT_NONE;
  uint8_t sMapX = 0, sMapY = 0;
  uint8_t sX, sY;
  uint32_t uiNewScreen = MAP_SCREEN;
  BOOLEAN fWasAlreadySelected;

  // Get Input from keyboard
  GetMapKeyboardInput(&uiNewEvent);

  CreateDestroyMapInvButton();

  // Get mouse
  PollLeftButtonInMapView(&uiNewEvent);
  PollRightButtonInMapView(&uiNewEvent);

  // Switch on event
  switch (uiNewEvent) {
    case MAP_EVENT_NONE:
      break;

    case MAP_EVENT_PLOT_PATH:
      GetMouseMapXY(&sMapX, &sMapY);

      // plotting for the chopper?
      if (fPlotForHelicopter == TRUE) {
        PlotPathForHelicopter(sMapX, sMapY);
        fTeamPanelDirty = TRUE;
      } else {
        // plot for character

        // check for valid character
        Assert(bSelectedDestChar != -1);
        if (bSelectedDestChar == -1) break;

        // check if last sector in character's path is same as where mouse is
        if (GetLastSectorIdInCharactersPath(GetSoldierByID(
                gCharactersList[bSelectedDestChar].usSolID)) != GetSectorID16(sMapX, sMapY)) {
          sX = (GetLastSectorIdInCharactersPath(
                    GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID)) %
                MAP_WORLD_X);
          sY = (GetLastSectorIdInCharactersPath(
                    GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID)) /
                MAP_WORLD_X);
          RestoreBackgroundForMapGrid(sX, sY);
        }

        if ((IsTheCursorAllowedToHighLightThisSector(sMapX, sMapY) == TRUE) &&
            (SectorInfo[(GetSectorID8(sMapX, sMapY))].ubTraversability[THROUGH_STRATEGIC_MOVE] !=
             GROUNDBARRIER)) {
          // Can we get go there?  (NULL temp character path)
          if (GetLengthOfPath(pTempCharacterPath) > 0) {
            PlotPathForCharacter(GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID), sMapX,
                                 sMapY, FALSE);

            // copy the path to every other selected character
            CopyPathToAllSelectedCharacters(
                GetSoldierMercPathPtr(MercPtrs[gCharactersList[bSelectedDestChar].usSolID]));

            StartConfirmMapMoveMode(sMapY);
            MarkForRedrawalStrategicMap();
            fTeamPanelDirty = TRUE;  // update team panel desinations
          } else {
            // means it's a vehicle and we've clicked an off-road sector
            MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapErrorString[40]);
          }
        }
      }
      break;

    case MAP_EVENT_CANCEL_PATH:
      CancelOrShortenPlottedPath();
      break;

    case MAP_EVENT_CLICK_SECTOR:

      // Get Current mouse position
      if (GetMouseMapXY(&sMapX, &sMapY)) {
        // not zoomed out, make sure this is a valid sector
        if (IsTheCursorAllowedToHighLightThisSector(sMapX, sMapY) == FALSE) {
          // do nothing, return
          return (MAP_SCREEN);
        }

        // while item in hand
        if (fMapInventoryItem) {
          // if not showing item counts on the map
          if (!fShowItemsFlag) {
            // turn that on
            ToggleItemsFilter();
          }

          // if item's owner is known
          if (gpItemPointerSoldier != NULL) {
            // make sure it's the owner's sector that's selected
            if ((gpItemPointerSoldier->sSectorX != sSelMapX) ||
                (gpItemPointerSoldier->sSectorY != sSelMapY) ||
                (gpItemPointerSoldier->bSectorZ != iCurrentMapSectorZ)) {
              ChangeSelectedMapSector((uint8_t)gpItemPointerSoldier->sSectorX,
                                      (uint8_t)gpItemPointerSoldier->sSectorY,
                                      gpItemPointerSoldier->bSectorZ);
            }
          }

          // if not already in sector inventory
          if (!fShowMapInventoryPool) {
            // start it up ( NOTE: for the item OWNER'S sector, regardless of which sector player
            // clicks )
            fShowMapInventoryPool = TRUE;
            CreateDestroyMapInventoryPoolButtons(TRUE);
          }

          return (MAP_SCREEN);
        }

        // don't permit other click handling while item is in cursor (entering PBI would permit item
        // teleports, etc.)
        Assert(!fMapInventoryItem);

        // this doesn't change selected sector
        if (gfInChangeArrivalSectorMode) {
          if (SectorInfo[(GetSectorID8(sMapX, sMapY))].ubTraversability[THROUGH_STRATEGIC_MOVE] !=
              GROUNDBARRIER) {
            // if it's not enemy air controlled
            if (StrategicMap[GetSectorID16(sMapX, sMapY)].fEnemyAirControlled == FALSE) {
              wchar_t sMsgString[128], sMsgSubString[64];

              // move the landing zone over here
              gsMercArriveSectorX = sMapX;
              gsMercArriveSectorY = sMapY;

              // change arrival sector for all mercs currently in transit who are showing up at the
              // landing zone
              UpdateAnyInTransitMercsWithGlobalArrivalSector();

              // we're done, cancel this mode
              CancelChangeArrivalSectorMode();

              // get the name of the sector
              GetSectorIDString(sMapX, sMapY, 0, sMsgSubString, ARR_SIZE(sMsgSubString), FALSE);

              // now build the string
              swprintf(sMsgString, ARR_SIZE(sMsgString), pBullseyeStrings[1], sMsgSubString);

              // confirm the change with overlay message
              MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, sMsgString);

              // update destination column for any mercs in transit
              fTeamPanelDirty = TRUE;
            } else {
              // message: not allowed, don't have airspace secured
              MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE,
                               pBullseyeStrings[2]);
            }
          }

          return (MAP_SCREEN);
        } else  // not already changing arrival sector
        {
          if (CanMoveBullseyeAndClickedOnIt(sMapX, sMapY)) {
            // if the click is ALSO over the helicopter icon
            // NOTE: The helicopter icon is NOT necessarily directly over the helicopter's current
            // sector!!!
            if (CheckForClickOverHelicopterIcon(sMapX, sMapY) == TRUE) {
              CreateBullsEyeOrChopperSelectionPopup();
            } else {
              StartChangeSectorArrivalMode();
            }

            return (MAP_SCREEN);
          }
        }

        // if new map sector was clicked on
        if ((sSelMapX != sMapX) || (sSelMapY != sMapY)) {
          fWasAlreadySelected = FALSE;

          // select the clicked sector, retaining the same sublevel depth
          ChangeSelectedMapSector(sMapX, sMapY, (int8_t)iCurrentMapSectorZ);
        } else {
          fWasAlreadySelected = TRUE;
        }

        // if showing item counts on the map, and not already in sector inventory
        if (fShowItemsFlag && !fShowMapInventoryPool) {
          // show sector inventory for this clicked sector
          ChangeSelectedMapSector(sMapX, sMapY, (int8_t)iCurrentMapSectorZ);

          fShowMapInventoryPool = TRUE;
          CreateDestroyMapInventoryPoolButtons(TRUE);

          return (MAP_SCREEN);
        }

        if (gfBlitBattleSectorLocator && sMapX == gubPBSectorX && sMapY == gubPBSectorY &&
            iCurrentMapSectorZ == gubPBSectorZ) {  // Bring up a non-persistant version of mapscreen
                                                   // if the user clicks on the sector where a
          // battle is taking place.
          InitPreBattleInterface(NULL, FALSE);
          return (MAP_SCREEN);
        }

        // if we're in airspace mode
        if (fShowAircraftFlag == TRUE) {
          // if not moving soldiers, and not yet plotting the helicopter
          if ((bSelectedDestChar == -1) && (fPlotForHelicopter == FALSE)) {
            // if we're on the surface level, and the click is over the helicopter icon
            // NOTE: The helicopter icon is NOT necessarily directly over the helicopter's current
            // sector!!!
            if ((iCurrentMapSectorZ == 0) &&
                CheckForClickOverHelicopterIcon(sMapX, sMapY) == TRUE) {
              RequestGiveSkyriderNewDestination();
              return (MAP_SCREEN);
            }
          }
        } else  // not in airspace mode
        {
          // sector must be already selected to initiate movement plotting!  This is to allow
          // selecting sectors with mercs in them without necessarily initiating movement right
          // away.
          if (fWasAlreadySelected) {
            // if there are any movable characters here
            if (AnyMovableCharsInOrBetweenThisSector(sMapX, sMapY, (int8_t)iCurrentMapSectorZ)) {
              // if showing the surface level map
              if (iCurrentMapSectorZ == 0) {
                TurnOnShowTeamsMode();

                // NOTE: must allow move box to come up, since there may be movable characters
                // between sectors which are unaffected by combat / hostiles / air raid in the
                // sector proper itself!! This also allows all strategic movement error handling to
                // be centralized in CanCharacterMoveInStrategic()

                // start the move box menu
                SetUpMovingListsForSector(sMapX, sMapY, (int8_t)iCurrentMapSectorZ);
              } else {
                // no strategic movement is possible from underground sectors
                DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapErrorString[1], MAP_SCREEN,
                                MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
                return (MAP_SCREEN);
              }
            }
          }
        }
      }
      break;

// Kris -- added hook so I can access AIView in non-release mode.
#ifdef JA2BETAVERSION
    case MAP_EVENT_VIEWAI:
      SetPendingNewScreen(AIVIEWER_SCREEN);
      CreateDestroyMapInvButton();
      break;
#endif
  }

  // if we pressed something that will cause a screen change
  if (guiPendingScreen != NO_PENDING_SCREEN) {
    uiNewScreen = guiPendingScreen;
  }

  return (uiNewScreen);
}

void GetMapKeyboardInput(uint32_t *puiNewEvent) {
  InputAtom InputEvent;
  int8_t bSquadNumber;
  uint8_t ubGroupId = 0;
  BOOLEAN fCtrl, fAlt;
  uint8_t sMapX, sMapY;

  fCtrl = _KeyDown(CTRL);
  fAlt = _KeyDown(ALT);

  while (DequeueEvent(&InputEvent)) {
    struct Point MousePos = GetMousePoint();

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

    if (InputEvent.usEvent == KEY_DOWN) {
      // if game is paused because of player, unpause with any key
      if (gfPauseDueToPlayerGamePause) {
        HandlePlayerPauseUnPauseOfGame();
        continue;
      }

      if (IsMapScreenHelpTextUp()) {
        // stop mapscreen text
        StopMapScreenHelpText();
        continue;
      }

      // handle for fast help text for interface stuff
      if (IsTheInterfaceFastHelpTextActive()) {
        ShutDownUserDefineHelpTextRegions();
      }

      switch (InputEvent.usParam) {
        case ESC:
          gfDontStartTransitionFromLaptop = TRUE;

          if (gfPreBattleInterfaceActive &&
              !gfPersistantPBI) {  // Non persistant PBI.  Allow ESC to close it and return to
                                   // mapscreen.
            KillPreBattleInterface();
            gpBattleGroup = NULL;
            return;
          }

          if (gfInChangeArrivalSectorMode) {
            CancelChangeArrivalSectorMode();
            MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[3]);
          }
          // ESC cancels MAP UI messages, unless we're in confirm map move mode
          else if ((giUIMessageOverlay != -1) && !gfInConfirmMapMoveMode) {
            CancelMapUIMessage();
          } else if (IsMapScreenHelpTextUp()) {
            StopMapScreenHelpText();
          } else if (gpCurrentTalkingFace != NULL && gpCurrentTalkingFace->fTalking) {
            // ATE: We want to stop speech if somebody is talking...
            StopAnyCurrentlyTalkingSpeech();
          } else if (fShowUpdateBox) {
            if (fShowUpdateBox) {
              EndUpdateBox(FALSE);  // stop time compression
            }
          } else if (fShowDescriptionFlag) {
            DeleteItemDescriptionBox();
          }
          // plotting movement?
          else if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
            AbortMovementPlottingMode();
          } else if (fShowAssignmentMenu) {
            // dirty region
            fTeamPanelDirty = TRUE;
            MarkForRedrawalStrategicMap();
            fCharacterInfoPanelDirty = TRUE;

            // stop showing current assignment box
            if (fShowAttributeMenu == TRUE) {
              fShowAttributeMenu = FALSE;
              MarkForRedrawalStrategicMap();
            } else if (fShowTrainingMenu == TRUE) {
              fShowTrainingMenu = FALSE;
            } else if (fShowSquadMenu == TRUE) {
              fShowSquadMenu = FALSE;
            } else if (fShowRepairMenu == TRUE) {
              fShowRepairMenu = FALSE;
            } else {
              fShowAssignmentMenu = FALSE;
            }
            giAssignHighLine = -1;
            // restore background to glow region
            RestoreBackgroundForAssignmentGlowRegionList();
          } else if (fShowContractMenu == TRUE) {
            fShowContractMenu = FALSE;

            // restore contract glow region
            RestoreBackgroundForContractGlowRegionList();
            fTeamPanelDirty = TRUE;
            fCharacterInfoPanelDirty = TRUE;
            giContractHighLine = -1;
          }
          // in militia popup?
          else if ((sSelectedMilitiaTown != 0) && (sGreensOnCursor == 0) &&
                   (sRegularsOnCursor == 0) && (sElitesOnCursor == 0)) {
            sSelectedMilitiaTown = 0;
            MarkForRedrawalStrategicMap();
          } else if (fShowTownInfo == TRUE) {
            fShowTownInfo = FALSE;
            CreateDestroyScreenMaskForAssignmentAndContractMenus();
          } else if (fShowDescriptionFlag) {
            if (gMPanelRegion.Cursor != EXTERN_CURSOR) {
              DeleteItemDescriptionBox();
            }
          } else if (InKeyRingPopup() == TRUE) {
            DeleteKeyRingPopup();
            fTeamPanelDirty = TRUE;
          } else if (fShowInventoryFlag == TRUE) {
            if (gMPanelRegion.Cursor != EXTERN_CURSOR && !InItemStackPopup()) {
              fEndShowInventoryFlag = TRUE;
            }
          } else if (MultipleCharacterListEntriesSelected()) {
            ResetSelectedListForMapScreen();
            if (bSelectedInfoChar != -1) {
              SetEntryInSelectedCharacterList(bSelectedInfoChar);
            }
            fTeamPanelDirty = TRUE;
            fCharacterInfoPanelDirty = TRUE;
          } else {
            RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
          }
          break;  // end of ESC

        case PAUSE:
          // Pause game!
          HandlePlayerPauseUnPauseOfGame();
          break;

        case LEFTARROW:
          // previous character
          GoToPrevCharacterInList();
          break;
        case RIGHTARROW:
          // next character
          GoToNextCharacterInList();
          break;

        case UPARROW:
          // up a line
          MapScreenMsgScrollUp(1);
          break;
        case DNARROW:
          // down a line
          MapScreenMsgScrollDown(1);
          break;

        case PGUP:
          // up a page
          MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);
          break;
        case PGDN:
          // down a page
          MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM);
          break;

        case HOME:
          // jump to top of message list
          ChangeCurrentMapscreenMessageIndex(0);
          break;

        case END:
          // jump to bottom of message list
          MoveToEndOfMapScreenMessageList();
          break;

        case INSERT:
          // up one sublevel
          GoUpOneLevelInMap();
          break;

        case DEL:
          // down one sublevel
          GoDownOneLevelInMap();
          break;

        case ENTER:
          RequestToggleMercInventoryPanel();
          break;

        case BACKSPACE:
          StopAnyCurrentlyTalkingSpeech();
          break;

        case F1:
        case F2:
        case F3:
        case F4:
        case F5:
        case F6:
          ChangeCharacterListSortMethod(InputEvent.usParam - F1);
          break;

        case F7:
#ifdef JA2TESTVERSION
          if (fAlt) {
            if (bSelectedInfoChar != -1) {
              struct SOLDIERTYPE *pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];
              if (pSoldier->inv[HANDPOS].usItem != 0) {
                pSoldier->inv[HANDPOS].bStatus[0] = 2;
              }
            }
          }
          if (fCtrl) {
            if (bSelectedInfoChar != -1) {
              struct SOLDIERTYPE *pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];
              if (pSoldier->inv[HANDPOS].usItem != 0) {
                pSoldier->inv[HANDPOS].usItem = GUN_BARREL_EXTENDER;
              }
            }
          }
#endif
          break;

        case F8:
#ifdef JA2TESTVERSION
          if (fAlt) {
            // reduce balance to $500
            AddTransactionToPlayersBook(PAYMENT_TO_NPC, SKYRIDER, -(MoneyGetBalance() - 500));
          }
#endif
          break;

        case F9:
#ifdef JA2TESTVERSION
          if (fAlt) {
            uint8_t ubSamIndex;

            // ALT-F9: Reveal all SAM sites
            for (ubSamIndex = 0; ubSamIndex < NUMBER_OF_SAM_SITES; ubSamIndex++) {
              SetSAMSiteAsFound(ubSamIndex);
            }
          }
#endif
          break;

        case F10:
#ifdef JA2TESTVERSION
          if (fAlt) {
            if (bSelectedInfoChar != -1) {
              // ALT-F10: force selected character asleep (ignores breathmax)
              PutMercInAsleepState(MercPtrs[gCharactersList[bSelectedInfoChar].usSolID]);
            }
          }
#endif
          break;

          /*
                                          case F11:
                                                  #ifdef JA2TESTVERSION
                                                          if( fAlt )
                                                          {
                                                                  // ALT-F11: make all sectors
             player controlled ClearMapControlledFlags( ); MarkForRedrawalStrategicMap();
                                                          }
                                                  #endif
                                                  break;
          */

        case F12:
#ifdef JA2BETAVERSION
          *puiNewEvent = MAP_EVENT_VIEWAI;
#endif
          break;

        case '+':
        case '=':
          if (CommonTimeCompressionChecks() == FALSE) RequestIncreaseInTimeCompression();
          break;

        case '-':
        case '_':
          if (CommonTimeCompressionChecks() == FALSE) RequestDecreaseInTimeCompression();
          break;

        case SPACE:
          if (fShowUpdateBox) {
            EndUpdateBox(TRUE);  // restart time compression
          } else {
            // toggle time compression
            if (CommonTimeCompressionChecks() == FALSE) RequestToggleTimeCompression();
          }
          break;

        case '`':
#ifdef JA2TESTVERSION
          if (fCtrl) {
            if (bSelectedInfoChar != -1) {
              TownMilitiaTrainingCompleted(
                  GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID), sSelMapX, sSelMapY);
            }
          }
#endif
          break;

        case '\\':
#ifdef JA2TESTVERSION
          if (fCtrl) {
            DumpItemsList();
          }
#endif
          break;

        case '>':
#ifdef JA2TESTVERSION
          if (fCtrl) {
            // free
          }
#endif
          break;

        case '?':
#ifdef JA2TESTVERSION
          if (fCtrl)
            MapScreenMessage(0, MSG_DEBUG, L"JA2Clock = %d", GetJA2Clock());
          else
            MapScreenMessage(0, MSG_DEBUG, L"Mouse X,Y = %d,%d", MSYS_CurrentMX, MSYS_CurrentMY);
#endif
          break;

        case '/':
#ifdef JA2TESTVERSION
          if (fAlt) {
            if (bSelectedInfoChar != -1) {
              StatChange(GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID), EXPERAMT, 1000,
                         FROM_SUCCESS);
            }
          }
#endif
          break;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          // multi-selects all characters in that squad.  SHIFT key and 1-0 for squads 11-20
          bSquadNumber = (int8_t)(InputEvent.usParam - '1');  // internal squad #s start at 0
          SelectAllCharactersInSquad(bSquadNumber);
          break;

        case '0':
          SelectAllCharactersInSquad(9);  // internal squad #s start at 0
          break;

        case '!':
          SelectAllCharactersInSquad(10);  // internal squad #s start at 0
          break;
        case '@':
          SelectAllCharactersInSquad(11);  // internal squad #s start at 0
          break;
        case '#':
          SelectAllCharactersInSquad(12);  // internal squad #s start at 0
          break;
        case '$':
          SelectAllCharactersInSquad(13);  // internal squad #s start at 0
          break;
        case '%':
          SelectAllCharactersInSquad(14);  // internal squad #s start at 0
          break;
        case '^':
          SelectAllCharactersInSquad(15);  // internal squad #s start at 0
          break;
        case '&':
          SelectAllCharactersInSquad(16);  // internal squad #s start at 0
          break;
        case '*':
          SelectAllCharactersInSquad(17);  // internal squad #s start at 0
          break;
        case '(':
          SelectAllCharactersInSquad(18);  // internal squad #s start at 0
          break;
        case ')':
          SelectAllCharactersInSquad(19);  // internal squad #s start at 0
          break;

        case 'a':
          if (fAlt) {
            if (giHighLine != -1) {
              if (gCharactersList[giHighLine].fValid == TRUE) {
                bSelectedAssignChar = (int8_t)giHighLine;
                RebuildAssignmentsBox();
                ChangeSelectedInfoChar((int8_t)giHighLine, FALSE);
                fShowAssignmentMenu = TRUE;
              }
            } else if (bSelectedInfoChar != -1) {
              if (gCharactersList[bSelectedInfoChar].fValid == TRUE) {
                bSelectedAssignChar = (int8_t)bSelectedInfoChar;
                RebuildAssignmentsBox();
                fShowAssignmentMenu = TRUE;
              }
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              if (gfAutoAmbush ^= 1)
                ScreenMsg(FONT_WHITE, MSG_TESTVERSION, L"Enemy ambush test mode enabled.");
              else
                ScreenMsg(FONT_WHITE, MSG_TESTVERSION, L"Enemy ambush test mode disabled.");
            }
          } else {
            if (gfPreBattleInterfaceActive) {
              // activate autoresolve in prebattle interface.
              ActivatePreBattleAutoresolveAction();
            } else {
              // only handle border button keyboard equivalents if the button is visible!
              if (!fShowMapInventoryPool) {
                ToggleAirspaceMode();
              }
            }
          }
          break;

        case 'b':
          break;

        case 'c':
          RequestContractMenu();
          break;

        case 'd':
#ifdef JA2TESTVERSION
          if (fAlt) {
            // prints out a text file in C:\TEMP telling you how many stat change chances/successes
            // each profile merc got
            TestDumpStatChanges();
          }
#endif
          break;

        case 'e':
          if (gfPreBattleInterfaceActive) {  // activate enter sector in prebattle interface.
            gfHotKeyEnterSector = TRUE;
          }
          break;
        case 'f':
#ifdef JA2TESTVERSION
          // CTRL-F: Refuel vehicle
          if ((fCtrl) && (bSelectedInfoChar != -1)) {
            struct SOLDIERTYPE *pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];

            if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
              pSoldier->sBreathRed = 10000;
              pSoldier->bBreath = 100;
              ScreenMsg(FONT_MCOLOR_RED, MSG_TESTVERSION, L"Vehicle refueled");

              fTeamPanelDirty = TRUE;
              fCharacterInfoPanelDirty = TRUE;
            }
          }
#endif
          if (fAlt) {
            if (INFORMATION_CHEAT_LEVEL()) {
              // Toggle Frame Rate Display
              gbFPSDisplay = !gbFPSDisplay;
              DisableFPSOverlay((BOOLEAN)!gbFPSDisplay);
            }
          }
          break;

        case 'h':
#ifdef JA2TESTVERSION
          if (fAlt) {
            // set up the helicopter over Omerta (if it's not already set up)
            SetUpHelicopterForPlayer(9, 1);
            // raise Drassen loyalty to minimum that will allow Skyrider to fly
            if (gTownLoyalty[DRASSEN].fStarted &&
                (gTownLoyalty[DRASSEN].ubRating < LOYALTY_LOW_THRESHOLD)) {
              SetTownLoyalty(DRASSEN, LOYALTY_LOW_THRESHOLD);
            }
            TurnOnAirSpaceMode();
          } else
#endif
          {
            // ARM: Feb01/98 - Cancel out of mapscreen movement plotting if Help subscreen is coming
            // up
            if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
              AbortMovementPlottingMode();
            }

            ShouldTheHelpScreenComeUp(HELP_SCREEN_MAPSCREEN, TRUE);
          }

          //					fShowMapScreenHelpText = TRUE;
          break;

        case 'i':
#ifdef JA2TESTVERSION
          if (fCtrl) {
            fDisableJustForIan = !fDisableJustForIan;
          } else {
            // only handle border button keyboard equivalents if the button is visible!
            if (!fShowMapInventoryPool) {
              ToggleItemsFilter();
            }
          }
#else
          // only handle border button keyboard equivalents if the button is visible!
          if (!fShowMapInventoryPool) {
            ToggleItemsFilter();
          }
#endif

          break;
        case 'l':
          if (fAlt) {
            // although we're not actually going anywhere, we must still be in a state where this is
            // permitted
            if (AllowedToExitFromMapscreenTo(MAP_EXIT_TO_LOAD)) {
              DoQuickLoad();
            }
          } else if (fCtrl) {
            // go to LOAD screen
            gfSaveGame = FALSE;
            RequestTriggerExitFromMapscreen(MAP_EXIT_TO_LOAD);
          } else {
            // go to LAPTOP
            RequestTriggerExitFromMapscreen(MAP_EXIT_TO_LAPTOP);
          }
          break;
        case 'm':
          // only handle border button keyboard equivalents if the button is visible!
          if (!fShowMapInventoryPool) {
            // toggle show mines flag
            ToggleShowMinesMode();
          }

          break;
        case 'n':
#ifdef JA2TESTVERSION
          if (fAlt) {
            static uint16_t gQuoteNum = 0;
            // Get Soldier
            TacticalCharacterDialogue(MercPtrs[gCharactersList[bSelectedInfoChar].usSolID],
                                      gQuoteNum);
            gQuoteNum++;
          } else if (fCtrl) {
            static uint16_t gQuoteNum = 0;
            // Get Soldier
            if (giHighLine != -1) {
              TacticalCharacterDialogue(MercPtrs[gCharactersList[giHighLine].usSolID], gQuoteNum);
              gQuoteNum++;
            }
          }
#endif
          break;
        case 'o':
          if (fAlt) {  // toggle if Orta & Tixa have been found
#ifdef JA2TESTVERSION
            fFoundOrta = !fFoundOrta;
            fFoundTixa = !fFoundTixa;
            MarkForRedrawalStrategicMap();
#endif
          } else {
            // go to OPTIONS screen
            RequestTriggerExitFromMapscreen(MAP_EXIT_TO_OPTIONS);
          }
          break;

        case 'p':
#ifdef JA2TESTVERSION
          if (fCtrl) {
            // CTRL-P: Display player's highest progress percentage
            DumpSectorDifficultyInfo();
          }
#endif

#ifdef JA2TESTVERSION
          // ALT-P: Make the selected character a POW!
          if ((fAlt) && (bSelectedInfoChar != -1)) {
            struct SOLDIERTYPE *pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];

            EnemyCapturesPlayerSoldier(pSoldier);

            if (IsSolInSector(pSoldier)) {
              RemoveSoldierFromTacticalSector(pSoldier, TRUE);
            }

            fTeamPanelDirty = TRUE;
            fCharacterInfoPanelDirty = TRUE;
            MarkForRedrawalStrategicMap();
          }
#endif
          break;

        case 'q':
#ifdef JA2TESTVERSION
          if (fAlt) {
            // initialize miners if not already done so (fakes entering Drassen mine first)
            HandleQuestCodeOnSectorEntry(13, 4, 0);
            // test miner quote system
            IssueHeadMinerQuote((int8_t)(1 + Random(MAX_NUMBER_OF_MINES - 1)),
                                (uint8_t)(1 + Random(2)));
          }
#endif
          break;
        case 'r':
          if (gfPreBattleInterfaceActive) {  // activate autoresolve in prebattle interface.
            ActivatePreBattleRetreatAction();
          }
          break;
        case 's':
          if (fAlt) {
            // although we're not actually going anywhere, we must still be in a state where this is
            // permitted
            if (AllowedToExitFromMapscreenTo(MAP_EXIT_TO_SAVE)) {
              // if the game CAN be saved
              if (CanGameBeSaved()) {
                guiPreviousOptionScreen = guiCurrentScreen;
                DoQuickSave();
              } else {
                // Display a message saying the player cant save now
                DoMapMessageBox(MSG_BOX_BASIC_STYLE,
                                zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW], MAP_SCREEN,
                                MSG_BOX_FLAG_OK, NULL);
              }
            }
          } else if (fCtrl) {
            // go to SAVE screen
            gfSaveGame = TRUE;
            RequestTriggerExitFromMapscreen(MAP_EXIT_TO_SAVE);
          }
          break;
        case 't':
          // Teleport: CTRL-T
          if ((fCtrl) && (CHEATER_CHEAT_LEVEL())) {
            // check if selected dest char,
            if ((bSelectedDestChar != -1) && (fPlotForHelicopter == FALSE) &&
                (iCurrentMapSectorZ == 0) && (GetMouseMapXY(&sMapX, &sMapY))) {
              int16_t sDeltaX, sDeltaY;
              uint8_t sPrevX, sPrevY;
              struct SOLDIERTYPE *pSoldier = MercPtrs[gCharactersList[bSelectedDestChar].usSolID];

              // can't teleport to where we already are
              if ((sMapX == GetSolSectorX(pSoldier)) && (sMapY == GetSolSectorY(pSoldier))) break;

              // cancel movement plotting
              AbortMovementPlottingMode();

              // nuke the UI message generated by this
              CancelMapUIMessage();

              // clear their strategic movement (mercpaths and waypoints)
              ClearMvtForThisSoldierAndGang(pSoldier);

              // select this sector
              ChangeSelectedMapSector(sMapX, sMapY, 0);

              // check to see if this person is moving, if not...then assign them to mvt group
              if (pSoldier->ubGroupID == 0) {
                ubGroupId = CreateNewPlayerGroupDepartingFromSector(
                    (int8_t)(GetSolSectorX(pSoldier)), (int8_t)(GetSolSectorY(pSoldier)));
                // assign to a group
                AddPlayerToGroup(ubGroupId, pSoldier);
              }

              // figure out where they would've come from
              sDeltaX = sMapX - GetSolSectorX(pSoldier);
              sDeltaY = sMapY - pSoldier->sSectorY;

              if (abs(sDeltaX) >= abs(sDeltaY)) {
                // use East or West
                if (sDeltaX > 0) {
                  // came in from the West
                  sPrevX = sMapX - 1;
                  sPrevY = sMapY;
                } else {
                  // came in from the East
                  sPrevX = sMapX + 1;
                  sPrevY = sMapY;
                }
              } else {
                // use North or South
                if (sDeltaY > 0) {
                  // came in from the North
                  sPrevX = sMapX;
                  sPrevY = sMapY - 1;
                } else {
                  // came in from the South
                  sPrevX = sMapX;
                  sPrevY = sMapY + 1;
                }
              }

              // set where they are, were/are going, then make them arrive there and check for
              // battle
              PlaceGroupInSector(pSoldier->ubGroupID, sPrevX, sPrevY, sMapX, sMapY, 0, TRUE);

              // unload the sector they teleported out of
              CheckAndHandleUnloadingOfCurrentWorld();
            }
          } else {
            // only handle border button keyboard equivalents if the button is visible!
            if (!fShowMapInventoryPool) {
              // Toggle show teams flag
              ToggleShowTeamsMode();
            }
          }
          break;
        case 'u':
#ifdef JA2TESTVERSION
        {
          if (fAlt) {
            uint32_t uiCnt;
            // initialize miners if not already done so (fakes entering Drassen mine first)
            HandleQuestCodeOnSectorEntry(13, 4, 0);
            // test running out
            for (uiCnt = 0; uiCnt < 10; uiCnt++) {
              HandleIncomeFromMines();
            }
          }
        }
#endif
        break;
        case 'v':
          if (!fCtrl) {
            DisplayGameSettings();
          }
          break;
        case 'w':
          // only handle border button keyboard equivalents if the button is visible!
          if (!fShowMapInventoryPool) {
            // toggle show towns filter
            ToggleShowTownsMode();
          }
          break;
        case 'x':
          if (fAlt) {
            HandleShortCutExitState();
          }
          break;

        case 'y':
          // ALT-Y: toggles SAM sites disable
          if (fAlt) {
#ifdef JA2TESTVERSION
            fSAMSitesDisabledFromAttackingPlayer = !fSAMSitesDisabledFromAttackingPlayer;
#endif
          }
          break;

        case 'z':
          // only handle border button keyboard equivalents if the button is visible!
          if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              if (gfAutoAIAware ^= 1)
                ScreenMsg(FONT_WHITE, MSG_TESTVERSION, L"Strategic AI awareness maxed.");
              else
                ScreenMsg(FONT_WHITE, MSG_TESTVERSION, L"Strategic AI awareness normal.");
            }
          } else if (!fShowMapInventoryPool) {
            // Toggle Show Militia ON/OFF
            ToggleShowMilitiaMode();
          }
          break;
      }
    } else if (InputEvent.usEvent == KEY_REPEAT) {
      switch (InputEvent.usParam) {
        case LEFTARROW:
          // previous character
          GoToPrevCharacterInList();
          break;
        case RIGHTARROW:
          // next character
          GoToNextCharacterInList();
          break;

        case UPARROW:
          // up a line
          MapScreenMsgScrollUp(1);
          break;
        case DNARROW:
          // down a line
          MapScreenMsgScrollDown(1);
          break;

        case PGUP:
          // up a page
          MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);
          break;
        case PGDN:
          // down a page
          MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM);
          break;
      }
    }
  }
}

void EndMapScreen(BOOLEAN fDuringFade) {
  if (fInMapMode == FALSE) {
    // shouldn't be here
    return;
  }

  /*
          // exit is called due to message box, leave
          if( fMapExitDueToMessageBox )
          {
                  fMapExitDueToMessageBox = FALSE;
                  return;
          }
  */

  fLeavingMapScreen = FALSE;

  SetRenderFlags(RENDER_FLAG_FULL);
  // MSYS_EnableRegion( &gViewportRegion );
  // MSYS_EnableRegion( &gRadarRegion );
  // ATE: Shutdown tactical interface panel
  //	ShutdownCurrentPanel( );

  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();

    return;
  }

  // still plotting movement?
  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    AbortMovementPlottingMode();
  }

  DestroyMouseRegionsForTeamList();

  MSYS_RemoveRegion(&gMapViewRegion);
  MSYS_RemoveRegion(&gCharInfoFaceRegion);
  MSYS_RemoveRegion(&gCharInfoHandRegion);
  MSYS_RemoveRegion(&gMPanelRegion);
  MSYS_RemoveRegion(&gMapScreenMaskRegion);
  fInMapMode = FALSE;

  // remove team panel sort button
  RemoveTeamPanelSortButtonsForMapScreen();

  // for th merc insurance help text
  CreateDestroyInsuranceMouseRegionForMercs(FALSE);

  // gonna need to remove the screen mask regions
  CreateDestroyMouseRegionMasksForTimeCompressionButtons();

  UnloadButtonImage(giMapContractButtonImage);
  RemoveButton(giMapContractButton);

  HandleShutDownOfMapScreenWhileExternfaceIsTalking();

  fShowInventoryFlag = FALSE;
  CreateDestroyMapInvButton();

  // no longer can we show assignments menu
  fShowAssignmentMenu = FALSE;

  // clear out mouse regions for pop up boxes
  DetermineWhichAssignmentMenusCanBeShown();

  sSelectedMilitiaTown = 0;
  CreateDestroyMilitiaPopUPRegions();
  CreateDestroyMilitiaSectorButtons();

  // stop showing contract menu
  fShowContractMenu = FALSE;
  // clear out contract menu
  DetermineIfContractMenuCanBeShown();
  // remove contract pop up box (always created upon mapscreen entry)
  RemoveBox(ghContractBox);
  ghContractBox = -1;

  CreateDestroyAssignmentPopUpBoxes();

  // shutdown movement box
  if (fShowMapScreenMovementList) {
    fShowMapScreenMovementList = FALSE;
    CreateDestroyMovementBox(0, 0, 0);
  }

  // the remove merc from team box
  RemoveBox(ghRemoveMercAssignBox);
  ghRemoveMercAssignBox = -1;

  // clear screen mask if needed
  ClearScreenMaskForMapScreenExit();

  // get rid of pause clock area
  RemoveMouseRegionForPauseOfClock();

  // get rid of pop up for town info, if being shown
  fShowTownInfo = FALSE;
  CreateDestroyTownInfoBox();

  // build squad list
  RebuildCurrentSquad();

  //
  DeleteMouseRegionsForLevelMarkers();

  if (fShowMapInventoryPool == FALSE) {
    // delete buttons
    DeleteMapBorderButtons();
  }

  if (fShowDescriptionFlag) {
    DeleteItemDescriptionBox();
  }

  fShowInventoryFlag = FALSE;
  CreateDestroyTrashCanRegion();

  if (!fDuringFade) {
    MSYS_SetCurrentCursor(SCREEN_CURSOR);
  }

  if (fPreLoadedMapGraphics == FALSE) {
    DeleteMapBottomGraphics();

    DeleteVideoObjectFromIndex(guiSubLevel1);
    DeleteVideoObjectFromIndex(guiSubLevel2);
    DeleteVideoObjectFromIndex(guiSubLevel3);
    DeleteVideoObjectFromIndex(guiSleepIcon);
    DeleteVideoObjectFromIndex(guiMAPCURSORS);
    DeleteVideoObjectFromIndex(guiCHARLIST);
    DeleteVideoObjectFromIndex(guiCHARINFO);
    DeleteVideoObjectFromIndex(guiCHARICONS);
    DeleteVideoObjectFromIndex(guiCROSS);
    DeleteVideoObjectFromIndex(guiSAMICON);
    DeleteVideoObjectFromIndex(guiMAPINV);
    DeleteVideoObjectFromIndex(guiMapInvSecondHandBlockout);
    DeleteVideoObjectFromIndex(guiULICONS);
    DeleteVideoObjectFromIndex(guiORTAICON);
    DeleteVideoObjectFromIndex(guiTIXAICON);
    DeleteVideoObjectFromIndex(guiCHARBETWEENSECTORICONS);
    DeleteVideoObjectFromIndex(guiCHARBETWEENSECTORICONSCLOSE);
    DeleteVideoObjectFromIndex(guiLEVELMARKER);
    DeleteVideoObjectFromIndex(guiMapBorderEtaPopUp);

    DeleteVideoObjectFromIndex(guiSecItemHiddenVO);
    DeleteVideoObjectFromIndex(guiSelectedCharArrow);
    DeleteVideoObjectFromIndex(guiMapBorderHeliSectors);
    DeleteVideoObjectFromIndex(guiHelicopterIcon);
    DeleteVideoObjectFromIndex(guiMINEICON);
    DeleteVideoObjectFromIndex(guiSectorLocatorGraphicID);

    DeleteVideoObjectFromIndex(guiBULLSEYE);

    // remove the militia pop up box
    RemoveMilitiaPopUpBox();

    // remove inventory pool graphic
    RemoveInventoryPoolGraphic();

    // get rid of border stuff
    DeleteMapBorderGraphics();

    // Kris:  Remove the email icons.
    DeleteVideoObjectFromIndex(guiNewMailIcons);
  }

  DeleteVideoObjectFromIndex(guiBrownBackgroundForTeamPanel);

  RemoveMapStatusBarsRegion();

  fShowUpdateBox = FALSE;
  CreateDestroyTheUpdateBox();

  // get rid of mapscreen bottom
  DeleteMapScreenInterfaceBottom();

  // shutdown any mapscreen UI overlay message
  CancelMapUIMessage();

  CreateDestroyMapCharacterScrollButtons();

  // if time was ever compressed while we were in mapscreen
  if (HasTimeCompressOccured()) {
    // make sure everything tactical got cleared out
    ClearTacticalStuffDueToTimeCompression();
  }

  CancelSectorInventoryDisplayIfOn(TRUE);

  SetAllAutoFacesInactive();
  if (fLapTop) {
    StopAnyCurrentlyTalkingSpeech();
    guiCurrentScreen = LAPTOP_SCREEN;
  } else {
    guiCurrentScreen = GAME_SCREEN;

    // remove the progress bar
    RemoveProgressBar(0);

    // enable scroll string video overlays
    EnableDisableScrollStringVideoOverlay(TRUE);
  }

  // if going to tactical next
  if (guiPendingScreen == GAME_SCREEN) {
    // set compressed mode to Normal (X1)
    SetGameTimeCompressionLevel(TIME_COMPRESS_X1);
  } else  // going to another screen (options, laptop, save/load)
  {
    StopTimeCompression();
  }

  // update paused states, we are exiting...need to reset for any pathing or menus displayed
  UnLockPauseState();
  UpdatePausedStatesDueToTimeCompression();

  if (!gfDontStartTransitionFromLaptop) {
    uint32_t uiLaptopOn;

    // Load a tiny graphic of the on screen and draw it to the buffer.
    PlayJA2SampleFromFile("SOUNDS\\Initial Power Up (8-11).wav", RATE_11025, HIGHVOLUME, 1,
                          MIDDLEPAN);
    if (!AddVObject(CreateVObjectFromFile("INTERFACE\\LaptopOn.sti"), &uiLaptopOn))
      AssertMsg(0, "Failed to load data\\Interface\\LaptopOn.sti");
    BltVObjectFromIndex(vsFB, uiLaptopOn, 0, 465, 417);
    InvalidateRegion(465, 417, 480, 427);
    ExecuteBaseDirtyRectQueue();
    EndFrameBufferRender();
    DeleteVideoObjectFromIndex(uiLaptopOn);
    RefreshScreen(NULL);
  }

  // Kris:  Removes the pre battle interface, but only if it exists.
  //		   It is internally considered.
  KillPreBattleInterface();

  // cancel request if we somehow leave first
  gfRequestGiveSkyriderNewDestination = FALSE;
}

BOOLEAN GetMouseMapXY(uint8_t *psMapWorldX, uint8_t *psMapWorldY) {
  if (IsMapScreenHelpTextUp()) {
    // don't show highlight while global help text is up
    return (FALSE);
  }

  struct Point MousePos = GetMousePoint();

  return (GetMapXY((int16_t)MousePos.x, (int16_t)MousePos.y, psMapWorldX, psMapWorldY));
}

static BOOLEAN GetMapXY(int16_t sX, int16_t sY, uint8_t *psMapWorldX, uint8_t *psMapWorldY) {
  // Subtract start of map view
  int16_t x = sX - MAP_VIEW_START_X;
  int16_t y = sY - MAP_VIEW_START_Y;

  if (x < MAP_GRID_X || y < MAP_GRID_Y) {
    return (FALSE);
  }
  if (x < 0 || y < 0) {
    return (FALSE);
  }

  if (x > MAP_VIEW_WIDTH + MAP_GRID_X - 1 || y > MAP_VIEW_HEIGHT + 7 /* +MAP_VIEW_HEIGHT */) {
    return (FALSE);
  }
  if (x < 1 || y < 1) {
    return (FALSE);
  }

  *psMapWorldX = (x / MAP_GRID_X);
  *psMapWorldY = (y / MAP_GRID_Y);

  return (TRUE);
}

static void RenderMapHighlight(uint8_t sMapX, uint8_t sMapY, uint16_t usLineColor,
                               BOOLEAN fStationary) {
  int16_t sScreenX, sScreenY;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;

  Assert((sMapX >= 1) && (sMapX <= 16));
  Assert((sMapY >= 1) && (sMapY <= 16));

  // if we are not allowed to highlight, leave
  if ((IsTheCursorAllowedToHighLightThisSector(sMapX, sMapY) == FALSE)) {
    return;
  }

  GetScreenXYFromMapXY(sMapX, sMapY, &sScreenX, &sScreenY);

  // blit in the highlighted sector
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  // clip to view region
  ClipBlitsToMapViewRegionForRectangleAndABit(uiDestPitchBYTES);

  // draw rectangle for zoom out
  RectangleDraw(TRUE, sScreenX, sScreenY - 1, sScreenX + MAP_GRID_X, sScreenY + MAP_GRID_Y - 1,
                usLineColor, pDestBuf);
  InvalidateRegion(sScreenX, sScreenY - 2, sScreenX + DMAP_GRID_X + 1, sScreenY + DMAP_GRID_Y - 1);

  RestoreClipRegionToFullScreenForRectangle(uiDestPitchBYTES);
  JSurface_Unlock(vsFB);
}

void PollLeftButtonInMapView(uint32_t *puiNewEvent) {
  static BOOLEAN fLBBeenPressedInMapView = FALSE;
  uint8_t sMapX, sMapY;

  // if the mouse is currently over the MAP area
  if (gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    // if L-button is down at the moment
    if (gMapViewRegion.ButtonState & MSYS_LEFT_BUTTON) {
      if (!fLBBeenPressedInMapView) {
        fLBBeenPressedInMapView = TRUE;
        RESETCOUNTER(LMOUSECLICK_DELAY_COUNTER);

        gfAllowSkyriderTooFarQuote = FALSE;
      }
    } else  // L-button is NOT down at the moment
    {
      if (fLBBeenPressedInMapView) {
        fLBBeenPressedInMapView = FALSE;
        RESETCOUNTER(LMOUSECLICK_DELAY_COUNTER);

        // if we are showing help text in mapscreen
        if (fShowMapScreenHelpText) {
          fShowMapScreenHelpText = FALSE;
          fCharacterInfoPanelDirty = TRUE;
          MarkForRedrawalStrategicMap();
          return;
        }

        // if in militia redistribution popup
        if (sSelectedMilitiaTown != 0) {
          // ignore clicks outside the box
          return;
        }

        // left click cancels MAP UI messages, unless we're in confirm map move mode
        if ((giUIMessageOverlay != -1) && !gfInConfirmMapMoveMode) {
          CancelMapUIMessage();

          // return unless moving the bullseye
          if (!gfInChangeArrivalSectorMode) return;
        }

        // ignore left clicks in the map screen if:
        // game just started or we're in the prebattle interface or if we are about to hit
        // pre-battle
        if ((gTacticalStatus.fDidGameJustStart == TRUE) || (gfPreBattleInterfaceActive == TRUE) ||
            (fDisableMapInterfaceDueToBattle == TRUE)) {
          return;
        }

        // if in "plot route" mode
        if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
          fEndPlotting = FALSE;

          GetMouseMapXY(&sMapX, &sMapY);

          // if he clicked on the last sector in his current path
          if (CheckIfClickOnLastSectorInPath(sMapX, sMapY)) {
            DestinationPlottingCompleted();
          } else  // clicked on a new sector
          {
            gfAllowSkyriderTooFarQuote = TRUE;

            // draw new map route
            *puiNewEvent = MAP_EVENT_PLOT_PATH;
          }
        } else  // not plotting movement
        {
          // if not plotting a path
          if ((fEndPlotting == FALSE) && (fJustFinishedPlotting == FALSE)) {
            // make this sector selected / trigger movement box / start helicopter plotting /
            // changing arrival sector
            *puiNewEvent = MAP_EVENT_CLICK_SECTOR;
          }

          fEndPlotting = FALSE;
        }

        // reset town info flag
        fShowTownInfo = FALSE;
      }
    }
  }

  fJustFinishedPlotting = FALSE;
}

void PollRightButtonInMapView(uint32_t *puiNewEvent) {
  static BOOLEAN fRBBeenPressedInMapView = FALSE;
  uint8_t sMapX, sMapY;

  // if the mouse is currently over the MAP area
  if (gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    // if R-button is down at the moment
    if (gMapViewRegion.ButtonState & MSYS_RIGHT_BUTTON) {
      if (!fRBBeenPressedInMapView) {
        fRBBeenPressedInMapView = TRUE;
        RESETCOUNTER(RMOUSECLICK_DELAY_COUNTER);
      }
    } else  // R-button is NOT down at the moment
    {
      if (fRBBeenPressedInMapView) {
        fRBBeenPressedInMapView = FALSE;
        RESETCOUNTER(RMOUSECLICK_DELAY_COUNTER);

        // if we are showing help text in mapscreen
        if (fShowMapScreenHelpText) {
          fShowMapScreenHelpText = FALSE;
          fCharacterInfoPanelDirty = TRUE;
          MarkForRedrawalStrategicMap();
          return;
        }

        // if in militia redistribution popup
        if (sSelectedMilitiaTown != 0) {
          // ignore clicks outside the box
          return;
        }

        if (gfInChangeArrivalSectorMode) {
          CancelChangeArrivalSectorMode();
          MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[3]);
          return;
        }

        // right click cancels MAP UI messages, unless we're in confirm map move mode
        if ((giUIMessageOverlay != -1) && !gfInConfirmMapMoveMode) {
          CancelMapUIMessage();
          return;
        }

        // ignore right clicks in the map area if:
        // game just started or we're in the prebattle interface or if we are about to hit
        // pre-battle
        if ((gTacticalStatus.fDidGameJustStart == TRUE) || (gfPreBattleInterfaceActive == TRUE) ||
            (fDisableMapInterfaceDueToBattle == TRUE)) {
          return;
        }

        if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
          // cancel/shorten the path
          *puiNewEvent = MAP_EVENT_CANCEL_PATH;
        } else {
          if (GetMouseMapXY(&sMapX, &sMapY)) {
            if ((sSelMapX != sMapX) || (sSelMapY != sMapY)) {
              ChangeSelectedMapSector(sMapX, sMapY, (int8_t)iCurrentMapSectorZ);
            }
          }

          // sector must be selected to bring up militia or town info boxes for it
          if ((sMapX == sSelMapX) && (sSelMapY == sMapY)) {
            if (fShowMilitia == TRUE) {
              HandleMilitiaRedistributionClick();
            } else  // show militia is OFF
            {
              // if on the surface, or a real underground sector (we've visited it)
              if ((iCurrentMapSectorZ == 0) ||
                  (GetSectorFlagStatus(sMapX, sMapY, (uint8_t)iCurrentMapSectorZ,
                                       SF_ALREADY_VISITED) == TRUE)) {
                // toggle sector info for this sector
                fShowTownInfo = !fShowTownInfo;
                MarkForRedrawalStrategicMap();
              }
            }

            //						fMapScreenBottomDirty = TRUE;

            CreateDestroyScreenMaskForAssignmentAndContractMenus();
            if (fShowTownInfo == FALSE) {
              // destroy town info box
              CreateDestroyTownInfoBox();
            }
          }
        }
      }
    }
  }
}

void PopupText(wchar_t *pFontString, ...) {
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;
  va_list argptr;
  int16_t sX, sY;
  wchar_t PopupString[512];

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(PopupString, ARR_SIZE(PopupString), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  FindFontCenterCoordinates(0, 0, SCREEN_WIDTH, INTERFACE_START_Y, PopupString, LARGEFONT1, &sX,
                            &sY);

  BltVSurfaceToVSurfaceFast(vsFB, vsINTEXT, 85, 160);

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  SetFont(LARGEFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_DKGRAY);

  mprintf_buffer(pDestBuf, uiDestPitchBYTES, LARGEFONT1, sX, sY, PopupString);

  JSurface_Unlock(vsFB);

  InvalidateScreen();
}

void CreateDestroyMapInvButton() {
  static BOOLEAN fOldShowInventoryFlag = FALSE;

  if (fShowInventoryFlag && !fOldShowInventoryFlag) {
    // create inventory button
    fOldShowInventoryFlag = TRUE;
    // giMapInvButtonImage=  LoadButtonImage( "INTERFACE\\mapinv.sti" ,-1,1,-1,2,-1 );
    // giMapInvButton= QuickCreateButton( giMapInvButtonImage, INV_BTN_X-1, INV_BTN_Y,
    //				BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
    //				BUTTON_NO_CALLBACK, (GUI_CALLBACK)BtnINVCallback);
    // disable allmouse regions in this space
    fTeamPanelDirty = TRUE;

    InitInvSlotInterface(gMapScreenInvPocketXY, &gSCamoXY, MAPInvMoveCallback, MAPInvClickCallback,
                         MAPInvMoveCamoCallback, MAPInvClickCamoCallback, FALSE);
    MSYS_EnableRegion(&gMPanelRegion);

    // switch hand region help text to "Exit Inventory"
    SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[2]);

    // reset inventory item help text
    memset(gubMAP_HandInvDispText, 0, sizeof(gubMAP_HandInvDispText));

    // dirty character info panel  ( Why? ARM )
    fCharacterInfoPanelDirty = TRUE;
  } else if (!fShowInventoryFlag && fOldShowInventoryFlag) {
    // destroy inventory button
    ShutdownInvSlotInterface();
    fOldShowInventoryFlag = FALSE;
    // RemoveButton( giMapInvButton );
    // UnloadButtonImage( giMapInvButtonImage );
    fTeamPanelDirty = TRUE;
    MSYS_DisableRegion(&gMPanelRegion);

    // switch hand region help text to "Enter Inventory"
    SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[0]);

    // force immediate reblit of item in HANDPOS now that it's not blitted while in inventory mode
    fCharacterInfoPanelDirty = TRUE;
  }
}

void BltCharInvPanel() {
  uint32_t uiDestPitchBYTES;
  uint16_t *pDestBuf;
  struct VObject *hCharListHandle;
  struct SOLDIERTYPE *pSoldier;
  wchar_t sString[32];
  int16_t usX, usY;

  // make sure we're here legally
  Assert(MapCharacterHasAccessibleInventory(bSelectedInfoChar));

  GetSoldier(&pSoldier, gCharactersList[bSelectedInfoChar].usSolID);

  pDestBuf = (uint16_t *)LockVSurface(vsSaveBuffer, &uiDestPitchBYTES);
  GetVideoObject(&hCharListHandle, guiMAPINV);
  Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, hCharListHandle, PLAYER_INFO_X,
                                      PLAYER_INFO_Y, 0);
  JSurface_Unlock(vsSaveBuffer);

  Assert(pSoldier);
  CreateDestroyMapInvButton();

  if (gbCheckForMouseOverItemPos != -1) {
    if (HandleCompatibleAmmoUIForMapScreen(pSoldier, (int32_t)gbCheckForMouseOverItemPos, TRUE,
                                           TRUE) == TRUE) {
      MarkForRedrawalStrategicMap();
    }
  }

  if ((fShowMapInventoryPool)) {
    if (iCurrentlyHighLightedItem != -1) {
      HandleCompatibleAmmoUIForMapScreen(
          pSoldier,
          (int32_t)(iCurrentlyHighLightedItem +
                    (iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT)),
          TRUE, FALSE);
    }
  }

  RenderInvBodyPanel(pSoldier, INV_BODY_X, INV_BODY_Y);

  // reset font destination buffer to the save buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  // render items in each of chars slots
  HandleRenderInvSlots(pSoldier, DIRTYLEVEL2);

  // reset font destination buffer
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  SetFont(BLOCKFONT2);

  // Render Values for stats!
  // Set font drawing to saved buffer
  SetFontDestBuffer(vsSaveBuffer, 0, 0, 640, 480, FALSE);

  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(MAP_INV_STATS_TITLE_FONT_COLOR);

  // print armor/weight/camo labels
  mprintf(MAP_ARMOR_LABEL_X, MAP_ARMOR_LABEL_Y, pInvPanelTitleStrings[0]);
  mprintf(MAP_ARMOR_PERCENT_X, MAP_ARMOR_PERCENT_Y, L"%%");

  mprintf(MAP_WEIGHT_LABEL_X, MAP_WEIGHT_LABEL_Y, pInvPanelTitleStrings[1]);
  mprintf(MAP_WEIGHT_PERCENT_X, MAP_WEIGHT_PERCENT_Y, L"%%");

  mprintf(MAP_CAMMO_LABEL_X, MAP_CAMMO_LABEL_Y, pInvPanelTitleStrings[2]);
  mprintf(MAP_CAMMO_PERCENT_X, MAP_CAMMO_PERCENT_Y, L"%%");

  // display armor value
  swprintf(sString, ARR_SIZE(sString), L"%3d", ArmourPercent(pSoldier));
  FindFontRightCoordinates(MAP_ARMOR_X, MAP_ARMOR_Y, MAP_PERCENT_WIDTH, MAP_PERCENT_HEIGHT, sString,
                           BLOCKFONT2, &usX, &usY);
  mprintf(usX, usY, sString);

  // Display weight value
  swprintf(sString, ARR_SIZE(sString), L"%3d", CalculateCarriedWeight(pSoldier));
  FindFontRightCoordinates(MAP_WEIGHT_X, MAP_WEIGHT_Y, MAP_PERCENT_WIDTH, MAP_PERCENT_HEIGHT,
                           sString, BLOCKFONT2, &usX, &usY);
  mprintf(usX, usY, sString);

  // Display camo value
  swprintf(sString, ARR_SIZE(sString), L"%3d", pSoldier->bCamo);
  FindFontRightCoordinates(MAP_CAMMO_X, MAP_CAMMO_Y, MAP_PERCENT_WIDTH, MAP_PERCENT_HEIGHT, sString,
                           BLOCKFONT2, &usX, &usY);
  mprintf(usX, usY, sString);

  if (InKeyRingPopup()) {
    // shade the background
    ShadowVideoSurfaceRect(vsSaveBuffer, PLAYER_INFO_X, PLAYER_INFO_Y, PLAYER_INFO_X + 261,
                           PLAYER_INFO_Y + (359 - 107));
  } else {
    // blit gold key on top of key ring if key ring is not empty
  }

  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
}

// check for and highlight any ammo
void HandleCursorOverRifleAmmo() {
  if (fShowInventoryFlag == FALSE) {
    return;
  }

  if (gbCheckForMouseOverItemPos == -1) {
    return;
  }

  if (gfCheckForMouseOverItem) {
    if (HandleCompatibleAmmoUI(GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID),
                               (int8_t)gbCheckForMouseOverItemPos, TRUE)) {
      if ((GetJA2Clock() - guiMouseOverItemTime) > 100) {
        fTeamPanelDirty = TRUE;
      }
    }
  }
}

void MAPInvClickCamoCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {}

void MAPInvMoveCamoCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {}

// this is Map Screen's version of SMInvMoveCallback()
void MAPInvMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiHandPos;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }

  // make sure we're here legally
  Assert(MapCharacterHasAccessibleInventory(bSelectedInfoChar));

  GetSoldier(&pSoldier, gCharactersList[bSelectedInfoChar].usSolID);

  uiHandPos = MSYS_GetRegionUserData(pRegion, 0);

  // gbCheckForMouseOverItemPos = -1;

  if (pSoldier->inv[uiHandPos].usItem == NOTHING) return;

  if (iReason == MSYS_CALLBACK_REASON_MOVE) {
  } else if (iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE)
  //  if( ( iReason == MSYS_CALLBACK_REASON_MOVE ) || ( iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE )
  //  )
  {
    gubMAP_HandInvDispText[uiHandPos] = 2;
    guiMouseOverItemTime = GetJA2Clock();
    gfCheckForMouseOverItem = TRUE;
    HandleCompatibleAmmoUI(pSoldier, (int8_t)uiHandPos, FALSE);
    gbCheckForMouseOverItemPos = (int8_t)uiHandPos;
  }
  if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE) {
    gubMAP_HandInvDispText[uiHandPos] = 1;
    HandleCompatibleAmmoUI(pSoldier, (int8_t)uiHandPos, FALSE);
    gfCheckForMouseOverItem = FALSE;
    fTeamPanelDirty = TRUE;
    gbCheckForMouseOverItemPos = -1;
  }
}

// mapscreen wrapper to init the item description box
BOOLEAN MAPInternalInitItemDescriptionBox(struct OBJECTTYPE *pObject, uint8_t ubStatusIndex,
                                          struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fRet;

  fRet = InternalInitItemDescriptionBox(pObject, MAP_ITEMDESC_START_X, MAP_ITEMDESC_START_Y,
                                        ubStatusIndex, pSoldier);

  fShowDescriptionFlag = TRUE;
  fTeamPanelDirty = TRUE;
  fInterfacePanelDirty = DIRTYLEVEL2;

  return (fRet);
}

// this is Map Screen's version of SMInvClickCallback()
void MAPInvClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiHandPos;
  uint16_t usOldItemIndex, usNewItemIndex;
  static BOOLEAN fRightDown = FALSE;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }

  // make sure we're here legally
  Assert(MapCharacterHasAccessibleInventory(bSelectedInfoChar));

  GetSoldier(&pSoldier, gCharactersList[bSelectedInfoChar].usSolID);

  uiHandPos = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // If we do not have an item in hand, start moving it
    if (gpItemPointer == NULL) {
      // Return if empty
      if (pSoldier->inv[uiHandPos].usItem == NOTHING) {
        return;
      }

      // ATE: Put this here to handle Nails refusal....
      if (HandleNailsVestFetish(pSoldier, uiHandPos, NOTHING)) {
        return;
      }

      if (_KeyDown(CTRL)) {
        CleanUpStack(&(pSoldier->inv[uiHandPos]), NULL);
      }

      // remember what it was
      usOldItemIndex = pSoldier->inv[uiHandPos].usItem;

      // pick it up
      MAPBeginItemPointer(pSoldier, (uint8_t)uiHandPos);

      // remember which gridno the object came from
      sObjectSourceGridNo = pSoldier->sGridNo;

      HandleTacticalEffectsOfEquipmentChange(pSoldier, uiHandPos, usOldItemIndex, NOTHING);

      fInterfacePanelDirty = DIRTYLEVEL2;
      fCharacterInfoPanelDirty = TRUE;
    } else  // item in cursor
    {
      // can we pass this part due to booby traps
      if (ContinuePastBoobyTrapInMapScreen(gpItemPointer, pSoldier) == FALSE) {
        return;
      }

      usOldItemIndex = pSoldier->inv[uiHandPos].usItem;
      usNewItemIndex = gpItemPointer->usItem;

      // ATE: Put this here to handle Nails refusal....
      if (HandleNailsVestFetish(pSoldier, uiHandPos, usNewItemIndex)) {
        return;
      }

      if (_KeyDown(CTRL)) {
        CleanUpStack(&(pSoldier->inv[uiHandPos]), gpItemPointer);
        if (gpItemPointer->ubNumberOfObjects == 0) {
          MAPEndItemPointer();
        }
        return;
      }

      // !!! ATTACHING/MERGING ITEMS IN MAP SCREEN IS NOT SUPPORTED !!!
      if (uiHandPos == HANDPOS || uiHandPos == SECONDHANDPOS || uiHandPos == HELMETPOS ||
          uiHandPos == VESTPOS || uiHandPos == LEGPOS) {
        // if ( ValidAttachmentClass( usNewItemIndex, usOldItemIndex ) )
        if (ValidAttachment(usNewItemIndex, usOldItemIndex)) {
          // it's an attempt to attach; bring up the inventory panel
          if (!InItemDescriptionBox()) {
            MAPInternalInitItemDescriptionBox(&(pSoldier->inv[uiHandPos]), 0, pSoldier);
          }
          return;
        } else if (ValidMerge(usNewItemIndex, usOldItemIndex)) {
          // bring up merge requestor
          // TOO PAINFUL TO DO!! --CC
          if (!InItemDescriptionBox()) {
            MAPInternalInitItemDescriptionBox(&(pSoldier->inv[uiHandPos]), 0, pSoldier);
          }

          /*
          gubHandPos = (uint8_t) uiHandPos;
          gusOldItemIndex = usOldItemIndex;
          gusNewItemIndex = usNewItemIndex;
          gfDeductPoints = fDeductPoints;

          DoScreenIndependantMessageBox( Message[ STR_MERGE_ITEMS ], MSG_BOX_FLAG_YESNO,
          MergeMessageBoxCallBack ); return;
          */
        }
        // else handle normally
      }

      // Else, try to place here
      if (PlaceObject(pSoldier, (uint8_t)uiHandPos, gpItemPointer)) {
        HandleTacticalEffectsOfEquipmentChange(pSoldier, uiHandPos, usOldItemIndex, usNewItemIndex);

        // Dirty
        fInterfacePanelDirty = DIRTYLEVEL2;
        fCharacterInfoPanelDirty = TRUE;
        MarkForRedrawalStrategicMap();

        // Check if cursor is empty now
        if (gpItemPointer->ubNumberOfObjects == 0) {
          MAPEndItemPointer();
        } else  // items swapped
        {
          // Update mouse cursor
          guiExternVo = GetInterfaceGraphicForItem(&(Item[gpItemPointer->usItem]));
          gusExternVoSubIndex = Item[gpItemPointer->usItem].ubGraphicNum;

          MSYS_ChangeRegionCursor(&gMPanelRegion, EXTERN_CURSOR);
          MSYS_SetCurrentCursor(EXTERN_CURSOR);
          fMapInventoryItem = TRUE;
          fTeamPanelDirty = TRUE;

          // remember which gridno the object came from
          sObjectSourceGridNo = pSoldier->sGridNo;
          // and who owned it last
          gpItemPointerSoldier = pSoldier;

          ReevaluateItemHatches(pSoldier, FALSE);
        }

        // re-evaluate repairs
        gfReEvaluateEveryonesNothingToDo = TRUE;

        // if item came from another merc
        if (gpItemPointerSoldier != pSoldier) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_ITEM_PASSED_TO_MERC],
                    ShortItemNames[usNewItemIndex], pSoldier->name);
        }
      }
    }
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    // if there is a map UI message being displayed
    if (giUIMessageOverlay != -1) {
      CancelMapUIMessage();
      return;
    }

    fRightDown = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP && fRightDown) {
    fRightDown = FALSE;

    // Return if empty
    if (pSoldier->inv[uiHandPos].usItem == NOTHING) {
      return;
    }

    // Some global stuff here - for esc, etc
    // Check for # of slots in item
    if ((pSoldier->inv[uiHandPos].ubNumberOfObjects > 1) &&
        (ItemSlotLimit(pSoldier->inv[uiHandPos].usItem, (uint8_t)uiHandPos) > 0)) {
      if (!InItemStackPopup()) {
        InitItemStackPopup(pSoldier, (uint8_t)uiHandPos, 0, INV_REGION_Y, 261, 248);
        fTeamPanelDirty = TRUE;
        fInterfacePanelDirty = DIRTYLEVEL2;
      }
    } else {
      if (!InItemDescriptionBox()) {
        InitItemDescriptionBox(pSoldier, (uint8_t)uiHandPos, MAP_ITEMDESC_START_X,
                               MAP_ITEMDESC_START_Y, 0);
        fShowDescriptionFlag = TRUE;
        fTeamPanelDirty = TRUE;
        fInterfacePanelDirty = DIRTYLEVEL2;
      }
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fRightDown = FALSE;
  }
}

void InternalMAPBeginItemPointer(struct SOLDIERTYPE *pSoldier) {
  // If not null return
  if (gpItemPointer != NULL) {
    return;
  }

  // Set global indicator
  gpItemPointer = &gItemPointer;
  gpItemPointerSoldier = pSoldier;

  // Set mouse
  guiExternVo = GetInterfaceGraphicForItem(&(Item[gpItemPointer->usItem]));
  gusExternVoSubIndex = Item[gpItemPointer->usItem].ubGraphicNum;

  MSYS_ChangeRegionCursor(&gMPanelRegion, EXTERN_CURSOR);
  MSYS_SetCurrentCursor(EXTERN_CURSOR);
  fMapInventoryItem = TRUE;
  fTeamPanelDirty = TRUE;

  // hatch out incompatible inventory slots
  ReevaluateItemHatches(pSoldier, FALSE);

  // re-evaluate repairs
  gfReEvaluateEveryonesNothingToDo = TRUE;
}

void MAPBeginItemPointer(struct SOLDIERTYPE *pSoldier, uint8_t ubHandPos) {
  BOOLEAN fOk;

  // If not null return
  if (gpItemPointer != NULL) {
    return;
  }

  if (_KeyDown(SHIFT)) {
    // Remove all from soldier's slot
    fOk = RemoveObjectFromSlot(pSoldier, ubHandPos, &gItemPointer);
  } else {
    GetObjFrom(&(pSoldier->inv[ubHandPos]), 0, &gItemPointer);
    fOk = (gItemPointer.ubNumberOfObjects == 1);
  }

  if (fOk) {
    InternalMAPBeginItemPointer(pSoldier);
  }
}

void MAPBeginKeyRingItemPointer(struct SOLDIERTYPE *pSoldier, uint8_t uiKeySlot) {
  // If not null return
  if (gpItemPointer != NULL) {
    return;
  }

  // Set mouse
  guiExternVo = GetInterfaceGraphicForItem(&(Item[gpItemPointer->usItem]));
  gusExternVoSubIndex = Item[gpItemPointer->usItem].ubGraphicNum;

  MSYS_ChangeRegionCursor(&gMPanelRegion, EXTERN_CURSOR);
  MSYS_SetCurrentCursor(EXTERN_CURSOR);
  fMapInventoryItem = TRUE;
  fTeamPanelDirty = TRUE;
}

void MAPEndItemPointer() {
  if (gpItemPointer != NULL) {
    gpItemPointer = NULL;
    MSYS_ChangeRegionCursor(&gMPanelRegion, CURSOR_NORMAL);
    MSYS_SetCurrentCursor(CURSOR_NORMAL);
    fMapInventoryItem = FALSE;
    fTeamPanelDirty = TRUE;

    if (fShowMapInventoryPool) {
      HandleButtonStatesWhileMapInventoryActive();
    }

    if (fShowInventoryFlag && bSelectedInfoChar >= 0) {
      ReevaluateItemHatches(MercPtrs[gCharactersList[bSelectedInfoChar].usSolID], FALSE);
    }
  }
}

void HandleMapInventoryCursor() {
  if (fMapInventoryItem) MSYS_SetCurrentCursor(EXTERN_CURSOR);
  return;
}

// will place down the upper left hand corner attribute strings
static void RenderAttributeStringsForUpperLeftHandCorner(struct JSurface *vsBufferToRenderTo) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  if ((bSelectedInfoChar != -1) && (gCharactersList[bSelectedInfoChar].fValid)) {
    pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];
  }

  SetFont(CHAR_FONT);
  SetFontForeground(CHAR_TITLE_FONT_COLOR);
  SetFontBackground(FONT_BLACK);
  SetFontDestBuffer(vsBufferToRenderTo, 0, 0, 640, 480, FALSE);

  // assignment strings
  DrawString(pUpperLeftMapScreenStrings[0],
             (uint16_t)(220 - StringPixLength(pUpperLeftMapScreenStrings[0], CHAR_FONT) / 2), 6,
             CHAR_FONT);

  // vehicles and robot don't have attributes, contracts, or morale
  if ((pSoldier == NULL) ||
      (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(pSoldier))) {
    // health
    DrawString(pUpperLeftMapScreenStrings[2], 87, 80, CHAR_FONT);

    for (iCounter = 0; iCounter < 5; iCounter++) {
      DrawString(pShortAttributeStrings[iCounter], 88, (int16_t)(22 + iCounter * 10), CHAR_FONT);
      DrawString(pShortAttributeStrings[iCounter + 5], 133, (int16_t)(22 + iCounter * 10),
                 CHAR_FONT);
    }

    // contract
    // DrawString(pUpperLeftMapScreenStrings[ 1 ], 194, 52,  CHAR_FONT);

    // morale
    DrawString(pUpperLeftMapScreenStrings[3], 87, 94, CHAR_FONT);
  } else {
    // condition
    DrawString(pUpperLeftMapScreenStrings[4], 96, 80, CHAR_FONT);
  }

  // restore buffer
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
}

void DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface(int16_t sMapX,
                                                                                int16_t sMapY) {
  // simply check if we want to refresh the screen to display path
  static int8_t bOldDestChar = -1;
  static int16_t sPrevMapX, sPrevMapY;
  int32_t iDifference = 0;

  if (bOldDestChar != bSelectedDestChar) {
    bOldDestChar = bSelectedDestChar;
    giPotCharPathBaseTime = GetJA2Clock();

    sPrevMapX = sMapX;
    sPrevMapY = sMapY;
    fTempPathAlreadyDrawn = FALSE;
    fDrawTempPath = FALSE;
  }

  if ((sMapX != sPrevMapX) || (sMapY != sPrevMapY)) {
    giPotCharPathBaseTime = GetJA2Clock();

    sPrevMapX = sMapX;
    sPrevMapY = sMapY;

    // path was plotted and we moved, re draw map..to clean up mess
    if (fTempPathAlreadyDrawn == TRUE) {
      MarkForRedrawalStrategicMap();
    }

    fTempPathAlreadyDrawn = FALSE;
    fDrawTempPath = FALSE;
  }

  iDifference = GetJA2Clock() - giPotCharPathBaseTime;

  if (fTempPathAlreadyDrawn == TRUE) {
    return;
  }

  if (iDifference > MIN_WAIT_TIME_FOR_TEMP_PATH) {
    fDrawTempPath = TRUE;
    giPotCharPathBaseTime = GetJA2Clock();
    fTempPathAlreadyDrawn = TRUE;
  }

  return;
}

void SetUpCursorForStrategicMap(void) {
  if (gfInChangeArrivalSectorMode == FALSE) {
    // check if character is in destination plotting mode
    if (fPlotForHelicopter == FALSE) {
      if (bSelectedDestChar == -1) {
        // no plot mode, reset cursor to normal
        ChangeMapScreenMaskCursor(CURSOR_NORMAL);
      } else  // yes - by character
      {
        // set cursor based on foot or vehicle
        if ((Menptr[gCharactersList[bSelectedDestChar].usSolID].bAssignment != VEHICLE) &&
            !(Menptr[gCharactersList[bSelectedDestChar].usSolID].uiStatusFlags & SOLDIER_VEHICLE)) {
          ChangeMapScreenMaskCursor(CURSOR_STRATEGIC_FOOT);
        } else {
          ChangeMapScreenMaskCursor(CURSOR_STRATEGIC_VEHICLE);
        }
      }
    } else  // yes - by helicopter
    {
      // set cursor to chopper
      ChangeMapScreenMaskCursor(CURSOR_CHOPPER);
    }
  } else {
    // set cursor to bullseye
    ChangeMapScreenMaskCursor(CURSOR_STRATEGIC_BULLSEYE);
  }
}

void HandleAnimatedCursorsForMapScreen() {
  if (COUNTERDONE(CURSORCOUNTER)) {
    RESETCOUNTER(CURSORCOUNTER);
    UpdateAnimatedCursorFrames(gMapScreenMaskRegion.Cursor);
    SetCurrentCursorFromDatabase(gMapScreenMaskRegion.Cursor);
  }
}

void AbortMovementPlottingMode(void) {
  // invalid if we're not plotting movement
  Assert((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE));

  // make everybody go back to where they were going before this plotting session started
  RestorePreviousPaths();

  // don't need the previous paths any more
  ClearPreviousPaths();

  // clear the character's temporary path (this is the route being constantly updated on the map)
  if (pTempCharacterPath) {
    // make sure we're at the beginning
    pTempCharacterPath = MoveToBeginningOfPathList(pTempCharacterPath);
    pTempCharacterPath = ClearStrategicPathList(pTempCharacterPath, 0);
  }

  // clear the helicopter's temporary path (this is the route being constantly updated on the map)
  if (pTempHelicopterPath) {
    // make sure we're at the beginning
    pTempHelicopterPath = MoveToBeginningOfPathList(pTempHelicopterPath);
    pTempHelicopterPath = ClearStrategicPathList(pTempHelicopterPath, 0);
  }

  EndConfirmMapMoveMode();

  // cancel destination line highlight
  giDestHighLine = -1;

  // cancel movement mode
  bSelectedDestChar = -1;
  fPlotForHelicopter = FALSE;

  // tell player the route was UNCHANGED
  MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[2]);

  // reset cursors
  ChangeMapScreenMaskCursor(CURSOR_NORMAL);
  SetUpCursorForStrategicMap();

  // restore glow region
  RestoreBackgroundForDestinationGlowRegionList();

  // we might be on the map, redraw to remove old path stuff
  MarkForRedrawalStrategicMap();
  fTeamPanelDirty = TRUE;

  gfRenderPBInterface = TRUE;
}

void CheckToSeeIfMouseHasLeftMapRegionDuringPathPlotting() {
  static BOOLEAN fInArea = FALSE;

  if ((gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA) == 0) {
    if (fInArea == TRUE) {
      fInArea = FALSE;

      // plotting path, clean up
      if (((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE) ||
           (fDrawTempHeliPath == TRUE)) &&
          (fTempPathAlreadyDrawn == TRUE)) {
        fDrawTempHeliPath = FALSE;
        MarkForRedrawalStrategicMap();
        gfRenderPBInterface = TRUE;

        // clear the temp path
        if (pTempCharacterPath) {
          pTempCharacterPath = ClearStrategicPathList(pTempCharacterPath, 0);
        }
      }

      // reset fact temp path has been drawn
      fTempPathAlreadyDrawn = FALSE;
    }
  } else {
    fInArea = TRUE;
  }

  return;
}

void BlitBackgroundToSaveBuffer(void) {
  // render map
  RenderMapRegionBackground();

  if (fDisableDueToBattleRoster == FALSE) {
    // render team
    RenderTeamRegionBackground();

    // render character info
    RenderCharacterInfoBackground();
  } else if (gfPreBattleInterfaceActive) {
    ForceButtonUnDirty(giMapContractButton);
    ForceButtonUnDirty(giCharInfoButton[0]);
    ForceButtonUnDirty(giCharInfoButton[1]);
    RenderPreBattleInterface();
  }

  // now render lower panel
  RenderMapScreenInterfaceBottom();
}

void CreateMouseRegionsForTeamList(void) {
  // will create mouse regions for assignments, path plotting, character info selection
  int16_t sCounter = 0;
  int16_t sYAdd = 0;

  // the info region...is the background for the list itself

  for (sCounter = 0; sCounter < MAX_CHARACTER_COUNT; sCounter++) {
    if (sCounter >= FIRST_VEHICLE) {
      sYAdd = 6;
    } else {
      sYAdd = 0;
    }

    // name region
    MSYS_DefineRegion(&gTeamListNameRegion[sCounter], NAME_X,
                      (int16_t)(Y_START + (sCounter) * (Y_SIZE + 2) + sYAdd), NAME_X + NAME_WIDTH,
                      (int16_t)(145 + (sCounter + 1) * (Y_SIZE + 2) + sYAdd), MSYS_PRIORITY_NORMAL,
                      MSYS_NO_CURSOR, TeamListInfoRegionMvtCallBack, TeamListInfoRegionBtnCallBack);

    // assignment region
    MSYS_DefineRegion(
        &gTeamListAssignmentRegion[sCounter], ASSIGN_X,
        (int16_t)(Y_START + (sCounter) * (Y_SIZE + 2) + sYAdd), ASSIGN_X + ASSIGN_WIDTH,
        (int16_t)(145 + (sCounter + 1) * (Y_SIZE + 2) + sYAdd), MSYS_PRIORITY_NORMAL + 1,
        MSYS_NO_CURSOR, TeamListAssignmentRegionMvtCallBack, TeamListAssignmentRegionBtnCallBack);

    // location region (same function as name regions, so uses the same callbacks)
    MSYS_DefineRegion(&gTeamListLocationRegion[sCounter], LOC_X,
                      (int16_t)(Y_START + (sCounter) * (Y_SIZE + 2) + sYAdd), LOC_X + LOC_WIDTH,
                      (int16_t)(145 + (sCounter + 1) * (Y_SIZE + 2) + sYAdd),
                      MSYS_PRIORITY_NORMAL + 1, MSYS_NO_CURSOR, TeamListInfoRegionMvtCallBack,
                      TeamListInfoRegionBtnCallBack);

    // destination region
    MSYS_DefineRegion(
        &gTeamListDestinationRegion[sCounter], DEST_ETA_X,
        (int16_t)(Y_START + (sCounter) * (Y_SIZE + 2) + sYAdd), DEST_ETA_X + DEST_ETA_WIDTH,
        (int16_t)(145 + (sCounter + 1) * (Y_SIZE + 2) + sYAdd), MSYS_PRIORITY_NORMAL + 1,
        MSYS_NO_CURSOR, TeamListDestinationRegionMvtCallBack, TeamListDestinationRegionBtnCallBack);

    // contract region
    MSYS_DefineRegion(&gTeamListContractRegion[sCounter], TIME_REMAINING_X,
                      (int16_t)(Y_START + (sCounter) * (Y_SIZE + 2) + sYAdd),
                      TIME_REMAINING_X + TIME_REMAINING_WIDTH,
                      (int16_t)(145 + (sCounter + 1) * (Y_SIZE + 2) + sYAdd),
                      MSYS_PRIORITY_NORMAL + 1, MSYS_NO_CURSOR, TeamListContractRegionMvtCallBack,
                      TeamListContractRegionBtnCallBack);

    // contract region
    MSYS_DefineRegion(&gTeamListSleepRegion[sCounter], SLEEP_X,
                      (int16_t)(Y_START + (sCounter) * (Y_SIZE + 2) + sYAdd), SLEEP_X + SLEEP_WIDTH,
                      (int16_t)(145 + (sCounter + 1) * (Y_SIZE + 2) + sYAdd),
                      MSYS_PRIORITY_NORMAL + 1, MSYS_NO_CURSOR, TeamListSleepRegionMvtCallBack,
                      TeamListSleepRegionBtnCallBack);

    MSYS_SetRegionUserData(&gTeamListNameRegion[sCounter], 0, sCounter);
    MSYS_SetRegionUserData(&gTeamListAssignmentRegion[sCounter], 0, sCounter);
    MSYS_SetRegionUserData(&gTeamListSleepRegion[sCounter], 0, sCounter);
    MSYS_SetRegionUserData(&gTeamListLocationRegion[sCounter], 0, sCounter);
    MSYS_SetRegionUserData(&gTeamListDestinationRegion[sCounter], 0, sCounter);
    MSYS_SetRegionUserData(&gTeamListContractRegion[sCounter], 0, sCounter);

    // set up help boxes
    SetRegionFastHelpText(&gTeamListNameRegion[sCounter], pMapScreenMouseRegionHelpText[0]);
    SetRegionFastHelpText(&gTeamListAssignmentRegion[sCounter], pMapScreenMouseRegionHelpText[1]);
    SetRegionFastHelpText(&gTeamListSleepRegion[sCounter], pMapScreenMouseRegionHelpText[5]);
    SetRegionFastHelpText(&gTeamListLocationRegion[sCounter], pMapScreenMouseRegionHelpText[0]);
    SetRegionFastHelpText(&gTeamListDestinationRegion[sCounter], pMapScreenMouseRegionHelpText[2]);
    SetRegionFastHelpText(&gTeamListContractRegion[sCounter], pMapScreenMouseRegionHelpText[3]);
  }

  return;
}

void DestroyMouseRegionsForTeamList(void) {
  // will destroy mouse regions overlaying the team list area
  int32_t sCounter = 0;

  for (sCounter = 0; sCounter < MAX_CHARACTER_COUNT; sCounter++) {
    MSYS_RemoveRegion(&gTeamListNameRegion[sCounter]);
    MSYS_RemoveRegion(&gTeamListAssignmentRegion[sCounter]);
    MSYS_RemoveRegion(&gTeamListSleepRegion[sCounter]);
    MSYS_RemoveRegion(&gTeamListDestinationRegion[sCounter]);
    MSYS_RemoveRegion(&gTeamListLocationRegion[sCounter]);
    MSYS_RemoveRegion(&gTeamListContractRegion[sCounter]);
  }
}

// mask for mapscreen region
void MapScreenMarkRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void ContractButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if ((iDialogueBox != -1)) {
    return;
  }

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }

    /*
                    if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
                    {
                            AbortMovementPlottingMode( );
                            return;
                    }
    */

    // redraw region
    if (btn->Area.uiFlags & MSYS_HAS_BACKRECT) {
      fCharacterInfoPanelDirty = TRUE;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }

    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      RequestContractMenu();
    }
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }
  }
}

void TeamListInfoRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set to new info character...make sure is valid
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if (gCharactersList[iValue].fValid == TRUE) {
      if (HandleCtrlOrShiftInTeamPanel((int8_t)iValue)) {
        return;
      }

      ChangeSelectedInfoChar((int8_t)iValue, TRUE);

      pSoldier = GetSoldierByID(gCharactersList[iValue].usSolID);

      // highlight
      giDestHighLine = -1;

      // reset character
      bSelectedAssignChar = -1;
      bSelectedDestChar = -1;
      bSelectedContractChar = -1;
      fPlotForHelicopter = FALSE;

      // if not dead or POW, select his sector
      if (IsSolAlive(pSoldier) && (pSoldier->bAssignment != ASSIGNMENT_POW)) {
        ChangeSelectedMapSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                GetSolSectorZ(pSoldier));
      }

      // unhilight contract line
      giContractHighLine = -1;

      // can't assign highlight line
      giAssignHighLine = -1;

      // dirty team and map regions
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      // fMapScreenBottomDirty = TRUE;
      gfRenderPBInterface = TRUE;
    } else {
      // reset selected characters
      ResetAllSelectedCharacterModes();
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if (gCharactersList[iValue].fValid == TRUE) {
      pSoldier = GetSoldierByID(gCharactersList[iValue].usSolID);

      // select this character
      ChangeSelectedInfoChar((int8_t)iValue, TRUE);

      RequestToggleMercInventoryPanel();

      // highlight
      giDestHighLine = -1;

      // reset character
      bSelectedAssignChar = -1;
      bSelectedDestChar = -1;
      bSelectedContractChar = -1;
      fPlotForHelicopter = FALSE;

      // if not dead or POW, select his sector
      if (IsSolAlive(pSoldier) && (pSoldier->bAssignment != ASSIGNMENT_POW)) {
        ChangeSelectedMapSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                GetSolSectorZ(pSoldier));
      }

      // unhilight contract line
      giContractHighLine = -1;

      // can't assign highlight line
      giAssignHighLine = -1;

      // dirty team and map regions
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      //			fMapScreenBottomDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }
  }
}

void TeamListInfoRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    if (gCharactersList[iValue].fValid == TRUE) {
      giHighLine = iValue;
    } else {
      giHighLine = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    giHighLine = -1;
  }
}

void TeamListAssignmentRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set to new info character...make sure is valid
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if (gCharactersList[iValue].fValid == TRUE) {
      if (HandleCtrlOrShiftInTeamPanel((int8_t)iValue)) {
        return;
      }

      // reset list if the clicked character isn't also selected
      ChangeSelectedInfoChar((int8_t)iValue,
                             (BOOLEAN)(IsEntryInSelectedListSet((int8_t)iValue) == FALSE));

      pSoldier = GetSoldierByID(gCharactersList[iValue].usSolID);

      // if alive (dead guys keep going, use remove menu instead),
      // and it's between sectors and it can be reassigned (non-vehicles)
      if ((pSoldier->bAssignment != ASSIGNMENT_DEAD) && IsSolAlive(pSoldier) &&
          (pSoldier->fBetweenSectors) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
        // can't reassign mercs while between sectors
        DoScreenIndependantMessageBox(pMapErrorString[41], MSG_BOX_FLAG_OK, NULL);
        return;
      }

      bSelectedAssignChar = (int8_t)iValue;
      RebuildAssignmentsBox();

      // reset dest character
      bSelectedDestChar = -1;
      fPlotForHelicopter = FALSE;

      // reset contract char
      bSelectedContractChar = -1;
      giContractHighLine = -1;

      // can't highlight line, anymore..if we were
      giDestHighLine = -1;

      // dirty team and map regions
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;

      // if this thing can be re-assigned
      if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
        giAssignHighLine = iValue;

        fShowAssignmentMenu = TRUE;

        if ((pSoldier->bLife == 0) || (GetSolAssignment(pSoldier) == ASSIGNMENT_POW)) {
          fShowRemoveMenu = TRUE;
        }
      } else {
        // can't highlight line
        giAssignHighLine = -1;

        // we can't highlight this line
        //				giHighLine = -1;
      }
    } else {
      // reset selected characters
      ResetAllSelectedCharacterModes();
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void TeamListAssignmentRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    if (gCharactersList[iValue].fValid == TRUE) {
      giHighLine = iValue;

      if (!(Menptr[gCharactersList[iValue].usSolID].uiStatusFlags & SOLDIER_VEHICLE)) {
        giAssignHighLine = iValue;
      } else {
        giAssignHighLine = -1;
      }
    } else {
      // can't highlight line
      giHighLine = -1;
      giAssignHighLine = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    giHighLine = -1;

    if (bSelectedAssignChar == -1) {
      giAssignHighLine = -1;
    }

    // restore background
    RestoreBackgroundForAssignmentGlowRegionList();
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if ((gCharactersList[iValue].fValid == TRUE) &&
        !(Menptr[gCharactersList[iValue].usSolID].uiStatusFlags & SOLDIER_VEHICLE)) {
      // play click
      PlayGlowRegionSound();
    }
  }
}

void TeamListDestinationRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive || fShowMapInventoryPool) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if (gCharactersList[iValue].fValid == TRUE) {
      if (HandleCtrlOrShiftInTeamPanel((int8_t)iValue)) {
        return;
      }

      // reset list if the clicked character isn't also selected
      ChangeSelectedInfoChar((int8_t)iValue,
                             (BOOLEAN)(IsEntryInSelectedListSet((int8_t)iValue) == FALSE));

      // deselect any characters/vehicles that can't accompany the clicked merc
      DeselectSelectedListMercsWhoCantMoveWithThisGuy(&(Menptr[gCharactersList[iValue].usSolID]));

      // select all characters/vehicles that MUST accompany the clicked merc (same squad/vehicle)
      SelectUnselectedMercsWhoMustMoveWithThisGuy();

      // Find out if this guy and everyone travelling with him is allowed to move strategically
      // NOTE: errors are reported within...
      if (CanChangeDestinationForCharSlot((int8_t)iValue, TRUE)) {
        // turn off sector inventory, turn on show teams filter, etc.
        MakeMapModesSuitableForDestPlotting((int8_t)iValue);

        // check if person is in a vehicle
        if (Menptr[gCharactersList[iValue].usSolID].bAssignment == VEHICLE) {
          // if he's in the helicopter
          if (Menptr[gCharactersList[iValue].usSolID].iVehicleId == iHelicopterVehicleId) {
            TurnOnAirSpaceMode();
            if (RequestGiveSkyriderNewDestination() == FALSE) {
              // not allowed to change destination of the helicopter
              return;
            }
          }
        }

        // select this character as the one plotting strategic movement
        bSelectedDestChar = (int8_t)iValue;

        // remember the current paths for all selected characters so we can restore them if need be
        RememberPreviousPathForAllSelectedChars();

        // check each person in this mvt group, if any bleeding, have them complain
        // CheckMembersOfMvtGroupAndComplainAboutBleeding( &( Menptr[ gCharactersList[
        // bSelectedDestChar ].usSolID ] ) );

        // highlight
        giDestHighLine = iValue;

        // can't assign highlight line
        giAssignHighLine = -1;

        // reset assign character
        bSelectedAssignChar = -1;

        // reset contract char
        bSelectedContractChar = -1;
        giContractHighLine = -1;

        // dirty team and map regions
        fTeamPanelDirty = TRUE;
        MarkForRedrawalStrategicMap();
        gfRenderPBInterface = TRUE;

        // set cursor
        SetUpCursorForStrategicMap();
      } else  // problem - this guy can't move
      {
        // cancel destination highlight
        giDestHighLine = -1;
      }
    } else  // empty char list slot
    {
      // reset selected characters
      ResetAllSelectedCharacterModes();
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    MakeMapModesSuitableForDestPlotting((int8_t)iValue);

    // reset list if the clicked character isn't also selected
    ChangeSelectedInfoChar((int8_t)iValue,
                           (BOOLEAN)(IsEntryInSelectedListSet((int8_t)iValue) == FALSE));

    CancelPathsOfAllSelectedCharacters();

    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void TeamListDestinationRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = -1;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    if (gCharactersList[iValue].fValid == TRUE) {
      giHighLine = iValue;
      giDestHighLine = iValue;
    } else {
      // can't highlight line
      giHighLine = -1;
      giDestHighLine = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    giHighLine = -1;

    if (bSelectedDestChar == -1) {
      giDestHighLine = -1;
    }

    // restore background
    RestoreBackgroundForDestinationGlowRegionList();
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (gCharactersList[iValue].fValid == TRUE) {
      // play click
      PlayGlowRegionSound();
    }
  }
}

void TeamListSleepRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // set to new info character...make sure is valid.. not in transit and alive and concious

    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if ((gCharactersList[iValue].fValid == TRUE)) {
      if (HandleCtrlOrShiftInTeamPanel((int8_t)iValue)) {
        return;
      }

      // reset list if the clicked character isn't also selected
      ChangeSelectedInfoChar((int8_t)iValue,
                             (BOOLEAN)(IsEntryInSelectedListSet((int8_t)iValue) == FALSE));

      // if this slot's sleep status can be changed
      if (CanChangeSleepStatusForCharSlot((int8_t)iValue)) {
        pSoldier = GetSoldierByID(gCharactersList[iValue].usSolID);

        if (pSoldier->fMercAsleep == TRUE) {
          // try to wake him up
          if (SetMercAwake(pSoldier, TRUE, FALSE)) {
            // propagate
            HandleSelectedMercsBeingPutAsleep(TRUE, TRUE);
            return;
          } else {
            HandleSelectedMercsBeingPutAsleep(TRUE, FALSE);
          }
        } else  // awake
        {
          // try to put him to sleep
          if (SetMercAsleep(pSoldier, TRUE)) {
            // propagate
            HandleSelectedMercsBeingPutAsleep(FALSE, TRUE);
            return;
          } else {
            HandleSelectedMercsBeingPutAsleep(FALSE, FALSE);
          }
        }
      }
    } else {
      // reset selected characters
      ResetAllSelectedCharacterModes();
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void TeamListSleepRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = -1;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    if (gCharactersList[iValue].fValid == TRUE) {
      giHighLine = iValue;

      if (CanChangeSleepStatusForCharSlot((int8_t)iValue)) {
        giSleepHighLine = iValue;
      } else {
        giSleepHighLine = -1;
      }
    } else {
      // can't highlight line
      giHighLine = -1;
      giSleepHighLine = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    giHighLine = -1;
    giSleepHighLine = -1;

    // restore background
    RestoreBackgroundForSleepGlowRegionList();
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (CanChangeSleepStatusForCharSlot((int8_t)iValue)) {
      // play click
      PlayGlowRegionSound();
    }
  }
}

void TeamListContractRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = 0;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (gCharactersList[iValue].fValid == TRUE) {
    if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      // select ONLY this dude
      ChangeSelectedInfoChar((int8_t)iValue, TRUE);

      // reset character
      giDestHighLine = -1;
      bSelectedAssignChar = -1;
      bSelectedDestChar = -1;
      bSelectedContractChar = -1;
      fPlotForHelicopter = FALSE;

      fTeamPanelDirty = TRUE;
    }

    ContractRegionBtnCallback(pRegion, iReason);
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void TeamListContractRegionMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iValue = -1;

  if (fLockOutMapScreenInterface || gfPreBattleInterfaceActive) {
    return;
  }

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_MOVE) {
    if (gCharactersList[iValue].fValid == TRUE) {
      giHighLine = iValue;

      if (CanExtendContractForCharSlot((int8_t)iValue)) {
        giContractHighLine = iValue;
      } else {
        giContractHighLine = -1;
      }
    } else {
      // can't highlight line
      giHighLine = -1;
      giContractHighLine = -1;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    giHighLine = -1;

    // no longer valid char?...reset display of highlight boxes
    if (fShowContractMenu == FALSE) {
      giContractHighLine = -1;
    }

    // restore background
    RestoreBackgroundForContractGlowRegionList();
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (CanExtendContractForCharSlot((int8_t)iValue)) {
      // play click
      PlayGlowRegionSound();
    }
  }
}

int32_t GetIndexForThisSoldier(struct SOLDIERTYPE *pSoldier) {
  int32_t iLastGuy;
  int32_t iIndex = 0;
  int32_t iCounter = 0;

  // get the index into the characters list for this soldier type
  iLastGuy = gTacticalStatus.Team[OUR_TEAM].bLastID;

  for (iCounter = 0; iCounter < iLastGuy; iCounter++) {
    if (IsCharListEntryValid(iCounter)) {
      if ((GetMercFromCharacterList(iCounter)) == pSoldier) {
        iIndex = iCounter;
        iCounter = iLastGuy;
      }
    }
  }
  return (iIndex);
}

BOOLEAN IsCursorWithInRegion(int16_t sLeft, int16_t sRight, int16_t sTop, int16_t sBottom) {
  struct Point MousePos = GetMousePoint();

  // is it within region?

  if ((sLeft < MousePos.x) && (sRight > MousePos.x) && (sTop < MousePos.y) &&
      (sBottom > MousePos.y)) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void HandleHighLightingOfLinesInTeamPanel(void) {
  if (fShowInventoryFlag) {
    return;
  }

  // will highlight or restore backgrounds to highlighted lines

  // restore backgrounds, if need be
  RestoreBackgroundForAssignmentGlowRegionList();
  RestoreBackgroundForDestinationGlowRegionList();
  RestoreBackgroundForContractGlowRegionList();
  RestoreBackgroundForSleepGlowRegionList();

  HighLightAssignLine();
  HighLightDestLine();
  HighLightSleepLine();

  // contracts?
  if (giContractHighLine != -1) {
    ContractListRegionBoxGlow((uint16_t)giContractHighLine);
  }
}

void PlotPermanentPaths(void) {
  if (fPlotForHelicopter == TRUE) {
    DisplayHelicopterPath();
  } else if (bSelectedDestChar != -1) {
    DisplaySoldierPath(GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID));
  }
}

void PlotTemporaryPaths(void) {
  uint8_t sMapX, sMapY;

  // check to see if we have in fact moved are are plotting a path?
  if (GetMouseMapXY(&sMapX, &sMapY)) {
    if (fPlotForHelicopter == TRUE) {
      Assert(fShowAircraftFlag == TRUE);
      /*
                              if( FALSE )
                              {
                                      sMapX =  ( int16_t )( ( ( iZoomX ) / ( WORLD_MAP_X ) ) + sMapX
         ); sMapX /= 2;

                                      sMapY =  ( int16_t )( ( ( iZoomY ) / ( WORLD_MAP_X ) ) + sMapY
         ); sMapY /= 2;
                              }
      */

      // plot temp path
      PlotATemporaryPathForHelicopter(sMapX, sMapY);

      // check if potential path is allowed
      DisplayThePotentialPathForHelicopter(sMapX, sMapY);

      if (fDrawTempHeliPath == TRUE) {
        // clip region
        ClipBlitsToMapViewRegion();
        // display heli temp path
        DisplayHelicopterTempPath();
        // restore
        RestoreClipRegionToFullScreen();
      }
    } else
      // dest char has been selected,
      if (bSelectedDestChar != -1) {
        /*
                                if( FALSE )
                                {
                                        sMapX =  ( int16_t )( ( ( iZoomX ) / ( MAP_GRID_X ) ) +
           sMapX
           ); sMapX /= 2;

                                        sMapY =  ( int16_t )( ( ( iZoomY ) / ( MAP_GRID_Y ) ) +
           sMapY
           ); sMapY /= 2;
                                }
        */

        PlotATemporaryPathForCharacter(GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID),
                                       sMapX, sMapY);

        // check to see if we are drawing path
        DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface(sMapX, sMapY);

        // if we need to draw path, do it
        if (fDrawTempPath == TRUE) {
          // clip region
          ClipBlitsToMapViewRegion();
          // blit
          DisplaySoldierTempPath(GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID));
          // restore
          RestoreClipRegionToFullScreen();
        }
      }
  }
}

void RenderMapRegionBackground(void) {
  // renders to save buffer when dirty flag set

  if (fMapPanelDirty == FALSE) {
    gfMapPanelWasRedrawn = FALSE;

    // not dirty, leave
    return;
  }

  // don't bother if showing sector inventory instead of the map!!!
  if (!fShowMapInventoryPool) {
    // draw map
    DrawMap();
  }

  // blit in border
  RenderMapBorder();

  if (ghAttributeBox != -1) {
    ForceUpDateOfBox(ghAttributeBox);
  }

  if (ghTownMineBox != -1) {
    // force update of town mine info boxes
    ForceUpDateOfBox(ghTownMineBox);
  }

  MapscreenMarkButtonsDirty();

  RestoreExternBackgroundRect(261, 0, 640 - 261, 359);

  // don't bother if showing sector inventory instead of the map!!!
  if (!fShowMapInventoryPool) {
    // if Skyrider can and wants to talk to us
    if (IsHelicopterPilotAvailable()) {
      // see if Skyrider has anything new to tell us
      CheckAndHandleSkyriderMonologues();
    }
  }

  // reset dirty flag
  fMapPanelDirty = FALSE;

  gfMapPanelWasRedrawn = TRUE;

  return;
}

void RenderTeamRegionBackground(void) {
  struct VObject *hHandle;

  // renders to save buffer when dirty flag set
  if (fTeamPanelDirty == FALSE) {
    // not dirty, leave
    return;
  }

  // show inventory or the team list?
  if (fShowInventoryFlag == FALSE) {
    GetVideoObject(&hHandle, guiCHARLIST);
    BltVideoObject(vsSaveBuffer, hHandle, 0, PLAYER_INFO_X, PLAYER_INFO_Y);
  } else {
    BltCharInvPanel();
  }

  if (!fShowInventoryFlag) {
    // if we are not in inventory mode, show character list
    HandleHighLightingOfLinesInTeamPanel();

    DisplayCharacterList();
  }

  fDrawCharacterList = FALSE;

  // display arrows by selected people
  HandleDisplayOfSelectedMercArrows();
  DisplayIconsForMercsAsleep();

  // reset dirty flag
  fTeamPanelDirty = FALSE;
  gfRenderPBInterface = TRUE;

  // mark all pop ups as dirty
  MarkAllBoxesAsAltered();

  // restore background for area
  RestoreExternBackgroundRect(0, 107, 261 - 0, 359 - 107);

  MapscreenMarkButtonsDirty();

  return;
}

void RenderCharacterInfoBackground(void) {
  struct VObject *hHandle;

  // will render the background for the character info panel

  if (fCharacterInfoPanelDirty == FALSE) {
    // not dirty, leave
    return;
  }

  // the upleft hand corner character info panel
  GetVideoObject(&hHandle, guiCHARINFO);
  BltVideoObject(vsSaveBuffer, hHandle, 0, TOWN_INFO_X, TOWN_INFO_Y);

  UpdateHelpTextForMapScreenMercIcons();

  if ((bSelectedInfoChar != -1) && (fDisableDueToBattleRoster == FALSE)) {
    // valid char to display
    DisplayCharacterInfo();
  }

  if ((fDisableDueToBattleRoster == FALSE)) {
    // draw attributes
    RenderAttributeStringsForUpperLeftHandCorner(vsSaveBuffer);
  }

  // reset dirty flag
  fCharacterInfoPanelDirty = FALSE;

  // redraw face
  fReDrawFace = TRUE;

  MapscreenMarkButtonsDirty();

  // mark all pop ups as dirty
  MarkAllBoxesAsAltered();

  // restore background for area
  RestoreExternBackgroundRect(0, 0, 261, 107);
}

void DetermineIfContractMenuCanBeShown(void) {
  if (fShowContractMenu == FALSE) {
    // destroy menus for contract region
    CreateDestroyMouseRegionsForContractMenu();

    // hide all boxes
    HideBox(ghContractBox);

    // make sure, absolutly sure we want to hide this box
    if (fShowAssignmentMenu == FALSE) {
      HideBox(ghRemoveMercAssignBox);
    }

    return;
  }

  // create mask, if needed
  CreateDestroyScreenMaskForAssignmentAndContractMenus();

  // create mouse regions for contract region
  CreateDestroyMouseRegionsForContractMenu();

  // determine which lines selectable
  HandleShadingOfLinesForContractMenu();

  if (Menptr[gCharactersList[bSelectedInfoChar].usSolID].bLife == 0) {
    // show basic assignment menu
    ShowBox(ghRemoveMercAssignBox);
  } else {
    // show basic contract menu
    ShowBox(ghContractBox);
  }
}

void CheckIfPlottingForCharacterWhileAirCraft(void) {
  // if we are in aircraft mode and plotting for character, reset plotting character

  if (fShowAircraftFlag == TRUE) {
    // if plotting, but not for heli
    if ((bSelectedDestChar != -1) && (fPlotForHelicopter == FALSE)) {
      // abort
      AbortMovementPlottingMode();
    }
  } else  // not in airspace mode
  {
    if (fPlotForHelicopter == TRUE) {
      // abort
      AbortMovementPlottingMode();
    }
  }
}

void ContractRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  struct SOLDIERTYPE *pSoldier = NULL;

  // btn callback handler for contract region

  if ((iDialogueBox != -1)) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if (CanExtendContractForCharSlot(bSelectedInfoChar)) {
      pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];

      // create
      RebuildContractBoxForMerc(pSoldier);

      // reset selected characters
      ResetAllSelectedCharacterModes();

      bSelectedContractChar = bSelectedInfoChar;
      giContractHighLine = bSelectedContractChar;

      // if not triggered internally
      if (CheckIfSalaryIncreasedAndSayQuote(pSoldier, TRUE) == FALSE) {
        // show contract box
        fShowContractMenu = TRUE;

        // stop any active dialogue
        StopAnyCurrentlyTalkingSpeech();
      }

      // fCharacterInfoPanelDirty = TRUE;
    } else {
      // reset selected characters
      ResetAllSelectedCharacterModes();
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void ContractRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for contract region
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    if (fGlowContractRegion == TRUE) {
      // not showing box and lost mouse?..stop glowing
      if (fShowContractMenu == FALSE) {
        fGlowContractRegion = FALSE;
        fCharacterInfoPanelDirty = TRUE;
        giContractHighLine = -1;

        // reset glow
        fResetContractGlow = TRUE;
      }
    }

  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (bSelectedInfoChar != -1) {
      if (gCharactersList[bSelectedInfoChar].fValid == TRUE) {
        if (fShowContractMenu == FALSE) {
          // glow region
          fGlowContractRegion = TRUE;

          giContractHighLine = bSelectedInfoChar;

          PlayGlowRegionSound();
        }
      }
    }
  }
}

void HandleShadingOfLinesForContractMenu(void) {
  struct SOLDIERTYPE *pSoldier;
  MERCPROFILESTRUCT *pProfile;

  if ((fShowContractMenu == FALSE) || (ghContractBox == -1)) {
    return;
  }

  // error check, return if not a valid character
  if (bSelectedContractChar == -1) {
    return;
  }

  // check if this character is a valid character
  if (gCharactersList[bSelectedContractChar].fValid == FALSE) {
    return;
  }

  Assert(CanExtendContractForCharSlot(bSelectedContractChar));

  // grab the character
  pSoldier = GetSoldierByID(gCharactersList[bSelectedContractChar].usSolID);

  // is guy in AIM? and well enough to talk and make such decisions?
  if ((pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) && (pSoldier->bLife >= OKLIFE)) {
    pProfile = &(gMercProfiles[GetSolProfile(pSoldier)]);

    // one day
    if (pProfile->sSalary > MoneyGetBalance()) {
      ShadeStringInBox(ghContractBox, CONTRACT_MENU_DAY);
    } else {
      UnShadeStringInBox(ghContractBox, CONTRACT_MENU_DAY);
    }

    // one week
    if ((int32_t)(pProfile->uiWeeklySalary) > MoneyGetBalance()) {
      ShadeStringInBox(ghContractBox, CONTRACT_MENU_WEEK);
    } else {
      UnShadeStringInBox(ghContractBox, CONTRACT_MENU_WEEK);
    }

    // two weeks
    if ((int32_t)(pProfile->uiBiWeeklySalary) > MoneyGetBalance()) {
      ShadeStringInBox(ghContractBox, CONTRACT_MENU_TWO_WEEKS);
    } else {
      UnShadeStringInBox(ghContractBox, CONTRACT_MENU_TWO_WEEKS);
    }
  } else {
    // can't extend contract duration
    ShadeStringInBox(ghContractBox, CONTRACT_MENU_DAY);
    ShadeStringInBox(ghContractBox, CONTRACT_MENU_WEEK);
    ShadeStringInBox(ghContractBox, CONTRACT_MENU_TWO_WEEKS);
  }

  // if THIS soldier is involved in a fight (dismissing in a hostile sector IS ok...)
  if ((gTacticalStatus.uiFlags & INCOMBAT) && pSoldier->bInSector) {
    ShadeStringInBox(ghContractBox, CONTRACT_MENU_TERMINATE);
  } else {
    UnShadeStringInBox(ghContractBox, CONTRACT_MENU_TERMINATE);
  }
}

void ReBuildCharactersList(void) {
  // rebuild character's list
  int16_t sCount = 0;

  // add in characters
  for (sCount = 0; sCount < MAX_CHARACTER_COUNT; sCount++) {
    // clear this slot
    gCharactersList[sCount].fValid = FALSE;
    gCharactersList[sCount].usSolID = 0;
  }

  for (sCount = 0; sCount < MAX_CHARACTER_COUNT; sCount++) {
    // add character into the cleared slot
    AddCharacter(GetSoldierByID(gTacticalStatus.Team[OUR_TEAM].bFirstID + sCount));
  }

  // sort them according to current sorting method
  SortListOfMercsInTeamPanel(FALSE);

  // if nobody is selected, or the selected merc has somehow become invalid
  if ((bSelectedInfoChar == -1) || (!gCharactersList[bSelectedInfoChar].fValid)) {
    // is the first character in the list valid?
    if (gCharactersList[0].fValid) {
      // select him
      ChangeSelectedInfoChar(0, TRUE);
    } else {
      // select no one
      ChangeSelectedInfoChar(-1, TRUE);
    }
  }

  // exit inventory mode
  fShowInventoryFlag = FALSE;
  // switch hand region help text to "Enter Inventory"
  SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[0]);
}

void HandleChangeOfInfoChar(void) {
  static int8_t bOldInfoChar = -1;

  if (bSelectedInfoChar != bOldInfoChar) {
    // set auto faces inactive

    // valid character?
    if (bOldInfoChar != -1) {
      if (gCharactersList[bOldInfoChar].fValid == TRUE) {
        // set face in active
        SetAutoFaceInActiveFromSoldier(Menptr[gCharactersList[bOldInfoChar].usSolID].ubID);
      }
    }

    // stop showing contract box
    // fShowContractMenu = FALSE;

    // update old info char value
    bOldInfoChar = bSelectedInfoChar;
  }
}

void RebuildContractBoxForMerc(struct SOLDIERTYPE *pCharacter) {
  // rebuild contractbox for this merc
  RemoveBox(ghContractBox);
  ghContractBox = -1;

  // recreate
  CreateContractBox(pCharacter);

  return;
}

void TestMessageSystem(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < 300; iCounter++) {
    MapScreenMessage(FONT_MCOLOR_DKRED, MSG_INTERFACE, L"%d", iCounter);
  }
  MapScreenMessage(FONT_MCOLOR_DKRED, MSG_INTERFACE, L"%d", iCounter);

  return;
}

void EnableDisableTeamListRegionsAndHelpText(void) {
  // check if valid character here, if so, then do nothing..other wise set help text timer to a
  // gazillion
  int8_t bCharNum;

  for (bCharNum = 0; bCharNum < MAX_CHARACTER_COUNT; bCharNum++) {
    if (gCharactersList[bCharNum].fValid == FALSE) {
      // disable regions in all team list columns
      MSYS_DisableRegion(&gTeamListNameRegion[bCharNum]);
      MSYS_DisableRegion(&gTeamListAssignmentRegion[bCharNum]);
      MSYS_DisableRegion(&gTeamListLocationRegion[bCharNum]);
      MSYS_DisableRegion(&gTeamListSleepRegion[bCharNum]);
      MSYS_DisableRegion(&gTeamListDestinationRegion[bCharNum]);
      MSYS_DisableRegion(&gTeamListContractRegion[bCharNum]);
    } else {
      // always enable Name and Location regions
      MSYS_EnableRegion(&gTeamListNameRegion[bCharNum]);
      MSYS_EnableRegion(&gTeamListLocationRegion[bCharNum]);

      // valid character.  If it's a vehicle, however
      if (Menptr[gCharactersList[bCharNum].usSolID].uiStatusFlags & SOLDIER_VEHICLE) {
        // Can't change assignment for vehicles
        MSYS_DisableRegion(&gTeamListAssignmentRegion[bCharNum]);
      } else {
        MSYS_EnableRegion(&gTeamListAssignmentRegion[bCharNum]);

        // POW or dead ?
        if ((Menptr[gCharactersList[bCharNum].usSolID].bAssignment == ASSIGNMENT_POW) ||
            (Menptr[gCharactersList[bCharNum].usSolID].bLife == 0)) {
          // "Remove Merc"
          SetRegionFastHelpText(&gTeamListAssignmentRegion[bCharNum], pRemoveMercStrings[0]);

          SetRegionFastHelpText(&gTeamListDestinationRegion[bCharNum], L"");
        } else {
          // "Assign Merc"
          SetRegionFastHelpText(&gTeamListAssignmentRegion[bCharNum],
                                pMapScreenMouseRegionHelpText[1]);
          // "Plot Travel Route"
          SetRegionFastHelpText(&gTeamListDestinationRegion[bCharNum],
                                pMapScreenMouseRegionHelpText[2]);
        }
      }

      if (CanExtendContractForCharSlot(bCharNum)) {
        MSYS_EnableRegion(&gTeamListContractRegion[bCharNum]);
      } else {
        MSYS_DisableRegion(&gTeamListContractRegion[bCharNum]);
      }

      if (CanChangeSleepStatusForCharSlot(bCharNum)) {
        MSYS_EnableRegion(&gTeamListSleepRegion[bCharNum]);
      } else {
        MSYS_DisableRegion(&gTeamListSleepRegion[bCharNum]);
      }

      // destination region is always enabled for all valid character slots.
      // if the character can't move at this time, then the region handler must be able to tell the
      // player why not
      MSYS_EnableRegion(&gTeamListDestinationRegion[bCharNum]);
    }
  }
}

void ResetAllSelectedCharacterModes(void) {
  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();
    return;
  }

  // if in militia redistribution popup
  if (sSelectedMilitiaTown != 0) {
    sSelectedMilitiaTown = 0;
  }

  // cancel destination line highlight
  giDestHighLine = -1;

  // cancel assign line highlight
  giAssignHighLine = -1;

  // unhilight contract line
  giContractHighLine = -1;

  // if we were plotting movement
  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    AbortMovementPlottingMode();
  }

  // reset assign character
  bSelectedAssignChar = -1;

  // reset contract character
  bSelectedContractChar = -1;

  // reset map cursor to normal
  if (!gfFadeOutDone && !gfFadeIn) {
    SetUpCursorForStrategicMap();
  }

  return;
}

void UpdatePausedStatesDueToTimeCompression(void) {
  // this executes every frame, so keep it optimized for speed!

  // if time is being compressed
  if (IsTimeBeingCompressed()) {
    // but it shouldn't be
    if (!AllowedToTimeCompress()) {
      // pause game to (temporarily) stop time compression
      PauseGame();
    }
  } else  // time is NOT being compressed
  {
    // but the player would like it to be compressing
    if (IsTimeCompressionOn() && !gfPauseDueToPlayerGamePause) {
      // so check if it's legal to start time compressing again
      if (AllowedToTimeCompress()) {
        // unpause game to restart time compresssion
        UnPauseGame();
      }
    }
  }

  return;
}

BOOLEAN ContinueDialogue(struct SOLDIERTYPE *pSoldier, BOOLEAN fDone) {
  // continue this grunts dialogue, restore when done
  static BOOLEAN fTalkingingGuy = FALSE;

  int8_t bCounter = 0;

  if (fDone == TRUE) {
    if (fTalkingingGuy == TRUE) {
      fCharacterInfoPanelDirty = TRUE;
      fTalkingingGuy = FALSE;
    }

    return (TRUE);
  }

  // check if valid character talking?
  if (pSoldier == NULL) {
    return FALSE;
  }

  // otherwise, find this character
  for (bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++) {
    if (gCharactersList[bCounter].fValid == TRUE) {
      if ((GetSoldierByID(gCharactersList[bCounter].usSolID)) == pSoldier) {
        if (bSelectedInfoChar != bCounter) {
          ChangeSelectedInfoChar(bCounter, TRUE);
        }
        fTalkingingGuy = TRUE;
        return (FALSE);
      }
    }
  }

  return (FALSE);
}

void HandleSpontanousTalking() {
  // simply polls if the talking guy is done, if so...send an end command to continue dialogue

  if (DialogueActive() == FALSE) {
    if ((bSelectedInfoChar != -1) && (bSelectedInfoChar < MAX_CHARACTER_COUNT)) {
      ContinueDialogue((GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID)), TRUE);
    }
  }

  return;
}

BOOLEAN CheckIfClickOnLastSectorInPath(uint8_t sX, uint8_t sY) {
  struct path **ppMovePath = NULL;
  BOOLEAN fLastSectorInPath = FALSE;
  int32_t iVehicleId = -1;
  struct path *pPreviousMercPath = NULL;

  // see if we have clicked on the last sector in the characters path

  // check if helicopter
  if (fPlotForHelicopter == TRUE) {
    if (GetSectorID16(sX, sY) == GetLastSectorOfHelicoptersPath()) {
      // helicopter route confirmed - take off
      TakeOffHelicopter();

      // rebuild waypoints - helicopter
      ppMovePath = &(pVehicleList[iHelicopterVehicleId].pMercPath);
      RebuildWayPointsForGroupPath(*ppMovePath, pVehicleList[iHelicopterVehicleId].ubMovementGroup);

      // pointer to previous helicopter path
      pPreviousMercPath = gpHelicopterPreviousMercPath;

      fLastSectorInPath = TRUE;
    }
  } else  // not doing helicopter movement
  {
    // if not doing a soldier either, we shouldn't be here!
    if (bSelectedDestChar == -1) {
      return (FALSE);
    }

    // invalid soldier?  we shouldn't be here!
    if (gCharactersList[bSelectedDestChar].fValid == FALSE) {
      bSelectedDestChar = -1;
      return (FALSE);
    }

    if (GetSectorID16(sX, sY) == GetLastSectorIdInCharactersPath((
                                     GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID)))) {
      // clicked on last sector, reset plotting mode

      // if he's IN a vehicle or IS a vehicle
      if ((Menptr[gCharactersList[bSelectedDestChar].usSolID].bAssignment == VEHICLE) ||
          (Menptr[gCharactersList[bSelectedDestChar].usSolID].uiStatusFlags & SOLDIER_VEHICLE)) {
        if (Menptr[gCharactersList[bSelectedDestChar].usSolID].bAssignment == VEHICLE) {
          // IN a vehicle
          iVehicleId = Menptr[gCharactersList[bSelectedDestChar].usSolID].iVehicleId;
        } else {
          // IS a vehicle
          iVehicleId = Menptr[gCharactersList[bSelectedDestChar].usSolID].bVehicleID;
        }

        // rebuild waypoints - vehicles
        ppMovePath = &(pVehicleList[iVehicleId].pMercPath);
      } else {
        // rebuild waypoints - mercs on foot
        ppMovePath = &(Menptr[gCharactersList[bSelectedDestChar].usSolID].pMercPath);
      }

      RebuildWayPointsForAllSelectedCharsGroups();

      // pointer to previous character path
      pPreviousMercPath = gpCharacterPreviousMercPath[bSelectedDestChar];

      fLastSectorInPath = TRUE;
    }
  }

  // if the click was over the last sector
  if (fLastSectorInPath) {
    // route has been confirmed
    EndConfirmMapMoveMode();

    // if we really did plot a path (this will skip message if left click on current sector with no
    // path)
    if (GetLengthOfPath(*ppMovePath) > 0) {
      // then verbally confirm this destination!
      HandleNewDestConfirmation(sX, sY);
    } else  // NULL path confirmed
    {
      // if previously there was a path
      if (pPreviousMercPath != NULL) {
        // then this means we've CANCELED it
        MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[3]);
      } else  // no previous path
      {
        // then it means the route was UNCHANGED
        MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[2]);
      }
    }
  }

  return (fLastSectorInPath);
}

void RebuildWayPointsForAllSelectedCharsGroups(void) {
  // rebuild the waypoints for everyone in the selected character list
  int32_t iCounter = 0;
  BOOLEAN fGroupIDRebuilt[256];
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iVehicleId;
  struct path **ppMovePath = NULL;
  uint8_t ubGroupId;

  memset(fGroupIDRebuilt, FALSE, sizeof(fGroupIDRebuilt));

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharSelected(iCounter)) {
      pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

      // if he's IN a vehicle or IS a vehicle
      if ((GetSolAssignment(pSoldier) == VEHICLE) || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
        if (GetSolAssignment(pSoldier) == VEHICLE) {
          // IN a vehicle
          iVehicleId = pSoldier->iVehicleId;
        } else {
          // IS a vehicle
          iVehicleId = pSoldier->bVehicleID;
        }

        // vehicles
        ppMovePath = &(pVehicleList[iVehicleId].pMercPath);
        ubGroupId = pVehicleList[iVehicleId].ubMovementGroup;
      } else {
        // mercs on foot
        ppMovePath = &(Menptr[gCharactersList[bSelectedDestChar].usSolID].pMercPath);
        ubGroupId = pSoldier->ubGroupID;
      }

      // if we haven't already rebuilt this group
      if (!fGroupIDRebuilt[ubGroupId]) {
        // rebuild it now
        RebuildWayPointsForGroupPath(*ppMovePath, ubGroupId);

        // mark it as rebuilt
        fGroupIDRebuilt[ubGroupId] = TRUE;
      }
    }
  }
}

void UpdateCursorIfInLastSector(void) {
  uint8_t sMapX = 0, sMapY = 0;

  // check to see if we are plotting a path, if so, see if we are highlighting the last sector int
  // he path, if so, change the cursor
  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    GetMouseMapXY(&sMapX, &sMapY);

    if (fShowAircraftFlag == FALSE) {
      if (bSelectedDestChar != -1) {
        // c heck if we are in the last sector of the characters path?
        if (GetSectorID16(sMapX, sMapY) == GetLastSectorIdInCharactersPath((GetSoldierByID(
                                               gCharactersList[bSelectedDestChar].usSolID)))) {
          // set cursor to checkmark
          ChangeMapScreenMaskCursor(CURSOR_CHECKMARK);
        } else if (fCheckCursorWasSet) {
          // reset to walking guy/vehicle
          SetUpCursorForStrategicMap();
        }
      }
    } else {
      // check for helicopter
      if (fPlotForHelicopter) {
        if (GetSectorID16(sMapX, sMapY) == GetLastSectorOfHelicoptersPath()) {
          // set cursor to checkmark
          ChangeMapScreenMaskCursor(CURSOR_CHECKMARK);
        } else if (fCheckCursorWasSet) {
          // reset to walking guy/vehicle
          SetUpCursorForStrategicMap();
        }
      } else {
        // reset to walking guy/vehicle
        SetUpCursorForStrategicMap();
      }
    }
  }

  return;
}

void FaceRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // error checking, make sure someone is there
  if (bSelectedInfoChar == -1) {
    return;
  } else if (gCharactersList[bSelectedInfoChar].fValid == FALSE) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    if (gfPreBattleInterfaceActive == TRUE) {
      return;
    }

    // now stop any dialogue in progress
    StopAnyCurrentlyTalkingSpeech();
  }

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    RequestToggleMercInventoryPanel();
  }
}

void FaceRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (bSelectedInfoChar == -1) {
    fShowFaceHightLight = FALSE;
    return;
  } else if (gCharactersList[bSelectedInfoChar].fValid == FALSE) {
    fShowFaceHightLight = FALSE;
    return;
  }

  if ((iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)) {
    fShowFaceHightLight = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fShowFaceHightLight = FALSE;
  }
}

void ItemRegionBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // left AND right button are handled the same way
  if (iReason & (MSYS_CALLBACK_REASON_RBUTTON_UP | MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    RequestToggleMercInventoryPanel();
  }
}

void ItemRegionMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (!CanToggleSelectedCharInventory()) {
    fShowItemHighLight = FALSE;
    return;
  }

  if ((iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)) {
    fShowItemHighLight = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fShowItemHighLight = FALSE;
  }
}

void HandleChangeOfHighLightedLine(void) {
  static int32_t iOldHighLine;

  if (fShowInventoryFlag) {
    return;
  }

  // check if change in highlight line
  if (giHighLine != iOldHighLine) {
    iOldHighLine = giHighLine;

    if (giHighLine == -1) {
      giSleepHighLine = -1;
      giAssignHighLine = -1;
      giContractHighLine = -1;
      giSleepHighLine = -1;

      // don't do during plotting, allowing selected character to remain highlighted and their
      // destination column to glow!
      if ((bSelectedDestChar == -1) && (fPlotForHelicopter == FALSE)) {
        giDestHighLine = -1;
      }
    }

    fDrawCharacterList = TRUE;
  }
}

void UpdateTownMinePopUpDisplay(void) {
  if (gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    ForceUpDateOfBox(ghTownMineBox);
    MapscreenMarkButtonsDirty();
  }
}

void HandleCharBarRender(void) {
  // check if the panel is disbled, if so, do not render
  if ((bSelectedInfoChar != -1) && (fDisableDueToBattleRoster == FALSE)) {
    // valid character?...render
    if (gCharactersList[bSelectedInfoChar].fValid == TRUE) {
      // if( !( ( fShowContractMenu)||( fShowAssignmentMenu ) ) )
      //{
      // draw bars for them
      DrawCharBars();

      UpdateCharRegionHelpText();
      //}
    }
  }
}

// update the status of the contract box
void UpDateStatusOfContractBox(void) {
  if (fShowContractMenu == TRUE) {
    ForceUpDateOfBox(ghContractBox);

    if ((Menptr[gCharactersList[bSelectedInfoChar].usSolID].bLife == 0) ||
        (Menptr[gCharactersList[bSelectedInfoChar].usSolID].bAssignment == ASSIGNMENT_POW)) {
      ForceUpDateOfBox(ghRemoveMercAssignBox);
    }
  }

  return;
}

void DestroyTheItemInCursor() {
  // actually destroy this item
  // End Item pickup
  MAPEndItemPointer();
  gpItemPointer = NULL;
}

void TrashItemMessageBoxCallBack(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_YES) {
    // find the item and get rid of it

    DestroyTheItemInCursor();

    // reset cursor
    MSYS_ChangeRegionCursor(&gSMPanelRegion, CURSOR_NORMAL);
    SetCurrentCursorFromDatabase(CURSOR_NORMAL);

    HandleButtonStatesWhileMapInventoryActive();
  }
}

void TrashCanBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    // check if an item is in the cursor, if so, warn player
    if (gpItemPointer != NULL) {
      // set up for mapscreen
      if (gpItemPointer->ubMission) {
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, pTrashItemText[1], MAP_SCREEN, MSG_BOX_FLAG_YESNO,
                        TrashItemMessageBoxCallBack);
      } else {
        DoMapMessageBox(MSG_BOX_BASIC_STYLE, pTrashItemText[0], MAP_SCREEN, MSG_BOX_FLAG_YESNO,
                        TrashItemMessageBoxCallBack);
      }
    }
  }
}

void TrashCanMoveCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (gMPanelRegion.Cursor == EXTERN_CURSOR) {
      fShowTrashCanHighLight = TRUE;
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fShowTrashCanHighLight = FALSE;
  }
}

void MapInvDoneButtonfastHelpCall() { SetPendingNewScreen(LAPTOP_SCREEN); }

void UpdateStatusOfMapSortButtons(void) {
  int32_t iCounter = 0;
  static BOOLEAN fShownLastTime = FALSE;

  if ((gfPreBattleInterfaceActive) || fShowInventoryFlag) {
    if (fShownLastTime) {
      for (iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++) {
        HideButton(giMapSortButton[iCounter]);
      }
      if (gfPreBattleInterfaceActive) {
        HideButton(giCharInfoButton[0]);
        HideButton(giCharInfoButton[1]);
      }

      fShownLastTime = FALSE;
    }
  } else {
    if (!fShownLastTime) {
      for (iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++) {
        ShowButton(giMapSortButton[iCounter]);
      }

      ShowButton(giCharInfoButton[0]);
      ShowButton(giCharInfoButton[1]);

      fShownLastTime = TRUE;
    }
  }
}

int8_t GetLastValidCharacterInTeamPanelList(void) {
  int8_t iCounter = 0, iValue = 0;

  // run through the list and find the last valid guy in the list
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharListEntryValid(iCounter)) {
      if ((Menptr[gCharactersList[iCounter].usSolID].bLife >= OKLIFE)) {
        if (fShowMapInventoryPool) {
          if (Menptr[gCharactersList[iCounter].usSolID].sSectorX == sSelMapX &&
              Menptr[gCharactersList[iCounter].usSolID].sSectorY == sSelMapY &&
              Menptr[gCharactersList[iCounter].usSolID].bSectorZ == (int8_t)(iCurrentMapSectorZ)) {
            iValue = iCounter;
          }
        } else {
          if (fShowInventoryFlag && (gMPanelRegion.Cursor == EXTERN_CURSOR)) {
            if (bSelectedInfoChar != -1) {
              if (gCharactersList[bSelectedInfoChar].fValid == TRUE) {
                if (Menptr[gCharactersList[iCounter].usSolID].sSectorX ==
                        Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorX &&
                    Menptr[gCharactersList[iCounter].usSolID].sSectorY ==
                        Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorY &&
                    Menptr[gCharactersList[iCounter].usSolID].bSectorZ ==
                        Menptr[gCharactersList[bSelectedInfoChar].usSolID].bSectorZ) {
                  iValue = iCounter;
                }
              }
            }
          } else {
            iValue = iCounter;
          }
        }
      }
    }
  }

  // return the character
  return (iValue);
}

/*
// NB These functions weren't being called anywhere!  Use GoToNextCharacterInList etc instead
int8_t GetPrevValidCharacterInTeamPanelList( int8_t bCurrentIndex )
{
        int8_t iCounter = bCurrentIndex, iValue = 0;

        // run through the list and find the last valid guy in the list
        for( iCounter = bCurrentIndex; iCounter > 0; iCounter-- )
        {
                if( gCharactersList[ iCounter ].fValid == TRUE )
                {
                        if( ( Menptr[ gCharactersList[ iCounter ].usSolID ].bLife >= OKLIFE ) )
                        {
                                if( fShowMapInventoryPool )
                                {
                                        if(  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX
== sSelMapX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == sSelMapY && Menptr[
gCharactersList[ iCounter ].usSolID ].bSectorZ == ( int8_t )( iCurrentMapSectorZ ) )
                                        {
                                                iValue = iCounter;
                                        }
                                }
                                else
                                {
                                        if( fShowInventoryFlag && ( gMPanelRegion.Cursor ==
EXTERN_CURSOR ) )
                                        {
                                                if( bSelectedInfoChar != -1 )
                                                {
                                                        if( gCharactersList[ bSelectedInfoChar
].fValid == TRUE )
                                                        {
                                                                if( Menptr[ gCharactersList[
iCounter ].usSolID ].sSectorX == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorX &&
Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == Menptr[ gCharactersList[ bSelectedInfoChar
].usSolID ].sSectorY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ ==Menptr[
gCharactersList[ bSelectedInfoChar ].usSolID ].bSectorZ )
                                                                {
                                                                        iValue = iCounter;
                                                                        iCounter = 0;
                                                                }
                                                        }
                                                }
                                        }
                                        else
                                        {
                                                iValue = iCounter;
                                        }
                                }
                        }
                }
        }

        // return the character
        return( iValue );
}

int8_t GetNextValidCharacterInTeamPanelList( int8_t bCurrentIndex )
{
        int8_t iCounter = bCurrentIndex, iValue = 0;

        // run through the list and find the last valid guy in the list
        for( iCounter = bCurrentIndex; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
        {
                if( gCharactersList[ iCounter ].fValid == TRUE )
                {
                        if( ( Menptr[ gCharactersList[ iCounter ].usSolID ].bLife >= OKLIFE ) )
                        {
                                if( fShowMapInventoryPool )
                                {
                                        if(  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX
== sSelMapX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == sSelMapY && Menptr[
gCharactersList[ iCounter ].usSolID ].bSectorZ == ( int8_t )( iCurrentMapSectorZ ) )
                                        {
                                                iValue = iCounter;
                                        }
                                }
                                else
                                {
                                        if( fShowInventoryFlag && ( gMPanelRegion.Cursor ==
EXTERN_CURSOR ) )
                                        {
                                                if( bSelectedInfoChar != -1 )
                                                {
                                                        if( gCharactersList[ bSelectedInfoChar
].fValid == TRUE )
                                                        {
                                                                if( Menptr[ gCharactersList[
iCounter ].usSolID ].sSectorX == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorX &&
Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == Menptr[ gCharactersList[ bSelectedInfoChar
].usSolID ].sSectorY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ ==Menptr[
gCharactersList[ bSelectedInfoChar ].usSolID ].bSectorZ )
                                                                {
                                                                        iValue = iCounter;
                                                                }
                                                        }
                                                }
                                        }
                                        else
                                        {
                                                iValue = iCounter;
                                        }
                                }
                        }
                }
        }

        // return the character
        return( iValue );
}
*/

void CreateDestroyTrashCanRegion(void) {
  static BOOLEAN fCreated = FALSE;

  if (fShowInventoryFlag && (fCreated == FALSE)) {
    fCreated = TRUE;

    // trash can
    MSYS_DefineRegion(&gTrashCanRegion, TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH,
                      TRASH_CAN_Y + TRASH_CAN_HEIGHT, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR,
                      TrashCanMoveCallback, TrashCanBtnCallback);

    // done inventory button define
    giMapInvButtonDoneImage = LoadButtonImage("INTERFACE\\done_button2.sti", -1, 0, -1, 1, -1);
    giMapInvDoneButton = QuickCreateButton(
        giMapInvButtonDoneImage, INV_BTN_X, INV_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
        (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)DoneInventoryMapBtnCallback);

    SetButtonFastHelpText(giMapInvDoneButton, pMiscMapScreenMouseRegionHelpText[2]);
    SetRegionFastHelpText(&gTrashCanRegion, pMiscMapScreenMouseRegionHelpText[1]);

    InitMapKeyRingInterface(KeyRingItemPanelButtonCallback);
    /*
                    giMapInvNextImage=  LoadButtonImage( "INTERFACE\\inventory_buttons.sti"
       ,-1,20,-1,22,-1 ); giMapInvNext= QuickCreateButton( giMapInvNextImage, ( 2 ), ( 79 ) ,
                                                                                    BUTTON_TOGGLE,
       MSYS_PRIORITY_HIGHEST - 1, ( GUI_CALLBACK )BtnGenericMouseMoveButtonCallback, (
       GUI_CALLBACK)NextInventoryMapBtnCallback );


                    giMapInvPrevImage=  LoadButtonImage( "INTERFACE\\inventory_buttons.sti"
       ,-1,21,-1,23,-1 ); giMapInvPrev= QuickCreateButton( giMapInvPrevImage, ( 30 ) , ( 79 ),
                                                                                    BUTTON_TOGGLE,
       MSYS_PRIORITY_HIGHEST - 1, ( GUI_CALLBACK )BtnGenericMouseMoveButtonCallback, (
       GUI_CALLBACK)PrevInventoryMapBtnCallback );

            */

    // reset the compatable item array at this point
    ResetCompatibleItemArray();

  } else if ((fShowInventoryFlag == FALSE) && (fCreated == TRUE)) {
    // trash can region
    fCreated = FALSE;
    MSYS_RemoveRegion(&gTrashCanRegion);

    // map inv done button
    RemoveButton(giMapInvDoneButton);

    // get rid of button image
    UnloadButtonImage(giMapInvButtonDoneImage);

    ShutdownKeyRingInterface();

    if (fShowDescriptionFlag == TRUE) {
      // kill description
      DeleteItemDescriptionBox();
    }
  }
}

void InvmaskRegionBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // CJC, December 15 1998: do NOTHING for clicks here
}

void DoneInventoryMapBtnCallback(GUI_BUTTON *btn, int32_t reason) {
  // prevent inventory from being closed while stack popup up!
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      if (gMPanelRegion.Cursor != EXTERN_CURSOR && !InItemStackPopup()) {
        fEndShowInventoryFlag = TRUE;
      }
    }
  }
}

void StartConfirmMapMoveMode(int16_t sMapY) {
  uint8_t ubPosition = (sMapY < 8) ? MSG_MAP_UI_POSITION_LOWER : MSG_MAP_UI_POSITION_UPPER;

  // tell player what to do - to click again to confirm move
  MapScreenMessage(FONT_MCOLOR_LTYELLOW, ubPosition, pMapPlotStrings[0]);

  gfInConfirmMapMoveMode = TRUE;
}

void EndConfirmMapMoveMode(void) {
  CancelMapUIMessage();

  gfInConfirmMapMoveMode = FALSE;
}

void CancelMapUIMessage(void) {
  // and kill the message overlay
  EndUIMessage();

  MarkForRedrawalStrategicMap();
}

// automatically turns off mapscreen ui overlay messages when appropriate
void MonitorMapUIMessage(void) {
  // if there is a map UI message being displayed
  if (giUIMessageOverlay != -1) {
    // and if we're not in the middle of the "confirm move" sequence
    //		if( !gfInConfirmMapMoveMode || bSelectedDestChar == -1 )
    {
      // and we've now exceed its period of maximum persistance (without user input)
      if ((GetJA2Clock() - guiUIMessageTime) > guiUIMessageTimeDelay) {
        // then cancel the message now
        CancelMapUIMessage();
      }
    }
  }
}

void HandlePreBattleInterfaceWithInventoryPanelUp(void) {
  if ((gfPreBattleInterfaceActive == TRUE) && (fShowInventoryFlag == TRUE)) {
    if (fShowDescriptionFlag == TRUE) {
      // kill description
      DeleteItemDescriptionBox();
    }

    // kill inventory panel
    fShowInventoryFlag = FALSE;
    CreateDestroyMapInvButton();
  }
}

// this puts anyone who is on NO_ASSIGNMENT onto a free squad
void UpdateBadAssignments(void) {
  uint32_t iCounter;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharListEntryValid(iCounter)) {
      CheckIfSoldierUnassigned(GetMercFromCharacterList(iCounter));
    }
  }

  return;
}

void InterruptTimeForMenus(void) {
  if ((fShowAssignmentMenu == TRUE) || (fShowContractMenu == TRUE)) {
    InterruptTime();
    PauseTimeForInterupt();
  } else if (fOneFrame) {
    InterruptTime();
    PauseTimeForInterupt();
  }

  return;
}

void HandleContractTimeFlashForMercThatIsAboutLeave(void) {
  int32_t iCurrentTime;

  // grab the current time
  iCurrentTime = GetJA2Clock();

  // only bother checking once flash interval has elapsed
  if ((iCurrentTime - giFlashContractBaseTime) >= DELAY_PER_FLASH_FOR_DEPARTING_PERSONNEL) {
    // update timer so that we only run check so often
    giFlashContractBaseTime = iCurrentTime;
    fFlashContractFlag = !fFlashContractFlag;

    // don't redraw unless we have to!
    if (AnyMercsLeavingRealSoon()) {
      // redraw character list
      fDrawCharacterList = TRUE;
    }
  }
}

BOOLEAN AnyMercsLeavingRealSoon() {
  uint32_t uiCounter = 0;
  uint32_t uiTimeInMin = GetWorldTotalMin();
  BOOLEAN fFoundOne = FALSE;

  for (uiCounter = 0; uiCounter < MAX_CHARACTER_COUNT; uiCounter++) {
    if (gCharactersList[uiCounter].fValid == TRUE) {
      if ((Menptr[gCharactersList[uiCounter].usSolID].iEndofContractTime - uiTimeInMin) <=
          MINS_TO_FLASH_CONTRACT_TIME) {
        fFoundOne = TRUE;
        break;
      }
    }
  }

  return (fFoundOne);
}

BOOLEAN HandlePreloadOfMapGraphics(void) {
  // check amt of memory, if above required amt...use it

  fPreLoadedMapGraphics = TRUE;

  vsBigMap = CreateVSurfaceFromFile("INTERFACE\\b_map.pcx");
  if (vsBigMap == NULL) {
    return FALSE;
  }

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mapcursr.sti"), &guiMAPCURSORS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\SAM.sti"), &guiSAMICON));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mapcursr.sti"), &guiMAPCURSORS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\sleepicon.sti"), &guiSleepIcon));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\charinfo.sti"), &guiCHARINFO));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\newgoldpiece3.sti"), &guiCHARLIST));

  // the sublevels
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Mine_1.sti"), &guiSubLevel1));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Mine_2.sti"), &guiSubLevel2));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Mine_3.sti"), &guiSubLevel3));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\boxes.sti"), &guiCHARICONS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\incross.sti"), &guiCROSS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mapinv.sti"), &guiMAPINV));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\map_inv_2nd_gun_cover.sti"),
                    &guiMapInvSecondHandBlockout));

  // the upper left corner piece icons
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\top_left_corner_icons.sti"), &guiULICONS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\prison.sti"), &guiTIXAICON));

  HandleLoadOfMapBottomGraphics();

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\map_item.sti"), &guiORTAICON));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mapcursr.sti"), &guiMAPCURSORS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\merc_between_sector_icons.sti"),
                    &guiCHARBETWEENSECTORICONS));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\merc_mvt_green_arrows.sti"),
                    &guiCHARBETWEENSECTORICONSCLOSE));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\GreenArr.sti"), &guiLEVELMARKER));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Helicop.sti"), &guiHelicopterIcon));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\eta_pop_up.sti"), &guiMapBorderEtaPopUp));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\pos2.sti"), &guiMapBorderHeliSectors));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\secondary_gun_hidden.sti"),
                    &guiSecItemHiddenVO));

  CHECKF(
      AddVObject(CreateVObjectFromFile("INTERFACE\\selectedchararrow.sti"), &guiSelectedCharArrow));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\mine.sti"), &guiMINEICON));

  AddVObject(CreateVObjectFromFile("INTERFACE\\hilite.sti"), &guiSectorLocatorGraphicID);

  // Kris:  Added this because I need to blink the icons button.
  AddVObject(CreateVObjectFromFile("INTERFACE\\newemail.sti"), &guiNewMailIcons);

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\BullsEye.sti"), &guiBULLSEYE));

  // graphic for pool inventory
  LoadInventoryPoolGraphic();

  // load border graphics
  LoadMapBorderGraphics();

  // load the pop up for the militia pop up box
  LoadMilitiaPopUpBox();

  return (TRUE);
}

void HandleRemovalOfPreLoadedMapGraphics(void) {
  if (fPreLoadedMapGraphics == TRUE) {
    DeleteMapBottomGraphics();
    DeleteVideoObjectFromIndex(guiMAPCURSORS);
    DeleteVideoObjectFromIndex(guiSleepIcon);
    DeleteVideoObjectFromIndex(guiCHARLIST);
    DeleteVideoObjectFromIndex(guiCHARINFO);
    DeleteVideoObjectFromIndex(guiCHARICONS);
    DeleteVideoObjectFromIndex(guiCROSS);
    DeleteVideoObjectFromIndex(guiSubLevel1);
    DeleteVideoObjectFromIndex(guiSubLevel2);
    DeleteVideoObjectFromIndex(guiSubLevel3);
    DeleteVideoObjectFromIndex(guiSAMICON);
    DeleteVideoObjectFromIndex(guiMAPINV);
    DeleteVideoObjectFromIndex(guiMapInvSecondHandBlockout);
    DeleteVideoObjectFromIndex(guiULICONS);
    DeleteVideoObjectFromIndex(guiORTAICON);
    DeleteVideoObjectFromIndex(guiTIXAICON);
    DeleteVideoObjectFromIndex(guiCHARBETWEENSECTORICONS);
    DeleteVideoObjectFromIndex(guiCHARBETWEENSECTORICONSCLOSE);
    DeleteVideoObjectFromIndex(guiLEVELMARKER);
    DeleteVideoObjectFromIndex(guiMapBorderEtaPopUp);
    DeleteVideoObjectFromIndex(guiSecItemHiddenVO);
    DeleteVideoObjectFromIndex(guiSelectedCharArrow);
    DeleteVideoObjectFromIndex(guiMapBorderHeliSectors);
    DeleteVideoObjectFromIndex(guiHelicopterIcon);
    DeleteVideoObjectFromIndex(guiMINEICON);
    DeleteVideoObjectFromIndex(guiSectorLocatorGraphicID);

    // Kris:  Remove the email icons.
    DeleteVideoObjectFromIndex(guiNewMailIcons);

    DeleteVideoObjectFromIndex(guiBULLSEYE);

    // remove the graphic for the militia pop up box
    RemoveMilitiaPopUpBox();

    // remove inventory pool graphic
    RemoveInventoryPoolGraphic();

    // get rid of border stuff
    DeleteMapBorderGraphics();
  }

  return;
}

BOOLEAN CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed(int8_t bCharId) {
  uint16_t usSoldierId = 0;

  // invalid char id
  if (bCharId == -1) {
    return (FALSE);
  }

  // valid char?
  if (gCharactersList[bCharId].fValid == FALSE) {
    return (FALSE);
  }

  // get the soldier id
  usSoldierId = gCharactersList[bCharId].usSolID;

  // char is in loaded sector
  if (Menptr[usSoldierId].sSectorX != gWorldSectorX ||
      Menptr[usSoldierId].sSectorY != gWorldSectorY ||
      Menptr[usSoldierId].bSectorZ != gbWorldSectorZ) {
    return (FALSE);
  }

  // not showing inventory?
  if (fShowInventoryFlag == FALSE) {
    // nope
    return (FALSE);
  }

  // picked something up?
  if (gMPanelRegion.Cursor != EXTERN_CURSOR) {
    // nope
    return (FALSE);
  }

  // only disallow when enemies in sector
  if (!gTacticalStatus.fEnemyInSector) {
    return (FALSE);
  }

  return (TRUE);
}

void UpdateTheStateOfTheNextPrevMapScreenCharacterButtons(void) {
  if (gfPreBattleInterfaceActive) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
    }
  } else if (bSelectedInfoChar == -1) {
    DisableButton(giCharInfoButton[0]);
    DisableButton(giCharInfoButton[1]);
    DisableButton(giMapContractButton);
  }
  /* ARM: Commented out at KM's request, it won't reenabled them when coming back from PBI, on Feb.
     22, 99 else if ( fShowInventoryFlag == FALSE ) // make sure that we are in fact showing the
     mapscreen inventory
          {
                  return;
          }
  */
  else {
    // standard checks
    if ((GetNumberOfPeopleInCharacterList() < 2) ||
        (CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed(bSelectedInfoChar)) ||
        (CharacterIsInTransitAndHasItemPickedUp(bSelectedInfoChar)) || (fShowDescriptionFlag)) {
      DisableButton(giCharInfoButton[0]);
      DisableButton(giCharInfoButton[1]);
    } else {
      EnableButton(giCharInfoButton[0]);
      EnableButton(giCharInfoButton[1]);
    }
  }
}

void PrevInventoryMapBtnCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      GoToPrevCharacterInList();
    }
  }
}

void NextInventoryMapBtnCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      GoToNextCharacterInList();
    }
  }
}

void CreateDestroyMapCharacterScrollButtons(void) {
  static BOOLEAN fCreated = FALSE;

  if ((fInMapMode == TRUE) && (fCreated == FALSE)) {
    // set the button image
    giCharInfoButtonImage[0] =
        LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 11, 4, -1, 6, -1);

    // set the button value
    giCharInfoButton[0] = QuickCreateButton(
        giCharInfoButtonImage[0], 67, 69, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
        BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)PrevInventoryMapBtnCallback);

    // set the button image
    giCharInfoButtonImage[1] =
        LoadButtonImage("INTERFACE\\map_screen_bottom_arrows.sti", 12, 5, -1, 7, -1);

    // set the button value
    giCharInfoButton[1] = QuickCreateButton(
        giCharInfoButtonImage[1], 67, 87, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
        BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)NextInventoryMapBtnCallback);

    SetButtonFastHelpText(giCharInfoButton[0], pMapScreenPrevNextCharButtonHelpText[0]);
    SetButtonFastHelpText(giCharInfoButton[1], pMapScreenPrevNextCharButtonHelpText[1]);

    fCreated = TRUE;

  } else if (((fInMapMode == FALSE)) && (fCreated == TRUE)) {
    UnloadButtonImage(giCharInfoButtonImage[0]);
    UnloadButtonImage(giCharInfoButtonImage[1]);
    RemoveButton(giCharInfoButton[0]);
    RemoveButton(giCharInfoButton[1]);

    fCreated = FALSE;
  }
}

void TellPlayerWhyHeCantCompressTime(void) {
  // if we're locked into paused time compression by some event that enforces that
  if (PauseStateLocked()) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION,
              L"(BETA) Can't compress time, pause state locked (reason %d). OK unless permanent.",
              guiLockPauseStateLastReasonId);
    ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION,
              L"(BETA) If permanent, take screenshot now, send with *previous* save & describe "
              L"what happened since.");
#endif
  } else if (gfAtLeastOneMercWasHired == FALSE) {
    // no mercs hired, ever
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapScreenJustStartedHelpText[0], MAP_SCREEN,
                    MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
  } else if (!AnyUsableRealMercenariesOnTeam()) {
    // no usable mercs left on team
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapErrorString[39], MAP_SCREEN, MSG_BOX_FLAG_OK,
                    MapScreenDefaultOkBoxCallback);
  } else if (ActiveTimedBombExists()) {
    // can't time compress when a bomb is about to go off!
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[2], MAP_SCREEN, MSG_BOX_FLAG_OK,
                    MapScreenDefaultOkBoxCallback);
  } else if (gfContractRenewalSquenceOn) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION,
              L"(BETA) Can't compress time while contract renewal sequence is on.");
#endif
  } else if (fDisableMapInterfaceDueToBattle) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION,
              L"(BETA) Can't compress time while disabled due to battle.");
#endif
  } else if (fDisableDueToBattleRoster) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION,
              L"(BETA) Can't compress time while in battle roster.");
#endif
  } else if (fMapInventoryItem) {
#ifdef JA2BETAVERSION
    ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION,
              L"(BETA) Can't compress time while still holding an inventory item.");
#endif
  } else if (fShowMapInventoryPool) {
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[55], MAP_SCREEN, MSG_BOX_FLAG_OK,
                    MapScreenDefaultOkBoxCallback);
  }
  // ARM: THIS TEST SHOULD BE THE LAST ONE, BECAUSE IT ACTUALLY RESULTS IN SOMETHING HAPPENING NOW.
  // KM:  Except if we are in a creature lair and haven't loaded the sector yet (no battle yet)
  else if (gTacticalStatus.uiFlags & INCOMBAT || gTacticalStatus.fEnemyInSector) {
    if (OnlyHostileCivsInSector()) {
      wchar_t str[256];
      wchar_t pSectorString[128];
      GetSectorIDString((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ,
                        pSectorString, ARR_SIZE(pSectorString), TRUE);
      swprintf(str, ARR_SIZE(str), gzLateLocalizedString[27], pSectorString);
      DoMapMessageBox(MSG_BOX_BASIC_STYLE, str, MAP_SCREEN, MSG_BOX_FLAG_OK,
                      MapScreenDefaultOkBoxCallback);
    } else {
      // The NEW non-persistant PBI is used instead of a dialog box explaining why we can't compress
      // time.
      InitPreBattleInterface(NULL, FALSE);
    }
  } else if (PlayerGroupIsInACreatureInfestedMine()) {
    DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[28], MAP_SCREEN, MSG_BOX_FLAG_OK,
                    MapScreenDefaultOkBoxCallback);
  }
}

void MapScreenDefaultOkBoxCallback(uint8_t bExitValue) {
  // yes, load the game
  if (bExitValue == MSG_BOX_RETURN_OK) {
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  }

  return;
}

void MapSortBtnCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t iValue = 0;

  // grab the button index value for the sort buttons
  iValue = MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (IsMapScreenHelpTextUp()) {
      // stop mapscreen text
      StopMapScreenHelpText();
      return;
    }

    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      ChangeCharacterListSortMethod(iValue);
    }
  }
}

void AddTeamPanelSortButtonsForMapScreen(void) {
  int32_t iCounter = 0;
  SGPFILENAME filename;
  int32_t iImageIndex[MAX_SORT_METHODS] = {0, 1, 5, 2, 3, 4};  // sleep image is out or order (last)

  GetMLGFilename(filename, MLG_GOLDPIECEBUTTONS);

  for (iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++) {
    giMapSortButtonImage[iCounter] =
        LoadButtonImage(filename, -1, iImageIndex[iCounter], -1, iImageIndex[iCounter] + 6, -1);

    // buttonmake
    giMapSortButton[iCounter] = QuickCreateButton(
        giMapSortButtonImage[iCounter], (int16_t)(gMapSortButtons[iCounter].iX),
        (int16_t)(gMapSortButtons[iCounter].iY), BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
        (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)MapSortBtnCallback);

    MSYS_SetBtnUserData(giMapSortButton[iCounter], 0, iCounter);

    SetButtonFastHelpText(giMapSortButton[iCounter], wMapScreenSortButtonHelpText[iCounter]);
  }
  return;
}

void SortListOfMercsInTeamPanel(BOOLEAN fRetainSelectedMercs) {
  int32_t iCounter = 0, iCounterA = 0;
  int16_t sEndSectorA, sEndSectorB;
  int32_t iExpiryTime, iExpiryTimeA;
  struct SOLDIERTYPE *pSelectedSoldier[MAX_CHARACTER_COUNT];
  struct SOLDIERTYPE *pCurrentSoldier = NULL;
  struct SOLDIERTYPE *pPreviousSelectedInfoChar = NULL;

  if (fRetainSelectedMercs) {
    // if we have anyone valid selected
    if ((bSelectedInfoChar != -1) && (gCharactersList[bSelectedInfoChar].fValid)) {
      pPreviousSelectedInfoChar = GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID);
    }

    for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
      // set current entry to null
      pSelectedSoldier[iCounter] = NULL;

      // is this entry even valid
      if (!IsCharListEntryValid(iCounter)) {
        continue;
      }

      // get soldier assoc. with entry
      pCurrentSoldier = GetMercFromCharacterList(iCounter);

      // check if soldier is active
      if (pCurrentSoldier->bActive == FALSE) {
        continue;
      }

      // are they selected?...
      if (IsCharSelected(iCounter)) {
        // yes, store pointer to them
        pSelectedSoldier[iCounter] = pCurrentSoldier;
      }
    }
  }

  // do the sort
  for (iCounter = 1; iCounter < FIRST_VEHICLE; iCounter++) {
    // have we gone too far
    if (!IsCharListEntryValid(iCounter)) {
      break;
    }

    switch (giSortStateForMapScreenList) {
      case (0):
        // by name
        for (iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++) {
          if (gCharactersList[iCounterA].fValid == FALSE) {
            break;
          }

          if ((wcscmp(Menptr[gCharactersList[iCounterA].usSolID].name,
                      Menptr[gCharactersList[iCounter].usSolID].name) > 0) &&
              (iCounterA < iCounter)) {
            SwapCharactersInList(iCounter, iCounterA);
          }
        }
        break;

      case (1):
        // by assignment
        for (iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++) {
          if (gCharactersList[iCounterA].fValid == FALSE) {
            break;
          }

          if ((Menptr[gCharactersList[iCounterA].usSolID].bAssignment >
               Menptr[gCharactersList[iCounter].usSolID].bAssignment) &&
              (iCounterA < iCounter)) {
            SwapCharactersInList(iCounter, iCounterA);
          } else if ((Menptr[gCharactersList[iCounterA].usSolID].bAssignment ==
                      Menptr[gCharactersList[iCounter].usSolID].bAssignment) &&
                     (iCounterA < iCounter)) {
            // same assignment

            // if it's in a vehicle
            if (Menptr[gCharactersList[iCounterA].usSolID].bAssignment == VEHICLE) {
              // then also compare vehicle IDs
              if ((Menptr[gCharactersList[iCounterA].usSolID].iVehicleId >
                   Menptr[gCharactersList[iCounter].usSolID].iVehicleId) &&
                  (iCounterA < iCounter)) {
                SwapCharactersInList(iCounter, iCounterA);
              }
            }
          }
        }
        break;

      case (2):
        // by sleep status
        for (iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++) {
          if (gCharactersList[iCounterA].fValid == FALSE) {
            break;
          }

          if ((Menptr[gCharactersList[iCounterA].usSolID].fMercAsleep == TRUE) &&
              (Menptr[gCharactersList[iCounter].usSolID].fMercAsleep == FALSE) &&
              (iCounterA < iCounter)) {
            SwapCharactersInList(iCounter, iCounterA);
          }
        }
        break;

      case (3):
        // by location

        sEndSectorA = CalcLocationValueForChar(iCounter);

        for (iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++) {
          if (gCharactersList[iCounterA].fValid == FALSE) {
            break;
          }

          sEndSectorB = CalcLocationValueForChar(iCounterA);

          if ((sEndSectorB > sEndSectorA) && (iCounterA < iCounter)) {
            SwapCharactersInList(iCounter, iCounterA);
          }
        }
        break;

      case (4):
        // by destination sector
        if (GetLengthOfMercPath(MercPtrs[gCharactersList[iCounter].usSolID]) == 0) {
          sEndSectorA = 9999;
        } else {
          sEndSectorA = GetLastSectorIdInCharactersPath(GetMercFromCharacterList(iCounter));
        }

        for (iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++) {
          if (gCharactersList[iCounterA].fValid == FALSE) {
            break;
          }

          if (GetLengthOfMercPath(MercPtrs[gCharactersList[iCounterA].usSolID]) == 0) {
            sEndSectorB = 9999;
          } else {
            sEndSectorB =
                GetLastSectorIdInCharactersPath(GetSoldierByID(gCharactersList[iCounterA].usSolID));
          }

          if ((sEndSectorB > sEndSectorA) && (iCounterA < iCounter)) {
            SwapCharactersInList(iCounter, iCounterA);
          }
        }
        break;

      case (5):
        iExpiryTime = GetContractExpiryTime(&(Menptr[gCharactersList[iCounter].usSolID]));

        // by contract expiry
        for (iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++) {
          if (gCharactersList[iCounterA].fValid == FALSE) {
            break;
          }

          iExpiryTimeA = GetContractExpiryTime(&(Menptr[gCharactersList[iCounterA].usSolID]));

          if ((iExpiryTimeA > iExpiryTime) && (iCounterA < iCounter)) {
            SwapCharactersInList(iCounter, iCounterA);
          }
        }
        break;

      default:
        Assert(0);
        return;
    }
  }

  if (fRetainSelectedMercs) {
    // select nobody & reset the selected list
    ChangeSelectedInfoChar(-1, TRUE);

    // now select all the soldiers that were selected before
    for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
      if (pSelectedSoldier[iCounter]) {
        for (iCounterA = 0; iCounterA < MAX_CHARACTER_COUNT; iCounterA++) {
          // is this entry even valid
          if (gCharactersList[iCounterA].fValid == FALSE) {
            continue;
          }

          // grab current soldier
          pCurrentSoldier = GetSoldierByID(gCharactersList[iCounterA].usSolID);

          // check if soldier is active
          if (pCurrentSoldier->bActive == FALSE) {
            continue;
          }

          // this guy is selected
          if (pSelectedSoldier[iCounter] == pCurrentSoldier) {
            SetEntryInSelectedCharacterList((int8_t)iCounterA);
          }

          // update who the currently selected info guy is
          if (pPreviousSelectedInfoChar == pCurrentSoldier) {
            ChangeSelectedInfoChar((int8_t)iCounterA, FALSE);
          }
        }
      }
    }
  } else {
    // keep currently selected merc, but reset the selected list (which isn't saved/restored, that's
    // why)
    ResetSelectedListForMapScreen();
  }

  // reset blinking animations
  SetAllAutoFacesInactive();

  // dirty the screen parts affected
  fTeamPanelDirty = TRUE;
  fCharacterInfoPanelDirty = TRUE;
}

void SwapCharactersInList(int32_t iCharA, int32_t iCharB) {
  uint16_t usTempSoldID;

  // swap
  usTempSoldID = gCharactersList[iCharA].usSolID;
  gCharactersList[iCharA].usSolID = gCharactersList[iCharB].usSolID;
  gCharactersList[iCharB].usSolID = usTempSoldID;
}

void RemoveTeamPanelSortButtonsForMapScreen(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++) {
    UnloadButtonImage(giMapSortButtonImage[iCounter]);
    RemoveButton(giMapSortButton[iCounter]);

    giMapSortButtonImage[iCounter] = -1;
    giMapSortButton[iCounter] = -1;
  }
  return;
}

void HandleCommonGlowTimer() {
  int32_t iCurrentTime = 0;

  // grab the current time
  iCurrentTime = GetJA2Clock();

  // only bother checking once flash interval has elapsed
  if ((iCurrentTime - giCommonGlowBaseTime) >= GLOW_DELAY) {
    // update timer so that we only run check so often
    giCommonGlowBaseTime = iCurrentTime;

    // set flag to trigger glow higlight updates
    gfGlowTimerExpired = TRUE;
  } else {
    gfGlowTimerExpired = FALSE;
  }
}

void HandleAssignmentsDoneAndAwaitingFurtherOrders(void) {
  // run through list of grunts and handle awating further orders
  int32_t iCounter = 0, iCurrentTime = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  // update "nothing to do" flags if necessary
  if (gfReEvaluateEveryonesNothingToDo) {
    ReEvaluateEveryonesNothingToDo();
  }

  // grab the current time
  iCurrentTime = GetJA2Clock();

  // only bother checking once flash interval has elapsed
  if ((iCurrentTime - giFlashAssignBaseTime) >= ASSIGNMENT_DONE_FLASH_TIME) {
    // update timer so that we only run check so often
    giFlashAssignBaseTime = iCurrentTime;

    for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
      if (!IsCharListEntryValid(iCounter)) {
        break;
      }

      pSoldier = &(Menptr[gCharactersList[iCounter].usSolID]);

      // toggle and redraw if flash was left ON even though the flag is OFF
      if (pSoldier->fDoneAssignmentAndNothingToDoFlag || fFlashAssignDone) {
        fFlashAssignDone = !fFlashAssignDone;
        fDrawCharacterList = TRUE;

        // only need to find one
        break;
      }
    }
  }
}

void DisplayIconsForMercsAsleep(void) {
  // run throught he list of grunts to see who is asleep and who isn't
  struct VObject *hHandle;
  int32_t iCounter;
  struct SOLDIERTYPE *pSoldier;

  // if we are in inventory
  if (fShowInventoryFlag == TRUE) {
    return;
  }

  GetVideoObject(&hHandle, guiSleepIcon);

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharListEntryValid(iCounter)) {
      pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];
      if (IsSolActive(pSoldier) && pSoldier->fMercAsleep &&
          CanChangeSleepStatusForSoldier(pSoldier)) {
        BltVideoObject(vsSaveBuffer, hHandle, 0, 125,
                       (int16_t)(Y_START + (iCounter * (Y_SIZE + 2))));
      }
    }
  }
  return;
}

// Kris:  Added this function to blink the email icon on top of the laptop button whenever we are in
//       mapscreen and we have new email to read.
void CheckForAndRenderNewMailOverlay() {
  if (fNewMailFlag) {
    if (GetJA2Clock() % 1000 < 667) {
      if (ButtonList[guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP]]->uiFlags &
          BUTTON_CLICKED_ON) {  // button is down, so offset the icon
        BltVObjectFromIndex(vsFB, guiNewMailIcons, 1, 465, 418);
        InvalidateRegion(465, 418, 480, 428);
      } else {  // button is up, so draw the icon normally
        BltVObjectFromIndex(vsFB, guiNewMailIcons, 0, 464, 417);
        if (!(ButtonList[guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP]]->uiFlags & BUTTON_ENABLED)) {
          uint32_t uiDestPitchBYTES;
          uint8_t *pDestBuf;
          SGPRect area = {463, 417, 477, 425};

          pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
          Blt16BPPBufferHatchRect((uint16_t *)pDestBuf, uiDestPitchBYTES, &area);
          JSurface_Unlock(vsFB);
        }
        InvalidateRegion(463, 417, 481, 430);
      }
    } else {  // The blink is now off, so mark the button dirty so that it'll render next frame.
      MarkAButtonDirty(guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP]);
    }
  }
}

BOOLEAN CanToggleSelectedCharInventory(void) {
  struct SOLDIERTYPE *pSoldier = NULL;

  if (gfPreBattleInterfaceActive == TRUE) {
    return (FALSE);
  }

  // already in inventory and an item picked up?
  if (fShowInventoryFlag && (gMPanelRegion.Cursor == EXTERN_CURSOR)) {
    return (FALSE);
  }

  // nobody selected?
  if (bSelectedInfoChar == -1) {
    return (FALSE);
  }

  // does the selected guy have inventory and can we get at it?
  if (!MapCharacterHasAccessibleInventory(bSelectedInfoChar)) {
    return (FALSE);
  }

  pSoldier = MercPtrs[gCharactersList[bSelectedInfoChar].usSolID];

  // if not in inventory, and holding an item from sector inventory
  if (!fShowInventoryFlag &&
      ((gMPanelRegion.Cursor == EXTERN_CURSOR) || gpItemPointer || fMapInventoryItem) &&
      (gpItemPointerSoldier == NULL)) {
    // make sure he's in that sector
    if ((GetSolSectorX(pSoldier) != sSelMapX) || (GetSolSectorY(pSoldier) != sSelMapY) ||
        (GetSolSectorZ(pSoldier) != iCurrentMapSectorZ) || pSoldier->fBetweenSectors) {
      return (FALSE);
    }
  }

  // passed!
  return (TRUE);
}

BOOLEAN MapCharacterHasAccessibleInventory(int8_t bCharNumber) {
  struct SOLDIERTYPE *pSoldier = NULL;

  Assert(bCharNumber >= 0);
  Assert(bCharNumber < MAX_CHARACTER_COUNT);

  // invalid character slot selected?
  if (gCharactersList[bCharNumber].fValid == FALSE) {
    return (FALSE);
  }

  pSoldier = MercPtrs[gCharactersList[bCharNumber].usSolID];

  if ((GetSolAssignment(pSoldier) == IN_TRANSIT) ||
      (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) ||
      (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (AM_A_ROBOT(pSoldier)) ||
      (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) || (pSoldier->bLife < OKLIFE)) {
    return (FALSE);
  }

  return (TRUE);
}

void CheckForInventoryModeCancellation() {
  if (fShowInventoryFlag || fShowDescriptionFlag) {
    // can't bail while player has an item in hand...
    if (gMPanelRegion.Cursor == EXTERN_CURSOR) {
      return;
    }

    if (!CanToggleSelectedCharInventory()) {
      // get out of inventory mode if it's on!  (could have just bled below OKLIFE)
      if (fShowInventoryFlag) {
        fShowInventoryFlag = FALSE;
        SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[0]);
        fTeamPanelDirty = TRUE;
      }

      // get out of inventory description if it's on!
      if (fShowDescriptionFlag) {
        DeleteItemDescriptionBox();
      }
    }
  }
}

void ChangeSelectedMapSector(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ) {
  // ignore while map inventory pool is showing, or else items can be replicated, since sector
  // inventory always applies only to the currently selected sector!!!
  if (fShowMapInventoryPool) return;

  if (gfPreBattleInterfaceActive) return;

  if (!IsTheCursorAllowedToHighLightThisSector(sMapX, sMapY)) return;

  // disallow going underground while plotting (surface) movement
  if ((bMapZ != 0) && ((bSelectedDestChar != -1) || fPlotForHelicopter)) return;

  sSelMapX = sMapX;
  sSelMapY = sMapY;
  iCurrentMapSectorZ = bMapZ;

  // if going underground while in airspace mode
  if ((bMapZ > 0) && (fShowAircraftFlag == TRUE)) {
    // turn off airspace mode
    ToggleAirspaceMode();
  }

  MarkForRedrawalStrategicMap();
  fMapScreenBottomDirty = TRUE;

  // also need this, to update the text coloring of mercs in this sector
  fTeamPanelDirty = TRUE;
}

BOOLEAN CanChangeDestinationForCharSlot(int8_t bCharNumber, BOOLEAN fShowErrorMessage) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int8_t bErrorNumber = -1;

  if (bCharNumber == -1) return (FALSE);

  if (gCharactersList[bCharNumber].fValid == FALSE) return (FALSE);

  pSoldier = MercPtrs[gCharactersList[bCharNumber].usSolID];

  // valid soldier?
  Assert(pSoldier);
  Assert(IsSolActive(pSoldier));

  if (CanEntireMovementGroupMercIsInMove(pSoldier, &bErrorNumber)) {
    return (TRUE);
  } else {
    // function may fail without returning any specific error # (-1).
    // if it gave us the # of an error msg, and we were told to display it
    if ((bErrorNumber != -1) && fShowErrorMessage) {
      ReportMapScreenMovementError(bErrorNumber);
    }

    return (FALSE);
  }
}

BOOLEAN CanExtendContractForCharSlot(int8_t bCharNumber) {
  struct SOLDIERTYPE *pSoldier = NULL;

  if (bCharNumber == -1) return (FALSE);

  if (gCharactersList[bCharNumber].fValid == FALSE) return (FALSE);

  pSoldier = MercPtrs[gCharactersList[bCharNumber].usSolID];

  // valid soldier?
  Assert(pSoldier);
  Assert(IsSolActive(pSoldier));

  // if a vehicle, in transit, or a POW
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (GetSolAssignment(pSoldier) == IN_TRANSIT) ||
      (GetSolAssignment(pSoldier) == ASSIGNMENT_POW)) {
    // can't extend contracts at this time
    return (FALSE);
  }

  // mercs below OKLIFE, M.E.R.C. mercs, EPCs, and the Robot use the Contract menu so they can be
  // DISMISSED/ABANDONED!

  // everything OK
  return (TRUE);
}

BOOLEAN CanChangeSleepStatusForCharSlot(int8_t bCharNumber) {
  struct SOLDIERTYPE *pSoldier = NULL;

  if (bCharNumber == -1) return (FALSE);

  if (gCharactersList[bCharNumber].fValid == FALSE) return (FALSE);

  pSoldier = MercPtrs[gCharactersList[bCharNumber].usSolID];

  return (CanChangeSleepStatusForSoldier(pSoldier));
}

BOOLEAN CanChangeSleepStatusForSoldier(struct SOLDIERTYPE *pSoldier) {
  // valid soldier?
  Assert(pSoldier);
  Assert(IsSolActive(pSoldier));

  // if a vehicle, robot, in transit, or a POW
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier) ||
      (GetSolAssignment(pSoldier) == IN_TRANSIT) ||
      (GetSolAssignment(pSoldier) == ASSIGNMENT_POW)) {
    // can't change the sleep status of such mercs
    return (FALSE);
  }

  // if dead
  if ((pSoldier->bLife <= 0) || (GetSolAssignment(pSoldier) == ASSIGNMENT_DEAD)) {
    return (FALSE);
  }

  // this merc MAY be able to sleep/wake up - we'll allow player to click and find out
  return (TRUE);
}

void ChangeMapScreenMaskCursor(uint16_t usCursor) {
  MSYS_SetCurrentCursor(usCursor);
  MSYS_ChangeRegionCursor(&gMapScreenMaskRegion, usCursor);

  if (usCursor == CURSOR_CHECKMARK)
    fCheckCursorWasSet = TRUE;
  else
    fCheckCursorWasSet = FALSE;

  if (usCursor == CURSOR_NORMAL) {
    if (!InItemStackPopup()) {
      // cancel mouse restriction
      FreeMouseCursor();
    }
  } else {
    // restrict mouse cursor to the map area
    RestrictMouseCursor(&MapScreenRect);
  }
}

void CancelOrShortenPlottedPath(void) {
  uint8_t sMapX, sMapY;
  uint32_t uiReturnValue;

  GetMouseMapXY(&sMapX, &sMapY);

  // check if we are in aircraft mode
  if (fShowAircraftFlag == TRUE) {
    // check for helicopter path being plotted
    if (!fPlotForHelicopter) return;

    // if player can't redirect it
    if (CanHelicopterFly() == FALSE) {
      // explain & ignore
      ExplainWhySkyriderCantFly();
      return;
    }

    // try to delete portion of path AFTER the current sector for the helicopter
    uiReturnValue = ClearPathAfterThisSectorForHelicopter(sMapX, sMapY);
  } else {
    // check for character path being plotted
    if (bSelectedDestChar == -1) return;

    // try to delete portion of path AFTER the current sector for the helicopter
    uiReturnValue = ClearPathAfterThisSectorForCharacter(
        GetSoldierByID(gCharactersList[bSelectedDestChar].usSolID), sMapX, sMapY);
  }

  switch (uiReturnValue) {
    case ABORT_PLOTTING:
      AbortMovementPlottingMode();
      break;

    case PATH_CLEARED:  // movement was canceled
      // message was already issued when path was cleared
      DestinationPlottingCompleted();
      break;

    case PATH_SHORTENED:  // route was shortened but isn't completely gone
      // display "route shortened" message
      MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[4]);
      break;

    default:
      Assert(FALSE);
      break;
  }

  // this triggers the path node animation to reset itself back to the first node
  fDeletedNode = TRUE;

  MarkForRedrawalStrategicMap();

  fTeamPanelDirty = TRUE;
  fCharacterInfoPanelDirty = TRUE;  // to update ETAs if path reversed or shortened
}

BOOLEAN HandleCtrlOrShiftInTeamPanel(int8_t bCharNumber) {
  // check if shift or ctrl held down, if so, set values in list
  if (_KeyDown(CTRL)) {
    ToggleEntryInSelectedList(bCharNumber);

    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;

    return (TRUE);
  } else if (_KeyDown(SHIFT)) {
    // build a list from the bSelectedInfoChar To here, reset everyone

    // empty the list
    ResetSelectedListForMapScreen();
    // rebuild the list
    BuildSelectedListFromAToB(bSelectedInfoChar, bCharNumber);

    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;

    return (TRUE);
  }

  return (FALSE);
}

int32_t GetContractExpiryTime(struct SOLDIERTYPE *pSoldier) {
  if ((pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) || (GetSolProfile(pSoldier) == SLAY)) {
    return (pSoldier->iEndofContractTime);
  } else {
    // never - really high number
    return (999999);
  }
}

void ChangeSelectedInfoChar(int8_t bCharNumber, BOOLEAN fResetSelectedList) {
  Assert((bCharNumber >= -1) && (bCharNumber < MAX_CHARACTER_COUNT));

  if ((bCharNumber != -1) && (gCharactersList[bCharNumber].fValid == FALSE)) return;

  // if holding an item
  if ((gMPanelRegion.Cursor == EXTERN_CURSOR) || gpItemPointer || fMapInventoryItem) {
    // make sure we can give it to this guy, otherwise don't allow the change
    if (!MapscreenCanPassItemToCharNum(bCharNumber)) {
      return;
    }
  }

  if (fResetSelectedList) {
    // reset selections of all listed characters.  Do this even if this guy is already selected.
    // NOTE: this keeps the currently selected info char selected
    ResetSelectedListForMapScreen();
  }

  // if this is really a change
  if (bSelectedInfoChar != bCharNumber) {
    // if resetting, and another guy was selected
    if (fResetSelectedList && (bSelectedInfoChar != -1)) {
      // deselect previously selected character
      ResetEntryForSelectedList(bSelectedInfoChar);
    }

    bSelectedInfoChar = bCharNumber;

    if (bCharNumber != -1) {
      // the selected guy must always be ON in the list of selected chars
      SetEntryInSelectedCharacterList(bCharNumber);
    }

    // if we're in the inventory panel
    if (fShowInventoryFlag) {
      // and we're changing to nobody or a guy whose inventory can't be accessed
      if ((bCharNumber == -1) || !MapCharacterHasAccessibleInventory(bCharNumber)) {
        // then get out of inventory mode
        fShowInventoryFlag = FALSE;
      }
    }

    fCharacterInfoPanelDirty = TRUE;

    // if showing sector inventory
    if (fShowMapInventoryPool) {
      // redraw right side to update item hatches
      MarkForRedrawalStrategicMap();
    }
  }

  fTeamPanelDirty = TRUE;
}

void CopyPathToAllSelectedCharacters(struct path *pPath) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  // run through list and copy paths for each selected character
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharSelected(iCounter)) {
      pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

      // skip itself!
      if (GetSoldierMercPathPtr(pSoldier) != pPath) {
        if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
          pVehicleList[pSoldier->bVehicleID].pMercPath =
              CopyPaths(pPath, pVehicleList[pSoldier->bVehicleID].pMercPath);
        } else if (GetSolAssignment(pSoldier) == VEHICLE) {
          pVehicleList[pSoldier->iVehicleId].pMercPath =
              CopyPaths(pPath, pVehicleList[pSoldier->iVehicleId].pMercPath);
        } else {
          pSoldier->pMercPath = CopyPaths(pPath, pSoldier->pMercPath);
        }

        // don't use CopyPathToCharactersSquadIfInOne(), it will whack the original pPath by
        // replacing that merc's path!
      }
    }
  }
}

void CancelPathsOfAllSelectedCharacters() {
  int8_t bCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  BOOLEAN fSkyriderMsgShown = FALSE;

  // cancel destination for the clicked and ALL other valid & selected characters with a route set
  for (bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++) {
    // if we've clicked on a selected valid character
    if ((gCharactersList[bCounter].fValid == TRUE) && IsEntryInSelectedListSet(bCounter)) {
      pSoldier = MercPtrs[gCharactersList[bCounter].usSolID];

      // and he has a route set
      if (GetLengthOfMercPath(pSoldier) > 0) {
        // if he's in the chopper, but player can't redirect it
        if ((GetSolAssignment(pSoldier) == VEHICLE) &&
            (pSoldier->iVehicleId == iHelicopterVehicleId) && (CanHelicopterFly() == FALSE)) {
          if (!fSkyriderMsgShown) {
            // explain
            ExplainWhySkyriderCantFly();
            fSkyriderMsgShown = TRUE;
          }

          // don't cancel, ignore
          continue;
        }

        // cancel the entire path (also clears vehicles for any passengers selected, and handles
        // reversing directions)
        if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
          CancelPathForVehicle(&(pVehicleList[pSoldier->bVehicleID]), FALSE);
        } else {
          CancelPathForCharacter(pSoldier);
        }
      }
    }
  }
}

void ConvertMinTimeToETADayHourMinString(uint32_t uiTimeInMin, wchar_t *sString, size_t bufSize) {
  uint32_t uiDay, uiHour, uiMin;

  uiDay = (uiTimeInMin / NUM_MIN_IN_DAY);
  uiHour = (uiTimeInMin - (uiDay * NUM_MIN_IN_DAY)) / NUM_MIN_IN_HOUR;
  uiMin = uiTimeInMin - ((uiDay * NUM_MIN_IN_DAY) + (uiHour * NUM_MIN_IN_HOUR));

  // there ain't enough room to show both the day and ETA: and without ETA it's confused as the
  // current time
  //	swprintf( sString, L"%s %s %d, %02d:%02d", pEtaString[ 0 ], pDayStrings[ 0 ], uiDay, uiHour,
  // uiMin ); 	swprintf( sString, L"%s %d, %02d:%02d", pDayStrings[ 0 ], uiDay, uiHour, uiMin );
  swprintf(sString, bufSize, L"%s %02d:%02d", pEtaString[0], uiHour, uiMin);
}

int32_t GetGroundTravelTimeOfCharacter(int8_t bCharNumber) {
  int32_t iTravelTime = 0;

  if (bCharNumber == -1) return (0);

  if (!gCharactersList[bCharNumber].fValid) return (0);

  // get travel time for the last path segment (stored in pTempCharacterPath)
  iTravelTime = GetPathTravelTimeDuringPlotting(pTempCharacterPath);

  // add travel time for any prior path segments (stored in the selected character's mercpath, but
  // waypoints aren't built)
  iTravelTime += GetPathTravelTimeDuringPlotting(
      GetSoldierMercPathPtr(MercPtrs[gCharactersList[bCharNumber].usSolID]));

  return (iTravelTime);
}

int16_t CalcLocationValueForChar(int32_t iCounter) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int16_t sLocValue = 0;

  Assert(iCounter < MAX_CHARACTER_COUNT);

  if (!IsCharListEntryValid(iCounter)) return (sLocValue);

  pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

  // don't reveal location of POWs!
  if (pSoldier->bAssignment != ASSIGNMENT_POW) {
    sLocValue = GetSolSectorID8(pSoldier);
    // underground: add 1000 per sublevel
    sLocValue += 1000 * (GetSolSectorZ(pSoldier));
  }

  return (sLocValue);
}

void CancelChangeArrivalSectorMode() {
  // "put him in change arrival sector" mode
  gfInChangeArrivalSectorMode = FALSE;

  // change the cursor to that mode
  SetUpCursorForStrategicMap();

  MarkForRedrawalStrategicMap();
}

void MakeMapModesSuitableForDestPlotting(int8_t bCharNumber) {
  struct SOLDIERTYPE *pSoldier = NULL;

  if (gCharactersList[bCharNumber].fValid == TRUE) {
    pSoldier = MercPtrs[gCharactersList[bCharNumber].usSolID];

    CancelSectorInventoryDisplayIfOn(FALSE);

    TurnOnShowTeamsMode();

    if ((GetSolAssignment(pSoldier) == VEHICLE) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      if (fShowAircraftFlag == FALSE) {
        // turn on airspace mode automatically
        ToggleAirspaceMode();
      }
    } else {
      if (fShowAircraftFlag == TRUE) {
        // turn off airspace mode automatically
        ToggleAirspaceMode();
      }
    }

    // if viewing a different sublevel
    if (iCurrentMapSectorZ != GetSolSectorZ(pSoldier)) {
      // switch to that merc's sublevel
      JumpToLevel(GetSolSectorZ(pSoldier));
    }
  }
}

BOOLEAN AnyMovableCharsInOrBetweenThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ) {
  int32_t iFirstId = 0, iLastId = 0;
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  // to speed it up a little?
  iFirstId = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  iLastId = gTacticalStatus.Team[OUR_TEAM].bLastID;

  for (iCounter = iFirstId; iCounter <= iLastId; iCounter++) {
    // get the soldier
    pSoldier = GetSoldierByID(iCounter);

    // is the soldier active
    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    // POWs, dead guys, guys in transit can't move
    if ((GetSolAssignment(pSoldier) == IN_TRANSIT) ||
        (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) ||
        (GetSolAssignment(pSoldier) == ASSIGNMENT_DEAD) || (pSoldier->bLife == 0)) {
      continue;
    }

    // don't count mercs aboard Skyrider
    if ((GetSolAssignment(pSoldier) == VEHICLE) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      continue;
    }

    // don't count vehicles - in order for them to move, somebody must be in the sector with them
    if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
      continue;
    }

    // is he here?
    if ((GetSolSectorX(pSoldier) == sSectorX) && (GetSolSectorY(pSoldier) == sSectorY) &&
        (GetSolSectorZ(pSoldier) == bSectorZ)) {
      // NOTE that we consider mercs between sectors, mercs < OKLIFE, and sleeping mercs to be
      // "movable". This lets CanCharacterMoveInStrategic() itself report the appropriate error
      // message when character is clicked
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN RequestGiveSkyriderNewDestination(void) {
  // should we allow it?
  if (CanHelicopterFly() == TRUE) {
    // if not warned already, and chopper empty, but mercs are in this sector
    if (!gfSkyriderEmptyHelpGiven && (GetNumberOfPassengersInHelicopter() == 0) &&
        (PlayerMercsInHelicopterSector() > 0)) {
      DoMapMessageBox(MSG_BOX_BASIC_STYLE, pSkyriderText[6], MAP_SCREEN, MSG_BOX_FLAG_OK,
                      MapScreenDefaultOkBoxCallback);
      gfSkyriderEmptyHelpGiven = TRUE;
      return (FALSE);
    }

    // say Yo!
    SkyRiderTalk(SKYRIDER_SAYS_HI);

    // start plotting helicopter movement
    fPlotForHelicopter = TRUE;

    // change cursor to the helicopter
    SetUpCursorForStrategicMap();

    // remember the helicopter's current path so we can restore it if need be
    gpHelicopterPreviousMercPath =
        CopyPaths(pVehicleList[iHelicopterVehicleId].pMercPath, gpHelicopterPreviousMercPath);

    // affects Skyrider's dialogue
    SetFactTrue(FACT_SKYRIDER_USED_IN_MAPSCREEN);

    return (TRUE);
  } else  // not allowed to reroute the chopper right now
  {
    // tell player why not
    ExplainWhySkyriderCantFly();

    return (FALSE);
  }
}

void ExplainWhySkyriderCantFly(void) {
  // do we owe him money?
  if (gMercProfiles[SKYRIDER].iBalance < 0) {
    // overdue cash
    SkyRiderTalk(OWED_MONEY_TO_SKYRIDER);
    return;
  }

  // is he returning to base?
  if (fHeliReturnStraightToBase) {
    // returning to base
    SkyRiderTalk(RETURN_TO_BASE);
    return;
  }

  // grounded by enemies in sector?
  if (CanHelicopterTakeOff() == FALSE) {
    SkyRiderTalk(CHOPPER_NOT_ACCESSIBLE);
    return;
  }

  // Drassen too disloyal to wanna help player?
  if (CheckFact(FACT_LOYALTY_LOW, SKYRIDER)) {
    SkyRiderTalk(DOESNT_WANT_TO_FLY);
    return;
  }

  // no explainable reason
}

uint8_t PlayerMercsInHelicopterSector(void) {
  struct GROUP *pGroup = NULL;

  Assert(iHelicopterVehicleId != -1);
  pGroup = GetGroup(pVehicleList[iHelicopterVehicleId].ubMovementGroup);

  if (pGroup->fBetweenSectors) {
    return (0);
  }

  return (PlayerMercsInSector(pGroup->ubSectorX, pGroup->ubSectorY, 0));
}

void HandleNewDestConfirmation(uint8_t sMapX, uint8_t sMapY) {
  uint8_t ubCurrentProgress;

  // if moving the chopper itself, or moving a character aboard the chopper
  if (fPlotForHelicopter) {
    // if there are no enemies in destination sector, or we don't know
    if ((NumEnemiesInSector(sMapX, sMapY) == 0) ||
        (WhatPlayerKnowsAboutEnemiesInSector(sMapX, sMapY) == KNOWS_NOTHING)) {
      // no problem

      // get current player progress
      ubCurrentProgress = CurrentPlayerProgressPercentage();

      // if we're doing a lot better than last time he said anything
      if ((ubCurrentProgress > gubPlayerProgressSkyriderLastCommentedOn) &&
          ((ubCurrentProgress - gubPlayerProgressSkyriderLastCommentedOn) >=
           MIN_PROGRESS_FOR_SKYRIDER_QUOTE_DOING_WELL)) {
        // kicking ass!
        SkyRiderTalk(THINGS_ARE_GOING_WELL);

        gubPlayerProgressSkyriderLastCommentedOn = ubCurrentProgress;
      }
      // if we're doing noticably worse than last time he said anything
      else if ((ubCurrentProgress < gubPlayerProgressSkyriderLastCommentedOn) &&
               ((gubPlayerProgressSkyriderLastCommentedOn - ubCurrentProgress) >=
                MIN_REGRESS_FOR_SKYRIDER_QUOTE_DOING_BADLY)) {
        // sucking rocks!
        SkyRiderTalk(THINGS_ARE_GOING_BADLY);

        gubPlayerProgressSkyriderLastCommentedOn = ubCurrentProgress;
      } else {
        // ordinary confirmation quote
        SkyRiderTalk(CONFIRM_DESTINATION);
      }
    } else {
      // ok, but... you know there are enemies there...
      SkyRiderTalk(BELIEVED_ENEMY_SECTOR);
    }
  } else {
    RandomAwakeSelectedMercConfirmsStrategicMove();

    // tell player the route was CONFIRMED
    // NOTE: We don't this this for the helicopter any more, since it clashes with Skyrider's own
    // confirmation msg
    MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[1]);
  }

  // wake up anybody who needs to be awake to travel
  WakeUpAnySleepingSelectedMercsOnFootOrDriving();
}

void RandomAwakeSelectedMercConfirmsStrategicMove(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iCounter;
  uint8_t ubSelectedMercID[20];
  uint8_t ubSelectedMercIndex[20];
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharSelected(iCounter)) {
      pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

      if (pSoldier->bLife >= OKLIFE && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) &&
          !AM_A_ROBOT(pSoldier) && !AM_AN_EPC(pSoldier) && !pSoldier->fMercAsleep) {
        ubSelectedMercID[ubNumMercs] = GetSolID(pSoldier);
        ubSelectedMercIndex[ubNumMercs] = (uint8_t)iCounter;

        ubNumMercs++;
      }
    }
  }

  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    // select that merc so that when he speaks we're showing his portrait and not someone else
    ChangeSelectedInfoChar(ubSelectedMercIndex[ubChosenMerc], FALSE);

    DoMercBattleSound(MercPtrs[ubSelectedMercID[ubChosenMerc]],
                      (uint8_t)(Random(2) ? BATTLE_SOUND_OK1 : BATTLE_SOUND_OK2));
    // TacticalCharacterDialogue( MercPtrs[ ubSelectedMercID[ ubChosenMerc ] ], ubQuoteNum );
  }
}

void DestinationPlottingCompleted(void) {
  // clear previous paths for selected characters and helicopter
  ClearPreviousPaths();

  fPlotForHelicopter = FALSE;
  bSelectedDestChar = -1;
  giDestHighLine = -1;

  MarkForRedrawalStrategicMap();

  // reset cursor
  SetUpCursorForStrategicMap();

  fJustFinishedPlotting = TRUE;
  fEndPlotting = TRUE;
}

void HandleMilitiaRedistributionClick(void) {
  TownID bTownId;
  BOOLEAN fTownStillHidden;
  wchar_t sString[128];

  // if on the surface
  if (iCurrentMapSectorZ == 0) {
    bTownId = GetTownIdForSector(sSelMapX, sSelMapY);
    fTownStillHidden = ((bTownId == TIXA) && !fFoundTixa) || ((bTownId == ORTA) && !fFoundOrta);

    if ((bTownId != BLANK_SECTOR) && !fTownStillHidden) {
      if (MilitiaTrainingAllowedInSector(sSelMapX, sSelMapY, (int8_t)iCurrentMapSectorZ)) {
        if (fShowTownInfo == TRUE) {
          fShowTownInfo = FALSE;
        }
        MarkForRedrawalStrategicMap();

        // check if there's combat in any of the town's sectors
        if (CanRedistributeMilitiaInSector(sSelMapX, sSelMapY, bTownId)) {
          // Nope, ok, set selected militia town
          sSelectedMilitiaTown = bTownId;
        } else {
          // can't redistribute militia during combat!
          DoScreenIndependantMessageBox(pMilitiaString[2], MSG_BOX_FLAG_OK, NULL);
        }
      } else {
        // can't have militia in this town
        swprintf(sString, ARR_SIZE(sString), pMapErrorString[31], pTownNames[bTownId]);
        DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
      }
    } else if (IsThisSectorASAMSector(sSelMapX, sSelMapY, 0) &&
               fSamSiteFound[GetSAMIdFromSector(sSelMapX, sSelMapY, 0)]) {
      // can't move militia around sam sites
      DoScreenIndependantMessageBox(pMapErrorString[30], MSG_BOX_FLAG_OK, NULL);
    }
  }
}

#ifdef JA2TESTVERSION
void DumpSectorDifficultyInfo(void) {
  // NOTE: This operates on the selected map sector!
  wchar_t wSectorName[128];

  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Playing Difficulty: %s",
            gzGIOScreenText[GIO_DIF_LEVEL_TEXT + gGameOptions.ubDifficultyLevel]);
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Highest Progress (0-100) = %d%%",
            HighestPlayerProgressPercentage());
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Player Kills = %d",
            gStrategicStatus.usPlayerKills);

  GetSectorIDString(sSelMapX, sSelMapY, (int8_t)iCurrentMapSectorZ, wSectorName,
                    ARR_SIZE(wSectorName), TRUE);
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"GetSectorID8: %s", wSectorName);

  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Pyth. Distance From Meduna (0-20) = %d",
            GetPythDistanceFromPalace(sSelMapX, sSelMapY));

  if ((gWorldSectorX == sSelMapX) && (gWorldSectorY == sSelMapY) &&
      (gbWorldSectorZ == iCurrentMapSectorZ)) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Enemy Difficulty Factor (0 to 100) = %d%%",
              CalcDifficultyModifier(SOLDIER_CLASS_ARMY));
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Avg Regular Enemy Exp. Level (2-6) = %d",
              2 + (CalcDifficultyModifier(SOLDIER_CLASS_ARMY) / 20));
  } else {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
              L"--Must load sector to calculate difficulty--");
  }
}
#endif

void StartChangeSectorArrivalMode(void) {
  // "put him in change arrival sector" mode
  gfInChangeArrivalSectorMode = TRUE;

  // redraw map with bullseye removed
  MarkForRedrawalStrategicMap();

  // change the cursor to that mode
  SetUpCursorForStrategicMap();

  // give instructions as overlay message
  MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[0]);
}

BOOLEAN CanMoveBullseyeAndClickedOnIt(uint8_t sMapX, uint8_t sMapY) {
  // if in airspace mode, and not plotting paths
  if ((fShowAircraftFlag == TRUE) && (bSelectedDestChar == -1) && (fPlotForHelicopter == FALSE)) {
    // don't allow moving bullseye until after initial arrival
    if (gTacticalStatus.fDidGameJustStart == FALSE) {
      // if he clicked on the bullseye, and we're on the surface level
      if ((sMapX == gsMercArriveSectorX) && (sMapY == gsMercArriveSectorY) &&
          (iCurrentMapSectorZ == 0)) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

void CreateBullsEyeOrChopperSelectionPopup(void) {
  wcscpy(gzUserDefinedButton1, pHelicopterEtaStrings[8]);
  wcscpy(gzUserDefinedButton2, pHelicopterEtaStrings[9]);

  // do a BULLSEYE/CHOPPER message box
  DoScreenIndependantMessageBox(pHelicopterEtaStrings[7], MSG_BOX_FLAG_GENERIC,
                                BullsEyeOrChopperSelectionPopupCallback);
}

void BullsEyeOrChopperSelectionPopupCallback(uint8_t ubExitValue) {
  // button 1 pressed?
  if (ubExitValue == MSG_BOX_RETURN_YES) {
    // chose chopper
    // have to set a flag 'cause first call to RequestGiveSkyriderNewDestination() triggers another
    // msg box & won't work
    gfRequestGiveSkyriderNewDestination = TRUE;
  }
  // button 2 pressed?
  else if (ubExitValue == MSG_BOX_RETURN_NO) {
    // chose bullseye
    StartChangeSectorArrivalMode();
  }
}

// wake up anybody who needs to be awake to travel
void WakeUpAnySleepingSelectedMercsOnFootOrDriving(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iCounter;
  BOOLEAN fSuccess = FALSE;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharSelected(iCounter)) {
      pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

      // if asleep
      if (pSoldier->fMercAsleep) {
        // and on foot or driving
        if ((pSoldier->bAssignment < ON_DUTY) ||
            ((GetSolAssignment(pSoldier) == VEHICLE) &&
             SoldierMustDriveVehicle(pSoldier, pSoldier->iVehicleId, FALSE))) {
          // we should be guaranteed that he CAN wake up to get this far, so report errors, but
          // don't force it
          fSuccess = SetMercAwake(pSoldier, TRUE, FALSE);
          Assert(fSuccess);
        }
      }
    }
  }
}

void HandlePostAutoresolveMessages() {
  // KM: Autoresolve sets up this global sector value whenever the enemy gains control of a sector.
  // As soon as we leave autoresolve and enter mapscreen, then this gets called and handles
  // ownership change for the sector. It also brings up a dialog stating to the player what
  // happened, however, the internals of those functions breaks autoresolve and the game crashes
  // after autoresolve is finished.  The value doesn't need to be saved.
  //
  // An additional case is when creatures kill all opposition in the sector.  For each surviving
  // monster, civilians are "virtually" murdered and loyalty hits will be processed.
  if (gsCiviliansEatenByMonsters >= 1) {
    AdjustLoyaltyForCivsEatenByMonsters(SectorID8_X((uint8_t)gsEnemyGainedControlOfSectorID),
                                        SectorID8_Y((uint8_t)gsEnemyGainedControlOfSectorID),
                                        (uint8_t)gsCiviliansEatenByMonsters);
    gsCiviliansEatenByMonsters = -2;
  } else if (gsCiviliansEatenByMonsters == -2) {
    MarkForRedrawalStrategicMap();
    gsCiviliansEatenByMonsters = -1;
    gsEnemyGainedControlOfSectorID = -1;
  } else if (gsEnemyGainedControlOfSectorID >= 0) {  // bring up the dialog box
    SetThisSectorAsEnemyControlled(SectorID8_X((uint8_t)gsEnemyGainedControlOfSectorID),
                                   SectorID8_Y((uint8_t)gsEnemyGainedControlOfSectorID), 0, TRUE);
    gsEnemyGainedControlOfSectorID = -2;
  } else if (gsEnemyGainedControlOfSectorID == -2) {
    // dirty the mapscreen after the dialog box goes away.
    MarkForRedrawalStrategicMap();
    gsEnemyGainedControlOfSectorID = -1;
  } else if (HasNewMilitiaPromotions()) {
    wchar_t str[512];
    BuildMilitiaPromotionsString(str, ARR_SIZE(str));
    DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
  }
}

void GetMapscreenMercAssignmentString(struct SOLDIERTYPE *pSoldier, wchar_t sString[]) {
  if (pSoldier->bAssignment != VEHICLE) {
    wcscpy(sString, pAssignmentStrings[pSoldier->bAssignment]);
  } else {
    wcscpy(sString, pShortVehicleStrings[pVehicleList[pSoldier->iVehicleId].ubVehicleType]);
  }
}

void GetMapscreenMercLocationString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                    int sStringSize) {
  wchar_t pTempString[32];

  if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
    // show blank
    wcscpy(sString, L"--");
  } else {
    if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
      // POW - location unknown
      swprintf(sString, sStringSize, L"%s", pPOWStrings[1]);
    } else {
      swprintf(pTempString, ARR_SIZE(pTempString), L"%s%s%s", pMapVertIndex[pSoldier->sSectorY],
               pMapHortIndex[GetSolSectorX(pSoldier)], pMapDepthIndex[pSoldier->bSectorZ]);

      if (pSoldier->fBetweenSectors) {
        // put brackets around it when he's between sectors!
        swprintf(sString, sStringSize, L"(%s)", pTempString);
      } else {
        wcsncpy(sString, pTempString, sStringSize);
      }
    }
  }
}

void GetMapscreenMercDestinationString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                       int sStringSize) {
  int32_t iSectorX, iSectorY;
  int16_t sSector = 0;
  struct GROUP *pGroup = NULL;

  // by default, show nothing
  wcsncpy(sString, L"", sStringSize);

  // if dead or POW - has no destination (no longer part of a group, for that matter)
  if ((GetSolAssignment(pSoldier) == ASSIGNMENT_DEAD) ||
      (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) || (pSoldier->bLife == 0)) {
    return;
  }

  if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
    // show the sector he'll be arriving in
    iSectorX = gsMercArriveSectorX;
    iSectorY = gsMercArriveSectorY;
  } else {
    // if he's going somewhere
    if (GetLengthOfMercPath(pSoldier) > 0) {
      sSector = GetLastSectorIdInCharactersPath(pSoldier);
      // convert
      iSectorX = SectorID16_X(sSector);
      iSectorY = SectorID16_Y(sSector);
    } else  // no movement path is set...
    {
      if (pSoldier->fBetweenSectors) {
        // he must be returning to his previous (reversed so as to be the next) sector, so show that
        // as his destination individual soldiers don't store previous/next sector coordinates, must
        // go to his group for that
        pGroup = GetGroup(GetSoldierGroupId(pSoldier));
        Assert(pGroup);
        iSectorX = pGroup->ubNextX;
        iSectorY = pGroup->ubNextY;
      } else {
        // show nothing
        return;
      }
    }
  }

  swprintf(sString, sStringSize, L"%s%s", pMapVertIndex[iSectorY], pMapHortIndex[iSectorX]);
}

void GetMapscreenMercDepartureString(struct SOLDIERTYPE *pSoldier, wchar_t sString[],
                                     int sStringSize, uint8_t *pubFontColor) {
  int32_t iMinsRemaining = 0;
  int32_t iDaysRemaining = 0;
  int32_t iHoursRemaining = 0;

  if ((pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC && GetSolProfile(pSoldier) != SLAY) ||
      pSoldier->bLife == 0) {
    swprintf(sString, sStringSize, L"%s", gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
  } else {
    iMinsRemaining = pSoldier->iEndofContractTime - GetWorldTotalMin();

    // if the merc is in transit
    if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
      // and if the time left on the cotract is greater then the contract time
      if (iMinsRemaining > (int32_t)(pSoldier->iTotalContractLength * NUM_MIN_IN_DAY)) {
        iMinsRemaining = (pSoldier->iTotalContractLength * NUM_MIN_IN_DAY);
      }
    }

    // if 3 or more days remain
    if (iMinsRemaining >= MAP_TIME_UNDER_THIS_DISPLAY_AS_HOURS) {
      iDaysRemaining = iMinsRemaining / (24 * 60);

      *pubFontColor = FONT_LTGREEN;

      swprintf(sString, sStringSize, L"%d%s", iDaysRemaining,
               gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
    } else  // less than 3 days
    {
      if (iMinsRemaining > 5) {
        iHoursRemaining = (iMinsRemaining + 59) / 60;
      } else {
        iHoursRemaining = 0;
      }

      // last 3 days is Red, last 4 hours start flashing red/white!
      if ((iMinsRemaining <= MINS_TO_FLASH_CONTRACT_TIME) && (fFlashContractFlag == TRUE)) {
        *pubFontColor = FONT_WHITE;
      } else {
        *pubFontColor = FONT_RED;
      }

      swprintf(sString, sStringSize, L"%d%s", iHoursRemaining,
               gpStrategicString[STR_PB_HOURS_ABBREVIATION]);
    }
  }
}

void InitPreviousPaths(void) {
  int32_t iCounter = 0;

  // init character previous paths
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    gpCharacterPreviousMercPath[iCounter] = NULL;
  }

  // init helicopter previous path
  gpHelicopterPreviousMercPath = NULL;
}

void RememberPreviousPathForAllSelectedChars(void) {
  int32_t iCounter = 0;
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharSelected(iCounter)) {
      // remember his previous path by copying it to his slot in the array kept for that purpose
      gpCharacterPreviousMercPath[iCounter] =
          CopyPaths(GetSoldierMercPathPtr(MercPtrs[gCharactersList[iCounter].usSolID]),
                    gpCharacterPreviousMercPath[iCounter]);
    }
  }
}

void RestorePreviousPaths(void) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  struct path **ppMovePath = NULL;
  uint8_t ubGroupId = 0;
  BOOLEAN fPathChanged = FALSE;

  // invalid if we're not plotting movement
  Assert((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE));

  if (fPlotForHelicopter == TRUE) {
    ppMovePath = &(pVehicleList[iHelicopterVehicleId].pMercPath);
    ubGroupId = pVehicleList[iHelicopterVehicleId].ubMovementGroup;

    // if the helicopter had a previous path
    if (gpHelicopterPreviousMercPath != NULL) {
      gpHelicopterPreviousMercPath = MoveToBeginningOfPathList(gpHelicopterPreviousMercPath);

      // clear current path
      *ppMovePath = ClearStrategicPathList(*ppMovePath, ubGroupId);
      // replace it with the previous one
      *ppMovePath = CopyPaths(gpHelicopterPreviousMercPath, *ppMovePath);
      // will need to rebuild waypoints
      fPathChanged = TRUE;
    } else  // no previous path
    {
      // if he currently has a path
      if (*ppMovePath) {
        // wipe it out!
        *ppMovePath = MoveToBeginningOfPathList(*ppMovePath);
        *ppMovePath = ClearStrategicPathList(*ppMovePath, ubGroupId);
        // will need to rebuild waypoints
        fPathChanged = TRUE;
      }
    }

    if (fPathChanged) {
      // rebuild waypoints
      RebuildWayPointsForGroupPath(*ppMovePath, ubGroupId);

      // copy his path to all selected characters
      CopyPathToAllSelectedCharacters(*ppMovePath);
    }
  } else  // character(s) plotting
  {
    for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
      // if selected
      if (IsCharSelected(iCounter)) {
        pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

        if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
          ppMovePath = &(pVehicleList[pSoldier->bVehicleID].pMercPath);
          ubGroupId = pVehicleList[pSoldier->bVehicleID].ubMovementGroup;
        } else if (GetSolAssignment(pSoldier) == VEHICLE) {
          ppMovePath = &(pVehicleList[pSoldier->iVehicleId].pMercPath);
          ubGroupId = pVehicleList[pSoldier->iVehicleId].ubMovementGroup;
        } else if (pSoldier->bAssignment < ON_DUTY) {
          ppMovePath = &(pSoldier->pMercPath);
          ubGroupId = pSoldier->ubGroupID;
        } else {
          // invalid pSoldier - that guy can't possibly be moving, he's on a non-vehicle assignment!
          Assert(0);
          continue;
        }

        fPathChanged = FALSE;

        // if we have the previous path stored for the dest char
        if (gpCharacterPreviousMercPath[iCounter]) {
          gpCharacterPreviousMercPath[iCounter] =
              MoveToBeginningOfPathList(gpCharacterPreviousMercPath[iCounter]);

          // clear current path
          *ppMovePath = ClearStrategicPathList(*ppMovePath, ubGroupId);
          // replace it with the previous one
          *ppMovePath = CopyPaths(gpCharacterPreviousMercPath[iCounter], *ppMovePath);
          // will need to rebuild waypoints
          fPathChanged = TRUE;
        } else  // no previous path stored
        {
          // if he has one now, wipe it out
          if (*ppMovePath) {
            // wipe it out!
            *ppMovePath = MoveToBeginningOfPathList(*ppMovePath);
            *ppMovePath = ClearStrategicPathList(*ppMovePath, ubGroupId);
            // will need to rebuild waypoints
            fPathChanged = TRUE;
          }
        }

        if (fPathChanged) {
          // rebuild waypoints
          RebuildWayPointsForGroupPath(*ppMovePath, ubGroupId);
        }
      }
    }
  }
}

void ClearPreviousPaths(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if (IsCharSelected(iCounter)) {
      gpCharacterPreviousMercPath[iCounter] =
          ClearStrategicPathList(gpCharacterPreviousMercPath[iCounter], 0);
    }
  }
  gpHelicopterPreviousMercPath = ClearStrategicPathList(gpHelicopterPreviousMercPath, 0);
}

void SelectAllCharactersInSquad(int8_t bSquadNumber) {
  int8_t bCounter;
  BOOLEAN fFirstOne = TRUE;
  struct SOLDIERTYPE *pSoldier;

  // ignore if that squad is empty
  if (SquadIsEmpty(bSquadNumber) == TRUE) {
    return;
  }

  // select nobody & reset the selected list
  ChangeSelectedInfoChar(-1, TRUE);

  // now select all the soldiers that are in this squad
  for (bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++) {
    // is this entry is valid
    if (gCharactersList[bCounter].fValid == TRUE) {
      pSoldier = MercPtrs[gCharactersList[bCounter].usSolID];

      // if this guy is on that squad or in a vehicle which is assigned to that squad
      // NOTE: There's no way to select everyone aboard Skyrider with this function...
      if ((GetSolAssignment(pSoldier) == bSquadNumber) ||
          IsSoldierInThisVehicleSquad(pSoldier, bSquadNumber)) {
        if (fFirstOne) {
          // make the first guy in the list who is in this squad the selected info char
          ChangeSelectedInfoChar(bCounter, FALSE);

          // select his sector
          ChangeSelectedMapSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                  GetSolSectorZ(pSoldier));

          fFirstOne = FALSE;
        }

        SetEntryInSelectedCharacterList(bCounter);
      }
    }
  }
}

BOOLEAN CanDrawSectorCursor(void) {
  if (/*( fCursorIsOnMapScrollButtons == FALSE ) && */
      (fShowTownInfo == FALSE) && (ghTownMineBox == -1) && (fShowUpdateBox == FALSE) &&
      (GetNumberOfMercsInUpdateList() == 0) && (sSelectedMilitiaTown == 0) &&
      (gfMilitiaPopupCreated == FALSE) && (gfStartedFromMapScreen == FALSE) &&
      (fShowMapScreenMovementList == FALSE) && (ghMoveBox == -1) && (fMapInventoryItem == FALSE)) {
    return (TRUE);
  }

  return (FALSE);
}

void RestoreMapSectorCursor(uint8_t sMapX, uint8_t sMapY) {
  int16_t sScreenX, sScreenY;

  Assert((sMapX >= 1) && (sMapX <= 16));
  Assert((sMapY >= 1) && (sMapY <= 16));

  GetScreenXYFromMapXY(sMapX, sMapY, &sScreenX, &sScreenY);

  sScreenY -= 1;

  /*
          if(FALSE)
                  RestoreExternBackgroundRect( ((int16_t)( sScreenX - MAP_GRID_X )), ((int16_t)(
     sScreenY - MAP_GRID_Y )), DMAP_GRID_ZOOM_X, DMAP_GRID_ZOOM_Y); else
  */
  RestoreExternBackgroundRect(sScreenX, sScreenY, DMAP_GRID_X, DMAP_GRID_Y);
}

void RequestToggleMercInventoryPanel(void) {
  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();
    return;
  }

  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    AbortMovementPlottingMode();
  }

  if (!CanToggleSelectedCharInventory()) {
    return;
  }

  if (fShowDescriptionFlag == TRUE) {
    // turn off item description
    DeleteItemDescriptionBox();
  } else {
    // toggle inventory mode
    fShowInventoryFlag = !fShowInventoryFlag;

    // set help text for item glow region
    if (fShowInventoryFlag) {
      SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[2]);
    } else {
      SetRegionFastHelpText(&gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[0]);
    }
  }

  fTeamPanelDirty = TRUE;
}

void RequestContractMenu(void) {
  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();
    return;
  }

  if (gfPreBattleInterfaceActive == TRUE) {
    return;
  }

  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    AbortMovementPlottingMode();
  }

  // in case we have multiple guys selected, turn off everyone but the guy we're negotiating with
  ChangeSelectedInfoChar(bSelectedInfoChar, TRUE);

  if (CanExtendContractForCharSlot(bSelectedInfoChar)) {
    // create
    RebuildContractBoxForMerc(GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID));

    // reset selected characters
    ResetAllSelectedCharacterModes();

    bSelectedContractChar = bSelectedInfoChar;
    giContractHighLine = bSelectedContractChar;

    // if not triggered internally
    if (CheckIfSalaryIncreasedAndSayQuote(MercPtrs[gCharactersList[bSelectedInfoChar].usSolID],
                                          TRUE) == FALSE) {
      // show contract box
      fShowContractMenu = TRUE;

      // stop any dialogue by character
      StopAnyCurrentlyTalkingSpeech();
    }

    // fCharacterInfoPanelDirty = TRUE;
  } else {
    // reset selected characters
    ResetAllSelectedCharacterModes();
  }
}

void ChangeCharacterListSortMethod(int32_t iValue) {
  Assert(iValue >= 0);
  Assert(iValue < MAX_SORT_METHODS);

  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();
    return;
  }

  if (gfPreBattleInterfaceActive == TRUE) {
    return;
  }

  if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
    AbortMovementPlottingMode();
  }

  giSortStateForMapScreenList = iValue;
  SortListOfMercsInTeamPanel(TRUE);
}

void MapscreenMarkButtonsDirty() {
  // redraw buttons
  MarkButtonsDirty();

  // if border buttons are created
  if (!fShowMapInventoryPool) {
    // if the attribute assignment menu is showing
    if (fShowAttributeMenu) {
      // don't redraw the town button, it would wipe out a chunk of the attribute menu
      UnMarkButtonDirty(giMapBorderButtons[MAP_BORDER_TOWN_BTN]);
    }
  }
}
