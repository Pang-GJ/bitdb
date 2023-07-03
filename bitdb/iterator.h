#pragma once

#include <shared_mutex>
#include "bitdb/common/logger.h"
#include "bitdb/db.h"
#include "bitdb/index/iterator.h"
#include "bitdb/status.h"

namespace bitdb {

class Iterator {
 public:
  Iterator(index::Iterator* iter, DB* db) : iter_(iter), db_(db) {}

  bool Valid() const { return iter_->Valid(); }

  std::string Key() const { return iter_->Key(); }

  std::string Value() {
    auto* pst = iter_->Value();
    std::shared_lock lock(db_->rwlock_);
    std::string value;
    auto status = db_->GetValueByLogRecordPst(pst, Key(), &value);
    if (!status.IsOk()) {
      LOG_ERROR("Iterator Value error, status: {}", status.ToString());
    }
    return value;
  }

  void Next() { iter_->Next(); }

  void Prev() { iter_->Prev(); }

  void Seek(const Bytes& target) { iter_->Seek(target); }

  void SeekToFirst() { iter_->SeekToFirst(); }

  void SeekToLast() { iter_->SeekToLast(); }

 private:
  index::Iterator* iter_;
  DB* db_;
};

}  // namespace bitdb