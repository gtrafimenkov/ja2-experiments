// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _HANDLE_UI_
#define _HANDLE_UI_

#include "SGP/Input.h"

struct SOLDIERTYPE;
struct TAG_UI_EVENT;

// DEFINES
#define UIEVENT_SINGLEEVENT 0x00000002
#define UIEVENT_SNAPMOUSE 0x00000008

#define NO_GUY_SELECTION 0
#define SELECTED_GUY_SELECTION 1
#define NONSELECTED_GUY_SELECTION 2
#define ENEMY_GUY_SELECTION 3

#define MOUSE_MOVING_IN_TILE 0x00000001
#define MOUSE_MOVING 0x00000002
#define MOUSE_MOVING_NEW_TILE 0x00000004
#define MOUSE_STATIONARY 0x00000008

#define MOVEUI_TARGET_INTTILES 1
#define MOVEUI_TARGET_ITEMS 2
#define MOVEUI_TARGET_MERCS 3
#define MOVEUI_TARGET_MERCSFORAID 5
#define MOVEUI_TARGET_WIREFENCE 6
#define MOVEUI_TARGET_BOMB 7
#define MOVEUI_TARGET_STEAL 8
#define MOVEUI_TARGET_REPAIR 9
#define MOVEUI_TARGET_JAR 10
#define MOVEUI_TARGET_CAN 11
#define MOVEUI_TARGET_REFUEL 12

#define MOVEUI_RETURN_ON_TARGET_MERC 1

typedef enum {
  DONT_CHANGEMODE,
  IDLE_MODE,
  MOVE_MODE,
  ACTION_MODE,
  MENU_MODE,
  POPUP_MODE,
  CONFIRM_MOVE_MODE,
  ADJUST_STANCE_MODE,
  CONFIRM_ACTION_MODE,
  HANDCURSOR_MODE,
  GETTINGITEM_MODE,
  ENEMYS_TURN_MODE,
  LOOKCURSOR_MODE,
  TALKINGMENU_MODE,
  TALKCURSOR_MODE,
  LOCKUI_MODE,
  OPENDOOR_MENU_MODE,
  LOCKOURTURN_UI_MODE,
  EXITSECTORMENU_MODE,
  RUBBERBAND_MODE,
  JUMPOVER_MODE,

} UI_MODE;

typedef uint32_t (*UI_HANDLEFNC)(struct TAG_UI_EVENT *);

typedef struct TAG_UI_EVENT {
  uint32_t uiFlags;
  UI_MODE ChangeToUIMode;
  UI_HANDLEFNC HandleEvent;
  BOOLEAN fFirstTime;
  BOOLEAN fDoneMenu;
  UI_MODE uiMenuPreviousMode;
  uint32_t uiParams[3];

} UI_EVENT;

// EVENT ENUMERATION
typedef enum {
  I_DO_NOTHING,
  I_EXIT,
  I_NEW_MERC,
  I_NEW_BADMERC,
  I_SELECT_MERC,
  I_ENTER_EDIT_MODE,
  I_ENTER_PALEDIT_MODE,
  I_ENDTURN,
  I_TESTHIT,
  I_CHANGELEVEL,
  I_ON_TERRAIN,
  I_CHANGE_TO_IDLE,
  I_LOADLEVEL,
  I_SOLDIERDEBUG,
  I_LOSDEBUG,
  I_LEVELNODEDEBUG,
  I_GOTODEMOMODE,
  I_LOADFIRSTLEVEL,
  I_LOADSECONDLEVEL,
  I_LOADTHIRDLEVEL,
  I_LOADFOURTHLEVEL,
  I_LOADFIFTHLEVEL,

  ET_ON_TERRAIN,
  ET_ENDENEMYS_TURN,

  M_ON_TERRAIN,
  M_CHANGE_TO_ACTION,
  M_CHANGE_TO_HANDMODE,
  M_CYCLE_MOVEMENT,
  M_CYCLE_MOVE_ALL,
  M_CHANGE_TO_ADJPOS_MODE,

  POPUP_DOMESSAGE,

  A_ON_TERRAIN,
  A_CHANGE_TO_MOVE,
  A_CHANGE_TO_CONFIM_ACTION,
  A_END_ACTION,
  U_MOVEMENT_MENU,
  U_POSITION_MENU,

  C_WAIT_FOR_CONFIRM,
  C_MOVE_MERC,
  C_ON_TERRAIN,

  PADJ_ADJUST_STANCE,

  CA_ON_TERRAIN,
  CA_MERC_SHOOT,
  CA_END_CONFIRM_ACTION,

  HC_ON_TERRAIN,

  G_GETTINGITEM,

  LC_ON_TERRAIN,
  LC_CHANGE_TO_LOOK,
  LC_LOOK,

  TA_TALKINGMENU,

  T_ON_TERRAIN,
  T_CHANGE_TO_TALKING,

  LU_ON_TERRAIN,
  LU_BEGINUILOCK,
  LU_ENDUILOCK,

  OP_OPENDOORMENU,

  LA_ON_TERRAIN,
  LA_BEGINUIOURTURNLOCK,
  LA_ENDUIOUTURNLOCK,

  EX_EXITSECTORMENU,

  RB_ON_TERRAIN,

  JP_ON_TERRAIN,
  JP_JUMP,

  NUM_UI_EVENTS

} UI_EVENT_DEFINES;

