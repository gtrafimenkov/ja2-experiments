// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "OptionsScreen.h"

#include "GameSettings.h"
#include "JAScreens.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameInit.h"
#include "Tactical/Gap.h"
#include "Tactical/InterfaceControl.h"
#include "Tactical/MapInformation.h"
#include "Tactical/Overhead.h"
#include "TileEngine/AmbientControl.h"
#include "TileEngine/ExitGrids.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SmokeEffects.h"
#include "TileEngine/SysUtil.h"
#include "TileEngine/WorldDat.h"
#include "TileEngine/WorldMan.h"
#include "UI.h"
#include "Utils/Cursors.h"
#include "Utils/FontControl.h"
#include "Utils/Message.h"
#include "Utils/MultiLanguageGraphicUtils.h"
#include "Utils/MusicControl.h"
#include "Utils/Slider.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/TextInput.h"
#include "Utils/TimerControl.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

/////////////////////////////////
//
//	Defines
//
/////////////////////////////////

#define OPTIONS_TITLE_FONT FONT14ARIAL
#define OPTIONS_TITLE_COLOR FONT_MCOLOR_WHITE

#define OPT_MAIN_FONT FONT12ARIAL
#define OPT_MAIN_COLOR OPT_BUTTON_ON_COLOR     // FONT_MCOLOR_WHITE
#define OPT_HIGHLIGHT_COLOR FONT_MCOLOR_WHITE  // FONT_MCOLOR_LTYELLOW

#define OPTIONS_SCREEN_WIDTH 440
#define OPTIONS_SCREEN_HEIGHT 400

#define OPTIONS__TOP_LEFT_X 100
#define OPTIONS__TOP_LEFT_Y 40
#define OPTIONS__BOTTOM_RIGHT_X OPTIONS__TOP_LEFT_X + OPTIONS_SCREEN_WIDTH
#define OPTIONS__BOTTOM_RIGHT_Y OPTIONS__TOP_LEFT_Y + OPTIONS_SCREEN_HEIGHT

#define OPT_SAVE_BTN_X 51
#define OPT_SAVE_BTN_Y 438

#define OPT_LOAD_BTN_X 190
#define OPT_LOAD_BTN_Y OPT_SAVE_BTN_Y

#define OPT_QUIT_BTN_X 329
#define OPT_QUIT_BTN_Y OPT_SAVE_BTN_Y

#define OPT_DONE_BTN_X 469
#define OPT_DONE_BTN_Y OPT_SAVE_BTN_Y

#define OPT_GAP_BETWEEN_TOGGLE_BOXES 31  // 40

// Text
#define OPT_TOGGLE_BOX_FIRST_COL_TEXT_X \
  OPT_TOGGLE_BOX_FIRST_COLUMN_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX      // 350
#define OPT_TOGGLE_BOX_FIRST_COL_TEXT_Y OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y  // 100

#define OPT_TOGGLE_BOX_SECOND_TEXT_X \
  OPT_TOGGLE_BOX_SECOND_COLUMN_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX   // 350
#define OPT_TOGGLE_BOX_SECOND_TEXT_Y OPT_TOGGLE_BOX_SECOND_COLUMN_START_Y  // 100

// toggle boxes
#define OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX 30  // 220
#define OPT_TOGGLE_TEXT_OFFSET_Y 2                // 3

#define OPT_TOGGLE_BOX_FIRST_COLUMN_X \
  265  // 257 //OPT_TOGGLE_BOX_TEXT_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX
#define OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y 89  // OPT_TOGGLE_BOX_TEXT_Y

#define OPT_TOGGLE_BOX_SECOND_COLUMN_X \
  428  // OPT_TOGGLE_BOX_TEXT_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX
#define OPT_TOGGLE_BOX_SECOND_COLUMN_START_Y OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y

#define OPT_TOGGLE_BOX_TEXT_WIDTH \
  OPT_TOGGLE_BOX_SECOND_COLUMN_X - OPT_TOGGLE_BOX_FIRST_COLUMN_X - 20

// Slider bar defines
#define OPT_GAP_BETWEEN_SLIDER_BARS 60
// #define		OPT_SLIDER_BAR_WIDTH 200
#define OPT_SLIDER_BAR_SIZE 258

#define OPT_SLIDER_TEXT_WIDTH 45

#define OPT_SOUND_FX_TEXT_X 38
#define OPT_SOUND_FX_TEXT_Y 87  // 116//110

#define OPT_SPEECH_TEXT_X 85                   // OPT_SOUND_FX_TEXT_X + OPT_SLIDER_TEXT_WIDTH
#define OPT_SPEECH_TEXT_Y OPT_SOUND_FX_TEXT_Y  // OPT_SOUND_FX_TEXT_Y + OPT_GAP_BETWEEN_SLIDER_BARS

#define OPT_MUSIC_TEXT_X 137
#define OPT_MUSIC_TEXT_Y OPT_SOUND_FX_TEXT_Y  // OPT_SPEECH_TEXT_Y + OPT_GAP_BETWEEN_SLIDER_BARS

#define OPT_TEXT_TO_SLIDER_OFFSET_Y 25

#define OPT_SOUND_EFFECTS_SLIDER_X 56
#define OPT_SOUND_EFFECTS_SLIDER_Y 126  // 110 + OPT_TEXT_TO_SLIDER_OFFSET_Y

#define OPT_SPEECH_SLIDER_X 107
#define OPT_SPEECH_SLIDER_Y OPT_SOUND_EFFECTS_SLIDER_Y

#define OPT_MUSIC_SLIDER_X 158
#define OPT_MUSIC_SLIDER_Y OPT_SOUND_EFFECTS_SLIDER_Y

#define OPT_MUSIC_SLIDER_PLAY_SOUND_DELAY 75

#define OPT_FIRST_COLUMN_TOGGLE_CUT_OFF 10  // 8

/////////////////////////////////
//
//	Global Variables
//
/////////////////////////////////

uint32_t guiOptionBackGroundImage;
uint32_t guiOptionsAddOnImages;

uint32_t guiSoundEffectsSliderID;
uint32_t guiSpeechSliderID;
uint32_t guiMusicSliderID;

