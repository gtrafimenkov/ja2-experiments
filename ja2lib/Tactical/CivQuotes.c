// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/CivQuotes.h"

#include "MessageBoxScreen.h"
#include "SGP/FileMan.h"
#include "SGP/MouseSystem.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/MapScreen.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/AnimationData.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "Town.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/FontControl.h"
#include "Utils/MercTextBox.h"
#include "Utils/Message.h"
#include "Utils/Text.h"

#define DIALOGUE_DEFAULT_WIDTH 200
#define EXTREAMLY_LOW_TOWN_LOYALTY 20
#define HIGH_TOWN_LOYALTY 80
#define CIV_QUOTE_HINT 99

typedef struct {
  uint8_t ubNumEntries;
  uint8_t ubUnusedCurrentEntry;

} CIV_QUOTE;

extern void CaptureTimerCallback(void);

BOOLEAN gfSurrendered = FALSE;

CIV_QUOTE gCivQuotes[NUM_CIV_QUOTES];

uint8_t gubNumEntries[NUM_CIV_QUOTES] = {15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

                                         15, 15, 15, 15, 15, 15, 15, 15, 15, 15,

                                         5,  5,  15, 15, 15, 15, 15, 15, 15, 15,

                                         15, 15, 2,  15, 15, 10, 10, 5,  3,  10,

                                         3,  3,  3,  3,  3,  3,  3,  3,  3,  3};

typedef struct {
  BOOLEAN bActive;
  struct MOUSE_REGION MouseRegion;
  int32_t iVideoOverlay;
  int32_t iDialogueBox;
  uint32_t uiTimeOfCreation;
  uint32_t uiDelayTime;
  struct SOLDIERTYPE *pCiv;
} QUOTE_SYSTEM_STRUCT;

QUOTE_SYSTEM_STRUCT gCivQuoteData;

wchar_t gzCivQuote[320];
uint16_t gusCivQuoteBoxWidth;
uint16_t gusCivQuoteBoxHeight;

void CopyNumEntriesIntoQuoteStruct() {
  int32_t cnt;

  for (cnt = 0; cnt < NUM_CIV_QUOTES; cnt++) {
    gCivQuotes[cnt].ubNumEntries = gubNumEntries[cnt];
  }
}

BOOLEAN GetCivQuoteText(uint8_t ubCivQuoteID, uint8_t ubEntryID, wchar_t *zQuote) {
  char zFileName[164];

  // Build filename....
  if (ubCivQuoteID == CIV_QUOTE_HINT) {
    if (gbWorldSectorZ > 0) {
      // sprintf( zFileName, "NPCData\\miners.edt" );
      sprintf(zFileName, "NPCDATA\\CIV%02d.edt", CIV_QUOTE_MINERS_NOT_FOR_PLAYER);
    } else {
      sprintf(zFileName, "NPCData\\%c%d.edt", 'A' + (gWorldSectorY - 1), gWorldSectorX);
    }
  } else {
    sprintf(zFileName, "NPCDATA\\CIV%02d.edt", ubCivQuoteID);
  }

  CHECKF(FileMan_Exists(zFileName));

  // Get data...
  LoadEncryptedDataFromFile(zFileName, zQuote, ubEntryID * 320, 320);

  if (zQuote[0] == 0) {
    return (FALSE);
  }

  return (TRUE);
}

void SurrenderMessageBoxCallBack(uint8_t ubExitValue) {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;

  if (ubExitValue == MSG_BOX_RETURN_YES) {
    // CJC Dec 1 2002: fix multiple captures
    BeginCaptureSquence();

    // Do capture....
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

    for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pTeamSoldier++) {
      // Are we active and in sector.....
      if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
        if (pTeamSoldier->bLife != 0) {
          EnemyCapturesPlayerSoldier(pTeamSoldier);

          RemoveSoldierFromTacticalSector(pTeamSoldier, TRUE);
        }
      }
    }

    EndCaptureSequence();

    gfSurrendered = TRUE;
    SetCustomizableTimerCallbackAndDelay(3000, CaptureTimerCallback, FALSE);

    ActionDone(gCivQuoteData.pCiv);
  } else {
    ActionDone(gCivQuoteData.pCiv);
  }
}

