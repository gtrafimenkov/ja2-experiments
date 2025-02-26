// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __AMBIENT_CONTROL
#define __AMBIENT_CONTROL

#include "SGP/Types.h"
#include "TileEngine/AmbientTypes.h"

BOOLEAN LoadAmbientControlFile(uint8_t ubAmbientID);

void HandleNewSectorAmbience(uint8_t ubAmbientID);
uint32_t SetupNewAmbientSound(uint32_t uiAmbientID);

void StopAmbients();
void DeleteAllAmbients();

extern AMBIENTDATA_STRUCT gAmbData[MAX_AMBIENT_SOUNDS];
extern int16_t gsNumAmbData;

BOOLEAN SetSteadyStateAmbience(uint8_t ubAmbience);

#define SOUND_NAME_SIZE 256
#define NUM_SOUNDS_PER_TIMEFRAME 8

enum {
  SSA_NONE,
  SSA_COUNTRYSIZE,
  SSA_NEAR_WATER,
  SSA_IN_WATER,
  SSA_HEAVY_FOREST,
  SSA_PINE_FOREST,
  SSA_ABANDONED,
  SSA_AIRPORT,
  SSA_WASTELAND,
  SSA_UNDERGROUND,
  SSA_OCEAN,
  NUM_STEADY_STATE_AMBIENCES
};

typedef struct {
  char zSoundNames[NUM_SOUNDS_PER_TIMEFRAME][SOUND_NAME_SIZE];

} STEADY_STATE_AMBIENCE;

#endif
