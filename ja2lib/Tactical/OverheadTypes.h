// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __OVERHEAD_TYPES_H
#define __OVERHEAD_TYPES_H
// GLOBAL HEADER FOR DATA, TYPES FOR TACTICAL ENGINE

#include "BuildDefines.h"
#include "SGP/Types.h"
#include "Team.h"

#define REFINE_AIM_1 0
#define REFINE_AIM_MID1 1
#define REFINE_AIM_2 2
#define REFINE_AIM_MID2 3
#define REFINE_AIM_3 4
#define REFINE_AIM_MID3 5
#define REFINE_AIM_4 6
#define REFINE_AIM_MID4 7
#define REFINE_AIM_5 8
#define REFINE_AIM_BURST 10

#define AIM_SHOT_RANDOM 0
#define AIM_SHOT_HEAD 1
#define AIM_SHOT_TORSO 2
#define AIM_SHOT_LEGS 3
#define AIM_SHOT_GLAND 4

#define MIN_AMB_LEVEL_FOR_MERC_LIGHTS 9

#define MAXTEAMS 6
#define MAXMERCS MAX_NUM_SOLDIERS

// TACTICAL OVERHEAD STUFF
#define NO_SOLDIER TOTAL_SOLDIERS  // SAME AS NOBODY
#define NOBODY NO_SOLDIER

// TACTICAL ENGINE STATUS FLAGS
#define REALTIME 0x000000002
#define TURNBASED 0x000000004
#define IN_ENDGAME_SEQUENCE 0x000000008
#define SHOW_ALL_ITEMS 0x000000010
#define SHOW_AP_LEFT 0x000000020
#define SHOW_ALL_MERCS 0x000000040
#define TRANSLUCENCY_TYPE 0x000000080
#define GODMODE 0x000000100
#define DEMOMODE 0x000000200
#define PLAYER_TEAM_DEAD 0x000000400
#define NPC_TEAM_DEAD 0x000000800
#define DISALLOW_SIGHT 0x000001000
#define CHECK_SIGHT_AT_END_OF_ATTACK 0x000002000
#define IN_CREATURE_LAIR 0x000004000
#define HIDE_TREES 0x000008000
#define NOHIDE_REDUNDENCY 0x000010000
#define DEBUGCLIFFS 0x000020000
#define INCOMBAT 0x000040000
#define ACTIVE 0x000100000
#define SHOW_Z_BUFFER 0x000200000
#define SLOW_ANIMATION 0x000400000
#define ENGAGED_IN_CONV 0x000800000
#define LOADING_SAVED_GAME 0x001000000
#define OUR_MERCS_AUTO_MOVE 0x002000000
#define SHOW_ALL_ROOFS 0x004000000
#define NEWLY_ENTERED_SECTOR 0x008000000
#define RED_ITEM_GLOW_ON 0x010000000
#define IGNORE_ENGAGED_IN_CONV_UI_UNLOCK 0x020000000
#define IGNORE_ALL_OBSTACLES 0x040000000
#define IN_DEIDRANNA_ENDGAME 0x080000000
#define DONE_DEIDRANNA_ENDGAME 0x100000000

#define OKBREATH 10
#define OKLIFE 15
#define CONSCIOUSNESS 10

// VIEWRANGE DEFINES
#define NORMAL_VIEW_RANGE 13
#define MIN_RANGE_FOR_BLOWNAWAY 40

// MODIFIERS FOR AP COST FOR MOVEMENT
#define RUNDIVISOR 1.8
#define WALKCOST -1
#define SWATCOST 0
#define CRAWLCOST 1

// defines
// ######################################################
#define MAX_PATH_LIST_SIZE 30
#define NUM_SOLDIER_SHADES 48
#define NUM_SOLDIER_EFFECTSHADES 2

// TIMER DELAYS
#define DAMAGE_DISPLAY_DELAY 250
#define FADE_DELAY 150
#define FLASH_SELECTOR_DELAY 4000
#define BLINK_SELECTOR_DELAY 250

#define PTR_OURTEAM (pSoldier->bTeam == gbPlayerNum)

#define DONTLOOK 0
#define LOOK 1

#define NOLOCATE 0
#define LOCATE 1