void ShutDownQuoteBox(BOOLEAN fForce) {
  if (!gCivQuoteData.bActive) {
    return;
  }

  // Check for min time....
  if ((GetJA2Clock() - gCivQuoteData.uiTimeOfCreation) > 300 || fForce) {
    RemoveVideoOverlay(gCivQuoteData.iVideoOverlay);

    // Remove mouse region...
    MSYS_RemoveRegion(&(gCivQuoteData.MouseRegion));

    RemoveMercPopupBoxFromIndex(gCivQuoteData.iDialogueBox);
    gCivQuoteData.iDialogueBox = -1;

    gCivQuoteData.bActive = FALSE;

    // do we need to do anything at the end of the civ quote?
    if (gCivQuoteData.pCiv && gCivQuoteData.pCiv->bAction == AI_ACTION_OFFER_SURRENDER) {
      DoMessageBox(MSG_BOX_BASIC_STYLE, Message[STR_SURRENDER], GAME_SCREEN,
                   (uint8_t)MSG_BOX_FLAG_YESNO, SurrenderMessageBoxCallBack, NULL);
    }
  }
}

BOOLEAN ShutDownQuoteBoxIfActive() {
  if (gCivQuoteData.bActive) {
    ShutDownQuoteBox(TRUE);

    return (TRUE);
  }

  return (FALSE);
}

int8_t GetCivType(struct SOLDIERTYPE *pCiv) {
  if (pCiv->ubProfile != NO_PROFILE) {
    return (CIV_TYPE_NA);
  }

  // ATE: Check if this person is married.....
  // 1 ) check sector....
  if (gWorldSectorX == 10 && gWorldSectorY == 6 && gbWorldSectorZ == 0) {
    // 2 ) the only female....
    if (pCiv->ubCivilianGroup == 0 && pCiv->bTeam != gbPlayerNum && pCiv->ubBodyType == REGFEMALE) {
      // She's a ho!
      return (CIV_TYPE_MARRIED_PC);
    }
  }

  // OK, look for enemy type - MUST be on enemy team, merc bodytype
  if (pCiv->bTeam == ENEMY_TEAM && IS_MERC_BODY_TYPE(pCiv)) {
    return (CIV_TYPE_ENEMY);
  }

  if (pCiv->bTeam != CIV_TEAM && pCiv->bTeam != MILITIA_TEAM) {
    return (CIV_TYPE_NA);
  }

  switch (pCiv->ubBodyType) {
    case REGMALE:
    case BIGMALE:
    case STOCKYMALE:
    case REGFEMALE:
    case FATCIV:
    case MANCIV:
    case MINICIV:
    case DRESSCIV:
    case CRIPPLECIV:

      return (CIV_TYPE_ADULT);
      break;

    case ADULTFEMALEMONSTER:
    case AM_MONSTER:
    case YAF_MONSTER:
    case YAM_MONSTER:
    case LARVAE_MONSTER:
    case INFANT_MONSTER:
    case QUEENMONSTER:

      return (CIV_TYPE_NA);

    case HATKIDCIV:
    case KIDCIV:

      return (CIV_TYPE_KID);

    default:

      return (CIV_TYPE_NA);
  }

  return (CIV_TYPE_NA);
}

void RenderCivQuoteBoxOverlay(VIDEO_OVERLAY *pBlitter) {
  if (gCivQuoteData.iVideoOverlay != -1) {
    RenderMercPopUpBoxFromIndex(gCivQuoteData.iDialogueBox, pBlitter->sX, pBlitter->sY,
                                pBlitter->vsDestBuff);

    InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusCivQuoteBoxWidth,
                     pBlitter->sY + gusCivQuoteBoxHeight);
  }
}

void QuoteOverlayClickCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  static BOOLEAN fLButtonDown = FALSE;

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fLButtonDown = TRUE;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP && fLButtonDown) {
    // Shutdown quote box....
    ShutDownQuoteBox(FALSE);
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    fLButtonDown = FALSE;
  }
}

