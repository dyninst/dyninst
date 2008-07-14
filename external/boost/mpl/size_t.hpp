
#ifndef BOOST_MPL_SIZE_T_HPP_INCLUDED
#define BOOST_MPL_SIZE_T_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /home/jaw/CVSROOT_20081103/CVSROOT/core/external/boost/mpl/size_t.hpp,v $
// $Date: 2008/07/14 21:54:41 $
// $Revision: 1.1 $

#include <boost/mpl/size_t_fwd.hpp>

#define AUX_WRAPPER_VALUE_TYPE std::size_t
#define AUX_WRAPPER_NAME size_t
#define AUX_WRAPPER_PARAMS(N) std::size_t N

#include <boost/mpl/aux_/integral_wrapper.hpp>

#endif // BOOST_MPL_SIZE_T_HPP_INCLUDED
