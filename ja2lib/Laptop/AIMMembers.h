// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __AIMMEMBERS_H_
#define __AIMMEMBERS_H_

#include "SGP/Types.h"

void GameInitAIMMembers();
BOOLEAN EnterAIMMembers();
void ExitAIMMembers();
void HandleAIMMembers();
BOOLEAN RenderAIMMembers();

BOOLEAN DrawNumeralsToScreen(int32_t iNumber, int8_t bWidth, uint16_t usLocX, uint16_t usLocY,
                             uint32_t ulFont, uint8_t ubColor);
BOOLEAN DrawMoneyToScreen(int32_t iNumber, int8_t bWidth, uint16_t usLocX, uint16_t usLocY,
                          uint32_t ulFont, uint8_t ubColor);

void DisplayTextForMercFaceVideoPopUp(wchar_t* pString);
BOOLEAN DisplayTalkingMercFaceForVideoPopUp(int32_t iFaceIndex);
void EnterInitAimMembers();
BOOLEAN RenderAIMMembersTopLevel();
void ResetMercAnnoyanceAtPlayer(uint8_t ubMercID);
BOOLEAN DisableNewMailMessage();
void DisplayPopUpBoxExplainingMercArrivalLocationAndTime();

// which mode are we in during video conferencing?..0 means no video conference
extern uint8_t gubVideoConferencingMode;

// TEMP!!!
#ifdef JA2TESTVERSION
void TempHiringOfMercs(uint8_t ubNumberOfMercs, BOOLEAN fReset);
#endif

#endif
