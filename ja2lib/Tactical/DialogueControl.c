// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/DialogueControl.h"

#include "GameRes.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/AIMMembers.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Mercs.h"
#include "SGP/ButtonSystem.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Sector.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/MercContract.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/Campaign.h"
#include "Tactical/CivQuotes.h"
#include "Tactical/EndGame.h"
#include "Tactical/Faces.h"
#include "Tactical/Gap.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/LOS.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/QArray.h"
#include "Tactical/ShopKeeperInterface.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "TacticalAI/AI.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/MercTextBox.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/WordWrap.h"

#define DIALOGUESIZE 480
#define QUOTE_MESSAGE_SIZE 520

#define TALK_PANEL_FACE_X 6
#define TALK_PANEL_FACE_Y 9
#define TALK_PANEL_NAME_X 5
#define TALK_PANEL_NAME_Y 114
#define TALK_PANEL_NAME_WIDTH 92
#define TALK_PANEL_NAME_HEIGHT 15
#define TALK_PANEL_MENU_STARTY 8
#define TALK_PANEL_MENU_HEIGHT 24
#define TALK_MENU_WIDTH 96
#define TALK_MENU_HEIGHT 16

#define DIALOGUE_DEFAULT_SUBTITLE_WIDTH 200
#define TEXT_DELAY_MODIFIER 60

typedef struct {
  uint16_t usQuoteNum;
  uint8_t ubCharacterNum;
  int8_t bUIHandlerID;
  int32_t iFaceIndex;
  int32_t iTimeStamp;
  uintptr_t uiSpecialEventFlag;
  uintptr_t uiSpecialEventData;
  uintptr_t uiSpecialEventData2;
  uint32_t uiSpecialEventData3;
  uint32_t uiSpecialEventData4;
  BOOLEAN fFromSoldier;
  BOOLEAN fDelayed;
  BOOLEAN fPauseTime;
} DIALOGUE_Q_STRUCT, *DIALOGUE_Q_STRUCT_PTR;

extern int32_t giMapInvPrev;
extern int32_t giMapInvNext;
extern BOOLEAN gfSKIScreenExit;
extern struct SOLDIERTYPE *pProcessingSoldier;
extern BOOLEAN fProcessingAMerc;
extern uint32_t guiPendingScreen;
extern BOOLEAN fReDrawFace;
extern BOOLEAN gfWaitingForTriggerTimer;

BOOLEAN fExternFacesLoaded = FALSE;

uint32_t uiExternalStaticNPCFaces[NUMBER_OF_EXTERNAL_NPC_FACES];
uint32_t uiExternalFaceProfileIds[NUMBER_OF_EXTERNAL_NPC_FACES] = {
    97, 106, 148, 156, 157, 158,
};

uint8_t gubMercValidPrecedentQuoteID[NUMBER_VALID_MERC_PRECEDENT_QUOTES] = {
    80, 81, 82, 83, 86, 87, 88, 95, 97, 99, 100, 101, 102};

extern int32_t iInterfaceDialogueBox;
extern BOOLEAN gfRerenderInterfaceFromHelpText;
extern uint32_t guiSKI_TransactionButton;

uint16_t gusStopTimeQuoteList[] = {QUOTE_BOOBYTRAP_ITEM, QUOTE_SUSPICIOUS_GROUND};

uint8_t gubNumStopTimeQuotes = 2;

// QUEUE UP DIALOG!
#define INITIAL_Q_SIZE 10
HQUEUE ghDialogueQ = NULL;
FACETYPE *gpCurrentTalkingFace = NULL;
uint8_t gubCurrentTalkingID = NO_PROFILE;
int8_t gbUIHandlerID;

int32_t giNPCReferenceCount = 0;
int32_t giNPCSpecialReferenceCount = 0;

int16_t gsExternPanelXPosition = DEFAULT_EXTERN_PANEL_X_POS;
int16_t gsExternPanelYPosition = DEFAULT_EXTERN_PANEL_Y_POS;

BOOLEAN gfDialogueQueuePaused = FALSE;
uint16_t gusSubtitleBoxWidth;
uint16_t gusSubtitleBoxHeight;
int32_t giTextBoxOverlay = -1;
BOOLEAN gfFacePanelActive = FALSE;
uint32_t guiScreenIDUsedWhenUICreated;
wchar_t gzQuoteStr[QUOTE_MESSAGE_SIZE];
struct MOUSE_REGION gTextBoxMouseRegion;
struct MOUSE_REGION gFacePopupMouseRegion;
BOOLEAN gfUseAlternateDialogueFile = FALSE;

// set the top position value for merc dialogue pop up boxes
int16_t gsTopPosition = 20;

int32_t iDialogueBox = -1;
void RenderSubtitleBoxOverlay(VIDEO_OVERLAY *pBlitter);
void RenderFaceOverlay(VIDEO_OVERLAY *pBlitter);

extern BOOLEAN ContinueDialogue(struct SOLDIERTYPE *pSoldier, BOOLEAN fDone);
extern void HandlePendingInitConv();
extern BOOLEAN WillMercRenew(struct SOLDIERTYPE *pSoldier, BOOLEAN fSayQuote);
extern void DrawFace(int16_t sCharNumber);

// the next said quote will pause time
BOOLEAN fPausedTimeDuringQuote = FALSE;
BOOLEAN fWasPausedDuringDialogue = FALSE;
extern BOOLEAN gfLockPauseState;

int8_t gubLogForMeTooBleeds = FALSE;

// has the text region been created?
BOOLEAN fTextBoxMouseRegionCreated = FALSE;
BOOLEAN fExternFaceBoxRegionCreated = FALSE;

// due to last quote system?
BOOLEAN fDialogueBoxDueToLastMessage = FALSE;

// last quote timers
uint32_t guiDialogueLastQuoteTime = 0;
uint32_t guiDialogueLastQuoteDelay = 0;

void CheckForStopTimeQuotes(uint16_t usQuoteNum);

void TextOverlayClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
void FaceOverlayClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// Handler functions for tactical ui diaplay
void HandleTacticalTextUI(int32_t iFaceIndex, struct SOLDIERTYPE *pSoldier, wchar_t *zQuoteStr);
void HandleTacticalNPCTextUI(uint8_t ubCharacterNum, wchar_t *zQuoteStr);
void HandleTacticalSpeechUI(uint8_t ubCharacterNum, int32_t iFaceIndex);
void DisplayTextForExternalNPC(uint8_t ubCharacterNum, wchar_t *zQuoteStr);
void CreateTalkingUI(int8_t bUIHandlerID, int32_t iFaceIndex, uint8_t ubCharacterNum,
                     struct SOLDIERTYPE *pSoldier, wchar_t *zQuoteStr);

void HandleExternNPCSpeechFace(int32_t iIndex);

extern BOOLEAN ContinueDialogue(struct SOLDIERTYPE *pSoldier, BOOLEAN fDone);
extern BOOLEAN DoSkiMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                               uint8_t ubFlags, MSGBOX_CALLBACK ReturnCallback);

void UnPauseGameDuringNextQuote(void) {
  fPausedTimeDuringQuote = FALSE;

  return;
}

void PauseTimeDuringNextQuote(void) {
  fPausedTimeDuringQuote = TRUE;

  return;
}

