// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/IMPVideoObjects.h"

#include "Laptop/IMPAttributeSelection.h"
#include "Laptop/IMPHomePage.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

// globals

// video object handles
uint32_t guiBACKGROUND;
uint32_t guiIMPSYMBOL;
uint32_t guiBEGININDENT;
uint32_t guiACTIVATIONINDENT;
uint32_t guiFRONTPAGEINDENT;
uint32_t guiFULLNAMEINDENT;
uint32_t guiNAMEINDENT;
uint32_t guiNICKNAMEINDENT;
uint32_t guiGENDERINDENT;
uint32_t guiSMALLFRAME;
uint32_t guiANALYSE;
uint32_t guiATTRIBUTEGRAPH;
uint32_t guiATTRIBUTEGRAPHBAR;
uint32_t guiSMALLSILHOUETTE;
uint32_t guiLARGESILHOUETTE;
uint32_t guiPORTRAITFRAME;
uint32_t guiSLIDERBAR;
uint32_t guiATTRIBUTEFRAME;
uint32_t guiATTRIBUTESCREENINDENT1;
uint32_t guiATTRIBUTESCREENINDENT2;
uint32_t guiATTRIBUTEBAR;
uint32_t guiBUTTON2IMAGE;
uint32_t guiBUTTON1IMAGE;
uint32_t guiBUTTON4IMAGE;
uint32_t guiMAININDENT;
uint32_t guiLONGINDENT;
uint32_t guiSHORTINDENT;
uint32_t guiSHORTHINDENT;
uint32_t guiSHORT2INDENT;
uint32_t guiLONGHINDENT;
uint32_t guiQINDENT;
uint32_t guiA1INDENT;
uint32_t guiA2INDENT;
uint32_t guiAVGMERCINDENT;
uint32_t guiABOUTUSINDENT;
uint32_t guiSHORT2HINDENT;

// position defines
#define CHAR_PROFILE_BACKGROUND_TILE_WIDTH 125
#define CHAR_PROFILE_BACKGROUND_TILE_HEIGHT 100

extern void DrawBonusPointsRemaining(void);

BOOLEAN LoadProfileBackGround(void) {
  // this procedure will load in the graphics for the generic background

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\MetalBackGround.sti"), &guiBACKGROUND));

  return (TRUE);
}

void RemoveProfileBackGround(void) {
  // remove background
  DeleteVideoObjectFromIndex(guiBACKGROUND);

  return;
}

void RenderProfileBackGround(void) {
  struct VObject* hHandle;
  int32_t iCounter = 0;

  // this procedure will render the generic backgound to the screen

  // get the video object
  GetVideoObject(&hHandle, guiBACKGROUND);

  // render each row 5 times wide, 5 tiles high
  for (iCounter = 0; iCounter < 4; iCounter++) {
    // blt background to screen from left to right
    BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + 0 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
    BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + 1 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
    BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + 2 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
    BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + 3 * CHAR_PROFILE_BACKGROUND_TILE_WIDTH,
                   LAPTOP_SCREEN_WEB_UL_Y + iCounter * CHAR_PROFILE_BACKGROUND_TILE_HEIGHT);
  }

  // dirty buttons
  MarkButtonsDirty();

  // force refresh of screen
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, 640, 480);

  return;
}

BOOLEAN LoadIMPSymbol(void) {
  CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_IMPSYMBOL), &guiIMPSYMBOL));
  return (TRUE);
}

void DeleteIMPSymbol(void) { DeleteVideoObjectFromIndex(guiIMPSYMBOL); }

void RenderIMPSymbol(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiIMPSYMBOL);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadBeginIndent(void) {
  // this procedure will load the indent main symbol into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BeginScreenIndent.sti"), &guiBEGININDENT));

  return (TRUE);
}

void DeleteBeginIndent(void) {
  // remove indent symbol

  DeleteVideoObjectFromIndex(guiBEGININDENT);

  return;
}

void RenderBeginIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBEGININDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadActivationIndent(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\ActivationIndent.sti"), &guiACTIVATIONINDENT));

  return (TRUE);
}

void DeleteActivationIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiACTIVATIONINDENT);

  return;
}

void RenderActivationIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiACTIVATIONINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadFrontPageIndent(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\FrontPageIndent.sti"), &guiFRONTPAGEINDENT));

  return (TRUE);
}

void DeleteFrontPageIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiFRONTPAGEINDENT);

  return;
}

void RenderFrontPageIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiFRONTPAGEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAnalyse(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Analyze.sti"), &guiANALYSE));

  return (TRUE);
}

