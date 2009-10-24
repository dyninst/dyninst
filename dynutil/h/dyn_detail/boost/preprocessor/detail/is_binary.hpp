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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_DETAIL_IS_BINARY_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_DETAIL_IS_BINARY_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/detail/check.hpp>
#
# /* DYN_DETAIL_BOOST_PP_IS_BINARY */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_IS_BINARY(x) DYN_DETAIL_BOOST_PP_CHECK(x, DYN_DETAIL_BOOST_PP_IS_BINARY_CHECK)
# else
#    define DYN_DETAIL_BOOST_PP_IS_BINARY(x) DYN_DETAIL_BOOST_PP_IS_BINARY_I(x)
#    define DYN_DETAIL_BOOST_PP_IS_BINARY_I(x) DYN_DETAIL_BOOST_PP_CHECK(x, DYN_DETAIL_BOOST_PP_IS_BINARY_CHECK)
# endif
#
# define DYN_DETAIL_BOOST_PP_IS_BINARY_CHECK(a, b) 1
# define DYN_DETAIL_BOOST_PP_CHECK_RESULT_DYN_DETAIL_BOOST_PP_IS_BINARY_CHECK 0, DYN_DETAIL_BOOST_PP_NIL
#
# endif
