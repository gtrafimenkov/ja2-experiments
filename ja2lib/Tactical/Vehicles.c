// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/Vehicles.h"

#include "JAScreens.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/TileAnimation.h"
#include "UI.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"

int8_t gubVehicleMovementGroups[MAX_VEHICLES];

// the list of vehicles
VEHICLETYPE *pVehicleList = NULL;

// number of vehicle slots on the list
uint8_t ubNumberOfVehicles = 0;

// the sqaud mvt groups
extern int8_t SquadMovementGroups[];

// ATE: These arrays below should all be in a large LUT which contains
// static info for each vehicle....

// the mvt groups associated with vehcile types
int32_t iMvtTypes[] = {
    CAR,  // eldorado
    CAR,  // hummer
    CAR,  // ice cream truck
    CAR,  // jeep
    CAR,  // tank

    AIR,  // helicopter

};

int32_t iSeatingCapacities[] = {
    6,  // eldorado
    6,  // hummer
    6,  // ice cream truck
    6,  // jeep
    6,  // tank
    6,  // helicopter
};

int32_t iEnterVehicleSndID[] = {

    S_VECH1_INTO, S_VECH1_INTO, S_VECH1_INTO, S_VECH1_INTO, S_VECH1_INTO, S_VECH1_INTO,

};

int32_t iMoveVehicleSndID[] = {

    S_VECH1_MOVE, S_VECH1_MOVE, S_VECH1_MOVE, S_VECH1_MOVE, S_VECH1_MOVE, S_VECH1_MOVE,
};

uint8_t ubVehicleTypeProfileID[] = {

    PROF_ELDERODO, PROF_HUMMER, PROF_ICECREAM, NPC164, NPC164, PROF_HELICOPTER};

/*
// location of crits based on facing
int8_t bInternalCritHitsByLocation[ NUMBER_OF_EXTERNAL_HIT_LOCATIONS_ON_VEHICLE ][
NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ]={ { ENGINE_HIT_LOCATION, ENGINE_HIT_LOCATION,
CREW_COMPARTMENT_HIT_LOCATION,CREW_COMPARTMENT_HIT_LOCATION, RF_TIRE_HIT_LOCATION,
LF_TIRE_HIT_LOCATION }, // front { ENGINE_HIT_LOCATION, LF_TIRE_HIT_LOCATION,
CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, LR_TIRE_HIT_LOCATION,
GAS_TANK_HIT_LOCATION}, // left side { ENGINE_HIT_LOCATION, RF_TIRE_HIT_LOCATION,
CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, RR_TIRE_HIT_LOCATION,
GAS_TANK_HIT_LOCATION}, // right side { CREW_COMPARTMENT_HIT_LOCATION,
CREW_COMPARTMENT_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION, RR_TIRE_HIT_LOCATION,
LR_TIRE_HIT_LOCATION, GAS_TANK_HIT_LOCATION }, // rear { ENGINE_HIT_LOCATION, RF_TIRE_HIT_LOCATION,
LF_TIRE_HIT_LOCATION, RR_TIRE_HIT_LOCATION,LR_TIRE_HIT_LOCATION, GAS_TANK_HIT_LOCATION,}, // bottom
side { ENGINE_HIT_LOCATION, ENGINE_HIT_LOCATION, ENGINE_HIT_LOCATION, CREW_COMPARTMENT_HIT_LOCATION,
CREW_COMPARTMENT_HIT_LOCATION, GAS_TANK_HIT_LOCATION }, // top
};
*/

// original armor values for vehicles
/*
        ELDORADO_CAR = 0,
        HUMMER,
        ICE_CREAM_TRUCK,
        JEEP_CAR,
        TANK_CAR,
        HELICOPTER,
*/

int16_t sVehicleArmourType[NUMBER_OF_TYPES_OF_VEHICLES] = {
    KEVLAR_VEST,   // El Dorado
    SPECTRA_VEST,  // Hummer
    KEVLAR_VEST,   // Ice cream truck
    KEVLAR_VEST,   // Jeep
    SPECTRA_VEST,  // Tank - do we want this?
    KEVLAR_VEST,   // Helicopter
};

/*
int16_t sVehicleExternalOrigArmorValues[ NUMBER_OF_TYPES_OF_VEHICLES ][
NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ]={ { 100,100,100,100,100,100 }, // helicopter {
500,500,500,500,500,500 }, // hummer
};
*/

/*
// external armor values
int16_t sVehicleInternalOrigArmorValues[ NUMBER_OF_TYPES_OF_VEHICLES ][
NUMBER_OF_INTERNAL_HIT_LOCATIONS_IN_VEHICLE ]={ { 250,250,250,250,250,250 }, // eldorado {
250,250,250,250,250,250 }, // hummer { 250,250,250,250,250,250 }, // ice cream {
250,250,250,250,250,250 }, // feep { 850,850,850,850,850,850 }, // tank { 50,50,50,50,50,50 }, //
helicopter
};
*/

// ap cost per crit
#define COST_PER_ENGINE_CRIT 15
#define COST_PER_TIRE_HIT 5
// #define VEHICLE_MAX_INTERNAL 250

// set the driver of the vehicle
void SetDriver(int32_t iID, uint8_t ubID);

// void RemoveSoldierFromVehicleBetweenSectors( pSoldier, iId );

void TeleportVehicleToItsClosestSector(int32_t iVehicleId, uint8_t ubGroupID);

// Loop through and create a few soldier squad ID's for vehicles ( max # 3 )
void InitVehicles() {
  int32_t cnt;
  struct GROUP *pGroup = NULL;

  for (cnt = 0; cnt < MAX_VEHICLES; cnt++) {
    // create mvt groups
    gubVehicleMovementGroups[cnt] = CreateNewVehicleGroupDepartingFromSector(1, 1, cnt);

    // Set persistent....
    pGroup = GetGroup(gubVehicleMovementGroups[cnt]);
    pGroup->fPersistant = TRUE;
  }
}

void SetVehicleValuesIntoSoldierType(struct SOLDIERTYPE *pVehicle) {
  wcscpy(pVehicle->name, zVehicleName[pVehicleList[pVehicle->bVehicleID].ubVehicleType]);

  pVehicle->ubProfile = pVehicleList[pVehicle->bVehicleID].ubProfileID;

  // Init fuel!
  pVehicle->sBreathRed = 10000;
  pVehicle->bBreath = 100;

  pVehicle->ubWhatKindOfMercAmI = MERC_TYPE__VEHICLE;
}

