// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

// Implementation of the platform layer on Windows.

#include "platform_win.h"

#include <windows.h>

#include "SGP/Debug.h"
#include "SGP/Timer.h"
#include "StrUtils.h"
#include "platform.h"

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void DebugPrint(const char *message) { OutputDebugStringA(message); }

/////////////////////////////////////////////////////////////////////////////////
// String
/////////////////////////////////////////////////////////////////////////////////

int strcasecmp(const char *s1, const char *s2) { return _stricmp(s1, s2); }

int strncasecmp(const char *s1, const char *s2, size_t n) { return _strnicmp(s1, s2, n); }

/////////////////////////////////////////////////////////////////////////////////
// Timers
/////////////////////////////////////////////////////////////////////////////////

extern uint32_t Plat_GetTickCount() { return GetTickCount(); }

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

// Main window frame for the application.
HWND ghWindow;

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

#define MAIN_TIMER_ID 1

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

uint32_t guiStartupTime;
uint32_t guiCurrentTime;

void CALLBACK Clock(HWND hWindow, UINT uMessage, UINT idEvent, DWORD dwTime) {
  guiCurrentTime = Plat_GetTickCount();
  if (guiCurrentTime < guiStartupTime) {
    // Adjust guiCurrentTime because of loopback on the timer value
    guiCurrentTime = guiCurrentTime + (0xffffffff - guiStartupTime);
  } else {
    // Adjust guiCurrentTime because of loopback on the timer value
    guiCurrentTime = guiCurrentTime - guiStartupTime;
  }
}

BOOLEAN InitializeClockManager(void) {
  // Register the start time (use WIN95 API call)
  guiCurrentTime = guiStartupTime = Plat_GetTickCount();
  SetTimer(ghWindow, MAIN_TIMER_ID, 10, (TIMERPROC)Clock);

  return TRUE;
}

void ShutdownClockManager(void) {
  // Make sure we kill the timer
  KillTimer(ghWindow, MAIN_TIMER_ID);
}

uint32_t GetClock(void) { return guiCurrentTime; }

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void Plat_OnSGPExit() { ShowCursor(TRUE); }

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

void Plat_ClipCursor(const struct Rect *rect) {
  if (rect) {
    RECT r = {rect->left, rect->right, rect->top, rect->bottom};
    ClipCursor(&r);
  } else {
    ClipCursor(NULL);
  }
}
/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
