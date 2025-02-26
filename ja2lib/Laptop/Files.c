// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/Files.h"

#include <stdio.h>
#include <string.h>

#include "Laptop/Email.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/WCheck.h"
#include "Strategic/GameClock.h"
#include "Utils/Cursors.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"
#include "platform.h"

#define TOP_X 0 + LAPTOP_SCREEN_UL_X
#define TOP_Y LAPTOP_SCREEN_UL_Y
#define BLOCK_FILE_HEIGHT 10
#define BOX_HEIGHT 14
#define TITLE_X 140
#define TITLE_Y 33
#define TEXT_X 140
#define PAGE_SIZE 22
#define FILES_TITLE_FONT FONT14ARIAL
#define FILES_TEXT_FONT FONT10ARIAL  // FONT12ARIAL
#define BLOCK_HEIGHT 10
#define FILES_SENDER_TEXT_X TOP_X + 15
#define MAX_FILES_LIST_LENGTH 28
#define NUMBER_OF_FILES_IN_FILE_MANAGER 20
#define FILE_VIEWER_X 236
#define FILE_VIEWER_Y 85
#define FILE_VIEWER_WIDTH 598 - 240
#define FILE_GAP 2
#define FILE_TEXT_COLOR FONT_BLACK
#define FILE_STRING_SIZE 400
#define MAX_FILES_PAGE MAX_FILES_LIST_LENGTH
#define FILES_LIST_X FILES_SENDER_TEXT_X
#define FILES_LIST_Y (9 * BLOCK_HEIGHT)
#define FILES_LIST_WIDTH 100
#define LENGTH_OF_ENRICO_FILE 68
#define MAX_FILE_MESSAGE_PAGE_SIZE 325
#define VIEWER_MESSAGE_BODY_START_Y FILES_LIST_Y
#define PREVIOUS_FILE_PAGE_BUTTON_X 553
#define PREVIOUS_FILE_PAGE_BUTTON_Y 53
#define NEXT_FILE_PAGE_BUTTON_X 577
#define NEXT_FILE_PAGE_BUTTON_Y PREVIOUS_FILE_PAGE_BUTTON_Y

#define FILES_COUNTER_1_WIDTH 7
#define FILES_COUNTER_2_WIDTH 43
#define FILES_COUNTER_3_WIDTH 45

// the highlighted line
int32_t iHighLightFileLine = -1;

// the files record list
FilesUnitPtr pFilesListHead = NULL;

FileStringPtr pFileStringList = NULL;

// are we in files mode
BOOLEAN fInFilesMode = FALSE;
BOOLEAN fOnLastFilesPageFlag = FALSE;

//. did we enter due to new file icon?
BOOLEAN fEnteredFileViewerFromNewFileIcon = FALSE;
BOOLEAN fWaitAFrame = FALSE;

// are there any new files
BOOLEAN fNewFilesInFileViewer = FALSE;

// graphics handles
uint32_t guiTITLE;
uint32_t guiFileBack;
uint32_t guiTOP;
uint32_t guiHIGHLIGHT;

// currewnt page of multipage files we are on
int32_t giFilesPage = 0;
// strings

#define SLAY_LENGTH 12
#define ENRICO_LENGTH 0

uint8_t ubFileRecordsLength[] = {
    ENRICO_LENGTH, SLAY_LENGTH, SLAY_LENGTH, SLAY_LENGTH, SLAY_LENGTH, SLAY_LENGTH, SLAY_LENGTH,
};

uint16_t ubFileOffsets[] = {
    0,
    ENRICO_LENGTH,
    SLAY_LENGTH + ENRICO_LENGTH,
    2 * SLAY_LENGTH + ENRICO_LENGTH,
    3 * SLAY_LENGTH + ENRICO_LENGTH,
    4 * SLAY_LENGTH + ENRICO_LENGTH,
    5 * SLAY_LENGTH + ENRICO_LENGTH,
};

uint16_t usProfileIdsForTerroristFiles[] = {
    0,    // no body
    112,  // elgin
    64,   // slay
    82,   // mom
    83,   // imposter
    110,  // tiff
    111,  // t-rex
    112,  // elgin
};
// buttons for next and previous pages
uint32_t giFilesPageButtons[2];
uint32_t giFilesPageButtonsImage[2];

// the previous and next pages buttons

enum {
  PREVIOUS_FILES_PAGE_BUTTON = 0,
  NEXT_FILES_PAGE_BUTTON,
};
// mouse regions
struct MOUSE_REGION pFilesRegions[MAX_FILES_PAGE];

// function definitions
void RenderFilesBackGround(void);
BOOLEAN LoadFiles(void);
void RemoveFiles(void);
uint32_t ProcessAndEnterAFilesRecord(uint8_t ubCode, uint32_t uiDate, uint8_t ubFormat,
                                     char *pFirstPicFile, char *pSecondPicFile, BOOLEAN fRead);
void OpenAndReadFilesFile(void);
BOOLEAN OpenAndWriteFilesFile(void);
void ClearFilesList(void);
void DrawFilesTitleText(void);
void DrawFilesListBackGround(void);
void DisplayFilesList(void);
BOOLEAN OpenAndWriteFilesFile(void);
void DisplayFileMessage(void);
void InitializeFilesMouseRegions(void);
void RemoveFilesMouseRegions(void);
BOOLEAN DisplayFormattedText(void);

// buttons
void CreateButtonsForFilesPage(void);
void DeleteButtonsForFilesPage(void);
void HandleFileViewerButtonStates(void);

// open new files for viewing
void OpenFirstUnreadFile(void);
void CheckForUnreadFiles(void);

// file string structure manipulations
void ClearFileStringList(void);
void AddStringToFilesList(wchar_t *pString);
BOOLEAN HandleSpecialFiles(uint8_t ubFormat);
BOOLEAN HandleSpecialTerroristFile(int32_t iFileNumber, char *sPictureName);

// callbacks
void FilesBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void BtnPreviousFilePageCallback(GUI_BUTTON *btn, int32_t reason);
void BtnNextFilePageCallback(GUI_BUTTON *btn, int32_t reason);

