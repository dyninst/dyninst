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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LIST_REVERSE_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LIST_REVERSE_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/list/fold_left.hpp>
#
# /* DYN_DETAIL_BOOST_PP_LIST_REVERSE */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_REVERSE(list) DYN_DETAIL_BOOST_PP_LIST_FOLD_LEFT(DYN_DETAIL_BOOST_PP_LIST_REVERSE_O, DYN_DETAIL_BOOST_PP_NIL, list)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_REVERSE(list) DYN_DETAIL_BOOST_PP_LIST_REVERSE_I(list)
#    define DYN_DETAIL_BOOST_PP_LIST_REVERSE_I(list) DYN_DETAIL_BOOST_PP_LIST_FOLD_LEFT(DYN_DETAIL_BOOST_PP_LIST_REVERSE_O, DYN_DETAIL_BOOST_PP_NIL, list)
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_REVERSE_O(d, s, x) (x, s)
#
# /* DYN_DETAIL_BOOST_PP_LIST_REVERSE_D */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_REVERSE_D(d, list) DYN_DETAIL_BOOST_PP_LIST_FOLD_LEFT_ ## d(DYN_DETAIL_BOOST_PP_LIST_REVERSE_O, DYN_DETAIL_BOOST_PP_NIL, list)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_REVERSE_D(d, list) DYN_DETAIL_BOOST_PP_LIST_REVERSE_D_I(d, list)
#    define DYN_DETAIL_BOOST_PP_LIST_REVERSE_D_I(d, list) DYN_DETAIL_BOOST_PP_LIST_FOLD_LEFT_ ## d(DYN_DETAIL_BOOST_PP_LIST_REVERSE_O, DYN_DETAIL_BOOST_PP_NIL, list)
# endif
#
# endif
