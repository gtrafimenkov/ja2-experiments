// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

/***********************************************************************************************
        Button System.c

        Rewritten mostly by Kris Morness
***********************************************************************************************/

#include "SGP/ButtonSystem.h"

#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Globals.h"
#include "SGP/ButtonSoundControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Line.h"
#include "SGP/MemMan.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "TileEngine/RenderDirty.h"
#include "Utils/FontControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// ATE: Added to let Wiz default creating mouse regions with no cursor, JA2 default to a cursor (
// first one )
#define MSYS_STARTING_CURSORVAL 0
#define COLOR_DKGREY 136

#define MAX_GENERIC_PICS 40
#define MAX_BUTTON_ICONS 40

#define GUI_BTN_NONE 0
#define GUI_BTN_DUPLICATE_VOBJ 1
#define GUI_BTN_EXTERNAL_VOBJ 2

char str[128];

// Kris:  December 2, 1997
// Special internal debugging utilities that will ensure that you don't attempt to delete
// an already deleted button, or it's images, etc.  It will also ensure that you don't create
// the same button that already exists.
// TO REMOVE ALL DEBUG FUNCTIONALITY:  simply comment out BUTTONSYSTEM_DEBUGGING definition
#ifdef _DEBUG
#define BUTTONSYSTEM_DEBUGGING
#endif

#ifdef BUTTONSYSTEM_DEBUGGING
BOOLEAN gfIgnoreShutdownAssertions;
// Called immediately before assigning the button to the button list.
void AssertFailIfIdenticalButtonAttributesFound(GUI_BUTTON *b) {
  int32_t x;
  GUI_BUTTON *c;
  for (x = 0; x < MAX_BUTTONS; x++) {
    c = ButtonList[x];
    if (!c) continue;
    if (c->uiFlags & BUTTON_DELETION_PENDING) continue;
    if (c->UserData[3] == 0xffffffff) continue;
    if (b->Area.PriorityLevel != c->Area.PriorityLevel) continue;
    if (b->Area.RegionTopLeftX != c->Area.RegionTopLeftX) continue;
    if (b->Area.RegionTopLeftY != c->Area.RegionTopLeftY) continue;
    if (b->Area.RegionBottomRightX != c->Area.RegionBottomRightX) continue;
    if (b->Area.RegionBottomRightY != c->Area.RegionBottomRightY) continue;
    if (b->ClickCallback != c->ClickCallback) continue;
    if (b->MoveCallback != c->MoveCallback) continue;
    if (b->XLoc != c->XLoc) continue;
    if (b->YLoc != c->YLoc) continue;
    // if we get this far, it is reasonably safe to assume that the newly created
    // button already exists.  Placing a break point on the following assert will
    // allow the coder to easily isolate the case!
    sprintf(str,
            "Attempting to create a button that has already been created (existing buttonID %d).",
            c->IDNum);
    AssertMsg(0, str);
  }
}
#endif

// Kris:
// These are the variables used for the anchoring of a particular button.
// When you click on a button, it get's anchored, until you release the mouse button.
// When you move around, you don't want to select other buttons, even when you release
// it.  This follows the Windows 95 convention.
GUI_BUTTON *gpAnchoredButton;
GUI_BUTTON *gpPrevAnchoredButton;
BOOLEAN gfAnchoredState;
void ReleaseAnchorMode();

int8_t gbDisabledButtonStyle;
void DrawHatchOnButton(GUI_BUTTON *b);
void DrawShadeOnButton(GUI_BUTTON *b);
void DrawDefaultOnButton(GUI_BUTTON *b);

GUI_BUTTON *gpCurrentFastHelpButton;

BOOLEAN gfRenderHilights = TRUE;

BUTTON_PICS ButtonPictures[MAX_BUTTON_PICS];
int32_t ButtonPicsLoaded;

struct JSurface *vsButtonDest;
uint32_t ButtonDestPitch = 640 * 2;
uint32_t ButtonDestBPP = 16;

GUI_BUTTON *ButtonList[MAX_BUTTONS];

int32_t ButtonsInList = 0;

uint16_t GetWidthOfButtonPic(uint16_t usButtonPicID, int32_t iSlot) {
  return ButtonPictures[usButtonPicID].vobj->pETRLEObject[iSlot].usWidth;
}

struct VObject *GenericButtonGrayed[MAX_GENERIC_PICS];
struct VObject *GenericButtonOffNormal[MAX_GENERIC_PICS];
struct VObject *GenericButtonOffHilite[MAX_GENERIC_PICS];
struct VObject *GenericButtonOnNormal[MAX_GENERIC_PICS];
struct VObject *GenericButtonOnHilite[MAX_GENERIC_PICS];
struct VObject *GenericButtonBackground[MAX_GENERIC_PICS];
uint16_t GenericButtonFillColors[MAX_GENERIC_PICS];
uint16_t GenericButtonBackgroundIndex[MAX_GENERIC_PICS];
int16_t GenericButtonOffsetX[MAX_GENERIC_PICS];
int16_t GenericButtonOffsetY[MAX_GENERIC_PICS];

struct VObject *GenericButtonIcons[MAX_BUTTON_ICONS];

// flag to state we wish to render buttons on the one after the next pass through render buttons
BOOLEAN fPausedMarkButtonsDirtyFlag = FALSE;
BOOLEAN fDisableHelpTextRestoreFlag = FALSE;

BOOLEAN gfDelayButtonDeletion = FALSE;
BOOLEAN gfPendingButtonDeletion = FALSE;
void RemoveButtonsMarkedForDeletion();

extern struct MOUSE_REGION *MSYS_PrevRegion;
extern struct MOUSE_REGION *MSYS_CurrRegion;

//=============================================================================
//	FindFreeButtonSlot
//
//	Finds an available slot for loading button pictures
//
int32_t FindFreeButtonSlot(void) {
  int slot;

  // Are there any slots available?
  if (ButtonPicsLoaded >= MAX_BUTTON_PICS) return (BUTTON_NO_SLOT);

  // Search for a slot
  for (slot = 0; slot < MAX_BUTTON_PICS; slot++) {
    if (ButtonPictures[slot].vobj == NULL) return (slot);
  }

  return (BUTTON_NO_SLOT);
}

//=============================================================================
//	LoadButtonImage
//
//	Load images for use with QuickButtons.
//
int32_t LoadButtonImage(char *filename, int32_t Grayed, int32_t OffNormal, int32_t OffHilite,
                        int32_t OnNormal, int32_t OnHilite) {
  uint32_t UseSlot;
  ETRLEObject *pTrav;
  uint32_t MaxHeight, MaxWidth, ThisHeight, ThisWidth;

  AssertMsg(filename != BUTTON_NO_FILENAME, "Attempting to LoadButtonImage() with null filename.");
  AssertMsg(strlen(filename), "Attempting to LoadButtonImage() with empty filename string.");

  // is there ANY file to open?
  if ((Grayed == BUTTON_NO_IMAGE) && (OffNormal == BUTTON_NO_IMAGE) &&
      (OffHilite == BUTTON_NO_IMAGE) && (OnNormal == BUTTON_NO_IMAGE) &&
      (OnHilite == BUTTON_NO_IMAGE)) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("No button pictures selected for %s", filename));
    return (-1);
  }

  // Get a button image slot
  if ((UseSlot = FindFreeButtonSlot()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("Out of button image slots for %s", filename));
    return (-1);
  }

  // Load the image
  if ((ButtonPictures[UseSlot].vobj = CreateVObjectFromFile(filename)) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("Couldn't create VOBJECT for %s", filename));
    return (-1);
  }

  // Init the QuickButton image structure with indexes to use
  ButtonPictures[UseSlot].Grayed = Grayed;
  ButtonPictures[UseSlot].OffNormal = OffNormal;
  ButtonPictures[UseSlot].OffHilite = OffHilite;
  ButtonPictures[UseSlot].OnNormal = OnNormal;
  ButtonPictures[UseSlot].OnHilite = OnHilite;
  ButtonPictures[UseSlot].fFlags = GUI_BTN_NONE;

  // Fit the button size to the largest image in the set
  MaxWidth = MaxHeight = 0;
  if (Grayed != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[Grayed]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OffNormal != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OffNormal]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OffHilite != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OffHilite]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OnNormal != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OnNormal]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OnHilite != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OnHilite]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  // Set the width and height for this image set
  ButtonPictures[UseSlot].MaxHeight = MaxHeight;
  ButtonPictures[UseSlot].MaxWidth = MaxWidth;

  // return the image slot number
  ButtonPicsLoaded++;
  return (UseSlot);
}

//=============================================================================
//	UseLoadedButtonImage
//
//	Uses a previously loaded quick button image for use with QuickButtons.
//	The function simply duplicates the vobj!
//
int32_t UseLoadedButtonImage(int32_t LoadedImg, int32_t Grayed, int32_t OffNormal,
                             int32_t OffHilite, int32_t OnNormal, int32_t OnHilite) {
  uint32_t UseSlot;
  ETRLEObject *pTrav;
  uint32_t MaxHeight, MaxWidth, ThisHeight, ThisWidth;

  // Is button image index given valid?
  if (ButtonPictures[LoadedImg].vobj == NULL) {
    DbgMessage(
        TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
        String("Invalid button picture handle given for pre-loaded button image %d", LoadedImg));
    return (-1);
  }

  // Is button image an external vobject?
  if (ButtonPictures[LoadedImg].fFlags & GUI_BTN_EXTERNAL_VOBJ) {
    DbgMessage(
        TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
        String(
            "Invalid button picture handle given (%d), cannot use external images as duplicates.",
            LoadedImg));
    return (-1);
  }

  // is there ANY file to open?
  if ((Grayed == BUTTON_NO_IMAGE) && (OffNormal == BUTTON_NO_IMAGE) &&
      (OffHilite == BUTTON_NO_IMAGE) && (OnNormal == BUTTON_NO_IMAGE) &&
      (OnHilite == BUTTON_NO_IMAGE)) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("No button pictures selected for pre-loaded button image %d", LoadedImg));
    return (-1);
  }

  // Get a button image slot
  if ((UseSlot = FindFreeButtonSlot()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("Out of button image slots for pre-loaded button image %d", LoadedImg));
    return (-1);
  }

  // Init the QuickButton image structure with indexes to use
  ButtonPictures[UseSlot].vobj = ButtonPictures[LoadedImg].vobj;
  ButtonPictures[UseSlot].Grayed = Grayed;
  ButtonPictures[UseSlot].OffNormal = OffNormal;
  ButtonPictures[UseSlot].OffHilite = OffHilite;
  ButtonPictures[UseSlot].OnNormal = OnNormal;
  ButtonPictures[UseSlot].OnHilite = OnHilite;
  ButtonPictures[UseSlot].fFlags = GUI_BTN_DUPLICATE_VOBJ;

  // Fit the button size to the largest image in the set
  MaxWidth = MaxHeight = 0;
  if (Grayed != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[Grayed]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OffNormal != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OffNormal]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OffHilite != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OffHilite]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OnNormal != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OnNormal]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OnHilite != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OnHilite]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  // Set the width and height for this image set
  ButtonPictures[UseSlot].MaxHeight = MaxHeight;
  ButtonPictures[UseSlot].MaxWidth = MaxWidth;

  // return the image slot number
  ButtonPicsLoaded++;
  return (UseSlot);
}

//=============================================================================
//	UseVObjAsButtonImage
//
//	Uses a previously loaded VObject for use with QuickButtons.
//	The function simply duplicates the vobj pointer and uses that.
//
//		**** NOTE ****
//			The image isn't unloaded with a call to UnloadButtonImage. The internal
//			structures are simply removed from the button image list. It's up to
//			the user to actually unload the image.
//
int32_t UseVObjAsButtonImage(struct VObject *hVObject, int32_t Grayed, int32_t OffNormal,
                             int32_t OffHilite, int32_t OnNormal, int32_t OnHilite) {
  uint32_t UseSlot;
  ETRLEObject *pTrav;
  uint32_t MaxHeight, MaxWidth, ThisHeight, ThisWidth;

  // Is button image index given valid?
  if (hVObject == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("UseVObjAsButtonImage: Invalid VObject image given"));
    return (-1);
  }

  // is there ANY file to open?
  if ((Grayed == BUTTON_NO_IMAGE) && (OffNormal == BUTTON_NO_IMAGE) &&
      (OffHilite == BUTTON_NO_IMAGE) && (OnNormal == BUTTON_NO_IMAGE) &&
      (OnHilite == BUTTON_NO_IMAGE)) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("UseVObjAsButtonImage: No button pictures indexes selected for VObject"));
    return (-1);
  }

  // Get a button image slot
  if ((UseSlot = FindFreeButtonSlot()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("UseVObjAsButtonImage: Out of button image slots for VObject"));
    return (-1);
  }

  // Init the QuickButton image structure with indexes to use
  ButtonPictures[UseSlot].vobj = hVObject;
  ButtonPictures[UseSlot].Grayed = Grayed;
  ButtonPictures[UseSlot].OffNormal = OffNormal;
  ButtonPictures[UseSlot].OffHilite = OffHilite;
  ButtonPictures[UseSlot].OnNormal = OnNormal;
  ButtonPictures[UseSlot].OnHilite = OnHilite;
  ButtonPictures[UseSlot].fFlags = GUI_BTN_EXTERNAL_VOBJ;

  // Fit the button size to the largest image in the set
  MaxWidth = MaxHeight = 0;
  if (Grayed != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[Grayed]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OffNormal != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OffNormal]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OffHilite != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OffHilite]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OnNormal != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OnNormal]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  if (OnHilite != BUTTON_NO_IMAGE) {
    pTrav = &(ButtonPictures[UseSlot].vobj->pETRLEObject[OnHilite]);
    ThisHeight = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    ThisWidth = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    if (MaxWidth < ThisWidth) MaxWidth = ThisWidth;
    if (MaxHeight < ThisHeight) MaxHeight = ThisHeight;
  }

  // Set the width and height for this image set
  ButtonPictures[UseSlot].MaxHeight = MaxHeight;
  ButtonPictures[UseSlot].MaxWidth = MaxWidth;

  // return the image slot number
  ButtonPicsLoaded++;
  return (UseSlot);
}

// Removes a QuickButton image from the system.
void UnloadButtonImage(int32_t Index) {
  int32_t x;
  BOOLEAN fDone;

  if (Index < 0 || Index >= MAX_BUTTON_PICS) {
    sprintf(str, "Attempting to UnloadButtonImage with out of range index %d.", Index);
    AssertMsg(0, str);
  }

  if (!ButtonPictures[Index].vobj) {
#ifdef BUTTONSYSTEM_DEBUGGING
    if (gfIgnoreShutdownAssertions)
#endif
      return;
    AssertMsg(0, "Attempting to UnloadButtonImage that has a null vobj (already deleted).");
  }

  // If this is a duplicated button image, then don't trash the vobject
  if (ButtonPictures[Index].fFlags & GUI_BTN_DUPLICATE_VOBJ ||
      ButtonPictures[Index].fFlags & GUI_BTN_EXTERNAL_VOBJ) {
    ButtonPictures[Index].vobj = NULL;
    ButtonPicsLoaded--;
  } else {
    // Deleting a non-duplicate, so see if any dups present. if so, then
    // convert one of them to an original!

    fDone = FALSE;
    for (x = 0; x < MAX_BUTTON_PICS && !fDone; x++) {
      if ((x != Index) && (ButtonPictures[x].vobj == ButtonPictures[Index].vobj)) {
        if (ButtonPictures[x].fFlags & GUI_BTN_DUPLICATE_VOBJ) {
          // If we got here, then we got a duplicate object of the one we
          // want to delete, so convert it to an original!
          ButtonPictures[x].fFlags &= (~GUI_BTN_DUPLICATE_VOBJ);

          // Now remove this button, but not it's vobject
          ButtonPictures[Index].vobj = NULL;

          fDone = TRUE;
          ButtonPicsLoaded--;
        }
      }
    }
  }

  // If image slot isn't empty, delete the image
  if (ButtonPictures[Index].vobj != NULL) {
    DeleteVideoObject(ButtonPictures[Index].vobj);
    ButtonPictures[Index].vobj = NULL;
    ButtonPicsLoaded--;
  }
}

