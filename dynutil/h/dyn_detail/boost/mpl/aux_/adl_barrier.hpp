
#ifndef DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_HPP_INCLUDED
#define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2002-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: adl_barrier.hpp 49267 2008-10-11 06:19:02Z agurtovoy $
// $Date: 2008-10-11 02:19:02 -0400 (Sat, 11 Oct 2008) $
// $Revision: 49267 $

#include <dyn_detail/boost/mpl/aux_/config/adl.hpp>
#include <dyn_detail/boost/mpl/aux_/config/gcc.hpp>
#include <dyn_detail/boost/mpl/aux_/config/workaround.hpp>

#if !defined(DYN_DETAIL_BOOST_MPL_CFG_NO_ADL_BARRIER_NAMESPACE)

#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE mpl_
#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN namespace mpl_ {
#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE  }
#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_DECL(type) \
    namespace dyn_detail {namespace boost { namespace mpl { \
    using ::dyn_detail::DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE::type; \
    } } } \
/**/

#if !defined(DYN_DETAIL_BOOST_MPL_PREPROCESSING_MODE)
namespace dyn_detail {namespace DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE { namespace aux {} } }
namespace dyn_detail {namespace boost { namespace mpl { using namespace DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE; 
namespace aux { using namespace DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE::aux; }
}}}
#endif

#else // DYN_DETAIL_BOOST_MPL_CFG_NO_ADL_BARRIER_NAMESPACE

#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE dyn_detail::boost::mpl
#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_OPEN namespace dyn_detail { namespace boost { namespace mpl {
#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_NAMESPACE_CLOSE }}}
#   define DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_DECL(type) /**/

#endif

#endif // DYN_DETAIL_BOOST_MPL_AUX_ADL_BARRIER_HPP_INCLUDED
