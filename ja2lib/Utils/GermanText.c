#include "LanguageDefines.h"
#ifdef GERMAN

#include "Utils/Text.h"
#endif

#ifdef GERMAN

/*
******************************************************************************************************
**
IMPORTANT TRANSLATION NOTES
**
******************************************************************************************************

GENERAL TOPWARE INSTRUCTIONS
- Always be aware that German strings should be of equal or shorter length than the English
equivalent. I know that this is difficult to do on many occasions due to the nature of the German
language when compared to English. By doing so, this will greatly reduce the amount of work on both
sides. In most cases (but not all), JA2 interfaces were designed with just enough space to fit the
English word. The general rule is if the string is very short (less than 10 characters), then it's
short because of interface limitations. On the other hand, full sentences commonly have little
limitations for length. Strings in between are a little dicey.
- Never translate a string to appear on multiple lines. All strings L"This is a really long
string...", must fit on a single line no matter how long the string is. All strings start with L"
and end with ",
- Never remove any extra spaces in strings. In addition, all strings containing multiple sentences
only have one space after a period, which is different than standard typing convention. Never modify
sections of strings contain combinations of % characters. These are special format characters and
are always used in conjunction with other characters. For example, %s means string, and is commonly
used for names, locations, items, etc. %d is used for numbers. %c%d is a character and a number
(such as A9).
        %% is how a single % character is built. There are countless types, but strings containing
these special characters are usually commented to explain what they mean. If it isn't commented,
then if you can't figure out the context, then feel free to ask SirTech.
- Comments are always started with // Anything following these two characters on the same line are
        considered to be comments. Do not translate comments. Comments are always applied to the
following string(s) on the next line(s), unless the comment is on the same line as a string.
- All new comments made by SirTech will use "//@@@ comment" (without the quotes) notation. By
searching for @@@ everytime you recieve a new version, it will simplify your task and identify
special instructions. Commonly, these types of comments will be used to ask you to abbreviate a
string. Please leave the comments intact, and SirTech will remove them once the translation for that
particular area is resolved.
- If you have a problem or question with translating certain strings, please use "//!!! comment"
        (without the quotes). The syntax is important, and should be identical to the comments used
with @@@ symbols. SirTech will search for !!! to look for Topware problems and questions. This is a
more efficient method than detailing questions in email, so try to do this whenever possible.



FAST HELP TEXT -- Explains how the syntax of fast help text works.
**************

1) BOLDED LETTERS
        The popup help text system supports special characters to specify the hot key(s) for a
button. Anytime you see a '|' symbol within the help text string, that means the following key is
assigned to activate the action which is usually a button.

        EX: L"|Map Screen"

        This means the 'M' is the hotkey. In the game, when somebody hits the 'M' key, it activates
that button. When translating the text to another language, it is best to attempt to choose a word
that uses 'M'. If you can't always find a match, then the best thing to do is append the 'M' at the
end of the string in this format:

        EX: L"Ecran De Carte (|M)" (this is the French translation)

        Other examples are used multiple times, like the Esc key or "|E|s|c" or Space ->
(|S|p|a|c|e)

2) NEWLINE
        Any place you see a \n within the string, you are looking at another string that is part of
the fast help text system. \n notation doesn't need to be precisely placed within that string, but
whereever you wish to start a new line.

        EX: L"Clears all the mercs' positions,\nand allows you to re-enter them manually."

        Would appear as:

                                Clears all the mercs' positions,
                                and allows you to re-enter them manually.

        NOTE: It is important that you don't pad the characters adjacent to the \n with spaces. If
we did this in the above example, we would see

        WRONG WAY -- spaces before and after the \n
        EX: L"Clears all the mercs' positions, \n and allows you to re-enter them manually."

        Would appear as: (the second line is moved in a character)

                                Clears all the mercs' positions,
                                 and allows you to re-enter them manually.


@@@ NOTATION
************

        Throughout the text files, you'll find an assortment of comments. Comments are used to
describe the text to make translation easier, but comments don't need to be translated. A good thing
is to search for
        "@@@" after receiving new version of the text file, and address the special notes in this
manner.

!!! NOTATION
************

        As described above, the "!!!" notation should be used by Topware to ask questions and
address problems as SirTech uses the "@@@" notation.

*/

UINT16 ItemNames[MAXITEMS][80] = {
    L"",
};

UINT16 ShortItemNames[MAXITEMS][80] = {
    L"",
};

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
UINT16 AmmoCaliber[][20] = {
    L"0",         L".38 Kal", L"9mm",    L".45 Kal", L".357 Kal",
    L"12 Kal",    L"CAWS",    L"5.45mm", L"5.56mm",  L"7.62mm NATO",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monster", L"Rakete",
    L"",          L"",
};

// This BobbyRayAmmoCaliber is virtually the same as AmmoCaliber however the bobby version doesnt
// have as much room for the words.
//
// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
UINT16 BobbyRayAmmoCaliber[][20] = {
    L"0",         L".38 Kal", L"9mm",    L".45 Kal", L".357 Kal",
    L"12 Kal",    L"CAWS",    L"5.45mm", L"5.56mm",  L"7.62mm N.",
    L"7.62mm WP", L"4.7mm",   L"5.7mm",  L"Monster", L"Rakete",
    L"",  // dart
};

UINT16 WeaponType[][30] = {
    L"Andere",           L"Pistole",
    L"Maschinenpistole", L"Schwere Maschinenpistole",
    L"Gewehr",           L"Scharfsch??tzengewehr",
    L"Sturmgewehr",      L"Leichtes Maschinengewehr",
    L"Schrotflinte",
};

UINT16 TeamTurnString[][STRING_LENGTH] = {
    L"Spielzug Spieler", L"Spielzug Gegner",     L"Spielzug Monster",
    L"Spielzug Miliz",   L"Spielzug Zivilisten",
    // planning turn
};

UINT16 Message[][STRING_LENGTH] = {
    L"",

    // In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

    L"%s am Kopf getroffen, verliert einen Weisheitspunkt!",
    L"%s an der Schulter getroffen, verliert Geschicklichkeitspunkt!",
    L"%s an der Brust getroffen, verliert einen Kraftpunkt!",
    L"%s an den Beinen getroffen, verliert einen Beweglichkeitspunkt!",
    L"%s am Kopf getroffen, verliert %d Weisheitspunkte!",
    L"%s an der Schulter getroffen, verliert %d Geschicklichkeitspunkte!",
    L"%s an der Brust getroffen, verliert %d Kraftpunkte!",
    L"%s an den Beinen getroffen, verliert %d Beweglichkeitspunkte!",
    L"Unterbrechung!",

    // The first %s is a merc's name, the second is a string from pNoiseVolStr,
    // the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

    L"",  // obsolete
    L"Verst??rkung ist angekommen!",

    // In the following four lines, all %s's are merc names

    L"%s l??dt nach.",
    L"%s hat nicht genug Action-Punkte!",
    L"%s leistet Erste Hilfe. (R??ckg??ngig mit beliebiger Taste.)",
    L"%s und %s leisten Erste Hilfe. (R??ckg??ngig mit beliebiger Taste.)",
    // the following 17 strings are used to create lists of gun advantages and disadvantages
    // (separated by commas)
    L"zuverl??ssig",
    L"unzuverl??ssig",
    L"Reparatur leicht",
    L"Reparatur schwer",
    L"gro??e Durchschlagskraft",
    L"kleine Durchschlagskraft",
    L"feuert schnell",
    L"feuert langsam",
    L"gro??e Reichweite",
    L"kurze Reichweite",
    L"leicht",
    L"schwer",
    L"klein",
    L"schneller Feuersto??",
    L"kein Feuersto??",
    L"gro??es Magazin",
    L"kleines Magazin",

    // In the following two lines, all %s's are merc names

    L"%ss Tarnung hat sich abgenutzt.",
    L"%ss Tarnung ist weggewaschen.",

    // The first %s is a merc name and the second %s is an item name

    L"Zweite Waffe hat keine Ammo!",
    L"%s hat %s gestohlen.",

    // The %s is a merc name

    L"%ss Waffe kann keinen Feuersto?? abgeben.",

    L"Sie haben schon eines davon angebracht.",
    L"Gegenst??nde zusammenf??gen?",

    // Both %s's are item names

    L"Sie k??nnen %s mit %s nicht zusammenf??gen",

    L"Keine",
    L"Ammo entfernen",
    L"Modifikationen",

    // You cannot use "item(s)" and your "other item" at the same time.
    // Ex: You cannot use sun goggles and you gas mask at the same time.
    L"Sie k??nnen %s nicht zusammen mit %s benutzen.",  //

    L"Der Gegenstand in Ihrem Cursor kann mit anderen Gegenst??nden verbunden werden, indem Sie ihn "
    L"in einem der vier Slots plazieren",
    L"Der Gegenstand in Ihrem Cursor kann mit anderen Gegenst??nden verbunden werden, indem Sie ihn "
    L"in einem der vier Attachment-Slots plazieren. (Aber in diesem Fall sind die Gegenst??nde "
    L"nicht kompatibel.)",
    L"Es sind noch Feinde im Sektor!",
    L"Geben Sie %s %s",
    L"%s am Kopf getroffen!",
    L"Kampf abbrechen?",
    L"Die Modifikation ist permanent. Weitermachen?",
    L"%s f??hlt sich frischer!",
    L"%s ist auf Murmeln ausgerutscht!",
    L"%s konnte %s nicht greifen!",
    L"%s hat %s repariert",
    L"Unterbrechung f??r ",
    L"Ergeben?",
    L"Diese Person will keine Hilfe.",
    L"Lieber NICHT!",
    L"Wenn Sie zu Skyriders Heli wollen, m??ssen Sie S??ldner einem FAHRZEUG/HELIKOPTER ZUWEISEN.",
    L"%s hat nur Zeit, EINE Waffe zu laden",
    L"Spielzug Bloodcats",
};

