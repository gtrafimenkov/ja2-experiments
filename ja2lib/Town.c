#include "Town.h"

#include "Strategic/Strategic.h"

static TownSectors allTownSectors;

const TownSectors* GetAllTownSectors() { return &allTownSectors; }

void BuildListOfTownSectors(void) {
  for (int i = 0; i < ARR_SIZE(allTownSectors); i++) {
    allTownSectors[i].townID = BLANK_SECTOR;
  }

  INT32 iCounter = 0;
  for (INT32 iCounterX = 0; iCounterX < MAP_WORLD_X; iCounterX++) {
    for (INT32 iCounterY = 0; iCounterY < MAP_WORLD_Y; iCounterY++) {
      UINT16 usSector = iCounterX + iCounterY * MAP_WORLD_X;

      if ((StrategicMap[usSector].bNameId >= FIRST_TOWN) &&
          (StrategicMap[usSector].bNameId < NUM_TOWNS)) {
        allTownSectors[iCounter].townID = StrategicMap[usSector].bNameId;
        allTownSectors[iCounter].sectorID = usSector;
        iCounter++;
      }
    }
  }
}
