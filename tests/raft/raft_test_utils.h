#pragma once
#include <doctest/doctest.h>
#include <sys/stat.h>
#include <cassert>
#include <mutex>
#include "bitdb/co/scheduler.h"
#include "bitdb/net/tcp/tcp_server.h"
#include "bitdb/raft/raft.h"
#include "bitdb/raft/raft_state_machine.h"
#include "bitdb/raft/raft_storage.h"
#include "bitdb/timer/time_utils.h"

int remove_directory(const char* path);
void mssleep(int ms);
/******************************************************************

                         For Raft Test

*******************************************************************/

std::vector<std::shared_ptr<bitdb::net::TcpServer>> create_random_tcp_servers(
    std::vector<bitdb::raft::RpcServerPtr>& rpc_servers);

std::vector<bitdb::raft::RpcServerPtr> create_random_rpc_servers(int num);

std::vector<bitdb::raft::RpcClientPtr> create_rpc_clients(
    const std::vector<std::shared_ptr<bitdb::net::TcpServer>>& tcp_servers);

class ListCommand : public bitdb::raft::RaftCommand {
 public:
  ListCommand() = default;
  ListCommand(const ListCommand& cmd) = default;
  explicit ListCommand(int v) : value(v) {}
  ~ListCommand() override = default;

  void serialize(bitdb::codec::Serializer* serializer) const override {
    serializer->serialize(value);
  }
  void deserialize(bitdb::codec::Serializer* serializer) override {
    serializer->deserialize(&value);
  }

  int value;
};

class ListStateMachine : public bitdb::raft::RaftStateMachine {
 public:
  ListStateMachine();
  ~ListStateMachine() override = default;
  std::vector<char> Snapshot() override;

  void ApplyLog(bitdb::raft::RaftCommand& cmd) override;

  void ApplySnapshot(const std::vector<char>& snapshot) override;

  std::mutex mtx;

  std::vector<int> store;
  int num_append_logs;
};

template <typename StateMachine, typename Command>
class RaftGroup {
 public:
  // typedef raft<list_state_machine, list_command> raft<state_machine,
  // command>;

  explicit RaftGroup(int num, const char* storage_dir = "raft_temp");
  ~RaftGroup();

  int check_exact_one_leader();
  void check_no_leader();

  void set_reliable(bool value);

  void disable_node(int i);

  void enable_node(int i);

  int check_same_term();

  int num_committed(int log_idx);
  int get_committed_value(int log_idx);

  int append_new_command(int value, int expected_servers);

  int wait_commit(int index, int num_committed_server, int start_term);

  int rpc_count(int node);

  int restart(int node);

  std::vector<bitdb::raft::Raft<StateMachine, Command>*> nodes;
  std::vector<bitdb::raft::RpcServerPtr> rpc_servers;
  std::vector<std::shared_ptr<bitdb::net::TcpServer>> tcp_servers;
  std::vector<std::vector<bitdb::raft::RpcClientPtr>> clients;
  std::vector<bitdb::raft::RaftStorage<Command>*> storages;
  std::vector<StateMachine*> states;
};

template <typename StateMachine, typename Command>
RaftGroup<StateMachine, Command>::RaftGroup(int num, const char* storage_dir) {
  nodes.resize(num, nullptr);
  rpc_servers = create_random_rpc_servers(num);
  tcp_servers = create_random_tcp_servers(rpc_servers);
  clients.resize(num);
  states.resize(num);
  storages.resize(num);
  remove_directory(storage_dir);
  // ASSERT(ret == 0 || , "cannot rmdir " << ret);
  CHECK_MESSAGE(mkdir(storage_dir, 0777) >= 0, "cannot create dir ",
                storage_dir);
  for (int i = 0; i < num; i++) {
    std::string dir_name(storage_dir);
    dir_name += "/raft_storage_" + std::to_string(i);

    CHECK_MESSAGE(mkdir(dir_name.c_str(), 0777) >= 0, "cannot create dir ",
                  std::string(storage_dir));

    bitdb::raft::RaftStorage<Command>* storage =
        new bitdb::raft::RaftStorage<Command>(dir_name);
    StateMachine* state = new StateMachine();
    auto client = create_rpc_clients(tcp_servers);
    bitdb::raft::Raft<StateMachine, Command>* node =
        new bitdb::raft::Raft<StateMachine, Command>(rpc_servers[i], client, i,
                                                     storage, state);
    nodes[i] = node;
    clients[i] = client;
    states[i] = state;
    storages[i] = storage;
  }
  for (int i = 0; i < num; i++) {
    nodes[i]->Start();
    std::thread(&bitdb::net::TcpServer::Start, tcp_servers[i].get(), true)
        .detach();
  }

  std::thread([]() {
    bitdb::co::co_join();
    bitdb::co::co_run();
  }).detach();
}

