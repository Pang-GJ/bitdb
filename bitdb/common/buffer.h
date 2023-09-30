#pragma once

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <queue>
#include "bitdb/common/spin_lock.h"
namespace bitdb::common {

template <typename T>
struct BufferBase {
  virtual ~BufferBase() = default;
  virtual void Push(T&& value) = 0;
  virtual bool TryPop(T* value) = 0;
};

/**
 * @brief 多生产者单消费者 ringbuffer
 *
 * @tparam T 类型 T 必须提供默认构造函数
 */
template <typename T>
struct RingBuffer : BufferBase<T> {
 public:
  struct alignas(64) Item {
    Item() : flag_(false), written_(0) {}

   private:
    friend RingBuffer<T>;

    std::atomic_flag flag_;
    char written_;
    char padding_[256 - sizeof(flag_) - sizeof(char) - sizeof(T)]{};
    T value_;
  };

  explicit RingBuffer(const std::size_t size)
      : size_(size),
        ring_buffer_(static_cast<Item*>(std::malloc(size * sizeof(Item)))),
        write_index_(0),
        read_index_(0) {
    for (std::size_t i = 0; i < size_; ++i) {
      new (&ring_buffer_[i]) Item();
    }
    static_assert(sizeof(Item) == 256, "Unexpected size != 256");
  }

  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  ~RingBuffer() {
    for (std::size_t i = 0; i < size_; ++i) {
      ring_buffer_[i].~Item();
    }
    std::free(ring_buffer_);
  }

  void Push(T&& value) override {
    auto write_index =
        write_index_.fetch_add(1, std::memory_order_relaxed) % size_;
    Item& item = ring_buffer_[write_index];
    SpinLock lock(item.flag_);
    item.value_ = value;
    item.written_ = 1;
  }

  bool TryPop(T* value) override {
    Item& item = ring_buffer_[read_index_ & size_];
    SpinLock lock(item.flag_);
    if (item.written_ == 1) {
      *value = std::move(item.value_);
      item.written_ = 0;
      ++read_index_;
      return true;
    }
    return false;
  }

 private:
  const std::size_t size_;
  Item* ring_buffer_;
  // 多写，需要线程安全
  std::atomic<uint32_t> write_index_;
  char pad_[64]{};
  // 单读，不需要线程安全
  uint32_t read_index_;
};

/**
 * @brief
 *
 * @tparam T 类型 T 提供移动构造
 */
template <typename T>
class Buffer {
 public:
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  struct Item {
    explicit Item(T&& value) : value_(std::move(value)) {}
    char padding_[256 - sizeof(T)]{};
    T value_;
  };

  static constexpr const size_t SIZE =
      32768;  // 8MB. Helps reduce memory fragmentation

  bool Full() const {
    auto curr_size = write_state_[SIZE].load(std::memory_order_acquire);
    return curr_size + 1 == SIZE;
  }

  // Returns true if we need to switch to next buffer
  bool Push(T&& value, const uint32_t write_index) {
    new (&buffer_[write_index]) Item(std::move(value));
    write_state_[write_index].store(1, std::memory_order_release);
    return write_state_[write_index].fetch_add(1, std::memory_order_acquire) +
               1 ==
           SIZE;
  }

  bool TryPop(T* value, const uint32_t read_index) {
    if (write_state_[read_index].load(std::memory_order_acquire) != 0U) {
      Item& item = buffer_[read_index];
      *value = std::move(item.value_);
      return true;
    }
    return false;
  }

 private:
  Item* buffer_;
  // write_state_ 最后一个位置用于记录是否满了
  std::atomic<uint32_t> write_state_[SIZE + 1];
};

template <typename T>
class QueueBuffer : BufferBase<T> {
 public:
  QueueBuffer(const QueueBuffer&) = delete;
  QueueBuffer& operator=(const QueueBuffer&) = delete;

  QueueBuffer()
      : current_read_buffer_(nullptr),
        write_index_(0),
        flag_(false),
        read_index_(0) {
    SetupNextWriteBuffer();
  }

  void Push(T&& value) override {
    auto write_index = write_index_.fetch_add(1, std::memory_order_relaxed);
    if (write_index < Buffer<T>::SIZE) {
      if (current_write_buffer_.load(std::memory_order_acquire)
              ->Push(std::move(value), write_index)) {
        SetupNextWriteBuffer();
      }
    } else {
      while (write_index_.load(std::memory_order_acquire) >= Buffer<T>::SIZE) {
        // ;
      }
      Push(std::move(value));
    }
  }

  bool TryPop(T* value) override {
    if (current_read_buffer_ == nullptr) {
      current_read_buffer_ = GetNextReadBuffer();
    }

    Buffer<T>* read_buffer = current_read_buffer_;
    if (read_buffer == nullptr) {
      return false;
    }

    if (read_buffer->TryPop(value, read_index_)) {
      read_index_++;
      if (read_index_ == Buffer<T>::SIZE) {
        read_index_ = 0;
        current_read_buffer_ = nullptr;
        SpinLock lock(flag_);
        buffers_.pop();
      }
      return true;
    }
    return false;
  }

 private:
  void SetupNextWriteBuffer() {
    auto next_write_buffer = std::make_unique<Buffer<T>>();
    current_write_buffer_.store(next_write_buffer.get(),
                                std::memory_order_release);
    SpinLock lock(flag_);
    buffers_.push(std::move(next_write_buffer));
    write_index_.store(0, std::memory_order_relaxed);
  }

  Buffer<T>* GetNextReadBuffer() {
    SpinLock lock(flag_);
    return buffers_.empty() ? nullptr : buffers_.front().get();
  }

  std::queue<std::unique_ptr<Buffer<T>>> buffers_;
  std::atomic<Buffer<T>*> current_write_buffer_;
  Buffer<T>* current_read_buffer_;
  std::atomic<uint32_t> write_index_;
  std::atomic_flag flag_;
  uint32_t read_index_;
};

}  // namespace bitdb::common