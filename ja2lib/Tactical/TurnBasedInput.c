// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Cheats.h"
#include "Editor/EditScreen.h"
#include "GameLoop.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "HelpScreen.h"
#include "JAScreens.h"
#include "Laptop/History.h"
#include "MessageBoxScreen.h"
#include "OptionsScreen.h"
#include "SGP/CursorControl.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Random.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "SaveLoadGame.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEvents.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/PreBattleInterface.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/QuestDebugSystem.h"
#include "Strategic/StrategicAI.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicStatus.h"
#include "SysGlobals.h"
#include "Tactical/AirRaid.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/AutoBandage.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/DisplayCover.h"
#include "Tactical/HandleUI.h"
#include "Tactical/HandleUIPlan.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceCursors.h"
#include "Tactical/InterfaceDialogue.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/InventoryChoosing.h"
#include "Tactical/LOS.h"
#include "Tactical/MercEntering.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/Points.h"
#include "Tactical/ShopKeeperInterface.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierFunctions.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/SoldierTile.h"
#include "Tactical/SpreadBurst.h"
#include "Tactical/Squads.h"
#include "Tactical/StrategicExitGUI.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/UICursors.h"
#include "Tactical/Vehicles.h"
#include "Tactical/Weapons.h"
#include "Tactical/WorldItems.h"
#include "TacticalAI/AI.h"
#include "TacticalAI/QuestDebug.h"
#include "TileEngine/AmbientControl.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/InteractiveTiles.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/OverheadMap.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

#ifdef __GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

extern UIKEYBOARD_HOOK gUIKeyboardHook;
extern BOOLEAN fRightButtonDown;
extern BOOLEAN fLeftButtonDown;
extern BOOLEAN fIgnoreLeftUp;
extern uint32_t guiCurrentEvent;
extern uint8_t gubIntTileCheckFlags;
extern uint32_t guiCurrentUICursor;
extern struct SOLDIERTYPE *gpSMCurrentMerc;
extern int16_t gsOverItemsGridNo;
extern int16_t gsOverItemsLevel;
extern BOOLEAN gfUIShowExitSouth;

extern BOOLEAN gfBeginBurstSpreadTracking;
extern BOOLEAN gfRTClickLeftHoldIntercepted;

extern BOOLEAN gfReportHitChances;

BOOLEAN gfFirstCycleMovementStarted = FALSE;

extern BOOLEAN gfNextShotKills;

uint32_t guiSoldierFlags;
uint32_t guiUITargetSoldierId = NOBODY;

void HandleTalkingMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent);
void HandleMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent);
void HandleItemMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent);
void HandleOpenDoorMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent);
void HandleSectorExitMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent);
void HandleStanceChangeFromUIKeys(uint8_t ubAnimHeight);

extern BOOLEAN ValidQuickExchangePosition();

BOOLEAN HandleUIReloading(struct SOLDIERTYPE *pSoldier);

extern struct SOLDIERTYPE *FindNextActiveSquad(struct SOLDIERTYPE *pSoldier);
extern struct SOLDIERTYPE *FindPrevActiveSquad(struct SOLDIERTYPE *pSoldier);
extern void ToggleItemGlow(BOOLEAN fOn);
extern void HandleTalkingMenuBackspace(void);
extern void BeginKeyPanelFromKeyShortcut();

extern int32_t iSMPanelButtons[NUM_SM_BUTTONS];
extern int32_t iTEAMPanelButtons[NUM_TEAM_BUTTONS];
extern int32_t giSMStealthButton;

struct SOLDIERTYPE *gpExchangeSoldier1;
struct SOLDIERTYPE *gpExchangeSoldier2;

BOOLEAN ConfirmActionCancel(uint16_t usMapPos, uint16_t usOldMapPos);

BOOLEAN gfNextFireJam = FALSE;

// Little functions called by keyboard input
void CreateRandomItem();
void MakeSelectedSoldierTired();
void ToggleRealTime(uint32_t *puiNewEvent);
void ToggleViewAllMercs();
void ToggleViewAllItems();
void TestExplosion();
void CycleSelectedMercsItem();
void ToggleWireFrame();
void RefreshSoldier();
void ChangeSoldiersBodyType(uint8_t ubBodyType, BOOLEAN fCreateNewPalette);
void TeleportSelectedSoldier();
void ToggleTreeTops();
void ToggleZBuffer();
void TogglePlanningMode();
void SetBurstMode();
void ObliterateSector();
void RandomizeMercProfile();
void JumpFence();
void CreateNextCivType();
void ToggleCliffDebug();
void CreateCow();
void CreatePlayerControlledCow();
void ToggleRealTimeConfirm();
void GrenadeTest1();
void GrenadeTest2();
void GrenadeTest3();
void TestMeanWhile(int32_t iID);
void CreatePlayerControlledMonster();
void ChangeCurrentSquad(int32_t iSquad);
void HandleSelectMercSlot(uint8_t ubPanelSlot, int8_t bCode);
void EscapeUILock();
void TestCapture();

#ifdef JA2BETAVERSION
void ToggleMapEdgepoints();
#endif
#ifdef JA2TESTVERSION
void ToggleMercsNeverQuit();
#endif
#ifdef JA2TESTVERSION
void DumpSectorDifficultyInfo(void);
#endif

void HandleStealthChangeFromUIKeys();

uint8_t gubCheatLevel = STARTING_CHEAT_LEVEL;

extern void DetermineWhichAssignmentMenusCanBeShown(void);

void GetTBMouseButtonInput(uint32_t *puiNewEvent) {
  QueryTBLeftButton(puiNewEvent);
  QueryTBRightButton(puiNewEvent);
}

void QueryTBLeftButton(uint32_t *puiNewEvent) {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  static BOOLEAN fClickHoldIntercepted = FALSE;
  static BOOLEAN fCanCheckForSpeechAdvance = FALSE;

  // LEFT MOUSE BUTTON
  if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    if (!GetMouseMapPos(&usMapPos) && !gfUIShowExitSouth) {
      return;
    }

    if (gViewportRegion.ButtonState & MSYS_LEFT_BUTTON) {
      if (!fLeftButtonDown) {
        fLeftButtonDown = TRUE;
        RESETCOUNTER(LMOUSECLICK_DELAY_COUNTER);

        {
          switch (gCurrentUIMode) {
            case CONFIRM_ACTION_MODE:

              if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
                pSoldier->sStartGridNo = usMapPos;
              }
              break;

            case MOVE_MODE:

              if (giUIMessageOverlay != -1) {
                EndUIMessage();
              } else {
                if (!HandleCheckForExitArrowsInput(FALSE) && gpItemPointer == NULL) {
                  // First check if we clicked on a guy, if so, make selected if it's ours
                  if (gfUIFullTargetFound && (guiUIFullTargetFlags & OWNED_MERC)) {
                    if (!(guiUIFullTargetFlags & UNCONSCIOUS_MERC)) {
                      fClickHoldIntercepted = TRUE;

                      // Select guy
                      if (GetSoldier(&pSoldier, gusUIFullTargetID) && gpItemPointer == NULL) {
                        if (pSoldier->bAssignment >= ON_DUTY) {
                          // do nothing
                          fClickHoldIntercepted = FALSE;
                        } else {
                          *puiNewEvent = I_SELECT_MERC;
                        }
                      } else {
                        *puiNewEvent = I_SELECT_MERC;
                      }
                    }
                  } else {
                    if (InUIPlanMode()) {
                      AddUIPlan(usMapPos, UIPLAN_ACTION_MOVETO);
                    } else {
                      // We're on terrain in which we can walk, walk
                      // If we're on terrain,
                      if (gusSelectedSoldier != NO_SOLDIER) {
                        int8_t bReturnVal = FALSE;

                        GetSoldier(&pSoldier, gusSelectedSoldier);

                        bReturnVal = HandleMoveModeInteractiveClick(usMapPos, puiNewEvent);

                        // All's OK for interactive tile?
                        if (bReturnVal == -2) {
                          // Confirm!
                          if (SelectedMercCanAffordMove()) {
                            *puiNewEvent = C_WAIT_FOR_CONFIRM;
                          }
                        } else if (bReturnVal == 0) {
                          if (gfUIAllMoveOn) {
                            *puiNewEvent = C_WAIT_FOR_CONFIRM;
                          } else {
                            if (gsCurrentActionPoints == 0) {
                              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                                        TacticalStr[NO_PATH]);
                            } else if (SelectedMercCanAffordMove()) {
                              BOOLEAN fResult;

                              if ((fResult = UIOKMoveDestination(MercPtrs[gusSelectedSoldier],
                                                                 usMapPos)) == 1) {
                                // ATE: CHECK IF WE CAN GET TO POSITION
                                // Check if we are not in combat
                                GetSoldier(&pSoldier, gusSelectedSoldier);

                                if (gsCurrentActionPoints == 0) {
                                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                                            TacticalStr[NO_PATH]);
                                } else {
                                  *puiNewEvent = C_WAIT_FOR_CONFIRM;
                                }
                              } else {
                                if (fResult == 2) {
                                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                                            TacticalStr[NOBODY_USING_REMOTE_STR]);
                                } else {
                                  // if ( usMapPos != sMoveClickGridNo || pSoldier->uiStatusFlags &
                                  // SOLDIER_ROBOT )
                                  //{
                                  //	sMoveClickGridNo					=
                                  // usMapPos;

                                  // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[
                                  // CANT_MOVE_THERE_STR ] );
                                  // Goto hand cursor mode....
                                  //	*puiNewEvent					  =
                                  // M_CHANGE_TO_HANDMODE;
                                  //	gsOverItemsGridNo				= usMapPos;
                                  //	gsOverItemsLevel				=
                                  // gsInterfaceLevel;
                                  //}
                                  // else
                                  //{
                                  //	sMoveClickGridNo = 0;
                                  //	*puiNewEvent = M_CHANGE_TO_HANDMODE;
                                  //}
                                }
                                // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"Invalid move
                                // destination." );
                              }
                            }
                          }
                        }
                        // OK, our first right-click is an all-cycle
                        gfUICanBeginAllMoveCycle = FALSE;
                      }
                      fClickHoldIntercepted = TRUE;
                    }
                  }
                } else {
                  fClickHoldIntercepted = TRUE;
                  fIgnoreLeftUp = TRUE;
                }
              }
              break;
          }
        }
        if (gfUIWaitingForUserSpeechAdvance) {
          fCanCheckForSpeechAdvance = TRUE;
        }
      }

      // HERE FOR CLICK-DRAG CLICK
      switch (gCurrentUIMode) {
        case MOVE_MODE:

          // First check if we clicked on a guy, if so, make selected if it's ours
          if (gfUIFullTargetFound) {
            // Select guy
            if ((guiUIFullTargetFlags & SELECTED_MERC) &&
                !(guiUIFullTargetFlags & UNCONSCIOUS_MERC) &&
                !(MercPtrs[gusUIFullTargetID]->uiStatusFlags & SOLDIER_VEHICLE)) {
              *puiNewEvent = M_CHANGE_TO_ADJPOS_MODE;
              fIgnoreLeftUp = FALSE;
            }
          }
          break;
      }

      // IF HERE, DO A CLICK-HOLD IF IN INTERVAL
      if (COUNTERDONE(LMOUSECLICK_DELAY_COUNTER) && !fClickHoldIntercepted) {
        // Switch on UI mode
        // switch( gCurrentUIMode )
        // {
        //
        // }
      }

    } else {
      if (fLeftButtonDown) {
        if (!fIgnoreLeftUp) {
          // FIRST CHECK FOR ANYTIME ( NON-INTERVAL ) CLICKS
          switch (gCurrentUIMode) {
            case ADJUST_STANCE_MODE:

              // If button has come up, change to mocve mode
              *puiNewEvent = PADJ_ADJUST_STANCE;
              break;
          }

          // CHECK IF WE CLICKED-HELD
          if (COUNTERDONE(LMOUSECLICK_DELAY_COUNTER)) {
            // LEFT CLICK-HOLD EVENT
            // Switch on UI mode
            switch (gCurrentUIMode) {
              case CONFIRM_ACTION_MODE:

                if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
                  if (pSoldier->bDoBurst) {
                    pSoldier->sEndGridNo = usMapPos;

                    gfBeginBurstSpreadTracking = FALSE;

                    if (pSoldier->sEndGridNo != pSoldier->sStartGridNo) {
                      pSoldier->fDoSpread = TRUE;

                      PickBurstLocations(pSoldier);

                      *puiNewEvent = CA_MERC_SHOOT;
                    } else {
                      pSoldier->fDoSpread = FALSE;
                    }

                    fClickHoldIntercepted = TRUE;
                  }
                }
                break;
            }
          }

          {
            // LEFT CLICK NORMAL EVENT
            // Switch on UI mode
            if (!fClickHoldIntercepted) {
              if (giUIMessageOverlay != -1) {
                EndUIMessage();
              } else {
                if (!HandleCheckForExitArrowsInput(TRUE)) {
                  if (gpItemPointer != NULL) {
                    if (HandleItemPointerClick(usMapPos)) {
                      // getout of mode
                      EndItemPointer();

                      *puiNewEvent = A_CHANGE_TO_MOVE;
                    }
                  } else {
                    // Check for wiating for keyboard advance
                    if (gfUIWaitingForUserSpeechAdvance && fCanCheckForSpeechAdvance) {
                      // We have a key, advance!
                      DialogueAdvanceSpeech();
                    } else {
                      switch (gCurrentUIMode) {
                        case MENU_MODE:

                          // If we get a hit here and we're in menu mode, quit the menu mode
                          EndMenuEvent(guiCurrentEvent);
                          break;

                        case IDLE_MODE:

                          // First check if we clicked on a guy, if so, make selected if it's ours
                          if (gfUIFullTargetFound) {
                            // Select guy
                            if (guiUIFullTargetFlags & OWNED_MERC &&
                                !(guiUIFullTargetFlags & UNCONSCIOUS_MERC)) {
                              *puiNewEvent = I_SELECT_MERC;
                            }
                          }
                          break;

                        case MOVE_MODE:

                          // Check if we should activate an interactive tile!
                          // Select guy
                          if ((guiUIFullTargetFlags & OWNED_MERC) &&
                              !(guiUIFullTargetFlags & UNCONSCIOUS_MERC)) {
                            // Select guy
                            if (GetSoldier(&pSoldier, gusUIFullTargetID) &&
                                (gpItemPointer == NULL) &&
                                !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
                              if (pSoldier->bAssignment >= ON_DUTY) {
                                PopupAssignmentMenuInTactical(pSoldier);
                              }
                            }
                          }
                          break;

                        case CONFIRM_MOVE_MODE:

                          *puiNewEvent = C_MOVE_MERC;
                          break;

                        case HANDCURSOR_MODE:

                          HandleHandCursorClick(usMapPos, puiNewEvent);
                          break;

                        case JUMPOVER_MODE:

                          if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
                            if (EnoughPoints(pSoldier, gsCurrentActionPoints, 0, TRUE)) {
                              *puiNewEvent = JP_JUMP;
                            }
                          }
                          break;

                        case ACTION_MODE:

                          if (InUIPlanMode()) {
                            AddUIPlan(usMapPos, UIPLAN_ACTION_FIRE);
                          } else {
                            if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
                              if (!HandleUIReloading(pSoldier)) {
                                // ATE: Reset refine aim..
                                pSoldier->bShownAimTime = 0;

                                if (gsCurrentActionPoints == 0) {
                                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                                            TacticalStr[NO_PATH]);
                                }
                                // Determine if we have enough action points!
                                else if (UIMouseOnValidAttackLocation(pSoldier) &&
                                         SelectedMercCanAffordAttack()) {
                                  *puiNewEvent = A_CHANGE_TO_CONFIM_ACTION;
                                  pSoldier->sStartGridNo = usMapPos;
                                }
                              }
                            }
                          }
                          break;

                        case CONFIRM_ACTION_MODE:

                          *puiNewEvent = CA_MERC_SHOOT;
                          break;

                        case LOOKCURSOR_MODE:
                          // If we cannot actually do anything, return to movement mode
                          *puiNewEvent = LC_LOOK;
                          break;

                        case TALKCURSOR_MODE:

                          if (HandleTalkInit()) {
                            *puiNewEvent = TA_TALKINGMENU;
                          }
                          break;

                        case GETTINGITEM_MODE:

                          // Remove menu!
                          // RemoveItemPickupMenu( );
                          break;

                        case TALKINGMENU_MODE:

                          // HandleTalkingMenuEscape( TRUE );
                          break;

                        case EXITSECTORMENU_MODE:

                          RemoveSectorExitMenu(FALSE);
                          break;

                        case OPENDOOR_MENU_MODE:

                          CancelOpenDoorMenu();
                          HandleOpenDoorMenu();
                          *puiNewEvent = A_CHANGE_TO_MOVE;
                          break;
                      }
                    }
                  }
                }
              }
            }
          }
        }

        // Reset flag
        fLeftButtonDown = FALSE;
        fIgnoreLeftUp = FALSE;
        fClickHoldIntercepted = FALSE;
        fCanCheckForSpeechAdvance = FALSE;
        gfFirstCycleMovementStarted = FALSE;

        // Reset counter
        RESETCOUNTER(LMOUSECLICK_DELAY_COUNTER);
      }
    }

  } else {
    // Set mouse down to false
    // fLeftButtonDown = FALSE;

    // OK, handle special cases like if we are dragging and holding for a burst spread and
    // release mouse over another mouse region
    if (gfBeginBurstSpreadTracking) {
      if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
        pSoldier->fDoSpread = FALSE;
      }
      gfBeginBurstSpreadTracking = FALSE;
    }
  }
}

