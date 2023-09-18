#include <string>
#include "bitdb/co/scheduler.h"
#include "bitdb/codec/type_helper.h"
#include "bitdb/net/inet_address.h"
#include "bitdb/net/rpc/rpc_server.h"

using namespace bitdb;  // NOLINT

struct Student {
  std::string name;
  int age;

  void serialize(codec::Serializer* serializer) const {
    serializer->serialize(name);
    serializer->serialize(age);
  }

  void deserialize(codec::Serializer* serializer) {
    serializer->deserialize(&name);
    serializer->deserialize(&age);
  }
};

Student get_stu(const std::string& name, int age) { return {name, age}; }

int add(int a, int b) {
  auto res = a + b;
  LOG_INFO("recv add request: {} + {} = {}", a, b, res);
  return res;
}

int main(int argc, char* argv[]) {
  bitdb::net::InetAddress addr{12345};
  bitdb::net::rpc::RpcServer rpc_app;

  rpc_app.Bind("add", add);
  rpc_app.Bind("get_stu", get_stu);

  bitdb::net::TcpServer server(addr, &rpc_app, 8);
  server.Start();
  LOG_INFO("server end");
  co::co_join();
  co::co_run();
  return 0;
}
