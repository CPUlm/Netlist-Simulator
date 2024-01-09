/* C++ code produced by gperf version 3.1 */
/* Command-line: 'C:\\dev\\vcpkg\\installed\\x64-windows\\tools\\gperf\\gperf.exe' -t --output-file=src/keywords.hpp src/keywords.def  */
/* Computed positions: -k'1-3' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 10 "src/keywords.def"

#include "token.hpp"
#line 14 "src/keywords.def"
struct KeywordInfo { const char* name; TokenKind token_kind; };
#include <string.h>
/* maximum key range = 46, duplicates = 0 */

class KeywordHashTable
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static const struct KeywordInfo *lookup (const char *str, size_t len);
};

inline unsigned int
KeywordHashTable::hash (const char *str, size_t len)
{
  static const unsigned char asso_values[] =
    {
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 25, 49,  0, 15,  0,
      49,  0, 49,  0, 49, 49,  0, 10,  5, 15,
       0, 49,  0,  0, 10,  0, 15, 49,  0, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
      49, 49, 49, 49, 49, 49
    };
  unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[static_cast<unsigned char>(str[2])];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[static_cast<unsigned char>(str[1])];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[static_cast<unsigned char>(str[0])];
        break;
    }
  return hval;
}

const struct KeywordInfo *
KeywordHashTable::lookup (const char *str, size_t len)
{
  enum
    {
      TOTAL_KEYWORDS = 18,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 6,
      MIN_HASH_VALUE = 3,
      MAX_HASH_VALUE = 48
    };

  static const struct KeywordInfo wordlist[] =
    {
      {""}, {""}, {""},
#line 28 "src/keywords.def"
      {"REG",     TokenKind::KEY_REG},
      {""},
#line 31 "src/keywords.def"
      {"SLICE",   TokenKind::KEY_SLICE},
#line 30 "src/keywords.def"
      {"SELECT",  TokenKind::KEY_SELECT},
#line 19 "src/keywords.def"
      {"IN",      TokenKind::KEY_IN},
      {""}, {""},
#line 17 "src/keywords.def"
      {"INPUT",   TokenKind::KEY_INPUT},
      {""}, {""},
#line 27 "src/keywords.def"
      {"MUX",     TokenKind::KEY_MUX},
      {""}, {""}, {""},
#line 23 "src/keywords.def"
      {"OR",      TokenKind::KEY_OR},
#line 25 "src/keywords.def"
      {"XOR",     TokenKind::KEY_XOR},
      {""}, {""}, {""}, {""},
#line 24 "src/keywords.def"
      {"NOR",     TokenKind::KEY_NOR},
#line 26 "src/keywords.def"
      {"XNOR",    TokenKind::KEY_XNOR},
      {""},
#line 29 "src/keywords.def"
      {"CONCAT",  TokenKind::KEY_CONCAT},
      {""},
#line 32 "src/keywords.def"
      {"ROM",     TokenKind::KEY_ROM},
      {""}, {""},
#line 16 "src/keywords.def"
      {"OUTPUT",  TokenKind::KEY_OUTPUT},
      {""},
#line 20 "src/keywords.def"
      {"NOT",     TokenKind::KEY_NOT},
      {""}, {""}, {""}, {""},
#line 33 "src/keywords.def"
      {"RAM",     TokenKind::KEY_RAM},
#line 22 "src/keywords.def"
      {"NAND",    TokenKind::KEY_NAND},
      {""}, {""}, {""},
#line 18 "src/keywords.def"
      {"VAR",     TokenKind::KEY_VAR},
      {""}, {""}, {""}, {""},
#line 21 "src/keywords.def"
      {"AND",     TokenKind::KEY_AND}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        {
          const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
#line 34 "src/keywords.def"

