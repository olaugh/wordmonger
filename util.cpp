#include "util.h"

WordString Util::EncodeWord(const QString& word) {
  //qInfo() << "EncodeWord(" << word << ")";
  WordString word_string;
  for (const auto& c : word) {
    //qInfo() << "c: " << c;
    word_string.push_back(EncodeLetter(c));
  }
  return word_string;
}

Letter Util::EncodeLetter(const QChar& c) {
  return FIRST_LETTER + c.toUpper().toLatin1() - 'A';
}

QString Util::DecodeWord(const WordString& word) {
  QString ret;
  for (char c : word) {
    ret = DecodeLetter(c) + ret;
  }
  return ret;
}

QChar Util::DecodeLetter(Letter c) {
  return QChar::fromLatin1('A' - FIRST_LETTER + c);
}
