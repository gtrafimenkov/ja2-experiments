// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/Interface.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "GameLoop.h"
#include "GameSettings.h"
#include "Globals.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/Faces.h"
#include "Tactical/HandleDoors.h"
#include "Tactical/HandleUI.h"
#include "Tactical/HandleUIPlan.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Keys.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierFunctions.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/Vehicles.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"

#define ARROWS_X_OFFSET 10
#define ARROWS_HEIGHT 20
#define ARROWS_WIDTH 20
#define UPARROW_Y_OFFSET -30
#define DOWNARROW_Y_OFFSET -10

#define BUTTON_PANEL_WIDTH 78
#define BUTTON_PANEL_HEIGHT 76

struct MOUSE_REGION gBottomPanalRegion;
BOOLEAN gfInMovementMenu = FALSE;
int32_t giMenuAnchorX, giMenuAnchorY;

#define PROG_BAR_START_X 5
#define PROG_BAR_START_Y 2
#define PROG_BAR_LENGTH 627

BOOLEAN gfProgBarActive = FALSE;
uint8_t gubProgNumEnemies = 0;
uint8_t gubProgCurEnemy = 0;

typedef struct {
  struct JSurface *dest;
  int8_t bCurrentMessage;
  uint32_t uiTimeOfLastUpdate;
  uint32_t uiTimeSinceLastBeep;
  int8_t bAnimate;
  int8_t bYPos;
  BOOLEAN fCreated;
  int16_t sWorldRenderX;
  int16_t sWorldRenderY;

} TOP_MESSAGE;

static TOP_MESSAGE gTopMessage;
BOOLEAN gfTopMessageDirty = FALSE;

void CreateTopMessage(struct JSurface *dest, uint8_t ubType, wchar_t *psString);
extern uint16_t GetAnimStateForInteraction(struct SOLDIERTYPE *pSoldier, BOOLEAN fDoor,
                                           uint16_t usAnimState);

struct MOUSE_REGION gMenuOverlayRegion;

uint16_t gusOldSelectedSoldier = NO_SOLDIER;

// OVerlay ID
int32_t giPopupSlideMessageOverlay = -1;
uint16_t gusOverlayPopupBoxWidth, gusOverlayPopupBoxHeight;
MercPopUpBox gpOverrideMercBox;

int32_t giUIMessageOverlay = -1;
uint16_t gusUIMessageWidth, gusUIMessageHeight;
MercPopUpBox gpUIMessageOverrideMercBox;
uint32_t guiUIMessageTime = 0;
int32_t iOverlayMessageBox = -1;
int32_t iUIMessageBox = -1;
uint32_t guiUIMessageTimeDelay = 0;
BOOLEAN gfUseSkullIconMessage = FALSE;

// Overlay callback
void BlitPopupText(VIDEO_OVERLAY *pBlitter);

BOOLEAN gfPanelAllocated = FALSE;

extern struct MOUSE_REGION gDisableRegion;
extern struct MOUSE_REGION gUserTurnRegion;
extern BOOLEAN gfUserTurnRegionActive;
extern uint8_t gubSelectSMPanelToMerc;
extern BOOLEAN gfIgnoreOnSelectedGuy;

enum {
  WALK_IMAGES = 0,
  SNEAK_IMAGES,
  RUN_IMAGES,
  CRAWL_IMAGES,
  LOOK_IMAGES,
  TALK_IMAGES,
  HAND_IMAGES,
  CANCEL_IMAGES,

  TARGETACTIONC_IMAGES,
  KNIFEACTIONC_IMAGES,
  AIDACTIONC_IMAGES,
  PUNCHACTIONC_IMAGES,
  BOMBACTIONC_IMAGES,

  OPEN_DOOR_IMAGES,
  EXAMINE_DOOR_IMAGES,
  LOCKPICK_DOOR_IMAGES,
  BOOT_DOOR_IMAGES,
  CROWBAR_DOOR_IMAGES,
  USE_KEY_IMAGES,
  USE_KEYRING_IMAGES,
  EXPLOSIVE_DOOR_IMAGES,

  TOOLKITACTIONC_IMAGES,
  WIRECUTACTIONC_IMAGES,

  NUM_ICON_IMAGES
};

int32_t iIconImages[NUM_ICON_IMAGES];

enum {
  WALK_ICON,
  SNEAK_ICON,
  RUN_ICON,
  CRAWL_ICON,
  LOOK_ICON,
  ACTIONC_ICON,
  TALK_ICON,
  HAND_ICON,

  OPEN_DOOR_ICON,
  EXAMINE_DOOR_ICON,
  LOCKPICK_DOOR_ICON,
  BOOT_DOOR_ICON,
  UNTRAP_DOOR_ICON,
  USE_KEY_ICON,
  USE_KEYRING_ICON,
  EXPLOSIVE_DOOR_ICON,
  USE_CROWBAR_ICON,

  CANCEL_ICON,
  NUM_ICONS
};

int32_t iActionIcons[NUM_ICONS];

// GLOBAL INTERFACE SURFACES
struct JSurface *vsINTEXT;
uint32_t guiCLOSE;
uint32_t guiDEAD;
uint32_t guiHATCH;
uint32_t guiGUNSM;
uint32_t guiP1ITEMS;
uint32_t guiP2ITEMS;
uint32_t guiP3ITEMS;
uint32_t guiBUTTONBORDER;
uint32_t guiRADIO;
uint32_t guiRADIO2;
uint32_t guiCOMPANEL;
uint32_t guiCOMPANELB;
uint32_t guiAIMCUBES;
uint32_t guiAIMBARS;
uint32_t guiVEHINV;
uint32_t guiBURSTACCUM;
uint32_t guiITEMPOINTERHATCHES;

// UI Globals
struct MOUSE_REGION gViewportRegion;
struct MOUSE_REGION gRadarRegion;

uint16_t gsUpArrowX;
uint16_t gsUpArrowY;
uint16_t gsDownArrowX;
uint16_t gsDownArrowY;

uint32_t giUpArrowRect;
uint32_t giDownArrowRect;

void DrawBarsInUIBox(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                     int16_t sHeight);
void PopupDoorOpenMenu(BOOLEAN fClosingDoor);

BOOLEAN fInterfacePanelDirty = DIRTYLEVEL2;
int16_t gsInterfaceLevel = I_GROUND_LEVEL;
int16_t gsCurrentSoldierGridNo = 0;
int16_t gsCurInterfacePanel = TEAM_PANEL;

// LOCAL FUCTIONS
void BtnPositionCallback(GUI_BUTTON *btn, int32_t reason);
void BtnMovementCallback(GUI_BUTTON *btn, int32_t reason);
void BtnDoorMenuCallback(GUI_BUTTON *btn, int32_t reason);
void MovementMenuBackregionCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void DoorMenuBackregionCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

uint32_t CalcUIMessageDuration(wchar_t *wString);

BOOLEAN InitializeTacticalInterface() {
  // Load button Interfaces
  iIconImages[WALK_IMAGES] = LoadButtonImage("INTERFACE\\newicons3.sti", -1, 3, 4, 5, -1);
  iIconImages[SNEAK_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 6, 7, 8, -1);
  iIconImages[RUN_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 0, 1, 2, -1);
  iIconImages[CRAWL_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 9, 10, 11, -1);
  iIconImages[LOOK_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 12, 13, 14, -1);
  iIconImages[TALK_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 21, 22, 23, -1);
  iIconImages[HAND_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 18, 19, 20, -1);
  iIconImages[CANCEL_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 15, 16, 17, -1);

  iIconImages[TARGETACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 24, 25, 26, -1);
  iIconImages[KNIFEACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 27, 28, 29, -1);
  iIconImages[AIDACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 30, 31, 32, -1);
  iIconImages[PUNCHACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 33, 34, 35, -1);
  iIconImages[BOMBACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 36, 37, 38, -1);
  iIconImages[TOOLKITACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 39, 40, 41, -1);
  iIconImages[WIRECUTACTIONC_IMAGES] =
      UseLoadedButtonImage(iIconImages[WALK_IMAGES], -1, 42, 43, 44, -1);

  iIconImages[OPEN_DOOR_IMAGES] = LoadButtonImage("INTERFACE\\door_op2.sti", -1, 9, 10, 11, -1);
  iIconImages[EXAMINE_DOOR_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 12, 13, 14, -1);
  iIconImages[LOCKPICK_DOOR_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 21, 22, 23, -1);
  iIconImages[BOOT_DOOR_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 25, 26, 27, -1);
  iIconImages[CROWBAR_DOOR_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 0, 1, 2, -1);
  iIconImages[USE_KEY_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 3, 4, 5, -1);
  iIconImages[USE_KEYRING_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 6, 7, 8, -1);
  iIconImages[EXPLOSIVE_DOOR_IMAGES] =
      UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES], -1, 15, 16, 17, -1);

  // Load interface panels
  vsINTEXT = CreateVSurfaceFromFile("INTERFACE\\IN_TEXT.STI");
  if (vsINTEXT == NULL) {
    AssertMsg(0, "Missing INTERFACE\\In_text.sti");
  }
  JSurface_SetColorKey(vsINTEXT, FROMRGB(255, 0, 0));

  // LOAD CLOSE ANIM
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\p_close.sti"), &guiCLOSE))
    AssertMsg(0, "Missing INTERFACE\\p_close.sti");

  // LOAD DEAD ANIM
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\p_dead.sti"), &guiDEAD))
    AssertMsg(0, "Missing INTERFACE\\p_dead.sti");

  // LOAD HATCH
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\hatch.sti"), &guiHATCH))
    AssertMsg(0, "Missing INTERFACE\\hatch.sti");

  // LOAD INTERFACE GUN PICTURES
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\mdguns.sti"), &guiGUNSM))
    AssertMsg(0, "Missing INTERFACE\\mdguns.sti");

  // LOAD INTERFACE ITEM PICTURES
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\mdp1items.sti"), &guiP1ITEMS))
    AssertMsg(0, "Missing INTERFACE\\mdplitems.sti");

  // LOAD INTERFACE ITEM PICTURES
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\mdp2items.sti"), &guiP2ITEMS))
    AssertMsg(0, "Missing INTERFACE\\mdp2items.sti");

  // LOAD INTERFACE ITEM PICTURES
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\mdp3items.sti"), &guiP3ITEMS))
    AssertMsg(0, "Missing INTERFACE\\mdp3items.sti");

  // LOAD INTERFACE BUTTON BORDER
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\button_frame.sti"), &guiBUTTONBORDER))
    AssertMsg(0, "Missing INTERFACE\\button_frame.sti");

  // LOAD AIM CUBES
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\aimcubes.sti"), &guiAIMCUBES))
    AssertMsg(0, "Missing INTERFACE\\aimcubes.sti");

  // LOAD AIM BARS
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\aimbars.sti"), &guiAIMBARS))
    AssertMsg(0, "Missing INTERFACE\\aimbars.sti");

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\inventor.sti"), &guiVEHINV))
    AssertMsg(0, "Missing INTERFACE\\inventor.sti");

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\burst1.sti"), &guiBURSTACCUM))
    AssertMsg(0, "Missing INTERFACE\\burst1.sti");

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\portraiticons.sti"), &guiPORTRAITICONS))
    AssertMsg(0, "Missing INTERFACE\\portraiticons.sti");

  // LOAD RADIO
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\radio.sti"), &guiRADIO))
    //	AssertMsg(0, "Missing INTERFACE\\bracket.sti" );
    AssertMsg(0, "Missing INTERFACE\\radio.sti");

  // LOAD RADIO2
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\radio2.sti"), &guiRADIO2))
    AssertMsg(0, "Missing INTERFACE\\radio2.sti");

  // LOAD com panel 2
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\communicationpopup.sti"), &guiCOMPANEL))
    AssertMsg(0, "Missing INTERFACE\\communicationpopup.sti");

  // LOAD ITEM GRIDS....
  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\itemgrid.sti"), &guiITEMPOINTERHATCHES))
    AssertMsg(0, "Missing INTERFACE\\itemgrid.sti");

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\communicationpopup_2.sti"), &guiCOMPANELB))
    AssertMsg(0, "Missing INTERFACE\\communicationpopup_2.sti");

  // Alocate message surfaces
  gTopMessage.dest = JSurface_Create16bpp(640, 20);
  if (gTopMessage.dest == NULL) {
    return FALSE;
  }

  InitItemInterface();
  InitRadarScreen();
  InitTEAMSlots();

  return (TRUE);
}

BOOLEAN ShutdownTacticalInterface() {
  ShutdownCurrentPanel();

  return (TRUE);
}

BOOLEAN InitializeCurrentPanel() {
  BOOLEAN fOK = FALSE;

  MoveRadarScreen();

  switch (gsCurInterfacePanel) {
    case SM_PANEL:
      // Set new viewport
      gsVIEWPORT_WINDOW_END_Y = 340;

      // Render full
      SetRenderFlags(RENDER_FLAG_FULL);
      fOK = InitializeSMPanel();
      break;

    case TEAM_PANEL:
      gsVIEWPORT_WINDOW_END_Y = 360;
      // Render full
      SetRenderFlags(RENDER_FLAG_FULL);
      fOK = InitializeTEAMPanel();
      break;
  }

  //	RefreshMouseRegions( );
  gfPanelAllocated = TRUE;

  return (fOK);
}

void ShutdownCurrentPanel() {
  if (gfPanelAllocated) {
    switch (gsCurInterfacePanel) {
      case SM_PANEL:
        ShutdownSMPanel();
        break;

      case TEAM_PANEL:
        ShutdownTEAMPanel();
        break;
    }

    gfPanelAllocated = FALSE;
  }
}

void SetCurrentTacticalPanelCurrentMerc(uint8_t ubID) {
  struct SOLDIERTYPE *pSoldier;

  // Disable faces
  SetAllAutoFacesInactive();

  if (gsCurInterfacePanel == SM_PANEL) {
    // If we are not of merc bodytype, or am an epc, and going into inv, goto another....
    pSoldier = MercPtrs[ubID];

    if (!IS_MERC_BODY_TYPE(pSoldier) || AM_AN_EPC(pSoldier)) {
      SetCurrentInterfacePanel(TEAM_PANEL);
    }
  }

  switch (gsCurInterfacePanel) {
    case SM_PANEL:
      // SetSMPanelCurrentMerc( ubID );
      gubSelectSMPanelToMerc = ubID;
      break;

    case TEAM_PANEL:
      SetTEAMPanelCurrentMerc((uint8_t)gusSelectedSoldier);
      break;
  }
}

