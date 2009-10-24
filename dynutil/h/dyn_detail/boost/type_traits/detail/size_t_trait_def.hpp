
// NO INCLUDE GUARDS, THE HEADER IS INTENDED FOR MULTIPLE INCLUSION

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)

// $Source$
// $Date: 2005-08-25 12:27:28 -0400 (Thu, 25 Aug 2005) $
// $Revision: 30670 $

#include <dyn_detail/boost/type_traits/detail/template_arity_spec.hpp>
#include <dyn_detail/boost/type_traits/integral_constant.hpp>
#include <dyn_detail/boost/mpl/aux_/lambda_support.hpp>
#include <dyn_detail/boost/mpl/size_t.hpp>

#include <cstddef>

#if !defined(DYN_DETAIL_BOOST_MSVC) || DYN_DETAIL_BOOST_MSVC >= 1300
#   define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_BASE(C) ::dyn_detail::boost::integral_constant<std::size_t,C>
#   define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) /**/
#else
#   define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_BASE(C) ::dyn_detail::boost::mpl::size_t<C>
#   define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) \
	typedef ::dyn_detail::boost::mpl::size_t<C> base_; \
    using base_::value; \
    /**/
#endif


#define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_DEF1(trait,T,C) \
template< typename T > struct trait \
    : DYN_DETAIL_BOOST_TT_AUX_SIZE_T_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(1,trait,(T)) \
}; \
\
DYN_DETAIL_BOOST_TT_AUX_TEMPLATE_ARITY_SPEC(1,trait) \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_SPEC1(trait,spec,C) \
template<> struct trait<spec> \
    : DYN_DETAIL_BOOST_TT_AUX_SIZE_T_BASE(C) \
{ \
    DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_VALUE_DECL(C) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(1,trait,(spec)) \
}; \
/**/

#define DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_PARTIAL_SPEC1_1(param,trait,spec,C) \
template< param > struct trait<spec> \
    : DYN_DETAIL_BOOST_TT_AUX_SIZE_T_BASE(C) \
{ \
}; \
/**/
