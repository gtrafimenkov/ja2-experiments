#ifndef __AIMMEMBERS_H_
#define __AIMMEMBERS_H_

#include "MouseInput.h"
#include "SGP/Types.h"

void GameInitAIMMembers();
BOOLEAN EnterAIMMembers();
void ExitAIMMembers();
void HandleAIMMembers(const struct MouseInput mouse);
BOOLEAN RenderAIMMembers();

BOOLEAN DrawNumeralsToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY,
                             UINT32 ulFont, UINT8 ubColor);
BOOLEAN DrawMoneyToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY, UINT32 ulFont,
                          UINT8 ubColor);

void DisplayTextForMercFaceVideoPopUp(STR16 pString);
BOOLEAN DisplayTalkingMercFaceForVideoPopUp(INT32 iFaceIndex, const struct MouseInput mouse);
void EnterInitAimMembers();
BOOLEAN RenderAIMMembersTopLevel();
void ResetMercAnnoyanceAtPlayer(UINT8 ubMercID);
BOOLEAN DisableNewMailMessage();
void DisplayPopUpBoxExplainingMercArrivalLocationAndTime();

// which mode are we in during video conferencing?..0 means no video conference
extern UINT8 gubVideoConferencingMode;

// TEMP!!!
#ifdef JA2TESTVERSION
void TempHiringOfMercs(UINT8 ubNumberOfMercs, BOOLEAN fReset);
#endif

#if defined(JA2TESTVERSION)
void DemoHiringOfMercs();
#endif

#endif
