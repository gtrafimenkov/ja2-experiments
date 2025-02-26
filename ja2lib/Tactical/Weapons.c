// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/Weapons.h"

#include <math.h>

#include "GameSettings.h"
#include "SGP/Random.h"
#include "SGP/SoundMan.h"
#include "SGP/Types.h"
#include "Soldier.h"
#include "Strategic/AutoResolve.h"
#include "Strategic/Meanwhile.h"
#include "Strategic/Quests.h"
#include "SysGlobals.h"
#include "Tactical/AnimationControl.h"
#include "Tactical/Bullets.h"
#include "Tactical/Campaign.h"
#include "Tactical/DialogueControl.h"
#include "Tactical/HandleItems.h"
#include "Tactical/HandleUI.h"
#include "Tactical/Interface.h"
#include "Tactical/Items.h"
#include "Tactical/LOS.h"
#include "Tactical/Menptr.h"
#include "Tactical/Morale.h"
#include "Tactical/OppList.h"
#include "Tactical/Overhead.h"
#include "Tactical/OverheadTypes.h"
#include "Tactical/Points.h"
#include "Tactical/SkillCheck.h"
#include "Tactical/SoldierControl.h"
#include "Tactical/SoldierMacros.h"
#include "Tactical/SoldierProfile.h"
#include "Tactical/Vehicles.h"
#include "TacticalAI/AI.h"
#include "TileEngine/ExplosionControl.h"
#include "TileEngine/IsometricUtils.h"
#include "TileEngine/Physics.h"
#include "TileEngine/RenderWorld.h"
#include "TileEngine/SaveLoadMap.h"
#include "TileEngine/SmokeEffects.h"
#include "TileEngine/Structure.h"
#include "TileEngine/StructureInternals.h"
#include "TileEngine/TileAnimation.h"
#include "TileEngine/TileDef.h"
#include "TileEngine/WorldMan.h"
#include "Utils/EventPump.h"
#include "Utils/Message.h"
#include "Utils/SoundControl.h"
#include "Utils/Text.h"

#define MINCHANCETOHIT 1
#define MAXCHANCETOHIT 99

// NB this is arbitrary, chances in DG ranged from 1 in 6 to 1 in 20
#define BASIC_DEPRECIATE_CHANCE 15

#define NORMAL_RANGE 90     // # world units considered an 'avg' shot
#define MIN_SCOPE_RANGE 60  // # world units after which scope's useful

#define MIN_TANK_RANGE 120  // range at which tank starts really having trouble aiming

// percent reduction in sight range per point of aiming
#define SNIPERSCOPE_AIM_BONUS 20
// bonus to hit with working laser scope
#define LASERSCOPE_BONUS 20

#define MANDATORY_WEAPON_DELAY 1200
#define NO_WEAPON_SOUND 0

#define HEAD_DAMAGE_ADJUSTMENT(x) ((x * 3) / 2)
#define LEGS_DAMAGE_ADJUSTMENT(x) (x / 2)

#define CRITICAL_HIT_THRESHOLD 30

#define HTH_MODE_PUNCH 1
#define HTH_MODE_STAB 2
#define HTH_MODE_STEAL 3

// JA2 GOLD: for weapons and attachments, give penalties only for status values below 85
#define WEAPON_STATUS_MOD(x) ((x) >= 85 ? 100 : (((x) * 100) / 85))

extern void TeamChangesSides(uint8_t ubTeam, int8_t bSide);

extern BOOLEAN gfNextFireJam;

BOOLEAN WillExplosiveWeaponFail(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj);

BOOLEAN UseGun(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo);
BOOLEAN UseBlade(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo);
BOOLEAN UseThrown(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo);
BOOLEAN UseLauncher(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo);

int32_t HTHImpact(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTarget, int32_t iHitBy,
                  BOOLEAN fBladeAttack);

BOOLEAN gfNextShotKills = FALSE;
BOOLEAN gfReportHitChances = FALSE;

// GLOBALS

// TODO: Move strings to extern file

#define PISTOL(ammo, update, impact, rt, rof, burstrof, burstpenal, deadl, acc, clip, range, av, \
               hv, sd, bsd)                                                                      \
  {                                                                                              \
      HANDGUNCLASS,                                                                              \
      GUN_PISTOL,                                                                                \
      ammo,                                                                                      \
      rt,                                                                                        \
      rof,                                                                                       \
      burstrof,                                                                                  \
      burstpenal,                                                                                \
      update,                                                                                    \
      (uint8_t)(impact),                                                                         \
      deadl,                                                                                     \
      acc,                                                                                       \
      clip,                                                                                      \
      range,                                                                                     \
      200,                                                                                       \
      av,                                                                                        \
      hv,                                                                                        \
      sd,                                                                                        \
      bsd,                                                                                       \
      S_RELOAD_PISTOL,                                                                           \
      S_LNL_PISTOL}
#define M_PISTOL(ammo, update, impact, rt, rof, burstrof, burstpenal, deadl, acc, clip, range, av, \
                 hv, sd, bsd)                                                                      \
  {                                                                                                \
      HANDGUNCLASS,                                                                                \
      GUN_M_PISTOL,                                                                                \
      ammo,                                                                                        \
      rt,                                                                                          \
      rof,                                                                                         \
      burstrof,                                                                                    \
      burstpenal,                                                                                  \
      update,                                                                                      \
      (uint8_t)(impact),                                                                           \
      deadl,                                                                                       \
      acc,                                                                                         \
      clip,                                                                                        \
      range,                                                                                       \
      200,                                                                                         \
      av,                                                                                          \
      hv,                                                                                          \
      sd,                                                                                          \
      bsd,                                                                                         \
      S_RELOAD_PISTOL,                                                                             \
      S_LNL_PISTOL}
#define SMG(ammo, update, impact, rt, rof, burstrof, burstpenal, deadl, acc, clip, range, av, hv, \
            sd, bsd)                                                                              \
  {SMGCLASS, GUN_SMG, ammo,  rt,  rof, burstrof, burstpenal, update, (uint8_t)(impact), deadl,    \
   acc,      clip,    range, 200, av,  hv,       sd,         bsd,    S_RELOAD_SMG,      S_LNL_SMG}
#define SN_RIFLE(ammo, update, impact, rt, rof, burstrof, deadl, acc, clip, range, av, hv, sd, \
                 bsd)                                                                          \
  {                                                                                            \
      RIFLECLASS,     GUN_SN_RIFLE, ammo, rt,    rof, burstrof, 0,  update, (uint8_t)(impact), \
      deadl,          acc,          clip, range, 200, av,       hv, sd,     bsd,               \
      S_RELOAD_RIFLE, S_LNL_RIFLE}
#define RIFLE(ammo, update, impact, rt, rof, burstrof, deadl, acc, clip, range, av, hv, sd, bsd) \
  {RIFLECLASS, GUN_RIFLE, ammo,  rt,  rof, burstrof, 0,  update, (uint8_t)(impact), deadl,       \
   acc,        clip,      range, 200, av,  hv,       sd, bsd,    S_RELOAD_RIFLE,    S_LNL_RIFLE}
#define ASRIFLE(ammo, update, impact, rt, rof, burstrof, burstpenal, deadl, acc, clip, range, av, \
                hv, sd, bsd)                                                                      \
  {                                                                                               \
      RIFLECLASS,                                                                                 \
      GUN_AS_RIFLE,                                                                               \
      ammo,                                                                                       \
      rt,                                                                                         \
      rof,                                                                                        \
      burstrof,                                                                                   \
      burstpenal,                                                                                 \
      update,                                                                                     \
      (uint8_t)(impact),                                                                          \
      deadl,                                                                                      \
      acc,                                                                                        \
      clip,                                                                                       \
      range,                                                                                      \
      200,                                                                                        \
      av,                                                                                         \
      hv,                                                                                         \
      sd,                                                                                         \
      bsd,                                                                                        \
      S_RELOAD_RIFLE,                                                                             \
      S_LNL_RIFLE}
#define SHOTGUN(ammo, update, impact, rt, rof, burstrof, burstpenal, deadl, acc, clip, range, av, \
                hv, sd, bsd)                                                                      \
  {SHOTGUNCLASS,                                                                                  \
   GUN_SHOTGUN,                                                                                   \
   ammo,                                                                                          \
   rt,                                                                                            \
   rof,                                                                                           \
   burstrof,                                                                                      \
   burstpenal,                                                                                    \
   update,                                                                                        \
   (uint8_t)(impact),                                                                             \
   deadl,                                                                                         \
   acc,                                                                                           \
   clip,                                                                                          \
   range,                                                                                         \
   200,                                                                                           \
   av,                                                                                            \
   hv,                                                                                            \
   sd,                                                                                            \
   bsd,                                                                                           \
   S_RELOAD_SHOTGUN,                                                                              \
   S_LNL_SHOTGUN}
#define LMG(ammo, update, impact, rt, rof, burstrof, burstpenal, deadl, acc, clip, range, av, hv, \
            sd, bsd)                                                                              \
  {MGCLASS, GUN_LMG, ammo,  rt,  rof, burstrof, burstpenal, update, (uint8_t)(impact), deadl,     \
   acc,     clip,    range, 200, av,  hv,       sd,         bsd,    S_RELOAD_LMG,      S_LNL_LMG}
#define BLADE(impact, rof, deadl, range, av, sd) \
  {KNIFECLASS,                                   \
   NOT_GUN,                                      \
   0,                                            \
   AP_READY_KNIFE,                               \
   rof,                                          \
   0,                                            \
   0,                                            \
   0,                                            \
   (uint8_t)(impact),                            \
   deadl,                                        \
   0,                                            \
   0,                                            \
   range,                                        \
   200,                                          \
   av,                                           \
   0,                                            \
   sd,                                           \
   NO_WEAPON_SOUND,                              \
   NO_WEAPON_SOUND,                              \
   NO_WEAPON_SOUND}
#define THROWINGBLADE(impact, rof, deadl, range, av, sd) \
  {KNIFECLASS,                                           \
   NOT_GUN,                                              \
   0,                                                    \
   AP_READY_KNIFE,                                       \
   rof,                                                  \
   0,                                                    \
   0,                                                    \
   0,                                                    \
   (uint8_t)(impact),                                    \
   deadl,                                                \
   0,                                                    \
   0,                                                    \
   range,                                                \
   200,                                                  \
   av,                                                   \
   0,                                                    \
   sd,                                                   \
   NO_WEAPON_SOUND,                                      \
   NO_WEAPON_SOUND,                                      \
   NO_WEAPON_SOUND}
#define PUNCHWEAPON(impact, rof, deadl, av, sd) \
  {KNIFECLASS,                                  \
   NOT_GUN,                                     \
   0,                                           \
   0,                                           \
   rof,                                         \
   0,                                           \
   0,                                           \
   0,                                           \
   (uint8_t)(impact),                           \
   deadl,                                       \
   0,                                           \
   0,                                           \
   10,                                          \
   200,                                         \
   av,                                          \
   0,                                           \
   sd,                                          \
   NO_WEAPON_SOUND,                             \
   NO_WEAPON_SOUND,                             \
   NO_WEAPON_SOUND}
#define LAUNCHER(update, rt, rof, deadl, acc, range, av, hv, sd) \
  {RIFLECLASS,                                                   \
   NOT_GUN,                                                      \
   NOAMMO,                                                       \
   rt,                                                           \
   rof,                                                          \
   0,                                                            \
   0,                                                            \
   update,                                                       \
   1,                                                            \
   deadl,                                                        \
   acc,                                                          \
   0,                                                            \
   range,                                                        \
   200,                                                          \
   av,                                                           \
   hv,                                                           \
   sd,                                                           \
   NO_WEAPON_SOUND,                                              \
   NO_WEAPON_SOUND,                                              \
   NO_WEAPON_SOUND}
#define LAW(update, rt, rof, deadl, acc, range, av, hv, sd) \
  {RIFLECLASS,                                              \
   NOT_GUN,                                                 \
   NOAMMO,                                                  \
   rt,                                                      \
   rof,                                                     \
   0,                                                       \
   0,                                                       \
   update,                                                  \
   80,                                                      \
   deadl,                                                   \
   acc,                                                     \
   1,                                                       \
   range,                                                   \
   200,                                                     \
   av,                                                      \
   hv,                                                      \
   sd,                                                      \
   NO_WEAPON_SOUND,                                         \
   NO_WEAPON_SOUND,                                         \
   NO_WEAPON_SOUND}
#define CANNON(update, rt, rof, deadl, acc, range, av, hv, sd) \
  {RIFLECLASS,                                                 \
   NOT_GUN,                                                    \
   NOAMMO,                                                     \
   rt,                                                         \
   rof,                                                        \
   0,                                                          \
   0,                                                          \
   update,                                                     \
   80,                                                         \
   deadl,                                                      \
   acc,                                                        \
   1,                                                          \
   range,                                                      \
   200,                                                        \
   av,                                                         \
   hv,                                                         \
   sd,                                                         \
   NO_WEAPON_SOUND,                                            \
   NO_WEAPON_SOUND,                                            \
   NO_WEAPON_SOUND}
#define MONSTSPIT(impact, rof, deadl, clip, range, av, hv, sd) \
  {MONSTERCLASS,                                               \
   NOT_GUN,                                                    \
   AMMOMONST,                                                  \
   AP_READY_KNIFE,                                             \
   rof,                                                        \
   0,                                                          \
   0,                                                          \
   250,                                                        \
   (uint8_t)(impact),                                          \
   deadl,                                                      \
   0,                                                          \
   clip,                                                       \
   range,                                                      \
   200,                                                        \
   av,                                                         \
   hv,                                                         \
   sd,                                                         \
   NO_WEAPON_SOUND,                                            \
   NO_WEAPON_SOUND,                                            \
   NO_WEAPON_SOUND}

// ranges are in world units, calculated by:
// 100 + real-range-in-metres/10
// then I scaled them down... I forget how much by!

// Accuracy is based on probability of shot being within 10cm of bullseye target on chest at 25m
// from Compendium of Modern Firearms (Edge of the Sword Vol 1)

// JA2 GOLD: reduced pistol ready time to 0, tweaked sniper rifle values and G11 range
WEAPONTYPE Weapon[MAX_WEAPONS] = {
    //          Description			  Ammo      Bullet	Ready	 4xSng
    //          Burst	Burst	Deadl	Accu	Clip	Range Attack Impact		Fire
    //										   Spd  Imp	Time
    // ROF
    // ROF
    // penal
    // iness
    // racy
    // Size
    // Vol
    // Vol			Sounds
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0},  // nada!  must have min range of 10
    PISTOL(/* Glock 17			*/ AMMO9, 24, 21, 0, 14, 0, 0, 8, 0, 15, 120, 60, 5,
           S_GLOCK17, NO_WEAPON_SOUND),  // wt 6  // Austria
    M_PISTOL(/* Glock 18		*/ AMMO9, 24, 21, 0, 14, 5, 15, 9, 0, 15, 120, 60, 5,
             S_GLOCK18, S_BURSTTYPE1),  // wt 6  // Austria
    PISTOL(/* Beretta 92F     */ AMMO9, 23, 22, 0, 16, 0, 0, 9, 0, 15, 120, 60, 5, S_BERETTA92,
           NO_WEAPON_SOUND),  // wt 11 // Italy
    M_PISTOL(/* Beretta 93R   */ AMMO9, 23, 22, 0, 13, 5, 15, 9, 0, 15, 120, 60, 5, S_BERETTA93,
             S_BURSTTYPE1),  // wt 11 // Italy
    PISTOL(/* .38 S&W Special */ AMMO38, 23, 22, 0, 11, 0, 0, 6, 0, 6, 130, 63, 5, S_SWSPECIAL,
           NO_WEAPON_SOUND),  // wt 11 // Britain
    PISTOL(/* .357 Barracuda  */ AMMO357, 23, 24, 0, 11, 0, 0, 7, 0, 6, 135, 66, 6, S_BARRACUDA,
           NO_WEAPON_SOUND),  // wt 10 // Belgium
    PISTOL(/* .357 DesertEagle*/ AMMO357, 24, 24, 0, 11, 0, 0, 7, 0, 9, 135, 66, 6, S_DESERTEAGLE,
           NO_WEAPON_SOUND),  // wt 17 // US
    PISTOL(/* .45 M1911       */ AMMO45, 24, 23, 0, 13, 0, 0, 9, 0, 7, 125, 69, 6, S_M1911,
           NO_WEAPON_SOUND),  // wt 12 // US

    SMG(/* H&K MP5K      	 */ AMMO9, 23, 23, 1, 15, 5, 8, 17, 0, 30, 200, 75, 7, S_MP5K,
        S_BURSTTYPE1),  // wt 21 // Germany; ROF 900 ?
    SMG(/* .45 MAC-10	     */ AMMO45, 23, 27, 2, 13, 5, 8, 20, 0, 30, 200, 75, 7, S_MAC10,
        S_BURSTTYPE1),  // wt 28 // US; ROF 1090
    SMG(/* Thompson M1A1   */ AMMO45, 23, 24, 2, 10, 4, 8, 14, 0, 30, 200, 75, 7, S_THOMPSON,
        S_BURSTTYPE1),  // wt 48 // US; ROF 700
    SMG(/* Colt Commando   */ AMMO556, 20, 29, 2, 15, 4, 8, 23, 0, 30, 200, 75, 7, S_COMMANDO,
        S_BURSTTYPE1),  // wt 26 // US; ROF
    SMG(/* H&K MP53		 		 */ AMMO556, 22, 25, 2, 12, 3, 8, 15, 0, 30, 200,
        75, 7, S_MP53, S_BURSTTYPE1),  // wt 31 // Germany // eff range assumed; ROF 700 ?
    SMG(/* AKSU-74         */ AMMO545, 21, 26, 2, 17, 4, 8, 21, 0, 30, 200, 75, 7, S_AKSU74,
        S_BURSTTYPE1),  // wt 39 // USSR; ROF 800
    SMG(/* 5.7mm FN P90    */ AMMO57, 21, 30, 2, 15, 5, 8, 42, 0, 50, 225, 75, 7, S_P90,
        S_BURSTTYPE1),  // wt 28 // Belgium; ROF 800-1000
    SMG(/* Type-85         */ AMMO762W, 23, 23, 1, 10, 4, 11, 12, 0, 30, 200, 75, 7, S_TYPE85,
        S_BURSTTYPE1),  // wt 19 // China; ROF 780

    RIFLE(/* SKS             */ AMMO762W, 22, 31, 2, 13, 0, 24, 0, 10, 300, 80, 8, S_SKS,
          S_BURSTTYPE1),  // wt 39 // USSR
    SN_RIFLE(/* Dragunov      */ AMMO762W, 21, 36, 5, 11, 0, 32, 0, 10, 750, 80, 8, S_DRAGUNOV,
             S_BURSTTYPE1),  // wt 43 // USSR
    SN_RIFLE(/* M24           */ AMMO762N, 21, 36, 5, 8, 0, 32, 0, 5, 800, 80, 8, S_M24,
             S_BURSTTYPE1),  // wt 66 // US

    ASRIFLE(/* Steyr AUG       */ AMMO556, 20, 30, 2, 13, 3, 8, 38, 0, 30, 500, 77, 8, S_AUG,
            S_BURSTTYPE1),  // wt 36 // Austria; ROF 650
    ASRIFLE(/* H&K G41         */ AMMO556, 20, 29, 2, 13, 4, 8, 27, 0, 30, 300, 77, 8, S_G41,
            S_BURSTTYPE1),  // wt 41 // Germany; ROF 850
    RIFLE(/* Ruger Mini-14	 */ AMMO556, 20, 30, 2, 13, 0, 20, 0, 30, 250, 77, 8, S_RUGERMINI,
          S_BURSTTYPE1),  // wt 29 // US; ROF 750
    ASRIFLE(/* C-7             */ AMMO556, 20, 30, 2, 15, 5, 8, 41, 0, 30, 400, 77, 8, S_C7,
            S_BURSTTYPE1),  // wt 36 // Canada; ROF 600-940
    ASRIFLE(/* FA-MAS          */ AMMO556, 20, 30, 2, 17, 5, 8, 32, 0, 30, 250, 77, 8, S_FAMAS,
            S_BURSTTYPE1),  // wt 36 // France; ROF 900-1000
    ASRIFLE(/* AK-74           */ AMMO545, 20, 28, 2, 17, 3, 8, 30, 0, 30, 350, 77, 8, S_AK74,
            S_BURSTTYPE1),  // wt 36 // USSR; ROF 650
    ASRIFLE(/* AKM             */ AMMO762W, 22, 29, 2, 17, 3, 11, 25, 0, 30, 250, 77, 8, S_AKM,
            S_BURSTTYPE1),  // wt 43 // USSR; ROF 600
    ASRIFLE(/* M-14            */ AMMO762N, 20, 33, 2, 13, 4, 11, 33, 0, 20, 330, 80, 8, S_M14,
            S_BURSTTYPE1),  // wt 29 // US; ROF 750
    ASRIFLE(/* FN-FAL          */ AMMO762N, 20, 32, 2, 17, 3, 11, 41, 0, 20, 425, 80, 8, S_FNFAL,
            S_BURSTTYPE1),  // wt 43 // Belgium; ROF
    ASRIFLE(/* H&K G3A3        */ AMMO762N, 21, 31, 2, 13, 3, 11, 26, 0, 20, 300, 80, 8, S_G3A3,
            S_BURSTTYPE1),  // wt 44 // Germany; ROF 500-600
    ASRIFLE(/* H&K G11         */ AMMO47, 20, 27, 2, 13, 3, 0, 40, 0, 50, 300, 80, 8, S_G11,
            S_BURSTTYPE1),  // wt 38 // Germany; ROF 600

    SHOTGUN(/* Remington M870  */ AMMO12G, 24, 32, 2, 7, 0, 0, 14, 0, 7, 135, 80, 8, S_M870,
            S_BURSTTYPE1),  // wt 36 // US; damage for solid slug
    SHOTGUN(/* SPAS-15				 */ AMMO12G, 24, 32, 2, 10, 0, 0, 18, 0, 7, 135, 80,
            8, S_SPAS, S_BURSTTYPE1),  // wt 38 // Italy; semi-auto; damage for solid slug
    SHOTGUN(/* CAWS            */ AMMOCAWS, 24, 40, 2, 10, 3, 11, 44, 0, 10, 135, 80, 8, S_CAWS,
            S_BURSTTYPE1),  // wt 41 // US; fires 8 flechettes at once in very close fixed pattern

    LMG(/* FN Minimi       */ AMMO556, 20, 28, 3, 13, 6, 5, 48, 0, 30, 500, 82, 8, S_FNMINI,
        S_BURSTTYPE1),  // wt 68 // Belgium; ROF 750-1000
    LMG(/* RPK-74          */ AMMO545, 21, 30, 2, 13, 5, 5, 49, 0, 30, 500, 82, 8, S_RPK74,
        S_BURSTTYPE1),  // wt 48 // USSR; ROF 800?
    LMG(/* H&K 21E         */ AMMO762N, 21, 32, 3, 13, 5, 7, 52, 0, 20, 500, 82, 8, S_21E,
        S_BURSTTYPE1),  // wt 93 // Germany; ROF 800

    // NB blade distances will be = strength + dexterity /2

    BLADE(/* Combat knife    */ 18, 12, 5, 40, 2, NO_WEAPON_SOUND),
    THROWINGBLADE(/* Throwing knife  */ 15, 12, 4, 150, 2, S_THROWKNIFE),
    {0},  // rock
    LAUNCHER(/* grenade launcher*/ 30, 3, 5, 80, 0, 500, 20, 10, S_GLAUNCHER),
    LAUNCHER(/* mortar */ 30, 0, 5, 100, 0, 550, 20, 10, S_MORTAR_SHOT),
    {0},  // another rock
    BLADE(/* yng male claws */ 14, 10, 1, 10, 2, NO_WEAPON_SOUND),
    BLADE(/* yng fem claws */ 18, 10, 1, 10, 2, NO_WEAPON_SOUND),
    BLADE(/* old male claws */ 20, 10, 1, 10, 2, NO_WEAPON_SOUND),
    BLADE(/* old fem claws */ 24, 10, 1, 10, 2, NO_WEAPON_SOUND),
    BLADE(/* queen's tentacles */ 20, 10, 1, 70, 2, NO_WEAPON_SOUND),
    MONSTSPIT(/* queen's spit */ 20, 10, 1, 50, 300, 10, 5, ACR_SPIT),
    PUNCHWEAPON(/* brass knuckles */ 12, 15, 1, 0, 0),
    LAUNCHER(/* underslung GL */ 30, 3, 7, 80, 0, 450, 20, 10, S_UNDER_GLAUNCHER),
    LAW(/* rocket laucher */ 30, 0, 5, 80, 0, 500, 80, 10, S_ROCKET_LAUNCHER),
    BLADE(/* bloodcat claws */ 12, 14, 1, 10, 2, NO_WEAPON_SOUND),
    BLADE(/* bloodcat bite */ 24, 10, 1, 10, 2, NO_WEAPON_SOUND),
    BLADE(/* machete */ 24, 9, 6, 40, 2, NO_WEAPON_SOUND),
    RIFLE(/* rocket rifle */ AMMOROCKET, 20, 38, 2, 10, 0, 62, 0, 5, 600, 80, 10,
          S_SMALL_ROCKET_LAUNCHER, NO_WEAPON_SOUND),
    PISTOL(/* automag III     */ AMMO762N, 24, 29, 1, 9, 0, 0, 13, 0, 5, 220, 72, 6, S_AUTOMAG,
           NO_WEAPON_SOUND),
    MONSTSPIT(/* infant spit */ 12, 13, 1, 5, 200, 10, 5, ACR_SPIT),
    MONSTSPIT(/* yng male spit */ 16, 10, 1, 10, 200, 10, 5, ACR_SPIT),
    MONSTSPIT(/* old male spit */ 20, 10, 1, 20, 200, 10, 5, ACR_SPIT),
    CANNON(/* tank cannon*/ 30, 0, 8, 80, 0, 800, 90, 10, S_TANK_CANNON),
    PISTOL(/* DART GUN		    */ AMMODART, 25, 2, 1, 13, 0, 0, 10, 0, 1, 200, 0, 0,
           NO_WEAPON_SOUND, NO_WEAPON_SOUND),
    THROWINGBLADE(/* Bloody Thrw KN */ 15, 12, 3, 150, 2, S_THROWKNIFE),

    SHOTGUN(/* Flamethrower */ AMMOFLAME, 24, 60, 2, 10, 0, 0, 53, 0, 5, 130, 40, 8, S_CAWS,
            S_BURSTTYPE1),
    PUNCHWEAPON(/* crowbar */ 25, 10, 4, 0, 0),
    ASRIFLE(/* auto rckt rifle */ AMMOROCKET, 20, 38, 2, 12, 5, 10, 97, 0, 5, 600, 80, 10,
            S_SMALL_ROCKET_LAUNCHER, S_BURSTTYPE1),
};

