#pragma once

#include <utility>

template <typename T>
class DeferImpl {
 public:
  explicit DeferImpl(T&& d) : defer_call_(std::move(d)) {}
  ~DeferImpl() { defer_call_(); }

 private:
  T defer_call_;
};

class Defer {
 public:
  template <typename T>
  DeferImpl<T> operator+(T&& d) {
    return DeferImpl<T>(std::forward<T>(d));
  }
};

#define CONCAT_IMPLE(prefix, suffix) prefix##suffix  // 宏连接
#define CONCAT(prefx, suffix) CONCAT_IMPLE(prefix, suffix)
#define UNIQUE_NAME(prefix) CONCAT(prefix, __COUNTER__)
#define defer auto UNIQUE_NAME(defer_) = Defer() + [&]  // NOLINT