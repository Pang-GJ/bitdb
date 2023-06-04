#pragma once

#include <cstdint>
#include <string>
#include "bitdb/utils/bytes.h"
namespace bitdb {

/**
 * @brief 对 value 进行 varint 数据编码
 *
 * @param dst 编码到的为止
 * @param value 进行编码的 value
 * @return int 编码的长度
 */
int EncodeVarint32(char* dst, uint32_t value);

/**
 * @brief 对 varint 进行解码
 *
 * @param src 数据编码后的 src
 * @param value 解码得到的结果
 * @return int 编码的长度
 */
int DecodeVarint32(const char* src, uint32_t* value);

// Lower-level versions of Put... that write directly into a character buffer
// REQUIRES: dst has enough space for the value being written
inline void EncodeFixed32(char* dst, uint32_t value) {
  auto* const buffer = reinterpret_cast<uint8_t*>(dst);
  /* 依次将 value 的 4 字节写入至 buffer 中，也就是写入到 dst 中。
   * 并且可以看到，buffer[0] 写入的是 value 低 8 位，buffer[1] 写入的是第二个低
   * 8 位。
   * 因此，数据存放的方式是按照先低位后高位的顺序存放的，也就是说，采用的是小端存储（Little-Endian）*/
  // Recent clang and gcc optimize this to a single mov / str instruction.
  buffer[0] = static_cast<uint8_t>(value);
  buffer[1] = static_cast<uint8_t>(value >> 8);
  buffer[2] = static_cast<uint8_t>(value >> 16);
  buffer[3] = static_cast<uint8_t>(value >> 24);
}

inline void EncodeFixed64(char* dst, uint64_t value) {
  uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);

  // Recent clang and gcc optimize this to a single mov / str instruction.
  buffer[0] = static_cast<uint8_t>(value);
  buffer[1] = static_cast<uint8_t>(value >> 8);
  buffer[2] = static_cast<uint8_t>(value >> 16);
  buffer[3] = static_cast<uint8_t>(value >> 24);
  buffer[4] = static_cast<uint8_t>(value >> 32);
  buffer[5] = static_cast<uint8_t>(value >> 40);
  buffer[6] = static_cast<uint8_t>(value >> 48);
  buffer[7] = static_cast<uint8_t>(value >> 56);
}

inline uint32_t DecodeFixed32(const char* ptr) {
  auto* const buffer = reinterpret_cast<const uint8_t*>(ptr);
  return (static_cast<uint32_t>(buffer[0])) |
         (static_cast<uint32_t>(buffer[1]) << 8) |
         (static_cast<uint32_t>(buffer[2]) << 16) |
         (static_cast<uint32_t>(buffer[3]) << 24);
}

inline uint64_t DecodeFixed64(const char* ptr) {
  auto* const buffer = reinterpret_cast<const uint8_t*>(ptr);
  return (static_cast<uint64_t>(buffer[0])) |
         (static_cast<uint64_t>(buffer[1]) << 8) |
         (static_cast<uint64_t>(buffer[2]) << 16) |
         (static_cast<uint64_t>(buffer[3]) << 24) |
         (static_cast<uint64_t>(buffer[4]) << 32) |
         (static_cast<uint64_t>(buffer[5]) << 40) |
         (static_cast<uint64_t>(buffer[6]) << 48) |
         (static_cast<uint64_t>(buffer[7]) << 56);
}

}  // namespace bitdb