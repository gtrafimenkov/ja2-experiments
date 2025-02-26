// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Types.h"
#include "Soldier.h"
#include "Tactical/Items.h"
#include "Tactical/Menptr.h"
#include "Tactical/OppList.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/AIInternals.h"

#define CAN_CALL(s) \
  (s->ubBodyType != BLOODCAT && s->ubBodyType != LARVAE_MONSTER && s->ubBodyType != INFANT_MONSTER)
#define CAN_LISTEN_TO_CALL(s) (s->ubBodyType != BLOODCAT && s->ubBodyType != LARVAE_MONSTER)

enum {
  CALLER_FEMALE = 0,
  CALLER_MALE,
  CALLER_INFANT,
  CALLER_QUEEN,
  NUM_CREATURE_CALLERS
} CreatureCaller;

enum { CREATURE_MOBILE = 0, CREATURE_CRAWLER, CREATURE_IMMOBILE } CreatureMobility;

#define FRENZY_THRESHOLD 8
#define MAX_EAT_DIST 5

int8_t gbCallPriority[NUM_CREATURE_CALLS][NUM_CREATURE_CALLERS] = {
    {0, 0, 0},   // CALL_NONE
    {3, 5, 12},  // CALL_1_PREY
    {5, 9, 12},  // CALL_MULTIPLE_PREY
    {4, 7, 12},  // CALL_ATTACKED
    {6, 9, 12},  // CALL_CRIPPLED
};

int8_t gbHuntCallPriority[NUM_CREATURE_CALLS] = {
    4,  // CALL_1_PREY
    5,  // CALL_MULTIPLE_PREY
    7,  // CALL_ATTACKED
    8   // CALL_CRIPPLED
};

#define PRIORITY_DECR_DISTANCE 30

#define CALL_1_OPPONENT CALL_1_PREY
#define CALL_MULTIPLE_OPPONENT CALL_MULTIPLE_PREY

void CreatureCall(struct SOLDIERTYPE *pCaller) {
  uint8_t ubCallerType = 0;
  uint8_t ubReceiver;
  int8_t bFullPriority;
  int8_t bPriority;
  struct SOLDIERTYPE *pReceiver;
  uint16_t usDistToCaller;
  // communicate call to all creatures on map through ultrasonics

  gTacticalStatus.Team[pCaller->bTeam].bAwareOfOpposition = TRUE;
  // bAction should be AI_ACTION_CREATURE_CALL (new)
  // usActionData is call enum #
  switch (pCaller->ubBodyType) {
    case ADULTFEMALEMONSTER:
    case YAF_MONSTER:
      ubCallerType = CALLER_FEMALE;
      break;
    case QUEENMONSTER:
      ubCallerType = CALLER_QUEEN;
      break;
    // need to add male
    case AM_MONSTER:
    case YAM_MONSTER:
      ubCallerType = CALLER_MALE;
      break;
    default:
      ubCallerType = CALLER_FEMALE;
      break;
  }
  if (pCaller->bHunting)  // which should only be set for females outside of the hive
  {
    bFullPriority = gbHuntCallPriority[pCaller->usActionData];
  } else {
    bFullPriority = gbCallPriority[pCaller->usActionData][ubCallerType];
  }

  // OK, do animation based on body type...
  switch (pCaller->ubBodyType) {
    case ADULTFEMALEMONSTER:
    case YAF_MONSTER:
    case AM_MONSTER:
    case YAM_MONSTER:

      EVENT_InitNewSoldierAnim(pCaller, MONSTER_UP, 0, FALSE);
      break;

    case QUEENMONSTER:

      EVENT_InitNewSoldierAnim(pCaller, QUEEN_CALL, 0, FALSE);
      break;
  }

  for (ubReceiver = gTacticalStatus.Team[pCaller->bTeam].bFirstID;
       ubReceiver <= gTacticalStatus.Team[pCaller->bTeam].bLastID; ubReceiver++) {
    pReceiver = MercPtrs[ubReceiver];
    if (pReceiver->bActive && pReceiver->bInSector && (pReceiver->bLife >= OKLIFE) &&
        (pReceiver != pCaller) && (pReceiver->bAlertStatus < STATUS_BLACK)) {
      if (pReceiver->ubBodyType != LARVAE_MONSTER && pReceiver->ubBodyType != INFANT_MONSTER &&
          pReceiver->ubBodyType != QUEENMONSTER) {
        usDistToCaller = PythSpacesAway(pReceiver->sGridNo, pCaller->sGridNo);
        bPriority = bFullPriority - (int8_t)(usDistToCaller / PRIORITY_DECR_DISTANCE);
        if (bPriority > pReceiver->bCallPriority) {
          pReceiver->bCallPriority = bPriority;
          pReceiver->bAlertStatus = STATUS_RED;  // our status can't be more than red to begin with
          pReceiver->ubCaller = pCaller->ubID;
          pReceiver->sCallerGridNo = pCaller->sGridNo;
          pReceiver->bCallActedUpon = FALSE;
          CancelAIAction(pReceiver, FORCE);
          if ((bPriority > FRENZY_THRESHOLD) && (pReceiver->ubBodyType == ADULTFEMALEMONSTER ||
                                                 pReceiver->ubBodyType == YAF_MONSTER)) {
            // go berzerk!
            pReceiver->bFrenzied = TRUE;
          }
        }
      }
    }
  }
}

