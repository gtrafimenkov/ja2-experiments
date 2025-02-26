// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/Faces.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>

#include "GameSettings.h"
#include "JAScreens.h"
#include "SGP/Container.h"
#include "SGP/Line.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/Quests.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/DrugsAndAlcohol.h"
#include "Tactical/Gap.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/InterfaceUtils.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Squads.h"
#include "Tactical/TeamTurns.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "UI.h"
#include "Utils/FontControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Utilities.h"

// Defines
#define NUM_FACE_SLOTS 50

#define END_FACE_OVERLAY_DELAY 2000

// GLOBAL FOR FACES LISTING
FACETYPE gFacesData[NUM_FACE_SLOTS];
uint32_t guiNumFaces = 0;
uint32_t guiPORTRAITICONS;

// LOCAL FUNCTIONS
void NewEye(FACETYPE *pFace);
void NewMouth(FACETYPE *pFace);
int32_t GetFreeFace(void);
void RecountFaces(void);
static void HandleRenderFaceAdjustments(FACETYPE *pFace, BOOLEAN fDisplayBuffer,
                                        BOOLEAN fUseExternBuffer, struct JSurface *buffer,
                                        int16_t sFaceX, int16_t sFaceY, uint16_t usEyesX,
                                        uint16_t usEyesY);

extern BOOLEAN gfInItemPickupMenu;

typedef struct {
  int8_t bEyesX;
  int8_t bEyesY;
  int8_t bMouthX;
  int8_t bMouthY;

} RPC_SMALL_FACE_VALUES;

RPC_SMALL_FACE_VALUES gRPCSmallFaceValues[] = {
    {9, 8, 8, 24},    // MIGUEL		( 57 )
    {8, 8, 7, 24},    // CARLOS		( 58 )
    {10, 8, 8, 26},   // IRA			( 59 )
    {7, 8, 7, 26},    // DIMITRI	( 60 )
    {6, 7, 7, 23},    // DEVIN		( 61 )
    {0, 0, 0, 0},     // THE RAT	( 62 )
    {8, 7, 8, 23},    //					( 63 )
    {8, 8, 8, 22},    // SLAY			( 64 )
    {0, 0, 0, 0},     //					( 65 )
    {9, 4, 7, 22},    // DYNAMO		( 66 )
    {8, 8, 8, 25},    // SHANK		( 67 )
    {4, 6, 5, 22},    // IGGY			( 68 )
    {8, 9, 7, 25},    // VINCE		( 69 )
    {4, 7, 5, 25},    // CONRAD		( 70 )
    {9, 7, 8, 22},    // CARL			( 71 )
    {9, 7, 9, 25},    // MADDOG		( 72 )
    {0, 0, 0, 0},     //					( 73 )
    {0, 0, 0, 0},     //					( 74 )
    {9, 3, 8, 23},    // MARIA		( 88 )
    {9, 3, 8, 25},    // JOEY			( 90 )
    {11, 7, 9, 24},   // SKYRIDER	( 97 )
    {9, 5, 7, 23},    // Miner	  ( 106 )
    {6, 4, 6, 24},    // JOHN			( 118 )
    {12, 4, 10, 24},  //					( 119 )
    {8, 6, 8, 23},    // Miner	( 148 )
    {6, 5, 6, 23},    // Miner	( 156 )
    {13, 7, 11, 24},  // Miner	( 157 )
    {9, 7, 8, 22},    // Miner	( 158 )

};

uint8_t gubRPCSmallFaceProfileNum[] = {
    57,  // entry 0
    58,  59,  60,  61,  62, 63, 64, 65,
    66,  // entry 9
    67,  68,  69,  70,  71, 72, 73, 74, 88,
    90,  // entry 19
    97,  106, 118, 119,
    148,  // entry 24
    156, 157, 158,

};

uint8_t ubRPCNumSmallFaceValues = 28;

extern BOOLEAN gfSMDisableForItems;
extern int16_t gsCurInterfacePanel;
extern uint16_t gusSMCurrentMerc;
extern BOOLEAN gfRerenderInterfaceFromHelpText;
extern BOOLEAN gfInItemPickupMenu;

BOOLEAN FaceRestoreSavedBackgroundRect(int32_t iFaceIndex, int16_t sDestLeft, int16_t sDestTop,
                                       int16_t sSrcLeft, int16_t sSrcTop, int16_t sWidth,
                                       int16_t sHeight);
void SetupFinalTalkingDelay(FACETYPE *pFace);

int32_t GetFreeFace(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < guiNumFaces; uiCount++) {
    if ((gFacesData[uiCount].fAllocated == FALSE)) return ((int32_t)uiCount);
  }

  if (guiNumFaces < NUM_FACE_SLOTS) return ((int32_t)guiNumFaces++);

  return (-1);
}

void RecountFaces(void) {
  int32_t uiCount;

  for (uiCount = guiNumFaces - 1; (uiCount >= 0); uiCount--) {
    if ((gFacesData[uiCount].fAllocated)) {
      guiNumFaces = (uint32_t)(uiCount + 1);
      break;
    }
  }
}

int32_t InitSoldierFace(struct SOLDIERTYPE *pSoldier) {
  int32_t iFaceIndex;

  // Check if we have a face init already
  iFaceIndex = pSoldier->iFaceIndex;

  if (iFaceIndex != -1) {
    return (iFaceIndex);
  }

  return (InitFace(GetSolProfile(pSoldier), GetSolID(pSoldier), 0));
}

int32_t InitFace(uint8_t usMercProfileID, uint8_t ubSoldierID, uint32_t uiInitFlags) {
  uint32_t uiBlinkFrequency;
  uint32_t uiExpressionFrequency;

  if (usMercProfileID == NO_PROFILE) {
    return (-1);
  }

  uiBlinkFrequency = gMercProfiles[usMercProfileID].uiBlinkFrequency;
  uiExpressionFrequency = gMercProfiles[usMercProfileID].uiExpressionFrequency;

  if (Random(2)) {
    uiBlinkFrequency += Random(2000);
  } else {
    uiBlinkFrequency -= Random(2000);
  }

  return (InternalInitFace(usMercProfileID, ubSoldierID, uiInitFlags,
                           gMercProfiles[usMercProfileID].ubFaceIndex, uiBlinkFrequency,
                           uiExpressionFrequency));
}

