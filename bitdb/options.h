#pragma once

#include <cstdint>
#include <string>
#include "bitdb/index/index.h"
#include "bitdb/status.h"
#include "bitdb/utils/os_utils.h"
namespace bitdb {

// db 的配置项
struct Options {
  std::string dir_path;           // NOLINT
  uint64_t data_file_size;        // NOLINT
  bool is_sync_write;             // 每次写入数据是否持久化 // NOLINT
  index::IndexerType index_type;  // NOLINT
};

inline Status CheckOptions(const Options& options) {
  if (options.dir_path.empty()) {
    return Status::InvalidArgument("Options", "dir_path is emtpy.");
  }
  if (options.data_file_size <= 0) {
    return Status::InvalidArgument("Options", "data file size isn't positive");
  }
  return Status::Ok();
}

// batch 配置项
struct WriteBatchOptions {
  uint64_t max_batch_num;  // NOLINT
  bool is_sync_write;      // NOLINT
};

// 默认配置
inline Options DefaultOptions() {
  Options default_options{
      .dir_path = GetTempDir(),
      .data_file_size = 256 * 1024 * 1024,
      .is_sync_write = false,
      .index_type = index::SkipListIndex,
  };
  return default_options;
}

inline WriteBatchOptions DefaultWriteBatchOptions() {
  WriteBatchOptions batch_options{.max_batch_num = 100, .is_sync_write = false};
  return batch_options;
}

}  // namespace bitdb