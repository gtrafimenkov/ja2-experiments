// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/HandleUI.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BuildDefines.h"
#include "GameSettings.h"
#include "Globals.h"
#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "OptionsScreen.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/Input.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "ScreenIDs.h"
#include "Screens.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/CivQuotes.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/HandleItems.h"
#include "Tactical/HandleUIPlan.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/LOS.h"
#include "Tactical/MapInformation.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/QArray.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/SoldierFunctions.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SpreadBurst.h"
#include "Tactical/Squads.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/UICursors.h"
#include "Tactical/Vehicles.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

#define MAX_ON_DUTY_SOLDIERS 6

/////////////////////////////////////////////////////////////////////////////////////
//											UI SYSTEM
// DESCRIPTION
//
//	The UI system here decouples event determination from event execution. IN other words,
//	first any user input is gathered and analysed for an event to happen. Once the event is
// determined, 	it is then executed. For example, if the left mouse button is used to select a guy,
// it does not
//  execute the code to selected the guy, rather it sets a flag to a particular event, in this case
//	the I_SELECT_MERC event is set. The code then executes this event after all input is
// analysed. In
//  this way, more than one input method from the user will cause the came event to occur and hence
//  no duplication of code. Also, events have cetain charactoristics. The select merc event is
//  executed just once and then returns to the previous event. Most events are set to run
//  continuously until new input changes to another event. Other events have a 'SNAP-BACK' feature
//  which snap the mouse back to it's position before the event was executed.  Another issue is UI
//  modes. In order to filter out input depending on other flags, for example we do not want to
//  cancel the confirm when a user moves to another
//	tile unless we are in the 'confirm' mode.  This could be done by flags ( and in effect it is
//) where
//  if staements are used, but here at input collection time, we can switch on our current mode to
//  handle input differently based on the mode. Doing it this way also allows us to group certain
//  commands togther like menu commands which are initialized and deleted in the same manner.
//
//	UI_EVENTS
/////////////
//
//	UI_EVENTS have flags to itendtify themselves with special charactoristics, a UI_MODE
// catagory which
//  signifies the UI mode this event will cause the system to move to. Also, a pointer to a handle
//  function is used to actually handle the particular event. UI_EVENTS also have a couple of param
//  variables and a number of boolean flags used during run-time to determine states of events.
//
////////////////////////////////////////////////////////////////////////////////////////////////

// LOCAL DEFINES
#define GO_MOVE_ONE 40
#define GO_MOVE_TWO 80
#define GO_MOVE_THREE 100

// CALLBACKS FOR EVENTS
uint32_t UIHandleIDoNothing(UI_EVENT *pUIEvent);
uint32_t UIHandleExit(UI_EVENT *pUIEvent);
uint32_t UIHandleNewMerc(UI_EVENT *pUIEvent);
uint32_t UIHandleNewBadMerc(UI_EVENT *pUIEvent);
uint32_t UIHandleSelectMerc(UI_EVENT *pUIEvent);
uint32_t UIHandleEnterEditMode(UI_EVENT *pUIEvent);
uint32_t UIHandleEnterPalEditMode(UI_EVENT *pUIEvent);
uint32_t UIHandleEndTurn(UI_EVENT *pUIEvent);
uint32_t UIHandleTestHit(UI_EVENT *pUIEvent);
uint32_t UIHandleIOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleIChangeToIdle(UI_EVENT *pUIEvent);
uint32_t UIHandleILoadLevel(UI_EVENT *pUIEvent);
uint32_t UIHandleISoldierDebug(UI_EVENT *pUIEvent);
uint32_t UIHandleILOSDebug(UI_EVENT *pUIEvent);
uint32_t UIHandleILevelNodeDebug(UI_EVENT *pUIEvent);
uint32_t UIHandleIGotoDemoMode(UI_EVENT *pUIEvent);

uint32_t UIHandleILoadFirstLevel(UI_EVENT *pUIEvent);
uint32_t UIHandleILoadSecondLevel(UI_EVENT *pUIEvent);
uint32_t UIHandleILoadThirdLevel(UI_EVENT *pUIEvent);
uint32_t UIHandleILoadFourthLevel(UI_EVENT *pUIEvent);
uint32_t UIHandleILoadFifthLevel(UI_EVENT *pUIEvent);

uint32_t UIHandleIETOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleIETEndTurn(UI_EVENT *pUIEvent);

uint32_t UIHandleMOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleMChangeToAction(UI_EVENT *pUIEvent);
uint32_t UIHandleMCycleMovement(UI_EVENT *pUIEvent);
uint32_t UIHandleMCycleMoveAll(UI_EVENT *pUIEvent);
uint32_t UIHandleMAdjustStanceMode(UI_EVENT *pUIEvent);
uint32_t UIHandleMChangeToHandMode(UI_EVENT *pUIEvent);

uint32_t UIHandlePOPUPMSG(UI_EVENT *pUIEvent);

uint32_t UIHandleAOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleAChangeToMove(UI_EVENT *pUIEvent);
uint32_t UIHandleAChangeToConfirmAction(UI_EVENT *pUIEvent);
uint32_t UIHandleAEndAction(UI_EVENT *pUIEvent);

uint32_t UIHandleMovementMenu(UI_EVENT *pUIEvent);
uint32_t UIHandlePositionMenu(UI_EVENT *pUIEvent);

uint32_t UIHandleCWait(UI_EVENT *pUIEvent);
uint32_t UIHandleCMoveMerc(UI_EVENT *pUIEvent);
uint32_t UIHandleCOnTerrain(UI_EVENT *pUIEvent);

uint32_t UIHandlePADJAdjustStance(UI_EVENT *pUIEvent);

uint32_t UIHandleCAOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleCAMercShoot(UI_EVENT *pUIEvent);
uint32_t UIHandleCAEndConfirmAction(UI_EVENT *pUIEvent);

uint32_t UIHandleHCOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleHCGettingItem(UI_EVENT *pUIEvent);

uint32_t UIHandleLCOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleLCChangeToLook(UI_EVENT *pUIEvent);
uint32_t UIHandleLCLook(UI_EVENT *pUIEvent);

uint32_t UIHandleTATalkingMenu(UI_EVENT *pUIEvent);

uint32_t UIHandleTOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleTChangeToTalking(UI_EVENT *pUIEvent);

uint32_t UIHandleLUIOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleLUIBeginLock(UI_EVENT *pUIEvent);
uint32_t UIHandleLUIEndLock(UI_EVENT *pUIEvent);

uint32_t UIHandleLAOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleLABeginLockOurTurn(UI_EVENT *pUIEvent);
uint32_t UIHandleLAEndLockOurTurn(UI_EVENT *pUIEvent);

uint32_t UIHandleRubberBandOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleJumpOverOnTerrain(UI_EVENT *pUIEvent);
uint32_t UIHandleJumpOver(UI_EVENT *pUIEvent);

uint32_t UIHandleOpenDoorMenu(UI_EVENT *pUIEvent);

uint32_t UIHandleEXExitSectorMenu(UI_EVENT *pUIEvent);

BOOLEAN ValidQuickExchangePosition();

extern void DebugLevelNodePage(void);

int16_t gsTreeRevealXPos, gsTreeRevealYPos;

BOOLEAN HandleMultiSelectionMove(int16_t sDestGridNo);
void ResetMultiSelection();

BOOLEAN SoldierCanAffordNewStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance);
void SetMovementModeCursor(struct SOLDIERTYPE *pSoldier);
void SetConfirmMovementModeCursor(struct SOLDIERTYPE *pSoldier, BOOLEAN fFromMove);
void SetUIbasedOnStance(struct SOLDIERTYPE *pSoldier, int8_t bNewStance);
int8_t DrawUIMovementPath(struct SOLDIERTYPE *pSoldier, uint16_t usMapPos, uint32_t uiFlags);
int8_t UIHandleInteractiveTilesAndItemsOnTerrain(struct SOLDIERTYPE *pSoldier, int16_t usMapPos,
                                                 BOOLEAN fUseOKCursor,
                                                 BOOLEAN fItemsOnlyIfOnIntTiles);

extern void EVENT_InternalSetSoldierDesiredDirection(struct SOLDIERTYPE *pSoldier,
                                                     uint16_t usNewDirection, BOOLEAN fInitalMove,
                                                     uint16_t usAnimState);

extern BOOLEAN gfExitDebugScreen;
extern int8_t gCurDebugPage;
extern BOOLEAN gfGetNewPathThroughPeople;
extern BOOLEAN gfIgnoreOnSelectedGuy;
extern BOOLEAN gfInOpenDoorMenu;

struct SOLDIERTYPE *gpRequesterMerc = NULL;
struct SOLDIERTYPE *gpRequesterTargetMerc = NULL;
int16_t gsRequesterGridNo;
int16_t gsOverItemsGridNo = NOWHERE;
int16_t gsOverItemsLevel = 0;
BOOLEAN gfUIInterfaceSetBusy = FALSE;
uint32_t guiUIInterfaceBusyTime = 0;

uint32_t gfTacticalForceNoCursor = FALSE;
struct LEVELNODE *gpInvTileThatCausedMoveConfirm = NULL;
BOOLEAN gfResetUIMovementOptimization = FALSE;
BOOLEAN gfResetUIItemCursorOptimization = FALSE;
BOOLEAN gfBeginVehicleCursor = FALSE;
uint16_t gsOutOfRangeGridNo = NOWHERE;
uint8_t gubOutOfRangeMerc = NOBODY;
BOOLEAN gfOKForExchangeCursor = FALSE;
uint32_t guiUIInterfaceSwapCursorsTime = 0;
int16_t gsJumpOverGridNo = 0;

