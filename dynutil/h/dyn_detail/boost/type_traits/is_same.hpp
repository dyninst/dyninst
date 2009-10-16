
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, 
//      Howard Hinnant and John Maddock 2000. 
//  (C) Copyright Mat Marcus, Jesse Jones and Adobe Systems Inc 2001

//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

//    Fixed is_pointer, is_reference, is_const, is_volatile, is_same, 
//    is_member_pointer based on the Simulated Partial Specialization work 
//    of Mat Marcus and Jesse Jones. See  http://opensource.adobe.com or 
//    http://groups.yahoo.com/group/boost/message/5441 
//    Some workarounds in here use ideas suggested from "Generic<Programming>: 
//    Mappings between Types and Values" 
//    by Andrei Alexandrescu (see http://www.cuj.com/experts/1810/alexandr.html).


#ifndef DYN_DETAIL_BOOST_TT_IS_SAME_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_SAME_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/config.hpp>
#ifdef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#error
#include <dyn_detail/boost/type_traits/detail/yes_no_type.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_and.hpp>
#include <dyn_detail/boost/type_traits/is_reference.hpp>
#endif
// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF2(is_same,T,U,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(typename T,is_same,T,T,true)
#if DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x600)
// without this, Borland's compiler gives the wrong answer for
// references to arrays:
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(typename T,is_same,T&,T&,true)
#endif

#else // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

namespace detail {

#ifdef DYN_DETAIL_BOOST_MSVC
// the following VC6 specific implementation is *NOT* legal
// C++, but has the advantage that it works for incomplete
// types.

template< typename T1 >
struct is_same_part_1
{
    template<typename T2>  struct part_2     { enum { value = false }; };
    template<>             struct part_2<T1> { enum { value = true }; };
};

template< typename T1, typename T2 >
struct is_same_impl
{
    enum { value = detail::is_same_part_1<T1>::template part_2<T2>::value };
};

#else // generic "no-partial-specialization" version

template <typename T>
::dyn_detail::boost::type_traits::yes_type
DYN_DETAIL_BOOST_TT_DECL is_same_tester(T*, T*);

::dyn_detail::boost::type_traits::no_type
DYN_DETAIL_BOOST_TT_DECL is_same_tester(...);

template <typename T, typename U>
struct is_same_impl
{
   static T t;
   static U u;

   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
      (::dyn_detail::boost::type_traits::ice_and<
         (sizeof(type_traits::yes_type) == sizeof(detail::is_same_tester(&t,&u))),
         (::dyn_detail::boost::is_reference<T>::value == ::dyn_detail::boost::is_reference<U>::value),
         (sizeof(T) == sizeof(U))
        >::value));
};

#endif // DYN_DETAIL_BOOST_MSVC

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF2(is_same,T,U,(::dyn_detail::boost::detail::is_same_impl<T,U>::value))

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif  // DYN_DETAIL_BOOST_TT_IS_SAME_HPP_INCLUDED

