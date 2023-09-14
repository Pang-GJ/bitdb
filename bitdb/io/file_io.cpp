#include "bitdb/io/file_io.h"
#include <sys/stat.h>
#include <unistd.h>
#include <cstddef>

namespace bitdb::io {

int FileIO::Read(char* buf, size_t count, off_t offset) {
  return ::pread64(fd_, buf, count, offset);
}

int FileIO::Write(const Bytes& bytes) {
  return ::write(fd_, bytes.data(), bytes.size());
}

int FileIO::Sync() { return ::fsync(fd_); }

size_t FileIO::Size() {
  struct stat st {};
  if (::fstat(fd_, &st) != 0) {
    return 0;
  }
  return st.st_size;
}

}  // namespace bitdb::io
