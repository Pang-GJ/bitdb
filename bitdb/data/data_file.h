#pragma once

#include <cstdint>
#include <memory>
#include "bitdb/data/log_record.h"
#include "bitdb/io/io_interface.h"
#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::data {

constexpr std::string_view K_DATA_FILE_SUFFIX = ".data";

struct DataFile {
  uint32_t file_id;             // NOLINT
  int64_t write_off;            // NOLINT
  io::IOInterface* io_manager;  // NOLINT

  DataFile(std::string_view dir_path, uint32_t file_id);

  static Status OpenDataFile(std::string_view path, uint32_t file_id,
                             std::unique_ptr<DataFile>* data_file_ptr);

  /**
   * @brief 读取数据日志记录
   *
   * @param offset
   * @param log_record
   * @param size 如果 EOF，返回 0，否则返回日志记录大小
   * @return Status
   */
  Status ReadLogRecord(int64_t offset, LogRecord* log_record, size_t* size);
  Status Write(const Bytes& buf);
  Status Sync();
};

}  // namespace bitdb::data