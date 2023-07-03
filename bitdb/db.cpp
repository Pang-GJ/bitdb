#include "bitdb/db.h"
#include <dirent.h>
#include <sys/types.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <utility>
#include "bitdb/data/data_file.h"
#include "bitdb/data/log_record.h"
#include "bitdb/index/index.h"
#include "bitdb/iterator.h"
#include "bitdb/options.h"
#include "bitdb/status.h"
#include "bitdb/utils/os_utils.h"
#include "bitdb/utils/string_utils.h"

namespace bitdb {
DB::DB(Options options)
    : options_(std::move(options)),
      index_(index::NewIndexer(options_.index_type)) {}

Status DB::Open(const Options& options, DB** db_ptr) {
  *db_ptr = nullptr;
  // 检查配置项
  CHECK_OK(CheckOptions(options));
  // 判断数据目录是否存在，不存在则新建
  CHECK_TRUE(CheckOrCreateDirectory(options.dir_path));
  *db_ptr = new DB(options);
  auto& db = **db_ptr;
  CHECK_OK(db.LoadDataFiles());
  CHECK_OK(db.LoadIndexFromDataFiles());
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

  data::LogRecord log_record{key.data(), value.data(), data::LogRecordNormal};
  auto* pst = new data::LogRecordPst;
  CHECK_OK(AppendLogRecord(log_record, pst));
  auto ok = index_->Put(key, pst);
  if (!ok) {
    return Status::Corruption("index put", "failed to update index");
  }
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
  auto* pst = index_->Get(key);
  if (pst == nullptr) {
    return Status::Ok("DB::Delete",
                      Format("key: {} don't exist.", key.ToString()));
  }
  data::LogRecord log_record{.key = key.data(), .type = data::LogRecordDeleted};
  CHECK_OK(AppendLogRecord(log_record, pst));
  CHECK_TRUE(index_->Delete(key, &pst),
             Format("index delete key: {} failed.", key.ToString()));
  delete pst;
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

Status DB::AppendLogRecord(const data::LogRecord& log_record,
                           data::LogRecordPst* pst) {
  std::unique_lock lock(rwlock_);
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
  return Status::Ok();
}

Status DB::LoadIndexFromDataFiles() {
  if (file_ids_->empty()) {
    return Status::Ok();
  }

  // 遍历所有的文件id，处理其中的日志记录
  const auto& sz = file_ids_->size();
  for (size_t i = 0; i < sz; ++i) {
    const auto& fid = file_ids_->at(i);
    std::unique_ptr<data::DataFile>* data_file_ptr;

    if (fid == active_file_->file_id) {
      data_file_ptr = &active_file_;
    } else {
      data_file_ptr = &older_files_[fid];
    }

    int64_t offset = 0;
    while (true) {
      data::LogRecord log_record;
      size_t sz;
      CHECK_OK((*data_file_ptr)->ReadLogRecord(offset, &log_record, &sz));
      if (sz == 0) {
        break;
      }

      if (log_record.type == data::LogRecordDeleted) {
        data::LogRecordPst* pst = nullptr;
        CHECK_TRUE(index_->Delete(log_record.key, &pst));
        delete pst;
      } else {
        auto* log_record_pst =
            new data::LogRecordPst{.fid = fid, .offset = offset};
        CHECK_TRUE(index_->Put(log_record.key, log_record_pst));
      }

      offset += sz;
    }

    // 如果是 active data file，更新这个文件的 offset
    if (i == sz - 1) {
      active_file_->write_off = offset;
    }
  }
  // 清空 file_ids
  file_ids_->clear();
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
  if (log_record.type == data::LogRecordDeleted) {
    return Status::NotFound("index",
                            "index of key " + key.ToString() + " not_found");
  }
  *value = log_record.value;
  return Status::Ok();
}

}  // namespace bitdb