int32_t AddVehicleToList(uint8_t sMapX, uint8_t sMapY, int16_t sGridNo, uint8_t ubType) {
  // insert this vehicle into the list
  // how many vehicles are there?
  int32_t iVehicleIdValue = -1;
  int32_t iCounter = 0, iCount = 0;
  VEHICLETYPE *pTempList = NULL;
  BOOLEAN fFoundEmpty = FALSE;
  struct GROUP *pGroup;

  if (pVehicleList != NULL) {
    // not the first, add to list
    for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
      // might have an empty slot
      if (pVehicleList[iCounter].fValid == FALSE) {
        iCount = iCounter;
        iCounter = ubNumberOfVehicles;
        fFoundEmpty = TRUE;
        iVehicleIdValue = iCount;
      }
    }
  }

  if (fFoundEmpty == FALSE) {
    iCount = ubNumberOfVehicles;
  }

  if (iCount == 0) {
    pVehicleList = (VEHICLETYPE *)MemAlloc(sizeof(VEHICLETYPE));

    // Set!
    memset(pVehicleList, 0, sizeof(VEHICLETYPE));

    ubNumberOfVehicles = 1;
    iVehicleIdValue = 0;
  }

  if ((iVehicleIdValue == -1) && (iCount != 0) && (fFoundEmpty == FALSE)) {
    // no empty slot found, need to realloc
    pTempList = (VEHICLETYPE *)MemAlloc(sizeof(VEHICLETYPE) * ubNumberOfVehicles);

    // copy to temp
    memcpy(pTempList, pVehicleList, sizeof(VEHICLETYPE) * ubNumberOfVehicles);

    // now realloc
    pVehicleList =
        (VEHICLETYPE *)MemRealloc(pVehicleList, (sizeof(VEHICLETYPE) * (ubNumberOfVehicles + 1)));

    // memset the stuff
    memset(pVehicleList, 0, (sizeof(VEHICLETYPE) * (ubNumberOfVehicles + 1)));

    // now copy the stuff back
    memcpy(pVehicleList, pTempList, sizeof(VEHICLETYPE) * (ubNumberOfVehicles));

    // now get rid of crap
    MemFree(pTempList);

    // now get the index value
    iVehicleIdValue = ubNumberOfVehicles;

    ubNumberOfVehicles++;
  }

  // found a slot
  pVehicleList[iCount].ubMovementGroup = 0;
  pVehicleList[iCount].sSectorX = sMapX;
  pVehicleList[iCount].sSectorY = sMapY;
  pVehicleList[iCount].sSectorZ = 0;
  pVehicleList[iCount].sGridNo = sGridNo;
  memset(pVehicleList[iCount].pPassengers, 0, 10 * sizeof(struct SOLDIERTYPE *));
  pVehicleList[iCount].fValid = TRUE;
  pVehicleList[iCount].ubVehicleType = ubType;
  pVehicleList[iCount].pMercPath = NULL;
  pVehicleList[iCount].fFunctional = TRUE;
  pVehicleList[iCount].fDestroyed = FALSE;
  pVehicleList[iCount].iMoveSound = iMoveVehicleSndID[ubType];
  pVehicleList[iCount].iOutOfSound = iEnterVehicleSndID[ubType];
  pVehicleList[iCount].ubProfileID = ubVehicleTypeProfileID[ubType];
  pVehicleList[iCount].ubMovementGroup = gubVehicleMovementGroups[iCount];

  // ATE: Add movement mask to group...
  pGroup = GetGroup(pVehicleList[iCount].ubMovementGroup);

  if (!pGroup) {
    if (gfEditMode) {
      // This is okay, no groups exist, so simply return.
      return iVehicleIdValue;
    }
    Assert(0);
  }

  pGroup->ubTransportationMask = (uint8_t)iMvtTypes[ubType];

  // ARM: setup group movement defaults
  pGroup->ubSectorX = (uint8_t)sMapX;
  pGroup->ubNextX = (uint8_t)sMapX;
  pGroup->ubSectorY = (uint8_t)sMapY;
  pGroup->ubNextY = (uint8_t)sMapY;
  pGroup->uiTraverseTime = 0;
  pGroup->uiArrivalTime = 0;

  SetUpArmorForVehicle((uint8_t)iCount);

  return (iVehicleIdValue);
}

BOOLEAN RemoveVehicleFromList(int32_t iId) {
  // remove this vehicle from the list

  // error check
  if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
    return (FALSE);
  }

  // clear remaining path nodes
  if (pVehicleList[iId].pMercPath != NULL) {
    pVehicleList[iId].pMercPath = ClearStrategicPathList(pVehicleList[iId].pMercPath, 0);
  }

  // zero out mem
  memset(&(pVehicleList[iId]), 0, sizeof(VEHICLETYPE));

  return (TRUE);
}

void ClearOutVehicleList(void) {
  int32_t iCounter;

  // empty out the vehicle list
  if (pVehicleList) {
    for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
      // if there is a valid vehicle
      if (pVehicleList[iCounter].fValid) {
        // if the vehicle has a valid path
        if (pVehicleList[iCounter].pMercPath) {
          // toast the vehicle path
          pVehicleList[iCounter].pMercPath =
              ClearStrategicPathList(pVehicleList[iCounter].pMercPath, 0);
        }
      }
    }

    MemFree(pVehicleList);
    pVehicleList = NULL;
    ubNumberOfVehicles = 0;
  }

  /*
          // empty out the vehicle list
          if( pVehicleList )
          {
                  MemFree( pVehicleList );
                  pVehicleList = NULL;
                  ubNumberOfVehicles = 0;
          }
  */
}

BOOLEAN IsThisVehicleAccessibleToSoldier(struct SOLDIERTYPE *pSoldier, int32_t iId) {
  if (pSoldier == NULL) {
    return (FALSE);
  }

  if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
    return (FALSE);
  }

  // now check if vehicle is valid
  if (pVehicleList[iId].fValid == FALSE) {
    return (FALSE);
  }

  // if the soldier or the vehicle is between sectors
  if (pSoldier->fBetweenSectors || pVehicleList[iId].fBetweenSectors) {
    return (FALSE);
  }

  // any sector values off?
  if ((GetSolSectorX(pSoldier) != pVehicleList[iId].sSectorX) ||
      (GetSolSectorY(pSoldier) != pVehicleList[iId].sSectorY) ||
      (GetSolSectorZ(pSoldier) != pVehicleList[iId].sSectorZ)) {
    return (FALSE);
  }

  // if vehicle is not ok to use then return false
  if (!OKUseVehicle(pVehicleList[iId].ubProfileID)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN AddSoldierToVehicle(struct SOLDIERTYPE *pSoldier, int32_t iId) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pVehicleSoldier = NULL;

  // Add Soldierto Vehicle
  if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
    return (FALSE);
  }

  // ok now check if any free slots in the vehicle

  // now check if vehicle is valid
  if (pVehicleList[iId].fValid == FALSE) {
    return (FALSE);
  }

  // get the vehicle soldiertype
  pVehicleSoldier = GetSoldierStructureForVehicle(iId);

  if (pVehicleSoldier) {
    if (pVehicleSoldier->bTeam != gbPlayerNum) {
      // Change sides...
      pVehicleSoldier = ChangeSoldierTeam(pVehicleSoldier, gbPlayerNum);
      // add it to mapscreen list
      fReBuildCharacterList = TRUE;
    }
  }

  // If vehicle is empty, add to unique squad now that it has somebody in it!
  if (GetNumberInVehicle(iId) == 0 && pVehicleSoldier) {
    // 2 ) Add to unique squad...
    AddCharacterToUniqueSquad(pVehicleSoldier);

    // ATE: OK funcky stuff here!
    // We have now a guy on a squad group, remove him!
    RemovePlayerFromGroup(SquadMovementGroups[pVehicleSoldier->bAssignment], pVehicleSoldier);

    // I really have vehicles.
    // ONLY add to vehicle group once!
    if (!DoesPlayerExistInPGroup(pVehicleList[iId].ubMovementGroup, pVehicleSoldier)) {
      // NOW.. add guy to vehicle group....
      AddPlayerToGroup(pVehicleList[iId].ubMovementGroup, pVehicleSoldier);
    } else {
      pVehicleSoldier->ubGroupID = pVehicleList[iId].ubMovementGroup;
    }
  }

  // check if the grunt is already here
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] == pSoldier) {
      // guy found, no need to add
      return (TRUE);
    }
  }

  if (pVehicleSoldier) {
    // can't call SelectSoldier in mapscreen, that will initialize interface panels!!!
    if (IsTacticalMode()) {
      SelectSoldier(pVehicleSoldier->ubID, FALSE, TRUE);
    }

    PlayJA2Sample(pVehicleList[pVehicleSoldier->bVehicleID].iOutOfSound, RATE_11025,
                  SoundVolume(HIGHVOLUME, pVehicleSoldier->sGridNo), 1,
                  SoundDir(pVehicleSoldier->sGridNo));
  }

  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    // check if slot free
    if (pVehicleList[iId].pPassengers[iCounter] == NULL) {
      // add person in
      pVehicleList[iId].pPassengers[iCounter] = pSoldier;

      if (GetSolAssignment(pSoldier) == VEHICLE) {
        TakeSoldierOutOfVehicle(pSoldier);
        // NOTE: This will leave the soldier on a squad.  Must be done PRIOR TO and in AS WELL AS
        // the call to RemoveCharacterFromSquads() that's coming up, to permit direct
        // vehicle->vehicle reassignment!
      }

      // if in a squad, remove from squad, if not, check if in vehicle, if so remove, if not, then
      // check if in mvt group..if so, move and destroy group
      if (pSoldier->bAssignment < ON_DUTY) {
        RemoveCharacterFromSquads(pSoldier);
      } else if (pSoldier->ubGroupID != 0) {
        // destroy group and set to zero
        RemoveGroup(pSoldier->ubGroupID);
        pSoldier->ubGroupID = 0;
      }

      if ((pSoldier->bAssignment != VEHICLE) || (pSoldier->iVehicleId != iId)) {
        SetTimeOfAssignmentChangeForMerc(pSoldier);
      }

      // set thier assignment
      ChangeSoldiersAssignment(pSoldier, VEHICLE);

      // set vehicle id
      pSoldier->iVehicleId = iId;

      // if vehicle is part of mvt group, then add character to mvt group
      if (pVehicleList[iId].ubMovementGroup != 0) {
        // add character
        AddPlayerToGroup(pVehicleList[iId].ubMovementGroup, pSoldier);
      }

      // Are we the first?
      if (GetNumberInVehicle(iId) == 1) {
        // Set as driver...
        pSoldier->uiStatusFlags |= SOLDIER_DRIVER;

        SetDriver(iId, GetSolID(pSoldier));

      } else {
        // Set as driver...
        pSoldier->uiStatusFlags |= SOLDIER_PASSENGER;
      }

      // Remove soldier's graphic
      RemoveSoldierFromGridNo(pSoldier);

      if (pVehicleSoldier) {
        // Set gridno for vehicle.....
        EVENT_SetSoldierPosition(pSoldier, pVehicleSoldier->dXPos, pVehicleSoldier->dYPos);

        // Stop from any movement.....
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

        // can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface
        // panels!!!
        if (IsTacticalMode()) {
          SetCurrentSquad(pVehicleSoldier->bAssignment, TRUE);
        }
      }

      return (TRUE);
    }
  }

  // no slots, leave
  return (FALSE);
}

