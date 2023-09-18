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
  ~RaftStorage();

 private:
  std::mutex mtx_;
};

}  // namespace bitdb::raft