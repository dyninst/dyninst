#ifndef DYNINST_DYNCOMPAT_FUNCTION_HPP
#define DYNINST_DYNCOMPAT_FUNCTION_HPP

#include <functional>

namespace dyncompat {

template <typename Signature>
using function = std::function<Signature>;

using std::ref;

} // namespace dyncompat

#endif
