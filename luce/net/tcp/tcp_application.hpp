#pragma once

#include <mutex>
#include <unordered_map>

#include "luce/coro/task.hpp"
#include "luce/net/tcp/tcp_connection.hpp"

namespace net {

class Socket;
class TcpServer;

class TcpApplication {
 public:
  TcpApplication() = default;

  co::Task<> HandleRequest(TcpConnectionPtr conn, TcpServer &server);

 protected:
  virtual co::Task<> OnRequest(TcpConnectionPtr conn, TcpServer &server) = 0;

 private:
  std::mutex mtx_;
  std::unordered_map<int, TcpConnectionWeakPtr> conn_map_;
};

}  // namespace net
