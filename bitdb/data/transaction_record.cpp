#include "bitdb/data/transaction_record.h"
#include <cstdint>
#include "bitdb/utils/coding.h"

namespace bitdb::data {

std::string EncodeLogRecordWithTranID(const Bytes& key, uint32_t tran_id) {
  char buf[K_MAX_VARINT32_LEN];
  auto encode_len = EncodeVarint32(buf, tran_id);
  std::string encoded_key;
  encoded_key.resize(encode_len);
  for (int i = 0; i < encode_len; ++i) {
    encoded_key[i] = buf[i];
  }
  encoded_key += key.data();
  return encoded_key;
}

std::pair<std::string, uint32_t> DecodeLogRecordWithTranID(Bytes key) {
  const auto* buf = key.data();
  uint32_t tran_id = 0;
  auto len = DecodeVarint32(buf, &tran_id);
  key.RemovePrefix(len);
  return {key.ToString(), tran_id};
}

}  // namespace bitdb::data