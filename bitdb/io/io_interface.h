#pragma once

#include <sys/types.h>
#include <cstdint>
#include <memory>
#include <string_view>
#include "bitdb/utils/bytes.h"

namespace bitdb::io {

#define DataFilePerm \
  (S_IRWXU | S_IROTH);  // 0644 表示创建一个文件，文件所有者可读写，其他人只能读

class IOHandler {
 public:
  IOHandler() = default;
  virtual ~IOHandler() = default;

  virtual int Read(char* buf, size_t count, off_t offset) = 0;

  virtual int Write(const Bytes& bytes) = 0;
  virtual int Sync() = 0;

  virtual size_t Size() = 0;
};

std::unique_ptr<IOHandler> NewIOHandler(std::string_view file_name);

}  // namespace bitdb::io