// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/AIMArchives.h"

#include <string.h>

#include "Laptop/AIM.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SysGlobals.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

#define AIM_ALUMNI_NAME_FILE "BINARYDATA\\AlumName.edt"
#define AIM_ALUMNI_FILE "BINARYDATA\\Alumni.edt"

#define AIM_ALUMNI_TITLE_FONT FONT14ARIAL
#define AIM_ALUMNI_TITLE_COLOR AIM_GREEN

#define AIM_ALUMNI_POPUP_FONT FONT10ARIAL
#define AIM_ALUMNI_POPUP_COLOR FONT_MCOLOR_WHITE

#define AIM_ALUMNI_POPUP_NAME_FONT FONT12ARIAL
#define AIM_ALUMNI_POPUP_NAME_COLOR FONT_MCOLOR_WHITE

#define AIM_ALUMNI_NAME_FONT FONT12ARIAL
#define AIM_ALUMNI_NAME_COLOR FONT_MCOLOR_WHITE
#define AIM_ALUMNI_PAGE_FONT FONT14ARIAL
#define AIM_ALUMNI_PAGE_COLOR_UP FONT_MCOLOR_DKWHITE
#define AIM_ALUMNI_PAGE_COLOR_DOWN 138

#define AIM_ALUMNI_NAME_LINESIZE 80 * 2
#define AIM_ALUMNI_ALUMNI_LINESIZE 7 * 80 * 2

#define AIM_ALUMNI_NUM_FACE_COLS 5
#define AIM_ALUMNI_NUM_FACE_ROWS 4
#define MAX_NUMBER_OLD_MERCS_ON_PAGE AIM_ALUMNI_NUM_FACE_ROWS *AIM_ALUMNI_NUM_FACE_COLS

#define AIM_ALUMNI_START_GRID_X LAPTOP_SCREEN_UL_X + 37
#define AIM_ALUMNI_START_GRID_Y LAPTOP_SCREEN_WEB_UL_Y + 68

#define AIM_ALUMNI_GRID_OFFSET_X 90
#define AIM_ALUMNI_GRID_OFFSET_Y 72

#define AIM_ALUMNI_ALUMNI_FRAME_WIDTH 66
#define AIM_ALUMNI_ALUMNI_FRAME_HEIGHT 64

#define AIM_ALUMNI_ALUMNI_FACE_WIDTH 56
#define AIM_ALUMNI_ALUMNI_FACE_HEIGHT 50

#define AIM_ALUMNI_NAME_OFFSET_X 5
#define AIM_ALUMNI_NAME_OFFSET_Y 55
#define AIM_ALUMNI_NAME_WIDTH AIM_ALUMNI_ALUMNI_FRAME_WIDTH - AIM_ALUMNI_NAME_OFFSET_X * 2

#define AIM_ALUMNI_PAGE1_X LAPTOP_SCREEN_UL_X + 100
#define AIM_ALUMNI_PAGE1_Y LAPTOP_SCREEN_WEB_UL_Y + 357
#define AIM_ALUMNI_PAGE_GAP BOTTOM_BUTTON_START_WIDTH + 25

#define AIM_ALUMNI_PAGE_END_X \
  AIM_ALUMNI_PAGE1_X + (BOTTOM_BUTTON_START_WIDTH + BOTTOM_BUTTON_START_WIDTH) * 3
#define AIM_ALUMNI_PAGE_END_Y AIM_ALUMNI_PAGE1_Y + BOTTOM_BUTTON_START_HEIGHT

#define AIM_ALUMNI_TITLE_X IMAGE_OFFSET_X + 149
#define AIM_ALUMNI_TITLE_Y AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y  // + 2
#define AIM_ALUMNI_TITLE_WIDTH AIM_SYMBOL_WIDTH

#define AIM_POPUP_WIDTH 309
#define AIM_POPUP_TEXT_WIDTH 296
#define AIM_POPUP_SECTION_HEIGHT 9

#define AIM_POPUP_X LAPTOP_SCREEN_UL_X + (500 - AIM_POPUP_WIDTH) / 2
#define AIM_POPUP_Y 120 + LAPTOP_SCREEN_WEB_DELTA_Y

#define AIM_POPUP_SHADOW_GAP 4

