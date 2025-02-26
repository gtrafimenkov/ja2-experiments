// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __MAP_INTERFACE_BORDER_H
#define __MAP_INTERFACE_BORDER_H

#include "SGP/Types.h"

#define MAP_BORDER_START_X 261
#define MAP_BORDER_START_Y 0

// scroll directions
enum {
  ZOOM_MAP_SCROLL_UP = 0,
  ZOOM_MAP_SCROLL_DWN,
  ZOOM_MAP_SCROLL_RIGHT,
  ZOOM_MAP_SCROLL_LEFT,
};

enum {
  EAST_DIR = 0,
  WEST_DIR,
  NORTH_DIR,
  SOUTH_DIR,
};
enum {
  MAP_BORDER_TOWN_BTN = 0,
  MAP_BORDER_MINE_BTN,
  MAP_BORDER_TEAMS_BTN,
  MAP_BORDER_AIRSPACE_BTN,
  MAP_BORDER_ITEM_BTN,
  MAP_BORDER_MILITIA_BTN,
};

/*
enum{
        MAP_BORDER_RAISE_LEVEL=0,
        MAP_BORDER_LOWER_LEVEL,
};
*/

#define MAP_LEVEL_MARKER_X 565
#define MAP_LEVEL_MARKER_Y 323
#define MAP_LEVEL_MARKER_DELTA 8
#define MAP_LEVEL_MARKER_WIDTH (620 - MAP_LEVEL_MARKER_X)

extern BOOLEAN fShowTownFlag;
extern BOOLEAN fShowMineFlag;
extern BOOLEAN fShowTeamFlag;
extern BOOLEAN fShowMilitia;
extern BOOLEAN fShowAircraftFlag;
extern BOOLEAN fShowItemsFlag;

// scroll animation
extern int32_t giScrollButtonState;

BOOLEAN LoadMapBorderGraphics(void);
void DeleteMapBorderGraphics(void);
void RenderMapBorder(void);
// void RenderMapBorderCorner( void );
// void ShowDestinationOfPlottedPath( wchar_t* pLoc );
// void ResetAircraftButton( void );
// void HandleMapScrollButtonStates( void );

void ToggleShowTownsMode(void);
void ToggleShowMinesMode(void);
void ToggleShowMilitiaMode(void);
void ToggleShowTeamsMode(void);
void ToggleAirspaceMode(void);
void ToggleItemsFilter(void);

void TurnOnShowTeamsMode(void);
void TurnOnAirSpaceMode(void);
void TurnOnItemFilterMode(void);

/*
// enable disable map border
void DisableMapBorderRegion( void );
void EnableMapBorderRegion( void );
*/

// create/destroy buttons for map border region
void DeleteMapBorderButtons(void);
BOOLEAN CreateButtonsForMapBorder(void);

// render the pop up for eta  in path plotting in map screen
void RenderMapBorderEtaPopUp(void);

// void UpdateLevelButtonStates( void );

// create mouse regions for level markers
void CreateMouseRegionsForLevelMarkers(void);
void DeleteMouseRegionsForLevelMarkers(void);

void InitMapScreenFlags(void);

void MapBorderButtonOff(uint8_t ubBorderButtonIndex);
void MapBorderButtonOn(uint8_t ubBorderButtonIndex);

#endif
