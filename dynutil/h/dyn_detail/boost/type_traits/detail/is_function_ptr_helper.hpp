
//  Copyright 2000 John Maddock (john@johnmaddock.co.uk)
//  Copyright 2002 Aleksey Gurtovoy (agurtovoy@meta-comm.com)
//
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#if !defined(DYN_DETAIL_BOOST_PP_IS_ITERATING)

///// header body

#ifndef DYN_DETAIL_BOOST_TT_DETAIL_IS_FUNCTION_PTR_HELPER_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_DETAIL_IS_FUNCTION_PTR_HELPER_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/config.hpp>

#if defined(DYN_DETAIL_BOOST_TT_PREPROCESSING_MODE)
#   include <dyn_detail/boost/preprocessor/iterate.hpp>
#   include <dyn_detail/boost/preprocessor/enum_params.hpp>
#   include <dyn_detail/boost/preprocessor/comma_if.hpp>
#endif

namespace dyn_detail {
namespace boost {
namespace type_traits {

template <class R>
struct is_function_ptr_helper
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
};

#if !defined(DYN_DETAIL_BOOST_TT_PREPROCESSING_MODE)
// preprocessor-generated part, don't edit by hand!

template <class R >
struct is_function_ptr_helper<R (*)()> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R >
struct is_function_ptr_helper<R (*)( ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0>
struct is_function_ptr_helper<R (*)( T0)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0>
struct is_function_ptr_helper<R (*)( T0 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1>
struct is_function_ptr_helper<R (*)( T0 , T1)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1>
struct is_function_ptr_helper<R (*)( T0 , T1 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21 , class T22>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 , T22)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21 , class T22>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 , T22 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21 , class T22 , class T23>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 , T22 , T23)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21 , class T22 , class T23>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 , T22 , T23 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21 , class T22 , class T23 , class T24>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 , T22 , T23 , T24)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R , class T0 , class T1 , class T2 , class T3 , class T4 , class T5 , class T6 , class T7 , class T8 , class T9 , class T10 , class T11 , class T12 , class T13 , class T14 , class T15 , class T16 , class T17 , class T18 , class T19 , class T20 , class T21 , class T22 , class T23 , class T24>
struct is_function_ptr_helper<R (*)( T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18 , T19 , T20 , T21 , T22 , T23 , T24 ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
#endif
#else

#undef DYN_DETAIL_BOOST_STATIC_CONSTANT
#define DYN_DETAIL_BOOST_PP_ITERATION_PARAMS_1 \
    (3, (0, 25, "boost/type_traits/detail/is_function_ptr_helper.hpp"))
#include DYN_DETAIL_BOOST_PP_ITERATE()

#endif // DYN_DETAIL_BOOST_TT_PREPROCESSING_MODE

} // namespace type_traits
} // namespace boost
} // namespace dyn_detail

#endif // DYN_DETAIL_BOOST_TT_DETAIL_IS_FUNCTION_PTR_HELPER_HPP_INCLUDED

///// iteration

#else
#define DYN_DETAIL_BOOST_PP_COUNTER DYN_DETAIL_BOOST_PP_FRAME_ITERATION(1)

template <class R DYN_DETAIL_BOOST_PP_COMMA_IF(DYN_DETAIL_BOOST_PP_COUNTER) DYN_DETAIL_BOOST_PP_ENUM_PARAMS(DYN_DETAIL_BOOST_PP_COUNTER,class T)>
struct is_function_ptr_helper<R (*)(DYN_DETAIL_BOOST_PP_ENUM_PARAMS(DYN_DETAIL_BOOST_PP_COUNTER,T))> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
@#ifndef DYN_DETAIL_BOOST_TT_NO_ELLIPSIS_IN_FUNC_TESTING
template <class R DYN_DETAIL_BOOST_PP_COMMA_IF(DYN_DETAIL_BOOST_PP_COUNTER) DYN_DETAIL_BOOST_PP_ENUM_PARAMS(DYN_DETAIL_BOOST_PP_COUNTER,class T)>
struct is_function_ptr_helper<R (*)(DYN_DETAIL_BOOST_PP_ENUM_PARAMS(DYN_DETAIL_BOOST_PP_COUNTER,T) ...)> { DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = true); };
@#endif
#undef DYN_DETAIL_BOOST_PP_COUNTER
#endif // DYN_DETAIL_BOOST_PP_IS_ITERATING