int32_t InternalInitFace(uint8_t usMercProfileID, uint8_t ubSoldierID, uint32_t uiInitFlags,
                         int32_t iFaceFileID, uint32_t uiBlinkFrequency,
                         uint32_t uiExpressionFrequency) {
  FACETYPE *pFace;
  uint32_t uiVideoObject;
  int32_t iFaceIndex;
  ETRLEObject ETRLEObject;
  struct VObject *hVObject;
  uint32_t uiCount;
  struct JPaletteEntry Pal[256];

  if ((iFaceIndex = GetFreeFace()) == (-1)) return (-1);

  // Load face file
  // ATE: If we are merc profile ID #151-154, all use 151's protrait....
  if (usMercProfileID >= 151 && usMercProfileID <= 154) {
    iFaceFileID = 151;
  }

  SGPFILENAME ImageFile;
  // Check if we are a big-face....
  if (uiInitFlags & FACE_BIGFACE) {
    // The filename is the profile ID!
    if (iFaceFileID < 100) {
      sprintf(ImageFile, "FACES\\b%02d.sti", iFaceFileID);
    } else {
      sprintf(ImageFile, "FACES\\b%03d.sti", iFaceFileID);
    }

    // ATE: Check for profile - if elliot , use special face :)
    if (usMercProfileID == ELLIOT) {
      if (gMercProfiles[ELLIOT].bNPCData > 3 && gMercProfiles[ELLIOT].bNPCData < 7) {
        sprintf(ImageFile, "FACES\\b%02da.sti", iFaceFileID);
      } else if (gMercProfiles[ELLIOT].bNPCData > 6 && gMercProfiles[ELLIOT].bNPCData < 10) {
        sprintf(ImageFile, "FACES\\b%02db.sti", iFaceFileID);
      } else if (gMercProfiles[ELLIOT].bNPCData > 9 && gMercProfiles[ELLIOT].bNPCData < 13) {
        sprintf(ImageFile, "FACES\\b%02dc.sti", iFaceFileID);
      } else if (gMercProfiles[ELLIOT].bNPCData > 12 && gMercProfiles[ELLIOT].bNPCData < 16) {
        sprintf(ImageFile, "FACES\\b%02dd.sti", iFaceFileID);
      } else if (gMercProfiles[ELLIOT].bNPCData == 17) {
        sprintf(ImageFile, "FACES\\b%02de.sti", iFaceFileID);
      }
    }
  } else {
    if (iFaceFileID < 100) {
      // The filename is the profile ID!
      sprintf(ImageFile, "FACES\\%02d.sti", iFaceFileID);
    } else {
      sprintf(ImageFile, "FACES\\%03d.sti", iFaceFileID);
    }
  }

  // Load
  if (AddVObject(CreateVObjectFromFile(ImageFile), &uiVideoObject) == FALSE) {
    // If we are a big face, use placeholder...
    if (uiInitFlags & FACE_BIGFACE) {
      sprintf(ImageFile, "FACES\\placeholder.sti");

      if (AddVObject(CreateVObjectFromFile(ImageFile), &uiVideoObject) == FALSE) {
        return (-1);
      }
    } else {
      return (-1);
    }
  }

  memset(&gFacesData[iFaceIndex], 0, sizeof(FACETYPE));

  pFace = &gFacesData[iFaceIndex];

  // Get profile data and set into face data
  pFace->ubSoldierID = ubSoldierID;

  pFace->iID = iFaceIndex;
  pFace->fAllocated = TRUE;

  // Default to off!
  pFace->fDisabled = TRUE;
  pFace->iVideoOverlay = -1;
  // pFace->uiEyeDelay			=	gMercProfiles[ usMercProfileID ].uiEyeDelay;
  // pFace->uiMouthDelay		= gMercProfiles[ usMercProfileID ].uiMouthDelay;
  pFace->uiEyeDelay = 50 + Random(30);
  pFace->uiMouthDelay = 120;
  pFace->ubCharacterNum = usMercProfileID;

  pFace->uiBlinkFrequency = uiBlinkFrequency;
  pFace->uiExpressionFrequency = uiExpressionFrequency;

  pFace->sEyeFrame = 0;
  pFace->sMouthFrame = 0;
  pFace->uiFlags = uiInitFlags;

  // Set palette
  if (GetVideoObject(&hVObject, uiVideoObject)) {
    // Build a grayscale palette! ( for testing different looks )
    for (uiCount = 0; uiCount < 256; uiCount++) {
      Pal[uiCount].red = 255;
      Pal[uiCount].green = 255;
      Pal[uiCount].blue = 255;
    }

    hVObject->pShades[FLASH_PORTRAIT_NOSHADE] =
        Create16BPPPaletteShaded(hVObject->pPaletteEntry, 255, 255, 255, FALSE);
    hVObject->pShades[FLASH_PORTRAIT_STARTSHADE] =
        Create16BPPPaletteShaded(Pal, 255, 255, 255, FALSE);
    hVObject->pShades[FLASH_PORTRAIT_ENDSHADE] =
        Create16BPPPaletteShaded(hVObject->pPaletteEntry, 250, 25, 25, TRUE);
    hVObject->pShades[FLASH_PORTRAIT_DARKSHADE] =
        Create16BPPPaletteShaded(hVObject->pPaletteEntry, 100, 100, 100, TRUE);
    hVObject->pShades[FLASH_PORTRAIT_LITESHADE] =
        Create16BPPPaletteShaded(hVObject->pPaletteEntry, 100, 100, 100, FALSE);

    for (uiCount = 0; uiCount < 256; uiCount++) {
      Pal[uiCount].red = (uint8_t)(uiCount % 128) + 128;
      Pal[uiCount].green = (uint8_t)(uiCount % 128) + 128;
      Pal[uiCount].blue = (uint8_t)(uiCount % 128) + 128;
    }
    hVObject->pShades[FLASH_PORTRAIT_GRAYSHADE] =
        Create16BPPPaletteShaded(Pal, 255, 255, 255, FALSE);
  }

  // Get FACE height, width
  if (GetVideoObjectETRLEPropertiesFromIndex(uiVideoObject, &ETRLEObject, 0) == FALSE) {
    return (-1);
  }
  pFace->usFaceWidth = ETRLEObject.usWidth;
  pFace->usFaceHeight = ETRLEObject.usHeight;

  // OK, check # of items
  if (hVObject->usNumberOfObjects == 8) {
    pFace->fInvalidAnim = FALSE;

    // Get EYE height, width
    if (GetVideoObjectETRLEPropertiesFromIndex(uiVideoObject, &ETRLEObject, 1) == FALSE) {
      return (-1);
    }
    pFace->usEyesWidth = ETRLEObject.usWidth;
    pFace->usEyesHeight = ETRLEObject.usHeight;

    // Get Mouth height, width
    if (GetVideoObjectETRLEPropertiesFromIndex(uiVideoObject, &ETRLEObject, 5) == FALSE) {
      return (-1);
    }
    pFace->usMouthWidth = ETRLEObject.usWidth;
    pFace->usMouthHeight = ETRLEObject.usHeight;
  } else {
    pFace->fInvalidAnim = TRUE;
  }

  // Set id
  pFace->uiVideoObject = uiVideoObject;

  return (iFaceIndex);
}

void DeleteSoldierFace(struct SOLDIERTYPE *pSoldier) {
  DeleteFace(pSoldier->iFaceIndex);

  pSoldier->iFaceIndex = -1;
}

