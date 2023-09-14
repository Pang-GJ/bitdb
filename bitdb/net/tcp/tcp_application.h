#pragma once

#include <mutex>
#include <unordered_map>

#include "bitdb/co/task.h"
#include "bitdb/net/tcp/tcp_connection.h"

namespace bitdb::net {

class Socket;
class TcpServer;

class TcpApplication {
 public:
  TcpApplication() = default;

  co::Task<> HandleRequest(TcpConnectionPtr conn, TcpServer& server);

 protected:
  virtual co::Task<> OnRequest(TcpConnectionPtr conn, TcpServer& server) = 0;

 private:
  std::mutex mtx_;
  std::unordered_map<int, TcpConnectionWeakPtr> conn_map_;
};

}  // namespace bitdb::net
