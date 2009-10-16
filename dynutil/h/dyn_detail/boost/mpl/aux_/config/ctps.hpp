
#ifndef DYN_DETAIL_BOOST_MPL_AUX_CONFIG_CTPS_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_CONFIG_CTPS_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: ctps.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/workaround.hpp>
#include <dyn_detail/boost/config.hpp>

#if    !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_NONTYPE_TEMPLATE_PARTIAL_SPEC) \
    && !defined(DYN_DETAIL_BOOST_MPL_PREPROCESSING_MODE) \
    && DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x582)

#   define DYN_DETAIL_BOOST_MPL_CFG_NO_NONTYPE_TEMPLATE_PARTIAL_SPEC

#endif

// DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION is defined in <boost/config.hpp>

#endif // DYN_DETAIL_BOOST_MPL_AUX_CONFIG_CTPS_HPP_INCLUDED
