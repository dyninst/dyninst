
//  (C) Copyright Rani Sharoni 2003.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.
 
#ifndef DYN_DETAIL_BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED
#define DYN_DETAIL_BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED

#include <dyn_detail/boost/type_traits/intrinsics.hpp>
#ifndef DYN_DETAIL_BOOST_IS_BASE_OF
#include <dyn_detail/boost/type_traits/is_class.hpp>
#include <dyn_detail/boost/type_traits/is_same.hpp>
#include <dyn_detail/boost/type_traits/is_convertible.hpp>
#include <dyn_detail/boost/type_traits/detail/ice_and.hpp>
#include <dyn_detail/boost/type_traits/remove_cv.hpp>
#include <dyn_detail/boost/config.hpp>
#include <dyn_detail/boost/static_assert.hpp>
#endif

// should be the last #include
#include <dyn_detail/boost/type_traits/detail/bool_trait_def.hpp>

namespace dyn_detail {
namespace boost {

namespace detail {

#ifndef DYN_DETAIL_BOOST_IS_BASE_OF
#if !DYN_DETAIL_BOOST_WORKAROUND(__BORLANDC__, DYN_DETAIL_BOOST_TESTED_AT(0x581)) \
 && !DYN_DETAIL_BOOST_WORKAROUND(__SUNPRO_CC , <= 0x540) \
 && !DYN_DETAIL_BOOST_WORKAROUND(__EDG_VERSION__, <= 243) \
 && !DYN_DETAIL_BOOST_WORKAROUND(__DMC__, DYN_DETAIL_BOOST_TESTED_AT(0x840))

                             // The EDG version number is a lower estimate.
                             // It is not currently known which EDG version
                             // exactly fixes the problem.

/*************************************************************************

This version detects ambiguous base classes and private base classes
correctly, and was devised by Rani Sharoni.

Explanation by Terje Slettebo and Rani Sharoni.

Let's take the multiple base class below as an example, and the following
will also show why there's not a problem with private or ambiguous base
class:

struct B {};
struct B1 : B {};
struct B2 : B {};
struct D : private B1, private B2 {};

is_base_and_derived<B, D>::value;

First, some terminology:

SC  - Standard conversion
UDC - User-defined conversion

A user-defined conversion sequence consists of an SC, followed by an UDC,
followed by another SC. Either SC may be the identity conversion.

When passing the default-constructed Host object to the overloaded check_sig()
functions (initialization 8.5/14/4/3), we have several viable implicit
conversion sequences:

For "static no_type check_sig(B const volatile *, int)" we have the conversion
sequences:

C -> C const (SC - Qualification Adjustment) -> B const volatile* (UDC)
C -> D const volatile* (UDC) -> B1 const volatile* / B2 const volatile* ->
     B const volatile* (SC - Conversion)

For "static yes_type check_sig(D const volatile *, T)" we have the conversion
sequence:

C -> D const volatile* (UDC)

According to 13.3.3.1/4, in context of user-defined conversion only the
standard conversion sequence is considered when selecting the best viable
function, so it only considers up to the user-defined conversion. For the
first function this means choosing between C -> C const and C -> C, and it
chooses the latter, because it's a proper subset (13.3.3.2/3/2) of the
former. Therefore, we have:

C -> D const volatile* (UDC) -> B1 const volatile* / B2 const volatile* ->
     B const volatile* (SC - Conversion)
C -> D const volatile* (UDC)

Here, the principle of the "shortest subsequence" applies again, and it
chooses C -> D const volatile*. This shows that it doesn't even need to
consider the multiple paths to B, or accessibility, as that possibility is
eliminated before it could possibly cause ambiguity or access violation.

If D is not derived from B, it has to choose between C -> C const -> B const
volatile* for the first function, and C -> D const volatile* for the second
function, which are just as good (both requires a UDC, 13.3.3.2), had it not
been for the fact that "static no_type check_sig(B const volatile *, int)" is
not templated, which makes C -> C const -> B const volatile* the best choice
(13.3.3/1/4), resulting in "no".

Also, if Host::operator B const volatile* hadn't been const, the two
conversion sequences for "static no_type check_sig(B const volatile *, int)", in
the case where D is derived from B, would have been ambiguous.

See also
http://groups.google.com/groups?selm=df893da6.0301280859.522081f7%40posting.
google.com and links therein.

*************************************************************************/

template <typename B, typename D>
struct bd_helper
{
   //
   // This VC7.1 specific workaround stops the compiler from generating
   // an internal compiler error when compiling with /vmg (thanks to
   // Aleksey Gurtovoy for figuring out the workaround).
   //
#if !DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, == 1310)
    template <typename T>
    static type_traits::yes_type check_sig(D const volatile *, T);
    static type_traits::no_type  check_sig(B const volatile *, int);
#else
    static type_traits::yes_type check_sig(D const volatile *, long);
    static type_traits::no_type  check_sig(B const volatile * const&, int);
#endif
};

template<typename B, typename D>
struct is_base_and_derived_impl2
{
#if DYN_DETAIL_BOOST_WORKAROUND(_MSC_FULL_VER, >= 140050000)
#pragma warning(push)
#pragma warning(disable:6334)
#endif
    //
    // May silently do the wrong thing with incomplete types
    // unless we trap them here:
    //
    DYN_DETAIL_BOOST_STATIC_ASSERT(sizeof(B) != 0);
    DYN_DETAIL_BOOST_STATIC_ASSERT(sizeof(D) != 0);

