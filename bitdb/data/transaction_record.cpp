#include "bitdb/data/transaction_record.h"
#include <cstdint>
#include "bitdb/utils/coding.h"

namespace bitdb::data {

std::string EncodeLogRecordWithTxnID(const Bytes& key, uint64_t txn_id) {
  char buf[K_MAX_VARINT32_LEN];
  auto encode_len = EncodeVarint64(buf, txn_id);
  std::string encoded_key;
  encoded_key.resize(encode_len);
  for (int i = 0; i < encode_len; ++i) {
    encoded_key[i] = buf[i];
  }
  encoded_key += key.data();
  return encoded_key;
}

std::pair<std::string, uint64_t> DecodeLogRecordWithTxnID(Bytes key) {
  const auto* buf = key.data();
  uint64_t txn_id = 0;
  auto len = DecodeVarint64(buf, &txn_id);
  key.RemovePrefix(len);
  return {key.ToString(), txn_id};
}

}  // namespace bitdb::data