
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef DYN_DETAIL_BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/config.hpp>
#include <dyn_detail/boost/detail/workaround.hpp>

#if !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) \
   && !DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x600) && !defined(DYN_DETAIL_BOOST_TT_TEST_MS_FUNC_SIGS)
   //
   // Note: we use the "workaround" version for MSVC because it works for 
   // __stdcall etc function types, where as the partial specialisation
   // version does not do so.
   //
#   include <dyn_detail/boost/type_traits/detail/is_mem_fun_pointer_impl.hpp>
#   include <dyn_detail/boost/type_traits/remove_cv.hpp>
#else
#   include <dyn_detail/boost/type_traits/is_reference.hpp>
#   include <dyn_detail/boost/type_traits/is_array.hpp>
#   include <dyn_detail/boost/type_traits/detail/yes_no_type.hpp>
#   include <dyn_detail/boost/type_traits/detail/false_result.hpp>
#   include <dyn_detail/boost/type_traits/detail/ice_or.hpp>
#   include <dyn_detail/boost/type_traits/detail/is_mem_fun_pointer_tester.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#if defined( __CODEGEARC__ )
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_member_function_pointer,T,__is_member_function_pointer( T ))
#elif !defined(DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION) && !DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, < 0x600) && !defined(DYN_DETAIL_BOOST_TT_TEST_MS_FUNC_SIGS)

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(
      is_member_function_pointer
    , T
    , ::dyn_detail::boost::type_traits::is_mem_fun_pointer_impl<typename remove_cv<T>::type>::value
    )

#else

namespace detail {

#ifndef __BORLANDC__

template <bool>
struct is_mem_fun_pointer_select
    : ::dyn_detail::boost::type_traits::false_result
{
};

template <>
struct is_mem_fun_pointer_select<false>
{
    template <typename T> struct result_
    {
#if DYN_DETAIL_BOOST_WORKAROUND(_MSC_FULL_VER, >= 140050000)
#pragma warning(push)
#pragma warning(disable:6334)
#endif
        static T* make_t;
        typedef result_<T> self_type;

        DYN_DETAIL_BOOST_STATIC_CONSTANT(
            bool, value = (
                1 == sizeof(::dyn_detail::boost::type_traits::is_mem_fun_pointer_tester(self_type::make_t))
            ));
#if DYN_DETAIL_BOOST_WORKAROUND(_MSC_FULL_VER, >= 140050000)
#pragma warning(pop)
#endif
    };
};

template <typename T>
struct is_member_function_pointer_impl
    : is_mem_fun_pointer_select<
          ::dyn_detail::boost::type_traits::ice_or<
              ::dyn_detail::boost::is_reference<T>::value
            , ::dyn_detail::boost::is_array<T>::value
            >::value
        >::template result_<T>
{
};

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
template <typename T>
struct is_member_function_pointer_impl<T&> : public false_type{};
#endif

#else // Borland C++

template <typename T>
struct is_member_function_pointer_impl
{
   static T* m_t;
   DYN_DETAIL_BOOST_STATIC_CONSTANT(
              bool, value =
               (1 == sizeof(type_traits::is_mem_fun_pointer_tester(m_t))) );
};

template <typename T>
struct is_member_function_pointer_impl<T&>
{
   DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
};

#endif

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void,false)
#ifndef DYN_DETAIL_BOOST_NO_CV_VOID_SPECIALIZATIONS
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void const,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void volatile,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_IMPL_SPEC1(is_member_function_pointer,void const volatile,false)
#endif

} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF1(is_member_function_pointer,T,::dyn_detail::boost::detail::is_member_function_pointer_impl<T>::value)

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_MEMBER_FUNCTION_POINTER_HPP_INCLUDED
