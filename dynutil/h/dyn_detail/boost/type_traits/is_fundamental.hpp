
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_IS_FUNDAMENTAL_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_FUNDAMENTAL_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/is_arithmetic.hpp>
#include <dyn_detail/boost/type_traits/is_void.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_or.hpp>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

namespace detail {

template <typename T> 
struct is_fundamental_impl
    : ::dyn_detail::boost::type_traits::ice_or< 
          ::dyn_detail::boost::is_arithmetic<T>::value
        , ::dyn_detail::boost::is_void<T>::value
        >
{ 
};

} // namespace detail

//* is a type T a fundamental type described in the standard (3.9.1)
#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_fundamental,T,__is_fundamental(T))
#else
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_fundamental,T,::dyn_detail::boost::detail::is_fundamental_impl<T>::value)
#endif

} // namespace boost
} // namespace dyn_detail 

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_FUNDAMENTAL_HPP_INCLUDED
