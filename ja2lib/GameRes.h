// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef JA2_GAME_RES
#define JA2_GAME_RES

#include <stdbool.h>
#include <stdint.h>

// Known type of game resources
enum ResourceVersion {
  RT_ENGLISH,
  RT_FRENCH,
  RT_GERMAN,
  RT_POLISH,
  RT_RUSSIAN_BUKA,
  RT_RUSSIAN_GOLD
};

void DetectResourcesVersion();
enum ResourceVersion GetResourceVersion();

bool UsingEnglishResources();
bool UsingFrenchResources();
bool UsingGermanResources();
bool UsingPolishResources();
bool UsingRussianBukaResources();
bool UsingRussianGoldResources();

wchar_t GetZeroGlyphChar();

void UseTextEnglish();
void UseTextFrench();
void UseTextGerman();
void UseTextRussian();
void UseTextPolish();
void UseTextItalian();
void UseTextDutch();

#endif JA2_GAME_RES
