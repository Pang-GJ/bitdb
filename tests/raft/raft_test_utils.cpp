#include "tests/raft/raft_test_utils.h"
#include <dirent.h>
#include <memory>
#include <mutex>
#include "bitdb/co/scheduler.h"
#include "bitdb/codec/serializer.h"
#include "bitdb/net/rpc/rpc_client.h"
#include "bitdb/net/rpc/rpc_server.h"
#include "bitdb/net/tcp/tcp_client.h"
#include "bitdb/net/tcp/tcp_server.h"
#include "bitdb/raft/raft.h"
#include "bitdb/timer/time_utils.h"

ListStateMachine::ListStateMachine() : num_append_logs(0) {
  store.emplace_back(0);
}

std::vector<char> ListStateMachine::Snapshot() {
  bitdb::codec::Serializer serializer;
  serializer.serialize(store);
  return {serializer.cbegin(), serializer.cend()};
}

void ListStateMachine::ApplyLog(bitdb::raft::RaftCommand& cmd) {
  std::lock_guard lock(mtx);
  const ListCommand& list_cmd = dynamic_cast<const ListCommand&>(cmd);
  store.emplace_back(list_cmd.value);
  num_append_logs++;
}

void ListStateMachine::ApplySnapshot(const std::vector<char>& snapshot) {
  bitdb::codec::Serializer serializer(snapshot.begin(), snapshot.end());
  std::lock_guard lock(mtx);
  serializer.deserialize(&store);
}

int remove_directory(const char* path) {
  DIR* d = opendir(path);
  size_t path_len = strlen(path);
  int r = -1;

  if (d != nullptr) {
    struct dirent* p;

    r = 0;
    while ((r == 0) && ((p = readdir(d)) != nullptr)) {
      int r2 = -1;
      char* buf;
      size_t len;

      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if ((strcmp(p->d_name, ".") == 0) || (strcmp(p->d_name, "..") == 0)) {
        continue;
      }

      len = path_len + strlen(p->d_name) + 2;
      buf = static_cast<char*>(malloc(len));

      if (buf != nullptr) {
        struct stat statbuf;

        snprintf(buf, len, "%s/%s", path, p->d_name);
        if (stat(buf, &statbuf) == 0) {
          if (S_ISDIR(statbuf.st_mode)) {
            r2 = remove_directory(buf);
          } else {
            r2 = unlink(buf);
          }
        }
        free(buf);
      }
      r = r2;
    }
    closedir(d);
  }

  if (r == 0) {
    r = rmdir(path);
  }

  return r;
}

void mssleep(int ms) { bitdb::timer::SleepForMS(ms); }

std::vector<std::shared_ptr<bitdb::net::TcpServer>> create_random_tcp_servers(
    std::vector<bitdb::raft::RpcServerPtr>& rpc_servers) {
  std::vector<std::shared_ptr<bitdb::net::TcpServer>> tcp_servers;
  static uint16_t port = 33333;
  const auto num = rpc_servers.size();
  for (int i = 0; i < num; ++i) {
    bitdb::net::InetAddress addr{port};
    port++;
    tcp_servers.emplace_back(
        std::make_shared<bitdb::net::TcpServer>(addr, rpc_servers[i].get(), 4));
  }
  return tcp_servers;
}

std::vector<bitdb::raft::RpcServerPtr> create_random_rpc_servers(int num) {
  std::vector<bitdb::raft::RpcServerPtr> rpc_servers{};
  for (int i = 0; i < num; ++i) {
    rpc_servers.emplace_back(  // NOLINT
        std::make_shared<bitdb::net::rpc::RpcServer>());
  }
  return rpc_servers;
}

std::vector<bitdb::raft::RpcClientPtr> create_rpc_clients(
    const std::vector<std::shared_ptr<bitdb::net::TcpServer>>& tcp_servers) {
  const auto num = tcp_servers.size();
  std::vector<bitdb::raft::RpcClientPtr> rpc_clients;
  // for (int i = 0; i < num; ++i) {
  //   rpc_clients.emplace_back(
  //       std::make_shared<bitdb::net::rpc::RpcClient>(tcp_client));
  //   const auto addr = tcp_servers[i]->GetLocalAddr();
  //   LOG_INFO("try to connect servers {}", i);

  //   if (!rpc_clients[i]->BlockConnect(addr.GetSockAddr())) {
  //     LOG_ERROR("client {} connect failed", i);
  //   }
  // }
  int port = 33333;
  auto tcp_client = std::make_shared<bitdb::net::TcpClient>();
  for (int i = 0; i < num; ++i) {
    rpc_clients.emplace_back(
        std::make_shared<bitdb::net::rpc::RpcClient>(tcp_client));
    if (!rpc_clients[i]->BlockConnect("127.0.0.1", port)) {
      LOG_ERROR("client {} connect failed", i);
    }
  }
  return rpc_clients;
}
