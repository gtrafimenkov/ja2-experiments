#include "TranslationTable.h"

#include "GameRes.h"

static uint8_t const TranslationTableStd[TranslationTableSize] = {
    [L'A'] = 0,
    [L'B'] = 1,
    [L'C'] = 2,
    [L'D'] = 3,
    [L'E'] = 4,
    [L'F'] = 5,
    [L'G'] = 6,
    [L'H'] = 7,
    [L'I'] = 8,
    [L'J'] = 9,
    [L'K'] = 10,
    [L'L'] = 11,
    [L'M'] = 12,
    [L'N'] = 13,
    [L'O'] = 14,
    [L'P'] = 15,
    [L'Q'] = 16,
    [L'R'] = 17,
    [L'S'] = 18,
    [L'T'] = 19,
    [L'U'] = 20,
    [L'V'] = 21,
    [L'W'] = 22,
    [L'X'] = 23,
    [L'Y'] = 24,
    [L'Z'] = 25,

    [L'a'] = 26,
    [L'b'] = 27,
    [L'c'] = 28,
    [L'd'] = 29,
    [L'e'] = 30,
    [L'f'] = 31,
    [L'g'] = 32,
    [L'h'] = 33,
    [L'i'] = 34,
    [L'j'] = 35,
    [L'k'] = 36,
    [L'l'] = 37,
    [L'm'] = 38,
    [L'n'] = 39,
    [L'o'] = 40,
    [L'p'] = 41,
    [L'q'] = 42,
    [L'r'] = 43,
    [L's'] = 44,
    [L't'] = 45,
    [L'u'] = 46,
    [L'v'] = 47,
    [L'w'] = 48,
    [L'x'] = 49,
    [L'y'] = 50,
    [L'z'] = 51,

    [L'0'] = 52,
    [L'1'] = 53,
    [L'2'] = 54,
    [L'3'] = 55,
    [L'4'] = 56,
    [L'5'] = 57,
    [L'6'] = 58,
    [L'7'] = 59,
    [L'8'] = 60,
    [L'9'] = 61,

    [L'!'] = 62,
    [L'@'] = 63,
    [L'#'] = 64,
    [L'$'] = 65,
    [L'%'] = 66,
    [L'^'] = 67,
    [L'&'] = 68,
    [L'*'] = 69,
    [L'('] = 70,
    [L')'] = 71,
    [L'-'] = 72,
    [L'_'] = 73,
    [L'+'] = 74,
    [L'='] = 75,
    [L'|'] = 76,
    [L'\\'] = 77,
    [L'{'] = 78,
    [L'}'] = 79,
    [L'['] = 80,
    [L']'] = 81,
    [L':'] = 82,
    [L';'] = 83,
    [L'"'] = 84,
    [L'\''] = 85,
    [L'<'] = 86,
    [L'>'] = 87,
    [L','] = 88,
    [L'.'] = 89,
    [L'?'] = 90,
    [L'/'] = 91,
    [L' '] = 92,
    [196] = 93,  // A umlaut
    [214] = 94,  // O umlaut
    [220] = 95,  // U umlaut
    [228] = 96,  // a umlaut
    [246] = 97,  // o umlaut
    [252] = 98,  // u umlaut
    [223] = 99,  // sharp s

    // duplicate 196, // Ä
    [192] = 133,  // À
    [193] = 134,  // Á
    [194] = 135,  // Â
    [199] = 136,  // Ç
    [203] = 137,  // Ë
    [200] = 138,  // È
    [201] = 139,  // É				140
    [202] = 140,  // Ê
    [207] = 141,  // Ï
    // duplicate 214, // Ö
    [210] = 143,  // Ò
    [211] = 144,  // Ó
    [212] = 145,  // Ô
    // duplicate 220, // Ü
    [217] = 147,  // Ù
    [218] = 148,  // Ú
    [219] = 149,  // Û				150
    // duplicate 228, // ä
    [224] = 151,  // à
    [225] = 152,  // á
    [226] = 153,  // â
    [231] = 154,  // ç
    [235] = 155,  // ë
    [232] = 156,  // è
    [233] = 157,  // é
    [234] = 158,  // ê
    [239] = 159,  // ï				160
    // duplicate 246, // ö
    [242] = 161,  // ò
    [243] = 162,  // ó
    [244] = 163,  // ô
    // duplicate 252, // ü
    [249] = 165,  // ù
    [250] = 166,  // ú
    [251] = 167,  // û

    [0x00CC] = 168,  // Ì
    [0x00EC] = 169,  // ì
    [0x0104] = 170,  // Ą
    [0x0106] = 171,  // Ć
    [0x0118] = 172,  // Ę
    [0x0141] = 173,  // Ł
    [0x0143] = 174,  // Ń
    //[0x00D3] = 175, // Ó (duplicate)
    [0x015A] = 176,  // Ś
    [0x017B] = 177,  // Ż
    [0x0179] = 178,  // Ź
    [0x0105] = 179,  // ą
    [0x0107] = 180,  // ć
    [0x0119] = 181,  // ę
    [0x0142] = 182,  // ł
    [0x0144] = 183,  // ń
    //[0x00F3] = 184, // ó (duplicate)
    [0x015B] = 185,  // ś
    [0x017C] = 186,  // ż
    [0x017A] = 187,  // ź

    [0x0410] = 100,  // cyrillic A
    [0x0411] = 101,  // cyrillic BE
    [0x0412] = 102,  // cyrillic VE
    [0x0413] = 103,  // cyrillic GHE
    [0x0414] = 104,  // cyrillic DE
    [0x0415] = 105,  // cyrillic IE
    [0x0401] = 106,  // cyrillic IO
    [0x0416] = 107,  // cyrillic ZHE
    [0x0417] = 108,  // cyrillic ZE
    [0x0418] = 109,  // cyrillic I
    [0x0419] = 110,  // cyrillic short I
    [0x041A] = 111,  // cyrillic KA
    [0x041B] = 112,  // cyrillic EL
    [0x041C] = 113,  // cyrillic EM
    [0x041D] = 114,  // cyrillic EN
    [0x041E] = 115,  // cyrillic O
    [0x041F] = 116,  // cyrillic PE
    [0x0420] = 117,  // cyrillic ER
    [0x0421] = 118,  // cyrillic ES
    [0x0422] = 119,  // cyrillic TE
    [0x0423] = 120,  // cyrillic U
    [0x0424] = 121,  // cyrillic EF
    [0x0425] = 122,  // cyrillic HA
    [0x0426] = 123,  // cyrillic TSE
    [0x0427] = 124,  // cyrillic CHE
    [0x0428] = 125,  // cyrillic SHA
    [0x0429] = 126,  // cyrillic SHCHA
    [0x042A] = 128,  // cyrillic capital hard sign, glyph is missing, mapped to soft sign
    [0x042B] = 127,  // cyrillic YERU
    [0x042C] = 128,  // cyrillic capital soft sign
    [0x042D] = 129,  // cyrillic E
    [0x042E] = 130,  // cyrillic YU
    [0x042F] = 131,  // cyrillic YA

    /* There are no lowercase cyrillic glyphs in the fonts (at least neither in
     * the german nor the polish datafiles), so reuse the uppercase cyrillic
     * glyphs. */
    [0x0430] = 100,  // cyrillic a
    [0x0431] = 101,  // cyrillic be
    [0x0432] = 102,  // cyrillic ve
    [0x0433] = 103,  // cyrillic ghe
    [0x0434] = 104,  // cyrillic de
    [0x0435] = 105,  // cyrillic ie
    [0x0451] = 106,  // cyrillic io
    [0x0436] = 107,  // cyrillic zhe
    [0x0437] = 108,  // cyrillic ze
    [0x0438] = 109,  // cyrillic i
    [0x0439] = 110,  // cyrillic short i
    [0x043A] = 111,  // cyrillic ka
    [0x043B] = 112,  // cyrillic el
    [0x043C] = 113,  // cyrillic em
    [0x043D] = 114,  // cyrillic en
    [0x043E] = 115,  // cyrillic o
    [0x043F] = 116,  // cyrillic pe
    [0x0440] = 117,  // cyrillic er
    [0x0441] = 118,  // cyrillic es
    [0x0442] = 119,  // cyrillic te
    [0x0443] = 120,  // cyrillic u
    [0x0444] = 121,  // cyrillic ef
    [0x0445] = 122,  // cyrillic ha
    [0x0446] = 123,  // cyrillic tse
    [0x0447] = 124,  // cyrillic che
    [0x0448] = 125,  // cyrillic sha
    [0x0449] = 126,  // cyrillic shcha
    [0x044A] = 128,  // cyrillic lowercase hard sign, glyph is missing, mapped to soft sign
    [0x044B] = 127,  // cyrillic yeru
    [0x044C] = 128,  // cyrillic lowercase soft sign
    [0x044D] = 129,  // cyrillic e
    [0x044E] = 130,  // cyrillic yu
    [0x044F] = 131   // cyrillic ya
};

