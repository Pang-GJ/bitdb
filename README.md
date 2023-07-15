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
