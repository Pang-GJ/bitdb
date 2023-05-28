#pragma once

#include <cstddef>
#include <cstring>
#include <string_view>

namespace bitdb {
struct ReflectionEnumInfo {
  bool scoped;                       // NOLINT
  std::string_view name;             // NOLINT
  std::string_view value_full_name;  // NOLINT
  std::string_view value_name;       // NOLINT

  // 构造时，从母串中按指定位置 得到各子串
  // info : 母串，即 __PRETTY_FUNCTION__ 得到的函数名
  // equal1:等号1位置; semicolon:分号位置; equal2:等号2位置; colon:分号位置;
  // end:]位置
  // info 示例: "const char* foo() [with T = Color; T V = Color::red]"
  constexpr ReflectionEnumInfo(const char* info, size_t equal1,
                               size_t semicolon, size_t equal2, size_t colon,
                               size_t end)
      : scoped(colon != 0),
        name(info + equal1 + 2, semicolon - equal1 - 2),
        value_full_name(info + equal2 + 2, end - equal2 - 2),
        value_name((scoped)
                       ? std::string_view(info + colon + 1, end - colon - 1)
                       : value_full_name) {}
};

template <typename E, E V>
constexpr ReflectionEnumInfo ReflectEnum() {
  const char* info = __PRETTY_FUNCTION__;

  // 找各个符号的位置
  size_t len = strlen(info);
  size_t equal1 = 0;
  size_t semicolon = 0;
  size_t equal2 = 0;
  size_t colon = 0;
  size_t end = 0;

  for (auto i = 0; i < len && !end; ++i) {
    switch (info[i]) {
      case '=':
        (!equal1) ? equal1 = i : equal2 = i;
        break;
      case ';':
        semicolon = i;
        break;
      case ':':
        colon = i;
        break;
      case ']':
        end = i;
        break;
    }
  }
  return {info, equal1, semicolon, equal2, colon, end};
}
}  // namespace bitdb