void QueryTBRightButton(uint32_t *puiNewEvent) {
  static BOOLEAN fClickHoldIntercepted = FALSE;
  static BOOLEAN fClickIntercepted = FALSE;
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  BOOLEAN fDone = FALSE;
  if (!GetMouseMapPos(&usMapPos)) {
    return;
  }

  if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    // RIGHT MOUSE BUTTON
    if (gViewportRegion.ButtonState & MSYS_RIGHT_BUTTON) {
      if (!fRightButtonDown) {
        fRightButtonDown = TRUE;
        RESETCOUNTER(RMOUSECLICK_DELAY_COUNTER);
      }

      // CHECK COMBINATIONS
      if (fLeftButtonDown) {
      } else {
        if (gpItemPointer == NULL) {
          // IF HERE, DO A CLICK-HOLD IF IN INTERVAL
          if (COUNTERDONE(RMOUSECLICK_DELAY_COUNTER) && !fClickHoldIntercepted) {
            // Switch on UI mode
            switch (gCurrentUIMode) {
              case IDLE_MODE:
              case ACTION_MODE:
              case HANDCURSOR_MODE:
              case LOOKCURSOR_MODE:
              case TALKCURSOR_MODE:
              case MOVE_MODE:

                // Check if we're on terrain
                // if ( !gfUIFullTargetFound )
                //{
                // ATE:
                fDone = FALSE;

                if ((guiUIFullTargetFlags & OWNED_MERC) &&
                    !(guiUIFullTargetFlags & UNCONSCIOUS_MERC)) {
                  // Select guy
                  if (GetSoldier(&pSoldier, gusUIFullTargetID) && (gpItemPointer == NULL) &&
                      !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE)) {
                    // if( pSoldier->bAssignment >= ON_DUTY )
                    {
                      PopupAssignmentMenuInTactical(pSoldier);
                      fClickHoldIntercepted = TRUE;
                    }
                  }
                }

                if (fDone == TRUE) {
                  break;
                }

                if (gusSelectedSoldier != NOBODY && !fClickHoldIntercepted) {
                  *puiNewEvent = U_MOVEMENT_MENU;
                  fClickHoldIntercepted = TRUE;
                }
                //}
                // else
                //{
                // If we are on a selected guy
                //	if ( guiUIFullTargetFlags & SELECTED_MERC && !( guiUIFullTargetFlags &
                // UNCONSCIOUS_MERC ) )
                //	{
                //*puiNewEvent = U_POSITION_MENU;
                // fClickHoldIntercepted = TRUE;
                //	}
                //		else if ( guiUIFullTargetFlags & OWNED_MERC )
                //		{
                // If we are on a non-selected guy selected guy
                //		}

                //}
                break;
            }
          }
        }
      }

    } else {
      if (fRightButtonDown) {
        if (fLeftButtonDown) {
          fIgnoreLeftUp = TRUE;

          if (gpItemPointer == NULL) {
            // ATE:
            if (gusSelectedSoldier != NOBODY) {
              switch (gCurrentUIMode) {
                case CONFIRM_MOVE_MODE:
                case MOVE_MODE:

                  if (gfUICanBeginAllMoveCycle) {
                    *puiNewEvent = M_CYCLE_MOVE_ALL;
                  } else {
                    if (!gfFirstCycleMovementStarted) {
                      gfFirstCycleMovementStarted = TRUE;

                      // OK, set this guy's movement mode to crawling fo rthat we will start cycling
                      // in run.....
                      if (MercPtrs[gusSelectedSoldier]->usUIMovementMode != RUNNING) {
                        // ATE: UNLESS WE ARE IN RUNNING MODE ALREADY
                        MercPtrs[gusSelectedSoldier]->usUIMovementMode = CRAWLING;
                      }
                    }

                    // Give event to cycle movement
                    *puiNewEvent = M_CYCLE_MOVEMENT;
                    break;
                  }
              }

              // ATE: Added cancel of burst mode....
              if (gfBeginBurstSpreadTracking) {
                gfBeginBurstSpreadTracking = FALSE;
                gfRTClickLeftHoldIntercepted = TRUE;
                MercPtrs[gusSelectedSoldier]->fDoSpread = FALSE;
                fClickHoldIntercepted = TRUE;
                *puiNewEvent = A_END_ACTION;
                gCurrentUIMode = MOVE_MODE;
              }
            }
          }
        } else {
          if (!fClickHoldIntercepted && !fClickIntercepted) {
            if (gpItemPointer == NULL) {
              // ATE:
              if (gusSelectedSoldier != NOBODY) {
                // Switch on UI mode
                switch (gCurrentUIMode) {
                  case IDLE_MODE:

                    break;

                  case MOVE_MODE:

                    // We have here a change to action mode
                    *puiNewEvent = M_CHANGE_TO_ACTION;
                    fClickIntercepted = TRUE;
                    break;

                  case ACTION_MODE:

                    // We have here a change to action mode
                    *puiNewEvent = A_CHANGE_TO_MOVE;
                    fClickIntercepted = TRUE;
                    break;

                  case CONFIRM_MOVE_MODE:

                    *puiNewEvent = A_CHANGE_TO_MOVE;
                    fClickIntercepted = TRUE;
                    break;

                  case HANDCURSOR_MODE:
                    // If we cannot actually do anything, return to movement mode
                    *puiNewEvent = A_CHANGE_TO_MOVE;
                    break;

                  case LOOKCURSOR_MODE:
                  case TALKCURSOR_MODE:

                    // If we cannot actually do anything, return to movement mode
                    *puiNewEvent = A_CHANGE_TO_MOVE;
                    break;

                  case CONFIRM_ACTION_MODE:

                    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
                      HandleRightClickAdjustCursor(pSoldier, usMapPos);
                    }
                    fClickIntercepted = TRUE;
                    break;

                  case MENU_MODE:

                    // If we get a hit here and we're in menu mode, quit the menu mode
                    EndMenuEvent(guiCurrentEvent);
                    fClickIntercepted = TRUE;
                    break;
                }
              }
            } else {
              if (gfUIFullTargetFound) {
                gfItemPointerDifferentThanDefault = !gfItemPointerDifferentThanDefault;
              }
            }
          }
        }
      }
      // Reset flag
      fRightButtonDown = FALSE;
      fClickHoldIntercepted = FALSE;
      fClickIntercepted = FALSE;

      // Reset counter
      RESETCOUNTER(RMOUSECLICK_DELAY_COUNTER);
    }
  }
}

extern BOOLEAN gUIActionModeChangeDueToMouseOver;

void GetTBMousePositionInput(uint32_t *puiNewEvent) {
  int16_t usMapPos;
  static uint16_t usOldMapPos = 0;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN bHandleCode;
  static BOOLEAN fOnValidGuy = FALSE;
  static uint32_t uiMoveTargetSoldierId = NO_SOLDIER;

  if (!GetMouseMapPos(&usMapPos)) {
    return;
  }

  if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA) {
    // Handle highlighting stuff
    HandleObjectHighlighting();

    // Check if we have an item in our hands...
    if (gpItemPointer != NULL) {
      *puiNewEvent = A_ON_TERRAIN;
      return;
    }

    // Switch on modes
    switch (gCurrentUIMode) {
      case LOCKUI_MODE:
        *puiNewEvent = LU_ON_TERRAIN;
        break;

      case LOCKOURTURN_UI_MODE:
        *puiNewEvent = LA_ON_TERRAIN;
        break;

      case IDLE_MODE:
        *puiNewEvent = I_ON_TERRAIN;
        break;

      case ENEMYS_TURN_MODE:
        *puiNewEvent = ET_ON_TERRAIN;
        break;

      case LOOKCURSOR_MODE:
        *puiNewEvent = LC_ON_TERRAIN;
        break;

      case TALKCURSOR_MODE:
        if (uiMoveTargetSoldierId != NOBODY) {
          if (gfUIFullTargetFound) {
            if (gusUIFullTargetID != uiMoveTargetSoldierId) {
              *puiNewEvent = A_CHANGE_TO_MOVE;
              return;
            }
          } else {
            *puiNewEvent = A_CHANGE_TO_MOVE;
            return;
          }
        }
        *puiNewEvent = T_ON_TERRAIN;
        break;

      case MOVE_MODE:

        uiMoveTargetSoldierId = NO_SOLDIER;

        // Check for being on terrain
        if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
          if (IsValidJumpLocation(pSoldier, usMapPos, TRUE)) {
            gsJumpOverGridNo = usMapPos;
            *puiNewEvent = JP_ON_TERRAIN;
            return;
          } else {
            if (gfUIFullTargetFound) {
              // ATE: Don't do this automatically for enemies......
              if (MercPtrs[gusUIFullTargetID]->bTeam != ENEMY_TEAM) {
                uiMoveTargetSoldierId = gusUIFullTargetID;
                if (IsValidTalkableNPC((uint8_t)gusUIFullTargetID, FALSE, FALSE, FALSE) &&
                    !_KeyDown(SHIFT) && !AM_AN_EPC(pSoldier) && !ValidQuickExchangePosition()) {
                  *puiNewEvent = T_CHANGE_TO_TALKING;
                  return;
                }
              }
            }
          }
        }
        *puiNewEvent = M_ON_TERRAIN;
        break;

      case ACTION_MODE:

        // First check if we are on a guy, if so, make selected if it's ours
        // Check if the guy is visible
        guiUITargetSoldierId = NOBODY;

        fOnValidGuy = FALSE;

        if (gfUIFullTargetFound)
        // if ( gfUIFullTargetFound )
        {
          if (IsValidTargetMerc((uint8_t)gusUIFullTargetID)) {
            guiUITargetSoldierId = gusUIFullTargetID;

            if (MercPtrs[gusUIFullTargetID]->bTeam != gbPlayerNum) {
              fOnValidGuy = TRUE;
            } else {
              if (gUIActionModeChangeDueToMouseOver) {
                *puiNewEvent = A_CHANGE_TO_MOVE;
                return;
              }
            }
          }
        } else {
          if (gUIActionModeChangeDueToMouseOver) {
            *puiNewEvent = A_CHANGE_TO_MOVE;
            return;
          }
        }
        *puiNewEvent = A_ON_TERRAIN;
        break;

      case GETTINGITEM_MODE:

        break;

      case TALKINGMENU_MODE:

        if (HandleTalkingMenu()) {
          *puiNewEvent = A_CHANGE_TO_MOVE;
        }
        break;

      case EXITSECTORMENU_MODE:

        if (HandleSectorExitMenu()) {
          *puiNewEvent = A_CHANGE_TO_MOVE;
        }
        break;

      case OPENDOOR_MENU_MODE:

        if ((bHandleCode = HandleOpenDoorMenu())) {
          // OK, IF we are not canceling, set ui back!
          if (bHandleCode == 2) {
            *puiNewEvent = A_CHANGE_TO_MOVE;
          } else {
          }
        }
        break;

      case JUMPOVER_MODE:

        // ATE: Make sure!
        if (gsJumpOverGridNo != usMapPos) {
          *puiNewEvent = A_CHANGE_TO_MOVE;
        } else {
          *puiNewEvent = JP_ON_TERRAIN;
        }
        break;

      case CONFIRM_MOVE_MODE:

        if (usMapPos != usOldMapPos) {
          // Switch event out of confirm mode
          *puiNewEvent = A_CHANGE_TO_MOVE;

          // Set off ALL move....
          gfUIAllMoveOn = FALSE;

          // ERASE PATH
          ErasePath(TRUE);
        }
        break;

      case CONFIRM_ACTION_MODE:

        // DONOT CANCEL IF BURST
        if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
          if (pSoldier->bDoBurst) {
            pSoldier->sEndGridNo = usMapPos;

            if (pSoldier->sEndGridNo != pSoldier->sStartGridNo && fLeftButtonDown) {
              pSoldier->fDoSpread = TRUE;
              gfBeginBurstSpreadTracking = TRUE;
            }

            if (pSoldier->fDoSpread) {
              // Accumulate gridno
              AccumulateBurstLocation(usMapPos);

              *puiNewEvent = CA_ON_TERRAIN;
              break;
            }
          }
        }

        // First check if we are on a guy, if so, make selected if it's ours
        if (gfUIFullTargetFound) {
          if (guiUITargetSoldierId != gusUIFullTargetID) {
            // Switch event out of confirm mode
            *puiNewEvent = CA_END_CONFIRM_ACTION;
          } else {
            *puiNewEvent = CA_ON_TERRAIN;
          }
        } else {
          // OK, if we were on a guy, and now we are off, go back!
          if (fOnValidGuy) {
            // Switch event out of confirm mode
            *puiNewEvent = CA_END_CONFIRM_ACTION;
          } else {
            if (ConfirmActionCancel(usMapPos, usOldMapPos)) {
              // Switch event out of confirm mode
              *puiNewEvent = CA_END_CONFIRM_ACTION;
            } else {
              *puiNewEvent = CA_ON_TERRAIN;
            }
          }
        }
        break;

      case HANDCURSOR_MODE:

        *puiNewEvent = HC_ON_TERRAIN;
        break;
    }

    usOldMapPos = usMapPos;
  }
}

