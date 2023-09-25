#pragma once

#include <fcntl.h>
#include <cstring>
#include <mutex>
#include <string_view>
#include <type_traits>
#include <vector>
#include "bitdb/codec/serializer.h"
#include "bitdb/io/file_io.h"
#include "bitdb/raft/raft_protocol.h"
#include "bitdb/raft/raft_state_machine.h"

namespace bitdb::raft {

template <typename Command>
class RaftStorage {
  static_assert(std::is_base_of<RaftCommand, Command>(),
                "Command must inherit from RaftCommand");

 public:
  explicit RaftStorage(std::string_view file_dir);
  ~RaftStorage();

  int32_t current_term;
  int32_t vote_for;
  std::vector<LogEntry<Command>> log;

  void AddLog(const LogEntry<Command>& entry) {
    std::lock_guard lock(mtx_);
    log.emplace_back(entry);
  }

  void AddLog(const std::vector<LogEntry<Command>>& entries) {
    std::lock_guard lock(mtx_);
    log.insert(log.end(), entries.cbegin(), entries.cend());
  }

  void AddLog(int log_insert_index,
              const std::vector<LogEntry<Command>>& entries) {
    std::lock_guard lock(mtx_);
    // 覆盖
    const auto need_size = log_insert_index + entries.size();
    if (log.size() < need_size) {
      log.resize(need_size);
    }
    for (size_t i = 0; i < entries.size(); i++) {
      log[log_insert_index + i] = entries[i];
    }
  }

  void Flush() {
    std::lock_guard lock(mtx_);
    codec::Serializer serializer;
    serializer.serialize(*this);
    auto res = io_handler_.Write(serializer.str());
    if (res < 0) {
      LOG_ERROR("RaftStorge Flush error, errno: {}, description: {}", errno,
                strerror(errno));
    }
  }

  // for serializer
  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(current_term);
    serializer->serialize(vote_for);
    serializer->serialize(log);
  }
  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&current_term);
    serializer->deserialize(&vote_for);
    serializer->deserialize(&log);
  }

 private:
  std::mutex mtx_;
  std::string file_path_;
  // use file io to write log
  io::FileIO io_handler_;
};

template <typename Command>
inline RaftStorage<Command>::RaftStorage(std::string_view file_dir)
    : file_path_(file_dir),
      io_handler_(file_path_ + "/data", O_CREAT | O_RDWR) {
  const auto file_size = io_handler_.Size();
  if (file_size > 0) {
    std::vector<char> buffer(file_size);
    auto res = io_handler_.Read(buffer.data(), file_size, 0);
    if (res < 0) {
      LOG_ERROR(
          "try to read RaftStorage failed, size: {}, errno: {}, description: "
          "{}",
          file_size, errno, strerror(errno));
      current_term = 0;
      vote_for = -1;
      log.clear();
    } else {
      codec::Serializer serializer(buffer.cbegin(), buffer.cend());
      // serializer.deserialize(*this);
      serializer.deserialize(&current_term);
      serializer.deserialize(&vote_for);
      serializer.deserialize(&log);
    }
  } else {
    current_term = 0;
    vote_for = -1;
    log.clear();
  }
}

template <typename Command>
inline RaftStorage<Command>::~RaftStorage() {
  Flush();
}

}  // namespace bitdb::raft