#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/data/data_file.h"
#include "bitdb/utils/logger.h"
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