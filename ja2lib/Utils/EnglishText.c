// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/Text.h"

/*

******************************************************************************************************
**                                  IMPORTANT TRANSLATION NOTES **
******************************************************************************************************

GENERAL INSTRUCTIONS
- Always be aware that foreign strings should be of equal or shorter length than the English
equivalent. I know that this is difficult to do on many occasions due to the nature of foreign
languages when compared to English.  By doing so, this will greatly reduce the amount of work on
both sides.  In most cases (but not all), JA2 interfaces were designed with just enough space to fit
the English word. The general rule is if the string is very short (less than 10 characters), then
it's short because of interface limitations.  On the other hand, full sentences commonly have little
limitations for length. Strings in between are a little dicey.
- Never translate a string to appear on multiple lines.  All strings L"This is a really long
string...", must fit on a single line no matter how long the string is.  All strings start with L"
and end with ",
- Never remove any extra spaces in strings.  In addition, all strings containing multiple sentences
only have one space after a period, which is different than standard typing convention.  Never
modify sections of strings contain combinations of % characters.  These are special format
characters and are always used in conjunction with other characters.  For example, %s means string,
and is commonly used for names, locations, items, etc.  %d is used for numbers.  %c%d is a character
and a number (such as A9).
        %% is how a single % character is built.  There are countless types, but strings containing
these special characters are usually commented to explain what they mean.  If it isn't commented,
then if you can't figure out the context, then feel free to ask SirTech.
- Comments are always started with // Anything following these two characters on the same line are
        considered to be comments.  Do not translate comments.  Comments are always applied to the
following string(s) on the next line(s), unless the comment is on the same line as a string.
- All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation.  By
searching for @@@ everytime you recieve a new version, it will simplify your task and identify
special instructions. Commonly, these types of comments will be used to ask you to abbreviate a
string.  Please leave the comments intact, and SirTech will remove them once the translation for
that particular area is resolved.
- If you have a problem or question with translating certain strings, please use "//!!! comment"
        (without the quotes).  The syntax is important, and should be identical to the comments used
with @@@ symbols.  SirTech will search for !!! to look for your problems and questions.  This is a
more efficient method than detailing questions in email, so try to do this whenever possible.



FAST HELP TEXT -- Explains how the syntax of fast help text works.
**************

1) BOLDED LETTERS
        The popup help text system supports special characters to specify the hot key(s) for a
button. Anytime you see a '|' symbol within the help text string, that means the following key is
assigned to activate the action which is usually a button.

        EX:  L"|Map Screen"

        This means the 'M' is the hotkey.  In the game, when somebody hits the 'M' key, it activates
that button.  When translating the text to another language, it is best to attempt to choose a word
that uses 'M'.  If you can't always find a match, then the best thing to do is append the 'M' at the
end of the string in this format:

        EX:  L"Ecran De Carte (|M)"  (this is the French translation)

        Other examples are used multiple times, like the Esc key  or "|E|s|c" or Space ->
(|S|p|a|c|e)

2) NEWLINE
  Any place you see a \n within the string, you are looking at another string that is part of the
fast help text system.  \n notation doesn't need to be precisely placed within that string, but
whereever you wish to start a new line.

        EX:  L"Clears all the mercs' positions,\nand allows you to re-enter them manually."

        Would appear as:

                                Clears all the mercs' positions,
                                and allows you to re-enter them manually.

        NOTE:  It is important that you don't pad the characters adjacent to the \n with spaces.  If
we did this in the above example, we would see

        WRONG WAY -- spaces before and after the \n
        EX:  L"Clears all the mercs' positions, \n and allows you to re-enter them manually."

        Would appear as: (the second line is moved in a character)

                                Clears all the mercs' positions,
                                 and allows you to re-enter them manually.


@@@ NOTATION
************

        Throughout the text files, you'll find an assortment of comments.  Comments are used to
describe the text to make translation easier, but comments don't need to be translated.  A good
thing is to search for
        "@@@" after receiving new version of the text file, and address the special notes in this
manner.

!!! NOTATION
************

        As described above, the "!!!" notation should be used by you to ask questions and address
problems as SirTech uses the "@@@" notation.

*/

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
static wchar_t* en_AmmoCaliber[] = {
    L"0",         L".38 cal", L"9mm",    L".45 cal", L".357 cal",
    L"12 gauge",  L"CAWS",    L"5.45mm", L"5.56mm",  L"7.62mm NATO",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monster", L"Rocket",
    L"",  // dart
    L"",  // flame
};

// This BobbyRayAmmoCaliber is virtually the same as AmmoCaliber however the bobby version doesnt
// have as much room for the words.
//
// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
static wchar_t* en_BobbyRayAmmoCaliber[] = {
    L"0",         L".38 cal", L"9mm",    L".45 cal", L".357 cal",
    L"12 gauge",  L"CAWS",    L"5.45mm", L"5.56mm",  L"7.62mm N.",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monster", L"Rocket",
    L"",  // dart
};

static wchar_t* en_WeaponType[] = {
    L"Other",        L"Pistol",        L"Machine pistol",    L"Submachine gun", L"Rifle",
    L"Sniper rifle", L"Assault rifle", L"Light machine gun", L"Shotgun",
};

static wchar_t* en_TeamTurnString[] = {
    L"Player's Turn",  // player's turn
    L"Opponents' Turn", L"Creatures' Turn", L"Militia's Turn", L"Civilians' Turn",
    // planning turn
};

static wchar_t* en_Message[] = {
    L"",

    // In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

    L"%s is hit in the head and loses a point of wisdom!",
    L"%s is hit in the shoulder and loses a point of dexterity!",
    L"%s is hit in the chest and loses a point of strength!",
    L"%s is hit in the legs and loses a point of agility!",
    L"%s is hit in the head and loses %d points of wisdom!",
    L"%s is hit in the shoulder and loses %d points of dexterity!",
    L"%s is hit in the chest and loses %d points of strength!",
    L"%s is hit in the legs and loses %d points of agility!",
    L"Interrupt!",

    // The first %s is a merc's name, the second is a string from pNoiseVolStr,
    // the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

    L"",  // OBSOLETE
    L"Your reinforcements have arrived!",

    // In the following four lines, all %s's are merc names

    L"%s reloads.",
    L"%s doesn't have enough Action Points!",
    L"%s is applying first aid. (Press any key to cancel.)",
    L"%s and %s are applying first aid. (Press any key to cancel.)",
    // the following 17 strings are used to create lists of gun advantages and disadvantages
    // (separated by commas)
    L"reliable",
    L"unreliable",
    L"easy to repair",
    L"hard to repair",
    L"high damage",
    L"low damage",
    L"quick firing",
    L"slow firing",
    L"long range",
    L"short range",
    L"light",
    L"heavy",
    L"small",
    L"fast burst fire",
    L"no burst fire",
    L"large magazine",
    L"small magazine",

    // In the following two lines, all %s's are merc names

    L"%s's camouflage has worn off.",
    L"%s's camouflage has washed off.",

    // The first %s is a merc name and the second %s is an item name

    L"Second weapon is out of ammo!",
    L"%s has stolen the %s.",

    // The %s is a merc name

    L"%s's weapon can't burst fire.",

    L"You've already got one of those attached.",
    L"Merge items?",

    // Both %s's are item names

    L"You can't attach a %s to a %s.",

    L"None",
    L"Eject ammo",
    L"Attachments",

    // You cannot use "item(s)" and your "other item" at the same time.
    // Ex:  You cannot use sun goggles and you gas mask at the same time.
    L"You cannot use %s and your %s at the same time.",

    L"The item you have in your cursor can be attached to certain items by placing it in one of "
    L"the four attachment slots.",
    L"The item you have in your cursor can be attached to certain items by placing it in one of "
    L"the four attachment slots. (However in this case, the item is not compatible.)",
    L"The sector isn't cleared of enemies!",
    L"You still need to give %s %s",
    L"%s is hit in the head!",
    L"Abandon the fight?",
    L"This attachment will be permanent.  Go ahead with it?",
    L"%s feels more energetic!",
    L"%s slipped on some marbles!",
    L"%s failed to grab the %s!",
    L"%s has repaired the %s",
    L"Interrupt for ",
    L"Surrender?",
    L"This person refuses your aid.",
    L"I DON'T think so!",
    L"To travel in Skyrider's chopper, you'll have to ASSIGN mercs to VEHICLE/HELICOPTER first.",
    L"%s only had enough time to reload ONE gun",
    L"Bloodcats' turn",
};

// the names of the towns in the game

