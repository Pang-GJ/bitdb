#pragma once

#include <cstdint>
#include <memory>
#include "bitdb/data/log_record.h"
#include "bitdb/io/io_interface.h"
#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::data {

constexpr std::string_view K_DATA_FILE_SUFFIX = ".data";
constexpr std::string_view K_HINT_FILE_NAME = "hint-index";
constexpr std::string_view K_MERGED_FILE_NAME = "merged";

struct DataFile {
  uint32_t file_id;                           // NOLINT
  int64_t write_off;                          // NOLINT
  std::unique_ptr<io::IOHandler> io_handler;  // NOLINT

  DataFile(uint32_t file_id, std::unique_ptr<io::IOHandler> io_handler);

  static Status OpenDataFile(std::string_view path, uint32_t file_id,
                             std::unique_ptr<DataFile>* data_file_ptr);

  static Status OpenHintFile(std::string_view path,
                             std::unique_ptr<DataFile>* data_file_ptr);

  static Status OpenMergedFile(std::string_view path,
                               std::unique_ptr<DataFile>* data_file_ptr);
  /**
   * @brief 读取数据日志记录
   *
   * @param offset
   * @param log_record
   * @param size 如果 EOF，返回 0，否则返回日志记录大小
   * @return Status
   */
  Status ReadLogRecord(uint64_t offset, LogRecord* log_record, size_t* size);
  Status Write(const Bytes& buf);
  Status Sync();

 private:
  static Status NewDataFile(std::string_view file_name, uint32_t file_id,
                            std::unique_ptr<DataFile>* data_file_ptr);

  std::string ReadNBytes(int64_t n, int64_t offset);
};

std::string GetHintFileName(std::string_view path);
std::string GetMergedFileName(std::string_view path);

Status WriteHintRecord(DataFile* hint_file, const Bytes& key,
                       LogRecordPst* pst);

}  // namespace bitdb::data