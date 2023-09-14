#pragma once

#include <cstdint>
#include "bitdb/codec/serializer.h"
#include "bitdb/net/rpc/invoke_helper.h"
namespace bitdb::net::rpc {

// 暂时将请求大小限制为 64K
constexpr int MAX_VALUE_SIZE = 1024 * 64;

template <typename T>
struct RpcResponse {
  using type = typename type_xx<T>::type;
  using msg_type = std::string;
  using code_type = uint16_t;

  RpcResponse() = default;
  ~RpcResponse() = default;

  T val() const { return detail_value; }

  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(err_code);
    serializer->serialize(err_msg);
    serializer->serialize(detail_value);
  }

  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&err_code);
    serializer->deserialize(&err_msg);
    serializer->deserialize(&detail_value);
  }

  code_type err_code{0};
  msg_type err_msg{};
  type detail_value{};
};

}  // namespace bitdb::net::rpc