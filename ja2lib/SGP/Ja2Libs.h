// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _JA2_LIBS_H_
#define _JA2_LIBS_H_

#include "SGP/Types.h"

enum {
  ENGLISH_LANG,
  RUSSIAN_LANG,
  GERMAN_LANG,
  DUTCH_LANG,
  POLISH_LANG,
  FRENCH_LANG,
  ITALIAN_LANG,

  LANG_NUMBER
};

extern int8_t gbLocale;

int8_t DetectLocale();

extern char* LocaleNames[LANG_NUMBER];

#define NUMBER_OF_LIBRARIES \
  ((gbLocale != ENGLISH_LANG) ? FULL_NUMBER_OF_LIBRARIES : (FULL_NUMBER_OF_LIBRARIES - 1))

// enums used for accessing the libraries
enum {
  LIBRARY_DATA,
  LIBRARY_AMBIENT,
  LIBRARY_ANIMS,
  LIBRARY_BATTLESNDS,
  LIBRARY_BIGITEMS,
  LIBRARY_BINARY_DATA,
  LIBRARY_CURSORS,
  LIBRARY_FACES,
  LIBRARY_FONTS,
  LIBRARY_INTERFACE,
  LIBRARY_LAPTOP,
  LIBRARY_MAPS,
  LIBRARY_MERCEDT,
  LIBRARY_MUSIC,
  LIBRARY_NPC_SPEECH,
  LIBRARY_NPC_DATA,
  LIBRARY_RADAR_MAPS,
  LIBRARY_SOUNDS,
  LIBRARY_SPEECH,
  //		LIBRARY_TILE_CACHE,
  LIBRARY_TILESETS,
  LIBRARY_LOADSCREENS,
  LIBRARY_INTRO,
  LIBRARY_NATIONAL_DATA,

  FULL_NUMBER_OF_LIBRARIES
};

#endif
