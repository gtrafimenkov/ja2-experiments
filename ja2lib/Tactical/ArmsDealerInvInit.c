// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Tactical/ArmsDealerInvInit.h"

#include "Laptop/BobbyR.h"
#include "Laptop/LaptopSave.h"
#include "SGP/Debug.h"
#include "SGP/Random.h"
#include "SGP/Types.h"
#include "Tactical/ArmsDealerInit.h"
#include "Tactical/Campaign.h"
#include "Tactical/ItemTypes.h"
#include "Tactical/ShopKeeperInterface.h"
#include "Tactical/Weapons.h"

extern int8_t gbSelectedArmsDealerID;

// This table controls the order items appear in inventory at BR's and dealers, and which kinds of
// items are sold used
ITEM_SORT_ENTRY DealerItemSortInfo[] = {
    //  item class					weapon class	sold used?
    {IC_GUN, HANDGUNCLASS, TRUE},
    {IC_GUN, SHOTGUNCLASS, TRUE},
    {IC_GUN, SMGCLASS, TRUE},
    {IC_GUN, RIFLECLASS, TRUE},
    {IC_GUN, MGCLASS, FALSE},
    {IC_LAUNCHER, NOGUNCLASS, FALSE},
    {IC_AMMO, NOGUNCLASS, FALSE},
    {IC_GRENADE, NOGUNCLASS, FALSE},
    {IC_BOMB, NOGUNCLASS, FALSE},
    {IC_BLADE, NOGUNCLASS, FALSE},
    {IC_THROWING_KNIFE, NOGUNCLASS, FALSE},
    {IC_PUNCH, NOGUNCLASS, FALSE},
    {IC_ARMOUR, NOGUNCLASS, TRUE},
    {IC_FACE, NOGUNCLASS, TRUE},
    {IC_MEDKIT, NOGUNCLASS, FALSE},
    {IC_KIT, NOGUNCLASS, FALSE},
    {IC_MISC, NOGUNCLASS, TRUE},
    {IC_THROWN, NOGUNCLASS, FALSE},
    {IC_KEY, NOGUNCLASS, FALSE},

    // marks end of list
    {
        IC_NONE,
        NOGUNCLASS,
    },
};

//
// Setup the inventory arrays for each of the arms dealers
//
//	The arrays are composed of pairs of numbers
//		The first is the item index
//		The second is the amount of the items the dealer will try to keep in his inventory

//
// Tony ( Weapons only )
//