void CreateCurrentTacticalPanelButtons() {
  switch (gsCurInterfacePanel) {
    case SM_PANEL:
      CreateSMPanelButtons();
      break;

    case TEAM_PANEL:
      CreateTEAMPanelButtons();
      break;
  }
}

void SetCurrentInterfacePanel(uint8_t ubNewPanel) {
  ShutdownCurrentPanel();

  // INit new panel
  gsCurInterfacePanel = ubNewPanel;

  InitializeCurrentPanel();
}

void ToggleTacticalPanels() {
  gfSwitchPanel = TRUE;
  gubNewPanelParam = (uint8_t)gusSelectedSoldier;

  if (gsCurInterfacePanel == SM_PANEL) {
    gbNewPanel = TEAM_PANEL;
  } else {
    gbNewPanel = SM_PANEL;
  }
}

void RemoveCurrentTacticalPanelButtons() {
  switch (gsCurInterfacePanel) {
    case SM_PANEL:
      RemoveSMPanelButtons();
      break;

    case TEAM_PANEL:
      RemoveTEAMPanelButtons();
      break;
  }
}

BOOLEAN IsMercPortraitVisible(uint8_t ubSoldierID) {
  if (gsCurInterfacePanel == TEAM_PANEL) {
    return (TRUE);
  }

  if (gsCurInterfacePanel == SM_PANEL) {
    if (GetSMPanelCurrentMerc() == ubSoldierID) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void HandleInterfaceBackgrounds() { HandleUpDownArrowBackgrounds(); }

void PopupPositionMenu(UI_EVENT *pUIEvent) {}

void PopDownPositionMenu() {}

void BtnPositionCallback(GUI_BUTTON *btn, int32_t reason) {}

void PopupMovementMenu(UI_EVENT *pUIEvent) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iMenuAnchorX, iMenuAnchorY;
  uint32_t uiActionImages;
  wchar_t zActionString[50];
  BOOLEAN fDisableAction = FALSE;

  // Erase other menus....
  EraseInterfaceMenus(TRUE);

  giMenuAnchorX = gusMouseXPos - 18;
  giMenuAnchorY = gusMouseYPos - 18;

  // ATE: OK loser, let's check if we're going off the screen!
  if (giMenuAnchorX < 0) {
    giMenuAnchorX = 0;
  }

  if (giMenuAnchorY < 0) {
    giMenuAnchorY = 0;
  }

  // Create mouse region over all area to facilitate clicking to end
  MSYS_DefineRegion(&gMenuOverlayRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL,
                    MSYS_NO_CALLBACK, MovementMenuBackregionCallback);
  // Add region
  MSYS_AddRegion(&gMenuOverlayRegion);

  // OK, CHECK FOR BOUNDARIES!
  if ((giMenuAnchorX + BUTTON_PANEL_WIDTH) > 640) {
    giMenuAnchorX = (640 - BUTTON_PANEL_WIDTH);
  }
  if ((giMenuAnchorY + BUTTON_PANEL_HEIGHT) > gsVIEWPORT_WINDOW_END_Y) {
    giMenuAnchorY = (gsVIEWPORT_WINDOW_END_Y - BUTTON_PANEL_HEIGHT);
  }

  if (gusSelectedSoldier != NOBODY) {
    pSoldier = MercPtrs[gusSelectedSoldier];
  }

  iMenuAnchorX = giMenuAnchorX + 9;
  iMenuAnchorY = giMenuAnchorY + 8;

  iActionIcons[RUN_ICON] =
      QuickCreateButton(iIconImages[RUN_IMAGES], (int16_t)(iMenuAnchorX + 20),
                        (int16_t)(iMenuAnchorY), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[RUN_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  SetButtonFastHelpText(iActionIcons[RUN_ICON], pTacticalPopupButtonStrings[RUN_ICON]);
  // SetButtonSavedRect( iActionIcons[ RUN_ICON ] );
  ButtonList[iActionIcons[RUN_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  if (MercInWater(pSoldier) || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) ||
      (pSoldier->uiStatusFlags & SOLDIER_ROBOT)) {
    DisableButton(iActionIcons[RUN_ICON]);
  }

  iActionIcons[WALK_ICON] =
      QuickCreateButton(iIconImages[WALK_IMAGES], (int16_t)(iMenuAnchorX + 40),
                        (int16_t)(iMenuAnchorY), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[WALK_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  // SetButtonSavedRect( iActionIcons[ WALK_ICON ] );

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    SetButtonFastHelpText(iActionIcons[WALK_ICON], TacticalStr[DRIVE_POPUPTEXT]);
  } else {
    SetButtonFastHelpText(iActionIcons[WALK_ICON], pTacticalPopupButtonStrings[WALK_ICON]);
  }

  ButtonList[iActionIcons[WALK_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
    if (!CanRobotBeControlled(pSoldier)) {
      DisableButton(iActionIcons[WALK_ICON]);
    }
  }

  iActionIcons[SNEAK_ICON] =
      QuickCreateButton(iIconImages[SNEAK_IMAGES], (int16_t)(iMenuAnchorX + 40),
                        (int16_t)(iMenuAnchorY + 20), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[SNEAK_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  SetButtonFastHelpText(iActionIcons[SNEAK_ICON], pTacticalPopupButtonStrings[SNEAK_ICON]);
  // SetButtonSavedRect( iActionIcons[ SNEAK_ICON ] );
  ButtonList[iActionIcons[SNEAK_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  // Check if this is a valid stance, diable if not!
  if (!IsValidStance(pSoldier, ANIM_CROUCH)) {
    DisableButton(iActionIcons[SNEAK_ICON]);
  }

  iActionIcons[CRAWL_ICON] =
      QuickCreateButton(iIconImages[CRAWL_IMAGES], (int16_t)(iMenuAnchorX + 40),
                        (int16_t)(iMenuAnchorY + 40), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[CRAWL_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  SetButtonFastHelpText(iActionIcons[CRAWL_ICON], pTacticalPopupButtonStrings[CRAWL_ICON]);
  // SetButtonSavedRect( iActionIcons[ CRAWL_ICON ] );
  ButtonList[iActionIcons[CRAWL_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  // Check if this is a valid stance, diable if not!
  if (!IsValidStance(pSoldier, ANIM_PRONE)) {
    DisableButton(iActionIcons[CRAWL_ICON]);
  }

  iActionIcons[LOOK_ICON] = QuickCreateButton(
      iIconImages[LOOK_IMAGES], (int16_t)(iMenuAnchorX), (int16_t)(iMenuAnchorY), BUTTON_NO_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 1, DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[LOOK_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  SetButtonFastHelpText(iActionIcons[LOOK_ICON], TacticalStr[LOOK_CURSOR_POPUPTEXT]);
  // SetButtonSavedRect( iActionIcons[ LOOK_ICON ] );
  ButtonList[iActionIcons[LOOK_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    DisableButton(iActionIcons[LOOK_ICON]);
  }

  if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
    if (!CanRobotBeControlled(pSoldier)) {
      DisableButton(iActionIcons[LOOK_ICON]);
    }
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // Until we get mounted weapons...
    uiActionImages = CANCEL_IMAGES;
    swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[NOT_APPLICABLE_POPUPTEXT]);
    fDisableAction = TRUE;
  } else {
    if (pSoldier->inv[HANDPOS].usItem == TOOLKIT) {
      uiActionImages = TOOLKITACTIONC_IMAGES;
      swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[NOT_APPLICABLE_POPUPTEXT]);
    } else if (pSoldier->inv[HANDPOS].usItem == WIRECUTTERS) {
      uiActionImages = WIRECUTACTIONC_IMAGES;
      swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[NOT_APPLICABLE_POPUPTEXT]);
    } else {
      // Create button based on what is in our hands at the moment!
      switch (Item[pSoldier->inv[HANDPOS].usItem].usItemClass) {
        case IC_PUNCH:

          uiActionImages = PUNCHACTIONC_IMAGES;
          swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[USE_HANDTOHAND_POPUPTEXT]);
          break;

        case IC_GUN:

          uiActionImages = TARGETACTIONC_IMAGES;
          swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[USE_FIREARM_POPUPTEXT]);
          break;

        case IC_BLADE:

          uiActionImages = KNIFEACTIONC_IMAGES;
          swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[USE_BLADE_POPUPTEXT]);
          break;

        case IC_GRENADE:
        case IC_BOMB:

          uiActionImages = BOMBACTIONC_IMAGES;
          swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[USE_EXPLOSIVE_POPUPTEXT]);
          break;

        case IC_MEDKIT:

          uiActionImages = AIDACTIONC_IMAGES;
          swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[USE_MEDKIT_POPUPTEXT]);
          break;

        default:

          uiActionImages = CANCEL_IMAGES;
          swprintf(zActionString, ARR_SIZE(zActionString), TacticalStr[NOT_APPLICABLE_POPUPTEXT]);
          fDisableAction = TRUE;
          break;
      }
    }
  }

  if (AM_AN_EPC(pSoldier)) {
    fDisableAction = TRUE;
  }

  iActionIcons[ACTIONC_ICON] =
      QuickCreateButton(iIconImages[uiActionImages], (int16_t)(iMenuAnchorX),
                        (int16_t)(iMenuAnchorY + 20), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[ACTIONC_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  // SetButtonSavedRect( iActionIcons[ ACTIONC_ICON ] );
  SetButtonFastHelpText(iActionIcons[ACTIONC_ICON], zActionString);
  ButtonList[iActionIcons[ACTIONC_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  if (fDisableAction) {
    DisableButton(iActionIcons[ACTIONC_ICON]);
  }

  iActionIcons[TALK_ICON] =
      QuickCreateButton(iIconImages[TALK_IMAGES], (int16_t)(iMenuAnchorX),
                        (int16_t)(iMenuAnchorY + 40), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[TALK_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  // SetButtonSavedRect( iActionIcons[ TALK_ICON ] );
  SetButtonFastHelpText(iActionIcons[TALK_ICON], pTacticalPopupButtonStrings[TALK_ICON]);
  ButtonList[iActionIcons[TALK_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  if (AM_AN_EPC(pSoldier) || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    DisableButton(iActionIcons[TALK_ICON]);
  }

  iActionIcons[HAND_ICON] =
      QuickCreateButton(iIconImages[HAND_IMAGES], (int16_t)(iMenuAnchorX + 20),
                        (int16_t)(iMenuAnchorY + 40), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[HAND_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  // SetButtonSavedRect( iActionIcons[ HAND_ICON ] );
  SetButtonFastHelpText(iActionIcons[HAND_ICON], pTacticalPopupButtonStrings[HAND_ICON]);
  ButtonList[iActionIcons[HAND_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  if (AM_AN_EPC(pSoldier) || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    DisableButton(iActionIcons[HAND_ICON]);
  }

  iActionIcons[CANCEL_ICON] =
      QuickCreateButton(iIconImages[CANCEL_IMAGES], (int16_t)(iMenuAnchorX + 20),
                        (int16_t)(iMenuAnchorY + 20), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnMovementCallback);
  if (iActionIcons[CANCEL_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  // SetButtonSavedRect( iActionIcons[ CANCEL_ICON ] );
  SetButtonFastHelpText(iActionIcons[CANCEL_ICON], pTacticalPopupButtonStrings[CANCEL_ICON]);
  ButtonList[iActionIcons[CANCEL_ICON]]->UserData[0] = (uintptr_t)pUIEvent;

  // LockTacticalInterface( );

  gfInMovementMenu = TRUE;

  // Ignore scrolling
  gfIgnoreScrolling = TRUE;
}

void PopDownMovementMenu() {
  if (gfInMovementMenu) {
    RemoveButton(iActionIcons[WALK_ICON]);
    RemoveButton(iActionIcons[SNEAK_ICON]);
    RemoveButton(iActionIcons[RUN_ICON]);
    RemoveButton(iActionIcons[CRAWL_ICON]);
    RemoveButton(iActionIcons[LOOK_ICON]);
    RemoveButton(iActionIcons[ACTIONC_ICON]);
    RemoveButton(iActionIcons[TALK_ICON]);
    RemoveButton(iActionIcons[HAND_ICON]);
    RemoveButton(iActionIcons[CANCEL_ICON]);

    // Turn off Ignore scrolling
    gfIgnoreScrolling = FALSE;

    // Rerender world
    SetRenderFlags(RENDER_FLAG_FULL);

    fInterfacePanelDirty = DIRTYLEVEL2;

    // UnLockTacticalInterface( );
    MSYS_RemoveRegion(&gMenuOverlayRegion);
  }

  gfInMovementMenu = FALSE;
}

void RenderMovementMenu() {
  if (gfInMovementMenu) {
    BltVObjectFromIndex(vsFB, guiBUTTONBORDER, 0, giMenuAnchorX, giMenuAnchorY);

    // Mark buttons dirty!
    MarkAButtonDirty(iActionIcons[WALK_ICON]);
    MarkAButtonDirty(iActionIcons[SNEAK_ICON]);
    MarkAButtonDirty(iActionIcons[RUN_ICON]);
    MarkAButtonDirty(iActionIcons[CRAWL_ICON]);
    MarkAButtonDirty(iActionIcons[LOOK_ICON]);
    MarkAButtonDirty(iActionIcons[ACTIONC_ICON]);
    MarkAButtonDirty(iActionIcons[TALK_ICON]);
    MarkAButtonDirty(iActionIcons[HAND_ICON]);
    MarkAButtonDirty(iActionIcons[CANCEL_ICON]);

    InvalidateRegion(giMenuAnchorX, giMenuAnchorY, giMenuAnchorX + BUTTON_PANEL_WIDTH,
                     giMenuAnchorY + BUTTON_PANEL_HEIGHT);
  }
}

void CancelMovementMenu() {
  // Signal end of event
  PopDownMovementMenu();
  guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
}

void BtnMovementCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t uiBtnID;
  UI_EVENT *pUIEvent;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags |= BUTTON_CLICKED_ON;

    uiBtnID = btn->IDNum;

    pUIEvent = (UI_EVENT *)(btn->UserData[0]);

    if (uiBtnID == iActionIcons[WALK_ICON]) {
      pUIEvent->uiParams[0] = MOVEMENT_MENU_WALK;
    } else if (uiBtnID == iActionIcons[RUN_ICON]) {
      pUIEvent->uiParams[0] = MOVEMENT_MENU_RUN;
    } else if (uiBtnID == iActionIcons[SNEAK_ICON]) {
      pUIEvent->uiParams[0] = MOVEMENT_MENU_SWAT;
    } else if (uiBtnID == iActionIcons[CRAWL_ICON]) {
      pUIEvent->uiParams[0] = MOVEMENT_MENU_PRONE;
    } else if (uiBtnID == iActionIcons[LOOK_ICON]) {
      pUIEvent->uiParams[2] = MOVEMENT_MENU_LOOK;
    } else if (uiBtnID == iActionIcons[ACTIONC_ICON]) {
      pUIEvent->uiParams[2] = MOVEMENT_MENU_ACTIONC;
    } else if (uiBtnID == iActionIcons[TALK_ICON]) {
      pUIEvent->uiParams[2] = MOVEMENT_MENU_TALK;
    } else if (uiBtnID == iActionIcons[HAND_ICON]) {
      pUIEvent->uiParams[2] = MOVEMENT_MENU_HAND;
    } else if (uiBtnID == iActionIcons[CANCEL_ICON]) {
      // Signal end of event
      EndMenuEvent(U_MOVEMENT_MENU);
      pUIEvent->uiParams[1] = FALSE;
      return;
    } else {
      return;
    }

    // Signal end of event
    EndMenuEvent(U_MOVEMENT_MENU);
    pUIEvent->uiParams[1] = TRUE;
  }
}

void HandleUpDownArrowBackgrounds() {
  static uint32_t uiOldShowUpDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

  // Check for change in mode
  if (guiShowUPDownArrows != uiOldShowUpDownArrows || gfUIRefreshArrows) {
    gfUIRefreshArrows = FALSE;

    // Hide position of new ones
    GetArrowsBackground();

    uiOldShowUpDownArrows = guiShowUPDownArrows;
  }
}

void RenderArrows() {
  TILE_ELEMENT TileElem;

  if (guiShowUPDownArrows & ARROWS_HIDE_UP && guiShowUPDownArrows & ARROWS_HIDE_DOWN) {
    return;
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_BESIDE) {
    TileElem = gTileDatabase[SECONDPOINTERS3];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_G) {
    TileElem = gTileDatabase[SECONDPOINTERS1];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_Y) {
    TileElem = gTileDatabase[SECONDPOINTERS3];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YG) {
    TileElem = gTileDatabase[SECONDPOINTERS3];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
    TileElem = gTileDatabase[SECONDPOINTERS1];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX,
                   gsUpArrowY + 20);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_GG) {
    TileElem = gTileDatabase[SECONDPOINTERS1];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX,
                   gsUpArrowY + 20);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YY) {
    TileElem = gTileDatabase[SECONDPOINTERS3];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX,
                   gsUpArrowY + 20);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB) {
    TileElem = gTileDatabase[SECONDPOINTERS8];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB2) {
    TileElem = gTileDatabase[SECONDPOINTERS3];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX,
                   gsUpArrowY + 20);
    TileElem = gTileDatabase[SECONDPOINTERS8];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB3) {
    TileElem = gTileDatabase[SECONDPOINTERS3];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
    TileElem = gTileDatabase[SECONDPOINTERS8];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX,
                   gsUpArrowY + 20);
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX,
                   gsUpArrowY + 40);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BESIDE) {
    TileElem = gTileDatabase[SECONDPOINTERS4];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_G) {
    TileElem = gTileDatabase[SECONDPOINTERS2];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_Y) {
    TileElem = gTileDatabase[SECONDPOINTERS4];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_CLIMB) {
    TileElem = gTileDatabase[SECONDPOINTERS7];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YG) {
    TileElem = gTileDatabase[SECONDPOINTERS2];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
    TileElem = gTileDatabase[SECONDPOINTERS4];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX,
                   gsDownArrowY + 20);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_GG) {
    TileElem = gTileDatabase[SECONDPOINTERS2];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX,
                   gsDownArrowY + 20);
  }

  if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YY) {
    TileElem = gTileDatabase[SECONDPOINTERS4];
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
    BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX,
                   gsDownArrowY + 20);
  }
}

void EraseRenderArrows() {
  if (giUpArrowRect != 0) {
    if (giUpArrowRect != -1) {
      FreeBackgroundRect(giUpArrowRect);
    }
  }
  giUpArrowRect = 0;

  if (giDownArrowRect != 0) {
    if (giDownArrowRect != -1) {
      FreeBackgroundRect(giDownArrowRect);
    }
  }
  giDownArrowRect = 0;
}

void GetArrowsBackground() {
  struct SOLDIERTYPE *pSoldier;
  int16_t sMercScreenX, sMercScreenY;
  uint16_t sArrowHeight = ARROWS_HEIGHT, sArrowWidth = ARROWS_WIDTH;

  if (guiShowUPDownArrows & ARROWS_HIDE_UP && guiShowUPDownArrows & ARROWS_HIDE_DOWN) {
    return;
  }

  if (gusSelectedSoldier != NO_SOLDIER) {
    // Get selected soldier
    GetSoldier(&pSoldier, gusSelectedSoldier);

    // Get screen position of our guy
    GetSoldierTRUEScreenPos(pSoldier, &sMercScreenX, &sMercScreenY);

    if (guiShowUPDownArrows & ARROWS_SHOW_UP_BESIDE) {
      // Setup blt rect
      gsUpArrowX = sMercScreenX + ARROWS_X_OFFSET;
      gsUpArrowY = sMercScreenY + UPARROW_Y_OFFSET;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_G ||
        guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_Y) {
      // Setup blt rect
      gsUpArrowX = sMercScreenX - 10;
      gsUpArrowY = sMercScreenY - 50;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YG ||
        guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_GG ||
        guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YY) {
      // Setup blt rect
      gsUpArrowX = sMercScreenX - 10;
      gsUpArrowY = sMercScreenY - 70;
      sArrowHeight = 3 * ARROWS_HEIGHT;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB) {
      // Setup blt rect
      gsUpArrowX = sMercScreenX - 10;
      gsUpArrowY = sMercScreenY - 70;
      sArrowHeight = 2 * ARROWS_HEIGHT;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB2) {
      // Setup blt rect
      gsUpArrowX = sMercScreenX - 10;
      gsUpArrowY = sMercScreenY - 80;
      sArrowHeight = 3 * ARROWS_HEIGHT;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB3) {
      // Setup blt rect
      gsUpArrowX = sMercScreenX - 10;
      gsUpArrowY = sMercScreenY - 900;
      sArrowHeight = 5 * ARROWS_HEIGHT;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BESIDE) {
      gsDownArrowX = sMercScreenX + ARROWS_X_OFFSET;
      gsDownArrowY = sMercScreenY + DOWNARROW_Y_OFFSET;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_Y ||
        guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_G) {
      gsDownArrowX = sMercScreenX - 10;
      gsDownArrowY = sMercScreenY + 10;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_CLIMB) {
      gsDownArrowX = sMercScreenX - 10;
      gsDownArrowY = sMercScreenY + 10;
      sArrowHeight = 3 * ARROWS_HEIGHT;
    }

    if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YG ||
        guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_GG ||
        guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YY) {
      gsDownArrowX = sMercScreenX - 10;
      gsDownArrowY = sMercScreenY + 10;
      sArrowHeight = 3 * ARROWS_HEIGHT;
    }

    // Adjust arrows based on level
    if (gsInterfaceLevel == I_ROOF_LEVEL) {
      //	gsDownArrowY -= ROOF_LEVEL_HEIGHT;
      //	gsUpArrowY	 -= ROOF_LEVEL_HEIGHT;
    }

    // Erase prevois ones...
    EraseRenderArrows();

    // Register dirty rects
    giDownArrowRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, gsDownArrowX, gsDownArrowY,
                                             (int16_t)(gsDownArrowX + sArrowWidth),
                                             (int16_t)(gsDownArrowY + sArrowHeight));
    giUpArrowRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, NULL, gsUpArrowX, gsUpArrowY,
                                           (int16_t)(gsUpArrowX + sArrowWidth),
                                           (int16_t)(gsUpArrowY + sArrowHeight));
  }
}

void GetSoldierAboveGuyPositions(struct SOLDIERTYPE *pSoldier, int16_t *psX, int16_t *psY,
                                 BOOLEAN fRadio) {
  int16_t sMercScreenX, sMercScreenY;
  int16_t sOffsetX, sOffsetY;
  uint8_t ubAnimUseHeight;
  int16_t sStanceOffset = 0;
  int16_t sTextBodyTypeYOffset = 62;

  // Find XY, dims, offsets
  GetSoldierScreenPos(pSoldier, &sMercScreenX, &sMercScreenY);
  GetSoldierAnimOffsets(pSoldier, &sOffsetX, &sOffsetY);

  // OK, first thing to do is subtract offsets ( because GetSoldierScreenPos adds them... )
  sMercScreenX -= sOffsetX;
  sMercScreenY -= sOffsetY;

  // Adjust based on stance
  if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_NOMOVE_MARKER)) {
    ubAnimUseHeight = gAnimControl[pSoldier->usAnimState].ubHeight;
  } else {
    ubAnimUseHeight = gAnimControl[pSoldier->usAnimState].ubEndHeight;
  }
  switch (ubAnimUseHeight) {
    case ANIM_STAND:
      break;

    case ANIM_PRONE:
      sStanceOffset = 25;
      break;

    case ANIM_CROUCH:
      sStanceOffset = 10;
      break;
  }

  // Adjust based on body type...
  switch (pSoldier->ubBodyType) {
    case CROW:

      sStanceOffset = 30;
      break;

    case ROBOTNOWEAPON:

      sStanceOffset = 30;
      break;
  }

  // sStanceOffset -= gpWorldLevelData[ pSoldier->sGridNo ].sHeight;

  // Adjust based on level
  if (pSoldier->bLevel == 1 && gsInterfaceLevel == 0) {
    // sStanceOffset -= ROOF_LEVEL_HEIGHT;
  }
  if (pSoldier->bLevel == 0 && gsInterfaceLevel == 1) {
    // sStanceOffset += ROOF_LEVEL_HEIGHT;
  }

  if (GetSolProfile(pSoldier) != NO_PROFILE) {
    if (fRadio) {
      *psX = sMercScreenX - (80 / 2) - pSoldier->sLocatorOffX;
      *psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;
    } else {
      *psX = sMercScreenX - (80 / 2) - pSoldier->sLocatorOffX;
      *psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;

      // OK, Check if we need to go below....
      // Can do this 1) if displaying damge or 2 ) above screen

      // If not a radio position, adjust if we are getting hit, to be lower!
      // If we are getting hit, lower them!
      if (pSoldier->fDisplayDamage || *psY < gsVIEWPORT_WINDOW_START_Y) {
        *psX = sMercScreenX - (80 / 2) - pSoldier->sLocatorOffX;
        *psY = sMercScreenY;
      }
    }

  } else {
    // Display Text!
    *psX = sMercScreenX - (80 / 2) - pSoldier->sLocatorOffX;
    *psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;
  }
}

void DrawSelectedUIAboveGuy(uint16_t usSoldierID) {
  struct SOLDIERTYPE *pSoldier;
  int16_t sXPos, sYPos;
  int16_t sX, sY;
  int32_t iBack;
  TILE_ELEMENT TileElem;
  wchar_t *pStr;
  wchar_t NameStr[50];
  uint16_t usGraphicToUse = THIRDPOINTERS1;
  BOOLEAN fRaiseName = FALSE;
  BOOLEAN fDoName = TRUE;

  GetSoldier(&pSoldier, usSoldierID);

  if (pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
    return;
  }

  if (pSoldier->sGridNo == NOWHERE) {
    return;
  }

  if (pSoldier->fFlashLocator) {
    if (pSoldier->bVisible == -1) {
      pSoldier->fFlashLocator = FALSE;
    } else {
      if (TIMECOUNTERDONE(pSoldier->BlinkSelCounter, 80)) {
        RESETTIMECOUNTER(pSoldier->BlinkSelCounter, 80);

        //	pSoldier->fShowLocator = !pSoldier->fShowLocator;

        pSoldier->fShowLocator = TRUE;

        // Update frame
        pSoldier->sLocatorFrame++;

        if (pSoldier->sLocatorFrame == 5) {
          // Update time we do this
          pSoldier->fFlashLocator++;
          pSoldier->sLocatorFrame = 0;
        }
      }

      // if ( TIMECOUNTERDONE( pSoldier->FlashSelCounter, 5000 ) )
      //{
      //	RESETTIMECOUNTER( pSoldier->FlashSelCounter, 5000 );

      //	pSoldier->fFlashLocator = FALSE;
      //	pSoldier->fShowLocator = FALSE;

      //}
      if (pSoldier->fFlashLocator == pSoldier->ubNumLocateCycles) {
        pSoldier->fFlashLocator = FALSE;
        pSoldier->fShowLocator = FALSE;
      }

      // if ( pSoldier->fShowLocator )
      {
        // Render the beastie
        GetSoldierAboveGuyPositions(pSoldier, &sXPos, &sYPos, TRUE);

        // Adjust for bars!
        sXPos += 25;
        sYPos += 25;

        // sXPos += 15;
        // sYPos += 21;

        // Add bars
        // iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, sXPos, sYPos, (int16_t)(sXPos +
        // 55
        // ), (int16_t)(sYPos + 80 ) );
        iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sXPos, sYPos, (int16_t)(sXPos + 40),
                                       (int16_t)(sYPos + 40));

        if (iBack != -1) {
          SetBackgroundRectFilled(iBack);
        }

        if ((!pSoldier->bNeutral && (pSoldier->bSide != gbPlayerNum))) {
          BltVObjectFromIndex(vsFB, guiRADIO2, pSoldier->sLocatorFrame, sXPos, sYPos);
        } else {
          BltVObjectFromIndex(vsFB, guiRADIO, pSoldier->sLocatorFrame, sXPos, sYPos);
        }
      }
    }
    // return;
  }

  if (!pSoldier->fShowLocator) {
    // RETURN IF MERC IS NOT SELECTED
    if (gfUIHandleSelectionAboveGuy && GetSolID(pSoldier) == gsSelectedGuy &&
        GetSolID(pSoldier) != gusSelectedSoldier && !gfIgnoreOnSelectedGuy) {
    } else if (pSoldier->ubID == gusSelectedSoldier && !gRubberBandActive) {
      usGraphicToUse = THIRDPOINTERS2;
    }
    // show all people's names if !
    // else if ( GetSolID(pSoldier) >= 20 && pSoldier->bVisible != -1 )
    //{

    //}
    else if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
    } else {
      return;
    }
  } else {
    if (pSoldier->ubID == gusSelectedSoldier && !gRubberBandActive) {
      usGraphicToUse = THIRDPOINTERS2;
    }
  }

  // If he is in the middle of a certain animation, ignore!
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_NOSHOW_MARKER) {
    return;
  }

  // Donot show if we are dead
  if ((pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
    return;
  }

  GetSoldierAboveGuyPositions(pSoldier, &sXPos, &sYPos, FALSE);

  // Display name
  SetFont(TINYFONT1);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_WHITE);

  if (GetSolProfile(pSoldier) != NO_PROFILE || (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    if (gfUIMouseOnValidCatcher == 1 && GetSolID(pSoldier) == gubUIValidCatcherID) {
      swprintf(NameStr, ARR_SIZE(NameStr), TacticalStr[CATCH_STR]);
      FindFontCenterCoordinates(sXPos, (int16_t)(sYPos), (int16_t)(80), 1, NameStr, TINYFONT1, &sX,
                                &sY);
      gprintfdirty(sX, sY, NameStr);
      mprintf(sX, sY, NameStr);
      fRaiseName = TRUE;
    } else if (gfUIMouseOnValidCatcher == 3 && GetSolID(pSoldier) == gubUIValidCatcherID) {
      swprintf(NameStr, ARR_SIZE(NameStr), TacticalStr[RELOAD_STR]);
      FindFontCenterCoordinates(sXPos, (int16_t)(sYPos), (int16_t)(80), 1, NameStr, TINYFONT1, &sX,
                                &sY);
      gprintfdirty(sX, sY, NameStr);
      mprintf(sX, sY, NameStr);
      fRaiseName = TRUE;
    } else if (gfUIMouseOnValidCatcher == 4 && GetSolID(pSoldier) == gubUIValidCatcherID) {
      swprintf(NameStr, ARR_SIZE(NameStr), pMessageStrings[MSG_PASS]);
      FindFontCenterCoordinates(sXPos, (int16_t)(sYPos), (int16_t)(80), 1, NameStr, TINYFONT1, &sX,
                                &sY);
      gprintfdirty(sX, sY, NameStr);
      mprintf(sX, sY, NameStr);
      fRaiseName = TRUE;
    } else if (pSoldier->bAssignment >= ON_DUTY) {
      SetFontForeground(FONT_YELLOW);
      swprintf(NameStr, ARR_SIZE(NameStr), L"(%s)", pAssignmentStrings[pSoldier->bAssignment]);
      FindFontCenterCoordinates(sXPos, (int16_t)(sYPos), (int16_t)(80), 1, NameStr, TINYFONT1, &sX,
                                &sY);
      gprintfdirty(sX, sY, NameStr);
      mprintf(sX, sY, NameStr);
      fRaiseName = TRUE;
    } else if (pSoldier->bTeam == gbPlayerNum && pSoldier->bAssignment < ON_DUTY &&
               pSoldier->bAssignment != CurrentSquad() &&
               !(pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED)) {
      swprintf(NameStr, ARR_SIZE(NameStr), gzLateLocalizedString[34], (pSoldier->bAssignment + 1));
      FindFontCenterCoordinates(sXPos, (int16_t)(sYPos), (int16_t)(80), 1, NameStr, TINYFONT1, &sX,
                                &sY);
      gprintfdirty(sX, sY, NameStr);
      mprintf(sX, sY, NameStr);
      fRaiseName = TRUE;
    }

    // If not in a squad....
    if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      if (GetNumberInVehicle(pSoldier->bVehicleID) == 0) {
        SetFontForeground(FONT_GRAY4);
      }
    } else {
      if (pSoldier->bAssignment >= ON_DUTY) {
        SetFontForeground(FONT_YELLOW);
      }
    }

    if (fDoName) {
      if (fRaiseName) {
        swprintf(NameStr, ARR_SIZE(NameStr), L"%s", pSoldier->name);
        FindFontCenterCoordinates(sXPos, (int16_t)(sYPos - 10), (int16_t)(80), 1, NameStr,
                                  TINYFONT1, &sX, &sY);
        gprintfdirty(sX, sY, NameStr);
        mprintf(sX, sY, NameStr);
      } else {
        swprintf(NameStr, ARR_SIZE(NameStr), L"%s", pSoldier->name);
        FindFontCenterCoordinates(sXPos, sYPos, (int16_t)(80), 1, NameStr, TINYFONT1, &sX, &sY);
        gprintfdirty(sX, sY, NameStr);
        mprintf(sX, sY, NameStr);
      }
    }

    if (GetSolProfile(pSoldier) < FIRST_RPC || RPC_RECRUITED(pSoldier) || AM_AN_EPC(pSoldier) ||
        (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
      // Adjust for bars!

      if (pSoldier->ubID == gusSelectedSoldier) {
        sXPos += 28;
        sYPos += 5;
      } else {
        sXPos += 30;
        sYPos += 7;
      }

      // Add bars
      iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sXPos, sYPos, (int16_t)(sXPos + 34),
                                     (int16_t)(sYPos + 11));

      if (iBack != -1) {
        SetBackgroundRectFilled(iBack);
      }
      TileElem = gTileDatabase[usGraphicToUse];
      BltVideoObject(vsFB, TileElem.hTileSurface, TileElem.usRegionIndex, sXPos, sYPos);

      // Draw life, breath
      // Only do this when we are a vehicle but on our team
      if (pSoldier->ubID == gusSelectedSoldier) {
        DrawBarsInUIBox(pSoldier, (int16_t)(sXPos + 1), (int16_t)(sYPos + 2), 16, 1);
      } else {
        DrawBarsInUIBox(pSoldier, (int16_t)(sXPos), (int16_t)(sYPos), 16, 1);
      }
    } else {
      if (gfUIMouseOnValidCatcher == 2 && GetSolID(pSoldier) == gubUIValidCatcherID) {
        SetFont(TINYFONT1);
        SetFontBackground(FONT_MCOLOR_BLACK);
        SetFontForeground(FONT_MCOLOR_WHITE);

        swprintf(NameStr, ARR_SIZE(NameStr), TacticalStr[GIVE_STR]);
        FindFontCenterCoordinates(sXPos, (int16_t)(sYPos + 10), (int16_t)(80), 1, NameStr,
                                  TINYFONT1, &sX, &sY);
        gprintfdirty(sX, sY, NameStr);
        mprintf(sX, sY, NameStr);
      } else {
        SetFont(TINYFONT1);
        SetFontBackground(FONT_MCOLOR_BLACK);
        SetFontForeground(FONT_MCOLOR_DKRED);

        pStr = GetSoldierHealthString(pSoldier);

        FindFontCenterCoordinates(sXPos, (int16_t)(sYPos + 10), (int16_t)(80), 1, pStr, TINYFONT1,
                                  &sX, &sY);
        gprintfdirty(sX, sY, pStr);
        mprintf(sX, sY, pStr);
      }
    }
  } else {
    if (pSoldier->bLevel != 0) {
      // Display name
      SetFont(TINYFONT1);
      SetFontBackground(FONT_MCOLOR_BLACK);
      SetFontForeground(FONT_YELLOW);

      swprintf(NameStr, ARR_SIZE(NameStr), gzLateLocalizedString[15]);
      FindFontCenterCoordinates(sXPos, (int16_t)(sYPos + 10), (int16_t)(80), 1, NameStr, TINYFONT1,
                                &sX, &sY);
      gprintfdirty(sX, sY, NameStr);
      mprintf(sX, sY, NameStr);
    }

    pStr = GetSoldierHealthString(pSoldier);

    SetFont(TINYFONT1);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_DKRED);

    FindFontCenterCoordinates(sXPos, sYPos, (int16_t)(80), 1, pStr, TINYFONT1, &sX, &sY);
    gprintfdirty(sX, sY, pStr);
    mprintf(sX, sY, pStr);
  }
}

void RenderOverlayMessage(VIDEO_OVERLAY *pBlitter) {
  // Override it!
  OverrideMercPopupBox(&gpOverrideMercBox);

  RenderMercPopupBox(pBlitter->sX, pBlitter->sY, pBlitter->vsDestBuff);

  // Set it back!
  ResetOverrideMercPopupBox();

  InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusOverlayPopupBoxWidth,
                   pBlitter->sY + gusOverlayPopupBoxHeight);
}

