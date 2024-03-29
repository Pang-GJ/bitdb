#pragma once
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <string_view>
#include "bitdb/common/logger.h"
#include "bitdb/common/string_utils.h"
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
inline bool CheckOrCreateDirectory(std::string_view dir_path) {
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

inline std::string GetTempDir() {
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

inline void RemoveFile(const std::string& file_name) {
  if (unlink(file_name.c_str()) != 0) {
    LOG_ERROR("Failed to remove file: {}", file_name);
    exit(-1);
  }
}

inline void RemoveDir(const std::string& dir_name) {
  DIR* dp = opendir(dir_name.c_str());
  if (dp == nullptr) {
    LOG_ERROR("Failed to open directory: {}", dir_name);
    exit(-1);
  }
  struct dirent* dirp;
  while ((dirp = readdir(dp)) != nullptr) {
    std::string file_name = dirp->d_name;
    if (file_name == "." || file_name == "..") {
      continue;
    }
    std::string full_file_name = Format("{}/{}", dir_name, file_name);
    struct stat stat_buf;
    if (stat(full_file_name.c_str(), &stat_buf) != 0) {
      continue;
    }
    if (S_ISDIR(stat_buf.st_mode)) {
      RemoveDir(full_file_name);
    } else {
      RemoveFile(full_file_name);
    }
  }
  closedir(dp);
  if (rmdir(dir_name.c_str()) != 0) {
    LOG_ERROR("Failed to remove directory: {}", dir_name);
    exit(-1);
  }
}

inline std::string PathBase(std::string_view full_path) {
  auto last_slash = full_path.find_last_of('/');
  if (last_slash != std::string::npos) {
    return full_path.substr(last_slash + 1).data();
  }
  // 如果没有 '/'，则 full_path 就是文件名
  return full_path.data();
}

inline bool IsFileExist(std::string_view file_path) {
  return access(file_path.data(), F_OK) == 0;
}

}  // namespace bitdb