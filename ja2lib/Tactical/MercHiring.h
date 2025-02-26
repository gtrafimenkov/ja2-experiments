// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _MERC_HIRING_H_
#define _MERC_HIRING_H_

#include "SGP/Types.h"

struct SOLDIERTYPE;

//
// Used with the HireMerc function
//
#define MERC_HIRE_OVER_20_MERCS_HIRED -1
#define MERC_HIRE_FAILED 0
#define MERC_HIRE_OK 1

#define MERC_ARRIVE_TIME_SLOT_1 (7 * 60 + 30)   // 7:30 a.m.
#define MERC_ARRIVE_TIME_SLOT_2 (13 * 60 + 30)  // 1:30 pm
#define MERC_ARRIVE_TIME_SLOT_3 (19 * 60 + 30)  // 7:30 pm

// ATE: This define has been moved to be a function so that
// we pick the most appropriate time of day to use...
// #define		MERC_ARRIVAL_TIME_OF_DAY				 (7 * 60 + 30)
//// 7:30 am

typedef struct {
  uint8_t ubProfileID;
  int16_t sSectorX;
  int16_t sSectorY;
  int8_t bSectorZ;
  int16_t iTotalContractLength;
  BOOLEAN fCopyProfileItemsOver;
  uint32_t uiTimeTillMercArrives;
  uint8_t ubInsertionCode;
  uint16_t usInsertionData;
  BOOLEAN fUseLandingZoneForArrival;

} MERC_HIRE_STRUCT;

// ATE: Globals that dictate where the mercs will land once being hired
extern uint8_t gsMercArriveSectorX;
extern uint8_t gsMercArriveSectorY;

int8_t HireMerc(MERC_HIRE_STRUCT *pHireMerc);
void MercArrivesCallback(uint8_t ubSoldierID);
BOOLEAN IsMercHireable(uint8_t ubMercID);
BOOLEAN IsMercDead(uint8_t ubMercID);
uint8_t NumberOfMercsOnPlayerTeam();
BOOLEAN IsTheSoldierAliveAndConcious(struct SOLDIERTYPE *pSoldier);
void HandleMercArrivesQuotes(struct SOLDIERTYPE *pSoldier);
void UpdateAnyInTransitMercsWithGlobalArrivalSector();

uint32_t GetMercArrivalTimeOfDay();

#endif
