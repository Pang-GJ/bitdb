#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>
#include "bitdb/utils/arena.h"
#include "bitdb/utils/random.h"

namespace bitdb::ds {

constexpr int K_SKIP_LIST_MAX_LEVEL = 40;

template <typename K, typename V>
class SkipList {
  struct Node;

 public:
  SkipList() : arena_(std::make_unique<Arena>()) {
    head_ = NewNode(K_SKIP_LIST_MAX_LEVEL, K{}, V{});
  }
  bool IsEmpty() const { return size_ == 0; }
  size_t Size() const { return size_; }
  void Clear() {
    level_ = 1;
    size_ = 0;
  }

  bool Has(const K& key) const {
    return const_cast<SkipList*>(this)->FindNode(key) != nullptr;
  }

  const V* Find(const K& key) const {
    auto* node = FindNode(key);
    if (node != nullptr) {
      return &node->value;
    }
    return nullptr;
  }

  void Insert(const K& key, const V& value) {
    auto&& [node, prevs] = FindInsertPoint(key);

    if (node != nullptr) {
      // 已经存在，更新值
      node->value = value;
      return;
    }

    auto level = RandomLevel();
    node = NewNode(level, key, value);

    for (auto i = 0; i < std::min(level, this->level_); ++i) {
      node->next[i] = prevs[i]->next[i];
      prevs[i]->next[i] = node;
    }

    if (level > this->level_) {
      for (auto i = this->level_; i < level; ++i) {
        head_->next[i] = node;
      }
      this->level_ = level;
    }
    ++this->size_;
  }

  bool Remove(const K& key) {
    auto&& [node, prevs] = FindRemovePoint(key);
    if (node == nullptr) {
      return false;
    }
    for (auto i = 0; i < node->level; ++i) {
      prevs[i]->next[i] = node->next[i];
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
  Node* NewNode(int level, const K& key, const V& value) {
    // TODO(pangguojian): 这里使用 Arena 会出现未知 coredump
    // auto* space = arena_->Allocate(sizeof(Node) + level * sizeof(Node*));
    auto* space = std::malloc(sizeof(Node) + level * sizeof(Node*));
    return new (space) Node{key, value, level};
  }

  void DeleteNode(Node* node) {
    node->~Node();
    std::free(node);
  }

  const Node* FindNode(const K& key) const {
    // auto* prev = static_cast<const Node*>(&this->head_);
    auto* prev = head_;
    for (auto i = level_ - 1; i >= 0; --i) {
      for (auto* curr = prev->next[i]; curr != nullptr; curr = curr->next[i]) {
        if (curr->key == key) {
          return curr;
        }
        if (curr->key > key) {
          // 这一层之后的所有的节点都比key大，搜素下一层
          break;
        }
        prev = curr;
      }
    }
    return prev->next[0];
  }

  std::vector<Node*>& FindPrevNodes(const K& key) {
    auto& prevs = prevs_node_cache_;
    prevs.resize(level_);

    // auto* prev = static_cast<const Node*>(&head_);
    auto* prev = head_;
    for (auto i = level_ - 1; i >= 0; --i) {
      for (auto* curr = prev->next[i]; curr != nullptr; curr = curr->next[i]) {
        if (curr->key >= key) {
          break;
        }
        prev = curr;
      }
      prevs[i] = prev;
    }
    return prevs;
  }

  // findInsertPoint returns (*node, nullptr) to the existed node if the key
  // exists, or (nullptr, []*node) to the previous nodes if the key doesn't
  // exist
  std::pair<Node*, std::vector<Node*>&> FindInsertPoint(const K& key) {
    auto& prevs = prevs_node_cache_;
    prevs.resize(level_);

    // auto* prev = static_cast<const Node*>(&head_);
    auto* prev = head_;
    for (auto i = level_ - 1; i >= 0; --i) {
      for (auto* curr = prev->next[i]; curr != nullptr; curr = curr->next[i]) {
        if (curr->key == key) {
          // The key is already existed, prevs are useless because no new node
          // insertion. stop searching.
          return {curr, prevs};
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

  std::pair<Node*, std::vector<Node*>&> FindRemovePoint(const K& key) {
    auto& prevs = FindPrevNodes(key);
    auto node = prevs[0]->next[0];
    if (node == nullptr || node->key != key) {
      return {nullptr, prevs};
    }
    return {node, prevs};
  }

  int RandomLevel() {
    // Increase height with probability 1 in kBranching
    static const unsigned int k_branching = 4;
    int level = 1;
    // 新加一层的概率是 1/4
    while (level < K_SKIP_LIST_MAX_LEVEL &&
           ((rander_.Next() % k_branching) == 0)) {
      level++;
    }
    assert(level > 0);
    assert(level <= K_SKIP_LIST_MAX_LEVEL);
    return level;
  }

  struct Node {
    K key{};        // NOLINT
    V value{};      // NOLINT
    int level{0};   // NOLINT
    Node* next[0];  // NOLINT
  };

  // struct HeadNode : Node {
  //   Node* next[K_SKIP_LIST_MAX_LEVEL];  // NOLINT
  // };

  int level_ = 1;
  size_t size_{0};
  Node* head_;
  std::vector<Node*> prevs_node_cache_;
  Random rander_;
  std::unique_ptr<Arena> arena_;
};

}  // namespace bitdb::ds