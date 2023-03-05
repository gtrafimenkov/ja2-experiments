#ifndef __TOWN_H
#define __TOWN_H

#include "LeanTypes.h"

// Sector name identifiers.  Also town names.
typedef enum {
  BLANK_SECTOR = 0,
  OMERTA,
  DRASSEN,
  ALMA,
  GRUMM,
  TIXA,
  CAMBRIA,
  SAN_MONA,
  ESTONI,
  ORTA,
  BALIME,
  MEDUNA,
  CHITZENA,
  NUM_TOWNS
} TownID;

i8 GetTownIdForSector(i16 sMapX, i16 sMapY);
i8 GetTownIdForStrategicMapIndex(i32 index);

#endif
