#include "bitdb/co/scheduler.h"
#include "bitdb/co/co_thread_pool.h"

namespace bitdb::co {

void Scheduler::co_spawn(Task<>&& task) noexcept {
  auto handle = task.get_handle();
  task.detach();
  tp_.ScheduleById(handle);
}

}  // namespace bitdb::co