uint8_t const TranslationTableFrench[TranslationTableSize] = {
    [L'A'] = 0,
    [L'B'] = 1,
    [L'C'] = 2,
    [L'D'] = 3,
    [L'E'] = 4,
    [L'F'] = 5,
    [L'G'] = 6,
    [L'H'] = 7,
    [L'I'] = 8,
    [L'J'] = 9,
    [L'K'] = 10,
    [L'L'] = 11,
    [L'M'] = 12,
    [L'N'] = 13,
    [L'O'] = 14,
    [L'P'] = 15,
    [L'Q'] = 16,
    [L'R'] = 17,
    [L'S'] = 18,
    [L'T'] = 19,
    [L'U'] = 20,
    [L'V'] = 21,
    [L'W'] = 22,
    [L'X'] = 23,
    [L'Y'] = 24,
    [L'Z'] = 25,

    [L'a'] = 26,
    [L'b'] = 27,
    [L'c'] = 28,
    [L'd'] = 29,
    [L'e'] = 30,
    [L'f'] = 31,
    [L'g'] = 32,
    [L'h'] = 33,
    [L'i'] = 34,
    [L'j'] = 35,
    [L'k'] = 36,
    [L'l'] = 37,
    [L'm'] = 38,
    [L'n'] = 39,
    [L'o'] = 40,
    [L'p'] = 41,
    [L'q'] = 42,
    [L'r'] = 43,
    [L's'] = 44,
    [L't'] = 45,
    [L'u'] = 46,
    [L'v'] = 47,
    [L'w'] = 48,
    [L'x'] = 49,
    [L'y'] = 50,
    [L'z'] = 51,

    [L'0'] = 52,
    [L'1'] = 53,
    [L'2'] = 54,
    [L'3'] = 55,
    [L'4'] = 56,
    [L'5'] = 57,
    [L'6'] = 58,
    [L'7'] = 59,
    [L'8'] = 60,
    [L'9'] = 61,

    [L'!'] = 62,
    [L'@'] = 63,
    [L'#'] = 64,
    [L'$'] = 65,
    [L'%'] = 66,
    [L'^'] = 67,
    [L'&'] = 68,
    [L'*'] = 69,
    [L'('] = 70,
    [L')'] = 71,
    [L'-'] = 72,
    [L'_'] = 73,
    [L'+'] = 74,
    [L'='] = 75,
    [L'|'] = 76,
    [L'\\'] = 77,
    [L'{'] = 78,
    [L'}'] = 79,
    [L'['] = 80,
    [L']'] = 81,
    [L':'] = 82,
    [L';'] = 83,
    [L'"'] = 84,
    [L'\''] = 85,
    [L'<'] = 86,
    [L'>'] = 87,
    [L','] = 88,
    [L'.'] = 89,
    [L'?'] = 90,
    [L'/'] = 91,
    [L' '] = 92,
    [196] = 93,  // A umlaut
    [214] = 94,  // O umlaut
    [220] = 95,  // U umlaut
    [228] = 96,  // a umlaut
    [246] = 97,  // o umlaut
    [252] = 98,  // u umlaut
    [223] = 99,  // sharp s

    // duplicate 196, // Ä
    [192] = 133,  // À
    [193] = 134,  // Á
    [194] = 135,  // Â
    [199] = 136,  // Ç
    [203] = 137,  // Ë
    [200] = 138,  // È
    [201] = 139,  // É				140
    [202] = 140,  // Ê
    [207] = 141,  // Ï
    // duplicate 214, // Ö
    [210] = 143,  // Ò
    [211] = 144,  // Ó
    [212] = 145,  // Ô
    // duplicate 220, // Ü
    [217] = 147,  // Ù
    [218] = 148,  // Ú
    [219] = 149,  // Û				150
    // duplicate 228, // ä
    [224] = 151,  // à
    [225] = 152,  // á
    [226] = 153,  // â
    [231] = 154,  // ç
    [235] = 155,  // ë
    [232] = 156,  // è
    [233] = 157,  // é
    [234] = 158,  // ê
    [239] = 159,  // ï				160
    // duplicate 246, // ö
    [242] = 161,  // ò
    [243] = 162,  // ó
    [244] = 163,  // ô
    // duplicate 252, // ü
    [249] = 165,  // ù
    [250] = 166,  // ú
    [251] = 167,  // û

    //////////////////////////////////////////////////////////////
    // french specific
    //////////////////////////////////////////////////////////////

    [0x00CC] = 168,  // Ì
    [0x00CE] = 169,  // Î
    [0x00EC] = 170,  // ì
    [0x00EE] = 171,  // î
    [0x0104] = 172,  // Ą
    [0x0106] = 173,  // Ć
    [0x0118] = 174,  // Ę
    [0x0141] = 175,  // Ł
    [0x0143] = 176,  // Ń
    //[0x00D3] = 177, // Ó (duplicate)
    [0x015A] = 178,  // Ś
    [0x017B] = 179,  // Ż
    [0x0179] = 180,  // Ź
    [0x0105] = 181,  // ą
    [0x0107] = 182,  // ć
    [0x0119] = 183,  // ę
    [0x0142] = 184,  // ł
    [0x0144] = 185,  // ń
    //[0x00F3] = 186, // ó (duplicate)
    [0x015B] = 187,  // ś
    [0x017C] = 188,  // ż
    [0x017A] = 189,  // ź

    //////////////////////////////////////////////////////////////
    // end of french specific
    //////////////////////////////////////////////////////////////

    [0x0410] = 100,  // cyrillic A
    [0x0411] = 101,  // cyrillic BE
    [0x0412] = 102,  // cyrillic VE
    [0x0413] = 103,  // cyrillic GHE
    [0x0414] = 104,  // cyrillic DE
    [0x0415] = 105,  // cyrillic IE
    [0x0401] = 106,  // cyrillic IO
    [0x0416] = 107,  // cyrillic ZHE
    [0x0417] = 108,  // cyrillic ZE
    [0x0418] = 109,  // cyrillic I
    [0x0419] = 110,  // cyrillic short I
    [0x041A] = 111,  // cyrillic KA
    [0x041B] = 112,  // cyrillic EL
    [0x041C] = 113,  // cyrillic EM
    [0x041D] = 114,  // cyrillic EN
    [0x041E] = 115,  // cyrillic O
    [0x041F] = 116,  // cyrillic PE
    [0x0420] = 117,  // cyrillic ER
    [0x0421] = 118,  // cyrillic ES
    [0x0422] = 119,  // cyrillic TE
    [0x0423] = 120,  // cyrillic U
    [0x0424] = 121,  // cyrillic EF
    [0x0425] = 122,  // cyrillic HA
    [0x0426] = 123,  // cyrillic TSE
    [0x0427] = 124,  // cyrillic CHE
    [0x0428] = 125,  // cyrillic SHA
    [0x0429] = 126,  // cyrillic SHCHA
    [0x042A] = 128,  // cyrillic capital hard sign, glyph is missing, mapped to soft sign
    [0x042B] = 127,  // cyrillic YERU
    [0x042C] = 128,  // cyrillic capital soft sign
    [0x042D] = 129,  // cyrillic E
    [0x042E] = 130,  // cyrillic YU
    [0x042F] = 131,  // cyrillic YA

    /* There are no lowercase cyrillic glyphs in the fonts (at least neither in
     * the german nor the polish datafiles), so reuse the uppercase cyrillic
     * glyphs. */
    [0x0430] = 100,  // cyrillic a
    [0x0431] = 101,  // cyrillic be
    [0x0432] = 102,  // cyrillic ve
    [0x0433] = 103,  // cyrillic ghe
    [0x0434] = 104,  // cyrillic de
    [0x0435] = 105,  // cyrillic ie
    [0x0451] = 106,  // cyrillic io
    [0x0436] = 107,  // cyrillic zhe
    [0x0437] = 108,  // cyrillic ze
    [0x0438] = 109,  // cyrillic i
    [0x0439] = 110,  // cyrillic short i
    [0x043A] = 111,  // cyrillic ka
    [0x043B] = 112,  // cyrillic el
    [0x043C] = 113,  // cyrillic em
    [0x043D] = 114,  // cyrillic en
    [0x043E] = 115,  // cyrillic o
    [0x043F] = 116,  // cyrillic pe
    [0x0440] = 117,  // cyrillic er
    [0x0441] = 118,  // cyrillic es
    [0x0442] = 119,  // cyrillic te
    [0x0443] = 120,  // cyrillic u
    [0x0444] = 121,  // cyrillic ef
    [0x0445] = 122,  // cyrillic ha
    [0x0446] = 123,  // cyrillic tse
    [0x0447] = 124,  // cyrillic che
    [0x0448] = 125,  // cyrillic sha
    [0x0449] = 126,  // cyrillic shcha
    [0x044A] = 128,  // cyrillic lowercase hard sign, glyph is missing, mapped to soft sign
    [0x044B] = 127,  // cyrillic yeru
    [0x044C] = 128,  // cyrillic lowercase soft sign
    [0x044D] = 129,  // cyrillic e
    [0x044E] = 130,  // cyrillic yu
    [0x044F] = 131   // cyrillic ya
};

