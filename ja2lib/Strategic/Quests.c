// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/Quests.h"

#include "GameSettings.h"
#include "Laptop/BobbyRMailOrder.h"
#include "Laptop/History.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/Boxing.h"
#include "Tactical/Campaign.h"
#include "Tactical/HandleItems.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "Town.h"
#include "Utils/Message.h"

#define TESTQUESTS

extern struct SOLDIERTYPE *gpSrcSoldier;
extern struct SOLDIERTYPE *gpDestSoldier;

uint8_t gubQuest[MAX_QUESTS];
uint8_t gubFact[NUM_FACTS];  // this has to be updated when we figure out how many facts we have

int16_t gsFoodQuestSectorX;
int16_t gsFoodQuestSectorY;

extern void GuaranteeAtLeastXItemsOfIndex(uint8_t ubArmsDealer, uint16_t usItemIndex,
                                          uint8_t ubHowMany);

void SetFactTrue(uint16_t usFact) {
  // This function is here just for control flow purposes (debug breakpoints)
  // and code is more readable that way

  // must intercept when Jake is first trigered to start selling fuel
  if ((usFact == FACT_ESTONI_REFUELLING_POSSIBLE) && (CheckFact(usFact, 0) == FALSE)) {
    // give him some gas...
    GuaranteeAtLeastXItemsOfIndex(ARMS_DEALER_JAKE, GAS_CAN, (uint8_t)(4 + Random(3)));
  }

  gubFact[usFact] = TRUE;
}

void SetFactFalse(uint16_t usFact) { gubFact[usFact] = FALSE; }

BOOLEAN CheckForNewShipment(void) {
  struct ITEM_POOL *pItemPool;

  if ((gWorldSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X) &&
      (gWorldSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y) &&
      (gbWorldSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z)) {
    if (GetItemPool(BOBBYR_SHIPPING_DEST_GRIDNO, &pItemPool, 0)) {
      return (!(ITEMPOOL_VISIBLE(pItemPool)));
    }
  }
  return (FALSE);
}

BOOLEAN CheckNPCWounded(uint8_t ubProfileID, BOOLEAN fByPlayerOnly) {
  struct SOLDIERTYPE *pSoldier;

  // is the NPC is wounded at all?
  pSoldier = FindSoldierByProfileID(ubProfileID, FALSE);
  if (pSoldier && pSoldier->bLife < pSoldier->bLifeMax) {
    if (fByPlayerOnly) {
      if (gMercProfiles[ubProfileID].ubMiscFlags & PROFILE_MISC_FLAG_WOUNDEDBYPLAYER) {
        return (TRUE);
      } else {
        return (FALSE);
      }
    } else {
      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

BOOLEAN CheckNPCInOkayHealth(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pSoldier;

  // is the NPC at better than half health?
  pSoldier = FindSoldierByProfileID(ubProfileID, FALSE);
  if (pSoldier && pSoldier->bLife > (pSoldier->bLifeMax / 2) && pSoldier->bLife > 30) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN CheckNPCBleeding(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pSoldier;

  // the NPC is wounded...
  pSoldier = FindSoldierByProfileID(ubProfileID, FALSE);
  if (pSoldier && pSoldier->bLife > 0 && pSoldier->bBleeding > 0) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN CheckNPCWithin(uint8_t ubFirstNPC, uint8_t ubSecondNPC, uint8_t ubMaxDistance) {
  struct SOLDIERTYPE *pFirstNPC, *pSecondNPC;

  pFirstNPC = FindSoldierByProfileID(ubFirstNPC, FALSE);
  pSecondNPC = FindSoldierByProfileID(ubSecondNPC, FALSE);
  if (!pFirstNPC || !pSecondNPC) {
    return (FALSE);
  }
  return (PythSpacesAway(pFirstNPC->sGridNo, pSecondNPC->sGridNo) <= ubMaxDistance);
}

BOOLEAN CheckGuyVisible(uint8_t ubNPC, uint8_t ubGuy) {
  // NB ONLY WORKS IF ON DIFFERENT TEAMS
  struct SOLDIERTYPE *pNPC, *pGuy;

  pNPC = FindSoldierByProfileID(ubNPC, FALSE);
  pGuy = FindSoldierByProfileID(ubGuy, FALSE);
  if (!pNPC || !pGuy) {
    return (FALSE);
  }
  if (pNPC->bOppList[pGuy->ubID] == SEEN_CURRENTLY) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN CheckNPCAt(uint8_t ubNPC, int16_t sGridNo) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubNPC, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  return (pNPC->sGridNo == sGridNo);
}

BOOLEAN CheckNPCIsEnemy(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  if (pNPC->bSide == gbPlayerNum || pNPC->bNeutral) {
    if (pNPC->ubCivilianGroup != NON_CIV_GROUP) {
      // although the soldier is NOW the same side, this civ group could be set to "will become
      // hostile"
      return (gTacticalStatus.fCivGroupHostile[pNPC->ubCivilianGroup] >=
              CIV_GROUP_WILL_BECOME_HOSTILE);
    } else {
      return (FALSE);
    }
  } else {
    return (TRUE);
  }
}

BOOLEAN CheckIfMercIsNearNPC(struct SOLDIERTYPE *pMerc, uint8_t ubProfileId) {
  struct SOLDIERTYPE *pNPC;
  int16_t sGridNo;

  // no merc nearby?
  if (pMerc == NULL) {
    return (FALSE);
  }

  pNPC = FindSoldierByProfileID(ubProfileId, FALSE);
  if (pNPC == NULL) {
    return (FALSE);
  }
  sGridNo = pNPC->sGridNo;

  // is the merc and NPC close enough?
  if (PythSpacesAway(sGridNo, pMerc->sGridNo) <= 9) {
    return (TRUE);
  }

  return (FALSE);
}

int8_t NumWoundedMercsNearby(uint8_t ubProfileID) {
  int8_t bNumber = 0;
  uint32_t uiLoop;
  struct SOLDIERTYPE *pNPC;
  struct SOLDIERTYPE *pSoldier;
  int16_t sGridNo;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  sGridNo = pNPC->sGridNo;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];

    if (pSoldier && pSoldier->bTeam == gbPlayerNum && pSoldier->bLife > 0 &&
        pSoldier->bLife < pSoldier->bLifeMax && pSoldier->bAssignment != ASSIGNMENT_HOSPITAL) {
      if (PythSpacesAway(sGridNo, pSoldier->sGridNo) <= HOSPITAL_PATIENT_DISTANCE) {
        bNumber++;
      }
    }
  }

  return (bNumber);
}

int8_t NumMercsNear(uint8_t ubProfileID, uint8_t ubMaxDist) {
  int8_t bNumber = 0;
  uint32_t uiLoop;
  struct SOLDIERTYPE *pNPC;
  struct SOLDIERTYPE *pSoldier;
  int16_t sGridNo;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  sGridNo = pNPC->sGridNo;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];

    if (pSoldier && pSoldier->bTeam == gbPlayerNum && pSoldier->bLife >= OKLIFE) {
      if (PythSpacesAway(sGridNo, pSoldier->sGridNo) <= ubMaxDist) {
        bNumber++;
      }
    }
  }

  return (bNumber);
}

BOOLEAN CheckNPCIsEPC(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pNPC;

  if (gMercProfiles[ubProfileID].bMercStatus == MERC_IS_DEAD) {
    return (FALSE);
  }

  pNPC = FindSoldierByProfileID(ubProfileID, TRUE);
  if (!pNPC) {
    return (FALSE);
  }
  return ((pNPC->ubWhatKindOfMercAmI == MERC_TYPE__EPC));
}

BOOLEAN NPCInRoom(uint8_t ubProfileID, uint8_t ubRoomID) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC || (gubWorldRoomInfo[pNPC->sGridNo] != ubRoomID)) {
    return (FALSE);
  }
  return (TRUE);
}

