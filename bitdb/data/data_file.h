#pragma once

#include <cstdint>
#include "bitdb/data/log_record.h"
#include "bitdb/io/io_interface.h"
#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"

namespace bitdb::data {

struct DataFile {
  uint32_t file_id;             // NOLINT
  int64_t write_off;            // NOLINT
  io::IOInterface* io_manager;  // NOLINT

  DataFile(std::string_view dir_path, uint32_t file_id);

  static Status OpenDataFile(std::string_view path, uint32_t file_id,
                             std::unique_ptr<DataFile>* data_file_ptr);

  Status ReadLogRecord(int64_t offset, LogRecord* log_record);
  Status Write(const Bytes& buf);
  Status Sync();
};

}  // namespace bitdb::data