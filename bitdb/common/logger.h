#pragma once

#include "bitdb/common/logger_impl.h"
#include "bitdb/common/singleton.h"

namespace bitdb {

#define LOG_INFO(LogMsgFormat, ...)                                  \
  do {                                                               \
    bitdb::Singleton<bitdb::common::Logger>::Get()->Log(             \
        bitdb::common::LogLevel::INFO, __FILE__, __func__, __LINE__, \
        LogMsgFormat, ##__VA_ARGS__);                                \
  } while (0)

#define LOG_WARN(LogMsgFormat, ...)                                  \
  do {                                                               \
    bitdb::Singleton<bitdb::common::Logger>::Get()->Log(             \
        bitdb::common::LogLevel::WARN, __FILE__, __func__, __LINE__, \
        LogMsgFormat, ##__VA_ARGS__);                                \
  } while (0)

#define LOG_DEBUG(LogMsgFormat, ...)                                  \
  do {                                                                \
    bitdb::Singleton<bitdb::common::Logger>::Get()->Log(              \
        bitdb::common::LogLevel::DEBUG, __FILE__, __func__, __LINE__, \
        LogMsgFormat, ##__VA_ARGS__);                                 \
  } while (0)

#define LOG_ERROR(LogMsgFormat, ...)                                  \
  do {                                                                \
    bitdb::Singleton<bitdb::common::Logger>::Get()->Log(              \
        bitdb::common::LogLevel::ERROR, __FILE__, __func__, __LINE__, \
        LogMsgFormat, ##__VA_ARGS__);                                 \
  } while (0)

#define LOG_TRACE(LogMsgFormat, ...)                                  \
  do {                                                                \
    bitdb::Singleton<bitdb::common::Logger>::Get()->Log(              \
        bitdb::common::LogLevel::TRACE, __FILE__, __func__, __LINE__, \
        LogMsgFormat, ##__VA_ARGS__);                                 \
  } while (0)

#define LOG_FATAL(LogMsgFormat, ...)                                  \
  do {                                                                \
    bitdb::Singleton<bitdb::common::Logger>::Get()->Log(              \
        bitdb::common::LogLevel::FATAL, __FILE__, __func__, __LINE__, \
        LogMsgFormat, ##__VA_ARGS__);                                 \
    exit(-1);                                                         \
  } while (0)

}  // namespace bitdb