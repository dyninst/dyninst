#ifndef DYN_DETAIL_BOOST_DETAIL_SPINLOCK_NT_HPP_INCLUDED
#define DYN_DETAIL_BOOST_DETAIL_SPINLOCK_NT_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  Copyright (c) 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <dyn_detail/boost/assert.hpp>

namespace dyn_detail
{
  
namespace boost
{

namespace detail
{

class spinlock
{
public:

    bool locked_;

public:

    inline bool try_lock()
    {
        if( locked_ )
        {
            return false;
        }
        else
        {
            locked_ = true;
            return true;
        }
    }

    inline void lock()
    {
        DYN_DETAIL_BOOST_ASSERT( !locked_ );
        locked_ = true;
    }

    inline void unlock()
    {
        DYN_DETAIL_BOOST_ASSERT( locked_ );
        locked_ = false;
    }

public:

    class scoped_lock
    {
    private:

        spinlock & sp_;

        scoped_lock( scoped_lock const & );
        scoped_lock & operator=( scoped_lock const & );

    public:

        explicit scoped_lock( spinlock & sp ): sp_( sp )
        {
            sp.lock();
        }

        ~scoped_lock()
        {
            sp_.unlock();
        }
    };
};

} // namespace detail
} // namespace boost
} // namespace dyn_detail

#define DYN_DETAIL_BOOST_DETAIL_SPINLOCK_INIT { false }

#endif // #ifndef DYN_DETAIL_BOOST_DETAIL_SPINLOCK_NT_HPP_INCLUDED