MAGTYPE Magazine[] = {
    // calibre,			 mag size,			ammo type
    {AMMO9, 15, AMMO_REGULAR},      {AMMO9, 30, AMMO_REGULAR},
    {AMMO9, 15, AMMO_AP},           {AMMO9, 30, AMMO_AP},
    {AMMO9, 15, AMMO_HP},           {AMMO9, 30, AMMO_HP},
    {AMMO38, 6, AMMO_REGULAR},      {AMMO38, 6, AMMO_AP},
    {AMMO38, 6, AMMO_HP},           {AMMO45, 7, AMMO_REGULAR},
    {AMMO45, 30, AMMO_REGULAR},     {AMMO45, 7, AMMO_AP},
    {AMMO45, 30, AMMO_AP},          {AMMO45, 7, AMMO_HP},
    {AMMO45, 30, AMMO_HP},          {AMMO357, 6, AMMO_REGULAR},
    {AMMO357, 9, AMMO_REGULAR},     {AMMO357, 6, AMMO_AP},
    {AMMO357, 9, AMMO_AP},          {AMMO357, 6, AMMO_HP},
    {AMMO357, 9, AMMO_HP},          {AMMO545, 30, AMMO_AP},
    {AMMO545, 30, AMMO_HP},         {AMMO556, 30, AMMO_AP},
    {AMMO556, 30, AMMO_HP},         {AMMO762W, 10, AMMO_AP},
    {AMMO762W, 30, AMMO_AP},        {AMMO762W, 10, AMMO_HP},
    {AMMO762W, 30, AMMO_HP},        {AMMO762N, 5, AMMO_AP},
    {AMMO762N, 20, AMMO_AP},        {AMMO762N, 5, AMMO_HP},
    {AMMO762N, 20, AMMO_HP},        {AMMO47, 50, AMMO_SUPER_AP},
    {AMMO57, 50, AMMO_AP},          {AMMO57, 50, AMMO_HP},
    {AMMO12G, 7, AMMO_BUCKSHOT},    {AMMO12G, 7, AMMO_REGULAR},
    {AMMOCAWS, 10, AMMO_BUCKSHOT},  {AMMOCAWS, 10, AMMO_SUPER_AP},
    {AMMOROCKET, 5, AMMO_SUPER_AP}, {AMMOROCKET, 5, AMMO_HE},
    {AMMOROCKET, 5, AMMO_HEAT},     {AMMODART, 1, AMMO_SLEEP_DART},
    {AMMOFLAME, 5, AMMO_BUCKSHOT},  {NOAMMO, 0, 0}};

ARMOURTYPE Armour[] = {
    //	Class					      Protection	Degradation%
    // Description
    //  -------------       ----------  ------------      ----------------
    {ARMOURCLASS_VEST, 10, 25},     /* Flak jacket     */
    {ARMOURCLASS_VEST, 13, 20},     /* Flak jacket w X */
    {ARMOURCLASS_VEST, 16, 15},     /* Flak jacket w Y */
    {ARMOURCLASS_VEST, 15, 20},     /* Kevlar jacket   */
    {ARMOURCLASS_VEST, 19, 15},     /* Kevlar jack w X */
    {ARMOURCLASS_VEST, 24, 10},     /* Kevlar jack w Y */
    {ARMOURCLASS_VEST, 30, 15},     /* Spectra jacket  */
    {ARMOURCLASS_VEST, 36, 10},     /* Spectra jack w X*/
    {ARMOURCLASS_VEST, 42, 5},      /* Spectra jack w Y*/
    {ARMOURCLASS_LEGGINGS, 15, 20}, /* Kevlar leggings */
    {ARMOURCLASS_LEGGINGS, 19, 15}, /* Kevlar legs w X */

    {ARMOURCLASS_LEGGINGS, 24, 10}, /* Kevlar legs w Y */
    {ARMOURCLASS_LEGGINGS, 30, 15}, /* Spectra leggings*/
    {ARMOURCLASS_LEGGINGS, 36, 10}, /* Spectra legs w X*/
    {ARMOURCLASS_LEGGINGS, 42, 5},  /* Spectra legs w Y*/
    {ARMOURCLASS_HELMET, 10, 5},    /* Steel helmet    */
    {ARMOURCLASS_HELMET, 15, 20},   /* Kevlar helmet   */
    {ARMOURCLASS_HELMET, 19, 15},   /* Kevlar helm w X */
    {ARMOURCLASS_HELMET, 24, 10},   /* Kevlar helm w Y */
    {ARMOURCLASS_HELMET, 30, 15},   /* Spectra helmet  */
    {ARMOURCLASS_HELMET, 36, 10},   /* Spectra helm w X*/

    {ARMOURCLASS_HELMET, 42, 5},  /* Spectra helm w Y*/
    {ARMOURCLASS_PLATE, 15, 200}, /* Ceramic plates  */
    {ARMOURCLASS_MONST, 3, 0},    /* Infant creature hide */
    {ARMOURCLASS_MONST, 5, 0},    /* Young male creature hide  */
    {ARMOURCLASS_MONST, 6, 0},    /* Male creature hide  */
    {ARMOURCLASS_MONST, 20, 0},   /* Queen creature hide  */
    {ARMOURCLASS_VEST, 2, 25},    /* Leather jacket    */
    {ARMOURCLASS_VEST, 12, 30},   /* Leather jacket w kevlar */
    {ARMOURCLASS_VEST, 16, 25},   /* Leather jacket w kevlar & compound 18 */
    {ARMOURCLASS_VEST, 19, 20},   /* Leather jacket w kevlar & queen blood */

    {ARMOURCLASS_MONST, 7, 0},  /* Young female creature hide */
    {ARMOURCLASS_MONST, 8, 0},  /* Old female creature hide  */
    {ARMOURCLASS_VEST, 1, 25},  /* T-shirt */
    {ARMOURCLASS_VEST, 22, 20}, /* Kevlar 2 jacket   */
    {ARMOURCLASS_VEST, 27, 15}, /* Kevlar 2 jack w X */
    {ARMOURCLASS_VEST, 32, 10}, /* Kevlar 2 jack w Y */
};

EXPLOSIVETYPE Explosive[] = {
    //	Type							Yield		Yield2
    // Radius
    // Volume
    // Volatility
    // Animation
    // Description
    //										-----
    //-------
    //------
    //------
    //----------
    //---------
    //------------------
    {EXPLOSV_STUN, 1, 70, 4, 0, 0, STUN_BLAST /* stun grenade       */},
    {EXPLOSV_TEARGAS, 0, 20, 4, 0, 0, TARGAS_EXP /* tear gas grenade   */},
    {EXPLOSV_MUSTGAS, 15, 40, 4, 0, 0, MUSTARD_EXP /* mustard gas grenade*/},
    {EXPLOSV_NORMAL, 15, 7, 3, 15, 1, BLAST_1 /* mini hand grenade  */},
    {EXPLOSV_NORMAL, 25, 10, 4, 25, 1, BLAST_1 /* reg hand grenade   */},
    {EXPLOSV_NORMAL, 40, 12, 5, 20, 10, BLAST_2 /* RDX                */},
    {EXPLOSV_NORMAL, 50, 15, 5, 50, 2, BLAST_2 /* TNT (="explosives")*/},
    {EXPLOSV_NORMAL, 60, 15, 6, 60, 2, BLAST_2 /* HMX (=RDX+TNT)     */},
    {EXPLOSV_NORMAL, 55, 15, 6, 55, 0, BLAST_2 /* C1  (=RDX+min oil) */},
    {EXPLOSV_NORMAL, 50, 22, 6, 50, 2, BLAST_2 /* mortar shell       */},

    {EXPLOSV_NORMAL, 30, 30, 2, 30, 2, BLAST_1 /* mine               */},
    {EXPLOSV_NORMAL, 65, 30, 7, 65, 0, BLAST_1 /* C4  ("plastique")  */},
    {EXPLOSV_FLARE, 0, 0, 10, 0, 0, BLAST_1 /* trip flare				  */},
    {EXPLOSV_NOISE, 0, 0, 50, 50, 0, BLAST_1 /* trip klaxon        */},
    {EXPLOSV_NORMAL, 20, 0, 1, 20, 0, BLAST_1 /* shaped charge      */},
    {EXPLOSV_FLARE, 0, 0, 10, 0, 0, BLAST_1, /* break light        */},
    {EXPLOSV_NORMAL, 25, 5, 4, 25, 1, BLAST_1, /* GL grenade					*/},
    {EXPLOSV_TEARGAS, 0, 20, 3, 0, 0, TARGAS_EXP, /* GL tear gas grenade*/},
    {EXPLOSV_STUN, 1, 50, 4, 0, 0, STUN_BLAST, /* GL stun grenade		*/},
    {EXPLOSV_SMOKE, 0, 0, 3, 0, 0, SMOKE_EXP, /* GL smoke grenade		*/},

    {EXPLOSV_SMOKE, 0, 0, 4, 0, 0, SMOKE_EXP, /* smoke grenade			*/},
    {EXPLOSV_NORMAL, 60, 20, 6, 60, 2, BLAST_2, /* Tank Shell         */},
    {EXPLOSV_NORMAL, 100, 0, 0, 0, 0, BLAST_1, /* Fake structure igniter*/},
    {EXPLOSV_NORMAL, 100, 0, 1, 0, 0, BLAST_1, /* creature cocktail */},
    {EXPLOSV_NORMAL, 50, 10, 5, 50, 2, BLAST_2, /* fake struct explosion*/},
    {EXPLOSV_NORMAL, 50, 10, 5, 50, 2, BLAST_3, /* fake vehicle explosion*/},
    {EXPLOSV_TEARGAS, 0, 40, 4, 0, 0, TARGAS_EXP /* big tear gas */},
    {EXPLOSV_CREATUREGAS, 5, 0, 1, 0, 0, NO_BLAST /* small creature gas*/},
    {EXPLOSV_CREATUREGAS, 8, 0, 3, 0, 0, NO_BLAST /* big creature gas*/},
    {EXPLOSV_CREATUREGAS, 0, 0, 0, 0, 0, NO_BLAST /* vry sm creature gas*/},
};

int8_t gzBurstSndStrings[][30] = {
    "",                    // NOAMMO
    "",                    // 38
    "9mm Burst ",          // 9mm
    "45 Caliber Burst ",   // 45
    "",                    // 357
    "",                    // 12G
    "Shotgun Burst ",      // CAWS
    "5,45 Burst ",         // 5.45
    "5,56 Burst ",         // 5.56
    "7,62 NATO Burst ",    // 7,62 N
    "7,62 WP Burst ",      // 7,62 W
    "4,7 Caliber Burst ",  // 4.7
    "5,7 Burst ",          // 5,7
    "",                    // Monster
    "RL Automatic ",       // Rocket
    "",                    // Dart
    "",                    // Flame (unused)
};

// the amount of momentum reduction for the head, torso, and legs
// used to determine whether the bullet will go through someone
uint8_t BodyImpactReduction[4] = {0, 15, 30, 23};

uint16_t GunRange(struct OBJECTTYPE *pObj) {
  int8_t bAttachPos;

  if (Item[pObj->usItem].usItemClass & IC_WEAPON) {
    bAttachPos = FindAttachment(pObj, GUN_BARREL_EXTENDER);

    if (bAttachPos == ITEM_NOT_FOUND) {
      return (Weapon[pObj->usItem].usRange);
    } else {
      return (Weapon[pObj->usItem].usRange +
              (GUN_BARREL_RANGE_BONUS * WEAPON_STATUS_MOD(pObj->bAttachStatus[bAttachPos]) / 100));
    }

  } else {
    // return a minimal value of 1 tile
    return (CELL_X_SIZE);
  }
}

int8_t EffectiveArmour(struct OBJECTTYPE *pObj) {
  int32_t iValue;
  int8_t bPlate;

  if (pObj == NULL || Item[pObj->usItem].usItemClass != IC_ARMOUR) {
    return (0);
  }
  iValue = Armour[Item[pObj->usItem].ubClassIndex].ubProtection;
  iValue = iValue * pObj->bStatus[0] / 100;

  bPlate = FindAttachment(pObj, CERAMIC_PLATES);
  if (bPlate != ITEM_NOT_FOUND) {
    int32_t iValue2;

    iValue2 = Armour[Item[CERAMIC_PLATES].ubClassIndex].ubProtection;
    iValue2 = iValue2 * pObj->bAttachStatus[bPlate] / 100;

    iValue += iValue2;
  }
  return ((int8_t)iValue);
}

int8_t ArmourPercent(struct SOLDIERTYPE *pSoldier) {
  int32_t iVest, iHelmet, iLeg;

  if (pSoldier->inv[VESTPOS].usItem) {
    iVest = EffectiveArmour(&(pSoldier->inv[VESTPOS]));
    // convert to % of best; ignoring bug-treated stuff
    iVest = 65 * iVest /
            (Armour[Item[SPECTRA_VEST_18].ubClassIndex].ubProtection +
             Armour[Item[CERAMIC_PLATES].ubClassIndex].ubProtection);
  } else {
    iVest = 0;
  }

  if (pSoldier->inv[HELMETPOS].usItem) {
    iHelmet = EffectiveArmour(&(pSoldier->inv[HELMETPOS]));
    // convert to % of best; ignoring bug-treated stuff
    iHelmet = 15 * iHelmet / Armour[Item[SPECTRA_HELMET_18].ubClassIndex].ubProtection;
  } else {
    iHelmet = 0;
  }

  if (pSoldier->inv[LEGPOS].usItem) {
    iLeg = EffectiveArmour(&(pSoldier->inv[LEGPOS]));
    // convert to % of best; ignoring bug-treated stuff
    iLeg = 25 * iLeg / Armour[Item[SPECTRA_LEGGINGS_18].ubClassIndex].ubProtection;
  } else {
    iLeg = 0;
  }
  return ((int8_t)(iHelmet + iVest + iLeg));
}

int8_t ExplosiveEffectiveArmour(struct OBJECTTYPE *pObj) {
  int32_t iValue;
  int8_t bPlate;

  if (pObj == NULL || Item[pObj->usItem].usItemClass != IC_ARMOUR) {
    return (0);
  }
  iValue = Armour[Item[pObj->usItem].ubClassIndex].ubProtection;
  iValue = iValue * pObj->bStatus[0] / 100;
  if (pObj->usItem == FLAK_JACKET || pObj->usItem == FLAK_JACKET_18 ||
      pObj->usItem == FLAK_JACKET_Y) {
    // increase value for flak jackets!
    iValue *= 3;
  }

  bPlate = FindAttachment(pObj, CERAMIC_PLATES);
  if (bPlate != ITEM_NOT_FOUND) {
    int32_t iValue2;

    iValue2 = Armour[Item[CERAMIC_PLATES].ubClassIndex].ubProtection;
    iValue2 = iValue2 * pObj->bAttachStatus[bPlate] / 100;

    iValue += iValue2;
  }
  return ((int8_t)iValue);
}

int8_t ArmourVersusExplosivesPercent(struct SOLDIERTYPE *pSoldier) {
  // returns the % damage reduction from grenades
  int32_t iVest, iHelmet, iLeg;

  if (pSoldier->inv[VESTPOS].usItem) {
    iVest = ExplosiveEffectiveArmour(&(pSoldier->inv[VESTPOS]));
    // convert to % of best; ignoring bug-treated stuff
    iVest = min(65, 65 * iVest /
                        (Armour[Item[SPECTRA_VEST_18].ubClassIndex].ubProtection +
                         Armour[Item[CERAMIC_PLATES].ubClassIndex].ubProtection));
  } else {
    iVest = 0;
  }

  if (pSoldier->inv[HELMETPOS].usItem) {
    iHelmet = ExplosiveEffectiveArmour(&(pSoldier->inv[HELMETPOS]));
    // convert to % of best; ignoring bug-treated stuff
    iHelmet = min(15, 15 * iHelmet / Armour[Item[SPECTRA_HELMET_18].ubClassIndex].ubProtection);
  } else {
    iHelmet = 0;
  }

  if (pSoldier->inv[LEGPOS].usItem) {
    iLeg = ExplosiveEffectiveArmour(&(pSoldier->inv[LEGPOS]));
    // convert to % of best; ignoring bug-treated stuff
    iLeg = min(25, 25 * iLeg / Armour[Item[SPECTRA_LEGGINGS_18].ubClassIndex].ubProtection);
  } else {
    iLeg = 0;
  }
  return ((int8_t)(iHelmet + iVest + iLeg));
}

void AdjustImpactByHitLocation(int32_t iImpact, uint8_t ubHitLocation, int32_t *piNewImpact,
                               int32_t *piImpactForCrits) {
  switch (ubHitLocation) {
    case AIM_SHOT_HEAD:
      // 1.5x damage from successful hits to the head!
      *piImpactForCrits = HEAD_DAMAGE_ADJUSTMENT(iImpact);
      *piNewImpact = *piImpactForCrits;
      break;
    case AIM_SHOT_LEGS:
      // half damage for determining critical hits
      // quarter actual damage
      *piImpactForCrits = LEGS_DAMAGE_ADJUSTMENT(iImpact);
      *piNewImpact = LEGS_DAMAGE_ADJUSTMENT(*piImpactForCrits);
      break;
    default:
      *piImpactForCrits = iImpact;
      *piNewImpact = iImpact;
      break;
  }
}

// #define	TESTGUNJAM

BOOLEAN CheckForGunJam(struct SOLDIERTYPE *pSoldier) {
  struct OBJECTTYPE *pObj;
  int32_t iChance, iResult;

  // should jams apply to enemies?
  if (pSoldier->uiStatusFlags & SOLDIER_PC) {
    if (Item[pSoldier->usAttackingWeapon].usItemClass == IC_GUN &&
        !EXPLOSIVE_GUN(pSoldier->usAttackingWeapon)) {
      pObj = &(pSoldier->inv[pSoldier->ubAttackingHand]);
      if (pObj->bGunAmmoStatus > 0) {
        // gun might jam, figure out the chance
        iChance = (80 - pObj->bGunStatus);

        // CJC: removed reliability from formula...

        // jams can happen to unreliable guns "earlier" than normal or reliable ones.
        // iChance = iChance - Item[pObj->usItem].bReliability * 2;

        // decrease the chance of a jam by 20% per point of reliability;
        // increased by 20% per negative point...
        // iChance = iChance * (10 - Item[pObj->usItem].bReliability * 2) / 10;

        if (pSoldier->bDoBurst > 1) {
          // if at bullet in a burst after the first, higher chance
          iChance -= PreRandom(80);
        } else {
          iChance -= PreRandom(100);
        }

#ifdef TESTGUNJAM
        if (1)
#else
        if ((int32_t)PreRandom(100) < iChance || gfNextFireJam)
#endif
        {
          gfNextFireJam = FALSE;

          // jam! negate the gun ammo status.
          pObj->bGunAmmoStatus *= -1;

          // Deduct AMMO!
          DeductAmmo(pSoldier, pSoldier->ubAttackingHand);

          TacticalCharacterDialogue(pSoldier, QUOTE_JAMMED_GUN);
          return (TRUE);
        }
      } else if (pObj->bGunAmmoStatus < 0) {
        // try to unjam gun
        iResult =
            SkillCheck(pSoldier, UNJAM_GUN_CHECK, (int8_t)(Item[pObj->usItem].bReliability * 4));
        if (iResult > 0) {
          // yay! unjammed the gun
          pObj->bGunAmmoStatus *= -1;

          // MECHANICAL/DEXTERITY GAIN: Unjammed a gun
          StatChange(pSoldier, MECHANAMT, 5, FALSE);
          StatChange(pSoldier, DEXTAMT, 5, FALSE);

          DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);

          // We unjammed gun, return appropriate value!
          return (255);
        } else {
          return (TRUE);
        }
      }
    }
  }
  return (FALSE);
}

BOOLEAN OKFireWeapon(struct SOLDIERTYPE *pSoldier) {
  BOOLEAN bGunJamVal;

  // 1) Are we attacking with our second hand?
  if (pSoldier->ubAttackingHand == SECONDHANDPOS) {
    if (!EnoughAmmo(pSoldier, FALSE, pSoldier->ubAttackingHand)) {
      if (pSoldier->bTeam == gbPlayerNum) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_2ND_CLIP_DEPLETED]);
        return (FALSE);
      }
    }
  }

  bGunJamVal = CheckForGunJam(pSoldier);

  if (bGunJamVal == 255) {
    return (255);
  }

  if (bGunJamVal) {
    return (FALSE);
  }

  return (TRUE);
}

