// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/HandleDoors.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "GameRes.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/Quests.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Interface.h"
#include "Tactical/Keys.h"
#include "Tactical/Overhead.h"
#include "Tactical/Points.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/StructureWrap.h"
#include "TacticalAI/AI.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"

BOOLEAN gfSetPerceivedDoorState = FALSE;

BOOLEAN HandleDoorsOpenClose(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                             struct STRUCTURE *pStructure, BOOLEAN fNoAnimations);

void HandleDoorChangeFromGridNo(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                BOOLEAN fNoAnimations) {
  struct STRUCTURE *pStructure;
  DOOR_STATUS *pDoorStatus;
  BOOLEAN fDoorsAnimated = FALSE;

  pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);

  if (pStructure == NULL) {
#ifdef JA2TESTVERSION
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
              L"ERROR: Told to handle door that does not exist at %d.", sGridNo);
#endif
    return;
  }

  fDoorsAnimated = HandleDoorsOpenClose(pSoldier, sGridNo, pStructure, fNoAnimations);
  if (SwapStructureForPartner(sGridNo, pStructure) != NULL) {
    RecompileLocalMovementCosts(sGridNo);
  }

  // set door busy
  pDoorStatus = GetDoorStatus(sGridNo);
  if (pDoorStatus == NULL) {
#ifdef JA2TESTVERSION
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION,
              L"ERROR: Told to set door busy but can't get door status at %d!", sGridNo);
#endif
    return;
  }

  // ATE: Only do if animated.....
  if (fDoorsAnimated) {
    pDoorStatus->ubFlags |= DOOR_BUSY;
  }
}

uint16_t GetAnimStateForInteraction(struct SOLDIERTYPE *pSoldier, BOOLEAN fDoor,
                                    uint16_t usAnimState) {
  switch (usAnimState) {
    case OPEN_DOOR:

      if (pSoldier->ubBodyType == CRIPPLECIV) {
        return (CRIPPLE_OPEN_DOOR);
      } else {
        if (fDoor) {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (OPEN_DOOR_CROUCHED);
          } else {
            return (usAnimState);
          }
        } else {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (BEGIN_OPENSTRUCT_CROUCHED);
          } else {
            return (BEGIN_OPENSTRUCT);
          }
        }
      }
      break;

    case CLOSE_DOOR:

      if (pSoldier->ubBodyType == CRIPPLECIV) {
        return (CRIPPLE_CLOSE_DOOR);
      } else {
        if (fDoor) {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (CLOSE_DOOR_CROUCHED);
          } else {
            return (usAnimState);
          }
        } else {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (OPEN_STRUCT_CROUCHED);
          } else {
            return (OPEN_STRUCT);
          }
        }
      }
      break;

    case END_OPEN_DOOR:

      if (pSoldier->ubBodyType == CRIPPLECIV) {
        return (CRIPPLE_END_OPEN_DOOR);
      } else {
        if (fDoor) {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (END_OPEN_DOOR_CROUCHED);
          } else {
            return (usAnimState);
          }
        } else {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (END_OPENSTRUCT_CROUCHED);
          } else {
            return (END_OPENSTRUCT);
          }
        }
      }
      break;

    case END_OPEN_LOCKED_DOOR:

      if (pSoldier->ubBodyType == CRIPPLECIV) {
        return (CRIPPLE_END_OPEN_LOCKED_DOOR);
      } else {
        if (fDoor) {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (END_OPEN_LOCKED_DOOR_CROUCHED);
          } else {
            return (END_OPEN_LOCKED_DOOR);
          }
        } else {
          if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
            return (END_OPENSTRUCT_LOCKED_CROUCHED);
          } else {
            return (END_OPENSTRUCT_LOCKED);
          }
        }
      }
      break;

    case PICK_LOCK:

      if (gAnimControl[pSoldier->usAnimState].ubEndHeight != ANIM_STAND) {
        return (LOCKPICK_CROUCHED);
      } else {
        return (PICK_LOCK);
      }
      break;

    default:
      // should never happen!
      Assert(FALSE);
      return (usAnimState);
      break;
  }
}

void InteractWithClosedDoor(struct SOLDIERTYPE *pSoldier, uint8_t ubHandleCode) {
  pSoldier->ubDoorHandleCode = ubHandleCode;

  switch (ubHandleCode) {
    case HANDLE_DOOR_OPEN:
    case HANDLE_DOOR_UNLOCK:
    case HANDLE_DOOR_EXAMINE:
    case HANDLE_DOOR_EXPLODE:
    case HANDLE_DOOR_LOCK:
    case HANDLE_DOOR_UNTRAP:
    case HANDLE_DOOR_CROWBAR:

      ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, TRUE, OPEN_DOOR), 0, FALSE);
      break;

    case HANDLE_DOOR_FORCE:

      ChangeSoldierState(pSoldier, KICK_DOOR, 0, FALSE);
      break;

    case HANDLE_DOOR_LOCKPICK:

      ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, TRUE, PICK_LOCK), 0, FALSE);
      break;
  }
}

