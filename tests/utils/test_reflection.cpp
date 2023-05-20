#include <iostream>
#include "bitdb/utils/reflection.h"

template <typename T, T V>
char const* foo() {
  return __PRETTY_FUNCTION__;
}

// 为方便输出 ReflectionEnumInfo ，重载一下 输出流操作
std::ostream& operator<<(std::ostream& os, ReflectionEnumInfo const& ri) {
  os << "scoped = " << std::boolalpha << ri.scoped << std::noboolalpha
     << "\nname = " << ri.name << "\nvalueName = " << ri.value_name
     << "\nvalueFullName = " << ri.value_full_name
     << "\n------------------------------\n";
  return os;
}

enum class Color {
  red,
  green,
};

int main(int argc, char* argv[]) {
  // std::cout << foo<bool, false>() << std::endl;
  // std::cout << foo<Color, Color::red>() << std::endl;
  auto ri1 = ReflectEnum<Color, Color::red>();
  std::cout << ri1 << std::endl;
  return 0;
}