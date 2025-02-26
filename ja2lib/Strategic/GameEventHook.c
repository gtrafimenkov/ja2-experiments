// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "JAScreens.h"
#include "Laptop/AIMMembers.h"
#include "Laptop/BobbyR.h"
#include "Laptop/BobbyRGuns.h"
#include "Laptop/Email.h"
#include "Laptop/InsuranceContract.h"
#include "Laptop/Laptop.h"
#include "Laptop/Mercs.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/MercContract.h"
#include "Strategic/Quests.h"
#include "Strategic/Scheduling.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMercHandler.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/StrategicTownReputation.h"
#include "Tactical/AirRaid.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/Campaign.h"
#include "Tactical/Menptr.h"
#include "Tactical/MercHiring.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/AmbientControl.h"
#include "TileEngine/Environment.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"

#ifdef JA2BETAVERSION
extern BOOLEAN gfMercsNeverQuit;
#endif

extern void ValidateGameEvents();
extern void HandleHourlyUpdate();
extern void HandleQuarterHourUpdate();
extern void HandleMinuteUpdate();
extern BOOLEAN gfProcessingGameEvents;
extern uint32_t guiTimeStampOfCurrentlyExecutingEvent;
extern BOOLEAN gfPreventDeletionOfAnyEvent;

BOOLEAN DelayEventIfBattleInProgress(STRATEGICEVENT *pEvent) {
  STRATEGICEVENT *pNewEvent;
  if (gTacticalStatus.fEnemyInSector) {
    pNewEvent = AddAdvancedStrategicEvent(pEvent->ubEventType, pEvent->ubCallbackID,
                                          pEvent->uiTimeStamp + 180 + Random(121), pEvent->uiParam);
    Assert(pNewEvent);
    pNewEvent->uiTimeOffset = pEvent->uiTimeOffset;
    return TRUE;
  }
  return FALSE;
}

