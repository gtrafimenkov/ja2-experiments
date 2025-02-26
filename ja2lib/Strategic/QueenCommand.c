// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/QueenCommand.h"

#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/CampaignInit.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/AnimationData.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Items.h"
#include "Tactical/Morale.h"
#include "Tactical/Overhead.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TacticalSave.h"
#include "TileEngine/MapEdgepoints.h"
#include "TileEngine/RenderWorld.h"
#include "TownMilitia.h"
#include "UI.h"
#include "Utils/Message.h"

#ifdef JA2BETAVERSION
extern BOOLEAN gfClearCreatureQuest;
#endif

// The sector information required for the strategic AI.  Contains the number of enemy troops,
// as well as intentions, etc.
SECTORINFO SectorInfo[256];
UNDERGROUND_SECTORINFO *gpUndergroundSectorInfoHead = NULL;
extern UNDERGROUND_SECTORINFO *gpUndergroundSectorInfoTail;
BOOLEAN gfPendingEnemies = FALSE;
extern void BuildUndergroundSectorInfoList();

extern void EndCreatureQuest();

extern GARRISON_GROUP *gGarrisonGroup;
extern int32_t giGarrisonArraySize;

#ifdef JA2TESTVERSION
extern BOOLEAN gfOverrideSector;
#endif

int16_t gsInterrogationGridNo[3] = {7756, 7757, 7758};

void ValidateEnemiesHaveWeapons() {
#ifdef JA2BETAVERSION
  SGPRect CenteringRect = {0, 0, 639, 479};
  int32_t i, iErrorDialog;
  struct SOLDIERTYPE *pSoldier;
  int32_t iNumInvalid = 0;

  for (i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID; i <= gTacticalStatus.Team[ENEMY_TEAM].bLastID;
       i++) {
    pSoldier = MercPtrs[i];
    if (!IsSolActive(pSoldier) || !pSoldier->bInSector) {
      continue;
    }
    if (!pSoldier->inv[HANDPOS].usItem) {
      iNumInvalid++;
    }
  }

  // do message box and return
  if (iNumInvalid) {
    wchar_t str[100];
    swprintf(str, ARR_SIZE(str),
             L"%d enemies have been added without any weapons!  KM:0.  Please note sector.",
             iNumInvalid);
    iErrorDialog =
        DoMessageBox(MSG_BOX_BASIC_STYLE, str, GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, &CenteringRect);
  }
#endif
}

// Counts enemies and crepitus, but not bloodcats.
uint8_t NumHostilesInSector(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ) {
  uint8_t ubNumHostiles = 0;

  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);
  Assert(sSectorZ >= 0 && sSectorZ <= 3);

  if (sSectorZ) {
    UNDERGROUND_SECTORINFO *pSector;
    pSector = FindUnderGroundSector(sSectorX, sSectorY, (uint8_t)sSectorZ);
    if (pSector) {
      ubNumHostiles = (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites +
                                pSector->ubNumCreatures);
    }
  } else {
    SECTORINFO *pSector;
    struct GROUP *pGroup;

    // Count stationary hostiles
    pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];
    ubNumHostiles = (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites +
                              pSector->ubNumCreatures);

    // Count mobile enemies
    pGroup = gpGroupList;
    while (pGroup) {
      if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == sSectorX &&
          pGroup->ubSectorY == sSectorY) {
        ubNumHostiles += pGroup->ubGroupSize;
      }
      pGroup = pGroup->next;
    }
  }

  return ubNumHostiles;
}

uint8_t NumEnemiesInAnySector(uint8_t sSectorX, uint8_t sSectorY, int8_t sSectorZ) {
  uint8_t ubNumEnemies = 0;

  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);
  Assert(sSectorZ >= 0 && sSectorZ <= 3);

  if (sSectorZ) {
    UNDERGROUND_SECTORINFO *pSector;
    pSector = FindUnderGroundSector(sSectorX, sSectorY, (uint8_t)sSectorZ);
    if (pSector) {
      ubNumEnemies = (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
    }
  } else {
    SECTORINFO *pSector;
    struct GROUP *pGroup;

    // Count stationary enemies
    pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];
    ubNumEnemies = (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);

    // Count mobile enemies
    pGroup = gpGroupList;
    while (pGroup) {
      if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == sSectorX &&
          pGroup->ubSectorY == sSectorY) {
        ubNumEnemies += pGroup->ubGroupSize;
      }
      pGroup = pGroup->next;
    }
  }

  return ubNumEnemies;
}

uint8_t NumEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY) {
  SECTORINFO *pSector;
  struct GROUP *pGroup;
  uint8_t ubNumTroops;
  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);
  pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];
  ubNumTroops = (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);

  pGroup = gpGroupList;
  while (pGroup) {
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == sSectorX &&
        pGroup->ubSectorY == sSectorY) {
      ubNumTroops += pGroup->ubGroupSize;
    }
    pGroup = pGroup->next;
  }
  return ubNumTroops;
}

uint8_t NumStationaryEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY) {
  SECTORINFO *pSector;
  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);
  pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];

  if (pSector->ubGarrisonID == NO_GARRISON) {  // If no garrison, no stationary.
    return (0);
  }

  // don't count roadblocks as stationary garrison, we want to see how many enemies are in them, not
  // question marks
  if (gGarrisonGroup[pSector->ubGarrisonID].ubComposition == ROADBLOCK) {
    // pretend they're not stationary
    return (0);
  }

  return (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
}

uint8_t NumMobileEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY) {
  struct GROUP *pGroup;
  SECTORINFO *pSector;
  uint8_t ubNumTroops;
  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);

  ubNumTroops = 0;
  pGroup = gpGroupList;
  while (pGroup) {
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == sSectorX &&
        pGroup->ubSectorY == sSectorY) {
      ubNumTroops += pGroup->ubGroupSize;
    }
    pGroup = pGroup->next;
  }

  pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];
  if (pSector->ubGarrisonID ==
      ROADBLOCK) {  // consider these troops as mobile troops even though they are in a garrison
    ubNumTroops += (uint8_t)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
  }

  return ubNumTroops;
}

void GetNumberOfMobileEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, uint8_t *pubNumAdmins,
                                      uint8_t *pubNumTroops, uint8_t *pubNumElites) {
  struct GROUP *pGroup;
  SECTORINFO *pSector;
  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);

  // Now count the number of mobile groups in the sector.
  *pubNumTroops = *pubNumElites = *pubNumAdmins = 0;
  pGroup = gpGroupList;
  while (pGroup) {
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == sSectorX &&
        pGroup->ubSectorY == sSectorY) {
      *pubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
      *pubNumElites += pGroup->pEnemyGroup->ubNumElites;
      *pubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
    }
    pGroup = pGroup->next;
  }

  pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];
  if (pSector->ubGarrisonID ==
      ROADBLOCK) {  // consider these troops as mobile troops even though they are in a garrison
    *pubNumAdmins += pSector->ubNumAdmins;
    *pubNumTroops += pSector->ubNumTroops;
    *pubNumElites += pSector->ubNumElites;
  }
}

void GetNumberOfStationaryEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, uint8_t *pubNumAdmins,
                                          uint8_t *pubNumTroops, uint8_t *pubNumElites) {
  SECTORINFO *pSector;
  Assert(sSectorX >= 1 && sSectorX <= 16);
  Assert(sSectorY >= 1 && sSectorY <= 16);
  pSector = &SectorInfo[GetSectorID8(sSectorX, sSectorY)];

  // grab the number of each type in the stationary sector
  *pubNumAdmins = pSector->ubNumAdmins;
  *pubNumTroops = pSector->ubNumTroops;
  *pubNumElites = pSector->ubNumElites;
}

void GetNumberOfEnemiesInSector(uint8_t sSectorX, uint8_t sSectorY, uint8_t *pubNumAdmins,
                                uint8_t *pubNumTroops, uint8_t *pubNumElites) {
  uint8_t ubNumAdmins, ubNumTroops, ubNumElites;

  GetNumberOfStationaryEnemiesInSector(sSectorX, sSectorY, pubNumAdmins, pubNumTroops,
                                       pubNumElites);

  GetNumberOfMobileEnemiesInSector(sSectorX, sSectorY, &ubNumAdmins, &ubNumTroops, &ubNumElites);

  *pubNumAdmins += ubNumAdmins;
  *pubNumTroops += ubNumTroops;
  *pubNumElites += ubNumElites;
}

