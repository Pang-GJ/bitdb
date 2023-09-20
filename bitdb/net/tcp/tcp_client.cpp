#include "bitdb/net/tcp/tcp_client.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <memory>
#include <string_view>
#include <thread>

namespace bitdb::net {

TcpClient::TcpClient() {
  reactor_ = std::make_shared<EventManager>(nullptr, 4);
  reactor_thread_ = std::thread(&EventManager::Start, reactor_);
}

TcpClient::~TcpClient() {
  reactor_->Shutdown();
  reactor_thread_.join();
}

co::Task<TcpConnectionPtr> TcpClient::connect(std::string_view server_ip,
                                              int server_port,
                                              int retry_times) {
  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip.data());

  auto ptr = co_await this->connect(
      reinterpret_cast<struct sockaddr*>(&server_addr), retry_times);
  co_return ptr;
}

co::Task<TcpConnectionPtr> TcpClient::connect(sockaddr* sock_addr,
                                              int retry_times) {
  auto sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
  if (sock_fd == -1) {
    LOG_ERROR("create socket error");
    co_return nullptr;
  }
  auto socket_ptr = std::make_shared<Socket>(sock_fd);
  while ((retry_times--) != 0) {
    int res = ::connect(sock_fd, sock_addr, sizeof(struct sockaddr));
    if (res == 0) {
      break;
    }
    if (res < 0) {
      if (errno == EINPROGRESS) {
        continue;
      }
      LOG_FATAL(
          "coroutine rpc client connect failed, errno: {}, description: {}",
          errno, strerror(errno));
    }
  }
  co_return std::make_shared<TcpConnection>(socket_ptr, *reactor_);
}

TcpConnectionPtr TcpClient::block_connect(std::string_view server_ip,
                                          int server_port, int retry_times) {
  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip.data());

  return block_connect(reinterpret_cast<struct sockaddr*>(&server_addr),
                       retry_times);
}

TcpConnectionPtr TcpClient::block_connect(sockaddr* sock_addr,
                                          int retry_times) {
  auto sock_fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
  if (sock_fd == -1) {
    LOG_ERROR("create socket error");
    return nullptr;
  }
  auto socket_ptr = std::make_shared<Socket>(sock_fd);
  while ((retry_times--) != 0) {
    int res = ::connect(sock_fd, sock_addr, sizeof(struct sockaddr));
    if (res == 0) {
      break;
    }
    if (res < 0) {
      if (errno == EINPROGRESS) {
        continue;
      }
      LOG_FATAL("block rpc client connect failed, errno: {}, description: {}",
                errno, strerror(errno));
    }
  }
  return std::make_shared<TcpConnection>(socket_ptr, *reactor_);
}
}  // namespace bitdb::net