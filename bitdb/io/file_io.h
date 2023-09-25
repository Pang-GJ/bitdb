#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include "bitdb/common/logger.h"
#include "bitdb/io/io_interface.h"

namespace bitdb::io {

class FileIO : public IOHandler {
 public:
  explicit FileIO(std::string_view file_name,
                  int flags = O_CREAT | O_RDWR | O_APPEND) {
    fd_ = ::open(file_name.data(), flags, S_IRWXU | S_IROTH);
    if (fd_ < 0) {
      LOG_ERROR("FilIO: open file_name: {} failed, errno: {}, description: {}",
                file_name, errno, strerror(errno));
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