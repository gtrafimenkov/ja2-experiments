// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/FOV.h"

#include "SGP/English.h"
#include "SGP/Input.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "SGP/Video.h"
#include "Soldier.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Boxing.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/HandleItems.h"
#include "Tactical/Keys.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/StructureWrap.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/FogOfWar.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/Smell.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/FontControl.h"

/* view directions */
#define DLEFT 0
#define DRIGHT 1
#define UP 2
#define LEFT 3
#define RIGHT 4
#define NOVIEW 5
#define MAXVIEWPATHS 17
#define VIEWPATHLENGTH 13

extern int16_t DirIncrementer[8];

uint8_t gubGridNoMarkers[WORLD_MAX];
uint8_t gubGridNoValue = 254;

#ifdef _DEBUG
uint8_t gubFOVDebugInfoInfo[WORLD_MAX];
#endif

uint8_t ViewPath[MAXVIEWPATHS][VIEWPATHLENGTH] = {
    {NOVIEW, UP, UP, UP, UP, UP, UP, UP, UP, UP, UP, UP, UP},
    {UP, UP, UP, UP, DRIGHT, UP, UP, UP, UP, UP, UP, UP, UP},
    {UP, UP, UP, UP, DLEFT, UP, UP, UP, UP, UP, UP, UP, UP},

    {UP, UP, DLEFT, UP, DLEFT, UP, UP, UP, UP, UP, UP, UP, UP},
    {UP, UP, DRIGHT, UP, DRIGHT, UP, UP, UP, UP, UP, UP, UP, UP},

    {UP, UP, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP, UP, UP},
    {UP, UP, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP, UP, UP},

    {UP, RIGHT, UP, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP, UP},
    {UP, LEFT, UP, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP, UP},

    {DLEFT, DLEFT, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP, UP, UP},
    {DRIGHT, DRIGHT, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP, UP, UP},

    {RIGHT, DRIGHT, DRIGHT, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP, UP},
    {LEFT, DLEFT, DLEFT, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP, UP},

    {DLEFT, LEFT, LEFT, UP, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},
    {LEFT, LEFT, LEFT, UP, LEFT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},

    {DRIGHT, RIGHT, RIGHT, UP, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW,
     NOVIEW},
    {RIGHT, RIGHT, RIGHT, UP, RIGHT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW,
     NOVIEW}};

uint8_t ViewPath2[MAXVIEWPATHS][VIEWPATHLENGTH] = {
    {NOVIEW, UP, UP, UP, UP, UP, UP, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},
    {UP, UP, DLEFT, UP, UP, UP, DLEFT, DRIGHT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},
    {UP, UP, DLEFT, UP, UP, UP, DRIGHT, DLEFT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},

    {UP, UP, DLEFT, UP, UP, DLEFT, DLEFT, UP, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},
    {UP, UP, DRIGHT, UP, UP, DRIGHT, DRIGHT, UP, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW},

    {UP, DLEFT, UP, UP, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP},
    {UP, DRIGHT, UP, UP, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP},

    {DLEFT, DLEFT, UP, UP, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP},
    {DRIGHT, DRIGHT, UP, UP, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP},

    {DLEFT, DLEFT, UP, DLEFT, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP},
    {DRIGHT, DRIGHT, UP, DRIGHT, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP},

    {DLEFT, DLEFT, DLEFT, DLEFT, DLEFT, DLEFT, DLEFT, UP, UP, UP, UP, UP, UP},
    {DRIGHT, DRIGHT, DRIGHT, DRIGHT, DRIGHT, DRIGHT, DRIGHT, UP, UP, UP, UP, UP, UP},

    {DLEFT, LEFT, DLEFT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW,
     NOVIEW},
    {DRIGHT, RIGHT, DRIGHT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW,
     NOVIEW},

    {LEFT, LEFT, DLEFT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW,
     NOVIEW},
    {RIGHT, RIGHT, DRIGHT, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW, NOVIEW,
     NOVIEW}

};

