//  (C) Copyright John Maddock and Steve Cleary 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_DETAIL_ICE_OR_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_DETAIL_ICE_OR_HPP_INCLUDED

#include <dyn_detail/boost/config.hpp>

namespace dyn_detail {
namespace boost {
namespace type_traits {

template <bool b1, bool b2, bool b3 = false, bool b4 = false, bool b5 = false, bool b6 = false, bool b7 = false>
struct ice_or;

template <bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7>
struct ice_or
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true);
};

template <>
struct ice_or<false, false, false, false, false, false, false>
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
};

} // namespace type_traits
} // namespace boost
} // namespace dyn_detail

#endif // DYN_DETAIL_BOOST_TT_DETAIL_ICE_OR_HPP_INCLUDED
