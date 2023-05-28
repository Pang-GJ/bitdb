#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include "bitdb/utils/bytes.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <memory>
#include <string>
#include "bitdb/io/file_io.h"
#include "bitdb/utils/defer.h"

using bitdb::io::FileIO;
using bitdb::Bytes;

void DestroyFile(const std::string& name) {
  if (unlink(name.c_str()) != 0) {
    throw std::runtime_error("Failed to remove file");
  }
}

TEST_CASE("new file io manager") {
  std::string filepath = "/tmp/a.data";
  auto fileio = std::make_unique<bitdb::io::FileIO>(filepath);
  defer { DestroyFile(filepath); };
  CHECK_NE(fileio, nullptr);
}

TEST_CASE("test file io write") {
  std::string filepath = "/tmp/a.data";
  auto fileio = std::make_unique<bitdb::io::FileIO>(filepath);
  defer { DestroyFile(filepath); };
  CHECK_NE(fileio, nullptr);

  auto n = fileio->Write(Bytes{""});
  CHECK_EQ(0, n);

  n = fileio->Write(Bytes{"BitDB"});
  CHECK_EQ(5, n);

  n = fileio->Write(Bytes{"BitDB1"});
  CHECK_EQ(6, n);

  n = fileio->Write(Bytes{"BitDBBitDBBitDB"});
  CHECK_EQ(15, n);
}

TEST_CASE("test file io read") {
  std::string filepath = "/tmp/a.data";
  auto fileio = std::make_unique<bitdb::io::FileIO>(filepath);
  defer { DestroyFile(filepath); };
  CHECK_NE(fileio, nullptr);

  auto n = fileio->Write(Bytes{"key-a"});
  CHECK_EQ(5, n);

  n = fileio->Write(Bytes{"key-b"});
  CHECK_EQ(5, n);

  char val1[5] = "";
  char val2[5] = "";

  auto r1 = fileio->Read(val1, 5, 0);
  CHECK_EQ(std::strcmp(val1, "key-a"), 0);
  CHECK_EQ(r1, 5);

  auto r2 = fileio->Read(val2, 5, 5);
  CHECK_EQ(std::strcmp(val2, "key-b"), 0);
  CHECK_EQ(r2, 5);
}

TEST_CASE("test file io sync") {
  std::string filepath = "/tmp/a.data";
  auto fileio = std::make_unique<bitdb::io::FileIO>(filepath);
  defer { DestroyFile(filepath); };
  CHECK_NE(fileio, nullptr);

  auto res = fileio->Sync();
  CHECK_EQ(res, 0);
}