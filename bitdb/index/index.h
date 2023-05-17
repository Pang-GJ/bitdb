#pragma once

#include "bitdb/data/log_record.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::index {

class Indexer {
 public:
  virtual bool Put(const Bytes& key, data::LogRecordPst* pos) = 0;
  virtual data::LogRecordPst* Get(const Bytes& key) = 0;
  virtual bool Delete(const Bytes& key) = 0;
};

}  // namespace bitdb::index