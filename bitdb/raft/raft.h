#pragma once

#include <cmath>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>
#include "bitdb/co/scheduler.h"
#include "bitdb/common/logger.h"
#include "bitdb/net/rpc/rpc_err_code.h"
#include "bitdb/net/rpc_all.h"
#include "bitdb/raft/raft_protocol.h"
#include "bitdb/raft/raft_state_machine.h"
#include "bitdb/raft/raft_storage.h"
#include "bitdb/timer/time_utils.h"
#include "bitdb/utils/random.h"

namespace bitdb::raft {
using RpcServerPtr = std::shared_ptr<net::rpc::RpcServer>;
using RpcClientPtr = std::shared_ptr<net::rpc::RpcClient>;
using ThreadPtr = std::unique_ptr<std::thread>;
using co::co_spawn;

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
  co::Task<> SendRequestVote(int target, RequestVoteArgs arg,
                             int32_t saved_current_term);

  co::Task<> SendAppendEntries(int target, AppendEntriesArgs<Command> arg);
  co::Task<> HandleAppendEntriesReply(int target,
                                      const AppendEntriesArgs<Command>& arg,
                                      const AppendEntriesReply& reply);

  co::Task<> SendInstallSnapshot(int target, InstallSnapshotArgs arg);
  co::Task<> HandleInstallSnapshotReply(int target,
                                        const InstallSnapshotArgs& arg,
                                        const InstallSnapshotReply& reply);

  int32_t GetLastLogIndex();
  int32_t GetLogTerm(int32_t log_index);

  bool is_stopped() const { return stopped_.load(); }
  int num_nodes() const { return rpc_clients_.size(); }

  // background worker
  void RunBackgroundElection();
  void RunBackgroundHeartbeat();
  void RunBackgroundCommit();
  void RunBackgroundApply();

  std::recursive_mutex mtx_;       // 可重入锁
  RaftStorage<Command>* storage_;  // to persist raft log
  StateMachine* state_machine_;    // the state machine that applys the raft log

  RpcServerPtr rpc_server_;  // rpc server to receive and handle rpc requests
  std::vector<RpcClientPtr>
      rpc_clients_;  // rpc clients of all the node including this node
  int my_id_;        // this index of this node in rpc_clients

  timer::MSTime last_received_rpc_time_;
  Random rander_;  // for random election timeout

  std::atomic<bool> stopped_;

  enum RaftRole { follower, candidate, leader };
  RaftRole role_;
  int leader_id_;
  std::set<int> follower_id_set_;  // 1. for leader 2. for candidate to
                                   // calculate the votes

  /* ----Persistent state on all server----  */
  int32_t current_term_;
  int vote_for_;  // 给哪个candidate投票

  /* ---- Volatile state on all server----  */

  /* ---- Volatile state on leader----  */
  // need to reinitialize after election

  // for each server, index of the next log entry to send to
  // that server(initialized to leader last log index + 1)
  std::vector<int> next_index_;
  // for each server, index of highest log entry known to be replicated on
  // server (initialized to 0, increases monotonically)
  std::vector<int> match_index_;

  ThreadPtr background_thread_election_;
  ThreadPtr background_thread_heartbeat_;
  ThreadPtr background_thread_commit_;
  ThreadPtr background_thread_apply_;

  void StartElection();

  void BeginNewTerm(int32_t term);
  void BecomeFollower(int32_t term);
  void BecomeCandidate(int32_t term);
  void BecomeLeader();

