
//  (C) Copyright Rani Sharoni 2003-2005.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.
 
#ifndef DYN_DETAIL_BOOST_TT_IS_BASE_OF_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_BASE_OF_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/is_base_and_derived.hpp>
#include <dyn_detail/boost/type_traits/is_same.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_or.hpp>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF2(
      is_base_of
    , Base
    , Derived
    , (::dyn_detail::boost::type_traits::ice_or<      
         (::dyn_detail::boost::detail::is_base_and_derived_impl<Base,Derived>::value),
         (::dyn_detail::boost::is_same<Base,Derived>::value)>::value)
    )

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_of,Base&,Derived,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_of,Base,Derived&,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_of,Base&,Derived&,false)
#endif

} // namespace boost
} // namespace dyn_detail 

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED
