// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include <process.h>
#include <stdio.h>

#include "FadeScreen.h"
#include "Globals.h"
#include "Local.h"
#include "Rect.h"
#include "SGP/Debug.h"
#include "SGP/Input.h"
#include "SGP/VObject.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/VideoInternal.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/TimerControl.h"
#include "platform.h"

#define INITGUID
#include <ddraw.h>
#include <windows.h>

#include "Smack.h"
#include "platform_win.h"

#ifndef _MT
#define _MT
#endif

#define BUFFER_READY 0x00
#define BUFFER_BUSY 0x01
#define BUFFER_DIRTY 0x02
#define BUFFER_DISABLED 0x03

#define MAX_CURSOR_WIDTH 64
#define VIDEO_NO_CURSOR 0xFFFF

extern int32_t giNumFrames;

static struct VSurface *CreateVideoSurfaceFromDDSurface(LPDIRECTDRAWSURFACE2 lpDDSurface);

extern BOOLEAN GetRGBDistribution(void);

// Surface Functions

static void DDCreateSurface(LPDIRECTDRAW2 pExistingDirectDraw, DDSURFACEDESC *pNewSurfaceDesc,
                            LPDIRECTDRAWSURFACE *ppNewSurface1,
                            LPDIRECTDRAWSURFACE2 *ppNewSurface2);
static void DDGetSurfaceDescription(LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc);
static void DDReleaseSurface(LPDIRECTDRAWSURFACE *ppOldSurface1,
                             LPDIRECTDRAWSURFACE2 *ppOldSurface2);
static void DDLockSurface(LPDIRECTDRAWSURFACE2 pSurface, LPRECT pDestRect,
                          LPDDSURFACEDESC pSurfaceDesc, uint32_t uiFlags, HANDLE hEvent);
static void DDBltFastSurface(LPDIRECTDRAWSURFACE2 pDestSurface, uint32_t uiX, uint32_t uiY,
                             LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, uint32_t uiTrans);
static void DDBltSurface(LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect,
                         LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, uint32_t uiFlags,
                         LPDDBLTFX pDDBltFx);
static void DDSetSurfaceColorKey(LPDIRECTDRAWSURFACE2 pSurface, uint32_t uiFlags,
                                 LPDDCOLORKEY pDDColorKey);

// Palette Functions
static void DDCreatePalette(LPDIRECTDRAW2 pDirectDraw, uint32_t uiFlags, LPPALETTEENTRY pColorTable,
                            LPDIRECTDRAWPALETTE FAR *ppDDPalette, IUnknown FAR *pUnkOuter);
static void DDSetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags,
                                uint32_t uiStartingEntry, uint32_t uiCount,
                                LPPALETTEENTRY pEntries);
static void DDReleasePalette(LPDIRECTDRAWPALETTE pPalette);
static void DDGetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags, uint32_t uiBase,
                                uint32_t uiNumEntries, LPPALETTEENTRY pEntries);

// local functions
static char *DirectXErrorDescription(int32_t iDXReturn);
static void DirectXAttempt(int32_t iErrorCode, int32_t nLine, char *szFilename);

#undef ATTEMPT
#define ATTEMPT(x) DirectXAttempt((x), __LINE__, __FILE__)

#undef DEBUGMSG
#define DEBUGMSG(x) DebugPrint(x)

#define MAX_DIRTY_REGIONS 128

#define VIDEO_OFF 0x00
#define VIDEO_ON 0x01
#define VIDEO_SHUTTING_DOWN 0x02
#define VIDEO_SUSPENDED 0x04

#define THREAD_OFF 0x00
#define THREAD_ON 0x01
#define THREAD_SUSPENDED 0x02

#define CURRENT_MOUSE_DATA 0
#define PREVIOUS_MOUSE_DATA 1

typedef struct {
  BOOLEAN fRestore;
  uint16_t usMouseXPos, usMouseYPos;
  uint16_t usLeft, usTop, usRight, usBottom;
  RECT Region;
  LPDIRECTDRAWSURFACE _pSurface;
  LPDIRECTDRAWSURFACE2 pSurface;

} MouseCursorBackground;

//
// Video state variables
//

#define MAX_NUM_FRAMES 25

static BOOLEAN gfVideoCapture = FALSE;
static uint32_t guiFramePeriod = (1000 / 15);
static uint32_t guiLastFrame;
static uint16_t *gpFrameData[MAX_NUM_FRAMES];
int32_t giNumFrames = 0;

//
// Direct Draw objects for both the Primary and Backbuffer surfaces
//

static LPDIRECTDRAW _gpDirectDrawObject = NULL;
static LPDIRECTDRAW2 gpDirectDrawObject = NULL;

static LPDIRECTDRAWSURFACE _gpPrimarySurface = NULL;
static LPDIRECTDRAWSURFACE2 gpPrimarySurface = NULL;
static LPDIRECTDRAWSURFACE2 gpBackBuffer = NULL;

//
// Direct Draw Objects for the frame buffer
//

static LPDIRECTDRAWSURFACE _gpFrameBuffer = NULL;
static LPDIRECTDRAWSURFACE2 gpFrameBuffer = NULL;

//
// Globals for mouse cursor
//

static uint16_t gusMouseCursorWidth;
static uint16_t gusMouseCursorHeight;
static int16_t gsMouseCursorXOffset;
static int16_t gsMouseCursorYOffset;

static LPDIRECTDRAWSURFACE _gpMouseCursor = NULL;
static LPDIRECTDRAWSURFACE2 gpMouseCursor = NULL;

static LPDIRECTDRAWSURFACE _gpMouseCursorOriginal = NULL;
static LPDIRECTDRAWSURFACE2 gpMouseCursorOriginal = NULL;

static MouseCursorBackground gMouseCursorBackground[2];

static struct VObject *gpCursorStore;

static char gFatalErrorString[512];

//
// Refresh thread based variables
//

static uint32_t guiFrameBufferState;    // BUFFER_READY, BUFFER_DIRTY
static uint32_t guiMouseBufferState;    // BUFFER_READY, BUFFER_DIRTY, BUFFER_DISABLED
static uint32_t guiVideoManagerState;   // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN
static uint32_t guiRefreshThreadState;  // THREAD_ON, THREAD_OFF, THREAD_SUSPENDED

//
// Dirty rectangle management variables
//

static SGPRect gListOfDirtyRegions[MAX_DIRTY_REGIONS];
static uint32_t guiDirtyRegionCount;
static BOOLEAN gfForceFullScreenRefresh;

static SGPRect gDirtyRegionsEx[MAX_DIRTY_REGIONS];
static uint32_t gDirtyRegionsFlagsEx[MAX_DIRTY_REGIONS];
static uint32_t guiDirtyRegionExCount;

//
// Screen output stuff
//

static BOOLEAN gfPrintFrameBuffer;
static uint32_t guiPrintFrameBufferIndex;

static uint16_t gusRedMask;
static uint16_t gusGreenMask;
static uint16_t gusBlueMask;
static int16_t gusRedShift;
static int16_t gusBlueShift;
static int16_t gusGreenShift;

static void AddRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags);
static void SnapshotSmall(void);
static void VideoMovieCapture(BOOLEAN fEnable);
static void RefreshMovieCache();

static void *LockPrimarySurface(LPDIRECTDRAWSURFACE2 _surf, uint32_t *uiPitch);

BOOLEAN InitializeVideoManager(struct PlatformInitParams *params) {
  uint32_t uiIndex, uiPitch;
  HRESULT ReturnCode;
  HWND hWindow;
  WNDCLASS WindowClass;
  char ClassName[] = APPLICATION_NAME;
  DDSURFACEDESC SurfaceDescription;
  DDCOLORKEY ColorKey;
  void *pTmpPointer;

  DDSCAPS SurfaceCaps;

  //
  // Register debug topics
  //

  RegisterDebugTopic(TOPIC_VIDEO, "Video");
  DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Initializing the video manager");

  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = (WNDPROC)params->WindowProc;
  WindowClass.cbClsExtra = 0;
  WindowClass.cbWndExtra = 0;
  WindowClass.hInstance = params->hInstance;
  WindowClass.hIcon = LoadIcon(params->hInstance, MAKEINTRESOURCE(params->iconID));
  WindowClass.hCursor = NULL;
  WindowClass.hbrBackground = NULL;
  WindowClass.lpszMenuName = NULL;
  WindowClass.lpszClassName = ClassName;
  RegisterClass(&WindowClass);

  //
  // Get a window handle for our application (gotta have on of those)
  // Don't change this
  //
  hWindow = CreateWindowEx(WS_EX_TOPMOST, ClassName, ClassName, WS_POPUP | WS_VISIBLE, 0, 0,
                           GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL,
                           params->hInstance, NULL);
  if (hWindow == NULL) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Failed to create window frame for Direct Draw");
    return FALSE;
  }

  //
  // Excellent. Now we record the hWindow variable for posterity (not)
  //

  memset(gpFrameData, 0, sizeof(gpFrameData));

  ghWindow = hWindow;

  //
  // Display our full screen window
  //

  ShowCursor(FALSE);
  ShowWindow(hWindow, params->usCommandShow);
  UpdateWindow(hWindow);
  SetFocus(hWindow);

  ReturnCode = DirectDrawCreate(NULL, &_gpDirectDrawObject, NULL);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  IID tmpID = IID_IDirectDraw2;
  ReturnCode =
      IDirectDraw_QueryInterface(_gpDirectDrawObject, &tmpID, (LPVOID *)&gpDirectDrawObject);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Set the exclusive mode
  //
  ReturnCode = IDirectDraw2_SetCooperativeLevel(gpDirectDrawObject, ghWindow,
                                                DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Set the display mode
  //
  ReturnCode =
      IDirectDraw2_SetDisplayMode(gpDirectDrawObject, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0, 0);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  gusScreenWidth = SCREEN_WIDTH;
  gusScreenHeight = SCREEN_HEIGHT;

  //
  // Initialize Primary Surface along with BackBuffer
  //

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
  SurfaceDescription.dwBackBufferCount = 1;

  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpPrimarySurface, NULL);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode =
      IDirectDrawSurface_QueryInterface(_gpPrimarySurface, &tmpID, (LPVOID *)&gpPrimarySurface);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  SurfaceCaps.dwCaps = DDSCAPS_BACKBUFFER;
  ReturnCode =
      IDirectDrawSurface2_GetAttachedSurface(gpPrimarySurface, &SurfaceCaps, &gpBackBuffer);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Initialize the frame buffer
  //

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = SCREEN_WIDTH;
  SurfaceDescription.dwHeight = SCREEN_HEIGHT;
  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpFrameBuffer, NULL);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpFrameBuffer, &tmpID, (LPVOID *)&gpFrameBuffer);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Blank out the frame buffer
  //

  pTmpPointer = LockVSurfaceByID(vsIndexFB, &uiPitch);
  memset(pTmpPointer, 0, 480 * uiPitch);
  UnlockVSurface(vsFB);

  //
  // Initialize the main mouse surfaces
  //

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  // SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
  SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
  ReturnCode =
      IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_gpMouseCursor, NULL);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0,
             String("Failed to create MouseCursor witd %ld", ReturnCode & 0x0f));
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpMouseCursor, &tmpID, (LPVOID *)&gpMouseCursor);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  ColorKey.dwColorSpaceLowValue = 0;
  ColorKey.dwColorSpaceHighValue = 0;
  ReturnCode = IDirectDrawSurface2_SetColorKey(gpMouseCursor, DDCKEY_SRCBLT, &ColorKey);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Initialize the main mouse original surface
  //

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
  SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
  ReturnCode = IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription,
                                          &_gpMouseCursorOriginal, NULL);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Failed to create MouseCursorOriginal");
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  tmpID = IID_IDirectDrawSurface2;
  ReturnCode = IDirectDrawSurface_QueryInterface(_gpMouseCursorOriginal, &tmpID,
                                                 (LPVOID *)&gpMouseCursorOriginal);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Initialize the main mouse background surfaces. There are two of them (one for each of the
  // Primary and Backbuffer surfaces
  //

  for (uiIndex = 0; uiIndex < 1; uiIndex++) {
    //
    // Initialize various mouse background variables
    //

    gMouseCursorBackground[uiIndex].fRestore = FALSE;

    //
    // Initialize the direct draw surfaces for the mouse background
    //

    memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
    SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SurfaceDescription.dwWidth = MAX_CURSOR_WIDTH;
    SurfaceDescription.dwHeight = MAX_CURSOR_HEIGHT;
    ReturnCode = IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription,
                                            &(gMouseCursorBackground[uiIndex]._pSurface), NULL);
    if (ReturnCode != DD_OK) {
      DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Failed to create MouseCursorBackground");
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return FALSE;
    }

    IID tmpID = IID_IDirectDrawSurface2;
    ReturnCode = IDirectDrawSurface_QueryInterface(
        (IDirectDrawSurface *)gMouseCursorBackground[uiIndex]._pSurface, &tmpID,
        (LPVOID *)&(gMouseCursorBackground[uiIndex].pSurface));
    if (ReturnCode != DD_OK) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return FALSE;
    }
  }

  //
  // Initialize state variables
  //

  guiFrameBufferState = BUFFER_DIRTY;
  guiMouseBufferState = BUFFER_DISABLED;
  guiVideoManagerState = VIDEO_ON;
  guiRefreshThreadState = THREAD_OFF;
  guiDirtyRegionCount = 0;
  gfForceFullScreenRefresh = TRUE;
  gpCursorStore = NULL;
  gfPrintFrameBuffer = FALSE;
  guiPrintFrameBufferIndex = 0;

  //
  // This function must be called to setup RGB information
  //

  GetRGBDistribution();

  return TRUE;
}

void ShutdownVideoManager(void) {
  // uint32_t  uiRefreshThreadState;

  DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Shutting down the video manager");

  //
  // Toggle the state of the video manager to indicate to the refresh thread that it needs to shut
  // itself down
  //

  IDirectDrawSurface2_Release(gpMouseCursorOriginal);
  IDirectDrawSurface2_Release(gpMouseCursor);
  IDirectDrawSurface2_Release(gMouseCursorBackground[0].pSurface);
  IDirectDrawSurface2_Release(gpBackBuffer);
  IDirectDrawSurface2_Release(gpPrimarySurface);

  IDirectDraw2_RestoreDisplayMode(gpDirectDrawObject);
  IDirectDraw2_SetCooperativeLevel(gpDirectDrawObject, ghWindow, DDSCL_NORMAL);
  IDirectDraw2_Release(gpDirectDrawObject);

  // destroy the window
  // DestroyWindow( ghWindow );

  guiVideoManagerState = VIDEO_OFF;

  if (gpCursorStore != NULL) {
    DeleteVideoObject(gpCursorStore);
    gpCursorStore = NULL;
  }

  // ATE: Release mouse cursor!
  FreeMouseCursor();

  UnRegisterDebugTopic(TOPIC_VIDEO, "Video");
}

void SuspendVideoManager(void) { guiVideoManagerState = VIDEO_SUSPENDED; }

BOOLEAN RestoreVideoManager(void) {
  HRESULT ReturnCode;

  //
  // Make sure the video manager is indeed suspended before moving on
  //

  if (guiVideoManagerState == VIDEO_SUSPENDED) {
    //
    // Restore the Primary and Backbuffer
    //

    ReturnCode = IDirectDrawSurface2_Restore(gpPrimarySurface);
    if (ReturnCode != DD_OK) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return FALSE;
    }

    ReturnCode = IDirectDrawSurface2_Restore(gpBackBuffer);
    if (ReturnCode != DD_OK) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return FALSE;
    }

    //
    // Restore the mouse surfaces and make sure to initialize the gpMouseCursor surface
    //

    ReturnCode = IDirectDrawSurface2_Restore(gMouseCursorBackground[0].pSurface);
    if (ReturnCode != DD_OK) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return FALSE;
    }

    ReturnCode = IDirectDrawSurface2_Restore(gpMouseCursor);
    if (ReturnCode != DD_OK) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return FALSE;
    } else {
      guiMouseBufferState = BUFFER_DIRTY;
    }

    //
    // Set the video state to VIDEO_ON
    //

    guiFrameBufferState = BUFFER_DIRTY;
    guiMouseBufferState = BUFFER_DIRTY;
    gfForceFullScreenRefresh = TRUE;
    guiVideoManagerState = VIDEO_ON;
    return TRUE;
  } else {
    return FALSE;
  }
}