void GetPolledKeyboardInput(uint32_t *puiNewEvent) {
  static BOOLEAN fShifted2 = FALSE;
  static BOOLEAN fCtrlDown = FALSE;
  static BOOLEAN fAltDown = FALSE;
  static BOOLEAN fDeleteDown = FALSE;
  static BOOLEAN fEndDown = FALSE;

  // CHECK FOR POLLED KEYS!!
  // CHECK FOR CTRL
  switch (gCurrentUIMode) {
    case DONT_CHANGEMODE:
    case CONFIRM_MOVE_MODE:
    case CONFIRM_ACTION_MODE:
    case LOOKCURSOR_MODE:
    case TALKCURSOR_MODE:
    case IDLE_MODE:
    case MOVE_MODE:
    case ACTION_MODE:
    case HANDCURSOR_MODE:

      if (_KeyDown(CTRL)) {
        if (fCtrlDown == FALSE) {
          ErasePath(TRUE);
          gfPlotNewMovement = TRUE;
        }
        fCtrlDown = TRUE;
        *puiNewEvent = HC_ON_TERRAIN;
      }
      if (!(_KeyDown(CTRL)) && fCtrlDown) {
        fCtrlDown = FALSE;
        *puiNewEvent = M_ON_TERRAIN;
        gfPlotNewMovement = TRUE;
      }
      break;
  }

  // CHECK FOR ALT
  switch (gCurrentUIMode) {
    case MOVE_MODE:

      if (_KeyDown(ALT)) {
        if (fAltDown == FALSE) {
          // Get currently selected guy and change reverse....
          if (gusSelectedSoldier != NOBODY) {
            gUIUseReverse = TRUE;
            ErasePath(TRUE);
            gfPlotNewMovement = TRUE;
          }
        }
        fAltDown = TRUE;
      }
      if (!(_KeyDown(ALT)) && fAltDown) {
        if (gusSelectedSoldier != NOBODY) {
          gUIUseReverse = FALSE;
          ErasePath(TRUE);
          gfPlotNewMovement = TRUE;
        }

        fAltDown = FALSE;
      }
      break;
  }

  // Check realtime input!
  if (((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT))) {
    // if ( _KeyDown( CAPS )  ) //&& !fShifted )
    //{
    //	fShifted = TRUE;
    //	if ( gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE )
    //	{
    //		*puiNewEvent = CA_ON_TERRAIN;
    //	}
    //}
    // if ( !(_KeyDown( CAPS ) ) && fShifted )
    //{
    //	fShifted = FALSE;
    //	{
    //		*puiNewEvent = M_ON_TERRAIN;
    //	}
    //}

    if (_KeyDown(SHIFT))  //&& !fShifted )
    {
      fShifted2 = TRUE;
      if (gCurrentUIMode != MOVE_MODE && gCurrentUIMode != CONFIRM_MOVE_MODE) {
        // puiNewEvent = M_ON_TERRAIN;
      }
    }
    if (!(_KeyDown(SHIFT)) && fShifted2) {
      fShifted2 = FALSE;
      if (gCurrentUIMode != ACTION_MODE && gCurrentUIMode != CONFIRM_ACTION_MODE) {
        //	*puiNewEvent = A_ON_TERRAIN;
      }
    }
  }

  if (_KeyDown(DEL)) {
    DisplayCoverOfSelectedGridNo();

    fDeleteDown = TRUE;
  }

  if (!_KeyDown(DEL) && fDeleteDown) {
    RemoveCoverOfSelectedGridNo();

    fDeleteDown = FALSE;
  }

  if (_KeyDown(END)) {
    DisplayGridNoVisibleToSoldierGrid();

    fEndDown = TRUE;
  }

  if (!_KeyDown(END) && fEndDown) {
    RemoveVisibleGridNoAtSelectedGridNo();

    fEndDown = FALSE;
  }
}

extern BOOLEAN gfDisableRegionActive;
extern BOOLEAN gfUserTurnRegionActive;

void GetKeyboardInput(uint32_t *puiNewEvent) {
  InputAtom InputEvent;
  BOOLEAN fKeyTaken = FALSE;
  int16_t usMapPos;
  BOOLEAN fGoodCheatLevelKey = FALSE;

  struct Point MousePos = GetMousePoint();

  GetMouseMapPos(&usMapPos);

  while (DequeueEvent(&InputEvent) == TRUE) {
    // HOOK INTO MOUSE HOOKS
    switch (InputEvent.usEvent) {
      case LEFT_BUTTON_DOWN:
        MouseSystemHook(LEFT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case LEFT_BUTTON_UP:
        MouseSystemHook(LEFT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case RIGHT_BUTTON_DOWN:
        MouseSystemHook(RIGHT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case RIGHT_BUTTON_UP:
        MouseSystemHook(RIGHT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
    }

    // handle for fast help text for interface stuff
    if (IsTheInterfaceFastHelpTextActive()) {
      if (InputEvent.usEvent == KEY_UP) {
        ShutDownUserDefineHelpTextRegions();
      }

      continue;
    }

    // Check for waiting for keyboard advance
    if (gfUIWaitingForUserSpeechAdvance && InputEvent.usEvent == KEY_UP) {
      // We have a key, advance!
      DialogueAdvanceSpeech();

      // Ignore anything else
      continue;
    }

    // ATE: if game paused because fo player, unpasue with any key
    if (gfPauseDueToPlayerGamePause && InputEvent.usEvent == KEY_UP) {
      HandlePlayerPauseUnPauseOfGame();

      continue;
    }

    if ((InputEvent.usEvent == KEY_DOWN)) {
      if (giUIMessageOverlay != -1) {
        EndUIMessage();
        continue;
      }

      // End auto bandage if we want....
      if (gTacticalStatus.fAutoBandageMode) {
        AutoBandage(FALSE);
        *puiNewEvent = LU_ENDUILOCK;
      }
    }

    if (gUIKeyboardHook != NULL) {
      fKeyTaken = gUIKeyboardHook(&InputEvent);
    }
    if (fKeyTaken) {
      continue;
    }

    /*
    if( (InputEvent.usEvent == KEY_DOWN )&& ( InputEvent.usParam == ) )
    {
            HandlePlayerPauseUnPauseOfGame( );
    }
    */

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == PAUSE) &&
        !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
      // Pause game!
      HandlePlayerPauseUnPauseOfGame();
    }

    // FIRST DO KEYS THAT ARE USED EVERYWHERE!
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == 'x') &&
        (InputEvent.usKeyState & ALT_DOWN)) {
      HandleShortCutExitState();
      //*puiNewEvent = I_EXIT;
    }

    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == ESC)) {
      if (AreInMeanwhile() && gCurrentMeanwhileDef.ubMeanwhileID != INTERROGATION) {
        DeleteTalkingMenu();
        EndMeanwhile();
      }
    }

    // Break of out IN CONV...
    if (CHEATER_CHEAT_LEVEL()) {
      if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ENTER) &&
          (InputEvent.usKeyState & ALT_DOWN)) {
        if (gTacticalStatus.uiFlags & ENGAGED_IN_CONV) {
          gTacticalStatus.uiFlags &= (~ENGAGED_IN_CONV);
          giNPCReferenceCount = 0;
        }
      }
    }

    if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
      {
        if (gTacticalStatus.ubCurrentTeam != gbPlayerNum) {
          if (CHEATER_CHEAT_LEVEL()) {
            if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ENTER) &&
                (InputEvent.usKeyState & ALT_DOWN)) {
              // ESCAPE ENEMY'S TURN
              EndAIDeadlock();

              // Decrease global busy  counter...
              gTacticalStatus.ubAttackBusyCount = 0;

              guiPendingOverrideEvent = LU_ENDUILOCK;
              UIHandleLUIEndLock(NULL);
            }
            if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ENTER) &&
                (InputEvent.usKeyState & CTRL_DOWN)) {
              EscapeUILock();
            }
          }
        } else {
          if (CHEATER_CHEAT_LEVEL() && (InputEvent.usEvent == KEY_DOWN) &&
              (InputEvent.usParam == ENTER) && (InputEvent.usKeyState & CTRL_DOWN)) {
            // UNLOCK UI
            EscapeUILock();
          } else if ((InputEvent.usEvent == KEY_DOWN) && InputEvent.usParam == ENTER) {
            // Cycle through enemys
            CycleThroughKnownEnemies();
          }
        }
      }
    }

    if (gfInTalkPanel) {
      HandleTalkingMenuKeys(&InputEvent, puiNewEvent);
    }

    // Do some checks based on what mode we are in
    switch (gCurrentUIMode) {
      case EXITSECTORMENU_MODE:

        HandleSectorExitMenuKeys(&InputEvent, puiNewEvent);
        continue;

      case GETTINGITEM_MODE:

        HandleItemMenuKeys(&InputEvent, puiNewEvent);
        continue;

      case MENU_MODE:

        HandleMenuKeys(&InputEvent, puiNewEvent);
        continue;

      case OPENDOOR_MENU_MODE:

        HandleOpenDoorMenuKeys(&InputEvent, puiNewEvent);
        continue;
    }

    // CHECK ESC KEYS HERE....
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == ESC)) {
      // EscapeUILock( );
#ifdef JA2TESTVERSION
      if (InAirRaid()) {
        EndAirRaid();
      }
#endif

      // Cancel out of spread burst...
      gfBeginBurstSpreadTracking = FALSE;
      gfRTClickLeftHoldIntercepted = TRUE;
      if (gusSelectedSoldier != NO_SOLDIER) {
        MercPtrs[gusSelectedSoldier]->fDoSpread = FALSE;
      }

      // Befone anything, delete popup box!
      EndUIMessage();

      // CANCEL FROM PLANNING MODE!
      if (InUIPlanMode()) {
        EndUIPlan();
      }

      if (InItemDescriptionBox()) {
        DeleteItemDescriptionBox();
      } else if (InKeyRingPopup()) {
        DeleteKeyRingPopup();
      }

      if (gCurrentUIMode == MENU_MODE) {
        // If we get a hit here and we're in menu mode, quit the menu mode
        EndMenuEvent(guiCurrentEvent);
      }

      if (gCurrentUIMode == HANDCURSOR_MODE) {
        *puiNewEvent = A_CHANGE_TO_MOVE;
      }

      if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
        if (gusSelectedSoldier != NO_SOLDIER) {
          // If soldier is not stationary, stop
          StopSoldier(MercPtrs[gusSelectedSoldier]);
          *puiNewEvent = A_CHANGE_TO_MOVE;
        }
        // ATE: OK, stop any mercs who are moving by selection method....
        StopRubberBandedMercFromMoving();
      }
    }

    // CHECK ESC KEYS HERE....
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == BACKSPACE)) {
      StopAnyCurrentlyTalkingSpeech();
    }

    // IF UI HAS LOCKED, ONLY ALLOW EXIT!
    if (gfDisableRegionActive || gfUserTurnRegionActive) {
      continue;
    }

    // Check all those we want if enemy's turn
    if ((InputEvent.usEvent == KEY_UP) && (InputEvent.usParam == 'q')) {
      if (InputEvent.usKeyState & ALT_DOWN) {
        if (CHEATER_CHEAT_LEVEL()) {
          static BOOLEAN fShowRoofs = TRUE;
          int32_t x;
          uint16_t usType;

          // Toggle removal of roofs...
          fShowRoofs = !fShowRoofs;

          for (x = 0; x < WORLD_MAX; x++) {
            for (usType = FIRSTROOF; usType <= LASTSLANTROOF; usType++) {
              HideStructOfGivenType(x, usType, (BOOLEAN)(!fShowRoofs));
            }
          }
          InvalidateWorldRedundency();
          SetRenderFlags(RENDER_FLAG_FULL);
        }

      } else if (InputEvent.usKeyState & CTRL_DOWN) {
#ifdef JA2BETAVERSION
        if (CHEATER_CHEAT_LEVEL()) {
          LeaveTacticalScreen(ANIEDIT_SCREEN);
        }
#endif
      } else {
        if (INFORMATION_CHEAT_LEVEL()) {
          *puiNewEvent = I_SOLDIERDEBUG;
        }
      }
    }

#ifdef JA2TESTVERSION
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == '0') &&
        (InputEvent.usKeyState & ALT_DOWN)) {
      int32_t i = 0;
      int16_t sGridNo;
      int32_t iTime = GetJA2Clock();
      int8_t ubLevel;

      for (i = 0; i < 1000; i++) {
        CalculateLaunchItemChanceToGetThrough(MercPtrs[gusSelectedSoldier],
                                              &(MercPtrs[gusSelectedSoldier]->inv[HANDPOS]),
                                              usMapPos, 0, 0, &sGridNo, TRUE, &ubLevel, TRUE);
      }

      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Physics 100 times: %d",
                (GetJA2Clock() - iTime));
    }
#endif

#ifdef NETWORKED
    // DEF: Test Networking
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == '0') &&
        (InputEvent.usKeyState & ALT_DOWN)) {
      DisplayMultiPlayerInfo();
    }
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == '9') &&
        (InputEvent.usKeyState & ALT_DOWN)) {
      DisplayDirectPlayInfo();
    }
    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == '8') &&
        (InputEvent.usKeyState & ALT_DOWN)) {
      DisplayDirectPlayPlayerInfo();
    }

    if ((InputEvent.usEvent == KEY_DOWN) && (InputEvent.usParam == '7') &&
        (InputEvent.usKeyState & ALT_DOWN)) {
      SetDisplayFlag();
    }
#endif

    if (InputEvent.usEvent == KEY_DOWN) {
      BOOLEAN fAlt, fCtrl, fShift;
      fAlt = InputEvent.usKeyState & ALT_DOWN ? TRUE : FALSE;
      fCtrl = InputEvent.usKeyState & CTRL_DOWN ? TRUE : FALSE;
      fShift = InputEvent.usKeyState & SHIFT_DOWN ? TRUE : FALSE;
      switch (InputEvent.usParam) {
        case SPACE:

          // nothing in hand and either not in SM panel, or the matching button is enabled if we are
          // in SM panel
          if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV) &&
              ((gsCurInterfacePanel != SM_PANEL) ||
               (ButtonList[iSMPanelButtons[NEXTMERC_BUTTON]]->uiFlags & BUTTON_ENABLED))) {
            if (!InKeyRingPopup()) {
              if (_KeyDown(SHIFT)) {
                struct SOLDIERTYPE *pNewSoldier;
                int32_t iCurrentSquad;

                if (gusSelectedSoldier != NO_SOLDIER) {
                  // only allow if nothing in hand and if in SM panel, the Change Squad button must
                  // be enabled
                  if (((gsCurInterfacePanel != TEAM_PANEL) ||
                       (ButtonList[iTEAMPanelButtons[CHANGE_SQUAD_BUTTON]]->uiFlags &
                        BUTTON_ENABLED))) {
                    // Select next squad
                    iCurrentSquad = CurrentSquad();

                    pNewSoldier = FindNextActiveSquad(MercPtrs[gusSelectedSoldier]);

                    if (pNewSoldier->bAssignment != iCurrentSquad) {
                      HandleLocateSelectMerc(pNewSoldier->ubID, LOCATEANDSELECT_MERC);

                      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
                                pMessageStrings[MSG_SQUAD_ACTIVE], (CurrentSquad() + 1));

                      // Center to guy....
                      LocateSoldier(gusSelectedSoldier, SETLOCATOR);
                    }
                  }
                }
              } else {
                if (gusSelectedSoldier != NO_SOLDIER) {  // Select next merc
                  uint8_t bID;

                  bID = FindNextMercInTeamPanel(MercPtrs[gusSelectedSoldier], FALSE, FALSE);
                  HandleLocateSelectMerc(bID, LOCATEANDSELECT_MERC);

                  // Center to guy....
                  LocateSoldier(gusSelectedSoldier, SETLOCATOR);
                }
              }

              *puiNewEvent = M_ON_TERRAIN;
            }
          }
          break;
        case TAB:
          // nothing in hand and either not in SM panel, or the matching button is enabled if we are
          // in SM panel
          if ((gpItemPointer == NULL) &&
              ((gsCurInterfacePanel != SM_PANEL) ||
               (ButtonList[iSMPanelButtons[UPDOWN_BUTTON]]->uiFlags & BUTTON_ENABLED))) {
            UIHandleChangeLevel(NULL);

            if (gsCurInterfacePanel == SM_PANEL) {
              // Remember soldier's new value
              gpSMCurrentMerc->bUIInterfaceLevel = (int8_t)gsInterfaceLevel;
            }
          }
          break;

        case F1:
          if (fShift) {
            HandleSelectMercSlot(0, LOCATE_MERC_ONCE);
          }
#ifdef JA2TESTVERSION
          else if (fAlt) {
            TestMeanWhile(15);
          } else if (fCtrl) {
            TestMeanWhile(10);
          }
#endif
          else
            HandleSelectMercSlot(0, LOCATEANDSELECT_MERC);
          break;
        case F2:
          if (fShift) HandleSelectMercSlot(1, LOCATE_MERC_ONCE);