void BeginOverlayMessage(uint32_t uiFont, wchar_t *pFontString, ...) {
  va_list argptr;
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  wchar_t SlideString[512];

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(SlideString, ARR_SIZE(SlideString), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  // Override it!
  OverrideMercPopupBox(&gpOverrideMercBox);

  SetPrepareMercPopupFlags(MERC_POPUP_PREPARE_FLAGS_TRANS_BACK | MERC_POPUP_PREPARE_FLAGS_MARGINS);

  // Prepare text box
  iOverlayMessageBox = PrepareMercPopupBox(iOverlayMessageBox, BASIC_MERC_POPUP_BACKGROUND,
                                           RED_MERC_POPUP_BORDER, SlideString, 200, 50, 0, 0,
                                           &gusOverlayPopupBoxWidth, &gusOverlayPopupBoxHeight);

  // Set it back!
  ResetOverrideMercPopupBox();

  if (giPopupSlideMessageOverlay == -1) {
    // Set Overlay
    VideoOverlayDesc.sLeft = (640 - gusOverlayPopupBoxWidth) / 2;
    VideoOverlayDesc.sTop = 100;
    VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + gusOverlayPopupBoxWidth;
    VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + gusOverlayPopupBoxHeight;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    VideoOverlayDesc.BltCallback = RenderOverlayMessage;

    giPopupSlideMessageOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
  }
}

void EndOverlayMessage() {
  if (giPopupSlideMessageOverlay != -1) {
    //		DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Removing Overlay message") );

    RemoveVideoOverlay(giPopupSlideMessageOverlay);

    giPopupSlideMessageOverlay = -1;
  }
}

void DrawBarsInUIBox(struct SOLDIERTYPE *pSoldier, int16_t sXPos, int16_t sYPos, int16_t sWidth,
                     int16_t sHeight) {
  float dWidth, dPercentage;
  // uint16_t usLineColor;

  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;
  int8_t bBandage;

  // Draw breath points

  // Draw new size
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, gsVIEWPORT_WINDOW_START_Y, 640,
                                 (gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y));

  // get amt bandaged
  bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

  // NOW DO BLEEDING
  if (pSoldier->bBleeding) {
    dPercentage = (float)(pSoldier->bBleeding + pSoldier->bLife + bBandage) / (float)100;
    dWidth = dPercentage * sWidth;
    usLineColor = rgb32_to_rgb16(FROMRGB(240, 240, 20));
    RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (int32_t)(sXPos + dWidth + 3), sYPos + 1, usLineColor,
                  pDestBuf);
  }

  if (bBandage) {
    dPercentage = (float)(pSoldier->bLife + bBandage) / (float)100;
    dWidth = dPercentage * sWidth;
    usLineColor = rgb32_to_rgb16(FROMRGB(222, 132, 132));
    RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (int32_t)(sXPos + dWidth + 3), sYPos + 1, usLineColor,
                  pDestBuf);
  }

  dPercentage = (float)pSoldier->bLife / (float)100;
  dWidth = dPercentage * sWidth;
  usLineColor = rgb32_to_rgb16(FROMRGB(200, 0, 0));
  RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (int32_t)(sXPos + dWidth + 3), sYPos + 1, usLineColor,
                pDestBuf);

  dPercentage = (float)(pSoldier->bBreathMax) / (float)100;
  dWidth = dPercentage * sWidth;
  usLineColor = rgb32_to_rgb16(FROMRGB(20, 20, 150));
  RectangleDraw(TRUE, sXPos + 3, sYPos + 4, (int32_t)(sXPos + dWidth + 3), sYPos + 4, usLineColor,
                pDestBuf);

  dPercentage = (float)(pSoldier->bBreath) / (float)100;
  dWidth = dPercentage * sWidth;
  usLineColor = rgb32_to_rgb16(FROMRGB(100, 100, 220));
  RectangleDraw(TRUE, sXPos + 3, sYPos + 4, (int32_t)(sXPos + dWidth + 3), sYPos + 4, usLineColor,
                pDestBuf);

  JSurface_Unlock(vsFB);
}

