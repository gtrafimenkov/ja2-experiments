// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "Utils/Text.h"

// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
static wchar_t* ru_AmmoCaliber[] = {
    L"0",         L",38 кал", L"9мм",    L",45 кал", L",357 кал",
    L"12 кал",    L"ОББ",     L"5,45мм", L"5,56мм",  L"7,62мм НАТО",
    L"7,62мм ВД", L"4,7мм",   L"5,7мм",  L"Монстр",  L"Ракета",
    L"",  // дротик
    L"",  // пламя
};

// This BobbyRayAmmoCaliber is virtually the same as AmmoCaliber however the bobby version doesnt
// have as much room for the words.
//
// Different weapon calibres
// CAWS is Close Assault Weapon System and should probably be left as it is
// NATO is the North Atlantic Treaty Organization
// WP is Warsaw Pact
// cal is an abbreviation for calibre
static wchar_t* ru_BobbyRayAmmoCaliber[] = {
    L"0",      L",38 кал",   L"9мм",       L",45 кал", L",357 кал", L"12 кал", L"ОББ",    L"5,45мм",
    L"5,56мм", L"7,62мм Н.", L"7,62мм ВД", L"4,7мм",   L"5.7мм",    L"Монстр", L"Ракета",
    L"",  // дротик
};

static wchar_t* ru_WeaponType[] = {L"Другое",      L"Пистолет",       L"Автопистолет",
                                   L"Полуавтомат", L"Винтовка",       L"Снайп.винтовка",
                                   L"Базука",      L"Легкий автомат", L"Револьвер"};

static wchar_t* ru_TeamTurnString[] = {
    L"Ход Игрока",  // player's turn
    L"Ход Оппонента", L"Ход Существа", L"Ход Ополчения", L"Ход Жителей"
    // planning turn
};

static wchar_t* ru_Message[] = {
    L"",

    // In the following 8 strings, the %s is the merc's name, and the %d (if any) is a number.

    L"%s:попадание в голову. Теряет в мудрости!",
    L"%s получает рану плеча и теряет в ловкости!",
    L"%s получает рану в грудь и теряет в силе!",
    L"%s получает рану ног и теряет в проворности!",
    L"%s получает рану головы и теряет %d очков мудрости!",
    L"%s получает рану в плечо теряет %d очков ловкости!",
    L"%s получает рану в грудь и теряет %d очков силы!",
    L"%s получает рану ног и теряет %d очков проворности!",
    L"Перерыв!",

    // The first %s is a merc's name, the second is a string from pNoiseVolStr,
    // the third is a string from pNoiseTypeStr, and the last is a string from pDirectionStr

    L"",  // OBSOLETE
    L"Подкрепление прибыло!",

    // In the following four lines, all %s's are merc names

    L"%s заряжает",
    L"%s недостаточно очков действия!",
    L"%s оказывает перв.помощь.(люб.клавиша-отмена)",
    L"%s и %s оказывают перв.помощь. (люб.клавиша-отмена.)",
    // the following 17 strings are used to create lists of gun advantages and disadvantages
    // (separated by commas)
    L"надежен",
    L"ненадежен",
    L"легко починить",
    L"трудно почин.",
    L"сильн.поврежд.",
    L"слаб.поврежд.",
    L"быстр.огонь",
    L"медл.огонь",
    L"дальний бой",
    L"ближний бой",
    L"легкий",
    L"тяжелый",
    L"малый",
    L"очередями",
    L"не очередями",
    L"бол.обойма",
    L"мал.обойма",

    // In the following two lines, all %s's are merc names

    L"%s:камуфляж изношен",
    L"%s:окраска камуфляжа смыта",

    // The first %s is a merc name and the second %s is an item name

    L"Втор.оружие:нет патронов!",
    L"%s крадет %s.",

    // The %s is a merc name

    L"%s:оружие не стрел.очеред.",

    L"вы повторяетесь!",
    L"Объединить?",

    // Both %s's are item names

    L"Нельзя соединить %s и %s.",

    L"Ничего",
    L"Разрядить",
    L"Приложения",

    // You cannot use "item(s)" and your "other item" at the same time.
    // Ex:  You cannot use sun goggles and you gas mask at the same time.
    L"Нельзя использовать %s и %s одновр.",

    L"Вещь,на к-рую указывает курсор,можно присоединить к другим вещам,поместив ее в одну из "
    L"связных ячеек.",
    L"Вещь,на к-рую указывает курсор,можно присоединить к другим вещам,поместив ее в одну из "
    L"связных ячеек.(Однако эти вещи несовместимы.)",
    L"В этом секторе еще остались враги!",
    L"Тебе нужно дать %s %s",
    L"%s:попадание в голову!",
    L"Покинуть битву?",
    L"Эта вещь останется у тебя.Оставить ее?",
    L"%s чувствует прилив энергии!",
    L"%s скользит по мрамору!",
    L"%s не получает %s!",
    L"%s чинит %s",
    L"Прервать для ",
    L"Сдаться?",
    L"Человек отверг твою помощь",
    L"Я так НЕ ДУМАЮ!",
    L"Чтобы воспользоваться вертолетеом Всадника, выберите ПРИНАДЛЕЖНОСТЬ и МАШИНА.",
    L"%s успевает зарядить только один пистолет",
    L"ход Кошки-Убийцы",
};

// the names of the towns in the game

static wchar_t* ru_pTownNames[] = {
    L"",         L"Омерта", L"Драссен", L"Альма",  L"Грам",   L"Тикса",   L"Камбрия",
    L"Сан Мона", L"Эстони", L"Орта",    L"Балайм", L"Медуна", L"Читзена",
};

// the types of time compression. For example: is the timer paused? at normal speed, 5 minutes per
// second, etc. min is an abbreviation for minutes

static wchar_t* ru_sTimeStrings[] = {
    L"Пауза", L"Норма", L"5 мин", L"30 мин", L"60 мин", L"6 час",
};

// Assignment Strings: what assignment does the merc  have right now? For example, are they on a
// squad, training, administering medical aid (doctor) or training a town. All are abbreviated. 8
// letters is the longest it can be.

static wchar_t* ru_pAssignmentStrings[] = {
    L"Отряд1",    L"Отряд2",  L"Отряд3",  L"Отряд4",  L"Отряд5",  L"Отряд6",  L"Отряд7",
    L"Отряд8",    L"Отряд9",  L"Отряд10", L"Отряд11", L"Отряд12", L"Отряд13", L"Отряд14",
    L"Отряд15",   L"Отряд16", L"Отряд17", L"Отряд18", L"Отряд19", L"Отряд20",
    L"На службе",  // on active duty
    L"Доктор",     // оказывает медпомощь
    L"Пациент",    // принимает медпомощь
    L"Машина",     // in a vehicle
    L"В пути",     // транзитом - сокращение
    L"Ремонт",     // ремонтируются
    L"Практика",   // тренируются
    L"Ополчение",  // готовят восстание среди горожан
    L"Тренер",     // training a teammate
    L"Студент",    // being trained by someone else
    L"Мертв",      // мертв
    L"Беспом.",    // abbreviation for incapacitated
    L"ВП",         // Prisoner of war - captured
    L"Госпиталь",  // patient in a hospital
    L"Пусто",      // Vehicle is empty
};

static wchar_t* ru_pMilitiaString[] = {
    L"Ополчение",     // the title of the militia box
    L"Не определен",  // the number of unassigned militia troops
    L"Ты не можешь перераспределять ополчение, когда кругом враги!",
};

static wchar_t* ru_pMilitiaButtonString[] = {
    L"Авто",    // auto place the militia troops for the player
    L"Готово",  // done placing militia troops
};

static wchar_t* ru_pConditionStrings[] = {
    L"Отлично",       // состояние солдата..отличное здоровье
    L"Хорошо",        // хорошее здоровье
    L"Прилично",      // нормальное здоровье
    L"Ранен",         // раны
    L"Устал",         // усталый
    L"Кровоточит",    // истекает кровью
    L"Без сознания",  // в обмороке
    L"Умирает",       // умирает
    L"Мертв",         // мертв
};

static wchar_t* ru_pEpcMenuStrings[] = {
    L"На службе",    // set merc on active duty
    L"Пациент",      // set as a patient to receive medical aid
    L"Машина",       // tell merc to enter vehicle
    L"Без эскорта",  // охрана покидает героя
    L"Отмена",       // выход из этого меню
};

// look at pAssignmentString above for comments

static wchar_t* ru_pPersonnelAssignmentStrings[] = {
    L"Отряд1",  L"Отряд2",  L"Отряд3",  L"Отряд4",  L"Отряд5",  L"Отряд6",    L"Отряд7",
    L"Отряд8",  L"Отряд9",  L"Отряд10", L"Отряд11", L"Отряд12", L"Отряд13",   L"Отряд14",
    L"Отряд15", L"Отряд16", L"Отряд17", L"Отряд18", L"Отряд19", L"Отряд20",   L"На службе",
    L"Доктор",  L"Пациент", L"Машина",  L"В пути",  L"Ремонт",  L"Практика",  L"Тренировка ополч",
    L"Тренер",  L"Ученик",  L"Мертв",   L"Беспом.", L"ВП",      L"Госпиталь",
    L"Пусто",  // Vehicle is empty
};

// refer to above for comments

static wchar_t* ru_pLongAssignmentStrings[] = {
    L"Отряд1",    L"Отряд2",  L"Отряд3",    L"Отряд4",        L"Отряд5",          L"Отряд6",
    L"Отряд7",    L"Отряд8",  L"Отряд9",    L"Отряд10",       L"Отряд11",         L"Отряд12",
    L"Отряд13",   L"Отряд14", L"Отряд15",   L"Отряд16",       L"Отряд17",         L"Отряд18",
    L"Отряд19",   L"Отряд20", L"На службе", L"Доктор",        L"пациент",         L"Машина",
    L"В пути",    L"Ремонт",  L"Практика",  L"Тренинг ополч", L"Тренинг команды", L"Ученик",
    L"Мертв",     L"Беспом.", L"ВП",
    L"Госпиталь",  // patient in a hospital
    L"Пусто",      // Vehicle is empty
};

// the contract options

static wchar_t* ru_pContractStrings[] = {
    L"Пункты контракта:",
    L"",                 // a blank line, required
    L"Предл.1 день",     // offer merc a one day contract extension
    L"Предложить 7дн",   // 1 week
    L"Предложить 14дн",  // 2 week
    L"Уволить",          // end merc's contract
    L"Отмена",           // stop showing this menu
};

static wchar_t* ru_pPOWStrings[] = {
    L"ВП",  // an acronym for Prisoner of War
    L"??",
};

static wchar_t* ru_pLongAttributeStrings[] = {
    L"СИЛА",     L"ЛОВКОСТЬ", L"ПОДВИЖНОСТЬ", L"МУДРОСТЬ", L"МЕТКОСТЬ",
    L"МЕДИЦИНА", L"МЕХАНИКА", L"ЛИДЕРСТВО",   L"ВЗРЫВНИК", L"УРОВЕНЬ",
};

static wchar_t* ru_pInvPanelTitleStrings[] = {
    L"Броня",     // the armor rating of the merc
    L"Вес",       // the weight the merc is carrying
    L"Камуфляж",  // the merc's camouflage rating
};

static wchar_t* ru_pShortAttributeStrings[] = {
    L"Пдв",  // the abbreviated version of : agility
    L"Лов",  // dexterity
    L"Сил",  // strength
    L"Лид",  // leadership
    L"Мдр",  // wisdom
    L"Урв",  // experience level
    L"Мтк",  // marksmanship skill
    L"Взр",  // explosive skill
    L"Мех",  // mechanical skill
    L"Мед",  // medical skill};
};

static wchar_t* ru_pUpperLeftMapScreenStrings[] = {
    L"Принадлежность",  // the mercs current assignment
    L"Контракт",        // the contract info about the merc
    L"Здоровье",        // the health level of the current merc
    L"Дух",             // the morale of the current merc
    L"Сост.",           // the condition of the current vehicle
    L"Горючее",         // the fuel level of the current vehicle
};

static wchar_t* ru_pTrainingStrings[] = {
    L"Практика",   // tell merc to train self
    L"Ополчение",  // tell merc to train town
    L"Тренер",     // tell merc to act as trainer
    L"Ученик",     // tell merc to be train by other
};

static wchar_t* ru_pGuardMenuStrings[] = {
    L"Возм.стрелять:",         // the allowable rate of fire for a merc who is guarding
    L" Агрессивный огонь",     // the merc can be aggressive in their choice of fire rates
    L" Беречь патроны",        // conserve ammo
    L" Пока не стрелять",      // fire only when the merc needs to
    L"Другие опции:",          // other options available to merc
    L" Отступить",             // merc can retreat
    L" Искать укрытие",        // merc is allowed to seek cover
    L" Можно помочь команде",  // merc can assist teammates
    L"Готово",                 // done with this menu
    L"Отмена",                 // cancel this menu
};

// This string has the same comments as above, however the * denotes the option has been selected by
// the player

static wchar_t* ru_pOtherGuardMenuStrings[] = {
    L"Тип Огня:",
    L" *Агрессивный огонь*",
    L" *Беречь патроны*",
    L" *Воздерж.от стрельбы*",
    L"Другие опции:",
    L" *Отступить*",
    L" *Искать укрытие*",
    L" *Помочь команде*",
    L"Готово",
    L"Отмена",
};

static wchar_t* ru_pAssignMenuStrings[] = {
    L"На службе",  // merc is on active duty
    L"Доктор",     // the merc is acting as a doctor
    L"Пациент",    // the merc is receiving medical attention
    L"Машина",     // the merc is in a vehicle
    L"Ремонт",     // the merc is repairing items
    L"Тренинг",    // the merc is training
    L"Отмена",     // cancel this menu
};

static wchar_t* ru_pRemoveMercStrings[] = {
    L"Убрать мертвеца",  // remove dead merc from current team
    L"Отмена",
};

static wchar_t* ru_pAttributeMenuStrings[] = {
    L"Сила",     L"Ловкость", L"Подвижность", L"Здоровье", L"Меткость",
    L"Медицина", L"Механика", L"Лидерство",   L"Взрывник", L"Отмена",
};

static wchar_t* ru_pTrainingMenuStrings[] = {
    L"Практика",  // train yourself
    L"Ополч.",    // train the town
    L"Тренер",    // train your teammates
    L"Ученик",    // be trained by an instructor
    L"Отмена",    // cancel this menu
};

static wchar_t* ru_pSquadMenuStrings[] = {
    L"Отряд  1", L"Отряд  2", L"Отряд  3", L"Отряд  4", L"Отряд  5", L"Отряд  6", L"Отряд  7",
    L"Отряд  8", L"Отряд  9", L"Отряд 10", L"Отряд 11", L"Отряд 12", L"Отряд 13", L"Отряд 14",
    L"Отряд 15", L"Отряд 16", L"Отряд 17", L"Отряд 18", L"Отряд 19", L"Отряд 20", L"Отмена",
};

static wchar_t* ru_pPersonnelTitle[] = {
    L"Персонал",  // the title for the personnel screen/program application
};

static wchar_t* ru_pPersonnelScreenStrings[] = {
    L"Здоровье: ",  // health of merc
    L"Подвижность: ",    L"Ловкость: ",  L"Сила: ",      L"Лидерство: ", L"Мудрость: ",
    L"Опытность: ",  // experience level
    L"Меткость: ",       L"Механика: ",  L"Взрывник.: ", L"Медицина: ",
    L"Мед.депозит: ",     // amount of medical deposit put down on the merc
    L"Контракт: ",        // cost of current contract
    L"Убийства: ",        // number of kills by merc
    L"Помощники: ",       // number of assists on kills by merc
    L"Цена в день:",      // daily cost of merc
    L"Общая стоимость:",  // total cost of merc
    L"Контракт:",         // cost of current contract
    L"Все услуги:",       // total service rendered by merc
    L"Долги по з/п:",     // amount left on MERC merc to be paid
    L"Проц.попаданий:",   // percentage of shots that hit target
    L"Битвы:",            // number of battles fought
    L"Кол-во ран:",       // number of times merc has been wounded
    L"Навыки:",          L"Нет навыков",
};

// These string correspond to enums used in by the SkillTrait enums in SoldierProfileType.h
static wchar_t* ru_gzMercSkillText[] = {
    L"Нет навыков",    L"Раб.с отмычкой", L"Плечом к плечу",   L"Электроника",
    L"Ночные опер.",   L"Броски",         L"Обучение",         L"Тяж.оружие",
    L"Автом.оружие",   L"Скрытный",       L"Оч.проворный",     L"Вор",
    L"Военное иск-во", L"Метание ножа",   L"Стрельба с крыши", L"Маскировка",
    L"(Эксперт)",
};

// This is pop up help text for the options that are available to the merc

static wchar_t* ru_pTacticalPopupButtonStrings[] = {
    L"Стоять/Идти (|S)", L"Cогнуться/Красться (|C)",
    L"Стоять/Бежать (|R)"
    L"Лечь/Ползти (|P)",
    L"Смотреть (|L)", L"Действие", L"Разговор", L"Проверить (|C|t|r|l)",

    // Pop up door menu
    L"Открыть вручную", L"Поиск ловушек", L"Поиск ловушек", L"Разрядить ловушки", L"Силой",
    L"Минировать", L"Открыть", L"Отмычкой", L"Исп.взрывчатку", L"Ломом", L"Отмена(|E|s|c)"};

// Door Traps. When we examine a door, it could have a particular trap on it. These are the traps.

