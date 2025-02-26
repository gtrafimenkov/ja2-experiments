// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/SoldierControl.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "GameScreen.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "SGP/Container.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/MemMan.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/CampaignTypes.h"
#include "Strategic/GameClock.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMercHandler.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicStatus.h"
#include "SysGlobals.h"
#include "Tactical/AnimationCache.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/Boxing.h"
#include "Tactical/Campaign.h"
#include "Tactical/CivQuotes.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/DrugsAndAlcohol.h"
#include "Tactical/Faces.h"
#include "Tactical/Gap.h"
#include "Tactical/HandleDoors.h"
#include "Tactical/HandleItems.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/Items.h"
#include "Tactical/Keys.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Menptr.h"
#include "Tactical/Morale.h"
#include "Tactical/OppList.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/RTTimeDefines.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/ShopKeeperInterface.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierAni.h"
#include "Tactical/SoldierFunctions.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierTile.h"
#include "Tactical/Squads.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/Vehicles.h"
#include "Tactical/Weapons.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Smell.h"
#include "TileEngine/SmokeEffects.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/EventPump.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"

extern int16_t DirIncrementer[8];

#define PALETTEFILENAME "BINARYDATA\\ja2pal.dat"

#define LOW_MORALE_BATTLE_SND_THREASHOLD 35

#define TURNING_FROM_PRONE_OFF 0
#define TURNING_FROM_PRONE_ON 1
#define TURNING_FROM_PRONE_START_UP_FROM_MOVE 2
#define TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE 3

#define MIN_SUBSEQUENT_SNDS_DELAY 2000

// Enumerate extended directions
enum {
  EX_NORTH = 0,
  EX_NORTHEAST = 4,
  EX_EAST = 8,
  EX_SOUTHEAST = 12,
  EX_SOUTH = 16,
  EX_SOUTHWEST = 20,
  EX_WEST = 24,
  EX_NORTHWEST = 28,
  EX_NUM_WORLD_DIRECTIONS = 32,
  EX_DIRECTION_IRRELEVANT
} ExtendedWorldDirections;

// LUT for conversion from 8-direction to extended direction
uint8_t ubExtDirection[] = {EX_NORTH, EX_NORTHEAST, EX_EAST, EX_SOUTHEAST,
                            EX_SOUTH, EX_SOUTHWEST, EX_WEST, EX_NORTHWEST};

uint8_t gExtOneCDirection[EX_NUM_WORLD_DIRECTIONS] = {
    4,  5,  6,  7,

    8,  9,  10, 11,

    12, 13, 14, 15,

    16, 17, 18, 19,

    20, 21, 22, 23,

    24, 25, 26, 27,

    28, 29, 30, 31,

    0,  1,  2,  3,
};

typedef struct {
  char zName[20];
  uint8_t ubRandomVal;
  BOOLEAN fPreload;
  BOOLEAN fBadGuy;
  BOOLEAN fDontAllowTwoInRow;
  BOOLEAN fStopDialogue;

} BATTLESNDS_STRUCT;

BATTLESNDS_STRUCT gBattleSndsData[] = {
    {"ok1", 2, 1, 1, 1, 2},    {"ok2", 0, 1, 1, 1, 2},   {"cool", 0, 1, 0, 1, 0},
    {"curse", 0, 1, 1, 1, 0},  {"hit1", 2, 1, 1, 1, 1},  {"hit2", 0, 1, 1, 1, 1},
    {"laugh", 0, 1, 1, 1, 0},  {"attn", 0, 1, 0, 1, 0},  {"dying", 0, 1, 1, 1, 1},
    {"humm", 0, 0, 0, 1, 1},   {"noth", 0, 0, 0, 1, 1},  {"gotit", 0, 0, 0, 1, 1},
    {"lmok1", 2, 1, 0, 1, 2},  {"lmok2", 0, 1, 0, 1, 2}, {"lmattn", 0, 1, 0, 1, 0},
    {"locked", 0, 0, 0, 1, 0}, {"enem", 0, 1, 1, 1, 0},
};

BOOLEAN IsValidSecondHandShot(struct SOLDIERTYPE *pSoldier);

uint8_t bHealthStrRanges[] = {15, 30, 45, 60, 75, 90, 101};

int16_t gsTerrainTypeSpeedModifiers[] = {
    5,   // Flat ground
    5,   // Floor
    5,   // Paved road
    5,   // Dirt road
    10,  // LOW GRASS
    15,  // HIGH GRASS
    20,  // TRAIN TRACKS
    20,  // LOW WATER
    25,  // MID WATER
    30   // DEEP WATER
};

// Kris:
// Temporary for testing the speed of the translucency.  Pressing Ctrl+L in turn based
// input will toggle this flag.  When clear, the translucency checking is turned off to
// increase the speed of the game.
BOOLEAN gfCalcTranslucency = FALSE;

int16_t gsFullTileDirections[MAX_FULLTILE_DIRECTIONS] = {-1, -WORLD_COLS - 1, -WORLD_COLS

};

// Palette ranges
uint32_t guiNumPaletteSubRanges;
PaletteSubRangeType *guipPaletteSubRanges = NULL;
// Palette replacements
uint32_t guiNumReplacements;
PaletteReplacementType *guipPaletteReplacements = NULL;
PaletteReplacementType *gpPalRep;
PaletteSubRangeType *gpPaletteSubRanges;

uint8_t *gubpNumReplacementsPerRange;

extern BOOLEAN fReDrawFace;
extern uint8_t gubWaitingForAllMercsToExitCode;
BOOLEAN gfGetNewPathThroughPeople = FALSE;

// LOCAL FUNCTIONS
// DO NOT CALL UNLESS THROUGH EVENT_SetSoldierPosition
uint16_t PickSoldierReadyAnimation(struct SOLDIERTYPE *pSoldier, BOOLEAN fEndReady);
BOOLEAN CheckForFullStruct(int16_t sGridNo, uint16_t *pusIndex);
void SetSoldierLocatorOffsets(struct SOLDIERTYPE *pSoldier);
void CheckForFullStructures(struct SOLDIERTYPE *pSoldier);
BOOLEAN InitNewSoldierState(struct SOLDIERTYPE *pSoldier, uint8_t ubNewState,
                            uint16_t usStartingAniCode);
uint16_t GetNewSoldierStateFromNewStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance);
void SetSoldierAniSpeed(struct SOLDIERTYPE *pSoldier);
void AdjustForFastTurnAnimation(struct SOLDIERTYPE *pSoldier);
uint16_t SelectFireAnimation(struct SOLDIERTYPE *pSoldier, uint8_t ubHeight);
void SelectFallAnimation(struct SOLDIERTYPE *pSoldier);
BOOLEAN FullStructAlone(int16_t sGridNo, uint8_t ubRadius);
void SoldierGotHitGunFire(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                          uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                          uint8_t ubSpecial, uint8_t ubHitLocation);
void SoldierGotHitBlade(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                        uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                        uint8_t ubSpecial, uint8_t ubHitLocation);
void SoldierGotHitPunch(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                        uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                        uint8_t ubSpecial, uint8_t ubHitLocation);
void SoldierGotHitExplosion(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                            uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                            uint8_t ubSpecial, uint8_t ubHitLocation);
uint8_t CalcScreamVolume(struct SOLDIERTYPE *pSoldier, uint8_t ubCombinedLoss);
void PlaySoldierFootstepSound(struct SOLDIERTYPE *pSoldier);
void HandleSystemNewAISituation(struct SOLDIERTYPE *pSoldier, BOOLEAN fResetABC);

uint16_t *CreateEnemyGlow16BPPPalette(struct JPaletteEntry *pPalette, uint32_t rscale,
                                      uint32_t gscale, BOOLEAN fAdjustGreen);
uint16_t *CreateEnemyGreyGlow16BPPPalette(struct JPaletteEntry *pPalette, uint32_t rscale,
                                          uint32_t gscale, BOOLEAN fAdjustGreen);

void SoldierBleed(struct SOLDIERTYPE *pSoldier, BOOLEAN fBandagedBleed);
int32_t CheckBleeding(struct SOLDIERTYPE *pSoldier);

void EVENT_InternalSetSoldierDesiredDirection(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection,
                                              BOOLEAN fInitalMove, uint16_t usAnimState);

#ifdef JA2BETAVERSION
extern void ValidatePlayersAreInOneGroupOnly();
extern void MapScreenDefaultOkBoxCallback(uint8_t bExitValue);
void SAIReportError(wchar_t *wErrorString);
#endif

uint32_t SleepDartSuccumbChance(struct SOLDIERTYPE *pSoldier);

void EnableDisableSoldierLightEffects(BOOLEAN fEnableLights);
void SetSoldierPersonalLightLevel(struct SOLDIERTYPE *pSoldier);

void HandleVehicleMovementSound(struct SOLDIERTYPE *pSoldier, BOOLEAN fOn) {
  VEHICLETYPE *pVehicle = &(pVehicleList[pSoldier->bVehicleID]);

  if (fOn) {
    if (pVehicle->iMovementSoundID == NO_SAMPLE) {
      pVehicle->iMovementSoundID =
          PlayJA2Sample(pVehicle->iMoveSound, RATE_11025,
                        SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo));
    }
  } else {
    if (pVehicle->iMovementSoundID != NO_SAMPLE) {
      SoundStop(pVehicle->iMovementSoundID);
      pVehicle->iMovementSoundID = NO_SAMPLE;
    }
  }
}

void AdjustNoAPToFinishMove(struct SOLDIERTYPE *pSoldier, BOOLEAN fSet) {
  if (pSoldier->ubBodyType == CROW) {
    return;
  }

  // Check if we are a vehicle first....
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    // Turn off sound effects....
    if (fSet) {
      HandleVehicleMovementSound(pSoldier, FALSE);
    }
  }

  // Turn off sound effects....
  if (fSet) {
    // Position light....
    // SetCheckSoldierLightFlag( pSoldier );
  } else {
    // DeleteSoldierLight( pSoldier );
  }

  pSoldier->fNoAPToFinishMove = fSet;

  if (!fSet) {
    // return reason to default value
    pSoldier->ubReasonCantFinishMove = REASON_STOPPED_NO_APS;
  }
}

void HandleCrowShadowVisibility(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->ubBodyType == CROW) {
    if (pSoldier->usAnimState == CROW_FLY) {
      if (pSoldier->pAniTile != NULL) {
        if (pSoldier->bLastRenderVisibleValue != -1) {
          HideAniTile(pSoldier->pAniTile, FALSE);
        } else {
          HideAniTile(pSoldier->pAniTile, TRUE);
        }
      }
    }
  }
}

void HandleCrowShadowNewGridNo(struct SOLDIERTYPE *pSoldier) {
  ANITILE_PARAMS AniParams;

  memset(&AniParams, 0, sizeof(ANITILE_PARAMS));

  if (pSoldier->ubBodyType == CROW) {
    if (pSoldier->pAniTile != NULL) {
      DeleteAniTile(pSoldier->pAniTile);
      pSoldier->pAniTile = NULL;
    }

    if (pSoldier->sGridNo != NOWHERE) {
      if (pSoldier->usAnimState == CROW_FLY) {
        AniParams.sGridNo = (int16_t)pSoldier->sGridNo;
        AniParams.ubLevelID = ANI_SHADOW_LEVEL;
        AniParams.sDelay = pSoldier->sAniDelay;
        AniParams.sStartFrame = 0;
        AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_LOOPING |
                            ANITILE_USE_DIRECTION_FOR_START_FRAME;
        AniParams.sX = pSoldier->sX;
        AniParams.sY = pSoldier->sY;
        AniParams.sZ = 0;
        strcpy(AniParams.zCachedFile, "TILECACHE\\FLY_SHDW.STI");

        AniParams.uiUserData3 = pSoldier->bDirection;

        pSoldier->pAniTile = CreateAnimationTile(&AniParams);

        HandleCrowShadowVisibility(pSoldier);
      }
    }
  }
}

void HandleCrowShadowRemoveGridNo(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->ubBodyType == CROW) {
    if (pSoldier->usAnimState == CROW_FLY) {
      if (pSoldier->pAniTile != NULL) {
        DeleteAniTile(pSoldier->pAniTile);
        pSoldier->pAniTile = NULL;
      }
    }
  }
}

void HandleCrowShadowNewDirection(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->ubBodyType == CROW) {
    if (pSoldier->usAnimState == CROW_FLY) {
      if (pSoldier->pAniTile != NULL) {
        pSoldier->pAniTile->uiUserData3 = pSoldier->bDirection;
      }
    }
  }
}

void HandleCrowShadowNewPosition(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->ubBodyType == CROW) {
    if (pSoldier->usAnimState == CROW_FLY) {
      if (pSoldier->pAniTile != NULL) {
        pSoldier->pAniTile->sRelativeX = pSoldier->sX;
        pSoldier->pAniTile->sRelativeY = pSoldier->sY;
      }
    }
  }
}

int8_t CalcActionPoints(struct SOLDIERTYPE *pSold) {
  uint8_t ubPoints, ubMaxAPs;
  int8_t bBandage;

  // dead guys don't get any APs (they shouldn't be here asking for them!)
  if (!pSold->bLife) return (0);

  // people with sleep dart drug who have collapsed get no APs
  if ((pSold->bSleepDrugCounter > 0) && pSold->bCollapsed) return (0);

  // Calculate merc's action points at 100% capability (range is 10 - 25)
  // round fractions of .5 up (that's why the +20 before the division!
  ubPoints = 5 + (((10 * EffectiveExpLevel(pSold) + 3 * EffectiveAgility(pSold) +
                    2 * pSold->bLifeMax + 2 * EffectiveDexterity(pSold)) +
                   20) /
                  40);

  // if (GameOption[INCREASEDAP] % 2 == 1)
  // points += AP_INCREASE;

  // Calculate bandage
  bBandage = pSold->bLifeMax - pSold->bLife - pSold->bBleeding;

  // If injured, reduce action points accordingly (by up to 2/3rds)
  if (pSold->bLife < pSold->bLifeMax) {
    ubPoints -=
        (2 * ubPoints * (pSold->bLifeMax - pSold->bLife + (bBandage / 2))) / (3 * pSold->bLifeMax);
  }

  // If tired, reduce action points accordingly (by up to 1/2)
  if (pSold->bBreath < 100) ubPoints -= (ubPoints * (100 - pSold->bBreath)) / 200;

  if (pSold->sWeightCarriedAtTurnStart > 100) {
    ubPoints = (uint8_t)(((uint32_t)ubPoints) * 100 / pSold->sWeightCarriedAtTurnStart);
  }

  // If resulting APs are below our permitted minimum, raise them to it!
  if (ubPoints < AP_MINIMUM) ubPoints = AP_MINIMUM;

  // make sure action points doesn't exceed the permitted maximum
  ubMaxAPs = gubMaxActionPoints[pSold->ubBodyType];

  // if (GameOption[INCREASEDAP] % 2 == 1)
  // maxAPs += AP_INCREASE;

  // If resulting APs are below our permitted minimum, raise them to it!
  if (ubPoints > ubMaxAPs) ubPoints = ubMaxAPs;

  if (pSold->ubBodyType == BLOODCAT) {
    // use same as young monsters
    ubPoints = (ubPoints * AP_YOUNG_MONST_FACTOR) / 10;
  } else if (pSold->uiStatusFlags & SOLDIER_MONSTER) {
    // young monsters get extra APs
    if (pSold->ubBodyType == YAF_MONSTER || pSold->ubBodyType == YAM_MONSTER ||
        pSold->ubBodyType == INFANT_MONSTER) {
      ubPoints = (ubPoints * AP_YOUNG_MONST_FACTOR) / 10;
    }

    // if frenzied, female monsters get more APs! (for young females, cumulative!)
    if (pSold->bFrenzied) {
      ubPoints = (ubPoints * AP_MONST_FRENZY_FACTOR) / 10;
    }
  }

  // adjust APs for phobia situations
  if (pSold->ubProfile != NO_PROFILE) {
    if ((gMercProfiles[pSold->ubProfile].bPersonalityTrait == CLAUSTROPHOBIC) &&
        (gbWorldSectorZ > 0)) {
      ubPoints = (ubPoints * AP_CLAUSTROPHOBE) / 10;
    } else if ((gMercProfiles[pSold->ubProfile].bPersonalityTrait == FEAR_OF_INSECTS) &&
               (MercSeesCreature(pSold))) {
      ubPoints = (ubPoints * AP_AFRAID_OF_INSECTS) / 10;
    }
  }

  // Adjusat APs due to drugs...
  HandleAPEffectDueToDrugs(pSold, &ubPoints);

  // If we are a vehicle, adjust APS...
  if (pSold->uiStatusFlags & SOLDIER_VEHICLE) {
    AdjustVehicleAPs(pSold, &ubPoints);
  }

  // if we are in boxing mode, adjust APs... THIS MUST BE LAST!
  if (gTacticalStatus.bBoxingState == BOXING || gTacticalStatus.bBoxingState == PRE_BOXING) {
    ubPoints /= 2;
  }

  return (ubPoints);
}

void CalcNewActionPoints(struct SOLDIERTYPE *pSoldier) {
  if (gTacticalStatus.bBoxingState == BOXING || gTacticalStatus.bBoxingState == PRE_BOXING) {
    // if we are in boxing mode, carry 1/2 as many points
    if (pSoldier->bActionPoints > MAX_AP_CARRIED / 2) {
      pSoldier->bActionPoints = MAX_AP_CARRIED / 2;
    }
  } else {
    if (pSoldier->bActionPoints > MAX_AP_CARRIED) {
      pSoldier->bActionPoints = MAX_AP_CARRIED;
    }
  }

  pSoldier->bActionPoints += CalcActionPoints(pSoldier);

  // Don't max out if we are drugged....
  if (!GetDrugEffect(pSoldier, DRUG_TYPE_ADRENALINE)) {
    pSoldier->bActionPoints =
        min(pSoldier->bActionPoints, gubMaxActionPoints[pSoldier->ubBodyType]);
  }

  pSoldier->bInitialActionPoints = pSoldier->bActionPoints;
}

void DoNinjaAttack(struct SOLDIERTYPE *pSoldier) {
  // uint32_t						uiMercFlags;
  uint16_t usSoldierIndex;
  struct SOLDIERTYPE *pTSoldier;
  uint8_t ubTDirection;
  uint8_t ubTargetStance;

  usSoldierIndex = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bLevel);
  if (usSoldierIndex != NOBODY) {
    GetSoldier(&pTSoldier, usSoldierIndex);

    // Look at stance of target
    ubTargetStance = gAnimControl[pTSoldier->usAnimState].ubEndHeight;

    // Get his life...if < certain value, do finish!
    if ((pTSoldier->bLife <= 30 || pTSoldier->bBreath <= 30) && ubTargetStance != ANIM_PRONE) {
      // Do finish!
      ChangeSoldierState(pSoldier, NINJA_SPINKICK, 0, FALSE);
    } else {
      if (ubTargetStance != ANIM_PRONE) {
        if (Random(2) == 0) {
          ChangeSoldierState(pSoldier, NINJA_LOWKICK, 0, FALSE);
        } else {
          ChangeSoldierState(pSoldier, NINJA_PUNCH, 0, FALSE);
        }

        // CHECK IF HE CAN SEE US, IF SO CHANGE DIRECTION
        if (pTSoldier->bOppList[pSoldier->ubID] == 0 && pTSoldier->bTeam != pSoldier->bTeam) {
          if (!(pTSoldier->uiStatusFlags & (SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE))) {
            ubTDirection = (uint8_t)GetDirectionFromGridNo(pSoldier->sGridNo, pTSoldier);
            SendSoldierSetDesiredDirectionEvent(pTSoldier, ubTDirection);
          }
        }
      } else {
        // CHECK OUR STANCE
        if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_CROUCH) {
          // SET DESIRED STANCE AND SET PENDING ANIMATION
          SendChangeSoldierStanceEvent(pSoldier, ANIM_CROUCH);
          pSoldier->usPendingAnimation = PUNCH_LOW;
        } else {
          // USE crouched one
          // NEED TO CHANGE STANCE IF NOT CROUCHD!
          EVENT_InitNewSoldierAnim(pSoldier, PUNCH_LOW, 0, FALSE);
        }
      }
    }
  }

  if (GetSolProfile(pSoldier) == 33) {
    uint32_t uiSoundID;
    SOUNDPARMS spParms;
    int32_t iFaceIndex;

    // Play sound!
    memset(&spParms, 0xff, sizeof(SOUNDPARMS));

    spParms.uiSpeed = RATE_11025;
    spParms.uiVolume = (int8_t)CalculateSpeechVolume(HIGHVOLUME);

    // If we are an enemy.....reduce due to volume
    if (pSoldier->bTeam != gbPlayerNum) {
      spParms.uiVolume = SoundVolume((uint8_t)spParms.uiVolume, pSoldier->sGridNo);
    }
    spParms.uiLoop = 1;
    spParms.uiPan = SoundDir(pSoldier->sGridNo);
    spParms.uiPriority = GROUP_PLAYER;

    if (pSoldier->usAnimState == NINJA_SPINKICK) {
      uiSoundID = SoundPlay("BATTLESNDS\\033_CHOP2.WAV", &spParms);
    } else {
      if (Random(2) == 0) {
        uiSoundID = SoundPlay("BATTLESNDS\\033_CHOP3.WAV", &spParms);
      } else {
        uiSoundID = SoundPlay("BATTLESNDS\\033_CHOP1.WAV", &spParms);
      }
    }

    if (uiSoundID != SOUND_ERROR) {
      pSoldier->uiBattleSoundID = uiSoundID;

      if (GetSolProfile(pSoldier) != NO_PROFILE) {
        // Get soldier's face ID
        iFaceIndex = pSoldier->iFaceIndex;

        // Check face index
        if (iFaceIndex != -1) {
          ExternSetFaceTalking(iFaceIndex, uiSoundID);
        }
      }
    }
  }
}

BOOLEAN CreateSoldierCommon(uint8_t ubBodyType, struct SOLDIERTYPE *pSoldier, uint16_t usSoldierID,
                            uint16_t usState) {
  BOOLEAN fSuccess = FALSE;
  int32_t iCounter = 0;

  // if we are loading a saved game, we DO NOT want to reset the opplist, look for enemies, or say a
  // dying commnet
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // Set initial values for opplist!
    InitSoldierOppList(pSoldier);
    HandleSight(pSoldier, SIGHT_LOOK);

    // Set some quote flags
    if (pSoldier->bLife >= OKLIFE) {
      pSoldier->fDyingComment = FALSE;
    } else {
      pSoldier->fDyingComment = TRUE;
    }
  }

  // ATE: Reset some timer flags...
  pSoldier->uiTimeSameBattleSndDone = 0;
  // ATE: Reset every time.....
  pSoldier->fSoldierWasMoving = TRUE;
  pSoldier->iTuringSoundID = NO_SAMPLE;
  pSoldier->uiTimeSinceLastBleedGrunt = 0;

  if (pSoldier->ubBodyType == QUEENMONSTER) {
    pSoldier->iPositionSndID =
        NewPositionSnd(NOWHERE, POSITION_SOUND_FROM_SOLDIER, pSoldier, QUEEN_AMBIENT_NOISE);
  }

  // ANYTHING AFTER HERE CAN FAIL
  do {
    if (usSoldierID <= gTacticalStatus.Team[OUR_TEAM].bLastID) {
      pSoldier->pKeyRing = (KEY_ON_RING *)MemAlloc(NUM_KEYS * sizeof(KEY_ON_RING));
      memset(pSoldier->pKeyRing, 0, NUM_KEYS * sizeof(KEY_ON_RING));

      for (iCounter = 0; iCounter < NUM_KEYS; iCounter++) {
        pSoldier->pKeyRing[iCounter].ubKeyID = INVALID_KEY_NUMBER;
      }
    } else {
      pSoldier->pKeyRing = NULL;
    }
    // Create frame cache
    if (InitAnimationCache(usSoldierID, &(pSoldier->AnimCache)) == FALSE) {
      DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Soldier: Failed animation cache creation"));
      break;
    }

    if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
      // Init new soldier state
      // OFFSET FIRST ANIMATION FRAME FOR NEW MERCS
      if (usState != STANDING) {
        EVENT_InitNewSoldierAnim(pSoldier, usState, (uint8_t)0, TRUE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, usState, (uint8_t)Random(10), TRUE);
      }
    } else {
      /// if we don't have a world loaded, and are in a bad anim, goto standing.
      // bad anims are: HOPFENCE,
      // CLIMBDOWNROOF, FALLFORWARD_ROOF,FALLOFF, CLIMBUPROOF
      if (!gfWorldLoaded &&
          (usState == HOPFENCE || usState == CLIMBDOWNROOF || usState == FALLFORWARD_ROOF ||
           usState == FALLOFF || usState == CLIMBUPROOF)) {
        EVENT_InitNewSoldierAnim(pSoldier, STANDING, 0, TRUE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, usState, pSoldier->usAniCode, TRUE);
      }
    }

    // if ( pSoldier->pBackGround != NULL )
    //	MemFree( pSoldier->pBackGround );

    // INIT ANIMATION DATA
    // if((pSoldier->pBackGround=MemAlloc(SOLDIER_UNBLIT_SIZE))==NULL)
    //{
    //	DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Soldier: Failed unblit memory allocation" ) );
    //	break;
    //}
    // memset(pSoldier->pBackGround, 0, SOLDIER_UNBLIT_SIZE);

    // if((pSoldier->pZBackground=MemAlloc(SOLDIER_UNBLIT_SIZE))==NULL)
    //{
    //	DebugMsg( TOPIC_JA2, DBG_LEVEL_0, String( "Soldier: Failed unblit memory allocation" ) );
    //	break;
    //}
    // memset(pSoldier->pZBackground, 0, SOLDIER_UNBLIT_SIZE);

    // Init palettes
    if (CreateSoldierPalettes(pSoldier) == FALSE) {
      DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Soldier: Failed in creating soldier palettes"));
      break;
    }

    fSuccess = TRUE;

  } while (FALSE);

  if (!fSuccess) {
    DeleteSoldier((pSoldier));
  }

  return (fSuccess);
}

BOOLEAN DeleteSoldier(struct SOLDIERTYPE *pSoldier) {
  uint32_t cnt;
  int32_t iGridNo;
  int8_t bDir;
  BOOLEAN fRet;

  if (pSoldier != NULL) {
    // if(pSoldier->pBackGround!=NULL)
    // MemFree(pSoldier->pBackGround);

    // if(pSoldier->pZBackground!=NULL)
    // MemFree(pSoldier->pZBackground);

    if (pSoldier->sGridNo != NOWHERE) {
      // Remove adjacency records
      for (bDir = 0; bDir < NUM_WORLD_DIRECTIONS; bDir++) {
        iGridNo = pSoldier->sGridNo + DirIncrementer[bDir];
        if (iGridNo >= 0 && iGridNo < WORLD_MAX) {
          gpWorldLevelData[iGridNo].ubAdjacentSoldierCnt--;
        }
      }
    }

    // Delete key ring
    if (pSoldier->pKeyRing) {
      MemFree(pSoldier->pKeyRing);
      pSoldier->pKeyRing = NULL;
    }

    // Delete faces
    DeleteSoldierFace(pSoldier);

    // FREE PALETTES
    if (pSoldier->p8BPPPalette != NULL) {
      MemFree(pSoldier->p8BPPPalette);
      pSoldier->p8BPPPalette = NULL;
    }

    if (pSoldier->p16BPPPalette != NULL) {
      MemFree(pSoldier->p16BPPPalette);
      pSoldier->p16BPPPalette = NULL;
    }

    for (cnt = 0; cnt < NUM_SOLDIER_SHADES; cnt++) {
      if (pSoldier->pShades[cnt] != NULL) {
        MemFree(pSoldier->pShades[cnt]);
        pSoldier->pShades[cnt] = NULL;
      }
    }
    for (cnt = 0; cnt < NUM_SOLDIER_EFFECTSHADES; cnt++) {
      if (pSoldier->pEffectShades[cnt] != NULL) {
        MemFree(pSoldier->pEffectShades[cnt]);
        pSoldier->pEffectShades[cnt] = NULL;
      }
    }

    // Delete glows
    for (cnt = 0; cnt < 20; cnt++) {
      if (pSoldier->pGlowShades[cnt] != NULL) {
        MemFree(pSoldier->pGlowShades[cnt]);
        pSoldier->pGlowShades[cnt] = NULL;
      }
    }

    if (pSoldier->ubBodyType == QUEENMONSTER) {
      DeletePositionSnd(pSoldier->iPositionSndID);
    }

    // Free any animations we may have locked...
    UnLoadCachedAnimationSurfaces(pSoldier->ubID, &(pSoldier->AnimCache));

    // Free Animation cache
    DeleteAnimationCache(pSoldier->ubID, &(pSoldier->AnimCache));

    // Soldier is not active
    pSoldier->bActive = FALSE;

    // Remove light
    DeleteSoldierLight(pSoldier);

    // Remove reseved movement value
    UnMarkMovementReserved(pSoldier);
  }

  // REMOVE SOLDIER FROM SLOT!
  fRet = RemoveMercSlot(pSoldier);

  if (!fRet) {
    RemoveAwaySlot(pSoldier);
  }

  return (TRUE);
}

BOOLEAN CreateSoldierLight(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->bTeam != gbPlayerNum) {
    return (FALSE);
  }

  // DO ONLY IF WE'RE AT A GOOD LEVEL
  if (pSoldier->iLight == -1) {
    // ATE: Check for goggles in headpos....
    if (pSoldier->inv[HEAD1POS].usItem == NIGHTGOGGLES ||
        pSoldier->inv[HEAD2POS].usItem == NIGHTGOGGLES) {
      if ((pSoldier->iLight = LightSpriteCreate("Light3", 0)) == (-1)) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Soldier: Failed loading light"));
        return (FALSE);
      } else {
        LightSprites[pSoldier->iLight].uiFlags |= MERC_LIGHT;
      }
    } else if (pSoldier->inv[HEAD1POS].usItem == UVGOGGLES ||
               pSoldier->inv[HEAD2POS].usItem == UVGOGGLES) {
      if ((pSoldier->iLight = LightSpriteCreate("Light4", 0)) == (-1)) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Soldier: Failed loading light"));
        return (FALSE);
      } else {
        LightSprites[pSoldier->iLight].uiFlags |= MERC_LIGHT;
      }
    } else {
      if ((pSoldier->iLight = LightSpriteCreate("Light2", 0)) == (-1)) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Soldier: Failed loading light"));
        return (FALSE);
      } else {
        LightSprites[pSoldier->iLight].uiFlags |= MERC_LIGHT;
      }
    }

    if (pSoldier->bLevel != 0) {
      LightSpriteRoofStatus(pSoldier->iLight, TRUE);
    }
  }

  return (TRUE);
}

BOOLEAN ReCreateSoldierLight(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->bTeam != gbPlayerNum) {
    return (FALSE);
  }

  if (!IsSolActive(pSoldier)) {
    return (FALSE);
  }

  if (!pSoldier->bInSector) {
    return (FALSE);
  }

  // Delete Light!
  DeleteSoldierLight(pSoldier);

  if (pSoldier->iLight == -1) {
    CreateSoldierLight(pSoldier);
  }

  return (TRUE);
}

BOOLEAN ReCreateSelectedSoldierLight() {
  struct SOLDIERTYPE *pSoldier;

  if (gusSelectedSoldier == NO_SOLDIER) {
    return (FALSE);
  }

  pSoldier = MercPtrs[gusSelectedSoldier];

  return (ReCreateSoldierLight(pSoldier));
}

BOOLEAN DeleteSoldierLight(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->iLight != (-1)) {
    LightSpriteDestroy(pSoldier->iLight);
    pSoldier->iLight = -1;
  }

  return (TRUE);
}

// FUNCTIONS CALLED BY EVENT PUMP
/////////////////////////////////

BOOLEAN ChangeSoldierState(struct SOLDIERTYPE *pSoldier, uint16_t usNewState,
                           uint16_t usStartingAniCode, BOOLEAN fForce) {
  EV_S_CHANGESTATE SChangeState;

  // Send message that we have changed states
  SChangeState.usNewState = usNewState;
  SChangeState.usSoldierID = GetSolID(pSoldier);
  SChangeState.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;
  SChangeState.usStartingAniCode = usStartingAniCode;
  SChangeState.sXPos = pSoldier->sX;
  SChangeState.sYPos = pSoldier->sY;
  SChangeState.fForce = fForce;
  SChangeState.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  // AddGameEvent( S_CHANGESTATE, 0, &SChangeState );
  EVENT_InitNewSoldierAnim(pSoldier, SChangeState.usNewState, SChangeState.usStartingAniCode,
                           SChangeState.fForce);

  return (TRUE);
}

// This function reevaluates the stance if the guy sees us!
BOOLEAN ReevaluateEnemyStance(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState) {
  int32_t cnt, iClosestEnemy = NOBODY;
  int16_t sTargetXPos, sTargetYPos;
  BOOLEAN fReturnVal = FALSE;
  int16_t sDist, sClosestDist = 10000;

  // make the chosen one not turn to face us
  if (OK_ENEMY_MERC(pSoldier) && GetSolID(pSoldier) != gTacticalStatus.ubTheChosenOne &&
      gAnimControl[usAnimState].ubEndHeight == ANIM_STAND &&
      !(pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL)) {
    if (pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_OFF) {
      // If we are a queen and see enemies, goto ready
      if (pSoldier->ubBodyType == QUEENMONSTER) {
        if (gAnimControl[usAnimState].uiFlags & (ANIM_BREATH)) {
          if (pSoldier->bOppCnt > 0) {
            EVENT_InitNewSoldierAnim(pSoldier, QUEEN_INTO_READY, 0, TRUE);
            return (TRUE);
          }
        }
      }

      // ATE: Don't do this if we're not a merc.....
      if (!IS_MERC_BODY_TYPE(pSoldier)) {
        return (FALSE);
      }

      if (gAnimControl[usAnimState].uiFlags & (ANIM_MERCIDLE | ANIM_BREATH)) {
        if (pSoldier->bOppCnt > 0) {
          // Pick a guy this buddy sees and turn towards them!
          for (cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
               cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++) {
            if (pSoldier->bOppList[cnt] == SEEN_CURRENTLY) {
              sDist = PythSpacesAway(pSoldier->sGridNo, MercPtrs[cnt]->sGridNo);
              if (sDist < sClosestDist) {
                sClosestDist = sDist;
                iClosestEnemy = cnt;
              }
            }
          }

          if (iClosestEnemy != NOBODY) {
            // Change to fire ready animation
            ConvertGridNoToXY(MercPtrs[iClosestEnemy]->sGridNo, &sTargetXPos, &sTargetYPos);

            pSoldier->fDontChargeReadyAPs = TRUE;

            // Ready weapon
            fReturnVal = SoldierReadyWeapon(pSoldier, sTargetXPos, sTargetYPos, FALSE);

            return (fReturnVal);
          }
        }
      }
    }
  }
  return (FALSE);
}

void CheckForFreeupFromHit(struct SOLDIERTYPE *pSoldier, uint32_t uiOldAnimFlags,
                           uint32_t uiNewAnimFlags, uint16_t usOldAniState, uint16_t usNewState) {
  // THIS COULD POTENTIALLY CALL EVENT_INITNEWAnim() if the GUY was SUPPRESSED
  // CHECK IF THE OLD ANIMATION WAS A HIT START THAT WAS NOT FOLLOWED BY A HIT FINISH
  // IF SO, RELEASE ATTACKER FROM ATTACKING

  // If old and new animations are the same, do nothing!
  if (usOldAniState == QUEEN_HIT && usNewState == QUEEN_HIT) {
    return;
  }

  if (usOldAniState != usNewState && (uiOldAnimFlags & ANIM_HITSTART) &&
      !(uiNewAnimFlags & ANIM_HITFINISH) && !(uiNewAnimFlags & ANIM_IGNOREHITFINISH) &&
      !(pSoldier->uiStatusFlags & SOLDIER_TURNINGFROMHIT)) {
    // Release attacker
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("@@@@@@@ Releasesoldierattacker, normal hit animation ended NEW: %s ( %d ) "
                    "OLD: %s ( %d )",
                    gAnimControl[usNewState].zAnimStr, usNewState,
                    gAnimControl[usOldAniState].zAnimStr, pSoldier->usOldAniState));
    ReleaseSoldiersAttacker(pSoldier);

    // FREEUP GETTING HIT FLAG
    pSoldier->fGettingHit = FALSE;

    // ATE: if our guy, have 10% change of say damn, if still conscious...
    if (pSoldier->bTeam == gbPlayerNum && pSoldier->bLife >= OKLIFE) {
      if (Random(10) == 0) {
        DoMercBattleSound(pSoldier, (int8_t)(BATTLE_SOUND_CURSE1));
      }
    }
  }

  // CHECK IF WE HAVE FINSIHED A HIT WHILE DOWN
  // OBLY DO THIS IF 1 ) We are dead already or 2 ) We are alive still
  if ((uiOldAnimFlags & ANIM_HITWHENDOWN) &&
      ((pSoldier->uiStatusFlags & SOLDIER_DEAD) || pSoldier->bLife != 0)) {
    // Release attacker
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("@@@@@@@ Releasesoldierattacker, animation of kill on the ground ended"));
    ReleaseSoldiersAttacker(pSoldier);

    // FREEUP GETTING HIT FLAG
    pSoldier->fGettingHit = FALSE;

    if (pSoldier->bLife == 0) {
      // ATE: Set previous attacker's value!
      // This is so that the killer can say their killed quote....
      pSoldier->ubAttackerID = pSoldier->ubPreviousAttackerID;
    }
  }
}

// THIS IS CALLED FROM AN EVENT ( S_CHANGESTATE )!
BOOLEAN EVENT_InitNewSoldierAnim(struct SOLDIERTYPE *pSoldier, uint16_t usNewState,
                                 uint16_t usStartingAniCode, BOOLEAN fForce) {
  int16_t sAPCost = 0;
  int16_t sBPCost = 0;
  uint32_t uiOldAnimFlags;
  uint32_t uiNewAnimFlags;
  uint16_t usSubState;
  uint16_t usItem;
  BOOLEAN fTryingToRestart = FALSE;

  CHECKF(usNewState < NUMANIMATIONSTATES);

  ///////////////////////////////////////////////////////////////////////
  //			DO SOME CHECKS ON OUR NEW ANIMATION!
  /////////////////////////////////////////////////////////////////////

  // If we are NOT loading a game, continue normally
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    // CHECK IF WE ARE TRYING TO INTURRUPT A SCRIPT WHICH WE DO NOT WANT INTERRUPTED!
    if (pSoldier->fInNonintAnim) {
      return (FALSE);
    }

    if (pSoldier->fRTInNonintAnim) {
      if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
        return (FALSE);
      } else {
        pSoldier->fRTInNonintAnim = FALSE;
      }
    }

    // Check if we can restart this animation if it's the same as our current!
    if (usNewState == pSoldier->usAnimState) {
      if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_NORESTART) && !fForce) {
        fTryingToRestart = TRUE;
      }
    }

    // Check state, if we are not at the same height, set this ani as the pending one and
    // change stance accordingly
    // ATE: ONLY IF WE ARE STARTING AT START OF ANIMATION!
    if (usStartingAniCode == 0) {
      if (gAnimControl[usNewState].ubHeight != gAnimControl[pSoldier->usAnimState].ubEndHeight &&
          !(gAnimControl[usNewState].uiFlags & (ANIM_STANCECHANGEANIM | ANIM_IGNORE_AUTOSTANCE))) {
        // Check if we are going from crouched height to prone height, and adjust fast turning
        // accordingly Make guy turn while crouched THEN go into prone
        if ((gAnimControl[usNewState].ubEndHeight == ANIM_PRONE &&
             gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH) &&
            !(gTacticalStatus.uiFlags & INCOMBAT)) {
          pSoldier->fTurningUntilDone = TRUE;
          pSoldier->ubPendingStanceChange = gAnimControl[usNewState].ubEndHeight;
          pSoldier->usPendingAnimation = usNewState;
          return (TRUE);
        }
        // Check if we are in realtime and we are going from stand to crouch
        else if (gAnimControl[usNewState].ubEndHeight == ANIM_CROUCH &&
                 gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND &&
                 (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) &&
                 ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT))) {
          pSoldier->ubDesiredHeight = gAnimControl[usNewState].ubEndHeight;
          // Continue with this course of action IE: Do animation and skip from stand to crouch
        }
        // Check if we are in realtime and we are going from crouch to stand
        else if (gAnimControl[usNewState].ubEndHeight == ANIM_STAND &&
                 gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH &&
                 (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) &&
                 ((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) &&
                 pSoldier->usAnimState != HELIDROP) {
          pSoldier->ubDesiredHeight = gAnimControl[usNewState].ubEndHeight;
          // Continue with this course of action IE: Do animation and skip from stand to crouch
        } else {
          // ONLY DO FOR EVERYONE BUT PLANNING GUYS
          if (pSoldier->ubID < MAX_NUM_SOLDIERS) {
            // Set our next moving animation to be pending, after
            pSoldier->usPendingAnimation = usNewState;
            // Set new state to be animation to move to new stance
            SendChangeSoldierStanceEvent(pSoldier, gAnimControl[usNewState].ubHeight);
            return (TRUE);
          }
        }
      }
    }

    if (usNewState == ADJACENT_GET_ITEM) {
      if (pSoldier->ubPendingDirection != NO_PENDING_DIRECTION) {
        EVENT_InternalSetSoldierDesiredDirection(pSoldier, pSoldier->ubPendingDirection, FALSE,
                                                 pSoldier->usAnimState);
        pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
        pSoldier->usPendingAnimation = ADJACENT_GET_ITEM;
        pSoldier->fTurningUntilDone = TRUE;
        SoldierGotoStationaryStance(pSoldier);
        return (TRUE);
      }
    }

    if (usNewState == CLIMBUPROOF) {
      if (pSoldier->ubPendingDirection != NO_PENDING_DIRECTION) {
        EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->ubPendingDirection);
        pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
        pSoldier->usPendingAnimation = CLIMBUPROOF;
        pSoldier->fTurningUntilDone = TRUE;
        SoldierGotoStationaryStance(pSoldier);
        return (TRUE);
      }
    }

    if (usNewState == CLIMBDOWNROOF) {
      if (pSoldier->ubPendingDirection != NO_PENDING_DIRECTION) {
        EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->ubPendingDirection);
        pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
        pSoldier->usPendingAnimation = CLIMBDOWNROOF;
        pSoldier->fTurningFromPronePosition = FALSE;
        pSoldier->fTurningUntilDone = TRUE;
        SoldierGotoStationaryStance(pSoldier);
        return (TRUE);
      }
    }

    // ATE: Don't raise/lower automatically if we are low on health,
    // as our gun looks lowered anyway....
    // if ( pSoldier->bLife > INJURED_CHANGE_THREASHOLD )
    {
      // Don't do some of this if we are a monster!
      // ATE: LOWER AIMATION IS GOOD, RAISE ONE HOWEVER MAY CAUSE PROBLEMS FOR AI....
      if (!(pSoldier->uiStatusFlags & SOLDIER_MONSTER) && pSoldier->ubBodyType != ROBOTNOWEAPON &&
          pSoldier->bTeam == gbPlayerNum) {
        // If this animation is a raise_weapon animation
        if ((gAnimControl[usNewState].uiFlags & ANIM_RAISE_WEAPON) &&
            !(gAnimControl[pSoldier->usAnimState].uiFlags &
              (ANIM_RAISE_WEAPON | ANIM_NOCHANGE_WEAPON))) {
          // We are told that we need to rasie weapon
          // Do so only if
          // 1) We have a rifle in hand...
          usItem = pSoldier->inv[HANDPOS].usItem;

          if (usItem != NOTHING && (Item[usItem].fFlags & ITEM_TWO_HANDED) &&
              usItem != ROCKET_LAUNCHER) {
            // Switch on height!
            switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
              case ANIM_STAND:

                // 2) OK, all's fine... lower weapon first....
                pSoldier->usPendingAnimation = usNewState;
                // Set new state to be animation to move to new stance
                usNewState = RAISE_RIFLE;
            }
          }
        }

        // If this animation is a lower_weapon animation
        if ((gAnimControl[usNewState].uiFlags & ANIM_LOWER_WEAPON) &&
            !(gAnimControl[pSoldier->usAnimState].uiFlags &
              (ANIM_LOWER_WEAPON | ANIM_NOCHANGE_WEAPON))) {
          // We are told that we need to rasie weapon
          // Do so only if
          // 1) We have a rifle in hand...
          usItem = pSoldier->inv[HANDPOS].usItem;

          if (usItem != NOTHING && (Item[usItem].fFlags & ITEM_TWO_HANDED) &&
              usItem != ROCKET_LAUNCHER) {
            // Switch on height!
            switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
              case ANIM_STAND:

                // 2) OK, all's fine... lower weapon first....
                pSoldier->usPendingAnimation = usNewState;
                // Set new state to be animation to move to new stance
                usNewState = LOWER_RIFLE;
            }
          }
        }
      }
    }

    // Are we cowering and are tyring to move, getup first...
    if (gAnimControl[usNewState].uiFlags & ANIM_MOVING && pSoldier->usAnimState == COWERING &&
        gAnimControl[usNewState].ubEndHeight == ANIM_STAND) {
      pSoldier->usPendingAnimation = usNewState;
      // Set new state to be animation to move to new stance
      usNewState = END_COWER;
    }

    // If we want to start swatting, put a pending animation
    if (pSoldier->usAnimState != START_SWAT && usNewState == SWATTING) {
      // Set new state to be animation to move to new stance
      usNewState = START_SWAT;
    }

    if (pSoldier->usAnimState == SWATTING && usNewState == CROUCHING) {
      // Set new state to be animation to move to new stance
      usNewState = END_SWAT;
    }

    if (pSoldier->usAnimState == WALKING && usNewState == STANDING &&
        pSoldier->bLife < INJURED_CHANGE_THREASHOLD && pSoldier->ubBodyType <= REGFEMALE &&
        !MercInWater(pSoldier)) {
      // Set new state to be animation to move to new stance
      usNewState = END_HURT_WALKING;
    }

    // Check if we are an enemy, and we are in an animation what should be overriden
    // by if he sees us or not.
    if (ReevaluateEnemyStance(pSoldier, usNewState)) {
      return (TRUE);
    }

    // OK.......
    if (pSoldier->ubBodyType > REGFEMALE) {
      if (pSoldier->bLife < INJURED_CHANGE_THREASHOLD) {
        if (usNewState == READY_RIFLE_STAND) {
          //	pSoldier->usPendingAnimation2 = usNewState;
          //	usNewState = FROM_INJURED_TRANSITION;
        }
      }
    }

    // Alrighty, check if we should free buddy up!
    if (usNewState == GIVING_AID) {
      UnSetUIBusy(pSoldier->ubID);
    }

    // SUBSTITUDE VARIOUS REG ANIMATIONS WITH ODD BODY TYPES
    if (SubstituteBodyTypeAnimation(pSoldier, usNewState, &usSubState)) {
      usNewState = usSubState;
    }

    // CHECK IF WE CAN DO THIS ANIMATION!
    if (IsAnimationValidForBodyType(pSoldier, usNewState) == FALSE) {
      return (FALSE);
    }

    // OK, make guy transition if a big merc...
    if (pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYTHREATENSTANCE) {
      if (usNewState == KNEEL_DOWN && pSoldier->usAnimState != BIGMERC_CROUCH_TRANS_INTO) {
        uint16_t usItem;

        // Do we have a rifle?
        usItem = pSoldier->inv[HANDPOS].usItem;

        if (usItem != NOTHING) {
          if (Item[usItem].usItemClass == IC_GUN && usItem != ROCKET_LAUNCHER) {
            if ((Item[usItem].fFlags & ITEM_TWO_HANDED)) {
              usNewState = BIGMERC_CROUCH_TRANS_INTO;
            }
          }
        }
      }

      if (usNewState == KNEEL_UP && pSoldier->usAnimState != BIGMERC_CROUCH_TRANS_OUTOF) {
        uint16_t usItem;

        // Do we have a rifle?
        usItem = pSoldier->inv[HANDPOS].usItem;

        if (usItem != NOTHING) {
          if (Item[usItem].usItemClass == IC_GUN && usItem != ROCKET_LAUNCHER) {
            if ((Item[usItem].fFlags & ITEM_TWO_HANDED)) {
              usNewState = BIGMERC_CROUCH_TRANS_OUTOF;
            }
          }
        }
      }
    }

    // OK, if we have reverse set, do the side step!
    if (pSoldier->bReverse) {
      if (usNewState == WALKING || usNewState == RUNNING || usNewState == SWATTING) {
        // CHECK FOR SIDEWAYS!
        if (pSoldier->bDirection ==
            gPurpendicularDirection[pSoldier->bDirection]
                                   [pSoldier->usPathingData[pSoldier->usPathIndex]]) {
          // We are perpendicular!
          usNewState = SIDE_STEP;
        } else {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH) {
            usNewState = SWAT_BACKWARDS;
          } else {
            // Here, change to  opposite direction
            usNewState = WALK_BACKWARDS;
          }
        }
      }
    }

    // ATE: Patch hole for breath collapse for roofs, fences
    if (usNewState == CLIMBUPROOF || usNewState == CLIMBDOWNROOF || usNewState == HOPFENCE) {
      // Check for breath collapse if a given animation like
      if (CheckForBreathCollapse(pSoldier) || pSoldier->bCollapsed) {
        // UNset UI
        UnSetUIBusy(pSoldier->ubID);

        SoldierCollapse(pSoldier);

        pSoldier->bBreathCollapsed = FALSE;

        return (FALSE);
      }
    }

    // If we are in water.....and trying to run, change to run
    if (pSoldier->bOverTerrainType == LOW_WATER || pSoldier->bOverTerrainType == MED_WATER) {
      // Check animation
      // Change to walking
      if (usNewState == RUNNING) {
        usNewState = WALKING;
      }
    }

    // Turn off anipause flag for any anim!
    pSoldier->uiStatusFlags &= (~SOLDIER_PAUSEANIMOVE);

    // Unset paused for no APs.....
    AdjustNoAPToFinishMove(pSoldier, FALSE);

    if (usNewState == CRAWLING && pSoldier->usDontUpdateNewGridNoOnMoveAnimChange == 1) {
      if (pSoldier->fTurningFromPronePosition != TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE) {
        pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_START_UP_FROM_MOVE;
      }

      // ATE: IF we are starting to crawl, but have to getup to turn first......
      if (pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_START_UP_FROM_MOVE) {
        usNewState = PRONE_UP;
        pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE;
      }
    }

    // We are about to start moving
    // Handle buddy beginning to move...
    // check new gridno, etc
    // ATE: Added: Make check that old anim is not a moving one as well
    if ((gAnimControl[usNewState].uiFlags & ANIM_MOVING &&
         !(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING)) ||
        (gAnimControl[usNewState].uiFlags & ANIM_MOVING && fForce)) {
      BOOLEAN fKeepMoving;

      if (usNewState == CRAWLING &&
          pSoldier->usDontUpdateNewGridNoOnMoveAnimChange == LOCKED_NO_NEWGRIDNO) {
        // Turn off lock once we are crawling once...
        pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = 1;
      }

      // ATE: Additional check here if we have just been told to update animation ONLY, not goto
      // gridno stuff...
      if (!pSoldier->usDontUpdateNewGridNoOnMoveAnimChange) {
        if (usNewState != SWATTING) {
          DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                   String("Handling New gridNo for %d: Old %s, New %s", GetSolID(pSoldier),
                          gAnimControl[pSoldier->usAnimState].zAnimStr,
                          gAnimControl[usNewState].zAnimStr));

          if (!(gAnimControl[usNewState].uiFlags & ANIM_SPECIALMOVE)) {
            // Handle goto new tile...
            if (HandleGotoNewGridNo(pSoldier, &fKeepMoving, TRUE, usNewState)) {
              if (!fKeepMoving) {
                return (FALSE);
              }

              // Make sure desy = zeroed out...
              // pSoldier->fPastXDest = pSoldier->fPastYDest = FALSE;
            } else {
              if (pSoldier->bBreathCollapsed) {
                // UNset UI
                UnSetUIBusy(pSoldier->ubID);

                SoldierCollapse(pSoldier);

                pSoldier->bBreathCollapsed = FALSE;
              }
              return (FALSE);
            }
          } else {
            // Change desired direction
            // Just change direction
            EVENT_InternalSetSoldierDestination(pSoldier,
                                                pSoldier->usPathingData[pSoldier->usPathIndex],
                                                FALSE, pSoldier->usAnimState);
          }

          // check for services
          ReceivingSoldierCancelServices(pSoldier);
          GivingSoldierCancelServices(pSoldier);

          // Check if we are a vehicle, and start playing noise sound....
          if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
            HandleVehicleMovementSound(pSoldier, TRUE);
          }
        }
      }
    } else {
      // Check for stopping movement noise...
      if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
        HandleVehicleMovementSound(pSoldier, FALSE);

        // If a vehicle, set hewight to 0
        SetSoldierHeight(pSoldier, (float)(0));
      }
    }

    // Reset to false always.....
    // ( Unless locked )
    if (gAnimControl[usNewState].uiFlags & ANIM_MOVING) {
      if (pSoldier->usDontUpdateNewGridNoOnMoveAnimChange != LOCKED_NO_NEWGRIDNO) {
        pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = FALSE;
      }
    }

    if (fTryingToRestart) {
      return (FALSE);
    }
  }

  // ATE: If this is an AI guy.. unlock him!
  if (gTacticalStatus.fEnemySightingOnTheirTurn) {
    if (gTacticalStatus.ubEnemySightingOnTheirTurnEnemyID == GetSolID(pSoldier)) {
      pSoldier->fPauseAllAnimation = FALSE;
      gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
    }
  }

  ///////////////////////////////////////////////////////////////////////
  //			HERE DOWN - WE HAVE MADE A DESCISION!
  /////////////////////////////////////////////////////////////////////

  uiOldAnimFlags = gAnimControl[pSoldier->usAnimState].uiFlags;
  uiNewAnimFlags = gAnimControl[usNewState].uiFlags;

  // CHECKING IF WE HAVE A HIT FINISH BUT NO DEATH IS DONE WITH A SPECIAL ANI CODE
  // IN THE HIT FINSIH ANI SCRIPTS

  // CHECKING IF WE HAVE FINISHED A DEATH ANIMATION IS DONE WITH A SPECIAL ANI CODE
  // IN THE DEATH SCRIPTS

  // CHECK IF THIS NEW STATE IS NON-INTERRUPTABLE
  // IF SO - SET NON-INT FLAG
  if (uiNewAnimFlags & ANIM_NONINTERRUPT) {
    pSoldier->fInNonintAnim = TRUE;
  }

  if (uiNewAnimFlags & ANIM_RT_NONINTERRUPT) {
    pSoldier->fRTInNonintAnim = TRUE;
  }

  // CHECK IF WE ARE NOT AIMING, IF NOT, RESET LAST TAGRET!
  if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIREREADY) &&
      !(gAnimControl[usNewState].uiFlags & ANIM_FIREREADY)) {
    // ATE: Also check for the transition anims to not reset this
    // this should have used a flag but we're out of them....
    if (usNewState != READY_RIFLE_STAND && usNewState != READY_RIFLE_PRONE &&
        usNewState != READY_RIFLE_CROUCH && usNewState != ROBOT_SHOOT) {
      pSoldier->sLastTarget = NOWHERE;
    }
  }

  // If a special move state, release np aps
  if ((gAnimControl[usNewState].uiFlags & ANIM_SPECIALMOVE)) {
    AdjustNoAPToFinishMove(pSoldier, FALSE);
  }

  if (gAnimControl[usNewState].uiFlags & ANIM_UPDATEMOVEMENTMODE) {
    if (pSoldier->bTeam == gbPlayerNum) {
      // pSoldier->usUIMovementMode =  GetMoveStateBasedOnStance( pSoldier, gAnimControl[ usNewState
      // ].ubEndHeight );
    }
  }

  // ATE: If not a moving animation - turn off reverse....
  if (!(gAnimControl[usNewState].uiFlags & ANIM_MOVING)) {
    pSoldier->bReverse = FALSE;
  }

  // ONLY DO FOR EVERYONE BUT PLANNING GUYS
  if (pSoldier->ubID < MAX_NUM_SOLDIERS) {
    // Do special things based on new state
    switch (usNewState) {
      case STANDING:

        // Update desired height
        pSoldier->ubDesiredHeight = ANIM_STAND;
        break;

      case CROUCHING:

        // Update desired height
        pSoldier->ubDesiredHeight = ANIM_CROUCH;
        break;

      case PRONE:

        // Update desired height
        pSoldier->ubDesiredHeight = ANIM_PRONE;
        break;

      case READY_RIFLE_STAND:
      case READY_RIFLE_PRONE:
      case READY_RIFLE_CROUCH:
      case READY_DUAL_STAND:
      case READY_DUAL_CROUCH:
      case READY_DUAL_PRONE:

        // OK, get points to ready weapon....
        if (!pSoldier->fDontChargeReadyAPs) {
          sAPCost = GetAPsToReadyWeapon(pSoldier, usNewState);
          DeductPoints(pSoldier, sAPCost, sBPCost);
        } else {
          pSoldier->fDontChargeReadyAPs = FALSE;
        }
        break;

      case WALKING:

        pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
        pSoldier->ubPendingActionAnimCount = 0;
        break;

      case SWATTING:

        pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
        pSoldier->ubPendingActionAnimCount = 0;
        break;

      case CRAWLING:

        // Turn off flag...
        pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
        pSoldier->ubPendingActionAnimCount = 0;
        pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
        break;

      case RUNNING:

        // Only if our previous is not running
        if (pSoldier->usAnimState != RUNNING) {
          sAPCost = AP_START_RUN_COST;
          DeductPoints(pSoldier, sAPCost, sBPCost);
        }
        // Set pending action count to 0
        pSoldier->ubPendingActionAnimCount = 0;
        pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
        break;

      case ADULTMONSTER_WALKING:
        pSoldier->ubPendingActionAnimCount = 0;
        break;

      case ROBOT_WALK:
        pSoldier->ubPendingActionAnimCount = 0;
        break;

      case KNEEL_UP:
      case KNEEL_DOWN:
      case BIGMERC_CROUCH_TRANS_INTO:
      case BIGMERC_CROUCH_TRANS_OUTOF:

        if (!pSoldier->fDontChargeAPsForStanceChange) {
          DeductPoints(pSoldier, AP_CROUCH, BP_CROUCH);
        }
        pSoldier->fDontChargeAPsForStanceChange = FALSE;
        break;

      case PRONE_UP:
      case PRONE_DOWN:

        // ATE: If we are NOT waiting for prone down...
        if (pSoldier->fTurningFromPronePosition < TURNING_FROM_PRONE_START_UP_FROM_MOVE &&
            !pSoldier->fDontChargeAPsForStanceChange) {
          // ATE: Don't do this if we are still 'moving'....
          if (pSoldier->sGridNo == pSoldier->sFinalDestination || pSoldier->usPathIndex == 0) {
            DeductPoints(pSoldier, AP_PRONE, BP_PRONE);
          }
        }
        pSoldier->fDontChargeAPsForStanceChange = FALSE;
        break;

        // Deduct points for stance change
        // sAPCost = GetAPsToChangeStance( pSoldier, gAnimControl[ usNewState ].ubEndHeight );
        // DeductPoints( pSoldier, sAPCost, 0 );
        // break;

      case START_AID:

        DeductPoints(pSoldier, AP_START_FIRST_AID, BP_START_FIRST_AID);
        break;

      case CUTTING_FENCE:
        DeductPoints(pSoldier, AP_USEWIRECUTTERS, BP_USEWIRECUTTERS);
        break;

      case PLANT_BOMB:

        DeductPoints(pSoldier, AP_DROP_BOMB, 0);
        break;

      case STEAL_ITEM:

        DeductPoints(pSoldier, AP_STEAL_ITEM, 0);
        break;

      case CROW_DIE:

        // Delete shadow of crow....
        if (pSoldier->pAniTile != NULL) {
          DeleteAniTile(pSoldier->pAniTile);
          pSoldier->pAniTile = NULL;
        }
        break;

      case CROW_FLY:

        // Ate: startup a shadow ( if gridno is set )
        HandleCrowShadowNewGridNo(pSoldier);
        break;

      case CROW_EAT:

        // ATE: Make sure height level is 0....
        SetSoldierHeight(pSoldier, (float)(0));
        HandleCrowShadowRemoveGridNo(pSoldier);
        break;

      case USE_REMOTE:

        DeductPoints(pSoldier, AP_USE_REMOTE, 0);
        break;

        // case PUNCH:

        // Deduct points for punching
        // sAPCost = MinAPsToAttack( pSoldier, pSoldier->sGridNo, FALSE );
        // DeductPoints( pSoldier, sAPCost, 0 );
        // break;

      case HOPFENCE:

        DeductPoints(pSoldier, AP_JUMPFENCE, BP_JUMPFENCE);
        break;

      // Deduct aps for falling down....
      case FALLBACK_HIT_STAND:
      case FALLFORWARD_FROMHIT_STAND:

        DeductPoints(pSoldier, AP_FALL_DOWN, BP_FALL_DOWN);
        break;

      case FALLFORWARD_FROMHIT_CROUCH:

        DeductPoints(pSoldier, (AP_FALL_DOWN / 2), (BP_FALL_DOWN / 2));
        break;

      case QUEEN_SWIPE:

        // ATE: set damage counter...
        pSoldier->uiPendingActionData1 = 0;
        break;

      case CLIMBDOWNROOF:

        // disable sight
        gTacticalStatus.uiFlags |= DISALLOW_SIGHT;

        DeductPoints(pSoldier, AP_CLIMBOFFROOF, BP_CLIMBOFFROOF);
        break;

      case CLIMBUPROOF:

        // disable sight
        gTacticalStatus.uiFlags |= DISALLOW_SIGHT;

        DeductPoints(pSoldier, AP_CLIMBROOF, BP_CLIMBROOF);
        break;

      case JUMP_OVER_BLOCKING_PERSON:

        // Set path....
        {
          uint16_t usNewGridNo;

          DeductPoints(pSoldier, AP_JUMP_OVER, BP_JUMP_OVER);

          usNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo, DirectionInc(pSoldier->bDirection));
          usNewGridNo = NewGridNo((uint16_t)usNewGridNo, DirectionInc(pSoldier->bDirection));

          pSoldier->usPathDataSize = 0;
          pSoldier->usPathIndex = 0;
          pSoldier->usPathingData[pSoldier->usPathDataSize] = pSoldier->bDirection;
          pSoldier->usPathDataSize++;
          pSoldier->usPathingData[pSoldier->usPathDataSize] = pSoldier->bDirection;
          pSoldier->usPathDataSize++;
          pSoldier->sFinalDestination = usNewGridNo;
          // Set direction
          EVENT_InternalSetSoldierDestination(pSoldier,
                                              pSoldier->usPathingData[pSoldier->usPathIndex], FALSE,
                                              JUMP_OVER_BLOCKING_PERSON);
        }
        break;

      case GENERIC_HIT_STAND:
      case GENERIC_HIT_CROUCH:
      case STANDING_BURST_HIT:
      case ADULTMONSTER_HIT:
      case ADULTMONSTER_DYING:
      case COW_HIT:
      case COW_DYING:
      case BLOODCAT_HIT:
      case BLOODCAT_DYING:
      case WATER_HIT:
      case WATER_DIE:
      case DEEP_WATER_HIT:
      case DEEP_WATER_DIE:
      case RIFLE_STAND_HIT:
      case LARVAE_HIT:
      case LARVAE_DIE:
      case QUEEN_HIT:
      case QUEEN_DIE:
      case INFANT_HIT:
      case INFANT_DIE:
      case CRIPPLE_HIT:
      case CRIPPLE_DIE:
      case CRIPPLE_DIE_FLYBACK:
      case ROBOTNW_HIT:
      case ROBOTNW_DIE:

        // Set getting hit flag to TRUE
        pSoldier->fGettingHit = TRUE;
        break;

      case CHARIOTS_OF_FIRE:
      case BODYEXPLODING:

        // Merc on fire!
        pSoldier->uiPendingActionData1 = PlaySoldierJA2Sample(
            GetSolID(pSoldier), (FIRE_ON_MERC), RATE_11025,
            SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 5, SoundDir(pSoldier->sGridNo), TRUE);
        break;
    }
  }

  // Remove old animation profile
  HandleAnimationProfile(pSoldier, pSoldier->usAnimState, TRUE);

  // From animation control, set surface
  if (SetSoldierAnimationSurface(pSoldier, usNewState) == FALSE) {
    return (FALSE);
  }

  // Set state
  pSoldier->usOldAniState = pSoldier->usAnimState;
  pSoldier->sOldAniCode = pSoldier->usAniCode;

  // Change state value!
  pSoldier->usAnimState = usNewState;

  pSoldier->sZLevelOverride = -1;

  if (!(pSoldier->uiStatusFlags & SOLDIER_LOCKPENDINGACTIONCOUNTER)) {
    // ATE Cancel ANY pending action...
    if (pSoldier->ubPendingActionAnimCount > 0 &&
        (gAnimControl[pSoldier->usOldAniState].uiFlags & ANIM_MOVING)) {
      // Do some special things for some actions
      switch (pSoldier->ubPendingAction) {
        case MERC_GIVEITEM:

          // Unset target as enaged
          MercPtrs[pSoldier->uiPendingActionData4]->uiStatusFlags &= (~SOLDIER_ENGAGEDINACTION);
          break;
      }
      pSoldier->ubPendingAction = NO_PENDING_ACTION;
    } else {
      // Increment this for almost all animations except some movement ones...
      // That's because this represents ANY animation other than the one we began when the pending
      // action was started ATE: Added to ignore this count if we are waiting for someone to move
      // out of our way...
      if (usNewState != START_SWAT && usNewState != END_SWAT &&
          !(gAnimControl[usNewState].uiFlags & ANIM_NOCHANGE_PENDINGCOUNT) &&
          !pSoldier->fDelayedMovement && !(pSoldier->uiStatusFlags & SOLDIER_ENGAGEDINACTION)) {
        pSoldier->ubPendingActionAnimCount++;
      }
    }
  }

  // Set new animation profile
  HandleAnimationProfile(pSoldier, usNewState, FALSE);

  // Reset some animation values
  pSoldier->fForceShade = FALSE;

  CheckForFreeupFromHit(pSoldier, uiOldAnimFlags, uiNewAnimFlags, pSoldier->usOldAniState,
                        usNewState);

  // CHECK IF WE ARE AT AN IDLE ACTION
#if 0
	if ( gAnimControl[ usNewState ].uiFlags & ANIM_IDLE )
	{
		pSoldier->bAction = ACTION_DONE;
	}
	else
	{
		pSoldier->bAction = ACTION_BUSY;
	}
#endif

  // Set current frame
  pSoldier->usAniCode = usStartingAniCode;

  // ATE; For some animations that could use some variations, do so....
  if (usNewState == CHARIOTS_OF_FIRE || usNewState == BODYEXPLODING) {
    pSoldier->usAniCode = (uint16_t)(Random(10));
  }

  // ATE: Default to first frame....
  // Will get changed ( probably ) by AdjustToNextAnimationFrame()
  ConvertAniCodeToAniFrame(pSoldier, (int16_t)(0));

  // Set delay speed
  SetSoldierAniSpeed(pSoldier);

  // Reset counters
  RESETTIMECOUNTER(pSoldier->UpdateCounter, pSoldier->sAniDelay);

  // Adjust to new animation frame ( the first one )
  AdjustToNextAnimationFrame(pSoldier);

  // Setup offset information for UI above guy
  SetSoldierLocatorOffsets(pSoldier);

  // If our own guy...
  if (pSoldier->bTeam == gbPlayerNum) {
    // Are we stationary?
    if (gAnimControl[usNewState].uiFlags & ANIM_STATIONARY) {
      // Position light....
      // SetCheckSoldierLightFlag( pSoldier );
    } else {
      // Hide light.....
      // DeleteSoldierLight( pSoldier );
    }
  }

  // If we are certain animations, reload palette
  if (usNewState == VEHICLE_DIE || usNewState == CHARIOTS_OF_FIRE || usNewState == BODYEXPLODING) {
    CreateSoldierPalettes(pSoldier);
  }

  // ATE: if the old animation was a movement, and new is not, play sound...
  // OK, play final footstep sound...
  if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
    if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY) &&
        (gAnimControl[pSoldier->usOldAniState].uiFlags & ANIM_MOVING)) {
      PlaySoldierFootstepSound(pSoldier);
    }
  }

  // Free up from stance change
  FreeUpNPCFromStanceChange(pSoldier);

  return (TRUE);
}

void InternalRemoveSoldierFromGridNo(struct SOLDIERTYPE *pSoldier, BOOLEAN fForce) {
  int8_t bDir;
  int32_t iGridNo;

  if ((pSoldier->sGridNo != NO_MAP_POS)) {
    if (pSoldier->bInSector || fForce) {
      // Remove from world ( old pos )
      RemoveMerc(pSoldier->sGridNo, pSoldier, FALSE);
      HandleAnimationProfile(pSoldier, pSoldier->usAnimState, TRUE);

      // Remove records of this guy being adjacent
      for (bDir = 0; bDir < NUM_WORLD_DIRECTIONS; bDir++) {
        iGridNo = pSoldier->sGridNo + DirIncrementer[bDir];
        if (iGridNo >= 0 && iGridNo < WORLD_MAX) {
          gpWorldLevelData[iGridNo].ubAdjacentSoldierCnt--;
        }
      }

      HandlePlacingRoofMarker(pSoldier, pSoldier->sGridNo, FALSE, FALSE);

      // Remove reseved movement value
      UnMarkMovementReserved(pSoldier);

      HandleCrowShadowRemoveGridNo(pSoldier);

      // Reset gridno...
      pSoldier->sGridNo = NO_MAP_POS;
    }
  }
}

void RemoveSoldierFromGridNo(struct SOLDIERTYPE *pSoldier) {
  InternalRemoveSoldierFromGridNo(pSoldier, FALSE);
}

void EVENT_InternalSetSoldierPosition(struct SOLDIERTYPE *pSoldier, float dNewXPos, float dNewYPos,
                                      BOOLEAN fUpdateDest, BOOLEAN fUpdateFinalDest,
                                      BOOLEAN fForceRemove) {
  int16_t sNewGridNo;

  // Not if we're dead!
  if ((pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
    return;
  }

  // Set new map index
  sNewGridNo = GETWORLDINDEXFROMWORLDCOORDS(dNewYPos, dNewXPos);

  if (fUpdateDest) {
    pSoldier->sDestination = sNewGridNo;
  }

  if (fUpdateFinalDest) {
    pSoldier->sFinalDestination = sNewGridNo;
  }

  // Copy old values
  pSoldier->dOldXPos = pSoldier->dXPos;
  pSoldier->dOldYPos = pSoldier->dYPos;

  // Set New pos
  pSoldier->dXPos = dNewXPos;
  pSoldier->dYPos = dNewYPos;

  pSoldier->sX = (int16_t)dNewXPos;
  pSoldier->sY = (int16_t)dNewYPos;

  HandleCrowShadowNewPosition(pSoldier);

  SetSoldierGridNo(pSoldier, sNewGridNo, fForceRemove);

  if (!(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
    if (gGameSettings.fOptions[TOPTION_MERC_ALWAYS_LIGHT_UP]) {
      SetCheckSoldierLightFlag(pSoldier);
    }
  }

  // ATE: Mirror calls if we are a vehicle ( for all our passengers )
  UpdateAllVehiclePassengersGridNo(pSoldier);
}

void EVENT_SetSoldierPosition(struct SOLDIERTYPE *pSoldier, float dNewXPos, float dNewYPos) {
  EVENT_InternalSetSoldierPosition(pSoldier, dNewXPos, dNewYPos, TRUE, TRUE, FALSE);
}

void EVENT_SetSoldierPositionForceDelete(struct SOLDIERTYPE *pSoldier, float dNewXPos,
                                         float dNewYPos) {
  EVENT_InternalSetSoldierPosition(pSoldier, dNewXPos, dNewYPos, TRUE, TRUE, TRUE);
}

void EVENT_SetSoldierPositionAndMaybeFinalDest(struct SOLDIERTYPE *pSoldier, float dNewXPos,
                                               float dNewYPos, BOOLEAN fUpdateFinalDest) {
  EVENT_InternalSetSoldierPosition(pSoldier, dNewXPos, dNewYPos, TRUE, fUpdateFinalDest, FALSE);
}

void EVENT_SetSoldierPositionAndMaybeFinalDestAndMaybeNotDestination(struct SOLDIERTYPE *pSoldier,
                                                                     float dNewXPos, float dNewYPos,
                                                                     BOOLEAN fUpdateDest,
                                                                     BOOLEAN fUpdateFinalDest) {
  EVENT_InternalSetSoldierPosition(pSoldier, dNewXPos, dNewYPos, fUpdateDest, fUpdateFinalDest,
                                   FALSE);
}

void InternalSetSoldierHeight(struct SOLDIERTYPE *pSoldier, float dNewHeight,
                              BOOLEAN fUpdateLevel) {
  int8_t bOldLevel = pSoldier->bLevel;

  pSoldier->dHeightAdjustment = dNewHeight;
  pSoldier->sHeightAdjustment = (int16_t)pSoldier->dHeightAdjustment;

  if (!fUpdateLevel) {
    return;
  }

  if (pSoldier->sHeightAdjustment > 0) {
    pSoldier->bLevel = SECOND_LEVEL;

    ApplyTranslucencyToWalls((int16_t)(pSoldier->dXPos / CELL_X_SIZE),
                             (int16_t)(pSoldier->dYPos / CELL_Y_SIZE));
    // LightHideTrees((int16_t)(pSoldier->dXPos/CELL_X_SIZE),
    // (int16_t)(pSoldier->dYPos/CELL_Y_SIZE)); ConcealAllWalls();

    // pSoldier->pLevelNode->ubShadeLevel=gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubShadeLevel;
    // pSoldier->pLevelNode->ubSumLights=gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubSumLights;
    // pSoldier->pLevelNode->ubMaxLights=gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubMaxLights;
    // pSoldier->pLevelNode->ubNaturalShadeLevel=gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubNaturalShadeLevel;
  } else {
    pSoldier->bLevel = FIRST_LEVEL;

    // pSoldier->pLevelNode->ubShadeLevel=gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;
    // pSoldier->pLevelNode->ubSumLights=gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubSumLights;
    // pSoldier->pLevelNode->ubMaxLights=gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubMaxLights;
    // pSoldier->pLevelNode->ubNaturalShadeLevel=gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubNaturalShadeLevel;
  }

  if (bOldLevel == 0 && pSoldier->bLevel == 0) {
  } else {
    // Show room at new level
    // HideRoom( pSoldier->sGridNo, pSoldier );
  }
}

void SetSoldierHeight(struct SOLDIERTYPE *pSoldier, float dNewHeight) {
  InternalSetSoldierHeight(pSoldier, dNewHeight, TRUE);
}

void SetSoldierGridNo(struct SOLDIERTYPE *pSoldier, int16_t sNewGridNo, BOOLEAN fForceRemove) {
  BOOLEAN fInWaterValue;
  int8_t bDir;
  int32_t cnt;
  struct SOLDIERTYPE *pEnemy;

  // int16_t	sX, sY, sWorldX, sZLevel;

  // Not if we're dead!
  if ((pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
    return;
  }

  if (sNewGridNo != pSoldier->sGridNo || pSoldier->pLevelNode == NULL) {
    // Check if we are moving AND this is our next dest gridno....
    if (gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_MOVING | ANIM_SPECIALMOVE)) {
      if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) {
        if (sNewGridNo != pSoldier->sDestination) {
          // THIS MUST be our new one......MAKE IT SO
          sNewGridNo = pSoldier->sDestination;
        }

        // Now check this baby....
        if (sNewGridNo == pSoldier->sGridNo) {
          return;
        }
      }
    }

    pSoldier->sOldGridNo = pSoldier->sGridNo;

    if (pSoldier->ubBodyType == QUEENMONSTER) {
      SetPositionSndGridNo(pSoldier->iPositionSndID, sNewGridNo);
    }

    if (!(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
      InternalRemoveSoldierFromGridNo(pSoldier, fForceRemove);
    }

    // CHECK IF OUR NEW GIRDNO IS VALID,IF NOT DONOT SET!
    if (!GridNoOnVisibleWorldTile(sNewGridNo)) {
      pSoldier->sGridNo = sNewGridNo;
      return;
    }

    // Alrighty, update UI for this guy, if he's the selected guy...
    if (gusSelectedSoldier == GetSolID(pSoldier)) {
      if (guiCurrentEvent == C_WAIT_FOR_CONFIRM) {
        // Update path!
        gfPlotNewMovement = TRUE;
      }
    }

    // Reset some flags for optimizations..
    pSoldier->sWalkToAttackGridNo = NOWHERE;

    // ATE: Make sure!
    // RemoveMerc( pSoldier->sGridNo, pSoldier, FALSE );

    pSoldier->sGridNo = sNewGridNo;

    // OK, check for special code to close door...
    if (pSoldier->bEndDoorOpenCode == 2) {
      pSoldier->bEndDoorOpenCode = 0;

      HandleDoorChangeFromGridNo(pSoldier, pSoldier->sEndDoorOpenCodeData, FALSE);
    }

    // OK, Update buddy's strategic insertion code....
    pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
    pSoldier->usStrategicInsertionData = sNewGridNo;

    // Remove this gridno as a reserved place!
    if (!(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
      UnMarkMovementReserved(pSoldier);
    }

    if (pSoldier->sInitialGridNo == 0) {
      pSoldier->sInitialGridNo = sNewGridNo;
      pSoldier->usPatrolGrid[0] = sNewGridNo;
    }

    // Add records of this guy being adjacent
    for (bDir = 0; bDir < NUM_WORLD_DIRECTIONS; bDir++) {
      gpWorldLevelData[pSoldier->sGridNo + DirIncrementer[bDir]].ubAdjacentSoldierCnt++;
    }

    if (!(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
      DropSmell(pSoldier);
    }

    // HANDLE ANY SPECIAL RENDERING SITUATIONS
    pSoldier->sZLevelOverride = -1;
    // If we are over a fence ( hopping ), make us higher!

    if (IsJumpableFencePresentAtGridno(sNewGridNo)) {
      // sX = MapX( sNewGridNo );
      // sY = MapY( sNewGridNo );
      // GetWorldXYAbsoluteScreenXY( sX, sY, &sWorldX, &sZLevel);
      // pSoldier->sZLevelOverride = (sZLevel*Z_SUBLAYERS)+ROOF_Z_LEVEL;
      pSoldier->sZLevelOverride = TOPMOST_Z_LEVEL;
    }

    // Add/ remove tree if we are near it
    // CheckForFullStructures( pSoldier );

    // Add merc at new pos
    if (!(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
      AddMercToHead(pSoldier->sGridNo, pSoldier, TRUE);

      // If we are in the middle of climbing the roof!
      if (pSoldier->usAnimState == CLIMBUPROOF) {
        if (pSoldier->iLight != (-1)) LightSpriteRoofStatus(pSoldier->iLight, TRUE);
      } else if (pSoldier->usAnimState == CLIMBDOWNROOF) {
        if (pSoldier->iLight != (-1)) LightSpriteRoofStatus(pSoldier->iLight, FALSE);
      }

      // JA2Gold:
      // if the player wants the merc to cast the fake light AND it is night
      if (pSoldier->bTeam != OUR_TEAM ||
          (gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT] && NightTime())) {
        if (pSoldier->bLevel > 0 && gpWorldLevelData[pSoldier->sGridNo].pRoofHead != NULL) {
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubShadeLevel =
              gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubShadeLevel;
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubSumLights =
              gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubSumLights;
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubMaxLights =
              gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubMaxLights;
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubNaturalShadeLevel =
              gpWorldLevelData[pSoldier->sGridNo].pRoofHead->ubNaturalShadeLevel;
        } else {
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubShadeLevel =
              gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubShadeLevel;
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubSumLights =
              gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubSumLights;
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubMaxLights =
              gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubMaxLights;
          gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubNaturalShadeLevel =
              gpWorldLevelData[pSoldier->sGridNo].pLandHead->ubNaturalShadeLevel;
        }
      }

      /// HandlePlacingRoofMarker( pSoldier, pSoldier->sGridNo, TRUE, FALSE );

      HandleAnimationProfile(pSoldier, pSoldier->usAnimState, FALSE);

      HandleCrowShadowNewGridNo(pSoldier);
    }

    pSoldier->bOldOverTerrainType = pSoldier->bOverTerrainType;
    pSoldier->bOverTerrainType = GetTerrainType(pSoldier->sGridNo);

    // OK, check that our animation is up to date!
    // Check our water value

    if (!(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
      fInWaterValue = MercInWater(pSoldier);

      // ATE: If ever in water MAKE SURE WE WALK AFTERWOODS!
      if (fInWaterValue) {
        pSoldier->usUIMovementMode = WALKING;
      }

      if (fInWaterValue != pSoldier->fPrevInWater) {
        // Update Animation data
        SetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

        // Update flag
        pSoldier->fPrevInWater = fInWaterValue;

        // Update sound...
        if (fInWaterValue) {
          PlaySoldierJA2Sample(pSoldier->ubID, ENTER_WATER_1, RATE_11025,
                               SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                               SoundDir(pSoldier->sGridNo), TRUE);
        } else {
          // ATE: Check if we are going from water to land - if so, resume
          // with regular movement mode...
          EVENT_InitNewSoldierAnim(pSoldier, pSoldier->usUIMovementMode, 0, FALSE);
        }
      }

      // OK, If we were not in deep water but we are now, handle deep animations!
      if (pSoldier->bOverTerrainType == DEEP_WATER && pSoldier->bOldOverTerrainType != DEEP_WATER) {
        // Based on our current animation, change!
        switch (pSoldier->usAnimState) {
          case WALKING:
          case RUNNING:

            // IN deep water, swim!

            // Make transition from low to deep
            EVENT_InitNewSoldierAnim(pSoldier, LOW_TO_DEEP_WATER, 0, FALSE);
            pSoldier->usPendingAnimation = DEEP_WATER_SWIM;

            PlayJA2Sample(ENTER_DEEP_WATER_1, RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo),
                          1, SoundDir(pSoldier->sGridNo));
        }
      }

      // Damage water if in deep water....
      if (pSoldier->bOverTerrainType == MED_WATER || pSoldier->bOverTerrainType == DEEP_WATER) {
        WaterDamage(pSoldier);
      }

      // OK, If we were in deep water but we are NOT now, handle mid animations!
      if (pSoldier->bOverTerrainType != DEEP_WATER && pSoldier->bOldOverTerrainType == DEEP_WATER) {
        // Make transition from low to deep
        EVENT_InitNewSoldierAnim(pSoldier, DEEP_TO_LOW_WATER, 0, FALSE);
        pSoldier->usPendingAnimation = pSoldier->usUIMovementMode;
      }
    }

    // are we now standing in tear gas without a decently working gas mask?
    if (GetSmokeEffectOnTile(sNewGridNo, pSoldier->bLevel)) {
      BOOLEAN fSetGassed = TRUE;

      // If we have a functioning gas mask...
      if (pSoldier->inv[HEAD1POS].usItem == GASMASK &&
          pSoldier->inv[HEAD1POS].bStatus[0] >= GASMASK_MIN_STATUS) {
        fSetGassed = FALSE;
      }
      if (pSoldier->inv[HEAD2POS].usItem == GASMASK &&
          pSoldier->inv[HEAD2POS].bStatus[0] >= GASMASK_MIN_STATUS) {
        fSetGassed = FALSE;
      }

      if (fSetGassed) {
        pSoldier->uiStatusFlags |= SOLDIER_GASSED;
      }
    }

    if (pSoldier->bTeam == gbPlayerNum && pSoldier->bStealthMode) {
      // Merc got to a new tile by "sneaking". Did we theoretically sneak
      // past an enemy?

      if (pSoldier->bOppCnt > 0)  // opponents in sight
      {
        // check each possible enemy
        for (cnt = 0; cnt < MAX_NUM_SOLDIERS; cnt++) {
          pEnemy = MercPtrs[cnt];
          // if this guy is here and alive enough to be looking for us
          if (pEnemy->bActive && pEnemy->bInSector && (pEnemy->bLife >= OKLIFE)) {
            // no points for sneaking by the neutrals & friendlies!!!
            if (!pEnemy->bNeutral && (pSoldier->bSide != pEnemy->bSide) &&
                (pEnemy->ubBodyType != COW && pEnemy->ubBodyType != CROW)) {
              // if we SEE this particular oppponent, and he DOESN'T see us... and he COULD see
              // us...
              if ((pSoldier->bOppList[cnt] == SEEN_CURRENTLY) &&
                  pEnemy->bOppList[pSoldier->ubID] != SEEN_CURRENTLY &&
                  PythSpacesAway(pSoldier->sGridNo, pEnemy->sGridNo) <
                      DistanceVisible(pEnemy, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT,
                                      pSoldier->sGridNo, pSoldier->bLevel)) {
                // AGILITY (5):  Soldier snuck 1 square past unaware enemy
                StatChange(pSoldier, AGILAMT, 5, FALSE);
                // Keep looping, we'll give'em 1 point for EACH such enemy!
              }
            }
          }
        }
      }
    }

    // Adjust speed based on terrain, etc
    SetSoldierAniSpeed(pSoldier);
  }
}

void EVENT_FireSoldierWeapon(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  int16_t sTargetXPos, sTargetYPos;
  BOOLEAN fDoFireRightAway = FALSE;

  // CANNOT BE SAME GRIDNO!
  if (pSoldier->sGridNo == sTargetGridNo) {
    return;
  }

  // Increment the number of people busy doing stuff because of an attack
  // if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
  //{
  gTacticalStatus.ubAttackBusyCount++;
  DebugMsg(
      TOPIC_JA2, DBG_LEVEL_3,
      String("!!!!!!! Starting attack, attack count now %d", gTacticalStatus.ubAttackBusyCount));
  //}

  // Set soldier's target gridno
  // This assignment was redundent because it's already set in
  // the actual event call
  pSoldier->sTargetGridNo = sTargetGridNo;
  // pSoldier->sLastTarget = sTargetGridNo;
  pSoldier->ubTargetID = WhoIsThere2(sTargetGridNo, pSoldier->bTargetLevel);

  if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass & IC_GUN) {
    if (pSoldier->bDoBurst) {
      // Set the TOTAL number of bullets to be fired
      // Can't shoot more bullets than we have in our magazine!
      pSoldier->bBulletsLeft =
          min(Weapon[pSoldier->inv[pSoldier->ubAttackingHand].usItem].ubShotsPerBurst,
              pSoldier->inv[pSoldier->ubAttackingHand].ubGunShotsLeft);
    } else if (IsValidSecondHandShot(pSoldier)) {
      // two-pistol attack - two bullets!
      pSoldier->bBulletsLeft = 2;
    } else {
      pSoldier->bBulletsLeft = 1;
    }
    if (pSoldier->inv[pSoldier->ubAttackingHand].ubGunAmmoType == AMMO_BUCKSHOT) {
      pSoldier->bBulletsLeft *= NUM_BUCKSHOT_PELLETS;
    }
  }
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("!!!!!!! Starting attack, bullets left %d", pSoldier->bBulletsLeft));

  // Convert our grid-not into an XY
  ConvertGridNoToXY(sTargetGridNo, &sTargetXPos, &sTargetYPos);

  // Change to fire animation
  // Ready weapon
  SoldierReadyWeapon(pSoldier, sTargetXPos, sTargetYPos, FALSE);

  // IF WE ARE AN NPC, SLIDE VIEW TO SHOW WHO IS SHOOTING
  {
    // if ( pSoldier->fDoSpread )
    //{
    // If we are spreading burst, goto right away!
    // EVENT_InitNewSoldierAnim( pSoldier, SelectFireAnimation( pSoldier, gAnimControl[
    // pSoldier->usAnimState ].ubEndHeight ), 0, FALSE );

    //}

    // else
    {
      if (pSoldier->uiStatusFlags & SOLDIER_MONSTER) {
        // Force our direction!
        EVENT_SetSoldierDirection(pSoldier, pSoldier->bDesiredDirection);
        EVENT_InitNewSoldierAnim(
            pSoldier,
            SelectFireAnimation(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight), 0,
            FALSE);
      } else {
        // IF WE ARE IN REAl-TIME, FIRE IMMEDIATELY!
        if (((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT))) {
          // fDoFireRightAway = TRUE;
        }

        // Check if our weapon has no intermediate anim...
        switch (pSoldier->inv[HANDPOS].usItem) {
          case ROCKET_LAUNCHER:
          case MORTAR:
          case GLAUNCHER:

            fDoFireRightAway = TRUE;
            break;
        }

        if (fDoFireRightAway) {
          // Set to true so we don't get toasted twice for APs..
          pSoldier->fDontUnsetLastTargetFromTurn = TRUE;

          // Make sure we don't try and do fancy prone turning.....
          pSoldier->fTurningFromPronePosition = FALSE;

          // Force our direction!
          EVENT_SetSoldierDirection(pSoldier, pSoldier->bDesiredDirection);

          EVENT_InitNewSoldierAnim(
              pSoldier,
              SelectFireAnimation(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight), 0,
              FALSE);
        } else {
          // Set flag indicating we are about to shoot once destination direction is hit
          pSoldier->fTurningToShoot = TRUE;

          if (pSoldier->bTeam != gbPlayerNum && pSoldier->bVisible != -1) {
            LocateSoldier(pSoldier->ubID, DONTSETLOCATOR);
          }
        }
      }
    }
  }
}

// gAnimControl[ pSoldier->usAnimState ].ubEndHeight
//					ChangeSoldierState( pSoldier, SHOOT_RIFLE_STAND, 0 , FALSE
//);

uint16_t SelectFireAnimation(struct SOLDIERTYPE *pSoldier, uint8_t ubHeight) {
  int16_t sDist;
  float dTargetX;
  float dTargetY;
  float dTargetZ;
  BOOLEAN fDoLowShot = FALSE;

  // Do different things if we are a monster
  if (pSoldier->uiStatusFlags & SOLDIER_MONSTER) {
    switch (pSoldier->ubBodyType) {
      case ADULTFEMALEMONSTER:
      case AM_MONSTER:
      case YAF_MONSTER:
      case YAM_MONSTER:

        return (MONSTER_SPIT_ATTACK);
        break;

      case LARVAE_MONSTER:

        break;

      case INFANT_MONSTER:

        return (INFANT_ATTACK);
        break;

      case QUEENMONSTER:

        return (QUEEN_SPIT);
        break;
    }
    return (TRUE);
  }

  if (pSoldier->ubBodyType == ROBOTNOWEAPON) {
    if (pSoldier->bDoBurst > 0) {
      return (ROBOT_BURST_SHOOT);
    } else {
      return (ROBOT_SHOOT);
    }
  }

  // Check for rocket laucncher....
  if (pSoldier->inv[HANDPOS].usItem == ROCKET_LAUNCHER) {
    return (SHOOT_ROCKET);
  }

  // Check for rocket laucncher....
  if (pSoldier->inv[HANDPOS].usItem == MORTAR) {
    return (SHOOT_MORTAR);
  }

  // Check for tank cannon
  if (pSoldier->inv[HANDPOS].usItem == TANK_CANNON) {
    return (TANK_SHOOT);
  }

  if (pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE) {
    return (TANK_BURST);
  }

  // Determine which animation to do...depending on stance and gun in hand...
  switch (ubHeight) {
    case ANIM_STAND:

      // CHECK 2ND HAND!
      if (IsValidSecondHandShot(pSoldier)) {
        // Increment the number of people busy doing stuff because of an attack
        // gTacticalStatus.ubAttackBusyCount++;
        // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Starting attack with 2 guns, attack
        // count now %d", gTacticalStatus.ubAttackBusyCount) );

        return (SHOOT_DUAL_STAND);
      } else {
        // OK, while standing check distance away from target, and shoot low if we should!
        sDist = PythSpacesAway(pSoldier->sGridNo, pSoldier->sTargetGridNo);

        // ATE: OK, SEE WERE WE ARE TARGETING....
        GetTargetWorldPositions(pSoldier, pSoldier->sTargetGridNo, &dTargetX, &dTargetY, &dTargetZ);

        // CalculateSoldierZPos( pSoldier, FIRING_POS, &dFirerZ );

        if (sDist <= 2 && dTargetZ <= 100) {
          fDoLowShot = TRUE;
        }

        // ATE: Made distence away long for psitols such that they never use this....
        // if ( !(Item[ usItem ].fFlags & ITEM_TWO_HANDED) )
        //{
        //	fDoLowShot = FALSE;
        //}

        // Don't do any low shots if in water
        if (MercInWater(pSoldier)) {
          fDoLowShot = FALSE;
        }

        if (pSoldier->bDoBurst > 0) {
          if (fDoLowShot) {
            return (FIRE_BURST_LOW_STAND);
          } else {
            return (STANDING_BURST);
          }
        } else {
          if (fDoLowShot) {
            return (FIRE_LOW_STAND);
          } else {
            return (SHOOT_RIFLE_STAND);
          }
        }
      }
      break;

    case ANIM_PRONE:

      if (pSoldier->bDoBurst > 0) {
        //				pSoldier->fBurstCompleted = FALSE;
        return (PRONE_BURST);
      } else {
        if (IsValidSecondHandShot(pSoldier)) {
          return (SHOOT_DUAL_PRONE);
        } else {
          return (SHOOT_RIFLE_PRONE);
        }
      }
      break;

    case ANIM_CROUCH:

      if (IsValidSecondHandShot(pSoldier)) {
        // Increment the number of people busy doing stuff because of an attack
        // gTacticalStatus.ubAttackBusyCount++;
        // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("!!!!!!! Starting attack with 2 guns, attack
        // count now %d", gTacticalStatus.ubAttackBusyCount) );

        return (SHOOT_DUAL_CROUCH);
      } else {
        if (pSoldier->bDoBurst > 0) {
          //				pSoldier->fBurstCompleted = FALSE;
          return (CROUCHED_BURST);
        } else {
          return (SHOOT_RIFLE_CROUCH);
        }
      }
      break;

    default:
      AssertMsg(FALSE, String("SelectFireAnimation: ERROR - Invalid height %d", ubHeight));
      break;
  }

  // If here, an internal error has occured!
  Assert(FALSE);
  return (0);
}

uint16_t GetMoveStateBasedOnStance(struct SOLDIERTYPE *pSoldier, uint8_t ubStanceHeight) {
  // Determine which animation to do...depending on stance and gun in hand...
  switch (ubStanceHeight) {
    case ANIM_STAND:
      if (pSoldier->fUIMovementFast && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
        return (RUNNING);
      } else {
        return (WALKING);
      }
      break;

    case ANIM_PRONE:
      if (pSoldier->fUIMovementFast) {
        return (CRAWLING);
      } else {
        return (CRAWLING);
      }
      break;

    case ANIM_CROUCH:
      if (pSoldier->fUIMovementFast) {
        return (SWATTING);
      } else {
        return (SWATTING);
      }
      break;

    default:
      AssertMsg(FALSE,
                String("GetMoveStateBasedOnStance: ERROR - Invalid height %d", ubStanceHeight));
      break;
  }

  // If here, an internal error has occured!
  Assert(FALSE);
  return (0);
}

void SelectFallAnimation(struct SOLDIERTYPE *pSoldier) {
  // Determine which animation to do...depending on stance and gun in hand...
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:
      EVENT_InitNewSoldierAnim(pSoldier, FLYBACK_HIT, 0, FALSE);
      break;

    case ANIM_PRONE:
      EVENT_InitNewSoldierAnim(pSoldier, FLYBACK_HIT, 0, FALSE);
      break;
  }
}

BOOLEAN SoldierReadyWeapon(struct SOLDIERTYPE *pSoldier, int16_t sTargetXPos, int16_t sTargetYPos,
                           BOOLEAN fEndReady) {
  int16_t sFacingDir;

  sFacingDir = GetDirectionFromXY(sTargetXPos, sTargetYPos, pSoldier);

  return (InternalSoldierReadyWeapon(pSoldier, (int8_t)sFacingDir, fEndReady));
}

BOOLEAN InternalSoldierReadyWeapon(struct SOLDIERTYPE *pSoldier, uint8_t sFacingDir,
                                   BOOLEAN fEndReady) {
  uint16_t usAnimState;
  BOOLEAN fReturnVal = FALSE;

  // Handle monsters differently
  if (pSoldier->uiStatusFlags & SOLDIER_MONSTER) {
    if (!fEndReady) {
      EVENT_SetSoldierDesiredDirection(pSoldier, sFacingDir);
    }
    return (FALSE);
  }

  usAnimState = PickSoldierReadyAnimation(pSoldier, fEndReady);

  if (usAnimState != INVALID_ANIMATION) {
    EVENT_InitNewSoldierAnim(pSoldier, usAnimState, 0, FALSE);
    fReturnVal = TRUE;
  }

  if (!fEndReady) {
    // Ready direction for new facing direction
    if (usAnimState == INVALID_ANIMATION) {
      usAnimState = pSoldier->usAnimState;
    }

    EVENT_InternalSetSoldierDesiredDirection(pSoldier, sFacingDir, FALSE, usAnimState);

    // Check if facing dir is different from ours and change direction if so!
    // if ( sFacingDir != pSoldier->bDirection )
    //{
    //	DeductPoints( pSoldier, AP_CHANGE_FACING, 0 );
    //}//
  }

  return (fReturnVal);
}

uint16_t PickSoldierReadyAnimation(struct SOLDIERTYPE *pSoldier, BOOLEAN fEndReady) {
  // Invalid animation if nothing in our hands
  if (pSoldier->inv[HANDPOS].usItem == NOTHING) {
    return (INVALID_ANIMATION);
  }

  if (pSoldier->bOverTerrainType == DEEP_WATER) {
    return (INVALID_ANIMATION);
  }

  if (pSoldier->ubBodyType == ROBOTNOWEAPON) {
    return (INVALID_ANIMATION);
  }

  // Check if we have a gun.....
  if (Item[pSoldier->inv[HANDPOS].usItem].usItemClass != IC_GUN &&
      pSoldier->inv[HANDPOS].usItem != GLAUNCHER) {
    return (INVALID_ANIMATION);
  }

  if (pSoldier->inv[HANDPOS].usItem == ROCKET_LAUNCHER) {
    return (INVALID_ANIMATION);
  }

  if (pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE) {
    return (INVALID_ANIMATION);
  }

  if (fEndReady) {
    // IF our gun is already drawn, do not change animation, just direction
    if (gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_FIREREADY | ANIM_FIRE)) {
      switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
        case ANIM_STAND:

          // CHECK 2ND HAND!
          if (IsValidSecondHandShot(pSoldier)) {
            return (END_DUAL_STAND);
          } else {
            return (END_RIFLE_STAND);
          }
          break;

        case ANIM_PRONE:

          if (IsValidSecondHandShot(pSoldier)) {
            return (END_DUAL_PRONE);
          } else {
            return (END_RIFLE_PRONE);
          }
          break;

        case ANIM_CROUCH:

          // CHECK 2ND HAND!
          if (IsValidSecondHandShot(pSoldier)) {
            return (END_DUAL_CROUCH);
          } else {
            return (END_RIFLE_CROUCH);
          }
          break;
      }
    }
  } else {
    // IF our gun is already drawn, do not change animation, just direction
    if (!(gAnimControl[pSoldier->usAnimState].uiFlags & (ANIM_FIREREADY | ANIM_FIRE))) {
      {
        switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
          case ANIM_STAND:

            // CHECK 2ND HAND!
            if (IsValidSecondHandShot(pSoldier)) {
              return (READY_DUAL_STAND);
            } else {
              return (READY_RIFLE_STAND);
            }
            break;

          case ANIM_PRONE:
            // Go into crouch, turn, then go into prone again
            // ChangeSoldierStance( pSoldier, ANIM_CROUCH );
            // pSoldier->ubDesiredHeight = ANIM_PRONE;
            // ChangeSoldierState( pSoldier, PRONE_UP );
            if (IsValidSecondHandShot(pSoldier)) {
              return (READY_DUAL_PRONE);
            } else {
              return (READY_RIFLE_PRONE);
            }
            break;

          case ANIM_CROUCH:

            // CHECK 2ND HAND!
            if (IsValidSecondHandShot(pSoldier)) {
              return (READY_DUAL_CROUCH);
            } else {
              return (READY_RIFLE_CROUCH);
            }
            break;
        }
      }
    }
  }

  return (INVALID_ANIMATION);
}

extern struct SOLDIERTYPE *FreeUpAttackerGivenTarget(uint8_t ubID, uint8_t ubTargetID);
extern struct SOLDIERTYPE *ReduceAttackBusyGivenTarget(uint8_t ubID, uint8_t ubTargetID);

// ATE: THIS FUNCTION IS USED FOR ALL SOLDIER TAKE DAMAGE FUNCTIONS!
void EVENT_SoldierGotHit(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                         int16_t sBreathLoss, uint16_t bDirection, uint16_t sRange,
                         uint8_t ubAttackerID, uint8_t ubSpecial, uint8_t ubHitLocation,
                         int16_t sSubsequent, int16_t sLocationGrid) {
  uint8_t ubCombinedLoss, ubVolume, ubReason;
  struct SOLDIERTYPE *pNewSoldier;

  ubReason = 0;

  // ATE: If we have gotten hit, but are still in our attack animation, reduce count!
  switch (pSoldier->usAnimState) {
    case SHOOT_ROCKET:
    case SHOOT_MORTAR:
    case THROW_ITEM:
    case LOB_ITEM:

      DebugMsg(
          TOPIC_JA2, DBG_LEVEL_3,
          String("@@@@@@@ Freeing up attacker - ATTACK ANIMATION %s ENDED BY HIT ANIMATION, Now %d",
                 gAnimControl[pSoldier->usAnimState].zAnimStr, gTacticalStatus.ubAttackBusyCount));
      ReduceAttackBusyCount(pSoldier->ubID, FALSE);
      break;
  }

  // DO STUFF COMMON FOR ALL TYPES
  if (ubAttackerID != NOBODY) {
    MercPtrs[ubAttackerID]->bLastAttackHit = TRUE;
  }

  // Set attacker's ID
  pSoldier->ubAttackerID = ubAttackerID;

  if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    // Increment  being attacked count
    pSoldier->bBeingAttackedCount++;
  }

  // if defender is a vehicle, there will be no hit animation played!
  if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    // Increment the number of people busy doing stuff because of an attack (busy doing hit anim!)
    gTacticalStatus.ubAttackBusyCount++;
    DebugMsg(
        TOPIC_JA2, DBG_LEVEL_3,
        String("!!!!!!! Person got hit, attack count now %d", gTacticalStatus.ubAttackBusyCount));
  }

  // ATE; Save hit location info...( for later anim determination stuff )
  pSoldier->ubHitLocation = ubHitLocation;

  // handle morale for heavy damage attacks
  if (sDamage > 25) {
    if (pSoldier->ubAttackerID != NOBODY &&
        MercPtrs[pSoldier->ubAttackerID]->bTeam == gbPlayerNum) {
      HandleMoraleEvent(MercPtrs[pSoldier->ubAttackerID], MORALE_DID_LOTS_OF_DAMAGE,
                        (uint8_t)MercPtrs[pSoldier->ubAttackerID]->sSectorX,
                        (uint8_t)MercPtrs[pSoldier->ubAttackerID]->sSectorY,
                        MercPtrs[pSoldier->ubAttackerID]->bSectorZ);
    }
    if (pSoldier->bTeam == gbPlayerNum) {
      HandleMoraleEvent(pSoldier, MORALE_TOOK_LOTS_OF_DAMAGE, GetSolSectorX(pSoldier),
                        GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));
    }
  }

  // SWITCH IN TYPE OF WEAPON
  if (ubSpecial == FIRE_WEAPON_TOSSED_OBJECT_SPECIAL) {
    ubReason = TAKE_DAMAGE_OBJECT;
  } else if (Item[usWeaponIndex].usItemClass & IC_TENTACLES) {
    ubReason = TAKE_DAMAGE_TENTACLES;
  } else if (Item[usWeaponIndex].usItemClass & (IC_GUN | IC_THROWING_KNIFE)) {
    if (ubSpecial == FIRE_WEAPON_SLEEP_DART_SPECIAL) {
      uint32_t uiChance;

      // put the drug in!
      pSoldier->bSleepDrugCounter = 10;

      uiChance = SleepDartSuccumbChance(pSoldier);

      if (PreRandom(100) < uiChance) {
        // succumb to the drug!
        sBreathLoss = (int16_t)(pSoldier->bBreathMax * 100);
      }

    } else if (ubSpecial == FIRE_WEAPON_BLINDED_BY_SPIT_SPECIAL) {
      // blinded!!
      if ((pSoldier->bBlindedCounter == 0)) {
        // say quote
        if (pSoldier->uiStatusFlags & SOLDIER_PC) {
          TacticalCharacterDialogue(pSoldier, QUOTE_BLINDED);
        }
        DecayIndividualOpplist(pSoldier);
      }
      // will always increase counter by at least 1
      pSoldier->bBlindedCounter += (sDamage / 8) + 1;

      // Dirty panel
      fInterfacePanelDirty = DIRTYLEVEL2;
    }
    sBreathLoss += BP_GET_HIT;
    ubReason = TAKE_DAMAGE_GUNFIRE;
  } else if (Item[usWeaponIndex].usItemClass & IC_BLADE) {
    sBreathLoss = BP_GET_HIT;
    ubReason = TAKE_DAMAGE_BLADE;
  } else if (Item[usWeaponIndex].usItemClass & IC_PUNCH) {
    // damage from hand-to-hand is 1/4 normal, 3/4 breath.. the sDamage value
    // is actually how much breath we'll take away
    sBreathLoss = sDamage * 100;
    sDamage = sDamage / PUNCH_REAL_DAMAGE_PORTION;
    if (AreInMeanwhile() && gCurrentMeanwhileDef.ubMeanwhileID == INTERROGATION) {
      sBreathLoss = 0;
      sDamage /= 2;
    }
    ubReason = TAKE_DAMAGE_HANDTOHAND;
  } else if (Item[usWeaponIndex].usItemClass & IC_EXPLOSV) {
    if (usWeaponIndex == STRUCTURE_EXPLOSION) {
      ubReason = TAKE_DAMAGE_STRUCTURE_EXPLOSION;
    } else {
      ubReason = TAKE_DAMAGE_EXPLOSION;
    }
  } else {
    DebugMsg(
        TOPIC_JA2, DBG_LEVEL_3,
        String("Soldier Control: Weapon class not handled in SoldierGotHit( ) %d", usWeaponIndex));
  }

  // CJC: moved to after SoldierTakeDamage so that any quotes from the defender
  // will not be said if they are knocked out or killed
  if (ubReason != TAKE_DAMAGE_TENTACLES && ubReason != TAKE_DAMAGE_OBJECT) {
    // OK, OK: THis is hairy, however, it's ness. because the normal freeup call uses the
    // attckers intended target, and here we want to use thier actual target....

    // ATE: If it's from GUNFIRE damage, keep in mind bullets...
    if (Item[usWeaponIndex].usItemClass & IC_GUN) {
      pNewSoldier = FreeUpAttackerGivenTarget(pSoldier->ubAttackerID, GetSolID(pSoldier));
    } else {
      pNewSoldier = ReduceAttackBusyGivenTarget(pSoldier->ubAttackerID, GetSolID(pSoldier));
    }

    if (pNewSoldier != NULL) {
      pSoldier = pNewSoldier;
    }
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("!!!!!!! Tried to free up attacker, attack count now %d",
                    gTacticalStatus.ubAttackBusyCount));
  }

  // OK, If we are a vehicle.... damage vehicle...( people inside... )
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    SoldierTakeDamage(pSoldier, ANIM_CROUCH, sDamage, sBreathLoss, ubReason, pSoldier->ubAttackerID,
                      NOWHERE, FALSE, TRUE);
    return;
  }

  // DEDUCT LIFE
  ubCombinedLoss = SoldierTakeDamage(pSoldier, ANIM_CROUCH, sDamage, sBreathLoss, ubReason,
                                     pSoldier->ubAttackerID, NOWHERE, FALSE, TRUE);

  // ATE: OK, Let's check our ASSIGNMENT state,
  // If anything other than on a squad or guard, make them guard....
  if (pSoldier->bTeam == gbPlayerNum) {
    if (pSoldier->bAssignment >= ON_DUTY && pSoldier->bAssignment != ASSIGNMENT_POW) {
      if (pSoldier->fMercAsleep) {
        pSoldier->fMercAsleep = FALSE;
        pSoldier->fForcedToStayAwake = FALSE;

        // refresh map screen
        fCharacterInfoPanelDirty = TRUE;
        fTeamPanelDirty = TRUE;
      }

      AddCharacterToAnySquad(pSoldier);
    }
  }

  // SCREAM!!!!
  ubVolume = CalcScreamVolume(pSoldier, ubCombinedLoss);

  // IF WE ARE AT A HIT_STOP ANIMATION
  // DO APPROPRIATE HITWHILE DOWN ANIMATION
  if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_HITSTOP) ||
      pSoldier->usAnimState != JFK_HITDEATH_STOP) {
    MakeNoise(pSoldier->ubID, pSoldier->sGridNo, pSoldier->bLevel, pSoldier->bOverTerrainType,
              ubVolume, NOISE_SCREAM);
  }

  // IAN ADDED THIS SAT JUNE 14th : HAVE TO SHOW VICTIM!
  if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) &&
      pSoldier->bVisible != -1 && pSoldier->bTeam == gbPlayerNum)
    LocateSoldier(pSoldier->ubID, DONTSETLOCATOR);

  if (Item[usWeaponIndex].usItemClass & IC_BLADE) {
    PlayJA2Sample((uint32_t)(KNIFE_IMPACT), RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo),
                  1, SoundDir(pSoldier->sGridNo));
  } else {
    PlayJA2Sample((uint32_t)(BULLET_IMPACT_1 + Random(3)), RATE_11025,
                  SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo));
  }

  // PLAY RANDOM GETTING HIT SOUND
  // ONLY IF WE ARE CONSCIOUS!
  if (pSoldier->bLife >= CONSCIOUSNESS) {
    if (pSoldier->ubBodyType == CROW) {
      // Exploding crow...
      PlayJA2Sample(CROW_EXPLODE_1, RATE_11025, SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
                    SoundDir(pSoldier->sGridNo));
    } else {
      // ATE: This is to disallow large amounts of smaples being played which is load!
      if (pSoldier->fGettingHit && pSoldier->usAniCode != STANDING_BURST_HIT) {
      } else {
        DoMercBattleSound(pSoldier, (int8_t)(BATTLE_SOUND_HIT1 + Random(2)));
      }
    }
  }

  // CHECK FOR DOING HIT WHILE DOWN
  if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_HITSTOP)) {
    switch (pSoldier->usAnimState) {
      case FLYBACKHIT_STOP:
        ChangeSoldierState(pSoldier, FALLBACK_DEATHTWICH, 0, FALSE);
        break;

      case STAND_FALLFORWARD_STOP:
        ChangeSoldierState(pSoldier, GENERIC_HIT_DEATHTWITCHNB, 0, FALSE);
        break;

      case JFK_HITDEATH_STOP:
        ChangeSoldierState(pSoldier, JFK_HITDEATH_TWITCHB, 0, FALSE);
        break;

      case FALLBACKHIT_STOP:
        ChangeSoldierState(pSoldier, FALLBACK_HIT_DEATHTWITCHNB, 0, FALSE);
        break;

      case PRONE_LAYFROMHIT_STOP:
        ChangeSoldierState(pSoldier, PRONE_HIT_DEATHTWITCHNB, 0, FALSE);
        break;

      case PRONE_HITDEATH_STOP:
        ChangeSoldierState(pSoldier, PRONE_HIT_DEATHTWITCHB, 0, FALSE);
        break;

      case FALLFORWARD_HITDEATH_STOP:
        ChangeSoldierState(pSoldier, GENERIC_HIT_DEATHTWITCHB, 0, FALSE);
        break;

      case FALLBACK_HITDEATH_STOP:
        ChangeSoldierState(pSoldier, FALLBACK_HIT_DEATHTWITCHB, 0, FALSE);
        break;

      case FALLOFF_DEATH_STOP:
        ChangeSoldierState(pSoldier, FALLOFF_TWITCHB, 0, FALSE);
        break;

      case FALLOFF_STOP:
        ChangeSoldierState(pSoldier, FALLOFF_TWITCHNB, 0, FALSE);
        break;

      case FALLOFF_FORWARD_DEATH_STOP:
        ChangeSoldierState(pSoldier, FALLOFF_FORWARD_TWITCHB, 0, FALSE);
        break;

      case FALLOFF_FORWARD_STOP:
        ChangeSoldierState(pSoldier, FALLOFF_FORWARD_TWITCHNB, 0, FALSE);
        break;

      default:
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("Soldier Control: Death state %d has no death hit", pSoldier->usAnimState));
    }
    return;
  }

  // Set goback to aim after hit flag!
  // Only if we were aiming!
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIREREADY) {
    pSoldier->fGoBackToAimAfterHit = TRUE;
  }

  // IF COWERING, PLAY SPECIFIC GENERIC HIT STAND...
  if (pSoldier->uiStatusFlags & SOLDIER_COWERING) {
    if (pSoldier->bLife == 0 || IS_MERC_BODY_TYPE(pSoldier)) {
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_STAND, 0, FALSE);
    } else {
      EVENT_InitNewSoldierAnim(pSoldier, CIV_COWER_HIT, 0, FALSE);
    }
    return;
  }

  // Change based on body type
  switch (pSoldier->ubBodyType) {
    case COW:
      EVENT_InitNewSoldierAnim(pSoldier, COW_HIT, 0, FALSE);
      return;
      break;

    case BLOODCAT:
      EVENT_InitNewSoldierAnim(pSoldier, BLOODCAT_HIT, 0, FALSE);
      return;
      break;

    case ADULTFEMALEMONSTER:
    case AM_MONSTER:
    case YAF_MONSTER:
    case YAM_MONSTER:

      EVENT_InitNewSoldierAnim(pSoldier, ADULTMONSTER_HIT, 0, FALSE);
      return;
      break;

    case LARVAE_MONSTER:
      EVENT_InitNewSoldierAnim(pSoldier, LARVAE_HIT, 0, FALSE);
      return;
      break;

    case QUEENMONSTER:
      EVENT_InitNewSoldierAnim(pSoldier, QUEEN_HIT, 0, FALSE);
      return;
      break;

    case CRIPPLECIV:

    {
      // OK, do some code here to allow the fact that poor buddy can be thrown back if it's a big
      // enough hit...
      EVENT_InitNewSoldierAnim(pSoldier, CRIPPLE_HIT, 0, FALSE);

      // pSoldier->bLife = 0;
      // EVENT_InitNewSoldierAnim( pSoldier, CRIPPLE_DIE_FLYBACK, 0 , FALSE );
    }
      return;
      break;

    case ROBOTNOWEAPON:
      EVENT_InitNewSoldierAnim(pSoldier, ROBOTNW_HIT, 0, FALSE);
      return;
      break;

    case INFANT_MONSTER:
      EVENT_InitNewSoldierAnim(pSoldier, INFANT_HIT, 0, FALSE);
      return;

    case CROW:

      EVENT_InitNewSoldierAnim(pSoldier, CROW_DIE, 0, FALSE);
      return;

    // case FATCIV:
    case MANCIV:
    case MINICIV:
    case DRESSCIV:
    case HATKIDCIV:
    case KIDCIV:

      // OK, if life is 0 and not set as dead ( this is a death hit... )
      if (!(pSoldier->uiStatusFlags & SOLDIER_DEAD) && pSoldier->bLife == 0) {
        // Randomize death!
        if (Random(2)) {
          EVENT_InitNewSoldierAnim(pSoldier, CIV_DIE2, 0, FALSE);
          return;
        }
      }

      // IF here, go generic hit ALWAYS.....
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_STAND, 0, FALSE);
      return;
      break;
  }

  // If here, we are a merc, check if we are in water
  if (pSoldier->bOverTerrainType == LOW_WATER) {
    EVENT_InitNewSoldierAnim(pSoldier, WATER_HIT, 0, FALSE);
    return;
  }
  if (pSoldier->bOverTerrainType == DEEP_WATER) {
    EVENT_InitNewSoldierAnim(pSoldier, DEEP_WATER_HIT, 0, FALSE);
    return;
  }

  // SWITCH IN TYPE OF WEAPON
  if (Item[usWeaponIndex].usItemClass & (IC_GUN | IC_THROWING_KNIFE)) {
    SoldierGotHitGunFire(pSoldier, usWeaponIndex, sDamage, bDirection, sRange, ubAttackerID,
                         ubSpecial, ubHitLocation);
  }
  if (Item[usWeaponIndex].usItemClass & IC_BLADE) {
    SoldierGotHitBlade(pSoldier, usWeaponIndex, sDamage, bDirection, sRange, ubAttackerID,
                       ubSpecial, ubHitLocation);
  }
  if (Item[usWeaponIndex].usItemClass & IC_EXPLOSV ||
      Item[usWeaponIndex].usItemClass & IC_TENTACLES) {
    SoldierGotHitExplosion(pSoldier, usWeaponIndex, sDamage, bDirection, sRange, ubAttackerID,
                           ubSpecial, ubHitLocation);
  }
  if (Item[usWeaponIndex].usItemClass & IC_PUNCH) {
    SoldierGotHitPunch(pSoldier, usWeaponIndex, sDamage, bDirection, sRange, ubAttackerID,
                       ubSpecial, ubHitLocation);
  }
}

uint8_t CalcScreamVolume(struct SOLDIERTYPE *pSoldier, uint8_t ubCombinedLoss) {
  // NB explosions are so loud they should drown out screams
  uint8_t ubVolume;

  if (ubCombinedLoss < 1) {
    ubVolume = 1;
  } else {
    ubVolume = ubCombinedLoss;
  }

  // Victim yells out in pain, making noise.  Yelps are louder from greater
  // wounds, but softer for more experienced soldiers.

  if (ubVolume > (10 - EffectiveExpLevel(pSoldier))) {
    ubVolume = 10 - EffectiveExpLevel(pSoldier);
  }

  /*
                  // the "Speck factor"...  He's a whiner, and extra-sensitive to pain!
                  if (ptr->trait == NERVOUS)
                          ubVolume += 2;
  */

  if (ubVolume < 0) {
    ubVolume = 0;
  }

  return (ubVolume);
}

void DoGenericHit(struct SOLDIERTYPE *pSoldier, uint8_t ubSpecial, int16_t bDirection) {
  // Based on stance, select generic hit animation
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:
      // For now, check if we are affected by a burst
      // For now, if the weapon was a gun, special 1 == burst
      // ATE: Only do this for mercs!
      if (ubSpecial == FIRE_WEAPON_BURST_SPECIAL && pSoldier->ubBodyType <= REGFEMALE) {
        // SetSoldierDesiredDirection( pSoldier, bDirection );
        EVENT_SetSoldierDirection(pSoldier, (int8_t)bDirection);
        EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->bDirection);

        EVENT_InitNewSoldierAnim(pSoldier, STANDING_BURST_HIT, 0, FALSE);
      } else {
        // Check in hand for rifle
        if (SoldierCarriesTwoHandedWeapon(pSoldier)) {
          EVENT_InitNewSoldierAnim(pSoldier, RIFLE_STAND_HIT, 0, FALSE);
        } else {
          EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_STAND, 0, FALSE);
        }
      }
      break;

    case ANIM_PRONE:

      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_PRONE, 0, FALSE);
      break;

    case ANIM_CROUCH:
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_CROUCH, 0, FALSE);
      break;
  }
}

void SoldierGotHitGunFire(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                          uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                          uint8_t ubSpecial, uint8_t ubHitLocation) {
  uint16_t usNewGridNo;
  BOOLEAN fBlownAway = FALSE;
  BOOLEAN fHeadHit = FALSE;
  BOOLEAN fFallenOver = FALSE;

  // MAYBE CHANGE TO SPECIAL ANIMATION BASED ON VALUE SET BY DAMAGE CALCULATION CODE
  // ALL THESE ONLY WORK ON STANDING PEOPLE
  if (!(pSoldier->uiStatusFlags & SOLDIER_MONSTER) &&
      gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
    if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
      if (ubSpecial == FIRE_WEAPON_HEAD_EXPLODE_SPECIAL) {
        if (gGameSettings.fOptions[TOPTION_BLOOD_N_GORE]) {
          if (SpacesAway(pSoldier->sGridNo, Menptr[ubAttackerID].sGridNo) <=
              MAX_DISTANCE_FOR_MESSY_DEATH) {
            usNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo,
                                    (int8_t)(DirectionInc(pSoldier->bDirection)));

            // CHECK OK DESTINATION!
            if (OKFallDirection(pSoldier, usNewGridNo, pSoldier->bLevel, pSoldier->bDirection,
                                JFK_HITDEATH)) {
              usNewGridNo =
                  NewGridNo((uint16_t)usNewGridNo, (int8_t)(DirectionInc(pSoldier->bDirection)));

              if (OKFallDirection(pSoldier, usNewGridNo, pSoldier->bLevel, pSoldier->bDirection,
                                  pSoldier->usAnimState)) {
                fHeadHit = TRUE;
              }
            }
          }
        }
      } else if (ubSpecial == FIRE_WEAPON_CHEST_EXPLODE_SPECIAL) {
        if (gGameSettings.fOptions[TOPTION_BLOOD_N_GORE]) {
          if (SpacesAway(pSoldier->sGridNo, Menptr[ubAttackerID].sGridNo) <=
              MAX_DISTANCE_FOR_MESSY_DEATH) {
            // possibly play torso explosion anim!
            if (pSoldier->bDirection == bDirection) {
              usNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo,
                                      DirectionInc(gOppositeDirection[pSoldier->bDirection]));

              if (OKFallDirection(pSoldier, usNewGridNo, pSoldier->bLevel,
                                  gOppositeDirection[bDirection], FLYBACK_HIT)) {
                usNewGridNo =
                    NewGridNo((uint16_t)usNewGridNo, DirectionInc(gOppositeDirection[bDirection]));

                if (OKFallDirection(pSoldier, usNewGridNo, pSoldier->bLevel,
                                    gOppositeDirection[bDirection], pSoldier->usAnimState)) {
                  fBlownAway = TRUE;
                }
              }
            }
          }
        }
      } else if (ubSpecial == FIRE_WEAPON_LEG_FALLDOWN_SPECIAL) {
        // possibly play fall over anim!
        // this one is NOT restricted by distance
        if (IsValidStance(pSoldier, ANIM_PRONE)) {
          // Can't be in water, or not standing
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND &&
              !MercInWater(pSoldier)) {
            fFallenOver = TRUE;
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[20],
                      pSoldier->name);
          }
        }
      }
    }
  }

  // IF HERE AND GUY IS DEAD, RETURN!
  if (pSoldier->uiStatusFlags & SOLDIER_DEAD) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Releasesoldierattacker,Dead soldier hit"));
    ReleaseSoldiersAttacker(pSoldier);
    return;
  }

  if (fFallenOver) {
    SoldierCollapse(pSoldier);
    return;
  }

  if (fBlownAway) {
    // Only for mercs...
    if (pSoldier->ubBodyType < 4) {
      ChangeToFlybackAnimation(pSoldier, (int8_t)bDirection);
      return;
    }
  }

  if (fHeadHit) {
    // Only for mercs ( or KIDS! )
    if (pSoldier->ubBodyType < 4 || pSoldier->ubBodyType == HATKIDCIV ||
        pSoldier->ubBodyType == KIDCIV) {
      EVENT_InitNewSoldierAnim(pSoldier, JFK_HITDEATH, 0, FALSE);
      return;
    }
  }

  DoGenericHit(pSoldier, ubSpecial, bDirection);
}

void SoldierGotHitExplosion(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                            uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                            uint8_t ubSpecial, uint8_t ubHitLocation) {
  int16_t sNewGridNo;

  // IF HERE AND GUY IS DEAD, RETURN!
  if (pSoldier->uiStatusFlags & SOLDIER_DEAD) {
    return;
  }

  // check for services
  ReceivingSoldierCancelServices(pSoldier);
  GivingSoldierCancelServices(pSoldier);

  if (gGameSettings.fOptions[TOPTION_BLOOD_N_GORE]) {
    if (Explosive[Item[usWeaponIndex].ubClassIndex].ubRadius >= 3 && pSoldier->bLife == 0 &&
        gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_PRONE) {
      if (sRange >= 2 && sRange <= 4) {
        DoMercBattleSound(pSoldier, (int8_t)(BATTLE_SOUND_HIT1 + Random(2)));

        EVENT_InitNewSoldierAnim(pSoldier, CHARIOTS_OF_FIRE, 0, FALSE);
        return;
      } else if (sRange <= 1) {
        DoMercBattleSound(pSoldier, (int8_t)(BATTLE_SOUND_HIT1 + Random(2)));

        EVENT_InitNewSoldierAnim(pSoldier, BODYEXPLODING, 0, FALSE);
        return;
      }
    }
  }

  // If we can't fal back or such, so generic hit...
  if (pSoldier->ubBodyType >= 4) {
    DoGenericHit(pSoldier, 0, bDirection);
    return;
  }

  // Based on stance, select generic hit animation
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:
    case ANIM_CROUCH:

      EVENT_SetSoldierDirection(pSoldier, (int8_t)bDirection);
      EVENT_SetSoldierDesiredDirection(pSoldier, pSoldier->bDirection);

      // Check behind us!
      sNewGridNo =
          NewGridNo((uint16_t)pSoldier->sGridNo, DirectionInc(gOppositeDirection[bDirection]));

      if (OKFallDirection(pSoldier, sNewGridNo, pSoldier->bLevel, gOppositeDirection[bDirection],
                          FLYBACK_HIT)) {
        ChangeToFallbackAnimation(pSoldier, (int8_t)bDirection);
      } else {
        if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
          BeginTyingToFall(pSoldier);
          EVENT_InitNewSoldierAnim(pSoldier, FALLFORWARD_FROMHIT_STAND, 0, FALSE);
        } else {
          SoldierCollapse(pSoldier);
        }
      }
      break;

    case ANIM_PRONE:

      SoldierCollapse(pSoldier);
      break;
  }
}

void SoldierGotHitBlade(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                        uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                        uint8_t ubSpecial, uint8_t ubHitLocation) {
  // IF HERE AND GUY IS DEAD, RETURN!
  if (pSoldier->uiStatusFlags & SOLDIER_DEAD) {
    return;
  }

  // Based on stance, select generic hit animation
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:

      // Check in hand for rifle
      if (SoldierCarriesTwoHandedWeapon(pSoldier)) {
        EVENT_InitNewSoldierAnim(pSoldier, RIFLE_STAND_HIT, 0, FALSE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_STAND, 0, FALSE);
      }
      break;

    case ANIM_CROUCH:
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_CROUCH, 0, FALSE);
      break;

    case ANIM_PRONE:
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_PRONE, 0, FALSE);
      break;
  }
}

void SoldierGotHitPunch(struct SOLDIERTYPE *pSoldier, uint16_t usWeaponIndex, int16_t sDamage,
                        uint16_t bDirection, uint16_t sRange, uint8_t ubAttackerID,
                        uint8_t ubSpecial, uint8_t ubHitLocation) {
  // IF HERE AND GUY IS DEAD, RETURN!
  if (pSoldier->uiStatusFlags & SOLDIER_DEAD) {
    return;
  }

  // Based on stance, select generic hit animation
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:
      // Check in hand for rifle
      if (SoldierCarriesTwoHandedWeapon(pSoldier)) {
        EVENT_InitNewSoldierAnim(pSoldier, RIFLE_STAND_HIT, 0, FALSE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_STAND, 0, FALSE);
      }
      break;

    case ANIM_CROUCH:
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_CROUCH, 0, FALSE);
      break;

    case ANIM_PRONE:
      EVENT_InitNewSoldierAnim(pSoldier, GENERIC_HIT_PRONE, 0, FALSE);
      break;
  }
}

BOOLEAN EVENT_InternalGetNewSoldierPath(struct SOLDIERTYPE *pSoldier, uint16_t sDestGridNo,
                                        uint16_t usMovementAnim, BOOLEAN fFromUI,
                                        BOOLEAN fForceRestartAnim) {
  int32_t iDest;
  BOOLEAN fContinue;
  uint32_t uiDist;
  uint16_t usAnimState;
  uint16_t usMoveAnimState = usMovementAnim;
  int16_t sMercGridNo;
  uint16_t usPathingData[MAX_PATH_LIST_SIZE];
  BOOLEAN fAdvancePath = TRUE;
  uint8_t fFlags = 0;

  // Ifd this code, make true if a player
  if (fFromUI == 3) {
    if (pSoldier->bTeam == gbPlayerNum) {
      fFromUI = 1;
    } else {
      fFromUI = 0;
    }
  }

  // ATE: if a civ, and from UI, and were cowering, remove from cowering
  if (AM_AN_EPC(pSoldier) && fFromUI) {
    if (pSoldier->uiStatusFlags & SOLDIER_COWERING) {
      SetSoldierCowerState(pSoldier, FALSE);
      usMoveAnimState = WALKING;
    }
  }

  pSoldier->bGoodContPath = FALSE;

  if (pSoldier->fDelayedMovement) {
    if (pSoldier->ubDelayedMovementFlags & DELAYED_MOVEMENT_FLAG_PATH_THROUGH_PEOPLE) {
      fFlags = PATH_THROUGH_PEOPLE;
    } else {
      fFlags = PATH_IGNORE_PERSON_AT_DEST;
    }
    pSoldier->fDelayedMovement = FALSE;
  }

  if (gfGetNewPathThroughPeople) {
    fFlags = PATH_THROUGH_PEOPLE;
  }

  // ATE: Some stuff here for realtime, going through interface....
  if ((!(gTacticalStatus.uiFlags & INCOMBAT) &&
       (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) && fFromUI == 1) ||
      fFromUI == 2) {
    if (pSoldier->bCollapsed) {
      return (FALSE);
    }

    sMercGridNo = pSoldier->sGridNo;
    pSoldier->sGridNo = pSoldier->sDestination;

    // Check if path is good before copying it into guy's path...
    if (FindBestPath(pSoldier, sDestGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode,
                     NO_COPYROUTE, fFlags) == 0) {
      // Set to old....
      pSoldier->sGridNo = sMercGridNo;

      return (FALSE);
    }

    uiDist = FindBestPath(pSoldier, sDestGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode,
                          COPYROUTE, fFlags);

    pSoldier->sGridNo = sMercGridNo;
    pSoldier->sFinalDestination = sDestGridNo;

    if (uiDist > 0) {
      // Add one to path data size....
      if (fAdvancePath) {
        memcpy(usPathingData, pSoldier->usPathingData, sizeof(usPathingData));
        memcpy(&(pSoldier->usPathingData[1]), usPathingData,
               sizeof(usPathingData) - sizeof(uint16_t));

        // If we have reach the max, go back one sFinalDest....
        if (pSoldier->usPathDataSize == MAX_PATH_LIST_SIZE) {
          // pSoldier->sFinalDestination = NewGridNo( (uint16_t)pSoldier->sFinalDestination,
          // DirectionInc( gOppositeDirection[ ubPathingMaxDirection ] ) );
        } else {
          pSoldier->usPathDataSize++;
        }
      }

      usMoveAnimState = pSoldier->usUIMovementMode;

      if (pSoldier->bOverTerrainType == DEEP_WATER) {
        usMoveAnimState = DEEP_WATER_SWIM;
      }

      // Change animation only.... set value to NOT call any goto new gridno stuff.....
      if (usMoveAnimState != pSoldier->usAnimState) {
        //
        pSoldier->usDontUpdateNewGridNoOnMoveAnimChange = TRUE;

        EVENT_InitNewSoldierAnim(pSoldier, usMoveAnimState, 0, FALSE);
      }

      return (TRUE);
    }

    return (FALSE);
  }

  // we can use the soldier's level here because we don't have pathing across levels right now...
  if (pSoldier->bPathStored) {
    fContinue = TRUE;
  } else {
    iDest =
        FindBestPath(pSoldier, sDestGridNo, pSoldier->bLevel, usMovementAnim, COPYROUTE, fFlags);
    fContinue = (iDest != 0);
  }

  // Only if we can get a path here
  if (fContinue) {
    // Debug messages
    DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String("Soldier %d: Get new path", GetSolID(pSoldier)));

    // Set final destination
    pSoldier->sFinalDestination = sDestGridNo;
    pSoldier->fPastXDest = 0;
    pSoldier->fPastYDest = 0;

    // CHECK IF FIRST TILE IS FREE
    NewGridNo((uint16_t)pSoldier->sGridNo,
              DirectionInc((uint8_t)pSoldier->usPathingData[pSoldier->usPathIndex]));

    // If true, we're OK, if not, WAIT for a guy to pass!
    // If we are in deep water, we can only swim!
    if (pSoldier->bOverTerrainType == DEEP_WATER) {
      usMoveAnimState = DEEP_WATER_SWIM;
    }

    // If we were aiming, end aim!
    usAnimState = PickSoldierReadyAnimation(pSoldier, TRUE);

    // Add a pending animation first!
    // Only if we were standing!
    if (usAnimState != INVALID_ANIMATION &&
        gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
      EVENT_InitNewSoldierAnim(pSoldier, usAnimState, 0, FALSE);
      pSoldier->usPendingAnimation = usMoveAnimState;
    } else {
      // Call local copy for change soldier state!
      EVENT_InitNewSoldierAnim(pSoldier, usMoveAnimState, 0, fForceRestartAnim);
    }

    // Change desired direction
    // ATE: Here we have a situation where in RT, we may have
    // gotten a new path, but we are alreayd moving.. so
    // at leasty change new dest. This will be redundent if the ANI is a totaly new one

    return (TRUE);
  }

  return (FALSE);
}

void EVENT_GetNewSoldierPath(struct SOLDIERTYPE *pSoldier, uint16_t sDestGridNo,
                             uint16_t usMovementAnim) {
  // ATE: Default restart of animation to TRUE
  EVENT_InternalGetNewSoldierPath(pSoldier, sDestGridNo, usMovementAnim, FALSE, TRUE);
}

// Change our state based on stance, to stop!
void StopSoldier(struct SOLDIERTYPE *pSoldier) {
  ReceivingSoldierCancelServices(pSoldier);
  GivingSoldierCancelServices(pSoldier);

  if (!(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY)) {
    // SoldierGotoStationaryStance( pSoldier );
    EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
  }

  // Set desination
  pSoldier->sFinalDestination = pSoldier->sGridNo;
}

void SoldierGotoStationaryStance(struct SOLDIERTYPE *pSoldier) {
  // ATE: This is to turn off fast movement, that us used to change movement mode
  // for ui display on stance changes....
  if (pSoldier->bTeam == gbPlayerNum) {
    // pSoldier->fUIMovementFast = FALSE;
  }

  // The queen, if she sees anybody, goes to ready, not normal breath....
  if (pSoldier->ubBodyType == QUEENMONSTER) {
    if (pSoldier->bOppCnt > 0 || pSoldier->bTeam == gbPlayerNum) {
      EVENT_InitNewSoldierAnim(pSoldier, QUEEN_READY, 0, TRUE);
      return;
    }
  }

  // Check if we are in deep water!
  if (pSoldier->bOverTerrainType == DEEP_WATER) {
    // IN deep water, tred!
    EVENT_InitNewSoldierAnim(pSoldier, DEEP_WATER_TRED, 0, FALSE);
  } else if (pSoldier->ubServicePartner != NOBODY && pSoldier->bLife >= OKLIFE &&
             pSoldier->bBreath > 0) {
    EVENT_InitNewSoldierAnim(pSoldier, GIVING_AID, 0, FALSE);
  } else {
    // Change state back to stationary state for given height
    switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
      case ANIM_STAND:

        // If we are cowering....goto cower state
        if (pSoldier->uiStatusFlags & SOLDIER_COWERING) {
          EVENT_InitNewSoldierAnim(pSoldier, START_COWER, 0, FALSE);
        } else {
          EVENT_InitNewSoldierAnim(pSoldier, STANDING, 0, FALSE);
        }
        break;

      case ANIM_CROUCH:

        // If we are cowering....goto cower state
        if (pSoldier->uiStatusFlags & SOLDIER_COWERING) {
          EVENT_InitNewSoldierAnim(pSoldier, COWERING, 0, FALSE);
        } else {
          EVENT_InitNewSoldierAnim(pSoldier, CROUCHING, 0, FALSE);
        }
        break;

      case ANIM_PRONE:
        EVENT_InitNewSoldierAnim(pSoldier, PRONE, 0, FALSE);
        break;
    }
  }
}

void ChangeSoldierStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance) {
  uint16_t usNewState;

  // Check if they are the same!
  if (ubDesiredStance == gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    // Free up from stance change
    FreeUpNPCFromStanceChange(pSoldier);
    return;
  }

  // Set UI Busy
  SetUIBusy(pSoldier->ubID);

  // ATE: If we are an NPC, cower....
  if (pSoldier->ubBodyType >= FATCIV && pSoldier->ubBodyType <= KIDCIV) {
    if (ubDesiredStance == ANIM_STAND) {
      SetSoldierCowerState(pSoldier, FALSE);
    } else {
      SetSoldierCowerState(pSoldier, TRUE);
    }
  } else {
    usNewState = GetNewSoldierStateFromNewStance(pSoldier, ubDesiredStance);

    // Set desired stance
    pSoldier->ubDesiredHeight = ubDesiredStance;

    // Now change to appropriate animation
    EVENT_InitNewSoldierAnim(pSoldier, usNewState, 0, FALSE);
  }
}

void EVENT_InternalSetSoldierDestination(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection,
                                         BOOLEAN fFromMove, uint16_t usAnimState) {
  uint16_t usNewGridNo;
  int16_t sXPos, sYPos;

  // Get dest gridno, convert to center coords
  usNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo, DirectionInc(usNewDirection));

  ConvertMapPosToWorldTileCenter(usNewGridNo, &sXPos, &sYPos);

  // Save new dest gridno, x, y
  pSoldier->sDestination = usNewGridNo;
  pSoldier->sDestXPos = sXPos;
  pSoldier->sDestYPos = sYPos;

  pSoldier->bMovementDirection = (int8_t)usNewDirection;

  // OK, ATE: If we are side_stepping, calculate a NEW desired direction....
  if (pSoldier->bReverse && usAnimState == SIDE_STEP) {
    uint8_t ubPerpDirection;

    // Get a new desired direction,
    ubPerpDirection = gPurpendicularDirection[pSoldier->bDirection][usNewDirection];

    // CHange actual and desired direction....
    EVENT_SetSoldierDirection(pSoldier, ubPerpDirection);
    pSoldier->bDesiredDirection = pSoldier->bDirection;
  } else {
    if (!(gAnimControl[usAnimState].uiFlags & ANIM_SPECIALMOVE)) {
      EVENT_InternalSetSoldierDesiredDirection(pSoldier, usNewDirection, fFromMove, usAnimState);
    }
  }
}

void EVENT_SetSoldierDestination(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection) {
  EVENT_InternalSetSoldierDestination(pSoldier, usNewDirection, FALSE, pSoldier->usAnimState);
}

// function to determine which direction a creature can turn in
int8_t MultiTiledTurnDirection(struct SOLDIERTYPE *pSoldier, int8_t bStartDirection,
                               int8_t bDesiredDirection) {
  int8_t bTurningIncrement;
  int8_t bCurrentDirection;
  int8_t bLoop;
  uint16_t usStructureID, usAnimSurface;
  struct STRUCTURE_FILE_REF *pStructureFileRef;
  BOOLEAN fOk = FALSE;

  // start by trying to turn in quickest direction
  bTurningIncrement = (int8_t)QuickestDirection(bStartDirection, bDesiredDirection);

  usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, pSoldier->usUIMovementMode);

  pStructureFileRef =
      GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, pSoldier->usUIMovementMode);
  if (!pStructureFileRef) {
    // without structure data, well, assume quickest direction
    return (bTurningIncrement);
  }

  // ATE: Only if we have a levelnode...
  if (pSoldier->pLevelNode != NULL && pSoldier->pLevelNode->pStructureData != NULL) {
    usStructureID = pSoldier->pLevelNode->pStructureData->usStructureID;
  } else {
    usStructureID = INVALID_STRUCTURE_ID;
  }

  bLoop = 0;
  bCurrentDirection = bStartDirection;

  while (bLoop < 2) {
    while (bCurrentDirection != bDesiredDirection) {
      bCurrentDirection += bTurningIncrement;

      // did we wrap directions?
      if (bCurrentDirection < 0) {
        bCurrentDirection = (MAXDIR - 1);
      } else if (bCurrentDirection >= MAXDIR) {
        bCurrentDirection = 0;
      }

      // check to see if we can add creature in that direction
      fOk = OkayToAddStructureToWorld(
          pSoldier->sGridNo, pSoldier->bLevel,
          &(pStructureFileRef->pDBStructureRef[gOneCDirection[bCurrentDirection]]), usStructureID);
      if (!fOk) {
        break;
      }
    }

    if ((bCurrentDirection == bDesiredDirection) && fOk) {
      // success!!
      return (bTurningIncrement);
    }

    bLoop++;
    if (bLoop < 2) {
      // change direction of loop etc
      bCurrentDirection = bStartDirection;
      bTurningIncrement *= -1;
    }
  }
  // nothing found... doesn't matter much what we return
  return (bTurningIncrement);
}

void EVENT_InternalSetSoldierDesiredDirection(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection,
                                              BOOLEAN fInitalMove, uint16_t usAnimState) {
  // if ( usAnimState == WALK_BACKWARDS )
  if (pSoldier->bReverse && usAnimState != SIDE_STEP) {
    // OK, check if we are going to go in the exact opposite than our facing....
    usNewDirection = gOppositeDirection[usNewDirection];
  }

  pSoldier->bDesiredDirection = (int8_t)usNewDirection;

  // If we are prone, goto crouched first!
  // ONly if we are stationary, and only if directions are differnet!

  // ATE: If we are fNoAPsToFinnishMove, stop what we were doing and
  // reset flag.....
  if (pSoldier->fNoAPToFinishMove && (gAnimControl[usAnimState].uiFlags & ANIM_MOVING)) {
    // ATE; Commented this out: NEVER, EVER, start a new anim from this function, as an eternal loop
    // will result....
    // SoldierGotoStationaryStance( pSoldier );
    // Reset flag!
    AdjustNoAPToFinishMove(pSoldier, FALSE);
  }

  if (pSoldier->bDesiredDirection != pSoldier->bDirection) {
    if (gAnimControl[usAnimState].uiFlags &
            (ANIM_BREATH | ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_FIREREADY) &&
        !fInitalMove && !pSoldier->fDontChargeTurningAPs) {
      // Deduct points for initial turn!
      switch (gAnimControl[usAnimState].ubEndHeight) {
        // Now change to appropriate animation
        case ANIM_STAND:
          DeductPoints(pSoldier, AP_LOOK_STANDING, 0);
          break;

        case ANIM_CROUCH:
          DeductPoints(pSoldier, AP_LOOK_CROUCHED, 0);
          break;

        case ANIM_PRONE:
          DeductPoints(pSoldier, AP_LOOK_PRONE, 0);
          break;
      }
    }

    pSoldier->fDontChargeTurningAPs = FALSE;

    if (fInitalMove) {
      if (gAnimControl[usAnimState].ubHeight == ANIM_PRONE) {
        if (pSoldier->fTurningFromPronePosition != TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE) {
          pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_START_UP_FROM_MOVE;
        }
      }
    }

    if (gAnimControl[usAnimState].uiFlags & ANIM_STATIONARY || pSoldier->fNoAPToFinishMove ||
        fInitalMove) {
      if (gAnimControl[usAnimState].ubHeight == ANIM_PRONE) {
        // Set this beasty of a flag to allow us to go back down to prone if we choose!
        // ATE: Alrighty, set flag to go back down only if we are not moving anywhere
        // if ( pSoldier->sDestination == pSoldier->sGridNo )
        if (!fInitalMove) {
          pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_ON;

          // Set a pending animation to change stance first...
          SendChangeSoldierStanceEvent(pSoldier, ANIM_CROUCH);
        }
      }
    }
  }

  // Set desired direction for the extended directions...
  pSoldier->ubHiResDesiredDirection = ubExtDirection[pSoldier->bDesiredDirection];

  if (pSoldier->bDesiredDirection != pSoldier->bDirection) {
    if (pSoldier->uiStatusFlags & (SOLDIER_VEHICLE) || CREATURE_OR_BLOODCAT(pSoldier)) {
      pSoldier->uiStatusFlags |= SOLDIER_PAUSEANIMOVE;
    }
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    pSoldier->bTurningIncrement =
        (int8_t)ExtQuickestDirection(pSoldier->ubHiResDirection, pSoldier->ubHiResDesiredDirection);
  } else {
    if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE) {
      pSoldier->bTurningIncrement = (int8_t)MultiTiledTurnDirection(pSoldier, pSoldier->bDirection,
                                                                    pSoldier->bDesiredDirection);
    } else {
      pSoldier->bTurningIncrement =
          (int8_t)QuickestDirection(pSoldier->bDirection, pSoldier->bDesiredDirection);
    }
  }
}

void EVENT_SetSoldierDesiredDirection(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection) {
  EVENT_InternalSetSoldierDesiredDirection(pSoldier, usNewDirection, FALSE, pSoldier->usAnimState);
}

void EVENT_SetSoldierDirection(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection) {
  // Remove old location data
  HandleAnimationProfile(pSoldier, pSoldier->usAnimState, TRUE);

  pSoldier->bDirection = (int8_t)usNewDirection;

  // Updated extended direction.....
  pSoldier->ubHiResDirection = ubExtDirection[pSoldier->bDirection];

  // Add new stuff
  HandleAnimationProfile(pSoldier, pSoldier->usAnimState, FALSE);

  // If we are turning, we have chaanged our aim!
  if (!pSoldier->fDontUnsetLastTargetFromTurn) {
    pSoldier->sLastTarget = NOWHERE;
  }

  AdjustForFastTurnAnimation(pSoldier);

  // Update structure info!
  //	 if ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE )
  {
    UpdateMercStructureInfo(pSoldier);
  }

  // Handle Profile data for hit locations
  HandleAnimationProfile(pSoldier, pSoldier->usAnimState, TRUE);

  HandleCrowShadowNewDirection(pSoldier);

  // Change values!
  SetSoldierLocatorOffsets(pSoldier);
}

void EVENT_BeginMercTurn(struct SOLDIERTYPE *pSoldier, BOOLEAN fFromRealTime,
                         int32_t iRealTimeCounter) {
  // NB realtimecounter is not used, always passed in as 0 now!

  int32_t iBlood;

  if (pSoldier->bUnderFire) {
    // UnderFire now starts at 2 for "under fire this turn",
    // down to 1 for "under fire last turn", to 0.
    pSoldier->bUnderFire--;
  }

  // ATE: Add decay effect sfor drugs...
  if (fFromRealTime)  //&& iRealTimeCounter % 300 )
  {
    HandleEndTurnDrugAdjustments(pSoldier);
  } else {
    HandleEndTurnDrugAdjustments(pSoldier);
  }

  // ATE: Don't bleed if in AUTO BANDAGE!
  if (!gTacticalStatus.fAutoBandageMode) {
    // Blood is not for the weak of heart, or mechanical
    if (!(pSoldier->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ROBOT))) {
      if (pSoldier->bBleeding || pSoldier->bLife < OKLIFE)  // is he bleeding or dying?
      {
        iBlood = CheckBleeding(pSoldier);  // check if he might lose another life point

        // ATE: Only if in sector!
        if (IsSolInSector(pSoldier)) {
          if (iBlood != NOBLOOD) {
            DropBlood(pSoldier, (int8_t)iBlood, pSoldier->bVisible);
          }
        }
      }
    }
  }

  // survived bleeding, but is he out of breath?
  if (pSoldier->bLife && !pSoldier->bBreath && MercInWater(pSoldier)) {
    // Drowning...
  }

  // if he is still alive (didn't bleed to death)
  if (pSoldier->bLife) {
    // reduce the effects of any residual shock from past injuries by half
    pSoldier->bShock /= 2;

    // if this person has heard a noise that hasn't been investigated
    if (pSoldier->sNoiseGridno != NOWHERE) {
      if (pSoldier->ubNoiseVolume)  // and the noise volume is still positive
      {
        pSoldier->ubNoiseVolume--;  // the volume of the noise "decays" by 1 point

        if (!pSoldier->ubNoiseVolume)  // if the volume has reached zero
        {
          pSoldier->sNoiseGridno = NOWHERE;  // forget about the noise!
        }
      }
    }

    // save unused action points up to a maximum
    /*
if ((savedPts = pSoldier->bActionPts) > MAX_AP_CARRIED)
savedPts = MAX_AP_CARRIED;
           */
    if (pSoldier->uiStatusFlags & SOLDIER_GASSED) {
      // then must get a gas mask or leave the gassed area to get over it
      if ((pSoldier->inv[HEAD1POS].usItem == GASMASK ||
           pSoldier->inv[HEAD2POS].usItem == GASMASK) ||
          !(GetSmokeEffectOnTile(pSoldier->sGridNo, pSoldier->bLevel))) {
        // Turn off gassed flag....
        pSoldier->uiStatusFlags &= (~SOLDIER_GASSED);
      }
    }

    if (pSoldier->bBlindedCounter > 0) {
      pSoldier->bBlindedCounter--;
      if (pSoldier->bBlindedCounter == 0) {
        // we can SEE!!!!!
        HandleSight(pSoldier, SIGHT_LOOK);
        // Dirty panel
        fInterfacePanelDirty = DIRTYLEVEL2;
      }
    }

    // ATE: To get around a problem...
    // If an AI guy, and we have 0 life, and are still at higher hieght,
    // Kill them.....

    pSoldier->sWeightCarriedAtTurnStart = (int16_t)CalculateCarriedWeight(pSoldier);

    UnusedAPsToBreath(pSoldier);

    // Set flag back to normal, after reaching a certain statge
    if (pSoldier->bBreath > 80) {
      pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_LOW_BREATH);
    }
    if (pSoldier->bBreath > 50) {
      pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_DROWNING);
    }

    if (pSoldier->ubTurnsUntilCanSayHeardNoise > 0) {
      pSoldier->ubTurnsUntilCanSayHeardNoise--;
    }

    if (IsSolInSector(pSoldier)) {
      CheckForBreathCollapse(pSoldier);
    }

    CalcNewActionPoints(pSoldier);

    pSoldier->bTilesMoved = 0;

    if (IsSolInSector(pSoldier)) {
      BeginSoldierGetup(pSoldier);

      // CJC Nov 30: handle RT opplist decaying in another function which operates less often
      if (gTacticalStatus.uiFlags & INCOMBAT) {
        VerifyAndDecayOpplist(pSoldier);

        // turn off xray
        if (pSoldier->uiXRayActivatedTime) {
          TurnOffXRayEffects(pSoldier);
        }
      }

      if ((pSoldier->bTeam == gbPlayerNum) && (GetSolProfile(pSoldier) != NO_PROFILE)) {
        switch (gMercProfiles[GetSolProfile(pSoldier)].bPersonalityTrait) {
          case FEAR_OF_INSECTS:
            if (MercSeesCreature(pSoldier)) {
              HandleMoraleEvent(pSoldier, MORALE_INSECT_PHOBIC_SEES_CREATURE,
                                GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                GetSolSectorZ(pSoldier));
              if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY)) {
                TacticalCharacterDialogue(pSoldier, QUOTE_PERSONALITY_TRAIT);
                pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
              }
            }
            break;
          case CLAUSTROPHOBIC:
            if (gbWorldSectorZ > 0 && Random(6 - gbWorldSectorZ) == 0) {
              // underground!
              HandleMoraleEvent(pSoldier, MORALE_CLAUSTROPHOBE_UNDERGROUND, GetSolSectorX(pSoldier),
                                GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));
              if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY)) {
                TacticalCharacterDialogue(pSoldier, QUOTE_PERSONALITY_TRAIT);
                pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
              }
            }
            break;
          case NERVOUS:
            if (DistanceToClosestFriend(pSoldier) > NERVOUS_RADIUS) {
              // augh!!
              if (pSoldier->bMorale < 50) {
                HandleMoraleEvent(pSoldier, MORALE_NERVOUS_ALONE, GetSolSectorX(pSoldier),
                                  GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));
                if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_PERSONALITY)) {
                  TacticalCharacterDialogue(pSoldier, QUOTE_PERSONALITY_TRAIT);
                  pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_PERSONALITY;
                }
              }
            } else {
              if (pSoldier->bMorale > 45) {
                // turn flag off, so that we say it every two turns
                pSoldier->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_PERSONALITY;
              }
            }
            break;
        }
      }
    }

    // Reset quote flags for under heavy fire and close call!
    pSoldier->usQuoteSaidFlags &= (~SOLDIER_QUOTE_SAID_BEING_PUMMELED);
    pSoldier->usQuoteSaidExtFlags &= (~SOLDIER_QUOTE_SAID_EXT_CLOSE_CALL);
    pSoldier->bNumHitsThisTurn = 0;
    pSoldier->ubSuppressionPoints = 0;
    pSoldier->fCloseCall = FALSE;

    pSoldier->ubMovementNoiseHeard = 0;

    // If soldier has new APs, reset flags!
    if (pSoldier->bActionPoints > 0) {
      pSoldier->fUIFirstTimeNOAP = FALSE;
      pSoldier->bMoved = FALSE;
      pSoldier->bPassedLastInterrupt = FALSE;
    }
  }
}

// UTILITY FUNCTIONS CALLED BY OVERHEAD.H
uint8_t gDirectionFrom8to2[] = {0, 0, 1, 1, 0, 1, 1, 0};

BOOLEAN ConvertAniCodeToAniFrame(struct SOLDIERTYPE *pSoldier, uint16_t usAniFrame) {
  uint16_t usAnimSurface;
  uint8_t ubTempDir;
  // Given ani code, adjust for facing direction

  // get anim surface and determine # of frames
  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  CHECKF(usAnimSurface != INVALID_ANIMATION_SURFACE);

  // COnvert world direction into sprite direction
  ubTempDir = gOneCDirection[pSoldier->bDirection];

  // If we are only one frame, ignore what the script is telling us!
  if (gAnimSurfaceDatabase[usAnimSurface].ubFlags & ANIM_DATA_FLAG_NOFRAMES) {
    usAniFrame = 0;
  }

  if (gAnimSurfaceDatabase[usAnimSurface].uiNumDirections == 32) {
    ubTempDir = gExtOneCDirection[pSoldier->ubHiResDirection];
  }
  // Check # of directions /surface, adjust if ness.
  else if (gAnimSurfaceDatabase[usAnimSurface].uiNumDirections == 4) {
    ubTempDir = ubTempDir / 2;
  }
  // Check # of directions /surface, adjust if ness.
  else if (gAnimSurfaceDatabase[usAnimSurface].uiNumDirections == 1) {
    ubTempDir = 0;
  }
  // Check # of directions /surface, adjust if ness.
  else if (gAnimSurfaceDatabase[usAnimSurface].uiNumDirections == 3) {
    if (pSoldier->bDirection == NORTHWEST) {
      ubTempDir = 1;
    }
    if (pSoldier->bDirection == WEST) {
      ubTempDir = 0;
    }
    if (pSoldier->bDirection == EAST) {
      ubTempDir = 2;
    }
  } else if (gAnimSurfaceDatabase[usAnimSurface].uiNumDirections == 2) {
    ubTempDir = gDirectionFrom8to2[pSoldier->bDirection];
  }

  pSoldier->usAniFrame =
      usAniFrame + (uint16_t)((gAnimSurfaceDatabase[usAnimSurface].uiNumFramesPerDir * ubTempDir));

  if (gAnimSurfaceDatabase[usAnimSurface].hVideoObject == NULL) {
    pSoldier->usAniFrame = 0;
    return (TRUE);
  }

  if (pSoldier->usAniFrame >= gAnimSurfaceDatabase[usAnimSurface].hVideoObject->usNumberOfObjects) {
    // Debug msg here....
    //		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Soldier Animation: Wrong Number
    // of frames per number of objects: %d vs %d, %S",  gAnimSurfaceDatabase[ usAnimSurface
    //].uiNumFramesPerDir, gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject->usNumberOfObjects,
    // gAnimControl[ pSoldier->usAnimState ].zAnimStr );

    pSoldier->usAniFrame = 0;
  }

  return (TRUE);
}

void TurnSoldier(struct SOLDIERTYPE *pSoldier) {
  int16_t sDirection;
  BOOLEAN fDoDirectionChange = TRUE;
  int32_t cnt;

  // If we are a vehicle... DON'T TURN!
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    if (pSoldier->ubBodyType != TANK_NW && pSoldier->ubBodyType != TANK_NE) {
      return;
    }
  }

  // We handle sight now....
  if (pSoldier->uiStatusFlags & SOLDIER_LOOK_NEXT_TURNSOLDIER) {
    if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY &&
         pSoldier->usAnimState != CLIMBUPROOF && pSoldier->usAnimState != CLIMBDOWNROOF)) {
      // HANDLE SIGHT!
      HandleSight(pSoldier, SIGHT_LOOK | SIGHT_RADIO);
    }
    // Turn off!
    pSoldier->uiStatusFlags &= (~SOLDIER_LOOK_NEXT_TURNSOLDIER);

    HandleSystemNewAISituation(pSoldier, FALSE);
  }

  if (pSoldier->fTurningToShoot) {
    if (pSoldier->bDirection == pSoldier->bDesiredDirection) {
      if (((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIREREADY) &&
           !pSoldier->fTurningFromPronePosition) ||
          pSoldier->ubBodyType == ROBOTNOWEAPON || pSoldier->ubBodyType == TANK_NW ||
          pSoldier->ubBodyType == TANK_NE) {
        EVENT_InitNewSoldierAnim(
            pSoldier,
            SelectFireAnimation(pSoldier, gAnimControl[pSoldier->usAnimState].ubEndHeight), 0,
            FALSE);
        pSoldier->fTurningToShoot = FALSE;

        // Save last target gridno!
        // pSoldier->sLastTarget = pSoldier->sTargetGridNo;

      }
      // Else check if we are trying to shoot and once was prone, but am now crouched because we
      // needed to turn...
      else if (pSoldier->fTurningFromPronePosition) {
        if (IsValidStance(pSoldier, ANIM_PRONE)) {
          SendChangeSoldierStanceEvent(pSoldier, ANIM_PRONE);
          pSoldier->usPendingAnimation = SelectFireAnimation(pSoldier, ANIM_PRONE);
        } else {
          EVENT_InitNewSoldierAnim(pSoldier, SelectFireAnimation(pSoldier, ANIM_CROUCH), 0, FALSE);
        }
        pSoldier->fTurningToShoot = FALSE;
        pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
      }
    }
  }

  if (pSoldier->fTurningToFall) {
    if (pSoldier->bDirection == pSoldier->bDesiredDirection) {
      SelectFallAnimation(pSoldier);
      pSoldier->fTurningToFall = FALSE;
    }
  }

  if (pSoldier->fTurningUntilDone && (pSoldier->ubPendingStanceChange != NO_PENDING_STANCE)) {
    if (pSoldier->bDirection == pSoldier->bDesiredDirection) {
      SendChangeSoldierStanceEvent(pSoldier, pSoldier->ubPendingStanceChange);
      pSoldier->ubPendingStanceChange = NO_PENDING_STANCE;
      pSoldier->fTurningUntilDone = FALSE;
    }
  }

  if (pSoldier->fTurningUntilDone && (pSoldier->usPendingAnimation != NO_PENDING_ANIMATION)) {
    if (pSoldier->bDirection == pSoldier->bDesiredDirection) {
      uint16_t usPendingAnimation;

      usPendingAnimation = pSoldier->usPendingAnimation;
      pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;

      EVENT_InitNewSoldierAnim(pSoldier, usPendingAnimation, 0, FALSE);
      pSoldier->fTurningUntilDone = FALSE;
    }
  }

  // Don't do anything if we are at dest direction!
  if (pSoldier->bDirection == pSoldier->bDesiredDirection) {
    if (pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE) {
      if (pSoldier->iTuringSoundID != NO_SAMPLE) {
        SoundStop(pSoldier->iTuringSoundID);
        pSoldier->iTuringSoundID = NO_SAMPLE;

        PlaySoldierJA2Sample(pSoldier->ubID, TURRET_STOP, RATE_11025,
                             SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
                             SoundDir(pSoldier->sGridNo), TRUE);
      }
    }

    // Turn off!
    pSoldier->uiStatusFlags &= (~SOLDIER_LOOK_NEXT_TURNSOLDIER);
    pSoldier->fDontUnsetLastTargetFromTurn = FALSE;

    // Unset ui busy if from ui
    if (pSoldier->bTurningFromUI && (pSoldier->fTurningFromPronePosition != 3) &&
        (pSoldier->fTurningFromPronePosition != 1)) {
      UnSetUIBusy(pSoldier->ubID);
      pSoldier->bTurningFromUI = FALSE;
    }

    if (pSoldier->uiStatusFlags & (SOLDIER_VEHICLE) || CREATURE_OR_BLOODCAT(pSoldier)) {
      pSoldier->uiStatusFlags &= (~SOLDIER_PAUSEANIMOVE);
    }

    FreeUpNPCFromTurning(pSoldier, LOOK);

    // Undo our flag for prone turning...
    // Else check if we are trying to shoot and once was prone, but am now crouched because we
    // needed to turn...
    if (pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_ON) {
      // ATE: Don't do this if we have something in our hands we are going to throw!
      if (IsValidStance(pSoldier, ANIM_PRONE) && pSoldier->pTempObject == NULL) {
        SendChangeSoldierStanceEvent(pSoldier, ANIM_PRONE);
      }
      pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
    }

    // If a special code, make guy crawl after stance change!
    if (pSoldier->fTurningFromPronePosition == TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE &&
        pSoldier->usAnimState != PRONE_UP && pSoldier->usAnimState != PRONE_DOWN) {
      if (IsValidStance(pSoldier, ANIM_PRONE)) {
        EVENT_InitNewSoldierAnim(pSoldier, CRAWLING, 0, FALSE);
      }
    }

    if (pSoldier->uiStatusFlags & SOLDIER_TURNINGFROMHIT) {
      if (pSoldier->fGettingHit == 1) {
        if (pSoldier->usPendingAnimation != FALLFORWARD_ROOF &&
            pSoldier->usPendingAnimation != FALLOFF && pSoldier->usAnimState != FALLFORWARD_ROOF &&
            pSoldier->usAnimState != FALLOFF) {
          // Go back to original direction
          EVENT_SetSoldierDesiredDirection(pSoldier, (int8_t)pSoldier->uiPendingActionData1);

          // SETUP GETTING HIT FLAG TO 2
          pSoldier->fGettingHit = 2;
        } else {
          pSoldier->uiStatusFlags &= (~SOLDIER_TURNINGFROMHIT);
        }
      } else if (pSoldier->fGettingHit == 2) {
        // Turn off
        pSoldier->uiStatusFlags &= (~SOLDIER_TURNINGFROMHIT);

        // Release attacker
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("@@@@@@@ Releasesoldierattacker, turning from hit animation ended"));
        ReleaseSoldiersAttacker(pSoldier);

        // FREEUP GETTING HIT FLAG
        pSoldier->fGettingHit = FALSE;
      }
    }

    return;
  }

  // IF WE ARE HERE, WE ARE IN THE PROCESS OF TURNING

  // double CHECK TO UNSET fNOAPs...
  if (pSoldier->fNoAPToFinishMove) {
    AdjustNoAPToFinishMove(pSoldier, FALSE);
  }

  // Do something different for vehicles....
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    fDoDirectionChange = FALSE;

    // Get new direction
    /*
    sDirection = pSoldier->ubHiResDirection + ExtQuickestDirection( pSoldier->ubHiResDirection,
    pSoldier->ubHiResDesiredDirection );
    */
    sDirection = pSoldier->ubHiResDirection + pSoldier->bTurningIncrement;
    if (sDirection > 31) {
      sDirection = 0;
    } else {
      if (sDirection < 0) {
        sDirection = 31;
      }
    }
    pSoldier->ubHiResDirection = (uint8_t)sDirection;

    // Are we at a multiple of a 'cardnal' direction?
    for (cnt = 0; cnt < 8; cnt++) {
      if (sDirection == ubExtDirection[cnt]) {
        fDoDirectionChange = TRUE;

        sDirection = (int16_t)cnt;

        break;
      }
    }

    if (pSoldier->ubBodyType == TANK_NW || pSoldier->ubBodyType == TANK_NE) {
      if (pSoldier->iTuringSoundID == NO_SAMPLE) {
        pSoldier->iTuringSoundID = PlaySoldierJA2Sample(pSoldier->ubID, TURRET_MOVE, RATE_11025,
                                                        SoundVolume(HIGHVOLUME, pSoldier->sGridNo),
                                                        100, SoundDir(pSoldier->sGridNo), TRUE);
      }
    }
  } else {
    // Get new direction
    // sDirection = pSoldier->bDirection + QuickestDirection( pSoldier->bDirection,
    // pSoldier->bDesiredDirection );
    sDirection = pSoldier->bDirection + pSoldier->bTurningIncrement;
    if (sDirection > 7) {
      sDirection = 0;
    } else {
      if (sDirection < 0) {
        sDirection = 7;
      }
    }
  }

  // CHECK FOR A VALID TURN DIRECTION
  // This is needed for prone animations as well as any multi-tiled structs
  if (fDoDirectionChange) {
    if (OKToAddMercToWorld(pSoldier, (int8_t)sDirection)) {
      // Don't do this if we are walkoing off screen...
      if (gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALKOFF_SCREEN ||
          gubWaitingForAllMercsToExitCode == WAIT_FOR_MERCS_TO_WALK_TO_GRIDNO) {
      } else {
        // ATE: We should only do this if we are STATIONARY!
        if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY)) {
          pSoldier->uiStatusFlags |= SOLDIER_LOOK_NEXT_TURNSOLDIER;
        }
        // otherwise, it's handled next tile...
      }

      EVENT_SetSoldierDirection(pSoldier, sDirection);

      if (pSoldier->ubBodyType != LARVAE_MONSTER && !MercInWater(pSoldier) &&
          pSoldier->bOverTerrainType != DIRT_ROAD && pSoldier->bOverTerrainType != PAVED_ROAD) {
        PlaySoldierFootstepSound(pSoldier);
      }
    } else {
      // Are we prone crawling?
      if (pSoldier->usAnimState == CRAWLING) {
        // OK, we want to getup, turn and go prone again....
        SendChangeSoldierStanceEvent(pSoldier, ANIM_CROUCH);
        pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_ENDING_UP_FROM_MOVE;
      }
      // If we are a creature, or multi-tiled, cancel AI action.....?
      else if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE) {
        pSoldier->bDesiredDirection = pSoldier->bDirection;
      }
    }
  }
}

uint8_t gRedGlowR[] = {
    0,  // Normal shades
    25, 50, 75, 100, 125, 150, 175, 200, 225,

    0,  // For gray palettes
    25, 50, 75, 100, 125, 150, 175, 200, 225,

};

#if 0
uint8_t	gOrangeGlowR[]=
{
	0,			// Normal shades
	20,
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,

	0,		// For gray palettes
	20,
	40,
	60,
	80,
	100,
	120,
	140,
	160,
	180,
};
#endif

uint8_t gOrangeGlowR[] = {
    0,  // Normal shades
    25, 50, 75, 100, 125, 150, 175, 200, 225,

    0,  // For gray palettes
    25, 50, 75, 100, 125, 150, 175, 200, 225,

};

#if 0
uint8_t	gOrangeGlowG[]=
{
	0,			// Normal shades
	5,
	10,
	25,
	30,
	35,
	40,
	45,
	50,
	55,

	0,		// For gray palettes
	5,
	10,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
};
#endif

uint8_t gOrangeGlowG[] = {
    0,  // Normal shades
    20, 40, 60, 80, 100, 120, 140, 160, 180,

    0,  // For gray palettes
    20, 40, 60, 80, 100, 120, 140, 160, 180,

};

BOOLEAN CreateSoldierPalettes(struct SOLDIERTYPE *pSoldier) {
  uint16_t usAnimSurface, usPaletteAnimSurface;
  char zColFilename[100];
  int32_t iWhich;
  int32_t cnt;
  int8_t bBodyTypePalette;
  struct JPaletteEntry Temp8BPPPalette[256];

  // NT32 uiCount;
  // PPaletteEntry Pal[256];

  if (pSoldier->p8BPPPalette != NULL) {
    MemFree(pSoldier->p8BPPPalette);
    pSoldier->p8BPPPalette = NULL;
  }

  // Allocate mem for new palette
  pSoldier->p8BPPPalette = (struct JPaletteEntry *)MemAlloc(sizeof(struct JPaletteEntry) * 256);
  memset(pSoldier->p8BPPPalette, 0, sizeof(struct JPaletteEntry) * 256);

  CHECKF(pSoldier->p8BPPPalette != NULL);

  // --- TAKE FROM CURRENT ANIMATION struct VObject*!
  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  CHECKF(usAnimSurface != INVALID_ANIMATION_SURFACE);

  if ((bBodyTypePalette = GetBodyTypePaletteSubstitutionCode(pSoldier, pSoldier->ubBodyType,
                                                             zColFilename)) == -1) {
    // ATE: here we want to use the breath cycle for the palette.....
    usPaletteAnimSurface = LoadSoldierAnimationSurface(pSoldier, STANDING);

    if (usPaletteAnimSurface != INVALID_ANIMATION_SURFACE) {
      // Use palette from struct VObject*, then use substitution for pants, etc
      memcpy(pSoldier->p8BPPPalette,
             gAnimSurfaceDatabase[usPaletteAnimSurface].hVideoObject->pPaletteEntry,
             sizeof(struct JPaletteEntry) * 256);

      // Substitute based on head, etc
      SetPaletteReplacement(pSoldier->p8BPPPalette, pSoldier->HeadPal);
      SetPaletteReplacement(pSoldier->p8BPPPalette, pSoldier->VestPal);
      SetPaletteReplacement(pSoldier->p8BPPPalette, pSoldier->PantsPal);
      SetPaletteReplacement(pSoldier->p8BPPPalette, pSoldier->SkinPal);
    }
  } else if (bBodyTypePalette == 0) {
    // Use palette from hvobject
    memcpy(pSoldier->p8BPPPalette, gAnimSurfaceDatabase[usAnimSurface].hVideoObject->pPaletteEntry,
           sizeof(struct JPaletteEntry) * 256);
  } else {
    // Use col file
    if (CreateSGPPaletteFromCOLFile(Temp8BPPPalette, zColFilename)) {
      // Copy into palette
      memcpy(pSoldier->p8BPPPalette, Temp8BPPPalette, sizeof(struct JPaletteEntry) * 256);
    } else {
      // Use palette from hvobject
      memcpy(pSoldier->p8BPPPalette,
             gAnimSurfaceDatabase[usAnimSurface].hVideoObject->pPaletteEntry,
             sizeof(struct JPaletteEntry) * 256);
    }
  }

  if (pSoldier->p16BPPPalette != NULL) {
    MemFree(pSoldier->p16BPPPalette);
    pSoldier->p16BPPPalette = NULL;
  }

  // -- BUILD 16BPP Palette from this
  pSoldier->p16BPPPalette = Create16BPPPalette(pSoldier->p8BPPPalette);

  for (iWhich = 0; iWhich < NUM_SOLDIER_SHADES; iWhich++) {
    if (pSoldier->pShades[iWhich] != NULL) {
      MemFree(pSoldier->pShades[iWhich]);
      pSoldier->pShades[iWhich] = NULL;
    }
  }

  for (iWhich = 0; iWhich < NUM_SOLDIER_EFFECTSHADES; iWhich++) {
    if (pSoldier->pEffectShades[iWhich] != NULL) {
      MemFree(pSoldier->pEffectShades[iWhich]);
      pSoldier->pEffectShades[iWhich] = NULL;
    }
  }

  for (iWhich = 0; iWhich < 20; iWhich++) {
    if (pSoldier->pGlowShades[iWhich] != NULL) {
      MemFree(pSoldier->pGlowShades[iWhich]);
      pSoldier->pGlowShades[iWhich] = NULL;
    }
  }

  CreateSoldierPaletteTables(pSoldier, HVOBJECT_GLOW_GREEN);

  // Build a grayscale palette for testing grayout of mercs
  // for(uiCount=0; uiCount < 256; uiCount++)
  //{
  //	Pal[uiCount].red=(uint8_t)(uiCount%128)+128;
  //	Pal[uiCount].green=(uint8_t)(uiCount%128)+128;
  //	Pal[uiCount].blue=(uint8_t)(uiCount%128)+128;
  //}
  pSoldier->pEffectShades[0] =
      Create16BPPPaletteShaded(pSoldier->p8BPPPalette, 100, 100, 100, TRUE);
  pSoldier->pEffectShades[1] =
      Create16BPPPaletteShaded(pSoldier->p8BPPPalette, 100, 150, 100, TRUE);

  // Build shades for glowing visible bad guy

  // First do visible guy
  pSoldier->pGlowShades[0] = Create16BPPPaletteShaded(pSoldier->p8BPPPalette, 255, 255, 255, FALSE);
  for (cnt = 1; cnt < 10; cnt++) {
    pSoldier->pGlowShades[cnt] =
        CreateEnemyGlow16BPPPalette(pSoldier->p8BPPPalette, gRedGlowR[cnt], 255, FALSE);
  }

  // Now for gray guy...
  pSoldier->pGlowShades[10] = Create16BPPPaletteShaded(pSoldier->p8BPPPalette, 100, 100, 100, TRUE);
  for (cnt = 11; cnt < 19; cnt++) {
    pSoldier->pGlowShades[cnt] =
        CreateEnemyGreyGlow16BPPPalette(pSoldier->p8BPPPalette, gRedGlowR[cnt], 0, FALSE);
  }
  pSoldier->pGlowShades[19] =
      CreateEnemyGreyGlow16BPPPalette(pSoldier->p8BPPPalette, gRedGlowR[18], 0, FALSE);

  // ATE: OK, piggyback on the shades we are not using for 2 colored lighting....
  // ORANGE, VISIBLE GUY
  pSoldier->pShades[20] = Create16BPPPaletteShaded(pSoldier->p8BPPPalette, 255, 255, 255, FALSE);
  for (cnt = 21; cnt < 30; cnt++) {
    pSoldier->pShades[cnt] = CreateEnemyGlow16BPPPalette(
        pSoldier->p8BPPPalette, gOrangeGlowR[(cnt - 20)], gOrangeGlowG[(cnt - 20)], TRUE);
  }

  // ORANGE, GREY GUY
  pSoldier->pShades[30] = Create16BPPPaletteShaded(pSoldier->p8BPPPalette, 100, 100, 100, TRUE);
  for (cnt = 31; cnt < 39; cnt++) {
    pSoldier->pShades[cnt] = CreateEnemyGreyGlow16BPPPalette(
        pSoldier->p8BPPPalette, gOrangeGlowR[(cnt - 20)], gOrangeGlowG[(cnt - 20)], TRUE);
  }
  pSoldier->pShades[39] = CreateEnemyGreyGlow16BPPPalette(pSoldier->p8BPPPalette, gOrangeGlowR[18],
                                                          gOrangeGlowG[18], TRUE);

  return (TRUE);
}

void AdjustAniSpeed(struct SOLDIERTYPE *pSoldier) {
  if ((gTacticalStatus.uiFlags & SLOW_ANIMATION)) {
    if (gTacticalStatus.bRealtimeSpeed == -1) {
      pSoldier->sAniDelay = 10000;
    } else {
      pSoldier->sAniDelay = pSoldier->sAniDelay * (1 * gTacticalStatus.bRealtimeSpeed / 2);
    }
  }

  RESETTIMECOUNTER(pSoldier->UpdateCounter, pSoldier->sAniDelay);
}

void CalculateSoldierAniSpeed(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pStatsSoldier) {
  uint32_t uiTerrainDelay;
  uint32_t uiSpeed = 0;

  int8_t bBreathDef, bLifeDef, bAgilDef;
  int8_t bAdditional = 0;

  // for those animations which have a speed of zero, we have to calculate it
  // here. Some animation, such as water-movement, have an ADDITIONAL speed
  switch (pSoldier->usAnimState) {
    case PRONE:
    case STANDING:

      pSoldier->sAniDelay = (pStatsSoldier->bBreath * 2) + (100 - pStatsSoldier->bLife);

      // Limit it!
      if (pSoldier->sAniDelay < 40) {
        pSoldier->sAniDelay = 40;
      }
      AdjustAniSpeed(pSoldier);
      return;

    case CROUCHING:

      pSoldier->sAniDelay = (pStatsSoldier->bBreath * 2) + ((100 - pStatsSoldier->bLife));

      // Limit it!
      if (pSoldier->sAniDelay < 40) {
        pSoldier->sAniDelay = 40;
      }
      AdjustAniSpeed(pSoldier);
      return;

    case WALKING:

      // Adjust based on body type
      bAdditional = (uint8_t)(gubAnimWalkSpeeds[pStatsSoldier->ubBodyType].sSpeed);
      if (bAdditional < 0) bAdditional = 0;
      break;

    case RUNNING:

      // Adjust based on body type
      bAdditional = (uint8_t)gubAnimRunSpeeds[pStatsSoldier->ubBodyType].sSpeed;
      if (bAdditional < 0) bAdditional = 0;
      break;

    case SWATTING:

      // Adjust based on body type
      if (pStatsSoldier->ubBodyType <= REGFEMALE) {
        bAdditional = (uint8_t)gubAnimSwatSpeeds[pStatsSoldier->ubBodyType].sSpeed;
        if (bAdditional < 0) bAdditional = 0;
      }
      break;

    case CRAWLING:

      // Adjust based on body type
      if (pStatsSoldier->ubBodyType <= REGFEMALE) {
        bAdditional = (uint8_t)gubAnimCrawlSpeeds[pStatsSoldier->ubBodyType].sSpeed;
        if (bAdditional < 0) bAdditional = 0;
      }
      break;

    case READY_RIFLE_STAND:

      // Raise rifle based on aim vs non-aim.
      if (pSoldier->bAimTime == 0) {
        // Quick shot
        pSoldier->sAniDelay = 70;
      } else {
        pSoldier->sAniDelay = 150;
      }
      AdjustAniSpeed(pSoldier);
      return;
  }

  // figure out movement speed (terrspeed)
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) {
    uiSpeed = gsTerrainTypeSpeedModifiers[pStatsSoldier->bOverTerrainType];

    uiTerrainDelay = uiSpeed;
  } else {
    uiTerrainDelay = 40;  // standing still
  }

  bBreathDef = 50 - (pStatsSoldier->bBreath / 2);

  if (bBreathDef > 30) bBreathDef = 30;

  bAgilDef = 50 - (EffectiveAgility(pStatsSoldier) / 4);
  bLifeDef = 50 - (pStatsSoldier->bLife / 2);

  uiTerrainDelay += (bLifeDef + bBreathDef + bAgilDef + bAdditional);

  pSoldier->sAniDelay = (int16_t)uiTerrainDelay;

  // If a moving animation and w/re on drugs, increase speed....
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) {
    if (GetDrugEffect(pSoldier, DRUG_TYPE_ADRENALINE)) {
      pSoldier->sAniDelay = pSoldier->sAniDelay / 2;
    }
  }

  // MODIFTY NOW BASED ON REAL-TIME, ETC
  // Adjust speed, make twice as fast if in turn-based!
  if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
    pSoldier->sAniDelay = pSoldier->sAniDelay / 2;
  }

  // MODIFY IF REALTIME COMBAT
  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    // ATE: If realtime, and stealth mode...
    if (pStatsSoldier->bStealthMode) {
      pSoldier->sAniDelay = (int16_t)(pSoldier->sAniDelay * 2);
    }

    // pSoldier->sAniDelay = pSoldier->sAniDelay * ( 1 * gTacticalStatus.bRealtimeSpeed / 2 );
  }
}

void SetSoldierAniSpeed(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pStatsSoldier;

  // ATE: If we are an enemy and are not visible......
  // Set speed to 0
  if ((gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) ||
      gTacticalStatus.fAutoBandageMode) {
    if (((pSoldier->bVisible == -1 && pSoldier->bVisible == pSoldier->bLastRenderVisibleValue) ||
         gTacticalStatus.fAutoBandageMode) &&
        pSoldier->usAnimState != MONSTER_UP) {
      pSoldier->sAniDelay = 0;
      RESETTIMECOUNTER(pSoldier->UpdateCounter, pSoldier->sAniDelay);
      return;
    }
  }

  // Default stats soldier to same as normal soldier.....
  pStatsSoldier = pSoldier;

  if (pSoldier->fUseMoverrideMoveSpeed) {
    pStatsSoldier = MercPtrs[pSoldier->bOverrideMoveSpeed];
  }

  // Only calculate if set to zero
  if ((pSoldier->sAniDelay = gAnimControl[pSoldier->usAnimState].sSpeed) == 0) {
    CalculateSoldierAniSpeed(pSoldier, pStatsSoldier);
  }

  AdjustAniSpeed(pSoldier);

  if (_KeyDown(SPACE)) {
    // pSoldier->sAniDelay = 1000;
  }
}

///////////////////////////////////////////////////////
// PALETTE REPLACEMENT FUNCTIONS
///////////////////////////////////////////////////////
BOOLEAN LoadPaletteData() {
  HWFILE hFile;
  uint32_t cnt, cnt2;

  hFile = FileMan_Open(PALETTEFILENAME, FILE_ACCESS_READ, FALSE);

  // Read # of types
  if (!FileMan_Read(hFile, &guiNumPaletteSubRanges, sizeof(guiNumPaletteSubRanges), NULL)) {
    return (FALSE);
  }

  // Malloc!
  gpPaletteSubRanges =
      (PaletteSubRangeType *)MemAlloc(sizeof(PaletteSubRangeType) * guiNumPaletteSubRanges);
  gubpNumReplacementsPerRange = (uint8_t *)MemAlloc(sizeof(uint8_t) * guiNumPaletteSubRanges);

  // Read # of types for each!
  for (cnt = 0; cnt < guiNumPaletteSubRanges; cnt++) {
    if (!FileMan_Read(hFile, &gubpNumReplacementsPerRange[cnt], sizeof(uint8_t), NULL)) {
      return (FALSE);
    }
  }

  // Loop for each one, read in data
  for (cnt = 0; cnt < guiNumPaletteSubRanges; cnt++) {
    if (!FileMan_Read(hFile, &gpPaletteSubRanges[cnt].ubStart, sizeof(uint8_t), NULL)) {
      return (FALSE);
    }
    if (!FileMan_Read(hFile, &gpPaletteSubRanges[cnt].ubEnd, sizeof(uint8_t), NULL)) {
      return (FALSE);
    }
  }

  // Read # of palettes
  if (!FileMan_Read(hFile, &guiNumReplacements, sizeof(guiNumReplacements), NULL)) {
    return (FALSE);
  }

  // Malloc!
  gpPalRep =
      (PaletteReplacementType *)MemAlloc(sizeof(PaletteReplacementType) * guiNumReplacements);

  // Read!
  for (cnt = 0; cnt < guiNumReplacements; cnt++) {
    // type
    if (!FileMan_Read(hFile, &gpPalRep[cnt].ubType, sizeof(gpPalRep[cnt].ubType), NULL)) {
      return (FALSE);
    }

    if (!FileMan_Read(hFile, &gpPalRep[cnt].ID, sizeof(gpPalRep[cnt].ID), NULL)) {
      return (FALSE);
    }

    // # entries
    if (!FileMan_Read(hFile, &gpPalRep[cnt].ubPaletteSize, sizeof(gpPalRep[cnt].ubPaletteSize),
                      NULL)) {
      return (FALSE);
    }

    // Malloc
    gpPalRep[cnt].r = (uint8_t *)MemAlloc(gpPalRep[cnt].ubPaletteSize);
    CHECKF(gpPalRep[cnt].r != NULL);
    gpPalRep[cnt].g = (uint8_t *)MemAlloc(gpPalRep[cnt].ubPaletteSize);
    CHECKF(gpPalRep[cnt].g != NULL);
    gpPalRep[cnt].b = (uint8_t *)MemAlloc(gpPalRep[cnt].ubPaletteSize);
    CHECKF(gpPalRep[cnt].b != NULL);

    for (cnt2 = 0; cnt2 < gpPalRep[cnt].ubPaletteSize; cnt2++) {
      if (!FileMan_Read(hFile, &gpPalRep[cnt].r[cnt2], sizeof(uint8_t), NULL)) {
        return (FALSE);
      }
      if (!FileMan_Read(hFile, &gpPalRep[cnt].g[cnt2], sizeof(uint8_t), NULL)) {
        return (FALSE);
      }
      if (!FileMan_Read(hFile, &gpPalRep[cnt].b[cnt2], sizeof(uint8_t), NULL)) {
        return (FALSE);
      }
    }
  }

  FileMan_Close(hFile);

  return (TRUE);
}

BOOLEAN SetPaletteReplacement(struct JPaletteEntry *p8BPPPalette, PaletteRepID aPalRep) {
  uint32_t cnt2;
  uint8_t ubType;
  uint8_t ubPalIndex;

  CHECKF(GetPaletteRepIndexFromID(aPalRep, &ubPalIndex));

  // Get range type
  ubType = gpPalRep[ubPalIndex].ubType;

  for (cnt2 = gpPaletteSubRanges[ubType].ubStart; cnt2 <= gpPaletteSubRanges[ubType].ubEnd;
       cnt2++) {
    p8BPPPalette[cnt2].red = gpPalRep[ubPalIndex].r[cnt2 - gpPaletteSubRanges[ubType].ubStart];
    p8BPPPalette[cnt2].green = gpPalRep[ubPalIndex].g[cnt2 - gpPaletteSubRanges[ubType].ubStart];
    p8BPPPalette[cnt2].blue = gpPalRep[ubPalIndex].b[cnt2 - gpPaletteSubRanges[ubType].ubStart];
  }

  return (TRUE);
}

BOOLEAN DeletePaletteData() {
  uint32_t cnt;

  // Free!
  if (gpPaletteSubRanges != NULL) {
    MemFree(gpPaletteSubRanges);
    gpPaletteSubRanges = NULL;
  }

  if (gubpNumReplacementsPerRange != NULL) {
    MemFree(gubpNumReplacementsPerRange);
    gubpNumReplacementsPerRange = NULL;
  }

  for (cnt = 0; cnt < guiNumReplacements; cnt++) {
    // Free
    if (gpPalRep[cnt].r != NULL) {
      MemFree(gpPalRep[cnt].r);
      gpPalRep[cnt].r = NULL;
    }
    if (gpPalRep[cnt].g != NULL) {
      MemFree(gpPalRep[cnt].g);
      gpPalRep[cnt].g = NULL;
    }
    if (gpPalRep[cnt].b != NULL) {
      MemFree(gpPalRep[cnt].b);
      gpPalRep[cnt].b = NULL;
    }
  }

  // Free
  if (gpPalRep != NULL) {
    MemFree(gpPalRep);
    gpPalRep = NULL;
  }

  return (TRUE);
}

BOOLEAN GetPaletteRepIndexFromID(PaletteRepID aPalRep, uint8_t *pubPalIndex) {
  uint32_t cnt;

  // Check if type exists
  for (cnt = 0; cnt < guiNumReplacements; cnt++) {
    if (COMPARE_PALETTEREP_ID(aPalRep, gpPalRep[cnt].ID)) {
      *pubPalIndex = (uint8_t)cnt;
      return (TRUE);
    }
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Invalid Palette Replacement ID given");
  return (FALSE);
}

uint16_t GetNewSoldierStateFromNewStance(struct SOLDIERTYPE *pSoldier, uint8_t ubDesiredStance) {
  uint16_t usNewState;
  int8_t bCurrentHeight;

  bCurrentHeight = (ubDesiredStance - gAnimControl[pSoldier->usAnimState].ubEndHeight);

  // Now change to appropriate animation

  switch (bCurrentHeight) {
    case ANIM_STAND - ANIM_CROUCH:
      usNewState = KNEEL_UP;
      break;
    case ANIM_CROUCH - ANIM_STAND:
      usNewState = KNEEL_DOWN;
      break;

    case ANIM_STAND - ANIM_PRONE:
      usNewState = PRONE_UP;
      break;
    case ANIM_PRONE - ANIM_STAND:
      usNewState = KNEEL_DOWN;
      break;

    case ANIM_CROUCH - ANIM_PRONE:
      usNewState = PRONE_UP;
      break;
    case ANIM_PRONE - ANIM_CROUCH:
      usNewState = PRONE_DOWN;
      break;

    default:

      // Cannot get here unless ub desired stance is bogus
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("GetNewSoldierStateFromNewStance bogus ubDesiredStance value %d",
                      ubDesiredStance));
      usNewState = pSoldier->usAnimState;
  }

  return (usNewState);
}

void MoveMercFacingDirection(struct SOLDIERTYPE *pSoldier, BOOLEAN fReverse, float dMovementDist) {
  float dAngle = (float)0;

  // Determine which direction we are in
  switch (pSoldier->bDirection) {
    case NORTH:
      dAngle = (float)(-1 * PI);
      break;

    case NORTHEAST:
      dAngle = (float)(PI * .75);
      break;

    case EAST:
      dAngle = (float)(PI / 2);
      break;

    case SOUTHEAST:
      dAngle = (float)(PI / 4);
      break;

    case SOUTH:
      dAngle = (float)0;
      break;

    case SOUTHWEST:
      // dAngle = (float)(  PI * -.25 );
      dAngle = (float)-0.786;
      break;

    case WEST:
      dAngle = (float)(PI * -.5);
      break;

    case NORTHWEST:
      dAngle = (float)(PI * -.75);
      break;
  }

  if (fReverse) {
    dMovementDist = dMovementDist * -1;
  }

  MoveMerc(pSoldier, dMovementDist, dAngle, FALSE);
}

void BeginSoldierClimbUpRoof(struct SOLDIERTYPE *pSoldier) {
  int8_t bNewDirection;

  if (FindHeigherLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection) &&
      (pSoldier->bLevel == 0)) {
    if (EnoughPoints(pSoldier, GetAPsToClimbRoof(pSoldier, FALSE), 0, TRUE)) {
      if (pSoldier->bTeam == gbPlayerNum) {
        // OK, SET INTERFACE FIRST
        SetUIBusy(pSoldier->ubID);
      }

      pSoldier->sTempNewGridNo =
          NewGridNo((uint16_t)pSoldier->sGridNo, (uint16_t)DirectionInc(bNewDirection));

      pSoldier->ubPendingDirection = bNewDirection;
      // pSoldier->usPendingAnimation = CLIMBUPROOF;
      EVENT_InitNewSoldierAnim(pSoldier, CLIMBUPROOF, 0, FALSE);

      InternalReceivingSoldierCancelServices(pSoldier, FALSE);
      InternalGivingSoldierCancelServices(pSoldier, FALSE);
    }
  }
}

void BeginSoldierClimbFence(struct SOLDIERTYPE *pSoldier) {
  int8_t bDirection;

  if (FindFenceJumpDirection(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bDirection)) {
    pSoldier->sTempNewGridNo =
        NewGridNo((uint16_t)pSoldier->sGridNo, (uint16_t)DirectionInc(bDirection));
    pSoldier->fDontChargeTurningAPs = TRUE;
    EVENT_InternalSetSoldierDesiredDirection(pSoldier, bDirection, FALSE, pSoldier->usAnimState);
    pSoldier->fTurningUntilDone = TRUE;
    // ATE: Reset flag to go back to prone...
    pSoldier->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
    pSoldier->usPendingAnimation = HOPFENCE;
  }
}

uint32_t SleepDartSuccumbChance(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiChance;
  int8_t bEffectiveStrength;

  // figure out base chance of succumbing,
  bEffectiveStrength = EffectiveStrength(pSoldier);

  if (bEffectiveStrength > 90) {
    uiChance = 110 - bEffectiveStrength;
  } else if (bEffectiveStrength > 80) {
    uiChance = 120 - bEffectiveStrength;
  } else if (bEffectiveStrength > 70) {
    uiChance = 130 - bEffectiveStrength;
  } else {
    uiChance = 140 - bEffectiveStrength;
  }

  // add in a bonus based on how long it's been since shot... highest chance at the beginning
  uiChance += (10 - pSoldier->bSleepDrugCounter);

  return (uiChance);
}

void BeginSoldierGetup(struct SOLDIERTYPE *pSoldier) {
  // RETURN IF WE ARE BEING SERVICED
  if (pSoldier->ubServiceCount > 0) {
    return;
  }

  // ATE: Don't getup if we are in a meanwhile
  if (AreInMeanwhile()) {
    return;
  }

  if (pSoldier->bCollapsed) {
    if (pSoldier->bLife >= OKLIFE && pSoldier->bBreath >= OKBREATH &&
        (pSoldier->bSleepDrugCounter == 0)) {
      // get up you hoser!

      pSoldier->bCollapsed = FALSE;
      pSoldier->bTurnsCollapsed = 0;

      if (IS_MERC_BODY_TYPE(pSoldier)) {
        switch (pSoldier->usAnimState) {
          case FALLOFF_FORWARD_STOP:
          case PRONE_LAYFROMHIT_STOP:
          case STAND_FALLFORWARD_STOP:
            ChangeSoldierStance(pSoldier, ANIM_CROUCH);
            break;

          case FALLBACKHIT_STOP:
          case FALLOFF_STOP:
          case FLYBACKHIT_STOP:
          case FALLBACK_HIT_STAND:
          case FALLOFF:
          case FLYBACK_HIT:

            // ROLL OVER
            EVENT_InitNewSoldierAnim(pSoldier, ROLLOVER, 0, FALSE);
            break;

          default:

            ChangeSoldierStance(pSoldier, ANIM_CROUCH);
            break;
        }
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, END_COWER, 0, FALSE);
      }
    } else {
      pSoldier->bTurnsCollapsed++;
      if ((gTacticalStatus.bBoxingState == BOXING) && (pSoldier->uiStatusFlags & SOLDIER_BOXER)) {
        if (pSoldier->bTurnsCollapsed > 1) {
          // We have a winnah!  But it isn't this boxer!
          EndBoxingMatch(pSoldier);
        }
      }
    }
  } else if (pSoldier->bSleepDrugCounter > 0) {
    uint32_t uiChance;

    uiChance = SleepDartSuccumbChance(pSoldier);

    if (PreRandom(100) < uiChance) {
      // succumb to the drug!
      DeductPoints(pSoldier, 0, (int16_t)(pSoldier->bBreathMax * 100));
      SoldierCollapse(pSoldier);
    }
  }

  if (pSoldier->bSleepDrugCounter > 0) {
    pSoldier->bSleepDrugCounter--;
  }
}

void HandleTakeDamageDeath(struct SOLDIERTYPE *pSoldier, uint8_t bOldLife, uint8_t ubReason) {
  switch (ubReason) {
    case TAKE_DAMAGE_BLOODLOSS:
    case TAKE_DAMAGE_ELECTRICITY:
    case TAKE_DAMAGE_GAS:

      if (IsSolInSector(pSoldier)) {
        if (pSoldier->bVisible != -1) {
          if (ubReason != TAKE_DAMAGE_BLOODLOSS) {
            DoMercBattleSound(pSoldier, BATTLE_SOUND_DIE1);
            pSoldier->fDeadSoundPlayed = TRUE;
          }
        }

        if ((ubReason == TAKE_DAMAGE_ELECTRICITY) && pSoldier->bLife < OKLIFE) {
          pSoldier->fInNonintAnim = FALSE;
        }

        // Check for < OKLIFE
        if (pSoldier->bLife < OKLIFE && pSoldier->bLife != 0 && !pSoldier->bCollapsed) {
          SoldierCollapse(pSoldier);
        }

        // THis is for the die animation that will be happening....
        if (pSoldier->bLife == 0) {
          pSoldier->fDoingExternalDeath = TRUE;
        }

        // Check if he is dead....
        CheckForAndHandleSoldierDyingNotFromHit(pSoldier);
      }

      {
        HandleSoldierTakeDamageFeedback(pSoldier);
      }

      if ((IsMapScreen()) || !pSoldier->bInSector) {
        if (pSoldier->bLife == 0 && !(pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
          StrategicHandlePlayerTeamMercDeath(pSoldier);

          // ATE: Here, force always to use die sound...
          pSoldier->fDieSoundUsed = FALSE;
          DoMercBattleSound(pSoldier, BATTLE_SOUND_DIE1);
          pSoldier->fDeadSoundPlayed = TRUE;

          // ATE: DO death sound
          PlayJA2Sample((uint8_t)DOORCR_1, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
          PlayJA2Sample((uint8_t)HEADCR_1, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
        }
      }
      break;
  }

  if (ubReason == TAKE_DAMAGE_ELECTRICITY) {
    if (pSoldier->bLife >= OKLIFE) {
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Freeing up attacker from electricity damage"));
      ReleaseSoldiersAttacker(pSoldier);
    }
  }
}

uint8_t SoldierTakeDamage(struct SOLDIERTYPE *pSoldier, int8_t bHeight, int16_t sLifeDeduct,
                          int16_t sBreathLoss, uint8_t ubReason, uint8_t ubAttacker,
                          int16_t sSourceGrid, int16_t sSubsequent, BOOLEAN fShowDamage) {
  int8_t bOldLife;
  uint8_t ubCombinedLoss;
  int8_t bBandage;
  int16_t sAPCost;
  uint8_t ubBlood;

  pSoldier->ubLastDamageReason = ubReason;

  // CJC Jan 21 99: add check to see if we are hurting an enemy in an enemy-controlled
  // sector; if so, this is a sign of player activity
  switch (pSoldier->bTeam) {
    case ENEMY_TEAM:
      // if we're in the wilderness this always counts
      if (StrategicMap[GetSectorID16((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
              .fEnemyControlled ||
          SectorInfo[GetSectorID8((uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY)]
                  .ubTraversability[THROUGH_STRATEGIC_MOVE] != TOWN) {
        // update current day of activity!
        UpdateLastDayOfPlayerActivity((uint16_t)GetWorldDay());
      }
      break;
    case CREATURE_TEAM:
      // always a sign of activity?
      UpdateLastDayOfPlayerActivity((uint16_t)GetWorldDay());
      break;
    case CIV_TEAM:
      if (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP &&
          gubQuest[QUEST_RESCUE_MARIA] == QUESTINPROGRESS &&
          gTacticalStatus.bBoxingState == NOT_BOXING) {
        struct SOLDIERTYPE *pMaria = FindSoldierByProfileID(MARIA, FALSE);
        if (pMaria && pMaria->bActive && pMaria->bInSector) {
          SetFactTrue(FACT_MARIA_ESCAPE_NOTICED);
        }
      }
      break;
    default:
      break;
  }

  // Deduct life!, Show damage if we want!
  bOldLife = pSoldier->bLife;

  // OK, If we are a vehicle.... damage vehicle...( people inside... )
  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    if (TANK(pSoldier)) {
      // sLifeDeduct = (sLifeDeduct * 2) / 3;
    } else {
      if (ubReason == TAKE_DAMAGE_GUNFIRE) {
        sLifeDeduct /= 3;
      } else if (ubReason == TAKE_DAMAGE_EXPLOSION && sLifeDeduct > 50) {
        // boom!
        sLifeDeduct *= 2;
      }
    }

    VehicleTakeDamage(pSoldier->bVehicleID, ubReason, sLifeDeduct, pSoldier->sGridNo, ubAttacker);
    HandleTakeDamageDeath(pSoldier, bOldLife, ubReason);
    return (0);
  }

  // ATE: If we are elloit being attacked in a meanwhile...
  if (pSoldier->uiStatusFlags & SOLDIER_NPC_SHOOTING) {
    // Almost kill but not quite.....
    sLifeDeduct = (pSoldier->bLife - 1);
    // Turn off
    pSoldier->uiStatusFlags &= (~SOLDIER_NPC_SHOOTING);
  }

  // CJC: make sure Elliot doesn't bleed to death!
  if (ubReason == TAKE_DAMAGE_BLOODLOSS && AreInMeanwhile()) {
    return (0);
  }

  // Calculate bandage
  bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

  if (IsMapScreen_2()) {
    fReDrawFace = TRUE;
  }

  if (CREATURE_OR_BLOODCAT(pSoldier)) {
    int16_t sReductionFactor = 0;

    if (pSoldier->ubBodyType == BLOODCAT) {
      sReductionFactor = 2;
    } else if (pSoldier->uiStatusFlags & SOLDIER_MONSTER) {
      switch (pSoldier->ubBodyType) {
        case LARVAE_MONSTER:
        case INFANT_MONSTER:
          sReductionFactor = 1;
          break;
        case YAF_MONSTER:
        case YAM_MONSTER:
          sReductionFactor = 4;
          break;
        case ADULTFEMALEMONSTER:
        case AM_MONSTER:
          sReductionFactor = 6;
          break;
        case QUEENMONSTER:
          // increase with range!
          if (ubAttacker == NOBODY) {
            sReductionFactor = 8;
          } else {
            sReductionFactor =
                4 + PythSpacesAway(MercPtrs[ubAttacker]->sGridNo, pSoldier->sGridNo) / 2;
          }
          break;
      }
    }

    if (ubReason == TAKE_DAMAGE_EXPLOSION) {
      sReductionFactor /= 4;
    }
    if (sReductionFactor > 1) {
      sLifeDeduct = (sLifeDeduct + (sReductionFactor / 2)) / sReductionFactor;
    } else if (ubReason == TAKE_DAMAGE_EXPLOSION) {
      // take at most 2/3rds
      sLifeDeduct = (sLifeDeduct * 2) / 3;
    }

    // reduce breath loss to a smaller degree, except for the queen...
    if (pSoldier->ubBodyType == QUEENMONSTER) {
      // in fact, reduce breath loss by MORE!
      sReductionFactor = min(sReductionFactor, 8);
      sReductionFactor *= 2;
    } else {
      sReductionFactor /= 2;
    }
    if (sReductionFactor > 1) {
      sBreathLoss = (sBreathLoss + (sReductionFactor / 2)) / sReductionFactor;
    }
  }

  if (sLifeDeduct > pSoldier->bLife) {
    pSoldier->bLife = 0;
  } else {
    // Decrease Health
    pSoldier->bLife -= sLifeDeduct;
  }

  // ATE: Put some logic in here to allow enemies to die quicker.....
  // Are we an enemy?
  if (pSoldier->bSide != gbPlayerNum && !pSoldier->bNeutral &&
      GetSolProfile(pSoldier) == NO_PROFILE) {
    // ATE: Give them a chance to fall down...
    if (pSoldier->bLife > 0 && pSoldier->bLife < (OKLIFE - 1)) {
      // Are we taking damage from bleeding?
      if (ubReason == TAKE_DAMAGE_BLOODLOSS) {
        // Fifty-fifty chance to die now!
        if (Random(2) == 0 || gTacticalStatus.Team[pSoldier->bTeam].bMenInSector == 1) {
          // Kill!
          pSoldier->bLife = 0;
        }
      } else {
        // OK, see how far we are..
        if (pSoldier->bLife < (OKLIFE - 3)) {
          // Kill!
          pSoldier->bLife = 0;
        }
      }
    }
  }

  if (fShowDamage) {
    pSoldier->sDamage += sLifeDeduct;
  }

  // Truncate life
  if (pSoldier->bLife < 0) {
    pSoldier->bLife = 0;
  }

  // Calculate damage to our items if from an explosion!
  if (ubReason == TAKE_DAMAGE_EXPLOSION || ubReason == TAKE_DAMAGE_STRUCTURE_EXPLOSION) {
    CheckEquipmentForDamage(pSoldier, sLifeDeduct);
  }

  // Calculate bleeding
  if (ubReason != TAKE_DAMAGE_GAS && !AM_A_ROBOT(pSoldier)) {
    if (ubReason == TAKE_DAMAGE_HANDTOHAND) {
      if (sLifeDeduct > 0) {
        // HTH does 1 pt bleeding per hit
        pSoldier->bBleeding = pSoldier->bBleeding + 1;
      }
    } else {
      pSoldier->bBleeding = pSoldier->bLifeMax - (pSoldier->bLife + bBandage);
    }
  }

  // Deduct breath AND APs!
  sAPCost = (sLifeDeduct / AP_GET_WOUNDED_DIVISOR);  // + fallCost;

  // ATE: if the robot, do not deduct
  if (!AM_A_ROBOT(pSoldier)) {
    DeductPoints(pSoldier, sAPCost, sBreathLoss);
  }

  ubCombinedLoss = (uint8_t)sLifeDeduct / 10 + sBreathLoss / 2000;

  // Add shock
  if (!AM_A_ROBOT(pSoldier)) {
    pSoldier->bShock += ubCombinedLoss;
  }

  // start the stopwatch - the blood is gushing!
  pSoldier->dNextBleed = CalcSoldierNextBleed(pSoldier);

  if (pSoldier->bInSector && pSoldier->bVisible != -1) {
    // If we are already dead, don't show damage!
    if (bOldLife != 0 && fShowDamage && sLifeDeduct != 0 && sLifeDeduct < 1000) {
      // Display damage
      int16_t sOffsetX, sOffsetY;

      // Set Damage display counter
      pSoldier->fDisplayDamage = TRUE;
      pSoldier->bDisplayDamageCount = 0;

      if (pSoldier->ubBodyType == QUEENMONSTER) {
        pSoldier->sDamageX = 0;
        pSoldier->sDamageY = 0;
      } else {
        GetSoldierAnimOffsets(pSoldier, &sOffsetX, &sOffsetY);
        pSoldier->sDamageX = sOffsetX;
        pSoldier->sDamageY = sOffsetY;
      }
    }
  }

  // OK, if here, let's see if we should drop our weapon....
  if (ubReason != TAKE_DAMAGE_BLOODLOSS && !(AM_A_ROBOT(pSoldier))) {
    int16_t sTestOne, sTestTwo, sChanceToDrop;
    int8_t bVisible = -1;

    sTestOne = EffectiveStrength(pSoldier);
    sTestTwo = (2 * (max(sLifeDeduct, (sBreathLoss / 100))));

    if (pSoldier->ubAttackerID != NOBODY &&
        MercPtrs[pSoldier->ubAttackerID]->ubBodyType == BLOODCAT) {
      // bloodcat boost, let them make people drop items more
      sTestTwo += 20;
    }

    // If damage > effective strength....
    sChanceToDrop = (max(0, (sTestTwo - sTestOne)));

    // ATE: Increase odds of NOT dropping an UNDROPPABLE OBJECT
    if ((pSoldier->inv[HANDPOS].fFlags & OBJECT_UNDROPPABLE)) {
      sChanceToDrop -= 30;
    }

#ifdef JA2TESTVERSION
    // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Chance To Drop Weapon: str: %d Dam: %d
    // Chance: %d", sTestOne, sTestTwo, sChanceToDrop );
#endif

    if (Random(100) < (uint16_t)sChanceToDrop) {
      // OK, drop item in main hand...
      if (pSoldier->inv[HANDPOS].usItem != NOTHING) {
        if (!(pSoldier->inv[HANDPOS].fFlags & OBJECT_UNDROPPABLE)) {
          // ATE: if our guy, make visible....
          if (pSoldier->bTeam == gbPlayerNum) {
            bVisible = 1;
          }

          AddItemToPool(pSoldier->sGridNo, &(pSoldier->inv[HANDPOS]), bVisible, pSoldier->bLevel, 0,
                        -1);
          DeleteObj(&(pSoldier->inv[HANDPOS]));
        }
      }
    }
  }

  // Drop some blood!
  // decide blood amt, if any
  ubBlood = (sLifeDeduct / BLOODDIVISOR);
  if (ubBlood > MAXBLOODQUANTITY) {
    ubBlood = MAXBLOODQUANTITY;
  }

  if (!(pSoldier->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ROBOT))) {
    if (ubBlood != 0) {
      if (IsSolInSector(pSoldier)) {
        DropBlood(pSoldier, ubBlood, pSoldier->bVisible);
      }
    }
  }

  // Set UI Flag for unconscious, if it's our own guy!
  if (pSoldier->bTeam == gbPlayerNum) {
    if (pSoldier->bLife < OKLIFE && pSoldier->bLife > 0 && bOldLife >= OKLIFE) {
      pSoldier->fUIFirstTimeUNCON = TRUE;
      fInterfacePanelDirty = DIRTYLEVEL2;
    }
  }

  if (IsSolInSector(pSoldier)) {
    CheckForBreathCollapse(pSoldier);
  }

  // EXPERIENCE CLASS GAIN (combLoss): Getting wounded in battle

  DirtyMercPanelInterface(pSoldier, DIRTYLEVEL1);

  if (ubAttacker != NOBODY) {
    // don't give exp for hitting friends!
    if ((MercPtrs[ubAttacker]->bTeam == gbPlayerNum) && (pSoldier->bTeam != gbPlayerNum)) {
      if (ubReason == TAKE_DAMAGE_EXPLOSION) {
        // EXPLOSIVES GAIN (combLoss):  Causing wounds in battle
        StatChange(MercPtrs[ubAttacker], EXPLODEAMT, (uint16_t)(10 * ubCombinedLoss), FROM_FAILURE);
      }
      /*
      else if ( ubReason == TAKE_DAMAGE_GUNFIRE )
      {
              // MARKSMANSHIP GAIN (combLoss):  Causing wounds in battle
              StatChange( MercPtrs[ ubAttacker ], MARKAMT, (uint16_t)( 5 * ubCombinedLoss ), FALSE
      );
      }
      */
    }
  }

  if (PTR_OURTEAM) {
    // EXPERIENCE GAIN: Took some damage
    StatChange(pSoldier, EXPERAMT, (uint16_t)(5 * ubCombinedLoss), FROM_FAILURE);

    // Check for quote
    if (!(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_BEING_PUMMELED)) {
      // Check attacker!
      if (ubAttacker != NOBODY && ubAttacker != GetSolID(pSoldier)) {
        pSoldier->bNumHitsThisTurn++;

        if ((pSoldier->bNumHitsThisTurn >= 3) && (pSoldier->bLife - pSoldier->bOldLife > 20)) {
          if (Random(100) < (uint16_t)((40 * (pSoldier->bNumHitsThisTurn - 2)))) {
            DelayedTacticalCharacterDialogue(pSoldier, QUOTE_TAKEN_A_BREATING);
            pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_BEING_PUMMELED;
            pSoldier->bNumHitsThisTurn = 0;
          }
        }
      }
    }
  }

  if ((ubAttacker != NOBODY) && (Menptr[ubAttacker].bTeam == OUR_TEAM) &&
      (GetSolProfile(pSoldier) != NO_PROFILE) && (GetSolProfile(pSoldier) >= FIRST_RPC)) {
    gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags |= PROFILE_MISC_FLAG_WOUNDEDBYPLAYER;
    if (GetSolProfile(pSoldier) == 114) {
      SetFactTrue(FACT_PACOS_KILLED);
    }
  }

  HandleTakeDamageDeath(pSoldier, bOldLife, ubReason);

  // Check if we are < unconscious, and shutup if so! also wipe sight
  if (pSoldier->bLife < CONSCIOUSNESS) {
    ShutupaYoFace(pSoldier->iFaceIndex);
  }

  if (pSoldier->bLife < OKLIFE) {
    DecayIndividualOpplist(pSoldier);
  }

  return (ubCombinedLoss);
}

extern BOOLEAN IsMercSayingDialogue(uint8_t ubProfileID);

BOOLEAN InternalDoMercBattleSound(struct SOLDIERTYPE *pSoldier, uint8_t ubBattleSoundID,
                                  int8_t bSpecialCode) {
  SGPFILENAME zFilename;
  SOUNDPARMS spParms;
  uint8_t ubSoundID;
  uint32_t uiSoundID;
  uint32_t iFaceIndex;
  BOOLEAN fDoSub = FALSE;
  int32_t uiSubSoundID = 0;

  // DOUBLECHECK RANGE
  CHECKF(ubBattleSoundID < NUM_MERC_BATTLE_SOUNDS);

  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    // Pick a passenger from vehicle....
    pSoldier = PickRandomPassengerFromVehicle(pSoldier);

    if (pSoldier == NULL) {
      return (FALSE);
    }
  }

  // If a death sound, and we have already done ours...
  if (ubBattleSoundID == BATTLE_SOUND_DIE1) {
    if (pSoldier->fDieSoundUsed) {
      return (TRUE);
    }
  }

  // Are we mute?
  if (pSoldier->uiStatusFlags & SOLDIER_MUTE) {
    return (FALSE);
  }

  //	uiTimeSameBattleSndDone

  // If we are a creature, etc, pick a better sound...
  if (ubBattleSoundID == BATTLE_SOUND_HIT1 || ubBattleSoundID == BATTLE_SOUND_HIT2) {
    switch (pSoldier->ubBodyType) {
      case COW:

        fDoSub = TRUE;
        uiSubSoundID = COW_HIT_SND;
        break;

      case YAF_MONSTER:
      case YAM_MONSTER:
      case ADULTFEMALEMONSTER:
      case AM_MONSTER:

        fDoSub = TRUE;

        if (Random(2) == 0) {
          uiSubSoundID = ACR_DIE_PART1;
        } else {
          uiSubSoundID = ACR_LUNGE;
        }
        break;

      case INFANT_MONSTER:

        fDoSub = TRUE;
        uiSubSoundID = BCR_SHRIEK;
        break;

      case QUEENMONSTER:

        fDoSub = TRUE;
        uiSubSoundID = LQ_SHRIEK;
        break;

      case LARVAE_MONSTER:

        fDoSub = TRUE;
        uiSubSoundID = BCR_SHRIEK;
        break;

      case BLOODCAT:

        fDoSub = TRUE;
        uiSubSoundID = BLOODCAT_HIT_1;
        break;

      case ROBOTNOWEAPON:

        fDoSub = TRUE;
        uiSubSoundID = (uint32_t)(S_METAL_IMPACT1 + Random(2));
        break;
    }
  }

  if (ubBattleSoundID == BATTLE_SOUND_DIE1) {
    switch (pSoldier->ubBodyType) {
      case COW:

        fDoSub = TRUE;
        uiSubSoundID = COW_DIE_SND;
        break;

      case YAF_MONSTER:
      case YAM_MONSTER:
      case ADULTFEMALEMONSTER:
      case AM_MONSTER:

        fDoSub = TRUE;
        uiSubSoundID = CREATURE_FALL_PART_2;
        break;

      case INFANT_MONSTER:

        fDoSub = TRUE;
        uiSubSoundID = BCR_DYING;
        break;

      case LARVAE_MONSTER:

        fDoSub = TRUE;
        uiSubSoundID = LCR_RUPTURE;
        break;

      case QUEENMONSTER:

        fDoSub = TRUE;
        uiSubSoundID = LQ_DYING;
        break;

      case BLOODCAT:

        fDoSub = TRUE;
        uiSubSoundID = BLOODCAT_DIE_1;
        break;

      case ROBOTNOWEAPON:

        fDoSub = TRUE;
        uiSubSoundID = (uint32_t)(EXPLOSION_1);
        PlayJA2Sample(ROBOT_DEATH, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
        break;
    }
  }

  // OK. any other sound, not hits, robot makes a beep
  if (pSoldier->ubBodyType == ROBOTNOWEAPON && !fDoSub) {
    fDoSub = TRUE;
    if (ubBattleSoundID == BATTLE_SOUND_ATTN1) {
      uiSubSoundID = ROBOT_GREETING;
    } else {
      uiSubSoundID = ROBOT_BEEP;
    }
  }

  if (fDoSub) {
    if (guiCurrentScreen != GAME_SCREEN) {
      PlayJA2Sample(uiSubSoundID, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
    } else {
      PlayJA2Sample(uiSubSoundID, RATE_11025,
                    SoundVolume((uint8_t)CalculateSpeechVolume(HIGHVOLUME), pSoldier->sGridNo), 1,
                    SoundDir(pSoldier->sGridNo));
    }
    return (TRUE);
  }

  // Check if this is the same one we just played...
  if (pSoldier->bOldBattleSnd == ubBattleSoundID &&
      gBattleSndsData[ubBattleSoundID].fDontAllowTwoInRow) {
    // Are we below the min delay?
    if ((GetJA2Clock() - pSoldier->uiTimeSameBattleSndDone) < MIN_SUBSEQUENT_SNDS_DELAY) {
      return (TRUE);
    }
  }

  // If a battle snd is STILL playing....
  if (SoundIsPlaying(pSoldier->uiBattleSoundID)) {
    // We can do a few things here....
    // Is this a crutial one...?
    if (gBattleSndsData[ubBattleSoundID].fStopDialogue == 1) {
      // Stop playing origonal
      SoundStop(pSoldier->uiBattleSoundID);
    } else {
      // Skip this one...
      return (TRUE);
    }
  }

  // If we are talking now....
  if (IsMercSayingDialogue(GetSolProfile(pSoldier))) {
    // We can do a couple of things now...
    if (gBattleSndsData[ubBattleSoundID].fStopDialogue == 1) {
      // Stop dialigue...
      DialogueAdvanceSpeech();
    } else if (gBattleSndsData[ubBattleSoundID].fStopDialogue == 2) {
      // Skip battle snd...
      return (TRUE);
    }
  }

  // Save this one we're doing...
  pSoldier->bOldBattleSnd = ubBattleSoundID;
  pSoldier->uiTimeSameBattleSndDone = GetJA2Clock();

  // Adjust based on morale...
  if (ubBattleSoundID == BATTLE_SOUND_OK1 && pSoldier->bMorale < LOW_MORALE_BATTLE_SND_THREASHOLD) {
    ubBattleSoundID = BATTLE_SOUND_LOWMARALE_OK1;
  }
  if (ubBattleSoundID == BATTLE_SOUND_ATTN1 &&
      pSoldier->bMorale < LOW_MORALE_BATTLE_SND_THREASHOLD) {
    ubBattleSoundID = BATTLE_SOUND_LOWMARALE_ATTN1;
  }

  ubSoundID = ubBattleSoundID;

  // if the sound to be played is a confirmation, check to see if we are to play it
  if (ubSoundID == BATTLE_SOUND_OK1) {
    if (gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS]) return (TRUE);
    // else a speech sound is to be played
  }

  // Randomize between sounds, if appropriate
  if (gBattleSndsData[ubSoundID].ubRandomVal != 0) {
    ubSoundID = ubSoundID + (uint8_t)Random(gBattleSndsData[ubSoundID].ubRandomVal);
  }

  // OK, build file and play!
  if (GetSolProfile(pSoldier) != NO_PROFILE) {
    sprintf(zFilename, "BATTLESNDS\\%03d_%s.wav", GetSolProfile(pSoldier),
            gBattleSndsData[ubSoundID].zName);

    if (!FileMan_Exists(zFilename)) {
      // OK, temp build file...
      if (pSoldier->ubBodyType == REGFEMALE) {
        sprintf(zFilename, "BATTLESNDS\\f_%s.wav", gBattleSndsData[ubSoundID].zName);
      } else {
        sprintf(zFilename, "BATTLESNDS\\m_%s.wav", gBattleSndsData[ubSoundID].zName);
      }
    }
  } else {
    // Check if we can play this!
    if (!gBattleSndsData[ubSoundID].fBadGuy) {
      return (FALSE);
    }

    if (pSoldier->ubBodyType == HATKIDCIV || pSoldier->ubBodyType == KIDCIV) {
      if (ubSoundID == BATTLE_SOUND_DIE1) {
        sprintf(zFilename, "BATTLESNDS\\kid%d_dying.wav", pSoldier->ubBattleSoundID);
      } else {
        sprintf(zFilename, "BATTLESNDS\\kid%d_%s.wav", pSoldier->ubBattleSoundID,
                gBattleSndsData[ubSoundID].zName);
      }
    } else {
      if (ubSoundID == BATTLE_SOUND_DIE1) {
        sprintf(zFilename, "BATTLESNDS\\bad%d_die.wav", pSoldier->ubBattleSoundID);
      } else {
        sprintf(zFilename, "BATTLESNDS\\bad%d_%s.wav", pSoldier->ubBattleSoundID,
                gBattleSndsData[ubSoundID].zName);
      }
    }
  }

  // Play sound!
  memset(&spParms, 0xff, sizeof(SOUNDPARMS));

  spParms.uiSpeed = RATE_11025;
  // spParms.uiVolume = CalculateSpeechVolume( pSoldier->bVocalVolume );

  spParms.uiVolume = (int8_t)CalculateSpeechVolume(HIGHVOLUME);

  // ATE: Reduce volume for OK sounds...
  // ( Only for all-moves or multi-selection cases... )
  if (bSpecialCode == BATTLE_SND_LOWER_VOLUME) {
    spParms.uiVolume = (int8_t)CalculateSpeechVolume(MIDVOLUME);
  }

  // If we are an enemy.....reduce due to volume
  if (pSoldier->bTeam != gbPlayerNum) {
    spParms.uiVolume = SoundVolume((uint8_t)spParms.uiVolume, pSoldier->sGridNo);
  }

  spParms.uiLoop = 1;
  spParms.uiPan = SoundDir(pSoldier->sGridNo);
  spParms.uiPriority = GROUP_PLAYER;

  if ((uiSoundID = SoundPlay(zFilename, &spParms)) == SOUND_ERROR) {
    return (FALSE);
  } else {
    pSoldier->uiBattleSoundID = uiSoundID;

    if (GetSolProfile(pSoldier) != NO_PROFILE) {
      // Get soldier's face ID
      iFaceIndex = pSoldier->iFaceIndex;

      // Check face index
      if (iFaceIndex != -1) {
        ExternSetFaceTalking(iFaceIndex, uiSoundID);
      }
    }

    return (TRUE);
  }
}

BOOLEAN DoMercBattleSound(struct SOLDIERTYPE *pSoldier, uint8_t ubBattleSoundID) {
  // We WANT to play some RIGHT AWAY.....
  if (gBattleSndsData[ubBattleSoundID].fStopDialogue == 1 ||
      (GetSolProfile(pSoldier) == NO_PROFILE) || InOverheadMap()) {
    return (InternalDoMercBattleSound(pSoldier, ubBattleSoundID, 0));
  }

  // So here, only if we were currently saying dialogue.....
  if (!IsMercSayingDialogue(GetSolProfile(pSoldier))) {
    return (InternalDoMercBattleSound(pSoldier, ubBattleSoundID, 0));
  }

  // OK, queue it up otherwise!
  TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0, DIALOGUE_SPECIAL_EVENT_DO_BATTLE_SND,
                                            ubBattleSoundID, 0);

  return (TRUE);
}

BOOLEAN PreloadSoldierBattleSounds(struct SOLDIERTYPE *pSoldier, BOOLEAN fRemove) {
  uint32_t cnt;

  CHECKF(IsSolActive(pSoldier) != FALSE);

  for (cnt = 0; cnt < NUM_MERC_BATTLE_SOUNDS; cnt++) {
    // OK, build file and play!
    if (GetSolProfile(pSoldier) != NO_PROFILE) {
      if (gBattleSndsData[cnt].fPreload) {
        if (fRemove) {
          SoundUnlockSample(gBattleSndsData[cnt].zName);
        } else {
          SoundLockSample(gBattleSndsData[cnt].zName);
        }
      }
    } else {
      if (gBattleSndsData[cnt].fPreload && gBattleSndsData[cnt].fBadGuy) {
        if (fRemove) {
          SoundUnlockSample(gBattleSndsData[cnt].zName);
        } else {
          SoundLockSample(gBattleSndsData[cnt].zName);
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN CheckSoldierHitRoof(struct SOLDIERTYPE *pSoldier) {
  // Check if we are near a lower level
  int8_t bNewDirection;
  BOOLEAN fReturnVal = FALSE;
  int16_t sNewGridNo;
  // Default to true
  BOOLEAN fDoForwards = TRUE;

  if (pSoldier->bLife >= OKLIFE) {
    return (FALSE);
  }

  if (FindLowerLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection) &&
      (pSoldier->bLevel > 0)) {
    // ONly if standing!
    if (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_STAND) {
      // We are near a lower level.
      // Use opposite direction
      bNewDirection = gOppositeDirection[bNewDirection];

      // Alrighty, let's not blindly change here, look at whether the dest gridno is good!
      sNewGridNo =
          NewGridNo((uint16_t)pSoldier->sGridNo, DirectionInc(gOppositeDirection[bNewDirection]));
      if (!NewOKDestination(pSoldier, sNewGridNo, TRUE, 0)) {
        return (FALSE);
      }
      sNewGridNo = NewGridNo((uint16_t)sNewGridNo, DirectionInc(gOppositeDirection[bNewDirection]));
      if (!NewOKDestination(pSoldier, sNewGridNo, TRUE, 0)) {
        return (FALSE);
      }

      // Are wee near enough to fall forwards....
      if (pSoldier->bDirection == gOneCDirection[bNewDirection] ||
          pSoldier->bDirection == gTwoCDirection[bNewDirection] ||
          pSoldier->bDirection == bNewDirection ||
          pSoldier->bDirection == gOneCCDirection[bNewDirection] ||
          pSoldier->bDirection == gTwoCCDirection[bNewDirection]) {
        // Do backwards...
        fDoForwards = FALSE;
      }

      // If we are facing the opposite direction, fall backwards
      // ATE: Make this more usefull...
      if (fDoForwards) {
        pSoldier->sTempNewGridNo =
            NewGridNo((uint16_t)pSoldier->sGridNo, (int16_t)(-1 * DirectionInc(bNewDirection)));
        pSoldier->sTempNewGridNo = NewGridNo((uint16_t)pSoldier->sTempNewGridNo,
                                             (int16_t)(-1 * DirectionInc(bNewDirection)));
        EVENT_SetSoldierDesiredDirection(pSoldier, gOppositeDirection[bNewDirection]);
        pSoldier->fTurningUntilDone = TRUE;
        pSoldier->usPendingAnimation = FALLFORWARD_ROOF;
        // EVENT_InitNewSoldierAnim( pSoldier, FALLFORWARD_ROOF, 0 , FALSE );

        // Deduct hitpoints/breath for falling!
        SoldierTakeDamage(pSoldier, ANIM_CROUCH, 100, 5000, TAKE_DAMAGE_FALLROOF, NOBODY, NOWHERE,
                          0, TRUE);

        fReturnVal = TRUE;

      } else {
        pSoldier->sTempNewGridNo =
            NewGridNo((uint16_t)pSoldier->sGridNo, (int16_t)(-1 * DirectionInc(bNewDirection)));
        pSoldier->sTempNewGridNo = NewGridNo((uint16_t)pSoldier->sTempNewGridNo,
                                             (int16_t)(-1 * DirectionInc(bNewDirection)));
        EVENT_SetSoldierDesiredDirection(pSoldier, bNewDirection);
        pSoldier->fTurningUntilDone = TRUE;
        pSoldier->usPendingAnimation = FALLOFF;

        // Deduct hitpoints/breath for falling!
        SoldierTakeDamage(pSoldier, ANIM_CROUCH, 100, 5000, TAKE_DAMAGE_FALLROOF, NOBODY, NOWHERE,
                          0, TRUE);

        fReturnVal = TRUE;
      }
    }
  }

  return (fReturnVal);
}

void BeginSoldierClimbDownRoof(struct SOLDIERTYPE *pSoldier) {
  int8_t bNewDirection;

  if (FindLowerLevel(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bNewDirection) &&
      (pSoldier->bLevel > 0)) {
    if (EnoughPoints(pSoldier, GetAPsToClimbRoof(pSoldier, TRUE), 0, TRUE)) {
      if (pSoldier->bTeam == gbPlayerNum) {
        // OK, SET INTERFACE FIRST
        SetUIBusy(pSoldier->ubID);
      }

      pSoldier->sTempNewGridNo =
          NewGridNo((uint16_t)pSoldier->sGridNo, (uint16_t)DirectionInc(bNewDirection));

      bNewDirection = gTwoCDirection[bNewDirection];

      pSoldier->ubPendingDirection = bNewDirection;
      EVENT_InitNewSoldierAnim(pSoldier, CLIMBDOWNROOF, 0, FALSE);

      InternalReceivingSoldierCancelServices(pSoldier, FALSE);
      InternalGivingSoldierCancelServices(pSoldier, FALSE);
    }
  }
}

void MoveMerc(struct SOLDIERTYPE *pSoldier, float dMovementChange, float dAngle,
              BOOLEAN fCheckRange) {
  float dDeltaPos;
  float dXPos, dYPos;
  BOOLEAN fStop = FALSE;

  // Find delta Movement for X pos
  dDeltaPos = (float)(dMovementChange * sin(dAngle));

  // Find new position
  dXPos = pSoldier->dXPos + dDeltaPos;

  if (fCheckRange) {
    fStop = FALSE;

    switch (pSoldier->bMovementDirection) {
      case NORTHEAST:
      case EAST:
      case SOUTHEAST:

        if (dXPos >= pSoldier->sDestXPos) {
          fStop = TRUE;
        }
        break;

      case NORTHWEST:
      case WEST:
      case SOUTHWEST:

        if (dXPos <= pSoldier->sDestXPos) {
          fStop = TRUE;
        }
        break;

      case NORTH:
      case SOUTH:

        fStop = TRUE;
        break;
    }

    if (fStop) {
      // dXPos = pSoldier->sDestXPos;
      pSoldier->fPastXDest = TRUE;

      if (pSoldier->sGridNo == pSoldier->sFinalDestination) {
        dXPos = pSoldier->sDestXPos;
      }
    }
  }

  // Find delta Movement for Y pos
  dDeltaPos = (float)(dMovementChange * cos(dAngle));

  // Find new pos
  dYPos = pSoldier->dYPos + dDeltaPos;

  if (fCheckRange) {
    fStop = FALSE;

    switch (pSoldier->bMovementDirection) {
      case NORTH:
      case NORTHEAST:
      case NORTHWEST:

        if (dYPos <= pSoldier->sDestYPos) {
          fStop = TRUE;
        }
        break;

      case SOUTH:
      case SOUTHWEST:
      case SOUTHEAST:

        if (dYPos >= pSoldier->sDestYPos) {
          fStop = TRUE;
        }
        break;

      case EAST:
      case WEST:

        fStop = TRUE;
        break;
    }

    if (fStop) {
      // dYPos = pSoldier->sDestYPos;
      pSoldier->fPastYDest = TRUE;

      if (pSoldier->sGridNo == pSoldier->sFinalDestination) {
        dYPos = pSoldier->sDestYPos;
      }
    }
  }

  // OK, set new position
  EVENT_InternalSetSoldierPosition(pSoldier, dXPos, dYPos, FALSE, FALSE, FALSE);

  //	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("X: %f Y: %f", dXPos, dYPos ) );
}

int16_t GetDirectionFromGridNo(int16_t sGridNo, struct SOLDIERTYPE *pSoldier) {
  int16_t sXPos, sYPos;

  ConvertGridNoToXY(sGridNo, &sXPos, &sYPos);

  return (GetDirectionFromXY(sXPos, sYPos, pSoldier));
}

int16_t GetDirectionToGridNoFromGridNo(int16_t sGridNoDest, int16_t sGridNoSrc) {
  int16_t sXPos2, sYPos2;
  int16_t sXPos, sYPos;

  ConvertGridNoToXY(sGridNoSrc, &sXPos, &sYPos);
  ConvertGridNoToXY(sGridNoDest, &sXPos2, &sYPos2);

  return (atan8(sXPos2, sYPos2, sXPos, sYPos));
}

int16_t GetDirectionFromXY(int16_t sXPos, int16_t sYPos, struct SOLDIERTYPE *pSoldier) {
  int16_t sXPos2, sYPos2;

  ConvertGridNoToXY(pSoldier->sGridNo, &sXPos2, &sYPos2);

  return (atan8(sXPos2, sYPos2, sXPos, sYPos));
}

#if 0
uint8_t  atan8( int16_t x1, int16_t y1, int16_t x2, int16_t y2 )
{
static int trig[8] = { 2, 3, 4, 5, 6, 7, 8, 1 };
// returned values are N=1, NE=2, E=3, SE=4, S=5, SW=6, W=7, NW=8
	double dx=(x2-x1);
	double dy=(y2-y1);
	double a;
	int i,k;
	if (dx==0)
		dx=0.00390625; // 1/256th
#define PISLICES (8)
	a=(atan2(dy,dx) + PI/PISLICES)/(PI/(PISLICES/2));
	i=(int)a;
	if (a>0)
		k=i; else
	if (a<0)
		k=i+(PISLICES-1); else
		k=0;
	return(trig[k]);
}
#endif

// #if 0
uint8_t atan8(int16_t sXPos, int16_t sYPos, int16_t sXPos2, int16_t sYPos2) {
  double test_x = sXPos2 - sXPos;
  double test_y = sYPos2 - sYPos;
  uint8_t mFacing = WEST;
  double angle;

  if (test_x == 0) {
    test_x = 0.04;
  }

  angle = atan2(test_x, test_y);

  do {
    if (angle >= -PI * .375 && angle <= -PI * .125) {
      mFacing = SOUTHWEST;
      break;
    }

    if (angle <= PI * .375 && angle >= PI * .125) {
      mFacing = SOUTHEAST;
      break;
    }

    if (angle >= PI * .623 && angle <= PI * .875) {
      mFacing = NORTHEAST;
      break;
    }

    if (angle <= -PI * .623 && angle >= -PI * .875) {
      mFacing = NORTHWEST;
      break;
    }

    if (angle > -PI * 0.125 && angle < PI * 0.125) {
      mFacing = SOUTH;
    }
    if (angle > PI * 0.375 && angle < PI * 0.623) {
      mFacing = EAST;
    }
    if ((angle > PI * 0.875 && angle <= PI) || (angle > -PI && angle < -PI * 0.875)) {
      mFacing = NORTH;
    }
    if (angle > -PI * 0.623 && angle < -PI * 0.375) {
      mFacing = WEST;
    }

  } while (FALSE);

  return (mFacing);
}

uint8_t atan8FromAngle(double angle) {
  uint8_t mFacing = WEST;

  if (angle > PI) {
    angle = (angle - PI) - PI;
  }
  if (angle < -PI) {
    angle = (PI - (fabs(angle) - PI));
  }

  do {
    if (angle >= -PI * .375 && angle <= -PI * .125) {
      mFacing = SOUTHWEST;
      break;
    }

    if (angle <= PI * .375 && angle >= PI * .125) {
      mFacing = SOUTHEAST;
      break;
    }

    if (angle >= PI * .623 && angle <= PI * .875) {
      mFacing = NORTHEAST;
      break;
    }

    if (angle <= -PI * .623 && angle >= -PI * .875) {
      mFacing = NORTHWEST;
      break;
    }

    if (angle > -PI * 0.125 && angle < PI * 0.125) {
      mFacing = SOUTH;
    }
    if (angle > PI * 0.375 && angle < PI * 0.623) {
      mFacing = EAST;
    }
    if ((angle > PI * 0.875 && angle <= PI) || (angle > -PI && angle < -PI * 0.875)) {
      mFacing = NORTH;
    }
    if (angle > -PI * 0.623 && angle < -PI * 0.375) {
      mFacing = WEST;
    }

  } while (FALSE);

  return (mFacing);
}

void CheckForFullStructures(struct SOLDIERTYPE *pSoldier) {
  // This function checks to see if we are near a specific structure type which requires us to blit
  // a small obscuring peice
  int16_t sGridNo;
  uint16_t usFullTileIndex;
  int32_t cnt;

  // Check in all 'Above' directions
  for (cnt = 0; cnt < MAX_FULLTILE_DIRECTIONS; cnt++) {
    sGridNo = pSoldier->sGridNo + gsFullTileDirections[cnt];

    if (CheckForFullStruct(sGridNo, &usFullTileIndex)) {
      // Add one for the item's obsuring part
      pSoldier->usFrontArcFullTileList[cnt] = usFullTileIndex + 1;
      pSoldier->usFrontArcFullTileGridNos[cnt] = sGridNo;
      AddTopmostToHead(sGridNo, pSoldier->usFrontArcFullTileList[cnt]);
    } else {
      if (pSoldier->usFrontArcFullTileList[cnt] != 0) {
        RemoveTopmost(pSoldier->usFrontArcFullTileGridNos[cnt],
                      pSoldier->usFrontArcFullTileList[cnt]);
      }
      pSoldier->usFrontArcFullTileList[cnt] = 0;
      pSoldier->usFrontArcFullTileGridNos[cnt] = 0;
    }
  }
}

BOOLEAN CheckForFullStruct(int16_t sGridNo, uint16_t *pusIndex) {
  struct LEVELNODE *pStruct = NULL;
  struct LEVELNODE *pOldStruct = NULL;
  uint32_t fTileFlags;

  pStruct = gpWorldLevelData[sGridNo].pStructHead;

  // Look through all structs and Search for type

  while (pStruct != NULL) {
    if (pStruct->usIndex != NO_TILE && pStruct->usIndex < NUMBEROFTILES) {
      GetTileFlags(pStruct->usIndex, &fTileFlags);

      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;

      // if( (pOldStruct->pStructureData!=NULL) && (
      // pOldStruct->pStructureData->fFlags&STRUCTURE_TREE ) )
      if (fTileFlags & FULL3D_TILE) {
        // CHECK IF THIS TREE IS FAIRLY ALONE!
        if (FullStructAlone(sGridNo, 2)) {
          // Return true and return index
          *pusIndex = pOldStruct->usIndex;
          return (TRUE);
        } else {
          return (FALSE);
        }
      }

    } else {
      // Advance to next
      pOldStruct = pStruct;
      pStruct = pStruct->pNext;
    }
  }

  // Could not find it, return FALSE
  return (FALSE);
}

BOOLEAN FullStructAlone(int16_t sGridNo, uint8_t ubRadius) {
  int16_t sTop, sBottom;
  int16_t sLeft, sRight;
  int16_t cnt1, cnt2;
  int16_t iNewIndex;
  int32_t leftmost;

  // Determine start end end indicies and num rows
  sTop = ubRadius;
  sBottom = -ubRadius;
  sLeft = -ubRadius;
  sRight = ubRadius;

  for (cnt1 = sBottom; cnt1 <= sTop; cnt1++) {
    leftmost = ((sGridNo + (WORLD_COLS * cnt1)) / WORLD_COLS) * WORLD_COLS;

    for (cnt2 = sLeft; cnt2 <= sRight; cnt2++) {
      iNewIndex = sGridNo + (WORLD_COLS * cnt1) + cnt2;

      if (iNewIndex >= 0 && iNewIndex < WORLD_MAX && iNewIndex >= leftmost &&
          iNewIndex < (leftmost + WORLD_COLS)) {
        if (iNewIndex != sGridNo) {
          if (FindStructure(iNewIndex, STRUCTURE_TREE) != NULL) {
            return (FALSE);
          }
        }
      }
    }
  }

  return (TRUE);
}

void AdjustForFastTurnAnimation(struct SOLDIERTYPE *pSoldier) {
  // CHECK FOR FASTTURN ANIMATIONS
  // ATE: Mod: Only fastturn for OUR guys!
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FASTTURN &&
      pSoldier->bTeam == gbPlayerNum && !(pSoldier->uiStatusFlags & SOLDIER_TURNINGFROMHIT)) {
    if (pSoldier->bDirection != pSoldier->bDesiredDirection) {
      pSoldier->sAniDelay = FAST_TURN_ANIM_SPEED;
    } else {
      SetSoldierAniSpeed(pSoldier);
      //	FreeUpNPCFromTurning( pSoldier, LOOK);
    }
  }
}

BOOLEAN IsActionInterruptable(struct SOLDIERTYPE *pSoldier) {
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_NONINTERRUPT) {
    return (FALSE);
  }
  return (TRUE);
}

// WRAPPER FUNCTIONS FOR SOLDIER EVENTS
void SendSoldierPositionEvent(struct SOLDIERTYPE *pSoldier, float dNewXPos, float dNewYPos) {
  // Sent event for position update
  EV_S_SETPOSITION SSetPosition;

  SSetPosition.usSoldierID = GetSolID(pSoldier);
  SSetPosition.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  SSetPosition.dNewXPos = dNewXPos;
  SSetPosition.dNewYPos = dNewYPos;

  AddGameEvent(S_SETPOSITION, 0, &SSetPosition);
}

void SendSoldierDestinationEvent(struct SOLDIERTYPE *pSoldier, uint16_t usNewDestination) {
  // Sent event for position update
  EV_S_CHANGEDEST SChangeDest;

  SChangeDest.usSoldierID = GetSolID(pSoldier);
  SChangeDest.usNewDestination = usNewDestination;
  SChangeDest.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  AddGameEvent(S_CHANGEDEST, 0, &SChangeDest);
}

void SendSoldierSetDirectionEvent(struct SOLDIERTYPE *pSoldier, uint16_t usNewDirection) {
  // Sent event for position update
  EV_S_SETDIRECTION SSetDirection;

  SSetDirection.usSoldierID = GetSolID(pSoldier);
  SSetDirection.usNewDirection = usNewDirection;
  SSetDirection.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  AddGameEvent(S_SETDIRECTION, 0, &SSetDirection);
}

void SendSoldierSetDesiredDirectionEvent(struct SOLDIERTYPE *pSoldier,
                                         uint16_t usDesiredDirection) {
  // Sent event for position update
  EV_S_SETDESIREDDIRECTION SSetDesiredDirection;

  SSetDesiredDirection.usSoldierID = GetSolID(pSoldier);
  SSetDesiredDirection.usDesiredDirection = usDesiredDirection;
  SSetDesiredDirection.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  AddGameEvent(S_SETDESIREDDIRECTION, 0, &SSetDesiredDirection);
}

void SendGetNewSoldierPathEvent(struct SOLDIERTYPE *pSoldier, uint16_t sDestGridNo,
                                uint16_t usMovementAnim) {
  EV_S_GETNEWPATH SGetNewPath;

  SGetNewPath.usSoldierID = GetSolID(pSoldier);
  SGetNewPath.sDestGridNo = sDestGridNo;
  SGetNewPath.usMovementAnim = usMovementAnim;
  SGetNewPath.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  AddGameEvent(S_GETNEWPATH, 0, &SGetNewPath);
}

void SendChangeSoldierStanceEvent(struct SOLDIERTYPE *pSoldier, uint8_t ubNewStance) {
#if 0
	EV_S_CHANGESTANCE			SChangeStance;

#ifdef NETWORKED
	if( !IsTheSolderUnderMyControl( GetSolID(pSoldier)) )
		return;
#endif

	SChangeStance.ubNewStance   = ubNewStance;
	SChangeStance.usSoldierID  = GetSolID(pSoldier);
	SChangeStance.sXPos				= pSoldier->sX;
	SChangeStance.sYPos				= pSoldier->sY;
	SChangeStance.uiUniqueId = pSoldier -> uiUniqueSoldierIdValue;

	AddGameEvent( S_CHANGESTANCE, 0, &SChangeStance );
#endif

  ChangeSoldierStance(pSoldier, ubNewStance);
}

void SendBeginFireWeaponEvent(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  EV_S_BEGINFIREWEAPON SBeginFireWeapon;

  SBeginFireWeapon.usSoldierID = GetSolID(pSoldier);
  SBeginFireWeapon.sTargetGridNo = sTargetGridNo;
  SBeginFireWeapon.bTargetLevel = pSoldier->bTargetLevel;
  SBeginFireWeapon.bTargetCubeLevel = pSoldier->bTargetCubeLevel;
  SBeginFireWeapon.uiUniqueId = pSoldier->uiUniqueSoldierIdValue;

  AddGameEvent(S_BEGINFIREWEAPON, 0, &SBeginFireWeapon);
}

// This function just encapolates the check for turnbased and having an attacker in the first place
void ReleaseSoldiersAttacker(struct SOLDIERTYPE *pSoldier) {
  int32_t cnt;
  uint8_t ubNumToFree;

  // if ( gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT) )
  {
    // ATE: Removed...
    // if ( pSoldier->ubAttackerID != NOBODY )
    {
      // JA2 Gold
      // set next-to-previous attacker, so long as this isn't a repeat attack
      if (pSoldier->ubPreviousAttackerID != pSoldier->ubAttackerID) {
        pSoldier->ubNextToPreviousAttackerID = pSoldier->ubPreviousAttackerID;
      }

      // get previous attacker id
      pSoldier->ubPreviousAttackerID = pSoldier->ubAttackerID;

      // Copy BeingAttackedCount here....
      ubNumToFree = pSoldier->bBeingAttackedCount;
      // Zero it out BEFORE, as supression may increase it again...
      pSoldier->bBeingAttackedCount = 0;

      for (cnt = 0; cnt < ubNumToFree; cnt++) {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("@@@@@@@ Freeing up attacker of %d (attacker is %d) - "
                        "releasesoldierattacker num to free is %d",
                        GetSolID(pSoldier), pSoldier->ubAttackerID, ubNumToFree));
        ReduceAttackBusyCount(pSoldier->ubAttackerID, FALSE);
      }

      // ATE: Set to NOBODY if this person is NOT dead
      // otherise, we keep it so the kill can be awarded!
      if (pSoldier->bLife != 0 && pSoldier->ubBodyType != QUEENMONSTER) {
        pSoldier->ubAttackerID = NOBODY;
      }
    }
  }
}

BOOLEAN MercInWater(struct SOLDIERTYPE *pSoldier) {
  // Our water texture , for now is of a given type
  if (pSoldier->bOverTerrainType == LOW_WATER || pSoldier->bOverTerrainType == MED_WATER ||
      pSoldier->bOverTerrainType == DEEP_WATER) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

void RevivePlayerTeam() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  // End the turn of player charactors
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    ReviveSoldier(pSoldier);
  }
}

void ReviveSoldier(struct SOLDIERTYPE *pSoldier) {
  int16_t sX, sY;

  if (pSoldier->bLife < OKLIFE && IsSolActive(pSoldier)) {
    // If dead or unconscious, revive!
    pSoldier->uiStatusFlags &= (~SOLDIER_DEAD);

    pSoldier->bLife = pSoldier->bLifeMax;
    pSoldier->bBleeding = 0;
    pSoldier->ubDesiredHeight = ANIM_STAND;

    AddManToTeam(pSoldier->bTeam);

    // Set to standing
    pSoldier->fInNonintAnim = FALSE;
    pSoldier->fRTInNonintAnim = FALSE;

    // Change to standing,unless we can getup with an animation
    EVENT_InitNewSoldierAnim(pSoldier, STANDING, 0, TRUE);
    BeginSoldierGetup(pSoldier);

    // Makesure center of tile
    sX = CenterX(pSoldier->sGridNo);
    sY = CenterY(pSoldier->sGridNo);

    EVENT_SetSoldierPosition(pSoldier, (float)sX, (float)sY);

    // Dirty INterface
    fInterfacePanelDirty = DIRTYLEVEL2;
  }
}

void HandleAnimationProfile(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState, BOOLEAN fRemove) {
  // #if 0
  struct ANIM_PROF *pProfile;
  struct ANIM_PROF_DIR *pProfileDir;
  struct ANIM_PROF_TILE *pProfileTile;
  int8_t bProfileID;
  uint32_t iTileCount;
  int16_t sGridNo;
  uint16_t usAnimSurface;

  // ATE

  // Get Surface Index
  usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usAnimState);

  CHECKV(usAnimSurface != INVALID_ANIMATION_SURFACE);

  bProfileID = gAnimSurfaceDatabase[usAnimSurface].bProfile;

  // Determine if this animation has a profile
  if (bProfileID != -1) {
    // Getprofile
    pProfile = &(gpAnimProfiles[bProfileID]);

    // Get direction
    pProfileDir = &(pProfile->Dirs[pSoldier->bDirection]);

    // Loop tiles and set accordingly into world
    for (iTileCount = 0; iTileCount < pProfileDir->ubNumTiles; iTileCount++) {
      pProfileTile = &(pProfileDir->pTiles[iTileCount]);

      sGridNo = pSoldier->sGridNo + ((WORLD_COLS * pProfileTile->bTileY) + pProfileTile->bTileX);

      // Check if in bounds
      if (!OutOfBounds(pSoldier->sGridNo, sGridNo)) {
        if (fRemove) {
          // Remove from world
          RemoveMerc(sGridNo, pSoldier, TRUE);
        } else {
          // PLace into world
          AddMercToHead(sGridNo, pSoldier, FALSE);
          // if ( pProfileTile->bTileY != 0 || pProfileTile->bTileX != 0 )
          {
            gpWorldLevelData[sGridNo].pMercHead->uiFlags |= LEVELNODE_MERCPLACEHOLDER;
            gpWorldLevelData[sGridNo].pMercHead->uiAnimHitLocationFlags = pProfileTile->usTileFlags;
          }
        }
      }
    }
  }

  // #endif
}

struct LEVELNODE *GetAnimProfileFlags(uint16_t sGridNo, uint16_t *usFlags,
                                      struct SOLDIERTYPE **ppTargSoldier,
                                      struct LEVELNODE *pGivenNode) {
  struct LEVELNODE *pNode;

  (*ppTargSoldier) = NULL;
  (*usFlags) = 0;

  if (pGivenNode == NULL) {
    pNode = gpWorldLevelData[sGridNo].pMercHead;
  } else {
    pNode = pGivenNode->pNext;
  }

  // #if 0

  if (pNode != NULL) {
    if (pNode->uiFlags & LEVELNODE_MERCPLACEHOLDER) {
      (*usFlags) = (uint16_t)pNode->uiAnimHitLocationFlags;
      (*ppTargSoldier) = pNode->pSoldier;
    }
  }

  // #endif

  return (pNode);
}

BOOLEAN GetProfileFlagsFromGridno(struct SOLDIERTYPE *pSoldier, uint16_t usAnimState,
                                  uint16_t sTestGridNo, uint16_t *usFlags) {
  struct ANIM_PROF *pProfile;
  struct ANIM_PROF_DIR *pProfileDir;
  struct ANIM_PROF_TILE *pProfileTile;
  int8_t bProfileID;
  uint32_t iTileCount;
  int16_t sGridNo;
  uint16_t usAnimSurface;

  // Get Surface Index
  usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usAnimState);

  CHECKF(usAnimSurface != INVALID_ANIMATION_SURFACE);

  bProfileID = gAnimSurfaceDatabase[usAnimSurface].bProfile;

  *usFlags = 0;

  // Determine if this animation has a profile
  if (bProfileID != -1) {
    // Getprofile
    pProfile = &(gpAnimProfiles[bProfileID]);

    // Get direction
    pProfileDir = &(pProfile->Dirs[pSoldier->bDirection]);

    // Loop tiles and set accordingly into world
    for (iTileCount = 0; iTileCount < pProfileDir->ubNumTiles; iTileCount++) {
      pProfileTile = &(pProfileDir->pTiles[iTileCount]);

      sGridNo = pSoldier->sGridNo + ((WORLD_COLS * pProfileTile->bTileY) + pProfileTile->bTileX);

      // Check if in bounds
      if (!OutOfBounds(pSoldier->sGridNo, sGridNo)) {
        if (sGridNo == sTestGridNo) {
          *usFlags = pProfileTile->usTileFlags;
          return (TRUE);
        }
      }
    }
  }

  return (FALSE);
}

void EVENT_SoldierBeginGiveItem(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pTSoldier;

  if (VerifyGiveItem(pSoldier, &pTSoldier)) {
    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    pSoldier->bDesiredDirection = pSoldier->bPendingActionData3;
    pSoldier->bDirection = pSoldier->bPendingActionData3;

    // begin animation
    EVENT_InitNewSoldierAnim(pSoldier, GIVE_ITEM, 0, FALSE);

  } else {
    UnSetEngagedInConvFromPCAction(pSoldier);

    MemFree(pSoldier->pTempObject);
  }
}

void EVENT_SoldierBeginBladeAttack(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                   uint8_t ubDirection) {
  struct SOLDIERTYPE *pTSoldier;
  // uint32_t uiMercFlags;
  uint16_t usSoldierIndex;
  uint8_t ubTDirection;
  ROTTING_CORPSE *pCorpse;

  // Increment the number of people busy doing stuff because of an attack
  // if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
  //{
  gTacticalStatus.ubAttackBusyCount++;
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("Begin blade attack: ATB  %d", gTacticalStatus.ubAttackBusyCount));

  //}

  // CHANGE DIRECTION AND GOTO ANIMATION NOW
  EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
  EVENT_SetSoldierDirection(pSoldier, ubDirection);
  // CHANGE TO ANIMATION

  // DETERMINE ANIMATION TO PLAY
  // LATER BASED ON IF TAREGT KNOWS OF US, STANCE, ETC
  // GET POINTER TO TAREGT
  if (pSoldier->uiStatusFlags & SOLDIER_MONSTER) {
    uint8_t ubTargetID;

    // Is there an unconscious guy at gridno......
    ubTargetID = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);

    if (ubTargetID != NOBODY &&
        ((MercPtrs[ubTargetID]->bLife < OKLIFE && MercPtrs[ubTargetID]->bLife > 0) ||
         (MercPtrs[ubTargetID]->bBreath < OKBREATH && MercPtrs[ubTargetID]->bCollapsed))) {
      pSoldier->uiPendingActionData4 = ubTargetID;
      // add regen bonus
      pSoldier->bRegenerationCounter++;
      EVENT_InitNewSoldierAnim(pSoldier, MONSTER_BEGIN_EATTING_FLESH, 0, FALSE);
    } else {
      if (PythSpacesAway(pSoldier->sGridNo, sGridNo) <= 1) {
        EVENT_InitNewSoldierAnim(pSoldier, MONSTER_CLOSE_ATTACK, 0, FALSE);
      } else {
        EVENT_InitNewSoldierAnim(pSoldier, ADULTMONSTER_ATTACKING, 0, FALSE);
      }
    }
  } else if (pSoldier->ubBodyType == BLOODCAT) {
    // Check if it's a claws or teeth...
    if (pSoldier->inv[HANDPOS].usItem == BLOODCAT_CLAW_ATTACK) {
      EVENT_InitNewSoldierAnim(pSoldier, BLOODCAT_SWIPE, 0, FALSE);
    } else {
      EVENT_InitNewSoldierAnim(pSoldier, BLOODCAT_BITE_ANIM, 0, FALSE);
    }
  } else {
    usSoldierIndex = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
    if (usSoldierIndex != NOBODY) {
      GetSoldier(&pTSoldier, usSoldierIndex);

      // Look at stance of target
      switch (gAnimControl[pTSoldier->usAnimState].ubEndHeight) {
        case ANIM_STAND:
        case ANIM_CROUCH:

          // CHECK IF HE CAN SEE US, IF SO RANDOMIZE
          if (pTSoldier->bOppList[pSoldier->ubID] == 0 && pTSoldier->bTeam != pSoldier->bTeam) {
            // WE ARE NOT SEEN
            EVENT_InitNewSoldierAnim(pSoldier, STAB, 0, FALSE);
          } else {
            // WE ARE SEEN
            if (Random(50) > 25) {
              EVENT_InitNewSoldierAnim(pSoldier, STAB, 0, FALSE);
            } else {
              EVENT_InitNewSoldierAnim(pSoldier, SLICE, 0, FALSE);
            }

            // IF WE ARE SEEN, MAKE SURE GUY TURNS!
            // Get direction to target
            // IF WE ARE AN ANIMAL, CAR, MONSTER, DONT'T TURN
            if (!(pTSoldier->uiStatusFlags &
                  (SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE))) {
              // OK, stop merc....
              EVENT_StopMerc(pTSoldier, pTSoldier->sGridNo, pTSoldier->bDirection);

              if (pTSoldier->bTeam != gbPlayerNum) {
                CancelAIAction(pTSoldier, TRUE);
              }

              ubTDirection = (uint8_t)GetDirectionFromGridNo(pSoldier->sGridNo, pTSoldier);
              SendSoldierSetDesiredDirectionEvent(pTSoldier, ubTDirection);
            }
          }

          break;

        case ANIM_PRONE:

          // CHECK OUR STANCE
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_CROUCH) {
            // SET DESIRED STANCE AND SET PENDING ANIMATION
            SendChangeSoldierStanceEvent(pSoldier, ANIM_CROUCH);
            pSoldier->usPendingAnimation = CROUCH_STAB;
          } else {
            // USE crouched one
            // NEED TO CHANGE STANCE IF NOT CROUCHD!
            EVENT_InitNewSoldierAnim(pSoldier, CROUCH_STAB, 0, FALSE);
          }
          break;
      }
    } else {
      // OK, SEE IF THERE IS AN OBSTACLE HERE...
      if (!NewOKDestination(pSoldier, sGridNo, FALSE, pSoldier->bLevel)) {
        EVENT_InitNewSoldierAnim(pSoldier, STAB, 0, FALSE);
      } else {
        // Check for corpse!
        pCorpse = GetCorpseAtGridNo(sGridNo, pSoldier->bLevel);

        if (pCorpse == NULL) {
          EVENT_InitNewSoldierAnim(pSoldier, CROUCH_STAB, 0, FALSE);
        } else {
          if (IsValidDecapitationCorpse(pCorpse)) {
            EVENT_InitNewSoldierAnim(pSoldier, DECAPITATE, 0, FALSE);
          } else {
            EVENT_InitNewSoldierAnim(pSoldier, CROUCH_STAB, 0, FALSE);
          }
        }
      }
    }
  }

  // SET TARGET GRIDNO
  pSoldier->sTargetGridNo = sGridNo;
  pSoldier->bTargetLevel = pSoldier->bLevel;
  pSoldier->ubTargetID = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
}

void EVENT_SoldierBeginPunchAttack(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                   uint8_t ubDirection) {
  BOOLEAN fMartialArtist = FALSE;
  struct SOLDIERTYPE *pTSoldier;
  // uint32_t uiMercFlags;
  uint16_t usSoldierIndex;
  uint8_t ubTDirection;
  BOOLEAN fChangeDirection = FALSE;
  uint16_t usItem;

  // Get item in hand...
  usItem = pSoldier->inv[HANDPOS].usItem;

  // Increment the number of people busy doing stuff because of an attack
  // if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
  //{
  gTacticalStatus.ubAttackBusyCount++;
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("Begin HTH attack: ATB  %d", gTacticalStatus.ubAttackBusyCount));

  //}

  // get target.....
  usSoldierIndex = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bLevel);
  if (usSoldierIndex != NOBODY) {
    GetSoldier(&pTSoldier, usSoldierIndex);

    fChangeDirection = TRUE;
  } else {
    return;
  }

  if (fChangeDirection) {
    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);
  }

  // Are we a martial artist?
  if (HAS_SKILL_TRAIT(pSoldier, MARTIALARTS)) {
    fMartialArtist = TRUE;
  }

  if (fMartialArtist && !AreInMeanwhile() && usItem != CROWBAR) {
    // Are we in attack mode yet?
    if (pSoldier->usAnimState != NINJA_BREATH &&
        gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_STAND &&
        gAnimControl[pTSoldier->usAnimState].ubHeight != ANIM_PRONE) {
      EVENT_InitNewSoldierAnim(pSoldier, NINJA_GOTOBREATH, 0, FALSE);
    } else {
      DoNinjaAttack(pSoldier);
    }
  } else {
    // Look at stance of target
    switch (gAnimControl[pTSoldier->usAnimState].ubEndHeight) {
      case ANIM_STAND:
      case ANIM_CROUCH:

        if (usItem != CROWBAR) {
          EVENT_InitNewSoldierAnim(pSoldier, PUNCH, 0, FALSE);
        } else {
          EVENT_InitNewSoldierAnim(pSoldier, CROWBAR_ATTACK, 0, FALSE);
        }

        // CHECK IF HE CAN SEE US, IF SO CHANGE DIR
        if (pTSoldier->bOppList[pSoldier->ubID] == 0 && pTSoldier->bTeam != pSoldier->bTeam) {
          // Get direction to target
          // IF WE ARE AN ANIMAL, CAR, MONSTER, DONT'T TURN
          if (!(pTSoldier->uiStatusFlags & (SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE))) {
            // OK, stop merc....
            EVENT_StopMerc(pTSoldier, pTSoldier->sGridNo, pTSoldier->bDirection);

            if (pTSoldier->bTeam != gbPlayerNum) {
              CancelAIAction(pTSoldier, TRUE);
            }

            ubTDirection = (uint8_t)GetDirectionFromGridNo(pSoldier->sGridNo, pTSoldier);
            SendSoldierSetDesiredDirectionEvent(pTSoldier, ubTDirection);
          }
        }
        break;

      case ANIM_PRONE:

        // CHECK OUR STANCE
        // ATE: Added this for CIV body types 'cause of elliot
        if (!IS_MERC_BODY_TYPE(pSoldier)) {
          EVENT_InitNewSoldierAnim(pSoldier, PUNCH, 0, FALSE);
        } else {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_CROUCH) {
            // SET DESIRED STANCE AND SET PENDING ANIMATION
            SendChangeSoldierStanceEvent(pSoldier, ANIM_CROUCH);
            pSoldier->usPendingAnimation = PUNCH_LOW;
          } else {
            // USE crouched one
            // NEED TO CHANGE STANCE IF NOT CROUCHD!
            EVENT_InitNewSoldierAnim(pSoldier, PUNCH_LOW, 0, FALSE);
          }
        }
        break;
    }
  }

  // SET TARGET GRIDNO
  pSoldier->sTargetGridNo = sGridNo;
  pSoldier->bTargetLevel = pSoldier->bLevel;
  pSoldier->sLastTarget = sGridNo;
  pSoldier->ubTargetID = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
}

void EVENT_SoldierBeginKnifeThrowAttack(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                        uint8_t ubDirection) {
  // Increment the number of people busy doing stuff because of an attack
  // if ( (gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) )
  //{
  gTacticalStatus.ubAttackBusyCount++;
  //}
  pSoldier->bBulletsLeft = 1;
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("!!!!!!! Starting knifethrow attack, bullets left %d", pSoldier->bBulletsLeft));

  EVENT_InitNewSoldierAnim(pSoldier, THROW_KNIFE, 0, FALSE);

  // CHANGE DIRECTION AND GOTO ANIMATION NOW
  EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
  EVENT_SetSoldierDirection(pSoldier, ubDirection);

  // SET TARGET GRIDNO
  pSoldier->sTargetGridNo = sGridNo;
  pSoldier->sLastTarget = sGridNo;
  pSoldier->fTurningFromPronePosition = 0;
  // NB target level must be set by functions outside of here... but I think it
  // is already set in HandleItem or in the AI code - CJC
  pSoldier->ubTargetID = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
}

void EVENT_SoldierBeginDropBomb(struct SOLDIERTYPE *pSoldier) {
  // Increment the number of people busy doing stuff because of an attack
  switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
    case ANIM_STAND:

      EVENT_InitNewSoldierAnim(pSoldier, PLANT_BOMB, 0, FALSE);
      break;

    default:

      // Call hander for planting bomb...
      HandleSoldierDropBomb(pSoldier, pSoldier->sPendingActionData2);
      SoldierGotoStationaryStance(pSoldier);
      break;
  }
}

void EVENT_SoldierBeginUseDetonator(struct SOLDIERTYPE *pSoldier) {
  // Increment the number of people busy doing stuff because of an attack
  switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
    case ANIM_STAND:

      EVENT_InitNewSoldierAnim(pSoldier, USE_REMOTE, 0, FALSE);
      break;

    default:

      // Call hander for planting bomb...
      HandleSoldierUseRemote(pSoldier, pSoldier->sPendingActionData2);
      break;
  }
}

void EVENT_SoldierBeginFirstAid(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                uint8_t ubDirection) {
  struct SOLDIERTYPE *pTSoldier;
  // uint32_t uiMercFlags;
  uint16_t usSoldierIndex;
  BOOLEAN fRefused = FALSE;

  usSoldierIndex = WhoIsThere2(sGridNo, pSoldier->bLevel);
  if (usSoldierIndex != NOBODY) {
    pTSoldier = MercPtrs[usSoldierIndex];

    // OK, check if we should play quote...
    if (pTSoldier->bTeam != gbPlayerNum) {
      if (pTSoldier->ubProfile != NO_PROFILE && pTSoldier->ubProfile >= FIRST_RPC &&
          !RPC_RECRUITED(pTSoldier)) {
        fRefused = PCDoesFirstAidOnNPC(pTSoldier->ubProfile);
      }

      if (!fRefused) {
        if (CREATURE_OR_BLOODCAT(pTSoldier)) {
          // nope!!
          fRefused = TRUE;
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                    Message[STR_REFUSE_FIRSTAID_FOR_CREATURE]);
        } else if (!pTSoldier->bNeutral && pTSoldier->bLife >= OKLIFE &&
                   pTSoldier->bSide != pSoldier->bSide) {
          fRefused = TRUE;
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[STR_REFUSE_FIRSTAID]);
        }
      }
    }

    if (fRefused) {
      UnSetUIBusy(pSoldier->ubID);
      return;
    }

    // ATE: We can only give firsty aid to one perosn at a time... cancel
    // any now...
    InternalGivingSoldierCancelServices(pSoldier, FALSE);

    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);

    // CHECK OUR STANCE AND GOTO CROUCH IF NEEDED
    // if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_CROUCH )
    //{
    // SET DESIRED STANCE AND SET PENDING ANIMATION
    //	SendChangeSoldierStanceEvent( pSoldier, ANIM_CROUCH );
    //	pSoldier->usPendingAnimation = START_AID;
    //}
    // else
    {
      // CHANGE TO ANIMATION
      EVENT_InitNewSoldierAnim(pSoldier, START_AID, 0, FALSE);
    }

    // SET TARGET GRIDNO
    pSoldier->sTargetGridNo = sGridNo;

    // SET PARTNER ID
    pSoldier->ubServicePartner = (uint8_t)usSoldierIndex;

    // SET PARTNER'S COUNT REFERENCE
    pTSoldier->ubServiceCount++;

    // If target and doer are no the same guy...
    if (pTSoldier->ubID != GetSolID(pSoldier) && !pTSoldier->bCollapsed) {
      SoldierGotoStationaryStance(pTSoldier);
    }
  }
}

void EVENT_SoldierEnterVehicle(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubDirection) {
  struct SOLDIERTYPE *pTSoldier;
  uint32_t uiMercFlags;
  uint16_t usSoldierIndex;

  if (FindSoldier(sGridNo, &usSoldierIndex, &uiMercFlags, FIND_SOLDIER_GRIDNO)) {
    pTSoldier = MercPtrs[usSoldierIndex];

    // Enter vehicle...
    EnterVehicle(pTSoldier, pSoldier);
  }

  UnSetUIBusy(pSoldier->ubID);
}

uint32_t SoldierDressWound(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pVictim,
                           int16_t sKitPts, int16_t sStatus) {
  uint32_t uiDressSkill, uiPossible, uiActual, uiMedcost, uiDeficiency, uiAvailAPs, uiUsedAPs;
  uint8_t ubBelowOKlife, ubPtsLeft;

  if (pVictim->bBleeding < 1 && pVictim->bLife >= OKLIFE) {
    return (0);  // nothing to do, shouldn't have even been called!
  }

  if (pVictim->bLife == 0) {
    return (0);
  }

  // in case he has multiple kits in hand, limit influence of kit status to 100%!
  if (sStatus >= 100) {
    sStatus = 100;
  }

  // calculate wound-dressing skill (3x medical, 2x equip, 1x level, 1x dex)
  uiDressSkill = ((3 * EffectiveMedical(pSoldier)) +    // medical knowledge
                  (2 * sStatus) +                       // state of medical kit
                  (10 * EffectiveExpLevel(pSoldier)) +  // battle injury experience
                  EffectiveDexterity(pSoldier)) /
                 7;  // general "handiness"

  // try to use every AP that the merc has left
  uiAvailAPs = pSoldier->bActionPoints;

  // OK, If we are in real-time, use another value...
  if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
    // Set to a value which looks good based on our tactical turns duration
    uiAvailAPs = RT_FIRST_AID_GAIN_MODIFIER;
  }

  // calculate how much bandaging CAN be done this turn
  uiPossible = (uiAvailAPs * uiDressSkill) / 50;  // max rate is 2 * fullAPs

  // if no healing is possible (insufficient APs or insufficient dressSkill)
  if (!uiPossible) return (0);

  if (pSoldier->inv[HANDPOS].usItem == MEDICKIT)  // using the GOOD medic stuff
  {
    uiPossible += (uiPossible / 2);  // add extra 50 %
  }

  uiActual = uiPossible;  // start by assuming maximum possible

  // figure out how far below OKLIFE the victim is
  if (pVictim->bLife >= OKLIFE) {
    ubBelowOKlife = 0;
  } else {
    ubBelowOKlife = OKLIFE - pVictim->bLife;
  }

  // figure out how many healing pts we need to stop dying (2x cost)
  uiDeficiency = (2 * ubBelowOKlife);

  // if, after that, the patient will still be bleeding
  if ((pVictim->bBleeding - ubBelowOKlife) > 0) {
    // then add how many healing pts we need to stop bleeding (1x cost)
    uiDeficiency += (pVictim->bBleeding - ubBelowOKlife);
  }

  // now, make sure we weren't going to give too much
  if (uiActual > uiDeficiency)  // if we were about to apply too much
    uiActual = uiDeficiency;    // reduce actual not to waste anything

  // now make sure we HAVE that much
  if (pSoldier->inv[HANDPOS].usItem == MEDICKIT) {
    uiMedcost = (uiActual + 1) / 2;  // cost is only half, rounded up

    if (uiMedcost > (uint32_t)sKitPts)  // if we can't afford this
    {
      uiMedcost = sKitPts;       // what CAN we afford?
      uiActual = uiMedcost * 2;  // give double this as aid
    }
  } else {
    uiMedcost = uiActual;

    if (uiMedcost > (uint32_t)sKitPts)  // can't afford it
    {
      uiMedcost = uiActual = sKitPts;  // recalc cost AND aid
    }
  }

  ubPtsLeft = (uint8_t)uiActual;

  // heal real life points first (if below OKLIFE) because we don't want the
  // patient still DYING if bandages run out, or medic is disabled/distracted!
  // NOTE: Dressing wounds for life below OKLIFE now costs 2 pts/life point!
  if (ubPtsLeft && pVictim->bLife < OKLIFE) {
    // if we have enough points to bring him all the way to OKLIFE this turn
    if (ubPtsLeft >= (2 * ubBelowOKlife)) {
      // raise life to OKLIFE
      pVictim->bLife = OKLIFE;

      // reduce bleeding by the same number of life points healed up
      pVictim->bBleeding -= ubBelowOKlife;

      // use up appropriate # of actual healing points
      ubPtsLeft -= (2 * ubBelowOKlife);
    } else {
      pVictim->bLife += (ubPtsLeft / 2);
      pVictim->bBleeding -= (ubPtsLeft / 2);

      ubPtsLeft = ubPtsLeft % 2;  // if ptsLeft was odd, ptsLeft = 1
    }

    // this should never happen any more, but make sure bleeding not negative
    if (pVictim->bBleeding < 0) {
      pVictim->bBleeding = 0;
    }

    // if this healing brought the patient out of the worst of it, cancel dying
    if (pVictim->bLife >= OKLIFE) {
      // pVictim->dying = pVictim->dyingComment = FALSE;
      // pVictim->shootOn = TRUE;

      // turn off merc QUOTE flags
      pVictim->fDyingComment = FALSE;
    }

    // update patient's entire panel (could have regained consciousness, etc.)
  }

  // if any healing points remain, apply that to any remaining bleeding (1/1)
  // DON'T spend any APs/kit pts to cure bleeding until merc is no longer dying
  // if ( ubPtsLeft && pVictim->bBleeding && !pVictim->dying)
  if (ubPtsLeft && pVictim->bBleeding) {
    // if we have enough points to bandage all remaining bleeding this turn
    if (ubPtsLeft >= pVictim->bBleeding) {
      ubPtsLeft -= pVictim->bBleeding;
      pVictim->bBleeding = 0;
    } else  // bandage what we can
    {
      pVictim->bBleeding -= ubPtsLeft;
      ubPtsLeft = 0;
    }

    // update patient's life bar only
  }

  // if wound has been dressed enough so that bleeding won't occur, turn off
  // the "warned about bleeding" flag so merc tells us about the next bleeding
  if (pVictim->bBleeding <= MIN_BLEEDING_THRESHOLD) {
    pVictim->fWarnedAboutBleeding = FALSE;
  }

  // if there are any ptsLeft now, then we didn't actually get to use them
  uiActual -= ubPtsLeft;

  // usedAPs equals (actionPts) * (%of possible points actually used)
  uiUsedAPs = (uiActual * uiAvailAPs) / uiPossible;

  if (pSoldier->inv[HANDPOS].usItem == MEDICKIT)  // using the GOOD medic stuff
  {
    uiUsedAPs = (uiUsedAPs * 2) / 3;  // reverse 50% bonus by taking 2/3rds
  }

  DeductPoints(pSoldier, (int16_t)uiUsedAPs, (int16_t)((uiUsedAPs * BP_PER_AP_LT_EFFORT)));

  if (PTR_OURTEAM) {
    // MEDICAL GAIN   (actual / 2):  Helped someone by giving first aid
    StatChange(pSoldier, MEDICALAMT, (uint16_t)(uiActual / 2), FALSE);

    // DEXTERITY GAIN (actual / 6):  Helped someone by giving first aid
    StatChange(pSoldier, DEXTAMT, (uint16_t)(uiActual / 6), FALSE);
  }

  return (uiMedcost);
}

void InternalReceivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier, BOOLEAN fPlayEndAnim) {
  struct SOLDIERTYPE *pTSoldier;
  int32_t cnt;

  if (pSoldier->ubServiceCount > 0) {
    // Loop through guys who have us as servicing
    for (pTSoldier = Menptr, cnt = 0; cnt < MAX_NUM_SOLDIERS; pTSoldier++, cnt++) {
      if (pTSoldier->bActive) {
        if (pTSoldier->ubServicePartner == GetSolID(pSoldier)) {
          // END SERVICE!
          pSoldier->ubServiceCount--;

          pTSoldier->ubServicePartner = NOBODY;

          if (gTacticalStatus.fAutoBandageMode) {
            pSoldier->ubAutoBandagingMedic = NOBODY;

            ActionDone(pTSoldier);
          } else {
            // don't use end aid animation in autobandage
            if (pTSoldier->bLife >= OKLIFE && pTSoldier->bBreath > 0 && fPlayEndAnim) {
              EVENT_InitNewSoldierAnim(pTSoldier, END_AID, 0, FALSE);
            }
          }
        }
      }
    }
  }
}

void ReceivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier) {
  InternalReceivingSoldierCancelServices(pSoldier, TRUE);
}

void InternalGivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier, BOOLEAN fPlayEndAnim) {
  struct SOLDIERTYPE *pTSoldier;

  // GET TARGET SOLDIER
  if (pSoldier->ubServicePartner != NOBODY) {
    pTSoldier = MercPtrs[pSoldier->ubServicePartner];

    // END SERVICE!
    pTSoldier->ubServiceCount--;

    pSoldier->ubServicePartner = NOBODY;

    if (gTacticalStatus.fAutoBandageMode) {
      pTSoldier->ubAutoBandagingMedic = NOBODY;

      ActionDone(pSoldier);
    } else {
      if (pSoldier->bLife >= OKLIFE && pSoldier->bBreath > 0 && fPlayEndAnim) {
        // don't use end aid animation in autobandage
        EVENT_InitNewSoldierAnim(pSoldier, END_AID, 0, FALSE);
      }
    }
  }
}

void GivingSoldierCancelServices(struct SOLDIERTYPE *pSoldier) {
  InternalGivingSoldierCancelServices(pSoldier, TRUE);
}

void HaultSoldierFromSighting(struct SOLDIERTYPE *pSoldier, BOOLEAN fFromSightingEnemy) {
  // SEND HUALT EVENT!
  // EV_S_STOP_MERC				SStopMerc;

  // SStopMerc.sGridNo					= pSoldier->sGridNo;
  // SStopMerc.bDirection			= pSoldier->bDirection;
  // SStopMerc.usSoldierID			= GetSolID(pSoldier);
  // AddGameEvent( S_STOP_MERC, 0, &SStopMerc );

  // If we are a 'specialmove... ignore...
  if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_SPECIALMOVE)) {
    return;
  }

  // OK, check if we were going to throw something, and give it back if so!
  if (pSoldier->pTempObject != NULL && fFromSightingEnemy) {
    // Place it back into inv....
    AutoPlaceObject(pSoldier, pSoldier->pTempObject, FALSE);
    MemFree(pSoldier->pTempObject);
    pSoldier->pTempObject = NULL;
    pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
    pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;

    // Decrement attack counter...
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("@@@@@@@ Reducing attacker busy count..., ending throw because saw something"));
    ReduceAttackBusyCount(pSoldier->ubID, FALSE);

    // ATE: Goto stationary stance......
    SoldierGotoStationaryStance(pSoldier);

    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
  }

  if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
    EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
  } else {
    // Pause this guy from no APS
    AdjustNoAPToFinishMove(pSoldier, TRUE);

    pSoldier->ubReasonCantFinishMove = REASON_STOPPED_SIGHT;

    // ATE; IF turning to shoot, stop!
    // ATE: We want to do this only for enemies, not items....
    if (pSoldier->fTurningToShoot && fFromSightingEnemy) {
      pSoldier->fTurningToShoot = FALSE;
      // Release attacker

      // OK - this is hightly annoying , but due to the huge combinations of
      // things that can happen - 1 of them is that sLastTarget will get unset
      // after turn is done - so set flag here to tell it not to...
      pSoldier->fDontUnsetLastTargetFromTurn = TRUE;

      DebugMsg(
          TOPIC_JA2, DBG_LEVEL_3,
          String("@@@@@@@ Reducing attacker busy count..., ending fire because saw something"));
      ReduceAttackBusyCount(pSoldier->ubID, FALSE);
    }

    // OK, if we are stopped at our destination, cancel pending action...
    if (fFromSightingEnemy) {
      if (pSoldier->ubPendingAction != NO_PENDING_ACTION &&
          pSoldier->sGridNo == pSoldier->sFinalDestination) {
        pSoldier->ubPendingAction = NO_PENDING_ACTION;
      }

      // Stop pending animation....
      pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
      pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;
    }

    if (!pSoldier->fTurningToShoot) {
      pSoldier->fTurningFromPronePosition = FALSE;
    }
  }

  // Unset UI!
  if (fFromSightingEnemy || (pSoldier->pTempObject == NULL && !pSoldier->fTurningToShoot)) {
    UnSetUIBusy(pSoldier->ubID);
  }

  pSoldier->bTurningFromUI = FALSE;

  UnSetEngagedInConvFromPCAction(pSoldier);
}

// HUALT EVENT IS USED TO STOP A MERC - NETWORKING SHOULD CHECK / ADJUST TO GRIDNO?
void EVENT_StopMerc(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, int8_t bDirection) {
  int16_t sX, sY;

  // MOVE GUY TO GRIDNO--- SHOULD BE THE SAME UNLESS IN MULTIPLAYER
  // Makesure center of tile
  sX = CenterX(sGridNo);
  sY = CenterY(sGridNo);

  // Cancel pending events
  if (!pSoldier->fDelayedMovement) {
    pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
    pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;
    pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
    pSoldier->ubPendingAction = NO_PENDING_ACTION;
  }

  pSoldier->bEndDoorOpenCode = 0;
  pSoldier->fTurningFromPronePosition = 0;

  // Cancel path data!
  pSoldier->usPathIndex = pSoldier->usPathDataSize = 0;

  // Set ext tile waiting flag off!
  pSoldier->fDelayedMovement = FALSE;

  // Turn off reverse...
  pSoldier->bReverse = FALSE;

  EVENT_SetSoldierPosition(pSoldier, (float)sX, (float)sY);
  pSoldier->sDestXPos = (int16_t)pSoldier->dXPos;
  pSoldier->sDestYPos = (int16_t)pSoldier->dYPos;
  EVENT_SetSoldierDirection(pSoldier, bDirection);

  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) {
    SoldierGotoStationaryStance(pSoldier);
  }

  // ATE; IF turning to shoot, stop!
  if (pSoldier->fTurningToShoot) {
    pSoldier->fTurningToShoot = FALSE;
    // Release attacker
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("@@@@@@@ Reducing attacker busy count..., ending fire because saw something"));
    ReduceAttackBusyCount(pSoldier->ubID, FALSE);
  }

  // Turn off multi-move speed override....
  if (pSoldier->sGridNo == pSoldier->sFinalDestination) {
    pSoldier->fUseMoverrideMoveSpeed = FALSE;
  }

  // Unset UI!
  UnSetUIBusy(pSoldier->ubID);

  UnMarkMovementReserved(pSoldier);
}

void ReLoadSoldierAnimationDueToHandItemChange(struct SOLDIERTYPE *pSoldier, uint16_t usOldItem,
                                               uint16_t usNewItem) {
  // DON'T continue aiming!
  // GOTO STANCE
  // CHECK FOR AIMING ANIMATIONS
  BOOLEAN fOldRifle = FALSE;
  BOOLEAN fNewRifle = FALSE;

  // Shutoff burst....
  // ( we could be on, then change gun that does not have burst )
  if (Weapon[usNewItem].ubShotsPerBurst == 0) {
    pSoldier->bDoBurst = FALSE;
    pSoldier->bWeaponMode = WM_NORMAL;
  }

  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIREREADY) {
    // Stop aiming!
    SoldierGotoStationaryStance(pSoldier);
  }

  // Cancel services...
  GivingSoldierCancelServices(pSoldier);

  // Did we have a rifle and do we now not have one?
  if (usOldItem != NOTHING) {
    if (Item[usOldItem].usItemClass == IC_GUN) {
      if ((Item[usOldItem].fFlags & ITEM_TWO_HANDED) && usOldItem != ROCKET_LAUNCHER) {
        fOldRifle = TRUE;
      }
    }
  }

  if (usNewItem != NOTHING) {
    if (Item[usNewItem].usItemClass == IC_GUN) {
      if ((Item[usNewItem].fFlags & ITEM_TWO_HANDED) && usNewItem != ROCKET_LAUNCHER) {
        fNewRifle = TRUE;
      }
    }
  }

  // Switch on stance!
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:

      if (fOldRifle && !fNewRifle) {
        // Put it away!
        EVENT_InitNewSoldierAnim(pSoldier, LOWER_RIFLE, 0, FALSE);
      } else if (!fOldRifle && fNewRifle) {
        // Bring it up!
        EVENT_InitNewSoldierAnim(pSoldier, RAISE_RIFLE, 0, FALSE);
      } else {
        SetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);
      }
      break;

    case ANIM_CROUCH:
    case ANIM_PRONE:

      SetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);
      break;
  }
}

uint16_t *CreateEnemyGlow16BPPPalette(struct JPaletteEntry *pPalette, uint32_t rscale,
                                      uint32_t gscale, BOOLEAN fAdjustGreen) {
  uint16_t *p16BPPPalette, usColor;
  uint32_t cnt;
  uint32_t rmod, gmod, bmod;
  uint8_t r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (uint16_t *)MemAlloc(sizeof(uint16_t) * 256);

  for (cnt = 0; cnt < 256; cnt++) {
    gmod = (pPalette[cnt].green);
    bmod = (pPalette[cnt].blue);

    rmod = max(rscale, (pPalette[cnt].red));

    if (fAdjustGreen) {
      gmod = max(gscale, (pPalette[cnt].green));
    }

    r = (uint8_t)min(rmod, 255);
    g = (uint8_t)min(gmod, 255);
    b = (uint8_t)min(bmod, 255);

    usColor = JVideo_PackRGB16(r, g, b);

    if ((usColor == 0) && ((r + g + b) != 0)) usColor = 0x0001;

    p16BPPPalette[cnt] = usColor;
  }
  return (p16BPPPalette);
}

uint16_t *CreateEnemyGreyGlow16BPPPalette(struct JPaletteEntry *pPalette, uint32_t rscale,
                                          uint32_t gscale, BOOLEAN fAdjustGreen) {
  uint16_t *p16BPPPalette, usColor;
  uint32_t cnt, lumin;
  uint32_t rmod, gmod, bmod;
  uint8_t r, g, b;

  Assert(pPalette != NULL);

  p16BPPPalette = (uint16_t *)MemAlloc(sizeof(uint16_t) * 256);

  for (cnt = 0; cnt < 256; cnt++) {
    lumin = (pPalette[cnt].red * 299 / 1000) + (pPalette[cnt].green * 587 / 1000) +
            (pPalette[cnt].blue * 114 / 1000);
    rmod = (100 * lumin) / 256;
    gmod = (100 * lumin) / 256;
    bmod = (100 * lumin) / 256;

    rmod = max(rscale, rmod);

    if (fAdjustGreen) {
      gmod = max(gscale, gmod);
    }

    r = (uint8_t)min(rmod, 255);
    g = (uint8_t)min(gmod, 255);
    b = (uint8_t)min(bmod, 255);

    usColor = JVideo_PackRGB16(r, g, b);

    // Prevent creation of pure black color
    if ((usColor == 0) && ((r + g + b) != 0)) usColor = 0x0001;

    p16BPPPalette[cnt] = usColor;
  }
  return (p16BPPPalette);
}

void ContinueMercMovement(struct SOLDIERTYPE *pSoldier) {
  int16_t sAPCost;
  int16_t sGridNo;

  sGridNo = pSoldier->sFinalDestination;

  // Can we afford this?
  if (pSoldier->bGoodContPath) {
    sGridNo = pSoldier->sContPathLocation;
  } else {
    // ATE: OK, don't cancel count, so pending actions are still valid...
    pSoldier->ubPendingActionAnimCount = 0;
  }

  // get a path to dest...
  if (FindBestPath(pSoldier, sGridNo, pSoldier->bLevel, pSoldier->usUIMovementMode, NO_COPYROUTE,
                   0)) {
    sAPCost = PtsToMoveDirection(pSoldier, (uint8_t)guiPathingData[0]);

    if (EnoughPoints(pSoldier, sAPCost, 0, (BOOLEAN)(pSoldier->bTeam == gbPlayerNum))) {
      // Acknowledge
      if (pSoldier->bTeam == gbPlayerNum) {
        DoMercBattleSound(pSoldier, BATTLE_SOUND_OK1);

        // If we have a face, tell text in it to go away!
        if (pSoldier->iFaceIndex != -1) {
          gFacesData[pSoldier->iFaceIndex].fDisplayTextOver = FACE_ERASE_TEXT_OVER;
        }
      }

      AdjustNoAPToFinishMove(pSoldier, FALSE);

      SetUIBusy(pSoldier->ubID);

      // OK, try and get a path to out dest!
      EVENT_InternalGetNewSoldierPath(pSoldier, sGridNo, pSoldier->usUIMovementMode, FALSE, TRUE);
    }
  }
}

BOOLEAN CheckForBreathCollapse(struct SOLDIERTYPE *pSoldier) {
  // Check if we are out of breath!
  // Only check if > 70
  if (pSoldier->bBreathMax > 70) {
    if (pSoldier->bBreath < 20 && !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_LOW_BREATH) &&
        gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_STAND) {
      // WARN!
      TacticalCharacterDialogue(pSoldier, QUOTE_OUT_OF_BREATH);

      // Set flag indicating we were warned!
      pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_LOW_BREATH;
    }
  }

  // Check for drowing.....
  // if ( pSoldier->bBreath < 10 && !(pSoldier->usQuoteSaidFlags & SOLDIER_QUOTE_SAID_DROWNING ) &&
  // pSoldier->bOverTerrainType == DEEP_WATER )
  //{
  // WARN!
  //	TacticalCharacterDialogue( pSoldier, QUOTE_DROWNING );

  // Set flag indicating we were warned!
  //	pSoldier->usQuoteSaidFlags |= SOLDIER_QUOTE_SAID_DROWNING;

  // WISDOM GAIN (25):  Starting to drown
  //  StatChange( pSoldier, WISDOMAMT, 25, FALSE );

  //}

  if (pSoldier->bBreath == 0 && !pSoldier->bCollapsed &&
      !(pSoldier->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ANIMAL | SOLDIER_MONSTER))) {
    // Collapse!
    // OK, Set a flag, because we may still be in the middle of an animation what is not
    // interruptable...
    pSoldier->bBreathCollapsed = TRUE;

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN InternalIsValidStance(struct SOLDIERTYPE *pSoldier, int8_t bDirection, int8_t bNewStance) {
  uint16_t usOKToAddStructID = 0;
  struct STRUCTURE_FILE_REF *pStructureFileRef;
  uint16_t usAnimSurface = 0;
  uint16_t usAnimState;

  // Check, if dest is prone, we can actually do this!

  // If we are a vehicle, we can only 'stand'
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && bNewStance != ANIM_STAND) {
    return (FALSE);
  }

  // Check if we are in water?
  if (MercInWater(pSoldier)) {
    if (bNewStance == ANIM_PRONE || bNewStance == ANIM_CROUCH) {
      return (FALSE);
    }
  }

  if (pSoldier->ubBodyType == ROBOTNOWEAPON && bNewStance != ANIM_STAND) {
    return (FALSE);
  }

  // Check if we are in water?
  if (AM_AN_EPC(pSoldier)) {
    if (bNewStance == ANIM_PRONE) {
      return (FALSE);
    } else {
      return (TRUE);
    }
  }

  if (pSoldier->bCollapsed) {
    if (bNewStance == ANIM_STAND || bNewStance == ANIM_CROUCH) {
      return (FALSE);
    }
  }

  // Check if we can do this....
  if (pSoldier->pLevelNode && pSoldier->pLevelNode->pStructureData != NULL) {
    usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
  } else {
    usOKToAddStructID = INVALID_STRUCTURE_ID;
  }

  switch (bNewStance) {
    case ANIM_STAND:

      usAnimState = STANDING;
      break;

    case ANIM_CROUCH:

      usAnimState = CROUCHING;
      break;

    case ANIM_PRONE:

      usAnimState = PRONE;
      break;

    default:

      // Something gone funny here....
      usAnimState = pSoldier->usAnimState;
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_BETAVERSION, L"Wrong desired stance given: %d, %d.",
                bNewStance, pSoldier->usAnimState);
  }

  usAnimSurface = DetermineSoldierAnimationSurface(pSoldier, usAnimState);

  // Get structure ref........
  pStructureFileRef = GetAnimationStructureRef(pSoldier->ubID, usAnimSurface, usAnimState);

  if (pStructureFileRef != NULL) {
    // Can we add structure data for this stance...?
    if (!OkayToAddStructureToWorld(
            pSoldier->sGridNo, pSoldier->bLevel,
            &(pStructureFileRef->pDBStructureRef[gOneCDirection[bDirection]]), usOKToAddStructID)) {
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN IsValidStance(struct SOLDIERTYPE *pSoldier, int8_t bNewStance) {
  return (InternalIsValidStance(pSoldier, pSoldier->bDirection, bNewStance));
}

BOOLEAN IsValidMovementMode(struct SOLDIERTYPE *pSoldier, int16_t usMovementMode) {
  // Check, if dest is prone, we can actually do this!

  // Check if we are in water?
  if (MercInWater(pSoldier)) {
    if (usMovementMode == RUNNING || usMovementMode == SWATTING || usMovementMode == CRAWLING) {
      return (FALSE);
    }
  }

  return (TRUE);
}

void SelectMoveAnimationFromStance(struct SOLDIERTYPE *pSoldier) {
  // Determine which animation to do...depending on stance and gun in hand...
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:
      EVENT_InitNewSoldierAnim(pSoldier, WALKING, 0, FALSE);
      break;

    case ANIM_PRONE:
      EVENT_InitNewSoldierAnim(pSoldier, CRAWLING, 0, FALSE);
      break;

    case ANIM_CROUCH:
      EVENT_InitNewSoldierAnim(pSoldier, SWATTING, 0, FALSE);
      break;
  }
}

void GetActualSoldierAnimDims(struct SOLDIERTYPE *pSoldier, int16_t *psHeight, int16_t *psWidth) {
  uint16_t usAnimSurface;
  ETRLEObject *pTrav;

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    *psHeight = (int16_t)5;
    *psWidth = (int16_t)5;

    return;
  }

  if (gAnimSurfaceDatabase[usAnimSurface].hVideoObject == NULL) {
    *psHeight = (int16_t)5;
    *psWidth = (int16_t)5;
    return;
  }

  // OK, noodle here on what we should do... If we take each frame, it will be different slightly
  // depending on the frame and the value returned here will vary thusly. However, for the
  // uses of this function, we should be able to use just the first frame...

  if (pSoldier->usAniFrame >= gAnimSurfaceDatabase[usAnimSurface].hVideoObject->usNumberOfObjects) {
  }

  pTrav = &(gAnimSurfaceDatabase[usAnimSurface].hVideoObject->pETRLEObject[pSoldier->usAniFrame]);

  *psHeight = (int16_t)pTrav->usHeight;
  *psWidth = (int16_t)pTrav->usWidth;
}

void GetActualSoldierAnimOffsets(struct SOLDIERTYPE *pSoldier, int16_t *sOffsetX,
                                 int16_t *sOffsetY) {
  uint16_t usAnimSurface;
  ETRLEObject *pTrav;

  usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);

  if (usAnimSurface == INVALID_ANIMATION_SURFACE) {
    *sOffsetX = (int16_t)0;
    *sOffsetY = (int16_t)0;

    return;
  }

  if (gAnimSurfaceDatabase[usAnimSurface].hVideoObject == NULL) {
    *sOffsetX = (int16_t)0;
    *sOffsetY = (int16_t)0;
    return;
  }

  pTrav = &(gAnimSurfaceDatabase[usAnimSurface].hVideoObject->pETRLEObject[pSoldier->usAniFrame]);

  *sOffsetX = (int16_t)pTrav->sOffsetX;
  *sOffsetY = (int16_t)pTrav->sOffsetY;
}

void SetSoldierLocatorOffsets(struct SOLDIERTYPE *pSoldier) {
  int16_t sHeight, sWidth;
  int16_t sOffsetX, sOffsetY;

  // OK, from our animation, get height, width
  GetActualSoldierAnimDims(pSoldier, &sHeight, &sWidth);
  GetActualSoldierAnimOffsets(pSoldier, &sOffsetX, &sOffsetY);

  // OK, here, use the difference between center of animation ( sWidth/2 ) and our offset!
  // pSoldier->sLocatorOffX = ( abs( sOffsetX ) ) - ( sWidth / 2 );

  pSoldier->sBoundingBoxWidth = sWidth;
  pSoldier->sBoundingBoxHeight = sHeight;
  pSoldier->sBoundingBoxOffsetX = sOffsetX;
  pSoldier->sBoundingBoxOffsetY = sOffsetY;
}

BOOLEAN SoldierCarriesTwoHandedWeapon(struct SOLDIERTYPE *pSoldier) {
  uint16_t usItem;

  usItem = pSoldier->inv[HANDPOS].usItem;

  if (usItem != NOTHING && (Item[usItem].fFlags & ITEM_TWO_HANDED)) {
    return (TRUE);
  }

  return (FALSE);
}

int32_t CheckBleeding(struct SOLDIERTYPE *pSoldier) {
  int8_t bBandaged;  //,savedOurTurn;
  int32_t iBlood = NOBLOOD;

  if (pSoldier->bLife != 0) {
    // if merc is hurt beyond the minimum required to bleed, or he's dying
    if ((pSoldier->bBleeding > MIN_BLEEDING_THRESHOLD) || pSoldier->bLife < OKLIFE) {
      // if he's NOT in the process of being bandaged or DOCTORed
      if ((pSoldier->ubServiceCount == 0) &&
          (AnyDoctorWhoCanHealThisPatient(pSoldier, HEALABLE_EVER) == NULL)) {
        // may drop blood whether or not any bleeding takes place this turn
        if (pSoldier->bTilesMoved < 1) {
          iBlood = ((pSoldier->bBleeding - MIN_BLEEDING_THRESHOLD) /
                    BLOODDIVISOR);  // + pSoldier->dying;
          if (iBlood > MAXBLOODQUANTITY) {
            iBlood = MAXBLOODQUANTITY;
          }
        } else {
          iBlood = NOBLOOD;
        }

        // Are we in a different mode?
        if (!(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT)) {
          pSoldier->dNextBleed -= (float)RT_NEXT_BLEED_MODIFIER;
        } else {
          // Do a single step descrease
          pSoldier->dNextBleed--;
        }

        // if it's time to lose some blood
        if (pSoldier->dNextBleed <= 0) {
          // first, calculate if soldier is bandaged
          bBandaged = pSoldier->bLifeMax - pSoldier->bBleeding - pSoldier->bLife;

          // as long as he's bandaged and not "dying"
          if (bBandaged && pSoldier->bLife >= OKLIFE) {
            // just bleeding through existing bandages
            pSoldier->bBleeding++;

            SoldierBleed(pSoldier, TRUE);
          } else  // soldier is either not bandaged at all or is dying
          {
            if (pSoldier->bLife < OKLIFE)  // if he's dying
            {
              // if he's conscious, and he hasn't already, say his "dying quote"
              if ((pSoldier->bLife >= CONSCIOUSNESS) && !pSoldier->fDyingComment) {
                TacticalCharacterDialogue(pSoldier, QUOTE_SERIOUSLY_WOUNDED);

                pSoldier->fDyingComment = TRUE;
              }

              // can't permit lifemax to ever bleed beneath OKLIFE, or that
              // soldier might as well be dead!
              if (pSoldier->bLifeMax >= OKLIFE) {
                // bleeding while "dying" costs a PERMANENT point of life each time!
                pSoldier->bLifeMax--;
                pSoldier->bBleeding--;
              }
            }
          }

          // either way, a point of life (health) is lost because of bleeding
          // This will also update the life bar

          SoldierBleed(pSoldier, FALSE);

          // if he's not dying (which includes him saying the dying quote just
          // now), and he hasn't warned us that he's bleeding yet, he does so
          // Also, not if they are being bandaged....
          if ((pSoldier->bLife >= OKLIFE) && !pSoldier->fDyingComment &&
              !pSoldier->fWarnedAboutBleeding && !gTacticalStatus.fAutoBandageMode &&
              pSoldier->ubServiceCount == 0) {
            TacticalCharacterDialogue(pSoldier, QUOTE_STARTING_TO_BLEED);

            // "starting to bleed" quote
            pSoldier->fWarnedAboutBleeding = TRUE;
          }

          pSoldier->dNextBleed = CalcSoldierNextBleed(pSoldier);
        }
      }
    }
  }
  return (iBlood);
}

void SoldierBleed(struct SOLDIERTYPE *pSoldier, BOOLEAN fBandagedBleed) {
  // OK, here make some stuff happen for bleeding
  // A banaged bleed does not show damage taken , just through existing bandages

  // ATE: Do this ONLY if buddy is in sector.....
  if ((pSoldier->bInSector && IsTacticalMode()) || guiCurrentScreen != GAME_SCREEN) {
    pSoldier->fFlashPortrait = TRUE;
    pSoldier->bFlashPortraitFrame = FLASH_PORTRAIT_STARTSHADE;
    RESETTIMECOUNTER(pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY);

    // If we are in mapscreen, set this person as selected
    if (IsMapScreen_2()) {
      SetInfoChar(pSoldier->ubID);
    }
  }

  // If we are already dead, don't show damage!
  if (!fBandagedBleed) {
    SoldierTakeDamage(pSoldier, ANIM_CROUCH, 1, 100, TAKE_DAMAGE_BLOODLOSS, NOBODY, NOWHERE, 0,
                      TRUE);
  }
}

void SoldierCollapse(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fMerc = FALSE;

  if (pSoldier->ubBodyType <= REGFEMALE) {
    fMerc = TRUE;
  }

  // If we are an animal, etc, don't do anything....
  switch (pSoldier->ubBodyType) {
    case ADULTFEMALEMONSTER:
    case AM_MONSTER:
    case YAF_MONSTER:
    case YAM_MONSTER:
    case LARVAE_MONSTER:
    case INFANT_MONSTER:
    case QUEENMONSTER:

      // Give breath back....
      DeductPoints(pSoldier, 0, (int16_t)-5000);
      return;
      break;
  }

  pSoldier->bCollapsed = TRUE;

  ReceivingSoldierCancelServices(pSoldier);

  // CC has requested - handle sight here...
  HandleSight(pSoldier, SIGHT_LOOK);

  // Check height
  switch (gAnimControl[pSoldier->usAnimState].ubEndHeight) {
    case ANIM_STAND:

      if (pSoldier->bOverTerrainType == DEEP_WATER) {
        EVENT_InitNewSoldierAnim(pSoldier, DEEP_WATER_DIE, 0, FALSE);
      } else if (pSoldier->bOverTerrainType == LOW_WATER) {
        EVENT_InitNewSoldierAnim(pSoldier, WATER_DIE, 0, FALSE);
      } else {
        BeginTyingToFall(pSoldier);
        EVENT_InitNewSoldierAnim(pSoldier, FALLFORWARD_FROMHIT_STAND, 0, FALSE);
      }
      break;

    case ANIM_CROUCH:

      // Crouched or prone, only for mercs!
      BeginTyingToFall(pSoldier);

      if (fMerc) {
        EVENT_InitNewSoldierAnim(pSoldier, FALLFORWARD_FROMHIT_CROUCH, 0, FALSE);
      } else {
        // For civs... use fall from stand...
        EVENT_InitNewSoldierAnim(pSoldier, FALLFORWARD_FROMHIT_STAND, 0, FALSE);
      }
      break;

    case ANIM_PRONE:

      switch (pSoldier->usAnimState) {
        case FALLFORWARD_FROMHIT_STAND:
        case ENDFALLFORWARD_FROMHIT_CROUCH:

          ChangeSoldierState(pSoldier, STAND_FALLFORWARD_STOP, 0, FALSE);
          break;

        case FALLBACK_HIT_STAND:
          ChangeSoldierState(pSoldier, FALLBACKHIT_STOP, 0, FALSE);
          break;

        default:
          EVENT_InitNewSoldierAnim(pSoldier, PRONE_LAY_FROMHIT, 0, FALSE);
          break;
      }
      break;
  }

  if (pSoldier->uiStatusFlags & SOLDIER_ENEMY) {
    if (!(gTacticalStatus.bPanicTriggerIsAlarm) &&
        (gTacticalStatus.ubTheChosenOne == GetSolID(pSoldier))) {
      // replace this guy as the chosen one!
      gTacticalStatus.ubTheChosenOne = NOBODY;
      MakeClosestEnemyChosenOne();
    }

    if ((gTacticalStatus.uiFlags & TURNBASED) && (gTacticalStatus.uiFlags & INCOMBAT) &&
        (pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL)) {
#ifdef TESTAICONTROL
      DebugAI(String("Ending turn for %d because of error from HandleItem", GetSolID(pSoldier)));
#endif

      EndAIGuysTurn(pSoldier);
    }
  }

  // DON'T DE-SELECT GUY.....
  // else
  //{
  // Check if this is our selected guy...
  //	if ( GetSolID(pSoldier) == gusSelectedSoldier )
  //	{
  //		SelectNextAvailSoldier( pSoldier );
  //		}
  //}
}

float CalcSoldierNextBleed(struct SOLDIERTYPE *pSoldier) {
  int8_t bBandaged;

  // calculate how many turns before he bleeds again
  // bleeding faster the lower life gets, and if merc is running around
  // pSoldier->nextbleed = 2 + (pSoldier->life / (10 + pSoldier->tilesMoved));  // min = 2

  // if bandaged, give 1/2 of the bandaged life points back into equation
  bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

  return ((float)1 +
          (float)((pSoldier->bLife + bBandaged / 2) / (10 + pSoldier->bTilesMoved)));  // min = 1
}

float CalcSoldierNextUnmovingBleed(struct SOLDIERTYPE *pSoldier) {
  int8_t bBandaged;

  // calculate bleeding rate without the penalty for tiles moved

  // if bandaged, give 1/2 of the bandaged life points back into equation
  bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

  return ((float)1 + (float)((pSoldier->bLife + bBandaged / 2) / 10));  // min = 1
}

void HandlePlacingRoofMarker(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, BOOLEAN fSet,
                             BOOLEAN fForce) {
  struct LEVELNODE *pRoofNode;
  struct LEVELNODE *pNode;

  if (pSoldier->bVisible == -1 && fSet) {
    return;
  }

  if (pSoldier->bTeam != gbPlayerNum) {
    // return;
  }

  // If we are on the roof, add roof UI peice!
  if (pSoldier->bLevel == SECOND_LEVEL) {
    // Get roof node
    pRoofNode = gpWorldLevelData[sGridNo].pRoofHead;

    // Return if we are still climbing roof....
    if (pSoldier->usAnimState == CLIMBUPROOF && !fForce) {
      return;
    }

    if (pRoofNode != NULL) {
      if (fSet) {
        if (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) {
          // Set some flags on this poor thing
          // pRoofNode->uiFlags |= ( LEVELNODE_USEBESTTRANSTYPE | LEVELNODE_REVEAL |
          // LEVELNODE_DYNAMIC  ); pRoofNode->uiFlags |= ( LEVELNODE_DYNAMIC ); pRoofNode->uiFlags
          // &= ( ~LEVELNODE_HIDDEN ); ResetSpecificLayerOptimizing( TILES_DYNAMIC_ROOF );
        }
      } else {
        if (gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REVEALED) {
          // Remove some flags on this poor thing
          // pRoofNode->uiFlags &= ~( LEVELNODE_USEBESTTRANSTYPE | LEVELNODE_REVEAL |
          // LEVELNODE_DYNAMIC );

          // pRoofNode->uiFlags |= LEVELNODE_HIDDEN;
        }
      }

      if (fSet) {
        // If it does not exist already....
        if (!IndexExistsInRoofLayer(sGridNo, FIRSTPOINTERS11)) {
          pNode = AddRoofToTail(sGridNo, FIRSTPOINTERS11);
          pNode->ubShadeLevel = DEFAULT_SHADE_LEVEL;
          pNode->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
        }
      } else {
        RemoveRoof(sGridNo, FIRSTPOINTERS11);
      }
    }
  }
}

void PositionSoldierLight(struct SOLDIERTYPE *pSoldier) {
  // DO ONLY IF WE'RE AT A GOOD LEVEL
  if (ubAmbientLightLevel < MIN_AMB_LEVEL_FOR_MERC_LIGHTS) {
    return;
  }

  if (!pSoldier->bInSector) {
    return;
  }

  if (pSoldier->bTeam != gbPlayerNum) {
    return;
  }

  if (pSoldier->bLife < OKLIFE) {
    return;
  }

  // if the player DOESNT want the merc to cast light
  if (!gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT]) {
    return;
  }

  if (pSoldier->iLight == -1) {
    CreateSoldierLight(pSoldier);
  }

  // if ( GetSolID(pSoldier) == gusSelectedSoldier )
  {
    LightSpritePower(pSoldier->iLight, TRUE);
    LightSpriteFake(pSoldier->iLight);

    LightSpritePosition(pSoldier->iLight, (int16_t)(pSoldier->sX / CELL_X_SIZE),
                        (int16_t)(pSoldier->sY / CELL_Y_SIZE));
  }
}

void SetCheckSoldierLightFlag(struct SOLDIERTYPE *pSoldier) {
  PositionSoldierLight(pSoldier);
  // pSoldier->uiStatusFlags |= SOLDIER_RECHECKLIGHT;
}

void PickPickupAnimation(struct SOLDIERTYPE *pSoldier, int32_t iItemIndex, int16_t sGridNo,
                         int8_t bZLevel) {
  int8_t bDirection;
  struct STRUCTURE *pStructure;
  BOOLEAN fDoNormalPickup = TRUE;

  // OK, Given the gridno, determine if it's the same one or different....
  if (sGridNo != pSoldier->sGridNo) {
    // Get direction to face....
    bDirection = (int8_t)GetDirectionFromGridNo(sGridNo, pSoldier);
    pSoldier->ubPendingDirection = bDirection;

    // Change to pickup animation
    EVENT_InitNewSoldierAnim(pSoldier, ADJACENT_GET_ITEM, 0, FALSE);

    if (!(pSoldier->uiStatusFlags & SOLDIER_PC)) {
      // set "pending action" value for AI so it will wait
      pSoldier->bAction = AI_ACTION_PENDING_ACTION;
    }

  } else {
    // If in water....
    if (MercInWater(pSoldier)) {
      UnSetUIBusy(pSoldier->ubID);
      HandleSoldierPickupItem(pSoldier, iItemIndex, sGridNo, bZLevel);
      SoldierGotoStationaryStance(pSoldier);
      if (!(pSoldier->uiStatusFlags & SOLDIER_PC)) {
        // reset action value for AI because we're done!
        ActionDone(pSoldier);
      }

    } else {
      // Don't show animation of getting item, if we are not standing
      switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
        case ANIM_STAND:

          // OK, if we are looking at z-level >0, AND
          // we have a strucxture with items in it
          // look for orientation and use angle accordingly....
          if (bZLevel > 0) {
            // #if 0
            //  Get direction to face....
            if ((pStructure = FindStructure(
                     (int16_t)sGridNo, (STRUCTURE_HASITEMONTOP | STRUCTURE_OPENABLE))) != NULL) {
              fDoNormalPickup = FALSE;

              // OK, look at orientation
              switch (pStructure->ubWallOrientation) {
                case OUTSIDE_TOP_LEFT:
                case INSIDE_TOP_LEFT:

                  bDirection = (int8_t)NORTH;
                  break;

                case OUTSIDE_TOP_RIGHT:
                case INSIDE_TOP_RIGHT:

                  bDirection = (int8_t)WEST;
                  break;

                default:

                  bDirection = pSoldier->bDirection;
                  break;
              }

              // pSoldier->ubPendingDirection = bDirection;
              EVENT_SetSoldierDesiredDirection(pSoldier, bDirection);
              EVENT_SetSoldierDirection(pSoldier, bDirection);

              // Change to pickup animation
              EVENT_InitNewSoldierAnim(pSoldier, ADJACENT_GET_ITEM, 0, FALSE);
            }
            // #endif
          }

          if (fDoNormalPickup) {
            EVENT_InitNewSoldierAnim(pSoldier, PICKUP_ITEM, 0, FALSE);
          }

          if (!(pSoldier->uiStatusFlags & SOLDIER_PC)) {
            // set "pending action" value for AI so it will wait
            pSoldier->bAction = AI_ACTION_PENDING_ACTION;
          }
          break;

        case ANIM_CROUCH:
        case ANIM_PRONE:

          UnSetUIBusy(pSoldier->ubID);
          HandleSoldierPickupItem(pSoldier, iItemIndex, sGridNo, bZLevel);
          SoldierGotoStationaryStance(pSoldier);
          if (!(pSoldier->uiStatusFlags & SOLDIER_PC)) {
            // reset action value for AI because we're done!
            ActionDone(pSoldier);
          }
          break;
      }
    }
  }
}

void PickDropItemAnimation(struct SOLDIERTYPE *pSoldier) {
  // Don't show animation of getting item, if we are not standing
  switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
    case ANIM_STAND:

      EVENT_InitNewSoldierAnim(pSoldier, DROP_ITEM, 0, FALSE);
      break;

    case ANIM_CROUCH:
    case ANIM_PRONE:

      SoldierHandleDropItem(pSoldier);
      SoldierGotoStationaryStance(pSoldier);
      break;
  }
}

void EVENT_SoldierBeginCutFence(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                uint8_t ubDirection) {
  // Make sure we have a structure here....
  if (IsCuttableWireFenceAtGridNo(sGridNo)) {
    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);

    // BOOLEAN CutWireFence( int16_t sGridNo )

    // SET TARGET GRIDNO
    pSoldier->sTargetGridNo = sGridNo;

    // CHANGE TO ANIMATION
    EVENT_InitNewSoldierAnim(pSoldier, CUTTING_FENCE, 0, FALSE);
  }
}

void EVENT_SoldierBeginRepair(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubDirection) {
  int8_t bRepairItem;
  uint8_t ubID;

  // Make sure we have a structure here....
  bRepairItem = IsRepairableStructAtGridNo(sGridNo, &ubID);

  if (bRepairItem) {
    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);

    // BOOLEAN CutWireFence( int16_t sGridNo )

    // SET TARGET GRIDNO
    // pSoldier->sTargetGridNo = sGridNo;

    // CHANGE TO ANIMATION
    EVENT_InitNewSoldierAnim(pSoldier, GOTO_REPAIRMAN, 0, FALSE);
    // SET BUDDY'S ASSIGNMENT TO REPAIR...

    // Are we a SAM site? ( 3 == SAM )
    if (bRepairItem == 3) {
      SetSoldierAssignment(pSoldier, REPAIR, TRUE, FALSE, -1);
    } else if (bRepairItem == 2)  // ( 2 == VEHICLE )
    {
      SetSoldierAssignment(pSoldier, REPAIR, FALSE, FALSE, ubID);
    }
  }
}

void EVENT_SoldierBeginRefuel(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubDirection) {
  int8_t bRefuelItem;
  uint8_t ubID;

  // Make sure we have a structure here....
  bRefuelItem = IsRefuelableStructAtGridNo(sGridNo, &ubID);

  if (bRefuelItem) {
    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);

    // BOOLEAN CutWireFence( int16_t sGridNo )

    // SET TARGET GRIDNO
    // pSoldier->sTargetGridNo = sGridNo;

    // CHANGE TO ANIMATION
    EVENT_InitNewSoldierAnim(pSoldier, REFUEL_VEHICLE, 0, FALSE);
    // SET BUDDY'S ASSIGNMENT TO REPAIR...
  }
}

void EVENT_SoldierBeginTakeBlood(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                 uint8_t ubDirection) {
  ROTTING_CORPSE *pCorpse;

  // See if these is a corpse here....
  pCorpse = GetCorpseAtGridNo(sGridNo, pSoldier->bLevel);

  if (pCorpse != NULL) {
    pSoldier->uiPendingActionData4 = pCorpse->iID;

    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);

    EVENT_InitNewSoldierAnim(pSoldier, TAKE_BLOOD_FROM_CORPSE, 0, FALSE);
  } else {
    // Say NOTHING quote...
    DoMercBattleSound(pSoldier, BATTLE_SOUND_NOTHING);
  }
}

void EVENT_SoldierBeginAttachCan(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                 uint8_t ubDirection) {
  struct STRUCTURE *pStructure;
  DOOR_STATUS *pDoorStatus;

  // OK, find door, attach to door, do animation...., remove item....

  // First make sure we still have item in hand....
  if (pSoldier->inv[HANDPOS].usItem != STRING_TIED_TO_TIN_CAN) {
    return;
  }

  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);

  if (pStructure == NULL) {
    return;
  }

  // Modify door status to make sure one is created for this door
  // Use the current door state for this
  if (!(pStructure->fFlags & STRUCTURE_OPEN)) {
    ModifyDoorStatus(sGridNo, FALSE, FALSE);
  } else {
    ModifyDoorStatus(sGridNo, TRUE, TRUE);
  }

  // Now get door status...
  pDoorStatus = GetDoorStatus(sGridNo);
  if (pDoorStatus == NULL) {
    // SOmething wrong here...
    return;
  }

  // OK set flag!
  pDoorStatus->ubFlags |= DOOR_HAS_TIN_CAN;

  // Do animation
  EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
  EVENT_SetSoldierDirection(pSoldier, ubDirection);

  EVENT_InitNewSoldierAnim(pSoldier, ATTACH_CAN_TO_STRING, 0, FALSE);

  // Remove item...
  DeleteObj(&(pSoldier->inv[HANDPOS]));
  fInterfacePanelDirty = DIRTYLEVEL2;
}

void EVENT_SoldierBeginReloadRobot(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                   uint8_t ubDirection, uint8_t ubMercSlot) {
  uint8_t ubPerson;

  // Make sure we have a robot here....
  ubPerson = WhoIsThere2(sGridNo, pSoldier->bLevel);

  if (ubPerson != NOBODY && MercPtrs[ubPerson]->uiStatusFlags & SOLDIER_ROBOT) {
    // CHANGE DIRECTION AND GOTO ANIMATION NOW
    EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
    EVENT_SetSoldierDirection(pSoldier, ubDirection);

    // CHANGE TO ANIMATION
    EVENT_InitNewSoldierAnim(pSoldier, RELOAD_ROBOT, 0, FALSE);
  }
}

void ResetSoldierChangeStatTimer(struct SOLDIERTYPE *pSoldier) {
  pSoldier->uiChangeLevelTime = 0;
  pSoldier->uiChangeHealthTime = 0;
  pSoldier->uiChangeStrengthTime = 0;
  pSoldier->uiChangeDexterityTime = 0;
  pSoldier->uiChangeAgilityTime = 0;
  pSoldier->uiChangeWisdomTime = 0;
  pSoldier->uiChangeLeadershipTime = 0;
  pSoldier->uiChangeMarksmanshipTime = 0;
  pSoldier->uiChangeExplosivesTime = 0;
  pSoldier->uiChangeMedicalTime = 0;
  pSoldier->uiChangeMechanicalTime = 0;

  return;
}

void ChangeToFlybackAnimation(struct SOLDIERTYPE *pSoldier, int8_t bDirection) {
  uint16_t usNewGridNo;

  // Get dest gridno, convert to center coords
  usNewGridNo =
      NewGridNo((uint16_t)pSoldier->sGridNo, DirectionInc(gOppositeDirection[bDirection]));
  usNewGridNo = NewGridNo((uint16_t)usNewGridNo, DirectionInc(gOppositeDirection[bDirection]));

  // Remove any previous actions
  pSoldier->ubPendingAction = NO_PENDING_ACTION;

  // Set path....
  pSoldier->usPathDataSize = 0;
  pSoldier->usPathIndex = 0;
  pSoldier->usPathingData[pSoldier->usPathDataSize] = gOppositeDirection[pSoldier->bDirection];
  pSoldier->usPathDataSize++;
  pSoldier->usPathingData[pSoldier->usPathDataSize] = gOppositeDirection[pSoldier->bDirection];
  pSoldier->usPathDataSize++;
  pSoldier->sFinalDestination = usNewGridNo;
  EVENT_InternalSetSoldierDestination(pSoldier, pSoldier->usPathingData[pSoldier->usPathIndex],
                                      FALSE, FLYBACK_HIT);

  // Get a new direction based on direction
  EVENT_InitNewSoldierAnim(pSoldier, FLYBACK_HIT, 0, FALSE);
}

void ChangeToFallbackAnimation(struct SOLDIERTYPE *pSoldier, int8_t bDirection) {
  uint16_t usNewGridNo;

  // Get dest gridno, convert to center coords
  usNewGridNo =
      NewGridNo((uint16_t)pSoldier->sGridNo, DirectionInc(gOppositeDirection[bDirection]));
  // usNewGridNo = NewGridNo( (uint16_t)usNewGridNo, (uint16_t)(-1 * DirectionInc( bDirection ) ) );

  // Remove any previous actions
  pSoldier->ubPendingAction = NO_PENDING_ACTION;

  // Set path....
  pSoldier->usPathDataSize = 0;
  pSoldier->usPathIndex = 0;
  pSoldier->usPathingData[pSoldier->usPathDataSize] = gOppositeDirection[pSoldier->bDirection];
  pSoldier->usPathDataSize++;
  pSoldier->sFinalDestination = usNewGridNo;
  EVENT_InternalSetSoldierDestination(pSoldier, pSoldier->usPathingData[pSoldier->usPathIndex],
                                      FALSE, FALLBACK_HIT_STAND);

  // Get a new direction based on direction
  EVENT_InitNewSoldierAnim(pSoldier, FALLBACK_HIT_STAND, 0, FALSE);
}

void SetSoldierCowerState(struct SOLDIERTYPE *pSoldier, BOOLEAN fOn) {
  // Robot's don't cower!
  if (pSoldier->ubBodyType == ROBOTNOWEAPON) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("ERROR: Robot was told to cower!"));
    return;
  }

  // OK< set flag and do anim...
  if (fOn) {
    if (!(pSoldier->uiStatusFlags & SOLDIER_COWERING)) {
      EVENT_InitNewSoldierAnim(pSoldier, START_COWER, 0, FALSE);

      pSoldier->uiStatusFlags |= SOLDIER_COWERING;

      pSoldier->ubDesiredHeight = ANIM_CROUCH;
    }
  } else {
    if ((pSoldier->uiStatusFlags & SOLDIER_COWERING)) {
      EVENT_InitNewSoldierAnim(pSoldier, END_COWER, 0, FALSE);

      pSoldier->uiStatusFlags &= (~SOLDIER_COWERING);

      pSoldier->ubDesiredHeight = ANIM_STAND;
    }
  }
}

void MercStealFromMerc(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTarget) {
  int16_t sActionGridNo, sGridNo, sAdjustedGridNo;
  uint8_t ubDirection;

  // OK, find an adjacent gridno....
  sGridNo = pTarget->sGridNo;

  // See if we can get there to punch
  sActionGridNo =
      FindAdjacentGridEx(pSoldier, sGridNo, &ubDirection, &sAdjustedGridNo, TRUE, FALSE);
  if (sActionGridNo != -1) {
    // SEND PENDING ACTION
    pSoldier->ubPendingAction = MERC_STEAL;
    pSoldier->sPendingActionData2 = pTarget->sGridNo;
    pSoldier->bPendingActionData3 = ubDirection;
    pSoldier->ubPendingActionAnimCount = 0;

    // CHECK IF WE ARE AT THIS GRIDNO NOW
    if (pSoldier->sGridNo != sActionGridNo) {
      // WALK UP TO DEST FIRST
      SendGetNewSoldierPathEvent(pSoldier, sActionGridNo, pSoldier->usUIMovementMode);
    } else {
      EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);
      EVENT_InitNewSoldierAnim(pSoldier, STEAL_ITEM, 0, FALSE);
    }

    // OK, set UI
    gTacticalStatus.ubAttackBusyCount++;
    // reset attacking item (hand)
    pSoldier->usAttackingWeapon = 0;
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("!!!!!!! Starting STEAL attack, attack count now %d",
                    gTacticalStatus.ubAttackBusyCount));

    SetUIBusy(pSoldier->ubID);
  }
}

BOOLEAN PlayerSoldierStartTalking(struct SOLDIERTYPE *pSoldier, uint8_t ubTargetID,
                                  BOOLEAN fValidate) {
  int16_t sFacingDir, sXPos, sYPos, sAPCost;
  struct SOLDIERTYPE *pTSoldier;
  uint32_t uiRange;

  if (ubTargetID == NOBODY) {
    return (FALSE);
  }

  pTSoldier = MercPtrs[ubTargetID];

  // Check distance again, to be sure
  if (fValidate) {
    // OK, since we locked this guy from moving
    // we should be close enough, so talk ( unless he is now dead )
    if (!IsValidTalkableNPC((uint8_t)ubTargetID, FALSE, FALSE, FALSE)) {
      return (FALSE);
    }

    uiRange = GetRangeFromGridNoDiff(pSoldier->sGridNo, pTSoldier->sGridNo);

    if (uiRange > (NPC_TALK_RADIUS * 2)) {
      // Todo here - should we follow dude?
      return (FALSE);
    }
  }

  // Get APs...
  sAPCost = AP_TALK;

  // Deduct points from our guy....
  DeductPoints(pSoldier, sAPCost, 0);

  ConvertGridNoToXY(pTSoldier->sGridNo, &sXPos, &sYPos);

  // Get direction from mouse pos
  sFacingDir = GetDirectionFromXY(sXPos, sYPos, pSoldier);

  // Set our guy facing
  SendSoldierSetDesiredDirectionEvent(pSoldier, sFacingDir);

  // Set NPC facing
  SendSoldierSetDesiredDirectionEvent(pTSoldier, gOppositeDirection[sFacingDir]);

  // Stop our guys...
  EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

  // ATE; Check for normal civs...
  if (GetCivType(pTSoldier) != CIV_TYPE_NA) {
    StartCivQuote(pTSoldier);
    return (FALSE);
  }

  // Are we an EPC that is being escorted?
  if (pTSoldier->ubProfile != NO_PROFILE && pTSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    return (InitiateConversation(pTSoldier, pSoldier, APPROACH_EPC_WHO_IS_RECRUITED, 0));
  } else if (pTSoldier->bNeutral) {
    switch (pTSoldier->ubProfile) {
      case JIM:
      case JACK:
      case OLAF:
      case RAY:
      case OLGA:
      case TYRONE:
        // Start combat etc
        DeleteTalkingMenu();
        CancelAIAction(pTSoldier, TRUE);
        AddToShouldBecomeHostileOrSayQuoteList(pTSoldier->ubID);
        break;
      default:
        // Start talking!
        return (InitiateConversation(pTSoldier, pSoldier, NPC_INITIAL_QUOTE, 0));
        break;
    }
  } else {
    // Start talking with hostile NPC
    return (InitiateConversation(pTSoldier, pSoldier, APPROACH_ENEMY_NPC_QUOTE, 0));
  }

  return (TRUE);
}

BOOLEAN IsValidSecondHandShot(struct SOLDIERTYPE *pSoldier) {
  if (Item[pSoldier->inv[SECONDHANDPOS].usItem].usItemClass == IC_GUN &&
      !(Item[pSoldier->inv[SECONDHANDPOS].usItem].fFlags & ITEM_TWO_HANDED) &&
      !pSoldier->bDoBurst && pSoldier->inv[HANDPOS].usItem != GLAUNCHER &&
      Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_GUN &&
      pSoldier->inv[SECONDHANDPOS].bGunStatus >= USABLE &&
      pSoldier->inv[SECONDHANDPOS].ubGunShotsLeft > 0) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN IsValidSecondHandShotForReloadingPurposes(struct SOLDIERTYPE *pSoldier) {
  // should be maintained as same as function above with line
  // about ammo taken out!
  if (Item[pSoldier->inv[SECONDHANDPOS].usItem].usItemClass == IC_GUN && !pSoldier->bDoBurst &&
      pSoldier->inv[HANDPOS].usItem != GLAUNCHER &&
      Item[pSoldier->inv[HANDPOS].usItem].usItemClass == IC_GUN &&
      pSoldier->inv[SECONDHANDPOS].bGunStatus >= USABLE  //&&
      //			 pSoldier->inv[SECONDHANDPOS].ubGunShotsLeft > 0 &&
      //			 gAnimControl[ pSoldier->usAnimState ].ubEndHeight != ANIM_PRONE )
  ) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN CanRobotBeControlled(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pController;

  if (!(pSoldier->uiStatusFlags & SOLDIER_ROBOT)) {
    return (FALSE);
  }

  if (pSoldier->ubRobotRemoteHolderID == NOBODY) {
    return (FALSE);
  }

  pController = MercPtrs[pSoldier->ubRobotRemoteHolderID];

  if (pController->bActive) {
    if (ControllingRobot(pController)) {
      // ALL'S OK!
      return (TRUE);
    }
  }

  return (FALSE);
}

BOOLEAN ControllingRobot(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pRobot;
  int8_t bPos;

  if (!IsSolActive(pSoldier)) {
    return (FALSE);
  }

  // EPCs can't control the robot (no inventory to hold remote, for one)
  if (AM_AN_EPC(pSoldier)) {
    return (FALSE);
  }

  // Don't require pSoldier->bInSector here, it must work from mapscreen!

  // are we in ok shape?
  if (pSoldier->bLife < OKLIFE || (pSoldier->bTeam != gbPlayerNum)) {
    return (FALSE);
  }

  // allow control from within vehicles - allows strategic travel in a vehicle with robot!
  if ((pSoldier->bAssignment >= ON_DUTY) && (pSoldier->bAssignment != VEHICLE)) {
    return (FALSE);
  }

  // is the soldier wearing a robot remote control?
  bPos = FindObj(pSoldier, ROBOT_REMOTE_CONTROL);
  if (bPos != HEAD1POS && bPos != HEAD2POS) {
    return (FALSE);
  }

  // Find the robot
  pRobot = FindSoldierByProfileID(ROBOT, TRUE);
  if (!pRobot) {
    return (FALSE);
  }

  if (pRobot->bActive) {
    // Are we in the same sector....?
    // ARM: CHANGED TO WORK IN MAPSCREEN, DON'T USE WorldSector HERE
    if (pRobot->sSectorX == GetSolSectorX(pSoldier) &&
        pRobot->sSectorY == GetSolSectorY(pSoldier) &&
        pRobot->bSectorZ == GetSolSectorZ(pSoldier)) {
      // they have to be either both in sector, or both on the road
      if (pRobot->fBetweenSectors == pSoldier->fBetweenSectors) {
        // if they're on the road...
        if (pRobot->fBetweenSectors) {
          // they have to be in the same squad or vehicle
          if (pRobot->bAssignment != pSoldier->bAssignment) {
            return (FALSE);
          }

          // if in a vehicle, must be the same vehicle
          if (pRobot->bAssignment == VEHICLE && (pRobot->iVehicleId != pSoldier->iVehicleId)) {
            return (FALSE);
          }
        }

        // all OK!
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

struct SOLDIERTYPE *GetRobotController(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->ubRobotRemoteHolderID == NOBODY) {
    return (NULL);
  } else {
    return (MercPtrs[pSoldier->ubRobotRemoteHolderID]);
  }
}

void UpdateRobotControllerGivenRobot(struct SOLDIERTYPE *pRobot) {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;

  // Loop through guys and look for a controller!

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // run through list
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if (ControllingRobot(pTeamSoldier)) {
        pRobot->ubRobotRemoteHolderID = pTeamSoldier->ubID;
        return;
      }
    }
  }

  pRobot->ubRobotRemoteHolderID = NOBODY;
}

void UpdateRobotControllerGivenController(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pTeamSoldier;
  int32_t cnt = 0;

  // First see if are still controlling the robot
  if (!ControllingRobot(pSoldier)) {
    return;
  }

  // set up soldier ptr as first element in mercptrs list
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // Loop through guys to find the robot....
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive && (pTeamSoldier->uiStatusFlags & SOLDIER_ROBOT)) {
      pTeamSoldier->ubRobotRemoteHolderID = GetSolID(pSoldier);
    }
  }
}

void HandleSoldierTakeDamageFeedback(struct SOLDIERTYPE *pSoldier) {
  // Do sound.....
  // if ( pSoldier->bLife >= CONSCIOUSNESS )
  {
    // ATE: Limit how often we grunt...
    if ((GetJA2Clock() - pSoldier->uiTimeSinceLastBleedGrunt) > 1000) {
      pSoldier->uiTimeSinceLastBleedGrunt = GetJA2Clock();

      DoMercBattleSound(pSoldier, (int8_t)(BATTLE_SOUND_HIT1 + Random(2)));
    }
  }

  // Flash portrait....
  pSoldier->fFlashPortrait = TRUE;
  pSoldier->bFlashPortraitFrame = FLASH_PORTRAIT_STARTSHADE;
  RESETTIMECOUNTER(pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY);
}

void HandleSystemNewAISituation(struct SOLDIERTYPE *pSoldier, BOOLEAN fResetABC) {
  // Are we an AI guy?
  if (gTacticalStatus.ubCurrentTeam != gbPlayerNum && pSoldier->bTeam != gbPlayerNum) {
    if (pSoldier->bNewSituation == IS_NEW_SITUATION) {
      // Cancel what they were doing....
      pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
      pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;
      pSoldier->fTurningFromPronePosition = FALSE;
      pSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
      pSoldier->ubPendingAction = NO_PENDING_ACTION;
      pSoldier->bEndDoorOpenCode = 0;

      // if this guy isn't under direct AI control, WHO GIVES A FLYING FLICK?
      if (pSoldier->uiStatusFlags & SOLDIER_UNDERAICONTROL) {
        if (pSoldier->fTurningToShoot) {
          pSoldier->fTurningToShoot = FALSE;
          // Release attacker
          // OK - this is hightly annoying , but due to the huge combinations of
          // things that can happen - 1 of them is that sLastTarget will get unset
          // after turn is done - so set flag here to tell it not to...
          pSoldier->fDontUnsetLastTargetFromTurn = TRUE;
          DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                   String("@@@@@@@ Reducing attacker busy count..., ending fire because saw "
                          "something: DONE IN SYSTEM NEW SITUATION"));
          ReduceAttackBusyCount(pSoldier->ubID, FALSE);
        }

        if (pSoldier->pTempObject != NULL) {
          // Place it back into inv....
          AutoPlaceObject(pSoldier, pSoldier->pTempObject, FALSE);
          MemFree(pSoldier->pTempObject);
          pSoldier->pTempObject = NULL;
          pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
          pSoldier->usPendingAnimation2 = NO_PENDING_ANIMATION;

          // Decrement attack counter...
          DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                   String("@@@@@@@ Reducing attacker busy count..., ending throw because saw "
                          "something: DONE IN SYSTEM NEW SITUATION"));
          ReduceAttackBusyCount(pSoldier->ubID, FALSE);
        }
      }
    }
  }
}

void InternalPlaySoldierFootstepSound(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubRandomSnd;
  int8_t bVolume = MIDVOLUME;
  // Assume outside
  uint32_t ubSoundBase = WALK_LEFT_OUT;
  uint8_t ubRandomMax = 4;

  // Determine if we are on the floor
  if (!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
    if (pSoldier->usAnimState == HOPFENCE) {
      bVolume = HIGHVOLUME;
    }

    if (pSoldier->uiStatusFlags & SOLDIER_ROBOT) {
      PlaySoldierJA2Sample(pSoldier->ubID, ROBOT_BEEP, RATE_11025,
                           SoundVolume(bVolume, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo),
                           TRUE);
      return;
    }

    // if ( SoldierOnScreen( GetSolID(pSoldier) ) )
    {
      if (pSoldier->usAnimState == CRAWLING) {
        ubSoundBase = CRAWL_1;
      } else {
        // Pick base based on terrain over....
        if (pSoldier->bOverTerrainType == FLAT_FLOOR) {
          ubSoundBase = WALK_LEFT_IN;
        } else if (pSoldier->bOverTerrainType == DIRT_ROAD ||
                   pSoldier->bOverTerrainType == PAVED_ROAD) {
          ubSoundBase = WALK_LEFT_ROAD;
        } else if (pSoldier->bOverTerrainType == LOW_WATER ||
                   pSoldier->bOverTerrainType == MED_WATER) {
          ubSoundBase = WATER_WALK1_IN;
          ubRandomMax = 2;
        } else if (pSoldier->bOverTerrainType == DEEP_WATER) {
          ubSoundBase = SWIM_1;
          ubRandomMax = 2;
        }
      }

      // Pick a random sound...
      do {
        ubRandomSnd = (uint8_t)Random(ubRandomMax);

      } while (ubRandomSnd == pSoldier->ubLastFootPrintSound);

      pSoldier->ubLastFootPrintSound = ubRandomSnd;

      // OK, if in realtime, don't play at full volume, because too many people walking around
      // sounds don't sound good - ( unless we are the selected guy, then always play at reg volume
      // )
      if (!(gTacticalStatus.uiFlags & INCOMBAT) && (pSoldier->ubID != gusSelectedSoldier)) {
        bVolume = LOWVOLUME;
      }

      PlaySoldierJA2Sample(pSoldier->ubID, ubSoundBase + pSoldier->ubLastFootPrintSound, RATE_11025,
                           SoundVolume(bVolume, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo),
                           TRUE);
    }
  }
}

void PlaySoldierFootstepSound(struct SOLDIERTYPE *pSoldier) {
  // normally, not in stealth mode
  if (!pSoldier->bStealthMode) {
    InternalPlaySoldierFootstepSound(pSoldier);
  }
}

void PlayStealthySoldierFootstepSound(struct SOLDIERTYPE *pSoldier) {
  // even if in stealth mode
  InternalPlaySoldierFootstepSound(pSoldier);
}

void CrowsFlyAway(uint8_t ubTeam) {
  uint32_t cnt;
  struct SOLDIERTYPE *pTeamSoldier;

  for (cnt = gTacticalStatus.Team[ubTeam].bFirstID, pTeamSoldier = MercPtrs[cnt];
       cnt <= gTacticalStatus.Team[ubTeam].bLastID; cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive && pTeamSoldier->bInSector) {
      if (pTeamSoldier->ubBodyType == CROW && pTeamSoldier->usAnimState != CROW_FLY) {
        // fly away even if not seen!
        HandleCrowFlyAway(pTeamSoldier);
      }
    }
  }
}

#ifdef JA2BETAVERSION
void DebugValidateSoldierData() {
  uint32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  wchar_t sString[1024];
  BOOLEAN fProblemDetected = FALSE;
  static uint32_t uiFrameCount = 0;

  // this function is too slow to run every frame, so do the check only every 50 frames
  if (uiFrameCount++ < 50) {
    return;
  }

  // reset frame counter
  uiFrameCount = 0;

  // Loop through our team...
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier)) {
      // OK, first check for alive people
      // Don't do this check if we are a vehicle...
      if (pSoldier->bLife > 0 && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
        // Alive -- now check for proper group IDs
        if (pSoldier->ubGroupID == 0 && pSoldier->bAssignment != IN_TRANSIT &&
            pSoldier->bAssignment != ASSIGNMENT_POW &&
            !(pSoldier->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))) {
          // This is bad!
          swprintf(sString, ARR_SIZE(sString),
                   L"Soldier Data Error: Soldier %d is alive but has a zero group ID.", cnt);
          fProblemDetected = TRUE;
        } else if ((pSoldier->ubGroupID != 0) && (GetGroup(pSoldier->ubGroupID) == NULL)) {
          // This is bad!
          swprintf(sString, ARR_SIZE(sString),
                   L"Soldier Data Error: Soldier %d has an invalid group ID of %d.", cnt,
                   pSoldier->ubGroupID);
          fProblemDetected = TRUE;
        }
      } else {
        if (pSoldier->ubGroupID != 0 && (pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
          // Dead guys should have 0 group IDs
          // swprintf( sString, L"GroupID Error: Soldier %d is dead but has a non-zero group ID.",
          // cnt ); fProblemDetected = TRUE;
        }
      }

      // check for invalid sector data
      if ((pSoldier->bAssignment != IN_TRANSIT) &&
          ((GetSolSectorX(pSoldier) <= 0) || (GetSolSectorX(pSoldier) >= 17) ||
           (pSoldier->sSectorY <= 0) || (pSoldier->sSectorY >= 17) || (pSoldier->bSectorZ < 0) ||
           (pSoldier->bSectorZ > 3))) {
        swprintf(sString, ARR_SIZE(sString),
                 L"Soldier Data Error: Soldier %d is located at %d/%d/%d.", cnt,
                 GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier));
        fProblemDetected = TRUE;
      }
    }

    if (fProblemDetected) {
      SAIReportError(sString);
      /*
                              if ( IsMapScreen_2() )
                                      DoMapMessageBox( MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN,
         MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback ); else DoMessageBox( MSG_BOX_BASIC_STYLE,
         sString, GAME_SCREEN, ( uint8_t )MSG_BOX_FLAG_OK, NULL, NULL );
      */
      break;
    }
  }

  // also do this
  ValidatePlayersAreInOneGroupOnly();
}
#endif

void BeginTyingToFall(struct SOLDIERTYPE *pSoldier) {
  pSoldier->bStartFallDir = pSoldier->bDirection;
  pSoldier->fTryingToFall = TRUE;

  // Randomize direction
  if (Random(50) < 25) {
    pSoldier->fFallClockwise = TRUE;
  } else {
    pSoldier->fFallClockwise = FALSE;
  }
}

void SetSoldierAsUnderAiControl(struct SOLDIERTYPE *pSoldierToSet) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t cnt;

  if (pSoldierToSet == NULL) {
    return;
  }

  // Loop through ALL teams...
  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[LAST_TEAM].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier)) {
      pSoldier->uiStatusFlags &= ~SOLDIER_UNDERAICONTROL;
    }
  }

  pSoldierToSet->uiStatusFlags |= SOLDIER_UNDERAICONTROL;
}

void HandlePlayerTogglingLightEffects(BOOLEAN fToggleValue) {
  if (fToggleValue) {
    // Toggle light status
    gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT] ^= TRUE;
  }

  // Update all the mercs in the sector
  EnableDisableSoldierLightEffects(gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT]);

  SetRenderFlags(RENDER_FLAG_FULL);
}

void EnableDisableSoldierLightEffects(BOOLEAN fEnableLights) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t cnt;

  // Loop through player teams...
  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++, pSoldier++) {
    // if the soldier is in the sector
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife >= OKLIFE) {
      // if we are to enable the lights
      if (fEnableLights) {
        // Add the light around the merc
        PositionSoldierLight(pSoldier);
      } else {
        // Delete the fake light the merc casts
        DeleteSoldierLight(pSoldier);

        // Light up the merc though
        SetSoldierPersonalLightLevel(pSoldier);
      }
    }
  }
}

void SetSoldierPersonalLightLevel(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier == NULL) {
    return;
  }

  if (pSoldier->sGridNo == NOWHERE) {
    return;
  }

  // THe light level for the soldier
  gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubShadeLevel = 3;
  gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubSumLights = 5;
  gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubMaxLights = 5;
  gpWorldLevelData[pSoldier->sGridNo].pMercHead->ubNaturalShadeLevel = 5;
}
