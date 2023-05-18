#include "bitdb/index/skiplist_index.h"
#include <mutex>
#include <shared_mutex>

namespace bitdb::index {

bool SkipListIndexer::Put(const Bytes& key, data::LogRecordPst* pos) {
  ds_.Insert(key, pos);
  return true;
}
data::LogRecordPst* SkipListIndexer::Get(const Bytes& key) {
  auto* res = ds_.Find(key);
  if (res == nullptr) {
    return nullptr;
  }
  return *res;
}
bool SkipListIndexer::Delete(const Bytes& key) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  return ds_.Remove(key);
}

}  // namespace bitdb::index