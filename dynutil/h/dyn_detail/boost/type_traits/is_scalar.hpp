
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_IS_SCALAR_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_SCALAR_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/is_arithmetic.hpp>
#include <dyn_detail/boost/type_traits/is_enum.hpp>
#include <dyn_detail/boost/type_traits/is_pointer.hpp>
#include <dyn_detail/boost/type_traits/is_member_pointer.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_or.hpp>
#include <dyn_detail/boost/config.hpp>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

namespace detail {

template <typename T>
struct is_scalar_impl
{ 
   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
      (::dyn_detail::boost::type_traits::ice_or<
         ::dyn_detail::boost::is_arithmetic<T>::value,
         ::dyn_detail::boost::is_enum<T>::value,
         ::dyn_detail::boost::is_pointer<T>::value,
         ::dyn_detail::boost::is_member_pointer<T>::value
      >::value));
};

// these specializations are only really needed for compilers 
// without partial specialization support:
template <> struct is_scalar_impl<void>{ DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false ); };
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
template <> struct is_scalar_impl<void const>{ DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false ); };
template <> struct is_scalar_impl<void volatile>{ DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false ); };
template <> struct is_scalar_impl<void const volatile>{ DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false ); };
#endif

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_scalar,T,::dyn_detail::boost::detail::is_scalar_impl<T>::value)

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_SCALAR_HPP_INCLUDED
