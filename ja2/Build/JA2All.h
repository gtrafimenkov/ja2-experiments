#ifndef __JA2_ALL_H
#define __JA2_ALL_H

#pragma message("GENERATED PCH FOR JA2 PROJECT.")

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include "SGP/SGP.h"
#include "GameLoop.h"
#include "SGP/HImage.h"
#include "SGP/VObject.h"
#include "SGP/VObjectPrivate.h"
#include "SGP/VObjectBlitters.h"
#include "SGP/Types.h"
#include "SGP/WCheck.h"
#include "TileEngine/RenderWorld.h"
#include "SGP/Input.h"
#include "ScreenIDs.h"
#include "Tactical/Overhead.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/RadarScreen.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/AnimationData.h"
#include "Utils/EventPump.h"
#include "Utils/TimerControl.h"
#include "TileEngine/RenderDirty.h"
#include "SysGlobals.h"
#include "Tactical/Interface.h"
#include "Tactical/SoldierAni.h"
#include <wchar.h>
#include <tchar.h>
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "MessageBoxScreen.h"
#include "SGP/SGP.h"
#include "FadeScreen.h"
#include "SGP/CursorControl.h"
#include "Utils/MusicControl.h"
#include "GameInitOptionsScreen.h"
#include "GameSettings.h"
#include "Utils/Utilities.h"
#include "Utils/FontControl.h"
#include "Utils/WordWrap.h"
#include "OptionsScreen.h"
#include "Utils/Cursors.h"
#include "Screens.h"
#include "Init.h"
#include "Laptop/Laptop.h"
#include "Strategic/MapScreen.h"
#include "Strategic/GameClock.h"
#include "SGP/LibraryDataBase.h"
#include "Strategic/MapScreenInterface.h"
#include "Tactical/TacticalSave.h"
#include "Tactical/InterfaceControl.h"
#include "JA2DemoAds.h"
#include "Utils/Text.h"
#include "Tactical/HandleUI.h"
#include "SGP/ButtonSystem.h"
#include "TileEngine/Lighting.h"
#include "TileEngine/Environment.h"
#include "Tactical/Bullets.h"
#include "Utils/Message.h"
#include <string.h>
#include "TileEngine/OverheadMap.h"
#include "Tactical/StrategicExitGUI.h"
#include "Strategic/StrategicMovement.h"
#include "TileEngine/TacticalPlacementGUI.h"
#include "Tactical/AirRaid.h"
#include "Strategic/GameInit.h"
// DEF: Test Code
#ifdef NETWORKED
#include "Networking.h"
#endif
#include "TileEngine/Physics.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/Faces.h"
#include "Strategic/StrategicMap.h"
#include "GameScreen.h"
#include "Strategic/StrategicTurns.h"
#include "Tactical/MercEntering.h"
#include "Tactical/SoldierCreate.h"
#include "Tactical/SoldierInitList.h"
#include "Tactical/InterfacePanels.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Squads.h"
#include "Tactical/interfaceDialogue.h"
#include "Tactical/AutoBandage.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/StrategicAI.h"
#include "Utils/SoundControl.h"
#include "SaveLoadScreen.h"
#include "GameVersion.h"
#include "SGP/Debug.h"
#include "LanguageDefines.h"
#include "SGP/MouseSystem.h"
#include "TileEngine/WorldDef.h"
#include "SGP/Video.h"
#include "Tactical/InterfaceItems.h"
#include "Utils/MapUtility.h"
#include "Strategic/Strategic.h"
#include "TacticalAI/NPC.h"
#include "Utils/MercTextBox.h"
#include "TileEngine/TileCache.h"
#include "TileEngine/ShadeTableUtil.h"
#include "TileEngine/ExitGrids.h"
#include "Editor/SummaryInfo.h"
#include <time.h>
#include "SGP/Font.h"
#include "SGP/Timer.h"
#include "TileEngine/TileDef.h"
#include "Editor/EditScreen.h"
#include "JAScreens.h"
#include "Tactical/AnimationCache.h"
#include "MainMenuScreen.h"
#include "SGP/Random.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "SaveLoadGame.h"
#include "Utils/TextInput.h"
#include "Utils/Slider.h"
#include "SGP/SoundMan.h"
#include "TileEngine/AmbientControl.h"
#include "TileEngine/WorldDat.h"
#include "Tactical/Gap.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Keys.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Files.h"
#include "Laptop/Email.h"
#include "Strategic/GameEvents.h"
#include "Laptop/LaptopSave.h"
#include "Strategic/QueenCommand.h"
#include "Strategic/Quests.h"
#include "Tactical/OppList.h"
#include "Tactical/MercHiring.h"
#include "TacticalAI/AI.h"
#include "TileEngine/SmokeEffects.h"
#include "Strategic/MapScreenInterfaceBorder.h"
#include "Strategic/MapScreenInterfaceBottom.h"
#include "Strategic/MapScreenHelicopter.h"
#include "Tactical/ArmsDealerInit.h"
#include "Strategic/StrategicMines.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/Vehicles.h"
#include "Strategic/MercContract.h"
#include "Strategic/StrategicPathing.h"
#include "Tactical/TeamTurns.h"
#include "TileEngine/ExplosionControl.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/StrategicStatus.h"
#include "Tactical/Boxing.h"
#include "Strategic/MapScreenInterfaceMap.h"
#include "TileEngine/LightEffects.h"
#include "JA2Splash.h"
#include "Strategic/Scheduling.h"
#include "Utils/Ja25EnglishText.h"

#endif
