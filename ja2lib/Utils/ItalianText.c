// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/Text.h"

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
static wchar_t* it_AmmoCaliber[] = {
    L"0",      L"cal .38", L"9 mm",    L"cal .45",      L"cal .357",   L"cal fisso 12",
    L"CAW",    L"5.45 mm", L"5.56 mm", L"7.62 mm NATO", L"7.62 mm WP", L"4.7 mm",
    L"5.7 mm", L"Mostro",  L"Missile",
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
static wchar_t* it_BobbyRayAmmoCaliber[] = {
    L"0",      L"cal .38", L"9 mm",    L"cal .45",    L"cal .357",   L"cal fisso 12",
    L"CAWS",   L"5.45 mm", L"5.56 mm", L"7.62 mm N.", L"7.62 mm WP", L"4.7 mm",
    L"5.7 mm", L"Mostro",  L"Missile",
    L"",  // dart
};

static wchar_t* it_WeaponType[] = {
    L"Altro",
    L"Arma",
    L"Mitragliatrice",
    L"Mitra",
    L"Fucile",
    L"Fucile del cecchino",
    L"Fucile d'assalto",
    L"Mitragliatrice leggera",
    L"Fucile a canne mozze",
};

static wchar_t* it_TeamTurnString[] = {
    L"Turno del giocatore",  // player's turn
    L"Turno degli avversari", L"Turno delle creature", L"Turno dell'esercito", L"Turno dei civili",
    // planning turn
};

static wchar_t* it_Message[] = {
    L"",

    // In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

    L"%s è stato colpito alla testa e perde un punto di saggezza!",
    L"%s è stato colpito alla spalla e perde un punto di destrezza!",
    L"%s è stato colpito al torace e perde un punto di forza!",
    L"%s è stato colpito alle gambe e perde un punto di agilità!",
    L"%s è stato colpito alla testa e perde %d punti di saggezza!",
    L"%s è stato colpito alle palle perde %d punti di destrezza!",
    L"%s è stato colpito al torace e perde %d punti di forza!",
    L"%s è stato colpito alle gambe e perde %d punti di agilità!",
    L"Interrompete!",

    // The first %s is a merc's name, the second is a string from pNoiseVolStr,
    // the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

    L"",  // OBSOLETE
    L"I vostri rinforzi sono arrivati!",

    // In the following four lines, all %s's are merc names

    L"%s ricarica.",
    L"%s non ha abbastanza Punti Azione!",
    L"%s ricorre al pronto soccorso. (Premete un tasto per annullare.)",
    L"%s e %s ricorrono al pronto soccorso. (Premete un tasto per annullare.)",
    // the following 17 strings are used to create lists of gun advantages and disadvantages
    // (separated by commas)
    L"affidabile",
    L"non affidabile",
    L"facile da riparare",
    L"difficile da riparare",
    L"danno grave",
    L"danno lieve",
    L"fuoco veloce",
    L"fuoco",
    L"raggio lungo",
    L"raggio corto",
    L"leggero",
    L"pesante",
    L"piccolo",
    L"fuoco a raffica",
    L"niente raffiche",
    L"grande deposito d'armi",
    L"piccolo deposito d'armi",

    // In the following two lines, all %s's are merc names

    L"Il travestimento di %s è stato scoperto.",
    L"Il travestimento di %s è stato scoperto.",

    // The first %s is a merc name and the second %s is an item name

    L"La seconda arma è priva di munizioni!",
    L"%s ha rubato il %s.",

    // The %s is a merc name

    L"L'arma di %s non può più sparare a raffica.",

    L"Ne avete appena ricevuto uno di quelli attaccati.",
    L"Volete combinare gli oggetti?",

    // Both %s's are item names

    L"Non potete attaccare %s a un %s.",

    L"Nessuno",
    L"Espelli munizioni",
    L"Attaccare",

    // You cannot use "item(s)" and your "other item" at the same time.
    // Ex:  You cannot use sun goggles and you gas mask at the same time.
    L"Non potete usare %s e il vostro %s contemporaneamente.",

    L"L'oggetto puntato dal vostro cursore può essere combinato ad alcuni oggetti ponendolo in uno "
    L"dei quattro slot predisposti.",
    L"L'oggetto puntato dal vostro cursore può essere combinato ad alcuni oggetti ponendolo in uno "
    L"dei quattro slot predisposti. (Comunque, in questo caso, l'oggetto non è compatibile.)",
    L"Il settore non è libero da nemici!",
    L"Vi dovete ancora dare %s %s",
    L"%s è stato colpito alla testa!",
    L"Abbandonate la battaglia?",
    L"Questo attaco sarà definitivo. Andate avanti?",
    L"%s si sente molto rinvigorito!",
    L"%s ha dormito di sasso!",
    L"%s non è riuscito a catturare il %s!",
    L"%s ha riparato il %s",
    L"Interrompete per ",
    L"Vi arrendete?",
    L"Questa persona rifiuta il vostro aiuto.",
    L"NON sono d'accordo!",
    L"Per viaggiare sull'elicottero di Skyrider, dovrete innanzitutto ASSEGNARE mercenari al "
    L"VEICOLO/ELICOTTERO.",
    L"solo %s aveva abbastanza tempo per ricaricare UNA pistola",
    L"Turno dei Bloodcat",
};

// the names of the towns in the game

static wchar_t* it_pTownNames[] = {
    L"",         L"Omerta", L"Drassen", L"Alma",   L"Grumm",  L"Tixa",     L"Cambria",
    L"San Mona", L"Estoni", L"Orta",    L"Balime", L"Meduna", L"Chitzena",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per
// second, etc. min is an abbreviation for minutes

static wchar_t* it_sTimeStrings[] = {
    L"Fermo", L"Normale", L"5 min", L"30 min", L"60 min", L"6 ore",
};

// Assignment Strings: what assignment does the merc  have right now? For example, are they on a
// squad, training, administering medical aid (doctor) or training a town. All are abbreviated. 8
// letters is the longest it can be.

static wchar_t* it_pAssignmentStrings[] = {
    L"Squad. 1",  L"Squad. 2",  L"Squad. 3",  L"Squad. 4",  L"Squad. 5",
    L"Squad. 6",  L"Squad. 7",  L"Squad. 8",  L"Squad. 9",  L"Squad. 10",
    L"Squad. 11", L"Squad. 12", L"Squad. 13", L"Squad. 14", L"Squad. 15",
    L"Squad. 16", L"Squad. 17", L"Squad. 18", L"Squad. 19", L"Squad. 20",
    L"Servizio",  // on active duty
    L"Dottore",   // administering medical aid
    L"Paziente",  // getting medical aid
    L"Veicolo",   // in a vehicle
    L"Transito",  // in transit - abbreviated form
    L"Riparare",  // repairing
    L"Esercit.",  // training themselves
    L"Esercit.",  // training a town to revolt
    L"Istrutt.",  // training a teammate
    L"Studente",  // being trained by someone else
    L"Morto",     // dead
    L"Incap.",    // abbreviation for incapacitated
    L"PDG",       // Prisoner of war - captured
    L"Ospedale",  // patient in a hospital
    L"Vuoto",     // Vehicle is empty
};

static wchar_t* it_pMilitiaString[] = {
    L"Esercito",        // the title of the militia box
    L"Non incaricato",  // the number of unassigned militia troops
    L"Non potete ridistribuire reclute, se ci sono nemici nei paraggi!",
};

static wchar_t* it_pMilitiaButtonString[] = {
    L"Auto",      // auto place the militia troops for the player
    L"Eseguito",  // done placing militia troops
};

static wchar_t* it_pConditionStrings[] = {
    L"Eccellente",  // the state of a soldier .. excellent health
    L"Buono",       // good health
    L"Discreto",    // fair health
    L"Ferito",      // wounded health
    L"Stanco",      // tired
    L"Grave",       // bleeding to death
    L"Svenuto",     // knocked out
    L"Morente",     // near death
    L"Morto",       // dead
};

static wchar_t* it_pEpcMenuStrings[] = {
    L"In servizio",   // set merc on active duty
    L"Paziente",      // set as a patient to receive medical aid
    L"Veicolo",       // tell merc to enter vehicle
    L"Non scortato",  // let the escorted character go off on their own
    L"Cancella",      // close this menu
};

// look at pAssignmentString above for comments

static wchar_t* it_pPersonnelAssignmentStrings[] = {
    L"Squadra 1",   L"Squadra 2",  L"Squadra 3",   L"Squadra 4",
    L"Squadra 5",   L"Squadra 6",  L"Squadra 7",   L"Squadra 8",
    L"Squadra 9",   L"Squadra 10", L"Squadra 11",  L"Squadra 12",
    L"Squadra 13",  L"Squadra 14", L"Squadra 15",  L"Squadra 16",
    L"Squadra 17",  L"Squadra 18", L"Squadra 19",  L"Squadra 20",
    L"In servizio", L"Dottore",    L"Paziente",    L"veicolo",
    L"In transito", L"Riparare",   L"Esercitarsi", L"Allenamento Esercito",
    L"Allenatore",  L"Studente",   L"Morto",       L"Incap.",
    L"PDG",         L"Ospedale",
    L"Vuoto",  // Vehicle is empty
};

// refer to above for comments

static wchar_t* it_pLongAssignmentStrings[] = {
    L"Squadra 1",
    L"Squadra 2",
    L"Squadra 3",
    L"Squadra 4",
    L"Squadra 5",
    L"Squadra 6",
    L"Squadra 7",
    L"Squadra 8",
    L"Squadra 9",
    L"Squadra 10",
    L"Squadra 11",
    L"Squadra 12",
    L"Squadra 13",
    L"Squadra 14",
    L"Squadra 15",
    L"Squadra 16",
    L"Squadra 17",
    L"Squadra 18",
    L"Squadra 19",
    L"Squadra 20",
    L"In servizio",
    L"Dottore",
    L"Paziente",
    L"Veicolo",
    L"In transito",
    L"Ripara",
    L"Esercitarsi",
    L"Allenatore esercito",
    L"Allena squadra",
    L"Studente",
    L"Morto",
    L"Incap.",
    L"PDG",
    L"Ospedale",  // patient in a hospital
    L"Vuoto",     // Vehicle is empty
};

// the contract options

static wchar_t* it_pContractStrings[] = {
    L"Opzioni del contratto:",
    L"",                   // a blank line, required
    L"Offri 1 giorno",     // offer merc a one day contract extension
    L"Offri 1 settimana",  // 1 week
    L"Offri 2 settimane",  // 2 week
    L"Termina contratto",  // end merc's contract
    L"Annulla",            // stop showing this menu
};

static wchar_t* it_pPOWStrings[] = {
    L"PDG",  // an acronym for Prisoner of War
    L"??",
};

static wchar_t* it_pLongAttributeStrings[] = {
    L"FORZA",        L"DESTREZZA", L"AGILITÀ", L"SAGGEZZA",  L"MIRA",
    L"PRONTO SOCC.", L"MECCANICA", L"COMANDO", L"ESPLOSIVI", L"LIVELLO",
};

static wchar_t* it_pInvPanelTitleStrings[] = {
    L"Giubb. A-P",  // the armor rating of the merc
    L"Peso",        // the weight the merc is carrying
    L"Trav.",       // the merc's camouflage rating
};

static wchar_t* it_pShortAttributeStrings[] = {
    L"Abi",  // the abbreviated version of : agility
    L"Des",  // dexterity
    L"For",  // strength
    L"Com",  // leadership
    L"Sag",  // wisdom
    L"Liv",  // experience level
    L"Tir",  // marksmanship skill
    L"Esp",  // explosive skill
    L"Mec",  // mechanical skill
    L"PS",   // medical skill
};

static wchar_t* it_pUpperLeftMapScreenStrings[] = {
    L"Compito",  // the mercs current assignment
    L"Accordo",  // the contract info about the merc
    L"Salute",   // the health level of the current merc
    L"Morale",   // the morale of the current merc
    L"Cond.",    // the condition of the current vehicle
    L"Benzina",  // the fuel level of the current vehicle
};

static wchar_t* it_pTrainingStrings[] = {
    L"Esercitarsi",  // tell merc to train self
    L"Esercito",     // tell merc to train town
    L"Allenatore",   // tell merc to act as trainer
    L"Studente",     // tell merc to be train by other
};

static wchar_t* it_pGuardMenuStrings[] = {
    L"Frequenza di fuoco:",   // the allowable rate of fire for a merc who is guarding
    L"Fuoco aggressivo",      // the merc can be aggressive in their choice of fire rates
    L"Conservare munizioni",  // conserve ammo
    L"Astenersi dal fuoco",   // fire only when the merc needs to
    L"Altre opzioni:",        // other options available to merc
    L"Può ritrattare",        // merc can retreat
    L"Può cercare rifugio",   // merc is allowed to seek cover
    L"Può assistere compagni di squadra",  // merc can assist teammates
    L"Fine",                               // done with this menu
    L"Annulla",                            // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by
// the player

static wchar_t* it_pOtherGuardMenuStrings[] = {
    L"Frequenza di fuoco:",
    L" *Fuoco aggressivo*",
    L" *Conservare munizioni*",
    L" *Astenersi dal fuoco*",
    L"Altre opzioni:",
    L" *Può ritrattare*",
    L" *Può cercare rifugio*",
    L" *Può assistere compagni di squadra*",
    L"Fine",
    L"Annulla",
};

static wchar_t* it_pAssignMenuStrings[] = {
    L"In servizio",  // merc is on active duty
    L"Dottore",      // the merc is acting as a doctor
    L"Paziente",     // the merc is receiving medical attention
    L"Veicolo",      // the merc is in a vehicle
    L"Ripara",       // the merc is repairing items
    L"Si esercita",  // the merc is training
    L"Annulla",      // cancel this menu
};

static wchar_t* it_pRemoveMercStrings[] = {
    L"Rimuovi Mercenario",  // remove dead merc from current team
    L"Annulla",
};

static wchar_t* it_pAttributeMenuStrings[] = {
    L"Forza",        L"Destrezza", L"Agilità", L"Salute",    L"Mira",
    L"Pronto socc.", L"Meccanica", L"Comando", L"Esplosivi", L"Annulla",
};

static wchar_t* it_pTrainingMenuStrings[] = {
    L"Allenati",    // train yourself
    L"Esercito",    // train the town
    L"Allenatore",  // train your teammates
    L"Studente",    // be trained by an instructor
    L"Annulla",     // cancel this menu
};

static wchar_t* it_pSquadMenuStrings[] = {
    L"Squadra  1", L"Squadra  2", L"Squadra  3", L"Squadra  4", L"Squadra  5", L"Squadra  6",
    L"Squadra  7", L"Squadra  8", L"Squadra  9", L"Squadra 10", L"Squadra 11", L"Squadra 12",
    L"Squadra 13", L"Squadra 14", L"Squadra 15", L"Squadra 16", L"Squadra 17", L"Squadra 18",
    L"Squadra 19", L"Squadra 20", L"Annulla",
};

static wchar_t* it_pPersonnelTitle[] = {
    L"Personale",  // the title for the personnel screen/program application
};

static wchar_t* it_pPersonnelScreenStrings[] = {
    L"Salute: ",  // health of merc
    L"Agilità: ",
    L"Destrezza: ",
    L"Forza: ",
    L"Comando: ",
    L"Saggezza: ",
    L"Liv. esp.: ",  // experience level
    L"Mira: ",
    L"Meccanica: ",
    L"Esplosivi: ",
    L"Pronto socc.: ",
    L"Deposito med.: ",             // amount of medical deposit put down on the merc
    L"Contratto in corso: ",        // cost of current contract
    L"Uccisi: ",                    // number of kills by merc
    L"Assistiti: ",                 // number of assists on kills by merc
    L"Costo giornaliero:",          // daily cost of merc
    L"Tot. costo fino a oggi:",     // total cost of merc
    L"Contratto:",                  // cost of current contract
    L"Tot. servizio fino a oggi:",  // total service rendered by merc
    L"Salario arretrato:",          // amount left on MERC merc to be paid
    L"Percentuale di colpi:",       // percentage of shots that hit target
    L"Battaglie:",                  // number of battles fought
    L"Numero ferite:",              // number of times merc has been wounded
    L"Destrezza:",
    L"Nessuna abilità",
};

// These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static wchar_t* it_gzMercSkillText[] = {
    L"Nessuna abilità",  L"Forzare serrature", L"Corpo a corpo",     L"Elettronica",
    L"Op. notturne",     L"Lanciare",          L"Istruire",          L"Armi pesanti",
    L"Armi automatiche", L"Clandestino",       L"Ambidestro",        L"Furtività",
    L"Arti marziali",    L"Coltelli",          L"Bonus per altezza", L"Camuffato",
    L"(Esperto)",
};

// This is pop up help text for the options that are available to the merc

static wchar_t* it_pTacticalPopupButtonStrings[] = {
    L"|Stare fermi/Camminare",
    L"|Accucciarsi/Muoversi accucciato",
    L"Stare fermi/|Correre",
    L"|Prono/Strisciare",
    L"|Guardare",
    L"Agire",
    L"Parlare",
    L"Esaminare (|C|t|r|l)",

    // Pop up door menu
    L"Aprire manualmente",
    L"Esaminare trappole",
    L"Grimaldello",
    L"Forzare",
    L"Liberare da trappole",
    L"Chiudere",
    L"Aprire",
    L"Usare esplosivo per porta",
    L"Usare piede di porco",
    L"Annulla (|E|s|c)",
    L"Chiudere",
};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static wchar_t* it_pDoorTrapStrings[] = {
    L"Nessuna trappola",
    L"una trappola esplosiva",
    L"una trappola elettrica",
    L"una trappola con sirena",
    L"una trappola con allarme insonoro",
};

// Contract Extension. These are used for the contract extension with AIM mercenaries.

static wchar_t* it_pContractExtendStrings[] = {
    L"giorno",
    L"settimana",
    L"due settimane",
};

// On the map screen, there are four columns. This text is popup help text that identifies the
// individual columns.

static wchar_t* it_pMapScreenMouseRegionHelpText[] = {
    L"Selezionare postazioni", L"Assegnare mercenario", L"Tracciare percorso di viaggio",
    L"Merc |Contratto",        L"Eliminare mercenario", L"Dormire",
};

// volumes of noises

static wchar_t* it_pNoiseVolStr[] = {
    L"DEBOLE",
    L"DEFINITO",
    L"FORTE",
    L"MOLTO FORTE",
};

// types of noises

static wchar_t* it_pNoiseTypeStr[] =  // OBSOLETE
    {
        L"SCONOSCIUTO",  L"rumore di MOVIMENTO",
        L"SCRICCHIOLIO", L"TONFO IN ACQUA",
        L"IMPATTO",      L"SPARO",
        L"ESPLOSIONE",   L"URLA",
        L"IMPATTO",      L"IMPATTO",
        L"FRASTUONO",    L"SCHIANTO",
};

// Directions that are used to report noises

static wchar_t* it_pDirectionStr[] = {
    L"il NORD-EST",  L"il EST",   L"il SUD-EST",    L"il SUD",
    L"il SUD-OVEST", L"il OVEST", L"il NORD-OVEST", L"il NORD",
};

// These are the different terrain types.

static wchar_t* it_pLandTypeStrings[] = {
    L"Urbano", L"Strada", L"Pianure", L"Deserto", L"Boschi", L"Foresta", L"Palude", L"Acqua",
    L"Colline", L"Impervio",
    L"Fiume",  // river from north to south
    L"Fiume",  // river from east to west
    L"Paese straniero",
    // NONE of the following are used for directional travel, just for the sector description.
    L"Tropicale", L"Campi", L"Pianure, strada", L"Boschi, strada", L"Fattoria, strada",
    L"Tropicale, strada", L"Foresta, strada", L"Linea costiera", L"Montagna, strada",
    L"Litoraneo, strada", L"Deserto, strada", L"Palude, strada", L"Boschi, postazione SAM",
    L"Deserto, postazione SAM", L"Tropicale, postazione SAM", L"Meduna, postazione SAM",

    // These are descriptions for special sectors
    L"Ospedale di Cambria", L"Aeroporto di Drassen", L"Aeroporto di Meduna", L"Postazione SAM",
    L"Nascondiglio ribelli",          // The rebel base underground in sector A10
    L"Prigione sotterranea di Tixa",  // The basement of the Tixa Prison (J9)
    L"Tana della creatura",           // Any mine sector with creatures in it
    L"Cantina di Orta",               // The basement of Orta (K4)
    L"Tunnel",                        // The tunnel access from the maze garden in Meduna
                                      // leading to the secret shelter underneath the palace
    L"Rifugio",                       // The shelter underneath the queen's palace
    L"",                              // Unused
};

static wchar_t* it_gpStrategicString[] = {
    L"",                                                                                 // Unused
    L"%s sono stati individuati nel settore %c%d e un'altra squadra sta per arrivare.",  // STR_DETECTED_SINGULAR
    L"%s sono stati individuati nel settore %c%d e un'altra squadra sta per arrivare.",  // STR_DETECTED_PLURAL
    L"Volete coordinare un attacco simultaneo?",  // STR_COORDINATE

    // Dialog strings for enemies.

    L"Il nemico offre la possibilità di arrendervi.",             // STR_ENEMY_SURRENDER_OFFER
    L"Il nemico ha catturato i vostri mercenari sopravvissuti.",  // STR_ENEMY_CAPTURED

    // The text that goes on the autoresolve buttons

    L"Ritirarsi",  // The retreat button				//STR_AR_RETREAT_BUTTON
    L"Fine",       // The done button				//STR_AR_DONE_BUTTON

    // The headers are for the autoresolve type (MUST BE UPPERCASE)

    L"DIFENDERE",   // STR_AR_DEFEND_HEADER
    L"ATTACCARE",   // STR_AR_ATTACK_HEADER
    L"INCONTRARE",  // STR_AR_ENCOUNTER_HEADER
    L"settore",     // The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

    // The battle ending conditions

    L"VITTORIA!",    // STR_AR_OVER_VICTORY
    L"SCONFITTA!",   // STR_AR_OVER_DEFEAT
    L"ARRENDERSI!",  // STR_AR_OVER_SURRENDERED
    L"CATTURATI!",   // STR_AR_OVER_CAPTURED
    L"RITIRARSI!",   // STR_AR_OVER_RETREATED

    // These are the labels for the different types of enemies we fight in autoresolve.

    L"Esercito",        // STR_AR_MILITIA_NAME,
    L"Èlite",           // STR_AR_ELITE_NAME,
    L"Truppa",          // STR_AR_TROOP_NAME,
    L"Amministratore",  // STR_AR_ADMINISTRATOR_NAME,
    L"Creatura",        // STR_AR_CREATURE_NAME,

    // Label for the length of time the battle took

    L"Tempo trascorso",  // STR_AR_TIME_ELAPSED,

    // Labels for status of merc if retreating.  (UPPERCASE)

    L"RITIRATOSI",  // STR_AR_MERC_RETREATED,
    L"RITIRARSI",   // STR_AR_MERC_RETREATING,
    L"RITIRATA",    // STR_AR_MERC_RETREAT,

    // PRE BATTLE INTERFACE STRINGS
    // Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
    // a system that automatically resolves the combat for the player without having to do anything.
    // These strings must be short (two lines -- 6-8 chars per line)

    L"Esito",           // STR_PB_AUTORESOLVE_BTN,
    L"Vai al settore",  // STR_PB_GOTOSECTOR_BTN,
    L"Ritira merc.",    // STR_PB_RETREATMERCS_BTN,

    // The different headers(titles) for the prebattle interface.
    L"SCONTRO NEMICO",                  // STR_PB_ENEMYENCOUNTER_HEADER,
    L"INVASIONE NEMICA",                // STR_PB_ENEMYINVASION_HEADER, // 30
    L"IMBOSCATA NEMICA",                // STR_PB_ENEMYAMBUSH_HEADER
    L"INTRUSIONE NEMICA NEL SETTORE",   // STR_PB_ENTERINGENEMYSECTOR_HEADER
    L"ATTACCO DELLE CREATURE",          // STR_PB_CREATUREATTACK_HEADER
    L"IMBOSCATA DEI BLOODCAT",          // STR_PB_BLOODCATAMBUSH_HEADER
    L"INTRUSIONE NELLA TANA BLOODCAT",  // STR_PB_ENTERINGBLOODCATLAIR_HEADER

    // Various single words for direct translation.  The Civilians represent the civilian
    // militia occupying the sector being attacked.  Limited to 9-10 chars

    L"Postazione",
    L"Nemici",
    L"Mercenari",
    L"Esercito",
    L"Creature",
    L"Bloodcat",
    L"Settore",
    L"Nessuno",  // If there are no uninvolved mercs in this fight.
    L"N/A",      // Acronym of Not Applicable
    L"g",        // One letter abbreviation of day
    L"o",        // One letter abbreviation of hour

    // TACTICAL PLACEMENT USER INTERFACE STRINGS
    // The four buttons

    L"Sgombro",
    L"Sparsi",
    L"In gruppo",
    L"Fine",

    // The help text for the four buttons.  Use \n to denote new line (just like enter).

    L"|Mostra chiaramente tutte le postazioni dei mercenari, \ne vi permette di rimetterli in "
    L"gioco manualmente.",
    L"A caso |sparge i vostri mercenari \nogni volta che lo premerete.",
    L"Vi permette di scegliere dove vorreste |raggruppare i vostri mercenari.",
    L"Cliccate su questo pulsante quando avrete \nscelto le postazioni dei vostri mercenari. "
    L"(|I|n|v|i|o)",
    L"Dovete posizionare tutti i vostri mercenari \nprima di iniziare la battaglia.",

    // Various strings (translate word for word)

    L"Settore",
    L"Scegliete le postazioni di intervento",

    // Strings used for various popup message boxes.  Can be as long as desired.

    L"Non sembra così bello qui. È inacessibile. Provate con una diversa postazione.",
    L"Posizionate i vostri mercenari nella sezione illuminata della mappa.",

    // This message is for mercs arriving in sectors.  Ex:  Red has arrived in sector A9.
    // Don't uppercase first character, or add spaces on either end.

    L"è arivato nel settore",

    // These entries are for button popup help text for the prebattle interface.  All popup help
    // text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
    L"|Automaticamente svolge i combattimenti al vostro posto\nsenza caricare la mappa.",
    L"Non è possibile utilizzare l'opzione di risoluzione automatica quando\nil giocatore sta "
    L"attaccando.",
    L"|Entrate nel settore per catturare il nemico.",
    L"|Rimandate il gruppo al settore precedente.",            // singular version
    L"|Rimandate tutti i gruppi ai loro settori precedenti.",  // multiple groups with same previous
                                                               // sector

    // various popup messages for battle conditions.

    //%c%d is the sector -- ex:  A9
    L"I nemici attaccano il vostro esercito nel settore %c%d.",
    //%c%d is the sector -- ex:  A9
    L"Le creature attaccano il vostro esercito nel settore %c%d.",
    // 1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
    // Note:  the minimum number of civilians eaten will be two.
    L"Le creature attaccano e uccidono %d civili nel settore %s.",
    //%s is the sector location -- ex:  A9: Omerta
    L"I nemici attaccano i vostri mercenari nel settore %s. Nessuno dei vostri mercenari è in "
    L"grado di combattere!",
    //%s is the sector location -- ex:  A9: Omerta
    L"I nemici attaccano i vostri mercenari nel settore %s. Nessuno dei vostri mercenari è in "
    L"grado di combattere!",

};

static wchar_t* it_gpGameClockString[] = {
    // This is the day represented in the game clock.  Must be very short, 4 characters max.
    L"Gg",
};

// When the merc finds a key, they can get a description of it which
// tells them where and when they found it.
static wchar_t* it_sKeyDescriptionStrings[2] = {
    L"Settore trovato:",
    L"Giorno trovato:",
};

// The headers used to describe various weapon statistics.

static wchar_t* it_gWeaponStatsDesc[] = {
    L"Peso (%s):", L"Stato:",
    L"Ammontare:",  // Number of bullets left in a magazine
    L"Git:",        // Range
    L"Dan:",        // Damage
    L"PA:",         // abbreviation for Action Points
    L"",           L"=",      L"=",
};

// The headers used for the merc's money.

static wchar_t* it_gMoneyStatsDesc[] = {
    L"Ammontare",
    L"Rimanenti:",  // this is the overall balance
    L"Ammontare",
    L"Da separare:",  // the amount he wants to separate from the overall balance to get two piles
                      // of money

    L"Bilancio",
    L"corrente",
    L"Ammontare",
    L"del prelievo",
};

// The health of various creatures, enemies, characters in the game. The numbers following each are
// for comment only, but represent the precentage of points remaining.

static wchar_t* it_zHealthStr[] = {
    L"MORENTE",     //	>= 0
    L"CRITICO",     //	>= 15
    L"DEBOLE",      //	>= 30
    L"FERITO",      //	>= 45
    L"SANO",        //	>= 60
    L"FORTE",       // 	>= 75
    L"ECCELLENTE",  // 	>= 90
};

static wchar_t* it_gzMoneyAmounts[6] = {
    L"$1000", L"$100", L"$10", L"Fine", L"Separare", L"Prelevare",
};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static wchar_t* it_gzProsLabel = L"Vant.:";

static wchar_t* it_gzConsLabel = L"Svant.:";

// Conversation options a player has when encountering an NPC
static wchar_t* it_zTalkMenuStrings[6] = {
    L"Vuoi ripetere?",  // meaning "Repeat yourself"
    L"Amichevole",      // approach in a friendly
    L"Diretto",         // approach directly - let's get down to business
    L"Minaccioso",      // approach threateningly - talk now, or I'll blow your face off
    L"Dai",
    L"Recluta",
};

// Some NPCs buy, sell or repair items. These different options are available for those NPCs as
// well.
static wchar_t* it_zDealerStrings[4] = {
    L"Compra/Vendi",
    L"Compra",
    L"Vendi",
    L"Ripara",
};

static wchar_t* it_zDialogActions[1] = {
    L"Fine",
};

// These are vehicles in the game.

static wchar_t* it_pVehicleStrings[] = {
    L"Eldorado",
    L"Hummer",  // a hummer jeep/truck -- military vehicle
    L"Icecream Truck", L"Jeep", L"Carro armato", L"Elicottero",
};

static wchar_t* it_pShortVehicleStrings[] = {
    L"Eldor.",
    L"Hummer",  // the HMVV
    L"Truck",  L"Jeep", L"Carro",
    L"Eli",  // the helicopter
};

static wchar_t* it_zVehicleName[] = {
    L"Eldorado",
    L"Hummer",  // a military jeep. This is a brand name.
    L"Truck",   // Ice cream truck
    L"Jeep",     L"Carro",
    L"Eli",  // an abbreviation for Helicopter
};

// These are messages Used in the Tactical Screen

static wchar_t* it_TacticalStr[] = {
    L"Attacco aereo", L"Ricorrete al pronto soccorso automaticamente?",

    // CAMFIELD NUKE THIS and add quote #66.

    L"%s nota ch egli oggetti mancano dall'equipaggiamento.",

    // The %s is a string from pDoorTrapStrings

    L"La serratura ha %s", L"Non ci sono serrature", L"Vittoria!", L"Fallimento", L"Vittoria!",
    L"Fallimento", L"La serratura non presenta trappole", L"Vittoria!",
    // The %s is a merc name
    L"%s non ha la chiave giusta", L"La serratura non presenta trappole",
    L"La serratura non presenta trappole", L"Serrato", L"", L"TRAPPOLE", L"SERRATO", L"APERTO",
    L"FRACASSATO", L"C'è un interruttore qui. Lo volete attivare?", L"Disattivate le trappole?",
    L"Prec...", L"Succ...", L"Più...",

    // In the next 2 strings, %s is an item name

    L"Il %s è stato posizionato sul terreno.", L"Il %s è stato dato a %s.",

    // In the next 2 strings, %s is a name

    L"%s è stato pagato completamente.", L"Bisogna ancora dare %d a %s.",
    L"Scegliete la frequenza di detonazione:",  // in this case, frequency refers to a radio signal
    L"Quante volte finché la bomba non esploderà:",    // how much time, in turns, until the bomb
                                                       // blows
    L"Stabilite la frequenza remota di detonazione:",  // in this case, frequency refers to a radio
                                                       // signal
    L"Disattivate le trappole?", L"Rimuovete la bandiera blu?", L"Mettete qui la bandiera blu?",
    L"Fine del turno",

    // In the next string, %s is a name. Stance refers to way they are standing.

    L"Siete sicuri di volere attaccare %s ?", L"Ah, i veicoli non possono cambiare posizione.",
    L"Il robot non può cambiare posizione.",

    // In the next 3 strings, %s is a name

    L"%s non può cambiare posizione.", L"%s non sono ricorsi al pronto soccorso qui.",
    L"%s non ha bisogno del pronto soccorso.", L"Non può muoversi là.",
    L"La vostra squadra è al completo. Non c'è spazio per una recluta.",  // there's no room for a
                                                                          // recruit on the player's
                                                                          // team

    // In the next string, %s is a name

    L"%s è stato reclutato.",

    // Here %s is a name and %d is a number

    L"Bisogna dare %d a $%s.",

    // In the next string, %s is a name

    L"Scortate %s?",

    // In the next string, the first %s is a name and the second %s is an amount of money (including
    // $ sign)

    L"Il salario di %s ammonta a %s per giorno?",

    // This line is used repeatedly to ask player if they wish to participate in a boxing match.

    L"Volete combattere?",

    // In the next string, the first %s is an item name and the
    // second %s is an amount of money (including $ sign)

    L"Comprate %s per %s?",

    // In the next string, %s is a name

    L"%s è scortato dalla squadra %d.",

    // These messages are displayed during play to alert the player to a particular situation

    L"INCEPPATA",                                     // weapon is jammed.
    L"Il robot ha bisogno di munizioni calibro %s.",  // Robot is out of ammo
    L"Cosa? Impossibile.",  // Merc can't throw to the destination he selected

    // These are different buttons that the player can turn on and off.

    L"Modalità furtiva (|Z)", L"Schermata della |mappa", L"Fine del turno (|D)", L"Parlato",
    L"Muto", L"Alza (|P|a|g|S|ù)", L"Livello della vista (|T|a|b)", L"Scala / Salta",
    L"Abbassa (|P|a|g|G|i|ù)", L"Esamina (|C|t|r|l)", L"Mercenario precedente",
    L"Prossimo mercenario (|S|p|a|z|i|o)", L"|Opzioni", L"Modalità a raffica (|B)",
    L"Guarda/Gira (|L)", L"Salute: %d/%d\nEnergia: %d/%d\nMorale: %s",
    L"Eh?",    // this means "what?"
    L"Fermo",  // an abbrieviation for "Continued"
    L"Audio on per %s.", L"Audio off per %s.", L"Salute: %d/%d\nCarburante: %d/%d",
    L"Uscita veicoli", L"Cambia squadra (|M|a|i|u|s|c |S|p|a|z|i|o)", L"Guida",
    L"N/A",  // this is an acronym for "Not Applicable."
    L"Usa (Corpo a corpo)", L"Usa (Arma da fuoco)", L"Usa (Lama)", L"Usa (Esplosivo)",
    L"Usa (Kit medico)", L"Afferra", L"Ricarica", L"Dai", L"%s è partito.", L"%s è arrivato.",
    L"%s ha esaurito i Punti Azione.", L"%s non è disponibile.", L"%s è tutto bendato.",
    L"%s non è provvisto di bende.", L"Nemico nel settore!", L"Nessun nemico in vista.",
    L"Punti Azione insufficienti.", L"Nessuno sta utilizzando il comando a distanza.",
    L"Il fuoco a raffica ha svuotato il caricatore!", L"SOLDATO", L"CREPITUS", L"ESERCITO",
    L"CIVILE", L"Settore di uscita", L"OK", L"Annulla", L"Merc. selezionato", L"Tutta la squadra",
    L"Vai nel settore", L"Vai alla mappa", L"Non puoi uscire dal settore da questa parte.",
    L"%s è troppo lontano.", L"Rimuovi le fronde degli alberi", L"Mostra le fronde degli alberi",
    L"CORVO",  // Crow, as in the large black bird
    L"COLLO", L"TESTA", L"TORSO", L"GAMBE", L"Vuoi dire alla Regina cosa vuole sapere?",
    L"Impronta digitale ID ottenuta", L"Impronta digitale ID non valida. Arma non funzionante",
    L"Raggiunto scopo", L"Sentiero bloccato",
    L"Deposita/Preleva soldi",  // Help text over the $ button on the Single Merc Panel
    L"Nessuno ha bisogno del pronto soccorso.",
    L"Bloccato.",           // Short form of JAMMED, for small inv slots
    L"Non può andare là.",  // used ( now ) for when we click on a cliff
    L"Il sentiero è bloccato. Vuoi scambiare le posizioni con questa persona?",
    L"La persona rifiuta di muoversi.",
    // In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
    L"Sei d'accordo a pagare %s?", L"Accetti il trattamento medico gratuito?",
    L"Vuoi sposare Daryl?", L"Quadro delle chiavi", L"Non puoi farlo con un EPC.",
    L"Risparmi Krott?", L"Fuori dalla gittata dell'arma", L"Minatore",
    L"Il veicolo può viaggiare solo tra i settori", L"Non è in grado di fasciarsi da solo ora",
    L"Sentiero bloccato per %s",
    //	L"I tuoi mercenari, che erano stati catturati dall'esercito di Deidranna, sono stati
    // imprigionati qui!",
    L"I mercenari catturati dall'esercito di Deidranna, sono stati imprigionati qui!",
    L"Serratura manomessa", L"Serratura distrutta",
    L"Qualcun altro sta provando a utilizzare questa porta.", L"Salute: %d/%d\nCarburante: %d/%d",
    L"%s non riesce a vedere %s.",  // Cannot see person trying to talk to
};

// Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different
// circumstances in the "exiting sector" interface.
static wchar_t* it_pExitingSectorHelpText[] = {
    // Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box
    // is checked.
    L"Se selezionato, il settore adiacente verrà immediatamente caricato.",
    L"Se selezionato, sarete automaticamente posti nella schermata della mappa\nvisto che i vostri "
    L"mercenari avranno bisogno di tempo per viaggiare.",

    // If you attempt to leave a sector when you have multiple squads in a hostile sector.
    L"Questo settore è occupato da nemicie non potete lasciare mercenari qui.\nDovete risolvere "
    L"questa situazione prima di caricare qualsiasi altro settore.",

    // Because you only have one squad in the sector, and the "move all" option is checked, the "go
    // to sector" option is locked to on. The helptext explains why it is locked.
    L"Rimuovendo i vostri mercenari da questo settore,\nil settore adiacente verrà immediatamente "
    L"caricato.",
    L"Rimuovendo i vostri mercenari da questo settore,\nverrete automaticamente postinella "
    L"schermata della mappa\nvisto che i vostri mercenari avranno bisogno di tempo per viaggiare.",

    // If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being
    // escorted.  The "single" button is disabled.
    L"%s ha bisogno di essere scortato dai vostri mercenari e non può lasciare questo settore da "
    L"solo.",

    // If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc
    // will be prohibited from leaving alone. There are several strings depending on the gender of
    // the merc and how many EPCs are in the squad. DO NOT USE THE NEWLINE HERE AS IT IS USED FOR
    // BOTH HELPTEXT AND SCREEN MESSAGES!
    L"%s non può lasciare questo settore da solo, perché sta scortando %s.",  // male singular
    L"%s non può lasciare questo settore da solo, perché sta scortando %s.",  // female singular
    L"%s non può lasciare questo settore da solo, perché sta scortando altre persone.",  // male
                                                                                         // plural
    L"%s non può lasciare questo settore da solo, perché sta scortando altre persone.",  // female
                                                                                         // plural

    // If one or more of your mercs in the selected squad aren't in range of the traversal area,
    // then the  "move all" option is disabled, and this helptext explains why.
    L"Tutti i vostri personaggi devono trovarsi nei paraggi\nin modo da permettere alla squadra di "
    L"attraversare.",

    L"",  // UNUSED

    // Standard helptext for single movement.  Explains what will happen (splitting the squad)
    L"Se selezionato, %s viaggerà da solo, e\nautomaticamente verrà riassegnato a un'unica "
    L"squadra.",

    // Standard helptext for all movement.  Explains what will happen (moving the squad)
    L"Se selezionato, la vostra \nsquadra attualmente selezionata viaggerà, lasciando questo "
    L"settore.",

    // This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC
    // selected and you attempt to tactically traverse the EPC while the escorting mercs aren't near
    // enough (or dead, dying, or unconscious), this message will appear and the "exiting sector"
    // interface will not appear.  This is just like the situation where This string is special, as
    // it is not used as helptext.  Do not use the special newline character (\n) for this string.
    L"%s è scortato dai vostri mercenari e non può lasciare questo settore da solo. Gli altri "
    L"vostri mercenari devono trovarsi nelle vicinanze prima che possiate andarvene.",
};

static wchar_t* it_pRepairStrings[] = {
    L"Oggetti",   // tell merc to repair items in inventory
    L"Sito SAM",  // tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
    L"Annulla",   // cancel this menu
    L"Robot",     // repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static wchar_t* it_sPreStatBuildString[] = {
    L"perduto",     // the merc has lost a statistic
    L"guadagnato",  // the merc has gained a statistic
    L"punto di",    // singular
    L"punti di",    // plural
    L"livello di",  // singular
    L"livelli di",  // plural
};

static wchar_t* it_sStatGainStrings[] = {
    L"salute.",
    L"agilità.",
    L"destrezza.",
    L"saggezza.",
    L"pronto socc.",
    L"abilità esplosivi.",
    L"abilità meccanica.",
    L"mira.",
    L"esperienza.",
    L"forza.",
    L"comando.",
};

static wchar_t* it_pHelicopterEtaStrings[] = {
    L"Distanza totale: ",  // total distance for helicopter to travel
    L"Sicura: ",           // distance to travel to destination
    L"Insicura: ",         // distance to return from destination to airport
    L"Costo totale: ",     // total cost of trip by helicopter
    L"TPA: ",              // ETA is an acronym for "estimated time of arrival"
    L"L'elicottero ha poco carburante e deve atterrare in territorio nemico!",  // warning that the
                                                                                // sector the
                                                                                // helicopter is
                                                                                // going to use for
                                                                                // refueling is
                                                                                // under enemy
                                                                                // control ->
    L"Passeggeri: ",
    L"Seleziona Skyrider o gli Arrivi Drop-off?",
    L"Skyrider",
    L"Arrivi",
};

static wchar_t* it_sMapLevelString[] = {
    L"Sottolivello:",  // what level below the ground is the player viewing in mapscreen
};

static wchar_t* it_gsLoyalString[] = {
    L"Leale",  // the loyalty rating of a town ie : Loyal 53%
};

// error message for when player is trying to give a merc a travel order while he's underground.

static wchar_t* it_gsUndergroundString[] = {
    L"non può portare ordini di viaggio sottoterra.",
};

static wchar_t* it_gsTimeStrings[] = {
    L"h",  // hours abbreviation
    L"m",  // minutes abbreviation
    L"s",  // seconds abbreviation
    L"g",  // days abbreviation
};

// text for the various facilities in the sector

static wchar_t* it_sFacilitiesStrings[] = {
    L"Nessuno",
    L"Ospedale",
    L"Fabbrica",
    L"Prigione",
    L"Militare",
    L"Aeroporto",
    L"Frequenza di fuoco",  // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static wchar_t* it_pMapPopUpInventoryText[] = {
    L"Inventario",
    L"Uscita",
};

// town strings

static wchar_t* it_pwTownInfoStrings[] = {
    L"Dimensione",  // 0 // size of the town in sectors
    L"",            // blank line, required
    L"Controllo",   // how much of town is controlled
    L"Nessuno",     // none of this town
    L"Miniera",     // mine associated with this town
    L"Lealtà",      // 5 // the loyalty level of this town
    L"Addestrato",  // the forces in the town trained by the player
    L"",
    L"Servizi principali",    // main facilities in this town
    L"Livello",               // the training level of civilians in this town
    L"addestramento civili",  // 10 // state of civilian training in town
    L"Esercito",              // the state of the trained civilians in the town
};

// Mine strings

static wchar_t* it_pwMineStrings[] = {
    L"Miniera",  // 0
    L"Argento",
    L"Oro",
    L"Produzione giornaliera",
    L"Produzione possibile",
    L"Abbandonata",  // 5
    L"Chiudi",
    L"Esci",
    L"Produci",
    L"Stato",
    L"Ammontare produzione",
    L"Tipo di minerale",  // 10
    L"Controllo della città",
    L"Lealtà della città",
    //	L"Minatori",
};

// blank sector strings

static wchar_t* it_pwMiscSectorStrings[] = {
    L"Forze nemiche", L"Settore", L"# di oggetti", L"Sconosciuto", L"Controllato", L"Sì", L"No",
};

// error strings for inventory

static wchar_t* it_pMapInventoryErrorString[] = {
    L"%s non è abbastanza vicino.",           // Merc is in sector with item but not close enough
    L"Non può selezionare quel mercenario.",  // MARK CARTER
    L"%s non si trova nel settore per prendere quell'oggetto.",
    L"Durante il combattimento, dovrete raccogliere gli oggetti manualmente.",
    L"Durante il combattimento, dovrete rilasciare gli oggetti manualmente.",
    L"%s non si trova nel settore per rilasciare quell'oggetto.",
};

static wchar_t* it_pMapInventoryStrings[] = {
    L"Posizione",       // sector these items are in
    L"Totale oggetti",  // total number of items in sector
};

// help text for the user

static wchar_t* it_pMapScreenFastHelpTextList[] = {
    L"Per cambiare l'incarico di un mercenario, come, ad esempio, cambiare la squadra, dottore o "
    L"riparare, cliccate dentro la colonna 'Compito'",
    L"Per assegnare a un mercenario una destinazione in un altro settore, cliccate dentro la "
    L"colonna 'Dest'",
    L"Una volta che a un mercenario è stato ordinato di procedere, una compressione di tempo gli "
    L"permetterà di muoversi.",
    L"Cliccando di sinistro, selezionerete il settore. Cliccando di sinistro un'altra volta, "
    L"darete al mercenario ordini di movimento. Cliccando di destro, darete informazioni sommarie "
    L"al settore.",
    L"Premete 'h' in questo settore di questa schermata ogni volta che vorrete accedere a questa "
    L"finestra d'aiuto.",
    L"Test Text",
    L"Test Text",
    L"Test Text",
    L"Test Text",
    L"Non potrete fare molto in questa schermata finché non arriverete ad Arulco. Quando avrete "
    L"definito la vostra squadra, cliccate sul pulsante Compressione di Tempo in basso a destra. "
    L"Questo diminuirà il tempo necessario alla vostra squadra per raggiungere Arulco.",
};

// movement menu text

static wchar_t* it_pMovementMenuStrings[] = {
    L"Muovere mercenari nel settore",  // title for movement box
    L"Rotta spostamento esercito",     // done with movement menu, start plotting movement
    L"Annulla",                        // cancel this menu
    L"Altro",                          // title for group of mercs not on squads nor in vehicles
};

static wchar_t* it_pUpdateMercStrings[] = {
    L"Oops:",                                  // an error has occured
    L"Scaduto contratto mercenari:",           // this pop up came up due to a merc contract ending
    L"Portato a termine incarico mercenari:",  // this pop up....due to more than one merc finishing
                                               // assignments
    L"Mercenari di nuovo al lavoro:",  // this pop up ....due to more than one merc waking up and
                                       // returing to work
    L"Mercenari a riposo:",  // this pop up ....due to more than one merc being tired and going to
                             // sleep
    L"Contratti in scadenza:",  // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static wchar_t* it_pMapScreenBorderButtonHelpText[] = {
    L"Mostra città (|w)",    L"Mostra |miniere", L"Mos|tra squadre & nemici",
    L"Mostra spazio |aereo", L"Mostra oggett|i", L"Mostra esercito & nemici (|Z)",
};

static wchar_t* it_pMapScreenBottomFastHelp[] = {
    L"Portati|le",
    L"Tattico (|E|s|c)",
    L"|Opzioni",
    L"Dilata tempo (|+)",                                     // time compress more
    L"Comprime tempo (|-)",                                   // time compress less
    L"Messaggio precedente (|S|u)\nIndietro (|P|a|g|S|u)",    // previous message in scrollable list
    L"Messaggio successivo (|G|i|ù)\nAvanti (|P|a|g|G|i|ù)",  // next message in the scrollable list
    L"Inizia/Ferma tempo (|S|p|a|z|i|o)",                     // start/stop time compression
};

static wchar_t* it_pMapScreenBottomText[] = {
    L"Bilancio attuale",  // current balance in player bank account
};

static wchar_t* it_pMercDeadString[] = {
    L"%s è morto.",
};

static wchar_t* it_pDayStrings[] = {
    L"Giorno",
};

// the list of email sender names

static wchar_t* it_pSenderNameList[] = {
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
    L"Assicurazione M.I.S.",
    L"Bobby Ray",
    L"Capo",
    L"John Kulba",
    L"A.I.M.",
};

// next/prev strings

static wchar_t* it_pTraverseStrings[] = {
    L"Indietro",
    L"Avanti",
};

// new mail notify string

static wchar_t* it_pNewMailStrings[] = {
    L"Avete una nuova E-mail...",
};

// confirm player's intent to delete messages

static wchar_t* it_pDeleteMailStrings[] = {
    L"Eliminate l'E-mail?",
    L"Eliminate l'E-mail NON LETTA?",
};

// the sort header strings

static wchar_t* it_pEmailHeaders[] = {
    L"Da:",
    L"Sogg.:",
    L"Giorno:",
};

// email titlebar text

static wchar_t* it_pEmailTitleText[] = {
    L"posta elettronica",
};

// the financial screen strings
static wchar_t* it_pFinanceTitle[] = {
    L"Contabile aggiuntivo",  // the name we made up for the financial program in the game
};

static wchar_t* it_pFinanceSummary[] = {
    L"Crediti:",  // credit (subtract from) to player's account
    L"Debiti:",   // debit (add to) to player's account
    L"Entrate effettive di ieri:",
    L"Altri depositi di ieri:",
    L"Debiti di ieri:",
    L"Bilancio di fine giornata:",
    L"Entrate effettive di oggi:",
    L"Altri depositi di oggi:",
    L"Debiti di oggi:",
    L"Bilancio attuale:",
    L"Entrate previste:",
    L"Bilancio previsto:",  // projected balance for player for tommorow
};

// headers to each list in financial screen

static wchar_t* it_pFinanceHeaders[] = {
    L"Giorno",       // the day column
    L"Crediti",      // the credits column (to ADD money to your account)
    L"Debiti",       // the debits column (to SUBTRACT money from your account)
    L"Transazione",  // transaction type - see TransactionText below
    L"Bilancio",     // balance at this point in time
    L"Pagina",       // page number
    L"Giorno(i)",    // the day(s) of transactions this page displays
};

static wchar_t* it_pTransactionText[] = {
    L"Interessi maturati",  // interest the player has accumulated so far
    L"Deposito anonimo",
    L"Tassa di transazione",
    L"Assunto",                  // Merc was hired
    L"Acquistato da Bobby Ray",  // Bobby Ray is the name of an arms dealer
    L"Acconti pagati al M.E.R.C.",
    L"Deposito medico per %s",      // medical deposit for merc
    L"Analisi del profilo I.M.P.",  // IMP is the acronym for International Mercenary Profiling
    L"Assicurazione acquistata per %s",
    L"Assicurazione ridotta per %s",
    L"Assicurazione estesa per %s",  // johnny contract extended
    L"Assicurazione annullata %s",
    L"Richiesta di assicurazione per %s",  // insurance claim for merc
    L"1 giorno",                           // merc's contract extended for a day
    L"1 settimana",                        // merc's contract extended for a week
    L"2 settimane",                        // ... for 2 weeks
    L"Entrata mineraria",
    L"",  // String nuked
    L"Fiori acquistati",
    L"Totale rimborso medico per %s",
    L"Parziale rimborso medico per %s",
    L"Nessun rimborso medico per %s",
    L"Pagamento a %s",                  // %s is the name of the npc being paid
    L"Trasferimento fondi a %s",        // transfer funds to a merc
    L"Trasferimento fondi da %s",       // transfer funds from a merc
    L"Equipaggiamento esercito in %s",  // initial cost to equip a town's militia
    L"Oggetti acquistati da%s.",  // is used for the Shop keeper interface.  The dealers name will
                                  // be appended to the end of the string.
    L"%s soldi depositati.",
};

static wchar_t* it_pTransactionAlternateText[] = {
    L"Assicurazione per",                   // insurance for a merc
    L"Est. contratto di %s per 1 giorno.",  // entend mercs contract by a day
    L"Est. %s contratto per 1 settimana.",
    L"Est. %s contratto per 2 settimane.",
};

// helicopter pilot payment

static wchar_t* it_pSkyriderText[] = {
    L"Skyrider è stato pagato $%d",           // skyrider was paid an amount of money
    L"A Skyrider bisogna ancora dare $%d",    // skyrider is still owed an amount of money
    L"Skyrider ha finito il carburante",      // skyrider has finished refueling
    L"",                                      // unused
    L"",                                      // unused
    L"Skyrider è di nuovo pronto a volare.",  // Skyrider was grounded but has been freed
    L"Skyrider non ha passeggeri. Se avete intenzione di trasportare mercenari in questo settore, "
    L"assegnateli prima al Veicolo/Elicottero.",
};

// strings for different levels of merc morale

static wchar_t* it_pMoralStrings[] = {
    L"Ottimo", L"Buono", L"Medio", L"Basso", L"Panico", L"Cattivo",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.

static wchar_t* it_pLeftEquipmentString[] = {
    L"L'equipaggio di %s è ora disponibile a Omerta (A9).",
    L"L'equipaggio di %s è ora disponibile a Drassen (B13).",
};

// Status that appears on the Map Screen

static wchar_t* it_pMapScreenStatusStrings[] = {
    L"Salute",     L"Energia", L"Morale",
    L"Condizione",  // the condition of the current vehicle (its "health")
    L"Carburante",  // the fuel level of the current vehicle (its "energy")
};

static wchar_t* it_pMapScreenPrevNextCharButtonHelpText[] = {
    L"Mercenario precedente (|S|i|n)",  // previous merc in the list
    L"Mercenario successivo (|D|e|s)",  // next merc in the list
};

static wchar_t* it_pEtaString[] = {
    L"TAP",  // eta is an acronym for Estimated Time of Arrival
};

static wchar_t* it_pTrashItemText[] = {
    L"Non lo vedrete mai più. Siete sicuri?",  // do you want to continue and lose the item
                                               // forever
    L"Questo oggetto sembra DAVVERO importante. Siete DAVVERO SICURISSIMI di volerlo gettare "
    L"via?",  // does the user REALLY want to trash this item
};

static wchar_t* it_pMapErrorString[] = {
    L"La squadra non può muoversi, se un mercenario dorme.",

    // 1-5
    L"Muovete la squadra al primo piano.",
    L"Ordini di movimento? È un settore nemico!",
    L"I mercenari devono essere assegnati a una squadra o a un veicolo per potersi "
    L"muovere.",
    L"Non avete ancora membri nella squadra.",          // you have no members, can't do
                                                        // anything
    L"I mercenari non possono attenersi agli ordini.",  // merc can't comply with your
                                                        // order
                                                        // 6-10
    L"ha bisogno di una scorta per muoversi. Inseritelo in una squadra che ne è "
    L"provvista.",  // merc can't move unescorted .. for a male
    L"ha bisogno di una scorta per muoversi. Inseritela in una squadra che ne è "
    L"provvista.",  // for a female
    L"Il mercenario non è ancora arrivato ad Arulco!",
    L"Sembra che ci siano negoziazioni di contratto da stabilire.",
    L"",
    // 11-15
    L"Ordini di movimento? È in corso una battaglia!",
    L"Siete stati vittima di un'imboscata da parte dai Bloodcat nel settore %s!",
    L"Siete appena entrati in quella che sembra una tana di un Bloodcat nel settore "
    L"I16!",
    L"",
    L"La zona SAM in %s è stata assediata.",
    // 16-20
    L"La miniera di %s è stata assediata. La vostra entrata giornaliera è stata "
    L"ridotta di %s per giorno.",
    L"Il nemico ha assediato il settore %s senza incontrare resistenza.",
    L"Almeno uno dei vostri mercenari non ha potuto essere affidato a questo incarico.",
    L"%s non ha potuto unirsi alla %s visto che è completamente pieno",
    L"%s non ha potuto unirsi alla %s visto che è troppo lontano.",
    // 21-25
    L"La miniera di %s è stata invasa dalle forze armate di Deidranna!",
    L"Le forze armate di Deidranna hanno appena invaso la zona SAM in %s",
    L"Le forze armate di Deidranna hanno appena invaso %s",
    L"Le forze armate di Deidranna sono appena state avvistate in %s.",
    L"Le forze armate di Deidranna sono appena partite per %s.",
    // 26-30
    L"Almeno uno dei vostri mercenari non può riposarsi.",
    L"Almeno uno dei vostri mercenari non è stato svegliato.",
    L"L'esercito non si farà vivo finché non avranno finito di esercitarsi.",
    L"%s non possono ricevere ordini di movimento adesso.",
    L"I militari che non si trovano entro i confini della città non possono essere "
    L"spostati inquesto settore.",
    // 31-35
    L"Non potete avere soldati in %s.",
    L"Un veicolo non può muoversi se è vuoto!",
    L"%s è troppo grave per muoversi!",
    L"Prima dovete lasciare il museo!",
    L"%s è morto!",
    // 36-40
    L"%s non può andare a %s perché si sta muovendo",
    L"%s non può salire sul veicolo in quel modo",
    L"%s non può unirsi alla %s",
    L"Non potete comprimere il tempo finché non arruolerete nuovi mercenari!",
    L"Questo veicolo può muoversi solo lungo le strade!",
    // 41-45
    L"Non potete riassegnare i mercenari che sono già in movimento",
    L"Il veicolo è privo di benzina!",
    L"%s è troppo stanco per muoversi.",
    L"Nessuno a bordo è in grado di guidare il veicolo.",
    L"Uno o più membri di questa squadra possono muoversi ora.",
    // 46-50
    L"Uno o più degli altri mercenari non può muoversi ora.",
    L"Il veicolo è troppo danneggiato!",
    L"Osservate che solo due mercenari potrebbero addestrare i militari in questo "
    L"settore.",
    L"Il robot non può muoversi senza il suo controller. Metteteli nella stessa "
    L"squadra.",
};

// help text used during strategic route plotting
static wchar_t* it_pMapPlotStrings[] = {
    L"Cliccate di nuovo su una destinazione per confermare la vostra meta finale, oppure cliccate "
    L"su un altro settore per fissare più tappe.",
    L"Rotta di spostamento confermata.",
    L"Destinazione immutata.",
    L"Rotta di spostamento annullata.",
    L"Rotta di spostamento accorciata.",
};

// help text used when moving the merc arrival sector
static wchar_t* it_pBullseyeStrings[] = {
    L"Cliccate sul settore dove desiderate che i mercenari arrivino.",
    L"OK. I mercenari che stavano arrivando si sono dileguati a %s",
    L"I mercenari non possono essere trasportati, lo spazio aereo non è sicuro!",
    L"Annullato. Il settore d'arrivo è immutato",
    L"Lo spazio aereo sopra %s non è più sicuro! Il settore d'arrivo è stato spostato a %s.",
};

// help text for mouse regions

static wchar_t* it_pMiscMapScreenMouseRegionHelpText[] = {
    L"Entra nell'inventario (|I|n|v|i|o)",
    L"Getta via l'oggetto",
    L"Esci dall'inventario (|I|n|v|i|o)",
};

// male version of where equipment is left
static wchar_t* it_pMercHeLeaveString[] = {
    L"Volete che %s lasci il suo equipaggiamento dove si trova ora (%s) o in seguito a Drassen "
    L"(B13) dopo aver preso il volo da Arulco?",
    L"Volete che %s lasci il suo equipaggiamento dove si trova ora (%s) o in seguito a Omerta (A9) "
    L"dopo aver preso il volo da Arulco?",
    L"sta per partire e spedirà il suo equipaggiamento a Omerta (A9).",
    L"sta per partire e spedirà il suo equipaggiamento a Drassen (B13).",
    L"%s sta per partire e spedirà il suo equipaggiamento a %s.",
};

// female version
static wchar_t* it_pMercSheLeaveString[] = {
    L"Volete che %s lasci il suo equipaggiamento dove si trova ora (%s) o in seguito a Drassen "
    L"(B13) dopo aver preso il volo da Arulco?",
    L"Volete che %s lasci il suo equipaggiamento dove si trova ora (%s) o in seguito a Omerta (A9) "
    L"dopo aver preso il volo da Arulco?",
    L"sta per partire e spedirà il suo equipaggiamento a Omerta (A9).",
    L"sta per partire e spedirà il suo equipaggiamento a Drassen (B13).",
    L"%s sta per partire e spedirà il suo equipaggiamento a %s.",
};

static wchar_t* it_pMercContractOverStrings[] = {
    L": contratto scaduto. Egli è tornato a casa.",  // merc's contract is over and has departed
    L": contratto scaduto. Ella è tornata a casa.",  // merc's contract is over and has departed
    L": contratto terminato. Egli è partito.",       // merc's contract has been terminated
    L": contratto terminato. Ella è partita.",       // merc's contract has been terminated
    L"Dovete al M.E.R.C. troppi soldi, così %s è partito.",  // Your M.E.R.C. account is invalid so
                                                             // merc left
};

// Text used on IMP Web Pages

static wchar_t* it_pImpPopUpStrings[] = {
    L"Codice di autorizzazione non valido",
    L"State per riiniziare l'intero processo di profilo. Ne siete certi?",
    L"Inserite nome e cognome corretti oltre che al sesso",
    L"L'analisi preliminare del vostro stato finanziario mostra che non potete offrire un'analisi "
    L"di profilo.",
    L"Opzione non valida questa volta.",
    L"Per completare un profilo accurato, dovete aver spazio per almeno uno dei membri della "
    L"squadra.",
    L"Profilo già completato.",
};

// button labels used on the IMP site

static wchar_t* it_pImpButtonText[] = {
    L"Cosa offriamo",  // about the IMP site
    L"INIZIO",         // begin profiling
    L"Personalità",    // personality section
    L"Attributi",      // personal stats/attributes section
    L"Ritratto",       // the personal portrait selection
    L"Voce %d",        // the voice selection
    L"Fine",           // done profiling
    L"Ricomincio",     // start over profiling
    L"Sì, scelgo la risposta evidenziata.",
    L"Sì",
    L"No",
    L"Finito",                    // finished answering questions
    L"Prec.",                     // previous question..abbreviated form
    L"Avanti",                    // next question
    L"SÌ, LO SONO.",              // yes, I am certain
    L"NO, VOGLIO RICOMINCIARE.",  // no, I want to start over the profiling process
    L"SÌ",
    L"NO",
    L"Indietro",  // back one page
    L"Annulla",   // cancel selection
    L"Sì, ne sono certo.",
    L"No, lasciami dare un'altra occhiata.",
    L"Immatricolazione",  // the IMP site registry..when name and gender is selected
    L"Analisi",           // analyzing your profile results
    L"OK",
    L"Voce",
};

static wchar_t* it_pExtraIMPStrings[] = {
    L"Per completare il profilo attuale, seleziona 'Personalità'.",
    L"Ora che hai completato la Personalità, seleziona i tuoi attributi.",
    L"Con gli attributi ora assegnati, puoi procedere alla selezione del ritratto.",
    L"Per completare il processo, seleziona il campione della voce che più ti piace.",
};

static wchar_t* it_pFilesTitle[] = {
    L"Gestione risorse",
};

static wchar_t* it_pFilesSenderList[] = {
    L"Rapporto",  // the recon report sent to the player. Recon is an abbreviation for reconissance
    L"Intercetta #1",  // first intercept file .. Intercept is the title of the organization sending
                       // the file...similar in function to INTERPOL/CIA/KGB..refer to fist record
                       // in files.txt for the translated title
    L"Intercetta #2",  // second intercept file
    L"Intercetta #3",  // third intercept file
    L"Intercetta #4",  // fourth intercept file
    L"Intercetta #5",  // fifth intercept file
    L"Intercetta #6",  // sixth intercept file
};

// Text having to do with the History Log

static wchar_t* it_pHistoryTitle[] = {
    L"Registro",
};

static wchar_t* it_pHistoryHeaders[] = {
    L"Giorno",     // the day the history event occurred
    L"Pagina",     // the current page in the history report we are in
    L"Giorno",     // the days the history report occurs over
    L"Posizione",  // location (in sector) the event occurred
    L"Evento",     // the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static wchar_t* it_pHistoryStrings[] = {
    L"",  // leave this line blank
    // 1-5
    L"%s è stato assunto dall'A.I.M.",   // merc was hired from the aim site
    L"%s è stato assunto dal M.E.R.C.",  // merc was hired from the aim site
    L"%s morì.",                         // merc was killed
    L"Acconti stanziati al M.E.R.C.",    // paid outstanding bills at MERC
    L"Assegno accettato da Enrico Chivaldori",
    // 6-10
    L"Profilo generato I.M.P.",
    L"Acquistato contratto d'assicurazione per %s.",    // insurance contract purchased
    L"Annullato contratto d'assicurazione per %s.",     // insurance contract canceled
    L"Versamento per richiesta assicurazione per %s.",  // insurance claim payout for merc
    L"Esteso contratto di %s di 1 giorno.",             // Extented "mercs name"'s for a day
    // 11-15
    L"Esteso contratto di %s di 1 settimana.",  // Extented "mercs name"'s for a week
    L"Esteso contratto di %s di 2 settimane.",  // Extented "mercs name"'s 2 weeks
    L"%s è stato congedato.",                   // "merc's name" was dismissed.
    L"%s è partito.",                           // "merc's name" quit.
    L"avventura iniziata.",                     // a particular quest started
    // 16-20
    L"avventura completata.",
    L"Parlato col capo minatore di %s",  // talked to head miner of town
    L"Liberato %s",
    L"Inganno utilizzato",
    L"Il cibo dovrebbe arrivare a Omerta domani",
    // 21-25
    L"%s ha lasciato la squadra per diventare la moglie di Daryl Hick",
    L"contratto di %s scaduto.",
    L"%s è stato arruolato.",
    L"Enrico si è lamentato della mancanza di progresso",
    L"Vinta battaglia",
    // 26-30
    L"%s miniera ha iniziato a esaurire i minerali",
    L"%s miniera ha esaurito i minerali",
    L"%s miniera è stata chiusa",
    L"%s miniera è stata riaperta",
    L"Trovata una prigione chiamata Tixa.",
    // 31-35
    L"Sentito di una fabbrica segreta di armi chiamata Orta.",
    L"Alcuni scienziati a Orta hanno donato una serie di lanciamissili.",
    L"La regina Deidranna ha bisogno di cadaveri.",
    L"Frank ha parlato di scontri a San Mona.",
    L"Un paziente pensa che lui abbia visto qualcosa nella miniera.",
    // 36-40
    L"Incontrato qualcuno di nome Devin - vende esplosivi.",
    L"Imbattutosi nel famoso ex-mercenario dell'A.I.M. Mike!",
    L"Incontrato Tony - si occupa di armi.",
    L"Preso un lanciamissili dal Sergente Krott.",
    L"Concessa a Kyle la licenza del negozio di pelle di Angel.",
    // 41-45
    L"Madlab ha proposto di costruire un robot.",
    L"Gabby può effettuare operazioni di sabotaggio contro sistemi d'allarme.",
    L"Keith è fuori dall'affare.",
    L"Howard ha fornito cianuro alla regina Deidranna.",
    L"Incontrato Keith - si occupa di un po' di tutto a Cambria.",
    // 46-50
    L"Incontrato Howard - si occupa di farmaceutica a Balime",
    L"Incontrato Perko - conduce una piccola impresa di riparazioni.",
    L"Incontrato Sam di Balime - ha un negozio di hardware.",
    L"Franz si occupa di elettronica e altro.",
    L"Arnold possiede un'impresa di riparazioni a Grumm.",
    // 51-55
    L"Fredo si occupa di elettronica a Grumm.",
    L"Donazione ricevuta da un ricco ragazzo a Balime.",
    L"Incontrato un rivenditore di un deposito di robivecchi di nome Jake.",
    L"Alcuni vagabondi ci hanno dato una scheda elettronica.",
    L"Corrotto Walter per aprire la porta del seminterrato.",
    // 56-60
    L"Se Dave ha benzina, potrà fare il pieno gratis.",
    L"Corrotto Pablo.",
    L"Kingpin tiene i soldi nella miniera di San Mona.",
    L"%s ha vinto il Combattimento Estremo",
    L"%s ha perso il Combattimento Estremo",
    // 61-65
    L"%s è stato squalificato dal Combattimento Estremo",
    L"trovati moltissimi soldi nascosti nella miniera abbandonata.",
    L"Incontrato assassino ingaggiato da Kingpin.",
    L"Perso il controllo del settore",  // ENEMY_INVASION_CODE
    L"Difeso il settore",
    // 66-70
    L"Persa la battaglia",  // ENEMY_ENCOUNTER_CODE
    L"Imboscata fatale",    // ENEMY_AMBUSH_CODE
    L"Annientata imboscata nemica",
    L"Attacco fallito",  // ENTERING_ENEMY_SECTOR_CODE
    L"Attacco riuscito!",
    // 71-75
    L"Creature attaccate",   // CREATURE_ATTACK_CODE
    L"Ucciso dai Bloodcat",  // BLOODCAT_AMBUSH_CODE
    L"Massacrati dai Bloodcat",
    L"%s è stato ucciso",
    L"Data a Carmen la testa di un terrorista",
    L"Massacro sinistro",
    L"Ucciso %s",
};

static wchar_t* it_pHistoryLocations[] = {
    L"N/A",  // N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop

static wchar_t* it_pLaptopIcons[] = {
    L"E-mail",      L"Rete", L"Finanza", L"Personale", L"Cronologia", L"File", L"Chiudi",
    L"sir-FER 4.0",  // our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static wchar_t* it_pBookMarkStrings[] = {
    L"A.I.M.",        L"Bobby Ray", L"I.M.P",         L"M.E.R.C.",
    L"Pompe funebri", L"Fiorista",  L"Assicurazione", L"Annulla",
};

static wchar_t* it_pBookmarkTitle[] = {
    L"Segnalibri",
    L"Cliccate con il destro per accedere a questo menu in futuro.",
};

// When loading or download a web page

static wchar_t* it_pDownloadString[] = {
    L"Caricamento",
    L"Caricamento",
};

// This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static wchar_t* it_gsAtmSideButtonText[] = {
    L"OK",
    L"Prendi",   // take money from merc
    L"Dai",      // give money to merc
    L"Annulla",  // cancel transaction
    L"Pulisci",  // clear amount being displayed on the screen
};

static wchar_t* it_gsAtmStartButtonText[] = {
    L"Trasferisce $",  // transfer money to merc -- short form
    L"Stato",          // view stats of the merc
    L"Inventario",     // view the inventory of the merc
    L"Impiego",
};

static wchar_t* it_sATMText[] = {
    L"Trasferisci fondi?",                       // transfer funds to merc?
    L"Ok?",                                      // are we certain?
    L"Inserisci somma",                          // enter the amount you want to transfer to merc
    L"Seleziona tipo",                           // select the type of transfer to merc
    L"Fondi insufficienti",                      // not enough money to transfer to merc
    L"La somma deve essere un multiplo di $10",  // transfer amount must be a multiple of $10
};

// Web error messages. Please use foreign language equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static wchar_t* it_pErrorStrings[] = {
    L"Errore",
    L"Il server non ha entrata NSD.",
    L"Controlla l'indirizzo LRU e prova di nuovo.",
    L"OK",
    L"Connessione intermittente all'host. Tempi d'attesa più lunghi per il trasferimento.",
};

static wchar_t* it_pPersonnelString[] = {
    L"Mercenari:",  // mercs we have
};

static wchar_t* it_pWebTitle[] = {
    L"sir-FER 4.0",  // our name for the version of the browser, play on company name
};

// The titles for the web program title bar, for each page loaded

static wchar_t* it_pWebPagesTitles[] = {
    L"A.I.M.",
    L"Membri dell'A.I.M.",
    L"Ritratti A.I.M.",  // a mug shot is another name for a portrait
    L"Categoria A.I.M.",
    L"A.I.M.",
    L"Membri dell'A.I.M.",
    L"Tattiche A.I.M.",
    L"Storia A.I.M.",
    L"Collegamenti A.I.M.",
    L"M.E.R.C.",
    L"Conti M.E.R.C.",
    L"Registrazione M.E.R.C.",
    L"Indice M.E.R.C.",
    L"Bobby Ray",
    L"Bobby Ray - Armi",
    L"Bobby Ray - Munizioni",
    L"Bobby Ray - Giubb. A-P",
    L"Bobby Ray - Varie",  // misc is an abbreviation for miscellaneous
    L"Bobby Ray - Usato",
    L"Bobby Ray - Ordine Mail",
    L"I.M.P.",
    L"I.M.P.",
    L"Servizio Fioristi Riuniti",
    L"Servizio Fioristi Riuniti - Galleria",
    L"Servizio Fioristi Riuniti - Ordine",
    L"Servizio Fioristi Riuniti - Card Gallery",
    L"Agenti assicurativi Malleus, Incus & Stapes",
    L"Informazione",
    L"Contratto",
    L"Commenti",
    L"Servizio di pompe funebri di McGillicutty",
    L"",
    L"URL non ritrovato.",
    L"Bobby Ray - Spedizioni recenti",
    L"",
    L"",
};

static wchar_t* it_pShowBookmarkString[] = {
    L"Aiuto",
    L"Cliccate su Rete un'altra volta per i segnalibri.",
};

static wchar_t* it_pLaptopTitles[] = {
    L"Cassetta della posta", L"Gestione risorse", L"Personale",
    L"Contabile aggiuntivo", L"Ceppo storico",
};

static wchar_t* it_pPersonnelDepartedStateStrings[] = {
    // reasons why a merc has left.
    L"Ucciso in azione", L"Licenziato", L"Altro", L"Sposato", L"Contratto Scaduto", L"Liberato",
};
// personnel strings appearing in the Personnel Manager on the laptop

static wchar_t* it_pPersonelTeamStrings[] = {
    L"Squadra attuale",  L"Partenze",          L"Costo giornaliero:", L"Costo più alto:",
    L"Costo più basso:", L"Ucciso in azione:", L"Licenziato:",        L"Altro:",
};

static wchar_t* it_pPersonnelCurrentTeamStatsStrings[] = {
    L"Più basso",
    L"Normale",
    L"Più alto",
};

static wchar_t* it_pPersonnelTeamStatsStrings[] = {
    L"SAL", L"AGI", L"DES", L"FOR", L"COM", L"SAG", L"LIV", L"TIR", L"MEC", L"ESP", L"PS",
};

// horizontal and vertical indices on the map screen

static wchar_t* it_pMapVertIndex[] = {
    L"X", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H",
    L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P",
};

static wchar_t* it_pMapHortIndex[] = {
    L"X", L"1",  L"2",  L"3",  L"4",  L"5",  L"6",  L"7",  L"8",
    L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16",
};

static wchar_t* it_pMapDepthIndex[] = {
    L"",
    L"-1",
    L"-2",
    L"-3",
};

// text that appears on the contract button

static wchar_t* it_pContractButtonString[] = {
    L"Contratto",
};

// text that appears on the update panel buttons

static wchar_t* it_pUpdatePanelButtons[] = {
    L"Continua",
    L"Fermati",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static wchar_t* it_LargeTacticalStr[] = {
    L"Siete stati sconfitti in questo settore!",
    L"Il nemico, non avendo alcuna pietà delle anime della squadra, divorerà ognuno di voi!",
    L"I membri inconscenti della vostra squadra sono stati catturati!",
    L"I membri della vostra squadra sono stati fatti prigionieri dal nemico.",
};

// Insurance Contract.c
// The text on the buttons at the bottom of the screen.

static wchar_t* it_InsContractText[] = {
    L"Indietro",
    L"Avanti",
    L"Accetta",
    L"Pulisci",
};

// Insurance Info
// Text on the buttons on the bottom of the screen

static wchar_t* it_InsInfoText[] = {
    L"Indietro",
    L"Avanti",
};

// For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static wchar_t* it_MercAccountText[] = {
    // Text on the buttons on the bottom of the screen
    L"Autorizza",
    L"Home Page",
    L"Conto #:",
    L"Merc",
    L"Giorni",
    L"Tasso",  // 5
    L"Costo",
    L"Totale:",
    L"Conferma il pagamento di %s?",  // the %s is a string that contains the dollar amount ( ex.
                                      // "$150" )
};

// For use at the M.E.R.C. web site. Text relating a MERC mercenary

static wchar_t* it_MercInfo[] = {
    L"Salute",
    L"Agilità",
    L"Destrezza",
    L"Forza",
    L"Comando",
    L"Saggezza",
    L"Liv. esperienza",
    L"Mira",
    L"Meccanica",
    L"Esplosivi",
    L"Pronto socc.",

    L"Indietro",
    L"Ricompensa",
    L"Successivo",
    L"Info. addizionali",
    L"Home Page",
    L"Assoldato",
    L"Salario:",
    L"Al giorno",
    L"Deceduto",

    L"Sembra che state arruolando troppi mercenari. Il vostro limite è di 18.",
    L"Non disponibile",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static wchar_t* it_MercNoAccountText[] = {
    // Text on the buttons at the bottom of the screen
    L"Apri conto",
    L"Annulla",
    L"Non hai alcun conto. Vuoi aprirne uno?",
};

// For use at the M.E.R.C. web site. MERC Homepage

static wchar_t* it_MercHomePageText[] = {
    // Description of various parts on the MERC page
    L"Speck T. Kline, fondatore e proprietario",
    L"Per aprire un conto, cliccate qui",
    L"Per visualizzare un conto, cliccate qui",
    L"Per visualizzare i file, cliccate qui",
    // The version number on the video conferencing system that pops up when Speck is talking
    L"Speck Com v3.2",
};

// For use at MiGillicutty's Web Page.

static wchar_t* it_sFuneralString[] = {
    L"Impresa di pompe funebri di McGillicutty: Il dolore delle famiglie che hanno fornito il loro "
    L"aiuto dal 1983.",
    L"Precedentemente mercenario dell'A.I.M. Murray \"Pops\" McGillicutty è un impresario di pompe "
    L"funebri qualificato e con molta esperienza.",
    L"Essendo coinvolto profondamente nella morte e nel lutto per tutta la sua vita, Pops sa "
    L"quanto sia difficile affrontarli.",
    L"L'impresa di pompe funebri di McGillicutty offre una vasta gamma di servizi funebri, da una "
    L"spalla su cui piangere a ricostruzioni post-mortem per corpi mutilati o sfigurati.",
    L"Lasciate che l'impresa di pompe funebri di McGillicutty vi aiuti e i vostri amati "
    L"riposeranno in pace.",

    // Text for the various links available at the bottom of the page
    L"SPEDISCI FIORI",
    L"ESPOSIZIONE DI BARE & URNE",
    L"SERVIZI DI CREMAZIONE",
    L"SERVIZI PRE-FUNEBRI",
    L"CERIMONIALE FUNEBRE",

    // The text that comes up when you click on any of the links ( except for send flowers ).
    L"Purtroppo, il resto di questo sito non è stato completato a causa di una morte in famiglia. "
    L"Aspettando la lettura del testamento e la riscossione dell'eredità, il sito verrà ultimato "
    L"non appena possibile.",
    L"Vi porgiamo, comunque, le nostre condolianze in questo momento di dolore. Contatteci ancora.",
};

// Text for the florist Home Page

static wchar_t* it_sFloristText[] = {
    // Text on the button on the bottom of the page

    L"Galleria",

    // Address of United Florist

    L"\"Ci lanciamo col paracadute ovunque\"",
    L"1-555-SCENT-ME",
    L"333 Dot. NoseGay, Seedy City, CA USA 90210",
    L"http://www.scent-me.com",

    // detail of the florist page

    L"Siamo veloci ed efficienti!",
    L"Consegna il giorno successivo in quasi tutto il mondo, garantito. Applicate alcune "
    L"restrizioni.",
    L"I prezzi più bassi in tutto il mondo, garantito!",
    L"Mostrateci un prezzo concorrente più basso per qualsiasi progetto, e riceverete una dozzina "
    L"di rose, gratuitamente.",
    L"Flora, fauna & fiori in volo dal 1981.",
    L"I nostri paracadutisti decorati ex-bomber lanceranno il vostro bouquet entro un raggio di "
    L"dieci miglia dalla locazione richiesta. Sempre e ovunque!",
    L"Soddisfiamo la vostra fantasia floreale.",
    L"Lasciate che Bruce, il nostro esperto in composizioni floreali, selezioni con cura i fiori "
    L"più freschi e della migliore qualità dalle nostre serre più esclusive.",
    L"E ricordate, se non l'abbiamo, possiamo coltivarlo - E subito!",
};

// Florist OrderForm

static wchar_t* it_sOrderFormText[] = {
    // Text on the buttons

    L"Indietro",
    L"Spedisci",
    L"Home",
    L"Galleria",

    L"Nome del bouquet:",
    L"Prezzo:",  // 5
    L"Numero ordine:",
    L"Data consegna",
    L"gior. succ.",
    L"arriva quando arriva",
    L"Luogo consegna",  // 10
    L"Servizi aggiuntivi",
    L"Bouquet schiacciato ($10)",
    L"Rose nere ($20)",
    L"Bouquet appassito ($10)",
    L"Torta di frutta (se disponibile 10$)",  // 15
    L"Sentimenti personali:",
    L"Il vostro messaggio non può essere più lungo di 75 caratteri.",
    L"... oppure sceglietene uno dai nostri",

    L"BIGLIETTI STANDARD",
    L"Informazioni sulla fatturazione",  // 20

    // The text that goes beside the area where the user can enter their name

    L"Nome:",
};

// Florist Gallery.c

static wchar_t* it_sFloristGalleryText[] = {
    // text on the buttons

    L"Prec.",  // abbreviation for previous
    L"Succ.",  // abbreviation for next

    L"Clicca sul modello che vuoi ordinare.",
    L"Ricorda: c'è un supplemento di 10$ per tutti i bouquet appassiti o schiacciati.",

    // text on the button

    L"Home",
};

// Florist Cards

static wchar_t* it_sFloristCards[] = {
    L"Cliccate sulla vostra selezione",
    L"Indietro",
};

// Text for Bobby Ray's Mail Order Site

static wchar_t* it_BobbyROrderFormText[] = {
    L"Ordine",                       // Title of the page
    L"Qta",                          // The number of items ordered
    L"Peso (%s)",                    // The weight of the item
    L"Nome oggetto",                 // The name of the item
    L"Prezzo unit.",                 // the item's weight
    L"Totale",                       // 5	// The total price of all of items of the same type
    L"Sotto-totale",                 // The sub total of all the item totals added
    L"S&C (Vedete luogo consegna)",  // S&H is an acronym for Shipping and Handling
    L"Totale finale",  // The grand total of all item totals + the shipping and handling
    L"Luogo consegna",
    L"Spedizione veloce",       // 10	// See below
    L"Costo (per %s.)",         // The cost to ship the items
    L"Espresso di notte",       // Gets deliverd the next day
    L"2 giorni d'affari",       // Gets delivered in 2 days
    L"Servizio standard",       // Gets delivered in 3 days
    L"Annulla ordine",          // 15			// Clears the order page
    L"Accetta ordine",          // Accept the order
    L"Indietro",                // text on the button that returns to the previous page
    L"Home Page",               // Text on the button that returns to the home Page
    L"* Indica oggetti usati",  // Disclaimer stating that the item is used
    L"Non potete permettervi di pagare questo.",  // 20	// A popup message that to warn of not
                                                  // enough money
    L"<NESSUNO>",  // Gets displayed when there is no valid city selected
    L"Siete sicuri di volere spedire quest'ordine a %s?",  // A popup that asks if the city selected
                                                           // is the correct one
    L"peso del pacco**",                                   // Displays the weight of the package
    L"** Peso min.",  // Disclaimer states that there is a minimum weight for the package
    L"Spedizioni",
};

// This text is used when on the various Bobby Ray Web site pages that sell items

static wchar_t* it_BobbyRText[] = {
    L"Ordini:",  // Title
    // instructions on how to order
    L"Cliccate sull'oggetto. Sinistro per aggiungere pezzi, destro per toglierne. Una volta "
    L"selezionata la quantità, procedete col nuovo ordine.",

    // Text on the buttons to go the various links

    L"Oggetti prec.",  //
    L"Armi",           // 3
    L"Munizioni",      // 4
    L"Giubb. A-P",     // 5
    L"Varie",          // 6	//misc is an abbreviation for miscellaneous
    L"Usato",          // 7
    L"Oggetti succ.",
    L"ORDINE",
    L"Home Page",  // 10

    // The following 2 lines are used on the Ammunition page.
    // They are used for help text to display how many items the player's merc has
    // that can use this type of ammo

    L"La vostra squadra ha",                          // 11
    L"arma(i) che usa(no) questo tipo di munizioni",  // 12

    // The following lines provide information on the items

    L"Peso:",               // Weight of all the items of the same type
    L"Cal.:",               // the caliber of the gun
    L"Mag.:",               // number of rounds of ammo the Magazine can hold
    L"Git.:",               // The range of the gun
    L"Dan.:",               // Damage of the weapon
    L"FFA:",                // Weapon's Rate Of Fire, acronym ROF
    L"Costo:",              // Cost of the item
    L"Inventario:",         // The number of items still in the store's inventory
    L"Num. ordine:",        // The number of items on order
    L"Danneggiato",         // If the item is damaged
    L"Peso:",               // the Weight of the item
    L"Totale:",             // The total cost of all items on order
    L"* funzionale al %%",  // if the item is damaged, displays the percent function of the item

    // Popup that tells the player that they can only order 10 items at a time

    L"Darn! Quest'ordine qui accetterà solo 10 oggetti. Se avete intenzione di ordinare più merce "
    L"(ed è quello che speriamo), fate un ordine a parte e accettate le nostre scuse.",

    // A popup that tells the user that they are trying to order more items then the store has in
    // stock

    L"Ci dispiace. Non disponiamo più di questo articolo. Riprovate più tardi.",

    // A popup that tells the user that the store is temporarily sold out

    L"Ci dispiace, ma siamo momentaneamente sprovvisti di oggetti di questo genere.",

};

// Text for Bobby Ray's Home Page

static wchar_t* it_BobbyRaysFrontText[] = {
    // Details on the web site

    L"Questo è il negozio con la fornitura militare e le armi più recenti e potenti!",
    L"Possiamo trovare la soluzione perfetta per tutte le vostre esigenze riguardo agli esplosivi.",
    L"Oggetti usati e riparati",

    // Text for the various links to the sub pages

    L"Varie",
    L"ARMI",
    L"MUNIZIONI",  // 5
    L"GIUBB. A-P",

    // Details on the web site

    L"Se non lo vendiamo, non potrete averlo!",
    L"In costruzione",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug
// shot page

static wchar_t* it_AimSortText[] = {
    L"Membri dell'A.I.M.",  // Title
    // Title for the way to sort
    L"Ordine per:",

    // sort by...

    L"Prezzo",
    L"Esperienza",
    L"Mira",
    L"Pronto socc.",
    L"Esplosivi",
    L"Meccanica",

    // Text of the links to other AIM pages

    L"Visualizza le facce dei mercenari disponibili",
    L"Rivedi il file di ogni singolo mercenario",
    L"Visualizza la galleria degli associati dell'A.I.M.",

    // text to display how the entries will be sorted

    L"Crescente",
    L"Decrescente",
};

// Aim Policies.c
// The page in which the AIM policies and regulations are displayed

static wchar_t* it_AimPolicyText[] = {
    // The text on the buttons at the bottom of the page

    L"Indietro", L"Home Page", L"Indice", L"Avanti", L"Disaccordo", L"Accordo",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot
// index

static wchar_t* it_AimMemberText[] = {
    L"Clic sinistro",
    L"per contattarlo",
    L"Clic destro",
    L"per i mercenari disponibili.",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

static wchar_t* it_CharacterInfo[] = {
    // The various attributes of the merc

    L"Salute", L"Agilità", L"Destrezza", L"Forza", L"Comando", L"Saggezza", L"Esperienza", L"Mira",
    L"Meccanica", L"Esplosivi",
    L"Pronto socc.",  // 10

    // the contract expenses' area

    L"Paga", L"Durata", L"1 giorno", L"1 settimana", L"2 settimane",

    // text for the buttons that either go to the previous merc,
    // start talking to the merc, or go to the next merc

    L"Indietro", L"Contratto", L"Avanti",

    L"Ulteriori informazioni",     // Title for the additional info for the merc's bio
    L"Membri attivi",              // 20		// Title of the page
    L"Dispositivo opzionale:",     // Displays the optional gear cost
    L"Deposito MEDICO richiesto",  // If the merc required a medical deposit, this is displayed
};

// Aim Member.c
// The page in which the player's hires AIM mercenaries

// The following text is used with the video conference popup

static wchar_t* it_VideoConfercingText[] = {
    L"Costo del contratto:",  // Title beside the cost of hiring the merc

    // Text on the buttons to select the length of time the merc can be hired

    L"1 giorno", L"1 settimana", L"2 settimane",

    // Text on the buttons to determine if you want the merc to come with the equipment

    L"Nessun equip.", L"Compra equip.",

    // Text on the Buttons

    L"TRASFERISCI FONDI",  // to actually hire the merc
    L"ANNULLA",            // go back to the previous menu
    L"ARRUOLA",            // go to menu in which you can hire the merc
    L"TACI",               // stops talking with the merc
    L"OK",
    L"LASCIA MESSAGGIO",  // if the merc is not there, you can leave a message

    // Text on the top of the video conference popup

    L"Videoconferenza con", L"Connessione...",

    L"con deposito"  // Displays if you are hiring the merc with the medical deposit
};

// Aim Member.c
// The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static wchar_t* it_AimPopUpText[] = {
    L"TRASFERIMENTO ELETTRONICO FONDI RIUSCITO",  // You hired the merc
    L"NON IN GRADO DI TRASFERIRE",                // Player doesn't have enough money, message 1
    L"FONDI INSUFFICIENTI",                       // Player doesn't have enough money, message 2

    // if the merc is not available, one of the following is displayed over the merc's face

    L"In missione",
    L"Lascia messaggio",
    L"Deceduto",

    // If you try to hire more mercs than game can support

    L"Avete già una squadra di 18 mercenari.",

    L"Messaggio già registrato",
    L"Messaggio registrato",
};

// AIM Link.c

static wchar_t* it_AimLinkText[] = {
    L"Collegamenti dell'A.I.M.",  // The title of the AIM links page
};

// Aim History

// This page displays the history of AIM

static wchar_t* it_AimHistoryText[] = {
    L"Storia dell'A.I.M.",  // Title

    // Text on the buttons at the bottom of the page

    L"Indietro",
    L"Home Page",
    L"Associati",
    L"Avanti",
};

// Aim Mug Shot Index

// The page in which all the AIM members' portraits are displayed in the order selected by the AIM
// sort page.

static wchar_t* it_AimFiText[] = {
    // displays the way in which the mercs were sorted

    L"Prezzo ",
    L"Esperienza",
    L"Mira",
    L"Pronto socc.",
    L"Esplosivi",
    L"Meccanica",

    // The title of the page, the above text gets added at the end of this text

    L"Membri scelti dell'A.I.M. in ordine crescente secondo %s",
    L"Membri scelti dell'A.I.M. in ordine decrescente secondo %s",

    // Instructions to the players on what to do

    L"Clic sinistro",
    L"Per scegliere un mercenario.",  // 10
    L"Clic destro",
    L"Per selezionare opzioni",

    // Gets displayed on top of the merc's portrait if they are...

    L"Via",
    L"Deceduto",  // 14
    L"In missione",
};

// AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer
// with AIM

static wchar_t* it_AimAlumniText[] = {
    // Text of the buttons

    L"PAGINA 1", L"PAGINA 2", L"PAGINA 3",

    L"Membri dell'A.I.M.",  // Title of the page

    L"FINE"  // Stops displaying information on selected merc
};

// AIM Home Page

static wchar_t* it_AimScreenText[] = {
    // AIM disclaimers

    L"A.I.M. e il logo A.I.M. sono marchi registrati in diversi paesi.",
    L"Di conseguenza, non cercate di copiarci.",
    L"Copyright 1998-1999 A.I.M., Ltd. Tutti i diritti riservati.",

    // Text for an advertisement that gets displayed on the AIM page

    L"Servizi riuniti floreali",
    L"\"Atterriamo col paracadute ovunque\"",  // 10
    L"Fallo bene",
    L"... la prima volta",
    L"Se non abbiamo armi e oggetti, non ne avrete bisogno.",
};

// Aim Home Page

static wchar_t* it_AimBottomMenuText[] = {
    // Text for the links at the bottom of all AIM pages
    L"Home Page", L"Membri", L"Associati", L"Assicurazioni", L"Storia", L"Collegamenti",
};

// ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static wchar_t* it_SKI_Text[] = {
    L"MERCANZIA IN STOCK",    // Header for the merchandise available
    L"PAGINA",                // The current store inventory page being displayed
    L"COSTO TOTALE",          // The total cost of the the items in the Dealer inventory area
    L"VALORE TOTALE",         // The total value of items player wishes to sell
    L"STIMATO",               // Button text for dealer to evaluate items the player wants to sell
    L"TRANSAZIONE",           // Button text which completes the deal. Makes the transaction.
    L"FINE",                  // Text for the button which will leave the shopkeeper interface.
    L"COSTO DI RIPARAZIONE",  // The amount the dealer will charge to repair the merc's goods
    L"1 ORA",     // SINGULAR! The text underneath the inventory slot when an item is given to the
                  // dealer to be repaired
    L"%d ORE",    // PLURAL!   The text underneath the inventory slot when an item is given to the
                  // dealer to be repaired
    L"RIPARATO",  // Text appearing over an item that has just been repaired by a NPC repairman
                  // dealer
    L"Non c'è abbastanza spazio nel vostro margine di ordine.",  // Message box that tells the user
                                                                 // there is no more room to put
                                                                 // there stuff
    L"%d MINUTI",  // The text underneath the inventory slot when an item is given to the dealer to
                   // be repaired
    L"Lascia oggetto a terra.",
};

// ShopKeeper Interface
// for the bank machine panels. Referenced here is the acronym ATM, which means Automatic Teller
// Machine

static wchar_t* it_SkiAtmText[] = {
    // Text on buttons on the banking machine, displayed at the bottom of the page
    L"0",       L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
    L"OK",       // Transfer the money
    L"Prendi",   // Take money from the player
    L"Dai",      // Give money to the player
    L"Annulla",  // Cancel the transfer
    L"Pulisci",  // Clear the money display
};

// Shopkeeper Interface
static wchar_t* it_gzSkiAtmText[] = {

    // Text on the bank machine panel that....
    L"Seleziona tipo",   // tells the user to select either to give or take from the merc
    L"Inserisci somma",  // Enter the amount to transfer
    L"Trasferisci fondi al mercenario",   // Giving money to the merc
    L"Trasferisci fondi dal mercenario",  // Taking money from the merc
    L"Fondi insufficienti",               // Not enough money to transfer
    L"Bilancio",                          // Display the amount of money the player currently has
};

static wchar_t* it_SkiMessageBoxText[] = {
    L"Volete sottrarre %s dal vostro conto principale per coprire la differenza?",
    L"Fondi insufficienti. Avete pochi %s",
    L"Volete sottrarre %s dal vostro conto principale per coprire la spesa?",
    L"Rivolgetevi all'operatore per iniziare la transazione",
    L"Rivolgetevi all'operatore per riparare gli oggetti selezionati",
    L"Fine conversazione",
    L"Bilancio attuale",
};

// OptionScreen.c

static wchar_t* it_zOptionsText[] = {
    // button Text
    L"Salva partita",
    L"Carica partita",
    L"Abbandona",
    L"Fine",

    // Text above the slider bars
    L"Effetti",
    L"Parlato",
    L"Musica",

    // Confirmation pop when the user selects..
    L"Volete terminare la partita e tornare al menu principale?",

    L"Avete bisogno dell'opzione 'Parlato' o di quella 'Sottotitoli' per poter giocare.",
};

// SaveLoadScreen
static wchar_t* it_zSaveLoadText[] = {
    L"Salva partita",
    L"Carica partita",
    L"Annulla",
    L"Salvamento selezionato",
    L"Caricamento selezionato",

    L"Partita salvata con successo",
    L"ERRORE durante il salvataggio!",
    L"Partita caricata con successo",
    L"ERRORE durante il caricamento!",

    L"La versione del gioco nel file della partita salvata è diverso dalla versione attuale. È "
    L"abbastanza sicuro proseguire. Continuate?",
    L"I file della partita salvata potrebbero essere annullati. Volete cancellarli tutti?",

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"La versionbe salvata è cambiata. Fateci avere un report, se incontrate problemi. Continuate?",
#else
    L"Tentativo di caricare una versione salvata più vecchia. Aggiornate e caricate "
    L"automaticamente quella salvata?",
#endif

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"La versione salvata e la versione della partita sono cambiate. Fateci avere un report, se "
    L"incontrate problemi. Continuate?",
#else
    L"Tentativo di caricare una vecchia versione salvata. Aggiornate e caricate automaticamente "
    L"quella salvata?",
#endif

    L"Siete sicuri di volere sovrascrivere la partita salvata nello slot #%d?",
    L"Volete caricare la partita dallo slot #",

    // The first %d is a number that contains the amount of free space on the users hard drive,
    // the second is the recommended amount of free space.
    L"Lo spazio su disco si sta esaurendo. Sono disponibili solo %d MB, mentre per giocare a "
    L"Jagged dovrebbero esserci almeno %d MB liberi .",

    L"Salvataggio in corso...",  // When saving a game, a message box with this string appears on
                                 // the screen

    L"Armi normali",
    L"Tonn. di armi",
    L"Stile realistico",
    L"Stile fantascientifico",

    L"Difficoltà",
};

// MapScreen
static wchar_t* it_zMarksMapScreenText[] = {
    L"Livello mappa",
    L"Non avete soldati. Avete bisogno di addestrare gli abitanti della città per poter disporre "
    L"di un esercito cittadino.",
    L"Entrata giornaliera",
    L"Il mercenario ha l'assicurazione sulla vita",
    L"%s non è stanco.",
    L"%s si sta muovendo e non può riposare",
    L"%s è troppo stanco, prova un po' più tardi.",
    L"%s sta guidando.",
    L"La squadra non può muoversi, se un mercenario dorme.",

    // stuff for contracts
    L"Visto che non potete pagare il contratto, non avete neanche i soldi per coprire il premio "
    L"dell'assicurazione sulla vita di questo nercenario.",
    L"%s premio dell'assicurazione costerà %s per %d giorno(i) extra. Volete pagare?",
    L"Settore inventario",
    L"Il mercenario ha una copertura medica.",

    // other items
    L"Medici",    // people acting a field medics and bandaging wounded mercs
    L"Pazienti",  // people who are being bandaged by a medic
    L"Fine",      // Continue on with the game after autobandage is complete
    L"Ferma",     // Stop autobandaging of patients by medics now
    L"Siamo spiacenti. Questa opzione è stata disabilitata in questo demo.",  // informs player this
                                                                              // option/button has
                                                                              // been disabled in
                                                                              // the demo
    L"%s non ha un kit di riparazione.",
    L"%s non ha un kit di riparazione.",
    L"Non ci sono abbastanza persone che vogliono essere addestrate ora.",
    L"%s è pieno di soldati.",
    L"Il mercenario ha un contratto a tempo determinato.",
    L"Il contratto del mercenario non è assicurato",
};

static wchar_t* it_pLandMarkInSectorString[] = {
    L"La squadra %d ha notato qualcuno nel settore %s",
};

// confirm the player wants to pay X dollars to build a militia force in town
static wchar_t* it_pMilitiaConfirmStrings[] = {
    L"Addestrare una squadra dell'esercito cittadino costerà $",  // telling player how much it will
                                                                  // cost
    L"Approvate la spesa?",        // asking player if they wish to pay the amount requested
    L"Non potete permettervelo.",  // telling the player they can't afford to train this town
    L"Continuate ad aeddestrare i soldati in %s (%s %d)?",  // continue training this town?
    L"Costo $",                                             // the cost in dollars to train militia
    L"(S/N)",                                               // abbreviated yes/no
    L"",                                                    // unused
    L"Addestrare l'esrecito cittadino nei settori di %d costerà $ %d. %s",  // cost to train sveral
                                                                            // sectors at once
    L"Non potete permettervi il $%d per addestrare l'esercito cittadino qui.",
    L"%s ha bisogno di una percentuale di %d affinché possiate continuare ad addestrare i soldati.",
    L"Non potete più addestrare i soldati a %s.",
};

// Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom
// of the single merc panel
static wchar_t* it_gzMoneyWithdrawMessageText[] = {
    L"Potete prelevare solo fino a $20,000 alla volta.",
    L"Sieti sicuri di voler depositare il %s sul vostro conto?",
};

static wchar_t* it_gzCopyrightText[] = {
    L"Copyright (C) 1999 Sir-tech Canada Ltd. Tutti i diritti riservati.",
};

// option Text
static wchar_t* it_zOptionsToggleText[] = {
    L"Parlato",
    L"Conferme mute",
    L"Sottotitoli",
    L"Mettete in pausa il testo del dialogo",
    L"Fumo dinamico",
    L"Sangue e violenza",
    L"Non è necessario usare il mouse!",
    L"Vecchio metodo di selezione",
    L"Mostra il percorso dei mercenari",
    L"Mostra traiettoria colpi sbagliati",
    L"Conferma in tempo reale",
    L"Visualizza gli avvertimenti sveglio/addormentato",
    L"Utilizza il sistema metrico",
    L"Tragitto illuminato durante gli spostamenti",
    L"Sposta il cursore sui mercenari",
    L"Sposta il cursore sulle porte",
    L"Evidenzia gli oggetti",
    L"Mostra le fronde degli alberi",
    L"Mostra strutture",
    L"Mostra il cursore 3D",
};

// This is the help text associated with the above toggles.
static wchar_t* it_zOptionsScreenHelpText[] = {
    // speech
    L"Attivate questa opzione, se volete ascoltare il dialogo dei personaggi.",

    // Mute Confirmation
    L"Attivate o disattivate le conferme verbali dei personaggi.",

    // Subtitles
    L"Controllate se il testo su schermo viene visualizzato per il dialogo.",

    // Key to advance speech
    L"Se i sottotitoli sono attivati, utilizzate questa opzione per leggere tranquillamente i "
    L"dialoghi NPC.",

    // Toggle smoke animation
    L"Disattivate questa opzione, se il fumo dinamico diminuisce la frequenza d'aggiornamento.",

    // Blood n Gore
    L"Disattivate questa opzione, se il sangue vi disturba.",

    // Never move my mouse
    L"Disattivate questa opzione per muovere automaticamente il mouse sulle finestre a comparsa di "
    L"conferma al loro apparire.",

    // Old selection method
    L"Attivate questa opzione per selezionare i personaggi e muoverli come nel vecchio JA (dato "
    L"che la funzione è stata invertita).",

    // Show movement path
    L"Attivate questa opzione per visualizzare i sentieri di movimento in tempo reale (oppure "
    L"disattivatela utilizzando il tasto MAIUSC).",

    // show misses
    L"Attivate per far sì che la partita vi mostri dove finiscono i proiettili quando "
    L"\"sbagliate\".",

    // Real Time Confirmation
    L"Se attivata, sarà richiesto un altro clic su \"salva\" per il movimento in tempo reale.",

    // Sleep/Wake notification
    L"Se attivata, verrete avvisati quando i mercenari in \"servizio\" vanno a riposare e quando "
    L"rientrano in servizio.",

    // Use the metric system
    L"Se attivata, utilizza il sistema metrico di misurazione; altrimenti ricorre al sistema "
    L"britannico.",

    // Merc Lighted movement
    L"Se attivata, il mercenario mostrerà il terreno su cui cammina. Disattivatela per un "
    L"aggiornamento più veloce.",

    // Smart cursor
    L"Se attivata, muovendo il cursore vicino ai vostri mercenari li evidenzierà automaticamente.",

    // snap cursor to the door
    L"Se attivata, muovendo il cursore vicino a una porta farà posizionare automaticamente il "
    L"cursore sopra di questa.",

    // glow items
    L"Se attivata, l'opzione evidenzierà gli |Oggetti automaticamente.",

    // toggle tree tops
    L"Se attivata, mostra le |fronde degli alberi.",

    // toggle wireframe
    L"Se attivata, visualizza le |Strutture dei muri nascosti.",

    L"Se attivata, il cursore di movimento verrà mostrato in 3D (|Home).",

};

static wchar_t* it_gzGIOScreenText[] = {
    L"INSTALLAZIONE INIZIALE DEL GIOCO",
    L"Versione del gioco",
    L"Realistica",
    L"Fantascientifica",
    L"Opzioni delle armi",
    L"Varietà di armi",
    L"Normale",
    L"Livello di difficoltà",
    L"Principiante",
    L"Esperto",
    L"Professionista",
    L"Ok",
    L"Annulla",
    L"Difficoltà extra",
    L"Tempo illimitato",
    L"Turni a tempo",
    L"Disabilitato per Demo",
};

static wchar_t* it_pDeliveryLocationStrings[] = {
    L"Austin",       // Austin, Texas, USA
    L"Baghdad",      // Baghdad, Iraq (Suddam Hussein's home)
    L"Drassen",      // The main place in JA2 that you can receive items.  The other towns are dummy
                     // names...
    L"Hong Kong",    // Hong Kong, Hong Kong
    L"Beirut",       // Beirut, Lebanon	(Middle East)
    L"Londra",       // London, England
    L"Los Angeles",  // Los Angeles, California, USA (SW corner of USA)
    L"Meduna",       // Meduna -- the other airport in JA2 that you can receive items.
    L"Metavira",     // The island of Metavira was the fictional location used by JA1
    L"Miami",        // Miami, Florida, USA (SE corner of USA)
    L"Mosca",        // Moscow, USSR
    L"New York",     // New York, New York, USA
    L"Ottawa",       // Ottawa, Ontario, Canada -- where JA2 was made!
    L"Parigi",       // Paris, France
    L"Tripoli",      // Tripoli, Libya (eastern Mediterranean)
    L"Tokyo",        // Tokyo, Japan
    L"Vancouver",    // Vancouver, British Columbia, Canada (west coast near US border)
};

static wchar_t* it_pSkillAtZeroWarning[] = {
    // This string is used in the IMP character generation.  It is possible to select 0 ability
    // in a skill meaning you can't use it.  This text is confirmation to the player.
    L"Siete sicuri? Un valore di zero significa NESSUNA abilità.",
};

static wchar_t* it_pIMPBeginScreenStrings[] = {
    L"(max 8 personaggi)",
};

static wchar_t* it_pIMPFinishButtonText[1] = {
    L"Analisi",
};

static wchar_t* it_pIMPFinishStrings[] = {
    L"Grazie, %s",  //%s is the name of the merc
};

// the strings for imp voices screen
static wchar_t* it_pIMPVoicesStrings[] = {
    L"Voce",
};

static wchar_t* it_pDepartedMercPortraitStrings[] = {
    L"Ucciso in azione",
    L"Licenziato",
    L"Altro",
};

// title for program
static wchar_t* it_pPersTitleText[] = {
    L"Manager del personale",
};

// paused game strings
static wchar_t* it_pPausedGameText[] = {
    L"Partita in pausa",
    L"Riprendi la partita (|P|a|u|s|a)",
    L"Metti in pausa la partita (|P|a|u|s|a)",
};

static wchar_t* it_pMessageStrings[] = {
    L"Vuoi uscire dalla partita?",
    L"OK",
    L"SÌ",
    L"NO",
    L"ANNULLA",
    L"RIASSUMI",
    L"MENTI",
    L"Nessuna descrizione",  // Save slots that don't have a description.
    L"Partita salvata.",
    L"Partita salvata.",
    L"Salvataggio rapido",  // The name of the quicksave file (filename, text reference)
    L"Partita salvata",     // The name of the normal savegame file, such as SaveGame01, SaveGame02,
                            // etc.
    L"salv.",               // The 3 character dos extension (represents sav)
    L"..\\Partite salvate",  // The name of the directory where games are saved.
    L"Giorno",
    L"Mercenari",
    L"Slot vuoto",  // An empty save game slot
    L"Demo",        // Demo of JA2
    L"Rimuovi",     // State of development of a project (JA2) that is a debug build
    L"Abbandona",   // Release build for JA2
    L"ppm",  // Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
    L"dm",   // Abbreviation for minute.
    L"m",    // One character abbreviation for meter (metric distance measurement unit).
    L"colpi",      // Abbreviation for rounds (# of bullets)
    L"kg",         // Abbreviation for kilogram (metric weight measurement unit)
    L"lb",         // Abbreviation for pounds (Imperial weight measurement unit)
    L"Home Page",  // Home as in homepage on the internet.
    L"USD",        // Abbreviation to US dollars
    L"n/a",        // Lowercase acronym for not applicable.
    L"In corso",   // Meanwhile
    L"%s si trova ora nel settore %s%s",  // Name/Squad has arrived in sector A9.  Order must not
                                          // change without notifying SirTech
    L"Versione",
    L"Slot di salvataggio rapido vuoto",
    L"Questo slot è riservato ai salvataggi rapidi fatti dalle schermate tattiche e dalla mappa "
    L"utilizzando ALT+S.",
    L"Aperto",
    L"Chiuso",
    L"Lo spazio su disco si sta esaurendo. Avete liberi solo %s MB e Jagged Alliance 2 ne richiede "
    L"%s.",
    L"Arruolato %s dall'A.I.M.",
    L"%s ha preso %s.",  //'Merc name' has caught 'item' -- let SirTech know if name comes after
                         // item.
    L"%s ha assunto della droga.",       //'Merc name' has taken the drug
    L"%s non ha alcuna abilità medica",  //'Merc name' has no medical skill.

    // CDRom errors (such as ejecting CD while attempting to read the CD)
    L"L'integrità del gioco è stata compromessa.",
    L"ERRORE: CD-ROM non valido",

    // When firing heavier weapons in close quarters, you may not have enough room to do so.
    L"Non c'è spazio per sparare da qui.",

    // Can't change stance due to objects in the way...
    L"Non potete cambiare posizione questa volta.",

    // Simple text indications that appear in the game, when the merc can do one of these things.
    L"Fai cadere",
    L"Getta",
    L"Passa",

    L"%s è passato a %s.",  //"Item" passed to "merc".  Please try to keep the item %s before the
                            // merc %s, otherwise, must notify SirTech.
    L"Nessun spazio per passare %s a %s.",  // pass "item" to "merc".  Same instructions as above.

    // A list of attachments appear after the items.  Ex:  Kevlar vest (Ceramic Plate 'Attached)'
    L" Compreso )",

    // Cheat modes
    L"Raggiunto il livello Cheat UNO",
    L"Raggiunto il livello Cheat DUE",

    // Toggling various stealth modes
    L"Squadra in modalità furtiva.",
    L"Squadra non in modalità furtiva.",
    L"%s in modalità furtiva.",
    L"%s non in modalità furtiva.",

    // Wireframes are shown through buildings to reveal doors and windows that can't otherwise be
    // seen in an isometric engine.  You can toggle this mode freely in the game.
    L"Strutture visibili",
    L"Strutture nascoste",

    // These are used in the cheat modes for changing levels in the game.  Going from a basement
    // level to an upper level, etc.
    L"Non potete passare al livello superiore...",
    L"Non esiste nessun livello inferiore...",
    L"Entra nel seminterrato %d...",
    L"Abbandona il seminterrato...",

    L"di",  // used in the shop keeper inteface to mark the ownership of the item eg Red's gun
    L"Modalità segui disattiva.",
    L"Modalità segui attiva.",
    L"Cursore 3D disattivo.",
    L"Cursore 3D attivo.",
    L"Squadra %d attiva.",
    L"Non potete permettervi di pagare a %s un salario giornaliero di %s",  // first %s is the mercs
                                                                            // name, the seconds is
                                                                            // a string containing
                                                                            // the salary
    L"Salta",
    L"%s non può andarsene da solo.",
    L"Un salvataggio è stato chiamato SaveGame99.sav. Se necessario, rinominatelo da SaveGame01 a "
    L"SaveGame10 e così potrete accedervi nella schermata di caricamento.",
    L"%s ha bevuto del %s",
    L"Un pacco è arivato a Drassen.",
    L"%s dovrebbe arrivare al punto designato di partenza (settore %s) nel giorno %d, "
    L"approssimativamente alle ore %s.",  // first %s is mercs name, next is the sector location and
                                          // name where they will be arriving in, lastely is the day
                                          // an the time of arrival
    L"Registro aggiornato.",
#ifdef JA2BETAVERSION
    L"Salvataggio riuscito della partita nello slot End Turn Auto Save.",
#endif
};

static wchar_t* it_ItemPickupHelpPopup[] = {
    L"OK", L"Scorrimento su", L"Seleziona tutto", L"Scorrimento giù", L"Annulla",
};

static wchar_t* it_pDoctorWarningString[] = {
    L"%s non è abbstanza vicina per poter esser riparata.",
    L"I vostri medici non sono riusciti a bendare completamente tutti.",
};

static wchar_t* it_pMilitiaButtonsHelpText[] = {
    L"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe verdi",  // button help text
                                                                                 // informing player
                                                                                 // they can pick up
                                                                                 // or drop militia
                                                                                 // with this button
    L"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe regolari",
    L"Raccogli (Clicca di destro)/lascia (Clicca di sinistro) le truppe veterane",
    L"Distribuisci equamente i soldati disponibili tra i vari settori",
};

static wchar_t* it_pMapScreenJustStartedHelpText[] = {
    L"Andate all'A.I.M. e arruolate alcuni mercenari (*Hint* è nel Laptop)",  // to inform the
                                                                              // player to hired
                                                                              // some mercs to get
                                                                              // things going
    L"Quando sarete pronti per partire per Arulco, cliccate sul pulsante nella parte in basso a "
    L"destra dello schermo.",  // to inform the player to hit time compression to get the game
                               // underway
};

static wchar_t* it_pAntiHackerString[] = {
    L"Errore. File mancanti o corrotti. Il gioco verrà completato ora.",
};

static wchar_t* it_gzLaptopHelpText[] = {
    // Buttons:
    L"Visualizza E-mail",
    L"Siti web",
    L"Visualizza file e gli attach delle E-mail",
    L"Legge il registro degli eventi",
    L"Visualizza le informazioni inerenti la squadra",
    L"Visualizza la situazione finanziaria e la storia",
    L"Chiude laptop",

    // Bottom task bar icons (if they exist):
    L"Avete nuove E-mail",
    L"Avete nuovi file",

    // Bookmarks:
    L"Associazione Internazionale Mercenari",
    L"Ordinativi di armi online dal sito di Bobby Ray",
    L"Istituto del Profilo del Mercenario",
    L"Centro più economico di reclutamento",
    L"Impresa di pompe funebri McGillicutty",
    L"Servizio Fioristi Riuniti",
    L"Contratti assicurativi per agenti A.I.M.",
};

static wchar_t* it_gzHelpScreenText[] = {
    L"Esci dalla schermata di aiuto",
};

static wchar_t* it_gzNonPersistantPBIText[] = {
    L"È in corso una battaglia. Potete solo ritirarvi dalla schermata delle tattiche.",
    L"|Entra nel settore per continuare l'attuale battaglia in corso.",
    L"|Automaticamente decide l'esito della battaglia in corso.",
    L"Non potete decidere l'esito della battaglia in corso automaticamente, se siete voi ad "
    L"attaccare.",
    L"Non potete decidere l'esito della battaglia in corso automaticamente, se subite "
    L"un'imboscata.",
    L"Non potete decidere l'esito della battaglia in corso automaticamente, se state combattendo "
    L"contro le creature nelle miniere.",
    L"Non potete decidere l'esito della battaglia in corso automaticamente, se ci sono civili "
    L"nemici.",
    L"Non potete decidere l'esito della battaglia in corso automaticamente, se ci sono dei "
    L"Bloodcat.",
    L"BATTAGLIA IN CORSO",
    L"Non potete ritirarvi ora.",
};

static wchar_t* it_gzMiscString[] = {
    L"I vostri soldati continuano a combattere senza l'aiuto dei vostri mercenari...",
    L"Il veicolo non ha più bisogno di carburante.",
    L"La tanica della benzina è piena %d%%.",
    L"L'esercito di Deidrannaha riguadagnato il controllo completo su %s.",
    L"Avete perso una stazione di rifornimento.",
};

static wchar_t* it_gzIntroScreen[] = {
    L"Video introduttivo non trovato",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static wchar_t* it_pNewNoiseStr[] = {
    L"%s sente un %s rumore proveniente da %s.",
    L"%s sente un %s rumore di MOVIMENTO proveniente da %s.",
    L"%s sente uno %s SCRICCHIOLIO proveniente da %s.",
    L"%s sente un %s TONFO NELL'ACQUA proveniente da %s.",
    L"%s sente un %s URTO proveniente da %s.",
    L"%s sente una %s ESPLOSIONE verso %s.",
    L"%s sente un %s URLO verso %s.",
    L"%s sente un %s IMPATTO verso %s.",
    L"%s sente un %s IMPATTO a %s.",
    L"%s sente un %s SCHIANTO proveniente da %s.",
    L"%s sente un %s FRASTUONO proveniente da %s.",
};

static wchar_t* it_wMapScreenSortButtonHelpText[] = {
    L"Nome (|F|1)",       L"Assegnato (|F|2)",    L"Tipo di riposo (|F|3)",
    L"Postazione (|F|4)", L"Destinazione (|F|5)", L"Durata dell'incarico (|F|6)",
};

static wchar_t* it_BrokenLinkText[] = {
    L"Errore 404",
    L"Luogo non trovato.",
};

static wchar_t* it_gzBobbyRShipmentText[] = {
    L"Spedizioni recenti",
    L"Ordine #",
    L"Numero di oggetti",
    L"Ordinato per",
};

static wchar_t* it_gzCreditNames[] = {
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

static wchar_t* it_gzCreditNameTitle[] = {
    L"Programmatore del gioco",                    // Chris Camfield
    L"Co-designer / Autore",                       // Shaun Lyng
    L"Programmatore sistemi strategici & Editor",  // Kris Marnes
    L"Produttore / Co-designer",                   // Ian Currie
    L"Co-designer / Designer della mappa",         // Linda Currie
    L"Grafico",                                    // Eric \"WTF\" Cheng
    L"Coordinatore beta, supporto",                // Lynn Holowka
    L"Grafico straordinario",                      // Norman \"NRG\" Olsen
    L"Guru dell'audio",                            // George Brooks
    L"Designer delle schermate / Grafico",         // Andrew Stacey
    L"Capo grafico / Animatore",                   // Scot Loving
    L"Capo programmatore",                         // Andrew \"Big Cheese Doddle\" Emmons
    L"Programmatore",                              // Dave French
    L"Programmatore sistemi & bilancio di gioco",  // Alex Meduna
    L"Grafico dei ritratti",                       // Joey \"Joeker\" Whelan",
};

static wchar_t* it_gzCreditNameFunny[] = {
    L"",                                                // Chris Camfield
    L"(deve ancora esercitarsi con la punteggiatura)",  // Shaun Lyng
    L"(\"Fatto. Devo solo perfezionarmi\")",            // Kris \"The Cow Rape Man\" Marnes
    L"(sta diventando troppo vecchio per questo)",      // Ian Currie
    L"(sta lavorando a Wizardry 8)",                    // Linda Currie
    L"(obbligato a occuparsi anche del CQ)",            // Eric \"WTF\" Cheng
    L"(ci ha lasciato per CFSA...)",                    // Lynn Holowka
    L"",                                                // Norman \"NRG\" Olsen
    L"",                                                // George Brooks
    L"(Testa matta e amante del jazz)",                 // Andrew Stacey
    L"(il suo nome vero è Robert)",                     // Scot Loving
    L"(l'unica persona responsabile)",                  // Andrew \"Big Cheese Doddle\" Emmons
    L"(può ora tornare al motocross)",                  // Dave French
    L"(rubato da Wizardry 8)",                          // Alex Meduna
    L"",                                                // Joey \"Joeker\" Whelan",
};

static wchar_t* it_sRepairsDoneString[] = {
    L"%s ha finito di riparare gli oggetti",
    L"%s ha finito di riparare le armi e i giubbotti antiproiettile di tutti",
    L"%s ha finito di riparare gli oggetti dell'equipaggiamento di tutti",
    L"%s ha finito di riparare gli oggetti trasportati di tutti",
};

static wchar_t* it_zGioDifConfirmText[] = {
    // L"You have chosen NOVICE mode. This setting is appropriate for those new to Jagged Alliance,
    // those new to strategy games in general, or those wishing shorter battles in the game. Your
    // choice will affect things throughout the entire course of the game, so choose wisely. Are you
    // sure you want to play in Novice mode?",
    L"Avete selezionato la modalità PRINCIPIANTE. Questo scenario è adatto a chi gioca per la "
    L"prima volta a Jagged Alliance, a chi prova a giocare per la prima volta in generale o a chi "
    L"desidera combattere battaglie più brevi nel gioco. La vostra decisione influirà sull'intero "
    L"corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella "
    L"modalità PRINCIPIANTE?",

    // L"You have chosen EXPERIENCED mode. This setting is suitable for those already familiar with
    // Jagged Alliance or similar games. Your choice will affect things throughout the entire course
    // of the game, so choose wisely. Are you sure you want to play in Experienced mode?",
    L"Avete selezionato la modalità ESPERTO. Questo scenario è adatto a chi ha già una certa "
    L"dimestichezza con Jagged Alliance o con giochi simili. La vostra decisione influirà "
    L"sull'intero corso della partita; scegliete, quindi, con attenzione. Siete sicuri di voler "
    L"giocare nella modalità ESPERTO?",

    // L"You have chosen EXPERT mode. We warned you. Don't blame us if you get shipped back in a
    // body bag. Your choice will affect things throughout the entire course of the game, so choose
    // wisely. Are you sure you want to play in Expert mode?",
    L"Avete selezionato la modalità PROFESSIONISTA. Siete avvertiti. Non malediteci, se vi "
    L"ritroverete a brandelli. La vostra decisione influirà sull'intero corso della partita; "
    L"scegliete, quindi, con attenzione. Siete sicuri di voler giocare nella modalità "
    L"PROFESSIONISTA?",

};

static wchar_t* it_gzLateLocalizedString[] = {
    L"%S file di dati della schermata di caricamento non trovato...",

    // 1-5
    L"Il robot non può lasciare questo settore, se nessuno sta usando il controller.",

    // This message comes up if you have pending bombs waiting to explode in tactical.
    L"Non potete comprimere il tempo ora. Aspettate le esplosioni!",

    //'Name' refuses to move.
    L"%s si rifiuta di muoversi.",

    //%s a merc name
    L"%s non ha abbastanza energia per cambiare posizione.",

    // A message that pops up when a vehicle runs out of gas.
    L"Il %s ha esaurito la benzina e ora è rimasto a piedi a %c%d.",

    // 6-10

    // the following two strings are combined with the pNewNoise[] strings above to report noises
    // heard above or below the merc
    L"sopra",
    L"sotto",

    // The following strings are used in autoresolve for autobandaging related feedback.
    L"Nessuno dei vostri mercenari non sa praticare il pronto soccorso.",
    L"Non ci sono supporti medici per bendare.",
    L"Non ci sono stati supporti medici sufficienti per bendare tutti.",
    L"Nessuno dei vostri mercenari ha bisogno di fasciature.",
    L"Fascia i mercenari automaticamento.",
    L"Tutti i vostri mercenari sono stati bendati.",

    // 14
    L"Arulco",

    L"(tetto)",

    L"Salute: %d/%d",

    // In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
    //"vs." is the abbreviation of versus.
    L"%d contro %d",

    L"Il %s è pieno!",  //(ex "The ice cream truck is full")

    L"%s non ha bisogno immediatamente di pronto soccorso o di fasciature, quanto piuttosto di "
    L"cure mediche più serie e/o riposo.",

    // 20
    // Happens when you get shot in the legs, and you fall down.
    L"%s è stato colpito alla gamba e collassa!",
    // Name can't speak right now.
    L"%s non può parlare ora.",

    // 22-24 plural versions
    L"%d l'esercito verde è stato promosso a veterano.",
    L"%d l'esercito verde è stato promosso a regolare.",
    L"%d l'esercito regolare è stato promosso a veterano.",

    // 25
    L"Interruttore",

    // 26
    // Name has gone psycho -- when the game forces the player into burstmode (certain unstable
    // characters)
    L"%s è impazzito!",

    // 27-28
    // Messages why a player can't time compress.
    L"Non è al momento sicuro comprimere il tempo visto che avete dei mercenari nel settore %s.",
    L"Non è al momento sicuro comprimere il tempo quando i mercenari sono nelle miniere infestate "
    L"dalle creature.",

    // 29-31 singular versions
    L"1 esercito verde è stato promosso a veterano.",
    L"1 esercito verde è stato promosso a regolare.",
    L"1 eserciro regolare è stato promosso a veterano.",

    // 32-34
    L"%s non dice nulla.",
    L"Andate in superficie?",
    L"(Squadra %d)",

    // 35
    // Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope,
    // Scope before MP5K)
    L"%s ha riparato %s's %s",

    // 36
    L"BLOODCAT",

    // 37-38 "Name trips and falls"
    L"%s trips and falls",
    L"Questo oggetto non può essere raccolto qui.",

    // 39
    L"Nessuno dei vostri rimanenti mercenari è in grado di combattere. L'esercito combatterà "
    L"contro le creature da solo.",

    // 40-43
    //%s is the name of merc.
    L"%s è rimasto sprovvisto di kit medici!",
    L"%s non è in grado di curare nessuno!",
    L"%s è rimasto sprovvisto di forniture mediche!",
    L"%s non è in grado di riparare niente!",

    // 44-45
    L"Tempo di riparazione",
    L"%s non può vedere questa persona.",

    // 46-48
    L"L'estensore della canna dell'arma di %s si è rotto!",
    L"Non più di %d allenatori di soldati sono ammessi in questo settore.",
    L"Siete sicuri?",

    // 49-50
    L"Compressione del tempo",
    L"La tanica della benzina del veicolo è ora piena.",

    // 51-52 Fast help text in mapscreen.
    L"Continua la compressione del tempo (|S|p|a|z|i|o)",
    L"Ferma la compressione del tempo (|E|s|c)",

    // 53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
    L"%s ha sbloccata il %s",
    L"%s ha sbloccato il %s di %s",

    // 55
    L"Non potete comprimere il tempo mentre visualizzate l'inventario del settore.",

    L"Il CD ddel gioco Jagged Alliance 2 non è stato trovato. Il programma verrà terminato.",

    L"Oggetti combinati con successo.",

    // 58
    // Displayed with the version information when cheats are enabled.
    L"Attuale/Massimo Progresso: %d%%/%d%%",

    // 59
    L"Accompagnate John e Mary?",

    L"Interruttore attivato.",
};

void UseTextItalian() {
  gzProsLabel = it_gzProsLabel;
  gzConsLabel = it_gzConsLabel;
  sRepairsDoneString = it_sRepairsDoneString;
  AmmoCaliber = it_AmmoCaliber;
  BobbyRayAmmoCaliber = it_BobbyRayAmmoCaliber;
  WeaponType = it_WeaponType;
  Message = it_Message;
  TeamTurnString = it_TeamTurnString;
  pAssignMenuStrings = it_pAssignMenuStrings;
  pTrainingStrings = it_pTrainingStrings;
  pTrainingMenuStrings = it_pTrainingMenuStrings;
  pAttributeMenuStrings = it_pAttributeMenuStrings;
  pVehicleStrings = it_pVehicleStrings;
  pShortAttributeStrings = it_pShortAttributeStrings;
  pLongAttributeStrings = it_pLongAttributeStrings;
  pContractStrings = it_pContractStrings;
  pAssignmentStrings = it_pAssignmentStrings;
  pConditionStrings = it_pConditionStrings;
  pTownNames = it_pTownNames;
  pPersonnelScreenStrings = it_pPersonnelScreenStrings;
  pPersonnelTitle = it_pPersonnelTitle;
  pUpperLeftMapScreenStrings = it_pUpperLeftMapScreenStrings;
  pTacticalPopupButtonStrings = it_pTacticalPopupButtonStrings;
  pSquadMenuStrings = it_pSquadMenuStrings;
  pDoorTrapStrings = it_pDoorTrapStrings;
  pLongAssignmentStrings = it_pLongAssignmentStrings;
  pContractExtendStrings = it_pContractExtendStrings;
  pMapScreenMouseRegionHelpText = it_pMapScreenMouseRegionHelpText;
  pPersonnelAssignmentStrings = it_pPersonnelAssignmentStrings;
  pNoiseVolStr = it_pNoiseVolStr;
  pNoiseTypeStr = it_pNoiseTypeStr;
  pDirectionStr = it_pDirectionStr;
  pRemoveMercStrings = it_pRemoveMercStrings;
  sTimeStrings = it_sTimeStrings;
  pLandTypeStrings = it_pLandTypeStrings;
  pGuardMenuStrings = it_pGuardMenuStrings;
  pOtherGuardMenuStrings = it_pOtherGuardMenuStrings;
  pInvPanelTitleStrings = it_pInvPanelTitleStrings;
  pPOWStrings = it_pPOWStrings;
  pMilitiaString = it_pMilitiaString;
  pMilitiaButtonString = it_pMilitiaButtonString;
  pEpcMenuStrings = it_pEpcMenuStrings;
  pRepairStrings = it_pRepairStrings;
  sPreStatBuildString = it_sPreStatBuildString;
  sStatGainStrings = it_sStatGainStrings;
  pHelicopterEtaStrings = it_pHelicopterEtaStrings;
  sMapLevelString = it_sMapLevelString;
  gsLoyalString = it_gsLoyalString;
  gsUndergroundString = it_gsUndergroundString;
  gsTimeStrings = it_gsTimeStrings;
  sFacilitiesStrings = it_sFacilitiesStrings;
  pMapPopUpInventoryText = it_pMapPopUpInventoryText;
  pwTownInfoStrings = it_pwTownInfoStrings;
  pwMineStrings = it_pwMineStrings;
  pwMiscSectorStrings = it_pwMiscSectorStrings;
  pMapInventoryErrorString = it_pMapInventoryErrorString;
  pMapInventoryStrings = it_pMapInventoryStrings;
  pMapScreenFastHelpTextList = it_pMapScreenFastHelpTextList;
  pMovementMenuStrings = it_pMovementMenuStrings;
  pUpdateMercStrings = it_pUpdateMercStrings;
  pMapScreenBorderButtonHelpText = it_pMapScreenBorderButtonHelpText;
  pMapScreenBottomFastHelp = it_pMapScreenBottomFastHelp;
  pMapScreenBottomText = it_pMapScreenBottomText;
  pMercDeadString = it_pMercDeadString;
  pSenderNameList = it_pSenderNameList;
  pTraverseStrings = it_pTraverseStrings;
  pNewMailStrings = it_pNewMailStrings;
  pDeleteMailStrings = it_pDeleteMailStrings;
  pEmailHeaders = it_pEmailHeaders;
  pEmailTitleText = it_pEmailTitleText;
  pFinanceTitle = it_pFinanceTitle;
  pFinanceSummary = it_pFinanceSummary;
  pFinanceHeaders = it_pFinanceHeaders;
  pTransactionText = it_pTransactionText;
  pTransactionAlternateText = it_pTransactionAlternateText;
  pMoralStrings = it_pMoralStrings;
  pSkyriderText = it_pSkyriderText;
  pLeftEquipmentString = it_pLeftEquipmentString;
  pMapScreenStatusStrings = it_pMapScreenStatusStrings;
  pMapScreenPrevNextCharButtonHelpText = it_pMapScreenPrevNextCharButtonHelpText;
  pEtaString = it_pEtaString;
  pShortVehicleStrings = it_pShortVehicleStrings;
  pTrashItemText = it_pTrashItemText;
  pMapErrorString = it_pMapErrorString;
  pMapPlotStrings = it_pMapPlotStrings;
  pMiscMapScreenMouseRegionHelpText = it_pMiscMapScreenMouseRegionHelpText;
  pMercHeLeaveString = it_pMercHeLeaveString;
  pMercSheLeaveString = it_pMercSheLeaveString;
  pImpPopUpStrings = it_pImpPopUpStrings;
  pImpButtonText = it_pImpButtonText;
  pExtraIMPStrings = it_pExtraIMPStrings;
  pFilesTitle = it_pFilesTitle;
  pFilesSenderList = it_pFilesSenderList;
  pHistoryLocations = it_pHistoryLocations;
  pHistoryStrings = it_pHistoryStrings;
  pHistoryHeaders = it_pHistoryHeaders;
  pHistoryTitle = it_pHistoryTitle;
  pShowBookmarkString = it_pShowBookmarkString;
  pWebPagesTitles = it_pWebPagesTitles;
  pWebTitle = it_pWebTitle;
  pPersonnelString = it_pPersonnelString;
  pErrorStrings = it_pErrorStrings;
  pDownloadString = it_pDownloadString;
  pBookmarkTitle = it_pBookmarkTitle;
  pBookMarkStrings = it_pBookMarkStrings;
  pLaptopIcons = it_pLaptopIcons;
  sATMText = it_sATMText;
  gsAtmStartButtonText = it_gsAtmStartButtonText;
  gsAtmSideButtonText = it_gsAtmSideButtonText;
  pPersonnelTeamStatsStrings = it_pPersonnelTeamStatsStrings;
  pPersonnelCurrentTeamStatsStrings = it_pPersonnelCurrentTeamStatsStrings;
  pPersonelTeamStrings = it_pPersonelTeamStrings;
  pPersonnelDepartedStateStrings = it_pPersonnelDepartedStateStrings;
  pMapHortIndex = it_pMapHortIndex;
  pMapVertIndex = it_pMapVertIndex;
  pMapDepthIndex = it_pMapDepthIndex;
  pLaptopTitles = it_pLaptopTitles;
  pDayStrings = it_pDayStrings;
  pMercContractOverStrings = it_pMercContractOverStrings;
  pMilitiaConfirmStrings = it_pMilitiaConfirmStrings;
  pDeliveryLocationStrings = it_pDeliveryLocationStrings;
  pSkillAtZeroWarning = it_pSkillAtZeroWarning;
  pIMPBeginScreenStrings = it_pIMPBeginScreenStrings;
  pIMPFinishButtonText = it_pIMPFinishButtonText;
  pIMPFinishStrings = it_pIMPFinishStrings;
  pIMPVoicesStrings = it_pIMPVoicesStrings;
  pDepartedMercPortraitStrings = it_pDepartedMercPortraitStrings;
  pPersTitleText = it_pPersTitleText;
  pPausedGameText = it_pPausedGameText;
  zOptionsToggleText = it_zOptionsToggleText;
  zOptionsScreenHelpText = it_zOptionsScreenHelpText;
  pDoctorWarningString = it_pDoctorWarningString;
  pMilitiaButtonsHelpText = it_pMilitiaButtonsHelpText;
  pMapScreenJustStartedHelpText = it_pMapScreenJustStartedHelpText;
  pLandMarkInSectorString = it_pLandMarkInSectorString;
  gzMercSkillText = it_gzMercSkillText;
  gzNonPersistantPBIText = it_gzNonPersistantPBIText;
  gzMiscString = it_gzMiscString;
  wMapScreenSortButtonHelpText = it_wMapScreenSortButtonHelpText;
  pNewNoiseStr = it_pNewNoiseStr;
  gzLateLocalizedString = it_gzLateLocalizedString;
  pAntiHackerString = it_pAntiHackerString;
  pMessageStrings = it_pMessageStrings;
  ItemPickupHelpPopup = it_ItemPickupHelpPopup;
  TacticalStr = it_TacticalStr;
  LargeTacticalStr = it_LargeTacticalStr;
  zDialogActions = it_zDialogActions;
  zDealerStrings = it_zDealerStrings;
  zTalkMenuStrings = it_zTalkMenuStrings;
  gzMoneyAmounts = it_gzMoneyAmounts;
  gMoneyStatsDesc = it_gMoneyStatsDesc;
  gWeaponStatsDesc = it_gWeaponStatsDesc;
  sKeyDescriptionStrings = it_sKeyDescriptionStrings;
  zHealthStr = it_zHealthStr;
  zVehicleName = it_zVehicleName;
  pExitingSectorHelpText = it_pExitingSectorHelpText;
  InsContractText = it_InsContractText;
  InsInfoText = it_InsInfoText;
  MercAccountText = it_MercAccountText;
  MercInfo = it_MercInfo;
  MercNoAccountText = it_MercNoAccountText;
  MercHomePageText = it_MercHomePageText;
  sFuneralString = it_sFuneralString;
  sFloristText = it_sFloristText;
  sOrderFormText = it_sOrderFormText;
  sFloristGalleryText = it_sFloristGalleryText;
  sFloristCards = it_sFloristCards;
  BobbyROrderFormText = it_BobbyROrderFormText;
  BobbyRText = it_BobbyRText;
  BobbyRaysFrontText = it_BobbyRaysFrontText;
  AimSortText = it_AimSortText;
  AimPolicyText = it_AimPolicyText;
  AimMemberText = it_AimMemberText;
  CharacterInfo = it_CharacterInfo;
  VideoConfercingText = it_VideoConfercingText;
  AimPopUpText = it_AimPopUpText;
  AimLinkText = it_AimLinkText;
  AimHistoryText = it_AimHistoryText;
  AimFiText = it_AimFiText;
  AimAlumniText = it_AimAlumniText;
  AimScreenText = it_AimScreenText;
  AimBottomMenuText = it_AimBottomMenuText;
  zMarksMapScreenText = it_zMarksMapScreenText;
  gpStrategicString = it_gpStrategicString;
  gpGameClockString = it_gpGameClockString;
  SKI_Text = it_SKI_Text;
  SkiAtmText = it_SkiAtmText;
  gzSkiAtmText = it_gzSkiAtmText;
  SkiMessageBoxText = it_SkiMessageBoxText;
  zSaveLoadText = it_zSaveLoadText;
  zOptionsText = it_zOptionsText;
  gzGIOScreenText = it_gzGIOScreenText;
  gzHelpScreenText = it_gzHelpScreenText;
  gzLaptopHelpText = it_gzLaptopHelpText;
  gzMoneyWithdrawMessageText = it_gzMoneyWithdrawMessageText;
  gzCopyrightText = it_gzCopyrightText;
  BrokenLinkText = it_BrokenLinkText;
  gzBobbyRShipmentText = it_gzBobbyRShipmentText;
  zGioDifConfirmText = it_zGioDifConfirmText;
  gzCreditNames = it_gzCreditNames;
  gzCreditNameTitle = it_gzCreditNameTitle;
  gzCreditNameFunny = it_gzCreditNameFunny;
  pUpdatePanelButtons = it_pUpdatePanelButtons;
  pBullseyeStrings = it_pBullseyeStrings;
  pContractButtonString = it_pContractButtonString;
}
