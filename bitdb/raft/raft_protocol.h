#pragma once

#include "bitdb/codec/serializer.h"
namespace bitdb::raft {

enum RaftRpcStatus { OK, RETRY, RPCERR, NOENT, IOERR };

#define RPC_REQUEST_VOTE "RequestVote"
#define RPC_APPEND_ENTRIES "AppendEntries"
#define RPC_INSTALL_SNAPSHOT "InstallSnapshot"

struct RequestVoteArgs {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

struct RequestVoteReply {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

template <typename Command>
struct LogEntry {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

template <typename Command>
struct AppendEntriesArgs {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

struct AppendEntriesReply {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

struct InstallSnapshotArgs {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

struct InstallSnapshotReply {
  void serialize(codec::Serializer* serializer) const;
  void deserialize(codec::Serializer* serializer);
};

}  // namespace bitdb::raft