// file width manipulation
void ClearOutWidthRecordsList(FileRecordWidthPtr pFileRecordWidthList);
FileRecordWidthPtr CreateWidthRecordsForAruloIntelFile(void);
FileRecordWidthPtr CreateWidthRecordsForTerroristFile(void);
FileRecordWidthPtr CreateRecordWidth(int32_t iRecordNumber, int32_t iRecordWidth,
                                     int32_t iRecordHeightAdjustment, uint8_t ubFlags);

uint32_t AddFilesToPlayersLog(uint8_t ubCode, uint32_t uiDate, uint8_t ubFormat,
                              char *pFirstPicFile, char *pSecondPicFile) {
  // adds Files item to player's log(Files List), returns unique id number of it
  // outside of the Files system(the code in this .c file), this is the only function you'll ever
  // need
  uint32_t uiId = 0;

  // if not in Files mode, read in from file
  if (!fInFilesMode) OpenAndReadFilesFile();

  // process the actual data
  uiId =
      ProcessAndEnterAFilesRecord(ubCode, uiDate, ubFormat, pFirstPicFile, pSecondPicFile, FALSE);

  // set unread flag, if nessacary
  CheckForUnreadFiles();

  // write out to file if not in Files mode
  if (!fInFilesMode) OpenAndWriteFilesFile();

  // return unique id of this transaction
  return uiId;
}
void GameInitFiles() {
  if ((FileMan_Exists(FILES_DAT_FILE) == TRUE)) {
    Plat_ClearFileAttributes(FILES_DAT_FILE);
    FileMan_Delete(FILES_DAT_FILE);
  }

  ClearFilesList();

  // add background check by RIS
  AddFilesToPlayersLog(ENRICO_BACKGROUND, 0, 255, NULL, NULL);
}

void EnterFiles() {
  // load grpahics for files system
  LoadFiles();

  // AddFilesToPlayersLog(1, 0, 0,"LAPTOP\\portrait.sti", "LAPTOP\\portrait.sti");
  // AddFilesToPlayersLog(0, 0, 3,"LAPTOP\\portrait.sti", "LAPTOP\\portrait.sti");
  // AddFilesToPlayersLog(2, 0, 1,"LAPTOP\\portrait.sti", "LAPTOP\\portrait.sti");
  // in files mode now, set the fact
  fInFilesMode = TRUE;

  // initialize mouse regions
  InitializeFilesMouseRegions();

  // create buttons
  CreateButtonsForFilesPage();

  // now set start states
  HandleFileViewerButtonStates();

  // build files list
  OpenAndReadFilesFile();

  // render files system
  RenderFiles();

  // entered due to icon
  if (fEnteredFileViewerFromNewFileIcon == TRUE) {
    OpenFirstUnreadFile();
    fEnteredFileViewerFromNewFileIcon = FALSE;
  }
}

void ExitFiles() {
  // write files list out to disk
  OpenAndWriteFilesFile();

  // remove mouse regions
  RemoveFilesMouseRegions();

  // delete buttons
  DeleteButtonsForFilesPage();

  fInFilesMode = FALSE;

  // remove files
  RemoveFiles();
}

void HandleFiles() { CheckForUnreadFiles(); }

void RenderFiles() {
  struct VObject *hHandle;

  // render the background
  RenderFilesBackGround();

  // draw the title bars text
  DrawFilesTitleText();

  // the columns
  DrawFilesListBackGround();

  // display the list of senders
  DisplayFilesList();

  // draw the highlighted file
  DisplayFileMessage();

  // title bar icon
  BlitTitleBarIcons();

  // display border
  GetVideoObject(&hHandle, guiLaptopBACKGROUND);
  BltVideoObject(vsFB, hHandle, 0, 108, 23);
}

void RenderFilesBackGround(void) {
  // render generic background for file system
  struct VObject *hHandle;

  // get title bar object
  GetVideoObject(&hHandle, guiTITLE);

  // blt title bar to screen
  BltVideoObject(vsFB, hHandle, 0, TOP_X, TOP_Y - 2);

  // get and blt the top part of the screen, video object and blt to screen
  GetVideoObject(&hHandle, guiTOP);
  BltVideoObject(vsFB, hHandle, 0, TOP_X, TOP_Y + 22);

  return;
}

void DrawFilesTitleText(void) {
  // setup the font stuff
  SetFont(FILES_TITLE_FONT);
  SetFontForeground(FONT_WHITE);
  SetFontBackground(FONT_BLACK);
  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  // draw the pages title
  mprintf(TITLE_X, TITLE_Y, pFilesTitle[0]);

  return;
}

BOOLEAN LoadFiles(void) {
  // load files video objects into memory

  // title bar
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\programtitlebar.sti"), &guiTITLE));

  // top portion of the screen background
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\fileviewer.sti"), &guiTOP));

  // the highlight
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\highlight.sti"), &guiHIGHLIGHT));

  // top portion of the screen background
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\fileviewerwhite.sti"), &guiFileBack));

  return (TRUE);
}

void RemoveFiles(void) {
  // delete files video objects from memory

  DeleteVideoObjectFromIndex(guiTOP);
  DeleteVideoObjectFromIndex(guiTITLE);
  DeleteVideoObjectFromIndex(guiHIGHLIGHT);
  DeleteVideoObjectFromIndex(guiFileBack);

  return;
}