UI_EVENT gEvents[NUM_UI_EVENTS] = {
    {0, IDLE_MODE, UIHandleIDoNothing, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, IDLE_MODE, UIHandleExit, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleNewMerc,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleNewBadMerc,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleSelectMerc, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     MOVE_MODE,
     UIHandleEnterEditMode,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     MOVE_MODE,
     UIHandleEnterPalEditMode,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleEndTurn,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleTestHit,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleChangeLevel, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, IDLE_MODE, UIHandleIOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     IDLE_MODE,
     UIHandleIChangeToIdle,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, IDLE_MODE, UIHandleILoadLevel, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleISoldierDebug,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILOSDebug,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILevelNodeDebug,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleIGotoDemoMode,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILoadFirstLevel,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILoadSecondLevel,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILoadThirdLevel,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILoadFourthLevel,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     DONT_CHANGEMODE,
     UIHandleILoadFifthLevel,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {0, ENEMYS_TURN_MODE, UIHandleIETOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleIETEndTurn, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, MOVE_MODE, UIHandleMOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     ACTION_MODE,
     UIHandleMChangeToAction,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     HANDCURSOR_MODE,
     UIHandleMChangeToHandMode,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     MOVE_MODE,
     UIHandleMCycleMovement,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     CONFIRM_MOVE_MODE,
     UIHandleMCycleMoveAll,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SNAPMOUSE,
     ADJUST_STANCE_MODE,
     UIHandleMAdjustStanceMode,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {0, POPUP_MODE, UIHandlePOPUPMSG, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, ACTION_MODE, UIHandleAOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     MOVE_MODE,
     UIHandleAChangeToMove,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     CONFIRM_ACTION_MODE,
     UIHandleAChangeToConfirmAction,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleAEndAction, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SNAPMOUSE, MENU_MODE, UIHandleMovementMenu, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SNAPMOUSE, MENU_MODE, UIHandlePositionMenu, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, CONFIRM_MOVE_MODE, UIHandleCWait, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleCMoveMerc, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, CONFIRM_MOVE_MODE, UIHandleCOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, MOVE_MODE, UIHandlePADJAdjustStance, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, CONFIRM_ACTION_MODE, UIHandleCAOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     ACTION_MODE,
     UIHandleCAMercShoot,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     ACTION_MODE,
     UIHandleCAEndConfirmAction,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {0, HANDCURSOR_MODE, UIHandleHCOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, GETTINGITEM_MODE, UIHandleHCGettingItem, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, LOOKCURSOR_MODE, UIHandleLCOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     LOOKCURSOR_MODE,
     UIHandleLCChangeToLook,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleLCLook, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, TALKINGMENU_MODE, UIHandleTATalkingMenu, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, TALKCURSOR_MODE, UIHandleTOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     TALKCURSOR_MODE,
     UIHandleTChangeToTalking,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {0, LOCKUI_MODE, UIHandleLUIOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, LOCKUI_MODE, UIHandleLUIBeginLock, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT, MOVE_MODE, UIHandleLUIEndLock, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, OPENDOOR_MENU_MODE, UIHandleOpenDoorMenu, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, LOCKOURTURN_UI_MODE, UIHandleLAOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, LOCKOURTURN_UI_MODE, UIHandleLABeginLockOurTurn, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {UIEVENT_SINGLEEVENT,
     MOVE_MODE,
     UIHandleLAEndLockOurTurn,
     FALSE,
     FALSE,
     DONT_CHANGEMODE,
     {0, 0, 0}},
    {0, EXITSECTORMENU_MODE, UIHandleEXExitSectorMenu, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, RUBBERBAND_MODE, UIHandleRubberBandOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, JUMPOVER_MODE, UIHandleJumpOverOnTerrain, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
    {0, MOVE_MODE, UIHandleJumpOver, FALSE, FALSE, DONT_CHANGEMODE, {0, 0, 0}},
};

UI_MODE gCurrentUIMode = IDLE_MODE;
UI_MODE gOldUIMode = IDLE_MODE;
uint32_t guiCurrentEvent = I_DO_NOTHING;
uint32_t guiOldEvent = I_DO_NOTHING;
uint32_t guiCurrentUICursor = NO_UICURSOR;
uint32_t guiNewUICursor = NORMAL_SNAPUICURSOR;
uint32_t guiPendingOverrideEvent = I_DO_NOTHING;
uint16_t gusSavedMouseX;
uint16_t gusSavedMouseY;
UIKEYBOARD_HOOK gUIKeyboardHook = NULL;
BOOLEAN gUIActionModeChangeDueToMouseOver = FALSE;

BOOLEAN gfDisplayTimerCursor = FALSE;
uint32_t guiTimerCursorID = 0;
uint32_t guiTimerLastUpdate = 0;
uint32_t guiTimerCursorDelay = 0;

wchar_t gzLocation[20];
BOOLEAN gfLocation = FALSE;

wchar_t gzIntTileLocation[20];
BOOLEAN gfUIIntTileLocation;

wchar_t gzIntTileLocation2[20];
BOOLEAN gfUIIntTileLocation2;

struct MOUSE_REGION gDisableRegion;
BOOLEAN gfDisableRegionActive = FALSE;

struct MOUSE_REGION gUserTurnRegion;
BOOLEAN gfUserTurnRegionActive = FALSE;

// For use with mouse button query routines
BOOLEAN fRightButtonDown = FALSE;
BOOLEAN fLeftButtonDown = FALSE;
BOOLEAN fIgnoreLeftUp = FALSE;

BOOLEAN gUITargetReady = FALSE;
BOOLEAN gUITargetShotWaiting = FALSE;
uint16_t gsUITargetShotGridNo = NOWHERE;
BOOLEAN gUIUseReverse = FALSE;

SGPRect gRubberBandRect = {0, 0, 0, 0};
BOOLEAN gRubberBandActive = FALSE;
BOOLEAN gfIgnoreOnSelectedGuy = FALSE;
BOOLEAN gfViewPortAdjustedForSouth = FALSE;

int16_t guiCreateGuyIndex = 0;
// Temp values for placing bad guys
int16_t guiCreateBadGuyIndex = 8;

uint32_t guiUIFullTargetFlags;
uint16_t gusUISelectiveTargetID;
BOOLEAN gfUISelectiveTargetFound;
uint32_t guiUISelectiveTargetFlags;
BOOLEAN gfUIBodyHitLocation;

// FLAGS
// These flags are set for a single frame execution and then are reset for the next iteration.
BOOLEAN gfUIDisplayActionPoints = FALSE;
BOOLEAN gfUIDisplayActionPointsInvalid = FALSE;
BOOLEAN gfUIDisplayActionPointsBlack = FALSE;
BOOLEAN gfUIDisplayActionPointsCenter = FALSE;

int16_t gUIDisplayActionPointsOffY = 0;
int16_t gUIDisplayActionPointsOffX = 0;
BOOLEAN gfUIDoNotHighlightSelMerc = FALSE;
BOOLEAN gfUIHandleSelection = FALSE;
BOOLEAN gfUIHandleSelectionAboveGuy = FALSE;
BOOLEAN gfUIInDeadlock = FALSE;
uint8_t gUIDeadlockedSoldier = NOBODY;
BOOLEAN gfUIHandleShowMoveGrid = FALSE;
uint16_t gsUIHandleShowMoveGridLocation = NOWHERE;
BOOLEAN gfUIOverItemPool = FALSE;
int16_t gfUIOverItemPoolGridNo = 0;
BOOLEAN gfUIHandlePhysicsTrajectory = FALSE;
BOOLEAN gfUIMouseOnValidCatcher = FALSE;
uint8_t gubUIValidCatcherID = 0;

BOOLEAN gfUIConfirmExitArrows = FALSE;

BOOLEAN gfUIShowCurIntTile = FALSE;

BOOLEAN gfUIWaitingForUserSpeechAdvance =
    FALSE;                                 // Waiting for key input/mouse click to advance speech
BOOLEAN gfUIKeyCheatModeOn = FALSE;        // Sets cool cheat keys on
BOOLEAN gfUIAllMoveOn = FALSE;             // Sets to all move
BOOLEAN gfUICanBeginAllMoveCycle = FALSE;  // GEts set so we know that the next right-click is a
                                           // move-call inc\stead of a movement cycle through

int16_t gsSelectedGridNo = 0;
int16_t gsSelectedLevel = I_GROUND_LEVEL;
int16_t gsSelectedGuy = NO_SOLDIER;

BOOLEAN gfUIDisplayDamage = FALSE;
int8_t gbDamage = 0;
uint16_t gsDamageGridNo = 0;

BOOLEAN gfUIRefreshArrows = FALSE;

// Thse flags are not re-set after each frame
BOOLEAN gfPlotNewMovement = FALSE;
BOOLEAN gfPlotNewMovementNOCOST = FALSE;
uint32_t guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;
int8_t gbAdjustStanceDiff = 0;
int8_t gbClimbID = 0;

BOOLEAN gfUIShowExitEast = FALSE;
BOOLEAN gfUIShowExitWest = FALSE;
BOOLEAN gfUIShowExitNorth = FALSE;
BOOLEAN gfUIShowExitSouth = FALSE;
BOOLEAN gfUIShowExitExitGrid = FALSE;

BOOLEAN gfUINewStateForIntTile = FALSE;

BOOLEAN gfUIForceReExamineCursorData = FALSE;

BOOLEAN gfUIFullTargetFound;
uint16_t gusUIFullTargetID;

void SetUIMouseCursor();
void ClearEvent(UI_EVENT *pUIEvent);
uint8_t GetAdjustedAnimHeight(uint8_t ubAnimHeight, int8_t bChange);

// MAIN TACTICAL UI HANDLER
uint32_t HandleTacticalUI(void) {
  uint32_t ReturnVal = GAME_SCREEN;
  uint32_t uiNewEvent;
  int16_t usMapPos;
  struct LEVELNODE *pIntTile;
  static struct LEVELNODE *pOldIntTile = NULL;

  // RESET FLAGS
  gfUIDisplayActionPoints = FALSE;
  gfUIDisplayActionPointsInvalid = FALSE;
  gfUIDisplayActionPointsBlack = FALSE;
  gfUIDisplayActionPointsCenter = FALSE;
  gfUIDoNotHighlightSelMerc = FALSE;
  gfUIHandleSelection = NO_GUY_SELECTION;
  gfUIHandleSelectionAboveGuy = FALSE;
  gfUIDisplayDamage = FALSE;
  guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;
  gfUIBodyHitLocation = FALSE;
  gfUIIntTileLocation = FALSE;
  gfUIIntTileLocation2 = FALSE;
  // gfUIForceReExamineCursorData		= FALSE;
  gfUINewStateForIntTile = FALSE;
  gfUIShowExitExitGrid = FALSE;
  gfUIOverItemPool = FALSE;
  gfUIHandlePhysicsTrajectory = FALSE;
  gfUIMouseOnValidCatcher = FALSE;
  gfIgnoreOnSelectedGuy = FALSE;

  // Set old event value
  guiOldEvent = uiNewEvent = guiCurrentEvent;

  if (gfUIInterfaceSetBusy) {
    if ((GetJA2Clock() - guiUIInterfaceBusyTime) > 25000) {
      gfUIInterfaceSetBusy = FALSE;

      // UNLOCK UI
      UnSetUIBusy((uint8_t)gusSelectedSoldier);

      // Decrease global busy  counter...
      gTacticalStatus.ubAttackBusyCount = 0;
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("Setting attack busy count to 0 due to ending AI lock"));

      guiPendingOverrideEvent = LU_ENDUILOCK;
      UIHandleLUIEndLock(NULL);
    }
  }

  if ((GetJA2Clock() - guiUIInterfaceSwapCursorsTime) > 1000) {
    gfOKForExchangeCursor = !gfOKForExchangeCursor;
    guiUIInterfaceSwapCursorsTime = GetJA2Clock();
  }

  // OK, do a check for on an int tile...
  pIntTile = GetCurInteractiveTile();

  if (pIntTile != pOldIntTile) {
    gfUINewStateForIntTile = TRUE;

    pOldIntTile = pIntTile;
  }

  if (guiPendingOverrideEvent == I_DO_NOTHING) {
    // When we are here, guiCurrentEvent is set to the last event
    // Within the input gathering phase, it may change

    // GATHER INPUT
    // Any new event will overwrite previous events. Therefore,
    // PRIOTITIES GO LIKE THIS:
    //						Mouse Movement
    //						Keyboard Polling
    //						Mouse Buttons
    //						Keyboard Queued Events ( will override always )

    // SWITCH ON INPUT GATHERING, DEPENDING ON MODE
    // IF WE ARE NOT IN COMBAT OR IN REALTIME COMBAT
    if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
      // FROM MOUSE POSITION
      GetRTMousePositionInput(&uiNewEvent);
      // FROM KEYBOARD POLLING
      GetPolledKeyboardInput(&uiNewEvent);
      // FROM MOUSE CLICKS
      GetRTMouseButtonInput(&uiNewEvent);
      // FROM KEYBOARD
      GetKeyboardInput(&uiNewEvent);

    } else {
      // FROM MOUSE POSITION
      GetTBMousePositionInput(&uiNewEvent);
      // FROM KEYBOARD POLLING
      GetPolledKeyboardInput(&uiNewEvent);
      // FROM MOUSE CLICKS
      GetTBMouseButtonInput(&uiNewEvent);
      // FROM KEYBOARD
      GetKeyboardInput(&uiNewEvent);
    }

  } else {
    uiNewEvent = guiPendingOverrideEvent;
    guiPendingOverrideEvent = I_DO_NOTHING;
  }

  if (HandleItemPickupMenu()) {
    uiNewEvent = A_CHANGE_TO_MOVE;
  }

  // Set Current event to new one!
  guiCurrentEvent = uiNewEvent;

  // ATE: New! Get flags for over soldier or not...
  gfUIFullTargetFound = FALSE;
  gfUISelectiveTargetFound = FALSE;

  if (GetMouseMapPos(&usMapPos)) {
    // Look for soldier full
    if (FindSoldier(usMapPos, &gusUIFullTargetID, &guiUIFullTargetFlags,
                    (FINDSOLDIERSAMELEVEL(gsInterfaceLevel)))) {
      gfUIFullTargetFound = TRUE;
    }

    // Look for soldier selective
    if (FindSoldier(usMapPos, &gusUISelectiveTargetID, &guiUISelectiveTargetFlags,
                    FINDSOLDIERSELECTIVESAMELEVEL(gsInterfaceLevel))) {
      gfUISelectiveTargetFound = TRUE;
    }
  }

  // Check if current event has changed and clear event if so, to prepare it for execution
  // Clearing it does things like set first time flag, param variavles, etc
  if (uiNewEvent != guiOldEvent) {
    // Snap mouse back if it's that type
    if (gEvents[guiOldEvent].uiFlags & UIEVENT_SNAPMOUSE) {
      SimulateMouseMovement((uint32_t)gusSavedMouseX, (uint32_t)gusSavedMouseY);
    }

    ClearEvent(&gEvents[uiNewEvent]);
  }

  // Restore not scrolling from stance adjust....
  if (gOldUIMode == ADJUST_STANCE_MODE) {
    gfIgnoreScrolling = FALSE;
  }

  // IF this event is of type snap mouse, save position
  if (gEvents[uiNewEvent].uiFlags & UIEVENT_SNAPMOUSE && gEvents[uiNewEvent].fFirstTime) {
    // Save mouse position
    gusSavedMouseX = gusMouseXPos;
    gusSavedMouseY = gusMouseYPos;
  }

  // HANDLE UI EVENT
  ReturnVal = gEvents[uiNewEvent].HandleEvent(&(gEvents[uiNewEvent]));

  if (gfInOpenDoorMenu) {
    return (ReturnVal);
  }

  // Set first time flag to false, now that it has been executed
  gEvents[uiNewEvent].fFirstTime = FALSE;

  // Check if UI mode has changed from previous event
  if (gEvents[uiNewEvent].ChangeToUIMode != gCurrentUIMode &&
      (gEvents[uiNewEvent].ChangeToUIMode != DONT_CHANGEMODE)) {
    gEvents[uiNewEvent].uiMenuPreviousMode = gCurrentUIMode;

    gOldUIMode = gCurrentUIMode;

    gCurrentUIMode = gEvents[uiNewEvent].ChangeToUIMode;

    // CHANGE MODE - DO SPECIAL THINGS IF WE ENTER THIS MODE
    if (gCurrentUIMode == ACTION_MODE) {
      ErasePath(TRUE);
    }
  }

  // Check if menu event is done and if so set to privious mode
  // This is needed to hook into the interface stuff which sets the fDoneMenu flag
  if (gEvents[uiNewEvent].fDoneMenu == TRUE) {
    if (gCurrentUIMode == MENU_MODE || gCurrentUIMode == POPUP_MODE ||
        gCurrentUIMode == LOOKCURSOR_MODE) {
      gCurrentUIMode = gEvents[uiNewEvent].uiMenuPreviousMode;
    }
  }
  // Check to return to privious mode
  // If the event is a single event, return to previous
  if (gEvents[uiNewEvent].uiFlags & UIEVENT_SINGLEEVENT) {
    // ATE: OK - don't revert to single event if our mouse is not
    // in viewport - rather use m_on_t event
    if ((gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA)) {
      guiCurrentEvent = guiOldEvent;
    } else {
      // ATE: Check first that some modes are met....
      if (gCurrentUIMode != HANDCURSOR_MODE && gCurrentUIMode != LOOKCURSOR_MODE &&
          gCurrentUIMode != TALKCURSOR_MODE) {
        guiCurrentEvent = M_ON_TERRAIN;
      }
    }
  }

  // Donot display APs if not in combat
  if (!(gTacticalStatus.uiFlags & INCOMBAT) || (gTacticalStatus.uiFlags & REALTIME)) {
    gfUIDisplayActionPoints = FALSE;
  }

  // Will set the cursor but only if different
  SetUIMouseCursor();

  // ATE: Check to reset selected guys....
  if (gTacticalStatus.fAtLeastOneGuyOnMultiSelect) {
    // If not in MOVE_MODE, CONFIRM_MOVE_MODE, RUBBERBAND_MODE, stop....
    if (gCurrentUIMode != MOVE_MODE && gCurrentUIMode != CONFIRM_MOVE_MODE &&
        gCurrentUIMode != RUBBERBAND_MODE && gCurrentUIMode != ADJUST_STANCE_MODE &&
        gCurrentUIMode != TALKCURSOR_MODE && gCurrentUIMode != LOOKCURSOR_MODE) {
      ResetMultiSelection();
    }
  }

  return (ReturnVal);
}

void SetUIMouseCursor() {
  uint32_t uiCursorFlags;
  uint32_t uiTraverseTimeInMinutes;
  BOOLEAN fForceUpdateNewCursor = FALSE;
  BOOLEAN fUpdateNewCursor = TRUE;
  static int16_t sOldExitGridNo = NOWHERE;
  static BOOLEAN fOkForExit = FALSE;

  // Check if we moved from confirm mode on exit arrows
  // If not in move mode, return!
  if (gCurrentUIMode == MOVE_MODE) {
    if (gfUIConfirmExitArrows) {
      GetCursorMovementFlags(&uiCursorFlags);

      if (uiCursorFlags & MOUSE_MOVING) {
        gfUIConfirmExitArrows = FALSE;
      }
    }

    if (gfUIShowExitEast) {
      gfUIDisplayActionPoints = FALSE;
      ErasePath(TRUE);

      if (OKForSectorExit(EAST_STRATEGIC_MOVE, 0, &uiTraverseTimeInMinutes)) {
        if (gfUIConfirmExitArrows) {
          guiNewUICursor = CONFIRM_EXIT_EAST_UICURSOR;
        } else {
          guiNewUICursor = EXIT_EAST_UICURSOR;
        }
      } else {
        guiNewUICursor = NOEXIT_EAST_UICURSOR;
      }

      if (gusMouseXPos < 635) {
        gfUIShowExitEast = FALSE;
      }
    }

    if (gfUIShowExitWest) {
      gfUIDisplayActionPoints = FALSE;
      ErasePath(TRUE);

      if (OKForSectorExit(WEST_STRATEGIC_MOVE, 0, &uiTraverseTimeInMinutes)) {
        if (gfUIConfirmExitArrows) {
          guiNewUICursor = CONFIRM_EXIT_WEST_UICURSOR;
        } else {
          guiNewUICursor = EXIT_WEST_UICURSOR;
        }
      } else {
        guiNewUICursor = NOEXIT_WEST_UICURSOR;
      }

      if (gusMouseXPos > 5) {
        gfUIShowExitWest = FALSE;
      }
    }

    if (gfUIShowExitNorth) {
      gfUIDisplayActionPoints = FALSE;
      ErasePath(TRUE);

      if (OKForSectorExit(NORTH_STRATEGIC_MOVE, 0, &uiTraverseTimeInMinutes)) {
        if (gfUIConfirmExitArrows) {
          guiNewUICursor = CONFIRM_EXIT_NORTH_UICURSOR;
        } else {
          guiNewUICursor = EXIT_NORTH_UICURSOR;
        }
      } else {
        guiNewUICursor = NOEXIT_NORTH_UICURSOR;
      }

      if (gusMouseYPos > 5) {
        gfUIShowExitNorth = FALSE;
      }
    }

    if (gfUIShowExitSouth) {
      gfUIDisplayActionPoints = FALSE;
      ErasePath(TRUE);

      if (OKForSectorExit(SOUTH_STRATEGIC_MOVE, 0, &uiTraverseTimeInMinutes)) {
        if (gfUIConfirmExitArrows) {
          guiNewUICursor = CONFIRM_EXIT_SOUTH_UICURSOR;
        } else {
          guiNewUICursor = EXIT_SOUTH_UICURSOR;
        }
      } else {
        guiNewUICursor = NOEXIT_SOUTH_UICURSOR;
      }

      if (gusMouseYPos < 478) {
        gfUIShowExitSouth = FALSE;

        // Define region for viewport
        MSYS_RemoveRegion(&gViewportRegion);

        MSYS_DefineRegion(&gViewportRegion, 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y,
                          MSYS_PRIORITY_NORMAL, VIDEO_NO_CURSOR, MSYS_NO_CALLBACK,
                          MSYS_NO_CALLBACK);

        // Adjust where we blit our cursor!
        gsGlobalCursorYOffset = 0;
        SetCurrentCursorFromDatabase(CURSOR_NORMAL);
      } else {
        if (gfScrollPending || gfScrollInertia) {
        } else {
          // Adjust viewport to edge of screen!
          // Define region for viewport
          MSYS_RemoveRegion(&gViewportRegion);
          MSYS_DefineRegion(&gViewportRegion, 0, 0, gsVIEWPORT_END_X, 480, MSYS_PRIORITY_NORMAL,
                            VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

          gsGlobalCursorYOffset = (480 - gsVIEWPORT_WINDOW_END_Y);
          SetCurrentCursorFromDatabase(gUICursors[guiNewUICursor].usFreeCursorName);

          gfViewPortAdjustedForSouth = TRUE;
        }
      }
    } else {
      if (gfViewPortAdjustedForSouth) {
        // Define region for viewport
        MSYS_RemoveRegion(&gViewportRegion);

        MSYS_DefineRegion(&gViewportRegion, 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y,
                          MSYS_PRIORITY_NORMAL, VIDEO_NO_CURSOR, MSYS_NO_CALLBACK,
                          MSYS_NO_CALLBACK);

        // Adjust where we blit our cursor!
        gsGlobalCursorYOffset = 0;
        SetCurrentCursorFromDatabase(CURSOR_NORMAL);

        gfViewPortAdjustedForSouth = FALSE;
      }
    }

    if (gfUIShowExitExitGrid) {
      int16_t usMapPos;
      uint8_t ubRoomNum;

      gfUIDisplayActionPoints = FALSE;
      ErasePath(TRUE);

      if (GetMouseMapPos(&usMapPos)) {
        if (gusSelectedSoldier != NOBODY && MercPtrs[gusSelectedSoldier]->bLevel == 0) {
          // ATE: Is this place revealed?
          if (!InARoom(usMapPos, &ubRoomNum) ||
              (InARoom(usMapPos, &ubRoomNum) &&
               gpWorldLevelData[usMapPos].uiFlags & MAPELEMENT_REVEALED)) {
            if (sOldExitGridNo != usMapPos) {
              fOkForExit = OKForSectorExit((int8_t)-1, usMapPos, &uiTraverseTimeInMinutes);
              sOldExitGridNo = usMapPos;
            }

            if (fOkForExit) {
              if (gfUIConfirmExitArrows) {
                guiNewUICursor = CONFIRM_EXIT_GRID_UICURSOR;
              } else {
                guiNewUICursor = EXIT_GRID_UICURSOR;
              }
            } else {
              guiNewUICursor = NOEXIT_GRID_UICURSOR;
            }
          }
        }
      }
    } else {
      sOldExitGridNo = NOWHERE;
    }

  } else {
    gsGlobalCursorYOffset = 0;
  }

  if (gfDisplayTimerCursor) {
    SetUICursor(guiTimerCursorID);

    fUpdateNewCursor = FALSE;

    if ((GetJA2Clock() - guiTimerLastUpdate) > guiTimerCursorDelay) {
      gfDisplayTimerCursor = FALSE;

      // OK, timer may be different, update...
      fForceUpdateNewCursor = TRUE;
      fUpdateNewCursor = TRUE;
    }
  }

  if (fUpdateNewCursor) {
    if (!gfTacticalForceNoCursor) {
      if (guiNewUICursor != guiCurrentUICursor || fForceUpdateNewCursor) {
        SetUICursor(guiNewUICursor);

        guiCurrentUICursor = guiNewUICursor;
      }
    }
  }
}

void SetUIKeyboardHook(UIKEYBOARD_HOOK KeyboardHookFnc) { gUIKeyboardHook = KeyboardHookFnc; }

void ClearEvent(UI_EVENT *pUIEvent) {
  memset(pUIEvent->uiParams, 0, sizeof(pUIEvent->uiParams));
  pUIEvent->fDoneMenu = FALSE;
  pUIEvent->fFirstTime = TRUE;
  pUIEvent->uiMenuPreviousMode = DONT_CHANGEMODE;
}

void EndMenuEvent(uint32_t uiEvent) { gEvents[uiEvent].fDoneMenu = TRUE; }

// HANDLER FUCNTIONS

uint32_t UIHandleIDoNothing(UI_EVENT *pUIEvent) {
  guiNewUICursor = NORMAL_SNAPUICURSOR;

  return (GAME_SCREEN);
}

uint32_t UIHandleExit(UI_EVENT *pUIEvent) {
  gfProgramIsRunning = FALSE;
  return (GAME_SCREEN);
}

uint32_t UIHandleNewMerc(UI_EVENT *pUIEvent) {
  static uint8_t ubTemp = 3;
  int16_t usMapPos;
  MERC_HIRE_STRUCT HireMercStruct;
  int8_t bReturnCode;
  struct SOLDIERTYPE *pSoldier;

  // Get Grid Corrdinates of mouse
  if (GetMouseMapPos(&usMapPos)) {
    ubTemp += 2;

    memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

    HireMercStruct.ubProfileID = ubTemp;

    // DEF: temp
    HireMercStruct.sSectorX = gWorldSectorX;
    HireMercStruct.sSectorY = gWorldSectorY;
    HireMercStruct.bSectorZ = gbWorldSectorZ;
    HireMercStruct.ubInsertionCode = INSERTION_CODE_GRIDNO;
    HireMercStruct.usInsertionData = usMapPos;
    HireMercStruct.fCopyProfileItemsOver = TRUE;
    HireMercStruct.iTotalContractLength = 7;

    // specify when the merc should arrive
    HireMercStruct.uiTimeTillMercArrives = 0;

    // if we succesfully hired the merc
    bReturnCode = HireMerc(&HireMercStruct);

    if (bReturnCode == MERC_HIRE_FAILED) {
      ScreenMsg(FONT_ORANGE, MSG_BETAVERSION,
                L"Merc hire failed:  Either already hired or dislikes you.");
    } else if (bReturnCode == MERC_HIRE_OVER_20_MERCS_HIRED) {
      ScreenMsg(FONT_ORANGE, MSG_BETAVERSION, L"Can't hire more than 20 mercs.");
    } else {
      // Get soldier from profile
      pSoldier = FindSoldierByProfileID(ubTemp, FALSE);

      MercArrivesCallback(pSoldier->ubID);
      SelectSoldier(pSoldier->ubID, FALSE, TRUE);
    }
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleNewBadMerc(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  uint16_t usRandom;

  // Get map postion and place the enemy there.
  if (GetMouseMapPos(&usMapPos)) {
    // Are we an OK dest?
    if (!IsLocationSittable(usMapPos, 0)) {
      return (GAME_SCREEN);
    }

    usRandom = (uint16_t)Random(10);
    if (usRandom < 4)
      pSoldier = TacticalCreateAdministrator();
    else if (usRandom < 8)
      pSoldier = TacticalCreateArmyTroop();
    else
      pSoldier = TacticalCreateEliteEnemy();

    // Add soldier strategic info, so it doesn't break the counters!
    if (pSoldier) {
      if (!gbWorldSectorZ) {
        SECTORINFO *pSector =
            &SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)];
        switch (pSoldier->ubSoldierClass) {
          case SOLDIER_CLASS_ADMINISTRATOR:
            pSector->ubNumAdmins++;
            pSector->ubAdminsInBattle++;
            break;
          case SOLDIER_CLASS_ARMY:
            pSector->ubNumTroops++;
            pSector->ubTroopsInBattle++;
            break;
          case SOLDIER_CLASS_ELITE:
            pSector->ubNumElites++;
            pSector->ubElitesInBattle++;
            break;
        }
      } else {
        UNDERGROUND_SECTORINFO *pSector =
            FindUnderGroundSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
        if (pSector) {
          switch (pSoldier->ubSoldierClass) {
            case SOLDIER_CLASS_ADMINISTRATOR:
              pSector->ubNumAdmins++;
              pSector->ubAdminsInBattle++;
              break;
            case SOLDIER_CLASS_ARMY:
              pSector->ubNumTroops++;
              pSector->ubTroopsInBattle++;
              break;
            case SOLDIER_CLASS_ELITE:
              pSector->ubNumElites++;
              pSector->ubElitesInBattle++;
              break;
          }
        }
      }

      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
      pSoldier->usStrategicInsertionData = usMapPos;
      UpdateMercInSector(pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
      AllTeamsLookForAll(NO_INTERRUPTS);
    }
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleEnterEditMode(UI_EVENT *pUIEvent) { return (EDIT_SCREEN); }

uint32_t UIHandleEnterPalEditMode(UI_EVENT *pUIEvent) { return (PALEDIT_SCREEN); }

uint32_t UIHandleEndTurn(UI_EVENT *pUIEvent) {
  // CANCEL FROM PLANNING MODE!
  if (InUIPlanMode()) {
    EndUIPlan();
  }

  // ATE: If we have an item pointer end it!
  CancelItemPointer();

  // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ ENDING_TURN ] );

  if (CheckForEndOfCombatMode(FALSE)) {
    // do nothing...
  } else {
    EndTurn(gbPlayerNum + 1);
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleTestHit(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int8_t bDamage;

  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (gfUIFullTargetFound) {
    // Get Soldier
    GetSoldier(&pSoldier, gusUIFullTargetID);

    if (_KeyDown(SHIFT)) {
      pSoldier->bBreath -= 30;

      if (pSoldier->bBreath < 0) pSoldier->bBreath = 0;

      bDamage = 1;
    } else {
      if (Random(2)) {
        bDamage = 20;
      } else {
        bDamage = 25;
      }
    }

    gTacticalStatus.ubAttackBusyCount++;

    EVENT_SoldierGotHit(pSoldier, 1, bDamage, 10, pSoldier->bDirection, 320, NOBODY,
                        FIRE_WEAPON_NO_SPECIAL, pSoldier->bAimShotLocation, 0, NOWHERE);
  }
  return (GAME_SCREEN);
}

void ChangeInterfaceLevel(int16_t sLevel) {
  // Only if different!
  if (sLevel == gsInterfaceLevel) {
    return;
  }

  gsInterfaceLevel = sLevel;

  if (gsInterfaceLevel == 1) {
    gsRenderHeight += ROOF_LEVEL_HEIGHT;
    gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
    InvalidateWorldRedundency();
  } else if (gsInterfaceLevel == 0) {
    gsRenderHeight -= ROOF_LEVEL_HEIGHT;
    gTacticalStatus.uiFlags &= (~SHOW_ALL_ROOFS);
    InvalidateWorldRedundency();
  }

  SetRenderFlags(RENDER_FLAG_FULL);
  // Remove any interactive tiles we could be over!
  BeginCurInteractiveTileCheck(INTILE_CHECK_SELECTIVE);
  gfPlotNewMovement = TRUE;
  ErasePath(FALSE);
}

uint32_t UIHandleChangeLevel(UI_EVENT *pUIEvent) {
  if (gsInterfaceLevel == 0) {
    ChangeInterfaceLevel(1);
  } else if (gsInterfaceLevel == 1) {
    ChangeInterfaceLevel(0);
  }

  return (GAME_SCREEN);
}

extern void InternalSelectSoldier(uint16_t usSoldierID, BOOLEAN fAcknowledge,
                                  BOOLEAN fForceReselect, BOOLEAN fFromUI);

uint32_t UIHandleSelectMerc(UI_EVENT *pUIEvent) {
  int32_t iCurrentSquad;

  // Get merc index at mouse and set current selection
  if (gfUIFullTargetFound) {
    iCurrentSquad = CurrentSquad();

    InternalSelectSoldier(gusUIFullTargetID, TRUE, FALSE, TRUE);

    // If different, display message
    if (CurrentSquad() != iCurrentSquad) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_SQUAD_ACTIVE],
                (CurrentSquad() + 1));
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleMOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  BOOLEAN fSetCursor = FALSE;
  uint32_t uiCursorFlags;
  struct LEVELNODE *pIntNode;
  EXITGRID ExitGrid;
  int16_t sIntTileGridNo;
  struct ITEM_POOL *pItemPool;

  static int16_t sGridNoForItemsOver;
  static int8_t bLevelForItemsOver;
  static uint32_t uiItemsOverTimer;
  static BOOLEAN fOverItems;

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  gUIActionModeChangeDueToMouseOver = FALSE;

  // If we are a vehicle..... just show an X
  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    if ((OK_ENTERABLE_VEHICLE(pSoldier))) {
      if (!UIHandleOnMerc(TRUE)) {
        guiNewUICursor = FLOATING_X_UICURSOR;
        return (GAME_SCREEN);
      }
    }
  }

  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (!UIHandleOnMerc(TRUE)) {
    // Are we over items...
    if (GetItemPool(usMapPos, &pItemPool, (uint8_t)gsInterfaceLevel) &&
        ITEMPOOL_VISIBLE(pItemPool)) {
      // Are we already in...
      if (fOverItems) {
        // Is this the same level & gridno...
        if (gsInterfaceLevel == (int16_t)bLevelForItemsOver && usMapPos == sGridNoForItemsOver) {
          // Check timer...
          if ((GetJA2Clock() - uiItemsOverTimer) > 1500) {
            // Change to hand curso mode
            guiPendingOverrideEvent = M_CHANGE_TO_HANDMODE;
            gsOverItemsGridNo = usMapPos;
            gsOverItemsLevel = gsInterfaceLevel;
            fOverItems = FALSE;
          }
        } else {
          uiItemsOverTimer = GetJA2Clock();
          bLevelForItemsOver = (int8_t)gsInterfaceLevel;
          sGridNoForItemsOver = usMapPos;
        }
      } else {
        fOverItems = TRUE;

        uiItemsOverTimer = GetJA2Clock();
        bLevelForItemsOver = (int8_t)gsInterfaceLevel;
        sGridNoForItemsOver = usMapPos;
      }
    } else {
      fOverItems = FALSE;
    }

    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
      if (pSoldier->sGridNo == NOWHERE) {
      }

      if (GetExitGrid(usMapPos, &ExitGrid) && pSoldier->bLevel == 0) {
        gfUIShowExitExitGrid = TRUE;
      }

      // ATE: Draw invalidc cursor if heights different
      if (gpWorldLevelData[usMapPos].sHeight != gpWorldLevelData[pSoldier->sGridNo].sHeight) {
        // ERASE PATH
        ErasePath(TRUE);

        guiNewUICursor = FLOATING_X_UICURSOR;

        return (GAME_SCREEN);
      }
    }

    // DO SOME CURSOR POSITION FLAGS SETTING
    GetCursorMovementFlags(&uiCursorFlags);

    if (gusSelectedSoldier != NO_SOLDIER) {
      // Get Soldier Pointer
      GetSoldier(&pSoldier, gusSelectedSoldier);

      // Get interactvie tile node
      pIntNode = GetCurInteractiveTileGridNo(&sIntTileGridNo);

      // Check were we are
      // CHECK IF WE CAN MOVE HERE
      // THIS IS JUST A CRUDE TEST FOR NOW
      if (pSoldier->bLife < OKLIFE) {
        uint8_t ubID;
        // Show reg. cursor
        // GO INTO IDLE MODE
        // guiPendingOverrideEvent = I_CHANGE_TO_IDLE;
        // gusSelectedSoldier = NO_SOLDIER;
        ubID = FindNextActiveAndAliveMerc(pSoldier, FALSE, FALSE);

        if (ubID != NOBODY) {
          SelectSoldier((int16_t)ubID, FALSE, FALSE);
        } else {
          gusSelectedSoldier = NO_SOLDIER;
          // Change UI mode to reflact that we are selected
          guiPendingOverrideEvent = I_ON_TERRAIN;
        }
      } else if ((UIOKMoveDestination(pSoldier, usMapPos) != 1) && pIntNode == NULL) {
        // ERASE PATH
        ErasePath(TRUE);

        guiNewUICursor = CANNOT_MOVE_UICURSOR;

      } else {
        if (!UIHandleInteractiveTilesAndItemsOnTerrain(pSoldier, usMapPos, FALSE, TRUE)) {
          // Are we in combat?
          if ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED)) {
            // If so, draw path, etc
            fSetCursor = HandleUIMovementCursor(pSoldier, uiCursorFlags, usMapPos, 0);
          } else {
            // Donot draw path until confirm
            fSetCursor = TRUE;

            // If so, draw path, etc
            fSetCursor = HandleUIMovementCursor(pSoldier, uiCursorFlags, usMapPos, 0);

            // ErasePath( TRUE );
          }

        } else {
          fSetCursor = TRUE;
        }
      }
    } else {
      // IF GUSSELECTEDSOLDIER != NOSOLDIER
      guiNewUICursor = NORMAL_SNAPUICURSOR;
    }
  } else {
    if (ValidQuickExchangePosition()) {
      // Do new cursor!
      guiNewUICursor = EXCHANGE_PLACES_UICURSOR;
    }
  }

  {
    // if ( fSetCursor && guiNewUICursor != ENTER_VEHICLE_UICURSOR )
    if (fSetCursor && !gfBeginVehicleCursor) {
      SetMovementModeCursor(pSoldier);
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleMovementMenu(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;

  // Get soldier
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  // Popup Menu
  if (pUIEvent->fFirstTime) {
    // Pop-up menu
    PopupMovementMenu(pUIEvent);

    // Change cusror to normal
    guiNewUICursor = NORMAL_FREEUICURSOR;
  }

  // Check for done flag
  if (pUIEvent->fDoneMenu) {
    PopDownMovementMenu();

    // Excecute command, if user hit a button
    if (pUIEvent->uiParams[1] == TRUE) {
      if (pUIEvent->uiParams[2] == MOVEMENT_MENU_LOOK) {
        guiPendingOverrideEvent = LC_CHANGE_TO_LOOK;
      } else if (pUIEvent->uiParams[2] == MOVEMENT_MENU_HAND) {
        guiPendingOverrideEvent = HC_ON_TERRAIN;
      } else if (pUIEvent->uiParams[2] == MOVEMENT_MENU_ACTIONC) {
        guiPendingOverrideEvent = M_CHANGE_TO_ACTION;
      } else if (pUIEvent->uiParams[2] == MOVEMENT_MENU_TALK) {
        guiPendingOverrideEvent = T_CHANGE_TO_TALKING;
      } else {
        // Change stance based on params!
        switch (pUIEvent->uiParams[0]) {
          case MOVEMENT_MENU_RUN:

            if (pSoldier->usUIMovementMode != WALKING && pSoldier->usUIMovementMode != RUNNING) {
              UIHandleSoldierStanceChange(pSoldier->ubID, ANIM_STAND);
              pSoldier->fUIMovementFast = 1;
            } else {
              pSoldier->fUIMovementFast = 1;
              pSoldier->usUIMovementMode = RUNNING;
              gfPlotNewMovement = TRUE;
            }
            break;

          case MOVEMENT_MENU_WALK:

            UIHandleSoldierStanceChange(pSoldier->ubID, ANIM_STAND);
            break;

          case MOVEMENT_MENU_SWAT:

            UIHandleSoldierStanceChange(pSoldier->ubID, ANIM_CROUCH);
            break;

          case MOVEMENT_MENU_PRONE:

            UIHandleSoldierStanceChange(pSoldier->ubID, ANIM_PRONE);
            break;
        }

        guiPendingOverrideEvent = A_CHANGE_TO_MOVE;

        // pSoldier->usUIMovementMode = (int8_t)pUIEvent->uiParams[ 0 ];
      }
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandlePositionMenu(UI_EVENT *pUIEvent) { return (GAME_SCREEN); }

uint32_t UIHandleAOnTerrain(UI_EVENT *pUIEvent) {
  int16_t usMapPos;
  struct SOLDIERTYPE *pSoldier;
  //	int16_t							sTargetXPos, sTargetYPos;

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (gpItemPointer != NULL) {
    return (GAME_SCREEN);
  }

  // Get soldier to determine range
  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    // ATE: Add stuff here to display a system message if we are targeting smeothing and
    //  are out of range.
    // Are we using a gun?
    if (GetActionModeCursor(pSoldier) == TARGETCURS) {
      SetActionModeDoorCursorText();

      // Yep, she's a gun.
      // Are we in range?
      if (!InRange(pSoldier, usMapPos)) {
        // Are we over a guy?
        if (gfUIFullTargetFound) {
          // No, ok display message IF this is the first time at this gridno
          if (gsOutOfRangeGridNo != MercPtrs[gusUIFullTargetID]->sGridNo ||
              gubOutOfRangeMerc != gusSelectedSoldier) {
            // Display
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[OUT_OF_RANGE_STRING]);

            // PlayJA2Sample( TARGET_OUT_OF_RANGE, RATE_11025, MIDVOLUME, 1, MIDDLEPAN );

            // Set
            gsOutOfRangeGridNo = MercPtrs[gusUIFullTargetID]->sGridNo;
            gubOutOfRangeMerc = (uint8_t)gusSelectedSoldier;
          }
        }
      }
    }

    guiNewUICursor = GetProperItemCursor((uint8_t)gusSelectedSoldier, pSoldier->inv[HANDPOS].usItem,
                                         usMapPos, FALSE);

    // Show UI ON GUY
    UIHandleOnMerc(FALSE);

    // If we are in realtime, and in a stationary animation, follow!
    if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
      if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY &&
          pSoldier->ubPendingAction == NO_PENDING_ACTION) {
        // Check if we have a shot waiting!
        if (gUITargetShotWaiting) {
          guiPendingOverrideEvent = CA_MERC_SHOOT;
        }

        if (!gUITargetReady) {
          // Move to proper stance + direction!
          // Convert our grid-not into an XY
          //	ConvertGridNoToXY( usMapPos, &sTargetXPos, &sTargetYPos );

          // Ready weapon
          //		SoldierReadyWeapon( pSoldier, sTargetXPos, sTargetYPos, FALSE );

          gUITargetReady = TRUE;
        }

      } else {
        gUITargetReady = FALSE;
      }
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleMChangeToAction(UI_EVENT *pUIEvent) {
  gUITargetShotWaiting = FALSE;

  EndPhysicsTrajectoryUI();

  // guiNewUICursor = CONFIRM_MOVE_UICURSOR;

  return (GAME_SCREEN);
}

uint32_t UIHandleMChangeToHandMode(UI_EVENT *pUIEvent) {
  ErasePath(FALSE);

  return (GAME_SCREEN);
}

uint32_t UIHandleAChangeToMove(UI_EVENT *pUIEvent) {
  // Set merc glow back to normal
  // ( could have been set when in target cursor )
  SetMercGlowNormal();

  // gsOutOfRangeGridNo = NOWHERE;

  gfPlotNewMovement = TRUE;

  return (GAME_SCREEN);
}

uint32_t UIHandleCWait(UI_EVENT *pUIEvent) {
  int16_t usMapPos;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiCursorFlags;
  struct LEVELNODE *pInvTile;

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    pInvTile = GetCurInteractiveTile();

    if (pInvTile && gpInvTileThatCausedMoveConfirm != pInvTile) {
      // Get out og this mode...
      guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
      return (GAME_SCREEN);
    }

    GetCursorMovementFlags(&uiCursorFlags);

    if (pInvTile != NULL) {
      HandleUIMovementCursor(pSoldier, uiCursorFlags, usMapPos, MOVEUI_TARGET_INTTILES);

      // Set UI CURSOR
      guiNewUICursor = GetInteractiveTileCursor(guiNewUICursor, TRUE);

      // Make red tile under spot... if we've previously found one...
      if (gfUIHandleShowMoveGrid) {
        gfUIHandleShowMoveGrid = 2;
      }

      return (GAME_SCREEN);
    }

    // Display action points
    gfUIDisplayActionPoints = TRUE;

    // Determine if we can afford!
    if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
      gfUIDisplayActionPointsInvalid = TRUE;
    }

    SetConfirmMovementModeCursor(pSoldier, FALSE);

    // If we are not in combat, draw path here!
    if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
      // DrawUIMovementPath( pSoldier, usMapPos,  0 );
      HandleUIMovementCursor(pSoldier, uiCursorFlags, usMapPos, 0);
    }
  }

  return (GAME_SCREEN);
}

// NOTE, ONCE AT THIS FUNCTION, WE HAVE ASSUMED TO HAVE CHECKED FOR ENOUGH APS THROUGH
// SelectedMercCanAffordMove
uint32_t UIHandleCMoveMerc(UI_EVENT *pUIEvent) {
  int16_t usMapPos;
  struct SOLDIERTYPE *pSoldier;
  int16_t sDestGridNo;
  int16_t sActionGridNo;
  struct STRUCTURE *pStructure;
  uint8_t ubDirection;
  BOOLEAN fAllMove;
  int8_t bLoop;
  struct LEVELNODE *pIntTile;
  int16_t sIntTileGridNo;
  BOOLEAN fOldFastMove;

  if (gusSelectedSoldier != NO_SOLDIER) {
    fAllMove = gfUIAllMoveOn;
    gfUIAllMoveOn = FALSE;

    if (!GetMouseMapPos(&usMapPos)) {
      return (GAME_SCREEN);
    }

    // ERASE PATH
    ErasePath(TRUE);

    if (fAllMove) {
      gfGetNewPathThroughPeople = TRUE;

      // Loop through all mercs and make go!
      // TODO: Only our squad!
      for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID, pSoldier = MercPtrs[bLoop];
           bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++, pSoldier++) {
        if (OK_CONTROLLABLE_MERC(pSoldier) && GetSolAssignment(pSoldier) == CurrentSquad() &&
            !pSoldier->fMercAsleep) {
          // If we can't be controlled, returninvalid...
          if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
            if (!CanRobotBeControlled(pSoldier)) {
              continue;
            }
          }

          AdjustNoAPToFinishMove(pSoldier, FALSE);

          fOldFastMove = pSoldier->fUIMovementFast;

          if (fAllMove == 2) {
            pSoldier->fUIMovementFast = TRUE;
            pSoldier->usUIMovementMode = RUNNING;
          } else {
            pSoldier->fUIMovementFast = FALSE;
            pSoldier->usUIMovementMode = GetMoveStateBasedOnStance(
                pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight);
          }

          // Remove any previous actions
          pSoldier->ubPendingAction = NO_PENDING_ACTION;

          // if ( !( gTacticalStatus.uiFlags & INCOMBAT ) && ( gAnimControl[ pSoldier->usAnimState
          // ].uiFlags & ANIM_MOVING ) )
          //{
          //	pSoldier->sRTPendingMovementGridNo = usMapPos;
          //	pSoldier->usRTPendingMovementAnim  = pSoldier->usUIMovementMode;
          //}
          // else
          if (EVENT_InternalGetNewSoldierPath(pSoldier, usMapPos, pSoldier->usUIMovementMode, TRUE,
                                              FALSE)) {
            InternalDoMercBattleSound(pSoldier, BATTLE_SOUND_OK1, BATTLE_SND_LOWER_VOLUME);
          } else {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[NO_PATH_FOR_MERC],
                      pSoldier->name);
          }

          pSoldier->fUIMovementFast = fOldFastMove;
        }
      }
      gfGetNewPathThroughPeople = FALSE;

      // RESET MOVE FAST FLAG
      SetConfirmMovementModeCursor(pSoldier, TRUE);

      gfUIAllMoveOn = 0;

    } else {
      // Get soldier
      if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
        // FOR REALTIME - DO MOVEMENT BASED ON STANCE!
        if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
          pSoldier->usUIMovementMode =
              GetMoveStateBasedOnStance(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight);
        }

        sDestGridNo = usMapPos;

        // Get structure info for in tile!
        pIntTile = GetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure);

        // We should not have null here if we are given this flag...
        if (pIntTile != NULL) {
          sActionGridNo =
              FindAdjacentGridEx(pSoldier, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE);
          if (sActionGridNo != -1) {
            SetUIBusy(pSoldier->ubID);

            // Set dest gridno
            sDestGridNo = sActionGridNo;

            // check if we are at this location
            if (pSoldier->sGridNo == sDestGridNo) {
              StartInteractiveObject(sIntTileGridNo, pStructure->usStructureID, pSoldier,
                                     ubDirection);
              InteractWithInteractiveObject(pSoldier, pStructure, ubDirection);
              return (GAME_SCREEN);
            }
          } else {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
            return (GAME_SCREEN);
          }
        }

        SetUIBusy(pSoldier->ubID);

        if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
          // RESET MOVE FAST FLAG
          SetConfirmMovementModeCursor(pSoldier, TRUE);

          if (!gTacticalStatus.fAtLeastOneGuyOnMultiSelect) {
            pSoldier->fUIMovementFast = FALSE;
          }

          // StartLooseCursor( usMapPos, 0 );
        }

        if (gTacticalStatus.fAtLeastOneGuyOnMultiSelect && pIntTile == NULL) {
          HandleMultiSelectionMove(sDestGridNo);
        } else {
          if (gUIUseReverse) {
            pSoldier->bReverse = TRUE;
          } else {
            pSoldier->bReverse = FALSE;
          }

          // Remove any previous actions
          pSoldier->ubPendingAction = NO_PENDING_ACTION;

          {
            EVENT_InternalGetNewSoldierPath(pSoldier, sDestGridNo, pSoldier->usUIMovementMode, TRUE,
                                            pSoldier->fNoAPToFinishMove);
          }

          if (pSoldier->usPathDataSize > 5) {
            DoMercBattleSound(pSoldier, BATTLE_SOUND_OK1);
          }

          // HANDLE ANY INTERACTIVE OBJECTS HERE!
          if (pIntTile != NULL) {
            StartInteractiveObject(sIntTileGridNo, pStructure->usStructureID, pSoldier,
                                   ubDirection);
          }
        }
      }
    }
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleMCycleMoveAll(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;

  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  if (gfUICanBeginAllMoveCycle) {
    gfUIAllMoveOn = TRUE;
    gfUICanBeginAllMoveCycle = FALSE;
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleMCycleMovement(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fGoodMode = FALSE;

  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  gfUIAllMoveOn = FALSE;

  if (pSoldier->ubBodyType == ROBOTNOWEAPON) {
    pSoldier->usUIMovementMode = WALKING;
    gfPlotNewMovement = TRUE;
    return (GAME_SCREEN);
  }

  do {
    // Cycle gmovement state
    if (pSoldier->usUIMovementMode == RUNNING) {
      pSoldier->usUIMovementMode = WALKING;
      if (IsValidMovementMode(pSoldier, WALKING)) {
        fGoodMode = TRUE;
      }
    } else if (pSoldier->usUIMovementMode == WALKING) {
      pSoldier->usUIMovementMode = SWATTING;
      if (IsValidMovementMode(pSoldier, SWATTING)) {
        fGoodMode = TRUE;
      }
    } else if (pSoldier->usUIMovementMode == SWATTING) {
      pSoldier->usUIMovementMode = CRAWLING;
      if (IsValidMovementMode(pSoldier, CRAWLING)) {
        fGoodMode = TRUE;
      }
    } else if (pSoldier->usUIMovementMode == CRAWLING) {
      pSoldier->fUIMovementFast = 1;
      pSoldier->usUIMovementMode = RUNNING;
      if (IsValidMovementMode(pSoldier, RUNNING)) {
        fGoodMode = TRUE;
      }
    }

  } while (fGoodMode != TRUE);

  gfPlotNewMovement = TRUE;

  return (GAME_SCREEN);
}

uint32_t UIHandleCOnTerrain(UI_EVENT *pUIEvent) { return (GAME_SCREEN); }

uint32_t UIHandleMAdjustStanceMode(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int32_t iPosDiff;
  static uint16_t gusAnchorMouseY;
  static uint16_t usOldMouseY;
  static BOOLEAN ubNearHeigherLevel;
  static BOOLEAN ubNearLowerLevel;
  static uint8_t ubUpHeight, ubDownDepth;
  static uint32_t uiOldShowUPDownArrows;
  int8_t bNewDirection;

  // Change cusror to normal
  guiNewUICursor = NO_UICURSOR;

  if (pUIEvent->fFirstTime) {
    gusAnchorMouseY = gusMouseYPos;
    usOldMouseY = gusMouseYPos;
    ubNearHeigherLevel = FALSE;
    ubNearLowerLevel = FALSE;

    guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
    uiOldShowUPDownArrows = guiShowUPDownArrows;

    gbAdjustStanceDiff = 0;
    gbClimbID = 0;

    gfIgnoreScrolling = TRUE;

    // Get soldier current height of animation
    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
      // IF we are on a basic level...(temp)
      if (pSoldier->bLevel == 0) {
        if (FindHeigherLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection)) {
          ubNearHeigherLevel = TRUE;
        }
      }

      // IF we are higher...
      if (pSoldier->bLevel > 0) {
        if (FindLowerLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection)) {
          ubNearLowerLevel = TRUE;
        }
      }

      switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
        case ANIM_STAND:
          if (ubNearHeigherLevel) {
            ubUpHeight = 1;
            ubDownDepth = 2;
          } else if (ubNearLowerLevel) {
            ubUpHeight = 0;
            ubDownDepth = 3;
          } else {
            ubUpHeight = 0;
            ubDownDepth = 2;
          }
          break;

        case ANIM_CROUCH:
          if (ubNearHeigherLevel) {
            ubUpHeight = 2;
            ubDownDepth = 1;
          } else if (ubNearLowerLevel) {
            ubUpHeight = 1;
            ubDownDepth = 2;
          } else {
            ubUpHeight = 1;
            ubDownDepth = 1;
          }
          break;

        case ANIM_PRONE:
          if (ubNearHeigherLevel) {
            ubUpHeight = 3;
            ubDownDepth = 0;
          } else if (ubNearLowerLevel) {
            ubUpHeight = 2;
            ubDownDepth = 1;
          } else {
            ubUpHeight = 2;
            ubDownDepth = 0;
          }
          break;
      }
    }
  }

  // Check if delta X has changed alot since last time
  iPosDiff = abs((int32_t)(usOldMouseY - gusMouseYPos));

  // guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
  guiShowUPDownArrows = uiOldShowUPDownArrows;

  {
    if (gusAnchorMouseY > gusMouseYPos) {
      // Get soldier
      if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
        if (iPosDiff < GO_MOVE_ONE && ubUpHeight >= 1) {
          // Change arrows to move down arrow + show
          // guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_Y;
          guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
          gbAdjustStanceDiff = 0;
          gbClimbID = 0;
        } else if (iPosDiff > GO_MOVE_ONE && iPosDiff < GO_MOVE_TWO && ubUpHeight >= 1) {
          // guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_G;
          if (ubUpHeight == 1 && ubNearHeigherLevel) {
            guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_CLIMB;
            gbClimbID = 1;
          } else {
            guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_Y;
            gbClimbID = 0;
          }
          gbAdjustStanceDiff = 1;
        } else if (iPosDiff >= GO_MOVE_TWO && iPosDiff < GO_MOVE_THREE && ubUpHeight >= 2) {
          if (ubUpHeight == 2 && ubNearHeigherLevel) {
            guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_CLIMB;
            gbClimbID = 1;
          } else {
            guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_YY;
            gbClimbID = 0;
          }
          gbAdjustStanceDiff = 2;
        } else if (iPosDiff >= GO_MOVE_THREE && ubUpHeight >= 3) {
          if (ubUpHeight == 3 && ubNearHeigherLevel) {
            guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_CLIMB;
            gbClimbID = 1;
          }
        }
      }
    }

    if (gusAnchorMouseY < gusMouseYPos) {
      // Get soldier
      if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
        if (iPosDiff < GO_MOVE_ONE && ubDownDepth >= 1) {
          // Change arrows to move down arrow + show
          // guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_Y;
          guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
          gbAdjustStanceDiff = 0;
          gbClimbID = 0;

        } else if (iPosDiff >= GO_MOVE_ONE && iPosDiff < GO_MOVE_TWO && ubDownDepth >= 1) {
          //						guiShowUPDownArrows =
          // ARROWS_SHOW_DOWN_BELOW_G;
          if (ubDownDepth == 1 && ubNearLowerLevel) {
            guiShowUPDownArrows = ARROWS_SHOW_DOWN_CLIMB;
            gbClimbID = -1;
          } else {
            guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_Y;
            gbClimbID = 0;
          }
          gbAdjustStanceDiff = -1;
        } else if (iPosDiff > GO_MOVE_TWO && iPosDiff < GO_MOVE_THREE && ubDownDepth >= 2) {
          // guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_GG;
          if (ubDownDepth == 2 && ubNearLowerLevel) {
            guiShowUPDownArrows = ARROWS_SHOW_DOWN_CLIMB;
            gbClimbID = -1;
          } else {
            guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_YY;
            gbClimbID = 0;
          }
          gbAdjustStanceDiff = -2;
        } else if (iPosDiff > GO_MOVE_THREE && ubDownDepth >= 3) {
          // guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_GG;
          if (ubDownDepth == 3 && ubNearLowerLevel) {
            guiShowUPDownArrows = ARROWS_SHOW_DOWN_CLIMB;
            gbClimbID = -1;
          }
        }
      }
    }
  }

  uiOldShowUPDownArrows = guiShowUPDownArrows;

  return (GAME_SCREEN);
}

