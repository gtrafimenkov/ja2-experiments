// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __HISTORY_H
#define __HISTORY_H

#include "SGP/Types.h"

void GameInitHistory();
void EnterHistory();
void ExitHistory();
void HandleHistory();
void RenderHistory();

#define HISTORY_DATA_FILE "TEMP\\History.dat"

// the financial structure
struct history {
  uint8_t ubCode;        // the code index in the finance code table
  uint32_t uiIdNumber;   // unique id number
  uint8_t ubSecondCode;  // secondary code
  uint32_t uiDate;       // time in the world in global time
  int16_t sSectorX;      // sector X this took place in
  int16_t sSectorY;      // sector Y this took place in
  int8_t bSectorZ;
  uint8_t ubColor;
  struct history *Next;  // next unit in the list
};

enum {
  HISTORY_ENTERED_HISTORY_MODE = 0,
  HISTORY_HIRED_MERC_FROM_AIM,
  HISTORY_HIRED_MERC_FROM_MERC,
  HISTORY_MERC_KILLED,
  HISTORY_SETTLED_ACCOUNTS_AT_MERC,
  HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO,
  HISTORY_CHARACTER_GENERATED,
  HISTORY_PURCHASED_INSURANCE,
  HISTORY_CANCELLED_INSURANCE,
  HISTORY_INSURANCE_CLAIM_PAYOUT,
  HISTORY_EXTENDED_CONTRACT_1_DAY,
  HISTORY_EXTENDED_CONTRACT_1_WEEK,
  HISTORY_EXTENDED_CONTRACT_2_WEEK,
  HISTORY_MERC_FIRED,
  HISTORY_MERC_QUIT,
  HISTORY_QUEST_STARTED,
  HISTORY_QUEST_FINISHED,
  HISTORY_TALKED_TO_MINER,
  HISTORY_LIBERATED_TOWN,
  HISTORY_CHEAT_ENABLED,
  HISTORY_TALKED_TO_FATHER_WALKER,
  HISTORY_MERC_MARRIED_OFF,
  HISTORY_MERC_CONTRACT_EXPIRED,
  HISTORY_RPC_JOINED_TEAM,
  HISTORY_ENRICO_COMPLAINED,
  HISTORY_WONBATTLE,
  HISTORY_MINE_RUNNING_OUT,
  HISTORY_MINE_RAN_OUT,
  HISTORY_MINE_SHUTDOWN,
  HISTORY_MINE_REOPENED,
  HISTORY_DISCOVERED_TIXA,
  HISTORY_DISCOVERED_ORTA,
  HISTORY_GOT_ROCKET_RIFLES,
  HISTORY_DEIDRANNA_DEAD_BODIES,
  HISTORY_BOXING_MATCHES,
  HISTORY_SOMETHING_IN_MINES,
  HISTORY_DEVIN,
  HISTORY_MIKE,
  HISTORY_TONY,
  HISTORY_KROTT,
  HISTORY_KYLE,
  HISTORY_MADLAB,
  HISTORY_GABBY,
  HISTORY_KEITH_OUT_OF_BUSINESS,
  HISTORY_HOWARD_CYANIDE,
  HISTORY_KEITH,
  HISTORY_HOWARD,
  HISTORY_PERKO,
  HISTORY_SAM,
  HISTORY_FRANZ,
  HISTORY_ARNOLD,
  HISTORY_FREDO,
  HISTORY_RICHGUY_BALIME,
  HISTORY_JAKE,
  HISTORY_BUM_KEYCARD,
  HISTORY_WALTER,
  HISTORY_DAVE,
  HISTORY_PABLO,
  HISTORY_KINGPIN_MONEY,
  HISTORY_WON_BOXING,
  HISTORY_LOST_BOXING,
  HISTORY_DISQUALIFIED_BOXING,
  HISTORY_FOUND_MONEY,
  HISTORY_ASSASSIN,
  HISTORY_LOSTTOWNSECTOR,
  HISTORY_DEFENDEDTOWNSECTOR,
  HISTORY_LOSTBATTLE,
  HISTORY_FATALAMBUSH,
  HISTORY_WIPEDOUTENEMYAMBUSH,
  HISTORY_UNSUCCESSFULATTACK,
  HISTORY_SUCCESSFULATTACK,
  HISTORY_CREATURESATTACKED,
  HISTORY_KILLEDBYBLOODCATS,
  HISTORY_SLAUGHTEREDBLOODCATS,
  HISTORY_NPC_KILLED,
  HISTORY_GAVE_CARMEN_HEAD,
  HISTORY_SLAY_MYSTERIOUSLY_LEFT,
  HISTORY_MERC_KILLED_CHARACTER,
};

typedef struct history HistoryUnit;
typedef struct history *HistoryUnitPtr;

extern HistoryUnitPtr pHistoryListHead;

// reset history fact..for quests
void ResetHistoryFact(uint8_t ubCode, uint8_t sSectorX, uint8_t sSectorY);

// set history fact...to allow for a different color for in progress quests
uint32_t SetHistoryFact(uint8_t ubCode, uint8_t ubSecondCode, uint32_t uiDate, uint8_t sSectorX, uint8_t sSectorY);

uint32_t AddHistoryToPlayersLog(uint8_t ubCode, uint8_t ubSecondCode, uint32_t uiDate, uint8_t sSectorX,
                              uint8_t sSectorY);
uint32_t GetTimeQuestWasStarted(uint8_t ubCode);

#endif
