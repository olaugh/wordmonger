#include "util.h"

Bag Util::ScrabbleBag() {
  QString bag("??AAAAAAAAABBCCDDDDEEEEEEEEEEEEFFGGGHHIIIIIIIIIJKLLLLMMNNNNNNOOOOOOOOPPQRRRRRRSSSSTTTTTTUUUUVVWWXYYZ");
  qInfo() << "bag.length():" << bag.length();
  return EncodeBag(bag);
}

WordString Util::BlankRack(const Bag& bag, int blanks, int size) {
  Bag bag_copy = bag;
  WordString rack;
  for (int i = 0; i < blanks; ++i) {
    rack += BLANK;
  }
  for (int i = blanks; i < size;) {
    const int letter_pos = rand() % bag.size();
    const Letter c = bag_copy[letter_pos];
    if (c >= FIRST_LETTER && c <= LAST_LETTER) {
      rack += c;
      i++;
    } else {
      bag_copy[letter_pos] = NOT_A_LETTER;
    }
  }
  return rack;
}

WordString Util::RandomRack(const Bag& bag, int size) {
  Bag bag_copy = bag;
  WordString rack;
  for (int i = 0; i < size;) {
    const int letter_pos = rand() % bag.size();
    const Letter c = bag_copy[letter_pos];
    if (c <= LAST_LETTER) {
      rack += c;
      i++;
    } else {
      bag_copy[letter_pos] = NOT_A_LETTER;
    }
  }
  return rack;
}

WordString Util::EncodeWord(const QString& word) {
  // qInfo() << "EncodeWord(" << word << ")";
  WordString word_string;
  for (const auto& c : word) {
    // qInfo() << "c: " << c;
    word_string.push_back(EncodeLetter(c));
  }
  return word_string;
}

Bag Util::EncodeBag(const QString& word) {
  //qInfo() << "EncodeWord(" << word << ")";
  Bag word_string;
  for (const auto& c : word) {
    //qInfo() << "c: " << c;
    word_string.push_back(EncodeLetter(c));
  }
  return word_string;
}

Letter Util::EncodeLetter(const QChar& c) {
  if (c == '?') return BLANK;
  return FIRST_LETTER + c.toUpper().toLatin1() - 'A';
}

QString Util::DecodeWord(const WordString& word) {
  QString ret;
  for (char c : word) {
    ret += DecodeLetter(c);
  }
  return ret;
}

QString Util::DecodeBag(const Bag& word) {
  QString ret;
  for (char c : word) {
    ret += DecodeLetter(c);
  }
  return ret;
}

QChar Util::DecodeLetter(Letter c) {
  if (c == BLANK) return '?';
  return QChar::fromLatin1('A' - FIRST_LETTER + c);
}

QString Util::DecodeBits(int32_t bits) {
  QString s;
  for (Letter c = FIRST_LETTER; c <= LAST_LETTER; ++c) {
    int32_t bit = 1 << c;
    if ((bits & bit) != 0) {
      s += DecodeLetter(c);
    }
  }
  return s;
}

QString Util::DecodeCounts(int* counts) {
  QString word;
  for (Letter c = BLANK; c <= LAST_LETTER; ++c) {
    for (int i = 0; i < counts[c]; i++) {
      word += DecodeLetter(c);
    }
  }
  return word;
}