#define DONTSETLOCATOR 0
#define SETLOCATOR 1
#define SETANDREMOVEPREVIOUSLOCATOR 2
#define SETLOCATORFAST 3

#define NOCENTERING 0

#define NOUPDATE 0
#define UPDATE 1

// ORDERS
enum {
  STATIONARY = 0,  // moves max 1 sq., no matter what's going on
  ONGUARD,         // moves max 2 sqs. until alerted by something
  CLOSEPATROL,     // patrols within 5 spaces until alerted
  FARPATROL,       // patrols within 15 spaces
  POINTPATROL,     // patrols using patrolGrids
  ONCALL,          // helps buddies anywhere within the sector
  SEEKENEMY,       // not tied down to any one particular spot
  RNDPTPATROL,     // patrols randomly using patrolGrids
  MAXORDERS
};

// ATTITUDES
enum {
  DEFENSIVE = 0,
  BRAVESOLO,
  BRAVEAID,
  CUNNINGSOLO,
  CUNNINGAID,
  AGGRESSIVE,
  MAXATTITUDES,
  ATTACKSLAYONLY  // special hyperaggressive vs Slay only value for Carmen the bounty hunter
};

// alert status types
enum {
  STATUS_GREEN = 0,  // everything's OK, no suspicion
  STATUS_YELLOW,     // he or his friend heard something
  STATUS_RED,        // has definite evidence of opponent
  STATUS_BLACK,      // currently sees an active opponent
  NUM_STATUS_STATES
};

enum {
  MORALE_HOPELESS = 0,
  MORALE_WORRIED,
  MORALE_NORMAL,
  MORALE_CONFIDENT,
  MORALE_FEARLESS,
  NUM_MORALE_STATES
};

// DEFINES FOR WEAPON HIT EVENT SPECIAL PARAM
#define FIRE_WEAPON_NO_SPECIAL 0
#define FIRE_WEAPON_BURST_SPECIAL 1
#define FIRE_WEAPON_HEAD_EXPLODE_SPECIAL 2
#define FIRE_WEAPON_CHEST_EXPLODE_SPECIAL 3
#define FIRE_WEAPON_LEG_FALLDOWN_SPECIAL 4
#define FIRE_WEAPON_HIT_BY_KNIFE_SPECIAL 5
#define FIRE_WEAPON_SLEEP_DART_SPECIAL 6
#define FIRE_WEAPON_BLINDED_BY_SPIT_SPECIAL 7
#define FIRE_WEAPON_TOSSED_OBJECT_SPECIAL 8

#define NO_INTERRUPTS 0
#define ALLOW_INTERRUPTS 1

#define SIGHT_LOOK 0x1
// #define SIGHT_SEND      0x2   // no longer needed using LOCAL OPPLISTs
#define SIGHT_RADIO 0x4
#define SIGHT_INTERRUPT 0x8
#define SIGHT_ALL 0xF

// CHANGE THIS VALUE TO AFFECT TOTAL SIGHT RANGE
#define STRAIGHT_RANGE 13

// CHANGE THESE VALUES TO ADJUST VARIOUS FOV ANGLES
#define STRAIGHT_RATIO 1
#define ANGLE_RATIO 0.857
#define SIDE_RATIO 0.571
// CJC: Changed SBEHIND_RATIO (side-behind ratio) to be 0 to make stealth attacks easier
// Changed on September 21, 1998
// #define SBEHIND_RATIO		0.142
#define SBEHIND_RATIO 0
#define BEHIND_RATIO 0

// looking distance defines
#define BEHIND (int8_t)(BEHIND_RATIO * STRAIGHT_RANGE)
#define SBEHIND (int8_t)(SBEHIND_RATIO * STRAIGHT_RANGE)
#define SIDE (int8_t)(SIDE_RATIO * STRAIGHT_RANGE)
#define ANGLE (int8_t)(ANGLE_RATIO * STRAIGHT_RANGE)
#define STRAIGHT (int8_t)(STRAIGHT_RATIO * STRAIGHT_RANGE)