void InvalidateRegion(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom) {
  if (gfForceFullScreenRefresh == TRUE) {
    //
    // There's no point in going on since we are forcing a full screen refresh
    //

    return;
  }

  if (guiDirtyRegionCount < MAX_DIRTY_REGIONS) {
    //
    // Well we haven't broken the MAX_DIRTY_REGIONS limit yet, so we register the new region
    //

    // DO SOME PREMIMARY CHECKS FOR VALID RECTS
    if (iLeft < 0) iLeft = 0;

    if (iTop < 0) iTop = 0;

    if (iRight > SCREEN_WIDTH) iRight = SCREEN_WIDTH;

    if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

    if ((iRight - iLeft) <= 0) return;

    if ((iBottom - iTop) <= 0) return;

    gListOfDirtyRegions[guiDirtyRegionCount].iLeft = iLeft;
    gListOfDirtyRegions[guiDirtyRegionCount].iTop = iTop;
    gListOfDirtyRegions[guiDirtyRegionCount].iRight = iRight;
    gListOfDirtyRegions[guiDirtyRegionCount].iBottom = iBottom;

    //		gDirtyRegionFlags[ guiDirtyRegionCount ] = TRUE;

    guiDirtyRegionCount++;

  } else {
    //
    // The MAX_DIRTY_REGIONS limit has been exceeded. Therefore we arbitrarely invalidate the entire
    // screen and force a full screen refresh
    //
    guiDirtyRegionExCount = 0;
    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = TRUE;
  }
}

void InvalidateRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags) {
  int32_t iOldBottom;

  iOldBottom = iBottom;

  // Check if we are spanning the rectangle - if so slit it up!
  if (iTop <= gsVIEWPORT_WINDOW_END_Y && iBottom > gsVIEWPORT_WINDOW_END_Y) {
    // Add new top region
    iBottom = gsVIEWPORT_WINDOW_END_Y;
    AddRegionEx(iLeft, iTop, iRight, iBottom, uiFlags);

    // Add new bottom region
    iTop = gsVIEWPORT_WINDOW_END_Y;
    iBottom = iOldBottom;
    AddRegionEx(iLeft, iTop, iRight, iBottom, uiFlags);

  } else {
    AddRegionEx(iLeft, iTop, iRight, iBottom, uiFlags);
  }
}

static void AddRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags) {
  if (guiDirtyRegionExCount < MAX_DIRTY_REGIONS) {
    // DO SOME PREMIMARY CHECKS FOR VALID RECTS
    if (iLeft < 0) iLeft = 0;

    if (iTop < 0) iTop = 0;

    if (iRight > SCREEN_WIDTH) iRight = SCREEN_WIDTH;

    if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

    if ((iRight - iLeft) <= 0) return;

    if ((iBottom - iTop) <= 0) return;

    gDirtyRegionsEx[guiDirtyRegionExCount].iLeft = iLeft;
    gDirtyRegionsEx[guiDirtyRegionExCount].iTop = iTop;
    gDirtyRegionsEx[guiDirtyRegionExCount].iRight = iRight;
    gDirtyRegionsEx[guiDirtyRegionExCount].iBottom = iBottom;

    gDirtyRegionsFlagsEx[guiDirtyRegionExCount] = uiFlags;

    guiDirtyRegionExCount++;

  } else {
    guiDirtyRegionExCount = 0;
    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = TRUE;
  }
}

void InvalidateScreen(void) {
  //
  // W A R N I N G ---- W A R N I N G ---- W A R N I N G ---- W A R N I N G ---- W A R N I N G ----
  //
  // This function is intended to be called by a thread which has already locked the
  // FRAME_BUFFER_MUTEX mutual exclusion section. Anything else will cause the application to
  // yack
  //

  guiDirtyRegionCount = 0;
  guiDirtyRegionExCount = 0;
  gfForceFullScreenRefresh = TRUE;
  guiFrameBufferState = BUFFER_DIRTY;
}

