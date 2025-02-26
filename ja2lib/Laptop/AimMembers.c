// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Laptop/AIMMembers.h"

#include <stdio.h>

#include "GameRes.h"
#include "GameSettings.h"
#include "Laptop/AIM.h"
#include "Laptop/AIMFacialIndex.h"
#include "Laptop/Email.h"
#include "Laptop/Finances.h"
#include "Laptop/History.h"
#include "Laptop/Laptop.h"
#include "Laptop/LaptopSave.h"
#include "Money.h"
#include "SGP/ButtonSystem.h"
#include "SGP/Debug.h"
#include "SGP/English.h"
#include "SGP/FileMan.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/VObject.h"
#include "SGP/VSurface.h"
#include "SGP/Video.h"
#include "SGP/WCheck.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/Assignments.h"
#include "Strategic/GameClock.h"
#include "Strategic/GameEventHook.h"
#include "Strategic/MercContract.h"
#include "Strategic/Quests.h"
#include "Strategic/Strategic.h"
#include "Strategic/StrategicMap.h"
#include "Strategic/StrategicMercHandler.h"
#include "Strategic/StrategicStatus.h"
#include "Strategic/StrategicTownLoyalty.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/Faces.h"
#include "Tactical/InterfaceItems.h"
#include "Tactical/Menptr.h"
#include "Tactical/MercHiring.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierAdd.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierProfile.h"
#include "TileEngine/RenderDirty.h"
#include "TileEngine/SysUtil.h"
#include "Utils/EncryptedFile.h"
#include "Utils/MercTextBox.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"
#include "Utils/Utilities.h"
#include "Utils/WordWrap.h"

//
//******  Defines  ******
//

#define MERCBIOSFILENAME "BINARYDATA\\aimbios.edt"

#define AIM_M_FONT_PREV_NEXT_CONTACT FONT14ARIAL
#define AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_UP FONT_MCOLOR_DKWHITE
#define AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_DOWN 138
#define AIM_M_FONT_STATIC_TEXT FONT12ARIAL
#define AIM_M_COLOR_STATIC_TEXT 146  // 75
#define AIM_M_FONT_DYNAMIC_TEXT FONT10ARIAL
#define AIM_M_COLOR_DYNAMIC_TEXT FONT_MCOLOR_WHITE
#define AIM_M_WEAPON_TEXT_FONT FONT10ARIAL
#define AIM_M_WEAPON_TEXT_COLOR FONT_MCOLOR_WHITE
#define AIM_M_NUMBER_FONT FONT12ARIAL
#define AIM_M_NUMBER_COLOR FONT_MCOLOR_WHITE
#define AIM_M_ACTIVE_MEMBER_TITLE_COLOR AIM_GREEN
#define AIM_M_FEE_CONTRACT_COLOR AIM_GREEN
#define AIM_M_VIDEO_TITLE_COLOR AIM_FONT_GOLD
#define AIM_M_VIDEO_NAME_COLOR FONT_MCOLOR_BLACK
#define AIM_M_VIDEO_NAME_SHADOWCOLOR AIM_FONT_GOLD

#define AIM_M_VIDEO_CONTRACT_LENGTH_FONT FONT12ARIAL
#define AIM_M_VIDEO_CONTRACT_LENGTH_COLOR FONT_MCOLOR_BLACK

#define AIM_M_VIDEO_CONTRACT_AMOUNT_FONT FONT10ARIAL
#define AIM_M_VIDEO_CONTRACT_AMOUNT_COLOR 183

#define AIM_POPUP_BOX_FONT FONT12ARIAL
#define AIM_POPUP_BOX_COLOR FONT_MCOLOR_BLACK

#define HIGH_STAT_COLOR FONT_MCOLOR_WHITE   // FONT_MCOLOR_LTGREEN
#define MED_STAT_COLOR FONT_MCOLOR_DKWHITE  // FONT_MCOLOR_WHITE
#define LOW_STAT_COLOR FONT_MCOLOR_DKWHITE  // FONT_MCOLOR_DKGRAY

#define SIZE_MERC_BIO_INFO 400 * 2
#define SIZE_MERC_ADDITIONAL_INFO 160 * 2

#define MERC_ANNOYED_WONT_CONTACT_TIME_MINUTES 6 * 60
#define NUMBER_HATED_MERCS_ONTEAM 3

#define STATS_X IMAGE_OFFSET_X + 121
#define STATS_Y IMAGE_OFFSET_Y + 66  // 69

#define PRICE_X IMAGE_OFFSET_X + 377
#define PRICE_Y STATS_Y
#define PRICE_WIDTH 116

#define PORTRAIT_X IMAGE_OFFSET_X + 8
#define PORTRAIT_Y STATS_Y
#define PORTRAIT_WIDTH 110
#define PORTRAIT_HEIGHT 126

#define FACE_X PORTRAIT_X + 2
#define FACE_Y PORTRAIT_Y + 2
#define FACE_WIDTH 106
#define FACE_HEIGHT 122

#define WEAPONBOX_X IMAGE_OFFSET_X + 6
#define WEAPONBOX_Y IMAGE_OFFSET_Y + 296  // 299
#define WEAPONBOX_SIZE_X 61
#define WEAPONBOX_SIZE_Y 31
#define WEAPONBOX_NUMBER 8

#define SPACE_BN_LINES 15  // 13
#define STATS_FIRST_COL STATS_X + 9
#define STATS_SECOND_COL STATS_FIRST_COL + 129
#define STATS_FIRST_NUM STATS_X + 111  // 91
#define STATS_SECOND_NUM STATS_X + 235

#define HEALTH_Y STATS_Y + 34
#define AGILITY_Y HEALTH_Y + SPACE_BN_LINES
#define DEXTERITY_Y AGILITY_Y + SPACE_BN_LINES
#define STRENGTH_Y DEXTERITY_Y + SPACE_BN_LINES
#define LEADERSHIP_Y STRENGTH_Y + SPACE_BN_LINES
#define WISDOM_Y LEADERSHIP_Y + SPACE_BN_LINES

#define EXPLEVEL_Y HEALTH_Y
#define MARKSMAN_Y AGILITY_Y
#define MECHANAICAL_Y DEXTERITY_Y
#define EXPLOSIVE_Y STRENGTH_Y
#define MEDICAL_Y LEADERSHIP_Y

#define NAME_X STATS_FIRST_COL
#define NAME_Y STATS_Y + 7

#define FEE_X PRICE_X + 7
#define FEE_Y NAME_Y
#define FEE_WIDTH 37  // 33

#define AIM_CONTRACT_X PRICE_X + 51
#define AIM_CONTRACT_Y FEE_Y
#define AIM_CONTRACT_WIDTH 59

#define ONEDAY_X AIM_CONTRACT_X
#define ONEWEEK_X AIM_CONTRACT_X
#define TWOWEEK_X AIM_CONTRACT_X

#define PREVIOUS_X 224
#define PREVIOUS_Y 386 + LAPTOP_SCREEN_WEB_DELTA_Y
#define PREVIOUS_BOX_Y PREVIOUS_Y - 4
#define PREVIOUS_BR_X PREVIOUS_X + BOTTOM_BUTTON_START_WIDTH
#define PREVIOUS_BR_Y PREVIOUS_BOX_Y + BOTTOM_BUTTON_START_HEIGHT

#define CONTACT_X 331
#define CONTACT_Y PREVIOUS_Y
#define CONTACT_BOX_Y CONTACT_Y - 4
#define CONTACT_BOX_WIDTH 75
#define CONTACT_BOX_HEIGHT 18
#define CONTACT_BR_X CONTACT_X + BOTTOM_BUTTON_START_WIDTH
#define CONTACT_BR_Y CONTACT_BOX_Y + BOTTOM_BUTTON_START_HEIGHT

#define NEXT_X 431
#define NEXT_Y PREVIOUS_Y
#define NEXT_BOX_Y NEXT_Y - 4
#define NEXT_BR_X NEXT_X + BOTTOM_BUTTON_START_WIDTH
#define NEXT_BR_Y NEXT_BOX_Y + BOTTOM_BUTTON_START_HEIGHT

#define AIM_MERC_INFO_X 124
#define AIM_MERC_INFO_Y 223 + LAPTOP_SCREEN_WEB_DELTA_Y

#define AIM_MERC_ADD_X AIM_MERC_ADD_INFO_X
#define AIM_MERC_ADD_Y 269 + LAPTOP_SCREEN_WEB_DELTA_Y

#define AIM_MERC_ADD_INFO_X AIM_MERC_INFO_X
#define AIM_MERC_ADD_INFO_Y AIM_MERC_ADD_Y + 15
#define AIM_MERC_INFO_WIDTH 470

#define AIM_MEDICAL_DEPOSIT_X PRICE_X + 5
#define AIM_MEDICAL_DEPOSIT_Y LEADERSHIP_Y
#define AIM_MEDICAL_DEPOSIT_WIDTH PRICE_WIDTH - 6

#define AIM_MEMBER_ACTIVE_TEXT_X IMAGE_OFFSET_X + 149
#define AIM_MEMBER_ACTIVE_TEXT_Y AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y - 1  // + 1
#define AIM_MEMBER_ACTIVE_TEXT_WIDTH AIM_SYMBOL_WIDTH

#define AIM_MEMBER_OPTIONAL_GEAR_X AIM_MERC_INFO_X
#define AIM_MEMBER_OPTIONAL_GEAR_Y WEAPONBOX_Y - 13
// #define		AIM_MEMBER_OPTIONAL_GEAR_NUMBER_X		AIM_MEMBER_OPTIONAL_GEAR_X

#define AIM_MEMBER_WEAPON_NAME_X WEAPONBOX_X
#define AIM_MEMBER_WEAPON_NAME_Y WEAPONBOX_Y + WEAPONBOX_SIZE_Y + 1
#define AIM_MEMBER_WEAPON_NAME_WIDTH WEAPONBOX_SIZE_X - 2
/*
#define		AIM_MEMBER_PREVIOUS 0
#define		AIM_MEMBER_CONTACT	1
#define		AIM_MEMBER_NEXT			2
*/

// video Conferencing Info
#define AIM_MEMBER_VIDEO_CONF_TERMINAL_X 125
#define AIM_MEMBER_VIDEO_CONF_TERMINAL_Y 97 + LAPTOP_SCREEN_WEB_DELTA_Y

#define AIM_MEMBER_VIDEO_CONF_TERMINAL_WIDTH 368
#define AIM_MEMBER_VIDEO_CONF_TERMINAL_HEIGHT 150

#define AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH 368
#define AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT 21
#define AIM_MEMBER_VIDEO_TITLE_ITERATIONS 18
#define AIM_MEMBER_VIDEO_TITLE_START_Y 382 + LAPTOP_SCREEN_WEB_DELTA_Y
#define AIM_MEMBER_VIDEO_TITLE_END_Y 96
#define AIM_MEMBER_VIDEO_TITLE_START_X 330
#define AIM_MEMBER_VIDEO_TITLE_END_X 125

#define AIM_MEMBER_VIDEO_CONF_TERMINAL_RIGHT \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_X + AIM_MEMBER_VIDEO_CONF_TERMINAL_WIDTH
#define AIM_MEMBER_VIDEO_CONF_TERMINAL_BOTTOM \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TERMINAL_HEIGHT

#define AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 6
#define AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_Y AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + 130

#define AIM_MEMBER_VIDEO_CONF_XCLOSE_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 348
#define AIM_MEMBER_VIDEO_CONF_XCLOSE_Y AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + 3

#define AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y 20

#define AIM_MEMBER_BUY_CONTRACT_LENGTH_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 113
#define AIM_MEMBER_BUY_CONTRACT_LENGTH_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 15

#define AIM_MEMBER_BUY_EQUIPMENT_GAP 23

#define AIM_MEMBER_BUY_EQUIPMENT_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 235

#define AIM_MEMBER_AUTHORIZE_PAY_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 113
#define AIM_MEMBER_AUTHORIZE_PAY_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 92
#define AIM_MEMBER_AUTHORIZE_PAY_WIDTH 116
#define AIM_MEMBER_AUTHORIZE_PAY_GAP 122

#define AIM_MEMBER_VIDEO_FACE_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 7 + 1
#define AIM_MEMBER_VIDEO_FACE_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 6 + 1

#define AIM_MEMBER_VIDEO_FACE_WIDTH 96
#define AIM_MEMBER_VIDEO_FACE_HEIGHT 86

#define AIM_MEMBER_VIDEO_NAME_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 7
#define AIM_MEMBER_VIDEO_NAME_Y AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + 5

#define AIM_CONTRACT_CHARGE_X AIM_MEMBER_VIDEO_NAME_X
#define AIM_CONTRACT_CHARGE_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 98

#define AIM_CONTRACT_LENGTH_ONE_DAY 0
#define AIM_CONTRACT_LENGTH_ONE_WEEK 1
#define AIM_CONTRACT_LENGTH_TWO_WEEKS 2

#define AIM_SELECT_LIGHT_ON_X 105
#define AIM_SELECT_LIGHT_ON_Y 8

#define AIM_SELECT_LIGHT_OFF_X 105
#define AIM_SELECT_LIGHT_OFF_Y 7

#define AIM_CONTRACT_CHARGE_AMOUNNT_X AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 7  // 8
#define AIM_CONTRACT_CHARGE_AMOUNNT_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 111  // 114
#define AIM_CONTRACT_CHARGE_AMOUNNT_WIDTH 98
#define AIM_CONTRACT_CHARGE_AMOUNNT_HEIGHT 12

#define AIM_POPUP_BOX_X 260
#define AIM_POPUP_BOX_Y 140 + LAPTOP_SCREEN_WEB_DELTA_Y

#define AIM_POPUP_BOX_WIDTH 162
#define AIM_POPUP_BOX_HEIGHT 100
#define AIM_POPUP_BOX_STRING1_Y 6
#define AIM_POPUP_BOX_BUTTON_OFFSET_X 20
#define AIM_POPUP_BOX_BUTTON_OFFSET_Y 62
#define AIM_POPUP_BOX_SUCCESS 0
#define AIM_POPUP_BOX_FAILURE 1

#define AIM_MEMBER_HANG_UP_X 290
#define AIM_MEMBER_HANG_UP_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 42

#define AIM_MEMBER_VIDEO_TALKING_TEXT_X AIM_MEMBER_AUTHORIZE_PAY_X
#define AIM_MEMBER_VIDEO_TALKING_TEXT_Y \
  AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 30
#define AIM_MEMBER_VIDEO_TALKING_TEXT_WIDTH 240

#define VC_CONTACT_STATIC_TIME 30
#define VC_CONTACT_FUZZY_LINE_TIME 100
#define VC_NUM_LINES_SNOW 6
#define VC_NUM_FUZZ_LINES 10
#define VC_NUM_STRAIGHT_LINES 9

#define VC_ANSWER_IMAGE_DELAY 100

#define QUOTE_FIRST_ATTITUDE_TIME 3000
#define QUOTE_ATTITUDE_TIME 10000

#define QUOTE_DELAY_SMALL_TALK 1
#define QUOTE_DELAY_IMPATIENT_TALK 2
#define QUOTE_DELAY_VERY_IMPATIENT_TALK 3
#define QUOTE_DELAY_HANGUP_TALK 4
#define QUOTE_DELAY_NO_ACTION 5
#define QUOTE_MERC_BUSY 6

#define TEXT_POPUP_WINDOW_X 180
#define TEXT_POPUP_WINDOW_Y 255 + LAPTOP_SCREEN_WEB_DELTA_Y
#define TEXT_POPUP_STRING_SIZE 512

#define FIRST_COLUMN_DOT 328  // 308
#define SECOND_COLUMN_DOT 451

#define MINIMUM_TALKING_TIME_FOR_MERC 1500

#define AIM_TEXT_SPEECH_MODIFIER 80

#define AIM_WEAPONBOX_NAME_WIDTH 93

// enumerated types used for the Video Conferencing Display
enum {
  AIM_VIDEO_NOT_DISPLAYED_MODE,  // The video popup is not displayed
  AIM_VIDEO_POPUP_MODE,          // The title bar pops up out of the Contact button
  AIM_VIDEO_INIT_MODE,  // When the player first tries to contact the merc, it will be snowy for a
                        // bit
  AIM_VIDEO_FIRST_CONTACT_MERC_MODE,  // The popup that is displayed when first contactinf the merc
  AIM_VIDEO_HIRE_MERC_MODE,  // The popup which deals with the contract length, and transfer funds
  AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE,  // The popup which will be instread of the
                                          // AIM_VIDEO_FIRST_CONTACT_MERC_MODE if the merc is not
                                          // there
  AIM_VIDEO_MERC_UNAVAILABLE_MODE,        // The popup which will be instread of the
                                    // AIM_VIDEO_FIRST_CONTACT_MERC_MODE if the merc is unavailable
  AIM_VIDEO_POPDOWN_MODE,  // The title bars pops down to the contact button
};

// Enumerated types used for the Pop Up Box
enum {
  AIM_POPUP_NOTHING,
  AIM_POPUP_CREATE,
  AIM_POPUP_DISPLAY,
  AIM_POPUP_DELETE,
};

// Enumerated Types used for the different types of video distortion applied to the video face
enum {
  VC_NO_STATIC,
  VC_FUZZY_LINE,
  VC_STRAIGHTLINE,
  VC_STATIC_IMAGE,
  VC_BW_SNOW,
  VC_PIXELATE,
  VC_TRANS_SNOW_IN,   // fade from clear to snowy
  VC_TRANS_SNOW_OUT,  // fade from snowy to clear
};

// Image Identifiers
uint32_t guiStats;
uint32_t guiPrice;
uint32_t guiPortrait;
uint32_t guiWeaponBox;
uint32_t guiFace;
// uint32_t		guiVideoFace;
// uint32_t		guiContactButton;
uint32_t guiVideoConfPopup;
uint32_t guiVideoConfTerminal;
uint32_t guiPopUpBox;
static struct JSurface *vsVideoFaceBackground;
uint32_t guiBWSnow;
uint32_t guiFuzzLine;
uint32_t guiStraightLine;
uint32_t guiTransSnow;
uint32_t guiVideoContractCharge;
// uint32_t		guiAnsweringMachineImage;
static struct JSurface *vsVideoTitleBar;
int32_t iAimMembersBoxId = -1;

uint8_t gbCurrentSoldier = 0;
uint8_t gbCurrentIndex = 0;

uint8_t gubVideoConferencingMode;
uint8_t gubVideoConferencingPreviousMode;
BOOLEAN gfJustSwitchedVideoConferenceMode;

BOOLEAN gfMercIsTalking = FALSE;
BOOLEAN gfVideoFaceActive = FALSE;

uint8_t gubPopUpBoxAction = AIM_POPUP_NOTHING;
BOOLEAN gfRedrawScreen = FALSE;
static uint8_t gubContractLength;
BOOLEAN gfBuyEquipment;
int32_t giContractAmount = 0;
int32_t giMercFaceIndex;
wchar_t gsTalkingMercText[TEXT_POPUP_STRING_SIZE];
uint32_t guiTimeThatMercStartedTalking;
uint32_t guiLastHandleMercTime;
BOOLEAN gfFirstTimeInContactScreen;

uint8_t gubCurrentCount;
uint8_t gubCurrentStaticMode;
uint32_t guiMercAttitudeTime;  // retains the amount of time the user is in a screen, if over a
                               // certain time, the merc gets miffed
