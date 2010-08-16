//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000-2003.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef DYN_DETAIL_BOOST_TT_IS_CLASS_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_CLASS_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/config.hpp>
#include <dyn_detail/boost/type_traits/intrinsics.hpp>
#ifndef DYN_DETAIL_BOOST_IS_CLASS
#   include <dyn_detail/boost/type_traits/is_union.hpp>
#   include <dyn_detail/boost/type_traits/detail/ice_and.hpp>
#   include <dyn_detail/boost/type_traits/detail/ice_not.hpp>

#ifdef DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION
#   include <dyn_detail/boost/type_traits/detail/yes_no_type.hpp>
#else
#   include <dyn_detail/boost/type_traits/is_scalar.hpp>
#   include <dyn_detail/boost/type_traits/is_array.hpp>
#   include <dyn_detail/boost/type_traits/is_reference.hpp>
#   include <dyn_detail/boost/type_traits/is_void.hpp>
#   include <dyn_detail/boost/type_traits/is_function.hpp>
#endif

#ifdef __EDG_VERSION__
#   include <dyn_detail/boost/type_traits/remove_cv.hpp>
#endif
#endif // DYN_DETAIL_BOOST_IS_CLASS

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

namespace detail {

#ifndef DYN_DETAIL_BOOST_IS_CLASS
#ifdef DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION

// This is actually the conforming implementation which works with
// abstract classes.  However, enough compilers have trouble with
// it that most will use the one in
// boost/type_traits/object_traits.hpp. This implementation
// actually works with VC7.0, but other interactions seem to fail
// when we use it.

// is_class<> metafunction due to Paul Mensonides
// (leavings@attbi.com). For more details:
// http://groups.google.com/groups?hl=en&selm=000001c1cc83%24e154d5e0%247772e50c%40c161550a&rnum=1
#if defined(__GNUC__)  && !defined(__EDG_VERSION__)

template <class U> ::dyn_detail::boost::type_traits::yes_type is_class_tester(void(U::*)(void));
template <class U> ::dyn_detail::boost::type_traits::no_type is_class_tester(...);

template <typename T>
struct is_class_impl
{

    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
        (::dyn_detail::boost::type_traits::ice_and<
            sizeof(is_class_tester<T>(0)) == sizeof(::dyn_detail::boost::type_traits::yes_type),
            ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_union<T>::value >::value
        >::value)
        );
};

#else

template <typename T>
struct is_class_impl
{
    template <class U> static ::dyn_detail::boost::type_traits::yes_type is_class_tester(void(U::*)(void));
    template <class U> static ::dyn_detail::boost::type_traits::no_type is_class_tester(...);

    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
        (::dyn_detail::boost::type_traits::ice_and<
            sizeof(is_class_tester<T>(0)) == sizeof(::dyn_detail::boost::type_traits::yes_type),
            ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_union<T>::value >::value
        >::value)
        );
};

#endif

#else

template <typename T>
struct is_class_impl
{
#   ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
    (::dyn_detail::boost::type_traits::ice_and<
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_union<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_scalar<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_array<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_reference<T>::value>::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_void<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_function<T>::value >::value
        >::value));
#   else
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
    (::dyn_detail::boost::type_traits::ice_and<
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_union<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_scalar<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_array<T>::value >::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_reference<T>::value>::value,
        ::dyn_detail::boost::type_traits::ice_not< ::dyn_detail::boost::is_void<T>::value >::value
        >::value));
#   endif
};

# endif // DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION
# else // DYN_DETAIL_BOOST_IS_CLASS
template <typename T>
struct is_class_impl
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = DYN_DETAIL_BOOST_IS_CLASS(T));
};
# endif // DYN_DETAIL_BOOST_IS_CLASS

} // namespace detail

# ifdef __EDG_VERSION__
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(
   is_class,T, ::dyn_detail::boost::detail::is_class_impl<typename ::dyn_detail::boost::remove_cv<T>::type>::value)
# else 
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_class,T,::dyn_detail::boost::detail::is_class_impl<T>::value)
# endif
    
} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_CLASS_HPP_INCLUDED
