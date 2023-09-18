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
  Scheduler() noexcept = default;

  void co_spawn(Task<>&& task) noexcept;

  template <typename Promise>
  void co_spawn(std::coroutine_handle<Promise> handle) noexcept {
//    if (handle.promise().thrd_id_ == -1) {
//      handle.promise().set_thrd(common::detail::ThisThreadId());
//    }
//    const int curr_id = handle.promise().thrd_id_ % tp_.GetThreadNum();
    tp_.ScheduleById(handle, -1);
  }

  void run();

  void join();

 private:
  co::ThreadPool tp_;
  std::atomic<int> id_generator_{0};
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