static wchar_t* ru_pDoorTrapStrings[] = {L"Ловушек нет", L"Бомба-Ловушка", L"Электроловушка",
                                         L"Ловушка-Сирена", L"Тихая сигнализация"};

// Contract Extension. These are used for the contract extension with AIM mercenaries.

static wchar_t* ru_pContractExtendStrings[] = {
    L"день",
    L"7дн.",
    L"14дн.",
};

// On the map screen, there are four columns. This text is popup help text that identifies the
// individual columns.

static wchar_t* ru_pMapScreenMouseRegionHelpText[] = {
    L"Выбрать наемника",     L"Назначить наемника", L"Направить",
    L"Наемн |Контракт (|C)", L"Убрать наемн",       L"Спать",
};

// volumes of noises

static wchar_t* ru_pNoiseVolStr[] = {L"ТИХИЙ", L"ЧЕТКИЙ", L"ГРОМКИЙ", L"ОЧ.ГРОМКИЙ"};

// types of noises

static wchar_t* ru_pNoiseTypeStr[] =  // OBSOLETE
    {L"НЕЗНАКОМЫЙ", L"звук ДВИЖЕНИЯ", L"СКРИП", L"ПЛЕСК", L"УДАР", L"ВЫСТРЕЛ",
     L"ВЗРЫВ",      L"КРИК",          L"УДАР",  L"УДАР",  L"ЗВОН", L"ГРОХОТ"};

// Directions that are used to report noises

static wchar_t* ru_pDirectionStr[] = {L"СЕВ-ВОСТОК", L"ВОСТОК", L"ЮГО-ВОСТОК", L"ЮГ",
                                      L"ЮГО-ЗАПАД",  L"ЗАПАД",  L"СЕВ-ЗАПАД",  L"СЕВЕР"};

// These are the different terrain types.

static wchar_t* ru_pLandTypeStrings[] = {
    L"Город", L"Дорога", L"Равнина", L"Пустыня", L"Леса", L"Роща", L"Болото", L"Вода", L"Холмы",
    L"Непроходимо",
    L"Река",  // river from north to south
    L"Река",  // river from east to west
    L"Чужая страна",
    // NONE of the following are used for directional travel, just for the sector description.
    L"Тропики", L"Фермы", L"Поля, дорога", L"Леса, дорога", L"Фермы, дорога", L"Тропики,дорога",
    L"Роща, дорога", L"Берег", L"Гора, дорога", L"Побережье,дорога", L"Пустыня, дорога",
    L"Болото, дорога", L"Леса,ПВО", L"Пустыня,ПВО", L"Тропики,ПВО", L"Медуна,ПВО",

    // These are descriptions for special sectors
    L"Госпит.Камбрии", L"Аэроп.Драссена", L"Аэроп.Медуны", L"ПВО",
    L"База повстанц.",  // The rebel base underground in sector A10
    L"Подзем.Тиксы",    // The basement of the Tixa Prison (J9)
    L"Логово существ",  // Any mine sector with creatures in it
    L"Подвалы Орты",    // The basement of Orta (K4)
    L"Туннель",         // The tunnel access from the maze garden in Meduna
                        // leading to the secret shelter underneath the palace
    L"Убежище",         // The shelter underneath the queen's palace
    L"",                // Unused
};

static wchar_t* ru_gpStrategicString[] = {
    L"",                                                                // Unused
    L"%s обнаружен в секторе %c%d и вот-вот прибудет еще один отряд.",  // STR_DETECTED_SINGULAR
    L"%s обнаружен в секторе %c%d и вот-вот прибудут еще отряды.",      // STR_DETECTED_PLURAL
    L"Вы хотите координировать одновременное прибытие?",                // STR_COORDINATE

    // Dialog strings for enemies.

    L"Враг дает шанс сдаться.",                                         // STR_ENEMY_SURRENDER_OFFER
    L"Враг захватил вашего наемника, который пребывает без сознания.",  // STR_ENEMY_CAPTURED

    // The text that goes on the autoresolve buttons

    L"Отступл.",  // The retreat button				//STR_AR_RETREAT_BUTTON
    L"Готово",    // The done button				//STR_AR_DONE_BUTTON

    // The headers are for the autoresolve type (MUST BE UPPERCASE)

    L"ЗАЩИТА",  // STR_AR_DEFEND_HEADER
    L"АТАКА",   // STR_AR_ATTACK_HEADER
    L"СТЫЧКА",  // STR_AR_ENCOUNTER_HEADER
    L"Сектор",  // The Sector A9 part of the header		//STR_AR_SECTOR_HEADER

    // The battle ending conditions

    L"ПОБЕДА!",    // STR_AR_OVER_VICTORY
    L"ЗАЩИТА!",    // STR_AR_OVER_DEFEAT
    L"СДАЛСЯ!",    // STR_AR_OVER_SURRENDERED
    L"ЗАХВАЧЕН!",  // STR_AR_OVER_CAPTURED
    L"ОТСТУПИЛ!",  // STR_AR_OVER_RETREATED

    // These are the labels for the different types of enemies we fight in autoresolve.

    L"Ополчен",   // STR_AR_MILITIA_NAME,
    L"Элита",     // STR_AR_ELITE_NAME,
    L"Войско",    // STR_AR_TROOP_NAME,
    L"Админ",     // STR_AR_ADMINISTRATOR_NAME,
    L"Существо",  // STR_AR_CREATURE_NAME,

    // Label for the length of time the battle took

    L"Время истекло",  // STR_AR_TIME_ELAPSED,

    // Labels for status of merc if retreating.  (UPPERCASE)

    L"ОТСТУПИЛ",     // STR_AR_MERC_RETREATED,
    L"ОТСТУПАЕТ",    // STR_AR_MERC_RETREATING,
    L"ОТСТУПЛЕНИЕ",  // STR_AR_MERC_RETREAT,

    // PRE BATTLE INTERFACE STRINGS
    // Goes on the three buttons in the prebattle interface.  The Auto resolve button represents
    // a system that automatically resolves the combat for the player without having to do anything.
    // These strings must be short (two lines -- 6-8 chars per line)

    L"Авто битва",     // STR_PB_AUTORESOLVE_BTN,
    L"Идти в Сектор",  // STR_PB_GOTOSECTOR_BTN,
    L"Отступить",      // STR_PB_RETREATMERCS_BTN,

    // The different headers(titles) for the prebattle interface.
    L"СТЫЧКА",                      // STR_PB_ENEMYENCOUNTER_HEADER,
    L"ВРАЖЕСК. ЗАХВАТ",             // STR_PB_ENEMYINVASION_HEADER, // 30
    L"ВРАЖ. ЗАСАДА",                // STR_PB_ENEMYAMBUSH_HEADER
    L"ВСТУПИТЬ ВО ВРАЖ. СЕКТОР",    // STR_PB_ENTERINGENEMYSECTOR_HEADER
    L"АТАКА СУЩЕСТВ",               // STR_PB_CREATUREATTACK_HEADER
    L"ЗАСАДА КОШКИ",                // STR_PB_BLOODCATAMBUSH_HEADER
    L"ИДТИ В ЛОГОВО КОШКИ-УБИЙЦЫ",  // STR_PB_ENTERINGBLOODCATLAIR_HEADER

    // Various single words for direct translation.  The Civilians represent the civilian
    // militia occupying the sector being attacked.  Limited to 9-10 chars

    L"Место",
    L"Враги",
    L"Наемники",
    L"Ополчение",
    L"Существа",
    L"Кошки-уб",
    L"Сектор",
    L"Никого",  // If there are no uninvolved mercs in this fight.
    L"Н/П",     // Acronym of Not Applicable
    L"д",       // One letter abbreviation of day
    L"ч",       // One letter abbreviation of hour

    // TACTICAL PLACEMENT USER INTERFACE STRINGS
    // The four buttons

    L"Очистить",
    L"Вручную",
    L"Группа",
    L"Готово",

    // The help text for the four buttons.  Use \n to denote new line (just like enter).

    L"Убрать позиции наемников \nдля повторного их ввода ( |C).",
    L"Рассредоточить наемников вручную (|S).",
    L"Выбрать место сбора наемников (|G).",
    L"Нажмите эту кнопку, когда закончите \nвыбор позиций для наемников. (|E|n|t|e|r)",
    L"Вы должны разместить всех наемн. \nперед началом битвы.",

    // Various strings (translate word for word)

    L"Сектор",
    L"Выбрать место входа",

    // Strings used for various popup message boxes.  Can be as long as desired.

    L"Выглядит непривлекательно. Место недоступно. Выберите другое место.",
    L"Поместите своих наемников в выделенное место на карте.",

    // This message is for mercs arriving in sectors.  Ex:  Red has arrived in sector A9.
    // Don't uppercase first character, or add spaces on either end.

    L"прибыл в сектор",

    // These entries are for button popup help text for the prebattle interface.  All popup help
    // text supports the use of \n to denote new line.  Do not use spaces before or after the \n.
    L"Битва разрешается автоматически\nбез загрузки карты(|A)",
    L"Нельзя исп.авторазрешение когда\nигрок атакует.",
    L"Войти в сектор:стычка с врагом (|E).",
    L"Группа отступает в прежний сектор (|R).",      // singular version
    L"Все группы отступают в прежние сектора (|R)",  // multiple groups with same previous sector
    //!!!What about repeated "R" as hotkey?
    // various popup messages for battle conditions.

    //%c%d is the sector -- ex:  A9
    L"Враги атакуют ваше ополчение в секторе %c%d.",
    //%c%d сектор -- напр:  A9
    L"Существа атакуют ваше ополч.в секторе %c%d.",
    // 1st %d refers to the number of civilians eaten by monsters,  %c%d is the sector -- ex:  A9
    // Note:  the minimum number of civilians eaten will be two.
    L"Существа атакуют и убивают %d жителей в секторе %s.",
    //%s is the sector location -- ex:  A9: Omerta
    L"Враги атакуют ваших наемн.в секторе %s. Никто из наемников не может драться!",
    //%s is the sector location -- ex:  A9: Omerta
    L"Существа атакуют ваших наемн.в секторе%s. Никто из наемников не может драться!",

};

static wchar_t* ru_gpGameClockString[] = {
    // This is the day represented in the game clock.  Must be very short, 4 characters max.
    L"День",
};

// When the merc finds a key, they can get a description of it which
// tells them where and when they found it.
static wchar_t* ru_sKeyDescriptionStrings[2] = {
    L"Сект.находки:",
    L"День находки:",
};

// The headers used to describe various weapon statistics.

static wchar_t* ru_gWeaponStatsDesc[] = {
    L"Вес (%s):", L"Статус:",
    L"Пули:",  // Number of bullets left in a magazine
    L"Дист:",  // Range
    L"Урон:",  // Damage
    L"ОД:",    // abbreviation for Action Points
    L"",          L"=",       L"=",
};

// The headers used for the merc's money.

static wchar_t* ru_gMoneyStatsDesc[] = {
    L"Кол-во",
    L"Осталось:",  // this is the overall balance
    L"Кол-во",
    L"Отделить:",  // the amount he wants to separate from the overall balance to get two piles of
                   // money

    L"Текущий",
    L"Баланс",
    L"Кол-во",
    L"Взять",
};

// The health of various creatures, enemies, characters in the game. The numbers following each are
// for comment only, but represent the precentage of points remaining.

static wchar_t* ru_zHealthStr[] = {
    L"УМИРАЕТ",   //	>= 0
    L"КРИТИЧЕН",  //	>= 15
    L"ПЛОХ",      //	>= 30
    L"РАНЕН",     //	>= 45
    L"ЗДОРОВ",    //	>= 60
    L"СИЛЕН",     // 	>= 75
    L"ОТЛИЧНО",   // 	>= 90
};

static wchar_t* ru_gzMoneyAmounts[6] = {L"1000$",  L"100$",     L"10$",
                                        L"Готово", L"Отделить", L"Взять"};

// short words meaning "Advantages" for "Pros" and "Disadvantages" for "Cons."
static wchar_t* ru_gzProsLabel = L"За:";

static wchar_t* ru_gzConsLabel = L"Прот:";

// Conversation options a player has when encountering an NPC
static wchar_t* ru_zTalkMenuStrings[6] = {
    L"Еще раз?",  // meaning "Repeat yourself"
    L"Дружески",  // approach in a friendly
    L"Прямо",     // approach directly - let's get down to business
    L"Угрожать",  // approach threateningly - talk now, or I'll blow your face off
    L"Дать",     L"Нанять"};

// Some NPCs buy, sell or repair items. These different options are available for those NPCs as
// well.
static wchar_t* ru_zDealerStrings[4] = {
    L"Куп/Прод",
    L"Куп.",
    L"Прод.",
    L"Ремонт",
};

static wchar_t* ru_zDialogActions[1] = {
    L"Готово",
};

// These are vehicles in the game.

static wchar_t* ru_pVehicleStrings[] = {
    L"Эльдорадо",
    L"Хаммер",  // a hummer jeep/truck -- military vehicle
    L"Трак с морож", L"Джип", L"Танк", L"Вертолет",
};

static wchar_t* ru_pShortVehicleStrings[] = {
    L"Эльдор",
    L"Хаммер",  // the HMVV
    L"Трак",   L"Джип", L"Танк",
    L"Верт",  // the helicopter
};

static wchar_t* ru_zVehicleName[] = {
    L"Эльдорадо",
    L"Хаммер",  // a military jeep. This is a brand name.
    L"Трак",    // Ice cream truck
    L"Джип",      L"Танк",
    L"Верт",  // an abbreviation for Helicopter
};

// These are messages Used in the Tactical Screen

static wchar_t* ru_TacticalStr[] = {
    L"Воздушный Рейд", L"Оказывать перв.помощь сразу?",

    // CAMFIELD NUKE THIS and add quote #66.

    L"%s замечает, что некоторые предметы не погрузили.",

    // The %s is a string from pDoorTrapStrings

    L"Замок (%s).", L"Тут нет замка.", L"Успех!", L"Провал.", L"Успех!", L"Провал",
    L"Замок без ловушки", L"Успех!",
    // The %s is a merc name
    L"%s:нет нужного ключа", L"Замок без ловушки", L"Замок без ловушки", L"Заперто", L"ДВЕРЬ",
    L"ЛОВУШКА", L"ЗАПЕРТО", L"НЕЗАПЕРТО", L"РАЗГРОМЛЕНО", L"Тут есть выключатель.Нажать?",
    L"Разрядить ловушку?", L"Пред...", L"След...", L"Еще...",

    // In the next 2 strings, %s is an item name

    L"%s помещен(а) на землю.", L"%s отдан(а) %s.",

    // In the next 2 strings, %s is a name

    L"%s.Оплачено сполна.", L"%s.Еще должен %d.",
    L"Выбрать частоту детонатора:",        // in this case, frequency refers to a radio signal
    L"Кол-во ходов перед взрывом:",        // how much time, in turns, until the bomb blows
    L"Устан.частоту дистанц.взрывателя:",  // in this case, frequency refers to a radio signal
    L"Разрядить ловушку?", L"Убрать голубой флаг?", L"Установить голубой флаг?", L"Завершающий ход",

    // In the next string, %s is a name. Stance refers to way they are standing.

    L"Уверен,что хочешь напасть на %s ?", L"Машина не может менять положения.",
    L"Робот не может менять положения.",

    // In the next 3 strings, %s is a name

    L"%s не может поменять положение здесь.", L"%s не может получить перв.помощь.",
    L"%s не нуждается в перв.помощи.", L"Туда идти нельзя.",
    L"Команда набрана.Мест нет.",  // there's no room for a recruit on the player's team

    // In the next string, %s is a name

    L"%s нанят.",

    // Here %s is a name and %d is a number

    L"%s должен получить $%d.",

    // In the next string, %s is a name

    L"Сопров. %s?",

    // In the next string, the first %s is a name and the second %s is an amount of money (including
    // $ sign)

    L"Нанять %s за %s в день?",

    // This line is used repeatedly to ask player if they wish to participate in a boxing match.

    L"Хотите драться?",

    // In the next string, the first %s is an item name and the
    // second %s is an amount of money (including $ sign)

    L"Купить %s за %s?",

    // In the next string, %s is a name

    L"%s сопровожден в отряд %d.",

    // These messages are displayed during play to alert the player to a particular situation

    L"ЗАКЛИНИЛО",                      // weapon is jammed.
    L"Роботу нужно пули %s калибра.",  // Robot is out of ammo
    L"Бросить туда? Нет. Не выйдет.",  // Merc can't throw to the destination he selected

    // These are different buttons that the player can turn on and off.

    L"Скрытно (|Z)", L"Окно карты (|M)", L"Готово (|D)(Завершить ход)", L"Говорить", L"Без звука",
    L"Подняться (|P|g|U|p)", L"Поз.курсора(|T|a|b)", L"Карабк./ Прыг.", L"Опуститься (|P|g|D|n)",
    L"Проверить (|C|t|r|l)", L"Пред.наемник", L"След.наемник (|S|p|a|c|e)", L"Настройки (|O)",
    L"Очередь (|B)", L"Смотреть/Повернуться (|L)", L"Здоровье: %d/%d\nЭнерг.: %d/%d\nДух: %s",
    L"Чего?",     // this means "what?"
    L"Продолж.",  // an abbrieviation for "Continued"
    L"Вкл.звук для %s.", L"Выкл.звук для %s.", L"Здоровье: %d/%d\nБенз: %d/%d", L"Выйти из машины",
    L"Поменять отряд ( |S|h|i|f|t |S|p|a|c|e )", L"Ехать",
    L"Н/П",  // this is an acronym for "Not Applicable."
    L"Исп ( Рука в руке )", L"Исп ( Огнестр.ор. )", L"Исп ( Лезвие )", L"Исп ( Взрывчатка )",
    L"Исп ( Аптечка )", L"(Поймать)", L"(Перезарядить)", L"(Дать)", L"%s отправлен.", L"%s прибыл.",
    L"%s:нет очков действия.", L"%s недоступен.", L"%s весь в бинтах.", L"%s:бинты сняты.",
    L"Враг в секторе!", L"Врага не видно.", L"Не хватает очков действия.",
    L"Никто не исп.дистанц.упр.", L"Обойма опустела!", L"СОЛДАТ", L"РЕПТИОНЫ", L"ОПОЛЧЕНИЕ",
    L"ЖИТЕЛЬ", L"Вход из сектора", L"OK", L"ОТМЕНА", L"Выбранный наемник", L"Все наемники в отряде",
    L"Идти в сектор", L"Идти на карту", L"Этот сектор отсюда покинуть нельзя.",
    L"%s слишком далеко.", L"Короткие деревья", L"Показать деревья",
    L"ВОРОНА",  // Crow, as in the large black bird
    L"ШЕЯ", L"ГОЛОВА", L"ТОРС", L"НОГИ", L"Сказать королеве то,что она хочет знать?",
    L"Отпечатки пальцев получены", L"Отпечатки неверные. Оружие не действует", L"Цель захвачена",
    L"Путь блокирован",
    L"Положить/Взять деньги со счета",  // Help text over the $ button on the Single Merc Panel
    L"Медпомощь никому не нужна.",
    L"Слом.",               // Short form of JAMMED, for small inv slots
    L"Туда не добраться.",  // used ( now ) for when we click on a cliff
    L"Путь блокирован. Хотите поменяться местами с этим человеком?",
    L"Человек отказывается двигаться.",
    // In the following message, '%s' would be replaced with a quantity of money (e.g. $200)
    L"Вы согласны заплатить %s?", L"Принять бесплатное лечение?", L"Согласны женить Дэррела?",
    L"Круглая панель управления", L"С эскортируемыми этого сделать нельзя.", L"Пощадить сержанта?",
    L"Вне досягаемости для оружия", L"Шахтер", L"Машина ходит только между сектор.",
    L"Автоперевязку сделать сейчас нельзя", L"Путь для %s блокирован",
    L"Наемники, захваченные армией Дейдранны, томятся здесь", L"Замок поражен", L"Замок разрушен",
    L"Кто-то еще пытается воспользов.этой дверью.", L"Здоровье: %d/%d\nБенз: %d/%d",
    L"%s не видит %s.",  // Cannot see person trying to talk to
};