template <typename StateMachine, typename Command>
RaftGroup<StateMachine, Command>::~RaftGroup() {
  set_reliable(true);
  for (size_t i = 0; i < nodes.size(); i++) {
    bitdb::raft::Raft<StateMachine, Command>* node = nodes[i];
    // disbale_node(i);
    node->Stop();
    // for (size_t j = 0; j < nodes.size(); j++) {
    //   clients[i][j]->cancel();
    //   delete clients[i][j];
    // }
    // delete servers[i];
    delete node;
    delete states[i];
    delete storages[i];
  }
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::check_exact_one_leader() {
  int num_checks = 10;
  int num_nodes = nodes.size();
  for (int i = 0; i < num_checks; i++) {
    std::map<int, int> term_leaders;
    for (int j = 0; j < num_nodes; j++) {
      auto* node = nodes[j];
      if (!rpc_servers[j]->reachable()) {
        continue;
      }
      int term = -1;
      bool is_leader = node->IsLeader(term);
      if (is_leader) {
        CHECK_MESSAGE(term > 0,
                      "term " << term << " should not have a leader.");
        CHECK_MESSAGE(term_leaders.find(term) == term_leaders.end(),
                      "term " << term << " has more than one leader.");
        term_leaders[term] = j;
      }
    }
    if (!term_leaders.empty()) {
      auto last_term = term_leaders.rbegin();
      return last_term->second;  // return the leader index
    }
    // sleep a while, in case the election is not successful.
    bitdb::timer::SleepForMS(500 + (random() % 10) * 30);
  }
  CHECK_MESSAGE(0, "There is no leader");  // no leader
  return -1;
}

template <typename StateMachine, typename Command>
void RaftGroup<StateMachine, Command>::check_no_leader() {
  int num_nodes = nodes.size();
  for (int j = 0; j < num_nodes; j++) {
    auto* node = nodes[j];
    auto server = rpc_servers[j];
    if (server->reachable()) {
      int term = -1;
      CHECK_MESSAGE(!node->IsLeader(term),
                    "Node " << j << " is leader, which is unexpected.");
    }
  }
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::check_same_term() {
  int current_term = -1;
  int num_nodes = nodes.size();
  for (int i = 0; i < num_nodes; i++) {
    int term = -1;
    auto* node = nodes[i];
    node->IsLeader(term);  // get the current term
    CHECK_MESSAGE(term >= 0, "invalid term: " << term);
    if (current_term == -1) {
      current_term = term;
    }
    CHECK_MESSAGE(current_term == term,
                  "inconsistent term: " << current_term << ", " << term);
  }
  return current_term;
}

template <typename StateMachine, typename Command>
void RaftGroup<StateMachine, Command>::disable_node(int i) {
  auto server = rpc_servers[i];
  std::vector<bitdb::raft::RpcClientPtr>& client = clients[i];
  server->set_reachable(false);
  for (const auto& c : client) {
    c->set_reachable(false);
  }
}

template <typename StateMachine, typename Command>
void RaftGroup<StateMachine, Command>::enable_node(int i) {
  auto server = rpc_servers[i];
  std::vector<bitdb::raft::RpcClientPtr>& client = clients[i];
  server->set_reachable(true);
  for (const auto& c : client) {
    c->set_reachable(true);
  }
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::num_committed(int log_idx) {
  int cnt = 0;
  int old_value = 0;
  for (size_t i = 0; i < nodes.size(); i++) {
    auto* state = states[i];
    bool has_log;
    int log_value;
    {
      std::unique_lock<std::mutex> lock(state->mtx);
      // fprintf(stderr, "apply_size: %d, log_idx: %d\n",
      // (int)state->store.size(), log_idx);
      if (static_cast<int>(state->store.size()) > log_idx) {
        log_value = state->store[log_idx];
        has_log = true;
      } else {
        has_log = false;
      }
    }
    if (has_log) {
      // fprintf(stderr, "haslog: id: %d log_idx: %d\n", (int)i, log_idx);
      LOG_ERROR("haslog: id: {} log_idx: {}\n", i, log_idx);
      cnt++;
      if (cnt == 1) {
        old_value = log_value;
      } else {
        CHECK_MESSAGE(old_value == log_value, "inconsistent log value: ("
                                                  << log_value << ", "
                                                  << old_value << ") at idx "
                                                  << log_idx);
      }
    }
  }
  return cnt;
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::get_committed_value(int log_idx) {
  for (size_t i = 0; i < nodes.size(); i++) {
    auto* state = states[i];
    int log_value;
    {
      std::unique_lock<std::mutex> lock(state->mtx);
      if (state->store.size() > log_idx) {
        log_value = state->store[log_idx];
        return log_value;
      }
    }
  }
  CHECK_MESSAGE(false, "log " << log_idx << " is not committed.");
  return -1;
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::append_new_command(int value,
                                                         int expected_servers) {
  ListCommand cmd(value);
  auto start = std::chrono::system_clock::now();
  int leader_idx = 0;
  while (std::chrono::system_clock::now() < start + std::chrono::seconds(10)) {
    int log_idx = -1;
    for (size_t i = 0; i < nodes.size(); i++) {
      leader_idx = (leader_idx + 1) % nodes.size();
      // FIXME: lock?
      if (!rpc_servers[leader_idx]->reachable()) {
        continue;
      }

      int temp_idx;
      int temp_term;
      bool is_leader = nodes[leader_idx]->NewCommand(cmd, temp_term, temp_idx);
      if (is_leader) {
        log_idx = temp_idx;
        break;
      }
    }
    if (log_idx != -1) {
      auto check_start = std::chrono::system_clock::now();
      while (std::chrono::system_clock::now() <
             check_start + std::chrono::seconds(2)) {
        int committed_server = num_committed(log_idx);
        if (committed_server >= expected_servers) {
          // The log is committed!
          int commited_value = get_committed_value(log_idx);
          if (commited_value == value) {
            return log_idx;  // and the log is what we want!
          }
        }
        bitdb::timer::SleepForMS(20);
      }
    } else {
      // no leader
      bitdb::timer::SleepForMS(50);
    }
  }
  CHECK_MESSAGE(0, "Cannot make agreement!");
  return -1;
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::wait_commit(int index,
                                                  int num_committed_server,
                                                  int start_term) {
  int sleep_for = 10;  // ms
  for (int iters = 0; iters < 30; iters++) {
    int nc = num_committed(index);
    if (nc >= num_committed_server) {
      break;
    }
    bitdb::timer::SleepForMS(sleep_for);
    if (sleep_for < 1000) {
      sleep_for *= 2;
    }
    if (start_term > -1) {
      for (auto node : nodes) {
        int current_term;
        node->IsLeader(current_term);
        if (current_term > start_term) {
          // someone has moved on
          // can no longer guarantee that we'll "win"
          return -1;
        }
      }
    }
  }
  int nc = num_committed(index);
  CHECK_MESSAGE(nc >= num_committed_server,
                "only " << nc << " decided for index " << index << "; wanted "
                        << num_committed_server);
  return get_committed_value(index);
}

template <typename state_machine, typename command>
int RaftGroup<state_machine, command>::rpc_count(int node) {
  int sum = 0;
  if (node == -1) {
    for (auto& cl : clients) {
      for (auto& cc : cl) {
        sum += cc->count();
      }
    }
  } else {
    for (auto& cc : clients[node]) {
      sum += cc->count();
    }
  }
  return sum;
}

template <typename StateMachine, typename Command>
int RaftGroup<StateMachine, Command>::restart(int node) {
  // std::cout << "restart " << node << std::endl;
  nodes[node]->stop();
  disable_node(node);
  // servers[node]->unreg_all();
  delete nodes[node];
  delete states[node];
  states[node] = new StateMachine();
  bitdb::raft::RaftStorage<Command>* storage =
      new bitdb::raft::RaftStorage<Command>(
          std::string("raft_temp/raft_storage_") + std::to_string(node));
  // recreate clients
  for (auto& cl : clients[node]) {
    cl.reset();
  }
  rpc_servers[node]->set_reachable(true);
  clients[node] = create_rpc_clients(tcp_servers);

  nodes[node] = new bitdb::raft::Raft<StateMachine, Command>(
      rpc_servers[node], clients[node], node, storage, states[node]);
  // disable_node(node);
  nodes[node]->start();
  return 0;
}

template <typename state_machine, typename command>
void RaftGroup<state_machine, command>::set_reliable(bool value) {
  for (const auto& server : rpc_servers) {
    server->set_reliable(value);
  }
}