void BeginCivQuote(struct SOLDIERTYPE *pCiv, uint8_t ubCivQuoteID, uint8_t ubEntryID, int16_t sX,
                   int16_t sY) {
  VIDEO_OVERLAY_DESC VideoOverlayDesc;
  wchar_t zQuote[320];

  // OK, do we have another on?
  if (gCivQuoteData.bActive) {
    // Delete?
    ShutDownQuoteBox(TRUE);
  }

  // get text
  if (!GetCivQuoteText(ubCivQuoteID, ubEntryID, zQuote)) {
    return;
  }

#ifdef TAIWANESE
  swprintf(gzCivQuote, ARR_SIZE(gzCivQuote), L"%s", zQuote);
#else
  swprintf(gzCivQuote, ARR_SIZE(gzCivQuote), L"\"%s\"", zQuote);
#endif

  if (ubCivQuoteID == CIV_QUOTE_HINT) {
    MapScreenMessage(FONT_MCOLOR_WHITE, MSG_DIALOG, L"%s", gzCivQuote);
  }

  // Create video oeverlay....
  memset(&VideoOverlayDesc, 0, sizeof(VIDEO_OVERLAY_DESC));

  // Prepare text box
  SET_USE_WINFONTS(TRUE);
  SET_WINFONT(giSubTitleWinFont);
  gCivQuoteData.iDialogueBox = PrepareMercPopupBox(
      gCivQuoteData.iDialogueBox, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, gzCivQuote,
      DIALOGUE_DEFAULT_WIDTH, 0, 0, 0, &gusCivQuoteBoxWidth, &gusCivQuoteBoxHeight);
  SET_USE_WINFONTS(FALSE);

  // OK, find center for box......
  sX = sX - (gusCivQuoteBoxWidth / 2);
  sY = sY - (gusCivQuoteBoxHeight / 2);

  // OK, limit to screen......
  {
    if (sX < 0) {
      sX = 0;
    }

    // CHECK FOR LEFT/RIGHT
    if ((sX + gusCivQuoteBoxWidth) > 640) {
      sX = 640 - gusCivQuoteBoxWidth;
    }

    // Now check for top
    if (sY < gsVIEWPORT_WINDOW_START_Y) {
      sY = gsVIEWPORT_WINDOW_START_Y;
    }

    // Check for bottom
    if ((sY + gusCivQuoteBoxHeight) > 340) {
      sY = 340 - gusCivQuoteBoxHeight;
    }
  }

  VideoOverlayDesc.sLeft = sX;
  VideoOverlayDesc.sTop = sY;
  VideoOverlayDesc.sRight = VideoOverlayDesc.sLeft + gusCivQuoteBoxWidth;
  VideoOverlayDesc.sBottom = VideoOverlayDesc.sTop + gusCivQuoteBoxHeight;
  VideoOverlayDesc.sX = VideoOverlayDesc.sLeft;
  VideoOverlayDesc.sY = VideoOverlayDesc.sTop;
  VideoOverlayDesc.BltCallback = RenderCivQuoteBoxOverlay;

  gCivQuoteData.iVideoOverlay = RegisterVideoOverlay(0, &VideoOverlayDesc);

  // Define main region
  MSYS_DefineRegion(&(gCivQuoteData.MouseRegion), VideoOverlayDesc.sLeft, VideoOverlayDesc.sTop,
                    VideoOverlayDesc.sRight, VideoOverlayDesc.sBottom, MSYS_PRIORITY_HIGHEST,
                    CURSOR_NORMAL, MSYS_NO_CALLBACK, QuoteOverlayClickCallback);
  // Add region
  MSYS_AddRegion(&(gCivQuoteData.MouseRegion));

  gCivQuoteData.bActive = TRUE;

  gCivQuoteData.uiTimeOfCreation = GetJA2Clock();

  gCivQuoteData.uiDelayTime = FindDelayForString(gzCivQuote) + 500;

  gCivQuoteData.pCiv = pCiv;
}

