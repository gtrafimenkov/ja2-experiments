// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/CharProfile.h"

#include <string.h>

#include "Laptop/IMPAboutUs.h"
#include "Laptop/IMPAttributeEntrance.h"
#include "Laptop/IMPAttributeFinish.h"
#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPBeginScreen.h"
#include "Laptop/IMPConfirm.h"
#include "Laptop/IMPFinish.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPPersonalityEntrance.h"
#include "Laptop/IMPPersonalityFinish.h"
#include "Laptop/IMPPersonalityQuiz.h"
#include "Laptop/IMPPortraits.h"
#include "Laptop/IMPTextSystem.h"
#include "Laptop/IMPVideoObjects.h"
#include "Laptop/IMPVoices.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "MessageBoxScreen.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "Utils/Cursors.h"
#include "Utils/Text.h"

// BOOLEAN fIMPCompletedFlag = FALSE;
BOOLEAN fReDrawCharProfile = FALSE;
BOOLEAN fButtonPendingFlag = FALSE;
BOOLEAN fAddCreatedCharToPlayersTeam = FALSE;
BOOLEAN fReEnterIMP = FALSE;

int32_t iCurrentImpPage = IMP_HOME_PAGE;
int32_t iPreviousImpPage = -1;

// attributes
int32_t iStrength = 55;
int32_t iDexterity = 55;
int32_t iAgility = 55;
int32_t iWisdom = 55;
int32_t iLeadership = 55;
int32_t iHealth = 55;

// skills
int32_t iMarksmanship = 55;
int32_t iMedical = 55;
int32_t iExplosives = 55;
int32_t iMechanical = 55;

// gender
BOOLEAN fCharacterIsMale = TRUE;

// name and nick name
wchar_t pFullName[32];
wchar_t pNickName[32];

// skills
int32_t iSkillA = 0;
int32_t iSkillB = 0;

// personality
int32_t iPersonality = 0;

// attitude
int32_t iAttitude = 0;

// additives, but no preservatives
int32_t iAddStrength = 0;
int32_t iAddDexterity = 0;
int32_t iAddAgility = 0;
int32_t iAddWisdom = 0;
int32_t iAddHealth = 0;
int32_t iAddLeadership = 0;

int32_t iAddMarksmanship = 0;
int32_t iAddMedical = 0;
int32_t iAddExplosives = 0;
int32_t iAddMechanical = 0;

// IMP global buttons
int32_t giIMPButton[1];
int32_t giIMPButtonImage[1];

// visted subpages
BOOLEAN fVisitedIMPSubPages[IMP_NUM_PAGES];
extern int32_t iCurrentPortrait;
extern int32_t iCurrentVoices;
extern int32_t giMaxPersonalityQuizQuestion;
extern BOOLEAN fStartOverFlag;

void ExitOldIMPMode(void);
void EnterNewIMPMode(void);
void LoadImpGraphics(void);
void RemoveImpGraphics(void);
void CreateIMPButtons(void);
void DestroyIMPButtons(void);
void BtnIMPCancelCallback(GUI_BUTTON *btn, int32_t reason);
BOOLEAN HasTheCurrentIMPPageBeenVisited(void);
extern void SetAttributes(void);

void GameInitCharProfile() {
  LaptopSaveInfo.iVoiceId = 0;
  iCurrentPortrait = 0;
  iCurrentVoices = 0;
  iPortraitNumber = 0;

  return;
}

void EnterCharProfile() {
  // reset previous page
  iPreviousImpPage = -1;

  // grab the graphics
  LoadImpGraphics();
}

void ExitCharProfile() {
  // get rid of graphics
  RemoveImpGraphics();

  // clean up past mode
  ExitOldIMPMode();
}