uint8_t gubMercAttitudeLevel;  // retains the current level the merc is  P.O.'ed at the caller.
BOOLEAN
gfHangUpMerc;  // if we have to cancel the video conferencing after the merc is finsihed talking
BOOLEAN gfIsShutUpMouseRegionActive;
BOOLEAN gfIsAnsweringMachineActive;
BOOLEAN gfRenderTopLevel;
BOOLEAN gfStopMercFromTalking;

uint16_t usAimMercSpeechDuration = 0;

BOOLEAN gfIsNewMailFlagSet = FALSE;

extern uint8_t gubBasicInventoryPositions[];
extern BOOLEAN fExitDueToMessageBox;

BOOLEAN gfWaitingForMercToStopTalkingOrUserToClick = FALSE;

int32_t giIdOfLastHiredMerc = -1;

BOOLEAN gfAimMemberDisplayFaceHelpText = FALSE;

BOOLEAN gfAimMemberCanMercSayOpeningQuote = TRUE;

////////////////////////////////////////////////////////////////
//
//	Mouse and Buttons
//
////////////////////////////////////////////////////////////////

// Graphic for following
int32_t guiPreviousContactNextButtonImage;
// Previous Button
void BtnPreviousButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giPreviousButton;

// Contact
void BtnContactButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giContactButton;

// NExt
void BtnNextButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giNextButton;

// Video conference buttons
int32_t guiVideoConferenceButtonImage[3];

// Contract Length Button
void BtnContractLengthButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giContractLengthButton[3];

// BuyEquipment Button
void BtnBuyEquipmentButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giBuyEquipmentButton[2];

// Authorize Payment Button
void BtnAuthorizeButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giAuthorizeButton[2];

// Hang up Button
void BtnHangUpButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giHangUpButton;

// PopupBox button
void BtnPopUpOkButtonCallback(GUI_BUTTON *btn, int32_t reason);
uint32_t guiPopUpOkButton;
int32_t guiPopUpImage;

// First Contact Screen, Goto Hire merc Button
void BtnFirstContactButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giFirstContactButton[2];

// Leave Message merc Button
void BtnAnsweringMachineButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giAnsweringMachineButton[2];

// X to Close the video conference Button
int32_t giXToCloseVideoConfButtonImage;
void BtnXToCloseVideoConfButtonCallback(GUI_BUTTON *btn, int32_t reason);
int32_t giXToCloseVideoConfButton;

// Mouse Regions
// Clicking on guys Face
struct MOUSE_REGION gSelectedFaceRegion;
void SelectFaceRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);
void SelectFaceMovementRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

// Clicking To shut merc up
struct MOUSE_REGION gSelectedShutUpMercRegion;
void SelectShutUpMercRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason);

//*******************************************
//
//	Function Prototypes
//
//*******************************************

BOOLEAN UpdateMercInfo(void);
BOOLEAN LoadMercBioInfo(uint8_t ubIndex, wchar_t *pInfoString, wchar_t *pAddInfo);
BOOLEAN DisplayMercsInventory(uint8_t ubMercID);
BOOLEAN DisplayMercsFace();
void DisplayMercStats();
int8_t AimMemberHireMerc();
BOOLEAN DisplayVideoConferencingDisplay();
BOOLEAN DisplayMercsVideoFace();
void DisplaySelectLights(BOOLEAN fContractDown, BOOLEAN fBuyEquipDown);
uint32_t DisplayMercChargeAmount();
BOOLEAN InitCreateDeleteAimPopUpBox(uint8_t ubFlag, wchar_t *sString1, wchar_t *sString2,
                                    uint16_t usPosX, uint16_t usPosY, uint8_t ubData);
BOOLEAN InitVideoFaceTalking(uint8_t ubMercID, uint16_t usQuoteNum);
BOOLEAN InitVideoFace(uint8_t ubMercID);
BOOLEAN DisplaySnowBackground();
uint8_t WillMercAcceptCall();
void HandleVideoDistortion();
uint8_t DisplayDistortionLine(uint8_t ubMode, uint32_t uiImageIdentifier, uint8_t ubMaxImages);
uint8_t DisplayPixelatedImage(uint8_t ubMaxImages);
void HandleMercAttitude();
void StopMercTalking();
uint8_t DisplayTransparentSnow(uint8_t ubMode, uint32_t uiImageIdentifier, uint8_t ubMaxImages,
                               BOOLEAN bForward);

BOOLEAN InitDeleteVideoConferencePopUp();
BOOLEAN DeleteVideoConfPopUp();
BOOLEAN HandleCurrentVideoConfMode();

BOOLEAN EnableDisableCurrentVideoConferenceButtons(BOOLEAN fEnable);

// BOOLEAN DisplayAnimatedAnsweringMachineMsg( BOOLEAN fInit, uint8_t ubNumSubImages);
// BOOLEAN HandleAnsweringMachineMessage();

BOOLEAN CanMercBeHired();
BOOLEAN DisplayMovingTitleBar(BOOLEAN fForward, BOOLEAN fInit);
BOOLEAN DisplayBlackBackground(uint8_t ubMaxNumOfLoops);
void DisplayDots(uint16_t usNameX, uint16_t usNameY, uint16_t usStatX, wchar_t *pString);

void DelayMercSpeech(uint8_t ubMercID, uint16_t usQuoteNum, uint16_t usDelay, BOOLEAN fNewQuote,
                     BOOLEAN fReset);
void DisplayPopUpBoxExplainingMercArrivalLocationAndTimeCallBack(uint8_t bExitValue);
void DisplayAimMemberClickOnFaceHelpText();

// ppp

uint8_t GetStatColor(int8_t bStat);

#ifdef JA2TESTVERSION
BOOLEAN QuickHireMerc();
void TempHandleAimMemberKeyBoardInput();
extern void SetFlagToForceHireMerc(BOOLEAN fForceHire);
#endif

void WaitForMercToFinishTalkingOrUserToClick();

//*******************************************
//
//	FUNCTIONS
//
//*******************************************

void GameInitAIMMembers() {}

void EnterInitAimMembers() {
  gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
  gubVideoConferencingPreviousMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
  gfVideoFaceActive = FALSE;
  // fShouldMercTalk = FALSE;
  gubPopUpBoxAction = AIM_POPUP_NOTHING;
  gfRedrawScreen = FALSE;
  giContractAmount = 0;
  giMercFaceIndex = 0;
  guiLastHandleMercTime = GetJA2Clock();
  gubCurrentCount = 0;
  gfFirstTimeInContactScreen = TRUE;

  // reset the variable so a pop up can be displyed this time in laptop
  LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop = FALSE;

  // reset the id of the last merc
  LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;
}

BOOLEAN EnterAIMMembers() {
  // Create a background video surface to blt the face onto
  vsVideoFaceBackground =
      JSurface_Create16bpp(AIM_MEMBER_VIDEO_FACE_WIDTH, AIM_MEMBER_VIDEO_FACE_HEIGHT);
  if (vsVideoFaceBackground == NULL) {
    return FALSE;
  }
  JSurface_SetColorKey(vsVideoFaceBackground, FROMRGB(0, 0, 0));

  // load the stats graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\stats.sti"), &guiStats));

  // load the Price graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\price.sti"), &guiPrice));

  // load the Portait graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\portrait.sti"), &guiPortrait));

  // load the WeaponBox graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\weaponbox.sti"), &guiWeaponBox));

  // load the videoconf Popup graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\VideoConfPopup.sti"), &guiVideoConfPopup));

  // load the video conf terminal graphic and add it
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\VideoConfTerminal.sti"), &guiVideoConfTerminal));

  // load the background snow for the video conf terminal
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\BWSnow.sti"), &guiBWSnow));

  // load the fuzzy line for the video conf terminal
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\FuzzLine.sti"), &guiFuzzLine));

  // load the line distortion for the video conf terminal
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\LineInterference.sti"), &guiStraightLine));

  // load the translucent snow for the video conf terminal
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\TransSnow.sti"), &guiTransSnow));

  // load the translucent snow for the video conf terminal
  CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\VideoContractCharge.sti"),
                    &guiVideoContractCharge));

  //** Mouse Regions **
  MSYS_DefineRegion(&gSelectedFaceRegion, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_X + PORTRAIT_WIDTH,
                    PORTRAIT_Y + PORTRAIT_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW,
                    SelectFaceMovementRegionCallBack, SelectFaceRegionCallBack);
  MSYS_AddRegion(&gSelectedFaceRegion);

  // Set the fast help for the mouse region
  //	SetRegionFastHelpText( &gSelectedFaceRegion, AimMemberText[ AIM_MEMBER_CLICK_INSTRUCTIONS ]
  //);

  // if user clicks in the area, the merc will shut up!
  MSYS_DefineRegion(&gSelectedShutUpMercRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y,
                    LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH - 1,
                    CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, SelectShutUpMercRegionCallBack);
  MSYS_AddRegion(&gSelectedShutUpMercRegion);
  // have it disbled at first
  MSYS_DisableRegion(&gSelectedShutUpMercRegion);

  // Button Regions
  giXToCloseVideoConfButtonImage = LoadButtonImage("LAPTOP\\x_button.sti", -1, 0, -1, 1, -1);

  guiPreviousContactNextButtonImage =
      LoadButtonImage("LAPTOP\\BottomButtons2.sti", -1, 0, -1, 1, -1);

  giPreviousButton = CreateIconAndTextButton(
      guiPreviousContactNextButtonImage, CharacterInfo[AIM_MEMBER_PREVIOUS],
      AIM_M_FONT_PREV_NEXT_CONTACT, AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_UP, DEFAULT_SHADOW,
      AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_DOWN, DEFAULT_SHADOW, TEXT_CJUSTIFIED, PREVIOUS_X,
      PREVIOUS_BOX_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnPreviousButtonCallback);
  SetButtonCursor(giPreviousButton, CURSOR_WWW);

  giContactButton = CreateIconAndTextButton(
      guiPreviousContactNextButtonImage, CharacterInfo[AIM_MEMBER_CONTACT],
      AIM_M_FONT_PREV_NEXT_CONTACT, AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_UP, DEFAULT_SHADOW,
      AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_DOWN, DEFAULT_SHADOW, TEXT_CJUSTIFIED, CONTACT_X,
      CONTACT_BOX_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
      BtnContactButtonCallback);
  SetButtonCursor(giContactButton, CURSOR_WWW);

  giNextButton = CreateIconAndTextButton(
      guiPreviousContactNextButtonImage, CharacterInfo[AIM_MEMBER_NEXT],
      AIM_M_FONT_PREV_NEXT_CONTACT, AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_UP, DEFAULT_SHADOW,
      AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_DOWN, DEFAULT_SHADOW, TEXT_CJUSTIFIED, NEXT_X, NEXT_BOX_Y,
      BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnNextButtonCallback);
  SetButtonCursor(giNextButton, CURSOR_WWW);

  gbCurrentSoldier = AimMercArray[gbCurrentIndex];

  gfStopMercFromTalking = FALSE;
  gubVideoConferencingMode = (uint8_t)giCurrentSubPage;
  gubVideoConferencingPreviousMode = AIM_VIDEO_NOT_DISPLAYED_MODE;

  gfRenderTopLevel = FALSE;

  // if we are re-entering but the video conference should still be up
  if (gubVideoConferencingMode != 0) {
    // if we need to re initialize the talking face
    if (gubVideoConferencingMode != AIM_VIDEO_FIRST_CONTACT_MERC_MODE)
      InitVideoFace(gbCurrentSoldier);

    InitDeleteVideoConferencePopUp();
  }

  InitAimMenuBar();
  InitAimDefaults();

  // LoadTextMercPopupImages( BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER);

  RenderAIMMembers();
  gfIsNewMailFlagSet = FALSE;
  gfAimMemberCanMercSayOpeningQuote = TRUE;

  return (TRUE);
}

void ExitAIMMembers() {
  RemoveAimDefaults();

  // if we are exiting and the transfer of funds popup is enable, make sure we dont come back to it
  if (gubPopUpBoxAction)
    giCurrentSubPage = AIM_VIDEO_NOT_DISPLAYED_MODE;
  else
    giCurrentSubPage = gubVideoConferencingMode;

  gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
  InitDeleteVideoConferencePopUp();

  JSurface_Free(vsVideoFaceBackground);

  DeleteVideoObjectFromIndex(guiStats);
  DeleteVideoObjectFromIndex(guiPrice);
  DeleteVideoObjectFromIndex(guiPortrait);
  DeleteVideoObjectFromIndex(guiWeaponBox);
  DeleteVideoObjectFromIndex(guiVideoConfPopup);
  DeleteVideoObjectFromIndex(guiVideoConfTerminal);
  DeleteVideoObjectFromIndex(guiBWSnow);
  DeleteVideoObjectFromIndex(guiFuzzLine);
  DeleteVideoObjectFromIndex(guiStraightLine);
  DeleteVideoObjectFromIndex(guiTransSnow);
  DeleteVideoObjectFromIndex(guiVideoContractCharge);

  UnloadButtonImage(guiPreviousContactNextButtonImage);
  UnloadButtonImage(giXToCloseVideoConfButtonImage);

  RemoveButton(giPreviousButton);
  RemoveButton(giContactButton);
  RemoveButton(giNextButton);

  MSYS_RemoveRegion(&gSelectedFaceRegion);
  MSYS_RemoveRegion(&gSelectedShutUpMercRegion);

  ExitAimMenuBar();

  InitCreateDeleteAimPopUpBox(AIM_POPUP_DELETE, NULL, NULL, 0, 0, 0);

  RemoveTextMercPopupImages();
}

void HandleAIMMembers() {
  // determine if the merc has a quote that is waiting to be said
  DelayMercSpeech(0, 0, 0, FALSE, FALSE);

  if (gfHangUpMerc && !gfMercIsTalking) {
    if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE)
      gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
    gfHangUpMerc = FALSE;
  }

  if (gfStopMercFromTalking) {
    StopMercTalking();
    gfStopMercFromTalking = FALSE;
    /*
                    //if we were waiting for the merc to stop talking
                    if( gfWaitingForMercToStopTalkingOrUserToClick )
                    {
                            gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
                            gfWaitingForMercToStopTalkingOrUserToClick = FALSE;
                    }
    */
  }

  // If we have to change video conference modes, change to new mode
  if (gubVideoConferencingMode != gubVideoConferencingPreviousMode &&
      gubPopUpBoxAction != AIM_POPUP_DISPLAY) {
    InitDeleteVideoConferencePopUp();

    // if we are exiting to display a popup box, dont rerender the display
    if (!fExitDueToMessageBox) gfRedrawScreen = TRUE;
  }

  // If we have to get rid of the popup box
  if (gubPopUpBoxAction == AIM_POPUP_DELETE) {
    InitCreateDeleteAimPopUpBox(AIM_POPUP_DELETE, NULL, NULL, 0, 0, 0);

    // if we are exiting to display a popup box, dont rerender the display
    if (!fExitDueToMessageBox) gfRedrawScreen = TRUE;
  }

  // Handle the current video conference screen
  HandleCurrentVideoConfMode();

  // If the answering machine is active, display the graphics for it
  //	if( gfIsAnsweringMachineActive )
  //		HandleAnsweringMachineMessage();

  // if the face is active, display the talking face
  if (gfVideoFaceActive) {
    gfMercIsTalking = DisplayTalkingMercFaceForVideoPopUp(giMercFaceIndex);

    // put the noise lines on the screen
    if (!gfIsAnsweringMachineActive) HandleVideoDistortion();

    // to handle when/if the merc is getting po'ed (waiting for player to do something)
    if (!gfMercIsTalking) HandleMercAttitude();
  }

  // if we have to rerender the popup, set the flag to render the PostButtonRender function in
  // laptop.c
  if (gubPopUpBoxAction == AIM_POPUP_DISPLAY) {
    fReDrawPostButtonRender = TRUE;
  }

  // Gets set in the InitDeleteVideoConferencePopUp() function
  if (gfJustSwitchedVideoConferenceMode) gfJustSwitchedVideoConferenceMode = FALSE;

  if (gfRedrawScreen) {
    RenderAIMMembers();
    gfRedrawScreen = FALSE;
  }

#ifdef JA2TESTVERSION
  TempHandleAimMemberKeyBoardInput();
#endif

  MarkButtonsDirty();
}

BOOLEAN RenderAIMMembersTopLevel() {
  InitCreateDeleteAimPopUpBox(AIM_POPUP_DISPLAY, NULL, NULL, 0, 0, 0);

  return (TRUE);
}

BOOLEAN RenderAIMMembers() {
  struct VObject *hStatsHandle;
  struct VObject *hPriceHandle;
  struct VObject *hWeaponBoxHandle;
  uint16_t x, uiPosX;
  wchar_t wTemp[50];

  DrawAimDefaults();

  // Stats
  GetVideoObject(&hStatsHandle, guiStats);
  BltVideoObject(vsFB, hStatsHandle, 0, STATS_X, STATS_Y);

  // Price
  GetVideoObject(&hPriceHandle, guiPrice);
  BltVideoObject(vsFB, hPriceHandle, 0, PRICE_X, PRICE_Y);

  // WeaponBox
  GetVideoObject(&hWeaponBoxHandle, guiWeaponBox);

  uiPosX = WEAPONBOX_X;
  for (x = 0; x < WEAPONBOX_NUMBER; x++) {
    BltVideoObject(vsFB, hWeaponBoxHandle, 0, uiPosX, WEAPONBOX_Y);
    uiPosX += WEAPONBOX_SIZE_X;
  }

  UpdateMercInfo();

  // Draw fee & contract
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_FEE], FEE_X, FEE_Y, 0, AIM_M_FONT_PREV_NEXT_CONTACT,
                   AIM_M_FEE_CONTRACT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_CONTRACT], AIM_CONTRACT_X, AIM_CONTRACT_Y,
                   AIM_CONTRACT_WIDTH, AIM_M_FONT_PREV_NEXT_CONTACT, AIM_M_FEE_CONTRACT_COLOR,
                   FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

  // Draw pay period (day, week, 2 week)
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_1_DAY], ONEDAY_X, EXPLEVEL_Y, AIM_CONTRACT_WIDTH,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_1_WEEK], ONEWEEK_X, MARKSMAN_Y, AIM_CONTRACT_WIDTH,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_2_WEEKS], TWOWEEK_X, MECHANAICAL_Y, AIM_CONTRACT_WIDTH,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);

  // Display AIM Member text
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_ACTIVE_MEMBERS], AIM_MEMBER_ACTIVE_TEXT_X,
                   AIM_MEMBER_ACTIVE_TEXT_Y, AIM_MEMBER_ACTIVE_TEXT_WIDTH, AIM_MAINTITLE_FONT,
                   AIM_M_ACTIVE_MEMBER_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  // Display Option Gear Cost text
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_OPTIONAL_GEAR], AIM_MEMBER_OPTIONAL_GEAR_X,
                   AIM_MEMBER_OPTIONAL_GEAR_Y, 0, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT,
                   FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  swprintf(wTemp, ARR_SIZE(wTemp), L"%d", gMercProfiles[gbCurrentSoldier].usOptionalGearCost);
  InsertCommasForDollarFigure(wTemp);
  InsertDollarSignInToString(wTemp);
  uiPosX = AIM_MEMBER_OPTIONAL_GEAR_X +
           StringPixLength(CharacterInfo[AIM_MEMBER_OPTIONAL_GEAR], AIM_M_FONT_STATIC_TEXT) + 5;
  DrawTextToScreen(wTemp, uiPosX, AIM_MEMBER_OPTIONAL_GEAR_Y, 0, AIM_M_FONT_STATIC_TEXT,
                   AIM_M_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  DisableAimButton();

  DisplayMercsInventory(gbCurrentSoldier);

  DisplayMercsFace();

  if (gubVideoConferencingMode) {
    DisplayMercStats();
    DisplayVideoConferencingDisplay();
  } else {
    // Display the mercs stats and face
    DisplayMercStats();

    gubMercAttitudeLevel = 0;
    gfIsAnsweringMachineActive = FALSE;
  }

  //	InitCreateDeleteAimPopUpBox( AIM_POPUP_DISPLAY, NULL, NULL, 0, 0, 0);

  // check to see if the merc is dead if so disable the contact button
  if (IsMercDead(gbCurrentSoldier)) {
    DisableButton(giContactButton);
  } else {
    EnableButton(giContactButton);
  }

  // if we are to renbder the 'click face' text
  if (gfAimMemberDisplayFaceHelpText) {
    DisplayAimMemberClickOnFaceHelpText();
  }

  RenderWWWProgramTitleBar();
  DisplayProgramBoundingBox(TRUE);
  fReDrawScreenFlag = TRUE;

  return (TRUE);
}

