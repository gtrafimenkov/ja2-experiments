#include "Tactical/Menptr.h"

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE *GetSoldierByID(int index) { return &Menptr[index]; }

i16 GetSolSectorX(const struct SOLDIERTYPE *s) { return s->sSectorX; }
i16 GetSolSectorY(const struct SOLDIERTYPE *s) { return s->sSectorY; }
i8 GetSolSectorZ(const struct SOLDIERTYPE *s) { return s->bSectorZ; }
bool IsSolActive(const struct SOLDIERTYPE *s) { return s->bActive; }