void DeleteAnalyse(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiANALYSE);

  return;
}

void RenderAnalyse(int16_t sX, int16_t sY, int8_t bImageNumber) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiANALYSE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, bImageNumber, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAttributeGraph(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Attributegraph.sti"), &guiATTRIBUTEGRAPH));

  return (TRUE);
}

void DeleteAttributeGraph(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiATTRIBUTEGRAPH);

  return;
}

void RenderAttributeGraph(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEGRAPH);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAttributeGraphBar(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\AttributegraphBar.sti"), &guiATTRIBUTEGRAPHBAR));

  return (TRUE);
}

void DeleteAttributeBarGraph(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiATTRIBUTEGRAPHBAR);

  return;
}

void RenderAttributeBarGraph(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEGRAPHBAR);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadFullNameIndent(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\FullNameIndent.sti"), &guiFULLNAMEINDENT));

  return (TRUE);
}

void DeleteFullNameIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiFULLNAMEINDENT);

  return;
}

void RenderFullNameIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiFULLNAMEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadNickNameIndent(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\NickName.sti"), &guiNICKNAMEINDENT));

  return (TRUE);
}

void DeleteNickNameIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiNICKNAMEINDENT);

  return;
}

void RenderNickNameIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiNICKNAMEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadNameIndent(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\NameIndent.sti"), &guiNAMEINDENT));

  return (TRUE);
}

void DeleteNameIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiNAMEINDENT);

  return;
}

void RenderNameIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiNAMEINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadGenderIndent(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\GenderIndent.sti"), &guiGENDERINDENT));

  return (TRUE);
}

void DeleteGenderIndent(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiGENDERINDENT);

  return;
}

void RenderGenderIndent(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiGENDERINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadSmallFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\SmallFrame.sti"), &guiSMALLFRAME));

  return (TRUE);
}

void DeleteSmallFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSMALLFRAME);

  return;
}

void RenderSmallFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSMALLFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadSmallSilhouette(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\SmallSilhouette.sti"), &guiSMALLSILHOUETTE));

  return (TRUE);
}

void DeleteSmallSilhouette(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSMALLSILHOUETTE);

  return;
}

void RenderSmallSilhouette(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSMALLSILHOUETTE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadLargeSilhouette(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\largesilhouette.sti"), &guiLARGESILHOUETTE));

  return (TRUE);
}

void DeleteLargeSilhouette(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiLARGESILHOUETTE);

  return;
}

void RenderLargeSilhouette(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLARGESILHOUETTE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAttributeFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\attributeframe.sti"), &guiATTRIBUTEFRAME));

  return (TRUE);
}

void DeleteAttributeFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiATTRIBUTEFRAME);

  return;
}

void RenderAttributeFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;
  int32_t iCounter = 0;
  int16_t sCurrentY = 0;

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  sCurrentY += 10;
  for (iCounter = 0; iCounter < 10; iCounter++) {
    // blt to sX, sY relative to upper left corner
    BltVideoObject(vsFB, hHandle, 2, LAPTOP_SCREEN_UL_X + sX + 134,
                   LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);
    BltVideoObject(vsFB, hHandle, 1, LAPTOP_SCREEN_UL_X + sX,
                   LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);
    BltVideoObject(vsFB, hHandle, 3, LAPTOP_SCREEN_UL_X + sX + 368,
                   LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

    sCurrentY += 20;
  }

  BltVideoObject(vsFB, hHandle, 4, LAPTOP_SCREEN_UL_X + sX,
                 LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

  return;
}

void RenderAttributeFrameForIndex(int16_t sX, int16_t sY, int32_t iIndex) {
  int16_t sCurrentY = 0;
  struct VObject* hHandle;

  // valid index?
  if (iIndex == -1) {
    return;
  }

  sCurrentY = (int16_t)(10 + (iIndex * 20));

  // get the video object
  GetVideoObject(&hHandle, guiATTRIBUTEFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 2, LAPTOP_SCREEN_UL_X + sX + 134,
                 LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY);

  RenderAttrib2IndentFrame(350, 42);

  // amt of bonus pts
  DrawBonusPointsRemaining();

  // render attribute boxes
  RenderAttributeBoxes();

  InvalidateRegion(LAPTOP_SCREEN_UL_X + sX + 134, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY,
                   LAPTOP_SCREEN_UL_X + sX + 400, LAPTOP_SCREEN_WEB_UL_Y + sY + sCurrentY + 21);

  return;
}

BOOLEAN LoadSliderBar(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\attributeslider.sti"), &guiSLIDERBAR));

  return (TRUE);
}

void DeleteSliderBar(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSLIDERBAR);

  return;
}

