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

#include <memory>
#include "pool_allocators.h"
#include "dthread.h"
#include "compiler_annotations.h"

// This is only safe for objects with nothrow constructors...
template <typename T, typename Alloc = std::allocator<T> >
class singleton_object_pool : public Alloc
{
    using typename Alloc::size_type;
    using pointer = T*;
public:
    static pointer allocate( size_type n ) DYNINST_MALLOC_ANNOTATION;
    static void deallocate( pointer p ) {
        Alloc().deallocate(p, 1);
    }

    template<typename... Args>
    static T* construct(Args&&... args)
    {
        T* p = allocate(1);
#if __cplusplus < 202002L
        Alloc().construct(p, std::forward<Args>(args)...);
#else
        std::construct_at(p, std::forward<Args>(args)...);
#endif
        return p;
    }
    static void destroy(pointer p)
    {
#if __cplusplus < 202002L
        Alloc().destroy(p);
#else
        std::destroy_at(p);
#endif
        deallocate(p);
    }

};

template <typename T, typename Alloc>
typename singleton_object_pool<T, Alloc>::pointer singleton_object_pool<T, Alloc>::allocate( size_type n ) {
    return  Alloc().allocate(n);
}

template <typename T>
struct PoolDestructor
{
  inline void operator()(T* e) {
      singleton_object_pool<T>::destroy(e);
  }
};

template <typename T> inline
boost::shared_ptr<T> make_shared(T* t)
{
    return boost::shared_ptr<T>(t, PoolDestructor<T>()/*, typename unlocked_fast_alloc<T>::type()*/);
}


#endif //!defined(SINGLETON_OBJECT_POOL_H)
