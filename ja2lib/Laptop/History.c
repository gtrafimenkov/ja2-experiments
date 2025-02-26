// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/History.h"

#include <stdio.h>

#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "Sector.h"
#include "Strategic/GameClock.h"
#include "Strategic/QuestText.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Message.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "platform.h"

#define TOP_X 0 + LAPTOP_SCREEN_UL_X
#define TOP_Y LAPTOP_SCREEN_UL_Y
#define BLOCK_HIST_HEIGHT 10
#define BOX_HEIGHT 14
#define TOP_DIVLINE_Y 101
#define DIVLINE_X 130
#define MID_DIVLINE_Y 155
#define BOT_DIVLINE_Y 204
#define TITLE_X 140
#define TITLE_Y 33
#define TEXT_X 140
#define PAGE_SIZE 22
#define RECORD_Y TOP_DIVLINE_Y
#define RECORD_HISTORY_WIDTH 200
#define PAGE_NUMBER_X TOP_X + 20
#define PAGE_NUMBER_Y TOP_Y + 33
#define HISTORY_DATE_X PAGE_NUMBER_X + 85
#define HISTORY_DATE_Y PAGE_NUMBER_Y
#define RECORD_LOCATION_WIDTH 142  // 95

#define HISTORY_HEADER_FONT FONT14ARIAL
#define HISTORY_TEXT_FONT FONT12ARIAL
#define RECORD_DATE_X TOP_X + 10
#define RECORD_DATE_WIDTH 31  // 68
#define RECORD_HEADER_Y 90

#define NUM_RECORDS_PER_PAGE PAGE_SIZE
#define SIZE_OF_HISTORY_FILE_RECORD                                                             \
  (sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + \
   sizeof(uint8_t) + sizeof(uint8_t))

// button positions
#define NEXT_BTN_X 577
#define PREV_BTN_X 553
#define BTN_Y 53

// graphics handles
// uint32_t guiGREYFRAME;
// uint32_t guiMIDDLE;
// uint32_t guiBOTTOM;
// uint32_t guiLINE;
uint32_t guiLONGLINE;
uint32_t guiSHADELINE;
// uint32_t guiVERTLINE;
// uint32_t guiBIGBOX;

enum {
  PREV_PAGE_BUTTON = 0,
  NEXT_PAGE_BUTTON,
};

// the page flipping buttons
int32_t giHistoryButton[2];
int32_t giHistoryButtonImage[2];
BOOLEAN fInHistoryMode = FALSE;

// current page displayed
int32_t iCurrentHistoryPage = 1;

// the History record list
HistoryUnitPtr pHistoryListHead = NULL;

// current History record (the one at the top of the current page)
HistoryUnitPtr pCurrentHistory = NULL;

// last page in list
uint32_t guiLastPageInHistoryRecordsList = 0;

// function definitions
BOOLEAN LoadHistory(void);
void RenderHistoryBackGround(void);
void RemoveHistory(void);
void CreateHistoryButtons(void);
void DestroyHistoryButtons(void);
void CreateHistoryButtons(void);
void DrawHistoryTitleText(void);
uint32_t ProcessAndEnterAHistoryRecord(uint8_t ubCode, uint32_t uiDate, uint8_t ubSecondCode,
                                       uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ,
                                       uint8_t ubColor);
void OpenAndReadHistoryFile(void);
BOOLEAN OpenAndWriteHistoryFile(void);
void ClearHistoryList(void);
void DisplayHistoryListHeaders(void);
void DisplayHistoryListBackground(void);
void DrawAPageofHistoryRecords(void);
BOOLEAN IncrementCurrentPageHistoryDisplay(void);
void DisplayPageNumberAndDateRange(void);
void ProcessHistoryTransactionString(wchar_t *pString, size_t bufSize, HistoryUnitPtr pHistory);
void SetHistoryButtonStates(void);
BOOLEAN LoadInHistoryRecords(uint32_t uiPage);
BOOLEAN LoadNextHistoryPage(void);
BOOLEAN LoadPreviousHistoryPage(void);
void SetLastPageInHistoryRecords(void);
uint32_t ReadInLastElementOfHistoryListAndReturnIdNumber(void);
BOOLEAN AppendHistoryToEndOfFile(HistoryUnitPtr pHistory);
BOOLEAN WriteOutHistoryRecords(uint32_t uiPage);
void GetQuestStartedString(uint8_t ubQuestValue, wchar_t *sQuestString);
void GetQuestEndedString(uint8_t ubQuestValue, wchar_t *sQuestString);
int32_t GetNumberOfHistoryPages();

#ifdef JA2TESTVERSION
void PerformCheckOnHistoryRecord(uint32_t uiErrorCode, uint8_t sSectorX, uint8_t sSectorY,
                                 int8_t bSectorZ);
#endif

// callbacks
void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON *btn, int32_t reason);
void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON *btn, int32_t reason);

uint32_t SetHistoryFact(uint8_t ubCode, uint8_t ubSecondCode, uint32_t uiDate, uint8_t sSectorX,
                        uint8_t sSectorY) {
  // adds History item to player's log(History List), returns unique id number of it
  // outside of the History system(the code in this .c file), this is the only function you'll ever
  // need
  uint32_t uiId = 0;
  uint8_t ubColor = 0;
  HistoryUnitPtr pHistory = pHistoryListHead;

  // clear the list
  ClearHistoryList();

  // process the actual data
  if (ubCode == HISTORY_QUEST_FINISHED) {
    ubColor = 0;
  } else {
    ubColor = 1;
  }
  uiId =
      ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, sSectorX, sSectorY, 0, ubColor);
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_HISTORY_UPDATED]);

  // history list head
  pHistory = pHistoryListHead;

  // append to end of file
  AppendHistoryToEndOfFile(pHistory);

  // if in history mode, reload current page
  if (fInHistoryMode) {
    iCurrentHistoryPage--;

    // load in first page
    LoadNextHistoryPage();
  }

  // return unique id of this transaction
  return uiId;
}