BOOLEAN NPCInRoomRange(uint8_t ubProfileID, uint8_t ubRoomID1, uint8_t ubRoomID2) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC || (gubWorldRoomInfo[pNPC->sGridNo] < ubRoomID1) ||
      (gubWorldRoomInfo[pNPC->sGridNo] > ubRoomID2)) {
    return (FALSE);
  }
  return (TRUE);
}

BOOLEAN PCInSameRoom(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pNPC;
  uint8_t ubRoom;
  int8_t bLoop;
  struct SOLDIERTYPE *pSoldier;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  ubRoom = gubWorldRoomInfo[pNPC->sGridNo];

  for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
       bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++) {
    pSoldier = MercPtrs[bLoop];
    if (pSoldier && IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (gubWorldRoomInfo[pSoldier->sGridNo] == ubRoom) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN CheckTalkerStrong(void) {
  if (gpSrcSoldier && gpSrcSoldier->bTeam == gbPlayerNum) {
    return (gpSrcSoldier->bStrength >= 84);
  } else if (gpDestSoldier && gpDestSoldier->bTeam == gbPlayerNum) {
    return (gpDestSoldier->bStrength >= 84);
  }
  return (FALSE);
}

BOOLEAN CheckTalkerFemale(void) {
  if (gpSrcSoldier && gpSrcSoldier->bTeam == gbPlayerNum && gpSrcSoldier->ubProfile != NO_PROFILE) {
    return (gMercProfiles[gpSrcSoldier->ubProfile].bSex == FEMALE);
  } else if (gpDestSoldier && gpDestSoldier->bTeam == gbPlayerNum &&
             gpDestSoldier->ubProfile != NO_PROFILE) {
    return (gMercProfiles[gpDestSoldier->ubProfile].bSex == FEMALE);
  }
  return (FALSE);
}

BOOLEAN CheckTalkerUnpropositionedFemale(void) {
  if (gpSrcSoldier && gpSrcSoldier->bTeam == gbPlayerNum && gpSrcSoldier->ubProfile != NO_PROFILE) {
    if (!(gMercProfiles[gpSrcSoldier->ubProfile].ubMiscFlags2 &
          PROFILE_MISC_FLAG2_ASKED_BY_HICKS)) {
      return (gMercProfiles[gpSrcSoldier->ubProfile].bSex == FEMALE);
    }
  } else if (gpDestSoldier && gpDestSoldier->bTeam == gbPlayerNum &&
             gpDestSoldier->ubProfile != NO_PROFILE) {
    if (!(gMercProfiles[gpDestSoldier->ubProfile].ubMiscFlags2 &
          PROFILE_MISC_FLAG2_ASKED_BY_HICKS)) {
      return (gMercProfiles[gpDestSoldier->ubProfile].bSex == FEMALE);
    }
  }
  return (FALSE);
}

int8_t NumMalesPresent(uint8_t ubProfileID) {
  int8_t bNumber = 0;
  uint32_t uiLoop;
  struct SOLDIERTYPE *pNPC;
  struct SOLDIERTYPE *pSoldier;
  int16_t sGridNo;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  sGridNo = pNPC->sGridNo;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];

    if (pSoldier && pSoldier->bTeam == gbPlayerNum && pSoldier->bLife >= OKLIFE) {
      if (GetSolProfile(pSoldier) != NO_PROFILE &&
          gMercProfiles[GetSolProfile(pSoldier)].bSex == MALE) {
        if (PythSpacesAway(sGridNo, pSoldier->sGridNo) <= 8) {
          bNumber++;
        }
      }
    }
  }

  return (bNumber);
}

