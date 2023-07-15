#include "bitdb/common/logger.h"
#include "bitdb/data/transaction_record.h"
#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/data/log_record.h"

TEST_CASE("encode and decode log_record") {
  for (auto i = 1; i <= 10000; ++i) {
    char ch = 'a' + (i % 25);
    std::string str(i, ch);
    uint32_t crc32;

    bitdb::data::LogRecord log_record{str, str, bitdb::data::NormalLogRecord};
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

std::string GetTestKey(int i) { return bitdb::Format("bitdb-key-{}", i); }

TEST_CASE("encode and decode log_record with txnsaction id") {
  for (size_t i = 0; i < 1000000; ++i) {
    std::string real_key = GetTestKey(i);
    auto encode_key = bitdb::data::EncodeLogRecordWithTxnID(real_key, i);
    auto [decode_real_key, decode_txn_id] =
        bitdb::data::DecodeLogRecordWithTxnID(encode_key);
    CHECK_EQ(i, decode_txn_id);
    CHECK_EQ(decode_real_key, real_key);
  }
}