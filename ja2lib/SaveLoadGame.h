// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _SAVE_LOAD_GAME_H_
#define _SAVE_LOAD_GAME_H_

#include "GameSettings.h"
#include "SGP/Types.h"

struct SOLDIERTYPE;

#define BYTESINMEGABYTE 1048576  // 1024*1024
#define REQUIRED_FREE_SPACE (20 * BYTESINMEGABYTE)

#define SIZE_OF_SAVE_GAME_DESC 128

#define GAME_VERSION_LENGTH 16

#define SAVE__ERROR_NUM 99
#define SAVE__END_TURN_NUM 98

typedef struct {
  uint32_t uiSavedGameVersion;
  char zGameVersionNumber[GAME_VERSION_LENGTH];

  wchar_t sSavedGameDesc[SIZE_OF_SAVE_GAME_DESC];

  uint32_t uiFlags;

  // The following will be used to quickly access info to display in the save/load screen
  uint32_t uiDay;
  uint8_t ubHour;
  uint8_t ubMin;
  int16_t sSectorX;
  int16_t sSectorY;
  int8_t bSectorZ;
  uint8_t ubNumOfMercsOnPlayersTeam;
  int32_t iCurrentBalance;

  uint32_t uiCurrentScreen;

  BOOLEAN fAlternateSector;

  BOOLEAN fWorldLoaded;

  uint8_t ubLoadScreenID;  // The load screen that should be used when loading the saved game

  GAME_OPTIONS sInitialGameOptions;  // need these in the header so we can get the info from it on
                                     // the save load screen.

  uint32_t uiRandom;

  uint8_t ubFiller[110];

} SAVED_GAME_HEADER;

extern uint32_t guiScreenToGotoAfterLoadingSavedGame;
extern uint32_t guiSaveGameVersion;

void CreateSavedGameFileNameFromNumber(uint8_t ubSaveGameID, char *pzNewFileName);

BOOLEAN SaveGame(uint8_t ubSaveGameID, wchar_t *pGameDesc, size_t bufSize);
BOOLEAN LoadSavedGame(uint8_t ubSavedGameID);

BOOLEAN CopySavedSoldierInfoToNewSoldier(struct SOLDIERTYPE *pDestSourceInfo,
                                         struct SOLDIERTYPE *pSourceInfo);

BOOLEAN SaveFilesToSavedGame(char *pSrcFileName, HWFILE hFile);
BOOLEAN LoadFilesFromSavedGame(char *pSrcFileName, HWFILE hFile);

BOOLEAN DoesUserHaveEnoughHardDriveSpace();

void GetBestPossibleSectorXYZValues(uint8_t *psSectorX, uint8_t *psSectorY, int8_t *pbSectorZ);

extern uint32_t guiLastSaveGameNum;
int8_t GetNumberForAutoSave(BOOLEAN fLatestAutoSave);

#endif
