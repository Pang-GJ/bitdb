#include <shared_mutex>
#include "bitdb/ds/skiplist.h"
#include "bitdb/index/index.h"
#include "bitdb/index/iterator.h"
#include "bitdb/index/skiplist_iterator.h"

namespace bitdb::index {

class SkipListIndexer : public Indexer {
 public:
  bool Put(const Bytes& key, data::LogRecordPst* pos) override;
  data::LogRecordPst* Get(const Bytes& key) override;
  bool Delete(const Bytes& key, data::LogRecordPst** pos) override;
  size_t Size() const override;
  ds::SkipList<std::string, data::LogRecordPst*>* GetIternalDataStructure() {
    return &ds_;
  }
  index::Iterator* Iterator() override;

 private:
  std::shared_mutex rwlock_;
  ds::SkipList<std::string, data::LogRecordPst*> ds_;
  index::Iterator* index_iter_;
};

}  // namespace bitdb::index