//=============================================================================
//	EnableButton
//
//	Enables an already created button.
//
BOOLEAN EnableButton(int32_t iButtonID) {
  GUI_BUTTON *b;
  uint32_t OldState;

  if (iButtonID < 0 || iButtonID >= MAX_BUTTONS) {
    sprintf(str, "Attempting to EnableButton with out of range buttonID %d.", iButtonID);
    AssertMsg(0, str);
  }

  b = ButtonList[iButtonID];

  // If button exists, set the ENABLED flag
  if (b) {
    OldState = b->uiFlags & BUTTON_ENABLED;
    b->uiFlags |= (BUTTON_ENABLED | BUTTON_DIRTY);
  } else
    OldState = 0;

  // Return previous ENABLED state of this button
  return ((OldState == BUTTON_ENABLED) ? TRUE : FALSE);
}

//=============================================================================
//	DisableButton
//
//	Disables a button. The button remains in the system list, and can be
//	reactivated by calling EnableButton.
//
//	Diabled buttons will appear "grayed out" on the screen (unless the
//	graphics for such are not available).
//
BOOLEAN DisableButton(int32_t iButtonID) {
  GUI_BUTTON *b;
  uint32_t OldState;

  if (iButtonID < 0 || iButtonID >= MAX_BUTTONS) {
    sprintf(str, "Attempting to DisableButton with out of range buttonID %d.", iButtonID);
    AssertMsg(0, str);
  }

  b = ButtonList[iButtonID];

  // If button exists, reset the ENABLED flag
  if (b) {
    OldState = b->uiFlags & BUTTON_ENABLED;
    b->uiFlags &= (~BUTTON_ENABLED);
    b->uiFlags |= BUTTON_DIRTY;
  } else
    OldState = 0;

  // Return previous ENABLED state of button
  return ((OldState == BUTTON_ENABLED) ? TRUE : FALSE);
}

//=============================================================================
//	InitializeButtonImageManager
//
//	Initializes the button image sub-system. This function is called by
//	InitButtonSystem.
//
BOOLEAN InitializeButtonImageManager() {
  uint8_t Pix;
  int x;

  vsButtonDest = vsFB;
  ButtonDestPitch = 640 * 2;
  ButtonDestBPP = 16;

  // Blank out all QuickButton images
  for (x = 0; x < MAX_BUTTON_PICS; x++) {
    ButtonPictures[x].vobj = NULL;
    ButtonPictures[x].Grayed = -1;
    ButtonPictures[x].OffNormal = -1;
    ButtonPictures[x].OffHilite = -1;
    ButtonPictures[x].OnNormal = -1;
    ButtonPictures[x].OnHilite = -1;
  }
  ButtonPicsLoaded = 0;

  // Blank out all Generic button data
  for (x = 0; x < MAX_GENERIC_PICS; x++) {
    GenericButtonGrayed[x] = NULL;
    GenericButtonOffNormal[x] = NULL;
    GenericButtonOffHilite[x] = NULL;
    GenericButtonOnNormal[x] = NULL;
    GenericButtonOnHilite[x] = NULL;
    GenericButtonBackground[x] = NULL;
    GenericButtonBackgroundIndex[x] = 0;
    GenericButtonFillColors[x] = 0;
    GenericButtonBackgroundIndex[x] = 0;
    GenericButtonOffsetX[x] = 0;
    GenericButtonOffsetY[x] = 0;
  }

  // Blank out all icon images
  for (x = 0; x < MAX_BUTTON_ICONS; x++) GenericButtonIcons[x] = NULL;

  // Load the default generic button images
  if ((GenericButtonOffNormal[0] = CreateVObjectFromFile(DEFAULT_GENERIC_BUTTON_OFF)) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "Couldn't create VOBJECT for " DEFAULT_GENERIC_BUTTON_OFF);
    return (FALSE);
  }

  if ((GenericButtonOnNormal[0] = CreateVObjectFromFile(DEFAULT_GENERIC_BUTTON_ON)) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "Couldn't create VOBJECT for " DEFAULT_GENERIC_BUTTON_ON);
    return (FALSE);
  }

  // Load up the off hilite and on hilite images. We won't check for errors because if the file
  // doesn't exists, the system simply ignores that file. These are only here as extra images, they
  // aren't required for operation (only OFF Normal and ON Normal are required).
  GenericButtonOffHilite[0] = CreateVObjectFromFile(DEFAULT_GENERIC_BUTTON_OFF_HI);

  GenericButtonOnHilite[0] = CreateVObjectFromFile(DEFAULT_GENERIC_BUTTON_ON_HI);

  Pix = 0;
  if (!GetETRLEPixelValue(&Pix, GenericButtonOffNormal[0], 8, 0, 0)) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "Couldn't get generic button's background pixel value");
    return (FALSE);
  }

  GenericButtonFillColors[0] = GenericButtonOffNormal[0]->p16BPPPalette[Pix];

  return (TRUE);
}

//=============================================================================
//	FindFreeGenericSlot
//
//	Finds the next available slot for generic (TEXT and/or ICONIC) buttons.
//
int16_t FindFreeGenericSlot(void) {
  int16_t slot, x;

  slot = BUTTON_NO_SLOT;
  for (x = 0; x < MAX_GENERIC_PICS && slot < 0; x++) {
    if (GenericButtonOffNormal[x] == NULL) slot = x;
  }

  return (slot);
}

//=============================================================================
//	FindFreeIconSlot
//
//	Finds the next available slot for button icon images.
//
int16_t FindFreeIconSlot(void) {
  int16_t slot, x;

  slot = BUTTON_NO_SLOT;
  for (x = 0; x < MAX_BUTTON_ICONS && slot < 0; x++) {
    if (GenericButtonIcons[x] == NULL) slot = x;
  }

  return (slot);
}

//=============================================================================
//	LoadGenericButtonIcon
//
//	Loads an image file for use as a button icon.
//
int16_t LoadGenericButtonIcon(char *filename) {
  int16_t ImgSlot;

  AssertMsg(filename != BUTTON_NO_FILENAME,
            "Attempting to LoadGenericButtonIcon() with null filename.");

  // Get slot for icon image
  if ((ImgSlot = FindFreeIconSlot()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "LoadGenericButtonIcon: Out of generic button icon slots");
    return (-1);
  }

  // Load the icon
  if ((GenericButtonIcons[ImgSlot] = CreateVObjectFromFile(filename)) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("LoadGenericButtonIcon: Couldn't create VOBJECT for %s", filename));
    return (-1);
  }

  // Return the slot number
  return (ImgSlot);
}

//=============================================================================
//	UnloadGenericButtonIcon
//
//	Removes a button icon graphic from the system
//
BOOLEAN UnloadGenericButtonIcon(int16_t GenImg) {
  if (GenImg < 0 || GenImg >= MAX_BUTTON_ICONS) {
    sprintf(str, "Attempting to UnloadGenericButtonIcon with out of range index %d.", GenImg);
    AssertMsg(0, str);
  }

  if (!GenericButtonIcons[GenImg]) {
#ifdef BUTTONSYSTEM_DEBUGGING
    if (gfIgnoreShutdownAssertions)
#endif
      return FALSE;
    AssertMsg(0, "Attempting to UnloadGenericButtonIcon that has no icon (already deleted).");
  }
  // If an icon is present in the slot, remove it.
  DeleteVideoObject(GenericButtonIcons[GenImg]);
  GenericButtonIcons[GenImg] = NULL;
  return TRUE;
}

//=============================================================================
//	UnloadGenericButtonImage
//
//	Removes the images associated with a generic button. Except the icon
//	image of iconic buttons. See above.
//
BOOLEAN UnloadGenericButtonImage(int16_t GenImg) {
  if (GenImg < 0 || GenImg >= MAX_GENERIC_PICS) {
    sprintf(str, "Attempting to UnloadGenericButtonImage with out of range index %d.", GenImg);
    AssertMsg(0, str);
  }

  // For each possible image type in a generic button, check if it's
  // present, and if so, remove it.
  if (GenericButtonGrayed[GenImg] != NULL) {
    DeleteVideoObject(GenericButtonGrayed[GenImg]);
    GenericButtonGrayed[GenImg] = NULL;
  }

  if (GenericButtonOffNormal[GenImg] != NULL) {
    DeleteVideoObject(GenericButtonOffNormal[GenImg]);
    GenericButtonOffNormal[GenImg] = NULL;
  }

  if (GenericButtonOffHilite[GenImg] != NULL) {
    DeleteVideoObject(GenericButtonOffHilite[GenImg]);
    GenericButtonOffHilite[GenImg] = NULL;
  }

  if (GenericButtonOnNormal[GenImg] != NULL) {
    DeleteVideoObject(GenericButtonOnNormal[GenImg]);
    GenericButtonOnNormal[GenImg] = NULL;
  }

  if (GenericButtonOnHilite[GenImg] != NULL) {
    DeleteVideoObject(GenericButtonOnHilite[GenImg]);
    GenericButtonOnHilite[GenImg] = NULL;
  }

  if (GenericButtonBackground[GenImg] != NULL) {
    DeleteVideoObject(GenericButtonBackground[GenImg]);
    GenericButtonBackground[GenImg] = NULL;
  }

  // Reset the remaining variables
  GenericButtonFillColors[GenImg] = 0;
  GenericButtonBackgroundIndex[GenImg] = 0;
  GenericButtonOffsetX[GenImg] = 0;
  GenericButtonOffsetY[GenImg] = 0;

  return (TRUE);
}

//=============================================================================
//	LoadGenericButtonImages
//
//	Loads the image files required for displaying a generic button.
//
int16_t LoadGenericButtonImages(char *GrayName, char *OffNormName, char *OffHiliteName,
                                char *OnNormName, char *OnHiliteName, char *BkGrndName,
                                int16_t Index, int16_t OffsetX, int16_t OffsetY) {
  int16_t ImgSlot;
  uint8_t Pix;

  // if the images for Off-Normal and On-Normal don't exist, abort call
  if ((OffNormName == BUTTON_NO_FILENAME) || (OnNormName == BUTTON_NO_FILENAME)) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "LoadGenericButtonImages: No filenames for OFFNORMAL and/or ONNORMAL images");
    return (-1);
  }

  // Get a slot number for these images
  if ((ImgSlot = FindFreeGenericSlot()) == -1) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "LoadGenericButtonImages: Out of generic button slots");
    return (-1);
  }

  // Load the image for the Off-Normal button state (required)
  if ((GenericButtonOffNormal[ImgSlot] = CreateVObjectFromFile(OffNormName)) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("LoadGenericButtonImages: Couldn't create VOBJECT for %s", OffNormName));
    return (-1);
  }

  // Load the image for the On-Normal button state (required)
  if ((GenericButtonOnNormal[ImgSlot] = CreateVObjectFromFile(OnNormName)) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               String("LoadGenericButtonImages: Couldn't create VOBJECT for %s", OnNormName));
    return (-1);
  }

  // For the optional button state images, see if a filename was given, and
  // if so, load it.

  if (GrayName != BUTTON_NO_FILENAME) {
    if ((GenericButtonGrayed[ImgSlot] = CreateVObjectFromFile(GrayName)) == NULL) {
      DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
                 String("LoadGenericButtonImages: Couldn't create VOBJECT for %s", GrayName));
      return (-1);
    }
  } else
    GenericButtonGrayed[ImgSlot] = NULL;

  if (OffHiliteName != BUTTON_NO_FILENAME) {
    if ((GenericButtonOffHilite[ImgSlot] = CreateVObjectFromFile(OffHiliteName)) == NULL) {
      DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
                 String("LoadGenericButtonImages: Couldn't create VOBJECT for %s", OffHiliteName));
      return (-1);
    }
  } else
    GenericButtonOffHilite[ImgSlot] = NULL;

  if (OnHiliteName != BUTTON_NO_FILENAME) {
    if ((GenericButtonOnHilite[ImgSlot] = CreateVObjectFromFile(OnHiliteName)) == NULL) {
      DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
                 String("LoadGenericButtonImages: Couldn't create VOBJECT for %s", OnHiliteName));
      return (-1);
    }
  } else
    GenericButtonOnHilite[ImgSlot] = NULL;

  if (BkGrndName != BUTTON_NO_FILENAME) {
    if ((GenericButtonBackground[ImgSlot] = CreateVObjectFromFile(BkGrndName)) == NULL) {
      DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
                 String("LoadGenericButtonImages: Couldn't create VOBJECT for %s", BkGrndName));
      return (-1);
    }
  } else
    GenericButtonBackground[ImgSlot] = NULL;

  GenericButtonBackgroundIndex[ImgSlot] = Index;

  // Get the background fill color from the last (9th) sub-image in the
  // Off-Normal image.
  Pix = 0;
  if (!GetETRLEPixelValue(&Pix, GenericButtonOffNormal[ImgSlot], 8, 0, 0)) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "LoadGenericButtonImages: Couldn't get generic button's background pixel value");
    return (-1);
  }

  GenericButtonFillColors[ImgSlot] = GenericButtonOffNormal[ImgSlot]->p16BPPPalette[Pix];

  // Set the button's background image adjustement offsets
  GenericButtonOffsetX[ImgSlot] = OffsetX;
  GenericButtonOffsetY[ImgSlot] = OffsetY;

  // Return the slot number used.
  return (ImgSlot);
}

//=============================================================================
//	ShutdownButtonImageManager
//
//	Cleans up, and shuts down the button image manager sub-system.
//
//	This function is called by ShutdownButtonSystem.
//
void ShutdownButtonImageManager(void) {
  int x;

#ifdef BUTTONSYSTEM_DEBUGGING
  gfIgnoreShutdownAssertions = TRUE;
#endif

  // Remove all QuickButton images
  for (x = 0; x < MAX_BUTTON_PICS; x++) UnloadButtonImage(x);

  // Remove all GenericButton images
  for (x = 0; x < MAX_GENERIC_PICS; x++) {
    if (GenericButtonGrayed[x] != NULL) {
      DeleteVideoObject(GenericButtonGrayed[x]);
      GenericButtonGrayed[x] = NULL;
    }

    if (GenericButtonOffNormal[x] != NULL) {
      DeleteVideoObject(GenericButtonOffNormal[x]);
      GenericButtonOffNormal[x] = NULL;
    }

    if (GenericButtonOffHilite[x] != NULL) {
      DeleteVideoObject(GenericButtonOffHilite[x]);
      GenericButtonOffHilite[x] = NULL;
    }

    if (GenericButtonOnNormal[x] != NULL) {
      DeleteVideoObject(GenericButtonOnNormal[x]);
      GenericButtonOnNormal[x] = NULL;
    }

    if (GenericButtonOnHilite[x] != NULL) {
      DeleteVideoObject(GenericButtonOnHilite[x]);
      GenericButtonOnHilite[x] = NULL;
    }

    if (GenericButtonBackground[x] != NULL) {
      DeleteVideoObject(GenericButtonBackground[x]);
      GenericButtonBackground[x] = NULL;
    }

    GenericButtonFillColors[x] = 0;
    GenericButtonBackgroundIndex[x] = 0;
    GenericButtonOffsetX[x] = 0;
    GenericButtonOffsetY[x] = 0;
  }

  // Remove all button icons
  for (x = 0; x < MAX_BUTTON_ICONS; x++) {
    if (GenericButtonIcons[x] != NULL) GenericButtonIcons[x] = NULL;
  }
}

