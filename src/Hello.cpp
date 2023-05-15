#include "Hello.h"
#include "fmt/core.h"
#include "spdlog/spdlog.h"

void Hello::DoSomething() {
  fmt::print("Hello, {}!\n", "fmt");
  spdlog::info("Hello, {}!\n", "spdlog");
}
