// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/MapScreenInterfaceBorder.h"

#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/MouseSystem.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MapScreenInterfaceMapInventory.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "TileEngine/SysUtil.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"

#define MAP_BORDER_X 261
#define MAP_BORDER_Y 0

#define MAP_BORDER_CORNER_X 584
#define MAP_BORDER_CORNER_Y 279

// extern to anchored button in winbart97
extern GUI_BUTTON *gpAnchoredButton;
extern BOOLEAN gfAnchoredState;

// mouse levels
struct MOUSE_REGION LevelMouseRegions[4];

// graphics
uint32_t guiMapBorder;
// uint32_t guiMapBorderCorner;

// scroll direction
int32_t giScrollButtonState = -1;

// flags
BOOLEAN fShowTownFlag = FALSE;
BOOLEAN fShowMineFlag = FALSE;
BOOLEAN fShowTeamFlag = FALSE;
BOOLEAN fShowMilitia = FALSE;
BOOLEAN fShowAircraftFlag = FALSE;
BOOLEAN fShowItemsFlag = FALSE;

// buttons & button images
int32_t giMapBorderButtons[6] = {-1, -1, -1, -1, -1, -1};
int32_t giMapBorderButtonsImage[6] = {-1, -1, -1, -1, -1, -1};

void DeleteMapBorderButtons(void);
BOOLEAN CreateButtonsForMapBorder(void);

// set button states to match map flags
void InitializeMapBorderButtonStates(void);

// blit in the level marker
void DisplayCurrentLevelMarker(void);

extern void CancelMapUIMessage(void);

// callbacks
void BtnTownCallback(GUI_BUTTON *btn, int32_t reason);
void BtnMineCallback(GUI_BUTTON *btn, int32_t reason);
void BtnItemCallback(GUI_BUTTON *btn, int32_t reason);
void BtnAircraftCallback(GUI_BUTTON *btn, int32_t reason);
void BtnTeamCallback(GUI_BUTTON *btn, int32_t reason);
void BtnMilitiaCallback(GUI_BUTTON *btn, int32_t reason);
// void BtnZoomCallback(GUI_BUTTON *btn,int32_t reason);

void BtnGenericMouseMoveButtonCallbackForMapBorder(GUI_BUTTON *btn, int32_t reason);
void LevelMarkerBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

void CommonBtnCallbackBtnDownChecks(void);

/*
void BtnScrollNorthMapScreenCallback( GUI_BUTTON *btn,int32_t reason );
void BtnScrollSouthMapScreenCallback( GUI_BUTTON *btn,int32_t reason );
void BtnScrollWestMapScreenCallback( GUI_BUTTON *btn,int32_t reason );
void BtnScrollEastMapScreenCallback( GUI_BUTTON *btn,int32_t reason );
void BtnLowerLevelBtnCallback(GUI_BUTTON *btn,int32_t reason);
void BtnRaiseLevelBtnCallback(GUI_BUTTON *btn,int32_t reason);
*/

BOOLEAN LoadMapBorderGraphics(void) {
  // this procedure will load the graphics needed for the map border

  // will load map border
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\MBS.sti"), &guiMapBorder));

  return (TRUE);
}

void DeleteMapBorderGraphics(void) {
  // procedure will delete graphics loaded for map border

  DeleteVideoObjectFromIndex(guiMapBorder);
  //	DeleteVideoObjectFromIndex( guiMapBorderCorner );

  return;
}

void RenderMapBorder(void) {
  // renders the actual border to the vsSaveBuffer
  struct VObject *hHandle;

  /*
          if( fDisabledMapBorder )
          {
                  return;
          }
  */

  if (fShowMapInventoryPool) {
    // render background, then leave
    BlitInventoryPoolGraphic();
    return;
  }

  // get and blt border
  GetVideoObject(&hHandle, guiMapBorder);
  BltVideoObject(vsSaveBuffer, hHandle, 0, MAP_BORDER_X, MAP_BORDER_Y);

  // show the level marker
  DisplayCurrentLevelMarker();

  return;
}

