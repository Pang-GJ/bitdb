#pragma once

#include <chrono>
#include <thread>
namespace bitdb::timer {

using MSTime = std::chrono::milliseconds;

inline MSTime ClockMS() {
  const auto curr_timepoint = std::chrono::system_clock::now();
  return std::chrono::duration_cast<MSTime>(curr_timepoint.time_since_epoch());
}

inline void SleepForMS(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

template <typename Time>
inline Time TimeDiff(const Time& t1, const Time& t2) {
  return std::chrono::duration_cast<Time>(t1 - t2);
}

template <typename Time>
inline Time TimeDiffNow(const Time& last_time) {
  const auto curr_time = std::chrono::duration_cast<Time>(
      std::chrono::system_clock::now().time_since_epoch());
  return std::chrono::duration_cast<Time>(curr_time - last_time);
}

}  // namespace bitdb::timer