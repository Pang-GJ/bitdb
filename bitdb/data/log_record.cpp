#include "bitdb/data/log_record.h"

namespace bitdb::data {

Bytes EncodeLogRecord(const LogRecord& log_record) { return {}; }

std::pair<std::unique_ptr<LogRecordHeader>, int64_t> DecodeLogRecordHeader(
    const Bytes& bytes) {
  return {};
}

uint32_t GetLogRecordCRC(const LogRecord& log_record, const Bytes& header) {
  return 0;
}
}  // namespace bitdb::data