void BuildSightDir(uint32_t dir, uint32_t *One, uint32_t *Two, uint32_t *Three, uint32_t *Four,
                   uint32_t *Five) {
  switch (dir) {
    case NORTH:
      *One = NORTHWEST;
      *Two = NORTHEAST;
      *Three = NORTH;
      *Four = WEST;
      *Five = EAST;
      break;
    case NORTHEAST:
      *One = NORTH;
      *Two = EAST;
      *Three = NORTHEAST;
      *Four = NORTHWEST;
      *Five = SOUTHEAST;
      break;
    case EAST:
      *One = NORTHEAST;
      *Two = SOUTHEAST;
      *Three = EAST;
      *Four = NORTH;
      *Five = SOUTH;
      break;
    case SOUTHEAST:
      *One = EAST;
      *Two = SOUTH;
      *Three = SOUTHEAST;
      *Four = NORTHEAST;
      *Five = SOUTHWEST;
      break;
    case SOUTH:
      *One = SOUTHEAST;
      *Two = SOUTHWEST;
      *Three = SOUTH;
      *Four = EAST;
      *Five = WEST;
      break;
    case SOUTHWEST:
      *One = SOUTH;
      *Two = WEST;
      *Three = SOUTHWEST;
      *Four = SOUTHEAST;
      *Five = NORTHWEST;
      break;
    case WEST:
      *One = SOUTHWEST;
      *Two = NORTHWEST;
      *Three = WEST;
      *Four = SOUTH;
      *Five = NORTH;
      break;
    case NORTHWEST:
      *One = WEST;
      *Two = NORTH;
      *Three = NORTHWEST;
      *Four = SOUTHWEST;
      *Five = NORTHEAST;
      break;
#ifdef BETAVERSION
    default:
      NumMessage("BuildSightDir:  Invalid 'dir' value, = ", dir);
#endif
  }
}

#define NUM_SLANT_ROOF_SLOTS 200

typedef struct {
  int16_t sGridNo;
  BOOLEAN fAllocated;

} SLANT_ROOF_FOV_TYPE;

SLANT_ROOF_FOV_TYPE gSlantRoofData[NUM_SLANT_ROOF_SLOTS];
uint32_t guiNumSlantRoofs = 0;

int32_t GetFreeSlantRoof(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumSlantRoofs; uiCount++) {
    if ((gSlantRoofData[uiCount].fAllocated == FALSE)) return ((int32_t)uiCount);
  }

  if (guiNumSlantRoofs < NUM_SLANT_ROOF_SLOTS) return ((int32_t)guiNumSlantRoofs++);

  return (-1);
}

void RecountSlantRoofs(void) {
  int32_t uiCount;

  for (uiCount = guiNumSlantRoofs - 1; (uiCount >= 0); uiCount--) {
    if ((gSlantRoofData[uiCount].fAllocated)) {
      guiNumSlantRoofs = (uint32_t)(uiCount + 1);
      break;
    }
  }
}

void ClearSlantRoofs(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumSlantRoofs; uiCount++) {
    if ((gSlantRoofData[uiCount].fAllocated)) {
      gSlantRoofData[uiCount].fAllocated = FALSE;
    }
  }

  guiNumSlantRoofs = 0;
}

