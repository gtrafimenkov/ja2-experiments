// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/MusicControl.h"

#include "FadeScreen.h"
#include "GameScreen.h"
#include "JAScreens.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "ScreenIDs.h"
#include "Soldier.h"
#include "Strategic/CreatureSpreading.h"
#include "Strategic/StrategicMap.h"
#include "Tactical/Overhead.h"
#include "Tactical/SoldierControl.h"
#include "UI.h"
#include "Utils/TimerControl.h"

uint32_t uiMusicHandle = NO_SAMPLE;
uint32_t uiMusicVolume = 50;
BOOLEAN fMusicPlaying = FALSE;
BOOLEAN fMusicFadingOut = FALSE;
BOOLEAN fMusicFadingIn = FALSE;

BOOLEAN gfMusicEnded = FALSE;

uint8_t gubMusicMode = 0;
uint8_t gubOldMusicMode = 0;

int8_t gbVictorySongCount = 0;
int8_t gbDeathSongCount = 0;

int8_t bNothingModeSong;
int8_t bEnemyModeSong;
int8_t bBattleModeSong;

int8_t gbFadeSpeed = 1;

char *szMusicList[NUM_MUSIC] = {
    "MUSIC\\marimbad 2.wav", "MUSIC\\menumix1.wav",  "MUSIC\\nothing A.wav",
    "MUSIC\\nothing B.wav",  "MUSIC\\nothing C.wav", "MUSIC\\nothing D.wav",
    "MUSIC\\tensor A.wav",   "MUSIC\\tensor B.wav",  "MUSIC\\tensor C.wav",
    "MUSIC\\triumph.wav",    "MUSIC\\death.wav",     "MUSIC\\battle A.wav",
    "MUSIC\\tensor B.wav",   "MUSIC\\creepy.wav",    "MUSIC\\creature battle.wav",
};

BOOLEAN gfForceMusicToTense = FALSE;
BOOLEAN gfDontRestartSong = FALSE;

BOOLEAN StartMusicBasedOnMode();
void DoneFadeOutDueToEndMusic(void);
extern void HandleEndDemoInCreatureLevel();

BOOLEAN NoEnemiesInSight() {
  struct SOLDIERTYPE *pSoldier;
  int32_t cnt;

  // Loop through our guys
  // End the turn of player charactors
  cnt = gTacticalStatus.Team[gbPlayerNum].bFirstID;

  // look for all mercs on the same team,
  for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[gbPlayerNum].bLastID;
       cnt++, pSoldier++) {
    if (IsSolActive(pSoldier) && pSoldier->bLife >= OKLIFE) {
      if (pSoldier->bOppCnt != 0) {
        return (FALSE);
      }
    }
  }

  return (TRUE);
}

void MusicStopCallback(void *pData);

//********************************************************************************
// MusicPlay
//
//		Starts up one of the tunes in the music list.
//
//	Returns:	TRUE if the music was started, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicPlay(uint32_t uiNum) {
  SOUNDPARMS spParms;

  if (fMusicPlaying) MusicStop();

  memset(&spParms, 0xff, sizeof(SOUNDPARMS));
  spParms.uiPriority = PRIORITY_MAX;
  spParms.uiVolume = 0;

  spParms.EOSCallback = MusicStopCallback;

  uiMusicHandle = SoundPlayStreamedFile(szMusicList[uiNum], &spParms);

  if (uiMusicHandle != SOUND_ERROR) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music PLay %d %d", uiMusicHandle, gubMusicMode));

    gfMusicEnded = FALSE;
    fMusicPlaying = TRUE;
    MusicFadeIn();
    return (TRUE);
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music PLay %d %d", uiMusicHandle, gubMusicMode));

  return (FALSE);
}

//********************************************************************************
// MusicSetVolume
//
//		Sets the volume on the currently playing music.
//
//	Returns:	TRUE if the volume was set, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicSetVolume(uint32_t uiVolume) {
  int32_t uiOldMusicVolume = uiMusicVolume;

  uiMusicVolume = min(uiVolume, 127);

  if (uiMusicHandle != NO_SAMPLE) {
    // get volume and if 0 stop music!
    if (uiMusicVolume == 0) {
      gfDontRestartSong = TRUE;
      MusicStop();
      return (TRUE);
    }

    SoundSetVolume(uiMusicHandle, uiMusicVolume);

    return (TRUE);
  }

  // If here, check if we need to re-start music
  // Have we re-started?
  if (uiMusicVolume > 0 && uiOldMusicVolume == 0) {
    StartMusicBasedOnMode();
  }

  return (FALSE);
}