BOOLEAN DrawNumeralsToScreen(int32_t iNumber, int8_t bWidth, uint16_t usLocX, uint16_t usLocY,
                             uint32_t ulFont, uint8_t ubColor) {
  wchar_t sStr[10];

  swprintf(sStr, ARR_SIZE(sStr), L"%d", iNumber);

  DrawTextToScreen(sStr, usLocX, usLocY, bWidth, ulFont, ubColor, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);

  return (TRUE);
}

BOOLEAN DrawMoneyToScreen(int32_t iNumber, int8_t bWidth, uint16_t usLocX, uint16_t usLocY,
                          uint32_t ulFont, uint8_t ubColor) {
  wchar_t sStr[10];

  swprintf(sStr, ARR_SIZE(sStr), L"%d", iNumber);
  InsertCommasForDollarFigure(sStr);
  InsertDollarSignInToString(sStr);

  //	DrawTextToScreen(L"$", usLocX, usLocY, 0, ulFont, ubColor, FONT_MCOLOR_BLACK, FALSE,
  // LEFT_JUSTIFIED);
  DrawTextToScreen(sStr, usLocX, usLocY, bWidth, ulFont, ubColor, FONT_MCOLOR_BLACK, FALSE,
                   RIGHT_JUSTIFIED);

  return (TRUE);
}

void SelectFaceRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_INIT) {
  } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
    guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX;
  } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    // if the merc is not dead, video conference with the merc
    if (!IsMercDead(gbCurrentSoldier)) {
      gubVideoConferencingMode = AIM_VIDEO_POPUP_MODE;
      gfFirstTimeInContactScreen = TRUE;
    }
  }
}

void SelectFaceMovementRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    gfAimMemberDisplayFaceHelpText = FALSE;
    gfRedrawScreen = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE) {
    gfAimMemberDisplayFaceHelpText = TRUE;
    gfRedrawScreen = TRUE;
  } else if (iReason & MSYS_CALLBACK_REASON_MOVE) {
  }
}

BOOLEAN UpdateMercInfo(void) {
  wchar_t MercInfoString[SIZE_MERC_BIO_INFO];
  wchar_t AdditionalInfoString[SIZE_MERC_BIO_INFO];

  // Display the salaries
  DrawMoneyToScreen(gMercProfiles[gbCurrentSoldier].sSalary, FEE_WIDTH, FEE_X, HEALTH_Y,
                    AIM_M_NUMBER_FONT, AIM_M_COLOR_DYNAMIC_TEXT);
  DrawMoneyToScreen(gMercProfiles[gbCurrentSoldier].uiWeeklySalary, FEE_WIDTH, FEE_X, AGILITY_Y,
                    AIM_M_NUMBER_FONT, AIM_M_COLOR_DYNAMIC_TEXT);
  DrawMoneyToScreen(gMercProfiles[gbCurrentSoldier].uiBiWeeklySalary, FEE_WIDTH, FEE_X, DEXTERITY_Y,
                    AIM_M_NUMBER_FONT, AIM_M_COLOR_DYNAMIC_TEXT);

  // if medical deposit is required
  if (gMercProfiles[gbCurrentSoldier].bMedicalDeposit) {
    wchar_t zTemp[40];
    wchar_t sMedicalString[40];

    // Display the medical cost
    swprintf(zTemp, ARR_SIZE(zTemp), L"%d", gMercProfiles[gbCurrentSoldier].sMedicalDepositAmount);
    InsertCommasForDollarFigure(zTemp);
    InsertDollarSignInToString(zTemp);

    swprintf(sMedicalString, ARR_SIZE(sMedicalString), L"%s %s", zTemp,
             CharacterInfo[AIM_MEMBER_MEDICAL_DEPOSIT_REQ]);

    // If the string will be displayed in more then 2 lines, recenter the string
    if ((DisplayWrappedString(0, 0, AIM_MEDICAL_DEPOSIT_WIDTH, 2, AIM_FONT12ARIAL,
                              AIM_M_COLOR_DYNAMIC_TEXT, sMedicalString, FONT_MCOLOR_BLACK, FALSE,
                              CENTER_JUSTIFIED | DONT_DISPLAY_TEXT) /
         GetFontHeight(AIM_FONT12ARIAL)) > 2) {
      DisplayWrappedString(AIM_MEDICAL_DEPOSIT_X,
                           (uint16_t)(AIM_MEDICAL_DEPOSIT_Y - GetFontHeight(AIM_FONT12ARIAL)),
                           AIM_MEDICAL_DEPOSIT_WIDTH, 2, AIM_FONT12ARIAL, AIM_M_COLOR_DYNAMIC_TEXT,
                           sMedicalString, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
    } else
      DisplayWrappedString(AIM_MEDICAL_DEPOSIT_X, AIM_MEDICAL_DEPOSIT_Y, AIM_MEDICAL_DEPOSIT_WIDTH,
                           2, AIM_FONT12ARIAL, AIM_M_COLOR_DYNAMIC_TEXT, sMedicalString,
                           FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  }

  LoadMercBioInfo(gbCurrentSoldier, MercInfoString, AdditionalInfoString);
  if (MercInfoString[0] != 0) {
    DisplayWrappedString(AIM_MERC_INFO_X, AIM_MERC_INFO_Y, AIM_MERC_INFO_WIDTH, 2,
                         AIM_M_FONT_DYNAMIC_TEXT, AIM_FONT_MCOLOR_WHITE, MercInfoString,
                         FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }
  if (AdditionalInfoString[0] != 0) {
    DrawTextToScreen(CharacterInfo[AIM_MEMBER_ADDTNL_INFO], AIM_MERC_ADD_X, AIM_MERC_ADD_Y, 0,
                     AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                     LEFT_JUSTIFIED);
    DisplayWrappedString(AIM_MERC_ADD_INFO_X, AIM_MERC_ADD_INFO_Y, AIM_MERC_INFO_WIDTH, 2,
                         AIM_M_FONT_DYNAMIC_TEXT, AIM_FONT_MCOLOR_WHITE, AdditionalInfoString,
                         FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }

  return (TRUE);
}

BOOLEAN LoadMercBioInfo(uint8_t ubIndex, wchar_t *pInfoString, wchar_t *pAddInfo) {
  HWFILE hFile;
  uint32_t uiBytesRead;
  uint32_t uiStartSeekAmount;

  hFile = FileMan_Open(MERCBIOSFILENAME, FILE_ACCESS_READ, FALSE);
  if (!hFile) {
    return (FALSE);
  }

  // Get current mercs bio info
  uiStartSeekAmount = (SIZE_MERC_BIO_INFO + SIZE_MERC_ADDITIONAL_INFO) * ubIndex;

  if (FileMan_Seek(hFile, uiStartSeekAmount, FILE_SEEK_FROM_START) == FALSE) {
    return (FALSE);
  }

  if (!FileMan_Read(hFile, pInfoString, SIZE_MERC_BIO_INFO, &uiBytesRead)) {
    return (FALSE);
  }

  DecodeEncryptedString(pInfoString, SIZE_MERC_BIO_INFO / 2);

  // Get the additional info
  uiStartSeekAmount =
      ((SIZE_MERC_BIO_INFO + SIZE_MERC_ADDITIONAL_INFO) * ubIndex) + SIZE_MERC_BIO_INFO;
  if (FileMan_Seek(hFile, uiStartSeekAmount, FILE_SEEK_FROM_START) == FALSE) {
    return (FALSE);
  }

  if (!FileMan_Read(hFile, pAddInfo, SIZE_MERC_ADDITIONAL_INFO, &uiBytesRead)) {
    return (FALSE);
  }

  DecodeEncryptedString(pAddInfo, SIZE_MERC_ADDITIONAL_INFO / 2);

  FileMan_Close(hFile);
  return (TRUE);
}

BOOLEAN DisplayMercsInventory(uint8_t ubMercID) {
  uint8_t i;
  int16_t PosX, PosY, sCenX, sCenY;
  uint16_t usItem;
  INVTYPE *pItem;
  struct VObject *hVObject;
  uint32_t usHeight, usWidth;
  ETRLEObject *pTrav;
  wchar_t gzItemName[SIZE_ITEM_NAME];
  uint8_t ubItemCount = 0;
  //	wchar_t			gzTempItemName[ SIZE_ITEM_INFO ];

  // if the mercs inventory has already been purchased, dont display the inventory
  if (gMercProfiles[ubMercID].ubMiscFlags & PROFILE_MISC_FLAG_ALREADY_USED_ITEMS) return (TRUE);

  PosY = WEAPONBOX_Y;
  PosX = WEAPONBOX_X +
         3;  // + 3 ( 1 to take care of the shadow, +2 to get past the weapon box border )
  for (i = 0; i < NUM_INV_SLOTS; i++) {
    usItem = gMercProfiles[ubMercID].inv[i];

    // if its a valid item AND we are only displaying less then 8 items
    if (usItem && ubItemCount < WEAPONBOX_NUMBER) {
      // increase the item count
      ubItemCount++;

      pItem = &Item[usItem];
      GetVideoObject(&hVObject, GetInterfaceGraphicForItem(pItem));
      pTrav = &(hVObject->pETRLEObject[pItem->ubGraphicNum]);

      usHeight = (uint32_t)pTrav->usHeight;
      usWidth = (uint32_t)pTrav->usWidth;

      sCenX =
          PosX + (abs((int32_t)((int32_t)WEAPONBOX_SIZE_X - 3 - usWidth)) / 2) - pTrav->sOffsetX;
      sCenY = PosY + (abs((int32_t)((int32_t)WEAPONBOX_SIZE_Y - usHeight)) / 2) - pTrav->sOffsetY;

      // blt the shadow of the item
      BltVideoObjectOutlineShadowFromIndex(vsFB, GetInterfaceGraphicForItem(pItem),
                                           pItem->ubGraphicNum, sCenX - 2, sCenY + 2);
      // blt the item
      BltVideoObjectOutlineFromIndex(vsFB, GetInterfaceGraphicForItem(pItem), pItem->ubGraphicNum,
                                     sCenX, sCenY, 0, FALSE);

      // if there are more then 1 piece of equipment in the current slot, display how many there are
      if (gMercProfiles[ubMercID].bInvNumber[i] > 1) {
        wchar_t zTempStr[32];
        //				uint16_t	usWidthOfNumber;

        swprintf(zTempStr, ARR_SIZE(zTempStr), L"x%d", gMercProfiles[ubMercID].bInvNumber[i]);

        DrawTextToScreen(zTempStr, (uint16_t)(PosX - 1), (uint16_t)(PosY + 20),
                         AIM_MEMBER_WEAPON_NAME_WIDTH, AIM_M_FONT_DYNAMIC_TEXT,
                         AIM_M_WEAPON_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
      } else {
      }

      wcscpy(gzItemName, ShortItemNames[usItem]);

      // if this will only be a single line, center it in the box
      if ((DisplayWrappedString((uint16_t)(PosX - 1), AIM_MEMBER_WEAPON_NAME_Y,
                                AIM_MEMBER_WEAPON_NAME_WIDTH, 2, AIM_M_WEAPON_TEXT_FONT,
                                AIM_M_WEAPON_TEXT_COLOR, gzItemName, FONT_MCOLOR_BLACK, FALSE,
                                CENTER_JUSTIFIED | DONT_DISPLAY_TEXT) /
           GetFontHeight(AIM_M_WEAPON_TEXT_FONT)) == 1)
        DisplayWrappedString(
            (uint16_t)(PosX - 1),
            (uint16_t)(AIM_MEMBER_WEAPON_NAME_Y + GetFontHeight(AIM_M_WEAPON_TEXT_FONT) / 2),
            AIM_MEMBER_WEAPON_NAME_WIDTH, 2, AIM_M_WEAPON_TEXT_FONT, AIM_M_WEAPON_TEXT_COLOR,
            gzItemName, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
      else
        DisplayWrappedString((uint16_t)(PosX - 1), AIM_MEMBER_WEAPON_NAME_Y,
                             AIM_MEMBER_WEAPON_NAME_WIDTH, 2, AIM_M_WEAPON_TEXT_FONT,
                             AIM_M_WEAPON_TEXT_COLOR, gzItemName, FONT_MCOLOR_BLACK, FALSE,
                             CENTER_JUSTIFIED);

      PosX += WEAPONBOX_SIZE_X;
    }
  }

  return (TRUE);
}

void BtnPreviousButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      InitCreateDeleteAimPopUpBox(AIM_POPUP_DELETE, NULL, NULL, 0, 0, 0);

      if (gbCurrentIndex > 0)
        gbCurrentIndex--;
      else
        gbCurrentIndex = MAX_NUMBER_MERCS - 1;

      gfRedrawScreen = TRUE;

      gbCurrentSoldier = AimMercArray[gbCurrentIndex];

      gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnContactButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      // if we are not already in the video conferemce mode, go in to it
      if (!gubVideoConferencingMode) {
        gubVideoConferencingMode = AIM_VIDEO_POPUP_MODE;
        //				gubVideoConferencingMode = AIM_VIDEO_INIT_MODE;
        gfFirstTimeInContactScreen = TRUE;
      }

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      InitCreateDeleteAimPopUpBox(AIM_POPUP_DELETE, NULL, NULL, 0, 0, 0);

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnNextButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      btn->uiFlags &= (~BUTTON_CLICKED_ON);
      InitCreateDeleteAimPopUpBox(AIM_POPUP_DELETE, NULL, NULL, 0, 0, 0);

      if (gbCurrentIndex < MAX_NUMBER_MERCS - 1)
        gbCurrentIndex++;
      else
        gbCurrentIndex = 0;

      gbCurrentSoldier = AimMercArray[gbCurrentIndex];

      gfRedrawScreen = TRUE;

      gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

BOOLEAN DisplayMercsFace() {
  struct VObject *hFaceHandle;
  struct VObject *hPortraitHandle;
  char *sFaceLoc = "FACES\\BIGFACES\\";
  char sTemp[100];
  struct SOLDIERTYPE *pSoldier = NULL;

  // See if the merc is currently hired
  pSoldier = FindSoldierByProfileID(gbCurrentSoldier, TRUE);

  // Portrait Frame
  GetVideoObject(&hPortraitHandle, guiPortrait);
  BltVideoObject(vsFB, hPortraitHandle, 0, PORTRAIT_X, PORTRAIT_Y);

  // load the Face graphic and add it
  sprintf(sTemp, "%s%02d.sti", sFaceLoc, gbCurrentSoldier);
  CHECKF(AddVObject(CreateVObjectFromFile(sTemp), &guiFace));

  // Blt face to screen
  GetVideoObject(&hFaceHandle, guiFace);
  BltVideoObject(vsFB, hFaceHandle, 0, FACE_X, FACE_Y);

  // if the merc is dead
  if (IsMercDead(gbCurrentSoldier)) {
    // shade the face red, (to signif that he is dead)
    hFaceHandle->pShades[0] =
        Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED,
                                 DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);

    // get the face object
    GetVideoObject(&hFaceHandle, guiFace);

    // set the red pallete to the face
    SetObjectHandleShade(guiFace, 0);

    // Blt face to screen
    BltVideoObject(vsFB, hFaceHandle, 0, FACE_X, FACE_Y);

    // if the merc is dead, display it
    DrawTextToScreen(AimPopUpText[AIM_MEMBER_DEAD], FACE_X + 1, FACE_Y + 107, FACE_WIDTH,
                     FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  }

  // else if the merc is currently a POW or, the merc was fired as a pow
  else if (gMercProfiles[gbCurrentSoldier].bMercStatus == MERC_FIRED_AS_A_POW ||
           (pSoldier && GetSolAssignment(pSoldier) == ASSIGNMENT_POW)) {
    ShadowVideoSurfaceRect(vsFB, FACE_X, FACE_Y, FACE_X + FACE_WIDTH, FACE_Y + FACE_HEIGHT);
    DrawTextToScreen(pPOWStrings[0], FACE_X + 1, FACE_Y + 107, FACE_WIDTH, FONT14ARIAL, 145,
                     FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  }

  // else if the merc has already been hired
  else if (FindSoldierByProfileID(gbCurrentSoldier, TRUE)) {
    ShadowVideoSurfaceRect(vsFB, FACE_X, FACE_Y, FACE_X + FACE_WIDTH, FACE_Y + FACE_HEIGHT);
    DrawTextToScreen(MercInfo[MERC_FILES_ALREADY_HIRED], FACE_X + 1, FACE_Y + 107, FACE_WIDTH,
                     FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  }

  else if (!IsMercHireable(gbCurrentSoldier)) {
    // else if the merc has a text file and the merc is not away
    ShadowVideoSurfaceRect(vsFB, FACE_X, FACE_Y, FACE_X + FACE_WIDTH, FACE_Y + FACE_HEIGHT);
    DrawTextToScreen(AimPopUpText[AIM_MEMBER_ON_ASSIGNMENT], FACE_X + 1, FACE_Y + 107, FACE_WIDTH,
                     FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  }

  DeleteVideoObjectFromIndex(guiFace);

  return (TRUE);
}

void DisplayMercStats() {
  uint8_t ubColor;

  //
  // Display all the static text
  //

  // First column in stats box.  Health, Agility, dexterity, strength, leadership, wisdom
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_HEALTH], STATS_FIRST_COL, HEALTH_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_FIRST_COL, HEALTH_Y, FIRST_COLUMN_DOT, CharacterInfo[AIM_MEMBER_HEALTH]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_AGILITY], STATS_FIRST_COL, AGILITY_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_FIRST_COL, AGILITY_Y, FIRST_COLUMN_DOT, CharacterInfo[AIM_MEMBER_AGILITY]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_DEXTERITY], STATS_FIRST_COL, DEXTERITY_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_FIRST_COL, DEXTERITY_Y, FIRST_COLUMN_DOT, CharacterInfo[AIM_MEMBER_DEXTERITY]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_STRENGTH], STATS_FIRST_COL, STRENGTH_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_FIRST_COL, STRENGTH_Y, FIRST_COLUMN_DOT, CharacterInfo[AIM_MEMBER_STRENGTH]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_LEADERSHIP], STATS_FIRST_COL, LEADERSHIP_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_FIRST_COL, LEADERSHIP_Y, FIRST_COLUMN_DOT,
              CharacterInfo[AIM_MEMBER_LEADERSHIP]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_WISDOM], STATS_FIRST_COL, WISDOM_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_FIRST_COL, WISDOM_Y, FIRST_COLUMN_DOT, CharacterInfo[AIM_MEMBER_WISDOM]);

  // Second column in stats box.  Exp.Level, Markmanship, mechanical, explosive, medical
  DrawTextToScreen(CharacterInfo[AIM_MEMBER_EXP_LEVEL], STATS_SECOND_COL, EXPLEVEL_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_SECOND_COL, EXPLEVEL_Y, SECOND_COLUMN_DOT, CharacterInfo[AIM_MEMBER_EXP_LEVEL]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_MARKSMANSHIP], STATS_SECOND_COL, MARKSMAN_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_SECOND_COL, MARKSMAN_Y, SECOND_COLUMN_DOT,
              CharacterInfo[AIM_MEMBER_MARKSMANSHIP]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_MECHANICAL], STATS_SECOND_COL, MECHANAICAL_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_SECOND_COL, MECHANAICAL_Y, SECOND_COLUMN_DOT,
              CharacterInfo[AIM_MEMBER_MECHANICAL]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_EXPLOSIVE], STATS_SECOND_COL, EXPLOSIVE_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_SECOND_COL, EXPLOSIVE_Y, SECOND_COLUMN_DOT,
              CharacterInfo[AIM_MEMBER_EXPLOSIVE]);

  DrawTextToScreen(CharacterInfo[AIM_MEMBER_MEDICAL], STATS_SECOND_COL, MEDICAL_Y, 0,
                   AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE,
                   LEFT_JUSTIFIED);
  DisplayDots(STATS_SECOND_COL, MEDICAL_Y, SECOND_COLUMN_DOT, CharacterInfo[AIM_MEMBER_MEDICAL]);

  //
  // Display all the Merc dynamic stat info
  //

  // Name
  DrawTextToScreen(gMercProfiles[gbCurrentSoldier].zName, NAME_X, NAME_Y, 0, FONT14ARIAL,
                   AIM_M_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

  // Numbers for above.   Health, Agility, dexterity, strength, leadership, wisdom

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bLife);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bLife, 3, STATS_FIRST_NUM, HEALTH_Y,
                       AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bAgility);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bAgility, 3, STATS_FIRST_NUM, AGILITY_Y,
                       AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bDexterity);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bDexterity, 3, STATS_FIRST_NUM, DEXTERITY_Y,
                       AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bStrength);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bStrength, 3, STATS_FIRST_NUM, STRENGTH_Y,
                       AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bLeadership);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bLeadership, 3, STATS_FIRST_NUM,
                       LEADERSHIP_Y, AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bWisdom);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bWisdom, 3, STATS_FIRST_NUM, WISDOM_Y,
                       AIM_M_NUMBER_FONT, ubColor);

  // Second column in stats box.  Exp.Level, Markmanship, mechanical, explosive, medical

  //	ubColor = GetStatColor( gMercProfiles[gbCurrentSoldier].bExpLevel );
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bExpLevel, 3, STATS_SECOND_NUM, EXPLEVEL_Y,
                       AIM_M_NUMBER_FONT, FONT_MCOLOR_WHITE);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bMarksmanship);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bMarksmanship, 3, STATS_SECOND_NUM,
                       MARKSMAN_Y, AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bMechanical);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bMechanical, 3, STATS_SECOND_NUM,
                       MECHANAICAL_Y, AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bExplosive);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bExplosive, 3, STATS_SECOND_NUM, EXPLOSIVE_Y,
                       AIM_M_NUMBER_FONT, ubColor);

  ubColor = GetStatColor(gMercProfiles[gbCurrentSoldier].bMedical);
  DrawNumeralsToScreen(gMercProfiles[gbCurrentSoldier].bMedical, 3, STATS_SECOND_NUM, MEDICAL_Y,
                       AIM_M_NUMBER_FONT, ubColor);
}

