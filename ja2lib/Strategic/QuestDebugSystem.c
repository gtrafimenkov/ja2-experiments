// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/QuestDebugSystem.h"

#include <stdarg.h>
#include <stdio.h>

#include "Globals.h"
#include "JAScreens.h"
#include "Laptop/AIMMembers.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/Line.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/QuestText.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/Keys.h"
#include "Tactical/Menptr.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/Environment.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// #ifdef JA2BETAVERSION

//*******************************
//
//	Defines
//
//*******************************

typedef void (*DROP_DOWN_DISPLAY_CALLBACK)(wchar_t *);
typedef void (*DROP_DOWN_SELECT_CALLBACK)(wchar_t *);

#define QUEST_DEBUG_FILE "QuestDebugRecordLog.txt"

#define QUEST_DBS_FONT_TITLE FONT14ARIAL
#define QUEST_DBS_COLOR_TITLE FONT_MCOLOR_LTGREEN
#define QUEST_DBS_COLOR_SUBTITLE FONT_MCOLOR_DKGRAY

#define QUEST_DBS_FONT_STATIC_TEXT FONT12ARIAL
#define QUEST_DBS_COLOR_STATIC_TEXT FONT_MCOLOR_WHITE

#define QUEST_DBS_FONT_DYNAMIC_TEXT FONT12ARIAL
#define QUEST_DBS_COLOR_DYNAMIC_TEXT FONT_MCOLOR_WHITE

#define QUEST_DBS_FONT_LISTBOX_TEXT FONT12ARIAL
#define QUEST_DBS_COLOR_LISTBOX_TEXT FONT_MCOLOR_WHITE

#define QUEST_DBS_FONT_TEXT_ENTRY FONT12ARIAL
#define QUEST_DBS_COLOR_TEXT_ENTRY FONT_MCOLOR_WHITE

#define QUEST_DBS_FIRST_SECTION_WIDTH 210
#define QUEST_DBS_SECOND_SECTION_WIDTH 230
#define QUEST_DBS_THIRD_SECTION_WIDTH 200

#define QUEST_DBS_NUMBER_COL_WIDTH 40
#define QUEST_DBS_TITLE_COL_WIDTH 120
#define QUEST_DBS_STATUS_COL_WIDTH 50

#define QUEST_DBS_FIRST_COL_NUMBER_X 5
#define QUEST_DBS_FIRST_COL_NUMBER_Y 50

#define QUEST_DBS_FIRST_COL_TITLE_X QUEST_DBS_FIRST_COL_NUMBER_X + QUEST_DBS_NUMBER_COL_WIDTH
#define QUEST_DBS_FIRST_COL_TITLE_Y QUEST_DBS_FIRST_COL_NUMBER_Y

#define QUEST_DBS_FIRST_COL_STATUS_X QUEST_DBS_FIRST_COL_TITLE_X + QUEST_DBS_TITLE_COL_WIDTH
#define QUEST_DBS_FIRST_COL_STATUS_Y QUEST_DBS_FIRST_COL_NUMBER_Y

#define QUEST_DBS_SECOND_NUMBER_COL_WIDTH 40
#define QUEST_DBS_SECOND_TITLE_COL_WIDTH 140
#define QUEST_DBS_SECOND_STATUS_COL_WIDTH 50

#define QUEST_DBS_SECOND_COL_NUMBER_X QUEST_DBS_FIRST_SECTION_WIDTH + 5
#define QUEST_DBS_SECOND_COL_NUMBER_Y QUEST_DBS_FIRST_COL_NUMBER_Y

#define QUEST_DBS_SECOND_COL_TITLE_X QUEST_DBS_SECOND_COL_NUMBER_X + QUEST_DBS_NUMBER_COL_WIDTH
#define QUEST_DBS_SECOND_COL_TITLE_Y QUEST_DBS_SECOND_COL_NUMBER_Y

#define QUEST_DBS_SECOND_COL_STATUS_X \
  QUEST_DBS_SECOND_COL_TITLE_X + QUEST_DBS_SECOND_TITLE_COL_WIDTH
#define QUEST_DBS_SECOND_COL_STATUS_Y QUEST_DBS_SECOND_COL_NUMBER_Y

#define QUEST_DBS_SECTION_TITLE_Y 30

#define QUEST_DBS_MAX_DISPLAYED_ENTRIES 20  // 25

#define QUEST_DBS_THIRD_COL_TITLE_X QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH

#define QUEST_DBS_NPC_CHCKBOX_TGL_X \
  QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH + 5
#define QUEST_DBS_NPC_CHCKBOX_TGL_Y QUEST_DBS_FIRST_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_SELECTED_NPC_BUTN_X QUEST_DBS_NPC_CHCKBOX_TGL_X
#define QUEST_DBS_SELECTED_NPC_BUTN_Y QUEST_DBS_NPC_CHCKBOX_TGL_Y + 22

#define QUEST_DBS_SELECTED_ITEM_BUTN_X \
  QUEST_DBS_SELECTED_NPC_BUTN_X  // QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH +
                                 // 105
#define QUEST_DBS_SELECTED_ITEM_BUTN_Y QUEST_DBS_SELECTED_NPC_BUTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_LIST_TEXT_OFFSET 26

#define QUEST_DBS_LIST_BOX_WIDTH 183  // 80

#define QUEST_DBS_SCROLL_BAR_WIDTH 11

#define QUEST_DBS_SCROLL_ARROW_HEIGHT 17

#define QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR 30

#define QUEST_DBS_ADD_NPC_BTN_X QUEST_DBS_SELECTED_NPC_BUTN_X
#define QUEST_DBS_ADD_NPC_BTN_Y QUEST_DBS_SELECTED_ITEM_BUTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_ADD_ITEM_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_ADD_ITEM_BTN_Y QUEST_DBS_ADD_NPC_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_Y QUEST_DBS_ADD_ITEM_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_CHANGE_DAY_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_CHANGE_DAY_BTN_Y QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_VIEW_NPC_INV_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_VIEW_NPC_INV_BTN_Y QUEST_DBS_CHANGE_DAY_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_RESTORE_NPC_INV_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_RESTORE_NPC_INV_BTN_Y QUEST_DBS_VIEW_NPC_INV_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_NPC_LOG_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_NPC_LOG_BTN_Y QUEST_DBS_RESTORE_NPC_INV_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_NPC_REFRESH_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_NPC_REFRESH_BTN_Y QUEST_DBS_NPC_LOG_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_START_MERC_TALKING_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_START_MERC_TALKING_BTN_Y QUEST_DBS_NPC_REFRESH_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_ADD_NPC_TO_TEAM_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y \
  QUEST_DBS_START_MERC_TALKING_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y \
  QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_NPC_CURRENT_GRIDNO_X QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_NPC_CURRENT_GRIDNO_Y \
  QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

// Text Entry Box
#define QUEST_DBS_TEB_X 200
#define QUEST_DBS_TEB_Y 160

#define QUEST_DBS_TEB_WIDTH 245
#define QUEST_DBS_TEB_HEIGHT 140

#define QUEST_DBS_NUM_DISPLAYED_QUESTS MAX_QUESTS
#define QUEST_DBS_NUM_DISPLAYED_FACTS 25

#define QUEST_DBS_TEXT_FIELD_WIDTH 7

// NPC Inventory Popup box
#define QUEST_DBS_NPC_INV_POPUP_X 150
#define QUEST_DBS_NPC_INV_POPUP_Y 110

#define QUEST_DBS_NPC_INV_POPUP_WIDTH 275
#define QUEST_DBS_NPC_INV_POPUP_HEIGHT 325

#define QUEST_DBS_SIZE_NPC_ARRAY TOTAL_SOLDIERS

#define QUEST_DBS_FACT_LIST_OFFSET 28

#define CLOCK_X 554
#define CLOCK_Y 459

#define QDS_BUTTON_HEIGHT 21

#define QDS_CURRENT_QUOTE_NUM_BOX_X 150
#define QDS_CURRENT_QUOTE_NUM_BOX_Y 300
#define QDS_CURRENT_QUOTE_NUM_BOX_WIDTH 285
#define QDS_CURRENT_QUOTE_NUM_BOX_HEIGHT 80

//
// drop down list box
//
enum {
  QD_DROP_DOWN_NO_ACTION = 0,
  QD_DROP_DOWN_CREATE,
  QD_DROP_DOWN_DESTROY,
  QD_DROP_DOWN_DISPLAY,
  QD_DROP_DOWN_CANCEL,
};

wchar_t *QuestStates[] = {
    L"N.S.",
    L"In Prog.",
    L"Done",
};

wchar_t *QuestDebugText[] = {
    L"Quest Debug System",
    L"Quests",
    L"Quest Number",
    L"Quest Title",
    L"Status",
    L"Facts",
    L"Fact Number",
    L"Desc.",
    L"Select Merc",
    L"Select Item",
    L"NPC RecordLog",
    L"Exit Quest Debug",
    L"NPC Info",
    L"** No Item **",
    L"Add Merc To Location",
    L"Add Item To Location",
    L"Change Day",
    L"NPC log Button",
    L"Please Enter the grid #",
    L"Give Item To NPC",
    L"View NPC's Inventory",
    L"Please enter the number of days to advance.",
    L"NPC Inventory",
    L"View NPC's in current sector",
    L"No NPC's In Sector",
    L"Please Enter New Value for ",
    L"0,1,2",
    L"0,1",
    L"Quest #",
    L"Fact #",
    L"Pg Facts Up",
    L"Pg Facts Down",
    L"No Text",
    L"CurrentGridNo",
    L"Refresh NPC Script",
    L"Succesfully Refreshed",
    L"Failed Refreshing",
    L"Restore All NPC's inventory",
    L"Start Merc Talking",
    L"Please enter a quote number for the selected merc to start talking from.",
    L"RPC is added to team",
    L"RPC says Sector Desc",
    L"Space:       Toggle Pausing Merc Speech",
    L"Left Arrow:  Previous Quote",
    L"Right Arrow: Next Quote",
    L"ESC:         To Stop the merc from Talking",
    L"",
    L"",

};

// enums for above strings
enum {
  QUEST_DBS_TITLE = 0,
  QUEST_DBS_QUESTS,
  QUEST_DBS_QUEST_NUMBER,
  QUEST_DBS_QUEST_TITLE,
  QUEST_DBS_STATUS,
  QUEST_DBS_FACTS,
  QUEST_DBS_FACT_NUMBER,
  QUEST_DBS_DESC,
  QUEST_DBS_SELECTED_NPC,
  QUEST_DBS_SELECTED_ITEM,
  QUEST_DBS_NPC_RECORDLOG,
  QUEST_DBS_EXIT_QUEST_DEBUG,
  QUEST_DBS_NPC_INFO,
  QUEST_DBS_NO_ITEM,
  QUEST_DBS_ADD_CURRENT_NPC,
  QUEST_DBS_ADD_CURRENT_ITEM,
  QUEST_DBS_CHANGE_DAY,
  QUEST_DBS_NPC_LOG_BUTTON,
  QUEST_DBS_ENTER_GRID_NUM,
  QUEST_DBS_GIVE_ITEM_TO_NPC,
  QUEST_DBS_VIEW_NPC_INVENTORY,
  QUEST_DBS_PLEASE_ENTER_DAY,
  QUEST_DBS_NPC_INVENTORY,
  QUEST_DBS_VIEW_LOCAL_NPC,
  QUEST_DBS_NO_NPC_IN_SECTOR,
  QUEST_DBS_ENTER_NEW_VALUE,
  QUEST_DBS_0_1_2,
  QUEST_DBS_0_1,
  QUEST_DBS_QUEST_NUM,
  QUEST_DBS_FACT_NUM,
  QUEST_DBS_PG_FACTS_UP,
  QUEST_DBS_PG_FACTS_DOWN,
  QUEST_DBS_NO_TEXT,
  QUEST_DBS_CURRENT_GRIDNO,
  QUEST_DBS_REFRESH_NPC,
  QUEST_DBS_REFRESH_OK,
  QUEST_DBS_REFRESH_FAILED,
  QUEST_DBS_RESTORE_NPC_INVENTORY,
  QUEST_DBS_START_MERC_TALKING,
  QUEST_DBS_START_MERC_TALKING_FROM,
  QUEST_DBS_ADD_NPC_TO_TEAM,
  QUEST_DBS_RPC_SAY_SECTOR_DESC,
  QUEST_DBS_PAUSE_SPEECH,
  QUEST_DBS_LEFT_ARROW_PREVIOUS_QUOTE,
  QUEST_DBS_RIGHT_ARROW_NEXT_QUOTE,
  QUEST_DBS_ESC_TOP_STOP_TALKING
};

wchar_t *PocketText[] = {
    L"Helmet",     L"Vest",        L"Leg",        L"Head1",      L"Head2",
    L"Hand",       L"Second Hand", L"Bigpock1",   L"Bigpock2",   L"Bigpock3",
    L"Bigpock4",   L"Smallpock1",  L"Smallpock2", L"Smallpock3", L"Smallpock4",
    L"Smallpock5", L"Smallpock6",  L"Smallpock7", L"Smallpock8",
};

//*******************************
//
//	Global Variables
//
//*******************************

extern uint32_t guiGameClock;
extern uint32_t guiBrownBackgroundForTeamPanel;

typedef void (*LISTBOX_DISPLAY_FNCTN)();       // Define Display Callback function
typedef void (*TEXT_ENTRY_CALLBACK)(int32_t);  // Callback for when the text entry field is finished

typedef struct {
  LISTBOX_DISPLAY_FNCTN DisplayFunction;  //	The array of items

  uint16_t usScrollPosX;    //	Top Left Pos of list box
  uint16_t usScrollPosY;    //	Top Left Pos of list box
  uint16_t usScrollHeight;  //	Height of list box
  uint16_t usScrollWidth;   //	Width of list box

  uint16_t usScrollBarHeight;    //	Height of Scroll box
  uint16_t usScrollBarWidth;     //	Width of Scroll box
  uint16_t usScrollBoxY;         //	Current Vertical location of the scroll box
  uint16_t usScrollBoxEndY;      //	Bottom position on the scroll box
  uint16_t usScrollArrowHeight;  //	Scroll Arrow height

  int16_t sCurSelectedItem;             //	Currently selected item
  uint16_t usItemDisplayedOnTopOfList;  //	item at the top of displayed list
  uint16_t usStartIndex;                //	index to start at for the array of elements
  uint16_t usMaxArrayIndex;             //	Max Size of the array
  uint16_t usNumDisplayedItems;         //	Num of displayed item
  uint16_t usMaxNumDisplayedItems;      //  Max number of Displayed items

  uint8_t ubCurScrollBoxAction;  //	Holds the status of the current action ( create; destroy...
                                 //)

} SCROLL_BOX;

// Enums for the possible panels the mercs can use
enum {
  QDS_REGULAR_PANEL,
  QDS_NPC_PANEL,
  QDS_NO_PANEL,
};

// image identifiers
uint32_t guiQdScrollArrowImage;

BOOLEAN gfQuestDebugEntry = TRUE;
BOOLEAN gfQuestDebugExit = FALSE;

BOOLEAN gfRedrawQuestDebugSystem = TRUE;

uint16_t gusQuestDebugBlue;
uint16_t gusQuestDebugLtBlue;
uint16_t gusQuestDebugDkBlue;

// int16_t		gsCurScrollBoxY=0;

SCROLL_BOX gNpcListBox;   // The Npc Scroll box
SCROLL_BOX gItemListBox;  // The Npc Scroll box

SCROLL_BOX *gpActiveListBox;  // Only 1 scroll box is active at a time, this is set to it.

int16_t gsQdsEnteringGridNo = 0;

uint8_t gubTextEntryAction = QD_DROP_DOWN_NO_ACTION;
BOOLEAN gfTextEntryActive = FALSE;
// wchar_t			gzTextEntryReturnString[ 16 ];

BOOLEAN gfUseLocalNPCs = FALSE;

uint8_t gubNPCInventoryPopupAction = QD_DROP_DOWN_NO_ACTION;

uint8_t gubCurrentNpcInSector[QUEST_DBS_SIZE_NPC_ARRAY];
uint8_t gubNumNPCinSector;

uint8_t gubCurQuestSelected;
uint16_t gusCurFactSelected;
uint16_t gusFactAtTopOfList;

// int16_t				gsCurrentNPCLog=-1;
// //If this is set, the value will be set to the
BOOLEAN gfNpcLogButton = FALSE;

int32_t giHaveSelectedItem = -1;  // If it is not the first time in, dont reset the Selected ITem
int32_t giHaveSelectedNPC = -1;   // If it is not the first time in, dont reset the selected NPC

