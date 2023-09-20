#pragma once

#include <mutex>
#include <string_view>
#include <type_traits>
#include "bitdb/raft/raft_state_machine.h"

namespace bitdb::raft {

template <typename Command>
class RaftStorage {
  static_assert(std::is_base_of<RaftCommand, Command>(),
                "Command must inherit from RaftCommand");

 public:
  explicit RaftStorage(std::string_view file_dir);
  ~RaftStorage() = default;

 private:
  std::mutex mtx_;
};

template <typename Command>
inline RaftStorage<Command>::RaftStorage(std::string_view file_dir) {
  //
}

}  // namespace bitdb::raft