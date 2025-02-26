// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/AutoBandage.h"

#include "CharList.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicTurns.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/Items.h"
#include "Tactical/Menptr.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierProfile.h"
#include "TacticalAI/AI.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/MercTextBox.h"
#include "Utils/Message.h"
#include "Utils/MusicControl.h"
#include "Utils/Text.h"
#include "Utils/WordWrap.h"

extern uint32_t giMercPanelImage;
extern FACETYPE *gpCurrentTalkingFace;

// max number of merc faces per row in autobandage box
#define NUMBER_MERC_FACES_AUTOBANDAGE_BOX 4

wchar_t *sAutoBandageString = NULL;
int32_t giBoxId = -1;
uint16_t gusTextBoxWidth = 0;
uint16_t gusTextBoxHeight = 0;
BOOLEAN gfBeginningAutoBandage = FALSE;
int16_t gsX = 0;
int16_t gsY = 0;
uint32_t guiAutoBandageSeconds = 0;
BOOLEAN fAutoBandageComplete = FALSE;
BOOLEAN fEndAutoBandage = FALSE;

BOOLEAN gfAutoBandageFailed;

// the button and associated image for ending autobandage
int32_t iEndAutoBandageButton[2];
int32_t iEndAutoBandageButtonImage[2];

struct MOUSE_REGION gAutoBandageRegion;

// the lists of the doctor and patient
int32_t iDoctorList[MAX_CHARACTER_COUNT];
int32_t iPatientList[MAX_CHARACTER_COUNT];

// faces for update panel
int32_t giAutoBandagesSoldierFaces[2 * MAX_CHARACTER_COUNT];

// has the button for autobandage end been setup yet
BOOLEAN fAutoEndBandageButtonCreated = FALSE;

void BeginAutoBandageCallBack(uint8_t bExitValue);
void CancelAutoBandageCallBack(uint8_t bExitValue);

// the update box for autobandaging mercs
void CreateTerminateAutoBandageButton(int16_t sX, int16_t sY);
void DestroyTerminateAutoBandageButton(void);
void DisplayAutoBandageUpdatePanel(void);
void EndAutobandageButtonCallback(GUI_BUTTON *btn, int32_t reason);
void SetUpAutoBandageUpdatePanel(void);
BOOLEAN AddFacesToAutoBandageBox(void);
BOOLEAN RenderSoldierSmallFaceForAutoBandagePanel(int32_t iIndex, int16_t sCurrentXPosition,
                                                  int16_t sCurrentYPosition);
void StopAutoBandageButtonCallback(GUI_BUTTON *btn, int32_t reason);
BOOLEAN RemoveFacesForAutoBandage(void);

extern BOOLEAN CanCharacterAutoBandageTeammate(struct SOLDIERTYPE *pSoldier);
extern BOOLEAN CanCharacterBeAutoBandagedByTeammate(struct SOLDIERTYPE *pSoldier);
extern uint8_t NumEnemyInSector();

