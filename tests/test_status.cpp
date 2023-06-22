#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "bitdb/status.h"
#include "bitdb/common/logger.h"

bitdb::Status ReturnOK() {
  return bitdb::Status::Ok("Return OK");
}

TEST_CASE("ok status") {
  auto s = ReturnOK();
  LOG_INFO("{}", s.ToString()); 
}