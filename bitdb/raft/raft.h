#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include "bitdb/net/rpc_all.h"
#include "bitdb/raft/raft_protocol.h"
#include "bitdb/raft/raft_state_machine.h"
#include "bitdb/raft/raft_storage.h"

namespace bitdb::raft {
using RpcServerPtr = std::shared_ptr<net::rpc::RpcServer>;
using RpcClientPtr = std::shared_ptr<net::rpc::RpcClient>;
using ThreadPtr = std::unique_ptr<std::thread>;

template <typename StateMachine, typename Command>
class Raft {
  static_assert(std::is_base_of<RaftStateMachine, StateMachine>(),
                "StateMachine must inherit from RaftStateMachine");
  static_assert(std::is_base_of<RaftCommand, Command>(),
                "Command must inherit from RaftCommand");

 public:
  Raft(RpcServerPtr rpc_server, std::vector<RpcClientPtr> rpc_clients, int idx,
       RaftStorage<Command>* storage, StateMachine* state_machine);
  ~Raft();

  /**
   * @brief start the raft node.
   * Please make sure all of the rpc request handlers have been registered
   * before this method.
   *
   */
  void Start();

  /**
   * @brief stop the raft node.
   * Please make sure all of the background threads are joined in this method.
   * Notice: you should check whether is server should be stopped by calling
   * is_stopped().
   * Once it returns true, you should break all of your long-running
   * loops in the background threads.
   */
  void Stop();

  //
  /**
   * @brief send a new command to the raft nodes.
   * @param cmd
   * @param term
   * @param index
   * @return true: this raft node is the leader that successfully appends the
   * log.
   * @return false: this node is not the leader
   */
  bool NewCommand(Command cmd, int& term, int& index);

  /**
   * @brief return is_leader? also set the current term
   *
   * @param term
   * @return whether this node is the leader
   */
  bool IsLeader(int& term);

  /**
   * @brief save a snapshot of the state machine and compact the log.
   *
   * @return true
   * @return false
   */
  bool SaveSnapshot();

 private:
  // RPC handlers
  RequestVoteReply RequestVote(const RequestVoteArgs& args);

  AppendEntriesReply AppendEntries(const AppendEntriesArgs<Command>& args);

  InstallSnapshotReply InstallSnapshot(const InstallSnapshotArgs& args);

  // RPC helpers
  co::Task<> SendRequestVote(int target, RequestVoteArgs arg);
  co::Task<> HandleRequestVoteReply(int target, const RequestVoteArgs& arg,
                                    const RequestVoteReply& reply);

  co::Task<> SendAppendEntries(int target, AppendEntriesArgs<Command> arg);
  co::Task<> HandleAppendEntriesReply(int target,
                                      const AppendEntriesArgs<Command>& arg,
                                      const AppendEntriesReply& reply);

  co::Task<> SendInstallSnapshot(int target, InstallSnapshotArgs arg);
  co::Task<> HandleInstallSnapshotReply(int target,
                                        const InstallSnapshotArgs& arg,
                                        const InstallSnapshotReply& reply);

  bool is_stopped() const { return stopped_.load(); }
  int num_nodes() const { return rpc_clients_.size(); }

  // background worker
  void RunBackgroundElection();
  void RunBackgroundHeartbeat();
  void RunBackgroundCommit();
  void RunBackgroundApply();

  std::mutex mtx_;
  RaftStorage<Command>* storage_;  // to persist raft log
  StateMachine* state_machine_;    // the state machine that applys the raft log

  RpcServerPtr rpc_server_;  // rpc server to receive and handle rpc requests
  std::vector<RpcClientPtr>
      rpc_clients_;  // rpc clients of all the node including this node
  int my_id_;        // this index of this node in rpc_clients

  std::atomic<bool> stopped_;

  enum RaftRole { follower, candidate, leader };
  RaftRole role_;
  int current_term_;
  int leader_id_;

  ThreadPtr background_thread_election_;
  ThreadPtr background_thread_heartbeat_;
  ThreadPtr background_thread_commit_;
  ThreadPtr background_thread_apply_;

  // Your code here:

  /* ----Persistent state on all server----  */

  /* ---- Volatile state on all server----  */

  /* ---- Volatile state on leader----  */
};

template <typename StateMachine, typename Command>
inline Raft<StateMachine, Command>::Raft(RpcServerPtr rpc_server,
                                         std::vector<RpcClientPtr> rpc_clients,
                                         int idx, RaftStorage<Command>* storage,
                                         StateMachine* state_machine)
    : storage_(storage),
      state_machine_(state_machine),
      rpc_server_(std::move(rpc_server)),
      rpc_clients_(std::move(rpc_clients)),
      my_id_(idx),
      stopped_(false),
      role_(follower),
      current_term_(0),
      background_thread_election_(nullptr),
      background_thread_heartbeat_(nullptr),
      background_thread_commit_(nullptr),
      background_thread_apply_(nullptr) {
  rpc_server_->Bind(RPC_REQUEST_VOTE, &Raft::RequestVote, this);
  rpc_server_->Bind(RPC_APPEND_ENTRIES, &Raft::AppendEntries, this);
  rpc_server_->Bind(RPC_INSTALL_SNAPSHOT, &Raft::InstallSnapshot, this);
}

template <typename StateMachine, typename Command>
inline Raft<StateMachine, Command>::~Raft() {
  if (!is_stopped()) {
    Stop();
  }
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::Start() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::Stop() {
  stopped_.store(true);
  if (background_thread_apply_) {
    background_thread_apply_->join();
  }
  if (background_thread_commit_) {
    background_thread_commit_->join();
  }
  if (background_thread_election_) {
    background_thread_election_->join();
  }
  if (background_thread_heartbeat_) {
    background_thread_heartbeat_->join();
  }
}

template <typename StateMachine, typename Command>
inline bool Raft<StateMachine, Command>::NewCommand(Command cmd, int& term,
                                                    int& index) {}

template <typename StateMachine, typename Command>
inline bool Raft<StateMachine, Command>::IsLeader(int& term) {}

template <typename StateMachine, typename Command>
inline bool Raft<StateMachine, Command>::SaveSnapshot() {}

template <typename StateMachine, typename Command>
inline RequestVoteReply Raft<StateMachine, Command>::RequestVote(
    const RequestVoteArgs& args) {}

template <typename StateMachine, typename Command>
inline AppendEntriesReply Raft<StateMachine, Command>::AppendEntries(
    const AppendEntriesArgs<Command>& args) {}

template <typename StateMachine, typename Command>
inline InstallSnapshotReply Raft<StateMachine, Command>::InstallSnapshot(
    const InstallSnapshotArgs& args) {}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::SendRequestVote(
    int target, RequestVoteArgs arg) {}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::HandleRequestVoteReply(
    int target, const RequestVoteArgs& arg, const RequestVoteReply& reply) {}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::SendAppendEntries(
    int target, AppendEntriesArgs<Command> arg) {}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::HandleAppendEntriesReply(
    int target, const AppendEntriesArgs<Command>& arg,
    const AppendEntriesReply& reply) {}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::SendInstallSnapshot(
    int target, InstallSnapshotArgs arg) {}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::HandleInstallSnapshotReply(
    int target, const InstallSnapshotArgs& arg,
    const InstallSnapshotReply& reply) {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundElection() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundHeartbeat() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundCommit() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundApply() {}

}  // namespace bitdb::raft