#ifndef FIXED_STRING_H
#define FIXED_STRING_H

#include <cassert>
#include <string>
#include <string.h>

#define FIXED_STRING_MAXIMUM_LENGTH 17

class FixedString {
 public:
  typedef char* iterator;
  typedef const char* const_iterator;
  typedef unsigned int size_type;
  typedef char& reference;
  typedef const char& const_reference;

  FixedString();
  FixedString(const char* s, size_type n);
  FixedString(size_type n, char c);
  FixedString(const char* s);
  FixedString(const FixedString& s);

  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  void erase(const iterator i);
  size_type length() const;
  FixedString substr(size_type pos, size_type n) const;
  bool empty() const;
  size_type size() const { return length(); }
  void clear() { m_end = m_data; }
  void push_back(char c);
  void pop_back();
  const char* constData() const { return m_data; }

  int compare(const FixedString& s) const;

  FixedString& operator+=(char c);
  FixedString& operator+=(const FixedString& s);

  const_reference operator[](size_type n) const { return m_data[n]; }
  FixedString& operator=(const FixedString& s);

  static const unsigned int maxSize = FIXED_STRING_MAXIMUM_LENGTH;

 private:
  static const std::string dummyString;  // just to get to traits
  char m_data[maxSize];
  char* m_end;  // points at the terminating NULL
};


inline FixedString
operator+(const FixedString &lhs, const FixedString& rhs) {
  FixedString str(lhs);
  str += rhs;
  return str;
}

inline FixedString
operator+(char lhs, const FixedString& rhs) {
  FixedString str(1, lhs);
  str += rhs;
  return str;
}

inline FixedString operator+(const FixedString& lhs, char rhs) {
  FixedString str(lhs);
  str += rhs;
  return str;
}

inline FixedString::FixedString() : m_end(m_data) {}

inline
FixedString::FixedString(const char* s, size_type n) {
  assert(n < maxSize);
  memcpy(m_data, s, n);
  m_end = m_data + n;
}

inline FixedString::FixedString(size_type n, char c) : m_end(m_data) {
  assert(n < maxSize);
  for (unsigned int i = 0; i < n; ++i) {
    *m_end++ = c;
  }
}

inline FixedString::FixedString(const char* s) {
  unsigned int sz = strlen(s);
  assert(sz < maxSize);
  memcpy(m_data, s, sz);
  m_end = m_data + sz;
}

inline FixedString::FixedString(const FixedString& s) {
  int sz = s.size();
  memcpy(m_data, s.m_data, sz);
  m_end = m_data + sz;
}

inline FixedString& FixedString::operator=(const FixedString& s) {
  int sz = s.size();
  memcpy(m_data, s.m_data, sz);
  m_end = m_data + sz;
  return *this;
}

inline FixedString::const_iterator FixedString::begin() const { return m_data; }

inline FixedString::const_iterator FixedString::end() const { return m_end; }

inline FixedString::iterator FixedString::begin() { return m_data; }

inline FixedString::iterator FixedString::end() { return m_end; }

inline void FixedString::erase(const iterator i) {
  memmove(i, i + 1, m_end - i);
  --m_end;
}

inline FixedString::size_type FixedString::length() const {
  return m_end - m_data;
}

inline FixedString FixedString::substr(size_type pos, size_type n) const {
  assert(pos + n <= size());
  return FixedString(&m_data[pos], n);
}

inline bool FixedString::empty() const { return length() == 0; }

inline FixedString& FixedString::operator+=(char c) {
  assert(size() < maxSize - 1);
  *m_end++ = c;
  return *this;
}

inline FixedString& FixedString::operator+=(const FixedString& s) {
  int sz = s.size();
  assert(size() + sz < maxSize);
  memcpy(m_end, s.m_data, sz);
  m_end += sz;
  return *this;
}

inline void FixedString::push_back(char c) { *this += c; }

inline void FixedString::pop_back() {
  assert(size() > 0);
  m_end--;
}

inline int FixedString::compare(const FixedString& s) const {
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

inline bool operator<(const FixedString& lhs,
                      const FixedString& rhs) {
  return (lhs.compare(rhs) < 0);
}

inline bool operator==(const FixedString& lhs,
                       const FixedString& rhs) {
  return (lhs.compare(rhs) == 0);
}

inline bool operator!=(const FixedString& lhs,
                       const FixedString& rhs) {
  return (lhs.compare(rhs) != 0);
}

#endif
