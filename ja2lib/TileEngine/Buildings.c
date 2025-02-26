// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/Buildings.h"

#include "SGP/Random.h"
#include "SGP/Types.h"
#include "Strategic/StrategicMap.h"
#include "SysGlobals.h"
#include "Tactical/Overhead.h"
#include "Tactical/PathAI.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/StructureWrap.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/RenderFun.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"

#define ROOF_LOCATION_CHANCE 8

uint8_t gubBuildingInfo[WORLD_MAX];
BUILDING gBuildings[MAX_BUILDINGS];
uint8_t gubNumberOfBuildings;

BUILDING* CreateNewBuilding(uint8_t* pubBuilding) {
  if (gubNumberOfBuildings + 1 >= MAX_BUILDINGS) {
    return (NULL);
  }
  // increment # of buildings
  gubNumberOfBuildings++;
  // clear entry
  gBuildings[gubNumberOfBuildings].ubNumClimbSpots = 0;
  *pubBuilding = gubNumberOfBuildings;
  // return pointer (have to subtract 1 since we just added 1
  return (&(gBuildings[gubNumberOfBuildings]));
}

BUILDING* GenerateBuilding(int16_t sDesiredSpot) {
  uint32_t uiLoop;
  int16_t sTempGridNo, sNextTempGridNo, sVeryTemporaryGridNo;
  int16_t sStartGridNo, sCurrGridNo, sPrevGridNo = NOWHERE, sRightGridNo;
  int8_t bDirection, bTempDirection;
  BOOLEAN fFoundDir, fFoundWall;
  uint32_t uiChanceIn = ROOF_LOCATION_CHANCE;  // chance of a location being considered
  int16_t sWallGridNo;
  int8_t bDesiredOrientation;
  int8_t bSkipSpots = 0;
  struct SOLDIERTYPE FakeSoldier;
  BUILDING* pBuilding;
  uint8_t ubBuildingID = 0;

  pBuilding = CreateNewBuilding(&ubBuildingID);
  if (!pBuilding) {
    return (NULL);
  }

  // set up fake soldier for location testing
  memset(&FakeSoldier, 0, sizeof(struct SOLDIERTYPE));
  FakeSoldier.sGridNo = sDesiredSpot;
  FakeSoldier.bLevel = 1;
  FakeSoldier.bTeam = 1;

#ifdef ROOF_DEBUG
  memset(gsCoverValue, 0x7F, sizeof(int16_t) * WORLD_MAX);
#endif

  // Set reachable
  RoofReachableTest(sDesiredSpot, ubBuildingID);

  // From sGridNo, search until we find a spot that isn't part of the building
  bDirection = NORTHWEST;
  sTempGridNo = sDesiredSpot;
  // using diagonal directions to hopefully prevent picking a
  // spot that
  while ((gpWorldLevelData[sTempGridNo].uiFlags & MAPELEMENT_REACHABLE)) {
    sNextTempGridNo = NewGridNo(sTempGridNo, DirectionInc(bDirection));
    if (sTempGridNo == sNextTempGridNo) {
      // hit edge of map!??!
      return (NULL);
    } else {
      sTempGridNo = sNextTempGridNo;
    }
  }

  // we've got our spot
  sStartGridNo = sTempGridNo;

  sCurrGridNo = sStartGridNo;
  sVeryTemporaryGridNo = NewGridNo(sCurrGridNo, DirectionInc(EAST));
  if (gpWorldLevelData[sVeryTemporaryGridNo].uiFlags & MAPELEMENT_REACHABLE) {
    // go north first
    bDirection = NORTH;
  } else {
    // go that way (east)
    bDirection = EAST;
  }

  gpWorldLevelData[sStartGridNo].ubExtFlags[0] |= MAPELEMENT_EXT_ROOFCODE_VISITED;

  while (1) {
    // if point to (2 clockwise) is not part of building and is not visited,
    // or is starting point, turn!
    sRightGridNo = NewGridNo(sCurrGridNo, DirectionInc(gTwoCDirection[bDirection]));
    sTempGridNo = sRightGridNo;
    if (((!(gpWorldLevelData[sTempGridNo].uiFlags & MAPELEMENT_REACHABLE) &&
          !(gpWorldLevelData[sTempGridNo].ubExtFlags[0] & MAPELEMENT_EXT_ROOFCODE_VISITED)) ||
         (sTempGridNo == sStartGridNo)) &&
        (sCurrGridNo != sStartGridNo)) {
      bDirection = gTwoCDirection[bDirection];
      // try in that direction
      continue;
    }

    // if spot ahead is part of building, turn
    sTempGridNo = NewGridNo(sCurrGridNo, DirectionInc(bDirection));
    if (gpWorldLevelData[sTempGridNo].uiFlags & MAPELEMENT_REACHABLE) {
      // first search for a spot that is neither part of the building or visited

      // we KNOW that the spot in the original direction is blocked, so only loop 3 times
      bTempDirection = gTwoCDirection[bDirection];
      fFoundDir = FALSE;
      for (uiLoop = 0; uiLoop < 3; uiLoop++) {
        sTempGridNo = NewGridNo(sCurrGridNo, DirectionInc(bTempDirection));
        if (!(gpWorldLevelData[sTempGridNo].uiFlags & MAPELEMENT_REACHABLE) &&
            !(gpWorldLevelData[sTempGridNo].ubExtFlags[0] & MAPELEMENT_EXT_ROOFCODE_VISITED)) {
          // this is the way to go!
          fFoundDir = TRUE;
          break;
        }
        bTempDirection = gTwoCDirection[bTempDirection];
      }
      if (!fFoundDir) {
        // now search for a spot that is just not part of the building
        bTempDirection = gTwoCDirection[bDirection];
        fFoundDir = FALSE;
        for (uiLoop = 0; uiLoop < 3; uiLoop++) {
          sTempGridNo = NewGridNo(sCurrGridNo, DirectionInc(bTempDirection));
          if (!(gpWorldLevelData[sTempGridNo].uiFlags & MAPELEMENT_REACHABLE)) {
            // this is the way to go!
            fFoundDir = TRUE;
            break;
          }
          bTempDirection = gTwoCDirection[bTempDirection];
        }
        if (!fFoundDir) {
          // WTF is going on?
          return (NULL);
        }
      }
      bDirection = bTempDirection;
      // try in that direction
      continue;
    }

    // move ahead
    sPrevGridNo = sCurrGridNo;
    sCurrGridNo = sTempGridNo;
    sRightGridNo = NewGridNo(sCurrGridNo, DirectionInc(gTwoCDirection[bDirection]));

#ifdef ROOF_DEBUG
    if (gsCoverValue[sCurrGridNo] == 0x7F7F) {
      gsCoverValue[sCurrGridNo] = 1;
    } else if (gsCoverValue[sCurrGridNo] >= 0) {
      gsCoverValue[sCurrGridNo]++;
    }

    DebugAI(String("Roof code visits %d", sCurrGridNo));
#endif

    if (sCurrGridNo == sStartGridNo) {
      // done
      break;
    }

    if (!(gpWorldLevelData[sCurrGridNo].ubExtFlags[0] & MAPELEMENT_EXT_ROOFCODE_VISITED)) {
      gpWorldLevelData[sCurrGridNo].ubExtFlags[0] |= MAPELEMENT_EXT_ROOFCODE_VISITED;

      // consider this location as possible climb gridno
      // there must be a regular wall adjacent to this for us to consider it a
      // climb gridno

      // if the direction is east or north, the wall would be in our gridno;
      // if south or west, the wall would be in the gridno two clockwise
      fFoundWall = FALSE;

      switch (bDirection) {
        case NORTH:
          sWallGridNo = sCurrGridNo;
          bDesiredOrientation = OUTSIDE_TOP_RIGHT;
          break;
        case EAST:
          sWallGridNo = sCurrGridNo;
          bDesiredOrientation = OUTSIDE_TOP_LEFT;
          break;
        case SOUTH:
          sWallGridNo = (int16_t)(sCurrGridNo + DirectionInc(gTwoCDirection[bDirection]));
          bDesiredOrientation = OUTSIDE_TOP_RIGHT;
          break;
        case WEST:
          sWallGridNo = (int16_t)(sCurrGridNo + DirectionInc(gTwoCDirection[bDirection]));
          bDesiredOrientation = OUTSIDE_TOP_LEFT;
          break;
        default:
          // what the heck?
          return (NULL);
      }

      if (bDesiredOrientation == OUTSIDE_TOP_LEFT) {
        if (WallExistsOfTopLeftOrientation(sWallGridNo)) {
          fFoundWall = TRUE;
        }
      } else {
        if (WallExistsOfTopRightOrientation(sWallGridNo)) {
          fFoundWall = TRUE;
        }
      }

      if (fFoundWall) {
        if (bSkipSpots > 0) {
          bSkipSpots--;
        } else if (Random(uiChanceIn) == 0) {
          // don't consider people as obstacles
          if (NewOKDestination(&FakeSoldier, sCurrGridNo, FALSE, 0)) {
            pBuilding->sUpClimbSpots[pBuilding->ubNumClimbSpots] = sCurrGridNo;
            pBuilding->sDownClimbSpots[pBuilding->ubNumClimbSpots] = sRightGridNo;
            pBuilding->ubNumClimbSpots++;

            if (pBuilding->ubNumClimbSpots == MAX_CLIMBSPOTS_PER_BUILDING) {
              // gotta stop!
              return (pBuilding);
            }

            // if location is added as a spot, reset uiChanceIn
            uiChanceIn = ROOF_LOCATION_CHANCE;
#ifdef ROOF_DEBUG
            gsCoverValue[sCurrGridNo] = 99;
#endif
            // skip the next spot
            bSkipSpots = 1;
          } else {
            // if location is not added, 100% chance of handling next location
            // and the next until we can add one
            uiChanceIn = 1;
          }
        } else {
          // didn't pick this location, so increase chance that next location
          // will be considered
          if (uiChanceIn > 2) {
            uiChanceIn--;
          }
        }

      } else {
        // can't select this spot
        if ((sPrevGridNo != NOWHERE) && (pBuilding->ubNumClimbSpots > 0)) {
          if (pBuilding->sDownClimbSpots[pBuilding->ubNumClimbSpots - 1] == sCurrGridNo) {
            // unselect previous spot
            pBuilding->ubNumClimbSpots--;
            // overwrote a selected spot so go into automatic selection for later
            uiChanceIn = 1;
#ifdef ROOF_DEBUG
            // reset marker
            gsCoverValue[sPrevGridNo] = 1;
#endif
          }
        }

        // skip the next gridno
        bSkipSpots = 1;
      }
    }
  }

  // at end could prune # of locations if there are too many

  /*
  #ifdef ROOF_DEBUG
          SetRenderFlags( RENDER_FLAG_FULL );
          RenderWorld();
          RenderCoverDebug( );
          InvalidateScreen( );
          EndFrameBufferRender();
          RefreshScreen( NULL );
  #endif
  */

  return (pBuilding);
}

