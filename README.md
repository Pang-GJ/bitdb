# bitdb
## warning
经测试，协程网络库 rpc 部分存在bug，目前忙于毕设项目暂未修复。
## 简介
基于`C++20 coroutine`的“高性能”epoll网络库，使用协程可以方便地用同步的代码写出异步的效果。

ps: “高性能”是指单看echo_server，性能跟`muduo`接近，没有进行更多的性能测试。  
同时`muduo`有很多内部的优化技巧，本项目暂未优化一些内部实现，所以有不少的提升空间。

### 期望
C++20带来了无栈协程，但是不方便普通用户使用，目前的标准库适合库作者使用。  
我想慢慢地封装一层协程异步框架，计划参考`rust`中的实现。

### 编译运行
本项目采用了`xmake`作为构建系统、依赖管理。
```
# 编译
xmake build
# 运行echo_server
xmake run echo_server
```

## 目前已实现的部分

- IO多路复用（epoll）+ MultiReactor
- C++20协程支持 co_await/co_return
    - 简单使用co::Task<>即可令函数成为协程
- 线程池
- EventManager/Reactor
- TcpServer/TcpAcceptor/TcpConnection/TcpApplication
- [高性能异步日志](https://github.com/Pang-GJ/plog)
  - 没有合并异步日志，仍在开发中，计划封装一下`fmt`或者`std::format`TODO
- [json解析](https://github.com/Pang-GJ/tinyjson)
- [类似`gin`的restful HTTP框架](./docs/http.md)

## 用法

参考`example/echo_server.cpp`  
用户需要继承`TcpApplication`(位于`include/net/tcp_application.h`)
实现自己的Tcp应用程序，  
具体来说要实现三个函数：

```cpp
 virtual co::Task<> OnRequest(TcpConnectionPtr conn,
                                     TcpServer &server) = 0;
 virtual co::Task<> OnOpen(TcpConnectionPtr conn) = 0;
 virtual co::Task<> OnClose(TcpConnectionPtr conn) = 0;
```

`OnRequest`表示请求到来时的回调（已经使用协程尽量减少回调的使用，但总有一些不好去除的回调，如果你有更好的想法，欢迎跟我讨论）。  
`OnOpen`表示连接刚开启的回调  
`OnClose`表示连接关闭时的回调

## 网络模型

Reactor

用户所在的线程运行main_reactor，负责监听到来的请求连接，连接后交由sub_reactor进行读写

## RoadMap

- 协程
    - 目前的协程使用感觉还不是很容易
    - 缺少了不少组件，例如调度器、同步原语
- io_uring
- HTTP封装
  - 实现类似`gin`的简易HTTP框架
- 简单protobuf RPC实现
    - 更进一步可以使用自己实现的序列化
    - TODO: 不用写`IDL`的RPC
- Zero Copy
# bitdb
bitdb 是一款基于 bitcask 存储模型的 nosql 数据库，目前暂时实现了存储引擎部分功能，后续会合并协程网络库、实现 Redis 数据结构、实现 raft 算法构建分布式 NoSQL 数据库。

## 使用
```cpp
  auto options = bitdb::DefaultOptions();
  options.dir_path = "/tmp/bitdb";
  bitdb::DB* db = nullptr;
  auto status = bitdb::DB::Open(options, &db);
  if (!status.IsOk()) {
    LOG_ERROR("Open db failed. reason: {}", status.ToString());
    exit(-1);
  }
  status = db->Put("name", "bitdb-example");
  if (!status.IsOk()) {
    LOG_ERROR("DB put failed. reason: {}", status.ToString());
    exit(-1);
  }

  std::string value;
  status = db->Get("name", &value);
  if (!status.IsOk()) {
    LOG_ERROR("DB get failed. reason: {}", status.ToString());
    exit(-1);
  }
  LOG_INFO("value: {}", value);

  status = db->Delete("name");
  if (!status.IsOk()) {
    LOG_ERROR("DB delete failed. reason: {}", status.ToString());
    exit(-1);
  }
  status = db->Get("name", &value);
  if (!status.IsNotFound()) {
    LOG_ERROR("DB item still can be founded after delete. reason: {}",
              status.ToString());
    exit(-1);
  }
```

## RoadMap
- 优化读写 IO
- Redis 数据结构
- 合并协程网络库
- 实现 raft 算法
