#pragma once
#include <shared_mutex>
#include "bitdb/data/log_record.h"
#include "bitdb/ds/hashmap.h"
#include "bitdb/index/index.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::index {

class HashIndexer : public Indexer {
 public:
  bool Put(const Bytes& key, data::LogRecordPst* pos) override;
  data::LogRecordPst* Get(const Bytes& key) override;
  bool Delete(const Bytes& key, data::LogRecordPst** pos) override;

 private:
  std::shared_mutex rwlock_;
  ds::HashMap<std::string, data::LogRecordPst*> ds_;
};

}  // namespace bitdb::index