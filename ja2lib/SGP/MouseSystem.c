// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/MouseSystem.h"

#include <memory.h>
#include <stdio.h>

#include "JAScreens.h"
#include "Local.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Line.h"
#include "SGP/MemMan.h"
#include "SGP/Timer.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "TileEngine/RenderDirty.h"
#include "UI.h"
#include "Utils/FontControl.h"

#define BASE_REGION_FLAGS (MSYS_REGION_ENABLED | MSYS_SET_CURSOR)

// Kris:	Nov 31, 1999 -- Added support for double clicking
//
// Max double click delay (in milliseconds) to be considered a double click
#define MSYS_DOUBLECLICK_DELAY 400
//
// Records and stores the last place the user clicked.  These values are compared to the current
// click to determine if a double click event has been detected.
struct MOUSE_REGION *gpRegionLastLButtonDown = NULL;
struct MOUSE_REGION *gpRegionLastLButtonUp = NULL;
uint32_t guiRegionLastLButtonDownTime = 0;

extern void ReleaseAnchorMode();  // private function used here (implemented in Button System.c)

// number of lines in height help text will be
int16_t GetNumberOfLinesInHeight(wchar_t *pStringA);
int16_t GetWidthOfString(wchar_t *pStringA);
void DisplayHelpTokenizedString(wchar_t *pStringA, int16_t sX, int16_t sY);

int32_t MSYS_ScanForID = FALSE;
int32_t MSYS_CurrentID = MSYS_ID_SYSTEM;

int16_t MSYS_CurrentMX = 0;
int16_t MSYS_CurrentMY = 0;
int16_t MSYS_CurrentButtons = 0;
int16_t MSYS_Action = 0;

BOOLEAN MSYS_SystemInitialized = FALSE;
BOOLEAN MSYS_UseMouseHandlerHook = FALSE;

BOOLEAN MSYS_Mouse_Grabbed = FALSE;
struct MOUSE_REGION *MSYS_GrabRegion = NULL;

uint16_t gusClickedIDNumber;
BOOLEAN gfClickedModeOn = FALSE;

struct MOUSE_REGION *MSYS_RegList = NULL;

struct MOUSE_REGION *MSYS_PrevRegion = NULL;
struct MOUSE_REGION *MSYS_CurrRegion = NULL;

// When set, the fast help text will be instantaneous, if consecutive regions with help text are
// hilighted.  It is set, whenever the timer for the first help button expires, and the mode is
// cleared as soon as the cursor moves into no region or a region with no helptext.
BOOLEAN gfPersistantFastHelpMode;

int16_t gsFastHelpDelay = 600;  // In timer ticks
BOOLEAN gfShowFastHelp = TRUE;

// help text is done, now execute callback, if there is one
void ExecuteMouseHelpEndCallBack(struct MOUSE_REGION *region);

// Kris:
// NOTE:  This doesn't really need to be here, however, it is a good indication that
// when an error appears here, that you need to go below to the init code and initialize the
// values there as well.  That's the only reason why I left this here.
struct MOUSE_REGION MSYS_SystemBaseRegion = {MSYS_ID_SYSTEM,
                                             MSYS_PRIORITY_SYSTEM,
                                             BASE_REGION_FLAGS,
                                             -32767,
                                             -32767,
                                             32767,
                                             32767,
                                             0,
                                             0,
                                             0,
                                             0,
                                             0,
                                             0,
                                             MSYS_NO_CALLBACK,
                                             MSYS_NO_CALLBACK,
                                             {0, 0, 0, 0},
                                             0,
                                             0,
                                             -1,
                                             MSYS_NO_CALLBACK,
                                             NULL,
                                             NULL};

BOOLEAN gfRefreshUpdate = FALSE;

// Kris:  December 3, 1997
// Special internal debugging utilities that will ensure that you don't attempt to delete
// an already deleted region.  It will also ensure that you don't create an identical region
// that already exists.
// TO REMOVE ALL DEBUG FUNCTIONALITY:  simply comment out MOUSESYSTEM_DEBUGGING definition
#ifdef _DEBUG
#define MOUSESYSTEM_DEBUGGING
#endif

//======================================================================================================
//	MSYS_Init
//
//	Initialize the mouse system.
//
int32_t MSYS_Init(void) {
  RegisterDebugTopic(TOPIC_MOUSE_SYSTEM, "Mouse Region System");

  if (MSYS_RegList != NULL) MSYS_TrashRegList();

  MSYS_CurrentID = MSYS_ID_SYSTEM;
  MSYS_ScanForID = FALSE;

  MSYS_CurrentMX = 0;
  MSYS_CurrentMY = 0;
  MSYS_CurrentButtons = 0;
  MSYS_Action = MSYS_NO_ACTION;

  MSYS_PrevRegion = NULL;
  MSYS_SystemInitialized = TRUE;
  MSYS_UseMouseHandlerHook = FALSE;

  MSYS_Mouse_Grabbed = FALSE;
  MSYS_GrabRegion = NULL;

  // Setup the system's background region
  MSYS_SystemBaseRegion.IDNumber = MSYS_ID_SYSTEM;
  MSYS_SystemBaseRegion.PriorityLevel = MSYS_PRIORITY_SYSTEM;
  MSYS_SystemBaseRegion.uiFlags = BASE_REGION_FLAGS;
  MSYS_SystemBaseRegion.RegionTopLeftX = -32767;
  MSYS_SystemBaseRegion.RegionTopLeftY = -32767;
  MSYS_SystemBaseRegion.RegionBottomRightX = 32767;
  MSYS_SystemBaseRegion.RegionBottomRightY = 32767;
  MSYS_SystemBaseRegion.MouseXPos = 0;
  MSYS_SystemBaseRegion.MouseYPos = 0;
  MSYS_SystemBaseRegion.RelativeXPos = 0;
  MSYS_SystemBaseRegion.RelativeYPos = 0;
  MSYS_SystemBaseRegion.ButtonState = 0;
  MSYS_SystemBaseRegion.Cursor = 0;
  MSYS_SystemBaseRegion.UserData[0] = 0;
  MSYS_SystemBaseRegion.UserData[1] = 0;
  MSYS_SystemBaseRegion.UserData[2] = 0;
  MSYS_SystemBaseRegion.UserData[3] = 0;
  MSYS_SystemBaseRegion.MovementCallback = MSYS_NO_CALLBACK;
  MSYS_SystemBaseRegion.ButtonCallback = MSYS_NO_CALLBACK;

  MSYS_SystemBaseRegion.FastHelpTimer = 0;
  MSYS_SystemBaseRegion.FastHelpText = 0;
  MSYS_SystemBaseRegion.FastHelpRect = -1;

  MSYS_SystemBaseRegion.next = NULL;
  MSYS_SystemBaseRegion.prev = NULL;

  // Add the base region to the list
  MSYS_AddRegionToList(&MSYS_SystemBaseRegion);

#ifdef _MOUSE_SYSTEM_HOOK_
  MSYS_UseMouseHandlerHook = TRUE;
#endif

  return (1);
}

