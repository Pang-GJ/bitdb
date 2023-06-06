#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/ds/skiplist.h"

using bitdb::ds::SkipList;

TEST_CASE("TestSkipList_Insert") {
  SkipList<int, int> sl;
  for (size_t i = 0; i < 100000; ++i) {
    CHECK_EQ(sl.Size(), i);
    CHECK_FALSE(sl.Has(i));
    sl.Insert(i, i);
    CHECK_EQ(sl.Size(), i + 1);
    CHECK_EQ(sl.Has(i), true);
  }
}

TEST_CASE("TestSkipList_Insert_Dup") {
  SkipList<int, int> sl;
  sl.Insert(1, 1);
  CHECK_EQ(sl.Size(), 1U);
  sl.Insert(1, 2);
  CHECK_EQ(sl.Size(), 1U);
  CHECK_EQ(*sl.Find(1), 2);
}

TEST_CASE("TestSkipList_Remove") {
  SkipList<int, int> sl;
  auto MakeSkipListN = [&](int n) {  // NOLINT
    for (auto i = 0; i < n; ++i) {
      sl.Insert(i, i);
    }
  };

  MakeSkipListN(10000);
  for (int i = 0; i < 10000; i++) {
    CHECK_EQ(true, sl.Remove(i, nullptr));
  }
  CHECK_EQ(true, sl.IsEmpty());
  CHECK_EQ(sl.Size(), 0U);
}

TEST_CASE("TestSkipList_Remove_Nonexist") {
  SkipList<int, int> sl;
  sl.Insert(1, 1);
  sl.Insert(2, 2);
  CHECK_FALSE(sl.Remove(0, nullptr));
  CHECK_FALSE(sl.Remove(3, nullptr));
  CHECK_EQ(sl.Size(), 2U);
}