void SetSoldierExitVehicleInsertionData(struct SOLDIERTYPE *pSoldier, int32_t iId) {
  if (iId == iHelicopterVehicleId && !pSoldier->bInSector) {
    if (GetSolSectorX(pSoldier) != BOBBYR_SHIPPING_DEST_SECTOR_X ||
        GetSolSectorY(pSoldier) != BOBBYR_SHIPPING_DEST_SECTOR_Y ||
        GetSolSectorZ(pSoldier) != BOBBYR_SHIPPING_DEST_SECTOR_Z) {
      // Not anything different here - just use center gridno......
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_CENTER;
    } else {
      // This is drassen, make insertion gridno specific...
      pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
      pSoldier->usStrategicInsertionData = 10125;
    }
  }
}

BOOLEAN RemoveSoldierFromVehicle(struct SOLDIERTYPE *pSoldier, int32_t iId) {
  // remove soldier from vehicle
  int32_t iCounter = 0;
  BOOLEAN fSoldierLeft = FALSE;
  BOOLEAN fSoldierFound = FALSE;
  struct SOLDIERTYPE *pVehicleSoldier;

  if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
    return (FALSE);
  }

  // now check if vehicle is valid
  if (pVehicleList[iId].fValid == FALSE) {
    return (FALSE);
  }

  // now look for the grunt
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] == pSoldier) {
      fSoldierFound = TRUE;

      pVehicleList[iId].pPassengers[iCounter]->ubGroupID = 0;
      pVehicleList[iId].pPassengers[iCounter]->sSectorY = pVehicleList[iId].sSectorY;
      pVehicleList[iId].pPassengers[iCounter]->sSectorX = pVehicleList[iId].sSectorX;
      pVehicleList[iId].pPassengers[iCounter]->bSectorZ = (int8_t)pVehicleList[iId].sSectorZ;
      pVehicleList[iId].pPassengers[iCounter] = NULL;

      pSoldier->uiStatusFlags &= (~(SOLDIER_DRIVER | SOLDIER_PASSENGER));

      // check if anyone left in vehicle
      fSoldierLeft = FALSE;
      for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType];
           iCounter++) {
        if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
          fSoldierLeft = TRUE;
        }
      }

      if (pVehicleList[iId].ubMovementGroup != 0) {
        RemovePlayerFromGroup(pVehicleList[iId].ubMovementGroup, pSoldier);

        /* ARM 20-01-99, now gonna disallow exiting vehicles between sectors except if merc is
           leaving (fired, dies, contract runs out) in which case we don't need to give them up for
           anything movement related since they're gone.

                                        // check if vehicle was between sectors, if so, grunt must
           go it on foot if( pVehicleList[ iId ].fBetweenSectors == TRUE )
                                        {
                                                RemoveSoldierFromVehicleBetweenSectors( pSoldier,
           iId );
                                        }
        */
      }

      break;
    }
  }

  if (!fSoldierFound) {
    return (FALSE);
  }

  // Are we the last?
  // if ( GetNumberInVehicle( iId ) == 0 )
  if (fSoldierLeft == FALSE) {
    // is the vehicle the helicopter?..it can continue moving when no soldiers aboard (Skyrider
    // remains)
    if (iId != iHelicopterVehicleId) {
      pVehicleSoldier = GetSoldierStructureForVehicle(iId);
      Assert(pVehicleSoldier);

      if (pVehicleSoldier) {
        // and he has a route set
        if (GetLengthOfMercPath(pVehicleSoldier) > 0) {
          // cancel the entire path (also handles reversing directions)
          CancelPathForVehicle(&(pVehicleList[iId]), FALSE);
        }

        // if the vehicle was abandoned between sectors
        if (pVehicleList[iId].fBetweenSectors) {
          // teleport it to the closer of its current and next sectors (it beats having it arrive
          // empty later)
          TeleportVehicleToItsClosestSector(iId, pVehicleSoldier->ubGroupID);
        }

        // Remove vehicle from squad.....
        RemoveCharacterFromSquads(pVehicleSoldier);
        // ATE: Add him back to vehicle group!
        if (!DoesPlayerExistInPGroup(pVehicleList[iId].ubMovementGroup, pVehicleSoldier)) {
          AddPlayerToGroup(pVehicleList[iId].ubMovementGroup, pVehicleSoldier);
        }
        ChangeSoldiersAssignment(pVehicleSoldier, ASSIGNMENT_EMPTY);

        /* ARM Removed Feb. 17, 99 - causes pVehicleSoldier->ubGroupID to become 0, which will cause
           assert later on RemovePlayerFromGroup( pVehicleSoldier->ubGroupID, pVehicleSoldier );
        */

        /*
                                        // Change sides...
                                        pVehicleSoldier = ChangeSoldierTeam( pVehicleSoldier,
           CIV_TEAM );
                                        // subtract it from mapscreen list
                                        fReBuildCharacterList = TRUE;

                                        RemoveCharacterFromSquads( pVehicleSoldier );
        */
      }
    }
  }

  // if he got out of the chopper
  if (iId == iHelicopterVehicleId) {
    // and he's alive
    if (pSoldier->bLife >= OKLIFE) {
      // mark the sector as visited (flying around in the chopper doesn't, so this does it as soon
      // as we get off it)
      SetSectorFlag(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier),
                    SF_ALREADY_VISITED);
    }

    SetSoldierExitVehicleInsertionData(pSoldier, iId);

    // Update in sector if this is the current sector.....
    if (GetSolSectorX(pSoldier) == gWorldSectorX && GetSolSectorY(pSoldier) == gWorldSectorY &&
        GetSolSectorZ(pSoldier) == gbWorldSectorZ) {
      UpdateMercInSector(pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
    }
  }

  // soldier successfully removed
  return (TRUE);
}

BOOLEAN MoveCharactersPathToVehicle(struct SOLDIERTYPE *pSoldier) {
  int32_t iId;
  // valid soldier?
  if (pSoldier == NULL) {
    return (FALSE);
  }

  // check if character is in fact in a vehicle
  if ((pSoldier->bAssignment != VEHICLE) && (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE))) {
    // now clear soldier's path
    pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, 0);
    return (FALSE);
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // grab the id the character is
    iId = pSoldier->bVehicleID;
  } else {
    // grab the id the character is
    iId = pSoldier->iVehicleId;
  }

  // check if vehicle is valid
  if (iId != -1) {
    // check if vehicle has mvt group, if not, get one for it
    if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
      // now clear soldier's path
      pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, 0);
      return (FALSE);
    }

    // now check if vehicle is valid
    if (pVehicleList[iId].fValid == FALSE) {
      // now clear soldier's path
      pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, 0);
      return (FALSE);
    }
  }

  // valid vehicle

  // now clear soldier's path
  pVehicleList[iId].pMercPath =
      ClearStrategicPathList(pVehicleList[iId].pMercPath, pVehicleList[iId].ubMovementGroup);

  // now copy over
  pVehicleList[iId].pMercPath = CopyPaths(pSoldier->pMercPath, pVehicleList[iId].pMercPath);

  // move to beginning
  pVehicleList[iId].pMercPath = MoveToBeginningOfPathList(pVehicleList[iId].pMercPath);

  // now clear soldier's path
  pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, 0);

  return (TRUE);
}