//======================================================================================================
//	MSYS_Shutdown
//
//	De-inits the "mousesystem" mouse region handling code.
//
void MSYS_Shutdown(void) {
#ifdef MOUSESYSTEM_DEBUGGING
  gfIgnoreShutdownAssertions = TRUE;
#endif
  MSYS_SystemInitialized = FALSE;
  MSYS_UseMouseHandlerHook = FALSE;
  MSYS_TrashRegList();
  UnRegisterDebugTopic(TOPIC_MOUSE_SYSTEM, "Mouse Region System");
}

//======================================================================================================
//	MSYS_SGP_Mouse_Handler_Hook
//
//	Hook to the SGP's mouse handler
//
void MSYS_SGP_Mouse_Handler_Hook(uint16_t Type, uint16_t Xcoord, uint16_t Ycoord,
                                 BOOLEAN LeftButton, BOOLEAN RightButton) {
  // If the mouse system isn't initialized, get out o' here
  if (!MSYS_SystemInitialized) return;

  // If we're not using the handler stuff, ignore this call
  if (!MSYS_UseMouseHandlerHook) return;

  MSYS_Action = MSYS_NO_ACTION;
  switch (Type) {
    case LEFT_BUTTON_DOWN:
    case LEFT_BUTTON_UP:
    case RIGHT_BUTTON_DOWN:
    case RIGHT_BUTTON_UP:
      // MSYS_Action|=MSYS_DO_BUTTONS;
      if (Type == LEFT_BUTTON_DOWN)
        MSYS_Action |= MSYS_DO_LBUTTON_DWN;
      else if (Type == LEFT_BUTTON_UP) {
        MSYS_Action |= MSYS_DO_LBUTTON_UP;
        // Kris:
        // Used only if applicable.  This is used for that special button that is locked with the
        // mouse press -- just like windows.  When you release the button, the previous state
        // of the button is restored if you released the mouse outside of it's boundaries.  If
        // you release inside of the button, the action is selected -- but later in the code.
        // NOTE:  It has to be here, because the mouse can be released anywhere regardless of
        // regions, buttons, etc.
        ReleaseAnchorMode();
      } else if (Type == RIGHT_BUTTON_DOWN)
        MSYS_Action |= MSYS_DO_RBUTTON_DWN;
      else if (Type == RIGHT_BUTTON_UP)
        MSYS_Action |= MSYS_DO_RBUTTON_UP;

      if (LeftButton)
        MSYS_CurrentButtons |= MSYS_LEFT_BUTTON;
      else
        MSYS_CurrentButtons &= (~MSYS_LEFT_BUTTON);

      if (RightButton)
        MSYS_CurrentButtons |= MSYS_RIGHT_BUTTON;
      else
        MSYS_CurrentButtons &= (~MSYS_RIGHT_BUTTON);

      if ((Xcoord != MSYS_CurrentMX) || (Ycoord != MSYS_CurrentMY)) {
        MSYS_Action |= MSYS_DO_MOVE;
        MSYS_CurrentMX = Xcoord;
        MSYS_CurrentMY = Ycoord;
      }

      MSYS_UpdateMouseRegion();
      break;

    // ATE: Checks here for mouse button repeats.....
    // Call mouse region with new reason
    case LEFT_BUTTON_REPEAT:
    case RIGHT_BUTTON_REPEAT:

      if (Type == LEFT_BUTTON_REPEAT)
        MSYS_Action |= MSYS_DO_LBUTTON_REPEAT;
      else if (Type == RIGHT_BUTTON_REPEAT)
        MSYS_Action |= MSYS_DO_RBUTTON_REPEAT;

      if ((Xcoord != MSYS_CurrentMX) || (Ycoord != MSYS_CurrentMY)) {
        MSYS_Action |= MSYS_DO_MOVE;
        MSYS_CurrentMX = Xcoord;
        MSYS_CurrentMY = Ycoord;
      }

      MSYS_UpdateMouseRegion();
      break;

    case MOUSE_POS:
      if ((Xcoord != MSYS_CurrentMX) || (Ycoord != MSYS_CurrentMY) || gfRefreshUpdate) {
        MSYS_Action |= MSYS_DO_MOVE;
        MSYS_CurrentMX = Xcoord;
        MSYS_CurrentMY = Ycoord;

        gfRefreshUpdate = FALSE;

        MSYS_UpdateMouseRegion();
      }
      break;

    default:
      DbgMessage(TOPIC_MOUSE_SYSTEM, DBG_LEVEL_0, "ERROR -- MSYS 2 SGP Mouse Hook got bad type");
      break;
  }
}

//======================================================================================================
//	MSYS_GetNewID
//
//	Returns a unique ID number for region nodes. If no new ID numbers can be found, the MAX
// value 	is returned.
//
int32_t MSYS_GetNewID(void) {
  int32_t retID;
  int32_t Current, found, done;
  struct MOUSE_REGION *node;

  retID = MSYS_CurrentID;
  MSYS_CurrentID++;

  // Crapy scan for an unused ID
  if ((MSYS_CurrentID >= MSYS_ID_MAX) || MSYS_ScanForID) {
    MSYS_ScanForID = TRUE;
    Current = MSYS_ID_BASE;
    done = found = FALSE;
    while (!done) {
      found = FALSE;
      node = MSYS_RegList;
      while (node != NULL && !found) {
        if (node->IDNumber == Current) found = TRUE;
      }

      if (found && Current < MSYS_ID_MAX)  // Current ID is in use, and their are more to scan
        Current++;
      else {
        done = TRUE;                       // Got an ID to use.
        if (found) Current = MSYS_ID_MAX;  // Ooops, ran out of IDs, use MAX value!
      }
    }
    MSYS_CurrentID = Current;
  }

  return (retID);
}

