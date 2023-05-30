#include "bitdb/data/data_file.h"
#include <cstddef>
#include <memory>
#include <string_view>
#include "bitdb/data/log_record.h"
#include "bitdb/io/io_interface.h"
#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/logger.h"
#include "bitdb/utils/string_utils.h"

namespace bitdb::data {

DataFile::DataFile(uint32_t file_id,
                   std::unique_ptr<io::IOInterface> io_manager)
    : file_id(file_id), write_off(0), io_manager(std::move(io_manager)) {}

Status DataFile::OpenDataFile(std::string_view path, uint32_t file_id,
                              std::unique_ptr<DataFile>* data_file_ptr) {
  // TODO(pangguojian): more check error
  auto file_name = Format("{}/{}{}", path, file_id, K_DATA_FILE_SUFFIX);
  auto io_manager = io::NewIOInterface(file_name);
  CHECK_NOT_NULL_STATUS(io_manager);
  *data_file_ptr = std::make_unique<DataFile>(file_id, std::move(io_manager));
  return Status::Ok();
}

Status DataFile::ReadLogRecord(int64_t offset, LogRecord* log_record,
                               size_t* size) {
  *size = 0;
  auto file_size = io_manager->Size();
  if (file_size == 0) {
    return Status::IOError("Data::ReadLogRecord", "file size equals zero.");
  }
  size_t read_header_size = K_MAX_LOG_RECORD_HEADER_ITEM_SIZE;
  if (offset + read_header_size > file_size) {
    read_header_size = file_size - offset;
  }

  // 读取 header 信息
  auto header_buf = ReadNBytes(read_header_size, offset);
  if (header_buf.empty()) {
    return Status::IOError("Data::ReadLogRecord", "header equals zero.");
  }
  auto&& [header, header_size] = DecodeLogRecordHeader(header_buf);
  if (header == nullptr) {
    return Status::IOError("Data::ReadLogRecord", "io EOF");
  }
  if (header->crc == 0 && header->key_size == 0 && header->value_size == 0) {
    return Status::IOError("Data::ReadLogRecord", "io EOF");
  }
  auto key_size = static_cast<int64_t>(header->key_size);
  auto value_size = static_cast<int64_t>(header->value_size);
  auto record_size = header_size + key_size + value_size;

  log_record->type = header->record_type;

  if (key_size > 0 && value_size > 0) {
    auto kv_buf = ReadNBytes(key_size + value_size, header_size + offset);
    if (kv_buf.empty()) {
      return Status::IOError("Data::ReadLogRecord",
                             "kv pair size equals zero.");
    }

    const auto* kv_data = kv_buf.data();
    log_record->key = Bytes(kv_data, key_size);
    log_record->value = Bytes(kv_data + key_size, value_size);
  }

  // 校验CRC
  auto crc = GetLogRecordCRC(
      *log_record,
      Bytes(header_buf.data() + K_CRC32_SIZE, header_size - K_CRC32_SIZE));
  if (crc != header->crc) {
    return Status::Corruption("DataFile::ReadLogRecord",
                              "invalid crc value, log record maybe corrupted.");
  }
  *size = record_size;
  return Status::Ok();
}

Status DataFile::Write(const Bytes& buf) {
  CHECK_NOT_NULL_STATUS(io_manager);
  auto size = io_manager->Write(buf);
  if (size < 0) {
    return Status::IOError("DataFile::Write", "write datafile failed.");
  }
  write_off += size;
  return Status::Ok();
}

Status DataFile::Sync() {
  CHECK_NOT_NULL_STATUS(io_manager);
  auto res = io_manager->Sync();
  if (res < 0) {
    return Status::IOError("DataFile::Sync()", "sync failed.");
  }
  return Status::Ok();
}

Bytes DataFile::ReadNBytes(int64_t n, int64_t offset) {
  char buffer[n];
  auto res = io_manager->Read(buffer, n, offset);
  if (res < 0) {
    return Bytes{};
  }
  // 这里可能会有内存安全问题？因为 buffer 的生命周期结束了
  return Bytes{buffer};
}
}  // namespace bitdb::data