uint32_t AddHistoryToPlayersLog(uint8_t ubCode, uint8_t ubSecondCode, uint32_t uiDate,
                                uint8_t sSectorX, uint8_t sSectorY) {
  // adds History item to player's log(History List), returns unique id number of it
  // outside of the History system(the code in this .c file), this is the only function you'll ever
  // need
  uint32_t uiId = 0;
  HistoryUnitPtr pHistory = pHistoryListHead;

  // clear the list
  ClearHistoryList();

  // process the actual data
  uiId = ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, sSectorX, sSectorY, 0, 0);
  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_HISTORY_UPDATED]);

  // history list head
  pHistory = pHistoryListHead;

  // append to end of file
  AppendHistoryToEndOfFile(pHistory);

  // if in history mode, reload current page
  if (fInHistoryMode) {
    iCurrentHistoryPage--;

    // load in first page
    LoadNextHistoryPage();
  }

  // return unique id of this transaction
  return uiId;
}

void GameInitHistory() {
  if ((FileMan_Exists(HISTORY_DATA_FILE))) {
    // unlink history file
    Plat_ClearFileAttributes(HISTORY_DATA_FILE);
    FileMan_Delete(HISTORY_DATA_FILE);
  }

  AddHistoryToPlayersLog(HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO, 0, GetWorldTotalMin(), -1, -1);
}

void EnterHistory() {
  // load the graphics
  LoadHistory();

  // create History buttons
  CreateHistoryButtons();

  // reset current to first page
  if (LaptopSaveInfo.iCurrentHistoryPage > 0)
    iCurrentHistoryPage = LaptopSaveInfo.iCurrentHistoryPage - 1;
  else
    iCurrentHistoryPage = 0;

  // load in first page
  LoadNextHistoryPage();

  // render hbackground
  RenderHistory();

  // set the fact we are in the history viewer
  fInHistoryMode = TRUE;

  // build Historys list
  // OpenAndReadHistoryFile( );

  // force redraw of the entire screen
  // fReDrawScreenFlag=TRUE;

  // set inital states
  SetHistoryButtonStates();

  return;
}

void ExitHistory() {
  LaptopSaveInfo.iCurrentHistoryPage = iCurrentHistoryPage;

  // not in History system anymore
  fInHistoryMode = FALSE;

  // write out history list to file
  // OpenAndWriteHistoryFile( );

  // delete graphics
  RemoveHistory();

  // delete buttons
  DestroyHistoryButtons();

  ClearHistoryList();

  return;
}

void HandleHistory() {
  // DEF 2/5/99 Dont need to update EVERY FRAME!!!!
  // check and update status of buttons
  //  SetHistoryButtonStates( );
}

void RenderHistory(void) {
  // render the background to the display
  RenderHistoryBackGround();

  // the title bar text
  DrawHistoryTitleText();

  // the actual lists background
  DisplayHistoryListBackground();

  // the headers to each column
  DisplayHistoryListHeaders();

  // render the currentpage of records
  DrawAPageofHistoryRecords();

  // stuff at top of page, the date range and page numbers
  DisplayPageNumberAndDateRange();

  // title bar icon
  BlitTitleBarIcons();

  return;
}

BOOLEAN LoadHistory(void) {
  // load History video objects into memory

  // title bar
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\programtitlebar.sti"), &guiTITLE));

  // top portion of the screen background
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\historywindow.sti"), &guiTOP));

  // shaded line
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\historylines.sti"), &guiSHADELINE));

  // black divider line - long ( 480 length)
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\divisionline480.sti"), &guiLONGLINE));

  return (TRUE);
}

void RemoveHistory(void) {
  // delete history video objects from memory
  DeleteVideoObjectFromIndex(guiLONGLINE);
  DeleteVideoObjectFromIndex(guiTOP);
  DeleteVideoObjectFromIndex(guiTITLE);
  DeleteVideoObjectFromIndex(guiSHADELINE);

  return;
}

void RenderHistoryBackGround(void) {
  // render generic background for history system
  struct VObject *hHandle;

  // get title bar object
  GetVideoObject(&hHandle, guiTITLE);

  // blt title bar to screen
  BltVideoObject(vsFB, hHandle, 0, TOP_X, TOP_Y - 2);

  // get and blt the top part of the screen, video object and blt to screen
  GetVideoObject(&hHandle, guiTOP);
  BltVideoObject(vsFB, hHandle, 0, TOP_X, TOP_Y + 22);

  // display background for history list
  DisplayHistoryListBackground();
  return;
}

void DrawHistoryTitleText(void) {
  // setup the font stuff
  SetFont(HISTORY_HEADER_FONT);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(DEFAULT_SHADOW);

  // draw the pages title
  mprintf(TITLE_X, TITLE_Y, pHistoryTitle[0]);

  return;
}

void CreateHistoryButtons(void) {
  // the prev page button
  giHistoryButtonImage[PREV_PAGE_BUTTON] = LoadButtonImage("LAPTOP\\arrows.sti", -1, 0, -1, 1, -1);
  giHistoryButton[PREV_PAGE_BUTTON] =
      QuickCreateButton(giHistoryButtonImage[PREV_PAGE_BUTTON], PREV_BTN_X, BTN_Y, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)BtnHistoryDisplayPrevPageCallBack);

  // the next page button
  giHistoryButtonImage[NEXT_PAGE_BUTTON] = LoadButtonImage("LAPTOP\\arrows.sti", -1, 6, -1, 7, -1);
  giHistoryButton[NEXT_PAGE_BUTTON] =
      QuickCreateButton(giHistoryButtonImage[NEXT_PAGE_BUTTON], NEXT_BTN_X, BTN_Y, BUTTON_TOGGLE,
                        MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
                        (GUI_CALLBACK)BtnHistoryDisplayNextPageCallBack);

  // set buttons
  SetButtonCursor(giHistoryButton[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giHistoryButton[1], CURSOR_LAPTOP_SCREEN);

  return;
}

void DestroyHistoryButtons(void) {
  // remove History buttons and images from memory

  // next page button
  RemoveButton(giHistoryButton[1]);
  UnloadButtonImage(giHistoryButtonImage[1]);

  // prev page button
  RemoveButton(giHistoryButton[0]);
  UnloadButtonImage(giHistoryButtonImage[0]);

  return;
}

void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON *btn, int32_t reason) {
  // force redraw
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fReDrawScreenFlag = TRUE;
  }

  // force redraw
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    fReDrawScreenFlag = TRUE;
    btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    // this page is > 0, there are pages before it, decrement

    if (iCurrentHistoryPage > 0) {
      LoadPreviousHistoryPage();
      // iCurrentHistoryPage--;
      DrawAPageofHistoryRecords();
    }

    // set new state
    SetHistoryButtonStates();
  }
}

