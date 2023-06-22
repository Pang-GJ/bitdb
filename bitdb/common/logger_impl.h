#pragma once

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <memory>
#include <thread>
#include <tuple>
#include <type_traits>
#include "bitdb/common/buffer.h"
#include "bitdb/utils/format.h"

namespace bitdb::common {

enum class LogLevel : uint8_t {
  TRACE,
  INFO,
  DEBUG,
  WARN,
  ERROR,
  CTRI,  // Critical
};

inline const char* LogLevelToString(LogLevel level) {
  switch (level) {
    case LogLevel::TRACE:
      return "TRACE";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::DEBUG:
      return "DEBUG";
    case LogLevel::WARN:
      return "WARN";
    case LogLevel::ERROR:
      return "ERROR";
    case LogLevel::CTRI:
      return "CTRI";
    default:
      return "XXXX";
  }
}

/**
 * @brief Non guaranteed logging
 * 可以使用环形缓冲区记录日志，但这种记录方式无法保证全部日志被完整记录。
 * 当环形缓冲区被填满时，最早的日志将被覆盖。此外，即使环形缓冲区已满，生产者也不会被阻塞。
 * 环形缓冲区的大小由参数ring_buffer_size_mb确定，以MB为单位。
 * 由于每个日志行的大小为256字节，因此实际缓冲区的大小将根据该参数进行计算。
 * ring_buffer_size = ring_buffer_size_mb * 1024 * 1024 / 256
 *
 */
struct NonGuaranteedLogger {
  explicit NonGuaranteedLogger(uint32_t size) : ring_buffer_size_mb(size) {}
  uint32_t ring_buffer_size_mb;  // NOLINT
};

/**
 * @brief 需要保证日志行不会丢失
 *
 */
struct GuaranteedLogger {};

namespace detail {

/**
 * @brief 返回微妙时间戳
 *
 * @return uint64_t
 */
inline uint64_t TimestampNow() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

/**
 * @brief 格式化为这种格式[2016-10-13 00:01:23.528514]
 *
 * @param os
 * @param timestamp
 */
inline std::string FormatTimestamp(uint64_t timestamp) {
  auto now = std::chrono::system_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch())
                  .count();

  std::time_t tm = std::chrono::system_clock::to_time_t(now);
  return Format("[{}.{}]",
                std::put_time(std::localtime(&tm), "%Y-%m-%d %H.%M.%S"), diff);
}

inline std::thread::id ThisThreadId() {
  const static thread_local std::thread::id id = std::this_thread::get_id();

  return id;
}

}  // namespace detail

struct LogLine {
  LogLevel level;             // NOLINT
  uint64_t timestamp;         // NOLINT
  std::thread::id thread_id;  // NOLINT
  std::string file_name;      // NOLINT
  std::string func_name;      // NOLINT
  int line_num;               // NOLINT
  std::string content;        // NOLINT
};

class FileWriter {
 public:
  FileWriter(std::string_view log_file_name, uint32_t log_file_roll_size_mb,
             bool is_stdout)
      : log_file_roll_size_bytes_(log_file_roll_size_mb * 1024 * 1024),
        name_(log_file_name),
        is_output_stdout_(is_stdout) {
    RollFile();
  }

  ~FileWriter() {
    if (file_id_ != -1) {
      ::fsync(file_id_);
      ::close(file_id_);
    }
    file_id_ = -1;
  }

  void Write(LogLine& log_line) {
    std::string log_data =
        Format("[{}] {} {} {}-{}({}): {}\n", LogLevelToString(log_line.level),
               detail::FormatTimestamp(log_line.timestamp), log_line.thread_id,
               log_line.file_name, log_line.line_num, log_line.func_name,
               log_line.content);
    auto written = ::write(file_id_, log_data.c_str(), log_data.length());
    if (is_output_stdout_) {
      ::write(STDOUT_FILENO, log_data.c_str(), log_data.length());
    }
    if (written < 0) {
      return;
    }
    bytes_written_ += written;
    if (bytes_written_ >= log_file_roll_size_bytes_) {
      RollFile();
    }
  }

 private:
  void RollFile() {
    if (file_id_ != -1) {
      ::fsync(file_id_);
      ::close(file_id_);
    }

    bytes_written_ = 0;
    std::string log_file_name = name_;
    log_file_name.append(Format(".{}.txt", ++file_writer_num_));
    file_id_ =
        ::open(log_file_name.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
  }

  uint32_t file_writer_num_ = 0;
  off_t bytes_written_ = 0;
  const uint32_t log_file_roll_size_bytes_;
  const std::string name_;
  int file_id_{-1};
  bool is_output_stdout_{false};  // 是否输出到标准输出
};

class Logger {
 public:
  Logger() : buffer_(512), file_writer_("./log", 32, true) {
    persist_thread_ = std::thread(&Logger::PersistThreadFunc, this);
  }

  ~Logger() {
    stop_ = true;
    persist_thread_.join();
  }

  template <typename... Args>
  void Log(LogLevel level, std::string_view file, std::string_view func,
           int line, std::string_view fmt, Args&&... args) {
    LogLine log_line{.level = level,
                     .timestamp = detail::TimestampNow(),
                     .thread_id = detail::ThisThreadId()};
    log_line.file_name = file;
    log_line.func_name = func;
    log_line.line_num = line;
    log_line.content = Format(fmt, std::forward<Args>(args)...);

    buffer_.Push(std::move(log_line));
  }

 private:
  void PersistThreadFunc() {
    while (!stop_) {
      LogLine log_line;
      while (buffer_.TryPop(&log_line)) {
        file_writer_.Write(log_line);
      }
    }
  }

  RingBuffer<LogLine> buffer_;
  FileWriter file_writer_;
  bool stop_{false};
  std::thread persist_thread_;
};

}  // namespace bitdb::common