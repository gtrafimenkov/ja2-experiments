// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/Video.h"

#include <process.h>
#include <stdio.h>
#include <string.h>

#include "DebugLog.h"
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
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "Utils/TimerControl.h"
#include "jplatform_video.h"
#include "platform.h"

void GetCurrentVideoSettings(uint16_t *usWidth, uint16_t *usHeight) {
  *usWidth = JVideo_GetScreenWidth();
  *usHeight = JVideo_GetScreenHeight();
}

void EraseMouseCursor() { VSurfaceErase(vsMouseBufferOriginal); }

typedef struct {
  BOOLEAN fRestore;
  uint16_t usMouseXPos, usMouseYPos;
  uint16_t usLeft, usTop, usRight, usBottom;
  struct Rect Region;
  struct VSurface *vs;
} MouseCursorBackground;

static MouseCursorBackground gMouseCursorBackground[2];

static void _blitFast(struct VSurface *dest, struct VSurface *src, uint32_t destX, uint32_t destY,
                      struct Rect *srcRect) {
  struct JRect srcBox = r2jr(srcRect);
  JSurface_BlitRectToPoint(src, dest, &srcBox, destX, destY);
}

#define BUFFER_READY 0x00
#define BUFFER_BUSY 0x01
#define BUFFER_DIRTY 0x02
#define BUFFER_DISABLED 0x03

#define VIDEO_NO_CURSOR 0xFFFF

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
// Globals for mouse cursor
//

static uint16_t gusMouseCursorWidth;
static uint16_t gusMouseCursorHeight;
static int16_t gsMouseCursorXOffset;
static int16_t gsMouseCursorYOffset;

static struct VObject *gpCursorStore;

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

static void AddRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags);
static void SnapshotSmall(void);
static void VideoMovieCapture(BOOLEAN fEnable);
static void RefreshMovieCache();

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAX_CURSOR_WIDTH 64
#define MAX_CURSOR_HEIGHT 64

