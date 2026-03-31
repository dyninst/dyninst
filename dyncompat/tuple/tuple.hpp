#ifndef DYNINST_DYNCOMPAT_TUPLE_TUPLE_HPP
#define DYNINST_DYNCOMPAT_TUPLE_TUPLE_HPP

#include <tuple>

namespace dyncompat {

template <typename... Ts>
using tuple = std::tuple<Ts...>;

using std::get;
using std::make_tuple;
using std::tie;

namespace tuples {
using std::ignore;
using std::tie;
} // namespace tuples

} // namespace dyncompat

#endif