//======================================================================================================
//	MSYS_TrashRegList
//
//	Deletes the entire region list.
//
void MSYS_TrashRegList(void) {
  while (MSYS_RegList) {
    if (MSYS_RegList->uiFlags & MSYS_REGION_EXISTS) {
      MSYS_RemoveRegion(MSYS_RegList);
    } else {
      MSYS_RegList = MSYS_RegList->next;
    }
  }
}

//======================================================================================================
//	MSYS_AddRegionToList
//
//	Add a region struct to the current list. The list is sorted by priority levels. If two
// entries 	have the same priority level, then the latest to enter the list gets the higher
// priority.
//
void MSYS_AddRegionToList(struct MOUSE_REGION *region) {
  struct MOUSE_REGION *curr;
  int32_t done;

  // If region seems to already be in list, delete it so we can
  // re-insert the region.
  if (region->next || region->prev) {  // if it wasn't actually there, then call does nothing!
    MSYS_DeleteRegionFromList(region);
  }

  // Set an ID number!
  region->IDNumber = (uint16_t)MSYS_GetNewID();

  region->next = NULL;
  region->prev = NULL;

  if (!MSYS_RegList) {  // Null list, so add it straight up.
    MSYS_RegList = region;
  } else {
    // Walk down list until we find place to insert (or at end of list)
    curr = MSYS_RegList;
    done = FALSE;
    while ((curr->next != NULL) && !done) {
      if (curr->PriorityLevel <= region->PriorityLevel)
        done = TRUE;
      else
        curr = curr->next;
    }

    if (curr->PriorityLevel > region->PriorityLevel) {
      // Add after curr node
      region->next = curr->next;
      curr->next = region;
      region->prev = curr;
      if (region->next != NULL) region->next->prev = region;
    } else {
      // Add before curr node
      region->next = curr;
      region->prev = curr->prev;

      curr->prev = region;
      if (region->prev != NULL) region->prev->next = region;

      if (MSYS_RegList == curr)  // Make sure if adding at start, to adjust the list pointer
        MSYS_RegList = region;
    }
  }
}

//======================================================================================================
//	MSYS_RegionInList
//
//	Scan region list for presence of a node with the same region ID number
//
int32_t MSYS_RegionInList(struct MOUSE_REGION *region) {
  struct MOUSE_REGION *Current;
  int32_t found;

  found = FALSE;
  Current = MSYS_RegList;
  while (Current && !found) {
    if (Current->IDNumber == region->IDNumber) found = TRUE;
    Current = Current->next;
  }
  return (found);
}

//======================================================================================================
//	MSYS_DeleteRegionFromList
//
//	Removes a region from the current list.
//
void MSYS_DeleteRegionFromList(struct MOUSE_REGION *region) {
  // If no list present, there's nothin' to do.
  if (!MSYS_RegList) return;

  // Check if region in list
  if (!MSYS_RegionInList(region)) return;

  // Remove a node from the list
  if (MSYS_RegList == region) {  // First node on list, adjust main pointer.
    MSYS_RegList = region->next;
    if (MSYS_RegList != NULL) MSYS_RegList->prev = NULL;
    region->next = region->prev = NULL;
  } else {
    if (region->prev) region->prev->next = region->next;
    // If not last node in list, adjust following node's ->prev entry.
    if (region->next) region->next->prev = region->prev;
    region->prev = region->next = NULL;
  }

  // Did we delete a grabbed region?
  if (MSYS_Mouse_Grabbed) {
    if (MSYS_GrabRegion == region) {
      MSYS_Mouse_Grabbed = FALSE;
      MSYS_GrabRegion = NULL;
    }
  }

  // Is only the system background region remaining?
  if (MSYS_RegList == &MSYS_SystemBaseRegion) {
    // Yup, so let's reset the ID values!
    MSYS_CurrentID = MSYS_ID_BASE;
    MSYS_ScanForID = FALSE;
  } else if (MSYS_RegList == NULL) {
    // Ack, we actually emptied the list, so let's reset for re-init possibilities
    MSYS_CurrentID = MSYS_ID_SYSTEM;
    MSYS_ScanForID = FALSE;
  }
}

