// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __MAP_INTERFACE_BOTTOM
#define __MAP_INTERFACE_BOTTOM

#include "SGP/Types.h"

#define MAX_MESSAGES_ON_MAP_BOTTOM 9

// exit to where defines
enum {
  MAP_EXIT_TO_LAPTOP = 0,
  MAP_EXIT_TO_TACTICAL,
  MAP_EXIT_TO_OPTIONS,
  MAP_EXIT_TO_LOAD,
  MAP_EXIT_TO_SAVE,
};

// there's no button for entering SAVE/LOAD screen directly...
extern uint32_t guiMapBottomExitButtons[3];

extern BOOLEAN fLapTop;
extern BOOLEAN fLeavingMapScreen;
extern BOOLEAN gfDontStartTransitionFromLaptop;
extern BOOLEAN gfStartMapScreenToLaptopTransition;

// function prototypes

BOOLEAN LoadMapScreenInterfaceBottom(void);
void DeleteMapScreenInterfaceBottom(void);
void DestroyButtonsForMapScreenInterfaceBottom(void);
BOOLEAN CreateButtonsForMapScreenInterfaceBottom(void);
void RenderMapScreenInterfaceBottom(void);

// delete map bottom graphics
void DeleteMapBottomGraphics(void);

// load bottom graphics
void HandleLoadOfMapBottomGraphics(void);

// allowed to time compress?
BOOLEAN AllowedToTimeCompress(void);

void EnableDisAbleMapScreenOptionsButton(BOOLEAN fEnable);

// create and destroy masks to cover the time compression buttons as needed
void CreateDestroyMouseRegionMasksForTimeCompressionButtons(void);

BOOLEAN CommonTimeCompressionChecks(void);

BOOLEAN AnyUsableRealMercenariesOnTeam(void);

void RequestTriggerExitFromMapscreen(int8_t bExitToWhere);
BOOLEAN AllowedToExitFromMapscreenTo(int8_t bExitToWhere);
void HandleExitsFromMapScreen(void);

void MapScreenMsgScrollDown(uint8_t ubLinesDown);
void MapScreenMsgScrollUp(uint8_t ubLinesUp);

void ChangeCurrentMapscreenMessageIndex(uint8_t ubNewMessageIndex);
void MoveToEndOfMapScreenMessageList(void);

#endif
