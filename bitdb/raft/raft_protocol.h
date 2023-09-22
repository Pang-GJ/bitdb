#pragma once

#include "bitdb/codec/serializer.h"
namespace bitdb::raft {

enum RaftRpcStatus { OK, RETRY, RPCERR, NOENT, IOERR };

#define RPC_REQUEST_VOTE "RequestVote"
#define RPC_APPEND_ENTRIES "AppendEntries"
#define RPC_INSTALL_SNAPSHOT "InstallSnapshot"

struct RequestVoteArgs {
  int32_t term;            // candidate 当前任期号
  int32_t candidate_id;    // 请求投票的 candidate id
  int32_t last_log_index;  // candidate 最后一个日志索引
  int32_t last_log_term;   // candidate 最后一个日志的任期

  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(term);
    serializer->serialize(candidate_id);
    serializer->serialize(last_log_index);
    serializer->serialize(last_log_term);
  };
  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&term);
    serializer->deserialize(&candidate_id);
    serializer->deserialize(&last_log_index);
    serializer->deserialize(&last_log_term);
  }
};

struct RequestVoteReply {
  int32_t term;       // current term, for candidate to update itself
  bool vote_granted;  // true 意味着 candidate 可以获得它的选票

  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(term);
    serializer->serialize(vote_granted);
  }
  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&term);
    serializer->deserialize(&vote_granted);
  }
};

template <typename Command>
struct LogEntry {
  Command cmd;
  int32_t term;
  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(cmd);
    serializer->serialize(term);
  }
  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&cmd);
    serializer->deserialize(&term);
  }
};

template <typename Command>
struct AppendEntriesArgs {
  int32_t term;
  int32_t leader_id;
  int32_t prev_log_index;
  int32_t prev_log_term;
  std::vector<LogEntry<Command>> entries;
  int32_t leader_commit;

  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(term);
    serializer->serialize(leader_id);
    serializer->serialize(prev_log_index);
    serializer->serialize(prev_log_term);
    serializer->serialize(entries);
    serializer->serialize(leader_commit);
  }
  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&term);
    serializer->deserialize(&leader_id);
    serializer->deserialize(&prev_log_index);
    serializer->deserialize(&prev_log_term);
    serializer->deserialize(&entries);
    serializer->deserialize(&leader_commit);
  }
};

struct AppendEntriesReply {
  int32_t term;
  bool success;

  void serialize(codec::Serializer* serializer) const {}
  void deserialize(codec::Serializer* serializer) {}
};

struct InstallSnapshotArgs {
  void serialize(codec::Serializer* serializer) const {}
  void deserialize(codec::Serializer* serializer) {}
};

struct InstallSnapshotReply {
  void serialize(codec::Serializer* serializer) const {}
  void deserialize(codec::Serializer* serializer) {}
};

}  // namespace bitdb::raft