//=============================================================================
//	InitButtonSystem
//
//	Initializes the GUI button system for use. Must be called before using
//	any other button functions.
//
BOOLEAN InitButtonSystem(void) {
  int32_t x;

#ifdef BUTTONSYSTEM_DEBUGGING
  gfIgnoreShutdownAssertions = FALSE;
#endif

  RegisterDebugTopic(TOPIC_BUTTON_HANDLER, "Button System & Button Image Manager");

  // Clear out button list
  for (x = 0; x < MAX_BUTTONS; x++) {
    ButtonList[x] = NULL;
  }

  // Initialize the button image manager sub-system
  if (InitializeButtonImageManager() == FALSE) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Failed button image manager init\n");
    return (FALSE);
  }

  ButtonsInList = 0;

  return (TRUE);
}

//=============================================================================
//	ShutdownButtonSystem
//
//	Shuts down and cleans up the GUI button system. Must be called before
//	exiting the program. Button functions should not be used after calling
//	this function.
//
void ShutdownButtonSystem(void) {
  int x;

  // Kill off all buttons in the system
  for (x = 0; x < MAX_BUTTONS; x++) {
    if (ButtonList[x] != NULL) RemoveButton(x);
  }
  // Shutdown the button image manager sub-system
  ShutdownButtonImageManager();

  UnRegisterDebugTopic(TOPIC_BUTTON_HANDLER, "Button System & Button Image Manager");
}

void RemoveButtonsMarkedForDeletion() {
  int32_t i;
  for (i = 0; i < MAX_BUTTONS; i++) {
    if (ButtonList[i] && ButtonList[i]->uiFlags & BUTTON_DELETION_PENDING) {
      RemoveButton(i);
    }
  }
}

//=============================================================================
//	RemoveButton
//
//	Removes a button from the system's list. All memory associated with the
//	button is released.
//
void RemoveButton(int32_t iButtonID) {
  GUI_BUTTON *b;

  if (iButtonID < 0 || iButtonID >= MAX_BUTTONS) {
    sprintf(str, "Attempting to RemoveButton with out of range buttonID %d.", iButtonID);
    AssertMsg(0, str);
  }

  b = ButtonList[iButtonID];

  // If button exists...
  if (!b) {
#ifdef BUTTONSYSTEM_DEBUGGING
    if (gfIgnoreShutdownAssertions)
#endif
      return;
    AssertMsg(0, "Attempting to remove a button that has already been deleted.");
  }

  // If we happen to be in the middle of a callback, and attempt to delete a button,
  // like deleting a node during list processing, then we delay it till after the callback
  // is completed.
  if (gfDelayButtonDeletion) {
    b->uiFlags |= BUTTON_DELETION_PENDING;
    gfPendingButtonDeletion = TRUE;
    return;
  }

  // Kris:
  if (b->uiFlags & BUTTON_SELFDELETE_IMAGE) {  // checkboxes and simple create buttons have their
                                               // own graphics associated with them,
    // and it is handled internally.  We delete it here.  This provides the advantage of less
    // micromanagement, but with the disadvantage of wasting more memory if you have lots of
    // buttons using the same graphics.
    UnloadButtonImage(b->ImageNum);
  }

  // ...kill it!!!
  MSYS_RemoveRegion(&b->Area);

  if (b->uiFlags & BUTTON_SAVEBACKGROUND) {
    FreeBackgroundRectPending(b->BackRect);
  }

  // Get rid of the text string
  if (b->string != NULL) MemFree(b->string);

  if (b == gpAnchoredButton) gpAnchoredButton = NULL;
  if (b == gpPrevAnchoredButton) gpPrevAnchoredButton = NULL;

  MemFree(b);
  b = NULL;
  ButtonList[iButtonID] = NULL;
}

//=============================================================================
//	GetNextButtonNumber
//
//	Finds the next available button slot.
//
int32_t GetNextButtonNumber(void) {
  int32_t x;

  for (x = 0; x < MAX_BUTTONS; x++) {
    if (ButtonList[x] == NULL) return (x);
  }

  return (BUTTON_NO_SLOT);
}

//=============================================================================
//	ResizeButton
//
//	Changes the size of a generic button.
//
//	QuickButtons cannot be resized, therefore this function ignores the
//	call if a QuickButton is given.
//
void ResizeButton(int32_t iButtonID, int16_t w, int16_t h) {
  GUI_BUTTON *b;
  int32_t xloc, yloc;

  if (iButtonID < 0 || iButtonID >= MAX_BUTTONS) {
    sprintf(str, "Attempting to resize button with out of range buttonID %d.", iButtonID);
    AssertMsg(0, str);
  }

  // if button size is too small, adjust it.
  if (w < 4) w = 4;
  if (h < 3) h = 3;

  b = ButtonList[iButtonID];

  if (!b) {
    sprintf(str, "Attempting to resize deleted button with buttonID %d", iButtonID);
    AssertMsg(0, str);
  }

  // If this is a QuickButton, ignore this call
  if ((b->uiFlags & BUTTON_TYPES) == BUTTON_QUICK) return;

  // Get current button screen location
  xloc = b->XLoc;
  yloc = b->YLoc;

  // Set the new struct MOUSE_REGION area values to reflect change in size.
  b->Area.RegionTopLeftX = (uint16_t)xloc;
  b->Area.RegionTopLeftY = (uint16_t)yloc;
  b->Area.RegionBottomRightX = (uint16_t)(xloc + w);
  b->Area.RegionBottomRightY = (uint16_t)(yloc + h);
  b->uiFlags |= BUTTON_DIRTY;

  if (b->uiFlags & BUTTON_SAVEBACKGROUND) {
    FreeBackgroundRectPending(b->BackRect);
    b->BackRect =
        RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, NULL, (int16_t)xloc,
                               (int16_t)yloc, (int16_t)(xloc + w), (int16_t)(yloc + h));
  }
}

//=============================================================================
//	SetButtonPosition
//
//	Sets the position of a button on the screen. The position is relative
//	to the top left corner of the button.
//
void SetButtonPosition(int32_t iButtonID, int16_t x, int16_t y) {
  GUI_BUTTON *b;
  int32_t xloc, yloc, w, h;

  if (iButtonID < 0 || iButtonID >= MAX_BUTTONS) {
    sprintf(str, "Attempting to set button position with out of range buttonID %d.", iButtonID);
    AssertMsg(0, str);
  }

  b = ButtonList[iButtonID];

  if (!b) {
    sprintf(str, "Attempting to set button position with buttonID %d", iButtonID);
    AssertMsg(0, str);
  }

  // Get new screen position
  xloc = (int16_t)x;
  yloc = (int16_t)y;
  // Compute current width and height of this button
  w = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
  h = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

  // Set button to new location
  b->XLoc = x;
  b->YLoc = y;
  // Set the buttons struct MOUSE_REGION to appropriate area
  b->Area.RegionTopLeftX = (uint16_t)xloc;
  b->Area.RegionTopLeftY = (uint16_t)yloc;
  b->Area.RegionBottomRightX = (uint16_t)(xloc + w);
  b->Area.RegionBottomRightY = (uint16_t)(yloc + h);
  b->uiFlags |= BUTTON_DIRTY;

  if (b->uiFlags & BUTTON_SAVEBACKGROUND) {
    FreeBackgroundRectPending(b->BackRect);
    b->BackRect =
        RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, NULL, (int16_t)xloc,
                               (int16_t)yloc, (int16_t)(xloc + w), (int16_t)(yloc + h));
  }
}

//=============================================================================
//	SetButtonIcon
//
//	Sets the icon to be displayed on a IconicButton.
//
//	Calling this function with a button type other than Iconic has no effect.
//
int32_t SetButtonIcon(int32_t iButtonID, int16_t Icon, int16_t IconIndex) {
  GUI_BUTTON *b;

  if (iButtonID < 0 || iButtonID >= MAX_BUTTONS) {
    sprintf(str, "Attempting to set button icon with out of range buttonID %d.", iButtonID);
    AssertMsg(0, str);
    return -1;
  }
  if (Icon < 0 || Icon >= MAX_BUTTON_ICONS) {
    sprintf(str, "Attempting to set button[%d] icon with out of range icon index %d.", iButtonID,
            Icon);
    AssertMsg(0, str);
    return -1;
  }

  b = ButtonList[iButtonID];

  if (!b) {
    sprintf(str, "Attempting to set deleted button icon with buttonID %d", iButtonID);
    AssertMsg(0, str);
    return -1;
  }

  // If button isn't an icon button, ignore this call
  if (((b->uiFlags & BUTTON_TYPES) == BUTTON_QUICK) ||
      ((b->uiFlags & BUTTON_TYPES) == BUTTON_HOT_SPOT) ||
      ((b->uiFlags & BUTTON_TYPES) == BUTTON_GENERIC)) {
    return -1;
  }

  // Set the icon number and index to use for this button
  b->iIconID = Icon;
  b->usIconIndex = IconIndex;

  return Icon;
}

//=============================================================================
//	CreateIconButton
//
//	Creates an Iconic type button.
//
int32_t CreateIconButton(int16_t Icon, int16_t IconIndex, int16_t GenImg, int16_t xloc,
                         int16_t yloc, int16_t w, int16_t h, int32_t Type, int16_t Priority,
                         GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback) {
  GUI_BUTTON *b;
  int32_t ButtonNum;
  int32_t BType, x;

  if (xloc < 0 || yloc < 0) {
    sprintf(str, "Attempting to CreateIconButton with invalid position of %d,%d", xloc, yloc);
    AssertMsg(0, str);
  }
  if (GenImg < -1 || GenImg >= MAX_GENERIC_PICS) {
    sprintf(str, "Attempting to CreateIconButton with out of range iconID %d.", GenImg);
    AssertMsg(0, str);
  }

  // if button size is too small, adjust it.
  if (w < 4) w = 4;
  if (h < 3) h = 3;

  // Strip off any extraneous bits from button type
  BType = Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE);

  // Get a button number (slot) for this new button
  if ((ButtonNum = GetNextButtonNumber()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateIconButton: No more button slots");
    return (-1);
  }

  // Allocate memory for the GUI_BUTTON structure
  if ((b = (GUI_BUTTON *)MemAlloc(sizeof(GUI_BUTTON))) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "CreateIconButton: Can't alloc mem for button struct");
    return (-1);
  }

  // Init the values in the struct
  b->uiFlags = BUTTON_DIRTY;
  b->uiOldFlags = 0;
  b->IDNum = ButtonNum;
  b->XLoc = xloc;
  b->YLoc = yloc;

  if (GenImg < 0)
    b->ImageNum = 0;
  else
    b->ImageNum = GenImg;

  for (x = 0; x < 4; x++) b->UserData[x] = 0;
  b->Group = -1;

  b->bDefaultStatus = DEFAULT_STATUS_NONE;
  b->bDisabledStyle = DISABLED_STYLE_DEFAULT;
  // Init text
  b->string = NULL;
  b->usFont = 0;
  b->fMultiColor = FALSE;
  b->sForeColor = 0;
  b->sWrappedWidth = -1;
  b->sShadowColor = -1;
  b->sForeColorDown = -1;
  b->sShadowColorDown = -1;
  b->sForeColorHilited = -1;
  b->sShadowColorHilited = -1;
  b->bJustification = BUTTON_TEXT_CENTER;
  b->bTextXOffset = -1;
  b->bTextYOffset = -1;
  b->bTextXSubOffSet = 0;
  b->bTextYSubOffSet = 0;
  b->fShiftText = TRUE;
  // Init icon
  b->iIconID = Icon;
  b->usIconIndex = IconIndex;
  b->bIconXOffset = -1;
  b->bIconYOffset = -1;
  b->fShiftImage = TRUE;

  // Set the click callback function (if any)
  if (ClickCallback != BUTTON_NO_CALLBACK) {
    b->ClickCallback = ClickCallback;
    BType |= BUTTON_CLICK_CALLBACK;
  } else
    b->ClickCallback = BUTTON_NO_CALLBACK;

  // Set the move callback function (if any)
  if (MoveCallback != BUTTON_NO_CALLBACK) {
    b->MoveCallback = MoveCallback;
    BType |= BUTTON_MOVE_CALLBACK;
  } else
    b->MoveCallback = BUTTON_NO_CALLBACK;

  // Define a mouse region for this button
  MSYS_DefineRegion(&b->Area, (uint16_t)xloc, (uint16_t)yloc, (uint16_t)(xloc + w),
                    (uint16_t)(yloc + h), (int8_t)Priority, MSYS_STARTING_CURSORVAL,
                    (MOUSE_CALLBACK)QuickButtonCallbackMMove,
                    (MOUSE_CALLBACK)QuickButtonCallbackMButn);

  // Link the mouse region to this button (for callback purposes)
  MSYS_SetRegionUserData(&b->Area, 0, ButtonNum);

  // Set this button's flags
  b->uiFlags |= (BUTTON_ENABLED | BType | BUTTON_GENERIC);

  b->BackRect = -1;

// Add button to the button list
#ifdef BUTTONSYSTEM_DEBUGGING
  AssertFailIfIdenticalButtonAttributesFound(b);
#endif
  ButtonList[ButtonNum] = b;

  SpecifyButtonSoundScheme(b->IDNum, BUTTON_SOUND_SCHEME_GENERIC);

  // return this button's slot number
  return (ButtonNum);
}

