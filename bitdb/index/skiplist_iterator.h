#pragma once

#include <string>
#include "bitdb/data/log_record.h"
#include "bitdb/ds/skiplist.h"
#include "bitdb/index/iterator.h"

namespace bitdb::index {

class SkipListIndexer;

class SkipListIndexerIterator : public Iterator {
 public:
  explicit SkipListIndexerIterator(SkipListIndexer* skiplist_index);

  bool Valid() const override;
  std::string Key() const override;
  data::LogRecordPst* Value() const override;
  void Next() override;
  void Prev() override;
  void Seek(const Bytes& target) override;
  void SeekToFirst() override;
  void SeekToLast() override;

 private:
  ds::SkipList<std::string, data::LogRecordPst*>::Iterator iter_;
};

}  // namespace bitdb::index