//======================================================================================================
//	MSYS_UpdateMouseRegion
//
//	Searches the list for the highest priority region and updates it's info. It also dispatches
//	the callback functions
//
void MSYS_UpdateMouseRegion(void) {
  int32_t found;
  uint32_t ButtonReason;
  struct MOUSE_REGION *pTempRegion;
  BOOLEAN fFound = FALSE;
  found = FALSE;

  // Check previous region!
  if (MSYS_Mouse_Grabbed) {
    MSYS_CurrRegion = MSYS_GrabRegion;
    found = TRUE;
  }
  if (!found) MSYS_CurrRegion = MSYS_RegList;

  while (!found && MSYS_CurrRegion) {
    if (MSYS_CurrRegion->uiFlags & (MSYS_REGION_ENABLED | MSYS_ALLOW_DISABLED_FASTHELP) &&
        (MSYS_CurrRegion->RegionTopLeftX <= MSYS_CurrentMX) &&  // Check boundaries
        (MSYS_CurrRegion->RegionTopLeftY <= MSYS_CurrentMY) &&
        (MSYS_CurrRegion->RegionBottomRightX >= MSYS_CurrentMX) &&
        (MSYS_CurrRegion->RegionBottomRightY >= MSYS_CurrentMY)) {
      // We got the right region. We don't need to check for priorities 'cause
      // the whole list is sorted the right way!
      found = TRUE;
    } else
      MSYS_CurrRegion = MSYS_CurrRegion->next;
  }

  if (MSYS_PrevRegion) {
    MSYS_PrevRegion->uiFlags &= (~MSYS_MOUSE_IN_AREA);

    if (MSYS_PrevRegion != MSYS_CurrRegion) {
      // Remove the help text for the previous region if one is currently being displayed.
      if (MSYS_PrevRegion->FastHelpText) {
        // ExecuteMouseHelpEndCallBack( MSYS_PrevRegion );

        if (MSYS_PrevRegion->uiFlags & MSYS_GOT_BACKGROUND)
          FreeBackgroundRectPending(MSYS_PrevRegion->FastHelpRect);
        MSYS_PrevRegion->uiFlags &= (~MSYS_GOT_BACKGROUND);
        MSYS_PrevRegion->uiFlags &= (~MSYS_FASTHELP_RESET);
      }

      MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;

      // Force a callbacks to happen on previous region to indicate that
      // the mouse has left the old region
      if (MSYS_PrevRegion->uiFlags & MSYS_MOVE_CALLBACK &&
          MSYS_PrevRegion->uiFlags & MSYS_REGION_ENABLED)
        (*(MSYS_PrevRegion->MovementCallback))(MSYS_PrevRegion, MSYS_CALLBACK_REASON_LOST_MOUSE);
    }
  }

  // If a region was found in the list, update it's data
  if (found) {
    if (MSYS_CurrRegion != MSYS_PrevRegion) {
      // Kris -- October 27, 1997
      // Implemented gain mouse region
      if (MSYS_CurrRegion->uiFlags & MSYS_MOVE_CALLBACK) {
        if (MSYS_CurrRegion->FastHelpText && !(MSYS_CurrRegion->uiFlags & MSYS_FASTHELP_RESET)) {
          // ExecuteMouseHelpEndCallBack( MSYS_CurrRegion );
          MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;
          if (MSYS_CurrRegion->uiFlags & MSYS_GOT_BACKGROUND)
            FreeBackgroundRectPending(MSYS_CurrRegion->FastHelpRect);
          MSYS_CurrRegion->uiFlags &= (~MSYS_GOT_BACKGROUND);
          MSYS_CurrRegion->uiFlags |= MSYS_FASTHELP_RESET;

          // if( b->uiFlags & BUTTON_ENABLED )
          //	b->uiFlags |= BUTTON_DIRTY;
        }
        if (MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED) {
          (*(MSYS_CurrRegion->MovementCallback))(MSYS_CurrRegion, MSYS_CALLBACK_REASON_GAIN_MOUSE);
        }
      }

      // if the cursor is set and is not set to no cursor
      if (MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED &&
          MSYS_CurrRegion->uiFlags & MSYS_SET_CURSOR && MSYS_CurrRegion->Cursor != MSYS_NO_CURSOR) {
        MSYS_SetCurrentCursor(MSYS_CurrRegion->Cursor);
      } else {
        // Addition Oct 10/1997 Carter, patch for mouse cursor
        // start at region and find another region encompassing
        pTempRegion = MSYS_CurrRegion->next;
        while ((pTempRegion != NULL) && (!fFound)) {
          if ((pTempRegion->uiFlags & MSYS_REGION_ENABLED) &&
              (pTempRegion->RegionTopLeftX <= MSYS_CurrentMX) &&
              (pTempRegion->RegionTopLeftY <= MSYS_CurrentMY) &&
              (pTempRegion->RegionBottomRightX >= MSYS_CurrentMX) &&
              (pTempRegion->RegionBottomRightY >= MSYS_CurrentMY) &&
              (pTempRegion->uiFlags & MSYS_SET_CURSOR)) {
            fFound = TRUE;
            if (pTempRegion->Cursor != MSYS_NO_CURSOR) {
              MSYS_SetCurrentCursor(pTempRegion->Cursor);
            }
          }
          pTempRegion = pTempRegion->next;
        }
      }
    }

    // OK, if we do not have a button down, any button is game!
    if (!gfClickedModeOn || (gfClickedModeOn && gusClickedIDNumber == MSYS_CurrRegion->IDNumber)) {
      MSYS_CurrRegion->uiFlags |= MSYS_MOUSE_IN_AREA;

      MSYS_CurrRegion->MouseXPos = MSYS_CurrentMX;
      MSYS_CurrRegion->MouseYPos = MSYS_CurrentMY;
      MSYS_CurrRegion->RelativeXPos = MSYS_CurrentMX - MSYS_CurrRegion->RegionTopLeftX;
      MSYS_CurrRegion->RelativeYPos = MSYS_CurrentMY - MSYS_CurrRegion->RegionTopLeftY;

      MSYS_CurrRegion->ButtonState = MSYS_CurrentButtons;

      if (MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED &&
          MSYS_CurrRegion->uiFlags & MSYS_MOVE_CALLBACK && MSYS_Action & MSYS_DO_MOVE) {
        (*(MSYS_CurrRegion->MovementCallback))(MSYS_CurrRegion, MSYS_CALLBACK_REASON_MOVE);
      }

      // ExecuteMouseHelpEndCallBack( MSYS_CurrRegion );
      // MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;

      MSYS_Action &= (~MSYS_DO_MOVE);

      if ((MSYS_CurrRegion->uiFlags & MSYS_BUTTON_CALLBACK) && (MSYS_Action & MSYS_DO_BUTTONS)) {
        if (MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED) {
          ButtonReason = MSYS_CALLBACK_REASON_NONE;
          if (MSYS_Action & MSYS_DO_LBUTTON_DWN) {
            ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_DWN;
            gfClickedModeOn = TRUE;
            // Set global ID
            gusClickedIDNumber = MSYS_CurrRegion->IDNumber;
          }

          if (MSYS_Action & MSYS_DO_LBUTTON_UP) {
            ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_UP;
            gfClickedModeOn = FALSE;
          }

          if (MSYS_Action & MSYS_DO_RBUTTON_DWN) {
            ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_DWN;
            gfClickedModeOn = TRUE;
            // Set global ID
            gusClickedIDNumber = MSYS_CurrRegion->IDNumber;
          }

          if (MSYS_Action & MSYS_DO_RBUTTON_UP) {
            ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_UP;
            gfClickedModeOn = FALSE;
          }

          // ATE: Added repeat resons....
          if (MSYS_Action & MSYS_DO_LBUTTON_REPEAT) {
            ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_REPEAT;
          }

          if (MSYS_Action & MSYS_DO_RBUTTON_REPEAT) {
            ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_REPEAT;
          }

          if (ButtonReason != MSYS_CALLBACK_REASON_NONE) {
            if (MSYS_CurrRegion->uiFlags & MSYS_FASTHELP) {
              // Button was clicked so remove any FastHelp text
              MSYS_CurrRegion->uiFlags &= (~MSYS_FASTHELP);
              if (MSYS_CurrRegion->uiFlags & MSYS_GOT_BACKGROUND)
                FreeBackgroundRectPending(MSYS_CurrRegion->FastHelpRect);
              MSYS_CurrRegion->uiFlags &= (~MSYS_GOT_BACKGROUND);

              // ExecuteMouseHelpEndCallBack( MSYS_CurrRegion );
              MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;
              MSYS_CurrRegion->uiFlags &= (~MSYS_FASTHELP_RESET);

              // if( b->uiFlags & BUTTON_ENABLED )
              //	b->uiFlags |= BUTTON_DIRTY;
            }

            // Kris: Nov 31, 1999 -- Added support for double click events.
            // This is where double clicks are checked and passed down.
            if (ButtonReason == MSYS_CALLBACK_REASON_LBUTTON_DWN) {
              uint32_t uiCurrTime = GetClock();
              if (gpRegionLastLButtonDown == MSYS_CurrRegion &&
                  gpRegionLastLButtonUp == MSYS_CurrRegion &&
                  uiCurrTime <=
                      guiRegionLastLButtonDownTime +
                          MSYS_DOUBLECLICK_DELAY) {  // Sequential left click on same button within
                                                     // the maximum time allowed for a double click
                // Double click check succeeded, set flag and reset double click globals.
                ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_DOUBLECLICK;
                gpRegionLastLButtonDown = NULL;
                gpRegionLastLButtonUp = NULL;
                guiRegionLastLButtonDownTime = 0;
              } else {  // First click, record time and region pointer (to check if 2nd click
                        // detected later)
                gpRegionLastLButtonDown = MSYS_CurrRegion;
                guiRegionLastLButtonDownTime = GetClock();
              }
            } else if (ButtonReason == MSYS_CALLBACK_REASON_LBUTTON_UP) {
              uint32_t uiCurrTime = GetClock();
              if (gpRegionLastLButtonDown == MSYS_CurrRegion &&
                  uiCurrTime <=
                      guiRegionLastLButtonDownTime +
                          MSYS_DOUBLECLICK_DELAY) {  // Double click is Left down, then left up,
                                                     // then left down.  We have just detected the
                                                     // left up here (step 2).
                gpRegionLastLButtonUp = MSYS_CurrRegion;
              } else {  // User released mouse outside of current button, so kill any chance of a
                        // double click happening.
                gpRegionLastLButtonDown = NULL;
                gpRegionLastLButtonUp = NULL;
                guiRegionLastLButtonDownTime = 0;
              }
            }

            (*(MSYS_CurrRegion->ButtonCallback))(MSYS_CurrRegion, ButtonReason);
          }
        }
      }

      MSYS_Action &= (~MSYS_DO_BUTTONS);
    } else if (MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED) {
      // OK here, if we have release a button, UNSET LOCK wherever you are....
      // Just don't give this button the message....
      if (MSYS_Action & MSYS_DO_RBUTTON_UP) {
        gfClickedModeOn = FALSE;
      }
      if (MSYS_Action & MSYS_DO_LBUTTON_UP) {
        gfClickedModeOn = FALSE;
      }

      // OK, you still want move messages however....
      MSYS_CurrRegion->uiFlags |= MSYS_MOUSE_IN_AREA;
      MSYS_CurrRegion->MouseXPos = MSYS_CurrentMX;
      MSYS_CurrRegion->MouseYPos = MSYS_CurrentMY;
      MSYS_CurrRegion->RelativeXPos = MSYS_CurrentMX - MSYS_CurrRegion->RegionTopLeftX;
      MSYS_CurrRegion->RelativeYPos = MSYS_CurrentMY - MSYS_CurrRegion->RegionTopLeftY;

      if ((MSYS_CurrRegion->uiFlags & MSYS_MOVE_CALLBACK) && (MSYS_Action & MSYS_DO_MOVE)) {
        (*(MSYS_CurrRegion->MovementCallback))(MSYS_CurrRegion, MSYS_CALLBACK_REASON_MOVE);
      }

      MSYS_Action &= (~MSYS_DO_MOVE);
    }
    MSYS_PrevRegion = MSYS_CurrRegion;
  } else
    MSYS_PrevRegion = NULL;
}