BOOLEAN FemalePresent(uint8_t ubProfileID) {
  uint32_t uiLoop;
  struct SOLDIERTYPE *pNPC;
  struct SOLDIERTYPE *pSoldier;
  int16_t sGridNo;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  sGridNo = pNPC->sGridNo;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];

    if (pSoldier && pSoldier->bTeam == gbPlayerNum && pSoldier->bLife >= OKLIFE) {
      if (GetSolProfile(pSoldier) != NO_PROFILE &&
          gMercProfiles[GetSolProfile(pSoldier)].bSex == FEMALE) {
        if (PythSpacesAway(sGridNo, pSoldier->sGridNo) <= 10) {
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN CheckPlayerHasHead(void) {
  int8_t bLoop;
  struct SOLDIERTYPE *pSoldier;

  for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
       bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++) {
    pSoldier = MercPtrs[bLoop];

    if (IsSolActive(pSoldier) && pSoldier->bLife > 0) {
      if (FindObjInObjRange(pSoldier, HEAD_2, HEAD_7) != NO_SLOT) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN CheckNPCSector(uint8_t ubProfileID, uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ) {
  struct SOLDIERTYPE *pSoldier;

  pSoldier = FindSoldierByProfileID(ubProfileID, TRUE);

  if (pSoldier) {
    if (GetSolSectorX(pSoldier) == sSectorX && GetSolSectorY(pSoldier) == sSectorY &&
        GetSolSectorZ(pSoldier) == bSectorZ) {
      return (TRUE);
    }
  } else if (gMercProfiles[ubProfileID].sSectorX == sSectorX &&
             gMercProfiles[ubProfileID].sSectorY == sSectorY &&
             gMercProfiles[ubProfileID].bSectorZ == bSectorZ) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN AIMMercWithin(int16_t sGridNo, int16_t sDistance) {
  uint32_t uiLoop;
  struct SOLDIERTYPE *pSoldier;

  for (uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++) {
    pSoldier = MercSlots[uiLoop];

    if (pSoldier && (pSoldier->bTeam == gbPlayerNum) && (pSoldier->bLife >= OKLIFE) &&
        (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)) {
      if (PythSpacesAway(sGridNo, pSoldier->sGridNo) <= sDistance) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN CheckNPCCowering(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  return (((pNPC->uiStatusFlags & SOLDIER_COWERING) != 0));
}

uint8_t CountBartenders(void) {
  uint8_t ubLoop;
  uint8_t ubBartenders = 0;

  for (ubLoop = HERVE; ubLoop <= CARLO; ubLoop++) {
    if (gMercProfiles[ubLoop].bNPCData != 0) {
      ubBartenders++;
    }
  }
  return (ubBartenders);
}

BOOLEAN CheckNPCIsUnderFire(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  return (pNPC->bUnderFire != 0);
}

BOOLEAN NPCHeardShot(uint8_t ubProfileID) {
  struct SOLDIERTYPE *pNPC;

  pNPC = FindSoldierByProfileID(ubProfileID, FALSE);
  if (!pNPC) {
    return (FALSE);
  }
  return (pNPC->ubMiscSoldierFlags & SOLDIER_MISC_HEARD_GUNSHOT);
}

BOOLEAN InTownSectorWithTrainingLoyalty(uint8_t sSectorX, uint8_t sSectorY) {
  uint8_t ubTown;

  ubTown = GetTownIdForSector(sSectorX, sSectorY);
  if ((ubTown != BLANK_SECTOR) && gTownLoyalty[ubTown].fStarted && gfTownUsesLoyalty[ubTown]) {
    return (gTownLoyalty[ubTown].ubRating >= MIN_RATING_TO_TRAIN_TOWN);
  } else {
    return (FALSE);
  }
}

BOOLEAN CheckFact(uint16_t usFact, uint8_t ubProfileID) {
  int8_t bTown = -1;

  switch (usFact) {
    case FACT_DIMITRI_DEAD:
      gubFact[usFact] = (gMercProfiles[DIMITRI].bMercStatus == MERC_IS_DEAD);
      break;
    case FACT_CURRENT_SECTOR_IS_SAFE:
      gubFact[FACT_CURRENT_SECTOR_IS_SAFE] =
          !(((gTacticalStatus.fEnemyInSector && NPCHeardShot(ubProfileID)) ||
             gTacticalStatus.uiFlags & INCOMBAT));
      break;
    case FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT:
    case FACT_NEW_BOBBYRAY_SHIPMENT_WAITING:
      if (gubFact[FACT_PABLO_PUNISHED_BY_PLAYER] == TRUE &&
          gubFact[FACT_PABLO_RETURNED_GOODS] == FALSE &&
          gMercProfiles[PABLO].bMercStatus != MERC_IS_DEAD) {
        gubFact[FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT] = FALSE;
        gubFact[FACT_NEW_BOBBYRAY_SHIPMENT_WAITING] = FALSE;
      } else {
        if (CheckForNewShipment())  // if new stuff waiting unseen in Drassen
        {
          gubFact[FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT] = FALSE;
          gubFact[FACT_NEW_BOBBYRAY_SHIPMENT_WAITING] = TRUE;
        } else if (CountNumberOfBobbyPurchasesThatAreInTransit() > 0)  // if stuff in transit
        {
          if (gubFact[FACT_PACKAGE_DAMAGED] == TRUE) {
            gubFact[FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT] = FALSE;
          } else {
            gubFact[FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT] = TRUE;
          }
          gubFact[FACT_NEW_BOBBYRAY_SHIPMENT_WAITING] = FALSE;
        } else {
          gubFact[FACT_BOBBYRAY_SHIPMENT_IN_TRANSIT] = FALSE;
          gubFact[FACT_NEW_BOBBYRAY_SHIPMENT_WAITING] = FALSE;
        }
      }
      break;
    case FACT_NPC_WOUNDED:
      gubFact[FACT_NPC_WOUNDED] = CheckNPCWounded(ubProfileID, FALSE);
      break;
    case FACT_NPC_WOUNDED_BY_PLAYER:
      gubFact[FACT_NPC_WOUNDED_BY_PLAYER] = CheckNPCWounded(ubProfileID, TRUE);
      break;
    case FACT_IRA_NOT_PRESENT:
      gubFact[FACT_IRA_NOT_PRESENT] = !CheckNPCWithin(ubProfileID, IRA, 10);
      break;
    case FACT_IRA_TALKING:
      gubFact[FACT_IRA_TALKING] = (gubSrcSoldierProfile == 59);
      break;
    case FACT_IRA_UNHIRED_AND_ALIVE:
      if (gMercProfiles[IRA].bMercStatus != MERC_IS_DEAD && CheckNPCSector(IRA, 10, 1, 1) &&
          !(gMercProfiles[IRA].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED)) {
        gubFact[FACT_IRA_UNHIRED_AND_ALIVE] = TRUE;
      } else {
        gubFact[FACT_IRA_UNHIRED_AND_ALIVE] = FALSE;
      }
      break;
    case FACT_NPC_BLEEDING:
      gubFact[FACT_NPC_BLEEDING] = CheckNPCBleeding(ubProfileID);
      break;
    case FACT_NPC_BLEEDING_BUT_OKAY:
      if (CheckNPCBleeding(ubProfileID) && CheckNPCInOkayHealth(ubProfileID)) {
        gubFact[FACT_NPC_BLEEDING_BUT_OKAY] = TRUE;
      } else {
        gubFact[FACT_NPC_BLEEDING_BUT_OKAY] = FALSE;
      }
      break;

    case FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_SAN_MONA:
      gubFact[usFact] = (CheckNPCSector(CARMEN, 5, MAP_ROW_C, 0) && CheckPlayerHasHead());
      break;

    case FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_CAMBRIA:
      gubFact[usFact] = (CheckNPCSector(CARMEN, 9, MAP_ROW_G, 0) && CheckPlayerHasHead());
      break;

    case FACT_PLAYER_HAS_HEAD_AND_CARMEN_IN_DRASSEN:
      gubFact[usFact] = (CheckNPCSector(CARMEN, 13, MAP_ROW_C, 0) && CheckPlayerHasHead());
      break;

    case FACT_NPC_OWED_MONEY:
      gubFact[FACT_NPC_OWED_MONEY] = (gMercProfiles[ubProfileID].iBalance < 0);
      break;

    case FACT_FATHER_DRUNK:
      gubFact[FACT_FATHER_DRUNK] = (gMercProfiles[FATHER].bNPCData >= 5);
      break;

    case FACT_MICKY_DRUNK:
      gubFact[FACT_MICKY_DRUNK] = (gMercProfiles[MICKY].bNPCData >= 5);
      break;

    case FACT_BRENDA_IN_STORE_AND_ALIVE:
      // ensure alive
      if (gMercProfiles[85].bMercStatus == MERC_IS_DEAD) {
        gubFact[FACT_BRENDA_IN_STORE_AND_ALIVE] = FALSE;
      }
      // ensure in a building and nearby
      else if (!(NPCInRoom(85, 47))) {
        gubFact[FACT_BRENDA_IN_STORE_AND_ALIVE] = FALSE;
      } else {
        gubFact[FACT_BRENDA_IN_STORE_AND_ALIVE] = CheckNPCWithin(ubProfileID, 85, 12);
      }
      break;
    case FACT_BRENDA_DEAD:
      gubFact[FACT_BRENDA_DEAD] = (gMercProfiles[85].bMercStatus == MERC_IS_DEAD);
      break;
    case FACT_NPC_IS_ENEMY:
      gubFact[FACT_NPC_IS_ENEMY] =
          CheckNPCIsEnemy(ubProfileID) ||
          gMercProfiles[ubProfileID].ubMiscFlags2 & PROFILE_MISC_FLAG2_NEEDS_TO_SAY_HOSTILE_QUOTE;
      break;
      /*
case FACT_SKYRIDER_CLOSE_TO_CHOPPER:
      SetUpHelicopterForPlayer( 13, MAP_ROW_B );
      break;
      */
    case FACT_SPIKE_AT_DOOR:
      gubFact[FACT_SPIKE_AT_DOOR] = CheckNPCAt(93, 9817);
      break;
    case FACT_WOUNDED_MERCS_NEARBY:
      gubFact[usFact] = (NumWoundedMercsNearby(ubProfileID) > 0);
      break;
    case FACT_ONE_WOUNDED_MERC_NEARBY:
      gubFact[usFact] = (NumWoundedMercsNearby(ubProfileID) == 1);
      break;
    case FACT_MULTIPLE_WOUNDED_MERCS_NEARBY:
      gubFact[usFact] = (NumWoundedMercsNearby(ubProfileID) > 1);
      break;
    case FACT_HANS_AT_SPOT:
      gubFact[usFact] = CheckNPCAt(117, 13523);
      break;
    case FACT_MULTIPLE_MERCS_CLOSE:
      gubFact[usFact] = (NumMercsNear(ubProfileID, 3) > 1);
      break;
    case FACT_SOME_MERCS_CLOSE:
      gubFact[usFact] = (NumMercsNear(ubProfileID, 3) > 0);
      break;
    case FACT_MARIA_ESCORTED:
      gubFact[usFact] = CheckNPCIsEPC(MARIA);
      break;
    case FACT_JOEY_ESCORTED:
      gubFact[usFact] = CheckNPCIsEPC(JOEY);
      break;
    case FACT_ESCORTING_SKYRIDER:
      gubFact[usFact] = CheckNPCIsEPC(SKYRIDER);
      break;
    case FACT_MARIA_ESCORTED_AT_LEATHER_SHOP:
      gubFact[usFact] = (CheckNPCIsEPC(MARIA) && (NPCInRoom(MARIA, 2)));
      break;
    case FACT_PC_STRONG_AND_LESS_THAN_3_MALES_PRESENT:
      gubFact[usFact] = (CheckTalkerStrong() && (NumMalesPresent(ubProfileID) < 3));
      break;
    case FACT_PC_STRONG_AND_3_PLUS_MALES_PRESENT:
      gubFact[usFact] = (CheckTalkerStrong() && (NumMalesPresent(ubProfileID) >= 3));
      break;
    case FACT_FEMALE_SPEAKING_TO_NPC:
      gubFact[usFact] = CheckTalkerFemale();
      break;
    case FACT_CARMEN_IN_C5:
      gubFact[usFact] = CheckNPCSector(78, 5, MAP_ROW_C, 0);
      break;
    case FACT_JOEY_IN_C5:
      gubFact[usFact] = CheckNPCSector(90, 5, MAP_ROW_C, 0);
      break;
    case FACT_JOEY_NEAR_MARTHA:
      gubFact[usFact] = CheckNPCWithin(90, 109, 5) &&
                        (CheckGuyVisible(MARTHA, JOEY) || CheckGuyVisible(JOEY, MARTHA));
      break;
    case FACT_JOEY_DEAD:
      gubFact[usFact] = gMercProfiles[JOEY].bMercStatus == MERC_IS_DEAD;
      break;
    case FACT_MERC_NEAR_MARTHA:
      gubFact[usFact] = (NumMercsNear(ubProfileID, 5) > 0);
      break;
    case FACT_REBELS_HATE_PLAYER:
      gubFact[usFact] = (gTacticalStatus.fCivGroupHostile[REBEL_CIV_GROUP] == CIV_GROUP_HOSTILE);
      break;
    case FACT_CURRENT_SECTOR_G9:
      gubFact[usFact] = (gWorldSectorX == 9 && gWorldSectorY == MAP_ROW_G && gbWorldSectorZ == 0);
      break;
    case FACT_CURRENT_SECTOR_C5:
      gubFact[usFact] = (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0);
      break;
    case FACT_CURRENT_SECTOR_C13:
      gubFact[usFact] = (gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0);
      break;
    case FACT_CARMEN_HAS_TEN_THOUSAND:
      gubFact[usFact] = (gMercProfiles[78].uiMoney >= 10000);
      break;
    case FACT_SLAY_IN_SECTOR:
      gubFact[usFact] = (gMercProfiles[SLAY].sSectorX == gWorldSectorX &&
                         gMercProfiles[SLAY].sSectorY == gWorldSectorY &&
                         gMercProfiles[SLAY].bSectorZ == gbWorldSectorZ);
      break;
    case FACT_SLAY_HIRED_AND_WORKED_FOR_48_HOURS:
      gubFact[usFact] = ((gMercProfiles[SLAY].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) &&
                         (gMercProfiles[SLAY].usTotalDaysServed > 1));
      break;
    case FACT_SHANK_IN_SQUAD_BUT_NOT_SPEAKING:
      gubFact[usFact] = ((FindSoldierByProfileID(SHANK, TRUE) != NULL) &&
                         (gMercProfiles[SHANK].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) &&
                         (gpSrcSoldier == NULL || gpSrcSoldier->ubProfile != SHANK));
      break;
    case FACT_SHANK_NOT_IN_SECTOR:
      gubFact[usFact] = (FindSoldierByProfileID(SHANK, FALSE) == NULL);
      break;
    case FACT_QUEEN_DEAD:
      gubFact[usFact] = (gMercProfiles[QUEEN].bMercStatus == MERC_IS_DEAD);
      break;
    case FACT_MINE_EMPTY:
      gubFact[usFact] = IsHisMineEmpty(ubProfileID);
      break;
    case FACT_MINE_RUNNING_OUT:
      gubFact[usFact] = IsHisMineRunningOut(ubProfileID);
      break;
    case FACT_MINE_PRODUCING_BUT_LOYALTY_LOW:
      gubFact[usFact] = HasHisMineBeenProducingForPlayerForSomeTime(ubProfileID) &&
                        IsHisMineDisloyal(ubProfileID);
      break;
    case FACT_CREATURES_IN_MINE:
      gubFact[usFact] = IsHisMineInfested(ubProfileID);
      break;
    case FACT_PLAYER_LOST_MINE:
      gubFact[usFact] = IsHisMineLostAndRegained(ubProfileID);
      break;
    case FACT_MINE_AT_FULL_PRODUCTION:
      gubFact[usFact] = IsHisMineAtMaxProduction(ubProfileID);
      break;
    case FACT_DYNAMO_IN_J9:
      gubFact[usFact] = CheckNPCSector(DYNAMO, 9, MAP_ROW_J, 0) && NumEnemiesInAnySector(9, 10, 0);
      break;
    case FACT_DYNAMO_ALIVE:
      gubFact[usFact] = (gMercProfiles[DYNAMO].bMercStatus != MERC_IS_DEAD);
      break;
    case FACT_DYNAMO_SPEAKING_OR_NEARBY:
      gubFact[usFact] =
          (gpSrcSoldier != NULL && (gpSrcSoldier->ubProfile == DYNAMO ||
                                    (CheckNPCWithin(gpSrcSoldier->ubProfile, DYNAMO, 10) &&
                                     CheckGuyVisible(gpSrcSoldier->ubProfile, DYNAMO))));
      break;
    case FACT_JOHN_EPC:
      gubFact[usFact] = CheckNPCIsEPC(JOHN);
      break;
    case FACT_MARY_EPC:
      gubFact[usFact] = CheckNPCIsEPC(MARY);
      break;
    case FACT_JOHN_AND_MARY_EPCS:
      gubFact[usFact] = CheckNPCIsEPC(JOHN) && CheckNPCIsEPC(MARY);
      break;
    case FACT_MARY_ALIVE:
      gubFact[usFact] = (gMercProfiles[MARY].bMercStatus != MERC_IS_DEAD);
      break;
    case FACT_MARY_BLEEDING:
      gubFact[usFact] = CheckNPCBleeding(MARY);
      break;
    case FACT_JOHN_ALIVE:
      gubFact[usFact] = (gMercProfiles[JOHN].bMercStatus != MERC_IS_DEAD);
      break;
    case FACT_JOHN_BLEEDING:
      gubFact[usFact] = CheckNPCBleeding(JOHN);
      break;
    case FACT_MARY_DEAD:
      gubFact[usFact] = (gMercProfiles[MARY].bMercStatus == MERC_IS_DEAD);
      break;

    case FACT_ANOTHER_FIGHT_POSSIBLE:
      gubFact[usFact] = AnotherFightPossible();
      break;

    case FACT_RECEIVING_INCOME_FROM_DCAC:
      gubFact[usFact] = ((PredictDailyIncomeFromAMine(MINE_DRASSEN) > 0) &&
                         (PredictDailyIncomeFromAMine(MINE_ALMA) > 0) &&
                         (PredictDailyIncomeFromAMine(MINE_CAMBRIA) > 0) &&
                         (PredictDailyIncomeFromAMine(MINE_CHITZENA) > 0));
      break;

    case FACT_PLAYER_BEEN_TO_K4: {
      UNDERGROUND_SECTORINFO *pUnderGroundSector;

      pUnderGroundSector = FindUnderGroundSector(4, MAP_ROW_K, 1);
      gubFact[usFact] = pUnderGroundSector->fVisited;
    } break;
    case FACT_WARDEN_DEAD:
      gubFact[usFact] = (gMercProfiles[WARDEN].bMercStatus == MERC_IS_DEAD);
      break;

    case FACT_PLAYER_PAID_FOR_TWO_IN_BROTHEL:
      gubFact[usFact] = (gMercProfiles[MADAME].bNPCData > 1);
      break;

    case FACT_LOYALTY_OKAY:
      bTown = gMercProfiles[ubProfileID].bTown;
      if ((bTown != BLANK_SECTOR) && gTownLoyalty[bTown].fStarted && gfTownUsesLoyalty[bTown]) {
        gubFact[usFact] = ((gTownLoyalty[bTown].ubRating >= LOYALTY_LOW_THRESHOLD) &&
                           (gTownLoyalty[bTown].ubRating < LOYALTY_OK_THRESHOLD));
      } else {
        gubFact[usFact] = FALSE;
      }
      break;

    case FACT_LOYALTY_LOW:
      bTown = gMercProfiles[ubProfileID].bTown;
      if ((bTown != BLANK_SECTOR) && gTownLoyalty[bTown].fStarted && gfTownUsesLoyalty[bTown]) {
        // if Skyrider, ignore low loyalty until he has monologues, and wait at least a day since
        // the latest monologue to avoid a hot/cold attitude
        if ((ubProfileID == SKYRIDER) &&
            ((guiHelicopterSkyriderTalkState == 0) ||
             ((GetWorldTotalMin() - guiTimeOfLastSkyriderMonologue) < (24 * 60)))) {
          gubFact[usFact] = FALSE;
        } else {
          gubFact[usFact] = (gTownLoyalty[bTown].ubRating < LOYALTY_LOW_THRESHOLD);
        }
      } else {
        gubFact[usFact] = FALSE;
      }
      break;

    case FACT_LOYALTY_HIGH:
      bTown = gMercProfiles[ubProfileID].bTown;
      if ((bTown != BLANK_SECTOR) && gTownLoyalty[bTown].fStarted && gfTownUsesLoyalty[bTown]) {
        gubFact[usFact] =
            (gTownLoyalty[gMercProfiles[ubProfileID].bTown].ubRating >= LOYALTY_HIGH_THRESHOLD);
      } else {
        gubFact[usFact] = FALSE;
      }
      break;

    case FACT_ELGIN_ALIVE:
      gubFact[usFact] = (gMercProfiles[DRUGGIST].bMercStatus != MERC_IS_DEAD);
      break;

    case FACT_SPEAKER_AIM_OR_AIM_NEARBY:
      gubFact[usFact] = gpDestSoldier && AIMMercWithin(gpDestSoldier->sGridNo, 10);
      break;

    case FACT_MALE_SPEAKING_FEMALE_PRESENT:
      gubFact[usFact] = (!CheckTalkerFemale() && FemalePresent(ubProfileID));
      break;

    case FACT_PLAYER_OWNS_2_TOWNS_INCLUDING_OMERTA:
      gubFact[usFact] = ((GetNumberOfWholeTownsUnderControl() == 3) &&
                         IsTownUnderCompleteControlByPlayer(OMERTA));
      break;

    case FACT_PLAYER_OWNS_3_TOWNS_INCLUDING_OMERTA:
      gubFact[usFact] = ((GetNumberOfWholeTownsUnderControl() == 5) &&
                         IsTownUnderCompleteControlByPlayer(OMERTA));
      break;

    case FACT_PLAYER_OWNS_4_TOWNS_INCLUDING_OMERTA:
      gubFact[usFact] = ((GetNumberOfWholeTownsUnderControl() >= 6) &&
                         IsTownUnderCompleteControlByPlayer(OMERTA));
      break;

    case FACT_PLAYER_FOUGHT_THREE_TIMES_TODAY:
      gubFact[usFact] = !BoxerAvailable();
      break;

    case FACT_PLAYER_DOING_POORLY:
      gubFact[usFact] = (CurrentPlayerProgressPercentage() < 20);
      break;

    case FACT_PLAYER_DOING_WELL:
      gubFact[usFact] = (CurrentPlayerProgressPercentage() > 50);
      break;

    case FACT_PLAYER_DOING_VERY_WELL:
      gubFact[usFact] = (CurrentPlayerProgressPercentage() > 80);
      break;

    case FACT_FATHER_DRUNK_AND_SCIFI_OPTION_ON:
      gubFact[usFact] = ((gMercProfiles[FATHER].bNPCData >= 5) && gGameOptions.fSciFi);
      break;

    case FACT_BLOODCAT_QUEST_STARTED_TWO_DAYS_AGO:
      gubFact[usFact] = ((gubQuest[QUEST_BLOODCATS] != QUESTNOTSTARTED) &&
                         (GetWorldTotalMin() - GetTimeQuestWasStarted(QUEST_BLOODCATS) >
                          2 * NUM_SEC_IN_DAY / NUM_SEC_IN_MIN));
      break;

    case FACT_NOTHING_REPAIRED_YET:
      gubFact[usFact] = RepairmanIsFixingItemsButNoneAreDoneYet(ubProfileID);
      break;

    case FACT_NPC_COWERING:
      gubFact[usFact] = CheckNPCCowering(ubProfileID);
      break;

    case FACT_TOP_AND_BOTTOM_LEVELS_CLEARED:
      gubFact[usFact] = (gubFact[FACT_TOP_LEVEL_CLEARED] && gubFact[FACT_BOTTOM_LEVEL_CLEARED]);
      break;

    case FACT_FIRST_BARTENDER:
      gubFact[usFact] = (gMercProfiles[ubProfileID].bNPCData == 1 ||
                         (gMercProfiles[ubProfileID].bNPCData == 0 && CountBartenders() == 0));
      break;

    case FACT_SECOND_BARTENDER:
      gubFact[usFact] = (gMercProfiles[ubProfileID].bNPCData == 2 ||
                         (gMercProfiles[ubProfileID].bNPCData == 0 && CountBartenders() == 1));
      break;

    case FACT_THIRD_BARTENDER:
      gubFact[usFact] = (gMercProfiles[ubProfileID].bNPCData == 3 ||
                         (gMercProfiles[ubProfileID].bNPCData == 0 && CountBartenders() == 2));
      break;

    case FACT_FOURTH_BARTENDER:
      gubFact[usFact] = (gMercProfiles[ubProfileID].bNPCData == 4 ||
                         (gMercProfiles[ubProfileID].bNPCData == 0 && CountBartenders() == 3));
      break;

    case FACT_NPC_NOT_UNDER_FIRE:
      gubFact[usFact] = !CheckNPCIsUnderFire(ubProfileID);
      break;

    case FACT_KINGPIN_NOT_IN_OFFICE:
      gubFact[usFact] =
          !(gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_D && NPCInRoomRange(KINGPIN, 30, 39));
      // 30 to 39
      break;

    case FACT_DONT_OWE_KINGPIN_MONEY:
      gubFact[usFact] = (gubQuest[QUEST_KINGPIN_MONEY] != QUESTINPROGRESS);
      break;

    case FACT_NO_CLUB_FIGHTING_ALLOWED:
      gubFact[usFact] = (gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS ||
                         gfBoxersResting);  // plus other conditions
      break;

    case FACT_MADDOG_IS_SPEAKER:
      gubFact[usFact] = (gubSrcSoldierProfile == MADDOG);
      break;

    case FACT_PC_HAS_CONRADS_RECRUIT_OPINION:
      gubFact[usFact] =
          (gpDestSoldier && (CalcDesireToTalk(gpDestSoldier->ubProfile, gubSrcSoldierProfile,
                                              APPROACH_RECRUIT) >= 50));
      break;

    case FACT_NPC_HOSTILE_OR_PISSED_OFF:
      gubFact[usFact] = CheckNPCIsEnemy(ubProfileID) || (gMercProfiles[ubProfileID].ubMiscFlags3 &
                                                         PROFILE_MISC_FLAG3_NPC_PISSED_OFF);
      break;

    case FACT_TONY_IN_BUILDING:
      gubFact[usFact] = CheckNPCSector(TONY, 5, MAP_ROW_C, 0) && NPCInRoom(TONY, 50);
      break;

    case FACT_SHANK_SPEAKING:
      gubFact[usFact] = (gpSrcSoldier && gpSrcSoldier->ubProfile == SHANK);
      break;

    case FACT_ROCKET_RIFLE_EXISTS:
      gubFact[usFact] = ItemTypeExistsAtLocation(10472, ROCKET_RIFLE, 0, NULL);
      break;

    case FACT_DOREEN_ALIVE:
      gubFact[usFact] = gMercProfiles[DOREEN].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_WALDO_ALIVE:
      gubFact[usFact] = gMercProfiles[WALDO].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_PERKO_ALIVE:
      gubFact[usFact] = gMercProfiles[PERKO].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_TONY_ALIVE:
      gubFact[usFact] = gMercProfiles[TONY].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_VINCE_ALIVE:
      gubFact[usFact] = gMercProfiles[VINCE].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_JENNY_ALIVE:
      gubFact[usFact] = gMercProfiles[JENNY].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_ARNOLD_ALIVE:
      gubFact[usFact] = gMercProfiles[ARNIE].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_I16_BLOODCATS_KILLED:
      gubFact[usFact] = (SectorInfo[SEC_I16].bBloodCats == 0);
      break;

    case FACT_NPC_BANDAGED_TODAY:
      gubFact[usFact] =
          (gMercProfiles[ubProfileID].ubMiscFlags2 & PROFILE_MISC_FLAG2_BANDAGED_TODAY) != 0;
      break;

    case FACT_PLAYER_IN_SAME_ROOM:
      gubFact[usFact] = PCInSameRoom(ubProfileID);
      break;

    case FACT_PLAYER_SPOKE_TO_DRASSEN_MINER:
      gubFact[usFact] = SpokenToHeadMiner(MINE_DRASSEN);
      break;
    case FACT_PLAYER_IN_CONTROLLED_DRASSEN_MINE:
      gubFact[usFact] =
          (GetIdOfMineForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ) ==
               MINE_DRASSEN &&
           !(StrategicMap[GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
                 .fEnemyControlled));
      break;
    case FACT_PLAYER_SPOKE_TO_CAMBRIA_MINER:
      gubFact[usFact] = SpokenToHeadMiner(MINE_CAMBRIA);
      break;
    case FACT_PLAYER_IN_CONTROLLED_CAMBRIA_MINE:
      gubFact[usFact] =
          (GetIdOfMineForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ) ==
               MINE_CAMBRIA &&
           !(StrategicMap[GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
                 .fEnemyControlled));
      break;
    case FACT_PLAYER_SPOKE_TO_CHITZENA_MINER:
      gubFact[usFact] = SpokenToHeadMiner(MINE_CHITZENA);
      break;
    case FACT_PLAYER_IN_CONTROLLED_CHITZENA_MINE:
      gubFact[usFact] =
          (GetIdOfMineForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ) ==
               MINE_CHITZENA &&
           !(StrategicMap[GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
                 .fEnemyControlled));
      break;
    case FACT_PLAYER_SPOKE_TO_ALMA_MINER:
      gubFact[usFact] = SpokenToHeadMiner(MINE_ALMA);
      break;
    case FACT_PLAYER_IN_CONTROLLED_ALMA_MINE:
      gubFact[usFact] =
          (GetIdOfMineForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ) ==
               MINE_ALMA &&
           !(StrategicMap[GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
                 .fEnemyControlled));
      break;
    case FACT_PLAYER_SPOKE_TO_GRUMM_MINER:
      gubFact[usFact] = SpokenToHeadMiner(MINE_GRUMM);
      break;
    case FACT_PLAYER_IN_CONTROLLED_GRUMM_MINE:
      gubFact[usFact] =
          (GetIdOfMineForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ) ==
               MINE_GRUMM &&
           !(StrategicMap[GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
                 .fEnemyControlled));
      break;

    case FACT_ENOUGH_LOYALTY_TO_TRAIN_MILITIA:
      gubFact[usFact] =
          InTownSectorWithTrainingLoyalty((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
      break;

    case FACT_WALKER_AT_BAR:
      gubFact[usFact] =
          (gMercProfiles[FATHER].sSectorX == 13 && gMercProfiles[FATHER].sSectorY == MAP_ROW_C);
      break;

    case FACT_JOEY_ALIVE:
      gubFact[usFact] = gMercProfiles[JOEY].bMercStatus != MERC_IS_DEAD;
      break;

    case FACT_UNPROPOSITIONED_FEMALE_SPEAKING_TO_NPC:
      gubFact[usFact] = CheckTalkerUnpropositionedFemale();
      break;

    case FACT_84_AND_85_TRUE:
      gubFact[usFact] = CheckFact(84, ubProfileID) && CheckFact(FACT_HANS_AT_SPOT, ubProfileID);
      break;

    case FACT_SKYRIDER_IN_B15:
      gubFact[usFact] = CheckNPCSector(SKYRIDER, 15, MAP_ROW_B, 0);
      break;

    case FACT_SKYRIDER_IN_C16:
      gubFact[usFact] = CheckNPCSector(SKYRIDER, 16, MAP_ROW_C, 0);
      break;
    case FACT_SKYRIDER_IN_E14:
      gubFact[usFact] = CheckNPCSector(SKYRIDER, 14, MAP_ROW_E, 0);
      break;
    case FACT_SKYRIDER_IN_D12:
      gubFact[usFact] = CheckNPCSector(SKYRIDER, 12, MAP_ROW_D, 0);
      break;

    case FACT_KINGPIN_IS_ENEMY:
      gubFact[usFact] =
          (gTacticalStatus.fCivGroupHostile[KINGPIN_CIV_GROUP] >= CIV_GROUP_WILL_BECOME_HOSTILE);
      break;

    case FACT_DYNAMO_NOT_SPEAKER:
      gubFact[usFact] = !(gpSrcSoldier != NULL && (gpSrcSoldier->ubProfile == DYNAMO));
      break;

    case FACT_PABLO_BRIBED:
      gubFact[usFact] = !CheckFact(FACT_PABLOS_BRIBED, ubProfileID);
      break;

    case FACT_VEHICLE_PRESENT:
      gubFact[usFact] = CheckFact(FACT_OK_USE_HUMMER, ubProfileID) &&
                        ((FindSoldierByProfileID(PROF_HUMMER, TRUE) != NULL) ||
                         (FindSoldierByProfileID(PROF_ICECREAM, TRUE) != NULL));
      break;

    case FACT_PLAYER_KILLED_BOXERS:
      gubFact[usFact] = !BoxerExists();
      break;

    case 245:  // Can dimitri be recruited? should be true if already true, OR if Miguel has been
               // recruited already
      gubFact[usFact] = (gubFact[usFact] || FindSoldierByProfileID(MIGUEL, TRUE));
      /*
                      case FACT_:
                              gubFact[usFact] = ;
                              break;
      */

    default:
      break;
  }
  return (gubFact[usFact]);
}

void StartQuest(uint8_t ubQuest, uint8_t sSectorX, uint8_t sSectorY) {
  InternalStartQuest(ubQuest, sSectorX, sSectorY, TRUE);
}

void InternalStartQuest(uint8_t ubQuest, uint8_t sSectorX, uint8_t sSectorY,
                        BOOLEAN fUpdateHistory) {
  if (gubQuest[ubQuest] == QUESTNOTSTARTED) {
    gubQuest[ubQuest] = QUESTINPROGRESS;

    if (fUpdateHistory) {
      SetHistoryFact(HISTORY_QUEST_STARTED, ubQuest, GetWorldTotalMin(), sSectorX, sSectorY);
    }
  } else {
    gubQuest[ubQuest] = QUESTINPROGRESS;
  }
}

void EndQuest(uint8_t ubQuest, uint8_t sSectorX, uint8_t sSectorY) {
  InternalEndQuest(ubQuest, sSectorX, sSectorY, TRUE);
}

void InternalEndQuest(uint8_t ubQuest, uint8_t sSectorX, uint8_t sSectorY, BOOLEAN fUpdateHistory) {
  if (gubQuest[ubQuest] == QUESTINPROGRESS) {
    gubQuest[ubQuest] = QUESTDONE;

    if (fUpdateHistory) {
      ResetHistoryFact(ubQuest, sSectorX, sSectorY);
    }
  } else {
    gubQuest[ubQuest] = QUESTDONE;
  }

  if (ubQuest == QUEST_RESCUE_MARIA) {
    // cheap hack to try to prevent Madame Layla from thinking that you are
    // still in the brothel with Maria...
    gMercProfiles[MADAME].bNPCData = 0;
    gMercProfiles[MADAME].bNPCData2 = 0;
  }
};

void InitQuestEngine() {
  memset(gubQuest, 0, sizeof(gubQuest));
  memset(gubFact, 0, sizeof(gubFact));

  // semi-hack to make the letter quest start right away
  CheckForQuests(1);

  if (gGameOptions.fSciFi) {
    // 3 medical boosters
    gubCambriaMedicalObjects = 21;
  } else {
    gubCambriaMedicalObjects = 18;
  }

  gubBoxingMatchesWon = 0;
  gubBoxersRests = 0;
  gfBoxersResting = FALSE;
}

void CheckForQuests(uint32_t uiDay) {
  // This function gets called at 8:00 AM time of the day

#ifdef TESTQUESTS
  ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Checking For Quests, Day %d", uiDay);
#endif

  // -------------------------------------------------------------------------------
  // QUEST 0 : DELIVER LETTER
  // -------------------------------------------------------------------------------
  // The game always starts with DELIVER LETTER quest, so turn it on if it hasn't
  // already started
  if (gubQuest[QUEST_DELIVER_LETTER] == QUESTNOTSTARTED) {
    StartQuest(QUEST_DELIVER_LETTER, -1, -1);
#ifdef TESTQUESTS
    ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Started DELIVER LETTER quest");
#endif
  }

  // This quest gets turned OFF through conversation with Miguel - when user hands
  // Miguel the letter
}

BOOLEAN SaveQuestInfoToSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten;

  // Save all the states if the Quests
  FileMan_Write(hFile, gubQuest, MAX_QUESTS, &uiNumBytesWritten);
  if (uiNumBytesWritten != MAX_QUESTS) {
    return (FALSE);
  }

  // Save all the states for the facts
  FileMan_Write(hFile, gubFact, NUM_FACTS, &uiNumBytesWritten);
  if (uiNumBytesWritten != NUM_FACTS) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadQuestInfoFromSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead;

  // Save all the states if the Quests
  FileMan_Read(hFile, gubQuest, MAX_QUESTS, &uiNumBytesRead);
  if (uiNumBytesRead != MAX_QUESTS) {
    return (FALSE);
  }

  // Save all the states for the facts
  FileMan_Read(hFile, gubFact, NUM_FACTS, &uiNumBytesRead);
  if (uiNumBytesRead != NUM_FACTS) {
    return (FALSE);
  }

  return (TRUE);
}
