#include "bitdb/data/data_file.h"
#include <string_view>
#include "bitdb/status.h"

namespace bitdb::data {

DataFile::DataFile(std::string_view dir_path, uint32_t file_id) {}

Status DataFile::ReadLogRecord(int64_t offset, LogRecord* log_record) {
  return Status::NotSupported("DataFile", "ReadLogRecord");
}

Status DataFile::Write(const Bytes& buf) {
  return Status::NotSupported("DataFile", "Write");
}
Status DataFile::Sync() { return Status::NotSupported("DataFile", "Sync"); }

}  // namespace bitdb::data