// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/Physics.h"

#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/VObject.h"
#include "SGP/WCheck.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Campaign.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/LOS.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/StructureWrap.h"
#include "Tactical/Weapons.h"
#include "Tactical/WorldItems.h"
#include "TileEngine/Environment.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/LightEffects.h"
#include "TileEngine/Structure.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/DebugControl.h"
#include "Utils/EventPump.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"

#define NO_TEST_OBJECT 0
#define TEST_OBJECT_NO_COLLISIONS 1
#define TEST_OBJECT_ANY_COLLISION 2
#define TEST_OBJECT_NOTWALLROOF_COLLISIONS 3

#define OUTDOORS_START_ANGLE (float)(PI / 4)
#define INDOORS_START_ANGLE (float)(PI / 30)
// #define INDOORS_START_ANGLE (float)(
//  0
//)
#define GLAUNCHER_START_ANGLE (float)(PI / 8)
#define GLAUNCHER_HIGHER_LEVEL_START_ANGLE (float)(PI / 6)

#define GET_THROW_HEIGHT(l) (int16_t)((l * 256))
#define GET_SOLDIER_THROW_HEIGHT(l) (int16_t)((l * 256) + STANDING_HEIGHT)

#define GET_OBJECT_LEVEL(z) ((int8_t)((z + 10) / HEIGHT_UNITS))
#define OBJECT_DETONATE_ON_IMPACT(o) \
  ((o->Obj.usItem ==                 \
    MORTAR_SHELL))  // && ( o->ubActionCode == THROW_ARM_ITEM || pObject->fTestObject ) )

#define MAX_INTEGRATIONS 8

#define TIME_MULTI 1.8

// #define					TIME_MULTI			2.2

#define DELTA_T (1.0 * TIME_MULTI)

#define GRAVITY (9.8 * 2.5)
// #define					GRAVITY ( 9.8
//* 2.8
//)

REAL_OBJECT ObjectSlots[NUM_OBJECT_SLOTS];
uint32_t guiNumObjectSlots = 0;
BOOLEAN fDampingActive = FALSE;
// real						Kdl	= (float)0.5;
// // LINEAR DAMPENING ( WIND RESISTANCE )
real Kdl = (float)(0.1 * TIME_MULTI);  // LINEAR DAMPENING ( WIND RESISTANCE )

#define EPSILONV 0.5
#define EPSILONP (real)0.01
#define EPSILONPZ 3

#define CALCULATE_OBJECT_MASS(m) ((float)(m * 2))
#define SCALE_VERT_VAL_TO_HORZ(f) ((f / HEIGHT_UNITS) * CELL_X_SIZE)
#define SCALE_HORZ_VAL_TO_VERT(f) ((f / CELL_X_SIZE) * HEIGHT_UNITS)

void SimulateObject(REAL_OBJECT *pObject, real deltaT);

void CheckForObjectHittingMerc(REAL_OBJECT *pObject, uint16_t usStructureID);
extern void DoGenericHit(struct SOLDIERTYPE *pSoldier, uint8_t ubSpecial, int16_t bDirection);

BOOLEAN PhysicsUpdateLife(REAL_OBJECT *pObject, real DeltaTime);
BOOLEAN PhysicsComputeForces(REAL_OBJECT *pObject);
BOOLEAN PhysicsIntegrate(REAL_OBJECT *pObject, real DeltaTime);
BOOLEAN PhysicsMoveObject(REAL_OBJECT *pObject);
BOOLEAN PhysicsCheckForCollisions(REAL_OBJECT *pObject, int32_t *piCollisionID);
void PhysicsResolveCollision(REAL_OBJECT *pObject, vector_3 *pVelocity, vector_3 *pNormal,
                             real CoefficientOfRestitution);
void PhysicsDeleteObject(REAL_OBJECT *pObject);
BOOLEAN PhysicsHandleCollisions(REAL_OBJECT *pObject, int32_t *piCollisionID, real DeltaTime);
float CalculateForceFromRange(int16_t sRange, float dDegrees);

uint16_t RandomGridFromRadius(int16_t sSweetGridNo, int8_t ubMinRadius, int8_t ubMaxRadius);

void HandleArmedObjectImpact(REAL_OBJECT *pObject);
void ObjectHitWindow(int16_t sGridNo, uint16_t usStructureID, BOOLEAN fBlowWindowSouth,
                     BOOLEAN fLargeForce);
float CalculateObjectTrajectory(int16_t sTargetZ, struct OBJECTTYPE *pItem, vector_3 *vPosition,
                                vector_3 *vForce, int16_t *psFinalGridNo);
vector_3 FindBestForceForTrajectory(int16_t sSrcGridNo, int16_t sGridNo, int16_t sStartZ,
                                    int16_t sEndZ, real dzDegrees, struct OBJECTTYPE *pItem,
                                    int16_t *psGridNo, float *pzMagForce);
int32_t ChanceToGetThroughObjectTrajectory(int16_t sTargetZ, struct OBJECTTYPE *pItem,
                                           vector_3 *vPosition, vector_3 *vForce,
                                           int16_t *psFinalGridNo, int8_t *pbLevel,
                                           BOOLEAN fFromUI);
float CalculateSoldierMaxForce(struct SOLDIERTYPE *pSoldier, float dDegrees,
                               struct OBJECTTYPE *pObject, BOOLEAN fArmed);
BOOLEAN AttemptToCatchObject(REAL_OBJECT *pObject);
BOOLEAN CheckForCatchObject(REAL_OBJECT *pObject);
BOOLEAN DoCatchObject(REAL_OBJECT *pObject);
BOOLEAN CheckForCatcher(REAL_OBJECT *pObject, uint16_t usStructureID);

/// OBJECT POOL FUNCTIONS
int32_t GetFreeObjectSlot(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumObjectSlots; uiCount++) {
    if ((ObjectSlots[uiCount].fAllocated == FALSE)) return ((int32_t)uiCount);
  }

  if (guiNumObjectSlots < NUM_OBJECT_SLOTS) return ((int32_t)guiNumObjectSlots++);

  return (-1);
}

void RecountObjectSlots(void) {
  int32_t uiCount;

  for (uiCount = guiNumObjectSlots - 1; (uiCount >= 0); uiCount--) {
    if ((ObjectSlots[uiCount].fAllocated)) {
      guiNumObjectSlots = (uint32_t)(uiCount + 1);
      return;
    }
  }

  guiNumObjectSlots = 0;
}

int32_t CreatePhysicalObject(struct OBJECTTYPE *pGameObj, real dLifeLength, real xPos, real yPos,
                             real zPos, real xForce, real yForce, real zForce, uint8_t ubOwner,
                             uint8_t ubActionCode, uint32_t uiActionData) {
  int32_t iObjectIndex;
  float mass;
  REAL_OBJECT *pObject;

  if ((iObjectIndex = GetFreeObjectSlot()) == (-1)) return (-1);

  pObject = &(ObjectSlots[iObjectIndex]);

  memset(pObject, 0, sizeof(REAL_OBJECT));

  // OK, GET OBJECT DATA AND COPY
  memcpy(&(pObject->Obj), pGameObj, sizeof(struct OBJECTTYPE));

  // Get mass
  mass = CALCULATE_OBJECT_MASS(Item[pGameObj->usItem].ubWeight);

  // If mass is z, make it something!
  if (mass == 0) {
    mass = 10;
  }

  // OK, mass determines the smoothness of the physics integration
  // For gameplay, we will use mass for maybe max throw distance
  mass = 60;

  // Set lifelength
  pObject->dLifeLength = dLifeLength;

  pObject->fAllocated = TRUE;
  pObject->fAlive = TRUE;
  pObject->fApplyFriction = FALSE;
  pObject->iSoundID = NO_SAMPLE;

  // Set values
  pObject->OneOverMass = 1 / mass;
  pObject->Position.x = xPos;
  pObject->Position.y = yPos;
  pObject->Position.z = zPos;
  pObject->fVisible = TRUE;
  pObject->ubOwner = ubOwner;
  pObject->ubActionCode = ubActionCode;
  pObject->uiActionData = uiActionData;
  pObject->fDropItem = TRUE;
  pObject->ubLastTargetTakenDamage = NOBODY;

  pObject->fFirstTimeMoved = TRUE;

  pObject->InitialForce.x = SCALE_VERT_VAL_TO_HORZ(xForce);
  pObject->InitialForce.y = SCALE_VERT_VAL_TO_HORZ(yForce);
  pObject->InitialForce.z = zForce;

  pObject->InitialForce = VDivScalar(&(pObject->InitialForce), (float)TIME_MULTI);
  pObject->InitialForce = VMultScalar(&(pObject->InitialForce), 1.5);

  // Calculate gridNo
  pObject->sGridNo = MAPROWCOLTOPOS(((int16_t)yPos / CELL_Y_SIZE), ((int16_t)xPos / CELL_X_SIZE));
  pObject->iID = iObjectIndex;
  pObject->pNode = NULL;
  pObject->pShadow = NULL;

  // If gridno not equal to NOWHERE, use sHeight of alnd....
  if (pObject->sGridNo != NOWHERE) {
    pObject->Position.z +=
        CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[pObject->sGridNo].sHeight);
    pObject->EndedWithCollisionPosition.z +=
        CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[pObject->sGridNo].sHeight);
  }

  PhysicsDebugMsg(String("NewPhysics Object"));

  return (iObjectIndex);
}

BOOLEAN RemoveObjectSlot(int32_t iObject) {
  CHECKF(iObject < NUM_OBJECT_SLOTS);

  ObjectSlots[iObject].fAllocated = FALSE;

  RecountObjectSlots();

  return (TRUE);
}

void SimulateWorld() {
  uint32_t cnt;
  REAL_OBJECT *pObject;

  if (COUNTERDONE(PHYSICSUPDATE)) {
    RESETCOUNTER(PHYSICSUPDATE);

    for (cnt = 0; cnt < guiNumObjectSlots; cnt++) {
      // CHECK FOR ALLOCATED
      if (ObjectSlots[cnt].fAllocated) {
        // Get object
        pObject = &(ObjectSlots[cnt]);

        SimulateObject(pObject, (real)DELTA_T);
      }
    }
  }
}

void RemoveAllPhysicsObjects() {
  uint32_t cnt;

  for (cnt = 0; cnt < guiNumObjectSlots; cnt++) {
    // CHECK FOR ALLOCATED
    if (ObjectSlots[cnt].fAllocated) {
      PhysicsDeleteObject(&(ObjectSlots[cnt]));
    }
  }
}

