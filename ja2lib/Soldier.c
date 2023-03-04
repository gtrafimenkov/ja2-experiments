#include "Tactical/Menptr.h"

// Get soldier by index.
// Valid indeces are [0..TOTAL_SOLDIERS).
struct SOLDIERTYPE* GetSoldierByID(int index) { return &Menptr[index]; }
