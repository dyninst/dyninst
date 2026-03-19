#ifndef DYNINST_DYNCOMPAT_THREAD_COMPAT_HPP
#define DYNINST_DYNCOMPAT_THREAD_COMPAT_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <utility>

namespace dyncompat {

using mutex = std::mutex;
using recursive_mutex = std::recursive_mutex;
using shared_mutex = std::shared_mutex;
using condition_variable = std::condition_variable_any;
using condition_variable_any = std::condition_variable_any;
using thread = std::thread;

template <typename Mutex>
using unique_lock = std::unique_lock<Mutex>;

template <typename Mutex>
using shared_lock = std::shared_lock<Mutex>;

template <typename Mutex>
using lock_guard = std::lock_guard<Mutex>;

template <typename Mutex>
lock_guard<Mutex> make_lock_guard(Mutex& mutex_ref) {
  return lock_guard<Mutex>(mutex_ref);
}

using std::lock;

struct once_init_t {};

class once_flag {
public:
  constexpr once_flag() noexcept = default;
  constexpr once_flag(once_init_t) noexcept {}

  once_flag(const once_flag&) = delete;
  once_flag& operator=(const once_flag&) = delete;

private:
  std::once_flag flag_;

  template <typename Callable, typename... Args>
  friend void call_once(once_flag&, Callable&&, Args&&...);
};

template <typename Callable, typename... Args>
void call_once(once_flag& flag, Callable&& callable, Args&&... args) {
  std::call_once(flag.flag_, std::forward<Callable>(callable), std::forward<Args>(args)...);
}

class barrier {
public:
  explicit barrier(unsigned count)
      : threshold_(count), count_(count), generation_(0) {}

  barrier(const barrier&) = delete;
  barrier& operator=(const barrier&) = delete;

  bool wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    const auto generation = generation_;

    if(--count_ == 0) {
      generation_++;
      count_ = threshold_;
      cv_.notify_all();
      return true;
    }

    cv_.wait(lock, [&] { return generation != generation_; });
    return false;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  const unsigned threshold_;
  unsigned count_;
  unsigned generation_;
};

template <typename Mutex>
class basic_lockable_adapter {
public:
  using mutex_type = Mutex;

  basic_lockable_adapter() = default;
  basic_lockable_adapter(const basic_lockable_adapter&) = delete;
  basic_lockable_adapter& operator=(const basic_lockable_adapter&) = delete;

  void lock() const { mutex_.lock(); }
  void unlock() const { mutex_.unlock(); }
  bool try_lock() const { return mutex_.try_lock(); }

  Mutex& lockable() const { return mutex_; }

private:
  mutable Mutex mutex_{};
};

template <typename Mutex>
using lockable_adapter = basic_lockable_adapter<Mutex>;

} // namespace dyncompat

#define DYNCOMPAT_ONCE_INIT ::dyncompat::once_init_t{}

#endif
