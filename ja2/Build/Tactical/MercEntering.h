#ifndef _MERC_ENTRING_H
#define _MERC_ENTRING_H

#include "SGP/Types.h"

void ResetHeliSeats();
void AddMercToHeli(UINT8 ubID);

void StartHelicopterRun(INT16 sGridNoSweetSpot);

void HandleHeliDrop();

extern BOOLEAN gfIngagedInDrop;

#endif
