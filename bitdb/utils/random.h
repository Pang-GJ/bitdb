#pragma once

// A very simple random number generator.  Not especially good at
// generating truly random bits, but good enough for our needs in this
// package.
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <random>

namespace bitdb {

class Random {
 private:
  uint32_t seed_;
  bool simple_mode_;
  std::default_random_engine engine_;

 public:
  explicit Random(uint32_t s = 0, bool simple_mode = false)
      : seed_(s), simple_mode_(simple_mode) {
    if (seed_ > 0) {
      engine_.seed(seed_);
    } else {
      engine_.seed(std::time(nullptr));
    }
  }

  /** 谷歌 leveldb 实现
  uint32_t Next() {
    static const uint32_t M = 2147483647L;  // 2^31-1 // NOLINT
    static const uint64_t A = 16807;  // bits 14, 8, 7, 5, 2, 1, 0 // NOLINT
    // We are computing
    //       seed_ = (seed_ * A) % M,    where M = 2^31-1
    //
    // seed_ must not be zero or M, or else all subsequent computed values
    // will be zero or M respectively.  For all other values, seed_ will end
    // up cycling through every number in [1,M-1]
    uint64_t product = seed_ * A;

    // Compute (product % M) using the fact that ((x << 31) % M) == x.
    seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
    // The first reduction may overflow by 1 bit, so we may need to
    // repeat.  mod == M is not possible; using > allows the faster
    // sign-bit-based test.
    if (seed_ > M) {
      seed_ -= M;
    }
    return seed_;
    return rand();
  }
  **/

  uint32_t GetSimpleRandomNum() { return rand(); }
  uint32_t GetRandomNum() { return engine_(); }

  uint32_t Next() {
    if (simple_mode_) {
      return GetSimpleRandomNum();
    }
    return GetRandomNum();
  }

  /**
   * @brief Returns a uniformly distributed value in the range [0..n-1]
   *
   * @param n n should > 0
   * @return uint32_t
   */
  uint32_t Uniform(int n) { return Next() % n; }

  /**
   * @brief Returns a uniformly distributed value in the range [begin..end-1]
   *
   * @param begin
   * @param end
   * @return uint32_t
   */
  uint32_t UniformRange(int begin, int end) {
    int delta = end - begin;
    return begin + Uniform(delta);
  }

  // Randomly returns true ~"1/n" of the time, and false otherwise.
  // REQUIRES: n > 0
  bool OneIn(int n) { return (Next() % n) == 0; }

  // Skewed: pick "base" uniformly from range [0,max_log] and then
  // return "base" random bits.  The effect is to pick a number in the
  // range [0,2^max_log-1] with exponential bias towards smaller numbers.
  uint32_t Skewed(int max_log) { return Uniform(1 << Uniform(max_log + 1)); }
};

}  // namespace bitdb