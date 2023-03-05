#include "Sector.h"

#include "Strategic/QueenCommand.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Overhead.h"

// Get SectorID8 from 0..15 coordinates.
SectorID8 SectorFrom015(u8 x, u8 y) { return x + y * MAP_WORLD_X; }

SectorID8 GetSectorID8(u8 x, u8 y) { return (y - 1) * 16 + x - 1; }

struct SectorInfo* GetSectorInfoByIndex(SectorID8 sectorIndex) { return &SectorInfo[sectorIndex]; }

BOOLEAN SectorOursAndPeaceful(INT16 sMapX, INT16 sMapY, INT8 bMapZ) {
  // if this sector is currently loaded
  if ((sMapX == gWorldSectorX) && (sMapY == gWorldSectorY) && (bMapZ == gbWorldSectorZ)) {
    // and either there are enemies prowling this sector, or combat is in progress
    if (gTacticalStatus.fEnemyInSector || (gTacticalStatus.uiFlags & INCOMBAT)) {
      return FALSE;
    }
  }

  // if sector is controlled by enemies, it's not ours (duh!)
  if (!bMapZ && IsSectorEnemyControlled(sMapX, sMapY)) {
    return FALSE;
  }

  if (NumHostilesInSector(sMapX, sMapY, bMapZ)) {
    return FALSE;
  }

  // safe & secure, s'far as we can tell
  return (TRUE);
}

BOOLEAN IsThisSectorASAMSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ) {
  // is the sector above ground?
  if (bSectorZ != 0) {
    return (FALSE);
  }

  if ((SAM_1_X == sSectorX) && (SAM_1_Y == sSectorY)) {
    return (TRUE);
  } else if ((SAM_2_X == sSectorX) && (SAM_2_Y == sSectorY)) {
    return (TRUE);
  } else if ((SAM_3_X == sSectorX) && (SAM_3_Y == sSectorY)) {
    return (TRUE);
  } else if ((SAM_4_X == sSectorX) && (SAM_4_Y == sSectorY)) {
    return (TRUE);
  }

  return (FALSE);
}

i16 GetLoadedSectorX() { return gWorldSectorX; }
i16 GetLoadedSectorY() { return gWorldSectorY; }

bool IsSectorEnemyControlled(i16 sMapX, i16 sMapY) {
  return StrategicMap[CALCULATE_STRATEGIC_INDEX(sMapX, sMapY)].fEnemyControlled;
}

// fEnemyControlled