BOOLEAN DoTrapCheckOnStartingMenu(struct SOLDIERTYPE *pSoldier, DOOR *pDoor) {
  int8_t bDetectLevel;

  if (pDoor && pDoor->fLocked && pDoor->ubTrapID != NO_TRAP &&
      pDoor->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN) {
    // check for noticing the trap
    bDetectLevel = CalcTrapDetectLevel(pSoldier, FALSE);
    if (bDetectLevel >= pDoor->ubTrapLevel) {
      // say quote, update status
      TacticalCharacterDialogue(pSoldier, QUOTE_BOOBYTRAP_ITEM);
      UpdateDoorPerceivedValue(pDoor);

      return (TRUE);
    }
  }

  return (FALSE);
}

void InteractWithOpenableStruct(struct SOLDIERTYPE *pSoldier, struct STRUCTURE *pStructure,
                                uint8_t ubDirection, BOOLEAN fDoor) {
  struct STRUCTURE *pBaseStructure;
  BOOLEAN fDoMenu = FALSE;
  DOOR *pDoor;
  DOOR_STATUS *pDoorStatus;
  BOOLEAN fTrapsFound = FALSE;

  pBaseStructure = FindBaseStructure(pStructure);

  if (fDoor) {
    // get door status, if busy then just return!
    pDoorStatus = GetDoorStatus(pBaseStructure->sGridNo);
    if (pDoorStatus && (pDoorStatus->ubFlags & DOOR_BUSY)) {
      // Send this guy into stationary stance....
      EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);

      if (pSoldier->bTeam == gbPlayerNum) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[DOOR_IS_BUSY]);
      } else {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("Trying to open door and door is busy: %d", GetSolID(pSoldier)));
      }
      return;
    }
  }

  EVENT_SetSoldierDesiredDirection(pSoldier, ubDirection);

  // Is the door opened?
  if (pStructure->fFlags & STRUCTURE_OPEN) {
    if (pSoldier->ubID <= gTacticalStatus.Team[gbPlayerNum].bLastID &&
        !(pStructure->fFlags & STRUCTURE_SWITCH)) {
      // Bring up menu to decide what to do....
      SoldierGotoStationaryStance(pSoldier);

      pDoor = FindDoorInfoAtGridNo(pBaseStructure->sGridNo);
      if (pDoor) {
        if (DoTrapCheckOnStartingMenu(pSoldier, pDoor)) {
          fTrapsFound = TRUE;
        }
      }

      // Pull Up Menu
      if (!fTrapsFound) {
        InitDoorOpenMenu(pSoldier, pStructure, ubDirection, TRUE);
      }
    } else {
      // Easily close door....
      ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, CLOSE_DOOR), 0,
                         FALSE);
    }
  } else {
    // Bring up the menu, only if it has a lock!
    if (pSoldier->ubID <= gTacticalStatus.Team[gbPlayerNum].bLastID) {
      pDoor = FindDoorInfoAtGridNo(pBaseStructure->sGridNo);

      if (pDoor != NULL) {
        // Assume true
        fDoMenu = TRUE;

        // Check if it's locked.....
        // If not locked, don't bring it up!
        if (!pDoor->fLocked) {
          fDoMenu = FALSE;
        }
      }
    }

    if (fDoMenu) {
      // Bring up menu to decide what to do....
      SoldierGotoStationaryStance(pSoldier);

      if (DoTrapCheckOnStartingMenu(pSoldier, pDoor)) {
        fTrapsFound = TRUE;
      }

      // Pull Up Menu
      if (!fTrapsFound) {
        InitDoorOpenMenu(pSoldier, pStructure, ubDirection, FALSE);
      } else {
        UnSetUIBusy(pSoldier->ubID);
      }
    } else {
      pSoldier->ubDoorHandleCode = HANDLE_DOOR_OPEN;

      ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, OPEN_DOOR), 0,
                         FALSE);
    }
  }
}

void ProcessImplicationsOfPCMessingWithDoor(struct SOLDIERTYPE *pSoldier) {
  uint8_t ubRoom;
  struct SOLDIERTYPE *pGoon;
  // if player is hacking at a door in the brothel and a kingpin guy can see him
  if ((InARoom(pSoldier->sGridNo, &ubRoom) && IN_BROTHEL(ubRoom)) ||
      (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_D && gbWorldSectorZ == 0 &&
       (pSoldier->sGridNo == 11010 || pSoldier->sGridNo == 11177 || pSoldier->sGridNo == 11176))) {
    uint8_t ubLoop;

    // see if a kingpin goon can see us
    for (ubLoop = gTacticalStatus.Team[CIV_TEAM].bFirstID;
         ubLoop <= gTacticalStatus.Team[CIV_TEAM].bLastID; ubLoop++) {
      pGoon = MercPtrs[ubLoop];
      if (pGoon->ubCivilianGroup == KINGPIN_CIV_GROUP && pGoon->bActive && pGoon->bInSector &&
          pGoon->bLife >= OKLIFE && pGoon->bOppList[pSoldier->ubID] == SEEN_CURRENTLY) {
        MakeCivHostile(pGoon, 2);
        if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
          EnterCombatMode(pGoon->bTeam);
        }
      }
    }
  }

  if (gWorldSectorX == TIXA_SECTOR_X && gWorldSectorY == TIXA_SECTOR_Y) {
    pGoon = FindSoldierByProfileID(WARDEN, FALSE);
    if (pGoon && pGoon->bAlertStatus < STATUS_RED &&
        PythSpacesAway(pSoldier->sGridNo, pGoon->sGridNo) <= 5) {
      // alert her if she hasn't been alerted
      pGoon->bAlertStatus = STATUS_RED;
      CheckForChangingOrders(pGoon);
      CancelAIAction(pGoon, TRUE);
    }
  }
}

