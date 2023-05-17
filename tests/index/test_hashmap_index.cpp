#include <memory>
#include "bitdb/data/log_record.h"
#include "bitdb/index/hashmap_index.h"
#include "bitdb/utils/bytes.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using bitdb::data::LogRecordPst;
using bitdb::index::HashIndexer;

TEST_CASE("test index put") {
  auto indexer = std::make_unique<HashIndexer>();
  LogRecordPst pst1{.fid = 1, .offset = 100};
  auto res1 = indexer->Put(Bytes{"a"}, &pst1);
  CHECK_EQ(true, res1);

  LogRecordPst pst2{.fid = 1, .offset = 200};
  auto res2 = indexer->Put(Bytes{"b"}, &pst2);
  CHECK_EQ(true, res2);
}

TEST_CASE("test index get") {
  auto indexer = std::make_unique<HashIndexer>();
  LogRecordPst pst0{.fid = 1, .offset = 100};
  auto res0 = indexer->Put(Bytes{""}, &pst0);
  CHECK_EQ(true, res0);

  auto* res_pos = indexer->Get(Bytes{""});
  CHECK_EQ(1, res_pos->fid);
  CHECK_EQ(100, res_pos->offset);

  LogRecordPst pst1{.fid = 1, .offset = 200};
  auto res1 = indexer->Put(Bytes{"a"}, &pst1);
  CHECK_EQ(true, res1);
  LogRecordPst pst2{.fid = 1, .offset = 300};
  auto res2 = indexer->Put(Bytes{"a"}, &pst2);
  CHECK_EQ(true, res2);

  auto* res_pos2 = indexer->Get(Bytes{"a"});
  CHECK_EQ(1, res_pos2->fid);
  CHECK_EQ(300, res_pos2->offset);
}

TEST_CASE("test index delete") {
  auto indexer = std::make_unique<HashIndexer>();
  LogRecordPst pst0{.fid = 1, .offset = 100};
  auto res0 = indexer->Put(Bytes{""}, &pst0);
  CHECK_EQ(true, res0);

  auto res1 = indexer->Delete(Bytes{""});
  CHECK_EQ(true, res1);

  LogRecordPst pst1{.fid = 1, .offset = 200};
  auto res2 = indexer->Put(Bytes{"abc"}, &pst1);
  CHECK_EQ(true, res2);
  res1 = indexer->Delete(Bytes{"abc"});
  CHECK_EQ(true, res1);
  auto res3 = indexer->Get(Bytes{"abc"});
  CHECK_EQ(nullptr, res3);
}