BOOLEAN InitializeVideoManager() {
  RegisterDebugTopic(TOPIC_VIDEO, "Video");
  DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Initializing the video manager");

  memset(gpFrameData, 0, sizeof(gpFrameData));

  // Initialize the frame buffer
  vsFB = JSurface_Create16bpp(SCREEN_WIDTH, SCREEN_HEIGHT);
  if (vsFB == NULL) {
    return FALSE;
  }

  // Initialize the main mouse surfaces
  vsMouseBuffer = JSurface_Create16bpp(MAX_CURSOR_WIDTH, MAX_CURSOR_HEIGHT);
  if (vsMouseBuffer == NULL) {
    return FALSE;
  }
  JSurface_SetColorKey(vsMouseBuffer, 0);

  // Initialize the main mouse original surface
  vsMouseBufferOriginal = JSurface_Create16bpp(MAX_CURSOR_WIDTH, MAX_CURSOR_HEIGHT);
  if (vsMouseBufferOriginal == NULL) {
    return FALSE;
  }

  // Initialize the main mouse background surfaces. There are two of them (one for each of the
  // Primary and Backbuffer surfaces
  for (uint32_t uiIndex = 0; uiIndex < 1; uiIndex++) {
    gMouseCursorBackground[uiIndex].fRestore = FALSE;
    gMouseCursorBackground[uiIndex].vs = JSurface_Create16bpp(MAX_CURSOR_WIDTH, MAX_CURSOR_HEIGHT);
    if (gMouseCursorBackground[uiIndex].vs == NULL) {
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

  return TRUE;
}

void ShutdownVideoManager(void) {
  // uint32_t  uiRefreshThreadState;

  DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Shutting down the video manager");

  //
  // Toggle the state of the video manager to indicate to the refresh thread that it needs to shut
  // itself down
  //

  JSurface_Free(vsMouseBufferOriginal);
  vsMouseBufferOriginal = NULL;
  JSurface_Free(vsMouseBuffer);
  vsMouseBuffer = NULL;
  JSurface_Free(vsBackBuffer);
  vsBackBuffer = NULL;
  JSurface_Free(vsPrimary);
  vsPrimary = NULL;
  JSurface_Free(vsFB);
  vsFB = NULL;

  JSurface_Free(gMouseCursorBackground[0].vs);
  gMouseCursorBackground[0].vs = NULL;

  JVideo_Shutdown();

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
  if (guiVideoManagerState == VIDEO_SUSPENDED) {
    JSurface_Restore(vsPrimary);
    JSurface_Restore(vsBackBuffer);
    JSurface_Restore(gMouseCursorBackground[0].vs);
    JSurface_Restore(vsMouseBuffer);

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

static void eraseZBuffer(int32_t x, int32_t y, int32_t width, int32_t height) {
  int32_t y_end = y + height;
  for (int32_t _y = y; _y < y_end; _y++) {
    memset((uint8_t *)gpZBuffer + (_y * 1280), 0, width * 2);
  }
}

static void ScrollJA2Background(uint32_t uiDirection) {
  struct Rect Region;
  uint16_t usNumStrips = 0;
  int32_t cnt;
  int32_t uiCountY;

  uint16_t screenWidth = JVideo_GetScreenWidth();
  uint16_t viewportWindowHeight = gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y;

  struct Rect StripRegions[2];
  StripRegions[0].left = gsVIEWPORT_START_X;
  StripRegions[0].right = gsVIEWPORT_END_X;
  StripRegions[0].top = gsVIEWPORT_WINDOW_START_Y;
  StripRegions[0].bottom = gsVIEWPORT_WINDOW_END_Y;
  StripRegions[1] = StripRegions[0];

  switch (uiDirection) {
    case SCROLL_LEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = screenWidth - (gsScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_END_Y;

      _blitFast(vsBackBuffer, vsPrimary, gsScrollXIncrement, gsVIEWPORT_WINDOW_START_Y, &Region);

      eraseZBuffer(0, gsVIEWPORT_WINDOW_START_Y, viewportWindowHeight, gsScrollXIncrement);

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + gsScrollXIncrement);

      usNumStrips = 1;
      break;

    case SCROLL_RIGHT:

      Region.left = gsScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = screenWidth;
      Region.bottom = gsVIEWPORT_WINDOW_END_Y;

      _blitFast(vsBackBuffer, vsPrimary, 0, gsVIEWPORT_WINDOW_START_Y, &Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280) +
                   ((gsVIEWPORT_END_X - gsScrollXIncrement) * 2),
               0, gsScrollXIncrement * 2);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - gsScrollXIncrement);

      usNumStrips = 1;
      break;

    case SCROLL_UP:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = screenWidth;
      Region.bottom = gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement;

      _blitFast(vsBackBuffer, vsPrimary, 0, gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement,
                &Region);

      for (uiCountY = gsScrollYIncrement - 1 + gsVIEWPORT_WINDOW_START_Y;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement);
      usNumStrips = 1;

      break;

    case SCROLL_DOWN:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement;
      Region.right = screenWidth;
      Region.bottom = gsVIEWPORT_WINDOW_END_Y;

      _blitFast(vsBackBuffer, vsPrimary, 0, gsVIEWPORT_WINDOW_START_Y, &Region);

      // Zero out z
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement);
      usNumStrips = 1;

      break;

    case SCROLL_UPLEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = screenWidth - (gsScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement;

      _blitFast(vsBackBuffer, vsPrimary, gsScrollXIncrement,
                gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement, &Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, gsScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + gsScrollXIncrement);
      StripRegions[1].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement);
      StripRegions[1].left = (int16_t)(gsVIEWPORT_START_X + gsScrollXIncrement);
      usNumStrips = 2;

      break;

    case SCROLL_UPRIGHT:

      Region.left = gsScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y;
      Region.right = screenWidth;
      Region.bottom = gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement;

      _blitFast(vsBackBuffer, vsPrimary, 0, gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement,
                &Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280) +
                   ((gsVIEWPORT_END_X - gsScrollXIncrement) * 2),
               0, gsScrollXIncrement * 2);
      }
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement - 1;
           uiCountY >= gsVIEWPORT_WINDOW_START_Y; uiCountY--) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - gsScrollXIncrement);
      StripRegions[1].bottom = (int16_t)(gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement);
      StripRegions[1].right = (int16_t)(gsVIEWPORT_END_X - gsScrollXIncrement);
      usNumStrips = 2;

      break;

    case SCROLL_DOWNLEFT:

      Region.left = 0;
      Region.top = gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement;
      Region.right = screenWidth - (gsScrollXIncrement);
      Region.bottom = gsVIEWPORT_WINDOW_END_Y;

      _blitFast(vsBackBuffer, vsPrimary, gsScrollXIncrement, gsVIEWPORT_WINDOW_START_Y, &Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, gsScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].right = (int16_t)(gsVIEWPORT_START_X + gsScrollXIncrement);

      StripRegions[1].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement);
      StripRegions[1].left = (int16_t)(gsVIEWPORT_START_X + gsScrollXIncrement);
      usNumStrips = 2;

      break;

    case SCROLL_DOWNRIGHT:

      Region.left = gsScrollXIncrement;
      Region.top = gsVIEWPORT_WINDOW_START_Y + gsScrollYIncrement;
      Region.right = screenWidth;
      Region.bottom = gsVIEWPORT_WINDOW_END_Y;

      _blitFast(vsBackBuffer, vsPrimary, 0, gsVIEWPORT_WINDOW_START_Y, &Region);

      // memset z-buffer
      for (uiCountY = gsVIEWPORT_WINDOW_START_Y; uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280) +
                   ((gsVIEWPORT_END_X - gsScrollXIncrement) * 2),
               0, gsScrollXIncrement * 2);
      }
      for (uiCountY = (gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement);
           uiCountY < gsVIEWPORT_WINDOW_END_Y; uiCountY++) {
        memset((uint8_t *)gpZBuffer + (uiCountY * 1280), 0, 1280);
      }

      StripRegions[0].left = (int16_t)(gsVIEWPORT_END_X - gsScrollXIncrement);
      StripRegions[1].top = (int16_t)(gsVIEWPORT_WINDOW_END_Y - gsScrollYIncrement);
      StripRegions[1].right = (int16_t)(gsVIEWPORT_END_X - gsScrollXIncrement);
      usNumStrips = 2;

      break;
  }

  for (cnt = 0; cnt < usNumStrips; cnt++) {
    RenderStaticWorldRect((int16_t)StripRegions[cnt].left, (int16_t)StripRegions[cnt].top,
                          (int16_t)StripRegions[cnt].right, (int16_t)StripRegions[cnt].bottom,
                          TRUE);
    _blitFast(vsBackBuffer, vsFB, StripRegions[cnt].left, StripRegions[cnt].top,
              &(StripRegions[cnt]));
  }

  switch (uiDirection) {
    case SCROLL_LEFT:
      RestoreShiftedVideoOverlays(gsScrollXIncrement, 0);
      break;

    case SCROLL_RIGHT:
      RestoreShiftedVideoOverlays(-gsScrollXIncrement, 0);
      break;

    case SCROLL_UP:
      RestoreShiftedVideoOverlays(0, gsScrollYIncrement);
      break;

    case SCROLL_DOWN:
      RestoreShiftedVideoOverlays(0, -gsScrollYIncrement);
      break;

    case SCROLL_UPLEFT:
      RestoreShiftedVideoOverlays(gsScrollXIncrement, gsScrollYIncrement);
      break;

    case SCROLL_UPRIGHT:
      RestoreShiftedVideoOverlays(-gsScrollXIncrement, gsScrollYIncrement);
      break;

    case SCROLL_DOWNLEFT:
      RestoreShiftedVideoOverlays(gsScrollXIncrement, -gsScrollYIncrement);
      break;

    case SCROLL_DOWNRIGHT:
      RestoreShiftedVideoOverlays(-gsScrollXIncrement, -gsScrollYIncrement);
      break;
  }

  SaveVideoOverlaysArea(vsBackBuffer);
  ExecuteVideoOverlaysToAlternateBuffer(vsBackBuffer);
}

