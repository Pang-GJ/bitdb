#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"
#include "bitdb/common/logger.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/data/log_record.h"

TEST_CASE("encode and decode log_record") {
  for (auto i = 1; i <= 10000; ++i) {
    char ch = 'a' + (i % 25);
    std::string str(i, ch);
    uint32_t crc32;

    bitdb::data::LogRecord log_record{str, str, bitdb::data::LogRecordNormal};
    auto encode_bytes = bitdb::data::EncodeLogRecord(log_record);
    auto [header, header_size] =
        bitdb::data::DecodeLogRecordHeader(encode_bytes);
    crc32 = bitdb::data::GetLogRecordCRC(
        log_record,
        std::string(encode_bytes.begin(), encode_bytes.begin() + header_size));
    CHECK_NE(0, header_size);
    CHECK_EQ(log_record.key.size(), header.key_size);
    CHECK_EQ(log_record.value.size(), header.value_size);
    CHECK_EQ(crc32, header.crc);
  }
}