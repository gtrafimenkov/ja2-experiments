// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef QUEST_DEBUG_H
#define QUEST_DEBUG_H

#include "SGP/Types.h"

// Type of quote
enum {
  QD_NPC_MSG,
  QD_QUEST_MSG,
};

// Output priority level for the npc and quest debug messages
enum {
  QD_LEVEL_1,
  QD_LEVEL_2,
  QD_LEVEL_3,
  QD_LEVEL_4,
  QD_LEVEL_5,
};

void ToggleQuestDebugModes(uint8_t ubType);
void QuestDebugFileMsg(uint8_t ubQuoteType, uint8_t ubPriority, char* pStringA, ...);

#endif
