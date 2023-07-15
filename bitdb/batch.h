#pragma once

#include <mutex>
#include <string>
#include "bitdb/data/log_record.h"
#include "bitdb/db.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/options.h"
#include "bitdb/status.h"

namespace bitdb {

class WriteBatch {
 public:
  WriteBatch(DB* db, const WriteBatchOptions& options)
      : db_(db), options_(options) {}
  Status Put(const Bytes& key, const Bytes& value);
  Status Delete(const Bytes& key);
  Status Commit();
  Status Rollback();

 private:
  std::mutex mtx_;
  DB* db_;
  WriteBatchOptions options_;
  ds::TreeMap<std::string, data::LogRecord> pending_writes_;
  bool committed_{false};
  bool rollbacked_{false};
};

}  // namespace bitdb