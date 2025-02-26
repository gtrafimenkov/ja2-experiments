// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

// FUNCTIONALITY:
// This popup menu functions IDENTICALLY to Windows95's methodology.  You can use the arrow keys, or
// the mouse in two methods.  The first method is where you release the mouse button before moving
// the cursor inside the popup menu, which makes it persistant (it won't go away until you click
// again), or non-persistant, where you release the button inside the menu after your initial click
// (non-persistant). IMPORTANT NOTE: Linda Currie, the main user of the editor dislikes the
// non-persistant functionality, however, it is still supported.  Just remove the commented line of
// code (search for UNCOMMENT), and it's fixed -- it is currently disabled.

#include "Editor/PopupMenu.h"

#include "BuildDefines.h"
#include "Editor/EditorDefines.h"
#include "Editor/EditorMercs.h"
#include "Editor/ItemStatistics.h"
#include "Editor/SelectWin.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Font.h"
#include "SGP/Line.h"
#include "SGP/MouseSystem.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Strategic/Scheduling.h"
#include "Tactical/OverheadTypes.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldDat.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"

CurrentPopupMenuInformation gPopup;

struct MOUSE_REGION popupRegion;

uint16_t gusEntryHeight;
BOOLEAN fWaitingForLButtonRelease = FALSE;

extern wchar_t gszScheduleActions[NUM_SCHEDULE_ACTIONS][20];

// Finds the string for any popup menu in JA2 -- the strings are stored
// in different ways in each instance.
wchar_t *GetPopupMenuString(uint8_t ubIndex) {
  switch (gPopup.ubPopupMenuID) {
    case CHANGETSET_POPUP:  // tile sets
      return gTilesets[ubIndex].zName;
    case OWNERSHIPGROUP_POPUP:  // civilian groups
    case CHANGECIVGROUP_POPUP:  // civilian groups
      return gszCivGroupNames[ubIndex];
    case SCHEDULEACTION_POPUP:  // setting scheduling actions
      return gszScheduleActions[ubIndex];
    case ACTIONITEM_POPUP:
      return gszActionItemDesc[ubIndex];
    default:
      return 0;
  }
}
/*
InitPopUpMenu
---------------
Creates the popup menu, then takes control of the i/o
effectively turning the popup into a modal system (forcing the
user to make a selection or cancel before doing anything else.
There are two modes of operation as soon as the user presses the mouse
on a popup menu activation button.
        1)  The user releases the mouse before moving the cursor onto
                        the popup region.  The popup menu will end as soon as the
                        user presses the mouse button.  If the cursor is out of the
                        popup region, then it returns 0.
        2)  The user holds the mouse while moving the cursor onto the
                        popup region.  The popup menu will end as soon as the user
                        releases the mouse button.  If the cursor is out of the popup
                        region, then it returns 0.


Returns 0 if nothing in menu was selected, else
returns the menu entry number starting from 1.
*/

