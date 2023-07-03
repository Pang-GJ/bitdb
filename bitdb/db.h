#pragma once

#include <functional>
#include <shared_mutex>
#include "bitdb/data/data_file.h"
#include "bitdb/data/log_record.h"
#include "bitdb/ds/hashmap.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/index/index.h"
#include "bitdb/options.h"
#include "bitdb/status.h"
namespace bitdb {

class Iterator;
class DB {
 public:
  explicit DB(Options options);

  /**
   * @brief 打开数据库
   *
   * @param options 配置项
   * @return Status
   */
  static Status Open(const Options& options, DB** db_ptr);
  static Status Close(DB** db_ptr);

  Status Put(const Bytes& key, const Bytes& value);
  Status Get(const Bytes& key, std::string* value);
  Status Delete(const Bytes& key);

  Status NewIterator(std::unique_ptr<Iterator>* iter);

  Status ListKeys(std::vector<std::string>* keys);

  Status Sync();

  /**
   * @brief  Fold retrieves all the data and iteratively executes an
   * user-specified operation (UDF) Once the UDF failed, the iteration will stop
   * intermediatelly
   *
   * @return Status
   */
  using UserOperationFunc =
      std::function<bool(const std::string&, const std::string&)>;
  Status Fold(const UserOperationFunc& fn);

  std::string GetDirPath() const { return options_.dir_path; }
  size_t GetOlderDataFileNum() const { return older_files_.size(); }

 private:
  Status AppendLogRecord(const data::LogRecord& log_record,
                         data::LogRecordPst* pst);
  Status NewActiveDataFile();

  Status LoadDataFiles();
  Status LoadIndexFromDataFiles();

  Status GetValueByLogRecordPst(data::LogRecordPst* log_record_pst,
                                const Bytes& key, std::string* value);

  friend class Iterator;

  std::shared_mutex rwlock_;
  Options options_;
  std::unique_ptr<data::DataFile> active_file_;
  ds::HashMap<uint32_t, std::unique_ptr<data::DataFile>> older_files_;
  std::unique_ptr<index::Indexer> index_;
  std::unique_ptr<std::vector<uint32_t>> file_ids_;  // 仅在加载文件时使用
  uint32_t next_file_id_{0};  // 仅在新建active data file 时使用
};

}  // namespace bitdb