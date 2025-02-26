// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __STRATMAP_H
#define __STRATMAP_H

#include "SGP/Types.h"
#include "Town.h"

struct SOLDIERTYPE;

// The maximum size for any team strategically speaking.  For example, we can't have more than 20
// enemies, militia, or creatures at a time.
#define MAX_STRATEGIC_TEAM_SIZE 20

// Codes for jumoing into adjacent sectors..
enum { JUMP_ALL_LOAD_NEW, JUMP_ALL_NO_LOAD, JUMP_SINGLE_LOAD_NEW, JUMP_SINGLE_NO_LOAD };

enum {
  CONTROLLED = 0,
  UNCONTROLLED,
};

// For speed, etc lets make these globals, forget the functions if you want
extern int16_t gWorldSectorX;
extern int16_t gWorldSectorY;
extern int8_t gbWorldSectorZ;

#define NUMBER_OF_SAMS 4

extern uint8_t pSamList[NUMBER_OF_SAMS];
extern int16_t pSamGridNoAList[NUMBER_OF_SAMS];
extern int16_t pSamGridNoBList[NUMBER_OF_SAMS];

extern BOOLEAN fFoundOrta;
extern BOOLEAN fSamSiteFound[NUMBER_OF_SAMS];

extern BOOLEAN gfUseAlternateMap;

// SAM sites
#define SAM_1_X 2
#define SAM_2_X 15
#define SAM_3_X 8
#define SAM_4_X 4

#define SAM_1_Y 4
#define SAM_2_Y 4
#define SAM_3_Y 9
#define SAM_4_Y 14

// min condition for sam site to be functional
#define MIN_CONDITION_FOR_SAM_SITE_TO_WORK 80

// FUNCTIONS FOR DERTERMINING GOOD GetSectorID8 EXIT DATA
#define CHECK_DIR_X_DELTA (WORLD_TILE_X * 4)
#define CHECK_DIR_Y_DELTA (WORLD_TILE_Y * 10)

void GetCurrentWorldSector(uint8_t *psMapX, uint8_t *psMapY);
BOOLEAN SetCurrentWorldSector(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);

BOOLEAN EnterSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);
void UpdateMercsInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);
void UpdateMercInSector(struct SOLDIERTYPE *pSoldier, uint8_t sSectorX, uint8_t sSectorY,
                        int8_t bSectorZ);

void InitializeMapStructure();

// get short sector name without town name
void GetShortSectorString(uint8_t sMapX, uint8_t sMapY, wchar_t *sString, size_t bufSize);

// NEW!
// Calculates the name of the sector based on the loaded sector values.
// Examples:		A9
//						A10_B1
//						J9_B2_A ( >= BETAVERSION ) else J9_B2 (release
// equivalent)
void GetLoadedSectorString(wchar_t *pString, size_t bufSize);

void GetMapFileName(uint8_t sMapX, uint8_t sMapY, int8_t bSectorZ, char *bString,
                    BOOLEAN fUsePlaceholder, BOOLEAN fAddAlternateMapLetter);

// Called from within tactical.....
void JumpIntoAdjacentSector(uint8_t ubDirection, uint8_t ubJumpCode, int16_t sAdditionalData);

BOOLEAN CanGoToTacticalInSector(int16_t sX, int16_t sY, uint8_t ubZ);

void UpdateAirspaceControl(void);

// init sam sites
void InitializeSAMSites(void);

// get town sector size
uint8_t GetTownSectorSize(TownID bTownId);

// get town sector size, that is under play control
uint8_t GetTownSectorsUnderControl(TownID bTownId);

// is this sector part of this town
BOOLEAN SectorIsPartOfTown(TownID bTownId, uint8_t sSectorX, uint8_t sSectorY);

// BOOLEAN IsThereAnyOneInThisTown( uint8_t ubTownId );

BOOLEAN SoldierOKForSectorExit(struct SOLDIERTYPE *pSoldier, int8_t bExitDirection,
                               uint16_t usAdditionalData);
BOOLEAN OKForSectorExit(int8_t bExitDirection, uint16_t usAdditionalData,
                        uint32_t *puiTraverseTimeInMinutes);
void SetupNewStrategicGame();

BOOLEAN LoadStrategicInfoFromSavedFile(HWFILE hFile);
BOOLEAN SaveStrategicInfoToSavedFile(HWFILE hFile);

void AllMercsHaveWalkedOffSector();

void AdjustSoldierPathToGoOffEdge(struct SOLDIERTYPE *pSoldier, int16_t sEndGridNo,
                                  uint8_t ubTacticalDirection);

void AllMercsWalkedToExitGrid();

void PrepareLoadedSector();

// handle for slay...no better place to really put this stuff
void HandleSlayDailyEvent(void);

void HandleQuestCodeOnSectorEntry(uint8_t sNewSectorX, uint8_t sNewSectorY, int8_t bNewSectorZ);

// handle a soldier leaving thier squad behind, this sets them up for mvt and potential rejoining of
// group
void HandleSoldierLeavingSectorByThemSelf(struct SOLDIERTYPE *pSoldier);

BOOLEAN CheckAndHandleUnloadingOfCurrentWorld();

// number of SAM sites under player control
int32_t GetNumberOfSAMSitesUnderPlayerControl(void);

// is there a FUNCTIONAL SAM site in this sector?
BOOLEAN IsThereAFunctionalSAMSiteInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

BOOLEAN IsSectorDesert(uint8_t sSectorX, uint8_t sSectorY);

// sam site under players control?
int32_t SAMSitesUnderPlayerControl(uint8_t sX, uint8_t sY);

int8_t GetSAMIdFromSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

void SetupProfileInsertionDataForSoldier(struct SOLDIERTYPE *pSoldier);

BOOLEAN HandlePotentialBringUpAutoresolveToFinishBattle();

// Used for determining the type of error message that comes up when you can't traverse to
// an adjacent sector.  THESE VALUES DO NOT NEED TO BE SAVED!
extern BOOLEAN gfInvalidTraversal;
extern BOOLEAN gfLoneEPCAttemptingTraversal;
extern BOOLEAN gfRobotWithoutControllerAttemptingTraversal;
extern uint8_t gubLoneMercAttemptingToAbandonEPCs;
extern int8_t gbPotentiallyAbandonedEPCSlotID;

#endif