BOOLEAN CopyVehiclePathToSoldier(struct SOLDIERTYPE *pSoldier) {
  int32_t iId;

  // valid soldier?
  if (pSoldier == NULL) {
    return (FALSE);
  }

  // check if character is in fact in a vehicle
  if ((pSoldier->bAssignment != VEHICLE) && (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE))) {
    return (FALSE);
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // grab the id the character is
    iId = pSoldier->bVehicleID;
  } else {
    // grab the id the character is
    iId = pSoldier->iVehicleId;
  }

  // check if vehicle is valid
  if (iId != -1) {
    // check if vehicle has mvt group, if not, get one for it
    if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
      return (FALSE);
    }

    // now check if vehicle is valid
    if (pVehicleList[iId].fValid == FALSE) {
      return (FALSE);
    }
  }

  // reset mvt group for the grunt
  // ATE: NOT if we are the vehicle
  if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    pSoldier->ubGroupID = pVehicleList[iId].ubMovementGroup;
  }

  // valid vehicle

  // clear grunt path
  if (pSoldier->pMercPath) {
    // clear soldier's path
    pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, 0);
  }

  // now copy over
  pSoldier->pMercPath = CopyPaths(pVehicleList[iId].pMercPath, pSoldier->pMercPath);

  return (TRUE);
}

BOOLEAN SetUpMvtGroupForVehicle(struct SOLDIERTYPE *pSoldier) {
  // given this grunt, find out if asscoiated vehicle has a mvt group, if so, set this grunts mvt
  // group tho the vehicle for pathing purposes, will be reset to zero in copying of path
  int32_t iId = 0;

  // check if character is in fact in a vehicle
  if ((pSoldier->bAssignment != VEHICLE) && (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE))) {
    return (FALSE);
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // grab the id the character is
    iId = pSoldier->bVehicleID;
  } else {
    // grab the id the character is
    iId = pSoldier->iVehicleId;
  }

  if (pSoldier->pMercPath) {
    // clear soldier's path
    pSoldier->pMercPath = ClearStrategicPathList(pSoldier->pMercPath, pSoldier->ubGroupID);
  }

  // if no group, create one for vehicle
  // if( pVehicleList[ iId ].ubMovementGroup == 0 )
  //{
  // get the vehicle a mvt group
  // pVehicleList[ iId ].ubMovementGroup = CreateNewVehicleGroupDepartingFromSector( ( uint8_t )(
  // pVehicleList[ iId ].sSectorX ), ( uint8_t )( pVehicleList[ iId ].sSectorY ), iId );
  // pVehicleList[ iId ].ubMovementGroup = CreateNewVehicleGroupDepartingFromSector( ( uint8_t )(
  // pVehicleList[ iId
  // ].sSectorX ), ( uint8_t )( pVehicleList[ iId ].sSectorY ), iId );

  // add everyone in vehicle to this mvt group
  // for( iCounter = 0; iCounter < iSeatingCapacities[ pVehicleList[ iId ].ubVehicleType ];
  // iCounter++ )
  //{
  //	if( pVehicleList[ iId ].pPassengers[ iCounter ] != NULL )
  //	{
  //			// add character
  //		AddPlayerToGroup( pVehicleList[ iId ].ubMovementGroup, pVehicleList[ iId
  //].pPassengers[ iCounter ] );
  //	}
  //}
  //}

  CopyVehiclePathToSoldier(pSoldier);

  // set up mvt group
  pSoldier->ubGroupID = pVehicleList[iId].ubMovementGroup;

  return (TRUE);
}
BOOLEAN VehicleIdIsValid(int32_t iId) {
  // check if vehicle has mvt group, if not, get one for it
  if ((iId >= ubNumberOfVehicles) || (iId < 0)) {
    return (FALSE);
  }
  // now check if vehicle is valid
  if (pVehicleList[iId].fValid == FALSE) {
    return (FALSE);
  }

  return (TRUE);
}

// get travel time of vehicle
int32_t GetTravelTimeOfVehicle(int32_t iId) {
  struct GROUP *pGroup;

  // valid vehicle?
  if (VehicleIdIsValid(iId) == FALSE) {
    return (0);
  }

  // no mvt group?
  if (pVehicleList[iId].ubMovementGroup == 0) {
    return (0);
  }

  pGroup = GetGroup(pVehicleList[iId].ubMovementGroup);

  if (pGroup == NULL) {
    pVehicleList[iId].ubMovementGroup = 0;
    return (0);
  }

  return (CalculateTravelTimeOfGroupId(pVehicleList[iId].ubMovementGroup));
}

void UpdatePositionOfMercsInVehicle(int32_t iId) {
  int32_t iCounter = 0;

  // update the position of all the grunts in the vehicle
  if (VehicleIdIsValid(iId) == FALSE) {
    return;
  }

  // go through list of mercs in vehicle and set all thier states as arrived
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
      pVehicleList[iId].pPassengers[iCounter]->sSectorY = pVehicleList[iId].sSectorY;
      pVehicleList[iId].pPassengers[iCounter]->sSectorX = pVehicleList[iId].sSectorX;
      pVehicleList[iId].pPassengers[iCounter]->fBetweenSectors = FALSE;
    }
  }

  return;
}

int32_t GivenMvtGroupIdFindVehicleId(uint8_t ubGroupId) {
  int32_t iCounter = 0;

  // given the id of a mvt group, find a vehicle in this group
  if (ubGroupId == 0) {
    return (-1);
  }

  for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
    // might have an empty slot
    if (pVehicleList[iCounter].fValid == TRUE) {
      if (pVehicleList[iCounter].ubMovementGroup == ubGroupId) {
        return (iCounter);
      }
    }
  }

  return (-1);
}

// add all people in this vehicle to the mvt group for benifit of prebattle interface
BOOLEAN AddVehicleMembersToMvtGroup(int32_t iId) {
  int32_t iCounter = 0;

  if (VehicleIdIsValid(iId) == FALSE) {
    return (FALSE);
  }

  // clear the vehicle people list out
  // RemoveAllPlayersFromGroup( pVehicleList[ iId ].ubMovementGroup );

  // go through list of mercs in vehicle and set all thier states as arrived
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
      AddPlayerToGroup(pVehicleList[iId].ubMovementGroup, pVehicleList[iId].pPassengers[iCounter]);
    }
  }

  return (TRUE);
}

BOOLEAN InjurePersonInVehicle(int32_t iId, struct SOLDIERTYPE *pSoldier, uint8_t ubPointsOfDmg) {
  // find this person, see if they have this many pts left, if not, kill them

  // find if vehicle is valid
  if (VehicleIdIsValid(iId) == FALSE) {
    return (FALSE);
  }

  // check if soldier is valid
  if (pSoldier == NULL) {
    return (FALSE);
  }

  // now check hpts of merc
  if (pSoldier->bLife == 0) {
    // guy is dead, leave
    return (FALSE);
  }

  // see if we will infact kill them
  if (ubPointsOfDmg >= pSoldier->bLife) {
    return (KillPersonInVehicle(iId, pSoldier));
  }

  // otherwise hurt them
  SoldierTakeDamage(pSoldier, 0, ubPointsOfDmg, ubPointsOfDmg, TAKE_DAMAGE_GUNFIRE, NOBODY, NOWHERE,
                    0, TRUE);

  HandleSoldierTakeDamageFeedback(pSoldier);

  return (TRUE);
}

BOOLEAN KillPersonInVehicle(int32_t iId, struct SOLDIERTYPE *pSoldier) {
  // find if vehicle is valid
  if (VehicleIdIsValid(iId) == FALSE) {
    return (FALSE);
  }

  // check if soldier is valid
  if (pSoldier == NULL) {
    return (FALSE);
  }

  // now check hpts of merc
  if (pSoldier->bLife == 0) {
    // guy is dead, leave
    return (FALSE);
  }

  // otherwise hurt them
  SoldierTakeDamage(pSoldier, 0, 100, 100, TAKE_DAMAGE_BLOODLOSS, NOBODY, NOWHERE, 0, TRUE);

  return (TRUE);
}

BOOLEAN KillAllInVehicle(int32_t iId) {
  int32_t iCounter = 0;

  // find if vehicle is valid
  if (VehicleIdIsValid(iId) == FALSE) {
    return (FALSE);
  }

  // go through list of occupants and kill them
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
      if (KillPersonInVehicle(iId, pVehicleList[iId].pPassengers[iCounter]) == FALSE) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

int32_t GetNumberInVehicle(int32_t iId) {
  // go through list of occupants in vehicles and count them
  int32_t iCounter = 0;
  int32_t iCount = 0;

  // find if vehicle is valid
  if (VehicleIdIsValid(iId) == FALSE) {
    return (0);
  }

  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
      iCount++;
    }
  }

  return (iCount);
}

