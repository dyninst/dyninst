
#ifndef DYN_DETAIL_BOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: overload_resolution.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/workaround.hpp>

#if    !defined(DYN_DETAIL_BOOST_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION) \
    && !defined(DYN_DETAIL_BOOST_MPL_PREPROCESSING_MODE) \
    && (   DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x590) \
        || DYN_DETAIL_BOOST_WORKAROUND(__MWERKS__, < 0x3001) \
        )

#   define DYN_DETAIL_BOOST_MPL_CFG_BROKEN_OVERLOAD_RESOLUTION

#endif

#endif // DYN_DETAIL_BOOST_MPL_AUX_CONFIG_OVERLOAD_RESOLUTION_HPP_INCLUDED