void InitPopupMenu(int32_t iButtonID, uint8_t ubPopupMenuID, uint8_t ubDirection) {
  uint16_t usX, usY;
  uint16_t usMenuHeight;
  uint16_t usMenuWidth = 0;
  uint16_t usCurrStrWidth;
  uint8_t ubColumn, ubEntry;
  uint8_t ubCounter;
  GUI_BUTTON *button;
  // calculate the location of the menu based on the button position.
  // This also calculates the menu's direction based on position.

  gPopup.usFont = (uint16_t)SMALLFONT1;
  gusEntryHeight = GetFontHeight(gPopup.usFont);

  button = ButtonList[iButtonID];
  MSYS_DisableRegion(&gBottomPanalRegion);

  switch (ubDirection) {
    case DIR_UPRIGHT:
      usX = button->Area.RegionTopLeftX;
      usY = button->Area.RegionTopLeftY;
      break;
    case DIR_UPLEFT:
      usX = button->Area.RegionBottomRightX;
      usY = button->Area.RegionTopLeftY;
      break;
    case DIR_DOWNRIGHT:
      usX = button->Area.RegionTopLeftX;
      usY = button->Area.RegionBottomRightY;
      break;
    case DIR_DOWNLEFT:
      usX = button->Area.RegionBottomRightX;
      usY = button->Area.RegionBottomRightY;
      break;
  }

  // Decipher the popupMenuID
  switch (ubPopupMenuID) {
    case CHANGETSET_POPUP:  // change tileset
      gPopup.ubNumEntries = NUM_TILESETS;
      break;
    case OWNERSHIPGROUP_POPUP:
    case CHANGECIVGROUP_POPUP:
      gPopup.ubNumEntries = NUM_CIV_GROUPS;
      break;
    case SCHEDULEACTION_POPUP:
      gPopup.ubNumEntries = NUM_SCHEDULE_ACTIONS;
      break;
    case ACTIONITEM_POPUP:
      gPopup.ubNumEntries = NUM_ACTIONITEMS;
      break;
    default:
      return;
  }

  gPopup.usFont = (uint16_t)SMALLFONT1;
  gusEntryHeight = GetFontHeight(gPopup.usFont);

  button = ButtonList[iButtonID];
  MSYS_DisableRegion(&gBottomPanalRegion);

  gPopup.ubPopupMenuID = ubPopupMenuID;
  gPopup.ubSelectedIndex = 0;
  gPopup.ubActiveType = POPUP_ACTIVETYPE_NOT_YET_DETERMINED;
  gPopup.fActive = TRUE;
  fWaitingForLButtonRelease = FALSE;
  gPopup.fUseKeyboardInfoUntilMouseMoves = FALSE;
  // Initialize the last mouse position to be out of bounds.
  gPopup.usLastMouseX = 1000;
  gPopup.usLastMouseY = 1000;
  // clear the column widths.
  for (ubColumn = 0; ubColumn < MAX_COLUMNS; ubColumn++) gPopup.ubColumnWidth[ubColumn] = 0;

  // 1) Calc total entry height of the popup region.
  gPopup.ubColumns = 1;
  gPopup.ubMaxEntriesPerColumn = gPopup.ubNumEntries;
  usMenuHeight = gPopup.ubNumEntries * gusEntryHeight + 3;
  while ((usMenuHeight >= usY && (ubDirection == DIR_UPLEFT || ubDirection == DIR_UPRIGHT)) ||
         (480 - usMenuHeight >= usY &&
          (ubDirection == DIR_DOWNLEFT || ubDirection == DIR_DOWNRIGHT))) {
    // menu has too many entries.  Increase the number
    // of columns until the height is
    // less than the max height.
    gPopup.ubMaxEntriesPerColumn =
        (gPopup.ubNumEntries + gPopup.ubColumns) / (gPopup.ubColumns + 1);
    usMenuHeight = gPopup.ubMaxEntriesPerColumn * gusEntryHeight + 3;
    gPopup.ubColumns++;
  }
  // now we have the number of columns as well as the max number of entries per column, and
  // the total menu height.

  // We now calculate the total width of the menu as well as the max width of each column.
  ubCounter = 0;
  usMenuWidth = 0;
  for (ubColumn = 0; ubColumn < gPopup.ubColumns; ubColumn++) {
    for (ubEntry = 0; ubEntry < gPopup.ubMaxEntriesPerColumn; ubEntry++) {
      if (ubCounter >= gPopup.ubNumEntries)
        break;  // done (don't want to process undefined entries...)
      usCurrStrWidth = 16 + StringPixLength(GetPopupMenuString(ubCounter), gPopup.usFont);
      if (usCurrStrWidth > gPopup.ubColumnWidth[ubColumn]) {
        gPopup.ubColumnWidth[ubColumn] = (uint8_t)usCurrStrWidth;
      }
      ubCounter++;
    }
    usMenuWidth += gPopup.ubColumnWidth[ubColumn];
  }

  // Calculate popup menu boundaries based on direction from argument point.
  switch (ubDirection) {
    case DIR_UPRIGHT:
      gPopup.usLeft = usX;
      gPopup.usTop = usY - usMenuHeight - 1;
      gPopup.usRight = usX + usMenuWidth;
      gPopup.usBottom = usY - 1;
      break;
    case DIR_UPLEFT:
      gPopup.usLeft = usX - usMenuWidth;
      gPopup.usTop = usY - usMenuHeight - 1;
      gPopup.usRight = usX;
      gPopup.usBottom = usY - 1;
      break;
    case DIR_DOWNRIGHT:
      gPopup.usLeft = usX;
      gPopup.usTop = usY + 1;
      gPopup.usRight = usX + usMenuWidth;
      gPopup.usBottom = usY + usMenuHeight + 1;
      break;
    case DIR_DOWNLEFT:
      gPopup.usLeft = usX - usMenuWidth;
      gPopup.usTop = usY + 1;
      gPopup.usRight = usX;
      gPopup.usBottom = usY + usMenuHeight + 1;
      break;
  }
  MSYS_DefineRegion(&popupRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL,
                    MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

  RenderPopupMenu();
}

void RenderPopupMenu() {
  uint16_t usX, usY;
  uint8_t ubColumn, ubEntry, ubCounter;
  uint8_t *pDestBuf;
  uint32_t uiDestPitchBYTES;
  uint16_t usLineColor;
  uint16_t usStringWidth;
  uint16_t usStart;

  // Draw the menu
  ColorFillVSurfaceArea(vsFB, gPopup.usLeft, gPopup.usTop, gPopup.usRight, gPopup.usBottom,
                        rgb32_to_rgb16(FROMRGB(128, 128, 128)));
  pDestBuf = LockVSurface(vsFB, &uiDestPitchBYTES);
  SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);
  usLineColor = rgb32_to_rgb16(FROMRGB(64, 64, 64));
  RectangleDraw(TRUE, gPopup.usLeft, gPopup.usTop, gPopup.usRight, gPopup.usBottom, usLineColor,
                pDestBuf);
  if (gPopup.ubColumns > 1) {  // draw a vertical line between each column
    usStart = gPopup.usLeft + gPopup.ubColumnWidth[0];
    for (ubColumn = 1; ubColumn < gPopup.ubColumns; ubColumn++) {
      LineDraw(TRUE, usStart, gPopup.usTop, usStart, gPopup.usBottom, usLineColor, pDestBuf);
    }
    usStart += (uint16_t)gPopup.ubColumnWidth[ubColumn];
  }
  JSurface_Unlock(vsFB);

  // Set up the text attributes.
  SetFont(gPopup.usFont);
  SetFontBackground(FONT_MCOLOR_BLACK);
  SetFontForeground(FONT_MCOLOR_WHITE);

  usX = gPopup.usLeft + 1;
  ubCounter = 0;
  usStart = gPopup.usLeft;
  for (ubColumn = 0; ubColumn < gPopup.ubColumns; ubColumn++) {
    for (ubEntry = 0; ubEntry < gPopup.ubMaxEntriesPerColumn; ubEntry++) {
      if (ubCounter >= gPopup.ubNumEntries) return;  // done
      // Calc current string's width in pixels.  Adding 14 pixels which is the width of
      // two padded gPopup.usFont spaces not stored in the string.
      usStringWidth = 14 + StringPixLength(GetPopupMenuString(ubCounter), gPopup.usFont);
      // Horizontally center the string inside the popup menu
      usX = usStart + (gPopup.ubColumnWidth[ubColumn] - usStringWidth) / 2;
      usY = gPopup.usTop + 1 + ubEntry * gusEntryHeight;
      if (ubCounter == gPopup.ubSelectedIndex - 1) {
        // This is the highlighted menu entry.
        SetFontForeground(FONT_MCOLOR_LTBLUE);
        mprintf(usX, usY, L" %s ", GetPopupMenuString(ubCounter));
        SetFontForeground(FONT_MCOLOR_WHITE);
      } else {
        mprintf(usX, usY, L" %s ", GetPopupMenuString(ubCounter));
      }
      ubCounter++;
    }
    usStart += gPopup.ubColumnWidth[ubColumn];
  }
}