uint32_t UIHandleAChangeToConfirmAction(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    HandleLeftClickCursor(pSoldier);
  }

  ResetBurstLocations();

  return (GAME_SCREEN);
}

uint32_t UIHandleCAOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    guiNewUICursor = GetProperItemCursor((uint8_t)gusSelectedSoldier, pSoldier->inv[HANDPOS].usItem,
                                         usMapPos, TRUE);

    UIHandleOnMerc(FALSE);
  }

  return (GAME_SCREEN);
}

void UIHandleMercAttack(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTargetSoldier,
                        uint16_t usMapPos) {
  int32_t iHandleReturn;
  int16_t sTargetGridNo;
  int8_t bTargetLevel;
  struct LEVELNODE *pIntNode;
  struct STRUCTURE *pStructure;
  int16_t sGridNo, sNewGridNo;
  uint8_t ubItemCursor;

  // get cursor
  ubItemCursor = GetActionModeCursor(pSoldier);

  if (!(gTacticalStatus.uiFlags & INCOMBAT) && pTargetSoldier &&
      Item[pSoldier->inv[HANDPOS].usItem].usItemClass & IC_WEAPON) {
    if (NPCFirstDraw(pSoldier, pTargetSoldier)) {
      // go into turnbased for that person
      CancelAIAction(pTargetSoldier, TRUE);
      AddToShouldBecomeHostileOrSayQuoteList(pTargetSoldier->ubID);
      // MakeCivHostile( pTargetSoldier, 2 );
      // TriggerNPCWithIHateYouQuote( pTargetSoldier->ubProfile );
      return;
    }
  }

  // Set aim time to one in UI
  pSoldier->bAimTime = (pSoldier->bShownAimTime / 2);

  // ATE: Check if we are targeting an interactive tile, and adjust gridno accordingly...
  pIntNode = GetCurInteractiveTileGridNoAndStructure(&sGridNo, &pStructure);

  if (pTargetSoldier != NULL) {
    sTargetGridNo = pTargetSoldier->sGridNo;
    bTargetLevel = pTargetSoldier->bLevel;
  } else {
    sTargetGridNo = usMapPos;
    bTargetLevel = (int8_t)gsInterfaceLevel;

    if (pIntNode != NULL) {
      // Change gridno....
      sTargetGridNo = sGridNo;
    }
  }

  // here, change gridno if we're targeting ourselves....
  if (pIntNode != NULL) {
    // Are we in the same gridno?
    if (sGridNo == pSoldier->sGridNo && ubItemCursor != AIDCURS) {
      // Get orientation....
      switch (pStructure->ubWallOrientation) {
        case OUTSIDE_TOP_LEFT:
        case INSIDE_TOP_LEFT:

          sNewGridNo = NewGridNo(sGridNo, DirectionInc(SOUTH));
          break;

        case OUTSIDE_TOP_RIGHT:
        case INSIDE_TOP_RIGHT:

          sNewGridNo = NewGridNo(sGridNo, DirectionInc(EAST));
          break;

        default:
          sNewGridNo = sGridNo;
      }

      // Set target gridno to this one...
      sTargetGridNo = sNewGridNo;

      // now set target cube height
      // CJC says to hardcode this value :)
      pSoldier->bTargetCubeLevel = 2;
    } else {
      // ATE: Refine this a bit - if we have nobody as a target...
      if (pTargetSoldier == NULL) {
        sTargetGridNo = sGridNo;
      }
    }
  }

  // Cannot be fire if we are already in a fire animation....
  // this is to stop the shooting trigger/happy duded from contiously pressing fire...
  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIRE) {
      return;
    }
  }

  // If in turn-based mode - return to movement
  if ((gTacticalStatus.uiFlags & INCOMBAT)) {
    // Reset some flags for cont move...
    pSoldier->sFinalDestination = pSoldier->sGridNo;
    pSoldier->bGoodContPath = FALSE;
    //  guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
  }

  if (pSoldier->bWeaponMode == WM_ATTACHED) {
    iHandleReturn = HandleItem(pSoldier, sTargetGridNo, bTargetLevel, UNDER_GLAUNCHER, TRUE);
  } else {
    iHandleReturn =
        HandleItem(pSoldier, sTargetGridNo, bTargetLevel, pSoldier->inv[HANDPOS].usItem, TRUE);
  }

  if (iHandleReturn < 0) {
    if (iHandleReturn == ITEM_HANDLE_RELOADING) {
      guiNewUICursor = ACTION_TARGET_RELOADING;
      return;
    }

    if (iHandleReturn == ITEM_HANDLE_NOROOM) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, pMessageStrings[MSG_CANT_FIRE_HERE]);
      return;
    }
  }

  if (gTacticalStatus.uiFlags & TURNBASED && !(gTacticalStatus.uiFlags & INCOMBAT)) {
    HandleUICursorRTFeedback(pSoldier);
  }

  gfUIForceReExamineCursorData = TRUE;
}