BOOLEAN gfOptionsScreenEntry = TRUE;
BOOLEAN gfOptionsScreenExit = FALSE;
BOOLEAN gfRedrawOptionsScreen = TRUE;

char gzSavedGameName[128];
BOOLEAN gfEnteredFromMapScreen = FALSE;

uint32_t guiOptionsScreen = OPTIONS_SCREEN;
uint32_t guiPreviousOptionScreen = OPTIONS_SCREEN;

BOOLEAN gfExitOptionsDueToMessageBox = FALSE;
BOOLEAN gfExitOptionsAfterMessageBox = FALSE;

uint32_t guiSoundFxSliderMoving = 0xffffffff;
uint32_t guiSpeechSliderMoving = 0xffffffff;

int32_t giOptionsMessageBox = -1;  // Options pop up messages index value

int8_t gbHighLightedOptionText = -1;

BOOLEAN gfHideBloodAndGoreOption =
    FALSE;  // If a germany build we are to hide the blood and gore option
uint8_t gubFirstColOfOptions = OPT_FIRST_COLUMN_TOGGLE_CUT_OFF;

BOOLEAN gfSettingOfTreeTopStatusOnEnterOfOptionScreen;
BOOLEAN gfSettingOfItemGlowStatusOnEnterOfOptionScreen;
BOOLEAN gfSettingOfDontAnimateSmoke;

// Goto save game Button
void BtnOptGotoSaveGameCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiOptGotoSaveGameBtn;
int32_t giOptionsButtonImages;

// Goto load game button
void BtnOptGotoLoadGameCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiOptGotoLoadGameBtn;
int32_t giGotoLoadBtnImage;

// QuitButton
void BtnOptQuitCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiQuitButton;
int32_t giQuitBtnImage;

// Done Button
void BtnDoneCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiDoneButton;
int32_t giDoneBtnImage;

// checkbox to toggle tracking mode on or off
uint32_t guiOptionsToggles[NUM_GAME_OPTIONS];
void BtnOptionsTogglesCallback(GUI_BUTTON *btn, int32_t reason);

// Mouse regions for the name of the option
struct MOUSE_REGION gSelectedOptionTextRegion[NUM_GAME_OPTIONS];
void SelectedOptionTextRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectedOptionTextRegionMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason);

// Mouse regions for the area around the toggle boxs
struct MOUSE_REGION gSelectedToggleBoxAreaRegion;
void SelectedToggleBoxAreaRegionMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason);

/////////////////////////////////
//
//	Function ProtoTypes
//
/////////////////////////////////

BOOLEAN EnterOptionsScreen();
void RenderOptionsScreen();
void ExitOptionsScreen();
void HandleOptionsScreen();
void GetOptionsScreenUserInput();
void SetOptionsExitScreen(uint32_t uiExitScreen);

void SoundFXSliderChangeCallBack(int32_t iNewValue);
void SpeechSliderChangeCallBack(int32_t iNewValue);
void MusicSliderChangeCallBack(int32_t iNewValue);
// BOOLEAN		DoOptionsMessageBox( uint8_t ubStyle, wchar_t *zString, uint32_t
// uiExitScreen, uint8_t ubFlags, MSGBOX_CALLBACK ReturnCallback );
void ConfirmQuitToMainMenuMessageBoxCallBack(uint8_t bExitValue);
void HandleSliderBarMovementSounds();
void HandleOptionToggle(uint8_t ubButton, BOOLEAN fState, BOOLEAN fDown, BOOLEAN fPlaySound);
void HandleHighLightedText(BOOLEAN fHighLight);

extern void ToggleItemGlow(BOOLEAN fOn);

// ppp

/////////////////////////////////
//
//	Code
//
/////////////////////////////////

uint32_t OptionsScreenInit() {
  // Set so next time we come in, we can set up
  gfOptionsScreenEntry = TRUE;

  return (TRUE);
}

uint32_t OptionsScreenHandle() {
  if (gfOptionsScreenEntry) {
    PauseGame();
    EnterOptionsScreen();
    gfOptionsScreenEntry = FALSE;
    gfOptionsScreenExit = FALSE;
    gfRedrawOptionsScreen = TRUE;
    RenderOptionsScreen();

    // Blit the background to the save buffer
    BlitBufferToBuffer(vsFB, vsSaveBuffer, 0, 0, 640, 480);
    InvalidateRegion(0, 0, 640, 480);
  }

  RestoreBackgroundRects();

  GetOptionsScreenUserInput();

  HandleOptionsScreen();

  if (gfRedrawOptionsScreen) {
    RenderOptionsScreen();
    RenderButtons();

    gfRedrawOptionsScreen = FALSE;
  }

  // Render the active slider bars
  RenderAllSliderBars();

  // render buttons marked dirty
  MarkButtonsDirty();
  RenderButtons();

  // ATE: Put here to save RECTS before any fast help being drawn...
  SaveBackgroundRects();
  RenderButtonsFastHelp();

  ExecuteBaseDirtyRectQueue();
  EndFrameBufferRender();

  if (gfOptionsScreenExit) {
    ExitOptionsScreen();
    gfOptionsScreenExit = FALSE;
    gfOptionsScreenEntry = TRUE;

    UnPauseGame();
  }

  return (guiOptionsScreen);
}

uint32_t OptionsScreenShutdown() { return (TRUE); }

