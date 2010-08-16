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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LIST_FOLD_RIGHT_HPP
#
# include <dyn_detail/boost/preprocessor/cat.hpp>
# include <dyn_detail/boost/preprocessor/control/while.hpp>
# include <dyn_detail/boost/preprocessor/debug/error.hpp>
# include <dyn_detail/boost/preprocessor/detail/auto_rec.hpp>
#
# if 0
#    define DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT(op, state, list)
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT DYN_DETAIL_BOOST_PP_CAT(DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_, DYN_DETAIL_BOOST_PP_AUTO_REC(DYN_DETAIL_BOOST_PP_WHILE_P, 256))
#
# define DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_257(o, s, l) DYN_DETAIL_BOOST_PP_ERROR(0x0004)
#
# define DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_D(d, o, s, l) DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_ ## d(o, s, l)
# define DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_2ND DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT
# define DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_2ND_D DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_D
#
# if DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    include <dyn_detail/boost/preprocessor/list/detail/fold_right.hpp>
# else
#    include <dyn_detail/boost/preprocessor/list/detail/fold_right.hpp>
# endif
#
# endif
