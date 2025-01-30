// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameVersion.h"

#include <stdio.h>

#include "BuildInfo.h"
#include "GameRes.h"
#include "SGP/Types.h"

const char *GetGameVersionStr() {
  static char version[100];
  snprintf(version, 100, "JA2 Vanilla (%s), %s game data", BUILD_INFO, GetResourceVersionStr());
  return version;
}

char czVersionNumber[16] = {"Build 04.12.02"};
wchar_t zTrackingNumber[16] = {L"Z"};

#define SAVE_GAME_VERSION 99

const uint32_t guiSavedGameVersion = SAVE_GAME_VERSION;
