#ifndef __SOLDIER_H
#define __SOLDIER_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE* GetSoldierByID(int index);

#endif