BOOLEAN FireWeapon(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  // ignore passed in target gridno for now

  // If realtime and we are reloading - do not fire until counter is done!
  if (((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT)) &&
      !pSoldier->bDoBurst) {
    if (pSoldier->fReloading) {
      return (FALSE);
    }
  }

  // if target gridno is the same as ours, do not fire!
  if (sTargetGridNo == pSoldier->sGridNo) {
    // FREE UP NPC!
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - attack on own gridno!"));
    FreeUpAttacker((uint8_t)pSoldier->ubID);
    return (FALSE);
  }

  // SET ATTACKER TO NOBODY, WILL GET SET EVENTUALLY
  pSoldier->ubOppNum = NOBODY;

  switch (Item[pSoldier->usAttackingWeapon].usItemClass) {
    case IC_THROWING_KNIFE:
    case IC_GUN:

      // ATE: PAtch up - bookkeeping for spreading done out of whak
      if (pSoldier->fDoSpread && !pSoldier->bDoBurst) {
        pSoldier->fDoSpread = FALSE;
      }

      if (pSoldier->fDoSpread >= 6) {
        pSoldier->fDoSpread = FALSE;
      }

      if (pSoldier->fDoSpread) {
        if (pSoldier->sSpreadLocations[pSoldier->fDoSpread - 1] != 0) {
          UseGun(pSoldier, pSoldier->sSpreadLocations[pSoldier->fDoSpread - 1]);
        } else {
          UseGun(pSoldier, sTargetGridNo);
        }
        pSoldier->fDoSpread++;
      } else {
        UseGun(pSoldier, sTargetGridNo);
      }
      break;
    case IC_BLADE:

      UseBlade(pSoldier, sTargetGridNo);
      break;
    case IC_PUNCH:
      UseHandToHand(pSoldier, sTargetGridNo, FALSE);
      break;

    case IC_LAUNCHER:
      UseLauncher(pSoldier, sTargetGridNo);
      break;

    default:
      // attempt to throw
      UseThrown(pSoldier, sTargetGridNo);
      break;
  }
  return (TRUE);
}

void GetTargetWorldPositions(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo, float *pdXPos,
                             float *pdYPos, float *pdZPos) {
  float dTargetX;
  float dTargetY;
  float dTargetZ;
  struct SOLDIERTYPE *pTargetSoldier;
  int8_t bStructHeight;
  int16_t sXMapPos, sYMapPos;
  uint32_t uiRoll;

  pTargetSoldier = SimpleFindSoldier(sTargetGridNo, pSoldier->bTargetLevel);
  if (pTargetSoldier) {
    // SAVE OPP ID
    pSoldier->ubOppNum = pTargetSoldier->ubID;
    dTargetX = (float)CenterX(pTargetSoldier->sGridNo);
    dTargetY = (float)CenterY(pTargetSoldier->sGridNo);
    if (pSoldier->bAimShotLocation == AIM_SHOT_RANDOM) {
      uiRoll = PreRandom(100);
      if (uiRoll < 15) {
        pSoldier->bAimShotLocation = AIM_SHOT_LEGS;
      } else if (uiRoll > 94) {
        pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
      } else {
        pSoldier->bAimShotLocation = AIM_SHOT_TORSO;
      }
      if (pSoldier->bAimShotLocation != AIM_SHOT_HEAD) {
        uint32_t uiChanceToGetThrough = SoldierToSoldierBodyPartChanceToGetThrough(
            pSoldier, pTargetSoldier, pSoldier->bAimShotLocation);

        if (uiChanceToGetThrough < 25) {
          if (SoldierToSoldierBodyPartChanceToGetThrough(pSoldier, pTargetSoldier, AIM_SHOT_HEAD) >
              uiChanceToGetThrough * 2) {
            // try for a head shot then
            pSoldier->bAimShotLocation = AIM_SHOT_HEAD;
          }
        }
      }
    }

    switch (pSoldier->bAimShotLocation) {
      case AIM_SHOT_HEAD:
        CalculateSoldierZPos(pTargetSoldier, HEAD_TARGET_POS, &dTargetZ);
        break;
      case AIM_SHOT_TORSO:
        CalculateSoldierZPos(pTargetSoldier, TORSO_TARGET_POS, &dTargetZ);
        break;
      case AIM_SHOT_LEGS:
        CalculateSoldierZPos(pTargetSoldier, LEGS_TARGET_POS, &dTargetZ);
        break;
      default:
        // %)@#&(%?
        CalculateSoldierZPos(pTargetSoldier, TARGET_POS, &dTargetZ);
        break;
    }
  } else {
    // GET TARGET XY VALUES
    ConvertGridNoToCenterCellXY(sTargetGridNo, &sXMapPos, &sYMapPos);

    // fire at centre of tile
    dTargetX = (float)sXMapPos;
    dTargetY = (float)sYMapPos;
    if (pSoldier->bTargetCubeLevel) {
      // fire at the centre of the cube specified
      dTargetZ =
          ((float)(pSoldier->bTargetCubeLevel + pSoldier->bTargetLevel * PROFILE_Z_SIZE) - 0.5f) *
          HEIGHT_UNITS_PER_INDEX;
    } else {
      bStructHeight =
          GetStructureTargetHeight(sTargetGridNo, (BOOLEAN)(pSoldier->bTargetLevel == 1));
      if (bStructHeight > 0) {
        // fire at the centre of the cube *one below* the tallest of the tallest structure
        if (bStructHeight > 1) {
          // reduce target level by 1
          bStructHeight--;
        }
        dTargetZ = ((float)(bStructHeight + pSoldier->bTargetLevel * PROFILE_Z_SIZE) - 0.5f) *
                   HEIGHT_UNITS_PER_INDEX;
      } else {
        // fire at 1 unit above the level of the ground
        dTargetZ = (float)(pSoldier->bTargetLevel * PROFILE_Z_SIZE) * HEIGHT_UNITS_PER_INDEX + 1;
      }
    }
    // adjust for terrain height
    dTargetZ += CONVERT_PIXELS_TO_HEIGHTUNITS(gpWorldLevelData[sTargetGridNo].sHeight);
  }

  *pdXPos = dTargetX;
  *pdYPos = dTargetY;
  *pdZPos = dTargetZ;
}

BOOLEAN UseGun(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  uint32_t uiHitChance, uiDiceRoll;
  int16_t sXMapPos, sYMapPos;
  int16_t sAPCost;
  float dTargetX;
  float dTargetY;
  float dTargetZ;
  uint16_t usItemNum;
  BOOLEAN fBuckshot;
  uint8_t ubVolume;
  int8_t bSilencerPos;
  char zBurstString[50];
  uint8_t ubDirection;
  int16_t sNewGridNo;
  uint8_t ubMerc;
  BOOLEAN fGonnaHit = FALSE;
  uint16_t usExpGain = 0;
  uint32_t uiDepreciateTest;

  // Deduct points!
  sAPCost = CalcTotalAPsToAttack(pSoldier, sTargetGridNo, FALSE, pSoldier->bAimTime);

  usItemNum = pSoldier->usAttackingWeapon;

  if (pSoldier->bDoBurst) {
    // ONly deduct points once
    if (pSoldier->bDoBurst == 1) {
      if (Weapon[usItemNum].sBurstSound != NO_WEAPON_SOUND) {
        // IF we are silenced?
        if (FindAttachment(&(pSoldier->inv[pSoldier->ubAttackingHand]), SILENCER) != NO_SLOT) {
          // Pick sound file baed on how many bullets we are going to fire...
          sprintf(zBurstString, "SOUNDS\\WEAPONS\\SILENCER BURST %d.wav", pSoldier->bBulletsLeft);

          // Try playing sound...
          pSoldier->iBurstSoundID = PlayJA2SampleFromFile(
              zBurstString, RATE_11025, SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
              SoundDir(pSoldier->sGridNo));
        } else {
          // Pick sound file baed on how many bullets we are going to fire...
          sprintf(zBurstString, "SOUNDS\\WEAPONS\\%s%d.wav",
                  gzBurstSndStrings[Weapon[usItemNum].ubCalibre], pSoldier->bBulletsLeft);

          // Try playing sound...
          pSoldier->iBurstSoundID = PlayJA2SampleFromFile(
              zBurstString, RATE_11025, SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
              SoundDir(pSoldier->sGridNo));
        }

        if (pSoldier->iBurstSoundID == NO_SAMPLE) {
          // If failed, play normal default....
          pSoldier->iBurstSoundID = PlayJA2Sample(Weapon[usItemNum].sBurstSound, RATE_11025,
                                                  SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
                                                  SoundDir(pSoldier->sGridNo));
        }
      }

      DeductPoints(pSoldier, sAPCost, 0);
    }

  } else {
    // ONLY DEDUCT FOR THE FIRST HAND when doing two-pistol attacks
    if (IsValidSecondHandShot(pSoldier) && pSoldier->inv[HANDPOS].bGunStatus >= USABLE &&
        pSoldier->inv[HANDPOS].bGunAmmoStatus > 0) {
      // only deduct APs when the main gun fires
      if (pSoldier->ubAttackingHand == HANDPOS) {
        DeductPoints(pSoldier, sAPCost, 0);
      }
    } else {
      DeductPoints(pSoldier, sAPCost, 0);
    }

    // PLAY SOUND
    // ( For throwing knife.. it's earlier in the animation
    if (Weapon[usItemNum].sSound != NO_WEAPON_SOUND &&
        Item[usItemNum].usItemClass != IC_THROWING_KNIFE) {
      // Switch on silencer...
      if (FindAttachment(&(pSoldier->inv[pSoldier->ubAttackingHand]), SILENCER) != NO_SLOT) {
        int32_t uiSound;

        if (Weapon[usItemNum].ubCalibre == AMMO9 || Weapon[usItemNum].ubCalibre == AMMO38 ||
            Weapon[usItemNum].ubCalibre == AMMO57) {
          uiSound = S_SILENCER_1;
        } else {
          uiSound = S_SILENCER_2;
        }

        PlayJA2Sample(uiSound, RATE_11025, SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1,
                      SoundDir(pSoldier->sGridNo));

      } else {
        PlayJA2Sample(Weapon[usItemNum].sSound, RATE_11025,
                      SoundVolume(HIGHVOLUME, pSoldier->sGridNo), 1, SoundDir(pSoldier->sGridNo));
      }
    }
  }

  // CALC CHANCE TO HIT
  if (Item[usItemNum].usItemClass == IC_THROWING_KNIFE) {
    uiHitChance = CalcThrownChanceToHit(pSoldier, sTargetGridNo, pSoldier->bAimTime,
                                        pSoldier->bAimShotLocation);
  } else {
    uiHitChance =
        CalcChanceToHitGun(pSoldier, sTargetGridNo, pSoldier->bAimTime, pSoldier->bAimShotLocation);
  }

  // ATE: Added if we are in meanwhile, we always hit...
  if (AreInMeanwhile()) {
    uiHitChance = 100;
  }

  // ROLL DICE
  uiDiceRoll = PreRandom(100);

#ifdef JA2BETAVERSION
  if (gfReportHitChances) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Hit chance was %ld, roll %ld (range %d)",
              uiHitChance, uiDiceRoll, PythSpacesAway(pSoldier->sGridNo, pSoldier->sTargetGridNo));
  }
#endif

  fGonnaHit = uiDiceRoll <= uiHitChance;

  // GET TARGET XY VALUES
  ConvertGridNoToCenterCellXY(sTargetGridNo, &sXMapPos, &sYMapPos);

  // ATE; Moved a whole blotch if logic code for finding target positions to a function
  // so other places can use it
  GetTargetWorldPositions(pSoldier, sTargetGridNo, &dTargetX, &dTargetY, &dTargetZ);

  // Some things we don't do for knives...
  if (Item[usItemNum].usItemClass != IC_THROWING_KNIFE) {
    // If realtime - set counter to freeup from attacking once done
    if (((gTacticalStatus.uiFlags & REALTIME) || !(gTacticalStatus.uiFlags & INCOMBAT))) {
      // Set delay based on stats, weapon type, etc
      pSoldier->sReloadDelay = (int16_t)(Weapon[usItemNum].usReloadDelay + MANDATORY_WEAPON_DELAY);

      // If a bad guy, double the delay!
      if ((pSoldier->uiStatusFlags & SOLDIER_ENEMY)) {
        pSoldier->sReloadDelay = (pSoldier->sReloadDelay * 2);
      }

      // slow down demo mode!
      if (gTacticalStatus.uiFlags & DEMOMODE) {
        pSoldier->sReloadDelay *= 2;
      }

      // pSoldier->fReloading		= TRUE;
      // RESETTIMECOUNTER( pSoldier->ReloadCounter, pSoldier->sReloadDelay );
    }

    // Deduct AMMO!
    DeductAmmo(pSoldier, pSoldier->ubAttackingHand);

    // ATE: Check if we should say quote...
    if (pSoldier->inv[pSoldier->ubAttackingHand].ubGunShotsLeft == 0 &&
        pSoldier->usAttackingWeapon != ROCKET_LAUNCHER) {
      if (pSoldier->bTeam == gbPlayerNum) {
        pSoldier->fSayAmmoQuotePending = TRUE;
      }
    }

    // NB bDoBurst will be 2 at this point for the first shot since it was incremented
    // above
    if (PTR_OURTEAM && pSoldier->ubTargetID != NOBODY &&
        (!pSoldier->bDoBurst || pSoldier->bDoBurst == 2) && (gTacticalStatus.uiFlags & INCOMBAT) &&
        (SoldierToSoldierBodyPartChanceToGetThrough(pSoldier, MercPtrs[pSoldier->ubTargetID],
                                                    pSoldier->bAimShotLocation) > 0)) {
      if (fGonnaHit) {
        // grant extra exp for hitting a difficult target
        usExpGain += (uint8_t)(100 - uiHitChance) / 25;

        if (pSoldier->bAimTime && !pSoldier->bDoBurst) {
          // gain extra exp for aiming, up to the amount from
          // the difficulty of the shot
          usExpGain += min(pSoldier->bAimTime, usExpGain);
        }

        // base pts extra for hitting
        usExpGain += 3;
      }

      // add base pts for taking a shot, whether it hits or misses
      usExpGain += 3;

      if (IsValidSecondHandShot(pSoldier) && pSoldier->inv[HANDPOS].bGunStatus >= USABLE &&
          pSoldier->inv[HANDPOS].bGunAmmoStatus > 0) {
        // reduce exp gain for two pistol shooting since both shots give xp
        usExpGain = (usExpGain * 2) / 3;
      }

      if (MercPtrs[pSoldier->ubTargetID]->ubBodyType == COW ||
          MercPtrs[pSoldier->ubTargetID]->ubBodyType == CROW) {
        usExpGain /= 2;
      } else if (MercPtrs[pSoldier->ubTargetID]->uiStatusFlags & SOLDIER_VEHICLE ||
                 AM_A_ROBOT(MercPtrs[pSoldier->ubTargetID]) ||
                 TANK(MercPtrs[pSoldier->ubTargetID])) {
        // no exp from shooting a vehicle that you can't damage and can't move!
        usExpGain = 0;
      }

      // MARKSMANSHIP GAIN: gun attack
      StatChange(pSoldier, MARKAMT, usExpGain, (uint8_t)(fGonnaHit ? FALSE : FROM_FAILURE));
    }

    // set buckshot and muzzle flash
    fBuckshot = FALSE;
    if (!CREATURE_OR_BLOODCAT(pSoldier)) {
      pSoldier->fMuzzleFlash = TRUE;
      switch (pSoldier->inv[pSoldier->ubAttackingHand].ubGunAmmoType) {
        case AMMO_BUCKSHOT:
          fBuckshot = TRUE;
          break;
        case AMMO_SLEEP_DART:
          pSoldier->fMuzzleFlash = FALSE;
          break;
        default:
          break;
      }
    }
  } else  //  throwing knife
  {
    fBuckshot = FALSE;
    pSoldier->fMuzzleFlash = FALSE;

    // Deduct knife from inv! (not here, later?)

    // Improve for using a throwing knife....
    if (PTR_OURTEAM && pSoldier->ubTargetID != NOBODY) {
      if (fGonnaHit) {
        // grant extra exp for hitting a difficult target
        usExpGain += (uint8_t)(100 - uiHitChance) / 10;

        if (pSoldier->bAimTime) {
          // gain extra exp for aiming, up to the amount from
          // the difficulty of the throw
          usExpGain += (2 * min(pSoldier->bAimTime, usExpGain));
        }

        // base pts extra for hitting
        usExpGain += 10;
      }

      // add base pts for taking a shot, whether it hits or misses
      usExpGain += 10;

      if (MercPtrs[pSoldier->ubTargetID]->ubBodyType == COW ||
          MercPtrs[pSoldier->ubTargetID]->ubBodyType == CROW) {
        usExpGain /= 2;
      } else if (MercPtrs[pSoldier->ubTargetID]->uiStatusFlags & SOLDIER_VEHICLE ||
                 AM_A_ROBOT(MercPtrs[pSoldier->ubTargetID]) ||
                 TANK(MercPtrs[pSoldier->ubTargetID])) {
        // no exp from shooting a vehicle that you can't damage and can't move!
        usExpGain = 0;
      }

      // MARKSMANSHIP/DEXTERITY GAIN: throwing knife attack
      StatChange(pSoldier, MARKAMT, (uint16_t)(usExpGain / 2),
                 (uint8_t)(fGonnaHit ? FALSE : FROM_FAILURE));
      StatChange(pSoldier, DEXTAMT, (uint16_t)(usExpGain / 2),
                 (uint8_t)(fGonnaHit ? FALSE : FROM_FAILURE));
    }
  }

  if (usItemNum == ROCKET_LAUNCHER) {
    if (WillExplosiveWeaponFail(pSoldier, &(pSoldier->inv[HANDPOS]))) {
      CreateItem(DISCARDED_LAW, pSoldier->inv[HANDPOS].bStatus[0], &(pSoldier->inv[HANDPOS]));
      DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);

      IgniteExplosion(pSoldier->ubID, (int16_t)CenterX(pSoldier->sGridNo),
                      (int16_t)CenterY(pSoldier->sGridNo), 0, pSoldier->sGridNo, C1,
                      pSoldier->bLevel);

      // Reduce again for attack end 'cause it has been incremented for a normal attack
      //
      DebugMsg(
          TOPIC_JA2, DBG_LEVEL_3,
          String("@@@@@@@ Freeing up attacker - ATTACK ANIMATION %s ENDED BY BAD EXPLOSIVE CHECK, "
                 "Now %d",
                 gAnimControl[pSoldier->usAnimState].zAnimStr, gTacticalStatus.ubAttackBusyCount));
      ReduceAttackBusyCount(pSoldier->ubID, FALSE);

      return (FALSE);
    }
  }

  FireBulletGivenTarget(pSoldier, dTargetX, dTargetY, dTargetZ, pSoldier->usAttackingWeapon,
                        (uint16_t)(uiHitChance - uiDiceRoll), fBuckshot, FALSE);

  ubVolume = Weapon[pSoldier->usAttackingWeapon].ubAttackVolume;

  if (Item[usItemNum].usItemClass == IC_THROWING_KNIFE) {
    // Here, remove the knife...	or (for now) rocket launcher
    RemoveObjs(&(pSoldier->inv[HANDPOS]), 1);
    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
  } else if (usItemNum == ROCKET_LAUNCHER) {
    CreateItem(DISCARDED_LAW, pSoldier->inv[HANDPOS].bStatus[0], &(pSoldier->inv[HANDPOS]));
    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);

    // Direction to center of explosion
    ubDirection = gOppositeDirection[pSoldier->bDirection];
    sNewGridNo = NewGridNo((uint16_t)pSoldier->sGridNo, (uint16_t)(1 * DirectionInc(ubDirection)));

    // Check if a person exists here and is not prone....
    ubMerc = WhoIsThere2(sNewGridNo, pSoldier->bLevel);

    if (ubMerc != NOBODY) {
      if (gAnimControl[MercPtrs[ubMerc]->usAnimState].ubHeight != ANIM_PRONE) {
        // Increment attack counter...
        gTacticalStatus.ubAttackBusyCount++;
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("Incrementing Attack: Exaust from LAW", gTacticalStatus.ubAttackBusyCount));

        EVENT_SoldierGotHit(MercPtrs[ubMerc], MINI_GRENADE, 10, 200, pSoldier->bDirection, 0,
                            GetSolID(pSoldier), 0, ANIM_CROUCH, 0, sNewGridNo);
      }
    }
  } else {
    // if the weapon has a silencer attached
    bSilencerPos = FindAttachment(&(pSoldier->inv[HANDPOS]), SILENCER);
    if (bSilencerPos != -1) {
      // reduce volume by a percentage equal to silencer's work %age (min 1)
      ubVolume =
          1 + ((100 - WEAPON_STATUS_MOD(pSoldier->inv[HANDPOS].bAttachStatus[bSilencerPos])) /
               (100 / (ubVolume - 1)));
    }
  }

  MakeNoise(pSoldier->ubID, pSoldier->sGridNo, pSoldier->bLevel, pSoldier->bOverTerrainType,
            ubVolume, NOISE_GUNFIRE);

  if (pSoldier->bDoBurst) {
    // done, if bursting, increment
    pSoldier->bDoBurst++;
  }

  // CJC: since jamming is no longer affected by reliability, increase chance of status going down
  // for really unreliabile guns
  uiDepreciateTest = BASIC_DEPRECIATE_CHANCE + 3 * Item[usItemNum].bReliability;

  if (!PreRandom(uiDepreciateTest) && (pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0] > 1)) {
    pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0]--;
  }

  // reduce monster smell (gunpowder smell)
  if (pSoldier->bMonsterSmell > 0 && Random(2) == 0) {
    pSoldier->bMonsterSmell--;
  }

  return (TRUE);
}