// Varying helptext explains (for the "Go to Sector/Map" checkbox) what will happen given different
// circumstances in the "exiting sector" interface.
static wchar_t* ru_pExitingSectorHelpText[] = {
    // Helptext for the "Go to Sector" checkbox button, that explains what will happen when the box
    // is checked.
    L"После проверки соседний сектор можно сразу занять.",
    L"После проверки вы автоматически оказываетесь в окне карты а\nвашему наемнику понадобится "
    L"время на дорогу.",

    // If you attempt to leave a sector when you have multiple squads in a hostile sector.
    L"Этот сектор занят врагами и здесь оставлять наемников нельзя.\nНадо решить эту проблему "
    L"перед тем как занимать другие сектора.",

    // Because you only have one squad in the sector, and the "move all" option is checked, the "go
    // to sector" option is locked to on. The helptext explains why it is locked.
    L"Выводя оставшихся наемников из этого сектора,\nучти, что соседний сектор будет занят "
    L"немедленно.",
    L"Выведя оставшихся наемников из этого сектора,\nвы автоматически перемещаетесь в окно карты "
    L"\nвашему наемнику понадобится время на дорогу.",

    // If an EPC is the selected merc, it won't allow the merc to leave alone as the merc is being
    // escorted.  The "single" button is disabled.
    L"%s не может покинуть этот сектор один, его надо сопроводить.",

    // If only one conscious merc is left and is selected, and there are EPCs in the squad, the merc
    // will be prohibited from leaving alone. There are several strings depending on the gender of
    // the merc and how many EPCs are in the squad. DO NOT USE THE NEWLINE HERE AS IT IS USED FOR
    // BOTH HELPTEXT AND SCREEN MESSAGES!
    L"%s не может покинуть сектор один-он сопровождает %s.",       // male singular
    L"%s не может покинуть сектор одна-она сопровождает %s.",      // female singular
    L"%s не может покинуть сектор один-он  сопровождает группу.",  // male plural
    L"%s не может покинуть сектор одна-она сопровождает группу.",  // female plural

    // If one or more of your mercs in the selected squad aren't in range of the traversal area,
    // then the  "move all" option is disabled, and this helptext explains why.
    L"Чтобы дать отряд мог пойти,\nвсе ваши наемники дожны быть в рядом.",

    L"",  // UNUSED

    // Standard helptext for single movement.  Explains what will happen (splitting the squad)
    L"После проверки %s поедет один, и\nавтоматически попадет в уникальный отряд.",

    // Standard helptext for all movement.  Explains what will happen (moving the squad)
    L"После проверки выбранный вами сейчас \nотряд покинет этот сектор.",

    // This strings is used BEFORE the "exiting sector" interface is created.  If you have an EPC
    // selected and you attempt to tactically traverse the EPC while the escorting mercs aren't near
    // enough (or dead, dying, or unconscious), this message will appear and the "exiting sector"
    // interface will not appear.  This is just like the situation where This string is special, as
    // it is not used as helptext.  Do not use the special newline character (\n) for this string.
    L"%s не может покинуть этот сектор один, его надо сопроводить. Остальные наемники остаются "
    L"пока с вами.",
};

static wchar_t* ru_pRepairStrings[] = {
    L"Вещи",    // tell merc to repair items in inventory
    L"ПВО",     // tell merc to repair SAM site - SAM is an acronym for Surface to Air Missile
    L"Отмена",  // cancel this menu
    L"Робот",   // repair the robot
};

// NOTE: combine prestatbuildstring with statgain to get a line like the example below.
// "John has gained 3 points of marksmanship skill."

static wchar_t* ru_sPreStatBuildString[] = {
    L"потерял",   // the merc has lost a statistic
    L"приобрел",  // the merc has gained a statistic
    L"очко",      // singular
    L"очки",      // plural
    L"уровень",   // singular
    L"уровня",    // plural
};

static wchar_t* ru_sStatGainStrings[] = {
    L"здор.",     L"подвижн.",  L"проворн.",   L"мудрость.", L"медицина",   L"взрывн.работы.",
    L"механика.", L"меткость.", L"опытность.", L"сила.",     L"лидерство.",
};

static wchar_t* ru_pHelicopterEtaStrings[] = {
    L"Общее расст.:  ",  // total distance for helicopter to travel
    L" Безопасно:  ",    // distance to travel to destination
    L" Опасно:",         // distance to return from destination to airport
    L"Общ.цена: ",       // total cost of trip by helicopter
    L"УВП:  ",           // ETA is an acronym for "estimated time of arrival"
    L"В вертолете мало топлива, он вынужд.сесть на враж.территории!",  // warning that the sector
                                                                       // the helicopter is going to
                                                                       // use for refueling is under
                                                                       // enemy control ->
    L"Пассажиры: ",
    L"Выбрать высадку Всадника или Прибывающих?",
    L"Всадник",
    L"Прибывающие",
};

static wchar_t* ru_sMapLevelString[] = {
    L"Подуровень ",  // what level below the ground is the player viewing in mapscreen
};

static wchar_t* ru_gsLoyalString[] = {
    L"Отнош",  // the loyalty rating of a town ie : Loyal 53%
};

// error message for when player is trying to give a merc a travel order while he's underground.

static wchar_t* ru_gsUndergroundString[] = {
    L"не приним.приказов идти под землей.",
};

static wchar_t* ru_gsTimeStrings[] = {
    L"ч",  // hours abbreviation
    L"м",  // minutes abbreviation
    L"с",  // seconds abbreviation
    L"д",  // days abbreviation
};

// text for the various facilities in the sector

static wchar_t* ru_sFacilitiesStrings[] = {
    L"Ничего",     L"Госпит.", L"Заводы", L"Тюрьма", L"Ополчен.", L"Аэропорт",
    L"Стрельбище",  // a field for soldiers to practise their shooting skills
};

// text for inventory pop up button

static wchar_t* ru_pMapPopUpInventoryText[] = {
    L"Инвентарь",
    L"Выход",
};

// town strings

static wchar_t* ru_pwTownInfoStrings[] = {
    L"Размер",        // 0 // size of the town in sectors
    L"",              // blank line, required
    L"Контроль",      // how much of town is controlled
    L"Ничего",        // none of this town
    L"Шахта города",  // mine associated with this town
    L"Верность",      // 5 // the loyalty level of this town
    L"Готовы",        // the forces in the town trained by the player
    L"",
    L"Осн.оборуд.",         // main facilities in this town
    L"Уровень",             // the training level of civilians in this town
    L"Подготовка жителей",  // 10 // state of civilian training in town
    L"Ополчение",           // the state of the trained civilians in the town
};

// Mine strings

static wchar_t* ru_pwMineStrings[] = {
    L"Шахта",  // 0
    L"Серебро",
    L"Золото",
    L"Производит/день",
    L"Производств.возм-ти",
    L"Брошено",  // 5
    L"Закрыто",
    L"Выработана",
    L"Работает",
    L"Статус",
    L"Производительность",
    L"Тип руды",  // 10
    L"Город контроля",
    L"Отношение города",
    //	L"Работ.шахтеры",
};

// blank sector strings

static wchar_t* ru_pwMiscSectorStrings[] = {
    L"Силы врага", L"Сектор", L"# вещей", L"Неизв.", L"Под контр.", L"Да", L"Нет",
};

// error strings for inventory

static wchar_t* ru_pMapInventoryErrorString[] = {
    L"%s недостаточно близко",  // Merc is in sector with item but not close enough
    L"Нельзя выбрать этого.",   // MARK CARTER
    L"%s не в секторе и не может взять эту вещь",
    L"Во время битвы надо подбирать эти вещи вручную",
    L"Во время битвы надо бросать вещи вручную.",
    L"%s не в секторе,чтобы бросить вещи.",
};

static wchar_t* ru_pMapInventoryStrings[] = {
    L"Место",        // sector these items are in
    L"Всего вещей",  // total number of items in sector
};

// help text for the user

static wchar_t* ru_pMapScreenFastHelpTextList[] = {
    L"Чтобы дать наемнику такие задания как идти в др.отряд, лечение или ремонт,выберите нужное в "
    L"'Принадлежность'",
    L"чтобы направить наемника в другой сектор, выберите нужное в колонке 'Куда'",
    L"Когда наемники получают приказ начать движение,компрессия позволит им это сделать.",
    L"Левый щелчок-выбрать сектор. Еще раз левый щелчок-дать наемнику команду начать "
    L"движение,правый щелчок-общая информация о секторе.",
    L"Нажать'h' в любое время, чтобы вызвать подсказку.",
    L"Проверка",
    L"Проверка",
    L"Проверка",
    L"Проверка",
    L"Пока команда не добралась до Арулько,в этом окне вам делать нечего.Когда вы укомплектуете "
    L"свою команду,нажмите на кнопку Сжатие Времени в правом нижнем углу экрана.Команда доберется "
    L"гораздо быстрее.",
};

// movement menu text

static wchar_t* ru_pMovementMenuStrings[] = {
    L"Преместить наемн.в сектор",  // title for movement box
    L"Путь",                       // done with movement menu, start plotting movement
    L"Отмена",                     // cancel this menu
    L"Другое",                     // title for group of mercs not on squads nor in vehicles
};

static wchar_t* ru_pUpdateMercStrings[] = {
    L"Ой!:",                       // an error has occured
    L"Контракты закончились:",     // this pop up came up due to a merc contract ending
    L"Наемник выполнил задание:",  // this pop up....due to more than one merc finishing assignments
    L"Наемн.снова работает:",  // this pop up ....due to more than one merc waking up and returing
                               // to work
    L"Наемники идут спать:",   // this pop up ....due to more than one merc being tired and going to
                               // sleep
    L"Контракты скоро кончатся:",  // this pop up came up due to a merc contract ending
};

// map screen map border buttons help text

static wchar_t* ru_pMapScreenBorderButtonHelpText[] = {
    L"Показать Города (|W)",       L"Показать Шахты (|M)",
    L"Показ.Команды и Врагов(|T)", L"Показать воздушное пространство(|A)",
    L"Показать Вещи (|I)",         L"Показ.ополчен.и врагов(|Z)",
};

static wchar_t* ru_pMapScreenBottomFastHelp[] = {
    L"Лэптоп (|L)",
    L"Тактика(|E|s|c)",
    L"Настройки (|O)",
    L"Сжатие врем.(|+)",                                 // time compress more
    L"Сжатие врем.(|-)",                                 // time compress less
    L"Предыдущ.сообщ (|U|p)\nПредыдущ.стр. (|P|g|U|p)",  // previous message in scrollable list
    L"След.сообщ. (|D|o|w|n)\nСлнд.стр. (|P|g|D|n)",     // next message in the scrollable list
    L"Пустить/Остановить время (|S|p|a|c|e)",            // start/stop time compression
};

static wchar_t* ru_pMapScreenBottomText[] = {
    L"Текущий баланс",  // current balance in player bank account
};

static wchar_t* ru_pMercDeadString[] = {
    L"%s мертв.",
};

static wchar_t* ru_pDayStrings[] = {
    L"День",
};

// the list of email sender names

static wchar_t* ru_pSenderNameList[] = {
    L"Энрико",
    L"Псих Про Инк",
    L"Помощь",
    L"Псих.Про Инк",
    L"Спек",
    L"R.I.S.",  // 5
    L"Барри",
    L"Блад",
    L"Рысь",
    L"Гризли",
    L"Вики",  // 10
    L"Тревор",
    L"Хряп",
    L"Иван",
    L"Анаболик",
    L"Игорь",  // 15
    L"Тень",
    L"Рыжий",
    L"Потрошитель",
    L"Фидель",
    L"Лиска",  // 20
    L"Сидней",
    L"Гас",
    L"Сдоба",
    L"Айс",
    L"Паук",  // 25
    L"Скала",
    L"Бык",
    L"Стрелок",
    L"Тоска",
    L"Рейдер",  // 30
    L"Сова",
    L"Статик",
    L"Лен",
    L"Данни",
    L"Маг",
    L"Стэфен",
    L"Лысый",
    L"Злобный",
    L"Доктор Кью",
    L"Гвоздь",
    L"Тор",
    L"Стрелка",
    L"Волк",
    L"ЭмДи",
    L"Лава",
    //----------
    L"M.I.S.Страх.",
    L"Бобби Рэй",
    L"Босс",
    L"Джон Калба",
    L"А.I.М.",
};

// next/prev strings

static wchar_t* ru_pTraverseStrings[] = {
    L"Предыд",
    L"След",
};

// new mail notify string

static wchar_t* ru_pNewMailStrings[] = {
    L"Есть новые сообщения...",
};

// confirm player's intent to delete messages

static wchar_t* ru_pDeleteMailStrings[] = {
    L"Стереть сообщение?",
    L"Стереть НЕПРОЧТЕННЫЕ?",
};

// the sort header strings

static wchar_t* ru_pEmailHeaders[] = {
    L"От:",
    L"Тема:",
    L"Дата:",
};

// email titlebar text

static wchar_t* ru_pEmailTitleText[] = {
    L"Почтовый ящик",
};

// the financial screen strings
static wchar_t* ru_pFinanceTitle[] = {
    L"Бухгалтер Плюс",  // the name we made up for the financial program in the game
};

static wchar_t* ru_pFinanceSummary[] = {
    L"Кредит:",  // credit (subtract from) to player's account
    L"Дебет:",   // debit (add to) to player's account
    L"Приход за вчерашний день:",
    L"Депозиты за вчерашн.день:",
    L"Дебет за вчерашн. день:",
    L"Баланс на конец дня:",
    L"Приход за сегодня:",
    L"Депозиты за сегодня:",
    L"Дебет на сегодня:",
    L"Текущий баланс:",
    L"Предполагаемый приход:",
    L"Предполагаемый баланс:",  // projected balance for player for tommorow
};

// headers to each list in financial screen

static wchar_t* ru_pFinanceHeaders[] = {
    L"Day",      // the day column
    L"Кредит",   // the credits column
    L"Дебет",    // the debits column
    L"Перевод",  // transaction type - see TransactionText below
    L"Баланс",   // balance at this point in time
    L"Стр.",     // page number
    L"Дн.",      // the day(s) of transactions this page displays
};