int32_t giSelectedMercCurrentQuote = -1;
struct SOLDIERTYPE *gTalkingMercSoldier = NULL;
BOOLEAN gfPauseTalkingMercPopup = FALSE;
extern BOOLEAN gfFacePanelActive;
BOOLEAN gfAddNpcToTeam = FALSE;
BOOLEAN gfRpcToSaySectorDesc = FALSE;
BOOLEAN gfNpcPanelIsUsedForTalkingMerc = FALSE;
extern struct SOLDIERTYPE *gpDestSoldier;

BOOLEAN gfBackgroundMaskEnabled = FALSE;

BOOLEAN gfExitQdsDueToMessageBox = FALSE;
int32_t giQdsMessageBox = -1;  // Qds pop up messages index value

BOOLEAN gfInDropDownBox = FALSE;
// BOOLEAN			gfExitOptionsAfterMessageBox = FALSE;

BOOLEAN gfAddKeyNextPass = FALSE;
BOOLEAN gfDropDamagedItems = FALSE;

//
// Mouse Regions
//
struct MOUSE_REGION gQuestDebugSysScreenRegions;
// void QuestDebugSysScreenRegionCallBack(struct MOUSE_REGION * pRegion, int32_t iReason );

uint32_t guiQuestDebugExitButton;
void BtnQuestDebugExitButtonCallback(GUI_BUTTON *btn, int32_t reason);

// checkbox for weather to show all npc or just npc in sector
uint32_t guiQuestDebugAllOrSectorNPCToggle;
void BtnQuestDebugAllOrSectorNPCToggleCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugCurNPCButton;
void BtnQuestDebugCurNPCButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugCurItemButton;
void BtnQuestDebugCurItemButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugAddNpcToLocationButton;
void BtnQuestDebugAddNpcToLocationButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugAddItemToLocationButton;
void BtnQuestDebugAddItemToLocationButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugGiveItemToNPCButton;
void BtnQuestDebugGiveItemToNPCButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugChangeDayButton;
void BtnQuestDebugChangeDayButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugViewNPCInvButton;
void BtnQuestDebugViewNPCInvButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugRestoreNPCInvButton;
void BtnQuestDebugRestoreNPCInvButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugNPCLogButtonButton;
void BtnQuestDebugNPCLogButtonButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugNPCRefreshButtonButton;
void BtnQuestDebugNPCRefreshButtonButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQuestDebugStartMercTalkingButtonButton;
void BtnQuestDebugStartMercTalkingButtonButtonCallback(GUI_BUTTON *btn, int32_t reason);

// checkbox for weather to add the merc to the players team
uint32_t guiQuestDebugAddNpcToTeamToggle;
void BtnQuestDebugAddNpcToTeamToggleCallback(GUI_BUTTON *btn, int32_t reason);

// checkbox for weather have rpc say the sector description
uint32_t guiQuestDebugRPCSaySectorDescToggle;
void BtnQuestDebugRPCSaySectorDescToggleCallback(GUI_BUTTON *btn, int32_t reason);

struct MOUSE_REGION gSelectedNpcListRegion[QUEST_DBS_MAX_DISPLAYED_ENTRIES];
void SelectNpcListRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectNpcListMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

struct MOUSE_REGION gScrollAreaRegion[QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR];
void ScrollAreaRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void ScrollAreaMovementCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

struct MOUSE_REGION gScrollArrowsRegion[2];
void ScrollArrowsRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// Text entry Disable the screen
struct MOUSE_REGION gQuestTextEntryDebugDisableScreenRegion;
void QuestDebugTextEntryDisableScreenRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// Ok button on the text entry form
uint32_t guiQuestDebugTextEntryOkBtn;
void BtnQuestDebugTextEntryOkBtnButtonCallback(GUI_BUTTON *btn, int32_t reason);

// Ok button on the NPC inventory form
uint32_t guiQuestDebugNPCInventOkBtn;
void BtnQuestDebugNPCInventOkBtnButtonCallback(GUI_BUTTON *btn, int32_t reason);

// Mouse regions for the Quests
struct MOUSE_REGION gQuestListRegion[QUEST_DBS_NUM_DISPLAYED_QUESTS];
void ScrollQuestListRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// Mouse regions for the Facts
struct MOUSE_REGION gFactListRegion[QUEST_DBS_NUM_DISPLAYED_FACTS];
void ScrollFactListRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

uint32_t guiQDPgUpButtonButton;
void BtnQDPgUpButtonButtonCallback(GUI_BUTTON *btn, int32_t reason);

uint32_t guiQDPgDownButtonButton;
void BtnQDPgDownButtonButtonCallback(GUI_BUTTON *btn, int32_t reason);

//*******************************
//
//	Function Prototypes
//
//*******************************

BOOLEAN EnterQuestDebugSystem();
void RenderQuestDebugSystem();
void ExitQuestDebugSystem();
void HandleQuestDebugSystem();
void GetUserInput();
void QuestDebug_ExitTactical();
void QuestDebug_EnterTactical();
void DisplayQuestInformation();
void DisplayFactInformation();
void DisplaySectionLine();
void DisplayQuestList();
void DisplayFactList();
void DisplayNPCInfo();
BOOLEAN CreateDestroyDisplaySelectNpcDropDownBox();
void DrawQdsScrollRectangle();
void CalcPositionOfNewScrollBoxLocation();
void DisplaySelectedListBox();
void DisplaySelectedNPC();
void DisplaySelectedItem();
void TextEntryBox(wchar_t *pString, TEXT_ENTRY_CALLBACK TextEntryCallBack);
BOOLEAN CreateDestroyDisplayTextEntryBox(uint8_t ubAction, wchar_t *pString,
                                         TEXT_ENTRY_CALLBACK TextEntryCallBack);
void InitQuestDebugTextInputBoxes();
void DestroyQuestDebugTextInputBoxes();
void AddNPCToGridNo(int32_t iGridNo);
void AddItemToGridNo(int32_t iGridNo);
void StartMercTalkingFromQuoteNum(int32_t iGridNo);
void AddKeyToGridNo(int32_t iGridNo);
void ChangeDayNumber(int32_t iDayToChangeTo);
void CreateDestroyDisplayNPCInventoryPopup(uint8_t ubAction);
void AddNPCsInSectorToArray();
void ChangeQuestState(int32_t iNumber);
void ChangeFactState(int32_t iNumber);
void DisplayCurrentGridNo();
void EnableQDSButtons();
BOOLEAN DoQDSMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen, uint8_t ubFlags,
                        MSGBOX_CALLBACK ReturnCallback);
void IncrementActiveDropDownBox(int16_t sIncrementValue);
int16_t IsMercInTheSector(uint16_t usMercID);
void RefreshAllNPCInventory();
void SetQDSMercProfile();
void HandleQDSTalkingMerc();
void DisplayQDSCurrentlyQuoteNum();
void SetTalkingMercPauseState(BOOLEAN fState);
uint8_t WhichPanelShouldTalkingMercUse();
void EndMercTalking();
void EnableFactMouseRegions();
void DisableFactMouseRegions();
int32_t GetMaxNumberOfQuotesToPlay();
void GetDebugLocationString(uint16_t usProfileID, wchar_t *pzText, size_t bufSize);

// ppp

//*******************************
//
//	Code
//
//*******************************

uint32_t QuestDebugScreenInit() {
  uint16_t usListBoxFontHeight = GetFontHeight(QUEST_DBS_FONT_LISTBOX_TEXT) + 2;

  // Set so next time we come in, we can set up
  gfQuestDebugEntry = TRUE;

  gusQuestDebugBlue = rgb32_to_rgb16(FROMRGB(65, 79, 94));

  // Initialize which facts are at the top of the list
  gusFactAtTopOfList = 0;

  gubCurQuestSelected = 0;
  gusCurFactSelected = 0;

  //
  // Set the Npc List box
  //
  memset(&gNpcListBox, 0, sizeof(SCROLL_BOX));
  gNpcListBox.DisplayFunction = DisplaySelectedNPC;  //	The function to display the entries

  gNpcListBox.usScrollPosX = QUEST_DBS_SELECTED_NPC_BUTN_X;
  gNpcListBox.usScrollPosY = QUEST_DBS_SELECTED_NPC_BUTN_Y + 25;
  gNpcListBox.usScrollHeight = usListBoxFontHeight * QUEST_DBS_MAX_DISPLAYED_ENTRIES;
  gNpcListBox.usScrollWidth = QUEST_DBS_LIST_BOX_WIDTH;
  gNpcListBox.usScrollArrowHeight = QUEST_DBS_SCROLL_ARROW_HEIGHT;
  gNpcListBox.usScrollBarHeight =
      gNpcListBox.usScrollHeight - (2 * gNpcListBox.usScrollArrowHeight);
  gNpcListBox.usScrollBarWidth = QUEST_DBS_SCROLL_BAR_WIDTH;

  gNpcListBox.sCurSelectedItem = -1;
  gNpcListBox.usItemDisplayedOnTopOfList = 0;  // FIRST_RPC;
  gNpcListBox.usStartIndex = 0;                // FIRST_RPC;
  gNpcListBox.usMaxArrayIndex = NUM_PROFILES;
  gNpcListBox.usNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;
  gNpcListBox.usMaxNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;

  gNpcListBox.ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;

  //
  // Set the Item List box
  //
  memset(&gItemListBox, 0, sizeof(SCROLL_BOX));
  gItemListBox.DisplayFunction = DisplaySelectedItem;  //	The function to display the entries

  gItemListBox.usScrollPosX = QUEST_DBS_SELECTED_ITEM_BUTN_X;
  gItemListBox.usScrollPosY = QUEST_DBS_SELECTED_ITEM_BUTN_Y + 25;
  gItemListBox.usScrollHeight = usListBoxFontHeight * QUEST_DBS_MAX_DISPLAYED_ENTRIES;
  gItemListBox.usScrollWidth = QUEST_DBS_LIST_BOX_WIDTH;
  gItemListBox.usScrollArrowHeight = QUEST_DBS_SCROLL_ARROW_HEIGHT;
  gItemListBox.usScrollBarHeight =
      gItemListBox.usScrollHeight - (2 * gItemListBox.usScrollArrowHeight);
  gItemListBox.usScrollBarWidth = QUEST_DBS_SCROLL_BAR_WIDTH;

  gItemListBox.sCurSelectedItem = -1;

  gItemListBox.usItemDisplayedOnTopOfList = 1;
  gItemListBox.usStartIndex = 1;
  gItemListBox.usMaxArrayIndex = MAXITEMS;
  gItemListBox.usNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;
  gItemListBox.usMaxNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;

  gItemListBox.ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;

  gfUseLocalNPCs = FALSE;

  // Set up the global list box
  gpActiveListBox = &gNpcListBox;

  return (TRUE);
}

uint32_t QuestDebugScreenHandle() {
  if (gfQuestDebugEntry) {
    PauseGame();

    EnterQuestDebugSystem();
    gfQuestDebugEntry = FALSE;
    gfQuestDebugExit = FALSE;

    RenderQuestDebugSystem();

    // At this point the background is pure, copy it to the save buffer
    BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 0, 639, 479);
  }
  RestoreBackgroundRects();

  // ATE: Disable messages....
  DisableScrollMessages();

  GetUserInput();

  if (gfTextEntryActive || gubTextEntryAction) {
    if (gubTextEntryAction != QD_DROP_DOWN_NO_ACTION) {
      CreateDestroyDisplayTextEntryBox(gubTextEntryAction, NULL, NULL);
      gubTextEntryAction = QD_DROP_DOWN_NO_ACTION;
    }

    RenderAllTextFields();

  } else
    HandleQuestDebugSystem();

  if (gfRedrawQuestDebugSystem) {
    RenderQuestDebugSystem();

    gfRedrawQuestDebugSystem = FALSE;
  }

  // if the merc is supposed to be talking
  if (giSelectedMercCurrentQuote != -1) {
    // and it is an npc
    if (WhichPanelShouldTalkingMercUse() == QDS_NPC_PANEL) {
      gTalkPanel.fDirtyLevel = DIRTYLEVEL2;
      ButtonList[guiQDPgUpButtonButton]->uiFlags |= BUTTON_FORCE_UNDIRTY;
      RenderTalkingMenu();
    }
  }

  // render buttons marked dirty
  RenderButtons();

  // To handle the dialog
  HandleDialogue();
  HandleAutoFaces();
  HandleTalkingAutoFaces();

  ExecuteVideoOverlays();

  SaveBackgroundRects();
  RenderButtonsFastHelp();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();

  if (gfQuestDebugExit) {
    ExitQuestDebugSystem();
    gfQuestDebugExit = FALSE;
    gfQuestDebugEntry = TRUE;

    UnPauseGame();
    return (GAME_SCREEN);
  }

  return (QUEST_DEBUG_SCREEN);
}

uint32_t QuestDebugScreenShutdown() { return (TRUE); }