uint8_t const TranslationTableRuBuka[TranslationTableSize] = {
    /* This is the table for the BUKA Agonia Vlasti release. The glyph set is,
     * except for two gaps (0x00-0x1F, 0x80-0xBF), identical to CP1251. */
    [L' '] = 0,     [L'!'] = 1,  [L'"'] = 2,  [L'#'] = 3,  [L'$'] = 4,   [L'%'] = 5,  [L'&'] = 6,
    [L'\''] = 7,    [L'('] = 8,  [L')'] = 9,  [L'*'] = 10, [L'+'] = 11,  [L','] = 12, [L'-'] = 13,
    [L'.'] = 14,    [L'/'] = 15, [L'0'] = 16, [L'1'] = 17, [L'2'] = 18,  [L'3'] = 19, [L'4'] = 20,
    [L'5'] = 21,    [L'6'] = 22, [L'7'] = 23, [L'8'] = 24, [L'9'] = 25,  [L':'] = 26, [L';'] = 27,
    [L'<'] = 28,    [L'='] = 29, [L'>'] = 30, [L'?'] = 31, [L'@'] = 32,  [L'A'] = 33, [L'B'] = 34,
    [L'C'] = 35,    [L'D'] = 36, [L'E'] = 37, [L'F'] = 38, [L'G'] = 39,  [L'H'] = 40, [L'I'] = 41,
    [L'J'] = 42,    [L'K'] = 43, [L'L'] = 44, [L'M'] = 45, [L'N'] = 46,  [L'O'] = 47, [L'P'] = 48,
    [L'Q'] = 49,    [L'R'] = 50, [L'S'] = 51, [L'T'] = 52, [L'U'] = 53,  [L'V'] = 54, [L'W'] = 55,
    [L'X'] = 56,    [L'Y'] = 57, [L'Z'] = 58, [L'['] = 59, [L'\\'] = 60, [L']'] = 61, [L'^'] = 62,
    [L'_'] = 63,    [L'`'] = 64, [L'a'] = 65, [L'b'] = 66, [L'c'] = 67,  [L'd'] = 68, [L'e'] = 69,
    [L'f'] = 70,    [L'g'] = 71, [L'h'] = 72, [L'i'] = 73, [L'j'] = 74,  [L'k'] = 75, [L'l'] = 76,
    [L'm'] = 77,    [L'n'] = 78, [L'o'] = 79, [L'p'] = 80, [L'q'] = 81,  [L'r'] = 82, [L's'] = 83,
    [L't'] = 84,    [L'u'] = 85, [L'v'] = 86, [L'w'] = 87, [L'x'] = 88,  [L'y'] = 89, [L'z'] = 90,
    [L'{'] = 91,    [L'|'] = 92, [L'}'] = 93, [L'~'] = 94,
    [0x007F] = 95,  // DELETE

    [0x0410] = 96,   // CYRILLIC CAPITAL LETTER A
    [0x0411] = 97,   // CYRILLIC CAPITAL LETTER BE
    [0x0412] = 98,   // CYRILLIC CAPITAL LETTER VE
    [0x0413] = 99,   // CYRILLIC CAPITAL LETTER GHE
    [0x0414] = 100,  // CYRILLIC CAPITAL LETTER DE
    [0x0415] = 101,  // CYRILLIC CAPITAL LETTER IE
    [0x0416] = 102,  // CYRILLIC CAPITAL LETTER ZHE
    [0x0417] = 103,  // CYRILLIC CAPITAL LETTER ZE
    [0x0418] = 104,  // CYRILLIC CAPITAL LETTER I
    [0x0419] = 105,  // CYRILLIC CAPITAL LETTER SHORT I
    [0x041A] = 106,  // CYRILLIC CAPITAL LETTER KA
    [0x041B] = 107,  // CYRILLIC CAPITAL LETTER EL
    [0x041C] = 108,  // CYRILLIC CAPITAL LETTER EM
    [0x041D] = 109,  // CYRILLIC CAPITAL LETTER EN
    [0x041E] = 110,  // CYRILLIC CAPITAL LETTER O
    [0x041F] = 111,  // CYRILLIC CAPITAL LETTER PE
    [0x0420] = 112,  // CYRILLIC CAPITAL LETTER ER
    [0x0421] = 113,  // CYRILLIC CAPITAL LETTER ES
    [0x0422] = 114,  // CYRILLIC CAPITAL LETTER TE
    [0x0423] = 115,  // CYRILLIC CAPITAL LETTER U
    [0x0424] = 116,  // CYRILLIC CAPITAL LETTER EF
    [0x0425] = 117,  // CYRILLIC CAPITAL LETTER HA
    [0x0426] = 118,  // CYRILLIC CAPITAL LETTER TSE
    [0x0427] = 119,  // CYRILLIC CAPITAL LETTER CHE
    [0x0428] = 120,  // CYRILLIC CAPITAL LETTER SHA
    [0x0429] = 121,  // CYRILLIC CAPITAL LETTER SHCHA
    [0x042A] = 122,  // CYRILLIC CAPITAL LETTER HARD SIGN
    [0x042B] = 123,  // CYRILLIC CAPITAL LETTER YERU
    [0x042C] = 124,  // CYRILLIC CAPITAL LETTER SOFT SIGN
    [0x042D] = 125,  // CYRILLIC CAPITAL LETTER E
    [0x042E] = 126,  // CYRILLIC CAPITAL LETTER YU
    [0x042F] = 127,  // CYRILLIC CAPITAL LETTER YA
    [0x0430] = 128,  // CYRILLIC SMALL LETTER A
    [0x0431] = 129,  // CYRILLIC SMALL LETTER BE
    [0x0432] = 130,  // CYRILLIC SMALL LETTER VE
    [0x0433] = 131,  // CYRILLIC SMALL LETTER GHE
    [0x0434] = 132,  // CYRILLIC SMALL LETTER DE
    [0x0435] = 133,  // CYRILLIC SMALL LETTER IE
    [0x0436] = 134,  // CYRILLIC SMALL LETTER ZHE
    [0x0437] = 135,  // CYRILLIC SMALL LETTER ZE
    [0x0438] = 136,  // CYRILLIC SMALL LETTER I
    [0x0439] = 137,  // CYRILLIC SMALL LETTER SHORT I
    [0x043A] = 138,  // CYRILLIC SMALL LETTER KA
    [0x043B] = 139,  // CYRILLIC SMALL LETTER EL
    [0x043C] = 140,  // CYRILLIC SMALL LETTER EM
    [0x043D] = 141,  // CYRILLIC SMALL LETTER EN
    [0x043E] = 142,  // CYRILLIC SMALL LETTER O
    [0x043F] = 143,  // CYRILLIC SMALL LETTER PE
    [0x0440] = 144,  // CYRILLIC SMALL LETTER ER
    [0x0441] = 145,  // CYRILLIC SMALL LETTER ES
    [0x0442] = 146,  // CYRILLIC SMALL LETTER TE
    [0x0443] = 147,  // CYRILLIC SMALL LETTER U
    [0x0444] = 148,  // CYRILLIC SMALL LETTER EF
    [0x0445] = 149,  // CYRILLIC SMALL LETTER HA
    [0x0446] = 150,  // CYRILLIC SMALL LETTER TSE
    [0x0447] = 151,  // CYRILLIC SMALL LETTER CHE
    [0x0448] = 152,  // CYRILLIC SMALL LETTER SHA
    [0x0449] = 153,  // CYRILLIC SMALL LETTER SHCHA
    [0x044A] = 154,  // CYRILLIC SMALL LETTER HARD SIGN
    [0x044B] = 155,  // CYRILLIC SMALL LETTER YERU
    [0x044C] = 156,  // CYRILLIC SMALL LETTER SOFT SIGN
    [0x044D] = 157,  // CYRILLIC SMALL LETTER E
    [0x044E] = 158,  // CYRILLIC SMALL LETTER YU
    [0x044F] = 159   // CYRILLIC SMALL LETTER YA
};

