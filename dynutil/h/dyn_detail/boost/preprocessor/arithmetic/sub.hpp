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
# ifndef DYN_DETAIL_BOOST_PREPROCESSOR_ARITHMETIC_SUB_HPP
# define DYN_DETAIL_BOOST_PREPROCESSOR_ARITHMETIC_SUB_HPP
#
# include <dyn_detail/boost/preprocessor/arithmetic/dec.hpp>
# include <dyn_detail/boost/preprocessor/config/config.hpp>
# include <dyn_detail/boost/preprocessor/control/while.hpp>
# include <dyn_detail/boost/preprocessor/tuple/elem.hpp>
#
# /* DYN_DETAIL_BOOST_PP_SUB */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_SUB(x, y) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 0, DYN_DETAIL_BOOST_PP_WHILE(DYN_DETAIL_BOOST_PP_SUB_P, DYN_DETAIL_BOOST_PP_SUB_O, (x, y)))
# else
#    define DYN_DETAIL_BOOST_PP_SUB(x, y) DYN_DETAIL_BOOST_PP_SUB_I(x, y)
#    define DYN_DETAIL_BOOST_PP_SUB_I(x, y) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 0, DYN_DETAIL_BOOST_PP_WHILE(DYN_DETAIL_BOOST_PP_SUB_P, DYN_DETAIL_BOOST_PP_SUB_O, (x, y)))
# endif
#
# define DYN_DETAIL_BOOST_PP_SUB_P(d, xy) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 1, xy)
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_MWCC()
#    define DYN_DETAIL_BOOST_PP_SUB_O(d, xy) DYN_DETAIL_BOOST_PP_SUB_O_I xy
# else
#    define DYN_DETAIL_BOOST_PP_SUB_O(d, xy) DYN_DETAIL_BOOST_PP_SUB_O_I(DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 0, xy), DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 1, xy))
# endif
#
# define DYN_DETAIL_BOOST_PP_SUB_O_I(x, y) (DYN_DETAIL_BOOST_PP_DEC(x), DYN_DETAIL_BOOST_PP_DEC(y))
#
# /* DYN_DETAIL_BOOST_PP_SUB_D */
#
# if ~DYN_DETAIL_BOOST_PP_CONFIG_FLAGS() & DYN_DETAIL_BOOST_PP_CONFIG_EDG()
#    define DYN_DETAIL_BOOST_PP_SUB_D(d, x, y) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 0, DYN_DETAIL_BOOST_PP_WHILE_ ## d(DYN_DETAIL_BOOST_PP_SUB_P, DYN_DETAIL_BOOST_PP_SUB_O, (x, y)))
# else
#    define DYN_DETAIL_BOOST_PP_SUB_D(d, x, y) DYN_DETAIL_BOOST_PP_SUB_D_I(d, x, y)
#    define DYN_DETAIL_BOOST_PP_SUB_D_I(d, x, y) DYN_DETAIL_BOOST_PP_TUPLE_ELEM(2, 0, DYN_DETAIL_BOOST_PP_WHILE_ ## d(DYN_DETAIL_BOOST_PP_SUB_P, DYN_DETAIL_BOOST_PP_SUB_O, (x, y)))
# endif
#
# endif