uint8_t GetStatColor(int8_t bStat) {
  if (bStat >= 80)
    return (HIGH_STAT_COLOR);
  else if (bStat >= 50)
    return (MED_STAT_COLOR);
  else
    return (LOW_STAT_COLOR);
}

// displays the dots between the stats and the stat name
void DisplayDots(uint16_t usNameX, uint16_t usNameY, uint16_t usStatX, wchar_t *pString) {
  uint16_t usStringLength = StringPixLength(pString, AIM_M_FONT_STATIC_TEXT);
  int16_t i;
  uint16_t usPosX;

  usPosX = usStatX;
  for (i = usNameX + usStringLength; i <= usPosX; usPosX -= 7) {
    DrawTextToScreen(L".", (uint16_t)usPosX, usNameY, 0, AIM_M_FONT_STATIC_TEXT,
                     AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }
}

void BtnContractLengthButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    uint8_t ubRetValue = (uint8_t)MSYS_GetBtnUserData(btn, 0);

    btn->uiFlags |= BUTTON_CLICKED_ON;

    gubContractLength = ubRetValue;
    DisplaySelectLights(TRUE, FALSE);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    DisplaySelectLights(FALSE, FALSE);

    guiMercAttitudeTime = GetJA2Clock();

    DisplayMercChargeAmount();
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);

    //		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
    // btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnBuyEquipmentButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    gfBuyEquipment = (uint8_t)MSYS_GetBtnUserData(btn, 0);
    DisplaySelectLights(FALSE, TRUE);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    DisplaySelectLights(FALSE, FALSE);
    DisplayMercChargeAmount();

    guiMercAttitudeTime = GetJA2Clock();

    //		InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);

    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

// Transfer funds button callback
void BtnAuthorizeButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (!(btn->uiFlags & BUTTON_ENABLED)) return;

  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    uint8_t ubRetValue = (uint8_t)MSYS_GetBtnUserData(btn, 0);
    btn->uiFlags &= (~BUTTON_CLICKED_ON);

    gfStopMercFromTalking = TRUE;
    gubMercAttitudeLevel = QUOTE_DELAY_NO_ACTION;

    // If we try to hire the merc
    if (ubRetValue == 0) {
      StopMercTalking();

      // can the merc be hired?  (does he like/not like people on the team
      //			if( CanMercBeHired() )
      {
        // Was the merc hired
        if (AimMemberHireMerc()) {
          // if merc was hired
          InitCreateDeleteAimPopUpBox(AIM_POPUP_CREATE,
                                      AimPopUpText[AIM_MEMBER_FUNDS_TRANSFER_SUCCESFUL], NULL,
                                      AIM_POPUP_BOX_X, AIM_POPUP_BOX_Y, AIM_POPUP_BOX_SUCCESS);
          DelayMercSpeech(gbCurrentSoldier, QUOTE_CONTRACT_ACCEPTANCE, 750, TRUE, FALSE);

          // Disable the buttons behind the message box
          EnableDisableCurrentVideoConferenceButtons(TRUE);

          SpecifyDisabledButtonStyle(giBuyEquipmentButton[0], DISABLED_STYLE_NONE);
          SpecifyDisabledButtonStyle(giBuyEquipmentButton[1], DISABLED_STYLE_NONE);

          giIdOfLastHiredMerc = AimMercArray[gbCurrentIndex];
        }
      }
      /*
                              else
                              {
                                      //else the merc doesnt like a player on the team, hang up when
         the merc is done complaining

                                      //reset ( in case merc was going to say something
                                      DelayMercSpeech( 0, 0, 0, FALSE, TRUE );

                                      gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
                              }
      */
    }
    // else we cancel
    else {
      gubVideoConferencingMode = AIM_VIDEO_FIRST_CONTACT_MERC_MODE;
    }
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

int8_t AimMemberHireMerc() {
  MERC_HIRE_STRUCT HireMercStruct;
  uint8_t ubCurrentSoldier = AimMercArray[gbCurrentIndex];
  int8_t bReturnCode;
  int16_t sSoldierID = 0;
  int8_t bTypeOfContract = 0;

  if (MoneyGetBalance() < giContractAmount) {
    // wasnt hired because of lack of funds
    InitCreateDeleteAimPopUpBox(AIM_POPUP_CREATE, AimPopUpText[AIM_MEMBER_FUNDS_TRANSFER_FAILED],
                                AimPopUpText[AIM_MEMBER_NOT_ENOUGH_FUNDS], AIM_POPUP_BOX_X,
                                AIM_POPUP_BOX_Y, AIM_POPUP_BOX_FAILURE);

    // Disable the buttons behind the message box
    EnableDisableCurrentVideoConferenceButtons(TRUE);

    SpecifyDisabledButtonStyle(giBuyEquipmentButton[0], DISABLED_STYLE_NONE);
    SpecifyDisabledButtonStyle(giBuyEquipmentButton[1], DISABLED_STYLE_NONE);

    DelayMercSpeech(gbCurrentSoldier, QUOTE_REFUSAL_TO_JOIN_LACK_OF_FUNDS, 750, TRUE, FALSE);

    return (FALSE);
  }

  memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

  HireMercStruct.ubProfileID = ubCurrentSoldier;

  // DEF: temp
  HireMercStruct.sSectorX = gsMercArriveSectorX;
  HireMercStruct.sSectorY = gsMercArriveSectorY;
  HireMercStruct.fUseLandingZoneForArrival = TRUE;
  HireMercStruct.ubInsertionCode = INSERTION_CODE_ARRIVING_GAME;

  HireMercStruct.fCopyProfileItemsOver = gfBuyEquipment;
  // if the players is buyibng the equipment
  if (gfBuyEquipment) {
    gMercProfiles[ubCurrentSoldier].ubMiscFlags |= PROFILE_MISC_FLAG_ALREADY_USED_ITEMS;
  }

  // If 1 day
  if (gubContractLength == AIM_CONTRACT_LENGTH_ONE_DAY) {
    bTypeOfContract = CONTRACT_EXTEND_1_DAY;
    HireMercStruct.iTotalContractLength = 1;
  } else if (gubContractLength == AIM_CONTRACT_LENGTH_ONE_WEEK) {
    bTypeOfContract = CONTRACT_EXTEND_1_WEEK;
    HireMercStruct.iTotalContractLength = 7;
  } else if (gubContractLength == AIM_CONTRACT_LENGTH_TWO_WEEKS) {
    bTypeOfContract = CONTRACT_EXTEND_2_WEEK;
    HireMercStruct.iTotalContractLength = 14;
  }

  // specify when the merc should arrive
  HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay();  // + ubCurrentSoldier

  // Set the time and ID of the last hired merc will arrive
  //	LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = HireMercStruct.ubProfileID;
  //	LaptopSaveInfo.sLastHiredMerc.uiArrivalTime = HireMercStruct.uiTimeTillMercArrives;

  // if we succesfully hired the merc
  bReturnCode = HireMerc(&HireMercStruct);
  if (bReturnCode == MERC_HIRE_OVER_20_MERCS_HIRED) {
    // display a warning saying u cant hire more then 20 mercs
    DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, AimPopUpText[AIM_MEMBER_ALREADY_HAVE_20_MERCS],
                       LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
    return (FALSE);
  } else if (bReturnCode == MERC_HIRE_FAILED) {
    return (FALSE);
  }

  // Set the type of contract the merc is on
  sSoldierID = GetSoldierIDFromMercID(ubCurrentSoldier);
  if (sSoldierID == -1) return (FALSE);
  Menptr[sSoldierID].bTypeOfLastContract = bTypeOfContract;

  // add an entry in the finacial page for the hiring of the merc
  AddTransactionToPlayersBook(
      HIRED_MERC, ubCurrentSoldier,
      -(giContractAmount - gMercProfiles[gbCurrentSoldier].sMedicalDepositAmount));

  if (gMercProfiles[gbCurrentSoldier].bMedicalDeposit) {
    // add an entry in the finacial page for the medical deposit
    AddTransactionToPlayersBook(MEDICAL_DEPOSIT, ubCurrentSoldier,
                                -(gMercProfiles[gbCurrentSoldier].sMedicalDepositAmount));
  }

  // add an entry in the history page for the hiring of the merc
  AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_AIM, ubCurrentSoldier, GetWorldTotalMin(), -1, -1);
  return (TRUE);
}

