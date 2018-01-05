#include <QtGlobal>
#include <QDebug>

#include "gaddag.h"

Gaddag::Gaddag(const char* data, Letter last_letter, int bitset_size,
               int index_size)
    : data_(reinterpret_cast<const unsigned char*>(data)),
      last_letter_(last_letter),
      bitset_size_(bitset_size),
      index_size_(index_size),
      completes_word_mask_(1 << (index_size * 8 - 1)),
      index_mask_(completes_word_mask_ - 1) {
  qInfo() << "completes_word_mask_: " << completes_word_mask_;
  qInfo() << "index_mask_: " << index_mask_;
}

uint32_t Gaddag::SharedChildren(const unsigned char* bitset_data1,
                                const unsigned char* bitset_data2) const {
  const uint32_t& bitset1 = *(reinterpret_cast<const uint32_t*>(bitset_data1));
  const uint32_t& bitset2 = *(reinterpret_cast<const uint32_t*>(bitset_data2));
  return bitset1 & bitset2;
}

uint32_t Gaddag::Intersection(const unsigned char* bitset_data,
                              uint32_t rack_bits) const {
  const uint32_t& bitset = *(reinterpret_cast<const uint32_t*>(bitset_data));
  return (bitset & rack_bits);
}

bool Gaddag::HasAnyChild(const unsigned char* bitset_data,
                         uint32_t rack_bits) const {
  const uint32_t& bitset = *(reinterpret_cast<const uint32_t*>(bitset_data));
  return (bitset & rack_bits) != 0;
}

int Gaddag::NumChildren(const unsigned char* bitset_data) const {
  std::bitset<32> bits(*(reinterpret_cast<const uint32_t*>(bitset_data)));
  return bits.count();
}
