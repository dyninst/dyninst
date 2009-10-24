
#ifndef DYN_DETAIL_BOOST_MPL_AUX_PREPROCESSOR_ENUM_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_PREPROCESSOR_ENUM_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: enum.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/preprocessor.hpp>

// DYN_DETAIL_BOOST_MPL_PP_ENUM(0,int): <nothing>
// DYN_DETAIL_BOOST_MPL_PP_ENUM(1,int): int
// DYN_DETAIL_BOOST_MPL_PP_ENUM(2,int): int, int
// DYN_DETAIL_BOOST_MPL_PP_ENUM(n,int): int, int, .., int

#if !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_OWN_PP_PRIMITIVES)

#   include <dyn_detail/boost/preprocessor/cat.hpp>

#   define DYN_DETAIL_BOOST_MPL_PP_ENUM(n, param) \
    DYN_DETAIL_BOOST_PP_CAT(DYN_DETAIL_BOOST_MPL_PP_ENUM_,n)(param) \
    /**/
    
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_0(p)
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_1(p) p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_2(p) p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_3(p) p,p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_4(p) p,p,p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_5(p) p,p,p,p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_6(p) p,p,p,p,p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_7(p) p,p,p,p,p,p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_8(p) p,p,p,p,p,p,p,p
#   define DYN_DETAIL_BOOST_MPL_PP_ENUM_9(p) p,p,p,p,p,p,p,p,p

#else

#   include <dyn_detail/boost/preprocessor/comma_if.hpp>
#   include <dyn_detail/boost/preprocessor/repeat.hpp>

#   define DYN_DETAIL_BOOST_MPL_PP_AUX_ENUM_FUNC(unused, i, param) \
    DYN_DETAIL_BOOST_PP_COMMA_IF(i) param \
    /**/

#   define DYN_DETAIL_BOOST_MPL_PP_ENUM(n, param) \
    DYN_DETAIL_BOOST_PP_REPEAT( \
          n \
        , DYN_DETAIL_BOOST_MPL_PP_AUX_ENUM_FUNC \
        , param \
        ) \
    /**/

#endif

#endif // DYN_DETAIL_BOOST_MPL_AUX_PREPROCESSOR_ENUM_HPP_INCLUDED