uint8_t DetermineCivQuoteEntry(struct SOLDIERTYPE *pCiv, uint8_t *pubCivHintToUse,
                               BOOLEAN fCanUseHints) {
  uint8_t ubCivType;
  TownID bTownId;
  BOOLEAN bCivLowLoyalty = FALSE;
  BOOLEAN bCivHighLoyalty = FALSE;
  int8_t bCivHint;
  int8_t bMineId;
  BOOLEAN bMiners = FALSE;

  (*pubCivHintToUse) = 0;

  ubCivType = GetCivType(pCiv);

  if (ubCivType == CIV_TYPE_ENEMY) {
    // Determine what type of quote to say...
    // Are are we going to attack?

    if (pCiv->bAction == AI_ACTION_TOSS_PROJECTILE || pCiv->bAction == AI_ACTION_FIRE_GUN ||
        pCiv->bAction == AI_ACTION_FIRE_GUN || pCiv->bAction == AI_ACTION_KNIFE_MOVE) {
      return (CIV_QUOTE_ENEMY_THREAT);
    } else if (pCiv->bAction == AI_ACTION_OFFER_SURRENDER) {
      return (CIV_QUOTE_ENEMY_OFFER_SURRENDER);
    }
    // Hurt?
    else if (pCiv->bLife < 30) {
      return (CIV_QUOTE_ENEMY_HURT);
    }
    // elite?
    else if (pCiv->ubSoldierClass == SOLDIER_CLASS_ELITE) {
      return (CIV_QUOTE_ENEMY_ELITE);
    } else {
      return (CIV_QUOTE_ENEMY_ADMIN);
    }
  }

  // Are we in a town sector?
  // get town id
  bTownId = GetTownIdForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY);

  // If a married PC...
  if (ubCivType == CIV_TYPE_MARRIED_PC) {
    return (CIV_QUOTE_PC_MARRIED);
  }

  // CIV GROUPS FIRST!
  // Hicks.....
  if (pCiv->ubCivilianGroup == HICKS_CIV_GROUP) {
    // Are they friendly?
    // if ( gTacticalStatus.fCivGroupHostile[ HICKS_CIV_GROUP ] < CIV_GROUP_WILL_BECOME_HOSTILE )
    if (pCiv->bNeutral) {
      return (CIV_QUOTE_HICKS_FRIENDLY);
    } else {
      return (CIV_QUOTE_HICKS_ENEMIES);
    }
  }

  // Goons.....
  if (pCiv->ubCivilianGroup == KINGPIN_CIV_GROUP) {
    // Are they friendly?
    // if ( gTacticalStatus.fCivGroupHostile[ KINGPIN_CIV_GROUP ] < CIV_GROUP_WILL_BECOME_HOSTILE )
    if (pCiv->bNeutral) {
      return (CIV_QUOTE_GOONS_FRIENDLY);
    } else {
      return (CIV_QUOTE_GOONS_ENEMIES);
    }
  }

  // ATE: Cowering people take precedence....
  if ((pCiv->uiStatusFlags & SOLDIER_COWERING) ||
      (pCiv->bTeam == CIV_TEAM && (gTacticalStatus.uiFlags & INCOMBAT))) {
    if (ubCivType == CIV_TYPE_ADULT) {
      return (CIV_QUOTE_ADULTS_COWER);
    } else {
      return (CIV_QUOTE_KIDS_COWER);
    }
  }

  // Kid slaves...
  if (pCiv->ubCivilianGroup == FACTORY_KIDS_GROUP) {
    // Check fact.....
    if (CheckFact(FACT_DOREEN_HAD_CHANGE_OF_HEART, 0) || !CheckFact(FACT_DOREEN_ALIVE, 0)) {
      return (CIV_QUOTE_KID_SLAVES_FREE);
    } else {
      return (CIV_QUOTE_KID_SLAVES);
    }
  }

  // BEGGERS
  if (pCiv->ubCivilianGroup == BEGGARS_CIV_GROUP) {
    // Check if we are in a town...
    if (bTownId != BLANK_SECTOR && gbWorldSectorZ == 0) {
      if (bTownId == SAN_MONA && ubCivType == CIV_TYPE_ADULT) {
        return (CIV_QUOTE_SAN_MONA_BEGGERS);
      }
    }

    // DO normal beggers...
    if (ubCivType == CIV_TYPE_ADULT) {
      return (CIV_QUOTE_ADULTS_BEGGING);
    } else {
      return (CIV_QUOTE_KIDS_BEGGING);
    }
  }

  // REBELS
  if (pCiv->ubCivilianGroup == REBEL_CIV_GROUP) {
    // DO normal beggers...
    if (ubCivType == CIV_TYPE_ADULT) {
      return (CIV_QUOTE_ADULTS_REBELS);
    } else {
      return (CIV_QUOTE_KIDS_REBELS);
    }
  }

  // Do miltitia...
  if (pCiv->bTeam == MILITIA_TEAM) {
    // Different types....
    if (pCiv->ubSoldierClass == SOLDIER_CLASS_GREEN_MILITIA) {
      return (CIV_QUOTE_GREEN_MILITIA);
    }
    if (pCiv->ubSoldierClass == SOLDIER_CLASS_REG_MILITIA) {
      return (CIV_QUOTE_MEDIUM_MILITIA);
    }
    if (pCiv->ubSoldierClass == SOLDIER_CLASS_ELITE_MILITIA) {
      return (CIV_QUOTE_ELITE_MILITIA);
    }
  }

  // If we are in medunna, and queen is dead, use these...
  if (bTownId == MEDUNA && CheckFact(FACT_QUEEN_DEAD, 0)) {
    return (CIV_QUOTE_DEIDRANNA_DEAD);
  }

  // if in a town
  if ((bTownId != BLANK_SECTOR) && (gbWorldSectorZ == 0) && gfTownUsesLoyalty[bTownId]) {
    // Check loyalty special quotes.....
    // EXTREMELY LOW TOWN LOYALTY...
    if (gTownLoyalty[bTownId].ubRating < EXTREAMLY_LOW_TOWN_LOYALTY) {
      bCivLowLoyalty = TRUE;
    }

    // HIGH TOWN LOYALTY...
    if (gTownLoyalty[bTownId].ubRating >= HIGH_TOWN_LOYALTY) {
      bCivHighLoyalty = TRUE;
    }
  }

  // ATE: OK, check if we should look for a civ hint....
  if (fCanUseHints) {
    bCivHint = ConsiderCivilianQuotes((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY,
                                      gbWorldSectorZ, FALSE);
  } else {
    bCivHint = -1;
  }

  // ATE: check miners......
  if (pCiv->ubSoldierClass == SOLDIER_CLASS_MINER) {
    bMiners = TRUE;

    // If not a civ hint available...
    if (bCivHint == -1) {
      // Check if they are under our control...

      // Should I go talk to miner?
      // Not done yet.

      // Are they working for us?
      bMineId =
          GetIdOfMineForSector((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);

      if (PlayerControlsMine(bMineId)) {
        return (CIV_QUOTE_MINERS_FOR_PLAYER);
      } else {
        return (CIV_QUOTE_MINERS_NOT_FOR_PLAYER);
      }
    }
  }

  // Is one availible?
  // If we are to say low loyalty, do chance
  if (bCivHint != -1 && bCivLowLoyalty && !bMiners) {
    if (Random(100) < 25) {
      // Get rid of hint...
      bCivHint = -1;
    }
  }

  // Say hint if availible...
  if (bCivHint != -1) {
    if (ubCivType == CIV_TYPE_ADULT) {
      (*pubCivHintToUse) = bCivHint;

      // Set quote as used...
      ConsiderCivilianQuotes((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ, TRUE);

      // retrun value....
      return (CIV_QUOTE_HINT);
    }
  }

  if (bCivLowLoyalty) {
    if (ubCivType == CIV_TYPE_ADULT) {
      return (CIV_QUOTE_ADULTS_EXTREMLY_LOW_LOYALTY);
    } else {
      return (CIV_QUOTE_KIDS_EXTREMLY_LOW_LOYALTY);
    }
  }

  if (bCivHighLoyalty) {
    if (ubCivType == CIV_TYPE_ADULT) {
      return (CIV_QUOTE_ADULTS_HIGH_LOYALTY);
    } else {
      return (CIV_QUOTE_KIDS_HIGH_LOYALTY);
    }
  }

  // All purpose quote here....
  if (ubCivType == CIV_TYPE_ADULT) {
    return (CIV_QUOTE_ADULTS_ALL_PURPOSE);
  } else {
    return (CIV_QUOTE_KIDS_ALL_PURPOSE);
  }
}

void HandleCivQuote() {
  if (gCivQuoteData.bActive) {
    // Check for min time....
    if ((GetJA2Clock() - gCivQuoteData.uiTimeOfCreation) > gCivQuoteData.uiDelayTime) {
      // Stop!
      ShutDownQuoteBox(TRUE);
    }
  }
}

void StartCivQuote(struct SOLDIERTYPE *pCiv) {
  uint8_t ubCivQuoteID;
  int16_t sX, sY;
  uint8_t ubEntryID = 0;
  int16_t sScreenX, sScreenY;
  uint8_t ubCivHintToUse;

  // ATE: Check for old quote.....
  // This could have been stored on last attempt...
  if (pCiv->bCurrentCivQuote == CIV_QUOTE_HINT) {
    // Determine which quote to say.....
    // CAN'T USE HINTS, since we just did one...
    pCiv->bCurrentCivQuote = -1;
    pCiv->bCurrentCivQuoteDelta = 0;
    ubCivQuoteID = DetermineCivQuoteEntry(pCiv, &ubCivHintToUse, FALSE);
  } else {
    // Determine which quote to say.....
    ubCivQuoteID = DetermineCivQuoteEntry(pCiv, &ubCivHintToUse, TRUE);
  }

  // Determine entry id
  // ATE: Try and get entry from soldier pointer....
  if (ubCivQuoteID != CIV_QUOTE_HINT) {
    if (pCiv->bCurrentCivQuote == -1) {
      // Pick random one
      pCiv->bCurrentCivQuote = (int8_t)Random(gCivQuotes[ubCivQuoteID].ubNumEntries - 2);
      pCiv->bCurrentCivQuoteDelta = 0;
    }

    ubEntryID = pCiv->bCurrentCivQuote + pCiv->bCurrentCivQuoteDelta;
  } else {
    ubEntryID = ubCivHintToUse;

    // ATE: set value for quote ID.....
    pCiv->bCurrentCivQuote = ubCivQuoteID;
    pCiv->bCurrentCivQuoteDelta = ubEntryID;
  }

  // Determine location...
  // Get location of civ on screen.....
  GetSoldierScreenPos(pCiv, &sScreenX, &sScreenY);
  sX = sScreenX;
  sY = sScreenY;

  // begin quote
  BeginCivQuote(pCiv, ubCivQuoteID, ubEntryID, sX, sY);

  // Increment use
  if (ubCivQuoteID != CIV_QUOTE_HINT) {
    pCiv->bCurrentCivQuoteDelta++;

    if (pCiv->bCurrentCivQuoteDelta == 2) {
      pCiv->bCurrentCivQuoteDelta = 0;
    }
  }
}

void InitCivQuoteSystem() {
  memset(&gCivQuotes, 0, sizeof(gCivQuotes));
  CopyNumEntriesIntoQuoteStruct();

  memset(&gCivQuoteData, 0, sizeof(gCivQuoteData));
  gCivQuoteData.bActive = FALSE;
  gCivQuoteData.iVideoOverlay = -1;
  gCivQuoteData.iDialogueBox = -1;
}

BOOLEAN SaveCivQuotesToSaveGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten;

  FileMan_Write(hFile, &gCivQuotes, sizeof(gCivQuotes), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(gCivQuotes)) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN LoadCivQuotesFromLoadGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead;

  FileMan_Read(hFile, &gCivQuotes, sizeof(gCivQuotes), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(gCivQuotes)) {
    return (FALSE);
  }

  CopyNumEntriesIntoQuoteStruct();

  return (TRUE);
}
