#include "Tactical/Menptr.h"

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE *GetSoldierByID(int index) { return &Menptr[index]; }

u8 GetSolID(const struct SOLDIERTYPE *s) { return s->ubID; }
u8 GetSolProfile(const struct SOLDIERTYPE *s) { return s->ubProfile; }

i16 GetSolSectorX(const struct SOLDIERTYPE *s) { return s->sSectorX; }
i16 GetSolSectorY(const struct SOLDIERTYPE *s) { return s->sSectorY; }
i8 GetSolSectorZ(const struct SOLDIERTYPE *s) { return s->bSectorZ; }

i8 GetSolAssignment(const struct SOLDIERTYPE *s) { return s->bAssignment; }

bool IsSolActive(const struct SOLDIERTYPE *s) { return s->bActive; }

void SetSolAssignmentDone(struct SOLDIERTYPE *s) {
  s->fDoneAssignmentAndNothingToDoFlag = FALSE;
  s->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;
}
