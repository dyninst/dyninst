#ifndef DYNINST_DYNCOMPAT_INTEGER_TRAITS_HPP
#define DYNINST_DYNCOMPAT_INTEGER_TRAITS_HPP

#include <limits>

namespace dyncompat {

template <typename T>
struct integer_traits {
  static constexpr T const_min = std::numeric_limits<T>::lowest();
  static constexpr T const_max = std::numeric_limits<T>::max();
};

} // namespace dyncompat

#endif