BOOLEAN UseBlade(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  struct SOLDIERTYPE *pTargetSoldier;
  int32_t iHitChance, iDiceRoll;
  int16_t sXMapPos, sYMapPos;
  int16_t sAPCost;
  EV_S_WEAPONHIT SWeaponHit;
  int32_t iImpact, iImpactForCrits;
  BOOLEAN fGonnaHit = FALSE;
  uint16_t usExpGain = 0;
  int8_t bMaxDrop;
  BOOLEAN fSurpriseAttack;

  // Deduct points!
  sAPCost = CalcTotalAPsToAttack(pSoldier, sTargetGridNo, FALSE, pSoldier->bAimTime);

  DeductPoints(pSoldier, sAPCost, 0);

  // GET TARGET XY VALUES
  ConvertGridNoToCenterCellXY(sTargetGridNo, &sXMapPos, &sYMapPos);

  // See if a guy is here!
  pTargetSoldier = SimpleFindSoldier(sTargetGridNo, pSoldier->bTargetLevel);
  if (pTargetSoldier) {
    // set target as noticed attack
    pSoldier->uiStatusFlags |= SOLDIER_ATTACK_NOTICED;
    pTargetSoldier->fIntendedTarget = TRUE;

    // SAVE OPP ID
    pSoldier->ubOppNum = pTargetSoldier->ubID;

    // CHECK IF BUDDY KNOWS ABOUT US
    if (pTargetSoldier->bOppList[pSoldier->ubID] == NOT_HEARD_OR_SEEN ||
        pTargetSoldier->bLife < OKLIFE || pTargetSoldier->bCollapsed) {
      iHitChance = 100;
      fSurpriseAttack = TRUE;
    } else {
      iHitChance = CalcChanceToStab(pSoldier, pTargetSoldier, pSoldier->bAimTime);
      fSurpriseAttack = FALSE;
    }

    // ROLL DICE
    iDiceRoll = (int32_t)PreRandom(100);
    // sprintf( gDebugStr, "Hit Chance: %d %d", (int)uiHitChance, uiDiceRoll );

    if (iDiceRoll <= iHitChance) {
      fGonnaHit = TRUE;

      // CALCULATE DAMAGE!
      // attack HITS, calculate damage (base damage is 1-maximum knife sImpact)
      iImpact = HTHImpact(pSoldier, pTargetSoldier, (iHitChance - iDiceRoll), TRUE);

      // modify this by the knife's condition (if it's dull, not much good)
      iImpact =
          (iImpact * WEAPON_STATUS_MOD(pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0])) / 100;

      // modify by hit location
      AdjustImpactByHitLocation(iImpact, pSoldier->bAimShotLocation, &iImpact, &iImpactForCrits);

      // bonus for surprise
      if (fSurpriseAttack) {
        iImpact = (iImpact * 3) / 2;
      }

      // any successful hit does at LEAST 1 pt minimum damage
      if (iImpact < 1) {
        iImpact = 1;
      }

      if (pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0] > USABLE) {
        bMaxDrop = (iImpact / 20);

        // the duller they get, the slower they get any worse...
        bMaxDrop = min(bMaxDrop, pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0] / 10);

        // as long as its still > USABLE, it drops another point 1/2 the time
        bMaxDrop = max(bMaxDrop, 2);

        pSoldier->inv[pSoldier->ubAttackingHand].bStatus[0] -=
            (int8_t)Random(bMaxDrop);  // 0 to (maxDrop - 1)
      }

      // Send event for getting hit
      memset(&(SWeaponHit), 0, sizeof(SWeaponHit));
      SWeaponHit.usSoldierID = pTargetSoldier->ubID;
      SWeaponHit.uiUniqueId = pTargetSoldier->uiUniqueSoldierIdValue;
      SWeaponHit.usWeaponIndex = pSoldier->usAttackingWeapon;
      SWeaponHit.sDamage = (int16_t)iImpact;
      SWeaponHit.usDirection = GetDirectionFromGridNo(pSoldier->sGridNo, pTargetSoldier);
      SWeaponHit.sXPos = (int16_t)pTargetSoldier->dXPos;
      SWeaponHit.sYPos = (int16_t)pTargetSoldier->dYPos;
      SWeaponHit.sZPos = 20;
      SWeaponHit.sRange = 1;
      SWeaponHit.ubAttackerID = GetSolID(pSoldier);
      SWeaponHit.fHit = TRUE;
      SWeaponHit.ubSpecial = FIRE_WEAPON_NO_SPECIAL;
      AddGameEvent(S_WEAPONHIT, (uint16_t)20, &SWeaponHit);
    } else {
      // if it was another team shooting at someone under our control
      if ((pSoldier->bTeam != Menptr[pTargetSoldier->ubID].bTeam)) {
        if (pTargetSoldier->bTeam == gbPlayerNum) {
          // AGILITY GAIN (10):  Target avoids a knife attack
          StatChange(MercPtrs[pTargetSoldier->ubID], AGILAMT, 10, FALSE);
        }
      }
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("@@@@@@@ Freeing up attacker - missed in knife attack"));
      FreeUpAttacker((uint8_t)pSoldier->ubID);
    }

    if (PTR_OURTEAM && pSoldier->ubTargetID != NOBODY) {
      if (fGonnaHit) {
        // grant extra exp for hitting a difficult target
        usExpGain += (uint8_t)(100 - iHitChance) / 10;

        if (pSoldier->bAimTime) {
          // gain extra exp for aiming, up to the amount from
          // the difficulty of the attack
          usExpGain += (2 * min(pSoldier->bAimTime, usExpGain));
        }

        // base pts extra for hitting
        usExpGain += 10;
      }

      // add base pts for taking a shot, whether it hits or misses
      usExpGain += 10;

      if (MercPtrs[pSoldier->ubTargetID]->ubBodyType == COW ||
          MercPtrs[pSoldier->ubTargetID]->ubBodyType == CROW) {
        usExpGain /= 2;
      } else if (MercPtrs[pSoldier->ubTargetID]->uiStatusFlags & SOLDIER_VEHICLE ||
                 AM_A_ROBOT(MercPtrs[pSoldier->ubTargetID]) ||
                 TANK(MercPtrs[pSoldier->ubTargetID])) {
        // no exp from shooting a vehicle that you can't damage and can't move!
        usExpGain = 0;
      }

      // DEXTERITY GAIN:  Made a knife attack, successful or not
      StatChange(pSoldier, DEXTAMT, usExpGain, (uint8_t)(fGonnaHit ? FALSE : FROM_FAILURE));
    }
  } else {
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("@@@@@@@ Freeing up attacker - missed in knife attack"));
    FreeUpAttacker((uint8_t)pSoldier->ubID);
  }

  // possibly reduce monster smell
  if (pSoldier->bMonsterSmell > 0 && Random(5) == 0) {
    pSoldier->bMonsterSmell--;
  }

  return (TRUE);
}

BOOLEAN UseHandToHand(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo, BOOLEAN fStealing) {
  struct SOLDIERTYPE *pTargetSoldier;
  int32_t iHitChance, iDiceRoll;
  int16_t sXMapPos, sYMapPos;
  int16_t sAPCost;
  EV_S_WEAPONHIT SWeaponHit;
  int32_t iImpact;
  uint16_t usOldItem;
  uint8_t ubExpGain;

  // Deduct points!
  // August 13 2002: unless stealing - APs already deducted elsewhere

  //	if (!fStealing)
  {
    sAPCost = CalcTotalAPsToAttack(pSoldier, sTargetGridNo, FALSE, pSoldier->bAimTime);

    DeductPoints(pSoldier, sAPCost, 0);
  }

  // See if a guy is here!
  pTargetSoldier = SimpleFindSoldier(sTargetGridNo, pSoldier->bTargetLevel);
  if (pTargetSoldier) {
    // set target as noticed attack
    pSoldier->uiStatusFlags |= SOLDIER_ATTACK_NOTICED;
    pTargetSoldier->fIntendedTarget = TRUE;

    // SAVE OPP ID
    pSoldier->ubOppNum = pTargetSoldier->ubID;

    if (fStealing) {
      if (AM_A_ROBOT(pTargetSoldier) || TANK(pTargetSoldier) ||
          CREATURE_OR_BLOODCAT(pTargetSoldier) || TANK(pTargetSoldier)) {
        iHitChance = 0;
      } else if (pTargetSoldier->bOppList[pSoldier->ubID] == NOT_HEARD_OR_SEEN) {
        // give bonus for surprise, but not so much as struggle would still occur
        iHitChance = CalcChanceToSteal(pSoldier, pTargetSoldier, pSoldier->bAimTime) + 20;
      } else if (pTargetSoldier->bLife < OKLIFE || pTargetSoldier->bCollapsed) {
        iHitChance = 100;
      } else {
        iHitChance = CalcChanceToSteal(pSoldier, pTargetSoldier, pSoldier->bAimTime);
      }
    } else {
      if (pTargetSoldier->bOppList[pSoldier->ubID] == NOT_HEARD_OR_SEEN ||
          pTargetSoldier->bLife < OKLIFE || pTargetSoldier->bCollapsed) {
        iHitChance = 100;
      } else {
        iHitChance = CalcChanceToPunch(pSoldier, pTargetSoldier, pSoldier->bAimTime);
      }
    }

    // ROLL DICE
    iDiceRoll = (int32_t)PreRandom(100);
    // sprintf( gDebugStr, "Hit Chance: %d %d", (int)uiHitChance, uiDiceRoll );

#ifdef JA2BETAVERSION
    if (gfReportHitChances) {
      ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Hit chance was %ld, roll %ld", iHitChance,
                iDiceRoll);
    }
#endif

    // GET TARGET XY VALUES
    ConvertGridNoToCenterCellXY(sTargetGridNo, &sXMapPos, &sYMapPos);

    if (fStealing) {
      if (pTargetSoldier->inv[HANDPOS].usItem != NOTHING) {
        if (iDiceRoll <= iHitChance) {
          // Was a good steal!
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_STOLE_SOMETHING],
                    pSoldier->name, ShortItemNames[pTargetSoldier->inv[HANDPOS].usItem]);

          usOldItem = pTargetSoldier->inv[HANDPOS].usItem;

          if (pSoldier->bTeam == gbPlayerNum && pTargetSoldier->bTeam != gbPlayerNum &&
              !(pTargetSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(pTargetSoldier) &&
              !TANK(pTargetSoldier)) {
            // made a steal; give experience
            StatChange(pSoldier, STRAMT, 8, FALSE);
          }

          if (iDiceRoll <= iHitChance * 2 / 3) {
            // Grabbed item
            if (AutoPlaceObject(pSoldier, &(pTargetSoldier->inv[HANDPOS]), TRUE)) {
              // Item transferred; remove it from the target's inventory
              DeleteObj(&(pTargetSoldier->inv[HANDPOS]));
            } else {
              // No room to hold it so the item should drop in our tile again
              AddItemToPool(pSoldier->sGridNo, &(pTargetSoldier->inv[HANDPOS]), 1, pSoldier->bLevel,
                            0, -1);
              DeleteObj(&(pTargetSoldier->inv[HANDPOS]));
            }
          } else {
            if (pSoldier->bTeam == gbPlayerNum) {
              DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
            }

            // Item dropped somewhere... roll based on the same chance to determine where!
            iDiceRoll = (int32_t)PreRandom(100);
            if (iDiceRoll < iHitChance) {
              // Drop item in the our tile
              AddItemToPool(pSoldier->sGridNo, &(pTargetSoldier->inv[HANDPOS]), 1, pSoldier->bLevel,
                            0, -1);
            } else {
              // Drop item in the target's tile
              AddItemToPool(pTargetSoldier->sGridNo, &(pTargetSoldier->inv[HANDPOS]), 1,
                            pSoldier->bLevel, 0, -1);
            }
            DeleteObj(&(pTargetSoldier->inv[HANDPOS]));
          }

          // Reload buddy's animation...
          ReLoadSoldierAnimationDueToHandItemChange(pTargetSoldier, usOldItem, NOTHING);

        } else {
          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_FAILED_TO_STEAL_SOMETHING],
                    pSoldier->name, ShortItemNames[pTargetSoldier->inv[HANDPOS].usItem]);
          if (pSoldier->bTeam == gbPlayerNum) {
            DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);
          }

          if (iHitChance > 0 && pSoldier->bTeam == gbPlayerNum &&
              pTargetSoldier->bTeam != gbPlayerNum &&
              !(pTargetSoldier->uiStatusFlags & SOLDIER_VEHICLE) && !AM_A_ROBOT(pTargetSoldier) &&
              !TANK(pTargetSoldier)) {
            // failed a steal; give some experience
            StatChange(pSoldier, STRAMT, 4, FROM_FAILURE);
          }
        }
      }

#ifdef JA2BETAVERSION
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - steal"));
#endif
      FreeUpAttacker((uint8_t)pSoldier->ubID);

    } else {
      // ATE/CC: if doing ninja spin kick (only), automatically make it a hit
      if (pSoldier->usAnimState == NINJA_SPINKICK) {
        // Let him to succeed by a random amount
        iDiceRoll = PreRandom(iHitChance);
      }

      if (pSoldier->bTeam == gbPlayerNum && pTargetSoldier->bTeam != gbPlayerNum) {
        // made an HTH attack; give experience
        if (iDiceRoll <= iHitChance) {
          ubExpGain = 8;

          if (pTargetSoldier->uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(pTargetSoldier) ||
              TANK(pTargetSoldier)) {
            ubExpGain = 0;
          } else if (pTargetSoldier->ubBodyType == COW || pTargetSoldier->ubBodyType == CROW) {
            ubExpGain /= 2;
          }

          StatChange(pSoldier, STRAMT, ubExpGain, FALSE);
          StatChange(pSoldier, DEXTAMT, ubExpGain, FALSE);
        } else {
          ubExpGain = 4;

          if (pTargetSoldier->uiStatusFlags & SOLDIER_VEHICLE || AM_A_ROBOT(pTargetSoldier) ||
              TANK(pTargetSoldier)) {
            ubExpGain = 0;
          } else if (pTargetSoldier->ubBodyType == COW || pTargetSoldier->ubBodyType == CROW) {
            ubExpGain /= 2;
          }

          StatChange(pSoldier, STRAMT, ubExpGain, FROM_FAILURE);
          StatChange(pSoldier, DEXTAMT, ubExpGain, FROM_FAILURE);
        }
      } else if (pSoldier->bTeam != gbPlayerNum && pTargetSoldier->bTeam == gbPlayerNum) {
        // being attacked... if successfully dodged, give experience
        if (iDiceRoll > iHitChance) {
          StatChange(pTargetSoldier, AGILAMT, 8, FALSE);
        }
      }

      if (iDiceRoll <= iHitChance || AreInMeanwhile()) {
        // CALCULATE DAMAGE!
        iImpact = HTHImpact(pSoldier, pTargetSoldier, (iHitChance - iDiceRoll), FALSE);

        // Send event for getting hit
        memset(&(SWeaponHit), 0, sizeof(SWeaponHit));
        SWeaponHit.usSoldierID = pTargetSoldier->ubID;
        SWeaponHit.usWeaponIndex = pSoldier->usAttackingWeapon;
        SWeaponHit.sDamage = (int16_t)iImpact;
        SWeaponHit.usDirection = GetDirectionFromGridNo(pSoldier->sGridNo, pTargetSoldier);
        SWeaponHit.sXPos = (int16_t)pTargetSoldier->dXPos;
        SWeaponHit.sYPos = (int16_t)pTargetSoldier->dYPos;
        SWeaponHit.sZPos = 20;
        SWeaponHit.sRange = 1;
        SWeaponHit.ubAttackerID = GetSolID(pSoldier);
        SWeaponHit.fHit = TRUE;
        SWeaponHit.ubSpecial = FIRE_WEAPON_NO_SPECIAL;
        AddGameEvent(S_WEAPONHIT, (uint16_t)20, &SWeaponHit);
      } else {
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("@@@@@@@ Freeing up attacker - missed in HTH attack"));
        FreeUpAttacker((uint8_t)pSoldier->ubID);
      }
    }
  }

  // possibly reduce monster smell (gunpowder smell)
  if (pSoldier->bMonsterSmell > 0 && Random(5) == 0) {
    pSoldier->bMonsterSmell--;
  }

  return (TRUE);
}

BOOLEAN UseThrown(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  uint32_t uiHitChance, uiDiceRoll;
  int8_t bLoop;
  uint8_t ubTargetID;
  struct SOLDIERTYPE *pTargetSoldier;

  uiHitChance = CalcThrownChanceToHit(pSoldier, sTargetGridNo, pSoldier->bAimTime, AIM_SHOT_TORSO);

  uiDiceRoll = PreRandom(100);

#ifdef JA2BETAVERSION
  if (gfReportHitChances) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Hit chance was %ld, roll %ld (range %d)",
              uiHitChance, uiDiceRoll, PythSpacesAway(pSoldier->sGridNo, sTargetGridNo));
  }
#endif

  if (pSoldier->bTeam == gbPlayerNum && gTacticalStatus.uiFlags & INCOMBAT) {
    // check target gridno
    ubTargetID = WhoIsThere2(pSoldier->sTargetGridNo, pSoldier->bTargetLevel);
    if (ubTargetID == NOBODY) {
      pTargetSoldier = NULL;
    } else {
      pTargetSoldier = MercPtrs[ubTargetID];
    }

    if (pTargetSoldier && pTargetSoldier->bTeam == pSoldier->bTeam) {
      // ignore!
      pTargetSoldier = NULL;
    }

    if (pTargetSoldier == NULL) {
      // search for an opponent near the target gridno
      for (bLoop = 0; bLoop < NUM_WORLD_DIRECTIONS; bLoop++) {
        ubTargetID = WhoIsThere2(NewGridNo(pSoldier->sTargetGridNo, DirectionInc(bLoop)),
                                 pSoldier->bTargetLevel);
        pTargetSoldier = NULL;
        if (ubTargetID != NOBODY) {
          pTargetSoldier = MercPtrs[ubTargetID];
          if (pTargetSoldier->bTeam != pSoldier->bTeam) {
            break;
          }
        }
      }
    }

    if (pTargetSoldier) {
      // ok this is a real attack on someone, grant experience
      StatChange(pSoldier, STRAMT, 5, FALSE);
      if (uiDiceRoll < uiHitChance) {
        StatChange(pSoldier, DEXTAMT, 5, FALSE);
        StatChange(pSoldier, MARKAMT, 5, FALSE);
      } else {
        StatChange(pSoldier, DEXTAMT, 2, FROM_FAILURE);
        StatChange(pSoldier, MARKAMT, 2, FROM_FAILURE);
      }
    }
  }

  CalculateLaunchItemParamsForThrow(
      pSoldier, sTargetGridNo, pSoldier->bTargetLevel, (int16_t)(pSoldier->bTargetLevel * 256),
      &(pSoldier->inv[HANDPOS]), (int8_t)(uiDiceRoll - uiHitChance), THROW_ARM_ITEM, 0);

  // OK, goto throw animation
  HandleSoldierThrowItem(pSoldier, pSoldier->sTargetGridNo);

  RemoveObjs(&(pSoldier->inv[HANDPOS]), 1);

  return (TRUE);
}

BOOLEAN UseLauncher(struct SOLDIERTYPE *pSoldier, int16_t sTargetGridNo) {
  uint32_t uiHitChance, uiDiceRoll;
  int16_t sAPCost = 0;
  int8_t bAttachPos;
  struct OBJECTTYPE Launchable;
  struct OBJECTTYPE *pObj;
  uint16_t usItemNum;

  usItemNum = pSoldier->usAttackingWeapon;

  if (!EnoughAmmo(pSoldier, TRUE, pSoldier->ubAttackingHand)) {
    return (FALSE);
  }

  pObj = &(pSoldier->inv[HANDPOS]);
  for (bAttachPos = 0; bAttachPos < MAX_ATTACHMENTS; bAttachPos++) {
    if (pObj->usAttachItem[bAttachPos] != NOTHING) {
      if (Item[pObj->usAttachItem[bAttachPos]].usItemClass & IC_EXPLOSV) {
        break;
      }
    }
  }
  if (bAttachPos == MAX_ATTACHMENTS) {
    // this should not happen!!
    return (FALSE);
  }

  CreateItem(pObj->usAttachItem[bAttachPos], pObj->bAttachStatus[bAttachPos], &Launchable);

  if (pSoldier->usAttackingWeapon == pObj->usItem) {
    DeductAmmo(pSoldier, HANDPOS);
  } else {
    // Firing an attached grenade launcher... the attachment we found above
    // is the one to remove!
    RemoveAttachment(pObj, bAttachPos, NULL);
  }

  // ATE: Check here if the launcher should fail 'cause of bad status.....
  if (WillExplosiveWeaponFail(pSoldier, pObj)) {
    // Explode dude!

    // So we still should have ABC > 0
    // Begin explosion due to failure...
    IgniteExplosion(pSoldier->ubID, (int16_t)CenterX(pSoldier->sGridNo),
                    (int16_t)CenterY(pSoldier->sGridNo), 0, pSoldier->sGridNo, Launchable.usItem,
                    pSoldier->bLevel);

    // Reduce again for attack end 'cause it has been incremented for a normal attack
    //
    DebugMsg(
        TOPIC_JA2, DBG_LEVEL_3,
        String("@@@@@@@ Freeing up attacker - ATTACK ANIMATION %s ENDED BY BAD EXPLOSIVE CHECK, "
               "Now %d",
               gAnimControl[pSoldier->usAnimState].zAnimStr, gTacticalStatus.ubAttackBusyCount));
    ReduceAttackBusyCount(pSoldier->ubID, FALSE);

    // So all's well, should be good from here....
    return (FALSE);
  }

  if (Weapon[usItemNum].sSound != NO_WEAPON_SOUND) {
    PlayJA2Sample(Weapon[usItemNum].sSound, RATE_11025, SoundVolume(HIGHVOLUME, pSoldier->sGridNo),
                  1, SoundDir(pSoldier->sGridNo));
  }

  uiHitChance = CalcThrownChanceToHit(pSoldier, sTargetGridNo, pSoldier->bAimTime, AIM_SHOT_TORSO);

  uiDiceRoll = PreRandom(100);

#ifdef JA2BETAVERSION
  if (gfReportHitChances) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"Hit chance was %ld, roll %ld (range %d)",
              uiHitChance, uiDiceRoll, PythSpacesAway(pSoldier->sGridNo, sTargetGridNo));
  }
#endif

  if (Item[usItemNum].usItemClass == IC_LAUNCHER) {
    // Preserve gridno!
    // pSoldier->sLastTarget = sTargetGridNo;

    sAPCost = MinAPsToAttack(pSoldier, sTargetGridNo, TRUE);
  } else {
    // Throw....
    sAPCost = MinAPsToThrow(pSoldier, sTargetGridNo, FALSE);
  }

  DeductPoints(pSoldier, sAPCost, 0);

  CalculateLaunchItemParamsForThrow(pSoldier, pSoldier->sTargetGridNo, pSoldier->bTargetLevel, 0,
                                    &Launchable, (int8_t)(uiDiceRoll - uiHitChance), THROW_ARM_ITEM,
                                    0);

  CreatePhysicalObject(
      pSoldier->pTempObject, pSoldier->pThrowParams->dLifeSpan, pSoldier->pThrowParams->dX,
      pSoldier->pThrowParams->dY, pSoldier->pThrowParams->dZ, pSoldier->pThrowParams->dForceX,
      pSoldier->pThrowParams->dForceY, pSoldier->pThrowParams->dForceZ, GetSolID(pSoldier),
      pSoldier->pThrowParams->ubActionCode, pSoldier->pThrowParams->uiActionData);

  MemFree(pSoldier->pTempObject);
  pSoldier->pTempObject = NULL;

  MemFree(pSoldier->pThrowParams);
  pSoldier->pThrowParams = NULL;

  return (TRUE);
}