typedef BOOLEAN (*UIKEYBOARD_HOOK)(InputAtom *pInputEvent);

// GLOBAL EVENT STRUCT
extern UI_EVENT gEvents[NUM_UI_EVENTS];

// GLOBAL STATUS VARS
extern UI_MODE gCurrentUIMode;
extern UI_MODE gOldUIMode;
extern uint32_t guiCurrentEvent;
extern int16_t gsSelectedLevel;
extern BOOLEAN gfPlotNewMovement;
extern uint32_t guiPendingOverrideEvent;

// GLOBALS
extern BOOLEAN gfUIDisplayActionPoints;
extern BOOLEAN gfUIDisplayActionPointsInvalid;
extern BOOLEAN gfUIDisplayActionPointsBlack;
extern BOOLEAN gfUIDisplayActionPointsCenter;
extern int16_t gUIDisplayActionPointsOffY;
extern int16_t gUIDisplayActionPointsOffX;
extern BOOLEAN gfUIDoNotHighlightSelMerc;
extern uint32_t guiShowUPDownArrows;
extern BOOLEAN gfUIHandleSelection;
extern BOOLEAN gfUIHandleSelectionAboveGuy;
extern int16_t gsSelectedGridNo;
extern int16_t gsSelectedGuy;
extern BOOLEAN gfUIInDeadlock;
extern uint8_t gUIDeadlockedSoldier;

extern BOOLEAN gfUIMouseOnValidCatcher;
extern uint8_t gubUIValidCatcherID;
extern BOOLEAN gUIUseReverse;

extern BOOLEAN gfUIHandleShowMoveGrid;
extern uint16_t gsUIHandleShowMoveGridLocation;

extern BOOLEAN gfUIDisplayDamage;
extern int8_t gbDamage;
extern uint16_t gsDamageGridNo;

extern BOOLEAN gfFontPopupDo;

extern BOOLEAN gUITargetReady;
extern BOOLEAN gUITargetShotWaiting;
extern uint16_t gsUITargetShotGridNo;

extern wchar_t gzLocation[20];
extern BOOLEAN gfUIBodyHitLocation;

extern wchar_t gzIntTileLocation[20];
extern BOOLEAN gfUIIntTileLocation;

extern wchar_t gzIntTileLocation2[20];
extern BOOLEAN gfUIIntTileLocation2;

extern BOOLEAN gfUIWaitingForUserSpeechAdvance;
extern BOOLEAN gfUIKeyCheatModeOn;

extern BOOLEAN gfUIAllMoveOn;
extern BOOLEAN gfUICanBeginAllMoveCycle;

extern BOOLEAN gfUIRefreshArrows;

extern BOOLEAN gfUIHandlePhysicsTrajectory;

// GLOBALS FOR FAST LOOKUP FOR FINDING MERCS FROM THE MOUSE
extern BOOLEAN gfUISelectiveTargetFound;
extern uint16_t gusUISelectiveTargetID;
extern uint32_t guiUISelectiveTargetFlags;

extern BOOLEAN gfUIFullTargetFound;
extern uint16_t gusUIFullTargetID;
extern uint32_t guiUIFullTargetFlags;

extern BOOLEAN gfUIConfirmExitArrows;
extern int16_t gsJumpOverGridNo;

uint32_t HandleTacticalUI(void);
uint32_t UIHandleEndTurn(UI_EVENT *pUIEvent);

extern BOOLEAN gfUIShowCurIntTile;

