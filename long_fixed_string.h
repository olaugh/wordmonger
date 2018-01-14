#ifndef LONG_FIXED_STRING_H
#define LONG_FIXED_STRING_H

#include <cassert>
#include <string>
#include <string.h>

#define LONG_FIXED_STRING_MAXIMUM_LENGTH 101

class LongFixedString {
 public:
  typedef char* iterator;
  typedef const char* const_iterator;
  typedef unsigned int size_type;
  typedef char& reference;
  typedef const char& const_reference;

  LongFixedString();
  LongFixedString(const char* s, size_type n);
  LongFixedString(size_type n, char c);
  LongFixedString(const char* s);
  LongFixedString(const LongFixedString& s);

  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  void erase(const iterator i);
  size_type length() const;
  LongFixedString substr(size_type pos, size_type n) const;
  bool empty() const;
  size_type size() const { return length(); }
  void clear() { m_end = m_data; }
  void push_back(char c);
  void pop_back();
  const char* constData() const { return m_data; }

  int compare(const LongFixedString& s) const;

  LongFixedString& operator+=(char c);
  LongFixedString& operator+=(const LongFixedString& s);

  char& operator[](size_type n) { return m_data[n]; }
  LongFixedString& operator=(const LongFixedString& s);

  static const unsigned int maxSize = LONG_FIXED_STRING_MAXIMUM_LENGTH;

 private:
  static const std::string dummyString;  // just to get to traits
  char m_data[maxSize];
  char* m_end;  // points at the terminating NULL
};


inline LongFixedString
operator+(const LongFixedString &lhs, const LongFixedString& rhs) {
  LongFixedString str(lhs);
  str += rhs;
  return str;
}

inline LongFixedString
operator+(char lhs, const LongFixedString& rhs) {
  LongFixedString str(1, lhs);
  str += rhs;
  return str;
}

inline LongFixedString operator+(const LongFixedString& lhs, char rhs) {
  LongFixedString str(lhs);
  str += rhs;
  return str;
}

inline LongFixedString::LongFixedString() : m_end(m_data) {}

inline
LongFixedString::LongFixedString(const char* s, size_type n) {
  assert(n < maxSize);
  memcpy(m_data, s, n);
  m_end = m_data + n;
}

inline LongFixedString::LongFixedString(size_type n, char c) : m_end(m_data) {
  assert(n < maxSize);
  for (unsigned int i = 0; i < n; ++i) {
    *m_end++ = c;
  }
}

inline LongFixedString::LongFixedString(const char* s) {
  unsigned int sz = strlen(s);
  assert(sz < maxSize);
  memcpy(m_data, s, sz);
  m_end = m_data + sz;
}

inline LongFixedString::LongFixedString(const LongFixedString& s) {
  int sz = s.size();
  memcpy(m_data, s.m_data, sz);
  m_end = m_data + sz;
}

inline LongFixedString& LongFixedString::operator=(const LongFixedString& s) {
  int sz = s.size();
  memcpy(m_data, s.m_data, sz);
  m_end = m_data + sz;
  return *this;
}

inline LongFixedString::const_iterator LongFixedString::begin() const {
  return m_data;
}

inline LongFixedString::const_iterator LongFixedString::end() const {
  return m_end;
}

inline LongFixedString::iterator LongFixedString::begin() { return m_data; }

inline LongFixedString::iterator LongFixedString::end() { return m_end; }

inline void LongFixedString::erase(const iterator i) {
  memmove(i, i + 1, m_end - i);
  --m_end;
}

inline LongFixedString::size_type LongFixedString::length() const {
  return m_end - m_data;
}

inline LongFixedString LongFixedString::substr(size_type pos,
                                               size_type n) const {
  assert(pos + n <= size());
  return LongFixedString(&m_data[pos], n);
}

inline bool LongFixedString::empty() const { return length() == 0; }

inline LongFixedString& LongFixedString::operator+=(char c) {
  assert(size() < maxSize - 1);
  *m_end++ = c;
  return *this;
}

inline LongFixedString& LongFixedString::operator+=(const LongFixedString& s) {
  int sz = s.size();
  assert(size() + sz < maxSize);
  memcpy(m_end, s.m_data, sz);
  m_end += sz;
  return *this;
}

inline void LongFixedString::push_back(char c) { *this += c; }

inline void LongFixedString::pop_back() {
  assert(size() > 0);
  m_end--;
}

inline int LongFixedString::compare(const LongFixedString& s) const {
  int size1 = size();
  int size2 = s.size();
  int sz = (size1 < size2) ? size1 : size2;
  for (int i = 0; i < sz; ++i) {
    if (m_data[i] < s.m_data[i]) {
      return -1;
    } else if (m_data[i] > s.m_data[i]) {
      return 1;
    }
  }
  if (size1 > size2) {
    return 1;
  } else if (size2 > size1) {
    return -1;
  }
  return 0;
}

inline bool operator<(const LongFixedString& lhs,
                      const LongFixedString& rhs) {
  return (lhs.compare(rhs) < 0);
}

inline bool operator==(const LongFixedString& lhs,
                       const LongFixedString& rhs) {
  return (lhs.compare(rhs) == 0);
}

inline bool operator!=(const LongFixedString& lhs,
                       const LongFixedString& rhs) {
  return (lhs.compare(rhs) != 0);
}

#endif