// This private function of PopupMenuHandle determines which menu entry
// is highlighted based on the mouse cursor position.  Returns 0 if the
// mouse is out of the menu region.
uint8_t GetPopupIndexFromMousePosition() {
  uint8_t ubNumEntriesDown;
  uint16_t usRelX;
  uint8_t ubCount;
  if (gusMouseXPos >= gPopup.usLeft && gusMouseXPos <= gPopup.usRight &&
      gusMouseYPos > gPopup.usTop             // one pixel gap on top ignored
      && gusMouseYPos < gPopup.usBottom - 2)  // two pixel gap on bottom ignored
  {
    // subtract the top y coord of the popup region from the mouse's yPos as well
    // as an extra pixel at the top of the region which is ignored in menu selection,
    // divide this number by the height of a menu entry, then add one.  This will
    // return the menu index from 1 (at the top) to n (at the bottom).
    ubNumEntriesDown = (gusMouseYPos - gPopup.usTop - 1) / gusEntryHeight + 1;
    usRelX = gusMouseXPos - gPopup.usLeft;
    ubCount = 0;
    while (usRelX > gPopup.ubColumnWidth[ubCount]) {
      usRelX -= gPopup.ubColumnWidth[ubCount];
      ubCount++;
      ubNumEntriesDown += gPopup.ubMaxEntriesPerColumn;
    }
    if (ubNumEntriesDown >= gPopup.ubNumEntries) ubNumEntriesDown = gPopup.ubNumEntries;
    return ubNumEntriesDown;
  }
  return 0;  // mouse not in valid region.
}

