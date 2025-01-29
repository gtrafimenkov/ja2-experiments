// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameRes.h"

#include "DebugLog.h"
#include "SGP/LibraryDataBasePub.h"

static enum ResourceVersion _resVersion = RT_ENGLISH;

void DetectResourcesVersion() {
  if (IsLibraryOpened("russian.slf")) {
    _resVersion = RT_RUSSIAN_BUKA;
  } else if (IsLibraryOpened("german.slf")) {
    _resVersion = RT_GERMAN;
  } else {
    _resVersion = RT_ENGLISH;
  }
  DebugLogF("Detect resources type: %d", _resVersion);

  // Library files that can be used to detect resource type:
  //   german.slf, IMPSYMBOL_GERMAN.PCX
  //   russian.slf, IMPSYMBOL_RUSSIAN.PCX
  //
  //   english, french and russian gold have the same file names inside of laptop.slf
  //   For resource type detection, it is better to check hash of some files, e.g hash of
  //   laptop/impsymbol.sti
}

enum ResourceVersion GetResourceVersion() { return _resVersion; }

bool UsingEnglishResources() { return _resVersion == RT_ENGLISH; }
bool UsingGermanResources() { return _resVersion == RT_GERMAN; }
bool UsingPolishResources() { return _resVersion == RT_POLISH; }
bool UsingRussianBukaResources() { return _resVersion == RT_RUSSIAN_BUKA; }
bool UsingRussianGoldResources() { return _resVersion == RT_RUSSIAN_GOLD; }

wchar_t GetZeroGlyphChar() {
  if (UsingRussianBukaResources()) {
    return L' ';
  } else {
    return L'A';
  }
}
