#ifndef DYNINST_DYNCOMPAT_ITERATOR_TRANSFORM_ITERATOR_HPP
#define DYNINST_DYNCOMPAT_ITERATOR_TRANSFORM_ITERATOR_HPP

#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace dyncompat {

template <typename UnaryFunction, typename Iterator>
class transform_iterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = typename std::iterator_traits<Iterator>::difference_type;
  using value_type = std::decay_t<decltype(std::declval<UnaryFunction>()(*std::declval<Iterator>()))>;
  using reference = decltype(std::declval<UnaryFunction>()(*std::declval<Iterator>()));
  using pointer = std::add_pointer_t<value_type>;

  transform_iterator() = default;
  explicit transform_iterator(Iterator iterator) : iterator_(iterator), function_() {}
  transform_iterator(Iterator iterator, UnaryFunction function)
      : iterator_(iterator), function_(std::move(function)) {}

  reference operator*() const { return function_(*iterator_); }

  pointer operator->() const {
    if constexpr (std::is_reference_v<reference>) {
      return std::addressof(function_(*iterator_));
    } else {
      cache_ = function_(*iterator_);
      return std::addressof(*cache_);
    }
  }

  transform_iterator& operator++() {
    ++iterator_;
    return *this;
  }

  transform_iterator operator++(int) {
    transform_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  transform_iterator& operator--() {
    --iterator_;
    return *this;
  }

  transform_iterator operator--(int) {
    transform_iterator tmp(*this);
    --(*this);
    return tmp;
  }

  bool operator==(const transform_iterator& other) const { return iterator_ == other.iterator_; }
  bool operator!=(const transform_iterator& other) const { return !(*this == other); }

private:
  Iterator iterator_{};
  UnaryFunction function_{};
  mutable std::optional<value_type> cache_{};
};

template <typename Iterator, typename UnaryFunction>
transform_iterator<UnaryFunction, Iterator> make_transform_iterator(Iterator iterator, UnaryFunction function) {
  return transform_iterator<UnaryFunction, Iterator>(iterator, std::move(function));
}

} // namespace dyncompat

#endif