//=================================================================================================
//	MSYS_DefineRegion
//
//	Inits a struct MOUSE_REGION structure for use with the mouse system
//
void MSYS_DefineRegion(struct MOUSE_REGION *region, uint16_t tlx, uint16_t tly, uint16_t brx,
                       uint16_t bry, int8_t priority, uint16_t crsr, MOUSE_CALLBACK movecallback,
                       MOUSE_CALLBACK buttoncallback) {
#ifdef MOUSESYSTEM_DEBUGGING
  if (region->uiFlags & MSYS_REGION_EXISTS)
    AssertMsg(0, "Attempting to define a region that already exists.");
#endif

  region->IDNumber = MSYS_ID_BASE;

  if (priority == MSYS_PRIORITY_AUTO)
    priority = MSYS_PRIORITY_BASE;
  else if (priority <= MSYS_PRIORITY_LOWEST)
    priority = MSYS_PRIORITY_LOWEST;
  else if (priority >= MSYS_PRIORITY_HIGHEST)
    priority = MSYS_PRIORITY_HIGHEST;

  region->PriorityLevel = priority;

  region->uiFlags = MSYS_NO_FLAGS;

  region->MovementCallback = movecallback;
  if (movecallback != MSYS_NO_CALLBACK) region->uiFlags |= MSYS_MOVE_CALLBACK;

  region->ButtonCallback = buttoncallback;
  if (buttoncallback != MSYS_NO_CALLBACK) region->uiFlags |= MSYS_BUTTON_CALLBACK;

  region->Cursor = crsr;
  if (crsr != MSYS_NO_CURSOR) region->uiFlags |= MSYS_SET_CURSOR;

  region->RegionTopLeftX = tlx;
  region->RegionTopLeftY = tly;
  region->RegionBottomRightX = brx;
  region->RegionBottomRightY = bry;

  region->MouseXPos = 0;
  region->MouseYPos = 0;
  region->RelativeXPos = 0;
  region->RelativeYPos = 0;
  region->ButtonState = 0;

  // Init fasthelp
  region->FastHelpText = NULL;
  region->FastHelpTimer = 0;

  region->next = NULL;
  region->prev = NULL;
  region->HelpDoneCallback = NULL;

  // Add region to system list
  MSYS_AddRegionToList(region);
  region->uiFlags |= MSYS_REGION_ENABLED | MSYS_REGION_EXISTS;

  // Dirty our update flag
  gfRefreshUpdate = TRUE;
}

