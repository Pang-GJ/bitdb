#include "bitdb/utils/arena.h"
#include <cassert>
#include <cstdint>

Arena::Arena()
    : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

Arena::~Arena() {
  for (auto& block : blocks_) {
    delete[] block;
  }
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
  char* new_block = new char[block_bytes];
  blocks_.push_back(new_block);
  memory_usage_ += block_bytes + sizeof(char*);
  return new_block;
}

char* Arena::AllocateFallback(size_t bytes) {
  if (bytes > K_BLOCK_SIZE / 4) {
    // 大于 1/4，也就是 1k，直接独立申请新一块内存
    return AllocateNewBlock(bytes);
  }

  // 否则共享分配一块内存
  alloc_ptr_ = AllocateNewBlock(K_BLOCK_SIZE);
  alloc_bytes_remaining_ = K_BLOCK_SIZE;

  char* result = alloc_ptr_;
  alloc_ptr_ += bytes;
  alloc_bytes_remaining_ -= bytes;
  return result;
}

char* Arena::Allocate(size_t bytes) {
  assert(bytes > 0);

  if (bytes <= alloc_bytes_remaining_) {
    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
  }
  // 空间不够用，申请新空间
  return AllocateFallback(bytes);
}

char* Arena::AllocateAligned(size_t bytes) {
  // step1: 判断当前系统需要对齐多少字节，指针的size就代表了系统的对齐字节
  // 如果对齐字节小于8，把align设置为8，反之设置为系统的对齐字节
  const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  // step2: 断言判断对齐字节数align是否为 2 的N次方，
  static_assert((align & (align - 1)) == 0,
                "pointer size should be a power of 2");
  // step3: 计算当前空闲内存的首地址未对齐的字节数 current_mod
  // 因为 align 一定是 2
  // 的n次方，所以其二进制形式为一个1加上n个0，那么align-1就是N个1, alloc_ptr
  // 进行与运算后只保留了低 N 位的值，就是未对齐的字节数
  size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
  // step4: 求需要补齐的字节数slop
  size_t slop = (current_mod == 0 ? 0 : align - current_mod);
  // step5: 实际申请内存数 = 用户申请的字节数 + 对齐补上的字节数
  size_t needed = bytes + slop;

  // 假设现在有一个新的对齐的内存块，首地址为0x00，系统的对齐字节数为8，我们先从内存块分配了5个字节内存，
  // 显然这个内存的首地址是对齐的，此时alloc_ptr_的值变为了0x05。
  // 如果此时我们再从剩下的空闲内存中分配5个字节的内存，新分配的5字节的首地址就是0x05了，
  // 没有对齐，所以需要补上3个字节进行对齐，这样新分配的内存的首地址就变成了0x08，就对齐了。
  char* result;
  if (needed <= alloc_bytes_remaining_) {
    result = alloc_ptr_ + slop;
    alloc_ptr_ += needed;
    alloc_bytes_remaining_ -= needed;
  } else {
    result = AllocateFallback(needed);
  }
  assert(reinterpret_cast<uintptr_t>(result) & (align - 1) == 0);
  return result;
}