void EndDeadlockMsg() {
  // Reset gridlock
  gfUIInDeadlock = FALSE;
}

void ClearInterface() {
  if ((IsMapScreen())) {
    return;
  }

  // Stop any UI menus!
  if (gCurrentUIMode == MENU_MODE) {
    EndMenuEvent(guiCurrentEvent);
  }

  if (gfUIHandleShowMoveGrid) {
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS4);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS2);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS13);
    RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS15);
  }

  // Remove any popup menus
  if (gCurrentUIMode == GETTINGITEM_MODE) {
    RemoveItemPickupMenu();
  }

  // Remove any popup menus
  if (gCurrentUIMode == OPENDOOR_MENU_MODE) {
    PopDownOpenDoorMenu();
  }

  // Remove UP/DOWN arrows...
  // EraseRenderArrows( );
  // Don't render arrows this frame!
  guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

  ResetPhysicsTrajectoryUI();

  // Remove any paths, cursors
  ErasePath(FALSE);

  // gfPlotNewMovement = TRUE;

  // Erase Interface cursors
  HideUICursor();

  MSYS_ChangeRegionCursor(&gViewportRegion, VIDEO_NO_CURSOR);

  // Hide lock UI cursors...
  MSYS_ChangeRegionCursor(&gDisableRegion, VIDEO_NO_CURSOR);
  MSYS_ChangeRegionCursor(&gUserTurnRegion, VIDEO_NO_CURSOR);

  // Remove special thing for south arrow...
  if (gsGlobalCursorYOffset == (480 - gsVIEWPORT_WINDOW_END_Y)) {
    SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
  }
}

