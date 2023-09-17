#pragma once

#include <memory>
#include "bitdb/co/task.h"
#include "bitdb/common/noncopyable.h"
#include "bitdb/net/event_manager.h"
#include "bitdb/net/socket.h"
#include "bitdb/net/tcp/tcp_connection.h"
namespace bitdb::net {

class TcpClient : noncopyable {
 public:
  TcpClient();
  ~TcpClient();

  co::Task<TcpConnectionPtr> connect(std::string_view server_ip,
                                     int server_port, int retry_times = 50);

 private:
  std::shared_ptr<EventManager> reactor_;
  std::thread reactor_thread_;
};

}  // namespace bitdb::net