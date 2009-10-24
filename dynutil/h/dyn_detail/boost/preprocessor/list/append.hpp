# /* Copyright (C) 2001
#  * Housemarque Oy
#  * http://www.housemarque.com
#  *
#  * Distributed under the Boost Software License, Version 1.0. (See
#  * accompanying file LICENSE_1_0.txt or copy at
#  * http://www.boost.org/LICENSE_1_0.txt)
#  */
#
# /* Revised by Paul Mensonides (2002) */
#
# /* See http://www.boost.org for most recent version. */
#
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LIST_APPEND_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LIST_APPEND_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/list/fold_right.hpp>
#
# /* DYN_DETAIL_BOOST_PP_LIST_APPEND */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_APPEND(a, b) DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT(DYN_DETAIL_BOOST_PP_LIST_APPEND_O, b, a)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_APPEND(a, b) DYN_DETAIL_BOOST_PP_LIST_APPEND_I(a, b)
#    define DYN_DETAIL_BOOST_PP_LIST_APPEND_I(a, b) DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT(DYN_DETAIL_BOOST_PP_LIST_APPEND_O, b, a)
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_APPEND_O(d, s, x) (x, s)
#
# /* DYN_DETAIL_BOOST_PP_LIST_APPEND_D */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_APPEND_D(d, a, b) DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_ ## d(DYN_DETAIL_BOOST_PP_LIST_APPEND_O, b, a)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_APPEND_D(d, a, b) DYN_DETAIL_BOOST_PP_LIST_APPEND_D_I(d, a, b)
#    define DYN_DETAIL_BOOST_PP_LIST_APPEND_D_I(d, a, b) DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_ ## d(DYN_DETAIL_BOOST_PP_LIST_APPEND_O, b, a)
# endif
#
# endif