void BeginAutoBandage() {
  int32_t cnt;
  BOOLEAN fFoundAGuy = FALSE;
  struct SOLDIERTYPE *pSoldier;
  BOOLEAN fFoundAMedKit = FALSE;

  // If we are in combat, we con't...
  if ((gTacticalStatus.uiFlags & INCOMBAT) || (NumEnemyInSector() != 0)) {
    DoMessageBox(MSG_BOX_BASIC_STYLE, Message[STR_SECTOR_NOT_CLEARED], GAME_SCREEN,
                 (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
    return;
  }

  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;
  // check for anyone needing bandages
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    // if the soldier isn't active or in sector, we have problems..leave
    if (!(IsSolActive(pSoldier)) || !IsSolInSector(pSoldier) ||
        (pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (GetSolAssignment(pSoldier) == VEHICLE)) {
      continue;
    }

    // can this character be helped out by a teammate?
    if (CanCharacterBeAutoBandagedByTeammate(pSoldier) == TRUE) {
      fFoundAGuy = TRUE;
      if (fFoundAGuy && fFoundAMedKit) {
        break;
      }
    }
    if (FindObjClass(pSoldier, IC_MEDKIT) != NO_SLOT) {
      fFoundAMedKit = TRUE;
      if (fFoundAGuy && fFoundAMedKit) {
        break;
      }
    }
  }

  if (!fFoundAGuy) {
    DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[AUTOBANDAGE_NOT_NEEDED], GAME_SCREEN,
                 (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
  } else if (!fFoundAMedKit) {
    DoMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[9], GAME_SCREEN,
                 (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
  } else {
    if (!CanAutoBandage(FALSE)) {
      DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[CANT_AUTOBANDAGE_PROMPT], GAME_SCREEN,
                   (uint8_t)MSG_BOX_FLAG_OK, NULL, NULL);
    } else {
      // Confirm if we want to start or not....
      DoMessageBox(MSG_BOX_BASIC_STYLE, TacticalStr[BEGIN_AUTOBANDAGE_PROMPT_STR], GAME_SCREEN,
                   (uint8_t)MSG_BOX_FLAG_YESNO, BeginAutoBandageCallBack, NULL);
    }
  }
}

void HandleAutoBandagePending() {
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier = NULL;

  // OK, if we have a pending autobandage....
  // check some conditions
  if (gTacticalStatus.fAutoBandagePending) {
    // All dailogue done, music, etc...
    // if ( gubMusicMode != MUSIC_TACTICAL_VICTORY && DialogueQueueIsEmpty( ) )
    if (!DialogueQueueIsEmpty()) {
      return;
    }

    // If there is no actively talking guy...
    if (gpCurrentTalkingFace != NULL) {
      return;
    }

    // Do any guys have pending actions...?
    cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         cnt++, pSoldier++) {
      // Are we in sector?
      if (IsSolActive(pSoldier)) {
        if (GetSolSectorX(pSoldier) == gWorldSectorX && GetSolSectorY(pSoldier) == gWorldSectorY &&
            GetSolSectorZ(pSoldier) == gbWorldSectorZ && !pSoldier->fBetweenSectors) {
          if (pSoldier->ubPendingAction != NO_PENDING_ACTION) {
            return;
          }
        }
      }
    }

    // Do was have any menus up?
    if (AreWeInAUIMenu()) {
      return;
    }

    // If here, all's a go!
    gTacticalStatus.fAutoBandagePending = FALSE;
    BeginAutoBandage();
  }
}

void SetAutoBandagePending(BOOLEAN fSet) { gTacticalStatus.fAutoBandagePending = fSet; }

// Should we ask buddy ti auto bandage...?
void ShouldBeginAutoBandage() {
  // If we are in combat, we con't...
  if (gTacticalStatus.uiFlags & INCOMBAT) {
    return;
  }

  // ATE: If not in endgame
  if ((gTacticalStatus.uiFlags & IN_DEIDRANNA_ENDGAME)) {
    return;
  }

  if (CanAutoBandage(FALSE)) {
    // OK, now setup as a pending event...
    SetAutoBandagePending(TRUE);
  }
}

BOOLEAN HandleAutoBandage() {
  InputAtom InputEvent;

  if (gTacticalStatus.fAutoBandageMode) {
    if (gfBeginningAutoBandage) {
      // Shadow area
      ShadowVideoSurfaceRect(vsFB, 0, 0, 640, 480);
      InvalidateScreen();
      RefreshScreen(NULL);
    }

    DisplayAutoBandageUpdatePanel();

    EndFrameBufferRender();

    // Handle strategic engine
    HandleStrategicTurn();

    HandleTeamServices(OUR_TEAM);

    if (guiAutoBandageSeconds <= 120) {
      guiAutoBandageSeconds += 5;
    }

    // Execute Tactical Overhead
    ExecuteOverhead();

    // Deque all game events
    DequeAllGameEvents(TRUE);

    while (DequeueEvent(&InputEvent) == TRUE) {
      if (InputEvent.usEvent == KEY_UP) {
        if (((InputEvent.usParam == ESC) && (fAutoBandageComplete == FALSE)) ||
            (((InputEvent.usParam == ENTER) || (InputEvent.usParam == SPACE)) &&
             (fAutoBandageComplete == TRUE))) {
          AutoBandage(FALSE);
        }
      }
    }

    gfBeginningAutoBandage = FALSE;

    if (fEndAutoBandage) {
      AutoBandage(FALSE);
      fEndAutoBandage = FALSE;
    }

    return (TRUE);
  }

  return (FALSE);
}

BOOLEAN CreateAutoBandageString(void) {
  int32_t cnt;
  uint8_t ubDoctor[20], ubDoctors = 0;
  uint32_t uiDoctorNameStringLength = 1;  // for end-of-string character
  wchar_t *sTemp;
  struct SOLDIERTYPE *pSoldier;

  cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bInSector && pSoldier->bLife >= OKLIFE &&
        !(pSoldier->bCollapsed) && pSoldier->bMedical > 0 &&
        FindObjClass(pSoldier, IC_MEDKIT) != NO_SLOT) {
      ubDoctor[ubDoctors] = GetSolID(pSoldier);
      ubDoctors++;
      // increase the length of the string by the size of the name
      // plus 2, one for the comma and one for the space after that
      uiDoctorNameStringLength += wcslen(pSoldier->name) + 2;
    }
  }
  if (ubDoctors == 0) {
    return (FALSE);
  }

  if (ubDoctors == 1) {
    uiDoctorNameStringLength += wcslen(Message[STR_IS_APPLYING_FIRST_AID]);
  } else {
    uiDoctorNameStringLength += wcslen(Message[STR_ARE_APPLYING_FIRST_AID]);
  }

  sAutoBandageString =
      (wchar_t *)MemRealloc(sAutoBandageString, uiDoctorNameStringLength * sizeof(wchar_t));
  if (!sAutoBandageString) {
    return (FALSE);
  }

  if (ubDoctors == 1) {
    swprintf(sAutoBandageString, uiDoctorNameStringLength, Message[STR_IS_APPLYING_FIRST_AID],
             MercPtrs[ubDoctor[0]]->name);
  } else {
    // make a temporary string to hold most of the doctors names joined by commas
    sTemp = (wchar_t *)MemAlloc(uiDoctorNameStringLength * sizeof(wchar_t));
    //	sTemp = MemAlloc( 1000 );
    if (!sTemp) {
      return (FALSE);
    }
    wcscpy(sTemp, L"");
    for (cnt = 0; cnt < ubDoctors - 1; cnt++) {
      wcscat(sTemp, MercPtrs[ubDoctor[cnt]]->name);
      if (ubDoctors > 2) {
        if (cnt == ubDoctors - 2) {
          wcscat(sTemp, L",");
        } else {
          wcscat(sTemp, L", ");
        }
      }
    }
    swprintf(sAutoBandageString, uiDoctorNameStringLength, Message[STR_ARE_APPLYING_FIRST_AID],
             sTemp, MercPtrs[ubDoctor[ubDoctors - 1]]->name);
    MemFree(sTemp);
  }
  return (TRUE);
}

