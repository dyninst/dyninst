
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


#ifndef DYN_DETAIL_BOOST_TT_IS_POINTER_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_POINTER_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/is_member_pointer.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_and.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_not.hpp>
#include <dyn_detail/boost/type_traits/config.hpp>
#if !DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC,<=1300)
#include <dyn_detail/boost/type_traits/remove_cv.hpp>
#endif

#ifdef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   include <dyn_detail/boost/type_traits/is_reference.hpp>
#   include <dyn_detail/boost/type_traits/is_array.hpp>
#   include <dyn_detail/boost/type_traits/detail/is_function_ptr_tester.hpp>
#   include <dyn_detail/boost/type_traits/detail/false_result.hpp>
#   include <dyn_detail/boost/type_traits/detail/ice_or.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_pointer,T,__is_pointer(T))
#elif !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

namespace detail {

template< typename T > struct is_pointer_helper
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
};

#   define TT_AUX_BOOL_TRAIT_HELPER_PARTIAL_SPEC(helper,sp,result) \
template< typename T > struct helper<sp> \
{ \
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = result); \
}; \
/**/

TT_AUX_BOOL_TRAIT_HELPER_PARTIAL_SPEC(is_pointer_helper,T*,true)

#   undef TT_AUX_BOOL_TRAIT_HELPER_PARTIAL_SPEC

template< typename T >
struct is_pointer_impl
{
#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC,<=1300)
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
        (::dyn_detail::boost::type_traits::ice_and<
              ::dyn_detail::boost::detail::is_pointer_helper<T>::value
            , ::dyn_detail::boost::type_traits::ice_not<
                ::dyn_detail::boost::is_member_pointer<T>::value
                >::value
            >::value)
        );
#else
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
        (::dyn_detail::boost::type_traits::ice_and<
        ::dyn_detail::boost::detail::is_pointer_helper<typename remove_cv<T>::type>::value
            , ::dyn_detail::boost::type_traits::ice_not<
                ::dyn_detail::boost::is_member_pointer<T>::value
                >::value
            >::value)
        );
#endif
};

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_pointer,T,::dyn_detail::boost::detail::is_pointer_impl<T>::value)

#if defined(__BORLANDC__) && !defined(__COMO__) && (__BORLANDC__ < 0x600)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_pointer,T&,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_pointer,T& const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_pointer,T& volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC1_1(typename T,is_pointer,T& const volatile,false)
#endif

#else // no partial template specialization

namespace detail {

struct pointer_helper
{
    pointer_helper(const volatile void*);
};

yes_type DYN_DETAIL_BOOST_TT_DECL is_pointer_tester(pointer_helper);
no_type DYN_DETAIL_BOOST_TT_DECL is_pointer_tester(...);

template <bool>
struct is_pointer_select
    : ::dyn_detail::boost::type_traits::false_result
{
};

template <>
struct is_pointer_select<false>
{
    template <typename T> struct result_
    {
        static T& make_t();
        DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
                (::dyn_detail::boost::type_traits::ice_or<
                    (1 == sizeof(is_pointer_tester(make_t()))),
                    (1 == sizeof(type_traits::is_function_ptr_tester(make_t())))
                >::value));
    };
};

template <typename T>
struct is_pointer_impl
    : is_pointer_select<
          ::dyn_detail::boost::type_traits::ice_or<
              ::dyn_detail::boost::is_reference<T>::value
            , ::dyn_detail::boost::is_array<T>::value
            >::value
        >::template result_<T>
{
};

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pointer,void,false)
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pointer,void const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pointer,void volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_pointer,void const volatile,false)
#endif

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_pointer,T,::dyn_detail::boost::detail::is_pointer_impl<T>::value)

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_POINTER_HPP_INCLUDED