BOOLEAN EnterQuestDebugSystem() {
  uint8_t i;
  uint16_t usPosX, usPosY;
  wchar_t zName[128];
  //	uint16_t	usListBoxFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;

  //	wchar_t	zItemName[ SIZE_ITEM_NAME ];
  //	wchar_t	zItemDesc[ SIZE_ITEM_INFO ];

  uint16_t usFontHeight = GetFontHeight(QUEST_DBS_FONT_DYNAMIC_TEXT) + 2;

  if (gfExitQdsDueToMessageBox) {
    gfRedrawQuestDebugSystem = TRUE;
    gfExitQdsDueToMessageBox = FALSE;
    return (TRUE);
  }

  QuestDebug_ExitTactical();

  MSYS_DefineRegion(&gQuestDebugSysScreenRegions, 0, 0, 640, 480, MSYS_PRIORITY_HIGH,
                    CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
  // Add region
  MSYS_AddRegion(&gQuestDebugSysScreenRegions);

  guiQuestDebugExitButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_EXIT_QUEST_DEBUG], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, 535, 450, 100, 25, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 10, BUTTON_NO_CALLBACK, BtnQuestDebugExitButtonCallback);

  // Check box to toggle between all and local npc's
  guiQuestDebugAllOrSectorNPCToggle = CreateCheckBoxButton(
      QUEST_DBS_NPC_CHCKBOX_TGL_X, QUEST_DBS_NPC_CHCKBOX_TGL_Y, "INTERFACE\\checkbox.sti",
      MSYS_PRIORITY_HIGH + 2, BtnQuestDebugAllOrSectorNPCToggleCallback);
  //	ButtonList[ iSummaryButton[ SUMMARY_PROGRESSCHECKBOX ] ]->uiFlags |= BUTTON_CLICKED_ON;

  // Currently Selected NPC button
  guiQuestDebugCurNPCButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_SELECTED_NPC], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_SELECTED_NPC_BUTN_X,
      QUEST_DBS_SELECTED_NPC_BUTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugCurNPCButtonCallback);

  // Currently Selected item button
  guiQuestDebugCurItemButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_SELECTED_ITEM], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_SELECTED_ITEM_BUTN_X,
      QUEST_DBS_SELECTED_ITEM_BUTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugCurItemButtonCallback);

  // Add NPC to location
  guiQuestDebugAddNpcToLocationButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_ADD_CURRENT_NPC], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_ADD_NPC_BTN_X,
      QUEST_DBS_ADD_NPC_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugAddNpcToLocationButtonCallback);

  // Add item to location
  guiQuestDebugAddItemToLocationButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_ADD_CURRENT_ITEM], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_ADD_ITEM_BTN_X,
      QUEST_DBS_ADD_ITEM_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugAddItemToLocationButtonCallback);

  // Give item to Npc
  guiQuestDebugGiveItemToNPCButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_GIVE_ITEM_TO_NPC], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_X,
      QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugGiveItemToNPCButtonCallback);

  // Change Day
  guiQuestDebugChangeDayButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_CHANGE_DAY], QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT,
      FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_CHANGE_DAY_BTN_X, QUEST_DBS_CHANGE_DAY_BTN_Y,
      QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH + 2,
      BUTTON_NO_CALLBACK, BtnQuestDebugChangeDayButtonCallback);

  // View NPC Inventory
  guiQuestDebugViewNPCInvButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_VIEW_NPC_INVENTORY], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_VIEW_NPC_INV_BTN_X,
      QUEST_DBS_VIEW_NPC_INV_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugViewNPCInvButtonCallback);

  // Restore NPC Inventory
  guiQuestDebugRestoreNPCInvButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_RESTORE_NPC_INVENTORY], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_RESTORE_NPC_INV_BTN_X,
      QUEST_DBS_RESTORE_NPC_INV_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugRestoreNPCInvButtonCallback);

  // NPC log button
  swprintf(zName, ARR_SIZE(zName), L"%s - (%s)", QuestDebugText[QUEST_DBS_NPC_LOG_BUTTON],
           gfNpcLogButton ? L"On" : L"Off");
  guiQuestDebugNPCLogButtonButton = CreateTextButton(
      zName, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK,
      BUTTON_USE_DEFAULT, QUEST_DBS_NPC_LOG_BTN_X, QUEST_DBS_NPC_LOG_BTN_Y,
      QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH + 2,
      BUTTON_NO_CALLBACK, BtnQuestDebugNPCLogButtonButtonCallback);

  guiQuestDebugNPCRefreshButtonButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_REFRESH_NPC], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT, QUEST_DBS_NPC_REFRESH_BTN_X,
      QUEST_DBS_NPC_REFRESH_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQuestDebugNPCRefreshButtonButtonCallback);

  // Start the selected merc talking
  guiQuestDebugStartMercTalkingButtonButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_START_MERC_TALKING], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT,
      QUEST_DBS_START_MERC_TALKING_BTN_X, QUEST_DBS_START_MERC_TALKING_BTN_Y,
      QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH + 2,
      BUTTON_NO_CALLBACK, BtnQuestDebugStartMercTalkingButtonButtonCallback);

  guiQDPgUpButtonButton = CreateTextButton(
      QuestDebugText[QUEST_DBS_PG_FACTS_UP], QUEST_DBS_FONT_STATIC_TEXT,
      QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT,
      QUEST_DBS_SECOND_COL_NUMBER_X + 5, QUEST_DBS_SECOND_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET,
      QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH + 2,
      BUTTON_NO_CALLBACK, BtnQDPgUpButtonButtonCallback);

  guiQDPgDownButtonButton =
      CreateTextButton(QuestDebugText[QUEST_DBS_PG_FACTS_DOWN], QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, BUTTON_USE_DEFAULT,
                       QUEST_DBS_SECOND_COL_NUMBER_X + 5,
                       QUEST_DBS_SECOND_COL_NUMBER_Y + 15 * QUEST_DBS_NUM_DISPLAYED_FACTS +
                           QUEST_DBS_LIST_TEXT_OFFSET,
                       QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, BUTTON_TOGGLE,
                       MSYS_PRIORITY_HIGH + 2, BUTTON_NO_CALLBACK, BtnQDPgDownButtonButtonCallback);

  // checkbox for weather to add the merc to the players team
  guiQuestDebugAddNpcToTeamToggle = CreateCheckBoxButton(
      QUEST_DBS_ADD_NPC_TO_TEAM_BTN_X, QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y, "INTERFACE\\checkbox.sti",
      MSYS_PRIORITY_HIGH + 2, BtnQuestDebugAddNpcToTeamToggleCallback);
  if (gfAddNpcToTeam) ButtonList[guiQuestDebugAddNpcToTeamToggle]->uiFlags |= BUTTON_CLICKED_ON;

  // checkbox for weather have rpc say the sector description
  guiQuestDebugRPCSaySectorDescToggle =
      CreateCheckBoxButton(QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_X,
                           QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y, "INTERFACE\\checkbox.sti",
                           MSYS_PRIORITY_HIGH + 2, BtnQuestDebugRPCSaySectorDescToggleCallback);
  if (gfRpcToSaySectorDesc)
    ButtonList[guiQuestDebugRPCSaySectorDescToggle]->uiFlags |= BUTTON_CLICKED_ON;

  // Setup mouse regions for the Quest list
  usPosX = QUEST_DBS_FIRST_COL_NUMBER_X;
  usPosY = QUEST_DBS_FIRST_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET;
  for (i = 0; i < QUEST_DBS_NUM_DISPLAYED_QUESTS; i++) {
    MSYS_DefineRegion(&gQuestListRegion[i], usPosX, usPosY,
                      (uint16_t)(usPosX + QUEST_DBS_FIRST_SECTION_WIDTH),
                      (uint16_t)(usPosY + usFontHeight), MSYS_PRIORITY_HIGH + 2, CURSOR_WWW,
                      MSYS_NO_CALLBACK, ScrollQuestListRegionCallBack);  // CURSOR_LAPTOP_SCREEN
    // Add region
    MSYS_AddRegion(&gQuestListRegion[i]);
    MSYS_SetRegionUserData(&gQuestListRegion[i], 0, i);

    usPosY += usFontHeight;
  }

  // Setup mouse regions for the Fact lists
  usPosX = QUEST_DBS_SECOND_COL_NUMBER_X;
  usPosY = QUEST_DBS_SECOND_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET + QUEST_DBS_FACT_LIST_OFFSET;
  for (i = 0; i < QUEST_DBS_NUM_DISPLAYED_FACTS; i++) {
    MSYS_DefineRegion(&gFactListRegion[i], usPosX, usPosY,
                      (uint16_t)(usPosX + QUEST_DBS_SECOND_SECTION_WIDTH),
                      (uint16_t)(usPosY + usFontHeight), MSYS_PRIORITY_HIGH + 2, CURSOR_WWW,
                      MSYS_NO_CALLBACK, ScrollFactListRegionCallBack);  // CURSOR_LAPTOP_SCREEN
    // Add region
    MSYS_AddRegion(&gFactListRegion[i]);
    MSYS_SetRegionUserData(&gFactListRegion[i], 0, i);

    usPosY += usFontHeight;
  }

  // load Scroll Horizontal Arrow graphic and add it
  CHECKF(
      AddVObject(CreateVObjectFromFile("INTERFACE\\Qd_ScrollArrows.sti"), &guiQdScrollArrowImage));

  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\Bars.sti"), &guiBrownBackgroundForTeamPanel));

  gfRedrawQuestDebugSystem = TRUE;

  AddNPCsInSectorToArray();

  // Remove the mouse region over the clock
  RemoveMouseRegionForPauseOfClock();

  // Disable the buttons the depend on a seleted item or npc
  DisableButton(guiQuestDebugAddNpcToLocationButton);
  DisableButton(guiQuestDebugStartMercTalkingButtonButton);
  DisableButton(guiQuestDebugAddItemToLocationButton);
  DisableButton(guiQuestDebugGiveItemToNPCButton);
  DisableButton(guiQuestDebugViewNPCInvButton);
  DisableButton(guiQuestDebugNPCLogButtonButton);
  DisableButton(guiQuestDebugNPCRefreshButtonButton);

  if (giHaveSelectedNPC != -1) {
    wchar_t zItemDesc[SIZE_ITEM_INFO];

    if (gfUseLocalNPCs)
      swprintf(zItemDesc, ARR_SIZE(zItemDesc), L"%d - %s", gubCurrentNpcInSector[giHaveSelectedNPC],
               gMercProfiles[gubCurrentNpcInSector[giHaveSelectedNPC]].zNickname);
    else
      swprintf(zItemDesc, ARR_SIZE(zItemDesc), L"%d - %s", giHaveSelectedNPC,
               gMercProfiles[giHaveSelectedNPC].zNickname);
    SpecifyButtonText(guiQuestDebugCurNPCButton, zItemDesc);

    gNpcListBox.sCurSelectedItem = (int16_t)giHaveSelectedNPC;

    EnableQDSButtons();
  }

  if (giHaveSelectedItem != -1) {
    wchar_t zItemName[SIZE_ITEM_NAME];
    wchar_t zItemDesc[SIZE_ITEM_INFO];

    wcscpy(zItemName, ShortItemNames[giHaveSelectedItem]);

    swprintf(zItemDesc, ARR_SIZE(zItemDesc), L"%d - %s", giHaveSelectedItem, zItemName);
    SpecifyButtonText(guiQuestDebugCurItemButton, zItemDesc);

    gItemListBox.sCurSelectedItem = (int16_t)giHaveSelectedItem;

    EnableQDSButtons();
  }

  return (TRUE);
}

void ExitQuestDebugSystem() {
  uint16_t i;

  if (gfExitQdsDueToMessageBox) {
    return;
  }
  MSYS_RemoveRegion(&gQuestDebugSysScreenRegions);
  QuestDebug_EnterTactical();

  RemoveButton(guiQuestDebugExitButton);

  RemoveButton(guiQuestDebugCurNPCButton);
  RemoveButton(guiQuestDebugCurItemButton);
  RemoveButton(guiQuestDebugAddNpcToLocationButton);
  RemoveButton(guiQuestDebugAddItemToLocationButton);
  RemoveButton(guiQuestDebugChangeDayButton);
  RemoveButton(guiQuestDebugNPCLogButtonButton);
  RemoveButton(guiQuestDebugGiveItemToNPCButton);
  RemoveButton(guiQuestDebugViewNPCInvButton);
  RemoveButton(guiQuestDebugRestoreNPCInvButton);
  RemoveButton(guiQuestDebugAllOrSectorNPCToggle);
  RemoveButton(guiQuestDebugNPCRefreshButtonButton);
  RemoveButton(guiQuestDebugStartMercTalkingButtonButton);
  RemoveButton(guiQuestDebugAddNpcToTeamToggle);
  RemoveButton(guiQuestDebugRPCSaySectorDescToggle);

  RemoveButton(guiQDPgUpButtonButton);
  RemoveButton(guiQDPgDownButtonButton);

  DeleteVideoObjectFromIndex(guiQdScrollArrowImage);

  //	DeleteVideoObjectFromIndex( guiBrownBackgroundForTeamPanel );

  gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
  CreateDestroyDisplaySelectNpcDropDownBox();
  gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;

  // Remove the quest list mouse regions
  for (i = 0; i < QUEST_DBS_NUM_DISPLAYED_QUESTS; i++) MSYS_RemoveRegion(&gQuestListRegion[i]);

  // Remove the fact list mouse regions
  for (i = 0; i < QUEST_DBS_NUM_DISPLAYED_FACTS; i++) MSYS_RemoveRegion(&gFactListRegion[i]);

  // Create the clock mouse region
  CreateMouseRegionForPauseOfClock(CLOCK_REGION_START_X, CLOCK_REGION_START_Y);

  giHaveSelectedNPC = gNpcListBox.sCurSelectedItem;
  giHaveSelectedItem = gItemListBox.sCurSelectedItem;

  EndMercTalking();

  giSelectedMercCurrentQuote = -1;
}

void HandleQuestDebugSystem() {
  wchar_t zTemp[512];

  // hhh

  HandleQDSTalkingMerc();

  //	if( !gfTextEntryActive )
  if (gubTextEntryAction != QD_DROP_DOWN_NO_ACTION) {
  }

  if (gpActiveListBox->ubCurScrollBoxAction != QD_DROP_DOWN_NO_ACTION) {
    CreateDestroyDisplaySelectNpcDropDownBox();

    if (gpActiveListBox->ubCurScrollBoxAction == QD_DROP_DOWN_CREATE)
      gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DISPLAY;
    else
      gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
  }

  if (gubNPCInventoryPopupAction != QD_DROP_DOWN_NO_ACTION) {
    CreateDestroyDisplayNPCInventoryPopup(gubNPCInventoryPopupAction);

    if (gubNPCInventoryPopupAction == QD_DROP_DOWN_CREATE)
      gubNPCInventoryPopupAction = QD_DROP_DOWN_DISPLAY;
    else
      gubNPCInventoryPopupAction = QD_DROP_DOWN_NO_ACTION;
  }

  if (gfAddKeyNextPass) {
    swprintf(zTemp, ARR_SIZE(zTemp), L"  Please enter the Keys ID. ( 0 - %d )", NUM_KEYS);
    TextEntryBox(zTemp, AddKeyToGridNo);
    gfAddKeyNextPass = FALSE;
  }
}

void RenderQuestDebugSystem() {
  ColorFillVSurfaceArea(vsButtonDest, 0, 0, 640, 480, gusQuestDebugBlue);

  // display the title
  DisplayWrappedString(0, 5, 640, 2, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_TITLE,
                       QuestDebugText[QUEST_DBS_TITLE], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display vertical lines b/n sections
  DisplaySectionLine();

  // Display the Quest Text
  DisplayQuestInformation();

  // Display the Fact Text
  DisplayFactInformation();

  // Display the list of quests
  DisplayQuestList();

  // Display the list of tasks
  DisplayFactList();

  // Display the NPC and Item info
  DisplayNPCInfo();

  // Display the text beside the NPC in current sector toggle box
  DrawTextToScreen(QuestDebugText[QUEST_DBS_VIEW_LOCAL_NPC], QUEST_DBS_NPC_CHCKBOX_TGL_X + 25,
                   QUEST_DBS_NPC_CHCKBOX_TGL_Y + 1, QUEST_DBS_NUMBER_COL_WIDTH,
                   QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the text beside the add npc to team toggle box
  DrawTextToScreen(QuestDebugText[QUEST_DBS_ADD_NPC_TO_TEAM], QUEST_DBS_NPC_CHCKBOX_TGL_X + 25,
                   QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y + 1, QUEST_DBS_NUMBER_COL_WIDTH,
                   QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the text beside the rpc say sector desc quotes
  DrawTextToScreen(QuestDebugText[QUEST_DBS_RPC_SAY_SECTOR_DESC], QUEST_DBS_NPC_CHCKBOX_TGL_X + 25,
                   QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y, QUEST_DBS_NUMBER_COL_WIDTH,
                   QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  DisplayCurrentGridNo();
  // rr

  if (gfTextEntryActive) {
    gubTextEntryAction = QD_DROP_DOWN_DISPLAY;
    CreateDestroyDisplayTextEntryBox(gubTextEntryAction, NULL, NULL);
    gubTextEntryAction = QD_DROP_DOWN_NO_ACTION;
  }

  // if there is a merc talking
  if (giSelectedMercCurrentQuote != -1) DisplayQDSCurrentlyQuoteNum();

  MarkButtonsDirty();
  InvalidateRegion(0, 0, 640, 480);
}

void DisplayCurrentGridNo() {
  if (gsQdsEnteringGridNo != 0) {
    wchar_t zTemp[512];

    swprintf(zTemp, ARR_SIZE(zTemp), L"%s:  %d", QuestDebugText[QUEST_DBS_CURRENT_GRIDNO],
             gsQdsEnteringGridNo);
    DrawTextToScreen(zTemp, QUEST_DBS_NPC_CURRENT_GRIDNO_X, QUEST_DBS_NPC_CURRENT_GRIDNO_Y,
                     QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT,
                     QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }
}

void GetUserInput() {
  InputAtom Event;
  uint8_t ubPanelMercShouldUse = WhichPanelShouldTalkingMercUse();

  while (DequeueEvent(&Event)) {
    if (!HandleTextInput(&Event) && Event.usEvent == KEY_DOWN) {
      switch (Event.usParam) {
        case ESC:
          gubTextEntryAction = QD_DROP_DOWN_CANCEL;

          gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
          CreateDestroyDisplaySelectNpcDropDownBox();
          gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
          gfAddKeyNextPass = FALSE;

          EndMercTalking();

          break;

        case SPACE:
          if (giSelectedMercCurrentQuote != -1)
            SetTalkingMercPauseState((BOOLEAN)!gfPauseTalkingMercPopup);
          break;

        case LEFTARROW:
          if (giSelectedMercCurrentQuote != -1) {
            if (ubPanelMercShouldUse == QDS_REGULAR_PANEL)
              ShutupaYoFace(gTalkingMercSoldier->iFaceIndex);
            else
              ShutupaYoFace(gTalkPanel.iFaceIndex);

            if (giSelectedMercCurrentQuote > 1) {
              giSelectedMercCurrentQuote--;
              giSelectedMercCurrentQuote--;
            } else
              giSelectedMercCurrentQuote = 0;

            DisplayQDSCurrentlyQuoteNum();
          }
          break;

        case RIGHTARROW:
          if (giSelectedMercCurrentQuote != -1) {
            if (ubPanelMercShouldUse == QDS_REGULAR_PANEL)
              ShutupaYoFace(gTalkingMercSoldier->iFaceIndex);
            else
              ShutupaYoFace(gTalkPanel.iFaceIndex);

            // if( giSelectedMercCurrentQuote < GetMaxNumberOfQuotesToPlay( ) )
            //{
            //	giSelectedMercCurrentQuote++;
            //}
            DisplayQDSCurrentlyQuoteNum();
          }
          break;

        case F11:
          gfQuestDebugExit = TRUE;
          break;

        case 'x':
          if (Event.usKeyState & ALT_DOWN) {
            gfQuestDebugExit = TRUE;
            gfProgramIsRunning = FALSE;
          }
          break;

        case ENTER:
          if (gfTextEntryActive)
            gubTextEntryAction = QD_DROP_DOWN_DESTROY;
          else if (gfInDropDownBox) {
            gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
          }

          break;

        case PGDN:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox(
                (int16_t)(gpActiveListBox->sCurSelectedItem + QUEST_DBS_MAX_DISPLAYED_ENTRIES));
          }
          break;

        case PGUP:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox(
                (int16_t)(gpActiveListBox->sCurSelectedItem - QUEST_DBS_MAX_DISPLAYED_ENTRIES));
          }
          break;

        case DNARROW:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox((int16_t)(gpActiveListBox->sCurSelectedItem + 1));
          }
          break;

        case UPARROW:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox((int16_t)(gpActiveListBox->sCurSelectedItem - 1));
          }
          break;

        case 'd': {
          wchar_t zTemp[512];

          // toggle whether dropped items are damaged or not
          gfDropDamagedItems ^= 1;
          swprintf(zTemp, ARR_SIZE(zTemp), L"Items dropped will be in %s condition",
                   gfDropDamagedItems ? L"DAMAGED" : L"PERFECT");
          DoQDSMessageBox(MSG_BOX_BASIC_STYLE, zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);
        } break;
      }
    }

    else if (Event.usEvent == KEY_REPEAT) {
      switch (Event.usParam) {
        case PGDN:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox(
                (int16_t)(gpActiveListBox->sCurSelectedItem + QUEST_DBS_MAX_DISPLAYED_ENTRIES));
          }
          break;

        case PGUP:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox(
                (int16_t)(gpActiveListBox->sCurSelectedItem - QUEST_DBS_MAX_DISPLAYED_ENTRIES));
          }
          break;

        case DNARROW:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox((int16_t)(gpActiveListBox->sCurSelectedItem + 1));
          }
          break;

        case UPARROW:
          if (gfInDropDownBox) {
            IncrementActiveDropDownBox((int16_t)(gpActiveListBox->sCurSelectedItem - 1));
          }
          break;

        case LEFTARROW:
          if (giSelectedMercCurrentQuote != -1) {
            if (ubPanelMercShouldUse == QDS_REGULAR_PANEL)
              ShutupaYoFace(gTalkingMercSoldier->iFaceIndex);
            else
              ShutupaYoFace(gTalkPanel.iFaceIndex);

            if (giSelectedMercCurrentQuote > 1) {
              giSelectedMercCurrentQuote--;
              giSelectedMercCurrentQuote--;
            } else
              giSelectedMercCurrentQuote = 0;

            DisplayQDSCurrentlyQuoteNum();
          }
          break;

        case RIGHTARROW:
          if (giSelectedMercCurrentQuote != -1) {
            DisplayQDSCurrentlyQuoteNum();

            if (ubPanelMercShouldUse == QDS_REGULAR_PANEL)
              ShutupaYoFace(gTalkingMercSoldier->iFaceIndex);
            else
              ShutupaYoFace(gTalkPanel.iFaceIndex);
          }
          break;
      }
    }
  }
}

