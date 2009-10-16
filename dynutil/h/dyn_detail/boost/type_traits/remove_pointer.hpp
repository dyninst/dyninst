
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_REMOVE_POINTER_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_REMOVE_POINTER_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/broken_compiler_spec.hpp>
#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/detail/workaround.hpp>

#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC,<=1300)
#include <dyn_detail/boost/type_traits/msvc/remove_pointer.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/type_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_DEF1(remove_pointer,T,T)
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T*,T)
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T* const,T)
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T* volatile,T)
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_pointer,T* const volatile,T)

#elif !DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC,<=1300)

DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_DEF1(remove_pointer,T,typename dyn_detail::boost::detail::remove_pointer_impl<T>::type)

#endif

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/type_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_REMOVE_POINTER_HPP_INCLUDED
