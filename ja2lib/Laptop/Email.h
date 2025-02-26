// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __EMAIL_H
#define __EMAIL_H

#include "SGP/Types.h"

// defines
#define MAX_EMAIL_LINES 10    // max number of lines can be shown in a message
#define MAX_MESSAGES_PAGE 18  // max number of messages per page

#define IMP_EMAIL_INTRO 0
#define IMP_EMAIL_INTRO_LENGTH 10
#define ENRICO_CONGRATS (IMP_EMAIL_INTRO + IMP_EMAIL_INTRO_LENGTH)
#define ENRICO_CONGRATS_LENGTH 3
#define IMP_EMAIL_AGAIN (ENRICO_CONGRATS + ENRICO_CONGRATS_LENGTH)
#define IMP_EMAIL_AGAIN_LENGTH 6
#define MERC_INTRO (IMP_EMAIL_AGAIN + IMP_EMAIL_AGAIN_LENGTH)
#define MERC_INTRO_LENGTH 5
#define MERC_NEW_SITE_ADDRESS (MERC_INTRO + MERC_INTRO_LENGTH)
#define MERC_NEW_SITE_ADDRESS_LENGTH 2
#define AIM_MEDICAL_DEPOSIT_REFUND (MERC_NEW_SITE_ADDRESS + MERC_NEW_SITE_ADDRESS_LENGTH)
#define AIM_MEDICAL_DEPOSIT_REFUND_LENGTH 3
#define IMP_EMAIL_PROFILE_RESULTS (AIM_MEDICAL_DEPOSIT_REFUND + AIM_MEDICAL_DEPOSIT_REFUND_LENGTH)
#define IMP_EMAIL_PROFILE_RESULTS_LENGTH 1
#define MERC_WARNING (IMP_EMAIL_PROFILE_RESULTS_LENGTH + IMP_EMAIL_PROFILE_RESULTS)
#define MERC_WARNING_LENGTH 2
#define MERC_INVALID (MERC_WARNING + MERC_WARNING_LENGTH)
#define MERC_INVALID_LENGTH 2
#define NEW_MERCS_AT_MERC (MERC_INVALID + MERC_INVALID_LENGTH)
#define NEW_MERCS_AT_MERC_LENGTH 2
#define MERC_FIRST_WARNING (NEW_MERCS_AT_MERC + NEW_MERCS_AT_MERC_LENGTH)
#define MERC_FIRST_WARNING_LENGTH 2
// merc up a level emails
#define MERC_UP_LEVEL_BIFF (MERC_FIRST_WARNING + MERC_FIRST_WARNING_LENGTH)
#define MERC_UP_LEVEL_LENGTH_BIFF 2
#define MERC_UP_LEVEL_HAYWIRE (MERC_UP_LEVEL_LENGTH_BIFF + MERC_UP_LEVEL_BIFF)
#define MERC_UP_LEVEL_LENGTH_HAYWIRE 2
#define MERC_UP_LEVEL_GASKET (MERC_UP_LEVEL_LENGTH_HAYWIRE + MERC_UP_LEVEL_HAYWIRE)
#define MERC_UP_LEVEL_LENGTH_GASKET 2
#define MERC_UP_LEVEL_RAZOR (MERC_UP_LEVEL_LENGTH_GASKET + MERC_UP_LEVEL_GASKET)
#define MERC_UP_LEVEL_LENGTH_RAZOR 2
#define MERC_UP_LEVEL_FLO (MERC_UP_LEVEL_LENGTH_RAZOR + MERC_UP_LEVEL_RAZOR)
#define MERC_UP_LEVEL_LENGTH_FLO 2
#define MERC_UP_LEVEL_GUMPY (MERC_UP_LEVEL_LENGTH_FLO + MERC_UP_LEVEL_FLO)
#define MERC_UP_LEVEL_LENGTH_GUMPY 2
#define MERC_UP_LEVEL_LARRY (MERC_UP_LEVEL_LENGTH_GUMPY + MERC_UP_LEVEL_GUMPY)
#define MERC_UP_LEVEL_LENGTH_LARRY 2
#define MERC_UP_LEVEL_COUGAR (MERC_UP_LEVEL_LENGTH_LARRY + MERC_UP_LEVEL_LARRY)
#define MERC_UP_LEVEL_LENGTH_COUGAR 2
#define MERC_UP_LEVEL_NUMB (MERC_UP_LEVEL_LENGTH_COUGAR + MERC_UP_LEVEL_COUGAR)
#define MERC_UP_LEVEL_LENGTH_NUMB 2
#define MERC_UP_LEVEL_BUBBA (MERC_UP_LEVEL_LENGTH_NUMB + MERC_UP_LEVEL_NUMB)
#define MERC_UP_LEVEL_LENGTH_BUBBA 2
// merc left-me-a-message-and-now-I'm-back emails
#define AIM_REPLY_BARRY (MERC_UP_LEVEL_LENGTH_BUBBA + MERC_UP_LEVEL_BUBBA)
#define AIM_REPLY_LENGTH_BARRY 2
#define AIM_REPLY_MELTDOWN (AIM_REPLY_BARRY + (39 * AIM_REPLY_LENGTH_BARRY))
#define AIM_REPLY_LENGTH_MELTDOWN AIM_REPLY_LENGTH_BARRY

