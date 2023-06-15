#include <algorithm>
#include "bitdb/index/index.h"
#include "bitdb/options.h"
#include "bitdb/utils/defer.h"
#include "bitdb/utils/logger.h"
#include "bitdb/utils/os_utils.h"
#include "bitdb/utils/random.h"
#include "bitdb/utils/string_utils.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/db.h"

void DestroyDB(bitdb::DB* db) {
  if (db != nullptr) {
    auto dir_path = db->GetDirPath();
    bitdb::DB::Close(&db);
    bitdb::RemoveDir(dir_path);
  }
}

std::string GetTestKey(int i) { return bitdb::Format("bitdb-key-{}", i); }

std::string GetRandomValue(size_t n) {
  static std::string letters{
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
  static bitdb::Random rd;

  std::string res;
  res.resize(n);
  for (int i = 0; i < n; ++i) {
    auto rand_idx = rd.Uniform(letters.size());
    res[i] = letters[rand_idx];
  }

  return bitdb::Format("bitdb-value-{}", res);
}

TEST_CASE("Open DB") {
  bitdb::DB* db = nullptr;
  defer { DestroyDB(db); };
  auto options = bitdb::DefaultOptions();
  options.dir_path = "/tmp/bitdb";
  auto status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);
}

TEST_CASE("DB Put") {
  bitdb::DB* db = nullptr;
  defer { DestroyDB(db); };
  auto options = bitdb::DefaultOptions();
  options.dir_path = "/tmp/bitdb-put";
  options.data_file_size = 64 * 1024 * 1024;
  options.index_type = bitdb::index::HashMapIndex;
  auto status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);

  // 写入一条新数据
  auto val1 = GetRandomValue(24);
  status = db->Put(GetTestKey(1), val1);
  CHECK_EQ(true, status.IsOk());
  std::string val_got;
  status = db->Get(GetTestKey(1), &val_got);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(val1, val_got);

  // 重复 put key相同的数据
  status = db->Put(GetTestKey(1), GetRandomValue(26));
  CHECK_EQ(true, status.IsOk());
  std::string val_got2;
  status = db->Get(GetTestKey(1), &val_got2);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(val1, val_got2);

  // key 为空
  status = db->Put("", GetRandomValue(24));
  CHECK_EQ(true, status.IsInvalidArgument());

  // value 为空
  status = db->Put(GetTestKey(2), "");
  CHECK_EQ(true, status.IsOk());
  status = db->Get(GetTestKey(2), &val_got);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(true, val_got.empty());

  // 写入大量数据，写到新建了数据文件
  for (auto i = 0; i < 1000000; ++i) {
    status = db->Put(GetTestKey(i), GetRandomValue(128));
    CHECK_EQ(true, status.IsOk());
  }
  CHECK_GE(db->GetOlderDataFileNum(), 2);

  // 重启后再 Put 数据
  status = bitdb::DB::Close(&db);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(nullptr, db);
  status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);

  auto val2 = GetRandomValue(128);
  status = db->Put(GetTestKey(55), val2);
  CHECK_EQ(true, status.IsOk());
  status = db->Get(GetTestKey(55), &val_got);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(val2, val_got);
}

TEST_CASE("DB Get") {
  bitdb::DB* db = nullptr;
  defer { DestroyDB(db); };
  auto options = bitdb::DefaultOptions();
  options.dir_path = "/tmp/bitdb-get";
  options.data_file_size = 64 * 1024 * 1024;
  options.index_type = bitdb::index::HashMapIndex;
  auto status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);

  // 正常读取一个写入的值
  std::string val1 = GetRandomValue(24);
  status = db->Put(GetTestKey(11), val1);
  CHECK_EQ(true, status.IsOk());
  std::string got_val1;
  status = db->Get(GetTestKey(11), &got_val1);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(val1, got_val1);

  // 读取一个不存在的值
  status = db->Get("Key don't exist", &got_val1);
  CHECK_EQ(true, status.IsNotFound());

  // 在旧数据文件上读
  for (auto i = 0; i < 1000000; ++i) {
    status = db->Put(GetTestKey(i), GetRandomValue(128));
    CHECK_EQ(true, status.IsOk());
  }
  CHECK_GE(db->GetOlderDataFileNum(), 2);
  status = db->Get(GetTestKey(101), &got_val1);
  CHECK_EQ(true, status.IsOk());

  // 重启后前面写的数据应该都能拿到
  status = bitdb::DB::Close(&db);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(nullptr, db);
  status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);

  status = db->Get(GetTestKey(111), &got_val1);
  CHECK_EQ(true, status.IsOk());
  LOG_INFO("111 key, value: {}", got_val1);

  status = db->Get(GetTestKey(222), &got_val1);
  CHECK_EQ(true, status.IsOk());
  LOG_INFO("222 key, value: {}", got_val1);

  status = db->Get(GetTestKey(333), &got_val1);
  CHECK_EQ(true, status.IsOk());
  LOG_INFO("333 key, value: {}", got_val1);
}

TEST_CASE("DB Delete") {
  bitdb::DB* db = nullptr;
  defer { DestroyDB(db); };
  auto options = bitdb::DefaultOptions();
  options.dir_path = "/tmp/bitdb-del";
  options.data_file_size = 64 * 1024 * 1024;
  options.index_type = bitdb::index::HashMapIndex;
  auto status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);

  // 正常删除一个存在的 key
  status = db->Put(GetTestKey(11), GetRandomValue(12));
  CHECK_EQ(true, status.IsOk());
  status = db->Delete(GetTestKey(11));
  CHECK_EQ(true, status.IsOk());
  std::string got_val;
  status = db->Get(GetTestKey(11), &got_val);
  CHECK_EQ(true, status.IsNotFound());

  // 删除一个不存在的 key
  status = db->Delete(GetTestKey(12));
  CHECK_EQ(true, status.IsOk());

  // 删除一个空的key
  status = db->Delete("");
  CHECK_EQ(true, status.IsInvalidArgument());

  // 值被删除之后重新 put
  status = db->Put(GetTestKey(13), GetRandomValue(123));
  CHECK_EQ(true, status.IsOk());
  status = db->Delete(GetTestKey(13));
  CHECK_EQ(true, status.IsOk());
  status = db->Put(GetTestKey(13), GetRandomValue(123));
  std::string val1;
  status = db->Get(GetTestKey(13), &val1);
  CHECK_EQ(true, status.IsOk());

  // 重启之后再校验
  status = bitdb::DB::Close(&db);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(nullptr, db);
  status = bitdb::DB::Open(options, &db);
  CHECK_EQ(true, status.IsOk());
  CHECK_NE(nullptr, db);

  std::string val2;
  status = db->Get(GetTestKey(11), &val2);
  CHECK_EQ(true, status.IsNotFound());
  status = db->Get(GetTestKey(13), &val2);
  CHECK_EQ(true, status.IsOk());
  CHECK_EQ(val1, val2);
}