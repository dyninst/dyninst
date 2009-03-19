#ifndef DYN_DETAIL_BOOST_THROW_EXCEPTION_HPP_INCLUDED
#define DYN_DETAIL_BOOST_THROW_EXCEPTION_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//
//  boost/throw_exception.hpp
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2008 Emil Dotchevski and Reverge Studios, Inc.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  http://www.boost.org/libs/utility/throw_exception.html
//

#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/detail/workaround.hpp>
#include <exception>

#if !defined( DYN_DETAIL_BOOST_EXCEPTION_DISABLE ) && defined( __BORLANDC__ ) && DYN_DETAIL_BOOST_WORKAROUND( __BORLANDC__, DYN_DETAIL_BOOST_TESTED_AT(0x593) )
# define DYN_DETAIL_BOOST_EXCEPTION_DISABLE
#endif

#if !defined( DYN_DETAIL_BOOST_EXCEPTION_DISABLE ) && defined( DYN_DETAIL_BOOST_MSVC ) && DYN_DETAIL_BOOST_WORKAROUND( DYN_DETAIL_BOOST_MSVC, < 1310 )
# define DYN_DETAIL_BOOST_EXCEPTION_DISABLE
#endif

#if !defined( DYN_DETAIL_BOOST_NO_EXCEPTIONS ) && !defined( DYN_DETAIL_BOOST_EXCEPTION_DISABLE )
#include <dyn_detail/boost/exception/exception.hpp>
#include <dyn_detail/boost/current_function.hpp>
# define DYN_DETAIL_BOOST_THROW_EXCEPTION(x) ::boost::throw_exception(::boost::enable_error_info(x) <<\
    ::boost::throw_function(DYN_DETAIL_BOOST_CURRENT_FUNCTION) <<\
    ::boost::throw_file(__FILE__) <<\
    ::boost::throw_line((int)__LINE__))
#else
# define DYN_DETAIL_BOOST_THROW_EXCEPTION(x) ::boost::throw_exception(x)
#endif
namespace dyn_detail
{
  
namespace boost
{

#ifdef DYN_DETAIL_BOOST_NO_EXCEPTIONS

void throw_exception( std::exception const & e ); // user defined

#else

inline void throw_exception_assert_compatibility( std::exception const & ) { }

template<class E> inline void throw_exception( E const & e )
{
    //All boost exceptions are required to derive std::exception,
    //to ensure compatibility with DYN_DETAIL_BOOST_NO_EXCEPTIONS.
    throw_exception_assert_compatibility(e);

#ifndef DYN_DETAIL_BOOST_EXCEPTION_DISABLE
    throw enable_current_exception(enable_error_info(e));
#else
    throw e;
#endif
}

#endif

} // namespace boost
} // namespace dyn_detail

#endif // #ifndef DYN_DETAIL_BOOST_THROW_EXCEPTION_HPP_INCLUDED