void SetAutoBandageComplete(void) {
  // this will set the fact autobandage is complete
  fAutoBandageComplete = TRUE;

  return;
}

void AutoBandage(BOOLEAN fStart) {
  SGPRect aRect;
  uint8_t ubLoop;
  int32_t cnt;
  struct SOLDIERTYPE *pSoldier;

  if (fStart) {
    gTacticalStatus.fAutoBandageMode = TRUE;
    gTacticalStatus.uiFlags |= OUR_MERCS_AUTO_MOVE;

    gfAutoBandageFailed = FALSE;

    // ste up the autobandage panel
    SetUpAutoBandageUpdatePanel();

    // Lock UI!
    // guiPendingOverrideEvent = LU_BEGINUILOCK;
    HandleTacticalUI();

    PauseGame();
    // Compress time...
    // SetGameTimeCompressionLevel( TIME_COMPRESS_5MINS );

    cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         cnt++, pSoldier++) {
      if (IsSolActive(pSoldier)) {
        pSoldier->bSlotItemTakenFrom = NO_SLOT;
        pSoldier->ubAutoBandagingMedic = NOBODY;
      }
    }

    ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Begin auto bandage.");

    if (CreateAutoBandageString()) {
      giBoxId = PrepareMercPopupBox(-1, DIALOG_MERC_POPUP_BACKGROUND, DIALOG_MERC_POPUP_BORDER,
                                    sAutoBandageString, 200, 40, 10, 30, &gusTextBoxWidth,
                                    &gusTextBoxHeight);
    }

    aRect.iTop = 0;
    aRect.iLeft = 0;
    aRect.iBottom = INV_INTERFACE_START_Y;
    aRect.iRight = 640;

    // Determine position ( centered in rect )
    gsX = (int16_t)((((aRect.iRight - aRect.iLeft) - gusTextBoxWidth) / 2) + aRect.iLeft);
    gsY = (int16_t)((((aRect.iBottom - aRect.iTop) - gusTextBoxHeight) / 2) + aRect.iTop);

    // build a mask
    MSYS_DefineRegion(&gAutoBandageRegion, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL,
                      MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

    gfBeginningAutoBandage = TRUE;

  } else {
    gTacticalStatus.fAutoBandageMode = FALSE;
    gTacticalStatus.uiFlags &= (~OUR_MERCS_AUTO_MOVE);

    // make sure anyone under AI control has their action cancelled
    cnt = gTacticalStatus.Team[OUR_TEAM].bFirstID;
    for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID;
         cnt++, pSoldier++) {
      if (IsSolActive(pSoldier)) {
        ActionDone(pSoldier);
        if (pSoldier->bSlotItemTakenFrom != NO_SLOT) {
          // swap our old hand item back to the main hand
          SwapObjs(&(pSoldier->inv[HANDPOS]), &(pSoldier->inv[pSoldier->bSlotItemTakenFrom]));
        }

        // ATE: Mkae everyone stand up!
        if (pSoldier->bLife >= OKLIFE && !pSoldier->bCollapsed) {
          if (gAnimControl[pSoldier->usAnimState].ubHeight != ANIM_STAND) {
            ChangeSoldierStance(pSoldier, ANIM_STAND);
          }
        }
      }
    }

    ubLoop = gTacticalStatus.Team[gbPlayerNum].bFirstID;
    for (; ubLoop <= gTacticalStatus.Team[gbPlayerNum].bLastID; ubLoop++) {
      ActionDone(MercPtrs[ubLoop]);

      // If anyone is still doing aid animation, stop!
      if (MercPtrs[ubLoop]->usAnimState == GIVING_AID) {
        SoldierGotoStationaryStance(MercPtrs[ubLoop]);
      }
    }

    // UnLock UI!
    guiPendingOverrideEvent = LU_ENDUILOCK;
    HandleTacticalUI();

    UnPauseGame();
    // Bring time back...
    // SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );

    // Warp game time by the amount of time it took to autobandage.
    WarpGameTime(guiAutoBandageSeconds, WARPTIME_NO_PROCESSING_OF_EVENTS);

    DestroyTerminateAutoBandageButton();

    // Delete popup!
    RemoveMercPopupBoxFromIndex(giBoxId);
    giBoxId = -1;
    ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"End auto bandage.");

    // build a mask
    MSYS_RemoveRegion(&gAutoBandageRegion);

    // clear faces for auto bandage
    RemoveFacesForAutoBandage();

    SetRenderFlags(RENDER_FLAG_FULL);
    fInterfacePanelDirty = DIRTYLEVEL2;

    if (gfAutoBandageFailed) {
      // inform player some mercs could not be bandaged
      DoScreenIndependantMessageBox(pDoctorWarningString[1], MSG_BOX_FLAG_OK, NULL);
      gfAutoBandageFailed = FALSE;
    }
  }
  guiAutoBandageSeconds = 0;

  ResetAllMercSpeeds();
}

