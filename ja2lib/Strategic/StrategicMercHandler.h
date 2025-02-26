// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _STRATEGIC_MERC_HANDLER_H_
#define _STRATEGIC_MERC_HANDLER_H_

#include "SGP/Types.h"

struct SOLDIERTYPE;

void StrategicHandlePlayerTeamMercDeath(struct SOLDIERTYPE *pSoldier);
void MercDailyUpdate();
void MercsContractIsFinished(uint8_t ubID);
void RPCWhineAboutNoPay(uint8_t ubID);
void MercComplainAboutEquipment(uint8_t ubProfileID);
BOOLEAN SoldierHasWorseEquipmentThanUsedTo(struct SOLDIERTYPE *pSoldier);
void UpdateBuddyAndHatedCounters(void);
void HourlyCamouflageUpdate(void);
#endif
