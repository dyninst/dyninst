
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_CONFIG_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_CONFIG_HPP_INCLUDED

#ifndef DYN_DETAIL_BOOST_CONFIG_HPP
#include <dyn_detail/boost/config.hpp>
#endif

#include <dyn_detail/boost/detail/workaround.hpp>

//
// whenever we have a conversion function with elipses
// it needs to be declared __cdecl to suppress compiler
// warnings from MS and Borland compilers (this *must*
// appear before we include is_same.hpp below):
#if defined(DYN_DETAIL_BOOST_MSVC) || (defined(__BORLANDC__) && !defined(DYN_DETAIL_BOOST_DISABLE_WIN32))
#   define DYN_DETAIL_BOOST_TT_DECL __cdecl
#else
#   define DYN_DETAIL_BOOST_TT_DECL /**/
#endif

# if (DYN_DETAIL_BOOST_WORKAROUND(__MWERKS__, < 0x3000)                         \
    || DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, <= 1301)                        \
    || !defined(__EDG_VERSION__) && DYN_DETAIL_BOOST_WORKAROUND(__GNUC__, < 3) \
    || DYN_DETAIL_BOOST_WORKAROUND(__IBMCPP__, < 600 )                         \
    || DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x5A0)                      \
    || defined(__ghs)                                               \
    || DYN_DETAIL_BOOST_WORKAROUND(__HP_aCC, < 60700)           \
    || DYN_DETAIL_BOOST_WORKAROUND(MPW_CPLUS, DYN_DETAIL_BOOST_TESTED_AT(0x890))          \
    || DYN_DETAIL_BOOST_WORKAROUND(__SUNPRO_CC, DYN_DETAIL_BOOST_TESTED_AT(0x580)))       \
    && defined(DYN_DETAIL_BOOST_NO_IS_ABSTRACT)

#   define DYN_DETAIL_BOOST_TT_NO_CONFORMING_IS_CLASS_IMPLEMENTATION 1

#endif

#ifndef DYN_DETAIL_BOOST_TT_NO_CONFORMING_IS_CLASS_IMPLEMENTATION
# define DYN_DETAIL_BOOST_TT_HAS_CONFORMING_IS_CLASS_IMPLEMENTATION 1
#endif

//
// Define DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING 
// when we can't test for function types with elipsis:
//
#if DYN_DETAIL_BOOST_WORKAROUND(__GNUC__, < 3)
#  define DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
#endif

//
// define DYN_DETAIL_BOOST_TT_TEST_MS_FUNC_SIGS
// when we want to test __stdcall etc function types with is_function etc
// (Note, does not work with Borland, even though it does support __stdcall etc):
//
#if defined(_MSC_EXTENSIONS) && !defined(__BORLANDC__)
#  define DYN_DETAIL_BOOST_TT_TEST_MS_FUNC_SIGS
#endif

//
// define DYN_DETAIL_BOOST_TT_NO_CV_FUNC_TEST
// if tests for cv-qualified member functions don't 
// work in is_member_function_pointer
//
#if DYN_DETAIL_BOOST_WORKAROUND(__MWERKS__, < 0x3000) || DYN_DETAIL_BOOST_WORKAROUND(__IBMCPP__, <= 600)
#  define DYN_DETAIL_BOOST_TT_NO_CV_FUNC_TEST
#endif

#endif // DYN_DETAIL_BOOST_TT_CONFIG_HPP_INCLUDED


