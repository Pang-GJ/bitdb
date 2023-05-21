#pragma once

#include <cstdint>
#include "bitdb/utils/bytes.h"
namespace bitdb::data {

// 数据内存索引，描述数据在磁盘上的位置
struct LogRecordPst {
  uint32_t fid;    // NOLINT
  int64_t offset;  // NOLINT
};

enum LogRecordType {
  LogRecordNormal = 0,
  LogRecordDeleted,
};

struct LogRecord {
  Bytes key;           // NOLINT
  Bytes value;         // NOLINT
  LogRecordType type;  // NOLINT
};

// 对 LogRecord 进行数据组织
Bytes EncodeLogRecord(const LogRecord& log_record);

}  // namespace bitdb::data