//=================================================================================================
//	MSYS_ChangeRegionCursor
//
void MSYS_ChangeRegionCursor(struct MOUSE_REGION *region, uint16_t crsr) {
  region->uiFlags &= (~MSYS_SET_CURSOR);
  region->Cursor = crsr;
  if (crsr != MSYS_NO_CURSOR) {
    region->uiFlags |= MSYS_SET_CURSOR;

    // If we are not in the region, donot update!
    if (!(region->uiFlags & MSYS_MOUSE_IN_AREA)) {
      return;
    }

    // Update cursor
    MSYS_SetCurrentCursor(crsr);
  }
}

//=================================================================================================
//	MSYS_AddRegion
//
//	Adds a defined mouse region to the system list. Once inserted, it enables the region then
//	calls the callback functions, if any, for initialization.
//
int32_t MSYS_AddRegion(struct MOUSE_REGION *region) { return (1); }

//=================================================================================================
//	MSYS_RemoveRegion
//
//	Removes a region from the list, disables it, then calls the callback functions for
//	de-initialization.
//
void MSYS_RemoveRegion(struct MOUSE_REGION *region) {
  if (!region) {
#ifdef MOUSESYSTEM_DEBUGGING
    if (gfIgnoreShutdownAssertions)
#endif
      return;
    AssertMsg(0, "Attempting to remove a NULL region.");
  }
#ifdef MOUSESYSTEM_DEBUGGING
  if (!(region->uiFlags & MSYS_REGION_EXISTS))
    AssertMsg(0, "Attempting to remove an already removed region.");
#endif

  if (region->uiFlags & MSYS_HAS_BACKRECT) {
    FreeBackgroundRectPending(region->FastHelpRect);
    region->uiFlags &= (~MSYS_HAS_BACKRECT);
  }

  // Get rid of the FastHelp text (if applicable)
  if (region->FastHelpText) {
    MemFree(region->FastHelpText);
  }
  region->FastHelpText = NULL;

  MSYS_DeleteRegionFromList(region);

  // if the previous region is the one that we are deleting, reset the previous region
  if (MSYS_PrevRegion == region) MSYS_PrevRegion = NULL;
  // if the current region is the one that we are deleting, then clear it.
  if (MSYS_CurrRegion == region) MSYS_CurrRegion = NULL;

  // dirty our update flag
  gfRefreshUpdate = TRUE;

  // Check if this is a locked region, and unlock if so
  if (gfClickedModeOn) {
    // Set global ID
    if (gusClickedIDNumber == region->IDNumber) {
      gfClickedModeOn = FALSE;
    }
  }

  // clear all internal values (including the region exists flag)
  memset(region, 0, sizeof(struct MOUSE_REGION));
}

//=================================================================================================
//	MSYS_EnableRegion
//
//	Enables a mouse region.
//
void MSYS_EnableRegion(struct MOUSE_REGION *region) { region->uiFlags |= MSYS_REGION_ENABLED; }

//=================================================================================================
//	MSYS_DisableRegion
//
//	Disables a mouse region without removing it from the system list.
//
void MSYS_DisableRegion(struct MOUSE_REGION *region) { region->uiFlags &= (~MSYS_REGION_ENABLED); }

//=================================================================================================
//	MSYS_SetCurrentCursor
//
//	Sets the mouse cursor to the regions defined value.
//
void MSYS_SetCurrentCursor(uint16_t Cursor) { SetCurrentCursorFromDatabase(Cursor); }

//=================================================================================================
//	MSYS_ChangeRegionPriority
//
//	Set the priority of a mouse region
//
void MSYS_ChangeRegionPriority(struct MOUSE_REGION *region, int8_t priority) {
  if (priority == MSYS_PRIORITY_AUTO) priority = MSYS_PRIORITY_NORMAL;

  region->PriorityLevel = priority;
}

//=================================================================================================
//	MSYS_SetRegionUserData
//
//	Sets one of the four user data entries in a mouse region
//
void MSYS_SetRegionUserData(struct MOUSE_REGION *region, int32_t index, int32_t userdata) {
  if (index < 0 || index > 3) {
    char str[80];
#ifdef MOUSESYSTEM_DEBUGGING
    if (gfIgnoreShutdownAssertions)
#endif
      return;
    sprintf(str, "Attempting MSYS_SetRegionUserData() with out of range index %d.", index);
    AssertMsg(0, str);
  }
  region->UserData[index] = userdata;
}

//=================================================================================================
//	MSYS_GetRegionUserData
//
//	Retrieves one of the four user data entries in a mouse region
//
int32_t MSYS_GetRegionUserData(struct MOUSE_REGION *region, int32_t index) {
  if (index < 0 || index > 3) {
    char str[80];
#ifdef MOUSESYSTEM_DEBUGGING
    if (gfIgnoreShutdownAssertions)
#endif
      return 0;
    sprintf(str, "Attempting MSYS_GetRegionUserData() with out of range index %d", index);
    AssertMsg(0, str);
  }
  return (region->UserData[index]);
}

