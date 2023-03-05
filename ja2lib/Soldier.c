#include "Soldier.h"

#include "Militia.h"
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

// feed this a SOLDIER_CLASS_, it will return you a _MITILIA rank, or -1 if the guy's not militia
INT8 SoldierClassToMilitiaRank(UINT8 ubSoldierClass) {
  INT8 bRank = -1;

  switch (ubSoldierClass) {
    case SOLDIER_CLASS_GREEN_MILITIA:
      bRank = GREEN_MILITIA;
      break;
    case SOLDIER_CLASS_REG_MILITIA:
      bRank = REGULAR_MILITIA;
      break;
    case SOLDIER_CLASS_ELITE_MILITIA:
      bRank = ELITE_MILITIA;
      break;
  }

  return (bRank);
}

// feed this a _MITILIA rank, it will return you a SOLDIER_CLASS_, or -1 if the guy's not militia
INT8 MilitiaRankToSoldierClass(UINT8 ubRank) {
  INT8 bSoldierClass = -1;

  switch (ubRank) {
    case GREEN_MILITIA:
      bSoldierClass = SOLDIER_CLASS_GREEN_MILITIA;
      break;
    case REGULAR_MILITIA:
      bSoldierClass = SOLDIER_CLASS_REG_MILITIA;
      break;
    case ELITE_MILITIA:
      bSoldierClass = SOLDIER_CLASS_ELITE_MILITIA;
      break;
  }

  return (bSoldierClass);
}
