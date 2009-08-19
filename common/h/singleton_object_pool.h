#if !defined(SINGLETON_OBJECT_POOL_H)
#define SINGLETON_OBJECT_POOL_H

#include <boost/pool/pool.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <pool_allocators.h>

#include <dyn_detail/boost/shared_ptr.hpp>

// This is only safe for objects with nothrow constructors...
template <typename T, typename Alloc = boost::default_user_allocator_new_delete>
class singleton_object_pool
{
 private:
 struct pool_impl
 {
   boost::pool<Alloc> p;
   pool_impl() : p(sizeof(T), 32) 
   {
   }
 };
 
 typedef boost::details::pool::singleton_default<pool_impl> singleton;
 static std::set<T*> allocated_objects;
 
  
 inline static void free(T* free_me)
 {
   singleton::instance().p.free(free_me);
 }
 
 inline static T* malloc() 
  {
    return static_cast<T*>(singleton::instance().p.malloc());
  }
 public:
  inline static bool is_from(T* t)
  {
    return singleton::instance().p.is_from(t);
  }
  
  static T* construct()
  {
    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T();
    return temp;
  }
  template <typename A1>
  static T* construct(const A1& a1)
  {
    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1);
    return temp;
  }
  template <typename A1, typename A2>
  static T* construct(const A1& a1, const A2& a2)
  {
    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2);
    return temp;
  }
  template <typename A1, typename A2, typename A3>
  static T* construct(const A1& a1, const A2& a2, const A3& a3)
  {
    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3);
    return temp;
  }
  template <typename A1, typename A2, typename A3, typename A4>
  static T* construct(const A1& a1, const A2& a2, const A3& a3, const A4& a4)
  {
    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3, a4);
    return temp;
  }
  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  static T* construct(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
  {
    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3, a4, a5);
    return temp;
  }
  inline static void destroy(T* const kill_me)
  {
    kill_me->~T();
    free(kill_me);
  }
  
  
};


template <typename T> 
struct PoolDestructor
{
  inline void operator()(T* e) 
  {
    // We'll see if this kills performance or not...
    if(singleton_object_pool<T>::is_from(e)) {
      singleton_object_pool<T>::destroy(e);
    }
    
  }
};

template <typename T> inline
dyn_detail::boost::shared_ptr<T> make_shared(T* t)
{
  return dyn_detail::boost::shared_ptr<T>(t, PoolDestructor<T>(), typename unlocked_fast_alloc<T>::type());
}
 

#endif //!defined(SINGLETON_OBJECT_POOL_H)