int8_t CreatureDecideActionGreen(struct SOLDIERTYPE *pSoldier) {
  int32_t iChance, iSneaky = 10;
  // int8_t		bInWater;
  int8_t bInGas;

  // bInWater = MercInWater(pSoldier);

  // NB creatures would ignore smoke completely :-)

  if (pSoldier->bMobility == CREATURE_CRAWLER &&
      pSoldier->bActionPoints < pSoldier->bInitialActionPoints) {
    return (AI_ACTION_NONE);
  }

  bInGas = InGas(pSoldier, pSoldier->sGridNo);

  if (pSoldier->bMobility == CREATURE_MOBILE) {
    if (TrackScent(pSoldier)) {
      return (AI_ACTION_TRACK);
    }

    ////////////////////////////////////////////////////////////////////////////
    // POINT PATROL: move towards next point unless getting a bit winded
    ////////////////////////////////////////////////////////////////////////////

    // this takes priority over water/gas checks, so that point patrol WILL work
    // from island to island, and through gas covered areas, too
    if ((pSoldier->bOrders == POINTPATROL) && (pSoldier->bBreath >= 50)) {
      if (PointPatrolAI(pSoldier)) {
        if (!gfTurnBasedAI) {
          // pause at the end of the walk!
          pSoldier->bNextAction = AI_ACTION_WAIT;
          pSoldier->usNextActionData = (uint16_t)REALTIME_CREATURE_AI_DELAY;
        }

        return (AI_ACTION_POINT_PATROL);
      }
    }

    if ((pSoldier->bOrders == RNDPTPATROL) && (pSoldier->bBreath >= 50)) {
      if (RandomPointPatrolAI(pSoldier)) {
        if (!gfTurnBasedAI) {
          // pause at the end of the walk!
          pSoldier->bNextAction = AI_ACTION_WAIT;
          pSoldier->usNextActionData = (uint16_t)REALTIME_CREATURE_AI_DELAY;
        }

        return (AI_ACTION_POINT_PATROL);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // WHEN LEFT IN WATER OR GAS, GO TO NEAREST REACHABLE SPOT OF UNGASSED LAND
    ////////////////////////////////////////////////////////////////////////////

    if (/*bInWater || */ bInGas) {
      pSoldier->usActionData = FindNearestUngassedLand(pSoldier);

      if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
        sprintf(tempstr, "%s - SEEKING NEAREST UNGASSED LAND at grid %d", pSoldier->name,
                pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif

        return (AI_ACTION_LEAVE_WATER_GAS);
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // REST IF RUNNING OUT OF BREATH
  ////////////////////////////////////////////////////////////////////////

  // if our breath is running a bit low, and we're not in the way or in water
  if ((pSoldier->bBreath < 75) /*&& !bInWater*/) {
    // take a breather for gods sake!
    pSoldier->usActionData = NOWHERE;
    return (AI_ACTION_NONE);
  }

  ////////////////////////////////////////////////////////////////////////////
  // RANDOM PATROL:  determine % chance to start a new patrol route
  ////////////////////////////////////////////////////////////////////////////

  if (pSoldier->bMobility != CREATURE_IMMOBILE) {
    iChance = 25;

    // set base chance according to orders
    switch (pSoldier->bOrders) {
      case STATIONARY:
        iChance += -20;
        break;
      case ONGUARD:
        iChance += -15;
        break;
      case ONCALL:
        break;
      case CLOSEPATROL:
        iChance += +15;
        break;
      case RNDPTPATROL:
      case POINTPATROL:
        iChance = 0;
        break;
      case FARPATROL:
        iChance += +25;
        break;
      case SEEKENEMY:
        iChance += -10;
        break;
    }

    // modify chance of patrol (and whether it's a sneaky one) by attitude
    switch (pSoldier->bAttitude) {
      case DEFENSIVE:
        iChance += -10;
        break;
      case BRAVESOLO:
        iChance += 5;
        break;
      case BRAVEAID:
        break;
      case CUNNINGSOLO:
        iChance += 5;
        iSneaky += 10;
        break;
      case CUNNINGAID:
        iSneaky += 5;
        break;
      case AGGRESSIVE:
        iChance += 10;
        iSneaky += -5;
        break;
    }

    // reduce chance for any injury, less likely to wander around when hurt
    iChance -= (pSoldier->bLifeMax - pSoldier->bLife);

    // reduce chance if breath is down, less likely to wander around when tired
    iChance -= (100 - pSoldier->bBreath);

    // if we're in water with land miles (> 25 tiles) away,
    // OR if we roll under the chance calculated
    if (/*bInWater ||*/ ((int16_t)PreRandom(100) < iChance)) {
      pSoldier->usActionData = RandDestWithinRange(pSoldier);

      if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
        sprintf(tempstr, "%s - RANDOM PATROL to grid %d", pSoldier->name, pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif

        if (!gfTurnBasedAI) {
          // pause at the end of the walk!
          pSoldier->bNextAction = AI_ACTION_WAIT;
          pSoldier->usNextActionData = (uint16_t)REALTIME_CREATURE_AI_DELAY;
          if (pSoldier->bMobility == CREATURE_CRAWLER) {
            pSoldier->usNextActionData *= 2;
          }
        }

        return (AI_ACTION_RANDOM_PATROL);
      }
    }

    /*
    if (pSoldier->bMobility == CREATURE_MOBILE)
    {
            ////////////////////////////////////////////////////////////////////////////
            // SEEK FRIEND: determine %chance for man to pay a friendly visit
            ////////////////////////////////////////////////////////////////////////////
            iChance = 25;

            // set base chance and maximum seeking distance according to orders
            switch (pSoldier->bOrders)
            {
                    case STATIONARY:     iChance += -20; break;
                    case ONGUARD:        iChance += -15; break;
                    case ONCALL:                         break;
                    case CLOSEPATROL:    iChance += +10; break;
                    case RNDPTPATROL:
                    case POINTPATROL:    iChance  = -10; break;
                    case FARPATROL:      iChance += +20; break;
                    case SEEKENEMY:      iChance += -10; break;
            }

            // modify for attitude
            switch (pSoldier->bAttitude)
            {
                    case DEFENSIVE:                       break;
                    case BRAVESOLO:      iChance /= 2;    break;  // loners
                    case BRAVEAID:       iChance += 10;   break;  // friendly
                    case CUNNINGSOLO:    iChance /= 2;    break;  // loners
                    case CUNNINGAID:     iChance += 10;   break;  // friendly
                    case AGGRESSIVE:                      break;
            }

            // reduce chance for any injury, less likely to wander around when hurt
            iChance -= (pSoldier->bLifeMax - pSoldier->bLife);

            // reduce chance if breath is down
            iChance -= (100 - pSoldier->bBreath);         // very likely to wait when exhausted

            if ((int16_t) PreRandom(100) < iChance)
            {
                    if (RandomFriendWithin(pSoldier))
                    {
    #ifdef DEBUGDECISIONS
                     sprintf(tempstr,"%s - SEEK FRIEND at grid
    %d",pSoldier->name,pSoldier->usActionData); AIPopMessage(tempstr); #endif

                            if (!gfTurnBasedAI)
                            {
                                    // pause at the end of the walk!
                                    pSoldier->bNextAction = AI_ACTION_WAIT;
                                    pSoldier->usNextActionData = (uint16_t)
    REALTIME_CREATURE_AI_DELAY;
                            }

                            return(AI_ACTION_SEEK_FRIEND);
                    }
            }
    }
    */

    ////////////////////////////////////////////////////////////////////////////
    // LOOK AROUND: determine %chance for man to turn in place
    ////////////////////////////////////////////////////////////////////////////

    // avoid 2 consecutive random turns in a row
    if (pSoldier->bLastAction != AI_ACTION_CHANGE_FACING &&
        (GetAPsToLook(pSoldier) <= pSoldier->bActionPoints)) {
      iChance = 25;

      // set base chance according to orders
      if (pSoldier->bOrders == STATIONARY) iChance += 25;

      if (pSoldier->bOrders == ONGUARD) iChance += 20;

      if (pSoldier->bAttitude == DEFENSIVE) iChance += 25;

      if ((int16_t)PreRandom(100) < iChance) {
        // roll random directions (stored in actionData) until different from current
        do {
          // if man has a LEGAL dominant facing, and isn't facing it, he will turn
          // back towards that facing 50% of the time here (normally just enemies)
          if ((pSoldier->bDominantDir >= 0) && (pSoldier->bDominantDir <= 8) &&
              (pSoldier->bDirection != pSoldier->bDominantDir) && PreRandom(2)) {
            pSoldier->usActionData = pSoldier->bDominantDir;
          } else {
            pSoldier->usActionData = (uint16_t)PreRandom(8);
          }
        } while (pSoldier->usActionData == pSoldier->bDirection);

#ifdef DEBUGDECISIONS
        sprintf(tempstr, "%s - TURNS to face direction %d", pSoldier->name, pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif

        if (ValidCreatureTurn(pSoldier, (int8_t)pSoldier->usActionData))

        // InternalIsValidStance( pSoldier, (int8_t) pSoldier->usActionData, ANIM_STAND ) )
        {
          if (!gfTurnBasedAI) {
            // pause at the end of the turn!
            pSoldier->bNextAction = AI_ACTION_WAIT;
            pSoldier->usNextActionData = (uint16_t)REALTIME_CREATURE_AI_DELAY;
          }

          return (AI_ACTION_CHANGE_FACING);
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // NONE:
  ////////////////////////////////////////////////////////////////////////////

  // by default, if everything else fails, just stands in place without turning
  pSoldier->usActionData = NOWHERE;

  return (AI_ACTION_NONE);
}

int8_t CreatureDecideActionYellow(struct SOLDIERTYPE *pSoldier) {
  // monster AI - heard something
  uint8_t ubNoiseDir;
  int16_t sNoiseGridNo;
  int32_t iNoiseValue;
  int32_t iChance, iSneaky;
  BOOLEAN fClimb;
  BOOLEAN fReachable;
  //	int16_t sClosestFriend;

  if (pSoldier->bMobility == CREATURE_CRAWLER &&
      pSoldier->bActionPoints < pSoldier->bInitialActionPoints) {
    return (AI_ACTION_NONE);
  }

  // determine the most important noise heard, and its relative value
  sNoiseGridNo = MostImportantNoiseHeard(pSoldier, &iNoiseValue, &fClimb, &fReachable);
  // NumMessage("iNoiseValue = ",iNoiseValue);

  if (sNoiseGridNo == NOWHERE) {
    // then we have no business being under YELLOW status any more!
#ifdef RECORDNET
    fprintf(NetDebugFile, "\nDecideActionYellow: ERROR - No important noise known by guynum %d\n\n",
            GetSolID(pSoldier));
#endif

#ifdef BETAVERSION
    NumMessage("DecideActionYellow: ERROR - No important noise known by guynum ",
               GetSolID(pSoldier));
#endif

    return (AI_ACTION_NONE);
  }

  ////////////////////////////////////////////////////////////////////////////
  // LOOK AROUND TOWARD NOISE: determine %chance for man to turn towards noise
  ////////////////////////////////////////////////////////////////////////////

  if (pSoldier->bMobility != CREATURE_IMMOBILE) {
    // determine direction from this soldier in which the noise lies
    ubNoiseDir = atan8(CenterX(pSoldier->sGridNo), CenterY(pSoldier->sGridNo),
                       CenterX(sNoiseGridNo), CenterY(sNoiseGridNo));

    // if soldier is not already facing in that direction,
    // and the noise source is close enough that it could possibly be seen
    if ((GetAPsToLook(pSoldier) <= pSoldier->bActionPoints) &&
        (pSoldier->bDirection != ubNoiseDir) &&
        PythSpacesAway(pSoldier->sGridNo, sNoiseGridNo) <= STRAIGHT) {
      // set base chance according to orders
      if ((pSoldier->bOrders == STATIONARY) || (pSoldier->bOrders == ONGUARD))
        iChance = 60;
      else  // all other orders
        iChance = 35;

      if (pSoldier->bAttitude == DEFENSIVE) iChance += 15;

      if ((int16_t)PreRandom(100) < iChance) {
        pSoldier->usActionData = ubNoiseDir;
#ifdef DEBUGDECISIONS
        sprintf(tempstr, "%s - TURNS TOWARDS NOISE to face direction %d", pSoldier->name,
                pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif
        // if ( InternalIsValidStance( pSoldier, (int8_t) pSoldier->usActionData, ANIM_STAND ) )
        if (ValidCreatureTurn(pSoldier, (int8_t)pSoldier->usActionData)) {
          return (AI_ACTION_CHANGE_FACING);
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // REST IF RUNNING OUT OF BREATH
  ////////////////////////////////////////////////////////////////////////

  // if our breath is running a bit low, and we're not in water
  if ((pSoldier->bBreath < 25) /*&& !MercInWater(pSoldier) */) {
    // take a breather for gods sake!
    pSoldier->usActionData = NOWHERE;
    return (AI_ACTION_NONE);
  }

  if (pSoldier->bMobility != CREATURE_IMMOBILE && fReachable) {
    ////////////////////////////////////////////////////////////////////////////
    // SEEK NOISE
    ////////////////////////////////////////////////////////////////////////////

    // remember that noise value is negative, and closer to 0 => more important!
    iChance = 75 + iNoiseValue;
    iSneaky = 30;

    // set base chance according to orders
    switch (pSoldier->bOrders) {
      case STATIONARY:
        iChance += -20;
        break;
      case ONGUARD:
        iChance += -15;
        break;
      case ONCALL:
        break;
      case CLOSEPATROL:
        iChance += -10;
        break;
      case RNDPTPATROL:
      case POINTPATROL:
        break;
      case FARPATROL:
        iChance += 10;
        break;
      case SEEKENEMY:
        iChance += 25;
        break;
    }

    // modify chance of patrol (and whether it's a sneaky one) by attitude
    switch (pSoldier->bAttitude) {
      case DEFENSIVE:
        iChance += -10;
        iSneaky += 15;
        break;
      case BRAVESOLO:
        iChance += 10;
        break;
      case BRAVEAID:
        iChance += 5;
        break;
      case CUNNINGSOLO:
        iChance += 5;
        iSneaky += 30;
        break;
      case CUNNINGAID:
        iSneaky += 30;
        break;
      case AGGRESSIVE:
        iChance += 20;
        iSneaky += -10;
        break;
    }

    // reduce chance if breath is down, less likely to wander around when tired
    iChance -= (100 - pSoldier->bBreath);

    if ((int16_t)PreRandom(100) < iChance) {
      pSoldier->usActionData =
          GoAsFarAsPossibleTowards(pSoldier, sNoiseGridNo, AI_ACTION_SEEK_NOISE);

      if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
        sprintf(tempstr, "%s - INVESTIGATING NOISE at grid %d, moving to %d", pSoldier->name,
                sNoiseGridNo, pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif

        return (AI_ACTION_SEEK_NOISE);
      }
    }
    // Okay, we're not following up on the noise... but let's follow any
    // scent trails available
    if (TrackScent(pSoldier)) {
      return (AI_ACTION_TRACK);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // DO NOTHING: Not enough points left to move, so save them for next turn
  ////////////////////////////////////////////////////////////////////////////

#ifdef DEBUGDECISIONS
  AINameMessage(pSoldier, "- DOES NOTHING (YELLOW)", 1000);
#endif

  // by default, if everything else fails, just stands in place without turning
  pSoldier->usActionData = NOWHERE;
  return (AI_ACTION_NONE);
}

int8_t CreatureDecideActionRed(struct SOLDIERTYPE *pSoldier, uint8_t ubUnconsciousOK) {
  // monster AI - hostile mammals somewhere around!
  int16_t iChance, sClosestOpponent /*,sClosestOpponent,sClosestFriend*/;
  int16_t sClosestDisturbance;
  int16_t sDistVisible;
  uint8_t ubCanMove, ubOpponentDir;
  // int8_t bInWater;
  int8_t bInGas;
  int16_t sAdjustedGridNo;
  BOOLEAN fChangeLevel;

  // if we have absolutely no action points, we can't do a thing under RED!
  if (!pSoldier->bActionPoints) {
    pSoldier->usActionData = NOWHERE;
    return (AI_ACTION_NONE);
  }

  if (pSoldier->bMobility == CREATURE_CRAWLER &&
      pSoldier->bActionPoints < pSoldier->bInitialActionPoints) {
    return (AI_ACTION_NONE);
  }

  // can this guy move to any of the neighbouring squares ? (sets TRUE/FALSE)
  ubCanMove = ((pSoldier->bMobility != CREATURE_IMMOBILE) &&
               (pSoldier->bActionPoints >= MinPtsToMove(pSoldier)));

  // determine if we happen to be in water (in which case we're in BIG trouble!)
  // bInWater = MercInWater(pSoldier);

  // check if standing in tear gas without a gas mask on
  bInGas = InGas(pSoldier, pSoldier->sGridNo);

  ////////////////////////////////////////////////////////////////////////////
  // WHEN IN GAS, GO TO NEAREST REACHABLE SPOT OF UNGASSED LAND
  ////////////////////////////////////////////////////////////////////////////

  if (bInGas && ubCanMove) {
    pSoldier->usActionData = FindNearestUngassedLand(pSoldier);

    if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
      sprintf(tempstr, "%s - SEEKING NEAREST UNGASSED LAND at grid %d", pSoldier->name,
              pSoldier->usActionData);
      AIPopMessage(tempstr);
#endif

      return (AI_ACTION_LEAVE_WATER_GAS);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // CALL FOR AID IF HURT
  ////////////////////////////////////////////////////////////////////////////
  if (CAN_CALL(pSoldier)) {
    if ((pSoldier->bActionPoints >= AP_RADIO) &&
        (gTacticalStatus.Team[pSoldier->bTeam].bMenInSector > 1)) {
      if (pSoldier->bLife < pSoldier->bOldLife) {
        // got injured, maybe call
        if ((pSoldier->bOldLife == pSoldier->bLifeMax) &&
            (pSoldier->bOldLife - pSoldier->bLife > 10)) {
          // hurt for first time!
          pSoldier->usActionData = CALL_CRIPPLED;
          pSoldier->bOldLife = pSoldier->bLife;  // don't want to call more than once
          return (AI_ACTION_CREATURE_CALL);
        } else if (pSoldier->bLifeMax / pSoldier->bLife > 2) {
          // crippled, 1/3 or less health!
          pSoldier->usActionData = CALL_ATTACKED;
          pSoldier->bOldLife = pSoldier->bLife;  // don't want to call more than once
          return (AI_ACTION_CREATURE_CALL);
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // CROUCH & REST IF RUNNING OUT OF BREATH
  ////////////////////////////////////////////////////////////////////////

  // if our breath is running a bit low, and we're not in water or under fire
  if ((pSoldier->bBreath < 25) /*&& !bInWater*/ && !pSoldier->bUnderFire) {
#ifdef DEBUGDECISIONS
    sprintf(tempstr, "%s RESTS (STATUS RED), breath = %d", pSoldier->name, pSoldier->bBreath);
    AIPopMessage(tempstr);
#endif

    pSoldier->usActionData = NOWHERE;
    return (AI_ACTION_NONE);
  }

  ////////////////////////////////////////////////////////////////////////////
  // CALL IN SIGHTING: determine %chance to call others and report contact
  ////////////////////////////////////////////////////////////////////////////

  // if we're a computer merc, and we have the action points remaining to RADIO
  // (we never want NPCs to choose to radio if they would have to wait a turn)
  if (CAN_CALL(pSoldier) && (!gTacticalStatus.Team[pSoldier->bTeam].bAwareOfOpposition)) {
    if ((pSoldier->bActionPoints >= AP_RADIO) &&
        (gTacticalStatus.Team[pSoldier->bTeam].bMenInSector > 1)) {
      // if there hasn't been a general sighting call sent yet

      // might want to check the specifics of who we see
      iChance = 20;

      if (iChance) {
#ifdef DEBUGDECISIONS
        AINumMessage("Chance to call sighting = ", iChance);
#endif

        if ((int16_t)PreRandom(100) < iChance) {
#ifdef DEBUGDECISIONS
          AINameMessage(pSoldier, "decides to call an alert!", 1000);
#endif
          pSoldier->usActionData = CALL_1_PREY;
          return (AI_ACTION_CREATURE_CALL);
        }
      }
    }
  }

  if (pSoldier->bMobility != CREATURE_IMMOBILE) {
    if (FindAIUsableObjClass(pSoldier, IC_WEAPON) == ITEM_NOT_FOUND) {
      // probably a baby bug... run away! run away!
      // look for best place to RUN AWAY to (farthest from the closest threat)
      pSoldier->usActionData = FindSpotMaxDistFromOpponents(pSoldier);

      if (pSoldier->usActionData != NOWHERE) {
        return (AI_ACTION_RUN_AWAY);
      } else {
        return (AI_ACTION_NONE);
      }
    }

    // Respond to call if any
    if (CAN_LISTEN_TO_CALL(pSoldier) && pSoldier->ubCaller != NOBODY) {
      if (PythSpacesAway(pSoldier->sGridNo, pSoldier->sCallerGridNo) <= STOPSHORTDIST) {
        // call completed... hmm, nothing found
        pSoldier->ubCaller = NOBODY;
      } else {
        pSoldier->usActionData = InternalGoAsFarAsPossibleTowards(
            pSoldier, pSoldier->sCallerGridNo, -1, AI_ACTION_SEEK_FRIEND, FLAG_STOPSHORT);

        if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
          sprintf(tempstr, "%s - SEEKING FRIEND at %d, MOVING to %d", pSoldier->name,
                  sClosestFriend, pSoldier->usActionData);
          AIPopMessage(tempstr);
#endif
          return (AI_ACTION_SEEK_FRIEND);
        }
      }
    }

    // get the location of the closest reachable opponent
    sClosestDisturbance = ClosestReachableDisturbance(pSoldier, ubUnconsciousOK, &fChangeLevel);
    // if there is an opponent reachable
    if (sClosestDisturbance != NOWHERE) {
      //////////////////////////////////////////////////////////////////////
      // SEEK CLOSEST DISTURBANCE: GO DIRECTLY TOWARDS CLOSEST KNOWN OPPONENT
      //////////////////////////////////////////////////////////////////////

      // try to move towards him
      pSoldier->usActionData =
          GoAsFarAsPossibleTowards(pSoldier, sClosestDisturbance, AI_ACTION_SEEK_OPPONENT);

      // if it's possible
      if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
        // do it!
        sprintf(tempstr, "%s - SEEKING OPPONENT at grid %d, MOVING to %d", pSoldier->name,
                sClosestDisturbance, pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif

        return (AI_ACTION_SEEK_OPPONENT);
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // TAKE A BITE, PERHAPS
    ////////////////////////////////////////////////////////////////////////////
    if (pSoldier->bHunting) {
      pSoldier->usActionData = FindNearestRottingCorpse(pSoldier);
      // need smell/visibility check?
      if (PythSpacesAway(pSoldier->sGridNo, pSoldier->usActionData) < MAX_EAT_DIST) {
        int16_t sGridNo;

        sGridNo = FindAdjacentGridEx(pSoldier, pSoldier->usActionData, &ubOpponentDir,
                                     &sAdjustedGridNo, FALSE, FALSE);

        if (sGridNo != -1) {
          pSoldier->usActionData = sGridNo;
          return (AI_ACTION_APPROACH_MERC);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////////
    // TRACK A SCENT, IF ONE IS PRESENT
    ////////////////////////////////////////////////////////////////////////////
    if (TrackScent(pSoldier)) {
      return (AI_ACTION_TRACK);
    }

    ////////////////////////////////////////////////////////////////////////////
    // LOOK AROUND TOWARD CLOSEST KNOWN OPPONENT, IF KNOWN
    ////////////////////////////////////////////////////////////////////////////
    if (GetAPsToLook(pSoldier) <= pSoldier->bActionPoints) {
      // determine the location of the known closest opponent
      // (don't care if he's conscious, don't care if he's reachable at all)
      sClosestOpponent = ClosestKnownOpponent(pSoldier, NULL, NULL);

      if (sClosestOpponent != NOWHERE) {
        // determine direction from this soldier to the closest opponent
        ubOpponentDir = atan8(CenterX(pSoldier->sGridNo), CenterY(pSoldier->sGridNo),
                              CenterX(sClosestOpponent), CenterY(sClosestOpponent));

        // if soldier is not already facing in that direction,
        // and the opponent is close enough that he could possibly be seen
        // note, have to change this to use the level returned from ClosestKnownOpponent
        sDistVisible = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT,
                                       sClosestOpponent, 0);

        if ((pSoldier->bDirection != ubOpponentDir) &&
            (PythSpacesAway(pSoldier->sGridNo, sClosestOpponent) <= sDistVisible)) {
          // set base chance according to orders
          if ((pSoldier->bOrders == STATIONARY) || (pSoldier->bOrders == ONGUARD))
            iChance = 50;
          else  // all other orders
            iChance = 25;

          if (pSoldier->bAttitude == DEFENSIVE) iChance += 25;

          // if ( (int16_t)PreRandom(100) < iChance && InternalIsValidStance( pSoldier,
          // ubOpponentDir, ANIM_STAND ) )
          if ((int16_t)PreRandom(100) < iChance && ValidCreatureTurn(pSoldier, ubOpponentDir)) {
            pSoldier->usActionData = ubOpponentDir;

#ifdef DEBUGDECISIONS
            sprintf(tempstr, "%s - TURNS TOWARDS CLOSEST ENEMY to face direction %d",
                    pSoldier->name, pSoldier->usActionData);
            AIPopMessage(tempstr);
#endif

            return (AI_ACTION_CHANGE_FACING);
          }
        }
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // LEAVE THE SECTOR
  ////////////////////////////////////////////////////////////////////////////

  // NOT IMPLEMENTED

  ////////////////////////////////////////////////////////////////////////////
  // DO NOTHING: Not enough points left to move, so save them for next turn
  ////////////////////////////////////////////////////////////////////////////

#ifdef DEBUGDECISIONS
  AINameMessage(ptr, "- DOES NOTHING (RED)", 1000);
#endif

  pSoldier->usActionData = NOWHERE;

  return (AI_ACTION_NONE);
}

int8_t CreatureDecideActionBlack(struct SOLDIERTYPE *pSoldier) {
  // monster AI - hostile mammals in sense range
  int16_t sClosestOpponent;
  int16_t sClosestDisturbance;
  uint8_t ubMinAPCost, ubCanMove /*,bInWater*/, bInGas;
  int8_t bDirection;
  uint8_t ubBestAttackAction;
  int8_t bCanAttack;
  int8_t bSpitIn, bWeaponIn;
  uint32_t uiChance;
  ATTACKTYPE BestShot, BestStab, BestAttack, CurrStab;
  BOOLEAN fRunAway = FALSE;
  BOOLEAN fChangeLevel;

  // if we have absolutely no action points, we can't do a thing under BLACK!
  if (!pSoldier->bActionPoints) {
    pSoldier->usActionData = NOWHERE;
    return (AI_ACTION_NONE);
  }

  if (pSoldier->bMobility == CREATURE_CRAWLER &&
      pSoldier->bActionPoints < pSoldier->bInitialActionPoints) {
    return (AI_ACTION_NONE);
  }

  ////////////////////////////////////////////////////////////////////////////
  // CALL FOR AID IF HURT OR IF OTHERS ARE UNAWARE
  ////////////////////////////////////////////////////////////////////////////

  if (CAN_CALL(pSoldier)) {
    if ((pSoldier->bActionPoints >= AP_RADIO) &&
        (gTacticalStatus.Team[pSoldier->bTeam].bMenInSector > 1)) {
      if (pSoldier->bLife < pSoldier->bOldLife) {
        // got injured, maybe call
        /*
        // don't call when crippled and have target... save breath for attacking!
        if ((pSoldier->bOldLife == pSoldier->bLifeMax) && (pSoldier->bOldLife - pSoldier->bLife >
        10))
        {
                // hurt for first time!
                pSoldier->usActionData = CALL_CRIPPLED;
                pSoldier->bOldLife = pSoldier->bLife;  // don't want to call more than once
                return(AI_ACTION_CREATURE_CALL);
        }
        else
        */
        if (pSoldier->bLifeMax / pSoldier->bLife > 2) {
          // crippled, 1/3 or less health!
          pSoldier->usActionData = CALL_ATTACKED;
          pSoldier->bOldLife = pSoldier->bLife;  // don't want to call more than once
          return (AI_ACTION_CREATURE_CALL);
        }
      } else {
        if (!(gTacticalStatus.Team[pSoldier->bTeam].bAwareOfOpposition)) {
          if (pSoldier->ubBodyType == QUEENMONSTER) {
            uiChance = 100;
          } else {
            uiChance = 20 * pSoldier->bOppCnt;
          }
          if (Random(100) < uiChance) {
            // alert! alert!
            if (pSoldier->bOppCnt > 1) {
              pSoldier->usActionData = CALL_MULTIPLE_PREY;
            } else {
              pSoldier->usActionData = CALL_1_PREY;
            }
            return (AI_ACTION_CREATURE_CALL);
          }
        }
      }
    }
  }

  // can this guy move to any of the neighbouring squares ? (sets TRUE/FALSE)
  ubCanMove = ((pSoldier->bMobility != CREATURE_IMMOBILE) &&
               (pSoldier->bActionPoints >= MinPtsToMove(pSoldier)));

  // determine if we happen to be in water (in which case we're in BIG trouble!)
  // bInWater = MercInWater(pSoldier);

  // check if standing in tear gas without a gas mask on
  bInGas = InGas(pSoldier, pSoldier->sGridNo);

  ////////////////////////////////////////////////////////////////////////////
  // IF GASSED, OR REALLY TIRED (ON THE VERGE OF COLLAPSING), TRY TO RUN AWAY
  ////////////////////////////////////////////////////////////////////////////

  // if we're desperately short on breath (it's OK if we're in water, though!)
  if (bInGas || (pSoldier->bBreath < 5)) {
    // if soldier has enough APs left to move at least 1 square's worth
    if (ubCanMove) {
      // look for best place to RUN AWAY to (farthest from the closest threat)
      pSoldier->usActionData = FindSpotMaxDistFromOpponents(pSoldier);

      if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
        sprintf(tempstr, "%s - GASSED or LOW ON BREATH (%d), RUNNING AWAY to grid %d",
                pSoldier->name, pSoldier->bBreath, pSoldier->usActionData);
        AIPopMessage(tempstr);
#endif

        return (AI_ACTION_RUN_AWAY);
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // STUCK IN WATER OR GAS, NO COVER, GO TO NEAREST SPOT OF UNGASSED LAND
  ////////////////////////////////////////////////////////////////////////////

  // if soldier in water/gas has enough APs left to move at least 1 square
  if ((/*bInWater ||*/ bInGas) && ubCanMove) {
    pSoldier->usActionData = FindNearestUngassedLand(pSoldier);

    if (pSoldier->usActionData != NOWHERE) {
#ifdef DEBUGDECISIONS
      sprintf(tempstr, "%s - SEEKING NEAREST UNGASSED LAND at grid %d", pSoldier->name,
              pSoldier->usActionData);
      AIPopMessage(tempstr);
#endif

      return (AI_ACTION_LEAVE_WATER_GAS);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // SOLDIER CAN ATTACK IF NOT IN WATER/GAS AND NOT DOING SOMETHING TOO FUNKY
  ////////////////////////////////////////////////////////////////////////////

  // NPCs in water/tear gas without masks are not permitted to shoot/stab/throw
  if ((pSoldier->bActionPoints < 2) /*|| bInWater*/ || bInGas) {
    bCanAttack = FALSE;
  } else {
    bCanAttack = CanNPCAttack(pSoldier);
    if (bCanAttack != TRUE) {
      if (bCanAttack == NOSHOOT_NOAMMO) {
        pSoldier->inv[HANDPOS].fFlags |= OBJECT_AI_UNUSABLE;

        // try to find a bladed weapon
        if (pSoldier->ubBodyType == QUEENMONSTER) {
          bWeaponIn = FindObjClass(pSoldier, IC_TENTACLES);
        } else {
          bWeaponIn = FindObjClass(pSoldier, IC_BLADE);
        }

        if (bWeaponIn != NO_SLOT) {
          RearrangePocket(pSoldier, HANDPOS, bWeaponIn, FOREVER);
          bCanAttack = TRUE;
        } else {
          // infants who exhaust their spit should flee!
          fRunAway = TRUE;
          bCanAttack = FALSE;
        }

      } else {
        bCanAttack = FALSE;
      }
    }
  }

  BestShot.ubPossible = FALSE;  // by default, assume Shooting isn't possible
  BestStab.ubPossible = FALSE;  // by default, assume Stabbing isn't possible

  BestAttack.ubChanceToReallyHit = 0;

  bSpitIn = NO_SLOT;

  // if we are able attack
  if (bCanAttack) {
    //////////////////////////////////////////////////////////////////////////
    // FIRE A GUN AT AN OPPONENT
    //////////////////////////////////////////////////////////////////////////

    pSoldier->bAimShotLocation = AIM_SHOT_RANDOM;

    bWeaponIn = FindObjClass(pSoldier, IC_GUN);

    if (bWeaponIn != NO_SLOT) {
      if (Item[pSoldier->inv[bWeaponIn].usItem].usItemClass == IC_GUN &&
          pSoldier->inv[bWeaponIn].bGunStatus >= USABLE) {
        if (pSoldier->inv[bWeaponIn].ubGunShotsLeft > 0) {
          bSpitIn = bWeaponIn;
          // if it's in another pocket, swap it into his hand temporarily
          if (bWeaponIn != HANDPOS) {
            RearrangePocket(pSoldier, HANDPOS, bWeaponIn, TEMPORARILY);
          }

          // now it better be a gun, or the guy can't shoot (but has other attack(s))

          // get the minimum cost to attack the same target with this gun
          ubMinAPCost = MinAPsToAttack(pSoldier, pSoldier->sLastTarget, DONTADDTURNCOST);

          // if we have enough action points to shoot with this gun
          if (pSoldier->bActionPoints >= ubMinAPCost) {
            // look around for a worthy target (which sets BestShot.ubPossible)
            CalcBestShot(pSoldier, &BestShot);

            if (BestShot.ubPossible) {
              BestShot.bWeaponIn = bWeaponIn;

              // if the selected opponent is not a threat (unconscious & !serviced)
              // (usually, this means all the guys we see our unconscious, but, on
              //  rare occasions, we may not be able to shoot a healthy guy, too)
              if ((Menptr[BestShot.ubOpponent].bLife < OKLIFE) &&
                  !Menptr[BestShot.ubOpponent].bService) {
                // if our attitude is NOT aggressive
                if (pSoldier->bAttitude != AGGRESSIVE) {
                  // get the location of the closest CONSCIOUS reachable opponent
                  sClosestDisturbance = ClosestReachableDisturbance(pSoldier, FALSE, &fChangeLevel);

                  // if we found one
                  if (sClosestDisturbance != NOWHERE) {
// don't bother checking GRENADES/KNIVES, he can't have conscious targets
#ifdef RECORDNET
                    fprintf(NetDebugFile,
                            "\tDecideActionBlack: all visible opponents unconscious, switching to "
                            "RED AI...\n");
#endif
                    // then make decision as if at alert status RED, but make sure
                    // we don't try to SEEK OPPONENT the unconscious guy!
                    return (DecideActionRed(pSoldier, FALSE));
                  }
                  // else kill the guy, he could be the last opponent alive in this sector
                }
                // else aggressive guys will ALWAYS finish off unconscious opponents
              }

              // now we KNOW FOR SURE that we will do something (shoot, at least)
              NPCDoesAct(pSoldier);
            }
          }
          // if it was in his holster, swap it back into his holster for now
          if (bWeaponIn != HANDPOS) {
            RearrangePocket(pSoldier, HANDPOS, bWeaponIn, TEMPORARILY);
          }
        } else {
          // out of ammo! reload if possible!
        }
      }
    }

    //////////////////////////////////////////////////////////////////////////
    // GO STAB AN OPPONENT WITH A KNIFE
    //////////////////////////////////////////////////////////////////////////

    // if soldier has a knife in his hand
    if (pSoldier->ubBodyType == QUEENMONSTER) {
      bWeaponIn = FindObjClass(pSoldier, IC_TENTACLES);
    } else if (pSoldier->ubBodyType == BLOODCAT) {
      // 1 in 3 attack with teeth, otherwise with claws
      if (PreRandom(3)) {
        bWeaponIn = FindObj(pSoldier, BLOODCAT_CLAW_ATTACK);
      } else {
        bWeaponIn = FindObj(pSoldier, BLOODCAT_BITE);
      }
    } else {
      if (bSpitIn != NO_SLOT && Random(4)) {
        // spitters only consider a blade attack 1 time in 4
        bWeaponIn = NO_SLOT;
      } else {
        bWeaponIn = FindObjClass(pSoldier, IC_BLADE);
      }
    }

    BestStab.iAttackValue = 0;

    // if the soldier does have a usable knife somewhere

    // spitters don't always consider using their claws
    if (bWeaponIn != NO_SLOT) {
      // if it's in his holster, swap it into his hand temporarily
      if (bWeaponIn != HANDPOS) {
        RearrangePocket(pSoldier, HANDPOS, bWeaponIn, TEMPORARILY);
      }

      // get the minimum cost to attack with this knife
      ubMinAPCost = MinAPsToAttack(pSoldier, pSoldier->sLastTarget, DONTADDTURNCOST);

      // sprintf(tempstr,"%s - ubMinAPCost = %d",pSoldier->name,ubMinAPCost);
      // PopMessage(tempstr);

      // if we can afford the minimum AP cost to stab with this knife weapon
      if (pSoldier->bActionPoints >= ubMinAPCost) {
        // then look around for a worthy target (which sets BestStab.ubPossible)

        if (pSoldier->ubBodyType == QUEENMONSTER) {
          CalcTentacleAttack(pSoldier, &CurrStab);
        } else {
          CalcBestStab(pSoldier, &CurrStab, TRUE);
        }

        if (CurrStab.ubPossible) {
          // now we KNOW FOR SURE that we will do something (stab, at least)
          NPCDoesAct(pSoldier);
        }

        // if it was in his holster, swap it back into his holster for now
        if (bWeaponIn != HANDPOS) {
          RearrangePocket(pSoldier, HANDPOS, bWeaponIn, TEMPORARILY);
        }

        if (CurrStab.iAttackValue > BestStab.iAttackValue) {
          CurrStab.bWeaponIn = bWeaponIn;
          memcpy(&BestStab, &CurrStab, sizeof(BestStab));
        }
      }
    }

    //////////////////////////////////////////////////////////////////////////
    // CHOOSE THE BEST TYPE OF ATTACK OUT OF THOSE FOUND TO BE POSSIBLE
    //////////////////////////////////////////////////////////////////////////
    if (BestShot.ubPossible) {
      BestAttack.iAttackValue = BestShot.iAttackValue;
      ubBestAttackAction = AI_ACTION_FIRE_GUN;
    } else {
      BestAttack.iAttackValue = 0;
      ubBestAttackAction = AI_ACTION_NONE;
    }
    if (BestStab.ubPossible && BestStab.iAttackValue > (BestAttack.iAttackValue * 12) / 10) {
      BestAttack.iAttackValue = BestStab.iAttackValue;
      ubBestAttackAction = AI_ACTION_KNIFE_MOVE;
    }

    // if attack is still desirable (meaning it's also preferred to taking cover)
    if (ubBestAttackAction != AI_ACTION_NONE) {
      // copy the information on the best action selected into BestAttack struct
      switch (ubBestAttackAction) {
        case AI_ACTION_FIRE_GUN:
          memcpy(&BestAttack, &BestShot, sizeof(BestAttack));
          break;

        case AI_ACTION_KNIFE_MOVE:
          memcpy(&BestAttack, &BestStab, sizeof(BestAttack));
          break;
      }

      // if necessary, swap the weapon into the hand position
      if (BestAttack.bWeaponIn != HANDPOS) {
        // IS THIS NOT BEING SET RIGHT?????
        RearrangePocket(pSoldier, HANDPOS, BestAttack.bWeaponIn, FOREVER);
      }

      //////////////////////////////////////////////////////////////////////////
      // GO AHEAD & ATTACK!
      //////////////////////////////////////////////////////////////////////////

      pSoldier->usActionData = BestAttack.sTarget;
      pSoldier->bAimTime = BestAttack.ubAimTime;

      if (ubBestAttackAction == AI_ACTION_FIRE_GUN && BestAttack.ubChanceToReallyHit > 50) {
        pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
      } else {
        pSoldier->bAimShotLocation = AIM_SHOT_RANDOM;
      }

#ifdef DEBUGDECISIONS
      DebugAI(
          String("%d(%s) %s %d(%s) at gridno %d (%d APs aim)\n", GetSolID(pSoldier), pSoldier->name,
                 (ubBestAttackAction == AI_ACTION_FIRE_GUN)
                     ? "SHOOTS"
                     : ((ubBestAttackAction == AI_ACTION_TOSS_PROJECTILE) ? "TOSSES AT" : "STABS"),
                 BestAttack.ubOpponent, ExtMen[BestAttack.ubOpponent].name, BestAttack.target,
                 BestAttack.aimTime));
#endif

      return (ubBestAttackAction);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // CLOSE ON THE CLOSEST KNOWN OPPONENT or TURN TO FACE HIM
  ////////////////////////////////////////////////////////////////////////////

  if (!fRunAway) {
    if ((GetAPsToLook(pSoldier) <= pSoldier->bActionPoints)) {
      // determine the location of the known closest opponent
      // (don't care if he's conscious, don't care if he's reachable at all)
      sClosestOpponent = ClosestKnownOpponent(pSoldier, NULL, NULL);
      // if we have a closest reachable opponent
      if (sClosestOpponent != NOWHERE) {
        if (ubCanMove && PythSpacesAway(pSoldier->sGridNo, sClosestOpponent) > 2) {
          if (bSpitIn != NO_SLOT) {
            pSoldier->usActionData = AdvanceToFiringRange(pSoldier, sClosestOpponent);
            if (pSoldier->usActionData == NOWHERE) {
              pSoldier->usActionData =
                  GoAsFarAsPossibleTowards(pSoldier, sClosestOpponent, AI_ACTION_SEEK_OPPONENT);
            }
          } else {
            pSoldier->usActionData =
                GoAsFarAsPossibleTowards(pSoldier, sClosestOpponent, AI_ACTION_SEEK_OPPONENT);
          }
        } else {
          pSoldier->usActionData = NOWHERE;
        }

        if (pSoldier->usActionData != NOWHERE)  // charge!
        {
          return (AI_ACTION_SEEK_OPPONENT);
        } else if (GetAPsToLook(pSoldier) <= pSoldier->bActionPoints)  // turn to face enemy
        {
          bDirection = atan8(CenterX(pSoldier->sGridNo), CenterY(pSoldier->sGridNo),
                             CenterX(sClosestOpponent), CenterY(sClosestOpponent));

          // if we're not facing towards him
          if (pSoldier->bDirection != bDirection && ValidCreatureTurn(pSoldier, bDirection)) {
            pSoldier->usActionData = bDirection;

#ifdef DEBUGDECISIONS
            sprintf(tempstr, "%s - TURNS to face CLOSEST OPPONENT in direction %d", pSoldier->name,
                    pSoldier->usActionData);
            AIPopMessage(tempstr);
#endif

            return (AI_ACTION_CHANGE_FACING);
          }
        }
      }
    }
  } else {
    // run away!
    if (ubCanMove) {
      // look for best place to RUN AWAY to (farthest from the closest threat)
      // pSoldier->usActionData = RunAway( pSoldier );
      pSoldier->usActionData = FindSpotMaxDistFromOpponents(pSoldier);

      if (pSoldier->usActionData != NOWHERE) {
        return (AI_ACTION_RUN_AWAY);
      }
    }
  }
  ////////////////////////////////////////////////////////////////////////////
  // DO NOTHING: Not enough points left to move, so save them for next turn
  ////////////////////////////////////////////////////////////////////////////

#ifdef DEBUGDECISIONS
  AINameMessage(pSoldier, "- DOES NOTHING (BLACK)", 1000);
#endif

  // by default, if everything else fails, just stand in place and wait
  pSoldier->usActionData = NOWHERE;
  return (AI_ACTION_NONE);
}

int8_t CreatureDecideAction(struct SOLDIERTYPE *pSoldier) {
  int8_t bAction = AI_ACTION_NONE;

  switch (pSoldier->bAlertStatus) {
    case STATUS_GREEN:
#ifdef DEBUGDECISIONS
      AIPopMessage("AlertStatus = GREEN");
#endif
      bAction = CreatureDecideActionGreen(pSoldier);
      break;

    case STATUS_YELLOW:
#ifdef DEBUGDECISIONS
      AIPopMessage("AlertStatus = YELLOW");
#endif
      bAction = CreatureDecideActionYellow(pSoldier);
      break;

    case STATUS_RED:
#ifdef DEBUGDECISIONS
      AIPopMessage("AlertStatus = RED");
#endif
      bAction = CreatureDecideActionRed(pSoldier, TRUE);
      break;

    case STATUS_BLACK:
#ifdef DEBUGDECISIONS
      AIPopMessage("AlertStatus = BLACK");
#endif
      bAction = CreatureDecideActionBlack(pSoldier);
      break;
  }

#ifdef DEBUGDECISIONS
  DebugAI(String("DecideAction: selected action %d, actionData %d\n\n", action,
                 pSoldier->usActionData));
#endif

  return (bAction);
}

void CreatureDecideAlertStatus(struct SOLDIERTYPE *pSoldier) {
  int8_t bOldStatus;
  int32_t iDummy;
  BOOLEAN fClimbDummy, fReachableDummy;

  // THE FOUR (4) POSSIBLE ALERT STATUSES ARE:
  // GREEN - No one sensed, no suspicious noise heard, go about doing regular stuff
  // YELLOW - Suspicious noise was heard personally
  // RED - Either saw OPPONENTS in person, or definite contact had been called
  // BLACK - Currently has one or more OPPONENTS in sight

  // set mobility
  switch (pSoldier->ubBodyType) {
    case ADULTFEMALEMONSTER:
    case YAF_MONSTER:
    case AM_MONSTER:
    case YAM_MONSTER:
    case INFANT_MONSTER:
      pSoldier->bMobility = CREATURE_MOBILE;
      break;
    case QUEENMONSTER:
      pSoldier->bMobility = CREATURE_IMMOBILE;
      break;
    case LARVAE_MONSTER:
      pSoldier->bMobility = CREATURE_CRAWLER;
      break;
  }

  if (pSoldier->ubBodyType == LARVAE_MONSTER) {
    // larvae never do anything much!
    pSoldier->bAlertStatus = STATUS_GREEN;
    return;
  }

  // save the man's previous status
  bOldStatus = pSoldier->bAlertStatus;

  // determine the current alert status for this category of man
  if (pSoldier->bOppCnt > 0)  // opponent(s) in sight
  {
    // must search through list of people to see if any of them have
    // attacked us, or do some check to see if we have been attacked
    switch (bOldStatus) {
      case STATUS_GREEN:
      case STATUS_YELLOW:
        pSoldier->bAlertStatus = STATUS_BLACK;
        break;
      case STATUS_RED:
      case STATUS_BLACK:
        pSoldier->bAlertStatus = STATUS_BLACK;
    }

  } else  // no opponents are in sight
  {
    switch (bOldStatus) {
      case STATUS_BLACK:
        // then drop back to RED status
        pSoldier->bAlertStatus = STATUS_RED;
        break;

      case STATUS_RED:
        // RED can never go back down below RED, only up to BLACK
        break;

      case STATUS_YELLOW:
        // if all enemies have been RED alerted, or we're under fire
        if (gTacticalStatus.Team[pSoldier->bTeam].bAwareOfOpposition || pSoldier->bUnderFire) {
          pSoldier->bAlertStatus = STATUS_RED;
        } else {
          // if we are NOT aware of any uninvestigated noises right now
          // and we are not currently in the middle of an action
          // (could still be on his way heading to investigate a noise!)
          if ((MostImportantNoiseHeard(pSoldier, &iDummy, &fClimbDummy, &fReachableDummy) ==
               NOWHERE) &&
              !pSoldier->bActionInProgress) {
            // then drop back to GREEN status
            pSoldier->bAlertStatus = STATUS_GREEN;
          }
        }
        break;

      case STATUS_GREEN:
        // if all enemies have been RED alerted, or we're under fire
        if (gTacticalStatus.Team[pSoldier->bTeam].bAwareOfOpposition || pSoldier->bUnderFire) {
          pSoldier->bAlertStatus = STATUS_RED;
        } else {
          // if we ARE aware of any uninvestigated noises right now
          if (MostImportantNoiseHeard(pSoldier, &iDummy, &fClimbDummy, &fReachableDummy) !=
              NOWHERE) {
            // then move up to YELLOW status
            pSoldier->bAlertStatus = STATUS_YELLOW;
          }
        }
        break;
    }
    // otherwise, RED stays RED, YELLOW stays YELLOW, GREEN stays GREEN
  }

  // if the creatures alert status has changed in any way
  if (pSoldier->bAlertStatus != bOldStatus) {
    // HERE ARE TRYING TO AVOID NPCs SHUFFLING BACK & FORTH BETWEEN RED & BLACK
    // if either status is < RED (ie. anything but RED->BLACK && BLACK->RED)
    if ((bOldStatus < STATUS_RED) || (pSoldier->bAlertStatus < STATUS_RED)) {
      // force a NEW action decision on next pass through HandleManAI()
      SetNewSituation(pSoldier);
    }

    // if this guy JUST discovered that there were opponents here for sure...
    if ((bOldStatus < STATUS_RED) && (pSoldier->bAlertStatus >= STATUS_RED)) {
      // might want to make custom to let them go anywhere
      CheckForChangingOrders(pSoldier);
    }

#ifdef DEBUGDECISIONS
    // don't report status changes for human-controlled mercs
    if (!pSoldier->human) {
      sprintf(tempstr, "%s's Alert Status changed from %d to %d", ExtMen[pSoldier->guynum].name,
              oldStatus, pSoldier->bAlertStatus);
      AIPopMessage(tempstr);
    }
#endif

  } else  // status didn't change
  {
    // if a guy on status GREEN or YELLOW is running low on breath
    if (((pSoldier->bAlertStatus == STATUS_GREEN) && (pSoldier->bBreath < 75)) ||
        ((pSoldier->bAlertStatus == STATUS_YELLOW) && (pSoldier->bBreath < 50))) {
      // as long as he's not in water (standing on a bridge is OK)
      if (!MercInWater(pSoldier)) {
        // force a NEW decision so that he can get some rest
        SetNewSituation(pSoldier);

        // current action will be canceled. if noise is no longer important
        if ((pSoldier->bAlertStatus == STATUS_YELLOW) &&
            (MostImportantNoiseHeard(pSoldier, &iDummy, &fClimbDummy, &fReachableDummy) ==
             NOWHERE)) {
          // then drop back to GREEN status
          pSoldier->bAlertStatus = STATUS_GREEN;
          CheckForChangingOrders(pSoldier);
        }
      }
    }
  }
}

int8_t CrowDecideActionRed(struct SOLDIERTYPE *pSoldier) {
  // OK, Fly away!
  // HandleCrowFlyAway( pSoldier );
  if (!gfTurnBasedAI) {
    pSoldier->usActionData = 30000;
    return (AI_ACTION_WAIT);
  } else {
    return (AI_ACTION_NONE);
  }
}

int8_t CrowDecideActionGreen(struct SOLDIERTYPE *pSoldier) {
  int16_t sCorpseGridNo;
  uint8_t ubDirection;
  int16_t sFacingDir;

  // Look for a corse!
  sCorpseGridNo = FindNearestRottingCorpse(pSoldier);

  if (sCorpseGridNo != NOWHERE) {
    // Are we close, if so , peck!
    if (SpacesAway(pSoldier->sGridNo, sCorpseGridNo) < 2) {
      // Change facing
      sFacingDir = GetDirectionFromGridNo(sCorpseGridNo, pSoldier);

      if (sFacingDir != pSoldier->bDirection) {
        pSoldier->usActionData = sFacingDir;
        return (AI_ACTION_CHANGE_FACING);
      } else if (!gfTurnBasedAI) {
        pSoldier->usActionData = 30000;
        return (AI_ACTION_WAIT);
      } else {
        return (AI_ACTION_NONE);
      }
    } else {
      // Walk to nearest one!
      pSoldier->usActionData = FindGridNoFromSweetSpot(pSoldier, sCorpseGridNo, 4, &ubDirection);
      if (pSoldier->usActionData != NOWHERE) {
        return (AI_ACTION_GET_CLOSER);
      }
    }
  }

  return (AI_ACTION_NONE);
}

int8_t CrowDecideAction(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->usAnimState == CROW_FLY) {
    return (AI_ACTION_NONE);
  }

  switch (pSoldier->bAlertStatus) {
    case STATUS_GREEN:
    case STATUS_YELLOW:
      return (CrowDecideActionGreen(pSoldier));

    case STATUS_RED:
    case STATUS_BLACK:
      return (CrowDecideActionRed(pSoldier));

    default:
      Assert(FALSE);
      return (AI_ACTION_NONE);
  }
}