BOOLEAN HandleOpenableStruct(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                             struct STRUCTURE *pStructure) {
  BOOLEAN fHandleDoor = FALSE;
  int16_t sAPCost = 0, sBPCost = 0;
  DOOR *pDoor;
  BOOLEAN fTrapFound = FALSE;
  BOOLEAN fDoAction = TRUE;
  BOOLEAN fDoor = FALSE;

  // Are we a door?
  if (pStructure->fFlags & STRUCTURE_ANYDOOR) {
    fDoor = TRUE;
  }

  // Calculate basic points...

  // We'll add any aps for things like lockpicking, booting, etc

  // If we are already open....no need for lockpick checks, etc
  if (pStructure->fFlags & STRUCTURE_OPEN) {
    // Set costs for these
    sAPCost = AP_OPEN_DOOR;
    sBPCost = BP_OPEN_DOOR;

    fHandleDoor = TRUE;
  } else {
    if (pSoldier->ubID < 20) {
      // Find locked door here....
      pDoor = FindDoorInfoAtGridNo(sGridNo);

      // Alrighty, first check for traps ( unless we are examining.... )
      if (pSoldier->ubDoorHandleCode != HANDLE_DOOR_EXAMINE &&
          pSoldier->ubDoorHandleCode != HANDLE_DOOR_UNTRAP &&
          pSoldier->ubDoorHandleCode != HANDLE_DOOR_UNLOCK) {
        if (pDoor != NULL) {
          // Do we have a trap? NB if door is unlocked disable all traps
          if (pDoor->fLocked && pDoor->ubTrapID != NO_TRAP) {
            fTrapFound = TRUE;

            // Set costs for these
            // Set AP costs to that of opening a door
            sAPCost = AP_OPEN_DOOR;
            sBPCost = BP_OPEN_DOOR;

            ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR),
                               0, FALSE);

            // Did we inadvertently set it off?
            if (HasDoorTrapGoneOff(pSoldier, pDoor)) {
              // Kaboom
              // Code to handle trap here...
              HandleDoorTrap(pSoldier, pDoor);
              if (DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_STOPS_ACTION) {
                // trap stops person from opening door!
                fDoAction = FALSE;
              }
              if (!(DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING)) {
                // trap only happens once
                pDoor->ubTrapLevel = 0;
                pDoor->ubTrapID = NO_TRAP;
              }
              UpdateDoorPerceivedValue(pDoor);
            } else {
              // If we didn't set it off then we must have noticed it or know about it already

              // do we know it's trapped?
              if (pDoor->bPerceivedTrapped == DOOR_PERCEIVED_UNKNOWN) {
                switch (pDoor->ubTrapID) {
                  case BROTHEL_SIREN:
                    ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                              TacticalStr[DOOR_LOCK_DESCRIPTION_STR], pDoorTrapStrings[SIREN]);
                    break;
                  case SUPER_ELECTRIC:
                    ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                              TacticalStr[DOOR_LOCK_DESCRIPTION_STR], pDoorTrapStrings[ELECTRIC]);
                    break;
                  default:
                    ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                              TacticalStr[DOOR_LOCK_DESCRIPTION_STR],
                              pDoorTrapStrings[pDoor->ubTrapID]);
                    break;
                }

                // Stop action this time....
                fDoAction = FALSE;

                // report!
                TacticalCharacterDialogue(pSoldier, QUOTE_BOOBYTRAP_ITEM);
              } else {
                // Set it off!
                HandleDoorTrap(pSoldier, pDoor);
                if (DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_STOPS_ACTION) {
                  // trap stops person from opening door!
                  fDoAction = FALSE;
                }
                if (!(DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING)) {
                  // trap only happens once
                  pDoor->ubTrapLevel = 0;
                  pDoor->ubTrapID = NO_TRAP;
                }
              }
              UpdateDoorPerceivedValue(pDoor);
            }
          }
        }
      }

      if (fDoAction) {
        // OK, switch based on how we are going to open door....
        switch (pSoldier->ubDoorHandleCode) {
          case HANDLE_DOOR_OPEN:

            // If we have no lock on door...
            if (pDoor == NULL) {
              // Set costs for these
              sAPCost = AP_OPEN_DOOR;
              sBPCost = BP_OPEN_DOOR;

              // Open if it's not locked....
              ChangeSoldierState(
                  pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
              fHandleDoor = TRUE;
              break;
            } else {
              if (pDoor->fLocked) {
                // it's locked....
                ChangeSoldierState(
                    pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0,
                    FALSE);

                // Do we have a quote for locked stuff?
                // Now just show on message bar
                if (!AM_AN_EPC(pSoldier)) {
                  DoMercBattleSound(pSoldier, BATTLE_SOUND_LOCKED);
                } else {
                  ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                            TacticalStr[DOOR_LOCK_HAS_BEEN_LOCKED_STR]);
                }
              } else {
                ChangeSoldierState(
                    pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
                fHandleDoor = TRUE;
              }
              UpdateDoorPerceivedValue(pDoor);
              break;
            }
            break;

          case HANDLE_DOOR_FORCE:

            // Set costs for these
            sAPCost = AP_BOOT_DOOR;
            sBPCost = BP_BOOT_DOOR;

            // OK, using force, if we have no lock, just open the door!
            if (pDoor == NULL) {
              ChangeSoldierState(
                  pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
              fHandleDoor = TRUE;

              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
            } else {
              // Attempt to force door
              if (AttemptToSmashDoor(pSoldier, pDoor)) {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESTROYED_STR ]
                // );
                // DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
                fHandleDoor = TRUE;
              } else {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                // DOOR_LOCK_NOT_DESTROYED_STR ] );
                UpdateDoorPerceivedValue(pDoor);
              }
              ProcessImplicationsOfPCMessingWithDoor(pSoldier);
            }
            break;

          case HANDLE_DOOR_CROWBAR:

            // Set costs for these
            sAPCost = AP_USE_CROWBAR;
            sBPCost = BP_USE_CROWBAR;

            // OK, using force, if we have no lock, just open the door!
            if (pDoor == NULL) {
              ChangeSoldierState(
                  pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
              fHandleDoor = TRUE;

              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
            } else {
              // Attempt to force door
              if (AttemptToCrowbarLock(pSoldier, pDoor)) {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESTROYED_STR ]
                // ); DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
                fHandleDoor = TRUE;
              } else {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                // DOOR_LOCK_NOT_DESTROYED_STR ] );
                UpdateDoorPerceivedValue(pDoor);
              }

              ProcessImplicationsOfPCMessingWithDoor(pSoldier);
            }
            break;

          case HANDLE_DOOR_EXPLODE:

            // Set costs for these
            sAPCost = AP_EXPLODE_DOOR;
            sBPCost = BP_EXPLODE_DOOR;

            if (pDoor == NULL) {
              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
            } else {
              // Attempt to force door
              if (AttemptToBlowUpLock(pSoldier, pDoor)) {
                // DoMercBattleSound( pSoldier, BATTLE_SOUND_COOL1 );
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_LOCK_DESTROYED_STR ]
                // );
                fHandleDoor = TRUE;
              } else {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                // DOOR_LOCK_NOT_DESTROYED_STR ] );
                UpdateDoorPerceivedValue(pDoor);
              }
              ProcessImplicationsOfPCMessingWithDoor(pSoldier);
            }
            break;

          case HANDLE_DOOR_LOCKPICK:

            // Set costs for these
            sAPCost = AP_PICKLOCK;
            sBPCost = BP_PICKLOCK;

            // Attempt to pick lock
            if (pDoor == NULL) {
              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
            } else {
              if (AttemptToPickLock(pSoldier, pDoor)) {
                DoMercBattleSound(pSoldier, BATTLE_SOUND_COOL1);
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                // DOOR_LOCK_HAS_BEEN_PICKED_STR ] );
                fHandleDoor = TRUE;
              } else {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                // DOOR_LOCK_HAS_NOT_BEEN_PICKED_STR ] );
              }
              ProcessImplicationsOfPCMessingWithDoor(pSoldier);
            }
            break;

          case HANDLE_DOOR_EXAMINE:

            // Set costs for these
            sAPCost = AP_EXAMINE_DOOR;
            sBPCost = BP_EXAMINE_DOOR;

            // Attempt to examine door
            // Whatever the result, end the open animation
            ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR),
                               0, FALSE);

            if (pDoor == NULL) {
              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
            } else {
              if (ExamineDoorForTraps(pSoldier, pDoor)) {
                // We have a trap. Use door pointer to determine what type, etc
                TacticalCharacterDialogue(pSoldier, QUOTE_BOOBYTRAP_ITEM);
                switch (pDoor->ubTrapID) {
                  case BROTHEL_SIREN:
                    ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                              TacticalStr[DOOR_LOCK_DESCRIPTION_STR], pDoorTrapStrings[SIREN]);
                    break;
                  case SUPER_ELECTRIC:
                    ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                              TacticalStr[DOOR_LOCK_DESCRIPTION_STR], pDoorTrapStrings[ELECTRIC]);
                    break;
                  default:
                    ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                              TacticalStr[DOOR_LOCK_DESCRIPTION_STR],
                              pDoorTrapStrings[pDoor->ubTrapID]);
                    break;
                }

                UpdateDoorPerceivedValue(pDoor);
              } else {
                ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_LOCK_UNTRAPPED_STR]);
              }
            }
            break;

          case HANDLE_DOOR_UNLOCK:

            // Set costs for these
            sAPCost = AP_UNLOCK_DOOR;
            sBPCost = BP_UNLOCK_DOOR;

            // OK, if we have no lock, show that!
            if (pDoor == NULL) {
              // Open if it's not locked....
              // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_THERE_IS_NO_LOCK_STR ]
              // );
              ChangeSoldierState(
                  pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
              break;
            } else {
              // it's locked....
              // Attempt to unlock....
              if (AttemptToUnlockDoor(pSoldier, pDoor)) {
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                // DOOR_LOCK_HAS_BEEN_UNLOCKED_STR ] ); DoMercBattleSound( pSoldier,
                // BATTLE_SOUND_COOL1 );

                ChangeSoldierState(
                    pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
                UpdateDoorPerceivedValue(pDoor);

                fHandleDoor = TRUE;
              } else {
                ChangeSoldierState(
                    pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0,
                    FALSE);
                // Do we have a quote for locked stuff?
                // Now just show on message bar
                // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[ DOOR_NOT_PROPER_KEY_STR
                // ], pSoldier->name );

                // OK PLay damn battle sound
                if (Random(2)) {
                  DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
                }
              }
            }
            break;

          case HANDLE_DOOR_UNTRAP:

            // Set costs for these
            sAPCost = AP_UNTRAP_DOOR;
            sBPCost = BP_UNTRAP_DOOR;

            // OK, if we have no lock, show that!
            if (pDoor == NULL) {
              // Open if it's not locked....
              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
              ChangeSoldierState(
                  pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
              break;
            } else {
              // Do we have a trap?
              if (pDoor->ubTrapID != NO_TRAP) {
                fTrapFound = TRUE;
              }

              if (fTrapFound) {
                if (AttemptToUntrapDoor(pSoldier, pDoor)) {
                  // ScreenMsg( MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[
                  // DOOR_LOCK_HAS_BEEN_UNTRAPPED_STR ] );
                  DoMercBattleSound(pSoldier, BATTLE_SOUND_COOL1);
                  ChangeSoldierState(pSoldier,
                                     GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0,
                                     FALSE);
                  UpdateDoorPerceivedValue(pDoor);
                  // fHandleDoor = TRUE;
                } else {
                  ChangeSoldierState(
                      pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_LOCKED_DOOR),
                      0, FALSE);
                  // Now just show on message bar
                  HandleDoorTrap(pSoldier, pDoor);

                  if (!(DoorTrapTable[pDoor->ubTrapID].fFlags & DOOR_TRAP_RECURRING)) {
                    // trap only happens once
                    pDoor->ubTrapLevel = 0;
                    pDoor->ubTrapID = NO_TRAP;
                  }

                  // Update perceived lock value
                  UpdateDoorPerceivedValue(pDoor);
                }
              } else {
                ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                          TacticalStr[DOOR_LOCK_IS_NOT_TRAPPED_STR]);
              }
            }
            break;

          case HANDLE_DOOR_LOCK:

            // Set costs for these
            sAPCost = AP_LOCK_DOOR;
            sBPCost = BP_LOCK_DOOR;

            // OK, if we have no lock, show that!
            if (pDoor == NULL) {
              // Open if it's not locked....
              ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_THERE_IS_NO_LOCK_STR]);
              ChangeSoldierState(
                  pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
              break;
            } else {
              // it's locked....
              // Attempt to unlock....
              if (AttemptToLockDoor(pSoldier, pDoor)) {
                ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE,
                          TacticalStr[DOOR_LOCK_HAS_BEEN_LOCKED_STR]);
                ChangeSoldierState(
                    pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0, FALSE);
                UpdateDoorPerceivedValue(pDoor);
              } else {
                ChangeSoldierState(
                    pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_LOCKED_DOOR), 0,
                    FALSE);
                // Do we have a quote for locked stuff?
                // Now just show on message bar
                ScreenMsg(MSG_FONT_YELLOW, MSG_INTERFACE, TacticalStr[DOOR_NOT_PROPER_KEY_STR],
                          pSoldier->name);

                // Update perceived lock value
                UpdateDoorPerceivedValue(pDoor);
              }
            }
            break;
        }
      }
    } else {
      // Set costs for these
      sAPCost = AP_OPEN_DOOR;
      sBPCost = BP_OPEN_DOOR;

      // Open if it's not locked....
      ChangeSoldierState(pSoldier, GetAnimStateForInteraction(pSoldier, fDoor, END_OPEN_DOOR), 0,
                         FALSE);
      fHandleDoor = TRUE;
    }
  }

  if (fHandleDoor) {
    if (fDoor) {
      HandleDoorChangeFromGridNo(pSoldier, sGridNo, FALSE);
    } else {
      HandleStructChangeFromGridNo(pSoldier, sGridNo);
    }
  }

  // Deduct points!
  // if ( fDoor )
  {
    DeductPoints(pSoldier, sAPCost, sBPCost);
  }

  return (fHandleDoor);
}

