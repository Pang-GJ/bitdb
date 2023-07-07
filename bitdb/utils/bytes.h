#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <string>

namespace bitdb {

// ready-only, to replace std::string
// learn from
// https://github.com/ideawu/ssdb/blob/master/src/util/bytes.h#LL13C13-L13C13
class Bytes {
 public:
  Bytes() : data_(""), size_(0) {}

  Bytes(void* data, size_t size)
      : data_(static_cast<char*>(data)), size_(size) {}

  Bytes(const char* data, size_t size) : data_(data), size_(size) {}

  Bytes(const std::string& str)  // NOLINT
      : data_(str.data()), size_(str.size()) {}

  Bytes(std::string_view str)  // NOLINT
      : data_(str.data()), size_(str.size()) {}

  Bytes(const char* str) : data_(str), size_(std::strlen(str)) {}  // NOLINT

  ~Bytes() {
    if (deep_copy_) {
      delete data_;
    }
  }

  Bytes(const Bytes& other) {
    deep_copy_ = other.deep_copy_;
    size_ = other.size_;
    if (deep_copy_) {
      data_ = new char[size_];
      std::memcpy(const_cast<char*>(data_), other.data_, size_);
    } else {
      data_ = other.data_;
    }
  }

  Bytes& operator=(const Bytes& other) {
    if (this != &other) {
      deep_copy_ = other.deep_copy_;
      size_ = other.size_;
      if (deep_copy_) {
        data_ = new char[size_];
        std::memcpy(const_cast<char*>(data_), other.data_, size_);
      } else {
        data_ = other.data_;
      }
    }
    return *this;
  }

  static Bytes DeepCopy(const char* data, size_t size) {
    return Bytes(data, size, true);
  }

  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  const char* data() const { return data_; }

  bool empty() const { return size_ == 0; }

  size_t size() const { return size_; }

  int compare(const Bytes& b) const {
    const auto min_len = std::min(b.size(), size_);
    int res = std::memcmp(data_, b.data_, min_len);
    if (res == 0) {
      if (size_ < b.size_) {
        res = -1;
      } else {
        res = 1;
      }
    }
    return res;
  }

  void RemovePrefix(size_t n) {
    assert(n <= size_);
    data_ += n;
    size_ -= n;
  }

  std::string ToString() const { return std::string{data_, size_}; }

 private:
  Bytes(const char* data, size_t size, bool deep_copy) {
    size_ = size;
    deep_copy_ = deep_copy;
    if (deep_copy) {
      // TOOD(pangguojian): still has bugs. how to deep copy for "const char*"
      data_ = new char[size];
      std::memcpy(const_cast<char*>(data_), data, size);
    } else {
      data_ = data;
    }
  }

  const char* data_;
  size_t size_;
  bool deep_copy_{false};  // 是否是深拷贝
};

inline bool operator==(const Bytes& x, const Bytes& y) {
  return (x.size() == y.size()) &&
         (std::memcmp(x.data(), y.data(), x.size()) == 0);
}

inline bool operator!=(const Bytes& x, const Bytes& y) { return !(x == y); }

inline bool operator>(const Bytes& x, const Bytes& y) {
  return x.compare(y) > 0;
}
inline bool operator>=(const Bytes& x, const Bytes& y) {
  return x.compare(y) >= 0;
}
inline bool operator<(const Bytes& x, const Bytes& y) {
  return x.compare(y) < 0;
}
inline bool operator<=(const Bytes& x, const Bytes& y) {
  return x.compare(y) <= 0;
}

}  // namespace bitdb

namespace std {
template <>
struct hash<bitdb::Bytes> {
  size_t operator()(const bitdb::Bytes& b) const {
    return std::hash<const char*>{}(b.data());
  }
};

}  // namespace std