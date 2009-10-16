
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


// Some fixes for is_array are based on a newgroup posting by Jonathan Lundquist.


#ifndef DYN_DETAIL_BOOST_TT_IS_ARRAY_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_ARRAY_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/config.hpp>

#ifdef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   include <dyn_detail/boost/type_traits/detail/yes_no_type.hpp>
#   include <dyn_detail/boost/type_traits/detail/wrap.hpp>
#endif

#include <cstddef>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_array,T,__is_array(T))
#elif !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_array,T,false)
#if !defined(DYN_DETAIL_BOOST_NO_ARRAY_TYPE_SPECIALIZATIONS)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T[N],true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T const[N],true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T volatile[N],true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,is_array,T const volatile[N],true)
#if !DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x600) && !defined(__IBMCPP__) &&  !DYN_DETAIL_BOOST_WORKAROUND(__DMC__, DYN_DETAIL_BOOST_TESTED_AT(0x840))
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T[],true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T const[],true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T volatile[],true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_array,T const volatile[],true)
#endif
#endif

#else // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

namespace detail {

using dyn_detail::boost::type_traits::yes_type;
using dyn_detail::boost::type_traits::no_type;
using dyn_detail::boost::type_traits::wrap;

template< typename T > T(* is_array_tester1(wrap<T>) )(wrap<T>);
char DYN_DETAIL_BOOST_TT_DECL is_array_tester1(...);

template< typename T> no_type is_array_tester2(T(*)(wrap<T>));
yes_type DYN_DETAIL_BOOST_TT_DECL is_array_tester2(...);

template< typename T >
struct is_array_impl
{ 
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = 
        sizeof(dyn_detail::boost::detail::is_array_tester2(
            dyn_detail::boost::detail::is_array_tester1(
                dyn_detail::boost::type_traits::wrap<T>()
                )
        )) == 1
    );
};

#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_array,void const volatile,false)
#endif

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_array,T,dyn_detail::boost::detail::is_array_impl<T>::value)

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_ARRAY_HPP_INCLUDED
