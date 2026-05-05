#ifndef DYNINST_DYNCOMPAT_VARIANT2_VARIANT_HPP
#define DYNINST_DYNCOMPAT_VARIANT2_VARIANT_HPP

#include <variant>

namespace dyncompat {
namespace variant2 {

template <typename... Ts>
using variant = std::variant<Ts...>;

using std::get;
using std::get_if;
using std::holds_alternative;

} // namespace variant2
} // namespace dyncompat

#endif