void RenderMapBorderEtaPopUp(void) {
  // renders map border corner to the vsFB
  struct VObject *hHandle;

  /*
          if( fDisabledMapBorder )
          {
                  return;
          }
  */

  if (fShowMapInventoryPool) {
    return;
  }

  if (fPlotForHelicopter == TRUE) {
    DisplayDistancesForHelicopter();
    return;
  }

  // get and blt ETA box
  GetVideoObject(&hHandle, guiMapBorderEtaPopUp);
  BltVideoObject(vsFB, hHandle, 0, MAP_BORDER_X + 215, 291);

  InvalidateRegion(MAP_BORDER_X + 215, 291, MAP_BORDER_X + 215 + 100, 310);

  return;
}

BOOLEAN CreateButtonsForMapBorder(void) {
  // will create the buttons needed for the map screen border region

  /*
          // up button
          guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_UP ] = LoadButtonImage(
    "INTERFACE\\map_screen_bottom_arrows.sti" ,11,4,-1,6,-1 ); guiMapBorderScrollButtons[
    ZOOM_MAP_SCROLL_UP ] = QuickCreateButton( guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_UP ],
    602, 303, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
    (GUI_CALLBACK)BtnScrollNorthMapScreenCallback);

          // dwn button
          guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_DWN ] = LoadButtonImage(
    "INTERFACE\\map_screen_bottom_arrows.sti" ,12,5,-1,7,-1 ); guiMapBorderScrollButtons[
    ZOOM_MAP_SCROLL_DWN ] = QuickCreateButton( guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_DWN
    ], 602, 338, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
    (GUI_CALLBACK)BtnScrollSouthMapScreenCallback);

          // left button
          guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_LEFT ] = LoadButtonImage(
    "INTERFACE\\map_screen_bottom_arrows.sti" ,9,0,-1,2,-1 ); guiMapBorderScrollButtons[
    ZOOM_MAP_SCROLL_LEFT ] = QuickCreateButton( guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_LEFT
    ], 584, 322, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
    (GUI_CALLBACK)BtnScrollWestMapScreenCallback);

          // right button
          guiMapBorderScrollButtonsImage[ ZOOM_MAP_SCROLL_RIGHT ] = LoadButtonImage(
    "INTERFACE\\map_screen_bottom_arrows.sti" ,10,1,-1,3,-1 ); guiMapBorderScrollButtons[
    ZOOM_MAP_SCROLL_RIGHT ] = QuickCreateButton( guiMapBorderScrollButtonsImage[
    ZOOM_MAP_SCROLL_RIGHT ], 619, 322, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
                                                                                  (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
    (GUI_CALLBACK)BtnScrollEastMapScreenCallback);

          // set up fast help text
          SetButtonFastHelpText( guiMapBorderScrollButtons[ 0 ], pMapScreenBorderButtonHelpText[ 6 ]
    ); SetButtonFastHelpText( guiMapBorderScrollButtons[ 1 ], pMapScreenBorderButtonHelpText[ 7 ] );
          SetButtonFastHelpText( guiMapBorderScrollButtons[ 2 ], pMapScreenBorderButtonHelpText[ 8 ]
    ); SetButtonFastHelpText( guiMapBorderScrollButtons[ 3 ], pMapScreenBorderButtonHelpText[ 9 ] );
  */

  // towns
  giMapBorderButtonsImage[MAP_BORDER_TOWN_BTN] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 5, -1, 14, -1);
  giMapBorderButtons[MAP_BORDER_TOWN_BTN] = QuickCreateButton(
      giMapBorderButtonsImage[MAP_BORDER_TOWN_BTN], 299, 323, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnTownCallback);

  // mines
  giMapBorderButtonsImage[MAP_BORDER_MINE_BTN] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 4, -1, 13, -1);
  giMapBorderButtons[MAP_BORDER_MINE_BTN] = QuickCreateButton(
      giMapBorderButtonsImage[MAP_BORDER_MINE_BTN], 342, 323, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnMineCallback);

  // people
  giMapBorderButtonsImage[MAP_BORDER_TEAMS_BTN] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 3, -1, 12, -1);
  giMapBorderButtons[MAP_BORDER_TEAMS_BTN] = QuickCreateButton(
      giMapBorderButtonsImage[MAP_BORDER_TEAMS_BTN], 385, 323, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnTeamCallback);

  // militia
  giMapBorderButtonsImage[MAP_BORDER_MILITIA_BTN] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 8, -1, 17, -1);
  giMapBorderButtons[MAP_BORDER_MILITIA_BTN] = QuickCreateButton(
      giMapBorderButtonsImage[MAP_BORDER_MILITIA_BTN], 428, 323, BUTTON_NO_TOGGLE,
      MSYS_PRIORITY_HIGH, (GUI_CALLBACK)MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnMilitiaCallback);

  // airspace
  giMapBorderButtonsImage[MAP_BORDER_AIRSPACE_BTN] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 2, -1, 11, -1);
  giMapBorderButtons[MAP_BORDER_AIRSPACE_BTN] = QuickCreateButton(
      giMapBorderButtonsImage[MAP_BORDER_AIRSPACE_BTN], 471, 323, BUTTON_NO_TOGGLE,
      MSYS_PRIORITY_HIGH, (GUI_CALLBACK)MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnAircraftCallback);

  // items
  giMapBorderButtonsImage[MAP_BORDER_ITEM_BTN] =
      LoadButtonImage("INTERFACE\\map_border_buttons.sti", -1, 1, -1, 10, -1);
  giMapBorderButtons[MAP_BORDER_ITEM_BTN] = QuickCreateButton(
      giMapBorderButtonsImage[MAP_BORDER_ITEM_BTN], 514, 323, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH,
      (GUI_CALLBACK)MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnItemCallback);

  // raise and lower view level

  // raise
  /*
  guiMapBorderLandRaiseButtonsImage[ MAP_BORDER_RAISE_LEVEL ] = LoadButtonImage(
  "INTERFACE\\map_screen_bottom_arrows.sti" ,11,4,-1,6,-1 ); guiMapBorderLandRaiseButtons[
  MAP_BORDER_RAISE_LEVEL ] = QuickCreateButton( guiMapBorderLandRaiseButtonsImage[
  MAP_BORDER_RAISE_LEVEL ], MAP_BORDER_X + 264, 322, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
                                                                          (GUI_CALLBACK)MSYS_NO_CALLBACK,
  (GUI_CALLBACK)BtnRaiseLevelBtnCallback);

  // lower
  guiMapBorderLandRaiseButtonsImage[ MAP_BORDER_LOWER_LEVEL ] = LoadButtonImage(
  "INTERFACE\\map_screen_bottom_arrows.sti" ,12,5,-1,7,-1  ); guiMapBorderLandRaiseButtons[
  MAP_BORDER_LOWER_LEVEL ] = QuickCreateButton( guiMapBorderLandRaiseButtonsImage[
  MAP_BORDER_LOWER_LEVEL ], MAP_BORDER_X + 264, 340, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
                                                                          (GUI_CALLBACK)MSYS_NO_CALLBACK,
  (GUI_CALLBACK)BtnLowerLevelBtnCallback);

*/
  // set up fast help text
  SetButtonFastHelpText(giMapBorderButtons[0], pMapScreenBorderButtonHelpText[0]);
  SetButtonFastHelpText(giMapBorderButtons[1], pMapScreenBorderButtonHelpText[1]);
  SetButtonFastHelpText(giMapBorderButtons[2], pMapScreenBorderButtonHelpText[2]);
  SetButtonFastHelpText(giMapBorderButtons[3], pMapScreenBorderButtonHelpText[3]);
  SetButtonFastHelpText(giMapBorderButtons[4], pMapScreenBorderButtonHelpText[4]);
  SetButtonFastHelpText(giMapBorderButtons[5], pMapScreenBorderButtonHelpText[5]);

  // SetButtonFastHelpText( guiMapBorderLandRaiseButtons[ 0 ], pMapScreenBorderButtonHelpText[ 10 ]
  // ); SetButtonFastHelpText( guiMapBorderLandRaiseButtons[ 1 ], pMapScreenBorderButtonHelpText[ 11
  // ] );

  SetButtonCursor(giMapBorderButtons[0], MSYS_NO_CURSOR);
  SetButtonCursor(giMapBorderButtons[1], MSYS_NO_CURSOR);
  SetButtonCursor(giMapBorderButtons[2], MSYS_NO_CURSOR);
  SetButtonCursor(giMapBorderButtons[3], MSYS_NO_CURSOR);
  SetButtonCursor(giMapBorderButtons[4], MSYS_NO_CURSOR);
  SetButtonCursor(giMapBorderButtons[5], MSYS_NO_CURSOR);

  //	SetButtonCursor(guiMapBorderLandRaiseButtons[ 0 ], MSYS_NO_CURSOR );
  //	SetButtonCursor(guiMapBorderLandRaiseButtons[ 1 ], MSYS_NO_CURSOR );

  InitializeMapBorderButtonStates();

  return (TRUE);
}

