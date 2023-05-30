#pragma once
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <string_view>
#include "bitdb/utils/logger.h"
/**
 * @brief 系统调用封装
 *
 */

namespace bitdb {
/**
 * @brief 判断目录是否存在，不存在则新建
 *
 * @param dir_path
 * @return true
 * @return false
 */
bool CheckOrCreateDirectory(std::string_view dir_path) {
  struct stat st;
  if (stat(dir_path.data(), &st) == 0) {
    return (st.st_mode & S_IFDIR) != 0;
  }
  if (errno == ENOENT) {
    if (mkdir(dir_path.data(), S_IRWXU | S_IRWXG | S_IRWXO) == 0) {
      return true;
    }
    LOG_ERROR("Failed to create directory: {}, because of {}.", dir_path,
              strerror(errno));
    return false;
  }
  LOG_ERROR("Failed to check directory: {}, because of {}.", dir_path,
            strerror(errno));
  return false;
}

std::string GetTempDir() {
  auto* tempdir = std::getenv("TMPDIR");  // 获取临时目录环境变量
  if (tempdir == nullptr) {               // 如果环境变量不存在
    tempdir = std::getenv("TEMP");
    if (tempdir == nullptr) {  // 如果环境变量仍不存在
      tempdir = std::getenv("TMP");
      if (tempdir == nullptr) {  // 如果环境变量仍不存在
        return {"/tmp/"};        // 使用默认的临时目录
      }
    }
  }
  return {tempdir};
}

}  // namespace bitdb