  // debug helper
  std::string RoleToString(RaftRole role) const {
    switch (role) {
      case follower:
        return "follower";
      case candidate:
        return "candidate";
      case leader:
        return "leader";
      default:
        return "unknown";
    }
  }
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
      last_received_rpc_time_(timer::ClockMS()),
      stopped_(false),
      role_(follower),
      current_term_(0),
      vote_for_(-1),
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
inline void Raft<StateMachine, Command>::Start() {
  LOG_INFO("Raft start");
  background_thread_election_ =
      std::make_unique<std::thread>(&Raft::RunBackgroundElection, this);
  background_thread_heartbeat_ =
      std::make_unique<std::thread>(&Raft::RunBackgroundHeartbeat, this);
  background_thread_apply_ =
      std::make_unique<std::thread>(&Raft::RunBackgroundApply, this);
  background_thread_commit_ =
      std::make_unique<std::thread>(&Raft::RunBackgroundCommit, this);
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::Stop() {
  LOG_INFO("Raft stop");
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
inline bool Raft<StateMachine, Command>::IsLeader(int& term) {
  term = current_term_;
  return role_ == leader;
}

template <typename StateMachine, typename Command>
inline bool Raft<StateMachine, Command>::SaveSnapshot() {}

template <typename StateMachine, typename Command>
inline RequestVoteReply Raft<StateMachine, Command>::RequestVote(
    const RequestVoteArgs& args) {
  LOG_INFO(
      "node {} receive RequestVote RPC, args.term: {}, args.candidate_id: {}, "
      "args.last_log_inex: {}, args.last_log_term: {}",
      my_id_, args.term, args.candidate_id, args.last_log_index,
      args.last_log_term);
  RequestVoteReply reply{.term = 0, .vote_granted = false};
  if (is_stopped()) {
    LOG_INFO("when node {} stop, receive RequestVote RPC", my_id_);
    return reply;
  }
  std::lock_guard lock(mtx_);
  if (args.term > current_term_) {
    LOG_INFO("receiver term out of date in RequestVote, become follower");
    BecomeFollower(args.term);
  }
  if (args.term == current_term_ &&
      (vote_for_ == -1 || vote_for_ == args.candidate_id)) {
    const int last_log_index = GetLastLogIndex();
    const int last_log_term = GetLogTerm(last_log_index);
    if (last_log_term < args.last_log_term ||
        (last_log_term == args.last_log_term &&
         last_log_index <= args.last_log_index)) {
      reply.vote_granted = true;
      vote_for_ = args.candidate_id;
    }
  } else {
    reply.vote_granted = false;
  }
  reply.term = current_term_;
  LOG_INFO("node {} receive RequestVote RPC, return term: {}, vote_granted: {}",
           my_id_, reply.term, reply.vote_granted);
  return reply;
}

template <typename StateMachine, typename Command>
inline AppendEntriesReply Raft<StateMachine, Command>::AppendEntries(
    const AppendEntriesArgs<Command>& args) {
  return {};
}

template <typename StateMachine, typename Command>
inline InstallSnapshotReply Raft<StateMachine, Command>::InstallSnapshot(
    const InstallSnapshotArgs& args) {
  return {};
}

template <typename StateMachine, typename Command>
inline co::Task<> Raft<StateMachine, Command>::SendRequestVote(
    int target, RequestVoteArgs arg, int32_t saved_current_term) {
  auto response = co_await rpc_clients_[target]->Call<RequestVoteReply>(
      RPC_REQUEST_VOTE, arg);
  if (response.err_code != net::rpc::RPC_SUCCECC) {
    LOG_INFO("node {} sending RequestVote RPC to client-{} error", my_id_,
             target);
    // 失败了重试
    co_return co_await SendRequestVote(target, arg, saved_current_term);
  }
  const RequestVoteReply reply = response.val();
  std::lock_guard lock(mtx_);
  // 不是 candidate，退出选举（可能退化为追随者，也可能已经胜选成为领导者）
  if (role_ != candidate) {
    LOG_INFO("node {} while waiting for RequestVote reply, state change to {}",
             my_id_, RoleToString(role_));
    co_return;
  }
  // 存在更高任期（新leader），转为 follower
  if (reply.term > saved_current_term) {
    LOG_INFO(
        "node {} candidate's term out of date while RequestVote, change to "
        "follower",
        my_id_);
    BecomeFollower(reply.term);
    co_return;
  }
  if (reply.term == saved_current_term && reply.vote_granted) {
    follower_id_set_.emplace(target);
  }
  // 这里 +1 是因为 candidate 给自己一票
  // num_nodes() / 2向上取整, eg: 3台至少需要2台、5台至少需要3台
  if (follower_id_set_.size() + 1 > std::ceil(num_nodes() / 2)) {
    LOG_INFO("node {} wins the vote with {} votes", my_id_,
             follower_id_set_.size() + 1);
    BecomeLeader();
  }
  co_return;
}

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
inline int32_t Raft<StateMachine, Command>::GetLastLogIndex() {
  // TODO(pangguojian): impl this
  return 0;
}

template <typename StateMachine, typename Command>
inline int32_t Raft<StateMachine, Command>::GetLogTerm(int32_t log_index) {
  // TODO(pangguojian): impl this
  return 0;
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundElection() {
  // Check the liveness of the leader.
  // Work for followers and candidates.

  // Hints: You should record the time you received the last RPC.
  //        And in this function, you can compare the current time with it.
  //        For example:
  //        if (current_time - last_received_RPC_time > timeout)
  //        start_election(); Actually, the timeout should be different between
  //        the follower (e.g. 300-500ms) and the candidate (e.g. 1s).
  while (true) {
    if (is_stopped()) {
      return;
    }
    {
      std::lock_guard lock(mtx_);
      if (role_ == leader) {
        return;
      }

      const auto random_timeout = rander_.UniformRange(150, 300);
      const auto time_diff =
          timer::TimeDiffNow(last_received_rpc_time_).count();
      if (time_diff > random_timeout) {
        StartElection();
      }
    }
    // 睡10ms
    timer::SleepForMS(10);
  }
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundHeartbeat() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundCommit() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::RunBackgroundApply() {}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::StartElection() {
  LOG_INFO("node {} start election", my_id_);
  RequestVoteArgs args{};
  // 用于判断 RequestVote
  // 返回时是否超时了(如果超时了，candidate重新选举，任期号加一)
  int32_t saved_current_term = 0;
  // {
  std::lock_guard lock(mtx_);
  BecomeCandidate(current_term_ + 1);
  saved_current_term = current_term_;

  args.term = saved_current_term;
  args.candidate_id = my_id_;
  args.last_log_index = GetLastLogIndex();
  args.last_log_term = GetLogTerm(args.last_log_index);
  // }

  const auto size = num_nodes();
  for (int i = 0; i < size; ++i) {
    if (i == my_id_) {
      continue;
    }
    co_spawn(SendRequestVote(i, args, saved_current_term));
  }
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::BeginNewTerm(int32_t term) {
  current_term_ = term;
  last_received_rpc_time_ = timer::ClockMS();
  follower_id_set_.clear();
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::BecomeFollower(int32_t term) {
  role_ = follower;
  BeginNewTerm(term);
  vote_for_ = -1;
  LOG_INFO("node {} become follower, term: {}", my_id_, current_term_);
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::BecomeCandidate(int32_t term) {
  role_ = candidate;
  vote_for_ = my_id_;
  BeginNewTerm(term);
  LOG_INFO("node {} become candidate, term: {}", my_id_, current_term_);
}

template <typename StateMachine, typename Command>
inline void Raft<StateMachine, Command>::BecomeLeader() {
  role_ = leader;
  next_index_ = std::vector<int>(num_nodes(), GetLastLogIndex() + 1);
  match_index_ = std::vector<int>(num_nodes(), 0);
  LOG_INFO("node {} become leader, term: {}", my_id_, current_term_);
}

}  // namespace bitdb::raft