void RenderSliderBar(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSLIDERBAR);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadButton2Image(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\button_2.sti"), &guiBUTTON2IMAGE));

  return (TRUE);
}

void DeleteButton2Image(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiBUTTON2IMAGE);

  return;
}

void RenderButton2Image(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON2IMAGE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadButton4Image(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\button_4.sti"), &guiBUTTON4IMAGE));

  return (TRUE);
}

void DeleteButton4Image(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiBUTTON4IMAGE);

  return;
}

void RenderButton4Image(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON4IMAGE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadButton1Image(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\button_1.sti"), &guiBUTTON1IMAGE));

  return (TRUE);
}

void DeleteButton1Image(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiBUTTON1IMAGE);

  return;
}

void RenderButton1Image(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiBUTTON1IMAGE);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadPortraitFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Voice_PortraitFrame.sti"), &guiPORTRAITFRAME));

  return (TRUE);
}

void DeletePortraitFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiPORTRAITFRAME);

  return;
}

void RenderPortraitFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiPORTRAITFRAME);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadMainIndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\mainprofilepageindent.sti"), &guiMAININDENT));

  return (TRUE);
}

void DeleteMainIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiMAININDENT);

  return;
}

void RenderMainIndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiMAININDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnLongIndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\longindent.sti"), &guiLONGINDENT));

  return (TRUE);
}

void DeleteQtnLongIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiLONGINDENT);

  return;
}

void RenderQtnLongIndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLONGINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnShortIndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\shortindent.sti"), &guiSHORTINDENT));

  return (TRUE);
}

void DeleteQtnShortIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORTINDENT);

  return;
}

void RenderQtnShortIndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORTINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnLongIndentHighFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\longindenthigh.sti"), &guiLONGHINDENT));

  return (TRUE);
}

void DeleteQtnLongIndentHighFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiLONGHINDENT);

  return;
}

void RenderQtnLongIndentHighFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiLONGHINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnShortIndentHighFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\shortindenthigh.sti"), &guiSHORTHINDENT));

  return (TRUE);
}

void DeleteQtnShortIndentHighFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORTHINDENT);

  return;
}

void RenderQtnShortIndentHighFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORTHINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnIndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\questionindent.sti"), &guiQINDENT));

  return (TRUE);
}

void DeleteQtnIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiQINDENT);

  return;
}

void RenderQtnIndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiQINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAttrib1IndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\attributescreenindent_1.sti"), &guiA1INDENT));

  return (TRUE);
}

void DeleteAttrib1IndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiA1INDENT);

  return;
}

void RenderAttrib1IndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiA1INDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAttrib2IndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\attributescreenindent_2.sti"), &guiA2INDENT));

  return (TRUE);
}

void DeleteAttrib2IndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiA2INDENT);

  return;
}

void RenderAttrib2IndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiA2INDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAvgMercIndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\anaveragemercindent.sti"), &guiAVGMERCINDENT));

  return (TRUE);
}

void DeleteAvgMercIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiAVGMERCINDENT);

  return;
}

void RenderAvgMercIndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiAVGMERCINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadAboutUsIndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\aboutusindent.sti"), &guiABOUTUSINDENT));

  return (TRUE);
}

void DeleteAboutUsIndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiABOUTUSINDENT);

  return;
}

void RenderAboutUsIndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiABOUTUSINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnShort2IndentFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\shortindent2.sti"), &guiSHORT2INDENT));

  return (TRUE);
}

void DeleteQtnShort2IndentFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORT2INDENT);

  return;
}

void RenderQtnShort2IndentFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORT2INDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}

BOOLEAN LoadQtnShort2IndentHighFrame(void) {
  // this procedure will load the activation indent into memory

  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\shortindent2High.sti"), &guiSHORT2HINDENT));

  return (TRUE);
}

void DeleteQtnShort2IndentHighFrame(void) {
  // remove activation indent symbol
  DeleteVideoObjectFromIndex(guiSHORT2HINDENT);

  return;
}

void RenderQtnShort2IndentHighFrame(int16_t sX, int16_t sY) {
  struct VObject* hHandle;

  // get the video object
  GetVideoObject(&hHandle, guiSHORT2HINDENT);

  // blt to sX, sY relative to upper left corner
  BltVideoObject(vsFB, hHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);

  return;
}
