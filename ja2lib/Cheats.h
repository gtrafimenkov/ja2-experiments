// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _CHEATS__H_
#define _CHEATS__H_

#include "SGP/Types.h"

extern uint8_t gubCheatLevel;

// GLOBALS FOR CHEAT MODE......
#ifdef JA2TESTVERSION
#define STARTING_CHEAT_LEVEL 6
#elif defined JA2BETAVERSION
#define STARTING_CHEAT_LEVEL 3
#else
#define STARTING_CHEAT_LEVEL 0
#endif

// ATE: remove cheats unless we're doing a debug build
// #ifdef JA2TESTVERSION
#define INFORMATION_CHEAT_LEVEL() (gubCheatLevel >= 3)
#define CHEATER_CHEAT_LEVEL() (gubCheatLevel >= 5)
#define DEBUG_CHEAT_LEVEL() (gubCheatLevel >= 6)
// #else
//	#define						INFORMATION_CHEAT_LEVEL( )
//( FALSE ) 	#define						CHEATER_CHEAT_LEVEL( ) ( FALSE )
//	#define						DEBUG_CHEAT_LEVEL( )
//( FALSE ) #endif

#define RESET_CHEAT_LEVEL() (gubCheatLevel = 0)
#endif
