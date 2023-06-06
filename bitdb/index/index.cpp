#include "bitdb/index/index.h"
#include <memory>
#include "bitdb/index/hashmap_index.h"
#include "bitdb/index/skiplist_index.h"
#include "bitdb/index/treemap_index.h"

namespace bitdb::index {

std::unique_ptr<Indexer> NewIndexer(IndexerType type) {
  switch (type) {
    case HashMapIndex:
      return std::make_unique<HashIndexer>();
    case TreeMapIndex:
      return std::make_unique<TreeIndexer>();
    case SkipListIndex:
      return std::make_unique<SkipListIndexer>();
  }
  return nullptr;
}

}  // namespace bitdb::index