void DeleteMapBorderButtons(void) {
  uint8_t ubCnt;

  /*
          RemoveButton( guiMapBorderScrollButtons[ 0 ]);
          RemoveButton( guiMapBorderScrollButtons[ 1 ]);
          RemoveButton( guiMapBorderScrollButtons[ 2 ]);
          RemoveButton( guiMapBorderScrollButtons[ 3 ]);
  */

  RemoveButton(giMapBorderButtons[0]);
  RemoveButton(giMapBorderButtons[1]);
  RemoveButton(giMapBorderButtons[2]);
  RemoveButton(giMapBorderButtons[3]);
  RemoveButton(giMapBorderButtons[4]);
  RemoveButton(giMapBorderButtons[5]);

  // RemoveButton( guiMapBorderLandRaiseButtons[ 0 ]);
  // RemoveButton( guiMapBorderLandRaiseButtons[ 1 ]);

  // images

  /*
          UnloadButtonImage( guiMapBorderScrollButtonsImage[ 0 ] );
          UnloadButtonImage( guiMapBorderScrollButtonsImage[ 1 ] );
          UnloadButtonImage( guiMapBorderScrollButtonsImage[ 2 ] );
          UnloadButtonImage( guiMapBorderScrollButtonsImage[ 3 ] );
  */

  UnloadButtonImage(giMapBorderButtonsImage[0]);
  UnloadButtonImage(giMapBorderButtonsImage[1]);
  UnloadButtonImage(giMapBorderButtonsImage[2]);
  UnloadButtonImage(giMapBorderButtonsImage[3]);
  UnloadButtonImage(giMapBorderButtonsImage[4]);
  UnloadButtonImage(giMapBorderButtonsImage[5]);

  // UnloadButtonImage( guiMapBorderLandRaiseButtonsImage[ 0 ] );
  // UnloadButtonImage( guiMapBorderLandRaiseButtonsImage[ 1 ] );

  for (ubCnt = 0; ubCnt < 6; ubCnt++) {
    giMapBorderButtons[ubCnt] = -1;
    giMapBorderButtonsImage[ubCnt] = -1;
  }
}