void SimulateObject(REAL_OBJECT *pObject, real deltaT) {
  real DeltaTime = 0;
  real CurrentTime = 0;
  real TargetTime = DeltaTime;
  int32_t iCollisionID;
  BOOLEAN fEndThisObject = FALSE;

  if (!PhysicsUpdateLife(pObject, (float)deltaT)) {
    return;
  }

  if (pObject->fAlive) {
    CurrentTime = 0;
    TargetTime = (float)deltaT;

    // Do subtime here....
    DeltaTime = (float)deltaT / (float)10;

    if (!PhysicsComputeForces(pObject)) {
      return;
    }

    while (CurrentTime < TargetTime) {
      if (!PhysicsIntegrate(pObject, DeltaTime)) {
        fEndThisObject = TRUE;
        break;
      }

      if (!PhysicsHandleCollisions(pObject, &iCollisionID, DeltaTime)) {
        fEndThisObject = TRUE;
        break;
      }

      if (iCollisionID != COLLISION_NONE) {
        break;
      }

      CurrentTime += DeltaTime;
    }

    if (fEndThisObject) {
      return;
    }

    if (!PhysicsMoveObject(pObject)) {
      return;
    }
  }
}

BOOLEAN PhysicsComputeForces(REAL_OBJECT *pObject) {
  vector_3 vTemp;

  // Calculate forces
  pObject->Force = VSetEqual(&(pObject->InitialForce));

  // Note: Only apply gravity if we are not resting on some structure surface
  if (!pObject->fZOnRest) {
    pObject->Force.z -= (real)GRAVITY;
  }

  // Set intial force to zero
  pObject->InitialForce = VMultScalar(&(pObject->InitialForce), 0);

  if (pObject->fApplyFriction) {
    vTemp = VMultScalar(&(pObject->Velocity), -pObject->AppliedMu);
    pObject->Force = VAdd(&(vTemp), &(pObject->Force));

    pObject->fApplyFriction = FALSE;
  }

  if (fDampingActive) {
    vTemp = VMultScalar(&(pObject->Velocity), -Kdl);
    pObject->Force = VAdd(&(vTemp), &(pObject->Force));
  }

  return (TRUE);
}