DEALER_POSSIBLE_INV gTonyInventory[] = {
    // Rare guns/ammo that Tony will buy although he won't ever sell them
    {ROCKET_RIFLE, 0},
    {AUTO_ROCKET_RIFLE, 0},
    {AUTOMAG_III, 0},
    //	{ FLAMETHROWER,					0 },

    // Weapons
    {GLOCK_17, 1},    /* Glock 17        */
    {GLOCK_18, 1},    /* Glock 18        */
    {BERETTA_92F, 1}, /* Beretta 92F     */
    {BERETTA_93R, 1}, /* Beretta 93R     */
    {SW38, 1},        /* .38 S&W Special */
    {BARRACUDA, 1},   /* .357 Barracuda  */
    {DESERTEAGLE, 1}, /* .357 DesertEagle*/
    {M1911, 1},       /* .45 M1911			 */
    {MP5K, 1},        /* H&K MP5K      	 */
    {MAC10, 1},       /* .45 MAC-10	     */

    {THOMPSON, 1}, /* Thompson M1A1   */
    {COMMANDO, 1}, /* Colt Commando   */
    {MP53, 1},     /* H&K MP53		 		 */
    {AKSU74, 1},   /* AKSU-74         */
    {TYPE85, 1},   /* Type-85         */
    {SKS, 1},      /* SKS             */
    {DRAGUNOV, 1}, /* Dragunov        */
    {M24, 1},      /* M24             */
    {AUG, 1},      /* Steyr AUG       */

    {G41, 1},    /* H&K G41         */
    {MINI14, 1}, /* Ruger Mini-14   */
    {C7, 1},     /* C-7             */
    {FAMAS, 1},  /* FA-MAS          */
    {AK74, 1},   /* AK-74           */
    {AKM, 1},    /* AKM             */
    {M14, 1},    /* M-14            */
    {G3A3, 1},   /* H&K G3A3        */
    {FNFAL, 1},  /* FN-FAL          */

    {MINIMI, 1},
    {RPK74, 1},
    {HK21E, 1},

    {M870, 1},   /* Remington M870  */
    {SPAS15, 1}, /* SPAS-15         */

    {GLAUNCHER, 1},       /* grenade launcher*/
    {UNDER_GLAUNCHER, 1}, /* underslung g.l. */
    {ROCKET_LAUNCHER, 1}, /* rocket Launcher */
    {MORTAR, 1},

    // SAP guns
    {G11, 1},
    {CAWS, 1},
    {P90, 1},

    {DART_GUN, 1},

    // Ammo
    {CLIP9_15, 8},
    {CLIP9_30, 6},
    {CLIP9_15_AP, 3}, /* CLIP9_15_AP */
    {CLIP9_30_AP, 3}, /* CLIP9_30_AP */
    {CLIP9_15_HP, 3}, /* CLIP9_15_HP */
    {CLIP9_30_HP, 3}, /* CLIP9_30_HP */

    {CLIP38_6, 10},   /* CLIP38_6 */
    {CLIP38_6_AP, 5}, /* CLIP38_6_AP */
    {CLIP38_6_HP, 5}, /* CLIP38_6_HP */

    {CLIP45_7, 6},
    /* CLIP45_7 */  // 70

    {CLIP45_30, 8},    /* CLIP45_30 */
    {CLIP45_7_AP, 3},  /* CLIP45_7_AP */
    {CLIP45_30_AP, 3}, /* CLIP45_30_AP */
    {CLIP45_7_HP, 3},  /* CLIP45_7_HP */
    {CLIP45_30_HP, 3}, /* CLIP45_30_HP */

    {CLIP357_6, 6},    /* CLIP357_6 */
    {CLIP357_9, 5},    /* CLIP357_9 */
    {CLIP357_6_AP, 3}, /* CLIP357_6_AP */
    {CLIP357_9_AP, 3}, /* CLIP357_9_AP */
    {CLIP357_6_HP, 3},
    /* CLIP357_6_HP */  // 80
    {CLIP357_9_HP, 3},  /* CLIP357_9_HP */

    {CLIP545_30_AP, 6}, /* CLIP545_30_AP */
    {CLIP545_30_HP, 3}, /* CLIP545_30_HP */

    {CLIP556_30_AP, 6}, /* CLIP556_30_AP */
    {CLIP556_30_HP, 3}, /* CLIP556_30_HP */

    {CLIP762W_10_AP, 6}, /* CLIP762W_10_AP */
    {CLIP762W_30_AP, 5}, /* CLIP762W_30_AP */
    {CLIP762W_10_HP, 3}, /* CLIP762W_10_HP */
    {CLIP762W_30_HP, 3}, /* CLIP762W_30_HP */

    {CLIP762N_5_AP, 8},
    /* CLIP762N_5_AP */  // 90
    {CLIP762N_20_AP, 5}, /* CLIP762N_20_AP */
    {CLIP762N_5_HP, 3},  /* CLIP762N_5_HP */
    {CLIP762N_20_HP, 3}, /* CLIP762N_20_HP */

    {CLIP47_50_SAP, 5}, /* CLIP47_50_SAP */

    {CLIP57_50_AP, 6}, /* CLIP57_50_AP */
    {CLIP57_50_HP, 3}, /* CLIP57_50_HP */

    {CLIP12G_7, 9},          /* CLIP12G_7 */
    {CLIP12G_7_BUCKSHOT, 9}, /* CLIP12G_7_BUCKSHOT */

    {CLIPCAWS_10_SAP, 5}, /* CLIPCAWS_10_SAP */
    {CLIPCAWS_10_FLECH, 3},
    /* CLIPCAWS_10_FLECH */  // 100

    {CLIPROCKET_AP, 3},
    {CLIPROCKET_HE, 1},
    {CLIPROCKET_HEAT, 1},

    {CLIPDART_SLEEP, 5},

    //	{ CLIPFLAME,						5	},

    // "launchables" (New! From McCain!) - these are basically ammo
    {GL_HE_GRENADE, 2},
    {GL_TEARGAS_GRENADE, 2},
    {GL_STUN_GRENADE, 2},
    {GL_SMOKE_GRENADE, 2},
    {MORTAR_SHELL, 1},

    // knives
    {COMBAT_KNIFE, 3},
    {THROWING_KNIFE, 6},
    {BRASS_KNUCKLES, 1},
    {MACHETE, 1},

    // attachments
    {SILENCER, 3},
    {SNIPERSCOPE, 3},
    {LASERSCOPE, 1},
    {BIPOD, 3},
    {DUCKBILL, 2},

    /*
            // grenades
            { STUN_GRENADE,					5 },
            { TEARGAS_GRENADE,			5 },
            { MUSTARD_GRENADE,			5 },
            { MINI_GRENADE,					5 },
            { HAND_GRENADE,					5 },
            { SMOKE_GRENADE,				5 },
    */

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Devin		( Explosives )
//
DEALER_POSSIBLE_INV gDevinInventory[] = {
    {STUN_GRENADE, 3},
    {TEARGAS_GRENADE, 3},
    {MUSTARD_GRENADE, 2},
    {MINI_GRENADE, 3},
    {HAND_GRENADE, 2},
    {SMOKE_GRENADE, 3},

    {GL_HE_GRENADE, 2},
    {GL_TEARGAS_GRENADE, 2},
    {GL_STUN_GRENADE, 2},
    {GL_SMOKE_GRENADE, 2},
    {MORTAR_SHELL, 1},

    {CLIPROCKET_AP, 1},
    {CLIPROCKET_HE, 1},
    {CLIPROCKET_HEAT, 1},

    {DETONATOR, 10},
    {REMDETONATOR, 5},
    {REMOTEBOMBTRIGGER, 5},

    {MINE, 6},
    {RDX, 5},
    {TNT, 5},
    {C1, 4},
    {HMX, 3},
    {C4, 2},

    {SHAPED_CHARGE, 5},

    //	{	TRIP_FLARE,								2 },
    //	{	TRIP_KLAXON,							2 },

    {GLAUNCHER, 1},       /* grenade launcher*/
    {UNDER_GLAUNCHER, 1}, /* underslung g.l. */
    {ROCKET_LAUNCHER, 1}, /* rocket Launcher */
    {MORTAR, 1},

    {METALDETECTOR, 2},
    {WIRECUTTERS, 1},
    {DUCT_TAPE, 1},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Franz	(Expensive pawn shop )
//
DEALER_POSSIBLE_INV gFranzInventory[] = {
    {NIGHTGOGGLES, 3},

    {LASERSCOPE, 3},
    {METALDETECTOR, 2},
    {EXTENDEDEAR, 2},

    {DART_GUN, 1},

    {KEVLAR_VEST, 1},
    {KEVLAR_LEGGINGS, 1},
    {KEVLAR_HELMET, 1},
    {KEVLAR2_VEST, 1},
    {SPECTRA_VEST, 1},
    {SPECTRA_LEGGINGS, 1},
    {SPECTRA_HELMET, 1},

    {CERAMIC_PLATES, 1},

    {CAMOUFLAGEKIT, 1},

    {VIDEO_CAMERA, 1},  // for robot quest

    {LAME_BOY, 1},
    {FUMBLE_PAK, 1},

    {GOLDWATCH, 1},
    {GOLFCLUBS, 1},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Keith		( Cheap Pawn Shop )
//
DEALER_POSSIBLE_INV gKeithInventory[] = {
    {FIRSTAIDKIT, 5},

    // WARNING: Keith must not carry any guns, it would conflict with his story/quest

    {COMBAT_KNIFE, 2},
    {THROWING_KNIFE, 3},
    {BRASS_KNUCKLES, 1},
    {MACHETE, 1},

    {SUNGOGGLES, 3},
    {FLAK_JACKET, 2},
    {STEEL_HELMET, 3},
    {LEATHER_JACKET, 1},

    {CANTEEN, 5},
    {CROWBAR, 1},
    {JAR, 6},

    {TOOLKIT, 1},
    {GASMASK, 1},

    {SILVER_PLATTER, 1},

    {WALKMAN, 1},
    {PORTABLETV, 1},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Sam		( Hardware )
//
DEALER_POSSIBLE_INV gSamInventory[] = {
    {FIRSTAIDKIT, 3},

    {LOCKSMITHKIT, 4},
    {TOOLKIT, 3},

    {CANTEEN, 5},

    {CROWBAR, 3},
    {WIRECUTTERS, 3},

    {DUCKBILL, 3},
    {JAR, 12},
    {BREAK_LIGHT, 12},  // flares

    {METALDETECTOR, 1},

    {VIDEO_CAMERA, 1},

    {QUICK_GLUE, 3},
    {COPPER_WIRE, 5},
    {BATTERIES, 10},

    {CLIP9_15, 5},
    {CLIP9_30, 5},
    {CLIP38_6, 5},
    {CLIP45_7, 5},
    {CLIP45_30, 5},
    {CLIP357_6, 5},
    {CLIP357_9, 5},
    {CLIP12G_7, 9},
    {CLIP12G_7_BUCKSHOT, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Jake			( Junk )
//
DEALER_POSSIBLE_INV gJakeInventory[] = {
    {FIRSTAIDKIT, 4},
    {MEDICKIT, 3},

    {SW38, 1},
    {CLIP38_6, 5},

    {JAR, 3},
    {CANTEEN, 2},
    {BEER, 6},

    {CROWBAR, 1},
    {WIRECUTTERS, 1},

    {COMBAT_KNIFE, 1},
    {THROWING_KNIFE, 1},
    {BRASS_KNUCKLES, 1},
    {MACHETE, 1},

    {BREAK_LIGHT, 5},  // flares

    {BIPOD, 1},

    {TSHIRT, 6},
    {CIGARS, 3},
    {PORNOS, 1},

    {LOCKSMITHKIT, 1},

    // "new" items, presumed unsafe for demo
    {TSHIRT_DEIDRANNA, 2},
    {XRAY_BULB, 1},

    // additional stuff possible in real game
    {GLOCK_17, 1},    /* Glock 17        */
    {GLOCK_18, 1},    /* Glock 18        */
    {BERETTA_92F, 1}, /* Beretta 92F     */
    {BERETTA_93R, 1}, /* Beretta 93R     */
    {BARRACUDA, 1},   /* .357 Barracuda  */
    {DESERTEAGLE, 1}, /* .357 DesertEagle*/
    {M1911, 1},       /* .45 M1911			 */

    {DISCARDED_LAW, 1},

    {STEEL_HELMET, 1},

    {TOOLKIT, 1},

    {WINE, 1},
    {ALCOHOL, 1},

    {GOLDWATCH, 1},
    {GOLFCLUBS, 1},
    {WALKMAN, 1},
    {PORTABLETV, 1},

    // stuff a real pawn shop wouldn't have, but it does make him a bit more useful
    {COMPOUND18, 1},
    {CERAMIC_PLATES, 1},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Howard		( Pharmaceuticals )
//
DEALER_POSSIBLE_INV gHowardInventory[] = {
    {FIRSTAIDKIT, 10},
    {MEDICKIT, 5},
    {ADRENALINE_BOOSTER, 5},
    {REGEN_BOOSTER, 5},

    {ALCOHOL, 3},
    {COMBAT_KNIFE, 2},

    {CLIPDART_SLEEP, 5},

    {CHEWING_GUM, 3},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Gabby			( Creature parts and Blood )
//
DEALER_POSSIBLE_INV gGabbyInventory[] = {
    {JAR, 12},
    {JAR_ELIXIR, 3},
    // buys these, but can't supply them (player is the only source)
    {JAR_CREATURE_BLOOD, 0},
    {JAR_QUEEN_CREATURE_BLOOD, 0},
    {BLOODCAT_CLAWS, 0},
    {BLOODCAT_TEETH, 0},
    {BLOODCAT_PELT, 0},
    {CREATURE_PART_CLAWS, 0},
    {CREATURE_PART_FLESH, 0},
    {CREATURE_PART_ORGAN, 0},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Frank  ( Alcohol )
//
DEALER_POSSIBLE_INV gFrankInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Elgin  ( Alcohol )
//
DEALER_POSSIBLE_INV gElginInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Manny  ( Alcohol )
//
DEALER_POSSIBLE_INV gMannyInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Herve Santos		( Alcohol )
//
DEALER_POSSIBLE_INV gHerveInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Peter Santos ( Alcohol )
//
DEALER_POSSIBLE_INV gPeterInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Alberto Santos		( Alcohol )
//
DEALER_POSSIBLE_INV gAlbertoInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Carlo Santos		( Alcohol )
//
DEALER_POSSIBLE_INV gCarloInventory[] = {
    {BEER, 12},
    {WINE, 6},
    {ALCOHOL, 9},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Micky	( BUYS Animal / Creature parts )
//

DEALER_POSSIBLE_INV gMickyInventory[] = {
    // ONLY BUYS THIS STUFF, DOESN'T SELL IT
    {BLOODCAT_CLAWS, 0},
    {BLOODCAT_TEETH, 0},
    {BLOODCAT_PELT, 0},
    {CREATURE_PART_CLAWS, 0},
    {CREATURE_PART_FLESH, 0},
    {CREATURE_PART_ORGAN, 0},
    {JAR_QUEEN_CREATURE_BLOOD, 0},

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Arnie		( Weapons REPAIR )
//
DEALER_POSSIBLE_INV gArnieInventory[] = {
    // NO INVENTORY

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Perko			( REPAIR)
//
DEALER_POSSIBLE_INV gPerkoInventory[] = {
    // NO INVENTORY

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

//
// Fredo			( Electronics REPAIR)
//
DEALER_POSSIBLE_INV gFredoInventory[] = {
    // NO INVENTORY

    {LAST_DEALER_ITEM, NO_DEALER_ITEM},  // Last One
};

// prototypes

int8_t GetMaxItemAmount(DEALER_POSSIBLE_INV *pInv, uint16_t usItemIndex);

int8_t GetDealersMaxItemAmount(uint8_t ubDealerID, uint16_t usItemIndex) {
  switch (ubDealerID) {
    case ARMS_DEALER_TONY:
      return (GetMaxItemAmount(gTonyInventory, usItemIndex));
      break;

    case ARMS_DEALER_FRANK:
      return (GetMaxItemAmount(gFrankInventory, usItemIndex));
      break;

    case ARMS_DEALER_MICKY:
      return (GetMaxItemAmount(gMickyInventory, usItemIndex));
      break;

    case ARMS_DEALER_ARNIE:
      return (GetMaxItemAmount(gArnieInventory, usItemIndex));
      break;

    case ARMS_DEALER_PERKO:
      return (GetMaxItemAmount(gPerkoInventory, usItemIndex));
      break;

    case ARMS_DEALER_KEITH:
      return (GetMaxItemAmount(gKeithInventory, usItemIndex));
      break;

    case ARMS_DEALER_BAR_BRO_1:
      return (GetMaxItemAmount(gHerveInventory, usItemIndex));
      break;

    case ARMS_DEALER_BAR_BRO_2:
      return (GetMaxItemAmount(gPeterInventory, usItemIndex));
      break;

    case ARMS_DEALER_BAR_BRO_3:
      return (GetMaxItemAmount(gAlbertoInventory, usItemIndex));
      break;

    case ARMS_DEALER_BAR_BRO_4:
      return (GetMaxItemAmount(gCarloInventory, usItemIndex));
      break;

    case ARMS_DEALER_JAKE:
      return (GetMaxItemAmount(gJakeInventory, usItemIndex));
      break;

    case ARMS_DEALER_FRANZ:
      return (GetMaxItemAmount(gFranzInventory, usItemIndex));
      break;

    case ARMS_DEALER_HOWARD:
      return (GetMaxItemAmount(gHowardInventory, usItemIndex));
      break;

    case ARMS_DEALER_SAM:
      return (GetMaxItemAmount(gSamInventory, usItemIndex));
      break;

    case ARMS_DEALER_FREDO:
      return (GetMaxItemAmount(gFredoInventory, usItemIndex));
      break;

    case ARMS_DEALER_GABBY:
      return (GetMaxItemAmount(gGabbyInventory, usItemIndex));
      break;

    case ARMS_DEALER_DEVIN:
      return (GetMaxItemAmount(gDevinInventory, usItemIndex));
      break;

    case ARMS_DEALER_ELGIN:
      return (GetMaxItemAmount(gElginInventory, usItemIndex));
      break;

    case ARMS_DEALER_MANNY:
      return (GetMaxItemAmount(gMannyInventory, usItemIndex));
      break;

    default:
      Assert(FALSE);
      return (0);
      break;
  }
}

int8_t GetMaxItemAmount(DEALER_POSSIBLE_INV *pInv, uint16_t usItemIndex) {
  uint16_t usCnt = 0;

  // loop through the array until a the LAST_DEALER_ITEM is hit
  while (pInv[usCnt].sItemIndex != LAST_DEALER_ITEM) {
    // if this item is the one we want
    if (pInv[usCnt].sItemIndex == usItemIndex) return (pInv[usCnt].ubOptimalNumber);

    // move to the next item
    usCnt++;
  }

  return (NO_DEALER_ITEM);
}

DEALER_POSSIBLE_INV *GetPointerToDealersPossibleInventory(uint8_t ubArmsDealerID) {
  switch (ubArmsDealerID) {
    case ARMS_DEALER_TONY:
      return (gTonyInventory);
      break;

    case ARMS_DEALER_FRANK:
      return (gFrankInventory);
      break;

    case ARMS_DEALER_MICKY:
      return (gMickyInventory);
      break;

    case ARMS_DEALER_ARNIE:
      return (gArnieInventory);
      break;

    case ARMS_DEALER_PERKO:
      return (gPerkoInventory);
      break;

    case ARMS_DEALER_KEITH:
      return (gKeithInventory);
      break;

    case ARMS_DEALER_BAR_BRO_1:
      return (gHerveInventory);
      break;

    case ARMS_DEALER_BAR_BRO_2:
      return (gPeterInventory);
      break;

    case ARMS_DEALER_BAR_BRO_3:
      return (gAlbertoInventory);
      break;

    case ARMS_DEALER_BAR_BRO_4:
      return (gCarloInventory);
      break;

    case ARMS_DEALER_JAKE:
      return (gJakeInventory);
      break;

    case ARMS_DEALER_FRANZ:
      return (gFranzInventory);
      break;

    case ARMS_DEALER_HOWARD:
      return (gHowardInventory);
      break;

    case ARMS_DEALER_SAM:
      return (gSamInventory);
      break;

    case ARMS_DEALER_FREDO:
      return (gFredoInventory);
      break;

    case ARMS_DEALER_GABBY:
      return (gGabbyInventory);
      break;

    case ARMS_DEALER_DEVIN:
      return (gDevinInventory);
      break;

    case ARMS_DEALER_ELGIN:
      return (gElginInventory);
      break;

    case ARMS_DEALER_MANNY:
      return (gMannyInventory);
      break;

    default:
      return (NULL);
  }
}

uint8_t GetCurrentSuitabilityForItem(int8_t bArmsDealer, uint16_t usItemIndex) {
  uint8_t ubItemCoolness;
  uint8_t ubMinCoolness, ubMaxCoolness;

  // item suitability varies with the player's maximum progress through the game.  The farther he
  // gets, the better items we make available.  Weak items become more and more infrequent later in
  // the game, although they never quite vanish.

  // items illegal in this game are unsuitable [this checks guns vs. current GunSet!]
  if (!ItemIsLegal(usItemIndex)) {
    return (ITEM_SUITABILITY_NONE);
  }

  // items normally not sold at shops are unsuitable
  if (Item[usItemIndex].fFlags & ITEM_NOT_BUYABLE) {
    return (ITEM_SUITABILITY_NONE);
  }

  ubItemCoolness = Item[usItemIndex].ubCoolness;

  if (ubItemCoolness == 0) {
    // items without a coolness rating can't be sold to the player by shopkeepers
    return (ITEM_SUITABILITY_NONE);
  }

  // the following staple items are always deemed highly suitable regardless of player's progress:
  switch (usItemIndex) {
    case CLIP38_6:
    case CLIP9_15:
    case CLIP9_30:
    case CLIP357_6:
    case CLIP357_9:
    case CLIP45_7:
    case CLIP45_30:
    case CLIP12G_7:
    case CLIP12G_7_BUCKSHOT:
    case CLIP545_30_HP:
    case CLIP556_30_HP:
    case CLIP762W_10_HP:
    case CLIP762W_30_HP:
    case CLIP762N_5_HP:
    case CLIP762N_20_HP:

    case FIRSTAIDKIT:
    case MEDICKIT:
    case TOOLKIT:
    case LOCKSMITHKIT:

    case CANTEEN:
    case CROWBAR:
    case JAR:
    case JAR_ELIXIR:
    case JAR_CREATURE_BLOOD:

      return (ITEM_SUITABILITY_ALWAYS);
  }

  // If it's not BobbyRay, Tony, or Devin
  if ((bArmsDealer != -1) && (bArmsDealer != ARMS_DEALER_TONY) &&
      (bArmsDealer != ARMS_DEALER_DEVIN)) {
    // all the other dealers have very limited inventories, so their suitability remains constant at
    // all times in game
    return (ITEM_SUITABILITY_HIGH);
  }

  // figure out the appropriate range of coolness based on player's maximum progress so far

  ubMinCoolness = HighestPlayerProgressPercentage() / 10;
  ubMaxCoolness = (HighestPlayerProgressPercentage() / 10) + 1;

  // Tony has the better stuff sooner (than Bobby R's)
  if (bArmsDealer == ARMS_DEALER_TONY) {
    ubMinCoolness += 1;
    ubMaxCoolness += 1;
  } else if (bArmsDealer == ARMS_DEALER_DEVIN) {
    // almost everything Devin sells is pretty cool (4+), so gotta apply a minimum or he'd have
    // nothing early on
    if (ubMinCoolness < 3) {
      ubMinCoolness = 3;
      ubMaxCoolness = 4;
    }
  }

  ubMinCoolness = max(1, min(9, ubMinCoolness));
  ubMaxCoolness = max(2, min(10, ubMaxCoolness));

  // if item is too cool for current level of progress
  if (ubItemCoolness > ubMaxCoolness) {
    return (ITEM_SUITABILITY_NONE);
  }

  // if item is exactly within the current coolness window
  if ((ubItemCoolness >= ubMinCoolness) && (ubItemCoolness <= ubMaxCoolness)) {
    return (ITEM_SUITABILITY_HIGH);
  }

  // if item is still relatively close to low end of the window
  if ((ubItemCoolness + 2) >= ubMinCoolness) {
    return (ITEM_SUITABILITY_MEDIUM);
  }

  // item is way uncool for player's current progress, but it's still possible for it to make an
  // appearance
  return (ITEM_SUITABILITY_LOW);
}

uint8_t ChanceOfItemTransaction(int8_t bArmsDealer, uint16_t usItemIndex, BOOLEAN fDealerIsSelling,
                                BOOLEAN fUsed) {
  uint8_t ubItemCoolness;
  uint8_t ubChance = 0;
  BOOLEAN fBobbyRay = FALSE;

  // make sure dealers don't carry used items that they shouldn't
  if (fUsed && !fDealerIsSelling && !CanDealerItemBeSoldUsed(usItemIndex)) return (0);

  if (bArmsDealer == -1) {
    // Bobby Ray has an easier time getting resupplied than the local dealers do
    fBobbyRay = TRUE;
  }

  ubItemCoolness = Item[usItemIndex].ubCoolness;

  switch (GetCurrentSuitabilityForItem(bArmsDealer, usItemIndex)) {
    case ITEM_SUITABILITY_NONE:
      if (fDealerIsSelling) {
        // dealer always gets rid of stuff that is too advanced or inappropriate ASAP
        ubChance = 100;
      } else  // dealer is buying
      {
        // can't get these at all
        ubChance = 0;
      }
      break;

    case ITEM_SUITABILITY_LOW:
      ubChance = (fBobbyRay) ? 25 : 15;
      break;

    case ITEM_SUITABILITY_MEDIUM:
      ubChance = (fBobbyRay) ? 50 : 30;
      break;

    case ITEM_SUITABILITY_HIGH:
      ubChance = (fBobbyRay) ? 75 : 50;
      break;

    case ITEM_SUITABILITY_ALWAYS:
      if (fDealerIsSelling) {
        // sells just like suitability high
        ubChance = 75;
      } else  // dealer is buying
      {
        // dealer can always get a (re)supply of these
        ubChance = 100;
      }
      break;

    default:
      Assert(0);
      break;
  }

  // if there's any uncertainty
  if ((ubChance > 0) && (ubChance < 100)) {
    // cooler items sell faster
    if (fDealerIsSelling) {
      ubChance += (5 * ubItemCoolness);

      // ARM: New - keep stuff on the shelves longer
      ubChance /= 2;
    }

    // used items are traded more rarely
    if (fUsed) {
      ubChance /= 2;
    }
  }

  return (ubChance);
}

BOOLEAN ItemTransactionOccurs(int8_t bArmsDealer, uint16_t usItemIndex, BOOLEAN fDealerIsSelling,
                              BOOLEAN fUsed) {
  uint8_t ubChance;
  int16_t sInventorySlot;

  ubChance = ChanceOfItemTransaction(bArmsDealer, usItemIndex, fDealerIsSelling, fUsed);

  // if the dealer is buying, and a chance exists (i.e. the item is "eligible")
  if (!fDealerIsSelling && (ubChance > 0)) {
    // mark it as such
    if (bArmsDealer == -1) {
      if (fUsed) {
        sInventorySlot =
            GetInventorySlotForItem(LaptopSaveInfo.BobbyRayUsedInventory, usItemIndex, fUsed);
        LaptopSaveInfo.BobbyRayUsedInventory[sInventorySlot].fPreviouslyEligible = TRUE;
      } else {
        sInventorySlot =
            GetInventorySlotForItem(LaptopSaveInfo.BobbyRayInventory, usItemIndex, fUsed);
        LaptopSaveInfo.BobbyRayInventory[sInventorySlot].fPreviouslyEligible = TRUE;
      }
    } else {
      gArmsDealersInventory[bArmsDealer][usItemIndex].fPreviouslyEligible = TRUE;
    }
  }

  // roll to see if a transaction occurs
  if (Random(100) < ubChance) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

uint8_t DetermineInitialInvItems(int8_t bArmsDealerID, uint16_t usItemIndex, uint8_t ubChances,
                                 BOOLEAN fUsed) {
  uint8_t ubNumBought;
  uint8_t ubCnt;

  // initial inventory is now rolled for one item at a time, instead of one type at a time, to
  // improve variety
  ubNumBought = 0;
  for (ubCnt = 0; ubCnt < ubChances; ubCnt++) {
    if (ItemTransactionOccurs(bArmsDealerID, usItemIndex, DEALER_BUYING, fUsed)) {
      ubNumBought++;
    }
  }

  return (ubNumBought);
}

uint8_t HowManyItemsAreSold(int8_t bArmsDealerID, uint16_t usItemIndex, uint8_t ubNumInStock,
                            BOOLEAN fUsed) {
  uint8_t ubNumSold;
  uint8_t ubCnt;

  // items are now virtually "sold" one at a time
  ubNumSold = 0;
  for (ubCnt = 0; ubCnt < ubNumInStock; ubCnt++) {
    if (ItemTransactionOccurs(bArmsDealerID, usItemIndex, DEALER_SELLING, fUsed)) {
      ubNumSold++;
    }
  }

  return (ubNumSold);
}

uint8_t HowManyItemsToReorder(uint8_t ubWanted, uint8_t ubStillHave) {
  uint8_t ubNumReordered;

  Assert(ubStillHave <= ubWanted);

  ubNumReordered = ubWanted - ubStillHave;

  // randomize the amount. 33% of the time we add to it, 33% we subtract from it, rest leave it
  // alone
  switch (Random(3)) {
    case 0:
      ubNumReordered += ubNumReordered / 2;
      break;
    case 1:
      ubNumReordered -= ubNumReordered / 2;
      break;
  }

  return (ubNumReordered);
}

int BobbyRayItemQsortCompare(const void *pArg1, const void *pArg2) {
  uint16_t usItem1Index;
  uint16_t usItem2Index;
  uint8_t ubItem1Quality;
  uint8_t ubItem2Quality;

  usItem1Index = ((STORE_INVENTORY *)pArg1)->usItemIndex;
  usItem2Index = ((STORE_INVENTORY *)pArg2)->usItemIndex;

  ubItem1Quality = ((STORE_INVENTORY *)pArg1)->ubItemQuality;
  ubItem2Quality = ((STORE_INVENTORY *)pArg2)->ubItemQuality;

  return (CompareItemsForSorting(usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality));
}

int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2) {
  uint16_t usItem1Index;
  uint16_t usItem2Index;
  uint8_t ubItem1Quality;
  uint8_t ubItem2Quality;

  usItem1Index = ((INVENTORY_IN_SLOT *)pArg1)->sItemIndex;
  usItem2Index = ((INVENTORY_IN_SLOT *)pArg2)->sItemIndex;

  ubItem1Quality = ((INVENTORY_IN_SLOT *)pArg1)->ItemObject.bStatus[0];
  ubItem2Quality = ((INVENTORY_IN_SLOT *)pArg2)->ItemObject.bStatus[0];

  return (CompareItemsForSorting(usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality));
}

int RepairmanItemQsortCompare(const void *pArg1, const void *pArg2) {
  INVENTORY_IN_SLOT *pInvSlot1;
  INVENTORY_IN_SLOT *pInvSlot2;
  uint32_t uiRepairTime1;
  uint32_t uiRepairTime2;

  pInvSlot1 = (INVENTORY_IN_SLOT *)pArg1;
  pInvSlot2 = (INVENTORY_IN_SLOT *)pArg2;

  Assert(pInvSlot1->sSpecialItemElement != -1);
  Assert(pInvSlot2->sSpecialItemElement != -1);

  uiRepairTime1 = gArmsDealersInventory[gbSelectedArmsDealerID][pInvSlot1->sItemIndex]
                      .SpecialItem[pInvSlot1->sSpecialItemElement]
                      .uiRepairDoneTime;
  uiRepairTime2 = gArmsDealersInventory[gbSelectedArmsDealerID][pInvSlot2->sItemIndex]
                      .SpecialItem[pInvSlot2->sSpecialItemElement]
                      .uiRepairDoneTime;

  // lower reapir time first
  if (uiRepairTime1 < uiRepairTime2) {
    return (-1);
  } else if (uiRepairTime1 > uiRepairTime2) {
    return (1);
  } else {
    return (0);
  }
}

int CompareItemsForSorting(uint16_t usItem1Index, uint16_t usItem2Index, uint8_t ubItem1Quality,
                           uint8_t ubItem2Quality) {
  uint8_t ubItem1Category;
  uint8_t ubItem2Category;
  uint16_t usItem1Price;
  uint16_t usItem2Price;
  uint8_t ubItem1Coolness;
  uint8_t ubItem2Coolness;

  ubItem1Category = GetDealerItemCategoryNumber(usItem1Index);
  ubItem2Category = GetDealerItemCategoryNumber(usItem2Index);

  // lower category first
  if (ubItem1Category < ubItem2Category) {
    return (-1);
  } else if (ubItem1Category > ubItem2Category) {
    return (1);
  } else {
    // the same category
    if (Item[usItem1Index].usItemClass == IC_AMMO && Item[usItem2Index].usItemClass == IC_AMMO) {
      uint8_t ubItem1Calibre;
      uint8_t ubItem2Calibre;
      uint8_t ubItem1MagSize;
      uint8_t ubItem2MagSize;

      // AMMO is sorted by caliber first
      ubItem1Calibre = Magazine[Item[usItem1Index].ubClassIndex].ubCalibre;
      ubItem2Calibre = Magazine[Item[usItem2Index].ubClassIndex].ubCalibre;
      if (ubItem1Calibre > ubItem2Calibre) {
        return (-1);
      } else if (ubItem1Calibre < ubItem2Calibre) {
        return (1);
      }
      // the same caliber - compare size of magazine, then fall out of if statement
      ubItem1MagSize = Magazine[Item[usItem1Index].ubClassIndex].ubMagSize;
      ubItem2MagSize = Magazine[Item[usItem2Index].ubClassIndex].ubMagSize;
      if (ubItem1MagSize > ubItem2MagSize) {
        return (-1);
      } else if (ubItem1MagSize < ubItem2MagSize) {
        return (1);
      }

    } else {
      // items other than ammo are compared on coolness first
      ubItem1Coolness = Item[usItem1Index].ubCoolness;
      ubItem2Coolness = Item[usItem2Index].ubCoolness;

      // higher coolness first
      if (ubItem1Coolness > ubItem2Coolness) {
        return (-1);
      } else if (ubItem1Coolness < ubItem2Coolness) {
        return (1);
      }
    }

    // the same coolness/caliber - compare base prices then
    usItem1Price = Item[usItem1Index].usPrice;
    usItem2Price = Item[usItem2Index].usPrice;

    // higher price first
    if (usItem1Price > usItem2Price) {
      return (-1);
    } else if (usItem1Price < usItem2Price) {
      return (1);
    } else {
      // the same price - compare item #s, then

      // lower index first
      if (usItem1Index < usItem2Index) {
        return (-1);
      } else if (usItem1Index > usItem2Index) {
        return (1);
      } else {
        // same item type = compare item quality, then

        // higher quality first
        if (ubItem1Quality > ubItem2Quality) {
          return (-1);
        } else if (ubItem1Quality < ubItem2Quality) {
          return (1);
        } else {
          // identical items!
          return (0);
        }
      }
    }
  }
}

uint8_t GetDealerItemCategoryNumber(uint16_t usItemIndex) {
  uint32_t uiItemClass;
  uint8_t ubWeaponClass;
  uint8_t ubCategory = 0;

  uiItemClass = Item[usItemIndex].usItemClass;

  if (usItemIndex < MAX_WEAPONS) {
    ubWeaponClass = Weapon[usItemIndex].ubWeaponClass;
  } else {
    // not a weapon, so no weapon class, this won't be needed
    ubWeaponClass = 0;
  }

  ubCategory = 0;

  // search table until end-of-list marker is encountered
  while (DealerItemSortInfo[ubCategory].uiItemClass != IC_NONE) {
    if (DealerItemSortInfo[ubCategory].uiItemClass == uiItemClass) {
      // if not a type of gun
      if (uiItemClass != IC_GUN) {
        // then we're found it
        return (ubCategory);
      } else {
        // for guns, must also match on weapon class
        if (DealerItemSortInfo[ubCategory].ubWeaponClass == ubWeaponClass) {
          // then we're found it
          return (ubCategory);
        }
      }
    }

    // check vs. next category in the list
    ubCategory++;
  }

  // should never be trying to locate an item that's not covered in the table!
  Assert(FALSE);
  return (0);
}

BOOLEAN CanDealerItemBeSoldUsed(uint16_t usItemIndex) {
  if (!(Item[usItemIndex].fFlags & ITEM_DAMAGEABLE)) return (FALSE);

  // certain items, although they're damagable, shouldn't be sold in a used condition
  return (DealerItemSortInfo[GetDealerItemCategoryNumber(usItemIndex)].fAllowUsed);
}
