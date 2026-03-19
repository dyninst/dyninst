#ifndef DYNINST_DYNCOMPAT_MAKE_SHARED_HPP
#define DYNINST_DYNCOMPAT_MAKE_SHARED_HPP

#include "shared_ptr.hpp"
#include <memory>
#include <utility>

namespace dyncompat {

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return shared_ptr<T>(std::make_shared<T>(std::forward<Args>(args)...));
}

} // namespace dyncompat

#endif
