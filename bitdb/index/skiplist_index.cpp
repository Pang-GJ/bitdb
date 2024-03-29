#include "bitdb/index/skiplist_index.h"
#include <mutex>
#include <shared_mutex>
#include "bitdb/data/log_record.h"

namespace bitdb::index {

bool SkipListIndexer::Put(const Bytes& key, data::LogRecordPst* pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  ds_.Insert(key.data(), pos);
  return true;
}
data::LogRecordPst* SkipListIndexer::Get(const Bytes& key) {
  std::shared_lock<std::shared_mutex> lock(rwlock_);
  auto* res = ds_.Find(key.data());
  if (res == nullptr) {
    return nullptr;
  }
  return *res;
}
bool SkipListIndexer::Delete(const Bytes& key, data::LogRecordPst** pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  auto res = ds_.Remove(key.data(), pos);
  return res;
}

size_t SkipListIndexer::Size() const { return ds_.Size(); }

index::Iterator* SkipListIndexer::Iterator() {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  if (index_iter_ == nullptr) {
    index_iter_ = new SkipListIndexerIterator(this);
  }
  return index_iter_;
}

}  // namespace bitdb::index