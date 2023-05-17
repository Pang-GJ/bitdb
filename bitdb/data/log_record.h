#pragma once

#include <cstdint>
namespace bitdb::data {

// 数据内存索引，描述数据在磁盘上的位置
struct LogRecordPst {
  uint32_t fid;    // NOLINT
  int64_t offset;  // NOLINT
};

}  // namespace bitdb::data