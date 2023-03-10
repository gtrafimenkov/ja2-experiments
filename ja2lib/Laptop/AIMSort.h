#ifndef __AIMSORT_H_
#define __AIMSORT_H_

#include "SGP/Types.h"

extern UINT8 gubCurrentSortMode;
extern UINT8 gubCurrentListMode;
extern UINT8 gbCurrentIndex;

#define AIM_ASCEND 6
#define AIM_DESCEND 7

void GameInitAimSort();
BOOLEAN EnterAimSort();
void ExitAimSort();
void HandleAimSort();
void RenderAimSort();

#endif