void RestoreInterface() {
  // Once we are done, plot path again!
  gfPlotNewMovement = TRUE;

  // OK, reset arrows too...
  gfUIRefreshArrows = TRUE;

  // SHow lock UI cursors...
  MSYS_ChangeRegionCursor(&gDisableRegion, CURSOR_WAIT);
  MSYS_ChangeRegionCursor(&gUserTurnRegion, CURSOR_WAIT);
}

void BlitPopupText(VIDEO_OVERLAY *pBlitter) {
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;

  BltVSurfaceToVSurfaceFast(pBlitter->vsDestBuff, vsINTEXT, pBlitter->pBackground->sLeft,
                            pBlitter->pBackground->sTop);

  pDestBuf = LockVSurface(pBlitter->vsDestBuff, &uiDestPitchBYTES);

  SetFont(pBlitter->uiFontID);
  SetFontBackground(pBlitter->ubFontBack);
  SetFontForeground(pBlitter->ubFontFore);

  mprintf_buffer(pDestBuf, uiDestPitchBYTES, pBlitter->uiFontID, pBlitter->sX, pBlitter->sY,
                 pBlitter->zText);

  JSurface_Unlock(pBlitter->vsDestBuff);
}

void DirtyMercPanelInterface(struct SOLDIERTYPE *pSoldier, uint8_t ubDirtyLevel) {
  if (pSoldier->bTeam == gbPlayerNum) {
    // ONly set to a higher level!
    if (fInterfacePanelDirty < ubDirtyLevel) {
      fInterfacePanelDirty = ubDirtyLevel;
    }
  }
}

typedef struct {
  struct SOLDIERTYPE *pSoldier;
  struct STRUCTURE *pStructure;
  uint8_t ubDirection;
  int16_t sX;
  int16_t sY;
  BOOLEAN fMenuHandled;
  BOOLEAN fClosingDoor;

} OPENDOOR_MENU;

OPENDOOR_MENU gOpenDoorMenu;
BOOLEAN gfInOpenDoorMenu = FALSE;

BOOLEAN InitDoorOpenMenu(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                         uint8_t ubDirection, BOOLEAN fClosingDoor) {
  int16_t sHeight, sWidth;
  int16_t sScreenX, sScreenY;

  // Erase other menus....
  EraseInterfaceMenus(TRUE);

  InterruptTime();
  PauseGame();
  LockPauseState(19);
  // Pause timers as well....
  PauseTime(TRUE);

  gOpenDoorMenu.pSoldier = pSoldier;
  gOpenDoorMenu.pStructure = pStructure;
  gOpenDoorMenu.ubDirection = ubDirection;
  gOpenDoorMenu.fClosingDoor = fClosingDoor;

  // OK, Determine position...
  // Center on guy
  // Locate to guy first.....
  LocateSoldier(pSoldier->ubID, FALSE);
  GetSoldierAnimDims(pSoldier, &sHeight, &sWidth);
  GetSoldierScreenPos(pSoldier, &sScreenX, &sScreenY);
  gOpenDoorMenu.sX = sScreenX - ((BUTTON_PANEL_WIDTH - sWidth) / 2);
  gOpenDoorMenu.sY = sScreenY - ((BUTTON_PANEL_HEIGHT - sHeight) / 2);

  // Alrighty, cancel lock UI if we havn't done so already
  UnSetUIBusy(pSoldier->ubID);

  // OK, CHECK FOR BOUNDARIES!
  if ((gOpenDoorMenu.sX + BUTTON_PANEL_WIDTH) > 640) {
    gOpenDoorMenu.sX = (640 - BUTTON_PANEL_WIDTH);
  }
  if ((gOpenDoorMenu.sY + BUTTON_PANEL_HEIGHT) > gsVIEWPORT_WINDOW_END_Y) {
    gOpenDoorMenu.sY = (gsVIEWPORT_WINDOW_END_Y - BUTTON_PANEL_HEIGHT);
  }
  if (gOpenDoorMenu.sX < 0) {
    gOpenDoorMenu.sX = 0;
  }
  if (gOpenDoorMenu.sY < 0) {
    gOpenDoorMenu.sY = 0;
  }

  gOpenDoorMenu.fMenuHandled = FALSE;

  guiPendingOverrideEvent = OP_OPENDOORMENU;
  HandleTacticalUI();

  PopupDoorOpenMenu(fClosingDoor);

  return (TRUE);
}

void PopupDoorOpenMenu(BOOLEAN fClosingDoor) {
  int32_t iMenuAnchorX, iMenuAnchorY;
  wchar_t zDisp[100];

  iMenuAnchorX = gOpenDoorMenu.sX;
  iMenuAnchorY = gOpenDoorMenu.sY;

  iMenuAnchorX = gOpenDoorMenu.sX + 9;
  iMenuAnchorY = gOpenDoorMenu.sY + 8;

  // Create mouse region over all area to facilitate clicking to end
  MSYS_DefineRegion(&gMenuOverlayRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL,
                    MSYS_NO_CALLBACK, DoorMenuBackregionCallback);
  // Add region
  MSYS_AddRegion(&gMenuOverlayRegion);

  iActionIcons[USE_KEYRING_ICON] =
      QuickCreateButton(iIconImages[USE_KEYRING_IMAGES], (int16_t)(iMenuAnchorX + 20),
                        (int16_t)(iMenuAnchorY), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[USE_KEYRING_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[USE_KEYRING_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[USE_KEYRING_ICON],
             AP_UNLOCK_DOOR);
  }
  SetButtonFastHelpText(iActionIcons[USE_KEYRING_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_UNLOCK_DOOR, BP_UNLOCK_DOOR, FALSE) ||
      fClosingDoor || AM_AN_EPC(gOpenDoorMenu.pSoldier)) {
    DisableButton(iActionIcons[USE_KEYRING_ICON]);
  }

  // Greyout if no keys found...
  if (!SoldierHasKey(gOpenDoorMenu.pSoldier, ANYKEY)) {
    DisableButton(iActionIcons[USE_KEYRING_ICON]);
  }

  iActionIcons[USE_CROWBAR_ICON] =
      QuickCreateButton(iIconImages[CROWBAR_DOOR_IMAGES], (int16_t)(iMenuAnchorX + 40),
                        (int16_t)(iMenuAnchorY), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[USE_CROWBAR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[USE_CROWBAR_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[USE_CROWBAR_ICON],
             AP_USE_CROWBAR);
  }
  SetButtonFastHelpText(iActionIcons[USE_CROWBAR_ICON], zDisp);

  // Greyout if no crowbar found...
  if (FindUsableObj(gOpenDoorMenu.pSoldier, CROWBAR) == NO_SLOT || fClosingDoor) {
    DisableButton(iActionIcons[USE_CROWBAR_ICON]);
  }

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_USE_CROWBAR, BP_USE_CROWBAR, FALSE)) {
    DisableButton(iActionIcons[USE_CROWBAR_ICON]);
  }

  iActionIcons[LOCKPICK_DOOR_ICON] =
      QuickCreateButton(iIconImages[LOCKPICK_DOOR_IMAGES], (int16_t)(iMenuAnchorX + 40),
                        (int16_t)(iMenuAnchorY + 20), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[LOCKPICK_DOOR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[LOCKPICK_DOOR_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[LOCKPICK_DOOR_ICON],
             AP_PICKLOCK);
  }
  SetButtonFastHelpText(iActionIcons[LOCKPICK_DOOR_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_PICKLOCK, BP_PICKLOCK, FALSE) || fClosingDoor ||
      AM_AN_EPC(gOpenDoorMenu.pSoldier)) {
    DisableButton(iActionIcons[LOCKPICK_DOOR_ICON]);
  }

  // Grayout if no lockpick found....
  if (FindObj(gOpenDoorMenu.pSoldier, LOCKSMITHKIT) == NO_SLOT) {
    DisableButton(iActionIcons[LOCKPICK_DOOR_ICON]);
  }

  iActionIcons[EXPLOSIVE_DOOR_ICON] =
      QuickCreateButton(iIconImages[EXPLOSIVE_DOOR_IMAGES], (int16_t)(iMenuAnchorX + 40),
                        (int16_t)(iMenuAnchorY + 40), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[EXPLOSIVE_DOOR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[EXPLOSIVE_DOOR_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[EXPLOSIVE_DOOR_ICON],
             AP_EXPLODE_DOOR);
  }
  SetButtonFastHelpText(iActionIcons[EXPLOSIVE_DOOR_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_EXPLODE_DOOR, BP_EXPLODE_DOOR, FALSE) ||
      fClosingDoor || AM_AN_EPC(gOpenDoorMenu.pSoldier)) {
    DisableButton(iActionIcons[EXPLOSIVE_DOOR_ICON]);
  }

  // Grayout if no lock explosive found....
  // For no use bomb1 until we get a special item for this
  if (FindObj(gOpenDoorMenu.pSoldier, SHAPED_CHARGE) == NO_SLOT) {
    DisableButton(iActionIcons[EXPLOSIVE_DOOR_ICON]);
  }

  iActionIcons[OPEN_DOOR_ICON] =
      QuickCreateButton(iIconImages[OPEN_DOOR_IMAGES], (int16_t)(iMenuAnchorX),
                        (int16_t)(iMenuAnchorY), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[OPEN_DOOR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (fClosingDoor) {
    if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
      swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[CANCEL_ICON + 1]);
    } else {
      swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[CANCEL_ICON + 1],
               AP_OPEN_DOOR);
    }
  } else {
    if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
      swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[OPEN_DOOR_ICON]);
    } else {
      swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[OPEN_DOOR_ICON],
               AP_OPEN_DOOR);
    }
  }
  SetButtonFastHelpText(iActionIcons[OPEN_DOOR_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_OPEN_DOOR, BP_OPEN_DOOR, FALSE)) {
    DisableButton(iActionIcons[OPEN_DOOR_ICON]);
  }

  // Create button based on what is in our hands at the moment!
  iActionIcons[EXAMINE_DOOR_ICON] =
      QuickCreateButton(iIconImages[EXAMINE_DOOR_IMAGES], (int16_t)(iMenuAnchorX),
                        (int16_t)(iMenuAnchorY + 20), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[EXAMINE_DOOR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[EXAMINE_DOOR_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[EXAMINE_DOOR_ICON],
             AP_EXAMINE_DOOR);
  }
  SetButtonFastHelpText(iActionIcons[EXAMINE_DOOR_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_EXAMINE_DOOR, BP_EXAMINE_DOOR, FALSE) ||
      fClosingDoor || AM_AN_EPC(gOpenDoorMenu.pSoldier)) {
    DisableButton(iActionIcons[EXAMINE_DOOR_ICON]);
  }

  iActionIcons[BOOT_DOOR_ICON] =
      QuickCreateButton(iIconImages[BOOT_DOOR_IMAGES], (int16_t)(iMenuAnchorX),
                        (int16_t)(iMenuAnchorY + 40), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[BOOT_DOOR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[BOOT_DOOR_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[BOOT_DOOR_ICON],
             AP_BOOT_DOOR);
  }
  SetButtonFastHelpText(iActionIcons[BOOT_DOOR_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_BOOT_DOOR, BP_BOOT_DOOR, FALSE) || fClosingDoor ||
      AM_AN_EPC(gOpenDoorMenu.pSoldier)) {
    DisableButton(iActionIcons[BOOT_DOOR_ICON]);
  }

  iActionIcons[UNTRAP_DOOR_ICON] =
      QuickCreateButton(iIconImages[UNTRAP_DOOR_ICON], (int16_t)(iMenuAnchorX + 20),
                        (int16_t)(iMenuAnchorY + 40), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[UNTRAP_DOOR_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }

  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    swprintf(zDisp, ARR_SIZE(zDisp), pTacticalPopupButtonStrings[UNTRAP_DOOR_ICON]);
  } else {
    swprintf(zDisp, ARR_SIZE(zDisp), L"%s ( %d )", pTacticalPopupButtonStrings[UNTRAP_DOOR_ICON],
             AP_UNTRAP_DOOR);
  }
  SetButtonFastHelpText(iActionIcons[UNTRAP_DOOR_ICON], zDisp);

  if (!EnoughPoints(gOpenDoorMenu.pSoldier, AP_UNTRAP_DOOR, BP_UNTRAP_DOOR, FALSE) ||
      fClosingDoor || AM_AN_EPC(gOpenDoorMenu.pSoldier)) {
    DisableButton(iActionIcons[UNTRAP_DOOR_ICON]);
  }

  iActionIcons[CANCEL_ICON] =
      QuickCreateButton(iIconImages[CANCEL_IMAGES], (int16_t)(iMenuAnchorX + 20),
                        (int16_t)(iMenuAnchorY + 20), BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
                        DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnDoorMenuCallback);
  if (iActionIcons[CANCEL_ICON] == -1) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
    return;
  }
  SetButtonFastHelpText(iActionIcons[CANCEL_ICON], pTacticalPopupButtonStrings[CANCEL_ICON]);

  // LockTacticalInterface( );

  gfInOpenDoorMenu = TRUE;

  // Ignore scrolling
  gfIgnoreScrolling = TRUE;
}