static wchar_t* ru_pTransactionText[] = {
    L"Интерес",  // interest the player has accumulated so far
    L"Анонимные вклады",
    L"Пеня за перевод",
    L"Нанят",               // Merc was hired
    L"Торговля Бобби Рэя",  // Bobby Ray is the name of an arms dealer
    L"Зарегистр.счета в M.E.R.C.",
    L"Мед Депозит: %s",  // medical deposit for merc
    L"IMP анализ",       // IMP is the acronym for International Mercenary Profiling
    L"Куплена страховка:%s",
    L"Понижена страховка: %s",
    L"Расширена страховка: %s",  // johnny contract extended
    L"Отменена страховка: %s",
    L"Страховой запрос: %s",  // insurance claim for merc
    L"в день",                // merc's contract extended for a day
    L"7 дней",                // merc's contract extended for a week
    L"14 дней",               // ... for 2 weeks
    L"Доход с шахт",
    L"",  // String nuked
    L"Торговля цветами",
    L"Полная оплата медуслуг.: %s",
    L"Частичн.оплата медуслуг: %s",
    L"Медуслуги не оплачены: %s",
    L"Выплаты: %s",                 // %s is the name of the npc being paid
    L"Перевод средств на имя %s",   // transfer funds to a merc
    L"Перевод средств от %s",       // transfer funds from a merc
    L"Стоим.экипировки ополч: %s",  // initial cost to equip a town's militia
    L"Покупки у %s.",  // is used for the Shop keeper interface.  The dealers name will be appended
                       // to the end of the string.
    L"%s положил деньги.",
};

static wchar_t* ru_pTransactionAlternateText[] = {
    L"Страховка",                       // insurance for a merc
    L"%s:продлить контракт на 1 день",  // entend mercs contract by a day
    L"%s:продлить контракт на 7 дней",
    L"%s:продлить контракт на 14 дней",
};

// helicopter pilot payment

static wchar_t* ru_pSkyriderText[] = {
    L"Всаднику заплачено $%d",       // skyrider was paid an amount of money
    L"Всаднику недоплачено $%d",     // skyrider is still owed an amount of money
    L"Всадник. Заправка завершена",  // skyrider has finished refueling
    L"",                             // unused
    L"",                             // unused
    L"Всадник готов к полету.",      // Skyrider was grounded but has been freed
    L"У Всадника нет пассажиров.Если вы хотите отправить наемников в этот "
    L"сектор, выберите ПРИНАДЛ. и МАШИНА"};

// strings for different levels of merc morale

static wchar_t* ru_pMoralStrings[] = {
    L"Отлично", L"Хорошо", L"Норм.", L"НеОчень", L"Паника", L"Плох",
};

// Mercs equipment has now arrived and is now available in Omerta or Drassen.

static wchar_t* ru_pLeftEquipmentString[] = {
    L"%s:экипировку можно получить в Омерте( A9 ).",
    L"%s:экипировку можно получить в Драссене( B13 ).",
};

// Status that appears on the Map Screen

static wchar_t* ru_pMapScreenStatusStrings[] = {
    L"Здоровье", L"Энергия", L"Дух",
    L"Сост.",   // the condition of the current vehicle (its "health")
    L"Бензин",  // the fuel level of the current vehicle (its "energy")
};

static wchar_t* ru_pMapScreenPrevNextCharButtonHelpText[] = {
    L"Пред.наемник (|L|e|f|t)",    // previous merc in the list
    L"След.наемник (|R|i|g|h|t)",  // next merc in the list
};

static wchar_t* ru_pEtaString[] = {
    L"УВП:",  // eta is an acronym for Estimated Time of Arrival
};

static wchar_t* ru_pTrashItemText[] = {
    L"Вы потеряете это навсегда.Выполнить?",  // do you want to continue and lose the item forever
    L"Это,кажется,и вправду ВАЖНАЯ вещь.Вы ВПОЛНЕ уверены, что хотите выбросить ее?",  // does the
                                                                                       // user
                                                                                       // REALLY
                                                                                       // want to
                                                                                       // trash this
                                                                                       // item
};

static wchar_t* ru_pMapErrorString[] = {
    L"Отряд не может двигаться со спящим наемн.",

    // 1-5
    L"Сперва выведите отряд на землю.",
    L"Приказ двигаться? Тут же кругом враги!",
    L"Наемн.должен быть назначен в сектор или машину,чтобы ехать.",
    L"У вас в команде еще никого нет",  // you have no members, can't do anything
    L"Наемн.не может выполнить.",       // merc can't comply with your order
                                        // 6-10
    L"чтобы двигаться,нужен эскорт.Обеспечьте его эскортом",  // merc can't move unescorted .. for a
                                                              // male
    L"чтобы двигаться, нужен эскорт.Обеспечьте ее эскортом.",  // for a female
    L"Наемник еще не прибыл в Арулько!",
    L"Кажется,сначала нужно уладить все проблемы с контрактом.",
    L"",
    // 11-15
    L"Приказ двигаться? Тут же битва идет!",
    L"Вы наткнулись на засаду Кошки-Убийцы в секторе %s!",
    L"Вы попали в логово Кошек-Убийц в секторе I16!",
    L"",
    L"ПВО в %s занята врагом.",
    // 16-20
    L"Шахта в %s взята. Ваш ежедневный доход упал до %s в день.",
    L"Противник взял сектор %s, не встретив сопротивления.",
    L"Как минимум одного из ваших наемн.нельзя назн.на это задание.",
    L"%s нельзя присоед.к %s. Уже полон",
    L"%s нельзя присоед.к %s. Слишком далеко.",
    // 21-25
    L"Шахта в %s захвачена войсками Дейдранны!",
    L"Войска Дейдранны только что захватили ПВО в %s",
    L"Войска Дейдранны только что захватили %s",
    L"Войска Дейдранны только что были замечены в %s.",
    L"Войска Дейдраннытолько что захватили %s.",
    // 26-30
    L"Как минимум один из ваших наемников невозможно уложить спать.",
    L"Как минимум одного из ваших наемников невозможно разбудить.",
    L"Ополчение не придет, пока не закончится его обучение.",
    L"%s сейчас не может принять приказ двигаться.",
    L"Ополчение, которое находится вне города,нельзя переместить в другой сектор.",
    // 31-35
    L"Нельзя держать ополчение в %s.",
    L"Пустая машина не может двигаться!",
    L"%s слишком изранен, чтобы идти!",
    L"Сперва надо покинуть музей!",
    L"%s мертв!",
    // 36-40
    L"%s не может перейти к %s: он в движении",
    L"%s не может сесть в машину так",
    L"%s не может присоед. к %s",
    L"Нельзя сжимать время пока нет наемников!",
    L"Эта машина может ездить только по дорогам!",
    // 41-45
    L"Нельзя переназначать движущихся наемников",
    L"В машине кончился бензин!",
    L"%s слишком устал,чтобы передвигаться.",
    L"Никто из сидящих в машине не может управлять ею.",
    L"Сейчас один/неск.наемн.этого отряда не могут двигаться.",
    // 46-50
    L"Сейчас один/неск.ДРУГИХ наемн.не могут двигаться.",
    L"Машина слишком побита!",
    L"Тренировать ополчение могут только 2 наемн.в секторе",
    L"Робот не может двигаться без управляющего.Поместите их в один отряд.",
};

// help text used during strategic route plotting
static wchar_t* ru_pMapPlotStrings[] = {
    L"Щелкните по месту,чтобы подтвердить конечное направление,или щелкните по другому сектору.",
    L"Направление подтверждено.",
    L"Место назн.не изменилось.",
    L"Направление отменено.",
    L"Путь укорочен.",
};

// help text used when moving the merc arrival sector
static wchar_t* ru_pBullseyeStrings[] = {
    L"Кликнуть на тот сектор, куда вы хотите отправить наемника.",
    L"OK.Прибывающий наемник будет высажен в %s",
    L"Наемнику нельзя туда лететь,воздушн.путь небезопасен!",
    L"Отмена. Сектор прибытия тот же",
    L"Возд.пространство над %s небезопасно!Сектор прибытия перемещен в %s.",
};

// help text for mouse regions

static wchar_t* ru_pMiscMapScreenMouseRegionHelpText[] = {
    L"Просмотр инвентаря(|E|n|t|e|r)",
    L"Выкинуть вещь",
    L"Выйти из инвентаря(|E|n|t|e|r)",
};

// male version of where equipment is left
static wchar_t* ru_pMercHeLeaveString[] = {
    L"%s должен оставить свое снаряжение здесь (%s) или позже в Драссене (B13)во время вылета из "
    L"Арулько?",
    L"%s должен оставить свое снаряжение здесь (%s) или позже в Омерте (А9) во время вылета из "
    L"Арулько?",
    L"отправляется и сбросит свое снаряжение в Омерте (A9).",
    L"отправляется и сбросит свое снаряжение в Драссене (B13).",
    L"%s отправляется и сбросит свое снаряжение в %s.",
};

// female version
static wchar_t* ru_pMercSheLeaveString[] = {
    L"%s должна оставить свое снаряжение здесь (%s) или позже в Драссене (B13)во время вылета из "
    L"Арулько?",
    L" должна оставить свое снаряжение здесь (%s) или позже в Омерте (А9)во время вылета из "
    L"Арулько?",
    L"отправляется и сбросит свое снаряжение в Омерте (A9).",
    L"отправляется и сбросит свое снаряжение в Драссене (B13).",
    L"%s отправляется и сбросит свое снаряжение в %s.",
};

static wchar_t* ru_pMercContractOverStrings[] = {
    L":его контракт закончился,он уехал домой.",    // merc's contract is over and has departed
    L":ее контракт закончился,она уехала домой.",   // merc's contract is over and has departed
    L":контракт кончился,он уехал.",                // merc's contract has been terminated
    L":контракт кончился она уехала.",              // merc's contract has been terminated
    L"Вы должны M.E.R.C. слишком много,%s уехал.",  // Your M.E.R.C. account is invalid so merc left
};

// Text used on IMP Web Pages

static wchar_t* ru_pImpPopUpStrings[] = {
    L"Неверный код авторизации",
    L"Вы уверены, что хотите начать процесс записи профайла заново?",
    L"Введите полное имя и пол",
    L"Предварит.анализ ваших финансов показал, что у вас недостаточно денег на анализ.",
    L"Сейчас вы не можете выбрать это.",
    L"Чтобы закончить анализ,нужно иметь место еще хотя бы для одного члена команды.",
    L"Анализ уже завершен.",
};

// button labels used on the IMP site

static wchar_t* ru_pImpButtonText[] = {
    L"Подробнее",  // about the IMP site
    L"НАЧАТЬ",     // begin profiling
    L"Личность",   // personality section
    L"Свойства",   // personal stats/attributes section
    L"Портрет",    // the personal portrait selection
    L"Голос %d",   // the voice selection
    L"Готово",     // done profiling
    L"Заново",     // start over profiling
    L"Да,выбрать выделенный ответ.",
    L"Да",
    L"Нет",
    L"Закончить",                  // finished answering questions
    L"Пред",                       // previous question..abbreviated form
    L"След",                       // next question
    L"ДА.",                        // yes, I am certain
    L"НЕТ, ХОЧУ НАЧАТЬ СНАЧАЛА.",  // no, I want to start over the profiling process
    L"ДА.",
    L"НЕТ",
    L"Назад",     // back one page
    L"Отменить",  // cancel selection
    L"Да,уверен.",
    L"Нет,просмотреть еще раз.",
    L"Зарегистр.",  // the IMP site registry..when name and gender is selected
    L"Анализ",      // analyzing your profile results
    L"OK",
    L"Голос",
};

static wchar_t* ru_pExtraIMPStrings[] = {
    L"Чтобы начать профилирование, выберите Личность.",
    L"Когда Личность завершена, выберите ваши Свойства.",
    L"Свойства приписаны,переходите к Портрету.",
    L"Чтобы завершить процесс,выберите голос,который вам подходит."};

static wchar_t* ru_pFilesTitle[] = {
    L"Просмотр файлов",
};

static wchar_t* ru_pFilesSenderList[] = {
    L"Отчет разведки",  // the recon report sent to the player. Recon is an abbreviation for
                        // reconissance
    L"Перехват #1",  // first intercept file .. Intercept is the title of the organization sending
                     // the file...similar in function to INTERPOL/CIA/KGB..refer to fist record in
                     // files.txt for the translated title
    L"Перехват #2",  // second intercept file
    L"Перехват #3",  // third intercept file
    L"Перехват #4",  // fourth intercept file
    L"Перехват #5",  // fifth intercept file
    L"Перехват #6",  // sixth intercept file
};

// Text having to do with the History Log

static wchar_t* ru_pHistoryTitle[] = {
    L"История",
};

static wchar_t* ru_pHistoryHeaders[] = {
    L"День",     // the day the history event occurred
    L"Стр.",     // the current page in the history report we are in
    L"День",     // the days the history report occurs over
    L"Место",    // location (in sector) the event occurred
    L"Событие",  // the event label
};

// various history events
// THESE STRINGS ARE "HISTORY LOG" STRINGS AND THEIR LENGTH IS VERY LIMITED.
// PLEASE BE MINDFUL OF THE LENGTH OF THESE STRINGS. ONE WAY TO "TEST" THIS
// IS TO TURN "CHEAT MODE" ON AND USE CONTROL-R IN THE TACTICAL SCREEN, THEN
// GO INTO THE LAPTOP/HISTORY LOG AND CHECK OUT THE STRINGS. CONTROL-R INSERTS
// MANY (NOT ALL) OF THE STRINGS IN THE FOLLOWING LIST INTO THE GAME.
static wchar_t* ru_pHistoryStrings[] = {
    L"",  // leave this line blank
    // 1-5
    L"%s нанят из A.I.M.",          // merc was hired from the aim site
    L"%s нанят из M.E.R.C.",        // merc was hired from the aim site
    L"%s умер.",                    // merc was killed
    L"Зарегистр.счета в M.E.R.C.",  // paid outstanding bills at MERC
    L"Принято назначение от Энрико Сальвадори",
    // 6-10
    L"IMP профайл сгенерирован",
    L"Подписан страховой контракт для %s.",  // insurance contract purchased
    L"Отменен страховой контракт для %s.",   // insurance contract canceled
    L"Страховая выплата для %s.",            // insurance claim payout for merc
    L"%s:контракт продлен на день.",         // Extented "mercs name"'s for a day
    // 11-15
    L"%s:контракт продлен на 7дн.",   // Extented "mercs name"'s for a week
    L"%s:контракт продлен на 14дн.",  // Extented "mercs name"'s 2 weeks
    L"%s уволен.",                    // "merc's name" was dismissed.
    L"%s ушел.",                      // "merc's name" quit.
    L"начало.",                       // a particular quest started
    // 16-20
    L"завершен.",
    L"Разговор с начальн.шахт в 						 s",  // talked to
                                                                                      // head miner
                                                                                      // of town
    L"Освобожден %s",
    L"Был использован обман",
    L"Пища должна быть в Омерте до завтра",
    // 21-25
    L"%s покинула команду и вышла замуж за Дэрила Хика",
    L"%s:срок контракта истек.",
    L"%s нанят.",
    L"Энрико жалуется на отсуствие прогресса",
    L"Битва выиграна",
    // 26-30
    L"%s:в шахте кончается руда",
    L"%s: шахта выработана",
    L"%s: шахта закрыта",
    L"%s: шахта вновь открыта",
    L"Получил сведения о тюрьме Тикса.",
    // 31-35
    L"Услышал о секретном военном заводе Орта.",
    L"Ученый с Орты помог с ракетным ружьем.",
    L"Дейдранна нашла применение трупам.",
    L"Франк говорил о боях в Сан Моне.",
    L"Пациент думает,что он что-то видел в шахтах.",
    // 36-40
    L"Встретил какого-то Девина - торгует взрывчаткой.",
    L"Столкнулся со знаменитым Майком!",
    L"Встретил Тони - он занимается оружием.",
    L"Получил ракетное ружье от сержанта Кротта.",
    L"Право собственности на магазин Энжела передано Кайлу.",
    // 41-45
    L"Шиз предлагает сделать робота.",
    L"Болтун может сделать тайное варево для жуков.",
    L"Кейт больше не работает.",
    L"Говард обеспечивает Дейдранну цианидом.",
    L"Встретил Кейта - своего человека в Камбрии.",
    // 46-50
    L"Встретил Говарда - фармацевта из Балимы.",
    L"Встретил Перко - у него маленький ремонтный бизнес.",
    L"Встретил Сэма из Балайма - у него компьютерный магазин.",
    L"Фрэнс занимается электроникой и другими вещами.",
    L"У Арнольда ремонтный магазин в Граме.",
    // 51-55
    L"Фредо ремонтирует электронику в Граме.",
    L"Получено пожертвование от богатого парня из Балайма.",
    L"Встретил старьевщика по имени Джейк.",
    L"Нам дали электронный ключ.",
    L"Подкупил Вальтера, чтобы он открыл дверь в подвал.",
    // 56-60
    L"Если у Дэвида есть бензин,он нам его даст бесплатно.",
    L"Дал взятку Пабло.",
    L"Босс хранит деньги в шахте Сан Моны.",
    L"%s выиграл кулачный бой",
    L"%s проиграл кулачный бой",
    // 61-65
    L"%s дисквалифицирован в кулачном бою",
    L"Нашел много денег в заброшенной шахте.",
    L"Захватил убийцу, подосланного Боссом.",
    L"Потерял контроль над сектором",  // ENEMY_INVASION_CODE
    L"Защитил сектор",
    // 66-70
    L"Проиграл битву",  // ENEMY_ENCOUNTER_CODE
    L"Засада",          // ENEMY_AMBUSH_CODE
    L"Засада перебита",
    L"Безуспешная атака",  // ENTERING_ENEMY_SECTOR_CODE
    L"Успешная атака!",
    // 71-75
    L"Существа атаковали",   // CREATURE_ATTACK_CODE
    L"Убит кошкой-убийцей",  // BLOODCAT_AMBUSH_CODE
    L"Перебил кошек-убийц",
    L"%s убит",
    L"Отдал голову террориста Слаю",
    L"Слай ушел",
    L"Убил %s",
};

