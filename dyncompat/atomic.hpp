#ifndef DYNINST_DYNCOMPAT_ATOMIC_HPP
#define DYNINST_DYNCOMPAT_ATOMIC_HPP

#include <atomic>

namespace dyncompat {

template <typename T>
using atomic = std::atomic<T>;

using memory_order = std::memory_order;

inline constexpr auto memory_order_relaxed = std::memory_order_relaxed;
inline constexpr auto memory_order_consume = std::memory_order_consume;
inline constexpr auto memory_order_acquire = std::memory_order_acquire;
inline constexpr auto memory_order_release = std::memory_order_release;
inline constexpr auto memory_order_acq_rel = std::memory_order_acq_rel;
inline constexpr auto memory_order_seq_cst = std::memory_order_seq_cst;

} // namespace dyncompat

#endif
