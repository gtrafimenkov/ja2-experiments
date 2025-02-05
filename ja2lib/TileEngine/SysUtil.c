// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/SysUtil.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"

struct VSurface *vsSaveBuffer = NULL;
struct VSurface *vsExtraBuffer = NULL;

BOOLEAN gfExtraBuffer = FALSE;

BOOLEAN InitializeSystemVideoObjects() { return (TRUE); }

BOOLEAN InitializeGameVideoObjects() {
  uint16_t usWidth;
  uint16_t usHeight;

  GetCurrentVideoSettings(&usWidth, &usHeight);

  vsSaveBuffer = CreateVSurfaceBlank16(usWidth, usHeight);
  if (vsSaveBuffer == NULL) {
    return FALSE;
  }
  JSurface_SetColorKey(vsSaveBuffer, FROMRGB(0, 0, 0));

  vsExtraBuffer = CreateVSurfaceBlank16(usWidth, usHeight);
  gfExtraBuffer = TRUE;

  return (TRUE);
}
