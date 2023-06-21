#include <iostream>
#include "bitdb/utils/format.h"
#include "bitdb/utils/string_utils.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("string output") {
  std::cout << bitdb::Format("hello {}", "world") << std::endl;

  bitdb::print("{} + {} = {}\n", 1, 1, 2);
  bitdb::println("{} + {} = {}", 1, 1, 2);
}