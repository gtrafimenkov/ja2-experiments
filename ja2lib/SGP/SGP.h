// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __SGP_H
#define __SGP_H

#include "SGP/Types.h"
#include "jplatform_video.h"

extern BOOLEAN InitializeStandardGamingPlatform(struct JVideoInitParams* videoInitParams,
                                                const char* dataDir);
extern void ShutdownStandardGamingPlatform(void);

#endif
