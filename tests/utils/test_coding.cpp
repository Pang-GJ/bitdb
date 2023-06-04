#include <cstdint>
#include "bitdb/data/log_record.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/utils/coding.h"

TEST_CASE("encode and decode single fixed32") {
  for (uint32_t i = 0; i < 1000000; ++i) {
    char buf[4];
    bitdb::EncodeFixed32(buf, i);
    auto decode_val = bitdb::DecodeFixed32(buf);
    CHECK_EQ(i, decode_val);
  }
}

TEST_CASE("encode and decode single variant32") {
  for (uint32_t i = 500000; i < 1000000; ++i) {
    char buf[4];
    auto encode_len = bitdb::EncodeVarint32(buf, i);
    uint32_t decode_val;
    auto decode_len = bitdb::DecodeVarint32(buf, &decode_val);
    CHECK_EQ(encode_len, decode_len);
    CHECK_EQ(i, decode_val);
  }
}

TEST_CASE("encode and decode multiple variant32") {
  for (uint32_t i = 500000; i < 1000000; ++i) {
    char buf[10];
    buf[9] = 'a';
    buf[8] = '1';
    uint32_t encode_val1 = i;
    uint32_t encode_val2 = i * 10;
    auto encode_len1 = bitdb::EncodeVarint32(buf, encode_val1);
    auto encode_len2 = bitdb::EncodeVarint32(buf + encode_len1, encode_val2);

    uint32_t decode_val1;
    uint32_t decode_val2;
    auto decode_len1 = bitdb::DecodeVarint32(buf, &decode_val1);
    CHECK_EQ(encode_len1, decode_len1);
    CHECK_EQ(encode_val1, decode_val1);

    auto decode_len2 = bitdb::DecodeVarint32(buf + decode_len1, &decode_val2);
    CHECK_EQ(encode_len2, decode_len2);
    CHECK_EQ(encode_val2, decode_val2);
  }
}