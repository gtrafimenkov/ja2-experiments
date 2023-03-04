#include "Militia.h"

#include "SGP/Types.h"
#include "Utils/Text.h"

i8 gbGreenToElitePromotions = 0;
i8 gbGreenToRegPromotions = 0;
i8 gbRegToElitePromotions = 0;
i8 gbMilitiaPromotions = 0;

void BuildMilitiaPromotionsString(CHAR16 *str, size_t bufSize) {
  CHAR16 pStr[256];
  BOOLEAN fAddSpace = FALSE;
  swprintf(str, bufSize, L"");

  if (!gbMilitiaPromotions) {
    return;
  }
  if (gbGreenToElitePromotions > 1) {
    swprintf(pStr, ARR_SIZE(pStr), gzLateLocalizedString[22], gbGreenToElitePromotions);
    wcsncat(str, pStr, bufSize);
    fAddSpace = TRUE;
  } else if (gbGreenToElitePromotions == 1) {
    wcsncat(str, gzLateLocalizedString[29], bufSize);
    fAddSpace = TRUE;
  }

  if (gbGreenToRegPromotions > 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    swprintf(pStr, ARR_SIZE(pStr), gzLateLocalizedString[23], gbGreenToRegPromotions);
    wcsncat(str, pStr, bufSize);
    fAddSpace = TRUE;
  } else if (gbGreenToRegPromotions == 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    wcsncat(str, gzLateLocalizedString[30], bufSize);
    fAddSpace = TRUE;
  }

  if (gbRegToElitePromotions > 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    swprintf(pStr, ARR_SIZE(pStr), gzLateLocalizedString[24], gbRegToElitePromotions);
    wcsncat(str, pStr, bufSize);
  } else if (gbRegToElitePromotions == 1) {
    if (fAddSpace) {
      wcsncat(str, L" ", bufSize);
    }
    wcsncat(str, gzLateLocalizedString[31], bufSize);
    fAddSpace = TRUE;
  }

  // Clear the fields
  gbGreenToElitePromotions = 0;
  gbGreenToRegPromotions = 0;
  gbRegToElitePromotions = 0;
  gbMilitiaPromotions = 0;
}
