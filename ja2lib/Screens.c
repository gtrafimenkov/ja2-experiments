// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Screens.h"

#include "JAScreens.h"
#include "OptionsScreen.h"

Screens GameScreens[MAX_SCREENS] = {
    {EditScreenInit, EditScreenHandle, EditScreenShutdown},
    {SavingScreenInitialize, SavingScreenHandle, SavingScreenShutdown},     // Title Screen
    {LoadingScreenInitialize, LoadingScreenHandle, LoadingScreenShutdown},  // Title Screen
    {ErrorScreenInitialize, ErrorScreenHandle, ErrorScreenShutdown},        // Title Screen
    {InitScreenInitialize, InitScreenHandle, InitScreenShutdown},           // Title Screen
    {MainGameScreenInit, MainGameScreenHandle, MainGameScreenShutdown},
    {AniEditScreenInit, AniEditScreenHandle, AniEditScreenShutdown},
    {PalEditScreenInit, PalEditScreenHandle, PalEditScreenShutdown},
    {DebugScreenInit, DebugScreenHandle, DebugScreenShutdown},
    {MapScreenInit, MapScreenHandle, MapScreenShutdown},
    {LaptopScreenInit, LaptopScreenHandle, LaptopScreenShutdown},
    {LoadSaveScreenInit, LoadSaveScreenHandle, LoadSaveScreenShutdown},
    {MapUtilScreenInit, MapUtilScreenHandle, MapUtilScreenShutdown},
    {FadeScreenInit, FadeScreenHandle, FadeScreenShutdown},
    {MessageBoxScreenInit, MessageBoxScreenHandle, MessageBoxScreenShutdown},
    {MainMenuScreenInit, MainMenuScreenHandle, MainMenuScreenShutdown},
    {AutoResolveScreenInit, AutoResolveScreenHandle, AutoResolveScreenShutdown},
    {SaveLoadScreenInit, SaveLoadScreenHandle, SaveLoadScreenShutdown},
    {OptionsScreenInit, OptionsScreenHandle, OptionsScreenShutdown},
    {ShopKeeperScreenInit, ShopKeeperScreenHandle, ShopKeeperScreenShutdown},
    {SexScreenInit, SexScreenHandle, SexScreenShutdown},
    {GameInitOptionsScreenInit, GameInitOptionsScreenHandle, GameInitOptionsScreenShutdown},
    {DemoExitScreenInit, DemoExitScreenHandle, DemoExitScreenShutdown},
    {IntroScreenInit, IntroScreenHandle, IntroScreenShutdown},
    {CreditScreenInit, CreditScreenHandle, CreditScreenShutdown},

#ifdef JA2BETAVERSION
    {AIViewerScreenInit, AIViewerScreenHandle, AIViewerScreenShutdown},
#endif

    {QuestDebugScreenInit, QuestDebugScreenHandle, QuestDebugScreenShutdown}};
