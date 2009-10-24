
#ifndef DYN_DETAIL_BOOST_MPL_LAMBDA_FWD_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_LAMBDA_FWD_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2001-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: lambda_fwd.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/void_fwd.hpp>
#include <dyn_detail/boost/mpl/aux_/na.hpp>
#include <dyn_detail/boost/mpl/aux_/config/lambda.hpp>

#if !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT)

#   include <dyn_detail/boost/mpl/int.hpp>
#   include <dyn_detail/boost/mpl/aux_/lambda_arity_param.hpp>
#   include <dyn_detail/boost/mpl/aux_/template_arity_fwd.hpp>

namespace dyn_detail {namespace boost { namespace mpl {

template< 
      typename T = na
    , typename Tag = void_
    DYN_DETAIL_BOOST_MPL_AUX_LAMBDA_ARITY_PARAM(
          typename Arity = int_< aux::template_arity<T>::value >
        )
    >
struct lambda;

}} }

#else // DYN_DETAIL_BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT

#   include <dyn_detail/boost/mpl/bool.hpp>

namespace dyn_detail { namespace boost { namespace mpl {

template< 
      typename T = na
    , typename Tag = void_
    , typename Protect = true_
    > 
struct lambda;

}} }

#endif

#endif // DYN_DETAIL_BOOST_MPL_LAMBDA_FWD_HPP_INCLUDED
