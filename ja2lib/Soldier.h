#ifndef __SOLDIER_H
#define __SOLDIER_H

#include "SGP/Types.h"

struct SOLDIERTYPE;

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE *GetSoldierByID(int index);

i16 GetSolSectorX(const struct SOLDIERTYPE *s);
i16 GetSolSectorY(const struct SOLDIERTYPE *s);
i8 GetSolSectorZ(const struct SOLDIERTYPE *s);

bool IsSolActive(const struct SOLDIERTYPE *s);

#endif
