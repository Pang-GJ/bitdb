#pragma once

#include <cstdint>
#include <memory>
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

// crc type KeySize ValueSize
// 4 + 1 +  5 +     5 (byte)
constexpr size_t K_MAX_LOG_RECORD_HEADER_ITEM_SIZE = 5;

struct LogRecordHeader {
  uint32_t crc;               // NOLINT
  LogRecordType record_type;  // NOLINT
  uint32_t key_size;          // NOLINT
  uint32_t value_size;        // NOLINT
};

struct LogRecord {
  Bytes key;           // NOLINT
  Bytes value;         // NOLINT
  LogRecordType type;  // NOLINT
};

/**
 * @brief 对 log_record 进行数据编码
 *
 * @param log_record
 * @return Bytes
 */
Bytes EncodeLogRecord(const LogRecord& log_record);

/**
 * @brief 对字节数组中的 Header 信息进行解码
 *
 * @param bytes
 * @return LogRecordHeader
 */
std::pair<std::unique_ptr<LogRecordHeader>, int64_t> DecodeLogRecordHeader(
    const Bytes& bytes);

/**
 * @brief Get the Log Record CRC object
 *
 * @param log_record
 * @param header
 * @return uint32_t
 */
uint32_t GetLogRecordCRC(const LogRecord& log_record, const Bytes& header);

constexpr size_t K_CRC32_SIZE = 5;
}  // namespace bitdb::data