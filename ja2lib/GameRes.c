// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameRes.h"

#include "DebugLog.h"
#include "SGP/FileMan.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/MemMan.h"
#include "StrUtils.h"

static enum ResourceVersion _resType = RT_ENGLISH;

void DetectResourcesVersion() {
  SHA256STR hashstr;
  if (FileMan_CalcSHA256("loadscreens\\titletext.sti", hashstr) &&
      strequal(hashstr, "6e901685639cdc1e2563fa6587204312605e920c195e3603c4d88a1391486d68")) {
    _resType = RT_FRENCH;
  } else if (IsLibraryOpened("russian.slf")) {
    _resType = RT_RUSSIAN_BUKA;
  } else if (IsLibraryOpened("german.slf")) {
    _resType = RT_GERMAN;
  } else {
    _resType = RT_ENGLISH;
  }
  DebugLogF("Detect resources type: %d", _resType);

  // Library files that can be used to detect resource type:
  //   german.slf, IMPSYMBOL_GERMAN.PCX
  //   russian.slf, IMPSYMBOL_RUSSIAN.PCX
  //
  //   english, french and russian gold have the same file names inside of laptop.slf
  //   For resource type detection, it is better to check hash of some files, e.g hash of
  //   laptop/impsymbol.sti
}

enum ResourceVersion GetResourceVersion() { return _resType; }

bool UsingEnglishResources() { return _resType == RT_ENGLISH; }
bool UsingFrenchResources() { return _resType == RT_FRENCH; }
bool UsingGermanResources() { return _resType == RT_GERMAN; }
bool UsingPolishResources() { return _resType == RT_POLISH; }
bool UsingRussianBukaResources() { return _resType == RT_RUSSIAN_BUKA; }
bool UsingRussianGoldResources() { return _resType == RT_RUSSIAN_GOLD; }

wchar_t GetZeroGlyphChar() {
  if (UsingRussianBukaResources()) {
    return L' ';
  } else {
    return L'A';
  }
}

float GetMajorMapVersion() {
  // Don't mess with this value, unless you want to force update all maps in the game!
  if (UsingRussianBukaResources()) {
    return 6.00;
  } else {
    return 5.00;
  }
}
