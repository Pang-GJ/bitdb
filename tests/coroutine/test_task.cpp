#include <iostream>
#include <thread>
#include "bitdb/co/scheduler.h"
#include "bitdb/co/task.h"
#include "bitdb/common/logger.h"

using bitdb::co::co_spawn;

bitdb::co::Task<int> simple_task2() {
  LOG_INFO("task 2 start...\n");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
  LOG_INFO("task 2 returns after 1s\n");
  co_return 2;
}

bitdb::co::Task<int> simple_task3() {
  LOG_INFO("task 3 start...\n");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
  LOG_INFO("task 3 returns after 2s\n");
  co_return 3;
}

bitdb::co::Task<int> simple_task() {
  LOG_INFO("task start...\n");
  LOG_INFO("waiting for result2...\n");
  auto result2 = co_await simple_task2();
  LOG_INFO("result from task2: {}\n", result2);
  auto result3 = co_await simple_task3();
  LOG_INFO("result from task3: {}\n", result3);
  co_return 1 + result2 + result3;
}

bitdb::co::Task<> co_main() {
  auto res = co_await simple_task();
  LOG_INFO("the result of simple_task: {}\n", res);
}

bitdb::co::Task<int> ans() { co_return 42; }

int main(int argc, char* argv[]) {
  co_spawn([]() -> bitdb::co::Task<> {
    auto res = co_await simple_task();
    LOG_INFO("the result of simple_task: {}\n", res);
  }());
  co_spawn(co_main());
  return 0;
}