// Creates a generic button with text on it.
int32_t CreateTextButton(wchar_t *string, uint32_t uiFont, int16_t sForeColor, int16_t sShadowColor,
                         int16_t GenImg, int16_t xloc, int16_t yloc, int16_t w, int16_t h,
                         int32_t Type, int16_t Priority, GUI_CALLBACK MoveCallback,
                         GUI_CALLBACK ClickCallback) {
  GUI_BUTTON *b;
  int32_t ButtonNum;
  int32_t BType, x;

  if (xloc < 0 || yloc < 0) {
    sprintf(str, "Attempting to CreateTextButton with invalid position of %d,%d", xloc, yloc);
    AssertMsg(0, str);
  }
  if (GenImg < -1 || GenImg >= MAX_GENERIC_PICS) {
    sprintf(str, "Attempting to CreateTextButton with out of range iconID %d.", GenImg);
    AssertMsg(0, str);
  }

  // if button size is too small, adjust it.
  if (w < 4) w = 4;
  if (h < 3) h = 3;

  // Strip off any extraneous bits from button type
  BType = Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE);

  // Get a button number for this new button
  if ((ButtonNum = GetNextButtonNumber()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateTextButton: No more button slots");
    return (-1);
  }

  // Allocate memory for a GUI_BUTTON structure
  if ((b = (GUI_BUTTON *)MemAlloc(sizeof(GUI_BUTTON))) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "CreateTextButton: Can't alloc mem for button struct");
    return (-1);
  }

  // Allocate memory for the button's text string...
  b->string = NULL;
  if (string && wcslen(string)) {
    b->string = (wchar_t *)MemAlloc((wcslen(string) + 1) * sizeof(wchar_t));
    AssertMsg(b->string, "Out of memory error:  Couldn't allocate string in CreateTextButton.");
    wcscpy(b->string, string);
  }

  // Init the button structure variables
  b->uiFlags = BUTTON_DIRTY;
  b->uiOldFlags = 0;
  b->IDNum = ButtonNum;
  b->XLoc = xloc;
  b->YLoc = yloc;

  if (GenImg < 0)
    b->ImageNum = 0;
  else
    b->ImageNum = GenImg;

  for (x = 0; x < 4; x++) b->UserData[x] = 0;
  b->Group = -1;
  b->bDefaultStatus = DEFAULT_STATUS_NONE;
  b->bDisabledStyle = DISABLED_STYLE_DEFAULT;
  // Init string
  b->usFont = (uint16_t)uiFont;
  b->fMultiColor = FALSE;
  b->sForeColor = sForeColor;
  b->sWrappedWidth = -1;
  b->sShadowColor = sShadowColor;
  b->sForeColorDown = -1;
  b->sShadowColorDown = -1;
  b->sForeColorHilited = -1;
  b->sShadowColorHilited = -1;
  b->bJustification = BUTTON_TEXT_CENTER;
  b->bTextXOffset = -1;
  b->bTextYOffset = -1;
  b->bTextXSubOffSet = 0;
  b->bTextYSubOffSet = 0;
  b->fShiftText = TRUE;
  // Init icon
  b->iIconID = -1;
  b->usIconIndex = -1;
  b->bIconXOffset = -1;
  b->bIconYOffset = -1;
  b->fShiftImage = TRUE;

  // Set the button click callback function (if any)
  if (ClickCallback != BUTTON_NO_CALLBACK) {
    b->ClickCallback = ClickCallback;
    BType |= BUTTON_CLICK_CALLBACK;
  } else
    b->ClickCallback = BUTTON_NO_CALLBACK;

  // Set the button's mouse movement callback function (if any)
  if (MoveCallback != BUTTON_NO_CALLBACK) {
    b->MoveCallback = MoveCallback;
    BType |= BUTTON_MOVE_CALLBACK;
  } else
    b->MoveCallback = BUTTON_NO_CALLBACK;

  // Define a struct MOUSE_REGION for this button
  MSYS_DefineRegion(&b->Area, (uint16_t)xloc, (uint16_t)yloc, (uint16_t)(xloc + w),
                    (uint16_t)(yloc + h), (int8_t)Priority, MSYS_STARTING_CURSORVAL,
                    (MOUSE_CALLBACK)QuickButtonCallbackMMove,
                    (MOUSE_CALLBACK)QuickButtonCallbackMButn);

  // Link the struct MOUSE_REGION to this button
  MSYS_SetRegionUserData(&b->Area, 0, ButtonNum);

  // Set the flags for this button
  b->uiFlags |= (BUTTON_ENABLED | BType | BUTTON_GENERIC);

  b->BackRect = -1;

// Add this button to the button list
#ifdef BUTTONSYSTEM_DEBUGGING
  AssertFailIfIdenticalButtonAttributesFound(b);
#endif
  ButtonList[ButtonNum] = b;

  SpecifyButtonSoundScheme(b->IDNum, BUTTON_SOUND_SCHEME_GENERIC);

  // return the slot number
  return (ButtonNum);
}

//=============================================================================
//	CreateHotSpot
//
//	Creates a button like HotSpot. HotSpots have no graphics associated with
//	them.
//
int32_t CreateHotSpot(int16_t xloc, int16_t yloc, int16_t Width, int16_t Height, int16_t Priority,
                      GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback) {
  GUI_BUTTON *b;
  int32_t ButtonNum;
  int16_t BType, x;

  if (xloc < 0 || yloc < 0 || Width < 0 || Height < 0) {
    sprintf(
        str,
        "Attempting to CreateHotSpot with invalid coordinates: %d,%d, width: %d, and height: %d.",
        xloc, yloc, Width, Height);
    AssertMsg(0, str);
  }

  BType = 0;

  // Get a button number for this hotspot
  if ((ButtonNum = GetNextButtonNumber()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateHotSpot: No more button slots");
    return (-1);
  }

  // Allocate memory for the GUI_BUTTON structure
  if ((b = (GUI_BUTTON *)MemAlloc(sizeof(GUI_BUTTON))) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "CreateHotSpot: Can't alloc mem for button struct");
    return (-1);
  }

  // Init the structure values
  b->uiFlags = 0;
  b->uiOldFlags = 0;
  b->IDNum = ButtonNum;
  b->XLoc = xloc;
  b->YLoc = yloc;
  b->ImageNum = 0xffffffff;
  for (x = 0; x < 4; x++) b->UserData[x] = 0;
  b->Group = -1;
  b->string = NULL;

  // Set the hotspot click callback function (if any)
  if (ClickCallback != BUTTON_NO_CALLBACK) {
    b->ClickCallback = ClickCallback;
    BType |= BUTTON_CLICK_CALLBACK;
  } else
    b->ClickCallback = BUTTON_NO_CALLBACK;

  // Set the hotspot's mouse movement callback function (if any)
  if (MoveCallback != BUTTON_NO_CALLBACK) {
    b->MoveCallback = MoveCallback;
    BType |= BUTTON_MOVE_CALLBACK;
  } else
    b->MoveCallback = BUTTON_NO_CALLBACK;

  // define a struct MOUSE_REGION for this hotspot
  MSYS_DefineRegion(&b->Area, (uint16_t)xloc, (uint16_t)yloc, (uint16_t)(xloc + Width),
                    (uint16_t)(yloc + Height), (int8_t)Priority, MSYS_STARTING_CURSORVAL,
                    (MOUSE_CALLBACK)QuickButtonCallbackMMove,
                    (MOUSE_CALLBACK)QuickButtonCallbackMButn);

  // Link the struct MOUSE_REGION to this hotspot
  MSYS_SetRegionUserData(&b->Area, 0, ButtonNum);

  // Set the flags entry for this hotspot
  b->uiFlags |= (BUTTON_ENABLED | BType | BUTTON_HOT_SPOT);

  b->BackRect = -1;

// Add this button (hotspot) to the button list
#ifdef BUTTONSYSTEM_DEBUGGING
  AssertFailIfIdenticalButtonAttributesFound(b);
#endif
  ButtonList[ButtonNum] = b;

  SpecifyButtonSoundScheme(b->IDNum, BUTTON_SOUND_SCHEME_GENERIC);

  // return the button slot number
  return (ButtonNum);
}

// ============================================================================
// Addition Oct15/97, Carter
// SetButtonCursor
// will simply set the cursor for the mouse region the button occupies
BOOLEAN SetButtonCursor(int32_t iBtnId, uint16_t crsr) {
  GUI_BUTTON *b;
  b = ButtonList[iBtnId];
  if (!b) return FALSE;
  b->Area.Cursor = crsr;
  return TRUE;
}

//=============================================================================
//	QuickCreateButton
//
//	Creates a QuickButton. QuickButtons only have graphics associated with
//	them. They cannot be re-sized, nor can the graphic be changed.
//
int32_t QuickCreateButton(uint32_t Image, int16_t xloc, int16_t yloc, int32_t Type,
                          int16_t Priority, GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback) {
  GUI_BUTTON *b;
  int32_t ButtonNum;
  int32_t BType, x;

  if (xloc < 0 || yloc < 0) {
    sprintf(str, "Attempting to QuickCreateButton with invalid position of %d,%d", xloc, yloc);
    AssertMsg(0, str);
  }
  if (Image < 0 || Image >= MAX_BUTTON_PICS) {
    sprintf(str, "Attempting to QuickCreateButton with out of range ImageID %d.", Image);
    AssertMsg(0, str);
  }

  // Strip off any extraneous bits from button type
  BType = Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE);

  // Is there a QuickButton image in the given image slot?
  if (ButtonPictures[Image].vobj == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "QuickCreateButton: Invalid button image number");
    return (-1);
  }

  // Get a new button number
  if ((ButtonNum = GetNextButtonNumber()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "QuickCreateButton: No more button slots");
    return (-1);
  }

  // Allocate memory for a GUI_BUTTON structure
  if ((b = (GUI_BUTTON *)MemAlloc(sizeof(GUI_BUTTON))) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "QuickCreateButton: Can't alloc mem for button struct");
    return (-1);
  }

  // Set the values for this buttn
  b->uiFlags = BUTTON_DIRTY;
  b->uiOldFlags = 0;

  // Set someflags if of s certain type....
  if (Type & BUTTON_NEWTOGGLE) {
    b->uiFlags |= BUTTON_NEWTOGGLE;
  }

  // shadow style
  b->bDefaultStatus = DEFAULT_STATUS_NONE;
  b->bDisabledStyle = DISABLED_STYLE_DEFAULT;

  b->Group = -1;
  // Init string
  b->string = NULL;
  b->usFont = 0;
  b->fMultiColor = FALSE;
  b->sForeColor = 0;
  b->sWrappedWidth = -1;
  b->sShadowColor = -1;
  b->sForeColorDown = -1;
  b->sShadowColorDown = -1;
  b->sForeColorHilited = -1;
  b->sShadowColorHilited = -1;
  b->bJustification = BUTTON_TEXT_CENTER;
  b->bTextXOffset = -1;
  b->bTextYOffset = -1;
  b->bTextXSubOffSet = 0;
  b->bTextYSubOffSet = 0;
  b->fShiftText = TRUE;
  // Init icon
  b->iIconID = -1;
  b->usIconIndex = -1;
  b->bIconXOffset = -1;
  b->bIconYOffset = -1;
  b->fShiftImage = TRUE;
  // Init quickbutton
  b->IDNum = ButtonNum;
  b->ImageNum = Image;
  for (x = 0; x < 4; x++) b->UserData[x] = 0;

  b->XLoc = xloc;
  b->YLoc = yloc;

  b->ubToggleButtonOldState = 0;
  b->ubToggleButtonActivated = FALSE;

  // Set the button click callback function (if any)
  if (ClickCallback != BUTTON_NO_CALLBACK) {
    b->ClickCallback = ClickCallback;
    BType |= BUTTON_CLICK_CALLBACK;
  } else
    b->ClickCallback = BUTTON_NO_CALLBACK;

  // Set the button's mouse movement callback function (if any)
  if (MoveCallback != BUTTON_NO_CALLBACK) {
    b->MoveCallback = MoveCallback;
    BType |= BUTTON_MOVE_CALLBACK;
  } else
    b->MoveCallback = BUTTON_NO_CALLBACK;

  memset(&b->Area, 0, sizeof(struct MOUSE_REGION));
  // Define a struct MOUSE_REGION for this QuickButton
  MSYS_DefineRegion(&b->Area, (uint16_t)xloc, (uint16_t)yloc,
                    (uint16_t)(xloc + (int16_t)ButtonPictures[Image].MaxWidth),
                    (uint16_t)(yloc + (int16_t)ButtonPictures[Image].MaxHeight), (int8_t)Priority,
                    MSYS_STARTING_CURSORVAL, (MOUSE_CALLBACK)QuickButtonCallbackMMove,
                    (MOUSE_CALLBACK)QuickButtonCallbackMButn);

  // Link the struct MOUSE_REGION with this QuickButton
  MSYS_SetRegionUserData(&b->Area, 0, ButtonNum);

  // Set the flags for this button
  b->uiFlags |= BUTTON_ENABLED | BType | BUTTON_QUICK;
  b->BackRect = -1;

// Add this QuickButton to the button list
#ifdef BUTTONSYSTEM_DEBUGGING
  AssertFailIfIdenticalButtonAttributesFound(b);
#endif
  ButtonList[ButtonNum] = b;

  SpecifyButtonSoundScheme(b->IDNum, BUTTON_SOUND_SCHEME_GENERIC);

  // return the button number (slot)
  return (ButtonNum);
}

// A hybrid of QuickCreateButton.  Takes a lot less parameters, but makes more assumptions.  It self
// manages the loading, and deleting of the image.  The size of the image determines the size of the
// button.  It also uses the default move callback which emulates Win95.  Finally, it sets the
// priority to normal.  The function you choose also determines the type of button (toggle,
// notoggle, or newtoggle)
int32_t CreateEasyNoToggleButton(int32_t x, int32_t y, char *filename, GUI_CALLBACK ClickCallback) {
  return CreateSimpleButton(x, y, filename, BUTTON_NO_TOGGLE, MSYS_PRIORITY_NORMAL, ClickCallback);
}

int32_t CreateEasyToggleButton(int32_t x, int32_t y, char *filename, GUI_CALLBACK ClickCallback) {
  return CreateSimpleButton(x, y, filename, BUTTON_TOGGLE, MSYS_PRIORITY_NORMAL, ClickCallback);
}

int32_t CreateEasyNewToggleButton(int32_t x, int32_t y, char *filename,
                                  GUI_CALLBACK ClickCallback) {
  return CreateSimpleButton(x, y, filename, BUTTON_NEWTOGGLE, MSYS_PRIORITY_NORMAL, ClickCallback);
}

// Same as above, but accepts specify toggle type
int32_t CreateEasyButton(int32_t x, int32_t y, char *filename, int32_t Type,
                         GUI_CALLBACK ClickCallback) {
  return CreateSimpleButton(x, y, filename, Type, MSYS_PRIORITY_NORMAL, ClickCallback);
}

// Same as above, but accepts priority specification.
int32_t CreateSimpleButton(int32_t x, int32_t y, char *filename, int32_t Type, int16_t Priority,
                           GUI_CALLBACK ClickCallback) {
  int32_t ButPic, ButNum;

  if (!filename || !strlen(filename))
    AssertMsg(0, "Attempting to CreateSimpleButton with null filename.");

  if ((ButPic = LoadButtonImage(filename, -1, 1, 2, 3, 4)) == -1) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "Can't load button image");
    return (-1);
  }

  ButNum = (int16_t)QuickCreateButton(ButPic, (int16_t)x, (int16_t)y, Type, Priority,
                                      DEFAULT_MOVE_CALLBACK, ClickCallback);

  AssertMsg(ButNum != -1, "Failed to CreateSimpleButton.");

  ButtonList[ButNum]->uiFlags |= BUTTON_SELFDELETE_IMAGE;

  SpecifyDisabledButtonStyle(ButNum, DISABLED_STYLE_SHADED);

  return (ButNum);
}

