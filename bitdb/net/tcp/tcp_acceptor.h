#pragma once

#include "bitdb/net/event_manager.h"
#include "bitdb/net/tcp/tcp_connection.h"
#include "bitdb/net/tcp/tcp_server.h"

namespace bitdb::net {

class TcpAcceptor {
 public:
  explicit TcpAcceptor(TcpServer& server, int sock_fd);

  ~TcpAcceptor() { LOG_INFO("delete acceptor"); }

  co::Task<TcpConnectionPtr> accept();

  auto GetEventManager() const -> EventManager& {
    return server_.GetMainReactor();
  }

  auto GetSocket() -> std::shared_ptr<Socket> { return socket_; }

 private:
  TcpServer& server_;
  std::shared_ptr<Socket> socket_;
};

};  // namespace bitdb::net