void AttackRequesterCallback(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_YES) {
    gTacticalStatus.ubLastRequesterTargetID = gpRequesterTargetMerc->ubProfile;

    UIHandleMercAttack(gpRequesterMerc, gpRequesterTargetMerc, gsRequesterGridNo);
  }
}

uint32_t UIHandleCAMercShoot(UI_EVENT *pUIEvent) {
  int16_t usMapPos;
  struct SOLDIERTYPE *pSoldier, *pTSoldier = NULL;
  BOOLEAN fDidRequester = FALSE;

  if (gusSelectedSoldier != NO_SOLDIER) {
    if (!GetMouseMapPos(&usMapPos)) {
      return (GAME_SCREEN);
    }

    // Get soldier
    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
      // Get target guy...
      if (gfUIFullTargetFound) {
        // Get target soldier, if one exists
        pTSoldier = MercPtrs[gusUIFullTargetID];
      }

      if (pTSoldier != NULL) {
        // If this is one of our own guys.....pop up requiester...
        if ((pTSoldier->bTeam == gbPlayerNum || pTSoldier->bTeam == MILITIA_TEAM) &&
            Item[pSoldier->inv[HANDPOS].usItem].usItemClass != IC_MEDKIT &&
            pSoldier->inv[HANDPOS].usItem != GAS_CAN &&
            gTacticalStatus.ubLastRequesterTargetID != pTSoldier->ubProfile &&
            (pTSoldier->ubID != GetSolID(pSoldier))) {
          wchar_t zStr[200];

          gpRequesterMerc = pSoldier;
          gpRequesterTargetMerc = pTSoldier;
          gsRequesterGridNo = usMapPos;

          fDidRequester = TRUE;

          swprintf(zStr, ARR_SIZE(zStr), TacticalStr[ATTACK_OWN_GUY_PROMPT], pTSoldier->name);

          DoMessageBox(MSG_BOX_BASIC_STYLE, zStr, GAME_SCREEN, (uint8_t)MSG_BOX_FLAG_YESNO,
                       AttackRequesterCallback, NULL);
        }
      }

      if (!fDidRequester) {
        UIHandleMercAttack(pSoldier, pTSoldier, usMapPos);
      }
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleAEndAction(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t sTargetXPos, sTargetYPos;
  int16_t usMapPos;

  // Get gridno at this location
  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
      if (gUITargetReady) {
        // Move to proper stance + direction!
        // Convert our grid-not into an XY
        ConvertGridNoToXY(usMapPos, &sTargetXPos, &sTargetYPos);

        // UNReady weapon
        SoldierReadyWeapon(pSoldier, sTargetXPos, sTargetYPos, TRUE);

        gUITargetReady = FALSE;
      }
    }
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleCAEndConfirmAction(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    HandleEndConfirmCursor(pSoldier);
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleIOnTerrain(UI_EVENT *pUIEvent) {
  int16_t usMapPos;

  // Get gridno at this location
  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (!UIHandleOnMerc(TRUE)) {
    // Check if dest is OK
    // if ( !NewOKDestination( usMapPos, FALSE ) || IsRoofVisible( usMapPos ) )
    ////{
    //	guiNewUICursor = CANNOT_MOVE_UICURSOR;
    //}
    // else
    { guiNewUICursor = NORMAL_SNAPUICURSOR; }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleIChangeToIdle(UI_EVENT *pUIEvent) { return (GAME_SCREEN); }

uint32_t UIHandlePADJAdjustStance(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  uint8_t ubNewStance;

  guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

  gfIgnoreScrolling = FALSE;

  if (gusSelectedSoldier != NO_SOLDIER && gbAdjustStanceDiff != 0) {
    // Get soldier
    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
      ubNewStance = GetAdjustedAnimHeight(gAnimControl[pSoldier->usAnimState].ubEndHeight,
                                          gbAdjustStanceDiff);

      if (gbClimbID == 1) {
        BeginSoldierClimbUpRoof(pSoldier);
      } else if (gbClimbID == -1) {
        BeginSoldierClimbDownRoof(pSoldier);
      } else {
        // Set state to result
        UIHandleSoldierStanceChange(pSoldier->ubID, ubNewStance);
      }

      // Once we have APs, we can safely reset nomove flag!
      // AdjustNoAPToFinishMove( pSoldier, FALSE );
    }
  }
  return (GAME_SCREEN);
}

uint8_t GetAdjustedAnimHeight(uint8_t ubAnimHeight, int8_t bChange) {
  uint8_t ubNewAnimHeight = ubAnimHeight;

  if (ubAnimHeight == ANIM_STAND) {
    if (bChange == -1) {
      ubNewAnimHeight = ANIM_CROUCH;
    }
    if (bChange == -2) {
      ubNewAnimHeight = ANIM_PRONE;
    }
    if (bChange == 1) {
      ubNewAnimHeight = 50;
    }
  } else if (ubAnimHeight == ANIM_CROUCH) {
    if (bChange == 1) {
      ubNewAnimHeight = ANIM_STAND;
    }
    if (bChange == -1) {
      ubNewAnimHeight = ANIM_PRONE;
    }
    if (bChange == -2) {
      ubNewAnimHeight = 55;
    }
  } else if (ubAnimHeight == ANIM_PRONE) {
    if (bChange == -1) {
      ubNewAnimHeight = 55;
    }
    if (bChange == 1) {
      ubNewAnimHeight = ANIM_CROUCH;
    }
    if (bChange == 2) {
      ubNewAnimHeight = ANIM_STAND;
    }
  }

  return (ubNewAnimHeight);
}

void HandleObjectHighlighting() {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;

  if (!GetMouseMapPos(&usMapPos)) {
    return;
  }

  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (gfUIFullTargetFound) {
    // Get Soldier
    GetSoldier(&pSoldier, gusUIFullTargetID);

    // If an enemy, and in a given mode, highlight
    if (guiUIFullTargetFlags & ENEMY_MERC) {
    } else if (guiUIFullTargetFlags & OWNED_MERC) {
      // Check for selected
      pSoldier->pCurrentShade = pSoldier->pShades[0];
      gfUIDoNotHighlightSelMerc = TRUE;
    }
  }
}

void AdjustSoldierCreationStartValues() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  guiCreateGuyIndex = (int16_t)cnt;

  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       pSoldier++, cnt++) {
    if (!IsSolActive(pSoldier)) {
      guiCreateGuyIndex = (int16_t)cnt;
      break;
    }
  }

  cnt = gTacticalStatus.Team[gbPlayerNum].bLastID + 1;
  guiCreateBadGuyIndex = (int16_t)cnt;

  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[LAST_TEAM].bLastID;
       pSoldier++, cnt++) {
    if (!IsSolActive(pSoldier) && cnt > gTacticalStatus.Team[gbPlayerNum].bLastID) {
      guiCreateBadGuyIndex = (int16_t)cnt;
      break;
    }
  }
}

BOOLEAN SelectedMercCanAffordAttack() {
  struct SOLDIERTYPE *pSoldier;
  struct SOLDIERTYPE *pTargetSoldier;
  int16_t usMapPos;
  int16_t sTargetGridNo;
  int16_t sAPCost;
  uint8_t ubItemCursor;

  if (gusSelectedSoldier != NO_SOLDIER) {
    if (!GetMouseMapPos(&usMapPos)) {
      return (GAME_SCREEN);
    }

    // Get soldier
    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
      // Get cursor value
      ubItemCursor = GetActionModeCursor(pSoldier);

      if (ubItemCursor == INVALIDCURS) {
        return (FALSE);
      }

      if (ubItemCursor == BOMBCURS) {
        // Check as...
        if (EnoughPoints(pSoldier, GetTotalAPsToDropBomb(pSoldier, usMapPos), 0, TRUE)) {
          return (TRUE);
        }
      } else if (ubItemCursor == REMOTECURS) {
        // Check as...
        if (EnoughPoints(pSoldier, GetAPsToUseRemote(pSoldier), 0, TRUE)) {
          return (TRUE);
        }
      } else {
        // Look for a soldier at this position
        if (gfUIFullTargetFound) {
          // GetSoldier
          GetSoldier(&pTargetSoldier, gusUIFullTargetID);
          sTargetGridNo = pTargetSoldier->sGridNo;
        } else {
          sTargetGridNo = usMapPos;
        }

        sAPCost = CalcTotalAPsToAttack(pSoldier, sTargetGridNo, TRUE,
                                       (int8_t)(pSoldier->bShownAimTime / 2));

        if (EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
          return (TRUE);
        } else {
          // Play curse....
          DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN SelectedMercCanAffordMove() {
  struct SOLDIERTYPE *pSoldier;
  uint16_t sAPCost = 0;
  int16_t usMapPos;
  struct LEVELNODE *pIntTile;

  // Get soldier
  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    if (!GetMouseMapPos(&usMapPos)) {
      return (GAME_SCREEN);
    }

    // IF WE ARE OVER AN INTERACTIVE TILE, GIVE GRIDNO OF POSITION
    pIntTile = GetCurInteractiveTile();

    if (pIntTile != NULL) {
      // CHECK APS
      if (EnoughPoints(pSoldier, gsCurrentActionPoints, 0, TRUE)) {
        return (TRUE);
      } else {
        return (FALSE);
      }
    }

    // Take the first direction!
    sAPCost = PtsToMoveDirection(pSoldier, (uint8_t)guiPathingData[0]);

    sAPCost += GetAPsToChangeStance(pSoldier, gAnimControl[pSoldier->usUIMovementMode].ubHeight);

    if (EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
      return (TRUE);
    } else {
      // OK, remember where we were trying to get to.....
      pSoldier->sContPathLocation = usMapPos;
      pSoldier->bGoodContPath = TRUE;
    }
  }

  return (FALSE);
}

void GetMercClimbDirection(uint8_t ubSoldierID, BOOLEAN *pfGoDown, BOOLEAN *pfGoUp) {
  int8_t bNewDirection;
  struct SOLDIERTYPE *pSoldier;

  *pfGoDown = FALSE;
  *pfGoUp = FALSE;

  if (!GetSoldier(&pSoldier, ubSoldierID)) {
    return;
  }

  // Check if we are close / can climb
  if (pSoldier->bLevel == 0) {
    // See if we are not in a building!
    if (FindHeigherLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection)) {
      *pfGoUp = TRUE;
    }
  }

  // IF we are higher...
  if (pSoldier->bLevel > 0) {
    if (FindLowerLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection)) {
      *pfGoDown = TRUE;
    }
  }
}

void RemoveTacticalCursor() {
  guiNewUICursor = NO_UICURSOR;
  ErasePath(TRUE);
}

uint32_t UIHandlePOPUPMSG(UI_EVENT *pUIEvent) { return (GAME_SCREEN); }

uint32_t UIHandleHCOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  // If we are out of breath, no cursor...
  if (pSoldier->bBreath < OKBREATH && pSoldier->bCollapsed) {
    guiNewUICursor = INVALID_ACTION_UICURSOR;
  } else {
    if (gsOverItemsGridNo != NOWHERE &&
        (usMapPos != gsOverItemsGridNo || gsInterfaceLevel != gsOverItemsLevel)) {
      gsOverItemsGridNo = NOWHERE;
      guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
    } else {
      guiNewUICursor = NORMALHANDCURSOR_UICURSOR;

      UIHandleInteractiveTilesAndItemsOnTerrain(pSoldier, usMapPos, TRUE, FALSE);
    }
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleHCGettingItem(UI_EVENT *pUIEvent) {
  guiNewUICursor = NORMAL_FREEUICURSOR;

  return (GAME_SCREEN);
}

uint32_t UIHandleTATalkingMenu(UI_EVENT *pUIEvent) {
  guiNewUICursor = NORMAL_FREEUICURSOR;

  return (GAME_SCREEN);
}

uint32_t UIHandleEXExitSectorMenu(UI_EVENT *pUIEvent) {
  guiNewUICursor = NORMAL_FREEUICURSOR;

  return (GAME_SCREEN);
}

uint32_t UIHandleOpenDoorMenu(UI_EVENT *pUIEvent) {
  guiNewUICursor = NORMAL_FREEUICURSOR;

  return (GAME_SCREEN);
}

void ToggleHandCursorMode(uint32_t *puiNewEvent) {
  // Toggle modes
  if (gCurrentUIMode == HANDCURSOR_MODE) {
    *puiNewEvent = A_CHANGE_TO_MOVE;
  } else {
    *puiNewEvent = M_CHANGE_TO_HANDMODE;
  }
}

void ToggleTalkCursorMode(uint32_t *puiNewEvent) {
  // Toggle modes
  if (gCurrentUIMode == TALKCURSOR_MODE) {
    *puiNewEvent = A_CHANGE_TO_MOVE;
  } else {
    *puiNewEvent = T_CHANGE_TO_TALKING;
  }
}

void ToggleLookCursorMode(uint32_t *puiNewEvent) {
  // Toggle modes
  if (gCurrentUIMode == LOOKCURSOR_MODE) {
    guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
    HandleTacticalUI();
  } else {
    guiPendingOverrideEvent = LC_CHANGE_TO_LOOK;
    HandleTacticalUI();
  }
}

BOOLEAN UIHandleOnMerc(BOOLEAN fMovementMode) {
  struct SOLDIERTYPE *pSoldier;
  uint16_t usSoldierIndex;
  uint32_t uiMercFlags;
  int16_t usMapPos;
  BOOLEAN fFoundMerc = FALSE;

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  // if ( fMovementMode )
  //{
  //	fFoundMerc			= gfUISelectiveTargetFound;
  //	usSoldierIndex	= gusUISelectiveTargetID;
  //	uiMercFlags			= guiUISelectiveTargetFlags;
  //}
  // else
  {
    fFoundMerc = gfUIFullTargetFound;
    usSoldierIndex = gusUIFullTargetID;
    uiMercFlags = guiUIFullTargetFlags;
  }

  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (fFoundMerc) {
    // Get Soldier
    GetSoldier(&pSoldier, usSoldierIndex);

    if (uiMercFlags & OWNED_MERC) {
      // ATE: Check if this is an empty vehicle.....
      // if ( OK_ENTERABLE_VEHICLE( pSoldier ) && GetNumberInVehicle( pSoldier->bVehicleID ) == 0 )
      //{
      //	return( FALSE );
      //}

      // If not unconscious, select
      if (!(uiMercFlags & UNCONSCIOUS_MERC)) {
        if (fMovementMode) {
          // ERASE PATH
          ErasePath(TRUE);

          // Show cursor with highlight on selected merc
          guiNewUICursor = NO_UICURSOR;

          // IF selected, do selection one
          if ((uiMercFlags & SELECTED_MERC)) {
            // Add highlight to guy in interface.c
            gfUIHandleSelection = SELECTED_GUY_SELECTION;

            if (gpItemPointer == NULL) {
              // Don't do this unless we want to

              // Check if buddy is stationary!
              if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY ||
                  pSoldier->fNoAPToFinishMove) {
                guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
              }
            }

          } else {
            // if ( ( uiMercFlags & ONDUTY_MERC ) && !( uiMercFlags & NOINTERRUPT_MERC ) )
            if (!(uiMercFlags & NOINTERRUPT_MERC)) {
              // Add highlight to guy in interface.c
              gfUIHandleSelection = NONSELECTED_GUY_SELECTION;
            } else {
              gfUIHandleSelection = ENEMY_GUY_SELECTION;
              gfUIHandleSelectionAboveGuy = TRUE;
            }
          }
        }
      }

      // If not dead, show above guy!
      if (!(uiMercFlags & DEAD_MERC)) {
        if (fMovementMode) {
          // ERASE PATH
          ErasePath(TRUE);

          // Show cursor with highlight on selected merc
          guiNewUICursor = NO_UICURSOR;

          gsSelectedGridNo = pSoldier->sGridNo;
          gsSelectedLevel = pSoldier->bLevel;
        }

        gsSelectedGuy = usSoldierIndex;
        gfUIHandleSelectionAboveGuy = TRUE;
      }

    } else if (((uiMercFlags & ENEMY_MERC) || (uiMercFlags & NEUTRAL_MERC)) &&
               (uiMercFlags & VISIBLE_MERC)) {
      // ATE: If we are a vehicle, let the mouse cursor be a wheel...
      if ((OK_ENTERABLE_VEHICLE(pSoldier))) {
        return (FALSE);
      } else {
        if (fMovementMode) {
          // Check if this guy is on the enemy team....
          if (!pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum)) {
            gUIActionModeChangeDueToMouseOver = TRUE;

            guiPendingOverrideEvent = M_CHANGE_TO_ACTION;
            // Return FALSE
            return (FALSE);
          } else {
            // ERASE PATH
            ErasePath(TRUE);

            // Show cursor with highlight on selected merc
            guiNewUICursor = NO_UICURSOR;
            // Show cursor with highlight
            gfUIHandleSelection = ENEMY_GUY_SELECTION;
            gsSelectedGridNo = pSoldier->sGridNo;
            gsSelectedLevel = pSoldier->bLevel;
          }
        }

        gfUIHandleSelectionAboveGuy = TRUE;
        gsSelectedGuy = usSoldierIndex;
      }
    } else {
      if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
        return (FALSE);
      }
    }
  } else {
    gfIgnoreOnSelectedGuy = FALSE;

    return (FALSE);
  }

  return (TRUE);
}

