#ifndef __LOADING_SCREEN_H
#define __LOADING_SCREEN_H

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
extern UINT8 gubLastLoadingScreenID;

// returns the UINT8 ID for the specified sector.
UINT8 GetLoadScreenID(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);

// sets up the loadscreen with specified ID, and draws it to the FRAME_BUFFER,
// and refreshing the screen with it.
void DisplayLoadScreenWithID(UINT8 ubLoadScreenID);

#endif