void HandleCharProfile() {
  if (fReDrawCharProfile) {
    // re draw
    RenderCharProfile();
    fReDrawCharProfile = FALSE;
  }

  // button pending, but not changing mode, still need a rernder, but under different circumstances
  if ((fButtonPendingFlag == TRUE) && (iCurrentImpPage == iPreviousImpPage)) {
    RenderCharProfile();
  }

  // page has changed, handle the fact..get rid of old page, load up new, and re render
  if ((iCurrentImpPage != iPreviousImpPage)) {
    if (fDoneLoadPending == FALSE) {
      // make sure we are not hosing memory
      Assert(iCurrentImpPage <= IMP_NUM_PAGES);

      fFastLoadFlag = HasTheCurrentIMPPageBeenVisited();
      fVisitedIMPSubPages[iCurrentImpPage] = TRUE;
      fConnectingToSubPage = TRUE;

      if (iPreviousImpPage != -1) {
        fLoadPendingFlag = TRUE;
        MarkButtonsDirty();
        return;
      } else {
        fDoneLoadPending = TRUE;
      }
    }

    fVisitedIMPSubPages[iCurrentImpPage] = TRUE;

    if (fButtonPendingFlag == TRUE) {
      // render screen
      RenderCharProfile();
      return;
    }

    // exity old mode
    ExitOldIMPMode();

    // set previous page
    iPreviousImpPage = iCurrentImpPage;

    // enter new
    EnterNewIMPMode();

    // render screen
    RenderCharProfile();

    // render title bar
  }

  // handle
  switch (iCurrentImpPage) {
    case (IMP_HOME_PAGE):
      HandleImpHomePage();
      break;
    case (IMP_BEGIN):
      HandleIMPBeginScreen();
      break;
    case (IMP_PERSONALITY):
      HandleIMPPersonalityEntrance();
      break;
    case (IMP_PERSONALITY_QUIZ):
      HandleIMPPersonalityQuiz();
      break;
    case (IMP_PERSONALITY_FINISH):
      HandleIMPPersonalityFinish();
      break;
    case (IMP_ATTRIBUTE_ENTRANCE):
      HandleIMPAttributeEntrance();
      break;
    case (IMP_ATTRIBUTE_PAGE):
      HandleIMPAttributeSelection();
      break;
    case (IMP_ATTRIBUTE_FINISH):
      HandleIMPAttributeFinish();
      break;
    case (IMP_PORTRAIT):
      HandleIMPPortraits();
      break;
    case (IMP_VOICE):
      HandleIMPVoices();
      break;
    case (IMP_FINISH):
      HandleIMPFinish();
      break;
    case (IMP_ABOUT_US):
      HandleIMPAboutUs();
      break;
    case (IMP_MAIN_PAGE):
      HandleIMPMainPage();
      break;
    case (IMP_CONFIRM):
      HandleIMPConfirm();
      break;
  }

  return;
}

void RenderCharProfile() {
  // button is waiting to go up?...do nothing,

  if (fButtonPendingFlag) {
    fPausedReDrawScreenFlag = TRUE;
    fButtonPendingFlag = FALSE;
    return;
  }

  switch (iCurrentImpPage) {
    case (IMP_HOME_PAGE):
      RenderImpHomePage();
      break;
    case (IMP_BEGIN):
      RenderIMPBeginScreen();
      break;
    case (IMP_PERSONALITY):
      RenderIMPPersonalityEntrance();
      break;
    case (IMP_PERSONALITY_QUIZ):
      RenderIMPPersonalityQuiz();
      break;
    case (IMP_PERSONALITY_FINISH):
      RenderIMPPersonalityFinish();
      break;
    case (IMP_ATTRIBUTE_ENTRANCE):
      RenderIMPAttributeEntrance();
      break;
    case (IMP_ATTRIBUTE_PAGE):
      RenderIMPAttributeSelection();
      break;
    case (IMP_ATTRIBUTE_FINISH):
      RenderIMPAttributeFinish();
      break;
    case (IMP_PORTRAIT):
      RenderIMPPortraits();
      break;
    case (IMP_VOICE):
      RenderIMPVoices();
      break;
    case (IMP_FINISH):
      RenderIMPFinish();
      break;
    case (IMP_ABOUT_US):
      RenderIMPAboutUs();
      break;
    case (IMP_MAIN_PAGE):
      RenderIMPMainPage();
      break;
    case (IMP_CONFIRM):
      RenderIMPConfirm();
      break;
  }

  // render title bar
  // RenderWWWProgramTitleBar( );

  // render the text
  PrintImpText();

  RenderWWWProgramTitleBar();

  DisplayProgramBoundingBox(TRUE);

  // InvalidateRegion( 0, 0, 640, 480 );
  return;
}

