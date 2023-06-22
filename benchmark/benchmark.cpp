#include <cassert>
#include "bitdb/db.h"
#include "bitdb/index/index.h"
#include "bitdb/options.h"
#include "bitdb/utils/defer.h"
#include "bitdb/utils/random.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

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
  for (size_t i = 0; i < n; ++i) {
    auto rand_idx = rd.Uniform(letters.size());
    res[i] = letters[rand_idx];
  }

  return bitdb::Format("bitdb-value-{}", res);
}

int main() {
  bitdb::DB* db = nullptr;
  defer { DestroyDB(db); };
  auto options = bitdb::DefaultOptions();
  options.data_file_size = 512 * 1024 * 1024;
  options.index_type = bitdb::index::TreeMapIndex;
  auto status = bitdb::DB::Open(options, &db);
  assert(status.IsOk());
  assert(db);

  ankerl::nanobench::Bench().run("db put", [&] {
    for (auto i = 0; i < 1000000; ++i) {
      status = db->Put(GetTestKey(i), GetRandomValue(128));
      assert(status.IsOk());
    }
  });

  ankerl::nanobench::Bench().run("db get", [&] {
    std::string val;
    for (auto i = 0; i < 1000000; ++i) {
      status = db->Get(GetTestKey(i), &val);
      assert(status.IsOk());
    }
  });
}