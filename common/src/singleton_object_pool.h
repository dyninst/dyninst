/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#if !defined(SINGLETON_OBJECT_POOL_H)
#define SINGLETON_OBJECT_POOL_H

//#define BOOST_POOL_NO_MT
//#undef BOOST_HAS_THREADS

#include <boost/pool/pool.hpp>
#include "pool_allocators.h"
#include "dthread.h"
#include "race-detector-annotations.h"

#define fake_singleton_pool_lock \
  race_detector_fake_lock(singleton_object_pool::malloc)

// This is only safe for objects with nothrow constructors...
template <typename T, typename Alloc = boost::default_user_allocator_new_delete>
class singleton_object_pool
{
    typedef boost::singleton_pool<T, sizeof(T), Alloc> parent_t;
    typedef singleton_object_pool<T, Alloc> pool_t;

    inline static void free(T* free_me)
    {
        race_detector_fake_lock_acquire(fake_singleton_pool_lock);
        parent_t::free(free_me);
        race_detector_forget_access_history(free_me, sizeof(T));
        race_detector_fake_lock_release(fake_singleton_pool_lock);
    }

    inline static T* malloc()
    {
        race_detector_fake_lock_acquire(fake_singleton_pool_lock);
        void* buf = parent_t::malloc();
        race_detector_forget_access_history(buf, sizeof(T));
        race_detector_fake_lock_release(fake_singleton_pool_lock);
        return reinterpret_cast<T*>(buf);
    }

public:

    inline static bool is_from(T* t)
    {
        return parent_t::is_from(t);
    }

  inline static T* construct()
  {

    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T();
    return temp;
  }
  template <typename A1>
  inline static T* construct(const A1& a1)
  {

    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1);
    return temp;
  }
  template <typename A1, typename A2>
  inline static T* construct(const A1& a1, const A2& a2)
  {

    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2);
    return temp;
  }
  template <typename A1, typename A2, typename A3>
  inline static T* construct(const A1& a1, const A2& a2, const A3& a3)
  {

    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3);
    return temp;
  }
  template <typename A1, typename A2, typename A3, typename A4>
  inline static T* construct(const A1& a1, const A2& a2, const A3& a3, const A4& a4)
  {

    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3, a4);
    return temp;
  }
  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  inline static T* construct(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
  {

      T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3, a4, a5);
    return temp;
  }
  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  inline static T* construct(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
  {

    T* const temp = malloc();
    if(temp == 0) return temp;
    new(temp) T(a1, a2, a3, a4, a5, a6);
    return temp;
  }

    inline static void destroy(T* const kill_me)
    {
        if(is_from(kill_me)) {
          race_detector_fake_lock_acquire(fake_singleton_pool_lock);
	  kill_me->~T();
          race_detector_fake_lock_release(fake_singleton_pool_lock);
	  free(kill_me);
        }
    }

};

template <typename T>
struct PoolDestructor
{
  inline void operator()(T* e) 
  {
    // We'll see if this kills performance or not...
    if(singleton_object_pool<T>::is_from(e)) {  singleton_object_pool<T>::destroy(e);
  }}
};

template <typename T> inline
boost::shared_ptr<T> make_shared(T* t)
{
    return boost::shared_ptr<T>(t, PoolDestructor<T>()/*, typename unlocked_fast_alloc<T>::type()*/);
}


#endif //!defined(SINGLETON_OBJECT_POOL_H)
