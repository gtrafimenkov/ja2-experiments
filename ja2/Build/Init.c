#include <stdio.h>

#include "Editor/EditScreen.h"
#include "Editor/SummaryInfo.h"
#include "GameLoop.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "Laptop/Laptop.h"
#include "SGP/ButtonSystem.h"
#include "SGP/CursorControl.h"
#include "SGP/MouseSystem.h"
#include "SGP/SGP.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "Screens.h"
#include "Strategic/GameInit.h"
#include "Strategic/MapScreen.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMovement.h"
#include "SysGlobals.h"
#include "Tactical/AnimationData.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "Tactical/Vehicles.h"
#include "TacticalAI/NPC.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/RadarScreen.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/ShadeTableUtil.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/WorldDef.h"
#include "Utils/Cursors.h"
#include "Utils/EventPump.h"
#include "Utils/FontControl.h"
#include "Utils/MapUtility.h"
#include "Utils/MercTextBox.h"
#include "Utils/MultilingualTextCodeGenerator.h"
#include "Utils/MusicControl.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"

extern BOOLEAN GetCDromDriveLetter(STR8 pString);

// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game

#ifdef JA2BETAVERSION
extern BOOLEAN gfUseConsecutiveQuickSaveSlots;
#endif

#if defined(GERMAN) && !defined(_DEBUG)
//#define LASERLOCK_ENABLED
#endif

#ifdef LASERLOCK_ENABLED
int LASERLOK_Init(HINSTANCE hInstance);
int LASERLOK_Run();
int LASERLOK_Check();

BOOLEAN PrepareLaserLockSystem();
void HandleLaserLockResult(BOOLEAN fSuccess);
//	int	TestCall( int n);
#endif

extern HINSTANCE ghInstance;

UINT32 InitializeJA2(void) {
#ifdef LASERLOCK_ENABLED
  HandleLaserLockResult(PrepareLaserLockSystem());
#endif

  HandleJA2CDCheck();

  gfWorldLoaded = FALSE;

  // Load external text
  LoadAllExternalText();

  // Init JA2 sounds
  InitJA2Sound();

  gsRenderCenterX = 805;
  gsRenderCenterY = 805;

  // Init data
  InitializeSystemVideoObjects();

  // Init animation system
  if (!InitAnimationSystem()) {
    return (ERROR_SCREEN);
  }

  // Init lighting system
  InitLightingSystem();

  // Init dialog queue system
  InitalizeDialogueControl();

  if (!InitStrategicEngine()) {
    return (ERROR_SCREEN);
  }

  // needs to be called here to init the SectorInfo struct
  InitStrategicMovementCosts();

  // Init tactical engine
  if (!InitTacticalEngine()) {
    return (ERROR_SCREEN);
  }

  // Init timer system
  // Moved to the splash screen code.
  // InitializeJA2Clock( );

  // INit shade tables
  BuildShadeTable();

  // INit intensity tables
  BuildIntensityTable();

  // Init Event Manager
  if (!InitializeEventManager()) {
    return (ERROR_SCREEN);
  }

  // Initailize World
  if (!InitializeWorld()) {
    return (ERROR_SCREEN);
  }

  InitTileCache();

  InitMercPopupBox();

  // Set global volume
  MusicSetVolume(gGameSettings.ubMusicVolumeSetting);

  DetermineRGBDistributionSettings();

#ifdef JA2EDITOR

  // UNCOMMENT NEXT LINE TO ALLOW FORCE UPDATES...
  // LoadGlobalSummary();
  if (gfMustForceUpdateAllMaps) {
    ApologizeOverrideAndForceUpdateEverything();
  }
#endif

#ifdef JA2BETAVERSION
  if (ProcessIfMultilingualCmdLineArgDetected(
          gzCommandLine)) {  // If the multilingual text code generator has activated, quit now.
    gfProgramIsRunning = FALSE;
    return (INIT_SCREEN);
  }
#endif

#ifdef JA2BETAVERSION
  // CHECK COMMANDLINE FOR SPECIAL UTILITY
  if (strcmp(gzCommandLine, "-DOMAPS") == 0) {
    return (MAPUTILITY_SCREEN);
  }
#endif

#ifdef JA2BETAVERSION
  // This allows the QuickSave Slots to be autoincremented, ie everytime the user saves, there will
  // be a new quick save file
  if (_stricmp(gzCommandLine, "-quicksave") == 0) {
    gfUseConsecutiveQuickSaveSlots = TRUE;
  }
#endif

#ifdef JA2EDITOR
  // CHECK COMMANDLINE FOR SPECIAL UTILITY
  if (!strcmp(gzCommandLine, "-EDITORAUTO")) {
    OutputDebugString("Beginning JA2 using -EDITORAUTO commandline argument...\n");
    // For editor purposes, need to know the default map file.
    sprintf(gubFilename, "none");
    // also set the sector
    gWorldSectorX = 0;
    gWorldSectorY = 0;
    gfAutoLoadA9 = TRUE;
    gfIntendOnEnteringEditor = TRUE;
    gGameOptions.fGunNut = TRUE;
    return (GAME_SCREEN);
  }
  if (strcmp(gzCommandLine, "-EDITOR") == 0) {
    OutputDebugString("Beginning JA2 using -EDITOR commandline argument...\n");
    // For editor purposes, need to know the default map file.
    sprintf(gubFilename, "none");
    // also set the sector
    gWorldSectorX = 0;
    gWorldSectorY = 0;
    gfAutoLoadA9 = FALSE;
    gfIntendOnEnteringEditor = TRUE;
    gGameOptions.fGunNut = TRUE;
    return (GAME_SCREEN);
  }
#endif

  return (INIT_SCREEN);
}

