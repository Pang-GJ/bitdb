#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include "bitdb/io/io_interface.h"
#include "bitdb/utils/logger.h"

namespace bitdb::io {

class FileIO : public IOInterface {
 public:
  explicit FileIO(const std::string& file_name) {
    fd_ = ::open(file_name.c_str(), O_CREAT | O_RDWR | O_APPEND,
                 S_IRWXU | S_IROTH);
    if (fd_ < 0) {
      LOG_ERROR("FilIO: open file_name: {} failed, error: {}.", file_name,
                errno);
    }
  }

  ~FileIO() override { ::close(fd_); }

  int Read(char* buf, size_t count, off_t offset) override;
  int Write(const Bytes& bytes) override;
  int Sync() override;
  size_t Size() override;

 private:
  int fd_;  // 文件描述符
};

}  // namespace bitdb::io