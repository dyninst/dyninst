#ifndef DYNINST_DYNCOMPAT_INTERPROCESS_SYNC_SCOPED_LOCK_HPP
#define DYNINST_DYNCOMPAT_INTERPROCESS_SYNC_SCOPED_LOCK_HPP

#include <mutex>

namespace dyncompat {
namespace interprocess {

template <typename Mutex>
using scoped_lock = std::unique_lock<Mutex>;

} // namespace interprocess
} // namespace dyncompat

#endif
