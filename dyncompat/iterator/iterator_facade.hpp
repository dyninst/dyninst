#ifndef DYNINST_DYNCOMPAT_ITERATOR_ITERATOR_FACADE_HPP
#define DYNINST_DYNCOMPAT_ITERATOR_ITERATOR_FACADE_HPP

#include <iterator>
#include <memory>
#include <optional>
#include <type_traits>

namespace dyncompat {

using bidirectional_traversal_tag = std::bidirectional_iterator_tag;

struct iterator_core_access {
  template <typename Derived>
  static decltype(auto) dereference(const Derived& derived) {
    return derived.dereference();
  }

  template <typename Derived>
  static bool equal(const Derived& lhs, const Derived& rhs) {
    return lhs.equal(rhs);
  }

  template <typename Derived>
  static void increment(Derived& derived) {
    derived.increment();
  }

  template <typename Derived>
  static void decrement(Derived& derived) {
    derived.decrement();
  }
};

template <typename Derived, typename Value, typename Category, typename Reference = const Value&, typename Difference = std::ptrdiff_t>
class iterator_facade {
public:
  using iterator_category = Category;
  using value_type = Value;
  using reference = Reference;
  using difference_type = Difference;
  using pointer = std::add_pointer_t<Value>;

  reference operator*() const {
    return iterator_core_access::dereference(derived());
  }

  pointer operator->() const {
    if constexpr (std::is_reference_v<reference>) {
      return std::addressof(iterator_core_access::dereference(derived()));
    } else {
      cache_ = iterator_core_access::dereference(derived());
      return std::addressof(*cache_);
    }
  }

  Derived& operator++() {
    iterator_core_access::increment(derived());
    return derived();
  }

  Derived operator++(int) {
    Derived tmp = derived();
    ++(*this);
    return tmp;
  }

  Derived& operator--() {
    iterator_core_access::decrement(derived());
    return derived();
  }

  Derived operator--(int) {
    Derived tmp = derived();
    --(*this);
    return tmp;
  }

  friend bool operator==(const iterator_facade& lhs, const iterator_facade& rhs) {
    return iterator_core_access::equal(lhs.derived(), rhs.derived());
  }

  friend bool operator!=(const iterator_facade& lhs, const iterator_facade& rhs) {
    return !(lhs == rhs);
  }

private:
  Derived& derived() { return static_cast<Derived&>(*this); }
  const Derived& derived() const { return static_cast<const Derived&>(*this); }

  mutable std::optional<Value> cache_{};
};

} // namespace dyncompat

#endif
