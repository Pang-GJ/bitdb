#include "bitdb/utils/coding.h"
#include <cstdint>

namespace bitdb {

int EncodeVarint32(char* dst, uint32_t value) {
  auto* target = reinterpret_cast<uint8_t*>(dst);
  auto target_begin = target;
  while (value >= 0x80) {  // 因为7个bit最大只能标识0x7F,
                           // 如果value>=0x80表示后面必然还有一个块
    *target = static_cast<uint8_t>(
        value | 0x80);  // 块的最高位bit设为1，用来标识后面还有块,
                        // 同时把7个bit复制到target这个块中
    value >>= 7;  // 处理下一个块(7个bit)
    ++target;
  }
  *target = static_cast<uint8_t>(value);  // 最后一个块的值
  return target + 1 - target_begin;
}

int DecodeVarint32(const char* src, uint32_t* value) {
  auto* buffer = reinterpret_cast<const uint8_t*>(src);
  auto* buffer_begin = buffer;
  uint32_t result = 0;
  int bitsize = 0;
  while (((*buffer) & 0x80) !=
         0) {  // 判断后面是否还有块，利用了编码时计算的最高位bit是否为1
    result |= ((*buffer) ^ 0x80)
              << bitsize;  // 把最高位变成0，然后往左移bitsize位放置,
                           // bitsize是7的倍数，因为编码的时候7个bit为一组
    bitsize += 7;
    ++buffer;
  }
  result |= (*buffer) << bitsize;  // 还原最高位，并且算出了答案
  if (result >= 0) {
    *value = result;
    return buffer + 1 - buffer_begin;
  }
  *value = 0;
  return 0;
}

int EncodeVarint64(char* dst, uint64_t value) {
  auto* target = reinterpret_cast<uint8_t*>(dst);
  auto* target_begin = target;
  while (value >= 0x80) {
    *target = static_cast<uint8_t>(value | 0x80);
    value >>= 7;
    ++target;
  }
  *target = static_cast<uint8_t>(value);
  return target + 1 - target_begin;
}

int DecodeVarint64(const char* src, uint64_t* value) {
  auto* buffer = reinterpret_cast<const uint8_t*>(src);
  auto* buffer_begin = buffer;
  uint64_t result = 0;
  int bitsize = 0;
  while (((*buffer) & 0x80) != 0) {
    result |= ((*buffer) ^ 0x80) << bitsize;
    bitsize += 7;
    ++buffer;
  }
  result |= (*buffer) << bitsize;
  if (result >= 0) {
    *value = result;
    return buffer + 1 - buffer_begin;
  }
  *value = 0;
  return 0;
}

}  // namespace bitdb