#define AIM_POPUP_TEXT_X AIM_POPUP_X

#define AIM_ALUMNI_FACE_PANEL_X AIM_POPUP_X + 6
#define AIM_ALUMNI_FACE_PANEL_Y AIM_POPUP_Y + 6
#define AIM_ALUMNI_FACE_PANEL_WIDTH 58
#define AIM_ALUMNI_FACE_PANEL_HEIGHT 52

#define AIM_ALUMNI_POPUP_NAME_X AIM_ALUMNI_FACE_PANEL_X + AIM_ALUMNI_FACE_PANEL_WIDTH + 10
#define AIM_ALUMNI_POPUP_NAME_Y AIM_ALUMNI_FACE_PANEL_Y + 20

#define AIM_ALUMNI_POPUP_DESC_X AIM_POPUP_X + 8
#define AIM_ALUMNI_POPUP_DESC_Y AIM_ALUMNI_FACE_PANEL_Y + AIM_ALUMNI_FACE_PANEL_HEIGHT + 5

#define AIM_ALUMNI_DONE_X AIM_POPUP_X + AIM_POPUP_WIDTH - AIM_ALUMNI_DONE_WIDTH - 7
#define AIM_ALUMNI_DONE_WIDTH 36
#define AIM_ALUMNI_DONE_HEIGHT 16

#define AIM_ALUMNI_NAME_SIZE 80 * 2
#define AIM_ALUMNI_DECRIPTION_SIZE 80 * 7 * 2
#define AIM_ALUMNI_FILE_RECORD_SIZE 80 * 8 * 2
// #define		AIM_ALUMNI_FILE_RECORD_SIZE			80 * 7 * 2
#define AIM_ALUMNI_FULL_NAME_SIZE 80 * 2

uint32_t guiAlumniFrame;
uint32_t guiOldAim;
uint32_t guiPageButtons;
uint32_t guiAlumniPopUp;
uint32_t guiPopUpPic;

uint8_t gubPageNum;
uint8_t gunAlumniButtonDown = 255;
BOOLEAN gfExitingAimArchives;
uint8_t gubDrawOldMerc;
uint8_t gfDrawPopUpBox = FALSE;
BOOLEAN gfDestroyPopUpBox;
BOOLEAN gfFaceMouseRegionsActive;
// BOOLEAN		gfDestroyDoneRegion;
BOOLEAN gfReDrawScreen = FALSE;

BOOLEAN AimArchivesSubPagesVisitedFlag[3] = {0, 0, 0};

// Mouse Regions

// Face regions
struct MOUSE_REGION gMercAlumniFaceMouseRegions[MAX_NUMBER_OLD_MERCS_ON_PAGE];
void SelectAlumniFaceRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// Done region
struct MOUSE_REGION gDoneRegion;
void SelectAlumniDoneRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// Previous Button
void BtnAlumniPageButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiAlumniPageButton[3];
int32_t guiAlumniPageButtonImage;

void ResetAimArchiveButtons();
void DisableAimArchiveButton();
void DisplayAlumniOldMercPopUp();
void DestroyPopUpBox();
void InitAlumniFaceRegions();
void RemoveAimAlumniFaceRegion();
void CreateDestroyDoneMouseRegion(uint16_t usPosY);
void ChangingAimArchiveSubPage(uint8_t ubSubPageNumber);

void GameInitAimArchives() {}

void EnterInitAimArchives() {
  gfDrawPopUpBox = FALSE;
  gfDestroyPopUpBox = FALSE;

  memset(&AimArchivesSubPagesVisitedFlag, 0, 3);
  AimArchivesSubPagesVisitedFlag[0] = TRUE;
}