BOOLEAN DisplayVideoConferencingDisplay() {
  wchar_t sMercName[128];

  if ((gubVideoConferencingMode == AIM_VIDEO_NOT_DISPLAYED_MODE) ||
      (gubVideoConferencingMode == AIM_VIDEO_POPUP_MODE))
    return (FALSE);

  DisplayMercsVideoFace();

  // Title & Name
  if (gubVideoConferencingMode == AIM_VIDEO_INIT_MODE) {
    swprintf(sMercName, ARR_SIZE(sMercName), L"%s", VideoConfercingText[AIM_MEMBER_CONNECTING]);
    DrawTextToScreen(sMercName, AIM_MEMBER_VIDEO_NAME_X, AIM_MEMBER_VIDEO_NAME_Y, 0, FONT12ARIAL,
                     AIM_M_VIDEO_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  } else {
    swprintf(sMercName, ARR_SIZE(sMercName), L"%s %s",
             VideoConfercingText[AIM_MEMBER_VIDEO_CONF_WITH],
             gMercProfiles[gbCurrentSoldier].zName);
    DrawTextToScreen(sMercName, AIM_MEMBER_VIDEO_NAME_X, AIM_MEMBER_VIDEO_NAME_Y, 0, FONT12ARIAL,
                     AIM_M_VIDEO_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }

  // Display Contract charge text
  if (gubVideoConferencingMode == AIM_VIDEO_HIRE_MERC_MODE) {
    // Display the contract charge
    SetFontShadow(AIM_M_VIDEO_NAME_SHADOWCOLOR);
    DrawTextToScreen(VideoConfercingText[AIM_MEMBER_CONTRACT_CHARGE], AIM_CONTRACT_CHARGE_X,
                     AIM_CONTRACT_CHARGE_Y, 0, FONT12ARIAL, AIM_M_VIDEO_NAME_COLOR,
                     FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
    SetFontShadow(DEFAULT_SHADOW);
  }

  DisplayMercChargeAmount();

  //	if( gfMercIsTalking && !gfIsAnsweringMachineActive)
  if (gfMercIsTalking && gGameSettings.fOptions[TOPTION_SUBTITLES]) {
    uint16_t usActualWidth;
    uint16_t usActualHeight;
    uint16_t usPosX;

    SET_USE_WINFONTS(TRUE);
    SET_WINFONT(giSubTitleWinFont);
    iAimMembersBoxId =
        PrepareMercPopupBox(iAimMembersBoxId, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER,
                            gsTalkingMercText, 300, 0, 0, 0, &usActualWidth, &usActualHeight);
    SET_USE_WINFONTS(FALSE);

    usPosX = (LAPTOP_SCREEN_LR_X - usActualWidth) / 2;

    RenderMercPopUpBoxFromIndex(iAimMembersBoxId, usPosX, TEXT_POPUP_WINDOW_Y, vsFB);

    if (RemoveMercPopupBoxFromIndex(iAimMembersBoxId)) {
      iAimMembersBoxId = -1;
    }
  }

  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);

  return (TRUE);
}

BOOLEAN DisplayMercsVideoFace() {
  struct VObject *hTerminalHandle;

  // Get and Blt Terminal Frame
  GetVideoObject(&hTerminalHandle, guiVideoConfTerminal);
  ShadowVideoSurfaceImage(vsFB, hTerminalHandle, AIM_MEMBER_VIDEO_CONF_TERMINAL_X,
                          AIM_MEMBER_VIDEO_CONF_TERMINAL_Y);
  BltVideoObject(vsFB, hTerminalHandle, 0, AIM_MEMBER_VIDEO_CONF_TERMINAL_X,
                 AIM_MEMBER_VIDEO_CONF_TERMINAL_Y);

  // Display the Select light on the merc
  if (gubVideoConferencingMode == AIM_VIDEO_HIRE_MERC_MODE) DisplaySelectLights(FALSE, FALSE);

  return (TRUE);
}

void DisplaySelectLights(BOOLEAN fContractDown, BOOLEAN fBuyEquipDown) {
  uint16_t i, usPosY, usPosX;

  // First draw the select light for the contract length buttons
  usPosY = AIM_MEMBER_BUY_CONTRACT_LENGTH_Y;
  for (i = 0; i < 3; i++) {
    // if the if is true, the light is on
    if (gubContractLength == i) {
      if (fContractDown) {
        usPosX = AIM_MEMBER_BUY_CONTRACT_LENGTH_X + AIM_SELECT_LIGHT_ON_X;
        ColorFillVSurfaceArea(vsFB, usPosX, usPosY + AIM_SELECT_LIGHT_ON_Y, usPosX + 8,
                              usPosY + AIM_SELECT_LIGHT_ON_Y + 8,
                              rgb32_to_rgb16(FROMRGB(0, 255, 0)));
      } else {
        usPosX = AIM_MEMBER_BUY_CONTRACT_LENGTH_X + AIM_SELECT_LIGHT_OFF_X;
        ColorFillVSurfaceArea(vsFB, usPosX, usPosY + AIM_SELECT_LIGHT_OFF_Y, usPosX + 8,
                              usPosY + AIM_SELECT_LIGHT_OFF_Y + 8,
                              rgb32_to_rgb16(FROMRGB(0, 255, 0)));
      }
    } else {
      usPosX = AIM_MEMBER_BUY_CONTRACT_LENGTH_X + AIM_SELECT_LIGHT_OFF_X;
      ColorFillVSurfaceArea(vsFB, usPosX, usPosY + AIM_SELECT_LIGHT_OFF_Y, usPosX + 8,
                            usPosY + AIM_SELECT_LIGHT_OFF_Y + 8, rgb32_to_rgb16(FROMRGB(0, 0, 0)));
    }
    usPosY += AIM_MEMBER_BUY_EQUIPMENT_GAP;
  }

  // draw the select light for the buy equipment buttons
  usPosY = AIM_MEMBER_BUY_CONTRACT_LENGTH_Y;
  for (i = 0; i < 2; i++) {
    if (gfBuyEquipment == i) {
      if (fBuyEquipDown) {
        usPosX = AIM_MEMBER_BUY_EQUIPMENT_X + AIM_SELECT_LIGHT_ON_X;
        ColorFillVSurfaceArea(vsFB, usPosX, usPosY + AIM_SELECT_LIGHT_ON_Y, usPosX + 8,
                              usPosY + AIM_SELECT_LIGHT_ON_Y + 8,
                              rgb32_to_rgb16(FROMRGB(0, 255, 0)));
      } else {
        usPosX = AIM_MEMBER_BUY_EQUIPMENT_X + AIM_SELECT_LIGHT_OFF_X;
        ColorFillVSurfaceArea(vsFB, usPosX, usPosY + AIM_SELECT_LIGHT_OFF_Y, usPosX + 8,
                              usPosY + AIM_SELECT_LIGHT_OFF_Y + 8,
                              rgb32_to_rgb16(FROMRGB(0, 255, 0)));
      }
    } else {
      usPosX = AIM_MEMBER_BUY_EQUIPMENT_X + AIM_SELECT_LIGHT_OFF_X;
      ColorFillVSurfaceArea(vsFB, usPosX, usPosY + AIM_SELECT_LIGHT_OFF_Y, usPosX + 8,
                            usPosY + AIM_SELECT_LIGHT_OFF_Y + 8, rgb32_to_rgb16(FROMRGB(0, 0, 0)));
    }
    usPosY += AIM_MEMBER_BUY_EQUIPMENT_GAP;
  }
  InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                   LAPTOP_SCREEN_WEB_LR_Y);
}

uint32_t DisplayMercChargeAmount() {
  wchar_t wTemp[50];
  wchar_t wDollarTemp[50];
  struct VObject *hImageHandle;

  if (gubVideoConferencingMode != AIM_VIDEO_HIRE_MERC_MODE) return (0);

  // Display the 'black hole'for the contract charge  in the video conference terminal
  GetVideoObject(&hImageHandle, guiVideoContractCharge);
  BltVideoObject(vsFB, hImageHandle, 0, AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_X,
                 AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_Y);

  if (FindSoldierByProfileID(gbCurrentSoldier, TRUE) == NULL) {
    giContractAmount = 0;

    // the contract charge amount

    // Get the salary rate
    if (gubContractLength == AIM_CONTRACT_LENGTH_ONE_DAY)
      giContractAmount = gMercProfiles[gbCurrentSoldier].sSalary;

    else if (gubContractLength == AIM_CONTRACT_LENGTH_ONE_WEEK)
      giContractAmount = gMercProfiles[gbCurrentSoldier].uiWeeklySalary;

    else if (gubContractLength == AIM_CONTRACT_LENGTH_TWO_WEEKS)
      giContractAmount = gMercProfiles[gbCurrentSoldier].uiBiWeeklySalary;

    // if there is a medical deposit, add it in
    if (gMercProfiles[gbCurrentSoldier].bMedicalDeposit) {
      giContractAmount += gMercProfiles[gbCurrentSoldier].sMedicalDepositAmount;
    }

    // If hired with the equipment, add it in aswell
    if (gfBuyEquipment) {
      giContractAmount += gMercProfiles[gbCurrentSoldier].usOptionalGearCost;
    }
  }

  swprintf(wDollarTemp, ARR_SIZE(wDollarTemp), L"%d", giContractAmount);
  InsertCommasForDollarFigure(wDollarTemp);
  InsertDollarSignInToString(wDollarTemp);

  // if the merc hasnt just been hired
  //	if( FindSoldierByProfileID( gbCurrentSoldier, TRUE ) == NULL )
  {
    if (gMercProfiles[gbCurrentSoldier].bMedicalDeposit)
      swprintf(wTemp, ARR_SIZE(wTemp), L"%s %s", wDollarTemp,
               VideoConfercingText[AIM_MEMBER_WITH_MEDICAL]);
    else
      swprintf(wTemp, ARR_SIZE(wTemp), L"%s", wDollarTemp);

    DrawTextToScreen(wTemp, AIM_CONTRACT_CHARGE_AMOUNNT_X + 1, AIM_CONTRACT_CHARGE_AMOUNNT_Y + 3, 0,
                     AIM_M_VIDEO_CONTRACT_AMOUNT_FONT, AIM_M_VIDEO_CONTRACT_AMOUNT_COLOR,
                     FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
  }

  return (giContractAmount);
}

BOOLEAN InitCreateDeleteAimPopUpBox(uint8_t ubFlag, wchar_t *sString1, wchar_t *sString2,
                                    uint16_t usPosX, uint16_t usPosY, uint8_t ubData) {
  struct VObject *hPopupBoxHandle;
  static uint16_t usPopUpBoxPosX, usPopUpBoxPosY;
  static wchar_t sPopUpString1[400], sPopUpString2[400];
  static BOOLEAN fPopUpBoxActive = FALSE;
  ;

  switch (ubFlag) {
    case AIM_POPUP_CREATE: {
      if (fPopUpBoxActive) return (FALSE);

      // Disable the 'X' to close the pop upi video
      DisableButton(giXToCloseVideoConfButton);

      if (sString1 != NULL)
        wcscpy(sPopUpString1, sString1);
      else
        sPopUpString1[0] = L'\0';

      if (sString2 != NULL)
        wcscpy(sPopUpString2, sString2);
      else
        sPopUpString2[0] = L'\0';

      usPopUpBoxPosX = usPosX;
      usPopUpBoxPosY = usPosY;

      // load the popup box graphic
      CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\VideoConfPopUp.sti"), &guiPopUpBox));

      GetVideoObject(&hPopupBoxHandle, guiPopUpBox);
      BltVideoObject(vsFB, hPopupBoxHandle, 0, usPosX, usPosY);

      // Create the popup boxes button
      guiPopUpImage = LoadButtonImage("LAPTOP\\VideoConfButtons.sti", -1, 2, -1, 3, -1);
      guiPopUpOkButton = CreateIconAndTextButton(
          guiPopUpImage, VideoConfercingText[AIM_MEMBER_OK], FONT14ARIAL, AIM_POPUP_BOX_COLOR,
          AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_POPUP_BOX_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR,
          TEXT_CJUSTIFIED, (uint16_t)(usPosX + AIM_POPUP_BOX_BUTTON_OFFSET_X),
          (uint16_t)(usPosY + AIM_POPUP_BOX_BUTTON_OFFSET_Y), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH + 5,
          DEFAULT_MOVE_CALLBACK, BtnPopUpOkButtonCallback);
      SetButtonCursor(guiPopUpOkButton, CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(guiPopUpOkButton, 0, ubData);

      fPopUpBoxActive = TRUE;
      gubPopUpBoxAction = AIM_POPUP_DISPLAY;

      // Disable the current video conference buttons
      // EnableDisableCurrentVideoConferenceButtons(TRUE);
      if (gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE) {
        // Enable the current video conference buttons
        EnableDisableCurrentVideoConferenceButtons(FALSE);
      }

      //
      //	Create a new flag for the PostButtonRendering function
      //
      fReDrawPostButtonRender = TRUE;
    } break;

    case AIM_POPUP_DISPLAY: {
      struct VObject *hPopupBoxHandle;
      uint16_t usTempPosY = usPopUpBoxPosY;

      if (gubPopUpBoxAction != AIM_POPUP_DISPLAY) return (FALSE);

      // load and display the popup box graphic
      GetVideoObject(&hPopupBoxHandle, guiPopUpBox);
      BltVideoObject(vsFB, hPopupBoxHandle, 0, usPopUpBoxPosX, usPopUpBoxPosY);

      SetFontShadow(AIM_M_VIDEO_NAME_SHADOWCOLOR);

      usTempPosY += AIM_POPUP_BOX_STRING1_Y;
      if (sPopUpString1[0] != L'\0')
        usTempPosY += DisplayWrappedString(usPopUpBoxPosX, usTempPosY, AIM_POPUP_BOX_WIDTH, 2,
                                           AIM_POPUP_BOX_FONT, AIM_POPUP_BOX_COLOR, sPopUpString1,
                                           FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
      if (sPopUpString2[0] != L'\0')
        DisplayWrappedString(usPopUpBoxPosX, (uint16_t)(usTempPosY + 4), AIM_POPUP_BOX_WIDTH, 2,
                             AIM_POPUP_BOX_FONT, AIM_POPUP_BOX_COLOR, sPopUpString2,
                             FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

      SetFontShadow(DEFAULT_SHADOW);

      InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X,
                       LAPTOP_SCREEN_WEB_LR_Y);

    } break;

    case AIM_POPUP_DELETE: {
      if (!fPopUpBoxActive) return (FALSE);

      // Disable the 'X' to close the pop upi video
      EnableButton(giXToCloseVideoConfButton);

      UnloadButtonImage(guiPopUpImage);
      RemoveButton(guiPopUpOkButton);
      DeleteVideoObjectFromIndex(guiPopUpBox);

      fPopUpBoxActive = FALSE;
      gubPopUpBoxAction = AIM_POPUP_NOTHING;

      if (gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE) {
        // Enable the current video conference buttons
        EnableDisableCurrentVideoConferenceButtons(FALSE);
      } else if (gubVideoConferencingPreviousMode == AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE) {
        EnableButton(giAnsweringMachineButton[1]);
      }
    } break;
  }

  return (TRUE);
}

void BtnPopUpOkButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  static BOOLEAN fInCallback = TRUE;

  if (fInCallback) {
    if (!(btn->uiFlags & BUTTON_ENABLED)) return;

    if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
      btn->uiFlags |= BUTTON_CLICKED_ON;
      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
    if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      uint8_t ubCurPageNum = (uint8_t)MSYS_GetBtnUserData(btn, 0);

      btn->uiFlags &= (~BUTTON_CLICKED_ON);
      fInCallback = FALSE;

      //			gfStopMercFromTalking = TRUE;

      gubPopUpBoxAction = AIM_POPUP_DELETE;

      if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE) {
        if (ubCurPageNum == AIM_POPUP_BOX_SUCCESS) {
          gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
          WaitForMercToFinishTalkingOrUserToClick();
        }
        //				gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
        else
          gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
      }

      fInCallback = TRUE;

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
}

// we first contact merc.  We either go to hire him or cancel the call
void BtnFirstContactButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      uint8_t ubRetValue = (uint8_t)MSYS_GetBtnUserData(btn, 0);

      //			gfStopMercFromTalking = TRUE;
      StopMercTalking();

      gfAimMemberCanMercSayOpeningQuote = FALSE;

      if (ubRetValue == 0) {
        if (CanMercBeHired()) {
          gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
        }
      } else {
        gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
      }

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnAnsweringMachineButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      uint8_t ubRetValue = (uint8_t)MSYS_GetBtnUserData(btn, 0);

      if (ubRetValue == 0) {
        // Set a flag indicating that the merc has a message
        gMercProfiles[gbCurrentSoldier].ubMiscFlags3 |=
            PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM;
        WaitForMercToFinishTalkingOrUserToClick();

        // Display a message box displaying a messsage that the message was recorded
        //				DoLapTopMessageBox( 10, AimPopUpText[
        // AIM_MEMBER_MESSAGE_RECORDED
        //], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL );
        InitCreateDeleteAimPopUpBox(AIM_POPUP_CREATE, L" ",
                                    AimPopUpText[AIM_MEMBER_MESSAGE_RECORDED], AIM_POPUP_BOX_X,
                                    AIM_POPUP_BOX_Y, AIM_POPUP_BOX_SUCCESS);

        SpecifyDisabledButtonStyle(giAnsweringMachineButton[1], DISABLED_STYLE_NONE);
        DisableButton(giAnsweringMachineButton[1]);
        DisableButton(giAnsweringMachineButton[0]);
      } else {
        gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
        //				gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
      }

      btn->uiFlags &= (~BUTTON_CLICKED_ON);
      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
}

void BtnHangUpButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(CONTACT_X, CONTACT_BOX_Y, CONTACT_BR_X, CONTACT_BR_Y);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      //			gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
      gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;

      btn->uiFlags &= (~BUTTON_CLICKED_ON);

      InvalidateRegion(CONTACT_X, CONTACT_BOX_Y, CONTACT_BR_X, CONTACT_BR_Y);
    }
  }
  if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
    btn->uiFlags &= (~BUTTON_CLICKED_ON);
    InvalidateRegion(CONTACT_X, CONTACT_BOX_Y, CONTACT_BR_X, CONTACT_BR_Y);
  }
}

// InitVideoFace() is called once to initialize things
BOOLEAN InitVideoFace(uint8_t ubMercID) {
  // Create the facial index
  giMercFaceIndex = InitFace(ubMercID, NOBODY, 0);

  SetAutoFaceActive(vsVideoFaceBackground, NULL, giMercFaceIndex, 0, 0);

  RenderAutoFace(giMercFaceIndex);

  gubCurrentStaticMode = VC_NO_STATIC;

  gfVideoFaceActive = TRUE;

  guiMercAttitudeTime = GetJA2Clock();

  return (TRUE);
}

// InitVideoFaceTalking() is called to start a merc speaking a particular message
BOOLEAN InitVideoFaceTalking(uint8_t ubMercID, uint16_t usQuoteNum) {
  // Starts the merc talking
  if (!CharacterDialogue(ubMercID, usQuoteNum, giMercFaceIndex, DIALOGUE_CONTACTPAGE_UI, FALSE,
                         FALSE)) {
    return (FALSE);
  }

  // Enables it so if a player clicks, he will shutup the merc
  MSYS_EnableRegion(&gSelectedShutUpMercRegion);

  gfIsShutUpMouseRegionActive = TRUE;
  gfMercIsTalking = TRUE;
  guiTimeThatMercStartedTalking = GetJA2Clock();
  return (TRUE);
}

BOOLEAN DisplayTalkingMercFaceForVideoPopUp(int32_t iFaceIndex) {
  static BOOLEAN fWasTheMercTalking = FALSE;
  BOOLEAN fIsTheMercTalking;
  SGPRect SrcRect;
  SGPRect DestRect;

  // Test
  SrcRect.iLeft = 0;
  SrcRect.iTop = 0;
  SrcRect.iRight = 48;
  SrcRect.iBottom = 43;

  DestRect.iLeft = AIM_MEMBER_VIDEO_FACE_X;
  DestRect.iTop = AIM_MEMBER_VIDEO_FACE_Y;
  DestRect.iRight = DestRect.iLeft + AIM_MEMBER_VIDEO_FACE_WIDTH;
  DestRect.iBottom = DestRect.iTop + AIM_MEMBER_VIDEO_FACE_HEIGHT;

  // If the answering machine graphics is up, dont handle the faces
  if (gfIsAnsweringMachineActive) {
    gFacesData[giMercFaceIndex].fInvalidAnim = TRUE;
  }

  HandleDialogue();
  HandleAutoFaces();
  HandleTalkingAutoFaces();

  // If the answering machine is up, dont display the face
  //	if( !gfIsAnsweringMachineActive )
  {
    // Blt the face surface to the video background surface
    if (!BltStretchVSurface(vsFB, vsVideoFaceBackground, &SrcRect, &DestRect)) return (FALSE);

    // if the merc is not at home and the players is leaving a message, shade the players face
    if (gfIsAnsweringMachineActive)
      ShadowVideoSurfaceRect(vsFB, DestRect.iLeft, DestRect.iTop, DestRect.iRight - 1,
                             DestRect.iBottom - 1);

    // If the answering machine graphics is up, place a message on the screen
    if (gfIsAnsweringMachineActive) {
      // display a message over the mercs face
      DisplayWrappedString(AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y + 20,
                           AIM_MEMBER_VIDEO_FACE_WIDTH, 2, FONT14ARIAL, 145,
                           AimPopUpText[AIM_MEMBER_PRERECORDED_MESSAGE], FONT_MCOLOR_BLACK, FALSE,
                           CENTER_JUSTIFIED);
    }

    InvalidateRegion(AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y,
                     AIM_MEMBER_VIDEO_FACE_X + AIM_MEMBER_VIDEO_FACE_WIDTH,
                     AIM_MEMBER_VIDEO_FACE_Y + AIM_MEMBER_VIDEO_FACE_HEIGHT);
  }

  fIsTheMercTalking = gFacesData[iFaceIndex].fTalking;

  // if the merc is talking, reset their attitude time
  if (fIsTheMercTalking) {
    // def added 3/18/99
    guiMercAttitudeTime = GetJA2Clock();
  }

  // if the text the merc is saying is really short, extend the time that it is on the screen
  if ((GetJA2Clock() - guiTimeThatMercStartedTalking) > usAimMercSpeechDuration) {
    // if the merc just stopped talking
    if (fWasTheMercTalking && !fIsTheMercTalking) {
      fWasTheMercTalking = FALSE;

      gfRedrawScreen = TRUE;
      guiMercAttitudeTime = GetJA2Clock();

      StopMercTalking();
    }
  } else if (fIsTheMercTalking) {
    fWasTheMercTalking = fIsTheMercTalking;
  }

  return (fIsTheMercTalking);
}

void DisplayTextForMercFaceVideoPopUp(wchar_t *pString) {
#ifdef TAIWANESE
  swprintf(gsTalkingMercText, ARR_SIZE(gsTalkingMercText), L"%s", pString);
#else
  swprintf(gsTalkingMercText, ARR_SIZE(gsTalkingMercText), L"\"%s\"", pString);
#endif

  // Set the minimum time for the dialogue text to be present
  usAimMercSpeechDuration = (uint16_t)(wcslen(gsTalkingMercText) * AIM_TEXT_SPEECH_MODIFIER);

  if (usAimMercSpeechDuration < MINIMUM_TALKING_TIME_FOR_MERC)
    usAimMercSpeechDuration = MINIMUM_TALKING_TIME_FOR_MERC;

  gfRedrawScreen = TRUE;
}

void SelectShutUpMercRegionCallBack(struct MOUSE_REGION *pRegion, int32_t iReason) {
  BOOLEAN fInCallBack = TRUE;

  if (fInCallBack) {
    if (iReason & MSYS_CALLBACK_REASON_INIT) {
    } else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
      gfStopMercFromTalking = TRUE;
    } else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
      fInCallBack = FALSE;

      gfStopMercFromTalking = TRUE;
      fInCallBack = TRUE;
    }
  }
}

uint8_t WillMercAcceptCall() {
  // if merc has hung up on the player twice within a period of time
  // (MERC_ANNOYED_WONT_CONTACT_TIME_MINUTES )the merc cant ber hired
  if (gMercProfiles[gbCurrentSoldier].bMercStatus == MERC_ANNOYED_WONT_CONTACT) {
    return (AIM_VIDEO_MERC_UNAVAILABLE_MODE);
  }

  // if the merc is currently on contract, the answering machine will pick up.
  if ((gMercProfiles[gbCurrentSoldier].bMercStatus > 0) ||
      (gMercProfiles[gbCurrentSoldier].bMercStatus == MERC_HAS_NO_TEXT_FILE) ||
      (gMercProfiles[gbCurrentSoldier].bMercStatus == MERC_HIRED_BUT_NOT_ARRIVED_YET)) {
    return (AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE);
  }

  // if the merc is at home, or if the merc is only slightly annoyed at the player,  he will greet
  // the player
  if (IsMercHireable(gbCurrentSoldier)) {
    return (AIM_VIDEO_FIRST_CONTACT_MERC_MODE);
  } else
    return (AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE);
}

