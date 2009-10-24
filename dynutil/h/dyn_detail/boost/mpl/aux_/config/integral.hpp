
#ifndef DYN_DETAIL_BOOST_MPL_AUX_CONFIG_INTEGRAL_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_CONFIG_INTEGRAL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: integral.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/msvc.hpp>
#include <dyn_detail/boost/mpl/aux_/config/workaround.hpp>

#if    !defined(DYN_DETAIL_BOOST_MPL_CFG_BCC_INTEGRAL_CONSTANTS) \
    && !defined(DYN_DETAIL_BOOST_MPL_PREPROCESSING_MODE) \
    && DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, DYN_DETAIL_BOOST_TESTED_AT(0x610))

#   define DYN_DETAIL_BOOST_MPL_CFG_BCC_INTEGRAL_CONSTANTS

#endif

#if    !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_NESTED_VALUE_ARITHMETIC) \
    && !defined(DYN_DETAIL_BOOST_MPL_PREPROCESSING_MODE) \
    && ( DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, <= 1300) \
        || DYN_DETAIL_BOOST_WORKAROUND(__EDG_VERSION__, <= 238) \
        )

#   define DYN_DETAIL_BOOST_MPL_CFG_NO_NESTED_VALUE_ARITHMETIC

#endif

#endif // DYN_DETAIL_BOOST_MPL_AUX_CONFIG_INTEGRAL_HPP_INCLUDED