void QuestDebug_ExitTactical() {}

void QuestDebug_EnterTactical() { EnterTacticalScreen(); }

void DisplaySectionLine() {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usStartX;
  uint16_t usStartY;
  uint16_t usEndX;
  uint16_t usEndY;

  usStartX = usEndX = QUEST_DBS_FIRST_SECTION_WIDTH;

  usStartY = QUEST_DBS_FIRST_COL_NUMBER_Y;
  usEndY = 475;

  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);

  // draw the line in b/n the first and second section
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  LineDraw(FALSE, usStartX, usStartY, usEndX, usEndY, rgb32_to_rgb16(FROMRGB(255, 255, 255)),
           pDestBuf);

  // draw the line in b/n the second and third section
  usStartX = usEndX = QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH;
  LineDraw(FALSE, usStartX, usStartY, usEndX, usEndY, rgb32_to_rgb16(FROMRGB(255, 255, 255)),
           pDestBuf);

  // draw the horizopntal line under the title
  usStartX = 0;
  usEndX = 639;
  usStartY = usEndY = 75;
  LineDraw(FALSE, usStartX, usStartY, usEndX, usEndY, rgb32_to_rgb16(FROMRGB(255, 255, 255)),
           pDestBuf);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
}

void DisplayQuestInformation() {
  // Display Quests
  DisplayWrappedString(0, QUEST_DBS_SECTION_TITLE_Y, QUEST_DBS_FIRST_SECTION_WIDTH, 2,
                       QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_SUBTITLE,
                       QuestDebugText[QUEST_DBS_QUESTS], FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  // Display Quest Number text
  DisplayWrappedString(QUEST_DBS_FIRST_COL_NUMBER_X, QUEST_DBS_FIRST_COL_NUMBER_Y,
                       QUEST_DBS_NUMBER_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_QUEST_NUMBER],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display Quest title text
  DisplayWrappedString(QUEST_DBS_FIRST_COL_TITLE_X, QUEST_DBS_FIRST_COL_TITLE_Y,
                       QUEST_DBS_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_QUEST_TITLE],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display Quest status text
  DisplayWrappedString(QUEST_DBS_FIRST_COL_STATUS_X, QUEST_DBS_FIRST_COL_STATUS_Y,
                       QUEST_DBS_STATUS_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_STATUS],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
}

void DisplayFactInformation() {
  // Display Fact
  DisplayWrappedString(QUEST_DBS_FIRST_SECTION_WIDTH, QUEST_DBS_SECTION_TITLE_Y,
                       QUEST_DBS_SECOND_SECTION_WIDTH, 2, QUEST_DBS_FONT_TITLE,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_FACTS], FONT_MCOLOR_BLACK,
                       FALSE, CENTER_JUSTIFIED);

  // Display Fact Number text
  DisplayWrappedString(QUEST_DBS_SECOND_COL_NUMBER_X, QUEST_DBS_SECOND_COL_NUMBER_Y,
                       QUEST_DBS_NUMBER_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_FACT_NUMBER],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display Fact title text
  DisplayWrappedString(QUEST_DBS_SECOND_COL_TITLE_X, QUEST_DBS_SECOND_COL_TITLE_Y,
                       QUEST_DBS_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_DESC], FONT_MCOLOR_BLACK,
                       FALSE, CENTER_JUSTIFIED);

  // Display Fact status text
  DisplayWrappedString(QUEST_DBS_SECOND_COL_STATUS_X, QUEST_DBS_SECOND_COL_STATUS_Y,
                       QUEST_DBS_STATUS_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_STATUS],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
}

void BtnQuestDebugExitButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    gfQuestDebugExit = TRUE;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void DisplayQuestList() {
  uint16_t usLoop1, usCount;
  uint16_t usTextHeight = GetFontHeight(QUEST_DBS_FONT_DYNAMIC_TEXT) + 2;
  wchar_t sTemp[15];
  uint16_t usPosY;

  usPosY = QUEST_DBS_FIRST_COL_NUMBER_Y +
           QUEST_DBS_LIST_TEXT_OFFSET;  //&& (usCount < QUEST_DBS_MAX_DISPLAYED_ENTRIES )
  for (usLoop1 = 0, usCount = 0; (usLoop1 < MAX_QUESTS); usLoop1++) {
    // Display Quest Number text
    swprintf(sTemp, ARR_SIZE(sTemp), L"%02d", usLoop1);
    DrawTextToScreen(sTemp, QUEST_DBS_FIRST_COL_NUMBER_X, usPosY, QUEST_DBS_NUMBER_COL_WIDTH,
                     QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                     FALSE, LEFT_JUSTIFIED);

    // Display Quest title text
    DisplayWrappedString(QUEST_DBS_FIRST_COL_TITLE_X, usPosY, QUEST_DBS_TITLE_COL_WIDTH, 2,
                         QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT,
                         QuestDescText[usLoop1], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

    // Display Quest status text
    DisplayWrappedString(QUEST_DBS_FIRST_COL_STATUS_X, usPosY, QUEST_DBS_STATUS_COL_WIDTH, 2,
                         QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT,
                         QuestStates[gubQuest[usLoop1]], FONT_MCOLOR_BLACK, FALSE,
                         CENTER_JUSTIFIED);
    //		swprintf( sTemp, L"%02d", gubQuest[ usLoop1 ] );
    //		DrawTextToScreen( sTemp, QUEST_DBS_FIRST_COL_STATUS_X, usPosY,
    // QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT,
    // FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED	);

    usPosY += usTextHeight;
    usCount++;
  }
}

void DisplayFactList() {
  uint16_t usLoop1, usCount;
  uint16_t usTextHeight = GetFontHeight(QUEST_DBS_FONT_DYNAMIC_TEXT) + 2;
  wchar_t sTemp[512];
  uint16_t usPosY;

  usPosY =
      QUEST_DBS_SECOND_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET + QUEST_DBS_FACT_LIST_OFFSET;  //

  if (gusFactAtTopOfList + QUEST_DBS_NUM_DISPLAYED_FACTS > NUM_FACTS)
    gusFactAtTopOfList = NUM_FACTS - QUEST_DBS_NUM_DISPLAYED_FACTS;

  for (usLoop1 = gusFactAtTopOfList, usCount = 0;
       (usLoop1 < NUM_FACTS) && (usCount < QUEST_DBS_NUM_DISPLAYED_FACTS); usLoop1++) {
    // Display Quest Number text
    swprintf(sTemp, ARR_SIZE(sTemp), L"%02d", usLoop1);
    DrawTextToScreen(sTemp, QUEST_DBS_SECOND_COL_NUMBER_X, usPosY, QUEST_DBS_NUMBER_COL_WIDTH,
                     QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                     FALSE, LEFT_JUSTIFIED);

    // Display Quest title text
    if (FactDescText[usLoop1][0] == '\0') {
      swprintf(sTemp, ARR_SIZE(sTemp), L"No Fact %03d Yet", usLoop1);
      DisplayWrappedString(QUEST_DBS_SECOND_COL_TITLE_X, usPosY, QUEST_DBS_SECOND_TITLE_COL_WIDTH,
                           2, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, sTemp,
                           FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
    } else {
      wcscpy(sTemp, FactDescText[usLoop1]);

      if (StringPixLength(sTemp, QUEST_DBS_FONT_DYNAMIC_TEXT) > QUEST_DBS_SECOND_TITLE_COL_WIDTH) {
        ReduceStringLength(sTemp, ARR_SIZE(sTemp), QUEST_DBS_SECOND_TITLE_COL_WIDTH,
                           QUEST_DBS_FONT_DYNAMIC_TEXT);
      }

      //			DisplayWrappedString( QUEST_DBS_SECOND_COL_TITLE_X, usPosY,
      // QUEST_DBS_SECOND_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_DYNAMIC_TEXT,
      // QUEST_DBS_COLOR_STATIC_TEXT, FactDescText[ usLoop1 ], FONT_MCOLOR_BLACK, FALSE,
      // CENTER_JUSTIFIED	);
      DrawTextToScreen(sTemp, QUEST_DBS_SECOND_COL_TITLE_X, usPosY,
                       QUEST_DBS_SECOND_TITLE_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT,
                       QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
    }

    DrawTextToScreen(gubFact[usLoop1] ? L"True" : L"False", QUEST_DBS_SECOND_COL_STATUS_X, usPosY,
                     QUEST_DBS_STATUS_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT,
                     QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

    usPosY += usTextHeight;
    usCount++;
  }
}

void BtnQuestDebugCurNPCButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // if there is an old list box active, destroy the new one
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
    CreateDestroyDisplaySelectNpcDropDownBox();
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;

    // Set up the global list box
    gpActiveListBox = &gNpcListBox;

    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_CREATE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugCurItemButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // if there is an old list box active, destroy the new one
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
    CreateDestroyDisplaySelectNpcDropDownBox();
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;

    // Set up the global list box
    gpActiveListBox = &gItemListBox;

    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_CREATE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void DisplayNPCInfo() {
  // display section title
  DisplayWrappedString(QUEST_DBS_THIRD_COL_TITLE_X, QUEST_DBS_SECTION_TITLE_Y,
                       QUEST_DBS_THIRD_SECTION_WIDTH, 2, QUEST_DBS_FONT_TITLE,
                       QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_NPC_INFO],
                       FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
}

BOOLEAN CreateDestroyDisplaySelectNpcDropDownBox() {
  static BOOLEAN fMouseRegionsCreated = FALSE;
  uint16_t i;
  uint16_t usPosX, usPosY;

  // if there are
  if (gpActiveListBox->usMaxArrayIndex == 0) return (FALSE);

  switch (gpActiveListBox->ubCurScrollBoxAction) {
    case QD_DROP_DOWN_NO_ACTION: {
    } break;

    case QD_DROP_DOWN_CREATE: {
      uint16_t usFontHeight = GetFontHeight(QUEST_DBS_FONT_LISTBOX_TEXT) + 2;

      // if the mouse regions have already been creates, return
      if (fMouseRegionsCreated) break;

      // if the are more entries then can be displayed
      //			if( gpActiveListBox->usMaxArrayIndex >
      // gpActiveListBox->usNumDisplayedItems )
      //			{
      usPosX = gpActiveListBox->usScrollPosX;
      usPosY = gpActiveListBox->usScrollPosY;

      // Set the initial value for the box
      //				if( gpActiveListBox == &gNpcListBox )
      //					gpActiveListBox->sCurSelectedItem = FIRST_RPC;
      //				else
      //					gpActiveListBox->sCurSelectedItem = 1;

      // create the scroll regions
      for (i = 0; i < gpActiveListBox->usNumDisplayedItems; i++) {
        MSYS_DefineRegion(&gSelectedNpcListRegion[i], usPosX, (uint16_t)(usPosY),
                          (uint16_t)(usPosX + gpActiveListBox->usScrollWidth),
                          (uint16_t)(usPosY + usFontHeight), MSYS_PRIORITY_HIGH + 20, CURSOR_WWW,
                          SelectNpcListMovementCallBack, SelectNpcListRegionCallBack);
        MSYS_AddRegion(&gSelectedNpcListRegion[i]);
        MSYS_SetRegionUserData(&gSelectedNpcListRegion[i], 0, i);

        usPosY += usFontHeight;
      }

      fMouseRegionsCreated = TRUE;
      //			}
      //			else
      //				fMouseRegionsCreated = FALSE;

      // Scroll bars
      usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
      usPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollArrowHeight + 2;

      for (i = 0; i < QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR; i++) {
        MSYS_DefineRegion(&gScrollAreaRegion[i], usPosX, usPosY,
                          (uint16_t)(usPosX + gpActiveListBox->usScrollBarWidth),
                          (uint16_t)(usPosY + gpActiveListBox->usScrollBarHeight),
                          MSYS_PRIORITY_HIGH + 20, CURSOR_WWW, ScrollAreaMovementCallBack,
                          ScrollAreaRegionCallBack);
        MSYS_AddRegion(&gScrollAreaRegion[i]);
        MSYS_SetRegionUserData(&gScrollAreaRegion[i], 0, i);
      }

      // Top Scroll arrow
      usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
      usPosY = gpActiveListBox->usScrollPosY + 2;

      MSYS_DefineRegion(&gScrollArrowsRegion[0], usPosX, (uint16_t)(usPosY),
                        (uint16_t)(usPosX + gpActiveListBox->usScrollBarWidth),
                        (uint16_t)(usPosY + gpActiveListBox->usScrollArrowHeight),
                        MSYS_PRIORITY_HIGH + 20, CURSOR_WWW, MSYS_NO_CALLBACK,
                        ScrollArrowsRegionCallBack);
      MSYS_AddRegion(&gScrollArrowsRegion[0]);
      MSYS_SetRegionUserData(&gScrollArrowsRegion[0], 0, 0);

      // Bottom Scroll arrow
      usPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollHeight -
               gpActiveListBox->usScrollArrowHeight - 2;

      MSYS_DefineRegion(&gScrollArrowsRegion[1], usPosX, usPosY,
                        (uint16_t)(usPosX + gpActiveListBox->usScrollBarWidth),
                        (uint16_t)(usPosY + gpActiveListBox->usScrollArrowHeight),
                        MSYS_PRIORITY_HIGH + 20, CURSOR_WWW, MSYS_NO_CALLBACK,
                        ScrollArrowsRegionCallBack);
      MSYS_AddRegion(&gScrollArrowsRegion[1]);
      MSYS_SetRegionUserData(&gScrollArrowsRegion[1], 0, 1);

      // create a mask to block out the screen
      if (!gfBackgroundMaskEnabled) {
        MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, 640, 480,
                          MSYS_PRIORITY_HIGH + 15, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                          QuestDebugTextEntryDisableScreenRegionCallBack);
        MSYS_AddRegion(&gQuestTextEntryDebugDisableScreenRegion);
        gfBackgroundMaskEnabled = TRUE;
      }

      gfInDropDownBox = TRUE;

      if (gpActiveListBox->sCurSelectedItem == -1) {
        gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usStartIndex;
        gpActiveListBox->sCurSelectedItem = gpActiveListBox->usStartIndex;
      } else
        gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->sCurSelectedItem;

    } break;

    case QD_DROP_DOWN_DESTROY: {
      // if the mouse regions are creates, destroy them
      if (fMouseRegionsCreated) {
        // delete the mouse regions for the words
        for (i = 0; i < gpActiveListBox->usNumDisplayedItems; i++)
          MSYS_RemoveRegion(&gSelectedNpcListRegion[i]);

        fMouseRegionsCreated = FALSE;

        // scroll arrows
        for (i = 0; i < 2; i++) MSYS_RemoveRegion(&gScrollArrowsRegion[i]);

        for (i = 0; i < QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR; i++) {
          MSYS_RemoveRegion(&gScrollAreaRegion[i]);
        }

        // remove the mask of the entire screen
        if (gfBackgroundMaskEnabled) {
          MSYS_RemoveRegion(&gQuestTextEntryDebugDisableScreenRegion);
          gfBackgroundMaskEnabled = FALSE;
        }

        EnableQDSButtons();
      }
      gfRedrawQuestDebugSystem = TRUE;
      gfInDropDownBox = FALSE;
    } break;

    case QD_DROP_DOWN_DISPLAY: {
      //			( *(gpActiveListBox->DisplayFunction))();
      //			(*(MSYS_CurrRegion->ButtonCallback))(MSYS_CurrRegion,ButtonReason);
      DisplaySelectedListBox();
    } break;
  }
  return (TRUE);
}

void DisplaySelectedListBox() {
  uint16_t usPosX, usPosY;
  struct VObject *hImageHandle;

  // DEBUG: make sure it wont go over array bounds
  if (gpActiveListBox->usMaxArrayIndex == 0) {
    return;
  } else {
    if (gpActiveListBox->sCurSelectedItem >= gpActiveListBox->usMaxArrayIndex) {
      if (gpActiveListBox->usMaxArrayIndex > 0)
        gpActiveListBox->sCurSelectedItem = gpActiveListBox->usMaxArrayIndex - 1;
      else
        gpActiveListBox->sCurSelectedItem = 0;

      if ((int16_t)(gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usNumDisplayedItems - 1) <
          0)
        gpActiveListBox->usItemDisplayedOnTopOfList = 0;
      else
        gpActiveListBox->usItemDisplayedOnTopOfList =
            gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usNumDisplayedItems - 1;
    } else if (!gfUseLocalNPCs &&
               ((gpActiveListBox->usItemDisplayedOnTopOfList +
                 gpActiveListBox->usMaxNumDisplayedItems) >= gpActiveListBox->usMaxArrayIndex)) {
      gpActiveListBox->usItemDisplayedOnTopOfList =
          gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems - 1;
    }
  }

  usPosX = gpActiveListBox->usScrollPosX;
  usPosY = gpActiveListBox->usScrollPosY + 2;

  // clear the background
  ColorFillVSurfaceArea(vsFB, usPosX, usPosY - 1, usPosX + gpActiveListBox->usScrollWidth,
                        usPosY + gpActiveListBox->usScrollHeight,
                        rgb32_to_rgb16(FROMRGB(45, 59, 74)));

  // Display the selected list box's display function
  (*(gpActiveListBox->DisplayFunction))();

  // Display the Scroll BAr area
  // clear the scroll bar background
  usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
  usPosY = gpActiveListBox->usScrollPosY + 2;

  ColorFillVSurfaceArea(vsFB, usPosX, usPosY - 1, usPosX + gpActiveListBox->usScrollBarWidth,
                        usPosY + gpActiveListBox->usScrollHeight,
                        rgb32_to_rgb16(FROMRGB(192, 192, 192)));

  // get and display the up and down arrows
  GetVideoObject(&hImageHandle, guiQdScrollArrowImage);
  // top arrow
  BltVideoObject(vsFB, hImageHandle, 0, usPosX - 5, usPosY - 1);

  // Bottom arrow
  BltVideoObject(vsFB, hImageHandle, 1, usPosX,
                 usPosY + gpActiveListBox->usScrollHeight - gpActiveListBox->usScrollArrowHeight);

  // display the scroll rectangle
  DrawQdsScrollRectangle();  // gpActiveListBox->sCurSelectedItem, usPosX, usPosY, (uint16_t)(usPosY
                             // + gpActiveListBox->usScrollHeight), NUM_PROFILES-FIRST_RPC );

  InvalidateRegion(0, 0, 640, 480);
}

void DisplaySelectedNPC() {
  uint16_t i;
  uint16_t usPosX, usPosY;
  int16_t usLocationX = 0, usLocationY = 0;
  uint16_t usFontHeight = GetFontHeight(QUEST_DBS_FONT_LISTBOX_TEXT) + 2;
  wchar_t sTempString[64];
  wchar_t zButtonName[256];

  usPosX = gpActiveListBox->usScrollPosX;
  usPosY = gpActiveListBox->usScrollPosY + 2;

  // display the names of the NPC's
  for (i = gpActiveListBox->usItemDisplayedOnTopOfList;
       i < gpActiveListBox->usItemDisplayedOnTopOfList + gpActiveListBox->usNumDisplayedItems;
       i++) {
    if (gfUseLocalNPCs) {
      DrawTextToScreen(gMercProfiles[gubCurrentNpcInSector[i]].zNickname, usPosX, usPosY, 0,
                       QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                       FALSE, LEFT_JUSTIFIED);

      GetDebugLocationString(gubCurrentNpcInSector[i], sTempString, ARR_SIZE(sTempString));

    } else {
      DrawTextToScreen(gMercProfiles[i].zNickname, usPosX, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT,
                       QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
      GetDebugLocationString(i, sTempString, ARR_SIZE(sTempString));
    }

    FindFontRightCoordinates(gpActiveListBox->usScrollPosX, usPosY, gpActiveListBox->usScrollWidth,
                             0, sTempString, QUEST_DBS_FONT_LISTBOX_TEXT, &usLocationX,
                             &usLocationY);

    // the location value
    DrawTextToScreen(sTempString, (uint16_t)(usLocationX - 2), usPosY, 0,
                     QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK,
                     FALSE, LEFT_JUSTIFIED);

    usPosY += usFontHeight;
  }

  // if there is a selected item, highlight it.
  if (gpActiveListBox->sCurSelectedItem >= 0) {
    usPosY = usFontHeight *
                 (gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList) +
             gpActiveListBox->usScrollPosY + 2;

    if (usPosY > 424) usPosY = usPosY;

    // display the name in the list
    ColorFillVSurfaceArea(vsFB, gpActiveListBox->usScrollPosX, usPosY - 1,
                          gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth,
                          usPosY + usFontHeight - 1, rgb32_to_rgb16(FROMRGB(255, 255, 255)));

    SetFontShadow(NO_SHADOW);

    // the highlighted name
    if (gfUseLocalNPCs) {
      DrawTextToScreen(
          gMercProfiles[gubCurrentNpcInSector[gpActiveListBox->sCurSelectedItem]].zNickname,
          gpActiveListBox->usScrollPosX, (uint16_t)(usPosY), 0, QUEST_DBS_FONT_LISTBOX_TEXT, 2,
          FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
      GetDebugLocationString(gubCurrentNpcInSector[gpActiveListBox->sCurSelectedItem], sTempString,
                             ARR_SIZE(sTempString));
    } else {
      DrawTextToScreen(gMercProfiles[gpActiveListBox->sCurSelectedItem].zNickname,
                       gpActiveListBox->usScrollPosX, (uint16_t)(usPosY), 0,
                       QUEST_DBS_FONT_LISTBOX_TEXT, 2, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

      GetDebugLocationString(gpActiveListBox->sCurSelectedItem, sTempString, ARR_SIZE(sTempString));
    }

    FindFontRightCoordinates(gpActiveListBox->usScrollPosX, (uint16_t)(usPosY),
                             gpActiveListBox->usScrollWidth, 0, sTempString,
                             QUEST_DBS_FONT_LISTBOX_TEXT, &usLocationX, &usLocationY);

    // the location value
    DrawTextToScreen(sTempString, usLocationX, (uint16_t)(usPosY), 0, QUEST_DBS_FONT_LISTBOX_TEXT,
                     2, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

    SetFontShadow(DEFAULT_SHADOW);

    if (gfUseLocalNPCs)
      swprintf(zButtonName, ARR_SIZE(zButtonName), L"%d - %s",
               gubCurrentNpcInSector[gpActiveListBox->sCurSelectedItem],
               gMercProfiles[gubCurrentNpcInSector[gpActiveListBox->sCurSelectedItem]].zNickname);
    else
      swprintf(zButtonName, ARR_SIZE(zButtonName), L"%d - %s", gpActiveListBox->sCurSelectedItem,
               gMercProfiles[gpActiveListBox->sCurSelectedItem].zNickname);

    SpecifyButtonText(guiQuestDebugCurNPCButton, zButtonName);
  }

  SetFontShadow(DEFAULT_SHADOW);
}

void DisplaySelectedItem() {
  uint16_t i;
  uint16_t usPosX, usPosY;
  uint16_t usFontHeight = GetFontHeight(QUEST_DBS_FONT_LISTBOX_TEXT) + 2;
  wchar_t zItemName[SIZE_ITEM_NAME];
  //	wchar_t	zItemDesc[ SIZE_ITEM_INFO ];

  wchar_t zButtonName[256];

  usPosX = gpActiveListBox->usScrollPosX;
  usPosY = gpActiveListBox->usScrollPosY + 2;

  // display the names of the NPC's
  for (i = gpActiveListBox->usItemDisplayedOnTopOfList;
       i < gpActiveListBox->usItemDisplayedOnTopOfList + gpActiveListBox->usNumDisplayedItems;
       i++) {
    //		if ( !LoadItemInfo( i, zItemName, zItemDesc ) )
    //			Assert(0);
    wcscpy(zItemName, ShortItemNames[i]);

    if (zItemName[0] == '\0') wcscpy(zItemName, QuestDebugText[QUEST_DBS_NO_ITEM]);

    DrawTextToScreen(zItemName, usPosX, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT,
                     QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    usPosY += usFontHeight;
  }

  // if there is a selected item, highlight it.
  if (gpActiveListBox->sCurSelectedItem >= 0) {
    usPosY = usFontHeight *
                 (gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList) +
             gpActiveListBox->usScrollPosY + 2;

    // display the name in the list
    ColorFillVSurfaceArea(vsFB, gpActiveListBox->usScrollPosX, usPosY - 1,
                          gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth,
                          usPosY + usFontHeight - 1, rgb32_to_rgb16(FROMRGB(255, 255, 255)));

    SetFontShadow(NO_SHADOW);

    //		if ( !LoadItemInfo( gpActiveListBox->sCurSelectedItem, zItemName, zItemDesc ) )
    //			Assert(0);
    wcscpy(zItemName, ShortItemNames[gpActiveListBox->sCurSelectedItem]);

    if (zItemName[0] == '\0') wcscpy(zItemName, QuestDebugText[QUEST_DBS_NO_ITEM]);

    DrawTextToScreen(zItemName, gpActiveListBox->usScrollPosX, (uint16_t)(usPosY), 0,
                     QUEST_DBS_FONT_LISTBOX_TEXT, 2, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    SetFontShadow(DEFAULT_SHADOW);

    swprintf(zButtonName, ARR_SIZE(zButtonName), L"%d - %s", gpActiveListBox->sCurSelectedItem,
             zItemName);

    SpecifyButtonText(guiQuestDebugCurItemButton, zButtonName);
  }

  SetFontShadow(DEFAULT_SHADOW);
}

void SelectNpcListRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint8_t ubSelected = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;  // qq
    gpActiveListBox->sCurSelectedItem = ubSelected + gpActiveListBox->usItemDisplayedOnTopOfList;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
    CreateDestroyDisplaySelectNpcDropDownBox();
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
  }
}

void SelectNpcListMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    int16_t sSelected = (int16_t)MSYS_GetRegionUserData(pRegion, 0);  // + gubCityAtTopOfList;

    pRegion->uiFlags |= BUTTON_CLICKED_ON;

    gpActiveListBox->sCurSelectedItem = sSelected + gpActiveListBox->usItemDisplayedOnTopOfList;

    // if we are at the top of the list
    //		if( sSelected == 0 )
    //			IncrementActiveDropDownBox( (int16_t)(gpActiveListBox->sCurSelectedItem - 1
    //)
    //);

    // else we are at the bottom of the list
    //		else if( sSelected == gpActiveListBox->usMaxNumDisplayedItems - 1 )
    //			IncrementActiveDropDownBox( (int16_t)(gpActiveListBox->sCurSelectedItem + 1
    //)
    //);

    DisplaySelectedListBox();

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_MOVE) {
    int16_t sSelected = (int16_t)MSYS_GetRegionUserData(pRegion, 0);  // + gubCityAtTopOfList;

    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);

    if (gpActiveListBox->sCurSelectedItem !=
        (sSelected + gpActiveListBox->usItemDisplayedOnTopOfList)) {
      gpActiveListBox->sCurSelectedItem = sSelected + gpActiveListBox->usItemDisplayedOnTopOfList;

      DisplaySelectedListBox();
    }

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

void DrawQdsScrollRectangle()  // int16_t sSelectedEntry, uint16_t usStartPosX, uint16_t
                               // usStartPosY, uint16_t usScrollAreaHeight, uint16_t usNumEntries )
{
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usWidth, usTempPosY;
  uint16_t usHeight, usPosY, usPosX;

  uint16_t usNumEntries = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usStartIndex - 1;

  uint16_t temp;

  usTempPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollArrowHeight;
  usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
  usWidth = gpActiveListBox->usScrollBarWidth;

  usHeight =
      (uint16_t)(gpActiveListBox->usScrollBarHeight / (float)(usNumEntries) + .5);  // qq+ 1 );

  if (usNumEntries > gpActiveListBox->usMaxNumDisplayedItems)
    usPosY = usTempPosY +
             (uint16_t)((gpActiveListBox->usScrollBarHeight / (float)(usNumEntries + 1)) *
                        (gpActiveListBox->sCurSelectedItem - gpActiveListBox->usStartIndex));
  else
    usPosY = usTempPosY;

  // bottom
  temp = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollBarHeight +
         gpActiveListBox->usScrollArrowHeight;

  if (usPosY >= temp)
    usPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollBarHeight +
             gpActiveListBox->usScrollArrowHeight - usHeight;

  gpActiveListBox->usScrollBoxY = usPosY;
  gpActiveListBox->usScrollBoxEndY = usPosY + usHeight;

  ColorFillVSurfaceArea(vsFB, usPosX, usPosY, usPosX + usWidth - 1, usPosY + usHeight,
                        rgb32_to_rgb16(FROMRGB(130, 132, 128)));

  // display the line
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

  // draw the gold highlite line on the top and left
  LineDraw(FALSE, usPosX, usPosY, usPosX + usWidth - 1, usPosY,
           rgb32_to_rgb16(FROMRGB(255, 255, 255)), pDestBuf);
  LineDraw(FALSE, usPosX, usPosY, usPosX, usPosY + usHeight, rgb32_to_rgb16(FROMRGB(255, 255, 255)),
           pDestBuf);

  // draw the shadow line on the bottom and right
  LineDraw(FALSE, usPosX, usPosY + usHeight, usPosX + usWidth - 1, usPosY + usHeight,
           rgb32_to_rgb16(FROMRGB(112, 110, 112)), pDestBuf);
  LineDraw(FALSE, usPosX + usWidth - 1, usPosY, usPosX + usWidth - 1, usPosY + usHeight,
           rgb32_to_rgb16(FROMRGB(112, 110, 112)), pDestBuf);

  // unlock frame buffer
  JSurface_Unlock(vsFB);
}

void ScrollArrowsRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) ||
             (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)) {
    uint8_t ubSelected = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    // if down arrow
    if (ubSelected) {
      // if not at end of list
      if (gpActiveListBox->sCurSelectedItem < gpActiveListBox->usMaxArrayIndex - 1)
        gpActiveListBox->sCurSelectedItem++;

      // if end of displayed list, increment top of list
      if ((gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList) >=
          gpActiveListBox->usNumDisplayedItems)
        gpActiveListBox->usItemDisplayedOnTopOfList++;
    }

    // else, up arrow
    else {
      // if not at end of list
      if (gpActiveListBox->sCurSelectedItem > gpActiveListBox->usStartIndex)
        gpActiveListBox->sCurSelectedItem--;

      // if top of displayed list
      if (gpActiveListBox->sCurSelectedItem < gpActiveListBox->usItemDisplayedOnTopOfList)
        gpActiveListBox->usItemDisplayedOnTopOfList--;
    }

    DisplaySelectedListBox();
  }
}

void ScrollAreaRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    CalcPositionOfNewScrollBoxLocation();
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
    CalcPositionOfNewScrollBoxLocation();
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
    CreateDestroyDisplaySelectNpcDropDownBox();
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
  }
}

void ScrollAreaMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    pRegion->uiFlags &= (~BUTTON_CLICKED_ON);

    //		CalcPositionOfNewScrollBoxLocation();

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    //		CalcPositionOfNewScrollBoxLocation();

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_MOVE) {
    if (gfLeftButtonState) {
      CalcPositionOfNewScrollBoxLocation();
    }

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

void CalcPositionOfNewScrollBoxLocation() {
  int16_t sMouseYPos;
  int16_t sIncrementValue;
  float dValue;
  int16_t sHeight = 0;
  int16_t sStartPosOfScrollArea =
      gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollArrowHeight;

  sMouseYPos = gusMouseYPos;

  // if we have to scroll
  if (sMouseYPos > sStartPosOfScrollArea ||
      sMouseYPos < (sStartPosOfScrollArea + gpActiveListBox->usScrollBarHeight)) {
    // Calculate the number of items we have to move
    sHeight = sMouseYPos - sStartPosOfScrollArea;

    dValue = sHeight / (float)(gpActiveListBox->usScrollBarHeight);
    sIncrementValue =
        (int16_t)((dValue) * (gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usStartIndex) +
                  .5) +
        gpActiveListBox->usStartIndex;

    IncrementActiveDropDownBox(sIncrementValue);
  }

  DisplaySelectedListBox();
}

void BtnQuestDebugAddNpcToLocationButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    wchar_t zTemp[512];
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    swprintf(zTemp, ARR_SIZE(zTemp), L"%s where %s will be added.",
             QuestDebugText[QUEST_DBS_ENTER_GRID_NUM],
             gMercProfiles[gNpcListBox.sCurSelectedItem].zNickname);
    TextEntryBox(zTemp, AddNPCToGridNo);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugAddItemToLocationButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    wchar_t zTemp[512];
    wchar_t zItemName[SIZE_ITEM_NAME];
    //		wchar_t	zItemDesc[ SIZE_ITEM_INFO ];
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    //		if ( !LoadItemInfo( gItemListBox.sCurSelectedItem, zItemName, zItemDesc ) )
    //			Assert(0);
    wcscpy(zItemName, ShortItemNames[gItemListBox.sCurSelectedItem]);

    swprintf(zTemp, ARR_SIZE(zTemp), L"%s where the %s will be added.",
             QuestDebugText[QUEST_DBS_ENTER_GRID_NUM], zItemName);
    TextEntryBox(zTemp, AddItemToGridNo);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugGiveItemToNPCButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    struct SOLDIERTYPE *pSoldier;
    struct OBJECTTYPE Object;

    CreateItem(gItemListBox.sCurSelectedItem, 100, &Object);

    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // if the selected merc is created
    if (gfUseLocalNPCs)
      pSoldier = FindSoldierByProfileID(gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem], FALSE);
    else
      pSoldier = FindSoldierByProfileID((uint8_t)gNpcListBox.sCurSelectedItem, FALSE);

    if (!pSoldier) {
      // Failed to get npc, put error message
      return;
    }

    // Give the selected item to the selected merc
    if (!AutoPlaceObject(pSoldier, &Object, TRUE)) {
      // failed to add item, put error message to screen
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

void BtnQuestDebugChangeDayButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    wchar_t zTemp[512];

    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    swprintf(zTemp, ARR_SIZE(zTemp), L"%s   Current Day is %d",
             QuestDebugText[QUEST_DBS_PLEASE_ENTER_DAY], GetWorldDay());

    // get the day to change the game day to
    TextEntryBox(zTemp, ChangeDayNumber);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugViewNPCInvButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    gubNPCInventoryPopupAction = QD_DROP_DOWN_CREATE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugRestoreNPCInvButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // loop through all the active NPC's and refresh their inventory
    RefreshAllNPCInventory();

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugNPCLogButtonButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    wchar_t zName[128];

    //		btn->uiFlags &= (~BUTTON_CLICKED_ON );

    if (gfNpcLogButton) {
      gfNpcLogButton = FALSE;
      btn->uiFlags &= (~BUTTON_CLICKED_ON);
    } else {
      gfNpcLogButton = TRUE;
      btn->uiFlags |= BUTTON_CLICKED_ON;
    }

    swprintf(zName, ARR_SIZE(zName), L"%s - (%s)", QuestDebugText[QUEST_DBS_NPC_LOG_BUTTON],
             gfNpcLogButton ? L"On" : L"Off");
    SpecifyButtonText(guiQuestDebugNPCLogButtonButton, zName);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugNPCRefreshButtonButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    BOOLEAN fRetVal = FALSE;
    wchar_t zTemp[128];
    uint8_t ubMercID = 0;

    if (gfUseLocalNPCs) {
      ubMercID = gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem];
      fRetVal = ReloadQuoteFile(gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem]);
    } else {
      if (gNpcListBox.sCurSelectedItem != -1) {
        // NB ubMercID is really profile ID
        ubMercID = (uint8_t)gNpcListBox.sCurSelectedItem;
        fRetVal = ReloadQuoteFile((uint8_t)gNpcListBox.sCurSelectedItem);
        gMercProfiles[ubMercID].ubLastDateSpokenTo = 0;
      }
    }

    // if the function succeded
    if (fRetVal) {
      swprintf(zTemp, ARR_SIZE(zTemp), L"%s %s", QuestDebugText[QUEST_DBS_REFRESH_OK],
               gMercProfiles[ubMercID].zNickname);
    } else {
      swprintf(zTemp, ARR_SIZE(zTemp), L"%s %s", QuestDebugText[QUEST_DBS_REFRESH_FAILED],
               gMercProfiles[ubMercID].zNickname);
    }

    DoQDSMessageBox(MSG_BOX_BASIC_STYLE, zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);

    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugStartMercTalkingButtonButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // Ask for the initial quote num to start talking from
    //		DoQDSMessageBox( MSG_BOX_BASIC_STYLE, zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK,
    // NULL
    //);

    // set the initial value
    gsQdsEnteringGridNo = 0;

    TextEntryBox(QuestDebugText[QUEST_DBS_START_MERC_TALKING_FROM], StartMercTalkingFromQuoteNum);

    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

