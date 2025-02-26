// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/IMPTextSystem.h"

#include "Laptop/CharProfile.h"
#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPMainPage.h"
#include "Laptop/IMPPersonalityFinish.h"
#include "Laptop/IMPPersonalityQuiz.h"
#include "Laptop/Laptop.h"
#include "SGP/Types.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/WordWrap.h"

#define IMP_SEEK_AMOUNT 5 * 80 * 2

#define IMP_LEFT_IDENT_TEXT_X 116
#define IMP_RIGHT_IDENT_TEXT_X 509
#define IMP_IDENT_WIDTH 96

BOOLEAN fInitialized = FALSE;

int32_t iIMPTextRecordLengths[300];

// the length of persona questions
int32_t iIMPQuestionLengths[25] = {
    7, 5, 5, 6, 5, 6, 5, 5, 5, 5, 6, 9, 5, 5, 5, 5, 5, 5, 5, 5, 7, 10, 6, 5, 5,
};

// function headers
void PrintIMPPersonalityQuizQuestionAndAnsers(void);
void OffSetQuestionForFemaleSpecificQuestions(int32_t *iCurrentOffset);

#define QTN_FIRST_COLUMN_X 80
#define QTN_SECOND_COLUMN_X 320

void LoadAndDisplayIMPText(int16_t sStartX, int16_t sStartY, int16_t sLineLength,
                           int16_t sIMPTextRecordNumber, uint32_t uiFont, uint8_t ubColor,
                           BOOLEAN fShadow, uint32_t uiFlags) {
  // this procedure will load and display to the screen starting at postion X, Y relative to the
  // start of the laptop screen it will access record sIMPTextRecordNumber and go until all records
  // following it but before the next IMP record are displayed in font uiFont
  wchar_t sString[1024];

  if (fShadow == FALSE) {
    // don't want shadow, remove it
    SetFontShadow(NO_SHADOW);
  }

  // load the string
  LoadEncryptedDataFromFile("BINARYDATA\\IMPText.EDT", sString,
                            (uint32_t)((sIMPTextRecordNumber)*IMP_SEEK_AMOUNT), IMP_SEEK_AMOUNT);

  // null put last char
  sString[wcslen(sString)] = 0;

  if (uiFlags == 0) {
    uiFlags = LEFT_JUSTIFIED;
  }

  DisplayWrappedString(sStartX, (int16_t)(sStartY), sLineLength, 2, uiFont, ubColor, sString,
                       FONT_BLACK, FALSE, uiFlags);

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);
}

void InitializeImpRecordLengthList(void) {
  // this procedure will setup the IMP records length list with the appropriate values

  return;
}

