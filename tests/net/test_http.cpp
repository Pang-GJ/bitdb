#include <string>
#include "bitdb/common/json.h"
#include "bitdb/common/logger.h"
#include "bitdb/net/http_all.h"
#include "bitdb/net/tcp_all.h"

using namespace bitdb;

int main(int argc, char* argv[]) {
  bitdb::net::InetAddress addr{12345};
  bitdb::net::http::HttpServer http_app;

  http_app.GET("/ping/", [](const bitdb::net::http::ContextPtr& ctx) {
    ctx->HTML(200, "Pong");
  });

  http_app.POST("/add/", [](const bitdb::net::http::ContextPtr& ctx) {
    LOG_INFO("POST run: {}", ctx->req_->body_.c_str());

    auto param1 = ctx->QueryBody("param1");
    auto param2 = ctx->QueryBody("param2");
    if (param1.empty() || param2.empty()) {
      ctx->HTML(404, "Error Param");
      return;
    }
    auto res = atoi(param1.c_str()) + atoi(param2.c_str());
    ctx->HTML(200, Format("res: {}\n", res));
  });

  bitdb::net::TcpServer server(addr, &http_app, 8);
  server.Start();
  LOG_INFO("all down");

  return 0;
}