// callbacks

/*
void BtnLowerLevelBtnCallback(GUI_BUTTON *btn,int32_t reason)
{


        if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                // are help messages being displayed?..redraw
                if( ScrollButtonsDisplayingHelpMessage( ) )
                {
                        MarkForRedrawalStrategicMap();
                }

                MarkButtonsDirty( );

          btn->uiFlags|=(BUTTON_CLICKED_ON);
        }
        else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
    if (btn->uiFlags & BUTTON_CLICKED_ON)
                {
      btn->uiFlags&=~(BUTTON_CLICKED_ON);

                  // go down one level
                  GoDownOneLevelInMap( );
                }
        }
}


void BtnRaiseLevelBtnCallback(GUI_BUTTON *btn,int32_t reason)
{
        if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
        {
                // are help messages being displayed?..redraw
                if( ScrollButtonsDisplayingHelpMessage( ) )
                {
                        MarkForRedrawalStrategicMap();
                }


                MarkButtonsDirty( );

          btn->uiFlags|=(BUTTON_CLICKED_ON);
        }
        else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
    if (btn->uiFlags & BUTTON_CLICKED_ON)
                {
      btn->uiFlags&=~(BUTTON_CLICKED_ON);
                        // go up one level
                  GoUpOneLevelInMap( );
                }
        }
}
*/

void BtnMilitiaCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
    ToggleShowMilitiaMode();
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
  }
}

void BtnTeamCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
    ToggleShowTeamsMode();
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
  }
}

void BtnTownCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
    ToggleShowTownsMode();
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
  }
}

void BtnMineCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
    ToggleShowMinesMode();
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
  }
}

void BtnAircraftCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();

    ToggleAirspaceMode();
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
  }
}

void BtnItemCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();

    ToggleItemsFilter();
  } else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
    CommonBtnCallbackBtnDownChecks();
  }
}

void ToggleShowTownsMode(void) {
  if (fShowTownFlag == TRUE) {
    fShowTownFlag = FALSE;
    MapBorderButtonOff(MAP_BORDER_TOWN_BTN);
  } else {
    fShowTownFlag = TRUE;
    MapBorderButtonOn(MAP_BORDER_TOWN_BTN);

    if (fShowMineFlag == TRUE) {
      fShowMineFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_MINE_BTN);
    }

    if (fShowAircraftFlag == TRUE) {
      fShowAircraftFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_AIRSPACE_BTN);
    }

    if (fShowItemsFlag == TRUE) {
      fShowItemsFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_ITEM_BTN);
    }
  }

  MarkForRedrawalStrategicMap();
}

void ToggleShowMinesMode(void) {
  if (fShowMineFlag == TRUE) {
    fShowMineFlag = FALSE;
    MapBorderButtonOff(MAP_BORDER_MINE_BTN);
  } else {
    fShowMineFlag = TRUE;
    MapBorderButtonOn(MAP_BORDER_MINE_BTN);

    if (fShowTownFlag == TRUE) {
      fShowTownFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_TOWN_BTN);
    }

    if (fShowAircraftFlag == TRUE) {
      fShowAircraftFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_AIRSPACE_BTN);
    }

    if (fShowItemsFlag == TRUE) {
      fShowItemsFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_ITEM_BTN);
    }
  }

  MarkForRedrawalStrategicMap();
}

void ToggleShowMilitiaMode(void) {
  if (fShowMilitia == TRUE) {
    fShowMilitia = FALSE;
    MapBorderButtonOff(MAP_BORDER_MILITIA_BTN);
  } else {
    // toggle militia ON
    fShowMilitia = TRUE;
    MapBorderButtonOn(MAP_BORDER_MILITIA_BTN);

    // if Team is ON, turn it OFF
    if (fShowTeamFlag == TRUE) {
      fShowTeamFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_TEAMS_BTN);
    }

    if (fShowItemsFlag == TRUE) {
      fShowItemsFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_ITEM_BTN);
    }

    // check if player has any militia
    if (DoesPlayerHaveAnyMilitia() == FALSE) {
      wchar_t *pwString = NULL;

      // no - so put up a message explaining how it works

      // if he's already training some
      if (IsAnyOneOnPlayersTeamOnThisAssignment(TRAIN_TOWN)) {
        // say they'll show up when training is completed
        pwString = pMapErrorString[28];
      } else {
        // say you need to train them first
        pwString = zMarksMapScreenText[1];
      }

      MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pwString);
    }
  }

  MarkForRedrawalStrategicMap();
}

void ToggleShowTeamsMode(void) {
  if (fShowTeamFlag == TRUE) {
    // turn show teams OFF
    fShowTeamFlag = FALSE;
    MapBorderButtonOff(MAP_BORDER_TEAMS_BTN);

    // dirty regions
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  } else {  // turn show teams ON
    TurnOnShowTeamsMode();
  }
}