//=================================================================================================
//	MSYS_GrabMouse
//
//	Assigns all mouse activity to a region, effectively blocking any other region from having
//	control.
//
int32_t MSYS_GrabMouse(struct MOUSE_REGION *region) {
  if (!MSYS_RegionInList(region)) return (MSYS_REGION_NOT_IN_LIST);

  if (MSYS_Mouse_Grabbed == TRUE) return (MSYS_ALREADY_GRABBED);

  MSYS_Mouse_Grabbed = TRUE;
  MSYS_GrabRegion = region;
  return (MSYS_GRABBED_OK);
}

//=================================================================================================
//	MSYS_ReleaseMouse
//
//	Releases a previously grabbed mouse region
//
void MSYS_ReleaseMouse(struct MOUSE_REGION *region) {
  if (MSYS_GrabRegion != region) return;

  if (MSYS_Mouse_Grabbed == TRUE) {
    MSYS_Mouse_Grabbed = FALSE;
    MSYS_GrabRegion = NULL;
    MSYS_UpdateMouseRegion();
  }
}

/* ==================================================================================
   MSYS_MoveMouseRegionTo( struct MOUSE_REGION *region, int16_t sX, int16_t sY)

         Moves a Mouse region to X Y on the screen

*/

void MSYS_MoveMouseRegionTo(struct MOUSE_REGION *region, int16_t sX, int16_t sY) {
  int16_t sWidth;
  int16_t sHeight;

  sWidth = region->RegionBottomRightX - region->RegionTopLeftX;
  sHeight = region->RegionBottomRightY - region->RegionTopLeftY;

  // move top left
  region->RegionTopLeftX = sX;
  region->RegionTopLeftY = sY;

  // now move bottom right based on topleft + width or height
  region->RegionBottomRightX = sX + sWidth;
  region->RegionBottomRightY = sY + sHeight;

  return;
}

/* ==================================================================================
   MSYS_MoveMouseRegionBy( struct MOUSE_REGION *region, int16_t sDeltaX, int16_t sDeltaY)

         Moves a Mouse region by sDeltaX sDeltaY on the screen

*/

void MSYS_MoveMouseRegionBy(struct MOUSE_REGION *region, int16_t sDeltaX, int16_t sDeltaY) {
  // move top left
  region->RegionTopLeftX = region->RegionTopLeftX + sDeltaX;
  region->RegionTopLeftY = region->RegionTopLeftY + sDeltaY;

  // now move bottom right
  region->RegionBottomRightX = region->RegionBottomRightX + sDeltaX;
  region->RegionBottomRightY = region->RegionBottomRightY + sDeltaY;

  return;
}

// This function will force a re-evaluation of mouse regions
// Usually used to force change of mouse cursor if panels switch, etc
void RefreshMouseRegions() {
  MSYS_Action |= MSYS_DO_MOVE;

  MSYS_UpdateMouseRegion();
}

void SetRegionFastHelpText(struct MOUSE_REGION *region, wchar_t *szText) {
  Assert(region);

  if (region->FastHelpText) MemFree(region->FastHelpText);

  region->FastHelpText = NULL;
  //	region->FastHelpTimer = 0;
  if (!(region->uiFlags & MSYS_REGION_EXISTS)) {
    return;
    // AssertMsg( 0, String( "Attempting to set fast help text, \"%S\" to an inactive region.",
    // szText ) );
  }

  if (!szText || !wcslen(szText)) return;  // blank (or clear)

  // Allocate memory for the button's FastHelp text string...
  region->FastHelpText = (wchar_t *)MemAlloc((wcslen(szText) + 1) * sizeof(wchar_t));
  Assert(region->FastHelpText);

  wcscpy(region->FastHelpText, szText);

  // ATE: We could be replacing already existing, active text
  // so let's remove the region so it be rebuilt...

  if (!IsMapScreen_2()) {
    if (region->uiFlags & MSYS_GOT_BACKGROUND) FreeBackgroundRectPending(region->FastHelpRect);
    region->uiFlags &= (~MSYS_GOT_BACKGROUND);
    region->uiFlags &= (~MSYS_FASTHELP_RESET);
  }

  // region->FastHelpTimer = gsFastHelpDelay;
}

int16_t GetNumberOfLinesInHeight(wchar_t *pStringA) {
  wchar_t *pToken;
  int16_t sCounter = 0;
  wchar_t pString[512];

  wcscpy(pString, pStringA);

  // tokenize
  wchar_t *buffer;
  pToken = wcstok(pString, L"\n", &buffer);

  while (pToken != NULL) {
    pToken = wcstok(NULL, L"\n", &buffer);
    sCounter++;
  }

  return (sCounter);
}

//=============================================================================
//	DisplayFastHelp
//
//
void DisplayFastHelp(struct MOUSE_REGION *region) {
  int32_t iX, iY, iW, iH;

  if (region->uiFlags & MSYS_FASTHELP) {
    iW = (int32_t)GetWidthOfString(region->FastHelpText) + 10;
    iH = (int32_t)(GetNumberOfLinesInHeight(region->FastHelpText) *
                       (GetFontHeight(FONT10ARIAL) + 1) +
                   8);

    iX = (int32_t)region->RegionTopLeftX + 10;

    if (iX < 0) iX = 0;

    if ((iX + iW) >= SCREEN_WIDTH) iX = (SCREEN_WIDTH - iW - 4);

    iY = (int32_t)region->RegionTopLeftY - (iH * 3 / 4);
    if (iY < 0) iY = 0;

    if ((iY + iH) >= SCREEN_HEIGHT) iY = (SCREEN_HEIGHT - iH - 15);

    if (!(region->uiFlags & MSYS_GOT_BACKGROUND)) {
      region->FastHelpRect =
          RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, NULL, (int16_t)iX,
                                 (int16_t)iY, (int16_t)(iX + iW), (int16_t)(iY + iH));
      region->uiFlags |= MSYS_GOT_BACKGROUND;
      region->uiFlags |= MSYS_HAS_BACKRECT;
    } else {
      uint8_t *pDestBuf;
      uint32_t uiDestPitchBYTES;
      pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
      SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
      RectangleDraw(TRUE, iX + 1, iY + 1, iX + iW - 1, iY + iH - 1,
                    rgb32_to_rgb16(FROMRGB(65, 57, 15)), pDestBuf);
      RectangleDraw(TRUE, iX, iY, iX + iW - 2, iY + iH - 2, rgb32_to_rgb16(FROMRGB(227, 198, 88)),
                    pDestBuf);
      JSurface_Unlock(vsFB);
      ShadowVideoSurfaceRect(vsFB, iX + 2, iY + 2, iX + iW - 3, iY + iH - 3);
      ShadowVideoSurfaceRect(vsFB, iX + 2, iY + 2, iX + iW - 3, iY + iH - 3);

      SetFont(FONT10ARIAL);
      SetFontShadow(FONT_NEARBLACK);
      DisplayHelpTokenizedString(region->FastHelpText, (int16_t)(iX + 5), (int16_t)(iY + 5));
      InvalidateRegion(iX, iY, (iX + iW), (iY + iH));
    }
  }
}

