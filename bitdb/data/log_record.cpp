#include "bitdb/data/log_record.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include "bitdb/common/logger.h"
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/coding.h"
#include "bitdb/utils/defer.h"
#include "crc32c/crc32c.h"

namespace bitdb::data {

std::string EncodeLogRecord(const LogRecord& log_record) {
  if (log_record.key.empty()) {
    LOG_ERROR("The log record's key is empty");
    return {};
  }
  char header[K_MAX_LOG_RECORD_HEADER_SIZE];
  // type 是 int8，刚好是 char 的大小
  // crc 后面是 type
  header[K_CRC_SIZE] = static_cast<char>(log_record.type);
  auto header_idx = K_CRC_SIZE + K_LOG_RECORD_TYPE_SIZE;
  // 5字节以之后存储的是 key 和 value 的长度信息
  // 使用变长变量，节省空间
  header_idx += EncodeVarint32(header + header_idx,
                               static_cast<uint32_t>(log_record.key.size()));
  header_idx += EncodeVarint32(header + header_idx,
                               static_cast<uint32_t>(log_record.value.size()));

  std::string res(header, header + header_idx);
  res.append(log_record.key);
  res.append(log_record.value);

  // 计算 crc32
  auto crc32 =
      crc32c::Crc32c(res.c_str() + K_CRC_SIZE, res.size() - K_CRC_SIZE);
  char crc32_buf[K_CRC_SIZE];
  EncodeFixed32(crc32_buf, crc32);
  for (size_t i = 0; i < K_CRC_SIZE; ++i) {
    res[i] = crc32_buf[i];
  }
  return res;
}

std::pair<LogRecordHeader, uint32_t> DecodeLogRecordHeader(const Bytes& bytes) {
  if (bytes.size() < K_CRC_SIZE) {
    LOG_ERROR("Log Record Header buf size less than default crc size.");
    return std::make_pair(LogRecordHeader{}, 0);
  }

  LogRecordHeader header{DecodeFixed32(bytes.data()),
                         static_cast<LogRecordType>(bytes[K_CRC_SIZE])};
  auto header_idx = K_CRC_SIZE + K_LOG_RECORD_TYPE_SIZE;
  // 取出 key /val 对应的 size
  uint32_t key_size;
  uint32_t value_size;
  header_idx += DecodeVarint32(bytes.data() + header_idx, &key_size);
  header_idx += DecodeVarint32(bytes.data() + header_idx, &value_size);
  header.key_size = key_size;
  header.value_size = value_size;
  return std::make_pair(header, header_idx);
}

uint32_t GetLogRecordCRC(const LogRecord& log_record,
                         const std::string& header) {
  if (header.empty()) {
    return 0;
  }
  auto crc32 =
      crc32c::Crc32c(header.data() + K_CRC_SIZE, header.size() - K_CRC_SIZE);
  crc32 = crc32c::Extend(
      crc32, reinterpret_cast<const uint8_t*>(log_record.key.data()),
      log_record.key.size());
  crc32 = crc32c::Extend(
      crc32, reinterpret_cast<const uint8_t*>(log_record.value.data()),
      log_record.value.size());
  return crc32;
}

std::string EncodeLogRecordPosition(LogRecordPst* pst) {
  char buffer[K_MAX_VARINT32_LEN + K_MAX_VARINT64_LEN];
  size_t index = 0;
  index += EncodeVarint32(buffer + index, pst->fid);
  index += EncodeVarint64(buffer + index, pst->offset);
  return std::string{buffer, buffer + index};
}

LogRecordPst* DecodeLogRecordPosition(const Bytes& bytes) {
  size_t index = 0;
  uint32_t fid;
  uint64_t offset;
  index += DecodeVarint32(bytes.data() + index, &fid);
  index += DecodeVarint64(bytes.data() + index, &offset);
  return new LogRecordPst{.fid = fid, .offset = offset};
}

}  // namespace bitdb::data