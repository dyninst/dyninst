
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// $Source$
// $Date: 2006-07-12 07:10:22 -0400 (Wed, 12 Jul 2006) $
// $Revision: 34511 $

#include <dyn_detail/boost/type_traits/detail/template_arity_spec.hpp>
#include <dyn_detail/boost/type_traits/integral_constant.hpp>
#include <dyn_detail/boost/mpl/bool.hpp>
#include <dyn_detail/boost/mpl/aux_/lambda_support.hpp>
#include <dyn_detail/boost/config.hpp>

//
// Unfortunately some libraries have started using this header without
// cleaning up afterwards: so we'd better undef the macros just in case 
// they've been defined already....
//
#ifdef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF2
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC2
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC2
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_PARTIAL_SPEC2_1
#undef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1
#endif

#if defined(__SUNPRO_CC) && (__SUNPRO_CC < 0x570)
#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
	typedef ::dyn_detail::boost::integral_constant<bool,C> type; \
    enum { value = type::value }; \
    /**/
#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C)

#elif defined(DYN_DETAIL_BOOST_MSVC) && DYN_DETAIL_BOOST_MSVC < 1300

#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    typedef ::dyn_detail::boost::integral_constant<bool,C> base_; \
    using base_::value; \
    /**/

#endif

#ifndef DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL
#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) /**/
#endif

#ifndef DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE
#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) : ::dyn_detail::boost::integral_constant<bool,C>
#endif 


#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(trait,T,C) \
template< typename T > struct trait \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(1,trait,(T)) \
}; \
\
DYN_DETAIL_BOOST_TT_AUX_TEMPLATE_ARITY_SPEC(1,trait) \
/**/


#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF2(trait,T1,T2,C) \
template< typename T1, typename T2 > struct trait \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(2,trait,(T1,T2)) \
}; \
\
DYN_DETAIL_BOOST_TT_AUX_TEMPLATE_ARITY_SPEC(2,trait) \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp,C) \
template<> struct trait< sp > \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(1,trait,(sp)) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC2(trait,sp1,sp2,C) \
template<> struct trait< sp1,sp2 > \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(2,trait,(sp1,sp2)) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(trait,sp,C) \
template<> struct trait##_impl< sp > \
{ \
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (C)); \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC2(trait,sp1,sp2,C) \
template<> struct trait##_impl< sp1,sp2 > \
{ \
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (C)); \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(param,trait,sp,C) \
template< param > struct trait< sp > \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_2(param1,param2,trait,sp,C) \
template< param1, param2 > struct trait< sp > \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(param,trait,sp1,sp2,C) \
template< param > struct trait< sp1,sp2 > \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(2,trait,(sp1,sp2)) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(param1,param2,trait,sp1,sp2,C) \
template< param1, param2 > struct trait< sp1,sp2 > \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_C_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_VALUE_DECL(C) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_PARTIAL_SPEC2_1(param,trait,sp1,sp2,C) \
template< param > struct trait##_impl< sp1,sp2 > \
{ \
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (C)); \
}; \
/**/

#ifndef DYN_DETAIL_BOOST_NO_CV_SPECIALIZATIONS
#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(trait,sp,value) \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp,value) \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp const,value) \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp volatile,value) \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp const volatile,value) \
    /**/
#else
#   define DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(trait,sp,value) \
    DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_SPEC1(trait,sp,value) \
    /**/
#endif
