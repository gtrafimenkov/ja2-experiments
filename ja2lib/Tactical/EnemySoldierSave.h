// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __ENEMY_SOLDIER_SAVE_H
#define __ENEMY_SOLDIER_SAVE_H

#include "SGP/Types.h"

// This funciton load both the enemies AND the civilians, now only used to load old saves
BOOLEAN LoadEnemySoldiersFromTempFile();

// The new way of loading in enemies from the temp file.  ONLY the ENEMIES are stored in the e_*
// temp file
BOOLEAN NewWayOfLoadingEnemySoldiersFromTempFile();

// The new way of loading in CIVILIANS from the temp file.  ONLY the CIVILIEANS are stored in the
// c_* temp file
BOOLEAN NewWayOfLoadingCiviliansFromTempFile();

BOOLEAN NewWayOfSavingEnemyAndCivliansToTempFile(uint8_t sSectorX, uint8_t sSectorY,
                                                 int8_t bSectorZ, BOOLEAN fEnemy,
                                                 BOOLEAN fValidateOnly);

BOOLEAN SaveEnemySoldiersToTempFile(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ,
                                    uint8_t ubFirstIdTeam, uint8_t ubLastIdTeam,
                                    BOOLEAN fAppendToFile);

extern BOOLEAN gfRestoringEnemySoldiersFromTempFile;

#endif
