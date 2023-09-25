#include "bitdb/co/scheduler.h"
#include <thread>

namespace bitdb::co {

Scheduler::Scheduler() noexcept
    : tp_(std::thread::hardware_concurrency(), true, true) {}

void Scheduler::co_spawn(Task<>&& task) noexcept {
  auto handle = task.get_handle();
  if (handle.promise().thrd_id_ == -1) {
    handle.promise().set_thrd(common::detail::ThisThreadId());
  }
  const int curr_id = handle.promise().thrd_id_ % tp_.GetThreadNum();
  task.detach();
  // tp_.ScheduleById(handle, curr_id);
  tp_.ScheduleById(handle);
}

void Scheduler::run() {
  run_thread_ = std::thread(&ThreadPool::WaitStop, &tp_);
}
void Scheduler::join() {
  if (run_thread_.joinable()) {
    LOG_DEBUG("run thread join");
    run_thread_.join();
  }
}

void co_spawn(Task<>&& task) noexcept {
  Singleton<Scheduler>::Get()->co_spawn(std::forward<Task<>>(task));
}

void co_run() { Singleton<Scheduler>::Get()->run(); }

void co_join() { Singleton<Scheduler>::Get()->join(); }

}  // namespace bitdb::co