BUILDING* FindBuilding(int16_t sGridNo) {
  uint8_t ubBuildingID;
  // uint8_t					ubRoomNo;

  if (sGridNo <= 0 || sGridNo > WORLD_MAX) {
    return (NULL);
  }

  // id 0 indicates no building
  ubBuildingID = gubBuildingInfo[sGridNo];
  if (ubBuildingID == NO_BUILDING) {
    return (NULL);
    /*
    // need extra checks to see if is valid spot...
    // must have valid room information and be a flat-roofed
    // building
    if ( InARoom( sGridNo, &ubRoomNo ) && (FindStructure( sGridNo, STRUCTURE_NORMAL_ROOF ) != NULL)
    )
    {
            return( GenerateBuilding( sGridNo ) );
    }
    else
    {
            return( NULL );
    }
    */
  } else if (ubBuildingID > gubNumberOfBuildings)  // huh?
  {
    return (NULL);
  }

  return (&(gBuildings[ubBuildingID]));
}

BOOLEAN InBuilding(int16_t sGridNo) {
  if (FindBuilding(sGridNo) == NULL) {
    return (FALSE);
  }
  return (TRUE);
}

void GenerateBuildings(void) {
  uint32_t uiLoop;

  // init building structures and variables
  memset(&gubBuildingInfo, 0, WORLD_MAX * sizeof(uint8_t));
  memset(&gBuildings, 0, MAX_BUILDINGS * sizeof(BUILDING));
  gubNumberOfBuildings = 0;

  if ((gbWorldSectorZ > 0) || gfEditMode) {
    return;
  }

  // reset ALL reachable flags
  // do once before we start building generation for
  // whole map
  for (uiLoop = 0; uiLoop < WORLD_MAX; uiLoop++) {
    gpWorldLevelData[uiLoop].uiFlags &= ~(MAPELEMENT_REACHABLE);
    gpWorldLevelData[uiLoop].ubExtFlags[0] &= ~(MAPELEMENT_EXT_ROOFCODE_VISITED);
  }

  // search through world
  // for each location in a room try to find building info

  for (uiLoop = 0; uiLoop < WORLD_MAX; uiLoop++) {
    if ((gubWorldRoomInfo[uiLoop] != NO_ROOM) && (gubBuildingInfo[uiLoop] == NO_BUILDING) &&
        (FindStructure((int16_t)uiLoop, STRUCTURE_NORMAL_ROOF) != NULL)) {
      GenerateBuilding((int16_t)uiLoop);
    }
  }
}

