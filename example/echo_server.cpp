// same as tests/net/echo_server2.cpp

#include "bitdb/common/logger.h"
#include "bitdb/common/thread_pool.h"
#include "bitdb/net/io_awaiter.h"
#include "bitdb/net/tcp_all.h"

using namespace bitdb;

class EchoServer : public bitdb::net::TcpApplication {
 private:
  co::Task<> OnRequest(bitdb::net::TcpConnectionPtr conn,
                       bitdb::net::TcpServer& server) override {
    while (true) {
      bitdb::net::IOBuffer buffer(512);
      ssize_t recv_len = co_await conn->AsyncRead(&buffer);
      if (recv_len < 0) {
        LOG_ERROR("EchoServer read error");
        break;
      }
      if (recv_len == 0) {
        LOG_INFO("client closed");
        break;
      }

      LOG_DEBUG("Done send\n");
      auto res = co_await conn->AsyncWrite(buffer);
      if (res != recv_len) {
        LOG_ERROR("EchoServer write error");
      }
    }
  }
};

int main(int argc, char* argv[]) {
  bitdb::net::InetAddress addr{12345};
  EchoServer app;
  bitdb::net::TcpServer server(addr, &app, 8);
  server.Start();
  LOG_INFO("all down");
}
