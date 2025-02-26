// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __AI_H
#define __AI_H

#include "SGP/Types.h"
#include "TileEngine/WorldDef.h"

struct SOLDIERTYPE;

#define TESTAICONTROL

extern int8_t gubAIPathCosts[19][19];
#define AI_PATHCOST_RADIUS 9

extern BOOLEAN gfDisplayCoverValues;
extern int16_t gsCoverValue[WORLD_MAX];

// AI actions

typedef enum {
  CALL_NONE = 0,
  CALL_1_PREY,
  CALL_MULTIPLE_PREY,
  CALL_ATTACKED,
  CALL_CRIPPLED,
  NUM_CREATURE_CALLS
} CreatureCalls;

#define DONTFORCE 0
#define FORCE 1

// ANY NEW ACTIONS ADDED - UPDATE OVERHEAD.C ARRAY WITH ACTION'S STRING VALUE
#define FIRST_MOVEMENT_ACTION AI_ACTION_RANDOM_PATROL
#define LAST_MOVEMENT_ACTION AI_ACTION_MOVE_TO_CLIMB
typedef enum {
  AI_ACTION_NONE = 0,  // maintain current position & facing

  // actions that involve a move to another tile
  AI_ACTION_RANDOM_PATROL,  // move towards a random destination
  AI_ACTION_SEEK_FRIEND,    // move towards friend in trouble
  AI_ACTION_SEEK_OPPONENT,  // move towards a reported opponent
  AI_ACTION_TAKE_COVER,     // run for nearest cover from threat
  AI_ACTION_GET_CLOSER,     // move closer to a strategic location

  AI_ACTION_POINT_PATROL,     // move towards next patrol point
  AI_ACTION_LEAVE_WATER_GAS,  // seek nearest spot of ungassed land
  AI_ACTION_SEEK_NOISE,       // seek most important noise heard
  AI_ACTION_ESCORTED_MOVE,    // go where told to by escortPlayer
  AI_ACTION_RUN_AWAY,         // run away from nearby opponent(s)

  AI_ACTION_KNIFE_MOVE,     // preparing to stab an opponent
  AI_ACTION_APPROACH_MERC,  // move up to a merc in order to talk with them; RT
  AI_ACTION_TRACK,          // track a scent
  AI_ACTION_EAT,            // monster eats corpse
  AI_ACTION_PICKUP_ITEM,    // grab things lying on the ground

  AI_ACTION_SCHEDULE_MOVE,  // move according to schedule
  AI_ACTION_WALK,           // walk somewhere (NPC stuff etc)
  AI_ACTION_RUN,            // run somewhere (NPC stuff etc)
  AI_ACTION_MOVE_TO_CLIMB,  // move to edge of roof/building
  // miscellaneous movement actions
  AI_ACTION_CHANGE_FACING,  // turn to face a different direction

  AI_ACTION_CHANGE_STANCE,  // stand, crouch, or go prone
  // actions related to items and attacks
  AI_ACTION_YELLOW_ALERT,   // tell friends opponent(s) heard
  AI_ACTION_RED_ALERT,      // tell friends opponent(s) seen
  AI_ACTION_CREATURE_CALL,  // creature communication
  AI_ACTION_PULL_TRIGGER,   // go off to activate a panic trigger

  AI_ACTION_USE_DETONATOR,    // grab detonator and set off bomb(s)
  AI_ACTION_FIRE_GUN,         // shoot at nearby opponent
  AI_ACTION_TOSS_PROJECTILE,  // throw grenade at/near opponent(s)
  AI_ACTION_KNIFE_STAB,       // during the actual knifing attack
  AI_ACTION_THROW_KNIFE,      // throw a knife

  AI_ACTION_GIVE_AID,        // help injured/dying friend
  AI_ACTION_WAIT,            // RT: don't do anything for a certain length of time
  AI_ACTION_PENDING_ACTION,  // RT: wait for pending action (pickup, door open, etc) to finish
  AI_ACTION_DROP_ITEM,       // duh
  AI_ACTION_COWER,           // for civilians:  cower in fear and stay there!

  AI_ACTION_STOP_COWERING,       // stop cowering
  AI_ACTION_OPEN_OR_CLOSE_DOOR,  // schedule-provoked; open or close door
  AI_ACTION_UNLOCK_DOOR,         // schedule-provoked; unlock door (don't open)
  AI_ACTION_LOCK_DOOR,           // schedule-provoked; lock door (close if necessary)
  AI_ACTION_LOWER_GUN,           // lower gun prior to throwing knife

  AI_ACTION_ABSOLUTELY_NONE,     // like "none" but can't be converted to a wait by realtime
  AI_ACTION_CLIMB_ROOF,          // climb up or down roof
  AI_ACTION_END_TURN,            // end turn (after final stance change)
  AI_ACTION_END_COWER_AND_MOVE,  // sort of dummy value, special for civilians who are to go
                                 // somewhere at end of battle
  AI_ACTION_TRAVERSE_DOWN,       // move down a level
  AI_ACTION_OFFER_SURRENDER,     // offer surrender to the player
} ActionType;