BOOLEAN DialogueActive() {
  if (gpCurrentTalkingFace != NULL) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN InitalizeDialogueControl() {
  ghDialogueQ = CreateQueue(INITIAL_Q_SIZE, sizeof(DIALOGUE_Q_STRUCT_PTR));

  // Initalize subtitle popup box
  //

  giNPCReferenceCount = 0;

  if (ghDialogueQ == NULL) {
    return (FALSE);
  } else {
    return (TRUE);
  }
}

void ShutdownDialogueControl() {
  if (ghDialogueQ != NULL) {
    // Empty
    EmptyDialogueQueue();

    // Delete
    DeleteQueue(ghDialogueQ);
    ghDialogueQ = NULL;
  }

  // shutdown external static NPC faces
  ShutdownStaticExternalNPCFaces();

  // gte rid of portraits for cars
  UnLoadCarPortraits();
  //
}

void InitalizeStaticExternalNPCFaces(void) {
  int32_t iCounter = 0;
  // go and grab all external NPC faces that are needed for the game who won't exist as soldiertypes

  if (fExternFacesLoaded == TRUE) {
    return;
  }

  fExternFacesLoaded = TRUE;

  for (iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++) {
    uiExternalStaticNPCFaces[iCounter] =
        (uint32_t)InitFace((uint8_t)(uiExternalFaceProfileIds[iCounter]), NOBODY, FACE_FORCE_SMALL);
  }

  return;
}

void ShutdownStaticExternalNPCFaces(void) {
  int32_t iCounter = 0;

  if (fExternFacesLoaded == FALSE) {
    return;
  }

  fExternFacesLoaded = FALSE;

  // remove all external npc faces
  for (iCounter = 0; iCounter < NUMBER_OF_EXTERNAL_NPC_FACES; iCounter++) {
    DeleteFace(uiExternalStaticNPCFaces[iCounter]);
  }
}

void EmptyDialogueQueue() {
  // If we have anything left in the queue, remove!
  if (ghDialogueQ != NULL) {
    /*
    DEF:  commented out because the Queue system ?? uses a contiguous memory block ??? for the queue
            so you cant delete a single node.  The DeleteQueue, below, will free the entire memory
    block

                    numDialogueItems = QueueSize( ghDialogueQ );

                    for ( cnt = numDialogueItems-1; cnt >= 0; cnt-- )
                    {
                            if ( PeekQueue( ghDialogueQ, &QItem ) )
                            {
                                            MemFree( QItem );
                            }
                    }
    */

    // Delete list
    DeleteQueue(ghDialogueQ);
    ghDialogueQ = NULL;

    // Recreate list
    ghDialogueQ = CreateQueue(INITIAL_Q_SIZE, sizeof(DIALOGUE_Q_STRUCT_PTR));
  }

  gfWaitingForTriggerTimer = FALSE;
}

BOOLEAN DialogueQueueIsEmpty() {
  int32_t numDialogueItems;

  if (ghDialogueQ != NULL) {
    numDialogueItems = QueueSize(ghDialogueQ);

    if (numDialogueItems == 0) {
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN DialogueQueueIsEmptyOrSomebodyTalkingNow() {
  if (gpCurrentTalkingFace != NULL) {
    return (FALSE);
  }

  if (!DialogueQueueIsEmpty()) {
    return (FALSE);
  }

  return (TRUE);
}

void DialogueAdvanceSpeech() {
  // Shut them up!
  InternalShutupaYoFace(gpCurrentTalkingFace->iID, FALSE);
}

void StopAnyCurrentlyTalkingSpeech() {
  // ATE; Make sure guys stop talking....
  if (gpCurrentTalkingFace != NULL) {
    InternalShutupaYoFace(gpCurrentTalkingFace->iID, TRUE);
  }
}

// ATE: Handle changes like when face goes from
// 'external' to on the team panel...
void HandleDialogueUIAdjustments() {
  struct SOLDIERTYPE *pSoldier;

  // OK, check if we are still taking
  if (gpCurrentTalkingFace != NULL) {
    if (gpCurrentTalkingFace->fTalking) {
      // ATE: Check for change in state for the guy currently talking on 'external' panel....
      if (gfFacePanelActive) {
        pSoldier = FindSoldierByProfileID(gubCurrentTalkingID, FALSE);

        if (pSoldier) {
          if (0) {
            // A change in plans here...
            // We now talk through the interface panel...
            if (gpCurrentTalkingFace->iVideoOverlay != -1) {
              RemoveVideoOverlay(gpCurrentTalkingFace->iVideoOverlay);
              gpCurrentTalkingFace->iVideoOverlay = -1;
            }
            gfFacePanelActive = FALSE;

            RemoveVideoOverlay(giTextBoxOverlay);
            giTextBoxOverlay = -1;

            if (fTextBoxMouseRegionCreated) {
              MSYS_RemoveRegion(&gTextBoxMouseRegion);
              fTextBoxMouseRegionCreated = FALSE;
            }

            // Setup UI again!
            CreateTalkingUI(gbUIHandlerID, pSoldier->iFaceIndex, GetSolProfile(pSoldier), pSoldier,
                            gzQuoteStr);
          }
        }
      }
    }
  }
}

void HandleDialogue() {
  int32_t iQSize;
  DIALOGUE_Q_STRUCT *QItem;
  static BOOLEAN fOldEngagedInConvFlagOn = FALSE;
  BOOLEAN fDoneTalking = FALSE;
  struct SOLDIERTYPE *pSoldier = NULL;
  wchar_t zText[512];
  wchar_t zMoney[128];

  // we don't want to just delay action of some events, we want to pause the whole queue, regardless
  // of the event
  if (gfDialogueQueuePaused) {
    return;
  }

  iQSize = QueueSize(ghDialogueQ);

  if (iQSize == 0 && gpCurrentTalkingFace == NULL) {
    HandlePendingInitConv();
  }

  HandleCivQuote();

  // Alrighty, check for a change in state, do stuff appropriately....
  // Turned on
  if (fOldEngagedInConvFlagOn == FALSE && (gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
    // OK, we have just entered...
    fOldEngagedInConvFlagOn = TRUE;

    // pause game..
    PauseGame();
    LockPauseState(14);
  } else if (fOldEngagedInConvFlagOn == TRUE && !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
    // OK, we left...
    fOldEngagedInConvFlagOn = FALSE;

    // Unpause game..
    UnLockPauseState();
    UnPauseGame();

    // if we're exiting boxing with the UI lock set then DON'T OVERRIDE THIS!
    if (!(gTacticalStatus.bBoxingState == WON_ROUND || gTacticalStatus.bBoxingState == LOST_ROUND ||
          gTacticalStatus.bBoxingState == DISQUALIFIED) &&
        !(gTacticalStatus.uiFlags & IGNORE_ENGAGED_IN_CONV_UI_UNLOCK)) {
      guiPendingOverrideEvent = LU_ENDUILOCK;
      HandleTacticalUI();

      // ATE: If this is NOT the player's turn.. engage AI UI lock!
      if (gTacticalStatus.ubCurrentTeam != gbPlayerNum) {
        // Setup locked UI
        guiPendingOverrideEvent = LU_BEGINUILOCK;
        HandleTacticalUI();
      }
    }

    gTacticalStatus.uiFlags &= (~IGNORE_ENGAGED_IN_CONV_UI_UNLOCK);
  }

  if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV) {
    // Are we in here because of the dialogue system up?
    if (!gfInTalkPanel) {
      // ATE: NOT if we have a message box pending....
      if (guiPendingScreen != MSG_BOX_SCREEN && guiCurrentScreen != MSG_BOX_SCREEN) {
        // No, so we should lock the UI!
        guiPendingOverrideEvent = LU_BEGINUILOCK;
        HandleTacticalUI();
      }
    }
  }

  // OK, check if we are still taking
  if (gpCurrentTalkingFace != NULL) {
    if (gpCurrentTalkingFace->fTalking) {
      // ATE: OK, MANAGE THE DISPLAY OF OUR CURRENTLY ACTIVE FACE IF WE / IT CHANGES STATUS
      // THINGS THAT CAN CHANGE STATUS:
      //		CHANGE TO MAPSCREEN
      //		CHANGE TO GAMESCREEN
      //		CHANGE IN MERC STATUS TO BE IN A SQUAD
      //    CHANGE FROM TEAM TO INV INTERFACE

      // Where are we and where did this face once exist?
      if (guiScreenIDUsedWhenUICreated == GAME_SCREEN && IsMapScreen_2()) {
        // GO FROM GAMESCREEN TO MAPSCREEN
        // REMOVE OLD UI
        // Set face inactive!
        // gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
        // SetAutoFaceInActive( gpCurrentTalkingFace->iID );
        // gfFacePanelActive = FALSE;

        // delete face panel if there is one!
        if (gfFacePanelActive) {
          // Set face inactive!
          if (gpCurrentTalkingFace->iVideoOverlay != -1) {
            RemoveVideoOverlay(gpCurrentTalkingFace->iVideoOverlay);
            gpCurrentTalkingFace->iVideoOverlay = -1;
          }

          if (fExternFaceBoxRegionCreated) {
            fExternFaceBoxRegionCreated = FALSE;
            MSYS_RemoveRegion(&(gFacePopupMouseRegion));
          }

          // Set face inactive....
          gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
          SetAutoFaceInActive(gpCurrentTalkingFace->iID);
          HandleTacticalSpeechUI(gubCurrentTalkingID, gpCurrentTalkingFace->iID);

          // ATE: Force mapscreen to set face active again.....
          fReDrawFace = TRUE;
          DrawFace(bSelectedInfoChar);

          gfFacePanelActive = FALSE;
        }

        guiScreenIDUsedWhenUICreated = guiCurrentScreen;
      } else if (guiScreenIDUsedWhenUICreated == MAP_SCREEN && IsTacticalMode()) {
        HandleTacticalSpeechUI(gubCurrentTalkingID, gpCurrentTalkingFace->iID);
        guiScreenIDUsedWhenUICreated = guiCurrentScreen;
      }
      return;
    } else {
      // Check special flags
      // If we are done, check special face flag for trigger NPC!
      if (gpCurrentTalkingFace->uiFlags & FACE_PCTRIGGER_NPC) {
        // Decrement refrence count...
        giNPCReferenceCount--;

        TriggerNPCRecord((uint8_t)gpCurrentTalkingFace->uiUserData1,
                         (uint8_t)gpCurrentTalkingFace->uiUserData2);
        // Reset flag!
        gpCurrentTalkingFace->uiFlags &= (~FACE_PCTRIGGER_NPC);
      }

      if (gpCurrentTalkingFace->uiFlags & FACE_MODAL) {
        gpCurrentTalkingFace->uiFlags &= (~FACE_MODAL);

        EndModalTactical();

        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Ending Modal Tactical Quote.");
      }

      if (gpCurrentTalkingFace->uiFlags & FACE_TRIGGER_PREBATTLE_INT) {
        UnLockPauseState();
        InitPreBattleInterface((struct GROUP *)gpCurrentTalkingFace->uiUserData1, TRUE);
        // Reset flag!
        gpCurrentTalkingFace->uiFlags &= (~FACE_TRIGGER_PREBATTLE_INT);
      }

      gpCurrentTalkingFace = NULL;
      gubCurrentTalkingID = NO_PROFILE;
      gTacticalStatus.ubLastQuoteProfileNUm = NO_PROFILE;
      fDoneTalking = TRUE;
    }
  }

  if ((fDoneTalking) && (fWasPausedDuringDialogue)) {
    fWasPausedDuringDialogue = FALSE;

    // unlock pause state
    UnLockPauseState();
    UnPauseGame();
  }

  if (iQSize == 0) {
    if (gfMikeShouldSayHi == TRUE) {
      struct SOLDIERTYPE *pMike;
      int16_t sPlayerGridNo;
      uint8_t ubPlayerID;

      pMike = FindSoldierByProfileID(MIKE, FALSE);
      if (pMike) {
        sPlayerGridNo = ClosestPC(pMike, NULL);
        if (sPlayerGridNo != NOWHERE) {
          ubPlayerID = WhoIsThere2(sPlayerGridNo, 0);
          if (ubPlayerID != NOBODY) {
            InitiateConversation(pMike, MercPtrs[ubPlayerID], NPC_INITIAL_QUOTE, 0);
            gMercProfiles[pMike->ubProfile].ubMiscFlags2 |= PROFILE_MISC_FLAG2_SAID_FIRSTSEEN_QUOTE;
            // JA2Gold: special hack value of 2 to prevent dialogue from coming up more than once
            gfMikeShouldSayHi = 2;
          }
        }
      }
    }

    return;
  }

  // ATE: Remove any civ quotes....
  // ShutDownQuoteBoxIfActive( TRUE );

  // If here, pick current one from queue and play

  // Get new one
  RemfromQueue(ghDialogueQ, &QItem);

  // If we are in auto bandage, ignore any quotes!
  if (gTacticalStatus.fAutoBandageMode) {
    if (QItem->fPauseTime) {
      UnLockPauseState();
      UnPauseGame();
    }

    // Delete memory
    MemFree(QItem);
    return;
  }

  // Check time delay

  // Alrighty, check if this one is to be delayed until we gain control.
  // If so, place it back in!
  if (QItem->fDelayed) {
    // Are we not in our turn and not interrupted
    if (gTacticalStatus.ubCurrentTeam != gbPlayerNum) {
      // Place back in!
      // Add to queue
      ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

      return;
    }
  }

  // ATE: OK: If a battle sound, and delay value was given, set time stamp
  // now...
  if (QItem->uiSpecialEventFlag == DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND) {
    if (QItem->uiSpecialEventData2 != 0) {
      if ((GetJA2Clock() - QItem->iTimeStamp) < QItem->uiSpecialEventData2) {
        // Place back in!
        // Add to queue
        ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

        return;
      }
    }
  }

  // Try to find soldier...
  pSoldier = FindSoldierByProfileID(QItem->ubCharacterNum, TRUE);

  if (pSoldier != NULL) {
    if (SoundIsPlaying(pSoldier->uiBattleSoundID)) {
      // Place back in!
      // Add to queue
      ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

      return;
    }
  }

  if ((IsMapScreen()) && (QItem->uiSpecialEventFlag == 0)) {
    QItem->fPauseTime = TRUE;
  }

  if (QItem->fPauseTime) {
    if (GamePaused() == FALSE) {
      PauseGame();
      LockPauseState(15);
      fWasPausedDuringDialogue = TRUE;
    }
  }

  // Now play first item in queue
  // If it's not a 'special' dialogue event, continue
  if (QItem->uiSpecialEventFlag == 0) {
    if (pSoldier) {
      // wake grunt up to say
      if (pSoldier->fMercAsleep) {
        pSoldier->fMercAsleep = FALSE;

        // refresh map screen
        fCharacterInfoPanelDirty = TRUE;
        fTeamPanelDirty = TRUE;

        // allow them to go back to sleep
        TacticalCharacterDialogueWithSpecialEvent(pSoldier, QItem->usQuoteNum,
                                                  DIALOGUE_SPECIAL_EVENT_SLEEP, 1, 0);
      }
    }

    gTacticalStatus.ubLastQuoteSaid = (uint8_t)QItem->usQuoteNum;
    gTacticalStatus.ubLastQuoteProfileNUm = (uint8_t)QItem->ubCharacterNum;

    // Setup face pointer
    gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
    gubCurrentTalkingID = QItem->ubCharacterNum;

    ExecuteCharacterDialogue(QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex,
                             QItem->bUIHandlerID, QItem->fFromSoldier);

  } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SKIP_A_FRAME) {
  } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE) {
    // locking or unlocking?
    if (QItem->uiSpecialEventData) {
      switch (QItem->uiSpecialEventData2) {
        case (MAP_SCREEN):
          fLockOutMapScreenInterface = TRUE;
          break;
      }
    } else {
      switch (QItem->uiSpecialEventData2) {
        case (MAP_SCREEN):
          fLockOutMapScreenInterface = FALSE;
          break;
      }
    }
  } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_REMOVE_EPC) {
    gMercProfiles[(uint8_t)QItem->uiSpecialEventData].ubMiscFlags &=
        ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
    UnRecruitEPC((uint8_t)QItem->uiSpecialEventData);
    ReBuildCharactersList();
  } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_WANTS_TO_RENEW) {
    HandleMercIsWillingToRenew((uint8_t)QItem->uiSpecialEventData);
  } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_NOGO_TO_RENEW) {
    HandleMercIsNotWillingToRenew((uint8_t)QItem->uiSpecialEventData);
  } else {
    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_USE_ALTERNATE_FILES) {
      gfUseAlternateDialogueFile = TRUE;

      // Setup face pointer
      gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
      gubCurrentTalkingID = QItem->ubCharacterNum;

      ExecuteCharacterDialogue(QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex,
                               QItem->bUIHandlerID, QItem->fFromSoldier);

      gfUseAlternateDialogueFile = FALSE;

    }
    // We could have a special flag, but dialogue as well
    else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC) {
      // Setup face pointer
      gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
      gubCurrentTalkingID = QItem->ubCharacterNum;

      ExecuteCharacterDialogue(QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex,
                               QItem->bUIHandlerID, QItem->fFromSoldier);

      // Setup face with data!
      gpCurrentTalkingFace->uiFlags |= FACE_PCTRIGGER_NPC;
      gpCurrentTalkingFace->uiUserData1 = QItem->uiSpecialEventData;
      gpCurrentTalkingFace->uiUserData2 = QItem->uiSpecialEventData2;

    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOW_CONTRACT_MENU) {
      // Setup face pointer
      // ATE: THis is working with MARK'S STUFF :(
      // Need this stuff so that bSelectedInfoChar is set...
      SetInfoChar(pSoldier->ubID);

      fShowContractMenu = TRUE;
      RebuildContractBoxForMerc(pSoldier);
      bSelectedContractChar = bSelectedInfoChar;
      pProcessingSoldier = pSoldier;
      fProcessingAMerc = TRUE;
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND) {
      // grab soldier ptr from profile ID
      pSoldier = FindSoldierByProfileID(QItem->ubCharacterNum, FALSE);

      // Do battle snounds......
      if (pSoldier) {
        InternalDoMercBattleSound(pSoldier, (uint8_t)QItem->uiSpecialEventData, 0);
      }
    }

    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START) {
      // Turn off item lock for locators...
      gTacticalStatus.fLockItemLocators = FALSE;

      // Slide to location!
      SlideToLocation(0, (uint16_t)QItem->uiSpecialEventData);

      gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
      gubCurrentTalkingID = QItem->ubCharacterNum;

      ExecuteCharacterDialogue(QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex,
                               QItem->bUIHandlerID, QItem->fFromSoldier);
    }

    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_ENABLE_AI) {
      // OK, allow AI to work now....
      UnPauseAI();
    }

    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_TRIGGERPREBATTLEINTERFACE) {
      UnLockPauseState();
      InitPreBattleInterface((struct GROUP *)QItem->uiSpecialEventData, TRUE);
    }
    if (QItem->uiSpecialEventFlag & DIALOGUE_ADD_EVENT_FOR_SOLDIER_UPDATE_BOX) {
      int32_t iReason = 0;
      struct SOLDIERTYPE *pUpdateSoldier = NULL;

      iReason = QItem->uiSpecialEventData;

      switch (iReason) {
        case (UPDATE_BOX_REASON_ADDSOLDIER):
          pUpdateSoldier = GetSoldierByID(QItem->uiSpecialEventData2);
          if (pUpdateSoldier->bActive == TRUE) {
            AddSoldierToUpdateBox(pUpdateSoldier);
          }
          break;
        case (UPDATE_BOX_REASON_SET_REASON):
          SetSoldierUpdateBoxReason(QItem->uiSpecialEventData2);
          break;
        case (UPDATE_BOX_REASON_SHOW_BOX):
          ShowUpdateBox();
          break;
      }
    }
    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_BEGINPREBATTLEINTERFACE) {
      // Setup face pointer
      gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
      gubCurrentTalkingID = QItem->ubCharacterNum;

      ExecuteCharacterDialogue(QItem->ubCharacterNum, QItem->usQuoteNum, QItem->iFaceIndex,
                               QItem->bUIHandlerID, QItem->fFromSoldier);

      // Setup face with data!
      gpCurrentTalkingFace->uiFlags |= FACE_TRIGGER_PREBATTLE_INT;
      gpCurrentTalkingFace->uiUserData1 = QItem->uiSpecialEventData;
      gpCurrentTalkingFace->uiUserData2 = QItem->uiSpecialEventData2;
    }

    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOPKEEPER) {
      if (QItem->uiSpecialEventData < 3) {
        // post a notice if the player wants to withdraw money from thier account to cover the
        // difference?
        swprintf(zMoney, ARR_SIZE(zMoney), L"%d", QItem->uiSpecialEventData2);
        InsertCommasForDollarFigure(zMoney);
        InsertDollarSignInToString(zMoney);
      }

      switch (QItem->uiSpecialEventData) {
        case (0):
          swprintf(zText, ARR_SIZE(zText), SkiMessageBoxText[SKI_SHORT_FUNDS_TEXT], zMoney);

          // popup a message stating the player doesnt have enough money
          DoSkiMessageBox(MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK,
                          ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack);
          break;
        case (1):
          // if the player is trading items
          swprintf(zText, ARR_SIZE(zText),
                   SkiMessageBoxText
                       [SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_DIFFERENCE],
                   zMoney);

          // ask them if we should deduct money out the players account to cover the difference
          DoSkiMessageBox(MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO,
                          ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack);

          break;
        case (2):
          swprintf(
              zText, ARR_SIZE(zText),
              SkiMessageBoxText[SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_COST],
              zMoney);

          // ask them if we should deduct money out the players account to cover the difference
          DoSkiMessageBox(MSG_BOX_BASIC_STYLE, zText, SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO,
                          ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack);
          break;
        case (3):
          // this means a dialogue event is in progress
          giShopKeepDialogueEventinProgress = QItem->uiSpecialEventData2;
          break;
        case (4):
          // this means a dialogue event has ended
          giShopKeepDialogueEventinProgress = -1;
          break;
        case (5):
          // this means a dialogue event has ended
          gfSKIScreenExit = TRUE;
          break;

        case (6):
          if (guiCurrentScreen == SHOPKEEPER_SCREEN) {
            DisableButton(guiSKI_TransactionButton);
          }
          break;
        case (7):
          if (guiCurrentScreen == SHOPKEEPER_SCREEN) {
            EnableButton(guiSKI_TransactionButton);
          }
          break;
      }
    }

    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_EXIT_MAP_SCREEN) {
      // select sector
      ChangeSelectedMapSector((uint8_t)QItem->uiSpecialEventData,
                              (uint8_t)QItem->uiSpecialEventData2,
                              (int8_t)QItem->uiSpecialEventData3);
      RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DISPLAY_STAT_CHANGE) {
      // grab soldier ptr from profile ID
      pSoldier = FindSoldierByProfileID(QItem->ubCharacterNum, FALSE);

      if (pSoldier) {
        wchar_t wTempString[128];

        // tell player about stat increase
        BuildStatChangeString(
            wTempString, ARR_SIZE(wTempString), pSoldier->name, (BOOLEAN)QItem->uiSpecialEventData,
            (int16_t)QItem->uiSpecialEventData2, (uint8_t)QItem->uiSpecialEventData3);
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, wTempString);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_UNSET_ARRIVES_FLAG) {
      gTacticalStatus.bMercArrivingQuoteBeingUsed = FALSE;
    }

    /*
    else if( QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DISPLAY_INVASION_MESSAGE )
    {
            HandlePlayerNotifyInvasionByEnemyForces( (int16_t)(QItem->uiSpecialEventData %
    MAP_WORLD_X), (int16_t)(SectorID16_Y(QItem->uiSpecialEventData)), 0, NULL );
    }
    */
    else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SKYRIDERMAPSCREENEVENT) {
      // Setup face pointer
      gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
      gubCurrentTalkingID = QItem->ubCharacterNum;

      // handle the monologue event
      HandleSkyRiderMonologueEvent(QItem->uiSpecialEventData, QItem->uiSpecialEventData2);
    }

    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT) {
      // Setup face pointer
      gpCurrentTalkingFace = &gFacesData[QItem->iFaceIndex];
      gubCurrentTalkingID = QItem->ubCharacterNum;

      // set up the mine highlgith events
      SetUpAnimationOfMineSectors(QItem->uiSpecialEventData);
    }

    // Switch on our special events
    if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_GIVE_ITEM) {
      if (QItem->bUIHandlerID == DIALOGUE_NPC_UI) {
        HandleNPCItemGiven((uint8_t)QItem->uiSpecialEventData,
                           (struct OBJECTTYPE *)QItem->uiSpecialEventData2,
                           (int8_t)QItem->uiSpecialEventData3);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_TRIGGER_NPC) {
      if (QItem->bUIHandlerID == DIALOGUE_NPC_UI) {
        HandleNPCTriggerNPC((uint8_t)QItem->uiSpecialEventData, (uint8_t)QItem->uiSpecialEventData2,
                            (BOOLEAN)QItem->uiSpecialEventData3,
                            (uint8_t)QItem->uiSpecialEventData4);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_GOTO_GRIDNO) {
      if (QItem->bUIHandlerID == DIALOGUE_NPC_UI) {
        HandleNPCGotoGridNo((uint8_t)QItem->uiSpecialEventData,
                            (uint16_t)QItem->uiSpecialEventData2,
                            (uint8_t)QItem->uiSpecialEventData3);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_DO_ACTION) {
      if (QItem->bUIHandlerID == DIALOGUE_NPC_UI) {
        HandleNPCDoAction((uint8_t)QItem->uiSpecialEventData, (uint16_t)QItem->uiSpecialEventData2,
                          (uint8_t)QItem->uiSpecialEventData3);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CLOSE_PANEL) {
      if (QItem->bUIHandlerID == DIALOGUE_NPC_UI) {
        HandleNPCClosePanel();
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SHOW_UPDATE_MENU) {
      SetUpdateBoxFlag(TRUE);
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTINUE_TRAINING_MILITIA) {
      // grab soldier ptr from profile ID
      pSoldier = FindSoldierByProfileID((uint8_t)(QItem->uiSpecialEventData), FALSE);

      // if soldier valid...
      if (pSoldier != NULL) {
        HandleInterfaceMessageForContinuingTrainingMilitia(pSoldier);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_ENTER_MAPSCREEN) {
      if (!(IsMapScreen())) {
        gfEnteringMapScreen = TRUE;
        fEnterMapDueToContract = TRUE;
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING) {
      // grab soldier ptr from profile ID
      pSoldier = FindSoldierByProfileID(QItem->ubCharacterNum, FALSE);

      // if soldier valid...
      if (pSoldier != NULL) {
        // .. remove the fired soldier again
        BeginStrategicRemoveMerc(pSoldier, (uint8_t)QItem->uiSpecialEventData);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP) {
      // grab soldier ptr from profile ID
      pSoldier = FindSoldierByProfileID(QItem->ubCharacterNum, FALSE);

      // if soldier valid...
      if (pSoldier != NULL) {
        // .. remove the fired soldier again
        StrategicRemoveMerc(pSoldier);
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_MULTIPURPOSE) {
      if (QItem->uiSpecialEventData & MULTIPURPOSE_SPECIAL_EVENT_DONE_KILLING_DEIDRANNA) {
        HandleDoneLastKilledQueenQuote();
      } else if (QItem->uiSpecialEventData & MULTIPURPOSE_SPECIAL_EVENT_TEAM_MEMBERS_DONE_TALKING) {
        HandleDoneLastEndGameQuote();
      }
    } else if (QItem->uiSpecialEventFlag & DIALOGUE_SPECIAL_EVENT_SLEEP) {
      // no soldier, leave now
      if (pSoldier == NULL) {
        return;
      }

      // wake merc up or put them back down?
      if (QItem->uiSpecialEventData == 1) {
        pSoldier->fMercAsleep = TRUE;
      } else {
        pSoldier->fMercAsleep = FALSE;
      }

      // refresh map screen
      fCharacterInfoPanelDirty = TRUE;
      fTeamPanelDirty = TRUE;
    }
  }

  // grab soldier ptr from profile ID
  pSoldier = FindSoldierByProfileID(QItem->ubCharacterNum, FALSE);

  if (pSoldier && pSoldier->bTeam == gbPlayerNum) {
    CheckForStopTimeQuotes(QItem->usQuoteNum);
  }

  if (QItem->fPauseTime) {
    fWasPausedDuringDialogue = TRUE;
  }

  // Delete memory
  MemFree(QItem);
}

BOOLEAN GetDialogue(uint8_t ubCharacterNum, uint16_t usQuoteNum, uint32_t iDataSize,
                    wchar_t *zDialogueText, int zDialogueTextSize, uint32_t *puiSoundID,
                    char *zSoundString);

BOOLEAN DelayedTacticalCharacterDialogue(struct SOLDIERTYPE *pSoldier, uint16_t usQuoteNum) {
  if (GetSolProfile(pSoldier) == NO_PROFILE) {
    return (FALSE);
  }

  if (pSoldier->bLife < CONSCIOUSNESS) return (FALSE);

  if (pSoldier->uiStatusFlags & SOLDIER_GASSED) return (FALSE);

  if ((AM_A_ROBOT(pSoldier))) {
    return (FALSE);
  }

  if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED) return (FALSE);

  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    return (FALSE);
  }

  return (CharacterDialogue(GetSolProfile(pSoldier), usQuoteNum, pSoldier->iFaceIndex,
                            DIALOGUE_TACTICAL_UI, TRUE, TRUE));
}

BOOLEAN TacticalCharacterDialogueWithSpecialEvent(struct SOLDIERTYPE *pSoldier, uint16_t usQuoteNum,
                                                  uint32_t uiFlag, uintptr_t uiData1,
                                                  uint32_t uiData2) {
  if (GetSolProfile(pSoldier) == NO_PROFILE) {
    return (FALSE);
  }

  if (uiFlag != DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND && uiData1 != BATTLE_SOUND_DIE1) {
    if (pSoldier->bLife < CONSCIOUSNESS) return (FALSE);

    if (pSoldier->uiStatusFlags & SOLDIER_GASSED) return (FALSE);
  }

  return (CharacterDialogueWithSpecialEvent(GetSolProfile(pSoldier), usQuoteNum,
                                            pSoldier->iFaceIndex, DIALOGUE_TACTICAL_UI, TRUE, FALSE,
                                            uiFlag, uiData1, uiData2));
}

BOOLEAN TacticalCharacterDialogueWithSpecialEventEx(struct SOLDIERTYPE *pSoldier,
                                                    uint16_t usQuoteNum, uint32_t uiFlag,
                                                    uint32_t uiData1, uint32_t uiData2,
                                                    uint32_t uiData3) {
  if (GetSolProfile(pSoldier) == NO_PROFILE) {
    return (FALSE);
  }

  if (uiFlag != DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND && uiData1 != BATTLE_SOUND_DIE1) {
    if (pSoldier->bLife < CONSCIOUSNESS) return (FALSE);

    if (pSoldier->uiStatusFlags & SOLDIER_GASSED) return (FALSE);

    if ((AM_A_ROBOT(pSoldier))) {
      return (FALSE);
    }

    if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED) return (FALSE);

    if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
      return (FALSE);
    }
  }

  return (CharacterDialogueWithSpecialEventEx(GetSolProfile(pSoldier), usQuoteNum,
                                              pSoldier->iFaceIndex, DIALOGUE_TACTICAL_UI, TRUE,
                                              FALSE, uiFlag, uiData1, uiData2, uiData3));
}

BOOLEAN TacticalCharacterDialogue(struct SOLDIERTYPE *pSoldier, uint16_t usQuoteNum) {
  if (GetSolProfile(pSoldier) == NO_PROFILE) {
    return (FALSE);
  }

  if (AreInMeanwhile()) {
    return (FALSE);
  }

  if (pSoldier->bLife < CONSCIOUSNESS) return (FALSE);

  if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED) return (FALSE);

  if (pSoldier->uiStatusFlags & SOLDIER_GASSED) return (FALSE);

  if ((AM_A_ROBOT(pSoldier))) {
    return (FALSE);
  }

  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    return (FALSE);
  }

  // OK, let's check if this is the exact one we just played, if so, skip.
  if (GetSolProfile(pSoldier) == gTacticalStatus.ubLastQuoteProfileNUm &&
      usQuoteNum == gTacticalStatus.ubLastQuoteSaid) {
    return (FALSE);
  }

  // If we are a robot, play the controller's quote!
  if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
    if (CanRobotBeControlled(pSoldier)) {
      return (TacticalCharacterDialogue(MercPtrs[pSoldier->ubRobotRemoteHolderID], usQuoteNum));
    } else {
      return (FALSE);
    }
  }

  if (AM_AN_EPC(pSoldier) &&
      !(gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE))
    return (FALSE);

  // Check for logging of me too bleeds...
  if (usQuoteNum == QUOTE_STARTING_TO_BLEED) {
    if (gubLogForMeTooBleeds) {
      // If we are greater than one...
      if (gubLogForMeTooBleeds > 1) {
        // Replace with me too....
        usQuoteNum = QUOTE_ME_TOO;
      }
      gubLogForMeTooBleeds++;
    }
  }

  return (CharacterDialogue(GetSolProfile(pSoldier), usQuoteNum, pSoldier->iFaceIndex,
                            DIALOGUE_TACTICAL_UI, TRUE, FALSE));
}

