#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <string_view>
namespace bitdb {

namespace detail {

template <typename T>
void FormatHelper(std::ostream& oss, std::string_view& str, const T& value) {
  auto open_bracket = str.find('{');
  if (open_bracket == std::string::npos) {
    return;
  }
  auto close_bracket = str.find('}', open_bracket + 1);
  if (close_bracket == std::string::npos) {
    return;
  }
  oss << str.substr(0, open_bracket) << value;
  str = str.substr(close_bracket + 1);
}

}  // namespace detail

template <typename... Args>
std::string Format(std::string_view str, Args&&... args) {
  std::ostringstream oss;
  // 折叠表达式
  (detail::FormatHelper(oss, str, args), ...);
  oss << str;
  return oss.str();
}

template <typename... Args>
void print(std::string_view real_time_fmt, Args&&... args) {  // NOLINT
  std::cout << Format(real_time_fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void println(std::string_view real_time_fmt, Args&&... args) {  // NOLINT
  std::cout << Format(real_time_fmt, std::forward<Args>(args)...) << std::endl;
}

}  // namespace bitdb