int32_t GetNumberOfNonEPCsInVehicle(int32_t iId) {
  // go through list of occupants in vehicles and count them
  int32_t iCounter = 0;
  int32_t iCount = 0;

  // find if vehicle is valid
  if (VehicleIdIsValid(iId) == FALSE) {
    return (0);
  }

  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL &&
        !AM_AN_EPC(pVehicleList[iId].pPassengers[iCounter])) {
      iCount++;
    }
  }

  return (iCount);
}

BOOLEAN IsRobotControllerInVehicle(int32_t iId) {
  // go through list of occupants in vehicles and count them
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier;

  // find if vehicle is valid
  if (VehicleIdIsValid(iId) == FALSE) {
    return (0);
  }

  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    pSoldier = pVehicleList[iId].pPassengers[iCounter];
    if (pSoldier != NULL && ControllingRobot(pSoldier)) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN AnyAccessibleVehiclesInSoldiersSector(struct SOLDIERTYPE *pSoldier) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
    if (pVehicleList[iCounter].fValid == TRUE) {
      if (IsThisVehicleAccessibleToSoldier(pSoldier, iCounter)) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

struct SOLDIERTYPE *GetDriver(int32_t iID) { return (MercPtrs[pVehicleList[iID].ubDriver]); }

void SetDriver(int32_t iID, uint8_t ubID) { pVehicleList[iID].ubDriver = ubID; }

#ifdef JA2TESTVERSION
void VehicleTest(void) { SetUpHelicopterForPlayer(9, 1); }
#endif

BOOLEAN IsEnoughSpaceInVehicle(int32_t iID) {
  // find if vehicle is valid
  if (VehicleIdIsValid(iID) == FALSE) {
    return (FALSE);
  }

  if (GetNumberInVehicle(iID) == iSeatingCapacities[pVehicleList[iID].ubVehicleType]) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN PutSoldierInVehicle(struct SOLDIERTYPE *pSoldier, int8_t bVehicleId) {
  struct SOLDIERTYPE *pVehicleSoldier = NULL;

  if ((GetSolSectorX(pSoldier) != gWorldSectorX) || (GetSolSectorY(pSoldier) != gWorldSectorY) ||
      (GetSolSectorZ(pSoldier) != 0) || (bVehicleId == iHelicopterVehicleId)) {
    // add the soldier
    return (AddSoldierToVehicle(pSoldier, bVehicleId));
  } else {
    // grab the soldier struct for the vehicle
    pVehicleSoldier = GetSoldierStructureForVehicle(bVehicleId);

    // enter the vehicle
    return (EnterVehicle(pVehicleSoldier, pSoldier));
  }
}

BOOLEAN TakeSoldierOutOfVehicle(struct SOLDIERTYPE *pSoldier) {
  // if not in vehicle, don't take out, not much point, now is there?
  if (pSoldier->bAssignment != VEHICLE) {
    return (FALSE);
  }

  if ((GetSolSectorX(pSoldier) != gWorldSectorX) || (GetSolSectorY(pSoldier) != gWorldSectorY) ||
      (GetSolSectorZ(pSoldier) != 0) || !pSoldier->bInSector) {
    // add the soldier
    return (RemoveSoldierFromVehicle(pSoldier, pSoldier->iVehicleId));
  } else {
    // helicopter isn't a soldiertype instance
    if (pSoldier->iVehicleId == iHelicopterVehicleId) {
      return (RemoveSoldierFromVehicle(pSoldier, pSoldier->iVehicleId));
    } else {
      // exit the vehicle
      return (ExitVehicle(pSoldier));
    }
  }
}

BOOLEAN EnterVehicle(struct SOLDIERTYPE *pVehicle, struct SOLDIERTYPE *pSoldier) {
  // TEST IF IT'S VALID...
  if (pVehicle->uiStatusFlags & SOLDIER_VEHICLE) {
    // Is there room...
    if (IsEnoughSpaceInVehicle(pVehicle->bVehicleID)) {
      // OK, add....
      AddSoldierToVehicle(pSoldier, pVehicle->bVehicleID);

      if (!(IsMapScreen())) {
        // Change to team panel if we are not already...
        SetCurrentInterfacePanel(TEAM_PANEL);
      }

      return (TRUE);
    }
  }

  return (FALSE);
}

struct SOLDIERTYPE *GetVehicleSoldierPointerFromPassenger(struct SOLDIERTYPE *pSrcSoldier) {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // End the turn of player charactors
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
      // Check ubID....
      if (pSoldier->bVehicleID == pSrcSoldier->iVehicleId) {
        return (pSoldier);
      }
    }
  }

  return (NULL);
}

BOOLEAN ExitVehicle(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pVehicle;
  uint8_t ubDirection;
  int16_t sGridNo;

  // Get vehicle from soldier...
  pVehicle = GetVehicleSoldierPointerFromPassenger(pSoldier);

  if (pVehicle == NULL) {
    return (FALSE);
  }

  // TEST IF IT'S VALID...
  if (pVehicle->uiStatusFlags & SOLDIER_VEHICLE) {
    sGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, pSoldier->usUIMovementMode,
                                                               5, &ubDirection, 3, pVehicle);

    if (sGridNo == NOWHERE) {
      // ATE: BUT we need a place, widen the search
      sGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(
          pSoldier, pSoldier->usUIMovementMode, 20, &ubDirection, 3, pVehicle);
    }

    // OK, remove....
    RemoveSoldierFromVehicle(pSoldier, pVehicle->bVehicleID);

    // Were we the driver, and if so, pick another....
    pSoldier->sInsertionGridNo = sGridNo;
    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
    pSoldier->usStrategicInsertionData = pSoldier->sInsertionGridNo;
    pSoldier->iVehicleId = -1;

    // AllTeamsLookForAll( FALSE );
    pSoldier->bOppList[pVehicle->ubID] = 1;

    // Add to sector....
    EVENT_SetSoldierPosition(pSoldier, CenterX(sGridNo), CenterY(sGridNo));

    // Update visiblity.....
    HandleSight(pSoldier, SIGHT_LOOK | SIGHT_RADIO);

    // Add to unique squad....
    AddCharacterToUniqueSquad(pSoldier);

    // can't call SetCurrentSquad OR SelectSoldier in mapscreen, that will initialize interface
    // panels!!!
    if (IsTacticalMode()) {
      SetCurrentSquad(pSoldier->bAssignment, TRUE);

      SelectSoldier(pSoldier->ubID, FALSE, TRUE);
    }

    PlayJA2Sample(pVehicleList[pVehicle->bVehicleID].iOutOfSound, RATE_11025,
                  SoundVolume(HIGHVOLUME, pVehicle->sGridNo), 1, SoundDir(pVehicle->sGridNo));
    return (TRUE);
  }

  return (FALSE);
}

void AddPassangersToTeamPanel(int32_t iId) {
  int32_t cnt;

  for (cnt = 0; cnt < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; cnt++) {
    if (pVehicleList[iId].pPassengers[cnt] != NULL) {
      // add character
      AddPlayerToInterfaceTeamSlot(pVehicleList[iId].pPassengers[cnt]->ubID);
    }
  }
}

void VehicleTakeDamage(uint8_t ubID, uint8_t ubReason, int16_t sDamage, int16_t sGridNo,
                       uint8_t ubAttackerID) {
  if (ubReason != TAKE_DAMAGE_GAS) {
    PlayJA2Sample((uint32_t)(S_METAL_IMPACT3), RATE_11025, SoundVolume(MIDVOLUME, sGridNo), 1,
                  SoundDir(sGridNo));
  }

  // check if there was in fact damage done to the vehicle
  if ((ubReason == TAKE_DAMAGE_HANDTOHAND) || (ubReason == TAKE_DAMAGE_GAS)) {
    // nope
    return;
  }

  if (pVehicleList[ubID].fDestroyed == FALSE) {
    switch (ubReason) {
      case (TAKE_DAMAGE_GUNFIRE):
      case (TAKE_DAMAGE_EXPLOSION):
      case (TAKE_DAMAGE_STRUCTURE_EXPLOSION):

        HandleCriticalHitForVehicleInLocation(ubID, sDamage, sGridNo, ubAttackerID);
        break;
    }
  }
}

