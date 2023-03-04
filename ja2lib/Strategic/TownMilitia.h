#ifndef _TOWN_MILITIA_H
#define _TOWN_MILITIA_H

// header for town militia strategic control module

#include "SGP/Types.h"

struct SOLDIERTYPE;

// how many militia of all ranks can be in any one sector at once
#define MAX_ALLOWABLE_MILITIA_PER_SECTOR 20

// how many new green militia civilians are trained at a time
#define MILITIA_TRAINING_SQUAD_SIZE 10  // was 6

// cost of starting a new militia training assignment
#define MILITIA_TRAINING_COST 750

// minimum loyalty rating before training is allowed in a town
#define MIN_RATING_TO_TRAIN_TOWN 20

// this handles what happens when a new militia unit is finishes getting trained
void TownMilitiaTrainingCompleted(struct SOLDIERTYPE *pTrainer, INT16 sMapX, INT16 sMapY);

// feed this a SOLDIER_CLASS_, it will return you a _MITILIA rank, or -1 if the guy's not militia
INT8 SoldierClassToMilitiaRank(UINT8 ubSoldierClass);
// feed this a _MITILIA rank, it will return you a SOLDIER_CLASS_, or -1 if the guy's not militia
INT8 MilitiaRankToSoldierClass(UINT8 ubRank);

// these add, promote, and remove militias of a certain rank
void StrategicAddMilitiaToSector(INT16 sMapX, INT16 sMapY, UINT8 ubRank, UINT8 ubHowMany);
void StrategicPromoteMilitiaInSector(INT16 sMapX, INT16 sMapY, UINT8 ubCurrentRank,
                                     UINT8 ubHowMany);
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
BOOLEAN IsTownFullMilitia(INT8 bTownId);
// is the SAM site full of militia?
BOOLEAN IsSAMSiteFullOfMilitia(INT16 sSectorX, INT16 sSectorY);

// now that town training is complete, handle the continue boxes
void HandleContinueOfTownTraining(void);

// handle completion of assignment byt his soldier too and inform the player
void HandleCompletionOfTownTrainingByGroupWithTrainer(struct SOLDIERTYPE *pTrainer);

// clear the list of training completed sectors
void ClearSectorListForCompletedTrainingOfMilitia(void);

BOOLEAN MilitiaTrainingAllowedInSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
BOOLEAN MilitiaTrainingAllowedInTown(INT8 bTownId);

#endif