BOOLEAN EnterOptionsScreen() {
  uint16_t usPosY;
  uint8_t cnt;
  uint16_t usTextWidth, usTextHeight;

  // Default this to off
  gfHideBloodAndGoreOption = FALSE;

  // if we are coming from mapscreen
  if (IsMapScreen()) {
    guiTacticalInterfaceFlags &= ~INTERFACE_MAPSCREEN;
    gfEnteredFromMapScreen = TRUE;
  }

  // Stop ambients...
  StopAmbients();

  guiOptionsScreen = OPTIONS_SCREEN;

  // Init the slider bar;
  InitSlider();

  if (gfExitOptionsDueToMessageBox) {
    gfRedrawOptionsScreen = TRUE;
    gfExitOptionsDueToMessageBox = FALSE;
    return (TRUE);
  }

  gfExitOptionsDueToMessageBox = FALSE;

  // load the options screen background graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("INTERFACE\\OptionScreenBase.sti"),
                    &guiOptionBackGroundImage));

  // load button, title graphic and add it
  CHECKF(AddVObject(CreateVObjectFromMLGFile(MLG_OPTIONHEADER), &guiOptionsAddOnImages));

  // Save game button
  giOptionsButtonImages = LoadButtonImage("INTERFACE\\OptionScreenAddons.sti", -1, 2, -1, 3, -1);
  guiOptGotoSaveGameBtn = CreateIconAndTextButton(
      giOptionsButtonImages, zOptionsText[OPT_SAVE_GAME], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR,
      DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, OPT_SAVE_BTN_X,
      OPT_SAVE_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnOptGotoSaveGameCallback);
  SpecifyDisabledButtonStyle(guiOptGotoSaveGameBtn, DISABLED_STYLE_HATCHED);
  if (guiPreviousOptionScreen == MAINMENU_SCREEN || !CanGameBeSaved()) {
    DisableButton(guiOptGotoSaveGameBtn);
  }

  // Load game button
  giGotoLoadBtnImage = UseLoadedButtonImage(giOptionsButtonImages, -1, 2, -1, 3, -1);
  guiOptGotoLoadGameBtn = CreateIconAndTextButton(
      giGotoLoadBtnImage, zOptionsText[OPT_LOAD_GAME], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR,
      DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, OPT_LOAD_BTN_X,
      OPT_LOAD_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnOptGotoLoadGameCallback);
  //	SpecifyDisabledButtonStyle( guiBobbyRAcceptOrder, DISABLED_STYLE_SHADED );

  // Quit to main menu button
  giQuitBtnImage = UseLoadedButtonImage(giOptionsButtonImages, -1, 2, -1, 3, -1);
  guiQuitButton = CreateIconAndTextButton(
      giQuitBtnImage, zOptionsText[OPT_MAIN_MENU], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR,
      DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, OPT_QUIT_BTN_X,
      OPT_QUIT_BTN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnOptQuitCallback);
  SpecifyDisabledButtonStyle(guiQuitButton, DISABLED_STYLE_HATCHED);
  //	DisableButton( guiQuitButton );

  // Done button
  giDoneBtnImage = UseLoadedButtonImage(giOptionsButtonImages, -1, 2, -1, 3, -1);
  guiDoneButton = CreateIconAndTextButton(
      giDoneBtnImage, zOptionsText[OPT_DONE], OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW,
      OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, TEXT_CJUSTIFIED, OPT_DONE_BTN_X, OPT_DONE_BTN_Y,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnDoneCallback);
  //	SpecifyDisabledButtonStyle( guiBobbyRAcceptOrder, DISABLED_STYLE_SHADED );

  //
  // Toggle Boxes
  //
  usTextHeight = GetFontHeight(OPT_MAIN_FONT);

  // Create the first column of check boxes
  usPosY = OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y;
  gubFirstColOfOptions = OPT_FIRST_COLUMN_TOGGLE_CUT_OFF;
  for (cnt = 0; cnt < gubFirstColOfOptions; cnt++) {
    // if this is the blood and gore option, and we are to hide the option
    if (cnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption) {
      gubFirstColOfOptions++;

      // advance to the next
      continue;
    }
    // Check box to toggle tracking mode
    guiOptionsToggles[cnt] = CreateCheckBoxButton(
        OPT_TOGGLE_BOX_FIRST_COLUMN_X, usPosY, "INTERFACE\\OptionsCheckBoxes.sti",
        MSYS_PRIORITY_HIGH + 10, BtnOptionsTogglesCallback);
    MSYS_SetBtnUserData(guiOptionsToggles[cnt], 0, cnt);

    usTextWidth = StringPixLength(zOptionsToggleText[cnt], OPT_MAIN_FONT);

    if (usTextWidth > OPT_TOGGLE_BOX_TEXT_WIDTH) {
      // Get how many lines will be used to display the string, without displaying the string
      uint8_t ubNumLines =
          DisplayWrappedString(0, 0, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT,
                               OPT_HIGHLIGHT_COLOR, zOptionsToggleText[cnt], FONT_MCOLOR_BLACK,
                               TRUE, LEFT_JUSTIFIED | DONT_DISPLAY_TEXT) /
          GetFontHeight(OPT_MAIN_FONT);

      usTextWidth = OPT_TOGGLE_BOX_TEXT_WIDTH;

      // Create mouse regions for the option toggle text
      MSYS_DefineRegion(&gSelectedOptionTextRegion[cnt], OPT_TOGGLE_BOX_FIRST_COLUMN_X + 13, usPosY,
                        (uint16_t)(OPT_TOGGLE_BOX_FIRST_COL_TEXT_X + usTextWidth),
                        (uint16_t)(usPosY + usTextHeight * ubNumLines), MSYS_PRIORITY_HIGH,
                        CURSOR_NORMAL, SelectedOptionTextRegionMovementCallBack,
                        SelectedOptionTextRegionCallBack);
      MSYS_AddRegion(&gSelectedOptionTextRegion[cnt]);
      MSYS_SetRegionUserData(&gSelectedOptionTextRegion[cnt], 0, cnt);
    } else {
      // Create mouse regions for the option toggle text
      MSYS_DefineRegion(&gSelectedOptionTextRegion[cnt], OPT_TOGGLE_BOX_FIRST_COLUMN_X + 13, usPosY,
                        (uint16_t)(OPT_TOGGLE_BOX_FIRST_COL_TEXT_X + usTextWidth),
                        (uint16_t)(usPosY + usTextHeight), MSYS_PRIORITY_HIGH, CURSOR_NORMAL,
                        SelectedOptionTextRegionMovementCallBack, SelectedOptionTextRegionCallBack);
      MSYS_AddRegion(&gSelectedOptionTextRegion[cnt]);
      MSYS_SetRegionUserData(&gSelectedOptionTextRegion[cnt], 0, cnt);
    }

    SetRegionFastHelpText(&gSelectedOptionTextRegion[cnt], zOptionsScreenHelpText[cnt]);
    SetButtonFastHelpText(guiOptionsToggles[cnt], zOptionsScreenHelpText[cnt]);

    usPosY += OPT_GAP_BETWEEN_TOGGLE_BOXES;
  }

  // Create the 2nd column of check boxes
  usPosY = OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y;
  for (cnt = gubFirstColOfOptions; cnt < NUM_GAME_OPTIONS; cnt++) {
    // Check box to toggle tracking mode
    guiOptionsToggles[cnt] = CreateCheckBoxButton(
        OPT_TOGGLE_BOX_SECOND_COLUMN_X, usPosY, "INTERFACE\\OptionsCheckBoxes.sti",
        MSYS_PRIORITY_HIGH + 10, BtnOptionsTogglesCallback);
    MSYS_SetBtnUserData(guiOptionsToggles[cnt], 0, cnt);

    //
    // Create mouse regions for the option toggle text
    //

    usTextWidth = StringPixLength(zOptionsToggleText[cnt], OPT_MAIN_FONT);

    if (usTextWidth > OPT_TOGGLE_BOX_TEXT_WIDTH) {
      // Get how many lines will be used to display the string, without displaying the string
      uint8_t ubNumLines =
          DisplayWrappedString(0, 0, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT,
                               OPT_HIGHLIGHT_COLOR, zOptionsToggleText[cnt], FONT_MCOLOR_BLACK,
                               TRUE, LEFT_JUSTIFIED | DONT_DISPLAY_TEXT) /
          GetFontHeight(OPT_MAIN_FONT);

      usTextWidth = OPT_TOGGLE_BOX_TEXT_WIDTH;

      MSYS_DefineRegion(&gSelectedOptionTextRegion[cnt], OPT_TOGGLE_BOX_SECOND_COLUMN_X + 13,
                        usPosY, (uint16_t)(OPT_TOGGLE_BOX_SECOND_TEXT_X + usTextWidth),
                        (uint16_t)(usPosY + usTextHeight * ubNumLines), MSYS_PRIORITY_HIGH,
                        CURSOR_NORMAL, SelectedOptionTextRegionMovementCallBack,
                        SelectedOptionTextRegionCallBack);
      MSYS_AddRegion(&gSelectedOptionTextRegion[cnt]);
      MSYS_SetRegionUserData(&gSelectedOptionTextRegion[cnt], 0, cnt);
    } else {
      MSYS_DefineRegion(&gSelectedOptionTextRegion[cnt], OPT_TOGGLE_BOX_SECOND_COLUMN_X + 13,
                        usPosY, (uint16_t)(OPT_TOGGLE_BOX_SECOND_TEXT_X + usTextWidth),
                        (uint16_t)(usPosY + usTextHeight), MSYS_PRIORITY_HIGH, CURSOR_NORMAL,
                        SelectedOptionTextRegionMovementCallBack, SelectedOptionTextRegionCallBack);
      MSYS_AddRegion(&gSelectedOptionTextRegion[cnt]);
      MSYS_SetRegionUserData(&gSelectedOptionTextRegion[cnt], 0, cnt);
    }

    SetRegionFastHelpText(&gSelectedOptionTextRegion[cnt], zOptionsScreenHelpText[cnt]);
    SetButtonFastHelpText(guiOptionsToggles[cnt], zOptionsScreenHelpText[cnt]);

    usPosY += OPT_GAP_BETWEEN_TOGGLE_BOXES;
  }

  // Create a mouse region so when the user leaves a togglebox text region we can detect it then
  // unselect the region
  MSYS_DefineRegion(&gSelectedToggleBoxAreaRegion, 0, 0, 640, 480, MSYS_PRIORITY_NORMAL,
                    CURSOR_NORMAL, SelectedToggleBoxAreaRegionMovementCallBack, MSYS_NO_CALLBACK);
  MSYS_AddRegion(&gSelectedToggleBoxAreaRegion);

  // Render the scene before adding the slider boxes
  RenderOptionsScreen();

  // Add a slider bar for the Sound Effects
  guiSoundEffectsSliderID = AddSlider(
      SLIDER_VERTICAL_STEEL, CURSOR_NORMAL, OPT_SOUND_EFFECTS_SLIDER_X, OPT_SOUND_EFFECTS_SLIDER_Y,
      OPT_SLIDER_BAR_SIZE, 127, MSYS_PRIORITY_HIGH, SoundFXSliderChangeCallBack, 0);
  AssertMsg(guiSoundEffectsSliderID, "Failed to AddSlider");
  SetSliderValue(guiSoundEffectsSliderID, GetSoundEffectsVolume());

  // Add a slider bar for the Speech
  guiSpeechSliderID =
      AddSlider(SLIDER_VERTICAL_STEEL, CURSOR_NORMAL, OPT_SPEECH_SLIDER_X, OPT_SPEECH_SLIDER_Y,
                OPT_SLIDER_BAR_SIZE, 127, MSYS_PRIORITY_HIGH, SpeechSliderChangeCallBack, 0);
  AssertMsg(guiSpeechSliderID, "Failed to AddSlider");
  SetSliderValue(guiSpeechSliderID, GetSpeechVolume());

  // Add a slider bar for the Music
  guiMusicSliderID =
      AddSlider(SLIDER_VERTICAL_STEEL, CURSOR_NORMAL, OPT_MUSIC_SLIDER_X, OPT_MUSIC_SLIDER_Y,
                OPT_SLIDER_BAR_SIZE, 127, MSYS_PRIORITY_HIGH, MusicSliderChangeCallBack, 0);
  AssertMsg(guiMusicSliderID, "Failed to AddSlider");
  SetSliderValue(guiMusicSliderID, MusicGetVolume());

  // Remove the mouse region over the clock
  RemoveMouseRegionForPauseOfClock();

  // Draw the screen
  gfRedrawOptionsScreen = TRUE;

  // Set the option screen toggle boxes
  SetOptionsScreenToggleBoxes();

  DisableScrollMessages();

  // reset
  gbHighLightedOptionText = -1;

  // get the status of the tree top option
  gfSettingOfTreeTopStatusOnEnterOfOptionScreen = gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS];

  // Get the status of the item glow option
  gfSettingOfItemGlowStatusOnEnterOfOptionScreen = gGameSettings.fOptions[TOPTION_GLOW_ITEMS];

  gfSettingOfDontAnimateSmoke = gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE];
  return (TRUE);
}