static wchar_t* ru_pHistoryLocations[] = {
    L"Н/П",  // N/A is an acronym for Not Applicable
};

// icon text strings that appear on the laptop

static wchar_t* ru_pLaptopIcons[] = {
    L"Почта",       L"Сеть", L"Финансы", L"Кадры", L"Журнал", L"Файлы", L"Выключить",
    L"сир-ФЕР 4.0",  // our play on the company name (Sirtech) and web surFER
};

// bookmarks for different websites
// IMPORTANT make sure you move down the Cancel string as bookmarks are being added

static wchar_t* ru_pBookMarkStrings[] = {
    L"А.I.M.", L"Бобби Рэй", L"I.M.P.", L"М.Е.R.С.", L"Морг", L"Цветы", L"Страховка", L"Отмена",
};

static wchar_t* ru_pBookmarkTitle[] = {
    L"Закладки",
    L"Чтобы попасть в это меню потом - правый щелчок.",
};

// When loading or download a web page

static wchar_t* ru_pDownloadString[] = {
    L"Загрузка",
    L"Перезагрузка",
};

// This is the text used on the bank machines, here called ATMs for Automatic Teller Machine

static wchar_t* ru_gsAtmSideButtonText[] = {
    L"OK",
    L"Взять",   // take money from merc
    L"Дать",    // give money to merc
    L"Отмена",  // cancel transaction
    L"Очист.",  // clear amount being displayed on the screen
};

static wchar_t* ru_gsAtmStartButtonText[] = {
    L"Перевести $",  // transfer money to merc -- short form
    L"Стат.",        // view stats of the merc
    L"Инвентарь",    // view the inventory of the merc
    L"Занятость",
};

static wchar_t* ru_sATMText[] = {
    L"Перевести деньги?",              // transfer funds to merc?
    L"Ok?",                            // are we certain?
    L"Введите сумму",                  // enter the amount you want to transfer to merc
    L"Выберите тип",                   // select the type of transfer to merc
    L"Недостаточно денег",             // not enough money to transfer to merc
    L"Сумма должна быть кратной $10",  // transfer amount must be a multiple of $10
};

// Web error messages. Please use German equivilant for these messages.
// DNS is the acronym for Domain Name Server
// URL is the acronym for Uniform Resource Locator

static wchar_t* ru_pErrorStrings[] = {
    L"Ошибка",
    L"Сервер не имеет DNS-входа.",
    L"Проверьте URL адрес и попробуйте еще раз.",
    L"OK",
    L"Плохое соединение.Попробуйте позднее.",
};

static wchar_t* ru_pPersonnelString[] = {
    L"Наемн:",  // mercs we have
};

static wchar_t* ru_pWebTitle[] = {
    L"сир-ФЕР 4.0",  // our name for the version of the browser, play on company name
};

// The titles for the web program title bar, for each page loaded

static wchar_t* ru_pWebPagesTitles[] = {
    L"А.I.M.",
    L"Члены A.I.M.",
    L"Фото A.I.M.",  // a mug shot is another name for a portrait
    L"A.I.M. Сортировка",
    L"A.I.M.",
    L"A.I.M.-История",  //$$
    L"A.I.M.-Политика",
    L"A.I.M.-Журнал",
    L"A.I.M.-Ссылки",
    L"M.E.R.C.",
    L"M.E.R.C.-Счета",
    L"M.E.R.C.-Регистрация",
    L"M.E.R.C.-Индекс",
    L"Бобби Рэй",
    L"Бобби Рэй - Пист.",
    L"Бобби Рэй - Оруж.",
    L"Бобби Рэй - Броня",
    L"Бобби Рэй - разное",  // misc is an abbreviation for miscellaneous
    L"Бобби Рэй - Б.У.",
    L"Бобби Рэй - Бланк",
    L"I.M.P.",
    L"I.M.P.",
    L"Объед.Служба Цветов",
    L"Объед.Служба Цветов - Галерея",
    L"Объед.Служба Цветов - Бланк Заказа",
    L"Объед.Служба Цветов - Открытки",
    L"Малеус,Инкус и Стэйпс:страховые агенты",
    L"Информация",
    L"Контракт",
    L"Комментарии",
    L"Морг Макгилликути",
    L"",
    L"URL не найден.",
    L"Бобби Рэй - Последние поступл.",  //@@@3 Translate new text
    L"",
    L"",
};

static wchar_t* ru_pShowBookmarkString[] = {
    L"Sir-Help",
    L"Закладки:щелкните еще раз по Web.",
};

static wchar_t* ru_pLaptopTitles[] = {
    L"Почтовый ящик", L"Просмотр файлов", L"Персонал", L"Бухгалтер Плюс", L"Журнал",
};

static wchar_t* ru_pPersonnelDepartedStateStrings[] = {
    // reasons why a merc has left.
    L"Убит в бою", L"Уволен", L"Другое", L"Женат", L"Контракт окончен", L"Выход",
};
// personnel strings appearing in the Personnel Manager on the laptop

static wchar_t* ru_pPersonelTeamStrings[] = {
    L"Текущий отряд", L"Отправления", L"Расходы/день:", L"Наиб.расход:",
    L"Наим.расход:",  L"Убит в бою:", L"Уволен:",       L"Другое:",
};

static wchar_t* ru_pPersonnelCurrentTeamStatsStrings[] = {
    L"Низкий",
    L"Средний",
    L"Высокий",
};

static wchar_t* ru_pPersonnelTeamStatsStrings[] = {
    L"ЗДОР", L"ПДВ", L"ЛОВ", L"СИЛ", L"ЛДР", L"МДР", L"УРВ", L"МТК", L"МЕХ", L"ВЗРВ", L"МЕД",
};

// horizontal and vertical indices on the map screen

static wchar_t* ru_pMapVertIndex[] = {
    L"X", L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H",
    L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P",
};

static wchar_t* ru_pMapHortIndex[] = {
    L"X", L"1",  L"2",  L"3",  L"4",  L"5",  L"6",  L"7",  L"8",
    L"9", L"10", L"11", L"12", L"13", L"14", L"15", L"16",
};

static wchar_t* ru_pMapDepthIndex[] = {
    L"",
    L"-1",
    L"-2",
    L"-3",
};

// text that appears on the contract button

static wchar_t* ru_pContractButtonString[] = {
    L"Контракт",
};

// text that appears on the update panel buttons

static wchar_t* ru_pUpdatePanelButtons[] = {
    L"Продолжить",
    L"Стоп",
};

// Text which appears when everyone on your team is incapacitated and incapable of battle

static wchar_t* ru_LargeTacticalStr[] = {
    L"В этом секторе вам нанесли поражение!",
    L"Враг, не испытывая угрызений совести, пожрет всех до единого!",
    L"Член вашей команды захвачен (он без сознания)!",
    L"Член вашей команды захвачен в плен врагом.",
};

// Insurance Contract.c
// The text on the buttons at the bottom of the screen.

static wchar_t* ru_InsContractText[] = {
    L"Пред.",
    L"След",
    L"ОК",
    L"Очистить",
};

// Insurance Info
// Text on the buttons on the bottom of the screen

static wchar_t* ru_InsInfoText[] = {L"Пред.", L"След."};

// For use at the M.E.R.C. web site. Text relating to the player's account with MERC

static wchar_t* ru_MercAccountText[] = {
    // Text on the buttons on the bottom of the screen
    L"Подтвердить",
    L"На гл.страницу",
    L"Счет #:",
    L"Наем.",
    L"Дни",
    L"Ставка",  // 5
    L"Стоимость",
    L"Всего:",
    L"Вы уверены, что хотите подтвердить выплату %s?",  // the %s is a string that contains the
                                                        // dollar amount ( ex. "$150" )
};

// For use at the M.E.R.C. web site. Text relating a MERC mercenary

static wchar_t* ru_MercInfo[] = {
    L"Здоровье",
    L"Подвижность",
    L"Проворность",
    L"Сила",
    L"Лидерство",
    L"Мудрость",
    L"Опытность",
    L"Меткость",
    L"Механика",
    L"Взрывн.раб.",
    L"Медицина",

    L"Пред.",
    L"Нанять",
    L"Далее",
    L"Дополн.информ.",
    L"На гл.страницу",
    L"Нанят",
    L"Зарплата:",
    L"в день   .",
    L"Мертвец",

    L"Похоже,вы увлеклись набором наемников.Ваш предел-18 чел.",
    L"Недоступно",
};

// For use at the M.E.R.C. web site. Text relating to opening an account with MERC

static wchar_t* ru_MercNoAccountText[] = {
    // Text on the buttons at the bottom of the screen
    L"Открыть счет", L"Отмена", L"У вас нет счета. Хотите открыть?"};

// For use at the M.E.R.C. web site. MERC Homepage

static wchar_t* ru_MercHomePageText[] = {
    // Description of various parts on the MERC page
    L"Спек Т.Клайн,основатель", L"Открытие счета", L"Просмотр счета", L"Просмотр файлов",
    // The version number on the video conferencing system that pops up when Speck is talking
    L"Спек Ком.v3.2"};

// For use at MiGillicutty's Web Page.

static wchar_t* ru_sFuneralString[] = {
    L"Морг Макгилликути:скорбим вместе с семьями усопших с 1983.",
    L"Директор по похоронам и бывший наемник А.I.М Мюррэй \"Попс\" Макгилликати-специалист по "
    L"части похорон.",
    L"Всю жизнь Попса сопровождали смерть и утраты,поэтому он как никто познал их тяжесть.",
    L"Морг Мак Гилликути предлагает широкий спектр похоронных услуг,от жилетки,в которую можно "
    L"поплакать,до восстановления сильно поврежденных останков.",
    L"Доверьтесь моргу Мак Гилликути, и ваши родственники почиют в мире.",

    // Text for the various links available at the bottom of the page
    L"ПОСЛАТЬ ЦВЕТЫ", L"КОЛЛЕКЦИЯ УРН И ГРОБОВ", L"УСЛУГИ ПО КРЕМАЦИИ", L"ПОДГОТОВКА ПОХОРОН",
    L"ПОХОРОННЫЙ ЭТИКЕТ",

    // The text that comes up when you click on any of the links ( except for send flowers ).
    L"Семья понесла тяжелую утрату.К сожалению, не все работы еще завершены.Приходите после "
    L"прочтения завещания и выплат долгов умершего.",
    L"Надеемся,что вы ощущаете наше сочувствие в это нелегкое время."};

// Text for the florist Home page

static wchar_t* ru_sFloristText[] = {
    // Text on the button on the bottom of the page

    L"Галерея",

    // Address of United Florist

    L"\"Мы сбрасываем цветы везде\"", L"1-555-SCENT-ME", L"333 Др.Ноуз-Гей,Сиди Сити,КА США 90210",
    L"http://www.scent-me.com",

    // detail of the florist page

    L"Мы работаем быстро и эффективно!",
    L"Гарантированная доставка в течение одного дня в любую точку земного шара.Есть ограничения.",
    L"Самые низкие в мире цены!",
    L"Покажите нам рекламу подобных услуг,которые стоят дешевле и получите 10роз бесплатно.",
    L"Летающая Флора,Фауна&Цветы с 1981.",
    L"Наши сотрудники-бывшие военные летчики-сбросят ваш букет в радиусе 10миль от нужного вам "
    L"места.В любое время!Всегда!",
    L"Позвольте нам воплотить ваши цветочные фантазии в жизнь.",
    L"Пусть Брюс,известный во всем мире флорист,собственноручно соберет вам букет свежайших цветов "
    L"из наших оранжерей.",
    L"И помните-то,чего у нас нет,мы можем вырастить-быстро!"};

// Florist OrderForm

static wchar_t* ru_sOrderFormText[] = {
    // Text on the buttons

    L"Назад",
    L"Послать",
    L"Очистить",
    L"Галерея",

    L"Назв.букета:",
    L"Цена:",  // 5
    L"Номер заказа:",
    L"День доставки",
    L"след.день",
    L"дойдет когда дойдет",
    L"Место доставки",  // 10
    L"Дополнит.услуги",
    L"Сломанные цветы($10)",
    L"Черные розы($20)",
    L"Увядший букет($10)",
    L"Фруктовый пирог(если есть)($10)",  // 15
    L"Личные переживания:",
    L"Ввиду небольшого размера карточек, 75 символов - максимум.",
    L"...или посмотрите на наши",

    L"СТАНДАРТНЫЕ КАРТЫ",
    L"Информация о счете",  // 20

    // The text that goes beside the area where the user can enter their name

    L"Имя:",
};

// Florist Gallery.c

static wchar_t* ru_sFloristGalleryText[] = {
    // text on the buttons

    L"Пред",  // abbreviation for previous
    L"След",  // abbreviation for next

    L"Щелкните по тому,что хотите заказать.",
    L"Примечание:за каждый увядший или сломанный букет дополн.плата $10.",

    // text on the button

    L"На гл.стр.",
};

// Florist Cards

static wchar_t* ru_sFloristCards[] = {L"Щелкните по выбранному", L"Назад"};

// Text for Bobby Ray's Mail Order Site

static wchar_t* ru_BobbyROrderFormText[] = {
    L"Бланк заказа",              // Title of the page
    L"Ед.",                       // The number of items ordered
    L"Вес (%s)",                  // The weight of the item
    L"Название",                  // The name of the item
    L"Цена",                      // the item's weight
    L"Всего",                     // 5	// The total price of all of items of the same type
    L"Стоимость",                 // The sub total of all the item totals added
    L"ДиУ (см. Место Доставки)",  // S&H is an acronym for Shipping and Handling
    L"Общая стоим.",              // The grand total of all item totals + the shipping and handling
    L"Место доставки",
    L"Скор.доставки",              // 10	// See below
    L"Стоим.(за %s.)",             // The cost to ship the items
    L"Доставка-1день",             // Gets deliverd the next day
    L"2 рабочих дня",              // Gets delivered in 2 days
    L"Стандартный срок",           // Gets delivered in 3 days
    L"Очистить",                   // 15			// Clears the order page
    L"Принять заказ",              // Accept the order
    L"Назад",                      // text on the button that returns to the previous page
    L"На гл.стр.",                 // Text on the button that returns to the home page
    L"* Указывает БУвещи",         // Disclaimer stating that the item is used
    L"У вас нет на это средств.",  // 20	// A popup message that to warn of not enough money
    L"<НЕТ>",                      // Gets displayed when there is no valid city selected
    L"Вы уверены,что надо послать этот заказ %s?",  // A popup that asks if the city selected is the
                                                    // correct one
    L"Вес упаковки**",                              // Displays the weight of the package
    L"** Мин.вес",  // Disclaimer states that there is a minimum weight for the package
    L"Заказы",
};

// This text is used when on the various Bobby Ray Web site pages that sell items

static wchar_t* ru_BobbyRText[] = {
    L"Заказать",  // Title
    // instructions on how to order
    L"Щелкните на вещь.Если вам нужно больше одной,щелкните еще. Правый клик-умень. кол-во "
    L"вещей.Когда выберете все,что хотите,заполняйте бланк заказа.",

    // Text on the buttons to go the various links

    L"Пред.вещи",  //
    L"Пист.",      // 3
    L"Амуниция",   // 4
    L"Броня",      // 5
    L"Разн.",      // 6	//misc is an abbreviation for miscellaneous
    L"Б.У.",       // 7
    L"Еще",
    L"БЛАНК",
    L"На гл.стр.",  // 10

    // The following 2 lines are used on the Ammunition page.
    // They are used for help text to display how many items the player's merc has
    // that can use this type of ammo

    L"У вашей команды есть",                // 11
    L"Оруж.,где исп.этот тип боеприпасов",  // 12

    // The following lines provide information on the items

    L"Вес:",            // Weight of all the items of the same type
    L"Кал:",            // the caliber of the gun
    L"Маг:",            // number of rounds of ammo the Magazine can hold
    L"Рнг:",            // The range of the gun
    L"Пвр:",            // Damage of the weapon
    L"УС:",             // Weapon's Rate Of Fire, acronym ROF
    L"Цена:",           // Cost of the item
    L"На складе:",      // The number of items still in the store's inventory
    L"Заказ:кол-во:",   // The number of items on order
    L"Повреждение",     // If the item is damaged
    L"Вес:",            // the Weight of the item
    L"Итого:",          // The total cost of all items on order
    L"* %% действует",  // if the item is damaged, displays the percent function of the item

    // Popup that tells the player that they can only order 10 items at a time

    L"Дорогие клиенты!Заказ в режиме on-line позволяет заказать не более 10 вещей. Если вы хотите "
    L"заказать больше,(а мы надемся,что так и есть),заполните еще один бланк и примите наши "
    L"извинения.",

    // A popup that tells the user that they are trying to order more items then the store has in
    // stock

    L"Извините.Этот товар закончился.Попробуйте заказать его позже.",

    // A popup that tells the user that the store is temporarily sold out

    L"Извините,но все товары этого типа закончились.",

};

// Text for Bobby Ray's Home Page

static wchar_t* ru_BobbyRaysFrontText[] = {
    // Details on the web site

    L"Здесь вы можете приобрести последние новинки производства оружия и сопутствующих товаров",
    L"Мы можем предложить вам все,что нужно для взрывных работ",
    L"Б.У.",

    // Text for the various links to the sub pages

    L"Разное",
    L"ПИСТОЛЕТЫ",
    L"АМУНИЦИЯ",  // 5
    L"БРОНЯ",

    // Details on the web site

    L"Если мы этого не продаем, вам это взять неоткуда!",
    L"В разработке",
};