//********************************************************************************
// MusicGetVolume
//
//		Gets the volume on the currently playing music.
//
//	Returns:	TRUE if the volume was set, FALSE if an error occurred
//
//********************************************************************************
uint32_t MusicGetVolume(void) { return (uiMusicVolume); }

//********************************************************************************
// MusicStop
//
//		Stops the currently playing music.
//
//	Returns:	TRUE if the music was stopped, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicStop(void) {
  if (uiMusicHandle != NO_SAMPLE) {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music Stop %d %d", uiMusicHandle, gubMusicMode));

    SoundStop(uiMusicHandle);
    fMusicPlaying = FALSE;
    uiMusicHandle = NO_SAMPLE;
    return (TRUE);
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music Stop %d %d", uiMusicHandle, gubMusicMode));

  return (FALSE);
}

//********************************************************************************
// MusicFadeOut
//
//		Fades out the current song.
//
//	Returns:	TRUE if the music has begun fading, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicFadeOut(void) {
  if (uiMusicHandle != NO_SAMPLE) {
    fMusicFadingOut = TRUE;
    return (TRUE);
  }
  return (FALSE);
}

//********************************************************************************
// MusicFadeIn
//
//		Fades in the current song.
//
//	Returns:	TRUE if the music has begun fading in, FALSE if an error occurred
//
//********************************************************************************
BOOLEAN MusicFadeIn(void) {
  if (uiMusicHandle != NO_SAMPLE) {
    fMusicFadingIn = TRUE;
    return (TRUE);
  }
  return (FALSE);
}

//********************************************************************************
// MusicPoll
//
//		Handles any maintenance the music system needs done. Should be polled from
//	the main loop, or somewhere with a high frequency of calls.
//
//	Returns:	TRUE always
//
//********************************************************************************
BOOLEAN MusicPoll(BOOLEAN fForce) {
  int32_t iVol;

  SoundServiceStreams();
  SoundServiceRandom();

  // Handle Sound every sound overhead time....
  if (COUNTERDONE(MUSICOVERHEAD)) {
    // Reset counter
    RESETCOUNTER(MUSICOVERHEAD);

    if (fMusicFadingIn) {
      if (uiMusicHandle != NO_SAMPLE) {
        iVol = SoundGetVolume(uiMusicHandle);
        iVol = min((int32_t)uiMusicVolume, iVol + gbFadeSpeed);
        SoundSetVolume(uiMusicHandle, iVol);
        if (iVol == (int32_t)uiMusicVolume) {
          fMusicFadingIn = FALSE;
          gbFadeSpeed = 1;
        }
      }
    } else if (fMusicFadingOut) {
      if (uiMusicHandle != NO_SAMPLE) {
        iVol = SoundGetVolume(uiMusicHandle);
        iVol = (iVol >= 1) ? iVol - gbFadeSpeed : 0;

        iVol = max((int32_t)iVol, 0);

        SoundSetVolume(uiMusicHandle, iVol);
        if (iVol == 0) {
          MusicStop();
          fMusicFadingOut = FALSE;
          gbFadeSpeed = 1;
        }
      }
    }

    // #endif

    if (gfMusicEnded) {
      // OK, based on our music mode, play another!
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music End Loop %d %d", uiMusicHandle, gubMusicMode));

      // If we were in victory mode, change!
      if (gbVictorySongCount == 1 || gbDeathSongCount == 1) {
        if (gbDeathSongCount == 1 && IsTacticalMode()) {
          CheckAndHandleUnloadingOfCurrentWorld();
        }

        if (gbVictorySongCount == 1) {
          SetMusicMode(MUSIC_TACTICAL_NOTHING);
        }
      } else {
        if (!gfDontRestartSong) {
          StartMusicBasedOnMode();
        }
      }

      gfMusicEnded = FALSE;
      gfDontRestartSong = FALSE;
    }
  }

  return (TRUE);
}

BOOLEAN SetMusicMode(uint8_t ubMusicMode) {
  static int8_t bPreviousMode = 0;

  // OK, check if we want to restore
  if (ubMusicMode == MUSIC_RESTORE) {
    if (bPreviousMode == MUSIC_TACTICAL_VICTORY || bPreviousMode == MUSIC_TACTICAL_DEATH) {
      bPreviousMode = MUSIC_TACTICAL_NOTHING;
    }

    ubMusicMode = bPreviousMode;
  } else {
    // Save previous mode...
    bPreviousMode = gubOldMusicMode;
  }

  // if different, start a new music song
  if (gubOldMusicMode != ubMusicMode) {
    // Set mode....
    gubMusicMode = ubMusicMode;

    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music New Mode %d %d", uiMusicHandle, gubMusicMode));

    gbVictorySongCount = 0;
    gbDeathSongCount = 0;

    if (uiMusicHandle != NO_SAMPLE) {
      // Fade out old music
      MusicFadeOut();
    } else {
      // Change music!
      StartMusicBasedOnMode();
    }
  }
  gubOldMusicMode = gubMusicMode;

  return (TRUE);
}

