#include "bitdb/common/format.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include "bitdb/utils/random.h"
#include "leveldb/db.h"

std::string GetTestKey(int i) { return bitdb::Format("bitdb-key-{}", i); }

std::string GetRandomValue(size_t n) {
  static std::string letters{
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
  static bitdb::Random rd;

  std::string res;
  res.resize(n);
  for (size_t i = 0; i < n; ++i) {
    auto rand_idx = rd.Uniform(letters.size());
    res[i] = letters[rand_idx];
  }

  return bitdb::Format("bitdb-value-{}", res);
}
int main(int argc, char* argv[]) {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  options.max_file_size = 512 * 1024 * 1024;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/test_db", &db);
  assert(status.ok());
  auto write_options = leveldb::WriteOptions();
  auto read_options = leveldb::ReadOptions();

  ankerl::nanobench::Bench().run("leveldb put", [&] {
    for (auto i = 0; i < 10000000; ++i) {
      status = db->Put(write_options, GetTestKey(i), GetRandomValue(128));
      assert(status.ok());
    }
  });

  ankerl::nanobench::Bench().run("leveldb get", [&] {
    std::string val;
    for (auto i = 0; i < 10000000; ++i) {
      status = db->Get(read_options, GetTestKey(i), &val);
      assert(status.ok());
    }
  });

  return 0;
}