uint32_t UIHandleILoadLevel(UI_EVENT *pUIEvent) { return (INIT_SCREEN); }

uint32_t UIHandleISoldierDebug(UI_EVENT *pUIEvent) {
  // Use soldier display pages
  SetDebugRenderHook((RENDER_HOOK)DebugSoldierPage1, 0);
  SetDebugRenderHook((RENDER_HOOK)DebugSoldierPage2, 1);
  SetDebugRenderHook((RENDER_HOOK)DebugSoldierPage3, 2);
  SetDebugRenderHook((RENDER_HOOK)DebugSoldierPage4, 3);
  gCurDebugPage = 1;

  return (DEBUG_SCREEN);
}

uint32_t UIHandleILOSDebug(UI_EVENT *pUIEvent) {
  SetDebugRenderHook((RENDER_HOOK)DebugStructurePage1, 0);
  return (DEBUG_SCREEN);
}

uint32_t UIHandleILevelNodeDebug(UI_EVENT *pUIEvent) {
  SetDebugRenderHook((RENDER_HOOK)DebugLevelNodePage, 0);
  return (DEBUG_SCREEN);
}

uint32_t UIHandleIETOnTerrain(UI_EVENT *pUIEvent) {
  // guiNewUICursor = CANNOT_MOVE_UICURSOR;
  guiNewUICursor = NO_UICURSOR;

  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  return (GAME_SCREEN);
}

void UIHandleSoldierStanceChange(uint8_t ubSoldierID, int8_t bNewStance) {
  struct SOLDIERTYPE *pSoldier;

  pSoldier = MercPtrs[ubSoldierID];

  // Is this a valid stance for our position?
  if (!IsValidStance(pSoldier, bNewStance)) {
    if (pSoldier->bCollapsed && pSoldier->bBreath < OKBREATH) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[4], pSoldier->name);
    } else {
      if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                  TacticalStr[VEHICLES_NO_STANCE_CHANGE_STR]);
      } else if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ROBOT_NO_STANCE_CHANGE_STR]);
      } else {
        if (pSoldier->bCollapsed) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, pMessageStrings[MSG_CANT_CHANGE_STANCE],
                    pSoldier->name);
        } else {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[CANNOT_STANCE_CHANGE_STR],
                    pSoldier->name);
        }
      }
    }
    return;
  }

  // IF turn-based - adjust stance now!
  if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
    pSoldier->fTurningFromPronePosition = FALSE;

    // Check if we have enough APS
    if (SoldierCanAffordNewStance(pSoldier, bNewStance)) {
      // Adjust stance
      // ChangeSoldierStance( pSoldier, bNewStance );
      SendChangeSoldierStanceEvent(pSoldier, bNewStance);

      pSoldier->sFinalDestination = pSoldier->sGridNo;
      pSoldier->bGoodContPath = FALSE;

    } else
      return;
  }

  // If realtime- change walking animation!
  if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    // If we are stationary, do something else!
    if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY) {
      // Change stance normally
      SendChangeSoldierStanceEvent(pSoldier, bNewStance);

    } else {
      // Pick moving animation based on stance

      // LOCK VARIBLE FOR NO UPDATE INDEX...
      pSoldier->usUIMovementMode = GetMoveStateBasedOnStance(pSoldier, bNewStance);

      if (pSoldier->usUIMovementMode == CRAWLING &&
          gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_PRONE) {
        pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = LOCKED_NO_NEWGRIDNO;
        pSoldier->bPathStored = FALSE;
      } else {
        pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = 1;
      }

      ChangeSoldierState(pSoldier, pSoldier->usUIMovementMode, 0, FALSE);
    }
  }

  // Set UI value for soldier
  SetUIbasedOnStance(pSoldier, bNewStance);

  gfUIStanceDifferent = TRUE;

  // ATE: If we are being serviced...stop...
  // InternalReceivingSoldierCancelServices( pSoldier, FALSE );
  InternalGivingSoldierCancelServices(pSoldier, FALSE);
  // gfPlotNewMovement   = TRUE;
}

uint32_t UIHandleIETEndTurn(UI_EVENT *pUIEvent) { return (GAME_SCREEN); }

uint32_t UIHandleIGotoDemoMode(UI_EVENT *pUIEvent) { return (EnterTacticalDemoMode()); }

uint32_t UIHandleILoadFirstLevel(UI_EVENT *pUIEvent) {
  gubCurrentScene = 0;
  return (INIT_SCREEN);
}

uint32_t UIHandleILoadSecondLevel(UI_EVENT *pUIEvent) {
  gubCurrentScene = 1;
  return (INIT_SCREEN);
}

uint32_t UIHandleILoadThirdLevel(UI_EVENT *pUIEvent) {
  gubCurrentScene = 2;
  return (INIT_SCREEN);
}

uint32_t UIHandleILoadFourthLevel(UI_EVENT *pUIEvent) {
  gubCurrentScene = 3;
  return (INIT_SCREEN);
}

uint32_t UIHandleILoadFifthLevel(UI_EVENT *pUIEvent) {
  gubCurrentScene = 4;
  return (INIT_SCREEN);
}

void GetCursorMovementFlags(uint32_t *puiCursorFlags) {
  int16_t usMapPos;
  int16_t sXPos, sYPos;

  static BOOLEAN fStationary = FALSE;
  static uint16_t usOldMouseXPos = 32000;
  static uint16_t usOldMouseYPos = 32000;
  static uint16_t usOldMapPos = 32000;

  static uint32_t uiSameFrameCursorFlags;
  static uint32_t uiOldFrameNumber = 99999;

  // Check if this is the same frame as before, return already calculated value if so!
  if (uiOldFrameNumber == guiGameCycleCounter) {
    (*puiCursorFlags) = uiSameFrameCursorFlags;
    return;
  }

  GetMouseMapPos(&usMapPos);
  ConvertGridNoToXY(usMapPos, &sXPos, &sYPos);

  *puiCursorFlags = 0;

  if (gusMouseXPos != usOldMouseXPos || gusMouseYPos != usOldMouseYPos) {
    (*puiCursorFlags) |= MOUSE_MOVING;

    // IF CURSOR WAS PREVIOUSLY STATIONARY, MAKE THE ADDITIONAL CHECK OF GRID POS CHANGE
    if (fStationary && usOldMapPos == usMapPos) {
      (*puiCursorFlags) |= MOUSE_MOVING_IN_TILE;
    } else {
      fStationary = FALSE;
      (*puiCursorFlags) |= MOUSE_MOVING_NEW_TILE;
    }

  } else {
    (*puiCursorFlags) |= MOUSE_STATIONARY;
    fStationary = TRUE;
  }

  usOldMapPos = usMapPos;
  usOldMouseXPos = gusMouseXPos;
  usOldMouseYPos = gusMouseYPos;

  uiOldFrameNumber = guiGameCycleCounter;
  uiSameFrameCursorFlags = (*puiCursorFlags);
}

BOOLEAN HandleUIMovementCursor(struct SOLDIERTYPE *pSoldier, uint32_t uiCursorFlags,
                               uint16_t usMapPos, uint32_t uiFlags) {
  BOOLEAN fSetCursor = FALSE;
  static uint16_t usTargetID = NOBODY;
  static BOOLEAN fTargetFound = FALSE;
  BOOLEAN fTargetFoundAndLookingForOne = FALSE;

  // Determine if we can afford!
  if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
    gfUIDisplayActionPointsInvalid = TRUE;
  }

  // Check if we're stationary
  if (((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) ||
      ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY) ||
       pSoldier->fNoAPToFinishMove) ||
      GetSolID(pSoldier) >= MAX_NUM_SOLDIERS) {
    // If we are targeting a merc for some reason, don't go thorugh normal channels if we are on
    // someone now
    if (uiFlags == MOVEUI_TARGET_MERCS || uiFlags == MOVEUI_TARGET_MERCSFORAID) {
      if (gfUIFullTargetFound != fTargetFound || usTargetID != gusUIFullTargetID ||
          gfResetUIMovementOptimization) {
        gfResetUIMovementOptimization = FALSE;

        // ERASE PATH
        ErasePath(TRUE);

        // Try and get a path right away
        DrawUIMovementPath(pSoldier, usMapPos, uiFlags);
      }

      // Save for next time...
      fTargetFound = gfUIFullTargetFound;
      usTargetID = gusUIFullTargetID;

      if (fTargetFound) {
        fTargetFoundAndLookingForOne = TRUE;
      }
    }

    if (uiFlags == MOVEUI_TARGET_ITEMS) {
      gfUIOverItemPool = TRUE;
      gfUIOverItemPoolGridNo = usMapPos;
    } else if (uiFlags == MOVEUI_TARGET_MERCSFORAID) {
      // Set values for AP display...
      gfUIDisplayActionPointsCenter = TRUE;
    }

    // IF CURSOR IS MOVING
    if ((uiCursorFlags & MOUSE_MOVING) || gfUINewStateForIntTile) {
      // SHOW CURSOR
      fSetCursor = TRUE;

      // IF CURSOR WAS PREVIOUSLY STATIONARY, MAKE THE ADDITIONAL CHECK OF GRID POS CHANGE
      if (((uiCursorFlags & MOUSE_MOVING_NEW_TILE) && !fTargetFoundAndLookingForOne) ||
          gfUINewStateForIntTile) {
        // ERASE PATH
        ErasePath(TRUE);

        // Reset counter
        RESETCOUNTER(PATHFINDCOUNTER);

        gfPlotNewMovement = TRUE;
      }

      if (uiCursorFlags & MOUSE_MOVING_IN_TILE) {
        gfUIDisplayActionPoints = TRUE;
      }
    }

    if (uiCursorFlags & MOUSE_STATIONARY) {
      // CURSOR IS STATIONARY
      if (_KeyDown(SHIFT) && !gfPlotNewMovementNOCOST) {
        gfPlotNewMovementNOCOST = TRUE;
        gfPlotNewMovement = TRUE;
      }
      if (!(_KeyDown(SHIFT)) && gfPlotNewMovementNOCOST) {
        gfPlotNewMovementNOCOST = FALSE;
        gfPlotNewMovement = TRUE;
      }

      // ONLY DIPSLAY PATH AFTER A DELAY
      if (COUNTERDONE(PATHFINDCOUNTER)) {
        // Reset counter
        RESETCOUNTER(PATHFINDCOUNTER);

        if (gfPlotNewMovement) {
          DrawUIMovementPath(pSoldier, usMapPos, uiFlags);

          gfPlotNewMovement = FALSE;
        }
      }

      fSetCursor = TRUE;

      // DISPLAY POINTS EVEN WITHOUT DELAY
      // ONLY IF GFPLOT NEW MOVEMENT IS FALSE!
      if (!gfPlotNewMovement) {
        if (gsCurrentActionPoints < 0 ||
            ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT))) {
          gfUIDisplayActionPoints = FALSE;
        } else {
          gfUIDisplayActionPoints = TRUE;

          if (uiFlags == MOVEUI_TARGET_INTTILES) {
            // Set values for AP display...
            gUIDisplayActionPointsOffX = 22;
            gUIDisplayActionPointsOffY = 15;
          }
          if (uiFlags == MOVEUI_TARGET_BOMB) {
            // Set values for AP display...
            gUIDisplayActionPointsOffX = 22;
            gUIDisplayActionPointsOffY = 15;
          } else if (uiFlags == MOVEUI_TARGET_ITEMS) {
            // Set values for AP display...
            gUIDisplayActionPointsOffX = 22;
            gUIDisplayActionPointsOffY = 15;
          } else {
            switch (pSoldier->usUIMovementMode) {
              case WALKING:

                gUIDisplayActionPointsOffY = 10;
                gUIDisplayActionPointsOffX = 10;
                break;

              case RUNNING:
                gUIDisplayActionPointsOffY = 15;
                gUIDisplayActionPointsOffX = 21;
                break;
            }
          }
        }
      }
    }

  } else {
    // THE MERC IS MOVING
    // We're moving, erase path, change cursor
    ErasePath(TRUE);

    fSetCursor = TRUE;
  }

  return (fSetCursor);
}

