//  (C) Copyright John Maddock 2005. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef DYN_DETAIL_BOOST_TYPE_TRAITS_INTEGRAL_CONSTANT_HPP
#define DYN_DETAIL_BOOST_TYPE_TRAITS_INTEGRAL_CONSTANT_HPP

#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/mpl/bool.hpp>
#include <dyn_detail/boost/mpl/integral_c.hpp>

namespace dyn_detail{
namespace boost{

#if defined(DYN_DETAIL_BOOST_NO_DEPENDENT_TYPES_IN_TEMPLATE_VALUE_PARAMETERS) || defined(__BORLANDC__)
template <class T, int val>
#else
template <class T, T val>
#endif
struct integral_constant : public mpl::integral_c<T, val>
{
   typedef integral_constant<T,val> type;
};

template<> struct integral_constant<bool,true> : public mpl::true_ 
{
#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1300)
# pragma warning(push)
# pragma warning(disable:4097)
   typedef mpl::true_ base_;
   using base_::value;
# pragma warning(pop)
#endif
   typedef integral_constant<bool,true> type;
};
template<> struct integral_constant<bool,false> : public mpl::false_ 
{
#if DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, < 1300)
# pragma warning(push)
# pragma warning(disable:4097)
   typedef mpl::false_ base_;
   using base_::value;
# pragma warning(pop)
#endif
   typedef integral_constant<bool,false> type;
};

typedef integral_constant<bool,true> true_type;
typedef integral_constant<bool,false> false_type;

}
}

#endif