uint32_t ProcessAndEnterAFilesRecord(uint8_t ubCode, uint32_t uiDate, uint8_t ubFormat,
                                     char *pFirstPicFile, char *pSecondPicFile, BOOLEAN fRead) {
  uint32_t uiId = 0;
  FilesUnitPtr pFiles = pFilesListHead;

  // add to Files list
  if (pFiles) {
    while (pFiles) {
      // check to see if the file is already there
      if (pFiles->ubCode == ubCode) {
        // if so, return it's id number
        return (pFiles->uiIdNumber);
      }

      // next in the list
      pFiles = pFiles->Next;
    }

    // reset pointer
    pFiles = pFilesListHead;

    // go to end of list
    while (pFiles->Next) {
      pFiles = pFiles->Next;
    }
    // alloc space
    pFiles->Next = (FilesUnit *)MemAlloc(sizeof(FilesUnit));

    // increment id number
    uiId = pFiles->uiIdNumber + 1;

    // set up information passed
    pFiles = pFiles->Next;
    pFiles->Next = NULL;
    pFiles->ubCode = ubCode;
    pFiles->uiDate = uiDate;
    pFiles->uiIdNumber = uiId;
    pFiles->ubFormat = ubFormat;
    pFiles->fRead = fRead;
  } else {
    // alloc space
    pFiles = (FilesUnit *)MemAlloc(sizeof(FilesUnit));

    // setup info passed
    pFiles->Next = NULL;
    pFiles->ubCode = ubCode;
    pFiles->uiDate = uiDate;
    pFiles->uiIdNumber = uiId;
    pFilesListHead = pFiles;
    pFiles->ubFormat = ubFormat;
    pFiles->fRead = fRead;
  }

  // null out ptr's to picture file names
  pFiles->pPicFileNameList[0] = NULL;
  pFiles->pPicFileNameList[1] = NULL;

  // copy file name strings

  // first file
  if (pFirstPicFile) {
    if ((pFirstPicFile[0]) != 0) {
      pFiles->pPicFileNameList[0] = (char *)MemAlloc(strlen(pFirstPicFile) + 1);
      strcpy(pFiles->pPicFileNameList[0], pFirstPicFile);
      pFiles->pPicFileNameList[0][strlen(pFirstPicFile)] = 0;
    }
  }

  // second file

  if (pSecondPicFile) {
    if ((pSecondPicFile[0]) != 0) {
      pFiles->pPicFileNameList[1] = (char *)MemAlloc(strlen(pSecondPicFile) + 1);
      strcpy(pFiles->pPicFileNameList[1], pSecondPicFile);
      pFiles->pPicFileNameList[1][strlen(pSecondPicFile)] = 0;
    }
  }

  // return unique id
  return uiId;
}

void OpenAndReadFilesFile(void) {
  // this procedure will open and read in data to the finance list
  HWFILE hFileHandle;
  uint8_t ubCode;
  uint32_t uiDate;
  uint32_t iBytesRead = 0;
  uint32_t uiByteCount = 0;
  char pFirstFilePath[128];
  char pSecondFilePath[128];
  uint8_t ubFormat;
  BOOLEAN fRead;

  // clear out the old list
  ClearFilesList();

  // no file, return
  if (!(FileMan_Exists(FILES_DAT_FILE))) return;

  // open file
  hFileHandle = FileMan_Open(FILES_DAT_FILE, (FILE_OPEN_EXISTING | FILE_ACCESS_READ), FALSE);

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
    // read in data
    FileMan_Read(hFileHandle, &ubCode, sizeof(uint8_t), &iBytesRead);

    FileMan_Read(hFileHandle, &uiDate, sizeof(uint32_t), &iBytesRead);

    FileMan_Read(hFileHandle, &pFirstFilePath, 128, &iBytesRead);

    FileMan_Read(hFileHandle, &pSecondFilePath, 128, &iBytesRead);

    FileMan_Read(hFileHandle, &ubFormat, sizeof(uint8_t), &iBytesRead);

    FileMan_Read(hFileHandle, &fRead, sizeof(uint8_t), &iBytesRead);
    // add transaction
    ProcessAndEnterAFilesRecord(ubCode, uiDate, ubFormat, pFirstFilePath, pSecondFilePath, fRead);

    // increment byte counter
    uiByteCount +=
        sizeof(uint32_t) + sizeof(uint8_t) + 128 + 128 + sizeof(uint8_t) + sizeof(BOOLEAN);
  }

  // close file
  FileMan_Close(hFileHandle);

  return;
}

BOOLEAN OpenAndWriteFilesFile(void) {
  // this procedure will open and write out data from the finance list
  HWFILE hFileHandle;
  FilesUnitPtr pFilesList = pFilesListHead;
  char pFirstFilePath[128];
  char pSecondFilePath[128];

  memset(&pFirstFilePath, 0, sizeof(pFirstFilePath));
  memset(&pSecondFilePath, 0, sizeof(pSecondFilePath));

  if (pFilesList != NULL) {
    if (pFilesList->pPicFileNameList[0]) {
      strcpy(pFirstFilePath, pFilesList->pPicFileNameList[0]);
    }
    if (pFilesList->pPicFileNameList[1]) {
      strcpy(pSecondFilePath, pFilesList->pPicFileNameList[1]);
    }
  }

  // open file
  hFileHandle = FileMan_Open(FILES_DAT_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE);

  // if no file exits, do nothing
  if (!hFileHandle) {
    return (FALSE);
  }
  // write info, while there are elements left in the list
  while (pFilesList) {
    // now write date and amount, and code
    FileMan_Write(hFileHandle, &(pFilesList->ubCode), sizeof(uint8_t), NULL);
    FileMan_Write(hFileHandle, &(pFilesList->uiDate), sizeof(uint32_t), NULL);
    FileMan_Write(hFileHandle, &(pFirstFilePath), 128, NULL);
    FileMan_Write(hFileHandle, &(pSecondFilePath), 128, NULL);
    FileMan_Write(hFileHandle, &(pFilesList->ubFormat), sizeof(uint8_t), NULL);
    FileMan_Write(hFileHandle, &(pFilesList->fRead), sizeof(uint8_t), NULL);

    // next element in list
    pFilesList = pFilesList->Next;
  }

  // close file
  FileMan_Close(hFileHandle);
  // clear out the old list
  ClearFilesList();

  return (TRUE);
}

void ClearFilesList(void) {
  // remove each element from list of transactions
  FilesUnitPtr pFilesList = pFilesListHead;
  FilesUnitPtr pFilesNode = pFilesList;

  // while there are elements in the list left, delete them
  while (pFilesList) {
    // set node to list head
    pFilesNode = pFilesList;

    // set list head to next node
    pFilesList = pFilesList->Next;

    // if present, dealloc string
    if (pFilesNode->pPicFileNameList[0]) {
      MemFree(pFilesNode->pPicFileNameList[0]);
    }

    if (pFilesNode->pPicFileNameList[1]) {
      MemFree(pFilesNode->pPicFileNameList[1]);
    }
    // delete current node
    MemFree(pFilesNode);
  }
  pFilesListHead = NULL;
  return;
}

void DrawFilesListBackGround(void) { return; }