void PopupMenuHandle() {
  InputAtom InputEvent;

  if (gPopup.ubActiveType == POPUP_ACTIVETYPE_NOT_YET_DETERMINED) {
    // Attempt to determine if the menu will be persistant or not.
    // Determination is made when the mouse's left button is released or if
    // the mouse cursor enters the menu region.
    if (gusMouseXPos >= gPopup.usLeft && gusMouseXPos <= gPopup.usRight &&
        gusMouseYPos > gPopup.usTop             // one pixel gap on top ignored
        && gusMouseYPos < gPopup.usBottom - 1)  // two pixel gap on bottom ignored
    {
      // mouse cursor has just entered the menu region -- nonpersistant.

      // KRIS:  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // Disabled this because Linda doesn't like it... (I like it though, and it works)
      //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      // UNCOMMENT IF NONPERSISTANT IS ALLOWED
      // gPopup.ubActiveType = POPUP_ACTIVETYPE_NONPERSISTANT;

      return;
    } else if (!gfLeftButtonState) {  // left button has been released before entering region --
                                      // persistant
      gPopup.ubActiveType = POPUP_ACTIVETYPE_PERSISTANT;
      return;
    }
  }
  if (!gPopup.fUseKeyboardInfoUntilMouseMoves) {
    // check menu entry based on mouse position
    gPopup.ubSelectedIndex = GetPopupIndexFromMousePosition();
  } else if (gusMouseXPos != gPopup.usLastMouseX || gusMouseYPos != gPopup.usLastMouseY) {
    // The keyboard determined the last entry, but the mouse has moved,
    // so use the mouse to determine the new entry.
    gPopup.fUseKeyboardInfoUntilMouseMoves = FALSE;
    gPopup.ubSelectedIndex = GetPopupIndexFromMousePosition();
  }
  // Check terminating conditions for persistant states.
  if (gfLeftButtonState && gPopup.ubActiveType == POPUP_ACTIVETYPE_PERSISTANT)
    fWaitingForLButtonRelease = TRUE;
  if ((gfLeftButtonState && gPopup.ubActiveType == POPUP_ACTIVETYPE_PERSISTANT) ||
      (!gfLeftButtonState && gPopup.ubActiveType == POPUP_ACTIVETYPE_NONPERSISTANT)) {
    // Selection conditions via mouse have been met whether the mouse is in the
    // menu region or not.
    gPopup.ubSelectedIndex = GetPopupIndexFromMousePosition();
    if (gPopup.ubSelectedIndex) {
      ProcessPopupMenuSelection();
    }
    gPopup.fActive = FALSE;
    MSYS_RemoveRegion(&popupRegion);
    gfRenderWorld = TRUE;
    gfRenderTaskbar = TRUE;
    return;
  }
  // Use keyboard input as well.
  while (DequeueEvent(&InputEvent)) {
    switch (InputEvent.usEvent) {
      case KEY_DOWN:
        switch (InputEvent.usParam) {
          case DNARROW:
            gPopup.fUseKeyboardInfoUntilMouseMoves = TRUE;
            gPopup.usLastMouseX = gusMouseXPos;
            gPopup.usLastMouseY = gusMouseYPos;
            gPopup.ubSelectedIndex++;
            if (gPopup.ubSelectedIndex > gPopup.ubNumEntries) {
              gPopup.ubSelectedIndex = 1;
            }
            break;
          case UPARROW:
            gPopup.fUseKeyboardInfoUntilMouseMoves = TRUE;
            gPopup.usLastMouseX = gusMouseXPos;
            gPopup.usLastMouseY = gusMouseYPos;
            if (gPopup.ubSelectedIndex < 2) {
              gPopup.ubSelectedIndex = gPopup.ubNumEntries;
            } else {
              gPopup.ubSelectedIndex--;
            }
            break;
          case ESC:
            gPopup.fActive = FALSE;
            MSYS_RemoveRegion(&popupRegion);
            gfRenderWorld = TRUE;
            gfRenderTaskbar = TRUE;
            break;
          case ENTER:
            ProcessPopupMenuSelection();
            gPopup.fActive = FALSE;
            MSYS_RemoveRegion(&popupRegion);
            gfRenderWorld = TRUE;
            gfRenderTaskbar = TRUE;
            break;
        }
        break;
    }
  }
}

