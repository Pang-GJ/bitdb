#pragma once

#include <string>
namespace bitdb {

// db 的配置项
struct Options {
  std::string dir_path;    // NOLINT
  int64_t data_file_size;  // NOLINT
  bool is_sync_write;      // 每次写入数据是否持久化 // NOLINT
};

}  // namespace bitdb