BOOLEAN HandleDoorsOpenClose(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                             struct STRUCTURE *pStructure, BOOLEAN fNoAnimations) {
  struct LEVELNODE *pNode;
  int32_t cnt;
  BOOLEAN fOpenedGraphic = FALSE;
  ANITILE_PARAMS AniParams;
  BOOLEAN fDoAnimation = TRUE;
  struct STRUCTURE *pBaseStructure;
  uint32_t uiSoundID;

  pBaseStructure = FindBaseStructure(pStructure);
  if (!pBaseStructure) {
    return (FALSE);
  }

  pNode = FindLevelNodeBasedOnStructure(pBaseStructure->sGridNo, pBaseStructure);
  if (!pNode) {
    return (FALSE);
  }

  // ATE: if we are about to swap, but have an animation playing here..... stop the animation....
  if ((pNode->uiFlags & LEVELNODE_ANIMATION)) {
    if (pNode->pAniTile != NULL) {
      if (pNode->pAniTile->uiFlags & ANITILE_DOOR) {
        // ATE: No two doors can exist ( there can be only one )
        // Update value.. ie: prematurely end door animation
        // Update current frame...

        if (pNode->pAniTile->uiFlags & ANITILE_FORWARD) {
          pNode->sCurrentFrame = pNode->pAniTile->sStartFrame + pNode->pAniTile->usNumFrames;
        }

        if (pNode->pAniTile->uiFlags & ANITILE_BACKWARD) {
          pNode->sCurrentFrame = pNode->pAniTile->sStartFrame - pNode->pAniTile->usNumFrames;
        }

        pNode->sCurrentFrame = pNode->pAniTile->usNumFrames - 1;

        // Delete...
        DeleteAniTile(pNode->pAniTile);

        pNode->uiFlags &= ~(LEVELNODE_LASTDYNAMIC | LEVELNODE_UPDATESAVEBUFFERONCE);

        if (GridNoOnScreen(pBaseStructure->sGridNo)) {
          SetRenderFlags(RENDER_FLAG_FULL);
        }
      }
    }
  }

  // Check the graphic which is down!
  // Check for Open Door!
  cnt = 0;
  while (gOpenDoorList[cnt] != -1) {
    // IF WE ARE A SHADOW TYPE
    if (pNode->usIndex == gOpenDoorList[cnt]) {
      fOpenedGraphic = TRUE;
      break;
    }
    cnt++;
  };

  if (!(pStructure->fFlags & STRUCTURE_OPEN)) {
    // ATE, the last parameter is the perceived value, I dont know what it is so could you please
    // add the value? ModifyDoorStatus( int16_t sGridNo, BOOLEAN fOpen, BOOLEAN fPercievedOpen )
    if (gfSetPerceivedDoorState) {
      ModifyDoorStatus(sGridNo, TRUE, TRUE);
    } else {
      ModifyDoorStatus(sGridNo, TRUE, DONTSETDOORSTATUS);
    }

    if (gWorldSectorX == 13 && gWorldSectorY == MAP_ROW_I) {
      DoPOWPathChecks();
    }

    if (pSoldier) {
      // OK, Are we a player merc or AI?
      if (pSoldier->bTeam != gbPlayerNum) {
        // If an AI guy... do LOS check first....
        // If guy is visible... OR fading...
        if (pSoldier->bVisible == -1 && !AllMercsLookForDoor(sGridNo, FALSE) &&
            !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
          fDoAnimation = FALSE;
        }
      }
    } else {
      // door opening by action item... just do a LOS check
      if (!AllMercsLookForDoor(sGridNo, FALSE)) {
        fDoAnimation = FALSE;
      }
    }

    if (fNoAnimations) {
      fDoAnimation = FALSE;
    }

    if (fDoAnimation) {
      // Update perceived value
      ModifyDoorStatus(sGridNo, DONTSETDOORSTATUS, TRUE);

      if (fOpenedGraphic) {
        memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
        AniParams.sGridNo = sGridNo;
        AniParams.ubLevelID = ANI_STRUCT_LEVEL;
        AniParams.usTileType = (uint16_t)gTileDatabase[pNode->usIndex].fType;
        AniParams.usTileIndex = pNode->usIndex;
        AniParams.sDelay = INTTILE_DOOR_OPENSPEED;
        AniParams.sStartFrame = pNode->sCurrentFrame;
        AniParams.uiFlags = ANITILE_DOOR | ANITILE_FORWARD | ANITILE_EXISTINGTILE;
        AniParams.pGivenLevelNode = pNode;

        CreateAnimationTile(&AniParams);
      } else {
        memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
        AniParams.sGridNo = sGridNo;
        AniParams.ubLevelID = ANI_STRUCT_LEVEL;
        AniParams.usTileType = (uint16_t)gTileDatabase[pNode->usIndex].fType;
        AniParams.usTileIndex = pNode->usIndex;
        AniParams.sDelay = INTTILE_DOOR_OPENSPEED;
        AniParams.sStartFrame = pNode->sCurrentFrame;
        AniParams.uiFlags = ANITILE_DOOR | ANITILE_BACKWARD | ANITILE_EXISTINGTILE;
        AniParams.pGivenLevelNode = pNode;

        CreateAnimationTile(&AniParams);
      }
    }

    // SHADOW STUFF HERE
    // if ( pShadowNode != NULL )
    //{
    //	pShadowNode->uiFlags |= LEVELNODE_ANIMATION;
    //	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_PLAYONCE;
    //	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_FORWARD;
    //	if ( pShadowNode->uiFlags & LEVELNODE_ANIMATION_BACKWARD )
    //		pShadowNode->uiFlags ^= LEVELNODE_ANIMATION_BACKWARD;
    //	pShadowNode->sDelay		= INTTILE_DOOR_OPENSPEED;
    //}

    if (fDoAnimation && pSoldier && pSoldier->ubDoorOpeningNoise) {
      // ATE; Default to normal door...
      uiSoundID = (DROPEN_1 + Random(3));

      // OK, check if this door is sliding and is multi-tiled...
      if (pStructure->fFlags & STRUCTURE_SLIDINGDOOR) {
        // Get database value...
        if (pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1) {
          // change sound ID
          uiSoundID = GARAGE_DOOR_OPEN;
        } else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_CLOTH) {
          // change sound ID
          uiSoundID = CURTAINS_OPEN;
        }
      } else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_METAL ||
                 pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_THICKER_METAL ||
                 pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_HEAVY_METAL) {
        // change sound ID
        uiSoundID = METAL_DOOR_OPEN;
      }

      // OK, We must know what sound to play, for now use same sound for all doors...
      PlayJA2Sample(uiSoundID, RATE_11025, SoundVolume(MIDVOLUME, sGridNo), 1, SoundDir(sGridNo));
    }

  } else {
    // ATE, the last parameter is the perceived value, I dont know what it is so could you please
    // add the value? ModifyDoorStatus( int16_t sGridNo, BOOLEAN fOpen, BOOLEAN
    // fInitiallyPercieveOpen
    // )

    if (gfSetPerceivedDoorState) {
      ModifyDoorStatus(sGridNo, FALSE, FALSE);
    } else {
      ModifyDoorStatus(sGridNo, FALSE, DONTSETDOORSTATUS);
    }

    if (pSoldier) {
      // OK, Are we a player merc or AI?
      if (pSoldier->bTeam != gbPlayerNum) {
        // If an AI guy... do LOS check first....
        // If guy is visible... OR fading...
        if (pSoldier->bVisible == -1 && !AllMercsLookForDoor(sGridNo, FALSE) &&
            !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) {
          fDoAnimation = FALSE;
        }
      }
    } else {
      // door opening by action item... just do a LOS check
      if (!AllMercsLookForDoor(sGridNo, FALSE)) {
        fDoAnimation = FALSE;
      }
    }

    if (fNoAnimations) {
      fDoAnimation = FALSE;
    }

    if (fDoAnimation) {
      // Update perceived value
      ModifyDoorStatus(sGridNo, DONTSETDOORSTATUS, FALSE);

      memset(&AniParams, 0, sizeof(ANITILE_PARAMS));

      // ATE; Default to normal door...
      uiSoundID = (DRCLOSE_1 + Random(2));

      // OK, check if this door is sliding and is multi-tiled...
      if (pStructure->fFlags & STRUCTURE_SLIDINGDOOR) {
        // Get database value...
        if (pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1) {
          // change sound ID
          uiSoundID = GARAGE_DOOR_CLOSE;
        } else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_CLOTH) {
          // change sound ID
          uiSoundID = CURTAINS_CLOSE;
        }
      } else if (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_METAL ||
                 pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_THICKER_METAL ||
                 pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_HEAVY_METAL) {
        // change sound ID
        uiSoundID = METAL_DOOR_CLOSE;
      }

      AniParams.uiKeyFrame1Code = ANI_KEYFRAME_DO_SOUND;
      AniParams.uiUserData = uiSoundID;
      AniParams.uiUserData3 = sGridNo;

      if (fOpenedGraphic) {
        AniParams.sGridNo = sGridNo;
        AniParams.ubLevelID = ANI_STRUCT_LEVEL;
        AniParams.usTileType = (uint16_t)gTileDatabase[pNode->usIndex].fType;
        AniParams.usTileIndex = pNode->usIndex;
        AniParams.sDelay = INTTILE_DOOR_OPENSPEED;
        AniParams.sStartFrame = pNode->sCurrentFrame;
        AniParams.uiFlags = ANITILE_DOOR | ANITILE_BACKWARD | ANITILE_EXISTINGTILE;
        AniParams.pGivenLevelNode = pNode;

        AniParams.ubKeyFrame1 = pNode->sCurrentFrame - 2;

        CreateAnimationTile(&AniParams);
      } else {
        AniParams.sGridNo = sGridNo;
        AniParams.ubLevelID = ANI_STRUCT_LEVEL;
        AniParams.usTileType = (uint16_t)gTileDatabase[pNode->usIndex].fType;
        AniParams.usTileIndex = pNode->usIndex;
        AniParams.sDelay = INTTILE_DOOR_OPENSPEED;
        AniParams.sStartFrame = pNode->sCurrentFrame;
        AniParams.uiFlags = ANITILE_DOOR | ANITILE_FORWARD | ANITILE_EXISTINGTILE;
        AniParams.pGivenLevelNode = pNode;

        AniParams.ubKeyFrame1 = pNode->sCurrentFrame + 2;

        CreateAnimationTile(&AniParams);
      }
    }

    // if ( pShadowNode != NULL )
    //{
    //	pShadowNode->uiFlags |= LEVELNODE_ANIMATION;
    //	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_PLAYONCE;
    //	pShadowNode->uiFlags |= LEVELNODE_ANIMATION_BACKWARD;
    //	if ( pShadowNode->uiFlags & LEVELNODE_ANIMATION_FORWARD )
    //		pShadowNode->uiFlags ^= LEVELNODE_ANIMATION_FORWARD;
    //	pShadowNode->sDelay		= INTTILE_DOOR_OPENSPEED;
    //}
  }

  if (fDoAnimation) {
    gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
    // FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
    InvalidateWorldRedundency();

    if (GridNoOnScreen(sGridNo)) {
      SetRenderFlags(RENDER_FLAG_FULL);
    }
  }

  return (fDoAnimation);
}

