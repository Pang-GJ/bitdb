#include <chrono>
#include <string>
#include <thread>
#include "bitdb/co/scheduler.h"
#include "bitdb/codec/serializer.h"
#include "bitdb/net/rpc/rpc_client.h"
#include "bitdb/net/rpc_all.h"

using namespace bitdb;  // NOLINT
using bitdb::co::co_join;
using bitdb::co::co_run;
using bitdb::co::co_spawn;

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

bitdb::co::Task<> co_main(std::shared_ptr<bitdb::net::TcpClient> tcp_client) {
  bitdb::net::rpc::RpcClient rpc_client(tcp_client);
  auto connect_res = co_await rpc_client.Connect("127.0.0.1", 12345);
  if (!connect_res) {
    LOG_ERROR("coroutine rpc cient connect error");
  }
  auto rpc_response1 = co_await rpc_client.Call<int>("add", 2, 3);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  LOG_INFO("call add response: {}", rpc_response1.val());

  auto rpc_response2 = co_await rpc_client.Call<Student>("get_stu", "pgj", 21);
  Student stu_res = rpc_response2.val();
  LOG_INFO("call get_stu response: name: {}, age: {}", stu_res.name,
           stu_res.age);
}

int main(int argc, char* argv[]) {
  bitdb::net::rpc::BlockingRpcClient client("127.0.0.1", 12345);
  int res = client.Call<int>("add", 2, 3).val();
  LOG_INFO("blocking call add response: {}", res);

  Student stu_res = client.Call<Student>("get_stu", "pgj", 21).val();
  LOG_INFO("bloking call get_stu response: name: {}, age: {}", stu_res.name,
           stu_res.age);

  auto tcp_client = std::make_shared<bitdb::net::TcpClient>();
  co_spawn(co_main(tcp_client));
  co_run();
  co_join();
  return 0;
}