BOOLEAN PhysicsUpdateLife(REAL_OBJECT *pObject, real DeltaTime) {
  uint8_t bLevel = 0;

  pObject->dLifeSpan += DeltaTime;

  // End life if time has ran out or we are stationary
  if (pObject->dLifeLength != -1) {
    if (pObject->dLifeSpan > pObject->dLifeLength) {
      pObject->fAlive = FALSE;
    }
  }

  // End life if we are out of bounds....
  if (!GridNoOnVisibleWorldTile(pObject->sGridNo)) {
    pObject->fAlive = FALSE;
  }

  if (!pObject->fAlive) {
    pObject->fAlive = FALSE;

    if (!pObject->fTestObject) {
      if (pObject->iSoundID != NO_SAMPLE) {
        SoundStop(pObject->iSoundID);
      }

      if (pObject->ubActionCode == THROW_ARM_ITEM && !pObject->fInWater) {
        HandleArmedObjectImpact(pObject);
      } else {
        // If we are in water, and we are a sinkable item...
        if (!pObject->fInWater || !(Item[pObject->Obj.usItem].fFlags & ITEM_SINKS)) {
          if (pObject->fDropItem) {
            // ATE: If we have collided with roof last...
            if (pObject->iOldCollisionCode == COLLISION_ROOF) {
              bLevel = 1;
            }

            // ATE; If an armed object, don't add....
            if (pObject->ubActionCode != THROW_ARM_ITEM) {
              AddItemToPool(pObject->sGridNo, &(pObject->Obj), 1, bLevel, 0, -1);
            }
          }
        }
      }

      // Make impact noise....
      if (pObject->Obj.usItem == ROCK || pObject->Obj.usItem == ROCK2) {
        MakeNoise(pObject->ubOwner, pObject->sGridNo, 0,
                  gpWorldLevelData[pObject->sGridNo].ubTerrainID, (uint8_t)(9 + PreRandom(9)),
                  NOISE_ROCK_IMPACT);
      } else if (Item[pObject->Obj.usItem].usItemClass & IC_GRENADE) {
        MakeNoise(pObject->ubOwner, pObject->sGridNo, 0,
                  gpWorldLevelData[pObject->sGridNo].ubTerrainID, (uint8_t)(9 + PreRandom(9)),
                  NOISE_GRENADE_IMPACT);
      }

      if (!pObject->fTestObject && pObject->iOldCollisionCode == COLLISION_GROUND) {
        PlayJA2Sample(THROW_IMPACT_2, RATE_11025, SoundVolume(MIDVOLUME, pObject->sGridNo), 1,
                      SoundDir(pObject->sGridNo));
      }

      DebugMsg(
          TOPIC_JA2, DBG_LEVEL_3,
          String("@@@@@@@ Reducing attacker busy count..., PHYSICS OBJECT DONE effect gone off"));
      ReduceAttackBusyCount(pObject->ubOwner, FALSE);

      // ATE: Handle end of animation...
      if (pObject->fCatchAnimOn) {
        struct SOLDIERTYPE *pSoldier;

        pObject->fCatchAnimOn = FALSE;

        // Get intended target
        pSoldier = MercPtrs[pObject->uiActionData];

        // Catch anim.....
        switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
          case ANIM_STAND:

            pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
            EVENT_InitNewSoldierAnim(pSoldier, END_CATCH, 0, FALSE);
            break;

          case ANIM_CROUCH:

            pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
            EVENT_InitNewSoldierAnim(pSoldier, END_CROUCH_CATCH, 0, FALSE);
            break;
        }

        PlayJA2Sample(CATCH_OBJECT, RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                      SoundDir(pSoldier->sGridNo));
      }
    }

    PhysicsDeleteObject(pObject);
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN PhysicsIntegrate(REAL_OBJECT *pObject, real DeltaTime) {
  vector_3 vTemp;

  // Save old position
  pObject->OldPosition = VSetEqual(&(pObject->Position));
  pObject->OldVelocity = VSetEqual(&(pObject->Velocity));

  vTemp = VMultScalar(&(pObject->Velocity), DeltaTime);
  pObject->Position = VAdd(&(pObject->Position), &vTemp);

  // Save test TargetPosition
  if (pObject->fTestPositionNotSet) {
    pObject->TestTargetPosition = VSetEqual(&(pObject->Position));
  }

  vTemp = VMultScalar(&(pObject->Force), (DeltaTime * pObject->OneOverMass));
  pObject->Velocity = VAdd(&(pObject->Velocity), &vTemp);

  if (pObject->fPotentialForDebug) {
    PhysicsDebugMsg(String("Object %d: Force		%f %f %f", pObject->iID, pObject->Force.x,
                           pObject->Force.y, pObject->Force.z));
    PhysicsDebugMsg(String("Object %d: Velocity %f %f %f", pObject->iID, pObject->Velocity.x,
                           pObject->Velocity.y, pObject->Velocity.z));
    PhysicsDebugMsg(String("Object %d: Position %f %f %f", pObject->iID, pObject->Position.x,
                           pObject->Position.y, pObject->Position.z));
    PhysicsDebugMsg(String("Object %d: Delta Pos %f %f %f", pObject->iID,
                           (pObject->OldPosition.x - pObject->Position.x),
                           (pObject->OldPosition.y - pObject->Position.y),
                           (pObject->OldPosition.z - pObject->Position.z)));
  }

  if (pObject->Obj.usItem == MORTAR_SHELL && !pObject->fTestObject &&
      pObject->ubActionCode == THROW_ARM_ITEM) {
    // Start soud if we have reached our max height
    if (pObject->OldVelocity.z >= 0 && pObject->Velocity.z < 0) {
      if (pObject->iSoundID == NO_SAMPLE) {
        pObject->iSoundID = PlayJA2Sample(MORTAR_WHISTLE, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
      }
    }
  }

  return (TRUE);
}

BOOLEAN PhysicsHandleCollisions(REAL_OBJECT *pObject, int32_t *piCollisionID, real DeltaTime) {
  float dDeltaX, dDeltaY, dDeltaZ;

  if (PhysicsCheckForCollisions(pObject, piCollisionID)) {
    dDeltaX = pObject->Position.x - pObject->OldPosition.x;
    dDeltaY = pObject->Position.y - pObject->OldPosition.y;
    dDeltaZ = pObject->Position.z - pObject->OldPosition.z;

    if (dDeltaX <= EPSILONV && dDeltaX >= -EPSILONV && dDeltaY <= EPSILONV &&
        dDeltaY >= -EPSILONV) {
      pObject->sConsecutiveZeroVelocityCollisions++;
    }

    if (pObject->sConsecutiveZeroVelocityCollisions > 3) {
      // We will continue with our Z velocity
      pObject->Velocity.x = 0;
      pObject->Velocity.y = 0;

      // Check that we are not colliding with structure z
      // if ( *piCollisionID == COLLISION_STRUCTURE_Z || *piCollisionID == COLLISION_ROOF )
      if (*piCollisionID == COLLISION_STRUCTURE_Z || *piCollisionID == COLLISION_ROOF ||
          *piCollisionID == COLLISION_GROUND) {
        pObject->Velocity.z = 0;

        // Set us not alive!
        pObject->fAlive = FALSE;
      }

      *piCollisionID = COLLISION_NONE;
    } else {
      // Set position back to before collision
      pObject->Position = VSetEqual(&(pObject->OldPosition));
      // Set old position!
      pObject->OldPosition.x = pObject->Position.y - dDeltaX;
      pObject->OldPosition.y = pObject->Position.x - dDeltaY;
      pObject->OldPosition.z = pObject->Position.z - dDeltaZ;

      PhysicsResolveCollision(pObject, &(pObject->CollisionVelocity), &(pObject->CollisionNormal),
                              pObject->CollisionElasticity);
    }

    if (pObject->Position.z < 0) {
      pObject->Position.z = 0;
    }
    // otherwise, continue falling downwards!

    // TO STOP?

    // Check for delta position values
    if (dDeltaZ <= EPSILONP && dDeltaZ >= -EPSILONP && dDeltaY <= EPSILONP &&
        dDeltaY >= -EPSILONP && dDeltaX <= EPSILONP && dDeltaX >= -EPSILONP) {
      // pObject->fAlive = FALSE;
      // return( FALSE );
    }

    // Check for repeated collisions...
    // if ( pObject->iOldCollisionCode == COLLISION_ROOF || pObject->iOldCollisionCode ==
    // COLLISION_GROUND || pObject->iOldCollisionCode == COLLISION_WATER )
    {
      // ATE: This is a safeguard
      if (pObject->sConsecutiveCollisions > 30) {
        pObject->fAlive = FALSE;
        return (FALSE);
      }
    }

    // Check for -ve velocity still...
    // if ( pObject->Velocity.z <= EPSILONV && pObject->Velocity.z >= -EPSILONV &&
    //		 pObject->Velocity.y <= EPSILONV && pObject->Velocity.y >= -EPSILONV &&
    //		 pObject->Velocity.x <= EPSILONV && pObject->Velocity.x >= -EPSILONV )
    //{
    // PhysicsDeleteObject( pObject );
    //	pObject->fAlive = FALSE;
    //	return( FALSE );
    //}
  }

  return (TRUE);
}

void PhysicsDeleteObject(REAL_OBJECT *pObject) {
  if (pObject->fAllocated) {
    if (pObject->pNode != NULL) {
      RemoveStructFromLevelNode(pObject->sLevelNodeGridNo, pObject->pNode);
    }

    if (pObject->pShadow != NULL) {
      RemoveShadowFromLevelNode(pObject->sLevelNodeGridNo, pObject->pShadow);
    }

    RemoveObjectSlot(pObject->iID);
  }
}

BOOLEAN PhysicsCheckForCollisions(REAL_OBJECT *pObject, int32_t *piCollisionID) {
  vector_3 vTemp;
  float dDeltaX, dDeltaY, dDeltaZ, dX, dY, dZ;
  int32_t iCollisionCode = COLLISION_NONE;
  BOOLEAN fDoCollision = FALSE;
  float dElasity = 1;
  uint16_t usStructureID;
  float dNormalX, dNormalY, dNormalZ;
  int16_t sGridNo;

  // Checkf for collisions
  dX = pObject->Position.x;
  dY = pObject->Position.y;
  dZ = pObject->Position.z;

  vTemp.x = 0;
  vTemp.y = 0;
  vTemp.z = 0;

  dDeltaX = dX - pObject->OldPosition.x;
  dDeltaY = dY - pObject->OldPosition.y;
  dDeltaZ = dZ - pObject->OldPosition.z;

  // Round delta pos to nearest 0.01
  // dDeltaX = (float)( (int)dDeltaX * 100 ) / 100;
  // dDeltaY = (float)( (int)dDeltaY * 100 ) / 100;
  // dDeltaZ = (float)( (int)dDeltaZ * 100 ) / 100;

  // SKIP FIRST GRIDNO, WE'LL COLLIDE WITH OURSELVES....
  if (pObject->fTestObject != TEST_OBJECT_NO_COLLISIONS) {
    iCollisionCode = CheckForCollision(dX, dY, dZ, dDeltaX, dDeltaY, dDeltaZ, &usStructureID,
                                       &dNormalX, &dNormalY, &dNormalZ);
  } else if (pObject->fTestObject == TEST_OBJECT_NO_COLLISIONS) {
    iCollisionCode = COLLISION_NONE;

    // Are we on a downward slope?
    if (dZ < pObject->TestZTarget && dDeltaZ < 0) {
      if (pObject->fTestPositionNotSet) {
        if (pObject->TestZTarget > 32) {
          pObject->fTestPositionNotSet = FALSE;
          pObject->TestZTarget = 0;
        } else {
          iCollisionCode = COLLISION_GROUND;
        }
      } else {
        iCollisionCode = COLLISION_GROUND;
      }
    }
  }

  // If a test object and we have collided with something ( should only be ground ( or roof? ) )
  // Or destination?
  if (pObject->fTestObject == TEST_OBJECT_ANY_COLLISION) {
    if (iCollisionCode != COLLISION_GROUND && iCollisionCode != COLLISION_ROOF &&
        iCollisionCode != COLLISION_WATER && iCollisionCode != COLLISION_NONE) {
      pObject->fTestEndedWithCollision = TRUE;
      pObject->fAlive = FALSE;
      return (FALSE);
    }
  }

  if (pObject->fTestObject == TEST_OBJECT_NOTWALLROOF_COLLISIONS) {
    // So we don't collide with ourselves.....
    if (iCollisionCode != COLLISION_WATER && iCollisionCode != COLLISION_GROUND &&
        iCollisionCode != COLLISION_NONE && iCollisionCode != COLLISION_ROOF &&
        iCollisionCode != COLLISION_INTERIOR_ROOF && iCollisionCode != COLLISION_WALL_SOUTHEAST &&
        iCollisionCode != COLLISION_WALL_SOUTHWEST && iCollisionCode != COLLISION_WALL_NORTHEAST &&
        iCollisionCode != COLLISION_WALL_NORTHWEST) {
      if (pObject->fFirstTimeMoved || pObject->sFirstGridNo == pObject->sGridNo) {
        iCollisionCode = COLLISION_NONE;
      }

      // If we are NOT a wall or window, ignore....
      if (pObject->uiNumTilesMoved < 4) {
        switch (iCollisionCode) {
          case COLLISION_MERC:
          case COLLISION_STRUCTURE:
          case COLLISION_STRUCTURE_Z:

            // Set to no collision ( we shot past )
            iCollisionCode = COLLISION_NONE;
            break;
        }
      }
    }

    switch (iCollisionCode) {
      // End test with any collision NOT a wall, roof...
      case COLLISION_STRUCTURE:
      case COLLISION_STRUCTURE_Z:

        // OK, if it's mercs... don't stop
        if (usStructureID >= INVALID_STRUCTURE_ID) {
          pObject->fTestEndedWithCollision = TRUE;

          if (!pObject->fEndedWithCollisionPositionSet) {
            pObject->fEndedWithCollisionPositionSet = TRUE;
            pObject->EndedWithCollisionPosition = VSetEqual(&(pObject->Position));
          }
          iCollisionCode = COLLISION_NONE;
        } else {
          if (!pObject->fEndedWithCollisionPositionSet) {
            pObject->fEndedWithCollisionPositionSet = TRUE;
            pObject->EndedWithCollisionPosition = VSetEqual(&(pObject->Position));
          }
        }
        break;

      case COLLISION_ROOF:

        if (!pObject->fEndedWithCollisionPositionSet) {
          pObject->fEndedWithCollisionPositionSet = TRUE;
          pObject->EndedWithCollisionPosition = VSetEqual(&(pObject->Position));
        }
        break;

      case COLLISION_WATER:
      case COLLISION_GROUND:
      case COLLISION_MERC:
      case COLLISION_INTERIOR_ROOF:
      case COLLISION_NONE:
      case COLLISION_WINDOW_SOUTHEAST:
      case COLLISION_WINDOW_SOUTHWEST:
      case COLLISION_WINDOW_NORTHEAST:
      case COLLISION_WINDOW_NORTHWEST:

        // Here we just keep going..
        break;

      default:

        // THis is for walls, windows, etc
        // here, we set test ended with collision, but keep going...
        pObject->fTestEndedWithCollision = TRUE;
        break;
    }
  }

  if (pObject->fTestObject != TEST_OBJECT_NOTWALLROOF_COLLISIONS) {
    if (iCollisionCode != COLLISION_WATER && iCollisionCode != COLLISION_GROUND &&
        iCollisionCode != COLLISION_NONE && iCollisionCode != COLLISION_ROOF &&
        iCollisionCode != COLLISION_INTERIOR_ROOF && iCollisionCode != COLLISION_WALL_SOUTHEAST &&
        iCollisionCode != COLLISION_WALL_SOUTHWEST && iCollisionCode != COLLISION_WALL_NORTHEAST &&
        iCollisionCode != COLLISION_WALL_NORTHWEST) {
      // So we don't collide with ourselves.....
      if (pObject->fFirstTimeMoved || pObject->sFirstGridNo == pObject->sGridNo) {
        iCollisionCode = COLLISION_NONE;
      }

      // If we are NOT a wall or window, ignore....
      if (pObject->uiNumTilesMoved < 4) {
        switch (iCollisionCode) {
          case COLLISION_MERC:
          case COLLISION_STRUCTURE:
          case COLLISION_STRUCTURE_Z:

            // Set to no collision ( we shot past )
            iCollisionCode = COLLISION_NONE;
            break;
        }
      }
    }
  }

  *piCollisionID = iCollisionCode;

  // If We hit the ground
  if (iCollisionCode > COLLISION_NONE) {
    if (pObject->iOldCollisionCode == iCollisionCode) {
      pObject->sConsecutiveCollisions++;
    } else {
      pObject->sConsecutiveCollisions = 1;
    }

    if (iCollisionCode == COLLISION_WINDOW_NORTHWEST ||
        iCollisionCode == COLLISION_WINDOW_NORTHEAST ||
        iCollisionCode == COLLISION_WINDOW_SOUTHWEST ||
        iCollisionCode == COLLISION_WINDOW_SOUTHEAST) {
      if (!pObject->fTestObject) {
        // Break window!
        PhysicsDebugMsg(String("Object %d: Collision Window", pObject->iID));

        sGridNo = MAPROWCOLTOPOS(((int16_t)pObject->Position.y / CELL_Y_SIZE),
                                 ((int16_t)pObject->Position.x / CELL_X_SIZE));

        ObjectHitWindow(sGridNo, usStructureID, FALSE, TRUE);
      }
      *piCollisionID = COLLISION_NONE;
      return (FALSE);
    }

    // ATE: IF detonate on impact, stop now!
    if (OBJECT_DETONATE_ON_IMPACT(pObject)) {
      pObject->fAlive = FALSE;
      return (TRUE);
    }

    if (iCollisionCode == COLLISION_GROUND) {
      vTemp.x = 0;
      vTemp.y = 0;
      vTemp.z = -1;

      pObject->fApplyFriction = TRUE;
      // pObject->AppliedMu			= (float)(0.54 * TIME_MULTI );
      pObject->AppliedMu = (float)(0.34 * TIME_MULTI);

      // dElasity = (float)1.5;
      dElasity = (float)1.3;

      fDoCollision = TRUE;

      if (!pObject->fTestObject && !pObject->fHaveHitGround) {
        PlayJA2Sample(THROW_IMPACT_2, RATE_11025, SoundVolume(MIDVOLUME, pObject->sGridNo), 1,
                      SoundDir(pObject->sGridNo));
      }

      pObject->fHaveHitGround = TRUE;
    } else if (iCollisionCode == COLLISION_WATER) {
      ANITILE_PARAMS AniParams;
      ANITILE *pNode;

      // Continue going...
      pObject->fApplyFriction = TRUE;
      pObject->AppliedMu = (float)(1.54 * TIME_MULTI);

      sGridNo = MAPROWCOLTOPOS(((int16_t)pObject->Position.y / CELL_Y_SIZE),
                               ((int16_t)pObject->Position.x / CELL_X_SIZE));

      // Make thing unalive...
      pObject->fAlive = FALSE;

      // If first time...
      if (pObject->fVisible) {
        if (pObject->fTestObject == NO_TEST_OBJECT) {
          // Make invisible
          pObject->fVisible = FALSE;

          // JA25 CJC Oct 13 1999 - if node pointer is null don't try to set flags inside it!
          if (pObject->pNode) {
            pObject->pNode->uiFlags |= LEVELNODE_HIDDEN;
          }

          pObject->fInWater = TRUE;

          // Make ripple
          memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
          AniParams.sGridNo = sGridNo;
          AniParams.ubLevelID = ANI_STRUCT_LEVEL;
          AniParams.usTileType = THIRDMISS;
          AniParams.usTileIndex = THIRDMISS1;
          AniParams.sDelay = 50;
          AniParams.sStartFrame = 0;
          AniParams.uiFlags = ANITILE_FORWARD;

          if (pObject->ubActionCode == THROW_ARM_ITEM) {
            AniParams.ubKeyFrame1 = 11;
            AniParams.uiKeyFrame1Code = ANI_KEYFRAME_CHAIN_WATER_EXPLOSION;
            AniParams.uiUserData = pObject->Obj.usItem;
            AniParams.ubUserData2 = pObject->ubOwner;
          }

          pNode = CreateAnimationTile(&AniParams);

          // Adjust for absolute positioning
          pNode->pLevelNode->uiFlags |= LEVELNODE_USEABSOLUTEPOS;

          pNode->pLevelNode->sRelativeX = (int16_t)pObject->Position.x;
          pNode->pLevelNode->sRelativeY = (int16_t)pObject->Position.y;
          pNode->pLevelNode->sRelativeZ =
              (int16_t)CONVERT_HEIGHTUNITS_TO_PIXELS((int16_t)pObject->Position.z);
        }
      }

    } else if (iCollisionCode == COLLISION_ROOF || iCollisionCode == COLLISION_INTERIOR_ROOF) {
      vTemp.x = 0;
      vTemp.y = 0;
      vTemp.z = -1;

      pObject->fApplyFriction = TRUE;
      pObject->AppliedMu = (float)(0.54 * TIME_MULTI);

      dElasity = (float)1.4;

      fDoCollision = TRUE;

    }
    // else if ( iCollisionCode == COLLISION_INTERIOR_ROOF )
    //{
    //	vTemp.x = 0;
    //	vTemp.y = 0;
    //		vTemp.z = 1;

    //	pObject->fApplyFriction = TRUE;
    //	pObject->AppliedMu			= (float)(0.54 * TIME_MULTI );

    //	dElasity = (float)1.4;

    //	fDoCollision = TRUE;

    //}
    else if (iCollisionCode == COLLISION_STRUCTURE_Z) {
      if (CheckForCatcher(pObject, usStructureID)) {
        return (FALSE);
      }

      CheckForObjectHittingMerc(pObject, usStructureID);

      vTemp.x = 0;
      vTemp.y = 0;
      vTemp.z = -1;

      pObject->fApplyFriction = TRUE;
      pObject->AppliedMu = (float)(0.54 * TIME_MULTI);

      dElasity = (float)1.2;

      fDoCollision = TRUE;

    } else if (iCollisionCode == COLLISION_WALL_SOUTHEAST ||
               iCollisionCode == COLLISION_WALL_SOUTHWEST ||
               iCollisionCode == COLLISION_WALL_NORTHEAST ||
               iCollisionCode == COLLISION_WALL_NORTHWEST) {
      // A wall, do stuff
      vTemp.x = dNormalX;
      vTemp.y = dNormalY;
      vTemp.z = dNormalZ;

      fDoCollision = TRUE;

      dElasity = (float)1.1;
    } else {
      vector_3 vIncident;

      if (CheckForCatcher(pObject, usStructureID)) {
        return (FALSE);
      }

      CheckForObjectHittingMerc(pObject, usStructureID);

      vIncident.x = dDeltaX;
      vIncident.y = dDeltaY;
      vIncident.z = 0;
      // Nomralize

      vIncident = VGetNormal(&vIncident);

      // vTemp.x = -1;
      // vTemp.y = 0;
      // vTemp.z = 0;
      vTemp.x = -1 * vIncident.x;
      vTemp.y = -1 * vIncident.y;
      vTemp.z = 0;

      fDoCollision = TRUE;

      dElasity = (float)1.1;
    }

    if (fDoCollision) {
      pObject->CollisionNormal.x = vTemp.x;
      pObject->CollisionNormal.y = vTemp.y;
      pObject->CollisionNormal.z = vTemp.z;
      pObject->CollisionElasticity = dElasity;
      pObject->iOldCollisionCode = iCollisionCode;

      // Save collision velocity
      pObject->CollisionVelocity = VSetEqual(&(pObject->OldVelocity));

      if (pObject->fPotentialForDebug) {
        PhysicsDebugMsg(String("Object %d: Collision %d", pObject->iID, iCollisionCode));
        PhysicsDebugMsg(String("Object %d: Collision Normal %f %f %f", pObject->iID, vTemp.x,
                               vTemp.y, vTemp.z));
        PhysicsDebugMsg(String("Object %d: Collision OldPos %f %f %f", pObject->iID,
                               pObject->Position.x, pObject->Position.y, pObject->Position.z));
        PhysicsDebugMsg(String("Object %d: Collision Velocity %f %f %f", pObject->iID,
                               pObject->CollisionVelocity.x, pObject->CollisionVelocity.y,
                               pObject->CollisionVelocity.z));
      }

      pObject->fColliding = TRUE;

    } else {
      pObject->fColliding = FALSE;
      pObject->sConsecutiveCollisions = 0;
      pObject->sConsecutiveZeroVelocityCollisions = 0;
      pObject->fHaveHitGround = FALSE;
    }
  }

  return (fDoCollision);
}

void PhysicsResolveCollision(REAL_OBJECT *pObject, vector_3 *pVelocity, vector_3 *pNormal,
                             real CoefficientOfRestitution) {
  real ImpulseNumerator, Impulse;
  vector_3 vTemp;

  ImpulseNumerator = -1 * CoefficientOfRestitution * VDotProduct(pVelocity, pNormal);

  Impulse = ImpulseNumerator;

  vTemp = VMultScalar(pNormal, Impulse);

  pObject->Velocity = VAdd(&(pObject->Velocity), &vTemp);
}

BOOLEAN PhysicsMoveObject(REAL_OBJECT *pObject) {
  struct LEVELNODE *pNode;
  int16_t sNewGridNo, sTileIndex;

  // Determine new gridno
  sNewGridNo = MAPROWCOLTOPOS(((int16_t)pObject->Position.y / CELL_Y_SIZE),
                              ((int16_t)pObject->Position.x / CELL_X_SIZE));

  if (pObject->fFirstTimeMoved) {
    pObject->fFirstTimeMoved = FALSE;
    pObject->sFirstGridNo = sNewGridNo;
  }

  // CHECK FOR RANGE< IF INVALID, REMOVE!
  if (sNewGridNo == -1) {
    PhysicsDeleteObject(pObject);
    return (FALSE);
  }

  // Look at old gridno
  if (sNewGridNo != pObject->sGridNo || pObject->pNode == NULL) {
    if (pObject->fVisible) {
      if (CheckForCatchObject(pObject)) {
        pObject->fVisible = FALSE;
      }
    }

    if (pObject->fVisible) {
      // Add smoke trails...
      if (pObject->Obj.usItem == MORTAR_SHELL && pObject->uiNumTilesMoved > 2 &&
          pObject->ubActionCode == THROW_ARM_ITEM) {
        if (sNewGridNo != pObject->sGridNo) {
          ANITILE_PARAMS AniParams;

          AniParams.sGridNo = (int16_t)sNewGridNo;
          AniParams.ubLevelID = ANI_STRUCT_LEVEL;
          AniParams.sDelay = (int16_t)(100 + PreRandom(100));
          AniParams.sStartFrame = 0;
          AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
          AniParams.sX = (int16_t)pObject->Position.x;
          AniParams.sY = (int16_t)pObject->Position.y;
          AniParams.sZ = (int16_t)CONVERT_HEIGHTUNITS_TO_PIXELS((int16_t)pObject->Position.z);

          strcpy(AniParams.zCachedFile, "TILECACHE\\MSLE_SMK.STI");

          CreateAnimationTile(&AniParams);
        }
      } else if (pObject->uiNumTilesMoved > 0) {
        if (sNewGridNo != pObject->sGridNo) {
          // We're at a new gridno!
          if (pObject->pNode != NULL) {
            RemoveStructFromLevelNode(pObject->sLevelNodeGridNo, pObject->pNode);
          }

          // We're at a new gridno!
          if (pObject->pShadow != NULL) {
            RemoveShadowFromLevelNode(pObject->sLevelNodeGridNo, pObject->pShadow);
          }

          // Now get graphic index
          sTileIndex = GetTileGraphicForItem(&(Item[pObject->Obj.usItem]));
          // sTileIndex = BULLETTILE1;

          // Set new gridno, add
          pNode = AddStructToTail(sNewGridNo, sTileIndex);
          pNode->ubShadeLevel = DEFAULT_SHADE_LEVEL;
          pNode->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
          pNode->uiFlags |= (LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT |
                             LEVELNODE_PHYSICSOBJECT | LEVELNODE_DYNAMIC);

          // Set levelnode
          pObject->pNode = pNode;

          // Add shadow
          AddShadowToHead(sNewGridNo, sTileIndex);
          pNode = gpWorldLevelData[sNewGridNo].pShadowHead;
          pNode->ubShadeLevel = DEFAULT_SHADE_LEVEL;
          pNode->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
          pNode->uiFlags |= (LEVELNODE_USEABSOLUTEPOS | LEVELNODE_IGNOREHEIGHT |
                             LEVELNODE_PHYSICSOBJECT | LEVELNODE_DYNAMIC);

          // Set levelnode
          pObject->pShadow = pNode;

          pObject->sLevelNodeGridNo = sNewGridNo;
        }
      }
    } else {
      // Remove!
      if (pObject->pNode != NULL) {
        RemoveStructFromLevelNode(pObject->sLevelNodeGridNo, pObject->pNode);
      }

      // We're at a new gridno!
      if (pObject->pShadow != NULL) {
        RemoveShadowFromLevelNode(pObject->sLevelNodeGridNo, pObject->pShadow);
      }

      pObject->pNode = NULL;
      pObject->pShadow = NULL;
    }

    if (sNewGridNo != pObject->sGridNo) {
      pObject->uiNumTilesMoved++;
    }

    pObject->sGridNo = sNewGridNo;

    if (pObject->fPotentialForDebug) {
      PhysicsDebugMsg(
          String("Object %d: uiNumTilesMoved: %d", pObject->iID, pObject->uiNumTilesMoved));
    }
  }

  if (pObject->fVisible) {
    if (pObject->Obj.usItem != MORTAR_SHELL || pObject->ubActionCode != THROW_ARM_ITEM) {
      if (pObject->pNode != NULL) {
        // Add new object / update position
        // Update position data
        pObject->pNode->sRelativeX = (int16_t)pObject->Position.x;  // + pTrav->sOffsetX;
        pObject->pNode->sRelativeY = (int16_t)pObject->Position.y;  // + pTrav->sOffsetY;
        pObject->pNode->sRelativeZ =
            (int16_t)CONVERT_HEIGHTUNITS_TO_PIXELS((int16_t)pObject->Position.z);

        // Update position data
        pObject->pShadow->sRelativeX = (int16_t)pObject->Position.x;  // + pTrav->sOffsetX;
        pObject->pShadow->sRelativeY = (int16_t)pObject->Position.y;  // + pTrav->sOffsetY;
        pObject->pShadow->sRelativeZ = (int16_t)gpWorldLevelData[pObject->sGridNo].sHeight;
      }
    }
  }

  return (TRUE);
}

void ObjectHitWindow(int16_t sGridNo, uint16_t usStructureID, BOOLEAN fBlowWindowSouth,
                     BOOLEAN fLargeForce) {
  WindowHit(sGridNo, usStructureID, fBlowWindowSouth, fLargeForce);
}

vector_3 FindBestForceForTrajectory(int16_t sSrcGridNo, int16_t sGridNo, int16_t sStartZ,
                                    int16_t sEndZ, real dzDegrees, struct OBJECTTYPE *pItem,
                                    int16_t *psGridNo, real *pdMagForce) {
  vector_3 vDirNormal, vPosition, vForce;
  int16_t sDestX, sDestY, sSrcX, sSrcY;
  real dForce = 20;
  real dRange;
  real dPercentDiff = 0;
  real dTestRange, dTestDiff;
  int32_t iNumChecks = 0;

  // Get XY from gridno
  ConvertGridNoToCenterCellXY(sGridNo, &sDestX, &sDestY);
  ConvertGridNoToCenterCellXY(sSrcGridNo, &sSrcX, &sSrcY);

  // Set position
  vPosition.x = sSrcX;
  vPosition.y = sSrcY;
  vPosition.z = sStartZ;

  // OK, get direction normal
  vDirNormal.x = (float)(sDestX - sSrcX);
  vDirNormal.y = (float)(sDestY - sSrcY);
  vDirNormal.z = 0;

  // NOmralize
  vDirNormal = VGetNormal(&vDirNormal);

  // From degrees, calculate Z portion of normal
  vDirNormal.z = (float)sin(dzDegrees);

  // Get range
  dRange = (float)GetRangeInCellCoordsFromGridNoDiff(sGridNo, sSrcGridNo);

  // calculate force needed
  { dForce = (float)(12 * (sqrt((GRAVITY * dRange) / sin(2 * dzDegrees)))); }

  do {
    // This first force is just an estimate...
    // now di a binary search to find best value....
    iNumChecks++;

    // Now use a force
    vForce.x = dForce * vDirNormal.x;
    vForce.y = dForce * vDirNormal.y;
    vForce.z = dForce * vDirNormal.z;

    dTestRange = CalculateObjectTrajectory(sEndZ, pItem, &vPosition, &vForce, psGridNo);

    // What's the diff?
    dTestDiff = dTestRange - dRange;

    // How have we done?
    // < 5% off...
    if (fabs((dTestDiff / dRange)) < .01) {
      break;
    }

    if (iNumChecks > MAX_INTEGRATIONS) {
      break;
    }

    // What is the Percentage difference?
    dPercentDiff = dForce * (dTestDiff / dRange);

    // Adjust force accordingly
    dForce = dForce - ((dPercentDiff) / 2);

  } while (TRUE);

  // OK, we have our force, calculate change to get through without collide
  // if ( ChanceToGetThroughObjectTrajectory( sEndZ, pItem, &vPosition, &vForce, NULL ) == 0 )
  {
    // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Chance to get through throw is 0." );
  }

  if (pdMagForce) {
    (*pdMagForce) = dForce;
  }

#ifdef JA2TESTVERSION
  // ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Number of integration: %d", iNumChecks );
#endif

  return (vForce);
}

int16_t FindFinalGridNoGivenDirectionGridNoForceAngle(int16_t sSrcGridNo, int16_t sGridNo,
                                                      int16_t sStartZ, int16_t sEndZ, real dForce,
                                                      real dzDegrees, struct OBJECTTYPE *pItem) {
  vector_3 vDirNormal, vPosition, vForce;
  int16_t sDestX, sDestY, sSrcX, sSrcY;
  int16_t sEndGridNo;

  // Get XY from gridno
  ConvertGridNoToCenterCellXY(sGridNo, &sDestX, &sDestY);
  ConvertGridNoToCenterCellXY(sSrcGridNo, &sSrcX, &sSrcY);

  // Set position
  vPosition.x = sSrcX;
  vPosition.y = sSrcY;
  vPosition.z = sStartZ;

  // OK, get direction normal
  vDirNormal.x = (float)(sDestX - sSrcX);
  vDirNormal.y = (float)(sDestY - sSrcY);
  vDirNormal.z = 0;

  // NOmralize
  vDirNormal = VGetNormal(&vDirNormal);

  // From degrees, calculate Z portion of normal
  vDirNormal.z = (float)sin(dzDegrees);

  // Now use a force
  vForce.x = dForce * vDirNormal.x;
  vForce.y = dForce * vDirNormal.y;
  vForce.z = dForce * vDirNormal.z;

  CalculateObjectTrajectory(sEndZ, pItem, &vPosition, &vForce, &sEndGridNo);

  return (sEndGridNo);
}

real FindBestAngleForTrajectory(int16_t sSrcGridNo, int16_t sGridNo, int16_t sStartZ, int16_t sEndZ,
                                real dForce, struct OBJECTTYPE *pItem, int16_t *psGridNo) {
  vector_3 vDirNormal, vPosition, vForce;
  int16_t sDestX, sDestY, sSrcX, sSrcY;
  real dRange;
  real dzDegrees = ((float)PI / 8);
  real dPercentDiff = 0;
  real dTestRange, dTestDiff;
  int32_t iNumChecks = 0;

  // Get XY from gridno
  ConvertGridNoToCenterCellXY(sGridNo, &sDestX, &sDestY);
  ConvertGridNoToCenterCellXY(sSrcGridNo, &sSrcX, &sSrcY);

  // Set position
  vPosition.x = sSrcX;
  vPosition.y = sSrcY;
  vPosition.z = sStartZ;

  // OK, get direction normal
  vDirNormal.x = (float)(sDestX - sSrcX);
  vDirNormal.y = (float)(sDestY - sSrcY);
  vDirNormal.z = 0;

  // NOmralize
  vDirNormal = VGetNormal(&vDirNormal);

  // From degrees, calculate Z portion of normal
  vDirNormal.z = (float)sin(dzDegrees);

  // Get range
  dRange = (float)GetRangeInCellCoordsFromGridNoDiff(sGridNo, sSrcGridNo);

  do {
    // This first direction is just an estimate...
    // now do a binary search to find best value....
    iNumChecks++;

    // Now use a force
    vForce.x = dForce * vDirNormal.x;
    vForce.y = dForce * vDirNormal.y;
    vForce.z = dForce * vDirNormal.z;

    dTestRange = CalculateObjectTrajectory(sEndZ, pItem, &vPosition, &vForce, psGridNo);

    // What's the diff?
    dTestDiff = dTestRange - dRange;

    // How have we done?
    // < 5% off...
    if (fabs((float)(dTestDiff / dRange)) < .05) {
      break;
    }

    if (iNumChecks > MAX_INTEGRATIONS) {
      break;
    }

    // What is the Percentage difference?
    dPercentDiff = dzDegrees * (dTestDiff / dRange);

    // Adjust degrees accordingly
    dzDegrees = dzDegrees - (dPercentDiff / 2);

    // OK, If our angle is too far either way, giveup!
    if (fabs(dzDegrees) >= (PI / 2) || fabs(dzDegrees) <= 0.005) {
      // Use 0.....
      dzDegrees = 0;
      // From degrees, calculate Z portion of normal
      vDirNormal.z = (float)sin(dzDegrees);
      // Now use a force
      vForce.x = dForce * vDirNormal.x;
      vForce.y = dForce * vDirNormal.y;
      vForce.z = dForce * vDirNormal.z;
      dTestRange = CalculateObjectTrajectory(sEndZ, pItem, &vPosition, &vForce, psGridNo);
      return ((float)(dzDegrees));
    }

    // From degrees, calculate Z portion of normal
    vDirNormal.z = (float)sin(dzDegrees);

  } while (TRUE);

  // OK, we have our force, calculate change to get through without collide
  // if ( ChanceToGetThroughObjectTrajectory( sEndZ, pItem, &vPosition, &vForce ) == 0 )
  //{
  //	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Chance to get through throw is 0." );
  //}

  return (dzDegrees);
}

void FindTrajectory(int16_t sSrcGridNo, int16_t sGridNo, int16_t sStartZ, int16_t sEndZ,
                    real dForce, real dzDegrees, struct OBJECTTYPE *pItem, int16_t *psGridNo) {
  vector_3 vDirNormal, vPosition, vForce;
  int16_t sDestX, sDestY, sSrcX, sSrcY;

  // Get XY from gridno
  ConvertGridNoToCenterCellXY(sGridNo, &sDestX, &sDestY);
  ConvertGridNoToCenterCellXY(sSrcGridNo, &sSrcX, &sSrcY);

  // Set position
  vPosition.x = sSrcX;
  vPosition.y = sSrcY;
  vPosition.z = sStartZ;

  // OK, get direction normal
  vDirNormal.x = (float)(sDestX - sSrcX);
  vDirNormal.y = (float)(sDestY - sSrcY);
  vDirNormal.z = 0;

  // NOmralize
  vDirNormal = VGetNormal(&vDirNormal);

  // From degrees, calculate Z portion of normal
  vDirNormal.z = (float)sin(dzDegrees);

  // Now use a force
  vForce.x = dForce * vDirNormal.x;
  vForce.y = dForce * vDirNormal.y;
  vForce.z = dForce * vDirNormal.z;

  CalculateObjectTrajectory(sEndZ, pItem, &vPosition, &vForce, psGridNo);
}

// OK, this will, given a target Z, INVTYPE, source, target gridnos, initial force vector, will
// return range

float CalculateObjectTrajectory(int16_t sTargetZ, struct OBJECTTYPE *pItem, vector_3 *vPosition,
                                vector_3 *vForce, int16_t *psFinalGridNo) {
  int32_t iID;
  REAL_OBJECT *pObject;
  float dDiffX, dDiffY;
  int16_t sGridNo;

  if (psFinalGridNo) {
    (*psFinalGridNo) = NOWHERE;
  }

  // OK, create a physics object....
  iID = CreatePhysicalObject(pItem, -1, vPosition->x, vPosition->y, vPosition->z, vForce->x,
                             vForce->y, vForce->z, NOBODY, NO_THROW_ACTION, 0);

  if (iID == -1) {
    return (-1);
  }

  pObject = &(ObjectSlots[iID]);

  // Set some special values...
  pObject->fTestObject = TEST_OBJECT_NO_COLLISIONS;
  pObject->TestZTarget = sTargetZ;
  pObject->fTestPositionNotSet = TRUE;
  pObject->fVisible = FALSE;

  // Alrighty, move this beast until it dies....
  while (pObject->fAlive) {
    SimulateObject(pObject, (float)DELTA_T);
  }

  // Calculate gridno from last position
  sGridNo = MAPROWCOLTOPOS(((int16_t)pObject->Position.y / CELL_Y_SIZE),
                           ((int16_t)pObject->Position.x / CELL_X_SIZE));

  PhysicsDeleteObject(pObject);

  // get new x, y, z values
  dDiffX = (pObject->TestTargetPosition.x - vPosition->x);
  dDiffY = (pObject->TestTargetPosition.y - vPosition->y);

  if (psFinalGridNo) {
    (*psFinalGridNo) = sGridNo;
  }

  return ((float)sqrt((dDiffX * dDiffX) + (dDiffY * dDiffY)));
}

int32_t ChanceToGetThroughObjectTrajectory(int16_t sTargetZ, struct OBJECTTYPE *pItem,
                                           vector_3 *vPosition, vector_3 *vForce,
                                           int16_t *psNewGridNo, int8_t *pbLevel, BOOLEAN fFromUI) {
  int32_t iID;
  REAL_OBJECT *pObject;

  // OK, create a physics object....
  iID = CreatePhysicalObject(pItem, -1, vPosition->x, vPosition->y, vPosition->z, vForce->x,
                             vForce->y, vForce->z, NOBODY, NO_THROW_ACTION, 0);

  if (iID == -1) {
    return (-1);
  }

  pObject = &(ObjectSlots[iID]);

  // Set some special values...
  pObject->fTestObject = TEST_OBJECT_NOTWALLROOF_COLLISIONS;
  pObject->fTestPositionNotSet = TRUE;
  pObject->TestZTarget = sTargetZ;
  pObject->fVisible = FALSE;
  // pObject->fPotentialForDebug = TRUE;

  // Alrighty, move this beast until it dies....
  while (pObject->fAlive) {
    SimulateObject(pObject, (float)DELTA_T);
  }

  if (psNewGridNo != NULL) {
    // Calculate gridno from last position

    // If NOT from UI, use exact collision position
    if (fFromUI) {
      (*psNewGridNo) = MAPROWCOLTOPOS(((int16_t)pObject->Position.y / CELL_Y_SIZE),
                                      ((int16_t)pObject->Position.x / CELL_X_SIZE));
    } else {
      (*psNewGridNo) =
          MAPROWCOLTOPOS(((int16_t)pObject->EndedWithCollisionPosition.y / CELL_Y_SIZE),
                         ((int16_t)pObject->EndedWithCollisionPosition.x / CELL_X_SIZE));
    }

    (*pbLevel) =
        GET_OBJECT_LEVEL(pObject->EndedWithCollisionPosition.z -
                         CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[(*psNewGridNo)].sHeight));
  }

  PhysicsDeleteObject(pObject);

  // See If we collided
  if (pObject->fTestEndedWithCollision) {
    return (0);
  }
  return (100);
}

float CalculateLaunchItemAngle(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubHeight,
                               real dForce, struct OBJECTTYPE *pItem, int16_t *psGridNo) {
  real dAngle;
  int16_t sSrcX, sSrcY;

  ConvertGridNoToCenterCellXY(pSoldier->sGridNo, &sSrcX, &sSrcY);

  dAngle = FindBestAngleForTrajectory(pSoldier->sGridNo, sGridNo,
                                      GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel), ubHeight, dForce,
                                      pItem, psGridNo);

  // new we have defaut angle value...
  return (dAngle);
}

