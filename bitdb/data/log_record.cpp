#include "bitdb/data/log_record.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/coding.h"
#include "bitdb/utils/defer.h"
#include "bitdb/utils/logger.h"
#include "crc32c/crc32c.h"

namespace bitdb::data {

std::string EncodeLogRecord(const LogRecord& log_record, uint32_t* crc32_ptr,
                            char** header_buf, size_t* header_buf_sz) {
  if (log_record.key.empty()) {
    LOG_ERROR("The log record's key is empty");
    return {};
  }
  char header[K_MAX_LOG_RECORD_HEADER_SIZE];
  // type 是 int8，刚好是 char 的大小
  header[4] = static_cast<char>(log_record.type);
  auto header_idx = 5;
  // 5字节以之后存储的是 key 和 value 的长度信息
  // 使用变长变量，节省空间
  header_idx += EncodeVarint32(header + header_idx,
                               static_cast<uint32_t>(log_record.key.size()));
  header_idx += EncodeVarint32(header + header_idx,
                               static_cast<uint32_t>(log_record.value.size()));
  auto size = header_idx + log_record.key.size() + log_record.value.size();
  // char encode_bytes[size + 1];
  auto encode_bytes = new char[size + 1];
  defer { delete[] encode_bytes; };

  std::memcpy(encode_bytes, header, header_idx);
  std::memcpy(encode_bytes + header_idx, log_record.key.c_str(),
              log_record.key.size());
  std::memcpy(encode_bytes + header_idx + log_record.key.size(),
              log_record.value.c_str(), log_record.value.size());

  std::string res{encode_bytes};
  if (res.empty()) {
    LOG_ERROR(
        "encode bytes is empty, but header_idx + key_size + value.size = {}",
        size);
  }
  // 计算 crc32
  auto crc32 = crc32c::Crc32c(encode_bytes + K_CRC32_SIZE, size - K_CRC32_SIZE);
  EncodeFixed32(encode_bytes, crc32);
  for (auto i = 0; i < K_CRC32_SIZE; ++i) {
    res[i] = encode_bytes[i];
  }
  return res;
}

std::pair<std::unique_ptr<LogRecordHeader>, uint32_t> DecodeLogRecordHeader(
    const Bytes& bytes) {
  if (bytes.size() < 4) {
    LOG_ERROR("Log Record Header buf size less than 4.");
    return std::make_pair(nullptr, 0);
  }

  auto header = std::make_unique<LogRecordHeader>(
      DecodeFixed32(bytes.data()), static_cast<LogRecordType>(bytes[4]));

  auto header_idx = 5;
  // 取出 key /val 对应的 size
  uint32_t key_size;
  uint32_t value_size;
  header_idx += DecodeVarint32(bytes.data() + header_idx, &key_size);
  header_idx += DecodeVarint32(bytes.data() + header_idx, &value_size);
  header->key_size = key_size;
  header->value_size = value_size;
  return std::make_pair(std::move(header), header_idx);
}

uint32_t GetLogRecordCRC(const LogRecord& log_record,
                         const std::string& header) {
  if (header.empty()) {
    return 0;
  }
  auto crc32 = crc32c::Crc32c(header.data() + K_CRC32_SIZE,
                              header.size() - K_CRC32_SIZE);
  crc32 = crc32c::Extend(
      crc32, reinterpret_cast<const uint8_t*>(log_record.key.data()),
      log_record.key.size());
  crc32 = crc32c::Extend(
      crc32, reinterpret_cast<const uint8_t*>(log_record.value.data()),
      log_record.value.size());
  return crc32;
}
}  // namespace bitdb::data