void RefreshScreen(void *DummyVariable) {
  static uint32_t uiRefreshThreadState, uiIndex;
  uint16_t usScreenWidth, usScreenHeight;
  static BOOLEAN fShowMouse;
  static struct Rect Region;
  static BOOLEAN fFirstTime = TRUE;
  uint32_t uiTime;

  usScreenWidth = usScreenHeight = 0;

  if (fFirstTime) {
    fShowMouse = FALSE;
  }

  fFirstTime = FALSE;

  // DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Looping in refresh");

  switch (guiVideoManagerState) {
    case VIDEO_ON:  //
      // Excellent, everything is cosher, we continue on
      //
      uiRefreshThreadState = guiRefreshThreadState = THREAD_ON;
      usScreenWidth = JVideo_GetScreenWidth();
      usScreenHeight = JVideo_GetScreenHeight();
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

    _blitFast(vsBackBuffer, gMouseCursorBackground[CURRENT_MOUSE_DATA].vs,
              gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
              gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos, &Region);

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

        _blitFast(vsBackBuffer, vsFB, 0, 0, &Region);
      } else {
        for (uiIndex = 0; uiIndex < guiDirtyRegionCount; uiIndex++) {
          Region.left = gListOfDirtyRegions[uiIndex].iLeft;
          Region.top = gListOfDirtyRegions[uiIndex].iTop;
          Region.right = gListOfDirtyRegions[uiIndex].iRight;
          Region.bottom = gListOfDirtyRegions[uiIndex].iBottom;

          _blitFast(vsBackBuffer, vsFB, Region.left, Region.top, &Region);
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

          _blitFast(vsBackBuffer, vsFB, Region.left, Region.top, &Region);
        }
      }
    }
    if (gfRenderScroll) {
      ScrollJA2Background(guiScrollDirection);
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
    FILE *OutputFile;
    char FileName[64];
    int32_t iIndex;
    uint16_t *p16BPPData;

    // Create temporary system memory surface. This is used to correct problems with the backbuffer
    // surface which can be interlaced or have a funky pitch
    struct VSurface *vsTmp = JSurface_Create16bpp(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Copy the primary surface to the temporary surface
    Region.left = 0;
    Region.top = 0;
    Region.right = usScreenWidth;
    Region.bottom = usScreenHeight;
    _blitFast(vsTmp, vsPrimary, 0, 0, &Region);

    //
    // Ok now that temp surface has contents of backbuffer, copy temp surface to disk
    //

    sprintf(FileName, "../SCREEN%03d.TGA", guiPrintFrameBufferIndex++);
    if ((OutputFile = fopen(FileName, "wb")) != NULL) {
      fprintf(OutputFile, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0x80, 0x02, 0xe0, 0x01, 0x10, 0);

      uint8_t *data;
      uint32_t pitch;

      data = LockVSurface(vsTmp, &pitch);

      //
      // Copy 16 bit buffer to file
      //

      uint32_t redMask;
      uint32_t greenMask;
      uint32_t blueMask;
      JVideo_GetRGBDistributionMasks(&redMask, &greenMask, &blueMask);

      // 5/6/5.. create buffer...
      if (redMask == 0xF800 && greenMask == 0x07E0 && blueMask == 0x001F) {
        p16BPPData = (uint16_t *)MemAlloc(640 * 2);
      }

      for (iIndex = 479; iIndex >= 0; iIndex--) {
        // ATE: OK, fix this such that it converts pixel format to 5/5/5
        // if current settings are 5/6/5....
        if (redMask == 0xF800 && greenMask == 0x07E0 && blueMask == 0x001F) {
          // Read into a buffer...
          memcpy(p16BPPData, (data + (iIndex * 640 * 2)), 640 * 2);

          // Convert....
          ConvertRGBDistribution565To555(p16BPPData, 640);

          // Write
          fwrite(p16BPPData, 640 * 2, 1, OutputFile);
        } else {
          fwrite((void *)(data + (iIndex * 640 * 2)), 640 * 2, 1, OutputFile);
        }
      }

      // 5/6/5.. Delete buffer...
      if (redMask == 0xF800 && greenMask == 0x07E0 && blueMask == 0x001F) {
        MemFree(p16BPPData);
      }

      fclose(OutputFile);

      JSurface_Unlock(vsTmp);
    }

    //
    // Release temp surface
    //

    gfPrintFrameBuffer = FALSE;

    JSurface_Free(vsTmp);
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

    _blitFast(vsMouseBuffer, vsMouseBufferOriginal, 0, 0, &Region);

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

        _blitFast(gMouseCursorBackground[CURRENT_MOUSE_DATA].vs, vsBackBuffer,
                  gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft,
                  gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop, &Region);

        //
        // Step (2) - Blit mouse cursor to back buffer
        //

        Region.left = gMouseCursorBackground[CURRENT_MOUSE_DATA].usLeft;
        Region.top = gMouseCursorBackground[CURRENT_MOUSE_DATA].usTop;
        Region.right = gMouseCursorBackground[CURRENT_MOUSE_DATA].usRight;
        Region.bottom = gMouseCursorBackground[CURRENT_MOUSE_DATA].usBottom;

        _blitFast(vsBackBuffer, vsMouseBuffer,
                  gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
                  gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos, &Region);
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

  // Step (1) - Flip pages
  if (!JSurface_Flip(vsPrimary)) {
    return;
  }

  //
  // Step (2) - Copy Primary Surface to the Back Buffer
  //
  if (gfRenderScroll) {
    Region.left = 0;
    Region.top = 0;
    Region.right = 640;
    Region.bottom = 360;

    _blitFast(vsBackBuffer, vsPrimary, 0, 0, &Region);

    // Get new background for mouse
    //
    // Ok, do the actual data save to the mouse background

    gfRenderScroll = FALSE;
    gfScrollStart = FALSE;
  }

  // COPY MOUSE AREAS FROM PRIMARY BACK!

  // FIRST OLD ERASED POSITION
  if (gMouseCursorBackground[PREVIOUS_MOUSE_DATA].fRestore == TRUE) {
    Region = gMouseCursorBackground[PREVIOUS_MOUSE_DATA].Region;

    _blitFast(vsBackBuffer, vsPrimary, gMouseCursorBackground[PREVIOUS_MOUSE_DATA].usMouseXPos,
              gMouseCursorBackground[PREVIOUS_MOUSE_DATA].usMouseYPos, &Region);
  }

  // NOW NEW MOUSE AREA
  if (gMouseCursorBackground[CURRENT_MOUSE_DATA].fRestore == TRUE) {
    Region = gMouseCursorBackground[CURRENT_MOUSE_DATA].Region;

    _blitFast(vsBackBuffer, vsPrimary, gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseXPos,
              gMouseCursorBackground[CURRENT_MOUSE_DATA].usMouseYPos, &Region);
  }

  if (gfForceFullScreenRefresh == TRUE) {
    //
    // Method (1) - We will be refreshing the entire screen
    //
    Region.left = 0;
    Region.top = 0;
    Region.right = SCREEN_WIDTH;
    Region.bottom = SCREEN_HEIGHT;

    _blitFast(vsBackBuffer, vsPrimary, 0, 0, &Region);

    guiDirtyRegionCount = 0;
    guiDirtyRegionExCount = 0;
    gfForceFullScreenRefresh = FALSE;
  } else {
    for (uiIndex = 0; uiIndex < guiDirtyRegionCount; uiIndex++) {
      Region.left = gListOfDirtyRegions[uiIndex].iLeft;
      Region.top = gListOfDirtyRegions[uiIndex].iTop;
      Region.right = gListOfDirtyRegions[uiIndex].iRight;
      Region.bottom = gListOfDirtyRegions[uiIndex].iBottom;

      _blitFast(vsBackBuffer, vsPrimary, Region.left, Region.top, &Region);
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

    _blitFast(vsBackBuffer, vsPrimary, Region.left, Region.top, &Region);
  }

  guiDirtyRegionExCount = 0;
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

  ReturnValue = BltVideoObject(vsMouseBufferOriginal, gpCursorStore, usVideoObjectSubIndex, 0, 0);
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
  uint16_t *pVideo, *pDest;

  uint32_t pitch;
  pVideo = (uint16_t *)LockVSurface(vsPrimary, &pitch);
  if (pVideo == NULL) {
    return;
  }

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

  JSurface_Unlock(vsPrimary);
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
  char cFilename[260];
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
