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

#define TITLETEXT_HASH_FRENCH "6e901685639cdc1e2563fa6587204312605e920c195e3603c4d88a1391486d68"
#define TITLETEXT_HASH_RU_GOLD "23bcf8a5e3fc64c730acd15b63b807b8a6cd3f11a7d107ec670f308e4d867623"

void DetectResourcesVersion() {
  // Using hash of the image with main menu buttons to detect exactly the kind of game resources.
  SHA256STR hashstr = "";
  FileMan_CalcSHA256("loadscreens\\titletext.sti", hashstr);
  if (strequal(hashstr, TITLETEXT_HASH_FRENCH)) {
    _resType = RT_FRENCH;
  } else if (strequal(hashstr, TITLETEXT_HASH_RU_GOLD)) {
    _resType = RT_RUSSIAN_GOLD;
  } else if (IsLibraryOpened("russian.slf")) {
    _resType = RT_RUSSIAN_BUKA;
  } else if (IsLibraryOpened("german.slf")) {
    _resType = RT_GERMAN;
  } else if (IsLibraryOpened("polish.slf")) {
    _resType = RT_POLISH;
  } else if (IsLibraryOpened("italian.slf")) {
    _resType = RT_ITALIAN;
  } else if (IsLibraryOpened("dutch.slf")) {
    _resType = RT_DUTCH;
  } else {
    _resType = RT_ENGLISH;
  }
  DebugLogF("Detect resources type: %s", GetResourceVersionStr());
}

enum ResourceVersion GetResourceVersion() { return _resType; }

const char *GetResourceVersionStr() {
  switch (_resType) {
    case RT_ENGLISH:
      return "English";
    case RT_FRENCH:
      return "French";
    case RT_GERMAN:
      return "German";
    case RT_POLISH:
      return "Polish";
    case RT_DUTCH:
      return "Dutch";
    case RT_ITALIAN:
      return "Italian";
    case RT_RUSSIAN_BUKA:
      return "Russian Buka";
    case RT_RUSSIAN_GOLD:
      return "Russian Gold";
    default:
      return "Unknown";
  }
}

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

void UseTextLanguagaeMatchingGameResourcesType() {
  switch (_resType) {
    case RT_FRENCH:
      UseTextFrench();
      break;
    case RT_GERMAN:
      UseTextGerman();
      break;
    case RT_DUTCH:
      UseTextDutch();
      break;
    case RT_ITALIAN:
      UseTextItalian();
      break;
    case RT_RUSSIAN_BUKA:
      UseTextRussian();
      break;
    case RT_RUSSIAN_GOLD:
      UseTextRussian();
      break;
    default:
      UseTextEnglish();
      break;
  }
}