int32_t CreateIconAndTextButton(int32_t Image, wchar_t *string, uint32_t uiFont, int16_t sForeColor,
                                int16_t sShadowColor, int16_t sForeColorDown,
                                int16_t sShadowColorDown, int8_t bJustification, int16_t xloc,
                                int16_t yloc, int32_t Type, int16_t Priority,
                                GUI_CALLBACK MoveCallback, GUI_CALLBACK ClickCallback) {
  GUI_BUTTON *b;
  int32_t iButtonID;
  int32_t BType, x;

  if (xloc < 0 || yloc < 0) {
    sprintf(str, "Attempting to CreateIconAndTextButton with invalid position of %d,%d", xloc,
            yloc);
    AssertMsg(0, str);
  }
  if (Image < 0 || Image >= MAX_BUTTON_PICS) {
    sprintf(str, "Attemting to CreateIconAndTextButton with out of range ImageID %d.", Image);
    AssertMsg(0, str);
  }

  // Strip off any extraneous bits from button type
  BType = Type & (BUTTON_TYPE_MASK | BUTTON_NEWTOGGLE);

  // Is there a QuickButton image in the given image slot?
  if (ButtonPictures[Image].vobj == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "QuickCreateButton: Invalid button image number");
    return (-1);
  }

  // Get a new button number
  if ((iButtonID = GetNextButtonNumber()) == BUTTON_NO_SLOT) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "QuickCreateButton: No more button slots");
    return (-1);
  }

  // Allocate memory for a GUI_BUTTON structure
  if ((b = (GUI_BUTTON *)MemAlloc(sizeof(GUI_BUTTON))) == NULL) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0,
               "QuickCreateButton: Can't alloc mem for button struct");
    return (-1);
  }

  // Set the values for this button
  b->uiFlags = BUTTON_DIRTY;
  b->uiOldFlags = 0;
  b->IDNum = iButtonID;
  b->XLoc = xloc;
  b->YLoc = yloc;
  b->ImageNum = Image;
  for (x = 0; x < 4; x++) b->UserData[x] = 0;
  b->Group = -1;
  b->bDefaultStatus = DEFAULT_STATUS_NONE;
  b->bDisabledStyle = DISABLED_STYLE_DEFAULT;

  // Allocate memory for the button's text string...
  b->string = NULL;
  if (string) {
    b->string = (wchar_t *)MemAlloc((wcslen(string) + 1) * sizeof(wchar_t));
    AssertMsg(b->string,
              "Out of memory error:  Couldn't allocate string in CreateIconAndTextButton.");
    wcscpy(b->string, string);
  }

  b->bJustification = bJustification;
  b->usFont = (uint16_t)uiFont;
  b->fMultiColor = FALSE;
  b->sForeColor = sForeColor;
  b->sWrappedWidth = -1;
  b->sShadowColor = sShadowColor;
  b->sForeColorDown = sForeColorDown;
  b->sShadowColorDown = sShadowColorDown;
  b->sForeColorHilited = -1;
  b->sShadowColorHilited = -1;
  b->bTextXOffset = -1;
  b->bTextYOffset = -1;
  b->bTextXSubOffSet = 0;
  b->bTextYSubOffSet = 0;
  b->fShiftText = TRUE;

  b->iIconID = -1;
  b->usIconIndex = 0;

  // Set the button click callback function (if any)
  if (ClickCallback != BUTTON_NO_CALLBACK) {
    b->ClickCallback = ClickCallback;
    BType |= BUTTON_CLICK_CALLBACK;
  } else
    b->ClickCallback = BUTTON_NO_CALLBACK;

  // Set the button's mouse movement callback function (if any)
  if (MoveCallback != BUTTON_NO_CALLBACK) {
    b->MoveCallback = MoveCallback;
    BType |= BUTTON_MOVE_CALLBACK;
  } else
    b->MoveCallback = BUTTON_NO_CALLBACK;

  // Define a struct MOUSE_REGION for this QuickButton
  MSYS_DefineRegion(&b->Area, (uint16_t)xloc, (uint16_t)yloc,
                    (uint16_t)(xloc + (int16_t)ButtonPictures[Image].MaxWidth),
                    (uint16_t)(yloc + (int16_t)ButtonPictures[Image].MaxHeight), (int8_t)Priority,
                    MSYS_STARTING_CURSORVAL, (MOUSE_CALLBACK)QuickButtonCallbackMMove,
                    (MOUSE_CALLBACK)QuickButtonCallbackMButn);

  // Link the struct MOUSE_REGION with this QuickButton
  MSYS_SetRegionUserData(&b->Area, 0, iButtonID);

  // Set the flags for this button
  b->uiFlags |= (BUTTON_ENABLED | BType | BUTTON_QUICK);

  b->BackRect = -1;

// Add this QuickButton to the button list
#ifdef BUTTONSYSTEM_DEBUGGING
  AssertFailIfIdenticalButtonAttributesFound(b);
#endif
  ButtonList[iButtonID] = b;

  SpecifyButtonSoundScheme(b->IDNum, BUTTON_SOUND_SCHEME_GENERIC);

  // return the button number (slot)
  return (iButtonID);
}

// New functions
void SpecifyButtonText(int32_t iButtonID, wchar_t *string) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);

  b = ButtonList[iButtonID];

  // free the previous strings memory if applicable
  if (b->string) MemFree(b->string);
  b->string = NULL;

  if (string && wcslen(string)) {
    // allocate memory for the new string
    b->string = (wchar_t *)MemAlloc((wcslen(string) + 1) * sizeof(wchar_t));
    Assert(b->string);
    // copy the string to the button
    wcscpy(b->string, string);
    b->uiFlags |= BUTTON_DIRTY;
  }
}

void SpecifyButtonMultiColorFont(int32_t iButtonID, BOOLEAN fMultiColor) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  b->fMultiColor = fMultiColor;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyButtonFont(int32_t iButtonID, uint32_t uiFont) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  b->usFont = (uint16_t)uiFont;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyButtonUpTextColors(int32_t iButtonID, int16_t sForeColor, int16_t sShadowColor) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  b->sForeColor = sForeColor;
  b->sShadowColor = sShadowColor;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyButtonDownTextColors(int32_t iButtonID, int16_t sForeColorDown,
                                 int16_t sShadowColorDown) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  b->sForeColorDown = sForeColorDown;
  b->sShadowColorDown = sShadowColorDown;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyButtonHilitedTextColors(int32_t iButtonID, int16_t sForeColorHilited,
                                    int16_t sShadowColorHilited) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  b->sForeColorHilited = sForeColorHilited;
  b->sShadowColorHilited = sShadowColorHilited;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyButtonTextJustification(int32_t iButtonID, int8_t bJustification) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Range check:  if invalid, then set it to center justified.
  if (bJustification < BUTTON_TEXT_LEFT || bJustification > BUTTON_TEXT_RIGHT)
    bJustification = BUTTON_TEXT_CENTER;
  b->bJustification = bJustification;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyFullButtonTextAttributes(int32_t iButtonID, wchar_t *string, int32_t uiFont,
                                     int16_t sForeColor, int16_t sShadowColor,
                                     int16_t sForeColorDown, int16_t sShadowColorDown,
                                     int8_t bJustification) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Copy over information
  SpecifyButtonText(iButtonID, string);
  b->usFont = (uint16_t)uiFont;
  b->sForeColor = sForeColor;
  b->sShadowColor = sShadowColor;
  b->sForeColorDown = sForeColorDown;
  b->sShadowColorDown = sShadowColorDown;
  // Range check:  if invalid, then set it to center justified.
  if (bJustification < BUTTON_TEXT_LEFT || bJustification > BUTTON_TEXT_RIGHT)
    bJustification = BUTTON_TEXT_CENTER;
  b->bJustification = bJustification;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyGeneralButtonTextAttributes(int32_t iButtonID, wchar_t *string, int32_t uiFont,
                                        int16_t sForeColor, int16_t sShadowColor) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Copy over information
  SpecifyButtonText(iButtonID, string);
  b->usFont = (uint16_t)uiFont;
  b->sForeColor = sForeColor;
  b->sShadowColor = sShadowColor;
  b->uiFlags |= BUTTON_DIRTY;
}

void SpecifyButtonTextOffsets(int32_t iButtonID, int8_t bTextXOffset, int8_t bTextYOffset,
                              BOOLEAN fShiftText) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Copy over information
  b->bTextXOffset = bTextXOffset;
  b->bTextYOffset = bTextYOffset;
  b->fShiftText = fShiftText;
}

void SpecifyButtonTextSubOffsets(int32_t iButtonID, int8_t bTextXOffset, int8_t bTextYOffset,
                                 BOOLEAN fShiftText) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Copy over information
  b->bTextXSubOffSet = bTextXOffset;
  b->bTextYSubOffSet = bTextYOffset;
  b->fShiftText = fShiftText;
}

void SpecifyButtonTextWrappedWidth(int32_t iButtonID, int16_t sWrappedWidth) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  b->sWrappedWidth = sWrappedWidth;
}

void SpecifyDisabledButtonStyle(int32_t iButtonID, int8_t bStyle) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  Assert(bStyle >= DISABLED_STYLE_NONE && bStyle <= DISABLED_STYLE_SHADED);

  b->bDisabledStyle = bStyle;
}

// Note:  Text is always on top
// If fShiftImage is true, then the image will shift down one pixel and right one pixel
// just like the text does.
BOOLEAN SpecifyButtonIcon(int32_t iButtonID, int32_t iVideoObjectID, uint16_t usVideoObjectIndex,
                          int8_t bXOffset, int8_t bYOffset, BOOLEAN fShiftImage) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  b->iIconID = iVideoObjectID;
  b->usIconIndex = usVideoObjectIndex;

  if (b->iIconID == -1) return FALSE;

  b->bIconXOffset = bXOffset;
  b->bIconYOffset = bYOffset;
  b->fShiftImage = TRUE;

  b->uiFlags |= BUTTON_DIRTY;

  return TRUE;
}

void RemoveTextFromButton(int32_t iButtonID) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Init string
  if (b->string) MemFree(b->string);
  b->string = NULL;
  b->usFont = 0;
  b->sForeColor = 0;
  b->sWrappedWidth = -1;
  b->sShadowColor = -1;
  b->sForeColorDown = -1;
  b->sShadowColorDown = -1;
  b->sForeColorHilited = -1;
  b->sShadowColorHilited = -1;
  b->bJustification = BUTTON_TEXT_CENTER;
  b->bTextXOffset = -1;
  b->bTextYOffset = -1;
  b->bTextXSubOffSet = 0;
  b->bTextYSubOffSet = 0;
  b->fShiftText = TRUE;
}

void RemoveIconFromButton(int32_t iButtonID) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);
  // Clear icon
  b->iIconID = -1;
  b->usIconIndex = -1;
  b->bIconXOffset = -1;
  b->bIconYOffset = -1;
  b->fShiftImage = TRUE;
}

void AllowDisabledButtonFastHelp(int32_t iButtonID, BOOLEAN fAllow) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  b->Area.uiFlags |= MSYS_ALLOW_DISABLED_FASTHELP;
}

//=============================================================================
//	SetButtonFastHelpText
//
//	Set the text that will be displayed as the FastHelp
//
void SetButtonFastHelpText(int32_t iButton, wchar_t *Text) {
  GUI_BUTTON *b;
  if (iButton < 0 || iButton > MAX_BUTTONS) return;
  b = ButtonList[iButton];
  AssertMsg(b, "Called SetButtonFastHelpText() with a non-existant button.");
  SetRegionFastHelpText(&b->Area, Text);
}

void SetBtnHelpEndCallback(int32_t iButton, MOUSE_HELPTEXT_DONE_CALLBACK CallbackFxn) {
  GUI_BUTTON *b;
  if (iButton < 0 || iButton > MAX_BUTTONS) return;
  b = ButtonList[iButton];
  AssertMsg(b, "Called SetBtnHelpEndCallback() with a non-existant button.");

  SetRegionHelpEndCallback(&b->Area, CallbackFxn);
}

//=============================================================================
//	QuickButtonCallbackMMove
//
//	Dispatches all button callbacks for mouse movement. This function gets
//	called by the Mouse System. *DO NOT CALL DIRECTLY*
//
void QuickButtonCallbackMMove(struct MOUSE_REGION *reg, int32_t reason) {
  GUI_BUTTON *b;
  int32_t iButtonID;

  Assert(reg != NULL);

  iButtonID = MSYS_GetRegionUserData(reg, 0);

  sprintf(str,
          "QuickButtonCallbackMMove: Mouse Region #%d (%d,%d to %d,%d) has invalid buttonID %d",
          reg->IDNumber, reg->RegionTopLeftX, reg->RegionTopLeftY, reg->RegionBottomRightX,
          reg->RegionBottomRightY, iButtonID);

  AssertMsg(iButtonID >= 0, str);
  AssertMsg(iButtonID < MAX_BUTTONS, str);

  b = ButtonList[iButtonID];

  AssertMsg(b != NULL, str);

  if (!b) return;  // This is getting called when Adding new regions...

  if (b->uiFlags & BUTTON_ENABLED &&
      reason & (MSYS_CALLBACK_REASON_LOST_MOUSE | MSYS_CALLBACK_REASON_GAIN_MOUSE)) {
    b->uiFlags |= BUTTON_DIRTY;
  }

  // Mouse moved on the button, so reset it's timer to maximum.
  if (b->Area.uiFlags & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // check for sound playing stuff
    if (b->ubSoundSchemeID) {
      if (&b->Area == MSYS_PrevRegion && !gpAnchoredButton) {
        if (b->uiFlags & BUTTON_ENABLED) {
          PlayButtonSound(iButtonID, BUTTON_SOUND_MOVED_ONTO);
        } else {
          PlayButtonSound(iButtonID, BUTTON_SOUND_DISABLED_MOVED_ONTO);
        }
      }
    }
  } else {
    // Check if we should play a sound
    if (b->ubSoundSchemeID) {
      if (b->uiFlags & BUTTON_ENABLED) {
        if (&b->Area == MSYS_PrevRegion && !gpAnchoredButton) {
          PlayButtonSound(iButtonID, BUTTON_SOUND_MOVED_OFF_OF);
        }
      } else {
        PlayButtonSound(iButtonID, BUTTON_SOUND_DISABLED_MOVED_OFF_OF);
      }
    }
  }

  // ATE: New stuff for toggle buttons that work with new Win95 paridigm
  if ((b->uiFlags & BUTTON_NEWTOGGLE)) {
    if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
      if (b->ubToggleButtonActivated) {
        b->ubToggleButtonActivated = FALSE;

        if (!b->ubToggleButtonOldState) {
          b->uiFlags &= (~BUTTON_CLICKED_ON);
        } else {
          b->uiFlags |= BUTTON_CLICKED_ON;
        }
      }
    }
  }

  // If this button is enabled and there is a callback function associated with it,
  // call the callback function.
  if ((b->uiFlags & BUTTON_ENABLED) && (b->uiFlags & BUTTON_MOVE_CALLBACK))
    (b->MoveCallback)(b, reason);
}

