// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/SGP.h"

#include <stdio.h>

#include "BuildDefines.h"
#include "DebugLog.h"
#include "GameLoop.h"
#include "GameRes.h"
#include "Globals.h"
#include "JA2Splash.h"
#include "Laptop/Laptop.h"
#include "Local.h"
#include "SGP/FileMan.h"
#include "SGP/Font.h"
#include "SGP/Input.h"
#include "SGP/LibraryDataBasePub.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Timer.h"
#include "SGP/TranslationTable.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/TimerControl.h"
#include "platform.h"

void SGPExit(void);

BOOLEAN InitializeStandardGamingPlatform(struct PlatformInitParams* params, const char* dataDir) {
  // now required by all (even JA2) in order to call ShutdownSGP
  atexit(SGPExit);

  // Initialize the Debug Manager - success doesn't matter
#ifdef SGP_DEBUG
  InitializeDebugManager();
#endif

  // Now start up everything else.
  RegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

  // this one needs to go ahead of all others (except Debug), for MemDebugCounter to work right...
  FastDebugMsg("Initializing Memory Manager");
  // Initialize the Memory Manager
  if (InitializeMemoryManager() == FALSE) {  // We were unable to initialize the memory manager
    FastDebugMsg("FAILED : Initializing Memory Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing File Manager");
  // Initialize the File Manager
  if (FileMan_Initialize() == FALSE) {  // We were unable to initialize the file manager
    FastDebugMsg("FAILED : Initializing File Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Containers Manager");
  InitializeContainers();

  FastDebugMsg("Initializing Input Manager");
  // Initialize the Input Manager
  if (InitializeInputManager() == FALSE) {  // We were unable to initialize the input manager
    FastDebugMsg("FAILED : Initializing Input Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Video Manager");
  // Initialize DirectDraw (DirectX 2)
  if (InitializeVideoManager(params) == FALSE) {
    FastDebugMsg("FAILED : Initializing Video Manager");
    return FALSE;
  }

  // Initialize Video Object Manager
  FastDebugMsg("Initializing Video Object Manager");
  if (!InitializeVideoObjectManager()) {
    FastDebugMsg("FAILED : Initializing Video Object Manager");
    return FALSE;
  }

  // Initialize Video Surface Manager
  FastDebugMsg("Initializing Video Surface Manager");
  if (!InitializeVideoSurfaceManager()) {
    FastDebugMsg("FAILED : Initializing Video Surface Manager");
    return FALSE;
  }

  {
    InitializeJA2Clock();

    char CurrentDir[256];
    Plat_GetExecutableDirectory(CurrentDir, sizeof(CurrentDir));

    // Adjust Current Dir
    char fullDataDir[300];
    snprintf(fullDataDir, ARR_SIZE(fullDataDir), "%s\\%s", CurrentDir, dataDir ? dataDir : "Data");
    DebugLogF("data directory: %s", fullDataDir);
    if (!Plat_SetCurrentDirectory(fullDataDir)) {
      FatalError("Data directory '%s' is not found", fullDataDir);
      return FALSE;
    }

    // Initialize the file database
    InitializeFileDatabase();
    DetectResourcesVersion();

    // always set the english first because localizations may be not complete
    UseTextEnglish();

    UseTextLanguagaeMatchingGameResourcesType();
    // UseTextFrench();
    // UseTextGerman();
    // UseTextRussian();
    // UseTextPolish();
    // UseTextItalian();
    // UseTextDutch();

    SelectCorrectTranslationTable();
  }
  InitJA2SplashScreen();

  // Make sure we start up our local clock (in milliseconds)
  // We don't need to check for a return value here since so far its always TRUE
  InitializeClockManager();  // must initialize after VideoManager, 'cause it uses ghWindow

  // Initialize Font Manager
  FastDebugMsg("Initializing the Font Manager");
  // Init the manager and copy the TransTable stuff into it.
  if (!InitializeFontManager(8)) {
    FastDebugMsg("FAILED : Initializing Font Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Sound Manager");
  // Initialize the Sound Manager (DirectSound)
  if (InitializeSoundManager() == FALSE) {  // We were unable to initialize the sound manager
    FastDebugMsg("FAILED : Initializing Sound Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing Random");
  // Initialize random number generator
  InitializeRandom();  // no Shutdown

  FastDebugMsg("Initializing Game Manager");
  // Initialize the Game
  if (InitializeGame() == FALSE) {  // We were unable to initialize the game
    FastDebugMsg("FAILED : Initializing Game Manager");
    return FALSE;
  }

  return TRUE;
}

void SGPExit(void) {
  static BOOLEAN fAlreadyExiting = FALSE;

  // helps prevent heap crashes when multiple assertions occur and call us
  if (fAlreadyExiting) {
    return;
  }

  fAlreadyExiting = TRUE;
  gfProgramIsRunning = FALSE;

  ShutdownStandardGamingPlatform();
  Plat_OnSGPExit();
}
