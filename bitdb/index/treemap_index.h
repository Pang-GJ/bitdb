#pragma once
#include <shared_mutex>
#include "bitdb/data/log_record.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/index/index.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::index {

class TreeIndexer : public Indexer {
 public:
  bool Put(const Bytes& key, data::LogRecordPst* pos) override;
  data::LogRecordPst* Get(const Bytes& key) override;
  bool Delete(const Bytes& key, data::LogRecordPst** pos) override;
 
 private:
  std::shared_mutex rwlock_;  
  ds::TreeMap<std::string, data::LogRecordPst*> ds_;
};

}  // namespace bitdb::index