// Text for the AIM page.
// This is the text used when the user selects the way to sort the aim mercanaries on the AIM mug
// shot page

static wchar_t* ru_AimSortText[] = {
    L"Члены А.I.M.",  // Title
                      // Title for the way to sort
    L"Сортировка:",

    // sort by...

    L"Цена", L"Опытность", L"Меткость", L"Медицина", L"Взрывн.раб.", L"Механика",

    // Text of the links to other AIM pages

    L"Просмотреть Фото наемников", L"Просмотреть Статистику наемников",
    L"Просмотреть Историю А.I.M.",

    // text to display how the entries will be sorted

    L"По возраст.", L"По убыв."};

// Aim Policies.c
// The page in which the AIM policies and regulations are displayed

static wchar_t* ru_AimPolicyText[] = {
    // The text on the buttons at the bottom of the page

    L"Пред.стр.", L"Гл. стр.AIM", L"Правила", L"След.стр.", L"Отвергнуть", L"Согл."};

// Aim Member.c
// The page in which the players hires AIM mercenaries

// Instructions to the user to either start video conferencing with the merc, or to go the mug shot
// index

static wchar_t* ru_AimMemberText[] = {
    L"Левый щелчок",
    L"контакт с наемн.",
    L"Правый щелчок",
    L"индекс фото.",
};

// Aim Member.c
// The page in which the players hires AIM mercenaries

static wchar_t* ru_CharacterInfo[] = {
    // The various attributes of the merc

    L"Здоровье", L"Подвижность", L"Проворность", L"Сила", L"Лидерство", L"Мудрость", L"Опытность",
    L"Меткость", L"Механика", L"Взрывн.раб.",
    L"Медицина",  // 10

    // the contract expenses' area

    L"Плата", L"Срок", L"1 день", L"7 дней", L"14 дней",

    // text for the buttons that either go to the previous merc,
    // start talking to the merc, or go to the next merc

    L"Пред.", L"Контакт", L"След.",

    L"Дополнит.инф.",            // Title for the additional info for the merc's bio
    L"Действ.члены",             // 20		// Title of the page
    L"Стоим.оборудования:",      // Displays the optional gear cost
    L"Необходимый мед.депозит",  // If the merc required a medical deposit, this is displayed
};

// Aim Member.c
// The page in which the player's hires AIM mercenaries

// The following text is used with the video conference popup

static wchar_t* ru_VideoConfercingText[] = {
    L"Цена контракта:",  // Title beside the cost of hiring the merc

    // Text on the buttons to select the length of time the merc can be hired

    L"1 день", L"7 дней", L"14 дней",

    // Text on the buttons to determine if you want the merc to come with the equipment

    L"Нет экипировки", L"Купить экипир.",

    // Text on the Buttons

    L"ПЕРЕВЕСТИ ФОНДЫ",  // to actually hire the merc
    L"ОТМЕНА",           // go back to the previous menu
    L"НАНЯТЬ",           // go to menu in which you can hire the merc
    L"ПРЕКР.РАЗГОВОР",   // stops talking with the merc
    L"OK",
    L"ОСТАВИТЬ СООБЩ.",  // if the merc is not there, you can leave a message

    // Text on the top of the video conference popup

    L"Видеоконференция с", L"Соединение. . .",

    L"с мед.депоз."  // Displays if you are hiring the merc with the medical deposit
};

// Aim Member.c
// The page in which the player hires AIM mercenaries

// The text that pops up when you select the TRANSFER FUNDS button

static wchar_t* ru_AimPopUpText[] = {
    L"ПЕРЕВОД ФОНДОВ ЗАВЕРШЕН УСПЕШНО",  // You hired the merc
    L"НЕЛЬЗЯ ПЕРЕВЕСТИ ФОНДЫ",           // Player doesn't have enough money, message 1
    L"НЕДОСТАТОЧНО СРЕДСТВ",             // Player doesn't have enough money, message 2

    // if the merc is not available, one of the following is displayed over the merc's face

    L"На задании",
    L"Оставьте сообщение",
    L"Скончался",

    // If you try to hire more mercs than game can support

    L"У вас уже набрано 18 наемников-полная команда.",

    L"Сообщение",
    L"Сообщ. оставлено",
};

// AIM Link.c

static wchar_t* ru_AimLinkText[] = {
    L"Линки A.I.M.",  // The title of the AIM links page
};

// Aim History

// This page displays the history of AIM

static wchar_t* ru_AimHistoryText[] = {L"Журнал A.I.M.",  // Title

                                       // Text on the buttons at the bottom of the page

                                       L"Пред.стр.", L"Гл.стр.",
                                       L"История A.I.M.",  //$$
                                       L"След.стр."};

// Aim Mug Shot Index

// The page in which all the AIM members' portraits are displayed in the order selected by the AIM
// sort page.

static wchar_t* ru_AimFiText[] = {
    // displays the way in which the mercs were sorted

    L"Цена",
    L"Опытность",
    L"Меткость",
    L"Медицина",
    L"Взрывн.раб.",
    L"Механика",

    // The title of the page, the above text gets added at the end of this text

    L"Члены A.I.M.:сортировка по возраст. %s",
    L"Члены A.I.M.:сортировка по убыв. %s",

    // Instructions to the players on what to do

    L"Левый щелчок",
    L"Выбрать наемн",  // 10
    L"Правый щелчок",
    L"Упорядочить выбор",

    // Gets displayed on top of the merc's portrait if they are...

    L"Отсутствует",
    L"Скончался",  // 14
    L"На задании",
};

// AimArchives.
// The page that displays information about the older AIM alumni merc... mercs who are no longer
// with AIM

static wchar_t* ru_AimAlumniText[] = {
    // Text of the buttons

    L"СТР 1", L"СТР 2", L"СТР 3",

    L"История A.I.M.",  // Title of the page //$$

    L"ОК"  // Stops displaying information on selected merc
};

// AIM Home Page

static wchar_t* ru_AimScreenText[] = {
    // AIM disclaimers

    L"A.I.M. и логотип A.I.M.-зарегистрированные во многих странах торговые марки.",
    L"Поэтому даже и не думайте нас копировать.",
    L"Копирайт 1998-1999 A.I.M.,Ltd.Все права защищены.",

    // Text for an advertisement that gets displayed on the AIM page

    L"Объединенные цветочные службы",
    L"\"Мы сбрасываем в любои месте\"",  // 10
    L"Сделай как надо",
    L"... первый раз",
    L"Если у нас нет такого пистолета,он вам и не нужен.",
};

// Aim Home Page

static wchar_t* ru_AimBottomMenuText[] = {
    // Text for the links at the bottom of all AIM pages
    L"Гл.стр.",  L"Члены",
    L"История",  //$$
    L"Принципы", L"Журнал", L"Ссылки"};

// ShopKeeper Interface
// The shopkeeper interface is displayed when the merc wants to interact with
// the various store clerks scattered through out the game.

static wchar_t* ru_SKI_Text[] = {
    L"ИМЕЮЩИЕСЯ ТОВАРЫ",  // Header for the merchandise available
    L"СТР",               // The current store inventory page being displayed
    L"ОБЩАЯ СТОИМ",       // The total cost of the the items in the Dealer inventory area
    L"ОБЩАЯ ЦЕНА",        // The total value of items player wishes to sell
    L"ОЦЕНКА",            // Button text for dealer to evaluate items the player wants to sell
    L"ПЕРЕДАЧА",          // Button text which completes the deal. Makes the transaction.
    L"ГОТОВО",            // Text for the button which will leave the shopkeeper interface.
    L"СТОИМ.РЕМОНТА",     // The amount the dealer will charge to repair the merc's goods
    L"1 ЧАС",     // SINGULAR! The text underneath the inventory slot when an item is given to the
                  // dealer to be repaired
    L"%d ЧАСОВ",  // PLURAL!   The text underneath the inventory slot when an item is given to the
                  // dealer to be repaired
    L"ОТРЕМОНТИРОВАНО",  // Text appearing over an item that has just been repaired by a NPC
                         // repairman dealer
    L"Вам уже некуда класть вещи.",  // Message box that tells the user there is no more room to put
                                     // there stuff
    L"%d МИНУТ",  // The text underneath the inventory slot when an item is given to the dealer to
                  // be repaired
    L"Бросьте вещи на землю.",
};

// ShopKeeper Interface
// for the bank machine panels. Referenced here is the acronym ATM, which means Automatic Teller
// Machine

static wchar_t* ru_SkiAtmText[] = {
    // Text on buttons on the banking machine, displayed at the bottom of the page
    L"0",        L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9",
    L"OK",        // Transfer the money
    L"Взять",     // Take money from the player
    L"Дать",      // Give money to the player
    L"Отмена",    // Cancel the transfer
    L"Очистить",  // Clear the money display
};

// Shopkeeper Interface
static wchar_t* ru_gzSkiAtmText[] = {

    // Text on the bank machine panel that....
    L"Select Type",             // tells the user to select either to give or take from the merc
    L"Введите сумму",           // Enter the amount to transfer
    L"Перевести деньги наемн",  // Giving money to the merc
    L"Забрать деньги у наемн",  // Taking money from the merc
    L"Недостаточно средств",    // Not enough money to transfer
    L"Баланс",                  // Display the amount of money the player currently has
};

static wchar_t* ru_SkiMessageBoxText[] = {
    L"Вы хотите снять %s со своего основного счета,чтобы покрыть разницу?",
    L"Недостаточно денег.Не хватает %s",
    L"Вы хотите снять %s со своего основного счета,чтобы покрыть стоимость?",
    L"Попросить торговца начать перевод",
    L"Попросить торговца починить выбр.вещи",
    L"Закончить разговор",
    L"Текущий баланс",
};

// OptionScreen.c

static wchar_t* ru_zOptionsText[] = {
    // button Text
    L"Сохранить",
    L"Загрузить",
    L"Выход",
    L"Готово",

    // Text above the slider bars
    L"Эффекты",
    L"Речь",
    L"Музыка",

    // Confirmation pop when the user selects..
    L"Выйти из игры и вернуться в главное меню?",

    L"Нужно выбрать либо РЕЧЬ, либо СУБТИТРЫ.",
};

// SaveLoadScreen
static wchar_t* ru_zSaveLoadText[] = {
    L"Сохранить",
    L"Загрузить",
    L"Отмена",
    L"Сохр.выбр.",
    L"Загр.выбр.",

    L"Игра сохранена",
    L"ОШИБКА при сохранении!",
    L"Игра загружена",
    L"ОШИБКА при загрузке!",

    L"Сохраненная версия игры отличается от текущей.Надежнее всего продолжить.Продолжить?",
    L"Файлы сохраненной игры могут быть с ошибкой.Уничтожить их все?",

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Сохр.версия была изменена.Сообщите о проблемах.Продолжить?",
#else
    L"Попытка загрузки старой версии. Обновить автоматически и загрузить?",
#endif

// Translators, the next two strings are for the same thing.  The first one is for beta version
// releases and the second one is used for the final version.  Please don't modify the "#ifdef
// JA2BETAVERSION" or the "#else" or the "#endif" as they are used by the compiler and will cause
// program errors if modified/removed.  It's okay to translate the strings though.
#ifdef JA2BETAVERSION
    L"Сохр.версии и версии игры были изменены. Сообщите о проблемах. Продолжить?",
#else
    L"Попытка загрузки старой версии. Обновить автоматически и загрузить?",
#endif

    L"Вы уверены,что хотите записать сох.игру поверх #%d?",
    L"Вы хотите загрузить игру из ячейки #",

    // The first %d is a number that contains the amount of free space on the users hard drive,
    // the second is the recommended amount of free space.
    L"У вас кончается дисковое пространство. Осталось всего %d Mбайт. Нужно как минимум %d "
    L"свободных Mбайт.",

    L"Сохранение...",  // When saving a game, a message box with this string appears on the screen

    L"Обычные пист.",
    L"Тонны пист.",
    L"Реалист.стиль",
    L"Фантаст. стиль",

    L"Сложн.",
};

// MapScreen
static wchar_t* ru_zMarksMapScreenText[] = {
    L"Уровень карты",
    L"У вас нет ополчения.Надо подготовить горожан,и у вас будет городское ополчение.",
    L"Доход в день",
    L"У наемн.есть страховка",
    L"%s не устал.",
    L"%s движется и спать не может",
    L"%s слишком устал,попробуйте позже.",
    L"%s за рулем.",
    L"Отряд не может двигаться,когда один наемн.спит.",

    // stuff for contracts
    L"Вы можете платить по контракту,но у вас нет денег на страховые премии этому наемн.",
    L"%s:страховая премия составит %s за %d дополн.дней.Хотите платить?",
    L"Инвентарь Сектора",
    L"У наемн.есть мед.депозит.",

    // other items
    L"Медики",    // people acting a field medics and bandaging wounded mercs
    L"Пациенты",  // people who are being bandaged by a medic
    L"Готово",    // Continue on with the game after autobandage is complete
    L"Стоп",      // Stop autobandaging of patients by medics now
    L"Извините.Эта опция невозможна,т.к.это демо-версия.",  // informs player this option/button has
                                                            // been disabled in the demo
    L"%s:нет ремонтных принадл.",
    L"%s:нет медицинских принадл.",
    L"Недостаточно людей,желающих пройти подготовку.",
    L"%s:много ополченцев.",
    L"У наемн.конечн.контракт.",
    L"Контракт наемн.не застрахован",
};

static wchar_t* ru_pLandMarkInSectorString[] = {
    L"Отряд %d заметил кого-то в секторе %s",
};

// confirm the player wants to pay X dollars to build a militia force in town
static wchar_t* ru_pMilitiaConfirmStrings[] = {
    L"Тренировка отряда город.ополч.будет стоить $",  // telling player how much it will cost
    L"Одобрить платеж?",         // asking player if they wish to pay the amount requested
    L"У вас нет денег на это.",  // telling the player they can't afford to train this town
    L"Продолжить тренировку ополчения в %s (%s %d)?",  // continue training this town?
    L"Стоит $",                                        // the cost in dollars to train militia
    L"( Д/Н )",                                        // abbreviated yes/no
    L"",                                               // unused
    L"Тренировка отряда город.ополч.в секторе %d будет стоить $ %d. %s",  // cost to train sveral
                                                                          // sectors at once
    L"У вас нет $%d на тренировку город.ополчения здесь.",
    L"%s:нужно %d процентов верности тебе,чтобы продолжить тренировку ополчения.",
    L"В %s больше нельзя тренировать ополчение.",
};

// Strings used in the popup box when withdrawing, or depositing money from the $ sign at the bottom
// of the single merc panel
static wchar_t* ru_gzMoneyWithdrawMessageText[] = {
    L"За один раз можно брать не более $20,000.",
    L"Вы уверены, что хотите положить %s на свой счет?",
};

static wchar_t* ru_gzCopyrightText[] = {
    L"Авторские права(C) 1999 Sir-Tech Canada Ltd. Все права защищены. Распространение на "
    L"территории стран СНГ компания БУКА",
};

// option Text
static wchar_t* ru_zOptionsToggleText[] = {
    L"Речь",
    L"Немое подтверждение",  //$$
    L"Субтитры",
    L"Диалоги с паузами",
    L"Анимированный дым",
    L"Кровища",
    L"Не трожь мою мышь!",
    L"Старый способ выбора",
    L"Показывать движения",
    L"Показывать промахи",
    L"Игра в реальном времени",
    L"Показать индикатор врага",
    L"Использовать метрич.систему",
    L"Выделять наемн.во время движения",
    L"Перевести курсор на наемн.",
    L"Перевести курсор на двери",
    L"Мерцание вещей",
    L"Показать верхушки деревьев",
    L"Показывать каркасы",
    L"Показать трехмерный курсор",
};

// This is the help text associated with the above toggles.
static wchar_t* ru_zOptionsScreenHelpText[] = {
    // speech
    L"Если вы хотите услышать диалог персонажей,включите эту опцию.",

    // Mute Confirmation
    L"Вкл/выкл вербальное подтверждение.",

    // Subtitles
    L" whether on-screen text is displayed for dialogue.",  //$$

    // Key to advance speech
    L"Если субитры включены,включите также это,чтобы читать NPC диалог.",

    // Toggle smoke animation
    L"Отключите эту опцию,если анимированный дым замедляет игру.",

    // Blood n Gore
    L"Отключите эту опцию,если боитесь крови.",

    // Never move my mouse
    L"Отключите эту опцию,чтобы курсор автоматически перемещался на всплывающее меню диалога",

    // Old selection method
    L"Включите эту опцию,чтобы персонажи работали,как в предыдущей версии Jagged Alliance (иначе "
    L"они будут работать по-другому).",

    // Show movement path
    L"Включите эту опцию,чтобы действие происходило в реальном времени(или используйте SHIFT при "
    L"отключенной опции).",

    // show misses
    L"Включите эту опцию,чтобы видеть,куда попадают пули при \"промахе\".",

    // Real Time Confirmation
    L"Когда опция вкл,дополн.\"безопасность\" щелчок нужен для перехода в реальное время.",

    // Display the enemy indicator
    L"Когда опция вкл,количество врагов,видных наемнику,высвечивается над его портретом.",

    // Use the metric system
    L"Когда опция вкл,исп.метрич.система,иначе-британская.",

    // Merc Lighted movement
    L"Когда опция вкл,путь наемника обозн.светящейся линией.Отключите для быстроты.",

    // Smart cursor
    L"Когда опция вкл,передвижение курсора на наемника будет его выделять.",

    // snap cursor to the door
    L"Когда опция вкл,передвижение курсора на дверь будет автоматически передвигать его поверх "
    L"двери.",

    // glow items
    L"Когда опция вкл,вещи светятся( |I)",

    // toggle tree tops
    L"Когда опция вкл,показываются верхушки деревьев.( |Е)",

    // toggle wireframe
    L"Когда опция вкл,видны каркасы домов. ( |W)",  //$$

    L"Когда опция вкл,движущийся курсор-трехмерный.( |Home )",

};

