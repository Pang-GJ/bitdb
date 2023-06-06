#include "bitdb/index/treemap_index.h"
#include <mutex>
#include "bitdb/data/log_record.h"

namespace bitdb::index {

bool TreeIndexer::Put(const Bytes& key, data::LogRecordPst* pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  ds_[key] = pos;
  return true;
}

data::LogRecordPst* TreeIndexer::Get(const Bytes& key) {
  std::shared_lock<std::shared_mutex> lock(rwlock_);
  if (ds_.count(key) != 0) {
    return ds_.at(key);
  }
  return nullptr;
}

bool TreeIndexer::Delete(const Bytes& key, data::LogRecordPst** pos) {
  std::unique_lock<std::shared_mutex> lock(rwlock_);
  if (pos != nullptr && ds_.count(key) != 0) {
    *pos = (ds_.at(key));
  }
  return ds_.erase(key) == 1;
}

}  // namespace bitdb::index