// This function takes a profile num, quote num, faceindex and a UI hander ID.
// What it does is queues up the dialog to be ultimately loaded/displayed
//				FACEINDEX
//						The face index is an index into an ACTIVE face. The
// face is
// considered to 						be active, and if it's not, either
// that has to be
// handled by the UI handler 						ir nothing will show.  What
// this function does is set the face to talking, 						and
// the face sprite system should handle the rest. 				bUIHandlerID Because
// this could be used in any place, the UI handleID is used to differentiate places in the game. For
// example, specific things happen in the tactical engine that may not be the place where in the AIM
// contract screen uses.....

// NB;				The queued system is not yet implemented, but will be transpatent to
// the caller....

BOOLEAN CharacterDialogueWithSpecialEvent(uint8_t ubCharacterNum, uint16_t usQuoteNum,
                                          int32_t iFaceIndex, uint8_t bUIHandlerID,
                                          BOOLEAN fFromSoldier, BOOLEAN fDelayed, uint32_t uiFlag,
                                          uintptr_t uiData1, uint32_t uiData2) {
  DIALOGUE_Q_STRUCT *QItem;

  // Allocate new item
  QItem = (DIALOGUE_Q_STRUCT *)MemAlloc(sizeof(DIALOGUE_Q_STRUCT));
  memset(QItem, 0, sizeof(DIALOGUE_Q_STRUCT));

  QItem->ubCharacterNum = ubCharacterNum;
  QItem->usQuoteNum = usQuoteNum;
  QItem->iFaceIndex = iFaceIndex;
  QItem->bUIHandlerID = bUIHandlerID;
  QItem->iTimeStamp = GetJA2Clock();
  QItem->fFromSoldier = fFromSoldier;
  QItem->fDelayed = fDelayed;

  // Set flag for special event
  QItem->uiSpecialEventFlag = uiFlag;
  QItem->uiSpecialEventData = uiData1;
  QItem->uiSpecialEventData2 = uiData2;

  // Add to queue
  ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

  if (uiFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC) {
    // Increment refrence count...
    giNPCReferenceCount++;
  }

  return (TRUE);
}