#ifdef JA2TESTVERSION
          else if (fAlt) {
            TestMeanWhile(1);
          } else if (fCtrl) {
            TestMeanWhile(11);
          }
#endif
          else
            HandleSelectMercSlot(1, LOCATEANDSELECT_MERC);
          break;
        case F3:
          if (fShift) HandleSelectMercSlot(2, LOCATE_MERC_ONCE);
#ifdef JA2TESTVERSION
          else if (fAlt) {
            TestMeanWhile(2);
          } else if (fCtrl) {
            TestMeanWhile(12);
          }
#endif
          else
            HandleSelectMercSlot(2, LOCATEANDSELECT_MERC);
          break;
        case F4:
          if (fShift) HandleSelectMercSlot(3, LOCATE_MERC_ONCE);
#ifdef JA2TESTVERSION
          else if (fAlt) {
            TestMeanWhile(3);
          } else if (fCtrl) {
            TestMeanWhile(13);
          }
#endif
          else
            HandleSelectMercSlot(3, LOCATEANDSELECT_MERC);
          break;
        case F5:
          if (fShift) HandleSelectMercSlot(4, LOCATE_MERC_ONCE);
#ifdef JA2TESTVERSION
          else if (fAlt) {
            TestMeanWhile(4);
          } else if (fCtrl) {
            TestMeanWhile(14);
          }
#endif
          else
            HandleSelectMercSlot(4, LOCATEANDSELECT_MERC);
          break;
        case F6:
          if (fShift) HandleSelectMercSlot(5, LOCATE_MERC_ONCE);
#ifdef JA2TESTVERSION
          else if (fAlt) {
            TestMeanWhile(5);
          } else if (fCtrl) {
            TestMeanWhile(15);
          }
#endif
          else
            HandleSelectMercSlot(5, LOCATEANDSELECT_MERC);
          break;

#ifdef JA2TESTVERSION
        case F7:
          if (fAlt) {
            TestMeanWhile(16);
          }
          break;
        case F8:

          if (fAlt) {
            TestMeanWhile(7);
          }
          break;

        case F9:

          if (fCtrl) {
            TestMeanWhile(8);
          } else {
#ifdef JA2EDITOR
            *puiNewEvent = I_ENTER_EDIT_MODE;
            gfMercResetUponEditorEntry = !fAlt;
#endif
          }
          break;
        case F10:

          if (fAlt) {
            TestMeanWhile(9);
          }
          break;
#endif

        case F11:

          if (fAlt) {
#ifdef JA2TESTVERSION
            struct SOLDIERTYPE *pSoldier;

            // Get selected soldier
            if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
              if (pSoldier->ubID == 46) {
                // Change guy to drunk larry
                ForceSoldierProfileID(pSoldier, 47);
              } else {
                // Change guy to normal larry
                ForceSoldierProfileID(pSoldier, 46);
              }

              // Dirty interface
              DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
            }
#endif
          }

          else {
            if (DEBUG_CHEAT_LEVEL()) {
              GetMouseMapPos(&gsQdsEnteringGridNo);
              LeaveTacticalScreen(QUEST_DEBUG_SCREEN);
            }
          }
          break;

        case F12:

#ifdef JA2TESTVERSION
          if (fAlt) {
            uint8_t ubProfile = TONY;

            GetMouseMapPos(&gsQdsEnteringGridNo);
            AddShopkeeperToGridNo(ubProfile, gsQdsEnteringGridNo);
            EnterShopKeeperInterfaceScreen(ubProfile);
          }
#endif
          // clear tactical of messages
          if (fCtrl) {
            ClearTacticalMessageQueue();
          } else if (!fAlt) {
            ClearDisplayedListOfTacticalStrings();
          }
          break;

        case '1':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              // ChangeSoldiersBodyType( TANK_NW, TRUE );
              // MercPtrs[ gusSelectedSoldier ]->uiStatusFlags |= SOLDIER_CREATURE;
              // EVENT_InitNewSoldierAnim( MercPtrs[ gusSelectedSoldier ], CRIPPLE_BEG, 0 , TRUE );
            }
          } else
            ChangeCurrentSquad(0);
          break;

        case '2':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              ChangeSoldiersBodyType(INFANT_MONSTER, TRUE);
            }
          } else if (fCtrl)  // toggle between the different npc debug modes
          {
            if (CHEATER_CHEAT_LEVEL()) {
              ToggleQuestDebugModes(QD_NPC_MSG);
            }
          } else
            ChangeCurrentSquad(1);
          break;

        case '3':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              EVENT_InitNewSoldierAnim(MercPtrs[gusSelectedSoldier], KID_SKIPPING, 0, TRUE);

              // ChangeSoldiersBodyType( LARVAE_MONSTER, TRUE );
              // MercPtrs[ gusSelectedSoldier ]->usAttackingWeapon = TANK_CANNON;
              // LocateSoldier( gusSelectedSoldier, FALSE );
              // EVENT_FireSoldierWeapon( MercPtrs[ gusSelectedSoldier ], usMapPos );
            }
          } else
            ChangeCurrentSquad(2);

          break;

        case '4':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              ChangeSoldiersBodyType(CRIPPLECIV, TRUE);
            }
          } else
            ChangeCurrentSquad(3);

          // ChangeSoldiersBodyType( BLOODCAT, FALSE );
          break;

        case '5':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              ChangeSoldiersBodyType(YAM_MONSTER, TRUE);
            }
          } else
            ChangeCurrentSquad(4);
          break;

        case '6':
          ChangeCurrentSquad(5);
          break;

        case '7':
          ChangeCurrentSquad(6);
          break;

        case '8':
          ChangeCurrentSquad(7);
          break;

        case '9':
          ChangeCurrentSquad(8);
          break;

        case '0':
          ChangeCurrentSquad(9);
          break;

        case 'x':

          if (!fCtrl && !fAlt) {
            // Exchange places...
            struct SOLDIERTYPE *pSoldier1, *pSoldier2;

            // Check if we have a good selected guy
            if (gusSelectedSoldier != NOBODY) {
              pSoldier1 = MercPtrs[gusSelectedSoldier];

              if (gfUIFullTargetFound) {
                // Get soldier...
                pSoldier2 = MercPtrs[gusUIFullTargetID];

                // Check if both OK....
                if (pSoldier1->bLife >= OKLIFE && pSoldier2->ubID != gusSelectedSoldier) {
                  if (pSoldier2->bLife >= OKLIFE) {
                    if (CanSoldierReachGridNoInGivenTileLimit(pSoldier1, pSoldier2->sGridNo, 1,
                                                              (int8_t)gsInterfaceLevel)) {
                      // Exclude enemies....
                      if (!pSoldier2->bNeutral && (pSoldier2->bSide != gbPlayerNum)) {
                      } else {
                        if (CanExchangePlaces(pSoldier1, pSoldier2, TRUE)) {
                          // All's good!
                          SwapMercPositions(pSoldier1, pSoldier2);

                          DeductPoints(pSoldier1, AP_EXCHANGE_PLACES, 0);
                          DeductPoints(pSoldier2, AP_EXCHANGE_PLACES, 0);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          break;

        case '/':

          // Center to guy....
          if (gusSelectedSoldier != NOBODY) {
            LocateSoldier(gusSelectedSoldier, 10);
          }
          break;

        case 'a':
          if (fCtrl) {
            if (gubCheatLevel == 1) {
              gubCheatLevel++;
              fGoodCheatLevelKey = TRUE;
            } else {
              RESET_CHEAT_LEVEL();
            }
          } else if (fAlt) {
#ifdef JA2TESTVERSION
            //	ToggleMercsNeverQuit();
            static uint8_t ubAmbientSound = 0;

            ubAmbientSound++;

            if (ubAmbientSound >= NUM_STEADY_STATE_AMBIENCES) {
              ubAmbientSound = 1;
            }

            SetSteadyStateAmbience(ubAmbientSound);

#endif
          } else {
            BeginAutoBandage();
          }
          break;

        case 'j':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              gfNextFireJam = TRUE;
            }
          } else if (fCtrl) {
#ifdef JA2BETAVERSION
            if (CHEATER_CHEAT_LEVEL()) {
              ToggleNPCRecordDisplay();
            }
#endif
          }
          break;

        case 'b':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              *puiNewEvent = I_NEW_BADMERC;
            }
          } else if (fCtrl) {
            if (gubCheatLevel == 2) {
              gubCheatLevel++;
              fGoodCheatLevelKey = TRUE;
            } else if (gubCheatLevel == 3) {
              gubCheatLevel++;
              fGoodCheatLevelKey = TRUE;
            } else if (gubCheatLevel == 5) {
              gubCheatLevel++;
              fGoodCheatLevelKey = TRUE;
            } else {
              RESET_CHEAT_LEVEL();
            }
            // gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] ^= TRUE;
          } else {
            // nothing in hand and either not in SM panel, or the matching button is enabled if we
            // are in SM panel
            if ((gpItemPointer == NULL) &&
                ((gsCurInterfacePanel != SM_PANEL) ||
                 (ButtonList[iSMPanelButtons[BURSTMODE_BUTTON]]->uiFlags & BUTTON_ENABLED))) {
              SetBurstMode();
            }
          }
          break;
        case 'c':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              CreateNextCivType();
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              ToggleCliffDebug();
            }
          } else {
            HandleStanceChangeFromUIKeys(ANIM_CROUCH);
          }
          break;

        case 'd':
          if (gTacticalStatus.uiFlags & TURNBASED && gTacticalStatus.uiFlags & INCOMBAT) {
            if (gTacticalStatus.ubCurrentTeam == gbPlayerNum) {
              // nothing in hand and the Done button for whichever panel we're in must be enabled
              if ((gpItemPointer == NULL) && !gfDisableTacticalPanelButtons &&
                  (((gsCurInterfacePanel == SM_PANEL) &&
                    (ButtonList[iSMPanelButtons[SM_DONE_BUTTON]]->uiFlags & BUTTON_ENABLED)) ||
                   ((gsCurInterfacePanel == TEAM_PANEL) &&
                    (ButtonList[iTEAMPanelButtons[TEAM_DONE_BUTTON]]->uiFlags & BUTTON_ENABLED)))) {
                if (fAlt) {
                  int32_t cnt;
                  struct SOLDIERTYPE *pSoldier;

                  if (CHEATER_CHEAT_LEVEL()) {
                    for (pSoldier = MercPtrs[gbPlayerNum], cnt = 0;
                         cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID; cnt++, pSoldier++) {
                      if (IsSolActive(pSoldier) && pSoldier->bLife > 0) {
                        // Get APs back...
                        CalcNewActionPoints(pSoldier);

                        fInterfacePanelDirty = DIRTYLEVEL2;
                      }
                    }
                  }
                } else  // End turn only if in combat and it is the player's turn
                  *puiNewEvent = I_ENDTURN;
              }
            }
          }
#ifdef JA2TESTVERSION
          else if (fCtrl)
            AdvanceToNextDay();
#endif
          break;

        case 'e':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              ToggleViewAllMercs();
              ToggleViewAllItems();
            }
          }
#ifdef JA2BETAVERSION
          else if (fCtrl) {
            ToggleMapEdgepoints();
          }
#endif
          else {
            struct SOLDIERTYPE *pSoldier;

            if (gusSelectedSoldier != NOBODY) {
              pSoldier = MercPtrs[gusSelectedSoldier];

              if (pSoldier->bOppCnt > 0) {
                // Cycle....
                CycleVisibleEnemies(pSoldier);
              } else {
                ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                          TacticalStr[NO_ENEMIES_IN_SIGHT_STR]);
              }
            }
          }
          break;

        case 'f':
          if (fCtrl) {
            if (INFORMATION_CHEAT_LEVEL()) {
              // Toggle Frame Rate Display
              gbFPSDisplay = !gbFPSDisplay;
              DisableFPSOverlay((BOOLEAN)!gbFPSDisplay);
              if (!gbFPSDisplay) SetRenderFlags(RENDER_FLAG_FULL);
            }
          } else if (fAlt) {
            if (gGameSettings.fOptions[TOPTION_TRACKING_MODE]) {
              gGameSettings.fOptions[TOPTION_TRACKING_MODE] = FALSE;

              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_TACKING_MODE_OFF]);
            } else {
              gGameSettings.fOptions[TOPTION_TRACKING_MODE] = TRUE;

              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_TACKING_MODE_ON]);
            }
          } else {
            int16_t sGridNo;

            // Get the gridno the cursor is at
            GetMouseMapPos(&sGridNo);

            // if there is a selected soldier, and the cursor location is valid
            if (gusSelectedSoldier != NOBODY && sGridNo != NOWHERE) {
              // if the cursor is over someone
              if (gfUIFullTargetFound) {
                // Display the range to the target
                DisplayRangeToTarget(MercPtrs[gusSelectedSoldier],
                                     MercPtrs[gusUIFullTargetID]->sGridNo);
              } else {
                // Display the range to the target
                DisplayRangeToTarget(MercPtrs[gusSelectedSoldier], sGridNo);
              }
            }
          }
          break;

        case 'g':

          if (fCtrl) {
            if (gubCheatLevel == 0) {
              gubCheatLevel++;
              fGoodCheatLevelKey = TRUE;
            } else {
              RESET_CHEAT_LEVEL();
            }
          } else if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              *puiNewEvent = I_NEW_MERC;
            }
          } else {
            HandlePlayerTogglingLightEffects(TRUE);
          }
          break;
        case 'H':
        case 'h':
          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              if (gfReportHitChances) {
                gfReportHitChances = FALSE;
              } else {
                gfReportHitChances = TRUE;
              }
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              *puiNewEvent = I_TESTHIT;
            }
          } else {
            ShouldTheHelpScreenComeUp(HELP_SCREEN_TACTICAL, TRUE);
          }
          break;

        case 'i':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              CreateRandomItem();
            }
          } else if (fCtrl) {
            if (gubCheatLevel == 4) {
              gubCheatLevel++;
              fGoodCheatLevelKey = TRUE;
              // ATE; We're done.... start cheat mode....
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_CHEAT_LEVEL_TWO]);
              SetHistoryFact(HISTORY_CHEAT_ENABLED, 0, GetWorldTotalMin(), -1, -1);
            } else {
              RESET_CHEAT_LEVEL();
            }
          } else {
            if (gGameSettings.fOptions[TOPTION_GLOW_ITEMS]) {
              gGameSettings.fOptions[TOPTION_GLOW_ITEMS] = FALSE;
              ToggleItemGlow(FALSE);
            } else {
              gGameSettings.fOptions[TOPTION_GLOW_ITEMS] = TRUE;
              ToggleItemGlow(TRUE);
            }
          }
          break;

        case '$':

          break;

        case 'k':
          if (fAlt) {
            if (fCtrl) {
              if (CHEATER_CHEAT_LEVEL()) {
                // next shot by anybody is auto kill
                if (gfNextShotKills) {
                  gfNextShotKills = FALSE;
                } else {
                  gfNextShotKills = TRUE;
                }
              }
            } else {
              if (CHEATER_CHEAT_LEVEL()) {
                GrenadeTest1();
              }
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              GrenadeTest2();
            }
          } else {
            BeginKeyPanelFromKeyShortcut();
          }
          break;

        case INSERT:

          GoIntoOverheadMap();
          break;

        case END:

          if (gusSelectedSoldier != NOBODY) {
            if (CheckForMercContMove(MercPtrs[gusSelectedSoldier])) {
              // Continue
              ContinueMercMovement(MercPtrs[gusSelectedSoldier]);
              ErasePath(TRUE);
            }
          }
          break;

        case HOME:

          if (gGameSettings.fOptions[TOPTION_3D_CURSOR]) {
            gGameSettings.fOptions[TOPTION_3D_CURSOR] = FALSE;

            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_3DCURSOR_OFF]);
          } else {
            gGameSettings.fOptions[TOPTION_3D_CURSOR] = TRUE;

            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_3DCURSOR_ON]);
          }
          break;

