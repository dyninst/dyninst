
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef DYN_DETAIL_BOOST_TT_IS_ENUM_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_ENUM_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/intrinsics.hpp>
#ifndef DYN_DETAIL_BOOST_IS_ENUM
#include <dyn_detail/boost/type_traits/add_reference.hpp>
#include <dyn_detail/boost/type_traits/is_arithmetic.hpp>
#include <dyn_detail/boost/type_traits/is_reference.hpp>
#include <dyn_detail/boost/type_traits/is_convertible.hpp>
#include <dyn_detail/boost/type_traits/is_array.hpp>
#ifdef __GNUC__
#include <dyn_detail/boost/type_traits/is_function.hpp>
#endif
#include <dyn_detail/boost/type_traits/config.hpp>
#if defined(DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION) 
#  include <dyn_detail/boost/type_traits/is_class.hpp>
#  include <dyn_detail/boost/type_traits/is_union.hpp>
#endif
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#ifndef DYN_DETAIL_BOOST_IS_ENUM
#if !(defined(__BORLANDC__) && (__BORLANDC__ <= 0x551))

namespace detail {

#if defined(DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION) 

template <typename T>
struct is_class_or_union
{
   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
      (::dyn_detail::boost::type_traits::ice_or<
           ::dyn_detail::boost::is_class<T>::value
         , ::dyn_detail::boost::is_union<T>::value
      >::value));
};

#else

template <typename T>
struct is_class_or_union
{
# if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1300) || DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, DYN_DETAIL_BOOST_TESTED_AT(0x581))// we simply can't detect it this way.
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
# else
    template <class U> static ::dyn_detail::boost::type_traits::yes_type is_class_or_union_tester(void(U::*)(void));

#  if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, == 1300)                 \
    || DYN_DETAIL_BOOST_WORKAROUND(__MWERKS__, <= 0x3000) // no SFINAE
    static ::dyn_detail::boost::type_traits::no_type is_class_or_union_tester(...);
    DYN_DETAIL_BOOST_STATIC_CONSTANT(
        bool, value = sizeof(is_class_or_union_tester(0)) == sizeof(::dyn_detail::boost::type_traits::yes_type));
#  else
    template <class U>
    static ::dyn_detail::boost::type_traits::no_type is_class_or_union_tester(...);
    DYN_DETAIL_BOOST_STATIC_CONSTANT(
        bool, value = sizeof(is_class_or_union_tester<T>(0)) == sizeof(::dyn_detail::boost::type_traits::yes_type));
#  endif
# endif
};
#endif

struct int_convertible
{
    int_convertible(int);
};

// Don't evaluate convertibility to int_convertible unless the type
// is non-arithmetic. This suppresses warnings with GCC.
template <bool is_typename_arithmetic_or_reference = true>
struct is_enum_helper
{
    template <typename T> struct type
    {
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
    };
};

template <>
struct is_enum_helper<false>
{
    template <typename T> struct type
       : ::dyn_detail::boost::is_convertible<typename ::dyn_detail::boost::add_reference<T>::type,::dyn_detail::boost::detail::int_convertible>
    {
    };
};

template <typename T> struct is_enum_impl
{
   //typedef ::boost::add_reference<T> ar_t;
   //typedef typename ar_t::type r_type;

#if defined(__GNUC__)

#ifdef DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION
    
   // We MUST check for is_class_or_union on conforming compilers in
   // order to correctly deduce that noncopyable types are not enums
   // (dwa 2002/04/15)...
   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, selector =
      (::dyn_detail::boost::type_traits::ice_or<
           ::dyn_detail::boost::is_arithmetic<T>::value
         , ::dyn_detail::boost::is_reference<T>::value
         , ::dyn_detail::boost::is_function<T>::value
         , is_class_or_union<T>::value
         , is_array<T>::value
      >::value));
#else
   // ...however, not checking is_class_or_union on non-conforming
   // compilers prevents a dependency recursion.
   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, selector =
      (::dyn_detail::boost::type_traits::ice_or<
           ::dyn_detail::boost::is_arithmetic<T>::value
         , ::dyn_detail::boost::is_reference<T>::value
         , ::dyn_detail::boost::is_function<T>::value
         , is_array<T>::value
      >::value));
#endif // DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION

#else // !defined(__GNUC__):
    
   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, selector =
      (::dyn_detail::boost::type_traits::ice_or<
           ::dyn_detail::boost::is_arithmetic<T>::value
         , ::dyn_detail::boost::is_reference<T>::value
         , is_class_or_union<T>::value
         , is_array<T>::value
      >::value));
    
#endif

#if DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x600)
   typedef ::dyn_detail::boost::detail::is_enum_helper<
          ::dyn_detail::boost::detail::is_enum_impl<T>::selector
        > se_t;
#else
   typedef ::dyn_detail::boost::detail::is_enum_helper<selector> se_t;
#endif

    typedef typename se_t::template type<T> helper;
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = helper::value);
};

// these help on compilers with no partial specialization support:
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_enum,void,false)
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_enum,void const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_enum,void volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_enum,void const volatile,false)
#endif

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_enum,T,::dyn_detail::boost::detail::is_enum_impl<T>::value)

#else // __BORLANDC__
//
// buggy is_convertible prevents working
// implementation of is_enum:
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_enum,T,false)

#endif

#else // DYN_DETAIL_BOOST_IS_ENUM

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_enum,T,DYN_DETAIL_BOOST_IS_ENUM(T))

#endif

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_ENUM_HPP_INCLUDED