void HandleCriticalHitForVehicleInLocation(uint8_t ubID, int16_t sDmg, int16_t sGridNo,
                                           uint8_t ubAttackerID) {
  // check state the armor was s'posed to be in vs. the current state..the difference / orig state
  // is % chance that a critical hit will occur
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fMadeCorpse = FALSE;

  pSoldier = GetSoldierStructureForVehicle(ubID);

  if (sDmg > pSoldier->bLife) {
    pSoldier->bLife = 0;
  } else {
    // Decrease Health
    pSoldier->bLife -= sDmg;
  }

  if (pSoldier->bLife < OKLIFE) {
    pSoldier->bLife = 0;
  }

  // Show damage
  pSoldier->sDamage += sDmg;

  if (pSoldier->bInSector && pSoldier->bVisible != -1) {
    // If we are already dead, don't show damage!
    if (sDmg != 0) {
      // Display damage
      int16_t sMercScreenX, sMercScreenY, sOffsetX, sOffsetY;

      // Set Damage display counter
      pSoldier->fDisplayDamage = TRUE;
      pSoldier->bDisplayDamageCount = 0;

      GetSoldierScreenPos(pSoldier, &sMercScreenX, &sMercScreenY);
      GetSoldierAnimOffsets(pSoldier, &sOffsetX, &sOffsetY);
      pSoldier->sDamageX = sOffsetX;
      pSoldier->sDamageY = sOffsetY;
    }
  }

  if (pSoldier->bLife == 0 && !pVehicleList[ubID].fDestroyed) {
    pVehicleList[ubID].fDestroyed = TRUE;

    // Explode vehicle...
    IgniteExplosion(ubAttackerID, CenterX(sGridNo), CenterY(sGridNo), 0, sGridNo,
                    GREAT_BIG_EXPLOSION, 0);

    if (pSoldier != NULL) {
      // Tacticlly remove soldier....
      // EVENT_InitNewSoldierAnim( pSoldier, VEHICLE_DIE, 0, FALSE );
      // TacticalRemoveSoldier( GetSolID(pSoldier) );

      CheckForAndHandleSoldierDeath(pSoldier, &fMadeCorpse);
    }

    // Kill all in vehicle...
    KillAllInVehicle(ubID);
  }

  return;
}

BOOLEAN DoesVehicleNeedAnyRepairs(int32_t iVehicleId) {
  struct SOLDIERTYPE *pVehicleSoldier = NULL;

  // is the vehicle in fact a valid vehicle
  if (VehicleIdIsValid(iVehicleId) == FALSE) {
    // nope
    return (FALSE);
  }

  // Skyrider isn't damagable/repairable
  if (iVehicleId == iHelicopterVehicleId) {
    return (FALSE);
  }

  // get the vehicle soldiertype
  pVehicleSoldier = GetSoldierStructureForVehicle(iVehicleId);

  if (pVehicleSoldier->bLife != pVehicleSoldier->bLifeMax) {
    return (TRUE);
  }

  // everything is in perfect condition
  return (FALSE);
}

int8_t RepairVehicle(int32_t iVehicleId, int8_t bRepairPtsLeft, BOOLEAN *pfNothingToRepair) {
  struct SOLDIERTYPE *pVehicleSoldier = NULL;
  int8_t bRepairPtsUsed = 0;
  int8_t bOldLife;

  // is the vehicle in fact a valid vehicle
  if (VehicleIdIsValid(iVehicleId) == FALSE) {
    // nope
    return (bRepairPtsUsed);
  }

  // Skyrider isn't damagable/repairable
  if (iVehicleId == iHelicopterVehicleId) {
    return (bRepairPtsUsed);
  }

  // get the vehicle soldiertype
  pVehicleSoldier = GetSoldierStructureForVehicle(iVehicleId);

  if (!DoesVehicleNeedAnyRepairs(iVehicleId)) {
    return (bRepairPtsUsed);
  }

  bOldLife = pVehicleSoldier->bLife;

  // Repair
  pVehicleSoldier->bLife += (bRepairPtsLeft / VEHICLE_REPAIR_POINTS_DIVISOR);

  // Check
  if (pVehicleSoldier->bLife > pVehicleSoldier->bLifeMax) {
    pVehicleSoldier->bLife = pVehicleSoldier->bLifeMax;
  }

  // Calculate pts used;
  bRepairPtsUsed = (pVehicleSoldier->bLife - bOldLife) * VEHICLE_REPAIR_POINTS_DIVISOR;

  // ARM: personally, I'd love to know where in Arulco the mechanic gets the PARTS to do this stuff,
  // but hey, it's a game!
  (*pfNothingToRepair) = !DoesVehicleNeedAnyRepairs(iVehicleId);

  return (bRepairPtsUsed);
}

/*
int16_t GetOrigInternalArmorValueForVehicleInLocation( uint8_t ubID, uint8_t ubLocation )
{
        int16_t sArmorValue = 0;

        sArmorValue = sVehicleInternalOrigArmorValues[ pVehicleList[ ubID ].ubVehicleType ][
ubLocation ];

        // return the armor value
        return( sArmorValue );
}
*/

struct SOLDIERTYPE *GetSoldierStructureForVehicle(int32_t iId) {
  struct SOLDIERTYPE *pSoldier = NULL, *pFoundSoldier = NULL;
  int32_t iCounter = 0, iNumberOnTeam = 0;

  // get number of mercs on team
  iNumberOnTeam = TOTAL_SOLDIERS;  // gTacticalStatus.Team[ OUR_TEAM ].bLastID;

  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pSoldier = GetSoldierByID(iCounter);

    if (IsSolActive(pSoldier)) {
      if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
        if (pSoldier->bVehicleID == iId) {
          pFoundSoldier = pSoldier;
          iCounter = iNumberOnTeam;
        }
      }
    }
  }

  return (pFoundSoldier);
}

void SetUpArmorForVehicle(uint8_t ubID) {
  // for armour type, store the index into the armour table itself
  pVehicleList[ubID].sArmourType =
      Item[sVehicleArmourType[pVehicleList[ubID].ubVehicleType]].ubClassIndex;
}

void AdjustVehicleAPs(struct SOLDIERTYPE *pSoldier, uint8_t *pubPoints) {
  uint8_t pubDeducations = 0;
  int32_t iCounter = 0;

  (*pubPoints) += 35;

  // check for state of critcals

  // handle for each engine crit
  pubDeducations +=
      pVehicleList[pSoldier->bVehicleID].sCriticalHits[ENGINE_HIT_LOCATION] * COST_PER_ENGINE_CRIT;

  // handle each tire
  for (iCounter = RF_TIRE_HIT_LOCATION; iCounter < LR_TIRE_HIT_LOCATION; iCounter++) {
    if (pVehicleList[pSoldier->bVehicleID].sCriticalHits[iCounter]) {
      pubDeducations += COST_PER_TIRE_HIT;
    }
  }

  // make sure we don't go too far
  if (pubDeducations > (*pubPoints)) {
    pubDeducations = (*pubPoints);
  }

  // now deduct pts
  (*pubPoints) -= pubDeducations;
}

BOOLEAN SaveVehicleInformationToSaveGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten;
  struct path *pTempPathPtr;
  uint32_t uiNodeCount = 0;
  uint8_t cnt;
  VEHICLETYPE TempVehicle;
  uint8_t ubPassengerCnt = 0;

  // Save the number of elements
  FileMan_Write(hFile, &ubNumberOfVehicles, sizeof(uint8_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint8_t)) {
    return (FALSE);
  }

  // loop through all the vehicles and save each one
  for (cnt = 0; cnt < ubNumberOfVehicles; cnt++) {
    // save if the vehicle spot is valid
    FileMan_Write(hFile, &pVehicleList[cnt].fValid, sizeof(BOOLEAN), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(BOOLEAN)) {
      return (FALSE);
    }

    if (pVehicleList[cnt].fValid) {
      // copy the node into the temp vehicle buffer ( need to do this because we cant save the
      // pointers to the soldier, therefore save the soldier ubProfile
      memcpy(&TempVehicle, &pVehicleList[cnt], sizeof(VEHICLETYPE));

      // loop through the passengers
      for (ubPassengerCnt = 0; ubPassengerCnt < 10; ubPassengerCnt++) {
        TempVehicle.pPassengers[ubPassengerCnt] = (struct SOLDIERTYPE *)NO_PROFILE;

        // if there is a passenger here
        if (pVehicleList[cnt].pPassengers[ubPassengerCnt]) {
          // assign the passengers profile to the struct
          // ! The pointer to the passenger is converted to a byte so that the Id of the soldier can
          // be saved. ! This means that the pointer contains a bogus pointer, but a real ID for the
          // soldier. ! When reloading, this bogus pointer is converted to a byte to contain the id
          // of the soldier so ! we can get the REAL pointer to the soldier
          TempVehicle.pPassengers[ubPassengerCnt] =
              (struct SOLDIERTYPE
                   *)((uintptr_t)pVehicleList[cnt].pPassengers[ubPassengerCnt]->ubProfile);
        }
      }

      // save the vehicle info
      FileMan_Write(hFile, &TempVehicle, sizeof(VEHICLETYPE), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(VEHICLETYPE)) {
        return (FALSE);
      }
      // count the number of nodes in the vehicles path
      uiNodeCount = 0;
      pTempPathPtr = pVehicleList[cnt].pMercPath;
      while (pTempPathPtr) {
        uiNodeCount++;
        pTempPathPtr = pTempPathPtr->pNext;
      }

      // Save the number of nodes
      FileMan_Write(hFile, &uiNodeCount, sizeof(uint32_t), &uiNumBytesWritten);
      if (uiNumBytesWritten != sizeof(uint32_t)) {
        return (FALSE);
      }

      // save all the nodes
      pTempPathPtr = pVehicleList[cnt].pMercPath;
      while (pTempPathPtr) {
        // Save the node
        FileMan_Write(hFile, pTempPathPtr, sizeof(struct path), &uiNumBytesWritten);
        if (uiNumBytesWritten != sizeof(struct path)) {
          return (FALSE);
        }

        pTempPathPtr = pTempPathPtr->pNext;
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadVehicleInformationFromSavedGameFile(HWFILE hFile, uint32_t uiSavedGameVersion) {
  uint32_t uiNumBytesRead;
  uint32_t uiTotalNodeCount = 0;
  uint8_t cnt;
  uint32_t uiNodeCount = 0;
  struct path *pPath = NULL;
  uint8_t ubPassengerCnt = 0;
  struct path *pTempPath;

  // Clear out th vehicle list
  ClearOutVehicleList();

  // Load the number of elements
  FileMan_Read(hFile, &ubNumberOfVehicles, sizeof(uint8_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint8_t)) {
    return (FALSE);
  }

  if (ubNumberOfVehicles != 0) {
    // allocate memory to hold the vehicle list
    pVehicleList = (VEHICLETYPE *)MemAlloc(sizeof(VEHICLETYPE) * ubNumberOfVehicles);
    if (pVehicleList == NULL) return (FALSE);
    memset(pVehicleList, 0, sizeof(VEHICLETYPE) * ubNumberOfVehicles);

    // loop through all the vehicles and load each one
    for (cnt = 0; cnt < ubNumberOfVehicles; cnt++) {
      // Load if the vehicle spot is valid
      FileMan_Read(hFile, &pVehicleList[cnt].fValid, sizeof(BOOLEAN), &uiNumBytesRead);
      if (uiNumBytesRead != sizeof(BOOLEAN)) {
        return (FALSE);
      }

      if (pVehicleList[cnt].fValid) {
        // load the vehicle info
        FileMan_Read(hFile, &pVehicleList[cnt], sizeof(VEHICLETYPE), &uiNumBytesRead);
        if (uiNumBytesRead != sizeof(VEHICLETYPE)) {
          return (FALSE);
        }

        //
        // Build the passenger list
        //

        // loop through all the passengers
        for (ubPassengerCnt = 0; ubPassengerCnt < 10; ubPassengerCnt++) {
          if (uiSavedGameVersion < 86) {
            if (pVehicleList[cnt].pPassengers[ubPassengerCnt] != 0) {
              // ! The id of the soldier was saved in the passenger pointer.  The passenger pointer
              // is converted back ! to a uint8_t so we can get the REAL pointer to the soldier.
              pVehicleList[cnt].pPassengers[ubPassengerCnt] = FindSoldierByProfileID(
                  (uint8_t)((uintptr_t)pVehicleList[cnt].pPassengers[ubPassengerCnt]), FALSE);
            }
          } else {
            if (pVehicleList[cnt].pPassengers[ubPassengerCnt] != (struct SOLDIERTYPE *)NO_PROFILE) {
              // ! The id of the soldier was saved in the passenger pointer.  The passenger pointer
              // is converted back ! to a uint8_t so we can get the REAL pointer to the soldier.
              pVehicleList[cnt].pPassengers[ubPassengerCnt] = FindSoldierByProfileID(
                  (uint8_t)((uintptr_t)pVehicleList[cnt].pPassengers[ubPassengerCnt]), FALSE);
            } else {
              pVehicleList[cnt].pPassengers[ubPassengerCnt] = NULL;
            }
          }
        }

        // Load the number of nodes
        FileMan_Read(hFile, &uiTotalNodeCount, sizeof(uint32_t), &uiNumBytesRead);
        if (uiNumBytesRead != sizeof(uint32_t)) {
          return (FALSE);
        }

        if (uiTotalNodeCount != 0) {
          pPath = NULL;

          pVehicleList[cnt].pMercPath = NULL;

          // loop through each node
          for (uiNodeCount = 0; uiNodeCount < uiTotalNodeCount; uiNodeCount++) {
            // allocate memory to hold the vehicle path
            pTempPath = (struct path *)MemAlloc(sizeof(struct path));
            if (pTempPath == NULL) return (FALSE);
            memset(pTempPath, 0, sizeof(struct path));

            // Load all the nodes
            FileMan_Read(hFile, pTempPath, sizeof(struct path), &uiNumBytesRead);
            if (uiNumBytesRead != sizeof(struct path)) {
              return (FALSE);
            }

            //
            // Setup the pointer info
            //

            if (pVehicleList[cnt].pMercPath == NULL) pVehicleList[cnt].pMercPath = pTempPath;

            // if there is a previous node
            if (pPath != NULL) {
              pPath->pNext = pTempPath;

              pTempPath->pPrev = pPath;
            } else
              pTempPath->pPrev = NULL;

            pTempPath->pNext = NULL;

            pPath = pTempPath;
          }
        } else {
          pVehicleList[cnt].pMercPath = NULL;
        }
      }
    }
  }
  return (TRUE);
}

void SetVehicleSectorValues(int32_t iVehId, uint8_t ubSectorX, uint8_t ubSectorY) {
  pVehicleList[iVehId].sSectorX = ubSectorX;
  pVehicleList[iVehId].sSectorY = ubSectorY;

  gMercProfiles[pVehicleList[iVehId].ubProfileID].sSectorX = ubSectorX;
  gMercProfiles[pVehicleList[iVehId].ubProfileID].sSectorY = ubSectorY;
}

void UpdateAllVehiclePassengersGridNo(struct SOLDIERTYPE *pSoldier) {
  int32_t iCounter, iId;
  struct SOLDIERTYPE *pPassenger;

  // If not a vehicle, ignore!
  if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    return;
  }

  iId = pSoldier->bVehicleID;

  // Loop through passengers and update each guy's position
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
      pPassenger = pVehicleList[iId].pPassengers[iCounter];

      // Set gridno.....
      EVENT_SetSoldierPosition(pPassenger, pSoldier->dXPos, pSoldier->dYPos);
    }
  }
}

BOOLEAN SaveVehicleMovementInfoToSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten = 0;

  // Save all the vehicle movement id's
  FileMan_Write(hFile, gubVehicleMovementGroups, sizeof(int8_t) * 5, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(int8_t) * 5) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadVehicleMovementInfoFromSavedGameFile(HWFILE hFile) {
  int32_t cnt;
  struct GROUP *pGroup = NULL;
  uint32_t uiNumBytesRead = 0;

  // Load in the Squad movement id's
  FileMan_Read(hFile, gubVehicleMovementGroups, sizeof(int8_t) * 5, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(int8_t) * 5) {
    return (FALSE);
  }

  for (cnt = 5; cnt < MAX_VEHICLES; cnt++) {
    // create mvt groups
    gubVehicleMovementGroups[cnt] = CreateNewVehicleGroupDepartingFromSector(1, 1, cnt);

    // Set persistent....
    pGroup = GetGroup(gubVehicleMovementGroups[cnt]);
    pGroup->fPersistant = TRUE;
  }

  return (TRUE);
}