void ProcessPopupMenuSelection() {
  switch (gPopup.ubPopupMenuID) {
    case CHANGETSET_POPUP:
      // change the tileset here.
      ReloadTileset((uint8_t)(gPopup.ubSelectedIndex - 1));
      InitJA2SelectionWindow();
      break;
    case CHANGECIVGROUP_POPUP:
      ChangeCivGroup((uint8_t)(gPopup.ubSelectedIndex - 1));
      break;
    case SCHEDULEACTION_POPUP:
      UpdateScheduleAction((uint8_t)(gPopup.ubSelectedIndex - 1));
      break;
    case ACTIONITEM_POPUP:
      UpdateActionItem((uint8_t)(gPopup.ubSelectedIndex - 1));
      break;
    case OWNERSHIPGROUP_POPUP:
      SetOwnershipGroup((uint8_t)(gPopup.ubSelectedIndex - 1));
      break;
  }
}

BOOLEAN ProcessPopupMenuIfActive() {
  if (!gPopup.fActive && !fWaitingForLButtonRelease) return FALSE;
  if (fWaitingForLButtonRelease) {
    if (!gfLeftButtonState) {
      fWaitingForLButtonRelease = FALSE;
      return FALSE;
    }
    return TRUE;
  }
  PopupMenuHandle();
  RenderPopupMenu();
  InvalidateRegion(gPopup.usLeft, gPopup.usTop, gPopup.usRight, gPopup.usBottom);
  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();
  return TRUE;
}
