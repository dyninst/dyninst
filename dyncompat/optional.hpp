#ifndef DYNINST_DYNCOMPAT_OPTIONAL_HPP
#define DYNINST_DYNCOMPAT_OPTIONAL_HPP

#include <optional>
#include <type_traits>
#include <utility>

namespace dyncompat {

template <typename T>
class optional : public std::optional<T> {
  using base = std::optional<T>;

public:
  using base::base;
  using base::operator=;

  optional() = default;
  optional(const optional&) = default;
  optional(optional&&) = default;
  optional(const base& other) : base(other) {}
  optional(base&& other) : base(std::move(other)) {}

  optional& operator=(const optional&) = default;
  optional& operator=(optional&&) = default;

  optional& operator=(const base& other) {
    base::operator=(other);
    return *this;
  }

  optional& operator=(base&& other) {
    base::operator=(std::move(other));
    return *this;
  }

  T& get() {
    return this->value();
  }

  const T& get() const {
    return this->value();
  }
};

using std::nullopt;
using std::nullopt_t;

template <typename T>
optional<std::decay_t<T>> make_optional(T&& value) {
  return optional<std::decay_t<T>>(std::forward<T>(value));
}

} // namespace dyncompat

#endif
