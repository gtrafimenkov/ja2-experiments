#ifndef __SECTOR_H
#define __SECTOR_H

#include "Militia.h"
#include "SGP/Types.h"

// Sector ID 0-255 (16x16)
typedef u8 SectorID8;

// Sector ID 0-324 (18x18)
typedef i16 SectorID16;

// Convert coordinates ([1-16], [1-16]) to 0-255 index.
// This function should be prefered over GetSectorID8_STATIC macro.
SectorID8 GetSectorID8(u8 x, u8 y);
#define GetSectorID8_STATIC(x, y) ((y - 1) * 16 + x - 1)
// Get X [1-16] from SectorID8
u8 SectorID8_X(SectorID8 sectorID);
// Get Y [1-16] from SectorID8
u8 SectorID8_Y(SectorID8 sectorID);

#define MAP_WORLD_X 18
#define MAP_WORLD_Y 18

// Convert coordinates (1-16, 1-16) to 0-324 index.
SectorID16 GetSectorID16(u8 x, u8 y);
// Get X [1-16] from SectorID16
u8 SectorID16_X(SectorID16 sectorID);
// Get Y [1-16] from SectorID16
u8 SectorID16_Y(SectorID16 sectorID);

SectorID16 SectorID8To16(SectorID8 sectorID);
SectorID8 SectorID16To8(SectorID16 sectorID);

struct SectorInfo;

struct SectorInfo* GetSectorInfoByID8(SectorID8 sectorIndex);
struct SectorInfo* GetSectorInfoByXY(u8 x, u8 y);

typedef struct SectorInfo SECTORINFO;

struct SectorInfo {
  // information pertaining to this sector
  UINT32 uiFlags;              // various special conditions
  UINT8 ubInvestigativeState;  // When the sector is attacked by the player, the state increases by
                               // 1 permanently. This value determines how quickly it is
                               // investigated by the enemy.
  UINT8 ubGarrisonID;  // IF the sector has an ID for this (non 255), then the queen values this
                       // sector and it indexes the garrison group.
  INT8 ubPendingReinforcements;  // when the enemy owns this sector, this value will keep track of
                                 // HIGH priority reinforcements -- not regular.
  BOOLEAN fMilitiaTrainingPaid;
  UINT8 ubMilitiaTrainingPercentDone;
  UINT8 ubMilitiaTrainingHundredths;
  // enemy military presence
  BOOLEAN fPlayer[4];  // whether the player THINKS the sector is unde his control or not. array is
                       // for sublevels
  // enemy only info
  UINT8 ubNumTroops;     // the actual number of troops here.
  UINT8 ubNumElites;     // the actual number of elites here.
  UINT8 ubNumAdmins;     // the actual number of admins here.
  UINT8 ubNumCreatures;  // only set when immediately before ground attack made!
  UINT8 ubTroopsInBattle, ubElitesInBattle, ubAdminsInBattle, ubCreaturesInBattle;

  INT8 bLastKnownEnemies;  // -1 means never been there, no idea, otherwise it's what we'd observed
                           // most recently while this is being maintained (partially, surely
                           // buggy), nothing uses it anymore. ARM

  UINT32 ubDayOfLastCreatureAttack;
  UINT32 uiFacilitiesFlags;  // the flags for various facilities

  UINT8 ubTraversability[5];  // determines the traversability ratings to adjacent sectors.
                              // The last index represents the traversability if travelling
                              // throught the sector without entering it.
  INT8 townID;
  INT8 bUSUSED;
  INT8 bBloodCats;
  INT8 bBloodCatPlacements;
  INT8 UNUSEDbSAMCondition;

  UINT8 ubTravelRating;  // Represents how travelled a sector is.  Typically, the higher the travel
                         // rating, the more people go near it.  A travel rating of 0 means there
                         // are never people around.  This value is used for determining how often
                         // items would "vanish" from a sector (nice theory, except it isn't being
                         // used that way.  Stealing is only in towns.  ARM)
  UINT8 ubNumberOfCivsAtLevel[MAX_MILITIA_LEVELS];  // town militia per experience class, 0/1/2 is
                                                    // GREEN/REGULAR/ELITE
  UINT16 usUNUSEDMilitiaLevels;                     // unused (ARM)
  UINT8 ubUNUSEDNumberOfJoeBlowCivilians;           // unused (ARM)
  UINT32 uiTimeCurrentSectorWasLastLoaded;  // Specifies the last time the player was in the sector
  UINT8 ubUNUSEDNumberOfEnemiesThoughtToBeHere;  // using bLastKnownEnemies instead
  UINT32 uiTimeLastPlayerLiberated;  // in game seconds (used to prevent the queen from attacking
                                     // for awhile)

  BOOLEAN fSurfaceWasEverPlayerControlled;

  UINT8 bFiller1;
  UINT8 bFiller2;
  UINT8 bFiller3;

  UINT32 uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;

  INT8 bPadding[41];
};

// Counts enemies and crepitus, but not bloodcats.
UINT8 NumHostilesInSector(INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ);

// Returns TRUE if sector is under player control, has no enemies in it, and isn't currently in
// combat mode
BOOLEAN SectorOursAndPeaceful(INT16 sMapX, INT16 sMapY, INT8 bMapZ);

BOOLEAN IsThisSectorASAMSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, CHAR16* zString,
                       size_t bufSize, BOOLEAN fDetailed);

i16 GetLoadedSectorX();
i16 GetLoadedSectorY();

bool IsSectorEnemyControlled(i8 sMapX, i8 sMapY);

#endif