// old EXISTING emails when player starts game. They must look "read"
#define OLD_ENRICO_1 (AIM_REPLY_LENGTH_MELTDOWN + AIM_REPLY_MELTDOWN)
#define OLD_ENRICO_1_LENGTH 3
#define OLD_ENRICO_2 (OLD_ENRICO_1 + OLD_ENRICO_1_LENGTH)
#define OLD_ENRICO_2_LENGTH 3
#define RIS_REPORT (OLD_ENRICO_2 + OLD_ENRICO_2_LENGTH)
#define RIS_REPORT_LENGTH 2
#define OLD_ENRICO_3 (RIS_REPORT + RIS_REPORT_LENGTH)
#define OLD_ENRICO_3_LENGTH 3

// emails that occur from Enrico once player accomplishes things
#define ENRICO_MIGUEL (OLD_ENRICO_3 + OLD_ENRICO_3_LENGTH)
#define ENRICO_MIGUEL_LENGTH 3
#define ENRICO_PROG_20 (ENRICO_MIGUEL + ENRICO_MIGUEL_LENGTH)
#define ENRICO_PROG_20_LENGTH 3
#define ENRICO_PROG_55 (ENRICO_PROG_20 + ENRICO_PROG_20_LENGTH)
#define ENRICO_PROG_55_LENGTH 3
#define ENRICO_PROG_80 (ENRICO_PROG_55 + ENRICO_PROG_55_LENGTH)
#define ENRICO_PROG_80_LENGTH 3
#define ENRICO_SETBACK (ENRICO_PROG_80 + ENRICO_PROG_80_LENGTH)
#define ENRICO_SETBACK_LENGTH 3
#define ENRICO_SETBACK_2 (ENRICO_SETBACK + ENRICO_SETBACK_LENGTH)
#define ENRICO_SETBACK_2_LENGTH 3
#define ENRICO_CREATURES (ENRICO_SETBACK_2 + ENRICO_SETBACK_2_LENGTH)
#define ENRICO_CREATURES_LENGTH 3

// insurance company emails
#define INSUR_PAYMENT (ENRICO_CREATURES + ENRICO_CREATURES_LENGTH)
#define INSUR_PAYMENT_LENGTH 3
#define INSUR_SUSPIC (INSUR_PAYMENT + INSUR_PAYMENT_LENGTH)
#define INSUR_SUSPIC_LENGTH 3
#define INSUR_INVEST_OVER (INSUR_SUSPIC + INSUR_SUSPIC_LENGTH)
#define INSUR_INVEST_OVER_LENGTH 3
#define INSUR_SUSPIC_2 (INSUR_INVEST_OVER + INSUR_INVEST_OVER_LENGTH)
#define INSUR_SUSPIC_2_LENGTH 3

