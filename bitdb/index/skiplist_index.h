#include <shared_mutex>
#include "bitdb/ds/skiplist.h"
#include "bitdb/index/index.h"

namespace bitdb::index {

class SkipListIndexer : public Indexer {
 public:
  bool Put(const Bytes& key, data::LogRecordPst* pos) override;
  data::LogRecordPst* Get(const Bytes& key) override;
  bool Delete(const Bytes& key, data::LogRecordPst** pos) override;

 private:
  std::shared_mutex rwlock_;  
  ds::SkipList<Bytes, data::LogRecordPst*> ds_;
};

}  // namespace bitdb::index