uint8_t const TranslationTableRuGold[TranslationTableSize] = {
    [L'A'] = 0,
    [L'B'] = 1,
    [L'C'] = 2,
    [L'D'] = 3,
    [L'E'] = 4,
    [L'F'] = 5,
    [L'G'] = 6,
    [L'H'] = 7,
    [L'I'] = 8,
    [L'J'] = 9,
    [L'K'] = 10,
    [L'L'] = 11,
    [L'M'] = 12,
    [L'N'] = 13,
    [L'O'] = 14,
    [L'P'] = 15,
    [L'Q'] = 16,
    [L'R'] = 17,
    [L'S'] = 18,
    [L'T'] = 19,
    [L'U'] = 20,
    [L'V'] = 21,
    [L'W'] = 22,
    [L'X'] = 23,
    [L'Y'] = 24,
    [L'Z'] = 25,

    [L'a'] = 26,
    [L'b'] = 27,
    [L'c'] = 28,
    [L'd'] = 29,
    [L'e'] = 30,
    [L'f'] = 31,
    [L'g'] = 32,
    [L'h'] = 33,
    [L'i'] = 34,
    [L'j'] = 35,
    [L'k'] = 36,
    [L'l'] = 37,
    [L'm'] = 38,
    [L'n'] = 39,
    [L'o'] = 40,
    [L'p'] = 41,
    [L'q'] = 42,
    [L'r'] = 43,
    [L's'] = 44,
    [L't'] = 45,
    [L'u'] = 46,
    [L'v'] = 47,
    [L'w'] = 48,
    [L'x'] = 49,
    [L'y'] = 50,
    [L'z'] = 51,

    [L'0'] = 52,
    [L'1'] = 53,
    [L'2'] = 54,
    [L'3'] = 55,
    [L'4'] = 56,
    [L'5'] = 57,
    [L'6'] = 58,
    [L'7'] = 59,
    [L'8'] = 60,
    [L'9'] = 61,

    [L'!'] = 62,
    [L'@'] = 63,
    [L'#'] = 64,
    [L'$'] = 65,
    [L'%'] = 66,
    [L'^'] = 67,
    [L'&'] = 68,
    [L'*'] = 69,
    [L'('] = 70,
    [L')'] = 71,
    [L'-'] = 72,
    [L'_'] = 73,
    [L'+'] = 74,
    [L'='] = 75,
    [L'|'] = 76,
    [L'\\'] = 77,
    [L'{'] = 78,
    [L'}'] = 79,
    [L'['] = 80,
    [L']'] = 81,
    [L':'] = 82,
    [L';'] = 83,
    [L'"'] = 84,
    [L'\''] = 85,
    [L'<'] = 86,
    [L'>'] = 87,
    [L','] = 88,
    [L'.'] = 89,
    [L'?'] = 90,
    [L'/'] = 91,
    [L' '] = 92,
    [196] = 93,  // A umlaut
    [214] = 94,  // O umlaut
    [220] = 95,  // U umlaut
    [228] = 96,  // a umlaut
    [246] = 97,  // o umlaut
    [252] = 98,  // u umlaut
    [223] = 99,  // sharp s

    // duplicate 196, // Ä
    [192] = 165,  // À
    [193] = 166,  // Á
    [194] = 167,  // Â
    [199] = 168,  // Ç
    [203] = 169,  // Ë
    [200] = 170,  // È
    [201] = 171,  // É
    [202] = 172,  // Ê
    [207] = 173,  // Ï
    // duplicate 214, // Ö
    [210] = 175,  // Ò
    [211] = 176,  // Ó
    [212] = 177,  // Ô
    // duplicate 220, // Ü
    [217] = 179,  // Ù
    [218] = 180,  // Ú
    [219] = 181,  // Û

    // missing lowercase glyphs, reuse uppercase from above
    // duplicate 228, // ä
    [224] = 165,  // à
    [225] = 166,  // á
    [226] = 167,  // â
    [231] = 168,  // ç
    [235] = 169,  // ë
    [232] = 170,  // è
    [233] = 171,  // é
    [234] = 172,  // ê
    [239] = 173,  // ï
    // duplicate 246, // ö
    [242] = 175,  // ò
    [243] = 176,  // ó
    [244] = 177,  // ô
    // duplicate 252, // ü
    [249] = 179,  // ù
    [250] = 180,  // ú
    [251] = 181,  // û

    [0x00CC] = 182,  // I with grave
    [0x00EC] = 183,  // i with grave
    [0x0104] = 184,  // A with ogonek
    [0x0106] = 185,  // C with acute
    [0x0118] = 186,  // E with ogonek
    [0x0141] = 187,  // L with stroke
    [0x0143] = 188,  // N with acute
    [0x00D3] = 189,  // O with acute (duplicate)
    [0x015A] = 190,  // S with acute
    [0x017B] = 191,  // Z with dot above
    [0x0179] = 192,  // Z with acute
    [0x0105] = 193,  // a with ogonek
    [0x0107] = 194,  // c with acute
    [0x0119] = 195,  // e with ogonek
    [0x0142] = 196,  // l with stroke
    [0x0144] = 197,  // n with acute
    [0x00F3] = 198,  // o with acute (duplicate)
    [0x015B] = 199,  // s with acute
    [0x017C] = 200,  // z with dot above
    [0x017A] = 201,  // z with acute

    [0x0410] = 100,  // CYRILLIC CAPITAL LETTER A
    [0x0411] = 101,  // CYRILLIC CAPITAL LETTER BE
    [0x0412] = 102,  // CYRILLIC CAPITAL LETTER VE
    [0x0413] = 103,  // CYRILLIC CAPITAL LETTER GHE
    [0x0414] = 104,  // CYRILLIC CAPITAL LETTER DE
    [0x0415] = 105,  // CYRILLIC CAPITAL LETTER IE
    [0x0416] = 106,  // CYRILLIC CAPITAL LETTER ZHE
    [0x0417] = 107,  // CYRILLIC CAPITAL LETTER ZE
    [0x0418] = 108,  // CYRILLIC CAPITAL LETTER I
    [0x0419] = 109,  // CYRILLIC CAPITAL LETTER SHORT I
    [0x041A] = 110,  // CYRILLIC CAPITAL LETTER KA
    [0x041B] = 111,  // CYRILLIC CAPITAL LETTER EL
    [0x041C] = 112,  // CYRILLIC CAPITAL LETTER EM
    [0x041D] = 113,  // CYRILLIC CAPITAL LETTER EN
    [0x041E] = 114,  // CYRILLIC CAPITAL LETTER O
    [0x041F] = 115,  // CYRILLIC CAPITAL LETTER PE
    [0x0420] = 116,  // CYRILLIC CAPITAL LETTER ER
    [0x0421] = 117,  // CYRILLIC CAPITAL LETTER ES
    [0x0422] = 118,  // CYRILLIC CAPITAL LETTER TE
    [0x0423] = 119,  // CYRILLIC CAPITAL LETTER U
    [0x0424] = 120,  // CYRILLIC CAPITAL LETTER EF
    [0x0425] = 121,  // CYRILLIC CAPITAL LETTER HA
    [0x0426] = 122,  // CYRILLIC CAPITAL LETTER TSE
    [0x0427] = 123,  // CYRILLIC CAPITAL LETTER CHE
    [0x0428] = 124,  // CYRILLIC CAPITAL LETTER SHA
    [0x0429] = 125,  // CYRILLIC CAPITAL LETTER SHCHA
    [0x042A] = 126,  // CYRILLIC CAPITAL LETTER HARD SIGN
    [0x042B] = 127,  // CYRILLIC CAPITAL LETTER YERU
    [0x042C] = 128,  // CYRILLIC CAPITAL LETTER SOFT SIGN
    [0x042D] = 129,  // CYRILLIC CAPITAL LETTER E
    [0x042E] = 130,  // CYRILLIC CAPITAL LETTER YU
    [0x042F] = 131,  // CYRILLIC CAPITAL LETTER YA
    [0x0430] = 132,  // CYRILLIC SMALL LETTER A
    [0x0431] = 133,  // CYRILLIC SMALL LETTER BE
    [0x0432] = 134,  // CYRILLIC SMALL LETTER VE
    [0x0433] = 135,  // CYRILLIC SMALL LETTER GHE
    [0x0434] = 136,  // CYRILLIC SMALL LETTER DE
    [0x0435] = 137,  // CYRILLIC SMALL LETTER IE
    [0x0436] = 138,  // CYRILLIC SMALL LETTER ZHE
    [0x0437] = 139,  // CYRILLIC SMALL LETTER ZE
    [0x0438] = 140,  // CYRILLIC SMALL LETTER I
    [0x0439] = 141,  // CYRILLIC SMALL LETTER SHORT I
    [0x043A] = 142,  // CYRILLIC SMALL LETTER KA
    [0x043B] = 143,  // CYRILLIC SMALL LETTER EL
    [0x043C] = 144,  // CYRILLIC SMALL LETTER EM
    [0x043D] = 145,  // CYRILLIC SMALL LETTER EN
    [0x043E] = 146,  // CYRILLIC SMALL LETTER O
    [0x043F] = 147,  // CYRILLIC SMALL LETTER PE
    [0x0440] = 148,  // CYRILLIC SMALL LETTER ER
    [0x0441] = 149,  // CYRILLIC SMALL LETTER ES
    [0x0442] = 150,  // CYRILLIC SMALL LETTER TE
    [0x0443] = 151,  // CYRILLIC SMALL LETTER U
    [0x0444] = 152,  // CYRILLIC SMALL LETTER EF
    [0x0445] = 153,  // CYRILLIC SMALL LETTER HA
    [0x0446] = 154,  // CYRILLIC SMALL LETTER TSE
    [0x0447] = 155,  // CYRILLIC SMALL LETTER CHE
    [0x0448] = 156,  // CYRILLIC SMALL LETTER SHA
    [0x0449] = 157,  // CYRILLIC SMALL LETTER SHCHA
    [0x044A] = 158,  // CYRILLIC SMALL LETTER HARD SIGN
    [0x044B] = 159,  // CYRILLIC SMALL LETTER YERU
    [0x044C] = 160,  // CYRILLIC SMALL LETTER SOFT SIGN
    [0x044D] = 161,  // CYRILLIC SMALL LETTER E
    [0x044E] = 162,  // CYRILLIC SMALL LETTER YU
    [0x044F] = 163   // CYRILLIC SMALL LETTER YA
};

uint8_t const *TranslationTable = TranslationTableStd;

void SelectCorrectTranslationTable() {
  if (UsingFrenchResources()) {
    TranslationTable = TranslationTableFrench;
  } else if (UsingRussianBukaResources()) {
    TranslationTable = TranslationTableRuBuka;
  } else if (UsingRussianGoldResources()) {
    TranslationTable = TranslationTableRuGold;
  } else {
    TranslationTable = TranslationTableStd;
  }
}
