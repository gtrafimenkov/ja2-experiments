// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameSettings.h"
#include "SGP/FileMan.h"
#include "Utils/EncryptedFile.h"
#include "Utils/Text.h"

BOOLEAN LoadItemInfo(uint16_t ubIndex, wchar_t* pNameString, wchar_t* pInfoString) {
  HWFILE hFile;
  uint32_t uiBytesRead;
  uint32_t uiStartSeekAmount;

  hFile = FileMan_Open(ITEMSTRINGFILENAME, FILE_ACCESS_READ, FALSE);
  if (!hFile) {
    return (FALSE);
  }

  // Get current mercs bio info
  uiStartSeekAmount = ((SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex);

  // Skip short names
  uiStartSeekAmount += SIZE_SHORT_ITEM_NAME;

  if (FileMan_Seek(hFile, uiStartSeekAmount, FILE_SEEK_FROM_START) == FALSE) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  if (!FileMan_Read(hFile, pNameString, SIZE_ITEM_NAME, &uiBytesRead)) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  DecodeEncryptedString(pNameString, SIZE_ITEM_NAME / 2);

  // condition added by Chris - so we can get the name without the item info
  // when desired, by passing in a null pInfoString

  if (pInfoString != NULL) {
    // Get the additional info
    uiStartSeekAmount = ((SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex) +
                        SIZE_ITEM_NAME + SIZE_SHORT_ITEM_NAME;
    if (FileMan_Seek(hFile, uiStartSeekAmount, FILE_SEEK_FROM_START) == FALSE) {
      FileMan_Close(hFile);
      return (FALSE);
    }

    if (!FileMan_Read(hFile, pInfoString, SIZE_ITEM_INFO, &uiBytesRead)) {
      FileMan_Close(hFile);
      return (FALSE);
    }

    DecodeEncryptedString(pInfoString, SIZE_ITEM_INFO / 2);
  }

  FileMan_Close(hFile);
  return (TRUE);
}

BOOLEAN LoadShortNameItemInfo(uint16_t ubIndex, wchar_t* pNameString) {
  HWFILE hFile;
  //  wchar_t		DestString[ SIZE_MERC_BIO_INFO ];
  uint32_t uiBytesRead;
  uint32_t uiStartSeekAmount;

  hFile = FileMan_Open(ITEMSTRINGFILENAME, FILE_ACCESS_READ, FALSE);
  if (!hFile) {
    return (FALSE);
  }

  // Get current mercs bio info
  uiStartSeekAmount = ((SIZE_SHORT_ITEM_NAME + SIZE_ITEM_NAME + SIZE_ITEM_INFO) * ubIndex);

  if (FileMan_Seek(hFile, uiStartSeekAmount, FILE_SEEK_FROM_START) == FALSE) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  if (!FileMan_Read(hFile, pNameString, SIZE_ITEM_NAME, &uiBytesRead)) {
    FileMan_Close(hFile);
    return (FALSE);
  }

  DecodeEncryptedString(pNameString, SIZE_ITEM_NAME / 2);

  FileMan_Close(hFile);
  return (TRUE);
}

void LoadAllItemNames(void) {
  uint16_t usLoop;

  for (usLoop = 0; usLoop < MAXITEMS; usLoop++) {
    LoadItemInfo(usLoop, ItemNames[usLoop], NULL);

    // Load short item info
    LoadShortNameItemInfo(usLoop, ShortItemNames[usLoop]);
  }
}

void LoadAllExternalText(void) { LoadAllItemNames(); }

wchar_t* GetWeightUnitString(void) {
  if (gGameSettings.fOptions[TOPTION_USE_METRIC_SYSTEM])  // metric
  {
    return (pMessageStrings[MSG_KILOGRAM_ABBREVIATION]);
  } else {
    return (pMessageStrings[MSG_POUND_ABBREVIATION]);
  }
}

float GetWeightBasedOnMetricOption(uint32_t uiObjectWeight) {
  float fWeight = 0.0f;

  // if the user is smart and wants things displayed in 'metric'
  if (gGameSettings.fOptions[TOPTION_USE_METRIC_SYSTEM])  // metric
  {
    fWeight = (float)uiObjectWeight;
  }

  // else the user is a caveman and display it in pounds
  else {
    fWeight = uiObjectWeight * 2.2f;
  }

  return (fWeight);
}