void ToggleAirspaceMode(void) {
  if (fShowAircraftFlag == TRUE) {
    // turn airspace OFF
    fShowAircraftFlag = FALSE;
    MapBorderButtonOff(MAP_BORDER_AIRSPACE_BTN);

    if (fPlotForHelicopter == TRUE) {
      AbortMovementPlottingMode();
    }

    // dirty regions
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  } else {  // turn airspace ON
    TurnOnAirSpaceMode();
  }
}

void ToggleItemsFilter(void) {
  if (fShowItemsFlag == TRUE) {
    // turn items OFF
    fShowItemsFlag = FALSE;
    MapBorderButtonOff(MAP_BORDER_ITEM_BTN);

    // dirty regions
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  } else {
    // turn items ON
    TurnOnItemFilterMode();
  }
}

// generic button mvt callback for mapscreen map border
void BtnGenericMouseMoveButtonCallbackForMapBorder(GUI_BUTTON *btn, int32_t reason) {
  // If the button isn't the anchored button, then we don't want to modify the button state.

  if (btn != gpAnchoredButton) {
    if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
      if (btn->Area.uiFlags & MSYS_FASTHELP) {
        // redraw area
        MarkForRedrawalStrategicMap();
      }
    }
    return;
  }

  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    if (!gfAnchoredState) btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

/*
BOOLEAN ScrollButtonsDisplayingHelpMessage( void )
{
        // return if any help messages are being displayed for the scroll buttons

        if( ( ButtonList[ guiMapBorderScrollButtons[ 0 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT )||
        ( ButtonList[ guiMapBorderScrollButtons[ 1 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT )||
        ( ButtonList[ guiMapBorderScrollButtons[ 2 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT )||
        ( ButtonList[ guiMapBorderScrollButtons[ 3 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT ) )
        {
                return( TRUE );
        }

        return( FALSE );
}
*/

void DisplayCurrentLevelMarker(void) {
  // display the current level marker on the map border

  struct VObject *hHandle;

  /*
          if( fDisabledMapBorder )
          {
                  return;
          }
  */

  // it's actually a white rectangle, not a green arrow!
  GetVideoObject(&hHandle, guiLEVELMARKER);
  BltVideoObject(vsSaveBuffer, hHandle, 0, MAP_LEVEL_MARKER_X + 1,
                 MAP_LEVEL_MARKER_Y + (MAP_LEVEL_MARKER_DELTA * (int16_t)iCurrentMapSectorZ));

  return;
}

void CreateMouseRegionsForLevelMarkers(void) {
  int16_t sCounter = 0;
  wchar_t sString[64];

  for (sCounter = 0; sCounter < 4; sCounter++) {
    MSYS_DefineRegion(&LevelMouseRegions[sCounter], MAP_LEVEL_MARKER_X,
                      (int16_t)(MAP_LEVEL_MARKER_Y + (MAP_LEVEL_MARKER_DELTA * sCounter)),
                      MAP_LEVEL_MARKER_X + MAP_LEVEL_MARKER_WIDTH,
                      (int16_t)(MAP_LEVEL_MARKER_Y + (MAP_LEVEL_MARKER_DELTA * (sCounter + 1))),
                      MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, LevelMarkerBtnCallback);

    MSYS_SetRegionUserData(&LevelMouseRegions[sCounter], 0, sCounter);

    swprintf(sString, ARR_SIZE(sString), L"%s %d", zMarksMapScreenText[0], sCounter + 1);
    SetRegionFastHelpText(&LevelMouseRegions[sCounter], sString);
  }
}

void DeleteMouseRegionsForLevelMarkers(void) {
  int16_t sCounter = 0;

  for (sCounter = 0; sCounter < 4; sCounter++) {
    MSYS_RemoveRegion(&LevelMouseRegions[sCounter]);
  }
}

void LevelMarkerBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment screen mask region
  int32_t iCounter = 0;

  iCounter = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    JumpToLevel(iCounter);
  }
}

/*
void DisableMapBorderRegion( void )
{
        // will shutdown map border region

        if( fDisabledMapBorder )
        {
                // checked, failed
                return;
        }

        // get rid of graphics and mouse regions
        DeleteMapBorderGraphics( );


        fDisabledMapBorder = TRUE;
}

void EnableMapBorderRegion( void )
{
        // will re-enable mapborder region

        if( fDisabledMapBorder == FALSE )
        {
                // checked, failed
                return;
        }

        // re load graphics and buttons
        LoadMapBorderGraphics( );

        fDisabledMapBorder = FALSE;

}
*/