void DisplayFilesList(void) {
  // this function will run through the list of files of files and display the 'sender'
  FilesUnitPtr pFilesList = pFilesListHead;
  int32_t iCounter = 0;
  struct VObject *hHandle;

  // font stuff
  SetFont(FILES_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
  SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

  // runt hrough list displaying 'sender'
  while ((pFilesList))  //&&(iCounter < MAX_FILES_LIST_LENGTH))
  {
    if (iCounter == iHighLightFileLine) {
      // render highlight
      GetVideoObject(&hHandle, guiHIGHLIGHT);
      BltVideoObject(vsFB, hHandle, 0, FILES_SENDER_TEXT_X - 5,
                     ((iCounter + 9) * BLOCK_HEIGHT) + (iCounter * 2) - 4);
    }
    mprintf(FILES_SENDER_TEXT_X, ((iCounter + 9) * BLOCK_HEIGHT) + (iCounter * 2) - 2,
            pFilesSenderList[pFilesList->ubCode]);
    iCounter++;
    pFilesList = pFilesList->Next;
  }

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void DisplayFileMessage(void) {
  // get the currently selected message
  if (iHighLightFileLine != -1) {
    // display text
    DisplayFormattedText();
  } else {
    HandleFileViewerButtonStates();
  }

  // reset shadow
  SetFontShadow(DEFAULT_SHADOW);

  return;
}

void InitializeFilesMouseRegions(void) {
  int32_t iCounter = 0;
  // init mouseregions
  for (iCounter = 0; iCounter < MAX_FILES_PAGE; iCounter++) {
    MSYS_DefineRegion(&pFilesRegions[iCounter], FILES_LIST_X,
                      (int16_t)(FILES_LIST_Y + iCounter * (BLOCK_HEIGHT + 2)),
                      FILES_LIST_X + FILES_LIST_WIDTH,
                      (int16_t)(FILES_LIST_Y + (iCounter + 1) * (BLOCK_HEIGHT + 2)),
                      MSYS_PRIORITY_NORMAL + 2, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, FilesBtnCallBack);
    MSYS_AddRegion(&pFilesRegions[iCounter]);
    MSYS_SetRegionUserData(&pFilesRegions[iCounter], 0, iCounter);
  }

  return;
}

void RemoveFilesMouseRegions(void) {
  int32_t iCounter = 0;
  for (iCounter = 0; iCounter < MAX_FILES_PAGE; iCounter++) {
    MSYS_RemoveRegion(&pFilesRegions[iCounter]);
  }
}

void FilesBtnCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  int32_t iFileId = -1;
  int32_t iCounter = 0;
  FilesUnitPtr pFilesList = pFilesListHead;

  if (iReason & MSYS_CALLBACK_REASON_INIT) {
    return;
  }

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // left button
    iFileId = MSYS_GetRegionUserData(pRegion, 0);

    // reset iHighLightListLine
    iHighLightFileLine = -1;

    if (iHighLightFileLine == iFileId) {
      return;
    }

    // make sure is a valid
    while (pFilesList) {
      // if iCounter = iFileId, is a valid file
      if (iCounter == iFileId) {
        giFilesPage = 0;
        iHighLightFileLine = iFileId;
      }

      // next element in list
      pFilesList = pFilesList->Next;

      // increment counter
      iCounter++;
    }

    fReDrawScreenFlag = TRUE;

    return;
  }
}

