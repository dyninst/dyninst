
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_IS_ARITHMETIC_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_ARITHMETIC_HPP_INCLUDED

#if !defined( __CODEGEARC__ )
#include <dyn_detail/boost/type_traits/is_integral.hpp>
#include <dyn_detail/boost/type_traits/is_float.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_or.hpp>
#include <dyn_detail/boost/config.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#if !defined(__CODEGEARC__)
namespace detail {

template< typename T >
struct is_arithmetic_impl
{ 
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = 
        (::dyn_detail::boost::type_traits::ice_or< 
            ::dyn_detail::boost::is_integral<T>::value,
            ::dyn_detail::boost::is_float<T>::value
        >::value)); 
};

} // namespace detail
#endif

//* is a type T an arithmetic type described in the standard (3.9.1p8)
#if defined(__CODEGEARC__)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_arithmetic,T,__is_arithmetic(T))
#else
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_arithmetic,T,::dyn_detail::boost::detail::is_arithmetic_impl<T>::value)
#endif

} // namespace boost
} // namespace dyn_detail 

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_ARITHMETIC_HPP_INCLUDED