void TurnOnShowTeamsMode(void) {
  // if mode already on, leave, else set and redraw

  if (fShowTeamFlag == FALSE) {
    fShowTeamFlag = TRUE;
    MapBorderButtonOn(MAP_BORDER_TEAMS_BTN);

    if (fShowMilitia == TRUE) {
      fShowMilitia = FALSE;
      MapBorderButtonOff(MAP_BORDER_MILITIA_BTN);
    }

    /*
                    if( fShowAircraftFlag == TRUE )
                    {
                            fShowAircraftFlag = FALSE;
                            MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
                    }
    */

    if (fShowItemsFlag == TRUE) {
      fShowItemsFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_ITEM_BTN);
    }

    // dirty regions
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  }
}

void TurnOnAirSpaceMode(void) {
  // if mode already on, leave, else set and redraw

  if (fShowAircraftFlag == FALSE) {
    fShowAircraftFlag = TRUE;
    MapBorderButtonOn(MAP_BORDER_AIRSPACE_BTN);

    // Turn off towns & mines (mostly because town/mine names overlap SAM site names)
    if (fShowTownFlag == TRUE) {
      fShowTownFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_TOWN_BTN);
    }

    if (fShowMineFlag == TRUE) {
      fShowMineFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_MINE_BTN);
    }

    /*
                    // Turn off teams and militia
                    if( fShowTeamFlag == TRUE )
                    {
                            fShowTeamFlag = FALSE;
                            MapBorderButtonOff( MAP_BORDER_TEAMS_BTN );
                    }

                    if( fShowMilitia == TRUE )
                    {
                            fShowMilitia = FALSE;
                            MapBorderButtonOff( MAP_BORDER_MILITIA_BTN );
                    }
    */

    // Turn off items
    if (fShowItemsFlag == TRUE) {
      fShowItemsFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_ITEM_BTN);
    }

    if (bSelectedDestChar != -1) {
      AbortMovementPlottingMode();
    }

    // if showing underground
    if (iCurrentMapSectorZ != 0) {
      // switch to the surface
      JumpToLevel(0);
    }

    // dirty regions
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  }
}

void TurnOnItemFilterMode(void) {
  // if mode already on, leave, else set and redraw

  if (fShowItemsFlag == FALSE) {
    fShowItemsFlag = TRUE;
    MapBorderButtonOn(MAP_BORDER_ITEM_BTN);

    // Turn off towns, mines, teams, militia & airspace if any are on
    if (fShowTownFlag == TRUE) {
      fShowTownFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_TOWN_BTN);
    }

    if (fShowMineFlag == TRUE) {
      fShowMineFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_MINE_BTN);
    }

    if (fShowTeamFlag == TRUE) {
      fShowTeamFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_TEAMS_BTN);
    }

    if (fShowMilitia == TRUE) {
      fShowMilitia = FALSE;
      MapBorderButtonOff(MAP_BORDER_MILITIA_BTN);
    }

    if (fShowAircraftFlag == TRUE) {
      fShowAircraftFlag = FALSE;
      MapBorderButtonOff(MAP_BORDER_AIRSPACE_BTN);
    }

    if ((bSelectedDestChar != -1) || (fPlotForHelicopter == TRUE)) {
      AbortMovementPlottingMode();
    }

    // dirty regions
    MarkForRedrawalStrategicMap();
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  }
}

/*
void UpdateLevelButtonStates( void )
{

        if( iCurrentMapSectorZ == 0 )
        {
                DisableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_RAISE_LEVEL ] );
        }
        else
        {
                EnableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_RAISE_LEVEL ] );
        }

        if( iCurrentMapSectorZ == 3 )
        {
                DisableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_LOWER_LEVEL ] );
        }
        else
        {
                EnableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_LOWER_LEVEL ] );
        }

        return;
}
*/

