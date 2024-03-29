#pragma once

#include <memory>
#include "bitdb/data/log_record.h"
#include "bitdb/index/iterator.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::index {

class Indexer {
 public:
  virtual ~Indexer() = default;
  virtual bool Put(const Bytes& key, data::LogRecordPst* pos) = 0;
  virtual data::LogRecordPst* Get(const Bytes& key) = 0;
  /**
   * @brief 删除 key value，并返回被删除的value
   *
   * @param key
   * @param pos
   * @return true
   * @return false
   */
  virtual bool Delete(const Bytes& key, data::LogRecordPst** pos) = 0;

  virtual size_t Size() const = 0;

  virtual index::Iterator* Iterator() = 0;
};

// 内存索引类型
enum IndexerType {
  HashMapIndex = 1,
  TreeMapIndex,
  SkipListIndex,
};

// 工厂接口，支持多种索引数据结构
std::unique_ptr<Indexer> NewIndexer(IndexerType type);

}  // namespace bitdb::index