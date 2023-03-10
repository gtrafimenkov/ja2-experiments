#ifndef __NEWSMOOTH_H
#define __NEWSMOOTH_H

#include "BuildDefines.h"
#include "SGP/Types.h"

void AddBuildingSectionToWorld(SGPRect *pSelectRegion);
void RemoveBuildingSectionFromWorld(SGPRect *pSelectRegion);

void AddCaveSectionToWorld(SGPRect *pSelectRegion);
void RemoveCaveSectionFromWorld(SGPRect *pSelectRegion);

void EraseBuilding(UINT32 iMapIndex);
void RebuildRoof(UINT32 iMapIndex, UINT16 usRoofType);
void RebuildRoofUsingFloorInfo(INT32 iMapIndex, UINT16 usRoofType);

void AddCave(INT32 iMapIndex, UINT16 usIndex);

void AnalyseCaveMapForStructureInfo();

#endif