BOOLEAN DoSpecialEffectAmmoMiss(uint8_t ubAttackerID, int16_t sGridNo, int16_t sXPos, int16_t sYPos,
                                int16_t sZPos, BOOLEAN fSoundOnly, BOOLEAN fFreeupAttacker,
                                int32_t iBullet) {
  ANITILE_PARAMS AniParams;
  uint8_t ubAmmoType;
  uint16_t usItem;

  ubAmmoType = MercPtrs[ubAttackerID]->inv[MercPtrs[ubAttackerID]->ubAttackingHand].ubGunAmmoType;
  usItem = MercPtrs[ubAttackerID]->inv[MercPtrs[ubAttackerID]->ubAttackingHand].usItem;

  memset(&AniParams, 0, sizeof(ANITILE_PARAMS));

  if (ubAmmoType == AMMO_HE || ubAmmoType == AMMO_HEAT) {
    if (!fSoundOnly) {
      AniParams.sGridNo = sGridNo;
      AniParams.ubLevelID = ANI_TOPMOST_LEVEL;
      AniParams.sDelay = (int16_t)(100);
      AniParams.sStartFrame = 0;
      AniParams.uiFlags = ANITILE_CACHEDTILE | ANITILE_FORWARD | ANITILE_ALWAYS_TRANSLUCENT;
      AniParams.sX = sXPos;
      AniParams.sY = sYPos;
      AniParams.sZ = sZPos;

      strcpy(AniParams.zCachedFile, "TILECACHE\\MINIBOOM.STI");

      CreateAnimationTile(&AniParams);

      if (fFreeupAttacker) {
        RemoveBullet(iBullet);
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("@@@@@@@ Freeing up attacker - bullet hit structure - explosive ammo"));
        FreeUpAttacker((uint8_t)ubAttackerID);
      }
    }

    if (sGridNo != NOWHERE) {
      PlayJA2Sample(SMALL_EXPLODE_1, RATE_11025, SoundVolume((int8_t)HIGHVOLUME, sGridNo), 1,
                    SoundDir(sGridNo));
    } else {
      PlayJA2Sample(SMALL_EXPLODE_1, RATE_11025, MIDVOLUME, 1, MIDDLE);
    }

    return (TRUE);
  } else if (usItem == CREATURE_OLD_MALE_SPIT || usItem == CREATURE_QUEEN_SPIT ||
             usItem == CREATURE_INFANT_SPIT || usItem == CREATURE_YOUNG_MALE_SPIT) {
    // Increment attack busy...
    // gTacticalStatus.ubAttackBusyCount++;
    // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Incrementing Attack: Explosion gone off, COunt now
    // %d", gTacticalStatus.ubAttackBusyCount ) );

    PlayJA2Sample(CREATURE_GAS_NOISE, RATE_11025, SoundVolume(HIGHVOLUME, sGridNo), 1,
                  SoundDir(sGridNo));

    // Do Spread effect.......
    switch (usItem) {
      case CREATURE_YOUNG_MALE_SPIT:
      case CREATURE_INFANT_SPIT:

        NewSmokeEffect(sGridNo, VERY_SMALL_CREATURE_GAS, 0, ubAttackerID);
        break;

      case CREATURE_OLD_MALE_SPIT:
        NewSmokeEffect(sGridNo, SMALL_CREATURE_GAS, 0, ubAttackerID);
        break;

      case CREATURE_QUEEN_SPIT:
        NewSmokeEffect(sGridNo, LARGE_CREATURE_GAS, 0, ubAttackerID);
        break;
    }
  }

  return (FALSE);
}

void WeaponHit(uint16_t usSoldierID, uint16_t usWeaponIndex, int16_t sDamage, int16_t sBreathLoss,
               uint16_t usDirection, int16_t sXPos, int16_t sYPos, int16_t sZPos, int16_t sRange,
               uint8_t ubAttackerID, BOOLEAN fHit, uint8_t ubSpecial, uint8_t ubHitLocation) {
  struct SOLDIERTYPE *pTargetSoldier;

  // Get Target
  pTargetSoldier = MercPtrs[usSoldierID];

  MakeNoise(ubAttackerID, pTargetSoldier->sGridNo, pTargetSoldier->bLevel,
            gpWorldLevelData[pTargetSoldier->sGridNo].ubTerrainID,
            Weapon[usWeaponIndex].ubHitVolume, NOISE_BULLET_IMPACT);

  if (EXPLOSIVE_GUN(usWeaponIndex)) {
    // Reduce attacker count!
    if (usWeaponIndex == ROCKET_LAUNCHER) {
      IgniteExplosion(ubAttackerID, sXPos, sYPos, 0,
                      (int16_t)(GETWORLDINDEXFROMWORLDCOORDS(sYPos, sXPos)), C1,
                      pTargetSoldier->bLevel);
    } else  // tank cannon
    {
      IgniteExplosion(ubAttackerID, sXPos, sYPos, 0,
                      (int16_t)(GETWORLDINDEXFROMWORLDCOORDS(sYPos, sXPos)), TANK_SHELL,
                      pTargetSoldier->bLevel);
    }

    DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - end of LAW fire"));
    FreeUpAttacker(ubAttackerID);
    return;
  }

  DoSpecialEffectAmmoMiss(ubAttackerID, pTargetSoldier->sGridNo, sXPos, sYPos, sZPos, FALSE, FALSE,
                          0);

  // OK, SHOT HAS HIT, DO THINGS APPROPRIATELY
  // ATE: This is 'cause of that darn smoke effect that could potnetially kill
  // the poor bastard .. so check
  if (!pTargetSoldier->fDoingExternalDeath) {
    EVENT_SoldierGotHit(pTargetSoldier, usWeaponIndex, sDamage, sBreathLoss, usDirection, sRange,
                        ubAttackerID, ubSpecial, ubHitLocation, 0, NOWHERE);
  } else {
    // Buddy had died from additional dammage - free up attacker here...
    ReduceAttackBusyCount(pTargetSoldier->ubAttackerID, FALSE);
    DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
             String("Special effect killed before bullet impact, attack count now %d",
                    gTacticalStatus.ubAttackBusyCount));
  }
}

void StructureHit(int32_t iBullet, uint16_t usWeaponIndex, int8_t bWeaponStatus,
                  uint8_t ubAttackerID, uint16_t sXPos, int16_t sYPos, int16_t sZPos,
                  uint16_t usStructureID, int32_t iImpact, BOOLEAN fStopped) {
  BOOLEAN fDoMissForGun = FALSE;
  ANITILE *pNode;
  int16_t sGridNo;
  ANITILE_PARAMS AniParams;
  uint16_t usMissTileIndex, usMissTileType;
  struct STRUCTURE *pStructure = NULL;
  uint32_t uiMissVolume = MIDVOLUME;
  BOOLEAN fHitSameStructureAsBefore;
  BULLET *pBullet;
  struct SOLDIERTYPE *pAttacker;

  pBullet = GetBulletPtr(iBullet);

  if (fStopped && ubAttackerID != NOBODY) {
    pAttacker = MercPtrs[ubAttackerID];

    if (pAttacker->ubOppNum != NOBODY) {
      // if it was another team shooting at someone under our control
      if ((pAttacker->bTeam != Menptr[pAttacker->ubOppNum].bTeam)) {
        // if OPPONENT is under our control
        if (Menptr[pAttacker->ubOppNum].bTeam == gbPlayerNum) {
          // AGILITY GAIN: Opponent "dodged" a bullet shot at him (it missed)
          StatChange(MercPtrs[pAttacker->ubOppNum], AGILAMT, 5, FROM_FAILURE);
        }
      }
    }
  }

  if (pBullet) {
    fHitSameStructureAsBefore = (usStructureID == pBullet->usLastStructureHit);
  } else {
    // WTF?
    fHitSameStructureAsBefore = FALSE;
  }

  sGridNo = MAPROWCOLTOPOS((sYPos / CELL_Y_SIZE), (sXPos / CELL_X_SIZE));
  if (!fHitSameStructureAsBefore) {
    if (sZPos > WALL_HEIGHT) {
      MakeNoise(ubAttackerID, sGridNo, 1, gpWorldLevelData[sGridNo].ubTerrainID,
                Weapon[usWeaponIndex].ubHitVolume, NOISE_BULLET_IMPACT);
    } else {
      MakeNoise(ubAttackerID, sGridNo, 0, gpWorldLevelData[sGridNo].ubTerrainID,
                Weapon[usWeaponIndex].ubHitVolume, NOISE_BULLET_IMPACT);
    }
  }

  if (fStopped) {
    if (usWeaponIndex == ROCKET_LAUNCHER) {
      RemoveBullet(iBullet);

      // Reduce attacker count!
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - end of LAW fire"));
      FreeUpAttacker(ubAttackerID);

      IgniteExplosion(ubAttackerID, (int16_t)CenterX(sGridNo), (int16_t)CenterY(sGridNo), 0,
                      sGridNo, C1, (int8_t)(sZPos >= WALL_HEIGHT));
      // FreeUpAttacker( (uint8_t) ubAttackerID );

      return;
    }

    if (usWeaponIndex == TANK_CANNON) {
      RemoveBullet(iBullet);

      // Reduce attacker count!
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - end of TANK fire"));
      FreeUpAttacker(ubAttackerID);

      IgniteExplosion(ubAttackerID, (int16_t)CenterX(sGridNo), (int16_t)CenterY(sGridNo), 0,
                      sGridNo, TANK_SHELL, (int8_t)(sZPos >= WALL_HEIGHT));
      // FreeUpAttacker( (uint8_t) ubAttackerID );

      return;
    }
  }

  // Get Structure pointer and damage it!
  if (usStructureID != INVALID_STRUCTURE_ID) {
    pStructure = FindStructureByID(sGridNo, usStructureID);

    DamageStructure(pStructure, (uint8_t)iImpact, STRUCTURE_DAMAGE_GUNFIRE, sGridNo, sXPos, sYPos,
                    ubAttackerID);
  }

  switch (Weapon[usWeaponIndex].ubWeaponClass) {
    case HANDGUNCLASS:
    case RIFLECLASS:
    case SHOTGUNCLASS:
    case SMGCLASS:
    case MGCLASS:

      // Guy has missed, play random sound
      if (MercPtrs[ubAttackerID]->bTeam == gbPlayerNum) {
        if (!MercPtrs[ubAttackerID]->bDoBurst) {
          if (Random(40) == 0) {
            DoMercBattleSound(MercPtrs[ubAttackerID], BATTLE_SOUND_CURSE1);
          }
        }
      }
      // fDoMissForGun = TRUE;
      // break;
      fDoMissForGun = TRUE;
      break;

    case MONSTERCLASS:

      DoSpecialEffectAmmoMiss(ubAttackerID, sGridNo, sXPos, sYPos, sZPos, FALSE, TRUE, iBullet);

      RemoveBullet(iBullet);
      DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
               String("@@@@@@@ Freeing up attacker - monster attack hit structure"));
      FreeUpAttacker((uint8_t)ubAttackerID);

      // PlayJA2Sample( SPIT_RICOCHET , RATE_11025, uiMissVolume, 1, SoundDir( sGridNo ) );
      break;

    case KNIFECLASS:

      // When it hits the ground, leave on map...
      if (Item[usWeaponIndex].usItemClass == IC_THROWING_KNIFE) {
        struct OBJECTTYPE Object;

        // OK, have we hit ground?
        if (usStructureID == INVALID_STRUCTURE_ID) {
          // Add item
          CreateItem(THROWING_KNIFE, bWeaponStatus, &Object);

          AddItemToPool(sGridNo, &Object, -1, 0, 0, -1);

          // Make team look for items
          NotifySoldiersToLookforItems();
        }

        if (!fHitSameStructureAsBefore) {
          PlayJA2Sample(MISS_KNIFE, RATE_11025, uiMissVolume, 1, SoundDir(sGridNo));
        }

        RemoveBullet(iBullet);
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("@@@@@@@ Freeing up attacker - knife attack hit structure"));
        FreeUpAttacker((uint8_t)ubAttackerID);
      }
  }

  if (fDoMissForGun) {
    // OK, are we a shotgun, if so , make sounds lower...
    if (Weapon[usWeaponIndex].ubWeaponClass == SHOTGUNCLASS) {
      uiMissVolume = LOWVOLUME;
    }

    // Free guy!
    // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - bullet hit
    // structure") ); FreeUpAttacker( (uint8_t) ubAttackerID );

    // PLAY SOUND AND FLING DEBRIS
    // RANDOMIZE SOUND SYSTEM

    // IF WE HIT THE GROUND

    if (fHitSameStructureAsBefore) {
      if (fStopped) {
        RemoveBullet(iBullet);
        DebugMsg(TOPIC_JA2, DBG_LEVEL_3,
                 String("@@@@@@@ Freeing up attacker - bullet hit same structure twice"));
        FreeUpAttacker((uint8_t)ubAttackerID);
      }
    } else {
      if (!fStopped || !DoSpecialEffectAmmoMiss(ubAttackerID, sGridNo, sXPos, sYPos, sZPos, FALSE,
                                                TRUE, iBullet)) {
        if (sZPos == 0) {
          PlayJA2Sample(MISS_G2, RATE_11025, uiMissVolume, 1, SoundDir(sGridNo));
        } else {
          PlayJA2Sample(MISS_1 + Random(8), RATE_11025, uiMissVolume, 1, SoundDir(sGridNo));
        }

        // Default hit is the ground
        usMissTileIndex = FIRSTMISS1;
        usMissTileType = FIRSTMISS;

        // Check if we are in water...
        if (gpWorldLevelData[sGridNo].ubTerrainID == LOW_WATER ||
            gpWorldLevelData[sGridNo].ubTerrainID == DEEP_WATER) {
          usMissTileIndex = SECONDMISS1;
          usMissTileType = SECONDMISS;

          // Add ripple
          memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
          AniParams.sGridNo = sGridNo;
          AniParams.ubLevelID = ANI_STRUCT_LEVEL;
          AniParams.usTileType = THIRDMISS;
          AniParams.usTileIndex = THIRDMISS1;
          AniParams.sDelay = 50;
          AniParams.sStartFrame = 0;
          AniParams.uiFlags = ANITILE_FORWARD;

          pNode = CreateAnimationTile(&AniParams);

          // Adjust for absolute positioning
          pNode->pLevelNode->uiFlags |= LEVELNODE_USEABSOLUTEPOS;
          pNode->pLevelNode->sRelativeX = sXPos;
          pNode->pLevelNode->sRelativeY = sYPos;
          pNode->pLevelNode->sRelativeZ = sZPos;
        }

        memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
        AniParams.sGridNo = sGridNo;
        AniParams.ubLevelID = ANI_STRUCT_LEVEL;
        AniParams.usTileType = usMissTileType;
        AniParams.usTileIndex = usMissTileIndex;
        AniParams.sDelay = 80;
        AniParams.sStartFrame = 0;
        if (fStopped) {
          AniParams.uiFlags = ANITILE_FORWARD | ANITILE_RELEASE_ATTACKER_WHEN_DONE;
        } else {
          AniParams.uiFlags = ANITILE_FORWARD;
        }
        // Save bullet ID!
        AniParams.uiUserData3 = iBullet;

        pNode = CreateAnimationTile(&AniParams);

        // Set attacker ID
        pNode->usMissAnimationPlayed = usMissTileType;
        pNode->ubAttackerMissed = ubAttackerID;
        // Adjust for absolute positioning
        pNode->pLevelNode->uiFlags |= LEVELNODE_USEABSOLUTEPOS;
        pNode->pLevelNode->sRelativeX = sXPos;
        pNode->pLevelNode->sRelativeY = sYPos;
        pNode->pLevelNode->sRelativeZ = sZPos;

        // ATE: Show misses...( if our team )
        if (gGameSettings.fOptions[TOPTION_SHOW_MISSES]) {
          if (ubAttackerID != NOBODY) {
            if (MercPtrs[ubAttackerID]->bTeam == gbPlayerNum) {
              LocateGridNo(sGridNo);
            }
          }
        }
      }

      pBullet->usLastStructureHit = usStructureID;
    }
  }
}

void WindowHit(int16_t sGridNo, uint16_t usStructureID, BOOLEAN fBlowWindowSouth,
               BOOLEAN fLargeForce) {
  struct STRUCTURE *pWallAndWindow;
  struct DB_STRUCTURE *pWallAndWindowInDB;
  int16_t sShatterGridNo;
  uint16_t usTileIndex;
  ANITILE_PARAMS AniParams;

  // ATE: Make large force always for now ( feel thing )
  fLargeForce = TRUE;

  // we have to do two things here: swap the window structure
  // (right now just using the partner stuff in a chain from
  // intact to cracked to shattered) and display the
  // animation if we've reached shattered

  // find the wall structure, and go one length along the chain
  pWallAndWindow = FindStructureByID(sGridNo, usStructureID);
  if (pWallAndWindow == NULL) {
    return;
  }

  pWallAndWindow = SwapStructureForPartner(sGridNo, pWallAndWindow);
  if (pWallAndWindow == NULL) {
    return;
  }

  // record window smash
  AddWindowHitToMapTempFile(sGridNo);

  pWallAndWindowInDB = pWallAndWindow->pDBStructureRef->pDBStructure;

  if (fLargeForce) {
    // Force to destruction animation!
    if (pWallAndWindowInDB->bPartnerDelta != NO_PARTNER_STRUCTURE) {
      pWallAndWindow = SwapStructureForPartner(sGridNo, pWallAndWindow);
      if (pWallAndWindow) {
        // record 2nd window smash
        AddWindowHitToMapTempFile(sGridNo);

        pWallAndWindowInDB = pWallAndWindow->pDBStructureRef->pDBStructure;
      }
    }
  }

  SetRenderFlags(RENDER_FLAG_FULL);

  if (pWallAndWindowInDB->ubArmour == MATERIAL_THICKER_METAL_WITH_SCREEN_WINDOWS) {
    // don't play any sort of animation or sound
    return;
  }

  if (pWallAndWindowInDB->bPartnerDelta !=
      NO_PARTNER_STRUCTURE) {  // just cracked; don't display the animation
    MakeNoise(NOBODY, sGridNo, 0, gpWorldLevelData[sGridNo].ubTerrainID, WINDOW_CRACK_VOLUME,
              NOISE_BULLET_IMPACT);
    return;
  }
  MakeNoise(NOBODY, sGridNo, 0, gpWorldLevelData[sGridNo].ubTerrainID, WINDOW_SMASH_VOLUME,
            NOISE_BULLET_IMPACT);
  if (pWallAndWindowInDB->ubWallOrientation == INSIDE_TOP_RIGHT ||
      pWallAndWindowInDB->ubWallOrientation == OUTSIDE_TOP_RIGHT) {
    /*
            sShatterGridNo = sGridNo + 1;
            // check for wrapping around edge of map
            if (sShatterGridNo % WORLD_COLS == 0)
            {
                    // in which case we don't play the animation!
                    return;
            }*/
    if (fBlowWindowSouth) {
      usTileIndex = WINDOWSHATTER1;
      sShatterGridNo = sGridNo + 1;
    } else {
      usTileIndex = WINDOWSHATTER11;
      sShatterGridNo = sGridNo;
    }

  } else {
    /*
            sShatterGridNo = sGridNo + WORLD_COLS;
            // check for wrapping around edge of map
            if (sShatterGridNo % WORLD_ROWS == 0)
            {
                    // in which case we don't play the animation!
                    return;
            }*/
    if (fBlowWindowSouth) {
      usTileIndex = WINDOWSHATTER6;
      sShatterGridNo = sGridNo + WORLD_COLS;
    } else {
      usTileIndex = WINDOWSHATTER16;
      sShatterGridNo = sGridNo;
    }
  }

  memset(&AniParams, 0, sizeof(ANITILE_PARAMS));
  AniParams.sGridNo = sShatterGridNo;
  AniParams.ubLevelID = ANI_STRUCT_LEVEL;
  AniParams.usTileType = WINDOWSHATTER;
  AniParams.usTileIndex = usTileIndex;
  AniParams.sDelay = 50;
  AniParams.sStartFrame = 0;
  AniParams.uiFlags = ANITILE_FORWARD;

  CreateAnimationTile(&AniParams);

  PlayJA2Sample(GLASS_SHATTER1 + Random(2), RATE_11025, MIDVOLUME, 1, SoundDir(sGridNo));
}

BOOLEAN InRange(struct SOLDIERTYPE *pSoldier, int16_t sGridNo) {
  int16_t sRange;
  uint16_t usInHand;

  usInHand = pSoldier->inv[HANDPOS].usItem;

  if (Item[usInHand].usItemClass == IC_GUN || Item[usInHand].usItemClass == IC_THROWING_KNIFE) {
    // Determine range
    sRange = (int16_t)GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo);

    if (Item[usInHand].usItemClass == IC_THROWING_KNIFE) {
      // NB CalcMaxTossRange returns range in tiles, not in world units
      if (sRange <= CalcMaxTossRange(pSoldier, THROWING_KNIFE, TRUE) * CELL_X_SIZE) {
        return (TRUE);
      }
    } else {
      // For given weapon, check range
      if (sRange <= GunRange(&(pSoldier->inv[HANDPOS]))) {
        return (TRUE);
      }
    }
  }
  return (FALSE);
}

