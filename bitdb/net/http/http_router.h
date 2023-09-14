#pragma once

#include "bitdb/common/logger.h"
#include "bitdb/common/string_utils.h"
#include "bitdb/common/format.h"
#include "bitdb/net/http/http_context.h"

#include <string>
#include <unordered_map>

namespace bitdb::net::http {

using HandleFunc = std::function<void(ContextPtr)>;

struct Router {
  void AddRouter(std::string_view method, std::string_view url,
                 const HandleFunc& handler) {
    if (method == "DELETE") {
      LOG_ERROR("could not DELETE now");
      return;
    }
    if (!String::EndsWith(url, "/")) {
      LOG_ERROR("register handler for {}: {} failed, url must ends with '/'",
                method.data(), url.data());
      return;
    }
    LOG_INFO("method: {}, url: {}", method.data(), url.data());
    auto key = String::Join({method, url}, "-");
    LOG_INFO("AddRouter key: {}", key.c_str());
    handlers_[key] = handler;
  }

  void Handle(const ContextPtr& ctx) {
    auto key = String::Join({ctx->method_, ctx->path_}, "-");
    if (handlers_.contains(key)) {
      const auto& handler = handlers_[key];
      handler(ctx);
    } else {
      ctx->HTML(404, Format("404 Not Found: {} not at {}\n",
                                    ctx->method_, ctx->path_));
    }
  }

  std::unordered_map<std::string, HandleFunc> handlers_{};
};

}  // namespace bitdb::net::http
