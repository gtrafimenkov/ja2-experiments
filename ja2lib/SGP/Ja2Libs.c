// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <stdio.h>
#include <string.h>

#include "SGP/LibraryDataBase.h"
#include "platform.h"

// int8_t gbLocale = ENGLISH_LANG;

// char* LocaleNames[LANG_NUMBER] = {"default", "russian", "german", "dutch",
//                                   "polish",  "french",  "italian"};

// #define _MAX_PATH 260

// int8_t DetectLocale() {
//   int8_t bLoc;
//   char zPath[_MAX_PATH];
//   char zLocalePath[_MAX_PATH];

//   if (!Plat_GetExecutableDirectory(zPath, sizeof(zPath))) {
//     return ENGLISH_LANG;
//   }

//   strcpy(zPath, "\\Data\\");

//   for (bLoc = RUSSIAN_LANG; bLoc < LANG_NUMBER; bLoc++) {
//     strcpy(zLocalePath, zPath);
//     strcat(zLocalePath, LocaleNames[bLoc]);
//     strcat(zLocalePath, ".slf");

//     if (Plat_FileEntityExists(zLocalePath)) {
//       gbLocale = bLoc;
//       snprintf(gGameLibaries[LIBRARY_NATIONAL_DATA].sLibraryName,
//                sizeof(gGameLibaries[LIBRARY_NATIONAL_DATA].sLibraryName), "%s.slf",
//                LocaleNames[gbLocale]);
//       break;
//     }
//   }
//   return gbLocale;
// }
