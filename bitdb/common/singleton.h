#pragma once

namespace bitdb {

template <class T>
class Singleton {
 public:
  static T* Get() {
    static T instance;
    return &instance;
  }

  Singleton(T&&) = delete;
  Singleton&& operator=(T&&) = delete;
  Singleton(const T&) = delete;
  Singleton& operator=(const T&) = delete;

 protected:
  Singleton() = default;
  virtual ~Singleton() = default;
};

}  // namespace bitdb