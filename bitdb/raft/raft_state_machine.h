#pragma once

#include "bitdb/codec/serializer.h"
namespace bitdb::raft {

class RaftCommand {
 public:
  virtual ~RaftCommand() = default;

  // serializer
  virtual void serialize(codec::Serializer* serializer) const = 0;
  virtual void deserialize(codec::Serializer* serializer) = 0;
};

class RaftStateMachine {
 public:
  virtual ~RaftStateMachine() = default;

  /**
   * @brief Apply a log to the state machine
   *
   */
  virtual void ApplyLog(RaftCommand&) = 0;

  /**
   * @brief generate a shapshot of current state
   *
   * @return std::vector<char>
   */
  virtual std::vector<char> Snapshot() = 0;

  /**
   * @brief apply the snapshot to the state machine
   *
   */
  virtual void ApplySnapshot(const std::vector<char>&) = 0;
};

}  // namespace bitdb::raft