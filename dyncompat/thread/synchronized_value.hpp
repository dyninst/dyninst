#ifndef DYNINST_DYNCOMPAT_THREAD_SYNCHRONIZED_VALUE_HPP
#define DYNINST_DYNCOMPAT_THREAD_SYNCHRONIZED_VALUE_HPP

#include "../dyninst_thread_compat.hpp"

namespace dyncompat {

template <typename T, typename Lockable = mutex>
class synchronized_value {
public:
  synchronized_value() = default;
  explicit synchronized_value(const T& value) : value_(value) {}

private:
  T value_{};
};

} // namespace dyncompat

#endif