void ScrollJA2Background(uint32_t uiDirection, int16_t sScrollXIncrement, int16_t sScrollYIncrement,
                         LPDIRECTDRAWSURFACE2 pSource, LPDIRECTDRAWSURFACE2 pDest,
                         BOOLEAN fRenderStrip, uint32_t uiCurrentMouseBackbuffer) {
  uint16_t usWidth, usHeight;
  HRESULT ReturnCode;
  static RECT Region;
  static uint16_t usMouseXPos, usMouseYPos;
  static RECT StripRegions[2], MouseRegion;
  uint16_t usNumStrips = 0;
  int32_t cnt;
  int16_t sShiftX, sShiftY;
  int32_t uiCountY;

  GetCurrentVideoSettings(&usWidth, &usHeight);
  usHeight = (gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);

  StripRegions[0].left = gsVIEWPORT_START_X;
  StripRegions[0].right = gsVIEWPORT_END_X;
  StripRegions[0].top = gsVIEWPORT_WINDOW_START_Y;
  StripRegions[0].bottom = gsVIEWPORT_WINDOW_END_Y;
  StripRegions[1].left = gsVIEWPORT_START_X;
  StripRegions[1].right = gsVIEWPORT_END_X;
  StripRegions[1].top = gsVIEWPORT_WINDOW_START_Y;
  StripRegions[1].bottom = gsVIEWPORT_WINDOW_END_Y;

  MouseRegion.left = gMouseCursorBackground[uiCurrentMouseBackbuffer].usLeft;
  MouseRegion.top = gMouseCursorBackground[uiCurrentMouseBackbuffer].usTop;
  MouseRegion.right = gMouseCursorBackground[uiCurrentMouseBackbuffer].usRight;
  MouseRegion.bottom = gMouseCursorBackground[uiCurrentMouseBackbuffer].usBottom;

  usMouseXPos = gMouseCursorBackground[uiCurrentMouseBackbuffer].usMouseXPos;
  usMouseYPos = gMouseCursorBackground[uiCurrentMouseBackbuffer].usMouseYPos;

  switch (uiDirection) {
    case SCROLL_LEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth - (sScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;

      do {
        ReturnCode =
            IDirectDrawSurface2_BltFast(pDest, sScrollXIncrement, gsVIEWPORT_WINDOW_START_Y,
                                        pSource, (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
      usMouseXPos += sScrollXIncrement;

      usNumStrips = 1;
      break;

    case SCROLL_RIGHT:

      Region.left = sScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;

      do {
        ReturnCode = IDirectDrawSurface2_BltFast(pDest, 0, gsVIEWPORT_WINDOW_START_Y, pSource,
                                                 (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset(
            (uint8_t *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      usMouseXPos -= sScrollXIncrement;

      usNumStrips = 1;
      break;

    case SCROLL_UP:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight - sScrollYIncrement;

      do {
        ReturnCode =
            IDirectDrawSurface2_BltFast(pDest, 0, gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement,
                                        pSource, (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      for (uiCountY = sScrollYIncrement - 1 + gsVIEWPORT_WINDOW_START_Y;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      usNumStrips = 1;

      usMouseYPos += sScrollYIncrement;

      break;

    case SCROLL_DOWN:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;

      do {
        ReturnCode = IDirectDrawSurface2_BltFast(pDest, 0, gsVIEWPORT_WINDOW_START_Y, pSource,
                                                 (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // Zero out z
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      usNumStrips = 1;

      usMouseYPos -= sScrollYIncrement;

      break;

    case SCROLL_UPLEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth - (sScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight - sScrollYIncrement;

      do {
        ReturnCode = IDirectDrawSurface2_BltFast(pDest, sScrollXIncrement,
                                                 gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement,
                                                 pSource, (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
      StripRegions[1].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      StripRegions[1].left = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos += sScrollYIncrement;
      usMouseXPos += sScrollXIncrement;

      break;

    case SCROLL_UPRIGHT:

      Region.left = sScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight - sScrollYIncrement;

      do {
        ReturnCode =
            IDirectDrawSurface2_BltFast(pDest, 0, gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement,
                                        pSource, (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset(
            (uint8_t *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      StripRegions[1].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement);
      StripRegions[1].right = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos += sScrollYIncrement;
      usMouseXPos -= sScrollXIncrement;

      break;

    case SCROLL_DOWNLEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
      Region.right = usWidth - (sScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;

      do {
        ReturnCode =
            IDirectDrawSurface2_BltFast(pDest, sScrollXIncrement, gsVIEWPORT_WINDOW_START_Y,
                                        pSource, (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, sScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);

      StripRegions[1].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      StripRegions[1].left = (int16_t)(gsVIEWPORT_START_X + sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos -= sScrollYIncrement;
      usMouseXPos += sScrollXIncrement;

      break;

    case SCROLL_DOWNRIGHT:

      Region.left = sScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
      Region.right = usWidth;
      Region.bottom = gsVIEWPORT_WINDOW_START_Y + usHeight;

      do {
        ReturnCode = IDirectDrawSurface2_BltFast(pDest, 0, gsVIEWPORT_WINDOW_START_Y, pSource,
                                                 (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);

          if (ReturnCode == DDERR_SURFACELOST) {
            break;
          }
        }
      } while (ReturnCode != DD_OK);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset(
            (uint8_t *)gpZBuffer + (uiCountY * 1280) + ((gsVIEWPORT_END_X - sScrollXIncrement) * 2),
            0, sScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      StripRegions[1].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement);
      StripRegions[1].right = (int16_t)(gsVIEWPORT_END_X - sScrollXIncrement);
      usNumStrips = 2;

      usMouseYPos -= sScrollYIncrement;
      usMouseXPos -= sScrollXIncrement;

      break;
  }

  if (fRenderStrip) {
    for (cnt = 0; cnt < usNumStrips; cnt++) {
      RenderStaticWorldRect((int16_t)StripRegions[cnt].left, (int16_t)StripRegions[cnt].top,
                            (int16_t)StripRegions[cnt].right, (int16_t)StripRegions[cnt].bottom,
                            TRUE);
      // Optimize Redundent tiles too!
      // ExamineZBufferRect( (int16_t)StripRegions[ cnt ].left, (int16_t)StripRegions[ cnt ].top,
      // (int16_t)StripRegions[ cnt ].right, (int16_t)StripRegions[ cnt ].bottom );

      do {
        ReturnCode = IDirectDrawSurface2_BltFast(
            pDest, StripRegions[cnt].left, StripRegions[cnt].top, gpFrameBuffer,
            (LPRECT) & (StripRegions[cnt]), DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);
        }

        if (ReturnCode == DDERR_SURFACELOST) {
          break;
        }
      } while (ReturnCode != DD_OK);
    }

    sShiftX = 0;
    sShiftY = 0;

    switch (uiDirection) {
      case SCROLL_LEFT:

        sShiftX = sScrollXIncrement;
        sShiftY = 0;
        break;

      case SCROLL_RIGHT:

        sShiftX = -sScrollXIncrement;
        sShiftY = 0;
        break;

      case SCROLL_UP:

        sShiftX = 0;
        sShiftY = sScrollYIncrement;
        break;

      case SCROLL_DOWN:

        sShiftX = 0;
        sShiftY = -sScrollYIncrement;
        break;

      case SCROLL_UPLEFT:

        sShiftX = sScrollXIncrement;
        sShiftY = sScrollYIncrement;
        break;

      case SCROLL_UPRIGHT:

        sShiftX = -sScrollXIncrement;
        sShiftY = sScrollYIncrement;
        break;

      case SCROLL_DOWNLEFT:

        sShiftX = sScrollXIncrement;
        sShiftY = -sScrollYIncrement;
        break;

      case SCROLL_DOWNRIGHT:

        sShiftX = -sScrollXIncrement;
        sShiftY = -sScrollYIncrement;
        break;
    }

    // RESTORE SHIFTED
    RestoreShiftedVideoOverlays(sShiftX, sShiftY);

    // SAVE NEW
    SaveVideoOverlaysArea(BACKBUFFER);

    // BLIT NEW
    ExecuteVideoOverlaysToAlternateBuffer(BACKBUFFER);
  }
}

void RefreshScreen(void *DummyVariable) {
  static uint32_t uiRefreshThreadState, uiIndex;
  uint16_t usScreenWidth, usScreenHeight;
  static BOOLEAN fShowMouse;
  HRESULT ReturnCode;
  static RECT Region;
  static BOOLEAN fFirstTime = TRUE;
  uint32_t uiTime;

  usScreenWidth = usScreenHeight = 0;

  if (fFirstTime) {
    fShowMouse = FALSE;
  }

  // DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Looping in refresh");

  switch (guiVideoManagerState) {
    case VIDEO_ON:  //
      // Excellent, everything is cosher, we continue on
      //
      uiRefreshThreadState = guiRefreshThreadState = THREAD_ON;
      usScreenWidth = gusScreenWidth;
      usScreenHeight = gusScreenHeight;
      break;
    case VIDEO_OFF:  //
      // Hot damn, the video manager is suddenly off. We have to bugger out of here. Don't forget to
      // leave the critical section
      //
      guiRefreshThreadState = THREAD_OFF;
      return;
    case VIDEO_SUSPENDED:  //
      // This are suspended. Make sure the refresh function does try to access any of the direct
      // draw surfaces
      //
      uiRefreshThreadState = guiRefreshThreadState = THREAD_SUSPENDED;
      break;
    case VIDEO_SHUTTING_DOWN:  //
                               // Well things are shutting down. So we need to bugger out of there.
                               // Don't forget to leave the critical section before returning
                               //
      guiRefreshThreadState = THREAD_OFF;
      return;
  }

  //
  // Get the current mouse position
  //

  struct Point MousePos = GetMousePoint();

  // RESTORE OLD POSITION OF MOUSE
  if (gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore == TRUE) {
    Region.left = gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft;
    Region.top = gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop;
    Region.right = gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight;
    Region.bottom = gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(
          gpBackBuffer, gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
          gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos,
          gMouseCursorBackground[CURRENT_MOUSE_DATA].pSurface, (LPRECT)&Region,
          DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);

        if (ReturnCode == DDERR_SURFACELOST) {
          goto ENDOFLOOP;
        }
      }
    } while (ReturnCode != DD_OK);

    // Save position into other background region
    memcpy(&(gMouseCursorBackground[PREVIOUS_MOUSE_DATA]),
           &(gMouseCursorBackground[CURRENT_MOUSE_DATA]), sizeof(MouseCursorBackground));
  }

  //
  // Ok we were able to get a hold of the frame buffer stuff. Check to see if it needs updating
  // if not, release the frame buffer stuff right away
  //
  if (guiFrameBufferState == BUFFER_DIRTY) {
    // Well the frame buffer is dirty.
    //

    if (gfFadeInitialized && gfFadeInVideo) {
      gFadeFunction();
    } else {
      //
      // Either Method (1) or (2)
      //
      if (gfForceFullScreenRefresh == TRUE) {
        //
        // Method (1) - We will be refreshing the entire screen
        //

        Region.left = 0;
        Region.top = 0;
        Region.right = usScreenWidth;
        Region.bottom = usScreenHeight;

        do {
          ReturnCode = IDirectDrawSurface2_BltFast(gpBackBuffer, 0, 0, gpFrameBuffer,
                                                   (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
          if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
            DirectXAttempt(ReturnCode, __LINE__, __FILE__);

            if (ReturnCode == DDERR_SURFACELOST) {
              goto ENDOFLOOP;
            }
          }
        } while (ReturnCode != DD_OK);

      } else {
        for (uiIndex = 0; uiIndex < guiDirtyRegionCount; uiIndex++) {
          Region.left = gListOfDirtyRegions[uiIndex].iLeft;
          Region.top = gListOfDirtyRegions[uiIndex].iTop;
          Region.right = gListOfDirtyRegions[uiIndex].iRight;
          Region.bottom = gListOfDirtyRegions[uiIndex].iBottom;

          do {
            ReturnCode =
                IDirectDrawSurface2_BltFast(gpBackBuffer, Region.left, Region.top, gpFrameBuffer,
                                            (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
            if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
              DirectXAttempt(ReturnCode, __LINE__, __FILE__);
            }

            if (ReturnCode == DDERR_SURFACELOST) {
              goto ENDOFLOOP;
            }
          } while (ReturnCode != DD_OK);
        }

        // Now do new, extended dirty regions
        for (uiIndex = 0; uiIndex < guiDirtyRegionExCount; uiIndex++) {
          Region.left = gDirtyRegionsEx[uiIndex].iLeft;
          Region.top = gDirtyRegionsEx[uiIndex].iTop;
          Region.right = gDirtyRegionsEx[uiIndex].iRight;
          Region.bottom = gDirtyRegionsEx[uiIndex].iBottom;

          // Do some checks if we are in the process of scrolling!
          if (gfRenderScroll) {
            // Check if we are completely out of bounds
            if (Region.top <= gsVIEWPORT_WINDOW_END_Y && Region.bottom <= gsVIEWPORT_WINDOW_END_Y) {
              continue;
            }
          }

          do {
            ReturnCode =
                IDirectDrawSurface2_BltFast(gpBackBuffer, Region.left, Region.top, gpFrameBuffer,
                                            (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
            if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
              DirectXAttempt(ReturnCode, __LINE__, __FILE__);
            }

            if (ReturnCode == DDERR_SURFACELOST) {
              goto ENDOFLOOP;
            }
          } while (ReturnCode != DD_OK);
        }
      }
    }
    if (gfRenderScroll) {
      ScrollJA2Background(guiScrollDirection, gsScrollXIncrement, gsScrollYIncrement,
                          gpPrimarySurface, gpBackBuffer, TRUE, PREVIOUS_MOUSE_DATA);
    }
    gfIgnoreScrollDueToCenterAdjust = FALSE;

    //
    // Update the guiFrameBufferState variable to reflect that the frame buffer can now be handled
    //

    guiFrameBufferState = BUFFER_READY;
  }

  //
  // Do we want to print the frame stuff ??
  //

  if (gfVideoCapture) {
    uiTime = Plat_GetTickCount();
    if ((uiTime < guiLastFrame) || (uiTime > (guiLastFrame + guiFramePeriod))) {
      SnapshotSmall();
      guiLastFrame = uiTime;
    }
  }

  if (gfPrintFrameBuffer == TRUE) {
    LPDIRECTDRAWSURFACE _pTmpBuffer;
    LPDIRECTDRAWSURFACE2 pTmpBuffer;
    DDSURFACEDESC SurfaceDescription;
    FILE *OutputFile;
    char FileName[64];
    int32_t iIndex;
    STRING512 ExecDir;
    uint16_t *p16BPPData;

    Plat_GetExecutableDirectory(ExecDir, sizeof(ExecDir));
    Plat_SetCurrentDirectory(ExecDir);

    //
    // Create temporary system memory surface. This is used to correct problems with the backbuffer
    // surface which can be interlaced or have a funky pitch
    //

    memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
    SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
    SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
    SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    SurfaceDescription.dwWidth = usScreenWidth;
    SurfaceDescription.dwHeight = usScreenHeight;
    ReturnCode =
        IDirectDraw2_CreateSurface(gpDirectDrawObject, &SurfaceDescription, &_pTmpBuffer, NULL);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    }

    IID tmpID = IID_IDirectDrawSurface2;
    ReturnCode = IDirectDrawSurface_QueryInterface((IDirectDrawSurface *)_pTmpBuffer, &tmpID,
                                                   (LPVOID *)&pTmpBuffer);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    }

    //
    // Copy the primary surface to the temporary surface
    //

    Region.left = 0;
    Region.top = 0;
    Region.right = usScreenWidth;
    Region.bottom = usScreenHeight;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(pTmpBuffer, 0, 0, gpPrimarySurface, &Region,
                                               DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      }
    } while (ReturnCode != DD_OK);

    //
    // Ok now that temp surface has contents of backbuffer, copy temp surface to disk
    //

    sprintf(FileName, "SCREEN%03d.TGA", guiPrintFrameBufferIndex++);
    if ((OutputFile = fopen(FileName, "wb")) != NULL) {
      fprintf(OutputFile, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0x80, 0x02, 0xe0, 0x01, 0x10, 0);

      //
      // Lock temp surface
      //

      memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
      SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
      ReturnCode = IDirectDrawSurface2_Lock(pTmpBuffer, NULL, &SurfaceDescription, 0, NULL);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      }

      //
      // Copy 16 bit buffer to file
      //

      // 5/6/5.. create buffer...
      if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F) {
        p16BPPData = (uint16_t *)MemAlloc(640 * 2);
      }

      for (iIndex = 479; iIndex >= 0; iIndex--) {
        // ATE: OK, fix this such that it converts pixel format to 5/5/5
        // if current settings are 5/6/5....
        if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F) {
          // Read into a buffer...
          memcpy(p16BPPData, (((uint8_t *)SurfaceDescription.lpSurface) + (iIndex * 640 * 2)),
                 640 * 2);

          // Convert....
          ConvertRGBDistribution565To555(p16BPPData, 640);

          // Write
          fwrite(p16BPPData, 640 * 2, 1, OutputFile);
        } else {
          fwrite((void *)(((uint8_t *)SurfaceDescription.lpSurface) + (iIndex * 640 * 2)), 640 * 2,
                 1, OutputFile);
        }
      }

      // 5/6/5.. Delete buffer...
      if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F) {
        MemFree(p16BPPData);
      }

      fclose(OutputFile);

      //
      // Unlock temp surface
      //

      memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
      SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
      ReturnCode = IDirectDrawSurface2_Unlock(pTmpBuffer, &SurfaceDescription);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      }
    }

    //
    // Release temp surface
    //

    gfPrintFrameBuffer = FALSE;
    IDirectDrawSurface2_Release(pTmpBuffer);

    strcat(ExecDir, "\\Data");
    Plat_SetCurrentDirectory(ExecDir);
  }

  //
  // Ok we were able to get a hold of the frame buffer stuff. Check to see if it needs updating
  // if not, release the frame buffer stuff right away
  //

  if (guiMouseBufferState == BUFFER_DIRTY) {
    //
    // Well the mouse buffer is dirty. Upload the whole thing
    //

    Region.left = 0;
    Region.top = 0;
    Region.right = gusMouseCursorWidth;
    Region.bottom = gusMouseCursorHeight;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(gpMouseCursor, 0, 0, gpMouseCursorOriginal,
                                               (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      }
    } while (ReturnCode != DD_OK);

    guiMouseBufferState = BUFFER_READY;
  }

  //
  // Check current state of the mouse cursor
  //

  if (fShowMouse == FALSE) {
    if (guiMouseBufferState == BUFFER_READY) {
      fShowMouse = TRUE;
    } else {
      fShowMouse = FALSE;
    }
  } else {
    if (guiMouseBufferState == BUFFER_DISABLED) {
      fShowMouse = FALSE;
    }
  }

  if (fShowMouse == TRUE) {
    //
    // Step (1) - Save mouse background
    //

    Region.left = MousePos.x - gsMouseCursorXOffset;
    Region.top = MousePos.y - gsMouseCursorYOffset;
    Region.right = Region.left + gusMouseCursorWidth;
    Region.bottom = Region.top + gusMouseCursorHeight;

    if (Region.right > usScreenWidth) {
      Region.right = usScreenWidth;
    }

    if (Region.bottom > usScreenHeight) {
      Region.bottom = usScreenHeight;
    }

    if ((Region.right > Region.left) && (Region.bottom > Region.top)) {
      //
      // Make sure the mouse background is marked for restore and coordinates are saved for the
      // future restore
      //

      gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = TRUE;
      gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight =
          (uint16_t)Region.right - (uint16_t)Region.left;
      gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom =
          (uint16_t)Region.bottom - (uint16_t)Region.top;
      if (Region.left < 0) {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft = (uint16_t)(0 - Region.left);
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos = 0;
        Region.left = 0;
      } else {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos =
            (uint16_t)MousePos.x - gsMouseCursorXOffset;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft = 0;
      }
      if (Region.top < 0) {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos = 0;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop = (uint16_t)(0 - Region.top);
        Region.top = 0;
      } else {
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos =
            (uint16_t)MousePos.y - gsMouseCursorYOffset;
        gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop = 0;
      }

      if ((Region.right > Region.left) && (Region.bottom > Region.top)) {
        // Save clipped region
        gMouseCursorBackground[CURRENT_MOUSE_DATA].Region = Region;

        //
        // Ok, do the actual data save to the mouse background
        //

        do {
          ReturnCode =
              IDirectDrawSurface2_BltFast(gMouseCursorBackground[CURRENT_MOUSE_DATA].pSurface,
                                          gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft,
                                          gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop,
                                          gpBackBuffer, &Region, DDBLTFAST_NOCOLORKEY);
          if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
            DirectXAttempt(ReturnCode, __LINE__, __FILE__);
          }

          if (ReturnCode == DDERR_SURFACELOST) {
            goto ENDOFLOOP;
          }
        } while (ReturnCode != DD_OK);

        //
        // Step (2) - Blit mouse cursor to back buffer
        //

        Region.left = gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft;
        Region.top = gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop;
        Region.right = gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight;
        Region.bottom = gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom;

        do {
          ReturnCode = IDirectDrawSurface2_BltFast(
              gpBackBuffer, gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
              gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos, gpMouseCursor, &Region,
              DDBLTFAST_SRCCOLORKEY);
          if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
            DirectXAttempt(ReturnCode, __LINE__, __FILE__);
          }

          if (ReturnCode == DDERR_SURFACELOST) {
            goto ENDOFLOOP;
          }
        } while (ReturnCode != DD_OK);
      } else {
        //
        // Hum, the mouse was not blitted this round. Henceforth we will flag fRestore as FALSE
        //

        gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = FALSE;
      }

    } else {
      //
      // Hum, the mouse was not blitted this round. Henceforth we will flag fRestore as FALSE
      //

      gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = FALSE;
    }
  } else {
    //
    // Well since there was no mouse handling this round, we disable the mouse restore
    //

    gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore = FALSE;
  }

  //
  // Step (1) - Flip pages
  //
  do {
    ReturnCode = IDirectDrawSurface_Flip(_gpPrimarySurface, NULL, DDFLIP_WAIT);
    //    if ((ReturnCode != DD_OK)&&(ReturnCode != DDERR_WASSTILLDRAWING))
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);

      if (ReturnCode == DDERR_SURFACELOST) {
        goto ENDOFLOOP;
      }
    }

  } while (ReturnCode != DD_OK);

  //
  // Step (2) - Copy Primary Surface to the Back Buffer
  //
  if (gfRenderScroll) {
    Region.left = 0;
    Region.top = 0;
    Region.right = 640;
    Region.bottom = 360;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(gpBackBuffer, 0, 0, gpPrimarySurface, &Region,
                                               DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);

        if (ReturnCode == DDERR_SURFACELOST) {
          goto ENDOFLOOP;
        }
      }
    } while (ReturnCode != DD_OK);

    // Get new background for mouse
    //
    // Ok, do the actual data save to the mouse background

    //

    gfRenderScroll = FALSE;
    gfScrollStart = FALSE;
  }

  // COPY MOUSE AREAS FROM PRIMARY BACK!

  // FIRST OLD ERASED POSITION
  if (gMouseCursorBackground[PREVIOUS_MOUSE_DATA].fRestore == TRUE) {
    Region = gMouseCursorBackground[PREVIOUS_MOUSE_DATA].Region;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(
          gpBackBuffer, gMouseCursorBackground[PREVIOUS_MOUSE_DATA].usMouseXPos,
          gMouseCursorBackground[PREVIOUS_MOUSE_DATA].usMouseYPos, gpPrimarySurface,
          (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
      if (ReturnCode != DD_OK && ReturnCode != DDERR_WASSTILLDRAWING) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);

        if (ReturnCode == DDERR_SURFACELOST) {
          goto ENDOFLOOP;
        }
      }
    } while (ReturnCode != DD_OK);
  }

  // NOW NEW MOUSE AREA
  if (gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore == TRUE) {
    Region = gMouseCursorBackground[CURRENT_MOUSE_DATA].Region;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(
          gpBackBuffer, gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
          gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos, gpPrimarySurface, (LPRECT)&Region,
          DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);

        if (ReturnCode == DDERR_SURFACELOST) {
          goto ENDOFLOOP;
        }
      }
    } while (ReturnCode != DD_OK);
  }

  if (gfForceFullScreenRefresh == TRUE) {
    //
    // Method (1) - We will be refreshing the entire screen
    //
    Region.left = 0;
    Region.top = 0;
    Region.right = SCREEN_WIDTH;
    Region.bottom = SCREEN_HEIGHT;

    do {
      ReturnCode = IDirectDrawSurface2_BltFast(gpBackBuffer, 0, 0, gpPrimarySurface, &Region,
                                               DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);

        if (ReturnCode == DDERR_SURFACELOST) {
          goto ENDOFLOOP;
        }
      }
    } while (ReturnCode != DD_OK);

    guiDirtyRegionCount = 0;
    guiDirtyRegionExCount = 0;
    gfForceFullScreenRefresh = FALSE;
  } else {
    for (uiIndex = 0; uiIndex < guiDirtyRegionCount; uiIndex++) {
      Region.left = gListOfDirtyRegions[uiIndex].iLeft;
      Region.top = gListOfDirtyRegions[uiIndex].iTop;
      Region.right = gListOfDirtyRegions[uiIndex].iRight;
      Region.bottom = gListOfDirtyRegions[uiIndex].iBottom;

      do {
        ReturnCode =
            IDirectDrawSurface2_BltFast(gpBackBuffer, Region.left, Region.top, gpPrimarySurface,
                                        (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
        if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
          DirectXAttempt(ReturnCode, __LINE__, __FILE__);
        }

        if (ReturnCode == DDERR_SURFACELOST) {
          goto ENDOFLOOP;
        }
      } while (ReturnCode != DD_OK);
    }

    guiDirtyRegionCount = 0;
    gfForceFullScreenRefresh = FALSE;
  }

  // Do extended dirty regions!
  for (uiIndex = 0; uiIndex < guiDirtyRegionExCount; uiIndex++) {
    Region.left = gDirtyRegionsEx[uiIndex].iLeft;
    Region.top = gDirtyRegionsEx[uiIndex].iTop;
    Region.right = gDirtyRegionsEx[uiIndex].iRight;
    Region.bottom = gDirtyRegionsEx[uiIndex].iBottom;

    if ((Region.top < gsVIEWPORT_WINDOW_END_Y) && gfRenderScroll) {
      continue;
    }

    do {
      ReturnCode =
          IDirectDrawSurface2_BltFast(gpBackBuffer, Region.left, Region.top, gpPrimarySurface,
                                      (LPRECT)&Region, DDBLTFAST_NOCOLORKEY);
      if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
        DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      }

      if (ReturnCode == DDERR_SURFACELOST) {
        goto ENDOFLOOP;
      }
    } while (ReturnCode != DD_OK);
  }

  guiDirtyRegionExCount = 0;

ENDOFLOOP:

  fFirstTime = FALSE;
}

static void *LockPrimarySurface(LPDIRECTDRAWSURFACE2 _surf, uint32_t *uiPitch) {
  HRESULT ReturnCode;
  DDSURFACEDESC SurfaceDescription;

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);

  do {
    ReturnCode = IDirectDrawSurface2_Lock(_surf, NULL, &SurfaceDescription, 0, NULL);
    if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
      DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Failed to lock");
      DirectXAttempt(ReturnCode, __LINE__, __FILE__);
      return NULL;
    }

  } while (ReturnCode != DD_OK);

  *uiPitch = SurfaceDescription.lPitch;
  return SurfaceDescription.lpSurface;
}

BOOLEAN GetRGBDistribution(void) {
  DDSURFACEDESC SurfaceDescription;
  uint16_t usBit;
  HRESULT ReturnCode;

  Assert(gpPrimarySurface != NULL);

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwFlags = DDSD_PIXELFORMAT;
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(gpPrimarySurface, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return FALSE;
  }

  //
  // Ok we now have the surface description, we now can get the information that we need
  //

  gusRedMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  gusGreenMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  gusBlueMask = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  // RGB 5,5,5
  if ((gusRedMask == 0x7c00) && (gusGreenMask == 0x03e0) && (gusBlueMask == 0x1f))
    guiTranslucentMask = 0x3def;
  // RGB 5,6,5
  else  // if((gusRedMask==0xf800) && (gusGreenMask==0x03e0) && (gusBlueMask==0x1f))
    guiTranslucentMask = 0x7bef;

  usBit = 0x8000;
  gusRedShift = 8;
  while (!(gusRedMask & usBit)) {
    usBit >>= 1;
    gusRedShift--;
  }

  usBit = 0x8000;
  gusGreenShift = 8;
  while (!(gusGreenMask & usBit)) {
    usBit >>= 1;
    gusGreenShift--;
  }

  usBit = 0x8000;
  gusBlueShift = 8;
  while (!(gusBlueMask & usBit)) {
    usBit >>= 1;
    gusBlueShift--;
  }

  return TRUE;
}

BOOLEAN GetPrimaryRGBDistributionMasks(uint32_t *RedBitMask, uint32_t *GreenBitMask,
                                       uint32_t *BlueBitMask) {
  *RedBitMask = gusRedMask;
  *GreenBitMask = gusGreenMask;
  *BlueBitMask = gusBlueMask;

  return TRUE;
}

BOOLEAN SetMouseCursorProperties(int16_t sOffsetX, int16_t sOffsetY, uint16_t usCursorHeight,
                                 uint16_t usCursorWidth) {
  gsMouseCursorXOffset = sOffsetX;
  gsMouseCursorYOffset = sOffsetY;
  gusMouseCursorWidth = usCursorWidth;
  gusMouseCursorHeight = usCursorHeight;
  return (TRUE);
}

void DirtyCursor() { guiMouseBufferState = BUFFER_DIRTY; }

BOOLEAN SetCurrentCursor(uint16_t usVideoObjectSubIndex, uint16_t usOffsetX, uint16_t usOffsetY) {
  BOOLEAN ReturnValue;
  ETRLEObject pETRLEPointer;

  //
  // Make sure we have a cursor store
  //

  if (gpCursorStore == NULL) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "ERROR : Cursor store is not loaded");
    return FALSE;
  }

  EraseMouseCursor();

  //
  // Get new cursor data
  //

  ReturnValue = BltVideoObjectOld(MOUSE_BUFFER, gpCursorStore, usVideoObjectSubIndex, 0, 0);
  guiMouseBufferState = BUFFER_DIRTY;

  if (GetVideoObjectETRLEProperties(gpCursorStore, &pETRLEPointer, usVideoObjectSubIndex)) {
    gsMouseCursorXOffset = usOffsetX;
    gsMouseCursorYOffset = usOffsetY;
    gusMouseCursorWidth = pETRLEPointer.usWidth + pETRLEPointer.sOffsetX;
    gusMouseCursorHeight = pETRLEPointer.usHeight + pETRLEPointer.sOffsetY;

    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "=================================================");
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0,
             String("Mouse Create with [ %d. %d ] [ %d, %d]", pETRLEPointer.sOffsetX,
                    pETRLEPointer.sOffsetY, pETRLEPointer.usWidth, pETRLEPointer.usHeight));
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "=================================================");
  } else {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Failed to get mouse info");
  }

  return ReturnValue;
}

void EndFrameBufferRender(void) { guiFrameBufferState = BUFFER_DIRTY; }

void PrintScreen(void) { gfPrintFrameBuffer = TRUE; }

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) {
  HRESULT ReturnCode;
  struct SGPPaletteEntry gSgpPalette[256];

  // If we are in 256 colors, then we have to initialize the palette system to 0 (faded out)
  memcpy(gSgpPalette, pPalette, sizeof(struct SGPPaletteEntry) * 256);

  LPDIRECTDRAWPALETTE gpDirectDrawPalette;
  ReturnCode =
      IDirectDraw_CreatePalette(gpDirectDrawObject, (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
                                (LPPALETTEENTRY)(&gSgpPalette[0]), &gpDirectDrawPalette, NULL);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, String("Failed to create palette (Rc = %d)", ReturnCode));
    return (FALSE);
  }
  // Apply the palette to the surfaces
  ReturnCode = IDirectDrawSurface_SetPalette(gpPrimarySurface, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, String("Failed to apply 8-bit palette to primary surface"));
    return (FALSE);
  }

  ReturnCode = IDirectDrawSurface_SetPalette(gpBackBuffer, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, String("Failed to apply 8-bit palette to back buffer"));
    return (FALSE);
  }

  ReturnCode = IDirectDrawSurface_SetPalette(gpFrameBuffer, gpDirectDrawPalette);
  if (ReturnCode != DD_OK) {
    DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, String("Failed to apply 8-bit palette to frame buffer"));
    return (FALSE);
  }

  return (TRUE);
}

