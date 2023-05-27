#pragma once

#include <string>
#include "bitdb/status.h"
namespace bitdb {

// db 的配置项
struct Options {
  std::string dir_path;    // NOLINT
  int64_t data_file_size;  // NOLINT
  bool is_sync_write;      // 每次写入数据是否持久化 // NOLINT
};

Status CheckOptions(const Options& options) {
  if (options.dir_path.empty()) {
    return Status::InvalidArgument("Options", "dir_path is emtpy.");
  }
  if (options.data_file_size <= 0) {
    return Status::InvalidArgument("Options", "data file size isn't positive");
  }
  return Status::Ok();
}

}  // namespace bitdb