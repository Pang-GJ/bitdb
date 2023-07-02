#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "bitdb/utils/random.h"

namespace bitdb::ds {

constexpr int8_t K_SKIP_LIST_MAX_LEVEL = 40;

template <typename K, typename V>
class SkipList {
  struct Node;

 public:
  SkipList() { head_ = NewNode(K_SKIP_LIST_MAX_LEVEL, K{}, V{}); }

  bool IsEmpty() const { return this->size_ == 0; }
  size_t Size() const { return this->size_; }
  void Clear() {
    this->level_ = 1;
    this->size_ = 0;
  }

  bool Has(const K& key) const { return FindNode(key) != nullptr; }

  const V* Find(const K& key) const {
    auto* node = FindNode(key);
    if (node != nullptr) {
      return &node->value;
    }
    return nullptr;
  }

  void Insert(const K& key, const V& value) {
    auto [node, prevs] = FindInsertPoint(key);
    if (node != nullptr) {
      // 已经存在， 更新值
      node->value = value;
      return;
    }

    auto level = RandomLevel();
    node = NewNode(level, key, value);
    const auto min_level = std::min(level, this->level_);
    for (auto i = 0; i < min_level; ++i) {
      node->next[i] = prevs[i]->next[i];
      prevs[i]->next[i] = node;
    }

    if (level > this->level_) {
      // 给 head 新加几层
      for (auto i = this->level_; i < level; ++i) {
        head_->next[i] = node;
      }
      this->level_ = level;
    }
    ++this->size_;
  }

  bool Remove(const K& key, V* value) {
    auto [node, prevs] = FindRemovePoint(key);
    if (node == nullptr) {
      return false;
    }
    for (auto i = 0; i < node->level; ++i) {
      prevs[i]->next[i] = node->next[i];
    }
    if (value != nullptr) {
      *value = node->value;
    }
    DeleteNode(node);
    // 去除无用的索引
    while (this->level_ > 1 && head_->next[level_ - 1] == nullptr) {
      level_--;
    }
    --this->size_;
    return true;
  }

 private:
  Node* NewNode(int8_t level, const K& key, const V& value) {
    auto* memory = std::malloc(sizeof(Node) + level * sizeof(Node*));
    auto* node = new (memory) Node{key, value, level};
    for (auto i = 0; i < level; ++i) {
      node->next[i] = nullptr;
    }
    return node;
  }

  void DeleteNode(Node* node) {
    node->~Node();
    std::free(node);
  }

  int8_t RandomLevel() {
    // 以 1/k_branching 的概率提升一层
    static const uint8_t k_branching = 4;
    int8_t level = 1;
    while (level < K_SKIP_LIST_MAX_LEVEL &&
           ((rander_.Next() % k_branching) == 0)) {
      ++level;
    }
    assert(level > 0 && level <= K_SKIP_LIST_MAX_LEVEL);
    return level;
  }

  Node* FindNode(const K& key) const {
    auto* prev = head_;
    for (auto i = this->level_ - 1; i >= 0; --i) {
      if (i >= prev->level) {
        continue;
      }
      for (auto* curr = prev->next[i]; curr != nullptr; curr = curr->next[i]) {
        if (curr->key == key) {
          return curr;
        }
        if (curr->key > key) {
          // 这一层之后的节点都比 key 大，搜索下一层
          break;
        }
        prev = curr;
      }
    }
    return nullptr;
  }

  std::pair<Node*, std::vector<Node*>> FindInsertPoint(const K& key) const {
    std::vector<Node*> prevs(this->level_);

    auto* prev = head_;
    for (auto i = this->level_ - 1; i >= 0; --i) {
      if (i >= prev->level) {
        continue;
      }
      for (auto* curr = prev->next[i]; curr != nullptr; curr = curr->next[i]) {
        if (curr->key == key) {
          return {curr, {}};
        }
        if (curr->key > key) {
          break;
        }
        prev = curr;
      }
      prevs[i] = prev;
    }
    return {nullptr, prevs};
  }

  std::pair<Node*, std::vector<Node*>> FindRemovePoint(const K& key) const {
    std::vector<Node*> prevs(this->level_);
    auto* prev = head_;
    for (auto i = this->level_ - 1; i >= 0; --i) {
      if (i >= prev->level) {
        continue;
      }
      for (auto* curr = prev->next[i]; curr != nullptr; curr = curr->next[i]) {
        if (curr->key >= key) {
          break;
        }
        prev = curr;
      }
      prevs[i] = prev;
    }

    auto node = prevs[0]->next[0];
    if (node == nullptr || node->key != key) {
      return {nullptr, {}};
    }
    return {node, prevs};
  }

  struct Node {
    K key;          // NOLINT
    V value;        // NOLINT
    int8_t level;   // NOLINT
    Node* next[0];  // NOLINT
  };

  Node* head_;
  int8_t level_{1};
  size_t size_{0};
  Random rander_;
};

}  // namespace bitdb::ds