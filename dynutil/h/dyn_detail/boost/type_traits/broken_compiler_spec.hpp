
//  Copyright 2001-2003 Aleksey Gurtovoy.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC_HPP_INCLUDED

#include <dyn_detail/boost/mpl/aux_/lambda_support.hpp>
#include <dyn_detail/boost/config.hpp>

// these are needed regardless of DYN_DETAIL_BOOST_TT_NO_BROKEN_COMPILER_SPEC 
#if defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
namespace dyn_detail {namespace boost { namespace detail {
template< typename T > struct remove_const_impl     { typedef T type; };
template< typename T > struct remove_volatile_impl  { typedef T type; };
template< typename T > struct remove_pointer_impl   { typedef T type; };
template< typename T > struct remove_reference_impl { typedef T type; };
typedef int invoke_DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC_outside_all_namespaces;
}}}
#endif

// agurt, 27/jun/03: disable the workaround if user defined 
// DYN_DETAIL_BOOST_TT_NO_BROKEN_COMPILER_SPEC
#if    !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
    || defined(DYN_DETAIL_BOOST_TT_NO_BROKEN_COMPILER_SPEC)

#   define DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(T) /**/

#else

// same as DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_IMPL_SPEC1 macro, except that it
// never gets #undef-ined
#   define DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(trait,spec,result) \
template<> struct trait##_impl<spec> \
{ \
    typedef result type; \
}; \
/**/

#   define DYN_DETAIL_BOOST_TT_AUX_REMOVE_CONST_VOLATILE_RANK1_SPEC(T)                         \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_const,T const,T)                    \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_const,T const volatile,T volatile)  \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_volatile,T volatile,T)              \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_volatile,T const volatile,T const)  \
    /**/

#   define DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T)                               \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*,T)                       \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*const,T)                  \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*volatile,T)               \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_pointer,T*const volatile,T)         \
    DYN_DETAIL_BOOST_TT_AUX_BROKEN_TYPE_TRAIT_SPEC1(remove_reference,T&,T)                     \
    /**/

#   define DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_2_SPEC(T)                               \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T)                                      \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T const)                                \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T volatile)                             \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_1_SPEC(T const volatile)                       \
    /**/

#   define DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T)                                   \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_PTR_REF_RANK_2_SPEC(T)                                      \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_CONST_VOLATILE_RANK1_SPEC(T)                                \
    /**/

#   define DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T)                                   \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T*)                                         \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T const*)                                   \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T volatile*)                                \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T const volatile*)                          \
    /**/

#   define DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(T)                                         \
    namespace dyn_detail {namespace boost { namespace detail {                                            \
    typedef invoke_DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC_outside_all_namespaces             \
      please_invoke_DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC_outside_all_namespaces;           \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_1_SPEC(T)                                          \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T)                                          \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T*)                                         \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T const*)                                   \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T volatile*)                                \
    DYN_DETAIL_BOOST_TT_AUX_REMOVE_ALL_RANK_2_SPEC(T const volatile*)                          \
    }}}                                                                              \
    /**/

#   include <dyn_detail/boost/type_traits/detail/type_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(bool)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(char)
#ifndef DYN_DETAIL_BOOST_NO_INTRINSIC_WCHAR_T
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(wchar_t)
#endif
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(signed char)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(unsigned char)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(signed short)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(unsigned short)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(signed int)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(unsigned int)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(signed long)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(unsigned long)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(float)
DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(double)
//DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(long double)

// for backward compatibility
#define DYN_DETAIL_BOOST_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(T) \
    DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC(T) \
/**/

#endif // DYN_DETAIL_BOOST_TT_BROKEN_COMPILER_SPEC_HPP_INCLUDED
