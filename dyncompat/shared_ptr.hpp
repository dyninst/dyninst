#ifndef DYNINST_DYNCOMPAT_SHARED_PTR_HPP
#define DYNINST_DYNCOMPAT_SHARED_PTR_HPP

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

#ifndef DYNCOMPAT_NOEXCEPT
#define DYNCOMPAT_NOEXCEPT noexcept
#endif

namespace dyncompat {

template <typename T>
void checked_delete(T* ptr) DYNCOMPAT_NOEXCEPT;

template <typename T>
class shared_ptr : public std::shared_ptr<T> {
  using base = std::shared_ptr<T>;

public:
  using element_type = typename base::element_type;

  shared_ptr() DYNCOMPAT_NOEXCEPT = default;
  shared_ptr(std::nullptr_t) DYNCOMPAT_NOEXCEPT : base(nullptr) {}
  shared_ptr(const shared_ptr&) DYNCOMPAT_NOEXCEPT = default;
  shared_ptr(shared_ptr&&) DYNCOMPAT_NOEXCEPT = default;

  template <typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
  shared_ptr(Integer value) : base(nullptr) {
    assert(value == 0);
  }

  shared_ptr(const base& other) DYNCOMPAT_NOEXCEPT : base(other) {}
  shared_ptr(base&& other) DYNCOMPAT_NOEXCEPT : base(std::move(other)) {}

  template <typename U>
  shared_ptr(const shared_ptr<U>& other) DYNCOMPAT_NOEXCEPT : base(other) {}

  template <typename U>
  shared_ptr(shared_ptr<U>&& other) DYNCOMPAT_NOEXCEPT : base(std::move(other)) {}

  explicit shared_ptr(T* ptr) : base(ptr, &dyncompat::checked_delete<T>) {}

  template <typename U>
  explicit shared_ptr(U* ptr) : base(ptr, &dyncompat::checked_delete<U>) {}

  template <typename U, typename Deleter>
  shared_ptr(U* ptr, Deleter deleter) : base(ptr, deleter) {}

  template <typename U, typename Deleter, typename Allocator>
  shared_ptr(U* ptr, Deleter deleter, Allocator alloc) : base(ptr, deleter, alloc) {}

  template <typename U>
  explicit shared_ptr(const std::weak_ptr<U>& other) : base(other) {}

  template <typename U>
  shared_ptr(const shared_ptr<U>& other, T* ptr) DYNCOMPAT_NOEXCEPT : base(other, ptr) {}

  shared_ptr& operator=(const shared_ptr&) DYNCOMPAT_NOEXCEPT = default;
  shared_ptr& operator=(shared_ptr&&) DYNCOMPAT_NOEXCEPT = default;

  template <typename Integer, typename = std::enable_if_t<std::is_integral_v<Integer>>>
  shared_ptr& operator=(Integer value) {
    assert(value == 0);
    base::reset();
    return *this;
  }

  shared_ptr& operator=(const base& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(other);
    return *this;
  }

  shared_ptr& operator=(base&& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(std::move(other));
    return *this;
  }

  template <typename U>
  shared_ptr& operator=(const shared_ptr<U>& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(other);
    return *this;
  }

  template <typename U>
  shared_ptr& operator=(shared_ptr<U>&& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(std::move(other));
    return *this;
  }

  void reset() DYNCOMPAT_NOEXCEPT {
    base::reset();
  }

  template <typename U>
  void reset(U* ptr) {
    base::reset(ptr, &dyncompat::checked_delete<U>);
  }

  template <typename U, typename Deleter>
  void reset(U* ptr, Deleter deleter) {
    base::reset(ptr, deleter);
  }

  template <typename U, typename Deleter, typename Allocator>
  void reset(U* ptr, Deleter deleter, Allocator alloc) {
    base::reset(ptr, deleter, alloc);
  }
};

template <typename T>
class weak_ptr : public std::weak_ptr<T> {
  using base = std::weak_ptr<T>;

public:
  weak_ptr() DYNCOMPAT_NOEXCEPT = default;
  weak_ptr(const weak_ptr&) DYNCOMPAT_NOEXCEPT = default;
  weak_ptr(weak_ptr&&) DYNCOMPAT_NOEXCEPT = default;

  weak_ptr(const base& other) DYNCOMPAT_NOEXCEPT : base(other) {}
  weak_ptr(base&& other) DYNCOMPAT_NOEXCEPT : base(std::move(other)) {}

  template <typename U>
  weak_ptr(const shared_ptr<U>& other) DYNCOMPAT_NOEXCEPT : base(other) {}

  template <typename U>
  weak_ptr(const weak_ptr<U>& other) DYNCOMPAT_NOEXCEPT : base(other) {}

  weak_ptr& operator=(const weak_ptr&) DYNCOMPAT_NOEXCEPT = default;
  weak_ptr& operator=(weak_ptr&&) DYNCOMPAT_NOEXCEPT = default;

  weak_ptr& operator=(const base& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(other);
    return *this;
  }

  weak_ptr& operator=(base&& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(std::move(other));
    return *this;
  }

  template <typename U>
  weak_ptr& operator=(const shared_ptr<U>& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(other);
    return *this;
  }

  template <typename U>
  weak_ptr& operator=(const weak_ptr<U>& other) DYNCOMPAT_NOEXCEPT {
    base::operator=(other);
    return *this;
  }

  shared_ptr<T> lock() const DYNCOMPAT_NOEXCEPT {
    return shared_ptr<T>(base::lock());
  }
};

template <typename T>
class enable_shared_from_this : public std::enable_shared_from_this<T> {
public:
  shared_ptr<T> shared_from_this() {
    return shared_ptr<T>(std::enable_shared_from_this<T>::shared_from_this());
  }

  shared_ptr<const T> shared_from_this() const {
    return shared_ptr<const T>(std::enable_shared_from_this<T>::shared_from_this());
  }

  weak_ptr<T> weak_from_this() DYNCOMPAT_NOEXCEPT {
    return std::enable_shared_from_this<T>::weak_from_this();
  }

  weak_ptr<const T> weak_from_this() const DYNCOMPAT_NOEXCEPT {
    return std::enable_shared_from_this<T>::weak_from_this();
  }
};

template <typename T, typename U>
shared_ptr<T> const_pointer_cast(const shared_ptr<U>& ptr) DYNCOMPAT_NOEXCEPT {
  return shared_ptr<T>(std::const_pointer_cast<T>(static_cast<const std::shared_ptr<U>&>(ptr)));
}

template <typename T, typename U>
shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& ptr) DYNCOMPAT_NOEXCEPT {
  return shared_ptr<T>(std::dynamic_pointer_cast<T>(static_cast<const std::shared_ptr<U>&>(ptr)));
}

template <typename T, typename U>
shared_ptr<T> reinterpret_pointer_cast(const shared_ptr<U>& ptr) DYNCOMPAT_NOEXCEPT {
  return shared_ptr<T>(std::reinterpret_pointer_cast<T>(static_cast<const std::shared_ptr<U>&>(ptr)));
}

template <typename T, typename U>
shared_ptr<T> static_pointer_cast(const shared_ptr<U>& ptr) DYNCOMPAT_NOEXCEPT {
  return shared_ptr<T>(std::static_pointer_cast<T>(static_cast<const std::shared_ptr<U>&>(ptr)));
}

template <typename T>
void checked_delete(T* ptr) DYNCOMPAT_NOEXCEPT {
  delete ptr;
}

} // namespace dyncompat

#endif
