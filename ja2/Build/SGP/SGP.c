#include "SGP/SGP.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>

#include "BuildDefines.h"
#include "GameLoop.h"
#include "Intro.h"
#include "JA2Splash.h"
#include "Laptop/Laptop.h"
#include "Local.h"
#include "SGP/ExceptionHandling.h"
#include "SGP/FileMan.h"
#include "SGP/Font.h"
#include "SGP/Input.h"
#include "SGP/Random.h"
#include "SGP/RegInst.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "Tactical/InterfacePanels.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "dbt.h"
#include "zmouse.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

extern UINT32 MemDebugCounter;
extern BOOLEAN gfPauseDueToPlayerGamePause;

extern BOOLEAN CheckIfGameCdromIsInCDromDrive();
extern void QueueEvent(UINT16 ubInputEvent, UINT32 usParam, UINT32 uiParam);

// Prototype Declarations

INT32 FAR PASCAL WindowProcedure(HWND hWindow, UINT16 Message, WPARAM wParam, LPARAM lParam);
BOOLEAN InitializeStandardGamingPlatform(HINSTANCE hInstance, int sCommandShow);
void ShutdownStandardGamingPlatform(void);
void GetRuntimeSettings();

int PASCAL HandledWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                          int sCommandShow);

#if !defined(JA2) && !defined(UTILS)
void ProcessCommandLine(CHAR8 *pCommandLine);
BOOLEAN RunSetup(void);

// Should the game immediately load the quick save at startup?
BOOLEAN gfLoadAtStartup = FALSE;
BOOLEAN gfUsingBoundsChecker = FALSE;
CHAR8 *gzStringDataOverride = NULL;
BOOLEAN gfCapturingVideo = FALSE;

#endif

HINSTANCE ghInstance;

void ProcessJa2CommandLineBeforeInitialization(CHAR8 *pCommandLine);

// Global Variable Declarations
#ifdef WINDOWED_MODE
RECT rcWindow;
#endif

// moved from header file: 24mar98:HJH
UINT32 giStartMem;

UINT32 guiMouseWheelMsg;  // For mouse wheel messages

BOOLEAN gfApplicationActive;
BOOLEAN gfProgramIsRunning;
BOOLEAN gfGameInitialized = FALSE;
BOOLEAN gfDontUseDDBlits = FALSE;

// There were TWO of them??!?! -- DB
// CHAR8		gzCommandLine[ 100 ];
CHAR8 gzCommandLine[100];  // Command line given

CHAR8 gzErrorMsg[2048] = "";
BOOLEAN gfIgnoreMessages = FALSE;

// GLOBAL VARIBLE, SET TO DEFAULT BUT CAN BE CHANGED BY THE GAME IF INIT FILE READ
UINT8 gbPixelDepth = PIXEL_DEPTH;

INT32 FAR PASCAL WindowProcedure(HWND hWindow, UINT16 Message, WPARAM wParam, LPARAM lParam) {
  static BOOLEAN fRestore = FALSE;

  if (gfIgnoreMessages) return (DefWindowProc(hWindow, Message, wParam, lParam));

  // ATE: This is for older win95 or NT 3.51 to get MOUSE_WHEEL Messages
  if (Message == guiMouseWheelMsg) {
    QueueEvent(MOUSE_WHEEL, wParam, lParam);
    return (0L);
  }

  switch (Message) {
    case WM_MOUSEWHEEL: {
      QueueEvent(MOUSE_WHEEL, wParam, lParam);
      break;
    }

#ifdef WINDOWED_MODE
    case WM_MOVE:

      GetClientRect(hWindow, &rcWindow);
      ClientToScreen(hWindow, (LPPOINT)&rcWindow);
      ClientToScreen(hWindow, (LPPOINT)&rcWindow + 1);
      break;
#endif

    case WM_ACTIVATEAPP:
      switch (wParam) {
        case TRUE:  // We are restarting DirectDraw
          if (fRestore == TRUE) {
            RestoreVideoManager();
            RestoreVideoSurfaces();  // Restore any video surfaces

            // unpause the JA2 Global clock
            if (!gfPauseDueToPlayerGamePause) {
              PauseTime(FALSE);
            }

            gfApplicationActive = TRUE;
          }
          break;
        case FALSE:  // We are suspending direct draw
                     // pause the JA2 Global clock
          PauseTime(TRUE);
          SuspendVideoManager();

          gfApplicationActive = FALSE;
          fRestore = TRUE;
          break;
      }
      break;

    case WM_CREATE:
      break;

    case WM_DESTROY:
      ShutdownStandardGamingPlatform();
      ShowCursor(TRUE);
      PostQuitMessage(0);
      break;

    case WM_SETFOCUS:
#if !defined(JA2) && !defined(UTIL)
      if (!VideoInspectorIsEnabled()) RestoreVideoManager();
      gfApplicationActive = TRUE;
//			RestrictMouseToXYXY(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
#else
      RestoreCursorClipRect();
#endif

      break;

    case WM_KILLFOCUS:
#if !defined(JA2) && !defined(UTIL)
      if (!VideoInspectorIsEnabled()) SuspendVideoManager();

      gfApplicationActive = FALSE;
      FreeMouseCursor();
#endif
      // Set a flag to restore surfaces once a WM_ACTIVEATEAPP is received
      fRestore = TRUE;
      break;

#if defined(JA2)
#ifndef JA2DEMO
    case WM_DEVICECHANGE: {
      DEV_BROADCAST_HDR *pHeader = (DEV_BROADCAST_HDR *)lParam;

      // if a device has been removed
      if (wParam == DBT_DEVICEREMOVECOMPLETE) {
        // if its  a disk
        if (pHeader->dbch_devicetype == DBT_DEVTYP_VOLUME) {
          // check to see if the play cd is still in the cdrom
          if (!CheckIfGameCdromIsInCDromDrive()) {
          }
        }
      }
    } break;
#endif
#endif

    default:
      return DefWindowProc(hWindow, Message, wParam, lParam);
  }
  return 0L;
}

