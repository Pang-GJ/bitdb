#include "bitdb/io/io_interface.h"
#include <memory>
#include <string_view>
#include "bitdb/io/file_io.h"

namespace bitdb::io {

std::unique_ptr<IOHandler> NewIOHandler(std::string_view file_name) {
  return std::make_unique<FileIO>(file_name);
}

}  // namespace bitdb::io