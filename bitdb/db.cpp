#include "bitdb/db.h"
#include <mutex>
#include "bitdb/data/data_file.h"
#include "bitdb/data/log_record.h"
#include "bitdb/status.h"

namespace bitdb {

Status DB::Put(const Bytes& key, const Bytes& value) {
  if (key.empty()) {
    return Status::InvalidArgument("DB::Put", "key is empty");
  }

  data::LogRecord log_record{key, value, data::LogRecordNormal};
  data::LogRecordPst pst;
  CHECK_OK(AppendLogRecord(log_record, &pst));
  auto ok = index_->Put(key, &pst);
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
  CHECK_OK(
      (*data_file_ptr)->ReadLogRecord(log_record_pst->offset, &log_record));
  if (log_record.type == data::LogRecordDeleted) {
    return Status::NotFound("index",
                            "index of key " + key.ToString() + " not_found");
  }
  *value = log_record.value.data();
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
  uint32_t init_fileid = 0;
  if (active_file_ != nullptr) {
    init_fileid = active_file_->file_id + 1;
  }

  // 打开新的数据文件
  std::unique_ptr<data::DataFile> data_file;
  CHECK_OK(
      data::DataFile::OpenDataFile(options_.dir_path, init_fileid, &data_file));
  active_file_ = std::move(data_file);
  return Status::Ok();
}

}  // namespace bitdb