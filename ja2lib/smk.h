// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <stdbool.h>
#include <stdint.h>

struct SmkFlic;

bool SmkPollFlics();

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight);

void SmkShutdown();

struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, bool fClose);

void SmkCloseFlic(struct SmkFlic *pSmack);