void CalculateLaunchItemBasicParams(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pItem,
                                    int16_t sGridNo, uint8_t ubLevel, int16_t sEndZ,
                                    float *pdMagForce, float *pdDegrees, int16_t *psFinalGridNo,
                                    BOOLEAN fArmed) {
  int16_t sInterGridNo;
  int16_t sStartZ;
  float dMagForce, dMaxForce, dMinForce;
  float dDegrees, dNewDegrees;
  BOOLEAN fThroughIntermediateGridNo = FALSE;
  uint16_t usLauncher;
  BOOLEAN fIndoors = FALSE;
  BOOLEAN fLauncher = FALSE;
  BOOLEAN fMortar = FALSE;
  BOOLEAN fGLauncher = FALSE;
  int16_t sMinRange = 0;

  // Start with default degrees/ force
  dDegrees = OUTDOORS_START_ANGLE;
  sStartZ = GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel);

  // Are we armed, and are we throwing a LAUNCHABLE?

  usLauncher = GetLauncherFromLaunchable(pItem->usItem);

  if (fArmed && (usLauncher == MORTAR || pItem->usItem == MORTAR)) {
    // Start at 0....
    sStartZ = (pSoldier->bLevel * 256);
    fMortar = TRUE;
    sMinRange = MIN_MORTAR_RANGE;
    // fLauncher = TRUE;
  }

  if (fArmed && (usLauncher == GLAUNCHER || usLauncher == UNDER_GLAUNCHER ||
                 pItem->usItem == GLAUNCHER || pItem->usItem == UNDER_GLAUNCHER)) {
    // OK, look at target level and decide angle to use...
    if (ubLevel == 1) {
      // dDegrees  = GLAUNCHER_START_ANGLE;
      dDegrees = GLAUNCHER_HIGHER_LEVEL_START_ANGLE;
    } else {
      dDegrees = GLAUNCHER_START_ANGLE;
    }
    fGLauncher = TRUE;
    sMinRange = MIN_MORTAR_RANGE;
    // fLauncher = TRUE;
  }

  // CHANGE DEGREE VALUES BASED ON IF WE ARE INSIDE, ETC
  // ARE WE INSIDE?

  if (gfCaves || gfBasement) {
    // Adjust angle....
    dDegrees = INDOORS_START_ANGLE;
    fIndoors = TRUE;
  }

  if ((IsRoofPresentAtGridno(pSoldier->sGridNo)) && pSoldier->bLevel == 0) {
    // Adjust angle....
    dDegrees = INDOORS_START_ANGLE;
    fIndoors = TRUE;
  }

  // IS OUR TARGET INSIDE?
  if (IsRoofPresentAtGridno(sGridNo) && ubLevel == 0) {
    // Adjust angle....
    dDegrees = INDOORS_START_ANGLE;
    fIndoors = TRUE;
  }

  // OK, look if we can go through a windows here...
  if (ubLevel == 0) {
    sInterGridNo = SoldierToLocationWindowTest(pSoldier, sGridNo);
  } else {
    sInterGridNo = NOWHERE;
  }

  if (sInterGridNo != NOWHERE) {
    // IF so, adjust target height, gridno....
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Through a window!");

    fThroughIntermediateGridNo = TRUE;
  }

  if (!fLauncher) {
    // Find force for basic
    FindBestForceForTrajectory(pSoldier->sGridNo, sGridNo, sStartZ, sEndZ, dDegrees, pItem,
                               psFinalGridNo, &dMagForce);

    // Adjust due to max range....
    dMaxForce = CalculateSoldierMaxForce(pSoldier, dDegrees, pItem, fArmed);

    if (fIndoors) {
      dMaxForce = dMaxForce * 2;
    }

    if (dMagForce > dMaxForce) {
      dMagForce = dMaxForce;
    }

    // ATE: If we are a mortar, make sure we are at min.
    if (fMortar || fGLauncher) {
      // find min force
      dMinForce = CalculateForceFromRange((int16_t)(sMinRange / 10), (float)(PI / 4));

      if (dMagForce < dMinForce) {
        dMagForce = dMinForce;
      }
    }

    if (fThroughIntermediateGridNo) {
      // Given this power, now try and go through this window....
      dDegrees = FindBestAngleForTrajectory(pSoldier->sGridNo, sInterGridNo,
                                            GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel), 150,
                                            dMagForce, pItem, psFinalGridNo);
    }
  } else {
    // Use MAX force, vary angle....
    dMagForce = CalculateSoldierMaxForce(pSoldier, dDegrees, pItem, fArmed);

    if (ubLevel == 0) {
      dMagForce = (float)(dMagForce * 1.25);
    }

    FindTrajectory(pSoldier->sGridNo, sGridNo, sStartZ, sEndZ, dMagForce, dDegrees, pItem,
                   psFinalGridNo);

    if (ubLevel == 1 && !fThroughIntermediateGridNo) {
      // Is there a guy here...?
      if (WhoIsThere2(sGridNo, ubLevel) != NO_SOLDIER) {
        dMagForce = (float)(dMagForce * 0.85);

        // Yep, try to get angle...
        dNewDegrees = FindBestAngleForTrajectory(pSoldier->sGridNo, sGridNo,
                                                 GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel), 150,
                                                 dMagForce, pItem, psFinalGridNo);

        if (dNewDegrees != 0) {
          dDegrees = dNewDegrees;
        }
      }
    }

    if (fThroughIntermediateGridNo) {
      dDegrees = FindBestAngleForTrajectory(pSoldier->sGridNo, sInterGridNo,
                                            GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel), 150,
                                            dMagForce, pItem, psFinalGridNo);
    }
  }

  (*pdMagForce) = dMagForce;
  (*pdDegrees) = dDegrees;
}