void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    fReDrawScreenFlag = TRUE;
  }

  // force redraw
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // increment currentPage
    btn->uiFlags &= ~(BUTTON_CLICKED_ON);
    LoadNextHistoryPage();
    // set new state
    SetHistoryButtonStates();
    fReDrawScreenFlag = TRUE;
  }
}

BOOLEAN IncrementCurrentPageHistoryDisplay(void) {
  // run through list, from pCurrentHistory, to NUM_RECORDS_PER_PAGE +1 HistoryUnits
  HWFILE hFileHandle;
  uint32_t uiFileSize = 0;
  uint32_t uiSizeOfRecordsOnEachPage = 0;

  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return (FALSE);

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    return (FALSE);
  }

  // make sure file is more than 0 length
  if (FileMan_GetSize(hFileHandle) == 0) {
    FileMan_Close(hFileHandle);
    return (FALSE);
  }

  uiFileSize = FileMan_GetSize(hFileHandle) - 1;
  uiSizeOfRecordsOnEachPage =
      (NUM_RECORDS_PER_PAGE * (sizeof(uint8_t) + sizeof(uint32_t) + 3 * sizeof(uint8_t) +
                               sizeof(int16_t) + sizeof(int16_t)));

  // is the file long enough?
  //  if( ( FileMan_GetSize( hFileHandle ) - 1 ) / ( NUM_RECORDS_PER_PAGE * ( sizeof( uint8_t ) +
  //  sizeof( uint32_t ) + 3*sizeof( uint8_t )+ sizeof(int16_t) + sizeof( int16_t ) ) ) + 1 < (
  //  uint32_t )( iCurrentHistoryPage + 1 ) )
  if (uiFileSize / uiSizeOfRecordsOnEachPage + 1 < (uint32_t)(iCurrentHistoryPage + 1)) {
    // nope
    FileMan_Close(hFileHandle);
    return (FALSE);
  } else {
    iCurrentHistoryPage++;
    FileMan_Close(hFileHandle);
  }

  // if ok to increment, increment

  return (TRUE);
}

uint32_t ProcessAndEnterAHistoryRecord(uint8_t ubCode, uint32_t uiDate, uint8_t ubSecondCode,
                                       uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ,
                                       uint8_t ubColor) {
  uint32_t uiId = 0;
  HistoryUnitPtr pHistory = pHistoryListHead;

  // add to History list
  if (pHistory) {
    // go to end of list
    while (pHistory->Next) pHistory = pHistory->Next;

    // alloc space
    pHistory->Next = (HistoryUnit *)MemAlloc(sizeof(HistoryUnit));

    // increment id number
    uiId = pHistory->uiIdNumber + 1;

    // set up information passed
    pHistory = pHistory->Next;
    pHistory->Next = NULL;
    pHistory->ubCode = ubCode;
    pHistory->ubSecondCode = ubSecondCode;
    pHistory->uiDate = uiDate;
    pHistory->uiIdNumber = uiId;
    pHistory->sSectorX = sSectorX;
    pHistory->sSectorY = sSectorY;
    pHistory->bSectorZ = bSectorZ;
    pHistory->ubColor = ubColor;

  } else {
    // alloc space
    pHistory = (HistoryUnit *)MemAlloc(sizeof(HistoryUnit));

    // setup info passed
    pHistory->Next = NULL;
    pHistory->ubCode = ubCode;
    pHistory->ubSecondCode = ubSecondCode;
    pHistory->uiDate = uiDate;
    pHistory->uiIdNumber = uiId;
    pHistoryListHead = pHistory;
    pHistory->sSectorX = sSectorX;
    pHistory->sSectorY = sSectorY;
    pHistory->bSectorZ = bSectorZ;
    pHistory->ubColor = ubColor;
  }

  return uiId;
}

void OpenAndReadHistoryFile(void) {
  // this procedure will open and read in data to the History list

  HWFILE hFileHandle;
  uint8_t ubCode, ubSecondCode;
  uint32_t uiDate;
  int16_t sSectorX, sSectorY;
  int8_t bSectorZ = 0;
  uint8_t ubColor;
  uint32_t iBytesRead = 0;
  uint32_t uiByteCount = 0;

  // clear out the old list
  ClearHistoryList();

  // no file, return
  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return;

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    return;
  }

  // make sure file is more than 0 length
  if (FileMan_GetSize(hFileHandle) == 0) {
    FileMan_Close(hFileHandle);
    return;
  }

  // file exists, read in data, continue until file end
  while (FileMan_GetSize(hFileHandle) > uiByteCount) {
    // read in other data
    FileMan_Read(hFileHandle, &ubCode, sizeof(uint8_t), &iBytesRead);
    FileMan_Read(hFileHandle, &ubSecondCode, sizeof(uint8_t), &iBytesRead);
    FileMan_Read(hFileHandle, &uiDate, sizeof(uint32_t), &iBytesRead);
    FileMan_Read(hFileHandle, &sSectorX, sizeof(int16_t), &iBytesRead);
    FileMan_Read(hFileHandle, &sSectorY, sizeof(int16_t), &iBytesRead);
    FileMan_Read(hFileHandle, &bSectorZ, sizeof(int8_t), &iBytesRead);
    FileMan_Read(hFileHandle, &ubColor, sizeof(uint8_t), &iBytesRead);

#ifdef JA2TESTVERSION
    // perform a check on the data to see if it is pooched
    PerformCheckOnHistoryRecord(1, (uint8_t)sSectorX, (uint8_t)sSectorY, bSectorZ);
#endif

    // add transaction
    ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, (uint8_t)sSectorX,
                                  (uint8_t)sSectorY, bSectorZ, ubColor);

    // increment byte counter
    uiByteCount += SIZE_OF_HISTORY_FILE_RECORD;
  }

  // close file
  FileMan_Close(hFileHandle);

  return;
}