int16_t GetWidthOfString(wchar_t *pStringA) {
  wchar_t pString[512];
  wchar_t *pToken;
  int16_t sWidth = 0;
  wcscpy(pString, pStringA);

  // tokenize
  wchar_t *buffer;
  pToken = wcstok(pString, L"\n", &buffer);

  while (pToken != NULL) {
    if (sWidth < StringPixLength(pToken, FONT10ARIAL)) {
      sWidth = StringPixLength(pToken, FONT10ARIAL);
    }

    pToken = wcstok(NULL, L"\n", &buffer);
  }

  return (sWidth);
}

void DisplayHelpTokenizedString(wchar_t *pStringA, int16_t sX, int16_t sY) {
  wchar_t *pToken;
  int32_t iCounter = 0, i;
  uint32_t uiCursorXPos;
  wchar_t pString[512];
  int32_t iLength;

  wcscpy(pString, pStringA);

  // tokenize
  wchar_t *buffer;
  pToken = wcstok(pString, L"\n", &buffer);

  while (pToken != NULL) {
    iLength = (int32_t)wcslen(pToken);
    for (i = 0; i < iLength; i++) {
      uiCursorXPos = StringPixLengthArgFastHelp(FONT10ARIAL, FONT10ARIALBOLD, i, pToken);
      if (pToken[i] == '|') {
        i++;
        SetFont(FONT10ARIALBOLD);
        SetFontForeground(146);
      } else {
        SetFont(FONT10ARIAL);
        SetFontForeground(FONT_BEIGE);
      }
      mprintf(sX + uiCursorXPos, sY + iCounter * (GetFontHeight(FONT10ARIAL) + 1), L"%c",
              pToken[i]);
    }
    pToken = wcstok(NULL, L"\n", &buffer);
    iCounter++;
  }
}

void RenderFastHelp() {
  static int32_t iLastClock;
  int32_t iTimeDifferential, iCurrentClock;

  if (!gfRenderHilights) return;

  iCurrentClock = GetClock();
  iTimeDifferential = iCurrentClock - iLastClock;
  if (iTimeDifferential < 0) iTimeDifferential += 0x7fffffff;
  iLastClock = iCurrentClock;

  if (MSYS_CurrRegion && MSYS_CurrRegion->FastHelpText) {
    if (!MSYS_CurrRegion->FastHelpTimer) {
      if (MSYS_CurrRegion->uiFlags & (MSYS_ALLOW_DISABLED_FASTHELP | MSYS_REGION_ENABLED)) {
        if (MSYS_CurrRegion->uiFlags & MSYS_MOUSE_IN_AREA)
          MSYS_CurrRegion->uiFlags |= MSYS_FASTHELP;
        else {
          MSYS_CurrRegion->uiFlags &= (~(MSYS_FASTHELP | MSYS_FASTHELP_RESET));
        }
        // Do I really need this?
        // MSYS_CurrRegion->uiFlags |= REGION_DIRTY;
        DisplayFastHelp(MSYS_CurrRegion);
      }
    } else {
      if (MSYS_CurrRegion->uiFlags & (MSYS_ALLOW_DISABLED_FASTHELP | MSYS_REGION_ENABLED)) {
        if (MSYS_CurrRegion->uiFlags & MSYS_MOUSE_IN_AREA &&
            !MSYS_CurrRegion->ButtonState)  // & (MSYS_LEFT_BUTTON|MSYS_RIGHT_BUTTON)) )
        {
          MSYS_CurrRegion->FastHelpTimer -= (int16_t)max(iTimeDifferential, 0);

          if (MSYS_CurrRegion->FastHelpTimer < 0) {
            MSYS_CurrRegion->FastHelpTimer = 0;
          }
        }
      }
    }
  }
}

BOOLEAN SetRegionSavedRect(struct MOUSE_REGION *region) { return FALSE; }

void FreeRegionSavedRect(struct MOUSE_REGION *region) {}

void MSYS_AllowDisabledRegionFastHelp(struct MOUSE_REGION *region, BOOLEAN fAllow) {
  if (fAllow) {
    region->uiFlags |= MSYS_ALLOW_DISABLED_FASTHELP;
  } else {
    region->uiFlags &= ~MSYS_ALLOW_DISABLED_FASTHELP;
  }
}

// new stuff to allow mouse callbacks when help text finishes displaying

void SetRegionHelpEndCallback(struct MOUSE_REGION *region,
                              MOUSE_HELPTEXT_DONE_CALLBACK CallbackFxn) {
  // make sure region is non null
  if (region == NULL) {
    return;
  }

  // now set the region help text
  region->HelpDoneCallback = CallbackFxn;

  return;
}

void ExecuteMouseHelpEndCallBack(struct MOUSE_REGION *region) {
  if (region == NULL) {
    return;
  }

  if (region->FastHelpTimer) {
    return;
  }
  // check if callback is non null
  if (region->HelpDoneCallback == NULL) {
    return;
  }

  // we have a callback, excecute
  // ATE: Disable these!
  //( *( region->HelpDoneCallback ) )( );

  return;
}

void SetFastHelpDelay(int16_t sFastHelpDelay) { gsFastHelpDelay = sFastHelpDelay; }

void EnableMouseFastHelp(void) { gfShowFastHelp = TRUE; }

void DisableMouseFastHelp(void) { gfShowFastHelp = FALSE; }

void ResetClickedMode(void) { gfClickedModeOn = FALSE; }