void PrintImpText(void) {
  int16_t sWidth = LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X + 1;

  // looks at current page and prints text needed
  switch (iCurrentImpPage) {
    case (IMP_HOME_PAGE):
      // the imp homepage
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_DELTA_Y + 43, sWidth, IMP_HOME_1,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_DELTA_Y + 60, sWidth, IMP_HOME_2,
                            FONT10ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_DELTA_Y + 208, sWidth, IMP_HOME_3,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(IMP_LEFT_IDENT_TEXT_X, LAPTOP_SCREEN_WEB_DELTA_Y + 99, IMP_IDENT_WIDTH,
                            IMP_HOME_7, FONT10ARIAL, 142, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(IMP_RIGHT_IDENT_TEXT_X, LAPTOP_SCREEN_WEB_DELTA_Y + 99, IMP_IDENT_WIDTH,
                            IMP_HOME_8, FONT10ARIAL, 142, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(258, LAPTOP_SCREEN_WEB_DELTA_Y + 362, (640), IMP_HOME_5, FONT14ARIAL,
                            FONT_BLACK, FALSE, 0);
      LoadAndDisplayIMPText(IMP_LEFT_IDENT_TEXT_X, LAPTOP_SCREEN_WEB_DELTA_Y + 188, IMP_IDENT_WIDTH,
                            IMP_HOME_9, FONT10ARIAL, 142, TRUE, RIGHT_JUSTIFIED);
      LoadAndDisplayIMPText(IMP_RIGHT_IDENT_TEXT_X, LAPTOP_SCREEN_WEB_DELTA_Y + 188,
                            IMP_IDENT_WIDTH, IMP_HOME_10, FONT10ARIAL, 142, TRUE, RIGHT_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_DELTA_Y + 402, sWidth, IMP_HOME_6,
                            FONT12ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      break;
    case (IMP_ABOUT_US):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 17, LAPTOP_SCREEN_WEB_UL_Y + 137, (640),
                            IMP_ABOUT_US_1, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 25, LAPTOP_SCREEN_WEB_UL_Y + 154, (337 - 124),
                            IMP_ABOUT_US_2, FONT10ARIAL, 142, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 25, LAPTOP_SCREEN_WEB_UL_Y + 235, (337 - 124),
                            IMP_ABOUT_US_3, FONT10ARIAL, 142, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 17, LAPTOP_SCREEN_WEB_UL_Y + 260, (640),
                            IMP_ABOUT_US_10, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 25, LAPTOP_SCREEN_WEB_UL_Y + 280, (337 - 124),
                            IMP_ABOUT_US_4, FONT10ARIAL, 142, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 267, LAPTOP_SCREEN_WEB_UL_Y + 137, (640),
                            IMP_ABOUT_US_11, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 275, LAPTOP_SCREEN_WEB_UL_Y + 154, (337 - 129),
                            IMP_ABOUT_US_5, FONT10ARIAL, 142, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 267, LAPTOP_SCREEN_WEB_UL_Y + 227, (640),
                            IMP_ABOUT_US_8, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 275, LAPTOP_SCREEN_WEB_UL_Y + 247, (337 - 129),
                            IMP_ABOUT_US_6, FONT10ARIAL, 142, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 267, LAPTOP_SCREEN_WEB_UL_Y + 277, (640),
                            IMP_ABOUT_US_9, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 275, LAPTOP_SCREEN_WEB_UL_Y + 297, (337 - 129),
                            IMP_ABOUT_US_7, FONT10ARIAL, 142, TRUE, 0);

      break;
    case (IMP_MAIN_PAGE):
      // title
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 19, sWidth, IMP_MAIN_1,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      // set up for IMP text for title box area
      switch (iCurrentProfileMode) {
        case (0):
          LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 173, LAPTOP_SCREEN_WEB_UL_Y + 91, (329 - 173),
                                IMP_MAIN_2, FONT10ARIAL, 142, TRUE, 0);
          break;
        case (1):
          IanDisplayWrappedString(LAPTOP_SCREEN_UL_X + 173, LAPTOP_SCREEN_WEB_UL_Y + 91,
                                  (329 - 173), 2, FONT10ARIAL, 142, pExtraIMPStrings[0], 0, FALSE,
                                  0);
          break;
        case (2):
          IanDisplayWrappedString(LAPTOP_SCREEN_UL_X + 173, LAPTOP_SCREEN_WEB_UL_Y + 91,
                                  (329 - 173), 2, FONT10ARIAL, 142, pExtraIMPStrings[1], 0, FALSE,
                                  0);
          break;
        case (3):
          IanDisplayWrappedString(LAPTOP_SCREEN_UL_X + 173, LAPTOP_SCREEN_WEB_UL_Y + 91,
                                  (329 - 173), 2, FONT10ARIAL, 142, pExtraIMPStrings[2], 0, FALSE,
                                  0);
          break;
        case (4):
          IanDisplayWrappedString(LAPTOP_SCREEN_UL_X + 173, LAPTOP_SCREEN_WEB_UL_Y + 91,
                                  (329 - 173), 2, FONT10ARIAL, 142, pExtraIMPStrings[3], 0, FALSE,
                                  0);
          break;
      }

      break;
    case (IMP_BEGIN):

      // title
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_BEGIN_1,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 105, LAPTOP_SCREEN_WEB_UL_Y + 67, (390 - 105),
                            IMP_BEGIN_2, FONT10ARIAL, 142, TRUE, CENTER_JUSTIFIED);

      // fullname
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 81, LAPTOP_SCREEN_WEB_UL_Y + 139, (640),
                            IMP_BEGIN_3, FONT14ARIAL, FONT_BLACK, FALSE, 0);

      // nick name
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 81, LAPTOP_SCREEN_WEB_UL_Y + 199, (640),
                            IMP_BEGIN_4, FONT14ARIAL, FONT_BLACK, FALSE, 0);

      // gender
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 81, LAPTOP_SCREEN_WEB_UL_Y + 259, (640),
                            IMP_BEGIN_6, FONT14ARIAL, FONT_BLACK, FALSE, 0);

      // male
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 240, LAPTOP_SCREEN_WEB_UL_Y + 259, (640),
                            IMP_BEGIN_10, FONT14ARIAL, FONT_BLACK, FALSE, 0);

      // female
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 360, LAPTOP_SCREEN_WEB_UL_Y + 259, (640),
                            IMP_BEGIN_11, FONT14ARIAL, FONT_BLACK, FALSE, 0);

      break;
    case (IMP_PERSONALITY):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 130, LAPTOP_SCREEN_WEB_UL_Y + 60, (456 - 200),
                            IMP_PERS_1, FONT12ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 130, LAPTOP_SCREEN_WEB_UL_Y + 130, (456 - 200),
                            IMP_PERS_2, FONT12ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_PERS_6,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      break;
    case (IMP_PERSONALITY_QUIZ):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 5, sWidth, IMP_PERS_6,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 293, LAPTOP_SCREEN_WEB_UL_Y + 370, (456 - 200),
                            IMP_PERS_11, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 363, LAPTOP_SCREEN_WEB_UL_Y + 370, (456 - 200),
                            IMP_PERS_12, FONT12ARIAL, FONT_WHITE, TRUE, 0);

      // print the question and suitable answers
      PrintIMPPersonalityQuizQuestionAndAnsers();

      break;
    case (IMP_PERSONALITY_FINISH):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_PERS_6,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      switch (bPersonalityEndState) {
        case (0):
          LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, (356 - 100),
                                IMP_PERS_F1, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
          break;
        case (1):
          LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, (356 - 100),
                                IMP_PERS_F4, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
          break;
        case (2):
          LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, (356 - 100),
                                IMP_PERS_F5, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
          break;
      }
      break;
    case (IMP_ATTRIBUTE_ENTRANCE):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth,
                            IMP_ATTRIB_1 - 1, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 110, LAPTOP_SCREEN_WEB_UL_Y + 50, (300),
                            IMP_ATTRIB_5, FONT12ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 110, LAPTOP_SCREEN_WEB_UL_Y + 130, (300),
                            IMP_ATTRIB_6, FONT12ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 110, LAPTOP_SCREEN_WEB_UL_Y + 200, (300),
                            IMP_ATTRIB_7, FONT12ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      break;
    case (IMP_ATTRIBUTE_PAGE):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth,
                            IMP_ATTRIB_1 - 1, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      // don't blit bonus if reviewing
      if (fReviewStats != TRUE) {
        LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 355, LAPTOP_SCREEN_WEB_UL_Y + 51, (640),
                              IMP_ATTRIB_SA_2 - 1, FONT12ARIAL, FONT_WHITE, TRUE, 0);
        LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 56, LAPTOP_SCREEN_WEB_UL_Y + 33, (240),
                              IMP_ATTRIB_SA_15, FONT10ARIAL, FONT_WHITE, TRUE, 0);
      } else {
        LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 56, LAPTOP_SCREEN_WEB_UL_Y + 33, (240),
                              IMP_ATTRIB_SA_18, FONT10ARIAL, FONT_WHITE, TRUE, 0);
      }
      // stats
      // health
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 0 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_6 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // dex
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 1 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_8 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // agili
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 2 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_7 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // str
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 3 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_9 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // wisdom
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 4 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_11 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // lead
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 5 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_10 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // marks
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 6 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_12 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // med
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 7 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_14 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // expl
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 8 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_15 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);
      // mech
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60,
                            LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 9 * SKILL_SLIDE_HEIGHT,
                            (100), IMP_ATTRIB_SA_13 - 1, FONT12ARIAL, FONT_WHITE, TRUE,
                            RIGHT_JUSTIFIED);

      // should we display zero warning or nowmal ' come on herc..' text

      break;
    case (IMP_ATTRIBUTE_FINISH):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth,
                            IMP_ATTRIB_1 - 1, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, (356 - 100),
                            IMP_AF_2 - 1, FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);

      break;
    case (IMP_PORTRAIT):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_POR_1 - 1,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 135, LAPTOP_SCREEN_WEB_UL_Y + 68, (240),
                            IMP_POR_2 - 1, FONT10ARIAL, 142, TRUE, 0);

      break;
    case (IMP_VOICE):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_VOC_1 - 1,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 135, LAPTOP_SCREEN_WEB_UL_Y + 70, (240),
                            IMP_VOC_2 - 1, FONT10ARIAL, 142, TRUE, 0);
      break;
    case (IMP_FINISH):
      // LoadAndDisplayIMPText( LAPTOP_SCREEN_UL_X + 160, LAPTOP_SCREEN_WEB_UL_Y + 7, ( 640  ),
      // IMP_FIN_1 - 1, FONT14ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 150, LAPTOP_SCREEN_WEB_UL_Y + 55, (200),
                            IMP_FIN_2 - 1, FONT12ARIAL, FONT_WHITE, TRUE, 0);
      break;
    case (IMP_CONFIRM):
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_CON_1,
                            FONT14ARIAL, FONT_WHITE, TRUE, CENTER_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 160, LAPTOP_SCREEN_WEB_UL_Y + 60, (200), IMP_CON_2,
                            FONT12ARIAL, FONT_WHITE, TRUE, 0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 160, LAPTOP_SCREEN_WEB_UL_Y + 145, (200),
                            IMP_CON_3, FONT12ARIAL, FONT_WHITE, TRUE, 0);

      break;
  }
}

