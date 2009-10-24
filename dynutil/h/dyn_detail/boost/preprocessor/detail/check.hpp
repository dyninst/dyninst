# /* **************************************************************************
#  *                                                                          *
#  *     (C) Copyright Paul Mensonides 2002.
#  *     Distributed under the Boost Software License, Version 1.0. (See
#  *     accompanying file LICENSE_1_0.txt or copy at
#  *     http://www.boost.org/LICENSE_1_0.txt)
#  *                                                                          *
#  ************************************************************************** */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_DETAIL_CHECK_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_DETAIL_CHECK_HPP
#
# include <dyn_detail/boost/preprocessor/cat.hpp>
# include <dyn_detail/boost/preprocessor/config/config.hpp>
#
# /* DYN_DETAIL_BOOST_PP_CHECK */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_CHECK(x, type) DYN_DETAIL_BOOST_PP_CHECK_D(x, type)
# else
#    define DYN_DETAIL_BOOST_PP_CHECK(x, type) DYN_DETAIL_BOOST_PP_CHECK_OO((x, type))
#    define DYN_DETAIL_BOOST_PP_CHECK_OO(par) DYN_DETAIL_BOOST_PP_CHECK_D ## par
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC() && ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_DMC()
#    define DYN_DETAIL_BOOST_PP_CHECK_D(x, type) DYN_DETAIL_BOOST_PP_CHECK_1(DYN_DETAIL_BOOST_PP_CAT(DYN_DETAIL_BOOST_PP_CHECK_RESULT_, type x))
#    define DYN_DETAIL_BOOST_PP_CHECK_1(chk) DYN_DETAIL_BOOST_PP_CHECK_2(chk)
#    define DYN_DETAIL_BOOST_PP_CHECK_2(res, _) res
# elif DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MSVC()
#    define DYN_DETAIL_BOOST_PP_CHECK_D(x, type) DYN_DETAIL_BOOST_PP_CHECK_1(type x)
#    define DYN_DETAIL_BOOST_PP_CHECK_1(chk) DYN_DETAIL_BOOST_PP_CHECK_2(chk)
#    define DYN_DETAIL_BOOST_PP_CHECK_2(chk) DYN_DETAIL_BOOST_PP_CHECK_3((DYN_DETAIL_BOOST_PP_CHECK_RESULT_ ## chk))
#    define DYN_DETAIL_BOOST_PP_CHECK_3(im) DYN_DETAIL_BOOST_PP_CHECK_5(DYN_DETAIL_BOOST_PP_CHECK_4 im)
#    define DYN_DETAIL_BOOST_PP_CHECK_4(res, _) res
#    define DYN_DETAIL_BOOST_PP_CHECK_5(res) res
# else /* DMC */
#    define DYN_DETAIL_BOOST_PP_CHECK_D(x, type) DYN_DETAIL_BOOST_PP_CHECK_OO((type x))
#    define DYN_DETAIL_BOOST_PP_CHECK_OO(par) DYN_DETAIL_BOOST_PP_CHECK_0 ## par
#    define DYN_DETAIL_BOOST_PP_CHECK_0(chk) DYN_DETAIL_BOOST_PP_CHECK_1(DYN_DETAIL_BOOST_PP_CAT(DYN_DETAIL_BOOST_PP_CHECK_RESULT_, chk))
#    define DYN_DETAIL_BOOST_PP_CHECK_1(chk) DYN_DETAIL_BOOST_PP_CHECK_2(chk)
#    define DYN_DETAIL_BOOST_PP_CHECK_2(res, _) res
# endif
#
# define DYN_DETAIL_BOOST_PP_CHECK_RESULT_1 1, DYN_DETAIL_BOOST_PP_NIL
#
# endif
