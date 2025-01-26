#ifndef TRANSLATIONTABLE_H
#define TRANSLATIONTABLE_H

#include <stdint.h>

#define TranslationTableSize 0x452

/* The code point to glyph map table. */
extern uint8_t const TranslationTable[TranslationTableSize];

#endif