BOOLEAN CreateDestroyDisplayTextEntryBox(uint8_t ubAction, wchar_t *pString,
                                         TEXT_ENTRY_CALLBACK EntryCallBack) {
  static BOOLEAN fMouseRegionCreated = FALSE;
  static wchar_t zString[256];
  static TEXT_ENTRY_CALLBACK TextEntryCallback;

  switch (ubAction) {
    case QD_DROP_DOWN_NO_ACTION: {
    } break;

    case QD_DROP_DOWN_CREATE: {
      if (fMouseRegionCreated) break;

      fMouseRegionCreated = TRUE;

      // create a mask to block out the screen
      if (!gfBackgroundMaskEnabled) {
        MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, 640, 480,
                          MSYS_PRIORITY_HIGH + 40, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                          QuestDebugTextEntryDisableScreenRegionCallBack);
        MSYS_AddRegion(&gQuestTextEntryDebugDisableScreenRegion);
        gfBackgroundMaskEnabled = TRUE;
      }

      // create the ok button
      guiQuestDebugTextEntryOkBtn = CreateTextButton(
          L"OK", QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK,
          BUTTON_USE_DEFAULT, QUEST_DBS_TEB_X + QUEST_DBS_TEB_WIDTH / 2 - 12,
          QUEST_DBS_TEB_Y + QUEST_DBS_TEB_HEIGHT - 30, 30, 25, BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGH + 50, BUTTON_NO_CALLBACK, BtnQuestDebugTextEntryOkBtnButtonCallback);
      SetButtonCursor(guiQuestDebugTextEntryOkBtn, CURSOR_WWW);

      wcscpy(zString, pString);

      gfTextEntryActive = TRUE;

      InitQuestDebugTextInputBoxes();

      TextEntryCallback = EntryCallBack;
    } break;

    case QD_DROP_DOWN_CANCEL:
    case QD_DROP_DOWN_DESTROY: {
      wchar_t zText[32];
      int32_t iTextEntryNumber;

      if (!fMouseRegionCreated) break;

      // Remove the mouse region that disables the screen
      if (gfBackgroundMaskEnabled) {
        MSYS_RemoveRegion(&gQuestTextEntryDebugDisableScreenRegion);
        gfBackgroundMaskEnabled = FALSE;
      }

      // remove the 'ok' button on the text entry field
      RemoveButton(guiQuestDebugTextEntryOkBtn);

      // Mouse regions are removed
      fMouseRegionCreated = FALSE;
      gfTextEntryActive = FALSE;

      // redraw the entire screen
      gfRedrawQuestDebugSystem = TRUE;

      // get the striong from the text field
      Get16BitStringFromField(0, zText, ARR_SIZE(zText));

      // if the text is not null
      if (zText[0] != '\0') {
        // get the number from the string
        swscanf(zText, L"%ld", &iTextEntryNumber);
      } else
        iTextEntryNumber = 0;

      // remove the text input field
      DestroyQuestDebugTextInputBoxes();

      if (ubAction != QD_DROP_DOWN_CANCEL) (*(TextEntryCallback))(iTextEntryNumber);

    } break;

    case QD_DROP_DOWN_DISPLAY: {
      // Display the text entry box frame
      ColorFillVSurfaceArea(
          vsFB, QUEST_DBS_TEB_X, QUEST_DBS_TEB_Y, QUEST_DBS_TEB_X + QUEST_DBS_TEB_WIDTH,
          QUEST_DBS_TEB_Y + QUEST_DBS_TEB_HEIGHT, rgb32_to_rgb16(FROMRGB(45, 59, 74)));

      // Display the text box caption
      DisplayWrappedString(QUEST_DBS_TEB_X + 10, QUEST_DBS_TEB_Y + 10, QUEST_DBS_TEB_WIDTH - 20, 2,
                           QUEST_DBS_FONT_TEXT_ENTRY, QUEST_DBS_COLOR_TEXT_ENTRY, zString,
                           FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

      InvalidateRegion(QUEST_DBS_TEB_X, QUEST_DBS_TEB_Y, QUEST_DBS_TEB_X + QUEST_DBS_TEB_WIDTH,
                       QUEST_DBS_TEB_Y + QUEST_DBS_TEB_HEIGHT);
    } break;
  }

  return (TRUE);
}

void QuestDebugTextEntryDisableScreenRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
    CreateDestroyDisplaySelectNpcDropDownBox();
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
    CreateDestroyDisplaySelectNpcDropDownBox();
    gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
  }
}

void BtnQuestDebugTextEntryOkBtnButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    gubTextEntryAction = QD_DROP_DOWN_DESTROY;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void TextEntryBox(wchar_t *pString, TEXT_ENTRY_CALLBACK TextEntryCallBack) {
  CreateDestroyDisplayTextEntryBox(QD_DROP_DOWN_CREATE, pString, TextEntryCallBack);
  gubTextEntryAction = QD_DROP_DOWN_DISPLAY;
}

void ScrollQuestListRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    wchar_t String[512];

    gubCurQuestSelected = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

    // qqq
    swprintf(String, ARR_SIZE(String), L"%s %s %d \"%s\" ( %s )",
             QuestDebugText[QUEST_DBS_ENTER_NEW_VALUE], QuestDebugText[QUEST_DBS_QUEST_NUM],
             gubCurQuestSelected, QuestDescText[gubCurQuestSelected],
             QuestDebugText[QUEST_DBS_0_1_2]);

    TextEntryBox(String, ChangeQuestState);

  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
  }
}

void ScrollFactListRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    wchar_t String[512];

    gusCurFactSelected = (uint8_t)MSYS_GetRegionUserData(pRegion, 0) + gusFactAtTopOfList;

    if (FactDescText[gusCurFactSelected][0] == '\0')
      swprintf(String, ARR_SIZE(String), L"%s %s %d \"%s\" ( %s )",
               QuestDebugText[QUEST_DBS_ENTER_NEW_VALUE], QuestDebugText[QUEST_DBS_FACT_NUM],
               gusCurFactSelected, QuestDebugText[QUEST_DBS_NO_TEXT],
               QuestDebugText[QUEST_DBS_0_1]);
    else
      swprintf(String, ARR_SIZE(String), L"%s %s %d \"%s\" ( %s )",
               QuestDebugText[QUEST_DBS_ENTER_NEW_VALUE], QuestDebugText[QUEST_DBS_FACT_NUM],
               gusCurFactSelected, FactDescText[gusCurFactSelected], QuestDebugText[QUEST_DBS_0_1]);

    TextEntryBox(String, ChangeFactState);

  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT) {
  }
}

void InitQuestDebugTextInputBoxes() {
  wchar_t sTemp[640];
  //	wchar_t	sText[ 640 ];

  InitTextInputMode();
  SetTextInputFont((uint16_t)FONT12ARIAL);
  Set16BPPTextFieldColor(rgb32_to_rgb16(FROMRGB(255, 255, 255)));
  SetBevelColors(rgb32_to_rgb16(FROMRGB(136, 138, 135)), rgb32_to_rgb16(FROMRGB(24, 61, 81)));
  SetTextInputRegularColors(2, FONT_WHITE);
  SetTextInputHilitedColors(FONT_WHITE, 2, 141);
  SetCursorColor(rgb32_to_rgb16(FROMRGB(0, 0, 0)));
  swprintf(sTemp, ARR_SIZE(sTemp), L"%d", gsQdsEnteringGridNo);

  // Text entry field
  AddTextInputField(QUEST_DBS_TEB_X + QUEST_DBS_TEB_WIDTH / 2 - 30, QUEST_DBS_TEB_Y + 65, 60, 15,
                    MSYS_PRIORITY_HIGH + 60, sTemp, QUEST_DBS_TEXT_FIELD_WIDTH,
                    INPUTTYPE_NUMERICSTRICT);
}

void DestroyQuestDebugTextInputBoxes() { KillTextInputMode(); }

void AddNPCToGridNo(int32_t iGridNo) {
  SOLDIERCREATE_STRUCT MercCreateStruct;
  uint8_t sSectorX, sSectorY;
  uint8_t ubID;

  GetCurrentWorldSector(&sSectorX, &sSectorY);

  memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
  MercCreateStruct.bTeam = CIV_TEAM;
  MercCreateStruct.ubProfile = (uint8_t)gpActiveListBox->sCurSelectedItem;
  MercCreateStruct.sSectorX = sSectorX;
  MercCreateStruct.sSectorY = sSectorY;
  MercCreateStruct.bSectorZ = gbWorldSectorZ;
  MercCreateStruct.sInsertionGridNo = (uint16_t)iGridNo;

  //	RandomizeNewSoldierStats( &MercCreateStruct );

  if (TacticalCreateSoldier(&MercCreateStruct, &ubID)) {
    AddSoldierToSector(ubID);

    // So we can see them!
    AllTeamsLookForAll(NO_INTERRUPTS);
  }

  // Add all the npc in the current sectory the npc array
  AddNPCsInSectorToArray();

  gsQdsEnteringGridNo = (int16_t)iGridNo;
}

void AddItemToGridNo(int32_t iGridNo) {
  struct OBJECTTYPE Object;

  gsQdsEnteringGridNo = (int16_t)iGridNo;

  if (Item[gItemListBox.sCurSelectedItem].usItemClass == IC_KEY) {
    gfAddKeyNextPass = TRUE;
    //		swprintf( zTemp, L"Please enter the Key ID" );
    //		TextEntryBox( zTemp, AddKeyToGridNo );
  } else {
    CreateItem(gItemListBox.sCurSelectedItem,
               (uint8_t)(gfDropDamagedItems ? (20 + Random(60)) : 100), &Object);

    // add the item to the world
    AddItemToPool((uint16_t)iGridNo, &Object, -1, 0, 0, 0);
  }
}

void AddKeyToGridNo(int32_t iKeyID) {
  struct OBJECTTYPE Object;

  if (iKeyID < NUM_KEYS) {
    CreateKeyObject(&Object, 1, (uint8_t)iKeyID);

    // add the item to the world
    AddItemToPool(gsQdsEnteringGridNo, &Object, -1, 0, 0, 0);
  } else
    gfAddKeyNextPass = TRUE;
}

void ChangeDayNumber(int32_t iDayToChangeTo) {
  int32_t uiDiff;
  uint32_t uiNewDayTimeInSec;

  if (iDayToChangeTo) {
    uiNewDayTimeInSec =
        (guiDay + iDayToChangeTo) * NUM_SEC_IN_DAY + 8 * NUM_SEC_IN_HOUR + 15 * NUM_SEC_IN_MIN;
    uiDiff = uiNewDayTimeInSec - guiGameClock;
    WarpGameTime(uiDiff, WARPTIME_PROCESS_EVENTS_NORMALLY);

    ForecastDayEvents();

    // empty dialogue que of all sounds ( guys complain about being tired )
    //
    //	ATE: Please Fix Me!
    //		EmptyDialogueQueue();
  }
}