BOOLEAN StartMusicBasedOnMode() {
  static BOOLEAN fFirstTime = TRUE;

  if (fFirstTime) {
    fFirstTime = FALSE;

    bNothingModeSong = NOTHING_A_MUSIC + (int8_t)Random(4);

    bEnemyModeSong = TENSOR_A_MUSIC + (int8_t)Random(3);

    bBattleModeSong = BATTLE_A_MUSIC + (int8_t)Random(2);
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
           String("StartMusicBasedOnMode() %d %d", uiMusicHandle, gubMusicMode));

  // Setup a song based on mode we're in!
  switch (gubMusicMode) {
    case MUSIC_MAIN_MENU:
      // ATE: Don't fade in
      gbFadeSpeed = (int8_t)uiMusicVolume;
      MusicPlay(MENUMIX_MUSIC);
      break;

    case MUSIC_LAPTOP:
      gbFadeSpeed = (int8_t)uiMusicVolume;
      MusicPlay(MARIMBAD2_MUSIC);
      break;

    case MUSIC_TACTICAL_NOTHING:
      // ATE: Don't fade in
      gbFadeSpeed = (int8_t)uiMusicVolume;
      if (gfUseCreatureMusic) {
        MusicPlay(CREEPY_MUSIC);
      } else {
        MusicPlay(bNothingModeSong);
        bNothingModeSong = NOTHING_A_MUSIC + (int8_t)Random(4);
      }
      break;

    case MUSIC_TACTICAL_ENEMYPRESENT:
      // ATE: Don't fade in EnemyPresent...
      gbFadeSpeed = (int8_t)uiMusicVolume;
      if (gfUseCreatureMusic) {
        MusicPlay(CREEPY_MUSIC);
      } else {
        MusicPlay(bEnemyModeSong);
        bEnemyModeSong = TENSOR_A_MUSIC + (int8_t)Random(3);
      }
      break;

    case MUSIC_TACTICAL_BATTLE:
      // ATE: Don't fade in
      gbFadeSpeed = (int8_t)uiMusicVolume;
      if (gfUseCreatureMusic) {
        MusicPlay(CREATURE_BATTLE_MUSIC);
      } else {
        MusicPlay(bBattleModeSong);
      }
      bBattleModeSong = BATTLE_A_MUSIC + (int8_t)Random(2);
      break;

    case MUSIC_TACTICAL_VICTORY:

      // ATE: Don't fade in EnemyPresent...
      gbFadeSpeed = (int8_t)uiMusicVolume;
      MusicPlay(TRIUMPH_MUSIC);
      gbVictorySongCount++;

      if (gfUseCreatureMusic &&
          !gbWorldSectorZ) {  // We just killed all the creatures that just attacked the town.
        gfUseCreatureMusic = FALSE;
      }
      break;

    case MUSIC_TACTICAL_DEATH:

      // ATE: Don't fade in EnemyPresent...
      gbFadeSpeed = (int8_t)uiMusicVolume;
      MusicPlay(DEATH_MUSIC);
      gbDeathSongCount++;
      break;

    default:
      MusicFadeOut();
      break;
  }

  return (TRUE);
}

void MusicStopCallback(void *pData) {
  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Music EndCallback %d %d", uiMusicHandle, gubMusicMode));

  gfMusicEnded = TRUE;
  uiMusicHandle = NO_SAMPLE;
}

void SetMusicFadeSpeed(int8_t bFadeSpeed) { gbFadeSpeed = bFadeSpeed; }

void FadeMusicForXSeconds(uint32_t uiDelay) {
  int16_t sNumTimeSteps, sNumVolumeSteps;

  // get # time steps in delay....
  sNumTimeSteps = (int16_t)(uiDelay / 10);

  // Devide this by music volume...
  sNumVolumeSteps = (int16_t)(uiMusicVolume / sNumTimeSteps);

  // Set fade delay...
  SetMusicFadeSpeed((int8_t)sNumVolumeSteps);
}

void DoneFadeOutDueToEndMusic(void) {
  // Quit game....
  InternalLeaveTacticalScreen(MAINMENU_SCREEN);
  // SetPendingNewScreen( MAINMENU_SCREEN );
}