void DeleteFace(int32_t iFaceIndex) {
  FACETYPE *pFace;

  // Check face index
  CHECKV(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  // Check for a valid slot!
  CHECKV(pFace->fAllocated != FALSE);

  pFace->fCanHandleInactiveNow = TRUE;

  if (!pFace->fDisabled) {
    SetAutoFaceInActive(iFaceIndex);
  }

  // If we are still talking, stop!
  if (pFace->fTalking) {
    // Call dialogue handler function
    pFace->fTalking = FALSE;

    HandleDialogueEnd(pFace);
  }

  // Delete vo
  DeleteVideoObjectFromIndex(pFace->uiVideoObject);

  // Set uncallocated
  pFace->fAllocated = FALSE;

  RecountFaces();
}

void SetAutoFaceActiveFromSoldier(struct JSurface *buffer, struct JSurface *restoreBuffer,
                                  uint8_t ubSoldierID, uint16_t usFaceX, uint16_t usFaceY) {
  if (ubSoldierID == NOBODY) {
    return;
  }

  SetAutoFaceActive(buffer, restoreBuffer, MercPtrs[ubSoldierID]->iFaceIndex, usFaceX, usFaceY);
}

void GetFaceRelativeCoordinates(FACETYPE *pFace, uint16_t *pusEyesX, uint16_t *pusEyesY,
                                uint16_t *pusMouthX, uint16_t *pusMouthY) {
  uint16_t usMercProfileID;
  uint16_t usEyesX;
  uint16_t usEyesY;
  uint16_t usMouthX;
  uint16_t usMouthY;
  int32_t cnt;

  usMercProfileID = pFace->ubCharacterNum;

  // Take eyes x,y from profile unless we are an RPC and we are small faced.....
  usEyesX = gMercProfiles[usMercProfileID].usEyesX;
  usEyesY = gMercProfiles[usMercProfileID].usEyesY;
  usMouthY = gMercProfiles[usMercProfileID].usMouthY;
  usMouthX = gMercProfiles[usMercProfileID].usMouthX;

  // Use some other values for x,y, base on if we are a RPC!
  if (!(pFace->uiFlags & FACE_BIGFACE) || (pFace->uiFlags & FACE_FORCE_SMALL)) {
    // Are we a recruited merc? .. or small?
    if ((gMercProfiles[usMercProfileID].ubMiscFlags &
         (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE)) ||
        (pFace->uiFlags & FACE_FORCE_SMALL)) {
      // Loop through all values of availible merc IDs to find ours!
      for (cnt = 0; cnt < ubRPCNumSmallFaceValues; cnt++) {
        // We've found one!
        if (gubRPCSmallFaceProfileNum[cnt] == usMercProfileID) {
          usEyesX = gRPCSmallFaceValues[cnt].bEyesX;
          usEyesY = gRPCSmallFaceValues[cnt].bEyesY;
          usMouthY = gRPCSmallFaceValues[cnt].bMouthY;
          usMouthX = gRPCSmallFaceValues[cnt].bMouthX;
        }
      }
    }
  }

  (*pusEyesX) = usEyesX;
  (*pusEyesY) = usEyesY;
  (*pusMouthX) = usMouthX;
  (*pusMouthY) = usMouthY;
}

// Same as above, yet used mostly internally. Is compatible with the fact that a soldier profile ID
// is not required...
static void InternalSetAutoFaceActive(struct JSurface *displayBuffer,
                                      struct JSurface *restoreBuffer, int32_t iFaceIndex,
                                      uint16_t usFaceX, uint16_t usFaceY, uint16_t usEyesX,
                                      uint16_t usEyesY, uint16_t usMouthX, uint16_t usMouthY);

void SetAutoFaceActive(struct JSurface *displayBuffer, struct JSurface *restoreBuffer,
                       int32_t iFaceIndex, uint16_t usFaceX, uint16_t usFaceY) {
  uint16_t usEyesX;
  uint16_t usEyesY;
  uint16_t usMouthX;
  uint16_t usMouthY;
  FACETYPE *pFace;

  // Check face index
  CHECKV(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  GetFaceRelativeCoordinates(pFace, &usEyesX, &usEyesY, &usMouthX, &usMouthY);

  InternalSetAutoFaceActive(displayBuffer, restoreBuffer, iFaceIndex, usFaceX, usFaceY, usEyesX,
                            usEyesY, usMouthX, usMouthY);
}

static void InternalSetAutoFaceActive(struct JSurface *displayBuffer,
                                      struct JSurface *restoreBuffer, int32_t iFaceIndex,
                                      uint16_t usFaceX, uint16_t usFaceY, uint16_t usEyesX,
                                      uint16_t usEyesY, uint16_t usMouthX, uint16_t usMouthY) {
  FACETYPE *pFace;

  // Check face index
  CHECKV(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  // IF we are already being contained elsewhere, return without doing anything!

  // ATE: Don't allow another activity from setting active....
  if (pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) {
    return;
  }

  // Check if we are active already, remove if so!
  if (pFace->fDisabled) {
    SetAutoFaceInActive(iFaceIndex);
  }

  if (restoreBuffer == NULL /*FACE_AUTO_RESTORE_BUFFER*/) {
    pFace->fAutoRestoreBuffer = TRUE;
    pFace->autoRestoreBuffer = JSurface_Create16bpp(pFace->usFaceWidth, pFace->usFaceHeight);
    if (pFace->autoRestoreBuffer == NULL) {
      return;
    }
    JSurface_SetColorKey(pFace->autoRestoreBuffer, FROMRGB(0, 0, 0));
  } else {
    pFace->fAutoRestoreBuffer = FALSE;
    pFace->autoRestoreBuffer = restoreBuffer;
  }

  if (displayBuffer == NULL) {
    pFace->fAutoDisplayBuffer = TRUE;
    pFace->autoDisplayBuffer = JSurface_Create16bpp(pFace->usFaceWidth, pFace->usFaceHeight);
    if (pFace->autoDisplayBuffer == NULL) {
      return;
    }
  } else {
    pFace->fAutoDisplayBuffer = FALSE;
    pFace->autoDisplayBuffer = displayBuffer;
  }

  pFace->usFaceX = usFaceX;
  pFace->usFaceY = usFaceY;
  pFace->fCanHandleInactiveNow = FALSE;

  // Take eyes x,y from profile unless we are an RPC and we are small faced.....
  pFace->usEyesX = usEyesX + usFaceX;
  pFace->usEyesY = usEyesY + usFaceY;
  pFace->usMouthY = usMouthY + usFaceY;
  pFace->usMouthX = usMouthX + usFaceX;

  // Save offset values
  pFace->usEyesOffsetX = usEyesX;
  pFace->usEyesOffsetY = usEyesY;
  pFace->usMouthOffsetY = usMouthY;
  pFace->usMouthOffsetX = usMouthX;

  if (pFace->usEyesY == usFaceY || pFace->usMouthY == usFaceY) {
    pFace->fInvalidAnim = TRUE;
  }

  pFace->fDisabled = FALSE;
  pFace->uiLastBlink = GetJA2Clock();
  pFace->uiLastExpression = GetJA2Clock();
  pFace->uiEyelast = GetJA2Clock();
  pFace->fStartFrame = TRUE;

  // Are we a soldier?
  if (pFace->ubSoldierID != NOBODY) {
    pFace->bOldSoldierLife = MercPtrs[pFace->ubSoldierID]->bLife;
  }
}

void SetAutoFaceInActiveFromSoldier(uint8_t ubSoldierID) {
  // Check for valid soldier
  CHECKV(ubSoldierID != NOBODY);

  SetAutoFaceInActive(MercPtrs[ubSoldierID]->iFaceIndex);
}

void SetAutoFaceInActive(int32_t iFaceIndex) {
  FACETYPE *pFace;
  struct SOLDIERTYPE *pSoldier;

  // Check face index
  CHECKV(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  // Check for a valid slot!
  CHECKV(pFace->fAllocated != FALSE);

  // Turn off some flags
  if (pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) {
    if (!pFace->fCanHandleInactiveNow) {
      return;
    }
  }

  if (pFace->uiFlags & FACE_MAKEACTIVE_ONCE_DONE) {
    //
    if (pFace->ubSoldierID != NOBODY) {
      pSoldier = MercPtrs[pFace->ubSoldierID];

      // IF we are in tactical
      if (GetSolAssignment(pSoldier) == iCurrentTacticalSquad && IsTacticalMode()) {
        // Make the interfac panel dirty..
        // This will dirty the panel next frame...
        gfRerenderInterfaceFromHelpText = TRUE;
      }
    }
  }

  if (pFace->fAutoRestoreBuffer) {
    JSurface_Free(pFace->autoRestoreBuffer);
    pFace->autoRestoreBuffer = NULL;
  }

  if (pFace->fAutoDisplayBuffer) {
    JSurface_Free(pFace->autoDisplayBuffer);
    pFace->autoDisplayBuffer = NULL;
  }

  if (pFace->iVideoOverlay != -1) {
    RemoveVideoOverlay(pFace->iVideoOverlay);
    pFace->iVideoOverlay = -1;
  }

  // Turn off some flags
  pFace->uiFlags &= (~FACE_INACTIVE_HANDLED_ELSEWHERE);

  // Disable!
  pFace->fDisabled = TRUE;
}

void SetAllAutoFacesInactive() {
  uint32_t uiCount;
  for (uiCount = 0; uiCount < guiNumFaces; uiCount++) {
    if (gFacesData[uiCount].fAllocated) {
      SetAutoFaceInActive(uiCount);
    }
  }
}

void BlinkAutoFace(int32_t iFaceIndex) {
  FACETYPE *pFace;
  int16_t sFrame;
  BOOLEAN fDoBlink = FALSE;

  if (gFacesData[iFaceIndex].fAllocated && !gFacesData[iFaceIndex].fDisabled &&
      !gFacesData[iFaceIndex].fInvalidAnim) {
    pFace = &gFacesData[iFaceIndex];

    // CHECK IF BUDDY IS DEAD, UNCONSCIOUS, ASLEEP, OR POW!
    if (pFace->ubSoldierID != NOBODY) {
      if ((MercPtrs[pFace->ubSoldierID]->bLife < OKLIFE) ||
          (MercPtrs[pFace->ubSoldierID]->fMercAsleep == TRUE) ||
          (MercPtrs[pFace->ubSoldierID]->bAssignment == ASSIGNMENT_POW)) {
        return;
      }
    }

    if (pFace->ubExpression == NO_EXPRESSION) {
      // Get Delay time, if the first frame, use a different delay
      if ((GetJA2Clock() - pFace->uiLastBlink) > pFace->uiBlinkFrequency) {
        pFace->uiLastBlink = GetJA2Clock();
        pFace->ubExpression = BLINKING;
        pFace->uiEyelast = GetJA2Clock();
      }

      if (pFace->fAnimatingTalking) {
        if ((GetJA2Clock() - pFace->uiLastExpression) > pFace->uiExpressionFrequency) {
          pFace->uiLastExpression = GetJA2Clock();

          if (Random(2) == 0) {
            pFace->ubExpression = ANGRY;
          } else {
            pFace->ubExpression = SURPRISED;
          }
        }
      }
    }

    if (pFace->ubExpression != NO_EXPRESSION) {
      if (pFace->fStartFrame) {
        if ((GetJA2Clock() - pFace->uiEyelast) > pFace->uiEyeDelay)  //> Random( 10000 ) )
        {
          fDoBlink = TRUE;
          pFace->fStartFrame = FALSE;
        }
      } else {
        if ((GetJA2Clock() - pFace->uiEyelast) > pFace->uiEyeDelay) {
          fDoBlink = TRUE;
        }
      }

      // Are we going to blink?
      if (fDoBlink) {
        pFace->uiEyelast = GetJA2Clock();

        // Adjust
        NewEye(pFace);

        sFrame = pFace->sEyeFrame;

        if (sFrame >= 5) {
          sFrame = 4;
        }

        if (sFrame > 0) {
          // Blit Accordingly!
          BltVObjectFromIndex(pFace->autoDisplayBuffer, pFace->uiVideoObject, (int16_t)(sFrame),
                              pFace->usEyesX, pFace->usEyesY);

          if (pFace->autoDisplayBuffer == vsFB) {
            InvalidateRegion(pFace->usEyesX, pFace->usEyesY, pFace->usEyesX + pFace->usEyesWidth,
                             pFace->usEyesY + pFace->usEyesHeight);
          }
        } else {
          // RenderFace( uiDestBuffer , uiCount );
          pFace->ubExpression = NO_EXPRESSION;
          // Update rects just for eyes

          if (pFace->autoRestoreBuffer == vsSaveBuffer) {
            FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usEyesX, pFace->usEyesY,
                                           pFace->usEyesX, pFace->usEyesY, pFace->usEyesWidth,
                                           pFace->usEyesHeight);
          } else {
            FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usEyesX, pFace->usEyesY,
                                           pFace->usEyesOffsetX, pFace->usEyesOffsetY,
                                           pFace->usEyesWidth, pFace->usEyesHeight);
          }
        }

        HandleRenderFaceAdjustments(pFace, TRUE, FALSE, NULL, pFace->usFaceX, pFace->usFaceY,
                                    pFace->usEyesX, pFace->usEyesY);
      }
    }
  }
}

static void HandleFaceHilights(FACETYPE *pFace, struct JSurface *buffer, int16_t sFaceX,
                               int16_t sFaceY) {
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;
  int32_t iFaceIndex;

  iFaceIndex = pFace->iID;

  if (!gFacesData[iFaceIndex].fDisabled) {
    if (pFace->autoDisplayBuffer == vsFB && IsTacticalMode()) {
      // If we are highlighted, do this now!
      if ((pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT)) {
        // Lock buffer
        pDestBuf = LockVSurface(buffer, &uiDestPitchBYTES);
        SetClippingRegionAndImageWidth(uiDestPitchBYTES, sFaceX - 2, sFaceY - 1,
                                       sFaceX + pFace->usFaceWidth + 4,
                                       sFaceY + pFace->usFaceHeight + 4);

        usLineColor = rgb32_to_rgb16(FROMRGB(255, 255, 255));
        RectangleDraw(TRUE, (sFaceX - 2), (sFaceY - 1), sFaceX + pFace->usFaceWidth + 1,
                      sFaceY + pFace->usFaceHeight, usLineColor, pDestBuf);

        SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

        JSurface_Unlock(buffer);
      } else if ((pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT)) {
        if (pFace->ubSoldierID != NOBODY) {
          if (MercPtrs[pFace->ubSoldierID]->bLife >= OKLIFE) {
            // Lock buffer
            pDestBuf = LockVSurface(buffer, &uiDestPitchBYTES);
            SetClippingRegionAndImageWidth(uiDestPitchBYTES, sFaceX - 2, sFaceY - 1,
                                           sFaceX + pFace->usFaceWidth + 4,
                                           sFaceY + pFace->usFaceHeight + 4);

            if (MercPtrs[pFace->ubSoldierID]->bStealthMode) {
              usLineColor = rgb32_to_rgb16(FROMRGB(158, 158, 12));
            } else {
              usLineColor = rgb32_to_rgb16(FROMRGB(8, 12, 118));
            }
            RectangleDraw(TRUE, (sFaceX - 2), (sFaceY - 1), sFaceX + pFace->usFaceWidth + 1,
                          sFaceY + pFace->usFaceHeight, usLineColor, pDestBuf);

            SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

            JSurface_Unlock(buffer);
          }
        }
      } else {
        // ATE: Zero out any highlight boxzes....
        // Lock buffer
        pDestBuf = LockVSurface(pFace->autoDisplayBuffer, &uiDestPitchBYTES);
        SetClippingRegionAndImageWidth(uiDestPitchBYTES, pFace->usFaceX - 2, pFace->usFaceY - 1,
                                       pFace->usFaceX + pFace->usFaceWidth + 4,
                                       pFace->usFaceY + pFace->usFaceHeight + 4);

        usLineColor = rgb32_to_rgb16(FROMRGB(0, 0, 0));
        RectangleDraw(TRUE, (pFace->usFaceX - 2), (pFace->usFaceY - 1),
                      pFace->usFaceX + pFace->usFaceWidth + 1, pFace->usFaceY + pFace->usFaceHeight,
                      usLineColor, pDestBuf);

        SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

        JSurface_Unlock(pFace->autoDisplayBuffer);
      }
    }
  }

  if ((pFace->fCompatibleItems && !gFacesData[iFaceIndex].fDisabled)) {
    // Lock buffer
    pDestBuf = LockVSurface(buffer, &uiDestPitchBYTES);
    SetClippingRegionAndImageWidth(uiDestPitchBYTES, sFaceX - 2, sFaceY - 1,
                                   sFaceX + pFace->usFaceWidth + 4,
                                   sFaceY + pFace->usFaceHeight + 4);

    usLineColor = rgb32_to_rgb16(FROMRGB(255, 0, 0));
    RectangleDraw(TRUE, (sFaceX - 2), (sFaceY - 1), sFaceX + pFace->usFaceWidth + 1,
                  sFaceY + pFace->usFaceHeight, usLineColor, pDestBuf);

    SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

    JSurface_Unlock(buffer);
  }
}

void MouthAutoFace(int32_t iFaceIndex) {
  FACETYPE *pFace;
  int16_t sFrame;

  if (gFacesData[iFaceIndex].fAllocated) {
    pFace = &gFacesData[iFaceIndex];

    // Remove video overlay is present....
    if (pFace->uiFlags & FACE_DESTROY_OVERLAY) {
      // if ( pFace->iVideoOverlay != -1 )
      //{
      //	if ( pFace->uiStopOverlayTimer != 0 )
      //	{
      //		if ( ( GetJA2Clock( ) - pFace->uiStopOverlayTimer ) > END_FACE_OVERLAY_DELAY
      //)
      //		{
      //	RemoveVideoOverlay( pFace->iVideoOverlay );
      //			pFace->iVideoOverlay = -1;
      //		}
      //	}
      //}
    }

    if (pFace->fTalking) {
      if (!gFacesData[iFaceIndex].fDisabled && !gFacesData[iFaceIndex].fInvalidAnim) {
        if (pFace->fAnimatingTalking) {
          PollAudioGap(pFace->uiSoundID, &(pFace->GapList));

          // Check if we have an audio gap
          if (pFace->GapList.audio_gap_active) {
            pFace->sMouthFrame = 0;

            if (pFace->autoRestoreBuffer == vsSaveBuffer) {
              FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usMouthX, pFace->usMouthY,
                                             pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth,
                                             pFace->usMouthHeight);
            } else {
              FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usMouthX, pFace->usMouthY,
                                             pFace->usMouthOffsetX, pFace->usMouthOffsetY,
                                             pFace->usMouthWidth, pFace->usMouthHeight);
            }

          } else {
            // Get Delay time
            if ((GetJA2Clock() - pFace->uiMouthlast) > pFace->uiMouthDelay) {
              pFace->uiMouthlast = GetJA2Clock();

              // Adjust
              NewMouth(pFace);

              sFrame = pFace->sMouthFrame;

              if (sFrame > 0) {
                // Blit Accordingly!
                BltVObjectFromIndex(pFace->autoDisplayBuffer, pFace->uiVideoObject,
                                    (int16_t)(sFrame + 4), pFace->usMouthX, pFace->usMouthY);

                // Update rects
                if (pFace->autoDisplayBuffer == vsFB) {
                  InvalidateRegion(pFace->usMouthX, pFace->usMouthY,
                                   pFace->usMouthX + pFace->usMouthWidth,
                                   pFace->usMouthY + pFace->usMouthHeight);
                }
              } else {
                // RenderFace( uiDestBuffer , uiCount );
                // pFace->fTaking = FALSE;
                // Update rects just for Mouth
                if (pFace->autoRestoreBuffer == vsSaveBuffer) {
                  FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usMouthX, pFace->usMouthY,
                                                 pFace->usMouthX, pFace->usMouthY,
                                                 pFace->usMouthWidth, pFace->usMouthHeight);
                } else {
                  FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usMouthX, pFace->usMouthY,
                                                 pFace->usMouthOffsetX, pFace->usMouthOffsetY,
                                                 pFace->usMouthWidth, pFace->usMouthHeight);
                }
              }

              HandleRenderFaceAdjustments(pFace, TRUE, FALSE, NULL, pFace->usFaceX, pFace->usFaceY,
                                          pFace->usEyesX, pFace->usEyesY);
            }
          }
        }
      }
    }

    if (!(pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE)) {
      HandleFaceHilights(pFace, pFace->autoDisplayBuffer, pFace->usFaceX, pFace->usFaceY);
    }
  }
}