BOOLEAN OpenAndWriteHistoryFile(void) {
  // this procedure will open and write out data from the History list

  HWFILE hFileHandle;
  HistoryUnitPtr pHistoryList = pHistoryListHead;

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE);

  // if no file exits, do nothing
  if (!hFileHandle) {
    return (FALSE);
  }
  // write info, while there are elements left in the list
  while (pHistoryList) {
#ifdef JA2TESTVERSION
    // perform a check on the data to see if it is pooched
    PerformCheckOnHistoryRecord(2, (uint8_t)pHistoryList->sSectorX, (uint8_t)pHistoryList->sSectorY,
                                pHistoryList->bSectorZ);
#endif

    // now write date and amount, and code
    FileMan_Write(hFileHandle, &(pHistoryList->ubCode), sizeof(uint8_t), NULL);
    FileMan_Write(hFileHandle, &(pHistoryList->ubSecondCode), sizeof(uint8_t), NULL);
    FileMan_Write(hFileHandle, &(pHistoryList->uiDate), sizeof(uint32_t), NULL);
    FileMan_Write(hFileHandle, &(pHistoryList->sSectorX), sizeof(int16_t), NULL);
    FileMan_Write(hFileHandle, &(pHistoryList->sSectorY), sizeof(int16_t), NULL);
    FileMan_Write(hFileHandle, &(pHistoryList->bSectorZ), sizeof(int8_t), NULL);
    FileMan_Write(hFileHandle, &(pHistoryList->ubColor), sizeof(uint8_t), NULL);

    // next element in list
    pHistoryList = pHistoryList->Next;
  }

  // close file
  FileMan_Close(hFileHandle);
  // clear out the old list
  ClearHistoryList();

  return (TRUE);
}

void ClearHistoryList(void) {
  // remove each element from list of transactions

  HistoryUnitPtr pHistoryList = pHistoryListHead;
  HistoryUnitPtr pHistoryNode = pHistoryList;

  // while there are elements in the list left, delete them
  while (pHistoryList) {
    // set node to list head
    pHistoryNode = pHistoryList;

    // set list head to next node
    pHistoryList = pHistoryList->Next;

    // delete current node
    MemFree(pHistoryNode);
  }
  pHistoryListHead = NULL;

  return;
}

void DisplayHistoryListHeaders(void) {
  // this procedure will display the headers to each column in History
  int16_t usX, usY;

  // font stuff
  SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // the date header
  FindFontCenterCoordinates(RECORD_DATE_X + 5, 0, RECORD_DATE_WIDTH, 0, pHistoryHeaders[0],
                            HISTORY_TEXT_FONT, &usX, &usY);
  mprintf(usX, RECORD_HEADER_Y, pHistoryHeaders[0]);

  // the date header
  FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH + 5, 0, RECORD_LOCATION_WIDTH, 0,
                            pHistoryHeaders[3], HISTORY_TEXT_FONT, &usX, &usY);
  mprintf(usX, RECORD_HEADER_Y, pHistoryHeaders[3]);

  // event header
  FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH + RECORD_LOCATION_WIDTH + 5, 0,
                            RECORD_LOCATION_WIDTH, 0, pHistoryHeaders[3], HISTORY_TEXT_FONT, &usX,
                            &usY);
  mprintf(usX, RECORD_HEADER_Y, pHistoryHeaders[4]);
  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);
  return;
}

void DisplayHistoryListBackground(void) {
  // this function will display the History list display background
  struct VObject *hHandle;
  int32_t iCounter = 0;

  // get shaded line object
  GetVideoObject(&hHandle, guiSHADELINE);
  for (iCounter = 0; iCounter < 11; iCounter++) {
    // blt title bar to screen
    BltVideoObject(vsFB, hHandle, 0, TOP_X + 15, (TOP_DIVLINE_Y + BOX_HEIGHT * 2 * iCounter));
  }

  // the long hortizontal line int he records list display region
  GetVideoObject(&hHandle, guiLONGLINE);
  BltVideoObject(vsFB, hHandle, 0, TOP_X + 9, (TOP_DIVLINE_Y));
  BltVideoObject(vsFB, hHandle, 0, TOP_X + 9, (TOP_DIVLINE_Y + BOX_HEIGHT * 2 * 11));

  return;
}

void DrawHistoryRecordsText(void) {
  // draws the text of the records
  HistoryUnitPtr pCurHistory = pHistoryListHead;
  wchar_t sString[512];
  int16_t usX, usY;
  int16_t sX = 0, sY = 0;

  // setup the font stuff
  SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // error check
  if (!pCurHistory) return;

  // loop through record list
  for (int iCounter = 0; iCounter < NUM_RECORDS_PER_PAGE; iCounter++) {
    if (pCurHistory->ubColor == 0) {
      SetFontForeground(FONT_BLACK);
    } else {
      SetFontForeground(FONT_RED);
    }
    // get and write the date
    swprintf(sString, ARR_SIZE(sString), L"%d", (pCurHistory->uiDate / (24 * 60)));
    FindFontCenterCoordinates(RECORD_DATE_X + 5, 0, RECORD_DATE_WIDTH, 0, sString,
                              HISTORY_TEXT_FONT, &usX, &usY);
    mprintf(usX, RECORD_Y + (iCounter * (BOX_HEIGHT)) + 3, sString);

    // now the actual history text
    // FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH,0,RECORD_HISTORY_WIDTH,0,
    // pHistoryStrings[pCurHistory->ubCode], HISTORY_TEXT_FONT,&usX, &usY);
    ProcessHistoryTransactionString(sString, ARR_SIZE(sString), pCurHistory);
    //	mprintf(RECORD_DATE_X + RECORD_DATE_WIDTH + 25, RECORD_Y + ( iCounter * ( BOX_HEIGHT ) ) +
    // 3, pHistoryStrings[pCurHistory->ubCode] );
    mprintf(RECORD_DATE_X + RECORD_LOCATION_WIDTH + RECORD_DATE_WIDTH + 15,
            RECORD_Y + (iCounter * (BOX_HEIGHT)) + 3, sString);

    // no location
    if ((pCurHistory->sSectorX == -1) || (pCurHistory->sSectorY == -1)) {
      FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH, 0, RECORD_LOCATION_WIDTH + 10, 0,
                                pHistoryLocations[0], HISTORY_TEXT_FONT, &sX, &sY);
      mprintf(sX, RECORD_Y + (iCounter * (BOX_HEIGHT)) + 3, pHistoryLocations[0]);
    } else {
      GetSectorIDString((uint8_t)pCurHistory->sSectorX, (uint8_t)pCurHistory->sSectorY,
                        pCurHistory->bSectorZ, sString, ARR_SIZE(sString), TRUE);
      FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH, 0, RECORD_LOCATION_WIDTH + 10, 0,
                                sString, HISTORY_TEXT_FONT, &sX, &sY);

      ReduceStringLength(sString, ARR_SIZE(sString), RECORD_LOCATION_WIDTH + 10, HISTORY_TEXT_FONT);

      mprintf(sX, RECORD_Y + (iCounter * (BOX_HEIGHT)) + 3, sString);
    }

    // restore font color
    SetFontForeground(FONT_BLACK);

    // next History
    pCurHistory = pCurHistory->Next;

    // last page, no Historys left, return
    if (!pCurHistory) {
      // restore shadow
      SetFontShadow(DEFAULT_SHADOW);
      return;
    }
  }

  // restore shadow
  SetFontShadow(DEFAULT_SHADOW);
  return;
}

