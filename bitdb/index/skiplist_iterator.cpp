#include "bitdb/index/skiplist_iterator.h"
#include "bitdb/index/skiplist_index.h"

namespace bitdb::index {

SkipListIndexerIterator::SkipListIndexerIterator(
    SkipListIndexer* skiplist_index)
    : iter_(*skiplist_index->GetIternalDataStructure()) {}

bool SkipListIndexerIterator::Valid() const { return iter_.Valid(); }

std::string SkipListIndexerIterator::Key() const { return iter_.Key(); }

data::LogRecordPst* SkipListIndexerIterator::Value() const {
  return iter_.Value();
}

void SkipListIndexerIterator::Next() { iter_.Next(); }

void SkipListIndexerIterator::Prev() { iter_.Prev(); }

void SkipListIndexerIterator::Seek(const Bytes& target) {
  iter_.Seek(target.data());
}

void SkipListIndexerIterator::SeekToFirst() { iter_.SeekToFirst(); }

void SkipListIndexerIterator::SeekToLast() { iter_.SeekToLast(); }

}  // namespace bitdb::index