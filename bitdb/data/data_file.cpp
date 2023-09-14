#include "bitdb/data/data_file.h"
#include "bitdb/common/logger.h"
#include "bitdb/data/log_record.h"
#include "bitdb/io/io_interface.h"
#include "bitdb/status.h"
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/format.h"
#include "bitdb/utils/os_utils.h"
#include "bitdb/utils/string_utils.h"

namespace bitdb::data {

DataFile::DataFile(uint32_t file_id, std::unique_ptr<io::IOHandler> io_handler)
    : file_id(file_id), write_off(0), io_handler(std::move(io_handler)) {}

Status DataFile::OpenDataFile(std::string_view path, uint32_t file_id,
                              std::unique_ptr<DataFile>* data_file_ptr) {
  return NewDataFile(GetDataFileName(path, file_id), file_id, data_file_ptr);
}

Status DataFile::OpenHintFile(std::string_view path,
                              std::unique_ptr<DataFile>* data_file_ptr) {
  return NewDataFile(GetHintFileName(path), 0, data_file_ptr);
}

Status DataFile::OpenMergedFile(std::string_view path,
                                std::unique_ptr<DataFile>* data_file_ptr) {
  return NewDataFile(GetMergedFileName(path), 0, data_file_ptr);
}

Status DataFile::ReadLogRecord(uint64_t offset, LogRecord* log_record,
                               size_t* size) {
  *size = 0;
  auto file_size = io_handler->Size();
  if (file_size == 0) {
    return Status::IOError("Data::ReadLogRecord", "file size equals zero.");
  }
  if (file_size == offset) {
    return Status::Ok("IO EOF");
  }
  size_t try_read_header_size = K_MAX_LOG_RECORD_HEADER_SIZE;
  if (offset + try_read_header_size > file_size) {
    // 这里让 read_header_size 缩小继续读的原因是
    // header 是变长的，如果 offset 后面还有内容，那么 header 肯定可以读完整
    // 因为写入的时候就保证了，不完整肯定不会写
    try_read_header_size = file_size - offset;
  }

  // 读取 header 信息
  auto header_buf = ReadNBytes(try_read_header_size, offset);
  if (header_buf.empty()) {
    return Status::IOError("Data::ReadLogRecord", "header equals zero.");
  }
  auto&& [header, header_size] = DecodeLogRecordHeader(header_buf);
  if (header_size == 0) {
    return Status::IOError("Data::ReadLogRecord", "io EOF");
  }
  if (header.crc == 0 && header.key_size == 0 && header.value_size == 0) {
    return Status::IOError("Data::ReadLogRecord", "io EOF");
  }
  auto key_size = header.key_size;
  auto value_size = header.value_size;
  auto record_size = header_size + key_size + value_size;

  log_record->type = header.record_type;

  if (key_size > 0 || value_size > 0) {
    assert(key_size + value_size < file_size);
    auto kv_buf = ReadNBytes(key_size + value_size, offset + header_size);
    if (kv_buf.empty()) {
      return Status::IOError("Data::ReadLogRecord", "kv pair size equals zero");
    }

    log_record->key = std::string(kv_buf.begin(), kv_buf.begin() + key_size);
    log_record->value = std::string(kv_buf.begin() + key_size,
                                    kv_buf.begin() + key_size + value_size);
  }

  // 校验CRC
  auto crc = GetLogRecordCRC(
      *log_record,
      std::string(header_buf.begin(), header_buf.begin() + header_size));
  if (crc != header.crc) {
    return Status::Corruption("DataFile::ReadLogRecord",
                              "invalid crc value, log record maybe corrupted.");
  }
  *size = record_size;
  return Status::Ok();
}

Status DataFile::Write(const Bytes& buf) {
  CHECK_NOT_NULL_STATUS(io_handler);
  auto size = io_handler->Write(buf);
  if (size < 0) {
    return Status::IOError("DataFile::Write", "write datafile failed.");
  }
  write_off += size;
  return Status::Ok();
}

Status DataFile::Sync() {
  CHECK_NOT_NULL_STATUS(io_handler);
  auto res = io_handler->Sync();
  if (res < 0) {
    return Status::IOError("DataFile::Sync()", "sync failed.");
  }
  return Status::Ok();
}

Status DataFile::NewDataFile(std::string_view file_name, uint32_t file_id,
                             std::unique_ptr<DataFile>* data_file_ptr) {
  CHECK_NOT_NULL_STATUS(data_file_ptr);
  auto io_handler = io::NewIOHandler(file_name);
  CHECK_NOT_NULL_STATUS(io_handler);
  *data_file_ptr = std::make_unique<DataFile>(file_id, std::move(io_handler));
  return Status::Ok();
}

std::string DataFile::ReadNBytes(int64_t n, int64_t offset) {
  char buffer[n + 1];
  buffer[n] = '\n';
  auto res = io_handler->Read(buffer, n, offset);
  if (res < 0) {
    LOG_WARN("io_handler Read res less than 0.");
    return {};
  }
  return {buffer, buffer + res};
}

std::string GetDataFileName(std::string_view path, uint32_t file_id) {
  return Format("{}/{}{}", path, file_id, K_DATA_FILE_SUFFIX);
}

std::string GetHintFileName(std::string_view path) {
  return Format("{}/{}", path, K_HINT_FILE_NAME);
}

std::string GetMergedFileName(std::string_view path) {
  return Format("{}/{}", path, K_MERGED_FILE_NAME);
}

std::string GetMergenceDirectory(std::string db_path) {
  return Format("{}/{}", db_path,
                PathBase(db_path).append(K_MERGENCE_FOLDER_SUFFIX));
}

Status WriteHintRecord(DataFile* hint_file, const Bytes& key,
                       const LogRecordPst& pst) {
  LogRecord log_record{.key = key.data(),
                       .value = EncodeLogRecordPosition(pst)};
  auto encode_bytes = EncodeLogRecord(log_record);
  return hint_file->Write(encode_bytes);
}

}  // namespace bitdb::data