void EndTacticalBattleForEnemy() {
  struct GROUP *pGroup;
  int32_t i;

  // Clear enemies in battle for all stationary groups in the sector.
  if (gbWorldSectorZ > 0) {
    UNDERGROUND_SECTORINFO *pSector;
    pSector = FindUnderGroundSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
    pSector->ubAdminsInBattle = 0;
    pSector->ubTroopsInBattle = 0;
    pSector->ubElitesInBattle = 0;
  } else if (!gbWorldSectorZ) {
    SECTORINFO *pSector;
    pSector = &SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)];
    // grab the number of each type in the stationary sector
    pSector->ubAdminsInBattle = 0;
    pSector->ubTroopsInBattle = 0;
    pSector->ubElitesInBattle = 0;
    pSector->ubNumCreatures = 0;
    pSector->ubCreaturesInBattle = 0;
  } else  // negative
    return;

  // Clear this value so that profiled enemies can be added into battles in the future.
  gfProfiledEnemyAdded = FALSE;

  // Clear enemies in battle for all mobile groups in the sector.
  pGroup = gpGroupList;
  while (pGroup) {
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == gWorldSectorX &&
        pGroup->ubSectorY == gWorldSectorY) {
      pGroup->pEnemyGroup->ubTroopsInBattle = 0;
      pGroup->pEnemyGroup->ubElitesInBattle = 0;
      pGroup->pEnemyGroup->ubAdminsInBattle = 0;
    }
    pGroup = pGroup->next;
  }

  // Check to see if any of our mercs have abandoned the militia during a battle.  This is cause for
  // a rather severe loyalty blow.
  for (i = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
       i <= gTacticalStatus.Team[MILITIA_TEAM].bLastID; i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bInSector &&
        MercPtrs[i]->bLife >=
            OKLIFE) {  // found one live militia, so look for any enemies/creatures.
      // NOTE: this is relying on ENEMY_TEAM being immediately followed by CREATURE_TEAM
      for (i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
           i <= gTacticalStatus.Team[CREATURE_TEAM].bLastID; i++) {
        if (MercPtrs[i]->bActive && MercPtrs[i]->bInSector &&
            MercPtrs[i]->bLife >=
                OKLIFE) {  // confirmed at least one enemy here, so do the loyalty penalty.
          HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ABANDON_MILITIA, (uint8_t)gWorldSectorX,
                                   (uint8_t)gWorldSectorY, 0);
          break;
        }
      }
      break;
    }
  }
}

uint8_t NumFreeEnemySlots() {
  uint8_t ubNumFreeSlots = 0;
  int32_t i;
  struct SOLDIERTYPE *pSoldier;
  // Count the number of free enemy slots.  It is possible to have multiple groups exceed the
  // maximum.
  for (i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID; i <= gTacticalStatus.Team[ENEMY_TEAM].bLastID;
       i++) {
    pSoldier = GetSoldierByID(i);
    if (!IsSolActive(pSoldier)) ubNumFreeSlots++;
  }
  return ubNumFreeSlots;
}

// Called when entering a sector so the campaign AI can automatically insert the
// correct number of troops of each type based on the current number in the sector
// in global focus (gWorldSectorX/Y)
BOOLEAN PrepareEnemyForSectorBattle() {
  SECTORINFO *pSector;
  struct GROUP *pGroup;
  struct SOLDIERTYPE *pSoldier;
  uint8_t ubNumAdmins, ubNumTroops, ubNumElites;
  uint8_t ubTotalAdmins, ubTotalElites, ubTotalTroops;
  uint8_t ubStationaryEnemies;
  int32_t i, num;
  int16_t sNumSlots;

  gfPendingEnemies = FALSE;

  if (gbWorldSectorZ > 0) return PrepareEnemyForUndergroundBattle();

  if (gpBattleGroup && !gpBattleGroup->fPlayer) {  // The enemy has instigated the battle which
                                                   // means they are the ones entering the conflict.
    // The player was actually in the sector first, and the enemy doesn't use reinforced placements
    HandleArrivalOfReinforcements(gpBattleGroup);
    // It is possible that other enemy groups have also arrived.  Add them in the same manner.
    pGroup = gpGroupList;
    while (pGroup) {
      if (pGroup != gpBattleGroup && !pGroup->fPlayer && !pGroup->fVehicle &&
          pGroup->ubSectorX == gpBattleGroup->ubSectorX &&
          pGroup->ubSectorY == gpBattleGroup->ubSectorY && !pGroup->pEnemyGroup->ubAdminsInBattle &&
          !pGroup->pEnemyGroup->ubTroopsInBattle && !pGroup->pEnemyGroup->ubElitesInBattle) {
        HandleArrivalOfReinforcements(pGroup);
      }
      pGroup = pGroup->next;
    }
    ValidateEnemiesHaveWeapons();
    return ((BOOLEAN)(gpBattleGroup->ubGroupSize > 0));
  }

  if (!gbWorldSectorZ) {
    if (NumEnemiesInSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY) > 32) {
      gfPendingEnemies = TRUE;
    }
  }

  pSector = &SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)];
  if (pSector->uiFlags &
      SF_USE_MAP_SETTINGS) {  // count the number of enemy placements in a map and use those
    SOLDIERINITNODE *curr;
    curr = gSoldierInitHead;
    ubTotalAdmins = ubTotalTroops = ubTotalElites = 0;
    while (curr) {
      if (curr->pBasicPlacement->bTeam == ENEMY_TEAM) {
        switch (curr->pBasicPlacement->ubSoldierClass) {
          case SOLDIER_CLASS_ADMINISTRATOR:
            ubTotalAdmins++;
            break;
          case SOLDIER_CLASS_ARMY:
            ubTotalTroops++;
            break;
          case SOLDIER_CLASS_ELITE:
            ubTotalElites++;
            break;
        }
      }
      curr = curr->next;
    }
    pSector->ubNumAdmins = ubTotalAdmins;
    pSector->ubNumTroops = ubTotalTroops;
    pSector->ubNumElites = ubTotalElites;
    pSector->ubAdminsInBattle = 0;
    pSector->ubTroopsInBattle = 0;
    pSector->ubElitesInBattle = 0;
  } else {
    ubTotalAdmins = (uint8_t)(pSector->ubNumAdmins - pSector->ubAdminsInBattle);
    ubTotalTroops = (uint8_t)(pSector->ubNumTroops - pSector->ubTroopsInBattle);
    ubTotalElites = (uint8_t)(pSector->ubNumElites - pSector->ubElitesInBattle);
  }
  ubStationaryEnemies = (uint8_t)(ubTotalAdmins + ubTotalTroops + ubTotalElites);

  if (ubTotalAdmins + ubTotalTroops + ubTotalElites > 32) {
#ifdef JA2BETAVERSION
    ScreenMsg(
        FONT_RED, MSG_ERROR, L"The total stationary enemy forces in sector %c%d is %d. (max %d)",
        gWorldSectorY + 'A' - 1, gWorldSectorX, ubTotalAdmins + ubTotalTroops + ubTotalElites, 32);
#endif

    ubTotalAdmins = min(32, ubTotalAdmins);
    ubTotalTroops = min(32 - ubTotalAdmins, ubTotalTroops);
    ubTotalElites = min(32 - ubTotalAdmins + ubTotalTroops, ubTotalElites);
  }

  pSector->ubAdminsInBattle += ubTotalAdmins;
  pSector->ubTroopsInBattle += ubTotalTroops;
  pSector->ubElitesInBattle += ubTotalElites;

#ifdef JA2TESTVERSION
  if (gfOverrideSector) {
    // if there are no troops in the current groups, then we're done.
    if (!ubTotalAdmins && !ubTotalTroops && !ubTotalElites) return FALSE;
    AddSoldierInitListEnemyDefenceSoldiers(ubTotalAdmins, ubTotalTroops, ubTotalElites);
    ValidateEnemiesHaveWeapons();
    return TRUE;
  }
