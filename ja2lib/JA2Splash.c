// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <stdio.h>

#include "MainMenuScreen.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/TopicIDs.h"
#include "SGP/TopicOps.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/TimerControl.h"
#include "platform.h"

uint32_t guiSplashFrameFade = 10;
uint32_t guiSplashStartTime = 0;

// Simply create videosurface, load image, and draw it to the screen.
void InitJA2SplashScreen() {
  char CurrentDir[256];
  char DataDir[300];

  InitializeJA2Clock();
  // Get Executable Directory
  Plat_GetExecutableDirectory(CurrentDir, sizeof(CurrentDir));

  // Adjust Current Dir
  snprintf(DataDir, ARR_SIZE(DataDir), "%s\\Data", CurrentDir);
  if (!Plat_SetCurrentDirectory(DataDir)) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Could not find data directory, shutting down");
    return;
  }

  // Initialize the file database
  InitializeFileDatabase();

#if !defined(ENGLISH) && defined(JA2TESTVERSION)
  uint32_t uiLogoID = 0;
  VSURFACE_DESC VSurfaceDesc;
  struct VSurface* hVSurface;
  memset(&VSurfaceDesc, 0, sizeof(VSURFACE_DESC));
  VSurfaceDesc.fCreateFlags = VSURFACE_CREATE_FROMFILE | VSURFACE_SYSTEM_MEM_USAGE;
  sprintf(VSurfaceDesc.ImageFile, "LOADSCREENS\\Notification.sti");
  if (!AddVideoSurface(&VSurfaceDesc, &uiLogoID)) {
    AssertMsg(0, String("Failed to load %s", VSurfaceDesc.ImageFile));
    return;
  }
  GetVideoSurface(&hVSurface, uiLogoID);
  BltVideoSurfaceToVideoSurface(ghFrameBuffer, hVSurface, 0, 0, 0, 0, NULL);
  DeleteVideoSurfaceFromIndex(uiLogoID);

  InvalidateScreen();
  RefreshScreen(NULL);

  guiSplashStartTime = GetJA2Clock();
  while (i < 60 * 15)  // guiSplashStartTime + 15000 > GetJA2Clock() )
  {
    // Allow the user to pick his bum.
    InvalidateScreen();
    RefreshScreen(NULL);
    i++;
  }
#endif

#ifdef ENGLISH
  ClearMainMenu();
#else
  {
    memset(&VSurfaceDesc, 0, sizeof(VSURFACE_DESC));
    VSurfaceDesc.fCreateFlags = VSURFACE_CREATE_FROMFILE | VSURFACE_SYSTEM_MEM_USAGE;
    GetMLGFilename(VSurfaceDesc.ImageFile, MLG_SPLASH);
    if (!AddVideoSurface(&VSurfaceDesc, &uiLogoID)) {
      AssertMsg(0, String("Failed to load %s", VSurfaceDesc.ImageFile));
      return;
    }

    GetVideoSurface(&hVSurface, uiLogoID);
    BltVideoSurfaceToVideoSurface(ghFrameBuffer, hVSurface, 0, 0, 0, 0, NULL);
    DeleteVideoSurfaceFromIndex(uiLogoID);
  }
#endif

  InvalidateScreen();
  RefreshScreen(NULL);

  guiSplashStartTime = GetJA2Clock();
}
