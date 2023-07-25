#include "bitdb/db.h"
#include "bitdb/utils/defer.h"
#include "bitdb/utils/random.h"

void DestroyDB(bitdb::DB* db) {
  if (db != nullptr) {
    auto dir_path = db->GetDirPath();
    bitdb::DB::Close(&db);
    bitdb::RemoveDir(dir_path);
  }
}

inline std::string GetTestKey(int i) {
  return bitdb::Format("bitdb-key-{}", i);
}

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

std::string TestPut(bitdb::DB* db) {
  auto value = GetRandomValue(128);
  for (auto i = 0; i < 5000000; ++i) {
    const std::string key = "bitdb-key-" + std::to_string(i);
    auto status = db->Put(key, value);
    assert(status.IsOk());
  }
  return value;
}

void TestGet(bitdb::DB* db, const std::string& ans) {
  for (auto i = 0; i < 5000000; ++i) {
    std::string value;
    const std::string key = "bitdb-key-" + std::to_string(i);
    auto status = db->Get(key, &value);
    assert(value == ans);
    assert(status.IsOk());
  }
}

int main(int argc, char* argv[]) {
  bitdb::DB* db = nullptr;
  defer { DestroyDB(db); };
  auto options = bitdb::DefaultOptions();
  options.data_file_size = 16 * 1024 * 1024;
  options.index_type = bitdb::index::SkipListIndex;
  auto status = bitdb::DB::Open(options, &db);
  assert(status.IsOk());
  assert(db);
  auto value = TestPut(db);
  TestGet(db, value);
  return 0;
}