#endif

  // Search for movement groups that happen to be in the sector.
  sNumSlots = NumFreeEnemySlots();
  // Test:  All slots should be free at this point!
  if (sNumSlots !=
      gTacticalStatus.Team[ENEMY_TEAM].bLastID - gTacticalStatus.Team[ENEMY_TEAM].bFirstID + 1) {
#ifdef JA2BETAVERSION
    ScreenMsg(
        FONT_RED, MSG_ERROR,
        L"All enemy slots should be free at this point.  Only %d of %d are available.\nTrying to "
        L"add %d admins, %d troops, and %d elites.",
        sNumSlots,
        gTacticalStatus.Team[ENEMY_TEAM].bLastID - gTacticalStatus.Team[ENEMY_TEAM].bFirstID + 1,
        ubTotalAdmins, ubTotalTroops, ubTotalElites);
#endif
  }
  // Subtract the total number of stationary enemies from the available slots, as stationary forces
  // take precendence in combat.  The mobile forces that could also be in the same sector are
  // considered later if all the slots fill up.
  sNumSlots -= ubTotalAdmins + ubTotalTroops + ubTotalElites;
  // Now, process all of the groups and search for both enemy and player groups in the sector.
  // For enemy groups, we fill up the slots until we have none left or all of the groups have been
  // processed.
  pGroup = gpGroupList;
  while (pGroup && sNumSlots) {
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == gWorldSectorX &&
        pGroup->ubSectorY == gWorldSectorY && !gbWorldSectorZ) {  // Process enemy group in sector.
      if (sNumSlots > 0) {
        ubNumAdmins =
            (uint8_t)(pGroup->pEnemyGroup->ubNumAdmins - pGroup->pEnemyGroup->ubAdminsInBattle);
        sNumSlots -= ubNumAdmins;
        if (sNumSlots < 0) {  // adjust the value to zero
          ubNumAdmins += sNumSlots;
          sNumSlots = 0;
          gfPendingEnemies = TRUE;
        }
        pGroup->pEnemyGroup->ubAdminsInBattle += ubNumAdmins;
        ubTotalAdmins += ubNumAdmins;
      }
      if (sNumSlots > 0) {  // Add regular army forces.
        ubNumTroops =
            (uint8_t)(pGroup->pEnemyGroup->ubNumTroops - pGroup->pEnemyGroup->ubTroopsInBattle);
        sNumSlots -= ubNumTroops;
        if (sNumSlots < 0) {  // adjust the value to zero
          ubNumTroops += sNumSlots;
          sNumSlots = 0;
          gfPendingEnemies = TRUE;
        }
        pGroup->pEnemyGroup->ubTroopsInBattle += ubNumTroops;
        ubTotalTroops += ubNumTroops;
      }
      if (sNumSlots > 0) {  // Add elite troops
        ubNumElites =
            (uint8_t)(pGroup->pEnemyGroup->ubNumElites - pGroup->pEnemyGroup->ubElitesInBattle);
        sNumSlots -= ubNumElites;
        if (sNumSlots < 0) {  // adjust the value to zero
          ubNumElites += sNumSlots;
          sNumSlots = 0;
          gfPendingEnemies = TRUE;
        }
        pGroup->pEnemyGroup->ubElitesInBattle += ubNumElites;
        ubTotalElites += ubNumElites;
      }
      // NOTE:
      // no provisions for profile troop leader or retreat groups yet.
    }
    if (pGroup->fPlayer && !pGroup->fVehicle && !pGroup->fBetweenSectors &&
        pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY &&
        !gbWorldSectorZ) {  // TEMP:  The player path needs to get destroyed, otherwise, it'll be
                            // impossible to move the
      //			 group after the battle is resolved.

      // no one in the group any more continue loop
      if (pGroup->pPlayerList == NULL) {
        pGroup = pGroup->next;
        continue;
      }

      // clear the movt for this grunt and his buddies
      RemoveGroupWaypoints(pGroup->ubGroupID);
    }
    pGroup = pGroup->next;
  }

  // if there are no troops in the current groups, then we're done.
  if (!ubTotalAdmins && !ubTotalTroops && !ubTotalElites) {
    return FALSE;
  }

  AddSoldierInitListEnemyDefenceSoldiers(ubTotalAdmins, ubTotalTroops, ubTotalElites);

  // Now, we have to go through all of the enemies in the new map, and assign their respective
  // groups if in a mobile group, but only for the ones that were assigned from the
  sNumSlots = 32 - ubStationaryEnemies;

  pGroup = gpGroupList;
  while (pGroup && sNumSlots) {
    i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID;
    pSoldier = GetSoldierByID(i);
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == gWorldSectorX &&
        pGroup->ubSectorY == gWorldSectorY && !gbWorldSectorZ) {
      num = pGroup->ubGroupSize;
      ubNumAdmins = pGroup->pEnemyGroup->ubAdminsInBattle;
      ubNumTroops = pGroup->pEnemyGroup->ubTroopsInBattle;
      ubNumElites = pGroup->pEnemyGroup->ubElitesInBattle;
      while (num && sNumSlots && i <= gTacticalStatus.Team[ENEMY_TEAM].bLastID) {
        while (!IsSolActive(pSoldier) || pSoldier->ubGroupID) {
          pSoldier = GetSoldierByID(++i);
          if (i > gTacticalStatus.Team[ENEMY_TEAM].bLastID) {
            AssertMsg(
                0,
                "Failed to assign battle counters for enemies properly. Please send save. KM:0.");
          }
        }
        switch (pSoldier->ubSoldierClass) {
          case SOLDIER_CLASS_ADMINISTRATOR:
            if (ubNumAdmins) {
              num--;
              sNumSlots--;
              ubNumAdmins--;
              pSoldier->ubGroupID = pGroup->ubGroupID;
            }
            break;
          case SOLDIER_CLASS_ARMY:
            if (ubNumTroops) {
              num--;
              sNumSlots--;
              ubNumTroops--;
              pSoldier->ubGroupID = pGroup->ubGroupID;
            }
            break;
          case SOLDIER_CLASS_ELITE:
            if (ubNumElites) {
              num--;
              sNumSlots--;
              ubNumElites--;
              pSoldier->ubGroupID = pGroup->ubGroupID;
            }
            break;
        }
        pSoldier = GetSoldierByID(++i);
      }
    }
    pGroup = pGroup->next;
  }

  ValidateEnemiesHaveWeapons();

  return TRUE;
}

BOOLEAN PrepareEnemyForUndergroundBattle() {
  UNDERGROUND_SECTORINFO *pUnderground;
  uint8_t ubTotalAdmins, ubTotalTroops, ubTotalElites;
  pUnderground = gpUndergroundSectorInfoHead;
  while (pUnderground) {
    if (pUnderground->ubSectorX == gWorldSectorX && pUnderground->ubSectorY == gWorldSectorY &&
        pUnderground->ubSectorZ ==
            gbWorldSectorZ) {  // This is the sector we are going to be fighting in.
      if (pUnderground->ubNumAdmins || pUnderground->ubNumTroops || pUnderground->ubNumElites) {
        ubTotalAdmins = (uint8_t)(pUnderground->ubNumAdmins - pUnderground->ubAdminsInBattle);
        ubTotalTroops = (uint8_t)(pUnderground->ubNumTroops - pUnderground->ubTroopsInBattle);
        ubTotalElites = (uint8_t)(pUnderground->ubNumElites - pUnderground->ubElitesInBattle);
        pUnderground->ubAdminsInBattle += ubTotalAdmins;
        pUnderground->ubTroopsInBattle += ubTotalTroops;
        pUnderground->ubElitesInBattle += ubTotalElites;
        AddSoldierInitListEnemyDefenceSoldiers(pUnderground->ubNumAdmins, pUnderground->ubNumTroops,
                                               pUnderground->ubNumElites);
        ValidateEnemiesHaveWeapons();
      }
      return ((BOOLEAN)(pUnderground->ubNumAdmins + pUnderground->ubNumTroops +
                            pUnderground->ubNumElites >
                        0));
    }
    pUnderground = pUnderground->next;
  }

  // underground sector not found in list
  Assert(FALSE);
  return FALSE;
}