int8_t DrawUIMovementPath(struct SOLDIERTYPE *pSoldier, uint16_t usMapPos, uint32_t uiFlags) {
  int16_t sAPCost, sBPCost;
  int16_t sActionGridNo;
  struct STRUCTURE *pStructure;
  uint8_t ubDirection;
  //	struct ITEM_POOL					*pItemPool;
  int16_t sAdjustedGridNo;
  int16_t sIntTileGridNo;
  struct LEVELNODE *pIntTile;
  int8_t bReturnCode = 0;
  BOOLEAN fPlot;
  uint8_t ubMercID;

  if (((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED)) ||
      _KeyDown(SHIFT)) {
    fPlot = PLOT;
  } else {
    fPlot = NO_PLOT;
  }

  sActionGridNo = usMapPos;
  sAPCost = 0;

  ErasePath(FALSE);

  // IF WE ARE OVER AN INTERACTIVE TILE, GIVE GRIDNO OF POSITION
  if (uiFlags == MOVEUI_TARGET_INTTILES) {
    // Get structure info for in tile!
    pIntTile = GetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure);

    // We should not have null here if we are given this flag...
    if (pIntTile != NULL) {
      sActionGridNo = FindAdjacentGridEx(pSoldier, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE);
      if (sActionGridNo == -1) {
        sActionGridNo = sIntTileGridNo;
      }
      CalcInteractiveObjectAPs(sIntTileGridNo, pStructure, &sAPCost, &sBPCost);
      // sAPCost += UIPlotPath( pSoldier, sActionGridNo, NO_COPYROUTE, PLOT, TEMPORARY,
      // (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints);
      sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);

      if (sActionGridNo != pSoldier->sGridNo) {
        gfUIHandleShowMoveGrid = TRUE;
        gsUIHandleShowMoveGridLocation = sActionGridNo;
      }

      // Add cost for stance change....
      sAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
    } else {
      sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);
    }
  } else if (uiFlags == MOVEUI_TARGET_WIREFENCE) {
    sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, &ubDirection, NULL, FALSE, TRUE);
    if (sActionGridNo == -1) {
      sAPCost = 0;
    } else {
      sAPCost = GetAPsToCutFence(pSoldier);

      sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);

      if (sActionGridNo != pSoldier->sGridNo) {
        gfUIHandleShowMoveGrid = TRUE;
        gsUIHandleShowMoveGridLocation = sActionGridNo;
      }
    }
  } else if (uiFlags == MOVEUI_TARGET_JAR) {
    sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, &ubDirection, NULL, FALSE, TRUE);
    if (sActionGridNo == -1) {
      sActionGridNo = usMapPos;
    }

    sAPCost = GetAPsToUseJar(pSoldier, sActionGridNo);

    sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                          (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                          pSoldier->bActionPoints);

    if (sActionGridNo != pSoldier->sGridNo) {
      gfUIHandleShowMoveGrid = TRUE;
      gsUIHandleShowMoveGridLocation = sActionGridNo;
    }
  } else if (uiFlags == MOVEUI_TARGET_CAN) {
    // Get structure info for in tile!
    pIntTile = GetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure);

    // We should not have null here if we are given this flag...
    if (pIntTile != NULL) {
      sActionGridNo = FindAdjacentGridEx(pSoldier, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE);
      if (sActionGridNo != -1) {
        sAPCost = AP_ATTACH_CAN;
        sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                              (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                              pSoldier->bActionPoints);

        if (sActionGridNo != pSoldier->sGridNo) {
          gfUIHandleShowMoveGrid = TRUE;
          gsUIHandleShowMoveGridLocation = sActionGridNo;
        }
      }
    } else {
      sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);
    }

  } else if (uiFlags == MOVEUI_TARGET_REPAIR) {
    // For repair, check if we are over a vehicle, then get gridnot to edge of that vehicle!
    if (IsRepairableStructAtGridNo(usMapPos, &ubMercID) == 2) {
      int16_t sNewGridNo;
      uint8_t ubDirection;

      sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(
          pSoldier, pSoldier->usUIMovementMode, 5, &ubDirection, 0, MercPtrs[ubMercID]);

      if (sNewGridNo != NOWHERE) {
        usMapPos = sNewGridNo;
      }
    }

    sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, &ubDirection, NULL, FALSE, TRUE);
    if (sActionGridNo == -1) {
      sActionGridNo = usMapPos;
    }

    sAPCost = GetAPsToBeginRepair(pSoldier);

    sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                          (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                          pSoldier->bActionPoints);

    if (sActionGridNo != pSoldier->sGridNo) {
      gfUIHandleShowMoveGrid = TRUE;
      gsUIHandleShowMoveGridLocation = sActionGridNo;
    }
  } else if (uiFlags == MOVEUI_TARGET_REFUEL) {
    // For repair, check if we are over a vehicle, then get gridnot to edge of that vehicle!
    if (IsRefuelableStructAtGridNo(usMapPos, &ubMercID) == 2) {
      int16_t sNewGridNo;
      uint8_t ubDirection;

      sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(
          pSoldier, pSoldier->usUIMovementMode, 5, &ubDirection, 0, MercPtrs[ubMercID]);

      if (sNewGridNo != NOWHERE) {
        usMapPos = sNewGridNo;
      }
    }

    sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, &ubDirection, NULL, FALSE, TRUE);
    if (sActionGridNo == -1) {
      sActionGridNo = usMapPos;
    }

    sAPCost = GetAPsToRefuelVehicle(pSoldier);

    sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                          (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                          pSoldier->bActionPoints);

    if (sActionGridNo != pSoldier->sGridNo) {
      gfUIHandleShowMoveGrid = TRUE;
      gsUIHandleShowMoveGridLocation = sActionGridNo;
    }
  } else if (uiFlags == MOVEUI_TARGET_MERCS) {
    int16_t sGotLocation = NOWHERE;
    BOOLEAN fGotAdjacent = FALSE;

    // Check if we are on a target
    if (gfUIFullTargetFound) {
      int32_t cnt;
      int16_t sSpot;
      uint8_t ubGuyThere;

      for (cnt = 0; cnt < NUM_WORLD_DIRECTIONS; cnt++) {
        sSpot = (int16_t)NewGridNo(pSoldier->sGridNo, DirectionInc((int8_t)cnt));

        // Make sure movement costs are OK....
        if (gubWorldMovementCosts[sSpot][cnt][gsInterfaceLevel] >= TRAVELCOST_BLOCKED) {
          continue;
        }

        // Check for who is there...
        ubGuyThere = WhoIsThere2(sSpot, pSoldier->bLevel);

        if (ubGuyThere == MercPtrs[gusUIFullTargetID]->ubID) {
          // We've got a guy here....
          // Who is the one we want......
          sGotLocation = sSpot;
          sAdjustedGridNo = MercPtrs[gusUIFullTargetID]->sGridNo;
          ubDirection = (uint8_t)cnt;
          break;
        }
      }

      if (sGotLocation == NOWHERE) {
        sActionGridNo = FindAdjacentGridEx(pSoldier, MercPtrs[gusUIFullTargetID]->sGridNo,
                                           &ubDirection, &sAdjustedGridNo, TRUE, FALSE);

        if (sActionGridNo == -1) {
          sGotLocation = NOWHERE;
        } else {
          sGotLocation = sActionGridNo;
        }
        fGotAdjacent = TRUE;
      }
    } else {
      sAdjustedGridNo = usMapPos;
      sGotLocation = sActionGridNo;
      fGotAdjacent = TRUE;
    }

    if (sGotLocation != NOWHERE) {
      sAPCost += MinAPsToAttack(pSoldier, sAdjustedGridNo, TRUE);
      sAPCost += UIPlotPath(pSoldier, sGotLocation, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);

      if (sGotLocation != pSoldier->sGridNo && fGotAdjacent) {
        gfUIHandleShowMoveGrid = TRUE;
        gsUIHandleShowMoveGridLocation = sGotLocation;
      }
    }
  } else if (uiFlags == MOVEUI_TARGET_STEAL) {
    // Check if we are on a target
    if (gfUIFullTargetFound) {
      sActionGridNo = FindAdjacentGridEx(pSoldier, MercPtrs[gusUIFullTargetID]->sGridNo,
                                         &ubDirection, &sAdjustedGridNo, TRUE, FALSE);
      if (sActionGridNo == -1) {
        sActionGridNo = sAdjustedGridNo;
      }
      sAPCost += AP_STEAL_ITEM;
      // CJC August 13 2002: take into account stance in AP prediction
      if (!(PTR_STANDING)) {
        sAPCost += GetAPsToChangeStance(pSoldier, ANIM_STAND);
      }
      sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);

      if (sActionGridNo != pSoldier->sGridNo) {
        gfUIHandleShowMoveGrid = TRUE;
        gsUIHandleShowMoveGridLocation = sActionGridNo;
      }
    }
  } else if (uiFlags == MOVEUI_TARGET_BOMB) {
    sAPCost += GetAPsToDropBomb(pSoldier);
    sAPCost += UIPlotPath(pSoldier, usMapPos, NO_COPYROUTE, fPlot, TEMPORARY,
                          (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                          pSoldier->bActionPoints);

    gfUIHandleShowMoveGrid = TRUE;
    gsUIHandleShowMoveGridLocation = usMapPos;
  } else if (uiFlags == MOVEUI_TARGET_MERCSFORAID) {
    if (gfUIFullTargetFound) {
      sActionGridNo = FindAdjacentGridEx(pSoldier, MercPtrs[gusUIFullTargetID]->sGridNo,
                                         &ubDirection, &sAdjustedGridNo, TRUE, FALSE);

      // Try again at another gridno...
      if (sActionGridNo == -1) {
        sActionGridNo =
            FindAdjacentGridEx(pSoldier, usMapPos, &ubDirection, &sAdjustedGridNo, TRUE, FALSE);

        if (sActionGridNo == -1) {
          sActionGridNo = sAdjustedGridNo;
        }
      }
      sAPCost += GetAPsToBeginFirstAid(pSoldier);
      sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                            (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                            pSoldier->bActionPoints);
      if (sActionGridNo != pSoldier->sGridNo) {
        gfUIHandleShowMoveGrid = TRUE;
        gsUIHandleShowMoveGridLocation = sActionGridNo;
      }
    }
  } else if (uiFlags == MOVEUI_TARGET_ITEMS) {
    // if ( GetItemPool( usMapPos, &pItemPool, pSoldier->bLevel ) )
    {
      // if ( ITEMPOOL_VISIBLE( pItemPool ) )
      {
        sActionGridNo = AdjustGridNoForItemPlacement(pSoldier, sActionGridNo);

        if (pSoldier->sGridNo != sActionGridNo) {
          sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                                (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                                pSoldier->bActionPoints);
          if (sAPCost != 0) {
            sAPCost += AP_PICKUP_ITEM;
          }
        } else {
          sAPCost += AP_PICKUP_ITEM;
        }

        if (sActionGridNo != pSoldier->sGridNo) {
          gfUIHandleShowMoveGrid = TRUE;
          gsUIHandleShowMoveGridLocation = sActionGridNo;
        }
      }
    }
  } else {
    sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, TEMPORARY,
                          (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                          pSoldier->bActionPoints);
  }

  if (gTacticalStatus.uiFlags & SHOW_AP_LEFT) {
    gsCurrentActionPoints = pSoldier->bActionPoints - sAPCost;
  } else {
    gsCurrentActionPoints = sAPCost;
  }

  return (bReturnCode);
}

BOOLEAN UIMouseOnValidAttackLocation(struct SOLDIERTYPE *pSoldier) {
  uint16_t usInHand;
  BOOLEAN fGuyHere = FALSE;
  struct SOLDIERTYPE *pTSoldier;
  uint8_t ubItemCursor;
  int16_t usMapPos;

  if (!GetMouseMapPos(&usMapPos)) {
    return (FALSE);
  }

  // LOOK IN GUY'S HAND TO CHECK LOCATION
  usInHand = pSoldier->inv[HANDPOS].usItem;

  // Get cursor value
  ubItemCursor = GetActionModeCursor(pSoldier);

  if (ubItemCursor == INVALIDCURS) {
    return (FALSE);
  }

  if (ubItemCursor == WIRECUTCURS) {
    if (IsCuttableWireFenceAtGridNo(usMapPos) && pSoldier->bLevel == 0) {
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  if (ubItemCursor == REPAIRCURS) {
    if (gfUIFullTargetFound) {
      usMapPos = MercPtrs[gusUIFullTargetID]->sGridNo;
    }

    if (IsRepairableStructAtGridNo(usMapPos, NULL) && pSoldier->bLevel == 0) {
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  if (ubItemCursor == REFUELCURS) {
    if (gfUIFullTargetFound) {
      usMapPos = MercPtrs[gusUIFullTargetID]->sGridNo;
    }

    if (IsRefuelableStructAtGridNo(usMapPos, NULL) && pSoldier->bLevel == 0) {
      return (TRUE);
    } else {
      return (FALSE);
    }
  }

  if (ubItemCursor == BOMBCURS) {
    if (usMapPos == pSoldier->sGridNo) {
      return (TRUE);
    }

    if (!NewOKDestination(pSoldier, usMapPos, TRUE, pSoldier->bLevel)) {
      return (FALSE);
    }
  }

  // SEE IF THERE IS SOMEBODY HERE
  if (gfUIFullTargetFound && ubItemCursor != KNIFECURS) {
    fGuyHere = TRUE;

    if (guiUIFullTargetFlags & SELECTED_MERC && Item[usInHand].usItemClass != IC_MEDKIT) {
      return (FALSE);
    }
  }

  if (!CanPlayerUseRocketRifle(pSoldier, TRUE)) {
    return (FALSE);
  }

  // if ( Item[ usInHand ].usItemClass == IC_BLADE && usInHand != THROWING_KNIFE )
  //{
  //	if ( !fGuyHere )
  //	{
  //	return( FALSE );
  //	}
  //}

  if (Item[usInHand].usItemClass == IC_PUNCH) {
    if (!fGuyHere) {
      return (FALSE);
    }
  }

  // if ( Item[ usInHand ].usItemClass == IC_BLADE )
  //{
  //	if ( !fGuyHere )
  //	{
  //		return( FALSE );
  //	}
  //}

  if (Item[usInHand].usItemClass == IC_MEDKIT) {
    if (!fGuyHere) {
      return (FALSE);
    }

    // IF a guy's here, chack if they need medical help!
    pTSoldier = MercPtrs[gusUIFullTargetID];

    // If we are a vehicle...
    if ((pTSoldier->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ROBOT))) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[CANNOT_DO_FIRST_AID_STR],
                pTSoldier->name);
      return (FALSE);
    }

    if (pSoldier->bMedical == 0) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, pMessageStrings[MSG_MERC_HAS_NO_MEDSKILL],
                pSoldier->name);
      return (FALSE);
    }

    if (pTSoldier->bBleeding == 0 && pTSoldier->bLife != pTSoldier->bLifeMax) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[19], pTSoldier->name);
      return (FALSE);
    }

    if (pTSoldier->bBleeding == 0 && pTSoldier->bLife >= OKLIFE) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[CANNOT_NO_NEED_FIRST_AID_STR],
                pTSoldier->name);
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN UIOkForItemPickup(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sAPCost;
  struct ITEM_POOL *pItemPool;

  sAPCost = GetAPsToPickupItem(pSoldier, sGridNo);

  if (sAPCost == 0) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
  } else {
    if (GetItemPool(sGridNo, &pItemPool, pSoldier->bLevel)) {
      // if ( !ITEMPOOL_VISIBLE( pItemPool ) )
      {
        //		return( FALSE );
      }
    }

    if (EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN SoldierCanAffordNewStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance) {
  int8_t bCurrentHeight;
  uint8_t bAP = 0, bBP = 0;

  bCurrentHeight = (ubDesiredStance - gAnimControl[pSoldier->usAnimState].ubEndHeight);

  // Now change to appropriate animation

  switch (bCurrentHeight) {
    case ANIM_STAND - ANIM_CROUCH:
    case ANIM_CROUCH - ANIM_STAND:

      bAP = AP_CROUCH;
      bBP = BP_CROUCH;
      break;

    case ANIM_STAND - ANIM_PRONE:
    case ANIM_PRONE - ANIM_STAND:

      bAP = AP_CROUCH + AP_PRONE;
      bBP = BP_CROUCH + BP_PRONE;
      break;

    case ANIM_CROUCH - ANIM_PRONE:
    case ANIM_PRONE - ANIM_CROUCH:

      bAP = AP_PRONE;
      bBP = BP_PRONE;
      break;
  }

  return (EnoughPoints(pSoldier, bAP, bBP, TRUE));
}

void SetUIbasedOnStance(struct SOLDIERTYPE *pSoldier, int8_t bNewStance) {
  // Set UI based on our stance!
  switch (bNewStance) {
    case ANIM_STAND:
      pSoldier->usUIMovementMode = WALKING;
      break;

    case ANIM_CROUCH:
      pSoldier->usUIMovementMode = SWATTING;
      break;

    case ANIM_PRONE:
      pSoldier->usUIMovementMode = CRAWLING;
      break;
  }

  // Set UI cursor!
}

void SetMovementModeCursor(struct SOLDIERTYPE *pSoldier) {
  if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
    if ((OK_ENTERABLE_VEHICLE(pSoldier))) {
      guiNewUICursor = MOVE_VEHICLE_UICURSOR;
    } else {
      // Change mouse cursor based on type of movement we want to do
      switch (pSoldier->usUIMovementMode) {
        case WALKING:
          guiNewUICursor = MOVE_WALK_UICURSOR;
          break;

        case RUNNING:
          guiNewUICursor = MOVE_RUN_UICURSOR;
          break;

        case SWATTING:
          guiNewUICursor = MOVE_SWAT_UICURSOR;
          break;

        case CRAWLING:
          guiNewUICursor = MOVE_PRONE_UICURSOR;
          break;
      }
    }
  }

  if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    if (gfUIAllMoveOn) {
      guiNewUICursor = ALL_MOVE_REALTIME_UICURSOR;
    } else {
      // if ( pSoldier->fUIMovementFast )
      //{
      //	BeginDisplayTimedCursor( MOVE_RUN_REALTIME_UICURSOR, 300 );
      //}

      guiNewUICursor = MOVE_REALTIME_UICURSOR;
    }
  }

  guiNewUICursor = GetInteractiveTileCursor(guiNewUICursor, FALSE);
}

void SetConfirmMovementModeCursor(struct SOLDIERTYPE *pSoldier, BOOLEAN fFromMove) {
  if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
    if (gfUIAllMoveOn) {
      if ((OK_ENTERABLE_VEHICLE(pSoldier))) {
        guiNewUICursor = ALL_MOVE_VEHICLE_UICURSOR;
      } else {
        // Change mouse cursor based on type of movement we want to do
        switch (pSoldier->usUIMovementMode) {
          case WALKING:
            guiNewUICursor = ALL_MOVE_WALK_UICURSOR;
            break;

          case RUNNING:
            guiNewUICursor = ALL_MOVE_RUN_UICURSOR;
            break;

          case SWATTING:
            guiNewUICursor = ALL_MOVE_SWAT_UICURSOR;
            break;

          case CRAWLING:
            guiNewUICursor = ALL_MOVE_PRONE_UICURSOR;
            break;
        }
      }
    } else {
      if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
        guiNewUICursor = CONFIRM_MOVE_VEHICLE_UICURSOR;
      } else {
        // Change mouse cursor based on type of movement we want to do
        switch (pSoldier->usUIMovementMode) {
          case WALKING:
            guiNewUICursor = CONFIRM_MOVE_WALK_UICURSOR;
            break;

          case RUNNING:
            guiNewUICursor = CONFIRM_MOVE_RUN_UICURSOR;
            break;

          case SWATTING:
            guiNewUICursor = CONFIRM_MOVE_SWAT_UICURSOR;
            break;

          case CRAWLING:
            guiNewUICursor = CONFIRM_MOVE_PRONE_UICURSOR;
            break;
        }
      }
    }
  }

  if ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    if (gfUIAllMoveOn) {
      if (gfUIAllMoveOn == 2) {
        BeginDisplayTimedCursor(MOVE_RUN_REALTIME_UICURSOR, 300);
      } else {
        guiNewUICursor = ALL_MOVE_REALTIME_UICURSOR;
      }
    } else {
      if (pSoldier->fUIMovementFast && pSoldier->usAnimState == RUNNING && fFromMove) {
        BeginDisplayTimedCursor(MOVE_RUN_REALTIME_UICURSOR, 300);
      }

      guiNewUICursor = CONFIRM_MOVE_REALTIME_UICURSOR;
    }
  }

  guiNewUICursor = GetInteractiveTileCursor(guiNewUICursor, TRUE);
}

uint32_t UIHandleLCOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t sFacingDir, sXPos, sYPos;

  guiNewUICursor = LOOK_UICURSOR;

  // Get soldier
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  gfUIDisplayActionPoints = TRUE;

  gUIDisplayActionPointsOffX = 14;
  gUIDisplayActionPointsOffY = 7;

  // Get soldier
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  GetMouseXY(&sXPos, &sYPos);

  // Get direction from mouse pos
  sFacingDir = GetDirectionFromXY(sXPos, sYPos, pSoldier);

  // Set # of APs
  if (sFacingDir != pSoldier->bDirection) {
    gsCurrentActionPoints = GetAPsToLook(pSoldier);
  } else {
    gsCurrentActionPoints = 0;
  }

  // Determine if we can afford!
  if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
    gfUIDisplayActionPointsInvalid = TRUE;
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleLCChangeToLook(UI_EVENT *pUIEvent) {
  ErasePath(TRUE);

  return (GAME_SCREEN);
}

BOOLEAN MakeSoldierTurn(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos) {
  int16_t sFacingDir, sAPCost;

  // Get direction from mouse pos
  sFacingDir = GetDirectionFromXY(sXPos, sYPos, pSoldier);

  if (sFacingDir != pSoldier->bDirection) {
    sAPCost = GetAPsToLook(pSoldier);

    // Check AP cost...
    if (!EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
      return (FALSE);
    }

    // ATE: make stationary if...
    if (pSoldier->fNoAPToFinishMove) {
      SoldierGotoStationaryStance(pSoldier);
    }

    // DEF:  made it an event
    SendSoldierSetDesiredDirectionEvent(pSoldier, sFacingDir);

    pSoldier->bTurningFromUI = TRUE;

    // ATE: Hard-code here previous event to ui busy event...
    guiOldEvent = LA_BEGINUIOURTURNLOCK;

    return (TRUE);
  }

  return (FALSE);
}

uint32_t UIHandleLCLook(UI_EVENT *pUIEvent) {
  int16_t sXPos, sYPos;
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  if (!GetMouseXY(&sXPos, &sYPos)) {
    return (GAME_SCREEN);
  }

  if (gTacticalStatus.fAtLeastOneGuyOnMultiSelect) {
    // OK, loop through all guys who are 'multi-selected' and
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pSoldier++) {
      if (IsSolActive(pSoldier) && pSoldier->bInSector) {
        if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
          MakeSoldierTurn(pSoldier, sXPos, sYPos);
        }
      }
    }
  } else {
    // Get soldier
    if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
      return (GAME_SCREEN);
    }

    if (MakeSoldierTurn(pSoldier, sXPos, sYPos)) {
      SetUIBusy(pSoldier->ubID);
    }
  }
  return (GAME_SCREEN);
}

