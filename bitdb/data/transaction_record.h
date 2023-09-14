#pragma once

#include "bitdb/data/log_record.h"

namespace bitdb::data {

struct TxnRecord {
  LogRecord log_record;  // NOLINT
  LogRecordPst* pst;     // NOLINT
};

// 表示这不是一个事务的id
constexpr uint64_t K_NON_TXN_ID = 0;

constexpr std::string_view K_TXN_FINISHED_KEY = "Txn-finished";

std::string EncodeLogRecordWithTxnID(const Bytes& key, uint64_t txn_id);

// record decoded_key, txn_id
std::pair<std::string, uint64_t> DecodeLogRecordWithTxnID(Bytes key);

}  // namespace bitdb::data