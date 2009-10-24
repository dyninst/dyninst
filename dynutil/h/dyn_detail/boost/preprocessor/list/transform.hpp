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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_LIST_TRANSFORM_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_LIST_TRANSFORM_HPP
#
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/list/fold_right.hpp>
# include <dyn_detail/boost/preprocessor/tuple/elem.hpp>
# include <dyn_detail/boost/preprocessor/tuple/rem.hpp>
#
# /* DYN_DETAIL_BOOST_PP_LIST_TRANSFORM */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM(op, data, list) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 2, DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT(DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O, (op, data, DYN_DETAIL_BOOST_PP_NIL), list))
# else
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM(op, data, list) DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_I(op, data, list)
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_I(op, data, list) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 2, DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT(DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O, (op, data, DYN_DETAIL_BOOST_PP_NIL), list))
# endif
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O(d, odr, elem) DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O_D(d, DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 0, odr), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 1, odr), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 2, odr), elem)
# else
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O(d, odr, elem) DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O_I(d, DYN_DETAIL_BOOST_PP_TUPLE_REM_3 odr, elem)
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O_I(d, im, elem) DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O_D(d, im, elem)
# endif
#
# define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O_D(d, op, data, res, elem) (op, data, (op(d, data, elem), res))
#
# /* DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_D */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_D(d, op, data, list) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 2, DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_ ## d(DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O, (op, data, DYN_DETAIL_BOOST_PP_NIL), list))
# else
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_D(d, op, data, list) DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_D_I(d, op, data, list)
#    define DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_D_I(d, op, data, list) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(3, 2, DYN_DETAIL_BOOST_PP_LIST_FOLD_RIGHT_ ## d(DYN_DETAIL_BOOST_PP_LIST_TRANSFORM_O, (op, data, DYN_DETAIL_BOOST_PP_NIL), list))
# endif
#
# endif