void FatalError(char *pError, ...) {
  va_list argptr;

  va_start(argptr, pError);  // Set up variable argument pointer
  vsprintf(gFatalErrorString, pError, argptr);
  va_end(argptr);

  IDirectDraw2_RestoreDisplayMode(gpDirectDrawObject);
  IDirectDraw2_Release(gpDirectDrawObject);
  ShowWindow(ghWindow, SW_HIDE);

  gfProgramIsRunning = FALSE;

  MessageBox(ghWindow, gFatalErrorString, "JA2 Fatal Error", MB_OK | MB_TASKMODAL);
}

/*********************************************************************************
 * SnapshotSmall
 *
 *		Grabs a screen from the [rimary surface, and stuffs it into a 16-bit (RGB 5,5,5),
 * uncompressed Targa file. Each time the routine is called, it increments the
 * file number by one. The files are create in the current directory, usually the
 * EXE directory. This routine produces 1/4 sized images.
 *
 *********************************************************************************/

#pragma pack(push, 1)

typedef struct {
  uint8_t ubIDLength;
  uint8_t ubColorMapType;
  uint8_t ubTargaType;
  uint16_t usColorMapOrigin;
  uint16_t usColorMapLength;
  uint8_t ubColorMapEntrySize;
  uint16_t usOriginX;
  uint16_t usOriginY;
  uint16_t usImageWidth;
  uint16_t usImageHeight;
  uint8_t ubBitsPerPixel;
  uint8_t ubImageDescriptor;

} TARGA_HEADER;

#pragma pack(pop)

static void SnapshotSmall(void) {
  int32_t iCountX, iCountY;
  DDSURFACEDESC SurfaceDescription;
  uint16_t *pVideo, *pDest;

  HRESULT ReturnCode;

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  ReturnCode = IDirectDrawSurface2_Lock(gpPrimarySurface, NULL, &SurfaceDescription, 0, NULL);
  if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
    return;
  }

  // Get the write pointer
  pVideo = (uint16_t *)SurfaceDescription.lpSurface;

  pDest = gpFrameData[giNumFrames];

  for (iCountY = SCREEN_HEIGHT - 1; iCountY >= 0; iCountY -= 1) {
    for (iCountX = 0; iCountX < SCREEN_WIDTH; iCountX += 1) {
      *(pDest + (iCountY * 640) + (iCountX)) = *(pVideo + (iCountY * 640) + (iCountX));
    }
  }

  giNumFrames++;

  if (giNumFrames == MAX_NUM_FRAMES) {
    RefreshMovieCache();
  }

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  ReturnCode = IDirectDrawSurface2_Unlock(gpPrimarySurface, &SurfaceDescription);
  if ((ReturnCode != DD_OK) && (ReturnCode != DDERR_WASSTILLDRAWING)) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
  }

  //	fclose(disk);
}

void VideoCaptureToggle(void) {
#ifdef JA2TESTVERSION
  VideoMovieCapture((BOOLEAN)!gfVideoCapture);
#endif
}

static void VideoMovieCapture(BOOLEAN fEnable) {
  int32_t cnt;

  gfVideoCapture = fEnable;
  if (fEnable) {
    for (cnt = 0; cnt < MAX_NUM_FRAMES; cnt++) {
      gpFrameData[cnt] = (uint16_t *)MemAlloc(640 * 480 * 2);
    }

    giNumFrames = 0;

    guiLastFrame = Plat_GetTickCount();
  } else {
    RefreshMovieCache();

    for (cnt = 0; cnt < MAX_NUM_FRAMES; cnt++) {
      if (gpFrameData[cnt] != NULL) {
        MemFree(gpFrameData[cnt]);
      }
    }
    giNumFrames = 0;
  }
}

static void RefreshMovieCache() {
  TARGA_HEADER Header;
  int32_t iCountX, iCountY;
  FILE *disk;
  char cFilename[_MAX_PATH];
  static uint32_t uiPicNum = 0;
  uint16_t *pDest;
  int32_t cnt;
  STRING512 ExecDir;

  PauseTime(TRUE);

  Plat_GetExecutableDirectory(ExecDir, sizeof(ExecDir));
  Plat_SetCurrentDirectory(ExecDir);

  for (cnt = 0; cnt < giNumFrames; cnt++) {
    sprintf(cFilename, "JA%5.5d.TGA", uiPicNum++);

    if ((disk = fopen(cFilename, "wb")) == NULL) return;

    memset(&Header, 0, sizeof(TARGA_HEADER));

    Header.ubTargaType = 2;  // Uncompressed 16/24/32 bit
    Header.usImageWidth = 640;
    Header.usImageHeight = 480;
    Header.ubBitsPerPixel = 16;

    fwrite(&Header, sizeof(TARGA_HEADER), 1, disk);

    pDest = gpFrameData[cnt];

    for (iCountY = 480 - 1; iCountY >= 0; iCountY -= 1) {
      for (iCountX = 0; iCountX < 640; iCountX++) {
        fwrite((pDest + (iCountY * 640) + iCountX), sizeof(uint16_t), 1, disk);
      }
    }

    fclose(disk);
  }

  PauseTime(FALSE);

  giNumFrames = 0;

  strcat(ExecDir, "\\Data");
  Plat_SetCurrentDirectory(ExecDir);
}

#include "SGP/VSurface.h"
#include "SGP/WCheck.h"

static void DeletePrimaryVideoSurfaces();

BOOLEAN SetPrimaryVideoSurfaces() {
  DeletePrimaryVideoSurfaces();

  vsPrimary = CreateVideoSurfaceFromDDSurface(gpPrimarySurface);
  if (!vsPrimary) {
    return FALSE;
  }

  vsBackBuffer = CreateVideoSurfaceFromDDSurface(gpBackBuffer);
  if (!vsBackBuffer) {
    return FALSE;
  }

  vsMouseBuffer = CreateVideoSurfaceFromDDSurface(gpMouseCursor);
  if (!vsMouseBuffer) {
    return FALSE;
  }

  vsFB = CreateVideoSurfaceFromDDSurface(gpFrameBuffer);
  if (!vsFB) {
    return FALSE;
  }

  return (TRUE);
}

static void DeletePrimaryVideoSurfaces() {
  //
  // If globals are not null, delete them
  //

  if (vsPrimary != NULL) {
    DeleteVideoSurface(vsPrimary);
    vsPrimary = NULL;
  }

  if (vsBackBuffer != NULL) {
    DeleteVideoSurface(vsBackBuffer);
    vsBackBuffer = NULL;
  }

  if (vsFB != NULL) {
    DeleteVideoSurface(vsFB);
    vsFB = NULL;
  }

  if (vsMouseBuffer != NULL) {
    DeleteVideoSurface(vsMouseBuffer);
    vsMouseBuffer = NULL;
  }
}

//////////////////////////////////////////////////////////////////
// VSurface
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "SGP/Debug.h"
#include "SGP/HImage.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "platform_strings.h"

extern void SetClippingRect(SGPRect *clip);
extern void GetClippingRect(SGPRect *clip);

#define DEFAULT_NUM_REGIONS 5
#define DEFAULT_VIDEO_SURFACE_LIST_SIZE 10

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                           struct Rect *SrcRect);

void DeletePrimaryVideoSurfaces();

#ifdef _DEBUG
enum {
  DEBUGSTR_NONE,
  DEBUGSTR_SETVIDEOSURFACETRANSPARENCY,
  DEBUGSTR_ADDVIDEOSURFACEREGION,
  DEBUGSTR_GETVIDEOSURFACEDESCRIPTION,
  DEBUGSTR_BLTVIDEOSURFACE_DST,
  DEBUGSTR_BLTVIDEOSURFACE_SRC,
  DEBUGSTR_COLORFILLVIDEOSURFACEAREA,
  DEBUGSTR_SHADOWVIDEOSURFACERECT,
  DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST,
  DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC,
  DEBUGSTR_DeleteVSurfaceByIndex
};

uint8_t gubVSDebugCode = 0;

void CheckValidVSurfaceIndex(uint32_t uiIndex);
#endif

int32_t giMemUsedInSurfaces;

BOOLEAN InitializeVideoSurfaceManager() {
  InitVSurfaceList();
  RegisterDebugTopic(TOPIC_VIDEOSURFACE, "Video Surface Manager");

  giMemUsedInSurfaces = 0;

  // Create primary and backbuffer from globals
  if (!SetPrimaryVideoSurfaces()) {
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_1, String("Could not create primary surfaces"));
    return FALSE;
  }

  return TRUE;
}

BOOLEAN ShutdownVideoSurfaceManager() {
  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Shutting down the Video Surface manager");
  DeletePrimaryVideoSurfaces();
  DeinitVSurfaceList();
  UnRegisterDebugTopic(TOPIC_VIDEOSURFACE, "Video Objects");
  return TRUE;
}

uint8_t *LockVSurfaceByID(uint32_t uiVSurface, uint32_t *puiPitch) {
  //
  // Check if given backbuffer or primary buffer
  //
  if (uiVSurface == PRIMARY_SURFACE) {
    return (uint8_t *)LockPrimarySurface(gpPrimarySurface, puiPitch);
  }

  if (uiVSurface == BACKBUFFER) {
    return (uint8_t *)LockPrimarySurface(gpBackBuffer, puiPitch);
  }

  if (uiVSurface == vsIndexFB) {
    return (uint8_t *)LockPrimarySurface(gpFrameBuffer, puiPitch);
  }

  if (uiVSurface == MOUSE_BUFFER) {
    return (uint8_t *)LockPrimarySurface(gpMouseCursorOriginal, puiPitch);
  }

  struct VSurface *vs = FindVSurface(uiVSurface);
  if (!vs) {
    return FALSE;
  }

  return LockVSurface(vs, puiPitch);
}

void UnlockVSurfaceByID(VSurfID id) {
  switch (id) {
    case PRIMARY_SURFACE:
      IDirectDrawSurface2_Unlock(gpPrimarySurface, NULL);
      break;
    case BACKBUFFER:
      IDirectDrawSurface2_Unlock(gpBackBuffer, NULL);
      break;
    case vsIndexFB:
      IDirectDrawSurface2_Unlock(gpFrameBuffer, NULL);
      break;
    case MOUSE_BUFFER:
      IDirectDrawSurface2_Unlock(gpMouseCursorOriginal, NULL);
      break;
    default: {
      struct VSurface *vs = FindVSurface(id);
      if (vs) {
        UnlockVSurface(vs);
      }
    }
  }
}

