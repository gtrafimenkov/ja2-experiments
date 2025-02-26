// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/FogOfWar.h"

#include "SGP/Types.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/SimpleRenderUtils.h"
#include "TileEngine/WorldMan.h"

// When line of sight reaches a gridno, and there is a light there, it turns it on.
// This is only done in the cave levels.
void RemoveFogFromGridNo(uint32_t uiGridNo) {
  int32_t i;
  int32_t x, y;
  x = uiGridNo % WORLD_COLS;
  y = uiGridNo / WORLD_COLS;
  for (i = 0; i < MAX_LIGHT_SPRITES; i++) {
    if (LightSprites[i].iX == x && LightSprites[i].iY == y) {
      if (!(LightSprites[i].uiFlags & LIGHT_SPR_ON)) {
        LightSpritePower(i, TRUE);
        LightDraw(LightSprites[i].uiLightType, LightSprites[i].iTemplate, LightSprites[i].iX,
                  LightSprites[i].iY, i);
        MarkWorldDirty();
        return;
      }
    }
  }
}
