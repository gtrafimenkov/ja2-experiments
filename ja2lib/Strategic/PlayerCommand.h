#ifndef _PLAYER_COMMAND_H
#define _PLAYER_COMMAND_H

#include "SGP/Types.h"

// build main facilities strings for sector
void GetSectorFacilitiesFlags(u8 sMapX, u8 sMapY, STR16 sFacilitiesString, size_t bufSize);

// set sector as enemy controlled
BOOLEAN SetThisSectorAsEnemyControlled(u8 sMapX, u8 sMapY, INT8 bMapZ, BOOLEAN fContested);

// set sector as player controlled
BOOLEAN SetThisSectorAsPlayerControlled(u8 sMapX, u8 sMapY, INT8 bMapZ, BOOLEAN fContested);

#ifdef JA2TESTVERSION
void ClearMapControlledFlags(void);
#endif

/*
// is this sector under player control
BOOLEAN IsTheSectorPerceivedToBeUnderEnemyControl( INT16 sMapX, INT16 sMapY, INT8 bMapZ );

// make player's perceived control over the sector reflect reality
void MakePlayerPerceptionOfSectorControlCorrect( INT16 sMapX, INT16 sMapY, INT8 bMapZ );
*/

void ReplaceSoldierProfileInPlayerGroup(UINT8 ubGroupID, UINT8 ubOldProfile, UINT8 ubNewProfile);

#endif
