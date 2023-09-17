#include "bitdb/co/scheduler.h"
#include "bitdb/co/co_thread_pool.h"
#include "bitdb/common/logger_impl.h"

namespace bitdb::co {

void Scheduler::co_spawn(Task<>&& task) noexcept {
  auto handle = task.get_handle();
  if (handle.promise().thrd_id_ == -1) {
    handle.promise().set_thrd(common::detail::ThisThreadId());
  }
  const int curr_id = handle.promise().thrd_id_ % tp_.GetThreadNum();
  task.detach();
  tp_.ScheduleById(handle, curr_id);
}

void Scheduler::run() noexcept { tp_.WaitStop(); }

}  // namespace bitdb::co