BOOLEAN CalculateLaunchItemChanceToGetThrough(struct SOLDIERTYPE *pSoldier,
                                              struct OBJECTTYPE *pItem, int16_t sGridNo,
                                              uint8_t ubLevel, int16_t sEndZ,
                                              int16_t *psFinalGridNo, BOOLEAN fArmed,
                                              int8_t *pbLevel, BOOLEAN fFromUI) {
  float dForce, dDegrees;
  int16_t sDestX, sDestY, sSrcX, sSrcY;
  vector_3 vForce, vPosition, vDirNormal;

  // Ge7t basic launch params...
  CalculateLaunchItemBasicParams(pSoldier, pItem, sGridNo, ubLevel, sEndZ, &dForce, &dDegrees,
                                 psFinalGridNo, fArmed);

  // Get XY from gridno
  ConvertGridNoToCenterCellXY(sGridNo, &sDestX, &sDestY);
  ConvertGridNoToCenterCellXY(pSoldier->sGridNo, &sSrcX, &sSrcY);

  // Set position
  vPosition.x = sSrcX;
  vPosition.y = sSrcY;
  vPosition.z = GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel);

  // OK, get direction normal
  vDirNormal.x = (float)(sDestX - sSrcX);
  vDirNormal.y = (float)(sDestY - sSrcY);
  vDirNormal.z = 0;

  // NOmralize
  vDirNormal = VGetNormal(&vDirNormal);

  // From degrees, calculate Z portion of normal
  vDirNormal.z = (float)sin(dDegrees);

  // Do force....
  vForce.x = dForce * vDirNormal.x;
  vForce.y = dForce * vDirNormal.y;
  vForce.z = dForce * vDirNormal.z;

  // OK, we have our force, calculate change to get through without collide
  if (ChanceToGetThroughObjectTrajectory(sEndZ, pItem, &vPosition, &vForce, psFinalGridNo, pbLevel,
                                         fFromUI) == 0) {
    return (FALSE);
  }

  if ((*pbLevel) != ubLevel) {
    return (FALSE);
  }

  if (!fFromUI && (*psFinalGridNo) != sGridNo) {
    return (FALSE);
  }

  return (TRUE);
}

