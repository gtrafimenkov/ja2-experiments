// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Strategic/Assignments.h"

#include <math.h>
#include <stdlib.h>

#include "CharList.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Line.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/MapScreenInterfaceMapInventory.h"
#include "Strategic/MercContract.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicEventHandler.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMercHandler.h"
#include "Strategic/StrategicMovement.h"
#include "Strategic/StrategicPathing.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Strategic/TownMilitia.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Campaign.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/ItemTypes.h"
#include "Tactical/Items.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierFind.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierProfileType.h"
#include "Tactical/Squads.h"
#include "Tactical/Vehicles.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderWorld.h"
#include "Town.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/PopUpBox.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"

// various reason an assignment can be aborted before completion
enum {
  NO_MORE_MED_KITS = 40,
  INSUF_DOCTOR_SKILL,
  NO_MORE_TOOL_KITS,
  INSUF_REPAIR_SKILL,

  NUM_ASSIGN_ABORT_REASONS
};

enum {
  REPAIR_MENU_VEHICLE1 = 0,
  REPAIR_MENU_VEHICLE2,
  REPAIR_MENU_VEHICLE3,
  //	REPAIR_MENU_SAM_SITE,
  REPAIR_MENU_ROBOT,
  REPAIR_MENU_ITEMS,
  REPAIR_MENU_CANCEL,
};

enum {
  VEHICLE_MENU_VEHICLE1 = 0,
  VEHICLE_MENU_VEHICLE2,
  VEHICLE_MENU_VEHICLE3,
  VEHICLE_MENU_CANCEL,
};

enum {
  REPAIR_HANDS_AND_ARMOR = 0,
  REPAIR_HEADGEAR,
  REPAIR_POCKETS,
  NUM_REPAIR_PASS_TYPES,
};

#define FINAL_REPAIR_PASS REPAIR_POCKETS

typedef struct REPAIR_PASS_SLOTS_TYPE {
  uint8_t ubChoices;  // how many valid choices there are in this pass
  int8_t bSlot[12];   // list of slots to be repaired in this pass
} REPAIR_PASS_SLOTS_TYPE;

REPAIR_PASS_SLOTS_TYPE gRepairPassSlotList[NUM_REPAIR_PASS_TYPES] = {
    // pass					# choices
    // slots
    // repaired
    // in
    // this
    // pass
    {/* hands and armor */ 5,
     {HANDPOS, SECONDHANDPOS, VESTPOS, HELMETPOS, LEGPOS, -1, -1, -1, -1, -1, -1, -1}},
    {/* headgear */ 2, {HEAD1POS, HEAD2POS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}},
    {/* pockets */ 12,
     {BIGPOCK1POS, BIGPOCK2POS, BIGPOCK3POS, BIGPOCK4POS, SMALLPOCK1POS, SMALLPOCK2POS,
      SMALLPOCK3POS, SMALLPOCK4POS, SMALLPOCK5POS, SMALLPOCK6POS, SMALLPOCK7POS, SMALLPOCK8POS}},
};

// PopUp Box Handles
int32_t ghAssignmentBox = -1;
int32_t ghEpcBox = -1;
int32_t ghSquadBox = -1;
int32_t ghVehicleBox = -1;
int32_t ghRepairBox = -1;
int32_t ghTrainingBox = -1;
int32_t ghAttributeBox = -1;
int32_t ghRemoveMercAssignBox = -1;
int32_t ghContractBox = -1;
int32_t ghMoveBox = -1;
// int32_t ghUpdateBox = -1;

// the x,y position of assignment pop up in tactical
int16_t gsAssignmentBoxesX, gsAssignmentBoxesY;

// assignment menu mouse regions
struct MOUSE_REGION gAssignmentMenuRegion[MAX_ASSIGN_STRING_COUNT];
struct MOUSE_REGION gTrainingMenuRegion[MAX_TRAIN_STRING_COUNT];
struct MOUSE_REGION gAttributeMenuRegion[MAX_ATTRIBUTE_STRING_COUNT];
struct MOUSE_REGION gSquadMenuRegion[MAX_SQUAD_MENU_STRING_COUNT];
struct MOUSE_REGION gContractMenuRegion[MAX_CONTRACT_MENU_STRING_COUNT];
struct MOUSE_REGION gRemoveMercAssignRegion[MAX_REMOVE_MERC_COUNT];
struct MOUSE_REGION gEpcMenuRegion[MAX_EPC_MENU_STRING_COUNT];
struct MOUSE_REGION gRepairMenuRegion[20];

// mouse region for vehicle menu
struct MOUSE_REGION gVehicleMenuRegion[20];

struct MOUSE_REGION gAssignmentScreenMaskRegion;

BOOLEAN fShownAssignmentMenu = FALSE;
BOOLEAN fShowVehicleMenu = FALSE;
BOOLEAN fShowRepairMenu = FALSE;
BOOLEAN fShownContractMenu = FALSE;

BOOLEAN fFirstClickInAssignmentScreenMask = FALSE;

// render pre battle interface?
extern BOOLEAN gfRenderPBInterface;
extern BOOLEAN fMapScreenBottomDirty;

// in the mapscreen?
extern BOOLEAN fInMapMode;

// we are in fact training?..then who temmates, or self?
int8_t gbTrainingMode = -1;

// who is the highlighted guy
extern uint16_t gusUIFullTargetID;

// showing town info?
extern BOOLEAN fShowTownInfo;

extern int32_t giMapBorderButtons[];

extern BOOLEAN fProcessingAMerc;
extern struct SOLDIERTYPE *pProcessingSoldier;

BOOLEAN gfAddDisplayBoxToWaitingQueue = FALSE;

// redraw character list
extern BOOLEAN fDrawCharacterList;

struct SOLDIERTYPE *gpDismissSoldier = NULL;

BOOLEAN gfReEvaluateEveryonesNothingToDo = FALSE;

// the amount time must be on assignment before it can have any effect
#define MINUTES_FOR_ASSIGNMENT_TO_COUNT 45

// number we divide the total pts accumlated per day by for each assignment period
#define ASSIGNMENT_UNITS_PER_DAY 24

// base skill to deal with an emergency
#define BASE_MEDICAL_SKILL_TO_DEAL_WITH_EMERGENCY 20

// multiplier for skill needed for each point below OKLIFE
#define MULTIPLIER_FOR_DIFFERENCE_IN_LIFE_VALUE_FOR_EMERGENCY 4

// number of pts needed for each point below OKLIFE
#define POINT_COST_PER_HEALTH_BELOW_OKLIFE 2

// how many points of healing each hospital patients gains per hour in the hospital
#define HOSPITAL_HEALING_RATE \
  5  // a top merc doctor can heal about 4 pts/hour maximum, but that's spread among patients!

// increase to reduce repair pts, or vice versa
#define REPAIR_RATE_DIVISOR 2500
// increase to reduce doctoring pts, or vice versa
#define DOCTORING_RATE_DIVISOR 2400  // at 2400, the theoretical maximum is 150 full healing pts/day

// cost to unjam a weapon in repair pts
#define REPAIR_COST_PER_JAM 2

// divisor for rate of self-training
#define SELF_TRAINING_DIVISOR 1000
// the divisor for rate of training bonus due to instructors influence
#define INSTRUCTED_TRAINING_DIVISOR 3000

// this controls how fast town militia gets trained
#define TOWN_TRAINING_RATE 4

#define MAX_MILITIA_TRAINERS_PER_SECTOR 2

// militia training bonus for EACH level of teaching skill (percentage points)
#define TEACH_BONUS_TO_TRAIN 30
// militia training bonus for RPC (percentage points)
#define RPC_BONUS_TO_TRAIN 10

// the bonus to training in marksmanship in the Alma gun range sector
#define GUN_RANGE_TRAINING_BONUS 25

// breath bonus divider
#define BREATH_BONUS_DIVIDER 10

// the min rating that is need to teach a fellow teammate
#define MIN_RATING_TO_TEACH 25

// activity levels for natural healing ( the higher the number, the slower the natural recover rate
#define LOW_ACTIVITY_LEVEL 1
#define MEDIUM_ACTIVITY_LEVEL 4
#define HIGH_ACTIVITY_LEVEL 12

/*
// the min breath to stay awake
#define MIN_BREATH_TO_STAY_AWAKE 15

// average number of hours a merc needs to sleep per day
#define AVG_NUMBER_OF_HOURS_OF_SLEEP_NEEDED 7
*/

/* Assignment distance limits removed.  Sep/11/98.  ARM
#define MAX_DISTANCE_FOR_DOCTORING	5
#define MAX_DISTANCE_FOR_REPAIR			5
#define MAX_DISTANCE_FOR_TRAINING		5
*/

/*
// controls how easily SAM sites are repaired
// NOTE: A repairman must generate a least this many points / hour to be ABLE to repair a SAM site
at all! #define SAM_SITE_REPAIR_DIVISOR		10

// minimum condition a SAM site must be in to be fixable
#define MIN_CONDITION_TO_FIX_SAM 20
*/

// how many points worth of tool kits does the character have?
uint16_t ToolKitPoints(struct SOLDIERTYPE *pSoldier);

// how many points worth of doctoring does the character have in his medical kits ?
uint16_t TotalMedicalKitPoints(struct SOLDIERTYPE *pSoldier);

// handle doctor in this sector
void HandleDoctorsInSector(int16_t sX, int16_t sY, int8_t bZ);

// handle any repair man in sector
void HandleRepairmenInSector(int16_t sX, int16_t sY, int8_t bZ);

// heal characters in this sector with this doctor
void HealCharacters(struct SOLDIERTYPE *pDoctor, int16_t sX, int16_t sY, int8_t bZ);

// update characters who might done healing but are still patients
void UpdatePatientsWhoAreDoneHealing(void);

// returns minimum medical skill necessary to treat this patient
uint8_t GetMinHealingSkillNeeded(struct SOLDIERTYPE *pPatient);

// heal patient, given doctor and total healing pts available to doctor at this time
uint16_t HealPatient(struct SOLDIERTYPE *pPatient, struct SOLDIERTYPE *pDoctor,
                     uint16_t usHundredthsHealed);

// can item be repaired?
BOOLEAN IsItemRepairable(uint16_t usItem, int8_t bStatus);

// does another merc have a repairable item on them?
int8_t FindRepairableItemOnOtherSoldier(struct SOLDIERTYPE *pSoldier, uint8_t ubPassType);

// repair stuff
void HandleRepairBySoldier(struct SOLDIERTYPE *pSoldier);

// rest the character
void RestCharacter(struct SOLDIERTYPE *pSoldier);
// fatigue the character
void FatigueCharacter(struct SOLDIERTYPE *pSoldier);

// a list of which sectors have characters
BOOLEAN fSectorsWithSoldiers[MAP_WORLD_X * MAP_WORLD_Y][4];

// can soldier repair robot
BOOLEAN CanCharacterRepairRobot(struct SOLDIERTYPE *pSoldier);

// can the character repair this vehicle?
BOOLEAN CanCharacterRepairVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId);

// handle training of character in sector
void HandleTrainingInSector(uint8_t sMapX, uint8_t sMapY, int8_t bZ);

// QSort compare function for town training
int TownTrainerQsortCompare(const void *pArg1, const void *pArg2);

// this function will actually pass on the pts to the mercs stat
void TrainSoldierWithPts(struct SOLDIERTYPE *pSoldier, int16_t sTrainPts);

// train militia in this sector with this soldier
static BOOLEAN TrainTownInSector(struct SOLDIERTYPE *pTrainer, uint8_t sMapX, uint8_t sMapY,
                                 uint16_t sTrainingPts);

// is the character between secotrs in mvt
BOOLEAN CharacterIsBetweenSectors(struct SOLDIERTYPE *pSoldier);

// update soldier life
void UpDateSoldierLife(struct SOLDIERTYPE *pSoldier);

// handle natural healing for all mercs on players team
void HandleNaturalHealing(void);

// handle natural healing for any individual grunt
void HandleHealingByNaturalCauses(struct SOLDIERTYPE *pSoldier);

/*
// auto sleep mercs
BOOLEAN AutoSleepMerc( struct SOLDIERTYPE *pSoldier );
*/

// assignment screen mask
void AssignmentScreenMaskBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);

// glow area for contract region?
BOOLEAN fGlowContractRegion = FALSE;

void HandleShadingOfLinesForSquadMenu(void);
void HandleShadingOfLinesForVehicleMenu(void);
void HandleShadingOfLinesForRepairMenu(void);
void HandleShadingOfLinesForTrainingMenu(void);
void HandleShadingOfLinesForAttributeMenus(void);

// post message about contract
void PostContractMessage(struct SOLDIERTYPE *pCharacter, int32_t iContract);

// post a terminate message
void PostTerminateMessage(struct SOLDIERTYPE *pCharacter);

BOOLEAN DisplayVehicleMenu(struct SOLDIERTYPE *pSoldier);
BOOLEAN DisplayRepairMenu(struct SOLDIERTYPE *pSoldier);

// create menus
void CreateEPCBox(void);
void CreateSquadBox(void);
void CreateVehicleBox();
void CreateRepairBox(void);

/*
// get how fast the person regains sleep
int8_t GetRegainDueToSleepNeeded( struct SOLDIERTYPE *pSoldier, int32_t iRateOfReGain );
*/

void PositionCursorForTacticalAssignmentBox(void);

// can this soldier be healed by this doctor?
BOOLEAN CanSoldierBeHealedByDoctor(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pDoctor,
                                   BOOLEAN fIgnoreAssignment, BOOLEAN fThisHour,
                                   BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck);
uint8_t GetNumberThatCanBeDoctored(struct SOLDIERTYPE *pDoctor, BOOLEAN fThisHour,
                                   BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck);
void CheckForAndHandleHospitalPatients(void);
void HealHospitalPatient(struct SOLDIERTYPE *pPatient, uint16_t usHealingPtsLeft);

void MakeSureToolKitIsInHand(struct SOLDIERTYPE *pSoldier);
BOOLEAN MakeSureMedKitIsInHand(struct SOLDIERTYPE *pSoldier);

void RepositionMouseRegions(void);
void CheckAndUpdateTacticalAssignmentPopUpPositions(void);
void HandleRestFatigueAndSleepStatus(void);
BOOLEAN CharacterIsTakingItEasy(struct SOLDIERTYPE *pSoldier);
void RepairMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason);
BOOLEAN CanCharacterDoctorButDoesntHaveMedKit(struct SOLDIERTYPE *pSoldier);
BOOLEAN CanCharacterRepairButDoesntHaveARepairkit(struct SOLDIERTYPE *pSoldier);

// robot replated stuff
BOOLEAN IsRobotInThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);
struct SOLDIERTYPE *GetRobotSoldier(void);
uint8_t RepairRobot(struct SOLDIERTYPE *pRobot, uint8_t ubRepairPts,
                    BOOLEAN *pfNothingLeftToRepair);
uint8_t HandleRepairOfRobotBySoldier(struct SOLDIERTYPE *pSoldier, uint8_t ubRepairPts,
                                     BOOLEAN *pfNothingLeftToRepair);
BOOLEAN HandleAssignmentExpansionAndHighLightForAssignMenu(struct SOLDIERTYPE *pSoldier);
BOOLEAN HandleAssignmentExpansionAndHighLightForTrainingMenu(void);
BOOLEAN HandleShowingOfMovementBox(void);
// BOOLEAN HandleShowingOfUpBox( void );
void ReportTrainersTraineesWithoutPartners(void);
BOOLEAN ValidTrainingPartnerInSameSectorOnAssignmentFound(struct SOLDIERTYPE *pSoldier,
                                                          int8_t bTargetAssignment,
                                                          int8_t bTargetStat);

extern void AddSectorForSoldierToListOfSectorsThatCompletedMilitiaTraining(
    struct SOLDIERTYPE *pSoldier);

extern BOOLEAN SectorIsImpassable(int16_t sSector);

extern BOOLEAN CanChangeSleepStatusForCharSlot(int8_t bCharNumber);

extern uint32_t VirtualSoldierDressWound(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pVictim,
                                         struct OBJECTTYPE *pKit, int16_t sKitPts, int16_t sStatus);

// only 2 trainers are allowed per sector, so this function counts the # in a guy's sector
int8_t CountMilitiaTrainersInSoldiersSector(struct SOLDIERTYPE *pSoldier);

// notify player of assignment attempt failure
void NotifyPlayerOfAssignmentAttemptFailure(int8_t bAssignment);

BOOLEAN PlayerSoldierTooTiredToTravel(struct SOLDIERTYPE *pSoldier);

void AssignmentAborted(struct SOLDIERTYPE *pSoldier, uint8_t ubReason);

uint8_t CalcSoldierNeedForSleep(struct SOLDIERTYPE *pSoldier);

uint32_t GetLastSquadListedInSquadMenu(void);

BOOLEAN IsAnythingAroundForSoldierToRepair(struct SOLDIERTYPE *pSoldier);
BOOLEAN HasCharacterFinishedRepairing(struct SOLDIERTYPE *pSoldier);
BOOLEAN DoesCharacterHaveAnyItemsToRepair(struct SOLDIERTYPE *pSoldier, int8_t bHighestPass);

BOOLEAN CanCharacterRepairAnotherSoldiersStuff(struct SOLDIERTYPE *pSoldier,
                                               struct SOLDIERTYPE *pOtherSoldier);

// can this character EVER train militia?
BOOLEAN BasicCanCharacterTrainMilitia(struct SOLDIERTYPE *pCharacter);

struct SOLDIERTYPE *GetSelectedAssignSoldier(BOOLEAN fNullOK);

BOOLEAN RepairObject(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pOwner,
                     struct OBJECTTYPE *pObj, uint8_t *pubRepairPtsLeft);
void RepairItemsOnOthers(struct SOLDIERTYPE *pSoldier, uint8_t *pubRepairPtsLeft);
BOOLEAN UnjamGunsOnSoldier(struct SOLDIERTYPE *pOwnerSoldier, struct SOLDIERTYPE *pRepairSoldier,
                           uint8_t *pubRepairPtsLeft);

void InitSectorsWithSoldiersList(void) {
  // init list of sectors
  memset(&fSectorsWithSoldiers, 0, sizeof(fSectorsWithSoldiers));

  return;
}

void BuildSectorsWithSoldiersList(void) {
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  pSoldier = MercPtrs[0];

  // fills array with pressence of player controlled characters
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      fSectorsWithSoldiers[pTeamSoldier->sSectorX + pTeamSoldier->sSectorY * MAP_WORLD_X]
                          [pTeamSoldier->bSectorZ] = TRUE;
    }
  }
}

void ChangeSoldiersAssignment(struct SOLDIERTYPE *pSoldier, int8_t bAssignment) {
  // This is the most basic assignment-setting function.  It must be called before setting any
  // subsidiary values like fFixingRobot.  It will clear all subsidiary values so we don't leave the
  // merc in a messed up state!

  pSoldier->bAssignment = bAssignment;
  /// don't kill iVehicleId, though, 'cause militia training tries to put guys back in their
  /// vehicles when it's done(!)

  pSoldier->fFixingSAMSite = FALSE;
  pSoldier->fFixingRobot = FALSE;
  pSoldier->bVehicleUnderRepairID = -1;

  if ((bAssignment == DOCTOR) || (bAssignment == PATIENT) || (bAssignment == ASSIGNMENT_HOSPITAL)) {
    AddStrategicEvent(EVENT_BANDAGE_BLEEDING_MERCS, GetWorldTotalMin() + 1, 0);
  }

  // update character info, and the team panel
  fCharacterInfoPanelDirty = TRUE;
  fTeamPanelDirty = TRUE;

  // merc may have come on/off duty, make sure map icons are updated
  MarkForRedrawalStrategicMap();
}

BOOLEAN BasicCanCharacterAssignment(struct SOLDIERTYPE *pSoldier, BOOLEAN fNotInCombat) {
  // global conditions restricting all assignment changes
  if (SectorIsImpassable((int16_t)GetSolSectorID8(pSoldier))) {
    return (FALSE);
  }

  if (fNotInCombat && IsSolActive(pSoldier) && pSoldier->bInSector &&
      gTacticalStatus.fEnemyInSector) {
    return (FALSE);
  }

  return (TRUE);
}

/*
BOOLEAN CanSoldierAssignment( struct SOLDIERTYPE *pSoldier, int8_t bAssignment )
{
        switch( bAssignment )
        {
                case( DOCTOR ):
                        return( CanCharacterDoctor( pSoldier ) );
                        break;
                case( PATIENT ):
                        return( CanCharacterPatient( pSoldier ) );
                        break;
                case( REPAIR ):
                        return( CanCharacterRepair( pSoldier ) );
                        break;
                case( TRAIN_TOWN ):
                        return( CanCharacterTrainMilitia( pSoldier ) );
                        break;
                case( TRAIN_SELF ):
                        return( CanCharacterTrainStat( pSoldier, pSoldier -> bTrainStat, TRUE, FALSE
) ); break; case( TRAIN_TEAMMATE ): return( CanCharacterTrainStat( pSoldier, pSoldier -> bTrainStat,
FALSE, TRUE ) ); break; case TRAIN_BY_OTHER: return( CanCharacterTrainStat( pSoldier, pSoldier ->
bTrainStat, TRUE, FALSE ) ); break; case( VEHICLE ): return( CanCharacterVehicle( pSoldier ) );
                        break;
                default:
                        return( (CanCharacterSquad( pSoldier, bAssignment ) ==
CHARACTER_CAN_JOIN_SQUAD ) ); break;
        }
}
*/

BOOLEAN CanCharacterDoctorButDoesntHaveMedKit(struct SOLDIERTYPE *pSoldier) {
  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // make sure character is alive and conscious
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  // has medical skill?
  if (pSoldier->bMedical <= 0) {
    // no skill whatsoever
    return (FALSE);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    // epcs can't do this
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

// is character capable of 'playing' doctor?
// check that character is alive, conscious, has medical skill and equipment
BOOLEAN CanCharacterDoctor(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fFoundMedKit = FALSE;
  int8_t bPocket = 0;

  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  if (CanCharacterDoctorButDoesntHaveMedKit(pSoldier) == FALSE) {
    return (FALSE);
  }

  // find med kit
  for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
    // doctoring is allowed using either type of med kit (but first aid kit halves doctoring
    // effectiveness)
    if (IsMedicalKitItem(&(pSoldier->inv[bPocket]))) {
      fFoundMedKit = TRUE;
      break;
    }
  }

  if (fFoundMedKit == FALSE) {
    return (FALSE);
  }

  // all criteria fit, can doctor
  return (TRUE);
}

BOOLEAN IsAnythingAroundForSoldierToRepair(struct SOLDIERTYPE *pSoldier) {
  int32_t iCounter;

  // items?
  if (DoesCharacterHaveAnyItemsToRepair(pSoldier, FINAL_REPAIR_PASS)) {
    return (TRUE);
  }

  // robot?
  if (CanCharacterRepairRobot(pSoldier)) {
    return (TRUE);
  }

  // vehicles?
  if (GetSolSectorZ(pSoldier) == 0) {
    for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
      if (pVehicleList[iCounter].fValid == TRUE) {
        // the helicopter, is NEVER repairable...
        if (iCounter != iHelicopterVehicleId) {
          if (IsThisVehicleAccessibleToSoldier(pSoldier, iCounter)) {
            if (CanCharacterRepairVehicle(pSoldier, iCounter) == TRUE) {
              // there is a repairable vehicle here
              return (TRUE);
            }
          }
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN HasCharacterFinishedRepairing(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fCanStillRepair;

  // NOTE: This must detect situations where the vehicle/robot has left the sector, in which case we
  // want the guy to say "assignment done", so we return that he can no longer repair

  // check if we are repairing a vehicle
  if (pSoldier->bVehicleUnderRepairID != -1) {
    fCanStillRepair = CanCharacterRepairVehicle(pSoldier, pSoldier->bVehicleUnderRepairID);
  }
  // check if we are repairing a robot
  else if (pSoldier->fFixingRobot) {
    fCanStillRepair = CanCharacterRepairRobot(pSoldier);
  } else  // repairing items
  {
    fCanStillRepair = DoesCharacterHaveAnyItemsToRepair(pSoldier, FINAL_REPAIR_PASS);
  }

  // if it's no longer damaged, we're finished!
  return (!fCanStillRepair);
}

BOOLEAN DoesCharacterHaveAnyItemsToRepair(struct SOLDIERTYPE *pSoldier, int8_t bHighestPass) {
  int8_t bPocket;
  uint8_t ubItemsInPocket, ubObjectInPocketCounter;
  int8_t bLoop;
  struct SOLDIERTYPE *pOtherSoldier;
  struct OBJECTTYPE *pObj;
  uint8_t ubPassType;

  // check for jams
  for (bPocket = HELMETPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
    ubItemsInPocket = pSoldier->inv[bPocket].ubNumberOfObjects;
    // unjam any jammed weapons
    // run through pocket and repair
    for (ubObjectInPocketCounter = 0; ubObjectInPocketCounter < ubItemsInPocket;
         ubObjectInPocketCounter++) {
      // jammed gun?
      if ((Item[pSoldier->inv[bPocket].usItem].usItemClass == IC_GUN) &&
          (pSoldier->inv[bPocket].bGunAmmoStatus < 0)) {
        return (TRUE);
      }
    }
  }

  // now check for items to repair
  for (bPocket = HELMETPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
    pObj = &(pSoldier->inv[bPocket]);

    ubItemsInPocket = pObj->ubNumberOfObjects;

    // run through pocket
    for (ubObjectInPocketCounter = 0; ubObjectInPocketCounter < ubItemsInPocket;
         ubObjectInPocketCounter++) {
      // if it's repairable and NEEDS repairing
      if (IsItemRepairable(pObj->usItem, pObj->bStatus[ubObjectInPocketCounter])) {
        return (TRUE);
      }
    }

    // have to check for attachments...
    for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++) {
      if (pObj->usAttachItem[bLoop] != NOTHING) {
        // if it's repairable and NEEDS repairing
        if (IsItemRepairable(pObj->usAttachItem[bLoop], pObj->bAttachStatus[bLoop])) {
          return (TRUE);
        }
      }
    }
  }

  // if we wanna check for the items belonging to others in the sector
  if (bHighestPass != -1) {
    // now look for items to repair on other mercs
    for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
         bLoop < gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++) {
      pOtherSoldier = MercPtrs[bLoop];

      if (CanCharacterRepairAnotherSoldiersStuff(pSoldier, pOtherSoldier)) {
        // okay, seems like a candidate!  Check if he has anything that needs unjamming or repairs

        for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
          // the object a weapon? and jammed?
          if ((Item[pOtherSoldier->inv[bPocket].usItem].usItemClass == IC_GUN) &&
              (pOtherSoldier->inv[bPocket].bGunAmmoStatus < 0)) {
            return (TRUE);
          }
        }

        // repair everyone's hands and armor slots first, then headgear, and pockets last
        for (ubPassType = REPAIR_HANDS_AND_ARMOR; ubPassType <= (uint8_t)bHighestPass;
             ubPassType++) {
          bPocket = FindRepairableItemOnOtherSoldier(pOtherSoldier, ubPassType);
          if (bPocket != NO_SLOT) {
            return (TRUE);
          }
        }
      }
    }
  }

  return (FALSE);
}

BOOLEAN BasicCanCharacterRepair(struct SOLDIERTYPE *pSoldier) {
  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // make sure character is alive and oklife
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  // has repair skill?
  if (pSoldier->bMechanical <= 0) {
    // no skill whatsoever
    return (FALSE);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    // epcs can't do this
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

BOOLEAN CanCharacterRepairButDoesntHaveARepairkit(struct SOLDIERTYPE *pSoldier) {
  if (BasicCanCharacterRepair(pSoldier) == FALSE) {
    return (FALSE);
  }

  // make sure he actually doesn't have a toolkit
  if (FindObj(pSoldier, TOOLKIT) != NO_SLOT) {
    return (FALSE);
  }

  return (TRUE);
}

// can character be assigned as repairman?
// check that character is alive, oklife, has repair skill, and equipment, etc.
BOOLEAN CanCharacterRepair(struct SOLDIERTYPE *pSoldier) {
  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  if (BasicCanCharacterRepair(pSoldier) == FALSE) {
    return (FALSE);
  }

  // make sure he has a toolkit
  if (FindObj(pSoldier, TOOLKIT) == NO_SLOT) {
    return (FALSE);
  }

  // anything around to fix?
  if (!IsAnythingAroundForSoldierToRepair(pSoldier)) {
    return (FALSE);
  }

  // NOTE: This will not detect situations where character lacks the SKILL to repair the stuff that
  // needs repairing... So, in that situation, his assignment will NOT flash, but a message to that
  // effect will be reported every hour.

  // all criteria fit, can repair
  return (TRUE);
}

// can character be set to patient?
BOOLEAN CanCharacterPatient(struct SOLDIERTYPE *pSoldier) {
  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // Robot must be REPAIRED to be "healed", not doctored
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
    return (FALSE);
  }

  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    return (FALSE);
  }

  // is character alive and not in perfect health?
  if ((pSoldier->bLife <= 0) || (pSoldier->bLife == pSoldier->bLifeMax)) {
    // dead or in perfect health
    return (FALSE);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  // alive and can be healed
  return (TRUE);
}

BOOLEAN BasicCanCharacterTrainMilitia(struct SOLDIERTYPE *pSoldier) {
  // is the character capable of training a town?
  // they must be alive/conscious and in the sector with the town
  BOOLEAN fSamSitePresent = FALSE;

  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // make sure character is alive and conscious
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  // underground training is not allowed (code doesn't support and it's a reasonable enough
  // limitation)
  if (GetSolSectorZ(pSoldier) != 0) {
    return (FALSE);
  }

  // is there a town in the character's current sector?
  if (StrategicMap[GetSectorID16(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier))].townID ==
      BLANK_SECTOR) {
    fSamSitePresent = IsThisSectorASAMSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                             GetSolSectorZ(pSoldier));

    // check if sam site
    if (fSamSitePresent == FALSE) {
      // nope
      return (FALSE);
    }
  }

  if (NumEnemiesInAnySector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                            GetSolSectorZ(pSoldier))) {
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    // epcs can't do this
    return (FALSE);
  }

  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
    return (FALSE);
  }

  // has leadership skill?
  if (pSoldier->bLeadership <= 0) {
    // no skill whatsoever
    return (FALSE);
  }

  // can train militia
  return (TRUE);
}

BOOLEAN CanCharacterTrainMilitia(struct SOLDIERTYPE *pSoldier) {
  if (BasicCanCharacterTrainMilitia(pSoldier) &&
      MilitiaTrainingAllowedInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                     GetSolSectorZ(pSoldier)) &&
      DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(pSoldier) &&
      (IsMilitiaTrainableFromSoldiersSectorMaxed(pSoldier) == FALSE) &&
      (CountMilitiaTrainersInSoldiersSector(pSoldier) < MAX_MILITIA_TRAINERS_PER_SECTOR)) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

BOOLEAN DoesTownHaveRatingToTrainMilitia(TownID bTownId) {
  // min loyalty rating?
  if ((gTownLoyalty[bTownId].ubRating < MIN_RATING_TO_TRAIN_TOWN)) {
    // nope
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(struct SOLDIERTYPE *pSoldier) {
  TownID bTownId = 0;
  BOOLEAN fSamSitePresent = FALSE;

  // underground training is not allowed (code doesn't support and it's a reasonable enough
  // limitation)
  if (GetSolSectorZ(pSoldier) != 0) {
    return (FALSE);
  }

  bTownId = GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

  // is there a town really here
  if (bTownId == BLANK_SECTOR) {
    fSamSitePresent = IsThisSectorASAMSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                             GetSolSectorZ(pSoldier));

    // if there is a sam site here
    if (fSamSitePresent) {
      return (TRUE);
    }

    return (FALSE);
  }

  // does this town have sufficient loyalty to train militia
  if (DoesTownHaveRatingToTrainMilitia(bTownId) == FALSE) {
    return (FALSE);
  }

  return (TRUE);
}

int8_t CountMilitiaTrainersInSoldiersSector(struct SOLDIERTYPE *pSoldier) {
  int8_t bLoop;
  struct SOLDIERTYPE *pOtherSoldier;
  int8_t bCount = 0;

  for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
       bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++) {
    pOtherSoldier = MercPtrs[bLoop];
    if (pSoldier != pOtherSoldier && pOtherSoldier->bActive && pOtherSoldier->bLife >= OKLIFE &&
        pOtherSoldier->sSectorX == GetSolSectorX(pSoldier) &&
        pOtherSoldier->sSectorY == GetSolSectorY(pSoldier) &&
        GetSolSectorZ(pSoldier) == pOtherSoldier->bSectorZ) {
      if (pOtherSoldier->bAssignment == TRAIN_TOWN) {
        bCount++;
      }
    }
  }
  return (bCount);
}

BOOLEAN IsMilitiaTrainableFromSoldiersSectorMaxed(struct SOLDIERTYPE *pSoldier) {
  TownID bTownId = 0;
  BOOLEAN fSamSitePresent = FALSE;

  if (GetSolSectorZ(pSoldier) != 0) {
    return (TRUE);
  }

  bTownId = GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

  // is there a town really here
  if (bTownId == BLANK_SECTOR) {
    fSamSitePresent = IsThisSectorASAMSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                             GetSolSectorZ(pSoldier));

    // if there is a sam site here
    if (fSamSitePresent) {
      if (IsSAMSiteFullOfMilitia(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier))) {
        return (TRUE);
      }
      return (FALSE);
    }

    return (FALSE);
  }

  // this considers *ALL* safe sectors of the town, not just the one soldier is in
  if (IsTownFullMilitia(bTownId)) {
    // town is full of militia
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN CanCharacterTrainStat(struct SOLDIERTYPE *pSoldier, int8_t bStat, BOOLEAN fTrainSelf,
                              BOOLEAN fTrainTeammate) {
  // is the character capable of training this stat? either self or as trainer

  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // alive and conscious
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  // underground training is not allowed (code doesn't support and it's a reasonable enough
  // limitation)
  if (GetSolSectorZ(pSoldier) != 0) {
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    // epcs can't do this
    return (FALSE);
  }

  // check stat values, 0 means no chance in hell
  switch (bStat) {
    case (STRENGTH):
      // strength
      if ((pSoldier->bStrength == 0) ||
          ((pSoldier->bStrength < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bStrength >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }
      break;
    case (DEXTERITY):
      // dexterity
      if ((pSoldier->bDexterity == 0) ||
          ((pSoldier->bDexterity < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bDexterity >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }
      break;
    case (AGILITY):
      // agility
      if ((pSoldier->bAgility == 0) ||
          ((pSoldier->bAgility < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bAgility >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }

      break;
    case (HEALTH):
      // wisdom
      if ((pSoldier->bLifeMax == 0) ||
          ((pSoldier->bLifeMax < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bLifeMax >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }

      break;
    case (MARKSMANSHIP):
      // marksmanship
      if ((pSoldier->bMarksmanship == 0) ||
          ((pSoldier->bMarksmanship < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bMarksmanship >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }

      break;
    case (MEDICAL):
      // medical
      if ((pSoldier->bMedical == 0) ||
          ((pSoldier->bMedical < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bMedical >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }

      break;
    case (MECHANICAL):
      // mechanical
      if ((pSoldier->bMechanical == 0) ||
          ((pSoldier->bMechanical < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bMechanical >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }
      break;
    case (LEADERSHIP):
      // leadership
      if ((pSoldier->bLeadership == 0) ||
          ((pSoldier->bLeadership < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bLeadership >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }
      break;
    case (EXPLOSIVE_ASSIGN):
      // explosives
      if ((pSoldier->bExplosive == 0) ||
          ((pSoldier->bExplosive < MIN_RATING_TO_TEACH) && (fTrainTeammate))) {
        return (FALSE);
      } else if ((pSoldier->bExplosive >= TRAINING_RATING_CAP) && (fTrainSelf)) {
        return (FALSE);
      }
      break;
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  // stat is ok and character alive and conscious
  return (TRUE);
}

BOOLEAN CanCharacterOnDuty(struct SOLDIERTYPE *pSoldier) {
  // can character commit themselves to on duty?

  // only need to be alive and well to do so right now
  // alive and conscious
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  if (!BasicCanCharacterAssignment(pSoldier, FALSE)) {
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }
  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // ARM: New rule: can't change squads or exit vehicles between sectors!
  if (pSoldier->fBetweenSectors) {
    return (FALSE);
  }

  /*
          if( pSoldier -> fBetweenSectors )
          {
                  if( pSoldier -> bAssignment == VEHICLE )
                  {
                          if( GetNumberInVehicle( pSoldier -> iVehicleId ) == 1 )
                          {
                                  // can't change, go away
                                  return( FALSE );
                          }
                  }
          }
  */

  return (TRUE);
}

BOOLEAN CanCharacterPractise(struct SOLDIERTYPE *pSoldier) {
  // can character practise right now?

  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // only need to be alive and well to do so right now
  // alive and conscious
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  if (GetSolSectorZ(pSoldier) != 0) {
    return (FALSE);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    // epcs can't do this
    return (FALSE);
  }

  // can practise
  return (TRUE);
}

BOOLEAN CanCharacterTrainTeammates(struct SOLDIERTYPE *pSoldier) {
  // can character train at all
  if (CanCharacterPractise(pSoldier) == FALSE) {
    // nope
    return (FALSE);
  }

  // if alone in sector, can't enter the attributes submenu at all
  if (PlayerMercsInSector((uint8_t)GetSolSectorX(pSoldier), (uint8_t)GetSolSectorY(pSoldier),
                          GetSolSectorZ(pSoldier)) == 0) {
    return (FALSE);
  }

  // ARM: we allow this even if there are no students assigned yet.  Flashing is warning enough.
  return (TRUE);
}

BOOLEAN CanCharacterBeTrainedByOther(struct SOLDIERTYPE *pSoldier) {
  // can character train at all
  if (CanCharacterPractise(pSoldier) == FALSE) {
    return (FALSE);
  }

  // if alone in sector, can't enter the attributes submenu at all
  if (PlayerMercsInSector((uint8_t)GetSolSectorX(pSoldier), (uint8_t)GetSolSectorY(pSoldier),
                          GetSolSectorZ(pSoldier)) == 0) {
    return (FALSE);
  }

  // ARM: we now allow this even if there are no trainers assigned yet.  Flashing is warning enough.
  return (TRUE);
}

// can character sleep right now?
BOOLEAN CanCharacterSleep(struct SOLDIERTYPE *pSoldier, BOOLEAN fExplainWhyNot) {
  wchar_t sString[128];

  // dead or dying?
  if (pSoldier->bLife < OKLIFE) {
    return (FALSE);
  }

  // vehicle or robot?
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
    return (FALSE);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // POW?
  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    return (FALSE);
  }

  // traveling?
  if (pSoldier->fBetweenSectors) {
    // if walking
    if (pSoldier->bAssignment != VEHICLE) {
      // can't sleep while walking or driving a vehicle
      if (fExplainWhyNot) {
        // on the move, can't sleep
        swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[5], pSoldier->name);
        DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
      }

      return (FALSE);
    } else  // in a vehicle
    {
      // if this guy has to drive ('cause nobody else can)
      if (SoldierMustDriveVehicle(pSoldier, pSoldier->iVehicleId, FALSE)) {
        // can't sleep while walking or driving a vehicle
        if (fExplainWhyNot) {
          // is driving, can't sleep
          swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[7], pSoldier->name);
          DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
        }

        return (FALSE);
      }
    }
  } else  // in a sector
  {
    // if not above it all...
    if (!SoldierAboardAirborneHeli(pSoldier)) {
      // if he's in the loaded sector, and it's hostile or in combat
      if (pSoldier->bInSector &&
          ((gTacticalStatus.uiFlags & INCOMBAT) || gTacticalStatus.fEnemyInSector)) {
        if (fExplainWhyNot) {
          DoScreenIndependantMessageBox(Message[STR_SECTOR_NOT_CLEARED], MSG_BOX_FLAG_OK, NULL);
        }

        return (FALSE);
      }

      // on surface, and enemies are in the sector
      if ((GetSolSectorZ(pSoldier) == 0) &&
          (NumEnemiesInAnySector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                 GetSolSectorZ(pSoldier)) > 0)) {
        if (fExplainWhyNot) {
          DoScreenIndependantMessageBox(Message[STR_SECTOR_NOT_CLEARED], MSG_BOX_FLAG_OK, NULL);
        }

        return (FALSE);
      }
    }
  }

  // not tired?
  if (pSoldier->bBreathMax >= BREATHMAX_FULLY_RESTED) {
    if (fExplainWhyNot) {
      swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[4], pSoldier->name);
      DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
    }

    return (FALSE);
  }

  // can sleep
  return (TRUE);
}

BOOLEAN CanCharacterBeAwakened(struct SOLDIERTYPE *pSoldier, BOOLEAN fExplainWhyNot) {
  wchar_t sString[128];

  // if dead tired
  if ((pSoldier->bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM) && !pSoldier->fMercCollapsedFlag) {
    // should be collapsed, then!
    pSoldier->fMercCollapsedFlag = TRUE;
  }

  // merc collapsed due to being dead tired, you can't wake him up until he recovers substantially
  if (pSoldier->fMercCollapsedFlag == TRUE) {
    if (fExplainWhyNot) {
      swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[6], pSoldier->name);
      DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
    }

    return (FALSE);
  }

  // can be awakened
  return (TRUE);
}

BOOLEAN CanCharacterVehicle(struct SOLDIERTYPE *pSoldier) {
  // can character enter/leave vehicle?

  if (!BasicCanCharacterAssignment(pSoldier, TRUE)) {
    return (FALSE);
  }

  // only need to be alive and well to do so right now
  // alive and conscious
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (FALSE);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (FALSE);
  }

  // character on the move?
  if (CharacterIsBetweenSectors(pSoldier)) {
    return (FALSE);
  }

  // underground?
  if (GetSolSectorZ(pSoldier) != 0) {
    return (FALSE);
  }

  // check in helicopter in hostile sector
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    if ((iHelicopterVehicleId != -1) && (pSoldier->iVehicleId == iHelicopterVehicleId)) {
      // enemies in sector
      if (NumEnemiesInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier)) > 0) {
        return (FALSE);
      }
    }
  }

  // any accessible vehicles in soldier's sector (excludes those between sectors, etc.)
  if (AnyAccessibleVehiclesInSoldiersSector(pSoldier) == FALSE) {
    return (FALSE);
  }

  // have to be in mapscreen (strictly for visual reasons - we don't want them just vanishing if in
  // tactical)
  if (fInMapMode == FALSE) {
    return (FALSE);
  }

  // if we're in BATTLE in the current sector, disallow
  if (gTacticalStatus.fEnemyInSector) {
    if ((GetSolSectorX(pSoldier) == gWorldSectorX) && (GetSolSectorY(pSoldier) == gWorldSectorY) &&
        (GetSolSectorZ(pSoldier) == gbWorldSectorZ)) {
      return (FALSE);
    }
  }

  return (TRUE);
}

int8_t CanCharacterSquad(struct SOLDIERTYPE *pSoldier, int8_t bSquadValue) {
  // can character join this squad?
  int16_t sX, sY, sZ;

  Assert(bSquadValue < ON_DUTY);

  if (GetSolAssignment(pSoldier) == bSquadValue) {
    return (CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT);
  }

  // is the character alive and well?
  if (pSoldier->bLife < OKLIFE) {
    // dead or unconscious...
    return (CHARACTER_CANT_JOIN_SQUAD);
  }

  // in transit?
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return (CHARACTER_CANT_JOIN_SQUAD);
  }

  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    // not allowed to be put on a squad
    return (CHARACTER_CANT_JOIN_SQUAD);
  }

  /* Driver can't abandon vehicle between sectors - OBSOLETE - nobody is allowed to change squads
     between sectors now! if( pSoldier -> fBetweenSectors )
          {
                  if( pSoldier -> bAssignment == VEHICLE )
                  {
                          if( GetNumberInVehicle( pSoldier -> iVehicleId ) == 1 )
                          {
                                  // can't change, go away
                                  return( CHARACTER_CANT_JOIN_SQUAD );
                          }
                  }
          }
  */

  // see if the squad us at the same x,y,z
  SectorSquadIsIn(bSquadValue, &sX, &sY, &sZ);

  // check sector x y and z, if not same, cannot join squad
  if ((sX != GetSolSectorX(pSoldier)) || (sY != GetSolSectorY(pSoldier)) ||
      (sZ != GetSolSectorZ(pSoldier))) {
    // is there anyone on this squad?
    if (NumberOfPeopleInSquad(bSquadValue) > 0) {
      return (CHARACTER_CANT_JOIN_SQUAD_TOO_FAR);
    }
  }

  if (IsThisSquadOnTheMove(bSquadValue) == TRUE) {
    // can't join while squad is moving
    return (CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING);
  }

  if (DoesVehicleExistInSquad(bSquadValue)) {
    // sorry can't change to this squad that way!
    return (CHARACTER_CANT_JOIN_SQUAD_VEHICLE);
  }

  if (NumberOfPeopleInSquad(bSquadValue) >= NUMBER_OF_SOLDIERS_PER_SQUAD) {
    return (CHARACTER_CANT_JOIN_SQUAD_FULL);
  }

  return (CHARACTER_CAN_JOIN_SQUAD);
}

BOOLEAN IsCharacterInTransit(struct SOLDIERTYPE *pSoldier) {
  // valid character?
  if (pSoldier == NULL) {
    return (FALSE);
  }

  // check if character is currently in transit
  if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
    // yep
    return (TRUE);
  }

  // no
  return (FALSE);
}

void UpdateAssignments() {
  int8_t sX, sY, bZ;

  // init sectors with soldiers list
  InitSectorsWithSoldiersList();

  // build list
  BuildSectorsWithSoldiersList();

  // handle natural healing
  HandleNaturalHealing();

  // handle any patients in the hospital
  CheckForAndHandleHospitalPatients();

  // see if any grunt or trainer just lost student/teacher
  ReportTrainersTraineesWithoutPartners();

  // clear out the update list
  ClearSectorListForCompletedTrainingOfMilitia();

  // rest resting mercs, fatigue active mercs,
  // check for mercs tired enough go to sleep, and wake up well-rested mercs
  HandleRestFatigueAndSleepStatus();

#ifdef JA2BETAVERSION
  // put this BEFORE training gets handled to avoid detecting an error everytime a sector completes
  // training
  VerifyTownTrainingIsPaidFor();
#endif

  // run through sectors and handle each type in sector
  for (sX = 0; sX < MAP_WORLD_X; sX++) {
    for (sY = 0; sY < MAP_WORLD_X; sY++) {
      for (bZ = 0; bZ < 4; bZ++) {
        // is there anyone in this sector?
        if (fSectorsWithSoldiers[GetSectorID16(sX, sY)][bZ] == TRUE) {
          // handle any doctors
          HandleDoctorsInSector(sX, sY, bZ);

          // handle any repairmen
          HandleRepairmenInSector(sX, sY, bZ);

          // handle any training
          HandleTrainingInSector(sX, sY, bZ);
        }
      }
    }
  }

  // check to see if anyone is done healing?
  UpdatePatientsWhoAreDoneHealing();

  // check if we have anyone who just finished their assignment
  if (gfAddDisplayBoxToWaitingQueue) {
    AddDisplayBoxToWaitingQueue();
    gfAddDisplayBoxToWaitingQueue = FALSE;
  }

  HandleContinueOfTownTraining();

  // check if anyone is on an assignment where they have nothing to do
  ReEvaluateEveryonesNothingToDo();

  // update mapscreen
  fCharacterInfoPanelDirty = TRUE;
  fTeamPanelDirty = TRUE;
  fMapScreenBottomDirty = TRUE;
}

#ifdef JA2BETAVERSION
void VerifyTownTrainingIsPaidFor(void) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // valid character?
    if (!IsCharListEntryValid(iCounter)) {
      // nope
      continue;
    }

    pSoldier = GetMercFromCharacterList(iCounter);

    if (IsSolActive(pSoldier) && (GetSolAssignment(pSoldier) == TRAIN_TOWN)) {
      // make sure that sector is paid up!
      if (!IsMilitiaTrainingPayedForSectorID8(GetSolSectorID8(pSoldier))) {
        // NOPE!  We've got a bug somewhere
        StopTimeCompression();

        // report the error
        DoScreenIndependantMessageBox(
            L"ERROR: Unpaid militia training. Describe *how* you're re-assigning mercs, how "
            L"many/where/when! Send *prior* save!",
            MSG_BOX_FLAG_OK, NULL);

        // avoid repeating this
        break;
      }
    }
  }
}
#endif

uint8_t FindNumberInSectorWithAssignment(uint8_t sX, uint8_t sY, int8_t bAssignment) {
  // run thought list of characters find number with this assignment
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;
  int8_t bNumberOfPeople = 0;

  // set psoldier as first in merc ptrs
  pSoldier = MercPtrs[0];

  // go through list of characters, find all who are on this assignment
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if ((pTeamSoldier->sSectorX == sX) && (pTeamSoldier->sSectorY == sY) &&
          (pTeamSoldier->bAssignment == bAssignment)) {
        // increment number of people who are on this assignment
        if (pTeamSoldier->bActive) bNumberOfPeople++;
      }
    }
  }

  return (bNumberOfPeople);
}

uint8_t GetNumberThatCanBeDoctored(struct SOLDIERTYPE *pDoctor, BOOLEAN fThisHour,
                                   BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck) {
  int cnt;
  struct SOLDIERTYPE *pSoldier = MercPtrs[0], *pTeamSoldier = NULL;
  uint8_t ubNumberOfPeople = 0;

  // go through list of characters, find all who are patients/doctors healable by this doctor
  for (cnt = 0, pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if (CanSoldierBeHealedByDoctor(pTeamSoldier, pDoctor, FALSE, fThisHour, fSkipKitCheck,
                                     fSkipSkillCheck) == TRUE) {
        // increment number of doctorable patients/doctors
        ubNumberOfPeople++;
      }
    }
  }

  return (ubNumberOfPeople);
}

struct SOLDIERTYPE *AnyDoctorWhoCanHealThisPatient(struct SOLDIERTYPE *pPatient,
                                                   BOOLEAN fThisHour) {
  int cnt;
  struct SOLDIERTYPE *pSoldier = MercPtrs[0], *pTeamSoldier = NULL;

  // go through list of characters, find all who are patients/doctors healable by this doctor
  for (cnt = 0, pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    // doctor?
    if ((pTeamSoldier->bActive) && (pTeamSoldier->bAssignment == DOCTOR)) {
      if (CanSoldierBeHealedByDoctor(pPatient, pTeamSoldier, FALSE, fThisHour, FALSE, FALSE) ==
          TRUE) {
        // found one
        return (pTeamSoldier);
      }
    }
  }

  // there aren't any doctors, or the ones there can't do the job
  return (NULL);
}

uint16_t CalculateHealingPointsForDoctor(struct SOLDIERTYPE *pDoctor, uint16_t *pusMaxPts,
                                         BOOLEAN fMakeSureKitIsInHand) {
  uint16_t usHealPts = 0;
  uint16_t usKitPts = 0;
  int8_t bMedFactor;

  // make sure he has a medkit in his hand, and preferably make it a medical bag, not a first aid
  // kit
  if (fMakeSureKitIsInHand) {
    if (!MakeSureMedKitIsInHand(pDoctor)) {
      return (0);
    }
  }

  // calculate effective doctoring rate (adjusted for drugs, alcohol, etc.)
  usHealPts =
      (EffectiveMedical(pDoctor) * ((EffectiveDexterity(pDoctor) + EffectiveWisdom(pDoctor)) / 2) *
       (100 + (5 * EffectiveExpLevel(pDoctor)))) /
      DOCTORING_RATE_DIVISOR;

  // calculate normal doctoring rate - what it would be if his stats were "normal" (ignoring drugs,
  // fatigue, equipment condition) and equipment was not a hindrance
  *pusMaxPts = (pDoctor->bMedical * ((pDoctor->bDexterity + pDoctor->bWisdom) / 2) *
                (100 + (5 * pDoctor->bExpLevel))) /
               DOCTORING_RATE_DIVISOR;

  // adjust for fatigue
  ReducePointsForFatigue(pDoctor, &usHealPts);

  // count how much medical supplies we have
  usKitPts = 100 * TotalMedicalKitPoints(pDoctor);

  // if we don't have enough medical KIT points, reduce what we can heal
  if (usKitPts < usHealPts) {
    usHealPts = usKitPts;
  }

  // get the type of medkit being used
  bMedFactor = IsMedicalKitItem(&(pDoctor->inv[HANDPOS]));

  if (bMedFactor != 0) {
    // no med kit left?
    // if he's working with only a first aid kit, the doctoring rate is halved!
    // for simplicity, we're ignoring the situation where a nearly empty medical bag in is hand and
    // the rest are just first aid kits
    usHealPts /= bMedFactor;
  } else {
    usHealPts = 0;
  }

  // return healing pts value
  return (usHealPts);
}

uint8_t CalculateRepairPointsForRepairman(struct SOLDIERTYPE *pSoldier, uint16_t *pusMaxPts,
                                          BOOLEAN fMakeSureKitIsInHand) {
  uint16_t usRepairPts;
  uint16_t usKitPts;
  uint8_t ubKitEffectiveness;

  // make sure toolkit in hand?
  if (fMakeSureKitIsInHand) {
    MakeSureToolKitIsInHand(pSoldier);
  }

  // can't repair at all without a toolkit
  if (pSoldier->inv[HANDPOS].usItem != TOOLKIT) {
    *pusMaxPts = 0;
    return (0);
  }

  // calculate effective repair rate (adjusted for drugs, alcohol, etc.)
  usRepairPts = (EffectiveMechanical(pSoldier) * EffectiveDexterity(pSoldier) *
                 (100 + (5 * EffectiveExpLevel(pSoldier)))) /
                (REPAIR_RATE_DIVISOR * ASSIGNMENT_UNITS_PER_DAY);

  // calculate normal repair rate - what it would be if his stats were "normal" (ignoring drugs,
  // fatigue, equipment condition) and equipment was not a hindrance
  *pusMaxPts = (pSoldier->bMechanical * pSoldier->bDexterity * (100 + (5 * pSoldier->bExpLevel))) /
               (REPAIR_RATE_DIVISOR * ASSIGNMENT_UNITS_PER_DAY);

  // adjust for fatigue
  ReducePointsForFatigue(pSoldier, &usRepairPts);

  // figure out what shape his "equipment" is in ("coming" in JA3: Viagra - improves the "shape"
  // your "equipment" is in)
  usKitPts = ToolKitPoints(pSoldier);

  // if kit(s) in extremely bad shape
  if (usKitPts < 25) {
    ubKitEffectiveness = 50;
  }
  // if kit(s) in pretty bad shape
  else if (usKitPts < 50) {
    ubKitEffectiveness = 75;
  } else {
    ubKitEffectiveness = 100;
  }

  // adjust for equipment
  usRepairPts = (usRepairPts * ubKitEffectiveness) / 100;

  // return current repair pts
  return ((uint8_t)usRepairPts);
}

uint16_t ToolKitPoints(struct SOLDIERTYPE *pSoldier) {
  uint16_t usKitpts = 0;
  uint8_t ubPocket;

  // add up kit points
  for (ubPocket = HANDPOS; ubPocket <= SMALLPOCK8POS; ubPocket++) {
    if (pSoldier->inv[ubPocket].usItem == TOOLKIT) {
      usKitpts += TotalPoints(&(pSoldier->inv[ubPocket]));
    }
  }

  return (usKitpts);
}

uint16_t TotalMedicalKitPoints(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubPocket;
  uint16_t usKitpts = 0;

  // add up kit points of all medkits
  for (ubPocket = HANDPOS; ubPocket <= SMALLPOCK8POS; ubPocket++) {
    // NOTE: Here, we don't care whether these are MEDICAL BAGS or FIRST AID KITS!
    if (IsMedicalKitItem(&(pSoldier->inv[ubPocket]))) {
      usKitpts += TotalPoints(&(pSoldier->inv[ubPocket]));
    }
  }

  return (usKitpts);
}

void HandleDoctorsInSector(int16_t sX, int16_t sY, int8_t bZ) {
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  // set psoldier as first in merc ptrs
  pSoldier = MercPtrs[0];

  // will handle doctor/patient relationship in sector

  // go through list of characters, find all doctors in sector
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if ((pTeamSoldier->sSectorX == sX) && (pTeamSoldier->sSectorY == sY) &&
          (pTeamSoldier->bSectorZ == bZ)) {
        if ((pTeamSoldier->bAssignment == DOCTOR) && (pTeamSoldier->fMercAsleep == FALSE)) {
          MakeSureMedKitIsInHand(pTeamSoldier);
          // character is in sector, check if can doctor, if so...heal people
          if (CanCharacterDoctor(pTeamSoldier) && EnoughTimeOnAssignment(pTeamSoldier)) {
            HealCharacters(pTeamSoldier, sX, sY, bZ);
          }
        }
        /*
        if( ( pTeamSoldier -> bAssignment == DOCTOR ) && ( pTeamSoldier->fMercAsleep == FALSE ) )
        {
                MakeSureMedKitIsInHand( pTeamSoldier );
        }

        // character is in sector, check if can doctor, if so...heal people
        if( CanCharacterDoctor( pTeamSoldier ) && ( pTeamSoldier -> bAssignment == DOCTOR ) && (
        pTeamSoldier->fMercAsleep == FALSE ) && EnoughTimeOnAssignment( pTeamSoldier ) )
        {
                HealCharacters( pTeamSoldier, sX, sY, bZ );
        }
        */
      }
    }
  }

  // total healing pts for this sector, now heal people
  return;
}

void UpdatePatientsWhoAreDoneHealing(void) {
  int32_t cnt = 0;
  struct SOLDIERTYPE *pSoldier = NULL, *pTeamSoldier = NULL;

  // set as first in list
  pSoldier = MercPtrs[0];

  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    // active soldier?
    if (pTeamSoldier->bActive) {
      // patient who doesn't need healing
      if ((pTeamSoldier->bAssignment == PATIENT) &&
          (pTeamSoldier->bLife == pTeamSoldier->bLifeMax)) {
        AssignmentDone(pTeamSoldier, TRUE, TRUE);
      }
    }
  }
  return;
}

void HealCharacters(struct SOLDIERTYPE *pDoctor, int16_t sX, int16_t sY, int8_t bZ) {
  // heal all patients in this sector
  uint16_t usAvailableHealingPts = 0;
  uint16_t usRemainingHealingPts = 0;
  uint16_t usUsedHealingPts = 0;
  uint16_t usEvenHealingAmount = 0;
  uint16_t usMax = 0;
  uint8_t ubTotalNumberOfPatients = 0;
  struct SOLDIERTYPE *pSoldier = MercPtrs[0], *pTeamSoldier = NULL, *pWorstHurtSoldier = NULL;
  int32_t cnt = 0;
  uint16_t usOldLeftOvers = 0;

  // now find number of healable mercs in sector that are wounded
  ubTotalNumberOfPatients = GetNumberThatCanBeDoctored(pDoctor, HEALABLE_THIS_HOUR, FALSE, FALSE);

  // if there is anybody who can be healed right now
  if (ubTotalNumberOfPatients > 0) {
    // get available healing pts
    usAvailableHealingPts = CalculateHealingPointsForDoctor(pDoctor, &usMax, TRUE);
    usRemainingHealingPts = usAvailableHealingPts;

    // find how many healing points can be evenly distributed to each wounded, healable merc
    usEvenHealingAmount = usRemainingHealingPts / ubTotalNumberOfPatients;

    // heal each of the healable mercs by this equal amount
    for (cnt = 0, pTeamSoldier = MercPtrs[cnt];
         cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pTeamSoldier++) {
      if (pTeamSoldier->bActive) {
        if (CanSoldierBeHealedByDoctor(pTeamSoldier, pDoctor, FALSE, HEALABLE_THIS_HOUR, FALSE,
                                       FALSE) == TRUE) {
          // can heal and is patient, heal them
          usRemainingHealingPts -= HealPatient(pTeamSoldier, pDoctor, usEvenHealingAmount);
        }
      }
    }

    // if we have any remaining pts
    if (usRemainingHealingPts > 0) {
      // split those up based on need - lowest life patients get them first
      do {
        // find the worst hurt patient
        pWorstHurtSoldier = NULL;

        for (cnt = 0, pTeamSoldier = MercPtrs[cnt];
             cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pTeamSoldier++) {
          if (pTeamSoldier->bActive) {
            if (CanSoldierBeHealedByDoctor(pTeamSoldier, pDoctor, FALSE, HEALABLE_THIS_HOUR, FALSE,
                                           FALSE) == TRUE) {
              if (pWorstHurtSoldier == NULL) {
                pWorstHurtSoldier = pTeamSoldier;
              } else {
                // check to see if this guy is hurt worse than anyone previous?
                if (pTeamSoldier->bLife < pWorstHurtSoldier->bLife) {
                  // he is now the worse hurt guy
                  pWorstHurtSoldier = pTeamSoldier;
                }
              }
            }
          }
        }

        if (pWorstHurtSoldier != NULL) {
          // heal the worst hurt guy
          usOldLeftOvers = usRemainingHealingPts;
          usRemainingHealingPts -= HealPatient(pWorstHurtSoldier, pDoctor, usRemainingHealingPts);

          // couldn't expend any pts, leave
          if (usRemainingHealingPts == usOldLeftOvers) {
            usRemainingHealingPts = 0;
          }
        }
      } while ((usRemainingHealingPts > 0) && (pWorstHurtSoldier != NULL));
    }

    usUsedHealingPts = usAvailableHealingPts - usRemainingHealingPts;

    // increment skills based on healing pts used
    StatChange(pDoctor, MEDICALAMT, (uint16_t)(usUsedHealingPts / 100), FALSE);
    StatChange(pDoctor, DEXTAMT, (uint16_t)(usUsedHealingPts / 200), FALSE);
    StatChange(pDoctor, WISDOMAMT, (uint16_t)(usUsedHealingPts / 200), FALSE);
  }

  // if there's nobody else here who can EVER be helped by this doctor (regardless of whether they
  // got healing this hour)
  if (GetNumberThatCanBeDoctored(pDoctor, HEALABLE_EVER, FALSE, FALSE) == 0) {
    // then this doctor has done all that he can do, but let's find out why and tell player the
    // reason

    // try again, but skip the med kit check!
    if (GetNumberThatCanBeDoctored(pDoctor, HEALABLE_EVER, TRUE, FALSE) > 0) {
      // he could doctor somebody, but can't because he doesn't have a med kit!
      AssignmentAborted(pDoctor, NO_MORE_MED_KITS);
    }
    // try again, but skip the skill check!
    else if (GetNumberThatCanBeDoctored(pDoctor, HEALABLE_EVER, FALSE, TRUE) > 0) {
      // he could doctor somebody, but can't because he doesn't have enough skill!
      AssignmentAborted(pDoctor, INSUF_DOCTOR_SKILL);
    } else {
      // all patients should now be healed - truly DONE!
      AssignmentDone(pDoctor, TRUE, TRUE);
    }
  }
}

/* Assignment distance limits removed.  Sep/11/98.  ARM
BOOLEAN IsSoldierCloseEnoughToADoctor( struct SOLDIERTYPE *pPatient )
{
        // run through all doctors in sector, if it is loaded
        // if no - one is close enough and there is a doctor assigned in sector, inform player
        BOOLEAN fDoctorInSector = FALSE;
        BOOLEAN fDoctorCloseEnough = FALSE;
        struct SOLDIERTYPE *pSoldier = NULL;
        int32_t iCounter = 0;
        wchar_t sString[ 128 ];

        if( ( pPatient->sSectorX != gWorldSectorX ) || ( pPatient->sSectorY != gWorldSectorY ) || (
pPatient->bSectorZ != gbWorldSectorZ ) )
        {
                // not currently loaded
                return( TRUE );
        }

        for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
        {
                pSoldier = &Menptr[ iCounter ];

                if( IsSolActive(pSoldier) )
                {

                        // are they two of these guys in the same sector?
                        if( ( GetSolSectorX(pSoldier) == pPatient->sSectorX ) && (
GetSolSectorY(pSoldier) == pPatient->sSectorY ) && ( GetSolSectorZ(pSoldier) == pPatient->bSectorZ )
)
                        {

                                // is a doctor
                                if( GetSolAssignment(pSoldier) == DOCTOR )
                                {

                                        // the doctor is in the house
                                        fDoctorInSector = TRUE;

                                        // can this patient be healed by the doctor?
                                        if( CanSoldierBeHealedByDoctor( pPatient, pSoldier, TRUE,
HEALABLE_EVER, FALSE, FALSE ) == TRUE )
                                        {
                                                // yep
                                                fDoctorCloseEnough = TRUE;
                                        }
                                }
                        }
                }
        }

        // there are doctors here but noone can heal this guy
        if( ( fDoctorInSector ) && ( fDoctorCloseEnough == FALSE ) )
        {
                swprintf( sString, pDoctorWarningString[ 0 ] , pPatient->name );
                DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL);
                return( FALSE );
        }

        return( TRUE );
}
*/

BOOLEAN CanSoldierBeHealedByDoctor(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pDoctor,
                                   BOOLEAN fIgnoreAssignment, BOOLEAN fThisHour,
                                   BOOLEAN fSkipKitCheck, BOOLEAN fSkipSkillCheck) {
  // must be an active guy
  if (IsSolActive(pSoldier) == FALSE) {
    return (FALSE);
  }

  // must be a patient or a doctor
  if ((pSoldier->bAssignment != PATIENT) && (pSoldier->bAssignment != DOCTOR) &&
      (fIgnoreAssignment == FALSE)) {
    return (FALSE);
  }

  // if dead or unhurt
  if ((pSoldier->bLife == 0) || (pSoldier->bLife == pSoldier->bLifeMax)) {
    return (FALSE);
  }

  // if we care about how long it's been, and he hasn't been on a healable assignment long enough
  if (fThisHour && (EnoughTimeOnAssignment(pSoldier) == FALSE) && (fIgnoreAssignment == FALSE)) {
    return (FALSE);
  }

  // must be in the same sector
  if ((GetSolSectorX(pSoldier) != pDoctor->sSectorX) ||
      (GetSolSectorY(pSoldier) != pDoctor->sSectorY) ||
      (GetSolSectorZ(pSoldier) != pDoctor->bSectorZ)) {
    return (FALSE);
  }

  // can't be between sectors (possible to get here if ignoring assignment)
  if (pSoldier->fBetweenSectors) {
    return (FALSE);
  }

  // if doctor's skill is unsufficient to save this guy
  if (!fSkipSkillCheck && (pDoctor->bMedical < GetMinHealingSkillNeeded(pSoldier))) {
    return (FALSE);
  }

  if (!fSkipKitCheck && (FindObj(pDoctor, MEDICKIT) == NO_SLOT)) {
    // no medical kit to use!
    return (FALSE);
  }

  return (TRUE);
}

uint8_t GetMinHealingSkillNeeded(struct SOLDIERTYPE *pPatient) {
  // get the minimum skill to handle a character under OKLIFE

  if (pPatient->bLife < OKLIFE) {
    // less than ok life, return skill needed
    return (BASE_MEDICAL_SKILL_TO_DEAL_WITH_EMERGENCY +
            (MULTIPLIER_FOR_DIFFERENCE_IN_LIFE_VALUE_FOR_EMERGENCY * (OKLIFE - pPatient->bLife)));
  } else {
    // only need some skill
    return (1);
  }
}

uint16_t HealPatient(struct SOLDIERTYPE *pPatient, struct SOLDIERTYPE *pDoctor,
                     uint16_t usHundredthsHealed) {
  // heal patient and return the number of healing pts used
  uint16_t usHealingPtsLeft;
  uint16_t usTotalFullPtsUsed = 0;
  uint16_t usTotalHundredthsUsed = 0;
  int8_t bPointsToUse = 0;
  int8_t bPointsUsed = 0;
  int8_t bPointsHealed = 0;
  int8_t bPocket = 0;
  int8_t bMedFactor;
  //	int8_t bOldPatientLife = pPatient -> bLife;

  pPatient->sFractLife += usHundredthsHealed;
  usTotalHundredthsUsed =
      usHundredthsHealed;  // we'll subtract any unused amount later if we become fully healed...

  // convert fractions into full points
  usHealingPtsLeft = pPatient->sFractLife / 100;
  pPatient->sFractLife %= 100;

  // if we haven't accumulated any full points yet
  if (usHealingPtsLeft == 0) {
    return (usTotalHundredthsUsed);
  }

  /* ARM - Eliminated.  We now have other methods of properly using doctors to auto-bandage
  bleeding,
  // using the correct kits points instead of this 1 pt. "special"

          // stop all bleeding of patient..for 1 pt (it's fast).  But still use up normal kit pts to
  do it if (pPatient -> bBleeding > 0)
          {
                  usHealingPtsLeft--;
                  usTotalFullPtsUsed++;

                  // get points needed to heal him to dress bleeding wounds
                  bPointsToUse = pPatient -> bBleeding;

                  // go through doctor's pockets and heal, starting at with his in-hand item
                  // the healing pts are based on what type of medkit is in his hand, so we HAVE to
  start there first! for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++)
                  {
                          bMedFactor = IsMedicalKitItem( &( pDoctor -> inv[ bPocket ] ) );
                          if ( bMedFactor > 0 )
                          {
                                  // ok, we have med kit in this pocket, use it

                                  // The medFactor here doesn't affect how much the doctor can heal
  (that's already factored into lower healing pts)
                                  // but it does effect how fast the medkit is used up!  First aid
  kits disappear at double their doctoring rate! bPointsUsed = (int8_t) UseKitPoints( &( pDoctor ->
  inv[ bPocket ] ), (uint16_t) (bPointsToUse * bMedFactor), pDoctor ); bPointsHealed = bPointsUsed /
  bMedFactor;

                                  bPointsToUse -= bPointsHealed;
                                  pPatient -> bBleeding -= bPointsHealed;

                                  // if we're done all we're supposed to, or the guy's no longer
  bleeding, bail if ( ( bPointsToUse <= 0 ) || ( pPatient -> bBleeding == 0 ) )
                                  {
                                          break;
                                  }
                          }
                  }
          }
  */

  // if below ok life, heal these first at double point cost
  if (pPatient->bLife < OKLIFE) {
    // get points needed to heal him to OKLIFE
    bPointsToUse = POINT_COST_PER_HEALTH_BELOW_OKLIFE * (OKLIFE - pPatient->bLife);

    // if he needs more than we have, reduce to that
    if (bPointsToUse > usHealingPtsLeft) {
      bPointsToUse = (int8_t)usHealingPtsLeft;
    }

    // go through doctor's pockets and heal, starting at with his in-hand item
    // the healing pts are based on what type of medkit is in his hand, so we HAVE to start there
    // first!
    for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
      bMedFactor = IsMedicalKitItem(&(pDoctor->inv[bPocket]));
      if (bMedFactor > 0) {
        // ok, we have med kit in this pocket, use it

        // The medFactor here doesn't affect how much the doctor can heal (that's already factored
        // into lower healing pts) but it does effect how fast the medkit is used up!  First aid
        // kits disappear at double their doctoring rate!
        bPointsUsed = (int8_t)UseKitPoints(&(pDoctor->inv[bPocket]),
                                           (uint16_t)(bPointsToUse * bMedFactor), pDoctor);
        bPointsHealed = bPointsUsed / bMedFactor;

        bPointsToUse -= bPointsHealed;
        usHealingPtsLeft -= bPointsHealed;
        usTotalFullPtsUsed += bPointsHealed;

        // heal person the amount / POINT_COST_PER_HEALTH_BELOW_OKLIFE
        pPatient->bLife += (bPointsHealed / POINT_COST_PER_HEALTH_BELOW_OKLIFE);

        // if we're done all we're supposed to, or the guy's at OKLIFE, bail
        if ((bPointsToUse <= 0) || (pPatient->bLife >= OKLIFE)) {
          break;
        }
      }
    }
  }

  // critical conditions handled, now apply normal healing

  if (pPatient->bLife < pPatient->bLifeMax) {
    bPointsToUse = (pPatient->bLifeMax - pPatient->bLife);

    // if guy is hurt more than points we have...heal only what we have
    if (bPointsToUse > usHealingPtsLeft) {
      bPointsToUse = (int8_t)usHealingPtsLeft;
    }

    // go through doctor's pockets and heal, starting at with his in-hand item
    // the healing pts are based on what type of medkit is in his hand, so we HAVE to start there
    // first!
    for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
      bMedFactor = IsMedicalKitItem(&(pDoctor->inv[bPocket]));
      if (bMedFactor > 0) {
        // ok, we have med kit in this pocket, use it  (use only half if it's worth double)

        // The medFactor here doesn't affect how much the doctor can heal (that's already factored
        // into lower healing pts) but it does effect how fast the medkit is used up!  First aid
        // kits disappear at double their doctoring rate!
        bPointsUsed = (int8_t)UseKitPoints(&(pDoctor->inv[bPocket]),
                                           (uint16_t)(bPointsToUse * bMedFactor), pDoctor);
        bPointsHealed = bPointsUsed / bMedFactor;

        bPointsToUse -= bPointsHealed;
        usHealingPtsLeft -= bPointsHealed;
        usTotalFullPtsUsed += bPointsHealed;

        pPatient->bLife += bPointsHealed;

        // if we're done all we're supposed to, or the guy's fully healed, bail
        if ((bPointsToUse <= 0) || (pPatient->bLife == pPatient->bLifeMax)) {
          break;
        }
      }
    }
  }

  // if this patient is fully healed
  if (pPatient->bLife == pPatient->bLifeMax) {
    // don't count unused full healing points as being used
    usTotalHundredthsUsed -= (100 * usHealingPtsLeft);

    // wipe out fractions of extra life, and DON'T count them as used
    usTotalHundredthsUsed -= pPatient->sFractLife;
    pPatient->sFractLife = 0;

    /* ARM Removed.  This is duplicating the check in UpdatePatientsWhoAreDoneHealing(), guy would
       show up twice!
                    // if it isn't the doctor himself)
                    if( ( pPatient != pDoctor )
                    {
                            AssignmentDone( pPatient, TRUE, TRUE );
                    }
    */
  }

  return (usTotalHundredthsUsed);
}

void CheckForAndHandleHospitalPatients(void) {
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  if (fSectorsWithSoldiers[GetSectorID16(HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y)][0] == FALSE) {
    // nobody in the hospital sector... leave
    return;
  }

  // set pSoldier as first in merc ptrs
  pSoldier = MercPtrs[0];

  // go through list of characters, find all who are on this assignment
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if (pTeamSoldier->bAssignment == ASSIGNMENT_HOSPITAL) {
        if ((pTeamSoldier->sSectorX == HOSPITAL_SECTOR_X) &&
            (pTeamSoldier->sSectorY == HOSPITAL_SECTOR_Y) && (pTeamSoldier->bSectorZ == 0)) {
          // heal this character
          HealHospitalPatient(pTeamSoldier, HOSPITAL_HEALING_RATE);
        }
      }
    }
  }
}

void HealHospitalPatient(struct SOLDIERTYPE *pPatient, uint16_t usHealingPtsLeft) {
  int8_t bPointsToUse;

  if (usHealingPtsLeft <= 0) {
    return;
  }

  /*  Stopping hospital patients' bleeding must be handled immediately, not during a regular hourly
     check
          // stop all bleeding of patient..for 1 pt.
          if (pPatient -> bBleeding > 0)
          {
                  usHealingPtsLeft--;
                  pPatient -> bBleeding = 0;
          }
  */

  // if below ok life, heal these first at double cost
  if (pPatient->bLife < OKLIFE) {
    // get points needed to heal him to OKLIFE
    bPointsToUse = POINT_COST_PER_HEALTH_BELOW_OKLIFE * (OKLIFE - pPatient->bLife);

    // if he needs more than we have, reduce to that
    if (bPointsToUse > usHealingPtsLeft) {
      bPointsToUse = (int8_t)usHealingPtsLeft;
    }

    usHealingPtsLeft -= bPointsToUse;

    // heal person the amount / POINT_COST_PER_HEALTH_BELOW_OKLIFE
    pPatient->bLife += (bPointsToUse / POINT_COST_PER_HEALTH_BELOW_OKLIFE);
  }

  // critical condition handled, now solve normal healing

  if (pPatient->bLife < pPatient->bLifeMax) {
    bPointsToUse = (pPatient->bLifeMax - pPatient->bLife);

    // if guy is hurt more than points we have...heal only what we have
    if (bPointsToUse > usHealingPtsLeft) {
      bPointsToUse = (int8_t)usHealingPtsLeft;
    }

    usHealingPtsLeft -= bPointsToUse;

    // heal person the amount
    pPatient->bLife += bPointsToUse;
  }

  // if this patient is fully healed
  if (pPatient->bLife == pPatient->bLifeMax) {
    AssignmentDone(pPatient, TRUE, TRUE);
  }
}

void HandleRepairmenInSector(int16_t sX, int16_t sY, int8_t bZ) {
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  // set psoldier as first in merc ptrs
  pSoldier = MercPtrs[0];

  // will handle doctor/patient relationship in sector

  // go through list of characters, find all doctors in sector
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if ((pTeamSoldier->sSectorX == sX) && (pTeamSoldier->sSectorY == sY) &&
          (pTeamSoldier->bSectorZ == bZ)) {
        if ((pTeamSoldier->bAssignment == REPAIR) && (pTeamSoldier->fMercAsleep == FALSE)) {
          MakeSureToolKitIsInHand(pTeamSoldier);
          // character is in sector, check if can repair
          if (CanCharacterRepair(pTeamSoldier) && (EnoughTimeOnAssignment(pTeamSoldier))) {
            HandleRepairBySoldier(pTeamSoldier);
          }
        }
      }
    }
  }

  return;
}

/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
int8_t HandleRepairOfSAMSite( struct SOLDIERTYPE *pSoldier, int8_t bPointsAvailable, BOOLEAN *
pfNothingLeftToRepair )
{
        int8_t bPtsUsed = 0;
        int16_t sStrategicSector = 0;

        if( IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier -> bSectorZ
) == FALSE )
        {
                return( bPtsUsed );
        }
        else if( ( pSoldier -> sSectorX == gWorldSectorX ) && ( pSoldier -> bSectorZ ==
gbWorldSectorZ )&&( pSoldier -> sSectorY == gWorldSectorY ) )
        {
                if( CanSoldierRepairSAM( pSoldier, bPointsAvailable ) == FALSE )
                {
                        return( bPtsUsed );
                }
        }

        // repair the SAM

        sStrategicSector = GetSectorID16( GetSolSectorX(pSoldier),
GetSolSectorY(pSoldier) );

        // do we have more than enough?
        if( 100 - StrategicMap[ sStrategicSector ].bSAMCondition >= bPointsAvailable /
SAM_SITE_REPAIR_DIVISOR )
        {
                // no, use up all we have
                StrategicMap[ sStrategicSector ].bSAMCondition += bPointsAvailable /
SAM_SITE_REPAIR_DIVISOR; bPtsUsed = bPointsAvailable - ( bPointsAvailable % SAM_SITE_REPAIR_DIVISOR
);

                // SAM site may have been put back into working order...
                UpdateAirspaceControl( );
        }
        else
        {
                // yep
                bPtsUsed = SAM_SITE_REPAIR_DIVISOR * ( 100 - StrategicMap[ sStrategicSector
].bSAMCondition ); StrategicMap[ sStrategicSector ].bSAMCondition = 100;

//ARM: NOTE THAT IF THIS CODE IS EVER RE-ACTIVATED, THE SAM GRAPHICS SHOULD CHANGE NOT WHEN THE SAM
SITE RETURNS TO
// FULL STRENGTH (condition 100), but as soon as it reaches MIN_CONDITION_TO_FIX_SAM!!!

                // Bring Hit points back up to full, adjust graphic to full graphic.....
                UpdateSAMDoneRepair( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier ->
bSectorZ );
        }

        if ( StrategicMap[ sStrategicSector ].bSAMCondition == 100 )
        {
                *pfNothingLeftToRepair = TRUE;
        }
        else
        {
                *pfNothingLeftToRepair = FALSE;
        }
        return( bPtsUsed );
}
*/

int8_t FindRepairableItemOnOtherSoldier(struct SOLDIERTYPE *pSoldier, uint8_t ubPassType) {
  int8_t bLoop, bLoop2;
  REPAIR_PASS_SLOTS_TYPE *pPassList;
  int8_t bSlotToCheck;
  struct OBJECTTYPE *pObj;

  Assert(ubPassType < NUM_REPAIR_PASS_TYPES);

  pPassList = &(gRepairPassSlotList[ubPassType]);

  for (bLoop = 0; bLoop < pPassList->ubChoices; bLoop++) {
    bSlotToCheck = pPassList->bSlot[bLoop];
    Assert(bSlotToCheck != -1);

    pObj = &(pSoldier->inv[bSlotToCheck]);
    for (bLoop2 = 0; bLoop2 < pSoldier->inv[bSlotToCheck].ubNumberOfObjects; bLoop2++) {
      if (IsItemRepairable(pObj->usItem, pObj->bStatus[bLoop2])) {
        return (bSlotToCheck);
      }
    }

    // have to check for attachments...
    for (bLoop2 = 0; bLoop2 < MAX_ATTACHMENTS; bLoop2++) {
      if (pObj->usAttachItem[bLoop2] != NOTHING) {
        if (IsItemRepairable(pObj->usAttachItem[bLoop2], pObj->bAttachStatus[bLoop2])) {
          return (bSlotToCheck);
        }
      }
    }
  }

  return (NO_SLOT);
}

void DoActualRepair(struct SOLDIERTYPE *pSoldier, uint16_t usItem, int8_t *pbStatus,
                    uint8_t *pubRepairPtsLeft) {
  int16_t sRepairCostAdj;
  uint16_t usDamagePts, usPtsFixed;

  // get item's repair ease, for each + point is 10% easier, each - point is 10% harder to repair
  sRepairCostAdj = 100 - (10 * Item[usItem].bRepairEase);

  // make sure it ain't somehow gone too low!
  if (sRepairCostAdj < 10) {
    sRepairCostAdj = 10;
  }

  // repairs on electronic items take twice as long if the guy doesn't have the skill
  if ((Item[usItem].fFlags & ITEM_ELECTRONIC) && (!(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS)))) {
    sRepairCostAdj *= 2;
  }

  // how many points of damage is the item down by?
  usDamagePts = 100 - *pbStatus;

  // adjust that by the repair cost adjustment percentage
  usDamagePts = (usDamagePts * sRepairCostAdj) / 100;

  // do we have enough pts to fully repair the item?
  if (*pubRepairPtsLeft >= usDamagePts) {
    // fix it to 100%
    *pbStatus = 100;
    *pubRepairPtsLeft -= usDamagePts;
  } else  // not enough, partial fix only, if any at all
  {
    // fix what we can - add pts left adjusted by the repair cost
    usPtsFixed = (*pubRepairPtsLeft * 100) / sRepairCostAdj;

    // if we have enough to actually fix anything
    // NOTE: a really crappy repairman with only 1 pt/hr CAN'T repair electronics or difficult
    // items!
    if (usPtsFixed > 0) {
      *pbStatus += usPtsFixed;

      // make sure we don't somehow end up over 100
      if (*pbStatus > 100) {
        *pbStatus = 100;
      }
    }

    *pubRepairPtsLeft = 0;
  }
}

BOOLEAN RepairObject(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pOwner,
                     struct OBJECTTYPE *pObj, uint8_t *pubRepairPtsLeft) {
  uint8_t ubLoop, ubItemsInPocket;
  BOOLEAN fSomethingWasRepaired = FALSE;

  ubItemsInPocket = pObj->ubNumberOfObjects;

  for (ubLoop = 0; ubLoop < ubItemsInPocket; ubLoop++) {
    // if it's repairable and NEEDS repairing
    if (IsItemRepairable(pObj->usItem, pObj->bStatus[ubLoop])) {
      // repairable, try to repair it

      // void DoActualRepair( struct SOLDIERTYPE * pSoldier, uint16_t usItem, int8_t * pbStatus,
      // uint8_t * pubRepairPtsLeft )
      DoActualRepair(pSoldier, pObj->usItem, &(pObj->bStatus[ubLoop]), pubRepairPtsLeft);

      fSomethingWasRepaired = TRUE;

      if (pObj->bStatus[ubLoop] == 100) {
        // report it as fixed
        if (pSoldier == pOwner) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_REPAIRED], pSoldier->name,
                    ItemNames[pObj->usItem]);
        } else {
          // NOTE: may need to be changed for localized versions
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[35], pSoldier->name,
                    pOwner->name, ItemNames[pObj->usItem]);
        }
      }

      if (*pubRepairPtsLeft == 0) {
        // we're out of points!
        break;
      }
    }
  }

  // now check for attachments
  for (ubLoop = 0; ubLoop < MAX_ATTACHMENTS; ubLoop++) {
    if (pObj->usAttachItem[ubLoop] != NOTHING) {
      if (IsItemRepairable(pObj->usAttachItem[ubLoop], pObj->bAttachStatus[ubLoop])) {
        // repairable, try to repair it

        DoActualRepair(pSoldier, pObj->usAttachItem[ubLoop], &(pObj->bAttachStatus[ubLoop]),
                       pubRepairPtsLeft);

        fSomethingWasRepaired = TRUE;

        if (pObj->bAttachStatus[ubLoop] == 100) {
          // report it as fixed
          if (pSoldier == pOwner) {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_REPAIRED], pSoldier->name,
                      ItemNames[pObj->usAttachItem[ubLoop]]);
          } else {
            // NOTE: may need to be changed for localized versions
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[35],
                      pSoldier->name, pOwner->name, ItemNames[pObj->usAttachItem[ubLoop]]);
          }
        }

        if (*pubRepairPtsLeft == 0) {
          // we're out of points!
          break;
        }
      }
    }
  }

  return (fSomethingWasRepaired);
}

void HandleRepairBySoldier(struct SOLDIERTYPE *pSoldier) {
  uint16_t usMax = 0;
  uint8_t ubRepairPtsLeft = 0;
  uint8_t ubInitialRepairPts = 0;
  uint8_t ubRepairPtsUsed = 0;
  int8_t bPocket = 0;
  BOOLEAN fNothingLeftToRepair = FALSE;
  int8_t bLoop, bLoopStart, bLoopEnd;
  BOOLEAN fAnyOfSoldiersOwnItemsWereFixed = FALSE;
  struct OBJECTTYPE *pObj;

  // grab max number of repair pts open to this soldier
  ubRepairPtsLeft = CalculateRepairPointsForRepairman(pSoldier, &usMax, TRUE);

  // no points
  if (ubRepairPtsLeft == 0) {
    AssignmentDone(pSoldier, TRUE, TRUE);
    return;
  }

  // remember what we've started off with
  ubInitialRepairPts = ubRepairPtsLeft;

  // check if we are repairing a vehicle
  if (pSoldier->bVehicleUnderRepairID != -1) {
    if (CanCharacterRepairVehicle(pSoldier, pSoldier->bVehicleUnderRepairID)) {
      // attempt to fix vehicle
      ubRepairPtsLeft -=
          RepairVehicle(pSoldier->bVehicleUnderRepairID, ubRepairPtsLeft, &fNothingLeftToRepair);
    }
  }
  // check if we are repairing a robot
  else if (pSoldier->fFixingRobot) {
    if (CanCharacterRepairRobot(pSoldier)) {
      // repairing the robot is very slow & difficult
      ubRepairPtsLeft /= 2;
      ubInitialRepairPts /= 2;

      if (!(HAS_SKILL_TRAIT(pSoldier, ELECTRONICS))) {
        ubRepairPtsLeft /= 2;
        ubInitialRepairPts /= 2;
      }

      // robot
      ubRepairPtsLeft -=
          HandleRepairOfRobotBySoldier(pSoldier, ubRepairPtsLeft, &fNothingLeftToRepair);
    }
  } else {
    fAnyOfSoldiersOwnItemsWereFixed = UnjamGunsOnSoldier(pSoldier, pSoldier, &ubRepairPtsLeft);

    // repair items on self
    for (bLoop = 0; bLoop < 2; bLoop++) {
      if (bLoop == 0) {
        bLoopStart = SECONDHANDPOS;
        bLoopEnd = SMALLPOCK8POS;
      } else {
        bLoopStart = HELMETPOS;
        bLoopEnd = HEAD2POS;
      }

      // now repair objects running from left hand to small pocket
      for (bPocket = bLoopStart; bPocket <= bLoopEnd; bPocket++) {
        pObj = &(pSoldier->inv[bPocket]);

        if (RepairObject(pSoldier, pSoldier, pObj, &ubRepairPtsLeft)) {
          fAnyOfSoldiersOwnItemsWereFixed = TRUE;

          // quit looking if we're already out
          if (ubRepairPtsLeft == 0) break;
        }
      }
    }

    // if he fixed something of his, and now has no more of his own items to fix
    if (fAnyOfSoldiersOwnItemsWereFixed && !DoesCharacterHaveAnyItemsToRepair(pSoldier, -1)) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sRepairsDoneString[0], pSoldier->name);

      // let player react
      StopTimeCompression();
    }

    // repair items on others
    RepairItemsOnOthers(pSoldier, &ubRepairPtsLeft);
  }

  // what are the total amount of pts used by character?
  ubRepairPtsUsed = ubInitialRepairPts - ubRepairPtsLeft;
  if (ubRepairPtsUsed > 0) {
    // improve stats
    StatChange(pSoldier, MECHANAMT, (uint16_t)(ubRepairPtsUsed / 2), FALSE);
    StatChange(pSoldier, DEXTAMT, (uint16_t)(ubRepairPtsUsed / 2), FALSE);

    // check if kit damaged/depleted
    if ((Random(100)) <
        (uint32_t)(ubRepairPtsUsed *
                   5))  // CJC: added a x5 as this wasn't going down anywhere fast enough
    {
      // kit item damaged/depleted, burn up points of toolkit..which is in right hand
      UseKitPoints(&(pSoldier->inv[HANDPOS]), 1, pSoldier);
    }
  }

  // if he really done
  if (HasCharacterFinishedRepairing(pSoldier)) {
    // yup, that's all folks
    AssignmentDone(pSoldier, TRUE, TRUE);
  } else  // still has stuff to repair
  {
    // if nothing got repaired, there's a problem
    if (ubRepairPtsUsed == 0) {
      // see if not having a toolkit is the problem
      if (FindObj(pSoldier, TOOLKIT) == NO_SLOT) {
        // he could (maybe) repair something, but can't because he doesn't have a tool kit!
        AssignmentAborted(pSoldier, NO_MORE_TOOL_KITS);
      } else {
        // he can't repair anything because he doesn't have enough skill!
        AssignmentAborted(pSoldier, INSUF_REPAIR_SKILL);
      }
    }
  }

  return;
}

BOOLEAN IsItemRepairable(uint16_t usItem, int8_t bStatus) {
  // check to see if item can/needs to be repaired
  if ((bStatus < 100) && (Item[usItem].fFlags & ITEM_REPAIRABLE)) {
    // yep
    return (TRUE);
  }

  // nope
  return (FALSE);
}

void HandleRestAndFatigueInSector(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ) {
  // this will handle all sleeping characters in this sector
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  pSoldier = MercPtrs[0];

  // go through list of characters, find all sleepers in sector
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if ((pTeamSoldier->bActive) && (pSoldier->bAssignment != ASSIGNMENT_POW)) {
      if ((pTeamSoldier->sSectorX == sMapX) && (pTeamSoldier->sSectorY == sMapY) &&
          (pTeamSoldier->bSectorZ == bMapZ)) {
      }
    }
  }
}

/*
int8_t GetRegainDueToSleepNeeded( struct SOLDIERTYPE *pSoldier, int32_t iRateOfReGain )
{
        // look at persons regain rate,
        // if they infact loses sleep, make sure it doesn't go below the current rate
        int8_t bRate = 0;
        uint8_t ubNeedForSleep = 0;

        // get profile id and then grab sleep need value
        ubNeedForSleep = gMercProfiles[ pSoldier -> ubProfile ].ubNeedForSleep;

        bRate = ( AVG_NUMBER_OF_HOURS_OF_SLEEP_NEEDED - ( int8_t )ubNeedForSleep );

        if( bRate >= iRateOfReGain )
        {
                bRate = ( - iRateOfReGain ) + 1;
        }
        return( bRate );
}
*/

void RestCharacter(struct SOLDIERTYPE *pSoldier) {
  // handle the sleep of this character, update bBreathMax based on sleep they have
  int8_t bMaxBreathRegain = 0;

  bMaxBreathRegain = 50 / CalcSoldierNeedForSleep(pSoldier);

  // if breath max is below the "really tired" threshold
  if (pSoldier->bBreathMax < BREATHMAX_PRETTY_TIRED) {
    // real tired, rest rate is 50% higher (this is to prevent absurdly long sleep times for totally
    // exhausted mercs)
    bMaxBreathRegain = (bMaxBreathRegain * 3 / 2);
  }

  pSoldier->bBreathMax += bMaxBreathRegain;

  if (pSoldier->bBreathMax > 100) {
    pSoldier->bBreathMax = 100;
  } else if (pSoldier->bBreathMax < BREATHMAX_ABSOLUTE_MINIMUM) {
    pSoldier->bBreathMax = BREATHMAX_ABSOLUTE_MINIMUM;
  }

  pSoldier->bBreath = pSoldier->bBreathMax;

  if (pSoldier->bBreathMax >= BREATHMAX_CANCEL_TIRED) {
    pSoldier->fComplainedThatTired = FALSE;
  }

  return;
}

void FatigueCharacter(struct SOLDIERTYPE *pSoldier) {
  // fatigue character
  int32_t iPercentEncumbrance;
  int32_t iBreathLoss;
  int8_t bMaxBreathLoss = 0, bDivisor = 1;

  // vehicle or robot?
  if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
    return;
  }

  // check if in transit, do not wear out
  if (IsCharacterInTransit(pSoldier) == TRUE) {
    return;
  }

  // POW?
  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    return;
  }

  bDivisor = 24 - CalcSoldierNeedForSleep(pSoldier);
  bMaxBreathLoss = 50 / bDivisor;

  if (bMaxBreathLoss < 2) {
    bMaxBreathLoss = 2;
  }

  // KM: Added encumbrance calculation to soldiers moving on foot.  Anything above 100% will
  // increase
  //    rate of fatigue.  200% encumbrance will cause soldiers to tire twice as quickly.
  if (pSoldier->fBetweenSectors &&
      pSoldier->bAssignment !=
          VEHICLE) {  // Soldier is on foot and travelling.  Factor encumbrance into fatigue rate.
    iPercentEncumbrance = CalculateCarriedWeight(pSoldier);
    if (iPercentEncumbrance > 100) {
      iBreathLoss = (bMaxBreathLoss * iPercentEncumbrance / 100);
      bMaxBreathLoss = (int8_t)min(127, iBreathLoss);
    }
  }

  // if breath max is below the "really tired" threshold
  if (pSoldier->bBreathMax < BREATHMAX_PRETTY_TIRED) {
    // real tired, fatigue rate is 50% higher
    bMaxBreathLoss = (bMaxBreathLoss * 3 / 2);
  }

  pSoldier->bBreathMax -= bMaxBreathLoss;

  if (pSoldier->bBreathMax > 100) {
    pSoldier->bBreathMax = 100;
  } else if (pSoldier->bBreathMax < BREATHMAX_ABSOLUTE_MINIMUM) {
    pSoldier->bBreathMax = BREATHMAX_ABSOLUTE_MINIMUM;
  }

  // current breath can't exceed maximum
  if (pSoldier->bBreath > pSoldier->bBreathMax) {
    pSoldier->bBreath = pSoldier->bBreathMax;
  }

  return;
}

// ONCE PER HOUR, will handle ALL kinds of training (self, teaching, and town) in this sector
void HandleTrainingInSector(uint8_t sMapX, uint8_t sMapY, int8_t bZ) {
  struct SOLDIERTYPE *pTrainer;
  struct SOLDIERTYPE *pStudent;
  uint8_t ubStat;
  BOOLEAN fAtGunRange = FALSE;
  uint32_t uiCnt = 0;
  int16_t sTotalTrainingPts = 0;
  uint16_t sTrainingPtsDueToInstructor = 0;
  struct SOLDIERTYPE *pStatTrainerList[NUM_TRAINABLE_STATS];  // can't have more "best" trainers
                                                              // than trainable stats
  int16_t sBestTrainingPts;
  int16_t sTownTrainingPts;
  TOWN_TRAINER_TYPE TownTrainer[MAX_CHARACTER_COUNT];
  uint8_t ubTownTrainers;
  uint16_t usMaxPts;
  BOOLEAN fSamSiteInSector = FALSE;
  BOOLEAN fTrainingCompleted = FALSE;

  // find out if a sam site here
  fSamSiteInSector = IsThisSectorASAMSector(sMapX, sMapY, 0);

  // Training in underground sectors is disallowed by the interface code, so there should never be
  // any
  if (bZ != 0) {
    return;
  }

  // if sector not under our control, has enemies in it, or is currently in combat mode
  if (!SectorOursAndPeaceful(sMapX, sMapY, bZ)) {
    // then training is canceled for this hour.
    // This is partly logical, but largely to prevent newly trained militia from appearing in
    // mid-battle
    return;
  }

  // are we training in the sector with gun range in Alma?
  if ((sMapX == GUN_RANGE_X) && (sMapY == GUN_RANGE_Y) && (bZ == GUN_RANGE_Z)) {
    fAtGunRange = TRUE;
  }

  // init trainer list
  memset(pStatTrainerList, 0, sizeof(pStatTrainerList));

  // build list of teammate trainers in this sector.

  // Only the trainer with the HIGHEST training ability in each stat is effective.  This is mainly
  // to avoid having to sort them from highest to lowest if some form of trainer degradation formula
  // was to be used for multiple trainers.

  // for each trainable stat
  for (ubStat = 0; ubStat < NUM_TRAINABLE_STATS; ubStat++) {
    sBestTrainingPts = -1;

    // search team for active instructors in this sector
    for (uiCnt = 0, pTrainer = MercPtrs[uiCnt];
         uiCnt <= gTacticalStatus.Team[MercPtrs[0]->bTeam].bLastID; uiCnt++, pTrainer++) {
      if (pTrainer->bActive && (pTrainer->sSectorX == sMapX) && (pTrainer->sSectorY == sMapY) &&
          (pTrainer->bSectorZ == bZ)) {
        // if he's training teammates in this stat
        if ((pTrainer->bAssignment == TRAIN_TEAMMATE) && (pTrainer->bTrainStat == ubStat) &&
            (EnoughTimeOnAssignment(pTrainer)) && (pTrainer->fMercAsleep == FALSE)) {
          sTrainingPtsDueToInstructor =
              GetBonusTrainingPtsDueToInstructor(pTrainer, NULL, ubStat, fAtGunRange, &usMaxPts);

          // if he's the best trainer so far for this stat
          if (sTrainingPtsDueToInstructor > sBestTrainingPts) {
            // then remember him as that, and the points he scored
            pStatTrainerList[ubStat] = pTrainer;
            sBestTrainingPts = sTrainingPtsDueToInstructor;
          }
        }
      }
    }
  }

  // now search team for active self-trainers in this sector
  for (uiCnt = 0, pStudent = MercPtrs[uiCnt];
       uiCnt <= gTacticalStatus.Team[MercPtrs[0]->bTeam].bLastID; uiCnt++, pStudent++) {
    // see if this merc is active and in the same sector
    if ((pStudent->bActive) && (pStudent->sSectorX == sMapX) && (pStudent->sSectorY == sMapY) &&
        (pStudent->bSectorZ == bZ)) {
      // if he's training himself (alone, or by others), then he's a student
      if ((pStudent->bAssignment == TRAIN_SELF) || (pStudent->bAssignment == TRAIN_BY_OTHER)) {
        if (EnoughTimeOnAssignment(pStudent) && (pStudent->fMercAsleep == FALSE)) {
          // figure out how much the grunt can learn in one training period
          sTotalTrainingPts =
              GetSoldierTrainingPts(pStudent, pStudent->bTrainStat, fAtGunRange, &usMaxPts);

          // if he's getting help
          if (pStudent->bAssignment == TRAIN_BY_OTHER) {
            // grab the pointer to the (potential) trainer for this stat
            pTrainer = pStatTrainerList[pStudent->bTrainStat];

            // if this stat HAS a trainer in sector at all
            if (pTrainer != NULL) {
              /* Assignment distance limits removed.  Sep/11/98.  ARM
                                                                      // if this sector either ISN'T
                 currently loaded, or it is but the trainer is close enough to the student if ( (
                 sMapX != gWorldSectorX ) || ( sMapY != gWorldSectorY ) || ( pStudent -> bSectorZ !=
                 gbWorldSectorZ ) || ( PythSpacesAway( pStudent -> sGridNo, pTrainer -> sGridNo ) <
                 MAX_DISTANCE_FOR_TRAINING ) && ( EnoughTimeOnAssignment( pTrainer ) ) )
              */
              // NB this EnoughTimeOnAssignment() call is redundent since it is called up above
              // if ( EnoughTimeOnAssignment( pTrainer ) )
              {
                // valid trainer is available, this gives the student a large training bonus!
                sTrainingPtsDueToInstructor = GetBonusTrainingPtsDueToInstructor(
                    pTrainer, pStudent, pStudent->bTrainStat, fAtGunRange, &usMaxPts);

                // add the bonus to what merc can learn on his own
                sTotalTrainingPts += sTrainingPtsDueToInstructor;
              }
            }
          }

          // now finally train the grunt
          TrainSoldierWithPts(pStudent, sTotalTrainingPts);
        }
      }
    }
  }

  // check if we're doing a sector where militia can be trained
  if (((StrategicMap[GetSectorID16(sMapX, (sMapY))].townID != BLANK_SECTOR) ||
       (fSamSiteInSector == TRUE)) &&
      (bZ == 0)) {
    // init town trainer list
    memset(TownTrainer, 0, sizeof(TownTrainer));
    ubTownTrainers = 0;

    // build list of all the town trainers in this sector and their training pts
    for (uiCnt = 0, pTrainer = MercPtrs[uiCnt];
         uiCnt <= gTacticalStatus.Team[MercPtrs[0]->bTeam].bLastID; uiCnt++, pTrainer++) {
      if (pTrainer->bActive && (pTrainer->sSectorX == sMapX) && (pTrainer->sSectorY == sMapY) &&
          (pTrainer->bSectorZ == bZ)) {
        if ((pTrainer->bAssignment == TRAIN_TOWN) && (EnoughTimeOnAssignment(pTrainer)) &&
            (pTrainer->fMercAsleep == FALSE)) {
          sTownTrainingPts = GetTownTrainPtsForCharacter(pTrainer, &usMaxPts);

          // if he's actually worth anything
          if (sTownTrainingPts > 0) {
            // remember this guy as a town trainer
            TownTrainer[ubTownTrainers].sTrainingPts = sTownTrainingPts;
            TownTrainer[ubTownTrainers].pSoldier = pTrainer;
            ubTownTrainers++;
          }
        }
      }
    }

    // if we have more than one
    if (ubTownTrainers > 1) {
      // sort the town trainer list from best trainer to worst
      qsort(TownTrainer, ubTownTrainers, sizeof(TOWN_TRAINER_TYPE), TownTrainerQsortCompare);
    }

    // for each trainer, in sorted order from the best to the worst
    for (uiCnt = 0; uiCnt < ubTownTrainers; uiCnt++) {
      // top trainer has full effect (divide by 1), then divide by 2, 4, 8, etc.
      // sTownTrainingPts = TownTrainer[ uiCnt ].sTrainingPts / (uint16_t) pow(2, uiCnt);
      // CJC: took this out and replaced with limit of 2 guys per sector
      sTownTrainingPts = TownTrainer[uiCnt].sTrainingPts;

      if (sTownTrainingPts > 0) {
        fTrainingCompleted =
            TrainTownInSector(TownTrainer[uiCnt].pSoldier, sMapX, sMapY, sTownTrainingPts);

        if (fTrainingCompleted) {
          // there's no carryover into next session for extra training (cause player might cancel),
          // so break out of loop
          break;
        }
      }
    }
  }
}

int TownTrainerQsortCompare(const void *pArg1, const void *pArg2) {
  if (((TOWN_TRAINER_TYPE *)pArg1)->sTrainingPts > ((TOWN_TRAINER_TYPE *)pArg2)->sTrainingPts) {
    return (-1);
  } else if (((TOWN_TRAINER_TYPE *)pArg1)->sTrainingPts <
             ((TOWN_TRAINER_TYPE *)pArg2)->sTrainingPts) {
    return (1);
  } else {
    return (0);
  }
}

int16_t GetBonusTrainingPtsDueToInstructor(struct SOLDIERTYPE *pInstructor,
                                           struct SOLDIERTYPE *pStudent, int8_t bTrainStat,
                                           BOOLEAN fAtGunRange, uint16_t *pusMaxPts) {
  // return the bonus training pts of this instructor with this student,...if student null, simply
  // assignment student skill of 0 and student wisdom of 100
  uint16_t sTrainingPts = 0;
  int8_t bTraineeEffWisdom = 0;
  int8_t bTraineeNatWisdom = 0;
  int8_t bTraineeSkill = 0;
  int8_t bTrainerEffSkill = 0;
  int8_t bTrainerNatSkill = 0;
  int8_t bTrainingBonus = 0;
  int8_t bOpinionFactor;

  // assume training impossible for max pts
  *pusMaxPts = 0;

  if (pInstructor == NULL) {
    // no instructor, leave
    return (0);
  }

  switch (bTrainStat) {
    case (STRENGTH):
      bTrainerEffSkill = EffectiveStrength(pInstructor);
      bTrainerNatSkill = pInstructor->bStrength;
      break;
    case (DEXTERITY):
      bTrainerEffSkill = EffectiveDexterity(pInstructor);
      bTrainerNatSkill = pInstructor->bDexterity;
      break;
    case (AGILITY):
      bTrainerEffSkill = EffectiveAgility(pInstructor);
      bTrainerNatSkill = pInstructor->bAgility;
      break;
    case (HEALTH):
      bTrainerEffSkill = pInstructor->bLifeMax;
      bTrainerNatSkill = pInstructor->bLifeMax;
      break;
    case (LEADERSHIP):
      bTrainerEffSkill = EffectiveLeadership(pInstructor);
      bTrainerNatSkill = pInstructor->bLeadership;
      break;
    case (MARKSMANSHIP):
      bTrainerEffSkill = EffectiveMarksmanship(pInstructor);
      bTrainerNatSkill = pInstructor->bMarksmanship;
      break;
    case (EXPLOSIVE_ASSIGN):
      bTrainerEffSkill = EffectiveExplosive(pInstructor);
      bTrainerNatSkill = pInstructor->bExplosive;
      break;
    case (MEDICAL):
      bTrainerEffSkill = EffectiveMedical(pInstructor);
      bTrainerNatSkill = pInstructor->bMedical;
      break;
    case (MECHANICAL):
      bTrainerEffSkill = EffectiveMechanical(pInstructor);
      bTrainerNatSkill = pInstructor->bMechanical;
      break;
    // NOTE: Wisdom can't be trained!
    default:
// BETA message
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_ORANGE, MSG_BETAVERSION,
                L"GetBonusTrainingPtsDueToInstructor: ERROR - Unknown bTrainStat %d", bTrainStat);
#endif
      return (0);
  }

  // if there's no student
  if (pStudent == NULL) {
    // assume these default values
    bTraineeEffWisdom = 100;
    bTraineeNatWisdom = 100;
    bTraineeSkill = 0;
    bOpinionFactor = 0;
  } else {
    // set student's variables
    bTraineeEffWisdom = EffectiveWisdom(pStudent);
    bTraineeNatWisdom = pStudent->bWisdom;

    // for trainee's stat skill, must use the natural value, not the effective one, to avoid drunks
    // training beyond cap
    switch (bTrainStat) {
      case (STRENGTH):
        bTraineeSkill = pStudent->bStrength;
        break;
      case (DEXTERITY):
        bTraineeSkill = pStudent->bDexterity;
        break;
      case (AGILITY):
        bTraineeSkill = pStudent->bAgility;
        break;
      case (HEALTH):
        bTraineeSkill = pStudent->bLifeMax;
        break;
      case (LEADERSHIP):
        bTraineeSkill = pStudent->bLeadership;
        break;
      case (MARKSMANSHIP):
        bTraineeSkill = pStudent->bMarksmanship;
        break;
      case (EXPLOSIVE_ASSIGN):
        bTraineeSkill = pStudent->bExplosive;
        break;
      case (MEDICAL):
        bTraineeSkill = pStudent->bMedical;
        break;
      case (MECHANICAL):
        bTraineeSkill = pStudent->bMechanical;
        break;
      // NOTE: Wisdom can't be trained!
      default:
// BETA message
#ifdef JA2BETAVERSION
        ScreenMsg(FONT_ORANGE, MSG_BETAVERSION,
                  L"GetBonusTrainingPtsDueToInstructor: ERROR - Unknown bTrainStat %d", bTrainStat);
#endif
        return (0);
    }

    // if trainee skill 0 or at/beyond the training cap, can't train
    if ((bTraineeSkill == 0) || (bTraineeSkill >= TRAINING_RATING_CAP)) {
      return 0;
    }

    // factor in their mutual relationship
    bOpinionFactor = gMercProfiles[pStudent->ubProfile].bMercOpinion[pInstructor->ubProfile];
    bOpinionFactor += gMercProfiles[pInstructor->ubProfile].bMercOpinion[pStudent->ubProfile] / 2;
  }

  // check to see if student better than/equal to instructor's effective skill, if so, return 0
  // don't use natural skill - if the guy's too doped up to tell what he know, student learns
  // nothing until sobriety returns!
  if (bTraineeSkill >= bTrainerEffSkill) {
    return (0);
  }

  // calculate effective training pts
  sTrainingPts =
      (bTrainerEffSkill - bTraineeSkill) *
      (bTraineeEffWisdom + (EffectiveWisdom(pInstructor) + EffectiveLeadership(pInstructor)) / 2) /
      INSTRUCTED_TRAINING_DIVISOR;

  // calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs,
  // fatigue)
  *pusMaxPts = (bTrainerNatSkill - bTraineeSkill) *
               (bTraineeNatWisdom + (pInstructor->bWisdom + pInstructor->bLeadership) / 2) /
               INSTRUCTED_TRAINING_DIVISOR;

  // put in a minimum (that can be reduced due to instructor being tired?)
  if (*pusMaxPts == 0) {
    // we know trainer is better than trainee, make sure they are at least 10 pts better
    if (bTrainerEffSkill > bTraineeSkill + 10) {
      sTrainingPts = 1;
      *pusMaxPts = 1;
    }
  }

  // check for teaching skill bonuses
  if (gMercProfiles[pInstructor->ubProfile].bSkillTrait == TEACHING) {
    bTrainingBonus += TEACH_BONUS_TO_TRAIN;
  }
  if (gMercProfiles[pInstructor->ubProfile].bSkillTrait2 == TEACHING) {
    bTrainingBonus += TEACH_BONUS_TO_TRAIN;
  }

  // teaching bonus is counted as normal, but gun range bonus is not
  *pusMaxPts += (((bTrainingBonus + bOpinionFactor) * *pusMaxPts) / 100);

  // get special bonus if we're training marksmanship and we're in the gun range sector in Alma
  if ((bTrainStat == MARKSMANSHIP) && fAtGunRange) {
    bTrainingBonus += GUN_RANGE_TRAINING_BONUS;
  }

  // adjust for any training bonuses and for the relationship
  sTrainingPts += (((bTrainingBonus + bOpinionFactor) * sTrainingPts) / 100);

  // adjust for instructor fatigue
  ReducePointsForFatigue(pInstructor, &sTrainingPts);

  return (sTrainingPts);
}

int16_t GetSoldierTrainingPts(struct SOLDIERTYPE *pSoldier, int8_t bTrainStat, BOOLEAN fAtGunRange,
                              uint16_t *pusMaxPts) {
  uint16_t sTrainingPts = 0;
  int8_t bTrainingBonus = 0;
  int8_t bSkill = 0;

  // assume training impossible for max pts
  *pusMaxPts = 0;

  // use NATURAL not EFFECTIVE values here
  switch (bTrainStat) {
    case (STRENGTH):
      bSkill = pSoldier->bStrength;
      break;
    case (DEXTERITY):
      bSkill = pSoldier->bDexterity;
      break;
    case (AGILITY):
      bSkill = pSoldier->bAgility;
      break;
    case (HEALTH):
      bSkill = pSoldier->bLifeMax;
      break;
    case (LEADERSHIP):
      bSkill = pSoldier->bLeadership;
      break;
    case (MARKSMANSHIP):
      bSkill = pSoldier->bMarksmanship;
      break;
    case (EXPLOSIVE_ASSIGN):
      bSkill = pSoldier->bExplosive;
      break;
    case (MEDICAL):
      bSkill = pSoldier->bMedical;
      break;
    case (MECHANICAL):
      bSkill = pSoldier->bMechanical;
      break;
    // NOTE: Wisdom can't be trained!
    default:
// BETA message
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_ORANGE, MSG_BETAVERSION,
                L"GetSoldierTrainingPts: ERROR - Unknown bTrainStat %d", bTrainStat);
#endif
      return (0);
  }

  // if skill 0 or at/beyond the training cap, can't train
  if ((bSkill == 0) || (bSkill >= TRAINING_RATING_CAP)) {
    return 0;
  }

  // calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs,
  // fatigue)
  *pusMaxPts =
      max(((pSoldier->bWisdom * (TRAINING_RATING_CAP - bSkill)) / SELF_TRAINING_DIVISOR), 1);

  // calculate effective training pts
  sTrainingPts = max(
      ((EffectiveWisdom(pSoldier) * (TRAINING_RATING_CAP - bSkill)) / SELF_TRAINING_DIVISOR), 1);

  // get special bonus if we're training marksmanship and we're in the gun range sector in Alma
  if ((bTrainStat == MARKSMANSHIP) && fAtGunRange) {
    bTrainingBonus += GUN_RANGE_TRAINING_BONUS;
  }

  // adjust for any training bonuses
  sTrainingPts += ((bTrainingBonus * sTrainingPts) / 100);

  // adjust for fatigue
  ReducePointsForFatigue(pSoldier, &sTrainingPts);

  return (sTrainingPts);
}

int16_t GetSoldierStudentPts(struct SOLDIERTYPE *pSoldier, int8_t bTrainStat, BOOLEAN fAtGunRange,
                             uint16_t *pusMaxPts) {
  uint16_t sTrainingPts = 0;
  int8_t bTrainingBonus = 0;
  int8_t bSkill = 0;

  int16_t sBestTrainingPts, sTrainingPtsDueToInstructor;
  uint16_t usMaxTrainerPts, usBestMaxTrainerPts;
  uint32_t uiCnt;
  struct SOLDIERTYPE *pTrainer;

  // assume training impossible for max pts
  *pusMaxPts = 0;

  // use NATURAL not EFFECTIVE values here
  switch (bTrainStat) {
    case (STRENGTH):
      bSkill = pSoldier->bStrength;
      break;
    case (DEXTERITY):
      bSkill = pSoldier->bDexterity;
      break;
    case (AGILITY):
      bSkill = pSoldier->bAgility;
      break;
    case (HEALTH):
      bSkill = pSoldier->bLifeMax;
      break;
    case (LEADERSHIP):
      bSkill = pSoldier->bLeadership;
      break;
    case (MARKSMANSHIP):
      bSkill = pSoldier->bMarksmanship;
      break;
    case (EXPLOSIVE_ASSIGN):
      bSkill = pSoldier->bExplosive;
      break;
    case (MEDICAL):
      bSkill = pSoldier->bMedical;
      break;
    case (MECHANICAL):
      bSkill = pSoldier->bMechanical;
      break;
    // NOTE: Wisdom can't be trained!
    default:
// BETA message
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_ORANGE, MSG_BETAVERSION,
                L"GetSoldierTrainingPts: ERROR - Unknown bTrainStat %d", bTrainStat);
#endif
      return (0);
  }

  // if skill 0 or at/beyond the training cap, can't train
  if ((bSkill == 0) || (bSkill >= TRAINING_RATING_CAP)) {
    return 0;
  }

  // calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs,
  // fatigue)
  *pusMaxPts =
      max(((pSoldier->bWisdom * (TRAINING_RATING_CAP - bSkill)) / SELF_TRAINING_DIVISOR), 1);

  // calculate effective training pts
  sTrainingPts = max(
      ((EffectiveWisdom(pSoldier) * (TRAINING_RATING_CAP - bSkill)) / SELF_TRAINING_DIVISOR), 1);

  // get special bonus if we're training marksmanship and we're in the gun range sector in Alma
  if ((bTrainStat == MARKSMANSHIP) && fAtGunRange) {
    bTrainingBonus += GUN_RANGE_TRAINING_BONUS;
  }

  // adjust for any training bonuses
  sTrainingPts += ((bTrainingBonus * sTrainingPts) / 100);

  // adjust for fatigue
  ReducePointsForFatigue(pSoldier, &sTrainingPts);

  // now add in stuff for trainer

  // for each trainable stat
  sBestTrainingPts = -1;

  // search team for active instructors in this sector
  for (uiCnt = 0, pTrainer = MercPtrs[uiCnt];
       uiCnt <= gTacticalStatus.Team[MercPtrs[0]->bTeam].bLastID; uiCnt++, pTrainer++) {
    if (pTrainer->bActive && (pTrainer->sSectorX == GetSolSectorX(pSoldier)) &&
        (pTrainer->sSectorY == GetSolSectorY(pSoldier)) &&
        (pTrainer->bSectorZ == GetSolSectorZ(pSoldier))) {
      // if he's training teammates in this stat
      // NB skip the EnoughTime requirement to display what the value should be even if haven't been
      // training long yet...
      if ((pTrainer->bAssignment == TRAIN_TEAMMATE) && (pTrainer->bTrainStat == bTrainStat) &&
          (pTrainer->fMercAsleep == FALSE)) {
        sTrainingPtsDueToInstructor = GetBonusTrainingPtsDueToInstructor(
            pTrainer, pSoldier, bTrainStat, fAtGunRange, &usMaxTrainerPts);

        // if he's the best trainer so far for this stat
        if (sTrainingPtsDueToInstructor > sBestTrainingPts) {
          // then remember him as that, and the points he scored
          sBestTrainingPts = sTrainingPtsDueToInstructor;
          usBestMaxTrainerPts = usMaxTrainerPts;
        }
      }
    }
  }

  if (sBestTrainingPts != -1) {
    // add the bonus to what merc can learn on his own
    sTrainingPts += sBestTrainingPts;
    *pusMaxPts += usBestMaxTrainerPts;
  }

  return (sTrainingPts);
}

void TrainSoldierWithPts(struct SOLDIERTYPE *pSoldier, int16_t sTrainPts) {
  uint8_t ubChangeStat = 0;

  if (sTrainPts <= 0) {
    return;
  }

  // which stat to modify?
  switch (pSoldier->bTrainStat) {
    case (STRENGTH):
      ubChangeStat = STRAMT;
      break;
    case (DEXTERITY):
      ubChangeStat = DEXTAMT;
      break;
    case (AGILITY):
      ubChangeStat = AGILAMT;
      break;
    case (HEALTH):
      ubChangeStat = HEALTHAMT;
      break;
    case (LEADERSHIP):
      ubChangeStat = LDRAMT;
      break;
    case (MARKSMANSHIP):
      ubChangeStat = MARKAMT;
      break;
    case (EXPLOSIVE_ASSIGN):
      ubChangeStat = EXPLODEAMT;
      break;
    case (MEDICAL):
      ubChangeStat = MEDICALAMT;
      break;
    case (MECHANICAL):
      ubChangeStat = MECHANAMT;
      break;
    // NOTE: Wisdom can't be trained!
    default:
// BETA message
#ifdef JA2BETAVERSION
      ScreenMsg(FONT_ORANGE, MSG_BETAVERSION, L"TrainSoldierWithPts: ERROR - Unknown bTrainStat %d",
                pSoldier->bTrainStat);
#endif
      return;
  }

  // give this merc a few chances to increase a stat (TRUE means it's training, reverse evolution
  // doesn't apply)
  StatChange(pSoldier, ubChangeStat, sTrainPts, FROM_TRAINING);
}

// will train a town in sector by character
static BOOLEAN TrainTownInSector(struct SOLDIERTYPE *pTrainer, uint8_t sMapX, uint8_t sMapY,
                                 uint16_t sTrainingPts) {
  SECTORINFO *pSectorInfo = &(SectorInfo[GetSectorID8(sMapX, sMapY)]);
  uint8_t ubTownId = 0;
  BOOLEAN fSamSiteInSector = FALSE;

  // find out if a sam site here
  fSamSiteInSector = IsThisSectorASAMSector(sMapX, sMapY, 0);

  // get town index
  ubTownId = StrategicMap[pTrainer->sSectorX + pTrainer->sSectorY * MAP_WORLD_X].townID;
  if (fSamSiteInSector == FALSE) {
    Assert(ubTownId != BLANK_SECTOR);
  }

  // trainer gains leadership - training argument is FALSE, because the trainer is not the one
  // training!
  StatChange(pTrainer, LDRAMT, (uint16_t)(1 + (sTrainingPts / 200)), FALSE);
  //	StatChange( pTrainer, WISDOMAMT, (uint16_t) ( 1 + ( sTrainingPts / 400 ) ), FALSE );

  // increase town's training completed percentage
  pSectorInfo->ubMilitiaTrainingPercentDone += (sTrainingPts / 100);
  pSectorInfo->ubMilitiaTrainingHundredths += (sTrainingPts % 100);

  if (pSectorInfo->ubMilitiaTrainingHundredths >= 100) {
    pSectorInfo->ubMilitiaTrainingPercentDone++;
    pSectorInfo->ubMilitiaTrainingHundredths -= 100;
  }

  // NOTE: Leave this at 100, change TOWN_TRAINING_RATE if necessary.  This value gets reported to
  // player as a %age!
  if (pSectorInfo->ubMilitiaTrainingPercentDone >= 100) {
    // zero out training completion - there's no carryover to the next training session
    pSectorInfo->ubMilitiaTrainingPercentDone = 0;
    pSectorInfo->ubMilitiaTrainingHundredths = 0;

    // make the player pay again next time he wants to train here
    SetMilitiaTrainingPayedForSectorID8(GetSectorID8(sMapX, sMapY), false);

    TownMilitiaTrainingCompleted(pTrainer, sMapX, sMapY);

    // training done
    return (TRUE);
  } else {
    // not done
    return (FALSE);
  }
}

int16_t GetTownTrainPtsForCharacter(struct SOLDIERTYPE *pTrainer, uint16_t *pusMaxPts) {
  uint16_t sTotalTrainingPts = 0;
  int8_t bTrainingBonus = 0;
  //	uint8_t ubTownId = 0;

  // calculate normal training pts - what it would be if his stats were "normal" (ignoring drugs,
  // fatigue)
  *pusMaxPts =
      (pTrainer->bWisdom + pTrainer->bLeadership + (10 * pTrainer->bExpLevel)) * TOWN_TRAINING_RATE;

  // calculate effective training points (this is hundredths of pts / hour)
  // typical: 300/hr, maximum: 600/hr
  sTotalTrainingPts = (EffectiveWisdom(pTrainer) + EffectiveLeadership(pTrainer) +
                       (10 * EffectiveExpLevel(pTrainer))) *
                      TOWN_TRAINING_RATE;

  // check for teaching bonuses
  if (gMercProfiles[pTrainer->ubProfile].bSkillTrait == TEACHING) {
    bTrainingBonus += TEACH_BONUS_TO_TRAIN;
  }
  if (gMercProfiles[pTrainer->ubProfile].bSkillTrait2 == TEACHING) {
    bTrainingBonus += TEACH_BONUS_TO_TRAIN;
  }

  // RPCs get a small training bonus for being more familiar with the locals and their customs/needs
  // than outsiders
  if (pTrainer->ubProfile >= FIRST_RPC) {
    bTrainingBonus += RPC_BONUS_TO_TRAIN;
  }

  // adjust for teaching bonus (a percentage)
  sTotalTrainingPts += ((bTrainingBonus * sTotalTrainingPts) / 100);
  // teach bonus is considered "normal" - it's always there
  *pusMaxPts += ((bTrainingBonus * *pusMaxPts) / 100);

  // adjust for fatigue of trainer
  ReducePointsForFatigue(pTrainer, &sTotalTrainingPts);

  /* ARM: Decided this didn't make much sense - the guys I'm training damn well BETTER be loyal -
     and screw the rest!
          // get town index
          ubTownId = StrategicMap[ pTrainer -> sSectorX + pTrainer -> sSectorY * MAP_WORLD_X
     ].townID; Assert(ubTownId != BLANK_SECTOR);

          // adjust for town loyalty
          sTotalTrainingPts = (sTotalTrainingPts * gTownLoyalty[ ubTownId ].ubRating) / 100;
  */

  return (sTotalTrainingPts);
}

void MakeSoldiersTacticalAnimationReflectAssignment(struct SOLDIERTYPE *pSoldier) {
  // soldier is in tactical, world loaded, he's OKLIFE
  if (IsSolInSector(pSoldier) && gfWorldLoaded && (pSoldier->bLife >= OKLIFE)) {
    // Set animation based on his assignment
    if (GetSolAssignment(pSoldier) == DOCTOR) {
      SoldierInSectorDoctor(pSoldier, pSoldier->usStrategicInsertionData);
    } else if (GetSolAssignment(pSoldier) == PATIENT) {
      SoldierInSectorPatient(pSoldier, pSoldier->usStrategicInsertionData);
    } else if (GetSolAssignment(pSoldier) == REPAIR) {
      SoldierInSectorRepair(pSoldier, pSoldier->usStrategicInsertionData);
    } else {
      if (pSoldier->usAnimState != WKAEUP_FROM_SLEEP && !(pSoldier->bOldAssignment < ON_DUTY)) {
        // default: standing
        ChangeSoldierState(pSoldier, STANDING, 1, TRUE);
      }
    }
  }
}

void AssignmentAborted(struct SOLDIERTYPE *pSoldier, uint8_t ubReason) {
  Assert(ubReason < NUM_ASSIGN_ABORT_REASONS);

  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[ubReason], pSoldier->name);

  StopTimeCompression();

  // update mapscreen
  fCharacterInfoPanelDirty = TRUE;
  fTeamPanelDirty = TRUE;
  fMapScreenBottomDirty = TRUE;
}

void AssignmentDone(struct SOLDIERTYPE *pSoldier, BOOLEAN fSayQuote, BOOLEAN fMeToo) {
  if (IsSolInSector(pSoldier) && (gfWorldLoaded)) {
    if (GetSolAssignment(pSoldier) == DOCTOR) {
      if (IsTacticalMode()) {
        ChangeSoldierState(pSoldier, END_DOCTOR, 1, TRUE);
      } else {
        ChangeSoldierState(pSoldier, STANDING, 1, TRUE);
      }

    } else if (GetSolAssignment(pSoldier) == REPAIR) {
      if (IsTacticalMode()) {
        ChangeSoldierState(pSoldier, END_REPAIRMAN, 1, TRUE);
      } else {
        ChangeSoldierState(pSoldier, STANDING, 1, TRUE);
      }
    } else if (GetSolAssignment(pSoldier) == PATIENT) {
      if (IsTacticalMode()) {
        ChangeSoldierStance(pSoldier, ANIM_CROUCH);
      } else {
        ChangeSoldierState(pSoldier, STANDING, 1, TRUE);
      }
    }
  }

  if (GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL) {
    // hack - reset AbsoluteFinalDestination in case it was left non-nowhere
    pSoldier->sAbsoluteFinalDestination = NOWHERE;
  }

  if (fSayQuote) {
    if ((fMeToo == FALSE) && (GetSolAssignment(pSoldier) == TRAIN_TOWN)) {
      TacticalCharacterDialogue(pSoldier, QUOTE_ASSIGNMENT_COMPLETE);

      if (GetSolAssignment(pSoldier) == TRAIN_TOWN) {
        AddSectorForSoldierToListOfSectorsThatCompletedMilitiaTraining(pSoldier);
      }
    }
  }

  // don't bother telling us again about guys we already know about
  if (!(pSoldier->usQuoteSaidExtFlags & SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT)) {
    pSoldier->usQuoteSaidExtFlags |= SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;

    if (fSayQuote) {
      if (GetSolAssignment(pSoldier) == DOCTOR || GetSolAssignment(pSoldier) == REPAIR ||
          GetSolAssignment(pSoldier) == PATIENT ||
          GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL) {
        TacticalCharacterDialogue(pSoldier, QUOTE_ASSIGNMENT_COMPLETE);
      }
    }

    AddReasonToWaitingListQueue(ASSIGNMENT_FINISHED_FOR_UPDATE);
    AddSoldierToWaitingListQueue(pSoldier);

    // trigger a single call AddDisplayBoxToWaitingQueue for assignments done
    gfAddDisplayBoxToWaitingQueue = TRUE;
  }

  // update mapscreen
  fCharacterInfoPanelDirty = TRUE;
  fTeamPanelDirty = TRUE;
  fMapScreenBottomDirty = TRUE;
}

BOOLEAN CharacterIsBetweenSectors(struct SOLDIERTYPE *pSoldier) {
  // is the character on the move
  if (pSoldier == NULL) {
    return (FALSE);
  } else {
    return (pSoldier->fBetweenSectors);
  }
}

void HandleNaturalHealing(void) {
  struct SOLDIERTYPE *pSoldier, *pTeamSoldier;
  int32_t cnt = 0;

  // set psoldier as first in merc ptrs
  pSoldier = MercPtrs[0];

  // go through list of characters, find all who are on this assignment
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      // mechanical members don't regenerate!
      if (!(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !(AM_A_ROBOT(pTeamSoldier))) {
        HandleHealingByNaturalCauses(pTeamSoldier);
      }
    }
  }

  return;
}

// handle healing of this soldier by natural causes.
void HandleHealingByNaturalCauses(struct SOLDIERTYPE *pSoldier) {
  uint32_t uiPercentHealth = 0;
  int8_t bActivityLevelDivisor = 0;

  // check if soldier valid
  if (pSoldier == NULL) {
    return;
  }

  // dead
  if (pSoldier->bLife == 0) {
    return;
  }

  // lost any pts?
  if (pSoldier->bLife == pSoldier->bLifeMax) {
    return;
  }

  // any bleeding pts - can' recover if still bleeding!
  if (pSoldier->bBleeding != 0) {
    return;
  }

  // not bleeding and injured...

  if (GetSolAssignment(pSoldier) == ASSIGNMENT_POW) {
    // use high activity level to simulate stress, torture, poor conditions for healing
    bActivityLevelDivisor = HIGH_ACTIVITY_LEVEL;
  }
  if ((pSoldier->fMercAsleep == TRUE) || (GetSolAssignment(pSoldier) == PATIENT) ||
      (GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL)) {
    bActivityLevelDivisor = LOW_ACTIVITY_LEVEL;
  } else if (pSoldier->bAssignment < ON_DUTY) {
    // if time is being compressed, and the soldier is not moving strategically
    if (IsTimeBeingCompressed() && !PlayerIDGroupInMotion(pSoldier->ubGroupID)) {
      // basically resting
      bActivityLevelDivisor = LOW_ACTIVITY_LEVEL;
    } else {
      // either they're on the move, or they're being tactically active
      bActivityLevelDivisor = HIGH_ACTIVITY_LEVEL;
    }
  } else  // this includes being in a vehicle - that's neither very strenous, nor very restful
  {
    bActivityLevelDivisor = MEDIUM_ACTIVITY_LEVEL;
  }

  // what percentage of health is he down to
  uiPercentHealth = (pSoldier->bLife * 100) / pSoldier->bLifeMax;

  // gain that many hundredths of life points back, divided by the activity level modifier
  pSoldier->sFractLife += (int16_t)(uiPercentHealth / bActivityLevelDivisor);

  // now update the real life values
  UpDateSoldierLife(pSoldier);

  return;
}

void UpDateSoldierLife(struct SOLDIERTYPE *pSoldier) {
  // update soldier life, make sure we don't go out of bounds
  pSoldier->bLife += pSoldier->sFractLife / 100;

  // keep remaining fract of life
  pSoldier->sFractLife %= 100;

  // check if we have gone too far
  if (pSoldier->bLife >= pSoldier->bLifeMax) {
    // reduce
    pSoldier->bLife = pSoldier->bLifeMax;
    pSoldier->sFractLife = 0;
  }
  return;
}

void CheckIfSoldierUnassigned(struct SOLDIERTYPE *pSoldier) {
  if (GetSolAssignment(pSoldier) == NO_ASSIGNMENT) {
    // unassigned
    AddCharacterToAnySquad(pSoldier);

    if ((gfWorldLoaded) && IsSolInSector(pSoldier)) {
      ChangeSoldierState(pSoldier, STANDING, 1, TRUE);
    }
  }

  return;
}

void CreateDestroyMouseRegionsForAssignmentMenu(void) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;
  static BOOLEAN fShowRemoveMenu = FALSE;

  // will create/destroy mouse regions for the map screen assignment main menu
  // check if we can only remove character from team..not assign
  if ((bSelectedAssignChar != -1) || (fShowRemoveMenu == TRUE)) {
    if (fShowRemoveMenu == TRUE) {
      // dead guy handle menu stuff
      fShowRemoveMenu = fShowAssignmentMenu | fShowContractMenu;

      CreateDestroyMouseRegionsForRemoveMenu();

      return;
    }
    if ((Menptr[gCharactersList[bSelectedAssignChar].usSolID].bLife == 0) ||
        (Menptr[gCharactersList[bSelectedAssignChar].usSolID].bAssignment == ASSIGNMENT_POW)) {
      // dead guy handle menu stuff
      fShowRemoveMenu = fShowAssignmentMenu | fShowContractMenu;

      CreateDestroyMouseRegionsForRemoveMenu();

      return;
    }
  }

  if ((fShowAssignmentMenu == TRUE) && (fCreated == FALSE)) {
    gfIgnoreScrolling = FALSE;

    if ((fShowAssignmentMenu) && (IsMapScreen_2())) {
      SetBoxPosition(ghAssignmentBox, AssignmentPosition);
    }

    pSoldier = GetSelectedAssignSoldier(FALSE);

    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      // grab height of font
      iFontHeight = GetLineSpace(ghEpcBox) + GetFontHeight(GetBoxFont(ghEpcBox));

      // get x.y position of box
      GetBoxPosition(ghEpcBox, &pPosition);

      // grab box x and y position
      iBoxXPosition = pPosition.iX;
      iBoxYPosition = pPosition.iY;

      // get dimensions..mostly for width
      GetBoxSize(ghEpcBox, &pDimensions);

      // get width
      iBoxWidth = pDimensions.iRight;

      SetCurrentBox(ghEpcBox);
    } else {
      // grab height of font
      iFontHeight = GetLineSpace(ghAssignmentBox) + GetFontHeight(GetBoxFont(ghAssignmentBox));

      // get x.y position of box
      GetBoxPosition(ghAssignmentBox, &pPosition);

      // grab box x and y position
      iBoxXPosition = pPosition.iX;
      iBoxYPosition = pPosition.iY;

      // get dimensions..mostly for width
      GetBoxSize(ghAssignmentBox, &pDimensions);

      // get width
      iBoxWidth = pDimensions.iRight;

      SetCurrentBox(ghAssignmentBox);
    }

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghAssignmentBox); iCounter++) {
      // add mouse region for each line of text..and set user data
      MSYS_DefineRegion(
          &gAssignmentMenuRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                    (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, AssignmentMenuMvtCallBack,
          AssignmentMenuBtnCallback);

      MSYS_SetRegionUserData(&gAssignmentMenuRegion[iCounter], 0, iCounter);
    }

    // created
    fCreated = TRUE;

    // unhighlight all strings in box
    UnHighLightBox(ghAssignmentBox);
    CheckAndUpdateTacticalAssignmentPopUpPositions();

    PositionCursorForTacticalAssignmentBox();
  } else if ((fShowAssignmentMenu == FALSE) && (fCreated == TRUE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghAssignmentBox); iCounter++) {
      MSYS_RemoveRegion(&gAssignmentMenuRegion[iCounter]);
    }

    fShownAssignmentMenu = FALSE;

    // not created
    fCreated = FALSE;
    SetRenderFlags(RENDER_FLAG_FULL);
  }
}

void CreateDestroyMouseRegionForVehicleMenu(void) {
  static BOOLEAN fCreated = FALSE;

  uint32_t uiMenuLine = 0;
  int32_t iVehicleId = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition, pPoint;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;
  struct SOLDIERTYPE *pSoldier = NULL;

  if (fShowVehicleMenu) {
    GetBoxPosition(ghAssignmentBox, &pPoint);

    // get dimensions..mostly for width
    GetBoxSize(ghAssignmentBox, &pDimensions);

    // vehicle position
    VehiclePosition.iX = pPoint.iX + pDimensions.iRight;

    SetBoxPosition(ghVehicleBox, VehiclePosition);
  }

  if ((fShowVehicleMenu == TRUE) && (fCreated == FALSE)) {
    // grab height of font
    iFontHeight = GetLineSpace(ghVehicleBox) + GetFontHeight(GetBoxFont(ghVehicleBox));

    // get x.y position of box
    GetBoxPosition(ghVehicleBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghVehicleBox, &pDimensions);
    SetBoxSecondaryShade(ghVehicleBox, FONT_YELLOW);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghVehicleBox);

    pSoldier = GetSelectedAssignSoldier(FALSE);

    // run through list of vehicles in sector
    for (iVehicleId = 0; iVehicleId < ubNumberOfVehicles; iVehicleId++) {
      if (pVehicleList[iVehicleId].fValid == TRUE) {
        if (IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleId)) {
          // add mouse region for each accessible vehicle
          MSYS_DefineRegion(&gVehicleMenuRegion[uiMenuLine], (int16_t)(iBoxXPosition),
                            (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                                      (iFontHeight)*uiMenuLine),
                            (int16_t)(iBoxXPosition + iBoxWidth),
                            (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                                      (iFontHeight) * (uiMenuLine + 1)),
                            MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, VehicleMenuMvtCallback,
                            VehicleMenuBtnCallback);

          MSYS_SetRegionUserData(&gVehicleMenuRegion[uiMenuLine], 0, uiMenuLine);
          // store vehicle ID in the SECOND user data
          MSYS_SetRegionUserData(&gVehicleMenuRegion[uiMenuLine], 1, iVehicleId);

          uiMenuLine++;
        }
      }
    }

    // cancel line
    MSYS_DefineRegion(
        &gVehicleMenuRegion[uiMenuLine], (int16_t)(iBoxXPosition),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight)*uiMenuLine),
        (int16_t)(iBoxXPosition + iBoxWidth),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                  (iFontHeight) * (uiMenuLine + 1)),
        MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, VehicleMenuMvtCallback, VehicleMenuBtnCallback);
    MSYS_SetRegionUserData(&gVehicleMenuRegion[uiMenuLine], 0, VEHICLE_MENU_CANCEL);

    // created
    fCreated = TRUE;

    // pause game
    PauseGame();

    // unhighlight all strings in box
    UnHighLightBox(ghVehicleBox);

    fCreated = TRUE;

    HandleShadingOfLinesForVehicleMenu();
  } else if (((fShowVehicleMenu == FALSE) || (fShowAssignmentMenu == FALSE)) &&
             (fCreated == TRUE)) {
    fCreated = FALSE;

    // remove these regions
    for (uiMenuLine = 0; uiMenuLine < GetNumberOfLinesOfTextInBox(ghVehicleBox); uiMenuLine++) {
      MSYS_RemoveRegion(&gVehicleMenuRegion[uiMenuLine]);
    }

    fShowVehicleMenu = FALSE;

    SetRenderFlags(RENDER_FLAG_FULL);

    HideBox(ghVehicleBox);

    if (fShowAssignmentMenu) {
      // remove highlight on the parent menu
      UnHighLightBox(ghAssignmentBox);
    }
  }

  return;
}

void HandleShadingOfLinesForVehicleMenu(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iVehicleId;
  uint32_t uiMenuLine = 0;

  if ((fShowVehicleMenu == FALSE) || (ghVehicleBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(FALSE);

  // run through list of vehicles
  for (iVehicleId = 0; iVehicleId < ubNumberOfVehicles; iVehicleId++) {
    if (pVehicleList[iVehicleId].fValid == TRUE) {
      // inaccessible vehicles aren't listed at all!
      if (IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleId)) {
        if (IsEnoughSpaceInVehicle(iVehicleId)) {
          // legal vehicle, leave it green
          UnShadeStringInBox(ghVehicleBox, uiMenuLine);
          UnSecondaryShadeStringInBox(ghVehicleBox, uiMenuLine);
        } else {
          // unjoinable vehicle - yellow
          SecondaryShadeStringInBox(ghVehicleBox, uiMenuLine);
        }

        uiMenuLine++;
      }
    }
  }
}

void VehicleMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment region
  int32_t iValue = -1, iVehicleID;
  struct SOLDIERTYPE *pSoldier;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (iValue == VEHICLE_MENU_CANCEL) {
      fShowVehicleMenu = FALSE;
      UnHighLightBox(ghAssignmentBox);
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;
      fCharacterInfoPanelDirty = TRUE;
      return;
    }

    pSoldier = GetSelectedAssignSoldier(FALSE);
    iVehicleID = MSYS_GetRegionUserData(pRegion, 1);

    // inaccessible vehicles shouldn't be listed in the menu!
    Assert(IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleID));

    if (IsEnoughSpaceInVehicle(iVehicleID)) {
      PutSoldierInVehicle(pSoldier, (int8_t)iVehicleID);
    } else {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[18],
                zVehicleName[pVehicleList[iVehicleID].ubVehicleType]);
    }

    fShowAssignmentMenu = FALSE;

    // update mapscreen
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;

    giAssignHighLine = -1;

    SetAssignmentForList(VEHICLE, (int8_t)iVehicleID);
  }
}

void VehicleMenuMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (iValue != VEHICLE_MENU_CANCEL) {
      // no shaded(disabled) lines actually appear in vehicle menus
      if (GetBoxShadeFlag(ghVehicleBox, iValue) == FALSE) {
        // highlight vehicle line
        HighLightBoxLine(ghVehicleBox, iValue);
      }
    } else {
      // highlight cancel line
      HighLightBoxLine(ghVehicleBox, GetNumberOfLinesOfTextInBox(ghVehicleBox) - 1);
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghVehicleBox);

    HandleShadingOfLinesForVehicleMenu();
  }
}

BOOLEAN DisplayRepairMenu(struct SOLDIERTYPE *pSoldier) {
  int32_t iVehicleIndex = 0;
  uint32_t hStringHandle = 0;

  // run through list of vehicles in sector and add them to pop up box
  // first, clear pop up box
  RemoveBox(ghRepairBox);
  ghRepairBox = -1;

  CreateRepairBox();
  SetCurrentBox(ghRepairBox);

  // PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must
  // remain in synch: CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and
  // HandleShadingOfLinesForRepairMenu().

  if (GetSolSectorZ(pSoldier) == 0) {
    // run through list of vehicles and see if any in sector
    for (iVehicleIndex = 0; iVehicleIndex < ubNumberOfVehicles; iVehicleIndex++) {
      if (pVehicleList[iVehicleIndex].fValid == TRUE) {
        // don't even list the helicopter, because it's NEVER repairable...
        if (iVehicleIndex != iHelicopterVehicleId) {
          if (IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleIndex)) {
            AddMonoString(&hStringHandle,
                          pVehicleStrings[pVehicleList[iVehicleIndex].ubVehicleType]);
          }
        }
      }
    }
  }

  /* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
          // is there a SAM SITE Here?
          if( ( IsThisSectorASAMSector( GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
     pSoldier->bSectorZ )
     == TRUE ) && ( IsTheSAMSiteInSectorRepairable( GetSolSectorX(pSoldier),
     GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier) ) ) )
          {
                  // SAM site
                  AddMonoString(&hStringHandle, pRepairStrings[ 1 ] );
          }
  */

  // is the ROBOT here?
  if (IsRobotInThisSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                          GetSolSectorZ(pSoldier))) {
    // robot
    AddMonoString(&hStringHandle, pRepairStrings[3]);
  }

  // items
  AddMonoString(&hStringHandle, pRepairStrings[0]);

  // cancel
  AddMonoString(&hStringHandle, pRepairStrings[2]);

  SetBoxFont(ghRepairBox, MAP_SCREEN_FONT);
  SetBoxHighLight(ghRepairBox, FONT_WHITE);
  SetBoxShade(ghRepairBox, FONT_GRAY7);
  SetBoxForeground(ghRepairBox, FONT_LTGREEN);
  SetBoxBackground(ghRepairBox, FONT_BLACK);

  // resize box to text
  ResizeBoxToText(ghRepairBox);

  CheckAndUpdateTacticalAssignmentPopUpPositions();

  return TRUE;
}

void HandleShadingOfLinesForRepairMenu(void) {
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iVehicleIndex = 0;
  int32_t iCount = 0;

  if ((fShowRepairMenu == FALSE) || (ghRepairBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(FALSE);

  // PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must
  // remain in synch: CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and
  // HandleShadingOfLinesForRepairMenu().

  if (GetSolSectorZ(pSoldier) == 0) {
    for (iVehicleIndex = 0; iVehicleIndex < ubNumberOfVehicles; iVehicleIndex++) {
      if (pVehicleList[iVehicleIndex].fValid == TRUE) {
        // don't even list the helicopter, because it's NEVER repairable...
        if (iVehicleIndex != iHelicopterVehicleId) {
          if (IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleIndex)) {
            if (CanCharacterRepairVehicle(pSoldier, iVehicleIndex) == TRUE) {
              // unshade vehicle line
              UnShadeStringInBox(ghRepairBox, iCount);
            } else {
              // shade vehicle line
              ShadeStringInBox(ghRepairBox, iCount);
            }

            iCount++;
          }
        }
      }
    }
  }

  /* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
          if( ( IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier ->
     bSectorZ ) == TRUE ) && ( IsTheSAMSiteInSectorRepairable( pSoldier -> sSectorX, pSoldier ->
     sSectorY, pSoldier -> bSectorZ ) ) )
          {
                  // handle enable disable of repair sam option
                  if( CanSoldierRepairSAM( pSoldier, SAM_SITE_REPAIR_DIVISOR ) )
                  {
                          // unshade SAM line
                          UnShadeStringInBox( ghRepairBox, iCount );
                  }
                  else
                  {
                          // shade SAM line
                          ShadeStringInBox( ghRepairBox, iCount );
                  }

                  iCount++;
          }
  */

  if (IsRobotInThisSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                          GetSolSectorZ(pSoldier))) {
    // handle shading of repair robot option
    if (CanCharacterRepairRobot(pSoldier)) {
      // unshade robot line
      UnShadeStringInBox(ghRepairBox, iCount);
    } else {
      // shade robot line
      ShadeStringInBox(ghRepairBox, iCount);
    }

    iCount++;
  }

  if (DoesCharacterHaveAnyItemsToRepair(pSoldier, FINAL_REPAIR_PASS)) {
    // unshade items line
    UnShadeStringInBox(ghRepairBox, iCount);
  } else {
    // shade items line
    ShadeStringInBox(ghRepairBox, iCount);
  }

  iCount++;

  return;
}

void CreateDestroyMouseRegionForRepairMenu(void) {
  static BOOLEAN fCreated = FALSE;

  uint32_t uiCounter = 0;
  int32_t iCount = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iVehicleIndex = 0;

  if ((fShowRepairMenu == TRUE) && (fCreated == FALSE)) {
    CheckAndUpdateTacticalAssignmentPopUpPositions();

    if ((fShowRepairMenu) && (IsMapScreen_2())) {
      // SetBoxPosition( ghRepairBox ,RepairPosition);
    }

    // grab height of font
    iFontHeight = GetLineSpace(ghRepairBox) + GetFontHeight(GetBoxFont(ghRepairBox));

    // get x.y position of box
    GetBoxPosition(ghRepairBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghRepairBox, &pDimensions);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghRepairBox);

    pSoldier = GetSelectedAssignSoldier(FALSE);

    // PLEASE NOTE: make sure any changes you do here are reflected in all 3 routines which must
    // remain in synch: CreateDestroyMouseRegionForRepairMenu(), DisplayRepairMenu(), and
    // HandleShadingOfLinesForRepairMenu().

    if (GetSolSectorZ(pSoldier) == 0) {
      // vehicles
      for (iVehicleIndex = 0; iVehicleIndex < ubNumberOfVehicles; iVehicleIndex++) {
        if (pVehicleList[iVehicleIndex].fValid == TRUE) {
          // don't even list the helicopter, because it's NEVER repairable...
          if (iVehicleIndex != iHelicopterVehicleId) {
            // other vehicles *in the sector* are listed, but later shaded dark if they're not
            // repairable
            if (IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleIndex)) {
              // add mouse region for each line of text..and set user data
              MSYS_DefineRegion(&gRepairMenuRegion[iCount], (int16_t)(iBoxXPosition),
                                (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                                          (iFontHeight)*iCount),
                                (int16_t)(iBoxXPosition + iBoxWidth),
                                (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                                          (iFontHeight) * (iCount + 1)),
                                MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback,
                                RepairMenuBtnCallback);

              MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 0, iCount);
              // 2nd user data is the vehicle index, which can easily be different from the region
              // index!
              MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 1, iVehicleIndex);
              iCount++;
            }
          }
        }
      }
    }

    /* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
                    // SAM site
                    if( ( IsThisSectorASAMSector( pSoldier -> sSectorX, pSoldier -> sSectorY,
       pSoldier -> bSectorZ ) == TRUE ) && ( IsTheSAMSiteInSectorRepairable( pSoldier -> sSectorX,
       pSoldier -> sSectorY, pSoldier -> bSectorZ ) ) )
                    {
                            MSYS_DefineRegion( &gRepairMenuRegion[ iCount ], 	( int16_t )(
       iBoxXPosition ), ( int16_t )( iBoxYPosition + GetTopMarginSize( ghAssignmentBox ) + (
       iFontHeight ) * iCount ), ( int16_t )( iBoxXPosition + iBoxWidth ), ( int16_t )(
       iBoxYPosition + GetTopMarginSize( ghAssignmentBox ) + ( iFontHeight ) * ( iCount + 1 ) ),
       MSYS_PRIORITY_HIGHEST - 4 , MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback );

                            MSYS_SetRegionUserData( &gRepairMenuRegion[ iCount ], 0,
       REPAIR_MENU_SAM_SITE ); iCount++;
                    }
    */

    // robot
    if (IsRobotInThisSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                            GetSolSectorZ(pSoldier))) {
      MSYS_DefineRegion(
          &gRepairMenuRegion[iCount], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight)*iCount),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                    (iFontHeight) * (iCount + 1)),
          MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

      MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 0, iCount);
      MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 1, REPAIR_MENU_ROBOT);
      iCount++;
    }

    // items
    MSYS_DefineRegion(
        &gRepairMenuRegion[iCount], (int16_t)(iBoxXPosition),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight)*iCount),
        (int16_t)(iBoxXPosition + iBoxWidth),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight) * (iCount + 1)),
        MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

    MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 0, iCount);
    MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 1, REPAIR_MENU_ITEMS);
    iCount++;

    // cancel
    MSYS_DefineRegion(
        &gRepairMenuRegion[iCount], (int16_t)(iBoxXPosition),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight)*iCount),
        (int16_t)(iBoxXPosition + iBoxWidth),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight) * (iCount + 1)),
        MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, RepairMenuMvtCallback, RepairMenuBtnCallback);

    MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 0, iCount);
    MSYS_SetRegionUserData(&gRepairMenuRegion[iCount], 1, REPAIR_MENU_CANCEL);

    PauseGame();

    // unhighlight all strings in box
    UnHighLightBox(ghRepairBox);

    fCreated = TRUE;
  } else if (((fShowRepairMenu == FALSE) || (fShowAssignmentMenu == FALSE)) && (fCreated == TRUE)) {
    fCreated = FALSE;

    // remove these regions
    for (uiCounter = 0; uiCounter < GetNumberOfLinesOfTextInBox(ghRepairBox); uiCounter++) {
      MSYS_RemoveRegion(&gRepairMenuRegion[uiCounter]);
    }

    fShowRepairMenu = FALSE;

    SetRenderFlags(RENDER_FLAG_FULL);

    HideBox(ghRepairBox);

    if (fShowAssignmentMenu) {
      // remove highlight on the parent menu
      UnHighLightBox(ghAssignmentBox);
    }
  }

  return;
}

void RepairMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment region
  int32_t iValue = -1;
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iRepairWhat;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  // ignore clicks on disabled lines
  if (GetBoxShadeFlag(ghRepairBox, iValue) == TRUE) {
    return;
  }

  // WHAT is being repaired is stored in the second user data argument
  iRepairWhat = MSYS_GetRegionUserData(pRegion, 1);

  pSoldier = GetSelectedAssignSoldier(FALSE);

  if (pSoldier && IsSolActive(pSoldier) && (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)) {
    if ((iRepairWhat >= REPAIR_MENU_VEHICLE1) && (iRepairWhat <= REPAIR_MENU_VEHICLE3)) {
      // repair VEHICLE

      pSoldier->bOldAssignment = pSoldier->bAssignment;

      if ((pSoldier->bAssignment != REPAIR) || (pSoldier->fFixingRobot) ||
          (pSoldier->fFixingSAMSite)) {
        SetTimeOfAssignmentChangeForMerc(pSoldier);
      }

      if (pSoldier->bOldAssignment == VEHICLE) {
        TakeSoldierOutOfVehicle(pSoldier);
      }

      // remove from squad
      RemoveCharacterFromSquads(pSoldier);
      MakeSureToolKitIsInHand(pSoldier);

      ChangeSoldiersAssignment(pSoldier, REPAIR);

      pSoldier->bVehicleUnderRepairID = (int8_t)iRepairWhat;

      MakeSureToolKitIsInHand(pSoldier);

      // assign to a movement group
      AssignMercToAMovementGroup(pSoldier);

      // set assignment for group
      SetAssignmentForList((int8_t)REPAIR, 0);
      fShowAssignmentMenu = FALSE;

    }
    /* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
                    else if( iRepairWhat == REPAIR_MENU_SAM_SITE )
                    {
                            // repair SAM site

                            // remove from squad
                            RemoveCharacterFromSquads( pSoldier );
                            MakeSureToolKitIsInHand( pSoldier );

                            if( ( pSoldier->bAssignment != REPAIR )|| ( pSoldier -> fFixingSAMSite
       == FALSE ) )
                            {
                                    SetTimeOfAssignmentChangeForMerc( pSoldier );
                            }

                            ChangeSoldiersAssignment( pSoldier, REPAIR );
                            pSoldier -> fFixingSAMSite = TRUE;

                            // the second argument is irrelevant here, function looks at pSoldier
       itself to know what's being repaired SetAssignmentForList( ( int8_t ) REPAIR, 0 );
                            fShowAssignmentMenu = FALSE;

                            MakeSureToolKitIsInHand( pSoldier );

                            // assign to a movement group
                            AssignMercToAMovementGroup( pSoldier );
                    }
    */
    else if (iRepairWhat == REPAIR_MENU_ROBOT) {
      // repair ROBOT
      pSoldier->bOldAssignment = pSoldier->bAssignment;

      // remove from squad
      if (pSoldier->bOldAssignment == VEHICLE) {
        TakeSoldierOutOfVehicle(pSoldier);
      }

      RemoveCharacterFromSquads(pSoldier);
      MakeSureToolKitIsInHand(pSoldier);

      if ((pSoldier->bAssignment != REPAIR) || (pSoldier->fFixingRobot == FALSE)) {
        SetTimeOfAssignmentChangeForMerc(pSoldier);
      }

      ChangeSoldiersAssignment(pSoldier, REPAIR);
      pSoldier->fFixingRobot = TRUE;

      // the second argument is irrelevant here, function looks at pSoldier itself to know what's
      // being repaired
      SetAssignmentForList((int8_t)REPAIR, 0);
      fShowAssignmentMenu = FALSE;

      MakeSureToolKitIsInHand(pSoldier);

      // assign to a movement group
      AssignMercToAMovementGroup(pSoldier);
    } else if (iRepairWhat == REPAIR_MENU_ITEMS) {
      // items
      SetSoldierAssignment(pSoldier, REPAIR, FALSE, FALSE, -1);

      // the second argument is irrelevant here, function looks at pSoldier itself to know what's
      // being repaired
      SetAssignmentForList((int8_t)REPAIR, 0);
      fShowAssignmentMenu = FALSE;
    } else {
      // CANCEL
      fShowRepairMenu = FALSE;
    }

    // update mapscreen
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;

    giAssignHighLine = -1;
  }
}

void RepairMenuMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    if (iValue < REPAIR_MENU_CANCEL) {
      if (GetBoxShadeFlag(ghRepairBox, iValue) == FALSE) {
        // highlight choice
        HighLightBoxLine(ghRepairBox, iValue);
      }
    } else {
      // highlight cancel line
      HighLightBoxLine(ghRepairBox, GetNumberOfLinesOfTextInBox(ghRepairBox) - 1);
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghRepairBox);
  }
}

void MakeSureToolKitIsInHand(struct SOLDIERTYPE *pSoldier) {
  int8_t bPocket = 0;

  // if there isn't a toolkit in his hand
  if (pSoldier->inv[HANDPOS].usItem != TOOLKIT) {
    // run through rest of inventory looking for toolkits, swap the first one into hand if found
    for (bPocket = SECONDHANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
      if (pSoldier->inv[bPocket].usItem == TOOLKIT) {
        SwapObjs(&pSoldier->inv[HANDPOS], &pSoldier->inv[bPocket]);
        break;
      }
    }
  }
}

BOOLEAN MakeSureMedKitIsInHand(struct SOLDIERTYPE *pSoldier) {
  int8_t bPocket = 0;

  fTeamPanelDirty = TRUE;

  // if there is a MEDICAL BAG in his hand, we're set
  if (pSoldier->inv[HANDPOS].usItem == MEDICKIT) {
    return (TRUE);
  }

  // run through rest of inventory looking 1st for MEDICAL BAGS, swap the first one into hand if
  // found
  for (bPocket = SECONDHANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
    if (pSoldier->inv[bPocket].usItem == MEDICKIT) {
      fCharacterInfoPanelDirty = TRUE;
      SwapObjs(&pSoldier->inv[HANDPOS], &pSoldier->inv[bPocket]);
      return (TRUE);
    }
  }

  // we didn't find a medical bag, so settle for a FIRST AID KIT
  if (pSoldier->inv[HANDPOS].usItem == FIRSTAIDKIT) {
    return (TRUE);
  }

  // run through rest of inventory looking 1st for MEDICAL BAGS, swap the first one into hand if
  // found
  for (bPocket = SECONDHANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
    if (pSoldier->inv[bPocket].usItem == FIRSTAIDKIT) {
      if ((Item[pSoldier->inv[HANDPOS].usItem].fFlags & IF_TWOHANDED_GUN) &&
          (bPocket >= SMALLPOCK1POS)) {
        // first move from hand to second hand
        SwapObjs(&pSoldier->inv[HANDPOS], &pSoldier->inv[SECONDHANDPOS]);

        // dirty mapscreen and squad panels
        fCharacterInfoPanelDirty = TRUE;
        fInterfacePanelDirty = DIRTYLEVEL2;
      }

      SwapObjs(&pSoldier->inv[HANDPOS], &pSoldier->inv[bPocket]);

      return (TRUE);
    }
  }

  // no medkit items in possession!
  return (FALSE);
}

void HandleShadingOfLinesForAssignmentMenus(void) {
  struct SOLDIERTYPE *pSoldier = NULL;

  // updates which menus are selectable based on character status

  if ((fShowAssignmentMenu == FALSE) || (ghAssignmentBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(FALSE);

  if (pSoldier && IsSolActive(pSoldier)) {
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      // patient
      if (CanCharacterPatient(pSoldier)) {
        // unshade patient line
        UnShadeStringInBox(ghEpcBox, EPC_MENU_PATIENT);
      } else {
        // shade patient line
        ShadeStringInBox(ghEpcBox, EPC_MENU_PATIENT);
      }

      if (CanCharacterOnDuty(pSoldier)) {
        // unshade on duty line
        UnShadeStringInBox(ghEpcBox, EPC_MENU_ON_DUTY);
      } else {
        // shade on duty line
        ShadeStringInBox(ghEpcBox, EPC_MENU_ON_DUTY);
      }

      if (CanCharacterVehicle(pSoldier)) {
        // unshade vehicle line
        UnShadeStringInBox(ghEpcBox, EPC_MENU_VEHICLE);
      } else {
        // shade vehicle line
        ShadeStringInBox(ghEpcBox, EPC_MENU_VEHICLE);
      }
    } else {
      // doctor
      if (CanCharacterDoctor(pSoldier)) {
        // unshade doctor line
        UnShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_DOCTOR);
        UnSecondaryShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_DOCTOR);
      } else {
        // only missing a med kit
        if (CanCharacterDoctorButDoesntHaveMedKit(pSoldier)) {
          SecondaryShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_DOCTOR);
        } else {
          // shade doctor line
          ShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_DOCTOR);
        }
      }

      // repair
      if (CanCharacterRepair(pSoldier)) {
        // unshade repair line
        UnShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_REPAIR);
        UnSecondaryShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_REPAIR);
      } else {
        // only missing a tool kit
        if (CanCharacterRepairButDoesntHaveARepairkit(pSoldier)) {
          SecondaryShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_REPAIR);
        } else {
          // shade repair line
          ShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_REPAIR);
        }
      }

      // patient
      if (CanCharacterPatient(pSoldier)) {
        // unshade patient line
        UnShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_PATIENT);
      } else {
        // shade patient line
        ShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_PATIENT);
      }

      if (CanCharacterOnDuty(pSoldier)) {
        // unshade on duty line
        UnShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_ON_DUTY);
      } else {
        // shade on duty line
        ShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_ON_DUTY);
      }

      if (CanCharacterPractise(pSoldier)) {
        // unshade train line
        UnShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_TRAIN);
      } else {
        // shade train line
        ShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_TRAIN);
      }

      if (CanCharacterVehicle(pSoldier)) {
        // unshade vehicle line
        UnShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_VEHICLE);
      } else {
        // shade vehicle line
        ShadeStringInBox(ghAssignmentBox, ASSIGN_MENU_VEHICLE);
      }
    }
  }

  // squad submenu
  HandleShadingOfLinesForSquadMenu();

  // vehicle submenu
  HandleShadingOfLinesForVehicleMenu();

  // repair submenu
  HandleShadingOfLinesForRepairMenu();

  // training submenu
  HandleShadingOfLinesForTrainingMenu();

  // training attributes submenu
  HandleShadingOfLinesForAttributeMenus();

  return;
}

void DetermineWhichAssignmentMenusCanBeShown(void) {
  BOOLEAN fCharacterNoLongerValid = FALSE;
  struct SOLDIERTYPE *pSoldier = NULL;

  if ((IsMapScreen())) {
    if (fShowMapScreenMovementList == TRUE) {
      if (bSelectedDestChar == -1) {
        fCharacterNoLongerValid = TRUE;
        HandleShowingOfMovementBox();
      } else {
        fShowMapScreenMovementList = FALSE;
        fCharacterNoLongerValid = TRUE;
      }
    }
    /*
                    else if( fShowUpdateBox )
                    {
                            //handle showing of the merc update box
                            HandleShowingOfUpBox( );
                    }
    */
    else if (bSelectedAssignChar == -1) {
      fCharacterNoLongerValid = TRUE;
    }

    // update the assignment positions
    UpdateMapScreenAssignmentPositions();
  }

  // determine which assign menu needs to be shown
  if (((fShowAssignmentMenu == FALSE)) || (fCharacterNoLongerValid == TRUE)) {
    // reset show assignment menus
    fShowAssignmentMenu = FALSE;
    fShowVehicleMenu = FALSE;
    fShowRepairMenu = FALSE;

    // destroy mask, if needed
    CreateDestroyScreenMaskForAssignmentAndContractMenus();

    // destroy menu if needed
    CreateDestroyMouseRegionForVehicleMenu();
    CreateDestroyMouseRegionsForAssignmentMenu();
    CreateDestroyMouseRegionsForTrainingMenu();
    CreateDestroyMouseRegionsForAttributeMenu();
    CreateDestroyMouseRegionsForSquadMenu(TRUE);
    CreateDestroyMouseRegionForRepairMenu();

    // hide all boxes being shown
    if (IsBoxShown(ghEpcBox)) {
      HideBox(ghEpcBox);
      fTeamPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }
    if (IsBoxShown(ghAssignmentBox)) {
      HideBox(ghAssignmentBox);
      fTeamPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }
    if (IsBoxShown(ghTrainingBox)) {
      HideBox(ghTrainingBox);
      fTeamPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }
    if (IsBoxShown(ghRepairBox)) {
      HideBox(ghRepairBox);
      fTeamPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }
    if (IsBoxShown(ghAttributeBox)) {
      HideBox(ghAttributeBox);
      fTeamPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }
    if (IsBoxShown(ghVehicleBox)) {
      HideBox(ghVehicleBox);
      fTeamPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    }

    // do we really want ot hide this box?
    if (fShowContractMenu == FALSE) {
      if (IsBoxShown(ghRemoveMercAssignBox)) {
        HideBox(ghRemoveMercAssignBox);
        fTeamPanelDirty = TRUE;
        gfRenderPBInterface = TRUE;
      }
    }
    // HideBox( ghSquadBox );

    // SetRenderFlags(RENDER_FLAG_FULL);

    // no menus, leave
    return;
  }

  // update the assignment positions
  UpdateMapScreenAssignmentPositions();

  // create mask, if needed
  CreateDestroyScreenMaskForAssignmentAndContractMenus();

  // created assignment menu if needed
  CreateDestroyMouseRegionsForAssignmentMenu();
  CreateDestroyMouseRegionsForTrainingMenu();
  CreateDestroyMouseRegionsForAttributeMenu();
  CreateDestroyMouseRegionsForSquadMenu(TRUE);
  CreateDestroyMouseRegionForRepairMenu();

  if (((Menptr[gCharactersList[bSelectedInfoChar].usSolID].bLife == 0) ||
       (Menptr[gCharactersList[bSelectedInfoChar].usSolID].bAssignment == ASSIGNMENT_POW)) &&
      ((IsMapScreen()))) {
    // show basic assignment menu
    ShowBox(ghRemoveMercAssignBox);
  } else {
    pSoldier = GetSelectedAssignSoldier(FALSE);

    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      ShowBox(ghEpcBox);
    } else {
      // show basic assignment menu

      ShowBox(ghAssignmentBox);
    }
  }

  // TRAINING menu
  if (fShowTrainingMenu == TRUE) {
    HandleShadingOfLinesForTrainingMenu();
    ShowBox(ghTrainingBox);
  } else {
    if (IsBoxShown(ghTrainingBox)) {
      HideBox(ghTrainingBox);
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;
      //	SetRenderFlags(RENDER_FLAG_FULL);
    }
  }

  // REPAIR menu
  if (fShowRepairMenu == TRUE) {
    HandleShadingOfLinesForRepairMenu();
    ShowBox(ghRepairBox);
  } else {
    // hide box
    if (IsBoxShown(ghRepairBox)) {
      HideBox(ghRepairBox);
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;
      //	SetRenderFlags(RENDER_FLAG_FULL);
    }
  }

  // ATTRIBUTE menu
  if (fShowAttributeMenu == TRUE) {
    HandleShadingOfLinesForAttributeMenus();
    ShowBox(ghAttributeBox);
  } else {
    if (IsBoxShown(ghAttributeBox)) {
      HideBox(ghAttributeBox);
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;
      //	SetRenderFlags(RENDER_FLAG_FULL);
    }
  }

  // VEHICLE menu
  if (fShowVehicleMenu == TRUE) {
    ShowBox(ghVehicleBox);
  } else {
    if (IsBoxShown(ghVehicleBox)) {
      HideBox(ghVehicleBox);
      fTeamPanelDirty = TRUE;
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;
      //	SetRenderFlags(RENDER_FLAG_FULL);
    }
  }

  CreateDestroyMouseRegionForVehicleMenu();

  return;
}

void CreateDestroyScreenMaskForAssignmentAndContractMenus(void) {
  static BOOLEAN fCreated = FALSE;
  // will create a screen mask to catch mouse input to disable assignment menus

  // not created, create
  if ((fCreated == FALSE) &&
      ((fShowAssignmentMenu == TRUE) || (fShowContractMenu == TRUE) || (fShowTownInfo == TRUE))) {
    MSYS_DefineRegion(&gAssignmentScreenMaskRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 4,
                      MSYS_NO_CURSOR, MSYS_NO_CALLBACK, AssignmentScreenMaskBtnCallback);

    // created
    fCreated = TRUE;

    if (!(IsMapScreen())) {
      MSYS_ChangeRegionCursor(&gAssignmentScreenMaskRegion, 0);
    }

  } else if ((fCreated == TRUE) && (fShowAssignmentMenu == FALSE) && (fShowContractMenu == FALSE) &&
             (fShowTownInfo == FALSE)) {
    // created, get rid of it
    MSYS_RemoveRegion(&gAssignmentScreenMaskRegion);

    // not created
    fCreated = FALSE;
  }

  return;
}

void AssignmentScreenMaskBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment screen mask region

  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) || (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)) {
    if (fFirstClickInAssignmentScreenMask == TRUE) {
      fFirstClickInAssignmentScreenMask = FALSE;
      return;
    }

    // button event, stop showing menus
    fShowAssignmentMenu = FALSE;

    fShowVehicleMenu = FALSE;

    fShowContractMenu = FALSE;

    // stop showing town mine info
    fShowTownInfo = FALSE;

    // reset contract character and contract highlight line
    giContractHighLine = -1;
    bSelectedContractChar = -1;
    fGlowContractRegion = FALSE;

    // update mapscreen
    fTeamPanelDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;

    gfRenderPBInterface = TRUE;
    SetRenderFlags(RENDER_FLAG_FULL);
  }
  return;
}

void ClearScreenMaskForMapScreenExit(void) {
  // reset show assignment menu
  fShowAssignmentMenu = FALSE;

  // update the assignment positions
  UpdateMapScreenAssignmentPositions();

  // stop showing town mine info too
  fShowTownInfo = FALSE;

  // destroy mask, if needed
  CreateDestroyScreenMaskForAssignmentAndContractMenus();

  // destroy assignment menu if needed
  CreateDestroyMouseRegionsForAssignmentMenu();
  CreateDestroyMouseRegionsForTrainingMenu();
  CreateDestroyMouseRegionsForAttributeMenu();
  CreateDestroyMouseRegionsForSquadMenu(TRUE);
  CreateDestroyMouseRegionForRepairMenu();

  return;
}

void CreateDestroyMouseRegions(void) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;

  // will create/destroy mouse regions for the map screen assignment main menu

  // do we show the remove menu
  if (fShowRemoveMenu) {
    CreateDestroyMouseRegionsForRemoveMenu();
    return;
  }

  if ((fShowAssignmentMenu == TRUE) && (fCreated == FALSE)) {
    // grab height of font
    iFontHeight = GetLineSpace(ghAssignmentBox) + GetFontHeight(GetBoxFont(ghAssignmentBox));

    // get x.y position of box
    GetBoxPosition(ghAssignmentBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghAssignmentBox, &pDimensions);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghAssignmentBox);

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghAssignmentBox); iCounter++) {
      // add mouse region for each line of text..and set user data

      MSYS_DefineRegion(
          &gAssignmentMenuRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAssignmentBox) +
                    (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, AssignmentMenuMvtCallBack,
          AssignmentMenuBtnCallback);

      // set user defines
      MSYS_SetRegionUserData(&gAssignmentMenuRegion[iCounter], 0, iCounter);
    }

    // created
    fCreated = TRUE;

    // pause game
    PauseGame();

    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;

    // unhighlight all strings in box
    UnHighLightBox(ghAssignmentBox);

  } else if ((fShowAssignmentMenu == FALSE) && (fCreated == TRUE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghAssignmentBox); iCounter++) {
      MSYS_RemoveRegion(&gAssignmentMenuRegion[iCounter]);
    }

    RestorePopUpBoxes();

    // not created
    fCreated = FALSE;
  }
}

void CreateDestroyMouseRegionsForContractMenu(void) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;
  static BOOLEAN fShowRemoveMenu = FALSE;

  // will create/destroy mouse regions for the map screen Contract main menu
  // will create/destroy mouse regions for the map screen assignment main menu
  // check if we can only remove character from team..not assign
  if ((bSelectedContractChar != -1) || (fShowRemoveMenu == TRUE)) {
    if (fShowRemoveMenu == TRUE) {
      // dead guy handle menu stuff
      fShowRemoveMenu = fShowContractMenu;

      // ATE: Added this setting of global variable 'cause
      // it will cause an assert failure in GetSelectedAssignSoldier()
      bSelectedAssignChar = bSelectedContractChar;

      CreateDestroyMouseRegionsForRemoveMenu();

      return;
    }
    if (Menptr[gCharactersList[bSelectedContractChar].usSolID].bLife == 0) {
      // dead guy handle menu stuff
      fShowRemoveMenu = fShowContractMenu;

      // ATE: Added this setting of global variable 'cause
      // it will cause an assert failure in GetSelectedAssignSoldier()
      bSelectedAssignChar = bSelectedContractChar;

      CreateDestroyMouseRegionsForRemoveMenu();

      return;
    }
  }

  if ((fShowContractMenu == TRUE) && (fCreated == FALSE)) {
    if (bSelectedContractChar == -1) {
      return;
    }

    if (fShowContractMenu) {
      SetBoxPosition(ghContractBox, ContractPosition);
    }
    // grab height of font
    iFontHeight = GetLineSpace(ghContractBox) + GetFontHeight(GetBoxFont(ghContractBox));

    // get x.y position of box
    GetBoxPosition(ghContractBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghContractBox, &pDimensions);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghContractBox);

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghContractBox); iCounter++) {
      // add mouse region for each line of text..and set user data

      MSYS_DefineRegion(
          &gContractMenuRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghContractBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghContractBox) +
                    (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, ContractMenuMvtCallback,
          ContractMenuBtnCallback);

      // set user defines
      MSYS_SetRegionUserData(&gContractMenuRegion[iCounter], 0, iCounter);
    }

    // created
    fCreated = TRUE;

    // pause game
    PauseGame();

    // unhighlight all strings in box
    UnHighLightBox(ghContractBox);

  } else if ((fShowContractMenu == FALSE) && (fCreated == TRUE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghContractBox); iCounter++) {
      MSYS_RemoveRegion(&gContractMenuRegion[iCounter]);
    }

    fShownContractMenu = FALSE;
    // if( ( fProcessingAMerc ) && ( pProcessingSoldier ) )
    //{
    //	if( (uint32_t)(pProcessingSoldier->iEndofContractTime) == GetWorldTotalMin() )
    //	{
    //		StrategicRemoveMerc( pProcessingSoldier, MERC_FIRED );
    //		pProcessingSoldier = NULL;
    //		fProcessingAMerc = FALSE;
    //	}
    //}

    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;

    RestorePopUpBoxes();

    // not created
    fCreated = FALSE;
  }
}

void CreateDestroyMouseRegionsForTrainingMenu(void) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;

  // will create/destroy mouse regions for the map screen assignment main menu

  if ((fShowTrainingMenu == TRUE) && (fCreated == FALSE)) {
    if ((fShowTrainingMenu) && (IsMapScreen_2())) {
      SetBoxPosition(ghTrainingBox, TrainPosition);
    }

    HandleShadingOfLinesForTrainingMenu();

    CheckAndUpdateTacticalAssignmentPopUpPositions();

    // grab height of font
    iFontHeight = GetLineSpace(ghTrainingBox) + GetFontHeight(GetBoxFont(ghTrainingBox));

    // get x.y position of box
    GetBoxPosition(ghTrainingBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghTrainingBox, &pDimensions);
    SetBoxSecondaryShade(ghTrainingBox, FONT_YELLOW);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghTrainingBox);

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghTrainingBox); iCounter++) {
      // add mouse region for each line of text..and set user data

      MSYS_DefineRegion(
          &gTrainingMenuRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghTrainingBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghTrainingBox) +
                    (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 3, MSYS_NO_CURSOR, TrainingMenuMvtCallBack,
          TrainingMenuBtnCallback);

      // set user defines
      MSYS_SetRegionUserData(&gTrainingMenuRegion[iCounter], 0, iCounter);
    }

    // created
    fCreated = TRUE;

    // unhighlight all strings in box
    UnHighLightBox(ghTrainingBox);

  } else if (((fShowAssignmentMenu == FALSE) || (fShowTrainingMenu == FALSE)) &&
             (fCreated == TRUE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghTrainingBox); iCounter++) {
      MSYS_RemoveRegion(&gTrainingMenuRegion[iCounter]);
    }

    // stop showing training menu
    if (fShowAssignmentMenu == FALSE) {
      fShowTrainingMenu = FALSE;
    }

    RestorePopUpBoxes();

    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
    HideBox(ghTrainingBox);
    SetRenderFlags(RENDER_FLAG_FULL);

    // not created
    fCreated = FALSE;

    if (fShowAssignmentMenu) {
      // remove highlight on the parent menu
      UnHighLightBox(ghAssignmentBox);
    }
  }
}

void CreateDestroyMouseRegionsForAttributeMenu(void) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;

  // will create/destroy mouse regions for the map screen attribute  menu

  if ((fShowAttributeMenu == TRUE) && (fCreated == FALSE)) {
    if ((fShowAssignmentMenu) && (IsMapScreen_2())) {
      SetBoxPosition(ghAssignmentBox, AssignmentPosition);
    }

    HandleShadingOfLinesForAttributeMenus();
    CheckAndUpdateTacticalAssignmentPopUpPositions();

    // grab height of font
    iFontHeight = GetLineSpace(ghAttributeBox) + GetFontHeight(GetBoxFont(ghAttributeBox));

    // get x.y position of box
    GetBoxPosition(ghAttributeBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghAttributeBox, &pDimensions);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghAttributeBox);

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghAttributeBox); iCounter++) {
      // add mouse region for each line of text..and set user data

      MSYS_DefineRegion(
          &gAttributeMenuRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAttributeBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAttributeBox) +
                    (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, AttributeMenuMvtCallBack,
          AttributesMenuBtnCallback);

      // set user defines
      MSYS_SetRegionUserData(&gAttributeMenuRegion[iCounter], 0, iCounter);
    }

    // created
    fCreated = TRUE;

    // unhighlight all strings in box
    UnHighLightBox(ghAttributeBox);

  } else if (((fShowAssignmentMenu == FALSE) || (fShowTrainingMenu == FALSE) ||
              (fShowAttributeMenu == FALSE)) &&
             (fCreated == TRUE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghAttributeBox); iCounter++) {
      MSYS_RemoveRegion(&gAttributeMenuRegion[iCounter]);
    }

    // stop showing training menu
    if ((fShowAssignmentMenu == FALSE) || (fShowTrainingMenu == FALSE) ||
        (fShowAttributeMenu == FALSE)) {
      fShowAttributeMenu = FALSE;
      gfRenderPBInterface = TRUE;
    }

    RestorePopUpBoxes();

    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
    HideBox(ghAttributeBox);
    SetRenderFlags(RENDER_FLAG_FULL);

    // not created
    fCreated = FALSE;

    if (fShowTrainingMenu) {
      // remove highlight on the parent menu
      UnHighLightBox(ghTrainingBox);
    }
  }
}

void CreateDestroyMouseRegionsForRemoveMenu(void) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;

  // will create/destroy mouse regions for the map screen attribute  menu
  if (((fShowAssignmentMenu == TRUE) || (fShowContractMenu == TRUE)) && (fCreated == FALSE)) {
    if (fShowContractMenu) {
      SetBoxPosition(ghContractBox, ContractPosition);
    } else {
      SetBoxPosition(ghAssignmentBox, AssignmentPosition);
    }

    if (fShowContractMenu) {
      // set box position to contract box position
      SetBoxPosition(ghRemoveMercAssignBox, ContractPosition);
    } else {
      // set box position to contract box position
      SetBoxPosition(ghRemoveMercAssignBox, AssignmentPosition);
    }

    CheckAndUpdateTacticalAssignmentPopUpPositions();

    // grab height of font
    iFontHeight =
        GetLineSpace(ghRemoveMercAssignBox) + GetFontHeight(GetBoxFont(ghRemoveMercAssignBox));

    // get x.y position of box
    GetBoxPosition(ghRemoveMercAssignBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghRemoveMercAssignBox, &pDimensions);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghRemoveMercAssignBox);

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghRemoveMercAssignBox); iCounter++) {
      // add mouse region for each line of text..and set user data

      MSYS_DefineRegion(
          &gRemoveMercAssignRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAttributeBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghAttributeBox) +
                    (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, RemoveMercMenuMvtCallBack,
          RemoveMercMenuBtnCallback);

      // set user defines
      MSYS_SetRegionUserData(&gRemoveMercAssignRegion[iCounter], 0, iCounter);
    }

    // created
    fCreated = TRUE;

    // unhighlight all strings in box
    UnHighLightBox(ghRemoveMercAssignBox);

  } else if ((fShowAssignmentMenu == FALSE) && (fCreated == TRUE) && (fShowContractMenu == FALSE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghRemoveMercAssignBox); iCounter++) {
      MSYS_RemoveRegion(&gRemoveMercAssignRegion[iCounter]);
    }

    fShownContractMenu = FALSE;

    // stop showing  menu
    if (fShowRemoveMenu == FALSE) {
      fShowAttributeMenu = FALSE;
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;
    }

    RestorePopUpBoxes();

    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;

    // turn off the GLOBAL fShowRemoveMenu flag!!!
    fShowRemoveMenu = FALSE;
    // and the assignment menu itself!!!
    fShowAssignmentMenu = FALSE;

    // not created
    fCreated = FALSE;
  }
}

void CreateDestroyMouseRegionsForSquadMenu(BOOLEAN fPositionBox) {
  static BOOLEAN fCreated = FALSE;
  uint32_t iCounter = 0;
  int32_t iFontHeight = 0;
  int32_t iBoxXPosition = 0;
  int32_t iBoxYPosition = 0;
  SGPPoint pPosition;
  int32_t iBoxWidth = 0;
  SGPRect pDimensions;

  // will create/destroy mouse regions for the map screen attribute  menu

  if ((fShowSquadMenu == TRUE) && (fCreated == FALSE)) {
    // create squad box
    CreateSquadBox();
    GetBoxSize(ghAssignmentBox, &pDimensions);

    CheckAndUpdateTacticalAssignmentPopUpPositions();

    // grab height of font
    iFontHeight = GetLineSpace(ghSquadBox) + GetFontHeight(GetBoxFont(ghSquadBox));

    // get x.y position of box
    GetBoxPosition(ghSquadBox, &pPosition);

    // grab box x and y position
    iBoxXPosition = pPosition.iX;
    iBoxYPosition = pPosition.iY;

    // get dimensions..mostly for width
    GetBoxSize(ghSquadBox, &pDimensions);

    // get width
    iBoxWidth = pDimensions.iRight;

    SetCurrentBox(ghSquadBox);

    // define regions
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghSquadBox) - 1; iCounter++) {
      // add mouse region for each line of text..and set user data
      MSYS_DefineRegion(
          &gSquadMenuRegion[iCounter], (int16_t)(iBoxXPosition),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghSquadBox) + (iFontHeight)*iCounter),
          (int16_t)(iBoxXPosition + iBoxWidth),
          (int16_t)(iBoxYPosition + GetTopMarginSize(ghSquadBox) + (iFontHeight) * (iCounter + 1)),
          MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, SquadMenuMvtCallBack, SquadMenuBtnCallback);

      MSYS_SetRegionUserData(&gSquadMenuRegion[iCounter], 0, iCounter);
    }

    // now create cancel region
    MSYS_DefineRegion(
        &gSquadMenuRegion[iCounter], (int16_t)(iBoxXPosition),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghSquadBox) + (iFontHeight)*iCounter),
        (int16_t)(iBoxXPosition + iBoxWidth),
        (int16_t)(iBoxYPosition + GetTopMarginSize(ghSquadBox) + (iFontHeight) * (iCounter + 1)),
        MSYS_PRIORITY_HIGHEST - 2, MSYS_NO_CURSOR, SquadMenuMvtCallBack, SquadMenuBtnCallback);

    MSYS_SetRegionUserData(&gSquadMenuRegion[iCounter], 0, SQUAD_MENU_CANCEL);

    // created
    fCreated = TRUE;

    // show the box
    ShowBox(ghSquadBox);

    // unhighlight all strings in box
    UnHighLightBox(ghSquadBox);

    // update based on current squad
    HandleShadingOfLinesForSquadMenu();
  } else if (((fShowAssignmentMenu == FALSE) || (fShowSquadMenu == FALSE)) && (fCreated == TRUE)) {
    // destroy
    for (iCounter = 0; iCounter < GetNumberOfLinesOfTextInBox(ghSquadBox); iCounter++) {
      MSYS_RemoveRegion(&gSquadMenuRegion[iCounter]);
    }

    fShowSquadMenu = FALSE;

    // remove squad box
    RemoveBox(ghSquadBox);
    ghSquadBox = -1;

    RestorePopUpBoxes();

    MarkForRedrawalStrategicMap();
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
    SetRenderFlags(RENDER_FLAG_FULL);

    // not created
    fCreated = FALSE;
    MarkForRedrawalStrategicMap();

    if (fShowAssignmentMenu) {
      // remove highlight on the parent menu
      UnHighLightBox(ghAssignmentBox);
    }
  }
}

void AssignmentMenuMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;
  struct SOLDIERTYPE *pSoldier;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  pSoldier = GetSelectedAssignSoldier(FALSE);

  if (HandleAssignmentExpansionAndHighLightForAssignMenu(pSoldier) == TRUE) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // is the line shaded?..if so, don't highlight
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      if (GetBoxShadeFlag(ghEpcBox, iValue) == FALSE) {
        HighLightBoxLine(ghEpcBox, iValue);
      }
    } else {
      if (GetBoxShadeFlag(ghAssignmentBox, iValue) == FALSE) {
        HighLightBoxLine(ghAssignmentBox, iValue);
      }
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      // unhighlight all strings in box
      UnHighLightBox(ghEpcBox);
    } else {
      // unhighlight all strings in box
      UnHighLightBox(ghAssignmentBox);
    }
  }
}

void RemoveMercMenuMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // highlight string

    // get the string line handle
    // is the line shaded?..if so, don't highlight
    if (GetBoxShadeFlag(ghRemoveMercAssignBox, iValue) == FALSE) {
      HighLightBoxLine(ghRemoveMercAssignBox, iValue);
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghRemoveMercAssignBox);
  }
}

void ContractMenuMvtCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for Contract region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // highlight string

    if (iValue != CONTRACT_MENU_CURRENT_FUNDS) {
      if (GetBoxShadeFlag(ghContractBox, iValue) == FALSE) {
        // get the string line handle
        HighLightBoxLine(ghContractBox, iValue);
      }
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghContractBox);
  }
}

void SquadMenuMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // highlight string

    if (iValue != SQUAD_MENU_CANCEL) {
      if (GetBoxShadeFlag(ghSquadBox, iValue) == FALSE) {
        // get the string line handle
        HighLightBoxLine(ghSquadBox, iValue);
      }
    } else {
      // highlight cancel line
      HighLightBoxLine(ghSquadBox, GetNumberOfLinesOfTextInBox(ghSquadBox) - 1);
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghSquadBox);

    // update based on current squad
    HandleShadingOfLinesForSquadMenu();
  }
}

void RemoveMercMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for contract region
  int32_t iValue = -1;
  struct SOLDIERTYPE *pSoldier = NULL;

  pSoldier = GetSelectedAssignSoldier(FALSE);

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    switch (iValue) {
      case (REMOVE_MERC_CANCEL):

        // stop showing menus
        fShowAssignmentMenu = FALSE;
        fShowContractMenu = FALSE;

        // reset characters
        bSelectedAssignChar = -1;
        bSelectedContractChar = -1;
        giAssignHighLine = -1;

        // dirty regions
        fCharacterInfoPanelDirty = TRUE;
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;

        // stop contratc glow if we are
        fGlowContractRegion = FALSE;
        giContractHighLine = -1;

        break;
      case (REMOVE_MERC):
        StrategicRemoveMerc(pSoldier);

        // dirty region
        fCharacterInfoPanelDirty = TRUE;
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;

        // stop contratc glow if we are
        fGlowContractRegion = FALSE;
        giContractHighLine = -1;

        // reset selected characters
        bSelectedAssignChar = -1;
        bSelectedContractChar = -1;
        giAssignHighLine = -1;

        // stop showing menus
        fShowAssignmentMenu = FALSE;
        fShowContractMenu = FALSE;

        // Def: 10/13/99:  When a merc is either dead or a POW, the Remove Merc popup comes up
        // instead of the
        // Assign menu popup.  When the the user removes the merc, we need to make sure the
        // assignment menu
        // doesnt come up ( because the both assign menu and remove menu flags are needed for the
        // remove pop up to appear dont ask why?!! )
        fShownContractMenu = FALSE;
        fShownAssignmentMenu = FALSE;
        fShowRemoveMenu = FALSE;

        break;
    }
  }
}

void BeginRemoveMercFromContract(struct SOLDIERTYPE *pSoldier) {
  // This function will setup the quote, then start dialogue beginning the actual leave sequence
  if (IsSolAlive(pSoldier) && (pSoldier->bAssignment != ASSIGNMENT_POW)) {
    if ((pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) ||
        (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC)) {
      HandleImportantMercQuote(pSoldier, QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO);

      SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 0, MAP_SCREEN, 0, 0, 0);
      TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING,
                                                1, 0);

    } else
      // quote is different if he's fired in less than 48 hours
      if ((GetWorldTotalMin() - pSoldier->uiTimeOfLastContractUpdate) < 60 * 48) {
        SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 1, MAP_SCREEN, 0, 0,
                                      0);
        if ((pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)) {
          // Only do this if they want to renew.....
          if (WillMercRenew(pSoldier, FALSE)) {
            HandleImportantMercQuote(
                pSoldier, QUOTE_DEPART_COMMET_CONTRACT_NOT_RENEWED_OR_TERMINATED_UNDER_48);
          }
        }

        SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 0, MAP_SCREEN, 0, 0,
                                      0);
        TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0,
                                                  DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 1, 0);

      } else {
        SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 1, MAP_SCREEN, 0, 0,
                                      0);
        if ((pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)) {
          // Only do this if they want to renew.....
          if (WillMercRenew(pSoldier, FALSE)) {
            HandleImportantMercQuote(pSoldier,
                                     QUOTE_DEPARTING_COMMENT_CONTRACT_NOT_RENEWED_OR_48_OR_MORE);
          }
        } else if ((pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC) ||
                   (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC)) {
          HandleImportantMercQuote(pSoldier,
                                   QUOTE_RESPONSE_TO_MIGUEL_SLASH_QUOTE_MERC_OR_RPC_LETGO);
        }

        SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_LOCK_INTERFACE, 0, MAP_SCREEN, 0, 0,
                                      0);
        TacticalCharacterDialogueWithSpecialEvent(pSoldier, 0,
                                                  DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 1, 0);
      }

    if ((GetWorldTotalMin() - pSoldier->uiTimeOfLastContractUpdate) < 60 * 3) {
      // this will cause him give us lame excuses for a while until he gets over it
      // 3-6 days (but the first 1-2 days of that are spent "returning" home)
      gMercProfiles[GetSolProfile(pSoldier)].ubDaysOfMoraleHangover = (uint8_t)(3 + Random(4));

      // if it's an AIM merc, word of this gets back to AIM...  Bad rep.
      if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC) {
        ModifyPlayerReputation(REPUTATION_EARLY_FIRING);
      }
    }
  }
}

void MercDismissConfirmCallBack(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_YES) {
    // Setup history code
    gpDismissSoldier->ubLeaveHistoryCode = HISTORY_MERC_FIRED;

    BeginRemoveMercFromContract(gpDismissSoldier);
  }
}

void ContractMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for contract region
  int32_t iValue = -1;
  BOOLEAN fOkToClose = FALSE;
  struct SOLDIERTYPE *pSoldier = NULL;

  if ((IsMapScreen())) {
    pSoldier = GetSoldierByID(gCharactersList[bSelectedInfoChar].usSolID);
  } else {
    // can't renew contracts from tactical!
  }

  Assert(pSoldier && IsSolActive(pSoldier));

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    fOkToClose = TRUE;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (GetBoxShadeFlag(ghContractBox, iValue) == TRUE) {
      // not valid
      return;
    }

    if (iValue == CONTRACT_MENU_CANCEL) {
      // reset contract character and contract highlight line
      giContractHighLine = -1;
      bSelectedContractChar = -1;
      fGlowContractRegion = FALSE;

      fShowContractMenu = FALSE;
      // dirty region
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;
      fCharacterInfoPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;

      if (gfInContractMenuFromRenewSequence) {
        BeginRemoveMercFromContract(pSoldier);
      }
      return;
    }

    // else handle based on contract

    switch (iValue) {
      case (CONTRACT_MENU_DAY):
        MercContractHandling(pSoldier, CONTRACT_EXTEND_1_DAY);
        PostContractMessage(pSoldier, CONTRACT_EXTEND_1_DAY);
        fOkToClose = TRUE;
        break;
      case (CONTRACT_MENU_WEEK):
        MercContractHandling(pSoldier, CONTRACT_EXTEND_1_WEEK);
        PostContractMessage(pSoldier, CONTRACT_EXTEND_1_WEEK);
        fOkToClose = TRUE;
        break;
      case (CONTRACT_MENU_TWO_WEEKS):
        MercContractHandling(pSoldier, CONTRACT_EXTEND_2_WEEK);
        PostContractMessage(pSoldier, CONTRACT_EXTEND_2_WEEK);
        fOkToClose = TRUE;
        break;

      case (CONTRACT_MENU_TERMINATE):
        gpDismissSoldier = pSoldier;

        // If in the renewal sequence.. do right away...
        // else put up requester.
        if (gfInContractMenuFromRenewSequence) {
          MercDismissConfirmCallBack(MSG_BOX_RETURN_YES);
        } else {
          DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[48], MAP_SCREEN,
                          MSG_BOX_FLAG_YESNO, MercDismissConfirmCallBack);
        }

        fOkToClose = TRUE;

        break;
    }

    pProcessingSoldier = NULL;
    fProcessingAMerc = FALSE;
  }

  if (fOkToClose == TRUE) {
    // reset contract character and contract highlight line
    giContractHighLine = -1;
    bSelectedContractChar = -1;
    fGlowContractRegion = FALSE;
    fShowContractMenu = FALSE;

    // dirty region
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
    gfRenderPBInterface = TRUE;
  }

  return;
}

void TrainingMenuMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (HandleAssignmentExpansionAndHighLightForTrainingMenu() == TRUE) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // highlight string

    // do not highlight current balance
    if (GetBoxShadeFlag(ghTrainingBox, iValue) == FALSE) {
      // get the string line handle
      HighLightBoxLine(ghTrainingBox, iValue);
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghTrainingBox);
  }
}

void AttributeMenuMvtCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // mvt callback handler for assignment region
  int32_t iValue = -1;

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    // highlight string
    if (GetBoxShadeFlag(ghAttributeBox, iValue) == FALSE) {
      // get the string line handle
      HighLightBoxLine(ghAttributeBox, iValue);
    }
  } else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    // unhighlight all strings in box
    UnHighLightBox(ghAttributeBox);
  }
}

void SquadMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment region
  int32_t iValue = -1;
  struct SOLDIERTYPE *pSoldier = NULL;
  wchar_t sString[128];
  int8_t bCanJoinSquad;
  /* ARM: Squad menu is now disabled for anyone between sectors
          uint8_t ubNextX, ubNextY, ubPrevX, ubPrevY;
          uint32_t uiTraverseTime, uiArriveTime;
          int32_t iOldSquadValue = -1;
          BOOLEAN fCharacterWasBetweenSectors = FALSE;
  */

  pSoldier = GetSelectedAssignSoldier(FALSE);

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (iValue == SQUAD_MENU_CANCEL) {
      // stop displaying, leave
      fShowSquadMenu = FALSE;

      // unhighlight the assignment box
      UnHighLightBox(ghAssignmentBox);

      // dirty region
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;
      fCharacterInfoPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;

      return;
    }

    bCanJoinSquad = CanCharacterSquad(pSoldier, (int8_t)iValue);
    // can the character join this squad?  (If already in it, accept that as a legal choice and exit
    // menu)
    if ((bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD) ||
        (bCanJoinSquad == CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT)) {
      if (bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD) {
        // able to add, do it

        /* ARM: Squad menu is now disabled for anyone between sectors
                                        // old squad character was in
                                        iOldSquadValue = SquadCharacterIsIn( pSoldier );

                                        // grab if char was between sectors
                                        fCharacterWasBetweenSectors = pSoldier -> fBetweenSectors;

                                        if( fCharacterWasBetweenSectors )
                                        {
                                                if( pSoldier -> bAssignment == VEHICLE )
                                                {
                                                        if( GetNumberInVehicle( pSoldier ->
           iVehicleId ) == 1 )
                                                        {
                                                                // can't change, go away
                                                                return;
                                                        }
                                                }
                                        }

                                        if( pSoldier -> ubGroupID )
                                        {
                                                GetGroupPosition(&ubNextX, &ubNextY, &ubPrevX,
           &ubPrevY, &uiTraverseTime, &uiArriveTime, pSoldier -> ubGroupID );
                                        }
        */
        pSoldier->bOldAssignment = pSoldier->bAssignment;

        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        AddCharacterToSquad(pSoldier, (int8_t)iValue);

        if (pSoldier->bOldAssignment == VEHICLE) {
          SetSoldierExitVehicleInsertionData(pSoldier, pSoldier->iVehicleId);
        }

        // Clear any desired squad assignments -- seeing the player has physically changed it!
        pSoldier->ubNumTraversalsAllowedToMerge = 0;
        pSoldier->ubDesiredSquadAssignment = NO_ASSIGNMENT;

        /* ARM: Squad menu is now disabled for anyone between sectors
                                        if( fCharacterWasBetweenSectors )
                                        {
                                                // grab location of old squad and set this value for
           new squad if( iOldSquadValue != -1 )
                                                {
                                                        GetSquadPosition( &ubNextX, &ubNextY,
           &ubPrevX, &ubPrevY, &uiTraverseTime, &uiArriveTime,  ( uint8_t )iOldSquadValue );
                                                }

                                                SetGroupPosition( ubNextX, ubNextY, ubPrevX,
           ubPrevY, uiTraverseTime, uiArriveTime, pSoldier -> ubGroupID );
                                        }
        */

        MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);
      }

      // stop displaying, leave
      fShowAssignmentMenu = FALSE;
      giAssignHighLine = -1;

      // dirty region
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;
      fCharacterInfoPanelDirty = TRUE;
      gfRenderPBInterface = TRUE;
    } else {
      BOOLEAN fDisplayError = TRUE;

      switch (bCanJoinSquad) {
        case CHARACTER_CANT_JOIN_SQUAD_SQUAD_MOVING:
          swprintf(sString, ARR_SIZE(sString), pMapErrorString[36], pSoldier->name,
                   pLongAssignmentStrings[iValue]);
          break;
        case CHARACTER_CANT_JOIN_SQUAD_VEHICLE:
          swprintf(sString, ARR_SIZE(sString), pMapErrorString[37], pSoldier->name);
          break;
        case CHARACTER_CANT_JOIN_SQUAD_TOO_FAR:
          swprintf(sString, ARR_SIZE(sString), pMapErrorString[20], pSoldier->name,
                   pLongAssignmentStrings[iValue]);
          break;
        case CHARACTER_CANT_JOIN_SQUAD_FULL:
          swprintf(sString, ARR_SIZE(sString), pMapErrorString[19], pSoldier->name,
                   pLongAssignmentStrings[iValue]);
          break;
        default:
          // generic "you can't join this squad" msg
          swprintf(sString, ARR_SIZE(sString), pMapErrorString[38], pSoldier->name,
                   pLongAssignmentStrings[iValue]);
          break;
      }

      if (fDisplayError) {
        DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
      }
    }

    // set this assignment for the list too
    SetAssignmentForList((int8_t)iValue, 0);
  }

  return;
}

void TrainingMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment region
  int32_t iValue = -1;
  struct SOLDIERTYPE *pSoldier = NULL;
  TownID bTownId;
  wchar_t sString[128];
  wchar_t sStringA[128];

  pSoldier = GetSelectedAssignSoldier(FALSE);

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) ||
      (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)) {
    if ((IsMapScreen()) && !fShowMapInventoryPool) {
      UnMarkButtonDirty(giMapBorderButtons[MAP_BORDER_TOWN_BTN]);
    }
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (fShowAttributeMenu) {
      // cancel attribute submenu
      fShowAttributeMenu = FALSE;
      // rerender tactical stuff
      gfRenderPBInterface = TRUE;

      return;
    }

    switch (iValue) {
      case (TRAIN_MENU_SELF):

        // practise in stat
        gbTrainingMode = TRAIN_SELF;

        // show menu
        fShowAttributeMenu = TRUE;
        DetermineBoxPositions();

        break;
      case (TRAIN_MENU_TOWN):
        if (BasicCanCharacterTrainMilitia(pSoldier)) {
          bTownId = GetTownIdForSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier));

          // if it's a town sector (the following 2 errors can't happen at non-town SAM sites)
          if (bTownId != BLANK_SECTOR) {
            // can we keep militia in this town?
            if (MilitiaTrainingAllowedInSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                                               GetSolSectorZ(pSoldier)) == FALSE) {
              swprintf(sString, ARR_SIZE(sString), pMapErrorString[31], pTownNames[bTownId]);
              DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
              break;
            }

            // is the current loyalty high enough to train some?
            if (DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(pSoldier) == FALSE) {
              DoScreenIndependantMessageBox(zMarksMapScreenText[20], MSG_BOX_FLAG_OK, NULL);
              break;
            }
          }

          if (IsMilitiaTrainableFromSoldiersSectorMaxed(pSoldier)) {
            if (bTownId == BLANK_SECTOR) {
              // SAM site
              GetShortSectorString(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), sStringA,
                                   ARR_SIZE(sStringA));
              swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[21], sStringA);
            } else {
              // town
              swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[21], pTownNames[bTownId]);
            }

            DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
            break;
          }

          if (CountMilitiaTrainersInSoldiersSector(pSoldier) >= MAX_MILITIA_TRAINERS_PER_SECTOR) {
            swprintf(sString, ARR_SIZE(sString), gzLateLocalizedString[47],
                     MAX_MILITIA_TRAINERS_PER_SECTOR);
            DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
            break;
          }

          // PASSED ALL THE TESTS - ALLOW SOLDIER TO TRAIN MILITIA HERE

          pSoldier->bOldAssignment = pSoldier->bAssignment;

          if ((pSoldier->bAssignment != TRAIN_TOWN)) {
            SetTimeOfAssignmentChangeForMerc(pSoldier);
          }

          MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);

          // stop showing menu
          fShowAssignmentMenu = FALSE;
          giAssignHighLine = -1;

          // remove from squad

          if (pSoldier->bOldAssignment == VEHICLE) {
            TakeSoldierOutOfVehicle(pSoldier);
          }
          RemoveCharacterFromSquads(pSoldier);

          ChangeSoldiersAssignment(pSoldier, TRAIN_TOWN);

          // assign to a movement group
          AssignMercToAMovementGroup(pSoldier);
          if (!IsMilitiaTrainingPayedForSectorID8(GetSolSectorID8(pSoldier))) {
            // show a message to confirm player wants to charge cost
            HandleInterfaceMessageForCostOfTrainingMilitia(pSoldier);
          } else {
            SetAssignmentForList(TRAIN_TOWN, 0);
          }

          gfRenderPBInterface = TRUE;
        }
        break;
      case (TRAIN_MENU_TEAMMATES):

        if (CanCharacterTrainTeammates(pSoldier) == TRUE) {
          // train teammates
          gbTrainingMode = TRAIN_TEAMMATE;

          // show menu
          fShowAttributeMenu = TRUE;
          DetermineBoxPositions();
        }
        break;
      case (TRAIN_MENU_TRAIN_BY_OTHER):

        if (CanCharacterBeTrainedByOther(pSoldier) == TRUE) {
          // train teammates
          gbTrainingMode = TRAIN_BY_OTHER;

          // show menu
          fShowAttributeMenu = TRUE;
          DetermineBoxPositions();
        }
        break;
      case (TRAIN_MENU_CANCEL):
        // stop showing menu
        fShowTrainingMenu = FALSE;

        // unhighlight the assignment box
        UnHighLightBox(ghAssignmentBox);

        // reset list
        ResetSelectedListForMapScreen();
        gfRenderPBInterface = TRUE;

        break;
    }

    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if (fShowAttributeMenu) {
      // cancel attribute submenu
      fShowAttributeMenu = FALSE;
      // rerender tactical stuff
      gfRenderPBInterface = TRUE;

      return;
    }
  }
}

void AttributesMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment region
  int32_t iValue = -1;
  struct SOLDIERTYPE *pSoldier = NULL;

  pSoldier = GetSelectedAssignSoldier(FALSE);

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (iValue == ATTRIB_MENU_CANCEL) {
      // cancel, leave

      // stop showing menu
      fShowAttributeMenu = FALSE;

      // unhighlight the training box
      UnHighLightBox(ghTrainingBox);
    } else if (CanCharacterTrainStat(
                   pSoldier, (int8_t)(iValue),
                   (BOOLEAN)((gbTrainingMode == TRAIN_SELF) || (gbTrainingMode == TRAIN_BY_OTHER)),
                   (BOOLEAN)(gbTrainingMode == TRAIN_TEAMMATE))) {
      pSoldier->bOldAssignment = pSoldier->bAssignment;

      if ((pSoldier->bAssignment != gbTrainingMode)) {
        SetTimeOfAssignmentChangeForMerc(pSoldier);
      }

      // set stat to train
      pSoldier->bTrainStat = (int8_t)iValue;

      MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);

      // stop showing ALL menus
      fShowAssignmentMenu = FALSE;
      giAssignHighLine = -1;

      // remove from squad/vehicle
      if (pSoldier->bOldAssignment == VEHICLE) {
        TakeSoldierOutOfVehicle(pSoldier);
      }
      RemoveCharacterFromSquads(pSoldier);

      // train stat
      ChangeSoldiersAssignment(pSoldier, gbTrainingMode);

      // assign to a movement group
      AssignMercToAMovementGroup(pSoldier);

      // set assignment for group
      SetAssignmentForList(gbTrainingMode, (int8_t)iValue);
    }

    // rerender tactical stuff
    gfRenderPBInterface = TRUE;

    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
  }
};

void AssignmentMenuBtnCallback(struct MOUSE_REGION *pRegion, int32_t iReason) {
  // btn callback handler for assignment region
  int32_t iValue = -1;
  wchar_t sString[128];

  struct SOLDIERTYPE *pSoldier = NULL;

  pSoldier = GetSelectedAssignSoldier(FALSE);

  iValue = MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if ((fShowAttributeMenu) || (fShowTrainingMenu) || (fShowRepairMenu) || (fShowVehicleMenu) ||
        (fShowSquadMenu)) {
      return;
    }

    UnHighLightBox(ghAssignmentBox);

    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      switch (iValue) {
        case (EPC_MENU_ON_DUTY):
          if (CanCharacterOnDuty(pSoldier)) {
            // put character on a team
            fShowSquadMenu = TRUE;
            fShowTrainingMenu = FALSE;
            fShowVehicleMenu = FALSE;
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;
          }
          break;
        case (EPC_MENU_PATIENT):
          // can character doctor?
          if (CanCharacterPatient(pSoldier)) {
            /* Assignment distance limits removed.  Sep/11/98.  ARM
                                                            if( IsSoldierCloseEnoughToADoctor(
               pSoldier ) == FALSE )
                                                            {
                                                                    return;
                                                            }
            */

            pSoldier->bOldAssignment = pSoldier->bAssignment;

            if ((pSoldier->bAssignment != PATIENT)) {
              SetTimeOfAssignmentChangeForMerc(pSoldier);
            }

            // stop showing menu
            fShowAssignmentMenu = FALSE;
            giAssignHighLine = -1;

            MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);

            // set dirty flag
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;

            // remove from squad

            if (pSoldier->bOldAssignment == VEHICLE) {
              TakeSoldierOutOfVehicle(pSoldier);
            }
            RemoveCharacterFromSquads(pSoldier);
            ChangeSoldiersAssignment(pSoldier, PATIENT);
            AssignMercToAMovementGroup(pSoldier);

            // set assignment for group
            SetAssignmentForList((int8_t)PATIENT, 0);
          }
          break;

        case (EPC_MENU_VEHICLE):
          if (CanCharacterVehicle(pSoldier)) {
            if (DisplayVehicleMenu(pSoldier)) {
              fShowVehicleMenu = TRUE;
              ShowBox(ghVehicleBox);
            } else {
              fShowVehicleMenu = FALSE;
            }
          }
          break;

        case (EPC_MENU_REMOVE):

          fShowAssignmentMenu = FALSE;
          UnEscortEPC(pSoldier);
          break;

        case (EPC_MENU_CANCEL):
          fShowAssignmentMenu = FALSE;
          giAssignHighLine = -1;

          // set dirty flag
          fTeamPanelDirty = TRUE;
          fMapScreenBottomDirty = TRUE;

          // reset list of characters
          ResetSelectedListForMapScreen();
          break;
      }
    } else {
      switch (iValue) {
        case (ASSIGN_MENU_ON_DUTY):
          if (CanCharacterOnDuty(pSoldier)) {
            // put character on a team
            fShowSquadMenu = TRUE;
            fShowTrainingMenu = FALSE;
            fShowVehicleMenu = FALSE;
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;
            fShowRepairMenu = FALSE;
          }
          break;
        case (ASSIGN_MENU_DOCTOR):

          // can character doctor?
          if (CanCharacterDoctor(pSoldier)) {
            // stop showing menu
            fShowAssignmentMenu = FALSE;
            giAssignHighLine = -1;

            pSoldier->bOldAssignment = pSoldier->bAssignment;

            if ((pSoldier->bAssignment != DOCTOR)) {
              SetTimeOfAssignmentChangeForMerc(pSoldier);
            }

            // remove from squad

            if (pSoldier->bOldAssignment == VEHICLE) {
              TakeSoldierOutOfVehicle(pSoldier);
            }
            RemoveCharacterFromSquads(pSoldier);

            ChangeSoldiersAssignment(pSoldier, DOCTOR);

            MakeSureMedKitIsInHand(pSoldier);
            AssignMercToAMovementGroup(pSoldier);

            MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);

            // set dirty flag
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;

            // set assignment for group
            SetAssignmentForList((int8_t)DOCTOR, 0);
          } else if (CanCharacterDoctorButDoesntHaveMedKit(pSoldier)) {
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;
            swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[19], pSoldier->name);

            DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
          }

          break;
        case (ASSIGN_MENU_PATIENT):

          // can character patient?
          if (CanCharacterPatient(pSoldier)) {
            /* Assignment distance limits removed.  Sep/11/98.  ARM
                                                            if( IsSoldierCloseEnoughToADoctor(
               pSoldier ) == FALSE )
                                                            {
                                                                    return;
                                                            }
            */

            pSoldier->bOldAssignment = pSoldier->bAssignment;

            if ((pSoldier->bAssignment != PATIENT)) {
              SetTimeOfAssignmentChangeForMerc(pSoldier);
            }

            MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);

            // stop showing menu
            fShowAssignmentMenu = FALSE;
            giAssignHighLine = -1;

            // set dirty flag
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;

            // remove from squad

            if (pSoldier->bOldAssignment == VEHICLE) {
              TakeSoldierOutOfVehicle(pSoldier);
            }
            RemoveCharacterFromSquads(pSoldier);
            ChangeSoldiersAssignment(pSoldier, PATIENT);

            AssignMercToAMovementGroup(pSoldier);

            // set assignment for group
            SetAssignmentForList((int8_t)PATIENT, 0);
          }
          break;

        case (ASSIGN_MENU_VEHICLE):
          if (CanCharacterVehicle(pSoldier)) {
            if (DisplayVehicleMenu(pSoldier)) {
              fShowVehicleMenu = TRUE;
              ShowBox(ghVehicleBox);
            } else {
              fShowVehicleMenu = FALSE;
            }
          }
          break;
        case (ASSIGN_MENU_REPAIR):
          if (CanCharacterRepair(pSoldier)) {
            fShowSquadMenu = FALSE;
            fShowTrainingMenu = FALSE;
            fShowVehicleMenu = FALSE;
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;

            pSoldier->bOldAssignment = pSoldier->bAssignment;

            if (GetSolSectorZ(pSoldier) == 0) {
              fShowRepairMenu = FALSE;

              if (DisplayRepairMenu(pSoldier)) {
                fShowRepairMenu = TRUE;
              }
            }

          } else if (CanCharacterRepairButDoesntHaveARepairkit(pSoldier)) {
            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;
            swprintf(sString, ARR_SIZE(sString), zMarksMapScreenText[18], pSoldier->name);

            DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
          }
          break;
        case (ASSIGN_MENU_TRAIN):
          if (CanCharacterPractise(pSoldier)) {
            fShowTrainingMenu = TRUE;
            DetermineBoxPositions();
            fShowSquadMenu = FALSE;
            fShowVehicleMenu = FALSE;
            fShowRepairMenu = FALSE;

            fTeamPanelDirty = TRUE;
            fMapScreenBottomDirty = TRUE;
          }
          break;
        case (ASSIGN_MENU_CANCEL):
          fShowAssignmentMenu = FALSE;
          giAssignHighLine = -1;

          // set dirty flag
          fTeamPanelDirty = TRUE;
          fMapScreenBottomDirty = TRUE;

          // reset list of characters
          ResetSelectedListForMapScreen();
          break;
      }
    }
    gfRenderPBInterface = TRUE;

  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    if ((fShowAttributeMenu) || (fShowTrainingMenu) || (fShowRepairMenu) || (fShowVehicleMenu) ||
        (fShowSquadMenu)) {
      fShowAttributeMenu = FALSE;
      fShowTrainingMenu = FALSE;
      fShowRepairMenu = FALSE;
      fShowVehicleMenu = FALSE;
      fShowSquadMenu = FALSE;

      // rerender tactical stuff
      gfRenderPBInterface = TRUE;

      // set dirty flag
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;

      return;
    }
  }
}

void RestorePopUpBoxes(void) {
  ContractPosition.iX = OrigContractPosition.iX;
  AttributePosition.iX = OrigAttributePosition.iX;
  SquadPosition.iX = OrigSquadPosition.iX;
  AssignmentPosition.iX = OrigAssignmentPosition.iX;
  TrainPosition.iX = OrigTrainPosition.iX;
  VehiclePosition.iX = OrigVehiclePosition.iX;

  return;
}

void CreateSquadBox(void) {
  // will create a pop up box for squad selection
  SGPPoint pPoint;
  SGPRect pDimensions;
  uint32_t hStringHandle;
  uint32_t uiCounter;
  wchar_t sString[64];
  uint32_t uiMaxSquad;

  // create basic box
  CreatePopUpBox(&ghSquadBox, SquadDimensions, SquadPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_RESIZE));

  // which buffer will box render to
  SetBoxBuffer(ghSquadBox, vsFB);

  // border type?
  SetBorderType(ghSquadBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghSquadBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghSquadBox, 6, 6, 4, 4);

  // space between lines
  SetLineSpace(ghSquadBox, 2);

  // set current box to this one
  SetCurrentBox(ghSquadBox);

  uiMaxSquad = GetLastSquadListedInSquadMenu();

  // add strings for box
  for (uiCounter = 0; uiCounter <= uiMaxSquad; uiCounter++) {
    // get info about current squad and put in  string
    swprintf(sString, ARR_SIZE(sString), L"%s ( %d/%d )", pSquadMenuStrings[uiCounter],
             NumberOfPeopleInSquad((int8_t)uiCounter), NUMBER_OF_SOLDIERS_PER_SQUAD);
    AddMonoString(&hStringHandle, sString);

    // make sure it is unhighlighted
    UnHighLightLine(hStringHandle);
  }

  // add cancel line
  AddMonoString(&hStringHandle, pSquadMenuStrings[NUMBER_OF_SQUADS]);

  // set font type
  SetBoxFont(ghSquadBox, MAP_SCREEN_FONT);

  // set highlight color
  SetBoxHighLight(ghSquadBox, FONT_WHITE);

  // unhighlighted color
  SetBoxForeground(ghSquadBox, FONT_LTGREEN);

  // the secondary shade color
  SetBoxSecondaryShade(ghSquadBox, FONT_YELLOW);

  // background color
  SetBoxBackground(ghSquadBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghSquadBox, FONT_GRAY7);

  // resize box to text
  ResizeBoxToText(ghSquadBox);

  DetermineBoxPositions();

  GetBoxPosition(ghSquadBox, &pPoint);
  GetBoxSize(ghSquadBox, &pDimensions);

  if (giBoxY + pDimensions.iBottom > 479) {
    pPoint.iY = SquadPosition.iY = 479 - pDimensions.iBottom;
  }

  SetBoxPosition(ghSquadBox, pPoint);
}

void CreateEPCBox(void) {
  // will create a pop up box for squad selection
  SGPPoint pPoint;
  SGPRect pDimensions;
  uint32_t hStringHandle;
  int32_t iCount;

  // create basic box
  CreatePopUpBox(&ghEpcBox, SquadDimensions, AssignmentPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_RESIZE | POPUP_BOX_FLAG_CENTER_TEXT));

  // which buffer will box render to
  SetBoxBuffer(ghEpcBox, vsFB);

  // border type?
  SetBorderType(ghEpcBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghEpcBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghEpcBox, 6, 6, 4, 4);

  // space between lines
  SetLineSpace(ghEpcBox, 2);

  // set current box to this one
  SetCurrentBox(ghEpcBox);

  for (iCount = 0; iCount < MAX_EPC_MENU_STRING_COUNT; iCount++) {
    AddMonoString(&hStringHandle, pEpcMenuStrings[iCount]);
  }

  // set font type
  SetBoxFont(ghEpcBox, MAP_SCREEN_FONT);

  // set highlight color
  SetBoxHighLight(ghEpcBox, FONT_WHITE);

  // unhighlighted color
  SetBoxForeground(ghEpcBox, FONT_LTGREEN);

  // background color
  SetBoxBackground(ghEpcBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghEpcBox, FONT_GRAY7);

  // resize box to text
  ResizeBoxToText(ghEpcBox);

  GetBoxPosition(ghEpcBox, &pPoint);

  GetBoxSize(ghEpcBox, &pDimensions);

  if (giBoxY + pDimensions.iBottom > 479) {
    pPoint.iY = AssignmentPosition.iY = 479 - pDimensions.iBottom;
  }

  SetBoxPosition(ghEpcBox, pPoint);
}

void HandleShadingOfLinesForSquadMenu(void) {
  // find current squad and set that line the squad box a lighter green
  uint32_t uiCounter;
  struct SOLDIERTYPE *pSoldier = NULL;
  uint32_t uiMaxSquad;
  int8_t bResult;

  if ((fShowSquadMenu == FALSE) || (ghSquadBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(FALSE);

  uiMaxSquad = GetLastSquadListedInSquadMenu();

  for (uiCounter = 0; uiCounter <= uiMaxSquad; uiCounter++) {
    if (pSoldier != NULL) {
      bResult = CanCharacterSquad(pSoldier, (int8_t)uiCounter);
    }

    // if no soldier, or a reason which doesn't have a good explanatory message
    if ((pSoldier == NULL) || (bResult == CHARACTER_CANT_JOIN_SQUAD)) {
      // darken /disable line
      ShadeStringInBox(ghSquadBox, uiCounter);
    } else {
      if (bResult == CHARACTER_CAN_JOIN_SQUAD) {
        // legal squad, leave it green
        UnShadeStringInBox(ghSquadBox, uiCounter);
        UnSecondaryShadeStringInBox(ghSquadBox, uiCounter);
      } else {
        // unjoinable squad - yellow
        SecondaryShadeStringInBox(ghSquadBox, uiCounter);
      }
    }
  }
}

void PostContractMessage(struct SOLDIERTYPE *pCharacter, int32_t iContract) {
  // do nothing
  return;

  // send a message stating that offer of contract extension made
  // MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Offered to extend %s's contract by
  // another %s.", pCharacter -> name, pContractExtendStrings[ iContract ] );

  return;
}

void PostTerminateMessage(struct SOLDIERTYPE *pCharacter) {
  // do nothing
  return;

  // send a message stating that termination of contract done
  // MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"%s's contract has been terminated.",
  // pCharacter -> name );
}

BOOLEAN DisplayVehicleMenu(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fVehiclePresent = FALSE;
  int32_t iCounter = 0;
  uint32_t hStringHandle = 0;

  // first, clear pop up box
  RemoveBox(ghVehicleBox);
  ghVehicleBox = -1;

  CreateVehicleBox();
  SetCurrentBox(ghVehicleBox);

  // run through list of vehicles in sector and add them to pop up box
  for (iCounter = 0; iCounter < ubNumberOfVehicles; iCounter++) {
    if (pVehicleList[iCounter].fValid == TRUE) {
      if (IsThisVehicleAccessibleToSoldier(pSoldier, iCounter)) {
        AddMonoString(&hStringHandle, pVehicleStrings[pVehicleList[iCounter].ubVehicleType]);
        fVehiclePresent = TRUE;
      }
    }
  }

  // cancel string (borrow the one in the squad menu)
  AddMonoString(&hStringHandle, pSquadMenuStrings[SQUAD_MENU_CANCEL]);

  SetBoxFont(ghVehicleBox, MAP_SCREEN_FONT);
  SetBoxHighLight(ghVehicleBox, FONT_WHITE);
  SetBoxForeground(ghVehicleBox, FONT_LTGREEN);
  SetBoxBackground(ghVehicleBox, FONT_BLACK);

  return fVehiclePresent;
}

void CreateVehicleBox() {
  CreatePopUpBox(&ghVehicleBox, VehicleDimensions, VehiclePosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_CENTER_TEXT | POPUP_BOX_FLAG_RESIZE));
  SetBoxBuffer(ghVehicleBox, vsFB);
  SetBorderType(ghVehicleBox, guiPOPUPBORDERS);
  SetBackGroundSurface(ghVehicleBox, vsPOPUPTEX);
  SetMargins(ghVehicleBox, 6, 6, 4, 4);
  SetLineSpace(ghVehicleBox, 2);
}

void CreateRepairBox(void) {
  CreatePopUpBox(&ghRepairBox, RepairDimensions, RepairPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_CENTER_TEXT | POPUP_BOX_FLAG_RESIZE));
  SetBoxBuffer(ghRepairBox, vsFB);
  SetBorderType(ghRepairBox, guiPOPUPBORDERS);
  SetBackGroundSurface(ghRepairBox, vsPOPUPTEX);
  SetMargins(ghRepairBox, 6, 6, 4, 4);
  SetLineSpace(ghRepairBox, 2);
}

void CreateContractBox(struct SOLDIERTYPE *pCharacter) {
  uint32_t hStringHandle;
  uint32_t uiCounter;
  wchar_t sString[50];
  wchar_t sDollarString[50];

  ContractPosition.iX = OrigContractPosition.iX;

  if (giBoxY != 0) {
    ContractPosition.iX = giBoxY;
  }

  CreatePopUpBox(&ghContractBox, ContractDimensions, ContractPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_RESIZE));
  SetBoxBuffer(ghContractBox, vsFB);
  SetBorderType(ghContractBox, guiPOPUPBORDERS);
  SetBackGroundSurface(ghContractBox, vsPOPUPTEX);
  SetMargins(ghContractBox, 6, 6, 4, 4);
  SetLineSpace(ghContractBox, 2);

  // set current box to this one
  SetCurrentBox(ghContractBox);

  // not null character?
  if (pCharacter != NULL) {
    for (uiCounter = 0; uiCounter < MAX_CONTRACT_MENU_STRING_COUNT; uiCounter++) {
      switch (uiCounter) {
        case (CONTRACT_MENU_CURRENT_FUNDS):
          /*
                                          // add current balance after title string
                                           swprintf( sDollarString, L"%d",
             MoneyGetBalance()); InsertCommasForDollarFigure( sDollarString );
                                           InsertDollarSignInToString( sDollarString );
                                           swprintf( sString, L"%s %s", pContractStrings[uiCounter],
             sDollarString ); AddMonoString(&hStringHandle, sString);
          */
          AddMonoString(&hStringHandle, pContractStrings[uiCounter]);
          break;
        case (CONTRACT_MENU_DAY):

          if (pCharacter->ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC) {
            swprintf(sDollarString, ARR_SIZE(sDollarString), L"%d", 0);
          } else {
            swprintf(sDollarString, ARR_SIZE(sDollarString), L"%d",
                     gMercProfiles[pCharacter->ubProfile].sSalary);
          }
          InsertCommasForDollarFigure(sDollarString);
          InsertDollarSignInToString(sDollarString);
          swprintf(sString, ARR_SIZE(sString), L"%s ( %s )", pContractStrings[uiCounter],
                   sDollarString);
          AddMonoString(&hStringHandle, sString);
          break;
        case (CONTRACT_MENU_WEEK):

          if (pCharacter->ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC) {
            swprintf(sDollarString, ARR_SIZE(sDollarString), L"%d", 0);
          } else {
            swprintf(sDollarString, ARR_SIZE(sDollarString), L"%d",
                     gMercProfiles[pCharacter->ubProfile].uiWeeklySalary);
          }

          InsertCommasForDollarFigure(sDollarString);
          InsertDollarSignInToString(sDollarString);
          swprintf(sString, ARR_SIZE(sString), L"%s ( %s )", pContractStrings[uiCounter],
                   sDollarString);
          AddMonoString(&hStringHandle, sString);
          break;
        case (CONTRACT_MENU_TWO_WEEKS):

          if (pCharacter->ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC) {
            swprintf(sDollarString, ARR_SIZE(sDollarString), L"%d", 0);
          } else {
            swprintf(sDollarString, ARR_SIZE(sDollarString), L"%d",
                     gMercProfiles[pCharacter->ubProfile].uiBiWeeklySalary);
          }

          InsertCommasForDollarFigure(sDollarString);
          InsertDollarSignInToString(sDollarString);
          swprintf(sString, ARR_SIZE(sString), L"%s ( %s )", pContractStrings[uiCounter],
                   sDollarString);
          AddMonoString(&hStringHandle, sString);
          break;
        default:
          AddMonoString(&hStringHandle, pContractStrings[uiCounter]);
          break;
      }
      UnHighLightLine(hStringHandle);
    }
  }

  SetBoxFont(ghContractBox, MAP_SCREEN_FONT);
  SetBoxHighLight(ghContractBox, FONT_WHITE);
  SetBoxForeground(ghContractBox, FONT_LTGREEN);
  SetBoxBackground(ghContractBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghContractBox, FONT_GRAY7);

  if (pCharacter != NULL) {
    // now set the color for the current balance value
    SetBoxLineForeground(ghContractBox, 0, FONT_YELLOW);
  }

  // resize box to text
  ResizeBoxToText(ghContractBox);
}

void CreateAttributeBox(void) {
  uint32_t hStringHandle;
  uint32_t uiCounter;

  // will create attribute pop up menu for mapscreen assignments

  AttributePosition.iX = OrigAttributePosition.iX;

  if (giBoxY != 0) {
    AttributePosition.iY = giBoxY;
  }

  // update screen assignment positions
  UpdateMapScreenAssignmentPositions();

  // create basic box
  CreatePopUpBox(&ghAttributeBox, AttributeDimensions, AttributePosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_CENTER_TEXT | POPUP_BOX_FLAG_RESIZE));

  // which buffer will box render to
  SetBoxBuffer(ghAttributeBox, vsFB);

  // border type?
  SetBorderType(ghAttributeBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghAttributeBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghAttributeBox, 6, 6, 4, 4);

  // space between lines
  SetLineSpace(ghAttributeBox, 2);

  // set current box to this one
  SetCurrentBox(ghAttributeBox);

  // add strings for box
  for (uiCounter = 0; uiCounter < MAX_ATTRIBUTE_STRING_COUNT; uiCounter++) {
    AddMonoString(&hStringHandle, pAttributeMenuStrings[uiCounter]);

    // make sure it is unhighlighted
    UnHighLightLine(hStringHandle);
  }

  // set font type
  SetBoxFont(ghAttributeBox, MAP_SCREEN_FONT);

  // set highlight color
  SetBoxHighLight(ghAttributeBox, FONT_WHITE);

  // unhighlighted color
  SetBoxForeground(ghAttributeBox, FONT_LTGREEN);

  // background color
  SetBoxBackground(ghAttributeBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghAttributeBox, FONT_GRAY7);

  // resize box to text
  ResizeBoxToText(ghAttributeBox);
}

void CreateTrainingBox(void) {
  uint32_t hStringHandle;
  uint32_t uiCounter;

  // will create attribute pop up menu for mapscreen assignments

  TrainPosition.iX = OrigTrainPosition.iX;

  if (giBoxY != 0) {
    TrainPosition.iY = giBoxY + (ASSIGN_MENU_TRAIN * GetFontHeight(MAP_SCREEN_FONT));
  }

  // create basic box
  CreatePopUpBox(&ghTrainingBox, TrainDimensions, TrainPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_CENTER_TEXT | POPUP_BOX_FLAG_RESIZE));

  // which buffer will box render to
  SetBoxBuffer(ghTrainingBox, vsFB);

  // border type?
  SetBorderType(ghTrainingBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghTrainingBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghTrainingBox, 6, 6, 4, 4);

  // space between lines
  SetLineSpace(ghTrainingBox, 2);

  // set current box to this one
  SetCurrentBox(ghTrainingBox);

  // add strings for box
  for (uiCounter = 0; uiCounter < MAX_TRAIN_STRING_COUNT; uiCounter++) {
    AddMonoString(&hStringHandle, pTrainingMenuStrings[uiCounter]);

    // make sure it is unhighlighted
    UnHighLightLine(hStringHandle);
  }

  // set font type
  SetBoxFont(ghTrainingBox, MAP_SCREEN_FONT);

  // set highlight color
  SetBoxHighLight(ghTrainingBox, FONT_WHITE);

  // unhighlighted color
  SetBoxForeground(ghTrainingBox, FONT_LTGREEN);

  // background color
  SetBoxBackground(ghTrainingBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghTrainingBox, FONT_GRAY7);

  // resize box to text
  ResizeBoxToText(ghTrainingBox);

  DetermineBoxPositions();
}

void CreateAssignmentsBox(void) {
  uint32_t hStringHandle;
  uint32_t uiCounter;
  wchar_t sString[128];
  struct SOLDIERTYPE *pSoldier = NULL;

  // will create attribute pop up menu for mapscreen assignments

  AssignmentPosition.iX = OrigAssignmentPosition.iX;

  if (giBoxY != 0) {
    AssignmentPosition.iY = giBoxY;
  }

  pSoldier = GetSelectedAssignSoldier(TRUE);
  // pSoldier NULL is legal here!  Gets called during every mapscreen initialization even when
  // nobody is assign char

  // create basic box
  CreatePopUpBox(&ghAssignmentBox, AssignmentDimensions, AssignmentPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_CENTER_TEXT | POPUP_BOX_FLAG_RESIZE));

  // which buffer will box render to
  SetBoxBuffer(ghAssignmentBox, vsFB);

  // border type?
  SetBorderType(ghAssignmentBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghAssignmentBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghAssignmentBox, 6, 6, 4, 4);

  // space between lines
  SetLineSpace(ghAssignmentBox, 2);

  // set current box to this one
  SetCurrentBox(ghAssignmentBox);

  // add strings for box
  for (uiCounter = 0; uiCounter < MAX_ASSIGN_STRING_COUNT; uiCounter++) {
    // if we have a soldier, and this is the squad line
    if ((uiCounter == ASSIGN_MENU_ON_DUTY) && (pSoldier != NULL) &&
        (pSoldier->bAssignment < ON_DUTY)) {
      // show his squad # in brackets
      swprintf(sString, ARR_SIZE(sString), L"%s(%d)", pAssignMenuStrings[uiCounter],
               pSoldier->bAssignment + 1);
    } else {
      swprintf(sString, ARR_SIZE(sString), pAssignMenuStrings[uiCounter]);
    }

    AddMonoString(&hStringHandle, sString);

    // make sure it is unhighlighted
    UnHighLightLine(hStringHandle);
  }

  // set font type
  SetBoxFont(ghAssignmentBox, MAP_SCREEN_FONT);

  // set highlight color
  SetBoxHighLight(ghAssignmentBox, FONT_WHITE);

  // unhighlighted color
  SetBoxForeground(ghAssignmentBox, FONT_LTGREEN);

  // background color
  SetBoxBackground(ghAssignmentBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghAssignmentBox, FONT_GRAY7);
  SetBoxSecondaryShade(ghAssignmentBox, FONT_YELLOW);

  // resize box to text
  ResizeBoxToText(ghAssignmentBox);

  DetermineBoxPositions();
}

void CreateMercRemoveAssignBox(void) {
  // will create remove mercbox to be placed in assignment area

  uint32_t hStringHandle;
  uint32_t uiCounter;
  // create basic box
  CreatePopUpBox(&ghRemoveMercAssignBox, AssignmentDimensions, AssignmentPosition,
                 (POPUP_BOX_FLAG_CLIP_TEXT | POPUP_BOX_FLAG_CENTER_TEXT | POPUP_BOX_FLAG_RESIZE));

  // which buffer will box render to
  SetBoxBuffer(ghRemoveMercAssignBox, vsFB);

  // border type?
  SetBorderType(ghRemoveMercAssignBox, guiPOPUPBORDERS);

  // background texture
  SetBackGroundSurface(ghRemoveMercAssignBox, vsPOPUPTEX);

  // margin sizes
  SetMargins(ghRemoveMercAssignBox, 6, 6, 4, 4);

  // space between lines
  SetLineSpace(ghRemoveMercAssignBox, 2);

  // set current box to this one
  SetCurrentBox(ghRemoveMercAssignBox);

  // add strings for box
  for (uiCounter = 0; uiCounter < MAX_REMOVE_MERC_COUNT; uiCounter++) {
    AddMonoString(&hStringHandle, pRemoveMercStrings[uiCounter]);

    // make sure it is unhighlighted
    UnHighLightLine(hStringHandle);
  }

  // set font type
  SetBoxFont(ghRemoveMercAssignBox, MAP_SCREEN_FONT);

  // set highlight color
  SetBoxHighLight(ghRemoveMercAssignBox, FONT_WHITE);

  // unhighlighted color
  SetBoxForeground(ghRemoveMercAssignBox, FONT_LTGREEN);

  // background color
  SetBoxBackground(ghRemoveMercAssignBox, FONT_BLACK);

  // shaded color..for darkened text
  SetBoxShade(ghRemoveMercAssignBox, FONT_GRAY7);

  // resize box to text
  ResizeBoxToText(ghRemoveMercAssignBox);
}

BOOLEAN CreateDestroyAssignmentPopUpBoxes(void) {
  static BOOLEAN fCreated = FALSE;

  if ((fShowAssignmentMenu == TRUE) && (fCreated == FALSE)) {
    CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\popup.sti"), &guiPOPUPBORDERS));

    vsPOPUPTEX = CreateVSurfaceFromFile("INTERFACE\\popupbackground.pcx");
    if (vsPOPUPTEX == NULL) {
      return FALSE;
    }
    JSurface_SetColorKey(vsPOPUPTEX, FROMRGB(0, 0, 0));

    // these boxes are always created while in mapscreen...
    CreateEPCBox();
    CreateAssignmentsBox();
    CreateTrainingBox();
    CreateAttributeBox();
    CreateVehicleBox();
    CreateRepairBox();

    UpdateMapScreenAssignmentPositions();

    fCreated = TRUE;
  } else if ((fShowAssignmentMenu == FALSE) && (fCreated == TRUE)) {
    DeleteVideoObjectFromIndex(guiPOPUPBORDERS);
    JSurface_Free(vsPOPUPTEX);
    vsPOPUPTEX = NULL;

    RemoveBox(ghAttributeBox);
    ghAttributeBox = -1;

    RemoveBox(ghVehicleBox);
    ghVehicleBox = -1;

    RemoveBox(ghAssignmentBox);
    ghAssignmentBox = -1;

    RemoveBox(ghEpcBox);
    ghEpcBox = -1;

    RemoveBox(ghRepairBox);
    ghRepairBox = -1;

    RemoveBox(ghTrainingBox);
    ghTrainingBox = -1;

    fCreated = FALSE;
    gfIgnoreScrolling = FALSE;
    RebuildCurrentSquad();
  }

  return (TRUE);
}

void DetermineBoxPositions(void) {
  // depending on how many boxes there are, reposition as needed
  SGPPoint pPoint;
  SGPPoint pNewPoint;
  SGPRect pDimensions;
  struct SOLDIERTYPE *pSoldier = NULL;

  if ((fShowAssignmentMenu == FALSE) || (ghAssignmentBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(TRUE);
  // pSoldier NULL is legal here!  Gets called during every mapscreen initialization even when
  // nobody is assign char
  if (pSoldier == NULL) {
    return;
  }

  if ((IsMapScreen())) {
    GetBoxPosition(ghAssignmentBox, &pPoint);
    gsAssignmentBoxesX = (int16_t)pPoint.iX;
    gsAssignmentBoxesY = (int16_t)pPoint.iY;
  }

  pPoint.iX = gsAssignmentBoxesX;
  pPoint.iY = gsAssignmentBoxesY;

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    SetBoxPosition(ghEpcBox, pPoint);
    GetBoxSize(ghEpcBox, &pDimensions);
  } else {
    SetBoxPosition(ghAssignmentBox, pPoint);
    GetBoxSize(ghAssignmentBox, &pDimensions);
  }

  // hang it right beside the assignment/EPC box menu
  pNewPoint.iX = pPoint.iX + pDimensions.iRight;
  pNewPoint.iY = pPoint.iY;

  if ((fShowSquadMenu == TRUE) && (ghSquadBox != -1)) {
    SetBoxPosition(ghSquadBox, pNewPoint);
  }

  if ((fShowRepairMenu == TRUE) && (ghRepairBox != -1)) {
    CreateDestroyMouseRegionForRepairMenu();
    pNewPoint.iY += ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR);

    SetBoxPosition(ghRepairBox, pNewPoint);
  }

  if ((fShowTrainingMenu == TRUE) && (ghTrainingBox != -1)) {
    pNewPoint.iY += ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN);
    SetBoxPosition(ghTrainingBox, pNewPoint);
    TrainPosition.iX = pNewPoint.iX;
    TrainPosition.iY = pNewPoint.iY;
    OrigTrainPosition.iY = pNewPoint.iY;
    OrigTrainPosition.iX = pNewPoint.iX;

    GetBoxSize(ghTrainingBox, &pDimensions);
    GetBoxPosition(ghTrainingBox, &pPoint);

    if ((fShowAttributeMenu == TRUE) && (ghAttributeBox != -1)) {
      // hang it right beside the training box menu
      pNewPoint.iX = pPoint.iX + pDimensions.iRight;
      pNewPoint.iY = pPoint.iY;
      SetBoxPosition(ghAttributeBox, pNewPoint);
    }
  }

  return;
}

void SetTacticalPopUpAssignmentBoxXY(void) {
  int16_t sX, sY;
  struct SOLDIERTYPE *pSoldier;

  // get the soldier
  pSoldier = GetSelectedAssignSoldier(FALSE);

  // grab soldier's x,y screen position
  GetSoldierScreenPos(pSoldier, &sX, &sY);

  if (sX < 0) {
    sX = 0;
  }

  gsAssignmentBoxesX = sX + 30;

  if (sY < 0) {
    sY = 0;
  }

  gsAssignmentBoxesY = sY;

  // ATE: Check if we are past tactical viewport....
  // Use estimate width's/heights
  if ((gsAssignmentBoxesX + 100) > 640) {
    gsAssignmentBoxesX = 540;
  }

  if ((gsAssignmentBoxesY + 130) > 320) {
    gsAssignmentBoxesY = 190;
  }

  return;
}

void RepositionMouseRegions(void) {
  int16_t sDeltaX, sDeltaY;
  int32_t iCounter = 0;

  if (fShowAssignmentMenu == TRUE) {
    sDeltaX = gsAssignmentBoxesX - gAssignmentMenuRegion[0].RegionTopLeftX;
    sDeltaY = (int16_t)(gsAssignmentBoxesY - gAssignmentMenuRegion[0].RegionTopLeftY +
                        GetTopMarginSize(ghAssignmentBox));

    // find the delta from the old to the new, and alter values accordingly
    for (iCounter = 0; iCounter < (int32_t)GetNumberOfLinesOfTextInBox(ghAssignmentBox);
         iCounter++) {
      gAssignmentMenuRegion[iCounter].RegionTopLeftX += sDeltaX;
      gAssignmentMenuRegion[iCounter].RegionTopLeftY += sDeltaY;

      gAssignmentMenuRegion[iCounter].RegionBottomRightX += sDeltaX;
      gAssignmentMenuRegion[iCounter].RegionBottomRightY += sDeltaY;
    }

    gfPausedTacticalRenderFlags = RENDER_FLAG_FULL;
  }
}

void CheckAndUpdateTacticalAssignmentPopUpPositions(void) {
  SGPRect pDimensions, pDimensions2, pDimensions3;
  SGPPoint pPoint;
  int16_t sLongest;
  struct SOLDIERTYPE *pSoldier = NULL;

  if (fShowAssignmentMenu == FALSE) {
    return;
  }

  if ((IsMapScreen())) {
    return;
  }

  // get the soldier
  pSoldier = GetSelectedAssignSoldier(FALSE);

  if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
    GetBoxSize(ghEpcBox, &pDimensions2);
  } else {
    GetBoxSize(ghAssignmentBox, &pDimensions2);
  }

  if (fShowRepairMenu == TRUE) {
    GetBoxSize(ghRepairBox, &pDimensions);

    if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight >= 640) {
      gsAssignmentBoxesX = (int16_t)(639 - (pDimensions2.iRight + pDimensions.iRight));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    if (pDimensions2.iBottom > pDimensions.iBottom) {
      sLongest = (int16_t)pDimensions2.iBottom +
                 ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR);
    } else {
      sLongest = (int16_t)pDimensions.iBottom +
                 ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR);
    }

    if (gsAssignmentBoxesY + sLongest >= 360) {
      gsAssignmentBoxesY = (int16_t)(359 - (sLongest));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
    pPoint.iY = gsAssignmentBoxesY + ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR);

    SetBoxPosition(ghRepairBox, pPoint);
  } else if (fShowSquadMenu == TRUE) {
    GetBoxSize(ghSquadBox, &pDimensions);

    if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight >= 640) {
      gsAssignmentBoxesX = (int16_t)(639 - (pDimensions2.iRight + pDimensions.iRight));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    if (pDimensions2.iBottom > pDimensions.iBottom) {
      sLongest = (int16_t)pDimensions2.iBottom;
    } else {
      sLongest = (int16_t)pDimensions.iBottom;
    }

    if (gsAssignmentBoxesY + sLongest >= 360) {
      gsAssignmentBoxesY = (int16_t)(359 - (sLongest));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
    pPoint.iY = gsAssignmentBoxesY;

    SetBoxPosition(ghSquadBox, pPoint);
  } else if (fShowAttributeMenu == TRUE) {
    GetBoxSize(ghTrainingBox, &pDimensions);
    GetBoxSize(ghAttributeBox, &pDimensions3);

    if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight + pDimensions3.iRight >=
        640) {
      gsAssignmentBoxesX =
          (int16_t)(639 - (pDimensions2.iRight + pDimensions.iRight + pDimensions3.iRight));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    if (gsAssignmentBoxesY + pDimensions3.iBottom +
            (GetFontHeight(MAP_SCREEN_FONT) * ASSIGN_MENU_TRAIN) >=
        360) {
      gsAssignmentBoxesY = (int16_t)(359 - (pDimensions3.iBottom));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight;
    pPoint.iY = gsAssignmentBoxesY;

    pPoint.iY += ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN);
    SetBoxPosition(ghAttributeBox, pPoint);

    pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
    pPoint.iY = gsAssignmentBoxesY;

    pPoint.iY += ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN);

    SetBoxPosition(ghTrainingBox, pPoint);

  } else if (fShowTrainingMenu == TRUE) {
    GetBoxSize(ghTrainingBox, &pDimensions);

    if (gsAssignmentBoxesX + pDimensions2.iRight + pDimensions.iRight >= 640) {
      gsAssignmentBoxesX = (int16_t)(639 - (pDimensions2.iRight + pDimensions.iRight));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    if (gsAssignmentBoxesY + pDimensions2.iBottom +
            ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN) >=
        360) {
      gsAssignmentBoxesY = (int16_t)(359 - (pDimensions2.iBottom) -
                                     ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    pPoint.iX = gsAssignmentBoxesX + pDimensions2.iRight;
    pPoint.iY = gsAssignmentBoxesY;
    pPoint.iY += ((GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN);

    SetBoxPosition(ghTrainingBox, pPoint);
  } else {
    // just the assignment box
    if (gsAssignmentBoxesX + pDimensions2.iRight >= 640) {
      gsAssignmentBoxesX = (int16_t)(639 - (pDimensions2.iRight));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    if (gsAssignmentBoxesY + pDimensions2.iBottom >= 360) {
      gsAssignmentBoxesY = (int16_t)(359 - (pDimensions2.iBottom));
      SetRenderFlags(RENDER_FLAG_FULL);
    }

    pPoint.iX = gsAssignmentBoxesX;
    pPoint.iY = gsAssignmentBoxesY;

    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      SetBoxPosition(ghEpcBox, pPoint);
    } else {
      SetBoxPosition(ghAssignmentBox, pPoint);
    }
  }

  RepositionMouseRegions();
}

void PositionCursorForTacticalAssignmentBox(void) {
  // position cursor over y of on duty in tactical assignments
  SGPPoint pPosition;
  SGPRect pDimensions;
  int32_t iFontHeight;

  // get x.y position of box
  GetBoxPosition(ghAssignmentBox, &pPosition);

  // get dimensions..mostly for width
  GetBoxSize(ghAssignmentBox, &pDimensions);

  iFontHeight = GetLineSpace(ghAssignmentBox) + GetFontHeight(GetBoxFont(ghAssignmentBox));

  if (gGameSettings.fOptions[TOPTION_DONT_MOVE_MOUSE] == FALSE) {
    SimulateMouseMovement(pPosition.iX + pDimensions.iRight - 6,
                          pPosition.iY + (iFontHeight / 2) + 2);
  }
}

void HandleRestFatigueAndSleepStatus(void) {
  int32_t iCounter = 0, iNumberOnTeam = 0;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fReasonAdded = FALSE;
  BOOLEAN fBoxSetUp = FALSE;
  BOOLEAN fMeToo = FALSE;

  iNumberOnTeam = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // run through all player characters and handle their rest, fatigue, and going to sleep
  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pSoldier = GetSoldierByID(iCounter);

    if (IsSolActive(pSoldier)) {
      if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
        continue;
      }

      if ((GetSolAssignment(pSoldier) == ASSIGNMENT_POW) ||
          (GetSolAssignment(pSoldier) == IN_TRANSIT)) {
        continue;
      }

      // if character CAN sleep, he doesn't actually have to be put asleep to get some rest,
      // many other assignments and conditions allow for automatic recovering from fatigue.
      if (CharacterIsTakingItEasy(pSoldier)) {
        // let them rest some
        RestCharacter(pSoldier);
      } else {
        // wear them down
        FatigueCharacter(pSoldier);
      }

      // CHECK FOR MERCS GOING TO SLEEP

      // if awake
      if (!pSoldier->fMercAsleep) {
        // if dead tired
        if (pSoldier->bBreathMax <= BREATHMAX_ABSOLUTE_MINIMUM) {
          // if between sectors, don't put tired mercs to sleep...  will be handled when they arrive
          // at the next sector
          if (pSoldier->fBetweenSectors) {
            continue;
          }

          // he goes to sleep, provided it's at all possible (it still won't happen in a hostile
          // sector, etc.)
          if (SetMercAsleep(pSoldier, FALSE)) {
            if ((pSoldier->bAssignment < ON_DUTY) || (GetSolAssignment(pSoldier) == VEHICLE)) {
              // on a squad/vehicle, complain, then drop
              TacticalCharacterDialogue(pSoldier, QUOTE_NEED_SLEEP);
              TacticalCharacterDialogueWithSpecialEvent(pSoldier, QUOTE_NEED_SLEEP,
                                                        DIALOGUE_SPECIAL_EVENT_SLEEP, 1, 0);
              fMeToo = TRUE;
            }

            // guy collapses
            pSoldier->fMercCollapsedFlag = TRUE;
          }
        }
        // if pretty tired, and not forced to stay awake
        else if ((pSoldier->bBreathMax < BREATHMAX_PRETTY_TIRED) &&
                 (pSoldier->fForcedToStayAwake == FALSE)) {
          // if not on squad/ in vehicle
          if ((pSoldier->bAssignment >= ON_DUTY) && (pSoldier->bAssignment != VEHICLE)) {
            // try to go to sleep on your own
            if (SetMercAsleep(pSoldier, FALSE)) {
              if (gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION]) {
                // if the first one
                if (fReasonAdded == FALSE) {
                  // tell player about it
                  AddReasonToWaitingListQueue(ASLEEP_GOING_AUTO_FOR_UPDATE);
                  TacticalCharacterDialogueWithSpecialEvent(
                      pSoldier, 0, DIALOGUE_SPECIAL_EVENT_SHOW_UPDATE_MENU, 0, 0);

                  fReasonAdded = TRUE;
                }

                AddSoldierToWaitingListQueue(pSoldier);
                fBoxSetUp = TRUE;
              }

              // seems unnecessary now?  ARM
              pSoldier->bOldAssignment = pSoldier->bAssignment;
            }
          } else  // tired, in a squad / vehicle
          {
            // if he hasn't complained yet
            if (!pSoldier->fComplainedThatTired) {
              // say quote
              if (fMeToo == FALSE) {
                TacticalCharacterDialogue(pSoldier, QUOTE_NEED_SLEEP);
                fMeToo = TRUE;
              } else {
                TacticalCharacterDialogue(pSoldier, QUOTE_ME_TOO);
              }

              pSoldier->fComplainedThatTired = TRUE;
            }
          }
        }
      }
    }
  }

  if (fBoxSetUp) {
    UnPauseGameDuringNextQuote();
    AddDisplayBoxToWaitingQueue();
    fBoxSetUp = FALSE;
  }

  fReasonAdded = FALSE;

  // now handle waking (needs seperate list queue, that's why it has its own loop)
  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pSoldier = GetSoldierByID(iCounter);

    if (IsSolActive(pSoldier)) {
      if ((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || AM_A_ROBOT(pSoldier)) {
        continue;
      }

      if ((GetSolAssignment(pSoldier) == ASSIGNMENT_POW) ||
          (GetSolAssignment(pSoldier) == IN_TRANSIT)) {
        continue;
      }

      // guys between sectors CAN wake up while between sectors (sleeping in vehicle)...

      // CHECK FOR MERCS WAKING UP

      if (pSoldier->bBreathMax >= BREATHMAX_CANCEL_COLLAPSE) {
        // reset the collapsed flag well before reaching the wakeup state
        pSoldier->fMercCollapsedFlag = FALSE;
      }

      // if asleep
      if (pSoldier->fMercAsleep) {
        // but has had enough rest?
        if (pSoldier->bBreathMax >= BREATHMAX_FULLY_RESTED) {
          // try to wake merc up
          if (SetMercAwake(pSoldier, FALSE, FALSE)) {
            // if not on squad/ in vehicle, tell player about it
            if ((pSoldier->bAssignment >= ON_DUTY) && (pSoldier->bAssignment != VEHICLE)) {
              if (gGameSettings.fOptions[TOPTION_SLEEPWAKE_NOTIFICATION]) {
                if (fReasonAdded == FALSE) {
                  AddReasonToWaitingListQueue(ASSIGNMENT_RETURNING_FOR_UPDATE);
                  fReasonAdded = TRUE;
                }

                AddSoldierToWaitingListQueue(pSoldier);
                fBoxSetUp = TRUE;
              }
            }
          }
        }
      }
    }
  }

  if (fBoxSetUp) {
    UnPauseGameDuringNextQuote();
    AddDisplayBoxToWaitingQueue();
    fBoxSetUp = FALSE;
  }

  return;
}

BOOLEAN CanCharacterRepairVehicle(struct SOLDIERTYPE *pSoldier, int32_t iVehicleId) {
  // is the vehicle valid?
  if (VehicleIdIsValid(iVehicleId) == FALSE) {
    return (FALSE);
  }

  // is vehicle destroyed?
  if (pVehicleList[iVehicleId].fDestroyed) {
    return (FALSE);
  }

  // is it damaged at all?
  if (!DoesVehicleNeedAnyRepairs(iVehicleId)) {
    return (FALSE);
  }

  // it's not Skyrider's helicopter (which isn't damagable/repairable)
  if (iVehicleId == iHelicopterVehicleId) {
    return (FALSE);
  }

  // same sector, neither is between sectors, and OK To Use (player owns it) ?
  if (!IsThisVehicleAccessibleToSoldier(pSoldier, iVehicleId)) {
    return (FALSE);
  }

  /* Assignment distance limits removed.  Sep/11/98.  ARM
          // if currently loaded sector, are we close enough?
          if( ( GetSolSectorX(pSoldier) == gWorldSectorX ) && ( GetSolSectorY(pSoldier) ==
     gWorldSectorY ) && ( GetSolSectorZ(pSoldier) == gbWorldSectorZ ) )
          {
                  if( PythSpacesAway( pSoldier -> sGridNo, pVehicleList[ iVehicleId ].sGridNo ) >
     MAX_DISTANCE_FOR_REPAIR )
                  {
                    return( FALSE );
                  }
          }
  */

  return (TRUE);
}

BOOLEAN IsRobotInThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ) {
  struct SOLDIERTYPE *pSoldier;

  pSoldier = GetRobotSoldier();

  if (pSoldier != NULL) {
    if ((GetSolSectorX(pSoldier) == sSectorX) && (GetSolSectorY(pSoldier) == sSectorY) &&
        (GetSolSectorZ(pSoldier) == bSectorZ) && (pSoldier->fBetweenSectors == FALSE)) {
      return (TRUE);
    }
  }

  return (FALSE);
}

struct SOLDIERTYPE *GetRobotSoldier(void) {
  struct SOLDIERTYPE *pSoldier = NULL, *pTeamSoldier = NULL;
  int32_t cnt = 0;

  // set pSoldier as first in merc ptrs
  pSoldier = MercPtrs[0];

  // go through list of characters, find all who are on this assignment
  for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID;
       cnt++, pTeamSoldier++) {
    if (pTeamSoldier->bActive) {
      if (AM_A_ROBOT(pTeamSoldier)) {
        return (pTeamSoldier);
      }
    }
  }

  return (NULL);
}

BOOLEAN CanCharacterRepairRobot(struct SOLDIERTYPE *pSoldier) {
  struct SOLDIERTYPE *pRobot = NULL;

  // do we in fact have the robot on the team?
  pRobot = GetRobotSoldier();
  if (pRobot == NULL) {
    return (FALSE);
  }

  // if robot isn't damaged at all
  if (pRobot->bLife == pRobot->bLifeMax) {
    return (FALSE);
  }

  // is the robot in the same sector
  if (IsRobotInThisSector(GetSolSectorX(pSoldier), GetSolSectorY(pSoldier),
                          GetSolSectorZ(pSoldier)) == FALSE) {
    return (FALSE);
  }

  /* Assignment distance limits removed.  Sep/11/98.  ARM
          // if that sector is currently loaded, check distance to robot
          if( ( pSoldier -> sSectorX == gWorldSectorX ) && ( pSoldier -> sSectorY == gWorldSectorY )
     && ( pSoldier -> bSectorZ == gbWorldSectorZ ) )
          {
                  if( PythSpacesAway( pSoldier -> sGridNo, pRobot -> sGridNo ) >
     MAX_DISTANCE_FOR_REPAIR )
                  {
                    return( FALSE );
                  }
          }
  */

  return (TRUE);
}

uint8_t HandleRepairOfRobotBySoldier(struct SOLDIERTYPE *pSoldier, uint8_t ubRepairPts,
                                     BOOLEAN *pfNothingLeftToRepair) {
  struct SOLDIERTYPE *pRobot = NULL;

  pRobot = GetRobotSoldier();

  // do the actual repairs
  return (RepairRobot(pRobot, ubRepairPts, pfNothingLeftToRepair));
}

uint8_t RepairRobot(struct SOLDIERTYPE *pRobot, uint8_t ubRepairPts,
                    BOOLEAN *pfNothingLeftToRepair) {
  uint8_t ubPointsUsed = 0;

  // is it "dead" ?
  if (pRobot->bLife == 0) {
    *pfNothingLeftToRepair = TRUE;
    return (ubPointsUsed);
  }

  // is it "unhurt" ?
  if (pRobot->bLife == pRobot->bLifeMax) {
    *pfNothingLeftToRepair = TRUE;
    return (ubPointsUsed);
  }

  // if we have enough or more than we need
  if (pRobot->bLife + ubRepairPts >= pRobot->bLifeMax) {
    ubPointsUsed = (pRobot->bLifeMax - pRobot->bLife);
    pRobot->bLife = pRobot->bLifeMax;
  } else {
    // not enough, do what we can
    ubPointsUsed = ubRepairPts;
    pRobot->bLife += ubRepairPts;
  }

  if (pRobot->bLife == pRobot->bLifeMax) {
    *pfNothingLeftToRepair = TRUE;
  } else {
    *pfNothingLeftToRepair = FALSE;
  }

  return (ubPointsUsed);
}

void SetSoldierAssignment(struct SOLDIERTYPE *pSoldier, int8_t bAssignment, int32_t iParam1,
                          int32_t iParam2, int32_t iParam3) {
  switch (bAssignment) {
    case (ASSIGNMENT_HOSPITAL):
      if (CanCharacterPatient(pSoldier)) {
        pSoldier->bOldAssignment = pSoldier->bAssignment;
        pSoldier->bBleeding = 0;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;

        // remove from squad

        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != ASSIGNMENT_HOSPITAL)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        RebuildCurrentSquad();

        ChangeSoldiersAssignment(pSoldier, ASSIGNMENT_HOSPITAL);

        AssignMercToAMovementGroup(pSoldier);
      }
      break;
    case (PATIENT):
      if (CanCharacterPatient(pSoldier)) {
        // set as doctor

        pSoldier->bOldAssignment = pSoldier->bAssignment;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;

        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != PATIENT)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, PATIENT);

        AssignMercToAMovementGroup(pSoldier);
      }

      break;
    case (DOCTOR):
      if (CanCharacterDoctor(pSoldier)) {
        pSoldier->bOldAssignment = pSoldier->bAssignment;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;

        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != DOCTOR)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, DOCTOR);

        MakeSureMedKitIsInHand(pSoldier);
        AssignMercToAMovementGroup(pSoldier);
      }
      break;
    case (TRAIN_TOWN):
      if (CanCharacterTrainMilitia(pSoldier)) {
        // train militia
        pSoldier->bOldAssignment = pSoldier->bAssignment;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;

        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != TRAIN_TOWN)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, TRAIN_TOWN);

        // probably this condition is not needed
        // if (pMilitiaTrainerSoldier == NULL) {
        if (!IsMilitiaTrainingPayedForSectorID8(GetSolSectorID8(pSoldier))) {
          // show a message to confirm player wants to charge cost
          HandleInterfaceMessageForCostOfTrainingMilitia(pSoldier);
        }
        // }

        AssignMercToAMovementGroup(pSoldier);
        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;
      }
      break;
    case (TRAIN_SELF):
      if (CanCharacterTrainStat(pSoldier, (int8_t)iParam1, TRUE, FALSE)) {
        // train stat
        pSoldier->bOldAssignment = pSoldier->bAssignment;

        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != TRAIN_SELF)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, TRAIN_SELF);

        AssignMercToAMovementGroup(pSoldier);

        // set stat to train
        pSoldier->bTrainStat = (int8_t)iParam1;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;
      }
      break;
    case (TRAIN_TEAMMATE):
      if (CanCharacterTrainStat(pSoldier, (int8_t)iParam1, FALSE, TRUE)) {
        pSoldier->bOldAssignment = pSoldier->bAssignment;
        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != TRAIN_TEAMMATE)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, TRAIN_TEAMMATE);
        AssignMercToAMovementGroup(pSoldier);

        // set stat to train
        pSoldier->bTrainStat = (int8_t)iParam1;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;
      }
      break;
    case (TRAIN_BY_OTHER):
      if (CanCharacterTrainStat(pSoldier, (int8_t)iParam1, TRUE, FALSE)) {
        // train stat
        pSoldier->bOldAssignment = pSoldier->bAssignment;

        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != TRAIN_BY_OTHER)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, TRAIN_BY_OTHER);

        AssignMercToAMovementGroup(pSoldier);

        // set stat to train
        pSoldier->bTrainStat = (int8_t)iParam1;

        // set dirty flag
        fTeamPanelDirty = TRUE;
        fMapScreenBottomDirty = TRUE;
        gfRenderPBInterface = TRUE;
      }
      break;
    case (REPAIR):
      if (CanCharacterRepair(pSoldier)) {
        pSoldier->bOldAssignment = pSoldier->bAssignment;

        // remove from squad
        RemoveCharacterFromSquads(pSoldier);

        // remove from any vehicle
        if (pSoldier->bOldAssignment == VEHICLE) {
          TakeSoldierOutOfVehicle(pSoldier);
        }

        if ((pSoldier->bAssignment != REPAIR) || (pSoldier->fFixingSAMSite != (uint8_t)iParam1) ||
            (pSoldier->fFixingRobot != (uint8_t)iParam2) ||
            (pSoldier->bVehicleUnderRepairID != (uint8_t)iParam3)) {
          SetTimeOfAssignmentChangeForMerc(pSoldier);
        }

        ChangeSoldiersAssignment(pSoldier, REPAIR);
        MakeSureToolKitIsInHand(pSoldier);
        AssignMercToAMovementGroup(pSoldier);
        pSoldier->fFixingSAMSite = (uint8_t)iParam1;
        pSoldier->fFixingRobot = (uint8_t)iParam2;
        pSoldier->bVehicleUnderRepairID = (int8_t)iParam3;
      }
      break;
    case (VEHICLE):
      if (CanCharacterVehicle(pSoldier) && IsThisVehicleAccessibleToSoldier(pSoldier, iParam1)) {
        if (IsEnoughSpaceInVehicle((int8_t)iParam1)) {
          pSoldier->bOldAssignment = pSoldier->bAssignment;

          // set dirty flag
          fTeamPanelDirty = TRUE;
          fMapScreenBottomDirty = TRUE;
          gfRenderPBInterface = TRUE;

          if (pSoldier->bOldAssignment == VEHICLE) {
            TakeSoldierOutOfVehicle(pSoldier);
          }

          // remove from squad
          RemoveCharacterFromSquads(pSoldier);

          if (PutSoldierInVehicle(pSoldier, (int8_t)(iParam1)) == FALSE) {
            AddCharacterToAnySquad(pSoldier);
          } else {
            if ((pSoldier->bAssignment != VEHICLE) || (pSoldier->iVehicleId != (uint8_t)iParam1)) {
              SetTimeOfAssignmentChangeForMerc(pSoldier);
            }

            pSoldier->iVehicleId = iParam1;
            ChangeSoldiersAssignment(pSoldier, VEHICLE);
            AssignMercToAMovementGroup(pSoldier);
          }
        } else {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[18],
                    zVehicleName[pVehicleList[iParam1].ubVehicleType]);
        }
      }
      break;
  }

  return;
}

/* No point in allowing SAM site repair any more.  Jan/13/99.  ARM
BOOLEAN CanSoldierRepairSAM( struct SOLDIERTYPE *pSoldier, int8_t bRepairPoints)
{
        int16_t sGridNoA = 0, sGridNoB = 0;

        // is the soldier in the sector as the SAM
        if( SoldierInSameSectorAsSAM( pSoldier ) == FALSE )
        {
                return( FALSE );
        }

        // is the soldier close enough to the control panel?
        if( IsSoldierCloseEnoughToSAMControlPanel( pSoldier ) == FALSE )
        {
                return( FALSE );
        }

        //can it be fixed?
        if( IsTheSAMSiteInSectorRepairable( pSoldier -> sSectorX, pSoldier -> sSectorY, pSoldier ->
bSectorZ ) == FALSE )
        {
                return( FALSE );
        }

        // is he good enough?  (Because of the division of repair pts in SAM repair formula, a guy
with any less that this
        // can't make any headway
        if (bRepairPoints < SAM_SITE_REPAIR_DIVISOR )
        {
                return( FALSE );
        }

        return( TRUE );
}

BOOLEAN IsTheSAMSiteInSectorRepairable( uint8_t sSectorX, uint8_t sSectorY, int16_t sSectorZ )
{
        int32_t iCounter = 0;
        int8_t bSAMCondition;


        // is the guy above ground, if not, it can't be fixed, now can it?
        if( sSectorZ != 0 )
        {
                return( FALSE );
        }

        for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
        {
                if( pSamList[ iCounter ] == GetSectorID8( sSectorX, sSectorY ) )
                {
                        bSAMCondition = StrategicMap[ GetSectorID16( sSectorX, sSectorY
) ].bSAMCondition;

                        if( ( bSAMCondition < 100 ) && ( bSAMCondition >= MIN_CONDITION_TO_FIX_SAM )
)
                        {
                                return( TRUE );
                        }
                        else
                        {
                                // it's not broken at all, or it's beyond repair
                                return( FALSE );
                        }
                }
        }

        // none found
        return( FALSE );
}

BOOLEAN SoldierInSameSectorAsSAM( struct SOLDIERTYPE *pSoldier )
{
        int32_t iCounter = 0;

        // is the soldier on the surface?
        if( pSoldier -> bSectorZ != 0 )
        {
                return( FALSE );
        }

        // now check each sam site in the list
        for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
        {
                if( pSamList[ iCounter] == GetSectorID8( pSoldier -> sSectorX, pSoldier -> sSectorY
) )
                {
                        return( TRUE );
                }
        }

        return( FALSE );
}

BOOLEAN IsSoldierCloseEnoughToSAMControlPanel( struct SOLDIERTYPE *pSoldier )
{

        int32_t iCounter = 0;

                // now check each sam site in the list
        for( iCounter = 0; iCounter < NUMBER_OF_SAMS; iCounter++ )
        {
                if( pSamList[ iCounter ] == GetSectorID8( pSoldier -> sSectorX, pSoldier -> sSectorY
) )
                {
// Assignment distance limits removed.  Sep/11/98.  ARM
//			if( ( PythSpacesAway( pSamGridNoAList[ iCounter ], pSoldier -> sGridNo ) <
MAX_DISTANCE_FOR_REPAIR )||( PythSpacesAway( pSamGridNoBList[ iCounter ], pSoldier -> sGridNo ) <
MAX_DISTANCE_FOR_REPAIR ) )
                        {
                                return( TRUE );
                        }
                }
        }

        return( FALSE );
}
*/

BOOLEAN HandleAssignmentExpansionAndHighLightForAssignMenu(struct SOLDIERTYPE *pSoldier) {
  if (fShowSquadMenu) {
    // squad menu up?..if so, highlight squad line the previous menu
    if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) {
      HighLightBoxLine(ghEpcBox, EPC_MENU_ON_DUTY);
    } else {
      HighLightBoxLine(ghAssignmentBox, ASSIGN_MENU_ON_DUTY);
    }

    return (TRUE);
  } else if (fShowTrainingMenu) {
    // highlight train line the previous menu
    HighLightBoxLine(ghAssignmentBox, ASSIGN_MENU_TRAIN);
    return (TRUE);
  } else if (fShowRepairMenu) {
    // highlight repair line the previous menu
    HighLightBoxLine(ghAssignmentBox, ASSIGN_MENU_REPAIR);
    return (TRUE);
  } else if (fShowVehicleMenu) {
    // highlight vehicle line the previous menu
    HighLightBoxLine(ghAssignmentBox, ASSIGN_MENU_VEHICLE);
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN HandleAssignmentExpansionAndHighLightForTrainingMenu(void) {
  if (fShowAttributeMenu) {
    switch (gbTrainingMode) {
      case TRAIN_SELF:
        HighLightBoxLine(ghTrainingBox, TRAIN_MENU_SELF);
        return (TRUE);
      case TRAIN_TEAMMATE:
        HighLightBoxLine(ghTrainingBox, TRAIN_MENU_TEAMMATES);
        return (TRUE);
      case TRAIN_BY_OTHER:
        HighLightBoxLine(ghTrainingBox, TRAIN_MENU_TRAIN_BY_OTHER);
        return (TRUE);

      default:
        Assert(FALSE);
        break;
    }
  }

  return (FALSE);
}

/*
BOOLEAN HandleShowingOfUpBox( void )
{

        // if the list is being shown, then show it
        if( fShowUpdateBox == TRUE )
        {
                MarkAllBoxesAsAltered( );
                ShowBox( ghUpdateBox );
                return( TRUE );
        }
        else
        {
                if( IsBoxShown( ghUpdateBox ) )
                {
                        HideBox( ghUpdateBox );
                        MarkForRedrawalStrategicMap();
                        gfRenderPBInterface = TRUE;
                        fTeamPanelDirty = TRUE;
                        fMapScreenBottomDirty = TRUE;
                        fCharacterInfoPanelDirty = TRUE;
                }
        }

        return( FALSE );
}
*/

BOOLEAN HandleShowingOfMovementBox(void) {
  // if the list is being shown, then show it
  if (fShowMapScreenMovementList == TRUE) {
    MarkAllBoxesAsAltered();
    ShowBox(ghMoveBox);
    return (TRUE);
  } else {
    if (IsBoxShown(ghMoveBox)) {
      HideBox(ghMoveBox);
      MarkForRedrawalStrategicMap();
      gfRenderPBInterface = TRUE;
      fTeamPanelDirty = TRUE;
      fMapScreenBottomDirty = TRUE;
      fCharacterInfoPanelDirty = TRUE;
    }
  }

  return (FALSE);
}

void HandleShadingOfLinesForTrainingMenu(void) {
  struct SOLDIERTYPE *pSoldier = NULL;

  // check if valid
  if ((fShowTrainingMenu == FALSE) || (ghTrainingBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(FALSE);

  // can character practise?
  if (CanCharacterPractise(pSoldier) == FALSE) {
    ShadeStringInBox(ghTrainingBox, TRAIN_MENU_SELF);
  } else {
    UnShadeStringInBox(ghTrainingBox, TRAIN_MENU_SELF);
  }

  // can character EVER train militia?
  if (BasicCanCharacterTrainMilitia(pSoldier)) {
    // can he train here, now?
    if (CanCharacterTrainMilitia(pSoldier)) {
      // unshade train militia line
      UnShadeStringInBox(ghTrainingBox, TRAIN_MENU_TOWN);
      UnSecondaryShadeStringInBox(ghTrainingBox, TRAIN_MENU_TOWN);
    } else {
      SecondaryShadeStringInBox(ghTrainingBox, TRAIN_MENU_TOWN);
    }
  } else {
    // shade train militia line
    ShadeStringInBox(ghTrainingBox, TRAIN_MENU_TOWN);
  }

  // can character train teammates?
  if (CanCharacterTrainTeammates(pSoldier) == FALSE) {
    ShadeStringInBox(ghTrainingBox, TRAIN_MENU_TEAMMATES);
  } else {
    UnShadeStringInBox(ghTrainingBox, TRAIN_MENU_TEAMMATES);
  }

  // can character be trained by others?
  if (CanCharacterBeTrainedByOther(pSoldier) == FALSE) {
    ShadeStringInBox(ghTrainingBox, TRAIN_MENU_TRAIN_BY_OTHER);
  } else {
    UnShadeStringInBox(ghTrainingBox, TRAIN_MENU_TRAIN_BY_OTHER);
  }

  return;
}

void HandleShadingOfLinesForAttributeMenus(void) {
  // will do the same as updateassignments...but with training pop up box strings
  struct SOLDIERTYPE *pSoldier;
  int8_t bAttrib = 0;
  BOOLEAN fStatTrainable;

  if ((fShowTrainingMenu == FALSE) || (ghTrainingBox == -1)) {
    return;
  }

  if ((fShowAttributeMenu == FALSE) || (ghAttributeBox == -1)) {
    return;
  }

  pSoldier = GetSelectedAssignSoldier(FALSE);

  for (bAttrib = 0; bAttrib < ATTRIB_MENU_CANCEL; bAttrib++) {
    switch (gbTrainingMode) {
      case TRAIN_SELF:
        fStatTrainable = CanCharacterTrainStat(pSoldier, bAttrib, TRUE, FALSE);
        break;
      case TRAIN_TEAMMATE:
        // DO allow trainers to be assigned without any partners (students)
        fStatTrainable = CanCharacterTrainStat(pSoldier, bAttrib, FALSE, TRUE);
        break;
      case TRAIN_BY_OTHER:
        // DO allow students to be assigned without any partners (trainers)
        fStatTrainable = CanCharacterTrainStat(pSoldier, bAttrib, TRUE, FALSE);
        break;
      default:
        Assert(FALSE);
        fStatTrainable = FALSE;
        break;
    }

    if (fStatTrainable) {
      // also unshade stat
      UnShadeStringInBox(ghAttributeBox, bAttrib);
    } else {
      // shade stat
      ShadeStringInBox(ghAttributeBox, bAttrib);
    }
  }

  return;
}

void ResetAssignmentsForAllSoldiersInSectorWhoAreTrainingTown(struct SOLDIERTYPE *pSoldier) {
  int32_t iNumberOnTeam = 0, iCounter = 0;
  struct SOLDIERTYPE *pCurSoldier = NULL;

  iNumberOnTeam = gTacticalStatus.Team[OUR_TEAM].bLastID;

  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pCurSoldier = GetSoldierByID(iCounter);

    if ((pCurSoldier->bActive) && (pCurSoldier->bLife >= OKLIFE)) {
      if (pCurSoldier->bAssignment == TRAIN_TOWN) {
        if ((pCurSoldier->sSectorX == GetSolSectorX(pSoldier)) &&
            (pCurSoldier->sSectorY == GetSolSectorY(pSoldier)) && (GetSolSectorZ(pSoldier) == 0)) {
          AddCharacterToAnySquad(pCurSoldier);
        }
      }
    }
  }

  return;
}

void ReportTrainersTraineesWithoutPartners(void) {
  struct SOLDIERTYPE *pTeamSoldier = NULL;
  int32_t iCounter = 0, iNumberOnTeam = 0;

  iNumberOnTeam = gTacticalStatus.Team[OUR_TEAM].bLastID;

  // check for each instructor
  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pTeamSoldier = GetSoldierByID(iCounter);

    if ((pTeamSoldier->bAssignment == TRAIN_TEAMMATE) && (pTeamSoldier->bLife > 0)) {
      if (!ValidTrainingPartnerInSameSectorOnAssignmentFound(pTeamSoldier, TRAIN_BY_OTHER,
                                                             pTeamSoldier->bTrainStat)) {
        AssignmentDone(pTeamSoldier, TRUE, TRUE);
      }
    }
  }

  // check each trainee
  for (iCounter = 0; iCounter < iNumberOnTeam; iCounter++) {
    pTeamSoldier = GetSoldierByID(iCounter);

    if ((pTeamSoldier->bAssignment == TRAIN_BY_OTHER) && (pTeamSoldier->bLife > 0)) {
      if (!ValidTrainingPartnerInSameSectorOnAssignmentFound(pTeamSoldier, TRAIN_TEAMMATE,
                                                             pTeamSoldier->bTrainStat)) {
        AssignmentDone(pTeamSoldier, TRUE, TRUE);
      }
    }
  }

  return;
}

BOOLEAN SetMercAsleep(struct SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning) {
  if (CanCharacterSleep(pSoldier, fGiveWarning)) {
    // put him to sleep
    PutMercInAsleepState(pSoldier);

    // successful
    return (TRUE);
  } else {
    // can't sleep for some other reason
    return (FALSE);
  }
}

BOOLEAN PutMercInAsleepState(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->fMercAsleep == FALSE) {
    if ((gfWorldLoaded) && IsSolInSector(pSoldier)) {
      if (IsTacticalMode()) {
        ChangeSoldierState(pSoldier, GOTO_SLEEP, 1, TRUE);
      } else {
        ChangeSoldierState(pSoldier, SLEEPING, 1, TRUE);
      }
    }

    // set merc asleep
    pSoldier->fMercAsleep = TRUE;

    // refresh panels
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;
  }

  return (TRUE);
}

BOOLEAN SetMercAwake(struct SOLDIERTYPE *pSoldier, BOOLEAN fGiveWarning, BOOLEAN fForceHim) {
  // forcing him skips all normal checks!
  if (!fForceHim) {
    if (!CanCharacterBeAwakened(pSoldier, fGiveWarning)) {
      return (FALSE);
    }
  }

  PutMercInAwakeState(pSoldier);
  return (TRUE);
}

BOOLEAN PutMercInAwakeState(struct SOLDIERTYPE *pSoldier) {
  if (pSoldier->fMercAsleep) {
    if ((gfWorldLoaded) && IsSolInSector(pSoldier)) {
      if (IsTacticalMode()) {
        ChangeSoldierState(pSoldier, WKAEUP_FROM_SLEEP, 1, TRUE);
      } else {
        ChangeSoldierState(pSoldier, STANDING, 1, TRUE);
      }
    }

    // set merc awake
    pSoldier->fMercAsleep = FALSE;

    // refresh panels
    fCharacterInfoPanelDirty = TRUE;
    fTeamPanelDirty = TRUE;

    // determine if merc is being forced to stay awake
    if (pSoldier->bBreathMax < BREATHMAX_PRETTY_TIRED) {
      pSoldier->fForcedToStayAwake = TRUE;
    } else {
      pSoldier->fForcedToStayAwake = FALSE;
    }
  }

  return (TRUE);
}

BOOLEAN IsThereASoldierInThisSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ) {
  if (fSectorsWithSoldiers[GetSectorID16(sSectorX, sSectorY)][bSectorZ] == TRUE) {
    return (TRUE);
  }

  return (FALSE);
}

// set the time this soldier's assignment changed
void SetTimeOfAssignmentChangeForMerc(struct SOLDIERTYPE *pSoldier) {
  // if someone is being taken off of HOSPITAL then track how much
  // of payment wasn't used up
  if (GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL) {
    giHospitalRefund += CalcPatientMedicalCost(pSoldier);
    pSoldier->bHospitalPriceModifier = 0;
  }

  // set time of last assignment change
  pSoldier->uiLastAssignmentChangeMin = GetWorldTotalMin();

  // assigning new PATIENTs gives a DOCTOR something to do, etc., so set flag to recheck them all.
  // CAN'T DO IT RIGHT AWAY IN HERE 'CAUSE WE TYPICALLY GET CALLED *BEFORE* bAssignment GETS SET TO
  // NEW VALUE!!
  gfReEvaluateEveryonesNothingToDo = TRUE;

  return;
}

// have we spent enough time on assignment for it to count?
BOOLEAN EnoughTimeOnAssignment(struct SOLDIERTYPE *pSoldier) {
  if (GetWorldTotalMin() - pSoldier->uiLastAssignmentChangeMin >= MINUTES_FOR_ASSIGNMENT_TO_COUNT) {
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN AnyMercInGroupCantContinueMoving(struct GROUP *pGroup) {
  PLAYERGROUP *pPlayer;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fMeToo = FALSE;
  BOOLEAN fGroupMustStop = FALSE;

  Assert(pGroup);
  Assert(pGroup->fPlayer);

  pPlayer = pGroup->pPlayerList;

  while (pPlayer) {
    // if group has player list...  and a valid first soldier
    if (pPlayer && pPlayer->pSoldier) {
      pSoldier = pPlayer->pSoldier;

      if (PlayerSoldierTooTiredToTravel(pSoldier)) {
        // NOTE: we only complain about it if it's gonna force the group to stop moving!
        fGroupMustStop = TRUE;

        // say quote
        if (fMeToo == FALSE) {
          HandleImportantMercQuote(pSoldier, QUOTE_NEED_SLEEP);
          fMeToo = TRUE;
        } else {
          HandleImportantMercQuote(pSoldier, QUOTE_ME_TOO);
        }

        // put him to bed
        PutMercInAsleepState(pSoldier);

        // player can't wake him up right away
        pSoldier->fMercCollapsedFlag = TRUE;
      }
    }

    pPlayer = pPlayer->next;
  }

  return (fGroupMustStop);
}

BOOLEAN PlayerSoldierTooTiredToTravel(struct SOLDIERTYPE *pSoldier) {
  Assert(pSoldier);

  // if this guy ever needs sleep at all
  if (CanChangeSleepStatusForSoldier(pSoldier)) {
    // if walking, or only remaining possible driver for a vehicle group
    if ((pSoldier->bAssignment != VEHICLE) ||
        SoldierMustDriveVehicle(pSoldier, pSoldier->iVehicleId, TRUE)) {
      // if awake, but so tired they can't move/drive anymore
      if ((!pSoldier->fMercAsleep) && (pSoldier->bBreathMax < BREATHMAX_GOTTA_STOP_MOVING)) {
        return (TRUE);
      }

      // asleep, and can't be awakened?
      if ((pSoldier->fMercAsleep) && !CanCharacterBeAwakened(pSoldier, FALSE)) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

BOOLEAN AssignMercToAMovementGroup(struct SOLDIERTYPE *pSoldier) {
  // if merc doesn't have a group or is in a vehicle or on a squad assign to group
  int8_t bGroupId = 0;

  // on a squad?
  if (pSoldier->bAssignment < ON_DUTY) {
    return (FALSE);
  }

  // in a vehicle?
  if (GetSolAssignment(pSoldier) == VEHICLE) {
    return (FALSE);
  }

  // in transit
  if (GetSolAssignment(pSoldier) == IN_TRANSIT) {
    return (FALSE);
  }

  // in a movement group?
  if (pSoldier->ubGroupID != 0) {
    return (FALSE);
  }

  // create group
  bGroupId = CreateNewPlayerGroupDepartingFromSector((uint8_t)(GetSolSectorX(pSoldier)),
                                                     (uint8_t)(GetSolSectorY(pSoldier)));

  if (bGroupId) {
    // add merc
    AddPlayerToGroup(bGroupId, pSoldier);

    // success
    return (TRUE);
  }

  return (TRUE);
}

void NotifyPlayerOfAssignmentAttemptFailure(int8_t bAssignment) {
  // notify player
  if (guiCurrentScreen != MSG_BOX_SCREEN) {
    DoScreenIndependantMessageBox(pMapErrorString[18], MSG_BOX_FLAG_OK, NULL);
  } else {
    // use screen msg instead!
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMapErrorString[18]);
  }

  if (bAssignment == TRAIN_TOWN) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMapErrorString[48]);
  }
}

BOOLEAN HandleSelectedMercsBeingPutAsleep(BOOLEAN fWakeUp, BOOLEAN fDisplayWarning) {
  BOOLEAN fSuccess = TRUE;
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  uint8_t ubNumberOfSelectedSoldiers = 0;
  wchar_t sString[128];

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    pSoldier = NULL;

    // if the current character in the list is valid...then grab soldier pointer for the character
    if (IsCharListEntryValid(iCounter)) {
      // get the soldier pointer
      pSoldier = GetMercFromCharacterList(iCounter);

      if (IsSolActive(pSoldier) == FALSE) {
        continue;
      }

      if (iCounter == bSelectedInfoChar) {
        continue;
      }

      if (IsEntryInSelectedListSet((int8_t)iCounter) == FALSE) {
        continue;
      }

      // don't try to put vehicles, robots, to sleep if they're also selected
      if (CanChangeSleepStatusForCharSlot((int8_t)iCounter) == FALSE) {
        continue;
      }

      // up the total number of soldiers
      ubNumberOfSelectedSoldiers++;

      if (fWakeUp) {
        // try to wake merc up
        if (SetMercAwake(pSoldier, FALSE, FALSE) == FALSE) {
          fSuccess = FALSE;
        }
      } else {
        // set this soldier asleep
        if (SetMercAsleep(pSoldier, FALSE) == FALSE) {
          fSuccess = FALSE;
        }
      }
    }
  }

  // if there was anyone processed, check for success and inform player of failure
  if (ubNumberOfSelectedSoldiers) {
    if (fSuccess == FALSE) {
      if (fWakeUp) {
        // inform player not everyone could be woke up
        swprintf(sString, ARR_SIZE(sString), pMapErrorString[27]);
      } else {
        // inform player not everyone could be put to sleep
        swprintf(sString, ARR_SIZE(sString), pMapErrorString[26]);
      }

      if (fDisplayWarning) {
        DoScreenIndependantMessageBox(sString, MSG_BOX_FLAG_OK, NULL);
      }
    }
  }

  return (fSuccess);
}

BOOLEAN IsAnyOneOnPlayersTeamOnThisAssignment(int8_t bAssignment) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;

  for (iCounter = gTacticalStatus.Team[OUR_TEAM].bFirstID;
       iCounter <= gTacticalStatus.Team[OUR_TEAM].bLastID; iCounter++) {
    // get the current soldier
    pSoldier = GetSoldierByID(iCounter);

    // active?
    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    if (GetSolAssignment(pSoldier) == bAssignment) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void RebuildAssignmentsBox(void) {
  // destroy and recreate assignments box
  if (ghAssignmentBox != -1) {
    RemoveBox(ghAssignmentBox);
    ghAssignmentBox = -1;
  }

  CreateAssignmentsBox();
}

void BandageBleedingDyingPatientsBeingTreated() {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  struct SOLDIERTYPE *pDoctor = NULL;
  int32_t iKitSlot;
  struct OBJECTTYPE *pKit = NULL;
  uint16_t usKitPts;
  uint32_t uiKitPtsUsed;
  BOOLEAN fSomeoneStillBleedingDying = FALSE;

  for (iCounter = gTacticalStatus.Team[OUR_TEAM].bFirstID;
       iCounter <= gTacticalStatus.Team[OUR_TEAM].bLastID; iCounter++) {
    // get the soldier
    pSoldier = GetSoldierByID(iCounter);

    // check if the soldier is currently active?
    if (IsSolActive(pSoldier) == FALSE) {
      continue;
    }

    // and he is bleeding or dying
    if ((pSoldier->bBleeding) || (pSoldier->bLife < OKLIFE)) {
      // if soldier is receiving care
      if ((GetSolAssignment(pSoldier) == PATIENT) ||
          (GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL) ||
          (GetSolAssignment(pSoldier) == DOCTOR)) {
        // if in the hospital
        if (GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL) {
          // this is instantaneous, and doesn't use up any bandages!

          // stop bleeding automatically
          pSoldier->bBleeding = 0;

          if (pSoldier->bLife < OKLIFE) {
            pSoldier->bLife = OKLIFE;
          }
        } else  // assigned to DOCTOR/PATIENT
        {
          // see if there's a doctor around who can help him
          pDoctor = AnyDoctorWhoCanHealThisPatient(pSoldier, HEALABLE_EVER);
          if (pDoctor != NULL) {
            iKitSlot = FindObjClass(pDoctor, IC_MEDKIT);
            if (iKitSlot != NO_SLOT) {
              pKit = &(pDoctor->inv[iKitSlot]);

              usKitPts = TotalPoints(pKit);
              if (usKitPts) {
                uiKitPtsUsed =
                    VirtualSoldierDressWound(pDoctor, pSoldier, pKit, usKitPts, usKitPts);
                UseKitPoints(pKit, (uint16_t)uiKitPtsUsed, pDoctor);

                // if he is STILL bleeding or dying
                if ((pSoldier->bBleeding) || (pSoldier->bLife < OKLIFE)) {
                  fSomeoneStillBleedingDying = TRUE;
                }
              }
            }
          }
        }
      }
    }
  }

  // this event may be posted many times because of multiple assignment changes.  Handle it only
  // once per minute!
  DeleteAllStrategicEventsOfType(EVENT_BANDAGE_BLEEDING_MERCS);

  if (fSomeoneStillBleedingDying) {
    AddStrategicEvent(EVENT_BANDAGE_BLEEDING_MERCS, GetWorldTotalMin() + 1, 0);
  }
}

void ReEvaluateEveryonesNothingToDo() {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  BOOLEAN fNothingToDo;

  for (iCounter = 0; iCounter <= gTacticalStatus.Team[OUR_TEAM].bLastID; iCounter++) {
    pSoldier = GetSoldierByID(iCounter);

    if (IsSolActive(pSoldier)) {
      switch (pSoldier->bAssignment) {
        case DOCTOR:
          fNothingToDo = !CanCharacterDoctor(pSoldier) ||
                         (GetNumberThatCanBeDoctored(pSoldier, HEALABLE_EVER, FALSE, FALSE) == 0);
          break;

        case PATIENT:
          fNothingToDo = !CanCharacterPatient(pSoldier) ||
                         (AnyDoctorWhoCanHealThisPatient(pSoldier, HEALABLE_EVER) == NULL);
          break;

        case ASSIGNMENT_HOSPITAL:
          fNothingToDo = !CanCharacterPatient(pSoldier);
          break;

        case REPAIR:
          fNothingToDo = !CanCharacterRepair(pSoldier) || HasCharacterFinishedRepairing(pSoldier);
          break;

        case TRAIN_TOWN:
          fNothingToDo = !CanCharacterTrainMilitia(pSoldier);
          break;

        case TRAIN_SELF:
          fNothingToDo = !CanCharacterTrainStat(pSoldier, pSoldier->bTrainStat, TRUE, FALSE);
          break;

        case TRAIN_TEAMMATE:
          fNothingToDo = !CanCharacterTrainStat(pSoldier, pSoldier->bTrainStat, FALSE, TRUE) ||
                         !ValidTrainingPartnerInSameSectorOnAssignmentFound(
                             pSoldier, TRAIN_BY_OTHER, pSoldier->bTrainStat);
          break;

        case TRAIN_BY_OTHER:
          fNothingToDo = !CanCharacterTrainStat(pSoldier, pSoldier->bTrainStat, TRUE, FALSE) ||
                         !ValidTrainingPartnerInSameSectorOnAssignmentFound(
                             pSoldier, TRAIN_TEAMMATE, pSoldier->bTrainStat);
          break;

        case VEHICLE:
        default:  // squads
          fNothingToDo = FALSE;
          break;
      }

      // if his flag is wrong
      if (fNothingToDo != pSoldier->fDoneAssignmentAndNothingToDoFlag) {
        // update it!
        pSoldier->fDoneAssignmentAndNothingToDoFlag = fNothingToDo;

        // update mapscreen's character list display
        fDrawCharacterList = TRUE;
      }

      // if he now has something to do, reset the quote flag
      if (!fNothingToDo) {
        pSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;
      }
    }
  }

  // re-evaluation completed
  gfReEvaluateEveryonesNothingToDo = FALSE;

  // redraw the map, in case we're showing teams, and someone just came on duty or off duty, their
  // icon needs updating
  MarkForRedrawalStrategicMap();
}

void SetAssignmentForList(int8_t bAssignment, int8_t bParam) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSelectedSoldier = NULL;
  struct SOLDIERTYPE *pSoldier = NULL;
  BOOLEAN fItWorked;
  BOOLEAN fRemoveFromSquad = TRUE;
  BOOLEAN fNotifiedOfFailure = FALSE;
  int8_t bCanJoinSquad;

  // if not in mapscreen, there is no functionality available to change multiple assignments
  // simultaneously!
  if (!(IsMapScreen())) {
    return;
  }

  // pSelectedSoldier is currently used only for REPAIR, and this block of code is copied from
  // RepairMenuBtnCallback()
  if (bSelectedAssignChar != -1) {
    if (gCharactersList[bSelectedAssignChar].fValid == TRUE) {
      pSelectedSoldier = GetSoldierByID(gCharactersList[bSelectedAssignChar].usSolID);
    }
  }

  Assert(pSelectedSoldier && pSelectedSoldier->bActive);

  // sets assignment for the list
  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    if ((IsCharListEntryValid(iCounter)) && IsCharSelected(iCounter) &&
        (iCounter != bSelectedAssignChar) &&
        !(Menptr[gCharactersList[iCounter].usSolID].uiStatusFlags & SOLDIER_VEHICLE)) {
      pSoldier = MercPtrs[gCharactersList[iCounter].usSolID];

      // assume it's NOT gonna work
      fItWorked = FALSE;

      switch (bAssignment) {
        case (DOCTOR):
          // can character doctor?
          if (CanCharacterDoctor(pSoldier)) {
            // set as doctor
            pSoldier->bOldAssignment = pSoldier->bAssignment;
            SetSoldierAssignment(pSoldier, DOCTOR, 0, 0, 0);
            fItWorked = TRUE;
          }
          break;
        case (PATIENT):
          // can character patient?
          if (CanCharacterPatient(pSoldier)) {
            // set as patient
            pSoldier->bOldAssignment = pSoldier->bAssignment;
            SetSoldierAssignment(pSoldier, PATIENT, 0, 0, 0);
            fItWorked = TRUE;
          }
          break;
        case (VEHICLE):
          if (CanCharacterVehicle(pSoldier) && IsThisVehicleAccessibleToSoldier(pSoldier, bParam)) {
            //						if ( IsEnoughSpaceInVehicle( bParam ) )
            {
              // if the vehicle is FULL, then this will return FALSE!
              fItWorked = PutSoldierInVehicle(pSoldier, bParam);
              // failure produces its own error popup
              fNotifiedOfFailure = TRUE;
            }
          }
          break;
        case (REPAIR):
          if (CanCharacterRepair(pSoldier)) {
            BOOLEAN fCanFixSpecificTarget = TRUE;

            // make sure he can repair the SPECIFIC thing being repaired too (must be in its sector,
            // for example)

            /*
                                                            if ( pSelectedSoldier->fFixingSAMSite )
                                                            {
                                                                    fCanFixSpecificTarget =
               CanSoldierRepairSAM( pSoldier, SAM_SITE_REPAIR_DIVISOR );
                                                            }
                                                            else
            */
            if (pSelectedSoldier->bVehicleUnderRepairID != -1) {
              fCanFixSpecificTarget =
                  CanCharacterRepairVehicle(pSoldier, pSelectedSoldier->bVehicleUnderRepairID);
            } else if (pSoldier->fFixingRobot) {
              fCanFixSpecificTarget = CanCharacterRepairRobot(pSoldier);
            }

            if (fCanFixSpecificTarget) {
              // set as repair
              pSoldier->bOldAssignment = pSoldier->bAssignment;
              SetSoldierAssignment(pSoldier, REPAIR, pSelectedSoldier->fFixingSAMSite,
                                   pSelectedSoldier->fFixingRobot,
                                   pSelectedSoldier->bVehicleUnderRepairID);
              fItWorked = TRUE;
            }
          }
          break;
        case (TRAIN_SELF):
          if (CanCharacterTrainStat(pSoldier, bParam, TRUE, FALSE)) {
            pSoldier->bOldAssignment = pSoldier->bAssignment;
            SetSoldierAssignment(pSoldier, TRAIN_SELF, bParam, 0, 0);
            fItWorked = TRUE;
          }
          break;
        case (TRAIN_TOWN):
          if (CanCharacterTrainMilitia(pSoldier)) {
            pSoldier->bOldAssignment = pSoldier->bAssignment;
            SetSoldierAssignment(pSoldier, TRAIN_TOWN, 0, 0, 0);
            fItWorked = TRUE;
          }
          break;
        case (TRAIN_TEAMMATE):
          if (CanCharacterTrainStat(pSoldier, bParam, FALSE, TRUE)) {
            pSoldier->bOldAssignment = pSoldier->bAssignment;
            SetSoldierAssignment(pSoldier, TRAIN_TEAMMATE, bParam, 0, 0);
            fItWorked = TRUE;
          }
          break;
        case TRAIN_BY_OTHER:
          if (CanCharacterTrainStat(pSoldier, bParam, TRUE, FALSE)) {
            pSoldier->bOldAssignment = pSoldier->bAssignment;
            SetSoldierAssignment(pSoldier, TRAIN_BY_OTHER, bParam, 0, 0);
            fItWorked = TRUE;
          }
          break;

        case (SQUAD_1):
        case (SQUAD_2):
        case (SQUAD_3):
        case (SQUAD_4):
        case (SQUAD_5):
        case (SQUAD_6):
        case (SQUAD_7):
        case (SQUAD_8):
        case (SQUAD_9):
        case (SQUAD_10):
        case (SQUAD_11):
        case (SQUAD_12):
        case (SQUAD_13):
        case (SQUAD_14):
        case (SQUAD_15):
        case (SQUAD_16):
        case (SQUAD_17):
        case (SQUAD_18):
        case (SQUAD_19):
        case (SQUAD_20):
          bCanJoinSquad = CanCharacterSquad(pSoldier, (int8_t)bAssignment);

          // if already in it, don't repor that as an error...
          if ((bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD) ||
              (bCanJoinSquad == CHARACTER_CANT_JOIN_SQUAD_ALREADY_IN_IT)) {
            if (bCanJoinSquad == CHARACTER_CAN_JOIN_SQUAD) {
              pSoldier->bOldAssignment = pSoldier->bAssignment;

              // is the squad between sectors
              if (Squad[bAssignment][0]) {
                if (Squad[bAssignment][0]->fBetweenSectors) {
                  // between sectors, remove from old mvt group
                  if (pSoldier->bOldAssignment >= ON_DUTY) {
                    // remove from group
                    // the guy wasn't in a sqaud, but moving through a sector?
                    if (pSoldier->ubGroupID != 0) {
                      // now remove from mvt group
                      RemovePlayerFromGroup(pSoldier->ubGroupID, pSoldier);
                    }
                  }
                }
              }

              if (pSoldier->bOldAssignment == VEHICLE) {
                TakeSoldierOutOfVehicle(pSoldier);
              }
              // remove from current squad, if any
              RemoveCharacterFromSquads(pSoldier);

              // able to add, do it
              AddCharacterToSquad(pSoldier, bAssignment);
            }

            fItWorked = TRUE;
            fRemoveFromSquad = FALSE;  // already done, would screw it up!
          }
          break;

        default:
          // remove from current vehicle/squad, if any
          if (GetSolAssignment(pSoldier) == VEHICLE) {
            TakeSoldierOutOfVehicle(pSoldier);
          }
          RemoveCharacterFromSquads(pSoldier);

          AddCharacterToAnySquad(pSoldier);

          fItWorked = TRUE;
          fRemoveFromSquad = FALSE;  // already done, would screw it up!
          break;
      }

      if (fItWorked) {
        if (fRemoveFromSquad) {
          // remove him from his old squad if he was on one
          RemoveCharacterFromSquads(pSoldier);
        }

        MakeSoldiersTacticalAnimationReflectAssignment(pSoldier);
      } else {
        // didn't work - report it once
        if (!fNotifiedOfFailure) {
          fNotifiedOfFailure = TRUE;
          NotifyPlayerOfAssignmentAttemptFailure(bAssignment);
        }
      }
    }
  }
  // reset list
  //	ResetSelectedListForMapScreen( );

  // check if we should start/stop flashing any mercs' assignment strings after these changes
  gfReEvaluateEveryonesNothingToDo = TRUE;

  return;
}

BOOLEAN IsCharacterAliveAndConscious(struct SOLDIERTYPE *pCharacter) {
  // is the character alive and conscious?
  if (pCharacter->bLife < CONSCIOUSNESS) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN ValidTrainingPartnerInSameSectorOnAssignmentFound(struct SOLDIERTYPE *pTargetSoldier,
                                                          int8_t bTargetAssignment,
                                                          int8_t bTargetStat) {
  int32_t iCounter = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  uint16_t sTrainingPts = 0;
  BOOLEAN fAtGunRange = FALSE;
  uint16_t usMaxPts;

  // this function only makes sense for training teammates or by others, not for self training which
  // doesn't require partners
  Assert((bTargetAssignment == TRAIN_TEAMMATE) || (bTargetAssignment == TRAIN_BY_OTHER));

  for (iCounter = 0; iCounter <= gTacticalStatus.Team[OUR_TEAM].bLastID; iCounter++) {
    pSoldier = GetSoldierByID(iCounter);

    if (IsSolActive(pSoldier)) {
      // if the guy is not the target, has the assignment we want, is training the same stat, and is
      // in our sector, alive and is training the stat we want
      if ((pSoldier != pTargetSoldier) && (GetSolAssignment(pSoldier) == bTargetAssignment) &&
          // CJC: this seems incorrect in light of the check for bTargetStat and in any case would
          // cause a problem if the trainer was assigned and we weren't!
          //( pSoldier -> bTrainStat == pTargetSoldier -> bTrainStat ) &&
          (GetSolSectorX(pSoldier) == pTargetSoldier->sSectorX) &&
          (GetSolSectorY(pSoldier) == pTargetSoldier->sSectorY) &&
          (GetSolSectorZ(pSoldier) == pTargetSoldier->bSectorZ) &&
          (pSoldier->bTrainStat == bTargetStat) && IsSolAlive(pSoldier)) {
        // so far so good, now let's see if the trainer can really teach the student anything new

        // are we training in the sector with gun range in Alma?
        if ((GetSolSectorX(pSoldier) == GUN_RANGE_X) && (GetSolSectorY(pSoldier) == GUN_RANGE_Y) &&
            (GetSolSectorZ(pSoldier) == GUN_RANGE_Z)) {
          fAtGunRange = TRUE;
        }

        if (GetSolAssignment(pSoldier) == TRAIN_TEAMMATE) {
          // pSoldier is the instructor, target is the student
          sTrainingPts = GetBonusTrainingPtsDueToInstructor(pSoldier, pTargetSoldier, bTargetStat,
                                                            fAtGunRange, &usMaxPts);
        } else {
          // target is the instructor, pSoldier is the student
          sTrainingPts = GetBonusTrainingPtsDueToInstructor(pTargetSoldier, pSoldier, bTargetStat,
                                                            fAtGunRange, &usMaxPts);
        }

        if (sTrainingPts > 0) {
          // yes, then he makes a valid training partner for us!
          return (TRUE);
        }
      }
    }
  }

  // no one found
  return (FALSE);
}

void UnEscortEPC(struct SOLDIERTYPE *pSoldier) {
  if (IsMapScreen()) {
    BOOLEAN fGotInfo;
    uint16_t usQuoteNum;
    uint16_t usFactToSetToTrue;

    SetupProfileInsertionDataForSoldier(pSoldier);

    fGotInfo = GetInfoForAbandoningEPC(GetSolProfile(pSoldier), &usQuoteNum, &usFactToSetToTrue);
    if (fGotInfo) {
      // say quote usQuoteNum
      gMercProfiles[GetSolProfile(pSoldier)].ubMiscFlags |= PROFILE_MISC_FLAG_FORCENPCQUOTE;
      TacticalCharacterDialogue(pSoldier, usQuoteNum);
      // the flag will be turned off in the remove-epc event
      // gMercProfiles[ GetSolProfile(pSoldier) ].ubMiscFlags &= ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
      SetFactTrue(usFactToSetToTrue);
    }
    SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_REMOVE_EPC, GetSolProfile(pSoldier), 0, 0,
                                  0, 0);

    HandleFactForNPCUnescorted(GetSolProfile(pSoldier));

    if (GetSolProfile(pSoldier) == JOHN) {
      struct SOLDIERTYPE *pSoldier2;

      // unrecruit Mary as well
      pSoldier2 = FindSoldierByProfileID(MARY, TRUE);
      if (pSoldier2) {
        SetupProfileInsertionDataForSoldier(pSoldier2);
        fGotInfo = GetInfoForAbandoningEPC(MARY, &usQuoteNum, &usFactToSetToTrue);
        if (fGotInfo) {
          // say quote usQuoteNum
          gMercProfiles[MARY].ubMiscFlags |= PROFILE_MISC_FLAG_FORCENPCQUOTE;
          TacticalCharacterDialogue(pSoldier2, usQuoteNum);
          // the flag will be turned off in the remove-epc event
          // gMercProfiles[ MARY ].ubMiscFlags &= ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
          SetFactTrue(usFactToSetToTrue);
        }

        SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_REMOVE_EPC, MARY, 0, 0, 0, 0);
      }
    } else if (GetSolProfile(pSoldier) == MARY) {
      struct SOLDIERTYPE *pSoldier2;

      // unrecruit John as well
      pSoldier2 = FindSoldierByProfileID(JOHN, TRUE);
      if (pSoldier2) {
        SetupProfileInsertionDataForSoldier(pSoldier2);
        fGotInfo = GetInfoForAbandoningEPC(JOHN, &usQuoteNum, &usFactToSetToTrue);
        if (fGotInfo) {
          // say quote usQuoteNum
          gMercProfiles[JOHN].ubMiscFlags |= PROFILE_MISC_FLAG_FORCENPCQUOTE;
          TacticalCharacterDialogue(pSoldier2, usQuoteNum);
          // the flag will be turned off in the remove-epc event
          // gMercProfiles[ JOHN ].ubMiscFlags &= ~PROFILE_MISC_FLAG_FORCENPCQUOTE;
          SetFactTrue(usFactToSetToTrue);
        }
        SpecialCharacterDialogueEvent(DIALOGUE_SPECIAL_EVENT_REMOVE_EPC, JOHN, 0, 0, 0, 0);
      }
    }
    // stop showing menu
    giAssignHighLine = -1;

    // set dirty flag
    fTeamPanelDirty = TRUE;
    fMapScreenBottomDirty = TRUE;
    fCharacterInfoPanelDirty = TRUE;
  } else {
    // how do we handle this if it's the right sector?
    TriggerNPCWithGivenApproach(GetSolProfile(pSoldier), APPROACH_EPC_IN_WRONG_SECTOR, TRUE);
  }
}

BOOLEAN CharacterIsTakingItEasy(struct SOLDIERTYPE *pSoldier) {
  // actually asleep?
  if (pSoldier->fMercAsleep == TRUE) {
    return (TRUE);
  }

  // if able to sleep
  if (CanCharacterSleep(pSoldier, FALSE)) {
    // on duty, but able to catch naps (either not traveling, or not the driver of the vehicle)
    // The actual checks for this are in the "can he sleep" check above
    if ((pSoldier->bAssignment < ON_DUTY) || (GetSolAssignment(pSoldier) == VEHICLE)) {
      return (TRUE);
    }

    // and healing up?
    if ((GetSolAssignment(pSoldier) == PATIENT) ||
        (GetSolAssignment(pSoldier) == ASSIGNMENT_HOSPITAL)) {
      return (TRUE);
    }

    // on a real assignment, but done with it?
    if (pSoldier->fDoneAssignmentAndNothingToDoFlag) {
      return (TRUE);
    }
  }

  // on assignment, or walking/driving & unable to sleep
  return (FALSE);
}

uint8_t CalcSoldierNeedForSleep(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubNeedForSleep;
  uint8_t ubPercentHealth;

  // base comes from profile
  ubNeedForSleep = gMercProfiles[GetSolProfile(pSoldier)].ubNeedForSleep;

  ubPercentHealth = pSoldier->bLife / pSoldier->bLifeMax;

  if (ubPercentHealth < 75) {
    ubNeedForSleep++;

    if (ubPercentHealth < 50) {
      ubNeedForSleep++;

      if (ubPercentHealth < 25) {
        ubNeedForSleep += 2;
      }
    }
  }

  // reduce for each Night Ops or Martial Arts trait
  ubNeedForSleep -= NUM_SKILL_TRAITS(pSoldier, NIGHTOPS);
  ubNeedForSleep -= NUM_SKILL_TRAITS(pSoldier, MARTIALARTS);

  if (ubNeedForSleep < 4) {
    ubNeedForSleep = 4;
  }

  if (ubNeedForSleep > 12) {
    ubNeedForSleep = 12;
  }

  return (ubNeedForSleep);
}

uint32_t GetLastSquadListedInSquadMenu(void) {
  uint32_t uiMaxSquad;

  uiMaxSquad = GetLastSquadActive() + 1;

  if (uiMaxSquad >= NUMBER_OF_SQUADS) {
    uiMaxSquad = NUMBER_OF_SQUADS - 1;
  }

  return (uiMaxSquad);
}

BOOLEAN CanCharacterRepairAnotherSoldiersStuff(struct SOLDIERTYPE *pSoldier,
                                               struct SOLDIERTYPE *pOtherSoldier) {
  if (pOtherSoldier == pSoldier) {
    return (FALSE);
  }
  if (!pOtherSoldier->bActive) {
    return (FALSE);
  }
  if (pOtherSoldier->bLife == 0) {
    return (FALSE);
  }
  if (pOtherSoldier->sSectorX != GetSolSectorX(pSoldier) ||
      pOtherSoldier->sSectorY != GetSolSectorY(pSoldier) ||
      pOtherSoldier->bSectorZ != GetSolSectorZ(pSoldier)) {
    return (FALSE);
  }

  if (pOtherSoldier->fBetweenSectors) {
    return (FALSE);
  }

  if ((pOtherSoldier->bAssignment == IN_TRANSIT) ||
      (pOtherSoldier->bAssignment == ASSIGNMENT_POW) ||
      (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (AM_A_ROBOT(pSoldier)) ||
      (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC) ||
      (pOtherSoldier->bAssignment == ASSIGNMENT_DEAD)) {
    return (FALSE);
  }

  return (TRUE);
}

struct SOLDIERTYPE *GetSelectedAssignSoldier(BOOLEAN fNullOK) {
  struct SOLDIERTYPE *pSoldier = NULL;

  if ((IsMapScreen())) {
    // mapscreen version
    if ((bSelectedAssignChar >= 0) && (bSelectedAssignChar < MAX_CHARACTER_COUNT) &&
        (gCharactersList[bSelectedAssignChar].fValid)) {
      pSoldier = GetSoldierByID(gCharactersList[bSelectedAssignChar].usSolID);
    }
  } else {
    // tactical version
    pSoldier = GetTacticalContextMenuMerc();
  }

  if (!fNullOK) {
    Assert(pSoldier);
  }

  if (pSoldier != NULL) {
    // better be an active person, not a vehicle
    Assert(IsSolActive(pSoldier));
    Assert(!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE));
  }

  return (pSoldier);
}

void ResumeOldAssignment(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN fOldAssignmentInvalid = FALSE;

  // ARM: I don't think the whole "old assignment" idea is a very good one, and I doubt the code
  // that maintains that variable is very foolproof, plus what meaning does the old assignemnt have
  // later, anyway? so I'd rather just settle for putting him into any squad:
  fOldAssignmentInvalid = TRUE;

  /*
          if ( pSoldier->bOldAssignment == pSoldier->bAssigment )
          {
                  // no good: we rely on this to make sure guys training militia STOP training
     militia! fOldAssignmentInvalid = TRUE;
          }
          else if( pSoldier->bOldAssignment == VEHICLE )
          {
                  SetSoldierAssignment( pSoldier, ( int8_t )( pSoldier->bOldAssignment ), (
     pSoldier->iVehicleId ), 0, 0 );

                  // it might not work - check
                  if ( pSoldier->bAssignment != VEHICLE )
                  {
                          fOldAssignmentInvalid = TRUE;
                  }
          }
          else if( pSoldier->bOldAssignment < ON_DUTY )
          {
                  if( AddCharacterToSquad( pSoldier, pSoldier->bOldAssignment ) == FALSE )
                  {
                          fOldAssignmentInvalid = TRUE;
                  }
          }
          else
          {
                  fOldAssignmentInvalid = TRUE;
          }
  */

  if (fOldAssignmentInvalid) {
    AddCharacterToAnySquad(pSoldier);
  }

  // make sure the player has time to OK this before proceeding
  StopTimeCompression();

  // assignment has changed, redraw left side as well as the map (to update on/off duty icons)
  fTeamPanelDirty = TRUE;
  fCharacterInfoPanelDirty = TRUE;
  MarkForRedrawalStrategicMap();
}

void RepairItemsOnOthers(struct SOLDIERTYPE *pSoldier, uint8_t *pubRepairPtsLeft) {
  uint8_t ubPassType;
  int8_t bLoop;
  int8_t bPocket;
  struct SOLDIERTYPE *pOtherSoldier;
  struct SOLDIERTYPE *pBestOtherSoldier;
  int8_t bPriority, bBestPriority = -1;
  BOOLEAN fSomethingWasRepairedThisPass;

  // repair everyone's hands and armor slots first, then headgear, and pockets last
  for (ubPassType = REPAIR_HANDS_AND_ARMOR; ubPassType <= FINAL_REPAIR_PASS; ubPassType++) {
    fSomethingWasRepairedThisPass = FALSE;

    // look for jammed guns on other soldiers in sector and unjam them
    for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
         bLoop < gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++) {
      pOtherSoldier = MercPtrs[bLoop];

      // check character is valid, alive, same sector, not between, has inventory, etc.
      if (CanCharacterRepairAnotherSoldiersStuff(pSoldier, pOtherSoldier)) {
        if (UnjamGunsOnSoldier(pOtherSoldier, pSoldier, pubRepairPtsLeft)) {
          fSomethingWasRepairedThisPass = TRUE;
        }
      }
    }

    while (*pubRepairPtsLeft > 0) {
      bBestPriority = -1;
      pBestOtherSoldier = NULL;

      // now look for items to repair on other mercs
      for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
           bLoop < gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++) {
        pOtherSoldier = MercPtrs[bLoop];

        // check character is valid, alive, same sector, not between, has inventory, etc.
        if (CanCharacterRepairAnotherSoldiersStuff(pSoldier, pOtherSoldier)) {
          // okay, seems like a candidate!
          if (FindRepairableItemOnOtherSoldier(pOtherSoldier, ubPassType) != NO_SLOT) {
            bPriority = pOtherSoldier->bExpLevel;
            if (bPriority > bBestPriority) {
              bBestPriority = bPriority;
              pBestOtherSoldier = pOtherSoldier;
            }
          }
        }
      }

      // did we find anyone to repair on this pass?
      if (pBestOtherSoldier != NULL) {
        // yes, repair all items (for this pass type!) on this soldier that need repair
        do {
          bPocket = FindRepairableItemOnOtherSoldier(pBestOtherSoldier, ubPassType);
          if (bPocket != NO_SLOT) {
            if (RepairObject(pSoldier, pBestOtherSoldier, &(pBestOtherSoldier->inv[bPocket]),
                             pubRepairPtsLeft)) {
              fSomethingWasRepairedThisPass = TRUE;
            }
          }
        } while (bPocket != NO_SLOT && *pubRepairPtsLeft > 0);
      } else {
        break;
      }
    }

    if (fSomethingWasRepairedThisPass && !DoesCharacterHaveAnyItemsToRepair(pSoldier, ubPassType)) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sRepairsDoneString[1 + ubPassType],
                pSoldier->name);

      // let player react
      StopTimeCompression();
    }
  }
}

BOOLEAN UnjamGunsOnSoldier(struct SOLDIERTYPE *pOwnerSoldier, struct SOLDIERTYPE *pRepairSoldier,
                           uint8_t *pubRepairPtsLeft) {
  BOOLEAN fAnyGunsWereUnjammed = FALSE;
  int8_t bPocket;

  // try to unjam everything before beginning any actual repairs.. successful unjamming costs 2
  // points per weapon
  for (bPocket = HANDPOS; bPocket <= SMALLPOCK8POS; bPocket++) {
    // the object a weapon? and jammed?
    if ((Item[pOwnerSoldier->inv[bPocket].usItem].usItemClass == IC_GUN) &&
        (pOwnerSoldier->inv[bPocket].bGunAmmoStatus < 0)) {
      if (*pubRepairPtsLeft >= REPAIR_COST_PER_JAM) {
        *pubRepairPtsLeft -= REPAIR_COST_PER_JAM;

        pOwnerSoldier->inv[bPocket].bGunAmmoStatus *= -1;

        // MECHANICAL/DEXTERITY GAIN: Unjammed a gun
        StatChange(pRepairSoldier, MECHANAMT, 5, FALSE);
        StatChange(pRepairSoldier, DEXTAMT, 5, FALSE);

        // report it as unjammed
        if (pRepairSoldier == pOwnerSoldier) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[53],
                    pRepairSoldier->name, ItemNames[pOwnerSoldier->inv[bPocket].usItem]);
        } else {
          // NOTE: may need to be changed for localized versions
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[54],
                    pRepairSoldier->name, pOwnerSoldier->name,
                    ItemNames[pOwnerSoldier->inv[bPocket].usItem]);
        }

        fAnyGunsWereUnjammed = TRUE;
      } else {
        // out of points, we're done for now
        break;
      }
    }
  }

  return (fAnyGunsWereUnjammed);
}