static wchar_t* ru_gzGIOScreenText[] = {
    L"УСТАНОВКА НАЧАЛА ИГРЫ",
    L"Стиль игры",
    L"Реалистичный",
    L"Фантастичный",
    L"Выбор пистолетов",
    L"Сотни пистолетов",
    L"Нормальный",
    L"Уровень сложности",
    L"Легкий",
    L"Нормальный",
    L"Трудный",
    L"Ok",
    L"Отмена",
    L"Дополнительная сложность",
    L"Без ограничений времени",
    L"Время хода ограничено",
    L"Отключено в демо-версии",
};

static wchar_t* ru_pDeliveryLocationStrings[] = {
    L"Остен",     // Austin, Texas, USA
    L"Багдад",    // Baghdad, Iraq (Suddam Hussein's home)
    L"Драссен",   // The main place in JA2 that you can receive items.  The other towns are dummy
                  // names...
    L"Гон Конг",  // Hong Kong, Hong Kong
    L"Бейрут",    // Beirut, Lebanon	(Middle East)
    L"Лондон",    // London, England
    L"Лос Анджелес",  // Los Angeles, California, USA (SW corner of USA)
    L"Медуна",        // Meduna -- the other airport in JA2 that you can receive items.
    L"Метавира",      // The island of Metavira was the fictional location used by JA1
    L"Майами",        // Miami, Florida, USA (SE corner of USA)
    L"Москва",        // Moscow, USSR
    L"Нью Йорк",      // New York, New York, USA
    L"Оттава",        // Ottawa, Ontario, Canada -- where JA2 was made!
    L"Париж",         // Paris, France
    L"Триполи",       // Tripoli, Libya (eastern Mediterranean)
    L"Токио",         // Tokyo, Japan
    L"Ванкувер",      // Vancouver, British Columbia, Canada (west coast near US border)
};

static wchar_t* ru_pSkillAtZeroWarning[] =
    {  // This string is used in the IMP character generation.  It is possible to select 0 ability
       // in a skill meaning you can't use it.  This text is confirmation to the player.
        L"Уверен? Ноль означает отсутствие навыков."};

static wchar_t* ru_pIMPBeginScreenStrings[] = {
    L"( 8 макс. символов )",
};

static wchar_t* ru_pIMPFinishButtonText[1] = {
    L"Анализ",
};

static wchar_t* ru_pIMPFinishStrings[] = {
    L"Спасибо,%s",  //%s is the name of the merc
};

// the strings for imp voices screen
static wchar_t* ru_pIMPVoicesStrings[] = {
    L"Голос",
};

static wchar_t* ru_pDepartedMercPortraitStrings[] = {
    L"Убит в бою",
    L"Уволен",
    L"Другое",
};

// title for program
static wchar_t* ru_pPersTitleText[] = {
    L"Кадры",
};

// paused game strings
static wchar_t* ru_pPausedGameText[] = {
    L"Пауза",
    L"Возобновить (|P|a|u|s|e)",
    L"Поставить на паузу (|P|a|u|s|e)",
};

static wchar_t* ru_pMessageStrings[] = {
    L"Выйти из игры?",
    L"OK",
    L"ДА",
    L"НЕТ",
    L"ОТМЕНА",
    L"НАНЯТЬ",
    L"ЛОЖЬ",
    L"No description",  // Save slots that don't have a description.
    L"Игра сохранена",
    L"Игра сохранена",
    L"QuickSave",  // The name of the quicksave file (filename, text reference)
    L"SaveGame",   // The name of the normal savegame file, such as SaveGame01, SaveGame02, etc.
    L"sav",        // The 3 character dos extension (represents sav)
    L"..\\SavedGames",  // The name of the directory where games are saved.
    L"День",
    L"Наемн",
    L"Пустая ячейка",  // An empty save game slot
    L"Демо",           // Demo of JA2
    L"Ловля Багов",    // State of development of a project (JA2) that is a debug build
    L"Release",        // Release build for JA2
    L"пвм",   // Abbreviation for Rounds per minute -- the potential # of bullets fired in a minute.
    L"мин",   // Abbreviation for minute.
    L"м",     // One character abbreviation for meter (metric distance measurement unit).
    L"пули",  // Abbreviation for rounds (# of bullets)
    L"кг",    // Abbreviation for kilogram (metric weight measurement unit)
    L"ф",     // Abbreviation for pounds (Imperial weight measurement unit)
    L"Гл.стр",                      // Home as in homepage on the internet.
    L"USD",                         // Abbreviation to US dollars
    L"н/п",                         // Lowercase acronym for not applicable.
    L"В это время",                 // Meanwhile
    L"%s прибыл(а) в сектор %s%s",  // Name/Squad has arrived in sector A9.  Order must not change
                                    // without notifying SirTech
    L"Версия",
    L"Пустая ячейка быстрого сохр",
    L"Эта ячейка-для быстрого сохранения экранов игры (ALT+S).",
    L"Открыто",
    L"Закрыто",
    L"У вас кончается дисковое пространство. У вас осталось %sМБ свободных,а для АЛЬЯНСА 2 "
    L"требуется %sMБ.",
    L"Нанят %s из AIM",
    L"%s поймал %s.",  //'Merc name' has caught 'item' -- let SirTech know if name comes after item.
    L"%s принял лекарство.",    //'Merc name' has taken the drug
    L"%s не имеет меднавыков",  //'Merc name' has no medical skill.

    // CDRom errors (such as ejecting CD while attempting to read the CD)
    L"Нарушена целостность программы.",
    L"ОШИБКА: Выньте CD-ROM",

    // When firing heavier weapons in close quarters, you may not have enough room to do so.
    L"Мало места для стрельбы.",

    // Can't change stance due to objects in the way...
    L"Сейчас изменить положение нельзя.",

    // Simple text indications that appear in the game, when the merc can do one of these things.
    L"Уронить",
    L"Бросить",
    L"Передать",

    L"%s передано %s.",  //"Item" passed to "merc".  Please try to keep the item %s before the merc
                         //%s, otherwise, must notify SirTech.
    L"Нельзя передать %s %s.",  // pass "item" to "merc".  Same instructions as above.

    // A list of attachments appear after the items.  Ex:  Kevlar vest ( Ceramic Plate 'Attached )'
    L" Присоединено )",

    // Cheat modes
    L"Достигнут чит-уровень один",
    L"Достигнут чит-уровень два",

    // Toggling various stealth modes
    L"Отряд скрыт.",
    L"Отряд виден.",
    L"%s скрыт.",
    L"%s открыт.",

    // Wireframes are shown through buildings to reveal doors and windows that can't otherwise be
    // seen in an isometric engine.  You can toggle this mode freely in the game.
    L"Дополнительные Каркасы Вкл",   //$$
    L"Дополнительные Каркасы Выкл",  //$$

    // These are used in the cheat modes for changing levels in the game.  Going from a basement
    // level to an upper level, etc.
    L"Нельзя подняться с этого уровня...",
    L"Ниже уровней нет...",
    L"Входим в подвальный уровень %d...",
    L"Уходим из подвала...",

    L".",  // used in the shop keeper inteface to mark the ownership of the item eg Red's gun
    L"ВЫКЛЮЧЕНО.",
    L"ВКЛЮЧЕНО.",
    L"3D-курсор ОТКЛ.",
    L"3D-курсор ВКЛ.",
    L"Отряд %d действует.",
    L"У вас нет денег,чтобы ежедневно выплачивать %s %s",  // first %s is the mercs name, the
                                                           // seconds is a string containing the
                                                           // salary
    L"Пропуск",
    L"%s не может уйти один.",
    L"Игра была сохранена под именем SaveGame99.sav. При необходимости пересохраните ее под именем "
    L"SaveGame01-SaveGame10 и тогда вы будете получите доступ к ней в экране Загрузка.",
    L"%s выпил немного %s",
    L"Багаж прибыл в Драссен.",
    L"%s должен прибыть в указанное место высадки (сектор %s) в день %d,примерно в %s.",  // first
                                                                                          // %s is
                                                                                          // mercs
                                                                                          // name,
                                                                                          // next is
                                                                                          // the
                                                                                          // sector
                                                                                          // location
                                                                                          // and
                                                                                          // name
                                                                                          // where
                                                                                          // they
                                                                                          // will be
                                                                                          // arriving
                                                                                          // in,
                                                                                          // lastely
                                                                                          // is the
                                                                                          // day an
                                                                                          // the
                                                                                          // time of
                                                                                          // arrival
    L"История обновлена.",
#ifdef JA2BETAVERSION
    L"Игра сохранена в ячейку авто-сохранения.",
#endif
};

static wchar_t* ru_ItemPickupHelpPopup[] = {L"OK", L"Листать вверх", L"Выделить все",
                                            L"Листать вниз", L"Отмена"};

static wchar_t* ru_pDoctorWarningString[] = {
    L"%s слишком далеко,чтобы его можно было лечить.",
    L"Медики не могут перевязать всех.",
};

static wchar_t* ru_pMilitiaButtonsHelpText[] = {
    L"Взять новичков(Правый щелчок)/Отвергнуть(Левый щелчок)",  // button help text informing player
                                                                // they can pick up or drop militia
                                                                // with this button
    L"Взять постоянные войска(Правый щелчок)/Отвергнуть(Левый щелчок)",
    L"Взять войска ветеранов(Правый щелчок)/Отвергнуть(Левый щелчок)",
    L"Равномерно распределить доступное ополчение по всем секторам",
};

static wchar_t* ru_pMapScreenJustStartedHelpText[] = {
    L"Иди в АIM и нанять наемников (*Подсказка* в лаптопе)",  // to inform the player to hired some
                                                              // mercs to get things going
    L"Если вы готовы отправиться в Арулько,щелкните по кнопке Компрессия времени в правом нижнем "
    L"углу экрана.",  // to inform the player to hit time compression to get the game underway
};

static wchar_t* ru_pAntiHackerString[] = {
    L"Ошибка.Испорченные или отсутствующие файлы.Вы выходите из игры.",
};

static wchar_t* ru_gzLaptopHelpText[] = {
    // Buttons:
    L"Просмотреть почту",
    L"Пролистать web страницы",
    L"Просмотреть файлы и аттачменты.",
    L"Прочитать последние события",
    L"Информация о команде",
    L"Просмотреть финансовое заключение и журнал",
    L"Закрыть лаптоп",

    // Bottom task bar icons (if they exist):
    L"Новое сообщение",
    L"Новые файлы",

    // Bookmarks:
    L"Международная Ассоциация Наемников",
    L"Бобби Рэй-заказ оружия в сети",
    L"Институт Психологии Наемников",
    L"Рекрутинговый Центр",
    L"Морг Мак Гилликути",
    L"Объединенная цветочная служба",
    L"Страховые агенты по контрактам A.I.M.",
};

static wchar_t* ru_gzHelpScreenText[] = {
    L"Выход из экрана помощь",
};

static wchar_t* ru_gzNonPersistantPBIText[] = {
    L"Идет бой. Вы можете только покинуть экран битвы.",
    L"Войти в сектор, чтобы продолжить бой( |E).",
    L"Автоматически остановить текущую битву ( |A).",
    L"Нельзя автоматически остановить битву, когда ты нападаешь.",
    L"Нельзя автоматически остановить битву,когда на тебя напали.",
    L"Нельзя автоматически остановить битву,когда дерешься с существами в шахтах.",
    L"Нельзя автоматически остановить битву,если поблизости враждебные жители.",
    L"Нельзя автоматически остановить битву,если поблизости кошки-убийцы.",
    L"ИДЕТ БОЙ",
    L"Сейчас отступать нельзя.",
};

static wchar_t* ru_gzMiscString[] = {
    L"Ваше ополчение дерется без помощи наемников...",
    L"Машине пока не нужно заправляться.",
    L"Бензобак полон на %d%%.",
    L"Армия Дейдранны полностью контролирует территорию %s.",
    L"Вы потеряли заправку.",
};

static wchar_t* ru_gzIntroScreen[] = {
    L"Невозможно найти вступительный ролик",
};

// These strings are combined with a merc name, a volume string (from pNoiseVolStr),
// and a direction (either "above", "below", or a string from pDirectionStr) to
// report a noise.
// e.g. "Sidney hears a loud sound of MOVEMENT coming from the SOUTH."
static wchar_t* ru_pNewNoiseStr[] = {
    L"%s слышит %s звук, идущий с %sА.",  L"%s слышит %s звук ДВИЖЕНИЯ, идущий с %sА.",
    L"%s слышит %s СКРИП, идущий с %sА.", L"%s слышит %s ПЛЕСК, идущий с %sА.",
    L"%s слышит %s УДАР, идущий с %sА.",  //$$
    L"%s слышит %s ВЗРЫВ на %sЕ.",        L"%s слышит %s КРИК с %sА.",
    L"%s слышит %s УДАР с %sА.",          L"%s слышит %s УДАР с %sА.",
    L"%s слышит %s ЗВОН, идущий с %sА.",  L"%s слышит %s ГРОХОТ, идущий  %sА.",
};

static wchar_t* ru_wMapScreenSortButtonHelpText[] = {
    L"Сортировка по имени (|F|1)",      L"Сортировка по назн. (|F|2)",
    L"Сортировка по сну (|F|3)",        L"Сортировка по месту (|F|4)",
    L"Сортировка по месту назн.(|F|5)", L"Сортировка по времени контракта (|F|6)",
};

static wchar_t* ru_BrokenLinkText[] = {
    L"Ошибка 404",
    L"URL не найден.",
};

static wchar_t* ru_gzBobbyRShipmentText[] = {
    L"Посл.поступления",
    L"Заказ #",
    L"Количество",
    L"Заказано",
};