BOOLEAN SetVideoSurfaceTransparency(uint32_t uiIndex, COLORVAL TransColor) {
  struct VSurface *hVSurface;

  //
  // Get Video Surface
  //

#ifdef _DEBUG
  gubVSDebugCode = DEBUGSTR_SETVIDEOSURFACETRANSPARENCY;
#endif
  CHECKF(GetVSurfaceByIndexOld(&hVSurface, uiIndex));

  //
  // Set transparency
  //

  SetVideoSurfaceTransparencyColor(hVSurface, TransColor);

  return (TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Given an index to the dest and src vobject contained in our private VSurface list
// Based on flags, blit accordingly
// There are two types, a BltFast and a Blt. BltFast is 10% faster, uses no
// clipping lists
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN BltVideoSurface(uint32_t uiDestVSurface, uint32_t uiSrcVSurface, uint16_t usRegionIndex,
                        int32_t iDestX, int32_t iDestY, uint32_t fBltFlags, SGPRect *srcRect) {
  struct VSurface *hDestVSurface;
  struct VSurface *hSrcVSurface;

#ifdef _DEBUG
  gubVSDebugCode = DEBUGSTR_BLTVIDEOSURFACE_DST;
#endif
  if (!GetVSurfaceByIndexOld(&hDestVSurface, uiDestVSurface)) {
    return FALSE;
  }
#ifdef _DEBUG
  gubVSDebugCode = DEBUGSTR_BLTVIDEOSURFACE_SRC;
#endif
  if (!GetVSurfaceByIndexOld(&hSrcVSurface, uiSrcVSurface)) {
    return FALSE;
  }
  if (!BltVSurfaceToVSurface(hDestVSurface, hSrcVSurface, usRegionIndex, iDestX, iDestY, fBltFlags,
                             srcRect)) {
    // VO Blitter will set debug messages for error conditions
    return FALSE;
  }
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Fills an rectangular area with a specified color value.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN ColorFillVideoSurfaceArea(uint32_t uiDestVSurface, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, uint16_t Color16BPP) {
  struct VSurface *hDestVSurface;

#ifdef _DEBUG
  gubVSDebugCode = DEBUGSTR_COLORFILLVIDEOSURFACEAREA;
#endif
  if (!GetVSurfaceByIndexOld(&hDestVSurface, uiDestVSurface)) {
    return FALSE;
  }

  SGPRect Clip;
  GetClippingRect(&Clip);

  if (iDestX1 < Clip.iLeft) iDestX1 = Clip.iLeft;

  if (iDestX1 > Clip.iRight) return (FALSE);

  if (iDestX2 > Clip.iRight) iDestX2 = Clip.iRight;

  if (iDestX2 < Clip.iLeft) return (FALSE);

  if (iDestY1 < Clip.iTop) iDestY1 = Clip.iTop;

  if (iDestY1 > Clip.iBottom) return (FALSE);

  if (iDestY2 > Clip.iBottom) iDestY2 = Clip.iBottom;

  if (iDestY2 < Clip.iTop) return (FALSE);

  if ((iDestX2 <= iDestX1) || (iDestY2 <= iDestY1)) return (FALSE);

  RECT r;
  r.left = iDestX1;
  r.top = iDestY1;
  r.right = iDestX2;
  r.bottom = iDestY2;

  {
    DDBLTFX BlitterFX;
    BlitterFX.dwSize = sizeof(DDBLTFX);
    BlitterFX.dwFillColor = Color16BPP;
    DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->_platformData2, &r, NULL, NULL,
                 DDBLT_COLORFILL, &BlitterFX);
  }
  return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Fills an rectangular area with a specified image value.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN ImageFillVideoSurfaceArea(uint32_t uiDestVSurface, int32_t iDestX1, int32_t iDestY1,
                                  int32_t iDestX2, int32_t iDestY2, struct VObject *BkgrndImg,
                                  uint16_t Index, int16_t Ox, int16_t Oy) {
  int16_t xc, yc, hblits, wblits, aw, pw, ah, ph, w, h, xo, yo;
  ETRLEObject *pTrav;
  SGPRect NewClip, OldClip;

  pTrav = &(BkgrndImg->pETRLEObject[Index]);
  ph = (int16_t)(pTrav->usHeight + pTrav->sOffsetY);
  pw = (int16_t)(pTrav->usWidth + pTrav->sOffsetX);

  ah = (int16_t)(iDestY2 - iDestY1);
  aw = (int16_t)(iDestX2 - iDestX1);

  Ox %= pw;
  Oy %= ph;

  if (Ox > 0) Ox -= pw;
  xo = (-Ox) % pw;

  if (Oy > 0) Oy -= ph;
  yo = (-Oy) % ph;

  if (Ox < 0)
    xo = (-Ox) % pw;
  else {
    xo = pw - (Ox % pw);
    Ox -= pw;
  }

  if (Oy < 0)
    yo = (-Oy) % ph;
  else {
    yo = ph - (Oy % pw);
    Oy -= ph;
  }

  hblits = ((ah + yo) / ph) + (((ah + yo) % ph) ? 1 : 0);
  wblits = ((aw + xo) / pw) + (((aw + xo) % pw) ? 1 : 0);

  if ((hblits == 0) || (wblits == 0)) return (FALSE);

  //
  // Clip fill region coords
  //

  GetClippingRect(&OldClip);

  NewClip.iLeft = iDestX1;
  NewClip.iTop = iDestY1;
  NewClip.iRight = iDestX2;
  NewClip.iBottom = iDestY2;

  if (NewClip.iLeft < OldClip.iLeft) NewClip.iLeft = OldClip.iLeft;

  if (NewClip.iLeft > OldClip.iRight) return (FALSE);

  if (NewClip.iRight > OldClip.iRight) NewClip.iRight = OldClip.iRight;

  if (NewClip.iRight < OldClip.iLeft) return (FALSE);

  if (NewClip.iTop < OldClip.iTop) NewClip.iTop = OldClip.iTop;

  if (NewClip.iTop > OldClip.iBottom) return (FALSE);

  if (NewClip.iBottom > OldClip.iBottom) NewClip.iBottom = OldClip.iBottom;

  if (NewClip.iBottom < OldClip.iTop) return (FALSE);

  if ((NewClip.iRight <= NewClip.iLeft) || (NewClip.iBottom <= NewClip.iTop)) return (FALSE);

  SetClippingRect(&NewClip);

  yc = (int16_t)iDestY1;
  for (h = 0; h < hblits; h++) {
    xc = (int16_t)iDestX1;
    for (w = 0; w < wblits; w++) {
      BltVideoObjectOld(uiDestVSurface, BkgrndImg, Index, xc + Ox, yc + Oy);
      xc += pw;
    }
    yc += ph;
  }

  SetClippingRect(&OldClip);
  return (TRUE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manipulation Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

struct VSurface *CreateVSurfaceBlank(uint16_t width, uint16_t height, uint8_t bitDepth) {
  Assert(height > 0);
  Assert(width > 0);

  DDPIXELFORMAT PixelFormat;
  memset(&PixelFormat, 0, sizeof(PixelFormat));
  PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

  switch (bitDepth) {
    case 8: {
      PixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
      PixelFormat.dwRGBBitCount = 8;
    } break;

    case 16: {
      PixelFormat.dwFlags = DDPF_RGB;
      PixelFormat.dwRGBBitCount = 16;

      uint32_t uiRBitMask;
      uint32_t uiGBitMask;
      uint32_t uiBBitMask;
      CHECKF(GetPrimaryRGBDistributionMasks(&uiRBitMask, &uiGBitMask, &uiBBitMask));
      PixelFormat.dwRBitMask = uiRBitMask;
      PixelFormat.dwGBitMask = uiGBitMask;
      PixelFormat.dwBBitMask = uiBBitMask;
    } break;

    default:
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid BPP value, can only be 8 or 16.");
      return (FALSE);
  }

  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(DDSURFACEDESC));
  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwWidth = width;
  SurfaceDescription.dwHeight = height;
  SurfaceDescription.ddpfPixelFormat = PixelFormat;

  LPDIRECTDRAWSURFACE lpDDS;
  LPDIRECTDRAWSURFACE2 lpDDS2;
  LPDIRECTDRAW2 lpDD2Object = gpDirectDrawObject;
  DDCreateSurface(lpDD2Object, &SurfaceDescription, &lpDDS, &lpDDS2);

  struct VSurface *vs = (struct VSurface *)MemAlloc(sizeof(struct VSurface));
  CHECKF(vs != NULL);
  memset(vs, 0, sizeof(struct VSurface));

  vs->usHeight = height;
  vs->usWidth = width;
  vs->ubBitDepth = bitDepth;
  vs->_platformData1 = (void *)lpDDS;
  vs->_platformData2 = (void *)lpDDS2;

  giMemUsedInSurfaces += (vs->usHeight * vs->usWidth * (vs->ubBitDepth / 8));

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Success in Creating Video Surface"));

  return (vs);
}

struct VSurface *CreateVSurfaceFromFile(const char *filepath) {
  DDSURFACEDESC SurfaceDescription;
  memset(&SurfaceDescription, 0, sizeof(DDSURFACEDESC));

  LPDIRECTDRAW2 lpDD2Object = gpDirectDrawObject;

  HIMAGE hImage = CreateImage(filepath, IMAGE_ALLIMAGEDATA);

  if (hImage == NULL) {
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid Image Filename given");
    return (NULL);
  }

  uint16_t usHeight = hImage->usHeight;
  uint16_t usWidth = hImage->usWidth;
  uint8_t ubBitDepth = hImage->ubBitDepth;

  Assert(usHeight > 0);
  Assert(usWidth > 0);

  DDPIXELFORMAT PixelFormat;
  memset(&PixelFormat, 0, sizeof(PixelFormat));
  PixelFormat.dwSize = sizeof(DDPIXELFORMAT);

  switch (ubBitDepth) {
    case 8: {
      PixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
      PixelFormat.dwRGBBitCount = 8;
    } break;

    case 16: {
      PixelFormat.dwFlags = DDPF_RGB;
      PixelFormat.dwRGBBitCount = 16;

      uint32_t uiRBitMask;
      uint32_t uiGBitMask;
      uint32_t uiBBitMask;
      CHECKF(GetPrimaryRGBDistributionMasks(&uiRBitMask, &uiGBitMask, &uiBBitMask));
      PixelFormat.dwRBitMask = uiRBitMask;
      PixelFormat.dwGBitMask = uiGBitMask;
      PixelFormat.dwBBitMask = uiBBitMask;
    } break;

    default:
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid BPP value, can only be 8 or 16.");
      return (FALSE);
  }

  SurfaceDescription.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;

  SurfaceDescription.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  SurfaceDescription.dwWidth = usWidth;
  SurfaceDescription.dwHeight = usHeight;
  SurfaceDescription.ddpfPixelFormat = PixelFormat;

  LPDIRECTDRAWSURFACE lpDDS;
  LPDIRECTDRAWSURFACE2 lpDDS2;
  DDCreateSurface(lpDD2Object, &SurfaceDescription, &lpDDS, &lpDDS2);

  struct VSurface *vs = (struct VSurface *)MemAlloc(sizeof(struct VSurface));
  memset(vs, 0, sizeof(struct VSurface));
  CHECKF(vs != NULL);

  vs->usHeight = usHeight;
  vs->usWidth = usWidth;
  vs->ubBitDepth = ubBitDepth;
  vs->_platformData1 = (void *)lpDDS;
  vs->_platformData2 = (void *)lpDDS2;
  vs->pPalette = NULL;
  vs->p16BPPPalette = NULL;
  vs->TransparentColor = FROMRGB(0, 0, 0);

  SGPRect tempRect;
  tempRect.iLeft = 0;
  tempRect.iTop = 0;
  tempRect.iRight = hImage->usWidth - 1;
  tempRect.iBottom = hImage->usHeight - 1;
  SetVideoSurfaceDataFromHImage(vs, hImage, 0, 0, &tempRect);

  if (hImage->ubBitDepth == 8) {
    SetVideoSurfacePalette(vs, hImage->pPalette);
  }

  DestroyImage(hImage);

  vs->usHeight = usHeight;
  vs->usWidth = usWidth;
  vs->ubBitDepth = ubBitDepth;

  giMemUsedInSurfaces += (vs->usHeight * vs->usWidth * (vs->ubBitDepth / 8));

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Success in CreateVSurfaceFromFile"));

  return vs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Called when surface is lost, for the most part called by utility functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t *LockVSurface(struct VSurface *vs, uint32_t *pPitch) {
  if (vs == NULL) {
    return NULL;
  }

  DDSURFACEDESC SurfaceDescription;

  DDLockSurface((LPDIRECTDRAWSURFACE2)vs->_platformData2, NULL, &SurfaceDescription, 0, NULL);

  *pPitch = SurfaceDescription.lPitch;

  return (uint8_t *)SurfaceDescription.lpSurface;
}

void UnlockVSurface(struct VSurface *vs) {
  if (vs == NULL) {
    return;
  }
  if (vs == vsPrimary) {
    IDirectDrawSurface2_Unlock(gpPrimarySurface, NULL);
  } else if (vs == vsBackBuffer) {
    IDirectDrawSurface2_Unlock(gpBackBuffer, NULL);
  } else if (vs == vsFB) {
    IDirectDrawSurface2_Unlock(gpFrameBuffer, NULL);
  } else if (vs == vsMouseBuffer) {
    IDirectDrawSurface2_Unlock(gpMouseCursor, NULL);
  } else {
    IDirectDrawSurface2_Unlock((LPDIRECTDRAWSURFACE2)vs->_platformData2, NULL);
  }
}

// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette) {
  Assert(hVSurface != NULL);

  // Create palette object if not already done so
  if (hVSurface->pPalette == NULL) {
    DDCreatePalette(gpDirectDrawObject, (DDPCAPS_8BIT | DDPCAPS_ALLOW256),
                    (LPPALETTEENTRY)(&pSrcPalette[0]), (LPDIRECTDRAWPALETTE *)&hVSurface->pPalette,
                    NULL);
  } else {
    // Just Change entries
    DDSetPaletteEntries((LPDIRECTDRAWPALETTE)hVSurface->pPalette, 0, 0, 256,
                        (PALETTEENTRY *)pSrcPalette);
  }

  // Delete 16BPP Palette if one exists
  if (hVSurface->p16BPPPalette != NULL) {
    MemFree(hVSurface->p16BPPPalette);
    hVSurface->p16BPPPalette = NULL;
  }

  // Create 16BPP Palette
  hVSurface->p16BPPPalette = Create16BPPPalette(pSrcPalette);

  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, String("Video Surface Palette change successfull"));
  return (TRUE);
}

// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
BOOLEAN SetVideoSurfaceTransparencyColor(struct VSurface *vs, COLORVAL TransColor) {
  DDCOLORKEY ColorKey;
  DWORD fFlags = CLR_INVALID;
  LPDIRECTDRAWSURFACE2 lpDDSurface;

  // Assertions
  Assert(vs != NULL);

  // Set trans color into Video Surface
  vs->TransparentColor = TransColor;

  // Get surface pointer
  lpDDSurface = (LPDIRECTDRAWSURFACE2)vs->_platformData2;
  CHECKF(lpDDSurface != NULL);

  // Get right pixel format, based on bit depth

  switch (vs->ubBitDepth) {
    case 8:

      // Use color directly
      ColorKey.dwColorSpaceLowValue = TransColor;
      ColorKey.dwColorSpaceHighValue = TransColor;
      break;

    case 16:

      fFlags = Get16BPPColor(TransColor);

      // fFlags now contains our closest match
      ColorKey.dwColorSpaceLowValue = fFlags;
      ColorKey.dwColorSpaceHighValue = ColorKey.dwColorSpaceLowValue;
      break;
  }

  DDSetSurfaceColorKey(lpDDSurface, DDCKEY_SRCBLT, &ColorKey);

  return (TRUE);
}

BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette) {
  CHECKF(hVSurface->pPalette != NULL);

  DDGetPaletteEntries((LPDIRECTDRAWPALETTE)hVSurface->pPalette, 0, 0, 256,
                      (PALETTEENTRY *)pPalette);

  return (TRUE);
}

// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoSurface(struct VSurface *hVSurface) {
  LPDIRECTDRAWSURFACE2 lpDDSurface;

  // Assertions
  CHECKF(hVSurface != NULL);

  // Release palette
  if (hVSurface->pPalette != NULL) {
    DDReleasePalette((LPDIRECTDRAWPALETTE)hVSurface->pPalette);
    hVSurface->pPalette = NULL;
  }

  // Get surface pointer
  lpDDSurface = (LPDIRECTDRAWSURFACE2)hVSurface->_platformData2;

  // Release surface
  if (hVSurface->_platformData1 != NULL) {
    DDReleaseSurface((LPDIRECTDRAWSURFACE *)&hVSurface->_platformData1, &lpDDSurface);
  }

  // If there is a 16bpp palette, free it
  if (hVSurface->p16BPPPalette != NULL) {
    MemFree(hVSurface->p16BPPPalette);
    hVSurface->p16BPPPalette = NULL;
  }

  giMemUsedInSurfaces -= (hVSurface->usHeight * hVSurface->usWidth * (hVSurface->ubBitDepth / 8));

  // Release object
  MemFree(hVSurface);

  return (TRUE);
}

static BOOLEAN GetVSurfaceRect(struct VSurface *hVSurface, RECT *pRect) {
  Assert(hVSurface != NULL);
  Assert(pRect != NULL);

  pRect->left = 0;
  pRect->top = 0;
  pRect->right = hVSurface->usWidth;
  pRect->bottom = hVSurface->usHeight;

  return (TRUE);
}

// *******************************************************************
//
// Blitting Functions
//
// *******************************************************************

// Blt  will use DD Blt or BltFast depending on flags.
// Will drop down into user-defined blitter if 8->16 BPP blitting is being done

BOOLEAN BltVSurfaceToVSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                              uint16_t usIndex, int32_t iDestX, int32_t iDestY, int32_t fBltFlags,
                              SGPRect *srcRect) {
  RECT SrcRect, DestRect;
  uint8_t *pSrcSurface8, *pDestSurface8;
  uint32_t uiSrcPitch, uiDestPitch, uiWidth, uiHeight;

  // Assertions
  Assert(hDestVSurface != NULL);

  // Check for source coordinate options - from region, specific rect or full src dimensions
  do {
    // Use SUBRECT if specified
    if (fBltFlags & VS_BLT_SRCSUBRECT) {
      SrcRect.top = srcRect->iTop;
      SrcRect.left = srcRect->iLeft;
      SrcRect.bottom = srcRect->iBottom;
      SrcRect.right = srcRect->iRight;

      break;
    }

    // Here, use default, which is entire Video Surface
    // Check Sizes, SRC size MUST be <= DEST size
    if (hDestVSurface->usHeight < hSrcVSurface->usHeight) {
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
                 String("Incompatible height size given in Video Surface blit"));
      return (FALSE);
    }
    if (hDestVSurface->usWidth < hSrcVSurface->usWidth) {
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
                 String("Incompatible height size given in Video Surface blit"));
      return (FALSE);
    }

    SrcRect.top = (int)0;
    SrcRect.left = (int)0;
    SrcRect.bottom = (int)hSrcVSurface->usHeight;
    SrcRect.right = (int)hSrcVSurface->usWidth;

  } while (FALSE);

  // Once here, assert valid Src
  Assert(hSrcVSurface != NULL);

  // clipping -- added by DB
  GetVSurfaceRect(hDestVSurface, &DestRect);
  uiWidth = SrcRect.right - SrcRect.left;
  uiHeight = SrcRect.bottom - SrcRect.top;

  // check for position entirely off the screen
  if (iDestX >= DestRect.right) return (FALSE);
  if (iDestY >= DestRect.bottom) return (FALSE);
  if ((iDestX + (int32_t)uiWidth) < (int32_t)DestRect.left) return (FALSE);
  if ((iDestY + (int32_t)uiHeight) < (int32_t)DestRect.top) return (FALSE);

  if ((iDestX + (int32_t)uiWidth) >= (int32_t)DestRect.right) {
    SrcRect.right -= ((iDestX + uiWidth) - DestRect.right);
    uiWidth -= ((iDestX + uiWidth) - DestRect.right);
  }
  if ((iDestY + (int32_t)uiHeight) >= (int32_t)DestRect.bottom) {
    SrcRect.bottom -= ((iDestY + uiHeight) - DestRect.bottom);
    uiHeight -= ((iDestY + uiHeight) - DestRect.bottom);
  }
  if (iDestX < DestRect.left) {
    SrcRect.left += (DestRect.left - iDestX);
    uiWidth -= (DestRect.left - iDestX);
    iDestX = DestRect.left;
  }
  if (iDestY < (int32_t)DestRect.top) {
    SrcRect.top += (DestRect.top - iDestY);
    uiHeight -= (DestRect.top - iDestY);
    iDestY = DestRect.top;
  }

  // Send dest position, rectangle, etc to DD bltfast function
  // First check BPP values for compatibility
  if (hDestVSurface->ubBitDepth == 16 && hSrcVSurface->ubBitDepth == 16) {
    struct Rect srcRect = {SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom};
    CHECKF(BltVSurfaceUsingDD(hDestVSurface, hSrcVSurface, fBltFlags, iDestX, iDestY, &srcRect));

  } else if (hDestVSurface->ubBitDepth == 8 && hSrcVSurface->ubBitDepth == 8) {
    if ((pSrcSurface8 = (uint8_t *)LockVSurface(hSrcVSurface, &uiSrcPitch)) == NULL) {
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
                 String("Failed on lock of 8BPP surface for blitting"));
      return (FALSE);
    }

    if ((pDestSurface8 = (uint8_t *)LockVSurface(hDestVSurface, &uiDestPitch)) == NULL) {
      UnlockVSurface(hSrcVSurface);
      DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
                 String("Failed on lock of 8BPP dest surface for blitting"));
      return (FALSE);
    }

    Blt8BPPTo8BPP(pDestSurface8, uiDestPitch, pSrcSurface8, uiSrcPitch, iDestX, iDestY,
                  SrcRect.left, SrcRect.top, uiWidth, uiHeight);
    UnlockVSurface(hSrcVSurface);
    UnlockVSurface(hDestVSurface);
    return (TRUE);
  } else {
    DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_2,
               String("Incompatible BPP values with src and dest Video Surfaces for blitting"));
    return (FALSE);
  }

  return (TRUE);
}