float CalculateForceFromRange(int16_t sRange, float dDegrees) {
  float dMagForce;
  int16_t sSrcGridNo, sDestGridNo;
  struct OBJECTTYPE Object;
  int16_t sFinalGridNo;

  // OK, use a fake gridno, find the new gridno based on range, use height of merc, end height of
  // ground, 45 degrees
  sSrcGridNo = 4408;
  sDestGridNo = 4408 + (sRange * WORLD_COLS);

  // Use a grenade objecttype
  CreateItem(HAND_GRENADE, 100, &Object);

  FindBestForceForTrajectory(sSrcGridNo, sDestGridNo, GET_SOLDIER_THROW_HEIGHT(0), 0, dDegrees,
                             &Object, &sFinalGridNo, &dMagForce);

  return (dMagForce);
}

float CalculateSoldierMaxForce(struct SOLDIERTYPE *pSoldier, float dDegrees,
                               struct OBJECTTYPE *pItem, BOOLEAN fArmed) {
  int32_t uiMaxRange;
  float dMagForce;

  dDegrees = (float)(PI / 4);

  uiMaxRange = CalcMaxTossRange(pSoldier, pItem->usItem, fArmed);

  dMagForce = CalculateForceFromRange((int16_t)uiMaxRange, dDegrees);

  return (dMagForce);
}

