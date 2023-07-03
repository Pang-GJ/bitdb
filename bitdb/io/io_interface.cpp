#include "bitdb/io/io_interface.h"
#include <memory>
#include "bitdb/io/file_io.h"

namespace bitdb::io {

std::unique_ptr<IOHandler> NewIOHandler(const std::string& file_name) {
  return std::make_unique<FileIO>(file_name);
}

}  // namespace bitdb::io