uint32_t CalcChanceToHitGun(struct SOLDIERTYPE *pSoldier, uint16_t sGridNo, uint8_t ubAimTime,
                            uint8_t ubAimPos) {
  // struct SOLDIERTYPE *vicpSoldier;
  struct SOLDIERTYPE *pTarget;
  int32_t iChance, iRange, iSightRange, iMaxRange, iScopeBonus, iBonus;  //, minRange;
  int32_t iGunCondition, iMarksmanship;
  int32_t iPenalty;
  uint16_t usInHand;
  struct OBJECTTYPE *pInHand;
  int8_t bAttachPos;
  int8_t bBandaged;
  int16_t sDistVis;
  uint8_t ubAdjAimPos;
  uint8_t ubTargetID;

  if (pSoldier->bMarksmanship == 0) {
    // always min chance
    return (MINCHANCETOHIT);
  }

  // make sure the guy's actually got a weapon in his hand!
  pInHand = &(pSoldier->inv[pSoldier->ubAttackingHand]);
  usInHand = pSoldier->usAttackingWeapon;

  // DETERMINE BASE CHANCE OF HITTING
  iGunCondition = WEAPON_STATUS_MOD(pInHand->bGunStatus);

  if (usInHand == ROCKET_LAUNCHER) {
    // use the same calculation as for mechanical thrown weapons
    iMarksmanship = (EffectiveDexterity(pSoldier) + EffectiveMarksmanship(pSoldier) +
                     EffectiveWisdom(pSoldier) + (10 * EffectiveExpLevel(pSoldier))) /
                    4;
    // heavy weapons trait helps out
    if (HAS_SKILL_TRAIT(pSoldier, HEAVY_WEAPS)) {
      iMarksmanship += gbSkillTraitBonus[HEAVY_WEAPS] * NUM_SKILL_TRAITS(pSoldier, HEAVY_WEAPS);
    }
  } else {
    iMarksmanship = EffectiveMarksmanship(pSoldier);

    if (AM_A_ROBOT(pSoldier)) {
      struct SOLDIERTYPE *pSoldier2;

      pSoldier2 = GetRobotController(pSoldier);
      if (pSoldier2) {
        iMarksmanship = max(iMarksmanship, EffectiveMarksmanship(pSoldier2));
      }
    }
  }

  // modify chance to hit by morale
  iMarksmanship += GetMoraleModifier(pSoldier);

  // penalize marksmanship for fatigue
  iMarksmanship -= GetSkillCheckPenaltyForFatigue(pSoldier, iMarksmanship);

  if (iGunCondition >= iMarksmanship)
    // base chance is equal to the shooter's marksmanship skill
    iChance = iMarksmanship;
  else
    // base chance is equal to the average of marksmanship & gun's condition!
    iChance = (iMarksmanship + iGunCondition) / 2;

  // if shooting same target as the last shot
  if (sGridNo == pSoldier->sLastTarget) iChance += AIM_BONUS_SAME_TARGET;  // give a bonus to hit

  if (GetSolProfile(pSoldier) != NO_PROFILE &&
      gMercProfiles[GetSolProfile(pSoldier)].bPersonalityTrait == PSYCHO) {
    iChance += AIM_BONUS_PSYCHO;
  }

  // calculate actual range (in units, 10 units = 1 tile)
  iRange = GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo);

  // if shooter is crouched, he aims slightly better (to max of AIM_BONUS_CROUCHING)
  if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_CROUCH) {
    iBonus = iRange / 10;
    if (iBonus > AIM_BONUS_CROUCHING) {
      iBonus = AIM_BONUS_CROUCHING;
    }
    iChance += iBonus;
  }
  // if shooter is prone, he aims even better, except at really close range
  else if (gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_PRONE) {
    if (iRange > MIN_PRONE_RANGE) {
      iBonus = iRange / 10;
      if (iBonus > AIM_BONUS_PRONE) {
        iBonus = AIM_BONUS_PRONE;
      }
      bAttachPos = FindAttachment(pInHand, BIPOD);
      if (bAttachPos !=
          ITEM_NOT_FOUND) {  // extra bonus to hit for a bipod, up to half the prone bonus itself
        iBonus += (iBonus * WEAPON_STATUS_MOD(pInHand->bAttachStatus[bAttachPos]) / 100) / 2;
      }
      iChance += iBonus;
    }
  }

  if (!(Item[usInHand].fFlags & ITEM_TWO_HANDED)) {
    // SMGs are treated as pistols for these purpose except there is a -5 penalty;
    if (Weapon[usInHand].ubWeaponClass == SMGCLASS) {
      iChance -= AIM_PENALTY_SMG;
    }

    /*
    if (pSoldier->inv[SECONDHANDPOS].usItem == NOTHING)
    {
            // firing with pistol in right hand, and second hand empty.
            iChance += AIM_BONUS_TWO_HANDED_PISTOL;
    }
    else */
    if (!HAS_SKILL_TRAIT(pSoldier, AMBIDEXT)) {
      if (IsValidSecondHandShot(pSoldier)) {
        // penalty to aim when firing two pistols
        iChance -= AIM_PENALTY_DUAL_PISTOLS;
      }
      /*
      else
      {
              // penalty to aim with pistol being fired one-handed
              iChance -= AIM_PENALTY_ONE_HANDED_PISTOL;
      }
      */
    }
  }

  // If in burst mode, deduct points for change to hit for each shot after the first
  if (pSoldier->bDoBurst) {
    iPenalty = Weapon[usInHand].ubBurstPenalty * (pSoldier->bDoBurst - 1);

    // halve the penalty for people with the autofire trait
    if (HAS_SKILL_TRAIT(pSoldier, AUTO_WEAPS)) {
      iPenalty /= 2 * NUM_SKILL_TRAITS(pSoldier, AUTO_WEAPS);
    }
    iChance -= iPenalty;
  }

  sDistVis = DistanceVisible(pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sGridNo, 0);

  // give some leeway to allow people to spot for each other...
  // use distance limitation for LOS routine of 2 x maximum distance EVER visible, so that we get
  // accurate calculations out to around 50 tiles.  Because we multiply max distance by 2, we must
  // divide by 2 later

  // CJC August 13 2002:  Wow, this has been wrong the whole time.  bTargetCubeLevel seems to be
  // generally set to 2 - but if a character is shooting at an enemy in a particular spot, then we
  // should be using the target position on the body.

  // CJC August 13, 2002
  // If the start soldier has a body part they are aiming at, and know about the person in the tile,
  // then use that height instead
  iSightRange = -1;

  ubTargetID = WhoIsThere2(sGridNo, pSoldier->bTargetLevel);
  // best to use team knowledge as well, in case of spotting for someone else
  if ((ubTargetID != NOBODY && pSoldier->bOppList[ubTargetID] == SEEN_CURRENTLY) ||
      gbPublicOpplist[pSoldier->bTeam][ubTargetID] == SEEN_CURRENTLY) {
    iSightRange = SoldierToBodyPartLineOfSightTest(pSoldier, sGridNo, pSoldier->bTargetLevel,
                                                   pSoldier->bAimShotLocation,
                                                   (uint8_t)(MaxDistanceVisible() * 2), TRUE);
  }

  if (iSightRange == -1)  // didn't do a bodypart-based test
  {
    iSightRange = SoldierTo3DLocationLineOfSightTest(pSoldier, sGridNo, pSoldier->bTargetLevel,
                                                     pSoldier->bTargetCubeLevel,
                                                     (uint8_t)(MaxDistanceVisible() * 2), TRUE);
  }

  iSightRange *= 2;

  if (iSightRange > (sDistVis * CELL_X_SIZE)) {
    // shooting beyond max normal vision... penalize such distance at double (also later we halve
    // the remaining chance)
    iSightRange += (iSightRange - sDistVis * CELL_X_SIZE);
  }

  // if shooter spent some extra time aiming and can see the target
  if (iSightRange > 0 && ubAimTime && !pSoldier->bDoBurst)
    iChance += (AIM_BONUS_PER_AP * ubAimTime);  // bonus for every pt of aiming

  if (!(pSoldier->uiStatusFlags & SOLDIER_PC))  // if this is a computer AI controlled enemy
  {
    if (gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY) {
      // On easy, penalize all enemies by 5%
      iChance -= 5;
    } else {
      // max with 0 to prevent this being a bonus, for JA2 it's just a penalty to make early enemies
      // easy CJC note: IDIOT!  This should have been a min.  It's kind of too late now... CJC
      // 2002-05-17: changed the max to a min to make this work.
      iChance += min(0, gbDiff[DIFF_ENEMY_TO_HIT_MOD][SoldierDifficultyLevel(pSoldier)]);
    }
  }

  // if shooter is being affected by gas
  if (pSoldier->uiStatusFlags & SOLDIER_GASSED) {
    iChance -= AIM_PENALTY_GASSED;
  }

  // if shooter is being bandaged at the same time, his concentration is off
  if (pSoldier->ubServiceCount > 0) iChance -= AIM_PENALTY_GETTINGAID;

  // if shooter is still in shock
  if (pSoldier->bShock) iChance -= (pSoldier->bShock * AIM_PENALTY_PER_SHOCK);

  if (Item[usInHand].usItemClass == IC_GUN) {
    bAttachPos = FindAttachment(pInHand, GUN_BARREL_EXTENDER);
    if (bAttachPos != ITEM_NOT_FOUND) {
      // reduce status and see if it falls off
      pInHand->bAttachStatus[bAttachPos] -= (int8_t)Random(2);

      if (pInHand->bAttachStatus[bAttachPos] - Random(35) - Random(35) < USABLE) {
        // barrel extender falls off!
        struct OBJECTTYPE Temp;

        // since barrel extenders are not removable we cannot call RemoveAttachment here
        // and must create the item by hand
        CreateItem(GUN_BARREL_EXTENDER, pInHand->bAttachStatus[bAttachPos], &Temp);
        pInHand->usAttachItem[bAttachPos] = NOTHING;
        pInHand->bAttachStatus[bAttachPos] = 0;

        // drop it to ground
        AddItemToPool(pSoldier->sGridNo, &Temp, 1, pSoldier->bLevel, 0, -1);

        // big penalty to hit
        iChance -= 30;

        // curse!
        if (pSoldier->bTeam == OUR_TEAM) {
          DoMercBattleSound(pSoldier, BATTLE_SOUND_CURSE1);

          ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[46], pSoldier->name);
        }
      }
    }

    iMaxRange = GunRange(pInHand);
  } else {
    iMaxRange = CELL_X_SIZE;  // one tile
  }

  if (iSightRange > 0) {
    if (pSoldier->inv[HEAD1POS].usItem == SUNGOGGLES ||
        pSoldier->inv[HEAD2POS].usItem == SUNGOGGLES) {
      // decrease effective range by 10% when using sungoggles (w or w/o scope)
      iSightRange -= iRange / 10;  // basically, +1% to hit per every 2 squares
    }

    bAttachPos = FindAttachment(pInHand, SNIPERSCOPE);

    // does gun have scope, long range recommends its use, and shooter's aiming?
    if (bAttachPos != NO_SLOT && (iRange > MIN_SCOPE_RANGE) && (ubAimTime > 0)) {
      // reduce effective sight range by 20% per extra aiming time AP of the distance
      // beyond MIN_SCOPE_RANGE.  Max reduction is 80% of the range beyond.
      iScopeBonus = ((SNIPERSCOPE_AIM_BONUS * ubAimTime) * (iRange - MIN_SCOPE_RANGE)) / 100;

      // adjust for scope condition, only has full affect at 100%
      iScopeBonus = (iScopeBonus * WEAPON_STATUS_MOD(pInHand->bAttachStatus[bAttachPos])) / 100;

      // reduce effective range by the bonus obtained from the scope
      iSightRange -= iScopeBonus;
      if (iSightRange < 1) {
        iSightRange = 1;
      }
    }

    bAttachPos = FindAttachment(pInHand, LASERSCOPE);
    if (usInHand == ROCKET_RIFLE || usInHand == AUTO_ROCKET_RIFLE ||
        bAttachPos != NO_SLOT)  // rocket rifle has one built in
    {
      int8_t bLaserStatus;

      if (usInHand == ROCKET_RIFLE || usInHand == AUTO_ROCKET_RIFLE) {
        bLaserStatus = WEAPON_STATUS_MOD(pInHand->bGunStatus);
      } else {
        bLaserStatus = WEAPON_STATUS_MOD(pInHand->bAttachStatus[bAttachPos]);
      }

      // laser scope isn't of much use in high light levels; add something for that
      if (bLaserStatus > 50) {
        iScopeBonus = LASERSCOPE_BONUS * (bLaserStatus - 50) / 50;
      } else {
        // laser scope in bad condition creates aim penalty!
        iScopeBonus = -LASERSCOPE_BONUS * (50 - bLaserStatus) / 50;
      }

      iChance += iScopeBonus;
    }
  }

  // if aiming at the head, reduce chance to hit
  if (ubAimPos == AIM_SHOT_HEAD) {
    // penalty of 3% per tile
    iPenalty = 3 * iSightRange / 10;
    iChance -= iPenalty;
  } else if (ubAimPos == AIM_SHOT_LEGS) {
    // penalty of 1% per tile
    iPenalty = iSightRange / 10;
    iChance -= iPenalty;
  }

  // NumMessage("EFFECTIVE RANGE = ",range);

  // ADJUST FOR RANGE
  // bonus if range is less than normal range, penalty if it's more
  // iChance += (NORMAL_RANGE - iRange) / (CELL_X_SIZE / 5);	// 5% per tile

  // Effects of actual gun max range... the numbers are based on wanting -40%
  // at range 26for a pistol with range 13, and -0 for a sniper rifle with range 80
  iPenalty = ((iMaxRange - iRange * 3) * 10) / (17 * CELL_X_SIZE);
  if (iPenalty < 0) {
    iChance += iPenalty;
  }
  // iChance -= 20 * iRange / iMaxRange;

  if (TANK(pSoldier) && (iRange / CELL_X_SIZE < MaxDistanceVisible())) {
    // tank; penalize at close range!
    // 2 percent per tile closer than max visible distance
    iChance -= 2 * (MaxDistanceVisible() - (iRange / CELL_X_SIZE));
  }

  if (iSightRange == 0) {
    // firing blind!
    iChance -= AIM_PENALTY_BLIND;
  } else {
    // Effects based on aiming & sight
    // From for JA2.5:  3% bonus/penalty for each tile different from range NORMAL_RANGE.
    // This doesn't provide a bigger bonus at close range, but stretches it out, making medium
    // range less penalized, and longer range more penalized
    iChance += 3 * (NORMAL_RANGE - iSightRange) / CELL_X_SIZE;
    /*
    if (iSightRange < NORMAL_RANGE)
    {
            // bonus to hit of 20% at point blank (would be 25% at range 0);
            //at NORMAL_RANGE, bonus is 0
            iChance += 25 * (NORMAL_RANGE - iSightRange) / NORMAL_RANGE;
    }
    else
    {
            // penalty of 2% / tile
            iChance -= (iSightRange - NORMAL_RANGE) / 5;
    }
    */
  }

  // adjust for roof/not on roof
  if (pSoldier->bLevel == 0) {
    if (pSoldier->bTargetLevel > 0) {
      // penalty for firing up
      iChance -= AIM_PENALTY_FIRING_UP;
    }
  } else  // pSoldier->bLevel > 0 )
  {
    if (pSoldier->bTargetLevel == 0) {
      iChance += AIM_BONUS_FIRING_DOWN;
    }
    // if have roof trait, give bonus
    if (HAS_SKILL_TRAIT(pSoldier, ONROOF)) {
      iChance += gbSkillTraitBonus[ONROOF] * NUM_SKILL_TRAITS(pSoldier, ONROOF);
    }
  }

  pTarget = SimpleFindSoldier(sGridNo, pSoldier->bTargetLevel);
  if (pTarget != NULL) {
    // targeting a merc
    // adjust for crouched/prone target
    switch (gAnimControl[pTarget->usAnimState].ubHeight) {
      case ANIM_CROUCH:
        if (TANK(pSoldier) && iRange < MIN_TANK_RANGE) {
          // 13% penalty per tile closer than min range
          iChance -= 13 * ((MIN_TANK_RANGE - iRange) / CELL_X_SIZE);
        } else {
          // at anything other than point-blank range
          if (iRange > POINT_BLANK_RANGE + 10 * (AIM_PENALTY_TARGET_CROUCHED / 3)) {
            iChance -= AIM_PENALTY_TARGET_CROUCHED;
          } else if (iRange > POINT_BLANK_RANGE) {
            // at close range give same bonus as prone, up to maximum of AIM_PENALTY_TARGET_CROUCHED
            iChance -= 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE);  // penalty -3%/tile
          }
        }
        break;
      case ANIM_PRONE:
        if (TANK(pSoldier) && iRange < MIN_TANK_RANGE) {
          // 25% penalty per tile closer than min range
          iChance -= 25 * ((MIN_TANK_RANGE - iRange) / CELL_X_SIZE);
        } else {
          // at anything other than point-blank range
          if (iRange > POINT_BLANK_RANGE) {
            // reduce chance to hit with distance to the prone/immersed target
            iPenalty = 3 * ((iRange - POINT_BLANK_RANGE) / CELL_X_SIZE);  // penalty -3%/tile
            iPenalty = min(iPenalty, AIM_PENALTY_TARGET_PRONE);

            iChance -= iPenalty;
          }
        }
        break;
      case ANIM_STAND:
        // if we are prone and at close range, then penalize shots to the torso or head!
        if (iRange <= MIN_PRONE_RANGE &&
            gAnimControl[pSoldier->usAnimState].ubEndHeight == ANIM_PRONE) {
          if (ubAimPos == AIM_SHOT_RANDOM || ubAimPos == AIM_SHOT_GLAND) {
            ubAdjAimPos = AIM_SHOT_TORSO;
          } else {
            ubAdjAimPos = ubAimPos;
          }
          // lose 10% per height difference, lessened by distance
          // e.g. 30% to aim at head at range 1, only 10% at range 3
          // or 20% to aim at torso at range 1, no penalty at range 3
          // NB torso aim position is 2, so (5-aimpos) is 3, for legs it's 2, for head 4
          iChance -= (5 - ubAdjAimPos - iRange / CELL_X_SIZE) * 10;
        }
        break;
      default:
        break;
    }

    // penalty for amount that enemy has moved
    iPenalty = min(((pTarget->bTilesMoved * 3) / 2), 30);
    iChance -= iPenalty;

    // if target sees us, he may have a chance to dodge before the gun goes off
    // but ability to dodge is reduced if crouched or prone!
    if (pTarget->bOppList[pSoldier->ubID] == SEEN_CURRENTLY && !TANK(pTarget) &&
        !(pSoldier->ubBodyType != QUEENMONSTER)) {
      iPenalty = (EffectiveAgility(pTarget) / 5 + EffectiveExpLevel(pTarget) * 2);
      switch (gAnimControl[pTarget->usAnimState].ubHeight) {
        case ANIM_CROUCH:
          iPenalty = iPenalty * 2 / 3;
          break;
        case ANIM_PRONE:
          iPenalty /= 3;
          break;
      }

      // reduce dodge ability by the attacker's stats
      iBonus = (EffectiveDexterity(pSoldier) / 5 + EffectiveExpLevel(pSoldier) * 2);
      if (TANK(pTarget) || (pSoldier->ubBodyType != QUEENMONSTER)) {
        // reduce ability to track shots
        iBonus = iBonus / 2;
      }

      if (iPenalty > iBonus) {
        iChance -= (iPenalty - iBonus);
      }
    }
  } else if (TANK(pSoldier) && iRange < MIN_TANK_RANGE) {
    // 25% penalty per tile closer than min range
    iChance -= 25 * ((MIN_TANK_RANGE - iRange) / CELL_X_SIZE);
  }

  // add camo effects

#if 0
	if ((victim = WhoIsThere(sGridNo)) < NOBODY)
	 {

		// if victim is 5 or more tiles away and camouflaged, reduce
		// chance to hit by 5%  (ALREADY HAVE THIS INFO)
		if (range > 75 && vicpSoldier->camouflage)
		 {
			switch(vicpSoldier->terrtype)
			{
			 case GROUNDTYPE:
			 case SANDTYPE  :
			 case GRASSTYPE :
			 case TGRASSTYPE:
			 case DGRASSTYPE:
			 case ROUGHTYPE : iChance += CAMOUFLAGE_TO_HIT_PENALTY;
						break;

			 case FLOORTYPE :
			 case LAKETYPE  :
			 case OCEANTYPE : break;

#ifdef BETAVERSION
			 default        : NumMessage("CHANCE TO HIT ERROR: Unknown camo terrtype ",vicpSoldier->terrtype);
#endif
			}
		 }
	 }
#endif

  // IF CHANCE EXISTS, BUT SHOOTER IS INJURED
  if ((iChance > 0) && (pSoldier->bLife < pSoldier->bLifeMax)) {
    // if bandaged, give 1/2 of the bandaged life points back into equation
    bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

    // injury penalty is based on % damage taken (max 2/3rds chance)
    iPenalty = (iChance * 2 * (pSoldier->bLifeMax - pSoldier->bLife + (bBandaged / 2))) /
               (3 * pSoldier->bLifeMax);

    // reduce injury penalty due to merc's experience level (he can take it!)
    iChance -= (iPenalty * (100 - (10 * (EffectiveExpLevel(pSoldier) - 1)))) / 100;
  }

  // IF CHANCE EXISTS, BUT SHOOTER IS LOW ON BREATH
  if ((iChance > 0) && (pSoldier->bBreath < 100)) {
    // breath penalty is based on % breath missing (max 1/2 chance)
    iPenalty = (iChance * (100 - pSoldier->bBreath)) / 200;
    // reduce breath penalty due to merc's dexterity (he can compensate!)
    iChance -= (iPenalty * (100 - (EffectiveDexterity(pSoldier) - 10))) / 100;
  }

  // CHECK IF TARGET IS WITHIN GUN'S EFFECTIVE MAXIMUM RANGE
  if (iRange > iMaxRange) {
    // a bullet WILL travel that far if not blocked, but it's NOT accurate,
    // because beyond maximum range, the bullet drops rapidly

    // This won't cause the bullet to be off to the left or right, only make it
    // drop in flight.
    iChance /= 2;
  }
  if (iSightRange > (sDistVis * CELL_X_SIZE)) {
    // penalize out of sight shots, cumulative to effective range penalty
    iChance /= 2;
  }

  // MAKE SURE CHANCE TO HIT IS WITHIN DEFINED LIMITS
  if (iChance < MINCHANCETOHIT) {
    if (TANK(pSoldier)) {
      // allow absolute minimums
      iChance = 0;
    } else {
      iChance = MINCHANCETOHIT;
    }
  } else {
    if (iChance > MAXCHANCETOHIT) iChance = MAXCHANCETOHIT;
  }

  //  NumMessage("ChanceToHit = ",chance);
  return (iChance);
}

uint32_t AICalcChanceToHitGun(struct SOLDIERTYPE *pSoldier, uint16_t sGridNo, uint8_t ubAimTime,
                              uint8_t ubAimPos) {
  uint16_t usTrueState;
  uint32_t uiChance;

  // same as CCTHG but fakes the attacker always standing
  usTrueState = pSoldier->usAnimState;
  pSoldier->usAnimState = STANDING;
  uiChance = CalcChanceToHitGun(pSoldier, sGridNo, ubAimTime, ubAimPos);
  pSoldier->usAnimState = usTrueState;
  return (uiChance);
}

int32_t CalcBodyImpactReduction(uint8_t ubAmmoType, uint8_t ubHitLocation) {
  // calculate how much bullets are slowed by passing through someone
  int32_t iReduction = BodyImpactReduction[ubHitLocation];

  switch (ubAmmoType) {
    case AMMO_HP:
      iReduction = AMMO_ARMOUR_ADJUSTMENT_HP(iReduction);
      break;
    case AMMO_AP:
    case AMMO_HEAT:
      iReduction = AMMO_ARMOUR_ADJUSTMENT_AP(iReduction);
      break;
    case AMMO_SUPER_AP:
      iReduction = AMMO_ARMOUR_ADJUSTMENT_SAP(iReduction);
      break;
    default:
      break;
  }
  return (iReduction);
}

int32_t ArmourProtection(struct SOLDIERTYPE *pTarget, uint8_t ubArmourType, int8_t *pbStatus,
                         int32_t iImpact, uint8_t ubAmmoType) {
  int32_t iProtection, iAppliedProtection, iFailure;

  iProtection = Armour[ubArmourType].ubProtection;

  if (!AM_A_ROBOT(pTarget)) {
    // check for the bullet hitting a weak spot in the armour
    iFailure = PreRandom(100) + 1 - *pbStatus;
    if (iFailure > 0) {
      iProtection -= iFailure;
      if (iProtection < 0) {
        return (0);
      }
    }
  }

  // adjust protection of armour due to different ammo types
  switch (ubAmmoType) {
    case AMMO_HP:
      iProtection = AMMO_ARMOUR_ADJUSTMENT_HP(iProtection);
      break;
    case AMMO_AP:
    case AMMO_HEAT:
      iProtection = AMMO_ARMOUR_ADJUSTMENT_AP(iProtection);
      break;
    case AMMO_SUPER_AP:
      iProtection = AMMO_ARMOUR_ADJUSTMENT_SAP(iProtection);
      break;
    default:
      break;
  }

  // figure out how much of the armour's protection value is necessary
  // in defending against this bullet
  if (iProtection > iImpact) {
    iAppliedProtection = iImpact;
  } else {
    // applied protection is the full strength of the armour, before AP/HP changes
    iAppliedProtection = Armour[ubArmourType].ubProtection;
  }

  // reduce armour condition

  if (ubAmmoType == AMMO_KNIFE || ubAmmoType == AMMO_SLEEP_DART) {
    // knives and darts damage armour but are not stopped by kevlar
    if (Armour[ubArmourType].ubArmourClass == ARMOURCLASS_VEST ||
        Armour[ubArmourType].ubArmourClass == ARMOURCLASS_LEGGINGS) {
      iProtection = 0;
    }
  } else if (ubAmmoType == AMMO_MONSTER) {
    // creature spit damages armour a lot! an extra 3x for a total of 4x normal
    *pbStatus -= 3 * (iAppliedProtection * Armour[ubArmourType].ubDegradePercent) / 100;

    // reduce amount of protection from armour
    iProtection /= 2;
  }

  if (!AM_A_ROBOT(pTarget)) {
    *pbStatus -= (iAppliedProtection * Armour[ubArmourType].ubDegradePercent) / 100;
  }

  // return armour protection
  return (iProtection);
}