void CreateDestroyDisplayNPCInventoryPopup(uint8_t ubAction) {
  static BOOLEAN fMouseRegionCreated = FALSE;
  uint16_t usPosY, i;
  struct SOLDIERTYPE *pSoldier;

  switch (ubAction) {
    case QD_DROP_DOWN_NO_ACTION:
      break;

    case QD_DROP_DOWN_CREATE:

      // if the soldier is active
      if (gfUseLocalNPCs)
        pSoldier =
            FindSoldierByProfileID(gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem], FALSE);
      else
        pSoldier = FindSoldierByProfileID((uint8_t)gNpcListBox.sCurSelectedItem, FALSE);

      if (!pSoldier) {
        // qq Display error box

        gubNPCInventoryPopupAction = QD_DROP_DOWN_NO_ACTION;
        break;
      }

      if (fMouseRegionCreated) break;

      fMouseRegionCreated = TRUE;

      // create a mask to block out the screen
      if (!gfBackgroundMaskEnabled) {
        MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, 640, 480,
                          MSYS_PRIORITY_HIGH + 40, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                          QuestDebugTextEntryDisableScreenRegionCallBack);
        MSYS_AddRegion(&gQuestTextEntryDebugDisableScreenRegion);
        gfBackgroundMaskEnabled = TRUE;
      }

      // create the ok button
      guiQuestDebugNPCInventOkBtn = CreateTextButton(
          L"OK", QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK,
          BUTTON_USE_DEFAULT, QUEST_DBS_NPC_INV_POPUP_X + QUEST_DBS_NPC_INV_POPUP_WIDTH / 2 - 12,
          QUEST_DBS_NPC_INV_POPUP_Y + QUEST_DBS_NPC_INV_POPUP_HEIGHT - 30, 30, 25, BUTTON_TOGGLE,
          MSYS_PRIORITY_HIGH + 50, BUTTON_NO_CALLBACK, BtnQuestDebugNPCInventOkBtnButtonCallback);
      SetButtonCursor(guiQuestDebugNPCInventOkBtn, CURSOR_WWW);

      break;

    case QD_DROP_DOWN_DESTROY:
      RemoveButton(guiQuestDebugNPCInventOkBtn);

      if (gfBackgroundMaskEnabled) MSYS_RemoveRegion(&gQuestTextEntryDebugDisableScreenRegion);
      gfBackgroundMaskEnabled = FALSE;

      gfRedrawQuestDebugSystem = TRUE;

      fMouseRegionCreated = FALSE;

      break;

    case QD_DROP_DOWN_DISPLAY: {
      wchar_t zItemName[SIZE_ITEM_NAME];
      //			wchar_t	zItemDesc[ SIZE_ITEM_INFO ];
      uint16_t usFontHeight = GetFontHeight(QUEST_DBS_FONT_LISTBOX_TEXT) + 2;

      // if the soldier is active
      // if the soldier is active
      if (gfUseLocalNPCs)
        pSoldier =
            FindSoldierByProfileID(gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem], FALSE);
      else
        pSoldier = FindSoldierByProfileID((uint8_t)gNpcListBox.sCurSelectedItem, FALSE);

      if (pSoldier) {
        // color the background of the popup
        ColorFillVSurfaceArea(vsFB, QUEST_DBS_NPC_INV_POPUP_X, QUEST_DBS_NPC_INV_POPUP_Y,
                              QUEST_DBS_NPC_INV_POPUP_X + QUEST_DBS_NPC_INV_POPUP_WIDTH,
                              QUEST_DBS_NPC_INV_POPUP_Y + QUEST_DBS_NPC_INV_POPUP_HEIGHT,
                              rgb32_to_rgb16(FROMRGB(45, 59, 74)));

        // Dispaly the NPC inve title
        DrawTextToScreen(QuestDebugText[QUEST_DBS_NPC_INVENTORY], QUEST_DBS_NPC_INV_POPUP_X,
                         QUEST_DBS_NPC_INV_POPUP_Y + 5, QUEST_DBS_NPC_INV_POPUP_WIDTH,
                         QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_TITLE, FONT_MCOLOR_BLACK, FALSE,
                         CENTER_JUSTIFIED);

        // Dispaly the current npc name
        if (gfUseLocalNPCs)
          DrawTextToScreen(
              gMercProfiles[gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem]].zNickname,
              QUEST_DBS_NPC_INV_POPUP_X, QUEST_DBS_NPC_INV_POPUP_Y + 20,
              QUEST_DBS_NPC_INV_POPUP_WIDTH, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_SUBTITLE,
              FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
        else
          DrawTextToScreen(gMercProfiles[gNpcListBox.sCurSelectedItem].zNickname,
                           QUEST_DBS_NPC_INV_POPUP_X, QUEST_DBS_NPC_INV_POPUP_Y + 20,
                           QUEST_DBS_NPC_INV_POPUP_WIDTH, QUEST_DBS_FONT_TITLE,
                           QUEST_DBS_COLOR_SUBTITLE, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

        usPosY = QUEST_DBS_NPC_INV_POPUP_Y + 40;
        for (i = 0; i < NUM_INV_SLOTS; i++) {
          //					if ( !LoadItemInfo( pSoldier->inv[ i ].usItem,
          // zItemName, zItemDesc ) ) 						Assert(0);
          wcscpy(zItemName, ShortItemNames[pSoldier->inv[i].usItem]);

          // Display Name of the pocket
          DrawTextToScreen(PocketText[i], QUEST_DBS_NPC_INV_POPUP_X + 10, usPosY, 0,
                           QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, FONT_MCOLOR_BLACK,
                           FALSE, LEFT_JUSTIFIED);

          // Display the contents of the pocket
          DrawTextToScreen(zItemName, QUEST_DBS_NPC_INV_POPUP_X + 140, usPosY, 0,
                           QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT,
                           FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
          usPosY += usFontHeight;
        }
      }
      InvalidateRegion(0, 0, 640, 480);
      MarkButtonsDirty();
    } break;
  }
}

void BtnQuestDebugNPCInventOkBtnButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    gubNPCInventoryPopupAction = QD_DROP_DOWN_DESTROY;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQuestDebugAllOrSectorNPCToggleCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (gfUseLocalNPCs) {
      gfUseLocalNPCs = FALSE;

      gNpcListBox.sCurSelectedItem = gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem];
      gNpcListBox.usItemDisplayedOnTopOfList = gNpcListBox.sCurSelectedItem;
      //			gNpcListBox.usStartIndex
      //= FIRST_RPC;

      gNpcListBox.usMaxArrayIndex = NUM_PROFILES;
      gNpcListBox.usNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;
      gNpcListBox.usMaxNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;
    } else {
      gfUseLocalNPCs = TRUE;

      gNpcListBox.sCurSelectedItem = -1;
      gNpcListBox.usItemDisplayedOnTopOfList = 0;
      gNpcListBox.usStartIndex = 0;
      gNpcListBox.usMaxArrayIndex = gubNumNPCinSector;

      if (gubNumNPCinSector < QUEST_DBS_MAX_DISPLAYED_ENTRIES) {
        gNpcListBox.usNumDisplayedItems = gubNumNPCinSector;
        gNpcListBox.usMaxNumDisplayedItems = gubNumNPCinSector;
      } else {
        gNpcListBox.usNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;
        gNpcListBox.usMaxNumDisplayedItems = QUEST_DBS_MAX_DISPLAYED_ENTRIES;
      }

      if (gNpcListBox.sCurSelectedItem == -1) {
        DisableButton(guiQuestDebugAddNpcToLocationButton);
        DisableButton(guiQuestDebugViewNPCInvButton);
        DisableButton(guiQuestDebugStartMercTalkingButtonButton);
      }

      if (IsMercInTheSector(gNpcListBox.sCurSelectedItem) == -1)
        DisableButton(guiQuestDebugViewNPCInvButton);

      EnableQDSButtons();
    }

    /*
                    if( gubNumNPCinSector == 0 )
                            SpecifyButtonText( guiQuestDebugCurNPCButton, QuestDebugText[
       QUEST_DBS_NO_NPC_IN_SECTOR ] ); else SpecifyButtonText( guiQuestDebugCurNPCButton,
       QuestDebugText[ QUEST_DBS_SELECTED_NPC ] );
    */
    gfRedrawQuestDebugSystem = TRUE;
  }
}

void AddNPCsInSectorToArray() {
  struct SOLDIERTYPE *pSoldier;
  uint16_t cnt, i;

  // Setup array of merc who are in the current sector
  i = 0;
  for (pSoldier = Menptr, cnt = 0; cnt < TOTAL_SOLDIERS; pSoldier++, cnt++) {
    if ((pSoldier != NULL) && IsSolActive(pSoldier)) {
      // if soldier is a NPC, add him to the local NPC array
      if ((GetSolProfile(pSoldier) >= FIRST_RPC) && (GetSolProfile(pSoldier) < NUM_PROFILES)) {
        gubCurrentNpcInSector[i] = GetSolProfile(pSoldier);
        i++;
      }
    }
  }
  gubNumNPCinSector = (uint8_t)i;
}

void ChangeQuestState(int32_t iNumber) {
  if ((iNumber >= 0) && (iNumber <= 2)) {
    gubQuest[gubCurQuestSelected] = (uint8_t)iNumber;
    gfRedrawQuestDebugSystem = TRUE;
  }
}

void ChangeFactState(int32_t iNumber) {
  if ((iNumber >= 0) && (iNumber <= 1)) {
    gubFact[gusCurFactSelected] = (uint8_t)iNumber;
    gfRedrawQuestDebugSystem = TRUE;
  }
}

void BtnQDPgUpButtonButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    if ((gusFactAtTopOfList - QUEST_DBS_NUM_DISPLAYED_FACTS) >= 0) {
      gusFactAtTopOfList -= QUEST_DBS_NUM_DISPLAYED_FACTS;
    } else
      gusFactAtTopOfList = 0;

    gfRedrawQuestDebugSystem = TRUE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnQDPgDownButtonButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    if ((gusFactAtTopOfList + QUEST_DBS_NUM_DISPLAYED_FACTS) < NUM_FACTS) {
      gusFactAtTopOfList += QUEST_DBS_NUM_DISPLAYED_FACTS;
    } else
      gusFactAtTopOfList = NUM_FACTS - QUEST_DBS_NUM_DISPLAYED_FACTS;

    gfRedrawQuestDebugSystem = TRUE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void NpcRecordLoggingInit(uint8_t ubNpcID, uint8_t ubMercID, uint8_t ubQuoteNum,
                          uint8_t ubApproach) {
  static BOOLEAN fFirstTimeIn = TRUE;

  HWFILE hFile;
  uint32_t uiByteWritten;
  char DestString[1024];
  //	char			MercName[ NICKNAME_LENGTH ];
  //	char			NpcName[ NICKNAME_LENGTH ];

  DestString[0] = '\0';

  // if the npc log button is turned off, ignore
  if (!gfNpcLogButton) return;

  // if the approach is NPC_INITIATING_CONV, return
  if (ubApproach == NPC_INITIATING_CONV) return;

  // if its the first time in the game
  if (fFirstTimeIn) {
    // open a new file for writing

    // if the file exists
    if (FileMan_Exists(QUEST_DEBUG_FILE)) {
      // delete the file
      if (!FileMan_Delete(QUEST_DEBUG_FILE)) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("FAILED to delete %s file", QUEST_DEBUG_FILE));
        return;
      }
    }
    fFirstTimeIn = FALSE;
  }

  // open the file
  hFile = FileMan_Open(QUEST_DEBUG_FILE, FILE_OPEN_ALWAYS | FILE_ACCESS_WRITE, FALSE);
  if (!hFile) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("FAILED to open Quest Debug File %s", QUEST_DEBUG_FILE));
    return;
  }

  if (FileMan_Seek(hFile, 0, FILE_SEEK_FROM_END) == FALSE) {
    // error
    FileMan_Close(hFile);
    return;
  }

  sprintf(DestString, "\n\n\nNew Approach for NPC ID: %d '%S' against Merc: %d '%S'", ubNpcID,
          gMercProfiles[ubNpcID].zNickname, ubMercID, gMercProfiles[ubMercID].zNickname);
  //	sprintf( DestString, "\n\n\nNew Approach for NPC ID: %d  against Merc: %d ", ubNpcID,
  // ubMercID );

  if (!FileMan_Write(hFile, DestString, strlen(DestString), &uiByteWritten)) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("FAILED to write to %s", QUEST_DEBUG_FILE));
    return;
  }

  // Testing Record #
  sprintf(DestString, "\n\tTesting Record #: %d", ubQuoteNum);

  // append to file
  if (!FileMan_Write(hFile, DestString, strlen(DestString), &uiByteWritten)) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("FAILED to write to %s", QUEST_DEBUG_FILE));
    return;
  }

  FileMan_Close(hFile);
}

void NpcRecordLogging(uint8_t ubApproach, char *pStringA, ...) {
  HWFILE hFile;
  uint32_t uiByteWritten;
  va_list argptr;
  char TempString[1000];
  char DestString[1024];

  TempString[0] = '\0';
  DestString[0] = '\0';

  // if the npc log button is turned off, ignore
  if (!gfNpcLogButton) return;

  // if the approach is NPC_INITIATING_CONV, return
  if (ubApproach == NPC_INITIATING_CONV) return;

  va_start(argptr, pStringA);              // Set up variable argument pointer
  vsprintf(TempString, pStringA, argptr);  // process gprintf string (get output str)
  va_end(argptr);

  // open the file
  hFile = FileMan_Open(QUEST_DEBUG_FILE, FILE_OPEN_ALWAYS | FILE_ACCESS_WRITE, FALSE);
  if (!hFile) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("FAILED to open Quest Debug File %s", QUEST_DEBUG_FILE));
    return;
  }

  if (FileMan_Seek(hFile, 0, FILE_SEEK_FROM_END) == FALSE) {
    // error
    FileMan_Close(hFile);
    return;
  }

  snprintf(DestString, ARR_SIZE(DestString), "\n\t\t%s", TempString);

  // append to file
  if (!FileMan_Write(hFile, DestString, strlen(DestString), &uiByteWritten)) {
    FileMan_Close(hFile);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("FAILED to write to %s", QUEST_DEBUG_FILE));
    return;
  }

  FileMan_Close(hFile);
}

void EnableQDSButtons() {
  if (gNpcListBox.sCurSelectedItem != -1) {
    EnableButton(guiQuestDebugAddNpcToLocationButton);
    EnableButton(guiQuestDebugStartMercTalkingButtonButton);
    EnableButton(guiQuestDebugNPCLogButtonButton);
    EnableButton(guiQuestDebugNPCRefreshButtonButton);
  } else {
    DisableButton(guiQuestDebugStartMercTalkingButtonButton);
    DisableButton(guiQuestDebugAddNpcToLocationButton);
    DisableButton(guiQuestDebugNPCLogButtonButton);
    DisableButton(guiQuestDebugNPCRefreshButtonButton);
  }

  if (gItemListBox.sCurSelectedItem != -1)
    EnableButton(guiQuestDebugAddItemToLocationButton);
  else
    DisableButton(guiQuestDebugAddItemToLocationButton);

  if (gItemListBox.sCurSelectedItem != -1 && gNpcListBox.sCurSelectedItem != 0) {
    EnableButton(guiQuestDebugGiveItemToNPCButton);
  } else {
    DisableButton(guiQuestDebugGiveItemToNPCButton);
  }

  if (gfUseLocalNPCs) {
    if (IsMercInTheSector(gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem]) != -1) {
      EnableButton(guiQuestDebugViewNPCInvButton);
      EnableButton(guiQuestDebugNPCRefreshButtonButton);
      EnableButton(guiQuestDebugAddNpcToLocationButton);
    } else {
      DisableButton(guiQuestDebugAddNpcToLocationButton);
      DisableButton(guiQuestDebugViewNPCInvButton);
      DisableButton(guiQuestDebugNPCRefreshButtonButton);
    }
  }
  /*
          else
          {
                  if( IsMercInTheSector( gNpcListBox.sCurSelectedItem ) != -1 )
                  {
                          EnableButton( guiQuestDebugAddNpcToLocationButton );
                          EnableButton( guiQuestDebugViewNPCInvButton );
                          EnableButton( guiQuestDebugNPCRefreshButtonButton );
                  }
                  else
                  {
  //			DisableButton( guiQuestDebugAddNpcToLocationButton );
  //			DisableButton( guiQuestDebugViewNPCInvButton );
  //			DisableButton( guiQuestDebugNPCRefreshButtonButton );
                  }
          }
  */
}

