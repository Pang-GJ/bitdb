#include "bitdb/net/tcp/tcp_client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <memory>
#include <string_view>
#include <thread>
#include "bitdb/common/logger.h"
#include "bitdb/net/event_manager.h"
#include "bitdb/net/socket.h"
#include "bitdb/net/tcp/tcp_connection.h"

namespace bitdb::net {

TcpClient::TcpClient() {
  reactor_ = std::make_shared<EventManager>(nullptr, 4);
  reactor_thread_ = std::thread([&]() { reactor_->Start(); });
}

TcpClient::~TcpClient() {
  reactor_->Shutdown();
  if (reactor_thread_.joinable()) {
    reactor_thread_.join();
  }
}

co::Task<TcpConnectionPtr> TcpClient::connect(std::string_view server_ip,
                                              int server_port,
                                              int retry_times) {
  auto sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
  if (sock_fd == -1) {
    LOG_ERROR("create socket error");
    co_return nullptr;
  }
  auto socket = std::make_shared<Socket>(sock_fd);
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip.data());

  while ((retry_times--) != 0) {
    int res =
        ::connect(sock_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                  sizeof(server_addr));
    if (res == 0) {
      break;
    }
    if (res < 0) {
      if (errno == EINPROGRESS) {
        continue;
      }
      LOG_FATAL("blocking rpc client connect failed, errno: {}", errno);
    }
  }
  co_return std::make_shared<TcpConnection>(socket, *reactor_);
}

}  // namespace bitdb::net