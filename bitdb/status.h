#pragma once

#include <cstdio>
#include <map>
#include <string>
#include "bitdb/utils/bytes.h"
#include "bitdb/utils/string_utils.h"

namespace bitdb {

// 定义一个辅助宏来声明一个新的状态分类，并生成相应的判断函数
#define REGISTER_STATUS_FUNC(name)                                 \
  static Status name(std::string_view msg,                         \
                     std::string_view msg2 = std::string_view()) { \
    return Status(StatusCode::k##name, msg, msg2);                 \
  }                                                                \
  inline bool Is##name() { return code() == StatusCode::k##name; }

#define REGISTER_STATUS_TOSTRING(name, msg) \
  { StatusCode::k##name, msg }

class Status {
 public:
  Status() : code_(StatusCode::kOk) {}

  std::string ToString() const;

  // Ok will be special, it doesn't need arg sometime
  static Status Ok() { return Status(); }
  REGISTER_STATUS_FUNC(Ok)
  REGISTER_STATUS_FUNC(NotFound)
  REGISTER_STATUS_FUNC(Corruption)
  REGISTER_STATUS_FUNC(InvalidArgument)
  REGISTER_STATUS_FUNC(NotSupported)
  REGISTER_STATUS_FUNC(IOError)
  REGISTER_STATUS_FUNC(InternalError)
  REGISTER_STATUS_FUNC(CheckError)

 private:
  // NOTE(pangguojian): 当你想要添加一个状态的时候，需要做三件事：
  // 1. 在 enum StatusCode 中添加新的状态，要求状态必须以 'k' 开头
  // 2. 在 public 部分中添加一条语句: REGISTER_STATUS(name)
  // 3. 在 CodeToString 中的 k_codes_to_stringmap 中添加
  // REGISTER_STATUS_TOSTRING(name, desc)
  enum StatusCode {
    kOk = 0,
    kNotFound,
    kCorruption,
    kInvalidArgument,
    kNotSupported,
    kIOError,
    kInternalError,
    kCheckError,
  };

  Status(const StatusCode& code, std::string_view msg, std::string_view msg2);

  static std::string CodeToString(StatusCode code) {
    static const std::map<StatusCode, std::string> k_codes_to_stringmap{
        REGISTER_STATUS_TOSTRING(Ok, "Ok"),
        REGISTER_STATUS_TOSTRING(NotFound, "Not Found"),
        REGISTER_STATUS_TOSTRING(Corruption, "Corruption"),
        REGISTER_STATUS_TOSTRING(InvalidArgument, "Invalid argument"),
        REGISTER_STATUS_TOSTRING(NotSupported, "Not implemented"),
        REGISTER_STATUS_TOSTRING(IOError, "IO error"),
        REGISTER_STATUS_TOSTRING(InternalError, "Internal Error in system"),
        REGISTER_STATUS_TOSTRING(CheckError, "Check failed"),
    };
    auto iter = k_codes_to_stringmap.find(code);
    std::string result;
    if (iter == k_codes_to_stringmap.end()) {
      result = Format("Unknown code: {}", code);
    }
    result = iter->second + ": ";
    return result;
  }

  StatusCode code() const { return code_; }

  StatusCode code_;
  std::string state_;
};

// Status 的 CHECK 宏
#define CHECK_OK(expression)  \
  do {                        \
    auto status = expression; \
    if (!status.IsOk()) {     \
      return status;          \
    }                         \
  } while (false)

#define CHECK_TRUE(expression, ...)                                       \
  do {                                                                    \
    if (!(expression)) {                                                  \
      return Status::CheckError(Format("{} at {}: {}: {}()", #expression, \
                                       __FILE__, __LINE__, __func__),     \
                                GetDebugStr(__VA_ARGS__));                \
    }                                                                     \
  } while (false)

#define CHECK_NOT_NULL_STATUS(pointer)                                      \
  do {                                                                      \
    if ((pointer) == nullptr) {                                             \
      LOG_ERROR("{} is nullptr, at {}: {}: {}()", #pointer, __FILE__,       \
                __LINE__, __func__);                                        \
      return Status::InternalError(Format("{} is nullptr, at {}: {}: {}()", \
                                          #pointer, __FILE__, __LINE__,     \
                                          __func__));                       \
    }                                                                       \
  } while (false)

#define CHECK_NOT_NULL(pointer)                                       \
  do {                                                                \
    if ((pointer) == nullptr) {                                       \
      LOG_ERROR("{} is nullptr, at {}: {}: {}()", #pointer, __FILE__, \
                __LINE__, __func__);                                  \
    }                                                                 \
  } while (false)

// std::string GetDebugStr(const char* fmt = "");
std::string GetDebugStr(std::string_view str = std::string_view());

}  // namespace bitdb