
#ifndef DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: na_spec.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#if !defined(DYN_DETAIL_BOOST_MPL_PREPROCESSING_MODE)
#   include <dyn_detail/boost/mpl/lambda_fwd.hpp>
#   include <dyn_detail/boost/mpl/int.hpp>
#   include <dyn_detail/boost/mpl/bool.hpp>
#   include <dyn_detail/boost/mpl/aux_/na.hpp>
#   include <dyn_detail/boost/mpl/aux_/arity.hpp>
#   include <dyn_detail/boost/mpl/aux_/template_arity_fwd.hpp>
#endif

#include <dyn_detail/boost/mpl/aux_/preprocessor/params.hpp>
#include <dyn_detail/boost/mpl/aux_/preprocessor/enum.hpp>
#include <dyn_detail/boost/mpl/aux_/preprocessor/def_params_tail.hpp>
#include <dyn_detail/boost/mpl/aux_/lambda_arity_param.hpp>
#include <dyn_detail/boost/mpl/aux_/config/dtp.hpp>
#include <dyn_detail/boost/mpl/aux_/config/eti.hpp>
#include <dyn_detail/boost/mpl/aux_/nttp_decl.hpp>
#include <dyn_detail/boost/mpl/aux_/config/ttp.hpp>
#include <dyn_detail/boost/mpl/aux_/config/lambda.hpp>
#include <dyn_detail/boost/mpl/aux_/config/overload_resolution.hpp>


#define DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) \
    DYN_DETAIL_BOOST_MPL_PP_ENUM(i, na) \
/**/

#if defined(DYN_DETAIL_BOOST_MPL_CFG_BROKEN_DEFAULT_PARAMETERS_IN_NESTED_TEMPLATES)
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ARITY(i, name) \
 namespace aux { \
template< DYN_DETAIL_BOOST_MPL_AUX_NTTP_DECL(int, N) > \
struct arity< \
          name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > \
        , N \
        > \
    : int_< DYN_DETAIL_BOOST_MPL_LIMIT_METAFUNCTION_ARITY > \
{ \
}; \
} \
/**/
#else
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ARITY(i, name) /**/
#endif

#define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_MAIN(i, name) \
template<> \
struct name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > \
{ \
    template< \
          DYN_DETAIL_BOOST_MPL_PP_PARAMS(i, typename T) \
        DYN_DETAIL_BOOST_MPL_PP_NESTED_DEF_PARAMS_TAIL(i, typename T, na) \
        > \
    struct apply \
        : name< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i, T) > \
    { \
    }; \
}; \
/**/

#if defined(DYN_DETAIL_BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_LAMBDA(i, name) \
template<> \
struct lambda< \
      name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > \
    , void_ \
    , true_ \
    > \
{ \
    typedef false_ is_le; \
    typedef name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > type; \
}; \
template<> \
struct lambda< \
      name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > \
    , void_ \
    , false_ \
    > \
{ \
    typedef false_ is_le; \
    typedef name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > type; \
}; \
/**/
#else
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_LAMBDA(i, name) \
template< typename Tag > \
struct lambda< \
      name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > \
    , Tag \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(int_<-1>) \
    > \
{ \
    typedef false_ is_le; \
    typedef name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > result_; \
    typedef name< DYN_DETAIL_BOOST_MPL_AUX_NA_PARAMS(i) > type; \
}; \
/**/
#endif

#if defined(DYN_DETAIL_BOOST_MPL_CFG_EXTENDED_TEMPLATE_PARAMETERS_MATCHING) \
    || defined(DYN_DETAIL_BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT) \
        && defined(DYN_DETAIL_BOOST_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION)
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_TEMPLATE_ARITY(i, j, name) \
namespace aux { \
template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(j, typename T) > \
struct template_arity< \
          name< DYN_DETAIL_BOOST_MPL_PP_PARAMS(j, T) > \
        > \
    : int_<j> \
{ \
}; \
\
template<> \
struct template_arity< \
          name< DYN_DETAIL_BOOST_MPL_PP_ENUM(i, na) > \
        > \
    : int_<-1> \
{ \
}; \
}  \
/**/
#else
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_TEMPLATE_ARITY(i, j, name) /**/
#endif

#if defined(DYN_DETAIL_BOOST_MPL_CFG_MSVC_ETI_BUG)
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ETI(i, name) \
template<> \
struct name< DYN_DETAIL_BOOST_MPL_PP_ENUM(i, int) > \
{ \
    typedef int type; \
    enum { value = 0 }; \
}; \
/**/
#else
#   define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ETI(i, name) /**/
#endif

#define DYN_DETAIL_BOOST_MPL_AUX_NA_PARAM(param) param = na

#define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_NO_ETI(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_MAIN(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_LAMBDA(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ARITY(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_TEMPLATE_ARITY(i, i, name) \
/**/

#define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_NO_ETI(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ETI(i, name) \
/**/

#define DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC2(i, j, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_MAIN(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ETI(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_LAMBDA(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_ARITY(i, name) \
DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_TEMPLATE_ARITY(i, j, name) \
/**/


#endif // DYN_DETAIL_BOOST_MPL_AUX_NA_SPEC_HPP_INCLUDED