BOOLEAN FindSlantRoofSlot(int16_t sGridNo) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumSlantRoofs; uiCount++) {
    if ((gSlantRoofData[uiCount].fAllocated)) {
      if (gSlantRoofData[uiCount].sGridNo == sGridNo) {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}

void AddSlantRoofFOVSlot(int16_t sGridNo) {
  int32_t iSlantRoofSlot;
  SLANT_ROOF_FOV_TYPE *pSlantRoof;

  // Check if this is a duplicate!
  if (FindSlantRoofSlot(sGridNo)) {
    return;
  }

  iSlantRoofSlot = GetFreeSlantRoof();

  if (iSlantRoofSlot != -1) {
    pSlantRoof = &gSlantRoofData[iSlantRoofSlot];
    pSlantRoof->sGridNo = sGridNo;
    pSlantRoof->fAllocated = TRUE;
  }
}

void ExamineSlantRoofFOVSlots() {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumSlantRoofs; uiCount++) {
    if ((gSlantRoofData[uiCount].fAllocated)) {
      ExamineGridNoForSlantRoofExtraGraphic(gSlantRoofData[uiCount].sGridNo);
    }
  }

  ClearSlantRoofs();
}

void RevealRoofsAndItems(struct SOLDIERTYPE *pSoldier, uint32_t itemsToo, BOOLEAN fShowLocators,
                         uint8_t ubLevel, BOOLEAN fForce) {
  uint32_t maincnt, markercnt, marker, tilesLeftToSee, cnt, prevmarker;
  uint32_t Inc[6], Dir[6];
  int8_t Blocking, markerDir;
  int8_t nextDir = 0;
  uint8_t dir, range, Path2;
  uint8_t ubRoomNo;
  struct ITEM_POOL *pItemPool;
  BOOLEAN fHiddenStructVisible;
  uint8_t ubMovementCost;
  BOOLEAN fTravelCostObs;
  BOOLEAN fGoneThroughDoor = FALSE;
  BOOLEAN fThroughWindow = FALSE;
  BOOLEAN fItemsQuoteSaid = FALSE;
  uint16_t usIndex;
  BOOLEAN fRevealItems = TRUE;
  BOOLEAN fStopRevealingItemsAfterThisTile = FALSE;
  int8_t bTallestStructureHeight;
  int32_t iDoorGridNo;
  struct STRUCTURE *pStructure, *pDummy;
  int8_t bStructHeight;
  int8_t bThroughWindowDirection;

  if (pSoldier->uiStatusFlags & SOLDIER_ENEMY) {
    // pSoldier->needToLookForItems = FALSE;
    return;
  }

  if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) {
    return;
  }

  // Return if this guy has no gridno, has bad life, etc
  if (pSoldier->sGridNo == NOWHERE || !pSoldier->bInSector || pSoldier->bLife < OKLIFE) {
    return;
  }

  if (pSoldier->bBlindedCounter > 0) {
    return;
  }

  gubGridNoValue++;

  if (gubGridNoValue == 255) {
    // Reset!
    memset(gubGridNoMarkers, 0, sizeof(gubGridNoMarkers));
    gubGridNoValue = 1;
  }

  // OK, look for doors
  MercLooksForDoors(pSoldier, TRUE);

  dir = pSoldier->bDirection;

  // NumMessage("good old reveal",dir);

  // a gassed merc can only see 1 tile away due to blurred vision
  if (pSoldier->uiStatusFlags & SOLDIER_GASSED) {
    range = 1;
  } else {
    range = pSoldier->bViewRange;
    // balance item viewing range between normal and the limit set by opplist-type functions -- CJC
    range = (AdjustMaxSightRangeForEnvEffects(
                 pSoldier, LightTrueLevel(pSoldier->sGridNo, pSoldier->bLevel), range) +
             range) /
            2;
  }

  BuildSightDir(dir, &Dir[0], &Dir[1], &Dir[2], &Dir[3], &Dir[4]);
  for (cnt = 0; cnt < 5; cnt++) Inc[cnt] = DirectionInc((int16_t)Dir[cnt]);

  // create gridno increment for NOVIEW - in other words, no increment!
  Inc[5] = 0;
  Dir[5] = pSoldier->bDirection;

  if (dir % 2 == 1) /* even numbers use ViewPath2 */
    Path2 = TRUE;
  else
    Path2 = FALSE;

  // ATE: if in this special cercumstance... our guys are moving on their own...
  // Stop sighting items
  // IN the future, we may want to do something else here...

  if (gTacticalStatus.uiFlags & OUR_MERCS_AUTO_MOVE) {
    itemsToo = FALSE;
  }

  for (maincnt = 0; maincnt < MAXVIEWPATHS; maincnt++) {
    marker = pSoldier->sGridNo;
    Blocking = FALSE;
    tilesLeftToSee = 99;
    fRevealItems = TRUE;
    fStopRevealingItemsAfterThisTile = FALSE;

#ifdef _DEBUG
    if (_KeyDown(NUM_LOCK)) {
      memset(gubFOVDebugInfoInfo, 0, sizeof(gubFOVDebugInfoInfo));

      SetRenderFlags(RENDER_FLAG_FULL);
      RenderWorld();
    }
#endif

    for (markercnt = 0; markercnt < range; markercnt++) {
      // fGoneThroughDoor = FALSE;
      // fThroughWindow		= FALSE;

      prevmarker = marker;

      nextDir = 99;
      fTravelCostObs = FALSE;
      if (fStopRevealingItemsAfterThisTile) {
        fRevealItems = FALSE;
        fStopRevealingItemsAfterThisTile = FALSE;
      }

      if (Path2) {
        markerDir = ViewPath2[maincnt][markercnt];
        if (markercnt < 12) nextDir = ViewPath2[maincnt][markercnt + 1];
      } else {
        markerDir = ViewPath[maincnt][markercnt];
        if (markercnt < 12) nextDir = ViewPath[maincnt][markercnt + 1];
      }

      // OK, check flags for going through door/window last tile
      if (fThroughWindow == 1) {
        // ATE: Make sure we are going through the same direction!
        // THis is to solve the drassen SAM problem with seeing through walls
        if (Dir[markerDir] == bThroughWindowDirection) {
          fThroughWindow = 2;
        } else {
          fThroughWindow = 0;
        }
      } else if (fThroughWindow == 2) {
        // We've overstayed our welcome - remove!
        fThroughWindow = 0;
      }

      if (fGoneThroughDoor == 1) {
        fGoneThroughDoor = 2;
      } else if (fGoneThroughDoor == 2) {
        // We've overstayed our welcome - remove!
        fGoneThroughDoor = 0;
      }

      // ATE CHECK FOR NOVIEW!
      if (nextDir == NOVIEW) {
        nextDir = 99;
      }

      marker = NewGridNo((int16_t)marker, (int16_t)Inc[markerDir]);

      // End if this is a no view...
      if (markerDir == NOVIEW && markercnt != 0) {
        break;
      }

#ifdef _DEBUG
      if (_KeyDown(NUM_LOCK)) {
        int cnt = GetJA2Clock();

        gubFOVDebugInfoInfo[marker] = (uint8_t)markercnt;

        RenderFOVDebug();

        SetFont(LARGEFONT1);
        SetFontBackground(FONT_MCOLOR_BLACK);
        SetFontForeground(FONT_MCOLOR_WHITE);
        mprintf(10, 10, L"%d", maincnt);
        // mprintf( 10,  20 , L"%d", marker  );
        // mprintf( 50,  20 , L"%d", pSoldier->sGridNo  );

        InvalidateScreen();
        EndFrameBufferRender();
        RefreshScreen(NULL);

        do {
        } while ((GetJA2Clock() - cnt) < 250);
      }
#endif

      // Check if we can get to this gridno from our direction in
      ubMovementCost = gubWorldMovementCosts[marker][Dir[markerDir]][ubLevel];

      // ATE: Added: If our current sector is below ground, ignore any blocks!
      if (gfCaves && ubMovementCost != TRAVELCOST_CAVEWALL) {
        ubMovementCost = TRAVELCOST_FLAT;
      }

      if (IS_TRAVELCOST_DOOR(ubMovementCost)) {
        ubMovementCost = DoorTravelCost(pSoldier, marker, ubMovementCost,
                                        (BOOLEAN)(pSoldier->bTeam == gbPlayerNum), &iDoorGridNo);
        pStructure = FindStructure((int16_t)iDoorGridNo, STRUCTURE_ANYDOOR);
        if (pStructure != NULL && pStructure->fFlags & STRUCTURE_TRANSPARENT) {
          // cell door or somehow otherwise transparent; allow merc to see through
          ubMovementCost = TRAVELCOST_FLAT;
        }
      }

      // If we have hit an obstacle, STOP HERE
      if (ubMovementCost >= TRAVELCOST_BLOCKED) {
        // We have an obstacle here...

        // If it is bigger than a breadbox... err... taller than a man...
        // Then stop path altogether
        // otherwise just stop revealing items

        // CJC:  only do this when the direction is horizontal; easier and faster to check
        // and the effect should still be good enough

        if (ubMovementCost == TRAVELCOST_WALL || ubMovementCost == TRAVELCOST_DOOR ||
            ubMovementCost == TRAVELCOST_EXITGRID) {
          fTravelCostObs = TRUE;
          fRevealItems = FALSE;
        } else {
          // walls are handled above, so the blocking object is guaranteed not to be a wall
          bTallestStructureHeight = GetTallestStructureHeight((int16_t)marker, FALSE);
          if (bTallestStructureHeight >= 3) {
            fTravelCostObs = TRUE;
            fStopRevealingItemsAfterThisTile = TRUE;
          } else if (bTallestStructureHeight != 0) {
            // stop revealing items after this tile but keep going
            fStopRevealingItemsAfterThisTile = TRUE;
          }
        }

        if ((Dir[markerDir] % 2) == 1) {
          // diagonal
          fTravelCostObs = TRUE;
          // cheap hack... don't reveal items
          fRevealItems = FALSE;
        } else {
          bTallestStructureHeight = GetTallestStructureHeight((int16_t)marker, FALSE);
          if (bTallestStructureHeight >= 3) {
            fTravelCostObs = TRUE;
            fStopRevealingItemsAfterThisTile = TRUE;
          } else if (bTallestStructureHeight != 0) {
            // stop revealing items after this tile but keep going
            fStopRevealingItemsAfterThisTile = TRUE;
          }
        }
      }

      // Check if it's been done already!
      if (gubGridNoMarkers[marker] != gubGridNoValue) {
        // Mark gridno
        gubGridNoMarkers[marker] = gubGridNoValue;

        // check and see if the gridno changed
        // if the gridno is the same, avoid redundancy and break
        if (marker == prevmarker && markercnt != 0) {
        } else  // it changed
        {
          // Skip others if we have gone through a door but are too far away....
          if (fGoneThroughDoor) {
            if (markercnt > 5)  // Are we near the door?
            {
              break;
            }
          }
          // DO MINE FINDING STUFF
          // GET INDEX FOR ITEM HERE
          // if there IS a direction after this one, nextdir WILL NOT be 99
          if (nextDir != 99) {
            Blocking = GetBlockingStructureInfo((int16_t)marker, (int8_t)Dir[markerDir],
                                                (int8_t)Dir[nextDir], ubLevel, &bStructHeight,
                                                &pDummy, FALSE);
          } else  // no "next" direction, so pass in a NOWHERE so that
          // "SpecialViewObstruction" will know not to take it UINT32o consideration
          {
            Blocking = GetBlockingStructureInfo((int16_t)marker, (int8_t)Dir[markerDir], (int8_t)30,
                                                ubLevel, &bStructHeight, &pDummy, FALSE);
          }

          if (gfCaves) {
            Blocking = NOTHING_BLOCKING;
          }

          // CHECK FOR ROOMS
          if (Blocking == BLOCKING_TOPLEFT_WINDOW || Blocking == BLOCKING_TOPLEFT_OPEN_WINDOW) {
            // CHECK FACING DIRECTION!
            if (Dir[markerDir] == NORTH || Dir[markerDir] == SOUTH) {
              if (markercnt <= 1)  // Are we right beside it?
              {
                fThroughWindow = TRUE;
                bThroughWindowDirection = (int8_t)Dir[markerDir];
              }
            }
          }
          if (Blocking == BLOCKING_TOPRIGHT_WINDOW || Blocking == BLOCKING_TOPRIGHT_OPEN_WINDOW) {
            // CHECK FACING DIRECTION!
            if (Dir[markerDir] == EAST || Dir[markerDir] == WEST) {
              if (markercnt <= 1)  // Are we right beside it?
              {
                fThroughWindow = TRUE;
                bThroughWindowDirection = (int8_t)Dir[markerDir];
              }
            }
          }

          if (Blocking == BLOCKING_TOPLEFT_DOOR) {
            fGoneThroughDoor = TRUE;
          }
          if (Blocking == BLOCKING_TOPRIGHT_DOOR) {
            fGoneThroughDoor = TRUE;
          }

          // ATE: If we hit this tile, find item always!
          // if (Blocking < FULL_BLOCKING )
          {
            // Handle special things for our mercs, like uncovering roofs
            // and revealing objects...
            // gpSoldier->shad |= SEENBIT;

            // itemVisible = ObjList[itemIndex].visible;

            // NOTE: don't allow object viewing if gassed XXX

            if (itemsToo && fRevealItems)  // && itemIndex < MAXOBJECTLIST)
            {
              // OK, look for corpses...
              LookForAndMayCommentOnSeeingCorpse(pSoldier, (int16_t)marker, ubLevel);

              if (GetItemPool((int16_t)marker, &pItemPool, ubLevel)) {
                if (SetItemPoolVisibilityOn(pItemPool, INVISIBLE, fShowLocators)) {
                  SetRenderFlags(RENDER_FLAG_FULL);

                  if (fShowLocators) {
                    // Set makred render flags
                    // gpWorldLevelData[marker].uiFlags|=MAPELEMENT_REDRAW;
                    // gpWorldLevelData[gusCurMousePos].pTopmostHead->uiFlags |= LEVELNODE_DYNAMIC;

                    // SetRenderFlags(RENDER_FLAG_MARKED);
                    SetRenderFlags(RENDER_FLAG_FULL);

                    // Hault soldier
                    // ATE: Only if in combat...
                    if (gTacticalStatus.uiFlags & INCOMBAT) {
                      HaultSoldierFromSighting(pSoldier, FALSE);
                    } else {
                      // ATE: Make sure we show locators...
                      gTacticalStatus.fLockItemLocators = FALSE;
                    }

                    if (!fItemsQuoteSaid && gTacticalStatus.fLockItemLocators == FALSE) {
                      gTacticalStatus.fLockItemLocators = TRUE;

                      if (gTacticalStatus.ubAttackBusyCount > 0 &&
                          (gTacticalStatus.uiFlags & INCOMBAT)) {
                        gTacticalStatus.fItemsSeenOnAttack = TRUE;
                        gTacticalStatus.ubItemsSeenOnAttackSoldier = GetSolID(pSoldier);
                        gTacticalStatus.usItemsSeenOnAttackGridNo = (int16_t)(marker);
                      } else {
                        // Display quote!
                        if (!AM_AN_EPC(pSoldier)) {
                          TacticalCharacterDialogueWithSpecialEvent(
                              pSoldier, (uint16_t)(QUOTE_SPOTTED_SOMETHING_ONE + Random(2)),
                              DIALOGUE_SPECIAL_EVENT_SIGNAL_ITEM_LOCATOR_START, (int16_t)(marker),
                              0);
                        } else {
                          // Turn off item lock for locators...
                          gTacticalStatus.fLockItemLocators = FALSE;
                          // Slide to location!
                          SlideToLocation(0, (int16_t)(marker));
                        }
                      }
                      fItemsQuoteSaid = TRUE;
                    }
                  }
                }
              }
            }

            // if blood here, let the user see it now...
            // if (ExtGrid[marker].patrolInfo < MAXBLOOD)
            //		gpSoldier->blood = ExtGrid[marker].patrolInfo;

            // DoRoofs(marker,gpSoldier);

            tilesLeftToSee--;
          }

          // CHECK FOR HIDDEN STRUCTS
          // IF we had a hidden struct here that is not visible ( which will still be true because
          // we set it revealed below...
          if (DoesGridnoContainHiddenStruct((uint16_t)marker, &fHiddenStructVisible)) {
            if (!fHiddenStructVisible) {
              gpWorldLevelData[marker].uiFlags |= MAPELEMENT_REDRAW;
              SetRenderFlags(RENDER_FLAG_MARKED);
              RecompileLocalMovementCosts((uint16_t)marker);
            }
          }

          if (tilesLeftToSee <= 0) break;

          if (Blocking == FULL_BLOCKING || (fTravelCostObs && !fThroughWindow)) {
            break;
          }

          // if ( Blocking == NOTHING_BLOCKING || Blocking == BLOCKING_NEXT_TILE )
          if (Blocking == NOTHING_BLOCKING) {
          }

          if (ubLevel != 0) {
          }

          // CHECK FOR SLANT ROOF!
          {
            struct STRUCTURE *pStructure;

            pStructure = FindStructure((int16_t)marker, STRUCTURE_SLANTED_ROOF);

            if (pStructure != NULL) {
              // ADD TO SLANTED ROOF LIST!
              AddSlantRoofFOVSlot((int16_t)marker);
            }
          }

          // Set gridno as revealed
          if (ubLevel == FIRST_LEVEL) {
            if (gfBasement || gfCaves) {
              // OK, if we are underground, we don't want to reveal stuff if
              // 1 ) there is a roof over us and
              // 2 ) we are not in a room
              if (gubWorldRoomInfo[marker] == NO_ROOM &&
                  TypeRangeExistsInRoofLayer(marker, FIRSTROOF, FOURTHROOF, &usIndex)) {
              } else {
                gpWorldLevelData[marker].uiFlags |= MAPELEMENT_REVEALED;
                if (gfCaves) {
                  RemoveFogFromGridNo(marker);
                }
              }
            } else {
              gpWorldLevelData[marker].uiFlags |= MAPELEMENT_REVEALED;
            }

            // CHECK FOR ROOMS
            // if ( fCheckForRooms )
            {
              if (InAHiddenRoom((int16_t)marker, &ubRoomNo)) {
                RemoveRoomRoof((int16_t)marker, ubRoomNo, pSoldier);
                if (ubRoomNo == ROOM_SURROUNDING_BOXING_RING && gWorldSectorX == BOXING_SECTOR_X &&
                    gWorldSectorY == BOXING_SECTOR_Y && gbWorldSectorZ == BOXING_SECTOR_Z) {
                  // reveal boxing ring at same time
                  RemoveRoomRoof((int16_t)marker, BOXING_RING, pSoldier);
                }
              }
            }
          } else {
            gpWorldLevelData[marker].uiFlags |= MAPELEMENT_REVEALED_ROOF;
          }

          // Check for blood....
          UpdateBloodGraphics((int16_t)marker, ubLevel);

          if (Blocking != NOTHING_BLOCKING && Blocking != BLOCKING_TOPLEFT_DOOR &&
              Blocking != BLOCKING_TOPRIGHT_DOOR && Blocking != BLOCKING_TOPLEFT_WINDOW &&
              Blocking != BLOCKING_TOPRIGHT_WINDOW && Blocking != BLOCKING_TOPRIGHT_OPEN_WINDOW &&
              Blocking != BLOCKING_TOPLEFT_OPEN_WINDOW) {
            break;
          }

          // gpWorldLevelData[ marker ].uiFlags |= MAPELEMENT_SHADELAND;
        }
      }  // End of duplicate check
      else {
        if (fTravelCostObs) {
          break;
        }
      }

    }  // end of one path

  }  // end of path loop

  // Loop through all availible slant roofs we collected and perform cool stuff on them
  ExamineSlantRoofFOVSlots();

  // pSoldier->needToLookForItems = FALSE;

  // LookForDoors(pSoldier,UNAWARE);
}

// #endif