void PopDownOpenDoorMenu() {
  if (gfInOpenDoorMenu) {
    UnLockPauseState();
    UnPauseGame();
    // UnPause timers as well....
    PauseTime(FALSE);

    RemoveButton(iActionIcons[USE_KEYRING_ICON]);
    RemoveButton(iActionIcons[USE_CROWBAR_ICON]);
    RemoveButton(iActionIcons[LOCKPICK_DOOR_ICON]);
    RemoveButton(iActionIcons[EXPLOSIVE_DOOR_ICON]);
    RemoveButton(iActionIcons[OPEN_DOOR_ICON]);
    RemoveButton(iActionIcons[EXAMINE_DOOR_ICON]);
    RemoveButton(iActionIcons[BOOT_DOOR_ICON]);
    RemoveButton(iActionIcons[UNTRAP_DOOR_ICON]);
    RemoveButton(iActionIcons[CANCEL_ICON]);

    // Turn off Ignore scrolling
    gfIgnoreScrolling = FALSE;

    // Rerender world
    SetRenderFlags(RENDER_FLAG_FULL);

    fInterfacePanelDirty = DIRTYLEVEL2;

    // UnLockTacticalInterface( );
    MSYS_RemoveRegion(&gMenuOverlayRegion);
  }

  gfInOpenDoorMenu = FALSE;
}

void RenderOpenDoorMenu() {
  if (gfInOpenDoorMenu) {
    BltVObjectFromIndex(vsFB, guiBUTTONBORDER, 0, gOpenDoorMenu.sX, gOpenDoorMenu.sY);

    // Mark buttons dirty!
    MarkAButtonDirty(iActionIcons[USE_KEYRING_ICON]);
    MarkAButtonDirty(iActionIcons[USE_CROWBAR_ICON]);
    MarkAButtonDirty(iActionIcons[LOCKPICK_DOOR_ICON]);
    MarkAButtonDirty(iActionIcons[EXPLOSIVE_DOOR_ICON]);
    MarkAButtonDirty(iActionIcons[OPEN_DOOR_ICON]);
    MarkAButtonDirty(iActionIcons[EXAMINE_DOOR_ICON]);
    MarkAButtonDirty(iActionIcons[BOOT_DOOR_ICON]);
    MarkAButtonDirty(iActionIcons[UNTRAP_DOOR_ICON]);
    MarkAButtonDirty(iActionIcons[CANCEL_ICON]);

    RenderButtons();

    // if game is paused, then render paused game text
    RenderPausedGameBox();

    InvalidateRegion(gOpenDoorMenu.sX, gOpenDoorMenu.sY, gOpenDoorMenu.sX + BUTTON_PANEL_WIDTH,
                     gOpenDoorMenu.sY + BUTTON_PANEL_HEIGHT);
  }
}

void CancelOpenDoorMenu() {
  // Signal end of event
  gOpenDoorMenu.fMenuHandled = 2;
}

void BtnDoorMenuCallback(GUI_BUTTON *btn, int32_t reason) {
  int32_t uiBtnID;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags |= BUTTON_CLICKED_ON;

    uiBtnID = btn->IDNum;

    // Popdown menu
    gOpenDoorMenu.fMenuHandled = TRUE;

    if (uiBtnID == iActionIcons[CANCEL_ICON]) {
      // OK, set cancle code!
      gOpenDoorMenu.fMenuHandled = 2;
    }

    // Switch on command....
    if (uiBtnID == iActionIcons[OPEN_DOOR_ICON]) {
      // Open door normally...
      // Check APs
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_OPEN_DOOR, BP_OPEN_DOOR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        if (gOpenDoorMenu.fClosingDoor) {
          ChangeSoldierState(gOpenDoorMenu.pSoldier,
                             GetAnimStateForInteraction(gOpenDoorMenu.pSoldier, TRUE, CLOSE_DOOR),
                             0, FALSE);
        } else {
          InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_OPEN);
        }
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[BOOT_DOOR_ICON]) {
      // Boot door
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_BOOT_DOOR, BP_BOOT_DOOR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_FORCE);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[USE_KEYRING_ICON]) {
      // Unlock door
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_UNLOCK_DOOR, BP_UNLOCK_DOOR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_UNLOCK);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[LOCKPICK_DOOR_ICON]) {
      // Lockpick
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_PICKLOCK, BP_PICKLOCK, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_LOCKPICK);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[EXAMINE_DOOR_ICON]) {
      // Lockpick
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_EXAMINE_DOOR, BP_EXAMINE_DOOR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_EXAMINE);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[EXPLOSIVE_DOOR_ICON]) {
      // Explode
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_EXPLODE_DOOR, BP_EXPLODE_DOOR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_EXPLODE);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[UNTRAP_DOOR_ICON]) {
      // Explode
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_UNTRAP_DOOR, BP_UNTRAP_DOOR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_UNTRAP);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    if (uiBtnID == iActionIcons[USE_CROWBAR_ICON]) {
      // Explode
      if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_USE_CROWBAR, BP_USE_CROWBAR, FALSE)) {
        // Set UI
        SetUIBusy((uint8_t)gOpenDoorMenu.pSoldier->ubID);

        InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_CROWBAR);
      } else {
        // OK, set cancle code!
        gOpenDoorMenu.fMenuHandled = 2;
      }
    }

    HandleOpenDoorMenu();
  }
}

BOOLEAN HandleOpenDoorMenu() {
  if (gOpenDoorMenu.fMenuHandled) {
    PopDownOpenDoorMenu();
    return (gOpenDoorMenu.fMenuHandled);
  }

  return (FALSE);
}

void RenderUIMessage(VIDEO_OVERLAY *pBlitter) {
  // Shade area first...
  ShadowVideoSurfaceRect(pBlitter->vsDestBuff, pBlitter->sX, pBlitter->sY,
                         pBlitter->sX + gusUIMessageWidth - 2,
                         pBlitter->sY + gusUIMessageHeight - 2);

  RenderMercPopUpBoxFromIndex(iUIMessageBox, pBlitter->sX, pBlitter->sY, pBlitter->vsDestBuff);

  InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusUIMessageWidth,
                   pBlitter->sY + gusUIMessageHeight);
}

void InternalBeginUIMessage(BOOLEAN fUseSkullIcon, wchar_t *pFontString, ...) {
  va_list argptr;
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  wchar_t MsgString[512];

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(MsgString, ARR_SIZE(MsgString), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  guiUIMessageTime = GetJA2Clock();
  guiUIMessageTimeDelay = CalcUIMessageDuration(MsgString);

  // Override it!
  OverrideMercPopupBox(&gpUIMessageOverrideMercBox);

  if (fUseSkullIcon) {
    SetPrepareMercPopupFlags(MERC_POPUP_PREPARE_FLAGS_MARGINS | MERC_POPUP_PREPARE_FLAGS_SKULLICON);
  } else {
    SetPrepareMercPopupFlags(MERC_POPUP_PREPARE_FLAGS_MARGINS | MERC_POPUP_PREPARE_FLAGS_STOPICON);
  }

  // Prepare text box
  iUIMessageBox =
      PrepareMercPopupBox(iUIMessageBox, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER,
                          MsgString, 200, 10, 0, 0, &gusUIMessageWidth, &gusUIMessageHeight);

  // Set it back!
  ResetOverrideMercPopupBox();

  if (giUIMessageOverlay != -1) {
    RemoveVideoOverlay(giUIMessageOverlay);

    giUIMessageOverlay = -1;
  }

  if (giUIMessageOverlay == -1) {
    memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

    // Set Overlay
    VideoOverlayDesc.sLeft = (640 - gusUIMessageWidth) / 2;
    VideoOverlayDesc.sTop = 150;
    VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + gusUIMessageWidth;
    VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + gusUIMessageHeight;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    VideoOverlayDesc.BltCallback = RenderUIMessage;

    giUIMessageOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
  }

  gfUseSkullIconMessage = fUseSkullIcon;
}

void BeginUIMessage(wchar_t *pFontString, ...) {
  va_list argptr;
  wchar_t MsgString[512];

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(MsgString, ARR_SIZE(MsgString), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  InternalBeginUIMessage(FALSE, MsgString);
}

void BeginMapUIMessage(uint8_t ubPosition, wchar_t *pFontString, ...) {
  va_list argptr;
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  wchar_t MsgString[512];

  memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));

  va_start(argptr, pFontString);  // Set up variable argument pointer
  vswprintf(MsgString, ARR_SIZE(MsgString), pFontString,
            argptr);  // process gprintf string (get output str)
  va_end(argptr);

  guiUIMessageTime = GetJA2Clock();
  guiUIMessageTimeDelay = CalcUIMessageDuration(MsgString);

  // Override it!
  OverrideMercPopupBox(&gpUIMessageOverrideMercBox);

  SetPrepareMercPopupFlags(MERC_POPUP_PREPARE_FLAGS_TRANS_BACK | MERC_POPUP_PREPARE_FLAGS_MARGINS);

  // Prepare text box
  iUIMessageBox =
      PrepareMercPopupBox(iUIMessageBox, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER,
                          MsgString, 200, 10, 0, 0, &gusUIMessageWidth, &gusUIMessageHeight);

  // Set it back!
  ResetOverrideMercPopupBox();

  if (giUIMessageOverlay == -1) {
    // Set Overlay
    VideoOverlayDesc.sLeft = 20 + MAP_VIEW_START_X + (MAP_VIEW_WIDTH - gusUIMessageWidth) / 2;

    VideoOverlayDesc.sTop = MAP_VIEW_START_Y + (MAP_VIEW_HEIGHT - gusUIMessageHeight) / 2;

    if (ubPosition == MSG_MAP_UI_POSITION_UPPER) {
      VideoOverlayDesc.sTop -= 100;
    } else if (ubPosition == MSG_MAP_UI_POSITION_LOWER) {
      VideoOverlayDesc.sTop += 100;
    }

    VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + gusUIMessageWidth;
    VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + gusUIMessageHeight;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    VideoOverlayDesc.BltCallback = RenderUIMessage;

    giUIMessageOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
  }
}

void EndUIMessage() {
  uint32_t uiClock = GetJA2Clock();

  if (giUIMessageOverlay != -1) {
    if (gfUseSkullIconMessage) {
      if ((uiClock - guiUIMessageTime) < 300) {
        return;
      }
    }

    //		DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Removing Overlay message") );

    RemoveVideoOverlay(giUIMessageOverlay);

    // Remove popup as well....
    if (iUIMessageBox != -1) {
      RemoveMercPopupBoxFromIndex(iUIMessageBox);
      iUIMessageBox = -1;
    }

    giUIMessageOverlay = -1;
  }
  // iUIMessageBox = -1;
}

#define PLAYER_TEAM_TIMER_INTTERUPT_GRACE (15000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_GRACE_PERIOD 1000
#define PLAYER_TEAM_TIMER_SEC_PER_TICKS 100
#define PLAYER_TEAM_TIMER_TICKS_PER_OK_MERC (15000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_TICKS_PER_NOTOK_MERC (5000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_TICKS_FROM_END_TO_START_BEEP (5000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_TIME_BETWEEN_BEEPS (500)
#define PLAYER_TEAM_TIMER_TICKS_PER_ENEMY (2000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)

BOOLEAN AddTopMessage(uint8_t ubType, wchar_t *pzString) {
  uint32_t cnt;
  BOOLEAN fFound = FALSE;

  // Set time of last update
  gTopMessage.uiTimeOfLastUpdate = GetJA2Clock();

  // Set flag to animate down...
  // gTopMessage.bAnimate = -1;
  // gTopMessage.bYPos		 = 2;

  gTopMessage.bAnimate = 0;
  gTopMessage.bYPos = 20;
  gTopMessage.fCreated = TRUE;

  fFound = TRUE;
  cnt = 0;

  if (fFound) {
    gTopMessage.bCurrentMessage = (int8_t)cnt;

    gTacticalStatus.ubTopMessageType = ubType;
    gTacticalStatus.fInTopMessage = TRUE;

    // Copy string
    wcscpy(gTacticalStatus.zTopMessageString, pzString);

    CreateTopMessage(gTopMessage.dest, ubType, pzString);

    return (TRUE);
  }

  return (FALSE);
}

void CreateTopMessage(struct JSurface *dest, uint8_t ubType, wchar_t *psString) {
  uint32_t uiBAR, uiPLAYERBAR, uiINTBAR;
  int16_t sX, sY;
  int32_t cnt2;
  int16_t sBarX = 0;
  uint32_t uiBarToUseInUpDate = 0;
  BOOLEAN fDoLimitBar = FALSE;

  float dNumStepsPerEnemy, dLength, dCurSize;

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\rect.sti"), &uiBAR))
    AssertMsg(0, "Missing INTERFACE\\rect.sti");

  // if ( gGameOptions.fTurnTimeLimit )
  {
    if (!AddVObject(CreateVObjectFromFile("INTERFACE\\timebargreen.sti"), &uiPLAYERBAR))
      AssertMsg(0, "Missing INTERFACE\\timebargreen.sti");
  }

  if (!AddVObject(CreateVObjectFromFile("INTERFACE\\timebaryellow.sti"), &uiINTBAR))
    AssertMsg(0, "Missing INTERFACE\\timebaryellow.sti");

  // Change dest buffer
  SetFontDestBuffer(dest, 0, 0, 640, 20, FALSE);
  SetFont(TINYFONT1);

  switch (ubType) {
    case COMPUTER_TURN_MESSAGE:
    case COMPUTER_INTERRUPT_MESSAGE:
    case MILITIA_INTERRUPT_MESSAGE:
    case AIR_RAID_TURN_MESSAGE:

      // Render rect into surface
      BltVObjectFromIndex(dest, uiBAR, 0, 0, 0);
      SetFontBackground(FONT_MCOLOR_BLACK);
      SetFontForeground(FONT_MCOLOR_WHITE);
      uiBarToUseInUpDate = uiBAR;
      fDoLimitBar = TRUE;
      break;

    case PLAYER_INTERRUPT_MESSAGE:

      // Render rect into surface
      BltVObjectFromIndex(dest, uiINTBAR, 0, 0, 0);
      SetFontBackground(FONT_MCOLOR_BLACK);
      SetFontForeground(FONT_MCOLOR_BLACK);
      SetFontShadow(NO_SHADOW);
      uiBarToUseInUpDate = uiINTBAR;
      break;

    case PLAYER_TURN_MESSAGE:

      // Render rect into surface
      BltVObjectFromIndex(dest, uiPLAYERBAR, 0, 0, 0);
      SetFontBackground(FONT_MCOLOR_BLACK);
      SetFontForeground(FONT_MCOLOR_BLACK);
      SetFontShadow(NO_SHADOW);
      uiBarToUseInUpDate = uiPLAYERBAR;
      break;
  }

  if (gGameOptions.fTurnTimeLimit) {
    if (ubType == PLAYER_TURN_MESSAGE || ubType == PLAYER_INTERRUPT_MESSAGE) {
      fDoLimitBar = TRUE;
    }
  }

  if (fDoLimitBar) {
    dNumStepsPerEnemy =
        (float)((float)PROG_BAR_LENGTH / (float)gTacticalStatus.usTactialTurnLimitMax);

    // Alrighty, do some fun stuff!

    // Render end peice
    sBarX = PROG_BAR_START_X;
    BltVObjectFromIndex(dest, uiBarToUseInUpDate, 1, sBarX, PROG_BAR_START_Y);

    // Determine Length
    dLength = (gTacticalStatus.usTactialTurnLimitCounter) * dNumStepsPerEnemy;

    dCurSize = 0;
    cnt2 = 0;

    while (dCurSize < dLength) {
      sBarX++;

      // Check sBarX, ( just as a precaution )
      if (sBarX > 639) {
        break;
      }

      BltVObjectFromIndex(dest, uiBarToUseInUpDate, (int16_t)(2 + cnt2), sBarX, PROG_BAR_START_Y);

      dCurSize++;
      cnt2++;

      if (cnt2 == 10) {
        cnt2 = 0;
      }
    }

    // Do end...
    if (gTacticalStatus.usTactialTurnLimitCounter == gTacticalStatus.usTactialTurnLimitMax) {
      sBarX++;
      BltVObjectFromIndex(dest, uiBarToUseInUpDate, (int16_t)(2 + cnt2), sBarX, PROG_BAR_START_Y);
      sBarX++;
      BltVObjectFromIndex(dest, uiBarToUseInUpDate, (int16_t)(12), sBarX, PROG_BAR_START_Y);
    }
  }

  // Delete rect
  DeleteVideoObjectFromIndex(uiBAR);
  DeleteVideoObjectFromIndex(uiINTBAR);

  // if ( gGameOptions.fTurnTimeLimit )
  { DeleteVideoObjectFromIndex(uiPLAYERBAR); }

  // Draw text....
  FindFontCenterCoordinates(320, 7, 1, 1, psString, TINYFONT1, &sX, &sY);
  mprintf(sX, sY, psString);

  // Change back...
  SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

  // Done!
  SetFontShadow(DEFAULT_SHADOW);

  gfTopMessageDirty = TRUE;
}

