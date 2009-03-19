#ifndef DYN_DETAIL_BOOST_DETAIL_ATOMIC_COUNT_WIN32_HPP_INCLUDED
#define DYN_DETAIL_BOOST_DETAIL_ATOMIC_COUNT_WIN32_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  boost/detail/atomic_count_win32.hpp
//
//  Copyright (c) 2001-2005 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <dyn_detail/boost/detail/interlocked.hpp>

namespace dyn_detail
{
  
namespace boost
{

namespace detail
{

class atomic_count
{
public:

    explicit atomic_count( long v ): value_( v )
    {
    }

    long operator++()
    {
        return DYN_DETAIL_BOOST_INTERLOCKED_INCREMENT( &value_ );
    }

    long operator--()
    {
        return DYN_DETAIL_BOOST_INTERLOCKED_DECREMENT( &value_ );
    }

    operator long() const
    {
        return static_cast<long const volatile &>( value_ );
    }

private:

    atomic_count( atomic_count const & );
    atomic_count & operator=( atomic_count const & );

    long value_;
};

} // namespace detail

} // namespace boost

} // namespace dyn_detail
#endif // #ifndef DYN_DETAIL_BOOST_DETAIL_ATOMIC_COUNT_WIN32_HPP_INCLUDED
