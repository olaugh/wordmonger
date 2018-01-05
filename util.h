#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

#include "fixed_string.h"

using Letter = unsigned char;
using WordString = FixedString;

#define DELIMITER 0
#define FIRST_LETTER 1
#define LAST_LETTER 26

class Util {
 public:
  static WordString EncodeWord(const QString& word);
  static Letter EncodeLetter(const QChar& c);

  static QString DecodeWord(const WordString& word);
  static QChar DecodeLetter(Letter c);
};

#endif // UTIL_H