BOOLEAN DoQDSMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen, uint8_t ubFlags,
                        MSGBOX_CALLBACK ReturnCallback) {
  SGPRect pCenteringRect = {0, 0, 639, 479};

  // reset exit mode
  gfExitQdsDueToMessageBox = TRUE;
  gfQuestDebugEntry = TRUE;

  // do message box and return
  giQdsMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                 (uint8_t)(ubFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                 ReturnCallback, &pCenteringRect);

  // send back return state
  return ((giQdsMessageBox != -1));
}

void IncrementActiveDropDownBox(int16_t sIncrementValue) {
  if (sIncrementValue < 0) sIncrementValue = 0;

  // if the mouse was clicked above the scroll box
  if (sIncrementValue < gpActiveListBox->sCurSelectedItem) {
    if ((sIncrementValue) <= gpActiveListBox->usStartIndex) {
      gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usStartIndex;
      sIncrementValue = gpActiveListBox->usStartIndex;
    } else if (sIncrementValue < gpActiveListBox->usItemDisplayedOnTopOfList &&
               gpActiveListBox->usItemDisplayedOnTopOfList > gpActiveListBox->usStartIndex) {
      gpActiveListBox->usItemDisplayedOnTopOfList = sIncrementValue;
    }
  }
  // else the mouse was clicked below the scroll box
  else {
    if (sIncrementValue >=
        (gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems)) {
      if (gpActiveListBox->usItemDisplayedOnTopOfList >=
          gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems)
        gpActiveListBox->usItemDisplayedOnTopOfList =
            gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems;
      else if ((sIncrementValue - gpActiveListBox->usItemDisplayedOnTopOfList) >=
               gpActiveListBox->usMaxNumDisplayedItems) {
        gpActiveListBox->usItemDisplayedOnTopOfList =
            sIncrementValue - gpActiveListBox->usMaxNumDisplayedItems + 1;
      }

      if (sIncrementValue >= gpActiveListBox->usMaxArrayIndex)
        sIncrementValue = gpActiveListBox->usMaxArrayIndex - 1;
    } else if (sIncrementValue >= gpActiveListBox->usMaxArrayIndex) {
      sIncrementValue = gpActiveListBox->usMaxArrayIndex - 1;
      gpActiveListBox->usItemDisplayedOnTopOfList =
          gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems;
    } else if (sIncrementValue >= gpActiveListBox->usItemDisplayedOnTopOfList +
                                      gpActiveListBox->usMaxNumDisplayedItems) {
      gpActiveListBox->usItemDisplayedOnTopOfList +=
          sIncrementValue - (gpActiveListBox->usItemDisplayedOnTopOfList +
                             gpActiveListBox->usMaxNumDisplayedItems - 1);
    }
  }

  gpActiveListBox->sCurSelectedItem = sIncrementValue;

  gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DISPLAY;
}

int16_t IsMercInTheSector(uint16_t usMercID) {
  uint8_t cnt;

  if (usMercID == -1) return (FALSE);

  for (cnt = 0; cnt <= TOTAL_SOLDIERS; cnt++) {
    // if the merc is active
    if (Menptr[cnt].ubProfile == usMercID) {
      if (Menptr[cnt].bActive) return (Menptr[cnt].ubID);
    }
  }

  return (-1);
}

void RefreshAllNPCInventory() {
  uint16_t usCnt;
  uint16_t usItemCnt;
  struct OBJECTTYPE TempObject;
  uint16_t usItem;

  for (usCnt = 0; usCnt < TOTAL_SOLDIERS; usCnt++) {
    // if the is active
    if (Menptr[usCnt].bActive == 1) {
      // is the merc a rpc or npc
      if (Menptr[usCnt].ubProfile >= FIRST_RPC) {
        // refresh the mercs inventory
        for (usItemCnt = 0; usItemCnt < NUM_INV_SLOTS; usItemCnt++) {
          // null out the items in the npc inventory
          memset(&GetSoldierByID(usCnt)->inv[usItemCnt], 0, sizeof(struct OBJECTTYPE));

          if (gMercProfiles[Menptr[usCnt].ubProfile].inv[usItemCnt] != NOTHING) {
            // get the item
            usItem = gMercProfiles[Menptr[usCnt].ubProfile].inv[usItemCnt];

            // Create the object
            CreateItem(usItem, 100, &TempObject);

            // copy the item into the soldiers inventory
            memcpy(&GetSoldierByID(usCnt)->inv[usItemCnt], &TempObject, sizeof(struct OBJECTTYPE));
          }
        }
      }
    }
  }
}

void StartMercTalkingFromQuoteNum(int32_t iQuoteToStartTalkingFrom) {
  wchar_t zTemp[512];
  int32_t uiMaxNumberOfQuotes = GetMaxNumberOfQuotesToPlay();

  // make sure the current character is created
  SetQDSMercProfile();

  SetTalkingMercPauseState(FALSE);

  // do some error checks
  if (iQuoteToStartTalkingFrom < 0 || iQuoteToStartTalkingFrom > uiMaxNumberOfQuotes) {
    swprintf(zTemp, ARR_SIZE(zTemp), L"Please enter a value between 0 and %d", uiMaxNumberOfQuotes);
    DoQDSMessageBox(MSG_BOX_BASIC_STYLE, zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);
  } else {
    // Start the merc talking from the selected quote number
    giSelectedMercCurrentQuote = iQuoteToStartTalkingFrom;
  }

  // create a mask to block out the screen
  if (!gfBackgroundMaskEnabled) {
    MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, 640, 480,
                      MSYS_PRIORITY_HIGH + 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
                      QuestDebugTextEntryDisableScreenRegionCallBack);
    MSYS_AddRegion(&gQuestTextEntryDebugDisableScreenRegion);
    gfBackgroundMaskEnabled = TRUE;
  }

  DisableFactMouseRegions();
}

void EndMercTalking() {
  // remove the talking dialogue
  if (gfNpcPanelIsUsedForTalkingMerc) DeleteTalkingMenu();
  gfNpcPanelIsUsedForTalkingMerc = FALSE;

  // remove the mask of the entire screen
  if (gfBackgroundMaskEnabled) {
    MSYS_RemoveRegion(&gQuestTextEntryDebugDisableScreenRegion);
    gfBackgroundMaskEnabled = FALSE;
  }

  giSelectedMercCurrentQuote = -1;

  // make sure we can dirty the button
  if (!gfQuestDebugExit) ButtonList[guiQDPgUpButtonButton]->uiFlags &= ~BUTTON_FORCE_UNDIRTY;

  // enable the fact mouse regions
  EnableFactMouseRegions();
}

void HandleQDSTalkingMerc() {
  //	static BOOLEAN	fWas
  BOOLEAN fIsTheMercTalking = FALSE;
  uint8_t ubPanelMercShouldUse;

  if (giSelectedMercCurrentQuote != -1) {
    if (gTalkingMercSoldier == NULL) return;

    // Call this function to enable or disable the flags in the faces struct ( without modifing the
    // pause state )
    SetTalkingMercPauseState(gfPauseTalkingMercPopup);

    ubPanelMercShouldUse = WhichPanelShouldTalkingMercUse();

    // find out if the merc is talking
    if (ubPanelMercShouldUse == QDS_REGULAR_PANEL)
      fIsTheMercTalking = gFacesData[gTalkingMercSoldier->iFaceIndex].fTalking;
    else
      fIsTheMercTalking = gFacesData[gTalkPanel.iFaceIndex].fTalking;

    // if the merc is not talking
    if (!fIsTheMercTalking) {
      // if we still have more quotes to say
      if (giSelectedMercCurrentQuote < GetMaxNumberOfQuotesToPlay()) {
        // if the user has paused the playing
        if (gfPauseTalkingMercPopup) {
          // get out
          return;
        }

        // Start the merc talking
        if (ubPanelMercShouldUse == QDS_REGULAR_PANEL)
          TacticalCharacterDialogue(gTalkingMercSoldier, (uint16_t)giSelectedMercCurrentQuote);
        else if (gfRpcToSaySectorDesc && gTalkingMercSoldier->ubProfile >= 57 &&
                 gTalkingMercSoldier->ubProfile <= 60) {
          // ATE: Trigger the sector desc here
          CharacterDialogueWithSpecialEvent(
              gTalkingMercSoldier->ubProfile, (uint16_t)giSelectedMercCurrentQuote,
              gTalkPanel.iFaceIndex, DIALOGUE_NPC_UI, TRUE, FALSE,
              DIALOGUE_SPECIAL_EVENT_USE_ALTERNATE_FILES, FALSE, FALSE);
        } else
          CharacterDialogue(gTalkingMercSoldier->ubProfile, (uint16_t)giSelectedMercCurrentQuote,
                            gTalkPanel.iFaceIndex, DIALOGUE_NPC_UI, FALSE, FALSE);

        // Incremenet the current quote number
        giSelectedMercCurrentQuote++;
      } else {
        // Stop the merc from talking
        giSelectedMercCurrentQuote = -1;

        EndMercTalking();
      }
    }

    // Redraw the screen
    gfRedrawQuestDebugSystem = TRUE;
  } else {
    /*
                    //as soon as the panel is no longer active, refresh the screen
                    if( gfFacePanelActive == FALSE )
                    {
                            //Redraw the screen
                            gfRedrawQuestDebugSystem = TRUE;
                    }
    */
  }
}

void SetTalkingMercPauseState(BOOLEAN fState) {
  if (fState) {
    gfPauseTalkingMercPopup = TRUE;

    if (gTalkingMercSoldier)
      gFacesData[gTalkingMercSoldier->iFaceIndex].uiFlags |= FACE_POTENTIAL_KEYWAIT;
  } else {
    gfPauseTalkingMercPopup = FALSE;

    if (gTalkingMercSoldier)
      gFacesData[gTalkingMercSoldier->iFaceIndex].uiFlags &= ~FACE_POTENTIAL_KEYWAIT;
  }
}

void SetQDSMercProfile() {
  // Get selected soldier
  if (GetSoldier(&gTalkingMercSoldier, gusSelectedSoldier)) {
    // Change guy!
    ForceSoldierProfileID(gTalkingMercSoldier, (uint8_t)gNpcListBox.sCurSelectedItem);

    // if it is an rpc
    if (gTalkingMercSoldier->ubProfile >= 57 && gTalkingMercSoldier->ubProfile <= 72) {
      if (gfAddNpcToTeam)
        gMercProfiles[gTalkingMercSoldier->ubProfile].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;
      else
        gMercProfiles[gTalkingMercSoldier->ubProfile].ubMiscFlags &= ~PROFILE_MISC_FLAG_RECRUITED;
    } else {
    }

    if (WhichPanelShouldTalkingMercUse() == QDS_NPC_PANEL) {
      // remove the talking dialogue
      if (gfNpcPanelIsUsedForTalkingMerc) DeleteTalkingMenu();

      gfNpcPanelIsUsedForTalkingMerc = TRUE;

      InternalInitTalkingMenu(gTalkingMercSoldier->ubProfile, 10, 10);
      gpDestSoldier = GetSoldierByID(21);
    }
  }
}

void DisplayQDSCurrentlyQuoteNum() {
  wchar_t zTemp[512];
  uint16_t usPosY;
  uint16_t usFontHeight = GetFontHeight(QUEST_DBS_FONT_TEXT_ENTRY) + 2;

  // Display the box frame
  ColorFillVSurfaceArea(vsFB, QDS_CURRENT_QUOTE_NUM_BOX_X, QDS_CURRENT_QUOTE_NUM_BOX_Y,
                        QDS_CURRENT_QUOTE_NUM_BOX_X + QDS_CURRENT_QUOTE_NUM_BOX_WIDTH,
                        QDS_CURRENT_QUOTE_NUM_BOX_Y + QDS_CURRENT_QUOTE_NUM_BOX_HEIGHT,
                        rgb32_to_rgb16(FROMRGB(32, 41, 53)));

  swprintf(zTemp, ARR_SIZE(zTemp), L"'%s' is currently saying quote #%d",
           gMercProfiles[gTalkingMercSoldier->ubProfile].zNickname, giSelectedMercCurrentQuote - 1);

  // Display the text box caption
  usPosY = QDS_CURRENT_QUOTE_NUM_BOX_Y + 4;
  DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY,
                       QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY,
                       FONT_MCOLOR_WHITE, zTemp, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display the Pause speech text
  usPosY += usFontHeight + 4;
  DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY,
                       QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY,
                       FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_PAUSE_SPEECH], FONT_MCOLOR_BLACK,
                       FALSE, LEFT_JUSTIFIED);

  // Display the left arrow quote
  usPosY += usFontHeight;
  DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY,
                       QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY,
                       FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_LEFT_ARROW_PREVIOUS_QUOTE],
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the right arrow quote
  usPosY += usFontHeight;
  DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY,
                       QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY,
                       FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_RIGHT_ARROW_NEXT_QUOTE],
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Display the right arrow quote
  usPosY += usFontHeight;
  DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY,
                       QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY,
                       FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_ESC_TOP_STOP_TALKING],
                       FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  InvalidateRegion(QDS_CURRENT_QUOTE_NUM_BOX_X, QDS_CURRENT_QUOTE_NUM_BOX_Y,
                   QDS_CURRENT_QUOTE_NUM_BOX_X + QDS_CURRENT_QUOTE_NUM_BOX_WIDTH,
                   QDS_CURRENT_QUOTE_NUM_BOX_Y + QDS_CURRENT_QUOTE_NUM_BOX_HEIGHT);
}

void BtnQuestDebugAddNpcToTeamToggleCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (gfAddNpcToTeam)
      gfAddNpcToTeam = FALSE;
    else
      gfAddNpcToTeam = TRUE;
  }
}

void BtnQuestDebugRPCSaySectorDescToggleCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (gfRpcToSaySectorDesc)
      gfRpcToSaySectorDesc = FALSE;
    else
      gfRpcToSaySectorDesc = TRUE;
  }
}

uint8_t WhichPanelShouldTalkingMercUse() {
  if (gTalkingMercSoldier == NULL) {
    return (QDS_NO_PANEL);
  }

  if (gTalkingMercSoldier->ubProfile < FIRST_RPC) {
    return (QDS_REGULAR_PANEL);
  } else {
    return (QDS_NPC_PANEL);
  }
}

void DisableFactMouseRegions() {
  unsigned int i;

  for (i = 0; i < QUEST_DBS_NUM_DISPLAYED_FACTS; i++) {
    MSYS_DisableRegion(&gFactListRegion[i]);
  }
}

void EnableFactMouseRegions() {
  unsigned int i;

  for (i = 0; i < QUEST_DBS_NUM_DISPLAYED_FACTS; i++) {
    MSYS_EnableRegion(&gFactListRegion[i]);
  }
}

int32_t GetMaxNumberOfQuotesToPlay() {
  int32_t iNumberOfQuotes = 0;
  uint8_t ubProfileID = (uint8_t)gNpcListBox.sCurSelectedItem;

  // if it is the RPCs and they are to say the sector descs
  if (gfRpcToSaySectorDesc && ubProfileID >= 57 && ubProfileID <= 60) {
    iNumberOfQuotes = 34;
  }

  // else if it is a RPC who is on our team
  else if (gMercProfiles[gNpcListBox.sCurSelectedItem].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED &&
           ubProfileID >= 57 && ubProfileID <= 72) {
    iNumberOfQuotes = 119;
  }

  // else if it is the queen
  else if (ubProfileID == QUEEN) {
    iNumberOfQuotes = 138;
  }

  // else if it is speck
  else if (ubProfileID == 159) {
    iNumberOfQuotes = 72;
  } else
    iNumberOfQuotes = 138;

  return (iNumberOfQuotes + 1);
}

void GetDebugLocationString(uint16_t usProfileID, wchar_t *pzText, size_t bufSize) {
  struct SOLDIERTYPE *pSoldier;

  // Get a soldier pointer
  pSoldier = FindSoldierByProfileID((uint8_t)usProfileID, FALSE);

  // if their is a soldier, the soldier is alive and the soldier is off the map
  if (pSoldier != NULL && IsSolActive(pSoldier) && pSoldier->uiStatusFlags & SOLDIER_OFF_MAP) {
    // the soldier is on schedule
    swprintf(pzText, bufSize, L"On Schdl.");
  }

  // if the soldier is dead
  else if (gMercProfiles[usProfileID].bMercStatus == MERC_IS_DEAD) {
    swprintf(pzText, bufSize, L"Dead");
  }

  // the soldier is in this sector
  else if (pSoldier != NULL) {
    GetShortSectorString(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), pzText, bufSize);
  }

  // else the soldier is in a different map
  else {
    GetShortSectorString((uint8_t)gMercProfiles[usProfileID].sSectorX,
                         (uint8_t)gMercProfiles[usProfileID].sSectorY, pzText, bufSize);
  }
}
