// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/StrategicEventHandler.h"

#include "Laptop/BobbyRMailOrder.h"
#include "Laptop/Email.h"
#include "Laptop/History.h"
#include "SGP/MemMan.h"
#include "SGP/Random.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/HandleItems.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/ItemTypes.h"
#include "Tactical/Items.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/TacticalSave.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"

#define MEDUNA_ITEM_DROP_OFF_GRIDNO 10959
#define MEDUNA_ITEM_DROP_OFF_SECTOR_X 3
#define MEDUNA_ITEM_DROP_OFF_SECTOR_Y 14
#define MEDUNA_ITEM_DROP_OFF_SECTOR_Z 0

extern int16_t gsRobotGridNo;

uint32_t guiPabloExtraDaysBribed = 0;

uint8_t gubCambriaMedicalObjects;

void DropOffItemsInMeduna(uint8_t ubOrderNum);

void BobbyRayPurchaseEventCallback(uint8_t ubOrderID) {
  uint8_t i, j;
  uint16_t usItem;
  struct OBJECTTYPE Object;
  uint16_t usMapPos, usStandardMapPos;
  uint16_t usNumberOfItems;
  BOOLEAN fSectorLoaded = FALSE;
  uint32_t uiCount = 0, uiStolenCount = 0;
  static uint8_t ubShipmentsSinceNoBribes = 0;
  uint32_t uiChanceOfTheft;
  BOOLEAN fPablosStoleSomething = FALSE;
  BOOLEAN fPablosStoleLastItem = FALSE;
  struct OBJECTTYPE *pObject = NULL;
  struct OBJECTTYPE *pStolenObject = NULL;
  BOOLEAN fThisShipmentIsFromJohnKulba = FALSE;  // if it is, dont add an email
  uint8_t ubItemsDelivered;
  uint8_t ubTempNumItems;
  uint8_t ubItemsPurchased;

  usStandardMapPos = BOBBYR_SHIPPING_DEST_GRIDNO;

  // if the delivery is for meduna, drop the items off there instead
  if (gpNewBobbyrShipments[ubOrderID].fActive &&
      gpNewBobbyrShipments[ubOrderID].ubDeliveryLoc == BR_MEDUNA) {
    DropOffItemsInMeduna(ubOrderID);
    return;
  }

  if (CheckFact(FACT_NEXT_PACKAGE_CAN_BE_LOST, 0)) {
    SetFactFalse(FACT_NEXT_PACKAGE_CAN_BE_LOST);
    if (Random(100) < 50) {
      // lose the whole shipment!
      gpNewBobbyrShipments[ubOrderID].fActive = FALSE;
      SetFactTrue(FACT_LAST_SHIPMENT_CRASHED);
      return;
    }

  } else if (CheckFact(FACT_NEXT_PACKAGE_CAN_BE_DELAYED, 0)) {
    // shipment went to wrong airport... reroute all items to a temporary
    // gridno to represent the other airport (and damage them)
    SetFactTrue(FACT_LAST_SHIPMENT_WENT_TO_WRONG_AIRPORT);
    usStandardMapPos = LOST_SHIPMENT_GRIDNO;
    SetFactFalse(FACT_NEXT_PACKAGE_CAN_BE_DELAYED);
  } else if ((gTownLoyalty[DRASSEN].ubRating < 20) ||
             StrategicMap[GetSectorID16(13, MAP_ROW_B)].fEnemyControlled) {
    // loss of the whole shipment
    gpNewBobbyrShipments[ubOrderID].fActive = FALSE;

    SetFactTrue(FACT_AGENTS_PREVENTED_SHIPMENT);
    return;
  }

  // Must get the total number of items ( all item types plus how many of each item type ordered )
  usNumberOfItems = 0;
  for (i = 0; i < gpNewBobbyrShipments[ubOrderID].ubNumberPurchases; i++) {
    // Count how many items were purchased
    usNumberOfItems += gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].ubNumberPurchased;

    // if any items are AutoMags
    if (gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].usItemIndex == AUTOMAG_III) {
      // This shipment is from John Kulba, dont add an email from bobby ray
      fThisShipmentIsFromJohnKulba = TRUE;
    }
  }

  // determine if the sector is loaded
  if ((gWorldSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X) &&
      (gWorldSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y) &&
      (gbWorldSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z))
    fSectorLoaded = TRUE;
  else
    fSectorLoaded = FALSE;

  // set crate to closed!
  if (fSectorLoaded) {
    SetOpenableStructureToClosed(BOBBYR_SHIPPING_DEST_GRIDNO, 0);
  } else {
    ChangeStatusOfOpenableStructInUnloadedSector(
        BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y, BOBBYR_SHIPPING_DEST_SECTOR_Z,
        BOBBYR_SHIPPING_DEST_GRIDNO, FALSE);
  }

  // if we are NOT currently in the right sector
  if (!fSectorLoaded) {
    // build an array of objects to be added
    pObject = (struct OBJECTTYPE *)MemAlloc(sizeof(struct OBJECTTYPE) * usNumberOfItems);
    pStolenObject = (struct OBJECTTYPE *)MemAlloc(sizeof(struct OBJECTTYPE) * usNumberOfItems);
    if (pObject == NULL || pStolenObject == NULL) return;
    memset(pObject, 0, sizeof(struct OBJECTTYPE) * usNumberOfItems);
    memset(pStolenObject, 0, sizeof(struct OBJECTTYPE) * usNumberOfItems);
  }

  // check for potential theft
  if (CheckFact(FACT_PABLO_WONT_STEAL, 0)) {
    uiChanceOfTheft = 0;
  } else if (CheckFact(FACT_PABLOS_BRIBED, 0)) {
    // Since Pacos has some money, reduce record of # of shipments since last bribed...
    ubShipmentsSinceNoBribes /= 2;
    uiChanceOfTheft = 0;
  } else {
    ubShipmentsSinceNoBribes++;
    // this chance might seem high but it's only applied at most to every second item
    uiChanceOfTheft = 12 + Random(4 * ubShipmentsSinceNoBribes);
  }

  uiCount = 0;
  for (i = 0; i < gpNewBobbyrShipments[ubOrderID].ubNumberPurchases; i++) {
    // Get the item
    usItem = gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].usItemIndex;

    // Create the item
    CreateItem(usItem, gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].bItemQuality, &Object);

    // if it's a gun
    if (Item[usItem].usItemClass == IC_GUN) {
      // Empty out the bullets put in by CreateItem().  We now sell all guns empty of bullets.  This
      // is done for BobbyR simply to be consistent with the dealers in Arulco, who must sell guns
      // empty to prevent ammo cheats by players.
      Object.ubGunShotsLeft = 0;
    }

    ubItemsDelivered = 0;

    // add all the items that were purchased
    ubItemsPurchased = gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].ubNumberPurchased;
    for (j = 0; j < ubItemsPurchased; j++) {
      // Pablos might steal stuff but only:
      // - if it's one of a group of items
      // - if he didn't steal the previous item in the group (so he never steals > 50%)
      // - if he has been bribed, he only sneaks out stuff which is cheap
      if (fSectorLoaded) {
        // add ubItemsPurchased to the chance of theft so the chance increases when there are more
        // items of a kind being ordered
        if (!fPablosStoleLastItem && uiChanceOfTheft > 0 &&
            Random(100) < (uiChanceOfTheft + ubItemsPurchased)) {
          uiStolenCount++;
          usMapPos = PABLOS_STOLEN_DEST_GRIDNO;  // off screen!
          fPablosStoleSomething = TRUE;
          fPablosStoleLastItem = TRUE;
        } else {
          usMapPos = usStandardMapPos;
          fPablosStoleLastItem = FALSE;

          if (usStandardMapPos == LOST_SHIPMENT_GRIDNO) {
            // damage the item a random amount!
            Object.bStatus[0] = (int8_t)(((70 + Random(11)) * (int32_t)Object.bStatus[0]) / 100);
            // make damn sure it can't hit 0
            if (Object.bStatus[0] == 0) {
              Object.bStatus[0] = 1;
            }
            AddItemToPool(usMapPos, &Object, -1, 0, 0, 0);
          } else {
            // record # delivered for later addition...
            ubItemsDelivered++;
          }
        }
      } else {
        if (j > 1 && !fPablosStoleLastItem && uiChanceOfTheft > 0 &&
            Random(100) < (uiChanceOfTheft + j)) {
          memcpy(&pStolenObject[uiStolenCount], &Object, sizeof(struct OBJECTTYPE));
          uiStolenCount++;
          fPablosStoleSomething = TRUE;
          fPablosStoleLastItem = TRUE;
        } else {
          fPablosStoleLastItem = FALSE;

          // else we are not currently in the sector, so we build an array of items to add in one
          // lump add the item to the item array

          if (usStandardMapPos == LOST_SHIPMENT_GRIDNO) {
            // damage the item a random amount!
            Object.bStatus[0] = (int8_t)(((70 + Random(11)) * (int32_t)Object.bStatus[0]) / 100);
            // make damn sure it can't hit 0
            if (Object.bStatus[0] == 0) {
              Object.bStatus[0] = 1;
            }
            memcpy(&pObject[uiCount], &Object, sizeof(struct OBJECTTYPE));
            uiCount++;
          } else {
            ubItemsDelivered++;
          }
        }
      }
    }

    if (gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].ubNumberPurchased == 1 &&
        ubItemsDelivered == 1) {
      // the item in Object will be the item to deliver
      if (fSectorLoaded) {
        AddItemToPool(usStandardMapPos, &Object, -1, 0, 0, 0);
      } else {
        memcpy(&pObject[uiCount], &Object, sizeof(struct OBJECTTYPE));
        uiCount++;
      }
    } else {
      while (ubItemsDelivered) {
        // treat 0s as 1s :-)
        ubTempNumItems = min(ubItemsDelivered, max(1, Item[usItem].ubPerPocket));
        CreateItems(usItem, gpNewBobbyrShipments[ubOrderID].BobbyRayPurchase[i].bItemQuality,
                    ubTempNumItems, &Object);

        // stack as many as possible
        if (fSectorLoaded) {
          AddItemToPool(usStandardMapPos, &Object, -1, 0, 0, 0);
        } else {
          memcpy(&pObject[uiCount], &Object, sizeof(struct OBJECTTYPE));
          uiCount++;
        }

        ubItemsDelivered -= ubTempNumItems;
      }
    }
  }

  // if we are NOT currently in the sector
  if (!fSectorLoaded) {
    // add all the items from the array that was built above
    usMapPos = PABLOS_STOLEN_DEST_GRIDNO;
    // The item are to be added to the Top part of Drassen, grid loc's  10112, 9950
    if (!AddItemsToUnLoadedSector(BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y,
                                  BOBBYR_SHIPPING_DEST_SECTOR_Z, usStandardMapPos, uiCount, pObject,
                                  0, 0, 0, -1, FALSE)) {
      // Error adding the items
      // return;
    }
    if (uiStolenCount > 0) {
      if (!AddItemsToUnLoadedSector(BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y,
                                    BOBBYR_SHIPPING_DEST_SECTOR_Z, PABLOS_STOLEN_DEST_GRIDNO,
                                    uiStolenCount, pStolenObject, 0, 0, 0, -1, FALSE)) {
        // Error adding the items
        // return;
      }
    }
    MemFree(pObject);
    MemFree(pStolenObject);
    pObject = NULL;
    pStolenObject = NULL;
  }

  if (fPablosStoleSomething) {
    SetFactTrue(FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT);
  } else {
    SetFactFalse(FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT);
  }

  SetFactFalse(FACT_LARGE_SIZED_OLD_SHIPMENT_WAITING);

  if (CheckFact(FACT_NEXT_PACKAGE_CAN_BE_DELAYED, 0)) {
    SetFactFalse(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
    SetFactFalse(FACT_LARGE_SIZED_SHIPMENT_WAITING);
    SetFactFalse(FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);
  } else {
    if (usNumberOfItems - uiStolenCount <= 5) {
      SetFactFalse(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
      SetFactFalse(FACT_LARGE_SIZED_SHIPMENT_WAITING);
    } else if (usNumberOfItems - uiStolenCount <= 15) {
      SetFactTrue(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
      SetFactFalse(FACT_LARGE_SIZED_SHIPMENT_WAITING);
    } else {
      SetFactFalse(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
      SetFactTrue(FACT_LARGE_SIZED_SHIPMENT_WAITING);
    }

    // this shipment isn't old yet...
    SetFactTrue(FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);

    // set up even to make shipment "old"
    AddSameDayStrategicEvent(EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay() + 120,
                             FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);
  }

  // We have received the shipment so fActice becomes fALSE
  gpNewBobbyrShipments[ubOrderID].fActive = FALSE;

  // Stop time compression the game
  StopTimeCompression();

  // if the shipment is NOT from John Kulba, send an email
  if (!fThisShipmentIsFromJohnKulba) {
    // Add an email from Bobby r telling the user the shipment 'Should' be there
    AddEmail(BOBBYR_SHIPMENT_ARRIVED, BOBBYR_SHIPMENT_ARRIVED_LENGTH, BOBBY_R, GetWorldTotalMin());
  } else {
    // if the shipment is from John Kulba

    // Add an email from kulba telling the user the shipment is there
    AddEmail(JOHN_KULBA_GIFT_IN_DRASSEN, JOHN_KULBA_GIFT_IN_DRASSEN_LENGTH, JOHN_KULBA,
             GetWorldTotalMin());
  }
}

void HandleDelayedItemsArrival(uint32_t uiReason) {
  // This function moves all the items that Pablos has stolen
  // (or items that were delayed) to the arrival location for new shipments,
  int16_t sStartGridNo;
  uint32_t uiNumWorldItems, uiLoop;
  BOOLEAN fOk;
  WORLDITEM *pTemp;
  uint8_t ubLoop;
  struct OBJECTTYPE Object;

  if (uiReason == NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_RETURN_STOLEN_SHIPMENT_ITEMS) {
    if (gMercProfiles[PABLO].bMercStatus == MERC_IS_DEAD) {
      // nothing arrives then!
      return;
    }
    // update some facts...
    SetFactTrue(FACT_PABLO_RETURNED_GOODS);
    SetFactFalse(FACT_PABLO_PUNISHED_BY_PLAYER);
    sStartGridNo = PABLOS_STOLEN_DEST_GRIDNO;

    // add random items

    for (ubLoop = 0; ubLoop < 2; ubLoop++) {
      switch (Random(10)) {
        case 0:
          // 1 in 10 chance of a badly damaged gas mask
          CreateItem(GASMASK, (int8_t)(20 + Random(10)), &Object);
          break;
        case 1:
        case 2:
          // 2 in 10 chance of a battered Desert Eagle
          CreateItem(DESERTEAGLE, (int8_t)(40 + Random(10)), &Object);
          break;
        case 3:
        case 4:
        case 5:
          // 3 in 10 chance of a stun grenade
          CreateItem(STUN_GRENADE, (int8_t)(70 + Random(10)), &Object);
          break;
        case 6:
        case 7:
        case 8:
        case 9:
          // 4 in 10 chance of two 38s!
          CreateItems(SW38, (int8_t)(90 + Random(10)), 2, &Object);
          break;
      }
      if ((gWorldSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X) &&
          (gWorldSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y) &&
          (gbWorldSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z)) {
        AddItemToPool(BOBBYR_SHIPPING_DEST_GRIDNO, &Object, -1, 0, 0, 0);
      } else {
        AddItemsToUnLoadedSector(BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y,
                                 BOBBYR_SHIPPING_DEST_SECTOR_Z, BOBBYR_SHIPPING_DEST_GRIDNO, 1,
                                 &Object, 0, 0, 0, -1, FALSE);
      }
    }
  } else if (uiReason == FACT_PACKAGE_DAMAGED) {
    sStartGridNo = LOST_SHIPMENT_GRIDNO;
  } else {
    return;
  }

  // If the Drassen airport sector is already loaded, move the item pools...
  if ((gWorldSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X) &&
      (gWorldSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y) &&
      (gbWorldSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z)) {
    // sector is loaded!
    // just move the hidden item pool
    MoveItemPools(sStartGridNo, BOBBYR_SHIPPING_DEST_GRIDNO);
  } else {
    // otherwise load the saved items from the item file and change the records of their locations
    fOk = GetNumberOfWorldItemsFromTempItemFile(
        BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y, BOBBYR_SHIPPING_DEST_SECTOR_Z,
        &uiNumWorldItems, FALSE);
    if (!fOk) {
      return;
    }
    pTemp = (WORLDITEM *)MemAlloc(sizeof(WORLDITEM) * uiNumWorldItems);
    if (!pTemp) {
      return;
    }
    fOk =
        LoadWorldItemsFromTempItemFile(BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y,
                                       BOBBYR_SHIPPING_DEST_SECTOR_Z, pTemp);
    if (fOk) {
      for (uiLoop = 0; uiLoop < uiNumWorldItems; uiLoop++) {
        if (pTemp[uiLoop].sGridNo == PABLOS_STOLEN_DEST_GRIDNO) {
          pTemp[uiLoop].sGridNo = BOBBYR_SHIPPING_DEST_GRIDNO;
        }
      }
      AddWorldItemsToUnLoadedSector(BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y,
                                    BOBBYR_SHIPPING_DEST_SECTOR_Z, 0, uiNumWorldItems, pTemp, TRUE);
    }
  }
}

void AddSecondAirportAttendant(void) {
  // add the second airport attendant to the Drassen airport...
  gMercProfiles[99].sSectorX = BOBBYR_SHIPPING_DEST_SECTOR_X;
  gMercProfiles[99].sSectorY = BOBBYR_SHIPPING_DEST_SECTOR_Y;
  gMercProfiles[99].bSectorZ = BOBBYR_SHIPPING_DEST_SECTOR_Z;
}

void SetPabloToUnbribed(void) {
  if (guiPabloExtraDaysBribed > 0) {
    // set new event for later on, because the player gave Pablo more money!
    AddFutureDayStrategicEvent(EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), FACT_PABLOS_BRIBED,
                               guiPabloExtraDaysBribed);
    guiPabloExtraDaysBribed = 0;
  } else {
    SetFactFalse(FACT_PABLOS_BRIBED);
  }
}

void HandlePossiblyDamagedPackage(void) {
  if (Random(100) < 70) {
    SetFactTrue(FACT_PACKAGE_DAMAGED);
    HandleDelayedItemsArrival(FACT_PACKAGE_DAMAGED);
  } else {
    // shipment lost forever!
    SetFactTrue(FACT_PACKAGE_LOST_PERMANENTLY);
  }
  // whatever happened, the shipment is no longer delayed
  SetFactFalse(FACT_SHIPMENT_DELAYED_24_HOURS);
}

void CheckForKingpinsMoneyMissing(BOOLEAN fFirstCheck) {
  uint32_t uiLoop;
  uint32_t uiTotalCash = 0;
  BOOLEAN fKingpinWillDiscover = FALSE, fKingpinDiscovers = FALSE;

  // money in D5b1 must be less than 30k

  for (uiLoop = 0; uiLoop < guiNumWorldItems; uiLoop++) {
    // loop through all items, look for ownership
    if (gWorldItems[uiLoop].fExists && gWorldItems[uiLoop].o.usItem == MONEY) {
      uiTotalCash += gWorldItems[uiLoop].o.uiMoneyAmount;
    }
  }

  // This function should be called every time sector D5/B1 is unloaded!
  if (fFirstCheck) {
    if (CheckFact(FACT_KINGPIN_WILL_LEARN_OF_MONEY_GONE, 0) == TRUE) {
      // unnecessary
      return;
    }

    if (uiTotalCash < 30000) {
      // add history log here
      AddHistoryToPlayersLog(HISTORY_FOUND_MONEY, 0, GetWorldTotalMin(), (uint8_t)gWorldSectorX,
                             (uint8_t)gWorldSectorY);

      SetFactTrue(FACT_KINGPIN_WILL_LEARN_OF_MONEY_GONE);
    }
  }

  if (CheckFact(FACT_KINGPIN_DEAD, 0) == TRUE) {
    return;
  }

  if (uiTotalCash < 30000) {
    if (fFirstCheck) {
      // add event to make Kingpin aware, two days from now
      fKingpinWillDiscover = TRUE;
    } else {
      fKingpinDiscovers = TRUE;
    }
  }

  if (fKingpinWillDiscover) {
    // set event for next day to check for real
    AddFutureDayStrategicEvent(EVENT_SET_BY_NPC_SYSTEM, Random(120), FACT_KINGPIN_KNOWS_MONEY_GONE,
                               1);

    // the sector is unloaded NOW so set Kingpin's balance and remove the cash
    gMercProfiles[KINGPIN].iBalance = -(30000 - (int32_t)uiTotalCash);
    // remove all money from map
    for (uiLoop = 0; uiLoop < guiNumWorldItems; uiLoop++) {
      // loop through all items, look for ownership
      if (gWorldItems[uiLoop].fExists && gWorldItems[uiLoop].o.usItem == MONEY) {
        // remove!
        gWorldItems[uiLoop].fExists = FALSE;
      }
    }
  } else if (fKingpinDiscovers) {
    // ok start things up here!
    SetFactTrue(FACT_KINGPIN_KNOWS_MONEY_GONE);

    // set event 2 days from now that if the player has not given Kingpin his money back,
    // he sends email to the player
    AddFutureDayStrategicEvent(EVENT_SET_BY_NPC_SYSTEM, Random(120), FACT_KINGPIN_KNOWS_MONEY_GONE,
                               2);
  }
}

void HandleNPCSystemEvent(uint32_t uiEvent) {
  if (uiEvent < NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS) {
    switch (uiEvent) {
      case FACT_PABLOS_BRIBED:
        // set Pacos to unbribed
        SetPabloToUnbribed();
        break;

      case FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING:
        // the shipment is no longer really new
        SetFactFalse(FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);
        if (CheckFact(FACT_LARGE_SIZED_SHIPMENT_WAITING, 0)) {
          // set "really heavy old shipment" fact
          SetFactTrue(FACT_LARGE_SIZED_OLD_SHIPMENT_WAITING);
        }
        break;

      case FACT_SHIPMENT_DELAYED_24_HOURS:
      case FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO:
      case FACT_24_HOURS_SINCE_JOEY_RESCUED:
        SetFactTrue((uint16_t)uiEvent);
        break;

      case FACT_KINGPIN_KNOWS_MONEY_GONE:
        // more generally events for kingpin quest
        if (CheckFact(FACT_KINGPIN_KNOWS_MONEY_GONE, 0) == FALSE) {
          // check for real whether to start quest
          CheckForKingpinsMoneyMissing(FALSE);
        } else if (CheckFact(FACT_KINGPIN_DEAD, 0) == FALSE) {
          if (gubQuest[QUEST_KINGPIN_MONEY] == QUESTNOTSTARTED) {
            // KP knows money is gone, hasn't told player, if this event is called then the 2
            // days are up... send email
            AddEmail(KING_PIN_LETTER, KING_PIN_LETTER_LENGTH, KING_PIN, GetWorldTotalMin());
            StartQuest(QUEST_KINGPIN_MONEY, 5, MAP_ROW_D);
            // add event to send terrorists two days from now
            AddFutureDayStrategicEvent(EVENT_SET_BY_NPC_SYSTEM, Random(120),
                                       FACT_KINGPIN_KNOWS_MONEY_GONE, 2);
          } else if (gubQuest[QUEST_KINGPIN_MONEY] == QUESTINPROGRESS) {
            // knows money gone, quest is still in progress
            // event indicates Kingpin can start to send terrorists
            SetFactTrue(FACT_KINGPIN_CAN_SEND_ASSASSINS);
            gMercProfiles[SPIKE].sSectorX = 5;
            gMercProfiles[SPIKE].sSectorY = MAP_ROW_C;
            gTacticalStatus.fCivGroupHostile[KINGPIN_CIV_GROUP] = CIV_GROUP_WILL_BECOME_HOSTILE;
          }
        }
        break;
    }
  } else {
    switch (uiEvent - NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS) {
      case NPC_ACTION_RETURN_STOLEN_SHIPMENT_ITEMS:
        HandleDelayedItemsArrival(uiEvent);
        break;
      case NPC_ACTION_SET_RANDOM_PACKAGE_DAMAGE_TIMER:
        HandlePossiblyDamagedPackage();
        break;
      case NPC_ACTION_ENABLE_CAMBRIA_DOCTOR_BONUS:
        SetFactTrue(FACT_WILLIS_HEARD_ABOUT_JOEY_RESCUE);
        break;
      case NPC_ACTION_TRIGGER_END_OF_FOOD_QUEST:
        if (gMercProfiles[FATHER].bMercStatus != MERC_IS_DEAD) {
          EndQuest(QUEST_FOOD_ROUTE, 10, MAP_ROW_A);
          SetFactTrue(FACT_FOOD_QUEST_OVER);
        }
        break;
      case NPC_ACTION_DELAYED_MAKE_BRENDA_LEAVE:
        // IC:
        // TriggerNPCRecord( 85, 9 );
        SetFactTrue(FACT_BRENDA_PATIENCE_TIMER_EXPIRED);
        break;
      case NPC_ACTION_SET_DELAY_TILL_GIRLS_AVAILABLE:
        HandleNPCDoAction(107, NPC_ACTION_SET_GIRLS_AVAILABLE, 0);
        break;

      case NPC_ACTION_READY_ROBOT: {
        if (CheckFact(FACT_FIRST_ROBOT_DESTROYED, 0)) {
          // second robot ready
          SetFactTrue(FACT_ROBOT_READY_SECOND_TIME);
          // resurrect robot
          gMercProfiles[ROBOT].bLife = gMercProfiles[ROBOT].bLifeMax;
          gMercProfiles[ROBOT].bMercStatus = MERC_OK;
        } else {
          // first robot ready
          SetFactTrue(FACT_ROBOT_READY);
        }

        gMercProfiles[ROBOT].sSectorX = gMercProfiles[MADLAB].sSectorX;
        gMercProfiles[ROBOT].sSectorY = gMercProfiles[MADLAB].sSectorY;
        gMercProfiles[ROBOT].bSectorZ = gMercProfiles[MADLAB].bSectorZ;

      } break;

      case NPC_ACTION_ADD_JOEY_TO_WORLD:
        // If Joey is not dead, escorted, or already delivered
        if (gMercProfiles[JOEY].bMercStatus != MERC_IS_DEAD && !CheckFact(FACT_JOEY_ESCORTED, 0) &&
            gMercProfiles[JOEY].sSectorX == 4 && gMercProfiles[JOEY].sSectorY == MAP_ROW_D &&
            gMercProfiles[JOEY].bSectorZ == 1) {
          struct SOLDIERTYPE *pJoey;

          pJoey = FindSoldierByProfileID(JOEY, FALSE);
          if (pJoey) {
            // he's in the currently loaded sector...delay this an hour!
            AddSameDayStrategicEvent(
                EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay() + 60,
                NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_ADD_JOEY_TO_WORLD);
          } else {
            // move Joey from caves to San Mona
            gMercProfiles[JOEY].sSectorX = 5;
            gMercProfiles[JOEY].sSectorY = MAP_ROW_C;
            gMercProfiles[JOEY].bSectorZ = 0;
          }
        }
        break;

      case NPC_ACTION_SEND_ENRICO_MIGUEL_EMAIL:
        AddEmail(ENRICO_MIGUEL, ENRICO_MIGUEL_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
        break;

      case NPC_ACTION_TIMER_FOR_VEHICLE:
        SetFactTrue(FACT_OK_USE_HUMMER);
        break;

      case NPC_ACTION_FREE_KIDS:
        SetFactTrue(FACT_KIDS_ARE_FREE);
        break;

      default:
        break;
    }
  }
}

void HandleEarlyMorningEvents(void) {
  uint32_t cnt;
  uint32_t uiAmount;

  // loop through all *NPCs* and reset "default response used recently" flags
  for (cnt = FIRST_RPC; cnt < NUM_PROFILES; cnt++) {
    gMercProfiles[cnt].bFriendlyOrDirectDefaultResponseUsedRecently = FALSE;
    gMercProfiles[cnt].bRecruitDefaultResponseUsedRecently = FALSE;
    gMercProfiles[cnt].bThreatenDefaultResponseUsedRecently = FALSE;
    gMercProfiles[cnt].ubMiscFlags2 &= (~PROFILE_MISC_FLAG2_BANDAGED_TODAY);
  }
  // reset Father Walker's drunkenness level!
  gMercProfiles[FATHER].bNPCData = (int8_t)Random(4);
  // set Walker's location
  if (Random(2)) {
    // move the father to the other sector, provided neither are loaded
    if (!((gWorldSectorX == 13) && ((gWorldSectorY == MAP_ROW_C) || gWorldSectorY == MAP_ROW_D) &&
          (gbWorldSectorZ == 0))) {
      gMercProfiles[FATHER].sSectorX = 13;
      // swap his location
      if (gMercProfiles[FATHER].sSectorY == MAP_ROW_C) {
        gMercProfiles[FATHER].sSectorY = MAP_ROW_D;
      } else {
        gMercProfiles[FATHER].sSectorY = MAP_ROW_C;
      }
    }
  }

  if (gMercProfiles[TONY].ubLastDateSpokenTo > 0 &&
      !(gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C && gbWorldSectorZ == 0)) {
    // San Mona C5 is not loaded so make Tony possibly not available
    if (Random(4)) {
      // Tony IS available
      SetFactFalse(FACT_TONY_NOT_AVAILABLE);
      gMercProfiles[TONY].sSectorX = 5;
      gMercProfiles[TONY].sSectorY = MAP_ROW_C;
    } else {
      // Tony is NOT available
      SetFactTrue(FACT_TONY_NOT_AVAILABLE);
      gMercProfiles[TONY].sSectorX = 0;
      gMercProfiles[TONY].sSectorY = 0;
    }
  }

  if (gMercProfiles[DEVIN].ubLastDateSpokenTo == 0) {
    // Does Devin move?
    gMercProfiles[DEVIN].bNPCData++;
    if (gMercProfiles[DEVIN].bNPCData > 3) {
      if (!((gWorldSectorX == gMercProfiles[DEVIN].sSectorX) &&
            (gWorldSectorY == gMercProfiles[DEVIN].sSectorY) && (gbWorldSectorZ == 0))) {
        // ok, Devin's sector not loaded, so time to move!
        // might be same sector as before, if so, oh well!
        switch (Random(5)) {
          case 0:
            gMercProfiles[DEVIN].sSectorX = 9;
            gMercProfiles[DEVIN].sSectorY = MAP_ROW_G;
            break;
          case 1:
            gMercProfiles[DEVIN].sSectorX = 13;
            gMercProfiles[DEVIN].sSectorY = MAP_ROW_D;
            break;
          case 2:
            gMercProfiles[DEVIN].sSectorX = 5;
            gMercProfiles[DEVIN].sSectorY = MAP_ROW_C;
            break;
          case 3:
            gMercProfiles[DEVIN].sSectorX = 2;
            gMercProfiles[DEVIN].sSectorY = MAP_ROW_H;
            break;
          case 4:
            gMercProfiles[DEVIN].sSectorX = 6;
            gMercProfiles[DEVIN].sSectorY = MAP_ROW_C;
            break;
        }
      }
    }
  }

  // Does Hamous move?

  // stop moving the truck if Hamous is dead!!
  // stop moving them if the player has the truck or Hamous is hired!
  if (gMercProfiles[HAMOUS].bLife > 0 && FindSoldierByProfileID(HAMOUS, TRUE) == NULL &&
      FindSoldierByProfileID(PROF_ICECREAM, TRUE) == NULL &&
      (!((gWorldSectorX == gMercProfiles[HAMOUS].sSectorX) &&
         (gWorldSectorY == gMercProfiles[HAMOUS].sSectorY) && (gbWorldSectorZ == 0)))) {
    // ok, HAMOUS's sector not loaded, so time to move!
    // might be same sector as before, if so, oh well!
    switch (Random(5)) {
      case 0:
        gMercProfiles[HAMOUS].sSectorX = 6;
        gMercProfiles[HAMOUS].sSectorY = MAP_ROW_G;
        gMercProfiles[PROF_ICECREAM].sSectorX = 6;
        gMercProfiles[PROF_ICECREAM].sSectorY = MAP_ROW_G;
        break;
      case 1:
        gMercProfiles[HAMOUS].sSectorX = 12;
        gMercProfiles[HAMOUS].sSectorY = MAP_ROW_F;
        gMercProfiles[PROF_ICECREAM].sSectorX = 12;
        gMercProfiles[PROF_ICECREAM].sSectorY = MAP_ROW_F;
        break;
      case 2:
        gMercProfiles[HAMOUS].sSectorX = 7;
        gMercProfiles[HAMOUS].sSectorY = MAP_ROW_D;
        gMercProfiles[PROF_ICECREAM].sSectorX = 7;
        gMercProfiles[PROF_ICECREAM].sSectorY = MAP_ROW_D;
        break;
      case 3:
        gMercProfiles[HAMOUS].sSectorX = 3;
        gMercProfiles[HAMOUS].sSectorY = MAP_ROW_D;
        gMercProfiles[PROF_ICECREAM].sSectorX = 3;
        gMercProfiles[PROF_ICECREAM].sSectorY = MAP_ROW_D;
        break;
      case 4:
        gMercProfiles[HAMOUS].sSectorX = 9;
        gMercProfiles[HAMOUS].sSectorY = MAP_ROW_D;
        gMercProfiles[PROF_ICECREAM].sSectorX = 9;
        gMercProfiles[PROF_ICECREAM].sSectorY = MAP_ROW_D;
        break;
    }
  }

  // Does Rat take off?
  if (gMercProfiles[RAT].bNPCData != 0) {
    gMercProfiles[RAT].sSectorX = 0;
    gMercProfiles[RAT].sSectorY = 0;
    gMercProfiles[RAT].bSectorZ = 0;
  }

  // Empty money from pockets of Vince 69, Willis 80, and Jenny 132
  SetMoneyInSoldierProfile(VINCE, 0);
  SetMoneyInSoldierProfile(STEVE, 0);  // Steven Willis
  SetMoneyInSoldierProfile(JENNY, 0);

  // Vince is no longer expecting money
  SetFactFalse(FACT_VINCE_EXPECTING_MONEY);

  // Reset Darren's balance and money
  gMercProfiles[DARREN].iBalance = 0;
  SetMoneyInSoldierProfile(DARREN, 15000);

  // set Carmen to be placed on the map in case he moved and is waiting off screen
  if (gMercProfiles[CARMEN].ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR) {
    gMercProfiles[CARMEN].ubMiscFlags2 &= ~(PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR);
    // move Carmen to C13
    gMercProfiles[CARMEN].sSectorX = 13;
    gMercProfiles[CARMEN].sSectorY = MAP_ROW_C;
    gMercProfiles[CARMEN].bSectorZ = 0;

    // we should also reset # of terrorist heads and give him cash
    if (gMercProfiles[CARMEN].bNPCData2 > 0) {
      if (gMercProfiles[CARMEN].uiMoney < 10000) {
        uiAmount = 0;
      } else {
        uiAmount = gMercProfiles[CARMEN].uiMoney;
      }
      uiAmount += 10000 * gMercProfiles[CARMEN].bNPCData2;
      SetMoneyInSoldierProfile(CARMEN, uiAmount);
      gMercProfiles[CARMEN].bNPCData2 = 0;

      for (cnt = HEAD_1; cnt <= HEAD_7; cnt++) {
        RemoveObjectFromSoldierProfile(CARMEN, (uint8_t)cnt);
      }
    }
  } else {
    // randomize where he'll be today... so long as his sector's not loaded

    if (gMercProfiles[CARMEN].sSectorX != gWorldSectorX ||
        gMercProfiles[CARMEN].sSectorY != gWorldSectorY) {
      switch (Random(3)) {
        case 0:
          gMercProfiles[CARMEN].sSectorX = 5;
          gMercProfiles[CARMEN].sSectorY = MAP_ROW_C;
          break;
        case 1:
          gMercProfiles[CARMEN].sSectorX = 13;
          gMercProfiles[CARMEN].sSectorY = MAP_ROW_C;
          break;
        case 2:
          gMercProfiles[CARMEN].sSectorX = 9;
          gMercProfiles[CARMEN].sSectorY = MAP_ROW_G;
          break;
      }
      // he should have $5000... unless the player forgot to meet him
      if (gMercProfiles[CARMEN].uiMoney < 5000) {
        SetMoneyInSoldierProfile(CARMEN, 5000);
      }
    }
  }

  if (PreRandom(3) == 0) {
    SetFactTrue(FACT_DAVE_HAS_GAS);
  } else {
    SetFactFalse(FACT_DAVE_HAS_GAS);
  }

  if (gWorldSectorX == HOSPITAL_SECTOR_X && gWorldSectorY == HOSPITAL_SECTOR_Y &&
      gbWorldSectorZ == HOSPITAL_SECTOR_Z) {
    CheckForMissingHospitalSupplies();
  }
}

void MakeCivGroupHostileOnNextSectorEntrance(uint8_t ubCivGroup) {
  // if it's the rebels that will become hostile, reduce town loyalties NOW, not later
  if (ubCivGroup == REBEL_CIV_GROUP &&
      gTacticalStatus.fCivGroupHostile[ubCivGroup] == CIV_GROUP_NEUTRAL) {
    ReduceLoyaltyForRebelsBetrayed();
  }

  gTacticalStatus.fCivGroupHostile[ubCivGroup] = CIV_GROUP_WILL_BECOME_HOSTILE;
}

void RemoveAssassin(uint8_t ubProfile) {
  gMercProfiles[ubProfile].sSectorX = 0;
  gMercProfiles[ubProfile].sSectorY = 0;
  gMercProfiles[ubProfile].bLife = gMercProfiles[ubProfile].bLifeMax;
}

void CheckForMissingHospitalSupplies(void) {
  uint32_t uiLoop;
  struct ITEM_POOL *pItemPool;
  struct OBJECTTYPE *pObj;
  uint8_t ubMedicalObjects = 0;

  for (uiLoop = 0; uiLoop < guiNumWorldItems; uiLoop++) {
    // loop through all items, look for ownership
    if (gWorldItems[uiLoop].fExists && gWorldItems[uiLoop].o.usItem == OWNERSHIP &&
        gWorldItems[uiLoop].o.ubOwnerCivGroup == DOCTORS_CIV_GROUP) {
      GetItemPool(gWorldItems[uiLoop].sGridNo, &pItemPool, 0);
      while (pItemPool) {
        pObj = &(gWorldItems[pItemPool->iItemIndex].o);

        if (pObj->bStatus[0] > 60) {
          if (pObj->usItem == FIRSTAIDKIT || pObj->usItem == MEDICKIT ||
              pObj->usItem == REGEN_BOOSTER || pObj->usItem == ADRENALINE_BOOSTER) {
            ubMedicalObjects++;
          }
        }

        pItemPool = pItemPool->pNext;
      }
    }
  }

  if (CheckFact(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN, 0) == TRUE) {
    // player returning stuff!  if back to full then can operate
    if (ubMedicalObjects >= gubCambriaMedicalObjects) {
      SetFactFalse(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN);
      SetFactFalse(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES);
      return;
    }
  }

  if (ubMedicalObjects < gubCambriaMedicalObjects) {
    // player's stolen something!
    if (CheckFact(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES, 0) == FALSE) {
      SetFactTrue(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES);
    }

    // if only 1/5 or less left, give up the ghost
    if (ubMedicalObjects * 5 <= gubCambriaMedicalObjects) {
      // run out!
      SetFactTrue(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN);
    }
  }
}

void DropOffItemsInMeduna(uint8_t ubOrderNum) {
  BOOLEAN fSectorLoaded = FALSE;
  struct OBJECTTYPE Object;
  uint32_t uiCount = 0;
  struct OBJECTTYPE *pObject = NULL;
  uint16_t usNumberOfItems = 0, usItem;
  uint8_t ubItemsDelivered, ubTempNumItems;
  uint32_t i;

  // if the player doesnt "own" the sector,
  if (StrategicMap[GetSectorID16(MEDUNA_ITEM_DROP_OFF_SECTOR_X, MEDUNA_ITEM_DROP_OFF_SECTOR_Y)]
          .fEnemyControlled) {
    // the items disappear
    gpNewBobbyrShipments[ubOrderNum].fActive = FALSE;
    return;
  }

  // determine if the sector is loaded
  if ((gWorldSectorX == MEDUNA_ITEM_DROP_OFF_SECTOR_X) &&
      (gWorldSectorY == MEDUNA_ITEM_DROP_OFF_SECTOR_Y) &&
      (gbWorldSectorZ == MEDUNA_ITEM_DROP_OFF_SECTOR_Z))
    fSectorLoaded = TRUE;
  else
    fSectorLoaded = FALSE;

  // set crate to closed!
  if (fSectorLoaded) {
    SetOpenableStructureToClosed(MEDUNA_ITEM_DROP_OFF_GRIDNO, 0);
  } else {
    ChangeStatusOfOpenableStructInUnloadedSector(
        MEDUNA_ITEM_DROP_OFF_SECTOR_X, MEDUNA_ITEM_DROP_OFF_SECTOR_Y, MEDUNA_ITEM_DROP_OFF_SECTOR_Z,
        MEDUNA_ITEM_DROP_OFF_GRIDNO, FALSE);
  }

  for (i = 0; i < gpNewBobbyrShipments[ubOrderNum].ubNumberPurchases; i++) {
    // Count how many items were purchased
    usNumberOfItems += gpNewBobbyrShipments[ubOrderNum].BobbyRayPurchase[i].ubNumberPurchased;
  }

  // if we are NOT currently in the right sector
  if (!fSectorLoaded) {
    // build an array of objects to be added
    pObject = (struct OBJECTTYPE *)MemAlloc(sizeof(struct OBJECTTYPE) * usNumberOfItems);
    if (pObject == NULL) return;
    memset(pObject, 0, sizeof(struct OBJECTTYPE) * usNumberOfItems);
  }

  uiCount = 0;

  // loop through the number of purchases
  for (i = 0; i < gpNewBobbyrShipments->ubNumberPurchases; i++) {
    ubItemsDelivered = gpNewBobbyrShipments[ubOrderNum].BobbyRayPurchase[i].ubNumberPurchased;
    usItem = gpNewBobbyrShipments[ubOrderNum].BobbyRayPurchase[i].usItemIndex;

    while (ubItemsDelivered) {
      // treat 0s as 1s :-)
      ubTempNumItems = min(ubItemsDelivered, max(1, Item[usItem].ubPerPocket));
      CreateItems(usItem, gpNewBobbyrShipments[ubOrderNum].BobbyRayPurchase[i].bItemQuality,
                  ubTempNumItems, &Object);

      // stack as many as possible
      if (fSectorLoaded) {
        AddItemToPool(MEDUNA_ITEM_DROP_OFF_GRIDNO, &Object, -1, 0, 0, 0);
      } else {
        memcpy(&pObject[uiCount], &Object, sizeof(struct OBJECTTYPE));
        uiCount++;
      }

      ubItemsDelivered -= ubTempNumItems;
    }
  }

  // if the sector WASNT loaded
  if (!fSectorLoaded) {
    // add all the items from the array that was built above

    // The item are to be added to the Top part of Drassen, grid loc's  10112, 9950
    if (!AddItemsToUnLoadedSector(MEDUNA_ITEM_DROP_OFF_SECTOR_X, MEDUNA_ITEM_DROP_OFF_SECTOR_Y,
                                  MEDUNA_ITEM_DROP_OFF_SECTOR_Z, MEDUNA_ITEM_DROP_OFF_GRIDNO,
                                  uiCount, pObject, 0, 0, 0, -1, FALSE)) {
      // error
      Assert(0);
    }
    MemFree(pObject);
    pObject = NULL;
  }

  // mark that the shipment has arrived
  gpNewBobbyrShipments[ubOrderNum].fActive = FALSE;

  // Add an email from kulba telling the user the shipment is there
  AddEmail(BOBBY_R_MEDUNA_SHIPMENT, BOBBY_R_MEDUNA_SHIPMENT_LENGTH, BOBBY_R, GetWorldTotalMin());
}