void TurnExpiredCallBack(uint8_t bExitValue) {
  // End turn...
  UIHandleEndTurn(NULL);
}

void CheckForAndHandleEndPlayerTimeLimit() {
  if (gTacticalStatus.fInTopMessage) {
    if (gGameOptions.fTurnTimeLimit) {
      if (gTacticalStatus.ubTopMessageType == PLAYER_TURN_MESSAGE ||
          gTacticalStatus.ubTopMessageType == PLAYER_INTERRUPT_MESSAGE) {
        if (gTacticalStatus.usTactialTurnLimitCounter ==
            (gTacticalStatus.usTactialTurnLimitMax - 1)) {
          // ATE: increase this so that we don't go into here again...
          gTacticalStatus.usTactialTurnLimitCounter++;

          // OK, set message that time limit has expired....
          // DoMessageBox( MSG_BOX_BASIC_STYLE, L"Turn has Expired!", GAME_SCREEN, ( uint8_t
          // )MSG_BOX_FLAG_OK, TurnExpiredCallBack, NULL );

          // End turn...
          UIHandleEndTurn(NULL);
        }
      }
    }
  }
}

void HandleTopMessages() {
  // OK, is out current count > 0 ?
  if (gTacticalStatus.fInTopMessage) {
    // gfTopMessageDirty = TRUE;

    // ATE: If we are told to go into top message, but we have not
    // initialized it yet...
    // This is mostly for loading saved games.....
    if (!gTopMessage.fCreated) {
      gfTopMessageDirty = TRUE;
      AddTopMessage(gTacticalStatus.ubTopMessageType, gTacticalStatus.zTopMessageString);
    }

    if (gTacticalStatus.ubTopMessageType == COMPUTER_TURN_MESSAGE ||
        gTacticalStatus.ubTopMessageType == COMPUTER_INTERRUPT_MESSAGE ||
        gTacticalStatus.ubTopMessageType == MILITIA_INTERRUPT_MESSAGE ||
        gTacticalStatus.ubTopMessageType == AIR_RAID_TURN_MESSAGE) {
      // OK, update timer.....
      if (TIMECOUNTERDONE(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS)) {
        RESETTIMECOUNTER(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS);

        // Update counter....
        if (gTacticalStatus.usTactialTurnLimitCounter < gTacticalStatus.usTactialTurnLimitMax) {
          gTacticalStatus.usTactialTurnLimitCounter++;
        }

        // Check if we have reach limit...
        if (gTacticalStatus.usTactialTurnLimitCounter >=
            ((gubProgCurEnemy)*PLAYER_TEAM_TIMER_TICKS_PER_ENEMY)) {
          gTacticalStatus.usTactialTurnLimitCounter =
              ((gubProgCurEnemy)*PLAYER_TEAM_TIMER_TICKS_PER_ENEMY);
        }

        CreateTopMessage(gTopMessage.dest, gTacticalStatus.ubTopMessageType,
                         gTacticalStatus.zTopMessageString);
      }
    } else if (gGameOptions.fTurnTimeLimit) {
      if (gTacticalStatus.ubTopMessageType == PLAYER_TURN_MESSAGE ||
          gTacticalStatus.ubTopMessageType == PLAYER_INTERRUPT_MESSAGE) {
        if (!gfUserTurnRegionActive && !AreWeInAUIMenu()) {
          // Check Grace period...
          if ((GetJA2Clock() - gTacticalStatus.uiTactialTurnLimitClock) >
              PLAYER_TEAM_TIMER_GRACE_PERIOD) {
            gTacticalStatus.uiTactialTurnLimitClock = 0;

            if (TIMECOUNTERDONE(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS)) {
              RESETTIMECOUNTER(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS);

              if (gTacticalStatus.fTactialTurnLimitStartedBeep) {
                if ((GetJA2Clock() - gTopMessage.uiTimeSinceLastBeep) >
                    PLAYER_TEAM_TIMER_TIME_BETWEEN_BEEPS) {
                  gTopMessage.uiTimeSinceLastBeep = GetJA2Clock();

                  // Start sample....
                  PlayJA2SampleFromFile("SOUNDS\\TURN_NEAR_END.WAV", RATE_11025, HIGHVOLUME, 1,
                                        MIDDLEPAN);
                }
              }

              // OK, have we gone past the time to
              if (!gTacticalStatus.fTactialTurnLimitStartedBeep &&
                  (gTacticalStatus.usTactialTurnLimitMax -
                   gTacticalStatus.usTactialTurnLimitCounter) <
                      PLAYER_TEAM_TIMER_TICKS_FROM_END_TO_START_BEEP) {
                gTacticalStatus.fTactialTurnLimitStartedBeep = TRUE;

                gTopMessage.uiTimeSinceLastBeep = GetJA2Clock();
              }

              // Update counter....
              if (gTacticalStatus.usTactialTurnLimitCounter <
                  gTacticalStatus.usTactialTurnLimitMax) {
                gTacticalStatus.usTactialTurnLimitCounter++;
              }

              CreateTopMessage(gTopMessage.dest, gTacticalStatus.ubTopMessageType,
                               gTacticalStatus.zTopMessageString);

              // Have we reached max?
              if (gTacticalStatus.usTactialTurnLimitCounter ==
                  (gTacticalStatus.usTactialTurnLimitMax - 1)) {
                // IF we are not in lock ui mode....
                CheckForAndHandleEndPlayerTimeLimit();
              }
            }
          }
        }
      }
    }

    // Set redner viewport value
    gsVIEWPORT_WINDOW_START_Y = 20;

    // Check if we have scrolled...
    if (gTopMessage.sWorldRenderX != gsRenderCenterX ||
        gTopMessage.sWorldRenderY != gsRenderCenterY) {
      gfTopMessageDirty = TRUE;
    }

    if (gfTopMessageDirty) {
      gTopMessage.sWorldRenderX = gsRenderCenterX;
      gTopMessage.sWorldRenderY = gsRenderCenterY;

      // Redner!
      struct Rect SrcRect;
      SrcRect.left = 0;
      SrcRect.top = 20 - gTopMessage.bYPos;
      SrcRect.right = 640;
      SrcRect.bottom = 20;
      BltVSurfaceToVSurfaceSubrect(vsFB, gTopMessage.dest, 0, 0, &SrcRect);

      // Save to save buffer....
      SrcRect.left = 0;
      SrcRect.top = 0;
      SrcRect.right = 640;
      SrcRect.bottom = 20;
      BltVSurfaceToVSurfaceSubrect(vsSaveBuffer, vsFB, 0, 0, &SrcRect);

      InvalidateRegion(0, 0, 640, 20);

      gfTopMessageDirty = FALSE;
    }

  } else {
    // Set redner viewport value
    gsVIEWPORT_WINDOW_START_Y = 0;
  }
}

void EndTopMessage() {
  // OK, end the topmost message!
  if (gTacticalStatus.fInTopMessage) {
    // Are we the last?
    // if ( gTopMessage.bCurrentMessage == 1 )
    {
      // We are....
      // Re-render our strip and then copy to the save buffer...
      gsVIEWPORT_WINDOW_START_Y = 0;
      gTacticalStatus.fInTopMessage = FALSE;

      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }
}

BOOLEAN InTopMessageBarAnimation() {
  if (gTacticalStatus.fInTopMessage) {
    if (gTopMessage.bAnimate != 0) {
      HandleTopMessages();

      return (TRUE);
    }
  }

  return (FALSE);
}

void PauseRT(BOOLEAN fPause) {
  // StopMercAnimation( fPause );

  if (fPause) {
    //	PauseGame( );
  } else {
    //	UnPauseGame( );
  }
}

void InitEnemyUIBar(uint8_t ubNumEnemies, uint8_t ubDoneEnemies) {
  // OK, set value
  gubProgNumEnemies = ubNumEnemies + ubDoneEnemies;
  gubProgCurEnemy = ubDoneEnemies;
  gfProgBarActive = TRUE;

  gTacticalStatus.usTactialTurnLimitCounter = ubDoneEnemies * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY;
  gTacticalStatus.usTactialTurnLimitMax =
      ((ubNumEnemies + ubDoneEnemies) * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY);
}

void UpdateEnemyUIBar() {
  // Are we active?
  if (gfProgBarActive) {
    // OK, update team limit counter....
    gTacticalStatus.usTactialTurnLimitCounter =
        (gubProgCurEnemy * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY);

    gubProgCurEnemy++;
  }

  // Do we have an active enemy bar?
  if (gTacticalStatus.fInTopMessage) {
    if (gTacticalStatus.ubTopMessageType == COMPUTER_TURN_MESSAGE) {
      // Update message!
      CreateTopMessage(gTopMessage.dest, COMPUTER_TURN_MESSAGE, gTacticalStatus.zTopMessageString);
    }
  }
}

void InitPlayerUIBar(BOOLEAN fInterrupt) {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;
  int8_t bNumOK = 0, bNumNotOK = 0;

  if (!gGameOptions.fTurnTimeLimit) {
    if (fInterrupt == TRUE) {
      AddTopMessage(PLAYER_INTERRUPT_MESSAGE, Message[STR_INTERRUPT]);
    } else {
      // EndTopMessage();
      AddTopMessage(PLAYER_TURN_MESSAGE, TeamTurnString[0]);
    }
    return;
  }

  // OK, calculate time....
  if (!fInterrupt || gTacticalStatus.usTactialTurnLimitMax == 0) {
    gTacticalStatus.usTactialTurnLimitCounter = 0;

    // IF IT'S THE SELECTED GUY, MAKE ANOTHER SELECTED!
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

    // look for all mercs on the same team,
    for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pTeamSoldier++) {
      // Are we active and in sector.....
      if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
        if (pTeamSoldier->bLife < OKLIFE) {
          bNumNotOK++;
        } else {
          bNumOK++;
        }
      }
    }

    gTacticalStatus.usTactialTurnLimitMax = (bNumOK * PLAYER_TEAM_TIMER_TICKS_PER_OK_MERC) +
                                            (bNumNotOK * PLAYER_TEAM_TIMER_TICKS_PER_NOTOK_MERC);
  } else {
    if (gTacticalStatus.usTactialTurnLimitCounter > PLAYER_TEAM_TIMER_INTTERUPT_GRACE) {
      gTacticalStatus.usTactialTurnLimitCounter -= PLAYER_TEAM_TIMER_INTTERUPT_GRACE;
    }
  }

  gTacticalStatus.uiTactialTurnLimitClock = 0;
  gTacticalStatus.fTactialTurnLimitStartedBeep = FALSE;

  // RESET COIUNTER...
  RESETTIMECOUNTER(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS);

  // OK, set value
  if (fInterrupt != TRUE) {
    AddTopMessage(PLAYER_TURN_MESSAGE, TeamTurnString[0]);
  } else {
    AddTopMessage(PLAYER_INTERRUPT_MESSAGE, Message[STR_INTERRUPT]);
  }
}

void MovementMenuBackregionCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    CancelMovementMenu();
  }
}

void DoorMenuBackregionCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    CancelOpenDoorMenu();
  }
}

wchar_t *GetSoldierHealthString(struct SOLDIERTYPE *pSoldier) {
  int32_t cnt, cntStart;
  if (pSoldier->bLife == pSoldier->bLifeMax) {
    cntStart = 4;
  } else {
    cntStart = 0;
  }
  // Show health on others.........
  for (cnt = cntStart; cnt < 6; cnt++) {
    if (pSoldier->bLife < bHealthStrRanges[cnt]) {
      break;
    }
  }
  return zHealthStr[cnt];
}

typedef struct {
  int8_t bHeight;
  int8_t bPower;
  int16_t sGridNo;
  uint8_t ubLevel;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fShowHeight;
  BOOLEAN fShowPower;
  BOOLEAN fActiveHeightBar;
  BOOLEAN fActivePowerBar;
  BOOLEAN fAtEndHeight;
  int16_t sTargetGridNo;
  float dInitialForce;
  float dForce;
  float dDegrees;
  float dMaxForce;
  uint8_t ubPowerIndex;

} AIMCUBE_UI_DATA;

