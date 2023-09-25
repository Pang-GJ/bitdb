#pragma once

#include <atomic>
#include <coroutine>
#include <utility>
#include "bitdb/co/co_thread_pool.h"
#include "bitdb/co/task.h"
#include "bitdb/common/singleton.h"
namespace bitdb::co {

class Scheduler {
 public:
  Scheduler() noexcept;

  void co_spawn(Task<>&& task) noexcept;

  template <typename Promise>
  void co_spawn(std::coroutine_handle<Promise> handle) noexcept {
    tp_.ScheduleById(handle);
  }

  void run();

  void join();

 private:
  co::ThreadPool tp_;
  std::thread run_thread_;
};

void co_spawn(Task<>&& task) noexcept;

template <typename Promise>
void co_spawn(std::coroutine_handle<Promise> handle) noexcept {
  Singleton<Scheduler>::Get()->co_spawn<Promise>(handle);
}

void co_run();

void co_join();

}  // namespace bitdb::co