//=============================================================================
//	QuickButtonCallbackMButn
//
//	Dispatches all button callbacks for button presses. This function is
//	called by the Mouse System. *DO NOT CALL DIRECTLY*
//
void QuickButtonCallbackMButn(struct MOUSE_REGION *reg, int32_t reason) {
  GUI_BUTTON *b;
  int32_t iButtonID;
  BOOLEAN MouseBtnDown;
  BOOLEAN StateBefore, StateAfter;

  Assert(reg != NULL);

  iButtonID = MSYS_GetRegionUserData(reg, 0);

  sprintf(str,
          "QuickButtonCallbackMButn: Mouse Region #%d (%d,%d to %d,%d) has invalid buttonID %d",
          reg->IDNumber, reg->RegionTopLeftX, reg->RegionTopLeftY, reg->RegionBottomRightX,
          reg->RegionBottomRightY, iButtonID);

  AssertMsg(iButtonID >= 0, str);
  AssertMsg(iButtonID < MAX_BUTTONS, str);

  b = ButtonList[iButtonID];

  AssertMsg(b != NULL, str);

  if (!b) return;

  if (reason & (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_RBUTTON_DWN))
    MouseBtnDown = TRUE;
  else
    MouseBtnDown = FALSE;

  StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? (TRUE) : (FALSE);

  // ATE: New stuff for toggle buttons that work with new Win95 paridigm
  if (b->uiFlags & BUTTON_NEWTOGGLE && b->uiFlags & BUTTON_ENABLED) {
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
      if (!b->ubToggleButtonActivated) {
        if (!(b->uiFlags & BUTTON_CLICKED_ON)) {
          b->ubToggleButtonOldState = FALSE;
          b->uiFlags |= BUTTON_CLICKED_ON;
        } else {
          b->ubToggleButtonOldState = TRUE;
          b->uiFlags &= (~BUTTON_CLICKED_ON);
        }
        b->ubToggleButtonActivated = TRUE;
      }
    } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      b->ubToggleButtonActivated = FALSE;
    }
  }

  // Kris:
  // Set the anchored button incase the user moves mouse off region while still holding
  // down the button, but only if the button is up.  In Win95, buttons that are already
  // down, and anchored never change state, unless you release the mouse in the button area.

  if (b->MoveCallback == DEFAULT_MOVE_CALLBACK && b->uiFlags & BUTTON_ENABLED) {
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
      gpAnchoredButton = b;
      gfAnchoredState = StateBefore;
      b->uiFlags |= BUTTON_CLICKED_ON;
    } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP && b->uiFlags & BUTTON_NO_TOGGLE) {
      b->uiFlags &= (~BUTTON_CLICKED_ON);
    }
  } else if (b->uiFlags & BUTTON_CHECKBOX) {
    if (reason &
        MSYS_CALLBACK_REASON_LBUTTON_DWN) {  // the check box button gets anchored, though it
                                             // doesn't actually use the anchoring move callback.
      // The effect is different, we don't want to toggle the button state, but we do want to anchor
      // this button so that we don't effect any other buttons while we move the mouse around in
      // anchor mode.
      gpAnchoredButton = b;
      gfAnchoredState = StateBefore;

      // Trick the before state of the button to be different so the sound will play properly as
      // checkbox buttons are processed differently.
      StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? FALSE : TRUE;
      StateAfter = !StateBefore;
    } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      b->uiFlags ^= BUTTON_CLICKED_ON;  // toggle the checkbox state upon release inside button
                                        // area.
      // Trick the before state of the button to be different so the sound will play properly as
      // checkbox buttons are processed differently.
      StateBefore = (b->uiFlags & BUTTON_CLICKED_ON) ? FALSE : TRUE;
      StateAfter = !StateBefore;
    }
  }

  // Should we play a sound if clicked on while disabled?
  if (b->ubSoundSchemeID && !(b->uiFlags & BUTTON_ENABLED) && MouseBtnDown) {
    PlayButtonSound(iButtonID, BUTTON_SOUND_DISABLED_CLICK);
  }

  // If this button is disabled, and no callbacks allowed when disabled
  // callback
  if (!(b->uiFlags & BUTTON_ENABLED) && !(b->uiFlags & BUTTON_ALLOW_DISABLED_CALLBACK)) return;

  // Button not enabled but allowed to use callback, then do that!
  if (!(b->uiFlags & BUTTON_ENABLED) && (b->uiFlags & BUTTON_ALLOW_DISABLED_CALLBACK)) {
    if (b->uiFlags & BUTTON_CLICK_CALLBACK) {
      (b->ClickCallback)(b, reason | BUTTON_DISABLED_CALLBACK);
    }
    return;
  }

  // If there is a callback function with this button, call it
  if (b->uiFlags & BUTTON_CLICK_CALLBACK) {
    // Kris:  January 6, 1998
    // Added these checks to avoid a case where it was possible to process a leftbuttonup message
    // when the button wasn't anchored, and should have been.
    gfDelayButtonDeletion = TRUE;
    if (!(reason & MSYS_CALLBACK_REASON_LBUTTON_UP) || b->MoveCallback != DEFAULT_MOVE_CALLBACK ||
        (b->MoveCallback == DEFAULT_MOVE_CALLBACK && gpPrevAnchoredButton == b))
      (b->ClickCallback)(b, reason);
    gfDelayButtonDeletion = FALSE;
  } else if ((reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) && !(b->uiFlags & BUTTON_IGNORE_CLICKS)) {
    // Otherwise, do default action with this button.
    b->uiFlags ^= BUTTON_CLICKED_ON;
  }

  if (b->uiFlags & BUTTON_CHECKBOX) {
    StateAfter = (b->uiFlags & BUTTON_CLICKED_ON) ? (TRUE) : (FALSE);
  }

  // Play sounds for this enabled button (disabled sounds have already been done)
  if (b->ubSoundSchemeID && b->uiFlags & BUTTON_ENABLED) {
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      if (b->ubSoundSchemeID && StateBefore && !StateAfter) {
        PlayButtonSound(iButtonID, BUTTON_SOUND_CLICKED_OFF);
      }
    } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
      if (b->ubSoundSchemeID && !StateBefore && StateAfter) {
        PlayButtonSound(iButtonID, BUTTON_SOUND_CLICKED_ON);
      }
    }
  }

  if (StateBefore != StateAfter) {
    InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX,
                     b->Area.RegionBottomRightY);
  }

  if (gfPendingButtonDeletion) {
    RemoveButtonsMarkedForDeletion();
  }
}

void RenderButtons(void) {
  int32_t iButtonID;
  BOOLEAN fOldButtonDown, fOldEnabled;
  GUI_BUTTON *b;

  SaveFontSettings();
  for (iButtonID = 0; iButtonID < MAX_BUTTONS; iButtonID++) {
    // If the button exists, and it's not owned by another object, draw it
    // Kris:  and make sure that the button isn't hidden.
    b = ButtonList[iButtonID];
    if (b && b->Area.uiFlags & MSYS_REGION_ENABLED) {
      // Check for buttonchanged status
      fOldButtonDown = (BOOLEAN)(b->uiFlags & BUTTON_CLICKED_ON);

      if (fOldButtonDown != (b->uiOldFlags & BUTTON_CLICKED_ON)) {
        // Something is different, set dirty!
        b->uiFlags |= BUTTON_DIRTY;
      }

      // Check for button dirty flags
      fOldEnabled = (BOOLEAN)(b->uiFlags & BUTTON_ENABLED);

      if (fOldEnabled != (b->uiOldFlags & BUTTON_ENABLED)) {
        // Something is different, set dirty!
        b->uiFlags |= BUTTON_DIRTY;
      }

      // If we ABSOLUTELY want to render every frame....
      if (b->uiFlags & BUTTON_SAVEBACKGROUND) {
        b->uiFlags |= BUTTON_DIRTY;
      }

      // Set old flags
      b->uiOldFlags = b->uiFlags;

      if (b->uiFlags & BUTTON_FORCE_UNDIRTY) {
        b->uiFlags &= ~(BUTTON_DIRTY);
        b->uiFlags &= ~(BUTTON_FORCE_UNDIRTY);
      }

      // Check if we need to update!
      if (b->uiFlags & BUTTON_DIRTY) {
        // Turn off dirty flag
        b->uiFlags &= (~BUTTON_DIRTY);
        DrawButtonFromPtr(b);

        InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX,
                         b->Area.RegionBottomRightY);
      }
    }
  }

  // check if we want to render 1 frame later?
  if ((fPausedMarkButtonsDirtyFlag == TRUE) && (fDisableHelpTextRestoreFlag == FALSE)) {
    fPausedMarkButtonsDirtyFlag = FALSE;
    MarkButtonsDirty();
  }

  RestoreFontSettings();
}

//*****************************************************************************
// MarkAButtonDirty
//
void MarkAButtonDirty(int32_t iButtonNum) {
  // surgical dirtying -> marks a user specified button dirty, without dirty the whole lot of them

  // If the button exists, and it's not owned by another object, draw it
  if (ButtonList[iButtonNum]) {
    // Turn on dirty flag
    ButtonList[iButtonNum]->uiFlags |= BUTTON_DIRTY;
  }
}

//=============================================================================
//	MarkButtonsDirty
//
void MarkButtonsDirty(void) {
  int32_t x;
  for (x = 0; x < MAX_BUTTONS; x++) {
    // If the button exists, and it's not owned by another object, draw it
    if (ButtonList[x]) {
      // Turn on dirty flag
      ButtonList[x]->uiFlags |= BUTTON_DIRTY;
    }
  }
}

void UnMarkButtonDirty(int32_t iButtonIndex) {
  if (ButtonList[iButtonIndex]) {
    ButtonList[iButtonIndex]->uiFlags &= ~(BUTTON_DIRTY);
  }
}

void UnmarkButtonsDirty(void) {
  int32_t x;
  for (x = 0; x < MAX_BUTTONS; x++) {
    // If the button exists, and it's not owned by another object, draw it
    if (ButtonList[x]) {
      UnMarkButtonDirty(x);
    }
  }
}

void ForceButtonUnDirty(int32_t iButtonIndex) {
  ButtonList[iButtonIndex]->uiFlags &= ~(BUTTON_DIRTY);
  ButtonList[iButtonIndex]->uiFlags |= BUTTON_FORCE_UNDIRTY;
}

//=============================================================================
// PauseMarkButtonsDirty
//

void PausedMarkButtonsDirty(void) {
  // set flag for frame after the next rendering of buttons
  fPausedMarkButtonsDirtyFlag = TRUE;

  return;
}

//=============================================================================
//	DrawButton
//
//	Draws a single button on the screen.
//
BOOLEAN DrawButton(int32_t iButtonID) {
  // Fail if button handle out of range
  if (iButtonID < 0 || iButtonID > MAX_BUTTONS) return FALSE;

  // Fail if button handle is invalid
  if (!ButtonList[iButtonID]) return FALSE;

  if (ButtonList[iButtonID]->string) SaveFontSettings();
  // Draw this button
  if (ButtonList[iButtonID]->Area.uiFlags & MSYS_REGION_ENABLED) {
    DrawButtonFromPtr(ButtonList[iButtonID]);
  }

  if (ButtonList[iButtonID]->string) RestoreFontSettings();
  return TRUE;
}

//=============================================================================
//	DrawButtonFromPtr
//
//	Given a pointer to a GUI_BUTTON structure, draws the button on the
//	screen.
//
void DrawButtonFromPtr(GUI_BUTTON *b) {
  Assert(b);
  // Draw the appropriate button according to button type
  gbDisabledButtonStyle = DISABLED_STYLE_NONE;
  switch (b->uiFlags & BUTTON_TYPES) {
    case BUTTON_QUICK:
      DrawQuickButton(b);
      break;
    case BUTTON_GENERIC:
      DrawGenericButton(b);
      break;
    case BUTTON_HOT_SPOT:
      if (b->uiFlags & BUTTON_NO_TOGGLE) b->uiFlags &= (~BUTTON_CLICKED_ON);
      return;  // hotspots don't have text, but if you want to, change this to a break!
    case BUTTON_CHECKBOX:
      DrawCheckBoxButton(b);
      break;
  }
  // If button has an icon, overlay it on current button.
  if (b->iIconID != -1) DrawIconOnButton(b);
  // If button has text, draw it now
  if (b->string) DrawTextOnButton(b);
  // If the button is disabled, and a style has been calculated, then
  // draw the style last.
  switch (gbDisabledButtonStyle) {
    case DISABLED_STYLE_HATCHED:
      DrawHatchOnButton(b);
      break;
    case DISABLED_STYLE_SHADED:
      DrawShadeOnButton(b);
      break;
  }
  if (b->bDefaultStatus) {
    DrawDefaultOnButton(b);
  }
}

