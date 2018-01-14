#ifndef GADDAG_H
#define GADDAG_H

#include <bitset>

#include "util.h"

#define GADDAG_SEPARATOR 0

class Gaddag {
 public:
  Gaddag(const char* data, Letter last_letter, int bitset_size,
         int index_size);

  inline const unsigned char* NextRackChild(const unsigned char* bitset_data,
                                            Letter min_letter,
                                            uint32_t rack_bits,
                                            int* child_index,
                                            Letter* next_letter) const {
    const uint32_t& bitset = *(reinterpret_cast<const uint32_t*>(bitset_data));
    for (;;) {
      const uint32_t inverse_mask = (1 << min_letter) - 1;
      *next_letter = __builtin_ffs(bitset & (~inverse_mask)) - 1;
      if (*next_letter > last_letter_) {
        return nullptr;
      }
      if (rack_bits & (1 << *next_letter)) {
        break;
      }
      min_letter = *next_letter + 1;
      (*child_index)++;
    }
    return bitset_data + bitset_size_ + (index_size_ * (*child_index));
  }

  inline const unsigned char* NextChild(const unsigned char* bitset_data,
                                        Letter min_letter, int* child_index,
                                        Letter* next_letter) const {
    const uint32_t& bitset = *(reinterpret_cast<const uint32_t*>(bitset_data));
    const uint32_t inverse_mask = (1 << min_letter) - 1;
    *next_letter = __builtin_ffs(bitset & (~inverse_mask)) - 1;
    if (*next_letter > last_letter_) return nullptr;
    return bitset_data + bitset_size_ + (index_size_ * (*child_index));
  }

  uint32_t SharedChildren(const unsigned char* bitset_data1,
                          const unsigned char* bitset_data2) const;
  uint32_t Intersection(const unsigned char* bitset_data,
                        uint32_t rack_bits) const;
  bool HasAnyChild(const unsigned char* bitset_data, uint32_t rack_bits) const;

  inline bool HasChild(const unsigned char* bitset_data, Letter letter) const {
    uint32_t letter_mask = 1 << letter;
    const uint32_t& bitset = *(reinterpret_cast<const uint32_t*>(bitset_data));
    return (bitset & letter_mask) != 0;
  }

  int NumChildren(const unsigned char* bitset_data) const;

  inline const unsigned char*
    ChangeDirection(const unsigned char* bitset_data) const {
    if (HasChild(bitset_data, GADDAG_SEPARATOR)) {
      return bitset_data + bitset_size_;
    } else {
      return nullptr;
    }
  }

  inline const unsigned char*
    Child(const unsigned char* bitset_data, Letter letter) const {
    const uint32_t& bitset = *(reinterpret_cast<const uint32_t*>(bitset_data));
    uint32_t before_letter_mask = (1 << letter) - 1;
    int index = __builtin_popcount(bitset & before_letter_mask);
    return bitset_data + bitset_size_ + (index_size_ * index);
  }

  inline bool CompletesWord(const unsigned char* index_data) const {
    uint32_t data = *(reinterpret_cast<const uint32_t*>(index_data));
    return (data & completes_word_mask_) != 0;
  }

  inline const unsigned char* FollowIndex(
      const unsigned char* index_data) const {
    uint32_t data = *(reinterpret_cast<const uint32_t*>(index_data));
    uint32_t index = data & index_mask_;
    if (index == 0) return nullptr;
    return data_ + index;
  }

  inline const unsigned char* Root() const { return data_; }

 private:
  const unsigned char* data_;
  const Letter last_letter_;
  const int bitset_size_;
  const int index_size_;
  const uint32_t completes_word_mask_;
  const uint32_t index_mask_;
};

#endif
