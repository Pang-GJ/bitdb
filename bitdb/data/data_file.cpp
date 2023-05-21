#include "bitdb/data/data_file.h"
#include "bitdb/status.h"

namespace bitdb::data {

Status DataFile::ReadLogRecord(int64_t offset, LogRecord* log_record) {
  return Status::NotSupported("DataFile", "ReadLogRecord");
}

Status DataFile::Write(const Bytes& buf) {
  return Status::NotSupported("DataFile", "Write");
}
Status DataFile::Sync() { return Status::NotSupported("DataFile", "Sync"); }

}  // namespace bitdb::data