int32_t TotalArmourProtection(struct SOLDIERTYPE *pFirer, struct SOLDIERTYPE *pTarget,
                              uint8_t ubHitLocation, int32_t iImpact, uint8_t ubAmmoType) {
  int32_t iTotalProtection = 0, iSlot;
  struct OBJECTTYPE *pArmour;
  int8_t bPlatePos = -1;

  if (pTarget->uiStatusFlags & SOLDIER_VEHICLE) {
    int8_t bDummyStatus = 100;

    // bDummyStatus = (int8_t) pVehicleList[ pTarget->bVehicleID ].sExternalArmorLocationsStatus[
    // ubHitLocation ];

    iTotalProtection +=
        ArmourProtection(pTarget, (uint8_t)pVehicleList[pTarget->bVehicleID].sArmourType,
                         &bDummyStatus, iImpact, ubAmmoType);

    // pVehicleList[ pTarget->bVehicleID ].sExternalArmorLocationsStatus[ ubHitLocation ] =
    // bDummyStatus;

  } else {
    switch (ubHitLocation) {
      case AIM_SHOT_GLAND:
        // creature hit in the glands!!! no armour there!
        return (0);
      case AIM_SHOT_HEAD:
        iSlot = HELMETPOS;
        break;
      case AIM_SHOT_LEGS:
        iSlot = LEGPOS;
        break;
      case AIM_SHOT_TORSO:
      default:
        iSlot = VESTPOS;
        break;
    }

    pArmour = &(pTarget->inv[iSlot]);
    if (pArmour->usItem != NOTHING) {
      // check plates first
      if (iSlot == VESTPOS) {
        bPlatePos = FindAttachment(pArmour, CERAMIC_PLATES);
        if (bPlatePos != -1) {
          // bullet got through jacket; apply ceramic plate armour
          iTotalProtection +=
              ArmourProtection(pTarget, Item[pArmour->usAttachItem[bPlatePos]].ubClassIndex,
                               &(pArmour->bAttachStatus[bPlatePos]), iImpact, ubAmmoType);
          if (pArmour->bAttachStatus[bPlatePos] < USABLE) {
            // destroy plates!
            pArmour->usAttachItem[bPlatePos] = NOTHING;
            pArmour->bAttachStatus[bPlatePos] = 0;
            DirtyMercPanelInterface(pTarget, DIRTYLEVEL2);
            if (pTarget->bTeam == gbPlayerNum) {
              // report plates destroyed!
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, gzLateLocalizedString[61],
                        pTarget->name);
            }
          }
        }
      }

      // if the plate didn't stop the bullet...
      if (iImpact > iTotalProtection) {
        iTotalProtection += ArmourProtection(pTarget, Item[pArmour->usItem].ubClassIndex,
                                             &(pArmour->bStatus[0]), iImpact, ubAmmoType);
        if (pArmour->bStatus[0] < USABLE) {
          DeleteObj(pArmour);
          DirtyMercPanelInterface(pTarget, DIRTYLEVEL2);
        }
      }
    }
  }
  return (iTotalProtection);
}

int32_t BulletImpact(struct SOLDIERTYPE *pFirer, struct SOLDIERTYPE *pTarget, uint8_t ubHitLocation,
                     int32_t iOrigImpact, int16_t sHitBy, uint8_t *pubSpecial) {
  int32_t iImpact, iFluke, iBonus, iImpactForCrits = 0;
  int8_t bStatLoss;
  uint8_t ubAmmoType;

  // NOTE: reduction of bullet impact due to range and obstacles is handled
  // in MoveBullet.

  // Set a few things up:
  if (Item[pFirer->usAttackingWeapon].usItemClass == IC_THROWING_KNIFE) {
    ubAmmoType = AMMO_KNIFE;
  } else {
    ubAmmoType = pFirer->inv[pFirer->ubAttackingHand].ubGunAmmoType;
  }

  if (TANK(pTarget)) {
    if (ubAmmoType != AMMO_HEAT) {
      // ping!
      return (0);
    }
  }

  // plus/minus up to 25% due to "random" factors (major organs hit or missed,
  // lucky lighter in breast pocket, divine intervention on behalf of "Rev"...)
  iFluke = PreRandom(51) - 25;  // gives (0 to 50 -25) -> -25% to +25%
  // NumMessage("Fluke = ",fluke);

  // up to 50% extra impact for making particularly accurate successful shots
  iBonus = sHitBy / 2;
  // NumMessage("Bonus = ",bonus);

  iOrigImpact = iOrigImpact * (100 + iFluke + iBonus) / 100;

  // at very long ranges (1.5x maxRange and beyond) impact could go negative
  if (iOrigImpact < 1) {
    iOrigImpact = 1;  // raise impact to a minimum of 1 for any hit
  }

  // adjust for HE rounds
  if (ubAmmoType == AMMO_HE || ubAmmoType == AMMO_HEAT) {
    iOrigImpact = AMMO_DAMAGE_ADJUSTMENT_HE(iOrigImpact);

    if (TANK(pTarget)) {
      // HEAT round on tank, divide by 3 for damage
      iOrigImpact /= 2;
    }
  }

  if (pubSpecial && *pubSpecial == FIRE_WEAPON_BLINDED_BY_SPIT_SPECIAL) {
    iImpact = iOrigImpact;
  } else {
    iImpact = iOrigImpact -
              TotalArmourProtection(pFirer, pTarget, ubHitLocation, iOrigImpact, ubAmmoType);
  }

  // calc minimum damage
  if (ubAmmoType == AMMO_HP || ubAmmoType == AMMO_SLEEP_DART) {
    if (iImpact < 0) {
      iImpact = 0;
    }
  } else {
    if (iImpact < ((iOrigImpact + 5) / 10)) {
      iImpact = (iOrigImpact + 5) / 10;
    }

    if ((ubAmmoType == AMMO_BUCKSHOT) && (pTarget->bNumPelletsHitBy > 0)) {
      iImpact += (pTarget->bNumPelletsHitBy - 1) / 2;
    }
  }

  if (gfNextShotKills) {
    // big time cheat key effect!
    iImpact = 100;
    gfNextShotKills = FALSE;
  }

  if (iImpact > 0 && !TANK(pTarget)) {
    if (ubAmmoType == AMMO_SLEEP_DART && sHitBy > 20) {
      if (pubSpecial) {
        *pubSpecial = FIRE_WEAPON_SLEEP_DART_SPECIAL;
      }
      return (iImpact);
    }

    if (ubAmmoType ==
        AMMO_HP) {  // good solid hit with a hollow-point bullet, which got through armour!
      iImpact = AMMO_DAMAGE_ADJUSTMENT_HP(iImpact);
    }

    AdjustImpactByHitLocation(iImpact, ubHitLocation, &iImpact, &iImpactForCrits);

    switch (ubHitLocation) {
      case AIM_SHOT_HEAD:
        // is the blow deadly enough for an instant kill?
        if (PythSpacesAway(pFirer->sGridNo, pTarget->sGridNo) <= MAX_DISTANCE_FOR_MESSY_DEATH) {
          if (iImpactForCrits > MIN_DAMAGE_FOR_INSTANT_KILL && iImpactForCrits < pTarget->bLife) {
            // blow to the head is so deadly that it causes instant death;
            // the target has more life than iImpact so we increase it
            iImpact = pTarget->bLife + Random(10);
            iImpactForCrits = iImpact;
          }

          if (pubSpecial) {
            // is the blow deadly enough to cause a head explosion?
            if (iImpactForCrits >= pTarget->bLife) {
              if (iImpactForCrits > MIN_DAMAGE_FOR_HEAD_EXPLOSION) {
                *pubSpecial = FIRE_WEAPON_HEAD_EXPLODE_SPECIAL;
              } else if (iImpactForCrits > (MIN_DAMAGE_FOR_HEAD_EXPLOSION / 2) &&
                         (PreRandom(MIN_DAMAGE_FOR_HEAD_EXPLOSION / 2) <
                          (uint32_t)(iImpactForCrits - MIN_DAMAGE_FOR_HEAD_EXPLOSION / 2))) {
                *pubSpecial = FIRE_WEAPON_HEAD_EXPLODE_SPECIAL;
              }
            }
          }
        }
        break;
      case AIM_SHOT_LEGS:
        // is the damage enough to make us fall over?
        if (pubSpecial && IS_MERC_BODY_TYPE(pTarget) &&
            gAnimControl[pTarget->usAnimState].ubEndHeight == ANIM_STAND &&
            pTarget->bOverTerrainType != LOW_WATER && pTarget->bOverTerrainType != MED_WATER &&
            pTarget->bOverTerrainType != DEEP_WATER) {
          if (iImpactForCrits > MIN_DAMAGE_FOR_AUTO_FALL_OVER) {
            *pubSpecial = FIRE_WEAPON_LEG_FALLDOWN_SPECIAL;
          }
          // else ramping up chance from 1/2 the automatic value onwards
          else if (iImpactForCrits > (MIN_DAMAGE_FOR_AUTO_FALL_OVER / 2) &&
                   (PreRandom(MIN_DAMAGE_FOR_AUTO_FALL_OVER / 2) <
                    (uint32_t)(iImpactForCrits - MIN_DAMAGE_FOR_AUTO_FALL_OVER / 2))) {
            *pubSpecial = FIRE_WEAPON_LEG_FALLDOWN_SPECIAL;
          }
        }
        break;
      case AIM_SHOT_TORSO:
        // normal damage to torso
        // is the blow deadly enough for an instant kill?
        // since this value is much lower than the others, it only applies at short range...
        if (PythSpacesAway(pFirer->sGridNo, pTarget->sGridNo) <= MAX_DISTANCE_FOR_MESSY_DEATH) {
          if (iImpact > MIN_DAMAGE_FOR_INSTANT_KILL && iImpact < pTarget->bLife) {
            // blow to the chest is so deadly that it causes instant death;
            // the target has more life than iImpact so we increase it
            iImpact = pTarget->bLife + Random(10);
            iImpactForCrits = iImpact;
          }
          // special thing for hitting chest - allow cumulative damage to count
          else if ((iImpact + pTarget->sDamage) >
                   (MIN_DAMAGE_FOR_BLOWN_AWAY + MIN_DAMAGE_FOR_INSTANT_KILL)) {
            iImpact = pTarget->bLife + Random(10);
            iImpactForCrits = iImpact;
          }

          // is the blow deadly enough to cause a chest explosion?
          if (pubSpecial) {
            if (iImpact > MIN_DAMAGE_FOR_BLOWN_AWAY && iImpact >= pTarget->bLife) {
              *pubSpecial = FIRE_WEAPON_CHEST_EXPLODE_SPECIAL;
            }
          }
        }
        break;
    }
  }

  if (AM_A_ROBOT(pTarget)) {
    iImpactForCrits = 0;
  }

  // don't do critical hits against people who are gonna die!
  if (!IsAutoResolveActive()) {
    if (ubAmmoType == AMMO_KNIFE && pFirer->bOppList[pTarget->ubID] == SEEN_CURRENTLY) {
      // is this a stealth attack?
      if (pTarget->bOppList[pFirer->ubID] == NOT_HEARD_OR_SEEN && !CREATURE_OR_BLOODCAT(pTarget) &&
          (ubHitLocation == AIM_SHOT_HEAD || ubHitLocation == AIM_SHOT_TORSO)) {
        if (PreRandom(100) < (uint32_t)(sHitBy + 10 * NUM_SKILL_TRAITS(pFirer, THROWING))) {
          // instant death!
          iImpact = pTarget->bLife + Random(10);
          iImpactForCrits = iImpact;
        }
      }
    }

    if (iImpactForCrits > 0 && iImpactForCrits < pTarget->bLife) {
      if (PreRandom(iImpactForCrits / 2 + pFirer->bAimTime * 5) + 1 > CRITICAL_HIT_THRESHOLD) {
        bStatLoss = (int8_t)PreRandom(iImpactForCrits / 2) + 1;
        switch (ubHitLocation) {
          case AIM_SHOT_HEAD:
            if (bStatLoss >= pTarget->bWisdom) {
              bStatLoss = pTarget->bWisdom - 1;
            }
            if (bStatLoss > 0) {
              pTarget->bWisdom -= bStatLoss;

              if (pTarget->ubProfile != NO_PROFILE) {
                gMercProfiles[pTarget->ubProfile].bWisdom = pTarget->bWisdom;
              }

              if (pTarget->name[0] && pTarget->bVisible == TRUE) {
                // make stat RED for a while...
                pTarget->uiChangeWisdomTime = GetJA2Clock();
                pTarget->usValueGoneUp &= ~(WIS_INCREASE);

                if (bStatLoss == 1) {
                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_1_WISDOM],
                            pTarget->name);
                } else {
                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_WISDOM],
                            pTarget->name, bStatLoss);
                }
              }
            } else if (pTarget->bNumPelletsHitBy == 0) {
              ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_HEAD_HIT], pTarget->name);
            }
            break;
          case AIM_SHOT_TORSO:
            if (PreRandom(1) == 0 && !(pTarget->uiStatusFlags & SOLDIER_MONSTER)) {
              if (bStatLoss >= pTarget->bDexterity) {
                bStatLoss = pTarget->bDexterity - 1;
              }
              if (bStatLoss > 0) {
                pTarget->bDexterity -= bStatLoss;

                if (pTarget->ubProfile != NO_PROFILE) {
                  gMercProfiles[pTarget->ubProfile].bDexterity = pTarget->bDexterity;
                }

                if (pTarget->name[0] && pTarget->bVisible == TRUE) {
                  // make stat RED for a while...
                  pTarget->uiChangeDexterityTime = GetJA2Clock();
                  pTarget->usValueGoneUp &= ~(DEX_INCREASE);

                  if (bStatLoss == 1) {
                    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_1_DEX],
                              pTarget->name);
                  } else {
                    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_DEX],
                              pTarget->name, bStatLoss);
                  }
                }
              }
            } else {
              if (bStatLoss >= pTarget->bStrength) {
                bStatLoss = pTarget->bStrength - 1;
              }
              if (bStatLoss > 0) {
                pTarget->bStrength -= bStatLoss;

                if (pTarget->ubProfile != NO_PROFILE) {
                  gMercProfiles[pTarget->ubProfile].bStrength = pTarget->bStrength;
                }

                if (pTarget->name[0] && pTarget->bVisible == TRUE) {
                  // make stat RED for a while...
                  pTarget->uiChangeStrengthTime = GetJA2Clock();
                  pTarget->usValueGoneUp &= ~(STRENGTH_INCREASE);

                  if (bStatLoss == 1) {
                    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_1_STRENGTH],
                              pTarget->name);
                  } else {
                    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_STRENGTH],
                              pTarget->name, bStatLoss);
                  }
                }
              }
            }
            break;
          case AIM_SHOT_LEGS:
            if (bStatLoss >= pTarget->bAgility) {
              bStatLoss = pTarget->bAgility - 1;
            }
            if (bStatLoss > 0) {
              pTarget->bAgility -= bStatLoss;

              if (pTarget->ubProfile != NO_PROFILE) {
                gMercProfiles[pTarget->ubProfile].bAgility = pTarget->bAgility;
              }

              if (pTarget->name[0] && pTarget->bVisible == TRUE) {
                // make stat RED for a while...
                pTarget->uiChangeAgilityTime = GetJA2Clock();
                pTarget->usValueGoneUp &= ~(AGIL_INCREASE);

                if (bStatLoss == 1) {
                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_1_AGIL],
                            pTarget->name);
                } else {
                  ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_LOSES_AGIL],
                            pTarget->name, bStatLoss);
                }
              }
            }
            break;
        }
      } else if (ubHitLocation == AIM_SHOT_HEAD && pTarget->bNumPelletsHitBy == 0) {
        ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_HEAD_HIT], pTarget->name);
      }
    }
  }

  return (iImpact);
}

int32_t HTHImpact(struct SOLDIERTYPE *pSoldier, struct SOLDIERTYPE *pTarget, int32_t iHitBy,
                  BOOLEAN fBladeAttack) {
  int32_t iImpact, iFluke, iBonus;

  if (fBladeAttack) {
    iImpact = (EffectiveExpLevel(pSoldier) / 2);  // 0 to 4 for level
    iImpact += Weapon[pSoldier->usAttackingWeapon].ubImpact;
    iImpact += EffectiveStrength(pSoldier) / 20;  // 0 to 5 for strength, adjusted by damage taken

    if (AM_A_ROBOT(pTarget)) {
      iImpact /= 4;
    }
  } else {
    iImpact = (EffectiveExpLevel(pSoldier) / 2);  // 0 to 4 for level
    iImpact += EffectiveStrength(pSoldier) / 5;   // 0 to 20 for strength, adjusted by damage taken

    // NB martial artists don't get a bonus for using brass knuckles!
    if (pSoldier->usAttackingWeapon && !(HAS_SKILL_TRAIT(pSoldier, MARTIALARTS))) {
      iImpact += Weapon[pSoldier->usAttackingWeapon].ubImpact;

      if (AM_A_ROBOT(pTarget)) {
        iImpact /= 2;
      }
    } else {
      // base HTH damage
      iImpact += 5;
      if (AM_A_ROBOT(pTarget)) {
        iImpact = 0;
      }
    }
  }

  iFluke = PreRandom(51) - 25;  // +/-25% bonus due to random factors
  iBonus = iHitBy / 2;          // up to 50% extra impact for accurate attacks

  iImpact = iImpact * (100 + iFluke + iBonus) / 100;

  if (!fBladeAttack) {
    // add bonuses for hand-to-hand and martial arts
    if (HAS_SKILL_TRAIT(pSoldier, MARTIALARTS)) {
      iImpact = iImpact *
                (100 + gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pSoldier, MARTIALARTS)) /
                100;
      if (pSoldier->usAnimState == NINJA_SPINKICK) {
        iImpact *= 2;
      }
    }
    if (HAS_SKILL_TRAIT(pSoldier, HANDTOHAND)) {
      // SPECIAL  - give TRIPLE bonus for damage for hand-to-hand trait
      // because the HTH bonus is half that of martial arts, and gets only 1x for to-hit bonus
      iImpact = iImpact *
                (100 + 3 * gbSkillTraitBonus[HANDTOHAND] * NUM_SKILL_TRAITS(pSoldier, HANDTOHAND)) /
                100;
    }
  }

  return (iImpact);
}

void ShotMiss(uint8_t ubAttackerID, int32_t iBullet) {
  BOOLEAN fDoMissForGun = FALSE;
  struct SOLDIERTYPE *pAttacker;
  BULLET *pBullet;

  pAttacker = MercPtrs[ubAttackerID];

  if (pAttacker->ubOppNum != NOBODY) {
    // if it was another team shooting at someone under our control
    if ((pAttacker->bTeam != Menptr[pAttacker->ubOppNum].bTeam)) {
      // if OPPONENT is under our control
      if (Menptr[pAttacker->ubOppNum].bTeam == gbPlayerNum) {
        // AGILITY GAIN: Opponent "dodged" a bullet shot at him (it missed)
        StatChange(MercPtrs[pAttacker->ubOppNum], AGILAMT, 5, FROM_FAILURE);
      }
    }
  }

  switch (Weapon[MercPtrs[ubAttackerID]->usAttackingWeapon].ubWeaponClass) {
    case HANDGUNCLASS:
    case RIFLECLASS:
    case SHOTGUNCLASS:
    case SMGCLASS:
    case MGCLASS:

      // Guy has missed, play random sound
      if (MercPtrs[ubAttackerID]->bTeam == gbPlayerNum) {
        if (Random(40) == 0) {
          DoMercBattleSound(MercPtrs[ubAttackerID], BATTLE_SOUND_CURSE1);
        }
      }
      fDoMissForGun = TRUE;
      break;

    case MONSTERCLASS:
      PlayJA2Sample(SPIT_RICOCHET, RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
      break;
  }

  if (fDoMissForGun) {
    // PLAY SOUND AND FLING DEBRIS
    // RANDOMIZE SOUND SYSTEM

    if (!DoSpecialEffectAmmoMiss(ubAttackerID, NOWHERE, 0, 0, 0, TRUE, TRUE, 0)) {
      PlayJA2Sample(MISS_1 + Random(8), RATE_11025, HIGHVOLUME, 1, MIDDLEPAN);
    }

    // ATE: Show misses...( if our team )
    if (gGameSettings.fOptions[TOPTION_SHOW_MISSES]) {
      pBullet = GetBulletPtr(iBullet);

      if (pAttacker->bTeam == gbPlayerNum) {
        LocateGridNo((int16_t)pBullet->sGridNo);
      }
    }
  }

  DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("@@@@@@@ Freeing up attacker - bullet missed"));
  FreeUpAttacker(ubAttackerID);
}