void PrintImpTextPostButtonRender(void) {
  // prints any text after IMP buttons have been rendered
  switch (iCurrentImpPage) {
    case (IMP_HOME_PAGE):
      // about us button
      break;
  }
}

void PrintIMPPersonalityQuizQuestionAndAnsers(void) {
  int32_t iCounter = 0;
  int32_t iOffset = 0;

  if (giCurrentPersonalityQuizQuestion < 0) {
    return;
  }

  // how far into text is the question?
  for (iCounter = 0; iCounter < giCurrentPersonalityQuizQuestion; iCounter++) {
    // incrment until question is found
    iOffset += iIMPQuestionLengths[iCounter];
  }

  // handle any female specifc questions
  if (fCharacterIsMale == FALSE) {
    OffSetQuestionForFemaleSpecificQuestions(&iOffset);
  }

  // how many answers are there?
  switch (iIMPQuestionLengths[giCurrentPersonalityQuizQuestion]) {
    case (5):
      // 4 answers, write down the side, extra wide columns

      // question is at IMP_QUESTION_1 + iOffset
      // and there are 4 answers afterwards
      BltAnswerIndents(4);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 20, LAPTOP_SCREEN_WEB_UL_Y + 30, (460),
                            (int16_t)(IMP_QUESTION_1 + iOffset), FONT10ARIAL, FONT_WHITE, TRUE,
                            LEFT_JUSTIFIED);

      // answers
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (390), (int16_t)(IMP_QUESTION_1 + iOffset + 1), FONT10ARIAL, 142, TRUE,
                            LEFT_JUSTIFIED);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 150,
                            (390), (int16_t)(IMP_QUESTION_1 + iOffset + 2), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 200,
                            (390), (int16_t)(IMP_QUESTION_1 + iOffset + 3), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 250,
                            (390), (int16_t)(IMP_QUESTION_1 + iOffset + 4), FONT10ARIAL, 142, TRUE,
                            0);

      iOffset = 0;
      break;
    case (6):
      // question is at IMP_QUESTION_1 + iOffset
      // and there are 5 answers afterwards
      BltAnswerIndents(5);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 20, LAPTOP_SCREEN_WEB_UL_Y + 30, (460),
                            (int16_t)(IMP_QUESTION_1 + iOffset), FONT10ARIAL, FONT_WHITE, TRUE,
                            LEFT_JUSTIFIED);

      // answers
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 1), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 150,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 2), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 200,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 3), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 250,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 4), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 5), FONT10ARIAL, 142, TRUE,
                            0);

      break;
    case (7):
      // question is at IMP_QUESTION_1 + iOffset
      // and there are 5 answers afterwards
      BltAnswerIndents(6);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 20, LAPTOP_SCREEN_WEB_UL_Y + 30, (460),
                            (int16_t)(IMP_QUESTION_1 + iOffset), FONT10ARIAL, FONT_WHITE, TRUE,
                            LEFT_JUSTIFIED);

      // answers
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 1), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 150,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 2), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 200,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 3), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 250,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 4), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 5), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 150,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 6), FONT10ARIAL, 142, TRUE,
                            0);

      break;
    case (9):
      // question is at IMP_QUESTION_1 + iOffset
      // and there are 8 answers afterwards
      BltAnswerIndents(8);

      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 20, LAPTOP_SCREEN_WEB_UL_Y + 30, (460),
                            (int16_t)(IMP_QUESTION_1 + iOffset), FONT10ARIAL, FONT_WHITE, TRUE,
                            LEFT_JUSTIFIED);

      // answers
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 1), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 150,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 2), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 200,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 3), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_FIRST_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 250,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 4), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 100,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 5), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 150,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 6), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 200,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 7), FONT10ARIAL, 142, TRUE,
                            0);
      LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + QTN_SECOND_COLUMN_X, LAPTOP_SCREEN_WEB_UL_Y + 250,
                            (160), (int16_t)(IMP_QUESTION_1 + iOffset + 8), FONT10ARIAL, 142, TRUE,
                            0);

      break;
  }

  return;
}

void OffSetQuestionForFemaleSpecificQuestions(int32_t *iCurrentOffset) {
  int32_t iExtraOffSet = 0;
  BOOLEAN fOffSet = TRUE;

  // find the extra
  switch (giCurrentPersonalityQuizQuestion) {
    case (0):
      iExtraOffSet = 0;
      break;
    case (3):
      iExtraOffSet = iIMPQuestionLengths[0];
      break;
    case (8):
      iExtraOffSet = iIMPQuestionLengths[0] + iIMPQuestionLengths[3];
      break;
    case (9):
      iExtraOffSet = iIMPQuestionLengths[0] + iIMPQuestionLengths[3] + iIMPQuestionLengths[8];
      break;
    case (13):
      iExtraOffSet = iIMPQuestionLengths[0] + iIMPQuestionLengths[3] + iIMPQuestionLengths[8] +
                     iIMPQuestionLengths[9];
      break;
    default:
      fOffSet = FALSE;
      break;
  }

  if (fOffSet) {
    *iCurrentOffset = IMP_CON_3 - (IMP_QUESTION_1 - 3);

    *iCurrentOffset += iExtraOffSet;
  }
}