//=============================================================================
//	DrawQuickButton
//
//	Draws a QuickButton type button on the screen.
//
void DrawQuickButton(GUI_BUTTON *b) {
  int32_t UseImage;
  UseImage = 0;
  // Is button Enabled, or diabled but no "Grayed" image associated with this QuickButton?
  if (b->uiFlags & BUTTON_ENABLED) {
    // Is the button's state ON?
    if (b->uiFlags & BUTTON_CLICKED_ON) {
      // Is the mouse over this area, and we have a hilite image?
      if ((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && gfRenderHilights &&
          (ButtonPictures[b->ImageNum].OnHilite != -1))
        UseImage = ButtonPictures[b->ImageNum].OnHilite;  // Use On-Hilite image
      else if (ButtonPictures[b->ImageNum].OnNormal != -1)
        UseImage = ButtonPictures[b->ImageNum].OnNormal;  // Use On-Normal image
    } else {
      // Is the mouse over the button, and do we have hilite image?
      if ((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && gfRenderHilights &&
          (ButtonPictures[b->ImageNum].OffHilite != -1))
        UseImage = ButtonPictures[b->ImageNum].OffHilite;  // Use Off-Hilite image
      else if (ButtonPictures[b->ImageNum].OffNormal != -1)
        UseImage = ButtonPictures[b->ImageNum].OffNormal;  // Use Off-Normal image
    }
  } else if (ButtonPictures[b->ImageNum].Grayed !=
             -1) {  // Button is diabled so use the "Grayed-out" image
    UseImage = ButtonPictures[b->ImageNum].Grayed;
  } else {
    UseImage = ButtonPictures[b->ImageNum].OffNormal;
    switch (b->bDisabledStyle) {
      case DISABLED_STYLE_DEFAULT:
        gbDisabledButtonStyle = b->string ? DISABLED_STYLE_SHADED : DISABLED_STYLE_HATCHED;
        break;
      case DISABLED_STYLE_HATCHED:
      case DISABLED_STYLE_SHADED:
        gbDisabledButtonStyle = b->bDisabledStyle;
        break;
    }
  }

  // Display the button image
  BltVideoObject(vsButtonDest, ButtonPictures[b->ImageNum].vobj, (uint16_t)UseImage, b->XLoc,
                 b->YLoc);
}

void DrawHatchOnButton(GUI_BUTTON *b) {
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;
  SGPRect ClipRect;
  ClipRect.iLeft = b->Area.RegionTopLeftX;
  ClipRect.iRight = b->Area.RegionBottomRightX - 1;
  ClipRect.iTop = b->Area.RegionTopLeftY;
  ClipRect.iBottom = b->Area.RegionBottomRightY - 1;
  pDestBuf = LockVSurface(vsButtonDest, &uiDestPitchBYTES);
  Blt16BPPBufferHatchRect((uint16_t *)pDestBuf, uiDestPitchBYTES, &ClipRect);
  JSurface_Unlock(vsButtonDest);
}

void DrawShadeOnButton(GUI_BUTTON *b) {
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;
  SGPRect ClipRect;
  ClipRect.iLeft = b->Area.RegionTopLeftX;
  ClipRect.iRight = b->Area.RegionBottomRightX - 1;
  ClipRect.iTop = b->Area.RegionTopLeftY;
  ClipRect.iBottom = b->Area.RegionBottomRightY - 1;
  pDestBuf = LockVSurface(vsButtonDest, &uiDestPitchBYTES);
  Blt16BPPBufferShadowRect((uint16_t *)pDestBuf, uiDestPitchBYTES, &ClipRect);
  JSurface_Unlock(vsButtonDest);
}

void DrawDefaultOnButton(GUI_BUTTON *b) {
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;
  pDestBuf = LockVSurface(vsButtonDest, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  if (b->bDefaultStatus == DEFAULT_STATUS_DARKBORDER ||
      b->bDefaultStatus == DEFAULT_STATUS_WINDOWS95) {
    // left (one thick)
    LineDraw(TRUE, b->Area.RegionTopLeftX - 1, b->Area.RegionTopLeftY - 1,
             b->Area.RegionTopLeftX - 1, b->Area.RegionBottomRightY + 1, 0, pDestBuf);
    // top (one thick)
    LineDraw(TRUE, b->Area.RegionTopLeftX - 1, b->Area.RegionTopLeftY - 1,
             b->Area.RegionBottomRightX + 1, b->Area.RegionTopLeftY - 1, 0, pDestBuf);
    // right (two thick)
    LineDraw(TRUE, b->Area.RegionBottomRightX, b->Area.RegionTopLeftY - 1,
             b->Area.RegionBottomRightX, b->Area.RegionBottomRightY + 1, 0, pDestBuf);
    LineDraw(TRUE, b->Area.RegionBottomRightX + 1, b->Area.RegionTopLeftY - 1,
             b->Area.RegionBottomRightX + 1, b->Area.RegionBottomRightY + 1, 0, pDestBuf);
    // bottom (two thick)
    LineDraw(TRUE, b->Area.RegionTopLeftX - 1, b->Area.RegionBottomRightY,
             b->Area.RegionBottomRightX + 1, b->Area.RegionBottomRightY, 0, pDestBuf);
    LineDraw(TRUE, b->Area.RegionTopLeftX - 1, b->Area.RegionBottomRightY + 1,
             b->Area.RegionBottomRightX + 1, b->Area.RegionBottomRightY + 1, 0, pDestBuf);
    InvalidateRegion(b->Area.RegionTopLeftX - 1, b->Area.RegionTopLeftY - 1,
                     b->Area.RegionBottomRightX + 1, b->Area.RegionBottomRightY + 1);
  }
  if (b->bDefaultStatus == DEFAULT_STATUS_DOTTEDINTERIOR ||
      b->bDefaultStatus == DEFAULT_STATUS_WINDOWS95) {  // Draw an internal dotted rectangle.
  }
  JSurface_Unlock(vsButtonDest);
}

void DrawCheckBoxButtonOn(int32_t iButtonID) {
  GUI_BUTTON *b;
  BOOLEAN fLeftButtonState = gfLeftButtonState;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  gfLeftButtonState = TRUE;
  b->Area.uiFlags |= MSYS_MOUSE_IN_AREA;

  DrawButton(iButtonID);

  gfLeftButtonState = fLeftButtonState;
}

void DrawCheckBoxButtonOff(int32_t iButtonID) {
  GUI_BUTTON *b;
  BOOLEAN fLeftButtonState = gfLeftButtonState;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  gfLeftButtonState = FALSE;
  b->Area.uiFlags |= MSYS_MOUSE_IN_AREA;

  DrawButton(iButtonID);

  gfLeftButtonState = fLeftButtonState;
}

void DrawCheckBoxButton(GUI_BUTTON *b) {
  int32_t UseImage;

  UseImage = 0;
  // Is button Enabled, or diabled but no "Grayed" image associated with this QuickButton?
  if (b->uiFlags & BUTTON_ENABLED) {
    // Is the button's state ON?
    if (b->uiFlags & BUTTON_CLICKED_ON) {
      // Is the mouse over this area, and we have a hilite image?
      if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA && gfRenderHilights && gfLeftButtonState &&
          ButtonPictures[b->ImageNum].OnHilite != -1)
        UseImage = ButtonPictures[b->ImageNum].OnHilite;  // Use On-Hilite image
      else if (ButtonPictures[b->ImageNum].OnNormal != -1)
        UseImage = ButtonPictures[b->ImageNum].OnNormal;  // Use On-Normal image
    } else {
      // Is the mouse over the button, and do we have hilite image?
      if (b->Area.uiFlags & MSYS_MOUSE_IN_AREA && gfRenderHilights && gfLeftButtonState &&
          ButtonPictures[b->ImageNum].OffHilite != -1)
        UseImage = ButtonPictures[b->ImageNum].OffHilite;  // Use Off-Hilite image
      else if (ButtonPictures[b->ImageNum].OffNormal != -1)
        UseImage = ButtonPictures[b->ImageNum].OffNormal;  // Use Off-Normal image
    }
  } else if (ButtonPictures[b->ImageNum].Grayed !=
             -1) {  // Button is disabled so use the "Grayed-out" image
    UseImage = ButtonPictures[b->ImageNum].Grayed;
  } else  // use the disabled style
  {
    if (b->uiFlags & BUTTON_CLICKED_ON)
      UseImage = ButtonPictures[b->ImageNum].OnHilite;
    else
      UseImage = ButtonPictures[b->ImageNum].OffHilite;
    switch (b->bDisabledStyle) {
      case DISABLED_STYLE_DEFAULT:
        gbDisabledButtonStyle = DISABLED_STYLE_HATCHED;
        break;
      case DISABLED_STYLE_HATCHED:
      case DISABLED_STYLE_SHADED:
        gbDisabledButtonStyle = b->bDisabledStyle;
        break;
    }
  }

  // Display the button image
  BltVideoObject(vsButtonDest, ButtonPictures[b->ImageNum].vobj, (uint16_t)UseImage, b->XLoc,
                 b->YLoc);
}

void DrawIconOnButton(GUI_BUTTON *b) {
  int32_t xp, yp, width, height, IconX, IconY;
  int32_t IconW, IconH;
  SGPRect NewClip, OldClip;
  ETRLEObject *pTrav;
  struct VObject *hvObject;

  // If there's an actual icon on this button, try to show it.
  if (b->iIconID >= 0) {
    // Get width and height of button area
    width = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
    height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

    // Compute viewable area (inside borders)
    NewClip.iLeft = b->XLoc + 3;
    NewClip.iRight = b->XLoc + width - 3;
    NewClip.iTop = b->YLoc + 2;
    NewClip.iBottom = b->YLoc + height - 2;

    // Get Icon's blit start coordinates
    IconX = NewClip.iLeft;
    IconY = NewClip.iTop;

    // Get current clip area
    GetClippingRect(&OldClip);

    // Clip button's viewable area coords to screen
    if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

    // Is button right off the right side of the screen?
    if (NewClip.iLeft > OldClip.iRight) return;

    if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

    // Is button completely off the left side of the screen?
    if (NewClip.iRight < OldClip.iLeft) return;

    if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

    // Are we right off the bottom of the screen?
    if (NewClip.iTop > OldClip.iBottom) return;

    if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

    // Are we off the top?
    if (NewClip.iBottom < OldClip.iTop) return;

    // Did we clip the viewable area out of existance?
    if ((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop)) return;

    // Get the width and height of the icon itself
    if (b->uiFlags & BUTTON_GENERIC)
      pTrav = &(GenericButtonIcons[b->iIconID]->pETRLEObject[b->usIconIndex]);
    else {
      GetVideoObject(&hvObject, b->iIconID);
      pTrav = &(hvObject->pETRLEObject[b->usIconIndex]);
    }
    IconH = (uint32_t)(pTrav->usHeight + pTrav->sOffsetY);
    IconW = (uint32_t)(pTrav->usWidth + pTrav->sOffsetX);

    // Compute coordinates for centering the icon on the button or
    // use the offset system.
    if (b->bIconXOffset == -1)
      xp = (((width - 6) - IconW) / 2) + IconX;
    else
      xp = b->Area.RegionTopLeftX + b->bIconXOffset;
    if (b->bIconYOffset == -1)
      yp = (((height - 4) - IconH) / 2) + IconY;
    else
      yp = b->Area.RegionTopLeftY + b->bIconYOffset;

    // Was the button clicked on? if so, move the image slightly for the illusion
    // that the image moved into the screen.
    if (b->uiFlags & BUTTON_CLICKED_ON && b->fShiftImage) {
      xp++;
      yp++;
    }

    // Set the clipping rectangle to the viewable area of the button
    SetClippingRect(&NewClip);
    // Blit the icon
    if (b->uiFlags & BUTTON_GENERIC)
      BltVideoObject(vsButtonDest, GenericButtonIcons[b->iIconID], b->usIconIndex, (int16_t)xp,
                     (int16_t)yp);
    else
      BltVideoObject(vsButtonDest, hvObject, b->usIconIndex, (int16_t)xp, (int16_t)yp);
    // Restore previous clip region
    SetClippingRect(&OldClip);
  }
}

// If a button has text attached to it, then it'll draw it last.
void DrawTextOnButton(GUI_BUTTON *b) {
  int32_t xp, yp, width, height, TextX, TextY;
  SGPRect NewClip, OldClip;
  int16_t sForeColor;

  // If this button actually has a string to print
  if (b->string) {
    // Get the width and height of this button
    width = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
    height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;

    // Compute the viewable area on this button
    NewClip.iLeft = b->XLoc + 3;
    NewClip.iRight = b->XLoc + width - 3;
    NewClip.iTop = b->YLoc + 2;
    NewClip.iBottom = b->YLoc + height - 2;

    // Get the starting coordinates to print
    TextX = NewClip.iLeft;
    TextY = NewClip.iTop;

    // Get the current clipping area
    GetClippingRect(&OldClip);

    // Clip the button's viewable area to the screen
    if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

    // Are we off hte right side?
    if (NewClip.iLeft > OldClip.iRight) return;

    if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

    // Are we off the left side?
    if (NewClip.iRight < OldClip.iLeft) return;

    if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

    // Are we off the bottom of the screen?
    if (NewClip.iTop > OldClip.iBottom) return;

    if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

    // Are we off the top?
    if (NewClip.iBottom < OldClip.iTop) return;

    // Did we clip the viewable area out of existance?
    if ((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop)) return;

    // Set the font printing settings to the buttons viewable area
    SetFontDestBuffer(vsButtonDest, NewClip.iLeft, NewClip.iTop, NewClip.iRight, NewClip.iBottom,
                      FALSE);

    // Compute the coordinates to center the text
    if (b->bTextYOffset == -1)
      yp = (((height)-GetFontHeight(b->usFont)) / 2) + TextY - 1;
    else
      yp = b->Area.RegionTopLeftY + b->bTextYOffset;
    if (b->bTextXOffset == -1) {
      switch (b->bJustification) {
        case BUTTON_TEXT_LEFT:
          xp = TextX + 3;
          break;
        case BUTTON_TEXT_RIGHT:
          xp = NewClip.iRight - StringPixLength(b->string, b->usFont) - 3;
          break;
        case BUTTON_TEXT_CENTER:
        default:
          xp = (((width - 6) - StringPixLength(b->string, b->usFont)) / 2) + TextX;
          break;
      }
    } else
      xp = b->Area.RegionTopLeftX + b->bTextXOffset;

    // Set the printing font to the button text font
    SetFont(b->usFont);

    // print the text
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground((uint8_t)b->sForeColor);
    sForeColor = b->sForeColor;
    if (b->sShadowColor != -1) SetFontShadow((uint8_t)b->sShadowColor);
    // Override the colors if necessary.
    if (b->uiFlags & BUTTON_ENABLED && b->Area.uiFlags & MSYS_MOUSE_IN_AREA &&
        b->sForeColorHilited != -1) {
      SetFontForeground((uint8_t)b->sForeColorHilited);
      sForeColor = b->sForeColorHilited;
    } else if (b->uiFlags & BUTTON_CLICKED_ON && b->sForeColorDown != -1) {
      SetFontForeground((uint8_t)b->sForeColorDown);
      sForeColor = b->sForeColorDown;
    }
    if (b->uiFlags & BUTTON_ENABLED && b->Area.uiFlags & MSYS_MOUSE_IN_AREA &&
        b->sShadowColorHilited != -1) {
      SetFontShadow((uint8_t)b->sShadowColorHilited);
    } else if (b->uiFlags & BUTTON_CLICKED_ON && b->sShadowColorDown != -1) {
      SetFontShadow((uint8_t)b->sShadowColorDown);
    }
    if (b->uiFlags & BUTTON_CLICKED_ON &&
        b->fShiftText) {  // Was the button clicked on? if so, move the text slightly for the
                          // illusion
      // that the text moved into the screen.
      xp++;
      yp++;
    }
    if (b->sWrappedWidth != -1) {
      uint8_t bJustified = 0;
      switch (b->bJustification) {
        case BUTTON_TEXT_LEFT:
          bJustified = LEFT_JUSTIFIED;
          break;
        case BUTTON_TEXT_RIGHT:
          bJustified = RIGHT_JUSTIFIED;
          break;
        case BUTTON_TEXT_CENTER:
          bJustified = CENTER_JUSTIFIED;
          break;
        default:
          Assert(0);
          break;
      }
      if (b->bTextXOffset == -1) {
        // Kris:
        // There needs to be recalculation of the start positions based on the
        // justification and the width specified wrapped width.  I was drawing a
        // double lined word on the right side of the button to find it drawing way
        // over to the left.  I've added the necessary code for the right and center
        // justification.
        yp = b->Area.RegionTopLeftY + 2;

        switch (b->bJustification) {
          case BUTTON_TEXT_RIGHT:
            xp = b->Area.RegionBottomRightX - 3 - b->sWrappedWidth;

            if (b->fShiftText && b->uiFlags & BUTTON_CLICKED_ON) xp++, yp++;
            break;
          case BUTTON_TEXT_CENTER:
            xp = b->Area.RegionTopLeftX + 3 + b->sWrappedWidth / 2;

            if (b->fShiftText && b->uiFlags & BUTTON_CLICKED_ON) xp++, yp++;
            break;
        }
      }
      yp += b->bTextYSubOffSet;
      xp += b->bTextXSubOffSet;
      DisplayWrappedString((uint16_t)xp, (uint16_t)yp, b->sWrappedWidth, 1, b->usFont,
                           (uint8_t)sForeColor, b->string, FONT_MCOLOR_BLACK, FALSE, bJustified);
    } else {
      yp += b->bTextYSubOffSet;
      xp += b->bTextXSubOffSet;
      mprintf(xp, yp, b->string);
    }
    // Restore the old text printing settings
  }
}

//=============================================================================
//	DrawGenericButton
//
//	This function is called by the DrawIconicButton and DrawTextButton
//	routines to draw the borders and background of the buttons.
//
void DrawGenericButton(GUI_BUTTON *b) {
  int32_t NumChunksWide, NumChunksHigh, cx, cy, width, height, hremain, wremain;
  int32_t q, ImgNum, ox, oy;
  int32_t iBorderHeight, iBorderWidth;
  struct VObject *BPic;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  SGPRect ClipRect;

  // Select the graphics to use depending on the current state of the button
  if (b->uiFlags & BUTTON_ENABLED) {
    if (!(b->uiFlags & BUTTON_ENABLED) && (GenericButtonGrayed[b->ImageNum] == NULL))
      BPic = GenericButtonOffNormal[b->ImageNum];
    else if (b->uiFlags & BUTTON_CLICKED_ON) {
      if ((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && (GenericButtonOnHilite[b->ImageNum] != NULL) &&
          gfRenderHilights)
        BPic = GenericButtonOnHilite[b->ImageNum];
      else
        BPic = GenericButtonOnNormal[b->ImageNum];
    } else {
      if ((b->Area.uiFlags & MSYS_MOUSE_IN_AREA) && (GenericButtonOffHilite[b->ImageNum] != NULL) &&
          gfRenderHilights)
        BPic = GenericButtonOffHilite[b->ImageNum];
      else
        BPic = GenericButtonOffNormal[b->ImageNum];
    }
  } else if (GenericButtonGrayed[b->ImageNum])
    BPic = GenericButtonGrayed[b->ImageNum];
  else {
    BPic = GenericButtonOffNormal[b->ImageNum];
    switch (b->bDisabledStyle) {
      case DISABLED_STYLE_DEFAULT:
        gbDisabledButtonStyle = b->string ? DISABLED_STYLE_SHADED : DISABLED_STYLE_HATCHED;
        break;
      case DISABLED_STYLE_HATCHED:
      case DISABLED_STYLE_SHADED:
        gbDisabledButtonStyle = b->bDisabledStyle;
        break;
    }
  }

  iBorderWidth = 3;
  iBorderHeight = 2;

  // DB - Added this to support more flexible sizing of border images
  // The 3x2 size was a bit limiting. JA2 should default to the original
  // size, unchanged

  // Compute the number of button "chunks" needed to be blitted
  width = b->Area.RegionBottomRightX - b->Area.RegionTopLeftX;
  height = b->Area.RegionBottomRightY - b->Area.RegionTopLeftY;
  NumChunksWide = width / iBorderWidth;
  NumChunksHigh = height / iBorderHeight;
  hremain = height % iBorderHeight;
  wremain = width % iBorderWidth;

  cx = (b->XLoc + ((NumChunksWide - 1) * iBorderWidth) + wremain);
  cy = (b->YLoc + ((NumChunksHigh - 1) * iBorderHeight) + hremain);

  // Fill the button's area with the button's background color
  ColorFillVSurfaceArea(vsButtonDest, b->Area.RegionTopLeftX, b->Area.RegionTopLeftY,
                        b->Area.RegionBottomRightX, b->Area.RegionBottomRightY,
                        GenericButtonFillColors[b->ImageNum]);

  // If there is a background image, fill the button's area with it
  if (GenericButtonBackground[b->ImageNum] != NULL) {
    ox = oy = 0;
    // if the button was clicked on, adjust the background image so that we get
    // the illusion that it is sunk into the screen.
    if (b->uiFlags & BUTTON_CLICKED_ON) ox = oy = 1;

    // Fill the area with the image, tilling it if need be.
    ImageFillVideoSurfaceArea(vsButtonDest, b->Area.RegionTopLeftX + ox,
                              b->Area.RegionTopLeftY + oy, b->Area.RegionBottomRightX,
                              b->Area.RegionBottomRightY, GenericButtonBackground[b->ImageNum],
                              GenericButtonBackgroundIndex[b->ImageNum],
                              GenericButtonOffsetX[b->ImageNum], GenericButtonOffsetY[b->ImageNum]);
  }

  // Lock the dest buffer
  pDestBuf = LockVSurface(vsButtonDest, &uiDestPitchBYTES);

  GetClippingRect(&ClipRect);

  // Draw the button's borders and corners (horizontally)
  for (q = 0; q < NumChunksWide; q++) {
    if (q == 0)
      ImgNum = 0;
    else
      ImgNum = 1;

    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES, BPic,
                                            (int32_t)(b->XLoc + (q * iBorderWidth)),
                                            (int32_t)b->YLoc, (uint16_t)ImgNum, &ClipRect);

    if (q == 0)
      ImgNum = 5;
    else
      ImgNum = 6;

    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES, BPic,
                                            (int32_t)(b->XLoc + (q * iBorderWidth)), cy,
                                            (uint16_t)ImgNum, &ClipRect);
  }
  // Blit the right side corners
  Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES, BPic, cx,
                                          (int32_t)b->YLoc, 2, &ClipRect);

  Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES, BPic, cx, cy, 7,
                                          &ClipRect);
  // Draw the vertical members of the button's borders
  NumChunksHigh--;

  if (hremain != 0) {
    q = NumChunksHigh;
    Blt8BPPDataTo16BPPBufferTransparentClip(
        (uint16_t *)pDestBuf, uiDestPitchBYTES, BPic, (int32_t)b->XLoc,
        (int32_t)(b->YLoc + (q * iBorderHeight) - (iBorderHeight - hremain)), 3, &ClipRect);
    Blt8BPPDataTo16BPPBufferTransparentClip(
        (uint16_t *)pDestBuf, uiDestPitchBYTES, BPic, cx,
        (int32_t)(b->YLoc + (q * iBorderHeight) - (iBorderHeight - hremain)), 4, &ClipRect);
  }

  for (q = 1; q < NumChunksHigh; q++) {
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES, BPic,
                                            (int32_t)b->XLoc,
                                            (int32_t)(b->YLoc + (q * iBorderHeight)), 3, &ClipRect);
    Blt8BPPDataTo16BPPBufferTransparentClip((uint16_t *)pDestBuf, uiDestPitchBYTES, BPic, cx,
                                            (int32_t)(b->YLoc + (q * iBorderHeight)), 4, &ClipRect);
  }

  // Unlock buffer
  JSurface_Unlock(vsButtonDest);
}