uint32_t UIHandleTOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  uint8_t ubTargID;
  uint32_t uiRange;
  int16_t usMapPos;
  BOOLEAN fValidTalkableGuy = FALSE;
  int16_t sTargetGridNo;
  int16_t sDistVisible;

  // Get soldier
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (ValidQuickExchangePosition()) {
    // Do new cursor!
    guiPendingOverrideEvent = M_ON_TERRAIN;
    return (UIHandleMOnTerrain(pUIEvent));
  }

  sTargetGridNo = usMapPos;

  UIHandleOnMerc(FALSE);

  // CHECK FOR VALID TALKABLE GUY HERE
  fValidTalkableGuy = IsValidTalkableNPCFromMouse(&ubTargID, FALSE, TRUE, FALSE);

  // USe cursor based on distance
  // Get distance away
  if (fValidTalkableGuy) {
    sTargetGridNo = MercPtrs[ubTargID]->sGridNo;
  }

  uiRange = GetRangeFromGridNoDiff(pSoldier->sGridNo, sTargetGridNo);

  // ATE: Check if we have good LOS
  // is he close enough to see that gridno if he turns his head?
  sDistVisible = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT,
                                 sTargetGridNo, pSoldier->bLevel);

  if (uiRange <= NPC_TALK_RADIUS) {
    if (fValidTalkableGuy) {
      guiNewUICursor = TALK_A_UICURSOR;
    } else {
      guiNewUICursor = TALK_NA_UICURSOR;
    }
  } else {
    if (fValidTalkableGuy) {
      // guiNewUICursor = TALK_OUT_RANGE_A_UICURSOR;
      guiNewUICursor = TALK_A_UICURSOR;
    } else {
      guiNewUICursor = TALK_OUT_RANGE_NA_UICURSOR;
    }
  }

  if (fValidTalkableGuy) {
    if (!SoldierTo3DLocationLineOfSightTest(pSoldier, sTargetGridNo, pSoldier->bLevel, 3,
                                            (uint8_t)sDistVisible, TRUE)) {
      //. ATE: Make range far, so we alternate cursors...
      guiNewUICursor = TALK_OUT_RANGE_A_UICURSOR;
    }
  }

  gfUIDisplayActionPoints = TRUE;

  gUIDisplayActionPointsOffX = 8;
  gUIDisplayActionPointsOffY = 3;

  // Set # of APs
  gsCurrentActionPoints = 6;

  // Determine if we can afford!
  if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
    gfUIDisplayActionPointsInvalid = TRUE;
  }

  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    if (gfUIFullTargetFound) {
      PauseRT(TRUE);
    } else {
      PauseRT(FALSE);
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleTChangeToTalking(UI_EVENT *pUIEvent) {
  ErasePath(TRUE);

  return (GAME_SCREEN);
}

uint32_t UIHandleLUIOnTerrain(UI_EVENT *pUIEvent) {
  // guiNewUICursor = NO_UICURSOR;
  //	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

  return (GAME_SCREEN);
}

uint32_t UIHandleLUIBeginLock(UI_EVENT *pUIEvent) {
  // Don't let both versions of the locks to happen at the same time!
  // ( They are mutually exclusive )!
  UIHandleLAEndLockOurTurn(NULL);

  if (!gfDisableRegionActive) {
    gfDisableRegionActive = TRUE;

    RemoveTacticalCursor();
    // SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

    MSYS_DefineRegion(&gDisableRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, CURSOR_WAIT,
                      MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
    // Add region
    MSYS_AddRegion(&gDisableRegion);

    // guiPendingOverrideEvent = LOCKUI_MODE;

    // UnPause time!
    PauseGame();
    LockPauseState(16);
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleLUIEndLock(UI_EVENT *pUIEvent) {
  if (gfDisableRegionActive) {
    gfDisableRegionActive = FALSE;

    // Add region
    MSYS_RemoveRegion(&gDisableRegion);
    RefreshMouseRegions();

    // SetCurrentCursorFromDatabase( guiCurrentUICursor );

    guiForceRefreshMousePositionCalculation = TRUE;
    UIHandleMOnTerrain(NULL);

    if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
      SetCurrentCursorFromDatabase(gUICursors[guiNewUICursor].usFreeCursorName);
    }

    guiPendingOverrideEvent = M_ON_TERRAIN;
    HandleTacticalUI();

    // ATE: Only if NOT in conversation!
    if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
      // UnPause time!
      UnLockPauseState();
      UnPauseGame();
    }
  }

  return (GAME_SCREEN);
}

void CheckForDisabledRegionRemove() {
  if (gfDisableRegionActive) {
    gfDisableRegionActive = FALSE;

    // Remove region
    MSYS_RemoveRegion(&gDisableRegion);

    UnLockPauseState();
    UnPauseGame();
  }

  if (gfUserTurnRegionActive) {
    gfUserTurnRegionActive = FALSE;

    gfUIInterfaceSetBusy = FALSE;

    // Remove region
    MSYS_RemoveRegion(&gUserTurnRegion);

    UnLockPauseState();
    UnPauseGame();
  }
}

uint32_t UIHandleLAOnTerrain(UI_EVENT *pUIEvent) {
  // guiNewUICursor = NO_UICURSOR;
  // SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

  return (GAME_SCREEN);
}

void GetGridNoScreenXY(int16_t sGridNo, int16_t *pScreenX, int16_t *pScreenY) {
  int16_t sScreenX, sScreenY;
  int16_t sOffsetX, sOffsetY;
  int16_t sTempX_S, sTempY_S;
  int16_t sXPos, sYPos;

  ConvertGridNoToCellXY(sGridNo, &sXPos, &sYPos);

  // Get 'TRUE' merc position
  sOffsetX = sXPos - gsRenderCenterX;
  sOffsetY = sYPos - gsRenderCenterY;

  FromCellToScreenCoordinates(sOffsetX, sOffsetY, &sTempX_S, &sTempY_S);

  sScreenX = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)sTempX_S;
  sScreenY = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)sTempY_S;

  // Adjust for offset position on screen
  sScreenX -= gsRenderWorldOffsetX;
  sScreenY -= gsRenderWorldOffsetY;
  sScreenY -= gpWorldLevelData[sGridNo].sHeight;

  // Adjust based on interface level

  // Adjust for render height
  sScreenY += gsRenderHeight;

  // Adjust y offset!
  sScreenY += (WORLD_TILE_Y / 2);

  (*pScreenX) = sScreenX;
  (*pScreenY) = sScreenY;
}

void EndMultiSoldierSelection(BOOLEAN fAcknowledge) {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;
  struct SOLDIERTYPE *pFirstSoldier = NULL;
  BOOLEAN fSelectedSoldierInBatch = FALSE;

  gTacticalStatus.fAtLeastOneGuyOnMultiSelect = FALSE;

  // OK, loop through all guys who are 'multi-selected' and
  // check if our currently selected guy is amoung the
  // lucky few.. if not, change to a guy who is...
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
        gTacticalStatus.fAtLeastOneGuyOnMultiSelect = TRUE;

        if (pSoldier->ubID != gusSelectedSoldier && pFirstSoldier == NULL) {
          pFirstSoldier = pSoldier;
        }

        if (pSoldier->ubID == gusSelectedSoldier) {
          fSelectedSoldierInBatch = TRUE;
        }

        if (!gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS] && fAcknowledge)
          InternalDoMercBattleSound(pSoldier, BATTLE_SOUND_ATTN1, BATTLE_SND_LOWER_VOLUME);

        if (pSoldier->fMercAsleep) {
          PutMercInAwakeState(pSoldier);
        }
      }
    }
  }

  // If here, select the first guy...
  if (pFirstSoldier != NULL && !fSelectedSoldierInBatch) {
    SelectSoldier(pFirstSoldier->ubID, TRUE, TRUE);
  }
}

void StopRubberBandedMercFromMoving() {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  if (!gTacticalStatus.fAtLeastOneGuyOnMultiSelect) {
    return;
  }

  // OK, loop through all guys who are 'multi-selected' and
  // check if our currently selected guy is amoung the
  // lucky few.. if not, change to a guy who is...
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
        pSoldier->fDelayedMovement = FALSE;
        pSoldier->sFinalDestination = pSoldier->sGridNo;
        StopSoldier(pSoldier);
      }
    }
  }
}

void EndRubberBanding() {
  if (gRubberBandActive) {
    FreeMouseCursor();
    gfIgnoreScrolling = FALSE;

    EndMultiSoldierSelection(TRUE);

    gRubberBandActive = FALSE;
  }
}

BOOLEAN HandleMultiSelectionMove(int16_t sDestGridNo) {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;
  BOOLEAN fAtLeastOneMultiSelect = FALSE;
  BOOLEAN fMoveFast = FALSE;

  // OK, loop through all guys who are 'multi-selected' and
  // Make them move....

  // Do a loop first to see if the selected guy is told to go fast...
  gfGetNewPathThroughPeople = TRUE;

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
        if (pSoldier->ubID == gusSelectedSoldier) {
          fMoveFast = pSoldier->fUIMovementFast;
          break;
        }
      }
    }
  }

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
        // If we can't be controlled, returninvalid...
        if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
          if (!CanRobotBeControlled(pSoldier)) {
            continue;
          }
        }

        pSoldier->fUIMovementFast = fMoveFast;
        pSoldier->usUIMovementMode =
            GetMoveStateBasedOnStance(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight);

        pSoldier->fUIMovementFast = FALSE;

        if (gUIUseReverse) {
          pSoldier->bReverse = TRUE;
        } else {
          pSoldier->bReverse = FALSE;
        }

        // Remove any previous actions
        pSoldier->ubPendingAction = NO_PENDING_ACTION;

        if (EVENT_InternalGetNewSoldierPath(pSoldier, sDestGridNo, pSoldier->usUIMovementMode, TRUE,
                                            pSoldier->fNoAPToFinishMove)) {
          InternalDoMercBattleSound(pSoldier, BATTLE_SOUND_OK1, BATTLE_SND_LOWER_VOLUME);
        } else {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[NO_PATH_FOR_MERC],
                    pSoldier->name);
        }

        fAtLeastOneMultiSelect = TRUE;
      }
    }
  }
  gfGetNewPathThroughPeople = FALSE;

  return (fAtLeastOneMultiSelect);
}

void ResetMultiSelection() {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  // OK, loop through all guys who are 'multi-selected' and
  // Make them move....

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
        pSoldier->uiStatusFlags &= (~SOLDIER_MULTI_SELECTED);
      }
    }
  }

  gTacticalStatus.fAtLeastOneGuyOnMultiSelect = FALSE;
}

uint32_t UIHandleRubberBandOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;
  int16_t sScreenX, sScreenY;
  int32_t iTemp;
  SGPRect aRect;
  BOOLEAN fAtLeastOne = FALSE;

  guiNewUICursor = NO_UICURSOR;
  // SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

  gRubberBandRect.iRight = gusMouseXPos;
  gRubberBandRect.iBottom = gusMouseYPos;

  // Copy into temp rect
  memcpy(&aRect, &gRubberBandRect, sizeof(gRubberBandRect));

  if (aRect.iRight < aRect.iLeft) {
    iTemp = aRect.iLeft;
    aRect.iLeft = aRect.iRight;
    aRect.iRight = iTemp;
  }

  if (aRect.iBottom < aRect.iTop) {
    iTemp = aRect.iTop;
    aRect.iTop = aRect.iBottom;
    aRect.iBottom = iTemp;
  }

  // ATE:Check at least for one guy that's in point!
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // Check if this guy is OK to control....
    if (OK_CONTROLLABLE_MERC(pSoldier) &&
        !(pSoldier->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER))) {
      // Get screen pos of gridno......
      GetGridNoScreenXY(pSoldier->sGridNo, &sScreenX, &sScreenY);

      // ATE: If we are in a hiehger interface level, subttrasct....
      if (gsInterfaceLevel == 1) {
        sScreenY -= 50;
      }

      if (IsPointInScreenRect(sScreenX, sScreenY, &aRect)) {
        fAtLeastOne = TRUE;
      }
    }
  }

  if (!fAtLeastOne) {
    return (GAME_SCREEN);
  }

  // ATE: Now loop through our guys and see if any fit!
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // Check if this guy is OK to control....
    if (OK_CONTROLLABLE_MERC(pSoldier) &&
        !(pSoldier->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER))) {
      if (!_KeyDown(ALT)) {
        pSoldier->uiStatusFlags &= (~SOLDIER_MULTI_SELECTED);
      }

      // Get screen pos of gridno......
      GetGridNoScreenXY(pSoldier->sGridNo, &sScreenX, &sScreenY);

      // ATE: If we are in a hiehger interface level, subttrasct....
      if (gsInterfaceLevel == 1) {
        sScreenY -= 50;
      }

      if (IsPointInScreenRect(sScreenX, sScreenY, &aRect)) {
        // Adjust this guy's flag...
        pSoldier->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
      }
    }
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleJumpOverOnTerrain(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;

  // Here, first get map screen
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (!IsValidJumpLocation(pSoldier, usMapPos, FALSE)) {
    guiPendingOverrideEvent = M_ON_TERRAIN;
    return (GAME_SCREEN);
  }

  // Display APs....
  gsCurrentActionPoints = GetAPsToJumpOver(pSoldier);

  gfUIDisplayActionPoints = TRUE;
  gfUIDisplayActionPointsCenter = TRUE;

  guiNewUICursor = JUMP_OVER_UICURSOR;

  return (GAME_SCREEN);
}

uint32_t UIHandleJumpOver(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  int8_t bDirection;

  // Here, first get map screen
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (GAME_SCREEN);
  }

  if (!GetMouseMapPos(&usMapPos)) {
    return (GAME_SCREEN);
  }

  if (!IsValidJumpLocation(pSoldier, usMapPos, FALSE)) {
    return (GAME_SCREEN);
  }

  SetUIBusy(pSoldier->ubID);

  // OK, Start jumping!
  // Remove any previous actions
  pSoldier->ubPendingAction = NO_PENDING_ACTION;

  // Get direction to goto....
  bDirection = (int8_t)GetDirectionFromGridNo(usMapPos, pSoldier);

  pSoldier->fDontChargeTurningAPs = TRUE;
  EVENT_InternalSetSoldierDesiredDirection(pSoldier, bDirection, FALSE, pSoldier->usAnimState);
  pSoldier->fTurningUntilDone = TRUE;
  // ATE: Reset flag to go back to prone...
  // pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
  pSoldier->usPendingAnimation = JUMP_OVER_BLOCKING_PERSON;

  return (GAME_SCREEN);
}

uint32_t UIHandleLABeginLockOurTurn(UI_EVENT *pUIEvent) {
  // Don't let both versions of the locks to happen at the same time!
  // ( They are mutually exclusive )!
  UIHandleLUIEndLock(NULL);

  if (!gfUserTurnRegionActive) {
    gfUserTurnRegionActive = TRUE;

    gfUIInterfaceSetBusy = TRUE;
    guiUIInterfaceBusyTime = GetJA2Clock();

    // guiNewUICursor = NO_UICURSOR;
    // SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

    MSYS_DefineRegion(&gUserTurnRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, CURSOR_WAIT,
                      MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
    // Add region
    MSYS_AddRegion(&gUserTurnRegion);

    // guiPendingOverrideEvent = LOCKOURTURN_UI_MODE;

    ErasePath(TRUE);

    // Pause time!
    PauseGame();
    LockPauseState(17);
  }

  return (GAME_SCREEN);
}

uint32_t UIHandleLAEndLockOurTurn(UI_EVENT *pUIEvent) {
  if (gfUserTurnRegionActive) {
    gfUserTurnRegionActive = FALSE;

    gfUIInterfaceSetBusy = FALSE;

    // Add region
    MSYS_RemoveRegion(&gUserTurnRegion);
    RefreshMouseRegions();
    // SetCurrentCursorFromDatabase( guiCurrentUICursor );

    gfPlotNewMovement = TRUE;

    guiForceRefreshMousePositionCalculation = TRUE;
    UIHandleMOnTerrain(NULL);

    if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
      SetCurrentCursorFromDatabase(gUICursors[guiNewUICursor].usFreeCursorName);
    }
    guiPendingOverrideEvent = M_ON_TERRAIN;
    HandleTacticalUI();

    TurnOffTeamsMuzzleFlashes(gbPlayerNum);

    // UnPause time!
    UnLockPauseState();
    UnPauseGame();
  }

  return (GAME_SCREEN);
}

BOOLEAN IsValidTalkableNPCFromMouse(uint8_t *pubSoldierID, BOOLEAN fGive, BOOLEAN fAllowMercs,
                                    BOOLEAN fCheckCollapsed) {
  // Check if there is a guy here to talk to!
  if (gfUIFullTargetFound) {
    *pubSoldierID = (uint8_t)gusUIFullTargetID;
    return (IsValidTalkableNPC((uint8_t)gusUIFullTargetID, fGive, fAllowMercs, fCheckCollapsed));
  }

  return (FALSE);
}

BOOLEAN IsValidTalkableNPC(uint8_t ubSoldierID, BOOLEAN fGive, BOOLEAN fAllowMercs,
                           BOOLEAN fCheckCollapsed) {
  struct SOLDIERTYPE *pSoldier = MercPtrs[ubSoldierID];
  BOOLEAN fValidGuy = FALSE;

  if (gusSelectedSoldier != NOBODY) {
    if (AM_A_ROBOT(MercPtrs[gusSelectedSoldier])) {
      return (FALSE);
    }
  }

  // CHECK IF ACTIVE!
  if (!IsSolActive(pSoldier)) {
    return (FALSE);
  }

  // CHECK IF DEAD
  if (pSoldier->bLife == 0) {
    return (FALSE);
  }

  if (pSoldier->bCollapsed && fCheckCollapsed) {
    return (FALSE);
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    return (FALSE);
  }

  // IF BAD GUY - CHECK VISIVILITY
  if (pSoldier->bTeam != gbPlayerNum) {
    if (pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
      return (FALSE);
    }
  }

  if (GetSolProfile(pSoldier) != NO_PROFILE && GetSolProfile(pSoldier) >= FIRST_RPC &&
      !RPC_RECRUITED(pSoldier) && !AM_AN_EPC(pSoldier)) {
    fValidGuy = TRUE;
  }

  // Check for EPC...
  if (GetSolProfile(pSoldier) != NO_PROFILE && (gCurrentUIMode == TALKCURSOR_MODE || fGive) &&
      AM_AN_EPC(pSoldier)) {
    fValidGuy = TRUE;
  }

  // ATE: We can talk to our own teammates....
  if (pSoldier->bTeam == gbPlayerNum && fAllowMercs) {
    fValidGuy = TRUE;
  }

  if (GetCivType(pSoldier) != CIV_TYPE_NA && !fGive) {
    fValidGuy = TRUE;
  }

  // Alright, let's do something special here for robot...
  if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
    if (fValidGuy == TRUE && !fGive) {
      // Can't talk to robots!
      fValidGuy = FALSE;
    }
  }

  // OK, check if they are stationary or not....
  // Do some checks common to all..
  if (fValidGuy) {
    if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) &&
        !(gTacticalStatus.uiFlags & INCOMBAT)) {
      return (FALSE);
    }

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN HandleTalkInit() {
  int16_t sAPCost;
  struct SOLDIERTYPE *pSoldier, *pTSoldier;
  uint32_t uiRange;
  int16_t usMapPos;
  int16_t sGoodGridNo;
  uint8_t ubNewDirection;
  uint8_t ubQuoteNum;
  uint8_t ubDiceRoll;
  int16_t sDistVisible;
  int16_t sActionGridNo;
  uint8_t ubDirection;

  // Get soldier
  if (!GetSoldier(&pSoldier, gusSelectedSoldier)) {
    return (FALSE);
  }

  if (!GetMouseMapPos(&usMapPos)) {
    return (FALSE);
  }

  // Check if there is a guy here to talk to!
  if (gfUIFullTargetFound) {
    // Is he a valid NPC?
    if (IsValidTalkableNPC((uint8_t)gusUIFullTargetID, FALSE, TRUE, FALSE)) {
      GetSoldier(&pTSoldier, gusUIFullTargetID);

      if (pTSoldier->ubID != GetSolID(pSoldier)) {
        // ATE: Check if we have good LOS
        // is he close enough to see that gridno if he turns his head?
        sDistVisible = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT,
                                       pTSoldier->sGridNo, pTSoldier->bLevel);

        // Check LOS!
        if (!SoldierTo3DLocationLineOfSightTest(pSoldier, pTSoldier->sGridNo, pTSoldier->bLevel, 3,
                                                (uint8_t)sDistVisible, TRUE)) {
          if (pTSoldier->ubProfile != NO_PROFILE) {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_LOS_TO_TALK_TARGET],
                      pSoldier->name, pTSoldier->name);
          } else {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[45],
                      pSoldier->name);
          }
          return (FALSE);
        }
      }

      if (pTSoldier->bCollapsed) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[21],
                  pTSoldier->name);
        return (FALSE);
      }

      // If Q on, turn off.....
      if (guiCurrentScreen == DEBUG_SCREEN) {
        gfExitDebugScreen = TRUE;
      }

      // ATE: if our own guy...
      if (pTSoldier->bTeam == gbPlayerNum && !AM_AN_EPC(pTSoldier)) {
        if (pTSoldier->ubProfile == DIMITRI) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[32],
                    pTSoldier->name);
          return (FALSE);
        }

        // Randomize quote to use....

        // If buddy had a social trait...
        if (gMercProfiles[pTSoldier->ubProfile].bAttitude != ATT_NORMAL) {
          ubDiceRoll = (uint8_t)Random(3);
        } else {
          ubDiceRoll = (uint8_t)Random(2);
        }

        // If we are a PC, only use 0
        if (pTSoldier->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER) {
          ubDiceRoll = 0;
        }

        switch (ubDiceRoll) {
          case 0:

            ubQuoteNum = QUOTE_NEGATIVE_COMPANY;
            break;

          case 1:

            if (QuoteExp_PassingDislike[pTSoldier->ubProfile]) {
              ubQuoteNum = QUOTE_PASSING_DISLIKE;
            } else {
              ubQuoteNum = QUOTE_NEGATIVE_COMPANY;
            }
            break;

          case 2:

            ubQuoteNum = QUOTE_SOCIAL_TRAIT;
            break;

          default:

            ubQuoteNum = QUOTE_NEGATIVE_COMPANY;
            break;
        }

        if (pTSoldier->ubProfile == IRA) {
          ubQuoteNum = QUOTE_PASSING_DISLIKE;
        }

        TacticalCharacterDialogue(pTSoldier, ubQuoteNum);

        return (FALSE);
      }

      // Check distance
      uiRange = GetRangeFromGridNoDiff(pSoldier->sGridNo, usMapPos);

      // Double check path
      if (GetCivType(pTSoldier) != CIV_TYPE_NA) {
        // ATE: If one is already active, just remove it!
        if (ShutDownQuoteBoxIfActive()) {
          return (FALSE);
        }
      }

      if (uiRange > NPC_TALK_RADIUS) {
        // First get an adjacent gridno....
        sActionGridNo =
            FindAdjacentGridEx(pSoldier, pTSoldier->sGridNo, &ubDirection, NULL, FALSE, TRUE);

        if (sActionGridNo == -1) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
          return (FALSE);
        }

        if (UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY,
                       (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                       pSoldier->bActionPoints) == 0) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
          return (FALSE);
        }

        // Walk up and talk to buddy....
        gfNPCCircularDistLimit = TRUE;
        sGoodGridNo = FindGridNoFromSweetSpotWithStructData(
            pSoldier, pSoldier->usUIMovementMode, pTSoldier->sGridNo, (NPC_TALK_RADIUS - 1),
            &ubNewDirection, TRUE);
        gfNPCCircularDistLimit = FALSE;

        // First calculate APs and validate...
        sAPCost = AP_TALK;
        // sAPCost += UIPlotPath( pSoldier, sGoodGridNo, NO_COPYROUTE, FALSE, TEMPORARY,
        // (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD, pSoldier->bActionPoints );

        // Check AP cost...
        if (!EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
          return (FALSE);
        }

        // Now walkup to talk....
        pSoldier->ubPendingAction = MERC_TALK;
        pSoldier->uiPendingActionData1 = pTSoldier->ubID;
        pSoldier->ubPendingActionAnimCount = 0;

        // WALK UP TO DEST FIRST
        EVENT_InternalGetNewSoldierPath(pSoldier, sGoodGridNo, pSoldier->usUIMovementMode, TRUE,
                                        pSoldier->fNoAPToFinishMove);

        return (FALSE);
      } else {
        sAPCost = AP_TALK;

        // Check AP cost...
        if (!EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
          return (FALSE);
        }

        // OK, startup!
        PlayerSoldierStartTalking(pSoldier, pTSoldier->ubID, FALSE);
      }

      if (GetCivType(pTSoldier) != CIV_TYPE_NA) {
        return (FALSE);
      } else {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

void SetUIBusy(uint8_t ubID) {
  if ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED) &&
      (gTacticalStatus.ubCurrentTeam == gbPlayerNum)) {
    if (gusSelectedSoldier == ubID) {
      guiPendingOverrideEvent = LA_BEGINUIOURTURNLOCK;
      HandleTacticalUI();
    }
  }
}