void BeginAutoBandageCallBack(uint8_t bExitValue) {
  if (bExitValue == MSG_BOX_RETURN_YES) {
    fRestoreBackgroundForMessageBox = TRUE;
    AutoBandage(TRUE);
  }
}

void SetUpAutoBandageUpdatePanel(void) {
  int32_t iNumberDoctoring = 0;
  int32_t iNumberPatienting = 0;
  int32_t iNumberOnTeam = 0;
  int32_t iCounterA = 0;

  // reset the tables of merc ids
  memset(iDoctorList, -1, sizeof(int32_t) * MAX_CHARACTER_COUNT);
  memset(iPatientList, -1, sizeof(int32_t) * MAX_CHARACTER_COUNT);

  // grab number of potential grunts on players team
  iNumberOnTeam = gTacticalStatus.Team[gbPlayerNum].bLastID;

  // run through mercs on squad...if they can doctor, add to list
  for (iCounterA = 0; iCounterA < iNumberOnTeam; iCounterA++) {
    if (CanCharacterAutoBandageTeammate(GetSoldierByID(iCounterA))) {
      // add to list, up the count
      iDoctorList[iNumberDoctoring] = iCounterA;
      iNumberDoctoring++;
    }
  }

  // run through mercs on squad, if they can patient, add to list
  for (iCounterA = 0; iCounterA < iNumberOnTeam; iCounterA++) {
    if (CanCharacterBeAutoBandagedByTeammate(GetSoldierByID(iCounterA))) {
      // add to list, up the count
      iPatientList[iNumberPatienting] = iCounterA;
      iNumberPatienting++;
    }
  }

  // makes sure there is someone to doctor and patient...
  if ((iNumberDoctoring == 0) || (iNumberPatienting == 0)) {
    // reset the tables of merc ids
    memset(iDoctorList, -1, sizeof(int32_t) * MAX_CHARACTER_COUNT);
    memset(iPatientList, -1, sizeof(int32_t) * MAX_CHARACTER_COUNT);
  }

  // now add the faces
  AddFacesToAutoBandageBox();

  fAutoBandageComplete = FALSE;

  return;
}