void HandleTalkingAutoFace(int32_t iFaceIndex) {
  FACETYPE *pFace;

  if (gFacesData[iFaceIndex].fAllocated) {
    pFace = &gFacesData[iFaceIndex];

    if (pFace->fTalking) {
      // Check if we are done!	( Check this first! )
      if (pFace->fValidSpeech) {
        // Check if we have finished, set some flags for the final delay down if so!
        if (!SoundIsPlaying(pFace->uiSoundID) && !pFace->fFinishTalking) {
          SetupFinalTalkingDelay(pFace);
        }
      } else {
        // Check if our delay is over
        if (!pFace->fFinishTalking) {
          if ((GetJA2Clock() - pFace->uiTalkingTimer) > pFace->uiTalkingDuration) {
            // If here, setup for last delay!
            SetupFinalTalkingDelay(pFace);
          }
        }
      }

      // Now check for end of talking
      if (pFace->fFinishTalking) {
        if ((GetJA2Clock() - pFace->uiTalkingTimer) > pFace->uiTalkingDuration) {
          pFace->fTalking = FALSE;
          pFace->fAnimatingTalking = FALSE;

          // Remove gap info
          AudioGapListDone(&(pFace->GapList));

          // Remove video overlay is present....
          if (pFace->iVideoOverlay != -1) {
            // if ( pFace->uiStopOverlayTimer == 0 )
            //{
            //	pFace->uiStopOverlayTimer = GetJA2Clock();
            //}
          }

          // Call dialogue handler function
          HandleDialogueEnd(pFace);
        }
      }
    }
  }
}

// Local function - uses these variables because they have already been validated
void SetFaceShade(struct SOLDIERTYPE *pSoldier, FACETYPE *pFace, BOOLEAN fExternBlit) {
  // Set to default
  SetObjectHandleShade(pFace->uiVideoObject, FLASH_PORTRAIT_NOSHADE);

  if (pFace->iVideoOverlay == -1 && !fExternBlit) {
    if ((pSoldier->bActionPoints == 0) && !(gTacticalStatus.uiFlags & REALTIME) &&
        (gTacticalStatus.uiFlags & INCOMBAT)) {
      SetObjectHandleShade(pFace->uiVideoObject, FLASH_PORTRAIT_LITESHADE);
    }
  }

  if (pSoldier->bLife < OKLIFE) {
    SetObjectHandleShade(pFace->uiVideoObject, FLASH_PORTRAIT_DARKSHADE);
  }

  // ATE: Don't shade for damage if blitting extern face...
  if (!fExternBlit) {
    if (pSoldier->fFlashPortrait == FLASH_PORTRAIT_START) {
      SetObjectHandleShade(pFace->uiVideoObject, pSoldier->bFlashPortraitFrame);
    }
  }
}

