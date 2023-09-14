#pragma once

#include <atomic>
namespace bitdb::common {

class SpinLock {
 public:
  explicit SpinLock(std::atomic_flag& flag) : flag_(flag) {
    while (flag_.test_and_set(std::memory_order_acquire)) {
      ;
    }
  }

  ~SpinLock() { flag_.clear(std::memory_order_release); }

 private:
  std::atomic_flag& flag_;
};

}  // namespace bitdb::common