void DrawAPageofHistoryRecords(void) {
  // this procedure will draw a series of history records to the screen
  pCurrentHistory = pHistoryListHead;

  // (re-)render background

  // the title bar text
  DrawHistoryTitleText();

  // the actual lists background
  DisplayHistoryListBackground();

  // the headers to each column
  DisplayHistoryListHeaders();

  // error check
  if (iCurrentHistoryPage == -1) {
    iCurrentHistoryPage = 0;
  }

  // current page is found, render  from here
  DrawHistoryRecordsText();

  // update page numbers, and date ranges
  DisplayPageNumberAndDateRange();

  return;
}

void DisplayPageNumberAndDateRange(void) {
  // this function will go through the list of 'histories' starting at current until end or
  // MAX_PER_PAGE...it will get the date range and the page number
  int32_t iLastPage = 0;
  int32_t iCounter = 0;
  uint32_t uiLastDate;
  HistoryUnitPtr pTempHistory = pHistoryListHead;
  wchar_t sString[50];

  // setup the font stuff
  SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  if (!pCurrentHistory) {
    swprintf(sString, ARR_SIZE(sString), L"%s  %d / %d", pHistoryHeaders[1], 1, 1);
    mprintf(PAGE_NUMBER_X, PAGE_NUMBER_Y, sString);

    swprintf(sString, ARR_SIZE(sString), L"%s %d - %d", pHistoryHeaders[2], 1, 1);
    mprintf(HISTORY_DATE_X, HISTORY_DATE_Y, sString);

    // reset shadow
    SetFontShadow(DEFAULT_SHADOW);

    return;
  }

  uiLastDate = pCurrentHistory->uiDate;

  iLastPage = GetNumberOfHistoryPages();

  // set temp to current, to get last date
  pTempHistory = pCurrentHistory;

  // reset counter
  iCounter = 0;

  // run through list until end or num_records, which ever first
  while ((pTempHistory) && (iCounter < NUM_RECORDS_PER_PAGE)) {
    uiLastDate = pTempHistory->uiDate;
    iCounter++;

    pTempHistory = pTempHistory->Next;
  }

  // get the last page

  swprintf(sString, ARR_SIZE(sString), L"%s  %d / %d", pHistoryHeaders[1], iCurrentHistoryPage,
           iLastPage + 1);
  mprintf(PAGE_NUMBER_X, PAGE_NUMBER_Y, sString);

  swprintf(sString, ARR_SIZE(sString), L"%s %d - %d", pHistoryHeaders[2],
           pCurrentHistory->uiDate / (24 * 60), uiLastDate / (24 * 60));
  mprintf(HISTORY_DATE_X, HISTORY_DATE_Y, sString);

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void ProcessHistoryTransactionString(wchar_t *pString, size_t bufSize, HistoryUnitPtr pHistory) {
  wchar_t sString[128];

  switch (pHistory->ubCode) {
    case HISTORY_ENTERED_HISTORY_MODE:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_ENTERED_HISTORY_MODE]);
      break;

    case HISTORY_HIRED_MERC_FROM_AIM:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_HIRED_MERC_FROM_AIM],
               gMercProfiles[pHistory->ubSecondCode].zName);
      break;

    case HISTORY_MERC_KILLED:
      if (pHistory->ubSecondCode != NO_PROFILE)
        swprintf(pString, bufSize, pHistoryStrings[HISTORY_MERC_KILLED],
                 gMercProfiles[pHistory->ubSecondCode].zName);
#ifdef JA2BETAVERSION
      else {
        swprintf(pString, bufSize, pHistoryStrings[HISTORY_MERC_KILLED], L"ERROR!!!  NO_PROFILE");
      }