void DisplayAutoBandageUpdatePanel(void) {
  int32_t iNumberDoctors = 0, iNumberPatients = 0;
  int32_t iNumberDoctorsHigh = 0, iNumberPatientsHigh = 0;
  int32_t iNumberDoctorsWide = 0, iNumberPatientsWide = 0;
  int32_t iTotalPixelsHigh = 0, iTotalPixelsWide = 0;
  int32_t iCurPixelY = 0;
  int16_t sXPosition = 0, sYPosition = 0;
  struct VObject *hBackGroundHandle;
  int32_t iCounterA = 0, iCounterB = 0;
  int32_t iIndex = 0;
  int16_t sCurrentXPosition = 0, sCurrentYPosition = 0;
  wchar_t sString[64];
  int16_t sX = 0, sY = 0;

  // are even in autobandage mode?
  if (gTacticalStatus.fAutoBandageMode == FALSE) {
    // nope,
    return;
  }

  // make sure there is someone to doctor and patient
  if ((iDoctorList[0] == -1) || (iPatientList[0] == -1)) {
    // nope, nobody here
    return;
  }

  // grab number of doctors
  for (iCounterA = 0; iDoctorList[iCounterA] != -1; iCounterA++) {
    iNumberDoctors++;
  }

  // grab number of patients
  for (iCounterA = 0; iPatientList[iCounterA] != -1; iCounterA++) {
    iNumberPatients++;
  }

  // build dimensions of box

  if (iNumberDoctors < NUMBER_MERC_FACES_AUTOBANDAGE_BOX) {
    // nope, get the base amount
    iNumberDoctorsWide = (iNumberDoctors % NUMBER_MERC_FACES_AUTOBANDAGE_BOX);
  } else {
    iNumberDoctorsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
  }

  // set the min number of mercs
  if (iNumberDoctorsWide < 3) {
    iNumberDoctorsWide = 2;
  } else {
    // a full row
    iNumberDoctorsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
  }

  // the doctors
  iNumberDoctorsHigh = (iNumberDoctors / (NUMBER_MERC_FACES_AUTOBANDAGE_BOX) + 1);

  if (iNumberDoctors % NUMBER_MERC_FACES_AUTOBANDAGE_BOX) {
    // now the patients
    iNumberDoctorsHigh = (iNumberDoctors / (NUMBER_MERC_FACES_AUTOBANDAGE_BOX) + 1);
  } else {
    // now the patients
    iNumberDoctorsHigh = (iNumberDoctors / (NUMBER_MERC_FACES_AUTOBANDAGE_BOX));
  }

  if (iNumberPatients < NUMBER_MERC_FACES_AUTOBANDAGE_BOX) {
    // nope, get the base amount
    iNumberPatientsWide = (iNumberPatients % NUMBER_MERC_FACES_AUTOBANDAGE_BOX);
  } else {
    iNumberPatientsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
  }

  // set the min number of mercs
  if (iNumberPatientsWide < 3) {
    iNumberPatientsWide = 2;
  } else {
    // a full row
    iNumberPatientsWide = NUMBER_MERC_FACES_AUTOBANDAGE_BOX;
  }

  if (iNumberPatients % NUMBER_MERC_FACES_AUTOBANDAGE_BOX) {
    // now the patients
    iNumberPatientsHigh = (iNumberPatients / (NUMBER_MERC_FACES_AUTOBANDAGE_BOX) + 1);
  } else {
    // now the patients
    iNumberPatientsHigh = (iNumberPatients / (NUMBER_MERC_FACES_AUTOBANDAGE_BOX));
  }

  // now the actual pixel dimensions

  iTotalPixelsHigh = (iNumberPatientsHigh + iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT;

  // see which is wider, and set to this
  if (iNumberDoctorsWide > iNumberPatientsWide) {
    iNumberPatientsWide = iNumberDoctorsWide;
  } else {
    iNumberDoctorsWide = iNumberPatientsWide;
  }

  iTotalPixelsWide = TACT_UPDATE_MERC_FACE_X_WIDTH * iNumberDoctorsWide;

  // now get the x and y position for the box
  sXPosition = (640 - iTotalPixelsWide) / 2;
  sYPosition = (INV_INTERFACE_START_Y - iTotalPixelsHigh) / 2;

  // now blit down the background
  GetVideoObject(&hBackGroundHandle, guiUpdatePanelTactical);

  // first the doctors on top
  for (iCounterA = 0; iCounterA < iNumberDoctorsHigh; iCounterA++) {
    for (iCounterB = 0; iCounterB < iNumberDoctorsWide; iCounterB++) {
      sCurrentXPosition = sXPosition + (iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH);
      sCurrentYPosition = sYPosition + (iCounterA * TACT_UPDATE_MERC_FACE_X_HEIGHT);

      // slap down background piece
      BltVideoObject(vsFB, hBackGroundHandle, 15, sCurrentXPosition, sCurrentYPosition);

      iIndex = iCounterA * iNumberDoctorsWide + iCounterB;

      if (iDoctorList[iIndex] != -1) {
        sCurrentXPosition += TACT_UPDATE_MERC_FACE_X_OFFSET;
        sCurrentYPosition += TACT_UPDATE_MERC_FACE_Y_OFFSET;

        // there is a face
        RenderSoldierSmallFaceForAutoBandagePanel(iIndex, sCurrentXPosition, sCurrentYPosition);

        // display the mercs name
        swprintf(sString, ARR_SIZE(sString), L"%s",
                 (Menptr[iDoctorList[iCounterA * iNumberDoctorsWide + iCounterB]]).name);
        FindFontCenterCoordinates((int16_t)(sCurrentXPosition), (int16_t)(sCurrentYPosition),
                                  (TACT_UPDATE_MERC_FACE_X_WIDTH - 25), 0, sString, TINYFONT1, &sX,
                                  &sY);
        SetFont(TINYFONT1);
        SetFontForeground(FONT_LTRED);
        SetFontBackground(FONT_BLACK);

        sY += 35;
        sCurrentXPosition -= TACT_UPDATE_MERC_FACE_X_OFFSET;
        sCurrentYPosition -= TACT_UPDATE_MERC_FACE_Y_OFFSET;

        // print name
        mprintf(sX, sY, sString);
        // sCurrentYPosition-= TACT_UPDATE_MERC_FACE_Y_OFFSET;
      }
    }
  }

  for (iCounterB = 0; iCounterB < iNumberPatientsWide; iCounterB++) {
    // slap down background piece
    BltVideoObject(vsFB, hBackGroundHandle, 16,
                   sXPosition + (iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH),
                   sCurrentYPosition + (TACT_UPDATE_MERC_FACE_X_HEIGHT));
    BltVideoObject(vsFB, hBackGroundHandle, 16,
                   sXPosition + (iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH), sYPosition - 9);
  }

  // bordering patient title
  BltVideoObject(vsFB, hBackGroundHandle, 11, sXPosition - 4,
                 sYPosition + ((iNumberDoctorsHigh)*TACT_UPDATE_MERC_FACE_X_HEIGHT));
  BltVideoObject(vsFB, hBackGroundHandle, 13, sXPosition + iTotalPixelsWide,
                 sYPosition + ((iNumberDoctorsHigh)*TACT_UPDATE_MERC_FACE_X_HEIGHT));

  SetFont(TINYFONT1);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  //	iCurPixelY = sYPosition;
  iCurPixelY = sYPosition + ((iCounterA - 1) * TACT_UPDATE_MERC_FACE_X_HEIGHT);

  swprintf(sString, ARR_SIZE(sString), L"%s", zMarksMapScreenText[13]);
  FindFontCenterCoordinates((int16_t)(sXPosition), (int16_t)(sCurrentYPosition),
                            (int16_t)(iTotalPixelsWide), 0, sString, TINYFONT1, &sX, &sY);
  // print medic
  mprintf(sX, sYPosition - 7, sString);

  // DisplayWrappedString( ( int16_t )( sXPosition ),  ( int16_t )( sCurrentYPosition - 40 ), (
  // int16_t )( iTotalPixelsWide ), 0, TINYFONT1, FONT_WHITE, pUpdateMercStrings[ 0 ], FONT_BLACK,
  // 0, 0 );

  sYPosition += 9;

  // now the patients
  for (iCounterA = 0; iCounterA < iNumberPatientsHigh; iCounterA++) {
    for (iCounterB = 0; iCounterB < iNumberPatientsWide; iCounterB++) {
      sCurrentXPosition = sXPosition + (iCounterB * TACT_UPDATE_MERC_FACE_X_WIDTH);
      sCurrentYPosition =
          sYPosition + ((iCounterA + iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT);

      // slap down background piece
      BltVideoObject(vsFB, hBackGroundHandle, 15, sCurrentXPosition, sCurrentYPosition);

      iIndex = iCounterA * iNumberPatientsWide + iCounterB;

      if (iPatientList[iIndex] != -1) {
        sCurrentXPosition += TACT_UPDATE_MERC_FACE_X_OFFSET;
        sCurrentYPosition += TACT_UPDATE_MERC_FACE_Y_OFFSET;

        // there is a face
        RenderSoldierSmallFaceForAutoBandagePanel(iIndex + iNumberDoctors, sCurrentXPosition,
                                                  sCurrentYPosition);

        // display the mercs name
        swprintf(sString, ARR_SIZE(sString), L"%s", (Menptr[iPatientList[iIndex]]).name);
        FindFontCenterCoordinates((int16_t)(sCurrentXPosition), (int16_t)(sCurrentYPosition),
                                  (TACT_UPDATE_MERC_FACE_X_WIDTH - 25), 0, sString, TINYFONT1, &sX,
                                  &sY);
        SetFont(TINYFONT1);
        SetFontForeground(FONT_LTRED);
        SetFontBackground(FONT_BLACK);
        sY += 35;

        // print name
        mprintf(sX, sY, sString);
      }
    }
  }

  // BORDER PIECES!!!!

  // bordering patients squares
  for (iCounterA = 0; iCounterA < iNumberPatientsHigh; iCounterA++) {
    BltVideoObject(
        vsFB, hBackGroundHandle, 3, sXPosition - 4,
        sYPosition + ((iCounterA + iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
    BltVideoObject(
        vsFB, hBackGroundHandle, 5, sXPosition + iTotalPixelsWide,
        sYPosition + ((iCounterA + iNumberDoctorsHigh) * TACT_UPDATE_MERC_FACE_X_HEIGHT));
  }

  // back up 11 pixels
  sYPosition -= 9;

  // pieces bordering doctor squares
  for (iCounterA = 0; iCounterA < iNumberDoctorsHigh; iCounterA++) {
    BltVideoObject(vsFB, hBackGroundHandle, 3, sXPosition - 4,
                   sYPosition + ((iCounterA)*TACT_UPDATE_MERC_FACE_X_HEIGHT));
    BltVideoObject(vsFB, hBackGroundHandle, 5, sXPosition + iTotalPixelsWide,
                   sYPosition + ((iCounterA)*TACT_UPDATE_MERC_FACE_X_HEIGHT));
  }

  // bordering doctor title
  BltVideoObject(vsFB, hBackGroundHandle, 11, sXPosition - 4, sYPosition - 9);
  BltVideoObject(vsFB, hBackGroundHandle, 13, sXPosition + iTotalPixelsWide, sYPosition - 9);

  // now the top pieces
  for (iCounterA = 0; iCounterA < iNumberPatientsWide; iCounterA++) {
    // the top bottom
    BltVideoObject(vsFB, hBackGroundHandle, 1,
                   sXPosition + TACT_UPDATE_MERC_FACE_X_WIDTH * (iCounterA), sYPosition - 13);
  }

  // the top corners
  BltVideoObject(vsFB, hBackGroundHandle, 0, sXPosition - 4, sYPosition - 13);
  BltVideoObject(vsFB, hBackGroundHandle, 2, sXPosition + iTotalPixelsWide, sYPosition - 13);

  iTotalPixelsHigh += 9;

  // the bottom
  BltVideoObject(vsFB, hBackGroundHandle, 17, sXPosition - 4, sYPosition + iTotalPixelsHigh);
  BltVideoObject(vsFB, hBackGroundHandle, 18,
                 sXPosition + iTotalPixelsWide - TACT_UPDATE_MERC_FACE_X_WIDTH,
                 sYPosition + iTotalPixelsHigh);

  if (iNumberPatientsWide == 2) {
    BltVideoObject(vsFB, hBackGroundHandle, 6, sXPosition - 4, sYPosition + iTotalPixelsHigh);
    CreateTerminateAutoBandageButton((int16_t)(sXPosition),
                                     (int16_t)(sYPosition + iTotalPixelsHigh + 3));
  } else {
    BltVideoObject(vsFB, hBackGroundHandle, 6, sXPosition + TACT_UPDATE_MERC_FACE_X_WIDTH - 4,
                   sYPosition + iTotalPixelsHigh);
    CreateTerminateAutoBandageButton((int16_t)(sXPosition + TACT_UPDATE_MERC_FACE_X_WIDTH),
                                     (int16_t)(sYPosition + iTotalPixelsHigh + 3));
  }

  SetFont(TINYFONT1);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);

  swprintf(sString, ARR_SIZE(sString), L"%s", zMarksMapScreenText[14]);
  FindFontCenterCoordinates((int16_t)(sXPosition), (int16_t)(sCurrentYPosition),
                            (int16_t)(iTotalPixelsWide), 0, sString, TINYFONT1, &sX, &sY);
  // print patient
  mprintf(sX, iCurPixelY + (TACT_UPDATE_MERC_FACE_X_HEIGHT) + 2, sString);

  MarkAButtonDirty(iEndAutoBandageButton[0]);
  MarkAButtonDirty(iEndAutoBandageButton[1]);

  DrawButton(iEndAutoBandageButton[0]);
  DrawButton(iEndAutoBandageButton[1]);

  iTotalPixelsHigh += 35;

  // if autobandage is complete, set the fact by enabling the done button
  if (fAutoBandageComplete == FALSE) {
    DisableButton(iEndAutoBandageButton[0]);
    EnableButton(iEndAutoBandageButton[1]);
  } else {
    DisableButton(iEndAutoBandageButton[1]);
    EnableButton(iEndAutoBandageButton[0]);
  }

  // now make sure it goes to the screen
  InvalidateRegion(sXPosition - 4, sYPosition - 18, (int16_t)(sXPosition + iTotalPixelsWide + 4),
                   (int16_t)(sYPosition + iTotalPixelsHigh));

  return;
}

void CreateTerminateAutoBandageButton(int16_t sX, int16_t sY) {
  // create the kill autobandage button
  if (fAutoEndBandageButtonCreated) {
    // button created, leave
    return;
  }

  fAutoEndBandageButtonCreated = TRUE;

  // the continue button

  // grab the image
  iEndAutoBandageButtonImage[0] =
      LoadButtonImage("INTERFACE\\group_confirm_tactical.sti", -1, 7, -1, 8, -1);

  // grab the button
  iEndAutoBandageButton[0] = QuickCreateButton(
      iEndAutoBandageButtonImage[0], sX, sY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
      (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)StopAutoBandageButtonCallback);

  // the cancel button
  // grab the image
  iEndAutoBandageButtonImage[1] =
      LoadButtonImage("INTERFACE\\group_confirm_tactical.sti", -1, 7, -1, 8, -1);

  // grab the button
  iEndAutoBandageButton[1] =
      QuickCreateButton(iEndAutoBandageButtonImage[1], (int16_t)(sX + 70), sY, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)StopAutoBandageButtonCallback);

  SpecifyButtonText(iEndAutoBandageButton[0], zMarksMapScreenText[15]);
  SpecifyButtonFont(iEndAutoBandageButton[0], MAP_SCREEN_FONT);
  SpecifyButtonUpTextColors(iEndAutoBandageButton[0], FONT_MCOLOR_BLACK, FONT_BLACK);
  SpecifyButtonDownTextColors(iEndAutoBandageButton[0], FONT_MCOLOR_BLACK, FONT_BLACK);

  SpecifyButtonText(iEndAutoBandageButton[1], zMarksMapScreenText[16]);
  SpecifyButtonFont(iEndAutoBandageButton[1], MAP_SCREEN_FONT);
  SpecifyButtonUpTextColors(iEndAutoBandageButton[1], FONT_MCOLOR_BLACK, FONT_BLACK);
  SpecifyButtonDownTextColors(iEndAutoBandageButton[1], FONT_MCOLOR_BLACK, FONT_BLACK);

  return;
}

void StopAutoBandageButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      fEndAutoBandage = TRUE;
    }
  }

  return;
}