#ifdef JA2BETAVERSION
        case 'L':
          gfDisplayStrategicAILogs ^= TRUE;
          if (gfDisplayStrategicAILogs) {
            ScreenMsg(FONT_LTKHAKI, MSG_INTERFACE, L"Strategic AI Log visually enabled.");
          } else {
            ScreenMsg(FONT_LTKHAKI, MSG_INTERFACE, L"Strategic AI Log visually disabled.");
          }
          break;
#endif

        case 'l':

          if (fAlt) {
            if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
              LeaveTacticalScreen(GAME_SCREEN);

              DoQuickLoad();
            }
          }

          else if (fCtrl) {
            if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
              gfSaveGame = FALSE;
              gfCameDirectlyFromGame = TRUE;

              guiPreviousOptionScreen = GAME_SCREEN;
              LeaveTacticalScreen(SAVE_LOAD_SCREEN);
            }
            /*
                                                            if ( INFORMATION_CHEAT_LEVEL( ) )
                                                            {
                                                                    *puiNewEvent = I_LEVELNODEDEBUG;
                                                                    CountLevelNodes();
                                                            }
            */
          } else {
            // nothing in hand and either not in SM panel, or the matching button is enabled if we
            // are in SM panel
            if ((gpItemPointer == NULL) &&
                ((gsCurInterfacePanel != SM_PANEL) ||
                 (ButtonList[iSMPanelButtons[LOOK_BUTTON]]->uiFlags & BUTTON_ENABLED))) {
              *puiNewEvent = LC_CHANGE_TO_LOOK;
            }
          }
          break;
        case 'm':
          if (fAlt) {
            if (INFORMATION_CHEAT_LEVEL()) {
              *puiNewEvent = I_LEVELNODEDEBUG;
              CountLevelNodes();
            }
          } else if (fCtrl) {
          } else {
            // nothing in hand and the Map Screen button for whichever panel we're in must be
            // enabled
            if ((gpItemPointer == NULL) && !gfDisableTacticalPanelButtons &&
                (((gsCurInterfacePanel == SM_PANEL) &&
                  (ButtonList[iSMPanelButtons[SM_MAP_SCREEN_BUTTON]]->uiFlags & BUTTON_ENABLED)) ||
                 ((gsCurInterfacePanel == TEAM_PANEL) &&
                  (ButtonList[iTEAMPanelButtons[TEAM_MAP_SCREEN_BUTTON]]->uiFlags &
                   BUTTON_ENABLED)))) {
              // go to Map screen
              if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
                GoToMapScreenFromTactical();
              }
            }
          }
          break;

        case PGDN:

          if (CHEATER_CHEAT_LEVEL()) {
            if (fCtrl) AttemptToChangeFloorLevel(+1);  // try to enter a lower underground level
          }

          if (guiCurrentScreen != DEBUG_SCREEN) {
            if (gusSelectedSoldier != NOBODY) {
              // nothing in hand and either not in SM panel, or the matching button is enabled if we
              // are in SM panel
              if ((gpItemPointer == NULL)) {
                GotoLowerStance(MercPtrs[gusSelectedSoldier]);
              }
            }
          }
          break;

        case PGUP:

          if (CHEATER_CHEAT_LEVEL()) {
            if (fCtrl) AttemptToChangeFloorLevel(-1);  // try to go up towards ground level
          }

          if (guiCurrentScreen != DEBUG_SCREEN) {
            if (gusSelectedSoldier != NOBODY) {
              // nothing in hand and either not in SM panel, or the matching button is enabled if we
              // are in SM panel
              if ((gpItemPointer == NULL)) {
                GotoHeigherStance(MercPtrs[gusSelectedSoldier]);
              }
            }
          }
          break;

        case '*':

          if (gTacticalStatus.uiFlags & RED_ITEM_GLOW_ON) {
            gTacticalStatus.uiFlags &= (~RED_ITEM_GLOW_ON);
          } else {
            gTacticalStatus.uiFlags |= RED_ITEM_GLOW_ON;
          }
          break;

        case 'n':

          if (fAlt) {
            static uint16_t gQuoteNum = 0;

            if (INFORMATION_CHEAT_LEVEL()) {
              if (gfUIFullTargetFound) {
                TacticalCharacterDialogue(MercPtrs[gusUIFullTargetID], gQuoteNum);
                gQuoteNum++;
              }
            }
          } else if (fCtrl) {
          } else if (!CycleSoldierFindStack(usMapPos))  // Are we over a merc stack?
            CycleIntTileFindStack(usMapPos);  // If not, now check if we are over a struct stack
          break;

        case 'o':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              gStrategicStatus.usPlayerKills += NumEnemiesInAnySector(
                  (uint8_t)gWorldSectorX, (uint8_t)gWorldSectorY, gbWorldSectorZ);
              ObliterateSector();
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              CreatePlayerControlledMonster();
            }
          } else {
            // nothing in hand and the Options Screen button for whichever panel we're in must be
            // enabled
            if ((gpItemPointer == NULL) && !gfDisableTacticalPanelButtons &&
                ((gsCurInterfacePanel != SM_PANEL) ||
                 (ButtonList[iSMPanelButtons[OPTIONS_BUTTON]]->uiFlags & BUTTON_ENABLED))) {
              if (!fDisableMapInterfaceDueToBattle) {
                // go to Options screen
                guiPreviousOptionScreen = GAME_SCREEN;
                LeaveTacticalScreen(OPTIONS_SCREEN);
              }
            }
          }
          break;

        case 'p':

#ifdef JA2BETAVERSION
          if (fAlt)
            ToggleRealTimeConfirm();
          else
#endif

#ifdef JA2TESTVERSION
              if (fCtrl) {
            // CTRL-P: Display player's highest progress percentage
            DumpSectorDifficultyInfo();
          } else
#endif
            HandleStanceChangeFromUIKeys(ANIM_PRONE);
          break;

        case 'r':
          if (gusSelectedSoldier != NO_SOLDIER) {
            if (fAlt)  // reload selected merc's weapon
            {
              if (CHEATER_CHEAT_LEVEL()) {
                ReloadWeapon(MercPtrs[gusSelectedSoldier],
                             MercPtrs[gusSelectedSoldier]->ubAttackingHand);
              }
            } else if (fCtrl) {
              if (INFORMATION_CHEAT_LEVEL()) {
                DoChrisTest(MercPtrs[gusSelectedSoldier]);
              }
            } else {
              if (!MercInWater(MercPtrs[gusSelectedSoldier]) &&
                  !(MercPtrs[gusSelectedSoldier]->uiStatusFlags & SOLDIER_ROBOT)) {
                // change selected merc to run
                if (MercPtrs[gusSelectedSoldier]->usUIMovementMode != WALKING &&
                    MercPtrs[gusSelectedSoldier]->usUIMovementMode != RUNNING) {
                  UIHandleSoldierStanceChange((uint8_t)gusSelectedSoldier, ANIM_STAND);
                  MercPtrs[gusSelectedSoldier]->fUIMovementFast = 1;
                } else {
                  MercPtrs[gusSelectedSoldier]->fUIMovementFast = 1;
                  MercPtrs[gusSelectedSoldier]->usUIMovementMode = RUNNING;
                  gfPlotNewMovement = TRUE;
                }
              }
            }
          }
          break;
        case 's':

          if (fCtrl) {
            if (!fDisableMapInterfaceDueToBattle && !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
              // if the game CAN be saved
              if (CanGameBeSaved()) {
                gfSaveGame = TRUE;
                gfCameDirectlyFromGame = TRUE;

                guiPreviousOptionScreen = GAME_SCREEN;
                LeaveTacticalScreen(SAVE_LOAD_SCREEN);
              } else {
                // Display a message saying the player cant save now
                DoMessageBox(MSG_BOX_BASIC_STYLE,
                             zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW], GAME_SCREEN,
                             (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
              }
            }
          } else if (fAlt) {
            if (!fDisableMapInterfaceDueToBattle && !(gTacticalStatus.uiFlags & ENGAGED_IN_CONV)) {
              // if the game CAN be saved
              if (CanGameBeSaved()) {
                guiPreviousOptionScreen = GAME_SCREEN;
                // guiPreviousOptionScreen = guiCurrentScreen;
                DoQuickSave();
              } else {
                // Display a message saying the player cant save now
                DoMessageBox(MSG_BOX_BASIC_STYLE,
                             zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW], GAME_SCREEN,
                             (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
              }
            }
          } else if (gusSelectedSoldier != NOBODY) {
            gfPlotNewMovement = TRUE;
            HandleStanceChangeFromUIKeys(ANIM_STAND);
          }
          break;

        case 't':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              TeleportSelectedSoldier();
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              TestCapture();

              // EnterCombatMode( gbPlayerNum );
            }
          } else
            ToggleTreeTops();
          break;

        case '=':
          // if the display cover or line of sight is being displayed
          if (_KeyDown(END) || _KeyDown(DEL)) {
            if (_KeyDown(DEL)) ChangeSizeOfDisplayCover(gGameSettings.ubSizeOfDisplayCover + 1);

            if (_KeyDown(END)) ChangeSizeOfLOS(gGameSettings.ubSizeOfLOS + 1);
          } else {
#ifdef JA2TESTVERSION
            if (fAlt) {
              WarpGameTime(60, TRUE);
              break;
            }
#endif

            // ATE: This key will select everybody in the sector
            if (!(gTacticalStatus.uiFlags & INCOMBAT)) {
              struct SOLDIERTYPE *pSoldier;
              int32_t cnt;

              cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
              for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
                   cnt++, pSoldier++) {
                // Check if this guy is OK to control....
                if (OK_CONTROLLABLE_MERC(pSoldier) &&
                    !(pSoldier->uiStatusFlags &
                      (SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER))) {
                  pSoldier->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
                }
              }
              EndMultiSoldierSelection(TRUE);
            }
          }
          break;

        case 'u':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              RefreshSoldier();
            }
          } else if (fCtrl) {
            int32_t cnt;
            struct SOLDIERTYPE *pSoldier;

            if (CHEATER_CHEAT_LEVEL() && gusSelectedSoldier != NOBODY) {
              for (pSoldier = MercPtrs[gbPlayerNum], cnt = 0;
                   cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID; cnt++, pSoldier++) {
                if (IsSolActive(pSoldier) && pSoldier->bLife > 0) {
                  // Get breath back
                  pSoldier->bBreath = pSoldier->bBreathMax;

                  // Get life back
                  pSoldier->bLife = pSoldier->bLifeMax;
                  pSoldier->bBleeding = 0;

                  fInterfacePanelDirty = DIRTYLEVEL2;
                }
              }
            }
          }

          else if (gusSelectedSoldier != NO_SOLDIER)
            *puiNewEvent = M_CHANGE_TO_ACTION;
          break;

        case 'v':
          if (fAlt) {
#ifdef JA2TESTVERSION
            if (gfDoVideoScroll ^= TRUE)
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Video Scroll ON");
            else
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Video Scroll OFF");
#endif
          } else if (!fCtrl) {
            DisplayGameSettings();
          }
          break;
        case 'w':
        case 'W':

          if (fAlt) {
            if (CHEATER_CHEAT_LEVEL()) {
              if (InItemDescriptionBox()) {
                // Swap item in description panel...
                CycleItemDescriptionItem();

              } else {
                CycleSelectedMercsItem();
              }
            }
          } else if (fCtrl) {
            if (CHEATER_CHEAT_LEVEL()) {
              if (gusSelectedSoldier != NOBODY) {
                CreateItem(FLAMETHROWER, 100, &(MercPtrs[gusSelectedSoldier]->inv[HANDPOS]));
              }
            }
          } else
            ToggleWireFrame();
          break;

        case 'y':
          if (fAlt) {
            struct OBJECTTYPE Object;
            struct SOLDIERTYPE *pSoldier;

            if (CHEATER_CHEAT_LEVEL()) {
              QuickCreateProfileMerc(CIV_TEAM, MARIA);  // Ira

              // Recruit!
              RecruitEPC(MARIA);
            }

            // Create object and set
            CreateItem((uint16_t)G41, 100, &Object);

            pSoldier = FindSoldierByProfileID(ROBOT, FALSE);

            AutoPlaceObject(pSoldier, &Object, FALSE);

          } else {
            if (INFORMATION_CHEAT_LEVEL()) {
              *puiNewEvent = I_LOSDEBUG;
            }
          }
          // else if( gusSelectedSoldier != NO_SOLDIER )
          break;
        case 'z':
          if (fCtrl) {
            if (INFORMATION_CHEAT_LEVEL()) {
              ToggleZBuffer();
            }
          } else if (fAlt) {
            // Toggle squad's stealth mode.....
            // For each guy on squad...
            {
              struct SOLDIERTYPE *pTeamSoldier;
              int8_t bLoop;
              BOOLEAN fStealthOn = FALSE;

              // Check if at least one guy is on stealth....
              for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID,
                  pTeamSoldier = MercPtrs[bLoop];
                   bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++, pTeamSoldier++) {
                if (OK_CONTROLLABLE_MERC(pTeamSoldier) &&
                    pTeamSoldier->bAssignment == CurrentSquad()) {
                  if (pTeamSoldier->bStealthMode) {
                    fStealthOn = TRUE;
                  }
                }
              }

              fStealthOn = !fStealthOn;

              for (bLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID,
                  pTeamSoldier = MercPtrs[bLoop];
                   bLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; bLoop++, pTeamSoldier++) {
                if (OK_CONTROLLABLE_MERC(pTeamSoldier) &&
                    pTeamSoldier->bAssignment == CurrentSquad() && !AM_A_ROBOT(pTeamSoldier)) {
                  if (gpSMCurrentMerc != NULL && bLoop == gpSMCurrentMerc->ubID) {
                    gfUIStanceDifferent = TRUE;
                  }

                  pTeamSoldier->bStealthMode = fStealthOn;
                }
              }

              fInterfacePanelDirty = DIRTYLEVEL2;

              // OK, display message
              if (fStealthOn) {
                ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
                          pMessageStrings[MSG_SQUAD_ON_STEALTHMODE]);
              } else {
                ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
                          pMessageStrings[MSG_SQUAD_OFF_STEALTHMODE]);
              }
            }
          } else {
            // nothing in hand and either not in SM panel, or the matching button is enabled if we
            // are in SM panel
            if ((gpItemPointer == NULL)) {
              HandleStealthChangeFromUIKeys();
            }
          }
          break;

        case '-':
        case '_':
          // if the display cover or line of sight is being displayed
          if (_KeyDown(END) || _KeyDown(DEL)) {
            if (_KeyDown(DEL)) ChangeSizeOfDisplayCover(gGameSettings.ubSizeOfDisplayCover - 1);

            if (_KeyDown(END)) ChangeSizeOfLOS(gGameSettings.ubSizeOfLOS - 1);
          } else {
            if (fAlt) {
              if (MusicGetVolume() >= 20)
                MusicSetVolume(MusicGetVolume() - 20);
              else
                MusicSetVolume(0);
            } else if (fCtrl) {
#ifdef JA2TESTVERSION
              gTacticalStatus.bRealtimeSpeed = max(1, gTacticalStatus.bRealtimeSpeed - 1);
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Decreasing Realtime speed to %d",
                        gTacticalStatus.bRealtimeSpeed);
#endif
            } else {
#ifdef JA2TESTVERSION
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Using Normal Scroll Speed");
              gubCurScrollSpeedID = 1;
#endif
            }
          }
          break;
        case '+':

#ifdef JA2TESTVERSION
          if (fAlt) {
            if (MusicGetVolume() <= 107)
              MusicSetVolume(MusicGetVolume() + 20);
            else
              MusicSetVolume(127);
          } else if (fCtrl) {
            gTacticalStatus.bRealtimeSpeed =
                min(MAX_REALTIME_SPEED_VAL, gTacticalStatus.bRealtimeSpeed + 1);
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Increasing Realtime speed to %d",
                      gTacticalStatus.bRealtimeSpeed);
          } else {
            ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Using Higher Scroll Speed");
            gubCurScrollSpeedID = 2;
          }