#endif
      break;

    case HISTORY_HIRED_MERC_FROM_MERC:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_HIRED_MERC_FROM_MERC],
               gMercProfiles[pHistory->ubSecondCode].zName);
      break;

    case HISTORY_SETTLED_ACCOUNTS_AT_MERC:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_SETTLED_ACCOUNTS_AT_MERC]);
      break;
    case HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO]);
      break;
    case (HISTORY_CHARACTER_GENERATED):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_CHARACTER_GENERATED]);
      break;
    case (HISTORY_PURCHASED_INSURANCE):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_PURCHASED_INSURANCE],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;
    case (HISTORY_CANCELLED_INSURANCE):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_CANCELLED_INSURANCE],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;
    case (HISTORY_INSURANCE_CLAIM_PAYOUT):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_INSURANCE_CLAIM_PAYOUT],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    case HISTORY_EXTENDED_CONTRACT_1_DAY:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_EXTENDED_CONTRACT_1_DAY],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    case HISTORY_EXTENDED_CONTRACT_1_WEEK:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_EXTENDED_CONTRACT_1_WEEK],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    case HISTORY_EXTENDED_CONTRACT_2_WEEK:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_EXTENDED_CONTRACT_2_WEEK],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    case (HISTORY_MERC_FIRED):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_MERC_FIRED],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    case (HISTORY_MERC_QUIT):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_MERC_QUIT],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    case (HISTORY_QUEST_STARTED):
      GetQuestStartedString(pHistory->ubSecondCode, sString);
      swprintf(pString, bufSize, sString);

      break;
    case (HISTORY_QUEST_FINISHED):
      GetQuestEndedString(pHistory->ubSecondCode, sString);
      swprintf(pString, bufSize, sString);

      break;
    case (HISTORY_TALKED_TO_MINER):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_TALKED_TO_MINER],
               pTownNames[pHistory->ubSecondCode]);
      break;
    case (HISTORY_LIBERATED_TOWN):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_LIBERATED_TOWN],
               pTownNames[pHistory->ubSecondCode]);
      break;
    case (HISTORY_CHEAT_ENABLED):
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_CHEAT_ENABLED]);
      break;
    case HISTORY_TALKED_TO_FATHER_WALKER:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_TALKED_TO_FATHER_WALKER]);
      break;
    case HISTORY_MERC_MARRIED_OFF:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_MERC_MARRIED_OFF],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;
    case HISTORY_MERC_CONTRACT_EXPIRED:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_MERC_CONTRACT_EXPIRED],
               gMercProfiles[pHistory->ubSecondCode].zName);
      break;
    case HISTORY_RPC_JOINED_TEAM:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_RPC_JOINED_TEAM],
               gMercProfiles[pHistory->ubSecondCode].zName);
      break;
    case HISTORY_ENRICO_COMPLAINED:
      swprintf(pString, bufSize, pHistoryStrings[HISTORY_ENRICO_COMPLAINED]);
      break;
    case HISTORY_MINE_RUNNING_OUT:
    case HISTORY_MINE_RAN_OUT:
    case HISTORY_MINE_SHUTDOWN:
    case HISTORY_MINE_REOPENED:
      // all the same format
      swprintf(pString, bufSize, pHistoryStrings[pHistory->ubCode],
               pTownNames[pHistory->ubSecondCode]);
      break;
    case HISTORY_LOST_BOXING:
    case HISTORY_WON_BOXING:
    case HISTORY_DISQUALIFIED_BOXING:
    case HISTORY_NPC_KILLED:
    case HISTORY_MERC_KILLED_CHARACTER:
      swprintf(pString, bufSize, pHistoryStrings[pHistory->ubCode],
               gMercProfiles[pHistory->ubSecondCode].zNickname);
      break;

    // ALL SIMPLE HISTORY LOG MSGS, NO PARAMS
    case HISTORY_FOUND_MONEY:
    case HISTORY_ASSASSIN:
    case HISTORY_DISCOVERED_TIXA:
    case HISTORY_DISCOVERED_ORTA:
    case HISTORY_GOT_ROCKET_RIFLES:
    case HISTORY_DEIDRANNA_DEAD_BODIES:
    case HISTORY_BOXING_MATCHES:
    case HISTORY_SOMETHING_IN_MINES:
    case HISTORY_DEVIN:
    case HISTORY_MIKE:
    case HISTORY_TONY:
    case HISTORY_KROTT:
    case HISTORY_KYLE:
    case HISTORY_MADLAB:
    case HISTORY_GABBY:
    case HISTORY_KEITH_OUT_OF_BUSINESS:
    case HISTORY_HOWARD_CYANIDE:
    case HISTORY_KEITH:
    case HISTORY_HOWARD:
    case HISTORY_PERKO:
    case HISTORY_SAM:
    case HISTORY_FRANZ:
    case HISTORY_ARNOLD:
    case HISTORY_FREDO:
    case HISTORY_RICHGUY_BALIME:
    case HISTORY_JAKE:
    case HISTORY_BUM_KEYCARD:
    case HISTORY_WALTER:
    case HISTORY_DAVE:
    case HISTORY_PABLO:
    case HISTORY_KINGPIN_MONEY:
    // VARIOUS BATTLE CONDITIONS
    case HISTORY_LOSTTOWNSECTOR:
    case HISTORY_DEFENDEDTOWNSECTOR:
    case HISTORY_LOSTBATTLE:
    case HISTORY_WONBATTLE:
    case HISTORY_FATALAMBUSH:
    case HISTORY_WIPEDOUTENEMYAMBUSH:
    case HISTORY_UNSUCCESSFULATTACK:
    case HISTORY_SUCCESSFULATTACK:
    case HISTORY_CREATURESATTACKED:
    case HISTORY_KILLEDBYBLOODCATS:
    case HISTORY_SLAUGHTEREDBLOODCATS:
    case HISTORY_GAVE_CARMEN_HEAD:
    case HISTORY_SLAY_MYSTERIOUSLY_LEFT:
      swprintf(pString, bufSize, pHistoryStrings[pHistory->ubCode]);
      break;
  }
}

void DrawHistoryLocation(uint8_t sSectorX, uint8_t sSectorY) {
  // will draw the location of the history event

  return;
}

void SetHistoryButtonStates(void) {
  // this function will look at what page we are viewing, enable and disable buttons as needed

  if (iCurrentHistoryPage == 1) {
    // first page, disable left buttons
    DisableButton(giHistoryButton[PREV_PAGE_BUTTON]);

  } else {
    // enable buttons
    EnableButton(giHistoryButton[PREV_PAGE_BUTTON]);
  }

  if (IncrementCurrentPageHistoryDisplay()) {
    // decrement page
    iCurrentHistoryPage--;
    DrawAPageofHistoryRecords();

    // enable buttons
    EnableButton(giHistoryButton[NEXT_PAGE_BUTTON]);

  } else {
    DisableButton(giHistoryButton[NEXT_PAGE_BUTTON]);
  }
}

BOOLEAN LoadInHistoryRecords(uint32_t uiPage) {
  // loads in records belogning, to page uiPage
  // no file, return
  BOOLEAN fOkToContinue = TRUE;
  int32_t iCount = 0;
  HWFILE hFileHandle;
  uint8_t ubCode, ubSecondCode;
  int16_t sSectorX, sSectorY;
  int8_t bSectorZ;
  uint32_t uiDate;
  uint8_t ubColor;
  uint32_t iBytesRead = 0;
  uint32_t uiByteCount = 0;

  // check if bad page
  if (uiPage == 0) {
    return (FALSE);
  }

  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return (FALSE);

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    return (FALSE);
  }

  // make sure file is more than 0 length
  if (FileMan_GetSize(hFileHandle) == 0) {
    FileMan_Close(hFileHandle);
    return (FALSE);
  }

  // is the file long enough?
  if ((FileMan_GetSize(hFileHandle) - 1) / (NUM_RECORDS_PER_PAGE * SIZE_OF_HISTORY_FILE_RECORD) +
          1 <
      uiPage) {
    // nope
    FileMan_Close(hFileHandle);
    return (FALSE);
  }

  FileMan_Seek(hFileHandle, (uiPage - 1) * NUM_RECORDS_PER_PAGE * (SIZE_OF_HISTORY_FILE_RECORD),
               FILE_SEEK_FROM_START);

  uiByteCount = (uiPage - 1) * NUM_RECORDS_PER_PAGE * (SIZE_OF_HISTORY_FILE_RECORD);
  // file exists, read in data, continue until end of page
  while ((iCount < NUM_RECORDS_PER_PAGE) && (fOkToContinue)) {
    // read in other data
    FileMan_Read(hFileHandle, &ubCode, sizeof(uint8_t), &iBytesRead);
    FileMan_Read(hFileHandle, &ubSecondCode, sizeof(uint8_t), &iBytesRead);
    FileMan_Read(hFileHandle, &uiDate, sizeof(uint32_t), &iBytesRead);
    FileMan_Read(hFileHandle, &sSectorX, sizeof(int16_t), &iBytesRead);
    FileMan_Read(hFileHandle, &sSectorY, sizeof(int16_t), &iBytesRead);
    FileMan_Read(hFileHandle, &bSectorZ, sizeof(int8_t), &iBytesRead);
    FileMan_Read(hFileHandle, &ubColor, sizeof(uint8_t), &iBytesRead);

#ifdef JA2TESTVERSION
    // perform a check on the data to see if it is pooched
    PerformCheckOnHistoryRecord(3, (uint8_t)sSectorX, (uint8_t)sSectorY, bSectorZ);
#endif

    // add transaction
    ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, (uint8_t)sSectorX,
                                  (uint8_t)sSectorY, bSectorZ, ubColor);

    // increment byte counter
    uiByteCount += SIZE_OF_HISTORY_FILE_RECORD;

    // we've overextended our welcome, and bypassed end of file, get out
    if (uiByteCount >= FileMan_GetSize(hFileHandle)) {
      // not ok to continue
      fOkToContinue = FALSE;
    }

    iCount++;
  }

  // close file
  FileMan_Close(hFileHandle);

  // check to see if we in fact have a list to display
  if (pHistoryListHead == NULL) {
    // got no records, return false
    return (FALSE);
  }

  // set up current finance
  pCurrentHistory = pHistoryListHead;

  return (TRUE);
}