BOOLEAN CharacterDialogueWithSpecialEventEx(uint8_t ubCharacterNum, uint16_t usQuoteNum,
                                            int32_t iFaceIndex, uint8_t bUIHandlerID,
                                            BOOLEAN fFromSoldier, BOOLEAN fDelayed, uint32_t uiFlag,
                                            uint32_t uiData1, uint32_t uiData2, uint32_t uiData3) {
  DIALOGUE_Q_STRUCT *QItem;

  // Allocate new item
  QItem = (DIALOGUE_Q_STRUCT *)MemAlloc(sizeof(DIALOGUE_Q_STRUCT));
  memset(QItem, 0, sizeof(DIALOGUE_Q_STRUCT));

  QItem->ubCharacterNum = ubCharacterNum;
  QItem->usQuoteNum = usQuoteNum;
  QItem->iFaceIndex = iFaceIndex;
  QItem->bUIHandlerID = bUIHandlerID;
  QItem->iTimeStamp = GetJA2Clock();
  QItem->fFromSoldier = fFromSoldier;
  QItem->fDelayed = fDelayed;

  // Set flag for special event
  QItem->uiSpecialEventFlag = uiFlag;
  QItem->uiSpecialEventData = uiData1;
  QItem->uiSpecialEventData2 = uiData2;
  QItem->uiSpecialEventData3 = uiData3;

  // Add to queue
  ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

  if (uiFlag & DIALOGUE_SPECIAL_EVENT_PCTRIGGERNPC) {
    // Increment refrence count...
    giNPCReferenceCount++;
  }

  return (TRUE);
}