extern SGPRect gRubberBandRect;
extern BOOLEAN gRubberBandActive;

void EndMenuEvent(uint32_t uiEvent);
void SetUIKeyboardHook(UIKEYBOARD_HOOK KeyboardHookFnc);
void HandleObjectHighlighting();

extern BOOLEAN gfUIForceReExamineCursorData;

extern int16_t guiCreateGuyIndex;
extern int16_t guiCreateBadGuyIndex;

// FUNCTIONS IN INPUT MODULES
void GetKeyboardInput(uint32_t *puiNewEvent);
void GetPolledKeyboardInput(uint32_t *puiNewEvent);

void GetTBMouseButtonInput(uint32_t *puiNewEvent);
void GetTBMousePositionInput(uint32_t *puiNewEvent);
void QueryTBLeftButton(uint32_t *puiNewEvent);
void QueryTBRightButton(uint32_t *puiNewEvent);
void HandleStanceChangeFromUIKeys(uint8_t ubAnimHeight);
void HandleKeyInputOnEnemyTurn();

void GetRTMouseButtonInput(uint32_t *puiNewEvent);
void GetRTMousePositionInput(uint32_t *puiNewEvent);
void QueryRTLeftButton(uint32_t *puiNewEvent);
void QueryRTRightButton(uint32_t *puiNewEvent);

void AdjustSoldierCreationStartValues();

BOOLEAN SelectedMercCanAffordAttack();
BOOLEAN SelectedMercCanAffordMove();
void GetMercClimbDirection(uint8_t ubSoldierID, BOOLEAN *pfGoDown, BOOLEAN *pfGoUp);

void ToggleHandCursorMode(uint32_t *puiNewEvent);
void ToggleTalkCursorMode(uint32_t *puiNewEvent);
void ToggleLookCursorMode(uint32_t *puiNewEvent);

void UIHandleSoldierStanceChange(uint8_t ubSoldierID, int8_t bNewStance);
void GetCursorMovementFlags(uint32_t *puiCursorFlags);

BOOLEAN HandleUIMovementCursor(struct SOLDIERTYPE *pSoldier, uint32_t uiCursorFlags,
                               uint16_t usMapPos, uint32_t uiFlags);
BOOLEAN UIMouseOnValidAttackLocation(struct SOLDIERTYPE *pSoldier);

BOOLEAN UIOkForItemPickup(struct SOLDIERTYPE *pSoldier, int16_t sGridNo);

BOOLEAN IsValidTalkableNPCFromMouse(uint8_t *pubSoldierID, BOOLEAN fGive, BOOLEAN fAllowMercs,
                                    BOOLEAN fCheckCollapsed);
BOOLEAN IsValidTalkableNPC(uint8_t ubSoldierID, BOOLEAN fGive, BOOLEAN fAllowMercs,
                           BOOLEAN fCheckCollapsed);

BOOLEAN HandleTalkInit();

BOOLEAN HandleCheckForExitArrowsInput(BOOLEAN fAdjustForConfirm);

void SetUIBusy(uint8_t ubID);
void UnSetUIBusy(uint8_t ubID);

uint32_t UIHandleLUIEndLock(UI_EVENT *pUIEvent);

void BeginDisplayTimedCursor(uint32_t uiCursorID, uint32_t uiDelay);

void HandleHandCursorClick(uint16_t usMapPos, uint32_t *puiNewEvent);
int8_t HandleMoveModeInteractiveClick(uint16_t usMapPos, uint32_t *puiNewEvent);

BOOLEAN HandleUIReloading(struct SOLDIERTYPE *pSoldier);

uint32_t UIHandleChangeLevel(UI_EVENT *pUIEvent);
BOOLEAN UIHandleOnMerc(BOOLEAN fMovementMode);

void ChangeInterfaceLevel(int16_t sLevel);

void EndRubberBanding();
void ResetMultiSelection();
void EndMultiSoldierSelection(BOOLEAN fAcknowledge);
void StopRubberBandedMercFromMoving();

BOOLEAN SelectedGuyInBusyAnimation();

void GotoLowerStance(struct SOLDIERTYPE *pSoldier);
void GotoHeigherStance(struct SOLDIERTYPE *pSoldier);

BOOLEAN IsValidJumpLocation(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, BOOLEAN fCheckForPath);

void PopupAssignmentMenuInTactical(struct SOLDIERTYPE *pSoldier);

#endif