BOOLEAN ExecuteStrategicEvent(STRATEGICEVENT *pEvent) {
  BOOLEAN fOrigPreventFlag;

  fOrigPreventFlag = gfPreventDeletionOfAnyEvent;
  gfPreventDeletionOfAnyEvent = TRUE;
  // No events can be posted before this time when gfProcessingGameEvents is set, otherwise,
  // we have a chance of running into an infinite loop.
  guiTimeStampOfCurrentlyExecutingEvent = pEvent->uiTimeStamp;

  if (pEvent->ubFlags & SEF_DELETION_PENDING) {
    gfPreventDeletionOfAnyEvent = fOrigPreventFlag;
    return FALSE;
  }

#ifdef JA2BETAVERSION
  // If we are currently in the AIViewer, only process the events that we want to process.
  // The rest of the events will be delayed until AFTER we leave the viewer.
  if (guiCurrentScreen == AIVIEWER_SCREEN) {
    if (pEvent->ubCallbackID != EVENT_BEGIN_CREATURE_QUEST &&
        pEvent->ubCallbackID != EVENT_CREATURE_SPREAD &&
        pEvent->ubCallbackID != EVENT_DELAYED_HIRING_OF_MERC &&
        pEvent->ubCallbackID != EVENT_GROUP_ARRIVAL &&
        pEvent->ubCallbackID != EVENT_EVALUATE_QUEEN_SITUATION &&
        pEvent->ubCallbackID != EVENT_CHECK_ENEMY_CONTROLLED_SECTOR) {
      gfPreventDeletionOfAnyEvent = fOrigPreventFlag;
      return FALSE;
    }
  }
#endif
#ifdef JA2BETAVERSION
  if (gfMercsNeverQuit) {
    if (pEvent->ubCallbackID == EVENT_MERC_ABOUT_TO_LEAVE_COMMENT ||
        pEvent->ubCallbackID == EVENT_MERC_CONTRACT_OVER) {
      gfPreventDeletionOfAnyEvent = fOrigPreventFlag;
      return FALSE;
    }
  }
#endif
  // Look at the ID of event and do stuff according to that!
  switch (pEvent->ubCallbackID) {
    case EVENT_CHANGELIGHTVAL:
      // Change light to value
      gubEnvLightValue = (uint8_t)pEvent->uiParam;
      if (!gfBasement && !gfCaves) gfDoLighting = TRUE;
      break;
    case EVENT_CHECKFORQUESTS:
      CheckForQuests(GetWorldDay());
      break;
    case EVENT_AMBIENT:
      if (pEvent->ubEventType == ENDRANGED_EVENT) {
        if (pEvent->uiParam != NO_SAMPLE) {
          SoundRemoveSampleFlags(pEvent->uiParam, SAMPLE_RANDOM);
        }
      } else {
        pEvent->uiParam = SetupNewAmbientSound(pEvent->uiParam);
      }
      break;
    case EVENT_AIM_RESET_MERC_ANNOYANCE:
      ResetMercAnnoyanceAtPlayer((uint8_t)pEvent->uiParam);
      break;
    // The players purchase from Bobby Ray has arrived
    case EVENT_BOBBYRAY_PURCHASE:
      BobbyRayPurchaseEventCallback((uint8_t)pEvent->uiParam);
      break;
    // Gets called once a day ( at BOBBYRAY_UPDATE_TIME).  To simulate the items being bought and
    // sold at bobby rays
    case EVENT_DAILY_UPDATE_BOBBY_RAY_INVENTORY:
      DailyUpdateOfBobbyRaysNewInventory();
      DailyUpdateOfBobbyRaysUsedInventory();
      DailyUpdateOfArmsDealersInventory();
      break;
    // Add items to BobbyR's new/used inventory
    case EVENT_UPDATE_BOBBY_RAY_INVENTORY:
      AddFreshBobbyRayInventory((uint16_t)pEvent->uiParam);
      break;
    // Called once a day to update the number of days that a hired merc from M.E.R.C. has been on
    // contract.
    // Also if the player hasn't paid for a while Specks will start sending e-mails to the player
    case EVENT_DAILY_UPDATE_OF_MERC_SITE:
      DailyUpdateOfMercSite((uint16_t)GetWorldDay());
      break;
    case EVENT_DAY3_ADD_EMAIL_FROM_SPECK:
      AddEmail(MERC_INTRO, MERC_INTRO_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());
      break;
    case EVENT_DAY2_ADD_EMAIL_FROM_IMP:
      AddEmail(IMP_EMAIL_PROFILE_RESULTS, IMP_EMAIL_PROFILE_RESULTS_LENGTH, IMP_PROFILE_RESULTS,
               GetWorldTotalMin());
      break;
    // If a merc gets hired and they dont show up immediately, the merc gets added to the queue and
    // shows up
    // uiTimeTillMercArrives  minutes later
    case EVENT_DELAYED_HIRING_OF_MERC:
      MercArrivesCallback((uint8_t)pEvent->uiParam);
      break;
    // handles the life insurance contract for a merc from AIM.
    case EVENT_HANDLE_INSURED_MERCS:
      DailyUpdateOfInsuredMercs();
      break;
    // handles when a merc is killed an there is a life insurance payout
    case EVENT_PAY_LIFE_INSURANCE_FOR_DEAD_MERC:
      InsuranceContractPayLifeInsuranceForDeadMerc((uint8_t)pEvent->uiParam);
      break;
    // gets called every day at midnight.
    case EVENT_MERC_DAILY_UPDATE:
      MercDailyUpdate();
      break;
    // gets when a merc is about to leave.
    case EVENT_MERC_ABOUT_TO_LEAVE_COMMENT:
      break;
    // show the update menu
    case (EVENT_SHOW_UPDATE_MENU):
      AddDisplayBoxToWaitingQueue();
      break;
    case EVENT_MERC_ABOUT_TO_LEAVE:
      FindOutIfAnyMercAboutToLeaveIsGonnaRenew();
      break;
    // When a merc is supposed to leave
    case EVENT_MERC_CONTRACT_OVER:
      MercsContractIsFinished((uint8_t)pEvent->uiParam);
      break;
    case EVENT_ADDSOLDIER_TO_UPDATE_BOX:
      // if the grunt is currently active, add to update box
      if (Menptr[pEvent->uiParam].bActive) {
        AddSoldierToWaitingListQueue(&(Menptr[pEvent->uiParam]));
      }
      break;
    case EVENT_SET_MENU_REASON:
      AddReasonToWaitingListQueue((uint8_t)pEvent->uiParam);
      break;
    // Whenever any group (player or enemy) arrives in a new sector during movement.
    case EVENT_GROUP_ARRIVAL:
      // ValidateGameEvents();
      GroupArrivedAtSector((uint8_t)pEvent->uiParam, TRUE, FALSE);
      // ValidateGameEvents();
      break;
    case EVENT_MERC_COMPLAIN_EQUIPMENT:
      MercComplainAboutEquipment((uint8_t)pEvent->uiParam);
      break;
    case EVENT_HOURLY_UPDATE:
      HandleHourlyUpdate();
      break;
    case EVENT_MINUTE_UPDATE:
      HandleMinuteUpdate();
      break;
    case EVENT_HANDLE_MINE_INCOME:
      HandleIncomeFromMines();
      // ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, L"Income From Mines at %d", GetWorldTotalMin(
      // ) );
      break;
    case EVENT_SETUP_MINE_INCOME:
      PostEventsForMineProduction();
      break;
    case EVENT_SETUP_TOWN_OPINION:
      PostEventsForSpreadOfTownOpinion();
      break;
    case EVENT_HANDLE_TOWN_OPINION:
      HandleSpreadOfAllTownsOpinion();
      break;
    case EVENT_SET_BY_NPC_SYSTEM:
      HandleNPCSystemEvent(pEvent->uiParam);
      break;
    case EVENT_SECOND_AIRPORT_ATTENDANT_ARRIVED:
      AddSecondAirportAttendant();
      break;
    case EVENT_HELICOPTER_HOVER_TOO_LONG:
      HandleHeliHoverLong();
      break;
    case EVENT_HELICOPTER_HOVER_WAY_TOO_LONG:
      HandleHeliHoverTooLong();
      break;
    case EVENT_MERC_LEAVE_EQUIP_IN_DRASSEN:
      HandleEquipmentLeftInDrassen(pEvent->uiParam);
      break;
    case EVENT_MERC_LEAVE_EQUIP_IN_OMERTA:
      HandleEquipmentLeftInOmerta(pEvent->uiParam);
      break;
    case EVENT_BANDAGE_BLEEDING_MERCS:
      BandageBleedingDyingPatientsBeingTreated();
      break;
    case EVENT_DAILY_EARLY_MORNING_EVENTS:
      HandleEarlyMorningEvents();
      break;
    case EVENT_GROUP_ABOUT_TO_ARRIVE:
      HandleGroupAboutToArrive();
      break;
    case EVENT_PROCESS_TACTICAL_SCHEDULE:
      ProcessTacticalSchedule((uint8_t)pEvent->uiParam);
      break;
    case EVENT_BEGINRAINSTORM:
      // EnvBeginRainStorm( (uint8_t)pEvent->uiParam );
      break;
    case EVENT_ENDRAINSTORM:
      // EnvEndRainStorm( );
      break;
    case EVENT_RAINSTORM:

      // ATE: Disabled
      //
      // if( pEvent->ubEventType == ENDRANGED_EVENT )
      //{
      //	EnvEndRainStorm( );
      //}
      // else
      //{
      //	EnvBeginRainStorm( (uint8_t)pEvent->uiParam );
      //}
      break;

    case EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE:
      MakeCivGroupHostileOnNextSectorEntrance((uint8_t)pEvent->uiParam);
      break;
    case EVENT_BEGIN_AIR_RAID:
      BeginAirRaid();
      break;
    case EVENT_MEANWHILE:
      if (!DelayEventIfBattleInProgress(pEvent)) {
        BeginMeanwhile((uint8_t)pEvent->uiParam);
        InterruptTime();
      }
      break;
    case EVENT_BEGIN_CREATURE_QUEST:
      break;
    case EVENT_CREATURE_SPREAD:
      SpreadCreatures();
      break;
    case EVENT_DECAY_CREATURES:
      DecayCreatures();
      break;
    case EVENT_CREATURE_NIGHT_PLANNING:
      CreatureNightPlanning();
      break;
    case EVENT_CREATURE_ATTACK:
      CreatureAttackTown((uint8_t)pEvent->uiParam, FALSE);
      break;
    case EVENT_EVALUATE_QUEEN_SITUATION:
      EvaluateQueenSituation();
      break;
    case EVENT_CHECK_ENEMY_CONTROLLED_SECTOR:
      CheckEnemyControlledSector((uint8_t)pEvent->uiParam);
      break;
    case EVENT_TURN_ON_NIGHT_LIGHTS:
      TurnOnNightLights();
      break;
    case EVENT_TURN_OFF_NIGHT_LIGHTS:
      TurnOffNightLights();
      break;
    case EVENT_TURN_ON_PRIME_LIGHTS:
      TurnOnPrimeLights();
      break;
    case EVENT_TURN_OFF_PRIME_LIGHTS:
      TurnOffPrimeLights();
      break;
    case EVENT_INTERRUPT_TIME:
      InterruptTime();
      break;
    case EVENT_ENRICO_MAIL:
      HandleEnricoEmail();
      break;
    case EVENT_INSURANCE_INVESTIGATION_STARTED:
      StartInsuranceInvestigation((uint8_t)pEvent->uiParam);
      break;
    case EVENT_INSURANCE_INVESTIGATION_OVER:
      EndInsuranceInvestigation((uint8_t)pEvent->uiParam);
      break;
    case EVENT_TEMPERATURE_UPDATE:
      UpdateTemperature((uint8_t)pEvent->uiParam);
      break;
    case EVENT_KEITH_GOING_OUT_OF_BUSINESS:
      // make sure killbillies are still alive, if so, set fact 274 true
      if (CheckFact(FACT_HILLBILLIES_KILLED, KEITH) == FALSE) {
        // s et the fact true keith is out of business
        SetFactTrue(FACT_KEITH_OUT_OF_BUSINESS);
      }
      break;
    case EVENT_MERC_SITE_BACK_ONLINE:
      GetMercSiteBackOnline();
      break;
    case EVENT_INVESTIGATE_SECTOR:
      InvestigateSector((uint8_t)pEvent->uiParam);
      break;
    case EVENT_CHECK_IF_MINE_CLEARED:
      // If so, the head miner will say so, and the mine's shutdown will be ended.
      HourlyMinesUpdate();  // not-so hourly, in this case!
      break;
    case EVENT_REMOVE_ASSASSIN:
      RemoveAssassin((uint8_t)pEvent->uiParam);
      break;
    case EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE:
      BeginContractRenewalSequence();
      break;
    case EVENT_RPC_WHINE_ABOUT_PAY:
      RPCWhineAboutNoPay((uint8_t)pEvent->uiParam);
      break;

    case EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL:
      HaventMadeImpMercEmailCallBack();
      break;

    case EVENT_QUARTER_HOUR_UPDATE:
      HandleQuarterHourUpdate();
      break;

    case EVENT_MERC_MERC_WENT_UP_LEVEL_EMAIL_DELAY:
      MERCMercWentUpALevelSendEmail((uint8_t)pEvent->uiParam);
      break;

    case EVENT_MERC_SITE_NEW_MERC_AVAILABLE:
      NewMercsAvailableAtMercSiteCallBack();
      break;
  }
  gfPreventDeletionOfAnyEvent = fOrigPreventFlag;
  return TRUE;
}
