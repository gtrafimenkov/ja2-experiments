// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

// Temporary stubs for missing linux platform functionality.

#include "Point.h"
#include "SGP/Input.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "Utils/Cinematics.h"
#include "Utils/TimerControl.h"
#include "platform.h"

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

char gzCommandLine[100];

void DebugPrint(const char *message) {}

int strcasecmp(const char *s1, const char *s2) { return 0; }

int strncasecmp(const char *s1, const char *s2, size_t n) { return 0; }

extern uint32_t Plat_GetTickCount() { return 0; }

uint32_t GetClock(void) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
// I/O
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfProgramIsRunning;

BOOLEAN Plat_GetCurrentDirectory(STRING512 pcDirectory) { return FALSE; }

uint32_t Plat_GetFreeSpaceOnHardDriveWhereGameIsRunningFrom() { return 0; }

BOOLEAN Plat_CreateDirectory(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_GetFileIsReadonly(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsSystem(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsHidden(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsDirectory(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsOffline(const struct GetFile *gfs) { return FALSE; }

BOOLEAN Plat_GetFileIsTemporary(const struct GetFile *gfs) { return FALSE; }

HWFILE FileMan_Open(char *strFilename, uint32_t uiOptions, BOOLEAN fDeleteOnClose) { return 0; }

void FileMan_Close(HWFILE hFile) {}

BOOLEAN FileMan_Read(HWFILE hFile, void *pDest, uint32_t uiBytesToRead, uint32_t *puiBytesRead) {
  return FALSE;
}

BOOLEAN FileMan_Write(HWFILE hFile, const void *pDest, uint32_t uiBytesToWrite,
                      uint32_t *puiBytesWritten) {
  return FALSE;
}

BOOLEAN FileMan_Seek(HWFILE hFile, uint32_t uiDistance, uint8_t uiHow) { return FALSE; }

int32_t FileMan_GetPos(HWFILE hFile) { return 0; }

uint32_t FileMan_GetSize(HWFILE hFile) { return 0; }

BOOLEAN Plat_DirectoryExists(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_RemoveDirectory(const char *pcDirectory, BOOLEAN fRecursive) { return FALSE; }

BOOLEAN Plat_EraseDirectory(const char *pcDirectory) { return FALSE; }

BOOLEAN Plat_GetFileFirst(char *pSpec, struct GetFile *pGFStruct) { return FALSE; }

BOOLEAN Plat_GetFileNext(struct GetFile *pGFStruct) { return FALSE; }

void Plat_GetFileClose(struct GetFile *pGFStruct) {}

BOOLEAN Plat_ClearFileAttributes(char *strFilename) { return FALSE; }

BOOLEAN FileMan_CheckEndOfFile(HWFILE hFile) { return FALSE; }

BOOLEAN FileMan_GetFileWriteTime(HWFILE hFile, uint64_t *pLastWriteTime) { return FALSE; }

uint32_t FileMan_Size(char *strFilename) { return 0; }

/////////////////////////////////////////////////////////////////////////////////
// Input
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN gfKeyState[256];
BOOLEAN gfLeftButtonState;
BOOLEAN gfRightButtonState;
uint16_t gusMouseXPos;
uint16_t gusMouseYPos;

BOOLEAN gfSGPInputReceived = FALSE;

BOOLEAN DequeueSpecificEvent(InputAtom *Event, uint32_t uiMaskFlags) { return FALSE; }

BOOLEAN DequeueEvent(InputAtom *Event) { return FALSE; }

void GetMousePos(SGPPoint *Point) {}

void RestrictMouseToXYXY(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2) {}

void RestrictMouseCursor(SGPRect *pRectangle) {}

void FreeMouseCursor(void) {}

void GetRestrictedClipCursor(SGPRect *pRectangle) {}

BOOLEAN IsCursorRestricted(void) { return FALSE; }

void SimulateMouseMovement(uint32_t uiNewXPos, uint32_t uiNewYPos) {}

void DequeueAllKeyBoardEvents() {}

struct Point GetMousePoint() {
  struct Point res = {0, 0};
  return res;
}

/////////////////////////////////////////////////////////////////////////////////
// Time
/////////////////////////////////////////////////////////////////////////////////

BOOLEAN InitializeJA2Clock(void) { return FALSE; }

void ShutdownJA2Clock(void) {}

void PauseTime(BOOLEAN fPaused) {}

void SetCustomizableTimerCallbackAndDelay(int32_t iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback,
                                          BOOLEAN fReplace) {}

void CheckCustomizableTimer(void) {}

/////////////////////////////////////////////////////////////////////////////////
// Sound
/////////////////////////////////////////////////////////////////////////////////

uint32_t SoundPlay(char *pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

uint32_t SoundPlayStreamedFile(char *pFilename, SOUNDPARMS *pParms) { return (SOUND_ERROR); }

uint32_t SoundPlayRandom(char *pFilename, RANDOMPARMS *pParms) { return (SOUND_ERROR); }

BOOLEAN SoundIsPlaying(uint32_t uiSoundID) { return FALSE; }

BOOLEAN SoundStop(uint32_t uiSoundID) { return FALSE; }

BOOLEAN SoundStopAll(void) { return FALSE; }

BOOLEAN SoundSetVolume(uint32_t uiSoundID, uint32_t uiVolume) { return FALSE; }

BOOLEAN SoundSetPan(uint32_t uiSoundID, uint32_t uiPan) { return FALSE; }

uint32_t SoundGetVolume(uint32_t uiSoundID) { return (SOUND_ERROR); }

BOOLEAN SoundServiceRandom(void) { return FALSE; }

BOOLEAN SoundStopAllRandom(void) { return FALSE; }

BOOLEAN SoundServiceStreams(void) { return FALSE; }

uint32_t SoundGetPosition(uint32_t uiSoundID) { return 0; }

uint32_t SoundLoadSample(char *pFilename) { return (NO_SAMPLE); }

uint32_t SoundLockSample(char *pFilename) { return (NO_SAMPLE); }

uint32_t SoundUnlockSample(char *pFilename) { return (NO_SAMPLE); }

void SoundRemoveSampleFlags(uint32_t uiSample, uint32_t uiFlags) {}

/////////////////////////////////////////////////////////////////////////////////
// Video
/////////////////////////////////////////////////////////////////////////////////

int32_t giNumFrames = 0;

void InvalidateRegion(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom) {}

void InvalidateRegionEx(int32_t iLeft, int32_t iTop, int32_t iRight, int32_t iBottom,
                        uint32_t uiFlags) {}

void InvalidateScreen(void) {}

void RefreshScreen(void *DummyVariable) {}

BOOLEAN GetPrimaryRGBDistributionMasks(uint32_t *RedBitMask, uint32_t *GreenBitMask,
                                       uint32_t *BlueBitMask) {
  return FALSE;
}

BOOLEAN SetMouseCursorProperties(int16_t sOffsetX, int16_t sOffsetY, uint16_t usCursorHeight,
                                 uint16_t usCursorWidth) {
  return FALSE;
}

void DirtyCursor() {}

BOOLEAN SetCurrentCursor(uint16_t usVideoObjectSubIndex, uint16_t usOffsetX, uint16_t usOffsetY) {
  return FALSE;
}

void EndFrameBufferRender(void) {}

BOOLEAN Set8BPPPalette(struct SGPPaletteEntry *pPalette) { return FALSE; }

void FatalError(char *pError, ...) {}

BOOLEAN ColorFillVSurfaceArea(struct VSurface *dest, int32_t iDestX1, int32_t iDestY1,
                              int32_t iDestX2, int32_t iDestY2, uint16_t Color16BPP) {
  return FALSE;
}

BOOLEAN SetVideoSurfacePalette(struct VSurface *hVSurface, struct SGPPaletteEntry *pSrcPalette) {
  return FALSE;
}

BOOLEAN GetVSurfacePaletteEntries(struct VSurface *hVSurface, struct SGPPaletteEntry *pPalette) {
  return FALSE;
}

BOOLEAN DeleteVSurface(struct VSurface *hVSurface) { return FALSE; }

BOOLEAN BltVSurfaceRectToPoint(struct VSurface *dest, struct VSurface *src, int32_t iDestX,
                               int32_t iDestY, struct Rect *SrcRect) {
  return FALSE;
}

void DDBltFast(struct VSurface *dest, struct VSurface *src, uint32_t destX, uint32_t destY,
               struct Rect *srcRect) {}

void DumpVSurfaceInfoIntoFile(char *filename, BOOLEAN fAppend) {}

uint8_t *LockVSurface(struct VSurface *hVSurface, uint32_t *pPitch) { return NULL; }
void UnlockVSurface(struct VSurface *hVSurface) {}

BOOLEAN SmkPollFlics(void) { return FALSE; }

void SmkInitialize(uint32_t uiWidth, uint32_t uiHeight) {}

void SmkShutdown(void) {}

struct SmkFlic *SmkPlayFlic(char *cFilename, uint32_t uiLeft, uint32_t uiTop, BOOLEAN fClose) {
  return NULL;
}

void SmkCloseFlic(struct SmkFlic *pSmack) {}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

uint16_t PackColorsToRGB16(uint8_t r, uint8_t g, uint8_t b) { return 0; }
void UnpackRGB16(uint16_t rgb16, uint8_t *r, uint8_t *g, uint8_t *b) {}
void GetRGB16Masks(uint16_t *red, uint16_t *green, uint16_t *blue) {}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
