#include "bitdb/net/tcp/tcp_application.h"
#include "bitdb/common/logger.h"
#include "bitdb/net/tcp/tcp_server.h"

namespace bitdb::net {

co::Task<> TcpApplication::HandleRequest(TcpConnectionPtr conn,
                                         TcpServer& server) {
  LOG_DEBUG("handing request");
  co_return co_await OnRequest(conn, server);
}

}  // namespace bitdb::net
