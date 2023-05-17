#pragma once

#include <spdlog/spdlog.h>

#define LOG_INFO(LogMsgFormat, ...)            \
  do {                                         \
    spdlog::info(LogMsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_WARN(LogMsgFormat, ...)            \
  do {                                         \
    spdlog::warn(LogMsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_DEBUG(LogMsgFormat, ...)            \
  do {                                          \
    spdlog::debug(LogMsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_ERROR(LogMsgFormat, ...)            \
  do {                                          \
    spdlog::error(LogMsgFormat, ##__VA_ARGS__); \
  } while (0)

#define LOG_TRACE(LogMsgFormat, ...)            \
  do {                                          \
    spdlog::trace(LogMsgFormat, ##__VA_ARGS__); \
  } while (0)