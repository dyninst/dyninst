
//  (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, Howard
//  Hinnant & John Maddock 2000.  
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.


#ifndef DYN_DETAIL_BOOST_TT_REMOVE_CV_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_REMOVE_CV_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/broken_compiler_spec.hpp>
#include <dyn_detail/boost/type_traits/detail/cv_traits_impl.hpp>
#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/detail/workaround.hpp>

#include <cstddef>

#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC,<=1300)
#include <dyn_detail/boost/type_traits/msvc/remove_cv.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/type_trait_def.hpp>

namespace dyn_detail {
namespace boost {

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

//  convert a type T to a non-cv-qualified type - remove_cv<T>
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_DEF1(remove_cv,T,typename ::dyn_detail::boost::detail::cv_traits_imp<T*>::unqualified_type)
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_1(typename T,remove_cv,T&,T&)
#if !defined(DYN_DETAIL_BOOST_NO_ARRAY_TYPE_SPECIALIZATIONS)
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_cv,T const[N],T type[N])
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_cv,T volatile[N],T type[N])
DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_PARTIAL_SPEC1_2(typename T,std::size_t N,remove_cv,T const volatile[N],T type[N])
#endif

#elif !DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC,<=1300)

namespace detail {
template <typename T>
struct remove_cv_impl
{
    typedef typename remove_volatile_impl< 
          typename remove_const_impl<T>::type
        >::type type;
};
}

DYN_DETAIL_BOOST_TT_AUX_TYPE_TRAIT_DEF1(remove_cv,T,typename ::dyn_detail::boost::detail::remove_cv_impl<T>::type)

#endif // DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost
} // namespace dyn_detail 

#include <dyn_detail/boost/type_traits/detail/type_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_REMOVE_CV_HPP_INCLUDED
