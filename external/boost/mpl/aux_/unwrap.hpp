
#ifndef BOOST_MPL_AUX_UNWRAP_HPP_INCLUDED
#define BOOST_MPL_AUX_UNWRAP_HPP_INCLUDED

// Copyright Peter Dimov and Multi Media Ltd 2001, 2002
// Copyright David Abrahams 2001
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /home/jaw/CVSROOT_20081103/CVSROOT/core/external/boost/mpl/aux_/unwrap.hpp,v $
// $Date: 2008/07/14 21:54:48 $
// $Revision: 1.1 $

#include <boost/ref.hpp>

namespace boost { namespace mpl { namespace aux {

template< typename F >
inline
F& unwrap(F& f, long)
{
    return f;
}

template< typename F >
inline
F&
unwrap(reference_wrapper<F>& f, int)
{
    return f;
}

template< typename F >
inline
F&
unwrap(reference_wrapper<F> const& f, int)
{
    return f;
}

}}}

#endif // BOOST_MPL_AUX_UNWRAP_HPP_INCLUDED