// opplist value constants
#define HEARD_3_TURNS_AGO -4
#define HEARD_2_TURNS_AGO -3
#define HEARD_LAST_TURN -2
#define HEARD_THIS_TURN -1
#define NOT_HEARD_OR_SEEN 0
#define SEEN_CURRENTLY 1
#define SEEN_THIS_TURN 2
#define SEEN_LAST_TURN 3
#define SEEN_2_TURNS_AGO 4
#define SEEN_3_TURNS_AGO 5

#define OLDEST_SEEN_VALUE SEEN_3_TURNS_AGO
#define OLDEST_HEARD_VALUE HEARD_3_TURNS_AGO

#define UNDER_FIRE 2
#define UNDER_FIRE_LAST_TURN 1

#define MAX_DISTANCE_FOR_PROXIMITY_SIGHT 15

// DEFINES FOR BODY TYPE SUBSTITUTIONS
#define SUB_ANIM_BIGGUYSHOOT2 0x00000001
#define SUB_ANIM_BIGGUYTHREATENSTANCE 0x00000002

// Enumerate directions
typedef enum {
  NORTH = 0,
  NORTHEAST,
  EAST,
  SOUTHEAST,
  SOUTH,
  SOUTHWEST,
  WEST,
  NORTHWEST,
  NUM_WORLD_DIRECTIONS,
  DIRECTION_IRRELEVANT,
  DIRECTION_EXITGRID = 255,
} WorldDirections;

// ENUMERATION OF SOLDIER POSIITONS IN GLOBAL SOLDIER LIST
#define MAX_NUM_SOLDIERS 148
#define NUM_PLANNING_MERCS 8
#define TOTAL_SOLDIERS (NUM_PLANNING_MERCS + MAX_NUM_SOLDIERS)

//-----------------------------------------------
//
// civilian "sub teams":
enum {
  NON_CIV_GROUP = 0,
  REBEL_CIV_GROUP,
  KINGPIN_CIV_GROUP,
  SANMONA_ARMS_GROUP,
  ANGELS_GROUP,
  BEGGARS_CIV_GROUP,
  TOURISTS_CIV_GROUP,
  ALMA_MILITARY_CIV_GROUP,
  DOCTORS_CIV_GROUP,
  COUPLE1_CIV_GROUP,
  HICKS_CIV_GROUP,
  WARDEN_CIV_GROUP,
  JUNKYARD_CIV_GROUP,
  FACTORY_KIDS_GROUP,
  QUEENS_CIV_GROUP,
  UNNAMED_CIV_GROUP_15,
  UNNAMED_CIV_GROUP_16,
  UNNAMED_CIV_GROUP_17,
  UNNAMED_CIV_GROUP_18,
  UNNAMED_CIV_GROUP_19,

  NUM_CIV_GROUPS
};

#define CIV_GROUP_NEUTRAL 0
#define CIV_GROUP_WILL_EVENTUALLY_BECOME_HOSTILE 1
#define CIV_GROUP_WILL_BECOME_HOSTILE 2
#define CIV_GROUP_HOSTILE 3

// boxing state
typedef enum {
  NOT_BOXING = 0,
  BOXING_WAITING_FOR_PLAYER,
  PRE_BOXING,
  BOXING,
  DISQUALIFIED,
  WON_ROUND,
  LOST_ROUND
} BoxingStates;

// NOTE:  The editor uses these enumerations, so please update the text as well if you modify or
//			 add new groups.  Try to abbreviate the team name as much as possible.  The
// text is in 			 EditorMercs.c
extern wchar_t gszCivGroupNames[NUM_CIV_GROUPS][20];

// PALETTE SUBSITUTION TYPES
typedef struct {
  uint8_t ubStart;
  uint8_t ubEnd;

} PaletteSubRangeType;

typedef char PaletteRepID[30];

typedef struct {
  uint8_t ubType;
  PaletteRepID ID;
  uint8_t ubPaletteSize;
  uint8_t *r;
  uint8_t *g;
  uint8_t *b;

} PaletteReplacementType;

// MACROS
// This will set an animation ID
#define SET_PALETTEREP_ID(a, b) (strcpy(a, b))
// strcmp returns 0 if true!
#define COMPARE_PALETTEREP_ID(a, b) (strcmp(a, b) ? FALSE : TRUE)

#endif