static wchar_t* en_pTownNames[] = {
    L"",         L"Omerta", L"Drassen", L"Alma",   L"Grumm",  L"Tixa",     L"Cambria",
    L"San Mona", L"Estoni", L"Orta",    L"Balime", L"Meduna", L"Chitzena",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per
// second, etc. min is an abbreviation for minutes

static wchar_t* en_sTimeStrings[] = {
    L"Paused", L"Normal", L"5 min", L"30 min", L"60 min", L"6 hrs",
};

// Assignment Strings: what assignment does the merc  have right now? For example, are they on a
// squad, training, administering medical aid (doctor) or training a town. All are abbreviated. 8
// letters is the longest it can be.

static wchar_t* en_pAssignmentStrings[] = {
    L"Squad 1",  L"Squad 2",  L"Squad 3",  L"Squad 4",  L"Squad 5",  L"Squad 6",  L"Squad 7",
    L"Squad 8",  L"Squad 9",  L"Squad 10", L"Squad 11", L"Squad 12", L"Squad 13", L"Squad 14",
    L"Squad 15", L"Squad 16", L"Squad 17", L"Squad 18", L"Squad 19", L"Squad 20",
    L"On Duty",   // on active duty
    L"Doctor",    // administering medical aid
    L"Patient",   // getting medical aid
    L"Vehicle",   // in a vehicle
    L"In Trans",  // in transit - abbreviated form
    L"Repair",    // repairing
    L"Practice",  // training themselves
    L"Militia",   // training a town to revolt
    L"Trainer",   // training a teammate
    L"Student",   // being trained by someone else
    L"Dead",      // dead
    L"Incap.",    // abbreviation for incapacitated
    L"POW",       // Prisoner of war - captured
    L"Hospital",  // patient in a hospital
    L"Empty",     // Vehicle is empty
};

static wchar_t* en_pMilitiaString[] = {
    L"Militia",     // the title of the militia box
    L"Unassigned",  // the number of unassigned militia troops
    L"You can't redistribute militia while there are hostilities in the area!",
};

static wchar_t* en_pMilitiaButtonString[] = {
    L"Auto",  // auto place the militia troops for the player
    L"Done",  // done placing militia troops
};

static wchar_t* en_pConditionStrings[] = {
    L"Excellent",    // the state of a soldier .. excellent health
    L"Good",         // good health
    L"Fair",         // fair health
    L"Wounded",      // wounded health
    L"Fatigued",     // tired
    L"Bleeding",     // bleeding to death
    L"Unconscious",  // knocked out
    L"Dying",        // near death
    L"Dead",         // dead
};

static wchar_t* en_pEpcMenuStrings[] = {
    L"On Duty",   // set merc on active duty
    L"Patient",   // set as a patient to receive medical aid
    L"Vehicle",   // tell merc to enter vehicle
    L"Unescort",  // let the escorted character go off on their own
    L"Cancel",    // close this menu
};

// look at pAssignmentString above for comments

static wchar_t* en_pPersonnelAssignmentStrings[] = {
    L"Squad 1",    L"Squad 2",  L"Squad 3",  L"Squad 4",          L"Squad 5",  L"Squad 6",
    L"Squad 7",    L"Squad 8",  L"Squad 9",  L"Squad 10",         L"Squad 11", L"Squad 12",
    L"Squad 13",   L"Squad 14", L"Squad 15", L"Squad 16",         L"Squad 17", L"Squad 18",
    L"Squad 19",   L"Squad 20", L"On Duty",  L"Doctor",           L"Patient",  L"Vehicle",
    L"In Transit", L"Repair",   L"Practice", L"Training Militia", L"Trainer",  L"Student",
    L"Dead",       L"Incap.",   L"POW",      L"Hospital",
    L"Empty",  // Vehicle is empty
};

// refer to above for comments

static wchar_t* en_pLongAssignmentStrings[] = {
    L"Squad 1",    L"Squad 2",  L"Squad 3",  L"Squad 4",       L"Squad 5",        L"Squad 6",
    L"Squad 7",    L"Squad 8",  L"Squad 9",  L"Squad 10",      L"Squad 11",       L"Squad 12",
    L"Squad 13",   L"Squad 14", L"Squad 15", L"Squad 16",      L"Squad 17",       L"Squad 18",
    L"Squad 19",   L"Squad 20", L"On Duty",  L"Doctor",        L"Patient",        L"Vehicle",
    L"In Transit", L"Repair",   L"Practice", L"Train Militia", L"Train Teammate", L"Student",
    L"Dead",       L"Incap.",   L"POW",
    L"Hospital",  // patient in a hospital
    L"Empty",     // Vehicle is empty
};

// the contract options

static wchar_t* en_pContractStrings[] = {
    L"Contract Options:",
    L"",                 // a blank line, required
    L"Offer One Day",    // offer merc a one day contract extension
    L"Offer One Week",   // 1 week
    L"Offer Two Weeks",  // 2 week
    L"Dismiss",          // end merc's contract
    L"Cancel",           // stop showing this menu
};

static wchar_t* en_pPOWStrings[] = {
    L"POW",  // an acronym for Prisoner of War
    L"??",
};

static wchar_t* en_pLongAttributeStrings[] = {
    L"STRENGTH", L"DEXTERITY",  L"AGILITY",    L"WISDOM",     L"MARKSMANSHIP",
    L"MEDICAL",  L"MECHANICAL", L"LEADERSHIP", L"EXPLOSIVES", L"LEVEL",
};

static wchar_t* en_pInvPanelTitleStrings[] = {
    L"Armor",   // the armor rating of the merc
    L"Weight",  // the weight the merc is carrying
    L"Camo",    // the merc's camouflage rating
};

static wchar_t* en_pShortAttributeStrings[] = {
    L"Agi",  // the abbreviated version of : agility
    L"Dex",  // dexterity
    L"Str",  // strength
    L"Ldr",  // leadership
    L"Wis",  // wisdom
    L"Lvl",  // experience level
    L"Mrk",  // marksmanship skill
    L"Exp",  // explosive skill
    L"Mec",  // mechanical skill
    L"Med",  // medical skill
};

static wchar_t* en_pUpperLeftMapScreenStrings[] = {
    L"Assignment",  // the mercs current assignment
    L"Contract",    // the contract info about the merc
    L"Health",      // the health level of the current merc
    L"Morale",      // the morale of the current merc
    L"Cond.",       // the condition of the current vehicle
    L"Fuel",        // the fuel level of the current vehicle
};

static wchar_t* en_pTrainingStrings[] = {
    L"Practice",  // tell merc to train self
    L"Militia",   // tell merc to train town
    L"Trainer",   // tell merc to act as trainer
    L"Student",   // tell merc to be train by other
};

static wchar_t* en_pGuardMenuStrings[] = {
    L"Fire Rate:",             // the allowable rate of fire for a merc who is guarding
    L" Aggressive Fire",       // the merc can be aggressive in their choice of fire rates
    L" Conserve Ammo",         // conserve ammo
    L" Refrain From Firing",   // fire only when the merc needs to
    L"Other Options:",         // other options available to merc
    L" Can Retreat",           // merc can retreat
    L" Can Seek Cover",        // merc is allowed to seek cover
    L" Can Assist Teammates",  // merc can assist teammates
    L"Done",                   // done with this menu
    L"Cancel",                 // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by
// the player

static wchar_t* en_pOtherGuardMenuStrings[] = {
    L"Fire Rate:",
    L" *Aggressive Fire*",
    L" *Conserve Ammo*",
    L" *Refrain From Firing*",
    L"Other Options:",
    L" *Can Retreat*",
    L" *Can Seek Cover*",
    L" *Can Assist Teammates*",
    L"Done",
    L"Cancel",
};

static wchar_t* en_pAssignMenuStrings[] = {
    L"On Duty",  // merc is on active duty
    L"Doctor",   // the merc is acting as a doctor
    L"Patient",  // the merc is receiving medical attention
    L"Vehicle",  // the merc is in a vehicle
    L"Repair",   // the merc is repairing items
    L"Train",    // the merc is training
    L"Cancel",   // cancel this menu
};

static wchar_t* en_pRemoveMercStrings[] = {
    L"Remove Merc",  // remove dead merc from current team
    L"Cancel",
};

static wchar_t* en_pAttributeMenuStrings[] = {
    L"Strength", L"Dexterity",  L"Agility",    L"Health",     L"Marksmanship",
    L"Medical",  L"Mechanical", L"Leadership", L"Explosives", L"Cancel",
};

static wchar_t* en_pTrainingMenuStrings[] = {
    L"Practice",  // train yourself
    L"Militia",   // train the town
    L"Trainer",   // train your teammates
    L"Student",   // be trained by an instructor
    L"Cancel",    // cancel this menu
};

static wchar_t* en_pSquadMenuStrings[] = {
    L"Squad  1", L"Squad  2", L"Squad  3", L"Squad  4", L"Squad  5", L"Squad  6", L"Squad  7",
    L"Squad  8", L"Squad  9", L"Squad 10", L"Squad 11", L"Squad 12", L"Squad 13", L"Squad 14",
    L"Squad 15", L"Squad 16", L"Squad 17", L"Squad 18", L"Squad 19", L"Squad 20", L"Cancel",
};

static wchar_t* en_pPersonnelTitle[] = {
    L"Personnel",  // the title for the personnel screen/program application
};

static wchar_t* en_pPersonnelScreenStrings[] = {
    L"Health: ",  // health of merc
    L"Agility: ",
    L"Dexterity: ",
    L"Strength: ",
    L"Leadership: ",
    L"Wisdom: ",
    L"Exp. Lvl: ",  // experience level
    L"Marksmanship: ",
    L"Mechanical: ",
    L"Explosives: ",
    L"Medical: ",
    L"Med. Deposit: ",         // amount of medical deposit put down on the merc
    L"Remaining Contract: ",   // cost of current contract
    L"Kills: ",                // number of kills by merc
    L"Assists: ",              // number of assists on kills by merc
    L"Daily Cost:",            // daily cost of merc
    L"Tot. Cost to Date:",     // total cost of merc
    L"Contract:",              // cost of current contract
    L"Tot. Service to Date:",  // total service rendered by merc
    L"Salary Owing:",          // amount left on MERC merc to be paid
    L"Hit Percentage:",        // percentage of shots that hit target
    L"Battles:",               // number of battles fought
    L"Times Wounded:",         // number of times merc has been wounded
    L"Skills:",
    L"No Skills",
};

// These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static wchar_t* en_gzMercSkillText[] = {
    L"No Skill",     L"Lock picking", L"Hand to hand",  L"Electronics",  L"Night ops",
    L"Throwing",     L"Teaching",     L"Heavy Weapons", L"Auto Weapons", L"Stealthy",
    L"Ambidextrous", L"Thief",        L"Martial Arts",  L"Knifing",      L"On Roof Bonus to hit",
    L"Camouflaged",  L"(Expert)",
};

// This is pop up help text for the options that are available to the merc

static wchar_t* en_pTacticalPopupButtonStrings[] = {
    L"|Stand/Walk",
    L"|Crouch/Crouched Move",
    L"Stand/|Run",
    L"|Prone/Crawl",
    L"|Look",
    L"Action",
    L"Talk",
    L"Examine (|C|t|r|l)",

    // Pop up door menu
    L"Open Manually",
    L"Examine for Traps",
    L"Lockpick",
    L"Force Open",
    L"Untrap",
    L"Lock",
    L"Unlock",
    L"Use Door Explosive",
    L"Use Crowbar",
    L"Cancel (|E|s|c)",
    L"Close",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static wchar_t* en_pDoorTrapStrings[] = {
    L"no trap", L"an explosion trap", L"an electric trap", L"a siren trap", L"a silent alarm trap",
};

// Contract Extension. These are used for the contract extension with AIM mercenaries.

static wchar_t* en_pContractExtendStrings[] = {
    L"day",
    L"week",
    L"two weeks",
};

// On the map screen, there are four columns. This text is popup help text that identifies the
// individual columns.

static wchar_t* en_pMapScreenMouseRegionHelpText[] = {
    L"Select Character", L"Assign Merc", L"Plot Travel Route",
    L"Merc |Contract",   L"Remove Merc", L"Sleep",
};

// volumes of noises

static wchar_t* en_pNoiseVolStr[] = {
    L"FAINT",
    L"DEFINITE",
    L"LOUD",
    L"VERY LOUD",
};

// types of noises

static wchar_t* en_pNoiseTypeStr[] =  // OBSOLETE
    {
        L"UNKNOWN",    L"sound of MOVEMENT",
        L"CREAKING",   L"SPLASHING",
        L"IMPACT",     L"GUNSHOT",
        L"EXPLOSION",  L"SCREAM",
        L"IMPACT",     L"IMPACT",
        L"SHATTERING", L"SMASH",
};

// Directions that are used to report noises

static wchar_t* en_pDirectionStr[] = {
    L"the NORTHEAST", L"the EAST", L"the SOUTHEAST", L"the SOUTH",
    L"the SOUTHWEST", L"the WEST", L"the NORTHWEST", L"the NORTH",
};

// These are the different terrain types.

static wchar_t* en_pLandTypeStrings[] = {
    L"Urban", L"Road", L"Plains", L"Desert", L"Woods", L"Forest", L"Swamp", L"Water", L"Hills",
    L"Impassable",
    L"River",  // river from north to south
    L"River",  // river from east to west
    L"Foreign Country",
    // NONE of the following are used for directional travel, just for the sector description.
    L"Tropical", L"Farmland", L"Plains, road", L"Woods, road", L"Farm, road", L"Tropical, road",
    L"Forest, road", L"Coastline", L"Mountain, road", L"Coastal, road", L"Desert, road",
    L"Swamp, road", L"Woods, SAM site", L"Desert, SAM site", L"Tropical, SAM site",
    L"Meduna, SAM site",

    // These are descriptions for special sectors
    L"Cambria Hospital", L"Drassen Airport", L"Meduna Airport", L"SAM site",
    L"Rebel Hideout",  // The rebel base underground in sector A10
    L"Tixa Dungeon",   // The basement of the Tixa Prison (J9)
    L"Creature Lair",  // Any mine sector with creatures in it
    L"Orta Basement",  // The basement of Orta (K4)
    L"Tunnel",         // The tunnel access from the maze garden in Meduna
                       // leading to the secret shelter underneath the palace
    L"Shelter",        // The shelter underneath the queen's palace
    L"",               // Unused
};

static wchar_t* en_gpStrategicString[] = {
    L"",                                                                            // Unused
    L"%s have been detected in sector %c%d and another squad is about to arrive.",  // STR_DETECTED_SINGULAR
    L"%s have been detected in sector %c%d and other squads are about to arrive.",  // STR_DETECTED_PLURAL
    L"Do you want to coordinate a simultaneous arrival?",  // STR_COORDINATE

    // Dialog strings for enemies.

    L"The enemy offers you the chance to surrender.",             // STR_ENEMY_SURRENDER_OFFER
    L"The enemy has captured your remaining unconscious mercs.",  // STR_ENEMY_CAPTURED

    // The text that goes on the autoresolve buttons

    L"Retreat",  // The retreat button				//STR_AR_RETREAT_BUTTON
    L"Done",     // The done button				//STR_AR_DONE_BUTTON

    // The headers are for the autoresolve type (MUST BE UPPERCASE)

    L"DEFENDING",  // STR_AR_DEFEND_HEADER
    L"ATTACKING",  // STR_AR_ATTACK_HEADER
    L"ENCOUNTER",  // STR_AR_ENCOUNTER_HEADER
    L"Sector",     // The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

    // The battle ending conditions

    L"VICTORY!",      // STR_AR_OVER_VICTORY
    L"DEFEAT!",       // STR_AR_OVER_DEFEAT
    L"SURRENDERED!",  // STR_AR_OVER_SURRENDERED
    L"CAPTURED!",     // STR_AR_OVER_CAPTURED
    L"RETREATED!",    // STR_AR_OVER_RETREATED

    // These are the labels for the different types of enemies we fight in autoresolve.

    L"Militia",   // STR_AR_MILITIA_NAME,
    L"Elite",     // STR_AR_ELITE_NAME,
    L"Troop",     // STR_AR_TROOP_NAME,
    L"Admin",     // STR_AR_ADMINISTRATOR_NAME,
    L"Creature",  // STR_AR_CREATURE_NAME,

    // Label for the length of time the battle took

    L"Time Elapsed",  // STR_AR_TIME_ELAPSED,

    // Labels for status of merc if retreating.  (UPPERCASE)

    L"RETREATED",   // STR_AR_MERC_RETREATED,
    L"RETREATING",  // STR_AR_MERC_RETREATING,
    L"RETREAT",     // STR_AR_MERC_RETREAT,

    // PRE BATTLE INTERFACE STRINGS
    // Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
    // a system that automatically resolves the combat for the player without having to do anything.
    // These strings must be short (two lines -- 6-8 chars per line)

    L"Auto Resolve",   // STR_PB_AUTORESOLVE_BTN,
    L"Go To Sector",   // STR_PB_GOTOSECTOR_BTN,
    L"Retreat Mercs",  // STR_PB_RETREATMERCS_BTN,

    // The different headers(titles) for the prebattle interface.
    L"ENEMY ENCOUNTER",         // STR_PB_ENEMYENCOUNTER_HEADER,
    L"ENEMY INVASION",          // STR_PB_ENEMYINVASION_HEADER, // 30
    L"ENEMY AMBUSH",            // STR_PB_ENEMYAMBUSH_HEADER
    L"ENTERING ENEMY SECTOR",   // STR_PB_ENTERINGENEMYSECTOR_HEADER
    L"CREATURE ATTACK",         // STR_PB_CREATUREATTACK_HEADER
    L"BLOODCAT AMBUSH",         // STR_PB_BLOODCATAMBUSH_HEADER
    L"ENTERING BLOODCAT LAIR",  // STR_PB_ENTERINGBLOODCATLAIR_HEADER

    // Various single words for direct translation.  The Civilians represent the civilian
    // militia occupying the sector being attacked.  Limited to 9-10 chars

    L"Location",
    L"Enemies",
    L"Mercs",
    L"Militia",
    L"Creatures",
    L"Bloodcats",
    L"Sector",
    L"None",  // If there are no uninvolved mercs in this fight.
    L"N/A",   // Acronym of Not Applicable
    L"d",     // One letter abbreviation of day
    L"h",     // One letter abbreviation of hour

    // TACTICAL PLACEMENT USER INTERFACE STRINGS
    // The four buttons

    L"Clear",
    L"Spread",
    L"Group",
    L"Done",

    // The help text for the four buttons.  Use \n to denote new line (just like enter).

    L"|Clears all the mercs' positions, \nand allows you to re-enter them manually.",
    L"Randomly |spreads your mercs out \neach time it's pressed.",
    L"Allows you to select where you wish to |group your mercs.",
    L"Click this button when you're finished \nchoosing your mercs' positions. (|E|n|t|e|r)",
    L"You must place all of your mercs \nbefore you start the battle.",

    // Various strings (translate word for word)

    L"Sector",
    L"Choose entry positions",

    // Strings used for various popup message boxes.  Can be as long as desired.

    L"Doesn't look so good there. It's inaccessible. Try a different location.",
    L"Place your mercs in the highlighted section of the map.",

    // This message is for mercs arriving in sectors.  Ex:  Red has arrived in sector A9.
    // Don't uppercase first character, or add spaces on either end.

    L"has arrived in sector",

    // These entries are for button popup help text for the prebattle interface.  All popup help
    // text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
    L"|Automatically resolves combat for you\nwithout loading map.",
    L"Can't use auto resolve feature when\nthe player is attacking.",
    L"|Enter the sector to engage the enemy.",
    L"|Retreat group to their previous sector.",        // singular version
    L"|Retreat all groups to their previous sectors.",  // multiple groups with same previous sector

    // various popup messages for battle conditions.

    //%c%d is the sector -- ex:  A9
    L"Enemies attack your militia in sector %c%d.",
    //%c%d is the sector -- ex:  A9
    L"Creatures attack your militia in sector %c%d.",
    // 1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
    // Note:  the minimum number of civilians eaten will be two.
    L"Creatures attack and kill %d civilians in sector %s.",
    //%s is the sector location -- ex:  A9: Omerta
    L"Enemies attack your mercs in sector %s.  None of your mercs are able to fight!",
    //%s is the sector location -- ex:  A9: Omerta
    L"Creatures attack your mercs in sector %s.  None of your mercs are able to fight!",

};

static wchar_t* en_gpGameClockString[] = {
    // This is the day represented in the game clock.  Must be very short, 4 characters max.
    L"Day",
};

// When the merc finds a key, they can get a description of it which
// tells them where and when they found it.
static wchar_t* en_sKeyDescriptionStrings[2] = {
    L"Sector Found:",
    L"Day Found:",
};

// The headers used to describe various weapon statistics.

static wchar_t* en_gWeaponStatsDesc[] = {
    L"Weight (%s):",
    L"Status:",
    L"Amount:",  // Number of bullets left in a magazine
    L"Rng:",     // Range
    L"Dam:",     // Damage
    L"AP:",      // abbreviation for Action Points
    L"",
    L"=",
    L"=",
};

// The headers used for the merc's money.

static wchar_t* en_gMoneyStatsDesc[] = {
    L"Amount",
    L"Remaining:",  // this is the overall balance
    L"Amount",
    L"To Split:",  // the amount he wants to separate from the overall balance to get two piles of
                   // money

    L"Current",
    L"Balance",
    L"Amount to",
    L"Withdraw",
};

// The health of various creatures, enemies, characters in the game. The numbers following each are
// for comment only, but represent the precentage of points remaining.

static wchar_t* en_zHealthStr[] = {
    L"DYING",      //	>= 0
    L"CRITICAL",   //	>= 15
    L"POOR",       //	>= 30
    L"WOUNDED",    //	>= 45
    L"HEALTHY",    //	>= 60
    L"STRONG",     // 	>= 75
    L"EXCELLENT",  // 	>= 90
};

static wchar_t* en_gzMoneyAmounts[6] = {
    L"$1000", L"$100", L"$10", L"Done", L"Separate", L"Withdraw",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static wchar_t* en_gzProsLabel = L"Pros:";

static wchar_t* en_gzConsLabel = L"Cons:";

// Conversation options a player has when encountering an NPC
static wchar_t* en_zTalkMenuStrings[6] = {
    L"Come Again?",  // meaning "Repeat yourself"
    L"Friendly",     // approach in a friendly
    L"Direct",       // approach directly - let's get down to business
    L"Threaten",     // approach threateningly - talk now, or I'll blow your face off
    L"Give",        L"Recruit",
};

// Some NPCs buy, sell or repair items. These different options are available for those NPCs as
// well.
static wchar_t* en_zDealerStrings[4] = {
    L"Buy/Sell",
    L"Buy",
    L"Sell",
    L"Repair",
};

static wchar_t* en_zDialogActions[1] = {
    L"Done",
};

// These are vehicles in the game.

static wchar_t* en_pVehicleStrings[] = {
    L"Eldorado",
    L"Hummer",  // a hummer jeep/truck -- military vehicle
    L"Icecream Truck", L"Jeep", L"Tank", L"Helicopter",
};

static wchar_t* en_pShortVehicleStrings[] = {
    L"Eldor.",
    L"Hummer",  // the HMVV
    L"Truck",  L"Jeep", L"Tank",
    L"Heli",  // the helicopter
};

static wchar_t* en_zVehicleName[] = {
    L"Eldorado",
    L"Hummer",  // a military jeep. This is a brand name.
    L"Truck",   // Ice cream truck
    L"Jeep",     L"Tank",
    L"Heli",  // an abbreviation for Helicopter
};

// These are messages Used in the Tactical Screen

static wchar_t* en_TacticalStr[] = {
    L"Air Raid", L"Apply first aid automatically?",

    // CAMFIELD NUKE THIS and add quote #66.

    L"%s notices that items are missing from the shipment.",

    // The %s is a string from pDoorTrapStrings

    L"The lock has %s.", L"There's no lock.", L"Success!", L"Failure.", L"Success!", L"Failure.",
    L"The lock isn't trapped.", L"Success!",
    // The %s is a merc name
    L"%s doesn't have the right key.", L"The lock is untrapped.", L"The lock isn't trapped.",
    L"Locked.", L"DOOR", L"TRAPPED", L"LOCKED", L"UNLOCKED", L"SMASHED",
    L"There's a switch here.  Activate it?", L"Disarm trap?", L"Prev...", L"Next...", L"More...",

    // In the next 2 strings, %s is an item name

    L"The %s has been placed on the ground.", L"The %s has been given to %s.",

    // In the next 2 strings, %s is a name

    L"%s has been paid in full.", L"%s is still owed %d.",
    L"Choose detonation frequency:",     // in this case, frequency refers to a radio signal
    L"How many turns 'til she blows:",   // how much time, in turns, until the bomb blows
    L"Set remote detonator frequency:",  // in this case, frequency refers to a radio signal
    L"Disarm boobytrap?", L"Remove blue flag?", L"Put blue flag here?", L"Ending Turn",

    // In the next string, %s is a name. Stance refers to way they are standing.

    L"You sure you want to attack %s ?", L"Ah, vehicles can't change stance.",
    L"The robot can't change its stance.",

    // In the next 3 strings, %s is a name

    L"%s can't change to that stance here.", L"%s can't have first aid done here.",
    L"%s doesn't need first aid.", L"Can't move there.",
    L"Your team's full. No room for a recruit.",  // there's no room for a recruit on the player's
                                                  // team

    // In the next string, %s is a name

    L"%s has been recruited.",

    // Here %s is a name and %d is a number

    L"%s is owed $%d.",

    // In the next string, %s is a name

    L"Escort %s?",

    // In the next string, the first %s is a name and the second %s is an amount of money (including
    // $ sign)

    L"Hire %s for %s per day?",

    // This line is used repeatedly to ask player if they wish to participate in a boxing match.

    L"You want to fight?",

    // In the next string, the first %s is an item name and the
    // second %s is an amount of money (including $ sign)

    L"Buy %s for %s?",

    // In the next string, %s is a name

    L"%s is being escorted on squad %d.",

    // These messages are displayed during play to alert the player to a particular situation

    L"JAMMED",                          // weapon is jammed.
    L"Robot needs %s caliber ammo.",    // Robot is out of ammo
    L"Throw there? Not gonna happen.",  // Merc can't throw to the destination he selected

    // These are different buttons that the player can turn on and off.

    L"Stealth Mode (|Z)", L"|Map Screen", L"|Done (End Turn)", L"Talk", L"Mute",
    L"Stance Up (|P|g|U|p)", L"Cursor Level (|T|a|b)", L"Climb / Jump", L"Stance Down (|P|g|D|n)",
    L"Examine (|C|t|r|l)", L"Previous Merc", L"Next Merc (|S|p|a|c|e)", L"|Options", L"|Burst Mode",
    L"|Look/Turn", L"Health: %d/%d\nEnergy: %d/%d\nMorale: %s",
    L"Heh?",  // this means "what?"
    L"Cont",  // an abbrieviation for "Continued"
    L"Mute off for %s.", L"Mute on for %s.", L"Health: %d/%d\nFuel: %d/%d", L"Exit Vehicle",
    L"Change Squad ( |S|h|i|f|t |S|p|a|c|e )", L"Drive",
    L"N/A",  // this is an acronym for "Not Applicable."
    L"Use ( Hand To Hand )", L"Use ( Firearm )", L"Use ( Blade )", L"Use ( Explosive )",
    L"Use ( Medkit )", L"(Catch)", L"(Reload)", L"(Give)", L"%s has been set off.",
    L"%s has arrived.", L"%s ran out of Action Points.", L"%s isn't available.",
    L"%s is all bandaged.", L"%s is out of bandages.", L"Enemy in sector!", L"No enemies in sight.",
    L"Not enough Action Points.", L"Nobody's using the remote.", L"Burst fire emptied the clip!",
    L"SOLDIER", L"CREPITUS", L"MILITIA", L"CIVILIAN", L"Exiting Sector", L"OK", L"Cancel",
    L"Selected Merc", L"All Mercs in Squad", L"Go to Sector", L"Go to Map",
    L"You can't leave the sector from this side.", L"%s is too far away.", L"Removing Treetops",
    L"Showing Treetops",
    L"CROW",  // Crow, as in the large black bird
    L"NECK", L"HEAD", L"TORSO", L"LEGS", L"Tell the Queen what she wants to know?",
    L"Fingerprint ID aquired", L"Invalid fingerprint ID. Weapon non-functional", L"Target aquired",
    L"Path Blocked",
    L"Deposit/Withdraw Money",  // Help text over the $ button on the Single Merc Panel
    L"No one needs first aid.",
    L"Jam.",              // Short form of JAMMED, for small inv slots
    L"Can't get there.",  // used ( now ) for when we click on a cliff
    L"Path is blocked. Do you want to switch places with this person?",
    L"The person refuses to move.",
    // In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
    L"Do you agree to pay %s?", L"Accept free medical treatment?", L"Agree to marry Daryl?",
    L"Key Ring Panel", L"You cannot do that with an EPC.", L"Spare Krott?",
    L"Out of effective weapon range.", L"Miner", L"Vehicle can only travel between sectors",
    L"Can't autobandage right now", L"Path Blocked for %s",
    L"Your mercs, who were captured by Deidranna's army are imprisoned here!", L"Lock hit",
    L"Lock destroyed", L"Somebody else is trying to use this door.", L"Health: %d/%d\nFuel: %d/%d",
    L"%s cannot see %s.",  // Cannot see person trying to talk to
};

// Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different
// circumstances in the "exiting sector" interface.
static wchar_t* en_pExitingSectorHelpText[] = {
    // Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box
    // is checked.
    L"If checked, the adjacent sector will be immediately loaded.",
    L"If checked, you will be placed automatically in the map screen\nas it will take time for "
    L"your mercs to travel.",

    // If you attempt to leave a sector when you have multiple squads in a hostile sector.
    L"This sector is enemy occupied and you can't leave mercs here.\nYou must deal with this "
    L"situation before loading any other sectors.",

    // Because you only have one squad in the sector, and the "move all" option is checked, the "go
    // to sector" option is locked to on. The helptext explains why it is locked.
    L"By moving your remaining mercs out of this sector,\nthe adjacent sector will immediately be "
    L"loaded.",
    L"By moving your remaining mercs out of this sector,\nyou will be placed automatically in the "
    L"map screen\nas it will take time for your mercs to travel.",

    // If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being
    // escorted.  The "single" button is disabled.
    L"%s needs to be escorted by your mercs and cannot leave this sector alone.",

    // If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc
    // will be prohibited from leaving alone. There are several strings depending on the gender of
    // the merc and how many EPCs are in the squad. DO NOT USE THE NEWLINE HERE AS IT IS USED FOR
    // BOTH HELPTEXT AND SCREEN MESSAGES!
    L"%s cannot leave this sector alone as he is escorting %s.",   // male singular
    L"%s cannot leave this sector alone as she is escorting %s.",  // female singular
    L"%s cannot leave this sector alone as he is escorting multiple characters.",   // male plural
    L"%s cannot leave this sector alone as she is escorting multiple characters.",  // female plural

    // If one or more of your mercs in the selected squad aren't in range of the traversal area,
    // then the  "move all" option is disabled, and this helptext explains why.
    L"All of your mercs must be in the vicinity\nin order to allow the squad to traverse.",

    L"",  // UNUSED

    // Standard helptext for single movement.  Explains what will happen (splitting the squad)
    L"If checked, %s will travel alone, and\nautomatically get reassigned to a unique squad.",

    // Standard helptext for all movement.  Explains what will happen (moving the squad)
    L"If checked, your currently selected\nsquad will travel, leaving this sector.",

    // This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC
    // selected and you attempt to tactically traverse the EPC while the escorting mercs aren't near
    // enough (or dead, dying, or unconscious), this message will appear and the "exiting sector"
    // interface will not appear.  This is just like the situation where This string is special, as
    // it is not used as helptext.  Do not use the special newline character (\n) for this string.
    L"%s is being escorted by your mercs and cannot leave this sector alone. Your other mercs must "
    L"be nearby before you can leave.",
};

static wchar_t* en_pRepairStrings[] = {
    L"Items",     // tell merc to repair items in inventory
    L"SAM Site",  // tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
    L"Cancel",    // cancel this menu
    L"Robot",     // repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static wchar_t* en_sPreStatBuildString[] = {
    L"lost",       // the merc has lost a statistic
    L"gained",     // the merc has gained a statistic
    L"point of",   // singular
    L"points of",  // plural
    L"level of",   // singular
    L"levels of",  // plural
};

static wchar_t* en_sStatGainStrings[] = {
    L"health.",        L"agility.",          L"dexterity.",        L"wisdom.",
    L"medical skill.", L"explosives skill.", L"mechanical skill.", L"marksmanship skill.",
    L"experience.",    L"strength.",         L"leadership.",
};

static wchar_t* en_pHelicopterEtaStrings[] = {
    L"Total Distance:  ",  // total distance for helicopter to travel
    L" Safe:  ",           // distance to travel to destination
    L" Unsafe:",           // distance to return from destination to airport
    L"Total Cost: ",       // total cost of trip by helicopter
    L"ETA:  ",             // ETA is an acronym for "estimated time of arrival"
    L"Helicopter is low on fuel and must land in hostile territory!",  // warning that the sector
                                                                       // the helicopter is going to
                                                                       // use for refueling is under
                                                                       // enemy control ->
    L"Passengers: ",
    L"Select Skyrider or the Arrivals Drop-off?",
    L"Skyrider",
    L"Arrivals",
};

static wchar_t* en_sMapLevelString[] = {
    L"Sublevel:",  // what level below the ground is the player viewing in mapscreen
};

static wchar_t* en_gsLoyalString[] = {
    L"Loyal",  // the loyalty rating of a town ie : Loyal 53%
};

// error message for when player is trying to give a merc a travel order while he's underground.

static wchar_t* en_gsUndergroundString[] = {
    L"can't get travel orders underground.",
};

static wchar_t* en_gsTimeStrings[] = {
    L"h",  // hours abbreviation
    L"m",  // minutes abbreviation
    L"s",  // seconds abbreviation
    L"d",  // days abbreviation
};

// text for the various facilities in the sector

static wchar_t* en_sFacilitiesStrings[] = {
    L"None",           L"Hospital", L"Industry", L"Prison", L"Military", L"Airport",
    L"Shooting Range",  // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static wchar_t* en_pMapPopUpInventoryText[] = {
    L"Inventory",
    L"Exit",
};

// town strings

static wchar_t* en_pwTownInfoStrings[] = {
    L"Size",             // 0 // size of the town in sectors
    L"",                 // blank line, required
    L"Control",          // how much of town is controlled
    L"None",             // none of this town
    L"Associated Mine",  // mine associated with this town
    L"Loyalty",          // 5 // the loyalty level of this town
    L"Trained",          // the forces in the town trained by the player
    L"",
    L"Main Facilities",    // main facilities in this town
    L"Level",              // the training level of civilians in this town
    L"Civilian Training",  // 10 // state of civilian training in town
    L"Militia",            // the state of the trained civilians in the town
};

// Mine strings

static wchar_t* en_pwMineStrings[] = {
    L"Mine",  // 0
    L"Silver",
    L"Gold",
    L"Daily Production",
    L"Possible Production",
    L"Abandoned",  // 5
    L"Shut Down",
    L"Running Out",
    L"Producing",
    L"Status",
    L"Production Rate",
    L"Ore Type",  // 10
    L"Town Control",
    L"Town Loyalty",
    //	L"Working Miners",
};

// blank sector strings

static wchar_t* en_pwMiscSectorStrings[] = {
    L"Enemy Forces", L"Sector", L"# of Items", L"Unknown", L"Controlled", L"Yes", L"No",
};

// error strings for inventory

static wchar_t* en_pMapInventoryErrorString[] = {
    L"%s isn't close enough.",   // Merc is in sector with item but not close enough
    L"Can't select that merc.",  // MARK CARTER
    L"%s isn't in the sector to take that item.",
    L"During combat, you'll have to pick up items manually.",
    L"During combat, you'll have to drop items manually.",
    L"%s isn't in the sector to drop that item.",
};

static wchar_t* en_pMapInventoryStrings[] = {
    L"Location",     // sector these items are in
    L"Total Items",  // total number of items in sector
};

// help text for the user

static wchar_t* en_pMapScreenFastHelpTextList[] = {
    L"To change a merc's assignment to such things as another squad, doctor or repair, click "
    L"within the 'Assign' column",
    L"To give a merc a destination in another sector, click within the 'Dest' column",
    L"Once a merc has been given a movement order, time compression allows them to get going.",
    L"Left click selects the sector. Left click again to give a merc movement orders, or Right "
    L"click to get sector summary information.",
    L"Press 'h' at any time in this screen to get this help dialogue up.",
    L"Test Text",
    L"Test Text",
    L"Test Text",
    L"Test Text",
    L"There isn't much you can do on this screen until you arrive in Arulco. When you've finalized "
    L"your team, click on the Time Compression button at the lower right. This will advance time "
    L"until your team arrives in Arulco.",
};

// movement menu text

static wchar_t* en_pMovementMenuStrings[] = {
    L"Move Mercs In Sector",  // title for movement box
    L"Plot Travel Route",     // done with movement menu, start plotting movement
    L"Cancel",                // cancel this menu
    L"Other",                 // title for group of mercs not on squads nor in vehicles
};

static wchar_t* en_pUpdateMercStrings[] = {
    L"Oops:",                        // an error has occured
    L"Mercs Contract Expired:",      // this pop up came up due to a merc contract ending
    L"Mercs Completed Assignment:",  // this pop up....due to more than one merc finishing
                                     // assignments
    L"Mercs Back on the Job:",  // this pop up ....due to more than one merc waking up and returing
                                // to work
    L"Mercs Catching Some Z's:",  // this pop up ....due to more than one merc being tired and going
                                  // to sleep
    L"Contracts Expiring Soon:",  // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static wchar_t* en_pMapScreenBorderButtonHelpText[] = {
    L"Show To|wns",    L"Show |Mines", L"Show |Teams & Enemies",
    L"Show |Airspace", L"Show |Items", L"Show Militia & Enemies (|Z)",
};

static wchar_t* en_pMapScreenBottomFastHelp[] = {
    L"|Laptop",
    L"Tactical (|E|s|c)",
    L"|Options",
    L"Time Compress (|+)",                                 // time compress more
    L"Time Compress (|-)",                                 // time compress less
    L"Previous Message (|U|p)\nPrevious Page (|P|g|U|p)",  // previous message in scrollable list
    L"Next Message (|D|o|w|n)\nNext Page (|P|g|D|n)",      // next message in the scrollable list
    L"Start/Stop Time (|S|p|a|c|e)",                       // start/stop time compression
};

static wchar_t* en_pMapScreenBottomText[] = {
    L"Current Balance",  // current balance in player bank account
};

static wchar_t* en_pMercDeadString[] = {
    L"%s is dead.",
};

static wchar_t* en_pDayStrings[] = {
    L"Day",
};

// the list of email sender names

static wchar_t* en_pSenderNameList[] = {
    L"Enrico",
    L"Psych Pro Inc",
    L"Help Desk",
    L"Psych Pro Inc",
    L"Speck",
    L"R.I.S.",  // 5
    L"Barry",
    L"Blood",
    L"Lynx",
    L"Grizzly",
    L"Vicki",  // 10
    L"Trevor",
    L"Grunty",
    L"Ivan",
    L"Steroid",
    L"Igor",  // 15
    L"Shadow",
    L"Red",
    L"Reaper",
    L"Fidel",
    L"Fox",  // 20
    L"Sidney",
    L"Gus",
    L"Buns",
    L"Ice",
    L"Spider",  // 25
    L"Cliff",
    L"Bull",
    L"Hitman",
    L"Buzz",
    L"Raider",  // 30
    L"Raven",
    L"Static",
    L"Len",
    L"Danny",
    L"Magic",
    L"Stephen",
    L"Scully",
    L"Malice",
    L"Dr.Q",
    L"Nails",
    L"Thor",
    L"Scope",
    L"Wolf",
    L"MD",
    L"Meltdown",
    //----------
    L"M.I.S. Insurance",
    L"Bobby Rays",
    L"Kingpin",
    L"John Kulba",
    L"A.I.M.",
};

// next/prev strings

static wchar_t* en_pTraverseStrings[] = {
    L"Previous",
    L"Next",
};

// new mail notify string

static wchar_t* en_pNewMailStrings[] = {
    L"You have new mail...",
};

// confirm player's intent to delete messages

static wchar_t* en_pDeleteMailStrings[] = {
    L"Delete mail?",
    L"Delete UNREAD mail?",
};

// the sort header strings

static wchar_t* en_pEmailHeaders[] = {
    L"From:",
    L"Subject:",
    L"Day:",
};

// email titlebar text

static wchar_t* en_pEmailTitleText[] = {
    L"Mail Box",
};

// the financial screen strings
static wchar_t* en_pFinanceTitle[] = {
    L"Bookkeeper Plus",  // the name we made up for the financial program in the game
};

static wchar_t* en_pFinanceSummary[] = {
    L"Credit:",  // credit (subtract from) to player's account
    L"Debit:",   // debit (add to) to player's account
    L"Yesterday's Actual Income:",
    L"Yesterday's Other Deposits:",
    L"Yesterday's Debits:",
    L"Balance At Day's End:",
    L"Today's Actual Income:",
    L"Today's Other Deposits:",
    L"Today's Debits:",
    L"Current Balance:",
    L"Forecasted Income:",
    L"Projected Balance:",  // projected balance for player for tommorow
};

// headers to each list in financial screen

static wchar_t* en_pFinanceHeaders[] = {
    L"Day",          // the day column
    L"Credit",       // the credits column (to ADD money to your account)
    L"Debit",        // the debits column (to SUBTRACT money from your account)
    L"Transaction",  // transaction type - see TransactionText below
    L"Balance",      // balance at this point in time
    L"Page",         // page number
    L"Day(s)",       // the day(s) of transactions this page displays
};

static wchar_t* en_pTransactionText[] = {
    L"Accrued Interest",  // interest the player has accumulated so far
    L"Anonymous Deposit",
    L"Transaction Fee",
    L"Hired",               // Merc was hired
    L"Bobby Ray Purchase",  // Bobby Ray is the name of an arms dealer
    L"Settled Accounts at M.E.R.C.",
    L"Medical Deposit for %s",  // medical deposit for merc
    L"IMP Profile Analysis",    // IMP is the acronym for International Mercenary Profiling
    L"Purchased Insurance for %s",
    L"Reduced Insurance for %s",
    L"Extended Insurance for %s",  // johnny contract extended
    L"Canceled Insurance for %s",
    L"Insurance Claim for %s",  // insurance claim for merc
    L"a day",                   // merc's contract extended for a day
    L"1 week",                  // merc's contract extended for a week
    L"2 weeks",                 // ... for 2 weeks
    L"Mine income",
    L"",  // String nuked
    L"Purchased Flowers",
    L"Full Medical Refund for %s",
    L"Partial Medical Refund for %s",
    L"No Medical Refund for %s",
    L"Payment to %s",             // %s is the name of the npc being paid
    L"Transfer Funds to %s",      // transfer funds to a merc
    L"Transfer Funds from %s",    // transfer funds from a merc
    L"Equip militia in %s",       // initial cost to equip a town's militia
    L"Purchased items from %s.",  // is used for the Shop keeper interface.  The dealers name will
                                  // be appended to the end of the string.
    L"%s deposited money.",
};

static wchar_t* en_pTransactionAlternateText[] = {
    L"Insurance for",                   // insurance for a merc
    L"Ext. %s's contract by one day.",  // entend mercs contract by a day
    L"Ext. %s contract by 1 week.",
    L"Ext. %s contract by 2 weeks.",
};

// helicopter pilot payment

static wchar_t* en_pSkyriderText[] = {
    L"Skyrider was paid $%d",                // skyrider was paid an amount of money
    L"Skyrider is still owed $%d",           // skyrider is still owed an amount of money
    L"Skyrider has finished refueling",      // skyrider has finished refueling
    L"",                                     // unused
    L"",                                     // unused
    L"Skyrider is ready to fly once more.",  // Skyrider was grounded but has been freed
    L"Skyrider has no passengers.  If it is your intention to transport mercs in this sector, "
    L"assign them to Vehicle/Helicopter first.",
};

// strings for different levels of merc morale

static wchar_t* en_pMoralStrings[] = {
    L"Great", L"Good", L"Stable", L"Poor", L"Panic", L"Bad",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.

static wchar_t* en_pLeftEquipmentString[] = {
    L"%s's equipment is now available in Omerta (A9).",
    L"%s's equipment is now available in Drassen (B13).",
};

// Status that appears on the Map Screen

static wchar_t* en_pMapScreenStatusStrings[] = {
    L"Health",    L"Energy", L"Morale",
    L"Condition",  // the condition of the current vehicle (its "health")
    L"Fuel",       // the fuel level of the current vehicle (its "energy")
};

static wchar_t* en_pMapScreenPrevNextCharButtonHelpText[] = {
    L"Previous Merc (|L|e|f|t)",  // previous merc in the list
    L"Next Merc (|R|i|g|h|t)",    // next merc in the list
};

static wchar_t* en_pEtaString[] = {
    L"ETA:",  // eta is an acronym for Estimated Time of Arrival
};

static wchar_t* en_pTrashItemText[] = {
    L"You'll never see it again. You sure?",  // do you want to continue and lose the
                                              // item forever
    L"This item looks REALLY important. Are you REALLY REALLY sure you want to trash "
    L"it?",  // does the user REALLY want to trash this item
};

static wchar_t* en_pMapErrorString[] = {
    L"Squad can't move with a sleeping merc on it.",

    // 1-5
    L"Move the squad above ground first.",
    L"Movement orders? It's a hostile sector!",
    L"Mercs must be assigned to a squad or vehicle in order to travel.",
    L"You don't have any team members yet.",  // you have no members, can't do anything
    L"Merc can't comply.",                    // merc can't comply with your order
                                              // 6-10
    L"needs an escort to move. Place him on a squad with one.",  // merc can't move unescorted ..
                                                                 // for a male
    L"needs an escort to move. Place her on a squad with one.",  // for a female
    L"Merc hasn't yet arrived in Arulco!",
    L"Looks like there's some contract negotiations to settle first.",
    L"",
    // 11-15
    L"Movement orders? There's a battle going on!",
    L"You have been ambushed by bloodcats in sector %s!",
    L"You have just entered what appears to be a bloodcat lair in sector I16!",
    L"",
    L"The SAM site in %s has been taken over.",
    // 16-20
    L"The mine in %s has been taken over. Your daily income has been reduced to %s per day.",
    L"The enemy has taken over sector %s uncontested.",
    L"At least one of your mercs could not be put on this assignment.",
    L"%s could not join %s as it is already full",
    L"%s could not join %s as it is too far away.",
    // 21-25
    L"The mine in %s has been captured by Deidranna's forces!",
    L"Deidranna's forces have just invaded the SAM site in %s",
    L"Deidranna's forces have just invaded %s",
    L"Deidranna's forces have just been spotted in %s.",
    L"Deidranna's forces have just taken over %s.",
    // 26-30
    L"At least one of your mercs could not be put asleep.",
    L"At least one of your mercs could not be woken up.",
    L"Militia will not appear until they have finished training.",
    L"%s cannot be given movement orders at this time.",
    L"Militia that are not within town boundaries cannot be moved to another sector.",
    // 31-35
    L"You can't have militia in %s.",
    L"A vehicle can't move while empty!",
    L"%s is too injured to travel!",
    L"You must leave the museum first!",
    L"%s is dead!",
    // 36-40
    L"%s can't switch to %s because it's moving",
    L"%s can't enter the vehicle that way",
    L"%s can't join %s",
    L"You can't compress time until you hire some new mercs!",
    L"This vehicle can only travel along roads!",
    // 41-45
    L"You can't reassign mercs who are on the move",
    L"Vehicle is out of gas!",
    L"%s is too tired to travel.",
    L"Nobody aboard is able to drive the vehicle.",
    L"One or more members of this squad can't move right now.",
    // 46-50
    L"One or more of the OTHER mercs can't move right now.",
    L"Vehicle is too damaged!",
    L"Note that only two mercs may train militia in each sector.",
    L"The robot can't move without its controller. Place them together in the same squad.",
};

// help text used during strategic route plotting
static wchar_t* en_pMapPlotStrings[] = {
    L"Click again on the destination to confirm your final route, or click on another sector to "
    L"place more waypoints.",
    L"Travel route confirmed.",
    L"Destination unchanged.",
    L"Travel route canceled.",
    L"Travel route shortened.",
};

// help text used when moving the merc arrival sector
static wchar_t* en_pBullseyeStrings[] = {
    L"Click on the sector where you would like the mercs to arrive instead.",
    L"OK.  Arriving mercs will be dropped off in %s",
    L"Mercs can't be flown there, the airspace isn't secured!",
    L"Canceled.  Arrival sector unchanged",
    L"Airspace over %s is no longer secure!  Arrival sector was moved to %s.",
};

// help text for mouse regions

static wchar_t* en_pMiscMapScreenMouseRegionHelpText[] = {
    L"Enter Inventory (|E|n|t|e|r)",
    L"Throw Item Away",
    L"Exit Inventory (|E|n|t|e|r)",
};

// male version of where equipment is left
static wchar_t* en_pMercHeLeaveString[] = {
    L"Have %s leave his equipment where he is now (%s) or later on in Drassen (B13) upon catching "
    L"flight out of Arulco?",
    L"Have %s leave his equipment where he is now (%s) or later on in Omerta (A9) upon catching "
    L"flight out of Arulco?",
    L"is about to leave and will drop off his equipment in Omerta (A9).",
    L"is about to leave and will drop off his equipment in Drassen (B13).",
    L"%s is about to leave and will drop off his equipment in %s.",
};

// female version
static wchar_t* en_pMercSheLeaveString[] = {
    L"Have %s leave her equipment where she is now (%s) or later on in Drassen (B13) upon catching "
    L"flight out of Arulco?",
    L"Have %s leave her equipment where she is now (%s) or later on in Omerta (A9) upon catching "
    L"flight out of Arulco?",
    L"is about to leave and will drop off her equipment in Omerta (A9).",
    L"is about to leave and will drop off her equipment in Drassen (B13).",
    L"%s is about to leave and will drop off her equipment in %s.",
};

static wchar_t* en_pMercContractOverStrings[] = {
    L"'s contract ended, so he's gone home.",            // merc's contract is over and has departed
    L"'s contract ended, so she's gone home.",           // merc's contract is over and has departed
    L"'s contract was terminated, so he left.",          // merc's contract has been terminated
    L"'s contract was terminated, so she left.",         // merc's contract has been terminated
    L"You owe M.E.R.C. too much cash, so %s took off.",  // Your M.E.R.C. account is invalid so merc
                                                         // left
};

// Text used on IMP Web Pages

static wchar_t* en_pImpPopUpStrings[] = {
    L"Invalid Authorization Code",
    L"You Are About To Restart The Entire Profiling Process. Are You Certain?",
    L"Please Enter A Valid Full Name and Gender",
    L"Preliminary analysis of your financial status shows that you cannot afford a profile "
    L"analysis.",
    L"Not A Valid Option At This Time.",
    L"To complete an accurate profile, you must have room for at least one team member.",
    L"Profile Already Completed.",
};

// button labels used on the IMP site

static wchar_t* en_pImpButtonText[] = {
    L"About Us",     // about the IMP site
    L"BEGIN",        // begin profiling
    L"Personality",  // personality section
    L"Attributes",   // personal stats/attributes section
    L"Portrait",     // the personal portrait selection
    L"Voice %d",     // the voice selection
    L"Done",         // done profiling
    L"Start Over",   // start over profiling
    L"Yes, I choose the highlighted answer.",
    L"Yes",
    L"No",
    L"Finished",                   // finished answering questions
    L"Prev",                       // previous question..abbreviated form
    L"Next",                       // next question
    L"YES, I AM.",                 // yes, I am certain
    L"NO, I WANT TO START OVER.",  // no, I want to start over the profiling process
    L"YES, I DO.",
    L"NO",
    L"Back",    // back one page
    L"Cancel",  // cancel selection
    L"Yes, I am certain.",
    L"No, let me have another look.",
    L"Registry",   // the IMP site registry..when name and gender is selected
    L"Analyzing",  // analyzing your profile results
    L"OK",
    L"Voice",
};

static wchar_t* en_pExtraIMPStrings[] = {
    L"To Commence Actual Profiling, Select Personality.",
    L"Now That You Have Completed Personality, Select Your Attributes.",
    L"With Attributes Now Allocated, You Can Proceed to Portrait Selection.",
    L"To Complete The Process, Select The Voice Sample That Best Fits You.",
};

static wchar_t* en_pFilesTitle[] = {
    L"File Viewer",
};

static wchar_t* en_pFilesSenderList[] = {
    L"Recon Report",  // the recon report sent to the player. Recon is an abbreviation for
                      // reconissance
    L"Intercept #1",  // first intercept file .. Intercept is the title of the organization sending
                      // the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in
                      // files.txt for the translated title
    L"Intercept #2",  // second intercept file
    L"Intercept #3",  // third intercept file
    L"Intercept #4",  // fourth intercept file
    L"Intercept #5",  // fifth intercept file
    L"Intercept #6",  // sixth intercept file
};

// Text having to do with the History Log

static wchar_t* en_pHistoryTitle[] = {
    L"History Log",
};

static wchar_t* en_pHistoryHeaders[] = {
    L"Day",       // the day the history event occurred
    L"Page",      // the current page in the history report we are in
    L"Day",       // the days the history report occurs over
    L"Location",  // location (in sector) the event occurred
    L"Event",     // the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static wchar_t* en_pHistoryStrings[] = {
    L"",  // leave this line blank
    // 1-5
    L"%s was hired from A.I.M.",      // merc was hired from the aim site
    L"%s was hired from M.E.R.C.",    // merc was hired from the aim site
    L"%s died.",                      // merc was killed
    L"Settled Accounts at M.E.R.C.",  // paid outstanding bills at MERC
    L"Accepted Assignment From Enrico Chivaldori",
    // 6-10
    L"IMP Profile Generated",
    L"Purchased Insurance Contract for %s.",  // insurance contract purchased
    L"Canceled Insurance Contract for %s.",   // insurance contract canceled
    L"Insurance Claim Payout for %s.",        // insurance claim payout for merc
    L"Extended %s's contract by a day.",      // Extented "mercs name"'s for a day
    // 11-15
    L"Extended %s's contract by 1 week.",   // Extented "mercs name"'s for a week
    L"Extended %s's contract by 2 weeks.",  // Extented "mercs name"'s 2 weeks
    L"%s was dismissed.",                   // "merc's name" was dismissed.
    L"%s quit.",                            // "merc's name" quit.
    L"quest started.",                      // a particular quest started
    // 16-20
    L"quest completed.",
    L"Talked to head miner of %s",  // talked to head miner of town
    L"Liberated %s",
    L"Cheat Used",
    L"Food should be in Omerta by tomorrow",
    // 21-25
    L"%s left team to become Daryl Hick's wife",
    L"%s's contract expired.",
    L"%s was recruited.",
    L"Enrico complained about lack of progress",
    L"Battle won",
    // 26-30
    L"%s mine started running out of ore",
    L"%s mine ran out of ore",
    L"%s mine was shut down",
    L"%s mine was reopened",
    L"Found out about a prison called Tixa.",
    // 31-35
    L"Heard about a secret weapons plant called Orta.",
    L"Scientist in Orta donated a slew of rocket rifles.",
    L"Queen Deidranna has a use for dead bodies.",
    L"Frank talked about fighting matches in San Mona.",
    L"A patient thinks he saw something in the mines.",
    // 36-40
    L"Met someone named Devin - he sells explosives.",
    L"Ran into the famous ex-AIM merc Mike!",
    L"Met Tony - he deals in arms.",
    L"Got a rocket rifle from Sergeant Krott.",
    L"Gave Kyle the deed to Angel's leather shop.",
    // 41-45
    L"Madlab offered to build a robot.",
    L"Gabby can make stealth concoction for bugs.",
    L"Keith is out of business.",
    L"Howard provided cyanide to Queen Deidranna.",
    L"Met Keith - all purpose dealer in Cambria.",
    // 46-50
    L"Met Howard - deals pharmaceuticals in Balime",
    L"Met Perko - runs a small repair business.",
    L"Met Sam of Balime - runs a hardware shop.",
    L"Franz deals in electronics and other goods.",
    L"Arnold runs a repair shop in Grumm.",
    // 51-55
    L"Fredo repairs electronics in Grumm.",
    L"Received donation from rich guy in Balime.",
    L"Met a junkyard dealer named Jake.",
    L"Some bum gave us an electronic keycard.",
    L"Bribed Walter to unlock the door to the basement.",
    // 56-60
    L"If Dave has gas, he'll provide free fillups.",
    L"Greased Pablo's palms.",
    L"Kingpin keeps money in San Mona mine.",
    L"%s won Extreme Fighting match",
    L"%s lost Extreme Fighting match",
    // 61-65
    L"%s was disqualified in Extreme Fighting",
    L"Found a lot of money stashed in the abandoned mine.",
    L"Encountered assassin sent by Kingpin.",
    L"Lost control of sector",  // ENEMY_INVASION_CODE
    L"Defended sector",
    // 66-70
    L"Lost battle",   // ENEMY_ENCOUNTER_CODE
    L"Fatal ambush",  // ENEMY_AMBUSH_CODE
    L"Wiped out enemy ambush",
    L"Unsuccessful attack",  // ENTERING_ENEMY_SECTOR_CODE
    L"Successful attack!",
    // 71-75
    L"Creatures attacked",   // CREATURE_ATTACK_CODE
    L"Killed by bloodcats",  // BLOODCAT_AMBUSH_CODE
    L"Slaughtered bloodcats",
    L"%s was killed",
    L"Gave Carmen a terrorist's head",
    L"Slay left",
    L"Killed %s",
};

static wchar_t* en_pHistoryLocations[] = {
    L"N/A",  // N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop

static wchar_t* en_pLaptopIcons[] = {
    L"E-mail",      L"Web", L"Financial", L"Personnel", L"History", L"Files", L"Shut Down",
    L"sir-FER 4.0",  // our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static wchar_t* en_pBookMarkStrings[] = {
    L"A.I.M.",   L"Bobby Ray's", L"I.M.P",     L"M.E.R.C.",
    L"Mortuary", L"Florist",     L"Insurance", L"Cancel",
};

static wchar_t* en_pBookmarkTitle[] = {
    L"Bookmarks",
    L"Right click to access this menu in the future.",
};

// When loading or download a web page

static wchar_t* en_pDownloadString[] = {
    L"Downloading",
    L"Reloading",
};

// This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static wchar_t* en_gsAtmSideButtonText[] = {
    L"OK",
    L"Take",    // take money from merc
    L"Give",    // give money to merc
    L"Cancel",  // cancel transaction
    L"Clear",   // clear amount being displayed on the screen
};

static wchar_t* en_gsAtmStartButtonText[] = {
    L"Transfer $",  // transfer money to merc -- short form
    L"Stats",       // view stats of the merc
    L"Inventory",   // view the inventory of the merc
    L"Employment",
};

static wchar_t* en_sATMText[] = {
    L"Transfer Funds?",                   // transfer funds to merc?
    L"Ok?",                               // are we certain?
    L"Enter Amount",                      // enter the amount you want to transfer to merc
    L"Select Type",                       // select the type of transfer to merc
    L"Insufficient Funds",                // not enough money to transfer to merc
    L"Amount must be a multiple of $10",  // transfer amount must be a multiple of $10
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static wchar_t* en_pErrorStrings[] = {
    L"Error",
    L"Server does not have DNS entry.",
    L"Check URL address and try again.",
    L"OK",
    L"Intermittent Connection to Host. Expect longer transfer times.",
};

static wchar_t* en_pPersonnelString[] = {
    L"Mercs:",  // mercs we have
};

static wchar_t* en_pWebTitle[] = {
    L"sir-FER 4.0",  // our name for the version of the browser, play on company name
};

// The titles for the web program title bar, for each page loaded

static wchar_t* en_pWebPagesTitles[] = {
    L"A.I.M.",
    L"A.I.M. Members",
    L"A.I.M. Mug Shots",  // a mug shot is another name for a portrait
    L"A.I.M. Sort",
    L"A.I.M.",
    L"A.I.M. Alumni",
    L"A.I.M. Policies",
    L"A.I.M. History",
    L"A.I.M. Links",
    L"M.E.R.C.",
    L"M.E.R.C. Accounts",
    L"M.E.R.C. Registration",
    L"M.E.R.C. Index",
    L"Bobby Ray's",
    L"Bobby Ray's - Guns",
    L"Bobby Ray's - Ammo",
    L"Bobby Ray's - Armor",
    L"Bobby Ray's - Misc",  // misc is an abbreviation for miscellaneous
    L"Bobby Ray's - Used",
    L"Bobby Ray's - Mail Order",
    L"I.M.P.",
    L"I.M.P.",
    L"United Floral Service",
    L"United Floral Service - Gallery",
    L"United Floral Service - Order Form",
    L"United Floral Service - Card Gallery",
    L"Malleus, Incus & Stapes Insurance Brokers",
    L"Information",
    L"Contract",
    L"Comments",
    L"McGillicutty's Mortuary",
    L"",
    L"URL not found.",
    L"Bobby Ray's - Recent Shipments",
    L"",
    L"",
};

static wchar_t* en_pShowBookmarkString[] = {
    L"Sir-Help",
    L"Click Web Again for Bookmarks.",
};

static wchar_t* en_pLaptopTitles[] = {
    L"Mail Box", L"File Viewer", L"Personnel", L"Bookkeeper Plus", L"History Log",
};

static wchar_t* en_pPersonnelDepartedStateStrings[] = {
    // reasons why a merc has left.
    L"Killed in Action", L"Dismissed", L"Other", L"Married", L"Contract Expired", L"Quit",
};
// personnel strings appearing in the Personnel Manager on the laptop

static wchar_t* en_pPersonelTeamStrings[] = {
    L"Current Team", L"Departures",        L"Daily Cost:", L"Highest Cost:",
    L"Lowest Cost:", L"Killed in Action:", L"Dismissed:",  L"Other:",
};

static wchar_t* en_pPersonnelCurrentTeamStatsStrings[] = {
    L"Lowest",
    L"Average",
    L"Highest",
};

static wchar_t* en_pPersonnelTeamStatsStrings[] = {
    L"HLTH", L"AGI", L"DEX", L"char*", L"LDR", L"WIS", L"LVL", L"MRKM", L"MECH", L"EXPL", L"MED",
};

// horizontal and vertical indices on the map screen

static wchar_t* en_pMapVertIndex[] = {
    L"X", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H",
    L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P",
};

static wchar_t* en_pMapHortIndex[] = {
    L"X", L"1",  L"2",  L"3",  L"4",  L"5",  L"6",  L"7",  L"8",
    L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16",
};

static wchar_t* en_pMapDepthIndex[] = {
    L"",
    L"-1",
    L"-2",
    L"-3",
};

// text that appears on the contract button

static wchar_t* en_pContractButtonString[] = {
    L"Contract",
};

// text that appears on the update panel buttons

static wchar_t* en_pUpdatePanelButtons[] = {
    L"Continue",
    L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static wchar_t* en_LargeTacticalStr[] = {
    L"You have been defeated in this sector!",
    L"The enemy, having no mercy for the team's soul, devours each and every one of you!",
    L"Your unconscious team members have been captured!",
    L"Your team members have been taken prisoner by the enemy.",
};

// Insurance Contract.c
// The text on the buttons at the bottom of the screen.

static wchar_t* en_InsContractText[] = {
    L"Previous",
    L"Next",
    L"Accept",
    L"Clear",
};

// Insurance Info
// Text on the buttons on the bottom of the screen

static wchar_t* en_InsInfoText[] = {
    L"Previous",
    L"Next",
};

// For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static wchar_t* en_MercAccountText[] = {
    // Text on the buttons on the bottom of the screen
    L"Authorize",
    L"Home",
    L"Account #:",
    L"Merc",
    L"Days",
    L"Rate",  // 5
    L"Charge",
    L"Total:",
    L"Are you sure you want to authorize the payment of %s?",  // the %s is a string that contains
                                                               // the dollar amount ( ex. "$150" )
};

// For use at the M.E.R.C. web site. Text relating a MERC mercenary

static wchar_t* en_MercInfo[] = {
    L"Health",
    L"Agility",
    L"Dexterity",
    L"Strength",
    L"Leadership",
    L"Wisdom",
    L"Experience Lvl",
    L"Marksmanship",
    L"Mechanical",
    L"Explosive",
    L"Medical",

    L"Previous",
    L"Hire",
    L"Next",
    L"Additional Info",
    L"Home",
    L"Hired",
    L"Salary:",
    L"Per Day",
    L"Deceased",

    L"Looks like you're trying to hire too many mercs. Your limit is 18.",
    L"Unavailable",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static wchar_t* en_MercNoAccountText[] = {
    // Text on the buttons at the bottom of the screen
    L"Open Account",
    L"Cancel",
    L"You have no account.  Would you like to open one?",
};

// For use at the M.E.R.C. web site. MERC Homepage

static wchar_t* en_MercHomePageText[] = {
    // Description of various parts on the MERC page
    L"Speck T. Kline, founder and owner",
    L"To open an account press here",
    L"To view account press here",
    L"To view files press here",
    // The version number on the video conferencing system that pops up when Speck is talking
    L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static wchar_t* en_sFuneralString[] = {
    L"McGillicutty's Mortuary: Helping families grieve since 1983.",
    L"Funeral Director and former A.I.M. mercenary Murray \"Pops\" McGillicutty is a highly "
    L"skilled and experienced mortician.",
    L"Having been intimately involved in death and bereavement throughout his life, Pops knows how "
    L"difficult it can be.",
    L"McGillicutty's Mortuary offers a wide range of bereavement services, from a shoulder to cry "
    L"on to post-mortem reconstruction for badly disfigured remains.",
    L"Let McGillicutty's Mortuary help you and your loved one rest in peace.",

    // Text for the various links available at the bottom of the page
    L"SEND FLOWERS",
    L"CASKET & URN COLLECTION",
    L"CREMATION SERVICES",
    L"PRE- FUNERAL PLANNING SERVICES",
    L"FUNERAL ETIQUETTE",

    // The text that comes up when you click on any of the links ( except for send flowers ).
    L"Regretably, the remainder of this site has not been completed due to a death in the family. "
    L"Pending reading of the will and disbursement of assets, the site will be completed as soon "
    L"as possible.",
    L"Our sympathies do, however, go out to you at this trying time.  Please come again.",
};

// Text for the florist Home page

static wchar_t* en_sFloristText[] = {
    // Text on the button on the bottom of the page

    L"Gallery",

    // Address of United Florist

    L"\"We air-drop anywhere\"",
    L"1-555-SCENT-ME",
    L"333 NoseGay Dr, Seedy City, CA USA 90210",
    L"http://www.scent-me.com",

    // detail of the florist page

    L"We're fast and efficient!",
    L"Next day delivery to most areas worldwide, guaranteed.  Some restrictions apply.",
    L"Lowest prices in the world, guaranteed!",
    L"Show us a lower advertised price for any arrangements, and receive a dozen roses, absolutely "
    L"free.",
    L"Flying Flora, Fauna & Flowers Since 1981.",
    L"Our decorated ex-bomber aviators will air-drop your bouquet within a ten mile radius of the "
    L"requested location.  Anytime - Everytime!",
    L"Let us satisfy your floral fantasy.",
    L"Let Bruce, our world-renowned floral designer, hand-pick the freshest, highest quality "
    L"flowers from our very own greenhouse.",
    L"And remember, if we don't have it, we can grow it - Fast!",
};

// Florist OrderForm

static wchar_t* en_sOrderFormText[] = {
    // Text on the buttons

    L"Back",
    L"Send",
    L"Clear",
    L"Gallery",

    L"Name of Bouquet:",
    L"Price:",  // 5
    L"Order Number:",
    L"Delivery Date",
    L"next day",
    L"gets there when it gets there",
    L"Delivery Location",  // 10
    L"Additional Services",
    L"Crushed Bouquet($10)",
    L"Black Roses($20)",
    L"Wilted Bouquet($10)",
    L"Fruit Cake (if available)($10)",  // 15
    L"Personal Sentiments:",
    L"Due to the size of gift cards, your message can be no longer than 75 characters.",
    L"...or select from one of our",

    L"STANDARDIZED CARDS",
    L"Billing Information",  // 20

    // The text that goes beside the area where the user can enter their name

    L"Name:",
};

// Florist Gallery.c

static wchar_t* en_sFloristGalleryText[] = {
    // text on the buttons

    L"Prev",  // abbreviation for previous
    L"Next",  // abbreviation for next

    L"Click on the selection you want to order.",
    L"Please Note: there is an additional $10 fee for all wilted or crushed bouquets.",

    // text on the button

    L"Home",
};

// Florist Cards

static wchar_t* en_sFloristCards[] = {
    L"Click on your selection",
    L"Back",
};

// Text for Bobby Ray's Mail Order Site

static wchar_t* en_BobbyROrderFormText[] = {
    L"Order Form",               // Title of the page
    L"Qty",                      // The number of items ordered
    L"Weight (%s)",              // The weight of the item
    L"Item Name",                // The name of the item
    L"Unit Price",               // the item's weight
    L"Total",                    // 5	// The total price of all of items of the same type
    L"Sub-Total",                // The sub total of all the item totals added
    L"S&H (See Delivery Loc.)",  // S&H is an acronym for Shipping and Handling
    L"Grand Total",              // The grand total of all item totals + the shipping and handling
    L"Delivery Location",
    L"Shipping Speed",                     // 10	// See below
    L"Cost (per %s.)",                     // The cost to ship the items
    L"Overnight Express",                  // Gets deliverd the next day
    L"2 Business Days",                    // Gets delivered in 2 days
    L"Standard Service",                   // Gets delivered in 3 days
    L"Clear Order",                        // 15			// Clears the order page
    L"Accept Order",                       // Accept the order
    L"Back",                               // text on the button that returns to the previous page
    L"Home",                               // Text on the button that returns to the home page
    L"* Denotes Used Items",               // Disclaimer stating that the item is used
    L"You can't afford to pay for this.",  // 20	// A popup message that to warn of not
                                           // enough money
    L"<NONE>",                             // Gets displayed when there is no valid city selected
    L"Are you sure you want to send this order to %s?",  // A popup that asks if the city selected
                                                         // is the correct one
    L"Package Weight**",                                 // Displays the weight of the package
    L"** Min. Wt.",  // Disclaimer states that there is a minimum weight for the package
    L"Shipments",
};

// This text is used when on the various Bobby Ray Web site pages that sell items

static wchar_t* en_BobbyRText[] = {
    L"To Order",  // Title
    // instructions on how to order
    L"Click on the item(s). For more than one, keep on clicking. Right click for less. Once you've "
    L"selected all you'd like to buy, go on to the order form.",

    // Text on the buttons to go the various links

    L"Previous Items",  //
    L"Guns",            // 3
    L"Ammo",            // 4
    L"Armor",           // 5
    L"Misc.",           // 6	//misc is an abbreviation for miscellaneous
    L"Used",            // 7
    L"More Items",
    L"ORDER FORM",
    L"Home",  // 10

    // The following 2 lines are used on the Ammunition page.
    // They are used for help text to display how many items the player's merc has
    // that can use this type of ammo

    L"Your team has",                         // 11
    L"weapon(s) that use this type of ammo",  // 12

    // The following lines provide information on the items

    L"Weight:",          // Weight of all the items of the same type
    L"Cal:",             // the caliber of the gun
    L"Mag:",             // number of rounds of ammo the Magazine can hold
    L"Rng:",             // The range of the gun
    L"Dam:",             // Damage of the weapon
    L"ROF:",             // Weapon's Rate Of Fire, acronym ROF
    L"Cost:",            // Cost of the item
    L"In stock:",        // The number of items still in the store's inventory
    L"Qty on Order:",    // The number of items on order
    L"Damaged",          // If the item is damaged
    L"Weight:",          // the Weight of the item
    L"SubTotal:",        // The total cost of all items on order
    L"* %% Functional",  // if the item is damaged, displays the percent function of the item

    // Popup that tells the player that they can only order 10 items at a time

    L"Darn!  This here on-line order form will only accept 10 items per order.  If you're looking "
    L"to order more stuff (and we hope you are), kindly make a separate order and accept our "
    L"apologies.",

    // A popup that tells the user that they are trying to order more items then the store has in
    // stock

    L"Sorry. We don't have any more of that in stock right now. Please try again later.",

    // A popup that tells the user that the store is temporarily sold out

    L"Sorry, but we are currently out of stock on all items of that type.",

};

// Text for Bobby Ray's Home Page

static wchar_t* en_BobbyRaysFrontText[] = {
    // Details on the web site

    L"This is the place to be for the newest and hottest in weaponry and military supplies",
    L"We can find the perfect solution for all your explosives needs",
    L"Used and refitted items",

    // Text for the various links to the sub pages

    L"Miscellaneous",
    L"GUNS",
    L"AMMUNITION",  // 5
    L"ARMOR",

    // Details on the web site

    L"If we don't sell it, you can't get it!",
    L"Under Construction",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug
// shot page

static wchar_t* en_AimSortText[] = {
    L"A.I.M. Members",  // Title
    // Title for the way to sort
    L"Sort By:",

    // sort by...

    L"Price",
    L"Experience",
    L"Marksmanship",
    L"Medical",
    L"Explosives",
    L"Mechanical",

    // Text of the links to other AIM pages

    L"View the mercenary mug shot index",
    L"Review the individual mercenary's file",
    L"Browse the A.I.M. Alumni Gallery",

    // text to display how the entries will be sorted

    L"Ascending",
    L"Descending",
};

// Aim Policies.c
// The page in which the AIM policies and regulations are displayed

static wchar_t* en_AimPolicyText[] = {
    // The text on the buttons at the bottom of the page

    L"Previous Page", L"AIM HomePage", L"Policy Index", L"Next Page", L"Disagree", L"Agree",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot
// index

static wchar_t* en_AimMemberText[] = {
    L"Left Click",
    L"to Contact Merc.",
    L"Right Click",
    L"for Mug Shot Index.",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

static wchar_t* en_CharacterInfo[] = {
    // The various attributes of the merc

    L"Health", L"Agility", L"Dexterity", L"Strength", L"Leadership", L"Wisdom", L"Experience Lvl",
    L"Marksmanship", L"Mechanical", L"Explosive",
    L"Medical",  // 10

    // the contract expenses' area

    L"Fee", L"Contract", L"one day", L"one week", L"two weeks",

    // text for the buttons that either go to the previous merc,
    // start talking to the merc, or go to the next merc

    L"Previous", L"Contact", L"Next",

    L"Additional Info",           // Title for the additional info for the merc's bio
    L"Active Members",            // 20		// Title of the page
    L"Optional Gear:",            // Displays the optional gear cost
    L"MEDICAL deposit required",  // If the merc required a medical deposit, this is displayed
};

// Aim Member.c
// The page in which the player's hires AIM mercenaries

// The following text is used with the video conference popup

static wchar_t* en_VideoConfercingText[] = {
    L"Contract Charge:",  // Title beside the cost of hiring the merc

    // Text on the buttons to select the length of time the merc can be hired

    L"One Day", L"One Week", L"Two Weeks",

    // Text on the buttons to determine if you want the merc to come with the equipment

    L"No Equipment", L"Buy Equipment",

    // Text on the Buttons

    L"TRANSFER FUNDS",  // to actually hire the merc
    L"CANCEL",          // go back to the previous menu
    L"HIRE",            // go to menu in which you can hire the merc
    L"HANG UP",         // stops talking with the merc
    L"OK",
    L"LEAVE MESSAGE",  // if the merc is not there, you can leave a message

    // Text on the top of the video conference popup

    L"Video Conferencing with", L"Connecting. . .",

    L"with medical"  // Displays if you are hiring the merc with the medical deposit
};

// Aim Member.c
// The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static wchar_t* en_AimPopUpText[] = {
    L"ELECTRONIC FUNDS TRANSFER SUCCESSFUL",  // You hired the merc
    L"UNABLE TO PROCESS TRANSFER",            // Player doesn't have enough money, message 1
    L"INSUFFICIENT FUNDS",                    // Player doesn't have enough money, message 2

    // if the merc is not available, one of the following is displayed over the merc's face

    L"On Assignment",
    L"Please Leave Message",
    L"Deceased",

    // If you try to hire more mercs than game can support

    L"You have a full team of 18 mercs already.",

    L"Pre-recorded message",
    L"Message recorded",
};

// AIM Link.c

static wchar_t* en_AimLinkText[] = {
    L"A.I.M. Links",  // The title of the AIM links page
};

// Aim History

// This page displays the history of AIM

static wchar_t* en_AimHistoryText[] = {
    L"A.I.M. History",  // Title

    // Text on the buttons at the bottom of the page

    L"Previous Page",
    L"Home",
    L"A.I.M. Alumni",
    L"Next Page",
};

// Aim Mug Shot Index

// The page in which all the AIM members' portraits are displayed in the order selected by the AIM
// sort page.

static wchar_t* en_AimFiText[] = {
    // displays the way in which the mercs were sorted

    L"Price",
    L"Experience",
    L"Marksmanship",
    L"Medical",
    L"Explosives",
    L"Mechanical",

    // The title of the page, the above text gets added at the end of this text

    L"A.I.M. Members Sorted Ascending By %s",
    L"A.I.M. Members Sorted Descending By %s",

    // Instructions to the players on what to do

    L"Left Click",
    L"To Select Merc",  // 10
    L"Right Click",
    L"For Sorting Options",

    // Gets displayed on top of the merc's portrait if they are...

    L"Away",
    L"Deceased",  // 14
    L"On Assign",
};

// AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer
// with AIM

static wchar_t* en_AimAlumniText[] = {
    // Text of the buttons

    L"PAGE 1", L"PAGE 2", L"PAGE 3",

    L"A.I.M. Alumni",  // Title of the page

    L"DONE"  // Stops displaying information on selected merc
};

// AIM Home Page

static wchar_t* en_AimScreenText[] = {
    // AIM disclaimers

    L"A.I.M. and the A.I.M. logo are registered trademarks in most countries.",
    L"So don't even think of trying to copy us.",
    L"Copyright 1998-1999 A.I.M., Ltd.  All rights reserved.",

    // Text for an advertisement that gets displayed on the AIM page

    L"United Floral Service",
    L"\"We air-drop anywhere\"",  // 10
    L"Do it right",
    L"... the first time",
    L"Guns and stuff, if we dont have it, you dont need it.",
};

// Aim Home Page

static wchar_t* en_AimBottomMenuText[] = {
    // Text for the links at the bottom of all AIM pages
    L"Home", L"Members", L"Alumni", L"Policies", L"History", L"Links",
};

// ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static wchar_t* en_SKI_Text[] = {
    L"MERCHANDISE IN STOCK",  // Header for the merchandise available
    L"PAGE",                  // The current store inventory page being displayed
    L"TOTAL COST",            // The total cost of the the items in the Dealer inventory area
    L"TOTAL VALUE",           // The total value of items player wishes to sell
    L"EVALUATE",              // Button text for dealer to evaluate items the player wants to sell
    L"TRANSACTION",           // Button text which completes the deal. Makes the transaction.
    L"DONE",                  // Text for the button which will leave the shopkeeper interface.
    L"REPAIR COST",           // The amount the dealer will charge to repair the merc's goods
    L"1 HOUR",    // SINGULAR! The text underneath the inventory slot when an item is given to the
                  // dealer to be repaired
    L"%d HOURS",  // PLURAL!   The text underneath the inventory slot when an item is given to the
                  // dealer to be repaired
    L"REPAIRED",  // Text appearing over an item that has just been repaired by a NPC repairman
                  // dealer
    L"There is not enough room in your offer area.",  // Message box that tells the user there is no
                                                      // more room to put there stuff
    L"%d MINUTES",  // The text underneath the inventory slot when an item is given to the dealer to
                    // be repaired
    L"Drop Item To Ground.",
};

// ShopKeeper Interface
// for the bank machine panels. Referenced here is the acronym ATM, which means Automatic Teller
// Machine

static wchar_t* en_SkiAtmText[] = {
    // Text on buttons on the banking machine, displayed at the bottom of the page
    L"0",      L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
    L"OK",      // Transfer the money
    L"Take",    // Take money from the player
    L"Give",    // Give money to the player
    L"Cancel",  // Cancel the transfer
    L"Clear",   // Clear the money display
};

// Shopkeeper Interface
static wchar_t* en_gzSkiAtmText[] = {

    // Text on the bank machine panel that....
    L"Select Type",               // tells the user to select either to give or take from the merc
    L"Enter Amount",              // Enter the amount to transfer
    L"Transfer Funds To Merc",    // Giving money to the merc
    L"Transfer Funds From Merc",  // Taking money from the merc
    L"Insufficient Funds",        // Not enough money to transfer
    L"Balance",                   // Display the amount of money the player currently has
};

static wchar_t* en_SkiMessageBoxText[] = {
    L"Do you want to deduct %s from your main account to cover the difference?",
    L"Not enough funds.  You're short %s",
    L"Do you want to deduct %s from your main account to cover the cost?",
    L"Ask the dealer to start the transaction",
    L"Ask the dealer to repair the selected items",
    L"End conversation",
    L"Current Balance",
};

// OptionScreen.c

static wchar_t* en_zOptionsText[] = {
    // button Text
    L"Save Game",
    L"Load Game",
    L"Quit",
    L"Done",

    // Text above the slider bars
    L"Effects",
    L"Speech",
    L"Music",

    // Confirmation pop when the user selects..
    L"Quit game and return to the main menu?",

    L"You need either the Speech option, or the Subtitle option to be enabled.",
};

// SaveLoadScreen
static wchar_t* en_zSaveLoadText[] = {
    L"Save Game",
    L"Load Game",
    L"Cancel",
    L"Save Selected",
    L"Load Selected",

    L"Saved the game successfully",
    L"ERROR saving the game!",
    L"Loaded the game successfully",
    L"ERROR loading the game!",

    L"The game version in the saved game file is different then the current version.  It is most "
    L"likely safe to continue.  Continue?",
    L"The saved game files may be invalidated.  Do you want them all deleted?",

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Save version has changed.  Please report if there any problems.  Continue?",
#else
    L"Attempting to load an older version save.  Automatically update and load the save?",
#endif

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Save version and game version have changed.  Please report if there are any problems.  "
    L"Continue?",
#else
    L"Attempting to load an older version save.  Automatically update and load the save?",
#endif

    L"Are you sure you want to overwrite the saved game in slot #%d?",
    L"Do you want to load the game from slot #",

    // The first %d is a number that contains the amount of free space on the users hard drive,
    // the second is the recommended amount of free space.
    L"You are running low on disk space.  You only have %d Megs free and Jagged should have at "
    L"least %d Megs free.",

    L"Saving...",  // When saving a game, a message box with this string appears on the screen

    L"Normal Guns",
    L"Tons of Guns",
    L"Realistic style",
    L"Sci Fi style",

    L"Difficulty",
};

// MapScreen
static wchar_t* en_zMarksMapScreenText[] = {
    L"Map Level",
    L"You have no militia.  You need to train town residents in order to have a town militia.",
    L"Daily Income",
    L"Merc has life insurance",
    L"%s isn't tired.",
    L"%s is on the move and can't sleep",
    L"%s is too tired, try a little later.",
    L"%s is driving.",
    L"Squad can't move with a sleeping merc on it.",

    // stuff for contracts
    L"While you can pay for the contract, you don't have the bucks to cover this merc's life "
    L"insurance premium.",
    L"%s insurance premium will cost %s for %d extra day(s). Do you want to pay?",
    L"Sector Inventory",
    L"Merc has a medical deposit.",

    // other items
    L"Medics",    // people acting a field medics and bandaging wounded mercs
    L"Patients",  // people who are being bandaged by a medic
    L"Done",      // Continue on with the game after autobandage is complete
    L"Stop",      // Stop autobandaging of patients by medics now
    L"Sorry. This option has been disabled in this demo.",  // informs player this option/button has
                                                            // been disabled in the demo
    L"%s doesn't have a repair kit.",
    L"%s doesn't have a medical kit.",
    L"There aren't enough people willing to be trained right now.",
    L"%s is full of militia.",
    L"Merc has a finite contract.",
    L"Merc's contract is not insured",
};

static wchar_t* en_pLandMarkInSectorString[] = {
    L"Squad %d has noticed someone in sector %s",
};

// confirm the player wants to pay X dollars to build a militia force in town
static wchar_t* en_pMilitiaConfirmStrings[] = {
    L"Training a squad of town militia will cost $",  // telling player how much it will cost
    L"Approve expenditure?",  // asking player if they wish to pay the amount requested
    L"You can't afford it.",  // telling the player they can't afford to train this town
    L"Continue training militia in %s (%s %d)?",  // continue training this town?
    L"Cost $",                                    // the cost in dollars to train militia
    L"( Y/N )",                                   // abbreviated yes/no
    L"",                                          // unused
    L"Training town militia in %d sectors will cost $ %d. %s",  // cost to train sveral sectors at
                                                                // once
    L"You cannot afford the $%d to train town militia here.",
    L"%s needs a loyalty of %d percent for you to be able to continue training militia.",
    L"You cannot train the militia in %s any further.",
};

// Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom
// of the single merc panel
static wchar_t* en_gzMoneyWithdrawMessageText[] = {
    L"You can only withdraw up to $20,000 at a time.",
    L"Are you sure you want to deposit the %s into your account?",
};

static wchar_t* en_gzCopyrightText[] = {
    L"Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.",
};

// option Text
static wchar_t* en_zOptionsToggleText[] = {
    L"Speech",
    L"Mute Confirmations",
    L"SubTitles",
    L"Pause Text Dialogue",
    L"Animate Smoke",
    L"Blood n Gore",
    L"Never Move My Mouse!",
    L"Old Selection Method",
    L"Show Movement Path",
    L"Show Misses",
    L"Real Time Confirmation",
    L"Display sleep/wake notifications",
    L"Use Metric System",
    L"Merc Lights during Movement",
    L"Snap Cursor to Mercs",
    L"Snap Cursor to Doors",
    L"Make Items Glow",
    L"Show Tree Tops",
    L"Show Wireframes",
    L"Show 3D Cursor",
};

// This is the help text associated with the above toggles.
static wchar_t* en_zOptionsScreenHelpText[] = {
    // speech
    L"Keep this option ON if you want to hear character dialogue.",

    // Mute Confirmation
    L"Turns characters' verbal confirmations on or off.",

    // Subtitles
    L"Controls whether on-screen text is displayed for dialogue.",

    // Key to advance speech
    L"If Subtitles are ON, turn this on also to be able to take your time reading NPC dialogue.",

    // Toggle smoke animation
    L"Turn off this option if animating smoke slows down your game's framerate.",

    // Blood n Gore
    L"Turn this option OFF if blood offends you.",

    // Never move my mouse
    L"Turn this option OFF to have your mouse automatically move over pop-up confirmation boxes "
    L"when they appear.",

    // Old selection method
    L"Turn this ON for character selection to work as in previous JAGGED ALLIANCE games (which is "
    L"the opposite of how it works otherwise).",

    // Show movement path
    L"Turn this ON to display movement paths in Real-time (or leave it off and use the SHIFT key "
    L"when you do want them displayed).",

    // show misses
    L"Turn ON to have the game show you where your bullets ended up when you \"miss\".",

    // Real Time Confirmation
    L"When ON, an additional \"safety\" click will be required for movement in Real-time.",

    // Sleep/Wake notification
    L"When ON, you will be notified when mercs on \"assignment\" go to sleep and resume work.",

    // Use the metric system
    L"When ON, uses the metric system for measurements; otherwise it uses the Imperial system.",

    // Merc Lighted movement
    L"When ON, the merc will light the ground while walking.  Turn OFF for faster frame rate.",

    // Smart cursor
    L"When ON, moving the cursor near your mercs will automatically highlight them.",

    // snap cursor to the door
    L"When ON, moving the cursor near a door will automatically position the cursor over the door.",

    // glow items
    L"When ON, |Items continuously glow",

    // toggle tree tops
    L"When ON, shows the |Tree tops.",

    // toggle wireframe
    L"When ON, displays |Wireframes for obscured walls.",

    L"When ON, the movement cursor is shown in 3D. ( |Home )",

};

static wchar_t* en_gzGIOScreenText[] = {
    L"INITIAL GAME SETTINGS",
    L"Game Style",
    L"Realistic",
    L"Sci Fi",
    L"Gun Options",
    L"Tons of Guns",
    L"Normal",
    L"Difficulty Level",
    L"Novice",
    L"Experienced",
    L"Expert",
    L"Ok",
    L"Cancel",
    L"Extra Difficulty",
    L"Save Anytime",
    L"Iron Man",
    L"Disabled for Demo",
};

static wchar_t* en_pDeliveryLocationStrings[] = {
    L"Austin",       // Austin, Texas, USA
    L"Baghdad",      // Baghdad, Iraq (Suddam Hussein's home)
    L"Drassen",      // The main place in JA2 that you can receive items.  The other towns are dummy
                     // names...
    L"Hong Kong",    // Hong Kong, Hong Kong
    L"Beirut",       // Beirut, Lebanon	(Middle East)
    L"London",       // London, England
    L"Los Angeles",  // Los Angeles, California, USA (SW corner of USA)
    L"Meduna",       // Meduna -- the other airport in JA2 that you can receive items.
    L"Metavira",     // The island of Metavira was the fictional location used by JA1
    L"Miami",        // Miami, Florida, USA (SE corner of USA)
    L"Moscow",       // Moscow, USSR
    L"New York",     // New York, New York, USA
    L"Ottawa",       // Ottawa, Ontario, Canada -- where JA2 was made!
    L"Paris",        // Paris, France
    L"Tripoli",      // Tripoli, Libya (eastern Mediterranean)
    L"Tokyo",        // Tokyo, Japan
    L"Vancouver",    // Vancouver, British Columbia, Canada (west coast near US border)
};

static wchar_t* en_pSkillAtZeroWarning[] = {
    // This string is used in the IMP character generation.  It is possible to select 0 ability
    // in a skill meaning you can't use it.  This text is confirmation to the player.
    L"Are you sure? A value of zero means NO ability in this skill.",
};

static wchar_t* en_pIMPBeginScreenStrings[] = {
    L"( 8 Characters Max )",
};

static wchar_t* en_pIMPFinishButtonText[1] = {
    L"Analyzing",
};

static wchar_t* en_pIMPFinishStrings[] = {
    L"Thank You, %s",  //%s is the name of the merc
};

// the strings for imp voices screen
static wchar_t* en_pIMPVoicesStrings[] = {
    L"Voice",
};

static wchar_t* en_pDepartedMercPortraitStrings[] = {
    L"Killed in Action",
    L"Dismissed",
    L"Other",
};

// title for program
static wchar_t* en_pPersTitleText[] = {
    L"Personnel Manager",
};

// paused game strings
static wchar_t* en_pPausedGameText[] = {
    L"Game Paused",
    L"Resume Game (|P|a|u|s|e)",
    L"Pause Game (|P|a|u|s|e)",
};

static wchar_t* en_pMessageStrings[] = {
    L"Exit Game?",
    L"OK",
    L"YES",
    L"NO",
    L"CANCEL",
    L"REHIRE",
    L"LIE",
    L"No description",  // Save slots that don't have a description.
    L"Game Saved.",
    L"Game Saved.",
    L"QuickSave",  // The name of the quicksave file (filename, text reference)
    L"SaveGame",   // The name of the normal savegame file, such as SaveGame01, SaveGame02, etc.
    L"sav",        // The 3 character dos extension (represents sav)
    L"..\\SavedGames",  // The name of the directory where games are saved.
    L"Day",
    L"Mercs",
    L"Empty Slot",  // An empty save game slot
    L"Demo",        // Demo of JA2
    L"Debug",       // State of development of a project (JA2) that is a debug build
    L"Release",     // Release build for JA2
    L"rpm",   // Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
    L"min",   // Abbreviation for minute.
    L"m",     // One character abbreviation for meter (metric distance measurement unit).
    L"rnds",  // Abbreviation for rounds (# of bullets)
    L"kg",    // Abbreviation for kilogram (metric weight measurement unit)
    L"lb",    // Abbreviation for pounds (Imperial weight measurement unit)
    L"Home",  // Home as in homepage on the internet.
    L"USD",   // Abbreviation to US dollars
    L"n/a",   // Lowercase acronym for not applicable.
    L"Meanwhile",                      // Meanwhile
    L"%s has arrived in sector %s%s",  // Name/Squad has arrived in sector A9.  Order must not
                                       // change without notifying SirTech
    L"Version",
    L"Empty Quick Save Slot",
    L"This slot is reserved for Quick Saves made from the tactical and map screens using ALT+S.",
    L"Opened",
    L"Closed",
    L"You are running low on disk space.  You only have %sMB free and Jagged Alliance 2 requires "
    L"%sMB.",
    L"Hired %s from AIM",
    L"%s has caught %s.",  //'Merc name' has caught 'item' -- let SirTech know if name comes after
                           // item.
    L"%s has taken the drug.",   //'Merc name' has taken the drug
    L"%s has no medical skill",  //'Merc name' has no medical skill.

    // CDRom errors (such as ejecting CD while attempting to read the CD)
    L"The integrity of the game has been compromised.",
    L"ERROR: Ejected CD-ROM",

    // When firing heavier weapons in close quarters, you may not have enough room to do so.
    L"There is no room to fire from here.",

    // Can't change stance due to objects in the way...
    L"Cannot change stance at this time.",

    // Simple text indications that appear in the game, when the merc can do one of these things.
    L"Drop",
    L"Throw",
    L"Pass",

    L"%s passed to %s.",  //"Item" passed to "merc".  Please try to keep the item %s before the merc
                          //%s, otherwise, must notify SirTech.
    L"No room to pass %s to %s.",  // pass "item" to "merc".  Same instructions as above.

    // A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
    L" Attached )",

    // Cheat modes
    L"Cheat level ONE reached",
    L"Cheat level TWO reached",

    // Toggling various stealth modes
    L"Squad on stealth mode.",
    L"Squad off stealth mode.",
    L"%s on stealth mode.",
    L"%s off stealth mode.",

    // Wireframes are shown through buildings to reveal doors and windows that can't otherwise be
    // seen in an isometric engine.  You can toggle this mode freely in the game.
    L"Extra Wireframes On",
    L"Extra Wireframes Off",

    // These are used in the cheat modes for changing levels in the game.  Going from a basement
    // level to an upper level, etc.
    L"Can't go up from this level...",
    L"There are no lower levels...",
    L"Entering basement level %d...",
    L"Leaving basement...",

    L"'s",  // used in the shop keeper inteface to mark the ownership of the item eg Red's gun
    L"Follow mode OFF.",
    L"Follow mode ON.",
    L"3D Cursor OFF.",
    L"3D Cursor ON.",
    L"Squad %d active.",
    L"You cannot afford to pay for %s's daily salary of %s",  // first %s is the mercs name, the
                                                              // seconds is a string containing the
                                                              // salary
    L"Skip",
    L"%s cannot leave alone.",
    L"A save has been created called, SaveGame99.sav.  If needed, rename it to SaveGame01 - "
    L"SaveGame10 and then you will have access to it in the Load screen.",
    L"%s drank some %s",
    L"A package has arrived in Drassen.",
    L"%s should arrive at the designated drop-off point (sector %s) on day %d, at approximately "
    L"%s.",  // first %s is mercs name, next is the sector location and name where they will be
             // arriving in, lastely is the day an the time of arrival
    L"History log updated.",
#ifdef JA2BETAVERSION
    L"Successfully Saved the Game into the End Turn Auto Save slot.",
#endif
};

static wchar_t* en_ItemPickupHelpPopup[] = {
    L"OK", L"Scroll Up", L"Select All", L"Scroll Down", L"Cancel",
};

static wchar_t* en_pDoctorWarningString[] = {
    L"%s isn't close enough to be healed.",
    L"Your medics were unable to completely bandage everyone.",
};

static wchar_t* en_pMilitiaButtonsHelpText[] = {
    L"Pick up(Right Click)/drop(Left Click) Green Troops",  // button help text informing player
                                                            // they can pick up or drop militia with
                                                            // this button
    L"Pick up(Right Click)/drop(Left Click) Regular Troops",
    L"Pick up(Right Click)/drop(Left Click) Veteran Troops",
    L"Distribute available militia equally among all sectors",
};

static wchar_t* en_pMapScreenJustStartedHelpText[] = {
    L"Go to AIM and hire some mercs ( *Hint* it's in the Laptop )",  // to inform the player to
                                                                     // hired some mercs to get
                                                                     // things going
    L"When you're ready to travel to Arulco, click on the Time Compression button at the bottom "
    L"right of the screen.",  // to inform the player to hit time compression to get the game
                              // underway
};

static wchar_t* en_pAntiHackerString[] = {
    L"Error. Missing or corrupted file(s). Game will exit now.",
};

static wchar_t* en_gzLaptopHelpText[] = {
    // Buttons:
    L"View email",
    L"Browse various web sites",
    L"View files and email attachments",
    L"Read log of events",
    L"View team info",
    L"View financial summary and history",
    L"Close laptop",

    // Bottom task bar icons (if they exist):
    L"You have new mail",
    L"You have new file(s)",

    // Bookmarks:
    L"Association of International Mercenaries",
    L"Bobby Ray's online weapon mail order",
    L"Institute of Mercenary Profiling",
    L"More Economic Recruiting Center",
    L"McGillicutty's Mortuary",
    L"United Floral Service",
    L"Insurance Brokers for A.I.M. contracts",
};

static wchar_t* en_gzHelpScreenText[] = {
    L"Exit help screen",
};

static wchar_t* en_gzNonPersistantPBIText[] = {
    L"There is a battle in progress. You can only retreat from the tactical screen.",
    L"|Enter sector to continue the current battle in progress.",
    L"|Automatically resolves the current battle.",
    L"You can't automatically resolve a battle when you are the attacker.",
    L"You can't automatically resolve a battle while you are being ambushed.",
    L"You can't automatically resolve a battle while you are fighting creatures in the mines.",
    L"You can't automatically resolve a battle while there are hostile civilians.",
    L"You can't automatically resolve a battle while there are bloodcats.",
    L"BATTLE IN PROGRESS",
    L"You cannot retreat at this time.",
};

static wchar_t* en_gzMiscString[] = {
    L"Your militia continue to battle without the aid of your mercs...",
    L"The vehicle does not need anymore fuel right now.",
    L"The fuel tank is %d%% full.",
    L"Deidranna's army has regained complete control over %s.",
    L"You have lost a refueling site.",
};

static wchar_t* en_gzIntroScreen[] = {
    L"Cannot find intro video",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static wchar_t* en_pNewNoiseStr[] = {
    L"%s hears a %s sound coming from %s.",
    L"%s hears a %s sound of MOVEMENT coming from %s.",
    L"%s hears a %s CREAKING coming from %s.",
    L"%s hears a %s SPLASHING coming from %s.",
    L"%s hears a %s IMPACT coming from %s.",
    L"%s hears a %s EXPLOSION to %s.",
    L"%s hears a %s SCREAM to %s.",
    L"%s hears a %s IMPACT to %s.",
    L"%s hears a %s IMPACT to %s.",
    L"%s hears a %s SHATTERING coming from %s.",
    L"%s hears a %s SMASH coming from %s.",
};

static wchar_t* en_wMapScreenSortButtonHelpText[] = {
    L"Sort by Name (|F|1)",     L"Sort by Assignment (|F|2)",  L"Sort by Sleep Status (|F|3)",
    L"Sort by Location (|F|4)", L"Sort by Destination (|F|5)", L"Sort by Departure Time (|F|6)",
};

static wchar_t* en_BrokenLinkText[] = {
    L"Error 404",
    L"Site not found.",
};

static wchar_t* en_gzBobbyRShipmentText[] = {
    L"Recent Shipments",
    L"Order #",
    L"Number Of Items",
    L"Ordered On",
};

static wchar_t* en_gzCreditNames[] = {
    L"Chris Camfield",
    L"Shaun Lyng",
    L"Kris Märnes",
    L"Ian Currie",
    L"Linda Currie",
    L"Eric \"WTF\" Cheng",
    L"Lynn Holowka",
    L"Norman \"NRG\" Olsen",
    L"George Brooks",
    L"Andrew Stacey",
    L"Scot Loving",
    L"Andrew \"Big Cheese\" Emmons",
    L"Dave \"The Feral\" French",
    L"Alex Meduna",
    L"Joey \"Joeker\" Whelan",
};

static wchar_t* en_gzCreditNameTitle[] = {
    L"Game Internals Programmer",                    // Chris Camfield
    L"Co-designer/Writer",                           // Shaun Lyng
    L"Strategic Systems & Editor Programmer",        // Kris \"The Cow Rape Man\" Marnes
    L"Producer/Co-designer",                         // Ian Currie
    L"Co-designer/Map Designer",                     // Linda Currie
    L"Artist",                                       // Eric \"WTF\" Cheng
    L"Beta Coordinator, Support",                    // Lynn Holowka
    L"Artist Extraordinaire",                        // Norman \"NRG\" Olsen
    L"Sound Guru",                                   // George Brooks
    L"Screen Designer/Artist",                       // Andrew Stacey
    L"Lead Artist/Animator",                         // Scot Loving
    L"Lead Programmer",                              // Andrew \"Big Cheese Doddle\" Emmons
    L"Programmer",                                   // Dave French
    L"Strategic Systems & Game Balance Programmer",  // Alex Meduna
    L"Portraits Artist",                             // Joey \"Joeker\" Whelan",
};

static wchar_t* en_gzCreditNameFunny[] = {
    L"",                                       // Chris Camfield
    L"(still learning punctuation)",           // Shaun Lyng
    L"(\"It's done. I'm just fixing it\")",    // Kris \"The Cow Rape Man\" Marnes
    L"(getting much too old for this)",        // Ian Currie
    L"(and working on Wizardry 8)",            // Linda Currie
    L"(forced at gunpoint to also do QA)",     // Eric \"WTF\" Cheng
    L"(Left us for the CFSA - go figure...)",  // Lynn Holowka
    L"",                                       // Norman \"NRG\" Olsen
    L"",                                       // George Brooks
    L"(Dead Head and jazz lover)",             // Andrew Stacey
    L"(his real name is Robert)",              // Scot Loving
    L"(the only responsible person)",          // Andrew \"Big Cheese Doddle\" Emmons
    L"(can now get back to motocrossing)",     // Dave French
    L"(stolen from Wizardry 8)",               // Alex Meduna
    L"(did items and loading screens too!)",   // Joey \"Joeker\" Whelan",
};

static wchar_t* en_sRepairsDoneString[] = {
    L"%s finished repairing own items",
    L"%s finished repairing everyone's guns & armor",
    L"%s finished repairing everyone's equipped items",
    L"%s finished repairing everyone's carried items",
};

static wchar_t* en_zGioDifConfirmText[] = {
    L"You have chosen NOVICE mode. This setting is appropriate for those new to Jagged Alliance, "
    L"those new to strategy games in general, or those wishing shorter battles in the game. Your "
    L"choice will affect things throughout the entire course of the game, so choose wisely. Are "
    L"you sure you want to play in Novice mode?",
    L"You have chosen EXPERIENCED mode. This setting is suitable for those already familiar with "
    L"Jagged Alliance or similar games. Your choice will affect things throughout the entire "
    L"course of the game, so choose wisely. Are you sure you want to play in Experienced mode?",
    L"You have chosen EXPERT mode. We warned you. Don't blame us if you get shipped back in a body "
    L"bag. Your choice will affect things throughout the entire course of the game, so choose "
    L"wisely. Are you sure you want to play in Expert mode?",
};

static wchar_t* en_gzLateLocalizedString[] = {
    L"%S loadscreen data file not found...",

    // 1-5
    L"The robot cannot leave this sector when nobody is using the controller.",

    // This message comes up if you have pending bombs waiting to explode in tactical.
    L"You can't compress time right now.  Wait for the fireworks!",

    //'Name' refuses to move.
    L"%s refuses to move.",

    //%s a merc name
    L"%s does not have enough energy to change stance.",

    // A message that pops up when a vehicle runs out of gas.
    L"The %s has run out of gas and is now stranded in %c%d.",

    // 6-10

    // the following two strings are combined with the pNewNoise[] strings above to report noises
    // heard above or below the merc
    L"above",
    L"below",

    // The following strings are used in autoresolve for autobandaging related feedback.
    L"None of your mercs have any medical ability.",
    L"There are no medical supplies to perform bandaging.",
    L"There weren't enough medical supplies to bandage everybody.",
    L"None of your mercs need bandaging.",
    L"Bandages mercs automatically.",
    L"All your mercs are bandaged.",

    // 14
    L"Arulco",

    L"(roof)",

    L"Health: %d/%d",

    // In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
    //"vs." is the abbreviation of versus.
    L"%d vs. %d",

    L"The %s is full!",  //(ex "The ice cream truck is full")

    L"%s does not need immediate first aid or bandaging but rather more serious medical attention "
    L"and/or rest.",

    // 20
    // Happens when you get shot in the legs, and you fall down.
    L"%s is hit in the leg and collapses!",
    // Name can't speak right now.
    L"%s can't speak right now.",

    // 22-24 plural versions
    L"%d green militia have been promoted to veteran militia.",
    L"%d green militia have been promoted to regular militia.",
    L"%d regular militia have been promoted to veteran militia.",

    // 25
    L"Switch",

    // 26
    // Name has gone psycho -- when the game forces the player into burstmode (certain unstable
    // characters)
    L"%s goes psycho!",

    // 27-28
    // Messages why a player can't time compress.
    L"It is currently unsafe to compress time because you have mercs in sector %s.",
    L"It is currently unsafe to compress time when mercs are in the creature infested mines.",

    // 29-31 singular versions
    L"1 green militia has been promoted to a veteran militia.",
    L"1 green militia has been promoted to a regular militia.",
    L"1 regular militia has been promoted to a veteran militia.",

    // 32-34
    L"%s doesn't say anything.",
    L"Travel to surface?",
    L"(Squad %d)",

    // 35
    // Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope,
    // Scope before MP5K)
    L"%s has repaired %s's %s",

    // 36
    L"BLOODCAT",

    // 37-38 "Name trips and falls"
    L"%s trips and falls",
    L"This item can't be picked up from here.",

    // 39
    L"None of your remaining mercs are able to fight.  The militia will fight the creatures on "
    L"their own.",

    // 40-43
    //%s is the name of merc.
    L"%s ran out of medical kits!",
    L"%s lacks the necessary skill to doctor anyone!",
    L"%s ran out of tool kits!",
    L"%s lacks the necessary skill to repair anything!",

    // 44-45
    L"Repair Time",
    L"%s cannot see this person.",

    // 46-48
    L"%s's gun barrel extender falls off!",
    L"No more than %d militia trainers are permitted per sector.",
    L"Are you sure?",

    // 49-50
    L"Time Compression",
    L"The vehicle's gas tank is now full.",

    // 51-52 Fast help text in mapscreen.
    L"Continue Time Compression (|S|p|a|c|e)",
    L"Stop Time Compression (|E|s|c)",

    // 53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
    L"%s has unjammed the %s",
    L"%s has unjammed %s's %s",

    // 55
    L"Can't compress time while viewing sector inventory.",

    L"The Jagged Alliance 2 PLAY DISK was not found. Program will now exit.",

    L"Items successfully combined.",

    // 58
    // Displayed with the version information when cheats are enabled.
    L"Current/Max Progress: %d%%/%d%%",

    L"Escort John and Mary?",

    // 60
    L"Switch Activated.",

    L"%s's ceramic plates have been smashed!",
};

// VERY TRUNCATED FILE COPIED FROM JA2.5 FOR ITS FEATURES FOR JA2 GOLD
wchar_t* en_zNewTacticalMessages[] = {
    L"Range to target: %d tiles",
    L"Attaching the transmitter to your laptop computer.",
    L"You cannot afford to hire %s",
    L"For a limited time, the above fee covers the cost of the entire mission and includes the "
    L"equipment listed below.",
    L"Hire %s now and take advantage of our unprecedented 'one fee covers all' pricing.  Also "
    L"included in this unbelievable offer is the mercenary's personal equipment at no charge.",
    L"Fee",
    L"There is someone else in the sector...",
    L"Gun Range: %d tiles, Range to target: %d tiles",
    L"Display Cover",
    L"Line of Sight",
    L"New Recruits cannot arrive there.",
    L"Since your laptop has no transmitter, you won't be able to hire new team members.  Perhaps "
    L"this would be a good time to load a saved game or start over!",
    L"%s hears the sound of crumpling metal coming from underneath Jerry's body.  It sounds "
    L"disturbingly like your laptop antenna being crushed.",
    L"After scanning the note left behind by Deputy Commander Morris, %s senses an oppurtinity.  "
    L"The note contains the coordinates for launching missiles against different towns in Arulco.  "
    L"It also gives the coodinates of the origin - the missile facility.",
    L"Noticing the control panel, %s figures the numbers can be reveresed, so that the missile "
    L"might destroy this very facility.  %s needs to find an escape route.  The elevator appears "
    L"to offer the fastest solution...",
    L"This is an IRON MAN game and you cannot save when enemies are around.",
    L"(Cannot save during combat)",
    L"The current campaign name is greater than 30 characters.",
    L"The current campaign cannot be found.",
    L"Campaign: Default ( %S )",
    L"Campaign: %S",
    L"You have selected the campaign %S. This campaign is a player-modified version of the "
    L"original Unfinished Business campaign. Are you sure you wish to play the %S "
    L"campaign?",
    L"In order to use the editor, please select a campaign other than the default.",
};

wchar_t* en_gzIronManModeWarningText[] = {
    L"You have chosen IRON MAN mode. This setting makes the game considerably more challenging as "
    L"you will not be able to save your game when in a sector occupied by enemies. This setting "
    L"will affect the entire course of the game.  Are you sure want to play in IRON MAN mode?",
};

void UseTextEnglish() {
  gzProsLabel = en_gzProsLabel;
  gzConsLabel = en_gzConsLabel;
  sRepairsDoneString = en_sRepairsDoneString;
  AmmoCaliber = en_AmmoCaliber;
  BobbyRayAmmoCaliber = en_BobbyRayAmmoCaliber;
  WeaponType = en_WeaponType;
  Message = en_Message;
  TeamTurnString = en_TeamTurnString;
  pAssignMenuStrings = en_pAssignMenuStrings;
  pTrainingStrings = en_pTrainingStrings;
  pTrainingMenuStrings = en_pTrainingMenuStrings;
  pAttributeMenuStrings = en_pAttributeMenuStrings;
  pVehicleStrings = en_pVehicleStrings;
  pShortAttributeStrings = en_pShortAttributeStrings;
  pLongAttributeStrings = en_pLongAttributeStrings;
  pContractStrings = en_pContractStrings;
  pAssignmentStrings = en_pAssignmentStrings;
  pConditionStrings = en_pConditionStrings;
  pTownNames = en_pTownNames;
  pPersonnelScreenStrings = en_pPersonnelScreenStrings;
  pPersonnelTitle = en_pPersonnelTitle;
  pUpperLeftMapScreenStrings = en_pUpperLeftMapScreenStrings;
  pTacticalPopupButtonStrings = en_pTacticalPopupButtonStrings;
  pSquadMenuStrings = en_pSquadMenuStrings;
  pDoorTrapStrings = en_pDoorTrapStrings;
  pLongAssignmentStrings = en_pLongAssignmentStrings;
  pContractExtendStrings = en_pContractExtendStrings;
  pMapScreenMouseRegionHelpText = en_pMapScreenMouseRegionHelpText;
  pPersonnelAssignmentStrings = en_pPersonnelAssignmentStrings;
  pNoiseVolStr = en_pNoiseVolStr;
  pNoiseTypeStr = en_pNoiseTypeStr;
  pDirectionStr = en_pDirectionStr;
  pRemoveMercStrings = en_pRemoveMercStrings;
  sTimeStrings = en_sTimeStrings;
  pLandTypeStrings = en_pLandTypeStrings;
  pGuardMenuStrings = en_pGuardMenuStrings;
  pOtherGuardMenuStrings = en_pOtherGuardMenuStrings;
  pInvPanelTitleStrings = en_pInvPanelTitleStrings;
  pPOWStrings = en_pPOWStrings;
  pMilitiaString = en_pMilitiaString;
  pMilitiaButtonString = en_pMilitiaButtonString;
  pEpcMenuStrings = en_pEpcMenuStrings;
  pRepairStrings = en_pRepairStrings;
  sPreStatBuildString = en_sPreStatBuildString;
  sStatGainStrings = en_sStatGainStrings;
  pHelicopterEtaStrings = en_pHelicopterEtaStrings;
  sMapLevelString = en_sMapLevelString;
  gsLoyalString = en_gsLoyalString;
  gsUndergroundString = en_gsUndergroundString;
  gsTimeStrings = en_gsTimeStrings;
  sFacilitiesStrings = en_sFacilitiesStrings;
  pMapPopUpInventoryText = en_pMapPopUpInventoryText;
  pwTownInfoStrings = en_pwTownInfoStrings;
  pwMineStrings = en_pwMineStrings;
  pwMiscSectorStrings = en_pwMiscSectorStrings;
  pMapInventoryErrorString = en_pMapInventoryErrorString;
  pMapInventoryStrings = en_pMapInventoryStrings;
  pMapScreenFastHelpTextList = en_pMapScreenFastHelpTextList;
  pMovementMenuStrings = en_pMovementMenuStrings;
  pUpdateMercStrings = en_pUpdateMercStrings;
  pMapScreenBorderButtonHelpText = en_pMapScreenBorderButtonHelpText;
  pMapScreenBottomFastHelp = en_pMapScreenBottomFastHelp;
  pMapScreenBottomText = en_pMapScreenBottomText;
  pMercDeadString = en_pMercDeadString;
  pSenderNameList = en_pSenderNameList;
  pTraverseStrings = en_pTraverseStrings;
  pNewMailStrings = en_pNewMailStrings;
  pDeleteMailStrings = en_pDeleteMailStrings;
  pEmailHeaders = en_pEmailHeaders;
  pEmailTitleText = en_pEmailTitleText;
  pFinanceTitle = en_pFinanceTitle;
  pFinanceSummary = en_pFinanceSummary;
  pFinanceHeaders = en_pFinanceHeaders;
  pTransactionText = en_pTransactionText;
  pTransactionAlternateText = en_pTransactionAlternateText;
  pMoralStrings = en_pMoralStrings;
  pSkyriderText = en_pSkyriderText;
  pLeftEquipmentString = en_pLeftEquipmentString;
  pMapScreenStatusStrings = en_pMapScreenStatusStrings;
  pMapScreenPrevNextCharButtonHelpText = en_pMapScreenPrevNextCharButtonHelpText;
  pEtaString = en_pEtaString;
  pShortVehicleStrings = en_pShortVehicleStrings;
  pTrashItemText = en_pTrashItemText;
  pMapErrorString = en_pMapErrorString;
  pMapPlotStrings = en_pMapPlotStrings;
  pMiscMapScreenMouseRegionHelpText = en_pMiscMapScreenMouseRegionHelpText;
  pMercHeLeaveString = en_pMercHeLeaveString;
  pMercSheLeaveString = en_pMercSheLeaveString;
  pImpPopUpStrings = en_pImpPopUpStrings;
  pImpButtonText = en_pImpButtonText;
  pExtraIMPStrings = en_pExtraIMPStrings;
  pFilesTitle = en_pFilesTitle;
  pFilesSenderList = en_pFilesSenderList;
  pHistoryLocations = en_pHistoryLocations;
  pHistoryStrings = en_pHistoryStrings;
  pHistoryHeaders = en_pHistoryHeaders;
  pHistoryTitle = en_pHistoryTitle;
  pShowBookmarkString = en_pShowBookmarkString;
  pWebPagesTitles = en_pWebPagesTitles;
  pWebTitle = en_pWebTitle;
  pPersonnelString = en_pPersonnelString;
  pErrorStrings = en_pErrorStrings;
  pDownloadString = en_pDownloadString;
  pBookmarkTitle = en_pBookmarkTitle;
  pBookMarkStrings = en_pBookMarkStrings;
  pLaptopIcons = en_pLaptopIcons;
  sATMText = en_sATMText;
  gsAtmStartButtonText = en_gsAtmStartButtonText;
  gsAtmSideButtonText = en_gsAtmSideButtonText;
  pPersonnelTeamStatsStrings = en_pPersonnelTeamStatsStrings;
  pPersonnelCurrentTeamStatsStrings = en_pPersonnelCurrentTeamStatsStrings;
  pPersonelTeamStrings = en_pPersonelTeamStrings;
  pPersonnelDepartedStateStrings = en_pPersonnelDepartedStateStrings;
  pMapHortIndex = en_pMapHortIndex;
  pMapVertIndex = en_pMapVertIndex;
  pMapDepthIndex = en_pMapDepthIndex;
  pLaptopTitles = en_pLaptopTitles;
  pDayStrings = en_pDayStrings;
  pMercContractOverStrings = en_pMercContractOverStrings;
  pMilitiaConfirmStrings = en_pMilitiaConfirmStrings;
  pDeliveryLocationStrings = en_pDeliveryLocationStrings;
  pSkillAtZeroWarning = en_pSkillAtZeroWarning;
  pIMPBeginScreenStrings = en_pIMPBeginScreenStrings;
  pIMPFinishButtonText = en_pIMPFinishButtonText;
  pIMPFinishStrings = en_pIMPFinishStrings;
  pIMPVoicesStrings = en_pIMPVoicesStrings;
  pDepartedMercPortraitStrings = en_pDepartedMercPortraitStrings;
  pPersTitleText = en_pPersTitleText;
  pPausedGameText = en_pPausedGameText;
  zOptionsToggleText = en_zOptionsToggleText;
  zOptionsScreenHelpText = en_zOptionsScreenHelpText;
  pDoctorWarningString = en_pDoctorWarningString;
  pMilitiaButtonsHelpText = en_pMilitiaButtonsHelpText;
  pMapScreenJustStartedHelpText = en_pMapScreenJustStartedHelpText;
  pLandMarkInSectorString = en_pLandMarkInSectorString;
  gzMercSkillText = en_gzMercSkillText;
  gzNonPersistantPBIText = en_gzNonPersistantPBIText;
  gzMiscString = en_gzMiscString;
  wMapScreenSortButtonHelpText = en_wMapScreenSortButtonHelpText;
  pNewNoiseStr = en_pNewNoiseStr;
  gzLateLocalizedString = en_gzLateLocalizedString;
  pAntiHackerString = en_pAntiHackerString;
  pMessageStrings = en_pMessageStrings;
  ItemPickupHelpPopup = en_ItemPickupHelpPopup;
  TacticalStr = en_TacticalStr;
  LargeTacticalStr = en_LargeTacticalStr;
  zDialogActions = en_zDialogActions;
  zDealerStrings = en_zDealerStrings;
  zTalkMenuStrings = en_zTalkMenuStrings;
  gzMoneyAmounts = en_gzMoneyAmounts;
  gMoneyStatsDesc = en_gMoneyStatsDesc;
  gWeaponStatsDesc = en_gWeaponStatsDesc;
  sKeyDescriptionStrings = en_sKeyDescriptionStrings;
  zHealthStr = en_zHealthStr;
  zVehicleName = en_zVehicleName;
  pExitingSectorHelpText = en_pExitingSectorHelpText;
  InsContractText = en_InsContractText;
  InsInfoText = en_InsInfoText;
  MercAccountText = en_MercAccountText;
  MercInfo = en_MercInfo;
  MercNoAccountText = en_MercNoAccountText;
  MercHomePageText = en_MercHomePageText;
  sFuneralString = en_sFuneralString;
  sFloristText = en_sFloristText;
  sOrderFormText = en_sOrderFormText;
  sFloristGalleryText = en_sFloristGalleryText;
  sFloristCards = en_sFloristCards;
  BobbyROrderFormText = en_BobbyROrderFormText;
  BobbyRText = en_BobbyRText;
  BobbyRaysFrontText = en_BobbyRaysFrontText;
  AimSortText = en_AimSortText;
  AimPolicyText = en_AimPolicyText;
  AimMemberText = en_AimMemberText;
  CharacterInfo = en_CharacterInfo;
  VideoConfercingText = en_VideoConfercingText;
  AimPopUpText = en_AimPopUpText;
  AimLinkText = en_AimLinkText;
  AimHistoryText = en_AimHistoryText;
  AimFiText = en_AimFiText;
  AimAlumniText = en_AimAlumniText;
  AimScreenText = en_AimScreenText;
  AimBottomMenuText = en_AimBottomMenuText;
  zMarksMapScreenText = en_zMarksMapScreenText;
  gpStrategicString = en_gpStrategicString;
  gpGameClockString = en_gpGameClockString;
  SKI_Text = en_SKI_Text;
  SkiAtmText = en_SkiAtmText;
  gzSkiAtmText = en_gzSkiAtmText;
  SkiMessageBoxText = en_SkiMessageBoxText;
  zSaveLoadText = en_zSaveLoadText;
  zOptionsText = en_zOptionsText;
  gzGIOScreenText = en_gzGIOScreenText;
  gzHelpScreenText = en_gzHelpScreenText;
  gzLaptopHelpText = en_gzLaptopHelpText;
  gzMoneyWithdrawMessageText = en_gzMoneyWithdrawMessageText;
  gzCopyrightText = en_gzCopyrightText;
  BrokenLinkText = en_BrokenLinkText;
  gzBobbyRShipmentText = en_gzBobbyRShipmentText;
  zGioDifConfirmText = en_zGioDifConfirmText;
  gzCreditNames = en_gzCreditNames;
  gzCreditNameTitle = en_gzCreditNameTitle;
  gzCreditNameFunny = en_gzCreditNameFunny;
  pUpdatePanelButtons = en_pUpdatePanelButtons;
  pBullseyeStrings = en_pBullseyeStrings;
  pContractButtonString = en_pContractButtonString;

  zNewTacticalMessages = en_zNewTacticalMessages;
  gzIronManModeWarningText = en_gzIronManModeWarningText;
}
