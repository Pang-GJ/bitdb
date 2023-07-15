#include "bitdb/batch.h"
#include <mutex>
#include "bitdb/data/log_record.h"
#include "bitdb/data/transaction_record.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/status.h"

namespace bitdb {

Status WriteBatch::Put(const Bytes& key, const Bytes& value) {
  if (key.empty()) {
    return Status::InvalidArgument("WriteBatch::Put", "key is empty");
  }
  std::unique_lock lock(mtx_);
  this->pending_writes_.emplace(
      key.data(),
      data::LogRecord{key.data(), value.data(), data::NormalLogRecord});
  return Status::Ok();
}

Status WriteBatch::Delete(const Bytes& key) {
  if (key.empty()) {
    return Status::InvalidArgument("DB::Delete", "key is empty");
  }
  if (db_->Get(key, nullptr).IsNotFound()) {
    if (this->pending_writes_.count(key.data()) != 0) {
      this->pending_writes_.erase(key.data());
    }
    return Status::Ok();
  }
  this->pending_writes_.emplace(
      key.data(),
      data::LogRecord{.key = key.data(), .type = data::DeletedLogRecord});
  return Status::Ok();
}

Status WriteBatch::Commit() {
  std::unique_lock lock(mtx_);
  if (this->pending_writes_.empty()) {
    return Status::Ok();
  }
  // too much pending data
  // TODO(pangguojian): early stop, when it match the max batch num, tell the
  // user.
  if (this->pending_writes_.size() > this->options_.max_batch_num) {
    return Status::NotSupported("WriteBatch::Commit",
                                "exceed max batch number");
  }
  ds::TreeMap<std::string, data::LogRecordPst*> positions;
  std::unique_lock db_lock(db_->rwlock_);
  auto txn_id = ++db_->txn_id_;
  for (auto& [key, log_record] : this->pending_writes_) {
    log_record.key = data::EncodeLogRecordWithTxnID(log_record.key, txn_id);
    auto* pst = new data::LogRecordPst;
    CHECK_OK(db_->AppendLogRecord(log_record, pst));
    positions.emplace(key, pst);
  }
  // 增加一个标志事务完成的 logrecord
  data::LogRecord finish_record{
      .key = data::EncodeLogRecordWithTxnID(data::K_TXN_FINISHED_KEY, txn_id),
      .type = data::TxnFinishedLogRecord};
  auto* finish_pst = new data::LogRecordPst;
  CHECK_OK(db_->AppendLogRecord(finish_record, finish_pst));
  // update in-memory index
  for (const auto& [key, log_record] : this->pending_writes_) {
    auto* pst = positions[key];
    if (log_record.type == data::NormalLogRecord) {
      CHECK_TRUE(db_->index_->Put(key, pst));
    } else if (log_record.type == data::DeletedLogRecord) {
      CHECK_TRUE(db_->index_->Delete(key, &pst));
    }
  }
  this->committed_ = true;
  return Status::Ok();
}

Status WriteBatch::Rollback() {
  if (this->committed_) {
    return Status::CheckError(
        "WriteBatch::Rollback()",
        "WriteBatch has been commited, could not rollback.");
  }
  if (this->rollbacked_) {
    return Status::CheckError("WriteBatch::Rollback()",
                              "WriteBatch has been rollbaked.");
  }
  this->pending_writes_.clear();
  this->rollbacked_ = true;
  return Status::Ok();
}

}  // namespace bitdb