void DestroyTerminateAutoBandageButton(void) {
  // destroy the kill autobandage button
  if (fAutoEndBandageButtonCreated == FALSE) {
    // not around, don't destroy what ain't there
    return;
  }

  fAutoEndBandageButtonCreated = FALSE;

  // remove button
  RemoveButton(iEndAutoBandageButton[0]);
  RemoveButton(iEndAutoBandageButton[1]);

  // unload image
  UnloadButtonImage(iEndAutoBandageButtonImage[0]);
  UnloadButtonImage(iEndAutoBandageButtonImage[1]);

  return;
}

BOOLEAN AddFacesToAutoBandageBox(void) {
  int32_t iCounter = 0;
  int32_t iNumberOfDoctors = 0;

  // reset
  memset(&giAutoBandagesSoldierFaces, -1, sizeof(giAutoBandagesSoldierFaces));

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // find a free slot
    if (iDoctorList[iCounter] != -1) {
      SGPFILENAME ImageFile;
      if (gMercProfiles[(Menptr[iDoctorList[iCounter]]).ubProfile].ubFaceIndex < 100) {
        // grab filename of face
        sprintf(ImageFile, "Faces\\65Face\\%02d.sti",
                gMercProfiles[(Menptr[iDoctorList[iCounter]]).ubProfile].ubFaceIndex);
      } else {
        // grab filename of face
        sprintf(ImageFile, "Faces\\65Face\\%03d.sti",
                gMercProfiles[(Menptr[iDoctorList[iCounter]]).ubProfile].ubFaceIndex);
      }

      // load the face
      AddVObject(CreateVObjectFromFile(ImageFile), &giAutoBandagesSoldierFaces[iCounter]);
      iNumberOfDoctors++;
    }
  }

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // find a free slot
    if (iPatientList[iCounter] != -1) {
      SGPFILENAME ImageFile;
      if (gMercProfiles[(Menptr[iPatientList[iCounter]]).ubProfile].ubFaceIndex < 100) {
        // grab filename of face
        sprintf(ImageFile, "Faces\\65Face\\%02d.sti",
                gMercProfiles[(Menptr[iPatientList[iCounter]]).ubProfile].ubFaceIndex);
      } else {
        // grab filename of face
        sprintf(ImageFile, "Faces\\65Face\\%03d.sti",
                gMercProfiles[(Menptr[iPatientList[iCounter]]).ubProfile].ubFaceIndex);
      }

      // load the face
      AddVObject(CreateVObjectFromFile(ImageFile),
                 &giAutoBandagesSoldierFaces[iCounter + iNumberOfDoctors]);
    }
  }

  // grab panels
  if (!AddVObject(CreateVObjectFromFile("Interface\\panels.sti"), &giMercPanelImage)) {
    AssertMsg(0, "Failed to load Interface\\panels.sti");
  }

  return (TRUE);
}

