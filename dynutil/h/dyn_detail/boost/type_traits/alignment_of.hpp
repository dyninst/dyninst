
//  (C) Copyright John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_ALIGNMENT_OF_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_ALIGNMENT_OF_HPP_INCLUDED

#include <dyn_detail/boost/config.hpp>
#include <cstddef>

#include <dyn_detail/boost/type_traits/intrinsics.hpp>
// should be the last #include
#include <dyn_detail/boost/type_traits/detail/size_t_trait_def.hpp>

#ifdef DYN_DETAIL_BOOST_MSVC
#   pragma warning(push)
#   pragma warning(disable: 4121 4512) // alignment is sensitive to packing
#endif
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x600)
#pragma option push -Vx- -Ve-
#endif

namespace dyn_detail {
namespace boost {

template <typename T> struct alignment_of;

// get the alignment of some arbitrary type:
namespace detail {

#ifdef DYN_DETAIL_BOOST_MSVC
#pragma warning(push)
#pragma warning(disable:4324) // structure was padded due to __declspec(align())
#endif
template <typename T>
struct alignment_of_hack
{
    char c;
    T t;
    alignment_of_hack();
};
#ifdef DYN_DETAIL_BOOST_MSVC
#pragma warning(pop)
#endif

template <unsigned A, unsigned S>
struct alignment_logic
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(std::size_t, value = A < S ? A : S);
};


template< typename T >
struct alignment_of_impl
{
#ifndef DYN_DETAIL_BOOST_ALIGNMENT_OF
    DYN_DETAIL_BOOST_STATIC_CONSTANT(std::size_t, value =
        (::dyn_detail::boost::detail::alignment_logic<
            sizeof(::dyn_detail::boost::detail::alignment_of_hack<T>) - sizeof(T),
            sizeof(T)
        >::value));
#else
   //
   // We put this here, rather than in the definition of
   // alignment_of below, because MSVC's __alignof doesn't
   // always work in that context for some unexplained reason.
   // (See type_with_alignment tests for test cases).
   //
   DYN_DETAIL_BOOST_STATIC_CONSTANT(std::size_t, value = DYN_DETAIL_BOOST_ALIGNMENT_OF(T));
#endif
};

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_DEF1(alignment_of,T,::dyn_detail::boost::detail::alignment_of_impl<T>::value)

// references have to be treated specially, assume
// that a reference is just a special pointer:
#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
template <typename T>
struct alignment_of<T&>
    : alignment_of<T*>
{
};
#endif
#ifdef __BORLANDC__
// long double gives an incorrect value of 10 (!)
// unless we do this...
struct long_double_wrapper{ long double ld; };
template<> struct alignment_of<long double>
   : public alignment_of<long_double_wrapper>{};
#endif

// void has to be treated specially:
DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_SPEC1(alignment_of,void,0)
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_SPEC1(alignment_of,void const,0)
DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_SPEC1(alignment_of,void volatile,0)
DYN_DETAIL_BOOST_TT_AUX_SIZE_T_TRAIT_SPEC1(alignment_of,void const volatile,0)
#endif

} // namespace boost
} // namespace dyn_detail

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x600)
#pragma option pop
#endif
#ifdef DYN_DETAIL_BOOST_MSVC
#   pragma warning(pop)
#endif

#include <dyn_detail/boost/type_traits/detail/size_t_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_ALIGNMENT_OF_HPP_INCLUDED