//=======================================================================================================
// Dialog box code
//

//=======================================================================================================
//=======================================================================================================
//
//	Very preliminary stuff follows
//
//=======================================================================================================
//=======================================================================================================

typedef struct _CreateDlgInfo {
  int32_t iFlags;  // Holds the creation flags

  int32_t iPosX;  // Screen position of dialog box
  int32_t iPosY;
  int32_t iWidth;  // Dimensions of dialog box (if needed)
  int32_t iHeight;

  int32_t iAreaWidth;    // Dimensions of area the dialog box will be
  int32_t iAreaHeight;   // placed (for auto-sizing and auto-placement
  int32_t iAreaOffsetX;  // only)
  int32_t iAreaOffsetY;

  uint16_t *zDlgText;   // Text to be displayed (if any)
  int32_t iTextFont;    // Font to be used for text (if any)
  uint16_t usTextCols;  // Font colors (for mono fonts only)

  int32_t iTextAreaX;  // Area in dialog box where text is to be
  int32_t iTextAreaY;  // put (for non-auto placed text)
  int32_t iTextAreaWidth;
  int32_t iTextAreaHeight;

  struct VObject *hBackImg;  // Background pic for dialog box (if any)
  int32_t iBackImgIndex;     // Sub-image index to use for image
  int32_t iBackOffsetX;      // Offset on dialog box where to put image
  int32_t iBackOffsetY;

  struct VObject *hIconImg;  // Icon image pic and index.
  int32_t iIconImgIndex;
  int32_t iIconPosX;
  int32_t iIconPosY;

  int32_t iBtnTypes;

  int32_t iOkPosX;  // Ok button info
  int32_t iOkPosY;
  int32_t iOkWidth;
  int32_t iOkHeight;
  int32_t iOkImg;

  int32_t iCnclPosX;  // Cancel button info
  int32_t iCnclPosY;
  int32_t iCnclWidth;
  int32_t iCnclHeight;
  int32_t iCnclImg;
} CreateDlgInfo;

#define DLG_RESTRICT_MOUSE 1
#define DLG_OK_BUTTON 2
#define DLG_CANCEL_BUTTON 4
#define DLG_AUTOSIZE 8
#define DLG_RECREATE 16
#define DLG_AUTOPOSITION 32
#define DLG_TEXT_IN_AREA 64
#define DLG_USE_BKGRND_IMAGE 128
#define DLG_USE_BORDERS 256
#define DLG_USE_BTN_HOTSPOTS 512
#define DLG_USE_MONO_FONTS 1024
#define DLG_IS_ACTIVE 2048

#define DLG_MANUAL_RENDER 0
#define DLG_START_RENDER 1
#define DLG_STOP_RENDER 2
#define DLG_AUTO_RENDER 3

#define DLG_GET_STATUS 0
#define DLG_WAIT_FOR_RESPONSE 1

#define DLG_STATUS_NONE 0
#define DLG_STATUS_OK 1
#define DLG_STATUS_CANCEL 2
#define DLG_STATUS_PENDING 3

//------------------------------------------------------------------------------------------------------

int32_t CreateCheckBoxButton(int16_t x, int16_t y, char *filename, int16_t Priority,
                             GUI_CALLBACK ClickCallback) {
  GUI_BUTTON *b;
  int32_t ButPic, iButtonID;
  Assert(filename != NULL);
  Assert(strlen(filename));
  if ((ButPic = LoadButtonImage(filename, -1, 0, 1, 2, 3)) == -1) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateCheckBoxButton: Can't load button image");
    return (-1);
  }
  iButtonID = (int16_t)QuickCreateButton((uint32_t)ButPic, x, y, BUTTON_CHECKBOX, Priority,
                                         MSYS_NO_CALLBACK, ClickCallback);
  if (iButtonID == -1) {
    DbgMessage(TOPIC_BUTTON_HANDLER, DBG_LEVEL_0, "CreateCheckBoxButton: Can't create button");
    return (-1);
  }

  // change the flags so that it isn't a quick button anymore
  b = ButtonList[iButtonID];
  b->uiFlags &= (~BUTTON_QUICK);
  b->uiFlags |= (BUTTON_CHECKBOX | BUTTON_SELFDELETE_IMAGE);

  return (iButtonID);
}

// Added Oct17, 97 Carter - kind of mindless, but might as well have it
void MSYS_SetBtnUserData(int32_t iButtonNum, int32_t index, int32_t userdata) {
  GUI_BUTTON *b;
  b = ButtonList[iButtonNum];
  if (index < 0 || index > 3) return;
  b->UserData[index] = userdata;
}

int32_t MSYS_GetBtnUserData(GUI_BUTTON *b, int32_t index) {
  if (index < 0 || index > 3) return (0);

  return (b->UserData[index]);
}

// Generic Button Movement Callback to reset the mouse button if the mouse is no longer
// in the button region.
void BtnGenericMouseMoveButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  // If the button isn't the anchored button, then we don't want to modify the button state.
  if (btn != gpAnchoredButton) return;
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    if (!gfAnchoredState) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);
      if (btn->ubSoundSchemeID) {
        PlayButtonSound(btn->IDNum, BUTTON_SOUND_CLICKED_OFF);
      }
    }
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    if (btn->ubSoundSchemeID) {
      PlayButtonSound(btn->IDNum, BUTTON_SOUND_CLICKED_ON);
    }
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void ReleaseAnchorMode() {
  if (!gpAnchoredButton) return;

  if (gusMouseXPos < gpAnchoredButton->Area.RegionTopLeftX ||
      gusMouseXPos > gpAnchoredButton->Area.RegionBottomRightX ||
      gusMouseYPos < gpAnchoredButton->Area.RegionTopLeftY ||
      gusMouseYPos > gpAnchoredButton->Area.RegionBottomRightY) {
    // released outside button area, so restore previous button state.
    if (gfAnchoredState)
      gpAnchoredButton->uiFlags |= BUTTON_CLICKED_ON;
    else
      gpAnchoredButton->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(gpAnchoredButton->Area.RegionTopLeftX, gpAnchoredButton->Area.RegionTopLeftY,
                     gpAnchoredButton->Area.RegionBottomRightX,
                     gpAnchoredButton->Area.RegionBottomRightY);
  }
  gpPrevAnchoredButton = gpAnchoredButton;
  gpAnchoredButton = NULL;
}

// Used to setup a dirtysaved region for buttons
BOOLEAN SetButtonSavedRect(int32_t iButton) {
  GUI_BUTTON *b;
  int32_t xloc, yloc, w, h;

  Assert(iButton >= 0);
  Assert(iButton < MAX_BUTTONS);

  b = ButtonList[iButton];

  xloc = b->XLoc;
  yloc = b->YLoc;
  w = (b->Area.RegionBottomRightX - b->Area.RegionTopLeftX);
  h = (b->Area.RegionBottomRightY - b->Area.RegionTopLeftY);

  if (!(b->uiFlags & BUTTON_SAVEBACKGROUND)) {
    b->uiFlags |= BUTTON_SAVEBACKGROUND;

    b->BackRect =
        RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, NULL, (int16_t)xloc,
                               (int16_t)yloc, (int16_t)(xloc + w), (int16_t)(yloc + h));
  }

  return (TRUE);
}

void FreeButtonSavedRect(int32_t iButton) {
  GUI_BUTTON *b;

  Assert(iButton >= 0);
  Assert(iButton < MAX_BUTTONS);

  b = ButtonList[iButton];

  if ((b->uiFlags & BUTTON_SAVEBACKGROUND)) {
    b->uiFlags &= (~BUTTON_SAVEBACKGROUND);
    FreeBackgroundRectPending(b->BackRect);
  }
}

// Kris:
// Yet new logical additions to the winbart library.
void HideButton(int32_t iButtonNum) {
  GUI_BUTTON *b;

  Assert(iButtonNum >= 0);
  Assert(iButtonNum < MAX_BUTTONS);

  b = ButtonList[iButtonNum];

  Assert(b);

  b->Area.uiFlags &= (~MSYS_REGION_ENABLED);
  b->uiFlags |= BUTTON_DIRTY;
  InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX,
                   b->Area.RegionBottomRightY);
}

void ShowButton(int32_t iButtonNum) {
  GUI_BUTTON *b;

  Assert(iButtonNum >= 0);
  Assert(iButtonNum < MAX_BUTTONS);

  b = ButtonList[iButtonNum];

  Assert(b);

  b->Area.uiFlags |= MSYS_REGION_ENABLED;
  b->uiFlags |= BUTTON_DIRTY;
  InvalidateRegion(b->Area.RegionTopLeftX, b->Area.RegionTopLeftY, b->Area.RegionBottomRightX,
                   b->Area.RegionBottomRightY);
}

void DisableButtonHelpTextRestore(void) { fDisableHelpTextRestoreFlag = TRUE; }

void EnableButtonHelpTextRestore(void) { fDisableHelpTextRestoreFlag = TRUE; }

void GiveButtonDefaultStatus(int32_t iButtonID, int32_t iDefaultStatus) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  // If new default status added, then this assert may need to be adjusted.
  AssertMsg(iDefaultStatus >= DEFAULT_STATUS_NONE && iDefaultStatus <= DEFAULT_STATUS_WINDOWS95,
            String("Illegal button default status of %d", iDefaultStatus));
  Assert(b);

  if (b->bDefaultStatus != (int8_t)iDefaultStatus) {
    b->bDefaultStatus = (int8_t)iDefaultStatus;
    b->uiFlags |= BUTTON_DIRTY;
  }
}

void RemoveButtonDefaultStatus(int32_t iButtonID) {
  GUI_BUTTON *b;
  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  b = ButtonList[iButtonID];
  Assert(b);

  if (b->bDefaultStatus) {
    b->bDefaultStatus = DEFAULT_STATUS_NONE;
    b->uiFlags |= BUTTON_DIRTY;
  }
}

BOOLEAN GetButtonArea(int32_t iButtonID, SGPRect *pRect) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);
  Assert(pRect);

  b = ButtonList[iButtonID];
  Assert(b);

  if ((pRect == NULL) || (b == NULL)) return (FALSE);

  pRect->iLeft = b->Area.RegionTopLeftX;
  pRect->iTop = b->Area.RegionTopLeftY;
  pRect->iRight = b->Area.RegionBottomRightX;
  pRect->iBottom = b->Area.RegionBottomRightY;

  return (TRUE);
}

int32_t GetButtonWidth(int32_t iButtonID) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);

  b = ButtonList[iButtonID];
  Assert(b);

  if (b == NULL) return (-1);

  return (b->Area.RegionBottomRightX - b->Area.RegionTopLeftX);
}

int32_t GetButtonHeight(int32_t iButtonID) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);

  b = ButtonList[iButtonID];
  Assert(b);

  if (b == NULL) return (-1);

  return (b->Area.RegionBottomRightY - b->Area.RegionTopLeftY);
}

int32_t GetButtonX(int32_t iButtonID) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);

  b = ButtonList[iButtonID];
  Assert(b);

  if (b == NULL) return (0);

  return (b->Area.RegionTopLeftX);
}

int32_t GetButtonY(int32_t iButtonID) {
  GUI_BUTTON *b;

  Assert(iButtonID >= 0);
  Assert(iButtonID < MAX_BUTTONS);

  b = ButtonList[iButtonID];
  Assert(b);

  if (b == NULL) return (0);

  return (b->Area.RegionTopLeftY);
}