typedef enum {
  QUOTE_ACTION_ID_CHECKFORDEST = 1,
  QUOTE_ACTION_ID_TURNTOWARDSPLAYER,
  QUOTE_ACTION_ID_DRAWGUN,
  QUOTE_ACTION_ID_LOWERGUN,
  QUOTE_ACTION_ID_TRAVERSE_EAST,
  QUOTE_ACTION_ID_TRAVERSE_SOUTH,
  QUOTE_ACTION_ID_TRAVERSE_WEST,
  QUOTE_ACTION_ID_TRAVERSE_NORTH,
} QuoteActionType;

#define RTP_COMBAT_AGGRESSIVE 1
#define RTP_COMBAT_CONSERVE 2
#define RTP_COMBAT_REFRAIN 3

// NB THESE THREE FLAGS SHOULD BE REMOVED FROM CODE
#define AI_RTP_OPTION_CAN_RETREAT 0x01
#define AI_RTP_OPTION_CAN_SEEK_COVER 0x02
#define AI_RTP_OPTION_CAN_HELP 0x04

#define AI_CAUTIOUS 0x08
#define AI_HANDLE_EVERY_FRAME 0x10
#define AI_ASLEEP 0x20
#define AI_LOCK_DOOR_INCLUDES_CLOSE 0x40
#define AI_CHECK_SCHEDULE 0x80

#define NOT_NEW_SITUATION 0
#define WAS_NEW_SITUATION 1
#define IS_NEW_SITUATION 2

#define DIFF_ENEMY_EQUIP_MOD 0
#define DIFF_ENEMY_TO_HIT_MOD 1
#define DIFF_ENEMY_INTERRUPT_MOD 2
#define DIFF_RADIO_RED_ALERT 3
#define DIFF_MAX_COVER_RANGE 4
#define MAX_DIFF_PARMS 5  // how many different difficulty variables?

extern int8_t gbDiff[MAX_DIFF_PARMS][5];

void ActionDone(struct SOLDIERTYPE *pSoldier);
int16_t ActionInProgress(struct SOLDIERTYPE *pSoldier);

int8_t CalcMorale(struct SOLDIERTYPE *pSoldier);
int32_t CalcPercentBetter(int32_t iOldValue, int32_t iNewValue, int32_t iOldScale,
                          int32_t iNewScale);
void CallAvailableEnemiesTo(int16_t sGridno);
void CallAvailableKingpinMenTo(int16_t sGridNo);
void CallAvailableTeamEnemiesTo(int16_t sGridno, int8_t bTeam);
void CallEldinTo(int16_t sGridNo);
void CancelAIAction(struct SOLDIERTYPE *pSoldier, uint8_t ubForce);
void CheckForChangingOrders(struct SOLDIERTYPE *pSoldier);

int8_t ClosestPanicTrigger(struct SOLDIERTYPE *pSoldier);

int16_t ClosestKnownOpponent(struct SOLDIERTYPE *pSoldier, int16_t *psGridNo, int8_t *pbLevel);
int16_t ClosestPC(struct SOLDIERTYPE *pSoldier, int16_t *psDistance);
BOOLEAN CanAutoBandage(BOOLEAN fDoFullCheck);

void DebugAI(char *szOutput);
int8_t DecideAction(struct SOLDIERTYPE *pSoldier);
int8_t DecideActionBlack(struct SOLDIERTYPE *pSoldier);
int8_t DecideActionEscort(struct SOLDIERTYPE *pSoldier);
int8_t DecideActionGreen(struct SOLDIERTYPE *pSoldier);
int8_t DecideActionRed(struct SOLDIERTYPE *pSoldier, uint8_t ubUnconsciousOK);
int8_t DecideActionYellow(struct SOLDIERTYPE *pSoldier);

int16_t DistanceToClosestFriend(struct SOLDIERTYPE *pSoldier);

void EndAIDeadlock(void);
void EndAIGuysTurn(struct SOLDIERTYPE *pSoldier);

int8_t ExecuteAction(struct SOLDIERTYPE *pSoldier);

