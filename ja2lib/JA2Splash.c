// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <stdio.h>

#include "GameRes.h"
#include "MainMenuScreen.h"
#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/TopicIDs.h"
#include "SGP/TopicOps.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/TimerControl.h"
#include "platform.h"

uint32_t guiSplashFrameFade = 10;
uint32_t guiSplashStartTime = 0;

// Simply create videosurface, load image, and draw it to the screen.
void InitJA2SplashScreen() {
#if defined(JA2TESTVERSION)
  if (!UsingEnglishResources()) {
    uint32_t uiLogoID = 0;
    struct VSurface* hVSurface;
    if (!AddVSurfaceFromFile("LOADSCREENS\\Notification.sti", &uiLogoID)) {
      AssertMsg(0, String("Failed to load %s", "LOADSCREENS\\Notification.sti"));
      return;
    }
    GetVSurfaceByIndexOld(&hVSurface, uiLogoID);
    BltVSurfaceToVSurface(vsFB, hVSurface, 0, 0, 0, 0, NULL);
    DeleteVSurfaceByIndex(uiLogoID);

    InvalidateScreen();
    RefreshScreen(NULL);

    guiSplashStartTime = GetJA2Clock();
    int32_t i = 0;
    while (i < 60 * 15)  // guiSplashStartTime + 15000 > GetJA2Clock() )
    {
      // Allow the user to pick his bum.
      InvalidateScreen();
      RefreshScreen(NULL);
      i++;
    }
  }
#endif

  if (UsingEnglishResources()) {
    ClearMainMenu();
  } else {
    uint32_t uiLogoID = 0;
    struct VSurface* hVSurface;
    SGPFILENAME ImageFile;
    GetMLGFilename(ImageFile, MLG_SPLASH);
    if (!AddVSurfaceFromFile(ImageFile, &uiLogoID)) {
      AssertMsg(0, String("Failed to load %s", ImageFile));
      return;
    }

    GetVSurfaceByIndexOld(&hVSurface, uiLogoID);
    BltVSurfaceToVSurface(vsFB, hVSurface, 0, 0, 0, 0, NULL);
    DeleteVSurfaceByIndex(uiLogoID);
  }

  InvalidateScreen();
  RefreshScreen(NULL);

  guiSplashStartTime = GetJA2Clock();
}
