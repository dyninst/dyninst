#ifndef DYNINST_DYNCOMPAT_ASSIGN_LIST_OF_HPP
#define DYNINST_DYNCOMPAT_ASSIGN_LIST_OF_HPP

#include <initializer_list>
#include <map>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dyncompat {
namespace assign {

template <typename T>
class list_builder {
public:
  explicit list_builder(const T& value) : values_{value} {}

  list_builder& operator()(const T& value) {
    values_.push_back(value);
    return *this;
  }

  operator std::vector<T>() const { return std::vector<T>(values_.begin(), values_.end()); }

  template <typename Container>
  operator Container() const {
    return Container(values_.begin(), values_.end());
  }

  template <typename Container>
  Container convert_to_container() const {
    return Container(values_.begin(), values_.end());
  }

private:
  std::vector<T> values_;
};

template <typename T>
list_builder<std::decay_t<T>> list_of(T&& value) {
  return list_builder<std::decay_t<T>>(std::forward<T>(value));
}

template <typename K, typename V>
class map_builder {
public:
  map_builder(const K& key, const V& value) { values_.emplace_back(key, value); }

  map_builder& operator()(const K& key, const V& value) {
    values_.emplace_back(key, value);
    return *this;
  }

  operator std::map<K, V>() const { return std::map<K, V>(values_.begin(), values_.end()); }

  operator std::unordered_map<K, V>() const {
    return std::unordered_map<K, V>(values_.begin(), values_.end());
  }

  template <typename Map>
  operator Map() const {
    return Map(values_.begin(), values_.end());
  }

  template <typename Map>
  Map convert_to_container() const {
    return Map(values_.begin(), values_.end());
  }

private:
  std::vector<std::pair<K, V>> values_;
};

template <typename K, typename V>
map_builder<std::decay_t<K>, std::decay_t<V>> map_list_of(K&& key, V&& value) {
  return map_builder<std::decay_t<K>, std::decay_t<V>>(std::forward<K>(key), std::forward<V>(value));
}

} // namespace assign
} // namespace dyncompat

#endif
