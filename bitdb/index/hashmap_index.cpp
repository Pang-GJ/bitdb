#include "bitdb/index/hashmap_index.h"
#include <mutex>
#include "bitdb/data/log_record.h"

namespace bitdb::index {

bool HashIndexer::Put(const Bytes& key, data::LogRecordPst* pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  ds_[key.data()] = pos;
  return true;
}
data::LogRecordPst* HashIndexer::Get(const Bytes& key) {
  std::shared_lock<std::shared_mutex> lock(rwlock_);
  if (ds_.count(key.data()) != 0) {
    return ds_.at(key.data());
  }
  return nullptr;
}
bool HashIndexer::Delete(const Bytes& key, data::LogRecordPst** pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  if (pos != nullptr && ds_.count(key.data()) != 0) {
    *pos = (ds_.at(key.data()));
  }
  return ds_.erase(key.data()) == 1;
}

}  // namespace bitdb::index