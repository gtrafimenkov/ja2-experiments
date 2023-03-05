#ifndef _TOWN_MILITIA_H
#define _TOWN_MILITIA_H

// header for town militia strategic control module

#include "SGP/Types.h"
#include "Town.h"

struct SOLDIERTYPE;

// how many militia of all ranks can be in any one sector at once
#define MAX_ALLOWABLE_MILITIA_PER_SECTOR 20

// minimum loyalty rating before training is allowed in a town
#define MIN_RATING_TO_TRAIN_TOWN 20

// this handles what happens when a new militia unit is finishes getting trained
void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, INT16 sMapX, INT16 sMapY);

void StrategicRemoveMilitiaFromSector(INT16 sMapX, INT16 sMapY, UINT8 ubRank, UINT8 ubHowMany);

// this will check for promotions and handle them for you
UINT8 CheckOneMilitiaForPromotion(INT16 sMapX, INT16 sMapY, UINT8 ubCurrentRank,
                                  UINT8 ubRecentKillPts);

void BuildMilitiaPromotionsString(CHAR16 *str, size_t bufSize);

UINT8 CountAllMilitiaInSector(INT16 sMapX, INT16 sMapY);
UINT8 MilitiaInSectorOfRank(INT16 sMapX, INT16 sMapY, UINT8 ubRank);

// tell player how much it will cost
void HandleInterfaceMessageForCostOfTrainingMilitia(struct SOLDIERTYPE *pSoldier);

// continue training?
void HandleInterfaceMessageForContinuingTrainingMilitia(struct SOLDIERTYPE *pSoldier);

// call this when the sector changes...
void HandleMilitiaStatusInCurrentMapBeforeLoadingNewMap(void);

// is there a town with militia here or nearby?
BOOLEAN CanNearbyMilitiaScoutThisSector(INT16 sSectorX, INT16 sSectorY);

// is the town militia full?
BOOLEAN IsTownFullMilitia(TownID bTownId);
// is the SAM site full of militia?
BOOLEAN IsSAMSiteFullOfMilitia(INT16 sSectorX, INT16 sSectorY);

// now that town training is complete, handle the continue boxes
void HandleContinueOfTownTraining(void);

// clear the list of training completed sectors
void ClearSectorListForCompletedTrainingOfMilitia(void);

BOOLEAN MilitiaTrainingAllowedInSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
BOOLEAN MilitiaTrainingAllowedInTown(TownID bTownId);

void PrepMilitiaPromotion();
void HandleSingleMilitiaPromotion(i16 sMapX, i16 sMapY, u8 soldierClass, u8 kills);
bool HasNewMilitiaPromotions();

#endif