BOOLEAN DisplayFormattedText(void) {
  FilesUnitPtr pFilesList = pFilesListHead;

  uint16_t usFirstWidth = 0;
  uint16_t usFirstHeight = 0;
  uint16_t usSecondWidth;
  uint16_t usSecondHeight;
  int32_t iCounter = 0;
  int32_t iLength = 0;
  int32_t iHeight = 0;
  int32_t iOffSet = 0;
  int32_t iMessageCode;
  wchar_t sString[2048];
  struct VObject *hHandle;
  uint32_t uiFirstTempPicture;
  uint32_t uiSecondTempPicture;
  int16_t usFreeSpace = 0;

  fWaitAFrame = FALSE;

  // get the file that was highlighted
  while (iCounter < iHighLightFileLine) {
    iCounter++;
    pFilesList = pFilesList->Next;
  }

  // message code found, reset counter
  iMessageCode = pFilesList->ubCode;
  iCounter = 0;

  // set file as read
  pFilesList->fRead = TRUE;

  // clear the file string structure list
  // get file background object
  GetVideoObject(&hHandle, guiFileBack);

  // blt background to screen
  BltVideoObject(vsFB, hHandle, 0, FILE_VIEWER_X, FILE_VIEWER_Y - 4);

  // get the offset in the file
  while (iCounter < iMessageCode) {
    // increment increment offset
    iOffSet += ubFileRecordsLength[iCounter];

    // increment counter
    iCounter++;
  }

  iLength = ubFileRecordsLength[pFilesList->ubCode];

  if (pFilesList->ubFormat < ENRICO_BACKGROUND) {
    LoadEncryptedDataFromFile("BINARYDATA\\Files.edt", sString, FILE_STRING_SIZE * (iOffSet) * 2,
                              FILE_STRING_SIZE * iLength * 2);
  }

  // reset counter
  iCounter = 0;

  // no shadow
  SetFontShadow(NO_SHADOW);

  switch (pFilesList->ubFormat) {
    case 0:

      // no format, all text

      while (iLength > iCounter) {
        // read one record from file manager file
        LoadEncryptedDataFromFile("BINARYDATA\\Files.edt", sString,
                                  FILE_STRING_SIZE * (iOffSet + iCounter) * 2,
                                  FILE_STRING_SIZE * 2);

        // display string and get height
        iHeight += IanDisplayWrappedString(FILE_VIEWER_X + 4, (uint16_t)(FILE_VIEWER_Y + iHeight),
                                           FILE_VIEWER_WIDTH, FILE_GAP, FILES_TEXT_FONT,
                                           FILE_TEXT_COLOR, sString, 0, FALSE, 0);

        // increment file record counter
        iCounter++;
      }
      break;

    case 1:

      // second format, one picture, all text below

      // load graphic
      CHECKF(
          AddVObject(CreateVObjectFromFile(pFilesList->pPicFileNameList[0]), &uiFirstTempPicture));

      GetVideoObjectETRLESubregionProperties(uiFirstTempPicture, 0, &usFirstWidth, &usFirstHeight);

      // get file background object
      GetVideoObject(&hHandle, uiFirstTempPicture);

      // blt background to screen
      BltVideoObject(vsFB, hHandle, 0, FILE_VIEWER_X + 4 + (FILE_VIEWER_WIDTH - usFirstWidth) / 2,
                     FILE_VIEWER_Y + 10);

      iHeight = usFirstHeight + 20;

      while (iLength > iCounter) {
        // read one record from file manager file
        LoadEncryptedDataFromFile("BINARYDATA\\Files.edt", sString,
                                  FILE_STRING_SIZE * (iOffSet + iCounter) * 2,
                                  FILE_STRING_SIZE * 2);

        // display string and get height
        iHeight += IanDisplayWrappedString(FILE_VIEWER_X + 4, (uint16_t)(FILE_VIEWER_Y + iHeight),
                                           FILE_VIEWER_WIDTH, FILE_GAP, FILES_TEXT_FONT,
                                           FILE_TEXT_COLOR, sString, 0, FALSE, 0);

        // increment file record counter
        iCounter++;
      }

      // delete video object
      DeleteVideoObjectFromIndex(uiFirstTempPicture);

      break;
    case 2:

      // third format, two pictures, side by side with all text below

      // load first graphic
      CHECKF(
          AddVObject(CreateVObjectFromFile(pFilesList->pPicFileNameList[0]), &uiFirstTempPicture));

      // load second graphic
      CHECKF(
          AddVObject(CreateVObjectFromFile(pFilesList->pPicFileNameList[1]), &uiSecondTempPicture));

      GetVideoObjectETRLESubregionProperties(uiFirstTempPicture, 0, &usFirstWidth, &usFirstHeight);
      GetVideoObjectETRLESubregionProperties(uiSecondTempPicture, 0, &usSecondWidth,
                                             &usSecondHeight);

      // get free space;
      usFreeSpace = FILE_VIEWER_WIDTH - usFirstWidth - usSecondWidth;

      usFreeSpace /= 3;
      // get file background object
      GetVideoObject(&hHandle, uiFirstTempPicture);

      // blt background to screen
      BltVideoObject(vsFB, hHandle, 0, FILE_VIEWER_X + usFreeSpace, FILE_VIEWER_Y + 10);

      // get file background object
      GetVideoObject(&hHandle, uiSecondTempPicture);

      // get position for second picture
      usFreeSpace *= 2;
      usFreeSpace += usFirstWidth;

      // blt background to screen
      BltVideoObject(vsFB, hHandle, 0, FILE_VIEWER_X + usFreeSpace, FILE_VIEWER_Y + 10);

      // delete video object
      DeleteVideoObjectFromIndex(uiFirstTempPicture);
      DeleteVideoObjectFromIndex(uiSecondTempPicture);

      // put in text
      iHeight = usFirstHeight + 20;

      while (iLength > iCounter) {
        // read one record from file manager file
        LoadEncryptedDataFromFile("BINARYDATA\\Files.edt", sString,
                                  FILE_STRING_SIZE * (iOffSet + iCounter) * 2,
                                  FILE_STRING_SIZE * 2);

        // display string and get height
        iHeight += IanDisplayWrappedString(FILE_VIEWER_X + 4, (uint16_t)(FILE_VIEWER_Y + iHeight),
                                           FILE_VIEWER_WIDTH, FILE_GAP, FILES_TEXT_FONT,
                                           FILE_TEXT_COLOR, sString, 0, FALSE, 0);

        // increment file record counter
        iCounter++;
      }

      break;

    case 3:
      // picture on the left, with text on right and below
      // load first graphic
      HandleSpecialTerroristFile(pFilesList->ubCode, pFilesList->pPicFileNameList[0]);
      break;
    default:
      HandleSpecialFiles(pFilesList->ubFormat);
      break;
  }

  HandleFileViewerButtonStates();
  SetFontShadow(DEFAULT_SHADOW);

  return (TRUE);
}

