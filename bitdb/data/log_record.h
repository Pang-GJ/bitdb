#pragma once

#include <cstdint>
#include <memory>
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/string_utils.h"
namespace bitdb::data {

// 数据内存索引，描述数据在磁盘上的位置
struct LogRecordPst {
  uint32_t fid;    // NOLINT
  int64_t offset;  // NOLINT
};

enum LogRecordType : int8_t {
  LogRecordNormal = 1,
  LogRecordDeleted,
};

// crc type KeySize ValueSize
// 4 + 1 +  5 +     5 (byte)
// +-------------+------------+------------+--------------+-------+---------+
// |  crc 校验值  |  type 类型  |  key size  |  value size  |  key  |  value  |
// +-------------+------------+------------+--------------+-------+---------+
// |   4 字节     |  1 字节     | 变长(最大5) |  变长(最大5)   |  变长  |  变长
// +-------------+------------+------------+--------------+-------+---------+
constexpr size_t K_MAX_LOG_RECORD_HEADER_SIZE = 5 * 2 + 5;

struct LogRecordHeader {
  uint32_t crc;               // NOLINT
  LogRecordType record_type;  // NOLINT
  uint32_t key_size;          // NOLINT
  uint32_t value_size;        // NOLINT

  LogRecordHeader(uint32_t crc, LogRecordType record_type)
      : crc(crc), record_type(record_type) {}
};

// TODO(pangguojian): 这里应该可以优化成 Bytes（深拷贝version)
struct LogRecord {
  std::string key;     // NOLINT
  std::string value;   // NOLINT
  LogRecordType type;  // NOLINT

  bool operator==(const LogRecord& other) const {
    return key == other.key && value == other.value && type == other.type;
  }

  std::string ToString() const {
    auto TypeToString = [](LogRecordType type) {  // NOLINT
      switch (type) {
        case LogRecordNormal:
          return "Normal";
        case LogRecordDeleted:
          return "Deleted";
        default:
          return "Unknown";
      }
    };
    return Format("{}_{}_{}", key, value, TypeToString(type));
  }
};

/**
 * @brief 对 log_record 进行数据编码
 *
 * @param log_record
 * @return std::string
 */
std::string EncodeLogRecord(const LogRecord& log_record,
                            uint32_t* crc32_ptr = nullptr,
                            char** header_buf = nullptr,
                            size_t* header_buf_sz = nullptr);

/**
 * @brief 对字节数组中的 Header 信息进行解码
 *
 * @param bytes
 * @return LogRecordHeader
 */
std::pair<std::unique_ptr<LogRecordHeader>, uint32_t> DecodeLogRecordHeader(
    const Bytes& bytes);

/**
 * @brief Get the Log Record CRC object
 *
 * @param log_record
 * @param header
 * @return uint32_t
 */
uint32_t GetLogRecordCRC(const LogRecord& log_record, const std::string& header);

constexpr size_t K_CRC32_SIZE = 4;
}  // namespace bitdb::data