uint32_t CalcChanceHTH(struct SOLDIERTYPE *pAttacker, struct SOLDIERTYPE *pDefender,
                       uint8_t ubAimTime, uint8_t ubMode) {
  uint16_t usInHand;
  uint8_t ubBandaged;
  int32_t iAttRating, iDefRating;
  int32_t iChance;

  usInHand = pAttacker->usAttackingWeapon;

  if ((usInHand != CREATURE_QUEEN_TENTACLES) &&
      (pDefender->bLife < OKLIFE || pDefender->bBreath < OKBREATH)) {
    // there is NO way to miss
    return (100);
  }

  if (ubMode == HTH_MODE_STAB) {
    // safety check
    if (Weapon[usInHand].ubWeaponClass != KNIFECLASS) {
#ifdef BETAVERSION
      NumMessage("CalcChanceToStab: ERROR - Attacker isn't holding a knife, usInHand = ", usInHand);
#endif
      return (0);
    }
  } else {
    if (Item[usInHand].usItemClass != IC_PUNCH) {
      return (0);
    }
  }

  // CALCULATE ATTACKER'S CLOSE COMBAT RATING (1-100)
  if (ubMode == HTH_MODE_STEAL) {
    // this is more of a brute force strength-vs-strength check
    iAttRating = (EffectiveDexterity(pAttacker) +        // coordination, accuracy
                  EffectiveAgility(pAttacker) +          // speed & reflexes
                  3 * pAttacker->bStrength +             // physical strength (TRIPLED!)
                  (10 * EffectiveExpLevel(pAttacker)));  // experience, knowledge
  } else {
    iAttRating = (3 * EffectiveDexterity(pAttacker) +    // coordination, accuracy (TRIPLED!)
                  EffectiveAgility(pAttacker) +          // speed & reflexes
                  pAttacker->bStrength +                 // physical strength
                  (10 * EffectiveExpLevel(pAttacker)));  // experience, knowledge
  }

  iAttRating /= 6;  // convert from 6-600 to 1-100

  // psycho bonus
  if (pAttacker->ubProfile != NO_PROFILE &&
      gMercProfiles[pAttacker->ubProfile].bPersonalityTrait == PSYCHO) {
    iAttRating += AIM_BONUS_PSYCHO;
  }

  // modify chance to hit by morale
  iAttRating += GetMoraleModifier(pAttacker);

  // modify for fatigue
  iAttRating -= GetSkillCheckPenaltyForFatigue(pAttacker, iAttRating);

  // if attacker spent some extra time aiming
  if (ubAimTime) {
    // use only HALF of the normal aiming bonus for knife aiming.
    // since there's no range penalty, the bonus is otherwise too generous
    iAttRating += ((AIM_BONUS_PER_AP * ubAimTime) / 2);  // bonus for aiming
  }

  if (!(pAttacker->uiStatusFlags & SOLDIER_PC))  // if attacker is a computer AI controlled enemy
  {
    iAttRating += gbDiff[DIFF_ENEMY_TO_HIT_MOD][SoldierDifficultyLevel(pAttacker)];
  }

  // if attacker is being affected by gas
  if (pAttacker->uiStatusFlags & SOLDIER_GASSED) iAttRating -= AIM_PENALTY_GASSED;

  // if attacker is being bandaged at the same time, his concentration is off
  if (pAttacker->ubServiceCount > 0) iAttRating -= AIM_PENALTY_GETTINGAID;

  // if attacker is still in shock
  if (pAttacker->bShock) iAttRating -= (pAttacker->bShock * AIM_PENALTY_PER_SHOCK);

  /*
    // if the attacker is an A.I.M. mercenary
    if (pAttacker->characternum < MAX_AIM_MERCS)	// exclude Gus
      iAttRating += AdjustChanceForProfile(pAttacker,pDefender);
  */

  // If attacker injured, reduce chance accordingly (by up to 2/3rds)
  if ((iAttRating > 0) && (pAttacker->bLife < pAttacker->bLifeMax)) {
    // if bandaged, give 1/2 of the bandaged life points back into equation
    ubBandaged = pAttacker->bLifeMax - pAttacker->bLife - pAttacker->bBleeding;

    iAttRating -= (2 * iAttRating * (pAttacker->bLifeMax - pAttacker->bLife + (ubBandaged / 2))) /
                  (3 * pAttacker->bLifeMax);
  }

  // If attacker tired, reduce chance accordingly (by up to 1/2)
  if ((iAttRating > 0) && (pAttacker->bBreath < 100))
    iAttRating -= (iAttRating * (100 - pAttacker->bBreath)) / 200;

  if (pAttacker->ubProfile != NO_PROFILE) {
    if (ubMode == HTH_MODE_STAB) {
      if (HAS_SKILL_TRAIT(pAttacker, KNIFING)) {
        iAttRating += gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pAttacker, KNIFING);
      }
    } else {
      // add bonuses for hand-to-hand and martial arts
      if (HAS_SKILL_TRAIT(pAttacker, MARTIALARTS)) {
        iAttRating += gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pAttacker, MARTIALARTS);
      }
      if (HAS_SKILL_TRAIT(pAttacker, HANDTOHAND)) {
        iAttRating += gbSkillTraitBonus[HANDTOHAND] * NUM_SKILL_TRAITS(pAttacker, HANDTOHAND);
      }
    }
  }

  if (iAttRating < 1) iAttRating = 1;

  // CALCULATE DEFENDER'S CLOSE COMBAT RATING (0-100)
  if (ubMode == HTH_MODE_STEAL) {
    iDefRating = (EffectiveAgility(pDefender)) +       // speed & reflexes
                 EffectiveDexterity(pDefender) +       // coordination, accuracy
                 3 * pDefender->bStrength +            // physical strength (TRIPLED!)
                 (10 * EffectiveExpLevel(pDefender));  // experience, knowledge
  } else {
    iDefRating = (3 * EffectiveAgility(pDefender)) +   // speed & reflexes (TRIPLED!)
                 EffectiveDexterity(pDefender) +       // coordination, accuracy
                 pDefender->bStrength +                // physical strength
                 (10 * EffectiveExpLevel(pDefender));  // experience, knowledge
  }

  iDefRating /= 6;  // convert from 6-600 to 1-100

  // modify chance to dodge by morale
  iDefRating += GetMoraleModifier(pDefender);

  // modify for fatigue
  iDefRating -= GetSkillCheckPenaltyForFatigue(pDefender, iDefRating);

  // if attacker is being affected by gas
  if (pDefender->uiStatusFlags & SOLDIER_GASSED) iDefRating -= AIM_PENALTY_GASSED;

  // if defender is being bandaged at the same time, his concentration is off
  if (pDefender->ubServiceCount > 0) iDefRating -= AIM_PENALTY_GETTINGAID;

  // if defender is still in shock
  if (pDefender->bShock) iDefRating -= (pDefender->bShock * AIM_PENALTY_PER_SHOCK);

  /*
    // if the defender is an A.I.M. mercenary
    if (pDefender->characternum < MAX_AIM_MERCS)	// exclude Gus
      iDefRating += AdjustChanceForProfile(pDefender,pAttacker);
  */

  // If defender injured, reduce chance accordingly (by up to 2/3rds)
  if ((iDefRating > 0) && (pDefender->bLife < pDefender->bLifeMax)) {
    // if bandaged, give 1/2 of the bandaged life points back into equation
    ubBandaged = pDefender->bLifeMax - pDefender->bLife - pDefender->bBleeding;

    iDefRating -= (2 * iDefRating * (pDefender->bLifeMax - pDefender->bLife + (ubBandaged / 2))) /
                  (3 * pDefender->bLifeMax);
  }

  // If defender tired, reduce chance accordingly (by up to 1/2)
  if ((iDefRating > 0) && (pDefender->bBreath < 100))
    iDefRating -= (iDefRating * (100 - pDefender->bBreath)) / 200;

  if ((usInHand == CREATURE_QUEEN_TENTACLES && pDefender->ubBodyType == LARVAE_MONSTER) ||
      pDefender->ubBodyType == INFANT_MONSTER) {
    // try to prevent queen from killing the kids, ever!
    iDefRating += 10000;
  }

  if (gAnimControl[pDefender->usAnimState].ubEndHeight < ANIM_STAND) {
    if (usInHand == CREATURE_QUEEN_TENTACLES) {
      if (gAnimControl[pDefender->usAnimState].ubEndHeight == ANIM_PRONE) {
        // make it well-nigh impossible to hit someone who is prone!
        iDefRating += 1000;
      } else {
        iDefRating += BAD_DODGE_POSITION_PENALTY * 2;
      }
    } else {
      // if defender crouched, reduce chance accordingly (harder to dodge)
      iDefRating -= BAD_DODGE_POSITION_PENALTY;
      // If our target is prone, double the penalty!
      if (gAnimControl[pDefender->usAnimState].ubEndHeight == ANIM_PRONE) {
        iDefRating -= BAD_DODGE_POSITION_PENALTY;
      }
    }
  }

  if (pDefender->ubProfile != NO_PROFILE) {
    if (ubMode == HTH_MODE_STAB) {
      if (Item[pDefender->inv[HANDPOS].usItem].usItemClass == IC_BLADE) {
        if (HAS_SKILL_TRAIT(pDefender, KNIFING)) {
          // good with knives, got one, so we're good at parrying
          iDefRating += gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pDefender, KNIFING);
        }
        if (HAS_SKILL_TRAIT(pDefender, MARTIALARTS)) {
          // the knife gets in the way but we're still better than nobody
          iDefRating +=
              (gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pDefender, MARTIALARTS)) / 3;
        }
      } else {
        if (HAS_SKILL_TRAIT(pDefender, KNIFING)) {
          // good with knives, don't have one, but we know a bit about dodging
          iDefRating += (gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pDefender, KNIFING)) / 3;
        }
        if (HAS_SKILL_TRAIT(pDefender, MARTIALARTS)) {
          // bonus for dodging knives
          iDefRating +=
              (gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pDefender, MARTIALARTS)) / 2;
        }
      }
    } else {  // punch/hand-to-hand/martial arts attack/steal
      if (Item[pDefender->inv[HANDPOS].usItem].usItemClass == IC_BLADE &&
          ubMode != HTH_MODE_STEAL) {
        if (HAS_SKILL_TRAIT(pDefender, KNIFING)) {
          // with our knife, we get some bonus at defending from HTH attacks
          iDefRating += (gbSkillTraitBonus[KNIFING] * NUM_SKILL_TRAITS(pDefender, KNIFING)) / 2;
        }
      } else {
        if (HAS_SKILL_TRAIT(pDefender, MARTIALARTS)) {
          iDefRating += gbSkillTraitBonus[MARTIALARTS] * NUM_SKILL_TRAITS(pDefender, MARTIALARTS);
        }
        if (HAS_SKILL_TRAIT(pDefender, HANDTOHAND)) {
          iDefRating += gbSkillTraitBonus[HANDTOHAND] * NUM_SKILL_TRAITS(pDefender, HANDTOHAND);
        }
      }
    }
  }

  if (iDefRating < 1) iDefRating = 1;

  // NumMessage("CalcChanceToStab - Attacker's Rating = ",iAttRating);
  // NumMessage("CalcChanceToStab - Defender's Rating = ",iDefRating);

  // calculate chance to hit by comparing the 2 opponent's ratings
  //  iChance = (100 * iAttRating) / (iAttRating + iDefRating);

  if (ubMode == HTH_MODE_STEAL) {
    // make this more extreme so that weak people have a harder time stealing from
    // the stronger
    iChance = 50 * iAttRating / iDefRating;
  } else {
    // Changed from DG by CJC to give higher chances of hitting with a stab or punch
    iChance = 67 + (iAttRating - iDefRating) / 3;

    if (pAttacker->bAimShotLocation == AIM_SHOT_HEAD) {
      // make this harder!
      iChance -= 20;
    }
  }

  // MAKE SURE CHANCE TO HIT IS WITHIN DEFINED LIMITS
  if (iChance < MINCHANCETOHIT) {
    iChance = MINCHANCETOHIT;
  } else {
    if (iChance > MAXCHANCETOHIT) iChance = MAXCHANCETOHIT;
  }

  // NumMessage("ChanceToStab = ",chance);

  return (iChance);
}

uint32_t CalcChanceToStab(struct SOLDIERTYPE *pAttacker, struct SOLDIERTYPE *pDefender,
                          uint8_t ubAimTime) {
  return (CalcChanceHTH(pAttacker, pDefender, ubAimTime, HTH_MODE_STAB));
}

uint32_t CalcChanceToPunch(struct SOLDIERTYPE *pAttacker, struct SOLDIERTYPE *pDefender,
                           uint8_t ubAimTime) {
  return (CalcChanceHTH(pAttacker, pDefender, ubAimTime, HTH_MODE_PUNCH));
}

uint32_t CalcChanceToSteal(struct SOLDIERTYPE *pAttacker, struct SOLDIERTYPE *pDefender,
                           uint8_t ubAimTime) {
  return (CalcChanceHTH(pAttacker, pDefender, ubAimTime, HTH_MODE_STEAL));
}

void ReloadWeapon(struct SOLDIERTYPE *pSoldier, uint8_t ubHandPos) {
  // NB this is a cheat function, don't award experience

  if (pSoldier->inv[ubHandPos].usItem != NOTHING) {
    pSoldier->inv[ubHandPos].ubGunShotsLeft = Weapon[pSoldier->inv[ubHandPos].usItem].ubMagSize;
    // Dirty Bars
    DirtyMercPanelInterface(pSoldier, DIRTYLEVEL1);
  }
}

BOOLEAN IsGunBurstCapable(struct SOLDIERTYPE *pSoldier, uint8_t ubHandPos, BOOLEAN fNotify) {
  BOOLEAN fCapable = FALSE;

  if (pSoldier->inv[ubHandPos].usItem != NOTHING) {
    // ATE: Check for being a weapon....
    if (Item[pSoldier->inv[ubHandPos].usItem].usItemClass & IC_WEAPON) {
      if (Weapon[pSoldier->inv[ubHandPos].usItem].ubShotsPerBurst > 0) {
        fCapable = TRUE;
      }
    }
  }

  if (fNotify && !fCapable) {
    ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, Message[STR_NOT_BURST_CAPABLE],
              pSoldier->name);
  }

  return (fCapable);
}

int32_t CalcMaxTossRange(struct SOLDIERTYPE *pSoldier, uint16_t usItem, BOOLEAN fArmed) {
  int32_t iRange;
  uint16_t usSubItem;

  if (EXPLOSIVE_GUN(usItem)) {
    // oops! return value in weapons table
    return (Weapon[usItem].usRange / CELL_X_SIZE);
  }

  // if item's fired mechanically
  // ATE: If we are sent in a LAUNCHABLE, get the LAUCNHER, and sub ONLY if we are armed...
  usSubItem = GetLauncherFromLaunchable(usItem);

  if (fArmed && usSubItem != NOTHING) {
    usItem = usSubItem;
  }

  if (Item[usItem].usItemClass == IC_LAUNCHER && fArmed) {
    // this function returns range in tiles so, stupidly, we have to divide by 10 here
    iRange = Weapon[usItem].usRange / CELL_X_SIZE;
  } else {
    if (Item[usItem].fFlags & ITEM_UNAERODYNAMIC) {
      iRange = 1;
    } else if (Item[usItem].usItemClass == IC_GRENADE) {
      // start with the range based on the soldier's strength and the item's weight
      int32_t iThrowingStrength = (EffectiveStrength(pSoldier) * 2 + 100) / 3;
      iRange = 2 + (iThrowingStrength / min((3 + (Item[usItem].ubWeight) / 3), 4));
    } else {  // not as aerodynamic!

      // start with the range based on the soldier's strength and the item's weight
      iRange = 2 + ((EffectiveStrength(pSoldier) / (5 + Item[usItem].ubWeight)));
    }

    // adjust for thrower's remaining breath (lose up to 1/2 of range)
    iRange -= (iRange * (100 - pSoldier->bBreath)) / 200;

    if (HAS_SKILL_TRAIT(pSoldier, THROWING)) {
      // better max range due to expertise
      iRange =
          iRange * (100 + gbSkillTraitBonus[THROWING] * NUM_SKILL_TRAITS(pSoldier, THROWING)) / 100;
    }
  }

  if (iRange < 1) {
    iRange = 1;
  }

  return (iRange);
}

uint32_t CalcThrownChanceToHit(struct SOLDIERTYPE *pSoldier, int16_t sGridNo, uint8_t ubAimTime,
                               uint8_t ubAimPos) {
  int32_t iChance, iMaxRange, iRange;
  uint16_t usHandItem;
  int8_t bPenalty, bBandaged;

  if (pSoldier->bWeaponMode == WM_ATTACHED) {
    usHandItem = UNDER_GLAUNCHER;
  } else {
    usHandItem = pSoldier->inv[HANDPOS].usItem;
  }

  /*
          // CJC: Grenade Launchers don't fire in a straight line!
          #ifdef BETAVERSION
          if (usHandItem == GLAUNCHER)
          {
                  PopMessage("CalcThrownChanceToHit: DOESN'T WORK ON GLAUNCHERs!");
                  return(0);
          }
          #endif
  */

  if (Item[usHandItem].usItemClass != IC_LAUNCHER && pSoldier->bWeaponMode != WM_ATTACHED) {
    // PHYSICALLY THROWN arced projectile (ie. grenade)
    // for lack of anything better, base throwing accuracy on dex & marskmanship
    iChance = (EffectiveDexterity(pSoldier) + EffectiveMarksmanship(pSoldier)) / 2;
    // throwing trait helps out
    if (HAS_SKILL_TRAIT(pSoldier, THROWING)) {
      iChance += gbSkillTraitBonus[THROWING] * NUM_SKILL_TRAITS(pSoldier, THROWING);
    }
  } else {
    // MECHANICALLY FIRED arced projectile (ie. mortar), need brains & know-how
    iChance = (EffectiveDexterity(pSoldier) + EffectiveMarksmanship(pSoldier) +
               EffectiveWisdom(pSoldier) + pSoldier->bExpLevel) /
              4;

    // heavy weapons trait helps out
    if (HAS_SKILL_TRAIT(pSoldier, HEAVY_WEAPS)) {
      iChance += gbSkillTraitBonus[HEAVY_WEAPS] * NUM_SKILL_TRAITS(pSoldier, HEAVY_WEAPS);
    }
  }

  // modify based on morale
  iChance += GetMoraleModifier(pSoldier);

  // modify by fatigue
  iChance -= GetSkillCheckPenaltyForFatigue(pSoldier, iChance);

  // if shooting same target from same position as the last shot
  if (sGridNo == pSoldier->sLastTarget) {
    iChance += AIM_BONUS_SAME_TARGET;  // give a bonus to hit
  }

  // ADJUST FOR EXTRA AIMING TIME
  if (ubAimTime) {
    iChance += (AIM_BONUS_PER_AP * ubAimTime);  // bonus for every pt of aiming
  }

  /*
          if (!pSoldier->human)	// if this is a computer AI controlled enemy
          {
                  iChance += Diff[DIFF_ENEMY_TO_HIT_MOD][GameOption[ENEMYDIFFICULTY]];
          }
  */

  // if shooter is being affected by gas
  if (pSoldier->uiStatusFlags & SOLDIER_GASSED) {
    iChance -= AIM_PENALTY_GASSED;
  }

  // if shooter is being bandaged at the same time, his concentration is off
  if (pSoldier->ubServiceCount > 0) {
    iChance -= AIM_PENALTY_GETTINGAID;
  }

  // if shooter is still in shock
  if (pSoldier->bShock) {
    iChance -= (pSoldier->bShock * AIM_PENALTY_PER_SHOCK);
  }

  // calculate actual range (in world units)
  iRange = (int16_t)GetRangeInCellCoordsFromGridNoDiff(pSoldier->sGridNo, sGridNo);

  // NumMessage("ACTUAL RANGE = ",range);

  if (pSoldier->inv[HEAD1POS].usItem == SUNGOGGLES ||
      pSoldier->inv[HEAD2POS].usItem == SUNGOGGLES) {
    // decrease effective range by 10% when using sungoggles (w or w/o scope)
    iRange -= iRange / 10;  // basically, +1% to hit per every 2 squares
  }

  // NumMessage("EFFECTIVE RANGE = ",range);

  // ADJUST FOR RANGE

  if (usHandItem == MORTAR && iRange < MIN_MORTAR_RANGE) {
    return (0);
  } else {
    iMaxRange = CalcMaxTossRange(pSoldier, usHandItem, TRUE) * CELL_X_SIZE;

    // NumMessage("MAX RANGE = ",maxRange);

    // bonus if range is less than 1/2 maximum range, penalty if it's more

    // bonus is 50% at range 0, -50% at maximum range

    iChance += 50 * 2 * ((iMaxRange / 2) - iRange) / iMaxRange;
    // iChance += ((iMaxRange / 2) - iRange);		// increments of 1% per pixel

    // IF TARGET IS BEYOND MAXIMUM THROWING RANGE
    if (iRange > iMaxRange) {
      // the object CAN travel that far if not blocked, but it's NOT accurate!
      iChance /= 2;
    }
  }

  // IF CHANCE EXISTS, BUT ATTACKER IS INJURED
  if ((iChance > 0) && (pSoldier->bLife < pSoldier->bLifeMax)) {
    // if bandaged, give 1/2 of the bandaged life points back into equation
    bBandaged = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;

    // injury penalty is based on % damage taken (max 2/3rds iChance)
    bPenalty = (2 * iChance * (pSoldier->bLifeMax - pSoldier->bLife + (bBandaged / 2))) /
               (3 * pSoldier->bLifeMax);

    // for mechanically-fired projectiles, reduce penalty in half
    if (Item[usHandItem].usItemClass == IC_LAUNCHER) {
      bPenalty /= 2;
    }

    // reduce injury penalty due to merc's experience level (he can take it!)
    iChance -= (bPenalty * (100 - (10 * (EffectiveExpLevel(pSoldier) - 1)))) / 100;
  }

  // IF CHANCE EXISTS, BUT ATTACKER IS LOW ON BREATH
  if ((iChance > 0) && (pSoldier->bBreath < 100)) {
    // breath penalty is based on % breath missing (max 1/2 iChance)
    bPenalty = (iChance * (100 - pSoldier->bBreath)) / 200;

    // for mechanically-fired projectiles, reduce penalty in half
    if (Item[usHandItem].usItemClass == IC_LAUNCHER) bPenalty /= 2;

    // reduce breath penalty due to merc's dexterity (he can compensate!)
    iChance -= (bPenalty * (100 - (EffectiveDexterity(pSoldier) - 10))) / 100;
  }

  // if iChance exists, but it's a mechanical item being used
  if ((iChance > 0) && (Item[usHandItem].usItemClass == IC_LAUNCHER))
    // reduce iChance to hit DIRECTLY by the item's working condition
    iChance = (iChance * WEAPON_STATUS_MOD(pSoldier->inv[HANDPOS].bStatus[0])) / 100;

  // MAKE SURE CHANCE TO HIT IS WITHIN DEFINED LIMITS
  if (iChance < MINCHANCETOHIT)
    iChance = MINCHANCETOHIT;
  else {
    if (iChance > MAXCHANCETOHIT) iChance = MAXCHANCETOHIT;
  }

  // NumMessage("ThrownChanceToHit = ",iChance);
  return (iChance);
}

void ChangeWeaponMode(struct SOLDIERTYPE *pSoldier) {
  // ATE: Don't do this if in a fire amimation.....
  if (gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_FIRE) {
    return;
  }

  if (FindAttachment(&(pSoldier->inv[HANDPOS]), UNDER_GLAUNCHER) == ITEM_NOT_FOUND ||
      FindLaunchableAttachment(&(pSoldier->inv[HANDPOS]), UNDER_GLAUNCHER) == ITEM_NOT_FOUND) {
    // swap between single/burst fire
    if (IsGunBurstCapable(pSoldier, HANDPOS, TRUE)) {
      pSoldier->bWeaponMode++;
      if (pSoldier->bWeaponMode > WM_BURST) {
        // return to normal mode after going past burst
        pSoldier->bWeaponMode = WM_NORMAL;
      }
    } else {
      // do nothing
      return;
    }
  } else {
    // grenade launcher available, makes things more complicated
    pSoldier->bWeaponMode++;
    if (pSoldier->bWeaponMode == NUM_WEAPON_MODES) {
      // return to the beginning
      pSoldier->bWeaponMode = WM_NORMAL;
    } else {
      // do NOT give message that gun is burst capable, because if we skip past
      // burst capable then we are going on to the grenade launcher
      if (pSoldier->bWeaponMode == WM_BURST && !(IsGunBurstCapable(pSoldier, HANDPOS, FALSE))) {
        // skip past that mode!
        pSoldier->bWeaponMode++;
      }
    }
  }

  if (pSoldier->bWeaponMode == WM_BURST) {
    pSoldier->bDoBurst = TRUE;
  } else {
    pSoldier->bDoBurst = FALSE;
  }
  DirtyMercPanelInterface(pSoldier, DIRTYLEVEL2);
  gfUIForceReExamineCursorData = TRUE;
}

void DishoutQueenSwipeDamage(struct SOLDIERTYPE *pQueenSoldier) {
  int8_t bValidDishoutDirs[3][3] = {{NORTH, NORTHEAST, -1}, {EAST, SOUTHEAST, -1}, {SOUTH, -1, -1}};

  uint32_t cnt, cnt2;
  struct SOLDIERTYPE *pSoldier;
  int8_t bDir;
  int32_t iChance;
  int32_t iImpact;
  int32_t iHitBy;

  // Loop through all mercs and make go
  for (cnt = 0; cnt < guiNumMercSlots; cnt++) {
    pSoldier = MercSlots[cnt];

    if (pSoldier != NULL) {
      if (pSoldier->ubID != pQueenSoldier->ubID) {
        // ATE: Ok, lets check for some basic things here!
        if (pSoldier->bLife >= OKLIFE && pSoldier->sGridNo != NOWHERE && IsSolActive(pSoldier) &&
            pSoldier->bInSector) {
          // Get Pyth spaces away....
          if (GetRangeInCellCoordsFromGridNoDiff(pQueenSoldier->sGridNo, pSoldier->sGridNo) <=
              Weapon[CREATURE_QUEEN_TENTACLES].usRange) {
            // get direction
            bDir = (int8_t)GetDirectionFromGridNo(pSoldier->sGridNo, pQueenSoldier);

            //
            for (cnt2 = 0; cnt2 < 2; cnt2++) {
              if (bValidDishoutDirs[pQueenSoldier->uiPendingActionData1][cnt2] == bDir) {
                iChance = CalcChanceToStab(pQueenSoldier, pSoldier, 0);

                // CC: Look here for chance to hit, damage, etc...
                // May want to not hit if target is prone, etc....
                iHitBy = iChance - (int32_t)PreRandom(100);
                if (iHitBy > 0) {
                  // Hit!
                  iImpact = HTHImpact(pQueenSoldier, pSoldier, iHitBy, TRUE);
                  EVENT_SoldierGotHit(pSoldier, CREATURE_QUEEN_TENTACLES, (int16_t)iImpact,
                                      (int16_t)iImpact, gOppositeDirection[bDir], 50,
                                      pQueenSoldier->ubID, 0, ANIM_CROUCH, 0, 0);
                }
              }
            }
          }
        }
      }
    }
  }

  pQueenSoldier->uiPendingActionData1++;
}

BOOLEAN WillExplosiveWeaponFail(struct SOLDIERTYPE *pSoldier, struct OBJECTTYPE *pObj) {
  if (pSoldier->bTeam == gbPlayerNum || pSoldier->bVisible == 1) {
    if ((int8_t)(PreRandom(40) + PreRandom(40)) > pObj->bStatus[0]) {
      // Do second dice roll
      if (PreRandom(2) == 1) {
        // Fail
        return (TRUE);
      }
    }
  }

  return (FALSE);
}
