// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "TileEngine/SysUtil.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "SGP/HImage.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"

uint32_t vsSB = 0;
uint32_t guiEXTRABUFFER = 0;

BOOLEAN gfExtraBuffer = FALSE;

BOOLEAN InitializeSystemVideoObjects() { return (TRUE); }

BOOLEAN InitializeGameVideoObjects() {
  uint16_t usWidth;
  uint16_t usHeight;

  GetCurrentVideoSettings(&usWidth, &usHeight);
  CHECKF(AddVSurface(CreateVSurfaceBlank16(usWidth, usHeight), &vsSB));
  CHECKF(AddVSurface(CreateVSurfaceBlank16(usWidth, usHeight), &guiEXTRABUFFER));
  gfExtraBuffer = TRUE;

  return (TRUE);
}