BOOLEAN CanMercBeHired() {
  uint8_t i, j;
  int8_t bMercID;
  BOOLEAN fRetVal = FALSE;
  BOOLEAN fBuddyOnTeam = FALSE;

  StopMercTalking();

  // if the merc recently came back with poor morale, and hasn't gotten over it yet
  if (gMercProfiles[gbCurrentSoldier].ubDaysOfMoraleHangover > 0) {
    // then he refuses with a lame excuse.  Buddy or no buddy.
    WaitForMercToFinishTalkingOrUserToClick();
    InitVideoFaceTalking(gbCurrentSoldier, QUOTE_LAME_REFUSAL);
    return (FALSE);
  }

  // loop through the list of people the merc hates
  for (i = 0; i < NUMBER_HATED_MERCS_ONTEAM; i++) {
    // see if someone the merc hates is on the team
    bMercID = gMercProfiles[gbCurrentSoldier].bHated[i];

    if (bMercID < 0) continue;

    // if the hated merc is dead
    if (IsMercDead(bMercID)) {
      // ignore the merc
      continue;
    }

    if (IsMercOnTeamAndInOmertaAlready(bMercID)) {
      // if the merc hates someone on the team, see if a buddy is on the team
      for (j = 0; j < NUMBER_HATED_MERCS_ONTEAM; j++) {
        // if a buddy is on the team, the merc will join
        bMercID = gMercProfiles[gbCurrentSoldier].bBuddy[j];

        if (bMercID < 0) continue;

        if (IsMercOnTeam(bMercID) && !IsMercDead(bMercID)) {
          if (j == 0) {
            InitVideoFaceTalking(gbCurrentSoldier, QUOTE_JOINING_CAUSE_BUDDY_1_ON_TEAM);
          } else if (j == 1) {
            InitVideoFaceTalking(gbCurrentSoldier, QUOTE_JOINING_CAUSE_BUDDY_2_ON_TEAM);
          } else {
            InitVideoFaceTalking(gbCurrentSoldier,
                                 QUOTE_JOINING_CAUSE_LEARNED_TO_LIKE_BUDDY_ON_TEAM);
          }

          return (TRUE);
        }
      }

      // the merc doesnt like anybody on the team
      // if merc doesnt like first hated merc
      if (i == 0) {
        if (gMercProfiles[gbCurrentSoldier].bHatedTime[i] < 24) {
          WaitForMercToFinishTalkingOrUserToClick();
          InitVideoFaceTalking(gbCurrentSoldier, QUOTE_HATE_MERC_1_ON_TEAM);
          fRetVal = FALSE;
        } else {
          InitVideoFaceTalking(gbCurrentSoldier, QUOTE_PERSONALITY_BIAS_WITH_MERC_1);
          fRetVal = TRUE;
        }
      } else if (i == 1) {
        if (gMercProfiles[gbCurrentSoldier].bHatedTime[i] < 24) {
          WaitForMercToFinishTalkingOrUserToClick();
          InitVideoFaceTalking(gbCurrentSoldier, QUOTE_HATE_MERC_2_ON_TEAM);
          fRetVal = FALSE;
        } else {
          InitVideoFaceTalking(gbCurrentSoldier, QUOTE_PERSONALITY_BIAS_WITH_MERC_2);
          //					DelayMercSpeech( gbCurrentSoldier,
          // QUOTE_PERSONALITY_BIAS_WITH_MERC_2, 750, TRUE, FALSE );
          fRetVal = TRUE;
        }
      } else {
        WaitForMercToFinishTalkingOrUserToClick();
        InitVideoFaceTalking(gbCurrentSoldier, QUOTE_LEARNED_TO_HATE_MERC_ON_TEAM);
        fRetVal = FALSE;
      }

      return (fRetVal);
    }
  }

  // Is a buddy working on the team
  fBuddyOnTeam = DoesMercHaveABuddyOnTheTeam(gbCurrentSoldier);

  // If the merc doesnt have a buddy on the team
  if (!fBuddyOnTeam) {
    // Check the players Death rate
    if (MercThinksDeathRateTooHigh(gbCurrentSoldier)) {
      WaitForMercToFinishTalkingOrUserToClick();
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_DEATH_RATE_REFUSAL);
      return (FALSE);
    }

    // Check the players Reputation
    if (MercThinksBadReputationTooHigh(gbCurrentSoldier)) {
      WaitForMercToFinishTalkingOrUserToClick();
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_REPUTATION_REFUSAL);
      return (FALSE);
    }
  }

  return (TRUE);
}

BOOLEAN DisplaySnowBackground() {
  uint32_t uiCurrentTime = 0;
  struct VObject *hSnowHandle;
  uint8_t ubCount;

  uiCurrentTime = GetJA2Clock();

  if (gubCurrentCount < VC_NUM_LINES_SNOW) {
    ubCount = gubCurrentCount;
  } else if (gubCurrentCount < VC_NUM_LINES_SNOW * 2) {
    ubCount = gubCurrentCount - VC_NUM_LINES_SNOW;
  } else {
    gfFirstTimeInContactScreen = FALSE;
    gubCurrentCount = 0;
    ubCount = 0;

    if (gubVideoConferencingMode == AIM_VIDEO_FIRST_CONTACT_MERC_MODE &&
        gfAimMemberCanMercSayOpeningQuote)
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_GREETING);

    return (TRUE);
  }

  // if it is time to update the snow image
  if ((uiCurrentTime - guiLastHandleMercTime) > VC_CONTACT_STATIC_TIME) {
    gubCurrentCount++;
    guiLastHandleMercTime = uiCurrentTime;
  }
  // Get the snow background, and blit it
  GetVideoObject(&hSnowHandle, guiBWSnow);
  BltVideoObject(vsFB, hSnowHandle, ubCount, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y);

  InvalidateRegion(AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y,
                   AIM_MEMBER_VIDEO_FACE_X + AIM_MEMBER_VIDEO_FACE_WIDTH,
                   AIM_MEMBER_VIDEO_FACE_Y + AIM_MEMBER_VIDEO_FACE_HEIGHT);

  return (FALSE);
}

BOOLEAN DisplayBlackBackground(uint8_t ubMaxNumOfLoops) {
  uint32_t uiCurrentTime = 0;

  uiCurrentTime = GetJA2Clock();

  if (gubCurrentCount < ubMaxNumOfLoops) {
  } else {
    gubCurrentCount = 0;
    return (TRUE);
  }

  // if it is time to update the snow image
  if ((uiCurrentTime - guiLastHandleMercTime) > VC_CONTACT_STATIC_TIME) {
    gubCurrentCount++;
    guiLastHandleMercTime = uiCurrentTime;
  }
  // Blit color to screen
  ColorFillVSurfaceArea(vsFB, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y,
                        AIM_MEMBER_VIDEO_FACE_X + AIM_MEMBER_VIDEO_FACE_WIDTH,
                        AIM_MEMBER_VIDEO_FACE_Y + AIM_MEMBER_VIDEO_FACE_HEIGHT,
                        rgb32_to_rgb16(FROMRGB(0, 0, 0)));
  InvalidateRegion(AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y,
                   AIM_MEMBER_VIDEO_FACE_X + AIM_MEMBER_VIDEO_FACE_WIDTH,
                   AIM_MEMBER_VIDEO_FACE_Y + AIM_MEMBER_VIDEO_FACE_HEIGHT);

  return (FALSE);
}

void HandleVideoDistortion() {
  static uint32_t uiStaticNoiseSound = NO_SAMPLE;
  uint8_t ubOldMode = gubCurrentStaticMode;

  // if we are just entering the contact page, display a snowy background
  if (gfFirstTimeInContactScreen && !gfIsAnsweringMachineActive) {
    DisplaySnowBackground();

    // if it is time to start playing another sound
    if (uiStaticNoiseSound == NO_SAMPLE) {
      uiStaticNoiseSound =
          PlayJA2SampleFromFile("LAPTOP\\static4.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
    }
  } else {
    switch (gubCurrentStaticMode) {
      case VC_NO_STATIC: {
        static uint32_t uiCurTime = 0;
        uint8_t ubNum;

        // if the sound is playing, stop it
        if (uiStaticNoiseSound != NO_SAMPLE) {
          SoundStop(uiStaticNoiseSound);
          uiStaticNoiseSound = NO_SAMPLE;
        }

        // DECIDE WHICH ONE TO BLIT NEXT
        if ((GetJA2Clock() - uiCurTime) > 2500) {
          ubNum = (uint8_t)Random(200);  // 125;

          if (ubNum < 15)
            gubCurrentStaticMode = VC_FUZZY_LINE;

          else if (ubNum < 25)
            gubCurrentStaticMode = VC_STRAIGHTLINE;

          else if (ubNum < 35)
            gubCurrentStaticMode = VC_BW_SNOW;

          else if (ubNum < 40)
            gubCurrentStaticMode = VC_PIXELATE;

          else if (ubNum < 80)
            gubCurrentStaticMode = VC_TRANS_SNOW_OUT;

          else if (ubNum < 100)
            gubCurrentStaticMode = VC_TRANS_SNOW_IN;

          uiCurTime = GetJA2Clock();
        }
      } break;

      case VC_FUZZY_LINE:
        gubCurrentStaticMode = DisplayDistortionLine(VC_FUZZY_LINE, guiFuzzLine, VC_NUM_FUZZ_LINES);

        // if it is time to start playing another sound
        if (uiStaticNoiseSound == NO_SAMPLE) {
          uiStaticNoiseSound =
              PlayJA2SampleFromFile("LAPTOP\\static1.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
        }
        break;

      case VC_STRAIGHTLINE:
        gubCurrentStaticMode =
            DisplayDistortionLine(VC_STRAIGHTLINE, guiStraightLine, VC_NUM_STRAIGHT_LINES);

        // if it is time to start playing another sound
        if (uiStaticNoiseSound == NO_SAMPLE) {
          uiStaticNoiseSound =
              PlayJA2SampleFromFile("LAPTOP\\static5.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
        }
        break;

      case VC_BW_SNOW:
        gubCurrentStaticMode = DisplayDistortionLine(VC_BW_SNOW, guiBWSnow, 5);

        // if it is time to start playing another sound
        if (uiStaticNoiseSound == NO_SAMPLE) {
          uiStaticNoiseSound =
              PlayJA2SampleFromFile("LAPTOP\\static6.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
        }
        break;

      case VC_PIXELATE:
        gubCurrentStaticMode = DisplayPixelatedImage(4);

        // if it is time to start playing another sound
        if (uiStaticNoiseSound == NO_SAMPLE) {
          uiStaticNoiseSound =
              PlayJA2SampleFromFile("LAPTOP\\static3.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
        }
        break;

      case VC_TRANS_SNOW_OUT:
        gubCurrentStaticMode = DisplayTransparentSnow(VC_TRANS_SNOW_OUT, guiTransSnow, 7, FALSE);

        // if it is time to start playing another sound
        if (uiStaticNoiseSound == NO_SAMPLE) {
          uiStaticNoiseSound =
              PlayJA2SampleFromFile("LAPTOP\\static5.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
        }
        break;

      case VC_TRANS_SNOW_IN:
        gubCurrentStaticMode = DisplayTransparentSnow(VC_TRANS_SNOW_IN, guiTransSnow, 7, TRUE);

        // if it is time to start playing another sound
        if (uiStaticNoiseSound == NO_SAMPLE) {
          uiStaticNoiseSound =
              PlayJA2SampleFromFile("LAPTOP\\static4.wav", RATE_11025, LOWVOLUME, 1, MIDDLEPAN);
        }
        break;
    }

    if (ubOldMode != gubCurrentStaticMode) {
      uiStaticNoiseSound = NO_SAMPLE;
    }
  }
}

// returns true when done. else false
uint8_t DisplayTransparentSnow(uint8_t ubMode, uint32_t uiImageIdentifier, uint8_t ubMaxImages,
                               BOOLEAN bForward) {
  struct VObject *hFuzzLineHandle;
  static int8_t bCount = 0;
  uint32_t uiCurrentTime = 0;
  static uint32_t uiLastTime = 0;

  uiCurrentTime = GetJA2Clock();

  if ((uiCurrentTime - uiLastTime) > 100) {
    if (bForward) {
      if (bCount > ubMaxImages - 1)
        bCount = 0;
      else
        bCount++;
    } else {
      if (bCount <= 0)
        bCount = ubMaxImages - 1;
      else
        bCount--;
    }
    uiLastTime = uiCurrentTime;
  }

  if (bCount >= ubMaxImages) bCount = ubMaxImages - 1;

  // Get the snow background, and blit it
  GetVideoObject(&hFuzzLineHandle, uiImageIdentifier);
  BltVideoObject(vsFB, hFuzzLineHandle, bCount, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y);

  if (bForward) {
    if (bCount == ubMaxImages - 1) {
      bCount = 0;
      return (VC_BW_SNOW);
    } else
      return (ubMode);
  } else {
    if (bCount == 0) {
      bCount = 0;
      return (VC_NO_STATIC);
    } else
      return (ubMode);
  }
}

// returns true when done. else false
uint8_t DisplayDistortionLine(uint8_t ubMode, uint32_t uiImageIdentifier, uint8_t ubMaxImages) {
  struct VObject *hFuzzLineHandle;
  static uint8_t ubCount = 255;
  uint32_t uiCurrentTime = 0;
  static uint32_t uiLastTime = 0;

  uiCurrentTime = GetJA2Clock();

  if ((uiCurrentTime - uiLastTime) > VC_CONTACT_FUZZY_LINE_TIME) {
    if (ubCount >= ubMaxImages - 1)
      ubCount = 0;
    else
      ubCount++;
    uiLastTime = uiCurrentTime;
  }

  if (ubCount >= ubMaxImages) ubCount = ubMaxImages - 1;

  // Get the snow background, and blit it
  GetVideoObject(&hFuzzLineHandle, uiImageIdentifier);
  BltVideoObject(vsFB, hFuzzLineHandle, ubCount, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y);

  if (ubCount == ubMaxImages - 1) {
    ubCount = 0;
    if (ubMode == VC_BW_SNOW)
      return (VC_TRANS_SNOW_OUT);
    else
      return (VC_NO_STATIC);
  } else
    return (ubMode);
}

uint8_t DisplayPixelatedImage(uint8_t ubMaxImages) {
  static uint8_t ubCount = 255;
  uint32_t uiCurrentTime = 0;
  static uint32_t uiLastTime = 0;

  uiCurrentTime = GetJA2Clock();

  if ((uiCurrentTime - uiLastTime) > VC_CONTACT_FUZZY_LINE_TIME) {
    if (ubCount >= ubMaxImages - 1)
      ubCount = 0;
    else
      ubCount++;
    uiLastTime = uiCurrentTime;
  }

  ShadowVideoSurfaceRect(vsFB, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y,
                         AIM_MEMBER_VIDEO_FACE_X + AIM_MEMBER_VIDEO_FACE_WIDTH - 1,
                         AIM_MEMBER_VIDEO_FACE_Y + AIM_MEMBER_VIDEO_FACE_HEIGHT - 1);

  if (ubCount == ubMaxImages - 1) {
    ubCount = 0;
    return (VC_NO_STATIC);
  } else
    return (VC_PIXELATE);
}

void HandleMercAttitude() {
  uint32_t uiCurrentTime = 0;

  uiCurrentTime = GetJA2Clock();

  if ((gubMercAttitudeLevel <= 1 &&
       ((uiCurrentTime - guiMercAttitudeTime) > QUOTE_FIRST_ATTITUDE_TIME)) ||
      ((uiCurrentTime - guiMercAttitudeTime) > QUOTE_ATTITUDE_TIME)) {
    if (gubMercAttitudeLevel == QUOTE_DELAY_SMALL_TALK) {
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_SMALL_TALK);
    } else if (gubMercAttitudeLevel == QUOTE_DELAY_IMPATIENT_TALK) {
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_IMPATIENT_QUOTE);
    } else if (gubMercAttitudeLevel == QUOTE_DELAY_VERY_IMPATIENT_TALK) {
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_PRECEDENT_TO_REPEATING_ONESELF);
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_IMPATIENT_QUOTE);
    } else if (gubMercAttitudeLevel == QUOTE_DELAY_HANGUP_TALK) {
      uint32_t uiResetTime;
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_COMMENT_BEFORE_HANG_UP);

      // if the merc is going to hang up disable the buttons, so user cant press any buttons
      //			EnableDisableCurrentVideoConferenceButtons( FALSE);
      if (gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE) {
        // Enable the current video conference buttons
        EnableDisableCurrentVideoConferenceButtons(FALSE);
      }

      // increments the merc 'annoyance' at the player
      if (gMercProfiles[gbCurrentSoldier].bMercStatus == 0)
        gMercProfiles[gbCurrentSoldier].bMercStatus = MERC_ANNOYED_BUT_CAN_STILL_CONTACT;
      else if (gMercProfiles[gbCurrentSoldier].bMercStatus == MERC_ANNOYED_BUT_CAN_STILL_CONTACT)
        gMercProfiles[gbCurrentSoldier].bMercStatus = MERC_ANNOYED_WONT_CONTACT;

      // add an event so we can reset the 'annoyance factor'
      uiResetTime = (Random(600));
      uiResetTime += GetWorldTotalMin() + MERC_ANNOYED_WONT_CONTACT_TIME_MINUTES;
      AddStrategicEvent(EVENT_AIM_RESET_MERC_ANNOYANCE, uiResetTime, gbCurrentSoldier);

      gfHangUpMerc = TRUE;
    }

    if (gubMercAttitudeLevel == QUOTE_MERC_BUSY) {
      InitVideoFaceTalking(gbCurrentSoldier, QUOTE_LAME_REFUSAL);
      gfHangUpMerc = TRUE;
    } else if (gubMercAttitudeLevel != QUOTE_DELAY_NO_ACTION)
      gubMercAttitudeLevel++;

    guiMercAttitudeTime = GetJA2Clock();
  }
}

void StopMercTalking() {
  if (gfIsShutUpMouseRegionActive) {
    MSYS_DisableRegion(&gSelectedShutUpMercRegion);

    ShutupaYoFace(giMercFaceIndex);
    gfMercIsTalking = FALSE;
    guiMercAttitudeTime = GetJA2Clock();
    gfIsShutUpMouseRegionActive = FALSE;
    gfRedrawScreen = TRUE;
  }
}

void BtnXToCloseVideoConfButtonCallback(GUI_BUTTON *btn, int32_t reason) {
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN) {
    btn->uiFlags |= BUTTON_CLICKED_ON;
    InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                     btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
  }
  if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP) {
    if (btn->uiFlags & BUTTON_CLICKED_ON) {
      gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
      //			gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
      btn->uiFlags &= (~BUTTON_CLICKED_ON);
      InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY,
                       btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
    }
  }
}

