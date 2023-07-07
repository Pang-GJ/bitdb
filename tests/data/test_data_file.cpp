#include "bitdb/data/log_record.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/data/data_file.h"
#include "bitdb/common/logger.h"
#include "bitdb/utils/os_utils.h"
using bitdb::data::DataFile;

TEST_CASE("test open data file") {
  std::unique_ptr<DataFile> data_file = nullptr;
  auto tmp_dir = bitdb::GetTempDir();
  LOG_INFO("temp_dir: {}", tmp_dir);
  auto status = bitdb::data::DataFile::OpenDataFile(tmp_dir, 0, &data_file);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, data_file);

  std::unique_ptr<DataFile> data_file1 = nullptr;
  auto status1 = bitdb::data::DataFile::OpenDataFile(tmp_dir, 1, &data_file1);
  CHECK_EQ(true, status1.IsOk());
  CHECK_NE(nullptr, data_file1);

  std::unique_ptr<DataFile> data_file2 = nullptr;
  auto status2 = bitdb::data::DataFile::OpenDataFile(tmp_dir, 1, &data_file2);
  CHECK_EQ(true, status2.IsOk());
  CHECK_NE(nullptr, data_file2);
}

TEST_CASE("test write data file") {
  std::unique_ptr<DataFile> data_file = nullptr;
  auto tmp_dir = bitdb::GetTempDir();
  LOG_INFO("temp_dir: {}", tmp_dir);
  auto status = bitdb::data::DataFile::OpenDataFile(tmp_dir, 0, &data_file);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, data_file);

  status = data_file->Write("abc");
  CHECK_EQ(true, status.IsOk());

  status = data_file->Write("hello");
  CHECK_EQ(true, status.IsOk());

  status = data_file->Write(" world");
  CHECK_EQ(true, status.IsOk());
}

TEST_CASE("test sync data file") {
  std::unique_ptr<DataFile> data_file = nullptr;
  auto tmp_dir = bitdb::GetTempDir();
  LOG_INFO("temp_dir: {}", tmp_dir);
  auto status = bitdb::data::DataFile::OpenDataFile(tmp_dir, 0, &data_file);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, data_file);

  status = data_file->Sync();
  CHECK_EQ(true, status.IsOk());
}

TEST_CASE("test read log_record from data file") {
  std::unique_ptr<DataFile> data_file = nullptr;
  auto tmp_dir = bitdb::GetTempDir();
  LOG_INFO("temp_dir: {}", tmp_dir);
  auto status = bitdb::data::DataFile::OpenDataFile(tmp_dir, 2, &data_file);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, data_file);

  // 只有一条日志记录
  bitdb::data::LogRecord log_record1{"name", "bitdb",
                                     bitdb::data::NormalLogRecord};
  auto encode_bytes1 = bitdb::data::EncodeLogRecord(log_record1);
  status = data_file->Write(encode_bytes1);
  CHECK_EQ(true, status.IsOk());

  bitdb::data::LogRecord read_log_record;
  size_t read_log_size;
  status = data_file->ReadLogRecord(0, &read_log_record, &read_log_size);
  CHECK_EQ(true, status.IsOk());
  if (!status.IsOk()) {
    LOG_ERROR("Read log record failed: {}.", status.ToString());
  }
  CHECK_EQ(log_record1, read_log_record);
  CHECK_EQ(encode_bytes1.size(), read_log_size);
  LOG_INFO("test single log record passed.");

  // 多条 log record 从不同位置读取
  bitdb::data::LogRecord log_record2{"name", "bitcask-kv",
                                     bitdb::data::NormalLogRecord};
  auto encode_bytes2 = bitdb::data::EncodeLogRecord(log_record2);
  LOG_INFO("encode_bytes2: {}", encode_bytes2);
  status = data_file->Write(encode_bytes2);
  CHECK_EQ(true, status.IsOk());
  size_t read_log_size2;
  status = data_file->ReadLogRecord(read_log_size, &read_log_record,
                                    &read_log_size2);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(log_record2, read_log_record);
  CHECK_EQ(encode_bytes2.size(), read_log_size2);
  LOG_INFO("test multi log record passed.");

  // 被删除的数据在文件末尾
  bitdb::data::LogRecord log_record3{"name", "delete-data",
                                     bitdb::data::DeletedLogRecord};
  auto encode_bytes3 = bitdb::data::EncodeLogRecord(log_record3);
  LOG_INFO("encode_bytes3: {}", encode_bytes3);
  status = data_file->Write(encode_bytes3);
  CHECK_EQ(true, status.IsOk());
  size_t read_log_size3;
  status = data_file->ReadLogRecord(read_log_size + read_log_size2,
                                    &read_log_record, &read_log_size3);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(log_record3, read_log_record);
  CHECK_EQ(encode_bytes3.size(), read_log_size3);
  LOG_INFO("test deleted log record passed.");
}