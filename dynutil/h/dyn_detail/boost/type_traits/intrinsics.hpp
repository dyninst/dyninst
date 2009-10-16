
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_INTRINSICS_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_INTRINSICS_HPP_INCLUDED

#ifndef DYN_DETAIL_BOOST_TT_CONFIG_HPP_INCLUDED
#include <dyn_detail/boost/type_traits/config.hpp>
#endif

//
// Helper macros for builtin compiler support.
// If your compiler has builtin support for any of the following
// traits concepts, then redefine the appropriate macros to pick
// up on the compiler support:
//
// (these should largely ignore cv-qualifiers)
// DYN_DETAIL_BOOST_IS_UNION(T) should evaluate to true if T is a union type
// DYN_DETAIL_BOOST_IS_POD(T) should evaluate to true if T is a POD type
// DYN_DETAIL_BOOST_IS_EMPTY(T) should evaluate to true if T is an empty struct or union
// DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) should evaluate to true if "T x;" has no effect
// DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) should evaluate to true if T(t) <==> memcpy
// DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) should evaluate to true if t = u <==> memcpy
// DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) should evaluate to true if ~T() has no effect
// DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR(T) should evaluate to true if "T x;" can not throw
// DYN_DETAIL_BOOST_HAS_NOTHROW_COPY(T) should evaluate to true if T(t) can not throw
// DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN(T) should evaluate to true if t = u can not throw
// DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR(T) should evaluate to true T has a virtual destructor
//
// The following can also be defined: when detected our implementation is greatly simplified.
// Note that unlike the macros above these do not have default definitions, so we can use
// #ifdef MACRONAME to detect when these are available.
//
// DYN_DETAIL_BOOST_IS_ABSTRACT(T) true if T is an abstract type
// DYN_DETAIL_BOOST_IS_BASE_OF(T,U) true if T is a base class of U
// DYN_DETAIL_BOOST_IS_CLASS(T) true if T is a class type
// DYN_DETAIL_BOOST_IS_CONVERTIBLE(T,U) true if T is convertible to U
// DYN_DETAIL_BOOST_IS_ENUM(T) true is T is an enum
// DYN_DETAIL_BOOST_IS_POLYMORPHIC(T) true if T is a polymorphic type
// DYN_DETAIL_BOOST_ALIGNMENT_OF(T) should evaluate to the alignment requirements of type T.

#ifdef DYN_DETAIL_BOOST_HAS_SGI_TYPE_TRAITS
    // Hook into SGI's __type_traits class, this will pick up user supplied
    // specializations as well as SGI - compiler supplied specializations.
#   include <dyn_detail/boost/type_traits/is_same.hpp>
#   ifdef __NetBSD__
      // There are two different versions of type_traits.h on NetBSD on Spark
      // use an implicit include via algorithm instead, to make sure we get
      // the same version as the std lib:
#     include <algorithm>
#   else
#    include <type_traits.h>
#   endif
#   define DYN_DETAIL_BOOST_IS_POD(T) ::dyn_detail::boost::is_same< typename ::__type_traits<T>::is_POD_type, ::__true_type>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) ::dyn_detail::boost::is_same< typename ::__type_traits<T>::has_trivial_default_constructor, ::__true_type>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) ::dyn_detail::boost::is_same< typename ::__type_traits<T>::has_trivial_copy_constructor, ::__true_type>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) ::dyn_detail::boost::is_same< typename ::__type_traits<T>::has_trivial_assignment_operator, ::__true_type>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) ::dyn_detail::boost::is_same< typename ::__type_traits<T>::has_trivial_destructor, ::__true_type>::value

#   ifdef __sgi
#      define DYN_DETAIL_BOOST_HAS_TYPE_TRAITS_INTRINSICS
#   endif
#endif

#if defined(__MSL_CPP__) && (__MSL_CPP__ >= 0x8000)
    // Metrowerks compiler is acquiring intrinsic type traits support
    // post version 8.  We hook into the published interface to pick up
    // user defined specializations as well as compiler intrinsics as 
    // and when they become available:
#   include <msl_utility>
#   define DYN_DETAIL_BOOST_IS_UNION(T) DYN_DETAIL_BOOST_STD_EXTENSION_NAMESPACE::is_union<T>::value
#   define DYN_DETAIL_BOOST_IS_POD(T) DYN_DETAIL_BOOST_STD_EXTENSION_NAMESPACE::is_POD<T>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) DYN_DETAIL_BOOST_STD_EXTENSION_NAMESPACE::has_trivial_default_ctor<T>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) DYN_DETAIL_BOOST_STD_EXTENSION_NAMESPACE::has_trivial_copy_ctor<T>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) DYN_DETAIL_BOOST_STD_EXTENSION_NAMESPACE::has_trivial_assignment<T>::value
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) DYN_DETAIL_BOOST_STD_EXTENSION_NAMESPACE::has_trivial_dtor<T>::value
#   define DYN_DETAIL_BOOST_HAS_TYPE_TRAITS_INTRINSICS
#endif

#if defined(DYN_DETAIL_BOOST_MSVC) && defined(_MSC_FULL_VER) && (_MSC_FULL_VER >=140050215)
#   include <dyn_detail/boost/type_traits/is_same.hpp>

#   define DYN_DETAIL_BOOST_IS_UNION(T) __is_union(T)
#   define DYN_DETAIL_BOOST_IS_POD(T) (__is_pod(T) && __has_trivial_constructor(T))
#   define DYN_DETAIL_BOOST_IS_EMPTY(T) __is_empty(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) __has_trivial_copy(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR(T) __has_nothrow_constructor(T)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_COPY(T) __has_nothrow_copy(T)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN(T) __has_nothrow_assign(T)
#   define DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR(T) __has_virtual_destructor(T)

#   define DYN_DETAIL_BOOST_IS_ABSTRACT(T) __is_abstract(T)
#   define DYN_DETAIL_BOOST_IS_BASE_OF(T,U) (__is_base_of(T,U) && !is_same<T,U>::value)
#   define DYN_DETAIL_BOOST_IS_CLASS(T) __is_class(T)
//  This one doesn't quite always do the right thing:
//  #   define DYN_DETAIL_BOOST_IS_CONVERTIBLE(T,U) __is_convertible_to(T,U)
#   define DYN_DETAIL_BOOST_IS_ENUM(T) __is_enum(T)
//  This one doesn't quite always do the right thing:
//  #   define DYN_DETAIL_BOOST_IS_POLYMORPHIC(T) __is_polymorphic(T)
#   define DYN_DETAIL_BOOST_ALIGNMENT_OF(T) __alignof(T)

#   define DYN_DETAIL_BOOST_HAS_TYPE_TRAITS_INTRINSICS
#endif

#if defined(__DMC__) && (__DMC__ >= 0x848)
// For Digital Mars C++, www.digitalmars.com
#   define DYN_DETAIL_BOOST_IS_UNION(T) (__typeinfo(T) & 0x400)
#   define DYN_DETAIL_BOOST_IS_POD(T) (__typeinfo(T) & 0x800)
#   define DYN_DETAIL_BOOST_IS_EMPTY(T) (__typeinfo(T) & 0x1000)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) (__typeinfo(T) & 0x10)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) (__typeinfo(T) & 0x20)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) (__typeinfo(T) & 0x40)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) (__typeinfo(T) & 0x8)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR(T) (__typeinfo(T) & 0x80)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_COPY(T) (__typeinfo(T) & 0x100)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN(T) (__typeinfo(T) & 0x200)
#   define DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR(T) (__typeinfo(T) & 0x4)
#   define DYN_DETAIL_BOOST_HAS_TYPE_TRAITS_INTRINSICS
#endif

#if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3)))
#   include <dyn_detail/boost/type_traits/is_same.hpp>
#   include <dyn_detail/boost/type_traits/is_reference.hpp>
#   include <dyn_detail/boost/type_traits/is_volatile.hpp>

#   define DYN_DETAIL_BOOST_IS_UNION(T) __is_union(T)
#   define DYN_DETAIL_BOOST_IS_POD(T) __is_pod(T)
#   define DYN_DETAIL_BOOST_IS_EMPTY(T) __is_empty(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) (__has_trivial_copy(T) && !is_reference<T>::value)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR(T) __has_nothrow_constructor(T)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_COPY(T) (__has_nothrow_copy(T) && !is_volatile<T>::value && !is_reference<T>::value)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN(T) (__has_nothrow_assign(T) && !is_volatile<T>::value)
#   define DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR(T) __has_virtual_destructor(T)