BOOLEAN InitDeleteVideoConferencePopUp() {
  static BOOLEAN fXRegionActive = FALSE;
  uint8_t i;
  uint16_t usPosX, usPosY;

  // remove the face help text
  gfAimMemberDisplayFaceHelpText = FALSE;

  // Gets reset to FALSE in the HandleCurrentVideoConfMode() function
  gfJustSwitchedVideoConferenceMode = TRUE;

  // remove old mode
  DeleteVideoConfPopUp();

  // reset ( in case merc was going to say something
  DelayMercSpeech(0, 0, 0, FALSE, TRUE);

  // if the video conferencing is currently displayed, put the 'x' to close it in the top right
  // corner and disable the ability to click on the BIG face to go to different screen
  if ((gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE) &&
      (gubVideoConferencingMode != AIM_VIDEO_POPUP_MODE)) {
    if (!fXRegionActive) {
      giXToCloseVideoConfButton =
          QuickCreateButton(giXToCloseVideoConfButtonImage, AIM_MEMBER_VIDEO_CONF_XCLOSE_X,
                            AIM_MEMBER_VIDEO_CONF_XCLOSE_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
                            DEFAULT_MOVE_CALLBACK, BtnXToCloseVideoConfButtonCallback);
      SetButtonCursor(giXToCloseVideoConfButton, CURSOR_LAPTOP_SCREEN);
      SpecifyDisabledButtonStyle(giXToCloseVideoConfButton, DISABLED_STYLE_NONE);
      fXRegionActive = TRUE;

      MSYS_DisableRegion(&gSelectedFaceRegion);
    }
  }

  // The video conference is not displayed
  if (gubVideoConferencingMode == AIM_VIDEO_NOT_DISPLAYED_MODE) {
    gubVideoConferencingPreviousMode = gubVideoConferencingMode;
    gfRedrawScreen = TRUE;

    if (gfVideoFaceActive) {
      StopMercTalking();

      // Get rid of the talking face
      DeleteFace(giMercFaceIndex);
    }

    // if the ansering machine is currently on, turn it off
    if (gfIsAnsweringMachineActive) gfIsAnsweringMachineActive = FALSE;

    gfVideoFaceActive = FALSE;

    if (fXRegionActive) {
      RemoveButton(giXToCloseVideoConfButton);
      fXRegionActive = FALSE;
    }

    MSYS_DisableRegion(&gSelectedShutUpMercRegion);

    // Enable the ability to click on the BIG face to go to different screen
    MSYS_EnableRegion(&gSelectedFaceRegion);

    //		EnableDisableCurrentVideoConferenceButtons(FALSE);
    if (gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE) {
      // Enable the current video conference buttons
      EnableDisableCurrentVideoConferenceButtons(FALSE);
    }

    fNewMailFlag = gfIsNewMailFlagSet;
    gfIsNewMailFlagSet = FALSE;
  }

  if (gubVideoConferencingMode == AIM_VIDEO_POPUP_MODE) {
    gubVideoConferencingPreviousMode = gubVideoConferencingMode;

    if (gfJustSwitchedVideoConferenceMode) {
      uint32_t uiVideoBackgroundGraphic;
      struct VObject *hImageHandle;

      // load the answering machine graphic and add it
      CHECKF(AddVObject(CreateVObjectFromFile("LAPTOP\\VideoTitleBar.sti"),
                        &uiVideoBackgroundGraphic));

      // Create a background video surface to blt the face onto
      vsVideoTitleBar =
          JSurface_Create16bpp(AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH, AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT);
      if (vsVideoTitleBar == NULL) {
        return FALSE;
      }
      JSurface_SetColorKey(vsVideoTitleBar, FROMRGB(0, 0, 0));

      gfAimMemberCanMercSayOpeningQuote = TRUE;

      GetVideoObject(&hImageHandle, uiVideoBackgroundGraphic);
      BltVideoObject(vsVideoTitleBar, hImageHandle, 0, 0, 0);

      DeleteVideoObjectFromIndex(uiVideoBackgroundGraphic);
    }
  }

  // The opening animation of the vc (fuzzy screen, then goes to black)
  if (gubVideoConferencingMode == AIM_VIDEO_INIT_MODE) {
    gubVideoConferencingPreviousMode = gubVideoConferencingMode;
    gubMercAttitudeLevel = 0;
    gubContractLength = AIM_CONTRACT_LENGTH_ONE_WEEK;

    if (gMercProfiles[gbCurrentSoldier].usOptionalGearCost == 0)
      gfBuyEquipment = FALSE;
    else
      gfBuyEquipment = TRUE;

    gfMercIsTalking = FALSE;
    gfVideoFaceActive = FALSE;
    guiLastHandleMercTime = 0;
    gfHangUpMerc = FALSE;
  }

  // The screen in which you first contact the merc, you have the option to hang up or goto hire
  // merc screen
  if (gubVideoConferencingMode == AIM_VIDEO_FIRST_CONTACT_MERC_MODE) {
    // if the last screen was the init screen, then we need to initialize the video face
    if ((gubVideoConferencingPreviousMode == AIM_VIDEO_INIT_MODE) ||
        (gubVideoConferencingPreviousMode == AIM_VIDEO_NOT_DISPLAYED_MODE)) {
      // Put the merc face up on the screen
      InitVideoFace(gbCurrentSoldier);

      //			if( gubVideoConferencingPreviousMode == AIM_VIDEO_INIT_MODE)
      //				InitVideoFaceTalking(gbCurrentSoldier, QUOTE_GREETING);
    }

    gubVideoConferencingPreviousMode = gubVideoConferencingMode;

    // Hang up button
    usPosX = AIM_MEMBER_AUTHORIZE_PAY_X;
    guiVideoConferenceButtonImage[2] =
        LoadButtonImage("LAPTOP\\VideoConfButtons.sti", -1, 2, -1, 3, -1);
    for (i = 0; i < 2; i++) {
      giAuthorizeButton[i] = CreateIconAndTextButton(
          guiVideoConferenceButtonImage[2], VideoConfercingText[i + AIM_MEMBER_HIRE], FONT12ARIAL,
          AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
          AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_CJUSTIFIED, usPosX, AIM_MEMBER_HANG_UP_Y,
          BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnFirstContactButtonCallback);

      MSYS_SetBtnUserData(giAuthorizeButton[i], 0, i);
      SetButtonCursor(giAuthorizeButton[i], CURSOR_LAPTOP_SCREEN);
      usPosX += AIM_MEMBER_AUTHORIZE_PAY_GAP;
    }

    if (gfWaitingForMercToStopTalkingOrUserToClick) {
      DisableButton(giAuthorizeButton[0]);
      gfWaitingForMercToStopTalkingOrUserToClick = FALSE;

      // Display a popup msg box telling the user when and where the merc will arrive
      //			DisplayPopUpBoxExplainingMercArrivalLocationAndTime(
      // giIdOfLastHiredMerc );
      giIdOfLastHiredMerc = -1;
    }
  }

  // The screen in which you set the contract length, and the ability to buy equipment..
  if (gubVideoConferencingMode == AIM_VIDEO_HIRE_MERC_MODE) {
    gubVideoConferencingPreviousMode = gubVideoConferencingMode;

    // Contract Length button
    guiVideoConferenceButtonImage[0] =
        LoadButtonImage("LAPTOP\\VideoConfButtons.sti", -1, 0, -1, 1, -1);
    usPosY = AIM_MEMBER_BUY_CONTRACT_LENGTH_Y;
    for (i = 0; i < 3; i++) {
      giContractLengthButton[i] = CreateIconAndTextButton(
          guiVideoConferenceButtonImage[0], VideoConfercingText[i + AIM_MEMBER_ONE_DAY],
          FONT12ARIAL, AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
          AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_LJUSTIFIED, AIM_MEMBER_BUY_CONTRACT_LENGTH_X, usPosY,
          BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
          BtnContractLengthButtonCallback);

      SetButtonCursor(giContractLengthButton[i], CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(giContractLengthButton[i], 0, i);
      SpecifyDisabledButtonStyle(giContractLengthButton[i], DISABLED_STYLE_NONE);
      usPosY += AIM_MEMBER_BUY_EQUIPMENT_GAP;
    }

    // BuyEquipment button
    usPosY = AIM_MEMBER_BUY_CONTRACT_LENGTH_Y;
    for (i = 0; i < 2; i++) {
      giBuyEquipmentButton[i] = CreateIconAndTextButton(
          guiVideoConferenceButtonImage[0], VideoConfercingText[i + AIM_MEMBER_NO_EQUIPMENT],
          FONT12ARIAL, AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
          AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_LJUSTIFIED, AIM_MEMBER_BUY_EQUIPMENT_X, usPosY,
          BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnBuyEquipmentButtonCallback);

      SetButtonCursor(giBuyEquipmentButton[i], CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(giBuyEquipmentButton[i], 0, i);
      SpecifyDisabledButtonStyle(giBuyEquipmentButton[i], DISABLED_STYLE_SHADED);
      usPosY += AIM_MEMBER_BUY_EQUIPMENT_GAP;
    }
    if (gMercProfiles[gbCurrentSoldier].usOptionalGearCost == 0)
      DisableButton(giBuyEquipmentButton[1]);

    // Authorize button
    usPosX = AIM_MEMBER_AUTHORIZE_PAY_X;
    guiVideoConferenceButtonImage[1] =
        LoadButtonImage("LAPTOP\\VideoConfButtons.sti", -1, 2, -1, 3, -1);
    for (i = 0; i < 2; i++) {
      giAuthorizeButton[i] = CreateIconAndTextButton(
          guiVideoConferenceButtonImage[1], VideoConfercingText[i + AIM_MEMBER_TRANSFER_FUNDS],
          FONT12ARIAL, AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
          AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_CJUSTIFIED, usPosX, AIM_MEMBER_AUTHORIZE_PAY_Y,
          BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnAuthorizeButtonCallback);

      SetButtonCursor(giAuthorizeButton[i], CURSOR_LAPTOP_SCREEN);
      MSYS_SetBtnUserData(giAuthorizeButton[i], 0, i);
      SpecifyDisabledButtonStyle(giAuthorizeButton[i], DISABLED_STYLE_NONE);
      usPosX += AIM_MEMBER_AUTHORIZE_PAY_GAP;
    }

    //		InitVideoFaceTalking(gbCurrentSoldier, QUOTE_LENGTH_OF_CONTRACT);
    DelayMercSpeech(gbCurrentSoldier, QUOTE_LENGTH_OF_CONTRACT, 750, TRUE, FALSE);
  }

  // The merc is not home and the player gets the answering machine
  if (gubVideoConferencingMode == AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE) {
    gubVideoConferencingPreviousMode = gubVideoConferencingMode;

    gfIsAnsweringMachineActive = TRUE;

    // Leave msg button
    usPosX = AIM_MEMBER_AUTHORIZE_PAY_X;
    guiVideoConferenceButtonImage[2] =
        LoadButtonImage("LAPTOP\\VideoConfButtons.sti", -1, 2, -1, 3, -1);

    giAnsweringMachineButton[0] = CreateIconAndTextButton(
        guiVideoConferenceButtonImage[2], VideoConfercingText[AIM_MEMBER_LEAVE_MESSAGE],
        FONT12ARIAL, AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
        AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_CJUSTIFIED, usPosX, AIM_MEMBER_HANG_UP_Y, BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnAnsweringMachineButtonCallback);
    MSYS_SetBtnUserData(giAnsweringMachineButton[0], 0, 0);
    SetButtonCursor(giAnsweringMachineButton[0], CURSOR_LAPTOP_SCREEN);

    // if the user has already left a message, disable the button
    if (gMercProfiles[gbCurrentSoldier].ubMiscFlags3 &
        PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM)
      DisableButton(giAnsweringMachineButton[0]);

    usPosX += AIM_MEMBER_AUTHORIZE_PAY_GAP;

    giAnsweringMachineButton[1] = CreateIconAndTextButton(
        guiVideoConferenceButtonImage[2], VideoConfercingText[AIM_MEMBER_HANG_UP], FONT12ARIAL,
        AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
        AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_CJUSTIFIED, usPosX, AIM_MEMBER_HANG_UP_Y, BUTTON_TOGGLE,
        MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnAnsweringMachineButtonCallback);

    MSYS_SetBtnUserData(giAnsweringMachineButton[1], 0, 1);
    SetButtonCursor(giAnsweringMachineButton[1], CURSOR_LAPTOP_SCREEN);

    // The face must be inited even though the face wont appear.  It is so the voice is played
    InitVideoFace(gbCurrentSoldier);

    // Make sure the merc doesnt ramble away to the player
    gubMercAttitudeLevel = QUOTE_DELAY_NO_ACTION;

    gubCurrentStaticMode = VC_NO_STATIC;
  }

  // The merc is home but for some reason doesnt want to work for player
  if (gubVideoConferencingMode == AIM_VIDEO_MERC_UNAVAILABLE_MODE) {
    gubVideoConferencingPreviousMode = gubVideoConferencingMode;

    // The hangup button
    guiVideoConferenceButtonImage[2] =
        LoadButtonImage("LAPTOP\\VideoConfButtons.sti", -1, 2, -1, 3, -1);

    giHangUpButton = CreateIconAndTextButton(
        guiVideoConferenceButtonImage[2], VideoConfercingText[AIM_MEMBER_HANG_UP], FONT12ARIAL,
        AIM_M_VIDEO_NAME_COLOR, AIM_M_VIDEO_NAME_SHADOWCOLOR, AIM_M_VIDEO_NAME_COLOR,
        AIM_M_VIDEO_NAME_SHADOWCOLOR, TEXT_CJUSTIFIED, AIM_MEMBER_HANG_UP_X, AIM_MEMBER_HANG_UP_Y,
        BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK, BtnHangUpButtonCallback);

    MSYS_SetBtnUserData(giHangUpButton, 0, 1);
    SetButtonCursor(giHangUpButton, CURSOR_LAPTOP_SCREEN);

    // set the flag saying specifying that merc is busy
    gubMercAttitudeLevel = QUOTE_MERC_BUSY;

    InitVideoFace(gbCurrentSoldier);
  }

  if (gubVideoConferencingMode == AIM_VIDEO_POPDOWN_MODE) {
    uint32_t uiVideoBackgroundGraphic;
    struct VObject *hImageHandle;

    if (gubPopUpBoxAction == AIM_POPUP_DISPLAY) {
      return (TRUE);
    }

    gubVideoConferencingPreviousMode = gubVideoConferencingMode;

    gfIsAnsweringMachineActive = FALSE;

    // load the Video conference background graphic and add it
    CHECKF(
        AddVObject(CreateVObjectFromFile("LAPTOP\\VideoTitleBar.sti"), &uiVideoBackgroundGraphic));

    // Create a background video surface to blt the face onto
    vsVideoTitleBar =
        JSurface_Create16bpp(AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH, AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT);
    JSurface_SetColorKey(vsVideoTitleBar, FROMRGB(0, 0, 0));
    if (vsVideoTitleBar == NULL) {
      return FALSE;
    }

    GetVideoObject(&hImageHandle, uiVideoBackgroundGraphic);
    BltVideoObject(vsVideoTitleBar, hImageHandle, 0, 0, 0);

    DeleteVideoObjectFromIndex(uiVideoBackgroundGraphic);
  }

  //	gfWaitingForMercToStopTalkingOrUserToClick = FALSE;

  // reset the time in which the merc will get annoyed
  guiMercAttitudeTime = GetJA2Clock();
  return (TRUE);
}

BOOLEAN DeleteVideoConfPopUp() {
  uint16_t i;

  // reset ( in case merc was going to say something
  DelayMercSpeech(0, 0, 0, FALSE, TRUE);

  switch (gubVideoConferencingPreviousMode) {
    // The video conference is not displayed
    case AIM_VIDEO_NOT_DISPLAYED_MODE: {
      break;
    }

    case AIM_VIDEO_POPUP_MODE: {
      JSurface_Free(vsVideoTitleBar);
      vsVideoTitleBar = NULL;
      break;
    }

    // The opening animation of the vc (fuzzy screen, then goes to black)
    case AIM_VIDEO_INIT_MODE: {
      break;
    }

    // The screen in which you first contact the merc, you have the option to hang up or goto hire
    // merc screen
    case AIM_VIDEO_FIRST_CONTACT_MERC_MODE: {
      // Remove the video conf buttons images
      UnloadButtonImage(guiVideoConferenceButtonImage[2]);

      // Remove the Hangup  buttons
      for (i = 0; i < 2; i++) RemoveButton(giAuthorizeButton[i]);

      break;
    }

    // The screen in which you set the contract length, and the ability to buy equipment..
    case AIM_VIDEO_HIRE_MERC_MODE: {
      // Remove the video conf buttons images
      for (i = 0; i < 2; i++) UnloadButtonImage(guiVideoConferenceButtonImage[i]);

      // Remove the Contracy Length button
      for (i = 0; i < 3; i++) RemoveButton(giContractLengthButton[i]);

      for (i = 0; i < 2; i++) RemoveButton(giBuyEquipmentButton[i]);

      for (i = 0; i < 2; i++) RemoveButton(giAuthorizeButton[i]);

      break;
    }

    // The merc is not home and the player gets the answering machine
    case AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE: {
      if (gubPopUpBoxAction == AIM_POPUP_DISPLAY) {
        //				return( TRUE );
      }

      // Remove the video conf buttons images
      UnloadButtonImage(guiVideoConferenceButtonImage[2]);

      // Remove the Answering machine buttons
      for (i = 0; i < 2; i++) RemoveButton(giAnsweringMachineButton[i]);

      //			DeleteVideoObjectFromIndex(guiAnsweringMachineImage);
      break;
    }

    // The merc is home but doesnt want to work for player
    case AIM_VIDEO_MERC_UNAVAILABLE_MODE: {
      RemoveButton(giHangUpButton);
      UnloadButtonImage(guiVideoConferenceButtonImage[2]);
      break;
    }

    case AIM_VIDEO_POPDOWN_MODE: {
      if (gubPopUpBoxAction == AIM_POPUP_DISPLAY) {
        return (TRUE);
      }

      if (gfWaitingForMercToStopTalkingOrUserToClick) {
        gfWaitingForMercToStopTalkingOrUserToClick = FALSE;

        //				DisplayPopUpBoxExplainingMercArrivalLocationAndTime(
        // giIdOfLastHiredMerc );
      }

      gfWaitingForMercToStopTalkingOrUserToClick = FALSE;
      JSurface_Free(vsVideoTitleBar);
      vsVideoTitleBar = NULL;
      break;
    }
  }
  return (TRUE);
}

BOOLEAN HandleCurrentVideoConfMode() {
  switch (gubVideoConferencingMode) {
    // The video conference is not displayed
    case AIM_VIDEO_NOT_DISPLAYED_MODE: {
      gfWaitingForMercToStopTalkingOrUserToClick = FALSE;

      break;
    }

    case AIM_VIDEO_POPUP_MODE: {
      BOOLEAN ubDone;

      if (gfJustSwitchedVideoConferenceMode)
        ubDone = DisplayMovingTitleBar(TRUE, TRUE);
      else
        ubDone = DisplayMovingTitleBar(TRUE, FALSE);

      if (ubDone) gubVideoConferencingMode = AIM_VIDEO_INIT_MODE;

      break;
    }

    // The opening animation of the vc (fuzzy screen, then goes to black)
    case AIM_VIDEO_INIT_MODE: {
      static uint8_t ubCurMode = 0;
      BOOLEAN fDone;

      if (ubCurMode == 0) {
        fDone = DisplayBlackBackground(10);
        if (fDone) ubCurMode = 1;
      } else
        fDone = DisplaySnowBackground();

      if (fDone && ubCurMode) {
        ubCurMode = 0;

        gubVideoConferencingMode = WillMercAcceptCall();
      }

      break;
    }

    // The screen in which you first contact the merc, you have the option to hang up or goto hire
    // merc screen
    case AIM_VIDEO_FIRST_CONTACT_MERC_MODE: {
      // if the merc is at home, play his greeting
      //			if( gfJustSwitchedVideoConferenceMode )
      //				InitVideoFaceTalking(gbCurrentSoldier, QUOTE_GREETING);

      break;
    }

    // The screen in which you set the contract length, and the ability to buy equipment..
    case AIM_VIDEO_HIRE_MERC_MODE: {
      break;
    }

    // The merc is not home and the player gets the answering machine
    case AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE: {
      // if the merc is not at home, play his answering machine
      if (gfJustSwitchedVideoConferenceMode) {
        InitVideoFaceTalking(gbCurrentSoldier, QUOTE_ANSWERING_MACHINE_MSG);
      }

      break;
    }

    // The merc is home but doesnt want to work for player
    case AIM_VIDEO_MERC_UNAVAILABLE_MODE: {
      break;
    }

    case AIM_VIDEO_POPDOWN_MODE: {
      BOOLEAN ubDone;

      if (gfJustSwitchedVideoConferenceMode)
        ubDone = DisplayMovingTitleBar(FALSE, TRUE);
      else
        ubDone = DisplayMovingTitleBar(FALSE, FALSE);

      if (ubDone) {
        gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;

        // display the popup telling the user when the just hired merc is going to land
        DisplayPopUpBoxExplainingMercArrivalLocationAndTime();

        // render the screen immediately to get rid of the pop down stuff
        InitDeleteVideoConferencePopUp();
        RenderAIMMembers();
        gfVideoFaceActive = FALSE;
      }

      break;
    }
  }

  // Gets set in the InitDeleteVideoConferencePopUp() function
  //	gfJustSwitchedVideoConferenceMode = FALSE;

  return (TRUE);
}

BOOLEAN EnableDisableCurrentVideoConferenceButtons(BOOLEAN fEnable) {
  int8_t i;
  static BOOLEAN fCreated = FALSE;
  if (!fEnable) {
    if (fCreated) {
      // enable buttons behind the acknowlegde button

      for (i = 0; i < 3; i++) EnableButton(giContractLengthButton[i]);

      for (i = 0; i < 2; i++) EnableButton(giBuyEquipmentButton[i]);

      for (i = 0; i < 2; i++) EnableButton(giAuthorizeButton[i]);

      fCreated = FALSE;
    }
  } else {
    if (!fCreated) {
      // disable buttons behind the acknowlegde button
      for (i = 0; i < 3; i++) DisableButton(giContractLengthButton[i]);

      for (i = 0; i < 2; i++) DisableButton(giBuyEquipmentButton[i]);

      for (i = 0; i < 2; i++) DisableButton(giAuthorizeButton[i]);

      fCreated = TRUE;
    }
  }
  return (TRUE);
}

void ResetMercAnnoyanceAtPlayer(uint8_t ubMercID) {
  // if merc is still annoyed, reset back to 0

  if (ubMercID == LARRY_NORMAL) {
    if (CheckFact(FACT_LARRY_CHANGED, 0)) {
      ubMercID = LARRY_DRUNK;
    }
  } else if (ubMercID == LARRY_DRUNK) {
    if (CheckFact(FACT_LARRY_CHANGED, 0) == FALSE) {
      ubMercID = LARRY_NORMAL;
    }
  }
  if ((gMercProfiles[ubMercID].bMercStatus == MERC_ANNOYED_WONT_CONTACT) ||
      (gMercProfiles[ubMercID].bMercStatus == MERC_ANNOYED_BUT_CAN_STILL_CONTACT))
    gMercProfiles[ubMercID].bMercStatus = 0;
}

BOOLEAN DisableNewMailMessage() {
  if (fNewMailFlag && gubVideoConferencingMode) {
    gfIsNewMailFlagSet = TRUE;
    fNewMailFlag = FALSE;
    gfRedrawScreen = TRUE;

    return (TRUE);
  }
  return (FALSE);
}

BOOLEAN DisplayMovingTitleBar(BOOLEAN fForward, BOOLEAN fInit) {
  static uint8_t ubCount;
  uint16_t usPosX, usPosY, usPosRightX, usPosBottomY, usWidth, usHeight;
  SGPRect SrcRect;
  SGPRect DestRect;
  static SGPRect LastRect;
  float usTemp;

  if (fForward) {
    if (fInit) ubCount = 1;

    usTemp = (331 - 125) / (float)AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
    usPosX = (uint16_t)(331 - usTemp * ubCount);

    usTemp = (490 - 405) / (float)AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
    usPosRightX = (uint16_t)(405 + usTemp * ubCount);

    usTemp = (AIM_MEMBER_VIDEO_TITLE_START_Y - 96) / (float)AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
    usPosY = (uint16_t)(AIM_MEMBER_VIDEO_TITLE_START_Y - usTemp * ubCount);

    usPosBottomY = AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT;
  } else {
    if (fInit) ubCount = AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 1;

    usTemp = (331 - 125) / (float)AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
    usPosX = (uint16_t)(331 - usTemp * ubCount);

    usTemp = (490 - 405) / (float)AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
    usPosRightX = (uint16_t)(405 + usTemp * ubCount);

    usTemp = (AIM_MEMBER_VIDEO_TITLE_START_Y - 96) / (float)AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
    usPosY = (uint16_t)(AIM_MEMBER_VIDEO_TITLE_START_Y - usTemp * ubCount);

    usPosBottomY = AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT;
  }

  SrcRect.iLeft = 0;
  SrcRect.iTop = 0;
  SrcRect.iRight = AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH;
  SrcRect.iBottom = AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT;

  DestRect.iLeft = usPosX;
  DestRect.iTop = usPosY;
  DestRect.iRight = usPosRightX;
  DestRect.iBottom = DestRect.iTop + usPosBottomY;

  if (fForward) {
    // Restore the old rect
    if (ubCount > 2) {
      usWidth = (uint16_t)(LastRect.iRight - LastRect.iLeft);
      usHeight = (uint16_t)(LastRect.iBottom - LastRect.iTop);
      BlitBufferToBuffer(vsSaveBuffer, vsFB, (uint16_t)LastRect.iLeft, (uint16_t)LastRect.iTop,
                         usWidth, usHeight);
    }

    // Save rectangle
    if (ubCount > 1) {
      usWidth = (uint16_t)(DestRect.iRight - DestRect.iLeft);
      usHeight = (uint16_t)(DestRect.iBottom - DestRect.iTop);
      BlitBufferToBuffer(vsFB, vsSaveBuffer, (uint16_t)DestRect.iLeft, (uint16_t)DestRect.iTop,
                         usWidth, usHeight);
    }
  } else {
    // Restore the old rect
    if (ubCount < AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 2) {
      usWidth = (uint16_t)(LastRect.iRight - LastRect.iLeft);
      usHeight = (uint16_t)(LastRect.iBottom - LastRect.iTop);
      BlitBufferToBuffer(vsSaveBuffer, vsFB, (uint16_t)LastRect.iLeft, (uint16_t)LastRect.iTop,
                         usWidth, usHeight);
    }

    // Save rectangle
    if (ubCount < AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 1) {
      usWidth = (uint16_t)(DestRect.iRight - DestRect.iLeft);
      usHeight = (uint16_t)(DestRect.iBottom - DestRect.iTop);
      BlitBufferToBuffer(vsFB, vsSaveBuffer, (uint16_t)DestRect.iLeft, (uint16_t)DestRect.iTop,
                         usWidth, usHeight);
    }
  }

  BltStretchVSurface(vsFB, vsVideoTitleBar, &SrcRect, &DestRect);

  InvalidateRegion(DestRect.iLeft, DestRect.iTop, DestRect.iRight, DestRect.iBottom);
  InvalidateRegion(LastRect.iLeft, LastRect.iTop, LastRect.iRight, LastRect.iBottom);

  LastRect = DestRect;

  if (fForward) {
    ubCount++;
    if (ubCount == AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 1)
      return (TRUE);
    else
      return (FALSE);
  } else {
    ubCount--;
    if (ubCount == 0)
      return (TRUE);
    else
      return (FALSE);
  }
}

#ifdef JA2TESTVERSION
// TEMP:
void TempHiringOfMercs(uint8_t ubNumberOfMercs, BOOLEAN fReset) {
  int16_t i;
  uint8_t MercID[] = {11, 16, 29, 36, 2, 10, 17, 6, 7, 12, 0, 1, 3, 4, 5, 8, 9, 13, 14, 15, 18, 19};
  MERC_HIRE_STRUCT HireMercStruct;
  static BOOLEAN fHaveCalledBefore = FALSE;

  // if we should reset the global variable
  if (fReset) {
    fHaveCalledBefore = FALSE;
    return;
  }

  if (fHaveCalledBefore) return;

  if (guiCurrentLaptopMode != LAPTOP_MODE_NONE) return;

  fHaveCalledBefore = TRUE;

  for (i = 0; i < ubNumberOfMercs; i++) {
    memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

    if (!IsMercHireable(MercID[i])) {
      ubNumberOfMercs++;
      continue;
    }

    HireMercStruct.ubProfileID = MercID[i];

    // DEF: temp
    HireMercStruct.sSectorX = gsMercArriveSectorX;
    HireMercStruct.sSectorY = gsMercArriveSectorY;
    HireMercStruct.fUseLandingZoneForArrival = TRUE;
    HireMercStruct.ubInsertionCode = INSERTION_CODE_ARRIVING_GAME;

    HireMercStruct.fCopyProfileItemsOver = TRUE;
    gMercProfiles[MercID[i]].ubMiscFlags |= PROFILE_MISC_FLAG_ALREADY_USED_ITEMS;

    if (gfKeyState[ALT])
      HireMercStruct.iTotalContractLength = 14;
    else if (gfKeyState[CTRL])
      HireMercStruct.iTotalContractLength = 7;
    else
      HireMercStruct.iTotalContractLength = 1;

    // specify when the merc should arrive
    HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay();  // + MercID[i];

    // since this is only a testing function, make the merc available
    gMercProfiles[MercID[i]].bMercStatus = 0;

    // if we succesfully hired the merc
    HireMerc(&HireMercStruct);

    // add an entry in the finacial page for the hiring of the merc
    AddTransactionToPlayersBook(HIRED_MERC, MercID[i],
                                -(int32_t)(gMercProfiles[MercID[i]].sSalary));

    if (gMercProfiles[MercID[i]].bMedicalDeposit) {
      // add an entry in the finacial page for the medical deposit
      AddTransactionToPlayersBook(MEDICAL_DEPOSIT, MercID[i],
                                  -(gMercProfiles[MercID[i]].sMedicalDepositAmount));
    }

    // add an entry in the history page for the hiring of the merc
    AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_AIM, MercID[i], GetWorldTotalMin(), -1, -1);
  }
}

#endif

void DelayMercSpeech(uint8_t ubMercID, uint16_t usQuoteNum, uint16_t usDelay, BOOLEAN fNewQuote,
                     BOOLEAN fReset) {
  static uint32_t uiLastTime = 0;
  uint32_t uiCurTime;
  static uint16_t usCurQuoteNum;
  static uint16_t usCurDelay;
  static BOOLEAN fQuoteWaiting = FALSE;  // a quote is waiting to be said
  static uint8_t ubCurMercID;
  static BOOLEAN fHangUpAfter = FALSE;

  uiCurTime = GetJA2Clock();

  if (fReset) fQuoteWaiting = FALSE;

  if (fNewQuote) {
    // set up the counters
    uiLastTime = uiCurTime;

    ubCurMercID = ubMercID;
    usCurQuoteNum = usQuoteNum;
    usCurDelay = usDelay;

    if (gfHangUpMerc) {
      gfHangUpMerc = FALSE;
      fHangUpAfter = TRUE;
    }

    fQuoteWaiting = TRUE;
  }

  if (fQuoteWaiting) {
    if ((uiCurTime - uiLastTime) > usCurDelay) {
      InitVideoFaceTalking(ubCurMercID, usCurQuoteNum);
      fQuoteWaiting = FALSE;

      if (fHangUpAfter) {
        gfHangUpMerc = TRUE;
        fHangUpAfter = FALSE;
      }
    }
  }
}

#ifdef JA2TESTVERSION

// TEMP!!!
BOOLEAN QuickHireMerc() {
  int8_t bReturnCode;
  MERC_HIRE_STRUCT HireMercStruct;
  uint8_t ubCurrentSoldier = AimMercArray[gbCurrentIndex];

  giContractAmount = 0;

  //	if( !IsMercHireable( ubCurrentSoldier ) )
  //		return( FALSE );
  if (FindSoldierByProfileID(ubCurrentSoldier, TRUE) != NULL) return (FALSE);

  HireMercStruct.ubProfileID = ubCurrentSoldier;

  // DEF: temp
  HireMercStruct.sSectorX = gsMercArriveSectorX;
  HireMercStruct.sSectorY = gsMercArriveSectorY;
  HireMercStruct.bSectorZ = 0;
  HireMercStruct.fUseLandingZoneForArrival = TRUE;
  HireMercStruct.ubInsertionCode = INSERTION_CODE_ARRIVING_GAME;

  HireMercStruct.fCopyProfileItemsOver = TRUE;
  gMercProfiles[ubCurrentSoldier].ubMiscFlags |= PROFILE_MISC_FLAG_ALREADY_USED_ITEMS;

  if (gfKeyState[ALT])
    HireMercStruct.iTotalContractLength = 14;
  else if (gfKeyState[CTRL])
    HireMercStruct.iTotalContractLength = 7;
  else
    HireMercStruct.iTotalContractLength = 1;

  // specify when the merc should arrive
  HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay();  // + ubCurrentSoldier;

  SetFlagToForceHireMerc(TRUE);
  bReturnCode = HireMerc(&HireMercStruct);
  SetFlagToForceHireMerc(FALSE);
  if (bReturnCode == MERC_HIRE_OVER_20_MERCS_HIRED) {
    // display a warning saying u cant hire more then 20 mercs
    DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, AimPopUpText[AIM_MEMBER_ALREADY_HAVE_20_MERCS],
                       LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
    return (FALSE);
  } else if (bReturnCode == MERC_HIRE_FAILED) {
    return (FALSE);
  }

  // add an entry in the finacial page for the hiring of the merc
  giContractAmount = gMercProfiles[gbCurrentSoldier].sSalary;

  AddTransactionToPlayersBook(HIRED_MERC, ubCurrentSoldier, -(giContractAmount));

  if (gMercProfiles[gbCurrentSoldier].bMedicalDeposit) {
    // add an entry in the finacial page for the medical deposit
    AddTransactionToPlayersBook(MEDICAL_DEPOSIT, ubCurrentSoldier,
                                -(gMercProfiles[gbCurrentSoldier].sMedicalDepositAmount));
  }

  // add an entry in the history page for the hiring of the merc
  AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_AIM, ubCurrentSoldier, GetWorldTotalMin(), -1, -1);

  gfRedrawScreen = TRUE;

  return (TRUE);
}

