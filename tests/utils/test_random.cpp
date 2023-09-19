#include "bitdb/common/logger.h"
#include "bitdb/utils/random.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("next") {
  bitdb::Random rander;
  for (int i = 0; i < 100; ++i) {
    LOG_INFO("next: {}", rander.Next());
  }
}

TEST_CASE("uniform") {
  bitdb::Random rander;
  for (int i = 0; i < 100; ++i) {
    LOG_INFO("uniform: {}", rander.Uniform(10));
  }
}

TEST_CASE("uniform range") {
  bitdb::Random rander;
  for (int i = 0; i < 100; ++i) {
    LOG_INFO("uniform range: {}", rander.UniformRange(5, 20));
  }
}