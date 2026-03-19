#ifndef DYNINST_DYNCOMPAT_FUNCTIONAL_HASH_HPP
#define DYNINST_DYNCOMPAT_FUNCTIONAL_HASH_HPP

#include <cstddef>
#include <functional>
#include <utility>

namespace dyncompat {

template <typename T>
struct hash {
  std::size_t operator()(const T& value) const {
    return std::hash<T>{}(value);
  }
};

template <typename T>
inline void hash_combine(std::size_t& seed, const T& value) {
  seed ^= std::hash<T>{}(value) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
}

template <typename T, typename U>
struct hash<std::pair<T, U>> {
  std::size_t operator()(const std::pair<T, U>& value) const {
    std::size_t seed = 0;
    hash_combine(seed, value.first);
    hash_combine(seed, value.second);
    return seed;
  }
};

} // namespace dyncompat

#endif