// *****************************************************************************
//
// Private DirectDraw manipulation functions
//
// *****************************************************************************

static struct VSurface *CreateVideoSurfaceFromDDSurface(LPDIRECTDRAWSURFACE2 lpDDSurface) {
  // Create Video Surface
  DDPIXELFORMAT PixelFormat;
  struct VSurface *hVSurface;
  DDSURFACEDESC DDSurfaceDesc;
  LPDIRECTDRAWPALETTE pDDPalette;
  struct SGPPaletteEntry SGPPalette[256];
  HRESULT ReturnCode;

  // Allocate Video Surface struct
  hVSurface = (struct VSurface *)MemAlloc(sizeof(struct VSurface));

  // Set values based on DD Surface given
  DDGetSurfaceDescription(lpDDSurface, &DDSurfaceDesc);
  PixelFormat = DDSurfaceDesc.ddpfPixelFormat;

  hVSurface->usHeight = (uint16_t)DDSurfaceDesc.dwHeight;
  hVSurface->usWidth = (uint16_t)DDSurfaceDesc.dwWidth;
  hVSurface->ubBitDepth = (uint8_t)PixelFormat.dwRGBBitCount;
  hVSurface->_platformData2 = (void *)lpDDSurface;
  hVSurface->_platformData1 = NULL;

  // Get and Set palette, if attached, allow to fail
  ReturnCode = IDirectDrawSurface2_GetPalette(lpDDSurface, &pDDPalette);

  if (ReturnCode == DD_OK) {
    // Set 8-bit Palette and 16 BPP palette
    hVSurface->pPalette = pDDPalette;

    // Create 16-BPP Palette
    DDGetPaletteEntries(pDDPalette, 0, 0, 256, (LPPALETTEENTRY)SGPPalette);
    hVSurface->p16BPPPalette = Create16BPPPalette(SGPPalette);
  } else {
    hVSurface->pPalette = NULL;
    hVSurface->p16BPPPalette = NULL;
  }

  // All is well
  DbgMessage(TOPIC_VIDEOSURFACE, DBG_LEVEL_0,
             String("Success in Creating Video Surface from DD Surface"));

  return (hVSurface);
}

// UTILITY FUNCTIONS FOR BLITTING

static BOOLEAN ClipReleatedSrcAndDestRectangles(struct VSurface *hDestVSurface,
                                                struct VSurface *hSrcVSurface, RECT *DestRect,
                                                RECT *SrcRect) {
  Assert(hDestVSurface != NULL);
  Assert(hSrcVSurface != NULL);

  // Check for invalid start positions and clip by ignoring blit
  if (DestRect->left >= hDestVSurface->usWidth || DestRect->top >= hDestVSurface->usHeight) {
    return (FALSE);
  }

  if (SrcRect->left >= hSrcVSurface->usWidth || SrcRect->top >= hSrcVSurface->usHeight) {
    return (FALSE);
  }

  // For overruns
  // Clip destination rectangles
  if (DestRect->right > hDestVSurface->usWidth) {
    // Both have to be modified or by default streching occurs
    DestRect->right = hDestVSurface->usWidth;
    SrcRect->right = SrcRect->left + (DestRect->right - DestRect->left);
  }
  if (DestRect->bottom > hDestVSurface->usHeight) {
    // Both have to be modified or by default streching occurs
    DestRect->bottom = hDestVSurface->usHeight;
    SrcRect->bottom = SrcRect->top + (DestRect->bottom - DestRect->top);
  }

  // Clip src rectangles
  if (SrcRect->right > hSrcVSurface->usWidth) {
    // Both have to be modified or by default streching occurs
    SrcRect->right = hSrcVSurface->usWidth;
    DestRect->right = DestRect->left + (SrcRect->right - SrcRect->left);
  }
  if (SrcRect->bottom > hSrcVSurface->usHeight) {
    // Both have to be modified or by default streching occurs
    SrcRect->bottom = hSrcVSurface->usHeight;
    DestRect->bottom = DestRect->top + (SrcRect->bottom - SrcRect->top);
  }

  // For underruns
  // Clip destination rectangles
  if (DestRect->left < 0) {
    // Both have to be modified or by default streching occurs
    DestRect->left = 0;
    SrcRect->left = SrcRect->right - (DestRect->right - DestRect->left);
  }
  if (DestRect->top < 0) {
    // Both have to be modified or by default streching occurs
    DestRect->top = 0;
    SrcRect->top = SrcRect->bottom - (DestRect->bottom - DestRect->top);
  }

  // Clip src rectangles
  if (SrcRect->left < 0) {
    // Both have to be modified or by default streching occurs
    SrcRect->left = 0;
    DestRect->left = DestRect->right - (SrcRect->right - SrcRect->left);
  }
  if (SrcRect->top < 0) {
    // Both have to be modified or by default streching occurs
    SrcRect->top = 0;
    DestRect->top = DestRect->bottom - (SrcRect->bottom - SrcRect->top);
  }

  return (TRUE);
}

BOOLEAN BltVSurfaceUsingDD(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                           uint32_t fBltFlags, int32_t iDestX, int32_t iDestY,
                           struct Rect *SrcRect) {
  uint32_t uiDDFlags;
  RECT DestRect;

  RECT srcRect = {SrcRect->left, SrcRect->top, SrcRect->right, SrcRect->bottom};

  // Blit using the correct blitter
  if (fBltFlags & VS_BLT_FAST) {
    // Validations
    CHECKF(iDestX >= 0);
    CHECKF(iDestY >= 0);

    // Default flags
    uiDDFlags = 0;

    // Convert flags into DD flags, ( for transparency use, etc )
    if (fBltFlags & VS_BLT_USECOLORKEY) {
      uiDDFlags |= DDBLTFAST_SRCCOLORKEY;
    }

    if (uiDDFlags == 0) {
      // Default here is no colorkey
      uiDDFlags = DDBLTFAST_NOCOLORKEY;
    }

    DDBltFastSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->_platformData2, iDestX, iDestY,
                     (LPDIRECTDRAWSURFACE2)hSrcVSurface->_platformData2, &srcRect, uiDDFlags);
  } else {
    // Normal, specialized blit for clipping, etc

    // Default flags
    uiDDFlags = DDBLT_WAIT;

    // Convert flags into DD flags, ( for transparency use, etc )
    if (fBltFlags & VS_BLT_USECOLORKEY) {
      uiDDFlags |= DDBLT_KEYSRC;
    }

    // Setup dest rectangle
    DestRect.top = (int)iDestY;
    DestRect.left = (int)iDestX;
    DestRect.bottom = (int)iDestY + (SrcRect->bottom - SrcRect->top);
    DestRect.right = (int)iDestX + (SrcRect->right - SrcRect->left);

    // Do Clipping of rectangles
    if (!ClipReleatedSrcAndDestRectangles(hDestVSurface, hSrcVSurface, &DestRect, &srcRect)) {
      // Returns false because dest start is > dest size
      return (TRUE);
    }

    // Check values for 0 size
    if (DestRect.top == DestRect.bottom || DestRect.right == DestRect.left) {
      return (TRUE);
    }

    // Check for -ve values

    DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->_platformData2, &DestRect,
                 (LPDIRECTDRAWSURFACE2)hSrcVSurface->_platformData2, &srcRect, uiDDFlags, NULL);
  }

  return (TRUE);
}

BOOLEAN BltVSurface(struct VSurface *hDestVSurface, struct VSurface *hSrcVSurface,
                    uint32_t fBltFlags, int32_t iDestX, int32_t iDestY, struct Rect *SrcRect,
                    struct Rect *DestRect) {
  uint32_t uiDDFlags;
  RECT srcRect = {SrcRect->left, SrcRect->top, SrcRect->right, SrcRect->bottom};
  RECT destRect = {DestRect->left, DestRect->top, DestRect->right, DestRect->bottom};

  // Default flags
  uiDDFlags = DDBLT_WAIT;

  // Convert flags into DD flags, ( for transparency use, etc )
  if (fBltFlags & VS_BLT_USECOLORKEY) {
    uiDDFlags |= DDBLT_KEYSRC;
  }

  DDBltSurface((LPDIRECTDRAWSURFACE2)hDestVSurface->_platformData2, &destRect,
               (LPDIRECTDRAWSURFACE2)hSrcVSurface->_platformData2, &srcRect, uiDDFlags, NULL);

  return (TRUE);
}