// The queen AI layer must process the event by subtracting forces, etc.
void ProcessQueenCmdImplicationsOfDeath(struct SOLDIERTYPE *pSoldier) {
  int32_t iNumEnemiesInSector;
  SECTORINFO *pSector;
  EvaluateDeathEffectsToSoldierInitList(pSoldier);

  switch (GetSolProfile(pSoldier)) {
    case MIKE:
    case IGGY:
      if (GetSolProfile(pSoldier) == IGGY &&
          !gubFact[FACT_IGGY_AVAILABLE_TO_ARMY]) {  // Iggy is on our team!
        break;
      }
      if (!GetSolSectorZ(pSoldier)) {
        pSector = &SectorInfo[GetSolSectorID8(pSoldier)];
        if (pSector->ubNumElites) {
          pSector->ubNumElites--;
        }
        if (pSector->ubElitesInBattle) {
          pSector->ubElitesInBattle--;
        }
      } else {
        UNDERGROUND_SECTORINFO *pUnderground;
        pUnderground = FindUnderGroundSector((uint8_t)GetSolSectorX(pSoldier),
                                             (uint8_t)GetSolSectorY(pSoldier),
                                             (uint8_t)GetSolSectorZ(pSoldier));
        Assert(pUnderground);
        if (pUnderground->ubNumElites) {
          pUnderground->ubNumElites--;
        }
        if (pUnderground->ubElitesInBattle) {
          pUnderground->ubElitesInBattle--;
        }
      }
      break;
  }

  if (pSoldier->bNeutral || (pSoldier->bTeam != ENEMY_TEAM && pSoldier->bTeam != CREATURE_TEAM))
    return;
  // we are recording an enemy death
  if (pSoldier->ubGroupID) {  // The enemy was in a mobile group
    struct GROUP *pGroup;
    pGroup = GetGroup(pSoldier->ubGroupID);
    if (!pGroup) {
#ifdef JA2BETAVERSION
      wchar_t str[256];
      swprintf(str, ARR_SIZE(str),
               L"Enemy soldier killed with ubGroupID of %d, and the group doesn't exist!",
               pSoldier->ubGroupID);
      DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
#endif
      return;
    }
    if (pGroup->fPlayer) {
#ifdef JA2BETAVERSION
      wchar_t str[256];
      swprintf(str, ARR_SIZE(str),
               L"Attempting to process player group thinking it's an enemy group in "
               L"ProcessQueenCmdImplicationsOfDeath(), %d",
               pSoldier->ubGroupID);
      DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
#endif
      return;
    }
    switch (pSoldier->ubSoldierClass) {
      case SOLDIER_CLASS_ELITE:
#ifdef JA2BETAVERSION
        if (!pGroup->pEnemyGroup->ubNumElites) {
          wchar_t str[100];
          swprintf(
              str, ARR_SIZE(str),
              L"Enemy elite killed with ubGroupID of %d, but the group doesn't contain elites!",
              pGroup->ubGroupID);
          DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
          break;
        }
        if (IsTacticalMode()) {
          if (pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE &&
                  pGroup->pEnemyGroup->ubNumElites != pGroup->pEnemyGroup->ubElitesInBattle &&
                  !gfPendingEnemies ||
              pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE ||
              pGroup->pEnemyGroup->ubNumElites > 50 || pGroup->pEnemyGroup->ubElitesInBattle > 50) {
            DoScreenIndependantMessageBox(
                L"Group elite counters are bad.  What were the last 2-3 things to die, and how?  "
                L"Save game and send to KM with info!!!",
                MSG_BOX_FLAG_OK, NULL);
          }
        }
#endif
        if (pGroup->pEnemyGroup->ubNumElites) {
          pGroup->pEnemyGroup->ubNumElites--;
        }
        if (pGroup->pEnemyGroup->ubElitesInBattle) {
          pGroup->pEnemyGroup->ubElitesInBattle--;
        }
        break;
      case SOLDIER_CLASS_ARMY:
#ifdef JA2BETAVERSION
        if (!pGroup->pEnemyGroup->ubNumTroops) {
          wchar_t str[100];
          swprintf(
              str, ARR_SIZE(str),
              L"Enemy troop killed with ubGroupID of %d, but the group doesn't contain elites!",
              pGroup->ubGroupID);
          DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
          break;
        }
        if (IsTacticalMode()) {
          if (pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE &&
                  pGroup->pEnemyGroup->ubNumTroops != pGroup->pEnemyGroup->ubTroopsInBattle &&
                  !gfPendingEnemies ||
              pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE ||
              pGroup->pEnemyGroup->ubNumTroops > 50 || pGroup->pEnemyGroup->ubTroopsInBattle > 50) {
            DoScreenIndependantMessageBox(
                L"Group troop counters are bad.  What were the last 2-3 things to die, and how?  "
                L"Save game and send to KM with info!!!",
                MSG_BOX_FLAG_OK, NULL);
          }
        }
#endif
        if (pGroup->pEnemyGroup->ubNumTroops) {
          pGroup->pEnemyGroup->ubNumTroops--;
        }
        if (pGroup->pEnemyGroup->ubTroopsInBattle) {
          pGroup->pEnemyGroup->ubTroopsInBattle--;
        }
        break;
      case SOLDIER_CLASS_ADMINISTRATOR:
#ifdef JA2BETAVERSION
        if (!pGroup->pEnemyGroup->ubNumAdmins) {
          wchar_t str[100];
          swprintf(str, ARR_SIZE(str),
                   L"Enemy administrator killed with ubGroupID of %d, but the group doesn't "
                   L"contain elites!",
                   pGroup->ubGroupID);
          DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
          break;
        }
        if (IsTacticalMode()) {
          if (pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE &&
                  pGroup->pEnemyGroup->ubNumAdmins != pGroup->pEnemyGroup->ubAdminsInBattle &&
                  !gfPendingEnemies ||
              pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE ||
              pGroup->pEnemyGroup->ubNumAdmins > 50 || pGroup->pEnemyGroup->ubAdminsInBattle > 50) {
            DoScreenIndependantMessageBox(
                L"Group admin counters are bad.  What were the last 2-3 things to die, and how?  "
                L"Save game and send to KM with info!!!",
                MSG_BOX_FLAG_OK, NULL);
          }
        }
#endif
        if (pGroup->pEnemyGroup->ubNumAdmins) {
          pGroup->pEnemyGroup->ubNumAdmins--;
        }
        if (pGroup->pEnemyGroup->ubAdminsInBattle) {
          pGroup->pEnemyGroup->ubAdminsInBattle--;
        }
        break;
    }
    if (pGroup->ubGroupSize) pGroup->ubGroupSize--;
    RecalculateGroupWeight(pGroup);
    if (!pGroup->ubGroupSize) {
      RemovePGroup(pGroup);
    }
  } else {                                           // The enemy was in a stationary defence group
    if (!gbWorldSectorZ || IsAutoResolveActive()) {  // ground level (SECTORINFO)
      SECTORINFO *pSector;
#ifdef JA2BETAVERSION
      uint32_t ubTotalEnemies;
#endif

      if (!IsAutoResolveActive()) {
        pSector = &SectorInfo[GetSolSectorID8(pSoldier)];
      } else {
        pSector = &SectorInfo[GetAutoResolveSectorID()];
      }

#ifdef JA2BETAVERSION
      ubTotalEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
#endif

      switch (pSoldier->ubSoldierClass) {
        case SOLDIER_CLASS_ADMINISTRATOR:
#ifdef JA2BETAVERSION
          if (IsTacticalMode()) {
            if (ubTotalEnemies <= 32 && pSector->ubNumAdmins != pSector->ubAdminsInBattle ||
                !pSector->ubNumAdmins || !pSector->ubAdminsInBattle || pSector->ubNumAdmins > 100 ||
                pSector->ubAdminsInBattle > 32) {
              DoScreenIndependantMessageBox(
                  L"Sector admin counters are bad.  What were the last 2-3 things to die, and how? "
                  L" Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
          }
#endif
          if (pSector->ubNumAdmins) {
            pSector->ubNumAdmins--;
          }
          if (pSector->ubAdminsInBattle) {
            pSector->ubAdminsInBattle--;
          }
          break;
        case SOLDIER_CLASS_ARMY:
#ifdef JA2BETAVERSION
          if (IsTacticalMode()) {
            if (ubTotalEnemies <= 32 && pSector->ubNumTroops != pSector->ubTroopsInBattle ||
                !pSector->ubNumTroops || !pSector->ubTroopsInBattle || pSector->ubNumTroops > 100 ||
                pSector->ubTroopsInBattle > 32) {
              DoScreenIndependantMessageBox(
                  L"Sector troop counters are bad.  What were the last 2-3 things to die, and how? "
                  L" Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
          }
#endif
          if (pSector->ubNumTroops) {
            pSector->ubNumTroops--;
          }
          if (pSector->ubTroopsInBattle) {
            pSector->ubTroopsInBattle--;
          }
          break;
        case SOLDIER_CLASS_ELITE:
#ifdef JA2BETAVERSION
          if (IsTacticalMode()) {
            if (ubTotalEnemies <= 32 && pSector->ubNumElites != pSector->ubElitesInBattle ||
                !pSector->ubNumElites || !pSector->ubElitesInBattle || pSector->ubNumElites > 100 ||
                pSector->ubElitesInBattle > 32) {
              DoScreenIndependantMessageBox(
                  L"Sector elite counters are bad.  What were the last 2-3 things to die, and how? "
                  L" Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
          }
#endif
          if (pSector->ubNumElites) {
            pSector->ubNumElites--;
          }
          if (pSector->ubElitesInBattle) {
            pSector->ubElitesInBattle--;
          }
          break;
        case SOLDIER_CLASS_CREATURE:
          if (pSoldier->ubBodyType != BLOODCAT) {
#ifdef JA2BETAVERSION
            if (IsTacticalMode()) {
              if (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE &&
                      pSector->ubNumCreatures != pSector->ubCreaturesInBattle ||
                  !pSector->ubNumCreatures || !pSector->ubCreaturesInBattle ||
                  pSector->ubNumCreatures > 50 || pSector->ubCreaturesInBattle > 50) {
                DoScreenIndependantMessageBox(
                    L"Sector creature counters are bad.  What were the last 2-3 things to die, and "
                    L"how?  Save game and send to KM with info!!!",
                    MSG_BOX_FLAG_OK, NULL);
              }
            }
#endif
            if (pSector->ubNumCreatures) {
              pSector->ubNumCreatures--;
            }
            if (pSector->ubCreaturesInBattle) {
              pSector->ubCreaturesInBattle--;
            }
          } else {
            if (pSector->bBloodCats) {
              pSector->bBloodCats--;
            }
          }

          break;
      }
      RecalculateSectorWeight((uint8_t)GetSolSectorID8(pSoldier));
    } else {  // basement level (UNDERGROUND_SECTORINFO)
      UNDERGROUND_SECTORINFO *pSector =
          FindUnderGroundSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
#ifdef JA2BETAVERSION
      uint32_t ubTotalEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
#endif
      if (pSector) {
        switch (pSoldier->ubSoldierClass) {
          case SOLDIER_CLASS_ADMINISTRATOR:
#ifdef JA2BETAVERSION
            if (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE &&
                    pSector->ubNumAdmins != pSector->ubAdminsInBattle ||
                !pSector->ubNumAdmins || !pSector->ubAdminsInBattle || pSector->ubNumAdmins > 100 ||
                pSector->ubAdminsInBattle > MAX_STRATEGIC_TEAM_SIZE) {
              DoScreenIndependantMessageBox(
                  L"Underground sector admin counters are bad.  What were the last 2-3 things to "
                  L"die, and how?  Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
#endif
            if (pSector->ubNumAdmins) {
              pSector->ubNumAdmins--;
            }
            if (pSector->ubAdminsInBattle) {
              pSector->ubAdminsInBattle--;
            }
            break;
          case SOLDIER_CLASS_ARMY:
#ifdef JA2BETAVERSION
            if (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE &&
                    pSector->ubNumTroops != pSector->ubTroopsInBattle ||
                !pSector->ubNumTroops || !pSector->ubTroopsInBattle || pSector->ubNumTroops > 100 ||
                pSector->ubTroopsInBattle > MAX_STRATEGIC_TEAM_SIZE) {
              DoScreenIndependantMessageBox(
                  L"Underground sector troop counters are bad.  What were the last 2-3 things to "
                  L"die, and how?  Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
#endif
            if (pSector->ubNumTroops) {
              pSector->ubNumTroops--;
            }
            if (pSector->ubTroopsInBattle) {
              pSector->ubTroopsInBattle--;
            }
            break;
          case SOLDIER_CLASS_ELITE:
#ifdef JA2BETAVERSION
            if (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE &&
                    pSector->ubNumElites != pSector->ubElitesInBattle ||
                !pSector->ubNumElites || !pSector->ubElitesInBattle || pSector->ubNumElites > 100 ||
                pSector->ubElitesInBattle > MAX_STRATEGIC_TEAM_SIZE) {
              DoScreenIndependantMessageBox(
                  L"Underground sector elite counters are bad.  What were the last 2-3 things to "
                  L"die, and how?  Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
#endif
            if (pSector->ubNumElites) {
              pSector->ubNumElites--;
            }
            if (pSector->ubElitesInBattle) {
              pSector->ubElitesInBattle--;
            }
            break;
          case SOLDIER_CLASS_CREATURE:
#ifdef JA2BETAVERSION
            if (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE &&
                    pSector->ubNumCreatures != pSector->ubCreaturesInBattle ||
                !pSector->ubNumCreatures || !pSector->ubCreaturesInBattle ||
                pSector->ubNumCreatures > 50 || pSector->ubCreaturesInBattle > 50) {
              DoScreenIndependantMessageBox(
                  L"Underground sector creature counters are bad.  What were the last 2-3 things "
                  L"to die, and how?  Save game and send to KM with info!!!",
                  MSG_BOX_FLAG_OK, NULL);
            }
#endif
            if (pSector->ubNumCreatures) {
              pSector->ubNumCreatures--;
            }
            if (pSector->ubCreaturesInBattle) {
              pSector->ubCreaturesInBattle--;
            }

            if (!pSector->ubNumCreatures && gWorldSectorX != 9 &&
                gWorldSectorY != 10) {  // If the player has successfully killed all creatures in
                                        // ANY underground sector except J9
              // then cancel any pending creature town attack.
              DeleteAllStrategicEventsOfType(EVENT_CREATURE_ATTACK);
            }

            // a monster has died.  Post an event to immediately check whether a mine has been
            // cleared.
            AddStrategicEventUsingSeconds(EVENT_CHECK_IF_MINE_CLEARED, GetWorldTotalSeconds() + 15,
                                          0);

            if (pSoldier->ubBodyType == QUEENMONSTER) {
              // Need to call this, as the queen is really big, and killing her leaves a bunch
              // of bad tiles in behind her.  Calling this function cleans it up.
              InvalidateWorldRedundency();
              // Now that the queen is dead, turn off the creature quest.
              EndCreatureQuest();
              EndQuest(QUEST_CREATURES, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
            }
            break;
        }
      }
    }
  }
  if (!GetSolSectorZ(pSoldier)) {
    pSector = &SectorInfo[GetSolSectorID8(pSoldier)];
    iNumEnemiesInSector = NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));
    if (iNumEnemiesInSector) {
      if (pSector->bLastKnownEnemies >= 0) {
        pSector->bLastKnownEnemies = (int8_t)iNumEnemiesInSector;
      }
    } else {
      pSector->bLastKnownEnemies = 0;
    }
  }
}

// Rarely, there will be more enemies than supported by the engine.  In this case, these
// soldier's are waiting for a slot to be free so that they can enter the battle.  This
// essentially allows for an infinite number of troops, though only 32 at a time can fight.
// This is also called whenever an enemy group's reinforcements arrive because the code is
// identical, though it is highly likely that they will all be successfully added on the first call.
void AddPossiblePendingEnemiesToBattle() {
  uint8_t ubSlots, ubNumAvailable;
  uint8_t ubNumElites, ubNumTroops, ubNumAdmins;
  struct GROUP *pGroup;
  if (!gfPendingEnemies) {  // Optimization.  No point in checking if we know that there aren't any
                            // more enemies that can
    // be added to this battle.  This changes whenever a new enemy group arrives at the scene.
    return;
  }
  ubSlots = NumFreeEnemySlots();
  if (!ubSlots) {  // no available slots to add enemies to.  Try again later...
    return;
  }
  pGroup = gpGroupList;
  while (pGroup && ubSlots) {
    if (!pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == gWorldSectorX &&
        pGroup->ubSectorY == gWorldSectorY &&
        !gbWorldSectorZ) {  // This enemy group is currently in the sector.
      ubNumElites = ubNumTroops = ubNumAdmins = 0;
      ubNumAvailable = pGroup->ubGroupSize - pGroup->pEnemyGroup->ubElitesInBattle -
                       pGroup->pEnemyGroup->ubTroopsInBattle -
                       pGroup->pEnemyGroup->ubAdminsInBattle;
      while (ubNumAvailable &&
             ubSlots) {  // This group has enemies waiting for a chance to enter the battle.
        if (pGroup->pEnemyGroup->ubTroopsInBattle <
            pGroup->pEnemyGroup->ubNumTroops) {  // Add a regular troop.
          pGroup->pEnemyGroup->ubTroopsInBattle++;
          ubNumAvailable--;
          ubSlots--;
          ubNumTroops++;
        } else if (pGroup->pEnemyGroup->ubElitesInBattle <
                   pGroup->pEnemyGroup->ubNumElites) {  // Add an elite troop
          pGroup->pEnemyGroup->ubElitesInBattle++;
          ubNumAvailable--;
          ubSlots--;
          ubNumElites++;
        } else if (pGroup->pEnemyGroup->ubAdminsInBattle <
                   pGroup->pEnemyGroup->ubNumAdmins) {  // Add an elite troop
          pGroup->pEnemyGroup->ubAdminsInBattle++;
          ubNumAvailable--;
          ubSlots--;
          ubNumAdmins++;
        } else {
          AssertMsg(0, "AddPossiblePendingEnemiesToBattle():  Logic Error -- by Kris");
        }
      }
      if (ubNumAdmins || ubNumTroops ||
          ubNumElites) {  // This group has contributed forces, then add them now, because different
        // groups appear on different sides of the map.
        uint8_t ubStrategicInsertionCode = 0;
        // First, determine which entrypoint to use, based on the travel direction of the group.
        if (pGroup->ubPrevX && pGroup->ubPrevY) {
          if (pGroup->ubSectorX < pGroup->ubPrevX)
            ubStrategicInsertionCode = INSERTION_CODE_EAST;
          else if (pGroup->ubSectorX > pGroup->ubPrevX)
            ubStrategicInsertionCode = INSERTION_CODE_WEST;
          else if (pGroup->ubSectorY < pGroup->ubPrevY)
            ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
          else if (pGroup->ubSectorY > pGroup->ubPrevY)
            ubStrategicInsertionCode = INSERTION_CODE_NORTH;
        } else if (pGroup->ubNextX && pGroup->ubNextY) {
          if (pGroup->ubSectorX < pGroup->ubNextX)
            ubStrategicInsertionCode = INSERTION_CODE_EAST;
          else if (pGroup->ubSectorX > pGroup->ubNextX)
            ubStrategicInsertionCode = INSERTION_CODE_WEST;
          else if (pGroup->ubSectorY < pGroup->ubNextY)
            ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
          else if (pGroup->ubSectorY > pGroup->ubNextY)
            ubStrategicInsertionCode = INSERTION_CODE_NORTH;
        }
        // Add the number of each type of troop and place them in the appropriate positions
        AddEnemiesToBattle(pGroup, ubStrategicInsertionCode, ubNumAdmins, ubNumTroops, ubNumElites,
                           FALSE);
      }
    }
    pGroup = pGroup->next;
  }
  if (ubSlots) {  // After going through the process, we have finished with some free slots and no
                  // more enemies to add.
    // So, we can turn off the flag, as this check is no longer needed.
    gfPendingEnemies = FALSE;
  }
}

void NotifyPlayersOfNewEnemies() {
  int32_t iSoldiers, iChosenSoldier, i;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fIgnoreBreath = FALSE;

  iSoldiers = 0;
  for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
       i++) {  // find a merc that is aware.
    pSoldier = MercPtrs[i];
    if (pSoldier->bInSector && IsSolActive(pSoldier) && pSoldier->bLife >= OKLIFE &&
        pSoldier->bBreath >= OKBREATH) {
      iSoldiers++;
    }
  }
  if (!iSoldiers) {  // look for an out of breath merc.
    fIgnoreBreath = TRUE;

    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {  // find a merc that is aware.
      pSoldier = MercPtrs[i];
      if (pSoldier->bInSector && IsSolActive(pSoldier) && pSoldier->bLife >= OKLIFE) {
        iSoldiers++;
      }
    }
  }
  if (iSoldiers) {
    iChosenSoldier = Random(iSoldiers);
    for (i = gTacticalStatus.Team[OUR_TEAM].bFirstID; i <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         i++) {  // find a merc that is aware.
      pSoldier = MercPtrs[i];
      if (pSoldier->bInSector && IsSolActive(pSoldier) && pSoldier->bLife >= OKLIFE &&
          ((pSoldier->bBreath >= OKBREATH) || fIgnoreBreath)) {
        if (!iChosenSoldier) {
          // ATE: This is to allow special handling of initial heli drop
          if (!DidGameJustStart()) {
            TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_ENEMY_PRESENCE, 0, 0, 0);
          }
          return;
        }
        iChosenSoldier--;
      }
    }
  } else {  // There is either nobody here or our mercs can't talk
  }
}

void AddEnemiesToBattle(struct GROUP *pGroup, uint8_t ubStrategicInsertionCode, uint8_t ubNumAdmins,
                        uint8_t ubNumTroops, uint8_t ubNumElites, BOOLEAN fMagicallyAppeared) {
  struct SOLDIERTYPE *pSoldier;
  MAPEDGEPOINTINFO MapEdgepointInfo;
  uint8_t ubCurrSlot;
  uint8_t ubTotalSoldiers;
  uint8_t bDesiredDirection = 0;
  switch (ubStrategicInsertionCode) {
    case INSERTION_CODE_NORTH:
      bDesiredDirection = SOUTHEAST;
      break;
    case INSERTION_CODE_EAST:
      bDesiredDirection = SOUTHWEST;
      break;
    case INSERTION_CODE_SOUTH:
      bDesiredDirection = NORTHWEST;
      break;
    case INSERTION_CODE_WEST:
      bDesiredDirection = NORTHEAST;
      break;
    default:
      AssertMsg(0, "Illegal direction passed to AddEnemiesToBattle()");
      break;
  }
#ifdef JA2TESTVERSION
  ScreenMsg(FONT_RED, MSG_INTERFACE,
            L"Enemy reinforcements have arrived!  (%d admins, %d troops, %d elite)", ubNumAdmins,
            ubNumTroops, ubNumElites);
#endif

  if (fMagicallyAppeared) {  // update the strategic counters
    if (!gbWorldSectorZ) {
      SECTORINFO *pSector =
          &SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)];
      pSector->ubNumAdmins += ubNumAdmins;
      pSector->ubAdminsInBattle += ubNumAdmins;
      pSector->ubNumTroops += ubNumTroops;
      pSector->ubTroopsInBattle += ubNumTroops;
      pSector->ubNumElites += ubNumElites;
      pSector->ubElitesInBattle += ubNumElites;
    } else {
      UNDERGROUND_SECTORINFO *pSector =
          FindUnderGroundSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
      if (pSector) {
        pSector->ubNumAdmins += ubNumAdmins;
        pSector->ubAdminsInBattle += ubNumAdmins;
        pSector->ubNumTroops += ubNumTroops;
        pSector->ubTroopsInBattle += ubNumTroops;
        pSector->ubNumElites += ubNumElites;
        pSector->ubElitesInBattle += ubNumElites;
      }
    }
    // Because the enemies magically appeared, have one of our soldiers say something...
    NotifyPlayersOfNewEnemies();
  }

  ubTotalSoldiers = ubNumAdmins + ubNumTroops + ubNumElites;

  ChooseMapEdgepoints(&MapEdgepointInfo, ubStrategicInsertionCode,
                      (uint8_t)(ubNumAdmins + ubNumElites + ubNumTroops));
  ubCurrSlot = 0;
  while (ubTotalSoldiers) {
    if (ubNumElites && Random(ubTotalSoldiers) < ubNumElites) {
      ubNumElites--;
      ubTotalSoldiers--;
      pSoldier = TacticalCreateEliteEnemy();
      if (pGroup) {
        pSoldier->ubGroupID = pGroup->ubGroupID;
      }

      pSoldier->ubInsertionDirection = bDesiredDirection;
      // Setup the position
      if (ubCurrSlot < MapEdgepointInfo.ubNumPoints) {  // using an edgepoint
        pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
        pSoldier->usStrategicInsertionData = MapEdgepointInfo.sGridNo[ubCurrSlot++];
      } else {  // no edgepoints left, so put him at the entrypoint.
        pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
      }
      UpdateMercInSector(pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, 0);
    } else if (ubNumTroops &&
               (uint8_t)Random(ubTotalSoldiers) < (uint8_t)(ubNumElites + ubNumTroops)) {
      ubNumTroops--;
      ubTotalSoldiers--;
      pSoldier = TacticalCreateArmyTroop();
      if (pGroup) {
        pSoldier->ubGroupID = pGroup->ubGroupID;
      }

      pSoldier->ubInsertionDirection = bDesiredDirection;
      // Setup the position
      if (ubCurrSlot < MapEdgepointInfo.ubNumPoints) {  // using an edgepoint
        pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
        pSoldier->usStrategicInsertionData = MapEdgepointInfo.sGridNo[ubCurrSlot++];
      } else {  // no edgepoints left, so put him at the entrypoint.
        pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
      }
      UpdateMercInSector(pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, 0);
    } else if (ubNumAdmins && (uint8_t)Random(ubTotalSoldiers) <
                                  (uint8_t)(ubNumElites + ubNumTroops + ubNumAdmins)) {
      ubNumAdmins--;
      ubTotalSoldiers--;
      pSoldier = TacticalCreateAdministrator();
      if (pGroup) {
        pSoldier->ubGroupID = pGroup->ubGroupID;
      }

      pSoldier->ubInsertionDirection = bDesiredDirection;
      // Setup the position
      if (ubCurrSlot < MapEdgepointInfo.ubNumPoints) {  // using an edgepoint
        pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
        pSoldier->usStrategicInsertionData = MapEdgepointInfo.sGridNo[ubCurrSlot++];
      } else {  // no edgepoints left, so put him at the entrypoint.
        pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
      }
      UpdateMercInSector(pSoldier, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, 0);
    }
  }
}

BOOLEAN SaveUnderGroundSectorInfoToSaveGame(HWFILE hFile) {
  uint32_t uiNumBytesWritten;
  uint32_t uiNumOfRecords = 0;
  UNDERGROUND_SECTORINFO *TempNode = gpUndergroundSectorInfoHead;

  // Loop through all the nodes to count how many there are
  while (TempNode) {
    uiNumOfRecords++;
    TempNode = TempNode->next;
  }

  // Write how many nodes there are
  FileMan_Write(hFile, &uiNumOfRecords, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  TempNode = gpUndergroundSectorInfoHead;

  // Go through each node and save it.
  while (TempNode) {
    FileMan_Write(hFile, TempNode, sizeof(UNDERGROUND_SECTORINFO), &uiNumBytesWritten);
    if (uiNumBytesWritten != sizeof(UNDERGROUND_SECTORINFO)) {
      return (FALSE);
    }

    TempNode = TempNode->next;
  }

  return (TRUE);
}

BOOLEAN LoadUnderGroundSectorInfoFromSavedGame(HWFILE hFile) {
  uint32_t uiNumBytesRead;
  uint32_t uiNumOfRecords = 0;
  uint32_t cnt = 0;
  UNDERGROUND_SECTORINFO *TempNode = NULL;
  UNDERGROUND_SECTORINFO *TempSpot = NULL;

  // Clear the current LL
  TrashUndergroundSectorInfo();

  // Read in the number of nodes stored
  FileMan_Read(hFile, &uiNumOfRecords, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    return (FALSE);
  }

  for (cnt = 0; cnt < uiNumOfRecords; cnt++) {
    // Malloc space for the new node
    TempNode = (UNDERGROUND_SECTORINFO *)MemAlloc(sizeof(UNDERGROUND_SECTORINFO));
    if (TempNode == NULL) return (FALSE);

    // read in the new node
    FileMan_Read(hFile, TempNode, sizeof(UNDERGROUND_SECTORINFO), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(UNDERGROUND_SECTORINFO)) {
      return (FALSE);
    }

    // If its the first time in, assign the node to the list
    if (cnt == 0) {
      gpUndergroundSectorInfoHead = TempNode;
      TempSpot = gpUndergroundSectorInfoHead;
      TempSpot->next = NULL;
    } else {
      // assign the new node to the LL
      TempSpot->next = TempNode;

      // advance to the next node
      TempSpot = TempSpot->next;
      TempSpot->next = NULL;
      gpUndergroundSectorInfoTail = TempSpot;
    }
  }

#ifdef JA2BETAVERSION
  if (!uiNumOfRecords) {
    BuildUndergroundSectorInfoList();
    gfClearCreatureQuest = TRUE;
  }
#endif

  return (TRUE);
}

UNDERGROUND_SECTORINFO *FindUnderGroundSector(uint8_t sMapX, uint8_t sMapY, uint8_t bMapZ) {
  UNDERGROUND_SECTORINFO *pUnderground;
  pUnderground = gpUndergroundSectorInfoHead;

  // Loop through all the underground sectors looking for specified sector
  while (pUnderground) {
    // If the sector is the right one
    if (pUnderground->ubSectorX == sMapX && pUnderground->ubSectorY == sMapY &&
        pUnderground->ubSectorZ == bMapZ) {
      return (pUnderground);
    }
    pUnderground = pUnderground->next;
  }

  return (NULL);
}

void BeginCaptureSquence() {
  if (!(gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE) ||
      !(gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE)) {
    gStrategicStatus.ubNumCapturedForRescue = 0;
  }
}

void EndCaptureSequence() {
  // Set flag...
  if (!(gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE) ||
      !(gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE)) {
    // CJC Dec 1 2002: fixing multiple captures:
    // gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;

    if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTNOTSTARTED) {
      // CJC Dec 1 2002: fixing multiple captures:
      gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;
      StartQuest(QUEST_HELD_IN_ALMA, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
    }
    // CJC Dec 1 2002: fixing multiple captures:
    // else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE )
    else if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTDONE &&
             gubQuest[QUEST_INTERROGATION] == QUESTNOTSTARTED) {
      StartQuest(QUEST_INTERROGATION, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);
      // CJC Dec 1 2002: fixing multiple captures:
      gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE;

      // OK! - Schedule Meanwhile now!
      {
        MEANWHILE_DEFINITION MeanwhileDef;

        MeanwhileDef.sSectorX = 7;
        MeanwhileDef.sSectorY = 14;
        MeanwhileDef.ubNPCNumber = QUEEN;
        MeanwhileDef.usTriggerEvent = 0;
        MeanwhileDef.ubMeanwhileID = INTERROGATION;

        ScheduleMeanwhileEvent(&MeanwhileDef, 10);
      }
    }
    // CJC Dec 1 2002: fixing multiple captures
    else {
      // !?!? set both flags
      gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;
      gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE;
    }
  }
}

void EnemyCapturesPlayerSoldier(struct SOLDIERTYPE *pSoldier) {
  int32_t i;
  WORLDITEM WorldItem;
  BOOLEAN fMadeCorpse;
  int32_t iNumEnemiesInSector;

  static int16_t sAlmaCaptureGridNos[] = {9208, 9688, 9215};
  static int16_t sAlmaCaptureItemsGridNo[] = {12246, 12406, 13046};

  static int16_t sInterrogationItemGridNo[] = {12089, 12089, 12089};

  // ATE: Check first if ! in player captured sequence already
  // CJC Dec 1 2002: fixing multiple captures
  if ((gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE) &&
      (gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE)) {
    return;
  }

  // ATE: If maximum prisoners captured, return!
  if (gStrategicStatus.ubNumCapturedForRescue > 3) {
    return;
  }

  // If this is an EPC , just kill them...
  if (AM_AN_EPC(pSoldier)) {
    pSoldier->bLife = 0;
    HandleSoldierDeath(pSoldier, &fMadeCorpse);
    return;
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    return;
  }

  // ATE: Patch fix If in a vehicle, remove from vehicle...
  TakeSoldierOutOfVehicle(pSoldier);

  // Are there anemies in ALMA? ( I13 )
  iNumEnemiesInSector = NumEnemiesInSector(13, 9);

  // IF there are no enemies, and we need to do alma, skip!
  if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTNOTSTARTED && iNumEnemiesInSector == 0) {
    InternalStartQuest(QUEST_HELD_IN_ALMA, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, FALSE);
    InternalEndQuest(QUEST_HELD_IN_ALMA, (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, FALSE);
  }

  HandleMoraleEvent(pSoldier, MORALE_MERC_CAPTURED, GetSolSectorX(pSoldier),
                    GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));

  // Change to POW....
  //-add him to a POW assignment/group
  if ((pSoldier->bAssignment != ASSIGNMENT_POW)) {
    SetTimeOfAssignmentChangeForMerc(pSoldier);
  }

  ChangeSoldiersAssignment(pSoldier, ASSIGNMENT_POW);
  // ATE: Make them neutral!
  if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTNOTSTARTED) {
    pSoldier->bNeutral = TRUE;
  }

  RemoveCharacterFromSquads(pSoldier);

  // Is this the first one..?
  if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTNOTSTARTED) {
    //-teleport him to NE Alma sector (not Tixa as originally planned)
    pSoldier->sSectorX = 13;
    pSoldier->sSectorY = 9;
    pSoldier->bSectorZ = 0;

    // put him on the floor!!
    pSoldier->bLevel = 0;

    // OK, drop all items!
    for (i = 0; i < NUM_INV_SLOTS; i++) {
      if (pSoldier->inv[i].usItem != 0) {
        WorldItem.fExists = TRUE;
        WorldItem.sGridNo = sAlmaCaptureItemsGridNo[gStrategicStatus.ubNumCapturedForRescue];
        WorldItem.ubLevel = 0;
        WorldItem.usFlags = 0;
        WorldItem.bVisible = FALSE;
        WorldItem.bRenderZHeightAboveLevel = 0;

        memcpy(&(WorldItem.o), &pSoldier->inv[i], sizeof(struct OBJECTTYPE));

        AddWorldItemsToUnLoadedSector(
            13, 9, 0, sAlmaCaptureItemsGridNo[gStrategicStatus.ubNumCapturedForRescue], 1,
            &WorldItem, FALSE);
        DeleteObj(&(pSoldier->inv[i]));
      }
    }

    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
    pSoldier->usStrategicInsertionData =
        sAlmaCaptureGridNos[gStrategicStatus.ubNumCapturedForRescue];

    gStrategicStatus.ubNumCapturedForRescue++;
  } else if (gubQuest[QUEST_HELD_IN_ALMA] == QUESTDONE) {
    //-teleport him to N7
    pSoldier->sSectorX = 7;
    pSoldier->sSectorY = 14;
    pSoldier->bSectorZ = 0;

    // put him on the floor!!
    pSoldier->bLevel = 0;

    // OK, drop all items!
    for (i = 0; i < NUM_INV_SLOTS; i++) {
      if (pSoldier->inv[i].usItem != 0) {
        WorldItem.fExists = TRUE;
        WorldItem.sGridNo = sInterrogationItemGridNo[gStrategicStatus.ubNumCapturedForRescue];
        WorldItem.ubLevel = 0;
        WorldItem.usFlags = 0;
        WorldItem.bVisible = FALSE;
        WorldItem.bRenderZHeightAboveLevel = 0;

        memcpy(&(WorldItem.o), &pSoldier->inv[i], sizeof(struct OBJECTTYPE));

        AddWorldItemsToUnLoadedSector(
            7, 14, 0, sInterrogationItemGridNo[gStrategicStatus.ubNumCapturedForRescue], 1,
            &WorldItem, FALSE);
        DeleteObj(&(pSoldier->inv[i]));
      }
    }

    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
    pSoldier->usStrategicInsertionData =
        gsInterrogationGridNo[gStrategicStatus.ubNumCapturedForRescue];

    gStrategicStatus.ubNumCapturedForRescue++;
  }

  // Bandaging him would prevent him from dying (due to low HP)
  pSoldier->bBleeding = 0;

  // wake him up
  if (pSoldier->fMercAsleep) {
    PutMercInAwakeState(pSoldier);
    pSoldier->fForcedToStayAwake = FALSE;
  }

  // Set his life to 50% + or - 10 HP.
  pSoldier->bLife = pSoldier->bLifeMax / 2;
  if (pSoldier->bLife <= 35) {
    pSoldier->bLife = 35;
  } else if (pSoldier->bLife >= 45) {
    pSoldier->bLife += (int8_t)(10 - Random(21));
  }

  // make him quite exhausted when found
  pSoldier->bBreath = pSoldier->bBreathMax = 50;
  pSoldier->sBreathRed = 0;
  pSoldier->fMercCollapsedFlag = FALSE;
}

void HandleEnemyStatusInCurrentMapBeforeLoadingNewMap() {
  int32_t i;
  BOOLEAN fMadeCorpse;
  int8_t bKilledEnemies = 0, bKilledCreatures = 0, bKilledRebels = 0, bKilledCivilians = 0;
  return;
  // If any of the soldiers are dying, kill them now.
  for (i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID; i <= gTacticalStatus.Team[ENEMY_TEAM].bLastID;
       i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife < OKLIFE && MercPtrs[i]->bLife) {
      MercPtrs[i]->bLife = 0;
      HandleSoldierDeath(MercPtrs[i], &fMadeCorpse);
      bKilledEnemies++;
    }
  }
  // Do the same for the creatures.
  for (i = gTacticalStatus.Team[CREATURE_TEAM].bFirstID;
       i <= gTacticalStatus.Team[CREATURE_TEAM].bLastID; i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife < OKLIFE && MercPtrs[i]->bLife) {
      MercPtrs[i]->bLife = 0;
      HandleSoldierDeath(MercPtrs[i], &fMadeCorpse);
      bKilledCreatures++;
    }
  }
  // Militia
  for (i = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
       i <= gTacticalStatus.Team[MILITIA_TEAM].bLastID; i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife < OKLIFE && MercPtrs[i]->bLife) {
      MercPtrs[i]->bLife = 0;
      HandleSoldierDeath(MercPtrs[i], &fMadeCorpse);
      bKilledRebels++;
    }
  }
  // Civilians
  for (i = gTacticalStatus.Team[CIV_TEAM].bFirstID; i <= gTacticalStatus.Team[CIV_TEAM].bLastID;
       i++) {
    if (MercPtrs[i]->bActive && MercPtrs[i]->bLife < OKLIFE && MercPtrs[i]->bLife) {
      MercPtrs[i]->bLife = 0;
      HandleSoldierDeath(MercPtrs[i], &fMadeCorpse);
      bKilledCivilians++;
    }
  }

  // TEST MESSAGES ONLY!
  if (bKilledCivilians)
    ScreenMsg(FONT_BLUE, MSG_TESTVERSION, L"%d civilians died after you left the sector.",
              bKilledCivilians);
  if (bKilledRebels)
    ScreenMsg(FONT_BLUE, MSG_TESTVERSION, L"%d militia died after you left the sector.",
              bKilledRebels);
  if (bKilledEnemies)
    ScreenMsg(FONT_BLUE, MSG_TESTVERSION, L"%d enemies died after you left the sector.",
              bKilledEnemies);
  if (bKilledCreatures)
    ScreenMsg(FONT_BLUE, MSG_TESTVERSION, L"%d creatures died after you left the sector.",
              bKilledCreatures);

  if (!gbWorldSectorZ) {
    SECTORINFO *pSector;
    pSector = &SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)];
    pSector->ubAdminsInBattle = 0;
    pSector->ubTroopsInBattle = 0;
    pSector->ubElitesInBattle = 0;
    pSector->ubCreaturesInBattle = 0;
    // RecalculateSectorWeight(
  } else if (gbWorldSectorZ > 0) {
    UNDERGROUND_SECTORINFO *pSector;
    pSector = FindUnderGroundSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
    if (!pSector) return;
    pSector->ubAdminsInBattle = 0;
    pSector->ubTroopsInBattle = 0;
    pSector->ubElitesInBattle = 0;
    pSector->ubCreaturesInBattle = 0;
  }
}

