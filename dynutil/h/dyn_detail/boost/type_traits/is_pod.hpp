
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef DYN_DETAIL_BOOST_TT_IS_POD_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_POD_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/config.hpp>
#include <dyn_detail/boost/type_traits/is_void.hpp>
#include <dyn_detail/boost/type_traits/is_scalar.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_or.hpp>
#include <dyn_detail/boost/type_traits/intrinsics.hpp>

#include <cstddef>

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

// forward declaration, needed by 'is_pod_array_helper' template below
template< typename T > struct is_POD;

namespace detail {

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

template <typename T> struct is_pod_impl
{ 
    DYN_DETAIL_BOOST_STATIC_CONSTANT(
        bool, value =
        (::dyn_detail::boost::type_traits::ice_or<
            ::dyn_detail::boost::is_scalar<T>::value,
            ::dyn_detail::boost::is_void<T>::value,
            DYN_DETAIL_BOOST_IS_POD(T)
         >::value));
};

#if !defined(DYN_DETAIL_BOOST_NO_ARRAY_TYPE_SPECIALIZATIONS)
template <typename T, std::size_t sz>
struct is_pod_impl<T[sz]>
    : is_pod_impl<T>
{
};
#endif

#else

template <bool is_array = false>
struct is_pod_helper
{
    template <typename T> struct result_
    {
        DYN_DETAIL_BOOST_STATIC_CONSTANT(
            bool, value =
            (::dyn_detail::boost::type_traits::ice_or<
                ::dyn_detail::boost::is_scalar<T>::value,
                ::dyn_detail::boost::is_void<T>::value,
                DYN_DETAIL_BOOST_IS_POD(T)
            >::value));
    };
};

template <bool b>
struct bool_to_yes_no_type
{
	typedef ::dyn_detail::boost::type_traits::no_type type;
};

template <>
struct bool_to_yes_no_type<true>
{
	typedef ::dyn_detail::boost::type_traits::yes_type type;
};

template <typename ArrayType>
struct is_pod_array_helper
{
    enum { is_pod = ::dyn_detail::boost::is_POD<ArrayType>::value }; // MSVC workaround
    typedef typename bool_to_yes_no_type<is_pod>::type type;
    type instance() const;
};

template <typename T>
is_pod_array_helper<T> is_POD_array(T*);

template <>
struct is_pod_helper<true>
{
    template <typename T> struct result_
    {
        static T& help();
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
            sizeof(is_POD_array(help()).instance()) == sizeof(::dyn_detail::boost::type_traits::yes_type)
            );
    };
};


template <typename T> struct is_pod_impl
{ 
   DYN_DETAIL_BOOST_STATIC_CONSTANT(
       bool, value = (
           ::dyn_detail::boost::detail::is_pod_helper<
              ::dyn_detail::boost::is_array<T>::value
           >::template result_<T>::value
           )
       );
};

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

// the following help compilers without partial specialization support:
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void,true)

#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void const,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void volatile,true)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pod,void const volatile,true)
#endif

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_POD,T,::dyn_detail::boost::detail::is_pod_impl<T>::value)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_pod,T,::dyn_detail::boost::detail::is_pod_impl<T>::value)

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_POD_HPP_INCLUDED