BOOLEAN EnterAimArchives() {
  uint16_t usPosX, i;

  gfExitingAimArchives = FALSE;
  //	gubDrawOldMerc = 255;
  gfDrawPopUpBox = FALSE;
  gfDestroyPopUpBox = FALSE;

  InitAimDefaults();
  InitAimMenuBar();

  gubPageNum = (uint8_t)giCurrentSubPage;

  // load the Alumni Frame and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\AlumniFrame.sti"), &guiAlumniFrame));

  // load the 1st set of faces and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Old_Aim.sti"), &guiOldAim));

  // load the Bottom Buttons graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BottomButton.sti"), &guiPageButtons));

  // load the PopupPic graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\PopupPicFrame.sti"), &guiPopUpPic));

  // load the AlumniPopUp graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\AlumniPopUp.sti"), &guiAlumniPopUp));

  // load the Done Button graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\DoneButton.sti"), &guiDoneButton));

  InitAlumniFaceRegions();

  // Load graphic for buttons
  guiAlumniPageButtonImage = LoadButtonImage("LAPTOP\\BottomButtons2.sti", -1, 0, -1, 1, -1);

  usPosX = AIM_ALUMNI_PAGE1_X;
  for (i = 0; i < 3; i++) {
    guiAlumniPageButton[i] = CreateIconAndTextButton(
        guiAlumniPageButtonImage, AimAlumniText[i], AIM_ALUMNI_PAGE_FONT, AIM_ALUMNI_PAGE_COLOR_UP,
        DEFAULT_SHADOW, AIM_ALUMNI_PAGE_COLOR_DOWN, DEFAULT_SHADOW, TEXT_CJUSTIFIED, usPosX,
        AIM_ALUMNI_PAGE1_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
        BtnAlumniPageButtonCallback);
    SetButtonCursor(guiAlumniPageButton[i], CURSOR_WWW);
    MSYS_SetBtnUserData(guiAlumniPageButton[i], 0, i);

    usPosX += AIM_ALUMNI_PAGE_GAP;
  }

  DisableAimArchiveButton();
  RenderAimArchives();
  return (TRUE);
}

void ExitAimArchives() {
  uint16_t i;

  gfExitingAimArchives = TRUE;

  DeleteVideoObjectFromIndex(guiAlumniFrame);
  DeleteVideoObjectFromIndex(guiOldAim);
  DeleteVideoObjectFromIndex(guiAlumniPopUp);
  DeleteVideoObjectFromIndex(guiPopUpPic);
  DeleteVideoObjectFromIndex(guiDoneButton);

  RemoveAimAlumniFaceRegion();

  UnloadButtonImage(guiAlumniPageButtonImage);
  for (i = 0; i < 3; i++) RemoveButton(guiAlumniPageButton[i]);

  RemoveAimDefaults();
  ExitAimMenuBar();
  giCurrentSubPage = gubPageNum;

  CreateDestroyDoneMouseRegion(0);
  gfDestroyPopUpBox = FALSE;
  gfDrawPopUpBox = FALSE;
}

void HandleAimArchives() {
  if (gfReDrawScreen) {
    //		RenderAimArchives();
    fPausedReDrawScreenFlag = TRUE;

    gfReDrawScreen = FALSE;
  }
  if (gfDestroyPopUpBox) {
    gfDestroyPopUpBox = FALSE;

    CreateDestroyDoneMouseRegion(0);
    InitAlumniFaceRegions();
    gfDestroyPopUpBox = FALSE;
  }
}