BOOLEAN CharacterDialogue(uint8_t ubCharacterNum, uint16_t usQuoteNum, int32_t iFaceIndex,
                          uint8_t bUIHandlerID, BOOLEAN fFromSoldier, BOOLEAN fDelayed) {
  DIALOGUE_Q_STRUCT *QItem;

  // Allocate new item
  QItem = (DIALOGUE_Q_STRUCT *)MemAlloc(sizeof(DIALOGUE_Q_STRUCT));
  memset(QItem, 0, sizeof(DIALOGUE_Q_STRUCT));

  QItem->ubCharacterNum = ubCharacterNum;
  QItem->usQuoteNum = usQuoteNum;
  QItem->iFaceIndex = iFaceIndex;
  QItem->bUIHandlerID = bUIHandlerID;
  QItem->iTimeStamp = GetJA2Clock();
  QItem->fFromSoldier = fFromSoldier;
  QItem->fDelayed = fDelayed;

  // check if pause already locked, if so, then don't mess with it
  if (gfLockPauseState == FALSE) {
    QItem->fPauseTime = fPausedTimeDuringQuote;
  }

  fPausedTimeDuringQuote = FALSE;

  // Add to queue
  ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

  return (TRUE);
}

BOOLEAN SpecialCharacterDialogueEvent(uintptr_t uiSpecialEventFlag, uintptr_t uiSpecialEventData1,
                                      uintptr_t uiSpecialEventData2, uint32_t uiSpecialEventData3,
                                      int32_t iFaceIndex, uint8_t bUIHandlerID) {
  DIALOGUE_Q_STRUCT *QItem;

  // Allocate new item
  QItem = (DIALOGUE_Q_STRUCT *)MemAlloc(sizeof(DIALOGUE_Q_STRUCT));
  memset(QItem, 0, sizeof(DIALOGUE_Q_STRUCT));

  QItem->uiSpecialEventFlag = uiSpecialEventFlag;
  QItem->uiSpecialEventData = uiSpecialEventData1;
  QItem->uiSpecialEventData2 = uiSpecialEventData2;
  QItem->uiSpecialEventData3 = uiSpecialEventData3;
  QItem->iFaceIndex = iFaceIndex;
  QItem->bUIHandlerID = bUIHandlerID;
  QItem->iTimeStamp = GetJA2Clock();

  // if paused state not already locked
  if (gfLockPauseState == FALSE) {
    QItem->fPauseTime = fPausedTimeDuringQuote;
  }

  fPausedTimeDuringQuote = FALSE;

  // Add to queue
  ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

  return (TRUE);
}

BOOLEAN SpecialCharacterDialogueEventWithExtraParam(uint32_t uiSpecialEventFlag,
                                                    uint32_t uiSpecialEventData1,
                                                    uint32_t uiSpecialEventData2,
                                                    uint32_t uiSpecialEventData3,
                                                    uint32_t uiSpecialEventData4,
                                                    int32_t iFaceIndex, uint8_t bUIHandlerID) {
  DIALOGUE_Q_STRUCT *QItem;

  // Allocate new item
  QItem = (DIALOGUE_Q_STRUCT *)MemAlloc(sizeof(DIALOGUE_Q_STRUCT));
  memset(QItem, 0, sizeof(DIALOGUE_Q_STRUCT));

  QItem->uiSpecialEventFlag = uiSpecialEventFlag;
  QItem->uiSpecialEventData = uiSpecialEventData1;
  QItem->uiSpecialEventData2 = uiSpecialEventData2;
  QItem->uiSpecialEventData3 = uiSpecialEventData3;
  QItem->uiSpecialEventData4 = uiSpecialEventData4;
  QItem->iFaceIndex = iFaceIndex;
  QItem->bUIHandlerID = bUIHandlerID;
  QItem->iTimeStamp = GetJA2Clock();

  // if paused state not already locked
  if (gfLockPauseState == FALSE) {
    QItem->fPauseTime = fPausedTimeDuringQuote;
  }

  fPausedTimeDuringQuote = FALSE;

  // Add to queue
  ghDialogueQ = AddtoQueue(ghDialogueQ, &QItem);

  return (TRUE);
}

BOOLEAN ExecuteCharacterDialogue(uint8_t ubCharacterNum, uint16_t usQuoteNum, int32_t iFaceIndex,
                                 uint8_t bUIHandlerID, BOOLEAN fFromSoldier) {
  char zSoundString[164];
  uint32_t uiSoundID;
  struct SOLDIERTYPE *pSoldier;

  // Check if we are dead now or not....( if from a soldier... )

  // Try to find soldier...
  pSoldier = FindSoldierByProfileID(ubCharacterNum, TRUE);

  if (pSoldier != NULL) {
    // Check vital stats
    if (pSoldier->bLife < CONSCIOUSNESS) {
      return (FALSE);
    }

    if (pSoldier->uiStatusFlags & SOLDIER_GASSED) return (FALSE);

    if ((AM_A_ROBOT(pSoldier))) {
      return (FALSE);
    }

    if (pSoldier->bLife < OKLIFE && usQuoteNum != QUOTE_SERIOUSLY_WOUNDED) {
      return (FALSE);
    }

    if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
      return (FALSE);
    }

    // sleeping guys don't talk.. go to standby to talk
    if (pSoldier->fMercAsleep == TRUE) {
      // check if the soldier was compaining about lack of sleep and was alseep, if so, leave them
      // alone
      if ((usQuoteNum == QUOTE_NEED_SLEEP) || (usQuoteNum == QUOTE_OUT_OF_BREATH)) {
        // leave them alone
        return (TRUE);
      }

      // may want to wake up any character that has VERY important dialogue to say
      // MC to flesh out
    }
  } else {
    // If from a soldier, and he does not exist anymore, donot play!
    if (fFromSoldier) {
      return (FALSE);
    }
  }

  // Check face index
  CHECKF(iFaceIndex != -1);

  if (!GetDialogue(ubCharacterNum, usQuoteNum, DIALOGUESIZE, gzQuoteStr, ARR_SIZE(gzQuoteStr),
                   &uiSoundID, zSoundString)) {
    return (FALSE);
  }

  if (bUIHandlerID == DIALOGUE_EXTERNAL_NPC_UI) {
    // external NPC
    SetFaceTalking(iFaceIndex, zSoundString, gzQuoteStr, RATE_11025, 30, 1, MIDDLEPAN);
  } else {
    // start "talking" system (portrait animation and start wav sample)
    SetFaceTalking(iFaceIndex, zSoundString, gzQuoteStr, RATE_11025, 30, 1, MIDDLEPAN);
  }
  // pSoldier can be null here... ( if NOT from an alive soldier )
  CreateTalkingUI(bUIHandlerID, iFaceIndex, ubCharacterNum, pSoldier, gzQuoteStr);

  // Set global handleer ID value, used when face desides it's done...
  gbUIHandlerID = bUIHandlerID;

  guiScreenIDUsedWhenUICreated = guiCurrentScreen;

  return (TRUE);
}

void CreateTalkingUI(int8_t bUIHandlerID, int32_t iFaceIndex, uint8_t ubCharacterNum,
                     struct SOLDIERTYPE *pSoldier, wchar_t *zQuoteStr) {
  // Show text, if on
  if (gGameSettings.fOptions[TOPTION_SUBTITLES] || !gFacesData[iFaceIndex].fValidSpeech) {
    switch (bUIHandlerID) {
      case DIALOGUE_TACTICAL_UI:

        HandleTacticalTextUI(iFaceIndex, pSoldier, zQuoteStr);
        break;

      case DIALOGUE_NPC_UI:

        HandleTacticalNPCTextUI(ubCharacterNum, zQuoteStr);
        break;

      case DIALOGUE_CONTACTPAGE_UI:
        DisplayTextForMercFaceVideoPopUp(zQuoteStr);
        break;

      case DIALOGUE_SPECK_CONTACT_PAGE_UI:
        DisplayTextForSpeckVideoPopUp(zQuoteStr);
        break;
      case DIALOGUE_EXTERNAL_NPC_UI:

        DisplayTextForExternalNPC(ubCharacterNum, zQuoteStr);
        break;

      case DIALOGUE_SHOPKEEPER_UI:
        InitShopKeeperSubTitledText(zQuoteStr);
        break;
    }
  }

  if (gGameSettings.fOptions[TOPTION_SPEECH]) {
    switch (bUIHandlerID) {
      case DIALOGUE_TACTICAL_UI:

        HandleTacticalSpeechUI(ubCharacterNum, iFaceIndex);
        break;

      case DIALOGUE_CONTACTPAGE_UI:
        break;

      case DIALOGUE_SPECK_CONTACT_PAGE_UI:
        break;
      case DIALOGUE_EXTERNAL_NPC_UI:
        HandleExternNPCSpeechFace(iFaceIndex);
        break;
    }
  }
}

char *GetDialogueDataFilename(uint8_t ubCharacterNum, uint16_t usQuoteNum, BOOLEAN fWavFile) {
  static char zFileName[164];
  uint8_t ubFileNumID;

  // Are we an NPC OR an RPC that has not been recruited?
  // ATE: Did the || clause here to allow ANY RPC that talks while the talking menu is up to use an
  // npc quote file
  if (gfUseAlternateDialogueFile) {
    if (fWavFile) {
      // build name of wav file (characternum + quotenum)
      sprintf(zFileName, "NPC_SPEECH\\d_%03d_%03d.wav", ubCharacterNum, usQuoteNum);
    } else {
      // assume EDT files are in EDT directory on HARD DRIVE
      sprintf(zFileName, "NPCDATA\\d_%03d.EDT", ubCharacterNum);
    }
  } else if (ubCharacterNum >= FIRST_RPC &&
             (!(gMercProfiles[ubCharacterNum].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) ||
              ProfileCurrentlyTalkingInDialoguePanel(ubCharacterNum) ||
              (gMercProfiles[ubCharacterNum].ubMiscFlags & PROFILE_MISC_FLAG_FORCENPCQUOTE))) {
    ubFileNumID = ubCharacterNum;

    // ATE: If we are merc profile ID #151-154, all use 151's data....
    if (ubCharacterNum >= 151 && ubCharacterNum <= 154) {
      ubFileNumID = 151;
    }

    // If we are character #155, check fact!
    if (ubCharacterNum == 155 && !gubFact[220]) {
      ubFileNumID = 155;
    }

    if (fWavFile) {
      sprintf(zFileName, "NPC_SPEECH\\%03d_%03d.wav", ubFileNumID, usQuoteNum);
    } else {
      // assume EDT files are in EDT directory on HARD DRIVE
      sprintf(zFileName, "NPCDATA\\%03d.EDT", ubFileNumID);
    }
  } else {
    if (fWavFile) {
      // build name of wav file (characternum + quotenum)
      if ((UsingRussianBukaResources() || UsingRussianGoldResources()) &&
          ubCharacterNum >= FIRST_RPC &&
          gMercProfiles[ubCharacterNum].ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED) {
        sprintf(zFileName, "SPEECH\\r_%03d_%03d.wav", ubCharacterNum, usQuoteNum);
      } else {
        sprintf(zFileName, "SPEECH\\%03d_%03d.wav", ubCharacterNum, usQuoteNum);
      }
    } else {
      // assume EDT files are in EDT directory on HARD DRIVE
      sprintf(zFileName, "MERCEDT\\%03d.EDT", ubCharacterNum);
    }
  }

  return (zFileName);
}