#   define DYN_DETAIL_BOOST_IS_ABSTRACT(T) __is_abstract(T)
#   define DYN_DETAIL_BOOST_IS_BASE_OF(T,U) (__is_base_of(T,U) && !is_same<T,U>::value)
#   define DYN_DETAIL_BOOST_IS_CLASS(T) __is_class(T)
#   define DYN_DETAIL_BOOST_IS_ENUM(T) __is_enum(T)
#   define DYN_DETAIL_BOOST_IS_POLYMORPHIC(T) __is_polymorphic(T)
#   define DYN_DETAIL_BOOST_ALIGNMENT_OF(T) __alignof__(T)

#   define DYN_DETAIL_BOOST_HAS_TYPE_TRAITS_INTRINSICS
#endif

# if defined(__CODEGEARC__)
#   include <dyn_detail/boost/type_traits/is_same.hpp>
#   include <dyn_detail/boost/type_traits/is_reference.hpp>
#   include <dyn_detail/boost/type_traits/is_volatile.hpp>
#   include <dyn_detail/boost/type_traits/is_void.hpp>

#   define DYN_DETAIL_BOOST_IS_UNION(T) __is_union(T)
#   define DYN_DETAIL_BOOST_IS_POD(T) __is_pod(T)
#   define DYN_DETAIL_BOOST_IS_EMPTY(T) __is_empty(T)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) (__has_trivial_default_constructor(T) || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) (__has_trivial_copy_constructor(T) && !is_volatile<T>::value && !is_reference<T>::value || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) (__has_trivial_assign(T) && !is_volatile<T>::value || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) (__has_trivial_destructor(T) || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR(T) (__has_nothrow_default_constructor(T) || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_COPY(T) (__has_nothrow_copy_constructor(T) && !is_volatile<T>::value && !is_reference<T>::value || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN(T) (__has_nothrow_assign(T) && !is_volatile<T>::value || is_void<T>::value)
#   define DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR(T) __has_virtual_destructor(T)

#   define DYN_DETAIL_BOOST_IS_ABSTRACT(T) __is_abstract(T)
#   define DYN_DETAIL_BOOST_IS_BASE_OF(T,U) (__is_base_of(T,U) && !is_void<T>::value && !is_void<U>::value)
#   define DYN_DETAIL_BOOST_IS_CLASS(T) __is_class(T)
#   define DYN_DETAIL_BOOST_IS_CONVERTIBLE(T,U) (__is_convertible(T,U) || is_void<U>::value)
#   define DYN_DETAIL_BOOST_IS_ENUM(T) __is_enum(T)
#   define DYN_DETAIL_BOOST_IS_POLYMORPHIC(T) __is_polymorphic(T)
#   define DYN_DETAIL_BOOST_ALIGNMENT_OF(T) alignof(T)

#   define DYN_DETAIL_BOOST_HAS_TYPE_TRAITS_INTRINSICS
#endif

#ifndef DYN_DETAIL_BOOST_IS_UNION
#   define DYN_DETAIL_BOOST_IS_UNION(T) false
#endif

#ifndef DYN_DETAIL_BOOST_IS_POD
#   define DYN_DETAIL_BOOST_IS_POD(T) false
#endif

#ifndef DYN_DETAIL_BOOST_IS_EMPTY
#   define DYN_DETAIL_BOOST_IS_EMPTY(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_CONSTRUCTOR(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_COPY(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_ASSIGN(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR
#   define DYN_DETAIL_BOOST_HAS_TRIVIAL_DESTRUCTOR(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_CONSTRUCTOR(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_NOTHROW_COPY
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_COPY(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN
#   define DYN_DETAIL_BOOST_HAS_NOTHROW_ASSIGN(T) false
#endif

#ifndef DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR
#   define DYN_DETAIL_BOOST_HAS_VIRTUAL_DESTRUCTOR(T) false
#endif

#endif // DYN_DETAIL_BOOST_TT_INTRINSICS_HPP_INCLUDED