void ExitOptionsScreen() {
  uint8_t cnt;

  if (gfExitOptionsDueToMessageBox) {
    gfOptionsScreenExit = FALSE;

    if (!gfExitOptionsAfterMessageBox) return;
    gfExitOptionsAfterMessageBox = FALSE;
    gfExitOptionsDueToMessageBox = FALSE;
  }

  // Get the current status of the toggle boxes
  GetOptionsScreenToggleBoxes();
  // The save the current settings to disk
  SaveGameSettings();

  // Create the clock mouse region
  CreateMouseRegionForPauseOfClock(CLOCK_REGION_START_X, CLOCK_REGION_START_Y);

  if (guiOptionsScreen == GAME_SCREEN) EnterTacticalScreen();

  RemoveButton(guiOptGotoSaveGameBtn);
  RemoveButton(guiOptGotoLoadGameBtn);
  RemoveButton(guiQuitButton);
  RemoveButton(guiDoneButton);

  UnloadButtonImage(giOptionsButtonImages);
  UnloadButtonImage(giGotoLoadBtnImage);
  UnloadButtonImage(giQuitBtnImage);
  UnloadButtonImage(giDoneBtnImage);

  DeleteVideoObjectFromIndex(guiOptionBackGroundImage);
  DeleteVideoObjectFromIndex(guiOptionsAddOnImages);

  // Remove the toggle buttons
  for (cnt = 0; cnt < NUM_GAME_OPTIONS; cnt++) {
    // if this is the blood and gore option, and we are to hide the option
    if (cnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption) {
      // advance to the next
      continue;
    }

    RemoveButton(guiOptionsToggles[cnt]);

    MSYS_RemoveRegion(&gSelectedOptionTextRegion[cnt]);
  }

  // REmove the slider bars
  RemoveSliderBar(guiSoundEffectsSliderID);
  RemoveSliderBar(guiSpeechSliderID);
  RemoveSliderBar(guiMusicSliderID);

  MSYS_RemoveRegion(&gSelectedToggleBoxAreaRegion);

  ShutDownSlider();

  // if we are coming from mapscreen
  if (gfEnteredFromMapScreen) {
    gfEnteredFromMapScreen = FALSE;
    guiTacticalInterfaceFlags |= INTERFACE_MAPSCREEN;
  }

  // if the user changed the  TREE TOP option, AND a world is loaded
  if (gfSettingOfTreeTopStatusOnEnterOfOptionScreen !=
          gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS] &&
      gfWorldLoaded) {
    SetTreeTopStateForMap();
  }

  // if the user has changed the item glow option AND a world is loaded
  if (gfSettingOfItemGlowStatusOnEnterOfOptionScreen !=
          gGameSettings.fOptions[TOPTION_GLOW_ITEMS] &&
      gfWorldLoaded) {
    ToggleItemGlow(gGameSettings.fOptions[TOPTION_GLOW_ITEMS]);
  }

  if (gfSettingOfDontAnimateSmoke != gGameSettings.fOptions[TOPTION_ANIMATE_SMOKE] &&
      gfWorldLoaded) {
    UpdateSmokeEffectGraphics();
  }
}