#define MAX_MISS_BY 30
#define MIN_MISS_BY 1
#define MAX_MISS_RADIUS 5

void CalculateLaunchItemParamsForThrow(struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                       uint8_t ubLevel, int16_t sEndZ, struct OBJECTTYPE *pItem,
                                       int8_t bMissBy, uint8_t ubActionCode,
                                       uint32_t uiActionData) {
  float dForce, dDegrees;
  int16_t sDestX, sDestY, sSrcX, sSrcY;
  vector_3 vForce, vDirNormal;
  int16_t sFinalGridNo;
  BOOLEAN fArmed = FALSE;
  uint16_t usLauncher;
  int16_t sStartZ;
  int8_t bMinMissRadius, bMaxMissRadius, bMaxRadius;
  float fScale;

  // Set target ID if anyone
  pSoldier->ubTargetID = WhoIsThere2(sGridNo, ubLevel);

  if (ubActionCode == THROW_ARM_ITEM) {
    fArmed = TRUE;
  }

  if (bMissBy < 0) {
    // then we hit!
    bMissBy = 0;
  }

  // if ( 0 )
  if (bMissBy > 0) {
    // Max the miss variance
    if (bMissBy > MAX_MISS_BY) {
      bMissBy = MAX_MISS_BY;
    }

    // Min the miss varience...
    if (bMissBy < MIN_MISS_BY) {
      bMissBy = MIN_MISS_BY;
    }

    // Adjust position, force, angle
#ifdef JA2TESTVERSION
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Throw miss by: %d", bMissBy);
#endif

    // Default to max radius...
    bMaxRadius = 5;

    // scale if pyth spaces away is too far
    if (PythSpacesAway(sGridNo, pSoldier->sGridNo) < ((float)bMaxRadius / (float)1.5)) {
      bMaxRadius = PythSpacesAway(sGridNo, pSoldier->sGridNo) / 2;
    }

    // Get radius
    fScale = ((float)bMissBy / (float)MAX_MISS_BY);

    bMaxMissRadius = (int8_t)(bMaxRadius * fScale);

    // Limit max radius...
    if (bMaxMissRadius > 4) {
      bMaxMissRadius = 4;
    }

    bMinMissRadius = bMaxMissRadius - 1;

    if (bMinMissRadius < 2) {
      bMinMissRadius = 2;
    }

    if (bMaxMissRadius < bMinMissRadius) {
      bMaxMissRadius = bMinMissRadius;
    }

    sGridNo = RandomGridFromRadius(sGridNo, bMinMissRadius, bMaxMissRadius);
  }

  // Get basic launch params...
  CalculateLaunchItemBasicParams(pSoldier, pItem, sGridNo, ubLevel, sEndZ, &dForce, &dDegrees,
                                 &sFinalGridNo, fArmed);

  // Get XY from gridno
  ConvertGridNoToCenterCellXY(sGridNo, &sDestX, &sDestY);
  ConvertGridNoToCenterCellXY(pSoldier->sGridNo, &sSrcX, &sSrcY);

  // OK, get direction normal
  vDirNormal.x = (float)(sDestX - sSrcX);
  vDirNormal.y = (float)(sDestY - sSrcY);
  vDirNormal.z = 0;

  // NOmralize
  vDirNormal = VGetNormal(&vDirNormal);

  // From degrees, calculate Z portion of normal
  vDirNormal.z = (float)sin(dDegrees);

  // Do force....
  vForce.x = dForce * vDirNormal.x;
  vForce.y = dForce * vDirNormal.y;
  vForce.z = dForce * vDirNormal.z;

  // Allocate Throw Parameters
  pSoldier->pThrowParams = (THROW_PARAMS *)MemAlloc(sizeof(THROW_PARAMS));
  memset(pSoldier->pThrowParams, 0, sizeof(THROW_PARAMS));

  pSoldier->pTempObject = (struct OBJECTTYPE *)MemAlloc(sizeof(struct OBJECTTYPE));

  memcpy(pSoldier->pTempObject, pItem, sizeof(struct OBJECTTYPE));
  pSoldier->pThrowParams->dX = (float)sSrcX;
  pSoldier->pThrowParams->dY = (float)sSrcY;

  sStartZ = GET_SOLDIER_THROW_HEIGHT(pSoldier->bLevel);
  usLauncher = GetLauncherFromLaunchable(pItem->usItem);
  if (fArmed && usLauncher == MORTAR) {
    // Start at 0....
    sStartZ = (pSoldier->bLevel * 256) + 50;
  }

  pSoldier->pThrowParams->dZ = (float)sStartZ;
  pSoldier->pThrowParams->dForceX = vForce.x;
  pSoldier->pThrowParams->dForceY = vForce.y;
  pSoldier->pThrowParams->dForceZ = vForce.z;
  pSoldier->pThrowParams->dLifeSpan = -1;
  pSoldier->pThrowParams->ubActionCode = ubActionCode;
  pSoldier->pThrowParams->uiActionData = uiActionData;

  // Dirty interface
  DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
}

BOOLEAN CheckForCatcher(REAL_OBJECT *pObject, uint16_t usStructureID) {
  // Do we want to catch?
  if (pObject->fTestObject == NO_TEST_OBJECT) {
    if (pObject->ubActionCode == THROW_TARGET_MERC_CATCH) {
      // Is it a guy?
      if (usStructureID < INVALID_STRUCTURE_ID) {
        // Is it the same guy?
        if (usStructureID == pObject->uiActionData) {
          if (DoCatchObject(pObject)) {
            pObject->fAlive = FALSE;
            return (TRUE);
          }
        }
      }
    }
  }
  return (FALSE);
}

void CheckForObjectHittingMerc(REAL_OBJECT *pObject, uint16_t usStructureID) {
  struct SOLDIERTYPE *pSoldier;
  int16_t sDamage, sBreath;

  // Do we want to catch?
  if (pObject->fTestObject == NO_TEST_OBJECT) {
    // Is it a guy?
    if (usStructureID < INVALID_STRUCTURE_ID) {
      if (pObject->ubLastTargetTakenDamage != (uint8_t)usStructureID) {
        pSoldier = MercPtrs[usStructureID];

        sDamage = 1;
        sBreath = 0;

        EVENT_SoldierGotHit(pSoldier, NOTHING, sDamage, sBreath, pSoldier->bDirection, 0,
                            pObject->ubOwner, FIRE_WEAPON_TOSSED_OBJECT_SPECIAL, 0, 0, NOWHERE);

        pObject->ubLastTargetTakenDamage = (uint8_t)(usStructureID);
      }
    }
  }
}

BOOLEAN CheckForCatchObject(REAL_OBJECT *pObject) {
  struct SOLDIERTYPE *pSoldier;
  uint32_t uiSpacesAway;

  // Do we want to catch?
  if (pObject->fTestObject == NO_TEST_OBJECT) {
    if (pObject->ubActionCode == THROW_TARGET_MERC_CATCH) {
      pSoldier = MercPtrs[pObject->uiActionData];

      // Is it a guy?
      // Are we close to this guy?
      uiSpacesAway = PythSpacesAway(pObject->sGridNo, pSoldier->sGridNo);

      if (uiSpacesAway < 4 && !pObject->fAttemptedCatch) {
        if (pSoldier->usAnimState != CATCH_STANDING && pSoldier->usAnimState != CATCH_CROUCHED &&
            pSoldier->usAnimState != LOWER_RIFLE) {
          if (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_STAND) {
            EVENT_InitNewSoldierAnim(pSoldier, CATCH_STANDING, 0, FALSE);
          } else if (gAnimControl[pSoldier->usAnimState].ubHeight == ANIM_CROUCH) {
            EVENT_InitNewSoldierAnim(pSoldier, CATCH_CROUCHED, 0, FALSE);
          }

          pObject->fCatchAnimOn = TRUE;
        }
      }

      pObject->fAttemptedCatch = TRUE;

      if (uiSpacesAway <= 1 && !pObject->fCatchCheckDone) {
        if (AttemptToCatchObject(pObject)) {
          return (TRUE);
        }
      }
    }
  }
  return (FALSE);
}

