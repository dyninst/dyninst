
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


#ifndef DYN_DETAIL_DYN_DETAIL_BOOST_TT_IS_CONST_HPP_INCLUDED
#define DYN_DETAIL_DYN_DETAIL_BOOST_TT_IS_CONST_HPP_INCLUDED

#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/detail/workaround.hpp>

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   include <dyn_detail/boost/type_traits/detail/cv_traits_impl.hpp>
#   ifdef __GNUC__
#       include <dyn_detail/boost/type_traits/is_reference.hpp>
#   endif
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

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_const,T,__is_const(T))

#elif !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

//* is a type T  declared const - is_const<T>
#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1400)
   DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_const,T,::dyn_detail::boost::detail::cv_traits_imp<typename remove_bounds<T>::type*>::is_const)
#else
   DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_const,T,::dyn_detail::boost::detail::cv_traits_imp<T*>::is_const)
#endif
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_const,T&,false)

#if  defined(DYN_DETAIL_BOOST_ILLEGAL_CV_REFERENCES)
// these are illegal specialisations; cv-qualifies applied to
// references have no effect according to [8.3.2p1],
// C++ Builder requires them though as it treats cv-qualified
// references as distinct types...
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_const,T& const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_const,T& volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_const,T& const volatile,false)
#endif

#if defined(__GNUC__) && (__GNUC__ < 3)
// special case for gcc where illegally cv-qualified reference types can be
// generated in some corner cases:
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_const,T const,!(::dyn_detail::boost::is_reference<T>::value))
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_const,T volatile const,!(::dyn_detail::boost::is_reference<T>::value))
#endif

#else

namespace detail {

	using ::dyn_detail::boost::type_traits::yes_type;
	using ::dyn_detail::boost::type_traits::no_type;

yes_type is_const_tester(const volatile void*);
no_type is_const_tester(volatile void *);

template <bool is_ref, bool array>
struct is_const_helper
    : ::dyn_detail::boost::type_traits::false_result
{
};

template <>
struct is_const_helper<false,false>
{
    template <typename T> struct result_
    {
        static T* t;
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (
            sizeof(detail::yes_type) == sizeof(detail::is_const_tester(t))
            ));
    };
};

template <>
struct is_const_helper<false,true>
{
    template <typename T> struct result_
    {
        static T t;
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = (
            sizeof(detail::yes_type) == sizeof(detail::is_const_tester(&t))
            ));
    };
};

template <typename T>
struct is_const_impl
    : is_const_helper<
          is_reference<T>::value
        , is_array<T>::value
        >::template result_<T>
{
};

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_const,void,false)
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_const,void const,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_const,void volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_const,void const volatile,true)
#endif

} // namespace detail

//* is a type T  declared const - is_const<T>
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_const,T,::dyn_detail::boost::detail::is_const_impl<T>::value)

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_CONST_HPP_INCLUDED

