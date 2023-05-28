#pragma once

#include <cstddef>
#include <vector>

constexpr int K_BLOCK_SIZE = 4096;

namespace bitdb {

class Arena {
 public:
  Arena();

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  ~Arena();

  char* Allocate(size_t bytes);
  char* AllocateAligned(size_t bytes);

  size_t MemoryUsage() const { return memory_usage_; }

 private:
  char* AllocateFallback(size_t bytes);

  char* AllocateNewBlock(size_t block_bytes);

  // 当前申请的内存块指针
  char* alloc_ptr_;

  // 已经申请内存块剩余的 bytes 数
  size_t alloc_bytes_remaining_;

  std::vector<char*> blocks_;

  size_t memory_usage_;
};

}  // namespace bitdb