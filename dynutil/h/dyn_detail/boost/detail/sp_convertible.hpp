#ifndef DYN_DETAIL_BOOST_DETAIL_SP_CONVERTIBLE_HPP_INCLUDED
#define DYN_DETAIL_BOOST_DETAIL_SP_CONVERTIBLE_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  detail/sp_convertible.hpp
//
//  Copyright 2008 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

#include <dyn_detail/boost/config.hpp>

#if !defined( DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE ) && defined( DYN_DETAIL_BOOST_NO_SFINAE )
# define DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE
#endif

#if !defined( DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE ) && defined( __GNUC__ ) && ( __GNUC__ * 100 + __GNUC_MINOR__ < 303 )
# define DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE
#endif

#if !defined( DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE ) && defined( __BORLANDC__ ) && ( __BORLANDC__ <= 0x610 )
# define DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE
#endif

#if !defined( DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE )

namespace dyn_detail
{
  
namespace boost
{

namespace detail
{

template< class Y, class T > struct sp_convertible
{
    typedef char (&yes) [1];
    typedef char (&no)  [2];

    static yes f( T* );
    static no  f( ... );

    enum _vt { value = sizeof( f( (Y*)0 ) ) == sizeof(yes) };
};

struct sp_empty
{
};

template< bool > struct sp_enable_if_convertible_impl;

template<> struct sp_enable_if_convertible_impl<true>
{
    typedef sp_empty type;
};

template<> struct sp_enable_if_convertible_impl<false>
{
};

template< class Y, class T > struct sp_enable_if_convertible: public sp_enable_if_convertible_impl< sp_convertible< Y, T >::value >
{
};

} // namespace detail

} // namespace boost

} // namespace dyn_detail

#endif // !defined( DYN_DETAIL_BOOST_SP_NO_SP_CONVERTIBLE )

#endif  // #ifndef DYN_DETAIL_BOOST_DETAIL_SP_TYPEINFO_HPP_INCLUDED