/*
void UpdateScrollButtonStatesWhileScrolling( void )
{
        // too far west, disable
        if ( iZoomY == NORTH_ZOOM_BOUND )
        {
                ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_UP ]
]->uiFlags&=~(BUTTON_CLICKED_ON); DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_UP ] );
        }
        else if(iZoomY == SOUTH_ZOOM_BOUND )
        {
                ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_DWN ]
]->uiFlags&=~(BUTTON_CLICKED_ON); DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_DWN ] );
        }

        // too far west, disable
        if ( iZoomX == WEST_ZOOM_BOUND )
        {
                ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_LEFT ]
]->uiFlags&=~(BUTTON_CLICKED_ON); DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_LEFT ]
);
        }
        else if(iZoomX == EAST_ZOOM_BOUND )
        {
                ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_RIGHT ]
]->uiFlags&=~(BUTTON_CLICKED_ON); DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_RIGHT ]
);
        }

}
*/

void InitializeMapBorderButtonStates(void) {
  if (fShowItemsFlag) {
    MapBorderButtonOn(MAP_BORDER_ITEM_BTN);
  } else {
    MapBorderButtonOff(MAP_BORDER_ITEM_BTN);
  }

  if (fShowTownFlag) {
    MapBorderButtonOn(MAP_BORDER_TOWN_BTN);
  } else {
    MapBorderButtonOff(MAP_BORDER_TOWN_BTN);
  }

  if (fShowMineFlag) {
    MapBorderButtonOn(MAP_BORDER_MINE_BTN);
  } else {
    MapBorderButtonOff(MAP_BORDER_MINE_BTN);
  }

  if (fShowTeamFlag) {
    MapBorderButtonOn(MAP_BORDER_TEAMS_BTN);
  } else {
    MapBorderButtonOff(MAP_BORDER_TEAMS_BTN);
  }

  if (fShowAircraftFlag) {
    MapBorderButtonOn(MAP_BORDER_AIRSPACE_BTN);
  } else {
    MapBorderButtonOff(MAP_BORDER_AIRSPACE_BTN);
  }

  if (fShowMilitia) {
    MapBorderButtonOn(MAP_BORDER_MILITIA_BTN);
  } else {
    MapBorderButtonOff(MAP_BORDER_MILITIA_BTN);
  }
}

void CommonBtnCallbackBtnDownChecks(void) {
  if (IsMapScreenHelpTextUp()) {
    // stop mapscreen text
    StopMapScreenHelpText();
  }

  // any click cancels MAP UI messages, unless we're in confirm map move mode
  if ((giUIMessageOverlay != -1) && !gfInConfirmMapMoveMode) {
    CancelMapUIMessage();
  }
}

void InitMapScreenFlags(void) {
  fShowTownFlag = TRUE;
  fShowMineFlag = FALSE;

  fShowTeamFlag = TRUE;
  fShowMilitia = FALSE;

  fShowAircraftFlag = FALSE;
  fShowItemsFlag = FALSE;
}

void MapBorderButtonOff(uint8_t ubBorderButtonIndex) {
  Assert(ubBorderButtonIndex < 6);

  if (fShowMapInventoryPool) {
    return;
  }

  // if button doesn't exist, return
  if (giMapBorderButtons[ubBorderButtonIndex] == -1) {
    return;
  }

  Assert(giMapBorderButtons[ubBorderButtonIndex] < MAX_BUTTONS);

  ButtonList[giMapBorderButtons[ubBorderButtonIndex]]->uiFlags &= ~(BUTTON_CLICKED_ON);
}

void MapBorderButtonOn(uint8_t ubBorderButtonIndex) {
  Assert(ubBorderButtonIndex < 6);

  if (fShowMapInventoryPool) {
    return;
  }

  // if button doesn't exist, return
  if (giMapBorderButtons[ubBorderButtonIndex] == -1) {
    return;
  }

  Assert(giMapBorderButtons[ubBorderButtonIndex] < MAX_BUTTONS);

  ButtonList[giMapBorderButtons[ubBorderButtonIndex]]->uiFlags |= BUTTON_CLICKED_ON;
}