#define BOBBYR_NOW_OPEN (INSUR_SUSPIC_2 + INSUR_SUSPIC_2_LENGTH)
#define BOBBYR_NOW_OPEN_LENGTH 3

#define KING_PIN_LETTER (BOBBYR_NOW_OPEN + BOBBYR_NOW_OPEN_LENGTH)
#define KING_PIN_LETTER_LENGTH 4

#define LACK_PLAYER_PROGRESS_1 (KING_PIN_LETTER + KING_PIN_LETTER_LENGTH)
#define LACK_PLAYER_PROGRESS_1_LENGTH 3

#define LACK_PLAYER_PROGRESS_2 (LACK_PLAYER_PROGRESS_1 + LACK_PLAYER_PROGRESS_1_LENGTH)
#define LACK_PLAYER_PROGRESS_2_LENGTH 3

#define LACK_PLAYER_PROGRESS_3 (LACK_PLAYER_PROGRESS_2 + LACK_PLAYER_PROGRESS_2_LENGTH)
#define LACK_PLAYER_PROGRESS_3_LENGTH 3

// A package from bobby r has arrived in Drassen
#define BOBBYR_SHIPMENT_ARRIVED (LACK_PLAYER_PROGRESS_3 + LACK_PLAYER_PROGRESS_3_LENGTH)
#define BOBBYR_SHIPMENT_ARRIVED_LENGTH 4

// John Kulba has left the gifts for theplayers in drassen
#define JOHN_KULBA_GIFT_IN_DRASSEN (BOBBYR_SHIPMENT_ARRIVED + BOBBYR_SHIPMENT_ARRIVED_LENGTH)
#define JOHN_KULBA_GIFT_IN_DRASSEN_LENGTH 4

// when a merc dies on ANOTHER assignment ( ie not with the player )
#define MERC_DIED_ON_OTHER_ASSIGNMENT \
  (JOHN_KULBA_GIFT_IN_DRASSEN + JOHN_KULBA_GIFT_IN_DRASSEN_LENGTH)
#define MERC_DIED_ON_OTHER_ASSIGNMENT_LENGTH 5

#define INSUR_1HOUR_FRAUD (MERC_DIED_ON_OTHER_ASSIGNMENT + MERC_DIED_ON_OTHER_ASSIGNMENT_LENGTH)
#define INSUR_1HOUR_FRAUD_LENGTH 3

// when a merc is fired, and is injured
#define AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND (INSUR_1HOUR_FRAUD + INSUR_1HOUR_FRAUD_LENGTH)
#define AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND_LENGTH 3

// when a merc is fired, and is dead
#define AIM_MEDICAL_DEPOSIT_NO_REFUND \
  (AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND + AIM_MEDICAL_DEPOSIT_PARTIAL_REFUND_LENGTH)
#define AIM_MEDICAL_DEPOSIT_NO_REFUND_LENGTH 3

#define BOBBY_R_MEDUNA_SHIPMENT \
  (AIM_MEDICAL_DEPOSIT_NO_REFUND + AIM_MEDICAL_DEPOSIT_NO_REFUND_LENGTH)
#define BOBBY_R_MEDUNA_SHIPMENT_LENGTH 4

struct message {
  wchar_t *pString;
  struct message *Next;
  struct message *Prev;
};

typedef struct message EmailMessage;
typedef EmailMessage *MessagePtr;

struct email {
  wchar_t *pSubject;
  uint16_t usOffset;
  uint16_t usLength;
  uint8_t ubSender;
  uint32_t iDate;
  int32_t iId;
  int32_t iFirstData;
  uint32_t uiSecondData;
  BOOLEAN fRead;
  BOOLEAN fNew;

  int32_t iThirdData;
  int32_t iFourthData;
  uint32_t uiFifthData;
  uint32_t uiSixData;

  struct email *Next;
  struct email *Prev;
};

typedef struct email Email;
typedef Email *EmailPtr;