BOOLEAN RenderAutoFaceFromSoldier(uint8_t ubSoldierID) {
  // Check for valid soldier
  CHECKF(ubSoldierID != NOBODY);

  return (RenderAutoFace(MercPtrs[ubSoldierID]->iFaceIndex));
}

void GetXYForIconPlacement(FACETYPE *pFace, uint16_t ubIndex, int16_t sFaceX, int16_t sFaceY,
                           int16_t *psX, int16_t *psY) {
  int16_t sX, sY;
  uint16_t usWidth, usHeight;
  ETRLEObject *pTrav;
  struct VObject *hVObject;

  // Get height, width of icon...
  GetVideoObject(&hVObject, guiPORTRAITICONS);
  pTrav = &(hVObject->pETRLEObject[ubIndex]);
  usHeight = pTrav->usHeight;
  usWidth = pTrav->usWidth;

  sX = sFaceX + pFace->usFaceWidth - usWidth - 1;
  sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

  *psX = sX;
  *psY = sY;
}

void GetXYForRightIconPlacement(FACETYPE *pFace, uint16_t ubIndex, int16_t sFaceX, int16_t sFaceY,
                                int16_t *psX, int16_t *psY, int8_t bNumIcons) {
  int16_t sX, sY;
  uint16_t usWidth, usHeight;
  ETRLEObject *pTrav;
  struct VObject *hVObject;

  // Get height, width of icon...
  GetVideoObject(&hVObject, guiPORTRAITICONS);
  pTrav = &(hVObject->pETRLEObject[ubIndex]);
  usHeight = pTrav->usHeight;
  usWidth = pTrav->usWidth;

  sX = sFaceX + (usWidth * bNumIcons) + 1;
  sY = sFaceY + pFace->usFaceHeight - usHeight - 1;

  *psX = sX;
  *psY = sY;
}

static void DoRightIcon(struct JSurface *renderBuffer, FACETYPE *pFace, int16_t sFaceX,
                        int16_t sFaceY, int8_t bNumIcons, int8_t sIconIndex) {
  int16_t sIconX, sIconY;

  // Find X, y for placement
  GetXYForRightIconPlacement(pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY, bNumIcons);
  BltVObjectFromIndex(renderBuffer, guiPORTRAITICONS, sIconIndex, sIconX, sIconY);
}

void HandleRenderFaceAdjustments(FACETYPE *pFace, BOOLEAN fDisplayBuffer, BOOLEAN fUseExternBuffer,
                                 struct JSurface *buffer, int16_t sFaceX, int16_t sFaceY,
                                 uint16_t usEyesX, uint16_t usEyesY) {
  int16_t sIconX, sIconY;
  int16_t sIconIndex = -1;
  BOOLEAN fDoIcon = FALSE;
  struct JSurface *renderBuffer;
  int16_t sPtsAvailable = 0;
  uint16_t usMaximumPts = 0;
  wchar_t sString[32];
  uint16_t usTextWidth;
  BOOLEAN fAtGunRange = FALSE;
  BOOLEAN fShowNumber = FALSE;
  BOOLEAN fShowMaximum = FALSE;
  struct SOLDIERTYPE *pSoldier;
  int16_t sFontX, sFontY;
  int16_t sX1, sY1, sY2, sX2;
  uint32_t uiDestPitchBYTES;
  uint8_t *pDestBuf;
  uint16_t usLineColor;
  int8_t bNumRightIcons = 0;

  // If we are using an extern buffer...
  if (fUseExternBuffer) {
    renderBuffer = buffer;
  } else {
    if (fDisplayBuffer) {
      renderBuffer = pFace->autoDisplayBuffer;
    } else {
      renderBuffer = pFace->autoRestoreBuffer;
    }
  }

  // BLIT HATCH
  if (pFace->ubSoldierID != NOBODY) {
    pSoldier = MercPtrs[pFace->ubSoldierID];

    if ((MercPtrs[pFace->ubSoldierID]->bLife < CONSCIOUSNESS ||
         MercPtrs[pFace->ubSoldierID]->fDeadPanel)) {
      // Blit Closed eyes here!
      BltVObjectFromIndex(renderBuffer, pFace->uiVideoObject, 1, usEyesX, usEyesY);

      // Blit hatch!
      BltVObjectFromIndex(renderBuffer, guiHATCH, 0, sFaceX, sFaceY);
    }

    if (MercPtrs[pFace->ubSoldierID]->fMercAsleep == TRUE) {
      // blit eyes closed
      BltVObjectFromIndex(renderBuffer, pFace->uiVideoObject, 1, usEyesX, usEyesY);
    }

    if ((pSoldier->uiStatusFlags & SOLDIER_DEAD)) {
      // IF we are in the process of doing any deal/close animations, show face, not skill...
      if (!pSoldier->fClosePanel && !pSoldier->fDeadPanel && !pSoldier->fUIdeadMerc &&
          !pSoldier->fUICloseMerc) {
        // Put close panel there
        BltVObjectFromIndex(renderBuffer, guiDEAD, 5, sFaceX, sFaceY);

        // Blit hatch!
        BltVObjectFromIndex(renderBuffer, guiHATCH, 0, sFaceX, sFaceY);
      }
    }

    // ATE: If talking in popup, don't do the other things.....
    if (pFace->fTalking && gTacticalStatus.uiFlags & IN_ENDGAME_SEQUENCE) {
      return;
    }

    // ATE: Only do this, because we can be talking during an interrupt....
    if ((pFace->uiFlags & FACE_INACTIVE_HANDLED_ELSEWHERE) && !fUseExternBuffer) {
      // Don't do this if we are being handled elsewhere and it's not an extern buffer...
    } else {
      HandleFaceHilights(pFace, renderBuffer, sFaceX, sFaceY);

#ifdef JA2BETAVERSION
      if (pSoldier->bOppCnt != 0)
#else
      if (pSoldier->bOppCnt > 0)
#endif
      {
        SetFontDestBuffer(renderBuffer, 0, 0, 640, 480, FALSE);

        swprintf(sString, ARR_SIZE(sString), L"%d", pSoldier->bOppCnt);

        SetFont(TINYFONT1);
        SetFontForeground(FONT_DKRED);
        SetFontBackground(FONT_NEARBLACK);

        sX1 = (int16_t)(sFaceX);
        sY1 = (int16_t)(sFaceY);

        sX2 = sX1 + StringPixLength(sString, TINYFONT1) + 1;
        sY2 = sY1 + GetFontHeight(TINYFONT1) - 1;

        mprintf((int16_t)(sX1 + 1), (int16_t)(sY1 - 1), sString);
        SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);

        // Draw box
        pDestBuf = LockVSurface(renderBuffer, &uiDestPitchBYTES);
        SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, 640, 480);

        usLineColor = rgb32_to_rgb16(FROMRGB(105, 8, 9));
        RectangleDraw(TRUE, sX1, sY1, sX2, sY2, usLineColor, pDestBuf);

        JSurface_Unlock(renderBuffer);
      }

      if ((MercPtrs[pFace->ubSoldierID]->bInSector &&
           (((gTacticalStatus.ubCurrentTeam != OUR_TEAM) ||
             !OK_INTERRUPT_MERC(MercPtrs[pFace->ubSoldierID])) &&
            !gfHiddenInterrupt)) ||
          ((gfSMDisableForItems && !gfInItemPickupMenu) && gusSMCurrentMerc == pFace->ubSoldierID &&
           gsCurInterfacePanel == SM_PANEL)) {
        // Blit hatch!
        BltVObjectFromIndex(renderBuffer, guiHATCH, 0, sFaceX, sFaceY);
      }

      if (!pFace->fDisabled && !pFace->fInvalidAnim) {
        // Render text above here if that's what was asked for
        if (pFace->fDisplayTextOver != FACE_NO_TEXT_OVER) {
          SetFont(TINYFONT1);
          SetFontBackground(FONT_MCOLOR_BLACK);
          SetFontForeground(FONT_MCOLOR_WHITE);

          SetFontDestBuffer(renderBuffer, 0, 0, 640, 480, FALSE);

          VarFindFontCenterCoordinates(sFaceX, sFaceY, pFace->usFaceWidth, pFace->usFaceHeight,
                                       TINYFONT1, &sFontX, &sFontY, pFace->zDisplayText);

          if (pFace->fDisplayTextOver == FACE_DRAW_TEXT_OVER) {
            gprintfinvalidate(sFontX, sFontY, pFace->zDisplayText);
            mprintf(sFontX, sFontY, pFace->zDisplayText);
          } else if (pFace->fDisplayTextOver == FACE_ERASE_TEXT_OVER) {
            gprintfRestore(sFontX, sFontY, pFace->zDisplayText);
            pFace->fDisplayTextOver = FACE_NO_TEXT_OVER;
          }

          SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
        }
      }
    }

    // Check if a robot and is not controlled....
    if (MercPtrs[pFace->ubSoldierID]->uiStatusFlags & SOLDIER_ROBOT) {
      if (!CanRobotBeControlled(MercPtrs[pFace->ubSoldierID])) {
        // Not controlled robot
        sIconIndex = 5;
        fDoIcon = TRUE;
      }
    }

    if (ControllingRobot(MercPtrs[pFace->ubSoldierID])) {
      // controlling robot
      sIconIndex = 4;
      fDoIcon = TRUE;
    }

    // If blind...
    if (MercPtrs[pFace->ubSoldierID]->bBlindedCounter > 0) {
      DoRightIcon(renderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 6);
      bNumRightIcons++;
    }

    if (MercPtrs[pFace->ubSoldierID]->bDrugEffect[DRUG_TYPE_ADRENALINE]) {
      DoRightIcon(renderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 7);
      bNumRightIcons++;
    }

    if (GetDrunkLevel(MercPtrs[pFace->ubSoldierID]) != SOBER) {
      DoRightIcon(renderBuffer, pFace, sFaceX, sFaceY, bNumRightIcons, 8);
      bNumRightIcons++;
    }

    switch (pSoldier->bAssignment) {
      case DOCTOR:

        sIconIndex = 1;
        fDoIcon = TRUE;
        sPtsAvailable =
            CalculateHealingPointsForDoctor(MercPtrs[pFace->ubSoldierID], &usMaximumPts, FALSE);
        fShowNumber = TRUE;
        fShowMaximum = TRUE;

        // divide both amounts by 10 to make the displayed numbers a little more user-palatable
        // (smaller)
        sPtsAvailable = (sPtsAvailable + 5) / 10;
        usMaximumPts = (usMaximumPts + 5) / 10;
        break;

      case PATIENT:

        sIconIndex = 2;
        fDoIcon = TRUE;
        // show current health / maximum health
        sPtsAvailable = MercPtrs[pFace->ubSoldierID]->bLife;
        usMaximumPts = MercPtrs[pFace->ubSoldierID]->bLifeMax;
        fShowNumber = TRUE;
        fShowMaximum = TRUE;
        break;

      case TRAIN_SELF:
      case TRAIN_TOWN:
      case TRAIN_TEAMMATE:
      case TRAIN_BY_OTHER:
        sIconIndex = 3;
        fDoIcon = TRUE;
        fShowNumber = TRUE;
        fShowMaximum = TRUE;
        // there could be bonus pts for training at gun range
        if ((MercPtrs[pFace->ubSoldierID]->sSectorX == 13) &&
            (MercPtrs[pFace->ubSoldierID]->sSectorY == MAP_ROW_H) &&
            (MercPtrs[pFace->ubSoldierID]->bSectorZ == 0)) {
          fAtGunRange = TRUE;
        }

        switch (MercPtrs[pFace->ubSoldierID]->bAssignment) {
          case (TRAIN_SELF):
            sPtsAvailable = GetSoldierTrainingPts(MercPtrs[pFace->ubSoldierID],
                                                  MercPtrs[pFace->ubSoldierID]->bTrainStat,
                                                  fAtGunRange, &usMaximumPts);
            break;
          case (TRAIN_BY_OTHER):
            sPtsAvailable = GetSoldierStudentPts(MercPtrs[pFace->ubSoldierID],
                                                 MercPtrs[pFace->ubSoldierID]->bTrainStat,
                                                 fAtGunRange, &usMaximumPts);
            break;
          case (TRAIN_TOWN):
            sPtsAvailable =
                GetTownTrainPtsForCharacter(MercPtrs[pFace->ubSoldierID], &usMaximumPts);
            // divide both amounts by 10 to make the displayed numbers a little more user-palatable
            // (smaller)
            sPtsAvailable = (sPtsAvailable + 5) / 10;
            usMaximumPts = (usMaximumPts + 5) / 10;
            break;
          case (TRAIN_TEAMMATE):
            sPtsAvailable = GetBonusTrainingPtsDueToInstructor(
                MercPtrs[pFace->ubSoldierID], NULL, MercPtrs[pFace->ubSoldierID]->bTrainStat,
                fAtGunRange, &usMaximumPts);
            break;
        }
        break;

      case REPAIR:

        sIconIndex = 0;
        fDoIcon = TRUE;
        sPtsAvailable =
            CalculateRepairPointsForRepairman(MercPtrs[pFace->ubSoldierID], &usMaximumPts, FALSE);
        fShowNumber = TRUE;
        fShowMaximum = TRUE;

        // check if we are repairing a vehicle
        if (Menptr[pFace->ubSoldierID].bVehicleUnderRepairID != -1) {
          // reduce to a multiple of VEHICLE_REPAIR_POINTS_DIVISOR.  This way skill too low will
          // show up as 0 repair pts.
          sPtsAvailable -= (sPtsAvailable % VEHICLE_REPAIR_POINTS_DIVISOR);
          usMaximumPts -= (usMaximumPts % VEHICLE_REPAIR_POINTS_DIVISOR);
        }

        break;
    }

    // Check for being serviced...
    if (MercPtrs[pFace->ubSoldierID]->ubServicePartner != NOBODY) {
      // Doctor...
      sIconIndex = 1;
      fDoIcon = TRUE;
    }

    if (MercPtrs[pFace->ubSoldierID]->ubServiceCount != 0) {
      // Patient
      sIconIndex = 2;
      fDoIcon = TRUE;
    }

    if (fDoIcon) {
      // Find X, y for placement
      GetXYForIconPlacement(pFace, sIconIndex, sFaceX, sFaceY, &sIconX, &sIconY);
      BltVObjectFromIndex(renderBuffer, guiPORTRAITICONS, sIconIndex, sIconX, sIconY);

      // ATE: Show numbers only in mapscreen
      if (fShowNumber) {
        SetFontDestBuffer(renderBuffer, 0, 0, 640, 480, FALSE);

        if (fShowMaximum) {
          swprintf(sString, ARR_SIZE(sString), L"%d/%d", sPtsAvailable, usMaximumPts);
        } else {
          swprintf(sString, ARR_SIZE(sString), L"%d", sPtsAvailable);
        }

        usTextWidth = StringPixLength(sString, FONT10ARIAL);
        usTextWidth += 1;

        SetFont(FONT10ARIAL);
        SetFontForeground(FONT_YELLOW);
        SetFontBackground(FONT_BLACK);

        mprintf(sFaceX + pFace->usFaceWidth - usTextWidth, (int16_t)(sFaceY + 3), sString);
        SetFontDestBuffer(vsFB, 0, 0, 640, 480, FALSE);
      }
    }
  } else {
    if (pFace->ubCharacterNum == FATHER || pFace->ubCharacterNum == MICKY) {
      if (gMercProfiles[pFace->ubCharacterNum].bNPCData >= 5) {
        DoRightIcon(renderBuffer, pFace, sFaceX, sFaceY, 0, 8);
      }
    }
  }
}

