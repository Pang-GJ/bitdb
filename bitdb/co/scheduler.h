#pragma once

#include <atomic>
#include <utility>
#include "bitdb/co/co_thread_pool.h"
#include "bitdb/co/task.h"
#include "bitdb/common/singleton.h"
namespace bitdb::co {

class Scheduler {
 public:
  Scheduler() noexcept = default;

  void co_spawn(Task<>&& task) noexcept;

  void run() noexcept;

 private:
  co::ThreadPool tp_;
  std::atomic<int> id_generator_{0};
};

void co_spawn(Task<>&& task) noexcept {
  Singleton<Scheduler>::Get()->co_spawn(std::forward<Task<>>(task));
}

}  // namespace bitdb::co