#include "bitdb/index/hashmap_index.h"
#include <mutex>

namespace bitdb::index {

bool HashIndexer::Put(const Bytes& key, data::LogRecordPst* pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  ds_[key] = pos;
  return true;
}
data::LogRecordPst* HashIndexer::Get(const Bytes& key) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  if (ds_.count(key) != 0) {
    return ds_.at(key);
  }
  return nullptr;
}
bool HashIndexer::Delete(const Bytes& key) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  return ds_.erase(key) == 1;
}

}  // namespace bitdb::index