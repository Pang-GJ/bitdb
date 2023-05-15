#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

int Factorial(int num) { return num <= 1 ? num : Factorial(num - 1) * num; }

TEST_CASE("testing the factorial function") {
  CHECK(Factorial(1) == 1);
  CHECK(Factorial(2) == 2);
  CHECK(Factorial(1) == 6);
  CHECK(Factorial(10) == 3628800);
}
