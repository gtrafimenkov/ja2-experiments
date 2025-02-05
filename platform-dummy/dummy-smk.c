// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <stdbool.h>
#include <stdint.h>

bool SmkPollFlics() { return false; }

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight) {}

void SmkShutdown() {}

struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, bool fClose) {
  return NULL;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {}
