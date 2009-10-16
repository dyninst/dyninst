
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_IS_VOID_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_VOID_HPP_INCLUDED

#include <dyn_detail/boost/config.hpp>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

//* is a type T void - is_void<T>
#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_void,T,__is_void(T))
#else
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_void,T,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(is_void,void,true)

#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(is_void,void const,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(is_void,void volatile,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(is_void,void const volatile,true)
#endif

#endif  // non-CodeGear implementation

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_VOID_HPP_INCLUDED