#endif
          break;
        case '`':

          // Switch panels...
          {
            ToggleTacticalPanels();

            if (CHEATER_CHEAT_LEVEL()) {
              // EnvBeginRainStorm( 1 );
            }
          }
          break;
      }

      if (!fGoodCheatLevelKey && gubCheatLevel < 4) {
        RESET_CHEAT_LEVEL();
      }
    }
  }
}

void HandleTalkingMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent) {
  // CHECK ESC KEYS HERE....
  if (pInputEvent->usEvent == KEY_UP) {
    if (pInputEvent->usParam == ESC) {
      // Handle esc in talking menu
      if (HandleTalkingMenuEscape(TRUE, TRUE)) {
        *puiNewEvent = A_CHANGE_TO_MOVE;
      }
    } else if (pInputEvent->usParam == BACKSPACE) {
      HandleTalkingMenuBackspace();
    }
  }
}

void HandleSectorExitMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent) {
  // CHECK ESC KEYS HERE....
  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == ESC)) {
    // Handle esc in talking menu
    RemoveSectorExitMenu(FALSE);

    *puiNewEvent = A_CHANGE_TO_MOVE;
  }
}

void HandleOpenDoorMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent) {
  // CHECK ESC KEYS HERE....
  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == ESC)) {
    // Handle esc in talking menu
    CancelOpenDoorMenu();
    HandleOpenDoorMenu();
    *puiNewEvent = A_CHANGE_TO_MOVE;
  }
}

void HandleMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent) {
  // CHECK ESC KEYS HERE....
  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == ESC)) {
    // Handle esc in talking menu
    CancelMovementMenu();

    *puiNewEvent = A_CHANGE_TO_MOVE;
  }
}

void HandleItemMenuKeys(InputAtom *pInputEvent, uint32_t *puiNewEvent) {
  // CHECK ESC KEYS HERE....
  if ((pInputEvent->usEvent == KEY_UP) && (pInputEvent->usParam == ESC)) {
    // Handle esc in talking menu
    RemoveItemPickupMenu();
    *puiNewEvent = A_CHANGE_TO_MOVE;
  }
}

BOOLEAN HandleCheckForExitArrowsInput(BOOLEAN fAdjustConfirm) {
  int16_t sMapPos;

  // If not in move mode, return!
  if (gCurrentUIMode != MOVE_MODE) {
    return (FALSE);
  }

  if (gusSelectedSoldier == NOBODY) {
    return (FALSE);
  }

  // ATE: Remove confirm for exit arrows...
  fAdjustConfirm = TRUE;
  gfUIConfirmExitArrows = TRUE;

  // Return right away, saying that we are in this mode, don't do any normal stuff!
  if (guiCurrentUICursor == NOEXIT_EAST_UICURSOR || guiCurrentUICursor == NOEXIT_WEST_UICURSOR ||
      guiCurrentUICursor == NOEXIT_NORTH_UICURSOR || guiCurrentUICursor == NOEXIT_SOUTH_UICURSOR ||
      guiCurrentUICursor == NOEXIT_GRID_UICURSOR) {
    // Yeah, but add a message....
    if (gfInvalidTraversal) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                TacticalStr[CANNOT_LEAVE_SECTOR_FROM_SIDE_STR]);
      gfInvalidTraversal = FALSE;
    } else if (gfRobotWithoutControllerAttemptingTraversal) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, gzLateLocalizedString[1]);
      gfRobotWithoutControllerAttemptingTraversal = FALSE;
    } else if (gfLoneEPCAttemptingTraversal) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
                pExitingSectorHelpText[EXIT_GUI_ESCORTED_CHARACTERS_CANT_LEAVE_SECTOR_ALONE_STR],
                MercPtrs[gusSelectedSoldier]->name);
      gfLoneEPCAttemptingTraversal = FALSE;
    } else if (gubLoneMercAttemptingToAbandonEPCs) {
      wchar_t str[256];
      if (gubLoneMercAttemptingToAbandonEPCs == 1) {  // Use the singular version of the string
        if (gMercProfiles[MercPtrs[gusSelectedSoldier]->ubProfile].bSex == MALE) {  // male singular
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_SINGULAR],
                   MercPtrs[gusSelectedSoldier]->name,
                   MercPtrs[gbPotentiallyAbandonedEPCSlotID]->name);
        } else {  // female singular
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_SINGULAR],
                   MercPtrs[gusSelectedSoldier]->name,
                   MercPtrs[gbPotentiallyAbandonedEPCSlotID]->name);
        }
      } else {  // Use the plural version of the string
        if (gMercProfiles[MercPtrs[gusSelectedSoldier]->ubProfile].bSex == MALE) {  // male plural
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_MALE_PLURAL],
                   MercPtrs[gusSelectedSoldier]->name);
        } else {  // female plural
          swprintf(str, ARR_SIZE(str),
                   pExitingSectorHelpText[EXIT_GUI_MERC_CANT_ISOLATE_EPC_HELPTEXT_FEMALE_PLURAL],
                   MercPtrs[gusSelectedSoldier]->name);
        }
      }
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, str);
      gubLoneMercAttemptingToAbandonEPCs = FALSE;
    } else {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[MERC_IS_TOO_FAR_AWAY_STR],
                MercPtrs[gusSelectedSoldier]->name);
    }

    return (TRUE);
  }

  // Check if we want to exit!
  if (guiCurrentUICursor == EXIT_GRID_UICURSOR ||
      guiCurrentUICursor == CONFIRM_EXIT_GRID_UICURSOR) {
    if (fAdjustConfirm) {
      if (!gfUIConfirmExitArrows) {
        gfUIConfirmExitArrows = TRUE;
      } else {
        if (!GetMouseMapPos(&sMapPos)) {
          return (FALSE);
        }

        // Goto next sector
        // SimulateMouseMovement( gusMouseXPos - 5, gusMouseYPos );
        InitSectorExitMenu(DIRECTION_EXITGRID, sMapPos);
      }
    }
    return (TRUE);
  }

  // Check if we want to exit!
  if (guiCurrentUICursor == EXIT_EAST_UICURSOR ||
      guiCurrentUICursor == CONFIRM_EXIT_EAST_UICURSOR) {
    if (fAdjustConfirm) {
      if (!gfUIConfirmExitArrows) {
        gfUIConfirmExitArrows = TRUE;
      } else {
        // Goto next sector
        // SimulateMouseMovement( gusMouseXPos - 5, gusMouseYPos );
        InitSectorExitMenu(EAST, 0);
      }
    }
    return (TRUE);
  }
  if (guiCurrentUICursor == EXIT_WEST_UICURSOR ||
      guiCurrentUICursor == CONFIRM_EXIT_WEST_UICURSOR) {
    if (fAdjustConfirm) {
      if (!gfUIConfirmExitArrows) {
        gfUIConfirmExitArrows = TRUE;
      } else {
        // Goto next sector
        // SimulateMouseMovement( gusMouseXPos + 5, gusMouseYPos );
        InitSectorExitMenu(WEST, 0);
      }
    }
    return (TRUE);
  }
  if (guiCurrentUICursor == EXIT_NORTH_UICURSOR ||
      guiCurrentUICursor == CONFIRM_EXIT_NORTH_UICURSOR) {
    if (fAdjustConfirm) {
      if (!gfUIConfirmExitArrows) {
        gfUIConfirmExitArrows = TRUE;
      } else {
        // Goto next sector
        // SimulateMouseMovement( gusMouseXPos, gusMouseYPos + 5 );
        InitSectorExitMenu(NORTH, 0);
      }
    }
    return (TRUE);
  }
  if (guiCurrentUICursor == EXIT_SOUTH_UICURSOR ||
      guiCurrentUICursor == CONFIRM_EXIT_SOUTH_UICURSOR) {
    if (fAdjustConfirm) {
      if (!gfUIConfirmExitArrows) {
        gfUIConfirmExitArrows = TRUE;
      } else {
        // Goto next sector
        // SimulateMouseMovement( gusMouseXPos, gusMouseYPos - 5);
        InitSectorExitMenu(SOUTH, 0);
      }
    }
    return (TRUE);
  }
  return (FALSE);
}

// Simple function implementations called by keyboard input

void CreateRandomItem() {
  struct OBJECTTYPE Object;
  int16_t usMapPos;
  if (GetMouseMapPos(&usMapPos)) {
    CreateItem((uint16_t)(Random(35) + 1), 100, &Object);
    AddItemToPool(usMapPos, &Object, -1, 0, 0, 0);
  }
}

void MakeSelectedSoldierTired() {
  // Key to make guy get tired!
  struct SOLDIERTYPE *pSoldier;
  struct OBJECTTYPE Object;
  int16_t usMapPos;
  if (GetMouseMapPos(&usMapPos)) {
    CreateItem((uint16_t)TNT, 100, &Object);
    AddItemToPool(usMapPos, &Object, -1, 0, 0, 0);
  }

  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (gfUIFullTargetFound) {
    // Get Soldier
    GetSoldier(&pSoldier, gusUIFullTargetID);

    // FatigueCharacter( pSoldier );

    fInterfacePanelDirty = DIRTYLEVEL2;
  }
}

void ToggleRealTime(uint32_t *puiNewEvent) {
  if (gTacticalStatus.uiFlags & TURNBASED) {
    // Change to real-time
    gTacticalStatus.uiFlags &= (~TURNBASED);
    gTacticalStatus.uiFlags |= REALTIME;

    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Switching to Realtime.");
  } else if (gTacticalStatus.uiFlags & REALTIME) {
    // Change to turn-based
    gTacticalStatus.uiFlags |= TURNBASED;
    gTacticalStatus.uiFlags &= (~REALTIME);

    *puiNewEvent = M_ON_TERRAIN;

    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Switching to Turnbased.");
  }

  // Plot new path!
  gfPlotNewMovement = TRUE;
}

void ToggleViewAllMercs() {
  // Set option to show all mercs
  if (gTacticalStatus.uiFlags & SHOW_ALL_MERCS) {
    gTacticalStatus.uiFlags &= (~SHOW_ALL_MERCS);
  } else {
    gTacticalStatus.uiFlags |= SHOW_ALL_MERCS;
  }

  // RE-RENDER
  SetRenderFlags(RENDER_FLAG_FULL);
}

void ToggleViewAllItems() {
  // Set option to show all mercs
  if (gTacticalStatus.uiFlags & SHOW_ALL_ITEMS) {
    gTacticalStatus.uiFlags &= ~SHOW_ALL_ITEMS;
  } else {
    gTacticalStatus.uiFlags |= SHOW_ALL_ITEMS;
  }

  if (gGameSettings.fOptions[TOPTION_GLOW_ITEMS]) {
    ToggleItemGlow(TRUE);
  } else {
    ToggleItemGlow(FALSE);
  }

  // RE-RENDER
  SetRenderFlags(RENDER_FLAG_FULL);
}

void TestExplosion() {
  int16_t usMapPos;
  if (GetMouseMapPos(&usMapPos)) {
    EXPLOSION_PARAMS ExpParams;
    ExpParams.uiFlags = 0;
    ExpParams.ubOwner = NOBODY;
    ExpParams.ubTypeID = STUN_BLAST;
    ExpParams.sGridNo = usMapPos;

    GenerateExplosion(&ExpParams);

    PlayJA2Sample(EXPLOSION_1, RATE_11025, MIDVOLUME, 1, MIDDLEPAN);
  }
}

void CycleSelectedMercsItem() {
  uint16_t usOldItem;
  struct SOLDIERTYPE *pSoldier;
  // Cycle selected guy's item...
  if (gfUIFullTargetFound) {
    // Get soldier...
    pSoldier = MercPtrs[gusUIFullTargetID];

    usOldItem = pSoldier->inv[HANDPOS].usItem;

    usOldItem++;

    if (usOldItem > MAX_WEAPONS) {
      usOldItem = 0;
    }

    CreateItem((uint16_t)usOldItem, 100, &(pSoldier->inv[HANDPOS]));

    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
  }
}

void ToggleWireFrame() {
  if (gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME]) {
    gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME] = FALSE;

    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_WIREFRAMES_REMOVED]);
  } else {
    gGameSettings.fOptions[TOPTION_TOGGLE_WIREFRAME] = TRUE;

    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_WIREFRAMES_ADDED]);
  }

  SetRenderFlags(RENDER_FLAG_FULL);
}

void RefreshSoldier() {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (gfUIFullTargetFound) {
    // Get Soldier
    GetSoldier(&pSoldier, gusUIFullTargetID);

    ReviveSoldier(pSoldier);
  }

  if (GetMouseMapPos(&usMapPos))
    sprintf(gDebugStr, "%d %d %d %d %d %d %d %d", gubWorldMovementCosts[usMapPos][0][0],
            gubWorldMovementCosts[usMapPos][1][gsInterfaceLevel],
            gubWorldMovementCosts[usMapPos][2][gsInterfaceLevel],
            gubWorldMovementCosts[usMapPos][3][gsInterfaceLevel],
            gubWorldMovementCosts[usMapPos][4][gsInterfaceLevel],
            gubWorldMovementCosts[usMapPos][5][gsInterfaceLevel],
            gubWorldMovementCosts[usMapPos][6][gsInterfaceLevel],
            gubWorldMovementCosts[usMapPos][7][gsInterfaceLevel]);
}

void ChangeSoldiersBodyType(uint8_t ubBodyType, BOOLEAN fCreateNewPalette) {
  struct SOLDIERTYPE *pSoldier;
  if (gusSelectedSoldier != NO_SOLDIER) {
    if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
      pSoldier->ubBodyType = ubBodyType;
      EVENT_InitNewSoldierAnim(pSoldier, STANDING, 0, TRUE);
      // SetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );
      if (fCreateNewPalette) {
        CreateSoldierPalettes(pSoldier);

        switch (ubBodyType) {
          case ADULTFEMALEMONSTER:
          case AM_MONSTER:
          case YAF_MONSTER:
          case YAM_MONSTER:
          case LARVAE_MONSTER:
          case INFANT_MONSTER:
          case QUEENMONSTER:

            pSoldier->uiStatusFlags |= SOLDIER_MONSTER;
            memset(&(pSoldier->inv), 0, sizeof(struct OBJECTTYPE) * NUM_INV_SLOTS);
            AssignCreatureInventory(pSoldier);

            CreateItem(CREATURE_YOUNG_MALE_SPIT, 100, &(pSoldier->inv[HANDPOS]));

            break;

          case TANK_NW:
          case TANK_NE:

            pSoldier->uiStatusFlags |= SOLDIER_VEHICLE;
            // pSoldier->inv[ HANDPOS ].usItem = TANK_CANNON;

            pSoldier->inv[HANDPOS].usItem = MINIMI;
            pSoldier->bVehicleID = (int8_t)AddVehicleToList(
                GetSolSectorX(pSoldier), GetSolSectorY(pSoldier), GetSolSectorZ(pSoldier), HUMMER);

            break;
        }
      }
    }
  }
}

void TeleportSelectedSoldier() {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  // CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    if (GetMouseMapPos(&usMapPos)) {
      // Check level first....
      if (gsInterfaceLevel == 0) {
        SetSoldierHeight(pSoldier, 0);
        TeleportSoldier(pSoldier, usMapPos, FALSE);
        EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
      } else {
        // Is there a roof?
        if (FindStructure(usMapPos, STRUCTURE_ROOF) != NULL) {
          SetSoldierHeight(pSoldier, 50.0);

          TeleportSoldier(pSoldier, usMapPos, TRUE);
          EVENT_StopMerc(pSoldier, pSoldier->sGridNo, pSoldier->bDirection);
        }
      }
    }
  }
}