BOOLEAN PlayerSectorDefended(uint8_t ubSectorID) {
  if (CountAllMilitiaInSectorID8(ubSectorID) > 0) {
    // militia in sector
    return TRUE;
  }
  if (FindMovementGroupInSector(SectorID8_X(ubSectorID), SectorID8_Y(ubSectorID), TRUE)) {
    // player in sector
    return TRUE;
  }
  return FALSE;
}

// Assumes gTacticalStatus.fEnemyInSector
BOOLEAN OnlyHostileCivsInSector() {
  struct SOLDIERTYPE *pSoldier;
  int32_t i;
  BOOLEAN fHostileCivs = FALSE;

  // Look for any hostile civs.
  for (i = gTacticalStatus.Team[CIV_TEAM].bFirstID; i <= gTacticalStatus.Team[CIV_TEAM].bLastID;
       i++) {
    pSoldier = MercPtrs[i];
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      if (!pSoldier->bNeutral) {
        fHostileCivs = TRUE;
        break;
      }
    }
  }
  if (!fHostileCivs) {  // No hostile civs, so return FALSE
    return FALSE;
  }
  // Look for anybody else hostile.  If found, return FALSE immediately.
  for (i = gTacticalStatus.Team[ENEMY_TEAM].bFirstID; i <= gTacticalStatus.Team[ENEMY_TEAM].bLastID;
       i++) {
    pSoldier = MercPtrs[i];
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      if (!pSoldier->bNeutral) {
        return FALSE;
      }
    }
  }
  for (i = gTacticalStatus.Team[CREATURE_TEAM].bFirstID;
       i <= gTacticalStatus.Team[CREATURE_TEAM].bLastID; i++) {
    pSoldier = MercPtrs[i];
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      if (!pSoldier->bNeutral) {
        return FALSE;
      }
    }
  }
  for (i = gTacticalStatus.Team[MILITIA_TEAM].bFirstID;
       i <= gTacticalStatus.Team[MILITIA_TEAM].bLastID; i++) {
    pSoldier = MercPtrs[i];
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife) {
      if (!pSoldier->bNeutral) {
        return FALSE;
      }
    }
  }
  // We only have hostile civilians, don't allow time compression.
  return TRUE;
}