BOOLEAN HandleSpecialFiles(uint8_t ubFormat) {
  int32_t iCounter = 0;
  wchar_t sString[2048];
  FileStringPtr pTempString = NULL;
  FileStringPtr pLocatorString = NULL;
  int32_t iYPositionOnPage = 0;
  int32_t iFileLineWidth = 0;
  int32_t iFileStartX = 0;
  uint32_t uiFlags = 0;
  uint32_t uiFont = 0;
  BOOLEAN fGoingOffCurrentPage = FALSE;
  FileRecordWidthPtr WidthList = NULL;

  uint32_t uiPicture;
  struct VObject *hHandle;

  ClearFileStringList();

  switch (ubFormat) {
    case (255):
      // load data
      // read one record from file manager file

      WidthList = CreateWidthRecordsForAruloIntelFile();
      while (iCounter < LENGTH_OF_ENRICO_FILE) {
        LoadEncryptedDataFromFile("BINARYDATA\\RIS.EDT", sString, FILE_STRING_SIZE * (iCounter) * 2,
                                  FILE_STRING_SIZE * 2);
        AddStringToFilesList(sString);
        iCounter++;
      }

      pTempString = pFileStringList;

      iYPositionOnPage = 0;
      iCounter = 0;
      pLocatorString = pTempString;

      pTempString = GetFirstStringOnThisPage(pFileStringList, FILES_TEXT_FONT, 350, FILE_GAP,
                                             giFilesPage, MAX_FILE_MESSAGE_PAGE_SIZE, WidthList);

      // find out where this string is
      while (pLocatorString != pTempString) {
        iCounter++;
        pLocatorString = pLocatorString->Next;
      }

      // move through list and display
      while (pTempString) {
        uiFlags = IAN_WRAP_NO_SHADOW;
        // copy over string
        wcscpy(sString, pTempString->pString);

        if (sString[0] == 0) {
          // on last page
          fOnLastFilesPageFlag = TRUE;
        }

        // set up font
        uiFont = FILES_TEXT_FONT;
        if (giFilesPage == 0) {
          switch (iCounter) {
            case (0):
              uiFont = FILES_TITLE_FONT;
              break;
          }
        }

        // reset width
        iFileLineWidth = 350;
        iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);

        // based on the record we are at, selected X start position and the width to wrap the line,
        // to fit around pictures

        if (iCounter == 0) {
          // title
          iFileLineWidth = 350;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);

        } else if (iCounter == 1) {
          // opening on first page
          iFileLineWidth = 350;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);

        } else if ((iCounter > 1) && (iCounter < FILES_COUNTER_1_WIDTH)) {
          iFileLineWidth = 350;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);

        } else if (iCounter == FILES_COUNTER_1_WIDTH) {
          if (giFilesPage == 0) {
            iYPositionOnPage += (MAX_FILE_MESSAGE_PAGE_SIZE - iYPositionOnPage);
          }
          iFileLineWidth = 350;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);
        }

        else if (iCounter == FILES_COUNTER_2_WIDTH) {
          iFileLineWidth = 200;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 150);
        } else if (iCounter == FILES_COUNTER_3_WIDTH) {
          iFileLineWidth = 200;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 150);
        }

        else {
          iFileLineWidth = 350;
          iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);
        }
        // not far enough, advance

        if ((iYPositionOnPage + IanWrappedStringHeight(0, 0, (uint16_t)iFileLineWidth, FILE_GAP,
                                                       uiFont, 0, sString, 0, 0, 0)) <
            MAX_FILE_MESSAGE_PAGE_SIZE) {
          // now print it
          iYPositionOnPage += (int32_t)IanDisplayWrappedString(
              (uint16_t)(iFileStartX), (uint16_t)(FILE_VIEWER_Y + iYPositionOnPage),
              (int16_t)iFileLineWidth, FILE_GAP, uiFont, FILE_TEXT_COLOR, sString, 0, FALSE,
              uiFlags);

          fGoingOffCurrentPage = FALSE;
        } else {
          // gonna get cut off...end now
          fGoingOffCurrentPage = TRUE;
        }

        pTempString = pTempString->Next;

        if (pTempString == NULL) {
          // on last page
          fOnLastFilesPageFlag = TRUE;
        } else {
          fOnLastFilesPageFlag = FALSE;
        }

        // going over the edge, stop now
        if (fGoingOffCurrentPage == TRUE) {
          pTempString = NULL;
        }
        iCounter++;
      }
      ClearOutWidthRecordsList(WidthList);
      ClearFileStringList();
      break;
  }

  // place pictures
  // page 1 picture of country
  if (giFilesPage == 0) {
    // title bar
    CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\ArucoFilesMap.sti"), &uiPicture));

    // get title bar object
    GetVideoObject(&hHandle, uiPicture);

    // blt title bar to screen
    BltVideoObject(vsFB, hHandle, 0, 300, 270);

    DeleteVideoObjectFromIndex(uiPicture);

  } else if (giFilesPage == 4) {
    // kid pic
    CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Enrico_Y.sti"), &uiPicture));

    // get title bar object
    GetVideoObject(&hHandle, uiPicture);

    // blt title bar to screen
    BltVideoObject(vsFB, hHandle, 0, 260, 225);

    DeleteVideoObjectFromIndex(uiPicture);

  } else if (giFilesPage == 5) {
    // wedding pic
    CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\Enrico_W.sti"), &uiPicture));

    // get title bar object
    GetVideoObject(&hHandle, uiPicture);

    // blt title bar to screen
    BltVideoObject(vsFB, hHandle, 0, 260, 85);

    DeleteVideoObjectFromIndex(uiPicture);
  }

  return (TRUE);
}

void AddStringToFilesList(wchar_t *pString) {
  FileStringPtr pFileString;
  FileStringPtr pTempString = pFileStringList;

  // create string structure
  pFileString = (FileString *)MemAlloc(sizeof(FileString));

  // alloc string and copy
  pFileString->pString = (wchar_t *)MemAlloc((wcslen(pString) * 2) + 2);
  wcscpy(pFileString->pString, pString);
  pFileString->pString[wcslen(pString)] = 0;

  // set Next to NULL

  pFileString->Next = NULL;
  if (pFileStringList == NULL) {
    pFileStringList = pFileString;
  } else {
    while (pTempString->Next) {
      pTempString = pTempString->Next;
    }
    pTempString->Next = pFileString;
  }

  return;
}

void ClearFileStringList(void) {
  FileStringPtr pFileString;
  FileStringPtr pDeleteFileString;

  pFileString = pFileStringList;

  if (pFileString == NULL) {
    return;
  }
  while (pFileString->Next) {
    pDeleteFileString = pFileString;
    pFileString = pFileString->Next;
    MemFree(pDeleteFileString);
  }

  // last one
  MemFree(pFileString);

  pFileStringList = NULL;
}

void CreateButtonsForFilesPage(void) {
  // will create buttons for the files page
  giFilesPageButtonsImage[0] = LoadButtonImage("LAPTOP\\arrows.sti", -1, 0, -1, 1, -1);
  giFilesPageButtons[0] = QuickCreateButton(
      giFilesPageButtonsImage[0], PREVIOUS_FILE_PAGE_BUTTON_X, PREVIOUS_FILE_PAGE_BUTTON_Y,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnPreviousFilePageCallback);

  giFilesPageButtonsImage[1] = LoadButtonImage("LAPTOP\\arrows.sti", -1, 6, -1, 7, -1);
  giFilesPageButtons[1] = QuickCreateButton(
      giFilesPageButtonsImage[1], NEXT_FILE_PAGE_BUTTON_X, NEXT_FILE_PAGE_BUTTON_Y, BUTTON_TOGGLE,
      MSYS_PRIORITY_HIGHEST - 1, (GUI_CALLBACK)BtnGenericMouseMoveButtonCallback,
      (GUI_CALLBACK)BtnNextFilePageCallback);

  SetButtonCursor(giFilesPageButtons[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giFilesPageButtons[1], CURSOR_LAPTOP_SCREEN);

  return;
}

void DeleteButtonsForFilesPage(void) {
  // destroy buttons for the files page

  RemoveButton(giFilesPageButtons[0]);
  UnloadButtonImage(giFilesPageButtonsImage[0]);

  RemoveButton(giFilesPageButtons[1]);
  UnloadButtonImage(giFilesPageButtonsImage[1]);

  return;
}

// callbacks
void BtnPreviousFilePageCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (fWaitAFrame == TRUE) {
      return;
    }

    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      btn->uiFlags |= (BUTTON_CLICKED_ON);
    }

  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (fWaitAFrame == TRUE) {
      return;
    }

    if ((btn->uiFlags & BUTTON_CLICKED_ON)) {
      if (giFilesPage > 0) {
        giFilesPage--;
        fWaitAFrame = TRUE;
      }

      fReDrawScreenFlag = TRUE;
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      MarkButtonsDirty();
    }
  }

  return;
}

void BtnNextFilePageCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (fWaitAFrame == TRUE) {
      return;
    }

    if (!(btn->uiFlags & BUTTON_CLICKED_ON)) {
      btn->uiFlags |= (BUTTON_CLICKED_ON);
    }

  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (fWaitAFrame == TRUE) {
      return;
    }

    if ((btn->uiFlags & BUTTON_CLICKED_ON)) {
      if ((fOnLastFilesPageFlag) == FALSE) {
        fWaitAFrame = TRUE;
        giFilesPage++;
      }

      fReDrawScreenFlag = TRUE;
      btn->uiFlags &= ~(BUTTON_CLICKED_ON);
      MarkButtonsDirty();
    }
  }

  return;
}

void HandleFileViewerButtonStates(void) {
  // handle state of email viewer buttons

  if (iHighLightFileLine == -1) {
    // not displaying message, leave
    DisableButton(giFilesPageButtons[0]);
    DisableButton(giFilesPageButtons[1]);
    ButtonList[giFilesPageButtons[0]]->uiFlags &= ~(BUTTON_CLICKED_ON);
    ButtonList[giFilesPageButtons[1]]->uiFlags &= ~(BUTTON_CLICKED_ON);

    return;
  }

  // turn off previous page button
  if (giFilesPage == 0) {
    DisableButton(giFilesPageButtons[0]);
    ButtonList[giFilesPageButtons[0]]->uiFlags &= ~(BUTTON_CLICKED_ON);

  } else {
    EnableButton(giFilesPageButtons[0]);
  }

  // turn off next page button
  if (fOnLastFilesPageFlag == TRUE) {
    DisableButton(giFilesPageButtons[1]);
    ButtonList[giFilesPageButtons[1]]->uiFlags &= ~(BUTTON_CLICKED_ON);
  } else {
    EnableButton(giFilesPageButtons[1]);
  }

  return;
}

FileRecordWidthPtr CreateRecordWidth(int32_t iRecordNumber, int32_t iRecordWidth,
                                     int32_t iRecordHeightAdjustment, uint8_t ubFlags) {
  FileRecordWidthPtr pTempRecord = NULL;

  // allocs and inits a width info record for the multipage file viewer...this will tell the
  // procedure that does inital computation on which record is the start of the current page how
  // wide special records are ( ones that share space with pictures )
  pTempRecord = (FileRecordWidth *)MemAlloc(sizeof(FileRecordWidth));

  pTempRecord->Next = NULL;
  pTempRecord->iRecordNumber = iRecordNumber;
  pTempRecord->iRecordWidth = iRecordWidth;
  pTempRecord->iRecordHeightAdjustment = iRecordHeightAdjustment;
  pTempRecord->ubFlags = ubFlags;

  return (pTempRecord);
}

FileRecordWidthPtr CreateWidthRecordsForAruloIntelFile(void) {
  // this fucntion will create the width list for the Arulco intelligence file
  FileRecordWidthPtr pTempRecord = NULL;
  FileRecordWidthPtr pRecordListHead = NULL;

  // first record width
  //	pTempRecord = CreateRecordWidth( 7, 350, 200,0 );
  pTempRecord = CreateRecordWidth(FILES_COUNTER_1_WIDTH, 350, MAX_FILE_MESSAGE_PAGE_SIZE, 0);

  // set up head of list now
  pRecordListHead = pTempRecord;

  // next record
  //	pTempRecord -> Next = CreateRecordWidth( 43, 200,0, 0 );
  pTempRecord->Next = CreateRecordWidth(FILES_COUNTER_2_WIDTH, 200, 0, 0);
  pTempRecord = pTempRecord->Next;

  // and the next..
  //	pTempRecord -> Next = CreateRecordWidth( 45, 200,0, 0 );
  pTempRecord->Next = CreateRecordWidth(FILES_COUNTER_3_WIDTH, 200, 0, 0);
  pTempRecord = pTempRecord->Next;

  return (pRecordListHead);
}

FileRecordWidthPtr CreateWidthRecordsForTerroristFile(void) {
  // this fucntion will create the width list for the Arulco intelligence file
  FileRecordWidthPtr pTempRecord = NULL;
  FileRecordWidthPtr pRecordListHead = NULL;

  // first record width
  pTempRecord = CreateRecordWidth(4, 170, 0, 0);

  // set up head of list now
  pRecordListHead = pTempRecord;

  // next record
  pTempRecord->Next = CreateRecordWidth(5, 170, 0, 0);
  pTempRecord = pTempRecord->Next;

  pTempRecord->Next = CreateRecordWidth(6, 170, 0, 0);
  pTempRecord = pTempRecord->Next;

  return (pRecordListHead);
}

void ClearOutWidthRecordsList(FileRecordWidthPtr pFileRecordWidthList) {
  FileRecordWidthPtr pTempRecord = NULL;
  FileRecordWidthPtr pDeleteRecord = NULL;

  // set up to head of the list
  pTempRecord = pDeleteRecord = pFileRecordWidthList;

  // error check
  if (pFileRecordWidthList == NULL) {
    return;
  }

  while (pTempRecord->Next) {
    // set up delete record
    pDeleteRecord = pTempRecord;

    // move to next record
    pTempRecord = pTempRecord->Next;

    MemFree(pDeleteRecord);
  }

  // now get the last element
  MemFree(pTempRecord);

  // null out passed ptr
  pFileRecordWidthList = NULL;

  return;
}

void OpenFirstUnreadFile(void) {
  // open the first unread file in the list
  int32_t iCounter = 0;
  FilesUnitPtr pFilesList = pFilesListHead;

  // make sure is a valid
  while (pFilesList) {
    // if iCounter = iFileId, is a valid file
    if (pFilesList->fRead == FALSE) {
      iHighLightFileLine = iCounter;
    }

    // next element in list
    pFilesList = pFilesList->Next;

    // increment counter
    iCounter++;
  }

  return;
}