void ToggleTreeTops() {
  if (gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS]) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[REMOVING_TREETOPS_STR]);
    WorldHideTrees();
    gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
  } else {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[SHOWING_TREETOPS_STR]);
    WorldShowTrees();
    gTacticalStatus.uiFlags &= (~NOHIDE_REDUNDENCY);
  }
  gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS] =
      !gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS];

  // FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
  InvalidateWorldRedundency();
}

void ToggleZBuffer() {
  // Set option to show all mercs
  if (gTacticalStatus.uiFlags & SHOW_Z_BUFFER) {
    gTacticalStatus.uiFlags &= (~SHOW_Z_BUFFER);
    SetRenderFlags(SHOW_Z_BUFFER);
  } else {
    gTacticalStatus.uiFlags |= SHOW_Z_BUFFER;
  }
}

void TogglePlanningMode() {
  struct SOLDIERTYPE *pSoldier;
  int16_t usMapPos;
  // DO ONLY IN TURNED BASED!
  if (gTacticalStatus.uiFlags & TURNBASED && (gTacticalStatus.uiFlags & INCOMBAT)) {
    // CANCEL FROM PLANNING MODE!
    if (InUIPlanMode()) {
      EndUIPlan();
    } else if (GetMouseMapPos(&usMapPos)) {
      switch (gCurrentUIMode) {
        case MOVE_MODE:
          if (gusSelectedSoldier != NO_SOLDIER) {
            GetSoldier(&pSoldier, gusSelectedSoldier);
            BeginUIPlan(pSoldier);
            AddUIPlan(usMapPos, UIPLAN_ACTION_MOVETO);
          }
          break;
        case ACTION_MODE:
          if (gusSelectedSoldier != NO_SOLDIER) {
            GetSoldier(&pSoldier, gusSelectedSoldier);
            BeginUIPlan(pSoldier);
            AddUIPlan(usMapPos, UIPLAN_ACTION_FIRE);
          }
          break;
      }
    }
  }
}

void SetBurstMode() {
  if (gusSelectedSoldier != NO_SOLDIER) {
    ChangeWeaponMode(MercPtrs[gusSelectedSoldier]);
  }
}

void ObliterateSector() {
  int32_t cnt;
  struct SOLDIERTYPE *pTSoldier;

  // Kill everybody!
  cnt = gTacticalStatus.Team[gbPlayerNum].bLastID + 1;

#ifdef JA2BETAVERSION
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Obliterating Sector!");
#endif

  for (pTSoldier = MercPtrs[cnt]; cnt < MAX_NUM_SOLDIERS; pTSoldier++, cnt++) {
    if (pTSoldier->bActive && !pTSoldier->bNeutral && (pTSoldier->bSide != gbPlayerNum)) {
      //	ANITILE_PARAMS	AniParams;
      //		memset( &AniParams, 0, sizeof( ANITILE_PARAMS ) );
      //		AniParams.sGridNo							=
      // pTSoldier->sGridNo; 		AniParams.ubLevelID = ANI_STRUCT_LEVEL;
      // AniParams.usTileType				  = FIRSTEXPLOSION; AniParams.usTileIndex
      //= FIRSTEXPLOSION1; 	AniParams.sDelay = 80; 	AniParams.sStartFrame
      // = 0; 	AniParams.uiFlags = ANITILE_FORWARD;

      //	CreateAnimationTile( &AniParams );
      // PlayJA2Sample( EXPLOSION_1, RATE_11025, MIDVOLUME, 1, MIDDLEPAN );

      EVENT_SoldierGotHit(pTSoldier, 0, 400, 0, pTSoldier->bDirection, 320, NOBODY,
                          FIRE_WEAPON_NO_SPECIAL, pTSoldier->bAimShotLocation, 0, NOWHERE);
    }
  }
}

void RandomizeMercProfile() {
  struct SOLDIERTYPE *pSoldier;
  // Get selected soldier
  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    // Change guy!
    ForceSoldierProfileID(pSoldier, (uint8_t)Random(30));

    // Dirty interface
    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
  }
}

void JumpFence() {
  struct SOLDIERTYPE *pSoldier;
  int8_t bDirection;
  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    if (FindFenceJumpDirection(pSoldier, pSoldier->sGridNo, pSoldier->bDirection, &bDirection)) {
      BeginSoldierClimbFence(pSoldier);
    }
  }
}

void CreateNextCivType() {
  int16_t sWorldX, sWorldY;
  SOLDIERCREATE_STRUCT MercCreateStruct;
  int16_t usMapPos;
  static int8_t bBodyType = FATCIV;
  // Get Grid Corrdinates of mouse
  if (GetMouseWorldCoordsInCenter(&sWorldX, &sWorldY) && GetMouseMapPos(&usMapPos)) {
    uint8_t iNewIndex;

    memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
    MercCreateStruct.ubProfile = NO_PROFILE;
    MercCreateStruct.sSectorX = gWorldSectorX;
    MercCreateStruct.sSectorY = gWorldSectorY;
    MercCreateStruct.bSectorZ = gbWorldSectorZ;
    MercCreateStruct.bBodyType = bBodyType;
    MercCreateStruct.bDirection = SOUTH;

    bBodyType++;

    if (bBodyType > KIDCIV) {
      bBodyType = FATCIV;
    }

    MercCreateStruct.bTeam = CIV_TEAM;
    MercCreateStruct.sInsertionGridNo = usMapPos;
    RandomizeNewSoldierStats(&MercCreateStruct);

    if (TacticalCreateSoldier(&MercCreateStruct, &iNewIndex)) {
      AddSoldierToSector(iNewIndex);

      // So we can see them!
      AllTeamsLookForAll(NO_INTERRUPTS);
    }
  }
}

void ToggleCliffDebug() {
  // Set option to show all mercs
  if (gTacticalStatus.uiFlags & DEBUGCLIFFS) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Cliff debug OFF.");

    gTacticalStatus.uiFlags &= (~DEBUGCLIFFS);
    SetRenderFlags(RENDER_FLAG_FULL);
  } else {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Cliff debug ON.");

    gTacticalStatus.uiFlags |= DEBUGCLIFFS;
  }
}

void CreateCow() {
  int16_t sWorldX, sWorldY;
  SOLDIERCREATE_STRUCT MercCreateStruct;
  int16_t usMapPos;
  // Get Grid Corrdinates of mouse
  if (GetMouseWorldCoordsInCenter(&sWorldX, &sWorldY) && GetMouseMapPos(&usMapPos)) {
    uint8_t iNewIndex;

    memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
    MercCreateStruct.ubProfile = NO_PROFILE;
    MercCreateStruct.sSectorX = gWorldSectorX;
    MercCreateStruct.sSectorY = gWorldSectorY;
    MercCreateStruct.bSectorZ = gbWorldSectorZ;
    MercCreateStruct.bBodyType = COW;
    // MercCreateStruct.bTeam				= SOLDIER_CREATE_AUTO_TEAM;
    MercCreateStruct.bTeam = CIV_TEAM;
    MercCreateStruct.sInsertionGridNo = usMapPos;
    RandomizeNewSoldierStats(&MercCreateStruct);

    if (TacticalCreateSoldier(&MercCreateStruct, &iNewIndex)) {
      AddSoldierToSector(iNewIndex);

      // So we can see them!
      AllTeamsLookForAll(NO_INTERRUPTS);
    }
  }
}

void CreatePlayerControlledCow() {
  int16_t sWorldX, sWorldY;
  SOLDIERCREATE_STRUCT MercCreateStruct;
  int16_t usMapPos;
  // Get Grid Corrdinates of mouse
  if (GetMouseWorldCoordsInCenter(&sWorldX, &sWorldY) && GetMouseMapPos(&usMapPos)) {
    uint8_t iNewIndex;

    memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
    MercCreateStruct.ubProfile = 12;
    MercCreateStruct.sSectorX = gWorldSectorX;
    MercCreateStruct.sSectorY = gWorldSectorY;
    MercCreateStruct.bSectorZ = gbWorldSectorZ;
    MercCreateStruct.bBodyType = COW;
    MercCreateStruct.sInsertionGridNo = usMapPos;
    MercCreateStruct.bTeam = SOLDIER_CREATE_AUTO_TEAM;
    MercCreateStruct.fPlayerMerc = TRUE;

    RandomizeNewSoldierStats(&MercCreateStruct);

    if (TacticalCreateSoldier(&MercCreateStruct, &iNewIndex)) {
      AddSoldierToSector(iNewIndex);

      // So we can see them!
      AllTeamsLookForAll(NO_INTERRUPTS);
    }
  }
}

void ToggleRealTimeConfirm() {}

void GrenadeTest1() {
  // Get mousexy
  int16_t sX, sY;
  if (GetMouseXY(&sX, &sY)) {
    struct OBJECTTYPE Object;
    Object.usItem = MUSTARD_GRENADE;
    Object.bStatus[0] = 100;
    Object.ubNumberOfObjects = 1;
    CreatePhysicalObject(&Object, 60, (float)(sX * CELL_X_SIZE), (float)(sY * CELL_Y_SIZE), 256,
                         -20, 20, 158, NOBODY, THROW_ARM_ITEM, 0);
  }
}

void GrenadeTest2() {
  // Get mousexy
  int16_t sX, sY;
  if (GetMouseXY(&sX, &sY)) {
    struct OBJECTTYPE Object;
    Object.usItem = HAND_GRENADE;
    Object.bStatus[0] = 100;
    Object.ubNumberOfObjects = 1;
    CreatePhysicalObject(&Object, 60, (float)(sX * CELL_X_SIZE), (float)(sY * CELL_Y_SIZE), 256, 0,
                         -30, 158, NOBODY, THROW_ARM_ITEM, 0);
  }
}

void GrenadeTest3() {
  // Get mousexy
  int16_t sX, sY;
  if (GetMouseXY(&sX, &sY)) {
    struct OBJECTTYPE Object;
    Object.usItem = HAND_GRENADE;
    Object.bStatus[0] = 100;
    Object.ubNumberOfObjects = 1;
    CreatePhysicalObject(&Object, 60, (float)(sX * CELL_X_SIZE), (float)(sY * CELL_Y_SIZE), 256,
                         -10, 10, 158, NOBODY, THROW_ARM_ITEM, 0);
  }
}

void CreatePlayerControlledMonster() {
  int16_t sWorldX, sWorldY;
  int16_t usMapPos;
  if (GetMouseWorldCoordsInCenter(&sWorldX, &sWorldY) && GetMouseMapPos(&usMapPos)) {
    SOLDIERCREATE_STRUCT MercCreateStruct;
    uint8_t iNewIndex;

    memset(&MercCreateStruct, 0, sizeof(MercCreateStruct));
    MercCreateStruct.ubProfile = NO_PROFILE;
    MercCreateStruct.sSectorX = gWorldSectorX;
    MercCreateStruct.sSectorY = gWorldSectorY;
    MercCreateStruct.bSectorZ = gbWorldSectorZ;

    // Note:  only gets called if Alt and/or Ctrl isn't pressed!
    if (_KeyDown(INSERT)) MercCreateStruct.bBodyType = QUEENMONSTER;
    // MercCreateStruct.bBodyType		= LARVAE_MONSTER;
    else
      MercCreateStruct.bBodyType = ADULTFEMALEMONSTER;
    MercCreateStruct.bTeam = SOLDIER_CREATE_AUTO_TEAM;
    MercCreateStruct.sInsertionGridNo = usMapPos;
    RandomizeNewSoldierStats(&MercCreateStruct);

    if (TacticalCreateSoldier(&MercCreateStruct, &iNewIndex)) {
      AddSoldierToSector(iNewIndex);
    }
  }
}

int8_t CheckForAndHandleHandleVehicleInteractiveClick(struct SOLDIERTYPE *pSoldier,
                                                      uint16_t usMapPos, BOOLEAN fMovementMode) {
  // Look for an item pool
  int16_t sActionGridNo;
  uint8_t ubDirection;
  struct SOLDIERTYPE *pTSoldier;
  int16_t sAPCost = 0;

  if (gfUIFullTargetFound) {
    pTSoldier = MercPtrs[gusUIFullTargetID];

    if (OK_ENTERABLE_VEHICLE(pTSoldier) && pTSoldier->bVisible != -1 &&
        OKUseVehicle(pTSoldier->ubProfile)) {
      if ((GetNumberInVehicle(pTSoldier->bVehicleID) == 0) || !fMovementMode) {
        // Find a gridno closest to sweetspot...
        sActionGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(
            pSoldier, pSoldier->usUIMovementMode, 5, &ubDirection, 0, pTSoldier);

        if (sActionGridNo != NOWHERE) {
          // Calculate AP costs...
          // sAPCost = GetAPsToBeginFirstAid( pSoldier );
          sAPCost += PlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, FALSE, TEMPORARY,
                              (uint16_t)pSoldier->usUIMovementMode, NOT_STEALTH, FORWARD,
                              pSoldier->bActionPoints);

          if (EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
            DoMercBattleSound(pSoldier, BATTLE_SOUND_OK1);

            // CHECK IF WE ARE AT THIS GRIDNO NOW
            if (pSoldier->sGridNo != sActionGridNo) {
              // SEND PENDING ACTION
              pSoldier->ubPendingAction = MERC_ENTER_VEHICLE;
              pSoldier->sPendingActionData2 = pTSoldier->sGridNo;
              pSoldier->bPendingActionData3 = ubDirection;
              pSoldier->ubPendingActionAnimCount = 0;

              // WALK UP TO DEST FIRST
              EVENT_InternalGetNewSoldierPath(pSoldier, sActionGridNo, pSoldier->usUIMovementMode,
                                              3, pSoldier->fNoAPToFinishMove);
            } else {
              EVENT_SoldierEnterVehicle(pSoldier, pTSoldier->sGridNo, ubDirection);
            }

            // OK, set UI
            SetUIBusy(pSoldier->ubID);
            // guiPendingOverrideEvent = A_CHANGE_TO_MOVE;

            return (-1);
          }
        }
      }
    }
  }

  return (0);
}

