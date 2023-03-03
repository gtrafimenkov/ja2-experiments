#ifndef __OVERHEADMAP_H
#define __OVERHEADMAP_H

#include "MouseInput.h"
#include "SGP/Types.h"

struct MOUSE_REGION;

void InitNewOverheadDB(UINT8 ubTilesetID);
void RenderOverheadMap(INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S,
                       INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS, BOOLEAN fFromMapUtility);

void HandleOverheadMap(const struct MouseInput mouse);
BOOLEAN InOverheadMap();
void GoIntoOverheadMap();
void HandleOverheadUI(const struct MouseInput mouse);
void KillOverheadMap(const struct MouseInput mouse);

void ClickOverheadRegionCallback(struct MOUSE_REGION *reg, INT32 reason,
                                 const struct MouseInput mouse);

void CalculateRestrictedMapCoords(INT8 bDirection, INT16 *psX1, INT16 *psY1, INT16 *psX2,
                                  INT16 *psY2, INT16 sEndXS, INT16 sEndYS);
void CalculateRestrictedScaleFactors(INT16 *pScaleX, INT16 *pScaleY);

void TrashOverheadMap();

BOOLEAN GetOverheadMouseGridNo(INT16 *psGridNo, const struct MouseInput mouse);
BOOLEAN GetOverheadMouseGridNoForFullSoldiersGridNo(INT16 *psGridNo, const struct MouseInput mouse);

#endif
