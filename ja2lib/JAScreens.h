#ifndef __JASCREENS_H_
#define __JASCREENS_H_

#include "GameInput.h"
#include "SGP/Types.h"

extern UINT32 EditScreenInit(void);
extern UINT32 EditScreenHandle(const struct GameInput *gameInput);
extern UINT32 EditScreenShutdown(void);

extern UINT32 LoadSaveScreenInit(void);
extern UINT32 LoadSaveScreenHandle(const struct GameInput *gameInput);
extern UINT32 LoadSaveScreenShutdown(void);

extern UINT32 SavingScreenInitialize(void);
extern UINT32 SavingScreenHandle(const struct GameInput *gameInput);
extern UINT32 SavingScreenShutdown(void);

extern UINT32 LoadingScreenInitialize(void);
extern UINT32 LoadingScreenHandle(const struct GameInput *gameInput);
extern UINT32 LoadingScreenShutdown(void);

extern UINT32 ErrorScreenInitialize(void);
extern UINT32 ErrorScreenHandle(const struct GameInput *gameInput);
extern UINT32 ErrorScreenShutdown(void);

extern UINT32 InitScreenInitialize(void);
extern UINT32 InitScreenHandle(const struct GameInput *gameInput);
extern UINT32 InitScreenShutdown(void);

extern UINT32 MainGameScreenInit(void);
extern UINT32 MainGameScreenHandle(const struct GameInput *gameInput);
extern UINT32 MainGameScreenShutdown(void);

#ifdef JA2BETAVERSION
extern UINT32 AIViewerScreenInit(void);
extern UINT32 AIViewerScreenHandle(const struct GameInput *gameInput);
extern UINT32 AIViewerScreenShutdown(void);
#endif

extern UINT32 QuestDebugScreenInit(void);
extern UINT32 QuestDebugScreenHandle(const struct GameInput *gameInput);
extern UINT32 QuestDebugScreenShutdown(void);

UINT32 AniEditScreenInit(void);
UINT32 AniEditScreenHandle(const struct GameInput *gameInput);
UINT32 AniEditScreenShutdown(void);

UINT32 PalEditScreenInit(void);
UINT32 PalEditScreenHandle(const struct GameInput *gameInput);
UINT32 PalEditScreenShutdown(void);

UINT32 DebugScreenInit(void);
UINT32 DebugScreenHandle(const struct GameInput *gameInput);
UINT32 DebugScreenShutdown(void);

extern UINT32 MapScreenInit(void);
extern UINT32 MapScreenHandle(const struct GameInput *gameInput);
extern UINT32 MapScreenShutdown(void);

UINT32 LaptopScreenInit(void);
UINT32 LaptopScreenHandle(const struct GameInput *gameInput);
UINT32 LaptopScreenShutdown(void);

UINT32 MapUtilScreenInit(void);
UINT32 MapUtilScreenHandle(const struct GameInput *gameInput);
UINT32 MapUtilScreenShutdown(void);

UINT32 FadeScreenInit(void);
UINT32 FadeScreenHandle(const struct GameInput *gameInput);
UINT32 FadeScreenShutdown(void);

UINT32 MessageBoxScreenInit(void);
UINT32 MessageBoxScreenHandle(const struct GameInput *gameInput);
UINT32 MessageBoxScreenShutdown(void);

UINT32 MainMenuScreenInit(void);
UINT32 MainMenuScreenHandle(const struct GameInput *gameInput);
UINT32 MainMenuScreenShutdown(void);

UINT32 AutoResolveScreenInit(void);
UINT32 AutoResolveScreenHandle(const struct GameInput *gameInput);
UINT32 AutoResolveScreenShutdown(void);

UINT32 SaveLoadScreenShutdown(void);
UINT32 SaveLoadScreenHandle(const struct GameInput *gameInput);
UINT32 SaveLoadScreenInit(void);

UINT32 ShopKeeperScreenInit(void);
UINT32 ShopKeeperScreenHandle(const struct GameInput *gameInput);
UINT32 ShopKeeperScreenShutdown(void);

UINT32 SexScreenInit(void);
UINT32 SexScreenHandle(const struct GameInput *gameInput);
UINT32 SexScreenShutdown(void);

UINT32 GameInitOptionsScreenInit(void);
UINT32 GameInitOptionsScreenHandle(const struct GameInput *gameInput);
UINT32 GameInitOptionsScreenShutdown(void);

UINT32 DemoExitScreenInit(void);
UINT32 DemoExitScreenHandle(const struct GameInput *gameInput);
UINT32 DemoExitScreenShutdown(void);

extern UINT32 IntroScreenShutdown(void);
extern UINT32 IntroScreenHandle(const struct GameInput *gameInput);
extern UINT32 IntroScreenInit(void);

// External functions
extern void DisplayFrameRate();

void HandleTitleScreenAnimation();

// External Globals
extern CHAR8 gubFilename[200];
extern UINT32 guiCurrentScreen;

typedef void (*RENDER_HOOK)(void);

void SetRenderHook(RENDER_HOOK pRenderOverride);
void SetDebugRenderHook(RENDER_HOOK pDebugRenderOverride, INT8 ubPage);

void DisableFPSOverlay(BOOLEAN fEnable);

void EnterTacticalScreen();

#endif