BOOLEAN NewSaveVehicleMovementInfoToSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten = 0;

  // Save all the vehicle movement id's
  FileMan_Write(hFile, gubVehicleMovementGroups, sizeof(int8_t) * MAX_VEHICLES, &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(int8_t) * MAX_VEHICLES) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN NewLoadVehicleMovementInfoFromSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead = 0;

  // Load in the Squad movement id's
  FileMan_Read(hFile, gubVehicleMovementGroups, sizeof(int8_t) * MAX_VEHICLES, &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(int8_t) * MAX_VEHICLES) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN OKUseVehicle(uint8_t ubProfile) {
  if (ubProfile == PROF_HUMMER) {
    return (CheckFact(FACT_OK_USE_HUMMER, NO_PROFILE));
  } else if (ubProfile == PROF_ICECREAM) {
    return (CheckFact(FACT_OK_USE_ICECREAM, NO_PROFILE));
  } else if (ubProfile == PROF_HELICOPTER) {
    // don't allow mercs to get inside vehicle if it's grounded (enemy controlled, Skyrider owed
    // money, etc.)
    return (CanHelicopterFly());
  } else {
    return (TRUE);
  }
}

void TeleportVehicleToItsClosestSector(int32_t iVehicleId, uint8_t ubGroupID) {
  struct GROUP *pGroup = NULL;
  uint32_t uiTimeToNextSector;
  uint32_t uiTimeToLastSector;
  uint8_t sPrevX, sPrevY, sNextX, sNextY;

  pGroup = GetGroup(ubGroupID);
  Assert(pGroup);

  Assert(pGroup->uiTraverseTime != -1);
  Assert((pGroup->uiTraverseTime > 0) && (pGroup->uiTraverseTime != 0xffffffff));

  Assert(pGroup->uiArrivalTime >= GetWorldTotalMin());
  uiTimeToNextSector = pGroup->uiArrivalTime - GetWorldTotalMin();

  Assert(pGroup->uiTraverseTime >= uiTimeToNextSector);
  uiTimeToLastSector = pGroup->uiTraverseTime - uiTimeToNextSector;

  if (uiTimeToNextSector >= uiTimeToLastSector) {
    // go to the last sector
    sPrevX = pGroup->ubNextX;
    sPrevY = pGroup->ubNextY;

    sNextX = pGroup->ubSectorX;
    sNextY = pGroup->ubSectorY;
  } else {
    // go to the next sector
    sPrevX = pGroup->ubSectorX;
    sPrevY = pGroup->ubSectorY;

    sNextX = pGroup->ubNextX;
    sNextY = pGroup->ubNextY;
  }

  // make it arrive immediately, not eventually (it's driverless)
  SetGroupArrivalTime(pGroup, GetWorldTotalMin());

  // change where it is and where it's going, then make it arrive there.  Don't check for battle
  PlaceGroupInSector(ubGroupID, sPrevX, sPrevY, sNextX, sNextY, 0, FALSE);
}

void AddVehicleFuelToSave() {
  int32_t iCounter;
  struct SOLDIERTYPE *pVehicleSoldier = NULL;

  for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
    // might have an empty slot
    if (pVehicleList[iCounter].fValid) {
      // get the vehicle soldiertype
      pVehicleSoldier = GetSoldierStructureForVehicle(iCounter);

      if (pVehicleSoldier) {
        // Init fuel!
        pVehicleSoldier->sBreathRed = 10000;
        pVehicleSoldier->bBreath = 100;
      }
    }
  }
}

BOOLEAN CanSoldierDriveVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId,
                               BOOLEAN fIgnoreAsleep) {
  Assert(pSoldier);

  if (pSoldier->bAssignment != VEHICLE) {
    // not in a vehicle!
    return (FALSE);
  }

  if (pSoldier->iVehicleId != iVehicleId) {
    // not in THIS vehicle!
    return (FALSE);
  }

  if (iVehicleId == iHelicopterVehicleId) {
    // only Skyrider can pilot the helicopter
    return (FALSE);
  }

  if (!fIgnoreAsleep && (pSoldier->fMercAsleep == TRUE)) {
    // asleep!
    return (FALSE);
  }

  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier) || AM_AN_EPC(pSoldier)) {
    // vehicles, robot, and EPCs can't drive!
    return (FALSE);
  }

  // too wounded to drive
  if (pSoldier->bLife < OKLIFE) {
    return (FALSE);
  }

  // too tired to drive
  if (pSoldier->bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM) {
    return (FALSE);
  }

  // yup, he could drive this vehicle
  return (TRUE);
}

BOOLEAN SoldierMustDriveVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId,
                                BOOLEAN fTryingToTravel) {
  Assert(pSoldier);

  // error check
  if ((iVehicleId >= ubNumberOfVehicles) || (iVehicleId < 0)) {
    return (FALSE);
  }

  // if vehicle is not going anywhere, then nobody has to be driving it!
  // need the path length check in case we're doing a test while actually in a sector even though
  // we're moving!
  if (!fTryingToTravel && (!pVehicleList[iVehicleId].fBetweenSectors) &&
      (GetLengthOfPath(pVehicleList[iVehicleId].pMercPath) == 0)) {
    return (FALSE);
  }

  // if he CAN drive it (don't care if he is currently asleep)
  if (CanSoldierDriveVehicle(pSoldier, iVehicleId, TRUE)) {
    // and he's the ONLY one aboard who can do so
    if (OnlyThisSoldierCanDriveVehicle(pSoldier, iVehicleId)) {
      return (TRUE);
    }
    // (if there are multiple possible drivers, than the assumption is that this guy ISN'T driving,
    // so he CAN sleep)
  }

  return (FALSE);
}

BOOLEAN OnlyThisSoldierCanDriveVehicle(struct SOLDIERTYPE *pThisSoldier, int32_t iVehicleId) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  for (iCounter = gTacticalStatus.Team[OUR_TEAM].bFirstID;
       iCounter <= gTacticalStatus.Team[OUR_TEAM].bLastID; iCounter++) {
    // get the current soldier
    pSoldier = GetSoldierByID(iCounter);

    // skip checking THIS soldier, we wanna know about everyone else
    if (pSoldier == pThisSoldier) {
      continue;
    }

    if (IsSolActive(pSoldier)) {
      // don't count mercs who are asleep here
      if (CanSoldierDriveVehicle(pSoldier, iVehicleId, FALSE)) {
        // this guy can drive it, too
        return (FALSE);
      }
    }
  }

  // you're da man!
  return (TRUE);
}

BOOLEAN IsSoldierInThisVehicleSquad(struct SOLDIERTYPE *pSoldier, int8_t bSquadNumber) {
  int32_t iVehicleId;
  struct SOLDIERTYPE *pVehicleSoldier;

  Assert(pSoldier);
  Assert((bSquadNumber >= 0) && (bSquadNumber < NUMBER_OF_SQUADS));

  // not in a vehicle?
  if (pSoldier->bAssignment != VEHICLE) {
    return (FALSE);
  }

  // get vehicle ID
  iVehicleId = pSoldier->iVehicleId;

  // if in helicopter
  if (iVehicleId == iHelicopterVehicleId) {
    // they don't get a squad #
    return (FALSE);
  }

  pVehicleSoldier = GetSoldierStructureForVehicle(iVehicleId);
  Assert(pVehicleSoldier);

  // check squad vehicle is on
  if (pVehicleSoldier->bAssignment != bSquadNumber) {
    return (FALSE);
  }

  // yes, he's in a vehicle assigned to this squad
  return (TRUE);
}

struct SOLDIERTYPE *PickRandomPassengerFromVehicle(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  int32_t iCounter, iId;

  // If not a vehicle, ignore!
  if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    return (NULL);
  }

  iId = pSoldier->bVehicleID;

  // Loop through passengers and update each guy's position
  for (iCounter = 0; iCounter < iSeatingCapacities[pVehicleList[iId].ubVehicleType]; iCounter++) {
    if (pVehicleList[iId].pPassengers[iCounter] != NULL) {
      ubNumMercs++;
    }
  }

  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    // If we are air raid, AND red exists somewhere...
    return (pVehicleList[iId].pPassengers[ubChosenMerc]);
  }

  return (NULL);
}

BOOLEAN DoesVehicleHaveAnyPassengers(int32_t iVehicleID) {
  if (!GetNumberInVehicle(iVehicleID)) {
    return FALSE;
  }
  return TRUE;
}

BOOLEAN DoesVehicleGroupHaveAnyPassengers(struct GROUP *pGroup) {
  int32_t iVehicleID;

  iVehicleID = GivenMvtGroupIdFindVehicleId(pGroup->ubGroupID);
  if (iVehicleID == -1) {
#ifdef JA2BETAVERSION
    AssertMsg(iVehicleID != -1,
              "DoesVehicleGroupHaveAnyPassengers() for vehicle group.  Invalid iVehicleID.");
#endif
    return FALSE;
  }

  return DoesVehicleHaveAnyPassengers(iVehicleID);
}
