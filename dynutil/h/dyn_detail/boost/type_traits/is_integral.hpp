
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_IS_INTEGRAL_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_INTEGRAL_HPP_INCLUDED

#include <dyn_detail/boost/config.hpp>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

//* is a type T an [cv-qualified-] integral type described in the standard (3.9.1p3)
// as an extention we include long long, as this is likely to be added to the
// standard at a later date
#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,__is_integral(T))
#else
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_integral,T,false)

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned char,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned short,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned int,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned long,true)

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed char,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed short,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed int,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,signed long,true)

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,bool,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,char,true)

#ifdef JAW_FIXME_KLUDGE_TO_GET_WINDOWS_BUILDING_NOT_DEFINED 
//#ifndef DYN_DETAIL_BOOST_NO_INTRINSIC_WCHAR_T
// If the following line fails to compile and you're using the Intel
// compiler, see http://lists.boost.org/MailArchives/boost-users/msg06567.php,
// and define DYN_DETAIL_BOOST_NO_INTRINSIC_WCHAR_T on the command line.
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,wchar_t,true)
#endif

// Same set of integral types as in boost/type_traits/integral_promotion.hpp.
// Please, keep in sync. -- Alexander Nasonov
#if (defined(DYN_DETAIL_BOOST_MSVC) && (DYN_DETAIL_BOOST_MSVC < 1300)) \
    || (defined(DYN_DETAIL_BOOST_INTEL_CXX_VERSION) && defined(_MSC_VER) && (DYN_DETAIL_BOOST_INTEL_CXX_VERSION <= 600)) \
    || (defined(__BORLANDC__) && (__BORLANDC__ == 0x600) && (_MSC_VER < 1300))
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int8,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int8,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int16,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int16,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int32,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int32,true)
#ifdef __BORLANDC__
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif
#endif

# if defined(DYN_DETAIL_BOOST_HAS_LONG_LONG)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral, ::dyn_detail::boost::ulong_long_type,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral, ::dyn_detail::boost::long_long_type,true)
#elif defined(DYN_DETAIL_BOOST_HAS_MS_INT64)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,unsigned __int64,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_integral,__int64,true)
#endif

#endif  // non-CodeGear implementation

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_INTEGRAL_HPP_INCLUDED