BOOLEAN InitializeStandardGamingPlatform(HINSTANCE hInstance, int sCommandShow) {
  FontTranslationTable *pFontTable;

  // now required by all (even JA2) in order to call ShutdownSGP
  atexit(SGPExit);

  // First, initialize the registry keys.
  InitializeRegistryKeys("Wizardry8", "Wizardry8key");

  // For rendering DLLs etc.
#ifndef JA2
  AddSubdirectoryToPath("DLL");
#endif

  // Second, read in settings
  GetRuntimeSettings();

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

  FastDebugMsg("Initializing Mutex Manager");
  // Initialize the Dirty Rectangle Manager
  if (InitializeMutexManager() == FALSE) {  // We were unable to initialize the game
    FastDebugMsg("FAILED : Initializing Mutex Manager");
    return FALSE;
  }

  FastDebugMsg("Initializing File Manager");
  // Initialize the File Manager
  if (InitializeFileManager(NULL) == FALSE) {  // We were unable to initialize the file manager
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
  if (InitializeVideoManager(hInstance, (UINT16)sCommandShow, (void *)WindowProcedure) ==
      FALSE) {  // We were unable to initialize the video manager
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

  InitJA2SplashScreen();

  // Make sure we start up our local clock (in milliseconds)
  // We don't need to check for a return value here since so far its always TRUE
  InitializeClockManager();  // must initialize after VideoManager, 'cause it uses ghWindow

  // Create font translation table (store in temp structure)
  pFontTable = CreateEnglishTransTable();
  if (pFontTable == NULL) {
    return (FALSE);
  }

  // Initialize Font Manager
  FastDebugMsg("Initializing the Font Manager");
  // Init the manager and copy the TransTable stuff into it.
  if (!InitializeFontManager(8, pFontTable)) {
    FastDebugMsg("FAILED : Initializing Font Manager");
    return FALSE;
  }
  // Don't need this thing anymore, so get rid of it (but don't de-alloc the contents)
  MemFree(pFontTable);

  FastDebugMsg("Initializing Sound Manager");
  // Initialize the Sound Manager (DirectSound)
#ifndef UTIL
  if (InitializeSoundManager() == FALSE) {  // We were unable to initialize the sound manager
    FastDebugMsg("FAILED : Initializing Sound Manager");
    return FALSE;
  }
#endif

  FastDebugMsg("Initializing Random");
  // Initialize random number generator
  InitializeRandom();  // no Shutdown

  FastDebugMsg("Initializing Game Manager");
  // Initialize the Game
  if (InitializeGame() == FALSE) {  // We were unable to initialize the game
    FastDebugMsg("FAILED : Initializing Game Manager");
    return FALSE;
  }

  // Register mouse wheel message
  guiMouseWheelMsg = RegisterWindowMessage(MSH_MOUSEWHEEL);

  gfGameInitialized = TRUE;

  return TRUE;
}

void ShutdownStandardGamingPlatform(void) {
#ifndef JA2
  static BOOLEAN Reenter = FALSE;

  //
  // Prevent multiple reentry into this function
  //

  if (Reenter == FALSE) {
    Reenter = TRUE;
  } else {
    return;
  }
#endif

  //
  // Shut down the different components of the SGP
  //

  // TEST
  SoundServiceStreams();

  if (gfGameInitialized) {
    ShutdownGame();
  }

  ShutdownButtonSystem();
  MSYS_Shutdown();

#ifndef UTIL
  ShutdownSoundManager();
#endif

  DestroyEnglishTransTable();  // has to go before ShutdownFontManager()
  ShutdownFontManager();

  ShutdownClockManager();  // must shutdown before VideoManager, 'cause it uses ghWindow

#ifdef SGP_VIDEO_DEBUGGING
  PerformVideoInfoDumpIntoFile("SGPVideoShutdownDump.txt", FALSE);
#endif

  ShutdownVideoSurfaceManager();
  ShutdownVideoObjectManager();
  ShutdownVideoManager();

  ShutdownInputManager();
  ShutdownContainers();
  ShutdownFileManager();
  ShutdownMutexManager();

#ifdef EXTREME_MEMORY_DEBUGGING
  DumpMemoryInfoIntoFile("ExtremeMemoryDump.txt", FALSE);
#endif

  ShutdownMemoryManager();  // must go last (except for Debug), for MemDebugCounter to work right...

  //
  // Make sure we unregister the last remaining debug topic before shutting
  // down the debugging layer
  UnRegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

#ifdef SGP_DEBUG
  ShutdownDebugManager();
#endif
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                   int sCommandShow) {
// If we are to use exception handling
#ifdef ENABLE_EXCEPTION_HANDLING
  int Result = -1;

  __try {
    Result = HandledWinMain(hInstance, hPrevInstance, pCommandLine, sCommandShow);
  } __except (RecordExceptionInfo(GetExceptionInformation())) {
    // Do nothing here - RecordExceptionInfo() has already done
    // everything that is needed. Actually this code won't even
    // get called unless you return EXCEPTION_EXECUTE_HANDLER from
    // the __except clause.
  }
  return Result;
}

// Do not place code in between WinMain and Handled WinMain

int PASCAL HandledWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                          int sCommandShow) {
// DO NOT REMOVE, used for exception handing list above in WinMain
#endif
  MSG Message;
  HWND hPrevInstanceWindow;

  // Make sure that only one instance of this application is running at once
  // // Look for prev instance by searching for the window
  hPrevInstanceWindow = FindWindowEx(NULL, NULL, APPLICATION_NAME, APPLICATION_NAME);

  // One is found, bring it up!
  if (hPrevInstanceWindow != NULL) {
    SetForegroundWindow(hPrevInstanceWindow);
    ShowWindow(hPrevInstanceWindow, SW_RESTORE);
    return (0);
  }

  ghInstance = hInstance;

  // Copy commandline!
  strncpy(gzCommandLine, pCommandLine, 100);
  gzCommandLine[99] = '\0';

  // Process the command line BEFORE initialization
  ProcessJa2CommandLineBeforeInitialization(pCommandLine);

  // Mem Usage
  giStartMem = MemGetFree() / 1024;

  // Handle Check for CD
  if (!HandleJA2CDCheck()) {
    return (0);
  }

  ShowCursor(FALSE);

  // Inititialize the SGP
  if (InitializeStandardGamingPlatform(hInstance, sCommandShow) ==
      FALSE) {  // We failed to initialize the SGP
    return 0;
  }

#ifdef ENGLISH
  SetIntroType(INTRO_SPLASH);
#endif

  gfApplicationActive = TRUE;
  gfProgramIsRunning = TRUE;

  FastDebugMsg("Running Game");

  // At this point the SGP is set up, which means all I/O, Memory, tools, etc... are available. All
  // we need to do is attend to the gaming mechanics themselves
  while (gfProgramIsRunning) {
    if (PeekMessage(&Message, NULL, 0, 0,
                    PM_NOREMOVE)) {  // We have a message on the WIN95 queue, let's get it
      if (!GetMessage(&Message, NULL, 0, 0)) {  // It's quitting time
        return Message.wParam;
      }
      // Ok, now that we have the message, let's handle it
      TranslateMessage(&Message);
      DispatchMessage(&Message);
    } else {  // Windows hasn't processed any messages, therefore we handle the rest
      if (gfApplicationActive ==
          FALSE) {  // Well we got nothing to do but to wait for a message to activate
        WaitMessage();
      } else {  // Well, the game is active, so we handle the game stuff
        GameLoop();

        // After this frame, reset input given flag
        gfSGPInputReceived = FALSE;
      }
    }
  }

  // This is the normal exit point
  FastDebugMsg("Exiting Game");
  PostQuitMessage(0);

  // SGPExit() will be called next through the atexit() mechanism...  This way we correctly process
  // both normal exits and emergency aborts (such as those caused by a failed assertion).

  // return wParam of the last message received
  return Message.wParam;
}

void SGPExit(void) {
  static BOOLEAN fAlreadyExiting = FALSE;
  BOOLEAN fUnloadScreens = TRUE;

  // helps prevent heap crashes when multiple assertions occur and call us
  if (fAlreadyExiting) {
    return;
  }

  fAlreadyExiting = TRUE;
  gfProgramIsRunning = FALSE;

// Wizardry only
#if !defined(JA2) && !defined(UTIL)
  if (gfGameInitialized) {
// ARM: if in DEBUG mode & we've ShutdownWithErrorBox, don't unload screens and release data structs
// to permit easier debugging
#ifdef _DEBUG
    if (gfIgnoreMessages) {
      fUnloadScreens = FALSE;
    }
#endif
    GameloopExit(fUnloadScreens);
  }
#endif

  ShutdownStandardGamingPlatform();
  ShowCursor(TRUE);
  if (strlen(gzErrorMsg)) {
    MessageBox(NULL, gzErrorMsg, "Error", MB_OK | MB_ICONERROR);
  }

#ifndef JA2
  VideoDumpMemoryLeaks();
#endif
}

void GetRuntimeSettings() {
  // Runtime settings - for now use INI file - later use registry
  STRING512 ExeDir;
  STRING512 INIFile;

  // Get Executable Directory
  GetExecutableDirectory(ExeDir);
  // Adjust Current Dir
  sprintf(INIFile, "%s\\sgp.ini", ExeDir);

  gbPixelDepth = GetPrivateProfileInt("SGP", "PIXEL_DEPTH", PIXEL_DEPTH, INIFile);
}

void ShutdownWithErrorBox(CHAR8 *pcMessage) {
  strncpy(gzErrorMsg, pcMessage, 255);
  gzErrorMsg[255] = '\0';
  gfIgnoreMessages = TRUE;

  exit(0);
}

#if !defined(JA2) && !defined(UTILS)

void ProcessCommandLine(CHAR8 *pCommandLine) {
  CHAR8 cSeparators[] = "\t =";
  CHAR8 *pCopy = NULL, *pToken;

  pCopy = (CHAR8 *)MemAlloc(strlen(pCommandLine) + 1);

  Assert(pCopy);
  if (!pCopy) return;

  memcpy(pCopy, pCommandLine, strlen(pCommandLine) + 1);

  pToken = strtok(pCopy, cSeparators);
  while (pToken) {
    if (!_strnicmp(pToken, "/NOSOUND", 8)) {
      SoundEnableSound(FALSE);
    } else if (!_strnicmp(pToken, "/INSPECTOR", 10)) {
      VideoInspectorEnable();
    } else if (!_strnicmp(pToken, "/VIDEOCFG", 9)) {
      pToken = strtok(NULL, cSeparators);
      VideoSetConfigFile(pToken);
    } else if (!_strnicmp(pToken, "/LOAD", 5)) {
      gfLoadAtStartup = TRUE;
    } else if (!_strnicmp(pToken, "/WINDOW", 7)) {
      VideoFullScreen(FALSE);
    } else if (!_strnicmp(pToken, "/BC", 7)) {
      gfUsingBoundsChecker = TRUE;
    } else if (!_strnicmp(pToken, "/CAPTURE", 7)) {
      gfCapturingVideo = TRUE;
    } else if (!_strnicmp(pToken, "/NOOCT", 6)) {
      NoOct();
    } else if (!_strnicmp(pToken, "/STRINGDATA", 11)) {
      pToken = strtok(NULL, cSeparators);
      gzStringDataOverride = (CHAR8 *)MemAlloc(strlen(pToken) + 1);
      strcpy(gzStringDataOverride, pToken);
    }

    pToken = strtok(NULL, cSeparators);
  }

  MemFree(pCopy);
}

BOOLEAN RunSetup(void) {
  if (!FileExists(VideoGetConfigFile()))
    _spawnl(_P_WAIT, "3DSetup.EXE", "3DSetup.EXE", VideoGetConfigFile(), NULL);

  return (FileExists(VideoGetConfigFile()));
}

#endif

void ProcessJa2CommandLineBeforeInitialization(CHAR8 *pCommandLine) {
  CHAR8 cSeparators[] = "\t =";
  CHAR8 *pCopy = NULL, *pToken;

  pCopy = (CHAR8 *)MemAlloc(strlen(pCommandLine) + 1);

  Assert(pCopy);
  if (!pCopy) return;

  memcpy(pCopy, pCommandLine, strlen(pCommandLine) + 1);

  pToken = strtok(pCopy, cSeparators);
  while (pToken) {
    // if its the NO SOUND option
    if (!_strnicmp(pToken, "/NOSOUND", 8)) {
      // disable the sound
      SoundEnableSound(FALSE);
    }

    // get the next token
    pToken = strtok(NULL, cSeparators);
  }

  MemFree(pCopy);
}
