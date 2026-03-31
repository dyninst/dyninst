#ifndef DYNINST_DYNCOMPAT_ANY_HPP
#define DYNINST_DYNCOMPAT_ANY_HPP

#include <any>

namespace dyncompat {

using any = std::any;
using bad_any_cast = std::bad_any_cast;
using std::any_cast;

} // namespace dyncompat

#endif