void ShutdownJA2(void) {
  UINT32 uiIndex;

  // Clear screen....
  ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)));
  InvalidateScreen();
  // Remove cursor....
  SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

  RefreshScreen(NULL);

  ShutdownStrategicLayer();

  // remove temp files built by laptop
  ClearOutTempLaptopFiles();

  // Shutdown queue system
  ShutdownDialogueControl();

  // Shutdown Screens
  for (uiIndex = 0; uiIndex < MAX_SCREENS; uiIndex++) {
    (*(GameScreens[uiIndex].ShutdownScreen))();
  }

  // Shutdown animation system
  DeInitAnimationSystem();

  ShutdownLightingSystem();

  CursorDatabaseClear();

  ShutdownTacticalEngine();

  // Shutdown Overhead
  ShutdownOverhead();

  DeinitializeWorld();

  DeleteTileCache();

  ShutdownJA2Clock();

  ShutdownFonts();

  ShutdownJA2Sound();

  ShutdownEventManager();

  ShutdownBaseDirtyRectQueue();

  // Unload any text box images!
  RemoveTextMercPopupImages();

  ClearOutVehicleList();
}

#ifdef LASERLOCK_ENABLED

BOOLEAN PrepareLaserLockSystem() {
  INT32 iInitRetVal = 0;
  INT32 iRunRetVal = 0;
  INT32 iCheckRetVal = 0;
  CHAR8 zDirectory[512];

  CHAR8 zCdLocation[SGPFILENAME_LEN];
  CHAR8 zCdFile[SGPFILENAME_LEN];

  // Get the "current" file directory
  GetFileManCurrentDirectory(zDirectory);

  if (GetCDromDriveLetter(zCdLocation)) {
    // OK, build filename
    sprintf(zCdFile, "%s%s", zCdLocation, "Data");
  } else {
    goto FAILED_LASERLOK;
  }

  // Go back to the root directory
  SetFileManCurrentDirectory(zCdFile);
  // Init the laser lock system
  iInitRetVal = LASERLOK_Init(ghInstance);
  if (iInitRetVal != 0) goto FAILED_LASERLOK;

  // makes the verification of the laserlok system
  iRunRetVal = LASERLOK_Run();
  if (iRunRetVal != 0) goto FAILED_LASERLOK;

  // checks the result of the laserlok run function
  iCheckRetVal = LASERLOK_Check();
  if (iCheckRetVal != 0) goto FAILED_LASERLOK;

  // Restore back to the proper directory
  SetFileManCurrentDirectory(zDirectory);
  return (TRUE);

FAILED_LASERLOK:
  // Restore back to the proper directory
  SetFileManCurrentDirectory(zDirectory);
  return (FALSE);
}

void HandleLaserLockResult(BOOLEAN fSuccess) {
  if (!fSuccess) {
    CHAR8 zString[512];

    sprintf(zString, "%S", gzLateLocalizedString[56]);

    ShowCursor(TRUE);
    ShowCursor(TRUE);
    ShutdownWithErrorBox(zString);
  }
}

#endif
