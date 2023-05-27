#include "bitdb/status.h"
#include <cassert>
#include <cstring>
#include <string_view>
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/string_utils.h"

namespace bitdb {

Status::Status(const StatusCode& code, std::string_view msg,
               std::string_view msg2) {
  code_ = code;
  state_ = msg;
  if (!msg2.empty()) {
    state_ += ": ";
    state_ += msg2;
  }
}

std::string Status::ToString() const {
  if (state_.empty()) {
    return "OK";
  }
  auto result = CodeToString(code());
  result.append(state_);
  return result;
}

std::string GetDebugStr(const char* fmt) { return fmt; }

}  // namespace bitdb