#ifdef _DEBUG
void CheckValidVSurfaceIndex(uint32_t uiIndex) {
  BOOLEAN fAssertError = FALSE;
  if (uiIndex == 0xffffffff) {  //-1 index -- deleted
    fAssertError = TRUE;
  } else if (uiIndex % 2 && uiIndex < 0xfffffff0) {  // odd numbers are reserved for vobjects
    fAssertError = TRUE;
  }

  if (fAssertError) {
    char str[60];
    switch (gubVSDebugCode) {
      case DEBUGSTR_SETVIDEOSURFACETRANSPARENCY:
        sprintf(str, "SetVideoSurfaceTransparency");
        break;
      case DEBUGSTR_ADDVIDEOSURFACEREGION:
        sprintf(str, "AddVideoSurfaceRegion");
        break;
      case DEBUGSTR_GETVIDEOSURFACEDESCRIPTION:
        sprintf(str, "GetVideoSurfaceDescription");
        break;
      case DEBUGSTR_BLTVIDEOSURFACE_DST:
        sprintf(str, "BltVideoSurface (dest)");
        break;
      case DEBUGSTR_BLTVIDEOSURFACE_SRC:
        sprintf(str, "BltVideoSurface (src)");
        break;
      case DEBUGSTR_COLORFILLVIDEOSURFACEAREA:
        sprintf(str, "ColorFillVideoSurfaceArea");
        break;
      case DEBUGSTR_SHADOWVIDEOSURFACERECT:
        sprintf(str, "ShadowVideoSurfaceRect");
        break;
      case DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST:
        sprintf(str, "BltStretchVideoSurface (dest)");
        break;
      case DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC:
        sprintf(str, "BltStretchVideoSurface (src)");
        break;
      case DEBUGSTR_DeleteVSurfaceByIndex:
        sprintf(str, "DeleteVSurfaceByIndex");
        break;
      case DEBUGSTR_NONE:
      default:
        sprintf(str, "GetVSurfaceByIndexOld");
        break;
    }
    if (uiIndex == 0xffffffff) {
      AssertMsg(0, String("Trying to %s with deleted index -1.", str));
    } else {
      AssertMsg(0, String("Trying to %s using a VOBJECT ID %d!", str, uiIndex));
    }
  }
}
#endif

//////////////////////////////////////////////////////////////////
// Cinematics
//////////////////////////////////////////////////////////////////

#include <crtdbg.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "SGP/Debug.h"
#include "SGP/FileMan.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Smack.h"
#include "Utils/Cinematics.h"
#include "Utils/radmalw.i"
#include "platform_win.h"

#if 1
// must go after other includes
#include <ddraw.h>
#endif

#define SMK_NUM_FLICS 4  // Maximum number of flics open

#define SMK_FLIC_OPEN 0x00000001       // Flic is open
#define SMK_FLIC_PLAYING 0x00000002    // Flic is playing
#define SMK_FLIC_LOOP 0x00000004       // Play flic in a loop
#define SMK_FLIC_AUTOCLOSE 0x00000008  // Close when done

struct SmkFlic {
  char *cFilename;
  HWFILE hFileHandle;
  struct SmackTag *SmackHandle;
  struct SmackBufTag *SmackBuffer;
  uint32_t uiFlags;
  LPDIRECTDRAWSURFACE2 lpDDS;
  HWND hWindow;
  uint32_t uiFrame;
  uint32_t uiLeft, uiTop;
};

struct SmkFlic SmkList[SMK_NUM_FLICS];

HWND hDisplayWindow = 0;
uint32_t uiDisplayHeight, uiDisplayWidth;
BOOLEAN fSuspendFlics = FALSE;
uint32_t uiFlicsPlaying = 0;
uint32_t guiSmackPixelFormat = SMACKBUFFER565;

LPDIRECTDRAWSURFACE lpVideoPlayback = NULL;
LPDIRECTDRAWSURFACE2 lpVideoPlayback2 = NULL;

//-Function-Prototypes-------------------------------------------------------------
void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight);
void SmkShutdown(void);
struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fAutoClose);
BOOLEAN SmkPollFlics(void);
struct SmkFlic *SmkOpenFlic(char *cFilename);
void SmkSetBlitPosition(struct SmkFlic *pSmack, uint32_t uiLeft, uint32_t uiTop);
void SmkCloseFlic(struct SmkFlic *pSmack);
struct SmkFlic *SmkGetFreeFlic(void);
void SmkSetupVideo(void);
void SmkShutdownVideo(void);

BOOLEAN SmkPollFlics(void) {
  uint32_t uiCount;
  BOOLEAN fFlicStatus = FALSE;
  DDSURFACEDESC SurfaceDescription;

  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++) {
    if (SmkList[uiCount].uiFlags & SMK_FLIC_PLAYING) {
      fFlicStatus = TRUE;
      if (!fSuspendFlics) {
        if (!SmackWait(SmkList[uiCount].SmackHandle)) {
          DDLockSurface(SmkList[uiCount].lpDDS, NULL, &SurfaceDescription, 0, NULL);
          SmackToBuffer(SmkList[uiCount].SmackHandle, SmkList[uiCount].uiLeft,
                        SmkList[uiCount].uiTop, SurfaceDescription.lPitch,
                        SmkList[uiCount].SmackHandle->Height, SurfaceDescription.lpSurface,
                        guiSmackPixelFormat);
          SmackDoFrame(SmkList[uiCount].SmackHandle);
          IDirectDrawSurface2_Unlock(SmkList[uiCount].lpDDS, SurfaceDescription.lpSurface);
          // temp til I figure out what to do with it
          // InvalidateRegion(0,0, 640, 480, FALSE);

          // Check to see if the flic is done the last frame
          if (SmkList[uiCount].SmackHandle->FrameNum ==
              (SmkList[uiCount].SmackHandle->Frames - 1)) {
            // If flic is looping, reset frame to 0
            if (SmkList[uiCount].uiFlags & SMK_FLIC_LOOP)
              SmackGoto(SmkList[uiCount].SmackHandle, 0);
            else if (SmkList[uiCount].uiFlags & SMK_FLIC_AUTOCLOSE)
              SmkCloseFlic(&SmkList[uiCount]);
          } else
            SmackNextFrame(SmkList[uiCount].SmackHandle);
        }
      }
    }
  }
  if (!fFlicStatus) SmkShutdownVideo();

  return (fFlicStatus);
}

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight) {
  void *pSoundDriver = NULL;

  // Wipe the flic list clean
  memset(SmkList, 0, sizeof(struct SmkFlic) * SMK_NUM_FLICS);

  // Set playback window properties
  hDisplayWindow = ghWindow;
  uiDisplayWidth = uiWidth;
  uiDisplayHeight = uiHeight;

  // Use MMX acceleration, if available
  SmackUseMMX(1);

  // Get the sound Driver handle
  pSoundDriver = SoundGetDriverHandle();

  // if we got the sound handle, use sound during the intro
  if (pSoundDriver) SmackSoundUseMSS(pSoundDriver);
}

void SmkShutdown(void) {
  uint32_t uiCount;

  // Close and deallocate any open flics
  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++) {
    if (SmkList[uiCount].uiFlags & SMK_FLIC_OPEN) SmkCloseFlic(&SmkList[uiCount]);
  }
}

struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fClose) {
  struct SmkFlic *pSmack;

  // Open the flic
  if ((pSmack = SmkOpenFlic(cFilename)) == NULL) return (NULL);

  // Set the blitting position on the screen
  SmkSetBlitPosition(pSmack, uiLeft, uiTop);

  // We're now playing, flag the flic for the poller to update
  pSmack->uiFlags |= SMK_FLIC_PLAYING;
  if (fClose) pSmack->uiFlags |= SMK_FLIC_AUTOCLOSE;

  return (pSmack);
}

struct SmkFlic *SmkOpenFlic(char *cFilename) {
  struct SmkFlic *pSmack;
  HANDLE hFile;

  // Get an available flic slot from the list
  if (!(pSmack = SmkGetFreeFlic())) {
    ErrorMsg("SMK ERROR: Out of flic slots, cannot open another");
    return (NULL);
  }

  // Attempt opening the filename
  if (!(pSmack->hFileHandle =
            FileMan_Open(cFilename, FILE_OPEN_EXISTING | FILE_ACCESS_READ, FALSE))) {
    ErrorMsg("SMK ERROR: Can't open the SMK file");
    return (NULL);
  }

  // Get the real file handle for the file man handle for the smacker file
  hFile = GetRealFileHandleFromFileManFileHandle(pSmack->hFileHandle);

  // Allocate a Smacker buffer for video decompression
  if (!(pSmack->SmackBuffer = SmackBufferOpen(hDisplayWindow, SMACKAUTOBLIT, 640, 480, 0, 0))) {
    ErrorMsg("SMK ERROR: Can't allocate a Smacker decompression buffer");
    return (NULL);
  }

  if (!(pSmack->SmackHandle =
            SmackOpen((char *)hFile, SMACKFILEHANDLE | SMACKTRACKS, SMACKAUTOEXTRA)))
  //	if(!(pSmack->SmackHandle=SmackOpen(cFilename, SMACKTRACKS, SMACKAUTOEXTRA)))
  {
    ErrorMsg("SMK ERROR: Smacker won't open the SMK file");
    return (NULL);
  }

  // Make sure we have a video surface
  SmkSetupVideo();

  pSmack->cFilename = cFilename;
  pSmack->lpDDS = lpVideoPlayback2;
  pSmack->hWindow = hDisplayWindow;

  // Smack flic is now open and ready to go
  pSmack->uiFlags |= SMK_FLIC_OPEN;

  return (pSmack);
}

void SmkSetBlitPosition(struct SmkFlic *pSmack, uint32_t uiLeft, uint32_t uiTop) {
  pSmack->uiLeft = uiLeft;
  pSmack->uiTop = uiTop;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {
  // Attempt opening the filename
  FileMan_Close(pSmack->hFileHandle);

  // Deallocate the smack buffers
  SmackBufferClose(pSmack->SmackBuffer);

  // Close the smack flic
  SmackClose(pSmack->SmackHandle);

  // Zero the memory, flags, etc.
  memset(pSmack, 0, sizeof(struct SmkFlic));
}

struct SmkFlic *SmkGetFreeFlic(void) {
  uint32_t uiCount;

  for (uiCount = 0; uiCount < SMK_NUM_FLICS; uiCount++)
    if (!(SmkList[uiCount].uiFlags & SMK_FLIC_OPEN)) return (&SmkList[uiCount]);

  return (NULL);
}

void SmkSetupVideo(void) {
  DDSURFACEDESC SurfaceDescription;
  HRESULT ReturnCode;
  uint16_t usRed, usGreen, usBlue;
  struct VSurface *hVSurface;

  GetVSurfaceByIndexOld(&hVSurface, vsIndexFB);
  lpVideoPlayback2 = (LPDIRECTDRAWSURFACE2)hVSurface->_platformData2;

  memset(&SurfaceDescription, 0, sizeof(SurfaceDescription));
  SurfaceDescription.dwSize = sizeof(DDSURFACEDESC);
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc(lpVideoPlayback2, &SurfaceDescription);
  if (ReturnCode != DD_OK) {
    DirectXAttempt(ReturnCode, __LINE__, __FILE__);
    return;
  }

  usRed = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwRBitMask;
  usGreen = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwGBitMask;
  usBlue = (uint16_t)SurfaceDescription.ddpfPixelFormat.dwBBitMask;

  if ((usRed == 0xf800) && (usGreen == 0x07e0) && (usBlue == 0x001f))
    guiSmackPixelFormat = SMACKBUFFER565;
  else
    guiSmackPixelFormat = SMACKBUFFER555;
}

void SmkShutdownVideo(void) {
  // DEF:
  //	CinematicModeOff();
}

//////////////////////////////////////////////////////////////////
// DirectDrawCalls
//////////////////////////////////////////////////////////////////

// DirectDrawSurface2 Calls
static void DDCreateSurface(LPDIRECTDRAW2 pExistingDirectDraw, DDSURFACEDESC *pNewSurfaceDesc,
                            LPDIRECTDRAWSURFACE *ppNewSurface1,
                            LPDIRECTDRAWSURFACE2 *ppNewSurface2) {
  Assert(pExistingDirectDraw != NULL);
  Assert(pNewSurfaceDesc != NULL);
  Assert(ppNewSurface1 != NULL);
  Assert(ppNewSurface2 != NULL);

  // create the directdraw surface
  ATTEMPT(IDirectDraw2_CreateSurface(pExistingDirectDraw, pNewSurfaceDesc, ppNewSurface1, NULL));

  // get the direct draw surface 2 interface
  IID tmpID = IID_IDirectDrawSurface2;
  ATTEMPT(IDirectDrawSurface_QueryInterface(*ppNewSurface1, &tmpID, (LPVOID *)ppNewSurface2));
}

// Lock, unlock calls
static void DDLockSurface(LPDIRECTDRAWSURFACE2 pSurface, LPRECT pDestRect,
                          LPDDSURFACEDESC pSurfaceDesc, uint32_t uiFlags, HANDLE hEvent) {
  HRESULT ReturnCode;

  Assert(pSurface != NULL);
  Assert(pSurfaceDesc != NULL);

  memset(pSurfaceDesc, 0, sizeof(LPDDSURFACEDESC));
  pSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);

  do {
    ReturnCode = IDirectDrawSurface2_Lock(pSurface, pDestRect, pSurfaceDesc, uiFlags, hEvent);

  } while (ReturnCode == DDERR_WASSTILLDRAWING);

  ATTEMPT(ReturnCode);
}

static void DDGetSurfaceDescription(LPDIRECTDRAWSURFACE2 pSurface, DDSURFACEDESC *pSurfaceDesc) {
  Assert(pSurface != NULL);
  Assert(pSurfaceDesc != NULL);

  memset(pSurfaceDesc, 0, sizeof(LPDDSURFACEDESC));
  pSurfaceDesc->dwSize = sizeof(DDSURFACEDESC);

  ATTEMPT(IDirectDrawSurface2_GetSurfaceDesc(pSurface, pSurfaceDesc));
}

static void DDReleaseSurface(LPDIRECTDRAWSURFACE *ppOldSurface1,
                             LPDIRECTDRAWSURFACE2 *ppOldSurface2) {
  Assert(ppOldSurface1 != NULL);
  Assert(ppOldSurface2 != NULL);
  Assert(*ppOldSurface1 != NULL);
  Assert(*ppOldSurface2 != NULL);

  ATTEMPT(IDirectDrawSurface2_Release(*ppOldSurface2));
  ATTEMPT(IDirectDrawSurface_Release(*ppOldSurface1));

  *ppOldSurface1 = NULL;
  *ppOldSurface2 = NULL;
}

static void DDBltFastSurface(LPDIRECTDRAWSURFACE2 pDestSurface, uint32_t uiX, uint32_t uiY,
                             LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, uint32_t uiTrans) {
  HRESULT ReturnCode;

  Assert(pDestSurface != NULL);
  Assert(pSrcSurface != NULL);

  do {
    ReturnCode =
        IDirectDrawSurface2_BltFast(pDestSurface, uiX, uiY, pSrcSurface, pSrcRect, uiTrans);

  } while (ReturnCode == DDERR_WASSTILLDRAWING);
}

static void DDBltSurface(LPDIRECTDRAWSURFACE2 pDestSurface, LPRECT pDestRect,
                         LPDIRECTDRAWSURFACE2 pSrcSurface, LPRECT pSrcRect, uint32_t uiFlags,
                         LPDDBLTFX pDDBltFx) {
  HRESULT ReturnCode;

  Assert(pDestSurface != NULL);

  do {
    ReturnCode =
        IDirectDrawSurface2_Blt(pDestSurface, pDestRect, pSrcSurface, pSrcRect, uiFlags, pDDBltFx);

  } while (ReturnCode == DDERR_WASSTILLDRAWING);

  ATTEMPT(ReturnCode);
}

static void DDCreatePalette(LPDIRECTDRAW2 pDirectDraw, uint32_t uiFlags, LPPALETTEENTRY pColorTable,
                            LPDIRECTDRAWPALETTE FAR *ppDDPalette, IUnknown FAR *pUnkOuter) {
  Assert(pDirectDraw != NULL);

  ATTEMPT(IDirectDraw2_CreatePalette(pDirectDraw, uiFlags, pColorTable, ppDDPalette, pUnkOuter));
}