void HandleHandCursorClick(uint16_t usMapPos, uint32_t *puiNewEvent) {
  struct SOLDIERTYPE *pSoldier;
  struct LEVELNODE *pIntTile;
  int16_t sIntTileGridNo;
  int16_t sActionGridNo;
  uint8_t ubDirection;
  int16_t sAPCost;
  int16_t sAdjustedGridNo;
  struct STRUCTURE *pStructure = NULL;
  struct ITEM_POOL *pItemPool;
  BOOLEAN fIgnoreItems = FALSE;

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    // If we are out of breath, no cursor...
    if (pSoldier->bBreath < OKBREATH && pSoldier->bCollapsed) {
      return;
    }

    if (CheckForAndHandleHandleVehicleInteractiveClick(pSoldier, usMapPos, FALSE) == -1) {
      return;
    }

    // Check if we are on a merc... if so.. steal!
    if (gfUIFullTargetFound) {
      if ((guiUIFullTargetFlags & ENEMY_MERC) && !(guiUIFullTargetFlags & UNCONSCIOUS_MERC)) {
        sActionGridNo = FindAdjacentGridEx(pSoldier, MercPtrs[gusUIFullTargetID]->sGridNo,
                                           &ubDirection, &sAdjustedGridNo, TRUE, FALSE);
        if (sActionGridNo == -1) {
          sActionGridNo = sAdjustedGridNo;
        }

        // Steal!
        sAPCost = GetAPsToStealItem(pSoldier, sActionGridNo);

        if (EnoughPoints(pSoldier, sAPCost, 0, TRUE)) {
          MercStealFromMerc(pSoldier, MercPtrs[gusUIFullTargetID]);

          *puiNewEvent = A_CHANGE_TO_MOVE;

          return;
        } else {
          return;
        }
      }
    }

    // Default action gridno to mouse....
    sActionGridNo = usMapPos;

    // If we are over an interactive struct, adjust gridno to this....
    pIntTile =
        ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure, FALSE);
    if (pIntTile != NULL) {
      sActionGridNo = sIntTileGridNo;

      // if ( pStructure->fFlags & ( STRUCTURE_SWITCH | STRUCTURE_ANYDOOR ) )
      if (pStructure->fFlags & (STRUCTURE_SWITCH)) {
        fIgnoreItems = TRUE;
      }

      if (pStructure->fFlags & (STRUCTURE_ANYDOOR) && sActionGridNo != usMapPos) {
        fIgnoreItems = TRUE;
      }
    }

    // Check if we are over an item pool
    // ATE: Ignore items will be set if over a switch interactive tile...
    if (GetItemPool(sActionGridNo, &pItemPool, pSoldier->bLevel) && ITEMPOOL_VISIBLE(pItemPool) &&
        !fIgnoreItems) {
      if (AM_AN_EPC(pSoldier)) {
        // Display message
        // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[EPC_CANNOT_DO_THAT]);
      } else if (UIOkForItemPickup(pSoldier, sActionGridNo)) {
        int8_t bZLevel;

        bZLevel = GetZLevelOfItemPoolGivenStructure(sActionGridNo, pSoldier->bLevel, pStructure);

        SoldierPickupItem(pSoldier, pItemPool->iItemIndex, sActionGridNo, bZLevel);

        *puiNewEvent = A_CHANGE_TO_MOVE;
      }
    } else {
      if (pIntTile != NULL && !(pStructure->fFlags & STRUCTURE_HASITEMONTOP)) {
        sActionGridNo =
            FindAdjacentGridEx(pSoldier, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE);
        if (sActionGridNo == -1) {
          sActionGridNo = sIntTileGridNo;
        }

        // If this is not the same tile as ours, check if we can get to dest!
        if (sActionGridNo != pSoldier->sGridNo && gsCurrentActionPoints == 0) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
        } else {
          if (SelectedMercCanAffordMove()) {
            *puiNewEvent = C_MOVE_MERC;
          }
        }
      } else {
        // ATE: Here, the poor player wants to search something that does not exist...
        // Why should we not let them make fools of themselves....?
        if (AM_AN_EPC(pSoldier)) {
          // Display message
          // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[EPC_CANNOT_DO_THAT]);
        } else {
          // Check morale, if < threashold, refuse...
          if (pSoldier->bMorale < 30) {
            TacticalCharacterDialogue(pSoldier, QUOTE_REFUSING_ORDER);
          } else {
            if (gsCurrentActionPoints == 0) {
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
            } else {
              SoldierPickupItem(pSoldier, NOTHING, sActionGridNo, 0);
              *puiNewEvent = A_CHANGE_TO_MOVE;
            }
          }
        }
      }
    }
  }
}

extern BOOLEAN AnyItemsVisibleOnLevel(struct ITEM_POOL *pItemPool, int8_t bZLevel);

void ExchangeMessageBoxCallBack(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_YES) {
    SwapMercPositions(gpExchangeSoldier1, gpExchangeSoldier2);
  }
}

int8_t HandleMoveModeInteractiveClick(uint16_t usMapPos, uint32_t *puiNewEvent) {
  // Look for an item pool
  struct ITEM_POOL *pItemPool;
  BOOLEAN fContinue = TRUE;
  struct SOLDIERTYPE *pSoldier;
  struct LEVELNODE *pIntTile;
  int16_t sIntTileGridNo;
  int16_t sActionGridNo;
  uint8_t ubDirection;
  int8_t bReturnCode = 0;
  int8_t bZLevel;
  struct STRUCTURE *pStructure = NULL;

  if (GetSoldier(&pSoldier, gusSelectedSoldier)) {
    // If we are out of breath, no cursor...
    // if ( pSoldier->bBreath < OKBREATH )
    //{
    //	  return( -1 );
    //}

    // ATE: If we are a vehicle, no moving!
    if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[VEHICLE_CANT_MOVE_IN_TACTICAL]);
      return (-3);
    }

    // OK, check for height differences.....
    if (gpWorldLevelData[usMapPos].sHeight != gpWorldLevelData[pSoldier->sGridNo].sHeight) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[CANT_GET_THERE]);
      return (-1);
    }

    // See if we are over a vehicle, and walk up to it and enter....
    if (CheckForAndHandleHandleVehicleInteractiveClick(pSoldier, usMapPos, TRUE) == -1) {
      return (-1);
    }

    // Check if we are over a civillian....
    if (gfUIFullTargetFound) {
      if (ValidQuickExchangePosition()) {
        // Check if we can...
        if (CanExchangePlaces(pSoldier, MercPtrs[gusUIFullTargetID], TRUE)) {
          gpExchangeSoldier1 = pSoldier;
          gpExchangeSoldier2 = MercPtrs[gusUIFullTargetID];

          // Do message box...
          // DoMessageBox( MSG_BOX_BASIC_STYLE, TacticalStr[ EXCHANGE_PLACES_REQUESTER ],
          // GAME_SCREEN, ( uint8_t )MSG_BOX_FLAG_YESNO, ExchangeMessageBoxCallBack, NULL );
          SwapMercPositions(gpExchangeSoldier1, gpExchangeSoldier2);
        }
      }
      return (-3);
    }

    pIntTile = GetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure);

    if (pIntTile != NULL) {
      bReturnCode = -3;

      // Check if we are over an item pool, take precedence over that.....
      // EXCEPT FOR SWITCHES!
      if (GetItemPool(sIntTileGridNo, &pItemPool, pSoldier->bLevel) &&
          !(pStructure->fFlags & (STRUCTURE_SWITCH | STRUCTURE_ANYDOOR))) {
        if (AM_AN_EPC(pSoldier)) {
          // Display message
          // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[EPC_CANNOT_DO_THAT]);
          bReturnCode = -1;
        } else if (UIOkForItemPickup(pSoldier, sIntTileGridNo)) {
          bZLevel = GetLargestZLevelOfItemPool(pItemPool);

          if (AnyItemsVisibleOnLevel(pItemPool, bZLevel)) {
            fContinue = FALSE;

            SetUIBusy(pSoldier->ubID);

            if ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.uiFlags & TURNBASED)) {
              // puiNewEvent = C_WAIT_FOR_CONFIRM;
              SoldierPickupItem(pSoldier, pItemPool->iItemIndex, sIntTileGridNo, bZLevel);
            } else {
              BeginDisplayTimedCursor(OKHANDCURSOR_UICURSOR, 300);

              SoldierPickupItem(pSoldier, pItemPool->iItemIndex, sIntTileGridNo, bZLevel);
            }
          }
        }
      }

      if (fContinue) {
        sActionGridNo = FindAdjacentGridEx(MercPtrs[gusSelectedSoldier], sIntTileGridNo,
                                           &ubDirection, NULL, FALSE, TRUE);
        if (sActionGridNo == -1) {
          sActionGridNo = sIntTileGridNo;
        }

        // If this is not the same tile as ours, check if we can get to dest!
        if (sActionGridNo != MercPtrs[gusSelectedSoldier]->sGridNo && gsCurrentActionPoints == 0) {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
          bReturnCode = -1;
        } else {
          bReturnCode = -2;
        }
      }
    }
  }

  return (bReturnCode);
}

BOOLEAN HandleUIReloading(struct SOLDIERTYPE *pSoldier) {
  int8_t bAPs = 0;

  // CHECK OUR CURRENT CURSOR...

  // Do we have the ammo to reload?
  if (guiCurrentUICursor == GOOD_RELOAD_UICURSOR) {
    // Check APs to reload...
    bAPs = GetAPsToAutoReload(pSoldier);

    if (EnoughPoints(pSoldier, bAPs, 0, TRUE)) {
      // OK, we have some ammo we can reload.... reload now!
      if (!AutoReload(pSoldier)) {
        // Do we say we could not reload gun...?
      }

      // ATE: Re-examine cursor info!
      gfUIForceReExamineCursorData = TRUE;
    }
    return (TRUE);
  }

  if (guiCurrentUICursor == BAD_RELOAD_UICURSOR) {
    // OK, we have been told to reload but have no ammo...
    // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, L"No ammo to reload." );
    if (Random(3) == 0) {
      TacticalCharacterDialogue(pSoldier, QUOTE_OUT_OF_AMMO);
    }
    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN ConfirmActionCancel(uint16_t usMapPos, uint16_t usOldMapPos) {
  // OK, most times we want to leave confirm mode if our
  // gridno is different... but if we are in the grenade throw
  // confirm UI, we want a bigger radius...
  // if ( InAimCubeUI( ) )
  //{
  // Calculate distence between both gridnos.....
  //	if ( GetRangeFromGridNoDiff( GetInAimCubeUIGridNo( ), usOldMapPos ) > 1 )
  // if ( usMapPos != usOldMapPos )
  //	{
  //		return( TRUE );
  //	}
  //
  // else
  {
    if (usMapPos != usOldMapPos) {
      return (TRUE);
    }
  }

  return (FALSE);
}

void ChangeCurrentSquad(int32_t iSquad) {
  // only allow if nothing in hand and the Change Squad button for whichever panel we're in must be
  // enabled
  if ((gpItemPointer == NULL) && !gfDisableTacticalPanelButtons &&
      ((gsCurInterfacePanel != TEAM_PANEL) ||
       (ButtonList[iTEAMPanelButtons[CHANGE_SQUAD_BUTTON]]->uiFlags & BUTTON_ENABLED))) {
    if (IsSquadOnCurrentTacticalMap(iSquad)) {
      SetCurrentSquad(iSquad, FALSE);
    }
  }
}

void HandleSelectMercSlot(uint8_t ubPanelSlot, int8_t bCode) {
  uint8_t ubID;

  if (GetPlayerIDFromInterfaceTeamSlot(ubPanelSlot, &ubID)) {
    HandleLocateSelectMerc(ubID, bCode);

    ErasePath(TRUE);
    gfPlotNewMovement = TRUE;
  }
}

void TestMeanWhile(int32_t iID) {
  MEANWHILE_DEFINITION MeanwhileDef;
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  MeanwhileDef.sSectorX = 3;
  MeanwhileDef.sSectorY = 16;
  MeanwhileDef.ubNPCNumber = QUEEN;
  MeanwhileDef.usTriggerEvent = 0;
  MeanwhileDef.ubMeanwhileID = (uint8_t)iID;

  if (iID == INTERROGATION) {
    MeanwhileDef.sSectorX = 7;
    MeanwhileDef.sSectorY = 14;

    // Loop through our mercs and set gridnos once some found.....
    // look for all mercs on the same team,
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pSoldier++) {
      // Are we a POW in this sector?
      if (IsSolActive(pSoldier) && pSoldier->bInSector) {
        ChangeSoldiersAssignment(pSoldier, ASSIGNMENT_POW);

        pSoldier->sSectorX = 7;
        pSoldier->sSectorY = 14;
      }
    }
  }

  ScheduleMeanwhileEvent(&MeanwhileDef, 10);
}

void EscapeUILock() {
  // UNLOCK UI
  UnSetUIBusy((uint8_t)gusSelectedSoldier);

  // Decrease global busy  counter...
  gTacticalStatus.ubAttackBusyCount = 0;

  guiPendingOverrideEvent = LU_ENDUILOCK;
  UIHandleLUIEndLock(NULL);
}

#ifdef JA2BETAVERSION
#include "TileEngine/MapEdgepoints.h"
void ToggleMapEdgepoints() {
#ifdef JA2EDITOR
  static BOOLEAN fToggleEdgepointDisplay = FALSE;
  if (fToggleEdgepointDisplay ^= TRUE) {  // Show edgepoints
    ShowMapEdgepoints();
  } else {  // Hide edgepoints
    HideMapEdgepoints();
  }
  SetRenderFlags(RENDER_FLAG_FULL);
#endif
}
#endif

#ifdef JA2BETAVERSION
BOOLEAN gfMercsNeverQuit = FALSE;
void ToggleMercsNeverQuit() {
  if (gfMercsNeverQuit) {
    gfMercsNeverQuit = FALSE;
    ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Merc contract expiring enabled.");
  } else {
    gfMercsNeverQuit ^= TRUE;
    ScreenMsg(FONT_RED, MSG_BETAVERSION, L"Merc contract expiring disabled.");
  }
}
#endif

void HandleStanceChangeFromUIKeys(uint8_t ubAnimHeight) {
  // If we have multiple guys selected, make all change stance!
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  if (gTacticalStatus.fAtLeastOneGuyOnMultiSelect) {
    // OK, loop through all guys who are 'multi-selected' and
    // check if our currently selected guy is amoung the
    // lucky few.. if not, change to a guy who is...
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pSoldier++) {
      if (IsSolActive(pSoldier) && pSoldier->bInSector) {
        if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
          UIHandleSoldierStanceChange(pSoldier->ubID, ubAnimHeight);
        }
      }
    }
  } else {
    if (gusSelectedSoldier != NO_SOLDIER)
      UIHandleSoldierStanceChange((uint8_t)gusSelectedSoldier, ubAnimHeight);
  }
}

void ToggleStealthMode(struct SOLDIERTYPE *pSoldier) {
  // nothing in hand and either not in SM panel, or the matching button is enabled if we are in SM
  // panel
  if ((gsCurInterfacePanel != SM_PANEL) ||
      (ButtonList[giSMStealthButton]->uiFlags & BUTTON_ENABLED)) {
    // ATE: Toggle stealth
    if (gpSMCurrentMerc != NULL && GetSolID(pSoldier) == gpSMCurrentMerc->ubID) {
      gfUIStanceDifferent = TRUE;
    }

    pSoldier->bStealthMode = !pSoldier->bStealthMode;
    gfPlotNewMovement = TRUE;
    fInterfacePanelDirty = DIRTYLEVEL2;

    if (pSoldier->bStealthMode) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_MERC_ON_STEALTHMODE],
                pSoldier->name);
    } else {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_MERC_OFF_STEALTHMODE],
                pSoldier->name);
    }
  }
}

void HandleStealthChangeFromUIKeys() {
  // If we have multiple guys selected, make all change stance!
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  if (gTacticalStatus.fAtLeastOneGuyOnMultiSelect) {
    // OK, loop through all guys who are 'multi-selected' and
    // check if our currently selected guy is amoung the
    // lucky few.. if not, change to a guy who is...
    cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
         cnt++, pSoldier++) {
      if (IsSolActive(pSoldier) && !AM_A_ROBOT(pSoldier) && pSoldier->bInSector) {
        if (pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED) {
          ToggleStealthMode(pSoldier);
        }
      }
    }
  } else {
    if (gusSelectedSoldier != NO_SOLDIER) {
      if (!AM_A_ROBOT(MercPtrs[gusSelectedSoldier])) {
        ToggleStealthMode(MercPtrs[gusSelectedSoldier]);
      }
    }
  }
}

void TestCapture() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiNumChosen = 0;

  // StartQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );
  // EndQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );

  BeginCaptureSquence();

  gStrategicStatus.uiFlags &= (~STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE);

  // loop through sodliers and pick 3 lucky ones....
  for (cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID, pSoldier = MercPtrs[cnt];
       cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID; cnt++, pSoldier++) {
    if (pSoldier->bLife >= OKLIFE && IsSolActive(pSoldier) && pSoldier->bInSector) {
      if (uiNumChosen < 3) {
        EnemyCapturesPlayerSoldier(pSoldier);

        // Remove them from tectical....
        RemoveSoldierFromGridNo(pSoldier);

        uiNumChosen++;
      }
    }
  }

  EndCaptureSequence();
}

void PopupAssignmentMenuInTactical(struct SOLDIERTYPE *pSoldier) {
  // do something
  fShowAssignmentMenu = TRUE;
  CreateDestroyAssignmentPopUpBoxes();
  SetTacticalPopUpAssignmentBoxXY();
  DetermineBoxPositions();
  DetermineWhichAssignmentMenusCanBeShown();
  fFirstClickInAssignmentScreenMask = TRUE;
  gfIgnoreScrolling = TRUE;
}

#ifdef __GCC
#pragma GCC diagnostic pop
#endif
