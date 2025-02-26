// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/Text.h"

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
wchar_t* nl_AmmoCaliber[] = {
    L"0",         L".38 kal", L"9mm",    L".45 kal", L".357 kal",
    L"12 gauge",  L"CAWS",    L"5.45mm", L"5.56mm",  L"7.62mm NAVO",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monster", L"Raket",
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
wchar_t* nl_BobbyRayAmmoCaliber[] = {
    L"0",         L".38 kal", L"9mm",    L".45 kal", L".357 kal",
    L"12 gauge",  L"CAWS",    L"5.45mm", L"5.56mm",  L"7.62mm N.",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monster", L"Raket",
    L"",  // dart
};

wchar_t* nl_WeaponType[] = {
    L"Other",        L"Pistol",        L"Machine pistol",    L"Machine Gun", L"Rifle",
    L"Sniper Rifle", L"Attack weapon", L"Light machine gun", L"Shotgun",
};

wchar_t* nl_TeamTurnString[] = {
    L"Beurt speler", L"Beurt opponent", L"Beurt beest", L"Beurt militie", L"Beurt burgers",
    // planning turn
};

wchar_t* nl_Message[] = {
    L"",

    // In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

    L"%s geraakt in hoofd en verliest een intelligentiepunt!",
    L"%s geraakt in de schouder en verliest een handigheidspunt!",
    L"%s geraakt in de borst en verliest een krachtspunt!",
    L"%s geraakt in het benen en verliest een beweeglijkspunt!",
    L"%s geraakt in het hoofd en verliest %d wijsheidspunten!",
    L"%s geraakt in de schouder en verliest %d handigheidspunten!",
    L"%s geraakt in de borst en verliest %d krachtspunten!",
    L"%s geraakt in de benen en verliest %d beweeglijkheidspunten!",
    L"Storing!",

    // The first %s is a merc's name, the second is a string from pNoiseVolStr,
    // the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

    L"",  // OBSOLETE
    L"Je versterkingen zijn gearriveerd!",

    // In the following four lines, all %s's are merc names

    L"%s herlaad.",
    L"%s heeft niet genoeg actiepunten!",
    L"%s verricht eerste hulp. (Druk een toets om te stoppen.)",
    L"%s en %s verrichten eerste hulp. (Druk een toets om te stoppen.)",
    // the following 17 strings are used to create lists of gun advantages and disadvantages
    // (separated by commas)
    L"reliable",
    L"unreliable",
    L"easy to repair",
    L"hard to repair",
    L"much damage",
    L"low damage",
    L"quick fire",
    L"slow fire",
    L"long range",
    L"short range",
    L"light",
    L"heavy",
    L"small",
    L"quick salvo",
    L"no salvo",
    L"large magazine",
    L"small magazine",

    // In the following two lines, all %s's are merc names

    L"%s's camouflage is verdwenen.",
    L"%s's camouflage is afgespoelt.",

    // The first %s is a merc name and the second %s is an item name

    L"Tweede wapen is leeg!",
    L"%s heeft %s gestolen.",

    // The %s is a merc name

    L"%s's wapen vuurt geen salvo.",

    L"Je hebt er al één van die vastgemaakt.",
    L"Samen voegen?",

    // Both %s's are item names

    L"Je verbindt %s niet met %s.",
    L"Geen",
    L"Eject ammo",
    L"Toebehoren",

    // You cannot use "item(s)" and your "other item" at the same time.
    // Ex:  You cannot use sun goggles and you gas mask at the same time.
    L"%s en %s zijn niet tegelijk te gebruiken.",

    L"Het item dat je aanwijst, kan vastgemaakt worden aan een bepaald item door het in een van de "
    L"vier uitbreidingssloten te plaatsen.",
    L"Het item dat je aanwijst, kan vastgemaakt worden aan een bepaald item door het in een van de "
    L"vier uitbreidingssloten te plaatsen. (Echter, het item is niet compatibel.)",
    L"Er zijn nog vijanden in de sector!",
    L"Je moet %s %s nog geven",
    L"kogel doorboorde %s in zijn hoofd!",
    L"Gevecht verlaten?",
    L"Dit samenvoegen is permanent. Verdergaan?",
    L"%s heeft meer energie!",
    L"%s is uitgegleden!",
    L"%s heeft %s niet gepakt!",
    L"%s repareert de %s",
    L"Stoppen voor ",
    L"Overgeven?",
    L"Deze persoon weigert je hulp.",
    L"Ik denk het NIET!",
    L"Chopper van Skyrider gebruiken? Eerst huurlingen TOEWIJZEN aan VOERTUIG/HELIKOPTER.",
    L"%s had tijd maar EEN geweer te herladen",
    L"Beurt bloodcats",
};

// the names of the towns in the game

wchar_t* nl_pTownNames[] = {
    L"",         L"Omerta", L"Drassen", L"Alma",   L"Grumm",  L"Tixa",     L"Cambria",
    L"San Mona", L"Estoni", L"Orta",    L"Balime", L"Meduna", L"Chitzena",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per
// second, etc. min is an abbreviation for minutes

wchar_t* nl_sTimeStrings[] = {
    L"Pause", L"Normal", L"5 min", L"30 min", L"60 min", L"6 uur",
};

// Assignment Strings: what assignment does the merc  have right now? For example, are they on a
// squad, training, administering medical aid (doctor) or training a town. All are abbreviated. 8
// letters is the longest it can be.

wchar_t* nl_pAssignmentStrings[] = {
    L"Team 1",   L"Team 2",  L"Team 3",  L"Team 4",  L"Team 5",  L"Team 6",  L"Team 7",
    L"Team 8",   L"Team 9",  L"Team 10", L"Team 11", L"Team 12", L"Team 13", L"Team 14",
    L"Team 15",  L"Team 16", L"Team 17", L"Team 18", L"Team 19", L"Team 20",
    L"Dienst",    // on active duty
    L"Dokter",    // administering medical aid
    L"Patiënt",   // getting medical aid
    L"Voertuig",  // in a vehicle
    L"Onderweg",  // in transit - abbreviated form
    L"Repareer",  // repairing
    L"Oefenen",   // training themselves
    L"Militie",   // training a town to revolt
    L"Trainer",   // training a teammate
    L"Student",   // being trained by someone else
    L"Dood",      // dead
    L"Uitgesc.",  // abbreviation for incapacitated
    L"POW",       // Prisoner of war - captured
    L"Kliniek",   // patient in a hospital
    L"Leeg",      // Vehicle is empty
};

wchar_t* nl_pMilitiaString[] = {
    L"Militie",     // the title of the militia box
    L"Unassigned",  // the number of unassigned militia troops
    L"Milities kunnen niet herplaatst worden als er nog vijanden in de buurt zijn!",
};

wchar_t* nl_pMilitiaButtonString[] = {
    L"Auto",  // auto place the militia troops for the player
    L"OK",    // done placing militia troops
};

wchar_t* nl_pConditionStrings[] = {
    L"Excellent",    // the state of a soldier .. excellent health
    L"Good",         // good health
    L"Fair",         // fair health
    L"Wounded",      // wounded health
    L"Tired",        // tired
    L"Bleeding",     // bleeding to death
    L"Knocked out",  // knocked out
    L"Dying",        // near death
    L"Dead",         // dead
};

wchar_t* nl_pEpcMenuStrings[] = {
    L"On duty",  // set merc on active duty
    L"Patient",  // set as a patient to receive medical aid
    L"Vehicle",  // tell merc to enter vehicle
    L"Alone",    // let the escorted character go off on their own
    L"Close",    // close this menu
};

// look at pAssignmentString above for comments

wchar_t* nl_pPersonnelAssignmentStrings[] = {
    L"Team 1",   L"Team 2",  L"Team 3",  L"Team 4",  L"Team 5",  L"Team 6",  L"Team 7",
    L"Team 8",   L"Team 9",  L"Team 10", L"Team 11", L"Team 12", L"Team 13", L"Team 14",
    L"Team 15",  L"Team 16", L"Team 17", L"Team 18", L"Team 19", L"Team 20",
    L"Dienst",    // on active duty
    L"Dokter",    // administering medical aid
    L"Patiënt",   // getting medical aid
    L"Voertuig",  // in a vehicle
    L"Onderweg",  // in transit - abbreviated form
    L"Repareer",  // repairing
    L"Oefenen",   // training themselves
    L"Militie",   // training a town to revolt
    L"Trainer",   // training a teammate
    L"Student",   // being trained by someone else
    L"Dood",      // dead
    L"Uitgesc.",  // abbreviation for incapacitated
    L"POW",       // Prisoner of war - captured
    L"Kliniek",   // patient in a hospital
    L"Leeg",      // Vehicle is empty
};

// refer to above for comments

wchar_t* nl_pLongAssignmentStrings[] = {
    L"Team 1",   L"Team 2",  L"Team 3",  L"Team 4",  L"Team 5",  L"Team 6",  L"Team 7",
    L"Team 8",   L"Team 9",  L"Team 10", L"Team 11", L"Team 12", L"Team 13", L"Team 14",
    L"Team 15",  L"Team 16", L"Team 17", L"Team 18", L"Team 19", L"Team 20",
    L"Dienst",    // on active duty
    L"Dokter",    // administering medical aid
    L"Patiënt",   // getting medical aid
    L"Voertuig",  // in a vehicle
    L"Onderweg",  // in transit - abbreviated form
    L"Repareer",  // repairing
    L"Oefenen",   // training themselves
    L"Militie",   // training a town to revolt
    L"Trainer",   // training a teammate
    L"Student",   // being trained by someone else
    L"Dood",      // dead
    L"Uitgesc.",  // abbreviation for incapacitated
    L"POW",       // Prisoner of war - captured
    L"Kliniek",   // patient in a hospital
    L"Leeg",      // Vehicle is empty
};

// the contract options

wchar_t* nl_pContractStrings[] = {
    L"Contract Opties:",
    L"",                 // a blank line, required
    L"Voor een dag",     // offer merc a one day contract extension
    L"Voor een week",    // 1 week
    L"Voor twee weken",  // 2 week
    L"Ontslag",          // end merc's contract
    L"Stop",             // stop showing this menu
};

wchar_t* nl_pPOWStrings[] = {
    L"POW",  // an acronym for Prisoner of War
    L"??",
};

wchar_t* nl_pLongAttributeStrings[] = {
    L"KRACHT",  L"HANDIGHEID", L"BEWEEGLIJKHEID", L"WIJSHEID",    L"TREFZEKERHEID",
    L"MEDISCH", L"TECHNISCH",  L"LEIDERSCHAP",    L"EXPLOSIEVEN", L"NIVEAU",
};

wchar_t* nl_pInvPanelTitleStrings[] = {
    L"Wapen",  // the armor rating of the merc
    L"Gew.",   // the weight the merc is carrying
    L"Camo",   // the merc's camouflage rating
};

wchar_t* nl_pShortAttributeStrings[] = {
    L"Bew",  // the abbreviated version of : agility
    L"Han",  // dexterity
    L"Kra",  // strength
    L"Ldr",  // leadership
    L"Wij",  // wisdom
    L"Niv",  // experience level
    L"Tre",  // marksmanship skill
    L"Exp",  // explosive skill
    L"Tec",  // mechanical skill
    L"Med",  // medical skill
};

wchar_t* nl_pUpperLeftMapScreenStrings[] = {
    L"Opdracht",  // the mercs current assignment
    L"Contract",  // the contract info about the merc
    L"Gezond",    // the health level of the current merc
    L"Moraal",    // the morale of the current merc
    L"Cond.",     // the condition of the current vehicle
    L"Tank",      // the fuel level of the current vehicle
};

wchar_t* nl_pTrainingStrings[] = {
    L"Oefen",    // tell merc to train self
    L"Militie",  // tell merc to train town
    L"Trainer",  // tell merc to act as trainer
    L"Student",  // tell merc to be train by other
};

wchar_t* nl_pGuardMenuStrings[] = {
    L"Schietniveau:",        // the allowable rate of fire for a merc who is guarding
    L" Agressief vuren",     // the merc can be aggressive in their choice of fire rates
    L" Spaar Munitie",       // conserve ammo
    L" Afzien van Vuren",    // fire only when the merc needs to
    L"Andere Opties:",       // other options available to merc
    L" Kan Vluchten",        // merc can retreat
    L" Kan Dekking Zoeken",  // merc is allowed to seek cover
    L" Kan Team Helpen",     // merc can assist teammates
    L"OK",                   // done with this menu
    L"Stop",                 // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by
// the player

wchar_t* nl_pOtherGuardMenuStrings[] = {
    L"Schietniveau:",
    L" *Agressief vuren*",
    L" *Spaar Munitie*",
    L" *Afzien van Vuren*",
    L"Andere Opties:",
    L" *Kan Vluchten*",
    L" *Kan Dekking Zoeken*",
    L" *Kan Team Helpen*",
    L"OK",
    L"Stop",
};

wchar_t* nl_pAssignMenuStrings[] = {
    L"On duty",  // merc is on active duty
    L"Doctor",   // the merc is acting as a doctor
    L"Patient",  // the merc is receiving medical attention
    L"Vehicle",  // the merc is in a vehicle
    L"Repair",   // the merc is repairing items
    L"Train",    // the merc is training
    L"Stop",     // cancel this menu
};

wchar_t* nl_pRemoveMercStrings[] = {
    L"Verw.Huurl.",  // remove dead merc from current team
    L"Stop",
};

wchar_t* nl_pAttributeMenuStrings[] = {
    L"Kracht",  L"Behendigheid", L"Lenigheid",   L"Gezondheid", L"Scherpschutterskunst",
    L"Medisch", L"Mechanisch",   L"Leiderschap", L"Explosief",  L"Annuleren",
};

/*
wchar_t* nl_pAttributeMenuStrings[] =
{
        L"Kracht",
        L"Behendig",
        L"Beweging",
        L"Schietkunst",
        L"Trefzekerheid",
        L"Medisch",
        L"Technisch",
        L"Leaderschip",
        L"Precisie",
        L"Stop",
};
*/

wchar_t* nl_pTrainingMenuStrings[] = {
    L"Oefenen",  // train yourself
    L"Militie",  // train the town
    L"Trainer",  // train your teammates
    L"Student",  // be trained by an instructor
    L"Stop",     // cancel this menu
};

wchar_t* nl_pSquadMenuStrings[] = {
    L"Team  1", L"Team  2", L"Team  3", L"Team  4", L"Team  5", L"Team  6", L"Team  7",
    L"Team  8", L"Team  9", L"Team 10", L"Team 11", L"Team 12", L"Team 13", L"Team 14",
    L"Team 15", L"Team 16", L"Team 17", L"Team 18", L"Team 19", L"Team 20", L"Stop",
};

wchar_t* nl_pPersonnelTitle[] = {
    L"Dossiers",  // the title for the personnel screen/program application
};

wchar_t* nl_pPersonnelScreenStrings[] = {
    L"Gezondheid: ",  // health of merc
    L"Beweeglijkheid: ",
    L"Handigheid: ",
    L"Kracht: ",
    L"Leiderschap; ",
    L"Wijsheid: ",
    L"Erv. Niv.: ",  // experience level
    L"Trefzekerheid: ",
    L"Techniek: ",
    L"Explosieven: ",
    L"Medisch: ",
    L"Med. Kosten: ",          // amount of medical deposit put down on the merc
    L"Rest Contract: ",        // cost of current contract
    L"Doden: ",                // number of kills by merc
    L"Hulp: ",                 // number of assists on kills by merc
    L"Dag. Kosten:",           // daily cost of merc
    L"Huidige Tot. Kosten:",   // total cost of merc
    L"Huidige Tot. Service:",  // total service rendered by merc
    L"Salaris Tegoed:",        // amount left on MERC merc to be paid
    L"Trefzekerheid:",         // percentage of shots that hit target
    L"Gevechten:",             // number of battles fought
    L"Keren Gewond:",          // number of times merc has been wounded
    L"Vaardigheden:",
    L"Vaardigheden:",
};

// These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
wchar_t* nl_gzMercSkillText[] = {
    L"No Skill",     L"Forceer slot", L"Man-tot-man",        L"Elektronica", L"Nachtops", L"Werpen",
    L"Lesgeven",     L"Zware Wapens", L"Auto Wapens",        L"Sluipen",     L"Handig",   L"Dief",
    L"Vechtkunsten", L"Mesworp",      L"Raak op dak! Bonus", L"Camouflaged", L"(Expert)",
};

// This is pop up help text for the options that are available to the merc

wchar_t* nl_pTacticalPopupButtonStrings[] = {
    L"|Staan/Lopen",
    L"Hurken/Gehurkt lopen (|C)",
    L"Staan/|Rennen",
    L"Liggen/Kruipen (|P)",
    L"Kijk (|L)",
    L"Actie",
    L"Praat",
    L"Bekijk (|C|t|r|l)",

    // Pop up door menu
    L"Handm. openen",
    L"Zoek boobytraps",
    L"Forceer",
    L"Met geweld",
    L"Verwijder boobytrap",
    L"Sluiten",
    L"Maak open",
    L"Gebruik explosief",
    L"Gebruik breekijzer",
    L"Stoppen (|E|s|c)",
    L"Stop",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

wchar_t* nl_pDoorTrapStrings[] = {
    L"geen val", L"een explosie", L"een elektrische val", L"alarm", L"stil alarm",
};

// Contract Extension. These are used for the contract extension with AIM mercenaries.

wchar_t* nl_pContractExtendStrings[] = {
    L"dag",
    L"week",
    L"twee weken",
};

// On the map screen, there are four columns. This text is popup help text that identifies the
// individual columns.

wchar_t* nl_pMapScreenMouseRegionHelpText[] = {
    L"Selecteer Karakter", L"Contracteer huurling", L"Plan Route",
    L"Huurling |Contract", L"Verwijder Huurling",   L"Slaap",
};

// volumes of noises

wchar_t* nl_pNoiseVolStr[] = {
    L"VAAG",
    L"ZEKER",
    L"HARD",
    L"ERG HARD",
};

// types of noises

wchar_t* nl_pNoiseTypeStr[] =  // OBSOLETE
    {
        L"ONBEKEND", L"geluid van BEWEGING",
        L"GEKRAAK",  L"PLONZEN",
        L"INSLAG",   L"SCHOT",
        L"EXPLOSIE", L"GEGIL",
        L"INSLAG",   L"INSLAG",
        L"BARSTEN",  L"DREUN",
};

// Directions that are used to report noises

wchar_t* nl_pDirectionStr[] = {
    L"het NOORDOOSTEN", L"het OOSTEN", L"het ZUIDOOSTEN",  L"het ZUIDEN",
    L"het ZUIDWESTEN",  L"het WESTEN", L"het NOORDWESTEN", L"het NOORDEN",
};

// These are the different terrain types.

wchar_t* nl_pLandTypeStrings[] = {
    L"Stad", L"Weg", L"Vlaktes", L"Woestijn", L"Bossen", L"Woud", L"Moeras", L"Water", L"Heuvels",
    L"Onbegaanbaar",
    L"Rivier",  // river from north to south
    L"Rivier",  // river from east to west
    L"Buitenland",
    // NONE of the following are used for directional travel, just for the sector description.
    L"Tropisch", L"Landbouwgrond", L"Vlaktes, weg", L"Bossen, weg", L"Boerderij, weg",
    L"Tropisch, weg", L"Woud, weg", L"Kustlijn", L"Bergen, weg", L"Kust-, weg", L"Woestijn, weg",
    L"Moeras, weg", L"Bossen, SAM-stelling", L"Woestijn, SAM-stelling", L"Tropisch, SAM-stelling",
    L"Meduna, SAM-stelling",

    // These are descriptions for special sectors
    L"Cambria Ziekenhuis", L"Drassen Vliegveld", L"Meduna Vliegveld", L"SAM-stelling",
    L"Schuilplaats Rebellen",  // The rebel base underground in sector A10
    L"Tixa Kerker",            // The basement of the Tixa Prison (J9)
    L"Hol Beest",              // Any mine sector with creatures in it
    L"Orta Basis",             // The basement of Orta (K4)
    L"Tunnel",                 // The tunnel access from the maze garden in Meduna
                               // leading to the secret shelter underneath the palace
    L"Schuilplaats",           // The shelter underneath the queen's palace
    L"",                       // Unused
};

wchar_t* nl_gpStrategicString[] = {
    L"",                                                                        // Unused
    L"%s zijn ontdekt in sector %c%d en een ander team arriveert binnenkort.",  // STR_DETECTED_SINGULAR
    L"%s zijn ontdekt in sector %c%d en andere teams arriveren binnenkort.",  // STR_DETECTED_PLURAL
    L"Wil je een gezamenlijke aankomst coördineren?",                         // STR_COORDINATE

    // Dialog strings for enemies.

    L"De vijand geeft je de kans om je over te geven.",  // STR_ENEMY_SURRENDER_OFFER
    L"De vijand heeft je overgebleven bewusteloze huurlingen gevangen.",  // STR_ENEMY_CAPTURED

    // The text that goes on the autoresolve buttons

    L"Vluchten",  // The retreat button			//STR_AR_RETREAT_BUTTON
    L"OK",        // The done button				//STR_AR_DONE_BUTTON

    // The headers are for the autoresolve type (MUST BE UPPERCASE)

    L"VERDEDIGEN",  // STR_AR_DEFEND_HEADER
    L"AANVALLEN",   // STR_AR_ATTACK_HEADER
    L"ONTDEKKEN",   // STR_AR_ENCOUNTER_HEADER
    L"Sector",      // The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

    // The battle ending conditions

    L"VICTORIE!",     // STR_AR_OVER_VICTORY
    L"NEDERLAAG!",    // STR_AR_OVER_DEFEAT
    L"OVERGEGEVEN!",  // STR_AR_OVER_SURRENDERED
    L"GEVANGEN!",     // STR_AR_OVER_CAPTURED
    L"GEVLUCHT!",     // STR_AR_OVER_RETREATED

    // These are the labels for the different types of enemies we fight in autoresolve.

    L"Militie",  // STR_AR_MILITIA_NAME,
    L"Elite",    // STR_AR_ELITE_NAME,
    L"Troep",    // STR_AR_TROOP_NAME,
    L"Admin",    // STR_AR_ADMINISTRATOR_NAME,
    L"Wezen",    // STR_AR_CREATURE_NAME,

    // Label for the length of time the battle took

    L"Tijd verstreken",  // STR_AR_TIME_ELAPSED,

    // Labels for status of merc if retreating.  (UPPERCASE)

    L"GEVLUCHT",  // STR_AR_MERC_RETREATED,
    L"VLUCHTEN",  // STR_AR_MERC_RETREATING,
    L"VLUCHT",    // STR_AR_MERC_RETREAT,

    // PRE BATTLE INTERFACE STRINGS
    // Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
    // a system that automatically resolves the combat for the player without having to do anything.
    // These strings must be short (two lines -- 6-8 chars per line)

    L"Autom. Opl.",     //!!! 1			//STR_PB_AUTORESOLVE_BTN,
    L"Naar Sector",     // STR_PB_GOTOSECTOR_BTN,
    L"Terug- trekken",  //!!! 2		//STR_PB_RETREATMERCS_BTN,

    // The different headers(titles) for the prebattle interface.
    L"VIJAND ONTDEKT",               // STR_PB_ENEMYENCOUNTER_HEADER,
    L"INVASIE VIJAND",               // STR_PB_ENEMYINVASION_HEADER, // 30
    L"HINDERLAAG VIJAND",            // STR_PB_ENEMYAMBUSH_HEADER
    L"BINNENGAAN VIJANDIGE SECTOR",  // STR_PB_ENTERINGENEMYSECTOR_HEADER
    L"AANVAL BEEST",                 // STR_PB_CREATUREATTACK_HEADER
    L"BLOODCAT VAL",                 // STR_PB_BLOODCATAMBUSH_HEADER
    L"BINNENGAAN HOL BLOODCAT",      // STR_PB_ENTERINGBLOODCATLAIR_HEADER

    // Various single words for direct translation.  The Civilians represent the civilian
    // militia occupying the sector being attacked.  Limited to 9-10 chars

    L"Locatie",
    L"Vijanden",
    L"Huurlingen",
    L"Milities",
    L"Beesten",
    L"Bloodcats",
    L"Sector",
    L"Geen",  // If there are no uninvolved mercs in this fight.
    L"NVT",   // Acronym of Not Applicable
    L"d",     // One letter abbreviation of day
    L"u",     // One letter abbreviation of hour

    // TACTICAL PLACEMENT USER INTERFACE STRINGS
    // The four buttons

    L"Weggaan",
    L"Verspreid",
    L"Groeperen",
    L"OK",

    // The help text for the four buttons.  Use \n to denote new line (just like enter).

    L"Maakt posities van huurlingen vrij en\nmaakt handmatig herinvoer mogelijk. (|C)",
    L"Ver|spreidt willekeurig je huurlingen\nelke keer als je de toets indrukt.",
    L"Hiermee is het mogelijk de huurlingen te |groeperen.",
    L"Druk op deze toets als je klaar bent met\nhet positioneren van je huurlingen. (|E|n|t|e|r)",
    L"Je moet al je huurlingen positioneren\nvoor je het gevecht kunt starten.",

    // Various strings (translate word for word)

    L"Sector",
    L"Kies posities binnenkomst",

    // Strings used for various popup message boxes.  Can be as long as desired.

    L"Ziet er hier niet goed uit. Het is onbegaanbaar. Probeer een andere locatie.",
    L"Plaats je huurlingen in de gemarkeerde sectie van de kaart.",

    // This message is for mercs arriving in sectors.  Ex:  Red has arrived in sector A9.
    // Don't uppercase first character, or add spaces on either end.

    L"is gearriveerd in sector",

    // These entries are for button popup help text for the prebattle interface.  All popup help
    // text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
    L"Lost het gevecht |Automatisch\nop zonder de kaart te laden.",
    L"Automatisch oplossen niet\nmogelijk als de speler aanvalt.",
    L"Ga sector binnen om tegen\nde vijand te strijden. (|E)",
    L"T|rek groep terug en ga naar de vorige sector.",            // singular version
    L"T|rek alle groepen terug en\nga naar hun vorige sectors.",  // multiple groups with same
                                                                  // previous sector

    // various popup messages for battle conditions.

    //%c%d is the sector -- ex:  A9
    L"Vijanden vallen je militie aan in sector %c%d.",
    //%c%d is the sector -- ex:  A9
    L"Beesten vallen je militie aan in sector %c%d.",
    // 1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
    // Note:  the minimum number of civilians eaten will be two.
    L"Beesten vallen aan en doden %d burgers in sector %s.",
    //%s is the sector location -- ex:  A9: Omerta
    L"Vijand valt je huurlingen aan in sector %s. Geen enkele huurling kan vechten!",
    //%s is the sector location -- ex:  A9: Omerta
    L"Beesten vallen je huurlingen aan in sector %s. Geen enkele huurling kan vechten!",

};

wchar_t* nl_gpGameClockString[] = {
    // This is the day represented in the game clock.  Must be very short, 4 characters max.
    L"Dag",
};

// When the merc finds a key, they can get a description of it which
// tells them where and when they found it.
wchar_t* nl_sKeyDescriptionStrings[2] = {
    L"Sector gevonden:",
    L"Dag gevonden:",
};

// The headers used to describe various weapon statistics.

wchar_t* nl_gWeaponStatsDesc[] = {
    L"Gewicht (%s):",
    L"Status:",
    L"Munitie:",  // Number of bullets left in a magazine
    L"Afst:",     // Range
    L"Sch:",      // Damage
    L"AP:",       // abbreviation for Action Points
    L"",
    L"=",
    L"=",
};

// The headers used for the merc's money.

wchar_t* nl_gMoneyStatsDesc[] = {
    L"Bedrag",
    L"Restbedrag:",  // this is the overall balance
    L"Bedrag",
    L"Splitsen:",  // the amount he wants to separate from the overall balance to get two piles of
                   // money

    L"Huidig",
    L"Saldo",
    L"Bedrag naar",
    L"Opnemen",
};

// The health of various creatures, enemies, characters in the game. The numbers following each are
// for comment only, but represent the precentage of points remaining.

wchar_t* nl_zHealthStr[] = {
    L"STERVEND",   //	>= 0
    L"KRITIEK",    //	>= 15
    L"SLECHT",     //	>= 30
    L"GEWOND",     //	>= 45
    L"GEZOND",     //	>= 60
    L"STERK",      // 	>= 75
    L"EXCELLENT",  // 	>= 90
};

wchar_t* nl_gzMoneyAmounts[6] = {
    L"$1000", L"$100", L"$10", L"OK", L"Splitsen", L"Opnemen",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
wchar_t* nl_gzProsLabel = {
    L"Voor:",
};

wchar_t* nl_gzConsLabel = {
    L"Tegen:",
};

// Conversation options a player has when encountering an NPC
wchar_t* nl_zTalkMenuStrings[6] = {
    L"Wat?",     // meaning "Repeat yourself"
    L"Aardig",   // approach in a friendly
    L"Direct",   // approach directly - let's get down to business
    L"Dreigen",  // approach threateningly - talk now, or I'll blow your face off
    L"Geef",
    L"Rekruut",  // recruit
};

// Some NPCs buy, sell or repair items. These different options are available for those NPCs as
// well.
wchar_t* nl_zDealerStrings[4] = {
    L"Koop/Verkoop",  // Buy/Sell
    L"Koop",          // Buy
    L"Verkoop",       // Sell
    L"Repareer",      // Repair
};

wchar_t* nl_zDialogActions[1] = {
    L"OK",
};

// These are vehicles in the game.

wchar_t* nl_pVehicleStrings[] = {
    L"Eldorado",
    L"Hummer",     // a hummer jeep/truck -- military vehicle
    L"Koeltruck",  // Icecream Truck
    L"Jeep",      L"Tank", L"Helikopter",
};

wchar_t* nl_pShortVehicleStrings[] = {
    L"Eldor.",
    L"Hummer",  // the HMVV
    L"Truck",  L"Jeep", L"Tank",
    L"Heli",  // the helicopter
};

wchar_t* nl_zVehicleName[] = {
    L"Eldorado",
    L"Hummer",  // a military jeep. This is a brand name.
    L"Truck",   // Ice cream truck
    L"Jeep",     L"Tank",
    L"Heli",  // an abbreviation for Helicopter
};

// These are messages Used in the Tactical Screen

wchar_t* nl_TacticalStr[] = {
    L"Luchtaanval", L"Automatisch EHBO toepassen?",

    // CAMFIELD NUKE THIS and add quote #66.

    L"%s ziet dat er items missen van de lading.",

    // The %s is a string from pDoorTrapStrings

    L"Het slot heeft %s.", L"Er is geen slot.", L"Gelukt!", L"Mislukt.", L"Gelukt!", L"Mislukt.",
    L"Geen boobytrap op het slot.", L"Gelukt!",
    // The %s is a merc name
    L"%s heeft niet de juiste sleutel.", L"Val weggehaald van slot.", L"Slot heeft geen boobytrap.",
    L"Op slot.", L"DEUR", L"VAL", L"OP SLOT", L"OPEN", L"KAPOT",
    L"Hier zit een schakelaar. Activeren?", L"Boobytrap ontmantelen?", L"Vorige...", L"Volgende...",
    L"Meer...",

    // In the next 2 strings, %s is an item name

    L"%s is op de grond geplaatst.", L"%s is gegeven aan %s.",

    // In the next 2 strings, %s is a name

    L"%s is helemaal betaald.", L"%s heeft tegoed nog %d.",
    L"Kies detonatie frequentie:",          // in this case, frequency refers to a radio signal
    L"Aantal beurten tot ontploffing:",     // how much time, in turns, until the bomb blows
    L"Stel frequentie in van ontsteking:",  // in this case, frequency refers to a radio signal
    L"Boobytrap ontmantelen?", L"Blauwe vlag weghalen?", L"Blauwe vlag hier neerzetten?",
    L"Laatste beurt",

    // In the next string, %s is a name. Stance refers to way they are standing.

    L"Zeker weten dat je %s wil aanvallen?", L"Ah, voertuigen kunnen plaats niet veranderen.",
    L"De robot kan niet van plaats veranderen.",

    // In the next 3 strings, %s is a name

    L"%s kan niet naar die plaats gaan.", L"%s kan hier geen EHBO krijgen.",
    L"%s heeft geen EHBO nodig.", L"Kan daar niet heen.",
    L"Je team is vol. Geen ruimte voor rekruut.",  // there's no room for a recruit on the player's
                                                   // team

    // In the next string, %s is a name

    L"%s is gerekruteerd.",

    // Here %s is a name and %d is a number

    L"%s ontvangt $%d.",

    // In the next string, %s is a name

    L"%s begeleiden?",

    // In the next string, the first %s is a name and the second %s is an amount of money (including
    // $ sign)

    L"%s inhuren voor %s per dag?",

    // This line is used repeatedly to ask player if they wish to participate in a boxing match.

    L"Wil je vechten?",

    // In the next string, the first %s is an item name and the
    // second %s is an amount of money (including $ sign)

    L"%s kopen voor %s?",

    // In the next string, %s is a name

    L"%s wordt begeleid door team %d.",

    // These messages are displayed during play to alert the player to a particular situation

    L"GEBLOKKEERD",                         // weapon is jammed.
    L"Robot heeft %s kal. munitie nodig.",  // Robot is out of ammo
    L"Hier gooien? Kan niet.",              // Merc can't throw to the destination he selected

    // These are different buttons that the player can turn on and off.

    L"Sluipmodus (|Z)",                           // L"Stealth Mode (|Z)",
    L"Landkaart (|M)",                            // L"|Map Screen",
    L"OK (Ein|de)",                               // L"|Done (End Turn)",
    L"Praat",                                     // L"Talk",
    L"Stil",                                      // L"Mute",
    L"Omhoog (|P|g|U|p)",                         // L"Stance Up (|P|g|U|p)",
    L"Cursor Niveau (|T|a|b)",                    // L"Cursor Level (|T|a|b)",
    L"Klim / Spring",                             // L"Climb / Jump",
    L"Omlaag (|P|g|D|n)",                         // L"Stance Down (|P|g|D|n)",
    L"Bekijk (|C|t|r|l)",                         // L"Examine (|C|t|r|l)",
    L"Vorige huurling",                           // L"Previous Merc",
    L"Volgende huurling (|S|p|a|c|e)",            // L"Next Merc (|S|p|a|c|e)",
    L"|Opties",                                   // L"|Options",
    L"Salvo's (|B)",                              // L"|Burst Mode",
    L"Kijk/draai (|L)",                           // L"|Look/Turn",
    L"Gezond: %d/%d\nKracht: %d/%d\nMoraal: %s",  // L"Health: %d/%d\nEnergy: %d/%d\nMorale: %s",
    L"Hé?",                                       // this means "what?"
    L"Door",                                      // an abbrieviation for "Continued"
    L"%s is praat weer.",                         // L"Mute off for %s.",
    L"%s is stil.",                               // L"Mute on for %s.",
    L"Gezond: %d/%d\nBrandst: %d/%d",             // L"Health: %d/%d\nFuel: %d/%d",
    L"Stap uit voertuig",                         // L"Exit Vehicle" ,
    L"Wissel Team ( |S|h|i|f|t |S|p|a|c|e )",     // L"Change Squad ( |S|h|i|f|t |S|p|a|c|e )",
    L"Rijden",                                    // L"Drive",
    L"Nvt",                                       // this is an acronym for "Not Applicable."
    L"Actie ( Man-tot-man )",                     // L"Use ( Hand To Hand )",
    L"Actie ( Firearm )",                         // L"Use ( Firearm )",
    L"Actie ( Mes )",                             // L"Use ( Blade )",
    L"Actie ( Explosieven )",                     // L"Use ( Explosive )",
    L"Actie ( EHBO )",                            // L"Use ( Medkit )",
    L"(Vang)",                                    // L"(Catch)",
    L"(Herlaad)",                                 // L"(Reload)",
    L"(Geef)",                                    // L"(Give)",
    L"%s is afgezet.",                            // L"%s has been set off.",
    L"%s is gearriveerd.",                        // L"%s has arrived.",
    L"%s heeft geen Actie Punten.",               // L"%s ran out of Action Points.",
    L"%s is niet beschikbaar.",                   // L"%s isn't available.",
    L"%s zit onder het verband.",                 // L"%s is all bandaged.",
    L"Verband van %s is op.",                     // L"%s is out of bandages.",
    L"Vijand in de sector!",                      // L"Enemy in sector!",
    L"Geen vijanden in zicht.",                   // L"No enemies in sight.",
    L"Niet genoeg Actie Punten.",                 // L"Not enough Action Points.",
    L"Niemand gebruikt afstandb.",                // L"Nobody's using the remote.",
    L"Magazijn leeg door salvovuur!",             // L"Burst fire emptied the clip!",
    L"SOLDAAT",                                   // L"SOLDIER",
    L"CREPITUS",                                  // L"CREPITUS",
    L"MILITIE",                                   // L"MILITIA",
    L"BURGER",                                    // L"CIVILIAN",
    L"Verlaten Sector",                           // L"Exiting Sector",
    L"OK",
    L"Stoppen",                                          // L"Cancel",
    L"Huurling gesel.",                                  // L"Selected Merc",
    L"Alle huurl. in team",                              // L"All Mercs in Squad",
    L"Naar Sector",                                      // L"Go to Sector",
    L"Naar Landk.",                                      // L"Go to Map",
    L"Vanaf deze kant kun je de sector niet verlaten.",  // L"You can't leave the sector from this
                                                         // side.",
    L"%s is te ver weg.",                                // L"%s is too far away.",
    L"Verwijder Boomtoppen",                             // L"Removing Treetops",
    L"Tonen Boomtoppen",                                 // L"Showing Treetops",
    L"KRAAI",                                            // Crow, as in the large black bird
    L"NEK", L"HOOFD", L"TORSO", L"BENEN",
    L"De Koningin vertellen wat ze wil weten?",      // L"Tell the Queen what she wants to know?",
    L"Vingerafdruk-ID nodig",                        // L"Fingerprint ID aquired",
    L"Vingerafdruk-ID ongeldig. Wapen funct. niet",  // L"Invalid fingerprint ID. Weapon
                                                     // non-functional",
    L"Doelwit nodig",                                // L"Target aquired",
    L"Pad geblokkeerd",                              // L"Path Blocked",
    L"Geld Storten/Opnemen",       // Help text over the $ button on the Single Merc Panel
                                   // ("Deposit/Withdraw Money")
    L"Niemand heeft EHBO nodig.",  // L"No one needs first aid.",
    L"Vast.",                      // Short form of JAMMED, for small inv slots
    L"Kan daar niet heen.",        // used ( now ) for when we click on a cliff
    L"Pad is geblokkeerd. Wil je met deze persoon van plaats wisselen?",
    L"Persoon weigert weg te gaan.",
    // In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
    L"Ben je het eens met %s?",                    // L"Do you agree to pay %s?",
    L"Wil je kostenloze medische hulp?",           // L"Accept free medical treatment?",
    L"Wil je trouwen met Daryl?",                  // L"Agree to marry Daryl?",
    L"Slot Ring Paneel",                           // L"Key Ring Panel",
    L"Dat kan niet met een EPC.",                  // L"You cannot do that with an EPC.",
    L"Krott sparen?",                              // L"Spare Krott?",
    L"Buiten wapenbereik",                         // L"Out of weapon range",
    L"Mijnwerker",                                 // L"Miner",
    L"Voertuig kan alleen tussen sectors reizen",  // L"Vehicle can only travel between sectors",
    L"Nu geen Auto-EHBO mogelijk",                 // L"Can't autobandage right now",
    L"Pad Geblokkeerd voor %s",                    // L"Path Blocked for %s",
    L"Je huurlingen, gevangen door Deidranna's leger, zitten hier opgesloten!",
    L"Slot geraakt",                                    // L"Lock hit",
    L"Slot vernielt",                                   // L"Lock destroyed",
    L"Iemand anders probeert deze deur te gebruiken.",  // L"Somebody else is trying to use this
                                                        // door.",
    L"Gezondheid: %d/%d\nBrandstof: %d/%d",             // L"Health: %d/%d\nFuel: %d/%d",
    L"%s kan %s niet zien.",                            // Cannot see person trying to talk to
};

// Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different
// circumstances in the "exiting sector" interface.
wchar_t* nl_pExitingSectorHelpText[] = {
    // Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box
    // is checked.
    L"Als aangekruist, dan wordt de aanliggende sector meteen geladen.",
    L"Als aangekruist, dan worden de huurlingen automatisch op de\nkaart geplaatst rekening "
    L"houdend met reistijden.",

    // If you attempt to leave a sector when you have multiple squads in a hostile sector.
    L"Deze sector is door de vijand bezet en huurlingen kun je niet achterlaten.\nJe moet deze "
    L"situatie oplossen voor het laden van andere sectors.",

    // Because you only have one squad in the sector, and the "move all" option is checked, the "go
    // to sector" option is locked to on. The helptext explains why it is locked.
    L"Als de overgebleven huurlingen uit deze sector trekken,\nwordt de aanliggende sector "
    L"onmiddellijk geladen.",
    L"Als de overgebleven huurlingen uit deze sector trekken,\nwordt je automatisch in het "
    L"landkaartscherm geplaatst,\nrekening houdend met de reistijd van je huurlingen.",

    // If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being
    // escorted.  The "single" button is disabled.
    L"%s moet geëscorteerd worden door jouw huurlingen\nen kan de sector niet alleen verlaten.",

    // If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc
    // will be prohibited from leaving alone. There are several strings depending on the gender of
    // the merc and how many EPCs are in the squad. DO NOT USE THE NEWLINE HERE AS IT IS USED FOR
    // BOTH HELPTEXT AND SCREEN MESSAGES!
    L"%s kan de sector niet alleen verlaten omdat hij %s escorteert.",  // male singular
    L"%s kan de sector niet alleen verlaten omdat zij %s escorteert.",  // female singular
    L"%s kan de sector niet alleen verlaten omdat hij meerdere karakters escorteert.",  // male
                                                                                        // plural
    L"%s kan de sector niet alleen verlaten omdat zij meerdere karakters escorteert.",  // female
                                                                                        // plural

    // If one or more of your mercs in the selected squad aren't in range of the traversal area,
    // then the  "move all" option is disabled, and this helptext explains why.
    L"Al je huurlingen moeten in de buurt zijn om het team te laten reizen.",

    L"",  // UNUSED

    // Standard helptext for single movement.  Explains what will happen (splitting the squad)
    L"Als aangekruist, dan zal %s alleen verder reizen\nen automatisch bij een uniek team gevoegd "
    L"worden.",

    // Standard helptext for all movement.  Explains what will happen (moving the squad)
    L"Als aangekruist, dan zal je geselecteerde\nteam verder reizen, de sector verlatend.",

    // This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC
    // selected and you attempt to tactically traverse the EPC while the escorting mercs aren't near
    // enough (or dead, dying, or unconscious), this message will appear and the "exiting sector"
    // interface will not appear.  This is just like the situation where This string is special, as
    // it is not used as helptext.  Do not use the special newline character (\n) for this string.
    L"%s wordt geëscorteerd door jouw huurlingen en kan de sector niet alleen verlaten. Je "
    L"huurlingen moeten eerst in de buurt zijn.",
};

wchar_t* nl_pRepairStrings[] = {
    L"Items",         // tell merc to repair items in inventory
    L"SAM-Stelling",  // tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
    L"Stop",          // cancel this menu
    L"Robot",         // repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

wchar_t* nl_sPreStatBuildString[] = {
    L"verliest",      // the merc has lost a statistic
    L"krijgt",        // the merc has gained a statistic
    L"punt voor",     // singular
    L"punten voor",   // plural
    L"niveau voor",   // singular
    L"niveaus voor",  // plural
};

wchar_t* nl_sStatGainStrings[] = {
    L"gezondheid.",     L"beweeglijkheid.", L"handigheid.",       L"wijsheid.",
    L"medisch kunnen.", L"explosieven.",    L"technisch kunnen.", L"trefzekerheid.",
    L"ervaring.",       L"kracht.",         L"leiderschap.",
};

wchar_t* nl_pHelicopterEtaStrings[] = {
    L"Totale Afstand: ",  // total distance for helicopter to travel
    L" Veilig: ",         // distance to travel to destination
    L" Onveilig:",        // distance to return from destination to airport
    L"Totale Kosten: ",   // total cost of trip by helicopter
    L"Aank: ",            // ETA is an acronym for "estimated time of arrival"
    L"Helikopter heeft weinig brandstof en moet landen in vijandelijk gebied!",  // warning that the
                                                                                 // sector the
                                                                                 // helicopter is
                                                                                 // going to use for
                                                                                 // refueling is
                                                                                 // under enemy
                                                                                 // control ->
    L"Passagiers: ",
    L"Selecteer Skyrider of Aanvoer Drop-plaats?",  // L"Select Skyrider or the Arrivals Drop-off?",
    L"Skyrider",
    L"Aanvoer",  // L"Arrivals",
};

wchar_t* nl_sMapLevelString[] = {
    L"Subniv.:",  // what level below the ground is the player viewing in mapscreen ("Sublevel:")
};

wchar_t* nl_gsLoyalString[] = {
    L"Loyaal",  // the loyalty rating of a town ie : Loyal 53%
};

// error message for when player is trying to give a merc a travel order while he's underground.

wchar_t* nl_gsUndergroundString[] = {
    L"kan geen reisorders ondergronds ontvangen.",  // L"can't get travel orders underground.",
};

wchar_t* nl_gsTimeStrings[] = {
    L"u",  // hours abbreviation
    L"m",  // minutes abbreviation
    L"s",  // seconds abbreviation
    L"d",  // days abbreviation
};

// text for the various facilities in the sector

wchar_t* nl_sFacilitiesStrings[] = {
    L"Geen",          L"Ziekenhuis", L"Industrie", L"Gevangenis", L"Krijgsmacht", L"Vliegveld",
    L"Schietterrein",  // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

wchar_t* nl_pMapPopUpInventoryText[] = {
    L"Inventaris",
    L"OK",
};

// town strings

wchar_t* nl_pwTownInfoStrings[] = {
    L"Grootte",        // 0 // size of the town in sectors
    L"",               // blank line, required
    L"Gezag",          // how much of town is controlled
    L"Geen",           // none of this town
    L"Verboden Mijn",  // mine associated with this town
    L"Loyaliteit",     // 5 // the loyalty level of this town
    L"Getraind",       // the forces in the town trained by the player
    L"",
    L"Voorzieningen",     // main facilities in this town
    L"Niveau",            // the training level of civilians in this town
    L"Training Burgers",  // 10 // state of civilian training in town
    L"Militie",           // the state of the trained civilians in the town
};

// Mine strings

wchar_t* nl_pwMineStrings[] = {
    L"Mijn",  // 0
    L"Zilver",
    L"Goud",
    L"Dagelijkse prod.",
    L"Mogelijke prod.",
    L"Verlaten",  // 5
    L"Gesloten",
    L"Raakt Op",
    L"Produceert",
    L"Status",
    L"Prod. Tempo",
    L"Ertstype",  // 10
    L"Gezag Dorp",
    L"Loyaliteit Dorp",
    //	L"Mijnwerkers",
};

// blank sector strings

wchar_t* nl_pwMiscSectorStrings[] = {
    L"Vijandelijke troepen", L"Sector", L"# Items", L"Onbekend", L"Gecontrolleerd", L"Ja", L"Nee",
};

// error strings for inventory

wchar_t* nl_pMapInventoryErrorString[] = {
    L"%s is niet dichtbij genoeg.",    // Merc is in sector with item but not close enough
    L"Kan huurling niet selecteren.",  // MARK CARTER
    L"%s is niet in de sector om dat item te pakken.",
    L"Tijdens gevechten moet je items handmatig oppakken.",
    L"Tijdens gevechten moet je items handmatig neerleggen.",
    L"%s is niet in de sector om dat item neer te leggen.",
};

wchar_t* nl_pMapInventoryStrings[] = {
    L"Locatie",       // sector these items are in
    L"Aantal Items",  // total number of items in sector
};

// help text for the user

wchar_t* nl_pMapScreenFastHelpTextList[] = {
    L"Om de taken van een huurling te veranderen, zoals team, dokter of repareren, klik dan in de "
    L"'Toewijzen'-kolom",
    L"Om een huurling een ander doel te geven, klik dan in de 'Doel'-kolom",
    L"Op het moment dat een huurling een reis-order gekregen heeft, kan deze met de "
    L"tijd-versneller in beweging worden gezet.",
    L"Links-klikken selecteert de sector. Nogmaals links-klikken geeft de huurling een reisorder. "
    L"Rechts-klikken geeft sector-informatie.",
    L"Druk op een willekeurig moment op 'h'om deze helptekst te krijgen.",
    L"Test Tekst",
    L"Test Tekst",
    L"Test Tekst",
    L"Test Tekst",
    L"Totdat je arriveert in Arulco is er niet veel te doen bij dit scherm. Als je klaar bent met "
    L"het samenstellen van je team, klik dan op de Tijd-Versnel-knop rechtsonder. Zo verstrijkt de "
    L"tijd totdat je team in Arulco aankomt.",
};

// movement menu text

wchar_t* nl_pMovementMenuStrings[] = {
    L"Huurlingen in Sector",  // title for movement box
    L"Teken Reisroute",       // done with movement menu, start plotting movement
    L"Stop",                  // cancel this menu
    L"Anders",                // title for group of mercs not on squads nor in vehicles
};

wchar_t* nl_pUpdateMercStrings[] = {
    L"Oeps:",                        // an error has occured
    L"Contract Huurling verlopen:",  // this pop up came up due to a merc contract ending
    L"Huurling Taak Volbracht:",  // this pop up....due to more than one merc finishing assignments
    L"Huurling weer aan het Werk:",  // this pop up ....due to more than one merc waking up and
                                     // returing to work
    L"Huurling zegt Zzzzzzz:",    // this pop up ....due to more than one merc being tired and going
                                  // to sleep
    L"Contract Loopt Bijna Af:",  // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

wchar_t* nl_pMapScreenBorderButtonHelpText[] = {
    L"Toon Dorpen (|W)",    L"Toon |Mijnen", L"Toon |Teams & Vijanden",
    L"Toon Luchtruim (|A)", L"Toon |Items",  L"Toon Milities & Vijanden (|Z)",
};

wchar_t* nl_pMapScreenBottomFastHelp[] = {
    L"|Laptop", L"Tactisch (|E|s|c)", L"|Opties",
    L"TijdVersneller (|+)",                             // time compress more
    L"TijdVersneller (|-)",                             // time compress less
    L"Vorig Bericht (|U|p)\nVorige Pagina (|P|g|U|p)",  // previous message in scrollable list
    L"Volgend Bericht (|D|o|w|n)\nVolgende pagina (|P|g|D|n)",  // next message in the scrollable
                                                                // list
    L"Start/Stop Tijd (|S|p|a|c|e)",                            // start/stop time compression
};

wchar_t* nl_pMapScreenBottomText[] = {
    L"Huidig Saldo",  // current balance in player bank account
};

wchar_t* nl_pMercDeadString[] = {
    L"%s is dood.",
};

wchar_t* nl_pDayStrings[] = {
    L"Dag",
};

// the list of email sender names

wchar_t* nl_pSenderNameList[] = {
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
    L"Stephan",
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
    L"M.I.S. Verzekeringen",
    L"Bobby Rays",
    L"Kingpin",
    L"John Kulba",
    L"A.I.M.",
};

// next/prev strings

wchar_t* nl_pTraverseStrings[] = {
    L"Vorige",
    L"Volgende",
};

// new mail notify string

wchar_t* nl_pNewMailStrings[] = {
    L"Je hebt nieuwe berichten...",
};

// confirm player's intent to delete messages

wchar_t* nl_pDeleteMailStrings[] = {
    L"Bericht verwijderen?",
    L"ONGELEZEN bericht(en) verwijderen?",
};

// the sort header strings

wchar_t* nl_pEmailHeaders[] = {
    L"Van:",
    L"Subject:",
    L"Dag:",
};

// email titlebar text

wchar_t* nl_pEmailTitleText[] = {
    L"Postvak",
};

// the financial screen strings
wchar_t* nl_pFinanceTitle[] = {
    L"Account Plus",  // the name we made up for the financial program in the game
};

wchar_t* nl_pFinanceSummary[] = {
    L"Credit:",  // credit (subtract from) to player's account
    L"Debet:",   // debit (add to) to player's account
    L"Saldo Gisteren:",
    L"Stortingen Gisteren:",
    L"Uitgaven Gisteren:",
    L"Saldo Eind van de Dag:",
    L"Saldo Vandaag:",
    L"Stortingen Vandaag:",
    L"Uitgaven Vandaag:",
    L"Huidig Saldo:",
    L"Voorspelde Inkomen:",
    L"Geschat Saldo:",  // projected balance for player for tommorow
};

// headers to each list in financial screen

wchar_t* nl_pFinanceHeaders[] = {
    L"Dag",         // the day column
    L"Credit",      // the credits column (to ADD money to your account)
    L"Debet",       // the debits column (to SUBTRACT money from your account)
    L"Transactie",  // transaction type - see TransactionText below
    L"Saldo",       // balance at this point in time
    L"Pag.",        // page number
    L"Dag(en)",     // the day(s) of transactions this page displays
};

wchar_t* nl_pTransactionText[] = {
    L"Toegenomen Interest",  // interest the player has accumulated so far
    L"Anonieme Storting",
    L"Transactiekosten",
    L"Gehuurd",                  // Merc was hired
    L"Bobby Ray's Wapenhandel",  // Bobby Ray is the name of an arms dealer
    L"Rekeningen Voldaan bij M.E.R.C.",
    L"Medische Storting voor %s",  // medical deposit for merc
    L"IMP Profiel Analyse",        // IMP is the acronym for International Mercenary Profiling
    L"Verzekering Afgesloten voor %s",
    L"Verzekering Verminderd voor %s",
    L"Verzekering Verlengd voor %s",  // johnny contract extended
    L"Verzekering Afgebroken voor %s",
    L"Verzekeringsclaim voor %s",  // insurance claim for merc
    L"een dag",                    // merc's contract extended for a day
    L"1 week",                     // merc's contract extended for a week
    L"2 weken",                    // ... for 2 weeks
    L"Inkomen Mijn",
    L"",  // String nuked
    L"Gekochte Bloemen",
    L"Volledige Medische Vergoeding voor %s",
    L"Gedeeltelijke Medische Vergoeding voor %s",
    L"Geen Medische Vergoeding voor %s",
    L"Betaling aan %s",         // %s is the name of the npc being paid
    L"Maak Geld over aan %s",   // transfer funds to a merc
    L"Maak Geld over van %s",   // transfer funds from a merc
    L"Rust militie uit in %s",  // initial cost to equip a town's militia
    L"Items gekocht van %s.",   // is used for the Shop keeper interface.  The dealers name will be
                                // appended to the end of the string.
    L"%s heeft geld gestort.",
};

wchar_t* nl_pTransactionAlternateText[] = {
    L"Verzekering voor",              // insurance for a merc
    L"Contract %s verl. met 1 dag.",  // entend mercs contract by a day
    L"Contract %s verl. met 1 week.",
    L"Contract %s verl. met 2 weken.",
};

// helicopter pilot payment

wchar_t* nl_pSkyriderText[] = {
    L"Skyrider is $%d betaald.",               // skyrider was paid an amount of money
    L"Skyrider heeft $%d tegoed.",             // skyrider is still owed an amount of money
    L"Skyrider is klaar met tanken",           // skyrider has finished refueling
    L"",                                       // unused
    L"",                                       // unused
    L"Skyrider is klaar om weer te vliegen.",  // Skyrider was grounded but has been freed
    L"Skyrider heeft geen passagiers. Als je huurlingen in deze sector wil vervoeren, wijs ze dan "
    L"eerst toe aan Voertuig/Helikopter.",
};

// strings for different levels of merc morale

wchar_t* nl_pMoralStrings[] = {
    L"Super", L"Goed", L"Stabiel", L"Mager", L"Paniek", L"Slecht",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.

wchar_t* nl_pLeftEquipmentString[] = {
    L"%s's uitrusting is nu beschikbaar in Omerta (A9).",
    L"%s's uitrusting is nu beschikbaar in Drassen (B13).",
};

// Status that appears on the Map Screen

wchar_t* nl_pMapScreenStatusStrings[] = {
    L"Gezondheid", L"Energie", L"Moraal",
    L"Conditie",   // the condition of the current vehicle (its "health")
    L"Brandstof",  // the fuel level of the current vehicle (its "energy")
};

wchar_t* nl_pMapScreenPrevNextCharButtonHelpText[] = {
    L"Vorige Huurling (|L|e|f|t)",      // previous merc in the list
    L"Volgende Huurling (|R|i|g|h|t)",  // next merc in the list
};

wchar_t* nl_pEtaString[] = {
    L"aank:",  // eta is an acronym for Estimated Time of Arrival
};

wchar_t* nl_pTrashItemText[] = {
    L"Je bent het voor altijd kwijt. Zeker weten?",  // do you want to continue and lose the
                                                     // item forever
    L"Dit item ziet er HEEL belangrijk uit. Weet je HEEL, HEEL zeker dat je het wil "
    L"weggooien?",  // does the user REALLY want to trash this item

};

wchar_t* nl_pMapErrorString[] = {
    L"Team kan niet verder reizen met een slapende huurling.",

    // 1-5
    L"Verplaats het team eerst bovengronds.",
    L"Reisorders? Het is vijandig gebied!",
    L"Om te verplaatsen moeten huurlingen eerst toegewezen worden aan een team of voertuig.",
    L"Je hebt nog geen team-leden.",                       // you have no members, can't do anything
    L"Huurling kan order niet opvolgen.",                  // merc can't comply with your order
                                                           // 6-10
    L"heeft een escorte nodig. Plaats hem in een team.",   // merc can't move unescorted .. for a
                                                           // male
    L"heeft een escorte nodig. Plaats haar in een team.",  // for a female
    L"Huurling is nog niet in Arulco aangekomen!",
    L"Het lijkt erop dat er eerst nog contractbesprekingen gehouden moeten worden.",
    L"",
    // 11-15
    L"Reisorders? Er is daar een gevecht gaande!",
    L"Je bent in een hinderlaag gelokt van Bloodcats in sector %s!",
    L"Je bent in sector I16 iets binnengelopen dat lijkt op het hol van een bloodcat!",
    L"",
    L"De SAM-stelling in %s is overgenomen.",
    // 16-20
    L"De mijn in %s is overgenomen. Je dagelijkse inkomen is gereduceerd tot %s per dag.",
    L"De vijand heeft sector %s onbetwist overgenomen.",
    L"Tenminste een van je huurlingen kan niet meedoen met deze opdracht.",
    L"%s kon niet meedoen met %s omdat het al vol is",
    L"%s kon niet meedoen met %s omdat het te ver weg is.",
    // 21-25
    L"De mijn in %s is buitgemaakt door Deidranna's troepen!",
    L"Deidranna's troepen zijn net de SAM-stelling in %s binnengevallen",
    L"Deidranna's troepen zijn net %s binnengevallen",
    L"Deidranna's troepen zijn gezien in %s.",
    L"Deidranna's troepen hebben zojuist %s overgenomen.",
    // 26-30
    L"Tenminste één huurling kon niet tot slapen gebracht worden.",
    L"Tenminste één huurling kon niet wakker gemaakt worden.",
    L"De Militie verschijnt niet totdat hun training voorbij is.",
    L"%s kan geen reisorders gegeven worden op dit moment.",
    L"Milities niet binnen de stadsgrenzen kunnen niet verplaatst worden naar een andere sector.",
    // 31-35
    L"Je kunt geen militie in %s hebben.",
    L"Een voertuig kan niet leeg rijden!",
    L"%s is te gewond om te reizen!",
    L"Je moet het museum eerst verlaten!",
    L"%s is dood!",
    // 36-40
    L"%s kan niet wisselen naar %s omdat het onderweg is",
    L"%s kan het voertuig op die manier niet in",
    L"%s kan zich niet aansluiten bij %s",
    L"Totdat je nieuwe huurlingen in dienst neemt, kan de tijd niet versneld worden!",
    L"Dit voertuig kan alleen over wegen rijden!",
    // 41-45
    L"Je kunt geen reizende huurlingen opnieuw toewijzen",
    L"Voertuig zit zonder brandstof!",
    L"%s is te moe om te reizen.",
    L"Niemand aan boord is in staat om het voertuig te besturen.",
    L"Eén of meer teamleden kunnen zich op dit moment niet verplaatsen.",
    // 46-50
    L"Eén of meer leden van de ANDERE huurlingen kunnen zich op dit moment niet verplaatsen.",
    L"Voertuig is te beschadigd!",
    L"Let op dat maar twee huurlingen milities in een sector mogen trainen.",
    L"De robot kan zich zonder bediening niet verplaatsen. Plaats ze in hetzelfde team.",
};

// help text used during strategic route plotting
wchar_t* nl_pMapPlotStrings[] = {
    L"Klik nogmaals op de bestemming om de route te bevestigen, of klik op een andere sector om "
    L"meer routepunten te plaatsen.",
    L"Route bevestigd.",
    L"Bestemming onveranderd.",
    L"Reis afgebroken.",
    L"Reis verkort.",
};

// help text used when moving the merc arrival sector
wchar_t* nl_pBullseyeStrings[] = {
    L"Klik op de sector waar de huurlingen in plaats daarvan moeten arriveren.",
    L"OK. Arriverende huurlingen worden afgezet in %s",
    L"Huurlingen kunnen hier niet ingevlogen worden, het luchtruim is onveilig!",
    L"Afgebroken. Aankomst-sector onveranderd",
    L"Luchtruim boven %s is niet langer veilig! Aankomst-sector is verplaatst naar %s.",
};

// help text for mouse regions

wchar_t* nl_pMiscMapScreenMouseRegionHelpText[] = {
    L"Naar Inventaris (|E|n|t|e|r)",
    L"Gooi Item Weg",
    L"Verlaat Inventaris (|E|n|t|e|r)",
};

// male version of where equipment is left
wchar_t* nl_pMercHeLeaveString[] = {
    L"Laat %s zijn uitrusting achterlaten waar hij nu is (%s) of in Drassen (B13) bij het nemen "
    L"van de vlucht uit Arulco?",
    L"Laat %s zijn uitrusting achterlaten waar hij nu is (%s) of later in Omerta (A9) bij het "
    L"nemen van de vlucht uit Arulco?",
    L"gaat binnenkort weg en laat zijn uitrusting achter in Omerta (A9).",
    L"gaat binnenkort weg en laat zijn uitrusting achter in Drassen (B13).",
    L"%s gaat binnenkort weg en laat zijn uitrusting achter in %s.",
};

// female version
wchar_t* nl_pMercSheLeaveString[] = {
    L"Laat %s haar uitrusting achterlaten waar ze nu is (%s) of in Drassen (B13) bij het nemen van "
    L"de vlucht uit Arulco?",
    L"Laat %s haar uitrusting achterlaten waar ze nu is (%s) of later in Omerta (A9) bij het nemen "
    L"van de vlucht uit Arulco?",
    L"gaat binnenkort weg en laat haar uitrusting achter in Omerta (A9).",
    L"gaat binnenkort weg en laat haar uitrusting achter in Drassen (B13).",
    L"%s gaat binnenkort weg en laat haar uitrusting achter in %s.",
};

wchar_t* nl_pMercContractOverStrings[] = {
    L"'s contract is geëindigd, hij is dus naar huis.",  // merc's contract is over and has departed
    L"'s contract is geëindigd, ze is dus naar huis.",   // merc's contract is over and has departed
    L"'s contract is opgezegd, hij is dus weg.",         // merc's contract has been terminated
    L"'s contract is opgezegd, ze is dus weg.",          // merc's contract has been terminated
    L"M.E.R.C. krijgt nog teveel geld van je, %s is dus weggegaan.",  // Your M.E.R.C. account is
                                                                      // invalid so merc left
};

// Text used on IMP Web Pages

wchar_t* nl_pImpPopUpStrings[] = {
    L"Ongeldige Autorisatiecode",
    L"Je wil het gehele persoonlijkheidsonderzoek te herstarten. Zeker weten?",
    L"Vul alsjeblieft de volledige naam en geslacht in",
    L"Voortijdig onderzoek van je financiële status wijst uit dat je een "
    L"persoonlijksheidsonderzoek niet kunt betalen.",
    L"Geen geldige optie op dit moment.",
    L"Om een nauwkeurig profiel te maken, moet je ruimte hebben voor tenminste één teamlid.",
    L"Profiel is al gemaakt.",
};

// button labels used on the IMP site

wchar_t* nl_pImpButtonText[] = {
    L"Info",                                     // about the IMP site ("About Us")
    L"BEGIN",                                    // begin profiling ("BEGIN")
    L"Persoonlijkheid",                          // personality section ("Personality")
    L"Eigenschappen",                            // personal stats/attributes section ("Attributes")
    L"Portret",                                  // the personal portrait selection ("Portrait")
    L"Stem %d",                                  // the voice selection ("Voice %d")
    L"OK",                                       // done profiling ("Done")
    L"Opnieuw",                                  // start over profiling ("Start Over")
    L"Ja, ik kies het geselecteerde antwoord.",  // ("Yes, I choose the highlighted answer.")
    L"Ja", L"Nee",
    L"OK",                             // finished answering questions
    L"Vor.",                           // previous question..abbreviated form
    L"Vol.",                           // next question
    L"JA ZEKER.",                      // yes, I am certain ("YES, I AM.")
    L"NEE, IK WIL OPNIEUW BEGINNEN.",  // no, I want to start over the profiling process ("NO, I
                                       // WANT TO START OVER.")
    L"JA, ZEKER.",                     // ("YES, I DO.")
    L"NEE",
    L"Terug",                          // back one page
    L"Stop",                           // cancel selection
    L"Ja, zeker weten.",               // ("Yes, I am certain.")
    L"Nee, laat me nog eens kijken.",  // ("No, let me have another look.")
    L"Registratie",                    // the IMP site registry..when name and gender is selected
    L"Analyseren",                     // analyzing your profile results
    L"OK",
    L"Stem",  // "Voice"
};

wchar_t* nl_pExtraIMPStrings[] = {
    L"Selecteer Persoonlijkheid om eigenlijke onderzoek te starten.",
    L"Nu het onderzoek compleet is, selecteer je eigenschappen.",
    L"Nu de eigenschappen gekozen zijn, kun je verder gaan met de portretselectie.",
    L"Selecteer de stem die het best bij je past om het proces te voltooien.",
};

wchar_t* nl_pFilesTitle[] = {
    L"Bestanden Bekijken",  // ("File Viewer")
};

wchar_t* nl_pFilesSenderList[] = {
    L"Int. Verslag",  // the recon report sent to the player. Recon is an abbreviation for
                      // reconissance
    L"Intercept.#1",  // first intercept file .. Intercept is the title of the organization sending
                      // the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in
                      // files.txt for the translated title
    L"Intercept.#2",  // second intercept file
    L"Intercept.#3",  // third intercept file
    L"Intercept.#4",  // fourth intercept file ("Intercept #4")
    L"Intercept.#5",  // fifth intercept file
    L"Intercept.#6",  // sixth intercept file
};

// Text having to do with the History Log

wchar_t* nl_pHistoryTitle[] = {
    L"Geschiedenis",
};

wchar_t* nl_pHistoryHeaders[] = {
    L"Dag",      // the day the history event occurred
    L"Pag.",     // the current page in the history report we are in
    L"Dag",      // the days the history report occurs over
    L"Locatie",  // location (in sector) the event occurred
    L"Geb.",     // the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
wchar_t* nl_pHistoryStrings[] = {
    L"",  // leave this line blank
    // 1-5
    L"%s ingehuurd via A.I.M.",                       // merc was hired from the aim site
    L"%s ingehuurd via M.E.R.C.",                     // merc was hired from the merc site
    L"%s gedood.",                                    // merc was killed
    L"Facturen betaald bij M.E.R.C.",                 // paid outstanding bills at MERC
    L"Opdracht van Enrico Chivaldori geaccepteerd.",  // ("Accepted Assignment From Enrico
                                                      // Chivaldori")
    // 6-10
    L"IMP Profiel Klaar",                     // ("IMP Profile Generated")
    L"Verzekeringspolis gekocht voor %s.",    // insurance contract purchased
    L"Verzekeringspolis afgebroken van %s.",  // insurance contract canceled
    L"Uitbetaling Verzekeringspolis %s.",     // insurance claim payout for merc
    L"%s's contract verlengd met 1 dag.",     // Extented "mercs name"'s for a day
    // 11-15
    L"%s's contract verlengd met 1 week.",   // Extented "mercs name"'s for a week
    L"%s's contract verlengd met 2 weken.",  // Extented "mercs name"'s 2 weeks
    L"%s is ontslagen.",                     // "merc's name" was dismissed.
    L"%s gestopt.",                          // "merc's name" quit.
    L"zoektocht gestart.",                   // a particular quest started
    // 16-20
    L"zoektocht afgesloten.",                     // ("quest completed.")
    L"Gepraat met hoofdmijnwerker van %s",        // talked to head miner of town
    L"%s bevrijd",                                // ("Liberated %s")
    L"Vals gespeeld",                             // ("Cheat Used")
    L"Voedsel zou morgen in Omerta moeten zijn",  // ("Food should be in Omerta by tomorrow")
    // 21-25
    L"%s weggegaan, wordt Daryl Hick's vrouw",  // ("%s left team to become Daryl Hick's wife")
    L"%s's contract afgelopen.",                // ("%s's contract expired.")
    L"%s aangenomen.",                          // ("%s was recruited.")
    L"Enrico klaagde over de voortgang",        // ("Enrico complained about lack of progress")
    L"Strijd gewonnen",                         // ("Battle won")
    // 26-30
    L"%s mijn raakt uitgeput",                // ("%s mine started running out of ore")
    L"%s mijn is uitgeput",                   // ("%s mine ran out of ore")
    L"%s mijn is gesloten",                   // ("%s mine was shut down")
    L"%s mijn heropend",                      // ("%s mine was reopened")
    L"Info verkregen over gevangenis Tixa.",  // ("Found out about a prison called Tixa.")
    // 31-35
    L"Van geheime wapenfabriek gehoord, Orta genaamd.",  // ("Heard about a secret weapons plant
                                                         // called Orta.")
    L"Onderzoeker in Orta geeft wat raketwerpers.",  // ("Scientist in Orta donated a slew of rocket
                                                     // rifles.")
    L"Koningin Deidranna kickt op lijken.",  // ("Queen Deidranna has a use for dead bodies.")
    L"Frank vertelde over knokwedstrijden in San Mona.",  // ("Frank talked about fighting matches
                                                          // in San Mona.")
    L"Een patiënt dacht dat ie iets in de mijnen zag.",   // ("A patient thinks he saw something in
                                                          // the mines.")
    // 36-40
    L"Pers. ontmoet; Devin - verkoopt explosieven.",  // ("Met someone named Devin - he sells
                                                      // explosives.")
    L"Beroemde ex-AIM huurling Mike ontmoet!",        // ("Ran into the famous ex-AIM merc Mike!")
    L"Tony ontmoet - handelt in wapens.",             // ("Met Tony - he deals in arms.")
    L"Raketwerper gekregen van Serg. Krott.",         // ("Got a rocket rifle from Sergeant Krott.")
    L"Kyle akte gegeven van Angel's leerwinkel.",     // ("Gave Kyle the deed to Angel's leather
                                                      // shop.")
    // 41-45
    L"Madlab bood aan robot te bouwen.",          // ("Madlab offered to build a robot.")
    L"Gabby maakt superbrouwsel tegen beesten.",  // ("Gabby can make stealth concoction for bugs.")
    L"Keith is er mee opgehouden.",               // ("Keith is out of business.")
    L"Howard geeft Koningin Deidranna cyanide.",  // ("Howard provided cyanide to Queen Deidranna.")
    L"Keith ontmoet - handelaar in Cambria.",     // ("Met Keith - all purpose dealer in Cambria.")
    // 46-50
    L"Howard ontmoet - medicijnendealer in Balime",  // ("Met Howard - deals pharmaceuticals in
                                                     // Balime")
    L"Perko ontmoet - heeft reparatiebedrijfje.",  // ("Met Perko - runs a small repair business.")
    L"Sam van Balime ontmoet - verkoopt ijzerwaren.",  // ("Met Sam of Balime - runs a hardware
                                                       // shop.")
    L"Franz verkoopt elektronica en andere dingen.",   // ("Franz deals in electronics and other
                                                       // goods.")
    L"Arnold runt reparatiezaak in Grumm.",            // ("Arnold runs a repair shop in Grumm.")
    // 51-55
    L"Fredo repareert elektronica in Grumm.",            // ("Fredo repairs electronics in Grumm.")
    L"Van rijke vent in Balime donatie gekregen.",       // ("Received donation from rich guy in
                                                         // Balime.")
    L"Schroothandelaar Jake ontmoet.",                   // ("Met a junkyard dealer named Jake.")
    L"Vaag iemand gaf ons elektronische sleutelkaart.",  // ("Some bum gave us an electronic
                                                         // keycard.")
    L"Walter omgekocht om kelderdeur open te maken.",  // ("Bribed Walter to unlock the door to the
                                                       // basement.")
    // 56-60
    L"Als Dave gas heeft, geeft hij deze weg.",  // ("If Dave has gas, he'll provide free fillups.")
    L"Geslijmd met Pablo.",                      // ("Greased Pablo's palms.")
    L"Kingpin bewaard geld in San Mona mine.",   // ("Kingpin keeps money in San Mona mine.")
    L"%s heeft Extreme Fighting gewonnen",       // ("%s won Extreme Fighting match")
    L"%s heeft Extreme Fighting verloren",       // ("%s lost Extreme Fighting match")
    // 61-65
    L"%s gediskwalificeerd v. Extreme Fighting",  // ("%s was disqualified in Extreme Fighting")
    L"Veel geld gevonden in een verlaten mijn.",  // ("Found a lot of money stashed in the abandoned
                                                  // mine.")
    L"Huurmoordenaar van Kingpin ontdekt.",       // ("Encountered assassin sent by Kingpin.")
    L"Controle over sector verloren",             // ENEMY_INVASION_CODE ("Lost control of sector")
    L"Sector verdedigd",                          // ("Defended sector")
    // 66-70
    L"Strijd verloren",           // ENEMY_ENCOUNTER_CODE ("Lost battle")
    L"Fatale val",                // ENEMY_AMBUSH_CODE ("Fatal ambush")
    L"Vijandige val weggevaagd",  // ("Wiped out enemy ambush")
    L"Aanval niet gelukt",        // ENTERING_ENEMY_SECTOR_CODE ("Unsuccessful attack")
    L"Aanval gelukt!",            // ("Successful attack!")
    // 71-75
    L"Beesten vielen aan",                 // CREATURE_ATTACK_CODE ("Creatures attacked")
    L"Gedood door bloodcats",              // BLOODCAT_AMBUSH_CODE ("Killed by bloodcats")
    L"Afgeslacht door bloodcats",          // ("Slaughtered bloodcats")
    L"%s was gedood",                      // ("%s was killed")
    L"Carmen kop v.e. terrorist gegeven",  // ("Gave Carmen a terrorist's head")
    L"Slay vertrok",                       // ("Slay left")
    L"%s vermoord",                        // ("Killed %s")
};

wchar_t* nl_pHistoryLocations[] = {
    L"Nvt",  // N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop

wchar_t* nl_pLaptopIcons[] = {
    L"E-mail",      L"Web", L"Financieel", L"Dossiers", L"Historie", L"Bestanden", L"Afsluiten",
    L"sir-FER 4.0",  // our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

wchar_t* nl_pBookMarkStrings[] = {
    L"A.I.M.",     L"Bobby Ray's", L"I.M.P",       L"M.E.R.C.",
    L"Mortuarium", L"Bloemist",    L"Verzekering", L"Stop",
};

wchar_t* nl_pBookmarkTitle[] = {
    L"Bladwijzer",
    L"Rechter muisklik om dit menu op te roepen.",
};

// When loading or download a web page

wchar_t* nl_pDownloadString[] = {
    L"Laden",
    L"Herladen",
};

// This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

wchar_t* nl_gsAtmSideButtonText[] = {
    L"OK",
    L"Neem",  // take money from merc
    L"Geef",  // give money to merc
    L"Stop",  // cancel transaction
    L"Leeg",  // clear amount being displayed on the screen
};

wchar_t* nl_gsAtmStartButtonText[] = {
    L"Maak over $",  // transfer money to merc -- short form
    L"Info",         // view stats of the merc
    L"Inventaris",   // view the inventory of the merc
    L"Werk",
};

wchar_t* nl_sATMText[] = {
    L"Overmaken geld?",                    // transfer funds to merc?
    L"Ok?",                                // are we certain?
    L"Geef bedrag",                        // enter the amount you want to transfer to merc
    L"Geef type",                          // select the type of transfer to merc
    L"Onvoldoende saldo",                  // not enough money to transfer to merc
    L"Bedrag moet veelvoud zijn van $10",  // transfer amount must be a multiple of $10
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

wchar_t* nl_pErrorStrings[] = {
    L"Fout",
    L"Server heeft geen DNS ingang.",
    L"Controleer URL adres en probeer opnieuw.",
    L"OK",
    L"Periodieke verbinding met host. Houdt rekening met lange wachttijden.",
};

wchar_t* nl_pPersonnelString[] = {
    L"Huurlingen:",  // mercs we have
};

wchar_t* nl_pWebTitle[] = {
    L"sir-FER 4.0",  // our name for the version of the browser, play on company name
};

// The titles for the web program title bar, for each page loaded

wchar_t* nl_pWebPagesTitles[] = {
    L"A.I.M.",
    L"A.I.M. Leden",
    L"A.I.M. Portretten",  // a mug shot is another name for a portrait
    L"A.I.M. Sorteer",
    L"A.I.M.",
    L"A.I.M. Veteranen",
    L"A.I.M. Regelement",
    L"A.I.M. Geschiedenis",
    L"A.I.M. Links",
    L"M.E.R.C.",
    L"M.E.R.C. Rekeningen",
    L"M.E.R.C. Registratie",
    L"M.E.R.C. Index",
    L"Bobby Ray's",
    L"Bobby Ray's - Wapens",
    L"Bobby Ray's - Munitie",
    L"Bobby Ray's - Pantsering",
    L"Bobby Ray's - Diversen",  // misc is an abbreviation for miscellaneous
    L"Bobby Ray's - Gebruikt",
    L"Bobby Ray's - Mail Order",
    L"I.M.P.",
    L"I.M.P.",
    L"United Floral Service",
    L"United Floral Service - Etalage",
    L"United Floral Service - Bestelformulier",
    L"United Floral Service - Kaart Etalage",
    L"Malleus, Incus & Stapes Verzekeringen",
    L"Informatie",
    L"Contract",
    L"Opmerkingen",
    L"McGillicutty's Mortuarium",
    L"",
    L"URL niet gevonden.",
    L"Bobby Ray's - Recentelijke Zendingen",
    L"",
    L"",
};

wchar_t* nl_pShowBookmarkString[] = {
    L"Sir-Help",
    L"Klik opnieuw voor Bookmarks.",
};

wchar_t* nl_pLaptopTitles[] = {
    L"E-Mail", L"Bestanden bekijken", L"Persoonlijk", L"Boekhouder Plus", L"Geschiedenis",
};

wchar_t* nl_pPersonnelDepartedStateStrings[] = {
    // reasons why a merc has left.
    L"Omgekomen tijdens gevechten", L"Weggestuurd", L"Anders", L"Getrouwd",
    L"Contract Afgelopen",          L"Gestopt",
};
// personnel strings appearing in the Personnel Manager on the laptop

wchar_t* nl_pPersonelTeamStrings[] = {
    L"Huidig Team",     L"Vertrekken",      L"Dag. Kosten:",
    L"Hoogste Kosten:", L"Laagste Kosten:", L"Omgekomen tijdens gevechten:",
    L"Weggestuurd:",    L"Anders:",
};

wchar_t* nl_pPersonnelCurrentTeamStatsStrings[] = {
    L"Laagste",
    L"Gemiddeld",
    L"Hoogste",
};

wchar_t* nl_pPersonnelTeamStatsStrings[] = {
    L"GZND", L"BEW", L"HAN", L"KRA", L"LDR", L"WIJ", L"NIV", L"TREF", L"MECH", L"EXPL", L"MED",
};

// horizontal and vertical indices on the map screen

wchar_t* nl_pMapVertIndex[] = {
    L"X", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H",
    L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P",
};

wchar_t* nl_pMapHortIndex[] = {
    L"X", L"1",  L"2",  L"3",  L"4",  L"5",  L"6",  L"7",  L"8",
    L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16",
};

wchar_t* nl_pMapDepthIndex[] = {
    L"",
    L"-1",
    L"-2",
    L"-3",
};

// text that appears on the contract button

wchar_t* nl_pContractButtonString[] = {
    L"Contract",
};

// text that appears on the update panel buttons

wchar_t* nl_pUpdatePanelButtons[] = {
    L"Doorgaan",
    L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

wchar_t* nl_LargeTacticalStr[] = {
    L"Je bent verslagen in deze sector!",
    L"De vijand, geen genade kennende, slacht ieder teamlid af!",
    L"Je bewusteloze teamleden zijn gevangen genomen!",
    L"Je teamleden zijn gevangen genomen door de vijand.",
};

// Insurance Contract.c
// The text on the buttons at the bottom of the screen.

wchar_t* nl_InsContractText[] = {
    L"Vorige",
    L"Volgende",
    L"OK",
    L"Leeg",
};

// Insurance Info
// Text on the buttons on the bottom of the screen

wchar_t* nl_InsInfoText[] = {
    L"Vorige",
    L"Volgende",
};

// For use at the M.E.R.C. web site. Text relating to the player's account with MERC

wchar_t* nl_MercAccountText[] = {
    // Text on the buttons on the bottom of the screen
    L"Autoriseer",
    L"Thuis",
    L"Rekening#:",
    L"Huurl.",
    L"Dagen",
    L"Tarief",  // 5
    L"Prijs",
    L"Totaal:",
    L"Weet je zeker de betaling van %s te autoriseren?",  // the %s is a string that contains the
                                                          // dollar amount ( ex. "$150" )
};

// For use at the M.E.R.C. web site. Text relating a MERC mercenary

wchar_t* nl_MercInfo[] = {
    L"Gezondheid",
    L"Beweeglijkheid",
    L"Handigheid",
    L"Kracht",
    L"Leiderschap",
    L"Wijsheid",
    L"Ervaringsniveau",
    L"Trefzekerheid",
    L"Technisch",
    L"Explosieven",
    L"Medisch",

    L"Vorige",
    L"Huur",
    L"Volgende",
    L"Extra Info",
    L"Thuis",
    L"Ingehuurd",
    L"Salaris:",
    L"Per Dag",
    L"Overleden",

    L"Lijkt erop dat je teveel huurlingen wil recruteren. Limiet is 18.",
    L"Niet beschikbaar",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

wchar_t* nl_MercNoAccountText[] = {
    // Text on the buttons at the bottom of the screen
    L"Open Rekening",
    L"Afbreken",
    L"Je hebt geen rekening. Wil je er één openen?",
};

// For use at the M.E.R.C. web site. MERC Homepage

wchar_t* nl_MercHomePageText[] = {
    // Description of various parts on the MERC page
    L"Speck T. Kline, oprichter en bezitter",
    L"Om een rekening te open, klik hier",
    L"Klik hier om rekening te bekijken",
    L"Klik hier om bestanden in te zien",
    // The version number on the video conferencing system that pops up when Speck is talking
    L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

wchar_t* nl_sFuneralString[] = {
    L"McGillicutty's Mortuarium: Helpt families rouwen sinds 1983.",
    L"Begrafenisondernemer en voormalig A.I.M. huurling Murray \"Pops\" McGillicutty is een kundig "
    L"en ervaren begrafenisondernemer.",
    L"Pops weet hoe moeilijk de dood kan zijn, in heel zijn leven heeft hij te maken gehad met de "
    L"dood en sterfgevallen.",
    L"McGillicutty's Mortuarium biedt een breed scala aan stervensbegeleiding, van een schouder om "
    L"uit te huilen tot recontructie van misvormde overblijfselen.",
    L"Laat McGillicutty's Mortuarium u helpen en laat uw dierbaren zacht rusten.",

    // Text for the various links available at the bottom of the page
    L"STUUR BLOEMEN",
    L"DOODSKIST & URN COLLECTIE",
    L"CREMATIE SERVICE",
    L"SERVICES",
    L"ETIQUETTE",

    // The text that comes up when you click on any of the links ( except for send flowers ).
    L"Helaas is deze pagina nog niet voltooid door een sterfgeval in de familie. Afhankelijk van "
    L"de laatste wil en uitbetaling van de beschikbare activa wordt de pagina zo snel mogelijk "
    L"voltooid.",
    L"Ons medeleven gaat uit naar jou, tijdens deze probeerperiode. Kom nog eens langs.",
};

// Text for the florist Home page

wchar_t* nl_sFloristText[] = {
    // Text on the button on the bottom of the page

    L"Etalage",

    // Address of United Florist

    L"\"We brengen overal langs\"",
    L"1-555-SCENT-ME",
    L"333 NoseGay Dr, Seedy City, CA USA 90210",
    L"http://www.scent-me.com",

    // detail of the florist page

    L"We zijn snel en efficiënt!",
    L"Volgende dag gebracht, wereldwijd, gegarandeerd. Enkele beperkingen zijn van toepassing.",
    L"Laagste prijs in de wereld, gegarandeerd!",
    L"Toon ons een lagere geadverteerde prijs voor een regeling en ontvang gratis een dozijn "
    L"rozen.",
    L"Flora, Fauna & Bloemen sinds 1981.",
    L"Onze onderscheiden ex-bommenwerperpiloten droppen je boeket binnen een tien kilometer radius "
    L"van de gevraagde locatie. Altijd!",
    L"Laat ons al je bloemenfantasieën waarmaken.",
    L"Laat Bruce, onze wereldberoemde bloemist, de verste bloemen met de hoogste kwaliteit uit "
    L"onze eigen kassen uitzoeken.",
    L"En onthoudt, als we het niet hebben, kunnen we het kweken - Snel!",
};

// Florist OrderForm

wchar_t* nl_sOrderFormText[] = {
    // Text on the buttons

    L"Terug",
    L"Verstuur",
    L"Leeg",
    L"Etalage",

    L"Naam vh Boeket:",
    L"Prijs:",  // 5
    L"Ordernummer:",
    L"Bezorgingsdatum",
    L"volgende dag",
    L"komt wanneer het komt",
    L"Locatie Bezorging",  // 10
    L"Extra Service",
    L"Geplet Boeket($10)",
    L"Zwarte Rozen ($20)",
    L"Verlept Boeket($10)",
    L"Fruitcake (indien beschikbaar)($10)",  // 15
    L"Persoonlijk Bericht:",
    L"Wegens de grootte kaarten, mogen je berichten niet langer zijn dan 75 karakters.",
    L"...of selecteer er één van de onze",

    L"STANDAARDKAARTEN",
    L"Factuurinformatie",  // 20

    // The text that goes beside the area where the user can enter their name

    L"Naam:",
};

// Florist Gallery.c

wchar_t* nl_sFloristGalleryText[] = {
    // text on the buttons

    L"Back",  // abbreviation for previous
    L"Next",  // abbreviation for next

    L"Klik op de selectie die je wil bestellen.",
    L"Let op: er geldt een extra tarief van $10 voor geplette en verlepte boeketten.",

    // text on the button

    L"Home",
};

// Florist Cards

wchar_t* nl_sFloristCards[] = {
    L"Klik op je selectie",
    L"Terug",
};

// Text for Bobby Ray's Mail Order Site

wchar_t* nl_BobbyROrderFormText[] = {
    L"Bestelformulier",         // Title of the page
    L"Hvl",                     // The number of items ordered
    L"Gewicht(%s)",             // The weight of the item
    L"Itemnaam",                // The name of the item
    L"Prijs unit",              // the item's weight
    L"Totaal",                  // 5	// The total price of all of items of the same type
    L"Sub-Totaal",              // The sub total of all the item totals added
    L"Porto (Zie Bezorgloc.)",  // S&H is an acronym for Shipping and Handling
    L"Eindtotaal",              // The grand total of all item totals + the shipping and handling
    L"Bezorglocatie",
    L"Verzendingssnelheid",         // 10	// See below
    L"Kosten (per %s.)",            // The cost to ship the items
    L"Nacht-Express",               // Gets deliverd the next day
    L"2 Werkdagen",                 // Gets delivered in 2 days
    L"Standaard Service",           // Gets delivered in 3 days
    L"Order Leegmaken",             // 15			// Clears the order page
    L"Accept. Order",               // Accept the order
    L"Terug",                       // text on the button that returns to the previous page
    L"Home",                        // Text on the button that returns to the home page
    L"* Duidt op Gebruikte Items",  // Disclaimer stating that the item is used
    L"Je kunt dit niet betalen.",   // 20	// A popup message that to warn of not enough money
    L"<GEEN>",                      // Gets displayed when there is no valid city selected
    L"Weet je zeker dat je de bestelling wil sturen naar %s?",  // A popup that asks if the city
                                                                // selected is the correct one
    L"Gewicht Pakket**",  // Displays the weight of the package
    L"** Min. Gew.",      // Disclaimer states that there is a minimum weight for the package
    L"Zendingen",
};

// This text is used when on the various Bobby Ray Web site pages that sell items

wchar_t* nl_BobbyRText[] = {
    L"Bestelling",  // Title
    // instructions on how to order
    L"Klik op de item(s). Voor meer dan één, blijf dan klikken. Rechtsklikken voor minder. Als je "
    L"alles geselecteerd hebt, dat je wil bestellen, ga dan naar het bestelformulier.",

    // Text on the buttons to go the various links

    L"Vorige Items",  //
    L"Wapens",        // 3
    L"Munitie",       // 4
    L"Pantser",       // 5
    L"Diversen",      // 6	//misc is an abbreviation for miscellaneous
    L"Gebruikt",      // 7
    L"Meer Items",
    L"BESTELFORMULIER",
    L"Home",  // 10

    // The following 2 lines are used on the Ammunition page.
    // They are used for help text to display how many items the player's merc has
    // that can use this type of ammo

    L"Je team heeft",                              // 11
    L"wapen(s) gebruik makende van deze munitie",  // 12

    // The following lines provide information on the items

    L"Gewicht:",          // Weight of all the items of the same type
    L"Kal:",              // the caliber of the gun
    L"Mag:",              // number of rounds of ammo the Magazine can hold
    L"Afs:",              // The range of the gun
    L"Sch:",              // Damage of the weapon
    L"ROF:",              // Weapon's Rate Of Fire, acronym ROF
    L"Kost:",             // Cost of the item
    L"Aanwezig:",         // The number of items still in the store's inventory
    L"# Besteld:",        // The number of items on order
    L"Beschadigd",        // If the item is damaged
    L"Gewicht:",          // the Weight of the item
    L"SubTotaal:",        // The total cost of all items on order
    L"* %% Functioneel",  // if the item is damaged, displays the percent function of the item

    // Popup that tells the player that they can only order 10 items at a time

    L"Verdraaid! Dit on-line bestelformulier accepteert maar 10 items per keer. Als je meer wil "
    L"bestellen (en dat hopen we), plaats dan afzonderlijke orders en accepteer onze excuses.",

    // A popup that tells the user that they are trying to order more items then the store has in
    // stock

    L"Sorry. We hebben niet meer van die zaken in het magazijn. Probeer het later nog eens.",

    // A popup that tells the user that the store is temporarily sold out

    L"Sorry, alle items van dat type zijn nu uitverkocht.",

};

// Text for Bobby Ray's Home Page

wchar_t* nl_BobbyRaysFrontText[] = {
    // Details on the web site

    L"Hier moet je zijn voor de nieuwste en beste wapens en militaire goederen",
    L"We kunnen de perfecte oplossing vinden voor elke explosiebehoefte",
    L"Gebruikte en opgeknapte items",

    // Text for the various links to the sub pages

    L"Diversen",
    L"WAPENS",
    L"MUNITIE",  // 5
    L"PANTSER",

    // Details on the web site

    L"Als wij het niet verkopen, dan kun je het nergens krijgen!",
    L"Under construction",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug
// shot page

wchar_t* nl_AimSortText[] = {
    L"A.I.M. Leden",  // Title
    // Title for the way to sort
    L"Sort. op:",

    // sort by...

    L"Prijs",
    L"Ervaring",
    L"Trefzekerheid",
    L"Medisch",
    L"Explosieven",
    L"Technisch",

    // Text of the links to other AIM pages

    L"Bekijk portretfotoindex van huurlingen",
    L"Bekijk het huurlingendossier",
    L"Bekijk de A.I.M. Veteranen",

    // text to display how the entries will be sorted

    L"Oplopend",
    L"Aflopend",
};

// Aim Policies.c
// The page in which the AIM policies and regulations are displayed

wchar_t* nl_AimPolicyText[] = {
    // The text on the buttons at the bottom of the page

    L"Previous", L"AIM HomePage", L"Index Regels", L"Next", L"Oneens", L"Mee eens",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot
// index

wchar_t* nl_AimMemberText[] = {
    L"Klik Links",
    L"voor Verbinding met Huurl.",
    L"Klik Rechts",
    L"voor Portretfotoindex.",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

wchar_t* nl_CharacterInfo[] = {
    // The various attributes of the merc

    L"Gezondheid", L"Beweeglijkheid", L"Handigheid", L"Kracht", L"Leiderschap", L"Wijsheid",
    L"Ervaringsniveau", L"Trefzekerheid", L"Technisch", L"Explosieven",
    L"Medisch",  // 10

    // the contract expenses' area

    L"Tarief", L"Contract", L"een dag", L"een week", L"twee weken",

    // text for the buttons that either go to the previous merc,
    // start talking to the merc, or go to the next merc

    L"Previous", L"Contact", L"Next",

    L"Extra Info",                  // Title for the additional info for the merc's bio
    L"Actieve Leden",               // 20		// Title of the page
    L"Aanv. Uitrusting:",           // Displays the optional gear cost
    L"MEDISCHE aanbetaling nodig",  // If the merc required a medical deposit, this is displayed
};

// Aim Member.c
// The page in which the player's hires AIM mercenaries

// The following text is used with the video conference popup

wchar_t* nl_VideoConfercingText[] = {
    L"Contractkosten:",  // Title beside the cost of hiring the merc

    // Text on the buttons to select the length of time the merc can be hired

    L"Een Dag", L"Een Week", L"Twee Weken",

    // Text on the buttons to determine if you want the merc to come with the equipment

    L"Geen Uitrusting", L"Koop Uitrusting",

    // Text on the Buttons

    L"HUUR IN",      // to actually hire the merc
    L"STOP",         // go back to the previous menu
    L"VOORWAARDEN",  // go to menu in which you can hire the merc
    L"OPHANGEN",     // stops talking with the merc
    L"OK",
    L"STUUR BERICHT",  // if the merc is not there, you can leave a message

    // Text on the top of the video conference popup

    L"Video Conference met", L"Verbinding maken. . .",

    L"+ med. depo",  // Displays if you are hiring the merc with the medical deposit
};

// Aim Member.c
// The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

wchar_t* nl_AimPopUpText[] = {
    L"BEDRAG OVERGEBOEKT",       // You hired the merc
    L"OVERMAKEN NIET MOGELIJK",  // Player doesn't have enough money, message 1
    L"ONVOLDOENDE GELD",         // Player doesn't have enough money, message 2

    // if the merc is not available, one of the following is displayed over the merc's face

    L"Op missie",
    L"Laat a.u.b. bericht achter",
    L"Overleden",

    // If you try to hire more mercs than game can support

    L"Je team bestaat al uit 18 huurlingen.",

    L"Opgenomen bericht",
    L"Bericht opgenomen",
};

// AIM Link.c

wchar_t* nl_AimLinkText[] = {
    L"A.I.M. Links",  // The title of the AIM links page
};

// Aim History

// This page displays the history of AIM

wchar_t* nl_AimHistoryText[] = {
    L"A.I.M. Geschiedenis",  // Title

    // Text on the buttons at the bottom of the page

    L"Previous",
    L"Home",
    L"A.I.M. Veteranen",
    L"Next",
};

// Aim Mug Shot Index

// The page in which all the AIM members' portraits are displayed in the order selected by the AIM
// sort page.

wchar_t* nl_AimFiText[] = {
    // displays the way in which the mercs were sorted

    L"Prijs",
    L"Ervaring",
    L"Trefzekerheid",
    L"Medisch",
    L"Explosieven",
    L"Technisch",

    // The title of the page, the above text gets added at the end of this text

    L"A.I.M. Leden Oplopend Gesorteerd op %s",
    L"A.I.M. Leden Aflopend Gesorteerd op %s",

    // Instructions to the players on what to do

    L"Klik Links",
    L"om Huurling te Selecteren",  // 10
    L"Klik Rechts",
    L"voor Sorteeropties",

    // Gets displayed on top of the merc's portrait if they are...

    L"Afwezig",
    L"Overleden",  // 14
    L"Op missie",
};

// AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer
// with AIM

wchar_t* nl_AimAlumniText[] = {
    // Text of the buttons

    L"PAG. 1",
    L"PAG. 2",
    L"PAG. 3",

    L"A.I.M. Veteranen",  // Title of the page

    L"OK",  // Stops displaying information on selected merc
};

// AIM Home Page

wchar_t* nl_AimScreenText[] = {
    // AIM disclaimers

    L"A.I.M. en A.I.M.-logo zijn geregistreerde handelsmerken in de meeste landen.",
    L"Dus denk er niet aan om ons te kopiëren.",
    L"Copyright 1998-1999 A.I.M., Ltd.  All rights reserved.",

    // Text for an advertisement that gets displayed on the AIM page

    L"United Floral Service",
    L"\"We droppen overal\"",  // 10
    L"Doe het goed",
    L"... de eerste keer",
    L"Wapens en zo, als we het niet hebben, dan heb je het ook niet nodig.",
};

// Aim Home Page

wchar_t* nl_AimBottomMenuText[] = {
    // Text for the links at the bottom of all AIM pages
    L"Home", L"Leden", L"Veteranen", L"Regels", L"Geschiedenis", L"Links",
};

// ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

wchar_t* nl_SKI_Text[] = {
    L"HANDELSWAAR OP VOORRAAD",  // Header for the merchandise available
    L"PAG.",                     // The current store inventory page being displayed
    L"TOTALE KOSTEN",            // The total cost of the the items in the Dealer inventory area
    L"TOTALE WAARDE",            // The total value of items player wishes to sell
    L"EVALUEER",     // Button text for dealer to evaluate items the player wants to sell
    L"TRANSACTIE",   // Button text which completes the deal. Makes the transaction.
    L"OK",           // Text for the button which will leave the shopkeeper interface.
    L"REP. KOSTEN",  // The amount the dealer will charge to repair the merc's goods
    L"1 UUR",    // SINGULAR! The text underneath the inventory slot when an item is given to the
                 // dealer to be repaired
    L"%d UREN",  // PLURAL!   The text underneath the inventory slot when an item is given to the
                 // dealer to be repaired
    L"GEREPAREERD",  // Text appearing over an item that has just been repaired by a NPC repairman
                     // dealer
    L"Er is geen ruimte meer.",  // Message box that tells the user there is no more room to put
                                 // there stuff
    L"%d MINUTEN",  // The text underneath the inventory slot when an item is given to the dealer to
                    // be repaired
    L"Drop Item op Grond.",
};

// ShopKeeper Interface
// for the bank machine panels. Referenced here is the acronym ATM, which means Automatic Teller
// Machine

wchar_t* nl_SkiAtmText[] = {
    // Text on buttons on the banking machine, displayed at the bottom of the page
    L"0",    L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
    L"OK",    // Transfer the money
    L"Neem",  // Take money from the player
    L"Geef",  // Give money to the player
    L"Stop",  // Cancel the transfer
    L"Leeg",  // Clear the money display
};

// Shopkeeper Interface
wchar_t* nl_gzSkiAtmText[] = {

    // Text on the bank machine panel that....
    L"Selecteer Type",              // tells the user to select either to give or take from the merc
    L"Voer Bedrag In",              // Enter the amount to transfer
    L"Maak Geld over naar Huurl.",  // Giving money to the merc
    L"Maak Geld over van Huurl.",   // Taking money from the merc
    L"Onvoldoende geld",            // Not enough money to transfer
    L"Saldo",                       // Display the amount of money the player currently has
};

wchar_t* nl_SkiMessageBoxText[] = {
    L"Wil je %s aftrekken van je hoofdrekening om het verschil op te vangen?",
    L"Niet genoeg geld. Je komt %s tekort",
    L"Wil je %s aftrekken van je hoofdrekening om de kosten te dekken?",
    L"Vraag de dealer om de transactie te starten",
    L"Vraag de dealer om de gesel. items te repareren",
    L"Einde conversatie",
    L"Huidige Saldo",
};

// OptionScreen.c

wchar_t* nl_zOptionsText[] = {
    // button Text
    L"Spel Bewaren",
    L"Spel Laden",
    L"Stop",
    L"OK",

    // Text above the slider bars
    L"Effecten",
    L"Spraak",
    L"Muziek",

    // Confirmation pop when the user selects..
    L"Spel verlaten en terugkeren naar hoofdmenu?",

    L"Je hebt of de Spraakoptie nodig of de ondertiteling.",
};

// SaveLoadScreen
wchar_t* nl_zSaveLoadText[] = {
    L"Spel Bewaren",
    L"Spel Laden",
    L"Stop",
    L"Bewaren Gesel.",
    L"Laden Gesel.",

    L"Spel Bewaren voltooid",
    L"FOUT bij bewaren spel!",
    L"Spel laden succesvol",
    L"FOUT bij laden spel!",

    L"De spelversie van het bewaarde spel verschilt van de huidige versie. Waarschijnlijk is het "
    L"veilig om door te gaan. Doorgaan?",
    L"De bewaarde spelen zijn waarschijnlijk ongeldig. Deze verwijderen?",

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

    L"Weet je zeker dat je het spel in slot #%d wil overschrijven?",
    L"Wil je het spel laden van slot #",

    // The first %d is a number that contains the amount of free space on the users hard drive,
    // the second is the recommended amount of free space.
    L"Er is te weinig ruimte op de harde schijf. Er is maar %d MB vrij en Jagged heeft tenminste "
    L"%d MB nodig.",

    L"Bewaren...",  // When saving a game, a message box with this string appears on the screen

    L"Normale Wapens",
    L"Stapels Wapens",
    L"Realistische stijl",
    L"SF stijl",

    L"Moeilijkheid",
};

// MapScreen
wchar_t* nl_zMarksMapScreenText[] = {
    L"Kaartniveau",
    L"Je hebt geen militie. Je moet stadsburgers trainen om een stadsmilitie te krijgen.",
    L"Dagelijks Inkomen",
    L"Huurling heeft levensverzekering",
    L"%s is niet moe.",
    L"%s is bezig en kan niet slapen",
    L"%s is te moe, probeer het later nog eens.",
    L"%s is aan het rijden.",
    L"Team kan niet reizen met een slapende huurling.",

    // stuff for contracts
    L"Je kunt wel het contract betalen, maar je hebt geen geld meer om de levensverzekering van de "
    L"huurling te betalen.",
    L"%s verzekeringspremie kost %s voor %d extra dag(en). Wil je betalen?",
    L"Inventaris Sector",
    L"Huurling heeft medische kosten.",

    // other items
    L"Medici",     // people acting a field medics and bandaging wounded mercs
    L"Patiënten",  // people who are being bandaged by a medic
    L"OK",         // Continue on with the game after autobandage is complete
    L"Stop",       // Stop autobandaging of patients by medics now
    L"Sorry. Optie niet mogelijk in deze demo.",  // informs player this option/button has been
                                                  // disabled in the demo
    L"%s heeft geen reparatie-kit.",
    L"%s heeft geen medische kit.",
    L"Er zijn nu niet genoeg mensen die getraind willen worden.",
    L"%s is vol met milities.",
    L"Huurling heeft eindig contract.",
    L"Contract Huurling is niet verzekerd",
};

wchar_t* nl_pLandMarkInSectorString[] = {
    L"Team %d is heeft iemand ontdekt in sector %s",
};

// confirm the player wants to pay X dollars to build a militia force in town
wchar_t* nl_pMilitiaConfirmStrings[] = {
    L"Een stadsmilitie trainen kost $",  // telling player how much it will cost
    L"Uitgave goedkeuren?",              // asking player if they wish to pay the amount requested
    L"je kunt dit niet betalen.",        // telling the player they can't afford to train this town
    L"Doorgaan met militie trainen %s (%s %d)?",           // continue training this town?
    L"Kosten $",                                           // the cost in dollars to train militia
    L"( J/N )",                                            // abbreviated yes/no
    L"",                                                   // unused
    L"Stadsmilities trainen in %d sectors kost $ %d. %s",  // cost to train sveral sectors at once
    L"Je kunt de $%d niet betalen om de stadsmilitie hier te trainen.",
    L"%s heeft een loyaliteit nodig van %d procent om door te gaan met milities trainen.",
    L"Je kunt de militie in %s niet meer trainen.",
};

// Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom
// of the single merc panel
wchar_t* nl_gzMoneyWithdrawMessageText[] = {
    L"Je kunt maximaal $20.000 in één keer opnemen.",
    L"Weet je zeker dat je %s wil storten op je rekening?",
};

wchar_t* nl_gzCopyrightText[] = {
    L"Copyright (C) 1999 Sir-tech Canada Ltd.  All rights reserved.",
};

// option Text
wchar_t* nl_zOptionsToggleText[] = {
    L"Spraak",
    L"Bevestigingen uit",
    L"Ondertitels",
    L"Wacht bij tekst-dialogen",
    L"Rook Animeren",
    L"Bloedsporen Tonen",
    L"Cursor Niet Bewegen",
    L"Oude Selectiemethode",
    L"Toon reisroute",
    L"Toon Missers",
    L"Bevestiging Real-Time",
    L"Slaap/wakker-berichten",
    L"Metrieke Stelsel",
    L"Huurling Oplichten",
    L"Auto-Cursor naar Huurling",
    L"Auto-Cursor naar Deuren",
    L"Items Oplichten",
    L"Toon Boomtoppen",
    L"Toon Draadmodellen",
    L"Toon 3D Cursor",
};

// This is the help text associated with the above toggles.
wchar_t* nl_zOptionsScreenHelpText[] = {
    // speech
    L"Schakel deze optie IN als je de karakter-dialogen wil horen.",

    // Mute Confirmation
    L"Schakelt verbale bevestigingen v.d. karakters in of uit.",

    // Subtitles
    L"Stelt in of dialoogteksten op het scherm worden getoond.",

    // Key to advance speech
    L"Als ondertitels AANstaan, schakel dit ook in om tijd te hebben de NPC-dialogen te lezen.",

    // Toggle smoke animation
    L"Schakel deze optie uit als rookanimaties het spel vertragen.",

    // Blood n Gore
    L"Schakel deze optie UIT als je bloed aanstootgevend vindt.",

    // Never move my mouse
    L"Schakel deze optie UIT als je wil dat de muis automatisch gepositioneerd wordt bij "
    L"bevestigingsdialogen.",

    // Old selection method
    L"Schakel deze optie IN als je karakters wil selecteren zoals in de vorige JAGGED ALLIANCE "
    L"(methode is tegengesteld dus).",

    // Show movement path
    L"Schakel deze optie IN om bewegingspaden te tonen in real-time (schakel het uit en gebruik "
    L"dan de SHIFT-toets om paden te tonen).",

    // show misses
    L"Schakel IN om het spel de plaats van inslag van je kogels te tonen wanneer je \"mist\".",

    // Real Time Confirmation
    L"Als INGESCHAKELD, een extra \"veiligheids\"-klik is nodig om in real-time te bewegen.",

    // Sleep/Wake notification
    L"INGESCHAKELD zorgt voor berichten of huurlingen op een \"missie\" slapen of werken.",

    // Use the metric system
    L"Wanneer INGESCHAKELD wordt het metrieke stelsel gebruikt, anders het Imperiale stelsel.",

    // Merc Lighted movement
    L"Wanneer INGESCHAKELD, de huurling verlicht de grond tijdens het lopen. Schakel UIT voor "
    L"sneller spelen.",

    // Smart cursor
    L"Wanneer INGESCHAKELD zullen huurlingen dichtbij de cursor automatisch oplichten.",

    // snap cursor to the door
    L"Wanneer INGESCHAKELD zal de cursor dichtbij een deur automatisch boven de deur "
    L"gepositioneerd worden.",

    // glow items
    L"Wanneer INGESCHAKELD lichten |Items altijd op",

    // toggle tree tops
    L"Wanneer INGESCHAKELD worden Boom|toppen getoond.",

    // toggle wireframe
    L"Wanneer INGESCHAKELD worden Draadmodellen (|W) van niet-zichtbare muren getoond.",

    L"Wanneer INGESCHAKELD wordt de cursor in 3D getoond. (|Home)",

};

wchar_t* nl_gzGIOScreenText[] = {
    L"SPEL-INSTELLINGEN",
    L"Speelstijl",
    L"Realistisch",
    L"SF",
    L"Wapenopties",
    L"Extra wapens",
    L"Normaal",
    L"Moeilijksheidsgraad",
    L"Beginneling",
    L"Ervaren",
    L"Expert",
    L"Ok",
    L"Stop",
    L"Extra Moeilijk",
    L"Ongelimiteerde Tijd",
    L"Getimede Beurten",
    L"Niet mogelijk bij Demo",
};

wchar_t* nl_pDeliveryLocationStrings[] = {
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

wchar_t* nl_pSkillAtZeroWarning[] = {
    // This string is used in the IMP character generation.  It is possible to select 0 ability
    // in a skill meaning you can't use it.  This text is confirmation to the player.
    L"Are you sure? A value of zero means NO ability in this skill.",
};

wchar_t* nl_pIMPBeginScreenStrings[] = {
    L"( 8 Karakters Max )",
};

wchar_t* nl_pIMPFinishButtonText[1] = {
    L"Analiseren",
};

wchar_t* nl_pIMPFinishStrings[] = {
    L"Bedankt, %s",  //%s is the name of the merc
};

// the strings for imp voices screen
wchar_t* nl_pIMPVoicesStrings[] = {
    L"Stem",
};

wchar_t* nl_pDepartedMercPortraitStrings[] = {
    L"Gedood tijdens gevecht",
    L"Ontslagen",
    L"Anders",
};

// title for program
wchar_t* nl_pPersTitleText[] = {
    L"Personeelsmanager",
};

// paused game strings
wchar_t* nl_pPausedGameText[] = {
    L"Spel Gepauzeerd",
    L"Doorgaan (|P|a|u|s|e)",
    L"Pauze Spel (|P|a|u|s|e)",
};

wchar_t* nl_pMessageStrings[] = {
    L"Spel verlaten?",
    L"OK",
    L"JA",
    L"NEE",
    L"STOPPEN",
    L"WEER AANNEMEN",
    L"LEUGEN",
    L"Geen beschrijving",  // Save slots that don't have a description.
    L"Spel opgeslagen.",
    L"Spel opgeslagen.",
    L"SnelBewaren",  // The name of the quicksave file (filename, text reference)
    L"SaveGame",     // The name of the normal savegame file, such as SaveGame01, SaveGame02, etc.
    L"sav",          // The 3 character dos extension (represents sav)
    L"..\\SavedGames",  // The name of the directory where games are saved.
    L"Dag",
    L"Huurl",
    L"Leeg Slot",  // An empty save game slot
    L"Demo",       // Demo of JA2
    L"Debug",      // State of development of a project (JA2) that is a debug build
    L"Release",    // Release build for JA2
    L"rpm",   // Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
    L"min",   // Abbreviation for minute.
    L"m",     // One character abbreviation for meter (metric distance measurement unit).
    L"rnds",  // Abbreviation for rounds (# of bullets)
    L"kg",    // Abbreviation for kilogram (metric weight measurement unit)
    L"lb",    // Abbreviation for pounds (Imperial weight measurement unit)
    L"Home",  // Home as in homepage on the internet.
    L"USD",   // Abbreviation to US dollars
    L"nvt",   // Lowercase acronym for not applicable.
    L"Intussen",                          // Meanwhile
    L"%s is gearriveerd in sector %s%s",  // Name/Squad has arrived in sector A9.  Order must not
                                          // change without notifying SirTech
    L"Versie",
    L"Leeg SnelBewaarSlot",
    L"Dit slot is gereserveerd voor SnelBewaren tijdens tactische en kaartoverzichten m.b.v. "
    L"ALT+S.",
    L"Geopend",
    L"Gesloten",
    L"Schijfruimte raakt op. Er is slects %s MB vrij en Jagged Alliance 2 heeft %s MB nodig.",
    L"%s ingehuurd van AIM",
    L"%s heeft %s gevangen.",      //'Merc name' has caught 'item' -- let SirTech know if name comes
                                   // after item.
    L"%s heeft de drug genomen.",  //'Merc name' has taken the drug
    L"%s heeft geen medische kennis",  //'Merc name' has no medical skill.

    // CDRom errors (such as ejecting CD while attempting to read the CD)
    L"De integriteit van het spel is aangetast.",
    L"FOUT: CD-ROM geopend",

    // When firing heavier weapons in close quarters, you may not have enough room to do so.
    L"Er is geen plaats om vanaf hier te schieten.",

    // Can't change stance due to objects in the way...
    L"Kan op dit moment geen standpunt wisselen.",

    // Simple text indications that appear in the game, when the merc can do one of these things.
    L"Drop",
    L"Gooi",
    L"Geef",

    L"%s gegeven aan %s.",  //"Item" passed to "merc".  Please try to keep the item %s before the
                            // merc %s, otherwise, must notify SirTech.
    L"Geen plaats om %s aan %s te geven.",  // pass "item" to "merc".  Same instructions as above.

    // A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
    L" eraan vastgemaakt )",

    // Cheat modes
    L"Vals spel niveau EEN",
    L"Vals spel niveau TWEE",

    // Toggling various stealth modes
    L"Team op sluipmodus.",
    L"Team niet op sluipmodus.",
    L"%s op sluipmodus.",
    L"%s niet op sluipmodus.",

    // Wireframes are shown through buildings to reveal doors and windows that can't otherwise be
    // seen in an isometric engine.  You can toggle this mode freely in the game.
    L"Extra Draadmodellen Aan",
    L"Extra Draadmodellen Uit",

    // These are used in the cheat modes for changing levels in the game.  Going from a basement
    // level to an upper level, etc.
    L"Kan niet naar boven vanaf dit niveau...",
    L"Er zijn geen lagere niveaus...",
    L"Betreden basisniveau %d...",
    L"Verlaten basisniveau...",

    L"'s",  // used in the shop keeper inteface to mark the ownership of the item eg Red's gun
    L"Volgmodus UIT.",
    L"Volgmodus AAN.",
    L"3D Cursor UIT.",
    L"3D Cursor AAN.",
    L"Team %d actief.",
    L"Je kunt %s's dagelijkse salaris van %s niet betalen",  // first %s is the mercs name, the
                                                             // seconds is a string containing the
                                                             // salary
    L"Overslaan",
    L"%s kan niet alleen weggaan.",
    L"Een spel is bewaard onder de naam SaveGame99.sav. Indien nodig, hernoem het naar SaveGame10 "
    L"zodat je het kan aanroepen in het Laden-scherm.",
    L"%s dronk wat %s",
    L"Een pakket is in Drassen gearriveerd.",
    L"%s zou moeten arriveren op het aangewezen punt (sector %s) op dag %d, om ongeveer %s.",
    L"Geschiedenisverslag bijgewerkt.",
#ifdef JA2BETAVERSION
    L"Spel succesvol bewaard in de Einde Beurt Auto Bewaar Slot.",
#endif
};

wchar_t* nl_ItemPickupHelpPopup[] = {
    L"OK", L"Scroll Omhoog", L"Selecteer Alles", L"Scroll Omlaag", L"Stop",
};

wchar_t* nl_pDoctorWarningString[] = {
    L"%s is niet dichtbij genoeg om te worden genezen.",
    L"Je medici waren niet in staat om iedereen te verbinden.",
};

wchar_t* nl_pMilitiaButtonsHelpText[] = {
    L"Raap op(Klik Rechts)/drop(Klik links) Groene Troepen",  // button help text informing player
                                                              // they can pick up or drop militia
                                                              // with this button
    L"Raap op(Klik Rechts)/drop(Klik links) Normale Troepen",
    L"Raap op(Klik Rechts)/drop(Klik links) Veteranentroepen",
    L"Verspreidt beschikbare milities evenredig over alle sectoren",
};

wchar_t* nl_pMapScreenJustStartedHelpText[] = {
    L"Ga naar AIM en huur wat huurlingen in ( *Hint* dat kan bij Laptop )",
    L"Als je klaar bent om naar Arulco te gaan, klik dan op TijdVersneller onder rechts op het "
    L"scherm.",  // to inform the player to hit time compression to get the game underway
};

wchar_t* nl_pAntiHackerString[] = {
    L"Fout. Bestanden missen of zijn beschadigd. Spel wordt beëindigd.",
};

wchar_t* nl_gzLaptopHelpText[] = {
    // Buttons:
    L"Lees E-mail",
    L"Bekijk web-pagina's",
    L"Bekijk bestanden en e-mail attachments",
    L"Lees verslag van gebeurtenissen",
    L"Bekijk team-info",
    L"Bekijk financieel overzicht",
    L"Sluit laptop",

    // Bottom task bar icons (if they exist):
    L"Je hebt nieuwe berichten",
    L"Je hebt nieuwe bestanden",

    // Bookmarks:
    L"Association of International Mercenaries",
    L"Bobby Ray's online weapon mail order",
    L"Institute of Mercenary Profiling",
    L"More Economic Recruiting Center",
    L"McGillicutty's Mortuarium",
    L"United Floral Service",
    L"Verzekeringsagenten voor A.I.M. contracten",
};

wchar_t* nl_gzHelpScreenText[] = {
    L"Verlaat help-scherm",
};

wchar_t* nl_gzNonPersistantPBIText[] = {
    L"Er is een gevecht gaande. Je kan alleen terugtrekken m.b.v. het tactische scherm.",
    L"B|etreedt sector om door te gaan met het huidige gevecht.",
    L"Los huidige gevecht |automatisch op.",
    L"Gevecht kan niet automatisch opgelost worden als je de aanvaller bent.",
    L"Gevecht kan niet automatisch opgelost worden als je in een hinderlaag ligt.",
    L"Gevecht kan niet automatisch opgelost worden als je vecht met beesten in de mijnen.",
    L"Gevecht kan niet automatisch opgelost worden als er vijandige burgers zijn.",
    L"Gevecht kan niet automatisch opgelost worden als er nog bloodcats zijn.",
    L"GEVECHT GAANDE",
    L"je kan je op dit moment niet terugtrekken.",
};

wchar_t* nl_gzMiscString[] = {
    L"Je militie vecht door zonder hulp van je huurlingen...",
    L"Het voertuig heeft geen brandstof meer nodig.",
    L"De brandstoftank is voor %d%% gevuld.",
    L"Het leger van Deidranna heeft totale controle verkregen over %s.",
    L"Je hebt een tankplaats verloren.",
};

wchar_t* nl_gzIntroScreen[] = {
    L"Kan intro video niet vinden",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
wchar_t* nl_pNewNoiseStr[] = {
    L"%s hoort een %s geluid uit %s.",         L"%s hoort een %s geluid van BEWEGING uit %s.",
    L"%s hoort een %s KRAKEND geluid uit %s.", L"%s hoort een %s SPETTEREND geluid uit %s.",
    L"%s hoort een %s INSLAG uit %s.",         L"%s hoort een %s EXPLOSIE naar %s.",
    L"%s hoort een %s SCHREEUW naar %s.",      L"%s hoort een %s INSLAG naar %s.",
    L"%s hoort een %s INSLAG naar %s.",        L"%s hoort een %s VERSPLINTEREN uit %s.",
    L"%s hoort een %s KLAP uit %s.",
};

wchar_t* nl_wMapScreenSortButtonHelpText[] = {
    L"Sorteer op Naam (|F|1)",    L"Sorteer op Taak (|F|2)",       L"Sorteer op Slaapstatus (|F|3)",
    L"Sorteer op locatie (|F|4)", L"Sorteer op Bestemming (|F|5)", L"Sorteer op Vertrektijd (|F|6)",
};

wchar_t* nl_BrokenLinkText[] = {
    L"Fout 404",
    L"Site niet gevonden.",
};

wchar_t* nl_gzBobbyRShipmentText[] = {
    L"Recentelijke ladingen",
    L"Order #",
    L"Aantal Items",
    L"Besteld op",
};

wchar_t* nl_gzCreditNames[] = {
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

wchar_t* nl_gzCreditNameTitle[] = {
    L"Spel Programmeur",                     // Chris Camfield "Game Internals Programmer"
    L"Co-ontwerper/Schrijver",               // Shaun Lyng "Co-designer/Writer"
    L"Strategische Systemen & Programmeur",  // Kris Marnes "Strategic Systems & Editor Programmer"
    L"Producer/Co-ontwerper",                // Ian Currie "Producer/Co-designer"
    L"Co-ontwerper/Kaartontwerp",            // Linda Currie "Co-designer/Map Designer"
    L"Artiest",                              // Eric \"WTF\" Cheng "Artist"
    L"Beta Coördinator, Ondersteuning",      // Lynn Holowka
    L"Artiest Extraordinaire",               // Norman \"NRG\" Olsen
    L"Geluidsgoeroe",                        // George Brooks
    L"Schermontwerp/Artiest",                // Andrew Stacey
    L"Hoofd-Artiest/Animator",               // Scot Loving
    L"Hoofd-Programmeur",                    // Andrew \"Big Cheese Doddle\" Emmons
    L"Programmeur",                          // Dave French
    L"Strategische Systemen & Spelbalans Programmeur",  // Alex Meduna
    L"Portret-Artiest",                                 // Joey \"Joeker\" Whelan",
};

wchar_t* nl_gzCreditNameFunny[] = {
    L"",                                      // Chris Camfield
    L"(leert nog steeds interpunctie)",       // Shaun Lyng
    L"(\"Het is klaar. Ben er mee bezig\")",  // Kris \"The Cow Rape Man\" Marnes
    L"(wordt veel te oud voor dit)",          // Ian Currie
    L"(en werkt aan Wizardry 8)",             // Linda Currie
    L"(moets onder bedreiging ook QA doen)",  // Eric \"WTF\" Cheng
    L"(Verliet ons voor CFSA - dus...)",      // Lynn Holowka
    L"",                                      // Norman \"NRG\" Olsen
    L"",                                      // George Brooks
    L"(Dead Head en jazz liefhebber)",        // Andrew Stacey
    L"(in het echt heet hij Robert)",         // Scot Loving
    L"(de enige verantwoordelijke persoon)",  // Andrew \"Big Cheese Doddle\" Emmons
    L"(kan nu weer motorcrossen)",            // Dave French
    L"(gestolen van Wizardry 8)",             // Alex Meduna
    L"(deed items en schermen-laden ook!)",   // Joey \"Joeker\" Whelan",
};

wchar_t* nl_sRepairsDoneString[] = {
    L"%s is klaar met reparatie van eigen items",
    L"%s is klaar met reparatie van ieders wapens en bepantering",
    L"%s is klaar met reparatie van ieders uitrusting",
    L"%s is klaar met reparatie van ieders vervoerde items",
};

/*wchar_t* nl_zGioDifConfirmText[]=
{
        L"You have chosen NOVICE mode. This setting is appropriate for those new to Jagged Alliance,
those new to strategy games in general, or those wishing shorter battles in the game. Your choice
will affect things throughout the entire course of the game, so choose wisely. Are you sure you want
to play in Novice mode?", L"You have chosen EXPERIENCED mode. This setting is suitable for those
already familiar with Jagged Alliance or similar games. Your choice will affect things throughout
the entire course of the game, so choose wisely. Are you sure you want to play in Experienced
mode?", L"You have chosen EXPERT mode. We warned you. Don't blame us if you get shipped back in a
body bag. Your choice will affect things throughout the entire course of the game, so choose wisely.
Are you sure you want to play in Expert mode?",
};
*/

wchar_t* nl_zGioDifConfirmText[] = {
    L"Je hebt de NOVICE-modus geselecteerd. Deze instelling is geschikt voor diegenen die Jagged "
    L"Alliance voor de eerste keer spelen, voor diegenen die nog niet zo bekend zijn met strategy "
    L"games, of voor diegenen die kortere gevechten in de game willen hebben.",  // Je keuze
                                                                                 // beïnvloedt
                                                                                 // dingen in het
                                                                                 // hele verloop van
                                                                                 // de game, dus
                                                                                 // weet wat je
                                                                                 // doet. Weet je
                                                                                 // zeker dat je in
                                                                                 // de Novice-modus
                                                                                 // wilt spelen?",
    L"Je hebt de EXPERIENCED-modus geselecteerd. Deze instelling is geschikt voor diegenen die al "
    L"bekend zijn met Jagged Alliance of dergelijke games. Je keuze beïnvloedt dingen in het hele "
    L"verloop van de game, dus weet wat je doet. Weet je zeker dat je in de Experienced-modus wilt "
    L"spelen ?",
    L"Je hebt de EXPERT-modus geselecteerd. We hebben je gewaarschuwd. Geef ons niet de schuld als "
    L"je in een kist terugkomt. Je keuze beïnvloedt dingen in het hele verloop van de game, dus "
    L"weet wat je doet. Weet je zeker dat je in de Expert-modus wilt spelen?",
};

wchar_t* nl_gzLateLocalizedString[] = {
    L"%S laadscherm-data niet gevonden...",

    // 1-5
    L"De robot kan de sector niet verlaten als niemand de besturing gebruikt.",

    // This message comes up if you have pending bombs waiting to explode in tactical.
    L"Je kan de tijd niet versnellen, Wacht op het vuurwerk!",

    //'Name' refuses to move.
    L"%s weigert zich te verplaatsen.",

    //%s a merc name
    L"%s heeft niet genoeg energie om standpunt te wisselen.",

    // A message that pops up when a vehicle runs out of gas.
    L"%s heeft geen brandstof en is gestrand in %c%d.",

    // 6-10

    // the following two strings are combined with the pNewNoise[] strings above to report noises
    // heard above or below the merc
    L"boven",
    L"onder",

    // The following strings are used in autoresolve for autobandaging related feedback.
    L"Niemand van je huurlingen heeft medische kennis.",
    L"Er zijn geen medische hulpmiddelen om mensen te verbinden.",
    L"Er waren niet genoeg medische hulpmiddelen om iedereen te verbinden.",
    L"Geen enkele huurling heeft medische hulp nodig.",
    L"Verbindt huurlingen automatisch.",
    L"Al je huurlingen zijn verbonden.",

    // 14
    L"Arulco",

    L"(dak)",

    L"Gezondheid: %d/%d",

    // In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
    //"vs." is the abbreviation of versus.
    L"%d vs. %d",

    L"%s is vol!",  //(ex "The ice cream truck is full")

    L"%s heeft geen eerste hulp nodig, maar échte medische hulp of iets dergelijks.",

    // 20
    // Happens when you get shot in the legs, and you fall down.
    L"%s is geraakt in het been en valt om!",
    // Name can't speak right now.
    L"%s kan nu niet praten.",

    // 22-24 plural versions
    L"%d groene milities zijn gepromoveerd tot veteranenmilitie.",
    L"%d groene milities zijn gepromoveerd tot reguliere militie.",
    L"%d reguliere milities zijn gepromoveerd tot veteranenmilitie.",

    // 25
    L"Schakelaar",

    // 26
    // Name has gone psycho -- when the game forces the player into burstmode (certain unstable
    // characters)
    L"%s wordt gek!",

    // 27-28
    // Messages why a player can't time compress.
    L"Het is nu onveilig om de tijd te versnellen omdat je huurlingen hebt in sector %s.",
    L"Het is nu onveilig om de tijd te versnellen als er huurlingen zijn in de mijnen met beesten.",

    // 29-31 singular versions
    L"1 groene militie is gepromoveerd tot veteranenmilitie.",
    L"1 groene militie is gepromoveerd tot reguliere militie.",
    L"1 reguliere militie is gepromoveerd tot veteranenmilitie.",

    // 32-34
    L"%s zegt helemaal niets.",
    L"Naar oppervlakte reizen?",
    L"(Team %d)",

    // 35
    // Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope,
    // Scope before MP5K)
    L"%s heeft %s's %s gerepareerd",

    // 36
    L"BLOODCAT",

    // 37-38 "Name trips and falls"
    L"%s ups en downs",
    L"Dit item kan vanaf hier niet opgepakt worden.",

    // 39
    L"Geen enkele huurling van je is in staat om te vechten. De militie zal zelf tegen de beesten "
    L"vechten.",

    // 40-43
    //%s is the name of merc.
    L"%s heeft geen medische kits meer!",
    L"%s heeft geen medische kennis om iemand te verzorgen!",
    L"%s heeft geen gereedschapkits meer!",
    L"%s heeft geen technische kennis om iets te repareren!",

    // 44-45
    L"Reparatietijd",
    L"%s kan deze persoon niet zien.",

    // 46-48
    L"%s's pistoolloopverlenger valt eraf!",
    L"Niet meer dan %d militietrainers zijn toegelaten per sector.",
    L"Zeker weten?",

    // 49-50
    L"Tijdversneller",
    L"De tank van het voertuig is nu vol.",

    // 51-52 Fast help text in mapscreen.
    L"Doorgaan met Tijdversnelling (|S|p|a|c|e)",
    L"Stop Tijdversnelling (|E|s|c)",

    // 53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
    L"%s heeft de %s gedeblokkeerd",
    L"%s heeft %s's %s gedeblokkeerd",

    // 55
    L"Kan tijd niet versneller tijdens bekijken van sector inventaris.",

    L"Kan de Jagged Alliance 2 SPEL CD niet vinden. Programma wordt afgesloten.",

    L"Items succesvol gecombineerd.",

    // 58
    // Displayed with the version information when cheats are enabled.
    L"Huidig/Max Voortgang: %d%%/%d%%",

    // 59
    L"John en Mary escorteren?",

    L"Schakelaar geactiveerd.",
};

void UseTextDutch() {
  gzProsLabel = nl_gzProsLabel;
  gzConsLabel = nl_gzConsLabel;
  sRepairsDoneString = nl_sRepairsDoneString;
  AmmoCaliber = nl_AmmoCaliber;
  BobbyRayAmmoCaliber = nl_BobbyRayAmmoCaliber;
  WeaponType = nl_WeaponType;
  Message = nl_Message;
  TeamTurnString = nl_TeamTurnString;
  pAssignMenuStrings = nl_pAssignMenuStrings;
  pTrainingStrings = nl_pTrainingStrings;
  pTrainingMenuStrings = nl_pTrainingMenuStrings;
  pAttributeMenuStrings = nl_pAttributeMenuStrings;
  pVehicleStrings = nl_pVehicleStrings;
  pShortAttributeStrings = nl_pShortAttributeStrings;
  pLongAttributeStrings = nl_pLongAttributeStrings;
  pContractStrings = nl_pContractStrings;
  pAssignmentStrings = nl_pAssignmentStrings;
  pConditionStrings = nl_pConditionStrings;
  pTownNames = nl_pTownNames;
  pPersonnelScreenStrings = nl_pPersonnelScreenStrings;
  pPersonnelTitle = nl_pPersonnelTitle;
  pUpperLeftMapScreenStrings = nl_pUpperLeftMapScreenStrings;
  pTacticalPopupButtonStrings = nl_pTacticalPopupButtonStrings;
  pSquadMenuStrings = nl_pSquadMenuStrings;
  pDoorTrapStrings = nl_pDoorTrapStrings;
  pLongAssignmentStrings = nl_pLongAssignmentStrings;
  pContractExtendStrings = nl_pContractExtendStrings;
  pMapScreenMouseRegionHelpText = nl_pMapScreenMouseRegionHelpText;
  pPersonnelAssignmentStrings = nl_pPersonnelAssignmentStrings;
  pNoiseVolStr = nl_pNoiseVolStr;
  pNoiseTypeStr = nl_pNoiseTypeStr;
  pDirectionStr = nl_pDirectionStr;
  pRemoveMercStrings = nl_pRemoveMercStrings;
  sTimeStrings = nl_sTimeStrings;
  pLandTypeStrings = nl_pLandTypeStrings;
  pGuardMenuStrings = nl_pGuardMenuStrings;
  pOtherGuardMenuStrings = nl_pOtherGuardMenuStrings;
  pInvPanelTitleStrings = nl_pInvPanelTitleStrings;
  pPOWStrings = nl_pPOWStrings;
  pMilitiaString = nl_pMilitiaString;
  pMilitiaButtonString = nl_pMilitiaButtonString;
  pEpcMenuStrings = nl_pEpcMenuStrings;
  pRepairStrings = nl_pRepairStrings;
  sPreStatBuildString = nl_sPreStatBuildString;
  sStatGainStrings = nl_sStatGainStrings;
  pHelicopterEtaStrings = nl_pHelicopterEtaStrings;
  sMapLevelString = nl_sMapLevelString;
  gsLoyalString = nl_gsLoyalString;
  gsUndergroundString = nl_gsUndergroundString;
  gsTimeStrings = nl_gsTimeStrings;
  sFacilitiesStrings = nl_sFacilitiesStrings;
  pMapPopUpInventoryText = nl_pMapPopUpInventoryText;
  pwTownInfoStrings = nl_pwTownInfoStrings;
  pwMineStrings = nl_pwMineStrings;
  pwMiscSectorStrings = nl_pwMiscSectorStrings;
  pMapInventoryErrorString = nl_pMapInventoryErrorString;
  pMapInventoryStrings = nl_pMapInventoryStrings;
  pMapScreenFastHelpTextList = nl_pMapScreenFastHelpTextList;
  pMovementMenuStrings = nl_pMovementMenuStrings;
  pUpdateMercStrings = nl_pUpdateMercStrings;
  pMapScreenBorderButtonHelpText = nl_pMapScreenBorderButtonHelpText;
  pMapScreenBottomFastHelp = nl_pMapScreenBottomFastHelp;
  pMapScreenBottomText = nl_pMapScreenBottomText;
  pMercDeadString = nl_pMercDeadString;
  pSenderNameList = nl_pSenderNameList;
  pTraverseStrings = nl_pTraverseStrings;
  pNewMailStrings = nl_pNewMailStrings;
  pDeleteMailStrings = nl_pDeleteMailStrings;
  pEmailHeaders = nl_pEmailHeaders;
  pEmailTitleText = nl_pEmailTitleText;
  pFinanceTitle = nl_pFinanceTitle;
  pFinanceSummary = nl_pFinanceSummary;
  pFinanceHeaders = nl_pFinanceHeaders;
  pTransactionText = nl_pTransactionText;
  pTransactionAlternateText = nl_pTransactionAlternateText;
  pMoralStrings = nl_pMoralStrings;
  pSkyriderText = nl_pSkyriderText;
  pLeftEquipmentString = nl_pLeftEquipmentString;
  pMapScreenStatusStrings = nl_pMapScreenStatusStrings;
  pMapScreenPrevNextCharButtonHelpText = nl_pMapScreenPrevNextCharButtonHelpText;
  pEtaString = nl_pEtaString;
  pShortVehicleStrings = nl_pShortVehicleStrings;
  pTrashItemText = nl_pTrashItemText;
  pMapErrorString = nl_pMapErrorString;
  pMapPlotStrings = nl_pMapPlotStrings;
  pMiscMapScreenMouseRegionHelpText = nl_pMiscMapScreenMouseRegionHelpText;
  pMercHeLeaveString = nl_pMercHeLeaveString;
  pMercSheLeaveString = nl_pMercSheLeaveString;
  pImpPopUpStrings = nl_pImpPopUpStrings;
  pImpButtonText = nl_pImpButtonText;
  pExtraIMPStrings = nl_pExtraIMPStrings;
  pFilesTitle = nl_pFilesTitle;
  pFilesSenderList = nl_pFilesSenderList;
  pHistoryLocations = nl_pHistoryLocations;
  pHistoryStrings = nl_pHistoryStrings;
  pHistoryHeaders = nl_pHistoryHeaders;
  pHistoryTitle = nl_pHistoryTitle;
  pShowBookmarkString = nl_pShowBookmarkString;
  pWebPagesTitles = nl_pWebPagesTitles;
  pWebTitle = nl_pWebTitle;
  pPersonnelString = nl_pPersonnelString;
  pErrorStrings = nl_pErrorStrings;
  pDownloadString = nl_pDownloadString;
  pBookmarkTitle = nl_pBookmarkTitle;
  pBookMarkStrings = nl_pBookMarkStrings;
  pLaptopIcons = nl_pLaptopIcons;
  sATMText = nl_sATMText;
  gsAtmStartButtonText = nl_gsAtmStartButtonText;
  gsAtmSideButtonText = nl_gsAtmSideButtonText;
  pPersonnelTeamStatsStrings = nl_pPersonnelTeamStatsStrings;
  pPersonnelCurrentTeamStatsStrings = nl_pPersonnelCurrentTeamStatsStrings;
  pPersonelTeamStrings = nl_pPersonelTeamStrings;
  pPersonnelDepartedStateStrings = nl_pPersonnelDepartedStateStrings;
  pMapHortIndex = nl_pMapHortIndex;
  pMapVertIndex = nl_pMapVertIndex;
  pMapDepthIndex = nl_pMapDepthIndex;
  pLaptopTitles = nl_pLaptopTitles;
  pDayStrings = nl_pDayStrings;
  pMercContractOverStrings = nl_pMercContractOverStrings;
  pMilitiaConfirmStrings = nl_pMilitiaConfirmStrings;
  pDeliveryLocationStrings = nl_pDeliveryLocationStrings;
  pSkillAtZeroWarning = nl_pSkillAtZeroWarning;
  pIMPBeginScreenStrings = nl_pIMPBeginScreenStrings;
  pIMPFinishButtonText = nl_pIMPFinishButtonText;
  pIMPFinishStrings = nl_pIMPFinishStrings;
  pIMPVoicesStrings = nl_pIMPVoicesStrings;
  pDepartedMercPortraitStrings = nl_pDepartedMercPortraitStrings;
  pPersTitleText = nl_pPersTitleText;
  pPausedGameText = nl_pPausedGameText;
  zOptionsToggleText = nl_zOptionsToggleText;
  zOptionsScreenHelpText = nl_zOptionsScreenHelpText;
  pDoctorWarningString = nl_pDoctorWarningString;
  pMilitiaButtonsHelpText = nl_pMilitiaButtonsHelpText;
  pMapScreenJustStartedHelpText = nl_pMapScreenJustStartedHelpText;
  pLandMarkInSectorString = nl_pLandMarkInSectorString;
  gzMercSkillText = nl_gzMercSkillText;
  gzNonPersistantPBIText = nl_gzNonPersistantPBIText;
  gzMiscString = nl_gzMiscString;
  wMapScreenSortButtonHelpText = nl_wMapScreenSortButtonHelpText;
  pNewNoiseStr = nl_pNewNoiseStr;
  gzLateLocalizedString = nl_gzLateLocalizedString;
  pAntiHackerString = nl_pAntiHackerString;
  pMessageStrings = nl_pMessageStrings;
  ItemPickupHelpPopup = nl_ItemPickupHelpPopup;
  TacticalStr = nl_TacticalStr;
  LargeTacticalStr = nl_LargeTacticalStr;
  zDialogActions = nl_zDialogActions;
  zDealerStrings = nl_zDealerStrings;
  zTalkMenuStrings = nl_zTalkMenuStrings;
  gzMoneyAmounts = nl_gzMoneyAmounts;
  gMoneyStatsDesc = nl_gMoneyStatsDesc;
  gWeaponStatsDesc = nl_gWeaponStatsDesc;
  sKeyDescriptionStrings = nl_sKeyDescriptionStrings;
  zHealthStr = nl_zHealthStr;
  zVehicleName = nl_zVehicleName;
  pExitingSectorHelpText = nl_pExitingSectorHelpText;
  InsContractText = nl_InsContractText;
  InsInfoText = nl_InsInfoText;
  MercAccountText = nl_MercAccountText;
  MercInfo = nl_MercInfo;
  MercNoAccountText = nl_MercNoAccountText;
  MercHomePageText = nl_MercHomePageText;
  sFuneralString = nl_sFuneralString;
  sFloristText = nl_sFloristText;
  sOrderFormText = nl_sOrderFormText;
  sFloristGalleryText = nl_sFloristGalleryText;
  sFloristCards = nl_sFloristCards;
  BobbyROrderFormText = nl_BobbyROrderFormText;
  BobbyRText = nl_BobbyRText;
  BobbyRaysFrontText = nl_BobbyRaysFrontText;
  AimSortText = nl_AimSortText;
  AimPolicyText = nl_AimPolicyText;
  AimMemberText = nl_AimMemberText;
  CharacterInfo = nl_CharacterInfo;
  VideoConfercingText = nl_VideoConfercingText;
  AimPopUpText = nl_AimPopUpText;
  AimLinkText = nl_AimLinkText;
  AimHistoryText = nl_AimHistoryText;
  AimFiText = nl_AimFiText;
  AimAlumniText = nl_AimAlumniText;
  AimScreenText = nl_AimScreenText;
  AimBottomMenuText = nl_AimBottomMenuText;
  zMarksMapScreenText = nl_zMarksMapScreenText;
  gpStrategicString = nl_gpStrategicString;
  gpGameClockString = nl_gpGameClockString;
  SKI_Text = nl_SKI_Text;
  SkiAtmText = nl_SkiAtmText;
  gzSkiAtmText = nl_gzSkiAtmText;
  SkiMessageBoxText = nl_SkiMessageBoxText;
  zSaveLoadText = nl_zSaveLoadText;
  zOptionsText = nl_zOptionsText;
  gzGIOScreenText = nl_gzGIOScreenText;
  gzHelpScreenText = nl_gzHelpScreenText;
  gzLaptopHelpText = nl_gzLaptopHelpText;
  gzMoneyWithdrawMessageText = nl_gzMoneyWithdrawMessageText;
  gzCopyrightText = nl_gzCopyrightText;
  BrokenLinkText = nl_BrokenLinkText;
  gzBobbyRShipmentText = nl_gzBobbyRShipmentText;
  zGioDifConfirmText = nl_zGioDifConfirmText;
  gzCreditNames = nl_gzCreditNames;
  gzCreditNameTitle = nl_gzCreditNameTitle;
  gzCreditNameFunny = nl_gzCreditNameFunny;
  pUpdatePanelButtons = nl_pUpdatePanelButtons;
  pBullseyeStrings = nl_pBullseyeStrings;
  pContractButtonString = nl_pContractButtonString;
}
