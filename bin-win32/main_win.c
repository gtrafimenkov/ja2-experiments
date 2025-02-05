// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <zmouse.h>

#include "BuildDefines.h"
#include "CmdLine.h"
#include "DebugLog.h"
#include "GameLoop.h"
#include "GameRes.h"
#include "Globals.h"
#include "Intro.h"
#include "JA2Splash.h"
#include "Laptop/Laptop.h"
#include "Local.h"
#include "Res/Resource.h"
#include "SGP/ButtonSystem.h"
#include "SGP/FileMan.h"
#include "SGP/Font.h"
#include "SGP/Input.h"
#include "SGP/Random.h"
#include "SGP/SGP.h"
#include "SGP/SoundMan.h"
#include "SGP/Timer.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "StrUtils.h"
#include "Strategic/MapScreen.h"
#include "Strategic/MapScreenInterface.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "StringVector.h"
#include "Tactical/InterfacePanels.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "jplatform-windd2/jplatform_video_windd2.h"
#include "platform.h"
#include "platform_strings.h"
#include "platform_win.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

extern BOOLEAN gfPauseDueToPlayerGamePause;

extern void QueueEvent(uint16_t ubInputEvent, uint32_t usParam, uint32_t uiParam);

// Prototype Declarations

int32_t FAR PASCAL WindowProcedure(HWND hWindow, uint16_t Message, WPARAM wParam, LPARAM lParam);

int PASCAL HandledWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                          int sCommandShow);

HINSTANCE ghInstance;

uint32_t guiMouseWheelMsg;  // For mouse wheel messages

BOOLEAN gfApplicationActive;
BOOLEAN gfProgramIsRunning;
BOOLEAN gfGameInitialized = FALSE;

// There were TWO of them??!?! -- DB
// char		gzCommandLine[ 100 ];
char gzCommandLine[100];  // Command line given

BOOLEAN gfIgnoreMessages = FALSE;

int32_t FAR PASCAL WindowProcedure(HWND hWindow, uint16_t Message, WPARAM wParam, LPARAM lParam) {
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

    case WM_ACTIVATEAPP:
      switch (wParam) {
        case TRUE:  // We are restarting DirectDraw
          if (fRestore == TRUE) {
            RestoreVideoManager();
            if (!gfPauseDueToPlayerGamePause) {
              PauseTime(FALSE);
            }
            gfApplicationActive = TRUE;
          }
          break;
        case FALSE:
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
      ReapplyCursorClipRect();
      break;

    case WM_KILLFOCUS:
      // Set a flag to restore surfaces once a WM_ACTIVEATEAPP is received
      fRestore = TRUE;
      break;

    case WM_DEVICECHANGE: {
    } break;

    default:
      return DefWindowProc(hWindow, Message, wParam, lParam);
  }
  return 0L;
}

void ShutdownStandardGamingPlatform(void) {
  SoundServiceStreams();

  if (gfGameInitialized) {
    ShutdownGame();
  }

  ShutdownButtonSystem();
  MSYS_Shutdown();

  ShutdownSoundManager();

  ShutdownFontManager();

  ShutdownClockManager();  // must shutdown before VideoManager, 'cause it uses ghWindow

  ShutdownVideoSurfaceManager();
  ShutdownVideoObjectManager();
  ShutdownVideoManager();

  ShutdownInputManager();
  ShutdownContainers();
  FileMan_Shutdown();

  ShutdownMemoryManager();  // must go last (except for Debug), for MemDebugCounter to work right...

  //
  // Make sure we unregister the last remaining debug topic before shutting
  // down the debugging layer
  UnRegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

#ifdef SGP_DEBUG
  ShutdownDebugManager();
#endif
}

// Split command line arguments given as a single string into a vector of arguments.
// Splitting by spaces.  Quoted substrings will not be split and will be stored without
// quotes.
static void splitCommandLine(const char *commandline, struct StringVector *vec) {
  if (!vec || !commandline) return;

  const char *start = commandline;
  while (*start) {
    while (isspace((unsigned char)*start)) start++;  // Skip leading spaces
    if (*start == '\0') break;

    const char *end = start;
    char quote = '\0';
    if (*start == '"' || *start == '\'') {
      quote = *start;
      start++;
      end = start;
      while (*end && *end != quote) end++;  // Find closing quote
    } else {
      while (*end && !isspace((unsigned char)*end)) end++;  // Find end of word
    }

    size_t len = end - start;
    char *word = (char *)malloc(len + 1);
    if (!word) return;

    memcpy(word, start, len);
    word[len] = '\0';

    sv_add_string_copy(vec, word);
    free(word);

    start = end + (quote ? 1 : 0);
  }
}

#define PROGRAM_NAME "ja2v"

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCommandLine,
                   int sCommandShow) {
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
  strcopy(gzCommandLine, ARR_SIZE(gzCommandLine), pCommandLine);

  // Splitting command line into list of arguments.
  // CommandLineToArgvW can split the command line into a list of arguments,
  // but there is no A variant of this function and I don't want to use wchar_t.
  CmdLineArgs cmdLineArgs;
  {
    struct StringVector *args = sv_new();
    sv_add_string_copy(args, PROGRAM_NAME);
    splitCommandLine(pCommandLine, args);
    if (!CmdLineParse(args->size, args->data, &cmdLineArgs)) {
      DebugLogF("Failed to parse command line: %s", cmdLineArgs.errorMessage);
      CmdLineFree(&cmdLineArgs);
      return 1;
    } else {
      DebugLog("Command line arguments:");
      DebugLogF("  help    = %d", cmdLineArgs.help);
      DebugLogF("  nosound = %d", cmdLineArgs.nosound);
      DebugLogF("  datadir = %s", cmdLineArgs.datadir);

      if (cmdLineArgs.help) {
        CmdLinePrintHelp(PROGRAM_NAME);
        return 0;
      }
    }
  }

  if (cmdLineArgs.nosound) {
    SoundEnableSound(FALSE);
  }

  ShowCursor(FALSE);

  // Inititialize the SGP
  struct JVideoInitParams videoInitParams = {hInstance, (uint16_t)sCommandShow,
                                             (void *)WindowProcedure, IDI_ICON1};
  if (InitializeStandardGamingPlatform(&videoInitParams, cmdLineArgs.datadir) == FALSE) {
    return 0;
  }

  // Register mouse wheel message
  guiMouseWheelMsg = RegisterWindowMessage(MSH_MOUSEWHEEL);

  gfGameInitialized = TRUE;

  if (UsingEnglishResources()) {
    SetIntroType(INTRO_SPLASH);
  }

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