void RenderAimArchives() {
  struct VObject *hFrameHandle;
  struct VObject *hFaceHandle;
  //  struct VObject*	hBottomButtonHandle;
  uint16_t usPosX, usPosY, x, y, i = 0;
  uint8_t ubNumRows = 0;
  uint32_t uiStartLoc = 0;
  wchar_t sText[400];

  DrawAimDefaults();
  DisableAimButton();

  // Draw Link Title
  DrawTextToScreen(AimAlumniText[AIM_ALUMNI_ALUMNI], AIM_ALUMNI_TITLE_X, AIM_ALUMNI_TITLE_Y,
                   AIM_ALUMNI_TITLE_WIDTH, AIM_ALUMNI_TITLE_FONT, AIM_ALUMNI_TITLE_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Draw the mug shot border and face
  GetVideoObject(&hFrameHandle, guiAlumniFrame);
  GetVideoObject(&hFaceHandle, guiOldAim);

  switch (gubPageNum) {
    case 0:
      ubNumRows = AIM_ALUMNI_NUM_FACE_ROWS;
      i = 0;
      break;
    case 1:
      ubNumRows = AIM_ALUMNI_NUM_FACE_ROWS;
      i = 20;
      break;
    case 2:
      ubNumRows = 2;
      i = 40;
      break;
    default:
      Assert(0);
      break;
  }

  usPosX = AIM_ALUMNI_START_GRID_X;
  usPosY = AIM_ALUMNI_START_GRID_Y;
  for (y = 0; y < ubNumRows; y++) {
    for (x = 0; x < AIM_ALUMNI_NUM_FACE_COLS; x++) {
      // Blt face to screen
      BltVideoObject(vsFB, hFaceHandle, i, usPosX + 4, usPosY + 4);

      // Blt the alumni frame background
      BltVideoObject(vsFB, hFrameHandle, 0, usPosX, usPosY);

      // Display the merc's name
      uiStartLoc = AIM_ALUMNI_NAME_LINESIZE * i;
      LoadEncryptedDataFromFile(AIM_ALUMNI_NAME_FILE, sText, uiStartLoc, AIM_ALUMNI_NAME_SIZE);
      DrawTextToScreen(sText, (uint16_t)(usPosX + AIM_ALUMNI_NAME_OFFSET_X),
                       (uint16_t)(usPosY + AIM_ALUMNI_NAME_OFFSET_Y), AIM_ALUMNI_NAME_WIDTH,
                       AIM_ALUMNI_NAME_FONT, AIM_ALUMNI_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

      usPosX += AIM_ALUMNI_GRID_OFFSET_X;
      i++;
    }
    usPosX = AIM_ALUMNI_START_GRID_X;
    usPosY += AIM_ALUMNI_GRID_OFFSET_Y;
  }

  // the 3rd page now has an additional row with 1 merc on it, so add a new row
  if (gubPageNum == 2) {
    // Blt face to screen
    BltVideoObject(vsFB, hFaceHandle, i, usPosX + 4, usPosY + 4);

    // Blt the alumni frame background
    BltVideoObject(vsFB, hFrameHandle, 0, usPosX, usPosY);

    // Display the merc's name
    uiStartLoc = AIM_ALUMNI_NAME_LINESIZE * i;
    LoadEncryptedDataFromFile(AIM_ALUMNI_NAME_FILE, sText, uiStartLoc, AIM_ALUMNI_NAME_SIZE);
    DrawTextToScreen(sText, (uint16_t)(usPosX + AIM_ALUMNI_NAME_OFFSET_X),
                     (uint16_t)(usPosY + AIM_ALUMNI_NAME_OFFSET_Y), AIM_ALUMNI_NAME_WIDTH,
                     AIM_ALUMNI_NAME_FONT, AIM_ALUMNI_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE,
                     CENTER_JUSTIFIED);

    usPosX += AIM_ALUMNI_GRID_OFFSET_X;
  }

  //	GetVideoObject(&hBottomButtonHandle, guiPageButtons);
  usPosX = AIM_ALUMNI_PAGE1_X;

  if (gfDrawPopUpBox) {
    DisplayAlumniOldMercPopUp();
    RemoveAimAlumniFaceRegion();
  }

  MarkButtonsDirty();

  RenderWWWProgramTitleBar();

  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void SelectAlumniFaceRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gfDrawPopUpBox = TRUE;
    gfReDrawScreen = TRUE;

    gubDrawOldMerc = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void BtnAlumniPageButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  uint8_t ubRetValue = (uint8_t)MSYS_GetBtnUserData(btn, 0);
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;

    gunAlumniButtonDown = ubRetValue;

    InvalidateRegion(AIM_ALUMNI_PAGE1_X, AIM_ALUMNI_PAGE1_Y, AIM_ALUMNI_PAGE_END_X,
                     AIM_ALUMNI_PAGE_END_Y);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      RemoveAimAlumniFaceRegion();

      ChangingAimArchiveSubPage(ubRetValue);

      gubPageNum = ubRetValue;

      gfReDrawScreen = TRUE;

      gfDestroyPopUpBox = TRUE;

      gunAlumniButtonDown = 255;
      ResetAimArchiveButtons();
      DisableAimArchiveButton();
      gfDrawPopUpBox = FALSE;

      InvalidateRegion(AIM_ALUMNI_PAGE1_X, AIM_ALUMNI_PAGE1_Y, AIM_ALUMNI_PAGE_END_X,
                       AIM_ALUMNI_PAGE_END_Y);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    gunAlumniButtonDown = 255;
    DisableAimArchiveButton();
    InvalidateRegion(AIM_ALUMNI_PAGE1_X, AIM_ALUMNI_PAGE1_Y, AIM_ALUMNI_PAGE_END_X,
                     AIM_ALUMNI_PAGE_END_Y);
  }
}

void ResetAimArchiveButtons() {
  int i = 0;

  for (i = 0; i < 3; i++) {
    ButtonList[guiAlumniPageButton[i]]->uiFlags &= ~BUTTON_CLICKED_ON;
  }
}

void DisableAimArchiveButton() {
  if (gfExitingAimArchives == TRUE) return;

  if ((gubPageNum == 0)) {
    ButtonList[guiAlumniPageButton[0]]->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (gubPageNum == 1) {
    ButtonList[guiAlumniPageButton[1]]->uiFlags |= (BUTTON_CLICKED_ON);
  } else if (gubPageNum == 2) {
    ButtonList[guiAlumniPageButton[2]]->uiFlags |= (BUTTON_CLICKED_ON);
  }
}

void DisplayAlumniOldMercPopUp() {
  uint8_t i, ubNumLines = 11;  // 17
  uint16_t usPosY;
  uint8_t ubNumDescLines;
  struct VObject *hAlumniPopUpHandle;
  struct VObject *hDoneHandle;
  struct VObject *hFacePaneHandle;
  struct VObject *hFaceHandle;
  //	WRAPPED_STRING *pFirstWrappedString, *pTempWrappedString;
  wchar_t sName[AIM_ALUMNI_NAME_SIZE];
  wchar_t sDesc[AIM_ALUMNI_DECRIPTION_SIZE];
  uint32_t uiStartLoc;
  uint16_t usStringPixLength;

  GetVideoObject(&hAlumniPopUpHandle, guiAlumniPopUp);
  GetVideoObject(&hDoneHandle, guiDoneButton);
  GetVideoObject(&hFacePaneHandle, guiPopUpPic);
  GetVideoObject(&hFaceHandle, guiOldAim);

  // Load the description
  uiStartLoc = AIM_ALUMNI_FILE_RECORD_SIZE * gubDrawOldMerc + AIM_ALUMNI_FULL_NAME_SIZE;
  LoadEncryptedDataFromFile(AIM_ALUMNI_FILE, sDesc, uiStartLoc, AIM_ALUMNI_DECRIPTION_SIZE);

  usStringPixLength = StringPixLength(sDesc, AIM_ALUMNI_POPUP_FONT);
  ubNumDescLines = (uint8_t)(usStringPixLength / AIM_POPUP_TEXT_WIDTH);

  ubNumLines += ubNumDescLines;

  usPosY = AIM_POPUP_Y;

  // draw top line of the popup background
  ShadowVideoSurfaceRect(vsFB, AIM_POPUP_X + AIM_POPUP_SHADOW_GAP, usPosY + AIM_POPUP_SHADOW_GAP,
                         AIM_POPUP_X + AIM_POPUP_WIDTH + AIM_POPUP_SHADOW_GAP,
                         usPosY + AIM_POPUP_SECTION_HEIGHT + AIM_POPUP_SHADOW_GAP - 1);
  BltVideoObject(vsFB, hAlumniPopUpHandle, 0, AIM_POPUP_X, usPosY);
  // draw mid section of the popup background
  usPosY += AIM_POPUP_SECTION_HEIGHT;
  for (i = 0; i < ubNumLines; i++) {
    ShadowVideoSurfaceRect(vsFB, AIM_POPUP_X + AIM_POPUP_SHADOW_GAP, usPosY + AIM_POPUP_SHADOW_GAP,
                           AIM_POPUP_X + AIM_POPUP_WIDTH + AIM_POPUP_SHADOW_GAP,
                           usPosY + AIM_POPUP_SECTION_HEIGHT + AIM_POPUP_SHADOW_GAP - 1);
    BltVideoObject(vsFB, hAlumniPopUpHandle, 1, AIM_POPUP_X, usPosY);
    usPosY += AIM_POPUP_SECTION_HEIGHT;
  }
  // draw the bottom line and done button
  ShadowVideoSurfaceRect(vsFB, AIM_POPUP_X + AIM_POPUP_SHADOW_GAP, usPosY + AIM_POPUP_SHADOW_GAP,
                         AIM_POPUP_X + AIM_POPUP_WIDTH + AIM_POPUP_SHADOW_GAP,
                         usPosY + AIM_POPUP_SECTION_HEIGHT + AIM_POPUP_SHADOW_GAP - 1);
  BltVideoObject(vsFB, hAlumniPopUpHandle, 2, AIM_POPUP_X, usPosY);
  BltVideoObject(vsFB, hDoneHandle, 0, AIM_ALUMNI_DONE_X, usPosY - AIM_ALUMNI_DONE_HEIGHT);
  DrawTextToScreen(AimAlumniText[AIM_ALUMNI_DONE], (uint16_t)(AIM_ALUMNI_DONE_X + 1),
                   (uint16_t)(usPosY - AIM_ALUMNI_DONE_HEIGHT + 3), AIM_ALUMNI_DONE_WIDTH,
                   AIM_ALUMNI_POPUP_NAME_FONT, AIM_ALUMNI_POPUP_NAME_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, CENTER_JUSTIFIED);

  CreateDestroyDoneMouseRegion(usPosY);

  /// blt face panale and the mecs fce
  BltVideoObject(vsFB, hFacePaneHandle, 0, AIM_ALUMNI_FACE_PANEL_X, AIM_ALUMNI_FACE_PANEL_Y);
  BltVideoObject(vsFB, hFaceHandle, gubDrawOldMerc, AIM_ALUMNI_FACE_PANEL_X + 1,
                 AIM_ALUMNI_FACE_PANEL_Y + 1);

  // Load and display the name
  //	uiStartLoc = AIM_ALUMNI_NAME_SIZE * gubDrawOldMerc;
  //	LoadEncryptedDataFromFile(AIM_ALUMNI_NAME_FILE, sName, uiStartLoc, AIM_ALUMNI_NAME_SIZE);
  uiStartLoc = AIM_ALUMNI_FILE_RECORD_SIZE * gubDrawOldMerc;
  LoadEncryptedDataFromFile(AIM_ALUMNI_FILE, sName, uiStartLoc, AIM_ALUMNI_FULL_NAME_SIZE);

  DrawTextToScreen(sName, AIM_ALUMNI_POPUP_NAME_X, AIM_ALUMNI_POPUP_NAME_Y, 0,
                   AIM_ALUMNI_POPUP_NAME_FONT, AIM_ALUMNI_POPUP_NAME_COLOR, FONT_MCOLOR_BLACK,
                   FALSE, LEFT_JUSTIFIED);

  // Display the description
  DisplayWrappedString(AIM_ALUMNI_POPUP_DESC_X, AIM_ALUMNI_POPUP_DESC_Y, AIM_POPUP_TEXT_WIDTH, 2,
                       AIM_ALUMNI_POPUP_FONT, AIM_ALUMNI_POPUP_COLOR, sDesc, FONT_MCOLOR_BLACK,
                       FALSE, LEFT_JUSTIFIED);

  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

void DestroyPopUpBox() {
  gfDestroyPopUpBox = FALSE;
  RenderAimArchives();
}

void InitAlumniFaceRegions() {
  uint16_t usPosX, usPosY, i, x, y, usNumRows;

  if (gfFaceMouseRegionsActive) return;

  if (gubPageNum == 2)
    usNumRows = 2;
  else
    usNumRows = AIM_ALUMNI_NUM_FACE_ROWS;

  usPosX = AIM_ALUMNI_START_GRID_X;
  usPosY = AIM_ALUMNI_START_GRID_Y;
  i = 0;
  for (y = 0; y < usNumRows; y++) {
    for (x = 0; x < AIM_ALUMNI_NUM_FACE_COLS; x++) {
      MSYS_DefineRegion(&gMercAlumniFaceMouseRegions[i], usPosX, usPosY,
                        (int16_t)(usPosX + AIM_ALUMNI_ALUMNI_FACE_WIDTH),
                        (int16_t)(usPosY + AIM_ALUMNI_ALUMNI_FACE_HEIGHT), MSYS_PRIORITY_HIGH,
                        CURSOR_WWW, MSYS_NO_CALLBACK, SelectAlumniFaceRegionCallBack);
      // Add region
      MSYS_AddRegion(&gMercAlumniFaceMouseRegions[i]);
      MSYS_SetRegionUserData(&gMercAlumniFaceMouseRegions[i], 0, i + (20 * gubPageNum));

      usPosX += AIM_ALUMNI_GRID_OFFSET_X;
      i++;
    }
    usPosX = AIM_ALUMNI_START_GRID_X;
    usPosY += AIM_ALUMNI_GRID_OFFSET_Y;
  }

  // the 3rd page now has an additional row with 1 merc on it, so add a new row
  if (gubPageNum == 2) {
    MSYS_DefineRegion(&gMercAlumniFaceMouseRegions[i], usPosX, usPosY,
                      (int16_t)(usPosX + AIM_ALUMNI_ALUMNI_FACE_WIDTH),
                      (int16_t)(usPosY + AIM_ALUMNI_ALUMNI_FACE_HEIGHT), MSYS_PRIORITY_HIGH,
                      CURSOR_WWW, MSYS_NO_CALLBACK, SelectAlumniFaceRegionCallBack);
    // Add region
    MSYS_AddRegion(&gMercAlumniFaceMouseRegions[i]);
    MSYS_SetRegionUserData(&gMercAlumniFaceMouseRegions[i], 0, i + (20 * gubPageNum));
  }

  gfFaceMouseRegionsActive = TRUE;
}

void RemoveAimAlumniFaceRegion() {
  uint16_t i;
  uint16_t usNumber = 0;

  if (!gfFaceMouseRegionsActive) return;

  switch (gubPageNum) {
    case 0:
      usNumber = AIM_ALUMNI_NUM_FACE_ROWS * AIM_ALUMNI_NUM_FACE_COLS;
      break;
    case 1:
      usNumber = AIM_ALUMNI_NUM_FACE_ROWS * AIM_ALUMNI_NUM_FACE_COLS;
      break;
    case 2:
      usNumber = 2 * AIM_ALUMNI_NUM_FACE_COLS + 1;

    default:
      break;
  }

  for (i = 0; i < usNumber; i++) {
    MSYS_RemoveRegion(&gMercAlumniFaceMouseRegions[i]);
  }
  gfFaceMouseRegionsActive = FALSE;
}

void CreateDestroyDoneMouseRegion(uint16_t usPosY) {
  static BOOLEAN DoneRegionCreated = FALSE;

  if ((!DoneRegionCreated) && (usPosY != 0)) {
    usPosY -= AIM_ALUMNI_DONE_HEIGHT;
    MSYS_DefineRegion(&gDoneRegion, AIM_ALUMNI_DONE_X - 2, usPosY,
                      (AIM_ALUMNI_DONE_X - 2 + AIM_ALUMNI_DONE_WIDTH),
                      (int16_t)(usPosY + AIM_ALUMNI_DONE_HEIGHT), MSYS_PRIORITY_HIGH, CURSOR_WWW,
                      MSYS_NO_CALLBACK, SelectAlumniDoneRegionCallBack);
    // Add region
    MSYS_AddRegion(&gDoneRegion);
    DoneRegionCreated = TRUE;
  }

  if (DoneRegionCreated && usPosY == 0) {
    MSYS_RemoveRegion(&gDoneRegion);
    DoneRegionCreated = FALSE;
    //		gfDestroyDoneRegion = FALSE;
  }
}

void SelectAlumniDoneRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    gfDestroyPopUpBox = TRUE;
    gfDrawPopUpBox = FALSE;
    gfReDrawScreen = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
  }
}

void ChangingAimArchiveSubPage(uint8_t ubSubPageNumber) {
  fLoadPendingFlag = TRUE;

  if (AimArchivesSubPagesVisitedFlag[ubSubPageNumber] == FALSE) {
    fConnectingToSubPage = TRUE;
    fFastLoadFlag = FALSE;

    AimArchivesSubPagesVisitedFlag[ubSubPageNumber] = TRUE;
  } else {
    fConnectingToSubPage = TRUE;
    fFastLoadFlag = TRUE;
  }
}
