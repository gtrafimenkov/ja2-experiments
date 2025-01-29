// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "GameRes.h"

bool IsEnglishVersion() {
#ifdef ENGLISH
  return true;
#else
  return false;
#endif
}

bool IsGermanVersion() {
#ifdef GERMAN
  return true;
#else
  return false;
#endif
}

bool IsPolishVersion() {
#ifdef POLISH
  return true;
#else
  return false;
#endif
}

bool IsRussianVersion() {
#ifdef RUSSIAN
  return true;
#else
  return false;
#endif
}

bool IsRussianGoldVersion() {
#ifdef RUSSIAN_GOLD
  return true;
#else
  return false;
#endif
}