void CheckForUnreadFiles(void) {
  BOOLEAN fStatusOfNewFileFlag = fNewFilesInFileViewer;

  // willc heck for any unread files and set flag if any
  FilesUnitPtr pFilesList = pFilesListHead;

  fNewFilesInFileViewer = FALSE;

  while (pFilesList) {
    // unread?...if so, set flag
    if (pFilesList->fRead == FALSE) {
      fNewFilesInFileViewer = TRUE;
    }
    // next element in list
    pFilesList = pFilesList->Next;
  }

  // if the old flag and the new flag arent the same, either create or destory the fast help region
  if (fNewFilesInFileViewer != fStatusOfNewFileFlag) {
    CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE,
                                          (BOOLEAN)!fNewFilesInFileViewer);
  }
}

BOOLEAN HandleSpecialTerroristFile(int32_t iFileNumber, char *sPictureName) {
  int32_t iCounter = 0;
  wchar_t sString[2048];
  FileStringPtr pTempString = NULL;
  FileStringPtr pLocatorString = NULL;
  int32_t iYPositionOnPage = 0;
  int32_t iFileLineWidth = 0;
  int32_t iFileStartX = 0;
  uint32_t uiFlags = 0;
  uint32_t uiFont = 0;
  BOOLEAN fGoingOffCurrentPage = FALSE;
  FileRecordWidthPtr WidthList = NULL;
  int32_t iOffset = 0;
  uint32_t uiPicture;
  struct VObject *hHandle;
  char sTemp[128];

  iOffset = ubFileOffsets[iFileNumber];

  // grab width list
  WidthList = CreateWidthRecordsForTerroristFile();

  while (iCounter < ubFileRecordsLength[iFileNumber]) {
    LoadEncryptedDataFromFile("BINARYDATA\\files.EDT", sString,
                              FILE_STRING_SIZE * (iOffset + iCounter) * 2, FILE_STRING_SIZE * 2);
    AddStringToFilesList(sString);
    iCounter++;
  }

  pTempString = pFileStringList;

  iYPositionOnPage = 0;
  iCounter = 0;
  pLocatorString = pTempString;

  pTempString = GetFirstStringOnThisPage(pFileStringList, FILES_TEXT_FONT, 350, FILE_GAP,
                                         giFilesPage, MAX_FILE_MESSAGE_PAGE_SIZE, WidthList);

  // find out where this string is
  while (pLocatorString != pTempString) {
    iCounter++;
    pLocatorString = pLocatorString->Next;
  }

  // move through list and display
  while (pTempString) {
    uiFlags = IAN_WRAP_NO_SHADOW;
    // copy over string
    wcscpy(sString, pTempString->pString);

    if (sString[0] == 0) {
      // on last page
      fOnLastFilesPageFlag = TRUE;
    }

    // set up font
    uiFont = FILES_TEXT_FONT;
    if (giFilesPage == 0) {
      switch (iCounter) {
        case (0):
          uiFont = FILES_TITLE_FONT;
          break;
      }
    }

    if ((iCounter > 3) && (iCounter < 7)) {
      iFileLineWidth = 170;
      iFileStartX = (uint16_t)(FILE_VIEWER_X + 180);
    } else {
      // reset width
      iFileLineWidth = 350;
      iFileStartX = (uint16_t)(FILE_VIEWER_X + 10);
    }

    // based on the record we are at, selected X start position and the width to wrap the line, to
    // fit around pictures
    if ((iYPositionOnPage + IanWrappedStringHeight(0, 0, (uint16_t)iFileLineWidth, FILE_GAP, uiFont,
                                                   0, sString, 0, 0, 0)) <
        MAX_FILE_MESSAGE_PAGE_SIZE) {
      // now print it
      iYPositionOnPage += (int32_t)IanDisplayWrappedString(
          (uint16_t)(iFileStartX), (uint16_t)(FILE_VIEWER_Y + iYPositionOnPage),
          (int16_t)iFileLineWidth, FILE_GAP, uiFont, FILE_TEXT_COLOR, sString, 0, FALSE, uiFlags);

      fGoingOffCurrentPage = FALSE;
    } else {
      // gonna get cut off...end now
      fGoingOffCurrentPage = TRUE;
    }

    pTempString = pTempString->Next;

    if ((pTempString == NULL) && (fGoingOffCurrentPage == FALSE)) {
      // on last page
      fOnLastFilesPageFlag = TRUE;
    } else {
      fOnLastFilesPageFlag = FALSE;
    }

    // going over the edge, stop now
    if (fGoingOffCurrentPage == TRUE) {
      pTempString = NULL;
    }

    // show picture
    if ((giFilesPage == 0) && (iCounter == 5)) {
      if (usProfileIdsForTerroristFiles[iFileNumber + 1] < 100) {
        sprintf(sTemp, "%s%02d.sti", "FACES\\BIGFACES\\",
                usProfileIdsForTerroristFiles[iFileNumber + 1]);
      } else {
        sprintf(sTemp, "%s%03d.sti", "FACES\\BIGFACES\\",
                usProfileIdsForTerroristFiles[iFileNumber + 1]);
      }

      CHECKF(AddVObject(CreateVObjectFromFile(sTemp), &uiPicture));

      // Blt face to screen to
      GetVideoObject(&hHandle, uiPicture);

      BltVideoObject(vsFB, hHandle, 0, (int16_t)(FILE_VIEWER_X + 30),
                     (int16_t)(iYPositionOnPage + 21));

      DeleteVideoObjectFromIndex(uiPicture);

      CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\InterceptBorder.sti"), &uiPicture));

      // Blt face to screen to
      GetVideoObject(&hHandle, uiPicture);

      BltVideoObject(vsFB, hHandle, 0, (int16_t)(FILE_VIEWER_X + 25),
                     (int16_t)(iYPositionOnPage + 16));

      DeleteVideoObjectFromIndex(uiPicture);
    }

    iCounter++;
  }

  ClearOutWidthRecordsList(WidthList);
  ClearFileStringList();

  return (TRUE);
}

// add a file about this terrorist
BOOLEAN AddFileAboutTerrorist(int32_t iProfileId) {
  int32_t iCounter = 0;

  for (iCounter = 1; iCounter < 7; iCounter++) {
    if (usProfileIdsForTerroristFiles[iCounter] == iProfileId) {
      // checked, and this file is there
      AddFilesToPlayersLog((uint8_t)iCounter, 0, 3, NULL, NULL);
      return (TRUE);
    }
  }

  return (FALSE);
}
