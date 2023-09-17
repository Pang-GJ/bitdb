#include <charconv>
#include <cstring>
#include <utility>

#include "bitdb/common/logger.h"
#include "bitdb/net/event_manager.h"
#include "bitdb/net/io_awaiter.h"
#include "bitdb/net/tcp/tcp_connection.h"

namespace bitdb::net {

TcpConnection::TcpConnection(std::shared_ptr<Socket> sock,
                             EventManager& event_manager)
    : event_manager_(event_manager), socket_(std::move(sock)) {}

TcpConnection::~TcpConnection() {
  LOG_DEBUG("TcpConnection destroy");
  if (socket_->GetFd() != -1) {
    if (socket_->Attached()) {
      event_manager_.Detach(GetSocket());
    }
  }
}

co::Task<size_t> TcpConnection::AsyncRead(IOBuffer* buffer) {
  co_return co_await ::bitdb::net::AsyncRead(this, *buffer);
}

co::Task<size_t> TcpConnection::AsyncWrite(const IOBuffer& buffer) {
  co_return co_await ::bitdb::net::AsyncWrite(this, buffer);
}

co::Task<bool> TcpConnection::AsyncReadPacket(IOBuffer* buffer) {
  co_return co_await ::bitdb::net::AsyncReadPacket(this, *buffer);
}

co::Task<bool> TcpConnection::AsyncWritePacket(const IOBuffer& buffer) {
  co_return co_await ::bitdb::net::AsyncWritePacket(this, buffer);
}

}  // namespace bitdb::net