void ExitOldIMPMode(void) {
  // exit old mode

  if (iPreviousImpPage == -1) {
    // don't both, leave
    return;
  }
  // remove old mode
  switch (iPreviousImpPage) {
    case (IMP_HOME_PAGE):
      ExitImpHomePage();
      break;
    case (IMP_BEGIN):
      DestroyIMPButtons();
      ExitIMPBeginScreen();
      break;
    case (IMP_FINISH):
      DestroyIMPButtons();
      ExitIMPFinish();
      break;
    case (IMP_PERSONALITY):
      DestroyIMPButtons();
      ExitIMPPersonalityEntrance();
      break;
    case (IMP_PERSONALITY_QUIZ):
      DestroyIMPButtons();
      ExitIMPPersonalityQuiz();
      break;
    case (IMP_PERSONALITY_FINISH):
      DestroyIMPButtons();
      ExitIMPPersonalityFinish();
      break;
    case (IMP_ATTRIBUTE_ENTRANCE):
      DestroyIMPButtons();
      ExitIMPAttributeEntrance();
      break;
    case (IMP_ATTRIBUTE_PAGE):
      DestroyIMPButtons();
      ExitIMPAttributeSelection();
      break;
    case (IMP_ATTRIBUTE_FINISH):
      DestroyIMPButtons();
      ExitIMPAttributeFinish();
      break;
    case (IMP_PORTRAIT):
      DestroyIMPButtons();
      ExitIMPPortraits();
      break;
    case (IMP_VOICE):
      DestroyIMPButtons();
      ExitIMPVoices();
      break;
    case (IMP_ABOUT_US):
      ExitIMPAboutUs();
      break;
    case (IMP_MAIN_PAGE):
      ExitIMPMainPage();
      break;
    case (IMP_CONFIRM):
      ExitIMPConfirm();
      break;
  }

  return;
}

void EnterNewIMPMode(void) {
  // enter new mode

  switch (iCurrentImpPage) {
    case (IMP_HOME_PAGE):
      EnterImpHomePage();
      break;
    case (IMP_BEGIN):
      CreateIMPButtons();
      EnterIMPBeginScreen();
      break;
    case (IMP_FINISH):
      CreateIMPButtons();
      EnterIMPFinish();
      break;
    case (IMP_PERSONALITY):
      CreateIMPButtons();
      EnterIMPPersonalityEntrance();
      break;
    case (IMP_PERSONALITY_QUIZ):
      CreateIMPButtons();
      EnterIMPPersonalityQuiz();
      break;
    case (IMP_PERSONALITY_FINISH):
      CreateIMPButtons();
      EnterIMPPersonalityFinish();
      break;
    case (IMP_ATTRIBUTE_ENTRANCE):
      CreateIMPButtons();
      EnterIMPAttributeEntrance();
      break;
    case (IMP_ATTRIBUTE_PAGE):
      CreateIMPButtons();
      EnterIMPAttributeSelection();
      break;
    case (IMP_ATTRIBUTE_FINISH):
      CreateIMPButtons();
      EnterIMPAttributeFinish();
      break;
    case (IMP_PORTRAIT):
      CreateIMPButtons();
      EnterIMPPortraits();
      break;
    case (IMP_VOICE):
      CreateIMPButtons();
      EnterIMPVoices();
      break;
    case (IMP_ABOUT_US):
      EnterIMPAboutUs();
      break;
    case (IMP_MAIN_PAGE):
      EnterIMPMainPage();
      break;
    case (IMP_CONFIRM):
      EnterIMPConfirm();
      break;
  }

  return;
}

void ResetCharacterStats(void) {
  // attributes
  iStrength = 55;
  iDexterity = 55;
  iAgility = 55;
  iWisdom = 55;
  iLeadership = 55;
  iHealth = 55;

  // skills
  iMarksmanship = 55;
  iMedical = 55;
  iExplosives = 55;
  iMechanical = 55;

  // skills
  iSkillA = 0;
  iSkillB = 0;

  // personality
  iPersonality = 0;

  // attitude
  iAttitude = 0;

  // names
  memset(&pFullName, 0, sizeof(pFullName));
  memset(&pNickName, 0, sizeof(pNickName));
}

void LoadImpGraphics(void) {
  // load all graphics needed for IMP

  LoadProfileBackGround();
  LoadIMPSymbol();
  LoadBeginIndent();
  LoadActivationIndent();
  LoadFrontPageIndent();
  LoadAnalyse();
  LoadAttributeGraph();
  LoadAttributeGraphBar();

  LoadFullNameIndent();
  LoadNameIndent();
  LoadGenderIndent();
  LoadNickNameIndent();

  // LoadSmallFrame( );

  LoadSmallSilhouette();
  LoadLargeSilhouette();

  LoadAttributeFrame();
  LoadSliderBar();

  LoadButton2Image();
  LoadButton4Image();
  LoadButton1Image();

  LoadPortraitFrame();
  LoadMainIndentFrame();

  LoadQtnLongIndentFrame();
  LoadQtnShortIndentFrame();
  LoadQtnLongIndentHighFrame();
  LoadQtnShortIndentHighFrame();
  LoadQtnShort2IndentFrame();
  LoadQtnShort2IndentHighFrame();

  LoadQtnIndentFrame();
  LoadAttrib1IndentFrame();
  LoadAttrib2IndentFrame();
  LoadAvgMercIndentFrame();
  LoadAboutUsIndentFrame();

  return;
}

