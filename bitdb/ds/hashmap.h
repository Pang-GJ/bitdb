#pragma once
#include <unordered_map>

namespace bitdb::ds {

template <typename Key, typename T, typename Hash = std::hash<Key>>
using HashMap = std::unordered_map<Key, T, Hash>;

}  // namespace bitdb::ds
