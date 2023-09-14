#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "bitdb/common/logger.h"
#include "spdlog/spdlog.h"

int main() {
  ankerl::nanobench::Bench().run("bitdb log data", [&] {
    for (auto i = 0; i < 1000000; ++i) {
      LOG_INFO("hello");
    }
  });

  // 初始化spdlog库
  spdlog::set_pattern("[%H:%M:%S] [%l] %v");

  // 创建文件日志记录器，输出到 example.log 文件
  auto file_logger = spdlog::basic_logger_mt("file_logger", "example.log");

  ankerl::nanobench::Bench().run("spdlog data", [&] {
    for (auto i = 0; i < 1000000; ++i) {
      file_logger->info("hello");
    }
  });
  spdlog::drop_all();
}