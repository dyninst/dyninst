
#ifndef BOOST_MPL_AUX_ERASE_KEY_IMPL_HPP_INCLUDED
#define BOOST_MPL_AUX_ERASE_KEY_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /home/jaw/CVSROOT_20081103/CVSROOT/core/external/boost/mpl/aux_/erase_key_impl.hpp,v $
// $Date: 2008/07/14 21:54:45 $
// $Revision: 1.1 $

#include <boost/mpl/erase_key_fwd.hpp>
#include <boost/mpl/aux_/traits_lambda_spec.hpp>

namespace boost { namespace mpl {

template< typename Tag >
struct erase_key_impl
{
    template< typename Sequence, typename Key > struct apply;
};

BOOST_MPL_ALGORITM_TRAITS_LAMBDA_SPEC(2, erase_key_impl)

}}

#endif // BOOST_MPL_AUX_ERASE_KEY_IMPL_HPP_INCLUDED