void HandleOptionsScreen() {
  HandleSliderBarMovementSounds();

  HandleHighLightedText(TRUE);
}

void RenderOptionsScreen() {
  struct VObject *hPixHandle;
  uint16_t usPosY;
  uint8_t cnt;
  uint16_t usWidth = 0;

  // Get and display the background image
  GetVideoObject(&hPixHandle, guiOptionBackGroundImage);
  BltVideoObject(vsFB, hPixHandle, 0, 0, 0);

  // Get and display the titla image
  GetVideoObject(&hPixHandle, guiOptionsAddOnImages);
  BltVideoObject(vsFB, hPixHandle, 0, 0, 0);
  BltVideoObject(vsFB, hPixHandle, 1, 0, 434);

  //
  // Text for the toggle boxes
  //

  usPosY = OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y;

  // Display the First column of toggles
  for (cnt = 0; cnt < gubFirstColOfOptions; cnt++) {
    // if this is the blood and gore option, and we are to hide the option
    if (cnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption) {
      // advance to the next
      continue;
    }

    usWidth = StringPixLength(zOptionsToggleText[cnt], OPT_MAIN_FONT);

    // if the string is going to wrap, move the string up a bit
    if (usWidth > OPT_TOGGLE_BOX_TEXT_WIDTH)
      DisplayWrappedString(OPT_TOGGLE_BOX_FIRST_COL_TEXT_X, usPosY, OPT_TOGGLE_BOX_TEXT_WIDTH, 2,
                           OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsToggleText[cnt],
                           FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    else
      DrawTextToScreen(zOptionsToggleText[cnt], OPT_TOGGLE_BOX_FIRST_COL_TEXT_X, usPosY, 0,
                       OPT_MAIN_FONT, OPT_MAIN_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

    usPosY += OPT_GAP_BETWEEN_TOGGLE_BOXES;
  }

  usPosY = OPT_TOGGLE_BOX_SECOND_COLUMN_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y;
  // Display the 2nd column of toggles
  for (cnt = gubFirstColOfOptions; cnt < NUM_GAME_OPTIONS; cnt++) {
    usWidth = StringPixLength(zOptionsToggleText[cnt], OPT_MAIN_FONT);

    // if the string is going to wrap, move the string up a bit
    if (usWidth > OPT_TOGGLE_BOX_TEXT_WIDTH)
      DisplayWrappedString(OPT_TOGGLE_BOX_SECOND_TEXT_X, usPosY, OPT_TOGGLE_BOX_TEXT_WIDTH, 2,
                           OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsToggleText[cnt],
                           FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    else
      DrawTextToScreen(zOptionsToggleText[cnt], OPT_TOGGLE_BOX_SECOND_TEXT_X, usPosY, 0,
                       OPT_MAIN_FONT, OPT_MAIN_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

    usPosY += OPT_GAP_BETWEEN_TOGGLE_BOXES;
  }

  //
  // Text for the Slider Bars
  //

  // Display the Sound Fx text
  DisplayWrappedString(OPT_SOUND_FX_TEXT_X, OPT_SOUND_FX_TEXT_Y, OPT_SLIDER_TEXT_WIDTH, 2,
                       OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsText[OPT_SOUND_FX], FONT_MCOLOR_BLACK,
                       FALSE, CENTER_JUSTIFIED);

  // Display the Speech text
  DisplayWrappedString(OPT_SPEECH_TEXT_X, OPT_SPEECH_TEXT_Y, OPT_SLIDER_TEXT_WIDTH, 2,
                       OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsText[OPT_SPEECH], FONT_MCOLOR_BLACK,
                       FALSE, CENTER_JUSTIFIED);

  // Display the Music text
  DisplayWrappedString(OPT_MUSIC_TEXT_X, OPT_MUSIC_TEXT_Y, OPT_SLIDER_TEXT_WIDTH, 2, OPT_MAIN_FONT,
                       OPT_MAIN_COLOR, zOptionsText[OPT_MUSIC], FONT_MCOLOR_BLACK, FALSE,
                       CENTER_JUSTIFIED);

  InvalidateRegion(OPTIONS__TOP_LEFT_X, OPTIONS__TOP_LEFT_Y, OPTIONS__BOTTOM_RIGHT_X,
                   OPTIONS__BOTTOM_RIGHT_Y);
}

void GetOptionsScreenUserInput() {
  InputAtom Event;
  struct Point MousePos = GetMousePoint();

  while (DequeueEvent(&Event)) {
    // HOOK INTO MOUSE HOOKS
    switch (Event.usEvent) {
      case LEFT_BUTTON_DOWN:
        MouseSystemHook(LEFT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case LEFT_BUTTON_UP:
        MouseSystemHook(LEFT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case RIGHT_BUTTON_DOWN:
        MouseSystemHook(RIGHT_BUTTON_DOWN, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case RIGHT_BUTTON_UP:
        MouseSystemHook(RIGHT_BUTTON_UP, (int16_t)MousePos.x, (int16_t)MousePos.y, _LeftButtonDown,
                        _RightButtonDown);
        break;
      case RIGHT_BUTTON_REPEAT:
        MouseSystemHook(RIGHT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
      case LEFT_BUTTON_REPEAT:
        MouseSystemHook(LEFT_BUTTON_REPEAT, (int16_t)MousePos.x, (int16_t)MousePos.y,
                        _LeftButtonDown, _RightButtonDown);
        break;
    }

    if (!HandleTextInput(&Event) && Event.usEvent == KEY_DOWN) {
      switch (Event.usParam) {
        case ESC:
          SetOptionsExitScreen(guiPreviousOptionScreen);
          break;

        // Enter the save game screen
        case 's':
        case 'S':
          // if the save game button isnt disabled
          if (ButtonList[guiOptGotoSaveGameBtn]->uiFlags & BUTTON_ENABLED) {
            SetOptionsExitScreen(SAVE_LOAD_SCREEN);
            gfSaveGame = TRUE;
          }
          break;

        // Enter the Load game screen
        case 'l':
        case 'L':
          SetOptionsExitScreen(SAVE_LOAD_SCREEN);
          gfSaveGame = FALSE;
          break;

#ifdef JA2TESTVERSION

        case 'r':
          gfRedrawOptionsScreen = TRUE;
          break;

        case 'i':
          InvalidateRegion(0, 0, 640, 480);
          break;

          // Test keys

        case 'y': {
          static uint32_t uiTest2 = NO_SAMPLE;
          if (!SoundIsPlaying(uiTest2))
            uiTest2 = PlayJA2SampleFromFile("Sounds\\RAID Dive.wav", RATE_11025, HIGHVOLUME, 1,
                                            MIDDLEPAN);
        } break;
        case 't': {
        } break;
#endif
      }
    }
  }
}

void SetOptionsExitScreen(uint32_t uiExitScreen) {
  guiOptionsScreen = uiExitScreen;
  gfOptionsScreenExit = TRUE;
}

void BtnOptGotoSaveGameCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    SetOptionsExitScreen(SAVE_LOAD_SCREEN);
    gfSaveGame = TRUE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnOptGotoLoadGameCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    SetOptionsExitScreen(SAVE_LOAD_SCREEN);
    gfSaveGame = FALSE;

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnOptQuitCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    // Confirm the Exit to the main menu screen
    DoOptionsMessageBox(MSG_BOX_BASIC_STYLE, zOptionsText[OPT_RETURN_TO_MAIN], OPTIONS_SCREEN,
                        MSG_BOX_FLAG_YESNO, ConfirmQuitToMainMenuMessageBoxCallBack);

    ///		SetOptionsExitScreen( MAINMENU_SCREEN );

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnDoneCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    SetOptionsExitScreen(guiPreviousOptionScreen);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnOptionsTogglesCallback(GUI_BUTTON *btn, int32_t reason) {
  uint8_t ubButton = (uint8_t)MSYS_GetBtnUserData(btn, 0);

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      HandleOptionToggle(ubButton, TRUE, FALSE, FALSE);

      //			gGameSettings.fOptions[ ubButton ] = TRUE;
      btn->uiFlags |= BUTTON_CLICKED_ON;
    } else {
      btn->uiFlags &= ~BUTTON_CLICKED_ON;

      HandleOptionToggle(ubButton, FALSE, FALSE, FALSE);
    }
  } else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      HandleOptionToggle(ubButton, TRUE, TRUE, FALSE);

      btn->uiFlags |= BUTTON_CLICKED_ON;
    } else {
      btn->uiFlags &= ~BUTTON_CLICKED_ON;

      HandleOptionToggle(ubButton, FALSE, TRUE, FALSE);
    }
  }
}

void HandleOptionToggle(uint8_t ubButton, BOOLEAN fState, BOOLEAN fDown, BOOLEAN fPlaySound) {
  static uint32_t uiOptionToggleSound = NO_SAMPLE;

  if (fState) {
    gGameSettings.fOptions[ubButton] = TRUE;

    ButtonList[guiOptionsToggles[ubButton]]->uiFlags |= BUTTON_CLICKED_ON;

    if (fDown) DrawCheckBoxButtonOn(guiOptionsToggles[ubButton]);
  } else {
    gGameSettings.fOptions[ubButton] = FALSE;

    ButtonList[guiOptionsToggles[ubButton]]->uiFlags &= ~BUTTON_CLICKED_ON;

    if (fDown) DrawCheckBoxButtonOff(guiOptionsToggles[ubButton]);

    // check to see if the user is unselecting either the spech or subtitles toggle
    if (ubButton == TOPTION_SPEECH || ubButton == TOPTION_SUBTITLES) {
      // make sure that at least of of the toggles is still enabled
      if (!(ButtonList[guiOptionsToggles[TOPTION_SPEECH]]->uiFlags & BUTTON_CLICKED_ON)) {
        if (!(ButtonList[guiOptionsToggles[TOPTION_SUBTITLES]]->uiFlags & BUTTON_CLICKED_ON)) {
          gGameSettings.fOptions[ubButton] = TRUE;
          ButtonList[guiOptionsToggles[ubButton]]->uiFlags |= BUTTON_CLICKED_ON;

          // Confirm the Exit to the main menu screen
          DoOptionsMessageBox(MSG_BOX_BASIC_STYLE,
                              zOptionsText[OPT_NEED_AT_LEAST_SPEECH_OR_SUBTITLE_OPTION_ON],
                              OPTIONS_SCREEN, MSG_BOX_FLAG_OK, NULL);
          gfExitOptionsDueToMessageBox = FALSE;
        }
      }
    }
  }

  // stop the sound if
  //	if( SoundIsPlaying( uiOptionToggleSound ) && !fDown )
  {
    SoundStop(uiOptionToggleSound);
  }

  if (fPlaySound) {
    if (fDown) {
      //				case BTN_SND_CLICK_OFF:
      PlayJA2Sample(BIG_SWITCH3_IN, RATE_11025, BTNVOLUME, 1, MIDDLEPAN);
    } else {
      //		case BTN_SND_CLICK_ON:
      PlayJA2Sample(BIG_SWITCH3_OUT, RATE_11025, BTNVOLUME, 1, MIDDLEPAN);
    }
  }
}

void SoundFXSliderChangeCallBack(int32_t iNewValue) {
  SetSoundEffectsVolume(iNewValue);

  guiSoundFxSliderMoving = GetJA2Clock();
}

void SpeechSliderChangeCallBack(int32_t iNewValue) {
  SetSpeechVolume(iNewValue);

  guiSpeechSliderMoving = GetJA2Clock();
}

void MusicSliderChangeCallBack(int32_t iNewValue) { MusicSetVolume(iNewValue); }

BOOLEAN DoOptionsMessageBoxWithRect(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                                    uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback,
                                    const SGPRect *pCenteringRect) {
  // reset exit mode
  gfExitOptionsDueToMessageBox = TRUE;

  // do message box and return
  giOptionsMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                     (uint16_t)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                     ReturnCallback, pCenteringRect);

  // send back return state
  return ((giOptionsMessageBox != -1));
}

BOOLEAN DoOptionsMessageBox(uint8_t ubStyle, wchar_t *zString, uint32_t uiExitScreen,
                            uint16_t usFlags, MSGBOX_CALLBACK ReturnCallback) {
  SGPRect CenteringRect = {0, 0, 639, 479};

  // reset exit mode
  gfExitOptionsDueToMessageBox = TRUE;

  // do message box and return
  giOptionsMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen,
                                     (uint16_t)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT),
                                     ReturnCallback, &CenteringRect);

  // send back return state
  return ((giOptionsMessageBox != -1));
}

void ConfirmQuitToMainMenuMessageBoxCallBack(uint8_t bExitValue) {
  // yes, Quit to main menu
  if (bExitValue == MSG_BOX_RETURN_YES) {
    gfEnteredFromMapScreen = FALSE;
    gfExitOptionsAfterMessageBox = TRUE;
    SetOptionsExitScreen(MAINMENU_SCREEN);

    // We want to reinitialize the game
    ReStartingGame();
  } else {
    gfExitOptionsAfterMessageBox = FALSE;
    gfExitOptionsDueToMessageBox = FALSE;
  }
}

void SetOptionsScreenToggleBoxes() {
  uint8_t cnt;

  for (cnt = 0; cnt < NUM_GAME_OPTIONS; cnt++) {
    if (gGameSettings.fOptions[cnt])
      ButtonList[guiOptionsToggles[cnt]]->uiFlags |= BUTTON_CLICKED_ON;
    else
      ButtonList[guiOptionsToggles[cnt]]->uiFlags &= (~BUTTON_CLICKED_ON);
  }
}

void GetOptionsScreenToggleBoxes() {
  uint8_t cnt;

  for (cnt = 0; cnt < NUM_GAME_OPTIONS; cnt++) {
    if (ButtonList[guiOptionsToggles[cnt]]->uiFlags & BUTTON_CLICKED_ON)
      gGameSettings.fOptions[cnt] = TRUE;
    else
      gGameSettings.fOptions[cnt] = FALSE;
  }
}

void HandleSliderBarMovementSounds() {
  static uint32_t uiLastSoundFxTime = 0;
  static uint32_t uiLastSpeechTime = 0;
  static uint32_t uiLastPlayingSoundID = NO_SAMPLE;
  static uint32_t uiLastPlayingSpeechID = NO_SAMPLE;

  if ((uiLastSoundFxTime - OPT_MUSIC_SLIDER_PLAY_SOUND_DELAY) > guiSoundFxSliderMoving) {
    guiSoundFxSliderMoving = 0xffffffff;

    // The slider has stopped moving, reset the ambient sector sounds ( so it will change the volume
    // )
    if (!DidGameJustStart()) HandleNewSectorAmbience(gTilesets[giCurrentTilesetID].ubAmbientID);

    if (!SoundIsPlaying(uiLastPlayingSoundID))
      uiLastPlayingSoundID = PlayJA2SampleFromFile("Sounds\\Weapons\\LMG Reload.wav", RATE_11025,
                                                   HIGHVOLUME, 1, MIDDLEPAN);
  } else
    uiLastSoundFxTime = GetJA2Clock();

  if ((uiLastSpeechTime - OPT_MUSIC_SLIDER_PLAY_SOUND_DELAY) > guiSpeechSliderMoving) {
    guiSpeechSliderMoving = 0xffffffff;

    if (!SoundIsPlaying(uiLastPlayingSpeechID))
      uiLastPlayingSpeechID =
          PlayJA2GapSample("BattleSnds\\m_cool.wav", RATE_11025, HIGHVOLUME, 1, MIDDLEPAN, NULL);
  } else
    uiLastSpeechTime = GetJA2Clock();
}

void SelectedOptionTextRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  uint8_t ubButton = (uint8_t)MSYS_GetRegionUserData(pRegion, 0);

  if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    HandleOptionToggle(ubButton, (BOOLEAN)(!gGameSettings.fOptions[ubButton]), FALSE, TRUE);

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }

  else if (iReason &
           MSYS_CALLBACK_REASON_LBUTTON_DWN)  // iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT ||
  {
    if (gGameSettings.fOptions[ubButton]) {
      HandleOptionToggle(ubButton, TRUE, TRUE, TRUE);
    } else {
      HandleOptionToggle(ubButton, FALSE, TRUE, TRUE);
    }
  }
}

void SelectedOptionTextRegionMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  int8_t bButton = (int8_t)MSYS_GetRegionUserData(pRegion, 0);

  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    HandleHighLightedText(FALSE);

    gbHighLightedOptionText = -1;

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    gbHighLightedOptionText = bButton;

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}

void HandleHighLightedText(BOOLEAN fHighLight) {
  uint16_t usPosX = 0;
  uint16_t usPosY = 0;
  uint8_t ubCnt;
  int8_t bHighLight = -1;
  uint16_t usWidth;

  static int8_t bLastRegion = -1;

  if (gbHighLightedOptionText == -1) fHighLight = FALSE;

  // if the user has the mouse in one of the checkboxes
  for (ubCnt = 0; ubCnt < NUM_GAME_OPTIONS; ubCnt++) {
    if (ubCnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption) {
      // advance to the next
      continue;
    }

    if (ButtonList[guiOptionsToggles[ubCnt]]->Area.uiFlags & MSYS_MOUSE_IN_AREA) {
      gbHighLightedOptionText = ubCnt;
      fHighLight = TRUE;
    }
  }

  // If there is a valid section being highlighted
  if (gbHighLightedOptionText != -1) {
    bLastRegion = gbHighLightedOptionText;
  }

  bHighLight = gbHighLightedOptionText;

  if (bLastRegion != -1 && gbHighLightedOptionText == -1) {
    fHighLight = FALSE;
    bHighLight = bLastRegion;
    bLastRegion = -1;
  }

  // If we are to hide the blood and gore option, and we are to highlight an option past the blood
  // and gore option reduce the highlight number by 1
  if (bHighLight >= TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption) {
    bHighLight--;
  }

  if (bHighLight != -1) {
    if (bHighLight < OPT_FIRST_COLUMN_TOGGLE_CUT_OFF) {
      usPosX = OPT_TOGGLE_BOX_FIRST_COL_TEXT_X;
      usPosY = OPT_TOGGLE_BOX_FIRST_COLUMN_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y +
               (bHighLight * OPT_GAP_BETWEEN_TOGGLE_BOXES);
    } else {
      usPosX = OPT_TOGGLE_BOX_SECOND_TEXT_X;
      usPosY = OPT_TOGGLE_BOX_SECOND_COLUMN_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y +
               ((bHighLight - OPT_FIRST_COLUMN_TOGGLE_CUT_OFF) * OPT_GAP_BETWEEN_TOGGLE_BOXES);
    }

    // If we are to hide the blood and gore option, and we are to highlight an option past the blood
    // and gore option reduce the highlight number by 1
    if (bHighLight >= TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption) {
      bHighLight++;
    }

    usWidth = StringPixLength(zOptionsToggleText[bHighLight], OPT_MAIN_FONT);

    // if the string is going to wrap, move the string up a bit
    if (usWidth > OPT_TOGGLE_BOX_TEXT_WIDTH) {
      if (fHighLight)
        DisplayWrappedString(usPosX, usPosY, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT,
                             OPT_HIGHLIGHT_COLOR, zOptionsToggleText[bHighLight], FONT_MCOLOR_BLACK,
                             TRUE, LEFT_JUSTIFIED);
      //				DrawTextToScreen( zOptionsToggleText[ bHighLight ], usPosX,
      // usPosY, 0, OPT_MAIN_FONT, OPT_HIGHLIGHT_COLOR, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED	);
      else
        DisplayWrappedString(usPosX, usPosY, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT,
                             OPT_MAIN_COLOR, zOptionsToggleText[bHighLight], FONT_MCOLOR_BLACK,
                             TRUE, LEFT_JUSTIFIED);
      //				DrawTextToScreen( zOptionsToggleText[ bHighLight ], usPosX,
      // usPosY, 0, OPT_MAIN_FONT, OPT_MAIN_COLOR, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED	);
    } else {
      if (fHighLight)
        DrawTextToScreen(zOptionsToggleText[bHighLight], usPosX, usPosY, 0, OPT_MAIN_FONT,
                         OPT_HIGHLIGHT_COLOR, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED);
      else
        DrawTextToScreen(zOptionsToggleText[bHighLight], usPosX, usPosY, 0, OPT_MAIN_FONT,
                         OPT_MAIN_COLOR, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED);
    }
  }
}

void SelectedToggleBoxAreaRegionMovementCallBack(struct MOUSE_REGION *pRegion, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
  } else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    uint8_t ubCnt;

    // loop through all the toggle box's and remove the in area flag
    for (ubCnt = 0; ubCnt < NUM_GAME_OPTIONS; ubCnt++) {
      ButtonList[guiOptionsToggles[ubCnt]]->Area.uiFlags &= ~MSYS_MOUSE_IN_AREA;
    }

    gbHighLightedOptionText = -1;

    InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX,
                     pRegion->RegionBottomRightY);
  }
}
