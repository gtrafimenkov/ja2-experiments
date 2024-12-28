// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Money.h"

#include "Laptop/LaptopSave.h"

// Get money balance (what is shown on the laptop screen).
int32_t MoneyGetBalance() { return LaptopSaveInfo.iCurrentBalance; }

// Get money balance (what is shown on the laptop screen).
void MoneySetBalance(int32_t value) { LaptopSaveInfo.iCurrentBalance = value; }