// Used to see if the dialog text file exists
BOOLEAN DialogueDataFileExistsForProfile(uint8_t ubCharacterNum, uint16_t usQuoteNum,
                                         BOOLEAN fWavFile, char **ppStr) {
  char *pFilename;

  pFilename = GetDialogueDataFilename(ubCharacterNum, usQuoteNum, fWavFile);

  if (ppStr) {
    (*ppStr) = pFilename;
  }

  return (FileMan_Exists(pFilename));
}

BOOLEAN GetDialogue(uint8_t ubCharacterNum, uint16_t usQuoteNum, uint32_t iDataSize,
                    wchar_t *zDialogueText, int zDialogueTextSize, uint32_t *puiSoundID,
                    char *zSoundString) {
  char *pFilename;

  // first things first  - grab the text (if player has SUBTITLE PREFERENCE ON)
  // if ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
  {
    if (DialogueDataFileExistsForProfile(ubCharacterNum, 0, FALSE, &pFilename)) {
      LoadEncryptedDataFromFile(pFilename, zDialogueText, usQuoteNum * iDataSize, iDataSize);
      if (zDialogueText[0] == 0) {
        swprintf(zDialogueText, zDialogueTextSize, L"I have no text in the EDT file ( %d ) %S",
                 usQuoteNum, pFilename);

#ifndef JA2BETAVERSION
        return (FALSE);
#endif
      }
    } else {
      swprintf(zDialogueText, zDialogueTextSize, L"I have no text in the file ( %d ) %S",
               usQuoteNum, pFilename);

#ifndef JA2BETAVERSION
      return (FALSE);
#endif
    }
  }

  // CHECK IF THE FILE EXISTS, IF NOT, USE DEFAULT!
  pFilename = GetDialogueDataFilename(ubCharacterNum, usQuoteNum, TRUE);

  // Copy
  strcpy(zSoundString, pFilename);

  // get speech if applicable
  if (gGameSettings.fOptions[TOPTION_SPEECH]) {
    // Load it into memory!
    *puiSoundID = SoundLoadSample(pFilename);
  }

  return (TRUE);
}

// Handlers for tactical UI stuff
void HandleTacticalNPCTextUI(uint8_t ubCharacterNum, wchar_t *zQuoteStr) {
  wchar_t zText[QUOTE_MESSAGE_SIZE];

  // Setup dialogue text box
  if (!IsMapScreen_2()) {
    gTalkPanel.fRenderSubTitlesNow = TRUE;
    gTalkPanel.fSetupSubTitles = TRUE;
  }

  // post message to mapscreen message system
#ifdef TAIWANESE
  swprintf(gTalkPanel.zQuoteStr, ARR_SIZE(gTalkPanel.zQuoteStr), L"%s", zQuoteStr);
#else
  swprintf(gTalkPanel.zQuoteStr, ARR_SIZE(gTalkPanel.zQuoteStr), L"\"%s\"", zQuoteStr);
  swprintf(zText, ARR_SIZE(zText), L"%s: \"%s\"", gMercProfiles[ubCharacterNum].zNickname,
           zQuoteStr);
  MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%s", zText);
#endif
}

// Handlers for tactical UI stuff
void DisplayTextForExternalNPC(uint8_t ubCharacterNum, wchar_t *zQuoteStr) {
  wchar_t zText[QUOTE_MESSAGE_SIZE];
  int16_t sLeft;

  // Setup dialogue text box
  if (!IsMapScreen_2()) {
    gTalkPanel.fRenderSubTitlesNow = TRUE;
    gTalkPanel.fSetupSubTitles = TRUE;
  }

  // post message to mapscreen message system
#ifdef TAIWANESE
  swprintf(gTalkPanel.zQuoteStr, ARR_SIZE(gTalkPanel.zQuoteStr), L"%s", zQuoteStr);
#else
  swprintf(gTalkPanel.zQuoteStr, ARR_SIZE(gTalkPanel.zQuoteStr), L"\"%s\"", zQuoteStr);
  swprintf(zText, ARR_SIZE(zText), L"%s: \"%s\"", gMercProfiles[ubCharacterNum].zNickname,
           zQuoteStr);
  MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%s", zText);
#endif

  if (IsMapScreen_2()) {
    sLeft = (gsExternPanelXPosition + 97);
    gsTopPosition = gsExternPanelYPosition;
  } else {
    sLeft = (110);
  }

  ExecuteTacticalTextBox(sLeft, gTalkPanel.zQuoteStr);

  return;
}

void HandleTacticalTextUI(int32_t iFaceIndex, struct SOLDIERTYPE *pSoldier, wchar_t *zQuoteStr) {
  wchar_t zText[QUOTE_MESSAGE_SIZE];
  int16_t sLeft = 0;

  // BUild text
  // How do we do this with defines?
  // swprintf( zText, L"\xb4\xa2 %s: \xb5 \"%s\"", gMercProfiles[ ubCharacterNum ].zNickname,
  // zQuoteStr );
#ifdef TAIWANESE
  swprintf(zText, ARR_SIZE(zText), L"%s", zQuoteStr);
#else
  swprintf(zText, ARR_SIZE(zText), L"\"%s\"", zQuoteStr);
#endif
  sLeft = 110;

  // previous version
  // sLeft = 110;

  ExecuteTacticalTextBox(sLeft, zText);

#ifndef TAIWANESE
  swprintf(zText, ARR_SIZE(zText), L"%s: \"%s\"", gMercProfiles[GetSolProfile(pSoldier)].zNickname,
           zQuoteStr);
  MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%s", zText);
#endif
}

void ExecuteTacticalTextBoxForLastQuote(int16_t sLeftPosition, wchar_t *pString) {
  uint32_t uiDelay = FindDelayForString(pString);

  fDialogueBoxDueToLastMessage = TRUE;

  guiDialogueLastQuoteTime = GetJA2Clock();

  guiDialogueLastQuoteDelay =
      ((uiDelay < FINAL_TALKING_DURATION) ? FINAL_TALKING_DURATION : uiDelay);

  // now execute box
  ExecuteTacticalTextBox(sLeftPosition, pString);
}

void ExecuteTacticalTextBox(int16_t sLeftPosition, wchar_t *pString) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;

  // check if mouse region created, if so, do not recreate
  if (fTextBoxMouseRegionCreated == TRUE) {
    return;
  }

  memset(&VideoOverlayDesc, 0, sizeof(VIDEO_OVERLAY_DESC));

  // Prepare text box
  SET_USE_WINFONTS(TRUE);
  SET_WINFONT(giSubTitleWinFont);
  iDialogueBox = PrepareMercPopupBox(
      iDialogueBox, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, pString,
      DIALOGUE_DEFAULT_SUBTITLE_WIDTH, 0, 0, 0, &gusSubtitleBoxWidth, &gusSubtitleBoxHeight);
  SET_USE_WINFONTS(FALSE);

  VideoOverlayDesc.sLeft = sLeftPosition;
  VideoOverlayDesc.sTop = gsTopPosition;
  VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + gusSubtitleBoxWidth;
  VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + gusSubtitleBoxHeight;
  VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
  VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
  VideoOverlayDesc.BltCallback = RenderSubtitleBoxOverlay;

  giTextBoxOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);

  gsTopPosition = 20;

  // Define main region
  MSYS_DefineRegion(&gTextBoxMouseRegion, VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,
                    VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
                    CURSOR_NORMAL, MSYS_NO_CALLBACK, TextOverlayClickCallback);
  // Add region
  MSYS_AddRegion(&(gTextBoxMouseRegion));

  fTextBoxMouseRegionCreated = TRUE;
}

void HandleExternNPCSpeechFace(int32_t iIndex) {
  int32_t iFaceIndex;
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  int32_t iFaceOverlay;

  // grab face index
  iFaceIndex = iIndex;

  // Enable it!
  SetAutoFaceActive(NULL, NULL, iFaceIndex, 0, 0);

  // Set flag to say WE control when to set inactive!
  gFacesData[iFaceIndex].uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

  if (!IsMapScreen_2()) {
    // Setup video overlay!
    VideoOverlayDesc.sLeft = 10;
    VideoOverlayDesc.sTop = 20;
    VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + 99;
    VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + 98;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    VideoOverlayDesc.BltCallback = RenderFaceOverlay;
  } else {
    // Setup video overlay!

    VideoOverlayDesc.sLeft = gsExternPanelXPosition;
    VideoOverlayDesc.sTop = gsExternPanelYPosition;

    VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + 99;
    VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + 98;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    VideoOverlayDesc.BltCallback = RenderFaceOverlay;
  }

  iFaceOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
  gpCurrentTalkingFace->iVideoOverlay = iFaceOverlay;

  RenderAutoFace(iFaceIndex);

  // ATE: Create mouse region.......
  if (!fExternFaceBoxRegionCreated) {
    fExternFaceBoxRegionCreated = TRUE;

    // Define main region
    MSYS_DefineRegion(&gFacePopupMouseRegion, VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,
                      VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
                      CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback);
    // Add region
    MSYS_AddRegion(&(gFacePopupMouseRegion));
  }

  gfFacePanelActive = TRUE;

  return;
}

void HandleTacticalSpeechUI(uint8_t ubCharacterNum, int32_t iFaceIndex) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  int32_t iFaceOverlay;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fDoExternPanel = FALSE;

  memset(&VideoOverlayDesc, 0, sizeof(VIDEO_OVERLAY_DESC));

  // Get soldier pointer, if there is one...
  // Try to find soldier...
  pSoldier = FindSoldierByProfileID(ubCharacterNum, FALSE);

  // PLEASE NOTE:  pSoldier may legally be NULL (e.g. Skyrider) !!!

  if (pSoldier == NULL) {
    fDoExternPanel = TRUE;
  } else {
    // If we are not an active face!
    if (!IsMapScreen_2()) {
      fDoExternPanel = TRUE;
    }
  }

  if (fDoExternPanel) {
    // Enable it!
    SetAutoFaceActive(NULL, NULL, iFaceIndex, 0, 0);

    // Set flag to say WE control when to set inactive!
    gFacesData[iFaceIndex].uiFlags |= (FACE_INACTIVE_HANDLED_ELSEWHERE | FACE_MAKEACTIVE_ONCE_DONE);

    // IF we are in tactical and this soldier is on the current squad
    if ((IsTacticalMode()) && (pSoldier != NULL) &&
        (GetSolAssignment(pSoldier) == iCurrentTacticalSquad)) {
      // Make the interface panel dirty..
      // This will dirty the panel next frame...
      gfRerenderInterfaceFromHelpText = TRUE;
    }

    // Setup video overlay!
    VideoOverlayDesc.sLeft = 10;
    VideoOverlayDesc.sTop = 20;
    VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + 99;
    VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + 98;
    VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
    VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
    VideoOverlayDesc.BltCallback = RenderFaceOverlay;

    iFaceOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
    gpCurrentTalkingFace->iVideoOverlay = iFaceOverlay;

    RenderAutoFace(iFaceIndex);

    // ATE: Create mouse region.......
    if (!fExternFaceBoxRegionCreated) {
      fExternFaceBoxRegionCreated = TRUE;

      // Define main region
      MSYS_DefineRegion(&gFacePopupMouseRegion, VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,
                        VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
                        CURSOR_NORMAL, MSYS_NO_CALLBACK, FaceOverlayClickCallback);
      // Add region
      MSYS_AddRegion(&(gFacePopupMouseRegion));
    }

    gfFacePanelActive = TRUE;

  } else if (IsMapScreen_2()) {
    // Are we in mapscreen?
    // If so, set current guy active to talk.....
    if (pSoldier != NULL) {
      ContinueDialogue(pSoldier, FALSE);
    }
  }
}