// This used when saving the emails to disk.
typedef struct {
  uint16_t usOffset;
  uint16_t usLength;
  uint8_t ubSender;
  uint32_t iDate;
  int32_t iId;
  int32_t iFirstData;
  uint32_t uiSecondData;

  int32_t iThirdData;
  int32_t iFourthData;
  uint32_t uiFifthData;
  uint32_t uiSixData;

  BOOLEAN fRead;
  BOOLEAN fNew;
} SavedEmailStruct;

struct pagemessages {
  int32_t iIds[MAX_MESSAGES_PAGE];
  int32_t iPageId;
  struct pagemessages *Next;
  struct pagemessages *Prev;
};

typedef struct pagemessages Page;
typedef Page *PagePtr;

struct messagerecord {
  //  wchar_t pRecord[ 320 ];
  wchar_t pRecord[640];
  struct messagerecord *Next;
};

typedef struct messagerecord Record;
typedef Record *RecordPtr;

typedef struct {
  RecordPtr pFirstRecord;
  RecordPtr pLastRecord;
  int32_t iPageNumber;
} EmailPageInfoStruct;

enum {
  SENDER = 0,
  RECEIVED,
  SUBJECT,
  READ,
};

enum {
  MAIL_ENRICO = 0,
  CHAR_PROFILE_SITE,
  GAME_HELP,
  IMP_PROFILE_RESULTS,
  SPECK_FROM_MERC,
  RIS_EMAIL,
  BARRY_MAIL,
  MELTDOWN_MAIL = BARRY_MAIL + 39,
  INSURANCE_COMPANY,
  BOBBY_R,
  KING_PIN,
  JOHN_KULBA,
  AIM_SITE,
};

// the length of the subject in char
#define EMAIL_SUBJECT_LENGTH 128

extern BOOLEAN fUnReadMailFlag;
extern BOOLEAN fNewMailFlag;
extern BOOLEAN fOldUnreadFlag;
extern BOOLEAN fOldNewMailFlag;
extern BOOLEAN fDeleteMailFlag;
extern BOOLEAN fDisplayMessageFlag;
extern BOOLEAN fReDrawNewMailFlag;
extern BOOLEAN fOpenMostRecentUnReadFlag;
extern EmailPtr pEmailList;
extern uint32_t guiEmailWarning;

void GameInitEmail();
BOOLEAN EnterEmail();
void ExitEmail();
void HandleEmail();
void RenderEmail();

#define CHECK_X 15
#define CHECK_Y 13
#define VIEWER_X 155
#define VIEWER_Y 70 + 21
#define MAIL_STRING_SIZE 640

// message manipulation
void AddEmailMessage(int32_t iMessageOffset, int32_t iMessageLength, wchar_t *pSubject,
                     int32_t iDate, uint8_t ubSender, BOOLEAN fAlreadyRead, int32_t uiFirstData,
                     uint32_t uiSecondData);
void RemoveEmailMessage(int32_t iId);
EmailPtr GetEmailMessage(int32_t iId);
void LookForUnread();
void AddEmail(int32_t iMessageOffset, int32_t iMessageLength, uint8_t ubSender, int32_t iDate);
void AddPreReadEmail(int32_t iMessageOffset, int32_t iMessageLength, uint8_t ubSender,
                     int32_t iDate);
BOOLEAN DisplayNewMailBox();
void CreateDestroyNewMailButton();
void CreateDestroyDeleteNoticeMailButton();
void AddDeleteRegionsToMessageRegion(int32_t iViewerY);
void DisplayEmailHeaders(void);
void ReDrawNewMailBox(void);
void ReDisplayBoxes(void);
void ShutDownEmailList();
void AddMessageToPages(int32_t iMessageId);
void AddEmailWithSpecialData(int32_t iMessageOffset, int32_t iMessageLength, uint8_t ubSender,
                             int32_t iDate, int32_t iFirstData, uint32_t uiSecondData);

#ifdef JA2BETAVERSION
void AddAllEmails();
#endif

#endif