BOOLEAN RenderAutoFace(int32_t iFaceIndex) {
  FACETYPE *pFace;

  // Check face index
  CHECKF(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  // Check for a valid slot!
  CHECKF(pFace->fAllocated != FALSE);

  // Check for disabled guy!
  CHECKF(pFace->fDisabled != TRUE);

  // Set shade
  if (pFace->ubSoldierID != NOBODY) {
    SetFaceShade(MercPtrs[pFace->ubSoldierID], pFace, FALSE);
  }

  // Blit face to save buffer!
  if (pFace->autoRestoreBuffer == vsSaveBuffer) {
    BltVObjectFromIndex(pFace->autoRestoreBuffer, pFace->uiVideoObject, 0, pFace->usFaceX,
                        pFace->usFaceY);
  } else {
    BltVObjectFromIndex(pFace->autoRestoreBuffer, pFace->uiVideoObject, 0, 0, 0);
  }

  HandleRenderFaceAdjustments(pFace, FALSE, FALSE, 0, pFace->usFaceX, pFace->usFaceY,
                              pFace->usEyesX, pFace->usEyesY);

  // Restore extern rect
  if (pFace->autoRestoreBuffer == vsSaveBuffer) {
    FaceRestoreSavedBackgroundRect(iFaceIndex, (int16_t)(pFace->usFaceX), (int16_t)(pFace->usFaceY),
                                   (int16_t)(pFace->usFaceX), (int16_t)(pFace->usFaceY),
                                   (int16_t)(pFace->usFaceWidth), (int16_t)(pFace->usFaceHeight));
  } else {
    FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usFaceX, pFace->usFaceY, 0, 0,
                                   pFace->usFaceWidth, pFace->usFaceHeight);
  }

  return (TRUE);
}

BOOLEAN ExternRenderFaceFromSoldier(struct JSurface *buffer, uint8_t ubSoldierID, int16_t sX,
                                    int16_t sY) {
  // Check for valid soldier
  CHECKF(ubSoldierID != NOBODY);

  return (ExternRenderFace(buffer, MercPtrs[ubSoldierID]->iFaceIndex, sX, sY));
}