static BOOLEAN gfInAimCubeUI = FALSE;
static AIMCUBE_UI_DATA gCubeUIData;

#define GET_CUBES_HEIGHT_FROM_UIHEIGHT(h) (32 + (h * 64))

void CalculateAimCubeUIPhysics() {
  uint8_t ubHeight;

  ubHeight = GET_CUBES_HEIGHT_FROM_UIHEIGHT(gCubeUIData.bHeight);

  if (gCubeUIData.fActiveHeightBar) {
    // OK, determine which power to use.....
    // TODO this: take force / max force * 10....
    gCubeUIData.ubPowerIndex = (uint8_t)(gCubeUIData.dForce / gCubeUIData.dMaxForce * 10);
  }

  if (gCubeUIData.fActivePowerBar) {
    gCubeUIData.dForce = (gCubeUIData.dMaxForce * ((float)gCubeUIData.ubPowerIndex / (float)10));

    // Limit to the max force...
    if (gCubeUIData.dForce > gCubeUIData.dMaxForce) {
      gCubeUIData.dForce = gCubeUIData.dMaxForce;
    }

    gCubeUIData.dDegrees = (float)CalculateLaunchItemAngle(
        gCubeUIData.pSoldier, gCubeUIData.sGridNo, ubHeight, gCubeUIData.dForce,
        &(gCubeUIData.pSoldier->inv[HANDPOS]), &(gCubeUIData.sTargetGridNo));
  }
}

int16_t GetInAimCubeUIGridNo() { return (gCubeUIData.sGridNo); }

BOOLEAN InAimCubeUI() { return (gfInAimCubeUI); }

BOOLEAN AimCubeUIClick() {
  if (!gfInAimCubeUI) {
    return (FALSE);
  }

  // If we have clicked, and we are only on height, continue with power
  if (gCubeUIData.fActiveHeightBar && gCubeUIData.bHeight != 0) {
    gCubeUIData.fShowPower = TRUE;
    gCubeUIData.fActiveHeightBar = FALSE;
    gCubeUIData.fActivePowerBar = TRUE;

    return (FALSE);
  } else {
    return (TRUE);
  }
}

void BeginAimCubeUI(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t ubLevel,
                    uint8_t bStartPower, int8_t bStartHeight) {
  gfInAimCubeUI = TRUE;

  gCubeUIData.sGridNo = sGridNo;
  gCubeUIData.ubLevel = ubLevel;
  gCubeUIData.pSoldier = pSoldier;
  gCubeUIData.bPower = bStartPower;
  gCubeUIData.bHeight = bStartHeight;
  gCubeUIData.fShowHeight = TRUE;
  gCubeUIData.fShowPower = FALSE;
  gCubeUIData.fActivePowerBar = FALSE;
  gCubeUIData.fActiveHeightBar = TRUE;
  gCubeUIData.fAtEndHeight = FALSE;
  gCubeUIData.dDegrees = (float)PI / 4;

  // Calculate Iniital force....
  CalculateAimCubeUIPhysics();
}

void EndAimCubeUI() { gfInAimCubeUI = FALSE; }

void IncrementAimCubeUI() {
  if (gCubeUIData.fActiveHeightBar) {
    // Cycle the last height yellow once
    if (gCubeUIData.bHeight == 3) {
      if (gCubeUIData.fAtEndHeight) {
        gCubeUIData.bHeight = 0;
        gCubeUIData.fAtEndHeight = 0;
      } else {
        gCubeUIData.fAtEndHeight = TRUE;
      }
    } else {
      gCubeUIData.bHeight++;
    }

    CalculateAimCubeUIPhysics();
  }

  if (gCubeUIData.fActivePowerBar) {
    if (gCubeUIData.ubPowerIndex == 10) {
      // Start back to basic7
      gCubeUIData.dDegrees = (float)(PI / 4);
      gCubeUIData.dInitialForce = gCubeUIData.dForce;

      // OK, determine which power to use.....
      // TODO this: take force / max force * 10....
      gCubeUIData.ubPowerIndex = (uint8_t)(gCubeUIData.dForce / gCubeUIData.dMaxForce * 10);
    }

    // Cycle the last height yellow once
    gCubeUIData.ubPowerIndex++;

    CalculateAimCubeUIPhysics();
  }
}

void SetupAimCubeAI() {
  if (gfInAimCubeUI) {
    AddTopmostToHead(gCubeUIData.sTargetGridNo, FIRSTPOINTERS2);
    gpWorldLevelData[gCubeUIData.sTargetGridNo].pTopmostHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
    gpWorldLevelData[gCubeUIData.sTargetGridNo].pTopmostHead->ubNaturalShadeLevel =
        DEFAULT_SHADE_LEVEL;

    // AddTopmostToHead( gCubeUIData.sGridNo, FIRSTPOINTERS2 );
    // gpWorldLevelData[ gCubeUIData.sGridNo ].pTopmostHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
    // gpWorldLevelData[ gCubeUIData.sGridNo
    // ].pTopmostHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
  }
}

void ResetAimCubeAI() {
  if (gfInAimCubeUI) {
    RemoveTopmost(gCubeUIData.sTargetGridNo, FIRSTPOINTERS2);
    // RemoveTopmost( gCubeUIData.sGridNo, FIRSTPOINTERS2 );
  }
}

void RenderAimCubeUI() {
  int16_t sScreenX, sScreenY;
  int32_t cnt;
  int16_t sBarHeight;
  int32_t iBack;
  int8_t bGraphicNum;

  if (gfInAimCubeUI) {
    // OK, given height
    if (gCubeUIData.fShowHeight) {
      // Determine screen location....
      GetGridNoScreenPos(gCubeUIData.sGridNo, gCubeUIData.ubLevel, &sScreenX, &sScreenY);

      // Save background
      iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sScreenX, (int16_t)(sScreenY - 70),
                                     (int16_t)(sScreenX + 40), (int16_t)(sScreenY + 50));
      if (iBack != -1) {
        SetBackgroundRectFilled(iBack);
      }

      sBarHeight = 0;
      bGraphicNum = 0;

      if (gCubeUIData.bHeight == 3 && gCubeUIData.fAtEndHeight) {
        bGraphicNum = 1;
      }

      // Do first level....
      BltVObjectFromIndex(vsFB, guiAIMCUBES, bGraphicNum, sScreenX, (sScreenY + sBarHeight));
      sBarHeight -= 3;
      BltVObjectFromIndex(vsFB, guiAIMCUBES, bGraphicNum, sScreenX, (sScreenY + sBarHeight));

      // Loop through height.....
      for (cnt = 1; cnt <= gCubeUIData.bHeight; cnt++) {
        sBarHeight -= 3;
        BltVObjectFromIndex(vsFB, guiAIMCUBES, bGraphicNum, sScreenX, (sScreenY + sBarHeight));
        sBarHeight -= 3;
        BltVObjectFromIndex(vsFB, guiAIMCUBES, bGraphicNum, sScreenX, (sScreenY + sBarHeight));
        sBarHeight -= 3;
        BltVObjectFromIndex(vsFB, guiAIMCUBES, bGraphicNum, sScreenX, (sScreenY + sBarHeight));
        sBarHeight -= 3;
        BltVObjectFromIndex(vsFB, guiAIMCUBES, bGraphicNum, sScreenX, (sScreenY + sBarHeight));
      }
    }

    if (gCubeUIData.fShowPower) {
      sBarHeight = -50;

      BltVObjectFromIndex(vsFB, guiAIMBARS, gCubeUIData.ubPowerIndex, sScreenX,
                          (sScreenY + sBarHeight));
    }
  }
}

void GetLaunchItemParamsFromUI() {}

static BOOLEAN gfDisplayPhysicsUI = FALSE;
static int16_t gsPhysicsImpactPointGridNo;
static int8_t gbPhysicsImpactPointLevel;
static BOOLEAN gfBadPhysicsCTGT = FALSE;

void BeginPhysicsTrajectoryUI(int16_t sGridNo, int8_t bLevel, BOOLEAN fBadCTGT) {
  gfDisplayPhysicsUI = TRUE;
  gsPhysicsImpactPointGridNo = sGridNo;
  gbPhysicsImpactPointLevel = bLevel;
  gfBadPhysicsCTGT = fBadCTGT;
}

void EndPhysicsTrajectoryUI() { gfDisplayPhysicsUI = FALSE; }

void SetupPhysicsTrajectoryUI() {
  if (gfDisplayPhysicsUI && gfUIHandlePhysicsTrajectory) {
    if (gbPhysicsImpactPointLevel == 0) {
      if (gfBadPhysicsCTGT) {
        AddTopmostToHead(gsPhysicsImpactPointGridNo, FIRSTPOINTERS12);
      } else {
        AddTopmostToHead(gsPhysicsImpactPointGridNo, FIRSTPOINTERS8);
      }
      gpWorldLevelData[gsPhysicsImpactPointGridNo].pTopmostHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsPhysicsImpactPointGridNo].pTopmostHead->ubNaturalShadeLevel =
          DEFAULT_SHADE_LEVEL;
    } else {
      if (gfBadPhysicsCTGT) {
        AddOnRoofToHead(gsPhysicsImpactPointGridNo, FIRSTPOINTERS12);
      } else {
        AddOnRoofToHead(gsPhysicsImpactPointGridNo, FIRSTPOINTERS8);
      }
      gpWorldLevelData[gsPhysicsImpactPointGridNo].pOnRoofHead->ubShadeLevel = DEFAULT_SHADE_LEVEL;
      gpWorldLevelData[gsPhysicsImpactPointGridNo].pOnRoofHead->ubNaturalShadeLevel =
          DEFAULT_SHADE_LEVEL;
    }
  }
}

void ResetPhysicsTrajectoryUI() {
  if (gfDisplayPhysicsUI) {
    RemoveTopmost(gsPhysicsImpactPointGridNo, FIRSTPOINTERS8);
    RemoveTopmost(gsPhysicsImpactPointGridNo, FIRSTPOINTERS12);
    RemoveOnRoof(gsPhysicsImpactPointGridNo, FIRSTPOINTERS8);
    RemoveOnRoof(gsPhysicsImpactPointGridNo, FIRSTPOINTERS12);
  }
}

void DirtyTopMessage() { gTopMessage.fCreated = FALSE; }

uint32_t CalcUIMessageDuration(wchar_t *wString) {
  // base + X per letter
  return (1000 + 50 * wcslen(wString));
}

BOOLEAN gfMultipurposeLocatorOn = FALSE;
uint32_t guiMultiPurposeLocatorLastUpdate;
int8_t gbMultiPurposeLocatorFrame;
int16_t gsMultiPurposeLocatorGridNo;
int8_t gbMultiPurposeLocatorLevel;
int8_t gbMultiPurposeLocatorCycles;

void BeginMultiPurposeLocator(int16_t sGridNo, int8_t bLevel, BOOLEAN fSlideTo) {
  guiMultiPurposeLocatorLastUpdate = 0;
  gbMultiPurposeLocatorCycles = 0;
  gbMultiPurposeLocatorFrame = 0;
  gfMultipurposeLocatorOn = TRUE;

  gsMultiPurposeLocatorGridNo = sGridNo;
  gbMultiPurposeLocatorLevel = bLevel;

  if (fSlideTo) {
    // FIRST CHECK IF WE ARE ON SCREEN
    if (GridNoOnScreen(sGridNo)) {
      return;
    }

    // sGridNo here for DG compatibility
    gTacticalStatus.sSlideTarget = sGridNo;
    gTacticalStatus.sSlideReason = NOBODY;

    // Plot new path!
    gfPlotNewMovement = TRUE;
  }
}

void HandleMultiPurposeLocator() {
  uint32_t uiClock;

  if (!gfMultipurposeLocatorOn) {
    return;
  }

  // Update radio locator
  uiClock = GetJA2Clock();

  // Update frame values!
  if ((uiClock - guiMultiPurposeLocatorLastUpdate) > 80) {
    guiMultiPurposeLocatorLastUpdate = uiClock;

    // Update frame
    gbMultiPurposeLocatorFrame++;

    if (gbMultiPurposeLocatorFrame == 5) {
      gbMultiPurposeLocatorFrame = 0;
      gbMultiPurposeLocatorCycles++;
    }

    if (gbMultiPurposeLocatorCycles == 8) {
      gfMultipurposeLocatorOn = FALSE;
    }
  }
}

void RenderTopmostMultiPurposeLocator() {
  float dOffsetX, dOffsetY;
  float dTempX_S, dTempY_S;
  int16_t sX, sY, sXPos, sYPos;
  int32_t iBack;

  if (!gfMultipurposeLocatorOn) {
    return;
  }

  ConvertGridNoToCenterCellXY(gsMultiPurposeLocatorGridNo, &sX, &sY);

  dOffsetX = (float)(sX - gsRenderCenterX);
  dOffsetY = (float)(sY - gsRenderCenterY);

  // Calculate guy's position
  FloatFromCellToScreenCoordinates(dOffsetX, dOffsetY, &dTempX_S, &dTempY_S);

  sXPos = ((gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2) + (int16_t)dTempX_S;
  sYPos = ((gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2) + (int16_t)dTempY_S -
          gpWorldLevelData[gsMultiPurposeLocatorGridNo].sHeight;

  // Adjust for offset position on screen
  sXPos -= gsRenderWorldOffsetX;
  sYPos -= gsRenderWorldOffsetY;

  // Adjust for render height
  sYPos += gsRenderHeight;

  // Adjust for level height
  if (gbMultiPurposeLocatorLevel) {
    sYPos -= ROOF_LEVEL_HEIGHT;
  }

  // Center circle!
  sXPos -= 20;
  sYPos -= 20;

  iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sXPos, sYPos, (int16_t)(sXPos + 40),
                                 (int16_t)(sYPos + 40));
  if (iBack != -1) {
    SetBackgroundRectFilled(iBack);
  }

  BltVObjectFromIndex(vsFB, guiRADIO, gbMultiPurposeLocatorFrame, sXPos, sYPos);
}
