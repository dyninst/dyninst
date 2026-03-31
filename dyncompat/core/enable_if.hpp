#ifndef DYNINST_DYNCOMPAT_CORE_ENABLE_IF_HPP
#define DYNINST_DYNCOMPAT_CORE_ENABLE_IF_HPP

#include <type_traits>

namespace dyncompat {

template <typename Cond, typename T = void>
struct enable_if : std::enable_if<Cond::value, T> {};

} // namespace dyncompat

#endif