int16_t FindClosestClimbPoint(int16_t sStartGridNo, int16_t sDesiredGridNo, BOOLEAN fClimbUp) {
  BUILDING* pBuilding;
  uint8_t ubNumClimbSpots;
  int16_t* psClimbSpots;
  uint8_t ubLoop;
  int16_t sDistance, sClosestDistance = 1000, sClosestSpot = NOWHERE;

  pBuilding = FindBuilding(sDesiredGridNo);
  if (!pBuilding) {
    return (NOWHERE);
  }

  ubNumClimbSpots = pBuilding->ubNumClimbSpots;

  if (fClimbUp) {
    psClimbSpots = pBuilding->sUpClimbSpots;
  } else {
    psClimbSpots = pBuilding->sDownClimbSpots;
  }

  for (ubLoop = 0; ubLoop < ubNumClimbSpots; ubLoop++) {
    if ((WhoIsThere2(pBuilding->sUpClimbSpots[ubLoop], 0) == NOBODY) &&
        (WhoIsThere2(pBuilding->sDownClimbSpots[ubLoop], 1) == NOBODY)) {
      sDistance = PythSpacesAway(sStartGridNo, psClimbSpots[ubLoop]);
      if (sDistance < sClosestDistance) {
        sClosestDistance = sDistance;
        sClosestSpot = psClimbSpots[ubLoop];
      }
    }
  }

  return (sClosestSpot);
}

BOOLEAN SameBuilding(int16_t sGridNo1, int16_t sGridNo2) {
  if (gubBuildingInfo[sGridNo1] == NO_BUILDING) {
    return (FALSE);
  }
  if (gubBuildingInfo[sGridNo2] == NO_BUILDING) {
    return (FALSE);
  }
  return ((BOOLEAN)(gubBuildingInfo[sGridNo1] == gubBuildingInfo[sGridNo2]));
}
