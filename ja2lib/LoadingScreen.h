// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __LOADING_SCREEN_H
#define __LOADING_SCREEN_H

#include "SGP/Types.h"

enum {
  LOADINGSCREEN_NOTHING,
  LOADINGSCREEN_DAYGENERIC,
  LOADINGSCREEN_DAYTOWN1,
  LOADINGSCREEN_DAYTOWN2,
  LOADINGSCREEN_DAYWILD,
  LOADINGSCREEN_DAYTROPICAL,
  LOADINGSCREEN_DAYFOREST,
  LOADINGSCREEN_DAYDESERT,
  LOADINGSCREEN_DAYPALACE,
  LOADINGSCREEN_NIGHTGENERIC,
  LOADINGSCREEN_NIGHTWILD,
  LOADINGSCREEN_NIGHTTOWN1,
  LOADINGSCREEN_NIGHTTOWN2,
  LOADINGSCREEN_NIGHTFOREST,
  LOADINGSCREEN_NIGHTTROPICAL,
  LOADINGSCREEN_NIGHTDESERT,
  LOADINGSCREEN_NIGHTPALACE,
  LOADINGSCREEN_HELI,
  LOADINGSCREEN_BASEMENT,
  LOADINGSCREEN_MINE,
  LOADINGSCREEN_CAVE,
  LOADINGSCREEN_DAYPINE,
  LOADINGSCREEN_NIGHTPINE,
  LOADINGSCREEN_DAYMILITARY,
  LOADINGSCREEN_NIGHTMILITARY,
  LOADINGSCREEN_DAYSAM,
  LOADINGSCREEN_NIGHTSAM,
  LOADINGSCREEN_DAYPRISON,
  LOADINGSCREEN_NIGHTPRISON,
  LOADINGSCREEN_DAYHOSPITAL,
  LOADINGSCREEN_NIGHTHOSPITAL,
  LOADINGSCREEN_DAYAIRPORT,
  LOADINGSCREEN_NIGHTAIRPORT,
  LOADINGSCREEN_DAYLAB,
  LOADINGSCREEN_NIGHTLAB,
  LOADINGSCREEN_DAYOMERTA,
  LOADINGSCREEN_NIGHTOMERTA,
  LOADINGSCREEN_DAYCHITZENA,
  LOADINGSCREEN_NIGHTCHITZENA,
  LOADINGSCREEN_DAYMINE,
  LOADINGSCREEN_NIGHTMINE,
  LOADINGSCREEN_DAYBALIME,
  LOADINGSCREEN_NIGHTBALIME,
};

// For use by the game loader, before it can possibly know the situation.
extern uint8_t gubLastLoadingScreenID;

// returns the uint8_t ID for the specified sector.
uint8_t GetLoadScreenID(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

// sets up the loadscreen with specified ID, and draws it to the FRAME_BUFFER,
// and refreshing the screen with it.
void DisplayLoadScreenWithID(uint8_t ubLoadScreenID);

#endif