int16_t FindAdjacentSpotBeside(struct SOLDIERTYPE *pSoldier, int16_t sGridno);
int16_t FindBestNearbyCover(struct SOLDIERTYPE *pSoldier, int32_t morale, int32_t *pPercentBetter);
int16_t FindClosestDoor(struct SOLDIERTYPE *pSoldier);
int16_t FindNearbyPointOnEdgeOfMap(struct SOLDIERTYPE *pSoldier, int8_t *pbDirection);
int16_t FindNearestEdgePoint(int16_t sGridNo);

// Kris:  Added these as I need specific searches on certain sides.
enum {
  NORTH_EDGEPOINT_SEARCH,
  EAST_EDGEPOINT_SEARCH,
  SOUTH_EDGEPOINT_SEARCH,
  WEST_EDGEPOINT_SEARCH,
};
int16_t FindNearestEdgepointOnSpecifiedEdge(int16_t sGridNo, int8_t bEdgeCode);

int16_t FindNearestUngassedLand(struct SOLDIERTYPE *pSoldier);
BOOLEAN FindRoofClimbingPoints(struct SOLDIERTYPE *pSoldier, int16_t sDesiredSpot);
int16_t FindSpotMaxDistFromOpponents(struct SOLDIERTYPE *pSoldier);
int16_t FindSweetCoverSpot(struct SOLDIERTYPE *pSoldier);

void FreeUpNPCFromAttacking(uint8_t ubID);
void FreeUpNPCFromPendingAction(struct SOLDIERTYPE *pSoldier);
void FreeUpNPCFromTurning(struct SOLDIERTYPE *pSoldier, int8_t bLook);
void FreeUpNPCFromStanceChange(struct SOLDIERTYPE *pSoldier);
void FreeUpNPCFromLoweringGun(struct SOLDIERTYPE *pSoldier);
void FreeUpNPCFromRoofClimb(struct SOLDIERTYPE *pSoldier);

uint8_t GetClosestOpponent(struct SOLDIERTYPE *pSoldier);
uint8_t GetMostThreateningOpponent(struct SOLDIERTYPE *pSoldier);

void HandleSoldierAI(struct SOLDIERTYPE *pSoldier);
void HandleInitialRedAlert(int8_t bTeam, uint8_t ubCommunicate);

void InitPanicSystem();
int16_t InWaterOrGas(struct SOLDIERTYPE *pSoldier, int16_t sGridno);
BOOLEAN IsActionAffordable(struct SOLDIERTYPE *pSoldier);
BOOLEAN InitAI(void);

void MakeClosestEnemyChosenOne();
void ManChecksOnFriends(struct SOLDIERTYPE *pSoldier);

void NewDest(struct SOLDIERTYPE *pSoldier, uint16_t sGridno);
int16_t NextPatrolPoint(struct SOLDIERTYPE *pSoldier);

int8_t PanicAI(struct SOLDIERTYPE *pSoldier, uint8_t ubCanMove);
void HaltMoveForSoldierOutOfPoints(struct SOLDIERTYPE *pSoldier);

int16_t RandDestWithinRange(struct SOLDIERTYPE *pSoldier);
int16_t RandomFriendWithin(struct SOLDIERTYPE *pSoldier);
int16_t RoamingRange(struct SOLDIERTYPE *pSoldier, uint16_t *pFromGridno);

void SetCivilianDestination(uint8_t ubWho, int16_t sGridno);
void SetNewSituation(struct SOLDIERTYPE *pSoldier);

uint8_t SoldierDifficultyLevel(struct SOLDIERTYPE *pSoldier);
void SoldierTriesToContinueAlongPath(struct SOLDIERTYPE *pSoldier);
void StartNPCAI(struct SOLDIERTYPE *pSoldier);
void TempHurt(struct SOLDIERTYPE *pVictim, struct SOLDIERTYPE *pAttacker);
int TryToResumeMovement(struct SOLDIERTYPE *pSoldier, int16_t sGridno);

BOOLEAN ValidCreatureTurn(struct SOLDIERTYPE *pCreature, int8_t bNewDirection);
#ifdef DEBUGDECISIONS
extern char tempstr[256];
void AIPopMessage(char *str);

void AINumMessage(const char *str, int32_t num);

void AINameMessage(struct SOLDIERTYPE *pSoldier, const char *str, int32_t num);

#endif

BOOLEAN WearGasMaskIfAvailable(struct SOLDIERTYPE *pSoldier);
int16_t WhatIKnowThatPublicDont(struct SOLDIERTYPE *pSoldier, uint8_t ubInSightOnly);

#endif
