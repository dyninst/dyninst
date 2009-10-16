
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


#ifndef DYN_DETAIL_BOOST_TT_IS_VOLATILE_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_VOLATILE_HPP_INCLUDED

#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/detail/workaround.hpp>

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   include <dyn_detail/boost/type_traits/detail/cv_traits_impl.hpp>
#   if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1400)
#       include <dyn_detail/boost/type_traits/remove_bounds.hpp>
#   endif
#else
#   include <dyn_detail/boost/type_traits/is_reference.hpp>
#   include <dyn_detail/boost/type_traits/is_array.hpp>
#   include <dyn_detail/boost/type_traits/detail/yes_no_type.hpp>
#   include <dyn_detail/boost/type_traits/detail/false_result.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_volatile,T,__is_volatile(T))
#elif !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

//* is a type T declared volatile - is_volatile<T>
#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1400)
   DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_volatile,T,::dyn_detail::boost::detail::cv_traits_imp<typename dyn_detail::boost::remove_bounds<T>::type*>::is_volatile)
#else
   DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_volatile,T,::dyn_detail::boost::detail::cv_traits_imp<T*>::is_volatile)
#endif
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T&,false)

#if  defined(DYN_DETAIL_BOOST_ILLEGAL_CV_REFERENCES)
// these are illegal specialisations; cv-qualifies applied to
// references have no effect according to [8.3.2p1],
// C++ Builder requires them though as it treats cv-qualified
// references as distinct types...
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T& const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T& volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_volatile,T& const volatile,false)
#endif

#else

namespace detail {

	using ::dyn_detail::boost::type_traits::yes_type;
	using ::dyn_detail::boost::type_traits::no_type;

yes_type is_volatile_tester(void const volatile*);
no_type is_volatile_tester(void const*);

template <bool is_ref, bool array>
struct is_volatile_helper
    : ::dyn_detail::boost::type_traits::false_result
{
};

template <>
struct is_volatile_helper<false,false>
{
    template <typename T> struct result_
    {
        static T* t;
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (
            sizeof(detail::yes_type) == sizeof(detail::is_volatile_tester(t))
            ));
    };
};

template <>
struct is_volatile_helper<false,true>
{
    template <typename T> struct result_
    {
        static T t;
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (
            sizeof(detail::yes_type) == sizeof(detail::is_volatile_tester(&t))
            ));
    };
};

template <typename T>
struct is_volatile_impl
    : is_volatile_helper<
          is_reference<T>::value
        , is_array<T>::value
        >::template result_<T>
{
};

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_volatile,void,false)
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_volatile,void const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_volatile,void volatile,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_volatile,void const volatile,true)
#endif

} // namespace detail

//* is a type T declared volatile - is_volatile<T>
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_volatile,T,::dyn_detail::boost::detail::is_volatile_impl<T>::value)

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_VOLATILE_HPP_INCLUDED