static void DDSetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags,
                                uint32_t uiStartingEntry, uint32_t uiCount,
                                LPPALETTEENTRY pEntries) {
  Assert(pPalette != NULL);
  Assert(pEntries != NULL);

  ATTEMPT(IDirectDrawPalette_SetEntries(pPalette, uiFlags, uiStartingEntry, uiCount, pEntries));
}

static void DDGetPaletteEntries(LPDIRECTDRAWPALETTE pPalette, uint32_t uiFlags, uint32_t uiBase,
                                uint32_t uiNumEntries, LPPALETTEENTRY pEntries) {
  Assert(pPalette != NULL);
  Assert(pEntries != NULL);

  ATTEMPT(IDirectDrawPalette_GetEntries(pPalette, uiFlags, uiBase, uiNumEntries, pEntries));
}

static void DDReleasePalette(LPDIRECTDRAWPALETTE pPalette) {
  Assert(pPalette != NULL);

  ATTEMPT(IDirectDrawPalette_Release(pPalette));
}

static void DDSetSurfaceColorKey(LPDIRECTDRAWSURFACE2 pSurface, uint32_t uiFlags,
                                 LPDDCOLORKEY pDDColorKey) {
  Assert(pSurface != NULL);
  Assert(pDDColorKey != NULL);

  ATTEMPT(IDirectDrawSurface2_SetColorKey(pSurface, uiFlags, pDDColorKey));
}

//////////////////////////////////////////////////////////////////
// DirectXCommon
//////////////////////////////////////////////////////////////////

static void DirectXAttempt(int32_t iErrorCode, int32_t nLine, char *szFilename) {
#ifdef _DEBUG
  if (iErrorCode != DD_OK) {
    FastDebugMsg("DIRECTX COMMON: DirectX Error\n");
    FastDebugMsg(DirectXErrorDescription(iErrorCode));
  }
#endif
}

static char *DirectXErrorDescription(int32_t iDXReturn) {
  switch (iDXReturn) {
    case DD_OK:
      return "No error.\0";
    case DDERR_ALREADYINITIALIZED:
      return "The object has already been initialized.";
    case DDERR_BLTFASTCANTCLIP:
      return "A DirectDrawClipper object is attached to a source surface that has passed into a "
             "call to the IDirectDrawSurface2::BltFast method.";
    case DDERR_CANNOTATTACHSURFACE:
      return "A surface cannot be attached to another requested surface.";
    case DDERR_CANNOTDETACHSURFACE:
      return "A surface cannot be detached from another requested surface.";
    case DDERR_CANTCREATEDC:
      return "Windows cannot create any more device contexts (DCs).";
    case DDERR_CANTDUPLICATE:
      return "Primary and 3D surfaces, or surfaces that are implicitly created, cannot be "
             "duplicated.";
    case DDERR_CANTLOCKSURFACE:
      return "Access to this surface is refused because an attempt was made to lock the primary "
             "surface without DCI support.";
    case DDERR_CANTPAGELOCK:
      return "An attempt to page lock a surface failed. Page lock will not work on a "
             "display-memory surface or an emulated primary surface.";
    case DDERR_CANTPAGEUNLOCK:
      return "An attempt to page unlock a surface failed. Page unlock will not work on a "
             "display-memory surface or an emulated primary surface.";
    case DDERR_CLIPPERISUSINGHWND:
      return "An attempt was made to set a clip list for a DirectDrawClipper object that is "
             "already monitoring a window handle.";
    case DDERR_COLORKEYNOTSET:
      return "No source color key is specified for this operation.";
    case DDERR_CURRENTLYNOTAVAIL:
      return "No support is currently available.";
    case DDERR_DCALREADYCREATED:
      return "A device context (DC) has already been returned for this surface. Only one DC can be "
             "retrieved for each surface.";
    case DDERR_DIRECTDRAWALREADYCREATED:
      return "A DirectDraw object representing this driver has already been created for this "
             "process.";
    case DDERR_EXCEPTION:
      return "An exception was encountered while performing the requested operation.";
    case DDERR_EXCLUSIVEMODEALREADYSET:
      return "An attempt was made to set the cooperative level when it was already set to "
             "exclusive.";
    case DDERR_GENERIC:
      return "There is an undefined error condition.";
    case DDERR_HEIGHTALIGN:
      return "The height of the provided rectangle is not a multiple of the required alignment.";
    case DDERR_HWNDALREADYSET:
      return "The DirectDraw cooperative level window handle has already been set. It cannot be "
             "reset while the process has surfaces or palettes created.";
    case DDERR_HWNDSUBCLASSED:
      return "DirectDraw is prevented from restoring state because the DirectDraw cooperative "
             "level window handle has been subclassed.";
    case DDERR_IMPLICITLYCREATED:
      return "The surface cannot be restored because it is an implicitly created surface.";
    case DDERR_INCOMPATIBLEPRIMARY:
      return "The primary surface creation request does not match with the existing primary "
             "surface.";
    case DDERR_INVALIDCAPS:
      return "One or more of the capability bits passed to the callback function are incorrect.";
    case DDERR_INVALIDCLIPLIST:
      return "DirectDraw does not support the provided clip list.";
    case DDERR_INVALIDDIRECTDRAWGUID:
      return "The globally unique identifier (GUID) passed to the DirectDrawCreate function is not "
             "a valid DirectDraw driver identifier.";
    case DDERR_INVALIDMODE:
      return "DirectDraw does not support the requested mode.";
    case DDERR_INVALIDOBJECT:
      return "DirectDraw received a pointer that was an invalid DirectDraw object.";
    case DDERR_INVALIDPARAMS:
      return "One or more of the parameters passed to the method are incorrect.";
    case DDERR_INVALIDPIXELFORMAT:
      return "The pixel format was invalid as specified.";
    case DDERR_INVALIDPOSITION:
      return "The position of the overlay on the destination is no longer legal.";
    case DDERR_INVALIDRECT:
      return "The provided rectangle was invalid.";
    case DDERR_INVALIDSURFACETYPE:
      return "The requested operation could not be performed because the surface was of the wrong "
             "type.";
    case DDERR_LOCKEDSURFACES:
      return "One or more surfaces are locked, causing the failure of the requested operation.";
    case DDERR_NO3D:
      return "No 3D hardware or emulation is present.";
    case DDERR_NOALPHAHW:
      return "No alpha acceleration hardware is present or available, causing the failure of the "
             "requested operation.";
    case DDERR_NOBLTHW:
      return "No blitter hardware is present.";
    case DDERR_NOCLIPLIST:
      return "No clip list is available.";
    case DDERR_NOCLIPPERATTACHED:
      return "No DirectDrawClipper object is attached to the surface object.";
    case DDERR_NOCOLORCONVHW:
      return "The operation cannot be carried out because no color-conversion hardware is present "
             "or available.";
    case DDERR_NOCOLORKEY:
      return "The surface does not currently have a color key.";
    case DDERR_NOCOLORKEYHW:
      return "The operation cannot be carried out because there is no hardware support for the "
             "destination color key.";
    case DDERR_NOCOOPERATIVELEVELSET:
      return "A create function is called without the IDirectDraw2::SetCooperativeLevel method "
             "being called.";
    case DDERR_NODC:
      return "No DC has ever been created for this surface.";
    case DDERR_NODDROPSHW:
      return "No DirectDraw raster operation (ROP) hardware is available.";
    case DDERR_NODIRECTDRAWHW:
      return "Hardware-only DirectDraw object creation is not possible; the driver does not "
             "support any hardware.";
    case DDERR_NODIRECTDRAWSUPPORT:
      return "DirectDraw support is not possible with the current display driver.";
    case DDERR_NOEMULATION:
      return "Software emulation is not available.";
    case DDERR_NOEXCLUSIVEMODE:
      return "The operation requires the application to have exclusive mode, but the application "
             "does not have exclusive mode.";
    case DDERR_NOFLIPHW:
      return "Flipping visible surfaces is not supported.";
    case DDERR_NOGDI:
      return "No GDI is present.";
    case DDERR_NOHWND:
      return "Clipper notification requires a window handle, or no window handle has been "
             "previously set as the cooperative level window handle.";
    case DDERR_NOMIPMAPHW:
      return "The operation cannot be carried out because no mipmap texture mapping hardware is "
             "present or available.";
    case DDERR_NOMIRRORHW:
      return "The operation cannot be carried out because no mirroring hardware is present or "
             "available.";
    case DDERR_NOOVERLAYDEST:
      return "The IDirectDrawSurface2::GetOverlayPosition method is called on an overlay that the "
             "IDirectDrawSurface2::UpdateOverlay method has not been called on to establish a "
             "destination.";
    case DDERR_NOOVERLAYHW:
      return "The operation cannot be carried out because no overlay hardware is present or "
             "available.";
    case DDERR_NOPALETTEATTACHED:
      return "No palette object is attached to this surface.";
    case DDERR_NOPALETTEHW:
      return "There is no hardware support for 16- or 256-color palettes.";
    case DDERR_NORASTEROPHW:
      return "The operation cannot be carried out because no appropriate raster operation hardware "
             "is present or available.";
    case DDERR_NOROTATIONHW:
      return "The operation cannot be carried out because no rotation hardware is present or "
             "available.";
    case DDERR_NOSTRETCHHW:
      return "The operation cannot be carried out because there is no hardware support for "
             "stretching.";
    case DDERR_NOT4BITCOLOR:
      return "The DirectDrawSurface object is not using a 4-bit color palette and the requested "
             "operation requires a 4-bit color palette.";
    case DDERR_NOT4BITCOLORINDEX:
      return "The DirectDrawSurface object is not using a 4-bit color index palette and the "
             "requested operation requires a 4-bit color index palette.";
    case DDERR_NOT8BITCOLOR:
      return "The DirectDrawSurface object is not using an 8-bit color palette and the requested "
             "operation requires an 8-bit color palette.";
    case DDERR_NOTAOVERLAYSURFACE:
      return "An overlay component is called for a non-overlay surface.";
    case DDERR_NOTEXTUREHW:
      return "The operation cannot be carried out because no texture-mapping hardware is present "
             "or available.";
    case DDERR_NOTFLIPPABLE:
      return "An attempt has been made to flip a surface that cannot be flipped.";
    case DDERR_NOTFOUND:
      return "The requested item was not found.";
    case DDERR_NOTINITIALIZED:
      return "An attempt was made to call an interface method of a DirectDraw object created by "
             "CoCreateInstance before the object was initialized.";
    case DDERR_NOTLOCKED:
      return "An attempt is made to unlock a surface that was not locked.";
    case DDERR_NOTPAGELOCKED:
      return "An attempt is made to page unlock a surface with no outstanding page locks.";
    case DDERR_NOTPALETTIZED:
      return "The surface being used is not a palette-based surface.";
    case DDERR_NOVSYNCHW:
      return "The operation cannot be carried out because there is no hardware support for "
             "vertical blank synchronized operations.";
    case DDERR_NOZBUFFERHW:
      return "The operation to create a z-buffer in display memory or to perform a blit using a "
             "z-buffer cannot be carried out because there is no hardware support for z-buffers.";
    case DDERR_NOZOVERLAYHW:
      return "The overlay surfaces cannot be z-layered based on the z-order because the hardware "
             "does not support z-ordering of overlays.";
    case DDERR_OUTOFCAPS:
      return "The hardware needed for the requested operation has already been allocated.";
    case DDERR_OUTOFMEMORY:
      return "DirectDraw does not have enough memory to perform the operation.";
    case DDERR_OUTOFVIDEOMEMORY:
      return "DirectDraw does not have enough display memory to perform the operation.";
    case DDERR_OVERLAYCANTCLIP:
      return "The hardware does not support clipped overlays.";
    case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
      return "An attempt was made to have more than one color key active on an overlay.";
    case DDERR_OVERLAYNOTVISIBLE:
      return "The IDirectDrawSurface2::GetOverlayPosition method is called on a hidden overlay.";
    case DDERR_PALETTEBUSY:
      return "Access to this palette is refused because the palette is locked by another thread.";
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
      return "This process has already created a primary surface.";
    case DDERR_REGIONTOOSMALL:
      return "The region passed to the IDirectDrawClipper::GetClipList method is too small.";
    case DDERR_SURFACEALREADYATTACHED:
      return "An attempt was made to attach a surface to another surface to which it is already "
             "attached.";
    case DDERR_SURFACEALREADYDEPENDENT:
      return "An attempt was made to make a surface a dependency of another surface to which it is "
             "already dependent.";
    case DDERR_SURFACEBUSY:
      return "Access to the surface is refused because the surface is locked by another thread.";
    case DDERR_SURFACEISOBSCURED:
      return "Access to the surface is refused because the surface is obscured.";
    case DDERR_SURFACELOST:
      return "Access to the surface is refused because the surface memory is gone. The "
             "DirectDrawSurface object representing this surface should have the "
             "IDirectDrawSurface2::Restore method called on it.";
    case DDERR_SURFACENOTATTACHED:
      return "The requested surface is not attached.";
    case DDERR_TOOBIGHEIGHT:
      return "The height requested by DirectDraw is too large.";
    case DDERR_TOOBIGSIZE:
      return "The size requested by DirectDraw is too large. However, the individual height and "
             "width are OK.";
    case DDERR_TOOBIGWIDTH:
      return "The width requested by DirectDraw is too large.";
    case DDERR_UNSUPPORTED:
      return "The operation is not supported.";
    case DDERR_UNSUPPORTEDFORMAT:
      return "The FourCC format requested is not supported by DirectDraw.";
    case DDERR_UNSUPPORTEDMASK:
      return "The bitmask in the pixel format requested is not supported by DirectDraw.";
    case DDERR_UNSUPPORTEDMODE:
      return "The display is currently in an unsupported mode.";
    case DDERR_VERTICALBLANKINPROGRESS:
      return "A vertical blank is in progress.";
    case DDERR_WASSTILLDRAWING:
      return "The previous blit operation that is transferring information to or from this surface "
             "is incomplete.";
    case DDERR_WRONGMODE:
      return "This surface cannot be restored because it was created in a different mode.";
    case DDERR_XALIGN:
      return "The provided rectangle was not horizontally aligned on a required boundary.";
    default:
      return "Unrecognized error value.\0";
  }
}

//////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////

uint16_t PackColorsToRGB16(uint8_t r, uint8_t g, uint8_t b) {
  uint16_t r16, g16, b16;

  if (gusRedShift < 0)
    r16 = ((uint16_t)r >> abs(gusRedShift));
  else
    r16 = ((uint16_t)r << gusRedShift);

  if (gusGreenShift < 0)
    g16 = ((uint16_t)g >> abs(gusGreenShift));
  else
    g16 = ((uint16_t)g << gusGreenShift);

  if (gusBlueShift < 0)
    b16 = ((uint16_t)b >> abs(gusBlueShift));
  else
    b16 = ((uint16_t)b << gusBlueShift);

  return (r16 & gusRedMask) | (g16 & gusGreenMask) | (b16 & gusBlueMask);
}

void UnpackRGB16(uint16_t rgb16, uint8_t *r, uint8_t *g, uint8_t *b) {
  uint16_t r16 = rgb16 & gusRedMask;
  uint16_t g16 = rgb16 & gusGreenMask;
  uint16_t b16 = rgb16 & gusBlueMask;

  if (gusRedShift < 0)
    *r = ((uint32_t)r16 << abs(gusRedShift));
  else
    *r = ((uint32_t)r16 >> gusRedShift);

  if (gusGreenShift < 0)
    *g = ((uint32_t)g16 << abs(gusGreenShift));
  else
    *g = ((uint32_t)g16 >> gusGreenShift);

  if (gusBlueShift < 0)
    *b = ((uint32_t)b16 << abs(gusBlueShift));
  else
    *b = ((uint32_t)b16 >> gusBlueShift);
}

void GetRGB16Masks(uint16_t *red, uint16_t *green, uint16_t *blue) {
  *red = gusRedMask;
  *green = gusGreenMask;
  *blue = gusBlueMask;
}