BOOLEAN RemoveFacesForAutoBandage(void) {
  int32_t iCounter = 0, iNumberOfDoctors = 0;

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // find a free slot
    if (iDoctorList[iCounter] != -1) {
      // load the face
      DeleteVideoObjectFromIndex(giAutoBandagesSoldierFaces[iCounter]);
      iNumberOfDoctors++;
    }
  }

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // find a free slot
    if (iPatientList[iCounter] != -1) {
      // load the face
      DeleteVideoObjectFromIndex(giAutoBandagesSoldierFaces[iCounter + iNumberOfDoctors]);
    }
  }

  DeleteVideoObjectFromIndex(giMercPanelImage);

  return (TRUE);
}

BOOLEAN RenderSoldierSmallFaceForAutoBandagePanel(int32_t iIndex, int16_t sCurrentXPosition,
                                                  int16_t sCurrentYPosition) {
  int32_t iStartY = 0;
  struct SOLDIERTYPE *pSoldier = NULL;
  int32_t iCounter = 0, iIndexCount = 0;
  struct VObject *hHandle;

  // grab the video object
  GetVideoObject(&hHandle, giAutoBandagesSoldierFaces[iIndex]);

  // fill the background for the info bars black
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 36, sCurrentYPosition + 2, sCurrentXPosition + 44,
                        sCurrentYPosition + 30, 0);

  // put down the background
  BltVObjectFromIndex(vsFB, giMercPanelImage, 0, sCurrentXPosition, sCurrentYPosition);

  // grab the face
  BltVideoObject(vsFB, hHandle, 0, sCurrentXPosition + 2, sCurrentYPosition + 2);

  for (iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++) {
    // find a free slot
    if (iDoctorList[iCounter] != -1) {
      iIndexCount++;
    }
  }

  // see if we are looking into doctor or patient lists?
  if (iIndexCount > iIndex) {
    // HEALTH BAR
    pSoldier = GetSoldierByID(iDoctorList[iIndex]);
  } else {
    // HEALTH BAR
    pSoldier = &Menptr[iPatientList[iIndex - iIndexCount]];
  }

  // is the merc alive?
  if (!pSoldier->bLife) return (FALSE);

  // yellow one for bleeding
  iStartY = sCurrentYPosition + 29 - 27 * pSoldier->bLifeMax / 100;
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 36, iStartY, sCurrentXPosition + 37,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(107, 107, 57)));
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 37, iStartY, sCurrentXPosition + 38,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(222, 181, 115)));

  // pink one for bandaged.
  iStartY += 27 * pSoldier->bBleeding / 100;
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 36, iStartY, sCurrentXPosition + 37,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(156, 57, 57)));
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 37, iStartY, sCurrentXPosition + 38,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(222, 132, 132)));

  // red one for actual health
  iStartY = sCurrentYPosition + 29 - 27 * pSoldier->bLife / 100;
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 36, iStartY, sCurrentXPosition + 37,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(107, 8, 8)));
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 37, iStartY, sCurrentXPosition + 38,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(206, 0, 0)));

  // BREATH BAR
  iStartY = sCurrentYPosition + 29 - 27 * pSoldier->bBreathMax / 100;
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 39, iStartY, sCurrentXPosition + 40,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(8, 8, 132)));
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 40, iStartY, sCurrentXPosition + 41,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(8, 8, 107)));

  // MORALE BAR
  iStartY = sCurrentYPosition + 29 - 27 * pSoldier->bMorale / 100;
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 42, iStartY, sCurrentXPosition + 43,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(8, 156, 8)));
  ColorFillVSurfaceArea(vsFB, sCurrentXPosition + 43, iStartY, sCurrentXPosition + 44,
                        sCurrentYPosition + 29, rgb32_to_rgb16(FROMRGB(8, 107, 8)));

  return (TRUE);
}