BOOLEAN AttemptToCatchObject(REAL_OBJECT *pObject) {
  struct SOLDIERTYPE *pSoldier;
  uint8_t ubChanceToCatch;

  // Get intended target
  pSoldier = MercPtrs[pObject->uiActionData];

  // OK, get chance to catch
  // base it on...? CC? Dexterity?
  ubChanceToCatch = 50 + EffectiveDexterity(pSoldier) / 2;

#ifdef JA2TESTVERSION
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Chance To Catch: %d", ubChanceToCatch);
#endif

  pObject->fCatchCheckDone = TRUE;

  if (PreRandom(100) > ubChanceToCatch) {
    return (FALSE);
  }

  pObject->fCatchGood = TRUE;

  return (TRUE);
}

BOOLEAN DoCatchObject(REAL_OBJECT *pObject) {
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fGoodCatch = FALSE;
  uint16_t usItem;

  // Get intended target
  pSoldier = MercPtrs[pObject->uiActionData];

  // Catch anim.....
  switch (gAnimControl[pSoldier->usAnimState].ubHeight) {
    case ANIM_STAND:

      pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
      EVENT_InitNewSoldierAnim(pSoldier, END_CATCH, 0, FALSE);
      break;

    case ANIM_CROUCH:

      pSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
      EVENT_InitNewSoldierAnim(pSoldier, END_CROUCH_CATCH, 0, FALSE);
      break;
  }

  PlayJA2Sample(CATCH_OBJECT, RATE_11025, SoundVolume(MIDVOLUME, pSoldier->sGridNo), 1,
                SoundDir(pSoldier->sGridNo));

  pObject->fCatchAnimOn = FALSE;

  if (!pObject->fCatchGood) {
    return (FALSE);
  }

  // Get item
  usItem = pObject->Obj.usItem;

  // Transfer object
  fGoodCatch = AutoPlaceObject(pSoldier, &(pObject->Obj), TRUE);

  // Report success....
  if (fGoodCatch) {
    pObject->fDropItem = FALSE;

    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_MERC_CAUGHT_ITEM],
              pSoldier->name, ShortItemNames[usItem]);
  }

  return (TRUE);
}

// #define TESTDUDEXPLOSIVES

void HandleArmedObjectImpact(REAL_OBJECT *pObject) {
  int16_t sZ;
  BOOLEAN fDoImpact = FALSE;
  BOOLEAN fCheckForDuds = FALSE;
  struct OBJECTTYPE *pObj;
  int32_t iTrapped = 0;
  uint16_t usFlags = 0;
  int8_t bLevel = 0;

  // Calculate pixel position of z
  sZ = (int16_t)CONVERT_HEIGHTUNITS_TO_PIXELS((int16_t)(pObject->Position.z)) -
       gpWorldLevelData[pObject->sGridNo].sHeight;

  // get struct OBJECTTYPE
  pObj = &(pObject->Obj);

  // ATE: Make sure number of objects is 1...
  pObj->ubNumberOfObjects = 1;

  if (Item[pObj->usItem].usItemClass & IC_GRENADE) {
    fCheckForDuds = TRUE;
  }

  if (pObj->usItem == MORTAR_SHELL) {
    fCheckForDuds = TRUE;
  }

  if (Item[pObj->usItem].usItemClass & IC_THROWN) {
    AddItemToPool(pObject->sGridNo, pObj, INVISIBLE, bLevel, usFlags, 0);
  }

  if (fCheckForDuds) {
    // If we landed on anything other than the floor, always! go off...
#ifdef TESTDUDEXPLOSIVES
    if (sZ != 0 || pObject->fInWater)
#else
    if (sZ != 0 || pObject->fInWater ||
        (pObj->bStatus[0] >= USABLE &&
         (PreRandom(100) < (uint32_t)pObj->bStatus[0] + PreRandom(50))))
#endif
    {
      fDoImpact = TRUE;
    } else  // didn't go off!
    {
#ifdef TESTDUDEXPLOSIVES
      if (1)
#else
      if (pObj->bStatus[0] >= USABLE && PreRandom(100) < (uint32_t)pObj->bStatus[0] + PreRandom(50))
#endif
      {
        iTrapped = PreRandom(4) + 2;
      }

      if (iTrapped) {
        // Start timed bomb...
        usFlags |= WORLD_ITEM_ARMED_BOMB;

        pObj->bDetonatorType = BOMB_TIMED;
        pObj->bDelay = (int8_t)(1 + PreRandom(2));
      }

      // ATE: If we have collided with roof last...
      if (pObject->iOldCollisionCode == COLLISION_ROOF) {
        bLevel = 1;
      }

      // Add item to pool....
      AddItemToPool(pObject->sGridNo, pObj, INVISIBLE, bLevel, usFlags, 0);

      // All teams lok for this...
      NotifySoldiersToLookforItems();

      if (pObject->ubOwner != NOBODY) {
        DoMercBattleSound(MercPtrs[pObject->ubOwner], (int8_t)(BATTLE_SOUND_CURSE1));
      }
    }
  } else {
    fDoImpact = TRUE;
  }

  if (fDoImpact) {
    if (pObject->Obj.usItem == BREAK_LIGHT) {
      // Add a light effect...
      NewLightEffect(pObject->sGridNo, LIGHT_FLARE_MARK_1);
    } else if (Item[pObject->Obj.usItem].usItemClass & IC_GRENADE) {
      /* ARM: Removed.  Rewards even missed throws, and pulling a pin doesn't really teach anything
         about explosives if ( MercPtrs[ pObject->ubOwner ]->bTeam == gbPlayerNum &&
         gTacticalStatus.uiFlags & INCOMBAT )
                              {
                                      // tossed grenade, not a dud, so grant xp
                                      // EXPLOSIVES GAIN (10):  Tossing grenade
              if ( pObject->ubOwner != NOBODY )
              {
                                        StatChange( MercPtrs[ pObject->ubOwner ], EXPLODEAMT, 10,
         FALSE );
              }
                              }
      */

      IgniteExplosion(
          pObject->ubOwner, (int16_t)pObject->Position.x, (int16_t)pObject->Position.y, sZ,
          pObject->sGridNo, pObject->Obj.usItem,
          GET_OBJECT_LEVEL(pObject->Position.z - CONVERT_PIXELS_TO_HEIGHTUNITS(
                                                     gpWorldLevelData[pObject->sGridNo].sHeight)));
    } else if (pObject->Obj.usItem == MORTAR_SHELL) {
      sZ = (int16_t)CONVERT_HEIGHTUNITS_TO_PIXELS((int16_t)pObject->Position.z);

      IgniteExplosion(
          pObject->ubOwner, (int16_t)pObject->Position.x, (int16_t)pObject->Position.y, sZ,
          pObject->sGridNo, pObject->Obj.usItem,
          GET_OBJECT_LEVEL(pObject->Position.z - CONVERT_PIXELS_TO_HEIGHTUNITS(
                                                     gpWorldLevelData[pObject->sGridNo].sHeight)));
    }
  }
}

BOOLEAN SavePhysicsTableToSaveGameFile(HWFILE hFile) {
  uint32_t uiNumBytesWritten = 0;
  uint16_t usCnt = 0;
  uint32_t usPhysicsCount = 0;

  for (usCnt = 0; usCnt < NUM_OBJECT_SLOTS; usCnt++) {
    // if the REAL_OBJECT is active, save it
    if (ObjectSlots[usCnt].fAllocated) {
      usPhysicsCount++;
    }
  }

  // Save the number of REAL_OBJECTs in the array
  FileMan_Write(hFile, &usPhysicsCount, sizeof(uint32_t), &uiNumBytesWritten);
  if (uiNumBytesWritten != sizeof(uint32_t)) {
    return (FALSE);
  }

  if (usPhysicsCount != 0) {
    for (usCnt = 0; usCnt < NUM_OBJECT_SLOTS; usCnt++) {
      // if the REAL_OBJECT is active, save it
      if (ObjectSlots[usCnt].fAllocated) {
        // Save the the REAL_OBJECT structure
        FileMan_Write(hFile, &ObjectSlots[usCnt], sizeof(REAL_OBJECT), &uiNumBytesWritten);
        if (uiNumBytesWritten != sizeof(REAL_OBJECT)) {
          return (FALSE);
        }
      }
    }
  }

  return (TRUE);
}

BOOLEAN LoadPhysicsTableFromSavedGameFile(HWFILE hFile) {
  uint32_t uiNumBytesRead = 0;
  uint16_t usCnt = 0;

  // make sure the objects are not allocated
  memset(ObjectSlots, 0, NUM_OBJECT_SLOTS * sizeof(REAL_OBJECT));

  // Load the number of REAL_OBJECTs in the array
  FileMan_Read(hFile, &guiNumObjectSlots, sizeof(uint32_t), &uiNumBytesRead);
  if (uiNumBytesRead != sizeof(uint32_t)) {
    return (FALSE);
  }

  // loop through and add the objects
  for (usCnt = 0; usCnt < guiNumObjectSlots; usCnt++) {
    // Load the the REAL_OBJECT structure
    FileMan_Read(hFile, &ObjectSlots[usCnt], sizeof(REAL_OBJECT), &uiNumBytesRead);
    if (uiNumBytesRead != sizeof(REAL_OBJECT)) {
      return (FALSE);
    }

    ObjectSlots[usCnt].pNode = NULL;
    ObjectSlots[usCnt].pShadow = NULL;
    ObjectSlots[usCnt].iID = usCnt;
  }

  return (TRUE);
}

uint16_t RandomGridFromRadius(int16_t sSweetGridNo, int8_t ubMinRadius, int8_t ubMaxRadius) {
  int16_t sX, sY;
  int16_t sGridNo;
  int32_t leftmost;
  BOOLEAN fFound = FALSE;
  uint32_t cnt = 0;

  if (ubMaxRadius == 0 || ubMinRadius == 0) {
    return (sSweetGridNo);
  }

  do {
    sX = (uint16_t)PreRandom(ubMaxRadius);
    sY = (uint16_t)PreRandom(ubMaxRadius);

    if ((sX < ubMinRadius || sY < ubMinRadius) && ubMaxRadius != ubMinRadius) {
      continue;
    }

    if (PreRandom(2) == 0) {
      sX = sX * -1;
    }

    if (PreRandom(2) == 0) {
      sY = sY * -1;
    }

    leftmost = ((sSweetGridNo + (WORLD_COLS * sY)) / WORLD_COLS) * WORLD_COLS;

    sGridNo = sSweetGridNo + (WORLD_COLS * sY) + sX;

    if (sGridNo == sSweetGridNo) {
      continue;
    }

    if (sGridNo >= 0 && sGridNo < WORLD_MAX && sGridNo >= leftmost &&
        sGridNo < (leftmost + WORLD_COLS)) {
      fFound = TRUE;
    }

    cnt++;

    if (cnt > 50) {
      return (NOWHERE);
    }

  } while (!fFound);

  return (sGridNo);
}