void RemoveImpGraphics(void) {
  // remove all graphics needed for IMP

  RemoveProfileBackGround();
  DeleteIMPSymbol();
  DeleteBeginIndent();
  DeleteActivationIndent();
  DeleteFrontPageIndent();
  DeleteAnalyse();
  DeleteAttributeGraph();
  DeleteAttributeBarGraph();

  DeleteFullNameIndent();
  DeleteNameIndent();
  DeleteGenderIndent();
  DeleteNickNameIndent();

  // DeleteSmallFrame( );

  DeleteSmallSilhouette();
  DeleteLargeSilhouette();

  DeleteAttributeFrame();
  DeleteSliderBar();

  DeleteButton2Image();
  DeleteButton4Image();
  DeleteButton1Image();

  DeletePortraitFrame();
  DeleteMainIndentFrame();

  DeleteQtnLongIndentFrame();
  DeleteQtnShortIndentFrame();
  DeleteQtnLongIndentHighFrame();
  DeleteQtnShortIndentHighFrame();
  DeleteQtnShort2IndentFrame();
  DeleteQtnShort2IndentHighFrame();

  DeleteQtnIndentFrame();
  DeleteAttrib1IndentFrame();
  DeleteAttrib2IndentFrame();
  DeleteAvgMercIndentFrame();
  DeleteAboutUsIndentFrame();
  return;
}

void CreateIMPButtons(void) {
  // create all the buttons global to the IMP system

  giIMPButtonImage[0] = LoadButtonImage("LAPTOP\\button_3.sti", -1, 0, -1, 1, -1);

  // cancel
  giIMPButton[0] = CreateIconAndTextButton(
      giIMPButtonImage[0], pImpButtonText[19], FONT12ARIAL, FONT_WHITE, DEFAULT_SHADOW, FONT_WHITE,
      DEFAULT_SHADOW, TEXT_CJUSTIFIED, LAPTOP_SCREEN_UL_X + 15, LAPTOP_SCREEN_WEB_UL_Y + (360),
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnIMPCancelCallback);

  SpecifyButtonTextSubOffsets(giIMPButton[0], 0, -1, FALSE);

  // set up generic www cursor
  SetButtonCursor(giIMPButton[0], CURSOR_WWW);

  return;
}

void DestroyIMPButtons(void) {
  // destroy the buttons we created
  RemoveButton(giIMPButton[0]);
  UnloadButtonImage(giIMPButtonImage[0]);

  return;
}

void BtnIMPCancelCallback(GUI_BUTTON *btn, int32_t reason) {
  // btn callback for IMP cancel button
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= (BUTTON_CLICKED_ON);
  }

  else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);

      // back to the main page, otherwise, back to home page
      if (iCurrentImpPage == IMP_MAIN_PAGE) {
        iCurrentImpPage = IMP_HOME_PAGE;
        fButtonPendingFlag = TRUE;
        iCurrentProfileMode = 0;
        fFinishedCharGeneration = FALSE;
        ResetCharacterStats();
      } else if (iCurrentImpPage == IMP_FINISH) {
        iCurrentImpPage = IMP_MAIN_PAGE;
        iCurrentProfileMode = 4;
        fFinishedCharGeneration = FALSE;
        fButtonPendingFlag = TRUE;
        // iCurrentProfileMode = 0;
        // fFinishedCharGeneration = FALSE;
        // ResetCharacterStats( );

      }

      else if (iCurrentImpPage == IMP_PERSONALITY_QUIZ ||
               iCurrentImpPage == IMP_PERSONALITY_FINISH) {
        giMaxPersonalityQuizQuestion = 0;
        fStartOverFlag = TRUE;
        iCurrentAnswer = -1;
        iCurrentImpPage = IMP_PERSONALITY;
        fButtonPendingFlag = TRUE;
      }

      else {
        if (iCurrentImpPage == IMP_ATTRIBUTE_PAGE) {
          SetAttributes();
        }
        iCurrentImpPage = IMP_MAIN_PAGE;
        iCurrentAnswer = -1;
      }
    }
  }

  return;
}

void InitIMPSubPageList(void) {
  int32_t iCounter = 0;

  for (iCounter = 0; iCounter < IMP_CONFIRM; iCounter++) {
    fVisitedIMPSubPages[iCounter] = FALSE;
  }

  return;
}

BOOLEAN HasTheCurrentIMPPageBeenVisited(void) {
  // returns if we have vsisted the current IMP PageAlready

  // make sure we are not hosing memory
  Assert(iCurrentImpPage <= IMP_NUM_PAGES);

  return (fVisitedIMPSubPages[iCurrentImpPage]);
}