static wchar_t* ru_gzCreditNames[] = {
    L"Chris Camfield",
    L"Shaun Lyng",
    L"Kris Mornes",
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

static wchar_t* ru_gzCreditNameTitle[] = {
    L"Ведущий программист игры",                         // Chris Camfield !!!
    L"Дизайн/Сценарий",                                  // Shaun Lyng
    L"Программист стратегической части и Редактора",     // Kris Marnes
    L"Продюсер/Дизайн",                                  // Ian Currie
    L"Дизайн/Дизайн карт",                               // Linda Currie
    L"Художник",                                         // Eric \"WTF\" Cheng
    L"Бета-Координатор, фин.поддержка",                  // Lynn Holowka
    L"Главный художник",                                 // Norman \"NRG\" Olsen
    L"Мастер по звуку",                                  // George Brooks
    L"Дизайн экрана/Художник",                           // Andrew Stacey
    L"Главный художник/Анимация",                        // Scot Loving
    L"Главный программист",                              // Andrew \"Big Cheese Doddle\" Emmons
    L"Программист",                                      // Dave French
    L"Программист стратегической части и баланса игры",  // Alex Meduna
    L"Художник по портретам",                            // Joey \"Joeker\" Whelan",
};

static wchar_t* ru_gzCreditNameFunny[] = {
    L"",                                             // Chris Camfield
    L"(все еще зубрит правила пунктуации)",          // Shaun Lyng
    L"(\"Готово. Я просто чиню\")",                  // Kris \"The Cow Rape Man\" Marnes
    L"(он уже слишком стар для этого)",              // Ian Currie
    L"(работает над Wizardry 8)",                    // Linda Currie
    L"(тестировал по дулом пистолета)",              // Eric \"WTF\" Cheng
    L"(Ушла от нас в CFSA - скатертью дорожка...)",  // Lynn Holowka
    L"",                                             // Norman \"NRG\" Olsen
    L"",                                             // George Brooks
    L"(Мертвая Голова и любитель джаза)",            // Andrew Stacey
    L"(его настоящее имя Роберт)",                   // Scot Loving
    L"(единственное ответственное лицо)",            // Andrew \"Big Cheese Doddle\" Emmons
    L"(может опять заняться мотогонками)",           // Dave French
    L"(украден с работы над Wizardry 8)",            // Alex Meduna
    L"(строил предметы и загрузочные экраны!)",      // Joey \"Joeker\" Whelan",
};

static wchar_t* ru_sRepairsDoneString[] = {
    L"%s закончил ремонт своих вещей",
    L"%s закончил ремонтировать все оружие и броню",
    L"%s закончил ремонтировать все снаряжение",
    L"%s закончил ремонтировать все транспортируемые вещи",
};

static wchar_t* ru_zGioDifConfirmText[] = {
    L"Вы выбрали ЛЕГКИЙ режим. Это подходит для новичков в Jagged Alliance 'Агония Власти', для "
    L"новичков в жанре стратегий, или для тех, кто желает сократить битвы в игре. Ваш выбор "
    L"скажется на игре в целом, так что выбирайте с умом. Вы уверены, что хотите играть в Легком "
    L"режиме?",
    L"Вы выбрали НОРМАЛЬНЫЙ режим. Это подходит для всех тех, кто уже знаком с Jagged Alliance "
    L"'Агония Власти' или с подобными играми. Ваш выбор скажется на игре в целом, так что "
    L"выбирайте с умом. Вы уверены, что хотите играть в Нормальном режиме?",
    L"Вы выбрали ТРУДНЫЙ режим. Мы Вас предупреждаем. Нечего на нас пенять, если вас доставят "
    L"назад в цинковом гробу. Ваш выбор скажется на игре в целом, так что выбирайте с умом. Вы "
    L"уверены, что хотите играть в Трудном режиме?",
};

static wchar_t* ru_gzLateLocalizedString[] = {
    L"%S файл для загрузки экрана не найден...",

    // 1-5
    L"Робот не может покинуть сектор,т.к.некому управлять им.",

    // This message comes up if you have pending bombs waiting to explode in tactical.
    L"Сейчас сжимать время нельзя.Подождите фейерверка!",

    //'Name' refuses to move.
    L"%s отказывается двигаться.",

    //%s a merc name
    L"%s:недостаточно энергии,чтобы поменять положение.",

    // A message that pops up when a vehicle runs out of gas.
    L"%s:кончилось топливо и он остается в %c%d.",

    // 6-10

    // the following two strings are combined with the pNewNoise[] strings above to report noises
    // heard above or below the merc
    L"над",
    L"под",

    // The following strings are used in autoresolve for autobandaging related feedback.
    L"Ни у кого из ваших наемников нет меднавыков.",
    L"Нет материала для перевязок.",
    L"Не хватает материалов,чтобы перевязать всех.",
    L"Перевязывать ваших наемников не нужно.",
    L"Перевязывать наемников автоматически.",
    L"Все наемники перевязаны.",

    // 14
    L"Арулько",

    L"(roof)",

    L"Здоровье: %d/%d",

    // In autoresolve if there were 5 mercs fighting 8 enemies the text would be "5 vs. 8"
    //"vs." is the abbreviation of versus.
    L"%d против %d",

    L"%s полон!",  //(ex "The ice cream truck is full")

    L"%s нуждается не в перевязке и первой помощи, а в серьезном медицинском обследовании и/или "
    L"отдыхе.",

    // 20
    // Happens when you get shot in the legs, and you fall down.
    L"%s ранен в ногу и без сознания!",
    // Name can't speak right now.
    L"%s сейчас говорить не может.",

    // 22-24 plural versions @@@2 elite to veteran
    L"%d новички стали ветеранами.",
    L"%d новички стали постояным ополчением.",
    L"%d постоянное ополчение стало ветеранами.",

    // 25
    L"Перекл.",

    // 26
    // Name has gone psycho -- when the game forces the player into burstmode (certain unstable
    // characters)
    L"%s двинулся умом!",

    // 27-28
    // Messages why a player can't time compress.
    L"Сейчас опасно сжимать время, поскольку у вас есть наемники в секторе %s.",  //
    L"Опасно сжимать время, когда наемники находятся в шахтах с существами.",     //

    // 29-31 singular versions @@@2 elite to veteran
    L"1 новичок стал ветеранами.",
    L"1 новичок стал постоянным ополчением.",
    L"1 постоянное ополчение стало заслуженным.",

    // 32-34
    L"%s ничего не говорит.",
    L"Выбираться на поверхность?",
    L"(Отряд %d)",

    // 35
    // Ex: "Red has repaired Scope's MP5K".  Careful to maintain the proper order (Red before Scope,
    // Scope before MP5K)
    L"%s починил %s %s",

    // 36
    L"КОШКА-УБИЙЦА",

    // 37-38 "Name trips and falls"
    L"%s падает",
    L"Эту вещь отсюда брать нельзя.",

    // 39
    L"Никто из оставшихся наемн.не может драться.Ополчение сразится с существами само.",

    // 40-43
    //%s is the name of merc.
    L"%s:медикаменты кончились!",
    L"%s не обладает навыками,чтобы лечить кого-либо!",
    L"%s:кончились инструменты!",
    L"%s не обладает навыками,чтобы ремонтировать что-либо!",

    // 44-45
    L"Время ремонта",
    L"%s Не может увидеть этого человека.",

    // 46-48
    L"%s'. Барабан его пистолета сломан!",
    L"Не разрешается больше %d тренеров ополчения на сектор.",
    L"Уверен?",

    // 49-50
    L"Компрессия времени",
    L"Бак теперь заправлен.",

    // 51-52 Fast help text in mapscreen.
    L"Продолжить компрессию времени (|S|p|a|c|e)",
    L"Прекратить компрессию времени (|E|s|c)",

    // 53-54 "Magic has unjammed the Glock 18" or "Magic has unjammed Raven's H&K G11"
    L"%s исправил(а) %s",
    L"%s исправил(а) %s (%s)",

    // 55
    L"Невозможно сжимать время при просмотре содержимого сектора.",

    L"CD Агония Власти не найден. Программа выходит в ОС.",

    L"Предметы успешно совмещены.",

    // 58
    // Displayed with the version information when cheats are enabled.
    L"Текущий/Максимальный: %d%%/%d%%",

    // 59
    L"Сопроводить Джона и Мэри?",

    L"Выключатель нажат.",
};

void UseTextRussian() {
  gzProsLabel = ru_gzProsLabel;
  gzConsLabel = ru_gzConsLabel;
  sRepairsDoneString = ru_sRepairsDoneString;
  AmmoCaliber = ru_AmmoCaliber;
  BobbyRayAmmoCaliber = ru_BobbyRayAmmoCaliber;
  WeaponType = ru_WeaponType;
  Message = ru_Message;
  TeamTurnString = ru_TeamTurnString;
  pAssignMenuStrings = ru_pAssignMenuStrings;
  pTrainingStrings = ru_pTrainingStrings;
  pTrainingMenuStrings = ru_pTrainingMenuStrings;
  pAttributeMenuStrings = ru_pAttributeMenuStrings;
  pVehicleStrings = ru_pVehicleStrings;
  pShortAttributeStrings = ru_pShortAttributeStrings;
  pLongAttributeStrings = ru_pLongAttributeStrings;
  pContractStrings = ru_pContractStrings;
  pAssignmentStrings = ru_pAssignmentStrings;
  pConditionStrings = ru_pConditionStrings;
  pTownNames = ru_pTownNames;
  pPersonnelScreenStrings = ru_pPersonnelScreenStrings;
  pPersonnelTitle = ru_pPersonnelTitle;
  pUpperLeftMapScreenStrings = ru_pUpperLeftMapScreenStrings;
  pTacticalPopupButtonStrings = ru_pTacticalPopupButtonStrings;
  pSquadMenuStrings = ru_pSquadMenuStrings;
  pDoorTrapStrings = ru_pDoorTrapStrings;
  pLongAssignmentStrings = ru_pLongAssignmentStrings;
  pContractExtendStrings = ru_pContractExtendStrings;
  pMapScreenMouseRegionHelpText = ru_pMapScreenMouseRegionHelpText;
  pPersonnelAssignmentStrings = ru_pPersonnelAssignmentStrings;
  pNoiseVolStr = ru_pNoiseVolStr;
  pNoiseTypeStr = ru_pNoiseTypeStr;
  pDirectionStr = ru_pDirectionStr;
  pRemoveMercStrings = ru_pRemoveMercStrings;
  sTimeStrings = ru_sTimeStrings;
  pLandTypeStrings = ru_pLandTypeStrings;
  pGuardMenuStrings = ru_pGuardMenuStrings;
  pOtherGuardMenuStrings = ru_pOtherGuardMenuStrings;
  pInvPanelTitleStrings = ru_pInvPanelTitleStrings;
  pPOWStrings = ru_pPOWStrings;
  pMilitiaString = ru_pMilitiaString;
  pMilitiaButtonString = ru_pMilitiaButtonString;
  pEpcMenuStrings = ru_pEpcMenuStrings;
  pRepairStrings = ru_pRepairStrings;
  sPreStatBuildString = ru_sPreStatBuildString;
  sStatGainStrings = ru_sStatGainStrings;
  pHelicopterEtaStrings = ru_pHelicopterEtaStrings;
  sMapLevelString = ru_sMapLevelString;
  gsLoyalString = ru_gsLoyalString;
  gsUndergroundString = ru_gsUndergroundString;
  gsTimeStrings = ru_gsTimeStrings;
  sFacilitiesStrings = ru_sFacilitiesStrings;
  pMapPopUpInventoryText = ru_pMapPopUpInventoryText;
  pwTownInfoStrings = ru_pwTownInfoStrings;
  pwMineStrings = ru_pwMineStrings;
  pwMiscSectorStrings = ru_pwMiscSectorStrings;
  pMapInventoryErrorString = ru_pMapInventoryErrorString;
  pMapInventoryStrings = ru_pMapInventoryStrings;
  pMapScreenFastHelpTextList = ru_pMapScreenFastHelpTextList;
  pMovementMenuStrings = ru_pMovementMenuStrings;
  pUpdateMercStrings = ru_pUpdateMercStrings;
  pMapScreenBorderButtonHelpText = ru_pMapScreenBorderButtonHelpText;
  pMapScreenBottomFastHelp = ru_pMapScreenBottomFastHelp;
  pMapScreenBottomText = ru_pMapScreenBottomText;
  pMercDeadString = ru_pMercDeadString;
  pSenderNameList = ru_pSenderNameList;
  pTraverseStrings = ru_pTraverseStrings;
  pNewMailStrings = ru_pNewMailStrings;
  pDeleteMailStrings = ru_pDeleteMailStrings;
  pEmailHeaders = ru_pEmailHeaders;
  pEmailTitleText = ru_pEmailTitleText;
  pFinanceTitle = ru_pFinanceTitle;
  pFinanceSummary = ru_pFinanceSummary;
  pFinanceHeaders = ru_pFinanceHeaders;
  pTransactionText = ru_pTransactionText;
  pTransactionAlternateText = ru_pTransactionAlternateText;
  pMoralStrings = ru_pMoralStrings;
  pSkyriderText = ru_pSkyriderText;
  pLeftEquipmentString = ru_pLeftEquipmentString;
  pMapScreenStatusStrings = ru_pMapScreenStatusStrings;
  pMapScreenPrevNextCharButtonHelpText = ru_pMapScreenPrevNextCharButtonHelpText;
  pEtaString = ru_pEtaString;
  pShortVehicleStrings = ru_pShortVehicleStrings;
  pTrashItemText = ru_pTrashItemText;
  pMapErrorString = ru_pMapErrorString;
  pMapPlotStrings = ru_pMapPlotStrings;
  pMiscMapScreenMouseRegionHelpText = ru_pMiscMapScreenMouseRegionHelpText;
  pMercHeLeaveString = ru_pMercHeLeaveString;
  pMercSheLeaveString = ru_pMercSheLeaveString;
  pImpPopUpStrings = ru_pImpPopUpStrings;
  pImpButtonText = ru_pImpButtonText;
  pExtraIMPStrings = ru_pExtraIMPStrings;
  pFilesTitle = ru_pFilesTitle;
  pFilesSenderList = ru_pFilesSenderList;
  pHistoryLocations = ru_pHistoryLocations;
  pHistoryStrings = ru_pHistoryStrings;
  pHistoryHeaders = ru_pHistoryHeaders;
  pHistoryTitle = ru_pHistoryTitle;
  pShowBookmarkString = ru_pShowBookmarkString;
  pWebPagesTitles = ru_pWebPagesTitles;
  pWebTitle = ru_pWebTitle;
  pPersonnelString = ru_pPersonnelString;
  pErrorStrings = ru_pErrorStrings;
  pDownloadString = ru_pDownloadString;
  pBookmarkTitle = ru_pBookmarkTitle;
  pBookMarkStrings = ru_pBookMarkStrings;
  pLaptopIcons = ru_pLaptopIcons;
  sATMText = ru_sATMText;
  gsAtmStartButtonText = ru_gsAtmStartButtonText;
  gsAtmSideButtonText = ru_gsAtmSideButtonText;
  pPersonnelTeamStatsStrings = ru_pPersonnelTeamStatsStrings;
  pPersonnelCurrentTeamStatsStrings = ru_pPersonnelCurrentTeamStatsStrings;
  pPersonelTeamStrings = ru_pPersonelTeamStrings;
  pPersonnelDepartedStateStrings = ru_pPersonnelDepartedStateStrings;
  pMapHortIndex = ru_pMapHortIndex;
  pMapVertIndex = ru_pMapVertIndex;
  pMapDepthIndex = ru_pMapDepthIndex;
  pLaptopTitles = ru_pLaptopTitles;
  pDayStrings = ru_pDayStrings;
  pMercContractOverStrings = ru_pMercContractOverStrings;
  pMilitiaConfirmStrings = ru_pMilitiaConfirmStrings;
  pDeliveryLocationStrings = ru_pDeliveryLocationStrings;
  pSkillAtZeroWarning = ru_pSkillAtZeroWarning;
  pIMPBeginScreenStrings = ru_pIMPBeginScreenStrings;
  pIMPFinishButtonText = ru_pIMPFinishButtonText;
  pIMPFinishStrings = ru_pIMPFinishStrings;
  pIMPVoicesStrings = ru_pIMPVoicesStrings;
  pDepartedMercPortraitStrings = ru_pDepartedMercPortraitStrings;
  pPersTitleText = ru_pPersTitleText;
  pPausedGameText = ru_pPausedGameText;
  zOptionsToggleText = ru_zOptionsToggleText;
  zOptionsScreenHelpText = ru_zOptionsScreenHelpText;
  pDoctorWarningString = ru_pDoctorWarningString;
  pMilitiaButtonsHelpText = ru_pMilitiaButtonsHelpText;
  pMapScreenJustStartedHelpText = ru_pMapScreenJustStartedHelpText;
  pLandMarkInSectorString = ru_pLandMarkInSectorString;
  gzMercSkillText = ru_gzMercSkillText;
  gzNonPersistantPBIText = ru_gzNonPersistantPBIText;
  gzMiscString = ru_gzMiscString;
  wMapScreenSortButtonHelpText = ru_wMapScreenSortButtonHelpText;
  pNewNoiseStr = ru_pNewNoiseStr;
  gzLateLocalizedString = ru_gzLateLocalizedString;
  pAntiHackerString = ru_pAntiHackerString;
  pMessageStrings = ru_pMessageStrings;
  ItemPickupHelpPopup = ru_ItemPickupHelpPopup;
  TacticalStr = ru_TacticalStr;
  LargeTacticalStr = ru_LargeTacticalStr;
  zDialogActions = ru_zDialogActions;
  zDealerStrings = ru_zDealerStrings;
  zTalkMenuStrings = ru_zTalkMenuStrings;
  gzMoneyAmounts = ru_gzMoneyAmounts;
  gMoneyStatsDesc = ru_gMoneyStatsDesc;
  gWeaponStatsDesc = ru_gWeaponStatsDesc;
  sKeyDescriptionStrings = ru_sKeyDescriptionStrings;
  zHealthStr = ru_zHealthStr;
  zVehicleName = ru_zVehicleName;
  pExitingSectorHelpText = ru_pExitingSectorHelpText;
  InsContractText = ru_InsContractText;
  InsInfoText = ru_InsInfoText;
  MercAccountText = ru_MercAccountText;
  MercInfo = ru_MercInfo;
  MercNoAccountText = ru_MercNoAccountText;
  MercHomePageText = ru_MercHomePageText;
  sFuneralString = ru_sFuneralString;
  sFloristText = ru_sFloristText;
  sOrderFormText = ru_sOrderFormText;
  sFloristGalleryText = ru_sFloristGalleryText;
  sFloristCards = ru_sFloristCards;
  BobbyROrderFormText = ru_BobbyROrderFormText;
  BobbyRText = ru_BobbyRText;
  BobbyRaysFrontText = ru_BobbyRaysFrontText;
  AimSortText = ru_AimSortText;
  AimPolicyText = ru_AimPolicyText;
  AimMemberText = ru_AimMemberText;
  CharacterInfo = ru_CharacterInfo;
  VideoConfercingText = ru_VideoConfercingText;
  AimPopUpText = ru_AimPopUpText;
  AimLinkText = ru_AimLinkText;
  AimHistoryText = ru_AimHistoryText;
  AimFiText = ru_AimFiText;
  AimAlumniText = ru_AimAlumniText;
  AimScreenText = ru_AimScreenText;
  AimBottomMenuText = ru_AimBottomMenuText;
  zMarksMapScreenText = ru_zMarksMapScreenText;
  gpStrategicString = ru_gpStrategicString;
  gpGameClockString = ru_gpGameClockString;
  SKI_Text = ru_SKI_Text;
  SkiAtmText = ru_SkiAtmText;
  gzSkiAtmText = ru_gzSkiAtmText;
  SkiMessageBoxText = ru_SkiMessageBoxText;
  zSaveLoadText = ru_zSaveLoadText;
  zOptionsText = ru_zOptionsText;
  gzGIOScreenText = ru_gzGIOScreenText;
  gzHelpScreenText = ru_gzHelpScreenText;
  gzLaptopHelpText = ru_gzLaptopHelpText;
  gzMoneyWithdrawMessageText = ru_gzMoneyWithdrawMessageText;
  gzCopyrightText = ru_gzCopyrightText;
  BrokenLinkText = ru_BrokenLinkText;
  gzBobbyRShipmentText = ru_gzBobbyRShipmentText;
  zGioDifConfirmText = ru_zGioDifConfirmText;
  gzCreditNames = ru_gzCreditNames;
  gzCreditNameTitle = ru_gzCreditNameTitle;
  gzCreditNameFunny = ru_gzCreditNameFunny;
  pUpdatePanelButtons = ru_pUpdatePanelButtons;
  pBullseyeStrings = ru_pBullseyeStrings;
  pContractButtonString = ru_pContractButtonString;
}
