#include "bitdb/data/data_file.h"
#include <string_view>
#include "bitdb/status.h"

namespace bitdb::data {

DataFile::DataFile(std::string_view dir_path, uint32_t file_id) {}

Status DataFile::OpenDataFile(std::string_view path, uint32_t file_id,
                              std::unique_ptr<DataFile>* data_file_ptr) {
  *data_file_ptr = std::make_unique<DataFile>(path, file_id);
  return Status::Ok();
}

Status DataFile::ReadLogRecord(int64_t offset, LogRecord* log_record,
                               size_t* size) {
  return Status::NotSupported("DataFile", "ReadLogRecord");
}

Status DataFile::Write(const Bytes& buf) {
  return Status::NotSupported("DataFile", "Write");
}
Status DataFile::Sync() { return Status::NotSupported("DataFile", "Sync"); }

}  // namespace bitdb::data