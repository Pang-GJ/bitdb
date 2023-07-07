#pragma once

#include "bitdb/data/log_record.h"

namespace bitdb::data {

struct TransactionRecord {
  LogRecord log_record;  // NOLINT
  LogRecordPst* pst;     // NOLINT
};

// 表示这不是一个事务的id
// TODO(pangguojian): 这里需要 64 吗？还是 32 够了
constexpr uint32_t K_NON_TRAN_ID = 0;

extern const std::string K_TRANSACTION_FINISHED_KEY;

std::string EncodeLogRecordWithTranID(const Bytes& key, uint32_t tran_id);

// record decoded_key, tran_id
std::pair<std::string, uint32_t> DecodeLogRecordWithTranID(Bytes key);

}  // namespace bitdb::data