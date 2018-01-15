#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

#include "fixed_string.h"
#include "long_fixed_string.h"

using Bag = LongFixedString;
using Letter = unsigned char;
using WordString = FixedString;

#define DELIMITER 0
#define BLANK 0
#define FIRST_LETTER 1
#define LAST_LETTER 26
#define NOT_A_LETTER 27

class Util {
 public:
  static Bag ScrabbleBag();
  static WordString BlankRack(const Bag& bag, int blanks, int size);
  static WordString RandomRack(const Bag& bag, int size);

  static WordString EncodeWord(const QString& word);
  static Bag EncodeBag(const QString& word);
  static Letter EncodeLetter(const QChar& c);

  static QString DecodeWord(const WordString& word);
  static QString DecodeBag(const Bag& word);
  static QChar DecodeLetter(Letter c);

  static QString DecodeBits(int32_t bits);
  static QString DecodeCounts(int* counts);
};


#endif // UTIL_H