void HandleDialogueEnd(FACETYPE *pFace) {
  if (gGameSettings.fOptions[TOPTION_SPEECH]) {
    if (pFace != gpCurrentTalkingFace) {
      // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"HandleDialogueEnd() face mismatch." );
      return;
    }

    if (pFace->fTalking) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"HandleDialogueEnd() face still talking.");
      return;
    }

    switch (gbUIHandlerID) {
      case DIALOGUE_TACTICAL_UI:

        if (gfFacePanelActive) {
          // Set face inactive!
          pFace->fCanHandleInactiveNow = TRUE;
          SetAutoFaceInActive(pFace->iID);
          gfFacePanelActive = FALSE;

          if (fExternFaceBoxRegionCreated) {
            fExternFaceBoxRegionCreated = FALSE;
            MSYS_RemoveRegion(&(gFacePopupMouseRegion));
          }
        }
        break;
      case DIALOGUE_NPC_UI:
        break;
      case DIALOGUE_EXTERNAL_NPC_UI:
        pFace->fCanHandleInactiveNow = TRUE;
        SetAutoFaceInActive(pFace->iID);
        gfFacePanelActive = FALSE;

        if (fExternFaceBoxRegionCreated) {
          fExternFaceBoxRegionCreated = FALSE;
          MSYS_RemoveRegion(&(gFacePopupMouseRegion));
        }

        break;
    }
  }

  if (gGameSettings.fOptions[TOPTION_SUBTITLES] || !pFace->fValidSpeech) {
    switch (gbUIHandlerID) {
      case DIALOGUE_TACTICAL_UI:
      case DIALOGUE_EXTERNAL_NPC_UI:
        // Remove if created
        if (giTextBoxOverlay != -1) {
          RemoveVideoOverlay(giTextBoxOverlay);
          giTextBoxOverlay = -1;

          if (fTextBoxMouseRegionCreated) {
            RemoveMercPopupBoxFromIndex(iDialogueBox);

            // reset box id
            iDialogueBox = -1;
            MSYS_RemoveRegion(&gTextBoxMouseRegion);
            fTextBoxMouseRegionCreated = FALSE;
          }
        }

        break;

      case DIALOGUE_NPC_UI:

        // Remove region
        if (gTalkPanel.fTextRegionOn) {
          MSYS_RemoveRegion(&(gTalkPanel.TextRegion));
          gTalkPanel.fTextRegionOn = FALSE;
        }

        SetRenderFlags(RENDER_FLAG_FULL);
        gTalkPanel.fRenderSubTitlesNow = FALSE;

        // Delete subtitle box
        gTalkPanel.fDirtyLevel = DIRTYLEVEL2;
        RemoveMercPopupBoxFromIndex(iInterfaceDialogueBox);
        iInterfaceDialogueBox = -1;
        break;

      case DIALOGUE_CONTACTPAGE_UI:
        break;

      case DIALOGUE_SPECK_CONTACT_PAGE_UI:
        break;
    }
  }

  TurnOffSectorLocator();

  gsExternPanelXPosition = DEFAULT_EXTERN_PANEL_X_POS;
  gsExternPanelYPosition = DEFAULT_EXTERN_PANEL_Y_POS;
}

void RenderFaceOverlay(VIDEO_OVERLAY *pBlitter) {
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint8_t *pDestBuf, *pSrcBuf;
  int16_t sFontX, sFontY;
  struct SOLDIERTYPE *pSoldier;
  wchar_t zTownIDString[50];

  if (gpCurrentTalkingFace == NULL) {
    return;
  }

  if (gfFacePanelActive) {
    pSoldier = FindSoldierByProfileID(gpCurrentTalkingFace->ubCharacterNum, FALSE);

    // a living soldier?..or external NPC?..choose panel based on this
    if (pSoldier) {
      BltVObjectFromIndex(pBlitter->vsDestBuff, guiCOMPANEL, 0, pBlitter->sX, pBlitter->sY);
    } else {
      BltVObjectFromIndex(pBlitter->vsDestBuff, guiCOMPANELB, 0, pBlitter->sX, pBlitter->sY);
    }

    // Display name, location ( if not current )
    SetFont(BLOCKFONT2);
    SetFontBackground(FONT_MCOLOR_BLACK);
    SetFontForeground(FONT_MCOLOR_LTGRAY);

    if (pSoldier) {
      // reset the font dest buffer
      SetFontDestBuffer(pBlitter->vsDestBuff, 0, 0, 640, 480, FALSE);

      VarFindFontCenterCoordinates((int16_t)(pBlitter->sX + 12), (int16_t)(pBlitter->sY + 55), 73,
                                   9, BLOCKFONT2, &sFontX, &sFontY, L"%s", pSoldier->name);
      mprintf(sFontX, sFontY, L"%s", pSoldier->name);

      // What sector are we in, ( and is it the same as ours? )
      if (GetSolSectorX(pSoldier) != gWorldSectorX || GetSolSectorY(pSoldier) != gWorldSectorY ||
          GetSolSectorZ(pSoldier) != gbWorldSectorZ || pSoldier->fBetweenSectors) {
        GetSectorIDString(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier),
                          zTownIDString, ARR_SIZE(zTownIDString), FALSE);

        ReduceStringLength(zTownIDString, ARR_SIZE(zTownIDString), 64, BLOCKFONT2);

        VarFindFontCenterCoordinates((int16_t)(pBlitter->sX + 12), (int16_t)(pBlitter->sY + 68), 73,
                                     9, BLOCKFONT2, &sFontX, &sFontY, L"%s", zTownIDString);
        mprintf(sFontX, sFontY, L"%s", zTownIDString);
      }

      // reset the font dest buffer
      SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

      // Display bars
      DrawLifeUIBarEx(pSoldier, (int16_t)(pBlitter->sX + 69), (int16_t)(pBlitter->sY + 47), 3, 42,
                      FALSE, pBlitter->vsDestBuff);
      DrawBreathUIBarEx(pSoldier, (int16_t)(pBlitter->sX + 75), (int16_t)(pBlitter->sY + 47), 3, 42,
                        FALSE, pBlitter->vsDestBuff);
      DrawMoraleUIBarEx(pSoldier, (int16_t)(pBlitter->sX + 81), (int16_t)(pBlitter->sY + 47), 3, 42,
                        FALSE, pBlitter->vsDestBuff);

    } else {
      VarFindFontCenterCoordinates((int16_t)(pBlitter->sX + 9), (int16_t)(pBlitter->sY + 55), 73, 9,
                                   BLOCKFONT2, &sFontX, &sFontY, L"%s",
                                   gMercProfiles[gpCurrentTalkingFace->ubCharacterNum].zNickname);
      mprintf(sFontX, sFontY, L"%s", gMercProfiles[gpCurrentTalkingFace->ubCharacterNum].zNickname);
    }

    // RenderAutoFace( gpCurrentTalkingFace->iID );
    // BlinkAutoFace( gpCurrentTalkingFace->iID );
    // MouthAutoFace( gpCurrentTalkingFace->iID );

    pDestBuf = LockVSurface(pBlitter->vsDestBuff, &uiDestPitchBYTES);
    pSrcBuf = LockVSurface(gpCurrentTalkingFace->autoDisplayBuffer, &uiSrcPitchBYTES);

    Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES,
                    (int16_t)(pBlitter->sX + 14), (int16_t)(pBlitter->sY + 6), 0, 0,
                    gpCurrentTalkingFace->usFaceWidth, gpCurrentTalkingFace->usFaceHeight);

    JSurface_Unlock(pBlitter->vsDestBuff);
    JSurface_Unlock(gpCurrentTalkingFace->autoDisplayBuffer);

    InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + 99, pBlitter->sY + 98);
  }
}

void RenderSubtitleBoxOverlay(VIDEO_OVERLAY *pBlitter) {
  if (giTextBoxOverlay != -1) {
    RenderMercPopUpBoxFromIndex(iDialogueBox, pBlitter->sX, pBlitter->sY, pBlitter->vsDestBuff);

    InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusSubtitleBoxWidth,
                     pBlitter->sY + gusSubtitleBoxHeight);
  }
}

void SayQuoteFromAnyBodyInSector(uint16_t usQuoteNum) {
  uint8_t ubMercsInSector[20] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt;

  // Loop through all our guys and randomly say one from someone in our sector

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    // Add guy if he's a candidate...
    if (OK_INSECTOR_MERC(pTeamSoldier) && !AM_AN_EPC(pTeamSoldier) &&
        !(pTeamSoldier->uiStatusFlags & SOLDIER_GASSED) && !(AM_A_ROBOT(pTeamSoldier)) &&
        !pTeamSoldier->fMercAsleep) {
      if (gTacticalStatus.bNumFoughtInBattle[ENEMY_TEAM] == 0) {
        // quotes referring to Deidranna's men so we skip quote if there were no army guys fought
        if ((usQuoteNum == QUOTE_SECTOR_SAFE) &&
            (pTeamSoldier->ubProfile == IRA || pTeamSoldier->ubProfile == MIGUEL ||
             pTeamSoldier->ubProfile == SHANK)) {
          continue;
        }
        if ((usQuoteNum == QUOTE_ENEMY_PRESENCE) &&
            (pTeamSoldier->ubProfile == IRA || pTeamSoldier->ubProfile == DIMITRI ||
             pTeamSoldier->ubProfile == DYNAMO || pTeamSoldier->ubProfile == SHANK)) {
          continue;
        }
      }

      ubMercsInSector[ubNumMercs] = (uint8_t)cnt;
      ubNumMercs++;
    }
  }

  // If we are > 0
  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    // If we are air raid, AND red exists somewhere...
    if (usQuoteNum == QUOTE_AIR_RAID) {
      for (cnt = 0; cnt < ubNumMercs; cnt++) {
        if (ubMercsInSector[cnt] == 11) {
          ubChosenMerc = (uint8_t)cnt;
          break;
        }
      }
    }

    TacticalCharacterDialogue(MercPtrs[ubMercsInSector[ubChosenMerc]], usQuoteNum);
  }
}

void SayQuoteFromAnyBodyInThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ,
                                     uint16_t usQuoteNum) {
  uint8_t ubMercsInSector[20] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt;

  // Loop through all our guys and randomly say one from someone in our sector

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      // Add guy if he's a candidate...
      if (pTeamSoldier->sSectorX == sSectorX && pTeamSoldier->sSectorY == sSectorY &&
          pTeamSoldier->bSectorZ == bSectorZ && !AM_AN_EPC(pTeamSoldier) &&
          !(pTeamSoldier->uiStatusFlags & SOLDIER_GASSED) && !(AM_A_ROBOT(pTeamSoldier)) &&
          !pTeamSoldier->fMercAsleep) {
        ubMercsInSector[ubNumMercs] = (uint8_t)cnt;
        ubNumMercs++;
      }
    }
  }

  // If we are > 0
  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    // If we are air raid, AND red exists somewhere...
    if (usQuoteNum == QUOTE_AIR_RAID) {
      for (cnt = 0; cnt < ubNumMercs; cnt++) {
        if (ubMercsInSector[cnt] == 11) {
          ubChosenMerc = (uint8_t)cnt;
          break;
        }
      }
    }

    TacticalCharacterDialogue(MercPtrs[ubMercsInSector[ubChosenMerc]], usQuoteNum);
  }
}

void SayQuoteFromNearbyMercInSector(int16_t sGridNo, int8_t bDistance, uint16_t usQuoteNum) {
  uint8_t ubMercsInSector[20] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt;

  // Loop through all our guys and randomly say one from someone in our sector

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    // Add guy if he's a candidate...
    if (OK_INSECTOR_MERC(pTeamSoldier) &&
        PythSpacesAway(sGridNo, pTeamSoldier->sGridNo) < bDistance && !AM_AN_EPC(pTeamSoldier) &&
        !(pTeamSoldier->uiStatusFlags & SOLDIER_GASSED) && !(AM_A_ROBOT(pTeamSoldier)) &&
        !pTeamSoldier->fMercAsleep &&
        SoldierTo3DLocationLineOfSightTest(pTeamSoldier, sGridNo, 0, 0,
                                           (uint8_t)MaxDistanceVisible(), TRUE)) {
      if (usQuoteNum == 66 && (int8_t)Random(100) > EffectiveWisdom(pTeamSoldier)) {
        continue;
      }
      ubMercsInSector[ubNumMercs] = (uint8_t)cnt;
      ubNumMercs++;
    }
  }

  // If we are > 0
  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);

    if (usQuoteNum == 66) {
      SetFactTrue(FACT_PLAYER_FOUND_ITEMS_MISSING);
    }
    TacticalCharacterDialogue(MercPtrs[ubMercsInSector[ubChosenMerc]], usQuoteNum);
  }
}

void SayQuote58FromNearbyMercInSector(int16_t sGridNo, int8_t bDistance, uint16_t usQuoteNum,
                                      int8_t bSex) {
  uint8_t ubMercsInSector[20] = {0};
  uint8_t ubNumMercs = 0;
  uint8_t ubChosenMerc;
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt;

  // Loop through all our guys and randomly say one from someone in our sector

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    // Add guy if he's a candidate...
    if (OK_INSECTOR_MERC(pTeamSoldier) &&
        PythSpacesAway(sGridNo, pTeamSoldier->sGridNo) < bDistance && !AM_AN_EPC(pTeamSoldier) &&
        !(pTeamSoldier->uiStatusFlags & SOLDIER_GASSED) && !(AM_A_ROBOT(pTeamSoldier)) &&
        !pTeamSoldier->fMercAsleep &&
        SoldierTo3DLocationLineOfSightTest(pTeamSoldier, sGridNo, 0, 0,
                                           (uint8_t)MaxDistanceVisible(), TRUE)) {
      // ATE: This is to check gedner for this quote...
      if (QuoteExp_GenderCode[pTeamSoldier->ubProfile] == 0 && bSex == FEMALE) {
        continue;
      }

      if (QuoteExp_GenderCode[pTeamSoldier->ubProfile] == 1 && bSex == MALE) {
        continue;
      }

      ubMercsInSector[ubNumMercs] = (uint8_t)cnt;
      ubNumMercs++;
    }
  }

  // If we are > 0
  if (ubNumMercs > 0) {
    ubChosenMerc = (uint8_t)Random(ubNumMercs);
    TacticalCharacterDialogue(MercPtrs[ubMercsInSector[ubChosenMerc]], usQuoteNum);
  }
}

void TextOverlayClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fLButtonDown = TRUE;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown) {
    if (gpCurrentTalkingFace != NULL) {
      InternalShutupaYoFace(gpCurrentTalkingFace->iID, FALSE);

      // Did we succeed in shutting them up?
      if (!gpCurrentTalkingFace->fTalking) {
        // shut down last quote box
        ShutDownLastQuoteTacticalTextBox();
      }
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void FaceOverlayClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fLButtonDown = TRUE;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown) {
    if (gpCurrentTalkingFace != NULL) {
      InternalShutupaYoFace(gpCurrentTalkingFace->iID, FALSE);
    }

  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void ShutDownLastQuoteTacticalTextBox(void) {
  if (fDialogueBoxDueToLastMessage) {
    RemoveVideoOverlay(giTextBoxOverlay);
    giTextBoxOverlay = -1;

    if (fTextBoxMouseRegionCreated) {
      MSYS_RemoveRegion(&gTextBoxMouseRegion);
      fTextBoxMouseRegionCreated = FALSE;
    }

    fDialogueBoxDueToLastMessage = FALSE;
  }
}

uint32_t FindDelayForString(wchar_t *sString) { return (wcslen(sString) * TEXT_DELAY_MODIFIER); }

void BeginLoggingForBleedMeToos(BOOLEAN fStart) { gubLogForMeTooBleeds = fStart; }

void SetEngagedInConvFromPCAction(struct SOLDIERTYPE *pSoldier) {
  // OK, If a good give, set engaged in conv...
  gTacticalStatus.uiFlags |= ENGAGED_IN_CONV;
  gTacticalStatus.ubEngagedInConvFromActionMercID = GetSolID(pSoldier);
}

void UnSetEngagedInConvFromPCAction(struct SOLDIERTYPE *pSoldier) {
  if (gTacticalStatus.ubEngagedInConvFromActionMercID == GetSolID(pSoldier)) {
    // OK, If a good give, set engaged in conv...
    gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV);
  }
}

BOOLEAN IsStopTimeQuote(uint16_t usQuoteNum) {
  int32_t cnt;

  for (cnt = 0; cnt < gubNumStopTimeQuotes; cnt++) {
    if (gusStopTimeQuoteList[cnt] == usQuoteNum) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void CheckForStopTimeQuotes(uint16_t usQuoteNum) {
  if (IsStopTimeQuote(usQuoteNum)) {
    // Stop Time, game
    EnterModalTactical(TACTICAL_MODAL_NOMOUSE);

    gpCurrentTalkingFace->uiFlags |= FACE_MODAL;

    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Starting Modal Tactical Quote.");
  }
}

void SetStopTimeQuoteCallback(MODAL_HOOK pCallBack) { gModalDoneCallback = pCallBack; }

BOOLEAN IsMercSayingDialogue(uint8_t ubProfileID) {
  if (gpCurrentTalkingFace != NULL && gubCurrentTalkingID == ubProfileID) {
    return (TRUE);
  }
  return (FALSE);
}

BOOLEAN ShouldMercSayPrecedentToRepeatOneSelf(uint8_t ubMercID, uint32_t uiQuoteID) {
  uint8_t ubQuoteBit = 0;

  // If the quote is not in the array
  if (!IsQuoteInPrecedentArray(uiQuoteID)) {
    return (FALSE);
  }

  ubQuoteBit = GetQuoteBitNumberFromQuoteID(uiQuoteID);
  if (ubQuoteBit == 0) return (FALSE);

  if (GetMercPrecedentQuoteBitStatus(ubMercID, ubQuoteBit)) {
    return (TRUE);
  } else {
    SetMercPrecedentQuoteBitStatus(ubMercID, ubQuoteBit);
  }

  return (FALSE);
}

BOOLEAN GetMercPrecedentQuoteBitStatus(uint8_t ubMercID, uint8_t ubQuoteBit) {
  if (gMercProfiles[ubMercID].uiPrecedentQuoteSaid & (1 << (ubQuoteBit - 1)))
    return (TRUE);
  else
    return (FALSE);
}

BOOLEAN SetMercPrecedentQuoteBitStatus(uint8_t ubMercID, uint8_t ubBitToSet) {
  // Set the bit
  gMercProfiles[ubMercID].uiPrecedentQuoteSaid |= 1 << (ubBitToSet - 1);

  return (TRUE);
}

BOOLEAN IsQuoteInPrecedentArray(uint32_t uiQuoteID) {
  uint8_t ubCnt;

  // If the quote id is above or below the ones in the array
  if (uiQuoteID < gubMercValidPrecedentQuoteID[0] ||
      uiQuoteID > gubMercValidPrecedentQuoteID[NUMBER_VALID_MERC_PRECEDENT_QUOTES - 1]) {
    return (FALSE);
  }

  // loop through all the quotes
  for (ubCnt = 0; ubCnt < NUMBER_VALID_MERC_PRECEDENT_QUOTES; ubCnt++) {
    if (gubMercValidPrecedentQuoteID[ubCnt] == uiQuoteID) {
      return (TRUE);
    }
  }

  return (FALSE);
}

uint8_t GetQuoteBitNumberFromQuoteID(uint32_t uiQuoteID) {
  uint8_t ubCnt;

  // loop through all the quotes
  for (ubCnt = 0; ubCnt < NUMBER_VALID_MERC_PRECEDENT_QUOTES; ubCnt++) {
    if (gubMercValidPrecedentQuoteID[ubCnt] == uiQuoteID) {
      return (ubCnt);
    }
  }

  return (0);
}

void HandleShutDownOfMapScreenWhileExternfaceIsTalking(void) {
  if ((fExternFaceBoxRegionCreated) && (gpCurrentTalkingFace)) {
    RemoveVideoOverlay(gpCurrentTalkingFace->iVideoOverlay);
    gpCurrentTalkingFace->iVideoOverlay = -1;
  }
}

void HandleImportantMercQuote(struct SOLDIERTYPE *pSoldier, uint16_t usQuoteNumber) {
  // wake merc up for THIS quote
  if (pSoldier->fMercAsleep) {
    TacticalCharacterDialogueWithSpecialEvent(pSoldier, usQuoteNumber, DIALOGUE_SPECIAL_EVENT_SLEEP,
                                              0, 0);
    TacticalCharacterDialogue(pSoldier, usQuoteNumber);
    TacticalCharacterDialogueWithSpecialEvent(pSoldier, usQuoteNumber, DIALOGUE_SPECIAL_EVENT_SLEEP,
                                              1, 0);
  } else {
    TacticalCharacterDialogue(pSoldier, usQuoteNumber);
  }
}

// handle pausing of the dialogue queue
void PauseDialogueQueue(void) {
  gfDialogueQueuePaused = TRUE;
  return;
}

// unpause the dialogue queue
void UnPauseDialogueQueue(void) {
  gfDialogueQueuePaused = FALSE;
  return;
}

void SetExternMapscreenSpeechPanelXY(int16_t sXPos, int16_t sYPos) {
  gsExternPanelXPosition = sXPos;
  gsExternPanelYPosition = sYPos;
}