    struct Host
    {
#if !DYN_DETAIL_BOOST_WORKAROUND(DYN_DETAIL_BOOST_MSVC, == 1310)
        operator B const volatile *() const;
#else
        operator B const volatile * const&() const;
#endif
        operator D const volatile *();
    };

    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
        sizeof(bd_helper<B,D>::check_sig(Host(), 0)) == sizeof(type_traits::yes_type));
#if DYN_DETAIL_BOOST_WORKAROUND(_MSC_FULL_VER, >= 140050000)
#pragma warning(pop)
#endif
};

#else

//
// broken version:
//
template<typename B, typename D>
struct is_base_and_derived_impl2
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value =
        (::dyn_detail::boost::is_convertible<D*,B*>::value));
};

#define DYN_DETAIL_BOOST_BROKEN_IS_BASE_AND_DERIVED

#endif

template <typename B, typename D>
struct is_base_and_derived_impl3
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = false);
};

template <bool ic1, bool ic2, bool iss>
struct is_base_and_derived_select
{
   template <class T, class U>
   struct rebind
   {
      typedef is_base_and_derived_impl3<T,U> type;
   };
};

template <>
struct is_base_and_derived_select<true,true,false>
{
   template <class T, class U>
   struct rebind
   {
      typedef is_base_and_derived_impl2<T,U> type;
   };
};

template <typename B, typename D>
struct is_base_and_derived_impl
{
    typedef typename remove_cv<B>::type ncvB;
    typedef typename remove_cv<D>::type ncvD;

    typedef is_base_and_derived_select<
       ::dyn_detail::boost::is_class<B>::value,
       ::dyn_detail::boost::is_class<D>::value,
       ::dyn_detail::boost::is_same<B,D>::value> selector;
    typedef typename selector::template rebind<ncvB,ncvD> binder;
    typedef typename binder::type bound_type;

    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = bound_type::value);
};
#else
template <typename B, typename D>
struct is_base_and_derived_impl
{
    DYN_DETAIL_BOOST_STATIC_CONSTANT(bool, value = DYN_DETAIL_BOOST_IS_BASE_OF(B,D));
};
#endif
} // namespace detail

DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_DEF2(
      is_base_and_derived
    , Base
    , Derived
    , (::dyn_detail::boost::detail::is_base_and_derived_impl<Base,Derived>::value)
    )

#ifndef DYN_DETAIL_BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_and_derived,Base&,Derived,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_and_derived,Base,Derived&,false)
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_2(typename Base,typename Derived,is_base_and_derived,Base&,Derived&,false)
#endif

#if DYN_DETAIL_BOOST_WORKAROUND(__CODEGEARC__, DYN_DETAIL_BOOST_TESTED_AT(0x610))
DYN_DETAIL_BOOST_TT_AUX_BOOL_TRAIT_PARTIAL_SPEC2_1(typename Base,is_base_and_derived,Base,Base,false)
#endif

} // namespace boost
} // namespace dyn_detail 

#include <dyn_detail/boost/type_traits/detail/bool_trait_undef.hpp>

#endif // DYN_DETAIL_BOOST_TT_IS_BASE_AND_DERIVED_HPP_INCLUDED