void UnSetUIBusy(uint8_t ubID) {
  if ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED) &&
      (gTacticalStatus.ubCurrentTeam == gbPlayerNum)) {
    if (!gTacticalStatus.fUnLockUIAfterHiddenInterrupt) {
      if (gusSelectedSoldier == ubID) {
        guiPendingOverrideEvent = LA_ENDUIOUTURNLOCK;
        HandleTacticalUI();

        // Set grace period...
        gTacticalStatus.uiTactialTurnLimitClock = GetJA2Clock();
      }
    }
    // player getting control back so reset all muzzle flashes
  }
}

void BeginDisplayTimedCursor(uint32_t uiCursorID, uint32_t uiDelay) {
  gfDisplayTimerCursor = TRUE;
  guiTimerCursorID = uiCursorID;
  guiTimerLastUpdate = GetJA2Clock();
  guiTimerCursorDelay = uiDelay;
}

int8_t UIHandleInteractiveTilesAndItemsOnTerrain(struct SOLDIERTYPE *pSoldier, int16_t usMapPos,
                                                 BOOLEAN fUseOKCursor,
                                                 BOOLEAN fItemsOnlyIfOnIntTiles) {
  struct ITEM_POOL *pItemPool;
  uint32_t uiCursorFlags;
  struct LEVELNODE *pIntTile;
  static BOOLEAN fOverPool = FALSE;
  static BOOLEAN fOverEnemy = FALSE;
  int16_t sActionGridNo;
  int16_t sIntTileGridNo;
  BOOLEAN fContinue = TRUE;
  struct STRUCTURE *pStructure = NULL;
  BOOLEAN fPoolContainsHiddenItems = FALSE;
  struct SOLDIERTYPE *pTSoldier;

  if (gfResetUIItemCursorOptimization) {
    gfResetUIItemCursorOptimization = FALSE;
    fOverPool = FALSE;
    fOverEnemy = FALSE;
  }

  GetCursorMovementFlags(&uiCursorFlags);

  // Default gridno to mouse pos
  sActionGridNo = usMapPos;

  // Look for being on a merc....
  // Steal.....
  UIHandleOnMerc(FALSE);

  gfBeginVehicleCursor = FALSE;

  if (gfUIFullTargetFound) {
    pTSoldier = MercPtrs[gusUIFullTargetID];

    if (OK_ENTERABLE_VEHICLE(pTSoldier) && pTSoldier->bVisible != -1) {
      // grab number of occupants in vehicles
      if (fItemsOnlyIfOnIntTiles) {
        if (!OKUseVehicle(pTSoldier->ubProfile)) {
          // Set UI CURSOR....
          guiNewUICursor = CANNOT_MOVE_UICURSOR;

          gfBeginVehicleCursor = TRUE;
          return (1);
        } else {
          if (GetNumberInVehicle(pTSoldier->bVehicleID) == 0) {
            // Set UI CURSOR....
            guiNewUICursor = ENTER_VEHICLE_UICURSOR;

            gfBeginVehicleCursor = TRUE;
            return (1);
          }
        }
      } else {
        // Set UI CURSOR....
        guiNewUICursor = ENTER_VEHICLE_UICURSOR;
        return (1);
      }
    }

    if (!fItemsOnlyIfOnIntTiles) {
      if ((guiUIFullTargetFlags & ENEMY_MERC) && !(guiUIFullTargetFlags & UNCONSCIOUS_MERC)) {
        if (!fOverEnemy) {
          fOverEnemy = TRUE;
          gfPlotNewMovement = TRUE;
        }

        // Set UI CURSOR
        if (fUseOKCursor ||
            ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED))) {
          guiNewUICursor = OKHANDCURSOR_UICURSOR;
        } else {
          guiNewUICursor = NORMALHANDCURSOR_UICURSOR;
        }

        HandleUIMovementCursor(pSoldier, uiCursorFlags, sActionGridNo, MOVEUI_TARGET_STEAL);

        // Display action points
        gfUIDisplayActionPoints = TRUE;

        // Determine if we can afford!
        if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
          gfUIDisplayActionPointsInvalid = TRUE;
        }

        return (0);
      }
    }
  }

  if (fOverEnemy) {
    ErasePath(TRUE);
    fOverEnemy = FALSE;
    gfPlotNewMovement = TRUE;
  }

  // If we are over an interactive struct, adjust gridno to this....
  pIntTile =
      ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure, FALSE);
  gpInvTileThatCausedMoveConfirm = pIntTile;

  if (pIntTile != NULL) {
    sActionGridNo = sIntTileGridNo;
  }

  // Check if we are over an item pool
  if (GetItemPool(sActionGridNo, &pItemPool, pSoldier->bLevel)) {
    // If we want only on int tiles, and we have no int tiles.. ignore items!
    if (fItemsOnlyIfOnIntTiles && pIntTile == NULL) {
    } else if (fItemsOnlyIfOnIntTiles && pIntTile != NULL &&
               (pStructure->fFlags & STRUCTURE_HASITEMONTOP)) {
      // if in this mode, we don't want to automatically show hand cursor over items on strucutres
    }
    // else if ( pIntTile != NULL && ( pStructure->fFlags & ( STRUCTURE_SWITCH | STRUCTURE_ANYDOOR )
    // ) )
    else if (pIntTile != NULL && (pStructure->fFlags & (STRUCTURE_SWITCH))) {
      // We don't want switches messing around with items ever!
    } else if ((pIntTile != NULL && (pStructure->fFlags & (STRUCTURE_ANYDOOR))) &&
               (sActionGridNo != usMapPos || fItemsOnlyIfOnIntTiles)) {
      // Next we look for if we are over a door and if the mouse position is != base door position,
      // ignore items!
    } else {
      fPoolContainsHiddenItems = DoesItemPoolContainAnyHiddenItems(pItemPool);

      // Adjust this if we have not visited this gridno yet...
      if (fPoolContainsHiddenItems) {
        if (!(gpWorldLevelData[sActionGridNo].uiFlags & MAPELEMENT_REVEALED)) {
          fPoolContainsHiddenItems = FALSE;
        }
      }

      if (ITEMPOOL_VISIBLE(pItemPool) || fPoolContainsHiddenItems) {
        if (!fOverPool) {
          fOverPool = TRUE;
          gfPlotNewMovement = TRUE;
        }

        // Set UI CURSOR
        if (fUseOKCursor ||
            ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED))) {
          guiNewUICursor = OKHANDCURSOR_UICURSOR;
        } else {
          guiNewUICursor = NORMALHANDCURSOR_UICURSOR;
        }

        HandleUIMovementCursor(pSoldier, uiCursorFlags, sActionGridNo, MOVEUI_TARGET_ITEMS);

        // Display action points
        gfUIDisplayActionPoints = TRUE;

        if (gsOverItemsGridNo == sActionGridNo) {
          gfPlotNewMovement = TRUE;
        }

        // Determine if we can afford!
        if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
          gfUIDisplayActionPointsInvalid = TRUE;
        }

        fContinue = FALSE;
      }
    }
  }

  if (fContinue) {
    // Try interactive tiles now....
    if (pIntTile != NULL) {
      if (fOverPool) {
        ErasePath(TRUE);
        fOverPool = FALSE;
        gfPlotNewMovement = TRUE;
      }

      HandleUIMovementCursor(pSoldier, uiCursorFlags, usMapPos, MOVEUI_TARGET_INTTILES);

      // Set UI CURSOR
      guiNewUICursor = GetInteractiveTileCursor(guiNewUICursor, fUseOKCursor);
    } else {
      if (!fItemsOnlyIfOnIntTiles) {
        // Let's at least show where the merc will walk to if they go here...
        if (!fOverPool) {
          fOverPool = TRUE;
          gfPlotNewMovement = TRUE;
        }

        HandleUIMovementCursor(pSoldier, uiCursorFlags, sActionGridNo, MOVEUI_TARGET_ITEMS);

        // Display action points
        gfUIDisplayActionPoints = TRUE;

        // Determine if we can afford!
        if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE)) {
          gfUIDisplayActionPointsInvalid = TRUE;
        }
      }
    }
  }

  if (pIntTile == NULL) {
    return (0);
  } else {
    return (1);
  }
}

void HandleTacticalUILoseCursorFromOtherScreen() {
  SetUICursor(0);

  gfTacticalForceNoCursor = TRUE;

  ErasePath(TRUE);

  (*(GameScreens[GAME_SCREEN].HandleScreen))();

  gfTacticalForceNoCursor = FALSE;

  SetUICursor(guiCurrentUICursor);
}

BOOLEAN SelectedGuyInBusyAnimation() {
  struct SOLDIERTYPE *pSoldier;

  if (gusSelectedSoldier != NOBODY) {
    pSoldier = MercPtrs[gusSelectedSoldier];

    if (pSoldier->usAnimState == LOB_ITEM || pSoldier->usAnimState == THROW_ITEM ||
        pSoldier->usAnimState == PICKUP_ITEM || pSoldier->usAnimState == DROP_ITEM ||
        pSoldier->usAnimState == OPEN_DOOR || pSoldier->usAnimState == OPEN_STRUCT ||
        pSoldier->usAnimState == OPEN_STRUCT || pSoldier->usAnimState == END_OPEN_DOOR ||
        pSoldier->usAnimState == END_OPEN_LOCKED_DOOR ||
        pSoldier->usAnimState == ADJACENT_GET_ITEM ||
        pSoldier->usAnimState == DROP_ADJACENT_OBJECT ||

        pSoldier->usAnimState == OPEN_DOOR_CROUCHED ||
        pSoldier->usAnimState == BEGIN_OPENSTRUCT_CROUCHED ||
        pSoldier->usAnimState == CLOSE_DOOR_CROUCHED ||
        pSoldier->usAnimState == OPEN_DOOR_CROUCHED ||
        pSoldier->usAnimState == OPEN_STRUCT_CROUCHED ||
        pSoldier->usAnimState == END_OPENSTRUCT_CROUCHED ||
        pSoldier->usAnimState == END_OPEN_DOOR_CROUCHED ||
        pSoldier->usAnimState == END_OPEN_LOCKED_DOOR_CROUCHED ||
        pSoldier->usAnimState == END_OPENSTRUCT_LOCKED_CROUCHED ||
        pSoldier->usAnimState == BEGIN_OPENSTRUCT) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void GotoHeigherStance(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fNearHeigherLevel;
  BOOLEAN fNearLowerLevel;

  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:

      // Nowhere
      // Try to climb
      GetMercClimbDirection(pSoldier->ubID, &fNearLowerLevel, &fNearHeigherLevel);

      if (fNearHeigherLevel) {
        BeginSoldierClimbUpRoof(pSoldier);
      }
      break;

    case ANIM_CROUCH:

      HandleStanceChangeFromUIKeys(ANIM_STAND);
      break;

    case ANIM_PRONE:

      HandleStanceChangeFromUIKeys(ANIM_CROUCH);
      break;
  }
}

void GotoLowerStance(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fNearHeigherLevel;
  BOOLEAN fNearLowerLevel;

  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:

      HandleStanceChangeFromUIKeys(ANIM_CROUCH);
      break;

    case ANIM_CROUCH:

      HandleStanceChangeFromUIKeys(ANIM_PRONE);
      break;

    case ANIM_PRONE:

      // Nowhere
      // Try to climb
      GetMercClimbDirection(pSoldier->ubID, &fNearLowerLevel, &fNearHeigherLevel);

      if (fNearLowerLevel) {
        BeginSoldierClimbDownRoof(pSoldier);
      }
      break;
  }
}

void SetInterfaceHeightLevel() {
  int16_t sHeight;
  static int16_t sOldHeight = 0;
  int16_t sGridNo;

  if (gfBasement || gfCaves) {
    gsRenderHeight = 0;
    sOldHeight = 0;

    return;
  }

  // ATE: Use an entry point to determine what height to use....
  if (gMapInformation.sNorthGridNo != -1)
    sGridNo = gMapInformation.sNorthGridNo;
  else if (gMapInformation.sEastGridNo != -1)
    sGridNo = gMapInformation.sEastGridNo;
  else if (gMapInformation.sSouthGridNo != -1)
    sGridNo = gMapInformation.sSouthGridNo;
  else if (gMapInformation.sWestGridNo != -1)
    sGridNo = gMapInformation.sWestGridNo;
  else {
    Assert(0);
    return;
  }

  sHeight = gpWorldLevelData[sGridNo].sHeight;

  if (sHeight != sOldHeight) {
    gsRenderHeight = sHeight;

    if (gsInterfaceLevel > 0) {
      gsRenderHeight += ROOF_LEVEL_HEIGHT;
    }

    SetRenderFlags(RENDER_FLAG_FULL);
    ErasePath(FALSE);

    sOldHeight = sHeight;
  }
}

BOOLEAN ValidQuickExchangePosition() {
  struct SOLDIERTYPE *pSoldier, *pOverSoldier;
  int16_t sDistVisible = FALSE;
  BOOLEAN fOnValidGuy = FALSE;
  static BOOLEAN fOldOnValidGuy = FALSE;

  // Check if we over a civ
  if (gfUIFullTargetFound) {
    pOverSoldier = MercPtrs[gusUIFullTargetID];

    // KM: Replaced this older if statement for the new one which allows exchanging with militia
    // if ( ( pOverSoldier->bSide != gbPlayerNum ) && pOverSoldier->bNeutral  )
    if ((pOverSoldier->bTeam != gbPlayerNum && pOverSoldier->bNeutral) ||
        (pOverSoldier->bTeam == MILITIA_TEAM && pOverSoldier->bSide == 0)) {
      // hehe - don't allow animals to exchange places
      if (!(pOverSoldier->uiStatusFlags & (SOLDIER_ANIMAL))) {
        // OK, we have a civ , now check if they are near selected guy.....
        if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
          if (PythSpacesAway(pSoldier->sGridNo, pOverSoldier->sGridNo) == 1) {
            // Check if we have LOS to them....
            sDistVisible = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT,
                                           pOverSoldier->sGridNo, pOverSoldier->bLevel);

            if (SoldierTo3DLocationLineOfSightTest(pSoldier, pOverSoldier->sGridNo,
                                                   pOverSoldier->bLevel, (uint8_t)3,
                                                   (uint8_t)sDistVisible, TRUE)) {
              // ATE:
              // Check that the path is good!
              if (FindBestPath(pSoldier, pOverSoldier->sGridNo, pSoldier->bLevel,
                               pSoldier->usUIMovementMode, NO_COPYROUTE,
                               PATH_IGNORE_PERSON_AT_DEST) == 1) {
                fOnValidGuy = TRUE;
              }
            }
          }
        }
      }
    }
  }

  if (fOldOnValidGuy != fOnValidGuy) {
    // Update timer....
    // ATE: Adjust clock for automatic swapping so that the 'feel' is there....
    guiUIInterfaceSwapCursorsTime = GetJA2Clock();
    // Default it!
    gfOKForExchangeCursor = TRUE;
  }

  // Update old value.....
  fOldOnValidGuy = fOnValidGuy;

  if (!gfOKForExchangeCursor) {
    fOnValidGuy = FALSE;
  }

  return (fOnValidGuy);
}

// This function contains the logic for allowing the player
// to jump over people.
BOOLEAN IsValidJumpLocation(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, BOOLEAN fCheckForPath) {
  int16_t sSpot, sIntSpot;
  int16_t sDirs[4] = {NORTH, EAST, SOUTH, WEST};
  int32_t cnt;
  uint8_t ubGuyThere;
  uint8_t ubMovementCost;
  int32_t iDoorGridNo;

  // First check that action point cost is zero so far
  // ie: NO PATH!
  if (gsCurrentActionPoints != 0 && fCheckForPath) {
    return (FALSE);
  }

  // Loop through positions...
  for (cnt = 0; cnt < 4; cnt++) {
    // MOVE OUT TWO DIRECTIONS
    sIntSpot = NewGridNo(sGridNo, DirectionInc(sDirs[cnt]));

    // ATE: Check our movement costs for going through walls!
    ubMovementCost = gubWorldMovementCosts[sIntSpot][sDirs[cnt]][pSoldier->bLevel];
    if (IS_TRAVELCOST_DOOR(ubMovementCost)) {
      ubMovementCost = DoorTravelCost(pSoldier, sIntSpot, ubMovementCost,
                                      (BOOLEAN)(pSoldier->bTeam == gbPlayerNum), &iDoorGridNo);
    }

    // If we have hit an obstacle, STOP HERE
    if (ubMovementCost >= TRAVELCOST_BLOCKED) {
      // no good, continue
      continue;
    }

    // TWICE AS FAR!
    sSpot = NewGridNo(sIntSpot, DirectionInc(sDirs[cnt]));

    // Is the soldier we're looking at here?
    ubGuyThere = WhoIsThere2(sSpot, pSoldier->bLevel);

    // Alright folks, here we are!
    if (ubGuyThere == GetSolID(pSoldier)) {
      // Double check OK destination......
      if (NewOKDestination(pSoldier, sGridNo, TRUE, (int8_t)gsInterfaceLevel)) {
        // If the soldier in the middle of doing stuff?
        if (!pSoldier->fTurningUntilDone) {
          // OK, NOW check if there is a guy in between us
          //
          //
          ubGuyThere = WhoIsThere2(sIntSpot, pSoldier->bLevel);

          // Is there a guy and is he prone?
          if (ubGuyThere != NOBODY && ubGuyThere != GetSolID(pSoldier) &&
              gAnimControl[MercPtrs[ubGuyThere]->usAnimState].ubHeight == ANIM_PRONE) {
            // It's a GO!
            return (TRUE);
          }
        }
      }
    }
  }

  return (FALSE);
}