BOOLEAN ExternRenderFace(struct JSurface *buffer, int32_t iFaceIndex, int16_t sX, int16_t sY) {
  uint16_t usEyesX;
  uint16_t usEyesY;
  uint16_t usMouthX;
  uint16_t usMouthY;
  FACETYPE *pFace;

  // Check face index
  CHECKF(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  // Check for a valid slot!
  CHECKF(pFace->fAllocated != FALSE);

  // Here, any face can be rendered, even if disabled

  // Set shade
  if (pFace->ubSoldierID != NOBODY) {
    SetFaceShade(MercPtrs[pFace->ubSoldierID], pFace, TRUE);
  }

  // Blit face to save buffer!
  BltVObjectFromIndex(buffer, pFace->uiVideoObject, 0, sX, sY);

  GetFaceRelativeCoordinates(pFace, &usEyesX, &usEyesY, &usMouthX, &usMouthY);

  HandleRenderFaceAdjustments(pFace, FALSE, TRUE, buffer, sX, sY, (uint16_t)(sX + usEyesX),
                              (uint16_t)(sY + usEyesY));

  // Restore extern rect
  if (buffer == vsSaveBuffer) {
    RestoreExternBackgroundRect(sX, sY, pFace->usFaceWidth, pFace->usFaceWidth);
  }

  return (TRUE);
}

void NewEye(FACETYPE *pFace) {
  switch (pFace->sEyeFrame) {
    case 0:  // pFace->sEyeFrame = (int16_t)Random(2);	// normal - can blink or frown
      if (pFace->ubExpression == ANGRY) {
        pFace->ubEyeWait = 0;
        pFace->sEyeFrame = 3;
      } else if (pFace->ubExpression == SURPRISED) {
        pFace->ubEyeWait = 0;
        pFace->sEyeFrame = 4;
      } else
        // if (pFace->sEyeFrame && Talk.talking && Talk.expression != DYING)
        ///    pFace->sEyeFrame = 3;
        // else
        pFace->sEyeFrame = 1;
      break;
    case 1:  // starting to blink  - has to finish unless dying
             // if (Talk.expression == DYING)
             //    pFace->sEyeFrame = 1;
             // else
      pFace->sEyeFrame = 2;
      break;
    case 2:  // pFace->sEyeFrame = (int16_t)Random(2);	// finishing blink - can go normal or frown
             // if (pFace->sEyeFrame && Talk.talking)
             //    pFace->sEyeFrame = 3;
             // else
             //   if (Talk.expression == ANGRY)
             // pFace->sEyeFrame = 3;
      //   else
      pFace->sEyeFrame = 0;
      break;

    case 3:  // pFace->sEyeFrame = 4; break;	// frown

      pFace->ubEyeWait++;

      if (pFace->ubEyeWait > 6) {
        pFace->sEyeFrame = 0;
      }
      break;

    case 4:

      pFace->ubEyeWait++;

      if (pFace->ubEyeWait > 6) {
        pFace->sEyeFrame = 0;
      }
      break;

    case 5:
      pFace->sEyeFrame = 6;

      pFace->sEyeFrame = 0;
      break;

    case 6:
      pFace->sEyeFrame = 7;
      break;
    case 7:
      pFace->sEyeFrame = (int16_t)Random(2);  // can stop frowning or continue
      // if (pFace->sEyeFrame && Talk.expression != DYING)
      //   pFace->sEyeFrame = 8;
      // else
      //    pFace->sEyeFrame = 0;
      // break;
    case 8:
      pFace->sEyeFrame = 9;
      break;
    case 9:
      pFace->sEyeFrame = 10;
      break;
    case 10:
      pFace->sEyeFrame = 11;
      break;
    case 11:
      pFace->sEyeFrame = 12;
      break;
    case 12:
      pFace->sEyeFrame = 0;
      break;
  }
}

void NewMouth(FACETYPE *pFace) {
  BOOLEAN OK = FALSE;
  uint16_t sOld = pFace->sMouthFrame;

  // if (audio_gap_active == 1)
  //  {
  //   Talk.mouth = 0;
  //   return;
  //  }

  do {
    // Talk.mouth = random(4);

    pFace->sMouthFrame = (int16_t)Random(6);

    if (pFace->sMouthFrame > 3) {
      pFace->sMouthFrame = 0;
    }

    switch (sOld) {
      case 0:
        if (pFace->sMouthFrame != 0) OK = TRUE;
        break;
      case 1:
        if (pFace->sMouthFrame != 1) OK = TRUE;
        break;
      case 2:
        if (pFace->sMouthFrame != 2) OK = TRUE;
        break;
      case 3:
        if (pFace->sMouthFrame != 3) OK = TRUE;
        break;
    }

  } while (!OK);
}

void HandleAutoFaces() {
  uint32_t uiCount;
  FACETYPE *pFace;
  int8_t bLife;
  int8_t bAPs;
  BOOLEAN fRerender = FALSE;
  BOOLEAN fHandleFace;
  BOOLEAN fHandleUIHatch;
  struct SOLDIERTYPE *pSoldier;

  for (uiCount = 0; uiCount < guiNumFaces; uiCount++) {
    fRerender = FALSE;
    fHandleFace = TRUE;
    fHandleUIHatch = FALSE;

    // OK, NOW, check if our bLife status has changed, re-render if so!
    if (gFacesData[uiCount].fAllocated) {
      pFace = &gFacesData[uiCount];

      // Are we a soldier?
      if (pFace->ubSoldierID != NOBODY) {
        // Get Life now
        pSoldier = MercPtrs[pFace->ubSoldierID];
        bLife = pSoldier->bLife;
        bAPs = pSoldier->bActionPoints;

        if (pSoldier->ubID == gsSelectedGuy && gfUIHandleSelectionAboveGuy) {
          pFace->uiFlags |= FACE_SHOW_WHITE_HILIGHT;
        } else {
          pFace->uiFlags &= (~FACE_SHOW_WHITE_HILIGHT);
        }

        if (pSoldier->sGridNo != pSoldier->sFinalDestination && pSoldier->sGridNo != NOWHERE) {
          pFace->uiFlags |= FACE_SHOW_MOVING_HILIGHT;
        } else {
          pFace->uiFlags &= (~FACE_SHOW_MOVING_HILIGHT);
        }

        if (pSoldier->bStealthMode != pFace->bOldStealthMode) {
          fRerender = TRUE;
        }

        // Check if we have fallen below OKLIFE...
        if (bLife < OKLIFE && pFace->bOldSoldierLife >= OKLIFE) {
          fRerender = TRUE;
        }

        if (bLife >= OKLIFE && pFace->bOldSoldierLife < OKLIFE) {
          fRerender = TRUE;
        }

        // Check if we have fallen below CONSCIOUSNESS
        if (bLife < CONSCIOUSNESS && pFace->bOldSoldierLife >= CONSCIOUSNESS) {
          fRerender = TRUE;
        }

        if (bLife >= CONSCIOUSNESS && pFace->bOldSoldierLife < CONSCIOUSNESS) {
          fRerender = TRUE;
        }

        if (pSoldier->bOppCnt != pFace->bOldOppCnt) {
          fRerender = TRUE;
        }

        // Check if assignment is idfferent....
        if (pSoldier->bAssignment != pFace->bOldAssignment) {
          pFace->bOldAssignment = pSoldier->bAssignment;
          fRerender = TRUE;
        }

        // Check if we have fallen below CONSCIOUSNESS
        if (bAPs == 0 && pFace->bOldActionPoints > 0) {
          fRerender = TRUE;
        }

        if (bAPs > 0 && pFace->bOldActionPoints == 0) {
          fRerender = TRUE;
        }

        if (!(pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT) && pFace->fOldShowHighlight) {
          fRerender = TRUE;
        }

        if ((pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT) && !(pFace->fOldShowHighlight)) {
          fRerender = TRUE;
        }

        if (!(pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT) && pFace->fOldShowMoveHilight) {
          fRerender = TRUE;
        }

        if ((pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT) && !(pFace->fOldShowMoveHilight)) {
          fRerender = TRUE;
        }

        if (pFace->ubOldServiceCount != pSoldier->ubServiceCount) {
          fRerender = TRUE;
          pFace->ubOldServiceCount = pSoldier->ubServiceCount;
        }

        if (pFace->fOldCompatibleItems != pFace->fCompatibleItems || gfInItemPickupMenu ||
            gpItemPointer != NULL) {
          fRerender = TRUE;
          pFace->fOldCompatibleItems = pFace->fCompatibleItems;
        }

        if (pFace->ubOldServicePartner != pSoldier->ubServicePartner) {
          fRerender = TRUE;
          pFace->ubOldServicePartner = pSoldier->ubServicePartner;
        }

        pFace->fOldHandleUIHatch = fHandleUIHatch;
        pFace->bOldSoldierLife = bLife;
        pFace->bOldActionPoints = bAPs;
        pFace->bOldStealthMode = pSoldier->bStealthMode;
        pFace->bOldOppCnt = pSoldier->bOppCnt;

        if (pFace->uiFlags & FACE_SHOW_WHITE_HILIGHT) {
          pFace->fOldShowHighlight = TRUE;
        } else {
          pFace->fOldShowHighlight = FALSE;
        }

        if (pFace->uiFlags & FACE_SHOW_MOVING_HILIGHT) {
          pFace->fOldShowMoveHilight = TRUE;
        } else {
          pFace->fOldShowMoveHilight = FALSE;
        }

        if (pSoldier->fGettingHit && pSoldier->fFlashPortrait == FLASH_PORTRAIT_STOP) {
          pSoldier->fFlashPortrait = TRUE;
          pSoldier->bFlashPortraitFrame = FLASH_PORTRAIT_STARTSHADE;
          RESETTIMECOUNTER(pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY);
          fRerender = TRUE;
        }
        if (pSoldier->fFlashPortrait == FLASH_PORTRAIT_START) {
          // Loop through flash values
          if (TIMECOUNTERDONE(pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY)) {
            RESETTIMECOUNTER(pSoldier->PortraitFlashCounter, FLASH_PORTRAIT_DELAY);
            pSoldier->bFlashPortraitFrame++;

            if (pSoldier->bFlashPortraitFrame > FLASH_PORTRAIT_ENDSHADE) {
              pSoldier->bFlashPortraitFrame = FLASH_PORTRAIT_ENDSHADE;

              if (pSoldier->fGettingHit) {
                pSoldier->fFlashPortrait = FLASH_PORTRAIT_WAITING;
              } else {
                // Render face again!
                pSoldier->fFlashPortrait = FLASH_PORTRAIT_STOP;
              }

              fRerender = TRUE;
            }
          }
        }
        // CHECK IF WE WERE WAITING FOR GETTING HIT TO FINISH!
        if (!pSoldier->fGettingHit && pSoldier->fFlashPortrait == FLASH_PORTRAIT_WAITING) {
          pSoldier->fFlashPortrait = FALSE;
          fRerender = TRUE;
        }

        if (pSoldier->fFlashPortrait == FLASH_PORTRAIT_START) {
          fRerender = TRUE;
        }

        if (pFace->uiFlags & FACE_REDRAW_WHOLE_FACE_NEXT_FRAME) {
          pFace->uiFlags &= ~FACE_REDRAW_WHOLE_FACE_NEXT_FRAME;

          fRerender = TRUE;
        }

        if (fInterfacePanelDirty == DIRTYLEVEL2 && IsTacticalMode()) {
          fRerender = TRUE;
        }

        if (fRerender) {
          RenderAutoFace(uiCount);
        }

        if (bLife < CONSCIOUSNESS) {
          fHandleFace = FALSE;
        }
      }

      if (fHandleFace) {
        BlinkAutoFace(uiCount);
      }

      MouthAutoFace(uiCount);
    }
  }
}

void HandleTalkingAutoFaces() {
  uint32_t uiCount;
  for (uiCount = 0; uiCount < guiNumFaces; uiCount++) {
    // OK, NOW, check if our bLife status has changed, re-render if so!
    if (gFacesData[uiCount].fAllocated) {
      HandleTalkingAutoFace(uiCount);
    }
  }
}

BOOLEAN FaceRestoreSavedBackgroundRect(int32_t iFaceIndex, int16_t sDestLeft, int16_t sDestTop,
                                       int16_t sSrcLeft, int16_t sSrcTop, int16_t sWidth,
                                       int16_t sHeight) {
  FACETYPE *pFace;
  uint32_t uiDestPitchBYTES, uiSrcPitchBYTES;
  uint8_t *pDestBuf, *pSrcBuf;

  // Check face index
  CHECKF(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  pDestBuf = LockVSurface(pFace->autoDisplayBuffer, &uiDestPitchBYTES);
  pSrcBuf = LockVSurface(pFace->autoRestoreBuffer, &uiSrcPitchBYTES);

  Blt16BPPTo16BPP((uint16_t *)pDestBuf, uiDestPitchBYTES, (uint16_t *)pSrcBuf, uiSrcPitchBYTES,
                  sDestLeft, sDestTop, sSrcLeft, sSrcTop, sWidth, sHeight);

  JSurface_Unlock(pFace->autoDisplayBuffer);
  JSurface_Unlock(pFace->autoRestoreBuffer);

  // Add rect to frame buffer queue
  if (pFace->autoDisplayBuffer == vsFB) {
    InvalidateRegionEx(sDestLeft - 2, sDestTop - 2, (sDestLeft + sWidth + 3),
                       (sDestTop + sHeight + 2), 0);
  }
  return (TRUE);
}

BOOLEAN SetFaceTalking(int32_t iFaceIndex, char *zSoundFile, wchar_t *zTextString, uint32_t usRate,
                       uint32_t ubVolume, uint32_t ubLoops, uint32_t uiPan) {
  FACETYPE *pFace;

  pFace = &gFacesData[iFaceIndex];

  // Set face to talking
  pFace->fTalking = TRUE;
  pFace->fAnimatingTalking = TRUE;
  pFace->fFinishTalking = FALSE;

  if (!AreInMeanwhile()) {
    TurnOnSectorLocator(pFace->ubCharacterNum);
  }

  // Play sample
  if (gGameSettings.fOptions[TOPTION_SPEECH])
    pFace->uiSoundID =
        PlayJA2GapSample(zSoundFile, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN, &(pFace->GapList));
  else
    pFace->uiSoundID = SOUND_ERROR;

  if (pFace->uiSoundID != SOUND_ERROR) {
    pFace->fValidSpeech = TRUE;

    pFace->uiTalkingFromVeryBeginningTimer = GetJA2Clock();
  } else {
    pFace->fValidSpeech = FALSE;

    // Set delay based on sound...
    pFace->uiTalkingTimer = pFace->uiTalkingFromVeryBeginningTimer = GetJA2Clock();

    pFace->uiTalkingDuration = FindDelayForString(zTextString);
  }

  return (TRUE);
}

BOOLEAN ExternSetFaceTalking(int32_t iFaceIndex, uint32_t uiSoundID) {
  FACETYPE *pFace;

  pFace = &gFacesData[iFaceIndex];

  // Set face to talki	ng
  pFace->fTalking = TRUE;
  pFace->fAnimatingTalking = TRUE;
  pFace->fFinishTalking = FALSE;
  pFace->fValidSpeech = TRUE;

  pFace->uiSoundID = uiSoundID;

  return (TRUE);
}

void InternalShutupaYoFace(int32_t iFaceIndex, BOOLEAN fForce) {
  FACETYPE *pFace;

  // Check face index
  CHECKV(iFaceIndex != -1);

  pFace = &gFacesData[iFaceIndex];

  if (pFace->fTalking) {
    // OK, only do this if we have been talking for a min. amount fo time...
    if ((GetJA2Clock() - pFace->uiTalkingFromVeryBeginningTimer) < 500 && !fForce) {
      return;
    }

    if (pFace->uiSoundID != SOUND_ERROR) {
      SoundStop(pFace->uiSoundID);
    }

    // Remove gap info
    AudioGapListDone(&(pFace->GapList));

    // Shutup mouth!
    pFace->sMouthFrame = 0;

    // ATE: Only change if active!
    if (!pFace->fDisabled) {
      if (pFace->autoRestoreBuffer == vsSaveBuffer) {
        FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usMouthX, pFace->usMouthY,
                                       pFace->usMouthX, pFace->usMouthY, pFace->usMouthWidth,
                                       pFace->usMouthHeight);
      } else {
        FaceRestoreSavedBackgroundRect(iFaceIndex, pFace->usMouthX, pFace->usMouthY,
                                       pFace->usMouthOffsetX, pFace->usMouthOffsetY,
                                       pFace->usMouthWidth, pFace->usMouthHeight);
      }
    }
    // OK, smart guy, make sure this guy has finished talking,
    // before attempting to end dialogue UI.
    pFace->fTalking = FALSE;

    // Call dialogue handler function
    HandleDialogueEnd(pFace);

    pFace->fTalking = FALSE;
    pFace->fAnimatingTalking = FALSE;

    gfUIWaitingForUserSpeechAdvance = FALSE;
  }
}

void ShutupaYoFace(int32_t iFaceIndex) { InternalShutupaYoFace(iFaceIndex, TRUE); }

void SetupFinalTalkingDelay(FACETYPE *pFace) {
  pFace->fFinishTalking = TRUE;

  pFace->fAnimatingTalking = FALSE;

  pFace->uiTalkingTimer = GetJA2Clock();

  if (gGameSettings.fOptions[TOPTION_SUBTITLES]) {
    // pFace->uiTalkingDuration = FINAL_TALKING_DURATION;
    pFace->uiTalkingDuration = 300;
  } else {
    pFace->uiTalkingDuration = 300;
  }

  pFace->sMouthFrame = 0;

  // Close mouth!
  if (!pFace->fDisabled) {
    if (pFace->autoRestoreBuffer == vsSaveBuffer) {
      FaceRestoreSavedBackgroundRect(pFace->iID, pFace->usMouthX, pFace->usMouthY, pFace->usMouthX,
                                     pFace->usMouthY, pFace->usMouthWidth, pFace->usMouthHeight);
    } else {
      FaceRestoreSavedBackgroundRect(pFace->iID, pFace->usMouthX, pFace->usMouthY,
                                     pFace->usMouthOffsetX, pFace->usMouthOffsetY,
                                     pFace->usMouthWidth, pFace->usMouthHeight);
    }
  }

  // Setup flag to wait for advance ( because we have no text! )
  if (gGameSettings.fOptions[TOPTION_KEY_ADVANCE_SPEECH] &&
      (pFace->uiFlags & FACE_POTENTIAL_KEYWAIT)) {
    // Check if we have had valid speech!
    if (!pFace->fValidSpeech || gGameSettings.fOptions[TOPTION_SUBTITLES]) {
      // Set false!
      pFace->fFinishTalking = FALSE;
      // Set waiting for advance to true!
      gfUIWaitingForUserSpeechAdvance = TRUE;
    }
  }

  // Set final delay!
  pFace->fValidSpeech = FALSE;
}