BOOLEAN WriteOutHistoryRecords(uint32_t uiPage) {
  // loads in records belogning, to page uiPage
  // no file, return
  BOOLEAN fOkToContinue = TRUE;
  int32_t iCount = 0;
  HWFILE hFileHandle;
  HistoryUnitPtr pList;

  // check if bad page
  if (uiPage == 0) {
    return (FALSE);
  }

  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return (FALSE);

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_WRITE), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    return (FALSE);
  }

  // make sure file is more than 0 length
  if (FileMan_GetSize(hFileHandle) == 0) {
    FileMan_Close(hFileHandle);
    return (FALSE);
  }

  // is the file long enough?
  if ((FileMan_GetSize(hFileHandle) - 1) / (NUM_RECORDS_PER_PAGE * SIZE_OF_HISTORY_FILE_RECORD) +
          1 <
      uiPage) {
    // nope
    FileMan_Close(hFileHandle);
    return (FALSE);
  }

  pList = pHistoryListHead;

  if (pList == NULL) {
    return (FALSE);
  }

  FileMan_Seek(hFileHandle,
               sizeof(int32_t) + (uiPage - 1) * NUM_RECORDS_PER_PAGE * SIZE_OF_HISTORY_FILE_RECORD,
               FILE_SEEK_FROM_START);

  // file exists, read in data, continue until end of page
  while ((iCount < NUM_RECORDS_PER_PAGE) && (fOkToContinue)) {
#ifdef JA2TESTVERSION
    // perform a check on the data to see if it is pooched
    PerformCheckOnHistoryRecord(4, (uint8_t)pList->sSectorX, (uint8_t)pList->sSectorY,
                                pList->bSectorZ);
#endif

    FileMan_Write(hFileHandle, &(pList->ubCode), sizeof(uint8_t), NULL);
    FileMan_Write(hFileHandle, &(pList->ubSecondCode), sizeof(uint8_t), NULL);
    FileMan_Write(hFileHandle, &(pList->uiDate), sizeof(uint32_t), NULL);
    FileMan_Write(hFileHandle, &(pList->sSectorX), sizeof(int16_t), NULL);
    FileMan_Write(hFileHandle, &(pList->sSectorY), sizeof(int16_t), NULL);
    FileMan_Write(hFileHandle, &(pList->bSectorZ), sizeof(int8_t), NULL);
    FileMan_Write(hFileHandle, &(pList->ubColor), sizeof(uint8_t), NULL);

    pList = pList->Next;

    // we've overextended our welcome, and bypassed end of file, get out
    if (pList == NULL) {
      // not ok to continue
      fOkToContinue = FALSE;
    }

    iCount++;
  }

  // close file
  FileMan_Close(hFileHandle);

  ClearHistoryList();

  return (TRUE);
}

BOOLEAN LoadNextHistoryPage(void) {
  // clear out old list of records, and load in previous page worth of records
  ClearHistoryList();

  // now load in previous page's records, if we can
  if (LoadInHistoryRecords(iCurrentHistoryPage + 1)) {
    iCurrentHistoryPage++;
    return (TRUE);
  } else {
    LoadInHistoryRecords(iCurrentHistoryPage);
    return (FALSE);
  }
}

BOOLEAN LoadPreviousHistoryPage(void) {
  // clear out old list of records, and load in previous page worth of records
  ClearHistoryList();

  // load previous page
  if ((iCurrentHistoryPage == 1)) {
    return (FALSE);
  }

  // now load in previous page's records, if we can
  if (LoadInHistoryRecords(iCurrentHistoryPage - 1)) {
    iCurrentHistoryPage--;
    return (TRUE);
  } else {
    LoadInHistoryRecords(iCurrentHistoryPage);
    return (FALSE);
  }
}

void SetLastPageInHistoryRecords(void) {
  // grabs the size of the file and interprets number of pages it will take up
  HWFILE hFileHandle;

  // no file, return
  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return;

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    guiLastPageInHistoryRecordsList = 1;
    return;
  }

  // make sure file is more than 0 length
  if (FileMan_GetSize(hFileHandle) == 0) {
    FileMan_Close(hFileHandle);
    guiLastPageInHistoryRecordsList = 1;
    return;
  }

  // done with file, close it
  FileMan_Close(hFileHandle);

  guiLastPageInHistoryRecordsList =
      ReadInLastElementOfHistoryListAndReturnIdNumber() / NUM_RECORDS_PER_PAGE;

  return;
}

uint32_t ReadInLastElementOfHistoryListAndReturnIdNumber(void) {
  // this function will read in the last unit in the history list, to grab it's id number

  HWFILE hFileHandle;
  int32_t iFileSize = 0;

  // no file, return
  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return 0;

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    return 0;
  }

  // make sure file is more than balance size + length of 1 record - 1 byte
  if (FileMan_GetSize(hFileHandle) < SIZE_OF_HISTORY_FILE_RECORD) {
    FileMan_Close(hFileHandle);
    return 0;
  }

  // size is?
  iFileSize = FileMan_GetSize(hFileHandle);

  // done with file, close it
  FileMan_Close(hFileHandle);

  // file size  / sizeof record in bytes is id
  return ((iFileSize) / (SIZE_OF_HISTORY_FILE_RECORD));
}

