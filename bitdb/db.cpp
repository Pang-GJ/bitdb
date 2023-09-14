#include "bitdb/db.h"
#include <dirent.h>
#include <sys/types.h>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>
#include "bitdb/batch.h"
#include "bitdb/data/data_file.h"
#include "bitdb/data/log_record.h"
#include "bitdb/data/transaction_record.h"
#include "bitdb/ds/treemap.h"
#include "bitdb/index/index.h"
#include "bitdb/iterator.h"
#include "bitdb/options.h"
#include "bitdb/status.h"
#include "bitdb/utils/defer.h"
#include "bitdb/utils/os_utils.h"
#include "bitdb/utils/string_utils.h"

namespace bitdb {
DB::DB(Options options)
    : options_(std::move(options)),
      index_(index::NewIndexer(options_.index_type)) {}

Status DB::Open(const Options& options, DB** db_ptr, bool merge_after_open) {
  *db_ptr = nullptr;
  // 检查配置项
  CHECK_OK(CheckOptions(options));
  // 判断数据目录是否存在，不存在则新建
  CHECK_TRUE(CheckOrCreateDirectory(options.dir_path));
  *db_ptr = new DB(options);
  auto& db = **db_ptr;
  CHECK_OK(db.LoadMergenceFiles());
  CHECK_OK(db.LoadDataFiles());
  CHECK_OK(db.LoadIndexFromHintFile());
  CHECK_OK(db.LoadIndexFromDataFiles());
  if (merge_after_open) {
    CHECK_OK(db.Merge());
  }
  return Status::Ok();
}

Status DB::Close(DB** db_ptr) {
  if (db_ptr == nullptr || (*db_ptr) == nullptr) {
    return Status::InvalidArgument("DB::Close", "db is null");
  }
  delete (*db_ptr);
  *db_ptr = nullptr;
  return Status::Ok();
}

Status DB::Put(const Bytes& key, const Bytes& value) {
  if (key.empty()) {
    return Status::InvalidArgument("DB::Put", "key is empty");
  }

  data::LogRecord log_record{
      data::EncodeLogRecordWithTxnID(key, data::K_NON_TXN_ID), value.data(),
      data::NormalLogRecord};
  auto* pst = new data::LogRecordPst;
  std::unique_lock lock(rwlock_);
  CHECK_OK(AppendLogRecord(log_record, pst));
  CHECK_TRUE(index_->Put(key, pst),
             Format("fail to put index of key: {}", key.ToString()));
  return Status::Ok();
}

Status DB::Get(const Bytes& key, std::string* value) {
  std::shared_lock lock(rwlock_);
  if (key.empty()) {
    return Status::InvalidArgument("DB::Get", "key is empty");
  }

  // 从索引中取出 key 对应的索引信息
  // TODO(pangguojian): update index->Get() to bool Get(const Bytes& key, V
  // *value);
  auto log_record_pst = index_->Get(key);
  if (log_record_pst == nullptr) {
    return Status::NotFound("index",
                            "index of key " + key.ToString() + " not_found");
  }
  CHECK_OK(GetValueByLogRecordPst(log_record_pst, key, value));
  return Status::Ok();
}

Status DB::Delete(const Bytes& key) {
  if (key.empty()) {
    return Status::InvalidArgument("DB::Delete", "key is empty");
  }
  std::unique_lock lock(rwlock_);
  auto* pst = index_->Get(key);
  if (pst == nullptr) {
    return Status::Ok("DB::Delete",
                      Format("key: {} don't exist.", key.ToString()));
  }
  data::LogRecord log_record{
      .key = data::EncodeLogRecordWithTxnID(key, data::K_NON_TXN_ID),
      .type = data::DeletedLogRecord};
  CHECK_OK(AppendLogRecord(log_record, pst));
  CHECK_TRUE(index_->Delete(key, &pst),
             Format("index delete key: {} failed.", key.ToString()));
  delete pst;
  return Status::Ok();
}

Status DB::NewWriteBach(WriteBatch** wb_ptr, const WriteBatchOptions& options) {
  if (wb_ptr == nullptr) {
    return Status::InvalidArgument("DB::NewWriteBatch", "wb is nullptr");
  }
  *wb_ptr = nullptr;
  *wb_ptr = new WriteBatch(this, options);
  return Status::Ok();
}

Status DB::NewIterator(std::unique_ptr<Iterator>* iter) {
  auto* index_iter = index_->Iterator();
  if (index_iter == nullptr) {
    return Status::NotSupported(
        "DB::NewIterator", "this kind of indexer didn't support iterator now");
  }
  *iter = std::make_unique<Iterator>(index_iter, this);
  return Status::Ok();
}

Status DB::ListKeys(std::vector<std::string>* keys) {
  auto* index_iter = index_->Iterator();
  if (index_iter == nullptr) {
    return Status::NotSupported(
        "DB::NewIterator", "this kind of indexer didn't support iterator now");
  }
  keys->resize(index_->Size());
  size_t idx = 0;
  for (index_iter->SeekToFirst(); index_iter->Valid(); index_iter->Next()) {
    (*keys)[idx] = index_iter->Key();
    ++idx;
  }
  return Status::Ok();
}

Status DB::Sync() {
  std::unique_lock lock(rwlock_);
  return active_file_->Sync();
}

Status DB::Fold(const UserOperationFunc& fn) {
  std::shared_lock lock(rwlock_);
  auto* index_iter = index_->Iterator();
  if (index_iter == nullptr) {
    return Status::NotSupported(
        "DB::NewIterator", "this kind of indexer didn't support iterator now");
  }
  for (index_iter->SeekToFirst(); index_iter->Valid(); index_iter->Next()) {
    auto key = index_iter->Key();
    std::string value;
    CHECK_OK(GetValueByLogRecordPst(index_iter->Value(), key, &value));
    if (!fn(key, value)) {
      break;
    }
  }
  return Status::Ok();
}

Status DB::Merge() {
  if (this->active_file_ == nullptr) {
    return Status::Ok();
  }
  std::unique_lock lock(this->rwlock_);

  // 同时只能处于一个 merge 过程中
  if (is_merging_) {
    return Status::CheckError(
        "DB::Merge",
        "merging is in process, one time for one merging, try again later");
  }
  is_merging_ = true;
  defer { is_merging_ = false; };

  // sync 当前的 activate data file
  // CHECK_OK(Sync()); // 不能用这个，会死锁，但又没有递归读写锁
  CHECK_OK(this->active_file_->Sync());

  // 现在的 activate data file 可以设置为不是活跃的了
  // 当当前的活跃文件转换为旧的活跃文件
  auto non_merged_file_id = active_file_->file_id;
  older_files_[active_file_->file_id] = std::move(active_file_);
  // 打开新的活跃文件
  CHECK_OK(NewActiveDataFile());

  using DataFilePtr = std::unique_ptr<data::DataFile>*;
  std::vector<DataFilePtr> files_to_be_merged;
  for (auto& pair : older_files_) {
    files_to_be_merged.emplace_back(&pair.second);
  }
  // 后面的部分不会有冲突了，解锁
  // lock.unlock();

  std::sort(files_to_be_merged.begin(), files_to_be_merged.end(),
            [](const DataFilePtr a, const DataFilePtr b) {
              return (*a)->file_id < (*b)->file_id;
            });

  auto merge_dir = data::GetMergenceDirectory(options_.dir_path);
  // 去除之前 merge 留下的目录
  if (IsFileExist(merge_dir)) {
    RemoveDir(merge_dir);
  }
  CHECK_TRUE(CheckOrCreateDirectory(merge_dir));

  auto mergence_options = options_;
  mergence_options.dir_path = merge_dir;
  mergence_options.is_sync_write = false;
  // temp DB
  // 它的作用其实是相当于新建一个文件夹，用来暂存合并后的数据文件
  DB* temp_db = nullptr;
  CHECK_OK(DB::Open(mergence_options, &temp_db));
  CHECK_NOT_NULL_STATUS(temp_db);

  // open hint file
  std::unique_ptr<data::DataFile> hint_file;
  CHECK_OK(data::DataFile::OpenHintFile(merge_dir, &hint_file));

  for (const auto* file_ptr : files_to_be_merged) {
    uint64_t offset = 0;
    while (true) {
      data::LogRecord log_record;
      size_t sz;
      CHECK_OK((*file_ptr)->ReadLogRecord(offset, &log_record, &sz));
      if (sz == 0) {
        break;
      }
      auto real_key = data::DecodeLogRecordWithTxnID(log_record.key).first;
      auto* pst = this->index_->Get(real_key);
      if (pst != nullptr && pst->fid == (*file_ptr)->file_id &&
          pst->offset == offset) {
        log_record.key =
            data::EncodeLogRecordWithTxnID(real_key, data::K_NON_TXN_ID);
        data::LogRecordPst tmp_pst;
        CHECK_OK(temp_db->AppendLogRecord(log_record, &tmp_pst));
        // 把当前位置写到 hint file 中
        CHECK_OK(data::WriteHintRecord(hint_file.get(), real_key, tmp_pst));
      }
      offset += sz;
    }
  }

  // sync the hint file and the temp db
  CHECK_OK(hint_file->Sync());
  CHECK_OK(temp_db->Sync());
  CHECK_OK(DB::Close(&temp_db));

  std::unique_ptr<data::DataFile> merged_file = nullptr;
  CHECK_OK(data::DataFile::OpenMergedFile(merge_dir, &merged_file));
  data::LogRecord merge_logrecord{.key = data::K_MERGED_LOG_RECORD_KEY.data(),
                                  .value = std::to_string(non_merged_file_id)};
  CHECK_OK(merged_file->Write(data::EncodeLogRecord(merge_logrecord)));
  CHECK_OK(merged_file->Sync());
  return Status::Ok();
}

Status DB::AppendLogRecord(const data::LogRecord& log_record,
                           data::LogRecordPst* pst) {
  if (pst == nullptr) {
    return Status::InvalidArgument("DB::AppendLogRecord",
                                   "pst should not be nullptr");
  }

  if (active_file_ == nullptr) {
    CHECK_OK(NewActiveDataFile());
  }

  // 写入数据编码
  auto encode_bytes = data::EncodeLogRecord(log_record);
  // 大于 DataFileSize 则先持久化到磁盘
  if (active_file_->write_off + encode_bytes.size() > options_.data_file_size) {
    // 持久化数据文件, 保证现在的活跃数据文件持久化到磁盘
    CHECK_OK(active_file_->Sync());
    // 当当前的活跃文件转换为旧的活跃文件
    older_files_[active_file_->file_id] = std::move(active_file_);
    // 打开新的活跃文件
    CHECK_OK(NewActiveDataFile());
  }

  auto write_off = active_file_->write_off;
  CHECK_OK(active_file_->Write(encode_bytes));
  // 根据用户配置决定是否每次都持久化
  if (options_.is_sync_write) {
    CHECK_OK(active_file_->Sync());
  }

  pst->fid = active_file_->file_id;
  pst->offset = write_off;
  return Status::Ok();
}

Status DB::NewActiveDataFile() {
  // 打开新的数据文件
  std::unique_ptr<data::DataFile> data_file;
  CHECK_OK(data::DataFile::OpenDataFile(options_.dir_path, next_file_id_,
                                        &data_file));
  CHECK_NOT_NULL_STATUS(data_file.get());
  ++next_file_id_;
  active_file_ = std::move(data_file);
  return Status::Ok();
}

Status DB::LoadDataFiles() {
  DIR* dir_ptr;
  struct dirent* dp;
  dir_ptr = opendir(options_.dir_path.c_str());
  if (dir_ptr == nullptr) {
    return Status::InvalidArgument("DB::LoadDataFiles", "failed to open dir.");
  }
  std::vector<uint32_t> filed_ids;
  while ((dp = readdir(dir_ptr)) != nullptr) {
    if (EndsWith(dp->d_name, data::K_DATA_FILE_SUFFIX)) {
      auto split_names = Split(dp->d_name, '.');
      if (split_names.size() != 2) {
        return Status::Corruption("DB::LoadDataFiles",
                                  "database dir maybe corrupted");
      }
      const auto file_id = std::stoi(split_names[0].data());
      filed_ids.emplace_back(file_id);
    }
  }
  // 对文件 id 进行排序，从小到大依次加载
  std::sort(filed_ids.begin(), filed_ids.end());
  file_ids_ = std::make_unique<std::vector<uint32_t>>(filed_ids.begin(),
                                                      filed_ids.end());

  // 遍历每个文件id，打开对应的数据文件
  const auto sz = file_ids_->size();
  for (size_t i = 0; i < sz; ++i) {
    const auto& fid = file_ids_->at(i);
    std::unique_ptr<data::DataFile> data_file;
    CHECK_OK(data::DataFile::OpenDataFile(options_.dir_path, fid, &data_file));
    if (i == sz - 1) {
      active_file_ = std::move(data_file);
    } else {
      older_files_[fid] = std::move(data_file);
    }
  }
  next_file_id_ = active_file_ != nullptr ? active_file_->file_id + 1 : 0;
  return Status::Ok();
}

Status DB::LoadMergenceFiles() {
  auto merge_dir = data::GetMergenceDirectory(options_.dir_path);
  DIR* dir_ptr;
  struct dirent* dp;
  dir_ptr = opendir(merge_dir.c_str());
  if (dir_ptr == nullptr) {
    return Status::Ok("DB::LoadMergenceFiles", "have no mergence files");
  }
  defer { RemoveDir(merge_dir); };
  bool mergence_finished = false;
  std::vector<std::string> merged_file_names;
  while ((dp = readdir(dir_ptr)) != nullptr) {
    if (strcmp(dp->d_name, data::K_MERGED_FILE_NAME.data()) == 0) {
      mergence_finished = true;
    }
    merged_file_names.emplace_back(dp->d_name);
  }
  if (!mergence_finished) {
    return Status::Ok("DB::LoadMergenceFiles", "mergence is not finished");
  }

  uint32_t non_merged_file_id;
  CHECK_OK(GetNonMergedFileID(merge_dir, &non_merged_file_id));
  for (uint32_t file_id = 0; file_id < non_merged_file_id; ++file_id) {
    // 删除已经合并的文件
    auto file_path = data::GetDataFileName(options_.dir_path, file_id);
    RemoveFile(file_path);
  }

  // 把 temp_db 目录的数据文件移动到数据目录
  for (const auto& file_name : merged_file_names) {
    auto src_path = Format("{}/{}", merge_dir, file_name);
    auto dst_path = Format("{}/{}", options_.dir_path, file_name);
    CHECK_TRUE(std::rename(src_path.c_str(), dst_path.c_str()) == 0);
  }

  return Status::Ok();
}

Status DB::GetNonMergedFileID(std::string_view directory, uint32_t* file_id) {
  std::unique_ptr<data::DataFile> merged_file = nullptr;
  CHECK_OK(data::DataFile::OpenMergedFile(directory, &merged_file));
  data::LogRecord log_record;
  size_t sz;
  CHECK_OK(merged_file->ReadLogRecord(0, &log_record, &sz));
  *file_id = std::stoi(log_record.value);
  return Status::Ok();
}

Status DB::LoadIndexFromDataFiles() {
  if (file_ids_->empty()) {
    return Status::Ok();
  }

  // 判断是否merge过
  bool has_merged = false;
  uint32_t non_merged_file_id = 0;
  auto merged_file_path =
      Format("{}/{}", options_.dir_path, data::K_MERGED_FILE_NAME);
  if (IsFileExist(merged_file_path)) {
    uint32_t file_id;
    CHECK_OK(GetNonMergedFileID(options_.dir_path, &file_id));
    has_merged = true;
    non_merged_file_id = file_id;
  }

  uint64_t current_txn_id = data::K_NON_TXN_ID;
  ds::TreeMap<uint32_t, std::vector<data::TxnRecord>> txn_records;
  // 遍历所有的文件id，处理其中的日志记录
  const auto& sz = file_ids_->size();
  for (size_t i = 0; i < sz; ++i) {
    const auto& fid = file_ids_->at(i);
    if (has_merged && fid < non_merged_file_id) {
      // merged 过了，则之前的部分不需要加载，可以用 hint file
      continue;
    }

    std::unique_ptr<data::DataFile>* data_file_ptr;
    if (fid == active_file_->file_id) {
      data_file_ptr = &active_file_;
    } else {
      data_file_ptr = &older_files_[fid];
    }

    auto update_index = [&](const Bytes& key, data::LogRecordType type,
                            data::LogRecordPst* pst) -> Status {
      if (type == data::DeletedLogRecord) {
        CHECK_TRUE(this->index_->Delete(key, nullptr));
      } else {
        CHECK_TRUE(this->index_->Put(key, pst));
      }
      return Status::Ok();
    };

    uint64_t offset = 0;
    while (true) {
      data::LogRecord log_record;
      size_t sz;
      CHECK_OK((*data_file_ptr)->ReadLogRecord(offset, &log_record, &sz));
      if (sz == 0) {
        break;
      }

      auto [real_key, txn_id] = data::DecodeLogRecordWithTxnID(log_record.key);

      if (txn_id == data::K_NON_TXN_ID) {
        auto* pst = log_record.type == data::DeletedLogRecord
                        ? nullptr
                        : new data::LogRecordPst{.fid = fid, .offset = offset};
        CHECK_OK(update_index(real_key, log_record.type, pst));
      } else {
        if (log_record.type == data::TxnFinishedLogRecord) {
          for (const auto& txn_record : txn_records[txn_id]) {
            CHECK_OK(update_index(txn_record.log_record.key,
                                  txn_record.log_record.type,
                                  txn_record.pst));
          }
          txn_records.erase(txn_id);
        } else {
          log_record.key = real_key;
          data::TxnRecord txn_record{
              .log_record = log_record,
              .pst = new data::LogRecordPst{.fid = fid, .offset = offset}};
          txn_records[txn_id].emplace_back(txn_record);
        }
      }
      if (txn_id > current_txn_id) {
        current_txn_id = txn_id;
      }

      offset += sz;
    }

    // 如果是 active data file，更新这个文件的 offset
    if (i == sz - 1) {
      active_file_->write_off = offset;
    }
  }
  this->txn_id_ = current_txn_id;
  // 清空 file_ids
  file_ids_->clear();
  return Status::Ok();
}

Status DB::LoadIndexFromHintFile() {
  auto hint_file_name = data::GetHintFileName(options_.dir_path);
  if (!IsFileExist(hint_file_name)) {
    return Status::Ok("DB::LoadIndexFromHintFile", "has no hint file");
  }
  std::unique_ptr<data::DataFile> hint_file;
  CHECK_OK(data::DataFile::OpenHintFile(options_.dir_path, &hint_file));
  uint64_t offset = 0;
  while (true) {
    data::LogRecord log_record;
    size_t sz;
    CHECK_OK(hint_file->ReadLogRecord(offset, &log_record, &sz));
    if (sz == 0) {
      break;
    }
    auto* pst = data::DecodeLogRecordPosition(log_record.value);
    CHECK_TRUE(index_->Put(log_record.key, pst),
               Format("fail to put index of key: {}", log_record.key));
    offset += sz;
  }
  return Status::Ok();
}

Status DB::GetValueByLogRecordPst(data::LogRecordPst* log_record_pst,
                                  const Bytes& key, std::string* value) {
  // 根据文件 id 找到对应的数据文件
  std::unique_ptr<data::DataFile>* data_file_ptr;
  if (log_record_pst->fid == active_file_->file_id) {
    data_file_ptr = &active_file_;
  } else {
    data_file_ptr = &(older_files_[log_record_pst->fid]);
  }

  if (*data_file_ptr == nullptr) {
    return Status::NotFound(
        "DataFile", "data file of key: " + key.ToString() + " Not found");
  }
  data::LogRecord log_record;
  size_t sz;
  CHECK_OK((*data_file_ptr)
               ->ReadLogRecord(log_record_pst->offset, &log_record, &sz));
  if (log_record.type == data::DeletedLogRecord) {
    return Status::NotFound("index",
                            "index of key " + key.ToString() + " not_found");
  }
  *value = log_record.value;
  return Status::Ok();
}

}  // namespace bitdb