void TempHandleAimMemberKeyBoardInput() {
  InputAtom InputEvent;

  while (DequeueEvent(&InputEvent) == TRUE) {  //! HandleTextInput( &InputEvent ) &&
    if (InputEvent.usEvent == KEY_DOWN) {
      switch (InputEvent.usParam) {
#ifdef JA2TESTVERSION
        case SPACE:
          QuickHireMerc();
          break;

        case '~':
          // to test going on other assignments, unhired merc improvements & deaths
          if (guiDay == 1) guiDay++;
          MercDailyUpdate();
          gfRedrawScreen = TRUE;
          break;
#endif

        default:
          HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam,
                                           InputEvent.usKeyState);
          break;
      }
    }
  }
}

#endif

void WaitForMercToFinishTalkingOrUserToClick() {
  // if the region is not active
  if (!gfIsShutUpMouseRegionActive) {
    // Enables it so if a player clicks, he will shutup the merc
    MSYS_EnableRegion(&gSelectedShutUpMercRegion);
    gfIsShutUpMouseRegionActive = TRUE;
  }

  if (gfIsAnsweringMachineActive)
    gubVideoConferencingMode = AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE;
  else
    gubVideoConferencingMode = AIM_VIDEO_FIRST_CONTACT_MERC_MODE;

  gfWaitingForMercToStopTalkingOrUserToClick = TRUE;
  gfHangUpMerc = TRUE;
  gfStopMercFromTalking = FALSE;
}

/*
BOOLEAN DisplayShadedStretchedMercFace( uint8_t ubMercID, uint16_t usPosX, uint16_t usPosY )
{
        SGPRect		SrcRect;
        SGPRect		DestRect;


        //Test
        SrcRect.iLeft = 0;
        SrcRect.iTop = 0;
        SrcRect.iRight = 48;
        SrcRect.iBottom = 43;

        DestRect.iLeft = AIM_MEMBER_VIDEO_FACE_X;
        DestRect.iTop = AIM_MEMBER_VIDEO_FACE_Y;
        DestRect.iRight = DestRect.iLeft + AIM_MEMBER_VIDEO_FACE_WIDTH;
        DestRect.iBottom = DestRect.iTop + AIM_MEMBER_VIDEO_FACE_HEIGHT;


        if(	!BltStretchVideoSurface(vsFB, guiVideoFaceBackground, 0, 0,
VO_BLT_SRCTRANSPARENCY, &SrcRect, &DestRect ) ) return(FALSE);


        return( TRUE );
}
*/

void DisplayPopUpBoxExplainingMercArrivalLocationAndTime() {
  wchar_t szLocAndTime[512];
  struct SOLDIERTYPE *pSoldier = NULL;
  wchar_t zTimeString[128];
  wchar_t zSectorIDString[512];
  uint32_t uiHour;

  // if the id of the merc is invalid, dont display the pop up
  if (LaptopSaveInfo.sLastHiredMerc.iIdOfMerc == -1) return;

  // if the pop up has already been displayed, dont display it again for this occurence of laptop
  if (LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop) return;

  pSoldier = FindSoldierByProfileID((uint8_t)LaptopSaveInfo.sLastHiredMerc.iIdOfMerc, TRUE);

  if (pSoldier == NULL) return;

  // calc the approximate hour the mercs will arrive at
  uiHour = ((LaptopSaveInfo.sLastHiredMerc.uiArrivalTime) -
            (((LaptopSaveInfo.sLastHiredMerc.uiArrivalTime) / 1440) * 1440)) /
           60;

  // create the time string
  swprintf(zTimeString, ARR_SIZE(zTimeString), L"%02d:%02d", uiHour, 0);

  // get the id string
  GetSectorIDString(gsMercArriveSectorX, gsMercArriveSectorY, 0, zSectorIDString,
                    ARR_SIZE(zSectorIDString), FALSE);

  // create the string to display to the user, looks like....
  //	L"%s should arrive at the designated drop-off point ( sector %d:%d %s ) on day %d, at
  // approximately %s.",		//first %s is mercs name, next is the sector location and
  // name where they will be arriving in, lastely is the day an the time of arrival

  if (UsingGermanResources()) {
    // Germans version has a different argument order
    swprintf(szLocAndTime, ARR_SIZE(szLocAndTime),
             pMessageStrings[MSG_JUST_HIRED_MERC_ARRIVAL_LOCATION_POPUP],
             gMercProfiles[GetSolProfile(pSoldier)].zNickname,
             LaptopSaveInfo.sLastHiredMerc.uiArrivalTime / 1440, zTimeString, zSectorIDString);
  } else {
    swprintf(szLocAndTime, ARR_SIZE(szLocAndTime),
             pMessageStrings[MSG_JUST_HIRED_MERC_ARRIVAL_LOCATION_POPUP],
             gMercProfiles[GetSolProfile(pSoldier)].zNickname, zSectorIDString,
             LaptopSaveInfo.sLastHiredMerc.uiArrivalTime / 1440, zTimeString);
  }

  // display the message box
  DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, szLocAndTime, LAPTOP_SCREEN, MSG_BOX_FLAG_OK,
                     DisplayPopUpBoxExplainingMercArrivalLocationAndTimeCallBack);

  // reset the id of the last merc
  LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;

  // set the fact that the pop up has been displayed this time in laptop
  LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop = TRUE;
}

void DisplayPopUpBoxExplainingMercArrivalLocationAndTimeCallBack(uint8_t bExitValue) {
  // unset the flag so the msgbox WONT dislay its save buffer
  gfDontOverRideSaveBuffer = FALSE;

  if (guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS) {
    // render the screen
    gfRedrawScreen = TRUE;
    RenderAIMMembers();
  }
}

void DisplayAimMemberClickOnFaceHelpText() {
  // display the 'left and right click' onscreen help msg
  DrawTextToScreen(AimMemberText[0], AIM_FI_LEFT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y,
                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_TITLE_FONT, AIM_FONT_MCOLOR_WHITE,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DrawTextToScreen(AimMemberText[1], AIM_FI_LEFT_CLICK_TEXT_X,
                   AIM_FI_LEFT_CLICK_TEXT_Y + AIM_FI_CLICK_DESC_TEXT_Y_OFFSET,
                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_FONT, AIM_FONT_MCOLOR_WHITE,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  DrawTextToScreen(AimMemberText[2], AIM_FI_RIGHT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y,
                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_TITLE_FONT, AIM_FONT_MCOLOR_WHITE,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
  DrawTextToScreen(AimMemberText[3], AIM_FI_RIGHT_CLICK_TEXT_X,
                   AIM_FI_LEFT_CLICK_TEXT_Y + AIM_FI_CLICK_DESC_TEXT_Y_OFFSET,
                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_FONT, AIM_FONT_MCOLOR_WHITE,
                   FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
}