STR16 pTownNames[] = {
    L"",         L"Omerta", L"Drassen", L"Alma",   L"Grumm",  L"Tixa",     L"Cambria",
    L"San Mona", L"Estoni", L"Orta",    L"Balime", L"Meduna", L"Chitzena",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per
// second, etc. min is an abbreviation for minutes
STR16 sTimeStrings[] = {
    L"Pause", L"Normal", L"5 Min", L"30 Min", L"60 Min", L"6 Std",
};

// Assignment Strings: what assignment does the merc have right now? For example, are they on a
// squad, training, administering medical aid (doctor) or training a town. All are abbreviated. 8
// letters is the longest it can be.
STR16 pAssignmentStrings[] = {
    L"Trupp 1",  L"Trupp 2",  L"Trupp 3",  L"Trupp 4",  L"Trupp 5",  L"Trupp 6",  L"Trupp 7",
    L"Trupp 8",  L"Trupp 9",  L"Trupp 10", L"Trupp 11", L"Trupp 12", L"Trupp 13", L"Trupp 14",
    L"Trupp 15", L"Trupp 16", L"Trupp 17", L"Trupp 18", L"Trupp 19", L"Trupp 20",
    L"Dienst",    // on active duty
    L"Doktor",    // administering medical aid
    L"Patient",   // getting medical aid
    L"Fahrzeug",  // in a vehicle
    L"Transit",   // in transit - abbreviated form
    L"Repar.",    // repairing
    L"??ben",      // training themselves
    L"Miliz",     // training a town to revolt
    L"Trainer",   // training a teammate
    L"Rekrut",    // being trained by someone else
    L"Tot",       // dead
    L"Unf??hig",   // abbreviation for incapacitated
    L"Gefangen",  // Prisoner of war - captured
    L"Hospital",  // patient in a hospital
    L"Leer",      // Vehicle is empty
};

STR16 pMilitiaString[] = {
    L"Miliz",         // the title of the militia box
    L"Ohne Aufgabe",  // the number of unassigned militia troops
    L"Mit Feinden im Sektor k??nnen Sie keine Miliz einsetzen!",
};

STR16 pMilitiaButtonString[] = {
    L"Autom.",  // auto place the militia troops for the player
    L"Fertig",  // done placing militia troops
};

STR16 pConditionStrings[] = {
    L"Sehr gut",    // the state of a soldier .. excellent health
    L"Gut",         // good health
    L"Mittel",      // fair health
    L"Verwundet",   // wounded health
    L"Ersch??pft",   // tired
    L"Verblutend",  // bleeding to death
    L"Bewu??tlos",   // knocked out
    L"Stirbt",      // near death
    L"Tot",         // dead
};

STR16 pEpcMenuStrings[] = {
    L"Dienst",     // set merc on active duty
    L"Patient",    // set as a patient to receive medical aid
    L"Fahrzeug",   // tell merc to enter vehicle
    L"Unbewacht",  // let the escorted character go off on their own
    L"Abbrechen",  // close this menu
};

// look at pAssignmentString above for comments
STR16 pPersonnelAssignmentStrings[] = {
    L"Trupp 1",  L"Trupp 2",  L"Trupp 3",  L"Trupp 4",  L"Trupp 5",  L"Trupp 6",  L"Trupp 7",
    L"Trupp 8",  L"Trupp 9",  L"Trupp 10", L"Trupp 11", L"Trupp 12", L"Trupp 13", L"Trupp 14",
    L"Trupp 15", L"Trupp 16", L"Trupp 17", L"Trupp 18", L"Trupp 19", L"Trupp 20", L"Dienst",
    L"Doktor",   L"Patient",  L"Fahrzeug", L"Transit",  L"Repar.",   L"??ben",     L"Miliz",
    L"Trainer",  L"Rekrut",   L"Tot",      L"Unf??hig",  L"Gefangen", L"Hospital",
    L"Leer",  // Vehicle is empty
};

// refer to above for comments
STR16 pLongAssignmentStrings[] = {
    L"Trupp 1",  L"Trupp 2",  L"Trupp 3",  L"Trupp 4",  L"Trupp 5",  L"Trupp 6",  L"Trupp 7",
    L"Trupp 8",  L"Trupp 9",  L"Trupp 10", L"Trupp 11", L"Trupp 12", L"Trupp 13", L"Trupp 14",
    L"Trupp 15", L"Trupp 16", L"Trupp 17", L"Trupp 18", L"Trupp 19", L"Trupp 20", L"Dienst",
    L"Doktor",   L"Patient",  L"Fahrzeug", L"Transit",  L"Repar.",   L"??ben",     L"Miliz",
    L"Trainer",  L"Rekrut",   L"Tot",      L"Unf??hig",  L"Gefangen",
    L"Hospital",  // patient in a hospital
    L"Leer",      // Vehicle is empty
};

// the contract options
STR16 pContractStrings[] = {
    L"Vertragsoptionen:",
    L"",                      // a blank line, required
    L"Einen Tag anbieten",    // offer merc a one day contract extension
    L"Eine Woche anbieten",   // 1 week
    L"Zwei Wochen anbieten",  // 2 week
    L"Entlassen",             // end merc's contract (used to be "Terminate")
    L"Abbrechen",             // stop showing this menu
};

STR16 pPOWStrings[] = {
    L"gefangen",  // an acronym for Prisoner of War
    L"??",
};

STR16 pLongAttributeStrings[] = {
    L"KRAFT",  // The merc's strength attribute. Others below represent the other attributes.
    L"GESCHICKLICHKEIT",
    L"BEWEGLICHKEIT",
    L"WEISHEIT",
    L"TREFFSICHERHEIT",
    L"MEDIZIN",
    L"TECHNIK",
    L"F??HRUNGSQUALIT??T",
    L"SPRENGSTOFFE",
    L"ERFAHRUNGSSTUFE",
};

STR16 pInvPanelTitleStrings[] = {
    L"R??stung",  // the armor rating of the merc
    L"Gewicht",  // the weight the merc is carrying
    L"Tarnung",  // the merc's camouflage rating
};

STR16 pShortAttributeStrings[] = {
    L"Bew",  // the abbreviated version of : agility
    L"Ges",  // dexterity
    L"Krf",  // strength
    L"Fhr",  // leadership
    L"Wsh",  // wisdom
    L"Erf",  // experience level
    L"Trf",  // marksmanship skill
    L"Spr",  // explosive skill
    L"Tec",  // mechanical skill
    L"Med",  // medical skill
};

STR16 pUpperLeftMapScreenStrings[] = {
    L"Aufgabe",   // the mercs current assignment
    L"Vertrag",   // the contract info about the merc
    L"Gesundh.",  // the health level of the current merc
    L"Moral",     // the morale of the current merc
    L"Zust.",     // the condition of the current vehicle
    L"Tank",      // the fuel level of the current vehicle
};

STR16 pTrainingStrings[] = {
    L"??ben",     // tell merc to train self
    L"Miliz",    // tell merc to train town //
    L"Trainer",  // tell merc to act as trainer
    L"Rekrut",   // tell merc to be train by other
};

STR16 pGuardMenuStrings[] = {
    L"Schu??rate:",              // the allowable rate of fire for a merc who is guarding
    L" Aggressiv feuern",       // the merc can be aggressive in their choice of fire rates
    L" Ammo sparen",            // conserve ammo
    L" Nur bei Bedarf feuern",  // fire only when the merc needs to
    L"Andere Optionen:",        // other options available to merc
    L" R??ckzug m??glich",        // merc can retreat
    L" Deckung m??glich",        // merc is allowed to seek cover
    L" Kann Kameraden helfen",  // merc can assist teammates
    L"Fertig",                  // done with this menu
    L"Abbruch",                 // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by
// the player
STR16 pOtherGuardMenuStrings[] = {
    L"Schu??rate:",
    L" *Aggressiv feuern*",
    L" *Ammo sparen*",
    L" *Nur bei Bedarf feuern*",
    L"Andere Optionen:",
    L" *R??ckzug m??glich*",
    L" *Deckung m??glich*",
    L" *Kann Kameraden helfen*",
    L"Fertig",
    L"Abbruch",
};

STR16 pAssignMenuStrings[] = {
    L"Dienst",     // merc is on active duty
    L"Doktor",     // the merc is acting as a doctor
    L"Patient",    // the merc is receiving medical attention
    L"Fahrzeug",   // the merc is in a vehicle
    L"Repar.",     // the merc is repairing items
    L"Training",   // the merc is training
    L"Abbrechen",  // cancel this menu
};

STR16 pRemoveMercStrings[] = {
    L"S??ldner entfernen",  // remove dead merc from current team
    L"Abbrechen",
};

STR16 pAttributeMenuStrings[] = {
    L"Kraft",   L"Geschicklichkeit", L"Beweglichkeit",    L"Gesundheit",   L"Treffsicherheit",
    L"Medizin", L"Technik",          L"F??hrungsqualit??t", L"Sprengstoffe", L"Abbrechen",
};

STR16 pTrainingMenuStrings[] = {
    L"??ben",       // train yourself
    L"Miliz",      // train the town
    L"Trainer",    // train your teammates
    L"Rekrut",     // be trained by an instructor
    L"Abbrechen",  // cancel this menu
};

STR16 pSquadMenuStrings[] = {
    L"Trupp  1", L"Trupp  2", L"Trupp  3", L"Trupp  4", L"Trupp  5", L"Trupp  6", L"Trupp  7",
    L"Trupp  8", L"Trupp  9", L"Trupp 10", L"Trupp 11", L"Trupp 12", L"Trupp 13", L"Trupp 14",
    L"Trupp 15", L"Trupp 16", L"Trupp 17", L"Trupp 18", L"Trupp 19", L"Trupp 20", L"Abbrechen",
};

STR16 pPersonnelTitle[] = {
    L"Personal",  // the title for the personnel screen/program application
};

STR16 pPersonnelScreenStrings[] = {
    L"Gesundheit: ",  // health of merc
    L"Beweglichkeit: ",
    L"Geschicklichkeit: ",
    L"Kraft: ",
    L"F??hrungsqualit??t: ",
    L"Weisheit: ",
    L"Erf. Stufe: ",  // experience level
    L"Treffsicherheit: ",
    L"Technik: ",
    L"Sprengstoffe: ",
    L"Medizin: ",
    L"Med. Versorgung: ",     // amount of medical deposit put down on the merc
    L"Laufzeit: ",            // time remaining on current contract
    L"Get??tet: ",             // number of kills by merc
    L"Mithilfe: ",            // number of assists on kills by merc
    L"Tgl. Kosten:",          // daily cost of merc
    L"Gesamtkosten:",         // total cost of merc
    L"Vertrag:",              // cost of current contract
    L"Diensttage:",           // total service rendered by merc
    L"Schulden:",             // amount left on MERC merc to be paid
    L"Prozentzahl Treffer:",  // percentage of shots that hit target
    L"Eins??tze:",             // number of battles fought
    L"Verwundungen:",         // number of times merc has been wounded
    L"F??higkeiten:",
    L"Keine F??higkeiten:",
};

// These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
STR16 gzMercSkillText[] = {
    L"Keine F??higkeiten",
    L"Schl??sser knacken",
    L"Nahkampf",
    L"Elektronik",
    L"Nachteinsatz",
    L"Werfen",
    L"Lehren",
    L"Schwere Waffen",
    L"Autom. Waffen",
    L"Schleichen",
    L"Geschickt",
    L"Dieb",
    L"Kampfsport",
    L"Messer",
    L"Dach-Treffer-Bonus",
    L"Getarnt",
    L"(Experte)",
};

// This is pop up help text for the options that are available to the merc
STR16 pTacticalPopupButtonStrings[] = {
    L"|Stehen/Gehen",
    L"Kauern/Kauernd bewegen (|C)",
    L"Stehen/|Rennen",
    L"Hinlegen/Kriechen (|P)",
    L"Schauen (|L)",
    L"Action",
    L"Reden",
    L"Untersuchen (|C|t|r|l)",

    // Pop up door menu
    L"Manuell ??ffnen",
    L"Auf Fallen untersuchen",
    L"Dietrich",
    L"Mit Gewalt ??ffnen",
    L"Falle entsch??rfen",
    L"Abschlie??en",
    L"Aufschlie??en",
    L"Sprengstoff an T??r benutzen",
    L"Brecheisen benutzen",
    L"R??ckg??ngig (|E|s|c)",
    L"Schlie??en",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.
STR16 pDoorTrapStrings[] = {
    L"keine Falle",           L"eine Sprengstoffalle",         L"eine elektrische Falle",
    L"eine Falle mit Sirene", L"eine Falle mit stummem Alarm",
};

// Contract Extension. These are used for the contract extension with AIM mercenaries.
STR16 pContractExtendStrings[] = {
    L"1 Tag", L"1 Woche",
    L"2 Wochen",  //
};

// On the map screen, there are four columns. This text is popup help text that identifies the
// individual columns.
STR16 pMapScreenMouseRegionHelpText[] = {
    L"Charakter ausw??hlen",
    L"S??ldner einteilen",
    L"Marschroute",

    // The new 'c' key activates this option. Either reword this string to include a 'c' in it, or
    // leave as is.
    L"Vertrag f??r S??ldner (|c)",

    L"S??ldner entfernen",
    L"Schlafen",
};

// volumes of noises
STR16 pNoiseVolStr[] = {
    L"LEISE",
    L"DEUTLICH",
    L"LAUT",
    L"SEHR LAUT",
};

// types of noises
STR16 pNoiseTypeStr[] = {
    L"EIN UNBEKANNTES GER??USCH", L"EINE BEWEGUNG",   L"EIN KNARREN",    L"EIN KLATSCHEN",
    L"EINEN AUFSCHLAG",          L"EINEN SCHUSS",    L"EINE EXPLOSION", L"EINEN SCHREI",
    L"EINEN AUFSCHLAG",          L"EINEN AUFSCHLAG", L"EIN ZERBRECHEN", L"EIN ZERSCHMETTERN",
};

// Directions that are used throughout the code for identification.
STR16 pDirectionStr[] = {
    L"NORDOSTEN", L"OSTEN",  L"S??DOSTEN",   L"S??DEN",
    L"S??DWESTEN", L"WESTEN", L"NORDWESTEN", L"NORDEN",
};

// These are the different terrain types.
STR16 pLandTypeStrings[] = {
    L"Stadt", L"Stra??e", L"Ebene", L"W??ste", L"Lichter Wald", L"Dichter Wald", L"Sumpf",
    L"See/Ozean", L"H??gel", L"Unpassierbar",
    L"Flu??",  // river from north to south
    L"Flu??",  // river from east to west
    L"Fremdes Land",
    // NONE of the following are used for directional travel, just for the sector description.
    L"Tropen", L"Farmland", L"Ebene, Stra??e", L"Wald, Stra??e", L"Farm, Stra??e", L"Tropen, Stra??e",
    L"Wald, Stra??e", L"K??ste", L"Berge, Stra??e", L"K??ste, Stra??e", L"W??ste, Stra??e",
    L"Sumpf, Stra??e", L"Wald, Raketen", L"W??ste, Raketen", L"Tropen, Raketen", L"Meduna, Raketen",

    // These are descriptions for special sectors
    L"Cambria Hospital", L"Drassen Flugplatz", L"Meduna Flugplatz", L"Raketen",
    L"Rebellenlager",  // The rebel base underground in sector A10
    L"Tixa, Keller",   // The basement of the Tixa Prison (J9)
    L"Monsterh??hle",   // Any mine sector with creatures in it
    L"Orta, Keller",   // The basement of Orta (K4)
    L"Tunnel",         // The tunnel access from the maze garden in Meduna
                       // leading to the secret shelter underneath the palace
    L"Bunker",         // The shelter underneath the queen's palace
    L"",               // Unused
};

STR16 gpStrategicString[] = {
    //     The first %s can either be bloodcats or enemies.
    L"",                                                                                // Unused
    L"%s wurden entdeckt in Sektor %c%d und ein weiterer Trupp wird gleich ankommen.",  // STR_DETECTED_SINGULAR
    L"%s wurden entdeckt in Sektor %c%d und weitere Trupps werden gleich ankommen.",  // STR_DETECTED_PLURAL
    L"Gleichzeitige Ankunft koordinieren?",  // STR_COORDINATE

    // Dialog strings for enemies.

    L"Feind bietet die Chance zum Aufgeben an.",                  // STR_ENEMY_SURRENDER_OFFER
    L"Feind hat restliche bewu??tlose S??ldner gefangengenommen.",  // STR_ENEMY_CAPTURED

    // The text that goes on the autoresolve buttons

    L"R??ckzug",  // The retreat button				//STR_AR_RETREAT_BUTTON
    L"Fertig",   // The done button				//STR_AR_DONE_BUTTON

    // The headers are for the autoresolve type (MUST BE UPPERCASE)

    L"VERTEIDIGUNG",  // STR_AR_DEFEND_HEADER
    L"ANGRIFF",       // STR_AR_ATTACK_HEADER
    L"BEGEGNUNG",     // STR_AR_ENCOUNTER_HEADER
    L"Sektor",        // The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

    // The battle ending conditions

    L"SIEG!",              // STR_AR_OVER_VICTORY
    L"NIEDERLAGE!",        // STR_AR_OVER_DEFEAT
    L"AUFGEGEBEN!",        // STR_AR_OVER_SURRENDERED
    L"GEFANGENGENOMMEN!",  // STR_AR_OVER_CAPTURED
    L"ZUR??CKGEZOGEN!",     // STR_AR_OVER_RETREATED

    // These are the labels for the different types of enemies we fight in autoresolve.

    L"Miliz",    // STR_AR_MILITIA_NAME,
    L"Elite",    // STR_AR_ELITE_NAME,
    L"Trupp",    // STR_AR_TROOP_NAME,
    L"Verwal",   // STR_AR_ADMINISTRATOR_NAME,
    L"Monster",  // STR_AR_CREATURE_NAME,

    // Label for the length of time the battle took

    L"Zeit verstrichen",  // STR_AR_TIME_ELAPSED,

    // Labels for status of merc if retreating. (UPPERCASE)

    L"HAT SICH ZUR??CKGEZOGEN",  // STR_AR_MERC_RETREATED,
    L"ZIEHT SICH ZUR??CK",       // STR_AR_MERC_RETREATING,
    L"R??CKZUG",                 // STR_AR_MERC_RETREAT,

    // PRE BATTLE INTERFACE STRINGS
    // Goes on the three buttons in the prebattle interface. The Auto resolve button represents
    // a system that automatically resolves the combat for the player without having to do anything.
    // These strings must be short (two lines -- 6-8 chars per line)

    L"PC Kampf",        // STR_PB_AUTORESOLVE_BTN,
    L"Gehe zu Sektor",  // STR_PB_GOTOSECTOR_BTN,
    L"R??ckzug",         // STR_PB_RETREATMERCS_BTN,

    // The different headers(titles) for the prebattle interface.
    L"FEINDBEGEGNUNG",
    L"FEINDLICHE INVASION",
    L"FEINDLICHER HINTERHALT",
    L"FEINDLICHEN SEKTOR BETRETEN",
    L"MONSTERANGRIFF",
    L"BLOODCAT-HINTERHALT",
    L"BLOODCAT-H??HLE BETRETEN",

    // Various single words for direct translation. The Civilians represent the civilian
    // militia occupying the sector being attacked. Limited to 9-10 chars

    L"Ort",
    L"Feinde",
    L"S??ldner",
    L"Miliz",
    L"Monster",
    L"Bloodcats",
    L"Sektor",
    L"Keine",  // If there are no uninvolved mercs in this fight.
    L"n.a.",   // Acronym of Not Applicable
    L"T",      // One letter abbreviation of day
    L"h",      // One letter abbreviation of hour

    // TACTICAL PLACEMENT USER INTERFACE STRINGS
    // The four buttons

    L"R??umen",
    L"Verteilen",
    L"Gruppieren",
    L"Fertig",

    // The help text for the four buttons. Use \n to denote new line (just like enter).

    L"S??ldner r??umen ihre Positionen\n und k??nnen manuell neu plaziert werden. (|C)",
    L"S??ldner |schw??rmen in alle Richtungen aus\n wenn der Button gedr??ckt wird.",
    L"Mit diesem Button k??nnen Sie w??hlen, wo die S??ldner |gruppiert werden sollen.",
    L"Klicken Sie auf diesen Button, wenn Sie die\n Positionen der S??ldner gew??hlt haben. "
    L"(|E|n|t|e|r)",
    L"Sie m??ssen alle S??ldner positionieren\n bevor die Schlacht beginnt.",

    // Various strings (translate word for word)

    L"Sektor",
    L"Eintrittspunkte w??hlen",

    // Strings used for various popup message boxes. Can be as long as desired.

    L"Das sieht nicht gut aus. Gel??nde ist unzug??nglich. Versuchen Sie es an einer anderen Stelle.",
    L"Plazieren Sie Ihre S??ldner in den markierten Sektor auf der Karte.",

    // This message is for mercs arriving in sectors. Ex: Red has arrived in sector A9.
    // Don't uppercase first character, or add spaces on either end.

    L"ist angekommen im Sektor",

    // These entries are for button popup help text for the prebattle interface. All popup help
    // text supports the use of \n to denote new line. Do not use spaces before or after the \n.
    L"Entscheidet Schlacht |automatisch f??r Sie\nohne Karte zu laden.",
    L"Sie k??nnen den PC-Kampf-Modus nicht benutzen, w??hrend Sie\neinen vom Feind verteidigten Ort "
    L"angreifen.",
    L"Sektor b|etreten und Feind in Kampf verwickeln.",
    L"Gruppe zum vorigen Sektor zu|r??ckziehen.",        // singular version
    L"Alle Gruppen zum vorigen Sektor zu|r??ckziehen.",  // multiple groups with same previous sector

    // various popup messages for battle conditions.

    //%c%d is the sector -- ex: A9
    L"Feinde attackieren Ihre Miliz im Sektor %c%d.",
    //%c%d is the sector -- ex: A9
    L"Monster attackieren Ihre Miliz im Sektor %c%d.",
    // 1st %d refers to the number of civilians eaten by monsters, %c%d is the sector -- ex: A9
    // Note: the minimum number of civilians eaten will be two.
    L"Monster attackieren und t??ten %d Zivilisten im Sektor %s.",
    //%s is the sector -- ex: A9
    L"Feinde attackieren Ihre S??ldner im Sektor %s. Alle S??ldner sind bewu??tlos!",
    //%s is the sector -- ex: A9
    L"Monster attackieren Ihre S??ldner im Sektor %s. Alle S??ldner sind bewu??tlos!",

};

STR16 gpGameClockString[] = {
    // This is the day represented in the game clock. Must be very short, 4 characters max.
    L"Tag",
};

// When the merc finds a key, they can get a description of it which
// tells them where and when they found it.
STR16 sKeyDescriptionStrings[2] = {
    L"gefunden im Sektor:",
    L"gefunden am:",
};

// The headers used to describe various weapon statistics.
INT16 gWeaponStatsDesc[][14] =  // USED TO BE 13
    {
        L"Gew. (%s):",  // weight
        L"Status:",
        L"Anzahl:",   // Number of bullets left in a magazine
        L"Reichw.:",  // Range
        L"Schaden:",
        L"AP:",  // abbreviation for Action Points
        L"",           L"=", L"=",
};

// The headers used for the merc's money.
INT16 gMoneyStatsDesc[][13] = {
    L"Betrag",
    L"Verbleibend:",  // this is the overall balance
    L"Betrag",
    L"Abteilen:",  // the amount he wants to separate from the overall balance to get two piles of
                   // money
    L"Konto",
    L"Saldo",
    L"Betrag",
    L"abheben",
};

// The health of various creatures, enemies, characters in the game. The numbers following each are
// for comment only, but represent the precentage of points remaining.
UINT16 zHealthStr[][13] =  // used to be 10
    {
        L"STIRBT",     //	>= 0
        L"KRITISCH",   //	>= 15
        L"SCHLECHT",   //	>= 30
        L"VERWUNDET",  //	>= 45
        L"GESUND",     //	>= 60
        L"STARK",      // 	>= 75
        L"SEHR GUT",   // 	>= 90
};

STR16 gzMoneyAmounts[6] = {
    L"$1000", L"$100", L"$10", L"OK", L"Abteilen", L"Abheben",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
INT16 gzProsLabel[10] = {
    L"Pro:",
};

INT16 gzConsLabel[10] = {
    L"Kontra:",
};

// Conversation options a player has when encountering an NPC
UINT16 zTalkMenuStrings[6][SMALL_STRING_LENGTH] = {
    L"Wie bitte?",  // meaning "Repeat yourself"
    L"Freundlich",  // approach in a friendly
    L"Direkt",      // approach directly - let's get down to business
    L"Drohen",      // approach threateningly - talk now, or I'll blow your face off
    L"Geben",      L"Rekrutieren",
};

// Some NPCs buy, sell or repair items. These different options are available for those NPCs as
// well.
UINT16 zDealerStrings[4][SMALL_STRING_LENGTH] = {
    L"Handeln",
    L"Kaufen",
    L"Verkaufen",
    L"Reparieren",
};

UINT16 zDialogActions[1][SMALL_STRING_LENGTH] = {
    L"Fertig",
};

STR16 pVehicleStrings[] = {
    L"Eldorado",
    L"Hummer",  // a hummer jeep/truck -- military vehicle
    L"Ice Cream Truck", L"Jeep", L"Tank", L"Helikopter",
};

STR16 pShortVehicleStrings[] = {
    L"Eldor.",
    L"Hummer",  // the HMVV
    L"Truck",  L"Jeep", L"Tank",
    L"Heli",  // the helicopter
};

STR16 zVehicleName[] = {
    L"Eldorado",
    L"Hummer",  // a military jeep. This is a brand name.
    L"Truck",   // Ice cream truck
    L"Jeep",     L"Tank",
    L"Heli",  // an abbreviation for Helicopter
};

// These are messages Used in the Tactical Screen
UINT16 TacticalStr[][MED_STRING_LENGTH] = {
    L"Luftangriff", L"Automatisch Erste Hilfe leisten?",

    // CAMFIELD NUKE THIS and add quote #66.

    L"%s bemerkt, da?? Teile aus der Lieferung fehlen.",

    // The %s is a string from pDoorTrapStrings

    L"Das Schlo?? hat %s.", L"Es gibt kein Schlo??.", L"Erfolg!", L"Fehlschlag.", L"Erfolg!",
    L"Fehlschlag.", L"Das Schlo?? hat keine Falle.", L"Erfolg!",
    // The %s is a merc name
    L"%s hat nicht den richtigen Schl??ssel.", L"Die Falle am Schlo?? ist entsch??rft.",
    L"Das Schlo?? hat keine Falle.", L"Geschl.", L"T??R", L"FALLE AN", L"Geschl.", L"GE??FFNET",
    L"EINGETRETEN", L"Hier ist ein Schalter. Bet??tigen?", L"Falle entsch??rfen?", L"Zur??ck...",
    L"Weiter...", L"Mehr...",

    // In the next 2 strings, %s is an item name

    L"%s liegt jetzt auf dem Boden.", L"%s ist jetzt bei %s.",

    // In the next 2 strings, %s is a name

    L"%s hat den vollen Betrag erhalten.", L"%s bekommt noch %d.",
    L"Detonationsfrequenz ausw??hlen:",      // in this case, frequency refers to a radio signal
    L"Wie viele Z??ge bis zur Explosion:",   // how much time, in turns, until the bomb blows
    L"Ferngesteuerte Z??ndung einstellen:",  // in this case, frequency refers to a radio signal
    L"Falle entsch??rfen?", L"Blaue Flagge wegnehmen?", L"Blaue Flagge hier aufstellen?",
    L"Zug beenden",

    // In the next string, %s is a name. Stance refers to way they are standing.

    L"Wollen Sie %s wirklich angreifen?", L"Fahrzeuge k??nnen ihre Position nicht ??ndern.",
    L"Der Roboter kann seine Position nicht ??ndern.",

    // In the next 3 strings, %s is a name

    //%s can't change to that stance here
    L"%s kann die Haltung hier nicht ??ndern.",

    L"%s kann hier nicht versorgt werden.", L"%s braucht keine Erste Hilfe.",
    L"Kann nicht dorthin gehen.",
    L"Ihr Team ist komplett. Kein Platz mehr f??r Rekruten.",  // there's no room for a recruit on
                                                              // the player's team

    // In the next string, %s is a name

    L"%s wird eskortiert von Trupp %d.",

    // Here %s is a name and %d is a number

    L"%s bekommt noch %d $.",

    // In the next string, %s is a name

    L"%s eskortieren?",

    // In the next string, the first %s is a name and the second %s is an amount of money (including
    // $ sign)

    L"%s f??r %s pro Tag anheuern?",

    // This line is used repeatedly to ask player if they wish to participate in a boxing match.

    L"K??mpfen?",

    // In the next string, the first %s is an item name and the
    // second %s is an amount of money (including $ sign)

    L"%s f??r %s kaufen?",

    // In the next string, %s is a name

    L"%s wird von Trupp %d eskortiert.",

    // These messages are displayed during play to alert the player to a particular situation

    L"KLEMMT",                            // weapon is jammed.
    L"Roboter braucht %s Kaliber Ammo.",  // Robot is out of ammo
    L"Dorthin werfen? Unm??glich.",        // Merc can't throw to the destination he selected

    // These are different buttons that the player can turn on and off.

    L"Stealth Mode (|Z)", L"Kartenbildschir|m", L"Spielzug been|den", L"Sprechen", L"Stumm",
    L"Aufrichten (|P|g|U|p)", L"Cursor Level (|T|a|b)", L"Klettern / Springen",
    L"Ducken (|P|g|D|n)", L"Untersuchen (|C|t|r|l)", L"Voriger S??ldner",
    L"N??chster S??ldner (|S|p|a|c|e)", L"|Optionen", L"Feuersto?? (|B)", L"B|lickrichtung",
    L"Gesundheit: %d/%d\nEnergie: %d/%d\nMoral: %s",
    L"Was?",    // this means "what?"
    L"Forts.",  // an abbrieviation for "Continued"
    L"Stumm aus f??r %s.",
    L"Stumm f??r %s.",  //
    L"Fahrer", L"Fahrzeug verlassen", L"Trupp wechseln", L"Fahren",
    L"n.a.",  // this is an acronym for "Not Applicable."
    L"Benutzen ( Faustkampf )", L"Benutzen ( Feuerwaffe )", L"Benutzen ( Hieb-/Stichwaffe )",
    L"Benutzen ( Sprengstoff )", L"Benutzen ( Verbandskasten )", L"(Fangen)", L"(Nachladen)",
    L"(Geben)", L"%s Falle wurde ausgel??st.", L"%s ist angekommen.",
    L"%s hat keine Action-Punkte mehr.", L"%s ist nicht verf??gbar.", L"%s ist fertig verbunden.",
    L"%s sind die Verb??nde ausgegangen.", L"Feind im Sektor!", L"Keine Feinde in Sicht.",
    L"Nicht genug Action-Punkte.", L"Niemand bedient die Fernbedienung.",
    L"Feuersto?? hat Magazin geleert!", L"SOLDAT", L"MONSTER", L"MILIZ", L"ZIVILIST",
    L"Sektor verlassen", L"OK", L"Abbruch", L"Gew??hlter S??ldner", L"Ganzer Trupp",
    L"Gehe zu Sektor",

    L"Gehe zu Karte",

    L"Sie k??nnen den Sektor von dieser Seite aus nicht verlassen.", L"%s ist zu weit weg.",
    L"Baumkronen entfernen", L"Baumkronen zeigen",
    L"KR??HE",  // Crow, as in the large black bird
    L"NACKEN", L"KOPF", L"TORSO", L"BEINE", L"Der Herrin sagen, was sie wissen will?",
    L"Fingerabdruck-ID gespeichert", L"Falsche Fingerabdruck-ID. Waffe au??er Betrieb",
    L"Ziel erfa??t", L"Weg blockiert",
    L"Geld einzahlen/abheben",  // Help text over the $ button on the Single Merc Panel
    L"Niemand braucht Erste Hilfe.",
    L"Klemmt.",             // Short form of JAMMED, for small inv slots
    L"Kann da nicht hin.",  // used ( now ) for when we click on a cliff
    L"Weg ist blockiert. Mit dieser Person den Platz tauschen?", L"Person will sich nicht bewegen",
    // In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
    L"Mit der Zahlung von %s einverstanden?", L"Gratisbehandlung akzeptieren?", L"Daryl heiraten?",
    L"Schl??sselring", L"Das ist mit einem EPC nicht m??glich.", L"Krott verschonen?",
    L"Au??er Reichweite",
    L"Arbeiter",  // People that work in mines to extract precious metals
    L"Fahrzeug kann nur zwischen Sektoren fahren", L"Automatische Erste Hilfe nicht m??glich",
    L"Weg blockiert f??r %s", L"Ihre von Deidrannas Truppe gefangenen Soldaten sind hier inhaftiert",
    L"Schlo?? getroffen", L"Schlo?? zerst??rt", L"Noch jemand an der T??r.",
    L"Gesundh.: %d/%d\nTank: %d/%d",
    L"%s kann %s nicht sehen.",  // Cannot see person trying to talk to
};

// Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different
// circumstances in the "exiting sector" interface.
STR16 pExitingSectorHelpText[] = {
    // Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box
    // is checked.
    L"Der n??chste Sektor wird sofort geladen, wenn Sie das K??stchen aktivieren.",
    L"Sie kommen sofort zum Kartenbildschirm, wenn Sie das K??stchen aktivieren\nweil die Reise "
    L"Zeit braucht.",

    // If you attempt to leave a sector when you have multiple squads in a hostile sector.
    L"Der Sektor ist von Feinden besetzt. Sie k??nnen keine S??ldner hierlassen.\nRegeln Sie das, "
    L"bevor Sie neue Sektoren laden.",

    // Because you only have one squad in the sector, and the "move all" option is checked, the "go
    // to sector" option is locked to on. The helptext explains why it is locked.
    L"Wenn die restlichen S??ldner den Sektor verlassen,\nwird sofort der n??chste Sektor geladen.",
    L"Wenn die restlichen S??ldner den Sektor verlassen,\nkommen Sie sofort zum "
    L"Kartenbildschirm\nweil die Reise Zeit braucht.",

    // If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being
    // escorted. The "single" button is disabled.
    L"%s kann den Sektor nicht ohne Eskorte verlassen.",

    // If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc
    // will be prohibited from leaving alone. There are several strings depending on the gender of
    // the merc and how many EPCs are in the squad. DO NOT USE THE NEWLINE HERE AS IT IS USED FOR
    // BOTH HELPTEXT AND SCREEN MESSAGES!
    L"%s kann den Sektor nicht verlassen, weil er %s eskortiert.",                // male singular
    L"%s kann den Sektor nicht verlassen, weil sie %s eskortiert.",               // female singular
    L"%s kann den Sektor nicht verlassen, weil er mehrere Personen eskortiert.",  // male plural
    L"%s kann den Sektor nicht verlassen, weil sie mehrere Personen eskortiert.",  // female plural

    // If one or more of your mercs in the selected squad aren't in range of the traversal area,
    // then the "move all" option is disabled, and this helptext explains why.
    L"Alle S??ldner m??ssen in der N??he sein\ndamit der Trupp weiterreisen kann.",

    L"",  // UNUSED

    // Standard helptext for single movement. Explains what will happen (splitting the squad)
    L"Bei aktiviertem K??stchen reist %s alleine und\nbildet automatisch wieder einen Trupp.",

    // Standard helptext for all movement. Explains what will happen (moving the squad)
    L"Bei aktiviertem K??stchen reist der ausgew??hlte Trupp\nweiter und verl????t den Sektor.",

    // This strings is used BEFORE the "exiting sector" interface is created. If you have an EPC
    // selected and you attempt to tactically traverse the EPC while the escorting mercs aren't near
    // enough (or dead, dying, or unconscious), this message will appear and the "exiting sector"
    // interface will not appear. This is just like the situation where This string is special, as
    // it
    // is not used as helptext. Do not use the special newline character (\n) for this string.
    L"%s wird von S??ldnern eskortiert und kann den Sektor nicht alleine verlassen. Die anderen "
    L"S??ldner m??ssen in der N??he sein.",
};

STR16 pRepairStrings[] = {
    L"Gegenst??nde",        // tell merc to repair items in inventory
    L"Raketenst??tzpunkt",  // tell merc to repair SAM site - SAM is an acronym for Surface to Air
                           // Missile
    L"Abbruch",            // cancel this menu
    L"Roboter",            // repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."
STR16 sPreStatBuildString[] = {
    L"verliert",  // the merc has lost a statistic
    L"gewinnt",   // the merc has gained a statistic
    L"Punkt",     // singular
    L"Punkte",    // plural
    L"Level",     // singular
    L"Level",     // plural
};

STR16 sStatGainStrings[] = {
    L"Gesundheit.",         L"Beweglichkeit",   L"Geschicklichkeit",  L"Weisheit.",
    L"an Medizin.",         L"an Sprengstoff.", L"an Technik.",       L"an Treffsicherheit.",
    L"Erfahrungsstufe(n).", L"Kraft.",          L"F??hrungsqualit??t.",
};

STR16 pHelicopterEtaStrings[] = {
    L"Gesamt: ",        // total distance for helicopter to travel
    L" Sicher: ",       // Number of safe sectors
    L" Unsicher:",      // Number of unsafe sectors
    L"Gesamtkosten: ",  // total cost of trip by helicopter
    L"Ank.: ",          // ETA is an acronym for "estimated time of arrival"

    // warning that the sector the helicopter is going to use for refueling is under enemy control
    L"Helikopter hat fast keinen Sprit mehr und mu?? im feindlichen Gebiet landen.", L"Passagiere: ",
    L"Skyrider oder Absprungsort ausw??hlen?", L"Skyrider",
    L"Absprung",  // make sure length doesn't exceed 8 characters (used to be "Absprungsort")
};

STR16 sMapLevelString[] = {
    L"Ebene:",  // what level below the ground is the player viewing in mapscreen
};

STR16 gsLoyalString[] = {
    L"Loyalit??t ",  // the loyalty rating of a town ie : Loyal 53%
};

// error message for when player is trying to give a merc a travel order while he's underground.
STR16 gsUndergroundString[] = {
    L"Ich kann unter der Erde keinen Marschbefehl empfangen.",
};

STR16 gsTimeStrings[] = {
    L"h",  // hours abbreviation
    L"m",  // minutes abbreviation
    L"s",  // seconds abbreviation
    L"T",  // days abbreviation
};

// text for the various facilities in the sector
STR16 sFacilitiesStrings[] = {
    L"Keine",      L"Hospital", L"Fabrik", L"Gef??ngnis", L"Milit??r", L"Flughafen",
    L"Reichweite",  // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button
STR16 pMapPopUpInventoryText[] = {
    L"Inventar",
    L"Exit",
};

// town strings
STR16 pwTownInfoStrings[] = {
    L"Gr????e",            // 0 // size of the town in sectors
    L"",                 // blank line, required
    L"unter Kontrolle",  // how much of town is controlled
    L"Keine",            // none of this town
    L"Mine",             // mine associated with this town
    L"Loyalit??t",        // 5 // the loyalty level of this town
    L"Trainiert",        // the forces in the town trained by the player
    L"",
    L"Wichtigste Geb??ude",  // main facilities in this town
    L"Level",               // the training level of civilians in this town
    L"Zivilistentraining",  // 10 // state of civilian training in town
    L"Miliz",               // the state of the trained civilians in the town
};

// Mine strings
STR16 pwMineStrings[] = {
    L"Mine",  // 0
    L"Silber",
    L"Gold",
    L"Tagesproduktion",
    L"Maximale Produktion",
    L"Aufgegeben",  // 5
    L"Geschlossen",
    L"Fast ersch??pft",
    L"Produziert",
    L"Status",
    L"Produktionsrate",
    L"Erzart",  // 10
    L"Kontrolle ??ber Stadt",
    L"Loyalit??t der Stadt",
    //	L"Minenarbeiter",
};

// blank sector strings
STR16 pwMiscSectorStrings[] = {
    L"Feindliche Verb??nde",
    L"Sektor",
    L"# der Gegenst??nde",
    L"Unbekannt",
    L"Kontrolliert",
    L"Ja",
    L"Nein",
};

// error strings for inventory
STR16 pMapInventoryErrorString[] = {
    L"%s ist nicht nah genug.",  // Merc is in sector with item but not close enough
    L"Diesen S??ldner k??nnen Sie nicht ausw??hlen.",
    L"%s ist nicht im Sektor.",
    L"W??hrend einer Schlacht m??ssen Sie Gegenst??nde manuell nehmen.",
    L"W??hrend einer Schlacht m??ssen Sie Gegenst??nde manuell fallenlassen.",
    L"%s ist nicht im Sektor und kann Gegenstand nicht fallen lassen.",
};

STR16 pMapInventoryStrings[] = {
    L"Ort",                   // sector these items are in
    L"Zahl der Gegenst??nde",  // total number of items in sector
};

// help text for the user
STR16 pMapScreenFastHelpTextList[] = {
    L"Um die Aufgabe eines S??ldners zu ??ndern und ihn einem anderen Trupp, einem Reparatur- oder "
    L"??rzteteam zuzuweisen, klicken Sie in die 'Auftr??ge'-Spalte.",
    L"Um einen S??ldner an einen anderen Bestimmungsort zu versetzen, klicken Sie in die "
    L"'Auftr??ge'-Spalte.",
    L"Wenn ein S??ldner seinen Marschbefehl erhalten hat, kann er sich mit dem Zeitraffer schneller "
    L"bewegen.",
    L"Die linke Maustaste w??hlt den Sektor aus. Zweiter Klick auf die linke Maustaste erteilt "
    L"Marschbefehl an S??ldner. Mit der rechten Maustaste erhalten Sie Kurzinfos ??ber den Sektor.",
    L"Hilfe aufrufen mit Taste 'h'.",
    L"Test-Text",
    L"Test-Text",
    L"Test-Text",
    L"Test-Text",
    L"In diesem Bildschirm k??nnen Sie nicht viel machen, bevor Sie in Arulco ankommen. Wenn Sie "
    L"Ihr Team fertiggestellt haben, klicken Sie auf den Zeitraffer-Button unten links. Dadurch "
    L"vergeht die Zeit schneller, bis Ihr Team in Arulco ankommt.",
};

// movement menu text
STR16 pMovementMenuStrings[] = {
    L"S??ldner in Sektor bewegen",  // title for movement box
    L"Route planen",               // done with movement menu, start plotting movement
    L"Abbruch",                    // cancel this menu
    L"Andere",                     // title for group of mercs not on squads nor in vehicles
};

STR16 pUpdateMercStrings[] = {
    L"Ups:",                       // an error has occured
    L"Vertrag ist abgelaufen:",    // this pop up came up due to a merc contract ending
    L"Auftrag wurde ausgef??hrt:",  // this pop up....due to more than one merc finishing assignments
    L"Diese S??ldner arbeiten wieder:",  // this pop up ....due to more than one merc waking up and
                                        // returing to work
    L"Diese S??ldner schlafen:",   // this pop up ....due to more than one merc being tired and going
                                  // to sleep
    L"Vertrag bald abgelaufen:",  // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text
STR16 pMapScreenBorderButtonHelpText[] = {
    L"St??dte zeigen (|W)", L"|Minen zeigen",           L"|Teams & Feinde zeigen",
    L"Luftr|aum zeigen",   L"Gegenst??nde zeigen (|I)", L"Miliz & Feinde zeigen (|Z)",
};

STR16 pMapScreenBottomFastHelp[] = {
    L"|Laptop", L"Taktik (|E|s|c)", L"|Optionen",
    L"Zeitraffer (|+)",                                   // time compress more
    L"Zeitraffer (|-)",                                   // time compress less
    L"Vorige Nachricht (|U|p)\nSeite zur??ck (|P|g|U|p)",  // previous message in scrollable list
    L"N??chste Nachricht (|D|o|w|n)\nN??chste Seite (|P|g|D|n)",  // next message in the scrollable
                                                                // list
    L"Zeit Start/Stop (|S|p|a|c|e)",                            // start/stop time compression
};

STR16 pMapScreenBottomText[] = {
    L"Kontostand",  // current balance in player bank account
};

STR16 pMercDeadString[] = {
    L"%s ist tot.",
};

STR16 pDayStrings[] = {
    L"Tag",
};

// the list of email sender names
STR16 pSenderNameList[] = {
    L"Enrico",
    L"Psych Pro Inc.",
    L"Online-Hilfe",
    L"Psych Pro Inc.",
    L"Speck",
    L"R.I.S.",
    L"Barry",
    L"Blood",
    L"Lynx",
    L"Grizzly",
    L"Vicki",
    L"Trevor",
    L"Grunty",
    L"Ivan",
    L"Steroid",
    L"Igor",
    L"Shadow",
    L"Red",
    L"Reaper",
    L"Fidel",
    L"Fox",
    L"Sidney",
    L"Gus",
    L"Buns",
    L"Ice",
    L"Spider",
    L"Cliff",
    L"Bull",
    L"Hitman",
    L"Buzz",
    L"Raider",
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
    L"H, A & S Versicherung",
    L"Bobby Rays",
    L"Kingpin",
    L"John Kulba",
    L"A.I.M.",
};

// next/prev strings
STR16 pTraverseStrings[] = {
    L"Vorige",
    L"N??chste",
};

// new mail notify string
STR16 pNewMailStrings[] = {
    L"Sie haben neue Mails...",
};

// confirm player's intent to delete messages
STR16 pDeleteMailStrings[] = {
    L"Mail l??schen?",
    L"UNGELESENE Mail l??schen?",
};

// the sort header strings
STR16 pEmailHeaders[] = {
    L"Absender:",
    L"Betreff:",
    L"Datum:",
};

// email titlebar text
STR16 pEmailTitleText[] = {
    L"Mailbox",
};

// the financial screen strings
STR16 pFinanceTitle[] = {
    L"Buchhalter Plus",  // the name we made up for the financial program in the game
};

STR16 pFinanceSummary[] = {
    L"Haben:",  // the credits column (to ADD money to your account)
    L"Soll:",   // the debits column (to SUBTRACT money from your account)
    L"Eink??nfte vom Vortag:",
    L"Sonstige Einzahlungen vom Vortag:",
    L"Haben vom Vortag:",
    L"Kontostand Ende des Tages:",
    L"Tagessatz:",
    L"Sonstige Einzahlungen von heute:",
    L"Haben von heute:",
    L"Kontostand:",
    L"Voraussichtliche Eink??nfte:",
    L"Prognostizierter Kontostand:",  // projected balance for player for tommorow
};

// headers to each list in financial screen
STR16 pFinanceHeaders[] = {
    L"Tag",              // the day column
    L"Haben",            // the credits column (to ADD money to your account)
    L"Soll",             // the debits column (to SUBTRACT money from your account)
    L"Kontobewegungen",  // transaction type - see TransactionText below
    L"Kontostand",       // balance at this point in time
    L"Seite",            // page number
    L"Tag(e)",           // the day(s) of transactions this page displays
};

STR16 pTransactionText[] = {
    L"Aufgelaufene Zinsen",  // interest the player has accumulated so far
    L"Anonyme Einzahlung",
    L"Bearbeitungsgeb??hr",
    L"Angeheuert",           // Merc was hired
    L"Kauf bei Bobby Rays",  // Bobby Ray is the name of an arms dealer
    L"Ausgeglichene Konten bei M.E.R.C.",
    L"Krankenversicherung f??r %s",  // medical deposit for merc
    L"BSE-Profilanalyse",           // IMP is the acronym for International Mercenary Profiling
    L"Versicherung f??r %s abgeschlossen",
    L"Versicherung f??r %s verringert",
    L"Versicherung f??r %s verl??ngert",  // johnny contract extended
    L"Versicherung f??r %s gek??ndigt",
    L"Versicherungsanspruch f??r %s",  // insurance claim for merc
    L"1 Tag",                         // merc's contract extended for a day
    L"1 Woche",                       // merc's contract extended for a week
    L"2 Wochen",                      // ... for 2 weeks
    L"Minenertrag",
    L"",
    L"Blumen kaufen",
    L"Volle R??ckzahlung f??r %s",
    L"Teilw. R??ckzahlung f??r %s",
    L"Keine R??ckzahlung f??r %s",
    L"Zahlung an %s",                // %s is the name of the npc being paid
    L"??berweisen an %s",             // transfer funds to a merc
    L"??berweisen von %s",            // transfer funds from a merc
    L"Miliz in %s ausbilden",        // initial cost to equip a town's militia
    L"Gegenst??nde von %s gekauft.",  // is used for the Shop keeper interface. The dealers name will
                                     // be appended to the end of the string.
    L"%s hat Geld angelegt.",
};

STR16 pTransactionAlternateText[] = {
    L"Versicherung f??r",            // insurance for a merc
    L"%ss Vertrag verl. um 1 Tag",  // entend mercs contract by a day
    L"%ss Vertrag verl. um 1 Woche",
    L"%ss Vertrag verl. um 2 Wochen",
};

// helicopter pilot payment
STR16 pSkyriderText[] = {
    L"Skyrider wurden $%d gezahlt",             // skyrider was paid an amount of money
    L"Skyrider bekommt noch $%d",               // skyrider is still owed an amount of money
    L"Skyrider hat aufgetankt",                 // skyrider has finished refueling
    L"",                                        // unused
    L"",                                        // unused
    L"Skyrider ist bereit f??r weiteren Flug.",  // Skyrider was grounded but has been freed
    L"Skyrider hat keine Passagiere. Wenn Sie S??ldner in den Sektor transportieren wollen, weisen "
    L"Sie sie einem Fahrzeug/Helikopter zu.",
};

// strings for different levels of merc morale
STR16 pMoralStrings[] = {
    L"Super", L"Gut", L"Stabil", L"Schlecht", L"Panik", L"Mies",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.
STR16 pLeftEquipmentString[] = {
    L"%ss Ausr??stung ist in Omerta angekommen (A9).",
    L"%ss Ausr??stung ist in Drassen angekommen (B13).",
};

// Status that appears on the Map Screen
STR16 pMapScreenStatusStrings[] = {
    L"Gesundheit", L"Energie", L"Moral",
    L"Zustand",  // the condition of the current vehicle (its "health")
    L"Tank",     // the fuel level of the current vehicle (its "energy")
};

STR16 pMapScreenPrevNextCharButtonHelpText[] = {
    L"Voriger S??ldner (|L|e|f|t)",     // previous merc in the list
    L"N??chster S??ldner (|R|i|g|h|t)",  // next merc in the list
};

STR16 pEtaString[] = {
    L"Ank.:",  // eta is an acronym for Estimated Time of Arrival
};

STR16 pTrashItemText[] = {
    L"Sie werden das Ding nie wiedersehen. Trotzdem wegwerfen?",  // do you want to continue and
                                                                  // lose the item forever
    L"Dieser Gegenstand sieht SEHR wichtig aus. Sie sie GANZ SICHER, da?? Sie ihn wegwerfen "
    L"wollen?",  // does the user REALLY want to trash this item
};

STR16 pMapErrorString[] = {
    L"Trupp kann nicht reisen, wenn einer schl??ft.",

    // 1-5
    L"Wir m??ssen erst an die Oberfl??che.",
    L"Marschbefehl? Wir sind in einem feindlichen Sektor!",
    L"Wenn S??ldner reisen sollen, m??ssen sie einem Trupp oder Fahrzeug zugewiesen werden.",
    L"Sie haben noch keine Teammitglieder.",  // you have no members, can't do anything
    L"S??ldner kann nicht gehorchen.",         // merc can't comply with your order
                                              // 6-10
    L"braucht eine Eskorte. Plazieren Sie ihn in einem Trupp mit Eskorte.",  // merc can't move
                                                                             // unescorted .. for a
                                                                             // male
    L"braucht eine Eskorte. Plazieren Sie sie in einem Trupp mit Eskorte.",  // for a female
    L"S??ldner ist noch nicht in Arulco!",
    L"Erst mal Vertrag aushandeln!",
    L"",
    // 11-15
    L"Marschbefehl? Hier tobt ein Kampf!",
    L"Sie sind von Bloodcats umstellt in Sektor %s!",
    L"Sie haben gerade eine Bloodcat-H??hle betreten in Sektor I16!",
    L"",
    L"Raketenst??tzpunkt in %s wurde erobert.",
    // 16-20
    L"Mine in %s wurde erobert. Ihre Tageseinnahmen wurden reduziert auf %s.",
    L"Gegner hat Sektor %s ohne Gegenwehr erobert.",
    L"Mindestens ein S??ldner konnte nicht eingeteilt werden.",
    L"%s konnte sich nicht anschlie??en, weil %s voll ist",
    L"%s konnte sich %s nicht anschlie??en, weil er zu weit weg ist.",
    // 21-25
    L"Die Mine in %s ist von Deidrannas Truppen erobert worden!",
    L"Deidrannas Truppen sind gerade in den Raketenst??tzpunkt in %s eingedrungen",
    L"Deidrannas Truppen sind gerade in %s eingedrungen",
    L"Deidrannas Truppen wurden gerade in %s gesichtet.",
    L"Deidrannas Truppen haben gerade %s erobert.",
    // 26-30
    L"Mindestens ein S??ldner kann nicht schlafen.",
    L"Mindestens ein S??ldner ist noch nicht wach.",
    L"Die Miliz kommt erst, wenn das Training beendet ist.",
    L"%s kann im Moment keine Marschbefehle erhalten.",
    L"Milizen au??erhalb der Stadtgrenzen k??nnen nicht in andere Sektoren reisen.",
    // 31-35
    L"Sie k??nnen keine Milizen in %s haben.",
    L"Leere Fahrzeuge fahren nicht!",
    L"%s ist nicht transportf??hig!",
    L"Sie m??ssen erst das Museum verlassen!",
    L"%s ist tot!",
    // 36-40
    L"%s kann nicht zu %s wechseln, weil der sich bewegt",
    L"%s kann so nicht einsteigen",
    L"%s kann sich nicht %s anschlie??en",
    L"Sie k??nnen den Zeitraffer erst mit neuen S??ldner benutzen!",
    L"Dieses Fahrzeug kann nur auf Stra??en fahren!",
    // 41-45
    L"Reisenden S??ldnern k??nnen Sie keine Auftr??ge erteilen.",
    L"Kein Benzin mehr!",
    L"%s ist zu m??de.",
    L"Keiner kann das Fahrzeug steuern.",
    L"Ein oder mehrere S??ldner dieses Trupps k??nnen sich jetzt nicht bewegen.",
    // 46-50
    L"Ein oder mehrere S??ldner des ANDEREN Trupps kann sich gerade nicht bewegen.",
    L"Fahrzeug zu stark besch??digt!",
    L"Nur zwei S??ldner pro Sektor k??nnen Milizen trainieren.",
    L"Roboter mu?? von jemandem bedient werden. Beide im selben Trupp plazieren.",

};

// help text used during strategic route plotting
STR16 pMapPlotStrings[] = {
    L"Klicken Sie noch einmal auf das Ziel, um die Route zu best??tigen. Klicken Sie auf andere "
    L"Sektoren, um die Route zu ??ndern.",
    L"Route best??tigt.",
    L"Ziel unver??ndert.",
    L"Route ge??ndert.",
    L"Route verk??rzt.",
};

// help text used when moving the merc arrival sector
STR16 pBullseyeStrings[] = {
    L"Klicken Sie auf den Sektor, in dem die S??ldner statt dessen ankommen sollen.",
    L"OK. S??ldner werden in %s abgesetzt",
    L"S??ldner k??nnen nicht dorthin fliegen. Luftraum nicht gesichert!",
    L"Abbruch. Ankunftssektor unver??ndert,",
    L"Luftraum ??ber %s ist nicht mehr sicher! Ankunftssektor jetzt in %s.",
};

// help text for mouse regions
STR16 pMiscMapScreenMouseRegionHelpText[] = {
    L"Ins Inventar gehen (|E|n|t|e|r)",
    L"Gegenstand wegwerfen",
    L"Inventar verlassen (|E|n|t|e|r)",
};

// male version of where equipment is left
STR16 pMercHeLeaveString[] = {
    L"Soll %s seine Ausr??stung hier lassen (%s) oder in Drassen (B13), wenn er Arulco verl????t?",
    L"Soll %s seine Ausr??stung hier lassen (%s) oder in Omerta (A9), wenn er Arulco verl????t?",
    L"geht bald und l????t seine Ausr??stung in Omerta (A9).",
    L"geht bald und l????t seine Ausr??stung in Drassen (B13).",
    L"%s geht bald und l????t seine Ausr??stung in %s.",
};

// female version
STR16 pMercSheLeaveString[] = {
    L"Soll %s ihre Ausr??stung hier lassen (%s) oder in Drassen (B13), bevor sie Arulco verl????t?",
    L"Soll %s ihre Ausr??stung hier lassen (%s) oder in Omerta (A9), bevor sie Arulco verl????t?",
    L"geht bald und l????t ihre Ausr??stung in Omerta (A9).",
    L"geht bald und l????t ihre Ausr??stung in Drassen (B13).",
    L"%s geht bald und l????t ihre Ausr??stung in %s.",
};

STR16 pMercContractOverStrings[] = {
    L"s Vertrag war abgelaufen, und er ist nach Hause gegangen.",   // merc's contract is over and
                                                                    // has departed
    L"s Vertrag war abgelaufen, und sie ist nach Hause gegangen.",  // merc's contract is over and
                                                                    // has departed
    L"s Vertrag wurde gek??ndigt, und er ist weggegangen.",   // merc's contract has been terminated
    L"s Vertrag wurde gek??ndigt, und sie ist weggegangen.",  // merc's contract has been terminated
    L"Sie schulden M.E.R.C. zuviel, also ist %s gegangen.",  // Your M.E.R.C. account is invalid so
                                                             // merc left
};

// Text used on IMP Web Pages
STR16 pImpPopUpStrings[] = {
    L"Ung??ltiger Code",
    L"Sie wollen gerade den ganzen Evaluierungsproze?? von vorn beginnen. Sind Sie sicher?",
    L"Bitte Ihren vollen Namen und Ihr Geschlecht eingeben",
    L"Die ??berpr??fung Ihrer finanziellen Mittel hat ergeben, da?? Sie sich keine Evaluierung "
    L"leisten k??nnen.",
    L"Option zur Zeit nicht g??ltig.",
    L"Um eine genaue Evaluierung durchzuf??hren, m??ssen Sie mindestens noch ein Teammitglied "
    L"aufnehmen k??nnen.",
    L"Evaluierung bereits durchgef??hrt.",
};

// button labels used on the IMP site
STR16 pImpButtonText[] = {
    L"Wir ??ber uns",        // about the IMP site
    L"BEGINNEN",            // begin profiling
    L"Pers??nlichkeit",      // personality section
    L"Eigenschaften",       // personal stats/attributes section
    L"Portr??t",             // the personal portrait selection
    L"Stimme %d",           // the voice selection
    L"Fertig",              // done profiling
    L"Von vorne anfangen",  // start over profiling
    L"Ja, die Antwort pa??t!",
    L"Ja",
    L"Nein",
    L"Fertig",                                // finished answering questions
    L"Zur??ck",                                // previous question..abbreviated form
    L"Weiter",                                // next question
    L"JA",                                    // yes, I am certain
    L"NEIN, ICH M??CHTE VON VORNE ANFANGEN.",  // no, I want to start over the profiling process
    L"JA",
    L"NEIN",
    L"Zur??ck",   // back one page
    L"Abbruch",  // cancel selection
    L"Ja",
    L"Nein, ich m??chte es mir nochmal ansehen.",
    L"Registrieren",               // the IMP site registry..when name and gender is selected
    L"Analyse wird durchgef??hrt",  // analyzing your profile results
    L"OK",
    L"Stimme",
};

STR16 pExtraIMPStrings[] = {
    L"Um mit der Evaluierung zu beginnen, Pers??nlichkeit ausw??hlen.",
    L"Da Sie nun mit der Pers??nlichkeit fertig sind, w??hlen Sie Ihre Eigenschaften aus.",
    L"Nach Festlegung der Eigenschaften k??nnen Sie nun mit der Portr??tauswahl fortfahren.",
    L"W??hlen Sie abschlie??end die Stimmprobe aus, die Ihrer eigenen Stimme am n??chsten kommt.",
};

STR16 pFilesTitle[] = {
    L"Akten einsehen",
};

STR16 pFilesSenderList[] = {
    L"Aufkl??rungsbericht",  // the recon report sent to the player. Recon is an abbreviation for
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
STR16 pHistoryTitle[] = {
    L"Logbuch",
};

STR16 pHistoryHeaders[] = {
    L"Tag",       // the day the history event occurred
    L"Seite",     // the current page in the history report we are in
    L"Tag",       // the days the history report occurs over
    L"Ort",       // location (in sector) the event occurred
    L"Ereignis",  // the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
STR16 pHistoryStrings[] = {
    L"",  // leave this line blank
    // 1-5
    L"%s von A.I.M angeheuert.",      // merc was hired from the aim site
    L"%s von M.E.R.C. angeheuert.",   // merc was hired from the aim site
    L"%s ist tot.",                   // merc was killed
    L"Rechnung an M.E.R.C. bezahlt",  // paid outstanding bills at MERC
    L"Enrico Chivaldoris Auftrag akzeptiert",
    // 6-10
    L"BSE-Profil erstellt",
    L"Versicherung abgeschlossen f??r %s.",  // insurance contract purchased
    L"Versicherung gek??ndigt f??r %s.",      // insurance contract canceled
    L"Versicherung ausgezahlt f??r %s.",     // insurance claim payout for merc
    L"%ss Vertrag um 1 Tag verl??ngert.",    // Extented "mercs name"'s for a day
    // 11-15
    L"%ss Vertrag um 1 Woche verl??ngert.",   // Extented "mercs name"'s for a week
    L"%ss Vertrag um 2 Wochen verl??ngert.",  // Extented "mercs name"'s 2 weeks
    L"%s entlassen.",                        // "merc's name" was dismissed.
    L"%s geht.",                             // "merc's name" quit.
    L"Quest begonnen.",                      // a particular quest started
    // 16-20
    L"Quest gel??st.",
    L"Mit Vorarbeiter in %s geredet",  // talked to head miner of town
    L"%s befreit", L"Cheat benutzt", L"Essen ist morgen in Omerta",
    // 21-25
    L"%s heiratet Daryl Hick", L"%ss Vertrag abgelaufen.", L"%s rekrutiert.",
    L"Enrico sieht kaum Fortschritte", L"Schlacht gewonnen",
    // 26-30
    L"Mine in %s produziert weniger", L"Mine in %s leer", L"Mine in %s geschlossen",
    L"Mine in %s wieder offen", L"Etwas ??ber Gef??ngnis in Tixa erfahren.",
    // 31-35
    L"Von Waffenfabrik in Orta geh??rt.", L"Forscher in Orta gab uns viele Raketengewehre.",
    L"Deidranna verf??ttert Leichen.", L"Frank erz??hlte von K??mpfen in San Mona.",
    L"Patient denkt, er hat in den Minen etwas gesehen.",
    // 36-40
    L"Devin getroffen - verkauft Sprengstoff", L"Ber??hmten Ex-AIM-Mann Mike getroffen!",
    L"Tony getroffen - verkauft Waffen.", L"Sergeant Krott gab mir Raketengewehr.",
    L"Kyle die Urkunde f??r Angels Laden gegeben.",
    // 41-45
    L"Madlab will Roboter bauen.", L"Gabby kann Tinktur gegen K??fer machen.",
    L"Keith nicht mehr im Gesch??ft.", L"Howard lieferte Gift an Deidranna.",
    L"Keith getroffen - verkauft alles in Cambria.",
    // 46-50
    L"Howard getroffen - Apotheker in Balime", L"Perko getroffen - hat kleinen Reparaturladen.",
    L"Sam aus Balime getroffen - hat Computerladen.", L"Franz hat Elektronik und andere Sachen.",
    L"Arnold repariert Sachen in Grumm.",
    // 51-55
    L"Fredo repariert Elektronik in Grumm.", L"Spende von Reichem aus Balime bekommen.",
    L"Schrotth??ndler Jake getroffen.", L"Ein Depp hat uns eine Codekarte gegeben.",
    L"Walter bestochen, damit er Keller ??ffnet.",
    // 56-60
    L"Wenn Dave Sprit hat, bekommen wir's gratis.", L"Pablo bestochen.",
    L"Kingpin hat Geld in San Mona-Mine.", L"%s gewinnt Extremkampf", L"%s verliert Extremkampf",
    // 61-65
    L"%s beim Extremkampf disqualifiziert", L"Viel Geld in verlassener Mine gefunden.",
    L"Von Kingpin geschickten M??rder getroffen", L"Kontrolle ??ber Sektor verloren",
    L"Sektor verteidigt",
    // 66-70
    L"Schlacht verloren",     // ENEMY_ENCOUNTER_CODE
    L"T??dlicher Hinterhalt",  // ENEMY_AMBUSH_CODE
    L"Hinterhalt ausgehoben",
    L"Angriff fehlgeschlagen",  // ENTERING_ENEMY_SECTOR_CODE
    L"Angriff erfolgreich",
    // 71-75
    L"Monster angegriffen",    // CREATURE_ATTACK_CODE
    L"Von Bloodcats get??tet",  // BLOODCAT_AMBUSH_CODE
    L"Bloodcats get??tet", L"%s wurde get??tet", L"Carmen den Kopf eines Terroristen gegeben",
    L"Slay ist gegangen",  // Slay is a merc and has left the team
    L"%s gekillt",         // History log for when a merc kills an NPC or PC
};

STR16 pHistoryLocations[] = {
    L"n.a",  // N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop
STR16 pLaptopIcons[] = {
    L"E-mail",      L"Web", L"Finanzen", L"Personal", L"Logbuch", L"Dateien", L"Schlie??en",
    L"sir-FER 4.0",  // our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added
STR16 pBookMarkStrings[] = {
    L"A.I.M.",           L"Bobby Rays", L"B.S.E",        L"M.E.R.C.",
    L"Bestattungsinst.", L"Florist",    L"Versicherung", L"Abbruch",
};

STR16 pBookmarkTitle[] = {
    L"Lesezeichen",
    L"Rechts klicken, um in Zukunft in dieses Men?? zu gelangen.",
};

// When loading or download a web page
STR16 pDownloadString[] = {
    L"Download l??uft",
    L"Neuladen l??uft",
};

// This is the text used on the bank machines, here called ATMs for Automatic Teller Machine
STR16 gsAtmSideButtonText[] = {
    L"OK",
    L"Nehmen",      // take money from merc
    L"Geben",       // give money to merc
    L"R??ckg??ngig",  // cancel transaction
    L"L??schen",     // clear amount being displayed on the screen
};

STR16 gsAtmStartButtonText[] = {
    L"??berw $",    // transfer money to merc -- short form
    L"Statistik",  // view stats of the merc
    L"Inventar",   // view the inventory of the merc
    L"Anstellung",
};

STR16 sATMText[] = {
    L"Geld ??berw.?",                       // transfer funds to merc?
    L"Ok?",                                // are we certain?
    L"Betrag eingeben",                    // enter the amount you want to transfer to merc
    L"Art ausw??hlen",                      // select the type of transfer to merc
    L"Nicht genug Geld",                   // not enough money to transfer to merc
    L"Betrag mu?? durch $10 teilbar sein",  // transfer amount must be a multiple of $10
};

// Web error messages. Please use German equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator
STR16 pErrorStrings[] = {
    L"Fehler",
    L"Server hat keinen DNS-Eintrag.",
    L"URL-Adresse ??berpr??fen und nochmal versuchen.",
    L"OK",
    L"Verbindung zum Host wird dauernd unterbrochen. Mit l??ngeren ??bertragungszeiten ist zu "
    L"rechnen.",
};

STR16 pPersonnelString[] = {
    L"S??ldner:",  // mercs we have
};

STR16 pWebTitle[] = {
    L"sir-FER 4.0",  // our name for the version of the browser, play on company name
};

// The titles for the web program title bar, for each page loaded
STR16 pWebPagesTitles[] = {
    L"A.I.M.",
    L"A.I.M. Mitglieder",
    L"A.I.M. Bilder",  // a mug shot is another name for a portrait
    L"A.I.M. Sortierfunktion",
    L"A.I.M.",
    L"A.I.M. Veteranen",
    L"A.I.M. Politik",
    L"A.I.M. Geschichte",
    L"A.I.M. Links",
    L"M.E.R.C.",
    L"M.E.R.C. Konten",
    L"M.E.R.C. Registrierung",
    L"M.E.R.C. Index",
    L"Bobby Rays",
    L"Bobby Rays - Waffen",
    L"Bobby Rays - Munition",
    L"Bobby Rays - R??stungen",
    L"Bobby Rays - Sonst.",  // misc is an abbreviation for miscellaneous
    L"Bobby Rays - Gebraucht",
    L"Bobby Rays - Mail Order",
    L"B.S.E",
    L"B.S.E",
    L"Fleuropa",
    L"Fleuropa - Gestecke",
    L"Fleuropa - Bestellformular",
    L"Fleuropa - Karten",
    L"Hammer, Ambo?? & Steigb??gel Versicherungsmakler",
    L"Information",
    L"Vertrag",
    L"Bemerkungen",
    L"McGillicuttys Bestattungen",
    L"",
    L"URL nicht gefunden.",
    L"Bobby Rays - Letzte Lieferungen",
    L"",
    L"",
};

STR16 pShowBookmarkString[] = {
    L"Sir-Help",
    L"Erneut auf Web klicken f??r Lesezeichen.",
};

STR16 pLaptopTitles[] = {
    L"E-Mail", L"Dateien", L"S??ldner-Manager", L"Buchhalter Plus", L"Logbuch",
};

STR16 pPersonnelDepartedStateStrings[] = {
    //(careful not to exceed 18 characters total including spaces)
    // reasons why a merc has left.
    L"Get??tet", L"Entlassen", L"Sonstiges", L"Heirat", L"Vertrag zu Ende", L"Quit",
};

// personnel strings appearing in the Personnel Manager on the laptop
STR16 pPersonelTeamStrings[] = {
    L"Aktuelles Team",     L"Ausgeschieden",     L"Tgl. Kosten:", L"H??chste Kosten:",
    L"Niedrigste Kosten:", L"Im Kampf get??tet:", L"Entlassen:",   L"Sonstiges:",
};

STR16 pPersonnelCurrentTeamStatsStrings[] = {
    L"Schlechteste",
    L"Durchschn.",
    L"Beste",
};

STR16 pPersonnelTeamStatsStrings[] = {
    L"GSND", L"BEW", L"GES", L"KRF", L"FHR", L"WSH", L"ERF", L"TRF", L"TEC", L"SPR", L"MED",
};

// horizontal and vertical indices on the map screen
STR16 pMapVertIndex[] = {
    L"X", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H",
    L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P",
};

STR16 pMapHortIndex[] = {
    L"X", L"1",  L"2",  L"3",  L"4",  L"5",  L"6",  L"7",  L"8",
    L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16",
};

STR16 pMapDepthIndex[] = {
    L"",
    L"-1",
    L"-2",
    L"-3",
};

// text that appears on the contract button
STR16 pContractButtonString[] = {
    L"Vertrag",
};

// text that appears on the update panel buttons
STR16 pUpdatePanelButtons[] = {
    L"Weiter",
    L"Stop",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle
UINT16 LargeTacticalStr[][LARGE_STRING_LENGTH] = {
    L"Sie sind in diesem Sektor geschlagen worden!",
    L"Der Feind hat kein Erbarmen mit den Seelen Ihrer Teammitglieder und verschlingt jeden "
    L"einzelnen.",
    L"Ihre bewu??tlosen Teammitglieder wurden gefangengenommen!",
    L"Ihre Teammitglieder wurden vom Feind gefangengenommen.",
};

// Insurance Contract.c
// The text on the buttons at the bottom of the screen.
STR16 InsContractText[] = {
    L"Zur??ck",
    L"Vor",
    L"OK",
    L"L??schen",
};

// Insurance Info
// Text on the buttons on the bottom of the screen
STR16 InsInfoText[] = {
    L"Zur??ck",
    L"Vor",
};

// For use at the M.E.R.C. web site. Text relating to the player's account with MERC
STR16 MercAccountText[] = {
    // Text on the buttons on the bottom of the screen
    L"Befugnis ert.",
    L"Home",
    L"Konto #:",
    L"S??ldner",
    L"Tage",
    L"Rate",  // 5
    L"Belasten",
    L"Gesamt:",
    L"Zahlung von %s wirklich genehmigen?",  // the %s is a string that contains the dollar amount (
                                             // ex. "$150" )
};

// For use at the M.E.R.C. web site. Text relating a MERC mercenary
STR16 MercInfo[] = {
    L"Gesundheit",
    L"Beweglichkeit",
    L"Geschicklichkeit",
    L"Kraft",
    L"F??hrungsqualit??t",
    L"Weisheit",
    L"Erfahrungsstufe",
    L"Treffsicherheit",
    L"Technik",
    L"Sprengstoffe",
    L"Medizin",

    L"Zur??ck",
    L"Anheuern",
    L"Weiter",
    L"Zusatzinfo",
    L"Home",
    L"Abwesend",
    L"Sold:",
    L"Pro Tag",
    L"Verstorben",

    L"Sie versuchen, zuviele S??ldner anzuheuern. 18 ist Maximum.",
    L"nicht da",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC
STR16 MercNoAccountText[] = {
    // Text on the buttons at the bottom of the screen
    L"Konto er??ffnen",
    L"R??ckg??ngig",
    L"Sie haben kein Konto. M??chten Sie eins er??ffnen?",
};

// For use at the M.E.R.C. web site. MERC Homepage
STR16 MercHomePageText[] = {
    // Description of various parts on the MERC page
    L"Speck T. Kline, Gr??nder und Besitzer",
    L"Hier klicken, um ein Konto zu er??ffnen",
    L"Hier klicken, um das Konto einzusehen",
    L"Hier klicken, um Dateien einzusehen.",
    // The version number on the video conferencing system that pops up when Speck is talking
    L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.
STR16 sFuneralString[] = {
    L"McGillicuttys Bestattungen: Wir tr??sten trauernde Familien seit 1983.",
    L"Der Bestattungsunternehmer und fr??here A.I.M.-S??ldner Murray \"Pops\" McGillicutty ist ein "
    L"ebenso versierter wie erfahrener Bestatter.",
    L"Pops hat sein ganzes Leben mit Todes- und Trauerf??llen verbracht. Deshalb wei?? er aus erster "
    L"Hand, wie schwierig das sein kann.",
    L"Das Bestattungsunternehmen McGillicutty bietet Ihnen einen umfassenden Service, angefangen "
    L"bei der Schulter zum Ausweinen bis hin zur kosmetischen Aufbereitung entstellter "
    L"K??rperteile.",
    L"McGillicuttys Bestattungen - und Ihre Lieben ruhen in Frieden.",

    // Text for the various links available at the bottom of the page
    L"BLUMEN",
    L"S??RGE UND URNEN",
    L"FEUERBEST.",
    L"GR??BER",
    L"PIET??T",

    // The text that comes up when you click on any of the links ( except for send flowers ).
    L"Leider ist diese Site aufgrund eines Todesfalles in der Familie noch nicht fertiggestellt. "
    L"Sobald das Testament er??ffnet worden und die Verteilung des Erbes gekl??rt ist, wird diese "
    L"Site fertiggestellt.",
    L"Unser Mitgef??hl gilt trotzdem all jenen, die es diesmal versucht haben. Bis sp??ter.",
};

// Text for the florist Home page
STR16 sFloristText[] = {
    // Text on the button on the bottom of the page

    L"Galerie",

    // Address of United Florist

    L"\"Wir werfen ??berall per Fallschirm ab\"",
    L"1-555-SCHNUPPER-MAL",
    L"333 Duftmarke Dr, Aroma City, CA USA 90210",
    L"http://www.schnupper-mal.com",

    // detail of the florist page

    L"Wir arbeiten schnell und effizient",
    L"Lieferung am darauffolgenden Tag, in fast jedes Land der Welt. Ausnahmen sind m??glich. ",
    L"Wir haben die garantiert niedrigsten Preise weltweit!",
    L"Wenn Sie anderswo einen niedrigeren Preis f??r irgend ein Arrangement sehen, bekommen Sie von "
    L"uns ein Dutzend Rosen umsonst!",
    L"Fliegende Flora, Fauna & Blumen seit 1981.",
    L"Unsere hochdekorierten Ex-Bomber-Piloten werfen das Bouquet in einem Radius von zehn Meilen "
    L"rund um den Bestimmungsort ab. Jederzeit!",
    L"Mit uns werden Ihre blumigsten Fantasien wahr",
    L"Bruce, unser weltber??hmter Designer-Florist, verwendet nur die frischesten handverlesenen "
    L"Blumen aus unserem eigenen Gew??chshaus.",
    L"Und denken Sie daran: Was wir nicht haben, pflanzen wir f??r Sie - und zwar schnell!",
};

// Florist OrderForm
STR16 sOrderFormText[] = {

    // Text on the buttons

    L"Zur??ck",
    L"Senden",
    L"L??schen",
    L"Galerie",

    L"Name des Gestecks:",
    L"Preis:",  // 5
    L"Bestellnr.:",
    L"Liefertermin",
    L"Morgen",
    L"Egal",
    L"Bestimmungsort",  // 10
    L"Extraservice",
    L"Kaputtes Gesteck($10)",
    L"Schwarze Rosen($20)",
    L"Welkes Gesteck($10)",
    L"Fr??chtekuchen (falls vorr??tig)($10)",  // 15
    L"Pers??nliche Worte:",
    L"Aufgrund der Kartengr????e darf Ihre Botschaft nicht l??nger sein als 75 Zeichen.",
    L"...oder w??hlen Sie eine unserer",

    L"STANDARD-KARTEN",
    L"Rechnung f??r",  // 20

    // The text that goes beside the area where the user can enter their name

    L"Name:",
};

// Florist Gallery.c
STR16 sFloristGalleryText[] = {
    // text on the buttons
    L"Zur??ck",  // abbreviation for previous
    L"Weiter",  // abbreviation for next
    L"Klicken Sie auf das Gesteck Ihrer Wahl",
    L"Bitte beachten Sie, da?? wir f??r jedes kaputte oder verwelkte Gesteck einen Aufpreis von $10 "
    L"berechnen.",
    L"Home",
};

STR16 sFloristCards[] = {
    L"Klicken Sie auf das Gesteck Ihrer Wahl",
    L"Zur??ck",
};

// Text for Bobby Ray's Mail Order Site
STR16 BobbyROrderFormText[] = {
    L"Bestellformular",                 // Title of the page
    L"St.",                             // The number of items ordered
    L"Gew. (%s)",                       // The weight of the item
    L"Artikel",                         // The name of the item
    L"Preis",                           // the item's weight
    L"Summe",                           // 5	// The total price of all of items of the same type
    L"Zwischensumme",                   // The sub total of all the item totals added
    L"Frachtk. (vgl. Bestimmungsort)",  // S&H is an acronym for Shipping and Handling
    L"Endbetrag",  // The grand total of all item totals + the shipping and handling
    L"Bestimmungsort",
    L"Liefergeschwindigkeit",              // 10	// See below
    L"$ (pro %s)",                         // The cost to ship the items
    L"??bernacht Express",                  // Gets deliverd the next day
    L"2 Arbeitstage",                      // Gets delivered in 2 days
    L"Standard-Service",                   // Gets delivered in 3 days
    L"L??schen",                            // 15			// Clears the order page
    L"Bestellen",                          // Accept the order
    L"Zur??ck",                             // text on the button that returns to the previous page
    L"Home",                               // Text on the button that returns to the home page
    L"* Gebrauchte Gegenst??nde anzeigen",  // Disclaimer stating that the item is used
    L"Sie haben nicht genug Geld.",        // 20	// A popup message that to warn of not enough money
    L"<KEINER>",                           // Gets displayed when there is no valid city selected
    L"Wollen Sie Ihre Bestellung wirklich nach %s schicken?",  // A popup that asks if the city
                                                               // selected is the correct one
    L"Packungs-Gew.**",                                        // Displays the weight of the package
    L"** Min. Gew.",  // Disclaimer states that there is a minimum weight for the package
    L"Lieferungen",
};

// This text is used when on the various Bobby Ray Web site pages that sell items
STR16 BobbyRText[] = {
    L"Bestellen",  // Title
    L"Klicken Sie auf den gew??nschten Gegenstand. Weiteres Klicken erh??ht die St??ckzahl. "
    L"Rechte Maustaste verringert St??ckzahl. Wenn Sie fertig sind, weiter mit dem "
    L"Bestellformular.",  // instructions on how to order

    // Text on the buttons to go the various links

    L"Zur??ck",     //
    L"Feuerwfn.",  // 3
    L"Munition",   // 4
    L"R??stung",    // 5
    L"Sonstiges",  // 6	//misc is an abbreviation for miscellaneous
    L"Gebraucht",  // 7
    L"Vor",
    L"BESTELLEN",
    L"Home",  // 10

    // The following 2 lines are used on the Ammunition page.
    // They are used for help text to display how many items the player's merc has
    // that can use this type of ammo

    L"Ihr Team hat",                           // 11
    L"Waffe(n), die dieses Kaliber benutzen",  // 12

    // The following lines provide information on the items

    L"Gew.:",          // Weight of all the items of the same type
    L"Kal:",           // the caliber of the gun
    L"Mag:",           // number of rounds of ammo the Magazine can hold
    L"Reichw.",        // The range of the gun
    L"Schaden",        // Damage of the weapon
    L"Freq.:",         // Weapon's Rate Of Fire, acroymn ROF
    L"Preis:",         // Cost of the item
    L"Vorr??tig:",      // The number of items still in the store's inventory
    L"Bestellt:",      // The number of items on order
    L"Besch??digt",     // If the item is damaged
    L"Gew.:",          // the Weight of the item
    L"Summe:",         // The total cost of all items on order
    L"* %% Funktion",  // if the item is damaged, displays the percent function of the item

    // Popup that tells the player that they can only order 10 items at a time
    L"Mist! Mit diesem Formular k??nnen Sie nur 10 Sachen bestellen. Wenn Sie mehr wollen (was "
    L"wir sehr hoffen), f??llen Sie bitte noch ein Formular aus.",

    // A popup that tells the user that they are trying to order more items then the store has
    // in stock

    L"Sorry. Davon haben wir leider im Moment nichts mehr auf Lager. Versuchen Sie es sp??ter "
    L"noch einmal.",

    // A popup that tells the user that the store is temporarily sold out

    L"Es tut uns sehr leid, aber im Moment sind diese Sachen total ausverkauft.",

};

// Text for Bobby Ray's Home Page
STR16 BobbyRaysFrontText[] = {
    // Details on the web site

    L"Dies ist die hei??este Site f??r Waffen und milit??rische Ausr??stung aller Art",
    L"Welchen Sprengstoff Sie auch immer brauchen - wir haben ihn.",
    L"SECOND HAND",

    // Text for the various links to the sub pages

    L"SONSTIGES",
    L"FEUERWAFFEN",
    L"MUNITION",  // 5
    L"R??STUNG",

    // Details on the web site

    L"Was wir nicht haben, das hat auch kein anderer",
    L"In Arbeit",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug
// shot page
STR16 AimSortText[] = {
    L"A.I.M. Mitglieder",  // Title
    L"Sortieren:",         // Title for the way to sort

    // sort by...

    L"Preis",
    L"Erfahrung",
    L"Treffsicherheit",
    L"Medizin",
    L"Sprengstoff",
    L"Technik",

    // Text of the links to other AIM pages

    L"Den S??ldner-Kurzindex ansehen",
    L"Personalakte der S??ldner ansehen",
    L"Die AIM-Veteranengalerie ansehen",

    // text to display how the entries will be sorted

    L"Aufsteigend",
    L"Absteigend",
};

// Aim Policies.c
// The page in which the AIM policies and regulations are displayed
STR16 AimPolicyText[] = {
    // The text on the buttons at the bottom of the page

    L"Zur??ck", L"AIM HomePage", L"Regel-Index", L"N??chste Seite", L"Ablehnen", L"Zustimmen",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries
// Instructions to the user to either start video conferencing with the merc, or to go the mug shot
// index
STR16 AimMemberText[] = {
    L"Linksklick", L"zum Kontaktieren.", L"Rechtsklick", L"zum Foto-Index.",
    //	L"Linksklick zum Kontaktieren. \nRechtsklick zum Foto-Index.",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries
STR16 CharacterInfo[] = {
    // The various attributes of the merc

    L"Gesundh.", L"Beweglichkeit", L"Geschicklichkeit", L"Kraft", L"F??hrungsqualit??t", L"Weisheit",
    L"Erfahrungsstufe", L"Treffsicherheit", L"Technik", L"Sprengstoff",
    L"Medizin",  // 10

    // the contract expenses' area

    L"Preis", L"Vertrag", L"1 Tag", L"1 Woche", L"2 Wochen",

    // text for the buttons that either go to the previous merc,
    // start talking to the merc, or go to the next merc

    L"Zur??ck", L"Kontakt", L"Weiter",
    L"Zusatzinfo",                 // Title for the additional info for the merc's bio
    L"Aktive Mitglieder",          // 20		// Title of the page
    L"Zus??tzl. Ausr??st:",          // Displays the optional gear cost
    L"VERSICHERUNG erforderlich",  // If the merc required a medical deposit, this is displayed
};

// Aim Member.c
// The page in which the player's hires AIM mercenaries
// The following text is used with the video conference popup
STR16 VideoConfercingText[] = {
    L"Vertragskosten:",  // Title beside the cost of hiring the merc

    // Text on the buttons to select the length of time the merc can be hired

    L"1 Tag", L"1 Woche", L"2 Wochen",

    // Text on the buttons to determine if you want the merc to come with the equipment

    L"Keine Ausr??stung", L"Ausr??stung kaufen",

    // Text on the Buttons

    L"GELD ??BERWEISEN",  // to actually hire the merc
    L"ABBRECHEN",        // go back to the previous menu
    L"ANHEUERN",         // go to menu in which you can hire the merc
    L"AUFLEGEN",         // stops talking with the merc
    L"OK",
    L"NACHRICHT",  // if the merc is not there, you can leave a message

    // Text on the top of the video conference popup

    L"Videokonferenz mit", L"Verbinde. . .",

    L"versichert",  // Displays if you are hiring the merc with the medical deposit

};

// Aim Member.c
// The page in which the player hires AIM mercenaries
// The text that pops up when you select the TRANSFER FUNDS button
STR16 AimPopUpText[] = {
    L"ELEKTRONISCHE ??BERWEISUNG AUSGEF??HRT",      // You hired the merc
    L"??BERWEISUNG KANN NICHT BEARBEITET WERDEN",  // Player doesn't have enough money, message 1
    L"NICHT GENUG GELD",                          // Player doesn't have enough money, message 2

    // if the merc is not available, one of the following is displayed over the merc's face

    L"Im Einsatz",
    L"Bitte Nachricht hinterlassen",
    L"Verstorben",

    // If you try to hire more mercs than game can support

    L"Sie haben bereits 18 S??ldner in Ihrem Team.",

    L"Mailbox",
    L"Nachricht aufgenommen",
};

// AIM Link.c
STR16 AimLinkText[] = {
    L"A.I.M. Links",  // The title of the AIM links page
};

// Aim History
// This page displays the history of AIM
STR16 AimHistoryText[] = {
    L"Die Geschichte von A.I.M.",  // Title

    // Text on the buttons at the bottom of the page

    L"Zur??ck",
    L"Home",
    L"Veteranen",
    L"Weiter",
};

// Aim Mug Shot Index
// The page in which all the AIM members' portraits are displayed in the order selected by the AIM
// sort page.
STR16 AimFiText[] = {
    // displays the way in which the mercs were sorted

    L"Preis",
    L"Erfahrung",
    L"Treffsicherheit",
    L"Medizin",
    L"Sprengstoff",
    L"Technik",

    // The title of the page, the above text gets added at the end of this text
    L"A.I.M.-Mitglieder ansteigend sortiert nach %s",
    L"A.I.M. Mitglieder absteigend sortiert nach %s",

    // Instructions to the players on what to do

    L"Linke Maustaste",
    L"um S??ldner auszuw??hlen",  // 10
    L"Rechte Maustaste",
    L"um Optionen einzustellen",

    // Gets displayed on top of the merc's portrait if they are...

    // Please be careful not to increase the size of strings for following three
    L"Abwesend",
    L"Verstorben",  // 14
    L"Im Dienst",
};

// AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer
// with AIM
STR16 AimAlumniText[] = {
    // Text of the buttons
    L"SEITE 1",
    L"SEITE 2",
    L"SEITE 3",
    L"A.I.M.-Veteranen",  // Title of the page
    L"ENDE",              // Stops displaying information on selected merc
};

// AIM Home Page
STR16 AimScreenText[] = {
    // AIM disclaimers

    L"A.I.M. und das A.I.M.-Logo sind in den meisten L??ndern eingetragene Warenzeichen.",
    L"Also denken Sie nicht mal daran, uns nachzumachen.",
    L"Copyright 1998-1999 A.I.M., Ltd. Alle Rechte vorbehalten.",

    // Text for an advertisement that gets displayed on the AIM page

    L"Fleuropa",
    L"\"Wir werfen ??berall per Fallschirm ab\"",  // 10
    L"Treffen Sie gleich zu Anfang",
    L"... die richtige Wahl.",
    L"Was wir nicht haben, das brauchen Sie auch nicht.",
};

// Aim Home Page
STR16 AimBottomMenuText[] = {
    // Text for the links at the bottom of all AIM pages

    L"Home", L"Mitglieder", L"Veteranen", L"Regeln", L"Geschichte", L"Links",
};

// ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.
STR16 SKI_Text[] = {
    L"WAREN VORR??TIG",  // Header for the merchandise available
    L"SEITE",           // The current store inventory page being displayed
    L"KOSTEN",          // The total cost of the the items in the Dealer inventory area
    L"WERT",            // The total value of items player wishes to sell
    L"SCH??TZUNG",       // Button text for dealer to evaluate items the player wants to sell
    L"TRANSAKTION",     // Button text which completes the deal. Makes the transaction.
    L"FERTIG",          // Text for the button which will leave the shopkeeper interface.
    L"KOSTEN",          // The amount the dealer will charge to repair the merc's goods
    L"1 STUNDE",    // SINGULAR! The text underneath the inventory slot when an item is given to the
                    // dealer to be repaired
    L"%d STUNDEN",  // PLURAL! The text underneath the inventory slot when an item is given to the
                    // dealer to be repaired
    L"REPARIERT",   // Text appearing over an item that has just been repaired by a NPC repairman
                    // dealer
    L"Es ist kein Platz mehr, um Sachen anzubieten.",  // Message box that tells the user there is
                                                       // no more room to put there stuff
    L"%d MINUTEN",  // The text underneath the inventory slot when an item is given to the dealer to
                    // be repaired
    L"Gegenstand fallenlassen.",
};

// ShopKeeper Interface
// for the bank machine panels. Referenced here is the acronym ATM, which means Automatic Teller
// Machine
STR16 SkiAtmText[] = {
    // Text on buttons on the banking machine, displayed at the bottom of the page
    L"0",       L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
    L"OK",       // Transfer the money
    L"Nehmen",   // Take money from the player
    L"Geben",    // Give money to the player
    L"Abbruch",  // Cancel the transfer
    L"L??schen",  // Clear the money display
};

// Shopkeeper Interface
STR16 gzSkiAtmText[] = {
    // Text on the bank machine panel that....
    L"Vorgang ausw??hlen",           // tells the user to select either to give or take from the merc
    L"Betrag eingeben",             // Enter the amount to transfer
    L"Geld an S??ldner ??berweisen",  // Giving money to the merc
    L"Geld von S??ldner ??berweisen",  // Taking money from the merc
    L"Nicht genug Geld",             // Not enough money to transfer
    L"Kontostand",                   // Display the amount of money the player currently has
};

STR16 SkiMessageBoxText[] = {
    L"M??chten Sie %s von Ihrem Konto abbuchen, um die Differenz zu begleichen?",
    L"Nicht genug Geld. Ihnen fehlen %s",
    L"M??chten Sie %s von Ihrem Konto abbuchen, um die Kosten zu decken?",
    L"H??ndler bitten, mit der ??berweisung zu beginnen.",
    L"H??ndler bitten, Gegenst??nde zu reparieren",
    L"Unterhaltung beenden",
    L"Kontostand",
};

// OptionScreen.c
STR16 zOptionsText[] = {
    // button Text
    L"Spiel sichern",
    L"Spiel laden",
    L"Spiel beenden",
    L"Fertig",
    // Text above the slider bars
    L"Effekte",
    L"Sprache",
    L"Musik",
    // Confirmation pop when the user selects..
    L"Spiel verlassen und zur??ck zum Hauptmen???",
    L"Sprachoption oder Untertitel m??ssen aktiviert sein.",
};

// SaveLoadScreen
STR16 zSaveLoadText[] = {
    L"Spiel sichern",
    L"Spiel laden",
    L"Abbrechen",
    L"Auswahl speichern",
    L"Auswahl laden",

    L"Spiel erfolgreich gespeichert",
    L"FEHLER beim Speichern des Spiels!",
    L"Spiel erfolgreich geladen",
    L"FEHLER beim Laden des Spiels!",

    L"Der gespeicherte Spielstand unterscheidet sich vom aktuellen Spielstand. Es kann "
    L"wahrscheinlich nichts passieren. Weiter?",
    L"Die gespeicherten Spielst??nde sind evtl. besch??digt Wollen Sie sie alle l??schen?",

// Translators, the next two strings are for the same thing. The first one is for beta version
// releases and the second one is used for the final version. Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed. It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Gespeicherte Version wurde ge??ndert. Bitte melden Sie etwaige Probleme. Weiter?",
#else
    L"Versuche, ??lteren Spielstand zu laden. Laden und automatisch aktualisieren?",
#endif

// Translators, the next two strings are for the same thing. The first one is for beta version
// releases and the second one is used for the final version. Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed. It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Spielstand und Spieleversion wurden ge??ndert. Bitte melden Sie etwaige Probleme. Weiter?",
#else
    L"Versuche, ??lteren Spielstand zu laden. Laden und automatisch aktualisieren?",
#endif

    L"Gespeichertes Spiel in Slot #%d wirklich ??berschreiben?",
    L"Wollen Sie das Spiel aus Slot # speichern?",

    // The first %d is a number that contains the amount of free space on the users hard drive,
    // the second is the recommended amount of free space.
    //
    L"Sie haben zu wenig Festplattenspeicher. Sie haben nur %d MB frei und JA2 ben??tigt mindestens "
    L"%d MB.",

    L"Speichere...",  // While the game is saving this message appears.

    L"Normale Waffen",
    L"Zusatzwaffen",
    L"Real-Stil",
    L"SciFi-Stil",
    L"Schwierigkeit",
};

// MapScreen
STR16 zMarksMapScreenText[] = {
    L"Map-Level",
    L"Sie haben gar keine Miliz. Sie m??ssen Bewohner der Stadt trainieren, wenn Sie dort eine "
    L"Miliz aufstellen wollen.",
    L"T??gl. Einkommen",
    L"S??ldner hat Lebensversicherung",
    L"%s ist nicht m??de.",
    L"%s ist unterwegs und kann nicht schlafen.",
    L"%s ist zu m??de. Versuchen Sie es ein bi??chen sp??ter nochmal.",
    L"%s f??hrt.",
    L"Der Trupp kann nicht weiter, wenn einer der S??ldner pennt.",

    // stuff for contracts
    L"Sie k??nnen zwar den Vertrag bezahlen, haben aber kein Geld f??r die Lebensversicherung.",
    L"%s Lebensversicherungspr??mien kosten %s pro %d Zusatztag(en). Wollen Sie das bezahlen?",
    L"Inventar ausw??hlen.",

    L"S??ldner hat Krankenversicherung.",

    // other items
    L"Sanit??ter",  // people acting a field medics and bandaging wounded mercs
    L"Patienten",  // people who are being bandaged by a medic
    L"Fertig",     // Continue on with the game after autobandage is complete
    L"Stop",       // Stop autobandaging of patients by medics now
    L"Sorry. Diese Option gibt es in der Demo nicht.",  // informs player this option/button has
                                                        // been disabled in the demo

    L"%s hat kein Werkzeug.",
    L"%s hat kein Verbandszeug.",
    L"Es sind nicht genug Leute zum Training bereit.",
    L"%s ist voller Milizen.",
    L"S??ldner hat begrenzten Vertrag.",
    L"Vertrag des S??ldners ist nicht versichert",
};

STR16 pLandMarkInSectorString[] = {
    L"Trupp %d hat in Sektor %s jemanden bemerkt",
};

// confirm the player wants to pay X dollars to build a militia force in town
STR16 pMilitiaConfirmStrings[] = {
    L"Eine Milizeinheit f??r diese Stadt zu trainieren kostet $",  // telling player how much it will
                                                                  // cost
    L"Ausgabe genehmigen?",          // asking player if they wish to pay the amount requested
    L"Sie haben nicht genug Geld.",  // telling the player they can't afford to train this town
    L"Miliz in %s (%s %d) weitertrainieren?",  // continue training this town?
    L"Preis $",                                // the cost in dollars to train militia
    L"( J/N )",                                // abbreviated yes/no
    L"Miliz auf der Raketenbasis im Sektor %s (%s %d) weitertrainieren?",  // continue trainign
                                                                           // militia in SAM site
                                                                           // sector
    L"Milizen in %d Sektoren zu trainieren kostet $ %d. %s",  // cost to train sveral sectors at
                                                              // once
    L"Sie k??nnen sich keine $%d f??r die Miliz hier leisten.",
    L"%s ben??tigt eine Loyalit??t von %d Prozent, um mit dem Milizen-Training fortzufahren.",
    L"Sie k??nnen die Miliz in %s nicht mehr trainieren.",
};

// Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom
// of the single merc panel
STR16 gzMoneyWithdrawMessageText[] = {
    L"Sie k??nnen nur max. 20,000$ abheben.",
    L"Wollen Sie wirklich %s auf Ihr Konto einzahlen?",
};

STR16 gzCopyrightText[] = {
    L"Copyright (C) 1999 Sir-tech Canada Ltd. Alle Rechte vorbehalten.",  //
};

// option Text
STR16 zOptionsToggleText[] = {
    L"Sprache",
    L"Stumme Best??tigungen",
    L"Untertitel",
    L"Dialoge Pause",
    L"Rauch animieren",
    L"Sichtbare Verletzungen",
    L"Cursor nicht bewegen!",
    L"Alte Auswahlmethode",
    L"Weg vorzeichnen",
    L"Fehlsch??sse anzeigen",
    L"Best??tigung bei Echtzeit",
    L"Schlaf-/Wachmeldung anzeigen",
    L"Metrisches System benutzen",
    L"Boden beleuchten",
    L"Cursor autom. auf S??ldner",
    L"Cursor autom. auf T??ren",
    L"Gegenst??nde leuchten",
    L"Baumkronen zeigen",
    L"Drahtgitter zeigen",
    L"3D Cursor zeigen",
};

// This is the help text associated with the above toggles.
STR16 zOptionsScreenHelpText[] = {
    // speech
    L"Mit dieser Option h??ren Sie die Dialoge.",

    // Mute Confirmation
    L"Schaltet die gesprochenen Best??tigungen an oder aus.",

    // Subtitles
    L"Schaltet Untertitel f??r Dialoge ein oder aus.",

    // Key to advance speech
    L"Wenn Untertitel AN sind, hat man durch diese Option Zeit, Dialoge von NPCs zu lesen.",

    // Toggle smoke animation
    L"Schalten Sie diese Option ab, wenn animierter Rauch Ihre Bildwiederholrate verlangsamt.",

    // Blood n Gore
    L"Diese Option abschalten, wenn Sie kein Blut sehen k??nnen. ",

    // Never move my mouse
    L"Wenn Sie diese Option abstellen, wird der Mauszeiger nicht mehr von den Popup-Fenstern "
    L"verdeckt.",

    // Old selection method
    L"Mit dieser Option funktioniert die Auswahl der S??ldner so wie in fr??heren JAGGED "
    L"ALLIANCE-Spielen (also genau andersherum als jetzt).",

    // Show movement path
    L"Diese Funktion ANschalten, um die geplanten Wege der S??ldner in Echtzeit anzuzeigen\n(oder "
    L"abgeschaltet lassen und bei gew??nschter Anzeige die SHIFT-Taste dr??cken).",

    // show misses
    L"Mit dieser Option zeigt Ihnen das Spiel, wo Ihre Kugeln hinfliegen, wenn Sie \"nicht "
    L"treffen\".",

    // Real Time Confirmation
    L"Durch diese Option wird vor der R??ckkehr in den Echtzeit-Modus ein zus??tzlicher "
    L"\"Sicherheits\"-Klick verlangt.",

    // Sleep/Wake notification
    L"Wenn ANgeschaltet werden Sie informiert, wann Ihre S??ldner, die sich im \"Dienst\" befinden "
    L"schlafen oder die Arbeit wieder aufnehmen.",

    // Use the metric system
    L"Mit dieser Option wird im Spiel das metrische System verwendet.",

    // Merc Lighted movement
    L"Diese Funktion ANschalten, wenn der S??ldner beim Gehen den Boden beleuchten soll. "
    L"AUSgeschaltet erh??ht sich die Bildwiederholrate.",

    // Smart cursor
    L"Wenn diese Funktion aktiviert ist, werden S??ldner automatisch hervorgehoben, sobald der "
    L"Cursor in ihrer N??he ist.",

    // snap cursor to the door
    L"Wenn diese Funktion aktiviert ist, springt der Cursor automatisch auf eine T??r, sobald eine "
    L"in der N??he ist.",

    // glow items
    L"Angeschaltet bekommen Gegenst??nde einen pulsierenden Rahmen(|I)",

    // toggle tree tops
    L"ANgeschaltet, werden die Baumkronen gezeigt (|T).",

    // toggle wireframe
    L"ANgeschaltet, werden Drahtgitter f??r verborgene W??nde gezeigt (|W).",

    L"ANgeschaltet, wird der Bewegungs-Cursor in 3D angezeigt. ( |Home )",

};

STR16 gzGIOScreenText[] = {
    L"GRUNDEINSTELLUNGEN",
    L"Spielmodus",
    L"Realistisch",
    L"SciFi",
    L"Waffen",
    L"Zus??tzliche Waffen",
    L"Normal",
    L"Schwierigkeitsgrad",
    L"Einsteiger",
    L"Profi",
    L"Alter Hase",
    L"Ok",
    L"Abbrechen",
    L"Extraschwer",
    L"Ohne Zeitlimit",
    L"Mit Zeitlimit",
    L"Option nicht verf??gbar",
};

STR16 pDeliveryLocationStrings[] = {
    L"Austin",       // Austin, Texas, USA
    L"Bagdad",       // Baghdad, Iraq (Suddam Hussein's home)
    L"Drassen",      // The main place in JA2 that you can receive items. The other towns are dummy
                     // names...
    L"Hong Kong",    // Hong Kong, Hong Kong
    L"Beirut",       // Beirut, Lebanon	(Middle East)
    L"London",       // London, England
    L"Los Angeles",  // Los Angeles, California, USA (SW corner of USA)
    L"Meduna",       // Meduna -- the other airport in JA2 that you can receive items.
    L"Metavira",     // The island of Metavira was the fictional location used by JA1
    L"Miami",        // Miami, Florida, USA (SE corner of USA)
    L"Moskau",       // Moscow, USSR
    L"New York",     // New York, New York, USA
    L"Ottawa",       // Ottawa, Ontario, Canada -- where JA2 was made!
    L"Paris",        // Paris, France
    L"Tripolis",     // Tripoli, Libya (eastern Mediterranean)
    L"Tokio",        // Tokyo, Japan
    L"Vancouver",    // Vancouver, British Columbia, Canada (west coast near US border)
};

STR16 pSkillAtZeroWarning[] = {
    // This string is used in the IMP character generation. It is possible to select 0 ability
    // in a skill meaning you can't use it. This text is confirmation to the player.
    L"Sind Sie sicher? Ein Wert von 0 bedeutet, da?? der Charakter diese F??higkeit nicht nutzen "
    L"kann.",
};

STR16 pIMPBeginScreenStrings[] = {
    L"(max. 8 Buchstaben)",
};

STR16 pIMPFinishButtonText[] = {
    L"Analyse wird durchgef??hrt",
};

STR16 pIMPFinishStrings[] = {
    L"Danke, %s",  //%s is the name of the merc
};

// the strings for imp voices screen
STR16 pIMPVoicesStrings[] = {
    L"Stimme",
};

STR16 pDepartedMercPortraitStrings[] = {
    L"Im Einsatz get??tet",
    L"Entlassen",
    L"Sonstiges",
};

// title for program
STR16 pPersTitleText[] = {
    L"S??ldner-Manager",
};

// paused game strings
STR16 pPausedGameText[] = {
    L"Pause",
    L"Zur??ck zum Spiel (|P|a|u|s|e)",
    L"Pause (|P|a|u|s|e)",
};

STR16 pMessageStrings[] = {
    L"Spiel beenden?",
    L"OK",
    L"JA",
    L"NEIN",
    L"ABBRECHEN",
    L"ZUR??CK",
    L"L??GEN",
    L"Keine Beschreibung",  // Save slots that don't have a description.
    L"Spiel gespeichert",
    L"Spiel gespeichert",
    L"QuickSave",   // The name of the quicksave file (filename, text reference)
    L"Spielstand",  // The name of the normal savegame file, such as SaveGame01, SaveGame02, etc.
    L"sav",         // The 3 character dos extension (represents sav)
    L"..\\Spielst??nde",  // The name of the directory where games are saved.
    L"Tag",
    L"S??ldner",
    L"Leerer Slot",       // An empty save game slot
    L"Demo",              // Demo of JA2
    L"Debug",             // State of development of a project (JA2) that is a debug build
    L"Ver??ffentlichung",  // Release build for JA2
    L"KpM",   // Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
    L"min",   // Abbreviation for minute.
    L"m",     // One character abbreviation for meter (metric distance measurement unit).
    L"Kgln",  // Abbreviation for rounds (# of bullets)
    L"kg",    // Abbreviation for kilogram (metric weight measurement unit)
    L"Pfd",   // Abbreviation for pounds (Imperial weight measurement unit)
    L"Home",  // Home as in homepage on the internet.
    L"US$",   // Abbreviation for US Dollars
    L"n.a",   // Lowercase acronym for not applicable.
    L"Inzwischen",                        // Meanwhile
    L"%s ist angekommen im Sektor %s%s",  // Name/Squad has arrived in sector A9. Order must not
                                          // change without notifying SirTech
    L"Version",
    L"Leerer Quick-Save-Slot",
    L"Dieser Slot ist nur f??r Quick-Saves aus den Map Screens und dem Taktik-Bildschirm. Speichern "
    L"mit ALT+S",
    L"offen",
    L"zu",
    L"Ihr Festplattenspeicher ist knapp. Sie haben lediglich %sMB frei und Jagged Alliance 2 "
    L"ben??tigt %sMB.",
    L"%s von AIM angeheuert",
    L"%s hat %s gefangen.",  //'Merc name' has caught 'item' -- let SirTech know if name comes after
                             // item.

    L"%s hat die Droge genommen.",              //'Merc name' has taken the drug
    L"%s hat keine medizinischen F??higkeiten",  //'Merc name' has no medical skill.

    // CDRom errors (such as ejecting CD while attempting to read the CD)
    L"Die Integrit??t des Spieles wurde besch??digt.",  // The integrity of the game has been
                                                      // compromised
    L"FEHLER: CD-ROM-Laufwerk schlie??en",

    // When firing heavier weapons in close quarters, you may not have enough room to do so.
    L"Kein Platz, um von hier aus zu feuern.",

    // Can't change stance due to objects in the way...
    L"Kann seine Position jetzt nicht ??ndern.",

    // Simple text indications that appear in the game, when the merc can do one of these things.
    L"Ablegen",
    L"Werfen",
    L"Weitergeben",

    L"%s weitergegeben an %s.",  //"Item" passed to "merc". Please try to keep the item %s before
                                 // the merc %s, otherwise, must notify SirTech.
    L"Kein Platz, um %s an %s weiterzugeben.",  // pass "item" to "merc". Same instructions as
                                                // above.

    // A list of attachments appear after the items. Ex: Kevlar vest ( Ceramic Plate 'Attached )'
    L" angebracht )",

    // Cheat modes
    L"Cheat-Level EINS erreicht",
    L"Cheat-Level ZWEI erreicht",

    // Toggling various stealth modes
    L"Stealth Mode f??r Trupp ein.",
    L"Stealth Mode f??r Trupp aus.",
    L"Stealth Mode f??r %s ein.",
    L"Stealth Mode f??r %s aus.",

    // Wireframes are shown through buildings to reveal doors and windows that can't otherwise be
    // seen in an isometric engine. You can toggle this mode freely in the game.
    L"Drahtgitter ein",
    L"Drahtgitter aus",

    // These are used in the cheat modes for changing levels in the game. Going from a basement
    // level to an upper level, etc.
    L"Von dieser Ebene geht es nicht nach oben...",
    L"Noch tiefere Ebenen gibt es nicht...",
    L"Gew??lbeebene %d betreten...",
    L"Gew??lbe verlassen...",

    L"s",  // used in the shop keeper inteface to mark the ownership of the item eg Red's gun
    L"Autoscrolling AUS.",
    L"Autoscrolling AN.",
    L"3D-Cursor AUS.",
    L"3D-Cursor AN.",
    L"Trupp %d aktiv.",
    L"Sie k??nnen %ss Tagessold von %s nicht zahlen",  // first %s is the mercs name, the second is a
                                                      // string containing the salary
    L"Abbruch",
    L"%s kann alleine nicht gehen.",
    L"Spielstand namens Spielstand99.sav kreiert. Wenn n??tig, in Spielstand01 - Spielstand10 "
    L"umbennen und ??ber die Option 'Laden' aufrufen.",
    L"%s hat %s getrunken.",
    L"Paket in Drassen angekommen.",
    L"%s kommt am %d. um ca. %s am Zielort an (Sektor %s).",  // first %s is mercs name(OK), next is
                                                              // the sector location and name where
                                                              // they will be arriving in, lastely
                                                              // is the day an the time of arrival
                                                              // !!!7 It should be like this: first
                                                              // one is merc (OK), next is day of
                                                              // arrival (OK) , next is time of the
                                                              // day for ex. 07:00 (not OK, now it
                                                              // is still sector), next should be
                                                              // sector (not OK, now it is still
                                                              // time of the day)
    L"Logbuch aktualisiert.",
#ifdef JA2BETAVERSION
    L"Spiel erfolgreich in Slot End Turn Auto Save gespeichert.",
#endif
};

UINT16 ItemPickupHelpPopup[][40] = {
    L"OK", L"Hochscrollen", L"Alle ausw??hlen", L"Runterscrollen", L"Abbrechen",
};

STR16 pDoctorWarningString[] = {
    L"%s ist nicht nahe genug, um geheilt zu werden",
    L"Ihre Mediziner haben noch nicht alle verbinden k??nnen.",
};

STR16 pMilitiaButtonsHelpText[] = {
    L"Gr??ne Miliz aufnehmen(Rechtsklick)/absetzen(Linksklick)",  // button help text informing
                                                                 // player they can pick up or drop
                                                                 // militia with this button
    L"Regul??re Milizen aufnehmen(Rechtsklick)/absetzen(Linksklick)",
    L"Elitemilizen aufnehmen(Rechtsklick)/absetzen(Linksklick)",
    L"Milizen gleichm????ig ??ber alle Sektoren verteilen",
};

STR16 pMapScreenJustStartedHelpText[] = {
    L"Zu AIM gehen und S??ldner anheuern ( *Tip*: Befindet sich im Laptop )",  // to inform the
                                                                              // player to hired
                                                                              // some mercs to get
                                                                              // things going
    L"Sobald Sie f??r die Reise nach Arulco bereit sind, klicken Sie auf den Zeitraffer-Button "
    L"unten rechts auf dem Bildschirm.",  // to inform the player to hit time compression to get the
                                          // game underway
};

STR16 pAntiHackerString[] = {
    L"Fehler. Fehlende oder fehlerhafte Datei(en). Spiel wird beendet.",
};

STR16 gzLaptopHelpText[] = {
    // Buttons:
    L"E-Mail einsehen",
    L"Websites durchbl??ttern",
    L"Dateien und Anlagen einsehen",
    L"Logbuch lesen",
    L"Team-Info einsehen",
    L"Finanzen und Notizen einsehen",

    L"Laptop schlie??en",

    // Bottom task bar icons (if they exist):
    L"Sie haben neue Mail",
    L"Sie haben neue Dateien",

    // Bookmarks:
    L"Association of International Mercenaries",
    L"Bobby Rays Online-Waffenversand",
    L"Bundesinstitut f??r S??ldnerevaluierung",
    L"More Economic Recruiting Center",
    L"McGillicuttys Bestattungen",
    L"Fleuropa",
    L"Versicherungsmakler f??r A.I.M.-Vertr??ge",
};

STR16 gzHelpScreenText[] = {
    L"Helpscreen verlassen",
};

STR16 gzNonPersistantPBIText[] = {
    L"Es tobt eine Schlacht. Sie k??nnen sich nur im Taktikbildschirm zur??ckziehen.",
    L"Sektor betreten und Kampf fortsetzen (|E).",
    L"Kampf durch PC entscheiden (|A).",
    L"Sie k??nnen den Kampf nicht vom PC entscheiden lassen, wenn Sie angreifen.",
    L"Sie k??nnen den Kampf nicht vom PC entscheiden lassen, wenn Sie in einem Hinterhalt sind.",
    L"Sie k??nnen den Kampf nicht vom PC entscheiden lassen, wenn Sie gegen Monster k??mpfen.",
    L"Sie k??nnen den Kampf nicht vom PC entscheiden lassen, wenn feindliche Zivilisten da sind.",
    L"Sie k??nnen einen Kampf nicht vom PC entscheiden lassen, wenn Bloodcats da sind.",
    L"KAMPF IM GANGE",
    L"Sie k??nnen sich nicht zur??ckziehen, wenn Sie in einem Hinterhalt sind.",
};

STR16 gzMiscString[] = {
    L"Ihre Milizen k??mpfen ohne die Hilfe der S??ldner weiter...",
    L"Das Fahrzeug mu?? nicht mehr aufgetankt werden.",
    L"Der Tank ist %d%% voll.",
    L"Deidrannas Armee hat wieder volle Kontrolle ??ber %s.",
    L"Sie haben ein Tanklager verloren.",
};

STR16 gzIntroScreen[] = {
    L"Kann Introvideo nicht finden",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
STR16 pNewNoiseStr[] = {
    // There really isn't any difference between using "coming from" or "to".
    // For the explosion case the string in English could be either:
    //	L"Gus hears a loud EXPLOSION 'to' the north.",
    //	L"Gus hears a loud EXPLOSION 'coming from' the north.",
    // For certain idioms, it sounds better to use one over the other. It is a matter of preference.
    L"%s h??rt %s aus dem %s.",
    L"%s h??rt eine BEWEGUNG (%s) von %s.",
    L"%s h??rt ein KNARREN (%s) von %s.",
    L"%s h??rt ein KLATSCHEN (%s) von %s.",
    L"%s h??rt einen AUFSCHLAG (%s) von %s.",
    L"%s h??rt eine EXPLOSION (%s) von %s.",
    L"%s h??rt einen SCHREI (%s) von %s.",
    L"%s h??rt einen AUFSCHLAG (%s) von %s.",
    L"%s h??rt einen AUFSCHLAG (%s) von %s.",
    L"%s h??rt ein ZERBRECHEN (%s) von %s.",
    L"%s h??rt ein ZERSCHMETTERN (%s) von %s.",
};

STR16 wMapScreenSortButtonHelpText[] = {
    L"Sort. nach Name (|F|1)", L"Sort. nach Auftrag (|F|2)", L"Sort. nach wach/schlafend (|F|3)",
    L"Sort. nach Ort (|F|4)",  L"Sort. nach Ziel (|F|5)",    L"Sort. nach Vertragsende (|F|6)",
};

STR16 BrokenLinkText[] = {
    L"Error 404",
    L"Site nicht gefunden.",
};

STR16 gzBobbyRShipmentText[] = {
    L"Letzte Lieferungen",
    L"Bestellung #",
    L"Artikelanzahl",
    L"Bestellt am",
};

STR16 gzCreditNames[] = {
    L"Chris Camfield",
    L"Shaun Lyng",
    L"Kris M??rnes",
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

STR16 gzCreditNameTitle[] = {
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

STR16 gzCreditNameFunny[] = {
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

STR16 sRepairsDoneString[] = {
    L"%s hat  seine eigenen Gegenst??nde repariert",
    L"%s hat die Waffen und R??stungen aller Teammitglieder repariert",
    L"%s hat die aktivierten Gegenst??nde aller Teammitglieder repariert",
    L"%s hat die mitgef??hrten Gegenst??nde aller Teammitglieder repariert",
};

STR16 zGioDifConfirmText[] = {
    L"Sie haben sich f??r den EINSTEIGER-Modus entschieden. Dies ist die passende Einstellung f??r "
    L"Spieler, die noch nie zuvor Jagged Alliance oder ??hnliche Spiele gespielt haben oder f??r "
    L"Spieler, die sich ganz einfach k??rzere Schlachten w??nschen. Ihre Wahl wird den Verlauf des "
    L"ganzen Spiels beeinflussen. Treffen Sie also eine sorgf??ltige Wahl. Sind Sie ganz sicher, "
    L"da?? Sie im Einsteiger-Modus spielen wollen?",
    L"Sie haben sich f??r den FORTGESCHRITTENEN-Modus entschieden. Dies ist die passende "
    L"Einstellung f??r Spieler, die bereits Erfahrung mit Jagged Alliance oder ??hnlichen Spielen "
    L"haben. Ihre Wahl wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine "
    L"sorgf??ltige Wahl. Sind Sie ganz sicher, da?? Sie im Fortgeschrittenen-Modus spielen wollen?",
    L"Sie haben sich f??r den PROFI-Modus entschieden. Na gut, wir haben Sie gewarnt. Machen Sie "
    L"hinterher blo?? nicht uns daf??r verantwortlich, wenn Sie im Sarg nach Hause kommen. Ihre Wahl "
    L"wird den Verlauf des ganzen Spiels beeinflussen. Treffen Sie also eine sorgf??ltige Wahl. "
    L"Sind Sie ganz sicher, da?? Sie im Profi-Modus spielen wollen?",
};

STR16 gzLateLocalizedString[] = {
    L"%S Loadscreen-Daten nicht gefunden...",

    // 1-5
    L"Der Roboter kann diesen Sektor nicht verlassen, wenn niemand die Fernbedienung benutzt.",

    L"Sie k??nnen den Zeitraffer jetzt nicht benutzen. Warten Sie das Feuerwerk ab!",
    L"%s will sich nicht bewegen.",
    L"%s hat nicht genug Energie, um die Position zu ??ndern.",
    L"%s hat kein Benzin mehr und steckt in %c%d fest.",

    // 6-10

    // the following two strings are combined with the strings below to report noises
    // heard above or below the merc
    L"oben",
    L"unten",

    // The following strings are used in autoresolve for autobandaging related feedback.
    L"Keiner der S??ldner hat medizinische F??higkeiten.",
    L"Sie haben kein Verbandszeug.",
    L"Sie haben nicht genug Verbandszeug, um alle zu verarzten.",
    L"Keiner der S??ldner mu?? verbunden werden.",
    L"S??ldner automatisch verbinden.",
    L"Alle S??ldner verarztet.",

    // 14-16
    L"Arulco",
    L"(Dach)",
    L"Gesundheit: %d/%d",

    // 17
    // In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
    //"vs." is the abbreviation of versus.
    L"%d gegen %d",

    // 18-19
    L"%s ist voll!",  //(ex "The ice cream truck is full")
    L"%s braucht nicht eine schnelle Erste Hilfe, sondern eine richtige medizinische Betreuung "
    L"und/oder Erholung.",

    // 20
    // Happens when you get shot in the legs, and you fall down.
    L"%s ist am Bein getroffen und hingefallen!",
    // Name can't speak right now.
    L"%s kann gerade nicht sprechen.",

    // 22-24 plural versions
    L"%d gr??ne Milizen wurden zu Elitemilizen bef??rdert.",
    L"%d gr??ne Milizen wurden zu regul??ren Milizen bef??rdert.",
    L"%d regul??re Milizen wurde zu Elitemilizen bef??rdert.",

    // 25
    L"Schalter",

    // 26
    // Name has gone psycho -- when the game forces the player into burstmode (certain unstable
    // characters)
    L"%s dreht durch!",

    // 27-28
    // Messages why a player can't time compress.
    L"Es ist momentan gef??hrlich den Zeitraffer zu bet??tigen, da Sie noch S??ldner in Sektor %s "
    L"haben.",
    L"Es ist gef??hrlich den Zeitraffer zu bet??tigen, wenn Sie noch S??ldner in den von Monstern "
    L"verseuchten Minen haben.",

    // 29-31 singular versions
    L"1 gr??ne Miliz wurde zur Elitemiliz bef??rdert.",
    L"1 gr??ne Miliz wurde zur regul??ren Miliz bef??rdert.",
    L"1 regul??re Miliz wurde zur Elitemiliz bef??rdert.",

    // 32-34
    L"%s sagt ??berhaupt nichts.",
    L"Zur Oberfl??che gehen?",
    L"(Trupp %d)",

    // 35
    L"%s reparierte %ss %s",

    // 36
    L"BLOODCAT",

    // 37-38 "Name trips and falls"
    L"%s stolpert und st??rzt",
    L"Dieser Gegenstand kann von hier aus nicht aufgehoben werden.",

    // 39
    L"Keiner Ihrer ??brigen S??ldner ist in der Lage zu k??mpfen. Die Miliz wird die Monster alleine "
    L"bek??mpfen",

    // 40-43
    //%s is the name of merc.
    L"%s hat keinen Erste-Hilfe-Kasten mehr!",
    L"%s hat nicht das geringste Talent jemanden zu verarzten!",
    L"%s hat keinen Werkzeugkasten mehr!",
    L"%s ist absolut unf??hig dazu, irgend etwas zu reparieren!",

    // 44
    L"Repar. Zeit",
    L"%s kann diese Person nicht sehen.",

    // 46-48
    L"%ss Gewehrlauf-Verl??ngerung f??llt ab!",
    L"Pro Sektor sind nicht mehr als %d Milizausbilder erlaubt.",
    L"Sind Sie sicher?",  //

    // 49-50
    L"Zeitraffer",  // time compression
    L"Der Fahrzeugtank ist jetzt voll.",

    // 51-52 Fast help text in mapscreen.
    L"Zeitraffer fortsetzen (|S|p|a|c|e)",
    L"Zeitraffer anhalten (|E|s|c)",

    // 53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
    L"%s hat die Ladehemmung der %s behoben",
    L"%s hat die Ladehemmung von %ss %s behoben",

    // 55
    L"Die Zeit kann nicht komprimiert werden w??hrend das Sektorinventar eingesehen wird.",

    L"Die Jagged Alliance 2 PLAY CD wurde nicht gefunden. Das Programm wird jetzt beendet.",

    // L"Im Sektor sind Feinde entdeckt worden",		//STR_DETECTED_SIMULTANEOUS_ARRIVAL

    L"Die Gegenst??nde wurden erfolgreich miteinander kombiniert.",

    // 58
    // Displayed with the version information when cheats are enabled.
    L"Aktueller/Max. Fortschritt: %d%%/%d%%",

    // 59
    L"John und Mary eskortieren?",

    L"Switch Activated.",

};

#endif  // GERMAN
