
#ifndef DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: lambda_support.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/lambda.hpp>

#if !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) /**/
#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(i,name,params) /**/

#else
#error

#   include <dyn_detail/boost/mpl/int_fwd.hpp>
#   include <dyn_detail/boost/mpl/aux_/yes_no.hpp>
#   include <dyn_detail/boost/mpl/aux_/na_fwd.hpp>
#   include <dyn_detail/boost/mpl/aux_/preprocessor/params.hpp>
#   include <dyn_detail/boost/mpl/aux_/preprocessor/enum.hpp>
#   include <dyn_detail/boost/mpl/aux_/config/msvc.hpp>
#   include <dyn_detail/boost/mpl/aux_/config/workaround.hpp>

#   include <dyn_detail/boost/preprocessor/tuple/to_list.hpp>
#   include <dyn_detail/boost/preprocessor/list/for_each_i.hpp>
#   include <dyn_detail/boost/preprocessor/inc.hpp>
#   include <dyn_detail/boost/preprocessor/cat.hpp>

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC(R,typedef_,i,param) \
    typedef_ param DYN_DETAIL_BOOST_PP_CAT(arg,DYN_DETAIL_BOOST_PP_INC(i)); \
    /**/

// agurt, 07/mar/03: restore an old revision for the sake of SGI MIPSpro C++
#if DYN_DETAIL_BOOST_WORKAROUND(__EDG_VERSION__, <= 238) 

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    typedef DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE::int_<i> arity; \
    DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R( \
          1 \
        , DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC \
        , typedef \
        , DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST(i,params) \
        ) \
    struct rebind \
    { \
        template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename U) > struct apply \
            : name< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,U) > \
        { \
        }; \
    }; \
    /**/

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    /**/

#elif DYN_DETAIL_BOOST_WORKAROUND(__EDG_VERSION__, <= 244) && !defined(DYN_DETAIL_BOOST_INTEL_CXX_VERSION)
// agurt, 18/jan/03: old EDG-based compilers actually enforce 11.4 para 9
// (in strict mode), so we have to provide an alternative to the 
// MSVC-optimized implementation

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
    typedef DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE::int_<i> arity; \
    DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R( \
          1 \
        , DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC \
        , typedef \
        , DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST(i,params) \
        ) \
    struct rebind; \
/**/

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
}; \
template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename T) > \
struct name<DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,T)>::rebind \
{ \
    template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename U) > struct apply \
        : name< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,U) > \
    { \
    }; \
/**/

#else // __EDG_VERSION__

namespace dyn_detail {namespace boost { namespace mpl { namespace aux {
template< typename T > struct has_rebind_tag;
}}}}

#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
    typedef DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE::int_<i> arity; \
    DYN_DETAIL_BOOST_PP_LIST_FOR_EACH_I_R( \
          1 \
        , DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_ARG_TYPEDEF_FUNC \
        , typedef \
        , DYN_DETAIL_BOOST_PP_TUPLE_TO_LIST(i,params) \
        ) \
    friend class DYN_DETAIL_BOOST_PP_CAT(name,_rebind); \
    typedef DYN_DETAIL_BOOST_PP_CAT(name,_rebind) rebind; \
/**/

#if DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, DYN_DETAIL_BOOST_TESTED_AT(0x610))
#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename T) > \
dyn_detail::boost::mpl::aux::yes_tag operator|( \
      dyn_detail::boost::mpl::aux::has_rebind_tag<int> \
    , name<DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,T)>* \
    ); \
dyn_detail::boost::mpl::aux::no_tag operator|( \
      dyn_detail::boost::mpl::aux::has_rebind_tag<int> \
    , name< DYN_DETAIL_BOOST_MPL_PP_ENUM(i,dyn_detail::boost::mpl::na) >* \
    ); \
/**/
#elif !DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1300)
#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename T) > \
dyn_detail::boost::mpl::aux::yes_tag operator|( \
      dyn_detail::boost::mpl::aux::has_rebind_tag<int> \
    , dyn_detail::boost::mpl::aux::has_rebind_tag< name<DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,T)> >* \
    ); \
/**/
#else
#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) /**/
#endif

#   if !defined(__BORLANDC__)
#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
}; \
DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
class DYN_DETAIL_BOOST_PP_CAT(name,_rebind) \
{ \
 public: \
    template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename U) > struct apply \
        : name< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,U) > \
    { \
    }; \
/**/
#   else
#   define DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT(i, name, params) \
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_SPEC(i, name, params) \
}; \
DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HAS_REBIND(i, name, params) \
class DYN_DETAIL_BOOST_PP_CAT(name,_rebind) \
{ \
 public: \
    template< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,typename U) > struct apply \
    { \
        typedef typename name< DYN_DETAIL_BOOST_MPL_PP_PARAMS(i,U) >::type type; \
    }; \
/**/
#   endif // __BORLANDC__

#endif // __EDG_VERSION__

#endif // DYN_DETAIL_BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT

#endif // DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_SUPPORT_HPP_INCLUDED
