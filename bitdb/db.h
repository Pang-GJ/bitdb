#pragma once

#include <shared_mutex>
#include "bitdb/data/data_file.h"
#include "bitdb/data/log_record.h"
#include "bitdb/ds/hashmap.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/index/index.h"
#include "bitdb/options.h"
#include "bitdb/status.h"
namespace bitdb {

class DB {
 public:
  Status Put(const Bytes& key, const Bytes& value);
  Status Get(const Bytes& key, std::string* value);

 private:

  
  Status AppendLogRecord(const data::LogRecord& log_record, data::LogRecordPst* pst);
  Status NewActiveDataFile();

  std::shared_mutex rwlock_;
  Options options_;
  std::unique_ptr<data::DataFile> active_file_;
  ds::HashMap<uint32_t, std::unique_ptr<data::DataFile>> older_files_;
  index::Indexer* index_;
};

}  // namespace bitdb