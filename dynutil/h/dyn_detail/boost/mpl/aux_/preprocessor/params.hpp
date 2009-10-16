
#ifndef DYN_DETAIL_BOOST_MPL_AUX_PREPROCESSOR_PARAMS_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_PREPROCESSOR_PARAMS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: params.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/preprocessor.hpp>

// DYN_DETAIL_BOOST_MPL_PP_PARAMS(0,T): <nothing>
// DYN_DETAIL_BOOST_MPL_PP_PARAMS(1,T): T1
// DYN_DETAIL_BOOST_MPL_PP_PARAMS(2,T): T1, T2
// DYN_DETAIL_BOOST_MPL_PP_PARAMS(n,T): T1, T2, .., Tn

#if !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <dyn_detail/boost/preprocessor/cat.hpp>

#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS(n,p) \
    DYN_DETAIL_BOOST_PP_CAT(DYN_DETAIL_BOOST_MPL_PP_PARAMS_,n)(p) \
    /**/

#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_0(p)
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_1(p) p##1
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_2(p) p##1,p##2
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_3(p) p##1,p##2,p##3
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_4(p) p##1,p##2,p##3,p##4
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_5(p) p##1,p##2,p##3,p##4,p##5
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_6(p) p##1,p##2,p##3,p##4,p##5,p##6
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_7(p) p##1,p##2,p##3,p##4,p##5,p##6,p##7
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_8(p) p##1,p##2,p##3,p##4,p##5,p##6,p##7,p##8
#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS_9(p) p##1,p##2,p##3,p##4,p##5,p##6,p##7,p##8,p##9

#else

#   include <dyn_detail/boost/preprocessor/comma_if.hpp>
#   include <dyn_detail/boost/preprocessor/repeat.hpp>
#   include <dyn_detail/boost/preprocessor/inc.hpp>
#   include <dyn_detail/boost/preprocessor/cat.hpp>

#   define DYN_DETAIL_BOOST_MPL_PP_AUX_PARAM_FUNC(unused, i, param) \
    DYN_DETAIL_BOOST_PP_COMMA_IF(i) \
    DYN_DETAIL_BOOST_PP_CAT(param, DYN_DETAIL_BOOST_PP_INC(i)) \
    /**/

#   define DYN_DETAIL_BOOST_MPL_PP_PARAMS(n, param) \
    DYN_DETAIL_BOOST_PP_REPEAT( \
          n \
        , DYN_DETAIL_BOOST_MPL_PP_AUX_PARAM_FUNC \
        , param \
        ) \
    /**/

#endif 

#endif // DYN_DETAIL_BOOST_MPL_AUX_PREPROCESSOR_PARAMS_HPP_INCLUDED