void SetDoorString(int16_t sGridNo) {
  DOOR *pDoor;
  DOOR_STATUS *pDoorStatus;
  struct STRUCTURE *pStructure;

  BOOLEAN fTrapped = FALSE;

  // Try and get a door if one exists here
  pDoor = FindDoorInfoAtGridNo(sGridNo);

  if (gfUIIntTileLocation == FALSE) {
    if (pDoor == NULL) {
      wcscpy(gzIntTileLocation, TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);
      gfUIIntTileLocation = TRUE;
    } else {
      wcscpy(gzIntTileLocation, TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);
      gfUIIntTileLocation = TRUE;

      // CHECK PERCEIVED VALUE
      switch (pDoor->bPerceivedTrapped) {
        case DOOR_PERCEIVED_TRAPPED:

          wcscpy(gzIntTileLocation2, TacticalStr[DOOR_TRAPPED_MOUSE_DESCRIPTION]);
          gfUIIntTileLocation2 = TRUE;
          fTrapped = TRUE;
          break;
      }

      if (!fTrapped) {
        // CHECK PERCEIVED VALUE
        switch (pDoor->bPerceivedLocked) {
          case DOOR_PERCEIVED_UNKNOWN:

            break;

          case DOOR_PERCEIVED_LOCKED:

            wcscpy(gzIntTileLocation2, TacticalStr[DOOR_LOCKED_MOUSE_DESCRIPTION]);
            gfUIIntTileLocation2 = TRUE;
            break;

          case DOOR_PERCEIVED_UNLOCKED:

            wcscpy(gzIntTileLocation2, TacticalStr[DOOR_UNLOCKED_MOUSE_DESCRIPTION]);
            gfUIIntTileLocation2 = TRUE;
            break;

          case DOOR_PERCEIVED_BROKEN:

            wcscpy(gzIntTileLocation2, TacticalStr[DOOR_BROKEN_MOUSE_DESCRIPTION]);
            gfUIIntTileLocation2 = TRUE;
            break;
        }
      }
    }
  }

  // ATE: If here, we try to say, opened or closed...
  if (gfUIIntTileLocation2 == FALSE) {
    if (UsingGermanResources()) {
      wcscpy(gzIntTileLocation2, TacticalStr[DOOR_DOOR_MOUSE_DESCRIPTION]);
      gfUIIntTileLocation2 = TRUE;

      // Try to get doors status here...
      pDoorStatus = GetDoorStatus(sGridNo);
      if (pDoorStatus == NULL ||
          (pDoorStatus != NULL && pDoorStatus->ubFlags & DOOR_PERCEIVED_NOTSET)) {
        // OK, get status based on graphic.....
        pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
        if (pStructure) {
          if (pStructure->fFlags & STRUCTURE_OPEN) {
            // Door is opened....
            wcscpy(gzIntTileLocation, pMessageStrings[MSG_OPENED]);
            gfUIIntTileLocation = TRUE;
          } else {
            // Door is closed
            wcscpy(gzIntTileLocation, pMessageStrings[MSG_CLOSED]);
            gfUIIntTileLocation = TRUE;
          }
        }
      } else {
        // Use percived value
        if (pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN) {
          // Door is opened....
          wcscpy(gzIntTileLocation, pMessageStrings[MSG_OPENED]);
          gfUIIntTileLocation = TRUE;
        } else {
          // Door is closed
          wcscpy(gzIntTileLocation, pMessageStrings[MSG_CLOSED]);
          gfUIIntTileLocation = TRUE;
        }
      }
    } else {
      // Try to get doors status here...
      pDoorStatus = GetDoorStatus(sGridNo);
      if (pDoorStatus == NULL ||
          (pDoorStatus != NULL && pDoorStatus->ubFlags & DOOR_PERCEIVED_NOTSET)) {
        // OK, get status based on graphic.....
        pStructure = FindStructure(sGridNo, STRUCTURE_ANYDOOR);
        if (pStructure) {
          if (pStructure->fFlags & STRUCTURE_OPEN) {
            // Door is opened....
            wcscpy(gzIntTileLocation2, pMessageStrings[MSG_OPENED]);
            gfUIIntTileLocation2 = TRUE;
          } else {
            // Door is closed
            wcscpy(gzIntTileLocation2, pMessageStrings[MSG_CLOSED]);
            gfUIIntTileLocation2 = TRUE;
          }
        }
      } else {
        // Use percived value
        if (pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN) {
          // Door is opened....
          wcscpy(gzIntTileLocation2, pMessageStrings[MSG_OPENED]);
          gfUIIntTileLocation2 = TRUE;
        } else {
          // Door is closed
          wcscpy(gzIntTileLocation2, pMessageStrings[MSG_CLOSED]);
          gfUIIntTileLocation2 = TRUE;
        }
      }
    }
  }
}