BOOLEAN AppendHistoryToEndOfFile(HistoryUnitPtr pHistory) {
  // will write the current finance to disk
  HWFILE hFileHandle;
  HistoryUnitPtr pHistoryList = pHistoryListHead;

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, FILE_ACCESS_WRITE | FILE_OPEN_ALWAYS, FALSE);

  // if no file exits, do nothing
  if (!hFileHandle) {
    return (FALSE);
  }

  // go to the end
  if (FileMan_Seek(hFileHandle, 0, FILE_SEEK_FROM_END) == FALSE) {
    // error
    FileMan_Close(hFileHandle);
    return (FALSE);
  }

#ifdef JA2TESTVERSION
  // perform a check on the data to see if it is pooched
  PerformCheckOnHistoryRecord(5, (uint8_t)pHistoryList->sSectorX, (uint8_t)pHistoryList->sSectorY,
                              pHistoryList->bSectorZ);
#endif

  // now write date and amount, and code
  FileMan_Write(hFileHandle, &(pHistoryList->ubCode), sizeof(uint8_t), NULL);
  FileMan_Write(hFileHandle, &(pHistoryList->ubSecondCode), sizeof(uint8_t), NULL);
  FileMan_Write(hFileHandle, &(pHistoryList->uiDate), sizeof(uint32_t), NULL);
  FileMan_Write(hFileHandle, &(pHistoryList->sSectorX), sizeof(int16_t), NULL);
  FileMan_Write(hFileHandle, &(pHistoryList->sSectorY), sizeof(int16_t), NULL);
  FileMan_Write(hFileHandle, &(pHistoryList->bSectorZ), sizeof(int8_t), NULL);
  FileMan_Write(hFileHandle, &(pHistoryList->ubColor), sizeof(uint8_t), NULL);

  // close file
  FileMan_Close(hFileHandle);

  return (TRUE);
}

void ResetHistoryFact(uint8_t ubCode, uint8_t sSectorX, uint8_t sSectorY) {
  // run through history list
  HistoryUnitPtr pList = pHistoryListHead;
  BOOLEAN fFound = FALSE;

  // set current page to before list
  iCurrentHistoryPage = 0;

  SetLastPageInHistoryRecords();

  OpenAndReadHistoryFile();

  pList = pHistoryListHead;

  while (pList) {
    if ((pList->ubSecondCode == ubCode) && (pList->ubCode == HISTORY_QUEST_STARTED)) {
      // reset color
      pList->ubColor = 0;
      fFound = TRUE;

      // save
      OpenAndWriteHistoryFile();
      pList = NULL;
    }

    if (fFound != TRUE) {
      pList = pList->Next;
    }
  }

  if (fInHistoryMode) {
    iCurrentHistoryPage--;

    // load in first page
    LoadNextHistoryPage();
  }

  SetHistoryFact(HISTORY_QUEST_FINISHED, ubCode, GetWorldTotalMin(), sSectorX, sSectorY);
  return;
}

uint32_t GetTimeQuestWasStarted(uint8_t ubCode) {
  // run through history list
  HistoryUnitPtr pList = pHistoryListHead;
  BOOLEAN fFound = FALSE;
  uint32_t uiTime = 0;

  // set current page to before list
  iCurrentHistoryPage = 0;

  SetLastPageInHistoryRecords();

  OpenAndReadHistoryFile();

  pList = pHistoryListHead;

  while (pList) {
    if ((pList->ubSecondCode == ubCode) && (pList->ubCode == HISTORY_QUEST_STARTED)) {
      uiTime = pList->uiDate;
      fFound = TRUE;

      pList = NULL;
    }

    if (fFound != TRUE) {
      pList = pList->Next;
    }
  }

  if (fInHistoryMode) {
    iCurrentHistoryPage--;

    // load in first page
    LoadNextHistoryPage();
  }

  return (uiTime);
}

void GetQuestStartedString(uint8_t ubQuestValue, wchar_t *sQuestString) {
  // open the file and copy the string
  LoadEncryptedDataFromFile("BINARYDATA\\quests.edt", sQuestString, 160 * (ubQuestValue * 2), 160);
}

void GetQuestEndedString(uint8_t ubQuestValue, wchar_t *sQuestString) {
  // open the file and copy the string
  LoadEncryptedDataFromFile("BINARYDATA\\quests.edt", sQuestString, 160 * ((ubQuestValue * 2) + 1),
                            160);
}

#ifdef JA2TESTVERSION
void PerformCheckOnHistoryRecord(uint32_t uiErrorCode, uint8_t sSectorX, uint8_t sSectorY,
                                 int8_t bSectorZ) {
  char zString[512];

  if (sSectorX > 16 || sSectorY > 16 || bSectorZ > 3 || sSectorX < -1 || sSectorY < -1 ||
      bSectorZ < 0) {
    sprintf(zString,
            "History page is pooched, please remember what you were just doing, send your latest "
            "save to dave, and tell him this number, Error #%d.",
            uiErrorCode);
    AssertMsg(0, zString);
  }
}
#endif

int32_t GetNumberOfHistoryPages() {
  HWFILE hFileHandle;
  uint32_t uiFileSize = 0;
  uint32_t uiSizeOfRecordsOnEachPage = 0;
  int32_t iNumberOfHistoryPages = 0;

  if (!(FileMan_Exists(HISTORY_DATA_FILE))) return (0);

  // open file
  hFileHandle = FileMan_Open(HISTORY_DATA_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

  // failed to get file, return
  if (!hFileHandle) {
    return (0);
  }

  // make sure file is more than 0 length
  if (FileMan_GetSize(hFileHandle) == 0) {
    FileMan_Close(hFileHandle);
    return (0);
  }

  uiFileSize = FileMan_GetSize(hFileHandle) - 1;
  uiSizeOfRecordsOnEachPage =
      (NUM_RECORDS_PER_PAGE * (sizeof(uint8_t) + sizeof(uint32_t) + 3 * sizeof(uint8_t) +
                               sizeof(int16_t) + sizeof(int16_t)));

  iNumberOfHistoryPages = (int32_t)(uiFileSize / uiSizeOfRecordsOnEachPage);

  FileMan_Close(hFileHandle);

  return (iNumberOfHistoryPages);
}
